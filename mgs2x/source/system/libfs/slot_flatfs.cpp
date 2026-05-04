#if MGS_VERSION==3
#  include "Engine/Stdafx.h"
#  include "MGS_Common.h"
#  include "sys_common.h"
#  include "def_ext.h"
#  include "ext_table.cnf"
#  include "datacnf_tag.h"
#elif MGS_VERSION==2
#  include <stdlib.h>
#  include <string.h>
#  include <math.h>
#  include <ctype.h>
#  include "bp_matrix.h"
#  include <algorithm>
#  include "MGS_SysCommon.h"
#else
#  error ERRORED!
#endif

#include "BP_Memory.h"
#include "loader_flatfs.h"
#include "Engine/System/CStopWatch.h"

/**
	@file slot処理


	スロット/ページの概念を導入。

	基本的にはresidentにスロットの領域を確保。
	pageデータはmkcdimgのステージパックデータに対応。

	SLOT, PAGEは、
	FS_SLOT_INFO, FS_PAGE_INFOで規定される構造体を別に確保し,
	そこで管理。

	PAGEの中にはmkcdimgで作られたステージパックデータが入る。

	DATA_CNF
	(2048 align)
	DATA...

	dataはロードされるとFCACHEに登録。解放時はFCACHEから削除。

	RESIDENTのREINIT時にはなめて再登録。
*/

#include "libgv.h"
#include "libfs.h"
#include "libdg.h"

#include <stdio.h>

#include "BP_FileSupport.h"
#include "BP_BaseRenderer.h"

#if MGS_VERSION==2
#  define GV_FCACHEID_RESIDENT 0
#  define EXT_ID_tri ( 't'-'a' )
#  define GV_GETSPEC( _id )	((u_int)(_id)>>24)
#endif

#if MGS_VERSION == 2
#  define SLOT_FOLDER_ROOT "face"
#else
#  define SLOT_FOLDER_ROOT "slot"
#endif

#define SLOT_PAGE_ENTRY_NAME_SIZE 64


//----------------------------------------------------------------------------

typedef struct _SSlotPackSetEntry
{
   unsigned id;
   void *ptr;
   unsigned size;
} SSlotPackSetEntry;

typedef struct _SSlotPackSetHeader
{
   char slotFolder[SLOT_PAGE_ENTRY_NAME_SIZE];
   SSlotPackSetEntry entries[0]; // Last entry has a NULL ptr
} SSlotPackSetHeader;

struct SSlotLoadState
{
   // SFlatFSLoadState is the first member.  It's used to 
   // get a slot load state from a flat load state.

   SFlatFSLoadState mFlatFSLoadState; // THIS MUST BE THE FIRST MEMBER
   SSlotPackSetHeader *mpWritingHeader;
   SSlotPackSetEntry *mpWriting;
};

#define MAX_SLOT_LOADS 64
static SSlotLoadState sLoadStates[ MAX_SLOT_LOADS ];

static SSlotLoadState *find_free_load_state()
{
   int i;
   for ( i = 0; i < MAX_SLOT_LOADS; ++i )
   {
      if ( sLoadStates[i].mpWritingHeader == NULL )
      {
         return sLoadStates + i;
      }
   }

   printf( "ERROR: Out of slot load states!\n" );
   HANGUP();
   return NULL;
}

static void free_load_state( SSlotLoadState *pState )
{
   memset( pState, 0, sizeof( *pState ) );
}

static inline SSlotLoadState * get_load_state_from_fs( SFlatFSLoadState *pfs )
{
   return ((SSlotLoadState *) (void *) pfs );
}

static SSlotLoadState *get_load_state_from_tag( unsigned int tag )
{
   for ( int i = 0; i < MAX_SLOT_LOADS; ++i )
   {
      if ( sLoadStates[i].mFlatFSLoadState.mAssetCacheTag == tag )
      {
         return sLoadStates + i;
      }
   }

   return NULL;
}

struct SSlotPageEntry
{
   char mName[ SLOT_PAGE_ENTRY_NAME_SIZE ];
   u_int mSizeAndOffset;
};

