#if MGS_VERSION==3
#include "Engine/Stdafx.h"

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "BP_BaseRenderer.h"

#include "MGS_Common.h"
#elif MGS_VERSION==2
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "bp_matrix.h"
#include "MGS_SysCommon.h"

#include "BP_BaseRenderer.h"
#endif

#include "BP_Misc.h"

#include "libfs.h"

#include "stdio.h"
#include "ctype.h"

#include "BP_FileSupport.h"

#include "loader_flatfs.h"
#include "Engine/Basics/BPEAssert.h"

extern "C" void BP_ConvertToFlatListFilename( char * path, const char * knownFlatPath, const char* texToolPath );

extern int IsDoneWithRenderTasks_c();

#define FLATFS_DEBUG_LOAD_TIMES 0
#define FLATFS_LOG_LOADS 0

#if FLATFS_DEBUG_LOAD_TIMES
extern u_int GV_BP_UpdateCount;
#endif

static int const skMaxSimutaneousLoads = FLATFS_MAX_LOAD_FILES;

static void log_async_load_begin( char const *filename )
{
#if FLATFS_DEBUG_LOAD_TIMES
   printf( "ASYNCREAD: %5d %s\n", GV_BP_UpdateCount, filename );
#endif
}

#if FLATFS_LOG_LOADS
#  define flatfs_log_printf printf
#else
#  define flatfs_log_printf(x,...) (0)
#endif

static void copy_begin_end( char *dst, char const *begin, char const *end )
{
   size_t newLen = end - begin;
   strncpy( dst, begin, newLen );
   dst[ newLen ] = 0;
}

static void begin_load_bp_file( SFlatFSLoadFileState *pFileOut, SFlatFSLoadFileState const *pPriorFileOut, SFlatFSLoadState const * pState)
{
   char const *unifiedPathStart = pState->mpCurrentBPAssetsLine;
   char const *comma;
   comma = strcspn( unifiedPathStart, ",\r\n" ) + unifiedPathStart;
   ASSERT( comma[0] == ',' );

   // Zero out the file that we're writing to
   memset( pFileOut, 0, sizeof( *pFileOut ) );

   char const *loadPathStart = comma + 1;
   comma = strcspn( loadPathStart, ",\r\n" ) + loadPathStart;
   ASSERT( comma[0] == ',' );

   char const *stagePathStart = comma + 1;
   char const *stagePathEnd = strcspn( stagePathStart, "\r\n" ) + stagePathStart;

   bool bDuplicateAsset = false;
   
   //If the current line's *flat* filename matches that of the previous line,
   //we don't try to load another copy of the asset but rather connect the archive path
   //to the already loaded asset.
   if ( pPriorFileOut )
   {
      bDuplicateAsset = strcmp( unifiedPathStart, pPriorFileOut->mUnifiedPath ) == 0;
   }

   copy_begin_end( pFileOut->mUnifiedPath, unifiedPathStart, loadPathStart - 1 );
   copy_begin_end( pFileOut->mLoadPath, loadPathStart, stagePathStart - 1 );
   copy_begin_end( pFileOut->mStagePath, stagePathStart, stagePathEnd );

   flatfs_log_printf( "begin_bp %d(%8.8x,%8.8x,%s): %s\n",
      pFileOut - pState->mLoadingFiles,
      pFileOut,
      pPriorFileOut,
      bDuplicateAsset ? "d" : "u",
      pFileOut->mStagePath );

   //Fix up the filename to flatlist / platform specific as necessary.
   char loadPath[FILENAME_MAX];
   strcpy( loadPath, pFileOut->mLoadPath );
   BP_ConvertToFlatListFilename( loadPath, pFileOut->mUnifiedPath, pFileOut->mStagePath );
   BP_InsertPlatformSubfolder( pFileOut->mLoadPath );
   BP_InsertPlatformSubfolder( pFileOut->mStagePath );
   char loadPathCommon[FILENAME_MAX];
   strcpy( loadPathCommon, pFileOut->mUnifiedPath );
   BP_ConvertToFlatListFilename( loadPathCommon, pFileOut->mUnifiedPath, pFileOut->mStagePath );
   BP_InsertPlatformSubfolder( pFileOut->mUnifiedPath );

   if( !bDuplicateAsset )
   {
      if( !BP_IsPsarcMounted() )
      {
         // Determine if a SKU/language specific override file exists and use that one instead.
         // NOTE: This is ONLY needed when running without an archive, because the tool that builds the archives contains the same logic.
         BP_GetPathPlatformSKUOverride(loadPath, loadPathCommon);
      }

      //Need to load this BP asset.
      pFileOut->mpFileHandle = BP_OpenFile( loadPath, loadPathCommon );
      pFileOut->mFileSize = BP_GetFileSize( pFileOut->mpFileHandle );
      pFileOut->mpFileData = (char*)malloc( pFileOut->mFileSize );;
      pFileOut->mpFileOp = BP_ReadFileAsync( pFileOut->mpFileHandle, pFileOut->mpFileData, pFileOut->mFileSize );
      log_async_load_begin( loadPath );
   }
   else
   {
      //Leave sFlatFSLoadState.mpFileOp == NULL and the next step will set a reference
      //to to the existing copy in the cache rather than constructing another one.
   }
}

