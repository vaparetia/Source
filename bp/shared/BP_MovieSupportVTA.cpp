#include "Engine/Stdafx.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Engine/Memory/VTAPhysContMemory.h"
#include "Engine/System/VTAThreadPriorities.h"

#include "BP_MovieSupport.h"
#include "BP_SoundSupport.h"
#include "BP_Memory.h"
#include "BP_RenderFX.h"
#include "BP_Renderer.h"
#include "BP_RendererDebug.h"

#include "../../../Builds/DiskBuilds/UseDiskBuild.h"
#include "BP_MovieSupportVTA.h"

#include <gxm.h>
#include <kernel/threadmgr.h>

extern int gBP_Movie_ShowStats;

//----------------------------------------------------------------------------
namespace
{
   int const kMaxMovieInstances = 3; // First movie is always null
   int const kDecoderActive = 1 << 0;

   CMoviePlayer* gMovieInstances[kMaxMovieInstances];
   
   SceUID gMovieDecodeThreadId = -1;
   SceUID gMovieDecodeThreadEventFlagId;
   volatile int gShouldMovieThreadRun;
}

//----------------------------------------------------------------------------
inline CMoviePlayer* internal_get_player(BP_Movie_Handle hMovie)
{
   BPE_VERIFY(hMovie > 0 && hMovie < kMaxMovieInstances, false, "Invalid movie handle.");

   CMoviePlayer* pMoviePlayer = gMovieInstances[hMovie];
   BPE_ASSERT_NO_MSG(pMoviePlayer);

   return pMoviePlayer;
}

//----------------------------------------------------------------------------
static SceInt32 BP_Movie_DecoderThreadMain(SceSize argSize, void *pArgBlock)
{
   // Loop forever decoding movies if they are in the play state.
   while (gShouldMovieThreadRun)
   {
      // Wait for the kDecoderActive flag to be set. This flag is set when a movie is played or a movie's current time
      // is set. This has the effect of "ticking" the decoding process and immediately waiting after a single decode 
      // session. If the movie is playing, its time will continue to be set causing the kDecoderActive flag to be set.
      BPE_CHECK_SCE( sceKernelWaitEventFlag(gMovieDecodeThreadEventFlagId, kDecoderActive, 
         SCE_KERNEL_EVF_WAITMODE_AND | SCE_KERNEL_EVF_WAITMODE_CLEAR_ALL, NULL, NULL) );

      for (int i = 1; i < kMaxMovieInstances; i++)
      {
         CMoviePlayer* pMoviePlayer = gMovieInstances[i];
         bool isReady = false;
         while ( !isReady )
         {
            isReady = pMoviePlayer->DecodeReturnReady();
            if ( !isReady )
            {
               // If we're not ready, wait 4ms and try again
               sceKernelDelayThread( 4000 );
            }
         }
      }
   }

   return 0;
}

//----------------------------------------------------------------------------
void BP_Movie_Init()
{
   memset(&gMovieInstances[0], 0, sizeof(gMovieInstances));

   for (int i = 1; i < kMaxMovieInstances; i++)
   {
      gMovieInstances[i] = new CMoviePlayer();
   }

   gMovieDecodeThreadEventFlagId = sceKernelCreateEventFlag("Movie Decode Event", 
      SCE_KERNEL_EVF_ATTR_TH_PRIO | SCE_KERNEL_EVF_ATTR_SINGLE, 0, NULL);
   BPE_VERIFY(gMovieDecodeThreadEventFlagId > 0, false, "Error creating decoder thread event flag.");

   gMovieDecodeThreadId = sceKernelCreateThread("Movie Decode Thread", BP_Movie_DecoderThreadMain, 
      NVtaThreadPriorities::kPriorityMovieUpdate, 64 * 1024, 0, SCE_KERNEL_CPU_MASK_USER_ALL, NULL);
   BPE_VERIFY(gMovieDecodeThreadId > 0, false, "Unable to create decoder thread.");

   gShouldMovieThreadRun = 1;
   sceKernelStartThread(gMovieDecodeThreadId, 0, NULL);
}

