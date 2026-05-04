//----------------------------------------------------------------------------
// BP_FileSupportPS3.cpp
//----------------------------------------------------------------------------

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/System/CSPURSManager.h"
#include "Engine/System/CStopWatch.h"
#include "Engine/StlExtras/BPEStlExtras.h"
#include "Engine/System/COsContext.h"
#include "Engine/Memory/CMemoryAllocator.h"
#include "boost/shared_ptr.hpp"
#include "Renderer/Base/Backend/CRenderBackend.h"

#ifdef _PS3

#include <sys/stat.h>
#include <cell/fios.h>
#include <cell/fios/compression/edge/edgezlib_decompressor.h>
#include <ctype.h>
#include <sysutil/sysutil_syscache.h>
#include <sysutil/sysutil_common.h>
#include <sys/timer.h>

#include "BP_FileSupport.h"
#include "BP_Misc.h"

// Include for global disk build 'enable'.
#include "../../../Builds/DiskBuilds/UseDiskBuild.h"

//----------------------------------------------------------------------------

//BP - Code for handling different filename paths based on disc or Target Manager hosting

//#define BP_FIOS_LOAD_NEWER_FROM_FS //BP - Comment in to load files from file system rather than psarc when newer

#ifndef BP_TEXTURE_TOOL_LOAD_TGAS
#ifndef BP_FIOS_LOAD_NEWER_FROM_FS

#define BP_FIOS_USE_RAM_CACHE
#define BP_FIOS_USE_HDD_CACHE

#endif
#endif

#if defined _BP_DISK_BUILD && !defined( GOLD_VERSION )
#define _USE_AUX_MEDIA
#endif

//#define _BP_DISK_SLOW_EMULATION  //BP - Comment in to enable slow disk emulation

int BP_IsDiskBuild()
{
#ifdef _BP_DISK_BUILD
   return 1;
#else
   return 0;
#endif
}

char const * BP_GetRootFilesystemPath()
{
#ifdef _BP_DISK_BUILD
   // See if we've already initialized the filesystem path
   static char sRootPath[128] = {0};
   if (sRootPath[0] == 0)
   {
      if (OsContext()->mBootType == COsContext::kBT_HDD)
      {
         // Set game content path (keep MGS directories for consistency with disc version)
#if MGS_VERSION==2
         sprintf(sRootPath, "%s/MGS2/", OsContext()->mGameUsrdirPath);
#else
         sprintf(sRootPath, "%s/MGS3/", OsContext()->mGameUsrdirPath);
#endif
      }      
      else
      {
         // Disc game loads content files from disc (this might change for patched data)
         // Hard coded disc path avoids issues with running from development host when testing
#if MGS_VERSION==2
         strcpy(sRootPath, "/dev_bdvd/PS3_GAME/USRDIR/MGS2/");
#else
         strcpy(sRootPath, "/dev_bdvd/PS3_GAME/USRDIR/MGS3/");
#endif
      }
      bpe_debugger_printf("*** Init BP_GetRootFilesystemPath: %s\n", sRootPath);
   } 
#else
   // Load files from development machine
   static char const * sRootPath = "/app_home/";
#endif
   return sRootPath;
}

const char * const kArchiveFilenames[] =
{
#if MGS_VERSION==2
   "mgs2",
   "mgs2_misc",
#elif MGS_VERSION==3
   "mgs3",
   "mgs3_misc",
#endif
};

int const kArchiveCount = BPE_ARRAY_SIZE(kArchiveFilenames);

static bool sDiscEjected = false;

//----------------------------------------------------------------------------

//BP - Convert string inplace to lower case
static void _str_tolower(char * pStr)
{
   for( char * pc = pStr; *pc != NULL; ++pc )
   {
      *pc = tolower( *pc );
   }
}

static void _str_toupper(char * pStr)
{
   for( char * pc = pStr; *pc != NULL; ++pc )
   {
      *pc = toupper( *pc );
   }
}

