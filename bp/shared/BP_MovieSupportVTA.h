#pragma once
#ifndef BP_MOVIESUPPORTVTA_H
#define BP_MOVIESUPPORTVTA_H

#include <sceavplayer.h>
#include "BP_MovieSupport.h"

class CMoviePlayer
{
public:

   // Number of buffers used to decode video frames. Currently we are double buffering to prevent tearing when drawing
   // the movie texture.
   int static const kNumVideoFrameBuffers = 2;
   
   CMoviePlayer();

   ~CMoviePlayer();

   void SetupCallbacksAndTexture( 
      DG_TEX_TRANS* const pTexTrans, 
      BP_Movie_Render_Callback streamCallback );

   void InitializeHighLevelPlayer( int const x, int const y, int const width, int const height, int const isFullscreenMovie, char const* const pMemoryStream, 
      int const memorySize );

   void ReinitializeAndContinueHighLevelPlayer(char const* const pMemoryStream, int const memorySize );

   void DestroyHighLevelPlayerKeepTextures();
   void DestroyHighLevelPlayer();

   void Start(unsigned int movieSizeInBytes);

   void Stop();

   void UpdateTexture(bool const shouldDraw);

   bool DecodeReturnReady();

   inline MovieState const GetState() const { return mMovieState; }

   inline DG_TEX* GetDGTex() { return &mDGTex; }

   inline void SetDesiredMovieTime(float const newTime) { mRequestedPlaytime = (unsigned int)(newTime * 1000.0f); }

   inline bool IsFullScreen() const { return mIsFullScreen; }

   inline CSyncCriticalSection& GetDecoderMutex() { return mDecodeInProgressMutex; }

   inline unsigned int GetFrameTextureSizeInBytes() const 
   {
      BPE_VERIFY(mMovieWidth > 0 && mMovieHeight > 0, false, "Invalid movie dimensions.");

      // Y (luminance) is stored at 8 bits per pixel at the resolution of the texture. VU (chromaticity) 
      // is 16-bit and downsampled vertically and horizontally with each chromaticity value shared by a 2x2 
      // block of luminance values.
      return (mMovieWidth * mMovieHeight) + ((mMovieWidth >> 1) * (mMovieHeight >> 1) << 1);
   }

private:

   static int const kMoviePacketSize = 65536;
   static int const kMovieStreamBufferSize = kMoviePacketSize * 8;

   CMoviePlayer(CMoviePlayer const& rhs) { }
   CMoviePlayer& operator=(CMoviePlayer const& rhs) { return *this; }

   inline void SetState(MovieState newState) { mMovieState = newState; }

   void FillStreamBuffer();

   void DestroyLowLevelAvPlayer();

   void SetMovieDimsAndCreateOrReuseTextures( int const width, int const height );

   void Restart();

   // The following are several callbacks invoked by the Sce AV player. We override these in order to simulate file 
   // access by returning data from the game stream.
   static void* playercallback_allocate(void* pUserData, uint32_t alignment, uint32_t size);
   static void playercallback_deallocate(void* pUserData, void* pMemory);
   static void* playercallback_allocate_texture(void* pUserData, uint32_t alignment, uint32_t size);
   static void playercallback_deallocate_texture(void* pUserData, void* pMemory);
   static int playercallback_open_file(void* pUserData, const char* argFilename);
   static int playercallback_close_file(void* pUserData);
   static int playercallback_read_offset_file(void* pUserData, uint8_t* pDstBuffer, uint64_t bytePos, uint32_t numBytes);
   static uint64_t playercallback_size_file(void* pUserData);
   static void playercallback_event(void* pUserData, int32_t argEventId, int32_t argSourceId, void* argEventData);

   // Handle the case where the physically continuous memory system needs to override access. Basically the movie player
   // is stopped in this case since it is not active anyway.
   static void handle_phys_cont_memory_unlock(void* pUserData);

   SceAvPlayerHandle mMovieHandle;
   volatile MovieState mMovieState;

   // Some movies are played directly from memory instead of from a stream. This stores the movie data if playing 
   // directly from memory.
   unsigned int mMemoryStreamSize;
   unsigned char* mpMemoryStream;
   
   volatile unsigned int mRequestedPlaytime;
   unsigned int mMovieSizeInBytes;
   uint64_t mDuration; // Length of the movie in milliseconds.

   unsigned char* mpMovieStreamBuffer;
   volatile unsigned int mMovieStreamBufferPos;
   volatile unsigned int mMovieStreamTotalBytesConsumed;
   volatile unsigned int mMovieReadBytePos;

   // Set to 1 when the end of the movie data stream has been reached. There still may be buffered data that still needs
   // playing though.
   volatile int mHasMovieStreamEnded;
   volatile int mHasMovieDecodingEnded;

   // Texture used to store the decoded video frame.
   CBaseTexture* mpVidFrameTex[kNumVideoFrameBuffers];
   unsigned char* mpLastDecodedFrame;
   unsigned int mVidFrameTexBufferIndex;
   unsigned int mMovieX;
   unsigned int mMovieY;
   unsigned int mMovieWidth;
   unsigned int mMovieHeight;
   unsigned int mPlayerMemNumAllocs; // Tracks the number of allocations we've made from the player mem block.
   
   // Called to pull data from the game stream.
   BP_Movie_Render_Callback mMovieStreamCallback;
   
   DG_TEX mDGTex;
   CSyncCriticalSection mDecodeInProgressMutex;

   bool mIsFullScreen;
   
   // Used to notify player when the first valid video frame has actually been decoded and is ready to display.
   bool mHaveValidVideoFrames;
};



#endif
