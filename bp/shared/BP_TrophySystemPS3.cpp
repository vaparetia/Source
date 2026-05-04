//------------------------------------------------------------------------------------------
// BP_TrophySystem.cpp
// Main functions for PS3 trophies.
//------------------------------------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cell/atomic.h>
#include <cell/cell_fs.h>
#include <cell/sysmodule.h>
#include <sys/ppu_thread.h>
#include <sys/event.h>
#include <sys/timer.h>
#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_savedata.h>
#include <np.h>

#include <string>
#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/BPEEngineAPI.h"
#include "Engine/System/COsContext.h"

#include "BP_TrophySystem.h"

#include "assert.h"
#include "BP_SaveLoadMGS.h"

#include "../../../Builds/DiskBuilds/UseDiskBuild.h"

extern "C" void bp_init_save_data2();

#ifdef GOLD_VERSION
   #define printf(...)
#endif

//----------------------------------------------------------------------------

static bool                sbTrophiesInitialized = false;
static bool                sbTrophiesAborted = false;
static bool                sbTrophiesDisabled = false;
static int                 sbTrophyRegisterError = SCE_NP_TROPHY_ERROR_NOT_INITIALIZED;
static uint64_t            sTrophyRequiredSpace = 0;
static uint32_t            sTrophyInitInProgress = 0;
static sys_ppu_thread_t    sTrophyThread;
static sys_event_port_t    sTrophyEventQueuePort;
static sys_event_queue_t   sTrophyEventQueue;
static int                 sTrophiesUnlocked[kTRP_Count] = {}; // Zero initialized array of ints

static SceNpTrophyContext  sTrophyContext = SCE_NP_TROPHY_INVALID_CONTEXT;
static SceNpTrophyHandle   sTrophyHandle = SCE_NP_TROPHY_INVALID_HANDLE;

static bool                sbInsufficientAppHDDSpace = false;

//----------------------------------------------------------------------------

#if MGS_VERSION==2

static const SceNpCommunicationId sNpCommunicationId = {
        {'N', 'P', 'W', 'R', '0', '2', '3', '8', '0'},
        '\0',
        0,
        0
};

/***
SceNpCommunicationPassphrase

480c4e2056b6a272276b35c7b4d500982537c4d8db644e3d289b70ad98bc61f43e4ff9f708f7b124a5d5aee092d4ec614cdef552379274eaa79c7b91b56d855d6603574bf8d3d0403673ddce1637f8d2a5f4273aad83625e5119a4217d43272e8c41137e41520c8b0f7afe1fda8e89dff90deb11b24f9061fbe2aa9acfbdaa44
***/

static const SceNpCommunicationPassphrase sNpCommunicationPassphrase = {
        {
                0x48,0x0c,0x4e,0x20,0x56,0xb6,0xa2,0x72,
                0x27,0x6b,0x35,0xc7,0xb4,0xd5,0x00,0x98,
                0x25,0x37,0xc4,0xd8,0xdb,0x64,0x4e,0x3d,
                0x28,0x9b,0x70,0xad,0x98,0xbc,0x61,0xf4,
                0x3e,0x4f,0xf9,0xf7,0x08,0xf7,0xb1,0x24,
                0xa5,0xd5,0xae,0xe0,0x92,0xd4,0xec,0x61,
                0x4c,0xde,0xf5,0x52,0x37,0x92,0x74,0xea,
                0xa7,0x9c,0x7b,0x91,0xb5,0x6d,0x85,0x5d,
                0x66,0x03,0x57,0x4b,0xf8,0xd3,0xd0,0x40,
                0x36,0x73,0xdd,0xce,0x16,0x37,0xf8,0xd2,
                0xa5,0xf4,0x27,0x3a,0xad,0x83,0x62,0x5e,
                0x51,0x19,0xa4,0x21,0x7d,0x43,0x27,0x2e,
                0x8c,0x41,0x13,0x7e,0x41,0x52,0x0c,0x8b,
                0x0f,0x7a,0xfe,0x1f,0xda,0x8e,0x89,0xdf,
                0xf9,0x0d,0xeb,0x11,0xb2,0x4f,0x90,0x61,
                0xfb,0xe2,0xaa,0x9a,0xcf,0xbd,0xaa,0x44
        }
};