// BP - called after a failed file operation.  The OS *should* give us a callback
// when the Blu-Ray is ejected or if it gives up on a file read so terminate==true.
static void wait_hard_shutdown( int errCode )
{
#if 1
   // simply call BP_WaitForHardShutdown, defined in BP_Misc.cpp
   bool const kWaitForShutdown = true;
   BP_CheckForHardShutdown(kWaitForShutdown);
#else
   // Left for reference.
   printf("BP_FILESYSTEM FIOS ERROR: 0x%08x\n", errCode );
   BP_TODO_BREAK;
#if 1
   // Immediately mute all sound to avoid streaming sound errors while we wait.
   //TODO: Do something comparable to BP_MultiStream_SetMasterVol(0.f);

   while (true)
   {
      // ClearAndSwapForOSD will check for game termination and perform shutdown.
      RenderBackend()->ClearAndSwapForOSD();

      // check the callback queue.
      cellSysutilCheckCallback();

      // sleep for a bit.
      sys_timer_usleep(500);
   }
#endif
   //BP - should never reach here.
   BP_BREAK;
#endif
}

// BP - handles disc eject notifications.
static cell::fios::err_t MediaCallback( void *context,
                                        cell::fios::media *media,
                                        cell::fios::mediaevent_t event,
                                        void* param )
{
   // the media has been ejected.  We need to mark it as such so that
   // we deal with "cancel" notifications properly.
   if ( event == cell::fios::kMEDIAEVENT_GONE )
   {
      sDiscEjected = true;
      return cell::fios::CELL_FIOS_ERROR_MEDIAGONE;
   }

   // unknown, so don't do anything with it.
   return cell::fios::CELL_FIOS_ERROR_UNKNOWNCALLBACKEVENT;
}

//----------------------------------------------------------------------------
// FIOS interface

namespace
{
   //(not really needed as we clear the system cache on app startup anyway)
#if MGS_VERSION==2
   const uint64_t skHDDCacheDiscIDPerSKU[] =
   {
      0xEB55AF5B30F3E1F5ULL,  //USA
      0x8243CD9C732D1F00ULL,  //Japan
      0x5B7A00632E8F0163ULL,  //Europe
   };
#elif MGS_VERSION==3
   const uint64_t skHDDCacheDiscIDPerSKU[] =
   {
      0x50644009BE1F858DULL,  //USA
      0xC90E235A921B989BULL,  //Japan
      0x4ff9AE636E624CC1ULL,  //Europe
   };
#endif

   BPE_CTASSERT( BPE_ARRAY_SIZE( skHDDCacheDiscIDPerSKU ) == COsContext::kBS_Count );

   class CFiosAllocator : public cell::fios::allocator
   {
   public:
      CFiosAllocator() {}
      virtual ~CFiosAllocator() {}

      void* Allocate(uint32_t size, uint32_t flags, const char* pFile = 0, int line = 0) {
         (void) pFile;
         (void) line;
         size_t const align = FIOS_ALIGNMENT_FROM_MEMFLAGS(flags);
         void * pPtr = BPE_MALLOC_ALIGNED( align, size );
         return pPtr;
      }

      void Deallocate(void* pMemory, uint32_t flags, const char* pFile = 0, int line = 0) {
         (void) flags;
         (void) pFile;
         (void) line;
         BPE_FREE_ALIGNED( pMemory );
      }

      void* Reallocate(void* pMemory, uint32_t newSize, uint32_t flags, const char* pFile = 0, int line = 0) {
         (void) pMemory;
         (void) newSize;
         (void) flags;
         (void) pFile;
         (void) line;
         return NULL; /* fios does not use Reallocate */
      }
   };

   class CFiosInterface {
   public:
      CFiosInterface();
      ~CFiosInterface();

      void Initialize();
      void Shutdown();

      CFiosAllocator mAllocator;

      cell::fios::media*    mAppHomeMedia;
#ifdef _USE_AUX_MEDIA
      cell::fios::media*    mAuxMedia;
#endif
      cell::fios::emulation * mEmulation;
      cell::fios::ramcache* mRAMCache;

      //HDD cache support
      CellSysCacheParam          mSysCacheParam;
      cell::fios::media*          mSysCacheMedia;
      cell::fios::scheduler*      mSysCacheScheduler;
      cell::fios::schedulercache* mSchedulerCache;

      cell::fios::Compression::EdgeZlibDecompressor* mEdgeZlibDecompressor;

