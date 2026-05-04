//------------------------------------------------------------------------------------------
// BP_TrophySystemX360.cpp
//
// X360 platform specific trophy functions.
//------------------------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "Engine/System/COsContext.h"
#include "Engine/System/VTAThreadPriorities.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//////// Naming conflict between PSP2 kernel.h and PS2 kernel.h... just gonna include everything from kernel.h
#include <sdk_version.h>

#include <kernel/common.h>
#include <kernel/cpu.h>
#include <kernel/dipsw.h>
#include <kernel/sysmem.h>
#include <kernel/threadmgr.h>
#include <kernel/iofilemgr.h>
#include <kernel/modulemgr.h>
#include <kernel/processmgr.h>
#include <kernel/process_param.h>
#include <kernel/libkernel.h>

#include <kernel/debug.h>
#include <kernel/backtrace.h>
////////

#include <apputil.h>
#include <np.h>
#include <np_trophy_setup_dialog.h>

#include "BP_TrophySystem.h"
#include "BP_SaveLoadMGS.h"
#include "BP_Memory.h"
#include "TransfarringVTACGlue.h"

#include "../../../Builds/DiskBuilds/UseDiskBuild.h"

#define BP_MAX_ACHIEVEMENTS		51
#define BP_NO_ACHIEVEMENT        -1

//----------------------------------------------------------------------------

// whether or not achievements are disabled for a given player.
static bool sbAchievementsEnabled = false;
static bool sbUpdateInProgress = false;

static SceNpTrophyContext  sTrophyContext = SCE_NP_TROPHY_INVALID_CONTEXT;
static int sTrophiesUnlocked[kTRP_Count] = {};

// Types of commands
enum ETrophyCommands
{
   kTC_UnlockTrophy,
   kTC_Shutdown,
   kTC_DebugPrint,

   kTC_Count
};

// The data that goes into the queue
struct STrophySystemJob
{
   STrophySystemJob()
      : command( kTC_DebugPrint )
      , trophyID( kTRP_Count )
   {
   }

   ETrophyCommands command;
   ETrophies trophyID;
};

static SceUID sJobPipeID = -1;
static SceUID sTrophyUnlockThreadId = -1;
static const uint32_t skMaxQueuedTrophies = 16;

#if MGS_VERSION==2

static const SceNpCommunicationId s_npCommunicationId = {
   {'N', 'P', 'W', 'R', '0', '3', '1', '2', '8'},
   '\0',
   0,
   0
};

/***
SceNpCommunicationPassphrase

105b86a1e5cf91667a86fa2753c0dbff8cfe76a0049695614e426cff378e5816e1eebe4449c3b181ef7e45fe69798c69bbfc87d233a56e30561dcb03eb747c378594e75a6d6efb85747f5f202bc4ac5f4baa2b968adc58a4d00bec82ebceeb3f9d93efa04cb36189d546c6e2c6a27a78bcb1a72cd2378d35ca872b74eaa79ab4
***/

static const SceNpCommunicationPassphrase s_npCommunicationPassphrase = {
   {
      0x10,0x5b,0x86,0xa1,0xe5,0xcf,0x91,0x66,
         0x7a,0x86,0xfa,0x27,0x53,0xc0,0xdb,0xff,
         0x8c,0xfe,0x76,0xa0,0x04,0x96,0x95,0x61,
         0x4e,0x42,0x6c,0xff,0x37,0x8e,0x58,0x16,
         0xe1,0xee,0xbe,0x44,0x49,0xc3,0xb1,0x81,
         0xef,0x7e,0x45,0xfe,0x69,0x79,0x8c,0x69,
         0xbb,0xfc,0x87,0xd2,0x33,0xa5,0x6e,0x30,
         0x56,0x1d,0xcb,0x03,0xeb,0x74,0x7c,0x37,
         0x85,0x94,0xe7,0x5a,0x6d,0x6e,0xfb,0x85,
         0x74,0x7f,0x5f,0x20,0x2b,0xc4,0xac,0x5f,
         0x4b,0xaa,0x2b,0x96,0x8a,0xdc,0x58,0xa4,
         0xd0,0x0b,0xec,0x82,0xeb,0xce,0xeb,0x3f,
         0x9d,0x93,0xef,0xa0,0x4c,0xb3,0x61,0x89,
         0xd5,0x46,0xc6,0xe2,0xc6,0xa2,0x7a,0x78,
         0xbc,0xb1,0xa7,0x2c,0xd2,0x37,0x8d,0x35,
         0xca,0x87,0x2b,0x74,0xea,0xa7,0x9a,0xb4
   }
};

