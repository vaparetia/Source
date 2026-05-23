//----------------------------------------------------------------------------
// VTACOsContext.cpp
// Copyright 2011
// Bluepoint/Armature
//----------------------------------------------------------------------------

#include "StdAfx.h"

#include BPE_PLATFORM_SPECIFIC( COsContext.h )

#include <stdlib.h>

#include <libsysmodule.h>
#include <libdbg.h>
#include <libnetctl.h>
#include <libssl.h>
#include <libhttp.h>
#include <sce_atomic.h>
#include <power.h>
#include <net.h>
#include <netcheck_dialog.h>
#include <common_dialog.h>
#include <apputil.h>
#include <sdk_version.h>

unsigned int sceLibcHeapExtendedAlloc = 1;  /* Switch to dynamic allocation */
unsigned int sceLibcHeapSize = SCE_LIBC_HEAP_SIZE_EXTENDED_ALLOC_NO_LIMIT; /* no upper limit for heap area */

extern "C"
{
   extern char const *gkBP_BluepointAssets_Vita_Archive_Filename_Base;
}

namespace
{
   char const *sRootFolder = NULL;
   char sDearchiverWorkBuffer[SCE_FIOS_PSARC_DEARCHIVER_WORK_BUFFER_SIZE] __attribute__((aligned(SCE_FIOS_PSARC_DEARCHIVER_WORK_BUFFER_ALIGNMENT)));

}

//----------------------------------------------------------------------------

static void load_vita_prx()
{
   sceSysmoduleLoadModule( SCE_SYSMODULE_FIBER );
   sceSysmoduleLoadModule( SCE_SYSMODULE_ULT );
   sceSysmoduleLoadModule( SCE_SYSMODULE_NP_BASIC );
   sceSysmoduleLoadModule( SCE_SYSMODULE_NET );
   sceSysmoduleLoadModule( SCE_SYSMODULE_HTTPS );
   sceSysmoduleLoadModule( SCE_SYSMODULE_NP );
   sceSysmoduleLoadModule( SCE_SYSMODULE_SYSTEM_GESTURE );
   sceSysmoduleLoadModule( SCE_SYSMODULE_SCREEN_SHOT );
   sceSysmoduleLoadModule( SCE_SYSMODULE_NP_TROPHY );
   sceSysmoduleLoadModule( SCE_SYSMODULE_SYSTEM_GESTURE );
   sceSysmoduleLoadModule( SCE_SYSMODULE_NP_TUS );

#ifndef GOLD_VERSION
   sceSysmoduleLoadModule( SCE_SYSMODULE_PERF );
#endif
}

//----------------------------------------------------------------------------

COsContext::COsContext()
: CBaseOsContext()
, mMountFileHandle( -1 )
, mpMountBuffer( NULL )
, mLowLevelLanguage( -1 )
, mSystemResumeCount( 0 )
{
   load_vita_prx();

   InitializeAppUtil();

   InitializeBuildSKU();
   InitializeLanguage();
   InitializeLowLevelLanguage();
   InitializeBuildVersion();
   InitializeFios();

   scePowerSetConfigurationMode( SCE_POWER_CONFIGURATION_MODE_C );

   InitializeNetwork();
   InitializeBGMConfig();
   InitializeCommonDialogConfig();
}

//----------------------------------------------------------------------------

COsContext::~COsContext()
{
   ShutdownFios();
}

//----------------------------------------------------------------------------

void COsContext::ShutdownFios()
{
   if ( mpMountBuffer )
   {
      BPE_CHECK_SCE( sceFiosArchiveUnmountSync( NULL, mMountFileHandle ) );
      free( mpMountBuffer );
      mpMountBuffer = NULL;
   }

   sRootFolder = NULL;

   BPE_CHECK_SCE( sceFiosIOFilterRemove( 0 ) );

   sceFiosTerminate();
}

//----------------------------------------------------------------------------

