//----------------------------------------------------------------------------
// BP_FileSupportX360.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "Engine/Math/BPETypeConversion.inl"
#include "Engine/Resource/CResourceManager.h"
#include "Engine/System/COsContext.h"

#include "BP_FileSupport.h"
// Include for global disk build 'enable'.
#include "../../../Builds/DiskBuilds/UseDiskBuild.h"

#ifdef BP_360

#include "Engine/ArchiveReader/CArchiveReader.h"

//----------------------------------------------------------------------------

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
   static char sRootPath[ 256 ];
#if MGS_VERSION==2
   strcpy( sRootPath, "D:\\mgs2\\" );
#else
   strcpy( sRootPath, "D:\\mgs3\\" );
#endif
   return sRootPath;
#else
   return gBP_X360PlatformRoot;
#endif
}

#if MGS_VERSION==2
#define BP_ARCHIVE_NAME         "mgs2"
#elif MGS_VERSION==3
#define BP_ARCHIVE_NAME         "mgs3"
#endif

//----------------------------------------------------------------------------

namespace
{
   CArchiveReader gArchiveReader;
}

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

//----------------------------------------------------------------------------

static void bp_fixup_final_path( char * const finalPath, const char * const path )
{
   char modifiedPath[FILENAME_MAX] = "";

   //volume path appended by archiver layer now.
   strcpy( finalPath, path );
   _str_tolower( finalPath );
}

//----------------------------------------------------------------------------

void BP_StartCommonArchiveReadThread()
{
   gArchiveReader.StartCommonArchiveInitializeThread();
}

//----------------------------------------------------------------------------

extern "C" void BP_InitSoundEffectOverride();   //TED - TEMP!!!

void BP_InitializeFileSupport()
{
   gArchiveReader.Initialize();
   char archivePath[FILENAME_MAX];

   const char * regionExt = "";
   switch( OsContext()->mBuildSKU )
   {
   case COsContext::kBS_Japan:
      regionExt = "_jp";
      break;
   case COsContext::kBS_Europe:
      regionExt = "_eu";
      break;
   }

   for( int i=0; i < 4; ++i )
   {
      bool isCommonArchive = false;
      if( i == 0 )
      {
         sprintf( archivePath, "%s\\" BP_ARCHIVE_NAME "%s.xbarc" , CResourceManager::GetRepositoryPath(), regionExt );
      }
      else if( i == 1 )
      {
         sprintf( archivePath, "%s\\" BP_ARCHIVE_NAME "_%d%s.xbarc" , CResourceManager::GetRepositoryPath(), i, regionExt );
      }
      else if( i == 2 )
      {
         sprintf( archivePath, "%s\\" BP_ARCHIVE_NAME "_misc%s.xbarc" , CResourceManager::GetRepositoryPath(), regionExt );
      }
      else if( i == 3 )
      {
         sprintf( archivePath, "%s\\" BP_ARCHIVE_NAME "%s_common.xbarc" , CResourceManager::GetRepositoryPath(), regionExt );
         isCommonArchive = true;
      }
      bool ret = gArchiveReader.OpenArchive( archivePath, isCommonArchive );
      if( ret )
      {
         printf("BP_InitializeFileSupport: Added archive '%s'.\n", archivePath );
         //Archive reader good to go.
      }
      else
      {
         printf("BP_InitializeFileSupport: Archive not found: '%s'.\n", archivePath );
      }
      //BP_TODO: remove this TGS hack and load the sound effect override data asynchronously *after* the common archive is done loading.
      //Any (few) sound effects used by the title stage that need overrides can be built into the executable.
      if( i == 2 )
      {
         //Misc archive was just loaded.  This is the place to do a synchronous load of all our sound effect override data.
         BP_InitSoundEffectOverride();
      }
   }
}

void BP_ShutdownFileSupport()
{
}


int BP_IsPsarcMounted()
{
   return gArchiveReader.HasArc();
}

void BP_IO_FlushRAMDiskCache()
{
}

void * BP_OpenFile( const char * const path, const char * const commonPath )
{
   char finalPath[FILENAME_MAX];
   bp_fixup_final_path( finalPath, path );

   char finalCommonPath[FILENAME_MAX];
   char* pfinalCommonPath = NULL;
   if( commonPath )
   {
      bp_fixup_final_path( finalCommonPath, commonPath );
      pfinalCommonPath = finalCommonPath;
   }   
   
   CArchiveFileHandle * fp = gArchiveReader.OpenFile(finalPath, pfinalCommonPath);
   BPE_ASSERT( fp != 0, "Failed to open file!" );
   return fp;
}

void BP_CloseFile( void * const fp )
{
   BPE_ASSERT( fp != NULL, "" );

   gArchiveReader.CloseFile((CArchiveFileHandle *)fp);
}

