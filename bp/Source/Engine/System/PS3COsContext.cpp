//----------------------------------------------------------------------------
// PS3COsContext.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"

#include BPE_PLATFORM_SPECIFIC( COsContext.h )

#include <stdlib.h>
#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_sysparam.h>
#include <sysutil/sysutil_gamecontent.h>
#include <sysutil/sysutil_savedata.h>
#include <np.h>
#include <np/drm.h>
#include <sys/process.h>
#include <sys/prx.h>
#include <cell/sysmodule.h>
#include <cell/http.h>
#include <cell/atomic.h>
#include <netex/net.h>
#include "Renderer/Base/Frontend/CRenderer.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Backend/PS3/PS3CRenderBackend.h"

//----------------------------------------------------------------------------

//#define ALLOC_ENOUGH_MEMORY_FOR_COMMERCE_AND_BROWSER

size_t const COsContext::skNPInitPoolSize = 128 * 1024;
size_t const COsContext::skHTTPInitPoolSize = 8 * 1024;

#ifdef ALLOC_ENOUGH_MEMORY_FOR_COMMERCE_AND_BROWSER
size_t const COsContext::skMemoryContainerSize = 72 * 1024 * 1024; //SCE_NP_COMMERCE_DO_CHECKOUT_MEMSIZE + 8 meg - big enough to hold anything (hopefully!). Commerce and web browsing works better with the extra 8 megs.
#else
size_t const COsContext::skMemoryContainerSize = 1 * 1024 * 1024;
#endif

static const SceNpDrmKey skDRMKey_Patch_NAJP = {0xb6, 0x08, 0xe7, 0x80, 0x18, 0xa3, 0x35, 0xe4, 0x94, 0xb9, 0x72, 0xa6, 0xb7, 0x17, 0xa1, 0x66};
static const SceNpDrmKey skDRMKey_Patch_EU   = {0xe3, 0xa1, 0x52, 0xef, 0x36, 0x24, 0xcf, 0x89, 0xab, 0xcd, 0xb1, 0x31, 0x7d, 0x57, 0xcc, 0x6b};

SceNpDrmKey const & GetDRMKeyPatch()
{
   switch(OsContext()->mBuildSKU)
   {
   case COsContext::kBS_Europe:
      return skDRMKey_Patch_EU;

   case COsContext::kBS_USA:
   case COsContext::kBS_Japan:
   default:
      return skDRMKey_Patch_NAJP;
   }
}

//------------------------------------------------------------------------------------------------------------------ 

#if MGS_VERSION==2

// NOTE: Only JP SKU Title Ids tested!
static char const skTitleId_JP_PSN[COsContext::kTitleIdLen] = "NPJB00157";
static char const skTitleId_NA_PSN[COsContext::kTitleIdLen] = "NPUB30609";
static char const skTitleId_EU_PSN[COsContext::kTitleIdLen] = "NPED00685";
static char const skTitleId_AP_PSN[COsContext::kTitleIdLen] = "NPHB00403";
static char const skTitleId_KR_PSN[COsContext::kTitleIdLen] = "NPHB00403";   // Intentionally the same as Asia

SceNpDrmKey skDRMKey_PSN = {0xD0, 0xFC, 0xBD, 0x9B, 0xA7, 0x56, 0x21, 0xDF, 0xAA, 0x69, 0xA2, 0x27, 0x0D, 0x1D, 0x51, 0xCF};

#elif MGS_VERSION==3

// NOTE: Only JP SKU Title Ids tested!
static char const skTitleId_JP_PSN[COsContext::kTitleIdLen] = "NPJB00156";
static char const skTitleId_NA_PSN[COsContext::kTitleIdLen] = "NPUB30610";
static char const skTitleId_EU_PSN[COsContext::kTitleIdLen] = "NPED00684";
static char const skTitleId_AP_PSN[COsContext::kTitleIdLen] = "NPHB00403";
static char const skTitleId_KR_PSN[COsContext::kTitleIdLen] = "NPHB00403";   // Intentionally the same as Asia

SceNpDrmKey skDRMKey_PSN = {0x83, 0xF2, 0x0D, 0xAE, 0x9A, 0x83, 0x36, 0xA8, 0xE3, 0x7E, 0x1E, 0x54, 0x03, 0x30, 0xA8, 0xB9};

#endif

//------------------------------------------------------------------------------------------------------------------ 

static void OsContext_InitializeMisc();

extern "C" void BP_TrophySystem_Disable();
extern "C" void BP_EndFrame();
extern "C" void BP_ShutdownCheck();
extern "C" void BP_SetAllDirectOutputStreamPaused( int paused );

//----------------------------------------------------------------------------

COsContext::COsContext()
:  CBaseOsContext()
,  mInstanceHandle(NULL)
,  mMainWindow(NULL)
,  mpNpInitPoolAlloc(NULL)
,  mSysMemoryContainer(SYS_MEMORY_CONTAINER_ID_INVALID)
,  mSysMemoryContainerRefCount(0)
,  mHDDFreeSizeKB(-1)
,  mGameBootType(-1)
,  mGameBootAttributes(0)
{
   mGameContentInfoPath[0] = 0;
   mGameUsrdirPath[0] = 0;
   mPatchContentInfoPath[0] = 0;
   mPatchUsrdirPath[0] = 0;

   // Load required PRX files
   OsContext_InitializeMisc();

   // Initialize networking.
#if 0
   // Network and HTTP disabled as unneeded for GoW
   sys_net_initialize_network();
#endif

   // Globally initialize NP
   mpNpInitPoolAlloc = malloc(skNPInitPoolSize);
   int ret = sceNpInit(skNPInitPoolSize, mpNpInitPoolAlloc);

   // Globally initialize HTTP
#if 0
   mpHTTPInitPoolAlloc = malloc(skHTTPInitPoolSize);
   ret = cellHttpInit(mpHTTPInitPoolAlloc, skHTTPInitPoolSize);
#endif

   cellSaveDataEnableOverlay(1);

   ret = sys_memory_container_create(&mSysMemoryContainer, skMemoryContainerSize);
   BPE_VERIFY(ret == CELL_OK, false, "Couldn't allocate sys memory container in os context.");

   InitializeBuildSKU();
   InitializeLanguage();
}

