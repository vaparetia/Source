#include "Engine/Stdafx.h"
// #include "Engine/Basics/BPEEnvironment.h"
// #include "Engine/Basics/BPEAssert.h"
// #include "Engine/StlExtras/BPEStlExtras.h"

#include "Engine/Graphics/CColor.h"

#include "Renderer/Base/Backend/CRenderBackend.h"

#include "BP_BuildDefines.h"
#include "BP_MovieSupport.h"
#include "BP_Renderer.h"
#include "BP_RendererDebug.h"
#include "BP_UIAdjust.h"
#include "BP_FileSupport.h"
#include "BP_Memory.h"

//----------------------------------------------------------------------------
// Stubs

#if !defined(BP_360) && !defined(BP_PS3) && !defined(BP_VITA)

void BP_Movie_Init()
{
}

void BP_Movie_Shutdown()
{
}

BP_Movie_Handle BP_Movie_Create(int x, int y, int width, int height, DG_TEX_TRANS* pTexTrans,
   BP_Movie_Render_Callback renderCallback, int isFullscreenMovie, char* pMemoryStream, int memorySize)
{
   return 0;
}

void BP_Movie_RenderActiveMovies(int const shouldDraw)
{
}

void BP_Movie_ResetTextureToBlack(BP_Movie_Handle hMovie)
{
}

void BP_Movie_Destroy(BP_Movie_Handle hMovie)
{
}

void BP_Movie_Set_Time(BP_Movie_Handle hMovie, float time)
{
}

float BP_Movie_Get_Frame_Time(BP_Movie_Handle hMovie)
{
   return 0.0f;
}

void BP_Movie_Start(BP_Movie_Handle hMovie, unsigned int movieDataSizeInBytes)
{
}

void BP_Movie_Stop(BP_Movie_Handle hMovie)
{
}

void BP_Movie_Reset(BP_Movie_Handle hMovie)
{
   BP_Movie_Stop(hMovie);
}

int BP_Movie_Get_State(BP_Movie_Handle hMovie)
{
   return MOVIE_STATE_STOPPED;
}

DG_TEX* BP_Movie_Get_Texture(BP_Movie_Handle hMovie)
{
   return NULL;
}

int BP_Movie_IsPlayingFullscreen()
{
   return 0;
}

#endif

//----------------------------------------------------------------------------

enum EMovieMode
{
   kMM_OriginalStretched,
   kMM_Original43,
   kMM_Fullscreen,
   kMM_FullscreenAspectCentered,
   kMM_FullscreenAspectOriginal
};

int gBP_Movie_DisplayMode = kMM_Fullscreen;
int gBP_Movie_ShowStats = 0;

namespace
{
   // original aspect ratio was 640x343, apparently
   float const skOriginalAspectRatio = 640.0f/343.0f;
   float const skOriginalBorderRatio = 4.0f/13.0f;
};

void BP_Movie_InitDebugMenu()
{
#if BP_ENABLE_DEBUG_MENU
   int const menu = BP_DebugMenu_AddMenu("Movie", -1);

   static const char* sMovieModes[] = { "Original (16:9)", "Original (4:3)", "Fullscreen", "Fullscreen (Aspect Correct, Centered)", "Fullscreen (Aspect Correct, Maintain Cinebar Ratio)" };
   BP_DebugMenu_AddEnum(menu, "Display Mode", sMovieModes, &gBP_Movie_DisplayMode, 0, 4);

   BP_DebugMenu_AddBool(menu, "Show Stats", &gBP_Movie_ShowStats);
#endif
}

//----------------------------------------------------------------------------

