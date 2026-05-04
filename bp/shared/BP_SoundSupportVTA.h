#pragma once
#ifndef __BP_SOUNDSUPPORTVTA__H
#define __BP_SOUNDSUPPORTVTA__H

#include "Engine/System/CSyncCriticalSection.h"
#include "Engine/System/CStopWatch.h"
#include "BP_SoundSupportVTA_Helpers.h"
#include "BP_ADSREnvelope.h"

namespace NVitaSound
{
   struct SReverbParams
   {
      SceFloat32 mRoom;                 /* room (main) volume control in mB                     -10000 to 0 mB ( 100 mB == 1 dB ) */
      SceFloat32 mRoomHF;               /* high frequency content input in mB, at fHF_reference  -10000 to 0 mB */
      SceFloat32 mDecayTime;            /* decay time of late reverberation in seconds          0.1 to 20 seconds */
      SceFloat32 mDecayHFRatio;         /* ratio of high frequency decay in late reverb         0.1 to 2.0 */
      SceFloat32 mReflections;          /* volume of early reflections in mB                    -10000 to 1000 mB */
      SceFloat32 mReflectionsDelay;     /* delay from input signal until first reflection       0.0 to 0.3 seconds */
      SceFloat32 mReverb;               /* volume of late reverb in mB                          -10000 to 2000 mB */
      SceFloat32 mReverbDelay;          /* delay from first reflection until late reverb        0.0 to 0.1 seconds */
      SceFloat32 mDiffusion;            /* echo density in late reverberation                   0 to 100 % */
      SceFloat32 mDensity;              /* modal density in late reverberation                  0 to 100 % */
      SceFloat32 mHFReference;          /* reference frequency for fRoom_HF and fDecay_HF_ratio   20 to 20000 Hertz */
      SceFloat32 mEarlyReflectionScalar;/* scaler 0 -> 100% for early reflections. 0 == single reflection 100 == widely spread reflections */
      SceFloat32 mLFReference;          /* reference low frequency                              20 to 20000 Hertz */
      SceFloat32 mRoomLF;               /* low frequency content input in mB, at fLF_reference   -10000 to 0 mB  */
      SceFloat32 mDryMB;                /* dry volume in mB                                     -10000 to 0 mB  */
   };

   //----------------------------------------------------------------------------
   struct SSpuVoiceState
   {
      unsigned int mSSA;
      unsigned int mLSAX;
      unsigned int mBlockBegin;
      unsigned int mBlockEnd;
      bool mBlockLoop;
      float mPitch;
      float mVolumeL;
      float mVolumeR;

      // This buffer is used to replace a single vag packet looping sound. A single packet of looping sound has the 
      // first three bits all set resulting in a value of 7. On Vita, a 7 indicates the end of a vag sound which doesn't 
      // produce the correct result. 

      // When calculating the vag endpoints, if a 7 is encountered in the packet header, it is replaced with two packets, 
      // the first packet is the same (marked with a begin loop flag) and the second packet is a duplicate of the first 
      // with the end loop flag set. This causes the packets to properly loop on Vita.
      bool mUseReplacementVagBuffer;
      unsigned char mReplacementVag[32];

      //If not NULL, this replaces SSA as the source of the waveform playback.
      unsigned char const* mpMemoryStream;
      unsigned int mMemoryStreamSize;
      float mMemoryStreamMuteVolume;
      
      SceNgsHVoice mVoiceHandle;
      SceNgsHPatch mPatchHandleDry; // Connects voice directly to master voice for dry signal
      SceNgsHPatch mPatchHandleWet; // Connects voice to the reverb voice for wet signal.

      //ADSR envelope support
      SADSREnvelopeState mADSREnvelope;

      // Cache the state of the voice so we can use a more optimal means of updating the voice state.
      SceNgsPlayerParamsBlock mParamsBlock;

      bool mReverbEnabledL;
      bool mReverbEnabledR;
   };

   class CAudioStream 
   {
   public:

      /// Audio streams can have 1 or more tracks. In all cases except for BGM streams in MGS3, track count is 1.
      struct SAudioTrack
      {
         // Used for AT9 streaming playback. This is the memory directly read by the AT9 voice playback.
         unsigned char mAT9PlaybackBuffers[NVitaSound::kNumAT9PlayBuffers][NVitaSound::kAT9PlayBufferSize];

         // Filled as data streams from the game.
         unsigned char mAT9StreamBuffer[NVitaSound::kAT9StreamBufferSize];

         // Location in the playback buffer of where the next write should occur.
         volatile int mCurrAT9StreamWritePos;

         // Tracks the byte position of where the next audio data should be copied from the game stream buffer to the audio
         // playback buffer. This is read from and written to by both threads.
         volatile int mCurrAT9StreamReadPos;

         // Total bytes we have read/decoded in the audio thread.
         volatile int mTotalBytesReadFromStreamBuffer;

         // Total bytes we have written to the stream buffer from the sound update thread.
         volatile int mTotalBytesWrittenToStreamBuffer;

         // Tracks the current buffer being read by the audio thread. Read and written to from the audio thread.
         volatile int mCurrAT9PlayBuffIndex;

         // Tracks how many bytes have been streamed from disk and are available for playback.
         volatile int mNumBytesAvailable;

