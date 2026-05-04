//----------------------------------------------------------------------------
// BP_FileSupportPS3.cpp
//----------------------------------------------------------------------------

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/StlExtras/BPEStlExtras.h"
#include "Engine/System/COsContext.h"

#include <fios2/fios2_all.h>

#include "BP_FileSupport.h"


//----------------------------------------------------------------------------

//BP - Code for handling different filename paths based on disc or Target Manager hosting

//#define BP_FIOS_LOAD_NEWER_FROM_FS //BP - Comment in to load files from file system rather than psarc when newer

#ifndef BP_TEXTURE_TOOL_LOAD_TGAS
#ifndef BP_FIOS_LOAD_NEWER_FROM_FS

#define BP_FIOS_USE_RAM_CACHE
//#define BP_FIOS_USE_HDD_CACHE

#endif
#endif

//#define _BP_DISK_BUILD   //BP - Comment in to enable running off BD

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
#if MGS_VERSION==2
   static char const * sRootPath = "/dev_bdvd/PS3_GAME/USRDIR/MGS2/";
#else
   static char const * sRootPath = "/dev_bdvd/PS3_GAME/USRDIR/MGS3/";
#endif
#else
   static char const * sRootPath = "/app_home/";
#endif
   return sRootPath;
}

#if MGS_VERSION==2
#define PSP2_ARCHIVE_NAME "mgs2"
#elif MGS_VERSION==3
#define PSP2_ARCHIVE_NAME "mgs3"
#endif

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
   printf("BP_FILESYSTEM FIOS ERROR: 0x%08x\n", errCode );
   BP_TODO_BREAK;
   // END TODO
#if 0
   //Immediately mute all sound to avoid streaming sound errors while we wait.
   BP_MultiStream_SetMasterVol(0.f);
   while (true)
   {
      // ClearAndSwapForOSD will check for game termination and perform shutdown.
      RenderBackend()->ClearAndSwapForOSD();
      //      int ret =
      cellSysutilCheckCallback();
      sys_timer_usleep(500);
   }
#endif
}

//----------------------------------------------------------------------------
// FIOS interface

namespace
{
#if 0 //TODO
   const uint64_t skHDDCacheDiscIDPerSKU[] =
   {
      0x3BA65FA89BF0057FULL,  //USA
      0x26A5FD4171DD391BULL,  //Japan
      0xA1FC679E3D6DF560ULL,  //Europe
      0x5959C52AB368A968ULL,  //UK
   };

   BPE_CTASSERT( BPE_ARRAY_SIZE( skHDDCacheDiscIDPerSKU ) == COsContext::kBS_Count );
#endif

   static char const *skAppPackageMountPoint = "app0:";
   static char const *skArchiveMountPoint = "/archive";

   inline void SceOk( int i )
   {
      BPE_ASSERT( i == 0, "Bad return result" );
   }

   inline int SceNoError( int i )
   {
      BPE_ASSERT( i >= 0, "Bad return result" );

      return i;
   }

   void fix_fios_name( char *outfile, char const *infile );

   class CFiosInitializer
   {
   public:
      CFiosInitializer()
      {
         memset( mpMountBuffers, 0, sizeof( mpMountBuffers ) );
      }

      ~CFiosInitializer()
      {
         Unmount();

      }

      bool IsMounted() const { return mpMountBuffers[0] != NULL; }
         
      bool Mount( char const *_filename, char const *_patchfilename );

      void Unmount();

      bool OpenFile( char const *filename, SceFiosFH *handle ) const;
   private:
      enum EMountFiles
      {
         kMF_Primary,
         kMF_Patch1,

         kMF_Count
      };

      SceFiosFH mMountFileHandles[kMF_Count];
      void *mpMountBuffers[kMF_Count];
   };

   bool CFiosInitializer::Mount( char const *_filename, char const *_patchfilename )
      {
         Unmount();

         char const *mountFiles[kMF_Count] = { _filename, _patchfilename };

         for ( int mountIndex = 0; mountIndex < kMF_Count; ++mountIndex )
         {  
            char const *filename = mountFiles[mountIndex];
            SceFiosSize mountBufferSize = sceFiosArchiveGetMountBufferSizeSync( NULL, filename, NULL );

            // If we can't get a mount buffer size then the archie doesn't exist
            // Stop here and don't try to continue
            if ( mountBufferSize < 0 )
            {
               break;
            }

            mpMountBuffers[mountIndex] = malloc( mountBufferSize );

            SceFiosBuffer buffer;
            buffer.set( mpMountBuffers[mountIndex], mountBufferSize );

            SceOk( sceFiosArchiveMountSync( NULL, &mMountFileHandles[mountIndex], filename, skArchiveMountPoint, buffer, NULL ) );

            printf( "******* Archive: %s(%d) for KP assets *******\n", filename, mountIndex );
         }

         // If we couldn't mount the first package, then die
         if ( mpMountBuffers[0] == NULL )
         {
            return false;
         }
         else
         {
            return true;
         }
      }