int BP_Movie_GetMovieCoordinates(int x, int y, int width, int height, float* x0, float* y0, float* x1, float* y1)
{
   int result;

   int displayMode = gBP_Movie_DisplayMode;

   if( width == 512 && height == 320 )
   {
      result = 1;
   }
   else
   {
      displayMode = kMM_OriginalStretched;
      result = 0;
   }

   switch(displayMode)
   {
   case kMM_OriginalStretched:
      // Displays original image stretching it to fit the widescreen image
      {
         *x0 = BP_ADJUST_SCREEN_X(x);
         *y0 = BP_ADJUST_SCREEN_Y(y);

         *x1 = BP_ADJUST_SCREEN_X(x + width);
         *y1 = BP_ADJUST_SCREEN_Y(y + height);
      }
      break;
   
   case kMM_Original43:
      // Displays original image with black bars on left/right sides to make it be at the correct aspect ratio.
      {
         *x0 = BP_ADJUST_SCREEN_X(BP_AdjustWidescreenCenterX(x, DRAW_WIDTH));
         *y0 = BP_ADJUST_SCREEN_Y(y);

         *x1 = BP_ADJUST_SCREEN_X(BP_AdjustWidescreenCenterX(x + width, DRAW_WIDTH));
         *y1 = BP_ADJUST_SCREEN_Y(y + height);
      }
      break;

   case kMM_Fullscreen:
      {
         *x0 = 0.0f;
         *x1 = (float)BP_REAL_SCREEN_X;
         *y0 = 0.0f;
         *y1 = (float)BP_REAL_SCREEN_Y;
      }
      break;

   case kMM_FullscreenAspectCentered:
      // Displays original image with correct aspect scaling it up to fit the screens left/right borders
      // Vertically it is centered on screen
      {
         static float const skRealX = BP_REAL_SCREEN_X;
         static float const skRealY = BP_REAL_SCREEN_Y;
         if (skRealX/skRealY < skOriginalAspectRatio)
         {
            static float const skBorderY = skRealY - skRealX/skOriginalAspectRatio;
            // top and bottom borders
            *x0 = 0.0f;
            *x1 = skRealX;
            *y0 = skBorderY/2.0f;
            *y1 = skRealY - skBorderY/2.0f;
         }
         else
         {
            static float const skBorderX = skRealX - skRealY*skOriginalAspectRatio;
            *x0 = skBorderX/2.0f;
            *x1 = skRealX - skBorderX/2.0f;
            *y0 = 0.0f;
            *y1 = skRealY;
         }
      }
      break;
   
   case kMM_FullscreenAspectOriginal:
      // Displays original image with correct aspect scaling it up to fit the screens left/right borders
      // Vertically it is positioned to maintain the relative size of the original cinema bars
      {
         static float const skRealX = BP_REAL_SCREEN_X;
         static float const skRealY = BP_REAL_SCREEN_Y;
         if (skRealX/skRealY < skOriginalAspectRatio)
         {
            static float const skBorderY = skRealY - skRealX/skOriginalAspectRatio;
            // top and bottom borders
            *x0 = 0.0f;
            *x1 = skRealX;
            *y0 = skBorderY*skOriginalBorderRatio;
            *y1 = skRealY - skBorderY*(1.0f - skOriginalBorderRatio);
         }
         else
         {
            static float const skBorderX = skRealX - skRealY*skOriginalAspectRatio;
            *x0 = skBorderX/2.0f;
            *x1 = skRealX - skBorderX/2.0f;
            *y0 = 0.0f;
            *y1 = skRealY;
         }
      }
      break;
   }

   return result;
}

//----------------------------------------------------------------------------

int gBP_CinemaBarsEnabled = 0;

extern void BP_Movie_SetCinemabarsDisabled(int moviePlayerUniqueId, int enabled)
{
   int const mask = (1 << moviePlayerUniqueId);

   if( enabled )
      gBP_CinemaBarsEnabled |= mask;
   else
      gBP_CinemaBarsEnabled &= ~mask;
}

extern int BP_Movie_IsCinemabarsDisabled()
{
   return gBP_CinemaBarsEnabled;
}


//----------------------------------------------------------------------------

int const kBufferSize = 256 * 1024;
int const kBufferCount = 2;