//----------------------------------------------------------------------------
void BP_Movie_Shutdown()
{
   gShouldMovieThreadRun = 0;
   
   int movieThreadExit = 0;
   BPE_CHECK_SCE( sceKernelWaitThreadEnd(gMovieDecodeThreadId, &movieThreadExit, NULL) );

   BPE_CHECK_SCE(  sceKernelDeleteThread(gMovieDecodeThreadId) );

   BPE_CHECK_SCE( sceKernelDeleteEventFlag(gMovieDecodeThreadEventFlagId) );

   for (int i = 0; i < kMaxMovieInstances; i++)
   {
      delete gMovieInstances[i];
   }
}

//----------------------------------------------------------------------------
BP_Movie_Handle BP_Movie_Create(int x, int y, int width, int height, DG_TEX_TRANS* pTexTrans, 
   BP_Movie_Render_Callback renderCallback, int isFullscreenMovie, char* pMemoryStream, int memorySize)
{
   BP_Movie_Handle retHandle = 0;

   for (int i = 1; i < kMaxMovieInstances; i++)
   {
      if (gMovieInstances[i]->GetState() == MOVIE_STATE_INVALID)
      {
         retHandle = i;
         break;
      }
   }

   BPE_VERIFY(retHandle != 0, false, "Unable to find free movie handle.");

   CMoviePlayer* pMoviePlayer = gMovieInstances[retHandle];
   BPE_ASSERT_NO_MSG(pMoviePlayer);

   pMoviePlayer->DestroyHighLevelPlayer();
   pMoviePlayer->SetupCallbacksAndTexture( pTexTrans, renderCallback );
   pMoviePlayer->InitializeHighLevelPlayer(x, y, width, height, isFullscreenMovie,
      pMemoryStream, memorySize);

   return retHandle;
}

//----------------------------------------------------------------------------
void BP_Movie_RenderActiveMovies(int const shouldDraw)
{
   for (int i = 1; i < kMaxMovieInstances; i++)
   {
      CMoviePlayer* pMoviePlayer = gMovieInstances[i];
      pMoviePlayer->UpdateTexture(shouldDraw);
   }
}

//----------------------------------------------------------------------------
void BP_Movie_Destroy(BP_Movie_Handle hMovie)
{
   internal_get_player(hMovie)->DestroyHighLevelPlayer();
}

//----------------------------------------------------------------------------
void BP_Movie_Set_Time(BP_Movie_Handle hMovie, float time)
{
   internal_get_player(hMovie)->SetDesiredMovieTime(time);

   // Signal decode thread to decode any frames necessary to catch up to the current time.
   BPE_CHECK_SCE( sceKernelSetEventFlag(gMovieDecodeThreadEventFlagId, kDecoderActive) );
}

//----------------------------------------------------------------------------
float BP_Movie_Get_Frame_Time(BP_Movie_Handle hMovie)
{
   return (1.0f / 30.0f);
}

//----------------------------------------------------------------------------
void BP_Movie_Start(BP_Movie_Handle hMovie, unsigned int movieDataSizeInBytes)
{
   BPE_VERIFY(movieDataSizeInBytes > 0, false, "Invalid movie data size.");
   internal_get_player(hMovie)->Start(movieDataSizeInBytes);
}

//----------------------------------------------------------------------------
void BP_Movie_Stop(BP_Movie_Handle hMovie)
{
   internal_get_player(hMovie)->Stop();
}

//----------------------------------------------------------------------------
void BP_Movie_Reset(BP_Movie_Handle hMovie)
{
   internal_get_player(hMovie)->Stop();
}

//----------------------------------------------------------------------------
int BP_Movie_Get_State(BP_Movie_Handle hMovie)
{
   return internal_get_player(hMovie)->GetState();
}

//----------------------------------------------------------------------------
DG_TEX* BP_Movie_Get_Texture(BP_Movie_Handle hMovie)
{
   return internal_get_player(hMovie)->GetDGTex();
}

//----------------------------------------------------------------------------
int BP_Movie_IsPlayingFullscreen()
{
   for (int i = 1; i < kMaxMovieInstances; i++)
   {
      if (gMovieInstances[i]->GetState() != MOVIE_STATE_INVALID && gMovieInstances[i]->IsFullScreen())
         return 1;
   }

   return 0;
}

//----------------------------------------------------------------------------
void BP_Movie_ResetTextureToBlack(BP_Movie_Handle hMovie)
{
   internal_get_player(hMovie)->GetDGTex()->BP_TextureHandle =  (unsigned int)&gpRenderBackend->GetBlackMap();
}