      cell::fios::dearchiver* mDearchiver;
      cell::fios::scheduler*  mMainScheduler;
#ifdef _USE_AUX_MEDIA
      cell::fios::scheduler*  mAuxScheduler;
#endif
      cell::fios::filehandle* mArchive[kArchiveCount];
      bool                    mbHasPsarc;
      uint64                  mPsarcModificationDate;
   };

   CFiosInterface sFios;
}

CFiosInterface::CFiosInterface()
: mAppHomeMedia(0)
#ifdef _USE_AUX_MEDIA
, mAuxMedia(0)
, mAuxScheduler(0)
#endif
, mEmulation(0)
, mRAMCache(0)
, mSysCacheParam()
, mSysCacheMedia(0)
, mSysCacheScheduler(0)
, mSchedulerCache(0)
, mEdgeZlibDecompressor(0)
, mDearchiver(0)
, mMainScheduler(0)
, mbHasPsarc( false )
, mPsarcModificationDate( 0 )
{
   for( int i = 0; i < kArchiveCount; ++i )
      mArchive[i] = 0;
}

CFiosInterface::~CFiosInterface()
{
}

void CFiosInterface::Initialize()
{
   int err;
   // Start with "reasonable defaults"
   cell::fios::fios_parameters parameters = FIOS_PARAMETERS_INITIALIZER;

   parameters.pAllocator = &mAllocator;
#if CELL_SDK_VERSION < 0x330001
   parameters.sharedPath = false;
#endif
   parameters.pLargeMemcpy = NULL; // Use memcpy
   parameters.pVprintf = NULL;     // Use vprintf
   //   parameters.profiling = cell::fios::kProfileMediaAccess | cell::fios::kProfileCache;
   //   parameters.profiling = cell::fios::kProfileAll;

   cell::fios::FIOSInit(&parameters);

   // Create media on /app_home.
   mAppHomeMedia = new cell::fios::ps3media(BP_GetRootFilesystemPath());
   BPE_ASSERT(mAppHomeMedia != NULL, ("Can't allocate mAppHomeMedia") );

   // Register the disc removal callback.
   mAppHomeMedia->setCallback( MediaCallback, 0 );

#ifdef _USE_AUX_MEDIA
   mAuxMedia = new cell::fios::ps3media("/app_home/");
#endif

   cell::fios::media * pMediaBelowDearchiver = mAppHomeMedia;
#ifdef _BP_DISK_SLOW_EMULATION
   cell::fios::emulation_params emulationParms( cell::fios::g_emulateBluray );
   // N.B. These defaults are copied from "Emulate Blu-Ray" settings:
   emulationParms.m_blockSize = 2048;
   emulationParms.m_readBandwidth = 8912896;
   emulationParms.m_writeBandwidth = 8912896;
   emulationParms.m_seekTime = 200000;
   emulationParms.m_latency = 50;

   mEmulation = new cell::fios::emulation( mAppHomeMedia, &emulationParms );
   pMediaBelowDearchiver = mEmulation;
#endif

#ifdef BP_FIOS_USE_HDD_CACHE
   strcpy(mSysCacheParam.cacheId, gpOsContext->mTitleId);
   mSysCacheParam.reserved = 0;
   err = cellSysCacheMount(&mSysCacheParam);

   if(err != CELL_SYSCACHE_RET_OK_CLEARED &&
      err != CELL_SYSCACHE_RET_OK_RELAYED) {
         printf("%s %d : %x\n", __FUNCTION__, __LINE__, err);
         BPE_ASSERT(false, ("System cache init failed") );
   }
   //If there were a way to get a checksum of our psarc or somesuch to mix in with the
   //hdd cache disc id, we could safely refrain from clearing on startup.  BUT having hit
   //a decompression crash once already due to stale data in sysCache from another version
   //it seems to me that this is the safest thing during development.
   err = cellSysCacheClear();
   if( err != CELL_OK )
   {
      printf("BP_FIOS: warning: sysCache clear failed\n");
   }

   printf("BP_FIOS: SysCache path: %s\n", mSysCacheParam.getCachePath);
   uint32_t blockSize;
   uint64_t freeBlockCount;
   err = cellFsGetFreeSize( mSysCacheParam.getCachePath, &blockSize, &freeBlockCount );
   if( err == CELL_OK )
   {
      printf("BP_FIOS: SysCache free size %d, %lld\n", blockSize, freeBlockCount);

      mSysCacheMedia = new cell::fios::ps3media(mSysCacheParam.getCachePath);
      BPE_ASSERT(mSysCacheMedia != NULL, ("Can't allocate mSysCacheMedia") );

      mSysCacheScheduler = cell::fios::scheduler::createSchedulerForMedia(mSysCacheMedia);
      BPE_ASSERT(mSysCacheScheduler != NULL, ("Can't allocate mSysCacheScheduler") );

      const uint64_t kHDDCacheDiscID            = skHDDCacheDiscIDPerSKU[gpOsContext->mBuildSKU];
      //2037 x 1MB blocks recommended by cell::fios::cache() documentation (apparently that's the maximum
      //to leave space for filesystem overhead)
      const int kHDDCacheBlockCount = 2037;
      const int kHDDCacheBlockSize = 1024 * 1024;
      mSchedulerCache = new cell::fios::schedulercache(pMediaBelowDearchiver, mSysCacheScheduler,
         "FIOSCACHE",   //cache directory
         kHDDCacheDiscID,
         true,    //use single file
         false,   //check modification date
         kHDDCacheBlockCount,
         kHDDCacheBlockSize
         );
      BPE_ASSERT(mSchedulerCache != NULL, ("Can't allocate mSchedulerCache") );
      pMediaBelowDearchiver = mSchedulerCache;
   }
   else
   {
      printf("BP_FIOS: warning: cellFsGetFreeSize failed %d\n", err);
   }

#endif

   // Create RAM cache.
#ifdef BP_FIOS_USE_RAM_CACHE
   const int kRAMCacheBlockCount = 8;
   const int kRAMCacheBlockSize = 512 * 1024;
   mRAMCache = new cell::fios::ramcache(pMediaBelowDearchiver, kRAMCacheBlockCount, kRAMCacheBlockSize);
   BPE_ASSERT(mRAMCache != NULL, ("Can't allocate RAM cache") );
   pMediaBelowDearchiver = mRAMCache;
#endif

   // Create EdgeZlibDecompressor for fios::dearchiver.
   {
      mEdgeZlibDecompressor = new cell::fios::Compression::EdgeZlibDecompressor();
      BPE_ASSERT(mEdgeZlibDecompressor != NULL, ("Can't allocate mAppHomeMedia") );

      uint8_t decompressorPriority[] = { 8, 8, 8, 8, 8, 8, 0, 0 };
      CellSpursTaskset* pTaskSet = mEdgeZlibDecompressor->createTaskset(gpSpursManager->mpSPURS_System.get(), decompressorPriority);
      mEdgeZlibDecompressor->init(gpSpursManager->mpSPURS_System.get(), pTaskSet);
   }

   // Create dearchiver.
   mDearchiver = new cell::fios::dearchiver(pMediaBelowDearchiver, mEdgeZlibDecompressor);
   BPE_ASSERT(mDearchiver != NULL, ("Can't allocate mDearchiver") );

   // Create main scheduler.
   mMainScheduler = cell::fios::scheduler::createSchedulerForMedia(mDearchiver);
   BPE_ASSERT(mMainScheduler != NULL, ("Can't allocate mMainScheduler") );

#ifdef _USE_AUX_MEDIA
   mAuxScheduler = cell::fios::scheduler::createSchedulerForMedia(mAuxMedia);
   BPE_ASSERT(mAuxScheduler!= NULL, ("Can't allocate mAuxScheduler") );
#endif

   //   mMainScheduler->setPrefetchDelay( cell::fios::FIOSMillisecondsToAbstime( 200 ) );

   // Make mMainScheduler the default scheduler.
   mMainScheduler->setDefault();


   // Don't use psarc when loading tgas directly
#ifndef BP_TEXTURE_TOOL_LOAD_TGAS

   char psarcPath[FILENAME_MAX];

   for( int i = 0; i < kArchiveCount; ++i )
   {
      strcpy( psarcPath, "/");
      strcat( psarcPath, kArchiveFilenames[i] );
      switch( OsContext()->mBuildSKU )
      {
      case COsContext::kBS_Japan:
         strcat( psarcPath, "_jp" );
         break;
      case COsContext::kBS_Europe:
         strcat( psarcPath, "_eu" );
         break;
      }
      strcat( psarcPath, ".psarc" );

      {
         // If we're running as a disk build and cannot open PSARC, try and reopen and also check for termination processing.
         bool bRecheckForPSARC = true;

         while (bRecheckForPSARC)
         {
#ifndef _BP_DISK_BUILD
            bRecheckForPSARC = false;
#endif

            err = mMainScheduler->openFileSync(0, psarcPath, cell::fios::kO_RDONLY, &mArchive[i]);
            if(err != cell::fios::CELL_FIOS_NOERROR )
            {
               //Okay to not have built this archive.
               bpe_debugger_printf("Archive not found: %s\n", kArchiveFilenames[i]);
            }
            else
            {
               mbHasPsarc = true;
               bRecheckForPSARC = false;

               //Cache modification date for comparison with file dates when checking to load outside psarc
               cell::fios::stat_t stat = { NULL };
               err = sFios.mMainScheduler->statSync( NULL, psarcPath, &stat );
               if( err != cell::fios::CELL_FIOS_NOERROR )
               {
                  printf("Warning: statSync (%s) failed %d\n", psarcPath, err );
               }
               // Get file modification date if it's the first one we're retrieving or grab whichever is OLDEST.
               if( mPsarcModificationDate == 0 || (stat.modificationDate < mPsarcModificationDate) )
                  mPsarcModificationDate = stat.modificationDate;
            }

            if (bRecheckForPSARC)
            {
               // We couldn't open PSARC in a disk build, check for termination processing
               // simply call BP_WaitForHardShutdown, defined in BP_Misc.cpp
               bool const kWaitForShutdown = false;
               BP_CheckForHardShutdown(kWaitForShutdown);
            }
         }
      }
   }

#endif//#ifndef BP_TEXTURE_TOOL_LOAD_TGAS
}