class CStandaloneMoviePlayer
{
   class CReadBufferAndManager
   {
   public:
      CReadBufferAndManager()
      :  mCurrentFileOp(NULL)
      ,  mCurrentFileOpReadSize(0)
      ,  mCommittedDataReadOffset(0)
      ,  mCommittedDataTotalSize(0)
      ,  mpMemory((char*) BP_Memory_Alloc( kBufferSize, 16, kMT_Permanent, kMC_Movie ) )
      {
      }

      ~CReadBufferAndManager()
      {
         if ( mCurrentFileOp != NULL )
         {
            BP_CancelAndDeleteFileOpSync( mCurrentFileOp );
         }

         BP_Memory_Free(mpMemory);
      }

      bool HasMoreData() const
      {
         return (mCommittedDataReadOffset < mCommittedDataTotalSize);
      }

      bool IsReadInProgress() const 
      {
         return (mCurrentFileOp != NULL);
      }

      int TryFinishRead()
      {
         if( mCurrentFileOp != NULL )
         {
            int result = BP_TryFinishFileOp(mCurrentFileOp);
            
            if( result )
            {
               mCommittedDataReadOffset = 0;
               mCommittedDataTotalSize = mCurrentFileOpReadSize;

               mCurrentFileOp = NULL;
               mCurrentFileOpReadSize = 0;
            }

            return result;
         }

         return 0;
      }

      void BeginRead(const char * const pFilename, int const offset, int const size)
      {
         BPE_VERIFY(mCurrentFileOp == NULL, false, "Invalid begin read.");

         mCurrentFileOp = BP_ReadFileByFilenameAsync(pFilename, mpMemory, offset, size);
         mCurrentFileOpReadSize = size;
         mCommittedDataTotalSize = 0;
      }

      int const GetCommittedDataTotalSize() const { return mCommittedDataTotalSize; }
      int const GetCommittedDataReadOffset() const { return mCommittedDataReadOffset; }
      int const GetCommittedDataRemainingSize() const { return mCommittedDataTotalSize - mCommittedDataReadOffset; }

      char *ConsumeCommittedMemory( int const amount )
      {
         char *pRet = mpMemory + mCommittedDataReadOffset;

         mCommittedDataReadOffset += amount;

         BPE_VERIFY( mCommittedDataReadOffset <= mCommittedDataTotalSize, false, "Read too much memory!" );

         return pRet;
      }

   private:
      void* mCurrentFileOp;
      int   mCurrentFileOpReadSize;

      int   mCommittedDataReadOffset;
      int   mCommittedDataTotalSize;
      char* mpMemory;

      BPE_DISABLE_COPY_AND_ASSIGNMENT( CReadBufferAndManager );
   };

public:
   CStandaloneMoviePlayer(BP_Movie_Handle handle, const char * const filename)
   :  mHandle(handle)
   ,  mFilename(filename)
   ,  mCurrentFileOffset(0)
   ,  mCurrentReadBuffer(0)
   ,  mCurrentStreamBuffer(0)
   ,  mIsEOF(0)
   {
      void* fileHandle = BP_OpenFile(filename, NULL);
      mFileSize = BP_GetFileSize(fileHandle);
      BP_CloseFile(fileHandle);
   }

   ~CStandaloneMoviePlayer()
   {
   }

   void UpdateBuffers()
   {
      CReadBufferAndManager & streamBuffer = mBuffers[mCurrentStreamBuffer];

      if( !streamBuffer.IsReadInProgress() && !streamBuffer.HasMoreData() )
      {
         int const currentReadSize = bpe::min_val(mFileSize - mCurrentFileOffset, kBufferSize);
         if( currentReadSize > 0 )
         {
            streamBuffer.BeginRead(mFilename, mCurrentFileOffset, currentReadSize);
            mCurrentFileOffset += currentReadSize;
         }
         else
         {
            mIsEOF = 1;
         }
      }

      if( streamBuffer.IsReadInProgress() )
      {
         if( streamBuffer.TryFinishRead() )
         {
            mCurrentStreamBuffer = (mCurrentStreamBuffer + 1) % kBufferCount;
         }
      }
   }