//----------------------------------------------------------------------------

COsContext::~COsContext()
{
   DestroyMemoryContainer();
   int ret = CELL_OK;
#if 0
   ret = cellHttpEnd();
   free(mpHTTPInitPoolAlloc);
#endif

   ret = sceNpTerm();
   free(mpNpInitPoolAlloc);

#if 0
   sys_net_finalize_network();
#endif
}

//----------------------------------------------------------------------------

std::string CBaseOsContext::GetMachineName()
{
   return "localhost";
}

//----------------------------------------------------------------------------

std::string CBaseOsContext::GetCurrentUserName()
{
   return "ps3_user";
}

//----------------------------------------------------------------------------

void COsContext::InitializeBuildSKU()
{
   // Initialize build SKU and region
   {
      // Perform cell game boot check

      cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_GAME);

      // Need to do this before calling cellGameGetParamString
      unsigned int type = 0;            
      unsigned int attributes = 0;
      CellGameContentSize size;
      char dirName[CELL_GAME_DIRNAME_SIZE];
      int ret = cellGameBootCheck(&type, &attributes, &size, dirName);
      if (ret != CELL_GAME_RET_OK)
      {
         bpe_debugger_printf("cellBootGameCheck failed: (0x%08x)\n", ret); 
      }
      else
      {
         mGameBootAttributes = attributes;
         mGameBootType = type;
         mHDDFreeSizeKB = size.hddFreeSizeKB;
         bpe_debugger_printf("cellGameBootCheck - type: %d - attribs: 0x%08x - hddDir: %s\n", mGameBootType, mGameBootAttributes, dirName ? dirName : "NULL");
         if (mGameBootType == CELL_GAME_GAMETYPE_HDD)
         {
            mBootType = kBT_HDD;
         }
         else
         {
            mBootType = kBT_Disc;
         }
      }
   }
   
   {
      // Get title id from PARAM.SFO
      char tempBuf[kTitleIdLen]; 
      int ret = cellGameGetParamString(CELL_GAME_PARAMID_TITLE_ID, tempBuf, sizeof(tempBuf)); 
      if (ret < 0) 
      { 
         bpe_debugger_printf("cellGameGetParamString failed: (0x%08x)\n", ret); 
      } 
      else 
      { 
         strcpy(mTitleId, tempBuf); 
         bpe_debugger_printf("Title Id %s\n", mTitleId); 
      } 
   }

   {
      // Finish game content processing (not patch)
      // We don't actually need these paths, we just need to call the function to 
      // notify the system that we're finished with the game content 'utility'
      int ret = cellGameContentPermit(mGameContentInfoPath, mGameUsrdirPath);
      if (ret != CELL_GAME_RET_OK)
      {
         bpe_debugger_printf("cellGameContentPermit failed: (0x%08x)\n", ret); 
      }

   }

   // Are we booted from a patch?
   if ((mGameBootAttributes & CELL_GAME_ATTRIBUTE_PATCH) != 0)
   {
      BPE_VERIFY(mBootType == COsContext::kBT_Disc, false, "No patch flag on HDD game as per SDK");
      mBootType = kBT_DiscPatch;
      bpe_debugger_printf("*** Patch Boot! ***\n"); 

      // Get information about patch boot
      CellGameContentSize size;
      int ret = cellGamePatchCheck(&size, NULL);
      if (ret != CELL_GAME_RET_OK)
      {
         bpe_debugger_printf("Patch: cellGamePatchCheck failed: (0x%08x)\n", ret); 
      }
      else
      {
         int ret = cellGameContentPermit(mPatchContentInfoPath, mPatchUsrdirPath);
         if (ret != CELL_GAME_RET_OK)
         {
            bpe_debugger_printf("Patch: cellGameContentPermit failed: (0x%08x)\n", ret); 
         }
         else
         {
            bpe_debugger_printf("*** Patch mPatchContentInfoPath: %s\n", mPatchContentInfoPath);
            bpe_debugger_printf("*** Patch mPatchUsrdirPath: %s\n", mPatchUsrdirPath);
         }
      }
   }

   // Don't need game content util anymore, unload
   cellSysmoduleUnloadModule(CELL_SYSMODULE_SYSUTIL_GAME);

   mControllerRegion = kCR_USA;

   if ((strcmp(mTitleId, skTitleId_EU) == 0) ||
       (strcmp(mTitleId, skTitleId_EU_PSN) == 0))
   {
      mBuildSKU = kBS_Europe;
      mSystemRegion = kR_Europe;
      strcpy( mTitleId_SaveData, skTitleId_EU_SaveData );
   }
   else if ((strcmp(mTitleId, skTitleId_NA) == 0) ||
            (strcmp(mTitleId, skTitleId_NA_PSN) == 0))
   {
      mBuildSKU = kBS_USA;
      mSystemRegion = kR_USA;
      strcpy( mTitleId_SaveData, skTitleId_NA_SaveData );
   }
   else if ((strcmp(mTitleId, skTitleId_JP) == 0) ||
            (strcmp(mTitleId, skTitleId_JP_PSN) == 0))
   {
      mBuildSKU = kBS_Japan;
      mSystemRegion = kR_Japan;
      strcpy( mTitleId_SaveData, skTitleId_JP_SaveData );
      mControllerRegion = kCR_Japan;
   }
   else if ((strcmp(mTitleId, skTitleId_AP) == 0) ||
            (strcmp(mTitleId, skTitleId_AP_PSN) == 0))

   {
      mBuildSKU = kBS_USA;
      mSystemRegion = kR_USA;
      strcpy( mTitleId_SaveData, skTitleId_AP_SaveData );
   }
   else if ((strcmp(mTitleId, skTitleId_KR) == 0) ||
            (strcmp(mTitleId, skTitleId_KR_PSN) == 0))

   {
      mBuildSKU = kBS_USA;
      mSystemRegion = kR_USA;
      strcpy( mTitleId_SaveData, skTitleId_KR_SaveData );
   }
   else
   {
      // Unknown title id
      bpe_debugger_printf("Unknown TITLE_ID: %s\nReverting to USA.\n", mTitleId);
      mBuildSKU = kBS_USA;
      mSystemRegion = kR_USA;
      strcpy( mTitleId_SaveData, skTitleId_NA_SaveData );
   }

   // Initialize "Is downloadable version" flag
   if( strcmp(mTitleId, skTitleId_EU_PSN) == 0 ||
       strcmp(mTitleId, skTitleId_NA_PSN) == 0 ||
       strcmp(mTitleId, skTitleId_JP_PSN) == 0 ||
       strcmp(mTitleId, skTitleId_AP_PSN) == 0 ||
       strcmp(mTitleId, skTitleId_KR_PSN) == 0 )
   {
      mIsDownloadableVersion = true;
   }
   else
   {
      mIsDownloadableVersion = false;
   }
}

