#if MGS_VERSION==3
#include "Engine/Stdafx.h"

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "BP_BaseRenderer.h"

#include "MGS_Common.h"

#include "def_ext.h"
#include "ext_table.cnf"
#include "libgv.h"
#include "datacnf_tag.h"

#elif MGS_VERSION==2

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bp_matrix.h"
#include "MGS_SysCommon.h"

#include "BP_BaseRenderer.h"
#include "Engine/System/CStopWatch.h"

#endif

#include "libfs.h"

#include "stdio.h"
#include "ctype.h"

#include "BP_FileSupport.h"

#if defined(BP_VITA)
#include "BP_UnifiedSDX.h"
#endif

#define BP_INVALID_TAG_CODE 0xffffffff

#include "loader_flatfs.h"

extern int FS_SlotPackLoadSync( void );

#if MGS_VERSION == 2

extern "C" int sd_set_cli(int);
#define GV_MallocAlign(SIZE, ALIGN) GV_AllocMemory(GV_NORMAL_MEMORY, NULL, SIZE, ALIGN)
#define EXT_ID_tri ('t' - 'a')

#endif

//----------------------------------------------------------------------------

struct SLoadInfo
{
   char mSubFolder[ 64 ];
};

struct SSectionMapping
{
   char const *name;
   enum loadfunc_mode type;
};

static struct SSectionMapping const skSecMapping[] =
{ 
   { "resident", GV_INIT_RESIDENT },
   { "cache", GV_INIT_CACHE },
   { "nocache", GV_INIT_NOCACHE },
   { NULL, (enum loadfunc_mode) 0 }
};

static SFlatFSLoadState sFlatFSLoadState = { kFlatFSLoadState_Idle };

struct SStageLoadFileState
{
   unsigned int      mFileId;
   enum loadfunc_mode mInitType;
   char mBPLoadDataPath[FILENAME_MAX];
};

struct SStageLoadState
{
   struct SLoadInfo *mpLoadInfo;
   SStageLoadFileState mFileStates[ FLATFS_MAX_LOAD_FILES ];
};

static SStageLoadState sStageLoadState = { 0 };
extern "C" char gCurrStageSoundDir[];

static const unsigned int kStageAssetCacheTag = 0; //fixed to 0 for stages.

//----------------------------------------------------------------------------

// Gets the mode for a section's path, returning where the filename starts
// Returns -1 on failure

int get_mode_for_section_return_fname_start( char const *relativePathName, enum loadfunc_mode *pSection )
{
   char const *pSlash = strchr( relativePathName, '/' );
   int const slash = pSlash ? ( pSlash - relativePathName ) : -1;

   if ( slash == -1 )
   {
      return -1;
   }
   else
   {
      int i;

      for ( i = 0; skSecMapping[i].name; ++i )
      {
         if ( strncmp( relativePathName, skSecMapping[i].name, slash ) == 0 )
         {
            *pSection = skSecMapping[i].type;
            return slash + 1;
         }
      }

      return -1;
   }
}

static unsigned get_tag_code_for_ext( char const *ext )
{
   int i;

   // Slots are special - even thought they have an extension code, what we 
   // really want is the tag id
   if ( !strcmp( ext, "slot" ) )
   {
      return 0x7D;
   }

#if MGS_VERSION == 2
   return ext[0] - 'a';
#else
   for ( i = 0; ext_table[i].name; ++i )
   {
      if ( !strcmp( ext, ext_table[i].name ) )
      {
         return ext_table[i].id;
      }
   }
#endif

   return BP_INVALID_TAG_CODE;
}

static unsigned decode_hex_fname( char const *fname )
{
   unsigned ret = 0;
   unsigned i;

   // Must be at least 8 chars and have a .
   if ( strlen( fname ) <= 8 || fname[8] != '.' )
   {
      return 0;
   }

   for ( i = 0; i < 8; ++i )
   {
      unsigned nibble;
      char ch = tolower( fname[i] );
      if ( ch >= '0' && ch <= '9' ) 
      {
         nibble = ch - '0';
      }
      else if ( ch >= 'a' && ch <= 'f' )
      {
         nibble = ch - 'a' + 10;
      }
      else
      {
         // out of bounds.  Punt
         return 0;
      }

      ret |= nibble << ( (7-i) * 4 );
   }

   return ret;
}