static void end_load_bp_file(SFlatFSLoadFileState * pFileState, SFlatFSLoadState const * pState)
{
   //Clean up.

   flatfs_log_printf( "end_bp %d(%8.8x): %s\n",
      pFileState - pState->mLoadingFiles,
      pFileState,
      pFileState->mStagePath );

   BP_CloseFile( pFileState->mpFileHandle );
   pFileState->mpFileHandle = pFileState->mpFileOp = NULL;
}

//
// loading_fifo_reset_with_one_loading
//
// Resets the fifo, sets it to one item loading, and returns the 
// index to that item
//
static int loading_fifo_reset_with_one_loading( SFlatFSLoadState *pState )
{
   pState->mFileLoadingHead = 0;
   pState->mFileLoadingCount = 1;

   return pState->mFileLoadingHead;
}

//
// loading_fifo_prepop_loaded
//
// Returns the index to the item that would be next popped, or
// -1 if there is no such item.  Note that this does not modify state
//
// The intended usage pattern is that this is used in a for 
// loop's initializer like so:
//   for ( int index = loading_fifo_prepop_loaded(state); index != -1; index = loading_fifo_poploaded_prepop_next(state) )
//
static int loading_fifo_prepop_loaded( SFlatFSLoadState *pState )
{
   if ( pState->mFileLoadingCount == 0 )
   {
      return -1;
   }
   else
   {
      return pState->mFileLoadingHead;
   }
}

//
// loading_fifo_poploaded_prepop_next
//
// Pops a loaded item from the FIFO and returns loading_fifo_prepop_loaded for the 
// next item.  This changes the state.
//
// The intended usage pattern is that this is used in a for 
// loop's initializer like so:
//   for ( int index = loading_fifo_prepop_loaded(state); index != -1; index = loading_fifo_poploaded_prepop_next(state) )
//
static int loading_fifo_poploaded_prepop_next( SFlatFSLoadState *pState )
{
   --pState->mFileLoadingCount;
   pState->mFileLoadingHead = ( pState->mFileLoadingHead + 1 ) % FLATFS_MAX_LOAD_FILES;

   return loading_fifo_prepop_loaded( pState );
}

//
// loading_fifo_prepush_loading
//
// Returns the index to the item that would next be pushed, or
// -1 if pushing the item would overflow the list.  This does not
// modify state.
//
// The intended usage pattern is that this is used in a for 
// loop's initializer like so:
//   for ( int index = loading_fifo_prepush_loading(state); index != -1; index = loading_fifo_pushloading_prepush_next(state) )
//
static int loading_fifo_prepush_loading( SFlatFSLoadState *pState )
{
   if ( pState->mFileLoadingCount >= skMaxSimutaneousLoads )
   {
      return -1;
   }
   else
   {
      return ( pState->mFileLoadingHead + pState->mFileLoadingCount ) % FLATFS_MAX_LOAD_FILES;
   }
}

//
// loading_fifo_pushloading_prepush_next
//
// Pushes an item to the FIFO, then returns loading_fifo_prepush_loading to give you
// the next item.
//
// The intended usage pattern is that this is used in a for 
// loop's initializer like so:
//   for ( int index = loading_fifo_prepush_loading(state); index != -1; index = loading_fifo_pushloading_prepush_next(state) )
//
static int loading_fifo_pushloading_prepush_next( SFlatFSLoadState *pState )
{
   ++pState->mFileLoadingCount;

   return loading_fifo_prepush_loading( pState );
}

