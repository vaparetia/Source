//-----------------------------------------------------------------------------
// BP_TextureTool.cpp
//-----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "Engine/Math/BPETypeConversion.inl"
#include "Engine/System/CStopWatch.h"

#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "BP_Renderer.h"

#include "BP_RendererDebug.h"
#include "BP_BaseRenderer.h"
#include "BP_FileSupport.h"

#include "MGS_Common.h"

#include "libgv.h"

#include "BP_TextureTool.h"
#include "BP_DebugPad.h"
#include "BP_RenderFX.h"

#ifdef BP_PS3
#include "cell/fs/cell_fs_file_api.h"
#endif

#include "Engine/Mechanics/TinyXml/tinyxml.h"
#include "Engine/Mechanics/TinyXml/xpath_processor.h"

#include <algorithm>

using namespace TinyXPath;

#ifndef _MSC_VER
#define stricmp                  strcasecmp
#endif

// paths to remove from texture file names.
#if defined( BP_360 )
#define PATH_PLATFORM            "_360"
#elif defined( BP_PS3 )
#define PATH_PLATFORM            "_ps3"
#elif defined( BP_WIN32 )
#define PATH_PLATFORM            "_win"
#elif defined( BP_VITA )
#define PATH_PLATFORM            "_vta"
#else
#  error Unknown platform!
#endif

#define TEX_LIST_FILE_NAME        "texture_select_list.txt"

// Log2 of the maximum supported texture size.
#define MAX_TEX_SIZE_LOG2        12

// map input flags to PS3 buttons.
#define PS3_LEFT                 BP_PAD_L
#define PS3_RIGHT                BP_PAD_R

#if MGS_VERSION==3
#  define PS3_X                    DEBUG_PAD_B
#  define PS3_C                    DEBUG_PAD_A
#  define PS3_S                    _PAD_Y
#  define PS3_T                    _PAD_X
#  define PS3_L1                   _PAD_L1
#  define PS3_L2                   _PAD_L2
#  define PS3_R1                   _PAD_R1
#  define PS3_R2                   _PAD_R2
#elif MGS_VERSION==2
#  define PS3_X                    PAD_B
#  define PS3_C                    PAD_A
#  define PS3_S                    PAD_Y
#  define PS3_T                    PAD_X
#  define PS3_L1                   PAD_L1
#  define PS3_L2                   PAD_L2
#  define PS3_R1                   PAD_R1
#  define PS3_R2                   PAD_R2
#endif

// abstraction of button mappings.
#define PS3_SELECT_PREV          PS3_LEFT
#define PS3_SELECT_NEXT          PS3_RIGHT
#define PS3_RELOAD               PS3_C
#define PS3_SELECT               PS3_X
#define PS3_SAVE_SELECTION       PS3_S
#define PS3_TOGGLE_SELECT_MODE   PS3_T
#define PS3_RELOAD_ALL           PS3_L1
#define PS3_RELOAD_CHANGED       PS3_L2
#define PS3_VIEW_EXPLORER        PS3_R1
#define PS3_VIEW_HISTORY         PS3_R2

#define TEXTURE_RELOAD_MSG_SHOW_COUNT        200
#define TEXTURE_SAVED_MSG_SHOW_COUNT         200

#define BINARY_SEARCH_MODE_TEXT  "Binary selection mode."
#define SEQUENTIAL_SEARCH_MODE_TEXT "Sequential selection mode."
#define TEXTURE_SELECTION_RELOADED_TEXT      "The selected texture was reloaded successfully."
#define TEXTURE_MODIFIED_RELOADED_TEXT       "Modified textures were reloaded successfully."
#define TEXTURE_ALL_RELOADED_TEXT            "All textures were reloaded successfully."
#define TEXTURE_SAVED_TEXT       "Texture path saved to '"TEX_LIST_FILE_NAME"'."

#define TEXTURE_RELOAD_ALL       1
#define TEXTURE_RELOAD_MODIFIED  2
#define TEXTURE_RELOAD_SELECTED  3

#ifdef min
#undef min
#endif

//-----------------------------------------------------------------------------

#if BP_ENABLE_TEXTURE_TOOL

extern "C"
{
   extern int gBP_DebugStageAction;
}

//-----------------------------------------------------------------------------
// STORAGE
//-----------------------------------------------------------------------------

namespace 
{
   // Types
   typedef std::set<unsigned int>            TextureSet;
   typedef std::map<int, DG_TEX*>            IndexToTexture;
   typedef std::map<DG_TEX*, u_long64>       TextureToTimeStamp;
   typedef std::map<std::string, std::string> RsrcNameToFlatListName;
   typedef std::map<unsigned int, CBaseTexture*>  DimToSearchGridTexture;
   typedef std::map<int, int>                IndexToSelection;

   enum ESelectMode
   {
      ESM_SEQUENTIAL,
      ESM_BINARY,
   };