/***
SceNpCommunicationSignature

b9dde13b0100000000000000fdda75b08b42d5c1ad7dafc1ab2860d0245b0b6618ce116a9fbed0e3daca6e8a6e0a852dd4a98805800e64d7d4beb04d0391694f1ee387bd2efc34a054341f9e8185a1f3499773fa06f1d313e7530acf7bf68c0e7fbe5bf1e0fd91084d8e18cf58774acfa35042aad129721c0e6dea1d01d7d07f846244b2190e222361f00d8640ecf8ab814e763509ee5aa888b8e88dcba07576
***/

static const SceNpCommunicationSignature s_npCommunicationSignature = {
   {
      0xb9,0xdd,0xe1,0x3b,0x01,0x00,0x00,0x00,
         0x00,0x00,0x00,0x00,0xfd,0xda,0x75,0xb0,
         0x8b,0x42,0xd5,0xc1,0xad,0x7d,0xaf,0xc1,
         0xab,0x28,0x60,0xd0,0x24,0x5b,0x0b,0x66,
         0x18,0xce,0x11,0x6a,0x9f,0xbe,0xd0,0xe3,
         0xda,0xca,0x6e,0x8a,0x6e,0x0a,0x85,0x2d,
         0xd4,0xa9,0x88,0x05,0x80,0x0e,0x64,0xd7,
         0xd4,0xbe,0xb0,0x4d,0x03,0x91,0x69,0x4f,
         0x1e,0xe3,0x87,0xbd,0x2e,0xfc,0x34,0xa0,
         0x54,0x34,0x1f,0x9e,0x81,0x85,0xa1,0xf3,
         0x49,0x97,0x73,0xfa,0x06,0xf1,0xd3,0x13,
         0xe7,0x53,0x0a,0xcf,0x7b,0xf6,0x8c,0x0e,
         0x7f,0xbe,0x5b,0xf1,0xe0,0xfd,0x91,0x08,
         0x4d,0x8e,0x18,0xcf,0x58,0x77,0x4a,0xcf,
         0xa3,0x50,0x42,0xaa,0xd1,0x29,0x72,0x1c,
         0x0e,0x6d,0xea,0x1d,0x01,0xd7,0xd0,0x7f,
         0x84,0x62,0x44,0xb2,0x19,0x0e,0x22,0x23,
         0x61,0xf0,0x0d,0x86,0x40,0xec,0xf8,0xab,
         0x81,0x4e,0x76,0x35,0x09,0xee,0x5a,0xa8,
         0x88,0xb8,0xe8,0x8d,0xcb,0xa0,0x75,0x76
   }
};

static const SceNpCommunicationConfig s_npCommunicationConfig =
{
   &s_npCommunicationId,
   &s_npCommunicationPassphrase,
   &s_npCommunicationSignature
};

#endif

//----------------------------------------------------------------------------

#if MGS_VERSION==3

static const SceNpCommunicationId s_npCommunicationId = {
   {'N', 'P', 'W', 'R', '0', '3', '1', '2', '4'},
   '\0',
   0,
   0
};

/***
SceNpCommunicationPassphrase

86ea614f8b830a463d41b4dde69dc5436c1a6e0e703b6b24c517bd14103fd1ae5e6b9d2ff79855f28bdec30fc14f5ae4f7982f2ee8f985e6a4be337c808db015a5ecc6b335ff67245b30aa340a591e6eacfd09f224bb8c94c947d1f96acae3efc7eb40048619711a99f78b69ba5d0b530807ad20fc23e8ac5ff8855045090a7e
***/