static void bp_delete_all_fios_ops( void * context, cell::fios::op * pOp )
{
   cell::fios::scheduler* pScheduler = (cell::fios::scheduler*)context;
   //   printf("bp_delete_all_fios_ops: cancelling / deleting op %p\n", pOp );
   pScheduler->deleteOp( pOp );
}

void CFiosInterface::Shutdown()
{
   mMainScheduler->cancelAllOps();
   mMainScheduler->iterateOps( bp_delete_all_fios_ops, mMainScheduler );
   mMainScheduler->closeAllFiles();
#if 0
   int err = mMainScheduler->closeFileSync( NULL, mArchive );
   if( err != cell::fios::CELL_FIOS_NOERROR )
   {
      printf("Failed to close archive " PS3_ARCHIVE_NAME "\n" );
   }
#endif
   cell::fios::scheduler::destroyScheduler(mMainScheduler);
#ifdef _USE_AUX_MEDIA
   cell::fios::scheduler::destroyScheduler(mAuxScheduler);
#endif

   delete mDearchiver;
   delete mEdgeZlibDecompressor;
   delete mRAMCache;

#ifdef BP_FIOS_USE_HDD_CACHE
   delete mSchedulerCache;
   mSysCacheScheduler->cancelAllOps();
   mSysCacheScheduler->iterateOps( bp_delete_all_fios_ops, mSysCacheScheduler );
   mSysCacheScheduler->closeAllFiles();
   cell::fios::scheduler::destroyScheduler(mSysCacheScheduler);
   delete mSysCacheMedia;
#endif

#ifdef _BP_DISK_SLOW_EMULATION
   delete mEmulation;
#endif

   delete mAppHomeMedia;
   mbHasPsarc = false;

#ifdef _USE_AUX_MEDIA
   delete mAuxMedia;
#endif

   cell::fios::FIOSTerminate();
}

