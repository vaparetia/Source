//----------------------------------------------------------------------------
// BP_FileSupport.h
//----------------------------------------------------------------------------
// IMPORTANT:
// This file will be included by C code, as such it MUST stay free of C++ of any sort.
//----------------------------------------------------------------------------

#include "BP_BuildDefines.h"
#include <stdio.h>
//----------------------------------------------------------------------------

#pragma once

#ifdef BP_PS3
#  define BP_PATH_SEPARATOR "/"
#  define BP_PLATFORM_FOLDER "_ps3"
#elif defined( BP_WIN32 )
#  define BP_PATH_SEPARATOR "/"
#  define BP_PLATFORM_FOLDER "_win"
#elif defined( BP_360 )
#  define BP_PATH_SEPARATOR "/"
#  define BP_PLATFORM_FOLDER "_360"
#elif defined( BP_VITA )
#  define BP_PATH_SEPARATOR "/"
#  define BP_PLATFORM_FOLDER "_vta"
#else
# error Set platform defines!
#endif

#define BP_REBUILTDATA_FOLDER "_bp"

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------

extern char * gBP_X360PlatformRoot;

void BP_BeginLoadData(const unsigned int assetCacheTag, const char* folderPath);
void BP_EndLoadData();
unsigned int BP_GetCurrentAssetCacheTag();
const char * BP_GetCurrentLoadPath();
char *BP_FGetsWinFiles( char *buf, int max_count, FILE *fp );
void BP_UnifyPath( char * path );
void BP_InsertPlatformSubfolder( char * path );

int BP_IsDiskBuild();
char const * BP_GetRootFilesystemPath();

#ifdef BP_360
void BP_StartCommonArchiveReadThread();
#endif
void BP_InitializeFileSupport();
void BP_ShutdownFileSupport();

int BP_IsPsarcMounted();
void BP_InitTextureConversionList();

void BP_IO_FlushRAMDiskCache();
void * BP_OpenFile( const char * const path, const char * const commonPath );
void * BP_OpenFileReadWrite( const char * const path, int truncate );
void BP_CloseFile( void * const fp );
void BP_SeekFile( void * const fp, const long offset, const int mode );
long BP_TellFile( void * const fp );
long BP_GetFileSize( void * const fp );
void BP_WriteFile( void * const fp, void const * pBuf, const size_t size );
void BP_ReadFile( void * const fp, void * const pBuf, const size_t size );
void BP_ReadFileByFilename( const char * const path, void * const pBuf, const long offset, const size_t size );

void * BP_ReadFileAsync( void * const fp, void * const pBuf, const size_t size );
void * BP_ReadFileByFilenameAsync( const char * const path, void * const pBuf, const long offset, const size_t size );
int BP_TryFinishFileOp( void * const pOp );
void BP_CancelFileOp( void * const pOp );
void BP_CancelAndDeleteFileOpSync( void * const pOp );

u_long64 BP_GetFileModificationDate( const char * const path );
long BP_GetFileSizeAttr( const char * const path, const char * const commonPath );
int BP_FileExists( const char * const path, const char * const commonPath );

void BP_Init_FS_Region();
void BP_LoadStreamRemappingTables();
int BP_GetRemappedStreamNameAndLoc( const int fileIndex, const unsigned int orgLsn, const int orgStreamTop, char * outFullPath );
const char * BP_FindStreamName( const int pos );

void BP_GetPathPlatformSKUOverride(char * loadPath, char * commonLoadPath);

// Used by the vox sound test to get an array of all the vox streams. (Only used for testing purposes!!)
int BP_GetNumFileEntriesFromRemapTable(int const fileIndex);
void BP_GetRemapTableEntries(int const fileIndex, unsigned int* pOffsets, char const** ppStreamNames);

//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif
