#include "Engine/Stdafx.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

#include "BP_MovieSupport.h"
#include "BP_SoundSupport.h"
#include "BP_Memory.h"
#include "BP_RenderFX.h"
#include "BP_Renderer.h"
#include "BP_RendererDebug.h"
#include "BP_Misc.h"
#include <xaudio2.h>
#include <xmedia2.h>

#include "../../../Builds/DiskBuilds/UseDiskBuild.h"

extern int gBP_Movie_ShowStats;

struct ReadBuffer
{
   char* pBuffer;
   int bufferSize;
   int offset;
   int hasData;

   ~ReadBuffer()
   {
      delete [] pBuffer;
      pBuffer = NULL;
   }
};

struct NewFileHandleReading
{
   static const int kSingleReadSize = 65536;
   static const int kNumReadBuffer = 16;

   ReadBuffer mReadBuffer[kNumReadBuffer];

   int currentWriteBuff;
   int currentReadBuff;

   int mReachedEndOfStream;
   int mFileOffsetLocation;

   CRITICAL_SECTION mSyncData;
   
   NewFileHandleReading()
   : currentWriteBuff(-1)
   , currentReadBuff(-1)
   , mReachedEndOfStream(0)
   , mFileOffsetLocation(0)
   {
      for( int ii=0;ii<kNumReadBuffer;++ii )
      {
         mReadBuffer[ii].pBuffer = new char[kSingleReadSize];
         mReadBuffer[ii].bufferSize = 0;
         mReadBuffer[ii].hasData = 0;
      }

      InitializeCriticalSection(&mSyncData);
   }

   ~NewFileHandleReading()
   {
      DeleteCriticalSection(&mSyncData);
   }

   void Reset_NewFile()
   {
      EnterCriticalSection(&mSyncData);

      currentWriteBuff = 0;
      currentReadBuff = 0;

      mReachedEndOfStream = 0;
      mFileOffsetLocation = 0;

      for( int ii=0;ii<kNumReadBuffer;++ii )
      {
         mReadBuffer[ii].hasData = 0;
         mReadBuffer[ii].bufferSize = 0;
      }

      LeaveCriticalSection(&mSyncData);
   }

   int GetBufferedSize()
   {
      EnterCriticalSection(&mSyncData);

      int bytesInBuffer = 0;

      for( int i = 0; i < kNumReadBuffer; ++i )
      {
         bytesInBuffer += mReadBuffer[i].bufferSize;
      }

      LeaveCriticalSection(&mSyncData);

      return bytesInBuffer;
   }

   int Pump_Read(struct MovieInstance* pMovie);
   int GetNewData(void* pDst, int dataAmmount);
};

struct MovieInstance
{
   int fullscreenMovie;

   unsigned int x;
   unsigned int y;
   unsigned int width;
   unsigned int height;

   int memoryStreamSize;
   char* pMemoryStream;

   int isDestroyed;
   IXMedia2XmvPlayer* pMoviePlayer;
   BP_Movie_Render_Callback renderCallback;

   MovieState state;
   bool startImmediatly;
   
   CBaseTexture* frameTex;
   CBaseTexture* yTex;
   CBaseTexture* uTex;
   CBaseTexture* vTex;
   DG_TEX dgTex;
   int currFrame;
   float desiredMovieTime;

   XMEDIA_VIDEO_DESCRIPTOR mVideoDescriptor;

   bool tickFrame;

   NewFileHandleReading mNewFileHandling;

   MovieInstance()
      : x(0)
      , y(0)
      , width(0)
      , height(0)
      , pMoviePlayer(NULL)
      , renderCallback(NULL)

      , state(MOVIE_STATE_INVALID)
      , startImmediatly(false)

      , frameTex(NULL)
      , yTex(NULL)
      , uTex(NULL)
      , vTex(NULL)
      , currFrame(-1)
      , tickFrame(false)
   {
      ZeroMemory(&dgTex, sizeof(dgTex));
      ZeroMemory(&mVideoDescriptor, sizeof(mVideoDescriptor));
   }
};