static bool should_load_from_psarc_file( const char * const path )
{
   if( sFios.mbHasPsarc )
   {
#ifdef BP_FIOS_LOAD_NEWER_FROM_FS
      char hostPath[FILENAME_MAX];

      strcpy( hostPath, get_path_without_precache_folder(path) );
      _str_toupper( hostPath );
      cell::fios::stat_t stat = { NULL };
      int err = sFios.mMainScheduler->statSync( NULL, hostPath, &stat );
      if( err != cell::fios::CELL_FIOS_NOERROR )
      {
         //This exact path is not in the file system. (happens with bp_precache textures atm)
         //load from psarc.
         return true;
      }
      if( stat.modificationDate > sFios.mPsarcModificationDate )
      {
         //Newer exists in the file system.
         //Don't load this one from the psarc.
         printf("FIOS: Loading %s direct because it's newer than PSARC\n", path);
         return false;
      }
#endif
      return true;
   }

   return false;
}

//----------------------------------------------------------------------------

extern "C" void BP_InitSoundEffectOverride();   //TED - TEMP!!!

void BP_InitializeFileSupport()
{
   sFios.Initialize();
   //BP_TODO: remove this TGS hack and load the sound effect override data asynchronously *after* the common archive is done loading.
   //Any (few) sound effects used by the title stage that need overrides can be built into the executable.
   BP_InitSoundEffectOverride();
}