/***
SceNpCommunicationSignature

b9dde13b01000000000000008a4869c65431c16011a963d549e0e75395fa7fde4148e18d538b73cef2d05163c0160aa048e65a462aaec8e7539af9c4db08b115af84160916240769c466d38aa1008d587be3c16fe5bd25c294ba9b4ca3b3ccbfee29ced2d8e6d11efadec35d485970130c8394e4ede4baa2c30b8a79d840aae54e15ce9a1090424b4a18442a0ea3e7a6264e3073391491b31b4150bb36753cf5
***/

static const SceNpCommunicationSignature sNpCommunicationSignature = {
        {
                0xb9,0xdd,0xe1,0x3b,0x01,0x00,0x00,0x00,
                0x00,0x00,0x00,0x00,0x8a,0x48,0x69,0xc6,
                0x54,0x31,0xc1,0x60,0x11,0xa9,0x63,0xd5,
                0x49,0xe0,0xe7,0x53,0x95,0xfa,0x7f,0xde,
                0x41,0x48,0xe1,0x8d,0x53,0x8b,0x73,0xce,
                0xf2,0xd0,0x51,0x63,0xc0,0x16,0x0a,0xa0,
                0x48,0xe6,0x5a,0x46,0x2a,0xae,0xc8,0xe7,
                0x53,0x9a,0xf9,0xc4,0xdb,0x08,0xb1,0x15,
                0xaf,0x84,0x16,0x09,0x16,0x24,0x07,0x69,
                0xc4,0x66,0xd3,0x8a,0xa1,0x00,0x8d,0x58,
                0x7b,0xe3,0xc1,0x6f,0xe5,0xbd,0x25,0xc2,
                0x94,0xba,0x9b,0x4c,0xa3,0xb3,0xcc,0xbf,
                0xee,0x29,0xce,0xd2,0xd8,0xe6,0xd1,0x1e,
                0xfa,0xde,0xc3,0x5d,0x48,0x59,0x70,0x13,
                0x0c,0x83,0x94,0xe4,0xed,0xe4,0xba,0xa2,
                0xc3,0x0b,0x8a,0x79,0xd8,0x40,0xaa,0xe5,
                0x4e,0x15,0xce,0x9a,0x10,0x90,0x42,0x4b,
                0x4a,0x18,0x44,0x2a,0x0e,0xa3,0xe7,0xa6,
                0x26,0x4e,0x30,0x73,0x39,0x14,0x91,0xb3,
                0x1b,0x41,0x50,0xbb,0x36,0x75,0x3c,0xf5
        }
};

#if 0 //AndyO: SDK360?
static const SceNpCommunicationConfig sNpCommunicationConfig =
{
        &sNpCommunicationId,
        &sNpCommunicationPassphrase,
        &sNpCommunicationSignature
};
#endif

#endif

//----------------------------------------------------------------------------

#if MGS_VERSION==3

static const SceNpCommunicationId sNpCommunicationId = {
        {'N', 'P', 'W', 'R', '0', '2', '3', '8', '1'},
        '\0',
        0,
        0
};

/***
SceNpCommunicationPassphrase

ba7e4c519a1c1d581f42ea4bd7e70aa805cdaca1072ef5a2182d2fe1b47c8da7493808f82978cfffce0ff8d7297c9a2cfdea63f85001edeecd6d065855c21d24af7e19e9c795b11bdf354727cfaa8eeabca8e281c7097531d439aefe51eedba96f2f0978335ac6e4db4b4f3661278465b611c7e37e0a2b2dd5dbcc4c1b30b207
***/