inline int NewFileHandleReading::Pump_Read(struct MovieInstance* pMovie)
{   
   EnterCriticalSection(&mSyncData);

   int didRead = 0;

   while( !mReachedEndOfStream && mReadBuffer[currentWriteBuff].hasData == 0 )
   {
      int bytesRead;
      pMovie->renderCallback((unsigned char*)mReadBuffer[currentWriteBuff].pBuffer, &bytesRead);
      BPE_VERIFY(bytesRead <= NewFileHandleReading::kSingleReadSize, false, "Movie X360::DoSingleReadPump_Internal: FS_Stream read more data then will fit into read buffer! Tell Dak");

      if( bytesRead > 0 )
      {
         didRead = 1;

         mReadBuffer[currentWriteBuff].offset = mFileOffsetLocation;
         mReadBuffer[currentWriteBuff].bufferSize = bytesRead;
         mReadBuffer[currentWriteBuff].hasData = 1;

         mFileOffsetLocation += bytesRead;

         currentWriteBuff = (currentWriteBuff + 1) % kNumReadBuffer;
         if( bytesRead == kSingleReadSize )
         {
            continue;
         }
         else if(pMovie->pMemoryStream == NULL)
         {
            // This is technically not the end of the stream but any packets not kSingleReadSize are the very last packets in a stream
            // This is neccessary on long streams that continue on after movie has already finished or stream with multiple movie files
            mReachedEndOfStream = 1; 
            break;
         }
      }
      else 
      {
         // If -1 returned that means the stream is done.
         if( bytesRead == -1 )
         {
            mReachedEndOfStream = 1;
         }

         // No data retrieved or EOF, break out of loop, we're done (for now).
         break;
      }
   }

   LeaveCriticalSection(&mSyncData);
   
   return didRead;
}

inline int NewFileHandleReading::GetNewData(void* pDst, int dataAmount)
{
   EnterCriticalSection(&mSyncData);

   //By here we have found the correct buffer to use
   ReadBuffer & currReadBuffer = mReadBuffer[currentReadBuff];

   int retSize = 0;

   if( currReadBuffer.hasData )
   {
      int const maxSizeToGet = bpe::min_val( currReadBuffer.bufferSize, dataAmount );
      XMemCpy(pDst, currReadBuffer.pBuffer, maxSizeToGet);

      retSize = maxSizeToGet;

      // Mark it as free.
      currReadBuffer.bufferSize = 0;
      currReadBuffer.hasData = 0;

      // Advance to next read buffer.
      currentReadBuff = (currentReadBuff + 1) % kNumReadBuffer;
   }

   LeaveCriticalSection(&mSyncData);

   return retSize;
}

std::vector<MovieInstance*> gpMoviePool;
static IXAudio2* gpAudio;
static IXAudio2MasteringVoice* gpMasterVoice;

int totalBytesRequested = 0;

static HRESULT OnXMediaVideoStreamRead(PVOID pvContext, ULONGLONG ullOffset, PVOID pvBuffer, DWORD dwBytesToRead, DWORD *pdwBytesRead)
{
   static int gReentrance = 0;

   ++gReentrance;

   BPE_VERIFY(gReentrance == 1, false, "Invalid");

   MovieInstance* pMovie = (MovieInstance*)pvContext;

   DWORD bytesRead = 0;
   // While ("we have not read as much as we want")
   // and ("we have not read the end of the stream into our read buffer" or "we still have some data in the read buffer")
   // and ("the movie was not destroyed")
   if( pMovie->pMemoryStream == NULL )
   {
      while( (bytesRead != dwBytesToRead) && (pMovie->mNewFileHandling.mReachedEndOfStream == FALSE || pMovie->mNewFileHandling.GetBufferedSize()) && (pMovie->isDestroyed == 0) )
      {
         bytesRead = pMovie->mNewFileHandling.GetNewData(pvBuffer, dwBytesToRead);

         if( bytesRead != dwBytesToRead )
         {
            Sleep(16); // Sleep for 1 frame until it is possible we have new data
         }
      }
   }
   else
   {
      if( pMovie->isDestroyed == 0 )
      {
         bytesRead = bpe::min_val( dwBytesToRead, DWORD(pMovie->memoryStreamSize - ullOffset) );
         XMemCpy(pvBuffer, &pMovie->pMemoryStream[ullOffset], bytesRead);
      }
   }

   totalBytesRequested += dwBytesToRead;

   *pdwBytesRead = bytesRead;

   --gReentrance;

   return S_OK;
}