void BP_ShutdownFileSupport()
{
   sFios.Shutdown();
}

int BP_IsPsarcMounted()
{
   return sFios.mbHasPsarc ? 1 : 0;
}

void BP_IO_FlushRAMDiskCache()
{
#ifdef BP_FIOS_USE_RAM_CACHE
   sFios.mRAMCache->flush();
#endif
}

void * BP_OpenFile( const char * const path, const char * const commonPath )
{
   char fiosPath[FILENAME_MAX];
   strcpy( fiosPath, path );
   _str_tolower( fiosPath );
   cell::fios::filehandle* pFiosHandle = NULL;
#ifdef _USE_AUX_MEDIA
   cell::fios::err_t ret = 0;
   if ( strstr( path, ".tga" ) != 0 )
      ret = sFios.mAuxScheduler->openFileSync( NULL, fiosPath, cell::fios::kO_RDONLY, &pFiosHandle );
   else
      ret = sFios.mMainScheduler->openFileSync( NULL, fiosPath, cell::fios::kO_RDONLY, &pFiosHandle );
#else
   cell::fios::err_t ret = sFios.mMainScheduler->openFileSync( NULL, fiosPath, cell::fios::kO_RDONLY, &pFiosHandle );
#endif

   BPE_ASSERT( ret == cell::fios::CELL_FIOS_NOERROR, "Failed to open file!" );
   return pFiosHandle;
}

void * BP_OpenFileReadWrite( const char * const path, int truncate )
{
   // this always serves out of app-home.
   char fiosPath[FILENAME_MAX];
   strcpy( fiosPath, path );
   _str_tolower( fiosPath );

   std::uint32_t accessFlags = cell::fios::kO_READ |
                               cell::fios::kO_WRITE |
                               cell::fios::kO_CREAT;
                               
   if ( truncate != 0 )
      accessFlags |= cell::fios::kO_TRUNC;

   cell::fios::filehandle* pFiosHandle = NULL;
#ifdef _USE_AUX_MEDIA
   cell::fios::err_t ret = sFios.mAuxScheduler->openFileSync( NULL, fiosPath, accessFlags, &pFiosHandle );
#else
   cell::fios::err_t ret = sFios.mMainScheduler->openFileSync( NULL, fiosPath, accessFlags, &pFiosHandle );
#endif
   BPE_ASSERT( ret == cell::fios::CELL_FIOS_NOERROR, "Failed to open file!" );
   return pFiosHandle;
}

void BP_CloseFile( void * const fp )
{
   BPE_ASSERT( fp != NULL, "" );
   cell::fios::filehandle* pFiosHandle = (cell::fios::filehandle*)fp;
   cell::fios::err_t ret = pFiosHandle->getScheduler()->closeFileSync( NULL, pFiosHandle );
   BPE_ASSERT( ret == cell::fios::CELL_FIOS_NOERROR, "Failed to close file!" );
}