//----------------------------------------------------------------------------

CMoviePlayer::CMoviePlayer()
: mMovieHandle(0)
, mMovieState(MOVIE_STATE_INVALID)
, mMemoryStreamSize(0)
, mpMemoryStream(NULL)
, mRequestedPlaytime(0)
, mMovieSizeInBytes(0)
, mDuration(0)
, mpMovieStreamBuffer(NULL)
, mMovieStreamBufferPos(0)
, mMovieStreamTotalBytesConsumed(0)
, mMovieReadBytePos(0)
, mHasMovieStreamEnded(0)
, mHasMovieDecodingEnded(0)
, mpLastDecodedFrame(NULL)
, mVidFrameTexBufferIndex(1)
, mMovieWidth(0)
, mMovieHeight(0)
, mPlayerMemNumAllocs(0)
, mMovieStreamCallback(NULL)
, mIsFullScreen(false) 
, mHaveValidVideoFrames(false)
{
   memset(&mDGTex, 0, sizeof(DG_TEX));
   memset(&mpVidFrameTex[0], 0, sizeof(mpVidFrameTex));
}

//----------------------------------------------------------------------------

CMoviePlayer::~CMoviePlayer()
{
   DestroyHighLevelPlayer();
}

//----------------------------------------------------------------------------

void CMoviePlayer::SetupCallbacksAndTexture( DG_TEX_TRANS* const pTexTrans, BP_Movie_Render_Callback streamCallback )
{
   mMovieStreamCallback = streamCallback;

   memset(&mDGTex, 0, sizeof(mDGTex));
   mDGTex.u_offset = 0;
   mDGTex.v_offset = 0;
   mDGTex.u_scale = 1;
   mDGTex.v_scale = 1;
   mDGTex.tex_id = 0;
   mDGTex.tri_id = 0;

#if MGS_VERSION == 3
   {
      if (pTexTrans)
      {
         memcpy(&mDGTex.tex_trans, pTexTrans, sizeof(*pTexTrans));

         int fullTexWidth, fullTexHeight, offsetX, offsetY;
         DG_GetTexelInfo(&fullTexWidth, &fullTexHeight, &offsetX, &offsetY, &mDGTex);

         mDGTex.u_scale = (real32)width / fullTexWidth;
         mDGTex.v_scale = (real32)height / fullTexHeight;
      }
      else
      {
         mDGTex.u_scale = 1.0f;
         mDGTex.v_scale = 1.0f;
      }
   }
#endif

#if MGS_VERSION == 2
   mDGTex.BP_flag = DG_TEXFLAG_UV_CLAMP;
#else
   // MGS3 - Only set up movies to clamp if is definitely a full screen movie
   // otherwise keep compatibility with HD edition

   if ( width == 960 && height == 544 )
   {
      mDGTex.BP_flag = DG_TEXFLAG_UV_CLAMP;
   }
#endif

}


//----------------------------------------------------------------------------

void CMoviePlayer::InitializeHighLevelPlayer(int const x, int const y, int const width, int const height, 
                                             int const isFullscreenMovie, 
                                             char const* const pMemoryStream, int const memorySize)
{
   mIsFullScreen = isFullscreenMovie;
   mMovieX = x;
   mMovieY = y;
   mMovieWidth = width;
   mMovieHeight = height;

   // Only reset the texture handle if the movie has not been destroyed. In the hold of MGS2, the projector movies are
   // recycled so the last frame of the previous movie continuously displays until the first frame of the next movie is 
   // ready.
   mDGTex.BP_TextureHandle = (unsigned int)&gpRenderBackend->GetBlackMap();
   mVidFrameTexBufferIndex = 1;

   ReinitializeAndContinueHighLevelPlayer( pMemoryStream, memorySize );
}

