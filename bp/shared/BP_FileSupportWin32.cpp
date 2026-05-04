//----------------------------------------------------------------------------
// BP_FileSupportWin32.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "Engine/Math/BPETypeConversion.inl"

#include "BP_FileSupport.h"

#ifdef BP_WIN32

#include <windows.h>

//----------------------------------------------------------------------------

extern "C" void BP_InitSoundEffectOverride();   //TED - TEMP!!!

void BP_InitializeFileSupport()
{
   //BP_TODO: remove this TGS hack and load the sound effect override data asynchronously *after* the common archive is done loading.
   //Any (few) sound effects used by the title stage that need overrides can be built into the executable.
   BP_InitSoundEffectOverride();
}

void BP_ShutdownFileSupport()
{
}

int BP_IsPsarcMounted()
{
   return 0;
}

void BP_IO_FlushRAMDiskCache()
{
}

void * BP_OpenFile( const char * const path, const char * const commonPath )
{
   FILE * fp = fopen( path, "rb" );
   BPE_ASSERT( fp != 0, "Failed to open file!" );
   return fp;
}

void * BP_OpenFileReadWrite( const char * const path, int truncate )
{
   FILE * fp = 0;
   if ( truncate )
   {
      fp = fopen( path, "wb" );
   }
   else
   {
      if ( BP_FileExists( path, NULL ) )
         fp = fopen( path, "r+b" );
      else
         fp = fopen( path, "wb" );
   }
   BPE_ASSERT( fp != 0, "Failed to open file!" );
   return fp;
}

void BP_CloseFile( void * const fp )
{
   BPE_ASSERT( fp != NULL, "" );
   fclose( (FILE*)fp );
}

void BP_SeekFile( void * const fp, const long offset, const int mode )
{
   BPE_ASSERT( fp != NULL, "" );
   fseek( (FILE*)fp, offset, mode );
}

long BP_TellFile( void * const fp )
{
   BPE_ASSERT( fp != NULL, "" );
   return ftell( (FILE*)fp );
}

long BP_GetFileSize( void * const fp )
{
   BPE_ASSERT( fp != NULL, "" );
   long was_at = ftell((FILE*)fp);
   BP_SeekFile( fp, 0L, SEEK_END );
   long sz = ftell((FILE*)fp);
   BP_SeekFile( fp, was_at, SEEK_SET );
   return sz;
}

void BP_WriteFile( void * const fp, void const * pBuf, const size_t size )
{
   BPE_ASSERT( fp != NULL, "" );
   int n = fwrite(pBuf, size, 1, (FILE*)fp);
}

void BP_ReadFile( void * const fp, void * const pBuf, const size_t size )
{
   BPE_ASSERT( fp != NULL, "" );
   int n = fread(pBuf, size, 1, (FILE*)fp);
}

void BP_ReadFileByFilename( const char * const path, void * const pBuf, const long offset, const size_t size )
{
   void * op = BP_ReadFileByFilenameAsync( path, pBuf, offset, size );
   if( !BP_TryFinishFileOp( op ) )  //sync on windows :)
      BP_BREAK;
}

void * BP_ReadFileAsync( void * const fp, void * const pBuf, const size_t size )
{
   int n = fread(pBuf, size, 1, (FILE*)fp);
   return fp;  //arbitrary non-NULL pointer value
}

void * BP_ReadFileByFilenameAsync( const char * const path, void * const pBuf, const long offset, const size_t size )
{
   FILE * fp = fopen( path, "rb" );
   BPE_ASSERT( fp != 0, "Failed to open file!" );
   fseek( fp, offset, SEEK_SET );
   int n = fread(pBuf, size, 1, fp);
   fclose( fp );
   return fp;  //arbitrary non-NULL pointer value
}

int BP_TryFinishFileOp( void * const pOp )
{
   //operation completed synchronously on Windows
   return 1;
}

void BP_CancelFileOp( void * const pOp )
{
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
   WIN32_FILE_ATTRIBUTE_DATA fileAttr = { 0 };
   BOOL b = GetFileAttributesExA( path, GetFileExInfoStandard, &fileAttr );
   if( !b )
   {
      printf("Warning: BP_GetFileModificationDate (%s) failed\n", path );
   }
   _ULARGE_INTEGER lastWriteTime;
   lastWriteTime.HighPart = fileAttr.ftLastWriteTime.dwHighDateTime;
   lastWriteTime.LowPart = fileAttr.ftLastWriteTime.dwLowDateTime;
   return lastWriteTime.QuadPart;
}

long BP_GetFileSizeAttr( const char * const path, const char * const commonPath )
{
   WIN32_FILE_ATTRIBUTE_DATA fileAttr = { 0 };
   BOOL b = GetFileAttributesExA( path, GetFileExInfoStandard, &fileAttr );
   if( !b )
   {
      printf("Warning: BP_GetFileSizeAttr (%s) failed\n", path );
   }
   _ULARGE_INTEGER byteSize;
   byteSize.HighPart = fileAttr.nFileSizeHigh;
   byteSize.LowPart = fileAttr.nFileSizeLow;
   return (long)byteSize.QuadPart;
}

int BP_FileExists( const char * const path, const char * const commonPath )
{
   WIN32_FILE_ATTRIBUTE_DATA fileAttr = { 0 };
   BOOL b = GetFileAttributesExA( path, GetFileExInfoStandard, &fileAttr );
   if( !b )
   {
      return 0;
   }

   return 1;
}

//----------------------------------------------------------------------------
#endif //BP_WIN32