void BP_SeekFile( void * const fp, const long offset, const int mode )
{
   BPE_ASSERT( fp != NULL, "" );
   cell::fios::filehandle* pFiosHandle = (cell::fios::filehandle*)fp;
   cell::fios::e_WHENCE whence = (cell::fios::e_WHENCE)mode;
   cell::fios::err_t ret = pFiosHandle->seek( offset, whence );
   BPE_ASSERT( ret == cell::fios::CELL_FIOS_NOERROR, "Failed to seek in file!" );
}

long BP_TellFile( void * const fp )
{
   BPE_ASSERT( fp != NULL, "" );
   cell::fios::filehandle* pFiosHandle = (cell::fios::filehandle*)fp;
   return (long)pFiosHandle->getOffset();
}

long BP_GetFileSize( void * const fp )
{
   BPE_ASSERT( fp != NULL, "" );
   cell::fios::filehandle* pFiosHandle = (cell::fios::filehandle*)fp;
   return (long)pFiosHandle->getFileSize();
}

void BP_WriteFile( void* const fp, void const * pBuf, const size_t size )
{
   BPE_ASSERT( fp != NULL, "" );
   cell::fios::filehandle* pFiosHandle = (cell::fios::filehandle*)fp;
   cell::fios::err_t ret = pFiosHandle->getScheduler()->writeFileSync( NULL, pFiosHandle, pBuf, size );
   if( ret == cell::fios::CELL_FIOS_ERROR_EOF || ret == cell::fios::CELL_FIOS_NOERROR )
   {
      //OK
      return;
   }
   else
   {
      wait_hard_shutdown( ret );
   }
}

void BP_ReadFile( void * const fp, void * const pBuf, const size_t size )
{
   BPE_ASSERT( fp != NULL, "" );
   cell::fios::filehandle* pFiosHandle = (cell::fios::filehandle*)fp;
   cell::fios::err_t ret = pFiosHandle->getScheduler()->readFileSync( NULL, pFiosHandle, pBuf, size );
   if( ret == cell::fios::CELL_FIOS_ERROR_EOF || ret == cell::fios::CELL_FIOS_NOERROR )
   {
      //OK
      return;
   }
   else
   {
      wait_hard_shutdown( ret );
   }
}

void BP_ReadFileByFilename( const char * const path, void * const pBuf, const long offset, const size_t size )
{
   char fiosPath[FILENAME_MAX];
   strcpy( fiosPath, path );
   _str_tolower( fiosPath );

   cell::fios::err_t ret = sFios.mMainScheduler->readFileSync( NULL, fiosPath, pBuf, offset, size );
   if( ret == cell::fios::CELL_FIOS_ERROR_EOF || ret == cell::fios::CELL_FIOS_NOERROR )
   {
      //OK
      return;
   }
   else
   {
      wait_hard_shutdown( ret );
   }
}

void * BP_ReadFileAsync( void * const fp, void * const pBuf, const size_t size )
{
   BPE_ASSERT( fp != NULL, "" );
   cell::fios::filehandle* pFiosHandle = (cell::fios::filehandle*)fp;
   cell::fios::op * pOp = pFiosHandle->getScheduler()->readFile( NULL, pFiosHandle, pBuf, size );

   return pOp;
}

void * BP_ReadFileByFilenameAsync( const char * const path, void * const pBuf, const long offset, const size_t size )
{
   char fiosPath[FILENAME_MAX];
   strcpy( fiosPath, path );
   _str_tolower( fiosPath );

#ifndef GOLD_VERSION
   bool bExists = false;
   int ret = sFios.mMainScheduler->fileExistsSync( NULL, fiosPath, &bExists );
   if( ret != cell::fios::CELL_FIOS_NOERROR || !bExists )
   {
      printf("BP_ReadFileByFilenameAsync: file not found %s\n", path );
      BP_BREAK;
   }
#endif

#ifdef _USE_AUX_MEDIA
   cell::fios::opattr_t opAttr = FIOS_OPATTR_INITIALIZER;
   cell::fios::op * pOp = 0;
   if ( strstr( path, ".tga" ) != 0 )
      pOp = sFios.mAuxScheduler->readFile( &opAttr, fiosPath, pBuf, offset, size );
   else
      pOp = sFios.mMainScheduler->readFile( &opAttr, fiosPath, pBuf, offset, size );
#else
   cell::fios::opattr_t opAttr = FIOS_OPATTR_INITIALIZER;
   cell::fios::op * pOp = sFios.mMainScheduler->readFile( &opAttr, fiosPath, pBuf, offset, size );
#endif
   return pOp;
}