//----------------------------------------------------------------------------

void COsContext::InitializeLanguage()
{
   int value = 0;
   if (cellSysutilGetSystemParamInt(CELL_SYSUTIL_SYSTEMPARAM_ID_LANG, &value) == 0)
   {
      if (value == CELL_SYSUTIL_LANG_JAPANESE)
      {
         mLanguage = kL_Japanese;         
      }
      else if (value == CELL_SYSUTIL_LANG_FRENCH)
      {
         mLanguage = kL_French;         
      }
      else if (value == CELL_SYSUTIL_LANG_GERMAN)
      {
         mLanguage = kL_German;         
      }
      else if (value == CELL_SYSUTIL_LANG_ITALIAN)
      {
         mLanguage = kL_Italian;         
      }
      else if (value == CELL_SYSUTIL_LANG_PORTUGUESE)
      {
         mLanguage = kL_Portuguese;         
      }
      else if (value == CELL_SYSUTIL_LANG_SPANISH)
      {
         mLanguage = kL_Spanish;         
      }
      else if (value == CELL_SYSUTIL_LANG_DUTCH)
      {
         mLanguage = kL_Dutch;         
      }   
      else if (value == CELL_SYSUTIL_LANG_RUSSIAN)
      {
         mLanguage = kL_Russian;
      }
      else
      {
         // Use English by default
         mLanguage = kL_English;               
      }      
   }
   else
   {
      // Error, use English
      mLanguage = kL_English;         
   }

   ForceSpecificLanguageBasedOnBuildSKU();

   return;   
}

//----------------------------------------------------------------------------

sys_memory_container_t COsContext::LockSysMemoryContainer(size_t const size)
{
   BPE_VERIFY(mSysMemoryContainer != SYS_MEMORY_CONTAINER_ID_INVALID, false, "Sys memory container not initialized.");
   BPE_VERIFY(size <= skMemoryContainerSize, false, "Lock size too large.");
   mSysMemoryContainerRefCount++;
   BPE_VERIFY(mSysMemoryContainerRefCount == 1, false, "Double lock on mSysMemoryContainer.");
   return mSysMemoryContainer;
}

//----------------------------------------------------------------------------

void COsContext::UnlockSysMemoryContainer()
{
   mSysMemoryContainerRefCount--;
   BPE_VERIFY(mSysMemoryContainerRefCount == 0, false, "Invalid unlock on mSysMemoryContainer.");
}

//----------------------------------------------------------------------------

size_t COsContext::GetSysMemoryContainerMaxSize() const
{
   return skMemoryContainerSize;
}

//----------------------------------------------------------------------------

void COsContext::DestroyMemoryContainer()
{
   int ret = sys_memory_container_destroy(mSysMemoryContainer);
   mSysMemoryContainer = SYS_MEMORY_CONTAINER_ID_INVALID;
}

//----------------------------------------------------------------------------
// This is required as part of the np reconnect process.
void COsContext::ReinitializeNp()
{
   int ret = sceNpTerm();
   ret = sceNpInit(skNPInitPoolSize, mpNpInitPoolAlloc);
}

//----------------------------------------------------------------------------

uint8 * COsContext::AllocateTempMemory(int32 const size)
{
   if (mSysMemoryContainerRefCount != 0)
   {
      // Memory container already in use
      return NULL;
   }

   size_t allocSize = (size + 0xFFFFF) & 0xFFF00000;  // 1 meg pages

   sys_addr_t pMem = NULL;
   int res =  sys_memory_allocate_from_container(allocSize, mSysMemoryContainer, SYS_MEMORY_PAGE_SIZE_1M, &pMem);
   if (res == CELL_OK)
   {
      mSysMemoryContainerRefCount++;
      return (uint8 *) pMem;
   }

   return NULL;
}

//----------------------------------------------------------------------------

void COsContext::DeallocateTempMemory(uint8 * pMemory)
{
   mSysMemoryContainerRefCount--;
   BPE_VERIFY(mSysMemoryContainerRefCount == 0, false, "Invalid DeallocateTempMemory on mSysMemoryContainer.");
   sys_memory_free((sys_addr_t) pMemory);
}