//----------------------------------------------------------------------------
void CMoviePlayer::ReinitializeAndContinueHighLevelPlayer( 
   char const* const pMemoryStream, int const memorySize)
{
   BPE_VERIFY(GetState() == MOVIE_STATE_INVALID || GetState() == MOVIE_STATE_STOPPED, false, 
      "Trying to initialize a movie without destroying it first.");

   mRequestedPlaytime = 0;
   mMovieSizeInBytes = 0;
   mMovieStreamBufferPos = 0;
   mMovieStreamTotalBytesConsumed = 0;
   mMovieReadBytePos = 0;
   mDuration = 0;
   mHaveValidVideoFrames = false;
   mHasMovieStreamEnded = 0;
   mHasMovieDecodingEnded = 0;

   mPlayerMemNumAllocs = 0;

   BPE_VERIFY(mpMemoryStream == NULL, false, "Memory stream was not freed!");
   if (pMemoryStream)
   {
      mMemoryStreamSize = memorySize;
      mpMemoryStream = new unsigned char[memorySize];
      memcpy(&mpMemoryStream[0], pMemoryStream, memorySize);
   }
   else
   {
      // If aren't playing a movie directly from a memory stream we must be playing from a file stream
      BPE_VERIFY(mpMovieStreamBuffer == NULL, false, "Movie stream buffer wasn't freed.");
      
      mpMovieStreamBuffer = reinterpret_cast<unsigned char*>(BP_Memory_Alloc(kMovieStreamBufferSize, 16, kMT_Permanent, kMC_Movie));
      memset(mpMovieStreamBuffer, 0, kMovieStreamBufferSize);
   }

   DestroyLowLevelAvPlayer();

   // Initialize the av player. We assign callbacks to the av player's file operations in order to access the game 
   // stream instead of a file when playing back the movie.
   SceAvPlayerInitData playerInit;
   memset(&playerInit, 0, sizeof(SceAvPlayerInitData));

   playerInit.memoryReplacement.objectPointer = this;
   playerInit.memoryReplacement.allocate = &CMoviePlayer::playercallback_allocate;
   playerInit.memoryReplacement.deallocate = &CMoviePlayer::playercallback_deallocate;
   playerInit.memoryReplacement.allocateTexture= &CMoviePlayer::playercallback_allocate_texture;
   playerInit.memoryReplacement.deallocateTexture = &CMoviePlayer::playercallback_deallocate_texture;

   playerInit.fileReplacement.objectPointer = this;
   playerInit.fileReplacement.open = &CMoviePlayer::playercallback_open_file;
   playerInit.fileReplacement.close = &CMoviePlayer::playercallback_close_file;
   playerInit.fileReplacement.readOffset = &CMoviePlayer::playercallback_read_offset_file;
   playerInit.fileReplacement.size = &CMoviePlayer::playercallback_size_file;

   playerInit.basePriority = 160; // Value recommended by the Vita sdk.
   playerInit.numOutputVideoFrameBuffers = 2;

   playerInit.eventReplacement.objectPointer = this;
   playerInit.eventReplacement.eventCallback = &CMoviePlayer::playercallback_event;

   playerInit.debugLevel = SCE_AVPLAYER_DBG_NONE;

   mMovieHandle = sceAvPlayerInit(&playerInit);
   BPE_VERIFY(mMovieHandle, false, "Unable to initialize movie player.");

   SetState(MOVIE_STATE_READY);
}

//----------------------------------------------------------------------------
void CMoviePlayer::Restart()
{
   BPE_VERIFY(mpMemoryStream == NULL, false, "Memory streams should not get restarted.");

   // Note, memory streams never get restarted.
   DestroyHighLevelPlayerKeepTextures();
   ReinitializeAndContinueHighLevelPlayer(NULL, 0);
}

//----------------------------------------------------------------------------

void CMoviePlayer::DestroyLowLevelAvPlayer()
{
   if ( mMovieHandle )
   {
      BPE_CHECK_SCE( sceAvPlayerClose( mMovieHandle ) );
      mMovieHandle = NULL;
   }
}

//----------------------------------------------------------------------------

void CMoviePlayer::DestroyHighLevelPlayerKeepTextures()
{
   // Make sure the main thread is not trying to render while the movie is being destroyed.
   CSyncCriticalSectionLocker decodeLock(mDecodeInProgressMutex);

   Stop();
   SetState(MOVIE_STATE_INVALID);

   if (mpLastDecodedFrame)
   {
      delete [] mpLastDecodedFrame;
      mpLastDecodedFrame = NULL;
   }
}