static const SceNpCommunicationPassphrase s_npCommunicationPassphrase = {
   {
      0x86,0xea,0x61,0x4f,0x8b,0x83,0x0a,0x46,
         0x3d,0x41,0xb4,0xdd,0xe6,0x9d,0xc5,0x43,
         0x6c,0x1a,0x6e,0x0e,0x70,0x3b,0x6b,0x24,
         0xc5,0x17,0xbd,0x14,0x10,0x3f,0xd1,0xae,
         0x5e,0x6b,0x9d,0x2f,0xf7,0x98,0x55,0xf2,
         0x8b,0xde,0xc3,0x0f,0xc1,0x4f,0x5a,0xe4,
         0xf7,0x98,0x2f,0x2e,0xe8,0xf9,0x85,0xe6,
         0xa4,0xbe,0x33,0x7c,0x80,0x8d,0xb0,0x15,
         0xa5,0xec,0xc6,0xb3,0x35,0xff,0x67,0x24,
         0x5b,0x30,0xaa,0x34,0x0a,0x59,0x1e,0x6e,
         0xac,0xfd,0x09,0xf2,0x24,0xbb,0x8c,0x94,
         0xc9,0x47,0xd1,0xf9,0x6a,0xca,0xe3,0xef,
         0xc7,0xeb,0x40,0x04,0x86,0x19,0x71,0x1a,
         0x99,0xf7,0x8b,0x69,0xba,0x5d,0x0b,0x53,
         0x08,0x07,0xad,0x20,0xfc,0x23,0xe8,0xac,
         0x5f,0xf8,0x85,0x50,0x45,0x09,0x0a,0x7e
   }
};

/***
SceNpCommunicationSignature

b9dde13b0100000000000000f04f3507e3740faaedd61fe15f75e724fdc5226b0a9a0caff17b76c704b37a4e8230a05d3c0de3ccf7c24537908e20a703acd08779876c2b1ad86c3bd42ef4589aef67532dd28e92e3d4d4e8787f020b824e3875fde3b628f83454ecb6a327e70e1f9ad07085e85596133d6a5fe5d37776cbe186c6e61075095c10436c35c818c0ebc18efddd6bdccce2d3da784153c18d5342f3
***/

static const SceNpCommunicationSignature s_npCommunicationSignature = {
   {
      0xb9,0xdd,0xe1,0x3b,0x01,0x00,0x00,0x00,
         0x00,0x00,0x00,0x00,0xf0,0x4f,0x35,0x07,
         0xe3,0x74,0x0f,0xaa,0xed,0xd6,0x1f,0xe1,
         0x5f,0x75,0xe7,0x24,0xfd,0xc5,0x22,0x6b,
         0x0a,0x9a,0x0c,0xaf,0xf1,0x7b,0x76,0xc7,
         0x04,0xb3,0x7a,0x4e,0x82,0x30,0xa0,0x5d,
         0x3c,0x0d,0xe3,0xcc,0xf7,0xc2,0x45,0x37,
         0x90,0x8e,0x20,0xa7,0x03,0xac,0xd0,0x87,
         0x79,0x87,0x6c,0x2b,0x1a,0xd8,0x6c,0x3b,
         0xd4,0x2e,0xf4,0x58,0x9a,0xef,0x67,0x53,
         0x2d,0xd2,0x8e,0x92,0xe3,0xd4,0xd4,0xe8,
         0x78,0x7f,0x02,0x0b,0x82,0x4e,0x38,0x75,
         0xfd,0xe3,0xb6,0x28,0xf8,0x34,0x54,0xec,
         0xb6,0xa3,0x27,0xe7,0x0e,0x1f,0x9a,0xd0,
         0x70,0x85,0xe8,0x55,0x96,0x13,0x3d,0x6a,
         0x5f,0xe5,0xd3,0x77,0x76,0xcb,0xe1,0x86,
         0xc6,0xe6,0x10,0x75,0x09,0x5c,0x10,0x43,
         0x6c,0x35,0xc8,0x18,0xc0,0xeb,0xc1,0x8e,
         0xfd,0xdd,0x6b,0xdc,0xcc,0xe2,0xd3,0xda,
         0x78,0x41,0x53,0xc1,0x8d,0x53,0x42,0xf3
   }
};