//----------------------------------------------------------------------------

void COsContext::_Exitspawn(char const * const pRelativePath, char *argv[]) const
{
   // Are we running a patch boot?
   if (mBootType == kBT_DiscPatch)
   {
      // Launch SELF from Patch USRDIR
      char bootPath[128];
      sprintf(bootPath, "%s/%s", mPatchUsrdirPath, pRelativePath);
      bpe_debugger_printf("sceNpDrmProcessExitSpawn2: %s (ARGV: 0x%08x)\n", bootPath, argv);
      int const kMainThreadPriority = 1001;
      int ret = sceNpDrmProcessExitSpawn2(&GetDRMKeyPatch(),
                                          bootPath,
                                          (char const **) argv,                                          
                                          0,
                                          0,
                                          0,
                                          kMainThreadPriority,
                                          SYS_PROCESS_PRIMARY_STACK_SIZE_32K | SCE_NP_DRM_EXITSPAWN2_EXIT_WO_FINI); 

      // Shouldn't get here!
      bpe_debugger_printf("ERROR: sceNpDrmProcessExitSpawn2 (0x%08x)\n", ret);      
   }
   // Are we running a HDD boot?
   else if (mBootType == kBT_HDD)
   {
      // Launch SELF from Patch USRDIR
      char bootPath[128];
      sprintf(bootPath, "%s/%s", mGameUsrdirPath, pRelativePath);
      bpe_debugger_printf("sceNpDrmProcessExitSpawn2: %s (ARGV: 0x%08x)\n", bootPath, argv);
      int const kMainThreadPriority = 1001;
      int ret = sceNpDrmProcessExitSpawn2(&skDRMKey_PSN,
         bootPath,
         (char const **) argv,                                          
         0,
         0,
         0,
         kMainThreadPriority,
         SYS_PROCESS_PRIMARY_STACK_SIZE_32K | SCE_NP_DRM_EXITSPAWN2_EXIT_WO_FINI); 

      // Shouldn't get here!
      bpe_debugger_printf("ERROR: sceNpDrmProcessExitSpawn2 (0x%08x)\n", ret);      
   }
   else
   {
      // Launch SELF from Game USRDIR

      char bootPath[128];
      sprintf(bootPath, "%s/%s", mGameUsrdirPath, pRelativePath);
      bpe_debugger_printf("_Exitspawn: %s (ARGV: 0x%08x)\n", bootPath, argv);
      int const kMainThreadPriority = 1001;
      ::_Exitspawn(bootPath,
                   argv,                                          
                   0,
                   0,
                   0,
                   kMainThreadPriority,
                   SYS_PROCESS_PRIMARY_STACK_SIZE_32K);


      // Shouldn't get here!
      bpe_debugger_printf("ERROR: _Exitspawn failed!\n");      
   }

}

//----------------------------------------------------------------------------

int COsContext::sys_prx_load_module(char const * const pRelativePath) const
{
   // If we're running a patch, we have to use a different path for the SPRX
   // Patch SPRX need to have check to see if it has a valid license.
   char sprxPath[128] = {0};

   // Are we running a patch boot?
   if (mBootType == kBT_DiscPatch)
   {
      sprintf(sprxPath, "%s/%s", mPatchUsrdirPath, pRelativePath);
      bpe_debugger_printf("Opening patch SPRX with sceNpDrmIsAvailable: %s\n", sprxPath);

      // Validate SPRX license.
      // NOTE: This call is 'blocking' but multithreaded renderer will kick in and continue flipping
      int ret = sceNpDrmIsAvailable(&GetDRMKeyPatch(), sprxPath);
      if (ret != CELL_OK)
      {
         // Failed to authorize SPRX, return failure
         bpe_debugger_printf("ERROR: sceNpDrmIsAvailable result: (0x%08x)\n", ret);
         return -1;
      }
   }
   // Are we running a HDD boot?
   else if (mBootType == kBT_HDD)
   {
      sprintf(sprxPath, "%s/%s", mGameUsrdirPath, pRelativePath);
      bpe_debugger_printf("Opening patch SPRX with sceNpDrmIsAvailable: %s\n", sprxPath);

      // Validate SPRX license.
      // NOTE: This call is 'blocking' but multithreaded renderer will kick in and continue flipping
      int ret = sceNpDrmIsAvailable(&skDRMKey_PSN, sprxPath);
      if (ret != CELL_OK)
      {
         // Failed to authorize SPRX, return failure
         bpe_debugger_printf("ERROR: sceNpDrmIsAvailable result: (0x%08x)\n", ret);
         return -1;
      }
   }
   else
   {
      // Use default path for SPRX
      sprintf(sprxPath, "%s/%s", mGameUsrdirPath, pRelativePath);
      bpe_debugger_printf("Opening default SPRX: %s\n", sprxPath);
   }

   // Now load SPRX 
   sys_prx_id_t const sprxId = ::sys_prx_load_module(sprxPath, 0, NULL);
   return (int) sprxId;
}

//----------------------------------------------------------------------------

CBaseOsContext::EBootType CBaseOsContext::GetBootType() const
{
   return mBootType;
}


//----------------------------------------------------------------------------

void CBaseOsContext::TerminateProcess()
{
   // Force verify, we don't support 'forced' terminate process
   BPE_VERIFYA(false, "CBaseOsContext::TerminateProcess().");
}


//----------------------------------------------------------------------------
// Sysutil callback and other system code

#include <sysutil/sysutil_msgdialog.h>
#include <sysutil/sysutil_gamecontent.h>
#include <cell/pad/libpad.h>
#include <sys/ppu_thread.h>
#include <sys/timer.h>