void BP_SeekFile( void * const fp, const long offset, const int mode )
{
   BPE_ASSERT( fp != NULL, "" );

   CArchiveFileHandle * fpArchive = (CArchiveFileHandle *)fp;
   fpArchive->Seek(offset, mode);
}

long BP_TellFile( void * const fp )
{
   BPE_ASSERT( fp != NULL, "" );

   CArchiveFileHandle * fpArchive = (CArchiveFileHandle *)fp;
   return fpArchive->Tell();
}

long BP_GetFileSize( void * const fp )
{
   BPE_ASSERT( fp != NULL, "" );
   CArchiveFileHandle * fpArchive = (CArchiveFileHandle *)fp;
   return fpArchive->GetFileSize();
}

void BP_ReadFile( void * const fp, void * const pBuf, const size_t size )
{
   BPE_ASSERT( fp != NULL, "" );
   CArchiveFileHandle * fpArchive = (CArchiveFileHandle *)fp;
   int ret = gArchiveReader.ReadFile( fpArchive, pBuf, size);
   BPE_ASSERT( ret == 0, "Read failure!" );
}

void BP_ReadFileByFilename( const char * const path, void * const pBuf, const long offset, const size_t size )
{
   char finalPath[FILENAME_MAX];
   bp_fixup_final_path( finalPath, path );

   CArchiveOp * pOp = gArchiveReader.ReadFileAsync( finalPath, pBuf, offset, size);
   while( !BP_TryFinishFileOp( pOp ) )
   {
      Sleep(1);
   }
}

void * BP_ReadFileAsync( void * const fp, void * const pBuf, const size_t size )
{
   CArchiveFileHandle * fpArchive = (CArchiveFileHandle *)fp;
   CArchiveOp * pOp =gArchiveReader.ReadFileAsync( fpArchive, pBuf, size);
   return pOp;
}

void * BP_ReadFileByFilenameAsync( const char * const path, void * const pBuf, const long offset, const size_t size )
{
   char finalPath[FILENAME_MAX];
   bp_fixup_final_path( finalPath, path );

   CArchiveOp * pOp = gArchiveReader.ReadFileAsync( finalPath, pBuf, offset, size);
   return pOp;
}

int BP_TryFinishFileOp( void * const pOp )
{
   CArchiveOp * pArchveOp = (CArchiveOp*)pOp;
   if( gArchiveReader.IsOpComplete( pArchveOp ) )
   {
      gArchiveReader.DeleteOp( pArchveOp );
      return 1;
   }

   return 0;
}

void BP_CancelFileOp( void * const pOp )
{
   // Note: Cancel does nothing on 360.  Anything calling cancel still needs to wait
   // for completion anyway.
   // TED NOTE: It would be tricky to cancel
   // the asynchronous read as well as potentially the decompression task associated with it
   // without introducing bugs.  This is a low-impact missing feature, as the only thing that
   // spinlocks on outstanding file ops is the survival viewer being opened. (it loops on cdbios_get_status()
   // and delays the main thread in ActSetup() )  The only potential outstanding file ops in question
   // seem to be vox streams that *just* started.
   // Just let the op complete normally!
   
   BPE_ASSERT( pOp != NULL, "" );
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
   BP_TODO_BREAK; //doesn't work on XBOX360!
   char finalPath[FILENAME_MAX];
   bp_fixup_final_path( finalPath, path );

   WIN32_FILE_ATTRIBUTE_DATA fileAttr = { 0 };
   BOOL b = GetFileAttributesExA( finalPath, GetFileExInfoStandard, &fileAttr );
   if( !b )
   {
      printf("Warning: BP_GetFileModificationDate (%s) failed\n", finalPath );
   }
   _ULARGE_INTEGER lastWriteTime;
   lastWriteTime.HighPart = fileAttr.ftLastWriteTime.dwHighDateTime;
   lastWriteTime.LowPart = fileAttr.ftLastWriteTime.dwLowDateTime;
   return lastWriteTime.QuadPart;
}

long BP_GetFileSizeAttr( const char * const path, const char * const commonPath )
{
   char finalPath[FILENAME_MAX];
   bp_fixup_final_path( finalPath, path );

   return gArchiveReader.GetFileSizeAttr( finalPath, commonPath );
}

int BP_FileExists( const char * const path, const char * const commonPath )
{
   char finalPath[FILENAME_MAX];
   bp_fixup_final_path( finalPath, path );
   
   char finalCommonPath[FILENAME_MAX];
   char* pfinalCommonPath = NULL;
   if( commonPath )
   {
      bp_fixup_final_path( finalCommonPath, commonPath );
      pfinalCommonPath = finalCommonPath;
   }   

   bool bExists = gArchiveReader.FileExists( finalPath, pfinalCommonPath );
   return bExists ? 1 : 0;
}

//----------------------------------------------------------------------------
#endif //BP_X360