static inline int NEED_ALIGN( unsigned int id )
{
   int type = ( id >> 24 );

   switch( type ){
     case EXT_ID_tri:
        return 128;
   }
   return 16;
}

#if MGS_VERSION == 3

static void *AllocSlotData( int size, int id, int align, int mode )
{
   void *ptr;
   if( mode == GV_INIT_RESIDENT ){
      ptr = GV_AllocResidentMemoryAligned( size, id, align );
   } else {
      ptr = GV_MemoryAlloc( GV_NORMAL_MEMORY, size, align );
      GV_MemorySetType( ptr, GV_MEMORY_TYPE_LOAD );
   }
   if( id != 0 ){
      GV_SetFCache( id, ptr );
   }
   return ptr;
}


static void load_stage_slot( int const fileStateIndex )
{
   FS_SLOT_INFO *slot;
   void *slotBody;
   int i;
   unsigned pageCount;
   unsigned totalSize;
   enum loadfunc_mode init_mode = sStageLoadState.mFileStates[fileStateIndex].mInitType;
   unsigned const fileId = sStageLoadState.mFileStates[fileStateIndex].mFileId;
   unsigned name = SLOT_NAME( fileId );
   long fileSize = sFlatFSLoadState.mLoadingFiles[fileStateIndex].mFileSize;
   void *buf = sFlatFSLoadState.mLoadingFiles[fileStateIndex].mpFileData;

   // The whole buffer should be 0 except for the top 8 bytes
   pageCount = BP_LE_SwapUInt( ((unsigned const *)buf)[0] );
   totalSize = BP_LE_SwapUInt( ((unsigned const *)buf)[1] );
   memset( buf, 0, 8 );

   if ( init_mode == GV_INIT_RESIDENT )
   {
      printf( "SLOT RESIDENT: %8.8x\n", GV_FCACHE_ID( name, EXT_ID_slot ) );
   }

   if ( init_mode == GV_INIT_RESIDENT_AGAIN )
   {
      printf( "SLOT RESIDENT AGAIN: %8.8x\n", GV_FCACHE_ID( name, EXT_ID_slot ) );
   }

   slot = (FS_SLOT_INFO*)AllocSlotData( sizeof( FS_SLOT_INFO ) + sizeof( FS_SLOT_PAGE ) * pageCount
      , GV_FCACHE_ID( name, EXT_ID_slot ), 16, init_mode );
   memset( slot, 0, sizeof( FS_SLOT_INFO ) + sizeof( FS_SLOT_PAGE ) * pageCount );

   slot->name = name;
   slot->size = totalSize;
   slot->page_num = pageCount;

   if ( init_mode == GV_INIT_RESIDENT )
   {
      slotBody = AllocSlotData( totalSize, 0, 128, init_mode );
	  // AS(JM) - Zero out the slot body for a resident allocation, as resident 
	  // memory isn't guaranteed to be zero
      memset( slotBody, 0, totalSize );
   }
   else
   {
      slotBody = buf;
   }

   for( i = 0; i < slot->page_num; i++ ){
      slot->pages[ i ].id = 0;
      slot->pages[ i ].page = ((char *) slotBody) + ( slot->size / slot->page_num ) * i;
      if( slot->page_num > 1 ){
         *( int * )slot->pages[ i ].page = 0;
      }
   }

   // Taken from IN_STEP_SLOT part of stagezcd.c
   // DISABLED because the code was actually a dead code path that would never ever happen
   // it's setting up slot stuff in a way that's never used in any of the stage.dat files.
#if 0
   {
      void *target;

      for( i = 0; i < slot->page_num; i++ ){
         if( slot->pages[ i ].id == 0 ){
            break;
         }
      }

      // This assert is here because the logic in stagezcd makes so sense..
      // It will initialize all of the stage page id's to 0 above, then then
      // in IN_STEP_SLOT, it finds the first slot (setting the variable i here)
      // It then gets a "target" ptr, which is the pointer to the page's data.
      // In the code in stagezcd, it then copies an amount of data equal to ALL 
      // of the slots' data over the memory section.
      //
      // I believe that the code actually meant to copy all of the slot data
      // over the resident allocation, and was written poorly to look like it 
      // was trying to copy only one page.

      XASSERT( i == 0, "Slot page had data that was not expected!" );

      FS_SlotPageSetUse( &slot->pages[ i ], fileId );
      target = FS_SlotPageGetPtr( &slot->pages[ i ] );

      if( init_mode == GV_INIT_RESIDENT )
      {
         // Read comments above - The original code was the memcpy( target, buf, slot->size)
         // below, which implies that it's copying all of the slot pages into this first page.
         // (slot->size is the page's size * slot->num_page)
         //memcpy( target, buf, slot->size );
         memcpy( slotBody, buf, slot->size );
      } 
      else 
      {
         XASSERT( target == slotBody && target == buf, "Wrong slot %X %X %X!!\n", target, buf, slotBody );
      }

      FS_SlotPageInit( &( slot->pages[ i ] ), init_mode );
   }
#endif
}