void BP_Movie_Init()
{
   // make 2 movie players by default and add them to our pool
//    for (int i = 0; i < 2; i++)
//    {
//       gpMoviePool.push_back( new MovieInstance );
//    }

   XAudio2Create(&gpAudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
   gpAudio->CreateMasteringVoice(&gpMasterVoice, 2, 48000, 0, 0, NULL);
}

void BP_Movie_Shutdown()
{
   // Marco/Dak - is there anything to do here??

   // BP JG - Now that the movie has finished, no need to halt the screen saver.
   BP_ConsoleScreenSaverResume();
}

BP_Movie_Handle BP_Movie_Create(int x, int y, int width, int height, DG_TEX_TRANS* pTexTrans, BP_Movie_Render_Callback renderCallback, int isFullscreenMovie, char* pMemoryStream, int memorySize)
{
   // Override passed in flag since it's wrong for MGS3 special/action movies etc?!
   // This simple method correctly detects full screen movies - see google doc "CP4 Movie Stats" for movie sizes.
   isFullscreenMovie = (width >= 512) && (height >= 320);


   // BP JG - prevent the x360 screen saver from activating during full screen movie playback.
   if(isFullscreenMovie)
   {
      BP_ConsoleScreenSaverSuspend();
   }


#if BP_TGS_DEMO()

   if(isFullscreenMovie)
      return 0;
#endif

   MovieInstance * pMovie = NULL;
   // first look for a free used movie
   for(int i = 0; i < gpMoviePool.size(); ++i)
   {      
      if(gpMoviePool[i]->state == MOVIE_STATE_INVALID)
      {
         pMovie = gpMoviePool[i];
         break;
      }
   }
   if( pMovie == NULL )
   {
      pMovie = new MovieInstance;
      gpMoviePool.push_back(pMovie);
   }
   BPE_ASSERT(pMovie != NULL, "Failed to create new movie player\n");
  
   pMovie->fullscreenMovie = isFullscreenMovie;
   pMovie->x = x;
   pMovie->y = y;
   pMovie->width = width;
   pMovie->height = height;
   if( pMemoryStream )
   {
      pMovie->memoryStreamSize = memorySize;
      pMovie->pMemoryStream = new char[memorySize];
      XMemCpy(pMovie->pMemoryStream, pMemoryStream, memorySize);
   }
   else
   {
      pMovie->memoryStreamSize = 0;
      pMovie->pMemoryStream = NULL;
   }
   pMovie->renderCallback = renderCallback;

   pMovie->tickFrame = true;
   pMovie->currFrame = -1;

   pMovie->dgTex.u_offset = 0;
   pMovie->dgTex.v_offset = 0;
   pMovie->dgTex.u_scale = 1;
   pMovie->dgTex.v_scale = 1;
   pMovie->dgTex.tex_id = 0;
   pMovie->dgTex.tri_id = 0;

#if MGS_VERSION == 3
   {
      if(pTexTrans)
      {
         memcpy(&pMovie->dgTex.tex_trans, pTexTrans, sizeof(*pTexTrans));
         int fullTexWidth, fullTexHeight, offsetX, offsetY;
         DG_GetTexelInfo(&fullTexWidth, &fullTexHeight, &offsetX, &offsetY, &pMovie->dgTex);
         pMovie->dgTex.u_scale = (real32)width / fullTexWidth;
         pMovie->dgTex.v_scale = (real32)height / fullTexHeight;
      }
      else
      {
         ZeroMemory(&pMovie->dgTex, sizeof(pMovie->dgTex));
         pMovie->dgTex.u_scale = 1.0f;
         pMovie->dgTex.v_scale = 1.0f;
      }
   }
#endif

#if MGS_VERSION == 2
   pMovie->dgTex.BP_flag = DG_TEXFLAG_UV_CLAMP;
#else
   pMovie->dgTex.BP_flag = 0;
#endif
   pMovie->dgTex.BP_TextureHandle = (unsigned int)&gpRenderBackend->GetBlackMap();

   //printf("MOVIESTATE[%d]: BP_Movie_Create->MOVIE_STATE_BUFFER_INIT\n", gpRenderBackend->GetFrameCount());

   pMovie->state = MOVIE_STATE_WAITING;

   return (BP_Movie_Handle)pMovie;
}

void BP_DestroyMovieTextures_Internal(MovieInstance* pMovie)
{
   if(pMovie->frameTex)
   {
      CBaseTexture::DeleteTexture(pMovie->frameTex);
      pMovie->frameTex = NULL;
   }

   if(pMovie->yTex)
   {
      CBaseTexture::DeleteTexture(pMovie->yTex);
      pMovie->yTex = NULL;
   }

   if(pMovie->uTex)
   {
      CBaseTexture::DeleteTexture(pMovie->uTex);
      pMovie->uTex = NULL;
   }

   if(pMovie->vTex)
   {
      CBaseTexture::DeleteTexture(pMovie->vTex);
      pMovie->vTex = NULL;
   }
}

bool BP_CreateMovie_Internal(MovieInstance* pMovie, IDirect3DDevice9* gpD3DDevice, int const currentMovieState)
{
   BPE_VERIFY(pMovie != NULL, false, "Must pass valid pMovie");

   if( currentMovieState == MOVIE_STATE_BUFFER_INIT )
   {
      pMovie->currFrame = -1;
      pMovie->desiredMovieTime = 0.0f;
      pMovie->isDestroyed = 0;
      pMovie->mNewFileHandling.Reset_NewFile();
      return false;
   }

   if( currentMovieState == MOVIE_STATE_BUFFER )
   {
      if( !pMovie->mNewFileHandling.Pump_Read( pMovie ) )
         return false;
   }

   printf("Movie: Creating new movie instance.\n");

   XMEDIA_XMV_CREATE_PARAMETERS xmvParams;
   memset(&xmvParams, 0, sizeof(xmvParams));
   xmvParams.dwAudioStreamId = XMEDIA_STREAM_ID_DONT_USE;
   xmvParams.dwVideoStreamId = XMEDIA_STREAM_ID_USE_DEFAULT;
   xmvParams.createType = XMEDIA_CREATE_FROM_USER_IO;
   xmvParams.createFromUserIo.pfnVideoStreamReadCallback = OnXMediaVideoStreamRead;
   xmvParams.createFromUserIo.pvVideoStreamContext = pMovie;

   HRESULT hr = XMedia2CreateXmvPlayer(gpD3DDevice, gpAudio, &xmvParams, &pMovie->pMoviePlayer);
   if(FAILED(hr))
   {
      printf("Failed to create movie");
      pMovie->state = MOVIE_STATE_INVALID;
      return false;
   }
   BPE_ASSERT(SUCCEEDED(hr), "Unable to create new movie instance.\n");

   XMEDIA_VIDEO_DESCRIPTOR videoDescriptor;
   pMovie->pMoviePlayer->GetVideoDescriptor(&videoDescriptor);
   XMEDIA_AUDIO_DESCRIPTOR audioDescriptor;
   pMovie->pMoviePlayer->GetAudioDescriptor(&audioDescriptor);

   if( (pMovie->frameTex == NULL) || (videoDescriptor.dwWidth != pMovie->mVideoDescriptor.dwWidth) || (videoDescriptor.dwHeight != pMovie->mVideoDescriptor.dwHeight) )
   {
      BP_DestroyMovieTextures_Internal(pMovie);
      pMovie->frameTex = CBaseTexture::Create(videoDescriptor.dwWidth, videoDescriptor.dwHeight, 1, CBaseTexture::kFormat_A8R8G8B8,
         CBaseTexture::kUsage_RenderTarget, CBaseTexture::kAA_None, kRM_System);

      pMovie->yTex = CBaseTexture::Create(videoDescriptor.dwWidth, videoDescriptor.dwHeight, 1, CBaseTexture::kFormat_Luminance8,
         CBaseTexture::kUsage_TextureLinear, CBaseTexture::kAA_None, kRM_System);
      pMovie->uTex = CBaseTexture::Create(videoDescriptor.dwWidth / 2, videoDescriptor.dwHeight / 2, 1, 
         CBaseTexture::kFormat_Luminance8, CBaseTexture::kUsage_TextureLinear, CBaseTexture::kAA_None, 
         kRM_System);
      pMovie->vTex = CBaseTexture::Create(videoDescriptor.dwWidth / 2, videoDescriptor.dwHeight / 2, 1, 
         CBaseTexture::kFormat_Luminance8, CBaseTexture::kUsage_TextureLinear, CBaseTexture::kAA_None, 
         kRM_System);
   }

   pMovie->mVideoDescriptor = videoDescriptor;

   return true;
}

void BP_DestroyMovie_Internal(MovieInstance* pMovie, bool fromEndOfVideo)
{
   BPE_VERIFY(pMovie != NULL, false, "Must pass valid pMovie");

   pMovie->isDestroyed = TRUE;
   if (pMovie->pMoviePlayer)
   {
      pMovie->pMoviePlayer->Stop(XMEDIA_STOP_IMMEDIATE);
      pMovie->pMoviePlayer->Release();
   }
   pMovie->pMoviePlayer = NULL;
   if( !fromEndOfVideo )
   {
      // release the textures too
   }

   if( pMovie->pMemoryStream )
   {
      delete [] pMovie->pMemoryStream;
      pMovie->pMemoryStream = NULL;
      pMovie->memoryStreamSize = 0;
   }

   pMovie->state = MOVIE_STATE_INVALID;
   printf("Movie: Destroyed movie instance.\n");
}

void BP_Movie_RenderActiveMovies(int const shouldDraw)
{
   IDirect3DDevice9* gpD3DDevice = RenderBackend()->GetDirect3DDevice().GetPtr();
   gpRenderBackend->AcquireRenderThreadOwnership();

   for(int i = 0; i < gpMoviePool.size(); ++i)
   {
      MovieInstance* pMovie = gpMoviePool[i];

      switch(pMovie->state)
      {
      case MOVIE_STATE_INVALID:
      case MOVIE_STATE_WAITING:
         break;

      case MOVIE_STATE_BUFFER_INIT:
         //printf("MOVIESTATE[%d]: MOVIE_STATE_BUFFER_INIT\n", gpRenderBackend->GetFrameCount());
         BP_CreateMovie_Internal(pMovie, gpD3DDevice, MOVIE_STATE_BUFFER_INIT);
         pMovie->state = MOVIE_STATE_BUFFER;
         
         // fall through intentional
      case MOVIE_STATE_BUFFER:
         {
            //printf("MOVIESTATE[%d]: MOVIE_STATE_BUFFER\n", gpRenderBackend->GetFrameCount());

            if( !BP_CreateMovie_Internal(pMovie, gpD3DDevice, MOVIE_STATE_BUFFER) )
               break;

            if( pMovie->startImmediatly )
            {
               pMovie->state = MOVIE_STATE_PLAYING;
               pMovie->startImmediatly = false;
            }
            else
            {
               pMovie->state = MOVIE_STATE_READY;
            }
         }
         // fall through intentional
      case MOVIE_STATE_READY:
      case MOVIE_STATE_PLAYING:
         {
            //printf("MOVIESTATE[%d]: MOVIE_STATE_READY/MOVIE_STATE_PLAYING\n", gpRenderBackend->GetFrameCount());

            BPE_VERIFY(pMovie->pMoviePlayer != NULL, false, "Must have movie player at this point");

            pMovie->mNewFileHandling.Pump_Read( pMovie );


            // NOTE: While theoretically it would be the right thing not to actually decode frames that can't be seen (due to rendering being disabled) 
            // this causes a movie not to properly finish playing if is still has data to flush (i.e. one single frame of data) and the rendering gets disabled due 
            // to the end of the "stream" being detected by the demo stream playback.
            // In this case the stream waits for it to finish, but the movie decoding object never detects it's own "end of stream" state.
            // This was the case in w11a when accessing the first node terminal (MGS2)
            // The cinematic ended exactly one frame too early before the movie causing it to stall waiting for the movie to finish.
            if(/*shouldDraw &&*/ pMovie->tickFrame)
            {
               pMovie->tickFrame = false;

               void* py;
               void* pu;
               void* pv;
               int yPitch;
               int uPitch;
               int vPitch;
               XMEDIA_VIDEO_FRAME videoFrame;

               ZeroMemory(&videoFrame, sizeof(XMEDIA_VIDEO_FRAME));
               videoFrame.videoFormat = XMEDIA_VIDEO_FORMAT_I420;
               videoFrame.dwFlags = XMEDIA_PLAY_DISABLE_AV_SYNC;
               if(pMovie->pMemoryStream) // memory streams loop around forever
               {
                  videoFrame.dwFlags |= XMEDIA_PLAY_LOOP;
               }
               videoFrame.lTimeToPresent = NULL;

               pMovie->yTex->Lock(&py, &yPitch);
               pMovie->uTex->Lock(&pu, &uPitch);
               pMovie->vTex->Lock(&pv, &vPitch);

               DWORD dwLuminanceSize   = pMovie->mVideoDescriptor.dwHeight * yPitch;
               DWORD dwChrominanceSize = pMovie->mVideoDescriptor.dwHeight / 2 * uPitch;

               videoFrame.i420.pvYBuffer = py;
               videoFrame.i420.pvUBuffer = pu;
               videoFrame.i420.pvVBuffer = pv;
               videoFrame.i420.dwYBufferSize = dwLuminanceSize;
               videoFrame.i420.dwUBufferSize = dwChrominanceSize;
               videoFrame.i420.dwVBufferSize = dwChrominanceSize;
               videoFrame.i420.dwYPitch = yPitch;
               videoFrame.i420.dwUPitch = uPitch;
               videoFrame.i420.dwVPitch = vPitch;

               int desiredFrame = int(pMovie->desiredMovieTime * pMovie->mVideoDescriptor.fFrameRate);
               HRESULT hr;
               if( pMovie->currFrame < desiredFrame )
               {
                  int didGetFrame = 0;

                  while( pMovie->currFrame < desiredFrame )
                  {
                     // Allow movie processing to occur when there there is data available for consumption OR
                     // If we reached the end of the stream, this is done so it can finish decoding whatever data is remaining in the internal buffers
                     // And then trigger an EOF.
                     if( pMovie->mNewFileHandling.GetBufferedSize() > 0 || pMovie->mNewFileHandling.mReachedEndOfStream )
                     {
                        hr = pMovie->pMoviePlayer->GetNextFrame(&videoFrame);

                        // EOF?
                        if(hr == XMEDIA_W_EOF)
                        {
                           if( !pMovie->pMemoryStream ) // memory streams loop around forever
                           {
                              printf("Stopping movie instance. Reached end of movie file.\n");
                              BP_DestroyMovie_Internal(pMovie, true);
                              pMovie->state = MOVIE_STATE_STOPPED; // NOTE: this is special: BP_DestroyMovie_Internal sets state to MOVIE_STATE_STOPPED but we overwrite this to set it to MOVIE_STATE_STOPPED
                           }
                           break;
                        }
                        // Decoded frame ok?
                        else if (hr == S_OK)
                        {
                           didGetFrame = 1;
                           ++pMovie->currFrame;

                        }
                        else if( hr == XMEDIA_W_NO_DATA )
                        {
                           break;
                        }
                        else
                        {
                           break;
                        }

                     }
                     else
                     {
                        // No more data available to catch up, break out of while loop.
                        break;
                     }
                  }
                  
                  if (didGetFrame)
                  {
                     BP_YUVConversion(pMovie->yTex, pMovie->uTex, pMovie->vTex, pMovie->frameTex);
                     pMovie->dgTex.BP_TextureHandle = (unsigned int)pMovie->frameTex;
                  } 
               }  
               
               pMovie->yTex->Unlock();
               pMovie->uTex->Unlock();
               pMovie->vTex->Unlock();
            }

            if( gBP_Movie_ShowStats )
            {
               int const bytesInBuffer = pMovie->mNewFileHandling.GetBufferedSize();
               int const totalBufferSize = NewFileHandleReading::kNumReadBuffer * NewFileHandleReading::kSingleReadSize;

               BP_DebugText_Print("MPEG: Buffer contains %d bytes (%d%%)", bytesInBuffer, bytesInBuffer * 100 / totalBufferSize);
            }
         }
         break;

      case MOVIE_STATE_DESTROYED:
         BP_DestroyMovie_Internal(pMovie, false);
         break;
      }
   }

   gpRenderBackend->ReleaseRenderThreadOwnership();
}

DG_TEX* BP_Movie_Get_Texture(BP_Movie_Handle hMovie)
{
   if( hMovie )
   {
      MovieInstance* pMovie = (MovieInstance*)hMovie;
      DG_TEX* pTex = &pMovie->dgTex;

      return pTex;
   }
   else
   {
      printf( "Invalid movie handle");
      return NULL;
   }
}

void BP_Movie_ResetTextureToBlack(BP_Movie_Handle hMovie)
{
   if( hMovie )
   {
      MovieInstance* pMovie = (MovieInstance*)hMovie;
      DG_TEX* pTex = &pMovie->dgTex;
      pTex->BP_TextureHandle = (unsigned int)&gpRenderBackend->GetBlackMap();
   }
   else
   {
      printf( "Invalid movie handle");
   }
}

void BP_Movie_Destroy(BP_Movie_Handle hMovie)
{
   if( hMovie )
   {
      MovieInstance* pMovie = (MovieInstance*)hMovie;

      if (pMovie->state != MOVIE_STATE_INVALID)
      {
         pMovie->state = MOVIE_STATE_DESTROYED;
      }
   }
   else
   {
      printf( "Invalid movie handle");
   }
}

void BP_Movie_Start(BP_Movie_Handle hMovie)
{
   if( hMovie )
   {
      MovieInstance* pMovie = (MovieInstance*)hMovie;
      
      switch(pMovie->state)
      {
      case MOVIE_STATE_READY:
      case MOVIE_STATE_PAUSED:
         pMovie->state = MOVIE_STATE_PLAYING;
         break;

      case MOVIE_STATE_BUFFER_INIT:
      case MOVIE_STATE_BUFFER:
         // If the movie is still in the process of being created allow the movie to start immediately after
         // it finishes initializing
         pMovie->startImmediatly = true;
         break;

      case MOVIE_STATE_WAITING:
      case MOVIE_STATE_STOPPED:
         // If the movie is stopped you have to recreate a new player
         pMovie->state = MOVIE_STATE_BUFFER_INIT;
         pMovie->startImmediatly = true;
         break;

      case MOVIE_STATE_PLAYING:
         BP_DAK_BREAK; // Dak wants to find out how this happened
         break;

      default:
         BPE_VERIFYA(false, "Movie must be ready to start");
         break;

      }
   }
   else
   {
      printf( "Invalid movie handle");
   }
}

void BP_Movie_Stop(BP_Movie_Handle hMovie)
{
   if( hMovie )
   {
      MovieInstance* pMovie = (MovieInstance*)hMovie;
      pMovie->state = MOVIE_STATE_STOPPED;
   }
   else
   {
      printf( "Invalid movie handle");
   }
}

void BP_Movie_Reset(BP_Movie_Handle hMovie)
{
   BP_Movie_Stop(hMovie);
}

void BP_Movie_Set_Time(BP_Movie_Handle hMovie, float time)
{
   if( hMovie )
   {
      MovieInstance* pMovie = (MovieInstance*)hMovie;

      pMovie->desiredMovieTime = bpe::max_val(time, 0.0f); //Ensure caller does not pass in a negative time
      pMovie->tickFrame = true;
   }
   else
   {
      printf( "Invalid movie handle");
   }
}

float BP_Movie_Get_Frame_Time(BP_Movie_Handle hMovie)
{
   //BP_TODO Marco: I'm pretty sure this assumption about frame rate is wrong!
   return 1.0f / 60.0f;
}

int BP_Movie_Get_State(BP_Movie_Handle hMovie)
{
   if( hMovie )
   {
      MovieInstance* pMovie = (MovieInstance*)hMovie;

      MovieState state = pMovie->state;
      if( state == MOVIE_STATE_WAITING )
      {
         return MOVIE_STATE_READY;
      }

      return state;
   }
   else
   {
      BPE_VERIFYA(false, "Invalid movie handle");
      return MOVIE_STATE_INVALID;
   }
}

//----------------------------------------------------------------------------

int BP_Movie_IsPlayingFullscreen()
{
   for(int i = 0; i < gpMoviePool.size(); ++i)
   {
      MovieInstance* pMovie = gpMoviePool[i];
      if( ( pMovie->state != MOVIE_STATE_INVALID ) && ( pMovie->fullscreenMovie ) )
      {
         return 1;
      }
   }

   return 0;
}

//----------------------------------------------------------------------------