int BP_TryFinishFileOp( void * const pOp )
{
   BPE_ASSERT( pOp != NULL, "" );
   cell::fios::op * const pFiosOp = (cell::fios::op*)pOp;
   if( !pFiosOp->isDone() )
   {
      return 0;
   }
   //Done; check for error and delete op.
   cell::fios::err_t ret = pFiosOp->wait();
   if( !(ret == cell::fios::CELL_FIOS_ERROR_EOF || ret == cell::fios::CELL_FIOS_NOERROR))
   {
      // check for a fatal error.
      if ( ret != cell::fios::CELL_FIOS_ERROR_CANCELLED || sDiscEjected )
      {
         wait_hard_shutdown( ret );
      }
   }
   pFiosOp->getScheduler()->deleteOp( pFiosOp );

   return 1;
}

void BP_CancelFileOp( void * const pOp )
{
   BPE_ASSERT( pOp != NULL, "" );
   cell::fios::op * const pFiosOp = (cell::fios::op*)pOp;
   pFiosOp->getScheduler()->cancelOp( pFiosOp );
}

void BP_CancelAndDeleteFileOpSync( void * const pOp )
{
   BP_CancelFileOp( pOp );
   
   while ( BP_TryFinishFileOp( pOp ) == 0 )
   {
   }
}

u_long64 BP_GetFileModificationDate( const char * const path )
{
   char fiosPath[FILENAME_MAX];
   strcpy( fiosPath, path );
   _str_tolower( fiosPath );

   cell::fios::stat_t stat = { NULL };
#ifdef _USE_AUX_MEDIA
   int err = 0;
   if ( strstr( path, ".tga" ) != 0 )
      err = sFios.mAuxScheduler->statSync( NULL, fiosPath, &stat );
   else
      err = sFios.mMainScheduler->statSync( NULL, fiosPath, &stat );
#else
   int err = sFios.mMainScheduler->statSync( NULL, fiosPath, &stat );
#endif
   if( err != cell::fios::CELL_FIOS_NOERROR )
   {
      printf("Warning: BP_GetFileModificationDate (%s) failed %d\n", path, err );
   }
   return stat.modificationDate;
}

long BP_GetFileSizeAttr( const char * const path, const char * const commonPath )
{
   char fiosPath[FILENAME_MAX];
   strcpy( fiosPath, path );
   _str_tolower( fiosPath );

   cell::fios::off_t byteSize;
   cell::fios::err_t ret = 0;
#ifdef _USE_AUX_MEDIA
   if ( strstr( path, ".tga" ) != 0 )
      ret = sFios.mAuxScheduler->getFileSizeSync( NULL, fiosPath, &byteSize );
   else
      ret = sFios.mMainScheduler->getFileSizeSync( NULL, fiosPath, &byteSize );
#else
   ret = sFios.mMainScheduler->getFileSizeSync( NULL, fiosPath, &byteSize );
#endif
   BPE_ASSERT( ret == cell::fios::CELL_FIOS_NOERROR, "Failed to get file size!" );
   return (long)byteSize;
}

int BP_FileExists( const char * const path, const char * const commonPath )
{
   char fiosPath[FILENAME_MAX];
   strcpy( fiosPath, path );
   _str_tolower( fiosPath );

   bool bExists;
   cell::fios::err_t ret = 0;
#ifdef _USE_AUX_MEDIA
   if ( strstr( path, ".tga" ) != 0 )
      ret = sFios.mAuxScheduler->fileExistsSync( NULL, fiosPath, &bExists );
   else
      ret = sFios.mMainScheduler->fileExistsSync( NULL, fiosPath, &bExists );
#else
   ret = sFios.mMainScheduler->fileExistsSync( NULL, fiosPath, &bExists );
#endif
   if( ret == cell::fios::CELL_FIOS_NOERROR && bExists)
   {
      return 1;
   }

   return 0;
}

//----------------------------------------------------------------------------
#endif //BP_PS3