#endif

static void load_stage_file( int const fileStateIndex )
{
   enum loadfunc_mode init_mode = sStageLoadState.mFileStates[ fileStateIndex ].mInitType;
   unsigned const fileId = sStageLoadState.mFileStates[ fileStateIndex ].mFileId;
   void *buf = sFlatFSLoadState.mLoadingFiles[ fileStateIndex ].mpFileData;
   long fileSize = sFlatFSLoadState.mLoadingFiles[ fileStateIndex ].mFileSize;

#if MGS_VERSION == 3
   GV_LoadInit( buf, fileId, init_mode, fileSize );
#else
   GV_LoadInit( buf, fileId, init_mode );
#endif
}

static unsigned get_stage_file_tag_id( char const *filename, int print_warn )
{
   unsigned const id_low = decode_hex_fname( filename );

   if ( id_low == 0 )
   {
      if ( print_warn )
         printf( "WARNING: Could not map %s to id\n", filename );
      return 0;
   }
   else
   {
      // filename+9 is safe because decode_hex_fname would have failed otherwise
      char const *ext = filename + 9;
      unsigned const code = get_tag_code_for_ext( ext );

      if ( code == BP_INVALID_TAG_CODE )
      {
         if ( print_warn )
            printf( "WARNING: Could not map ext for %s to id\n", filename );
         return 0;
      }

      return id_low | ( code << 24 );
   }
}

//----------------------------------------------------------------------------
// FS_CdStageFileInit
// 
// Reads in the list of stages off disk and keeps them
// around for easy look up.
//----------------------------------------------------------------------------


void FS_CdStageFileInit( int alt )
{
}

//----------------------------------------------------------------------------
// FS_LoadSoundPak
// 
//Reads in a sound pak by index
//----------------------------------------------------------------------------

#if MGS_VERSION==2
extern "C" void BP_BuildSoundPakFilename( char * path, int code );
#endif
void FS_LoadSoundPak( int code )
{
#if MGS_VERSION==3
   int		strcode;

   static int prev_sound_code = -1;
   static int prev_sound_dir  = -1;

   int curr_sound_dir = GV_StrCode(gCurrStageSoundDir);

   if((prev_sound_code == code) && (curr_sound_dir == prev_sound_dir)) return;

   prev_sound_code = code;
   prev_sound_dir  = curr_sound_dir;
   jSdSet( setSdCmd(KCEJSD_EFX_LOAD) | setSdVal0( 1 ), code, 0, 0 ); 
#elif MGS_VERSION==2

// On Vita we load from a unified archive of wav data instead of individual sdx files.

   char fullPath[FILENAME_MAX];
   BP_BuildSoundPakFilename( fullPath, code );

#if !defined(BP_VITA)
   if( BP_FileExists( fullPath, NULL ) )
   {
      printf( "FS_LOADSOUNDPAK %d: %s\n", code, fullPath );
      //BP - this function originally found the offset of the sound data of this code
      //within the dat file.  We load by filename instead.
      sd_set_cli( 0xC0000000 | code );
   }
   else
   {
      //BP - not a fatal error, they seem to have faulty scripting in some cases.
      printf( "SOUND CODE ERROR %d: %s!!\n", code, fullPath );
   }
#else
   // AS MCampbell - Need to check if the sound pak file existed in the original data by checking the unified archive 
   // manifest. This is the Vita fix for the faulty scripting issue.
   // Bug: MGSTWO-3065
   if (BP_DoesPakFileExistInUnifiedArchive(fullPath))
   {
      sd_set_cli( 0xC0000000 | code );
   }
#endif
#endif
}