//----------------------------------------------------------------------------
void CMoviePlayer::DestroyHighLevelPlayer()
{
   // Make sure the main thread is not trying to render while the movie is being destroyed.
   CSyncCriticalSectionLocker decodeLock(mDecodeInProgressMutex);

   DestroyHighLevelPlayerKeepTextures();

   for (int i = 0; i < CMoviePlayer::kNumVideoFrameBuffers; i++)
   {
      if (mpVidFrameTex[i])
      {
         CBaseTexture::DeleteTexture(mpVidFrameTex[i]);
         mpVidFrameTex[i] = NULL;
      }
   }

   // If we are deleting the textures, we need to set our DG tex to black
   mDGTex.BP_TextureHandle = (unsigned int)&gpRenderBackend->GetBlackMap();
}

//----------------------------------------------------------------------------
void CMoviePlayer::UpdateTexture(bool const shouldDraw)
{
   if (GetState() == MOVIE_STATE_PLAYING)
   {
      CSyncCriticalSectionLocker decodeLock(mDecodeInProgressMutex);

      // Double checked locking here is necessary since we checked the state before actually locking the mutex to 
      // optimize locking frequency.
      if (GetState() == MOVIE_STATE_PLAYING)
      {
         if (shouldDraw && mpLastDecodedFrame && mHaveValidVideoFrames)
         {
            void* pTexMem = NULL;
            int pitch;

            mpVidFrameTex[mVidFrameTexBufferIndex]->Lock(&pTexMem, &pitch);
            memcpy(pTexMem, mpLastDecodedFrame, GetFrameTextureSizeInBytes());
            mpVidFrameTex[mVidFrameTexBufferIndex]->Unlock();

            // Set the video frame to be the texture once we've gotten the first valid data.
            unsigned int vidFrameTexId = reinterpret_cast<unsigned int>(
               mpVidFrameTex[ (mVidFrameTexBufferIndex + 1) % CMoviePlayer::kNumVideoFrameBuffers ]);

            mDGTex.BP_TextureHandle = vidFrameTexId;
            mVidFrameTexBufferIndex = (mVidFrameTexBufferIndex + 1) % CMoviePlayer::kNumVideoFrameBuffers;
         }

         if (mHasMovieDecodingEnded)
            Stop();
      }
   }
}

//----------------------------------------------------------------------------
bool CMoviePlayer::DecodeReturnReady()
{
   CSyncCriticalSectionLocker decodeLock(mDecodeInProgressMutex);

   if (GetState() != MOVIE_STATE_PLAYING)
      return true;

   if (!sceAvPlayerIsActive(mMovieHandle))
      return true;

   uint64_t targetTime = (uint64_t)(mRequestedPlaytime);
   uint64_t currTime = sceAvPlayerCurrentTime(mMovieHandle);
   SceAvPlayerFrameInfo videoFrame;
   bool gotValidFrame = false;

   while (currTime < targetTime && currTime < mDuration)
   {
      if (sceAvPlayerGetVideoData(mMovieHandle, &videoFrame))
      {
         gotValidFrame = true;
      }
      else
      {
         return false; // Sleep us
      }

      currTime = sceAvPlayerCurrentTime(mMovieHandle);
   }

   if (currTime >= mDuration)
      mHasMovieDecodingEnded = 1;

   // Check pData too. While in the debugger at times, gotValidFrame would be true but pData would be NULL
   if (gotValidFrame && videoFrame.pData)
   {
      memcpy(mpLastDecodedFrame, videoFrame.pData, GetFrameTextureSizeInBytes());
      mHaveValidVideoFrames = true;
   }

   return true;
}

//----------------------------------------------------------------------------
void CMoviePlayer::Start(unsigned int movieSizeInBytes)
{
   if (GetState() != MOVIE_STATE_READY)
      Restart();

   mMovieSizeInBytes = movieSizeInBytes;

   // We need to invoke the stream callback until we've filled up half our streaming buffer. This essentially primes 
   // the stream buffer and also puts the data necessary for the movie player to initialize itself into the buffer.
   int const halfBufferSize = kMovieStreamBufferSize / 2;
   int bytesRead = 0;

   while (mMovieStreamBufferPos < halfBufferSize && mMovieStreamBufferPos < movieSizeInBytes)
   {
      mMovieStreamCallback(&mpMovieStreamBuffer[mMovieStreamBufferPos], &bytesRead);
      mMovieStreamBufferPos += bytesRead;
      mMovieStreamTotalBytesConsumed += bytesRead;
   }

   // Pass a fake file name to the add source function. This will invoke our custom file handling which is actually
   // going to read from the streaming buffer instead of a file.
   BPE_CHECK_SCE( sceAvPlayerAddSource(mMovieHandle, "app0:dummy.mp4") );

   SetState(MOVIE_STATE_BUFFER_INIT);
}