//
// loading_fifo_prior_file
//
// This simply returns the prior entry in the FIFO, taking wraparound into account
//

int loading_fifo_prior_file( int const loadIndex )
{
   if ( loadIndex == 0 )
   {
      return FLATFS_MAX_LOAD_FILES - 1;
   }
   else
   {
      return loadIndex - 1;
   }
}

void BP_BeginLoadFlatFS( SFlatFSLoadState * pState )
{
   BP_VerifyEmptyStageAssetCache( pState->mAssetCacheTag );
   pState->mLoadState = kFlatFSLoadState_ReadingManifest;

   SFlatFSLoadFileState *pManifestFileState = pState->mLoadingFiles + 0;

   pManifestFileState->mStagePath[0] = 0;
   pManifestFileState->mUnifiedPath[0] = 0;
   pManifestFileState->mLoadPath[0] = 0;

   pManifestFileState->mpFileHandle = BP_OpenFile( pState->mManifestPath, NULL );

   if( !pManifestFileState->mpFileHandle )
      BP_BREAK;

   pManifestFileState->mFileSize = BP_GetFileSize( pManifestFileState->mpFileHandle );
   pState->mpManifestData = (char*)malloc( pManifestFileState->mFileSize+1 );
   pState->mpManifestData[pManifestFileState->mFileSize] = '\0';
   pManifestFileState->mpFileOp = BP_ReadFileAsync( pManifestFileState->mpFileHandle, pState->mpManifestData, pManifestFileState->mFileSize );
   log_async_load_begin( pState->mManifestPath );
}