static const SceNpCommunicationConfig s_npCommunicationConfig =
{
   &s_npCommunicationId,
   &s_npCommunicationPassphrase,
   &s_npCommunicationSignature
};

#endif

//----------------------------------------------------------------------------

static void push_trophy_job( STrophySystemJob const &job )
{
   BPE_CHECK_SCE( sceKernelSendMsgPipe( sJobPipeID, &job, sizeof( STrophySystemJob ), SCE_KERNEL_MSG_PIPE_MODE_FULL | SCE_KERNEL_MSG_PIPE_MODE_WAIT, NULL, NULL ) );
}


//----------------------------------------------------------------------------

// Only call this from the unlock thread. GetTrophyUnlockState blocks.
static void build_unlocked_trophy_list()
{
   SceNpTrophyHandle handle = SCE_NP_TROPHY_INVALID_HANDLE;
   sceNpTrophyCreateHandle(&handle);

   SceNpTrophyFlagArray flags;
   SceUInt32 count = 0;

   SCE_NP_TROPHY_FLAG_ZERO(&flags);

   sceNpTrophyGetTrophyUnlockState(sTrophyContext, handle, &flags, &count);

   for (unsigned int i=0; i < kTRP_Count; i++)
   {
      if (SCE_NP_TROPHY_FLAG_ISSET(i, &flags))
      {
         sTrophiesUnlocked[i] = 1;
      }
   }

   sceNpTrophyDestroyHandle(handle);
}

bool UnlockTrophyInternal_UnlockThread(ETrophies trophyID)
{
   bool retval = true;

   SceNpTrophyHandle handle = SCE_NP_TROPHY_INVALID_HANDLE;
   SceNpTrophyId platinumID = SCE_NP_TROPHY_INVALID_TROPHY_ID;
   int success;

   // Create handle
   success = sceNpTrophyCreateHandle(&handle);
   if (success < 0)
   {
      // Handle failure
   }

   // Unlock trophy, and don't let the user interrupt it
   OsContext()->SetUnsafeToShutDownFlag( CBaseOsContext::kUSDF_Trophies );
   success = sceNpTrophyUnlockTrophy(sTrophyContext, handle, trophyID, &platinumID);
   OsContext()->ClearUnsafeToShutDownFlag( CBaseOsContext::kUSDF_Trophies );

   if (success < 0)
   {
      if (success == SCE_NP_TROPHY_ERROR_TROPHY_ALREADY_UNLOCKED)
      {
         // Add this to the list of unlocked trophies so it doesn't try again
         sTrophiesUnlocked[trophyID] = 1;
      }
      else if (success == SCE_NP_TROPHY_ERROR_UNLOCK_DENIED)
      {
         // Shut down the whole system
         sbAchievementsEnabled = false;
         retval = false;
      }
   }

   // Destroy handle
   success = sceNpTrophyDestroyHandle(handle);

   return retval;
}

// Looped in the thread that handles trophy unlocking
SceInt32 trophy_system_unlock_thread(SceSize argSize, void *pArgBlock )
{
   // If the trophy data is being updating, don't try to unlock anything
   while(sbUpdateInProgress)
   {
      // Spin
      sceKernelDelayThread(1000);
   }

   // Populate the list of unlocked trophies so we don't try to unlock trophies we already have
   build_unlocked_trophy_list();

   // Get into a loop that processes commands
   bool bQuit = false;
   while (!bQuit)
   {
      STrophySystemJob job;

      BPE_CHECK_SCE( sceKernelReceiveMsgPipe( sJobPipeID, &job, sizeof( STrophySystemJob ), SCE_KERNEL_MSG_PIPE_MODE_FULL | SCE_KERNEL_MSG_PIPE_MODE_WAIT, NULL, NULL ) );

      switch(job.command)
      {
      case kTC_UnlockTrophy:
         bQuit = !UnlockTrophyInternal_UnlockThread( job.trophyID );
         break;
      case kTC_Shutdown:
         bQuit = true;
         break;
      case kTC_DebugPrint:
         printf( "TrophySystemVTA: Trophy debug print from job thread!\n" );
         break;
      }
   }

   return 0;
}