//----------------------------------------------------------------------------

static bool                         sIsReleaseMode = false;
uint64_t                            gPS3LastSystemCallbackStatus = -1;  // Used to check when dialogs are completed drawing

//----------------------------------------------------------------------------
// Called from PS3 system, used to detect message display and game quit
static void systemCallback(const uint64_t status, const uint64_t param, void *userdata)
{
   (void)param;
   (void)userdata;
   bpe_debugger_printf("system notification: %d\n", status);
   switch (status)
   {
   case CELL_SYSUTIL_REQUEST_EXITGAME:
      gpOsContext->mShouldTerminateApplication = true;      
      gpOsContext->mShutdownType = COsContext::kST_StandardShutdown;
      gPS3LastSystemCallbackStatus = status;
      break;

   case CELL_SYSUTIL_DRAWING_BEGIN:
      // Pause taken care of in TryIdle_PlatformSpecific
      //gpOsContext->mShouldPauseApplication = true;      
      gPS3LastSystemCallbackStatus = status;
      break;

   case CELL_SYSUTIL_DRAWING_END:
      //gpOsContext->mShouldPauseApplication = false;      
      gPS3LastSystemCallbackStatus = status;
      break;

   case CELL_SYSUTIL_NP_INVITATION_SELECTED:
      // We're received an in-game invite for PW
      bpe_debugger_printf("CELL_SYSUTIL_NP_INVITATION_SELECTED received.\n");
      gpOsContext->mShouldTerminateApplication = true;      
      gpOsContext->mShutdownType = COsContext::kST_LaunchPWForInvite;
      gPS3LastSystemCallbackStatus = status;
      break;

#if 0 // Not needed for GoW
   case CELL_SYSUTIL_NET_CTL_NETSTART_LOADED:
      gCNetManager.onSysUtilStateReceived(status);
      break;

   case CELL_SYSUTIL_NET_CTL_NETSTART_FINISHED:
      gCNetManager.onSysUtilStateReceived(status);
      break;

   case CELL_SYSUTIL_NET_CTL_NETSTART_UNLOADED:
      gCNetManager.onSysUtilStateReceived(status);
      break;
#endif
   default:  
      break;
   }
}

//----------------------------------------------------------------------------

static void load_ps3_prx()
{
    // Load PRX's
   int ret;
   /* Load the libfiber PRX. */
   ret = cellSysmoduleLoadModule(CELL_SYSMODULE_FIBER);
   ret = cellSysmoduleLoadModule(CELL_SYSMODULE_AUDIO);
   ret = cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
   ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SPURS);
   ret = cellSysmoduleLoadModule(CELL_SYSMODULE_GCM_SYS);
   ret = cellSysmoduleLoadModule(CELL_SYSMODULE_IO);
   ret = cellSysmoduleLoadModule(CELL_SYSMODULE_RTC);
   ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP);
   ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_NP_TROPHY);
   ret = cellSysmoduleLoadModule(CELL_SYSMODULE_VDEC_MPEG2);   
   ret = cellSysmoduleLoadModule(CELL_SYSMODULE_AVCONF_EXT);  // for cellVideoOutGetScreenSize()
#ifndef GOLD_VERSION
   ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_SCREENSHOT);
#endif
   ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_SAVEDATA);
}

//----------------------------------------------------------------------------

static void OsContext_InitializeMisc()
{
   // Register a callback for game quit
   int result = cellSysutilRegisterCallback(0, systemCallback, NULL);
   // Load PRX files
   load_ps3_prx();
}

//----------------------------------------------------------------------------

static int sCellGameContentErrorDialog_Type = 0;
static int sCellGameContentErrorDialog_errNeedSizeKB = 0;
static void ps3_cellGameContentErrorDialog_thread(uint64_t arg)
{
   int ret;
   ret = cellGameContentErrorDialog(sCellGameContentErrorDialog_Type, sCellGameContentErrorDialog_errNeedSizeKB, NULL);  

   sys_ppu_thread_exit(ret);
}

// Show error message dialog on new thread (prevent blocking), can only be called once and assumes termination dialog.
// Assumes application will terminate after calling this function.
static void ps3_cellGameContentErrorDialog(int type, int errNeedSizeKB)
{
   bpe_debugger_printf("ps3_cellGameContentErrorDialog: %d needSize: %d\n", type, errNeedSizeKB);
   static bool checkReentrance = false;
   BPE_VERIFY(checkReentrance == false, false, "ps3_cellGameContentErrorDialog can only be called once.");

   int ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_GAME);

	sCellGameContentErrorDialog_Type = type;
   sCellGameContentErrorDialog_errNeedSizeKB = errNeedSizeKB;
   sys_ppu_thread_t tid;
	
	ret = sys_ppu_thread_create(&tid,
		ps3_cellGameContentErrorDialog_thread, NULL,
		1001, 16 * 1024,
		SYS_PPU_THREAD_CREATE_JOINABLE,
      "gameContentErrorDialog");

   gpRenderBackend->SuspendGameRenderThread();

   // Wait for termination event.
   while (!gpOsContext->mShouldTerminateApplication)
   {
      BP_EndFrame(); //allow sound system to update and handle gpOsContext->ProcessPlatform().
      BP_ShutdownCheck();
   }

   gpRenderBackend->ResumeGameRenderThread();

   // Wait for error dialog thread to terminate
   uint64_t exit_status;
   sys_ppu_thread_join(tid, &exit_status);
}

//----------------------------------------------------------------------------