int BP_LoadFlatFSSync( SFlatFSLoadState * pState, BP_BeginLoadKPAssetCallback begin_load_kp_file, BP_EndLoadKPAssetCallback end_load_kp_file )
{
   switch( pState->mLoadState )
   {
   case kFlatFSLoadState_Idle:
      return 0;   //done with last operation.
   case kFlatFSLoadState_ReadingManifest:
      {
         {
            SFlatFSLoadFileState *pManifestFileState = pState->mLoadingFiles + 0;

            if( !BP_TryFinishFileOp( pManifestFileState->mpFileOp ) )
            {
               return 1;
            }
            //Clean up.
            BP_CloseFile( pManifestFileState->mpFileHandle );
            pManifestFileState->mpFileHandle = pManifestFileState->mpFileOp = NULL;

            pState->mpCurrentManifestLine = pState->mpManifestData;
         }

         SFlatFSLoadFileState *pBPAssetsFileState = pState->mLoadingFiles + 0;

         //Start reading BP assets list.
         pBPAssetsFileState->mpFileHandle = BP_OpenFile( pState->mBPAssetsListPath, NULL );

         if( !pBPAssetsFileState->mpFileHandle )
            BP_BREAK;

         pBPAssetsFileState->mFileSize = BP_GetFileSize( pBPAssetsFileState->mpFileHandle );
         if( !pBPAssetsFileState->mFileSize )
         {
            //no BP assets for this slot or stage.
            BP_CloseFile( pBPAssetsFileState->mpFileHandle );
            pBPAssetsFileState->mpFileHandle = NULL;
            pState->mLoadState = kFlatFSLoadState_ReadingKPFiles;
            begin_load_kp_file( loading_fifo_reset_with_one_loading( pState ), pState );
            flatfs_log_printf( "begin_kp(start1) %d: %s\n", 0, pState->mLoadingFiles[0].mUnifiedPath );

            return 1;
         }
         else
         {
            //Load BP assets list.
            pState->mpBPAssetsData = (char*)malloc( pBPAssetsFileState->mFileSize+1 );
            pState->mpBPAssetsData[pBPAssetsFileState->mFileSize] = '\0';
            pBPAssetsFileState->mpFileOp = BP_ReadFileAsync( pBPAssetsFileState->mpFileHandle, pState->mpBPAssetsData, pBPAssetsFileState->mFileSize );
            pState->mLoadState = kFlatFSLoadState_ReadingBPAssetsList;
            log_async_load_begin( pState->mBPAssetsListPath );
         }
      }
      //fallthrough
   case kFlatFSLoadState_ReadingBPAssetsList:
      {
         SFlatFSLoadFileState *pBPAssetsFileState = pState->mLoadingFiles + 0;

         if( !BP_TryFinishFileOp( pBPAssetsFileState->mpFileOp ) )
         {
            return 1;
         }
         //Clean up.
         BP_CloseFile( pBPAssetsFileState->mpFileHandle );
         pBPAssetsFileState->mpFileHandle = pBPAssetsFileState->mpFileOp = NULL;

         pState->mpCurrentBPAssetsLine = pState->mpBPAssetsData;

         //Start parsing BP assets data.
         pState->mLoadState = kFlatFSLoadState_ReadingBPFiles;
         begin_load_bp_file( &( pState->mLoadingFiles[ loading_fifo_reset_with_one_loading( pState ) ] ), NULL, pState );
      }
      //fallthrough
   case kFlatFSLoadState_ReadingBPFiles:
      {
         // If doneWithLoaded is set to false, that means that we need to keep looping through this state
         bool doneWithLoaded = true;

         //
         // First, try to finish any outstanding loads
         //

         for ( int loadingFile = loading_fifo_prepop_loaded( pState ); loadingFile != -1 ; loadingFile = loading_fifo_poploaded_prepop_next( pState ) )
         {
            SFlatFSLoadFileState *pCurrentFile = &( pState->mLoadingFiles[ loadingFile ] );

            if( !pCurrentFile->mpFileOp )
            {
               //The last asset in the list was a duplicate asset so we did not load it.
               //Now, connect this archive path to the asset already in the cache for the stage.
               BP_AddReferenceToLastStageAssetCacheEntry( pState->mAssetCacheTag, pCurrentFile->mStagePath );
            }
            else
            {
               // If we can't load new texture, wait till we can
               // yeesh
               if( !BP_CanLoadNewTexture() )
               {
                  doneWithLoaded = false;
                  break;
               }

               // If we haven't finished loading yet, then obviously, break out and try later.
               if( !BP_TryFinishFileOp( pCurrentFile->mpFileOp ) )
               {
                  doneWithLoaded = false;
                  break;
               }
               else
               {
                  flatfs_log_printf( "bp_file_op done: %8.8x\n", pCurrentFile->mpFileOp );
               }

               end_load_bp_file(pCurrentFile, pState);

               //construct BP asset and add to stage cache.
               flatfs_log_printf( "bp_asset tag %8.8x: %s\n", pState->mAssetCacheTag, pCurrentFile->mStagePath );
               BP_AddResourceToStageAssetCache( pState->mAssetCacheTag, pCurrentFile->mStagePath, pCurrentFile->mUnifiedPath, pCurrentFile->mpFileData, pCurrentFile->mFileSize );

               //BP assets are not constructed in-place.  We're done with this memory.
               free( pCurrentFile->mpFileData );
               pCurrentFile->mpFileData = NULL;
            }
         } // end loop loaded files

         //
         // Now we try to start new loads
         //

         // if pState->mpCurrentBPAssetsLine is NULL, then we have nothing else to pend loading
         if ( pState->mpCurrentBPAssetsLine )
         {
            for ( int loadIndex = loading_fifo_prepush_loading( pState ); loadIndex != -1; loadIndex = loading_fifo_pushloading_prepush_next( pState ) )
            {
               //Next asset in the bp assets list, if any.
               pState->mpCurrentBPAssetsLine = strchr( pState->mpCurrentBPAssetsLine, '\n' );

               if( !pState->mpCurrentBPAssetsLine || *( pState->mpCurrentBPAssetsLine + 1 ) == '\0' )
               {
                  //Done with BP assets.  Start parsing the manifest file for KP assets.
                  free( pState->mpBPAssetsData );
                  pState->mpBPAssetsData = pState->mpCurrentBPAssetsLine = NULL;

                  // Break out so we don't "push" anything else
                  // Note that if there's nothing waiting to be loaded, we're done here.
                  break;
               }
               else
               {
                  //Start next BP asset loading.
                  ++pState->mpCurrentBPAssetsLine;
                  begin_load_bp_file( &( pState->mLoadingFiles[ loadIndex ] ), &( pState->mLoadingFiles[ loading_fifo_prior_file( loadIndex ) ] ), pState );

                  // Since we started a load, we're not done
                  doneWithLoaded = false;
               }
            }
         } // endif pState->mCurrentBPAssetsLine

         // If we are definitely done loading, then move on to the next state
         if ( doneWithLoaded )
         {
            BPE_VERIFY( pState->mFileLoadingCount == 0, false, "Stale files?!" );

            pState->mLoadState = kFlatFSLoadState_SyncPendingBPFiles;
            begin_load_kp_file( loading_fifo_reset_with_one_loading( pState ), pState );
            flatfs_log_printf( "begin_kp(start2) %d: %s\n", 0, pState->mLoadingFiles[0].mUnifiedPath );
         }
         else
         {
            flatfs_log_printf( "bp_loop\n" );

            return 1;
         }
      } // end case kFlatFSLoadState_ReadingBPFiles

      //fallthrough
   case kFlatFSLoadState_SyncPendingBPFiles:
      {
#if BP_360
         if( !IsDoneWithRenderTasks_c() )
         {
            return 1;
         }
#endif
         pState->mLoadState = kFlatFSLoadState_ReadingKPFiles;
      }  // end case kFlatFSLoadState_SyncPendingBPFiles
      //fallthrough
   case kFlatFSLoadState_ReadingKPFiles:
      {
         // If doneWithLoaded is set to false, that means that we need to keep looping through this state
         bool doneWithLoaded = true;

         //
         // First, try to finish any out standing loads
         //

         for ( int loadingFile = loading_fifo_prepop_loaded( pState ); loadingFile != -1 ; loadingFile = loading_fifo_poploaded_prepop_next( pState ) )
         {
            SFlatFSLoadFileState *pCurrentFile = &( pState->mLoadingFiles[ loadingFile ] );

            if( !BP_TryFinishFileOp( pCurrentFile->mpFileOp ) )
            {
               doneWithLoaded = false;
               flatfs_log_printf( "kp_wait %d: %s (%d b)\n", loadingFile, pState->mLoadingFiles[loadingFile].mUnifiedPath, pState->mLoadingFiles[loadingFile].mFileSize );
               break;
            }

            //construct asset.
            flatfs_log_printf( "end_kp %d cnt %d: %s (%d b)\n", loadingFile, pState->mFileLoadingCount, pState->mLoadingFiles[loadingFile].mUnifiedPath, pState->mLoadingFiles[loadingFile].mFileSize );
            end_load_kp_file( loadingFile, pState );
         }

         
         //
         // Now we try to start new loads
         //

         // if pState->mpCurrentBPAssetsLine is NULL, then we have nothing else to pend loading
         if ( pState->mpCurrentManifestLine )
         {
            for ( 
               int loadIndex = loading_fifo_prepush_loading( pState ); 
               loadIndex != -1; 
               loadIndex = loading_fifo_pushloading_prepush_next( pState ) )
            {
               // Since lines can be skipped as we're reading, we need to read one line at a time
               // and see if we should skip it.

               int skipFile = 0;
               do 
               {
                  // Default to not skip
                  skipFile = 0;

                  //Next asset in the manifest file, if any.
                  pState->mpCurrentManifestLine = strchr( pState->mpCurrentManifestLine, '\n' ) + 1;

                  if( *pState->mpCurrentManifestLine == '\0' )
                  {
                     //Done with this stage.
                     free( pState->mpManifestData );
                     pState->mpManifestData = pState->mpCurrentManifestLine = NULL;

                     // We have no more manifest data, we need to check for this at the bottom of the while loop
                     break;
                  }
                  else
                  {
                     // Check to see if we should skip this file
                     char unifiedPath[FILENAME_MAX];
                     char loadPath[FILENAME_MAX];
                     char relativePath[FILENAME_MAX];
                     skipFile = BP_SplitManifestLine( pState->mpCurrentManifestLine, "", unifiedPath, loadPath, relativePath );
                  }

               } while( skipFile );

               if ( pState->mpManifestData )
               {
                  // If we still have manifest data, we load it here.

                  begin_load_kp_file( loadIndex, pState );
                  flatfs_log_printf( "begin_kp %d cnt %d: %s (%d b)\n", loadIndex, pState->mFileLoadingCount, pState->mLoadingFiles[loadIndex].mUnifiedPath, pState->mLoadingFiles[loadIndex].mFileSize );
                  doneWithLoaded = false;
               }
               else
               {
                  // Break out of the loop here if we're at the end of the manifest.
                  // We have to break here rather than the loop conditional so we don't push an extra load state

                  break;
               }
            }
         }

         flatfs_log_printf( "kp loop end: cnt: %d dwl: %d\n",
            pState->mFileLoadingCount,
            doneWithLoaded );

         if ( doneWithLoaded )
         {
            BPE_VERIFY( pState->mFileLoadingCount == 0, false, "Stale files?!" );

            pState->mLoadState = kFlatFSLoadState_Idle;

            // Done with loading!  Return success, which is 0
            return 0;
         }
         else
         {
            // Otherwise we are still loading, return 1, which means we're still working
            return 1;
         }
      } // end case kFlatFSLoadState_ReadingKPFiles
   }

   BP_BREAK; //TODO!
   return 0;
}