static SSlotPageEntry *   sSlotPageEntries;
static int                sSlotPageEntryCount;

#if MGS_VERSION==3
extern "C" void DG_RefreshTextureOffset( void );
#else
static void DG_RefreshTextureOffset()
{
}
#endif

//#define IS_VALID_TAG( _id ) ( !IS_SECTION_TAG( _id ) && !IS_END_TAG( _id ) )

//----------------------------------------------------------------------------

/*
	パックデータ処理。
	FS_SLOTPACKは読み込まれたデータの先頭。
*/

void *FS_SlotPackGetData( FS_SLOTPACK *pack, int id )
{
   SSlotPackSetHeader *pSlotPackInfo = (SSlotPackSetHeader *) pack;
   SSlotPackSetEntry *pCurrentInfo = pSlotPackInfo->entries;

   for ( ; pCurrentInfo->ptr; ++pCurrentInfo )
   {
      if ( id == pCurrentInfo->id )
      {
         return pCurrentInfo->ptr;
      }
   }

   return NULL;
}

int FS_SlotPackGetDataNum( FS_SLOTPACK *pack )
{
   SSlotPackSetHeader *pSlotPackInfo = (SSlotPackSetHeader *) pack;
   SSlotPackSetEntry *pCurrentInfo = pSlotPackInfo->entries;
   int n = 0;

   for ( ; pCurrentInfo->ptr; ++pCurrentInfo )
   {
      ++n;
   }

   return n;
}

void FS_SlotPackGetDataTable( FS_SLOTPACK *pack, FS_SLOTPACK_TAG *output )
{
   SSlotPackSetHeader *pSlotPackInfo = (SSlotPackSetHeader *) pack;
   SSlotPackSetEntry *pCurrentInfo = pSlotPackInfo->entries;

   for ( ; pCurrentInfo->ptr; ++pCurrentInfo )
   {
      output->id = pCurrentInfo->id;
      output->size = pCurrentInfo->size;
      output->data = pCurrentInfo->ptr;

      ++output;
   }
}

/*
	パックデータのセットアップ。

	@param pack パックデータへのポインタ
	@param mode enum #loadfunc_mode
*/
void FS_SlotPackInit( FS_SLOTPACK *pack, int mode )
{
   SSlotPackSetHeader *pSlotPackInfo = (SSlotPackSetHeader *) pack;
   SSlotPackSetEntry *pCurrentInfo = pSlotPackInfo->entries;
   int tex_flag = 0;

#ifdef DEBUG_MODE
   if( mode == GV_INIT_RESIDENT ){
      GV_MemoryNoLeakCheckStart();
   }
#endif
   printf( "slot init %X\n", pack );

//   XASSERT( pSlotPackInfo->slotCode != 0 || pCurrentInfo->ptr == NULL, "Invalid slot code" );

   // Only do processing for slots with data in it
   if( pCurrentInfo->ptr )
   {
      char slotPath[FILENAME_MAX];
      unsigned int const assetCacheTag = (unsigned int) pSlotPackInfo;

      printf( " - with slot folder %s\n", pSlotPackInfo->slotFolder );
      sprintf(slotPath, "%s/" SLOT_FOLDER_ROOT "/%s/cache/", FS_BP_GetCurrentRegionFolder(), pSlotPackInfo->slotFolder );
      BP_UnifyPath(slotPath);
      BP_BeginLoadData(assetCacheTag, slotPath);

      for ( ; pCurrentInfo->ptr; ++pCurrentInfo )
      {
         int id = pCurrentInfo->id;
         void *ptr = pCurrentInfo->ptr;
         int size = pCurrentInfo->size;
#if MGS_VERSION==3
         int real_mode = mode;
#else
         // For MGS2, the "face" stuff inits textures as CACHE and non-textures as NOCACHE
         int real_mode;
         if ( mode == SLOTPACK_INIT_MGS2_FACE )
         {
            real_mode = ( GV_GETSPEC( id ) == EXT_ID_tri ) ? GV_INIT_CACHE : GV_INIT_NOCACHE;
         }
         else
         {
            real_mode = mode;
         }
#endif

         if( real_mode == GV_INIT_RESIDENT_AGAIN ){
			   id |= GV_FCACHEID_RESIDENT;
		   }
   printf( "Slot LoadInit %x %x\n", id, size );
#if MGS_VERSION == 3
         GV_LoadInit( ptr, id, real_mode, size );
#else
         GV_LoadInit( ptr, id, real_mode );
#endif
         if( GV_GETSPEC( id ) == EXT_ID_tri ){
			   tex_flag = 1;
		   }
	   }

      BP_VerifyEmptyStageAssetCache( assetCacheTag );

      BP_EndLoadData();
   }

   if( tex_flag ){
		DG_RefreshTextureOffset();
	}

#ifdef DEBUG_MODE
	if( mode == GV_INIT_RESIDENT ){
		GV_MemoryNoLeakCheckEnd();
	}
#endif
}