//----------------------------------------------------------------------------

extern "C" void BP_TrophySystem_Init()
{
#if BP_TGS_DEMO()
   // No trophies on TGS Demo as per KJP 2011-08-24
   printf("BP_TrophySystem_Init() - Trophies disabled for TGS Demo.\n");
   return;
#endif

   //TED - there's an interdependency b/t trophy and savedata initialization on PS3.
   //There's no such issue on X360, but for consistency I'm initializing savedata in
   //the same place here.
   MGS_SaveStatus_Init();

   sbAchievementsEnabled = true;

   int success;

   //   int success = sceNpInit(NULL, NULL);
   //   sceNpInit now happens elsewhere

   // Trophy module was already loaded in VTACOsContext.cpp
   // Initialize trophy library
   success = sceNpTrophyInit(NULL);

   // Create trophy context
   success = sceNpTrophyCreateContext(&sTrophyContext, &s_npCommunicationId, &s_npCommunicationSignature, 0);

   SceNpTrophySetupDialogParam param;
   memset(&param, 0, sizeof(SceNpTrophySetupDialogParam));
   sceNpTrophySetupDialogParamInit(&param);
   param.context = sTrophyContext;
#if SCE_PSP2_SDK_VERSION >= 0x01600061
   param.options |= SCE_NP_TROPHY_SETUP_DIALOG_OPTION_SHOW_PROCESSING_DIALOG;
#endif

   sbUpdateInProgress = true;

   // Initialize our message pipe
   sJobPipeID = BPE_CHECK_SCE( sceKernelCreateMsgPipe( "Trophy Pipe", SCE_KERNEL_MSG_PIPE_TYPE_USER_MAIN, 
      SCE_KERNEL_MSG_PIPE_ATTR_TH_FIFO | SCE_KERNEL_EVENT_ATTR_AUTO_RESET | SCE_KERNEL_ATTR_NOTIFY_CB_ALL,
      4096,
      NULL ) );

   // launch the trophy unlocking thread
   sTrophyUnlockThreadId = BPE_CHECK_SCE( sceKernelCreateThread( 
      "TrophySystem_TrophyUnlockThread",
      trophy_system_unlock_thread,
      NVtaThreadPriorities::kPriorityUltWorkers,
      32 * 1024 /* stack size */,
      0,
      SCE_KERNEL_CPU_MASK_USER_ALL,
      NULL ) );

   BPE_CHECK_SCE( sceKernelStartThread( sTrophyUnlockThreadId, 0, NULL ) );

#if 0
   // Put some debug prints in
   STrophySystemJob job;

   job.command = kTC_DebugPrint;

   for ( int i = 0; i < 10; ++i )
   {
      printf( "TrophySystemVTA: Pushing debug job %d\n", i );
      push_trophy_job( job );
   }
#endif

   // This function saves the trophy set into memory, updates it if necessary, and synchronizes it with the server
   success = sceNpTrophySetupDialogInit(&param);
   if (success < 0)
   {
      // TODO: Handle error
   }
}

//----------------------------------------------------------------------------