static const SceNpCommunicationPassphrase sNpCommunicationPassphrase = {
        {
                0xba,0x7e,0x4c,0x51,0x9a,0x1c,0x1d,0x58,
                0x1f,0x42,0xea,0x4b,0xd7,0xe7,0x0a,0xa8,
                0x05,0xcd,0xac,0xa1,0x07,0x2e,0xf5,0xa2,
                0x18,0x2d,0x2f,0xe1,0xb4,0x7c,0x8d,0xa7,
                0x49,0x38,0x08,0xf8,0x29,0x78,0xcf,0xff,
                0xce,0x0f,0xf8,0xd7,0x29,0x7c,0x9a,0x2c,
                0xfd,0xea,0x63,0xf8,0x50,0x01,0xed,0xee,
                0xcd,0x6d,0x06,0x58,0x55,0xc2,0x1d,0x24,
                0xaf,0x7e,0x19,0xe9,0xc7,0x95,0xb1,0x1b,
                0xdf,0x35,0x47,0x27,0xcf,0xaa,0x8e,0xea,
                0xbc,0xa8,0xe2,0x81,0xc7,0x09,0x75,0x31,
                0xd4,0x39,0xae,0xfe,0x51,0xee,0xdb,0xa9,
                0x6f,0x2f,0x09,0x78,0x33,0x5a,0xc6,0xe4,
                0xdb,0x4b,0x4f,0x36,0x61,0x27,0x84,0x65,
                0xb6,0x11,0xc7,0xe3,0x7e,0x0a,0x2b,0x2d,
                0xd5,0xdb,0xcc,0x4c,0x1b,0x30,0xb2,0x07
        }
};

/***
SceNpCommunicationSignature

b9dde13b0100000000000000ad3948c7b8b6ad9222db2fcbc951b06ecfccef890313e4b0ab80b3ed0679524f46f916c9ef9d2b589059a06b78bd9d7f56ebc9ff3c709cc1d1abbee30756f5c5cb2c351d2d0eabdbac298d431c3cadd023231c5b8d0d42f50481092e7d29ff8b480d09b89e9e26c0455aba166c1576ed28c0662d972f2b44dc33613293333537a766ed16539ab528dcf3e09916ba15210980971b
***/

static const SceNpCommunicationSignature sNpCommunicationSignature = {
        {
                0xb9,0xdd,0xe1,0x3b,0x01,0x00,0x00,0x00,
                0x00,0x00,0x00,0x00,0xad,0x39,0x48,0xc7,
                0xb8,0xb6,0xad,0x92,0x22,0xdb,0x2f,0xcb,
                0xc9,0x51,0xb0,0x6e,0xcf,0xcc,0xef,0x89,
                0x03,0x13,0xe4,0xb0,0xab,0x80,0xb3,0xed,
                0x06,0x79,0x52,0x4f,0x46,0xf9,0x16,0xc9,
                0xef,0x9d,0x2b,0x58,0x90,0x59,0xa0,0x6b,
                0x78,0xbd,0x9d,0x7f,0x56,0xeb,0xc9,0xff,
                0x3c,0x70,0x9c,0xc1,0xd1,0xab,0xbe,0xe3,
                0x07,0x56,0xf5,0xc5,0xcb,0x2c,0x35,0x1d,
                0x2d,0x0e,0xab,0xdb,0xac,0x29,0x8d,0x43,
                0x1c,0x3c,0xad,0xd0,0x23,0x23,0x1c,0x5b,
                0x8d,0x0d,0x42,0xf5,0x04,0x81,0x09,0x2e,
                0x7d,0x29,0xff,0x8b,0x48,0x0d,0x09,0xb8,
                0x9e,0x9e,0x26,0xc0,0x45,0x5a,0xba,0x16,
                0x6c,0x15,0x76,0xed,0x28,0xc0,0x66,0x2d,
                0x97,0x2f,0x2b,0x44,0xdc,0x33,0x61,0x32,
                0x93,0x33,0x35,0x37,0xa7,0x66,0xed,0x16,
                0x53,0x9a,0xb5,0x28,0xdc,0xf3,0xe0,0x99,
                0x16,0xba,0x15,0x21,0x09,0x80,0x97,0x1b
        }
};

#if 0 //AndyO: SDK360?
static const SceNpCommunicationConfig s_npCommunicationConfig =
{
        &sNpCommunicationId,
        &sNpCommunicationPassphrase,
        &sNpCommunicationSignature
};
#endif

#endif

//----------------------------------------------------------------------------