/*
	パックデータの終了処理。

	@param pack パックデータへのポインタ
	@param mode enum #loadfunc_mode
*/

int const kMaxDelayedFreeCount = 1024;
static void* gDelayedFreeList[kMaxDelayedFreeCount];
int gDelayedFreeCount = 0;

void Internal_FS_SlotPackClose( FS_SLOTPACK *pack , int mode, int isDelayedFree )
{
   SSlotPackSetHeader *pSlotPackInfo = (SSlotPackSetHeader *) pack;
   SSlotPackSetEntry *pCurrentInfo = pSlotPackInfo->entries;
   int tex_flag = 0;

   // Better to be safe than sorry - let's sync the endthread up when the slot pack closes
   DG_AS_EndframeThread_WaitDone();

//   XASSERT( pSlotPackInfo->slotCode != 0 || pCurrentInfo->ptr == NULL, "Invalid slot code" );

   printf( "slot close %X\n", pack );

   unsigned int const assetCacheTag = (unsigned int) pSlotPackInfo;
   BP_ClearStageAssetCacheByTag( assetCacheTag );

   SSlotLoadState *pState = get_load_state_from_tag( assetCacheTag );
   void *ppFileDataToFree[ FLATFS_MAX_LOAD_FILES ] = { 0 };
   int fileDataToFreeCount = 0;
   if ( pState )
   {
      BP_CancelLoadFlatFSReturnFileData( &pState->mFlatFSLoadState, ppFileDataToFree, &fileDataToFreeCount );

      free_load_state( pState );
   }

   for ( ; pCurrentInfo->ptr; ++pCurrentInfo )
   {
      int id = pCurrentInfo->id;
      void *ptr = pCurrentInfo->ptr;

      if ( ptr == NULL )
      {
         // Has the slot entry already been closed?
         continue;
      }

      // If this was the last file data loading, then we are going to free it
      // here, so don't free it later
      BP_IgnoreThisTemporaryData( ptr, ppFileDataToFree, fileDataToFreeCount );

      if( GV_GETSPEC( id ) == EXT_ID_tri ){
         // TRIのみ削除処理が必要。
         void *res;
         if( ( res = DG_FreeTextureCache( ptr ) ) != NULL ){
#if MGS_VERSION==3
            GV_MemorySetDelayedFree( res );
            GV_Free( res );
#else
            GV_DelayedFree(res);
#endif
            tex_flag = 1;
         }
      }
      if( mode != GV_INIT_NOCACHE ){
#if MGS_VERSION==3
         GV_RemoveFCache( id, ptr );
#endif
      }

      BP_DestroyResources((unsigned int)pCurrentInfo->ptr, (unsigned int)pCurrentInfo->ptr + pCurrentInfo->size);

      if( isDelayedFree )
      {
         if( gDelayedFreeCount >= kMaxDelayedFreeCount )
            BP_BREAK;

         gDelayedFreeList[gDelayedFreeCount] = ptr;
         ++gDelayedFreeCount;
      }
      else
      {
         BP_Memory_Free( ptr );
      }

      // Mark the slot entry as unallocated, in case of a double-close
      pCurrentInfo->ptr = NULL;
   }

   BP_FreeTemporaryFlatFSFileData( ppFileDataToFree, fileDataToFreeCount );

   BP_FlushDestroyedResources();

   if( tex_flag ){	
      DG_RefreshTextureOffset();
   }
}