void BP_LoadCompleteFlatFS( SFlatFSLoadState * pState )
{
   BP_VerifyEmptyStageAssetCache( pState->mAssetCacheTag );
   pState->mAssetCacheTag = 0;
}

int BP_IsDelimiter( const char c )
{
   return ( (c == ' ') || (c == '\t') || (c == ',') );
}

int BP_IsEndOfElement( const char c )
{
   return( (c == '\0') || (c == '\r') || (c == '\n') || (c == ',') );
}

const char* BP_SkipDelimiters( const char* srcLine )
   {
   // Skip element delimiters ready for reading next element
   int src = 0;
   while( BP_IsDelimiter(srcLine[src]) )
   {
      src++;
   }

   // Return next parse position
   return &srcLine[src];
}

const char* BP_ParseElement( const char* srcLine, char* dstPath )
{
   // Copy element
   int src = 0;
   int dst = 0;
   while(!BP_IsEndOfElement(srcLine[src]) )
   {
      dstPath[dst++] = srcLine[src++];
   }

   // Terminate element
   dstPath[dst] = 0;

   // Return next element
   return BP_SkipDelimiters( &srcLine[src] );
}

int BP_SplitManifestLine( char const *manifestLine, char const *manifestRoot, char *unifiedPath, char *loadPath, char *relativePath )
{
   char fileFps[FILENAME_MAX];

   // "manifestLine" coming in is not a line, but just a position in the rest of the manifest file
   // It either ends in a newline or NULL
   // Also, it is either 
   //   unifiedPath,loadPath,relativePath,fps(optional)
   // or just
   //   relativePath

   // Parse elements
   const char* parse = BP_SkipDelimiters(manifestLine);
   parse = BP_ParseElement( parse, unifiedPath );
   parse = BP_ParseElement( parse, loadPath );
   parse = BP_ParseElement( parse, relativePath );
   parse = BP_ParseElement( parse, fileFps );

   // Just one parameter?
   if( loadPath[0] == 0 )
   {
      BP_ParseElement( manifestLine, relativePath );

      strcpy( unifiedPath, manifestRoot );
      strcat( unifiedPath, relativePath );

      strcpy( loadPath, unifiedPath );
   }

   // Check fps restriction?
   if( fileFps[0] )
   {
      // Skip file if fps spec does not match runtime fps
      const char* runtimeFps = BP_IsPAL() ? "50HZ" : "60HZ";
      if( strcmp( fileFps, runtimeFps ) != 0 )
      {
         return true;
      }
}

   // Load file
   return false;
}