   // Data
   int                                       mTexToolDebugMenu = -1;
   CBaseTexture*                             mpGridTexture = NULL;
   CBaseTexture*                             mpGridDeleteTexture = NULL;
   DimToSearchGridTexture                    mDimToSearchGridTextures;
   DimToSearchGridTexture                    mDimToSearchDeleteGridTextures;
   IndexToSelection                          mIndexToSelection;
   TextureSet                                mTextureSet;
   TextureToTimeStamp                        mTextureToTimeStamp;
   RsrcNameToFlatListName                    mRsrcNameToFlatListName;
   int                                       mFlashCount = 0;
   unsigned int                              mSelectedIndex = 0;
   int                                       mSearchRangeMin = 0;
   int                                       mSearchRangeMax = 0;
   ESelectMode                               mSelectMode = ESM_SEQUENTIAL;
   int                                       mBinarySelectSet = 0;
   int                                       mTexturesReloadedCounter = 0;
   int                                       mTexturesReloadedMode = 0;
   int                                       mTextureSavedCounter = 0;
   unsigned int                              mMetaDataTextureIndex = 0;
   char                                      mMetaData[ 1024 ];
   int                                       mEnabled = 0;
   int                                       mPrevEnabled = 0;

   // buffer used for strings.
   char                                      mStringBuf[ 1024 ];

   // calculate the pivot value for the binary search.
   int CalcSearchPivot( int rangeMin, int rangeMax )
   {
      return rangeMin + ( ( rangeMax - rangeMin + 1 ) >> 1 );
   }

   // builds a TGA path from a given texture path.  Note that the TGA path
   // returned is only valid until this function is called again.  Also, this
   // function does not guarantee that a TGA path returned is that of a valid
   // TGA file (the path returned may not point to an actual file).
   const char* BuildTextureTGAPath( const char* path )
   {
      // copy the original path into our string buffer so that we can remove
      // the platform path.
      strcpy( mStringBuf, path );

      // This is kind of hacky...
      // remove the intermediate path if necessary.  For paths that show up
      // in the flat list (presumably all of them?), this is not necessary.
      char* platformPath = strstr( mStringBuf, PATH_PLATFORM );
      if ( platformPath )
      {
         // when advancing past the platform path, we add 1 to account for the
         // trailing '/'.
         char* remaining = platformPath + strlen( PATH_PLATFORM ) + 1;
         memmove( platformPath, remaining, strlen( remaining ) + 1 );
      }

      // copy the flat-list path into the string buffer if possible.  If not,
      // simply use the original file path.
      RsrcNameToFlatListName::iterator iter =
         mRsrcNameToFlatListName.find( mStringBuf );
      if ( iter != mRsrcNameToFlatListName.end() )
         strcpy( mStringBuf, iter->second.c_str() );

      // change the extension to tga.  We do this by first removing the
      // existing extension, then appending the new one.
      char* ext = strrchr( mStringBuf, '.' );
      if ( ext != 0 )
         *ext = '\0';
      strcat( mStringBuf, ".tga" );

      // return the new path.
      return mStringBuf;
   }

   // finds the timestamp of the coresponding TGA for the specified DG_TEX.
   u_long64 FindTGATimeStamp(DG_TEX* tex, const char** tgaFilePath)
   {
      // get the original texture name.
      const char* path = BP_GetTextureNameByMGSAddr((unsigned int)tex);

      // grab the coresponding file's TGA counterpart.
      const char* tgaPath = BuildTextureTGAPath( path );

      // get the TGA file's timestamp.  Note that we have to check the
      // result as the TGA file in question may not exist.  If the file
      // doesn't exist, then we record a timestamp of 0.  If a TGA file
      // eventually shows up, then we'll assume it's a replacement.
      u_long64 timeStamp = 0;
      if ( BP_FileExists( tgaPath, NULL ) )
         timeStamp = BP_GetFileModificationDate( tgaPath );

      // check to see if the caller would like the name of the TGA file.
      if ( tgaFilePath )
         *tgaFilePath = tgaPath;

      // return the TGA file's timestamp.
      return timeStamp;
   }
};

//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

CBaseTexture* BP_TextureTool_GetGridTexture(const int width, const int height)
{
   // Use cached texture?
   if( ( mpGridTexture ) && ( mpGridTexture->GetWidth() == width ) && ( mpGridTexture->GetHeight() == height ) )
   {
      return mpGridTexture;
   }

   // Delete old grid texture?
   if( mpGridDeleteTexture )
   {
      CBaseTexture::DeleteTexture(mpGridDeleteTexture);
   }

   // Mark current grid texture for delete
   mpGridDeleteTexture = mpGridTexture;

   // Create new texture
   CColor fg( 255, 0, 255, 128 );
   CColor bg( 255, 255, 255, 128 );
   mpGridTexture = CBaseTexture::CreateCheckerboard(width, height, fg, bg);

   // return the new grid texture.
   return mpGridTexture;
}

//-----------------------------------------------------------------------------