void FS_SlotPackClose( FS_SLOTPACK *pack, int mode )
{
   Internal_FS_SlotPackClose(pack, mode, 0);
}

void FS_SlotPackDelayedClose( FS_SLOTPACK *pack , int mode )
{
   Internal_FS_SlotPackClose(pack, mode, 1);
}

void FS_SlotProcessDelayedFree()
{
   if( gDelayedFreeCount > 0 )
   {
      for( int i = 0; i < gDelayedFreeCount; ++i )
      {
         void* ptr = gDelayedFreeList[i];
         
         // Not necessary but makes it less confusing to look at the list in the debugger.
         gDelayedFreeList[i] = NULL; 

         BP_Memory_Free(ptr);
      }

      gDelayedFreeCount = 0;
   }
}

#if MGS_VERSION==3

FS_SLOT_INFO *FS_SlotGetInfo( u_int id )
{
	return (FS_SLOT_INFO *)GV_GetFCache( GV_FCACHE_ID( id, EXT_ID_slot ) );
}

static int LoadInitSlot( void *ptr, int id, int mode, int size )
{
	if( mode == GV_INIT_RESIDENT_AGAIN ){
		FS_SLOT_INFO *slot = (FS_SLOT_INFO *)ptr;
		int i;

		for( i = 0; i < slot->page_num; i++ ){
			FS_SlotPageInit( &( slot->pages[ i ] ), mode );
		}
	}

   return GV_LOAD_OK;
}

#endif

static bool BuildSlotPageEntry( SSlotPageEntry *pEntry, char const *str )
{
   // Return TRUE if we built an entry
   size_t nameSize = strcspn( str, " \r\n" );

   if ( nameSize >= sizeof( pEntry->mName ) / sizeof( pEntry->mName[0] ) )
   {
      // Name is too big.  Ignore
      BP_BREAK;
      return false;
   }

   if ( str[ nameSize ] != ' ' )
   {
      // Incorrect format
      return false;
   }

   strncpy( pEntry->mName, str, nameSize );
   pEntry->mName[ nameSize ] = 0;

   // Now we get the code
   if ( 1 == sscanf( str + nameSize + 1, "%x", &( pEntry->mSizeAndOffset ) ) )
   {
      return true;
   }
   else
   {
      return false;
   }
}

static char const *next_line_in_string( char const *str )
{
   // If we're at the end, returns NULL

   size_t s = strcspn( str, "\n" );
   if ( str[s] == 0 )
   {
      return NULL;
   }
   else
   {
      return str + s + 1;
   }
}

void FS_SlotSystemInit( void )
{
#if MGS_VERSION==3
   GV_LoaderSet( EXT_ID_slot, LoadInitSlot, GV_LOADSET_REINIT_RESIDENT );
#endif

   char pageFileName[ 255 ];

   sprintf( pageFileName, "%s/%s/pages.txt", FS_BP_GetCurrentRegionFolder(), SLOT_FOLDER_ROOT );

   {
      void *fp = BP_OpenFile( pageFileName, NULL );
      if ( !fp )
      {
         HANGUP(); 
      }

      size_t const pagesFileSize = BP_GetFileSize( fp );
      char* pagesFileData = (char *) BP_Memory_Alloc( pagesFileSize + 1, 4, kMT_Permanent, kMC_Slot );
      pagesFileData[ pagesFileSize ] = 0;
      BP_ReadFile( fp, pagesFileData, pagesFileSize );
      BP_CloseFile( fp );

      // Count the newlines to see what our upper limit of entries is
      // Start with 1 in case we don't end with a newline
      int entryCountUpperBound = 1;
      for ( char *c = pagesFileData; *c; ++c )
      {
         if ( *c == '\n' )
         {
            ++entryCountUpperBound;
         }
      }

      sSlotPageEntries = (SSlotPageEntry *) BP_Memory_Alloc( sizeof( SSlotPageEntry ) * entryCountUpperBound, 4, kMT_Permanent, kMC_Slot );
      sSlotPageEntryCount = 0;

      for ( char const *line = pagesFileData; line != NULL; line = next_line_in_string( line ) )
      {
         if ( BuildSlotPageEntry( sSlotPageEntries + sSlotPageEntryCount, line ) )
         {
            ++sSlotPageEntryCount;
         }
      }

      XASSERT( sSlotPageEntryCount <= entryCountUpperBound, "More slot page entries than we expected!" );

      BP_Memory_Free( pagesFileData );
   }
}