static volatile bool sNonFatalErrorThreadComplete = false;
static void ps3_cellGameContentErrorDialogNonFatal_thread(uint64_t arg)
{
   int ret;
   ret = cellGameContentErrorDialog(sCellGameContentErrorDialog_Type, sCellGameContentErrorDialog_errNeedSizeKB, NULL);  

#ifdef MGS_VERSION
   if( ret == 0 && sCellGameContentErrorDialog_Type == CELL_GAME_ERRDIALOG_NOSPACE)
   {
      //Piggyback onto this thread a "delete from list" so the user can free up some space.
      ret = cellSaveDataDelete2(SYS_MEMORY_CONTAINER_ID_INVALID);
   }
#endif

   sNonFatalErrorThreadComplete = true;
   sys_ppu_thread_exit(ret);
}

// Show error message dialog on new thread (prevent blocking), can only be called once and assumes termination dialog.
// Assumes application will terminate after calling this function.
static void ps3_cellGameContentErrorDialogNonFatal(int type, int errNeedSizeKB)
{
   BP_SetAllDirectOutputStreamPaused( 1 );
   bpe_debugger_printf("ps3_cellGameContentErrorDialog: %d needSize: %d\n", type, errNeedSizeKB);
   static bool checkReentrance = false;
   BPE_VERIFY(checkReentrance == false, false, "ps3_cellGameContentErrorDialog can only be called once.");

   int ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_GAME);

	sCellGameContentErrorDialog_Type = type;
   sCellGameContentErrorDialog_errNeedSizeKB = errNeedSizeKB;
   sys_ppu_thread_t tid;
	
   sNonFatalErrorThreadComplete = false;	
	ret = sys_ppu_thread_create(&tid,
		ps3_cellGameContentErrorDialogNonFatal_thread, NULL,
		1001, 16 * 1024,
		SYS_PPU_THREAD_CREATE_JOINABLE,
      "gameContentErrorDialog");

   gpRenderBackend->SuspendGameRenderThread();

   // Wait for end of non-fatal error thread
   while (!sNonFatalErrorThreadComplete)
   {
      BP_EndFrame(); //allow sound system to update and handle gpOsContext->ProcessPlatform().
      BP_ShutdownCheck();
   }

   gpRenderBackend->ResumeGameRenderThread();

   // Wait for error dialog thread to terminate
   uint64_t exit_status;
   sys_ppu_thread_join(tid, &exit_status);
   BP_SetAllDirectOutputStreamPaused( 0 );
}

//----------------------------------------------------------------------------

void COsContext::TerminateLowDiskSpace(int needSizeKB)
{
   ps3_cellGameContentErrorDialog(CELL_GAME_ERRDIALOG_NOSPACE_EXIT, needSizeKB);
}

//----------------------------------------------------------------------------

void COsContext::TerminateLicenseCheckFailed()
{
   ps3_cellGameContentErrorDialog(CELL_GAME_ERRDIALOG_BROKEN_EXIT_HDDGAME, 0);
}

//----------------------------------------------------------------------------

enum ESaveGameMessage
{
   kSGM_WrongUser,
   kSGM_WrongUserPresent,  //special case for MGS2 trophies based on wrong user but not loaded
   kSGM_CorruptSave,
   kSGM_CorruptSaveFile,

   kSGM_Count
};


static char const * skSGM_WrongUserString_E = "This save data was created by another user.\nGame progress cannot be saved nor trophies obtained using this data.";
static char const * skSGM_WrongUserString_F = "Cette sauvegarde a été créée par un autre utilisateur.\nImpossible de sauvegarder la progression ni de remporter des trophées avec cette sauvegarde.";
static char const * skSGM_WrongUserString_I = "Questi dati salvati sono stati creati da un altro utente.\nUtilizzando questi dati salvati non sarà possibile né salvare i progressi di gioco né ottenere trofei.";
static char const * skSGM_WrongUserString_G = "Speicherdaten wurden von anderem Benutzer erstellt.\nMit diesen Daten können weder Spielfortschritte gespeichert noch Trophäen erlangt werden.";
static char const * skSGM_WrongUserString_S = "Estos datos guardados pertenecen a otro usuario.\nNo se pueden guardar los progresos del juego ni obtener trofeos con estos datos.";
static char const * skSGM_WrongUserString_J = "他ユーザーが作成したセーブデータがロードされました。\nこのままゲームを続けてもトロフィーを獲得できません。\nまた、進行状況をセーブすることができません。";

static char const * skSGM_WrongUserPresentString_E = "This game section was unlocked by save data created by another user.\nTrophies cannot be obtained using this data.";
static char const * skSGM_WrongUserPresentString_F = "Cette partie du jeu a été débloquée par des données de sauvegarde\ncréées par un autre joueur.\nLes trophées ne sont pas accessibles avec ces données.";
static char const * skSGM_WrongUserPresentString_I = "Questa sezione del gioco è stata sbloccata con i dati salvati\ncreati da un altro utente.\nUsando questi dati salvati è impossibile ottenere i trofei.";
static char const * skSGM_WrongUserPresentString_G = "Dieser Spielabschnitt wurde mit Speicherdaten eines\nanderen Nutzers freigeschaltet.\nMit diesen Daten können Sie keine Trophäen erhalten.";
static char const * skSGM_WrongUserPresentString_S = "Sección desbloqueada por datos guardados creados por otro usuario.\nNo es posible obtener trofeos con estos datos.";
static char const * skSGM_WrongUserPresentString_J = "このゲームセクションは他のユーザーが作成したセーブデータに\nよって解除されています。\nこのデータを使用した場合、トロフィーは獲得できません。";