//----------------------------------------------------------------------------
void CMoviePlayer::Stop()
{
   CSyncCriticalSectionLocker decodeLock(mDecodeInProgressMutex);

   // When a movie is played from rdr_movie.c and the movie is destroyed, the rdr_movie Die() function will destroy the 
   // movie but then the ipustream.c Die() function resets the movie which calls Stop() on an already destroyed movie.
   // Don't set the movie state to Stop as this will cause the system to think the movie is in use when it is not.
   // Bug: MGSTWO-3375
   if (GetState() == MOVIE_STATE_INVALID)
   {
      return;
   }

   // We need to destroy the resources used by the movie player when it is stopped in case textures are loaded after
   // the movie is stopped by before it is destroyed. This happens during some of the codec sequences.
   if (mMovieHandle)
   {
      // Block until the movie callback has initialized the movie. This prevents the movie from being stopped while 
      // it is in a partially initialized state. Note, the movie will never get into this state if it hasn't been told 
      // to play yet.
      while (GetState() == MOVIE_STATE_BUFFER_INIT)
      {
         sceKernelDelayThread(250);
      }

      if (GetState() == MOVIE_STATE_PLAYING)
      {
         BPE_CHECK_SCE( sceAvPlayerStop(mMovieHandle) );
      }

      DestroyLowLevelAvPlayer();
   }

   if (mpMemoryStream)
   {
      delete [] mpMemoryStream;
      mpMemoryStream = NULL;
   }

   if (mpMovieStreamBuffer)
   {
      BP_Memory_Free(mpMovieStreamBuffer);
      mpMovieStreamBuffer = NULL;
   }

   SetState(MOVIE_STATE_STOPPED);
}

//----------------------------------------------------------------------------
void CMoviePlayer::FillStreamBuffer()
{
   if (mpMemoryStream || mHasMovieStreamEnded)
      return;

   int const kReadAheadSize = kMovieStreamBufferSize / 2;
   int bytesAvail = mMovieStreamTotalBytesConsumed - mMovieReadBytePos;

   while (bytesAvail < kReadAheadSize)
   {
      int bytesCopiedIntoStreamBuffer = 0;

      mMovieStreamCallback(&mpMovieStreamBuffer[mMovieStreamBufferPos], &bytesCopiedIntoStreamBuffer);

      // Sleep for a frame if there are no bytes available. This will occur if playing using the standalone movie player
      // while it waits on an async. read operation.
      if (!bytesAvail)
      {
         sceKernelDelayThread(33333);
         continue;
      }
      
      // Check for -1 bytes copied. This indicates the end of a game stream. For streams that contain movies that end
      // at the end of a stream this works fine. HOWEVER, we also need to check if we have consumed all the available 
      // movie bytes. This check is necessary for when movie data is in the middle of a game stream.
      if (bytesCopiedIntoStreamBuffer == -1 || mMovieStreamTotalBytesConsumed >= mMovieSizeInBytes)
      {
         mHasMovieStreamEnded = 1;
         break;
      }
      else
      {
         mMovieStreamBufferPos = (mMovieStreamBufferPos + bytesCopiedIntoStreamBuffer) % kMovieStreamBufferSize;
         mMovieStreamTotalBytesConsumed += bytesCopiedIntoStreamBuffer;
      }

      bytesAvail = mMovieStreamTotalBytesConsumed - mMovieReadBytePos;
   }
}

//----------------------------------------------------------------------------
void* CMoviePlayer::playercallback_allocate(void* pUserData, uint32_t alignment, uint32_t size)
{
   return memalign(alignment, size);
}

//----------------------------------------------------------------------------
void CMoviePlayer::playercallback_deallocate(void* pUserData, void* pMemory)
{
   BPE_ASSERT_NO_MSG(pMemory);
   free(pMemory);
}