void COsContext::InitializeFios()
{
   // Let's initialize FIOS here
   mDearchiverContext = (SceFiosPsarcDearchiverContext) SCE_FIOS_PSARC_DEARCHIVER_CONTEXT_INITIALIZER;
   SceFiosParams params = SCE_FIOS_PARAMS_INITIALIZER;

   /*E Provide required storage buffers. */
   params.opStorage.pPtr = mOpStorage;
   params.opStorage.length = sizeof(mOpStorage);
   params.chunkStorage.pPtr = mChunkStorage;
   params.chunkStorage.length = sizeof(mChunkStorage);
   params.fhStorage.pPtr = mFHStorage;
   params.fhStorage.length = sizeof(mFHStorage);
   params.dhStorage.pPtr = mDHStorage;
   params.dhStorage.length = sizeof(mDHStorage);

   params.pathMax = MAX_PATH_LENGTH;

   params.pMemcpy = memcpy;

   BPE_CHECK_SCE( sceFiosInitialize(&params) );

   mDearchiverContext.workBufferSize = sizeof( sDearchiverWorkBuffer );
   mDearchiverContext.pWorkBuffer = sDearchiverWorkBuffer;

   BPE_CHECK_SCE( sceFiosIOFilterAdd(0, sceFiosIOFilterPsarcDearchiver, &mDearchiverContext) );

   // Try to mount bp path
   static char const *skArchiveMountPoint = "/bp";

   static char const * const skFilenameLoadOrder[] = 
   {
      "app0:",
   };

   // Set to host0 by default in case we are loading from loose files.
   // Not app0 because app0 is forced to be case-sensitive
#ifdef GOLD_VERSION
   sRootFolder = "app0:"; 
#else
   sRootFolder = "host0:"; 
#endif

   // TryMountArchive will set sRootFolder if it succeeds
   
   bool anyMountWorked = false;

   for ( int i = 0; i < BPE_ARRAY_SIZE( skFilenameLoadOrder ); ++i )
   {
      char fname[ 255 ];
      strcpy( fname, skFilenameLoadOrder[i] );
      strcat( fname, gkBP_BluepointAssets_Vita_Archive_Filename_Base );

      switch ( mBuildSKU )
      {
      case kBS_Europe:
         strcat( fname, "_eu" );
         break;
      case kBS_Japan:
         strcat( fname, "_jp" );
         break;
      case kBS_USA:
         break;
      }

      strcat( fname, ".psp2arc" );

      if ( TryMountArchive( fname, skArchiveMountPoint ) )
      {
         printf( "******* Archive: %s for Bluepoint assets *******\n", fname );

         anyMountWorked = true;
         break;
      }
   }

   if ( !anyMountWorked )
   {
      printf( "******* Archive: NONE for Bluepoint assets - Loose files only *******\n" );
   }
}

//----------------------------------------------------------------------------

void COsContext::InitializeAppUtil()
{
   memset(&mAppUtilInitParam, 0, sizeof(SceAppUtilInitParam));
   memset(&mAppUtilBootParam, 0, sizeof(SceAppUtilBootParam));

   int res = sceAppUtilInit(&mAppUtilInitParam, &mAppUtilBootParam);
   BPE_VERIFY(res == SCE_OK, false, "Error initializing apputil library.");
}

//----------------------------------------------------------------------------

void COsContext::InitializeCommonDialogConfig()
{
   SceCommonDialogConfigParam config;
   
   memset(&config, 0, sizeof(SceCommonDialogConfigParam));
   
   sceCommonDialogConfigParamInit(&config);
   config.language = mLowLevelLanguage;
   sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON_ASSIGN, &config.enterButtonAssign);
   int res = sceCommonDialogSetConfigParam(&config);
   BPE_VERIFY(res == SCE_OK, false, "Error initializing sceCommonDialogSetConfigParam.");
}


//----------------------------------------------------------------------------

char const *COsContext::GetRootFolder()
{
   return sRootFolder;
}

//----------------------------------------------------------------------------

bool COsContext::TryMountArchive( char const *file, char const *mountPoint )
{
   BPE_ASSERT( mpMountBuffer == NULL, "Shouldn't be re-mounting something" );
   if ( mpMountBuffer )
   {
      return false;
   }

   SceFiosSize mountBufferSize = sceFiosArchiveGetMountBufferSizeSync( NULL, file, NULL );
   if ( mountBufferSize >= 0 )
   {
      mpMountBuffer = malloc( mountBufferSize );

      SceFiosBuffer buffer;
      buffer.set( mpMountBuffer, mountBufferSize );

      BPE_CHECK_SCE( sceFiosArchiveMountSync( NULL, &mMountFileHandle, file, mountPoint, buffer, NULL ) );
      sRootFolder = mountPoint;

      return true;
   }
   else
   {
      return false;
   }
}

//----------------------------------------------------------------------------

std::string CBaseOsContext::GetMachineName()
{
   return "localhost";
}

//----------------------------------------------------------------------------

std::string CBaseOsContext::GetCurrentUserName()
{
   return "vta_user";
}

//----------------------------------------------------------------------------