// デバッグ用。
#ifdef DEBUG_MODE

void FS_SlotDump( FS_SLOT_INFO *info )
{
	int i;
	printf( "SLOT %X size %X page %d\n", info->name, info->size, info->page_num );
	for( i = 0; i < info->page_num; i++ ){
		FS_SLOT_PAGE *page = &( info->pages[ i ] );
		printf( "PAGE %08X BUF %08X\n", page->id, page->page );
	}
}

#endif

static unsigned get_code_for_ext( char const *ext )
{
#if MGS_VERSION==3
   int i;

   for ( i = 0; ext_table[i].name; ++i )
   {
      if ( !strcmp( ext, ext_table[i].name ) )
      {
         return ext_table[i].id;
      }
   }

   return 0;
#else
   return tolower( ext[0] ) - 'a';
#endif
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

static unsigned slot_file_id( char const *filename, int print_warn )
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
      unsigned const code = get_code_for_ext( ext );

      if ( code == 0 )
      {
         if ( print_warn )
            printf( "WARNING: Could not map ext for %s to id\n", filename );
         return 0;
      }

      return id_low | ( code << 24 );
   }
}

static void slot_begin_load_kp_file( int const fileStateIndex, SFlatFSLoadState * pState )
{
   SSlotLoadState *pSlotState = get_load_state_from_fs( pState );
   SFlatFSLoadFileState *pFileState = &( pState->mLoadingFiles[fileStateIndex] );
   char folderRoot[ FILENAME_MAX ];
   sprintf( folderRoot, "%s/" SLOT_FOLDER_ROOT "/%s/", FS_BP_GetCurrentRegionFolder(), pSlotState->mpWritingHeader->slotFolder );

   char relativePath[FILENAME_MAX];
   char unifiedPath[FILENAME_MAX];
   char loadPath[ FILENAME_MAX ];

   BP_SplitManifestLine( pState->mpCurrentManifestLine, folderRoot, unifiedPath, loadPath, relativePath );

//   unsigned int offset = FS_SLOT_CODE_TO_OFFSET( sSlotLoadState.mpWritingHeader->slotCode );
   unsigned int fileId = slot_file_id( strchr( relativePath, '/' ) + 1, 1 );

   strcpy( pFileState->mUnifiedPath, unifiedPath );
   strcpy( pFileState->mLoadPath, loadPath );
   sprintf(pFileState->mStagePath, "%s/" SLOT_FOLDER_ROOT "/%s/%s", FS_BP_GetCurrentRegionFolder(), 
      pSlotState->mpWritingHeader->slotFolder, relativePath );
   BP_UnifyPath(pFileState->mStagePath);

   char assetLoadPath[FILENAME_MAX];
   char assetLoadPathCommon[FILENAME_MAX];
   BP_GetAssetLoadFullPath( assetLoadPath, assetLoadPathCommon, pFileState->mLoadPath, pFileState->mUnifiedPath );
   pFileState->mpFileHandle = BP_OpenFile( assetLoadPath, assetLoadPathCommon );

   if( !pFileState->mpFileHandle )
      BP_BREAK;

   pFileState->mFileSize = BP_GetFileSize( pFileState->mpFileHandle );

#if MGS_VERSION==3
   // Set the memory type of the slot allocations to the memory type of
   // the slot to prevent memory leaks
   enum kMemoryType const memoryType = BP_Memory_Classify( pSlotState->mpWritingHeader );

   pFileState->mpFileData = (char*)BP_Memory_Alloc( pFileState->mFileSize, 128, memoryType, kMC_Slot );
#else
   pFileState->mpFileData = (char*)BP_Memory_Alloc( pFileState->mFileSize, 128, kMT_Permanent, kMC_Slot );
#endif
   pFileState->mpFileOp = BP_ReadFileAsync( pFileState->mpFileHandle, pFileState->mpFileData, pFileState->mFileSize );

   pSlotState->mpWriting->id = fileId;
   pSlotState->mpWriting->ptr = pFileState->mpFileData;
   pSlotState->mpWriting->size = pFileState->mFileSize;
   ++pSlotState->mpWriting;
}