void BP_GetAssetLoadFullPath( char * const fullPath, char * const unifiedFullPath, const char * const manifestLoadPath, const char * const manifestUnifiedPath )
{
   int isPsarcMounted = BP_IsPsarcMounted();
#if !defined(BP_VITA)
   if ( isPsarcMounted )
   {
      strcpy( fullPath, manifestLoadPath );
      strcpy( unifiedFullPath, manifestUnifiedPath );
   }
   else
   {
      strcpy( fullPath, manifestUnifiedPath );
      strcpy( unifiedFullPath, manifestUnifiedPath );
   }
#else
   // On vita we always use the unified path.
   strcpy( fullPath, manifestUnifiedPath );
   strcpy( unifiedFullPath, manifestUnifiedPath );
#endif

#if BP_USE_NEW_FONT_SYSTEM()
   if( strstr( fullPath, ".gcx" ) )
   {
      //Look for a BP rebuilt version of this file, load it if present.
      //This exists check likely has a small performance impact when we're connected to the host PC FS, so limit it to gcx files.
      //TODO: can just update the manifest file for these to remove this step when the time is more convenient.
      char fullPath_JustFilename[FILENAME_MAX];
      char * const fullPath_LastSlash = strrchr( fullPath, '/' );
      strcpy( fullPath_JustFilename, fullPath_LastSlash+1 );
      strcpy( fullPath_LastSlash+1, BP_REBUILTDATA_FOLDER "/" );
      strcat( fullPath_LastSlash+1, fullPath_JustFilename );

      //
      char unifiedFullPath_JustFilename[FILENAME_MAX];
      char * const unifiedFullPath_LastSlash = strrchr( unifiedFullPath, '/' );
      strcpy( unifiedFullPath_JustFilename, unifiedFullPath_LastSlash+1 );
      strcpy( unifiedFullPath_LastSlash+1, BP_REBUILTDATA_FOLDER "/" );
      strcat( unifiedFullPath_LastSlash+1, unifiedFullPath_JustFilename );

      //
      if( !BP_FileExists( fullPath, unifiedFullPath ) )
      {
         //No BP rebuilt file.  Revert the filename.
         strcpy( fullPath_LastSlash+1, fullPath_JustFilename );
         strcpy( unifiedFullPath_LastSlash+1, unifiedFullPath_JustFilename );
      }
   }
#endif

   if( !isPsarcMounted )
   {
      // Determine if a SKU/language specific override file exists and use that one instead.
      // NOTE: This is ONLY needed when running without an archive, because the tool that builds the archives contains the same logic.
      BP_GetPathPlatformSKUOverride(fullPath, unifiedFullPath);
   }
}