void COsContext::InitializeBuildSKU()
{
   mBuildSKU = kBS_USA;
   mSystemRegion = kR_USA;

   SceIoStat fileStatus = { 0 };

   // AS(JM) - Check for the existence of a region_<region>.txt file.  If that's 
   // there, then we're running in the reason.
   //
   // Should probably change this to checking by title id, but we don't have title
   // id's yet.

   if ( SCE_OK == sceIoGetstat( "app0:region_jp.txt", &fileStatus ) )
   {
      mBuildSKU = kBS_Japan;
      mSystemRegion = kR_Japan;
      mControllerRegion = kCR_Japan;
      strcpy( mTitleId_SaveData, skTitleId_JP_SaveData );
   }
   else if ( SCE_OK == sceIoGetstat( "app0:region_eu.txt", &fileStatus ) )
   {
      mBuildSKU = kBS_Europe;
      mSystemRegion = kR_Europe;
      mControllerRegion = kCR_USA;
      strcpy( mTitleId_SaveData, skTitleId_EU_SaveData );
   }
   else
   {
      mBuildSKU = kBS_USA;
      mSystemRegion = kR_USA;
      mControllerRegion = kCR_USA;
      strcpy( mTitleId_SaveData, skTitleId_NA_SaveData );
   }
   
   // AS(JM) - Check if this is a "no bootloader" version
   // That's what "downloadable" means in this context.
   if ( SCE_OK == sceIoGetstat( "app0:no_bootloader.txt", &fileStatus ) )
   {
      mIsDownloadableVersion = true;
   }
}

//----------------------------------------------------------------------------

void COsContext::InitializeBuildVersion()
{
   SceUID changelistFile = sceIoOpen( "app0:changelist.txt", SCE_O_RDONLY, 0 );

   if ( changelistFile >= 0 )
   {
      int const skMaxFileSize = sizeof( mBuildVersionString ) - 1;
         
      int bytesRead = sceIoRead( changelistFile, mBuildVersionString, skMaxFileSize );
      if ( bytesRead >= 0 )
      {
         mBuildVersionString[ bytesRead ] = 0;
         size_t realStrSize = strcspn( mBuildVersionString, "\r\n\026" );
         mBuildVersionString[ realStrSize ] = 0;
      }
      else
      {
         memset( mBuildVersionString, 0, sizeof( mBuildVersionString ) );
      }

      sceIoClose( changelistFile );
   }
}

//----------------------------------------------------------------------------

void COsContext::InitializeLanguage()
{
   // This function translates the system language to a Bluepoint language,
   // then calls a function to filter the result based on the build sku

   mLanguage = kL_English;

   SceInt32 language = 0;

   if ( SCE_OK == BPE_CHECK_SCE( sceAppUtilSystemParamGetInt( SCE_SYSTEM_PARAM_ID_LANG, &language ) ) )
   {
      switch ( language )
      {
      case SCE_SYSTEM_PARAM_LANG_JAPANESE:
         mLanguage = kL_Japanese;
         break;

      case SCE_SYSTEM_PARAM_LANG_FRENCH:
         mLanguage = kL_French;
         break;

      case SCE_SYSTEM_PARAM_LANG_SPANISH:
         mLanguage = kL_Spanish;
         break;

      case SCE_SYSTEM_PARAM_LANG_GERMAN:
         mLanguage = kL_German;
         break;

      case SCE_SYSTEM_PARAM_LANG_ITALIAN:
         mLanguage = kL_Italian;
         break;

      case SCE_SYSTEM_PARAM_LANG_PORTUGUESE_PT:
      case SCE_SYSTEM_PARAM_LANG_PORTUGUESE_BR:
         mLanguage = kL_Portuguese;
         break;

      case SCE_SYSTEM_PARAM_LANG_DUTCH:
         mLanguage = kL_Dutch;
         break;

      case SCE_SYSTEM_PARAM_LANG_RUSSIAN:
         mLanguage = kL_Russian;
         break;

      case SCE_SYSTEM_PARAM_LANG_ENGLISH_US:
      case SCE_SYSTEM_PARAM_LANG_KOREAN:
      case SCE_SYSTEM_PARAM_LANG_CHINESE_T:
      case SCE_SYSTEM_PARAM_LANG_CHINESE_S:
      case SCE_SYSTEM_PARAM_LANG_FINNISH:
      case SCE_SYSTEM_PARAM_LANG_SWEDISH:
      case SCE_SYSTEM_PARAM_LANG_DANISH:
      case SCE_SYSTEM_PARAM_LANG_NORWEGIAN:
      case SCE_SYSTEM_PARAM_LANG_POLISH:
      case SCE_SYSTEM_PARAM_LANG_ENGLISH_GB:
      default:
         // All other languages are english
         mLanguage = kL_English;
         break;
      }
   }

   ForceSpecificLanguageBasedOnBuildSKU();
}

//----------------------------------------------------------------------------