static void slot_end_load_kp_file( int const fileStateIndex, SFlatFSLoadState * pState )
{
   SFlatFSLoadFileState *pFileState = &( pState->mLoadingFiles[fileStateIndex] );

   BP_CloseFile( pFileState->mpFileHandle );
   pFileState->mpFileHandle = pFileState->mpFileOp = NULL;
   pFileState->mpFileData = NULL;
}

static bool FindSlotByCodeAndGetFolder( char *pFolder, u_int code )
{
   for ( int i = 0; i < sSlotPageEntryCount; ++i )
   {
      if ( sSlotPageEntries[i].mSizeAndOffset == code )
      {
         strcpy( pFolder, sSlotPageEntries[i].mName );

         return true;
      }
   }

   return false;
}

int FS_SlotPackLoadSet( void *buffer, int code )
{
   char slotFolder[ SLOT_PAGE_ENTRY_NAME_SIZE ] = { 0 };
   if ( !FindSlotByCodeAndGetFolder( slotFolder, code ) )
   {
      printf( "ERROR: Slot code %8.8x not found - Perhaps size is off?\n", code );
      HANGUP();
   }

   SSlotLoadState *pLoadState = find_free_load_state();
   SFlatFSLoadState *pFS = &( pLoadState->mFlatFSLoadState );
   sprintf(pFS->mManifestPath, "%s/" SLOT_FOLDER_ROOT "/%s/manifest.txt", FS_BP_GetCurrentRegionFolder(), slotFolder);
   BP_UnifyPath(pFS->mManifestPath);
   sprintf(pFS->mBPAssetsListPath, "%s/" SLOT_FOLDER_ROOT "/%s/bp_assets.txt", FS_BP_GetCurrentRegionFolder(), slotFolder);
   BP_UnifyPath(pFS->mBPAssetsListPath);

   printf( "FS_SlotPackLoadSet: %s\n", pFS->mManifestPath );

   pFS->mAssetCacheTag = (unsigned int)buffer;
   BP_BeginLoadFlatFS( pFS );

   pLoadState->mpWritingHeader = (SSlotPackSetHeader *) buffer;
   strcpy( pLoadState->mpWritingHeader->slotFolder, slotFolder );
   pLoadState->mpWriting = pLoadState->mpWritingHeader->entries;

   return 1;
}

static int _FS_SlotPackLoadSync_OneFrame()
{
   for ( int i = 0; i < MAX_SLOT_LOADS; ++i )
   {
      if ( sLoadStates[i].mpWritingHeader )
      {
         int ret = BP_LoadFlatFSSync( &sLoadStates[i].mFlatFSLoadState, slot_begin_load_kp_file, slot_end_load_kp_file );

         // If we have an incomplete load, return it immediately
         if ( ret != 0 )
         {
            return ret;
         }
         else
         {
            // Our load just completed, the last item in slot pack is NULL
            sLoadStates[i].mpWriting->id = 0;
            sLoadStates[i].mpWriting->ptr = NULL;
            sLoadStates[i].mpWriting->size = 0;

            free_load_state( sLoadStates + i );
         }
      }
   }

   return 0;
}

int FS_SlotPackLoadSync( void )
{
   CStopWatch slotpackTimer;

   do 
   {
      int result = _FS_SlotPackLoadSync_OneFrame();

      if ( result == 0 )
      {
         return 0;
      }
   } while ( slotpackTimer.GetElapsedMilliseconds() < 30 );

   return 1;
}