int BP_TextureTool_GetIndexFromTexture(DG_TEX* pTex)
{
   // verify that the texture specified has been accounted for.
   TextureSet::iterator const found = mTextureSet.find((unsigned int)pTex);
   if( found == mTextureSet.end() )
   {
      mTextureSet.insert( TextureSet::value_type( ( unsigned int )pTex ) );
      TextureToTimeStamp::iterator iter = mTextureToTimeStamp.find( pTex );
      if ( iter == mTextureToTimeStamp.end() )
         mTextureToTimeStamp.insert( TextureToTimeStamp::value_type( pTex, FindTGATimeStamp( pTex, 0 ) ) );
   }

   // find usage index.
   return ( int )pTex;
}

//-----------------------------------------------------------------------------

DG_TEX* BP_TextureTool_GetTextureFromIndex(int index)
{
   // only return a valid texture if the texture has been registered.
   if ( mTextureSet.find( index ) == mTextureSet.end() )
      return 0;

   // return the address of the texture as the index.
   return (DG_TEX*)index;
}

//-----------------------------------------------------------------------------

void BP_TextureTool_SelectNext()
{
   // Reset flash count so highlighted texture is visible
   mFlashCount = 0;

   // If there are no textures to select, clear the current selection.
   if( mTextureSet.size() == 0 )
   {
      mSelectedIndex = 0;
      return;
   }

   // search for the first entry with an index that is higher than the current
   // index.
   TextureSet::iterator iter = mTextureSet.begin();
   const TextureSet::iterator end = mTextureSet.end();
   for ( ; iter != end; ++iter )
   {
      if ( mSelectedIndex < *iter )
      {
         mSelectedIndex = *iter;
         return;
      }
   }

   // if nothing comes after the currently selected item, wrap to the front of
   // the list.
   mSelectedIndex = *mTextureSet.begin();
}

//-----------------------------------------------------------------------------

void BP_TextureTool_SelectPrev()
{
   // Reset flash count so highlighted texture is visible
   mFlashCount = 0;

   // If there are no textures to select, clear the current selection.
   if( mTextureSet.size() == 0 )
   {
      mSelectedIndex = 0;
      return;
   }

   // search for the last entry with an index that is lower than the current
   // index.
   TextureSet::reverse_iterator iter = mTextureSet.rbegin();
   const TextureSet::reverse_iterator end = mTextureSet.rend();
   for ( ; iter != end; ++iter )
   {
      if ( mSelectedIndex > ( unsigned int )*iter )
      {
         mSelectedIndex = ( unsigned int )*iter;
         return;
      }
   }

   // if nothing comes after the currently selected item, wrap to the back of
   // the list.
   mSelectedIndex = *mTextureSet.rbegin();
}

//-----------------------------------------------------------------------------

CBaseTexture* BP_TextureTool_GetSearchGridTexture(DG_TEX* pTex)
{
   // get the BP texture object assigned to the specified DG_TEX object.
   CBaseTexture* texture = ( CBaseTexture* )pTex->BP_TextureHandle;

   // get the texture's dimensions.
   unsigned int width = texture->GetWidth();
   unsigned int height = texture->GetHeight();

   // build a hash for the current texture's dimensions.
   unsigned int hash = ( ( height - 1 ) << MAX_TEX_SIZE_LOG2 ) | ( width - 1 );

   // check to see if a texture of the appropriate size already exists.
   DimToSearchGridTexture::iterator iter = mDimToSearchGridTextures.find(hash);
   if ( iter == mDimToSearchGridTextures.end() )
   {
      // create a new search grid texture.
      CColor fg( 255, 0, 255, 128 );
      CColor bg( 255, 255, 255, 128 );
      texture = CBaseTexture::CreateCheckerboard(width, height, fg, bg);

      // store the texture in the map.
      mDimToSearchGridTextures.insert(std::pair<unsigned int, CBaseTexture*>(hash, texture));
   }
   else
   {
      // return the existing texture.
      texture = iter->second;
   }

   // return the texture.
   return texture;
}

//-----------------------------------------------------------------------------

void BP_TextureTool_FlushSearchTextures()
{
   BPE_ASSERT( mDimToSearchDeleteGridTextures.size() == 0, "Textures are being leaked!\n" );
   mDimToSearchDeleteGridTextures = mDimToSearchGridTextures;
   mDimToSearchGridTextures.clear();
}

//-----------------------------------------------------------------------------

void BP_TextureTool_UpdateSelection()
{
   // resize the array to 0.
   mIndexToSelection.clear();

   // clamp the search range in case it's too large for some reason (perhaps
   // the camera moved).
   mSearchRangeMax = std::min( mSearchRangeMax, ( int )mTextureSet.size() );

   // push selected indices onto the array.
   TextureSet::iterator iter = mTextureSet.begin();
   for ( unsigned int i = 0; i < mSearchRangeMax; ++i, ++iter )
   {
      if ( i >= mSearchRangeMin )
         mIndexToSelection.insert( std::pair< int, int >( *iter, i ) );
   }
}

//-----------------------------------------------------------------------------