void COsContext::InitializeLowLevelLanguage()
{
   mLowLevelLanguage = SCE_SYSTEM_PARAM_LANG_ENGLISH_US;

   switch ( mLanguage )
   {
   case kL_English:
      if ( mBuildSKU == kBS_Europe ) 
      {
         mLowLevelLanguage = SCE_SYSTEM_PARAM_LANG_ENGLISH_GB;
      }
      else
      {
         mLowLevelLanguage = SCE_SYSTEM_PARAM_LANG_ENGLISH_US;
      }
      break;
   case kL_French:
      mLowLevelLanguage = SCE_SYSTEM_PARAM_LANG_FRENCH;
      break;
   case kL_Japanese:
      mLowLevelLanguage = SCE_SYSTEM_PARAM_LANG_JAPANESE;
      break;
   case kL_German:
      mLowLevelLanguage = SCE_SYSTEM_PARAM_LANG_GERMAN;
      break;
   case kL_Italian:
      mLowLevelLanguage = SCE_SYSTEM_PARAM_LANG_ITALIAN;
      break;
   case kL_Portuguese:
      if ( mBuildSKU == kBS_Europe ) 
      {
         mLowLevelLanguage = SCE_SYSTEM_PARAM_LANG_PORTUGUESE_PT;
      }
      else
      {
         mLowLevelLanguage = SCE_SYSTEM_PARAM_LANG_PORTUGUESE_BR;
      }
      break;
   case kL_Spanish:
      mLowLevelLanguage = SCE_SYSTEM_PARAM_LANG_SPANISH;
      break;
   case kL_Dutch:
      mLowLevelLanguage = SCE_SYSTEM_PARAM_LANG_DUTCH;
      break;
   case kL_Russian:
      mLowLevelLanguage = SCE_SYSTEM_PARAM_LANG_RUSSIAN;
      break;
   default:
      BPE_ASSERT( false, "Invalid language!" );
      mLowLevelLanguage = SCE_SYSTEM_PARAM_LANG_ENGLISH_US;
      break;
   }
}

//----------------------------------------------------------------------------

void CBaseOsContext::TerminateProcess()
{
   // Force verify, we don't support 'forced' terminate process
   BPE_VERIFYA(false, "CBaseOsContext::TerminateProcess().");
}

//----------------------------------------------------------------------------

CBaseOsContext::EProfileStatus CBaseOsContext::GetUserProfile()
{
   return kPS_Valid;
}

//----------------------------------------------------------------------------

void CBaseOsContext::SetUnsafeToShutDownFlag( uint32 const flag )
{
   sceAtomicOr32( (volatile int32_t*)&mUnsafeToShutDownFlags, flag );
}

//----------------------------------------------------------------------------

void CBaseOsContext::ClearUnsafeToShutDownFlag( uint32 const flag )
{
   sceAtomicAnd32( (volatile int32_t*)&mUnsafeToShutDownFlags, ~flag );
}

//----------------------------------------------------------------------------

bool CBaseOsContext::IsSafeToShutDown() const
{
   return mUnsafeToShutDownFlags == 0;
}

//----------------------------------------------------------------------------

void COsContext::InitializeNetwork()
{
   static char sNetInitMemory[32 * 1024]; // recommended by docs for SceNetInitParam
   SceNetInitParam param;

   param.memory = sNetInitMemory;
   param.size = sizeof(sNetInitMemory);
   param.flags = 0;

   BPE_CHECK_SCE( sceNetInit(&param) );
   BPE_CHECK_SCE( sceNetCtlInit() );
   BPE_CHECK_SCE( sceSslInit( SCE_NETCHECK_DIALOG_LEAST_SSL_POOL_SIZE * 2) );
   BPE_CHECK_SCE( sceHttpInit( SCE_NETCHECK_DIALOG_LEAST_HTTP_POOL_SIZE * 2) );
}

//----------------------------------------------------------------------------

void COsContext::InitializeBGMConfig()
{
   // Take over the BGM port.  Should always work, but if it doesn't it's
   // not fatal.
   sceAppMgrAcquireBgmPort();
}

//----------------------------------------------------------------------------

void COsContext::ProcessPlatform()
{
   // Check to see if we have a "reusme" event in the queue. If so,
   // increment the system resume count.

   SceAppMgrAppState appState;
   int ret;

   ret = sceAppMgrGetAppState ( &appState );
   if ( ( ret == SCE_OK ) && (appState.systemEventNum > 0 ) )
   {
      for ( int i = 0; i < appState.systemEventNum; i++ )
      {
         SceAppMgrSystemEvent systemEvent;

         ret = sceAppMgrReceiveSystemEvent( &systemEvent );
         
         if ( ret >= SCE_OK )
         {
            if( systemEvent.systemEvent == SCE_APPMGR_SYSTEMEVENT_ON_RESUME)
            {
               ++mSystemResumeCount;
            }
         }
      }
   }

}

//----------------------------------------------------------------------------

extern "C"
{
   SceInt32 sceDbgLoggingHandler(const char *pFile, int line, int severity, const char *pComponent, const char *pMessage, ...)
   {
      va_list args;
      va_start( args, pMessage );
      printf( "%s(%d): severity %d: %s: ", pFile, line, severity, pComponent );
      vprintf( pMessage, args );
      va_end( args );

      return SCE_OK;
   }
}