void BP_CancelLoadFlatFSReturnFileData( SFlatFSLoadState *pState, void **ppFileDatas, int *pFileDatasCount )
{
   // Cancels the load in-progress and frees everything that the load process
   // would have allocated

   int fileDatasCount = 0;

   for ( int i = 0; i < BPE_ARRAY_SIZE( pState->mLoadingFiles ); ++i )
   {
      if ( pState->mLoadingFiles[i].mpFileHandle )
      {
         if ( pState->mLoadingFiles[i].mpFileOp )
         {
            BP_CancelFileOp( pState->mLoadingFiles[i].mpFileOp );
         }

         BP_CloseFile( pState->mLoadingFiles[i].mpFileHandle );
      }

      if ( pState->mLoadingFiles[ i ].mpFileData )
      {
         ppFileDatas[ fileDatasCount ++ ] = pState->mLoadingFiles[i].mpFileData;
      }
   }

   *pFileDatasCount = fileDatasCount;

   if ( pState->mpManifestData )
   {
      free( pState->mpManifestData );
      BP_IgnoreThisTemporaryData( pState->mpManifestData, ppFileDatas, fileDatasCount );
      pState->mpManifestData = NULL;
   }

   if ( pState->mpBPAssetsData )
   {
      free( pState->mpBPAssetsData );
      BP_IgnoreThisTemporaryData( pState->mpBPAssetsData, ppFileDatas, fileDatasCount );
      pState->mpBPAssetsData = NULL;
   }

   // Note that the mpFileData is explicitly NOT freed here.  That's
   // because the filedata is can be allocated and freed outside
   // of the flatfs system
}

void BP_IgnoreThisTemporaryData( void *ptr, void **ppFileDatas, int fileDatasCount )
{
   for ( int i = 0; i < fileDatasCount; ++i )
   {
       if ( ppFileDatas[i] == ptr )
       {
          ppFileDatas[i] = NULL;
       }
   }
}

void BP_FreeTemporaryFlatFSFileData( void **ppFileDatas, int fileDatasCount )
{
   for ( int i = 0; i < fileDatasCount; ++i )
   {
      free( ppFileDatas[i] );
   }
}