int BP_TextureTool_GetSearchSelectedIndex()
{
   if ( mIndexToSelection.size() != 1 )
      return -1;
   return mIndexToSelection.begin()->first;
}

//-----------------------------------------------------------------------------

void BP_TextureTool_SelectSet0()
{
   // Reset flash count so highlighted texture is visible
   mFlashCount = 0;

   // check to see if we need to start a new search.
   if ( ( mSearchRangeMax - mSearchRangeMin ) <= 1 )
   {
      // start our binary search, based on the size of the number of objects
      // currently residing in the mIndexToTexture map.
      mSearchRangeMax = mTextureSet.size();
      mSearchRangeMin = 0;
      BP_TextureTool_UpdateSelection();
      return;
   }

   // clamp the search range.
   mSearchRangeMax = std::min( mSearchRangeMax, (int)mTextureSet.size() );
   mSearchRangeMin = std::min( mSearchRangeMin, (int)mTextureSet.size() - 1 );

   // the texture exists in the currently selected range.
   mSearchRangeMax = CalcSearchPivot( mSearchRangeMin, mSearchRangeMax );
   BP_TextureTool_UpdateSelection();

   // check to see if we've found the desired object.
   if ( ( mSearchRangeMax - mSearchRangeMin ) <= 1 )
   {
      mSelectedIndex = BP_TextureTool_GetSearchSelectedIndex();
      BP_TextureTool_FlushSearchTextures();
   }
}

//-----------------------------------------------------------------------------

void BP_TextureTool_SelectSet1()
{
   // Reset flash count so highlighted texture is visible
   mFlashCount = 0;

   // simply do nothing if a binary search is not currently active.
   if ( ( mSearchRangeMax - mSearchRangeMin ) <= 1 )
      return;

   // the texture exists in the currently selected range.
   mSearchRangeMin = CalcSearchPivot( mSearchRangeMin, mSearchRangeMax );

   // clamp the search range to the number of objects currently in the
   // mIndexToTexture map.
   mSearchRangeMax = std::min( mSearchRangeMax, (int)mTextureSet.size() );
   mSearchRangeMin = std::min( mSearchRangeMin, (int)mTextureSet.size() - 1 );
   BP_TextureTool_UpdateSelection();

   // check to see if we've found the desired object.
   if ( ( mSearchRangeMax - mSearchRangeMin ) <= 1 )
   {
      mSelectedIndex = BP_TextureTool_GetSearchSelectedIndex();
      BP_TextureTool_FlushSearchTextures();
   }
}

//-----------------------------------------------------------------------------

int BP_TextureTool_CalcTexSearchIndex( DG_TEX* pTex )
{
   // get the index of the texture passed in.
   int index = BP_TextureTool_GetIndexFromTexture( pTex );

   // determine the selection index of the texture.  If we don't find it,
   // simply return -1 (not selected).
   IndexToSelection::iterator iter = mIndexToSelection.find( index );
   if ( iter == mIndexToSelection.end() )
      return -1;

   // simply return the selection index of the texture.
   return iter->second;
}

//-----------------------------------------------------------------------------

const char* BP_TextureTool_GetTextureMetaData( unsigned int index )
{
   // check to see if we need to reload metadata.
   if ( mMetaDataTextureIndex != index )
   {
      // if the index is not 0, then load the metadata.
      mMetaData[ 0 ] = '\0';
      if ( index != 0 )
      {
         // get the texture's name using it's MGS address.
         const char* path = BP_GetTextureNameByMGSAddr( index );

         // get the current texture's tga path.
         const char* tgaPath = BuildTextureTGAPath( path );
         if ( strlen( tgaPath ) == 0 )
            return "";

         // build the meta data path.
         char pathBuf[ 1024 ];
         strcpy( pathBuf, "/app_home/" );
         strcat( pathBuf, tgaPath );
         strcat( pathBuf, ".meta" );

         // load the XML file using TinyXML.
         TiXmlDocument doc;
         if ( doc.LoadFile( pathBuf ) )
         {
            // find a 'UsingProperty' node with a 'name' attribute whose value
            // is set to 'GenerateMipmaps'.
            TiXmlNode* root = doc.FirstChild( "UsingPropertyContainer" );
            xpath_processor xpath( root, "//UsingProperty[@name='GenerateMipmaps']/Value" );
            expression_result result = xpath.er_compute_xpath();

            // verify that we have at least one matching node.
            node_set* nodes = result.nsp_get_node_set();
            if ( nodes && nodes->u_get_nb_node_in_set() )
            {
               // get the first node's value.
               const TiXmlNode* node = nodes->XNp_get_node_in_set( 0 );
               const TiXmlNode* child = node->FirstChild();
               while ( child != 0 )
               {
                  // get the current element's value.
                  const TiXmlText* text = child->ToText();
                  if ( text != 0 )
                  {
                     // check to see if mipmaps are enabled.
                     const char* value = text->Value();
                     if ( stricmp( value, "kForceOn" ) == 0 || stricmp( value, "kDefault" ) == 0 )
                     {
                        // YES!
                        strcpy( mMetaData, "MipMaps on." );
                     }
                     else if ( stricmp( value, "kForceOff" ) == 0 )
                     {
                        // NO!
                        strcpy( mMetaData, "MipMaps off." );
                     }
                  }

                  // get the next sibling element.
                  child = child->NextSibling();
               }
            }
            else
            {
               strcpy( mMetaData, "MipMaps on." );
            }
         }
         else
         {
            strcpy( mMetaData, "MipMaps on." );
         }
      }

      // store the new index.
      mMetaDataTextureIndex = index;
   }

   // return the metadata.
   return mMetaData;
}