extern "C" void BP_TrophySystem_Shutdown()
{
   if (sbAchievementsEnabled)
   {
      sbAchievementsEnabled = false;

      // Kill the thread
      STrophySystemJob job;
      job.command = kTC_Shutdown;
      job.trophyID = kTRP_Count; // Bogus

      push_trophy_job( job );

      // Wait for the thread to exit

      BPE_CHECK_SCE( sceKernelWaitThreadEnd( sTrophyUnlockThreadId, NULL, NULL ) );
      sTrophyUnlockThreadId = -1;

      BPE_CHECK_SCE( sceKernelDeleteMsgPipe( sJobPipeID ) );
      sJobPipeID = -1;

      // Destroy the trophy-related contexts
      sceNpTrophyDestroyContext(sTrophyContext);

      // Shut down the trophy library
      sceNpTrophyTerm();
   }
}

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

extern "C" int BP_TrophySystem_UpdateInProgress()
{
   return sbUpdateInProgress;
}

//----------------------------------------------------------------------------
// Renderer depends on Shared, so rather than call the renderer from the trophy system, 
// we'll have the renderer tell the trophy system when the dialog is complete.
extern "C" void BP_TrophySystem_SignalUpdateFinished()
{
   sbUpdateInProgress = false;
}

//----------------------------------------------------------------------------

extern "C" void BP_TrophySystem_Disable()
{
   // disable trophies.
   sbAchievementsEnabled = false;
}

//----------------------------------------------------------------------------

extern "C" void BP_TrophySystem_Enable()
{
   sbAchievementsEnabled = true;
}

//----------------------------------------------------------------------------

extern "C" int BP_TrophySystem_IsDisabled()
{
   return sbAchievementsEnabled ? 0 : 1;
}

//----------------------------------------------------------------------------

extern "C" void BP_TrophySystem_UnlockTrophy(ETrophies trophyId)
{
   // Platinum trophies are handled by the OS.
   BPE_ASSERT( trophyId != kTRP_Platinum, "kTRP_Platinum may not be used as a trophy!\n" );
   BPE_ASSERT( trophyId < kTRP_Count, "Invalid trophy index!\n" );

   if( sbAchievementsEnabled )
   {
      BP_TrophySystem_SaveTrophyToBitfield( trophyId, &bp_linkvars.mTrophyDeltaBitfield );
   }

   // Check to see if this trophy was already unlocked, to avoid spamming the queue
   if (sTrophiesUnlocked[trophyId] != 0)
   {
      return;
   }

   // Flag trophy as unlocked
   sTrophiesUnlocked[trophyId] = 1;

   if (sbAchievementsEnabled)
   {
      // Add to the event queue
      STrophySystemJob job;
      job.command = kTC_UnlockTrophy;
      job.trophyID = trophyId;

      push_trophy_job( job );
   }
}

//----------------------------------------------------------------------------

extern "C" void BP_TrophySystem_UnlockTrophiesFromBitfield( const STrophyDeltaBitfield* bitfield )
{
   // For each byte in the array
   for( unsigned char i=0; i < sizeof( bitfield->mBits ); i++ )
   {
      // For each bit in the byte
      for( unsigned char j=0; j < 8; j++ )
      {
         // If the bit is on, unlock it
         if( bitfield->mBits[i] & (1 << j) )
         {
            ETrophies trophyID = (ETrophies)( i * 8 + j );
            BP_TrophySystem_UnlockTrophy( trophyID );
         }
      }
   }
}

//----------------------------------------------------------------------------

extern "C" void BP_TrophySystem_SaveTrophiesToBitfield( STrophyDeltaBitfield* bitfield )
{
   for( unsigned int i=0; i < kTRP_Count; i++ )
   {
      if( i == kTRP_Platinum )
      {
         continue;
      }

      if( sTrophiesUnlocked[i] )
      {
         int byteIndex = i / 8;
         int bitIndex = i % 8;
         bitfield->mBits[byteIndex] |= 1 << bitIndex;
      }
   }
}

void BP_TrophySystem_SaveTrophyToBitfield( ETrophies trophyID, STrophyDeltaBitfield* pBitfield )
{
   if( trophyID != kTRP_Platinum )
   {
      int byteIndex = trophyID / 8;
      int bitIndex = trophyID % 8;
      pBitfield->mBits[byteIndex] |= 1 << bitIndex;
   }
}