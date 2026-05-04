
#ifndef __LOADER_FLATFS_H
#define __LOADER_FLATFS_H

#ifdef __cplusplus
extern "C" {
#endif

enum EFlatFSLoadState
{
   kFlatFSLoadState_Idle,
   kFlatFSLoadState_Pending,
   kFlatFSLoadState_ReadingManifest,
   kFlatFSLoadState_ReadingBPAssetsList,
   kFlatFSLoadState_ReadingBPFiles,
   kFlatFSLoadState_ReadingKPFiles,

   kFlatFSLoadState_SyncPendingBPFiles,
};

struct SFlatFSLoadFileState
{
   char              mUnifiedPath[FILENAME_MAX];
   char              mLoadPath[FILENAME_MAX];
   char              mStagePath[FILENAME_MAX];
   void *            mpFileHandle;
   void *            mpFileOp;
   char *            mpFileData;
   int               mFileSize;
};

#define FLATFS_MAX_LOAD_FILES 32

struct SFlatFSLoadState
{
   EFlatFSLoadState   mLoadState;
   unsigned int      mAssetCacheTag;

   char              mManifestPath[FILENAME_MAX];
   char *            mpManifestData;
   char *            mpCurrentManifestLine;

   char              mBPAssetsListPath[FILENAME_MAX];
   char *            mpBPAssetsData;
   char *            mpCurrentBPAssetsLine;

   SFlatFSLoadFileState
                     mLoadingFiles[ FLATFS_MAX_LOAD_FILES ];
   int               mFileLoadingHead;
   int               mFileLoadingCount;

};

typedef void ( *BP_BeginLoadKPAssetCallback )( int const fileStateIndex, SFlatFSLoadState * pState );
typedef void ( *BP_EndLoadKPAssetCallback )( int const fileStateIndex, SFlatFSLoadState * pState );

void BP_BeginLoadFlatFS( SFlatFSLoadState * pState );
int BP_LoadFlatFSSync( SFlatFSLoadState * pState, BP_BeginLoadKPAssetCallback begin_load_kp_file, BP_EndLoadKPAssetCallback end_load_kp_file );
void BP_LoadCompleteFlatFS( SFlatFSLoadState * pState );

// Frees the flat fs state except for filedata.  If you allocated the returned pointer,
// free it yourself.  If not, call BP_FreeTemporaryFlatFSFileData
void BP_CancelLoadFlatFSReturnFileData( SFlatFSLoadState *pState, void **ppFileDatas, int *pFileDatasCount );
void BP_IgnoreThisTemporaryData( void *ptr, void **ppFileDatas, int fileDatasCount );

void BP_FreeTemporaryFlatFSFileData( void **ppFileDatas, int fileDatasCount );

int BP_SplitManifestLine( char const *manifestLine, char const *manifestRoot, char *unifiedPath, char *loadPath, char *relativePath );
void BP_GetAssetLoadFullPath( char * const fullPath, char * const unifiedFullPath, const char * const manifestLoadPath, const char * const manifestUnifiedPath );

#ifdef __cplusplus
}
#endif

#endif