//----------------------------------------------------------------------------
// FS_LoadStageSync
//
// Keep calling until it returns 0
// info is passed in from FS_LoadStageRequest
//----------------------------------------------------------------------------

static void stage_begin_load_kp_file( int const fileStateIndex, SFlatFSLoadState * pState )
{
   char folderRoot[ FILENAME_MAX ];
   sprintf( folderRoot, "%s/stage/%s/", FS_BP_GetCurrentRegionFolder(), sStageLoadState.mpLoadInfo->mSubFolder );
   SFlatFSLoadFileState *pFileState = &( pState->mLoadingFiles[ fileStateIndex ] );

   char relativePath[FILENAME_MAX];
   char unifiedPath[FILENAME_MAX];
   char loadPath[ FILENAME_MAX ];

   BP_SplitManifestLine( pState->mpCurrentManifestLine, folderRoot, unifiedPath, loadPath, relativePath );

   int fname_begin = get_mode_for_section_return_fname_start( relativePath, &sStageLoadState.mFileStates[ fileStateIndex ].mInitType );

   if ( fname_begin == -1 )
      BP_BREAK;

   char const *filename = relativePath + fname_begin;
   sStageLoadState.mFileStates[ fileStateIndex ].mFileId = get_stage_file_tag_id( filename, 1 );

   if ( sStageLoadState.mFileStates[ fileStateIndex ].mFileId == 0 )
      BP_BREAK;

   char manifestFolder[FILENAME_MAX];

   strcpy(manifestFolder, relativePath);
   manifestFolder[fname_begin] = 0;

   // Cache the load data path
   sprintf(sStageLoadState.mFileStates[ fileStateIndex ].mBPLoadDataPath, "%s/stage/%s/%s", FS_BP_GetCurrentRegionFolder(), sStageLoadState.mpLoadInfo->mSubFolder, manifestFolder );
   BP_UnifyPath(sStageLoadState.mFileStates[ fileStateIndex ].mBPLoadDataPath);

   strcpy( pFileState->mUnifiedPath, unifiedPath );
   strcpy( pFileState->mStagePath, relativePath );
   strcpy( pFileState->mLoadPath, loadPath );

//   sprintf( pState->mArchivePath, "%s%s", folderPath, filename );

   char assetLoadPath[FILENAME_MAX];
   char assetLoadPathCommon[FILENAME_MAX];
   BP_GetAssetLoadFullPath( assetLoadPath, assetLoadPathCommon, pFileState->mLoadPath, pFileState->mUnifiedPath );
   pFileState->mpFileHandle = BP_OpenFile( assetLoadPath, assetLoadPathCommon );

   pFileState->mFileSize = BP_GetFileSize( pFileState->mpFileHandle );
#if MGS_VERSION==3
   if( IS_SLOT_TAG( sStageLoadState.mFileStates[ fileStateIndex ].mFileId ) )
   {
      pFileState->mpFileData = (char*)GV_MallocAlign(pFileState->mFileSize, 128);
   }
   else
#endif
   {
      if ( sStageLoadState.mFileStates[fileStateIndex].mInitType == GV_INIT_RESIDENT )
      {
         pFileState->mpFileData = (char*)GV_AllocResidentMemoryAligned( pFileState->mFileSize, sStageLoadState.mFileStates[fileStateIndex].mFileId, NEED_ALIGN( sStageLoadState.mFileStates[fileStateIndex].mFileId ) );
      }
      else
      {
         pFileState->mpFileData = (char*)GV_MallocAlign( pFileState->mFileSize, 128 );
      }

   }
   pFileState->mpFileOp = BP_ReadFileAsync( pFileState->mpFileHandle, pFileState->mpFileData, pFileState->mFileSize );
}