   void GetData(unsigned char* pBuffer, int* pOutBytesRead)
   {
      UpdateBuffers();

      {
         int availableForConsumption = 0;

         for( int i = 0; i < kBufferCount; ++i )
         {
            if( !mBuffers[i].IsReadInProgress() )
               availableForConsumption += mBuffers[i].GetCommittedDataRemainingSize();
         }

         if( gBP_Movie_ShowStats )
         {
            printf("Standalone Player: Read Buffers: %d%%\n", availableForConsumption * 100 / (kBufferSize*kBufferCount));
         }
      }

      int const kMaxDataSize = 64 * 1024;
      
      CReadBufferAndManager & currentBuffer = mBuffers[mCurrentReadBuffer];
      
      if( currentBuffer.HasMoreData() )
      {
         int const remainingDataInBuffer = currentBuffer.GetCommittedDataRemainingSize();
         int const amountOfDataToGrab = bpe::min_val(remainingDataInBuffer, kMaxDataSize);

         memcpy(pBuffer, currentBuffer.ConsumeCommittedMemory( amountOfDataToGrab ), amountOfDataToGrab);
         *pOutBytesRead = amountOfDataToGrab;

         // Did we completely consume this buffer?
         if( !currentBuffer.HasMoreData() )
         {
            mCurrentReadBuffer = (mCurrentReadBuffer + 1) % kBufferCount;
         }
         
         return;
      }
      else if( mCurrentReadBuffer == mCurrentStreamBuffer )
      {
         // We have caught up with the streaming buffer, we need to wait until it finishes reading the data.
         *pOutBytesRead = 0;
      }

      // EOF?
      if( mIsEOF )
      {
         *pOutBytesRead = -1;
         return;
      }
   }

   int GetMovieFileSize() const { return mFileSize; }

public:
   BP_Movie_Handle   mHandle;

private:
   const char*       mFilename;

   int               mCurrentFileOffset;
   int               mFileSize;
   int               mIsEOF;

   int               mCurrentReadBuffer;
   int               mCurrentStreamBuffer;
   CReadBufferAndManager           mBuffers[kBufferCount];
};

static CStandaloneMoviePlayer* gMoviePlayer = NULL;

static void Standalone_Player_ReadCallback(unsigned char* pBuffer, int* pOutBytesRead)
{
   if( gMoviePlayer )
      gMoviePlayer->GetData(pBuffer, pOutBytesRead);
}

BP_Movie_Handle BP_Movie_CreateStandalonePlayer(const char* pFilename)
{
   BPE_VERIFY(gMoviePlayer == NULL, false, "Invalid call to BP_Movie_CreateStandalonePlayer");

   BP_Movie_Handle handle = BP_Movie_Create(0, 0, BP_REAL_SCREEN_X, BP_REAL_SCREEN_Y, NULL, &Standalone_Player_ReadCallback, 1 /*isFullscreenMovie*/, NULL /*pMemoryStream*/, 0 /*memorySize*/);
   
   gMoviePlayer = new CStandaloneMoviePlayer(handle, pFilename);

   return handle;
}

void BP_Movie_DestroyStandalonePlayer(BP_Movie_Handle handle)
{
   BPE_VERIFY(gMoviePlayer != NULL && gMoviePlayer->mHandle == handle, false, "Invalid BP_Movie_DestroyStandalonePlayer" );

   delete gMoviePlayer;
   gMoviePlayer = NULL;

   BP_Movie_Destroy(handle);
}

void BP_Movie_StandalonePlayer_Play(BP_Movie_Handle handle)
{
   BPE_VERIFY(gMoviePlayer != NULL && gMoviePlayer->mHandle == handle, false, "Invalid movie handle." );
   BP_Movie_Start(gMoviePlayer->mHandle, gMoviePlayer->GetMovieFileSize());
}