static int cb_trophy(SceNpTrophyContext context, SceNpTrophyStatus status, int completed, int total, void *arg)
{
	int ret;

	(void)arg;
	static int sPreviousStatus = -1;
	if (status != sPreviousStatus)
	{
		printf("cb_trophy context=%d, status=%d, completed=%d/%d\n", context, status, completed, total);
		sPreviousStatus = status;
	}

	switch (status)
	{
	case SCE_NP_TROPHY_STATUS_NOT_INSTALLED:
		break;
	case SCE_NP_TROPHY_STATUS_DATA_CORRUPT:
		break;
	case SCE_NP_TROPHY_STATUS_INSTALLED:
		break;
	case SCE_NP_TROPHY_STATUS_REQUIRES_UPDATE:
		break;
	case SCE_NP_TROPHY_STATUS_PROCESSING_SETUP:
		break;
	case SCE_NP_TROPHY_STATUS_PROCESSING_PROGRESS:
		break;
	case SCE_NP_TROPHY_STATUS_PROCESSING_FINALIZE:
		break;
	case SCE_NP_TROPHY_STATUS_PROCESSING_COMPLETE:
		break;

	case SCE_NP_TROPHY_STATUS_UNKNOWN:
	default:
		printf("cb_trophy SCE_NP_TROPHY_STATUS_UNKNOWN (%d)\n", status);
		break;
	}

	// Check for termination
	if (gpOsContext->mShouldTerminateApplication)
	{
		// Application wants to quit, terminate trophy processing
		sbTrophiesAborted = true;
		return -1;
	}

	return 0;
}

//----------------------------------------------------------------------------

enum ETrophyCommands
{
	kTC_UnlockTrophy,
	kTC_Shutdown,

	kTC_Count
};