static void stage_end_load_kp_file( int const fileStateIndex, SFlatFSLoadState * pState )
{
   BP_BeginLoadData( kStageAssetCacheTag, sStageLoadState.mFileStates[fileStateIndex].mBPLoadDataPath);

#if MGS_VERSION == 3
   if ( IS_SLOT_TAG( sStageLoadState.mFileStates[fileStateIndex].mFileId ) )
   {
      load_stage_slot( fileStateIndex );
   }
   else
#endif
   {
      load_stage_file( fileStateIndex );
   }

   BP_EndLoadData();

   //Clean up.
   SFlatFSLoadFileState *pFileState = &( pState->mLoadingFiles[ fileStateIndex ] );
   
   BP_CloseFile( pFileState->mpFileHandle );
   pFileState->mpFileHandle = pFileState->mpFileOp = NULL;
   pFileState->mpFileData = NULL;
}

int FS_LoadStageSync( void *info )
{
   if( sFlatFSLoadState.mLoadState == kFlatFSLoadState_Pending )
   {
      int slotLoading = FS_SlotPackLoadSync();
      if( slotLoading != 0 )
      {
         printf("FS_LoadStageSync: waiting for slot to finish loading...\n" );
         return 1;
      }
      //Kick it into actual loading state.
      BP_ForceClearStageAssetCache();
      sFlatFSLoadState.mAssetCacheTag = kStageAssetCacheTag;
      BP_BeginLoadFlatFS( &sFlatFSLoadState );
   }

   CStopWatch loadingStopwatch;
   
   do
   {
      int result = BP_LoadFlatFSSync( &sFlatFSLoadState, stage_begin_load_kp_file, stage_end_load_kp_file );

      if ( result == 0 )
      {
         // No more work to do.  Stage loaded.
         return 0;
      }
   } while ( loadingStopwatch.GetElapsedMilliseconds() < 30 );

   return 1;
}

//----------------------------------------------------------------------------
// FS_LoadStageRequest
//
// Pass in the stagename (dirname)
// Returns pointer to information to pass to FS_LoadStageSync
//----------------------------------------------------------------------------

void *FS_LoadStageRequest( char *dirname )
{
   struct SLoadInfo *pInfo = (struct SLoadInfo *) GV_Malloc( sizeof( struct SLoadInfo ) );

   strcpy( gCurrStageSoundDir, dirname );

   memset( pInfo, 0, sizeof( *pInfo ) );
   strcpy( pInfo->mSubFolder, dirname );

   XASSERT( sFlatFSLoadState.mLoadState == kFlatFSLoadState_Idle, "already busy loading" );

   //Start the manifest file loading.
   sStageLoadState.mpLoadInfo = pInfo;

   sprintf( sFlatFSLoadState.mManifestPath, "%s/stage/%s/manifest.txt", FS_BP_GetCurrentRegionFolder(), pInfo->mSubFolder );
   BP_UnifyPath(sFlatFSLoadState.mManifestPath);
   sprintf( sFlatFSLoadState.mBPAssetsListPath, "%s/stage/%s/bp_assets.txt", FS_BP_GetCurrentRegionFolder(), pInfo->mSubFolder );
   BP_UnifyPath(sFlatFSLoadState.mBPAssetsListPath);

   printf( "FS_LoadStageRequest: %s\n", sFlatFSLoadState.mManifestPath );

   sFlatFSLoadState.mLoadState = kFlatFSLoadState_Pending;

   return pInfo;
}

//----------------------------------------------------------------------------
// FS_LoadStageComplete
//
// Call this when the stage load is done.  It frees the work memory used
// by LoadStageRequest
//----------------------------------------------------------------------------

void FS_LoadStageComplete( void *info )
{
   XASSERT( sFlatFSLoadState.mLoadState == kFlatFSLoadState_Idle, "not done loading" );
   GV_Free( info );
   sStageLoadState.mpLoadInfo = NULL;
   BP_LoadCompleteFlatFS( &sFlatFSLoadState );
}

//----------------------------------------------------------------------------
// FS_LoadStagePreseek
//
// Preseek before loading
//----------------------------------------------------------------------------


void FS_LoadStagePreseek( char *dirname )
{

}