//----------------------------------------------------------------------------
void* CMoviePlayer::playercallback_allocate_texture(void* pUserData, uint32_t alignment, uint32_t size)
{
   CMoviePlayer* pMoviePlayer = reinterpret_cast<CMoviePlayer*>(pUserData);

   if (!pMoviePlayer->mPlayerMemNumAllocs)
   {
      NPhysContMem::LockPhysContMem(NPhysContMem::OWNER_TYPE_MOVIE_PLAYBACK, 
         &CMoviePlayer::handle_phys_cont_memory_unlock, pMoviePlayer);
   }

   uint32_t alignedSize = (size + (NPhysContMem::kPhysContMemAlignment - 1)) & ~(NPhysContMem::kPhysContMemAlignment - 1);
   SceUID memBlock = sceKernelAllocMemBlock("video_player", SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_NC_RW, 
      alignedSize, NULL);

   void* pMem = NULL;
   BPE_CHECK_SCE( sceKernelGetMemBlockBase(memBlock, &pMem) );

   BPE_CHECK_SCE( sceGxmMapMemory(pMem, alignedSize, SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE) );

   pMoviePlayer->mPlayerMemNumAllocs++;
   return pMem;
}

//----------------------------------------------------------------------------
void CMoviePlayer::playercallback_deallocate_texture(void* pUserData, void* pMemory)
{
   CMoviePlayer* pMoviePlayer = reinterpret_cast<CMoviePlayer*>(pUserData);
   SceUID memBlock = sceKernelFindMemBlockByAddr(pMemory, 0);

   BPE_CHECK_SCE( sceGxmUnmapMemory(pMemory) );

   BPE_CHECK_SCE( sceKernelFreeMemBlock(memBlock) );

   pMoviePlayer->mPlayerMemNumAllocs--;
   if (pMoviePlayer->mPlayerMemNumAllocs == 0)
      NPhysContMem::UnlockPhysContMem();
}

//----------------------------------------------------------------------------
int CMoviePlayer::playercallback_open_file(void* pUserData, const char* argFilename)
{
   // Since we are playing a movie either from the streaming buffer or a fixed size memory stream, we don't actually 
   // open a file.
   return 1;
}

//----------------------------------------------------------------------------
int CMoviePlayer::playercallback_close_file(void* pUserData)
{
   // Since we are playing a movie either from the streaming buffer or a fixed size memory stream, we don't actually 
   // open a file.
   return SCE_OK;
}

//----------------------------------------------------------------------------
int CMoviePlayer::playercallback_read_offset_file(void* pUserData, uint8_t* pDstBuffer, uint64_t bytePos, uint32_t numBytes)
{
   // We don't read from a file but copy data from our streaming buffer to the specified buffer.
   CMoviePlayer* pMoviePlayer = reinterpret_cast<CMoviePlayer*>(pUserData);
   
   if (!pMoviePlayer->mpMemoryStream)
   {
      // bytePos is an absolute position in the movie data. (0 - mMovieSizeInBytes)
      pMoviePlayer->FillStreamBuffer();
      BPE_VERIFY((bytePos + numBytes) <= pMoviePlayer->mMovieStreamTotalBytesConsumed, false, 
         "Movie playback is starved.");

      int wrappedBytePos = bytePos % kMovieStreamBufferSize;
      
      if ((wrappedBytePos + numBytes) > kMovieStreamBufferSize)
      {
         int firstByteCopySize = kMovieStreamBufferSize - wrappedBytePos;
         memcpy(pDstBuffer, &pMoviePlayer->mpMovieStreamBuffer[wrappedBytePos], firstByteCopySize);
         memcpy(pDstBuffer + firstByteCopySize, &pMoviePlayer->mpMovieStreamBuffer[0], numBytes - firstByteCopySize);
      }
      else
      {
         memcpy(pDstBuffer, &pMoviePlayer->mpMovieStreamBuffer[wrappedBytePos], numBytes);
      }
   }
   else
   {
      BPE_VERIFY((bytePos + numBytes) <= pMoviePlayer->mMemoryStreamSize, false, "Trying to read past the end of the memory stream.");
      memcpy(pDstBuffer, &pMoviePlayer->mpMemoryStream[bytePos], numBytes);
   }

   // Check to see how far we are reading into the movie stream. This is needed by the update logic to know whether or 
   // not to buffer more data.
   pMoviePlayer->mMovieReadBytePos = (pMoviePlayer->mMovieReadBytePos > bytePos) ? pMoviePlayer->mMovieReadBytePos : bytePos;

   return numBytes;
}