   void CFiosInitializer::Unmount()
      {
         if ( !IsMounted() )
         {
            return;
         }

         for ( int mountIndex = 0; mountIndex < kMF_Count; ++mountIndex )
         {  
            if ( mpMountBuffers[mountIndex] )
            {
               SceOk( sceFiosArchiveUnmountSync( NULL, mMountFileHandles[mountIndex] ) );
               free( mpMountBuffers[mountIndex] );
               mpMountBuffers[mountIndex] = NULL;
            }
         }
      }

   bool CFiosInitializer::OpenFile( char const *filename, SceFiosFH *handle ) const
      {
         char fixedName[ FILENAME_MAX ];

         fix_fios_name( fixedName, filename );

         int ret = sceFiosFHOpenSync( NULL, handle, fixedName, NULL );

         if ( ret != SCE_FIOS_OK )
         {
            return false;
         }
         else
         {
            return true;
         }
      }

   CFiosInitializer sFios;

   void fix_fios_name_with_mountpoint( char *outfile, char const *infile, char const *mountPoint )
   {
      if ( !strncmp( mountPoint, infile, strlen( mountPoint ) ) ) 
      {
         strcpy(outfile, infile);
         _str_tolower( outfile );
      }
      else
      {
         sprintf( outfile, "%s/%s", 
            mountPoint,
            infile );
         _str_tolower( outfile );
      }
   }

   void fix_fios_name( char *outfile, char const *infile  )
   {
      // Files loaded by the game that are supposed to come from the BP misc. archive must be prefixed with the 
      // following string.
      char const* const kBPArchiveNamePrefix = "misc/";

      if ( !strncasecmp(kBPArchiveNamePrefix, infile, strlen(kBPArchiveNamePrefix)) )
      {
         fix_fios_name_with_mountpoint( outfile, infile, OsContext()->GetRootFolder() );
      }
      else
      {
         char const* mountPoint = sFios.IsMounted() ? skArchiveMountPoint : skAppPackageMountPoint;
         fix_fios_name_with_mountpoint( outfile, infile, mountPoint );
      }
   }

}
//----------------------------------------------------------------------------
extern "C" void BP_InitSoundEffectOverride();   //TED - TEMP!!!

static void build_and_fix_fios_path_for_kp_archive( char *fiosPath, char const *archivePostPend, char const *mountpoint )
{
   char psp2ArcPath[FILENAME_MAX];

   strcpy(psp2ArcPath, PSP2_ARCHIVE_NAME);
   strcat(psp2ArcPath, archivePostPend );

   switch( OsContext()->mBuildSKU )
   {
   case COsContext::kBS_Japan:
      strcat(psp2ArcPath, "_jp" );
      break;
   case COsContext::kBS_Europe:
      strcat(psp2ArcPath, "_eu" );
      break;
   }

   strcat(psp2ArcPath, ".psp2arc");

   fix_fios_name_with_mountpoint( fiosPath, psp2ArcPath, mountpoint );
}

void BP_InitializeFileSupport()
{
   static char const * const skDeviceOrder[] = 
   {
      skAppPackageMountPoint,
   };

   bool anyMountWorked = false;

   for ( int i = 0; i < BPE_ARRAY_SIZE( skDeviceOrder ); ++i )
   {
      char fiosPathUnpatch[FILENAME_MAX];
      char fiosPathPatch[FILENAME_MAX];

      build_and_fix_fios_path_for_kp_archive( fiosPathUnpatch, "", skDeviceOrder[i] );
      build_and_fix_fios_path_for_kp_archive( fiosPathPatch, "d", skDeviceOrder[i] );

      if ( sFios.Mount(fiosPathUnpatch, fiosPathPatch) )
      {
         anyMountWorked = true;
         break;
      }
   }

   if ( !anyMountWorked )
   {
      printf( "******* Archive: NONE for KP assets - Loose files only *******\n" );
   }

   BP_InitSoundEffectOverride();
}

void BP_ShutdownFileSupport()
{
}

int BP_IsPsarcMounted()
{
   return sFios.IsMounted();
}

void BP_IO_FlushRAMDiskCache()
{
}

static inline SceFiosFH bp_ptr_to_fios_handle( void *ptr )
{
   int handle = reinterpret_cast<int>( ptr );

   return SceFiosFH( handle - 1 );
}

static inline void *fios_op_to_bp_ptr( SceFiosOp handle )
{
   int int_handle = handle;
   return reinterpret_cast<void *>( int_handle + 1 );
}

static inline SceFiosOp bp_ptr_to_fios_op( void *ptr )
{
   int handle = reinterpret_cast<int>( ptr );

   return SceFiosOp( handle - 1 );
}

static inline void *fios_handle_to_bp_ptr( SceFiosFH handle )
{
   int int_handle = handle;

   return reinterpret_cast<void *>( int_handle + 1 );
}