//-----------------------------------------------------------------------------

void BP_TextureTool_Reset()
{
   mSelectedIndex = 0;
   mSearchRangeMin = 0;
   mSearchRangeMax = 0;
   mBinarySelectSet = 0;
}

//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS
//-----------------------------------------------------------------------------

void BP_TextureTool_Init()
{
   // force the recreation of the selected texture list file.
   void* fp = BP_OpenFileReadWrite( TEX_LIST_FILE_NAME, 1 );
   if ( fp )
      BP_CloseFile( fp );

   // add debug menu options.
   mTexToolDebugMenu = BP_DebugMenu_AddMenu( "Texture Tool", -1 );
   BP_DebugMenu_AddBool( mTexToolDebugMenu, "Enable", &mEnabled );
}

//-----------------------------------------------------------------------------

void BP_TextureTool_SetEnable( bool enabled )
{
   mEnabled = enabled ? 1 : 0;
   mRsrcNameToFlatListName.clear();
   mTextureToTimeStamp.clear();
}

//-----------------------------------------------------------------------------

bool BP_TextureTool_GetEnable()
{
   return ( mEnabled != 0 );
}

//-----------------------------------------------------------------------------

int BP_TextureTool_RemapPath( char* path )
{
   // if the texture tool isn't enabled, don't remap the path so that it will
   // load the cooked version of the texture.
   if ( !mEnabled )
      return 0;

   // Lookup extension
   char* ext = path + strlen( path ) - 5;

   // Remap texture?
   if( strcmp( ext, ".ctxr" ) == 0 )
   {
      // Replace with .tga extension
      strcpy( ext, ".tga" );
      return 1;
   }

   if ( stricmp( path, "NULL.tga" ) == 0 )
   {
      bool breakpt = true;
      breakpt = breakpt;
   }

   // return 0 to indicate that the path was not remapped.
   return 0;
}

//-----------------------------------------------------------------------------