//----------------------------------------------------------------------------
uint64_t CMoviePlayer::playercallback_size_file(void* pUserData)
{
   CMoviePlayer* pMoviePlayer = reinterpret_cast<CMoviePlayer*>(pUserData);

   BPE_VERIFY(pMoviePlayer->mMovieSizeInBytes > 0, false, "Movie size was invalid.");
   return pMoviePlayer->mMovieSizeInBytes;
}

//----------------------------------------------------------------------------

void CMoviePlayer::SetMovieDimsAndCreateOrReuseTextures( int const width, int const height )
{
   bool sizesMatch = ( mMovieWidth == width && mMovieHeight == height );

   for ( int texIndex = 0; texIndex < kNumVideoFrameBuffers; ++texIndex )
   {
      // If the sizes don't match and we have a texture, kill it
      if ( !sizesMatch && mpVidFrameTex[texIndex] )
      {
         CBaseTexture::DeleteTexture( mpVidFrameTex[ texIndex ] );
         mpVidFrameTex[texIndex] = NULL;
      }

      if ( !mpVidFrameTex[ texIndex ] )
      {
         mpVidFrameTex[texIndex] = CBaseTexture::Create(width, height, 1, 
            CBaseTexture::kFormat_YVU420P2_CSC1, CBaseTexture::kUsage_TextureLinear, CBaseTexture::kAA_None, 
            kRM_System);
      }
   }

   mMovieWidth = width;
   mMovieHeight = height;
}

//----------------------------------------------------------------------------
void CMoviePlayer::playercallback_event(void* pUserData, int32_t argEventId, int32_t argSourceId, void* argEventData)
{
   CMoviePlayer* pMoviePlayer = reinterpret_cast<CMoviePlayer*>(pUserData);

   switch (argEventId) 
   {
   case SCE_AVPLAYER_STATE_READY:
      {
         BPE_VERIFY(pMoviePlayer->GetState() == MOVIE_STATE_BUFFER_INIT, false, "Movie is not in the correct state.");

         int const streamCount = sceAvPlayerStreamCount(pMoviePlayer->mMovieHandle);
         BPE_VERIFY(streamCount == 1, false, "Should only be one data stream.");

         SceAvPlayerStreamInfo streamInfo;
         BPE_CHECK_SCE( sceAvPlayerGetStreamInfo(pMoviePlayer->mMovieHandle, 0, &streamInfo) );
         
         BPE_VERIFY(streamInfo.type == SCE_AVPLAYER_VIDEO, false, "Expected a video stream.");

         BPE_CHECK_SCE( sceAvPlayerEnableStream(pMoviePlayer->mMovieHandle, 0) );

         pMoviePlayer->SetMovieDimsAndCreateOrReuseTextures( streamInfo.details.video.width, streamInfo.details.video.height );
         pMoviePlayer->mDuration = streamInfo.duration;

         pMoviePlayer->mpLastDecodedFrame = new unsigned char[pMoviePlayer->GetFrameTextureSizeInBytes()];

         if (pMoviePlayer->mpMemoryStream) // Memory movie streams loop forever.
         {
            BPE_CHECK_SCE( sceAvPlayerSetLooping(pMoviePlayer->mMovieHandle, true) );
         }
         else
         {
            BPE_CHECK_SCE( sceAvPlayerSetLooping(pMoviePlayer->mMovieHandle, false) );
         }

         BPE_CHECK_SCE( sceAvPlayerStart(pMoviePlayer->mMovieHandle) );

         pMoviePlayer->SetState(MOVIE_STATE_PLAYING);

         // Signal the decoder thread to start decoding.
         BPE_CHECK_SCE( sceKernelSetEventFlag(gMovieDecodeThreadEventFlagId, kDecoderActive) );
      }
      break;

   case SCE_AVPLAYER_STATE_PLAY:
   case SCE_AVPLAYER_STATE_STOP:
   case SCE_AVPLAYER_STATE_PAUSE:
   case SCE_AVPLAYER_STATE_BUFFERING:
      break;
   };
}

//----------------------------------------------------------------------------
void CMoviePlayer::handle_phys_cont_memory_unlock(void* pUserData)
{
   CMoviePlayer* pMoviePlayer = reinterpret_cast<CMoviePlayer*>(pUserData);
   pMoviePlayer->Stop();
}