void * BP_OpenFile( const char * const path, const char * const commonPath )
{
   SceFiosFH handle = -1;
   
   if ( sFios.OpenFile( path, &handle ) )
   {
      return fios_handle_to_bp_ptr( handle );
   }
   else
   {
      BPE_ASSERTA( "Bad file handle" );
      return NULL;
   }
}

void * BP_OpenFileReadWrite( const char * const path, int truncate )
{
   BP_VITA_TODO_BREAK;
   return NULL;
}

void BP_CloseFile( void * const fp )
{
   BPE_ASSERT( fp != NULL, "" );
   SceFiosFH handle = bp_ptr_to_fios_handle( fp );

   SceOk( sceFiosFHCloseSync( NULL, handle ) );
}

void BP_SeekFile( void * const fp, const long offset, const int mode )
{
   BPE_ASSERT( fp != NULL, "" );
   SceFiosFH handle = bp_ptr_to_fios_handle( fp );

   SceNoError( sceFiosFHSeek( handle, offset, SceFiosWhence( mode ) ) );
}

long BP_TellFile( void * const fp )
{
   BPE_ASSERT( fp != NULL, "" );
   SceFiosFH handle = bp_ptr_to_fios_handle( fp );

   return SceNoError( sceFiosFHTell( handle ) );
}

long BP_GetFileSize( void * const fp )
{
   BPE_ASSERT( fp != NULL, "" );
   SceFiosFH handle = bp_ptr_to_fios_handle( fp );

   return SceNoError( sceFiosFHGetSize( handle ) );
}

void BP_WriteFile( void* const fp, void const * pBuf, const size_t size )
{
   BPE_ASSERT( fp != NULL, "" );
   SceFiosFH handle = bp_ptr_to_fios_handle( fp );

   SceNoError( sceFiosFHWriteSync( NULL, handle, pBuf, size ) );
}

void BP_ReadFile( void * const fp, void * const pBuf, const size_t size )
{
   BPE_ASSERT( fp != NULL, "" );
   SceFiosFH handle = bp_ptr_to_fios_handle( fp );

   SceFiosSize readSize = sceFiosFHReadSync( NULL, handle, pBuf, size );

   SceNoError( readSize );
}

void BP_ReadFileByFilename( const char * const path, void * const pBuf, const long offset, const size_t size )
{
   void *fp = BP_OpenFile( path, NULL );

   BPE_ASSERT( fp != NULL, "couldn't open file  in BP_ReadFileByFilename" );
   if ( fp )
   {
      BP_SeekFile( fp, offset, SEEK_SET );
      BP_ReadFile( fp, pBuf, size );
      BP_CloseFile( fp );
   }
}

void * BP_ReadFileAsync( void * const fp, void * const pBuf, const size_t size )
{
   BPE_ASSERT( fp != NULL, "" );
   SceFiosFH handle = bp_ptr_to_fios_handle( fp );

   SceFiosOp op = sceFiosFHRead( NULL, handle, pBuf, size );
   SceNoError( op );

   return fios_op_to_bp_ptr( op );
}

void * BP_ReadFileByFilenameAsync( const char * const path, void * const pBuf, const long offset, const size_t size )
{
   char fiosPath[FILENAME_MAX];

   fix_fios_name( fiosPath, path );

#ifndef GOLD_VERSION
   if ( !sceFiosFileExistsSync( NULL, fiosPath ) )
   {
      printf("BP_ReadFileByFilenameAsync: file not found %s\n", path );
      BP_BREAK;
   }
#endif
   // Fix the FIOS path
   SceFiosOp op = sceFiosFileRead( NULL, fiosPath, pBuf, size, offset );
   SceNoError( op );

   return fios_op_to_bp_ptr( op );
}

int BP_TryFinishFileOp( void * const pOp )
{
   BPE_ASSERT( pOp != NULL, "" );
   SceFiosOp op = bp_ptr_to_fios_op( pOp );

   if ( !sceFiosOpIsDone( op ) )
   {
      return 0;
   }
   
   sceFiosOpDelete( op );
   return 1;
}

void BP_CancelFileOp( void * const pOp )
{
   BPE_ASSERT( pOp != NULL, "" );
   SceFiosOp op = bp_ptr_to_fios_op( pOp );

   sceFiosOpCancel( op );
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
   SceFiosStat stat = { 0 };

   if ( void *fp = BP_OpenFile( path, NULL ) )
   {
      SceOk( sceFiosFHStatSync( NULL, bp_ptr_to_fios_handle( fp ), &stat ) );

      BP_CloseFile( fp );
   }
   
   return stat.modificationDate;
}

long BP_GetFileSizeAttr( const char * const path, const char * const commonPath )
{
   void *fp = BP_OpenFile( path, commonPath );
   long size = BP_GetFileSize( fp );
   BP_CloseFile( fp );

   return size;
}

int BP_FileExists( const char * const path, const char * const commonPath )
{
   char fiosPath[FILENAME_MAX];
   fix_fios_name( fiosPath, path );

   return sceFiosFileExistsSync( NULL, fiosPath );
}