         // Keeps track of the number of bytes read from the stream but resets each time the stream loops. This only 
         // applies to BGM streams.
         volatile int mLoopedBytePos;

         // Internal handle used to interact with the NGS system.
         SceNgsHVoice mVoiceHandle;

         // Internal handle used to set parameters like volume and reverb on the voice. Wet is routed through reverb
         // when enabled, dry is always routed directly to the master voice.
         SceNgsHPatch mPatchHandleDry;
         SceNgsHPatch mPatchHandleWet;

         // Tracks the current volume of the track.
         float mVolume;

         // Always points to the stream owning the track.
         CAudioStream* mpOwner;
      };

      CAudioStream() 
         : mAudioRackAT9(0)
         , mpRackMemAT9(NULL)
         , mPlaybackBufferSize(0)
         , mTotalStreamSize(0)
         , mTotalStreamSamples(0)
         , mStreamTop(0)
         , mTrackCount(0)
         , mFrequency(0)
         , mNumChannels(0)
         , mPlaybackState(NVitaSound::kDOSS_Closed)
         , mAudioFormat(NVitaSound::kAUDIO_FORMAT_UNKNOWN)
         , mStreamType(BP_DOSTREAM_TYPE_DIRECT)
         , mConfigData(-1)
         , mStartSampleSkip(0)
         , mEndSampleSkip(0)
         , mIsLooping(false)
         , mPaused(false)
         , mIsReverbEnabled(false)
         , mIsBGMStream(false)
         , mIsAT9StreamDonePlaying(false)
      {
         memset(&mAudioTracks[0], 0, sizeof(SAudioTrack) * BP_DOSTREAM_TRACK_COUNT);
         for (int i = 0; i < BP_DOSTREAM_TRACK_COUNT; i++)
            mAudioTracks[i].mpOwner = this;
      }

      /// Creates the NGS specific data structures and initializes each track of the stream.
      void Initialize();

      /// Prepares for playback of a new audio data stream.
      void SetupStream(DirectOutputStreamInitData const* pInitInfo);

      /// Depending on the type of audio stream and it's current patch configuration, the stream is patched to the 
      /// master voice or to the reverb voice.
      void UpdatePatchRoute();

      // Toggle reverb on the audio stream.
      void SetReverb(bool isEnabled) 
      {
         mIsReverbEnabled = isEnabled;
         UpdatePatchRoute();
      }

      /// Copies data streamed from the game into the appropriate audio buffers.
      void ConsumeAudioData(unsigned char const* pBuffer, int const numBytesInPacket);

      void Pause(bool isPaused);

      void ApplyPause();

      void Stop(bool bImmediate);

      int GetBytePosition();

      bool IsPlaying() const { return mPlaybackState == NVitaSound::kDOSS_Playing; }

      double GetPlayTime();

      void SetTrackVolume(int const track, float const fVol);

      void SetTrackVolume(int const track, float const volumes[2][2], BP_SURROUND_VOICE_TYPE surroundType);

      int GetTrackCount() const { return mTrackCount; }

      void ShowStreamStats();

      NVitaSound::EAudioFormat GetAudioFormat() const { return mAudioFormat; }

      bool IsStreamDonePlaying() const { return mIsAT9StreamDonePlaying; }

      int GetPlaybackBufferSize() const { return mPlaybackBufferSize; }

      CSyncCriticalSection& GetStreamMutex() { return mStreamMutex; }

      NVitaSound::ESoundPlaybackState GetState() const { return mPlaybackState; }

      SAudioTrack const* const GetAudioTrack(int track) const 
      { 
         BPE_ASSERT_NO_MSG(track < mTrackCount);
         return &mAudioTracks[track]; 
      }

   private:

      static void AT9StreamCallback(SceNgsCallbackInfo const* pCallbackInfo);

      void PrepareAT9StreamForPlayback();

      SAudioTrack mAudioTracks[BP_DOSTREAM_TRACK_COUNT];
      CSyncCriticalSection mStreamMutex;

      SceNgsHRack mAudioRackAT9;
      void* mpRackMemAT9;

      int mPlaybackBufferSize;
      int mTotalStreamSize;
      int mTotalStreamSamples;

      // Stream top is a number used by the stream system to map the original stream to it's new name. We use this to 
      // detect whether or not we are playing a BGM stream.
      int mStreamTop;

      int mTrackCount;
      int mFrequency;
      int mNumChannels;
      NVitaSound::ESoundPlaybackState mPlaybackState;
      NVitaSound::EAudioFormat mAudioFormat;

      int mStreamType;

      // Special data flags used by the Atrac9 player.
      int mConfigData;

      // Used to configure the atrac9 player. These values are retrieved using the atrac9 api in conjunction with the 
      // configData and total number of samples.
      SceInt16 mStartSampleSkip;
      SceInt16 mEndSampleSkip;

      // True if the audio stream should be continuously looped until explicitly stopped.
      bool mIsLooping;

      bool mPaused;

      bool mIsReverbEnabled;

      // BGM streams need to fade in/out when started and stopped. Since BGM streams have been converted to aTrac9 
      // streams we need to fade them through the Vita audio system.
      bool mIsBGMStream;

      // Flagged when no more audio data is left in the stream.
      int mIsAT9StreamDonePlaying;
   };
}

#endif