static char const * skSGM_CorruptSaveString_E = "Load failed due to corrupted save data.\nAttempt load of save data again?";
static char const * skSGM_CorruptSaveString_F = "Échec du chargement. Sauvegarde endommagée.\nRéessayer de charger ?";
static char const * skSGM_CorruptSaveString_I = "Caricamento fallito. I dati salvati sono danneggiati.\nCaricare di nuovo i dati salvati?";
static char const * skSGM_CorruptSaveString_G = "Ladevorgang wegen beschädigter Speicherdaten gescheitert. Erneut versuchen?";
static char const * skSGM_CorruptSaveString_S = "No se pueden cargar los datos porque están dañados.\n¿Quieres intentar cargar los datos de nuevo?";
static char const * skSGM_CorruptSaveString_J = "セーブデータが壊れているため、読み込みに失敗しました。\nセーブデータの読み込みをリトライしますか？";
static char const * skSGM_CorruptSaveStringFile_E = "Unable to load.\nThis file is corrupted.";
static char const * skSGM_CorruptSaveStringFile_F = "Impossible de charger.\nCe fichier est endommagé.";
static char const * skSGM_CorruptSaveStringFile_I = "Caricamento fallito.\nIl file è danneggiato.";
static char const * skSGM_CorruptSaveStringFile_G = "Ladevorgang nicht möglich.\nDiese Datei ist beschädigt.";
static char const * skSGM_CorruptSaveStringFile_S = "No se pueden cargar los datos.\nEl archivo está dañado.";
static char const * skSGM_CorruptSaveStringFile_J = "ロードに失敗しました。\nこのファイルは壊れています。";

static char const * skSGM_Unknown = "Unknown";

static char const * const _get_save_game_message_localized_string(ESaveGameMessage const string)
{
   switch (string)
   {
   case kSGM_WrongUser:
      switch (gpOsContext->mLanguage)
      {
      case CBaseOsContext::kL_Japanese:
         return skSGM_WrongUserString_J;

      case CBaseOsContext::kL_French:
         return skSGM_WrongUserString_F;

      case CBaseOsContext::kL_Italian:
         return skSGM_WrongUserString_I;

      case CBaseOsContext::kL_German:
         return skSGM_WrongUserString_G;

      case CBaseOsContext::kL_Spanish:
         return skSGM_WrongUserString_S;

      default: // Default to English
         return skSGM_WrongUserString_E;
      }
      break;

   case kSGM_WrongUserPresent:
      switch (gpOsContext->mLanguage)
      {
      case CBaseOsContext::kL_Japanese:
         return skSGM_WrongUserPresentString_J;

      case CBaseOsContext::kL_French:
         return skSGM_WrongUserPresentString_F;

      case CBaseOsContext::kL_Italian:
         return skSGM_WrongUserPresentString_I;

      case CBaseOsContext::kL_German:
         return skSGM_WrongUserPresentString_G;

      case CBaseOsContext::kL_Spanish:
         return skSGM_WrongUserPresentString_S;

      default: // Default to English
         return skSGM_WrongUserPresentString_E;
      }
      break;

   case kSGM_CorruptSave:
      switch (gpOsContext->mLanguage)
      {
      case CBaseOsContext::kL_Japanese:
         return skSGM_CorruptSaveString_J;

      case CBaseOsContext::kL_French:
         return skSGM_CorruptSaveString_F;

      case CBaseOsContext::kL_Italian:
         return skSGM_CorruptSaveString_I;

      case CBaseOsContext::kL_German:
         return skSGM_CorruptSaveString_G;

      case CBaseOsContext::kL_Spanish:
         return skSGM_CorruptSaveString_S;

      default: // Default to English
         return skSGM_CorruptSaveString_E;
      }
      break;

   case kSGM_CorruptSaveFile:
      switch (gpOsContext->mLanguage)
      {
      case CBaseOsContext::kL_Japanese:
         return skSGM_CorruptSaveStringFile_J;

      case CBaseOsContext::kL_French:
         return skSGM_CorruptSaveStringFile_F;

      case CBaseOsContext::kL_Italian:
         return skSGM_CorruptSaveStringFile_I;

      case CBaseOsContext::kL_German:
         return skSGM_CorruptSaveStringFile_G;

      case CBaseOsContext::kL_Spanish:
         return skSGM_CorruptSaveStringFile_S;

      default: // Default to English
         return skSGM_CorruptSaveStringFile_E;
      }
      break;
   }
   return skSGM_Unknown;
}


//----------------------------------------------------------------------------

static const char* sCellGameContentDialog_Msg = NULL;
static int* sCellGameContentDialog_Return = NULL;

static void ps3_cellGameContentDialog_Result(int buttonType, void *userData)
{
   if (sCellGameContentDialog_Return)
   {
	   switch( buttonType ) {
	   case CELL_MSGDIALOG_BUTTON_YES:
		   // Processing for positive response ("Yes")
         *sCellGameContentDialog_Return = 1;
		   break;
	   case CELL_MSGDIALOG_BUTTON_NO:
		   // Processing for negative response ("No")
         *sCellGameContentDialog_Return = 0;
		   break;
	   }
   }
}

enum DialogType
{
   DIALOG_TYPE_OK,
   DIALOG_TYPE_YES_NO
};

static void ps3_cellGameContentDialog_thread(uint64_t arg)
{
   if (sCellGameContentDialog_Return)
      *sCellGameContentDialog_Return = -1;

   static const unsigned int  s_msgType[] =
   {
   //DIALOG_TYPE_OK,
      CELL_MSGDIALOG_TYPE_SE_TYPE_NORMAL | CELL_MSGDIALOG_TYPE_BUTTON_TYPE_OK | CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_ON,

   //DIALOG_TYPE_YES_NO
      CELL_MSGDIALOG_TYPE_SE_TYPE_NORMAL | CELL_MSGDIALOG_TYPE_BUTTON_TYPE_YESNO | CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_ON,
   };

   int ret = cellMsgDialogOpen2(
	   s_msgType[arg],
	   sCellGameContentDialog_Msg,
	   ps3_cellGameContentDialog_Result,
	   NULL,
	   NULL);

   sys_ppu_thread_exit(0);
}