static void thr_trophy_init_run(uint64_t arg)
{
	printf( "thr_trophy_init_run() start\n");

	int ret;

	// Load PRX
	ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP_TROPHY);

	ret = sceNpTrophyInit(NULL, 0, SYS_MEMORY_CONTAINER_ID_INVALID, 0);
	if (ret < 0) {
		printf("sceNpTrophyInit() failed. ret = 0x%x\n", ret);
		sceNpTrophyTerm();
		cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP_TROPHY);
		return;
	}

	ret = sceNpTrophyCreateContext(
		&sTrophyContext,
		&sNpCommunicationId,
		&sNpCommunicationSignature,
		0);
	if (ret < 0)
	{
		printf("Trophy initialization error!\n");
		printf("sceNpTrophyCreateContext() failed. ret = 0x%x\n", ret);
		sceNpTrophyTerm();
		cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP_TROPHY);
		cellAtomicStore32(&sTrophyInitInProgress, 0);
		sys_ppu_thread_exit(0);
	}

	ret = sceNpTrophyCreateHandle(&sTrophyHandle);
	if (ret < 0)
	{
		printf("Trophy initialization error!\n");
		printf("sceNpTrophyCreateHandle() failed. ret = 0x%x\n", ret);
		sceNpTrophyDestroyContext(sTrophyContext);
		sceNpTrophyTerm();
		cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP_TROPHY);
		cellAtomicStore32(&sTrophyInitInProgress, 0);
		sys_ppu_thread_exit(0);
	}

   ret = sceNpTrophyGetRequiredDiskSpace(sTrophyContext, sTrophyHandle, &sTrophyRequiredSpace, 0);
   bpe_debugger_printf("sceNpTrophyGetRequiredDiskSpace: %dB / %dKB\n", sTrophyRequiredSpace, sTrophyRequiredSpace / 1024);

   //Immediately start initial savedata loading.
   //Trophy system needs to know HDD space req. for savedata, if any, before it tries to install
   //trophy pack.  If there is insufficient space for either we must bail without writing any data.
   MGS_SaveStatus_Init();

   //Must stall on this.
   while (!MGS_SaveStatus_IsDone())
   {
      sys_timer_usleep(1000);
   }

   //The only case we check here is for insufficient space.
   if( MGS_SaveStatus_HasNoSpace() )
   {
      sbInsufficientAppHDDSpace = true;
   }
   else
   {
	   // register the trophy context (installs as well if necessary)
      OsContext()->SetUnsafeToShutDownFlag( CBaseOsContext::kUSDF_Trophies );  // don't allow the user to interrupt this
	   sbTrophyRegisterError = sceNpTrophyRegisterContext(
		   sTrophyContext,
		   sTrophyHandle,
		   cb_trophy,
		   NULL,
		   0);
      OsContext()->ClearUnsafeToShutDownFlag( CBaseOsContext::kUSDF_Trophies );   // the critical process is complete.. you may shut down now
   }

	if (sbTrophyRegisterError < 0 || sbInsufficientAppHDDSpace)
	{
		if (sbTrophyRegisterError == (int)SCE_NP_TROPHY_ERROR_ABORT)
		{
			sbTrophiesAborted = true;
		}

		printf("Trophy initialization error!\n");
      if( sbInsufficientAppHDDSpace )
      {
         printf("Insufficient HDD space.  Extra req: %d\n", MGS_SaveStatus_ExtraSpaceReq() );
      }
      else
      {
   		printf("sceNpTrophyRegisterContext() failed. ret = 0x%x\n", sbTrophyRegisterError);
      }

		sceNpTrophyDestroyHandle(sTrophyHandle);
		sceNpTrophyDestroyContext(sTrophyContext);
		sceNpTrophyTerm();
		cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP_TROPHY);
		cellAtomicStore32(&sTrophyInitInProgress, 0);
		sys_ppu_thread_exit(0);
	}

   //Reset req. space to 0 after trophies have been successfully installed, so correct req. for savedata can be calculated later
   sTrophyRequiredSpace = 0;

	// Initialization completed
	sbTrophiesInitialized = true;
	cellAtomicStore32(&sTrophyInitInProgress, 0);

	// Wait for events
	bool bQuit = false;
	while (!bQuit)
	{
		sys_event_t event;
		ret = sys_event_queue_receive(sTrophyEventQueue, &event, SYS_NO_TIMEOUT);
		if (ret == ECANCELED)
		{
			// Event queue has been destroyed.
			break;
		}
		switch (event.data1)
		{
		case kTC_UnlockTrophy:
			{
				int32_t const trophyId = (int32_t) event.data2;
				SceNpTrophyId platinumId = SCE_NP_TROPHY_INVALID_TROPHY_ID;
            OsContext()->SetUnsafeToShutDownFlag( CBaseOsContext::kUSDF_Trophies );  // don't allow the user to interrupt this
				ret = sceNpTrophyUnlockTrophy(sTrophyContext, sTrophyHandle, trophyId, &platinumId);
            OsContext()->ClearUnsafeToShutDownFlag( CBaseOsContext::kUSDF_Trophies );   // the critical process is complete.. you may shut down now
				char const * const pTrophyDebugString = BP_TrophySystem_GetDebugString((ETrophies) trophyId);
				if ((ret < 0) && (ret != SCE_NP_TROPHY_ERROR_ALREADY_UNLOCKED))
				{
					printf("Couldn't unlock trophy (%08x): [%02d] %s\n", ret, trophyId, pTrophyDebugString);
				}
				else
				{
					printf("Trophy unlocked: [%02d] %s\n", trophyId, pTrophyDebugString);
				}
			}
			break;
		case kTC_Shutdown:
			bQuit = true;
			break;
		default:
			printf("Unknown trophy event: %d\n", event.data1);
			break;
		}
	}

	sbTrophiesInitialized = false;
	sceNpTrophyDestroyHandle(sTrophyHandle);
	sceNpTrophyDestroyContext(sTrophyContext);
	sceNpTrophyTerm();
	cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_NP_TROPHY);

	sys_ppu_thread_exit(0);
}

//----------------------------------------------------------------------------