void BP_TextureTool_BeginFrame()
{
   // Clear maps
   mTextureSet.clear();
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

void BP_TextureTool_EndFrame()
{
   // reload if necessary.
   if ( mEnabled != mPrevEnabled )
   {
      gBP_DebugStageAction = 0;
      mPrevEnabled = mEnabled;
   }

   // Lookup max number of textures
   if( mTextureSet.size() == 0 || !mEnabled )
   {
      BP_TextureTool_Reset();
      return;
   }

   // end the shader.
   BP_EndShader();

   // check to see if the debug menu is active.  If so, ignore input to the
   // texture tool.
   int debugMenuActive = BP_DebugMenu_IsActive();

   // toggle the current selection mode.
   int port = 1;
   if ( BP_DebugPad_Press( port, PS3_TOGGLE_SELECT_MODE ) && !debugMenuActive )
   {
      // toggle the selection mode.
      mSelectMode = ( mSelectMode == ESM_SEQUENTIAL ) ? ESM_BINARY : ESM_SEQUENTIAL;

      // reset search variables.
      BP_TextureTool_Reset();
      if ( mSelectMode == ESM_BINARY )
         BP_TextureTool_SelectSet0();
   }

   // Controller input
   if ( mSelectMode == ESM_SEQUENTIAL )
   {
      // scan to the previous/next texture.
      if( BP_DebugPad_Repeat( port, PS3_SELECT_PREV ) && !debugMenuActive )
         BP_TextureTool_SelectPrev();
      if ( BP_DebugPad_Repeat( port, PS3_SELECT_NEXT ) && !debugMenuActive )
         BP_TextureTool_SelectNext();
   }
   else
   {
      // change the currently selected set.
      if ( ( mSearchRangeMax - mSearchRangeMin ) > 1 )
      {
         // if nothing is selected, change the selection set.
         if ( BP_DebugPad_Press( port, PS3_SELECT_PREV | PS3_SELECT_NEXT ) && !debugMenuActive )
         {
            mBinarySelectSet ^= 1;
            mFlashCount = 0;
         }
      }
   }

   // check to see if we need to delete textures from the previous frame.
   if ( mDimToSearchDeleteGridTextures.size() > 0 )
   {
      // delete the grid textures.
      DimToSearchGridTexture::iterator iter = mDimToSearchDeleteGridTextures.begin();
      const DimToSearchGridTexture::iterator end = mDimToSearchDeleteGridTextures.end();
      for ( ; iter != end; ++iter )
         CBaseTexture::DeleteTexture(iter->second);

      // clear the grid texture deletion map.
      mDimToSearchDeleteGridTextures.clear();
   }

   // auto search for next texture if current is not valid.
   DG_TEX* pTex = BP_TextureTool_GetTextureFromIndex( mSelectedIndex );
   if( ( mSelectedIndex != 0 ) && ( pTex == NULL ) )
   {
      BP_TextureTool_SelectNext();
      pTex = BP_TextureTool_GetTextureFromIndex( mSelectedIndex );
   }

   // check to see if we need to save the current selection to the text file.
   if ( BP_DebugPad_Press( port, PS3_SAVE_SELECTION ) && !debugMenuActive && pTex != NULL )
   {
      const char* tgaPathName = "";
      FindTGATimeStamp( pTex, &tgaPathName );
      if ( strlen( tgaPathName ) > 0 )
      {
         void* fp = BP_OpenFileReadWrite( TEX_LIST_FILE_NAME, 0 );
         if ( fp != 0 )
         {
            std::string curLine = CStringExtras::Stringize("/%s\n", tgaPathName );
            // Replace '/' with '\' to work with Texture Viewer drag and drop selection
            while (curLine.find('/') != -1)
            {
               curLine[curLine.find('/')] = '\\';
            }

            BP_SeekFile( fp, 0, SEEK_END );
            BP_WriteFile( fp, ( const void* )curLine.c_str(), curLine.length() );
            BP_CloseFile( fp );

            // display the texture saved message.
            mTextureSavedCounter = TEXTURE_SAVED_MSG_SHOW_COUNT;
         }
      }
   }

   // check to see if the current texture was selected.
   if ( BP_DebugPad_Press( port, PS3_SELECT ) && !debugMenuActive )
   {
      // if we're not doing a binary search, or the search has drilled down
      // to a single item, then select the current texture.
      if ( ( mSearchRangeMax - mSearchRangeMin ) > 1 )
      {
         // select the desired set.
         if ( mBinarySelectSet == 0 )
            BP_TextureTool_SelectSet0();
         else
            BP_TextureTool_SelectSet1();
      }
   }

   // reload textures as needed.
   if ( BP_DebugPad_Press( port, PS3_RELOAD ) && !debugMenuActive )
   {
      if ( BP_DebugPad_Status( port, PS3_RELOAD_ALL ) )
      {
         BP_TextureTool_ReloadTextures( false );
         mTexturesReloadedCounter = TEXTURE_RELOAD_MSG_SHOW_COUNT;
         mTexturesReloadedMode = TEXTURE_RELOAD_ALL;
      }
      else if ( BP_DebugPad_Status( port, PS3_RELOAD_CHANGED ) )
      {
         BP_TextureTool_ReloadTextures( true );
         mTexturesReloadedCounter = TEXTURE_RELOAD_MSG_SHOW_COUNT;
         mTexturesReloadedMode = TEXTURE_RELOAD_MODIFIED;
      }
      else if ( pTex )
      {
         const char* tgaPathName = "";
         u_long64 newTimeStamp = FindTGATimeStamp( pTex, &tgaPathName );
         BP_ResetTextureByMGSAddr( ( unsigned int )pTex, tgaPathName );
         mTextureToTimeStamp.insert( TextureToTimeStamp::value_type( pTex, newTimeStamp ) );
         mTexturesReloadedCounter = TEXTURE_RELOAD_MSG_SHOW_COUNT;
         mTexturesReloadedMode = TEXTURE_RELOAD_SELECTED;
      }
   }

#ifdef BP_PS3
   // find the file in windows explorer if necessary.
   if ( BP_DebugPad_Press( port, PS3_VIEW_EXPLORER ) && !debugMenuActive )
   {
      // if there is a selected file, select it in windows explorer.
      if ( mSelectedIndex )
      {
         // find the path of the currently selected MGS texture.
         const char* path = BP_GetTextureNameByMGSAddr( mSelectedIndex );

         // build the TGA path.
         const char* tgaPath = BuildTextureTGAPath( path );
         strcpy( mStringBuf, tgaPath );

         // convert forward slashes to back slashes.
         size_t len = strlen( mStringBuf );
         for ( size_t i = 0; i < len; ++i )
         {
            if ( mStringBuf[ i ] == '/' )
               mStringBuf[ i ] = '\\';
         }

         // build the command line.
         char cmdLine[ 1024 ];
         strcpy( cmdLine, "/app_home/EXEC:explorer.exe /select,\".\\" );
         strcat( cmdLine, mStringBuf );
         strcat( cmdLine, "\"" );

         // execute the command.
         int fd = 0;
         cellFsOpen( cmdLine, 0, &fd, NULL, 0 );
      }      
   }

   // show the file's history if necessary.
   if ( BP_DebugPad_Press( port, PS3_VIEW_HISTORY ) && !debugMenuActive )
   {
      // if there is a selected file, open the history for it.
      if ( mSelectedIndex )
      {
         // find the path of the currently selected MGS texture.
         const char* path = BP_GetTextureNameByMGSAddr( mSelectedIndex );

         // build the command line.
         char p4WinCmdLine[ 1024 ];
         strcpy( p4WinCmdLine, "/app_home/EXEC:p4win.exe -H " );
         strcat( p4WinCmdLine, BuildTextureTGAPath( path ) );

         // execute the command.
         int fd = 0;
         cellFsOpen( p4WinCmdLine, 0, &fd, NULL, 0 );
      }
   }
#endif

   // Show selected texture?
   if ( pTex && pTex->BP_TextureHandle && !debugMenuActive )
   {
      // Display texture name
      float y = gpRenderBackend->GetMainFrameBufferHeight() - 34.0f;
      std::string texName = BP_GetTextureNameByMGSAddr((unsigned int)pTex);

      // build the TGA's path.
      const char* texPath = BuildTextureTGAPath( texName.c_str() );

      // stringize the TGA path and display an output message.
      std::string textureName = CStringExtras::Stringize("Texture: %s", texPath );
      gpRenderBackend->RenderText(textureName.c_str(), 32.0f, y );

      // Print out some useful information about the texture
      std::string metaData = BP_TextureTool_GetTextureMetaData( mSelectedIndex );
      CBaseTexture* pBPTexture = (CBaseTexture*)pTex->BP_TextureHandle;
      std::string textureInfo = CStringExtras::Stringize("[%d x %d] %s", pBPTexture->GetWidth(),
         pBPTexture->GetHeight(), metaData.c_str());

      static float sTextureInfoPosX = 32.0f;
      static float sTextureInfoPosY = 270.0f;
      gpRenderBackend->RenderText(textureInfo.c_str(), sTextureInfoPosX, sTextureInfoPosY );

      // store off the original render target
      SRenderTarget const originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();

      // determine the aspect ratio of the texture.
      int texWidth = pBPTexture->GetWidth();
      int texHeight = pBPTexture->GetHeight();
      float texAspect = texWidth / ( float )texHeight;

      // draw the texture in middle left corner so it's not obstructing any UI elements
      static float sTextureImagePosX = -0.95f;
      static float sTextureImagePosY = -0.375f;
      if ( texAspect > 1.0f )
      {
         float adj = 0.57f - 0.57f / texAspect;
         BP_DrawRectTextureModulate(pBPTexture, sTextureImagePosX, adj + sTextureImagePosY, 0.35f, 0.57f / texAspect,
            CVector4( 1,1,1,1 ), CVector4( 0,0,1,1), 0, 0);
      }
      else if ( texAspect < 1.0f )
      {
         float adj = 0.35f - texAspect * 0.35f;
         BP_DrawRectTextureModulate(pBPTexture, sTextureImagePosX, sTextureImagePosY, texAspect * 0.35f, 0.57f,
            CVector4( 1,1,1,1 ), CVector4( 0,0,1,1), 0, 0);
      }
      else
      {
         BP_DrawRectTextureModulate(pBPTexture, sTextureImagePosX, sTextureImagePosY, 0.35f, 0.57f,
            CVector4( 1,1,1,1 ), CVector4( 0,0,1,1), 0, 0);
      }

      // reset the render target back to the original
      gpRenderBackend->SetRenderTarget(originalRenderTarget);
   }
   BP_EndShader();

   // determine if we're performing a binary search, and if so, provide the
   // user with some details.
   if ( mSelectMode == ESM_BINARY )
   {
      CVector2 extents = gpRenderBackend->GetTextExtents( BINARY_SEARCH_MODE_TEXT );
      gpRenderBackend->RenderText( BINARY_SEARCH_MODE_TEXT,
         gpRenderBackend->GetMainFrameBufferWidth() - extents.GetX() - 32.0f, 16.0f );
   }
   else
   {
      CVector2 extents = gpRenderBackend->GetTextExtents( SEQUENTIAL_SEARCH_MODE_TEXT );
      gpRenderBackend->RenderText( SEQUENTIAL_SEARCH_MODE_TEXT,
         gpRenderBackend->GetMainFrameBufferWidth() - extents.GetX() - 32.0f, 16.0f ); 
   }
   BP_EndShader();

   if ( ( mSearchRangeMax - mSearchRangeMin ) > 1 )
   {
      int searchPivot = CalcSearchPivot( mSearchRangeMin, mSearchRangeMax );

      std::string searchInfo = CStringExtras::Stringize("Binary search active... number of items in selection: %d",
         mSearchRangeMax - mSearchRangeMin );
      float y = gpRenderBackend->GetMainFrameBufferHeight() - 34.0f;
      gpRenderBackend->RenderText(searchInfo.c_str(), 32.0f, y );
   }
   BP_EndShader();

   // display the texture saved message if necessary.
   if ( mTextureSavedCounter > 0 )
   {
      // display the message and adjust the counter.
      CVector2 extents = gpRenderBackend->GetTextExtents( TEXTURE_SAVED_TEXT );
      float x = 0.5f * ( gpRenderBackend->GetMainFrameBufferWidth() - extents.GetX() );
      float y = 16.0f;
      if ( mTexturesReloadedCounter > 0 )
         y += 16;
      gpRenderBackend->RenderText( TEXTURE_SAVED_TEXT, x, y, kRenderFonts_Console,
         CColor( 0xFF, 0xFF, 0, 0xFF ) );
      --mTextureSavedCounter;
   }

   // display the texture reload message if necessary.
   if ( mTexturesReloadedCounter > 0 )
   {
      // determine which message to show.
      char* message = TEXTURE_SELECTION_RELOADED_TEXT;
      if ( mTexturesReloadedMode == TEXTURE_RELOAD_ALL )
         message = TEXTURE_ALL_RELOADED_TEXT;
      else if ( mTexturesReloadedMode == TEXTURE_RELOAD_MODIFIED )
         message = TEXTURE_MODIFIED_RELOADED_TEXT;

      // display the message and adjust the counter.
      CVector2 extents = gpRenderBackend->GetTextExtents( message );
      float x = 0.5f * ( gpRenderBackend->GetMainFrameBufferWidth() - extents.GetX() );
      gpRenderBackend->RenderText( message, x, 16.0f, kRenderFonts_Console,
         CColor( 0xFF, 0xFF, 0, 0xFF ) );
      --mTexturesReloadedCounter;
   }

   // Update flash count
   mFlashCount++;
}

//-----------------------------------------------------------------------------

CBaseTexture * BP_TextureTool_GetBPTexture(DG_TEX* pTex)
{
   // lookup texture
   CBaseTexture* pBPTexture = (CBaseTexture*)pTex->BP_TextureHandle;
   if ( !mEnabled )
      return pBPTexture;

   // find usage index
   int index = BP_TextureTool_GetIndexFromTexture(pTex);

   // flash?
   if ( ( mSearchRangeMax - mSearchRangeMin ) > 1 )  // binary search?
   {
      // check to see if the current texture falls within the search range.
      int searchPivot = CalcSearchPivot( mSearchRangeMin, mSearchRangeMax );
      int searchIndex = BP_TextureTool_CalcTexSearchIndex( pTex );
      if ( searchIndex >= mSearchRangeMin && searchIndex < searchPivot && mBinarySelectSet == 0 && !( mFlashCount & 32 ) )
      {
         // return a grid texture if the texture is currently selected.
         pBPTexture = BP_TextureTool_GetSearchGridTexture( pTex );
      }
      else if ( searchIndex >= searchPivot && searchIndex < mSearchRangeMax && mBinarySelectSet == 1 && !( mFlashCount & 32 ) )
      {
         // return a grid texture if the texture is currently selected.
         pBPTexture = BP_TextureTool_GetSearchGridTexture( pTex );
      }
   }
   else if ( ( mSelectedIndex != 0 ) && ( index == mSelectedIndex ) )
   {
      if ( !( mFlashCount & 32 ) )
      {
         pBPTexture = BP_TextureTool_GetGridTexture( pBPTexture->GetWidth(), pBPTexture->GetHeight() );
      }
   }

   // return BP texture
   return pBPTexture;
}

//-----------------------------------------------------------------------------

void BP_TextureTool_StorePathRemapping(const char* originalPath,
                                       const char* flatPath)
{
   // store the remapping.
   mRsrcNameToFlatListName.insert(
      std::pair< std::string, std::string >( originalPath, flatPath )
      );
}

//-----------------------------------------------------------------------------

void BP_TextureTool_ReloadTextures(bool modifiedOnly)
{
   TextureToTimeStamp::iterator iter = mTextureToTimeStamp.begin();
   const TextureToTimeStamp::iterator end = mTextureToTimeStamp.end();
   for ( ; iter != end; ++iter )
   {
      // get the current texture.
      DG_TEX* pTex = iter->first;
      u_long64 timeStamp = iter->second;

      // get the timestamp for the TGA associated with the current DG_TEX.
      const char* tgaPathName = "";
      u_long64 newTimeStamp = FindTGATimeStamp( pTex, &tgaPathName );

      // check to see if we need to reload the TGA file.
      if ( newTimeStamp > timeStamp || !modifiedOnly )
      {
         // create a new CBaseTexture.
         BP_ResetTextureByMGSAddr( ( unsigned int )pTex, tgaPathName );
         iter->second = newTimeStamp;
      }
   }

}

//-----------------------------------------------------------------------------

void BP_TextureTool_TextureDestroyed(DG_TEX* pTex)
{
   TextureToTimeStamp::iterator iter = mTextureToTimeStamp.find( pTex );
   if ( iter != mTextureToTimeStamp.end() )
      mTextureToTimeStamp.erase( iter );
}

//-----------------------------------------------------------------------------

#endif   //#if BP_ENABLE_TEXTURE_TOOL