// Show error message dialog on new thread (prevent blocking), can only be called once and assumes termination dialog.
// Assumes application will terminate after calling this function.
static void ps3_cellGameContentDialog(const char *msg, int *pResult, DialogType dialogType)
{
   bpe_debugger_printf("ps3_cellGameContentDialog: msg: %s\n", msg);
   static bool checkReentrance = false;
   BPE_VERIFY(checkReentrance == false, false, "ps3_cellGameContentDialog can only be called once.");

   int ret = cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_GAME);

   sCellGameContentDialog_Msg = msg;
   sCellGameContentDialog_Return = pResult;
   sys_ppu_thread_t tid;
	
	ret = sys_ppu_thread_create(&tid,
		ps3_cellGameContentDialog_thread, dialogType,
		1001, 16 * 1024,
		0,
      "gameContentDialog");
}

extern "C" void ShowWrongUserWarning(int *pResult)
{
   //Pause all streams.
   BP_SetAllDirectOutputStreamPaused( 1 );
   // moved the string contents into Game project so it can be switched based on region w/o an ugly header dependency
   volatile int threadResult = -1;  // wait until this changes to proceed
   ps3_cellGameContentDialog(_get_save_game_message_localized_string(kSGM_WrongUser), (int *)&threadResult, DIALOG_TYPE_OK);

   gpRenderBackend->SuspendGameRenderThread();

   while (threadResult==-1)
   {
      BP_EndFrame(); //allow sound system to update and handle gpOsContext->ProcessPlatform().
      BP_ShutdownCheck();
   }

   gpRenderBackend->ResumeGameRenderThread();

   // Prevent wrong user from unlocking trophies
   BP_TrophySystem_Disable();
   
   BP_SetAllDirectOutputStreamPaused( 0 );
   *pResult = threadResult;
}

extern "C" void ShowWrongUserWarningPresent(int *pResult)
{
   BP_SetAllDirectOutputStreamPaused( 1 );
   // moved the string contents into Game project so it can be switched based on region w/o an ugly header dependency
   volatile int threadResult = -1;  // wait until this changes to proceed
   ps3_cellGameContentDialog(_get_save_game_message_localized_string(kSGM_WrongUserPresent), (int *)&threadResult, DIALOG_TYPE_OK);

   gpRenderBackend->SuspendGameRenderThread();

   while (threadResult==-1)
   {
      BP_EndFrame(); //allow sound system to update and handle gpOsContext->ProcessPlatform().
      BP_ShutdownCheck();
   }

   gpRenderBackend->ResumeGameRenderThread();

   BP_SetAllDirectOutputStreamPaused( 0 );
   *pResult = threadResult;
}

extern "C" void ShowLowDiskSpaceWarning(int *pResult, int kbNeeded)
{
   // PS3 version - use the system message to stop the game
   if (pResult)
   {
      ps3_cellGameContentErrorDialogNonFatal(CELL_GAME_ERRDIALOG_NOSPACE, kbNeeded);   
      *pResult = 0;
   }
   else
   {
      OsContext()->TerminateLowDiskSpace(kbNeeded);  // this NEVER returns.  Game always exits.
   }
}

extern "C" void ShowCorruptSaveWarning(int *pResult)  // MASTER.BIN is corrupt
{
   BP_SetAllDirectOutputStreamPaused( 1 );
   // moved the string contents into Game project so it can be switched based on region w/o an ugly header dependency
   volatile int threadResult = -1;  // wait until this changes to proceed
   ps3_cellGameContentDialog(_get_save_game_message_localized_string(kSGM_CorruptSave), (int *)&threadResult, DIALOG_TYPE_YES_NO);

   gpRenderBackend->SuspendGameRenderThread();

   while (threadResult==-1)
   {
      BP_EndFrame(); //allow sound system to update and handle gpOsContext->ProcessPlatform().
      BP_ShutdownCheck();
   }

   gpRenderBackend->ResumeGameRenderThread();

   BP_SetAllDirectOutputStreamPaused( 0 );
   *pResult = threadResult;   
}

//----------------------------------------------------------------------------

void COsContext::ProcessPlatform()
{
   // Check for callbacks
   cellSysutilCheckCallback();

   // Check for exclusive system access to controller
   CellPadInfo2 padInfo;
   
   if (cellPadGetInfo2(&padInfo) != 0)
   {
      bpe_debugger_printf("Error : cellPadGetPadInfo - PS3Main.cpp\n");
   }
   else
   {
      if ((padInfo.system_info & CELL_PAD_INFO_INTERCEPTED) != 0)
      {
         // Freeze game
         gpOsContext->mShouldPauseApplication = true;      
      }
      else
      {
         // Unfreeze game
         gpOsContext->mShouldPauseApplication = false;      
      }
   }

}

void CBaseOsContext::SetUnsafeToShutDownFlag( uint32 const flag )
{
   cellAtomicOr32( (uint32_t*)&mUnsafeToShutDownFlags, flag );
}

void CBaseOsContext::ClearUnsafeToShutDownFlag( uint32 const flag )
{
   cellAtomicAnd32( (uint32_t*)&mUnsafeToShutDownFlags, ~flag );
}

bool CBaseOsContext::IsSafeToShutDown() const
{
   uint32_t safe = cellAtomicNop32( &mUnsafeToShutDownFlags );
   return safe == 0;
}

//----------------------------------------------------------------------------

CBaseOsContext::EProfileStatus CBaseOsContext::GetUserProfile()
{
   return kPS_Valid;
}

//----------------------------------------------------------------------------