static bool trophy_data_init_run()
{
   BPE_VERIFY(!BP_TrophySystem_UpdateInProgress(), false, "Trophy update already in progress!");

	// Set trophy init in progress   
	cellAtomicStore32(&sTrophyInitInProgress, 1);

	int ret = 0;

	// Create an event queue to send unlock commands to the trophy system.
	ret = sys_event_port_create(&sTrophyEventQueuePort, SYS_EVENT_PORT_LOCAL, SYS_EVENT_PORT_NO_NAME);
	sys_event_queue_attribute_t queue_attr = {SYS_SYNC_FIFO, SYS_PPU_QUEUE};
	uint64_t queue_size = 32; // 32 queue entries should be enough for unlocking trophies
	ret = sys_event_queue_create(&sTrophyEventQueue, &queue_attr, SYS_EVENT_QUEUE_LOCAL, queue_size);
	ret = sys_event_port_connect_local(sTrophyEventQueuePort, sTrophyEventQueue);

	static int const skSavedataCmdPrio = 1001;
	static int const skSavedataStackSize = 32 * 1024;  // Allow enough stack for TRACE(SET_GENERAL,  (16k OK otherwise)

	// To prevent interrupting the processing of the main thread, save data utility function must be called using a sub thread
	ret = sys_ppu_thread_create(&sTrophyThread,
		thr_trophy_init_run,
		(uint64_t) NULL,
		skSavedataCmdPrio, 
		skSavedataStackSize,
		SYS_PPU_THREAD_CREATE_JOINABLE,
		"BPETrophyInitThread");


	if (ret != 0)
	{
		printf("Trophy thread create failed %d\n", ret);
		return false;
	}

	return true;
}

//----------------------------------------------------------------------------

extern "C" int BP_TrophySystem_UpdateInProgress()
{
	int const bInProgress = (cellAtomicNop32(&sTrophyInitInProgress) == 1);

	return bInProgress;
}

//----------------------------------------------------------------------------

extern "C" void BP_TrophySystem_Init()
{
#if BP_TGS_DEMO()
   // No trophies on TGS Demo as per KJP 2011-08-24
	printf("BP_TrophySystem_Init() - Trophies disabled for TGS Demo.\n");
   return;
#endif

	if (sbTrophiesInitialized)
	{
		printf("trophy_data_init: Already initialized.\n");
		return;
	}
	trophy_data_init_run();
}

//----------------------------------------------------------------------------

extern "C" void BP_TrophySystem_UnlockTrophy(ETrophies trophy)
{
	if (sbTrophiesDisabled)
	{
		return;
	}

	if (!sbTrophiesInitialized && !BP_TrophySystem_UpdateInProgress())
	{
		// Trophies aren't available.
		return;
	}

   if (sTrophiesUnlocked[trophy] != 0)
   {
      // Trophy has already been unlocked this play session, avoid spamming event queue.
      return;
   }

   // Flag trophy as unlocked
   sTrophiesUnlocked[trophy] = 1;

	// Trophies are available or initializing, add command.
   int ret = sys_event_port_send(sTrophyEventQueuePort, kTC_UnlockTrophy, trophy, 0);
   if (ret != CELL_OK)
   {
      printf("sys_event_port_send error: %d\n", ret);
   }
}

//----------------------------------------------------------------------------

extern "C" int BP_TrophySystem_HDDNeededKB()
{
	return (sTrophyRequiredSpace + 1023) / 1024;
}

//----------------------------------------------------------------------------

extern "C" int BP_TrophySystem_GetError()
{
	return sbTrophyRegisterError;
}

//----------------------------------------------------------------------------

extern "C" int BP_TrophySystem_IsInsufficientHDDSpaceForApp()
{
   return sbInsufficientAppHDDSpace ? 1 : 0;
}

//----------------------------------------------------------------------------

extern "C" void BP_TrophySystem_Disable()
{
	sbTrophiesDisabled = true;
}

//----------------------------------------------------------------------------

extern "C" void BP_TrophySystem_Enable()
{
   sbTrophiesDisabled = false;
}

//----------------------------------------------------------------------------

extern "C" int BP_TrophySystem_IsDisabled()
{
   return sbTrophiesDisabled  ? 1 : 0;
}

//----------------------------------------------------------------------------

extern "C" void BP_TrophySystem_Shutdown()
{
	while (BP_TrophySystem_UpdateInProgress())
	{
		sys_timer_usleep(1000);
	}
	if (!sbTrophiesInitialized)
	{
		return;
	}
	sys_event_port_send(sTrophyEventQueuePort, kTC_Shutdown, 0, 0);
	// Wait for thread to finish
	uint64_t exitCode;
	sys_ppu_thread_join(sTrophyThread, &exitCode);

	// Delete event queue
	sys_event_queue_destroy(sTrophyEventQueue, SYS_EVENT_QUEUE_DESTROY_FORCE);
	sys_event_port_destroy(sTrophyEventQueuePort);
}

//----------------------------------------------------------------------------
