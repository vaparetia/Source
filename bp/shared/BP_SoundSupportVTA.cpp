//----------------------------------------------------------------------------
// BP_SoundSupportVTA.cpp
//----------------------------------------------------------------------------

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/StlExtras/BPEStlExtras.h"
#include "Engine/System/CStopWatch.h"
#include "Engine/Graphics/CColor.h"

#include "Engine/System/VTAThreadPriorities.h"
#include "Engine/System/CSyncCriticalSection.h"

#include "BP_EndianSupport.h"
#include "BP_BuildDefines.h"
#include "BP_SoundSupport.h"
#include "BP_RendererDebug.h"
#include "BP_Debug.h"

#include "libsd.h"
#include "MGS_Common.h"

#include <kernel.h>
#include <kernel/threadmgr.h>
#include <libsysmodule.h>
#include <stdlib.h>

#include "BP_SoundSupportVTA.h"
#include "BP_Memory.h"

extern "C" void mtast_open_mta_header_vita(void* pMTAHeader);

using namespace NVitaSound;

//----------------------------------------------------------------------------
extern unsigned char gBP_SpuMemory[];
extern int gBP_MTAChannel;
extern int gReverbModePS2;
extern int gReverbDepthPS2;
extern int gDirectOutputStreamsPaused;

extern NVitaSound::SNgsState gNgsState;

int gBP_WavFormPitch = 32000;
float gBP_WavFormVolume = 0.15f;
int gBP_BGMDebugVoiceNum = -1;
int gBP_OverriddenVitaReverbMode = 0;
int gBP_UseCustomReverbSettings = 0;

int gBP_DumpDecayValues = 0;

// Used to control the overall strength of the reverb effect. Exposed in the debug menu for tweaking by JP.
float gBP_MasterReverbStrength = 1.5f;

//----------------------------------------------------------------------------
void CAudioStream::Initialize()
{
   struct SceNgsVoiceDefinition const* pVoiceDefAt9 = sceNgsVoiceDefGetSimpleAtrac9Voice();

   SceNgsRackDescription rackDesc;
   SceNgsBufferInfo bufferInfo;

   rackDesc.nChannelsPerVoice = 2; // Will be either mono or stereo
   rackDesc.nVoices = BP_DOSTREAM_TRACK_COUNT; 
   rackDesc.pVoiceDefn = pVoiceDefAt9;
   rackDesc.nMaxPatchesPerInput = 0;
   rackDesc.nPatchesPerOutput = 2;

   int returnCode = sceNgsRackGetRequiredMemorySize(gNgsState.mNgsSysHandle, &rackDesc, &bufferInfo.size);
   BPE_VERIFY(returnCode == SCE_NGS_OK, false, "Unable to determine memory size for AT9 rack.");

   mpRackMemAT9 = BP_Memory_Alloc(bufferInfo.size, SCE_NGS_MEMORY_ALIGN_SIZE, kMT_Permanent, kMC_Sound);
   BPE_VERIFY(mpRackMemAT9 != NULL, false, "Error allocating memory for AT9 rack.");
   memset(mpRackMemAT9, 0, bufferInfo.size);

   bufferInfo.data = mpRackMemAT9;
   returnCode = sceNgsRackInit(gNgsState.mNgsSysHandle, &bufferInfo, &rackDesc, &mAudioRackAT9);
   BPE_VERIFY(returnCode == SCE_NGS_OK, false, "Error initializing PCM/VAG rack.");

   for (int currTrack = 0; currTrack < BP_DOSTREAM_TRACK_COUNT; currTrack++)
   {
      SceNgsPatchSetupInfo patchSetupInfo;
      SceNgsPatchRouteInfo patchRouteInfo;
      SAudioTrack& track = mAudioTracks[currTrack];

      returnCode = sceNgsRackGetVoiceHandle(mAudioRackAT9, currTrack, &track.mVoiceHandle);
      BPE_VERIFY(returnCode == SCE_OK, false, "Unable to get voice handle for AT9 output stream.");

      // Register a callback with the atrac9 voice so it can inform the audio stream what read buffer it is currently on.
      returnCode = sceNgsVoiceSetModuleCallback(track.mVoiceHandle, SCE_NGS_VOICE_AT9_PLAYER,
         &CAudioStream::AT9StreamCallback, &track);
      BPE_VERIFY(returnCode == SCE_OK, false, "Unable to set voice callback.");

      // Always route through the master voice for the dry signal.
      patchSetupInfo.hVoiceSource = track.mVoiceHandle;
      patchSetupInfo.nSourceOutputIndex = 0;
      patchSetupInfo.nSourceOutputSubIndex = SCE_NGS_VOICE_PATCH_AUTO_SUBINDEX;
      patchSetupInfo.hVoiceDestination = gNgsState.mMasterVoice;
      patchSetupInfo.nTargetInputIndex = 0;

      returnCode = sceNgsPatchCreateRouting(&patchSetupInfo, &track.mPatchHandleDry);
      BPE_VERIFY(returnCode == SCE_OK, false, "Unable to create patch route for At9 stream voice.");

      returnCode = sceNgsPatchGetInfo(track.mPatchHandleDry, &patchRouteInfo, NULL);
      BPE_VERIFY(returnCode == SCE_OK, false, "Unable to get patch info. for At9 stream voice.");

      patchRouteInfo.vols.m[0][0] = track.mVolume;
      patchRouteInfo.vols.m[0][1] = 0.0f;
      patchRouteInfo.vols.m[1][0] = 0.0f;
      patchRouteInfo.vols.m[1][1] = track.mVolume;

      returnCode = sceNgsVoicePatchSetVolumesMatrix(track.mPatchHandleDry, &patchRouteInfo.vols);
      BPE_VERIFY(returnCode == SCE_OK, false, "Unable to set volume matrix on direct output stream voice.");
   }

   UpdatePatchRoute();
}

//----------------------------------------------------------------------------
void CAudioStream::UpdatePatchRoute()
{
   for (int currTrack = 0; currTrack < BP_DOSTREAM_TRACK_COUNT; currTrack++)
   {
      SAudioTrack& track = mAudioTracks[currTrack];
      BPE_VERIFY(track.mVoiceHandle, false, "Unable to get voice handle for AT9 output stream.");

      // If reverb is enabled on the stream a patch is created through the reverb voice.
      bool bEnableReverb = (mStreamType == BP_DOSTREAM_TYPE_VOICE && mIsReverbEnabled && gBP_EnableAudioReverb);
      int returnCode = SCE_OK;

      if (bEnableReverb)
      {
         if (!track.mPatchHandleWet)
         {
            SceNgsPatchSetupInfo patchSetupInfo;
            SceNgsPatchRouteInfo patchRouteInfo;

            patchSetupInfo.hVoiceSource = track.mVoiceHandle;
            patchSetupInfo.nSourceOutputIndex = 0;
            patchSetupInfo.nSourceOutputSubIndex = SCE_NGS_VOICE_PATCH_AUTO_SUBINDEX;
            patchSetupInfo.hVoiceDestination = gNgsState.mReverbVoice;
            patchSetupInfo.nTargetInputIndex = 0;

            returnCode = sceNgsPatchCreateRouting(&patchSetupInfo, &track.mPatchHandleWet);
            BPE_VERIFY(returnCode == SCE_OK, false, "Unable to create patch route for stream channel.");

            returnCode = sceNgsPatchGetInfo(track.mPatchHandleWet, &patchRouteInfo, NULL);
            BPE_VERIFY(returnCode == SCE_OK, false, "Unable to get patch info. for stream channel voice.");

            patchRouteInfo.vols.m[0][0] = 1.0f; // left to left
            patchRouteInfo.vols.m[0][1] = 0.0f; // left to right
            patchRouteInfo.vols.m[1][0] = 0.0f; // right to left
            patchRouteInfo.vols.m[1][1] = 1.0f; // right to right

            returnCode = sceNgsVoicePatchSetVolumesMatrix(track.mPatchHandleWet, &patchRouteInfo.vols);
            BPE_VERIFY(returnCode == SCE_OK, false, "Unable to set volume matrix on direct output stream voice.");
         }
      }
      else
      {
         if (track.mPatchHandleWet)
         {
            returnCode = sceNgsPatchRemoveRouting(track.mPatchHandleWet);
            BPE_VERIFY(returnCode == SCE_OK, false, "Error removing patch routing.");
            track.mPatchHandleWet = 0;
         }
      }
   }
}

//----------------------------------------------------------------------------
void CAudioStream::PrepareAT9StreamForPlayback()
{
   int returnCode;
   SceNgsAT9SkipBufferInfo skipInfo;

   returnCode = sceNgsAT9GetSectionDetails(0, mTotalStreamSamples, mConfigData, &skipInfo);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to get skip info from AT9 stream.");
   mStartSampleSkip = skipInfo.nStartSkip;
   mEndSampleSkip = skipInfo.nEndSkip;

   for (int i = 0; i < mTrackCount; i++)
   {
      SAudioTrack& currTrack = mAudioTracks[i];
      SceNgsBufferInfo bufferInfo;
      SceNgsAT9Params* pAT9Params;

      float zeroVol[2][2] = { 0.0f, 0.0f, 0.0f, 0.0f };
      SetTrackVolume(i, zeroVol, BP_SURROUND_VOICE_TYPE_STEREO);

      returnCode = sceNgsVoiceLockParams(currTrack.mVoiceHandle, SCE_NGS_VOICE_AT9_PLAYER, SCE_NGS_AT9_PARAMS_STRUCT_ID, 
         &bufferInfo);

      BPE_VERIFY(returnCode == SCE_OK, false, "Unable to lock voice parameters on direct output stream.");

      memset(bufferInfo.data, 0, bufferInfo.size);
      pAT9Params = (SceNgsAT9Params*)bufferInfo.data;
      pAT9Params->desc.id = SCE_NGS_AT9_PARAMS_STRUCT_ID;
      pAT9Params->desc.size = sizeof(SceNgsAT9Params);

      pAT9Params->fPlaybackFrequency = (SceFloat32)mFrequency;
      pAT9Params->fPlaybackScalar = 1.0f;
      pAT9Params->nLeadInSamples = 0;
      pAT9Params->nLimitNumberOfSamplesPlayed = 0;
      pAT9Params->nChannels = mNumChannels;
      pAT9Params->configData = mConfigData;

      switch (pAT9Params->nChannels)
      {
      case 1:
         pAT9Params->nChannelMap[0] = SCE_NGS_AT9_LEFT_CHANNEL;
         pAT9Params->nChannelMap[1] = SCE_NGS_AT9_LEFT_CHANNEL;
         break;

      case 2:
         pAT9Params->nChannelMap[0] = SCE_NGS_AT9_LEFT_CHANNEL;
         pAT9Params->nChannelMap[1] = SCE_NGS_AT9_RIGHT_CHANNEL;
         break;

      default:
         BPE_VERIFY(false, false, "Unsupported channel count!");
      }

      BPE_VERIFY(NVitaSound::kNumAT9PlayBuffers <= 4, false, "Unsupported buffer count.");

      for (int currBuff = 0; currBuff < NVitaSound::kNumAT9PlayBuffers; currBuff++)
      {
         pAT9Params->buffs[currBuff].pBuffer = &currTrack.mAT9PlaybackBuffers[currBuff][0];
         pAT9Params->buffs[currBuff].nNumBytes = 0;
         pAT9Params->buffs[currBuff].nLoopCount = 0;
         pAT9Params->buffs[currBuff].nNextBuff = (currBuff + 1) % NVitaSound::kNumAT9PlayBuffers;
      }

      // Prime the audio buffers by copying currently cached audio data.
      for (int currBuff = 0; currBuff < NVitaSound::kNumAT9PlayBuffers; currBuff++)
      {
         int numBytes = bpe::min_val(NVitaSound::kAT9PlayBufferSize, (int)currTrack.mNumBytesAvailable);

         memcpy(&currTrack.mAT9PlaybackBuffers[currBuff][0], &currTrack.mAT9StreamBuffer[currTrack.mCurrAT9StreamReadPos], numBytes);
         pAT9Params->buffs[currBuff].nNumBytes = numBytes;
         pAT9Params->buffs[currBuff].nSamplesDiscardStart = 0;
         pAT9Params->buffs[currBuff].nSamplesDiscardEnd = 0;

         currTrack.mCurrAT9StreamReadPos += numBytes;
         currTrack.mTotalBytesReadFromStreamBuffer += numBytes;
         currTrack.mNumBytesAvailable -= numBytes;

         if (currTrack.mTotalBytesReadFromStreamBuffer == mTotalStreamSize)
         {
            pAT9Params->buffs[currBuff].nSamplesDiscardStart = 0;
            pAT9Params->buffs[currBuff].nSamplesDiscardEnd = mEndSampleSkip;
            pAT9Params->buffs[currBuff].nNextBuff = -1;
            break;
         }
      }

      returnCode = sceNgsVoiceUnlockParams(currTrack.mVoiceHandle, SCE_NGS_VOICE_AT9_PLAYER);
      if (returnCode != SCE_NGS_OK)
      {
         char msgBuf[128];
         sceNgsVoiceGetParamsOutOfRange(currTrack.mVoiceHandle, SCE_NGS_VOICE_AT9_PLAYER, msgBuf);
         BPE_VERIFY(returnCode == SCE_OK, false, "Unable to unlock voice parameters on direct output stream.");
      }
   }
}

//----------------------------------------------------------------------------
void CAudioStream::SetupStream(DirectOutputStreamInitData const* pInitInfo)
{
   Stop(true); // Must do this as not all streams are stopped before starting again.

   mPlaybackBufferSize = pInitInfo->playbackBufferSize;
   mTotalStreamSize = pInitInfo->totalStreamSize;
   mTotalStreamSamples = pInitInfo->totalStreamSamples;
   mTrackCount = pInitInfo->trackCount;
   mFrequency = pInitInfo->freq;
   mNumChannels = pInitInfo->channelCount & 0xffff;
   mPlaybackState = NVitaSound::kDOSS_Pending;
   mAudioFormat = NVitaSound::kAUDIO_FORMAT_ATRAC9;
   mPaused = false;
   mConfigData = *(int*)&pInitInfo->configData[0];
   mIsAT9StreamDonePlaying = 0;
   mStartSampleSkip = 0;
   mEndSampleSkip = 0;
   mStreamType = pInitInfo->streamType;
   mStreamTop = pInitInfo->streamTop;
   mIsBGMStream = NVitaSound::is_bgm_stream(mStreamTop);

#if MGS_VERSION == 3
   mIsLooping = (pInitInfo->channel == gBP_MTAChannel);
#elif MGS_VERSION == 2
   mIsLooping = false; // No looping streams in MGS2
#endif

#if MGS_VERSION == 2
   BPE_VERIFY(mTrackCount == 1, false, "Should only be single tracks with MGS2");
#endif

   for (int i = 0; i < mTrackCount; i++)
   {
      SAudioTrack& currTrack = mAudioTracks[i];

      currTrack.mCurrAT9PlayBuffIndex = 0;
      currTrack.mCurrAT9StreamWritePos = 0;
      currTrack.mCurrAT9StreamReadPos = 0;
      currTrack.mTotalBytesReadFromStreamBuffer = 0;
      currTrack.mTotalBytesWrittenToStreamBuffer = 0;
      currTrack.mNumBytesAvailable = 0;
      currTrack.mLoopedBytePos = 0;
      currTrack.mVolume = 0.0f;
   }

   UpdatePatchRoute();

   // Initialize the MTA playback structures so we get proper cross fading.
#if MGS_VERSION == 3
   if (mIsBGMStream && !IsPlaying())
   {
      unsigned int const kMTAId = 'FATM';
      void* pMTAHeader = (void*)&pInitInfo->mMTAHeader[0];

      if (!memcmp(pMTAHeader, &kMTAId, sizeof(kMTAId)))
      {
         BPE_VERIFY(mIsBGMStream, false, "Should be a BGM stream.");
         mtast_open_mta_header_vita(pMTAHeader);
      }
   }
#endif
}

//----------------------------------------------------------------------------
void CAudioStream::AT9StreamCallback(SceNgsCallbackInfo const* pCallbackInfo)
{
   CAudioStream::SAudioTrack* pTrack = reinterpret_cast<CAudioStream::SAudioTrack*>(pCallbackInfo->pUserData);
   CAudioStream* pAudioStream = pTrack->mpOwner;

   BPE_VERIFY(pAudioStream, false, "Track does not have a parent stream.");

   CSyncCriticalSectionLocker lock(pAudioStream->mStreamMutex);

   if (pCallbackInfo->nCallbackData == SCE_NGS_AT9_END_OF_DATA)
   {
      BPE_VERIFY(!pAudioStream->mIsLooping, false, "Should not reach the end of audio data when looping is enabled.");
      pAudioStream->mIsAT9StreamDonePlaying = 1;
      return;
   }

   SceNgsBufferInfo buffInfo;
   SceNgsAT9Params* pAT9Params;

   int returnCode = sceNgsVoiceLockParams(pTrack->mVoiceHandle, SCE_NGS_VOICE_AT9_PLAYER, SCE_NGS_AT9_PARAMS_STRUCT_ID, &buffInfo );
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to lock voice parameters.");
   pAT9Params = (SceNgsAT9Params*)buffInfo.data;

   // Curr buffer at this point is a buffer previously played so recycle it.
   int currBuffer = pTrack->mCurrAT9PlayBuffIndex;
   if (pTrack->mNumBytesAvailable > 0)
   {
      int numBytes = bpe::min_val(NVitaSound::kAT9PlayBufferSize, (int)pTrack->mNumBytesAvailable);

      // Copy audio data from our intermediary buffer to the actual playback buffer.
      memcpy(&pTrack->mAT9PlaybackBuffers[currBuffer][0], &pTrack->mAT9StreamBuffer[pTrack->mCurrAT9StreamReadPos], numBytes);
      pAT9Params->buffs[currBuffer].nNumBytes = numBytes;

      pTrack->mCurrAT9StreamReadPos = (pTrack->mCurrAT9StreamReadPos + numBytes) % NVitaSound::kAT9StreamBufferSize;

      pTrack->mTotalBytesReadFromStreamBuffer += numBytes;
      pTrack->mLoopedBytePos += numBytes;
      pTrack->mNumBytesAvailable -= numBytes;

      BPE_VERIFY(pTrack->mTotalBytesReadFromStreamBuffer <= pTrack->mTotalBytesWrittenToStreamBuffer, false, 
         "Read has overlapped write.");
   }
   else
   {
      memset(&pTrack->mAT9PlaybackBuffers[currBuffer][0], 0, NVitaSound::kAT9PlayBufferSize);
   }

   // If we reach the end of the original stream either loop it or terminate it. In the case of looping the total 
   // bytes read will continue to increment, incrementing the loop count each time it wraps.
   if (pTrack->mTotalBytesReadFromStreamBuffer >= pAudioStream->mTotalStreamSize)
   {
      if (!pAudioStream->mIsLooping)
      {
         pAT9Params->buffs[currBuffer].nSamplesDiscardStart = 0;
         pAT9Params->buffs[currBuffer].nSamplesDiscardEnd = pAudioStream->mEndSampleSkip;
         pAT9Params->buffs[currBuffer].nNextBuff = -1;
      }
      else
      {
         if (pTrack->mLoopedBytePos >= pAudioStream->mTotalStreamSize) // Time to loop.
         {
            pAT9Params->buffs[currBuffer].nSamplesDiscardStart = pAudioStream->mStartSampleSkip;
            pAT9Params->buffs[currBuffer].nSamplesDiscardEnd = 0;
            pTrack->mLoopedBytePos = 0;
         }
         else // In the middle of a loop
         {
            pAT9Params->buffs[currBuffer].nSamplesDiscardStart = 0;
            pAT9Params->buffs[currBuffer].nSamplesDiscardEnd = 0;
         }
      }
   }
   else
   {
      pAT9Params->buffs[currBuffer].nSamplesDiscardStart = 0;
      pAT9Params->buffs[currBuffer].nSamplesDiscardEnd = 0;
   }

   pTrack->mCurrAT9PlayBuffIndex = (pTrack->mCurrAT9PlayBuffIndex + 1) % NVitaSound::kNumAT9PlayBuffers;

   returnCode = sceNgsVoiceUnlockParams(pCallbackInfo->hVoiceHandle, SCE_NGS_VOICE_AT9_PLAYER);
   if (returnCode != SCE_OK)
   {
      char paramErrorMsg[512];
      sceNgsVoiceGetParamsOutOfRange(pCallbackInfo->hVoiceHandle, SCE_NGS_VOICE_AT9_PLAYER, paramErrorMsg);
      BPE_VERIFY(false, false, paramErrorMsg);
   }
}

//----------------------------------------------------------------------------
void CAudioStream::ConsumeAudioData(unsigned char const* pBuffer, int const numBytesInPacket)
{
   BPE_VERIFY(mAudioFormat == NVitaSound::kAUDIO_FORMAT_ATRAC9, false, "Unsupported audio format.");

   // numBytes is set from the "option" field of the packet. In the case of encoding atrac9 audio packets into game 
   // streams, the option field represents the size of the actual data not including any padding.
   CSyncCriticalSectionLocker lock(mStreamMutex);

   // Silly way to mark a sector padding packet.. but.. 
   if (*(unsigned int*)pBuffer == 0xdeadbeef)
   {
      return;
   }

   int numBytesPerTrack = numBytesInPacket / mTrackCount;
   for (int i = 0; i < mTrackCount; i++)
   {
      SAudioTrack& currTrack = mAudioTracks[i];

      // Audio packets are powers of two as are the buffer sizes so as long as these size ratios hold true, we don't 
      // need to worry about partial copies.
      int numBytes = bpe::min_val(NVitaSound::kAT9StreamBufferSize - currTrack.mCurrAT9StreamWritePos, 
         numBytesPerTrack);

      memcpy(&currTrack.mAT9StreamBuffer[currTrack.mCurrAT9StreamWritePos], pBuffer, numBytes);
         
      currTrack.mCurrAT9StreamWritePos = (currTrack.mCurrAT9StreamWritePos + numBytes) % NVitaSound::kAT9StreamBufferSize;
      currTrack.mNumBytesAvailable += numBytes;
      currTrack.mTotalBytesWrittenToStreamBuffer += numBytes;

      // Make sure we haven't overflowed any buffers.
      BPE_VERIFY(currTrack.mNumBytesAvailable < NVitaSound::kAT9StreamBufferSize, false, "Buffer overflow!");

      // Go to start of the next track's worth of audio data.
      pBuffer += numBytesPerTrack;
   }

   // For ATRAC9 streams, we start playback of the stream here once we have valid data to start playing. Wait until we 
   // can fill up the AT9 buffers we can recycle them in the audio callback. Since tracks are kept in sync, we can use
   // the first track to determine when to start playing all tracks.
   SceNgsVoiceInfo voiceInfo;
   int res = SCE_OK;

   if (mPlaybackState == NVitaSound::kDOSS_Pending)
   {
      // Queue up a full set of playback buffers before starting the 
      int const kNumBytesToQueue = bpe::min_val(
         NVitaSound::kAT9PlayBufferSize * NVitaSound::kNumAT9PlayBuffers, mTotalStreamSize);
      
      SAudioTrack& currTrack = mAudioTracks[0];

      // Wait until we have enough data to start playback.
      if (currTrack.mNumBytesAvailable < kNumBytesToQueue)
         return;

      // Kill any zombie voices that the game did not actually stop
      for (int i = 0; i < mTrackCount; i++)
      {
         res = sceNgsVoiceGetInfo(mAudioTracks[i].mVoiceHandle, &voiceInfo);
         BPE_VERIFY(res == SCE_OK, false, "Could not get voice information.");

         if (voiceInfo.uVoiceState != SCE_NGS_VOICE_STATE_AVAILABLE)
         {
            res = sceNgsVoiceKill(mAudioTracks[i].mVoiceHandle);
            BPE_VERIFY(res == SCE_OK, false, "Could not kill voice.");
         }
      }

      mPlaybackState = NVitaSound::kDOSS_Queuing;
   }
   
   if (mPlaybackState == NVitaSound::kDOSS_Queuing)
   {
      bool tracksReady = false;

      // Depending on the voice state, killing it above may have not actually killed it (or finished killing it rather)
      // So wait until they are in fact available. 
      while (!tracksReady)
      {
         for (int i = 0; i < mTrackCount; i++)
         {
            int res = sceNgsVoiceGetInfo(mAudioTracks[i].mVoiceHandle, &voiceInfo);
            BPE_VERIFY(res == SCE_OK, false, "Could not get voice information.");

            // Apparently killing a paused voice does not actually kill it. Resume here and immediately kill it.
            if (voiceInfo.uVoiceState == SCE_NGS_VOICE_STATE_PAUSED)
            {
               res = sceNgsVoiceResume(mAudioTracks[i].mVoiceHandle);
               BPE_VERIFY(res == SCE_OK, false, "Could not kill voice.");
               continue;
            }

            if (voiceInfo.uVoiceState != SCE_NGS_VOICE_STATE_AVAILABLE)
            {
               res = sceNgsVoiceKill(mAudioTracks[i].mVoiceHandle);
               BPE_VERIFY(res == SCE_OK, false, "Could not kill voice.");
               continue;
            }

            tracksReady = true;
         }
         
         mPaused = false;
      }

      PrepareAT9StreamForPlayback();

      // Finally we can play!
      mPlaybackState = NVitaSound::kDOSS_Playing;
      for (int i = 0; i < mTrackCount; i++)
         sceNgsVoicePlay(mAudioTracks[i].mVoiceHandle);
   }
}

//----------------------------------------------------------------------------
void CAudioStream::Pause(bool isPaused)
{
   BPE_VERIFY(mAudioFormat == NVitaSound::kAUDIO_FORMAT_ATRAC9, false, "Invalid audio format.");
   mPaused = isPaused;
   ApplyPause();
}

//----------------------------------------------------------------------------
void CAudioStream::ApplyPause()
{
   // Make sure we aren't currently queuing up audio data...
   CSyncCriticalSectionLocker lock(mStreamMutex);

   for (int currTrack = 0; currTrack < mTrackCount; currTrack++)
   {
      int returnCode;

      if (mPaused || gDirectOutputStreamsPaused)
      {
         returnCode = sceNgsVoicePause(mAudioTracks[currTrack].mVoiceHandle);
      }
      else
      {
         returnCode = sceNgsVoiceResume(mAudioTracks[currTrack].mVoiceHandle);
      }

      BPE_VERIFY(returnCode == SCE_OK, false, "Unable to pause audio.");
   }
}

//----------------------------------------------------------------------------
void CAudioStream::Stop(bool bImmediate)
{
   // Make sure we aren't currently queuing up audio data...
   CSyncCriticalSectionLocker lock(mStreamMutex);

   if (GetState() == NVitaSound::kDOSS_Closed)
      return;

   SceNgsVoiceInfo voiceInfo;
   int currTrack = 0;
   while (currTrack < mTrackCount)
   {
      int res = sceNgsVoiceGetInfo(mAudioTracks[currTrack].mVoiceHandle, &voiceInfo);
      BPE_VERIFY(res == SCE_OK, false, "Could not get voice information.");

      // Apparently killing a paused voice does not actually kill it. Resume here and immediately kill it.
      if (voiceInfo.uVoiceState == SCE_NGS_VOICE_STATE_PAUSED)
      {
         res = sceNgsVoiceResume(mAudioTracks[currTrack].mVoiceHandle);
         BPE_VERIFY(res == SCE_OK, false, "Could not resume voice.");
         continue;
      }

      currTrack++;
   }

   for (currTrack = 0; currTrack < mTrackCount; currTrack++)
   {
      int returnCode = sceNgsVoiceKeyOff(mAudioTracks[currTrack].mVoiceHandle);
      BPE_VERIFY(returnCode == SCE_OK, false, "Unable to key off audio voice.");
   }

   mIsAT9StreamDonePlaying = true;
   mPlaybackState = NVitaSound::kDOSS_Closed;
}

//----------------------------------------------------------------------------
int CAudioStream::GetBytePosition()
{
   // Finish any data shuffling...
   CSyncCriticalSectionLocker lock(mStreamMutex);

   if (!IsPlaying())
      return 0;

   // We must multiple by the number of tracks to get the actual position within the stream since tracks are combined
   // into single packets.
   return (mAudioTracks[0].mTotalBytesReadFromStreamBuffer * mTrackCount) % NVitaSound::kAT9StreamBufferSize;
}

//----------------------------------------------------------------------------
double CAudioStream::GetPlayTime()
{
   if (!IsPlaying())
      return 0.0;

   // Since the tracks are synchronized just the first track's play time.

   // Check the voice info to make sure there have been active updates. If active updates is zero the voice state may
   // contain stale data!
   SceNgsVoiceInfo voiceInfo;
   int returnCode = sceNgsVoiceGetInfo(mAudioTracks[0].mVoiceHandle, &voiceInfo);
   if (voiceInfo.uUpdateCallsActive == 0)
      return 0.0;

   SceNgsAT9States voiceState;
   returnCode = sceNgsVoiceGetStateData(mAudioTracks[0].mVoiceHandle, SCE_NGS_VOICE_AT9_PLAYER, &voiceState, sizeof(SceNgsAT9States));
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to get voice state.");

   // All ATRAC9 audio streams come out as stereo samples even though the source data is mono. Basically the voice takes the mono
   // input data once processing is complete on the voice a stereo sample is output. SamplesGeneratedSinceKeyOn is the 
   // stereo samples.

   // CHANGED THIS TO HARDCODE 48khz. The AT9 files were changed by JP which threw off the timing. The system is generating
   // samples at 48khz so use this value in all cases.
#if MGS_VERSION == 3
   return (double)voiceState.nSamplesGeneratedSinceKeyOn / (48000 * 2.0);
#else
   return (double)voiceState.nSamplesGeneratedSinceKeyOn / (mFrequency * 2.0);
#endif
}

//----------------------------------------------------------------------------
void CAudioStream::SetTrackVolume(int const track, float const fVol)
{
   if (track >= BP_DOSTREAM_TRACK_COUNT || mNumChannels == 0 || track >= mTrackCount)
      return; // Unused track...

   if (fVol >= 0.0f)
      mAudioTracks[track].mVolume = fVol;
   else
      mAudioTracks[track].mVolume = fVol;

   SceNgsPatchRouteInfo patchInfo;
   sceNgsPatchGetInfo(mAudioTracks[track].mPatchHandleDry, &patchInfo, NULL);

   patchInfo.vols.m[0][0] = mAudioTracks[track].mVolume;
   patchInfo.vols.m[0][1] = 0.0f; // left to right
   patchInfo.vols.m[1][0] = 0.0f; // right to left
   patchInfo.vols.m[1][1] = mAudioTracks[track].mVolume;

   int returnCode = sceNgsVoicePatchSetVolumesMatrix(mAudioTracks[track].mPatchHandleDry, &patchInfo.vols);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to set volume on track.");
}

//----------------------------------------------------------------------------
void CAudioStream::SetTrackVolume(int const track, float const volumes[2][2], BP_SURROUND_VOICE_TYPE surroundType)
{
   if (track >= BP_DOSTREAM_TRACK_COUNT || mNumChannels == 0)
      return; // Unused track...

   BPE_VERIFY(track < mTrackCount, false, "Invalid track.");

   // Set volume on both the wet AND dry signal so reverb effects are lower as the camera distance changes.

   SceNgsVolumeMatrix newVolMatrix;
   
   // For mono surround streams, L/R stereo volume is stored in channel 0
   if (mNumChannels == 1 && surroundType == BP_SURROUND_VOICE_TYPE_SURROUND)
   {
      newVolMatrix.m[0][0] = volumes[0][0];
      newVolMatrix.m[0][1] = 0.0f;
      newVolMatrix.m[1][0] = 0.0f;
      newVolMatrix.m[1][1] = volumes[0][1];
   }
   else
   {
      // Mono center spread streams or stereo streams.
      if (mNumChannels == 1)
      {
#if MGS_VERSION == 2
         // MGS2 puts the center spread volumes in the first channel.
         newVolMatrix.m[0][0] = volumes[0][0]; // left to left
         newVolMatrix.m[0][1] = 0.0f; // left to right
         newVolMatrix.m[1][0] = 0.0f; // right to left
         newVolMatrix.m[1][1] = volumes[0][1]; // right to right
#else
         // MGS3 puts center spread in L of channel 0 and R of channel 1
         newVolMatrix.m[0][0] = volumes[0][0]; // left to left
         newVolMatrix.m[0][1] = 0.0f; // left to right
         newVolMatrix.m[1][0] = 0.0f; // right to left
         newVolMatrix.m[1][1] = volumes[1][1]; // right to right
#endif
      }
      else
      {
         newVolMatrix.m[0][0] = volumes[0][0]; // left to left
         newVolMatrix.m[0][1] = 0.0f; // left to right
         newVolMatrix.m[1][0] = 0.0f; // right to left
         newVolMatrix.m[1][1] = volumes[1][1]; // right to right
      }
   }

   int returnCode = sceNgsVoicePatchSetVolumesMatrix(mAudioTracks[track].mPatchHandleDry, &newVolMatrix);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to set volume on track.");

   if (mAudioTracks[track].mPatchHandleWet)
   {
      int returnCode = sceNgsVoicePatchSetVolumesMatrix(mAudioTracks[track].mPatchHandleWet, &newVolMatrix);
      BPE_VERIFY(returnCode == SCE_OK, false, "Unable to set volume on track.");
   }
}

//----------------------------------------------------------------------------
void CAudioStream::ShowStreamStats()
{
   int numLoops = mTotalStreamSize > 0 ? (mAudioTracks[0].mTotalBytesReadFromStreamBuffer / mTotalStreamSize) : 0;

   BP_DebugText_Print("AT9Stream: Tracks(%d), TotalRead(%d) TotalWritten(%d) Avail(%d) NumTimesLooped(%d)", 
      mTrackCount, 
      mAudioTracks[0].mTotalBytesReadFromStreamBuffer, 
      mAudioTracks[0].mTotalBytesWrittenToStreamBuffer,
      mAudioTracks[0].mNumBytesAvailable,
      numLoops);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void BP_SoundSupport_Init_PlatformSpecific()
{
   NVitaSound::init_audio();
}

//----------------------------------------------------------------------------
void BP_SoundSupport_Shutdown()
{
   NVitaSound::shutdown_audio();
}

//----------------------------------------------------------------------------
void BP_SoundSupport_SetMasterVolume(float volume)
{
   int volumeMatrix[2] = { (int)(SCE_AUDIO_VOLUME_0dB * volume), (int)(SCE_AUDIO_VOLUME_0dB * volume) };
   sceAudioOutSetVolume(gNgsState.mAudioHardwarePort, (SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH), 
      volumeMatrix);
}

//----------------------------------------------------------------------------
void BP_InitDirectOutputStream(DirectOutputStreamInitData const* pInitInfo)
{
   BPE_ASSERT_NO_MSG(pInitInfo->trackCount > 0 && pInitInfo->trackCount <= BP_DOSTREAM_TRACK_COUNT);
   BPE_ASSERT_NO_MSG(pInitInfo->format == NVitaSound::kAUDIO_FORMAT_ATRAC9);
   gNgsState.mAudioStreams[pInitInfo->channel]->SetupStream(pInitInfo);
}

//----------------------------------------------------------------------------
unsigned char* BP_GetDirectOutputStreamPlaybackBuffer(const int channel)
{
   BPE_VERIFY(gNgsState.mAudioStreams[channel]->GetAudioFormat() == NVitaSound::kAUDIO_FORMAT_ATRAC9, 
      false, "Invalid audio format.");
   return NULL; // ATRAC9 audio formatted streams used a different buffer.
}

//----------------------------------------------------------------------------
void BP_ContinuePlayDirectOutputStream(const int channel)
{
   BPE_VERIFY(gNgsState.mAudioStreams[channel]->GetAudioFormat() == NVitaSound::kAUDIO_FORMAT_ATRAC9, 
      false, "Invalid audio format.");
   return; // AT9 streams use a different interface.
}

//----------------------------------------------------------------------------
void BP_NotifyDirectOutputStreamBytesAvailable(int channel, void* buffer, int sizeBytes)
{
   BPE_VERIFY(gNgsState.mAudioStreams[channel]->GetAudioFormat() == NVitaSound::kAUDIO_FORMAT_ATRAC9, 
      false, "Invalid audio format.");
   gNgsState.mAudioStreams[channel]->ConsumeAudioData(reinterpret_cast<unsigned char const*>(buffer), sizeBytes);
}

//----------------------------------------------------------------------------
void BP_SetDirectOutputStreamPaused(const int channel, int paused)
{
   BPE_VERIFY(gNgsState.mAudioStreams[channel]->GetAudioFormat() == NVitaSound::kAUDIO_FORMAT_ATRAC9, 
      false, "Invalid audio format.");
   gNgsState.mAudioStreams[channel]->Pause(paused != 0);
}

//----------------------------------------------------------------------------
int BP_IsDirectOutputStreamDone( const int channel )
{
   if (gNgsState.mAudioStreams[channel]->IsStreamDonePlaying())
   {
      BP_StopDirectOutputStream(channel);
   }

   return gNgsState.mAudioStreams[channel]->IsStreamDonePlaying();
}

//----------------------------------------------------------------------------
void BP_StopDirectOutputStream(const int channel)
{
   gNgsState.mAudioStreams[channel]->Stop(false);
}

//----------------------------------------------------------------------------
int BP_GetDirectOutputStreamPosition(const int channel)
{
   return gNgsState.mAudioStreams[channel]->GetBytePosition();
}

//----------------------------------------------------------------------------
int BP_GetDirectOutputStreamReadAheadSize(const int channel)
{
   return NVitaSound::kAT9StreamBufferSize / 2;
}

//----------------------------------------------------------------------------
double BP_GetDirectOutputStreamPlayTime( const int channel )
{
   return gNgsState.mAudioStreams[channel]->GetPlayTime();
}

//----------------------------------------------------------------------------
void BP_SoundSupport_Update()
{
   for (int channel=0; channel < NVitaSound::kNumSpuVoiceChannels; ++channel)
   {
      for (int voice=0; voice < NVitaSound::kNumSpuVoices; ++voice)
      {
         SSpuVoiceState* pVoiceState = gNgsState.mpSpuVoiceState[channel][voice];

         if (pVoiceState->mADSREnvelope.mADSRState != kADSRState_Off)
         {
            NVitaSound::apply_spu_voice_volume(pVoiceState, true);
         }
      }
   }
}

//----------------------------------------------------------------------------
void BP_SoundSupport_InitADPCMStreamingBuffer( const int channel, const int lr, const int spuAddr, const int spuSize )
{
   // Vita does not need to do this since we are not doing any ADPCM decoding manually.
}

//----------------------------------------------------------------------------
void BP_SetDirectOutputStreamVolume_Direct(const int channel, const int track, const float fVol)
{
   gNgsState.mAudioStreams[channel]->SetTrackVolume(track, fVol);
}

//----------------------------------------------------------------------------
void BP_SetDirectOutputStreamVolume_Voice( const int channel, int stereoVols[2][2], float pan3d, float vol3d, BP_SURROUND_VOICE_TYPE surroundType)
{
   (void)channel;
   (void)stereoVols;
   (void)pan3d;
   (void)vol3d;
   (void)surroundType;

   // On Vita, we only can support stereo audio so we can ignore 3d panning and 3d volume and just use the stereo volumes.
   float vols[2][2];

   vols[0][0] = BP_PS2VolToFVol(stereoVols[0][0]);
   vols[0][1] = BP_PS2VolToFVol(stereoVols[0][1]);
   vols[1][0] = BP_PS2VolToFVol(stereoVols[1][0]);
   vols[1][1] = BP_PS2VolToFVol(stereoVols[1][1]);

   gNgsState.mAudioStreams[channel]->SetTrackVolume(0, vols, surroundType);
}

//----------------------------------------------------------------------------
void BP_SoundSupport_SurroundSoundSetVoiceType(const int core, const int voice, BP_SURROUND_VOICE_TYPE voiceType)
{
   (void)core;
   (void)voice;
   (void)voiceType;

   // Vita only supports stereo audio so make sure the game is handling Vita correctly.
   // AS MCampbell 3/21/12 - In order to correctly simulate 3D volume using stereo output the code was changed to use 
   // voiceType as an indicator. Therefore, the code needs to use voice types other than stereo.
   // Bug: MGSTWO-3245, MGSTWO-3244
   //BPE_VERIFY(voiceType == BP_SURROUND_VOICE_TYPE_STEREO, false, "Only stereo output is supported on Vita.");
}

//----------------------------------------------------------------------------
void BP_SetDirectOutputStreamDsp_Voice( const int channel, int bDspEnabled )
{
   gNgsState.mAudioStreams[channel]->SetReverb(bDspEnabled);
}

//----------------------------------------------------------------------------
void BP_SoundSupport_SurroundSoundSetVoicePanVol(const int core, const int voice, float pan, float vol)
{
   (void)core;
   (void)voice;
   (void)pan;
   (void)vol;

   // On Vita, we only support stereo audio output so just use the original PS2 stereo left/right volume already set
   // on the voice.
}

//----------------------------------------------------------------------------
void BP_PlayMemStream(const int core, const int voice, const void * const playbackBuffer, const int playbackBufferSize)
{
   BPE_ASSERT_NO_MSG(core < NVitaSound::kNumSpuVoiceChannels && voice < NVitaSound::kNumSpuVoices);

   NVitaSound::SSpuVoiceState* pVoiceState = gNgsState.mpSpuVoiceState[core][voice];
   pVoiceState->mpMemoryStream = (const unsigned char*)playbackBuffer;
   pVoiceState->mMemoryStreamSize = playbackBufferSize;
   pVoiceState->mMemoryStreamMuteVolume = 1;
}

//----------------------------------------------------------------------------
int BP_GetMemStreamPosition(const int core, const int voice)
{
   return NVitaSound::get_spu_stream_byte_pos(gNgsState.mpSpuVoiceState[core][voice]);
}

//----------------------------------------------------------------------------
void BP_SetMemStreamMuted(const int core, const int voice, const int muted)
{
   BPE_ASSERT_NO_MSG(core < NVitaSound::kNumSpuVoiceChannels && voice < NVitaSound::kNumSpuVoices);

   NVitaSound::SSpuVoiceState* pVoice = gNgsState.mpSpuVoiceState[core][voice];
   if (pVoice->mpMemoryStream)
   {
      pVoice->mMemoryStreamMuteVolume = muted ? 0 : 1;
      NVitaSound::apply_spu_voice_volume(pVoice, false);
   }
}

//----------------------------------------------------------------------------
void BP_SoundSupport_ApplyVoiceVolume( const int core, const int voice )
{
   BPE_ASSERT( core >= 0 && core < 2, "" );
   BPE_ASSERT( voice >= 0 && voice < NVitaSound::kNumSpuVoices, "" );
   NVitaSound::apply_spu_voice_volume(gNgsState.mpSpuVoiceState[core][voice], false);
}

//----------------------------------------------------------------------------
void BP_sceSdSetParam(unsigned short entry, unsigned short value)
{
   int reg = entry & 0xffc0;
   int core = entry & 1;
   int voice = ( entry & ~0xffc0 ) >> 1;

   BPE_ASSERT_NO_MSG(voice < NVitaSound::kNumSpuVoices);
   NVitaSound::SSpuVoiceState* pVoiceState = gNgsState.mpSpuVoiceState[core][voice];

   switch( reg )
   {
   case SD_VP_VOLL:
      {
         //voice volume (left)
         if( value & 0x8000 )
         {
            BP_TODO_BREAK;
         }
         else
         {
            pVoiceState->mVolumeL = BP_PS2VolToFVol(value);

            // Volumes set in BP_SoundSupport_ApplyVoiceVolume
         }
      }
      break;

   case SD_VP_VOLR:
      {
         //voice volume (right)
         if( value & 0x8000 )
         {
            BP_TODO_BREAK;
         }
         else
         {
            pVoiceState->mVolumeR = BP_PS2VolToFVol(value);

            // Volumes set in BP_SoundSupport_ApplyVoiceVolume
         }
      }
      break;

   case SD_VP_PITCH:
      {
         // Convert from PS2 pitch (0 - 0x3fff) to frequency.
         float newPitch = value * 48000.0f / 4096.0f;
         if (newPitch != pVoiceState->mPitch)
         {
            pVoiceState->mPitch = newPitch;
            NVitaSound::apply_spu_pitch(pVoiceState);
         }
      }
      break;

   case SD_VP_ADSR1:
      pVoiceState->mADSREnvelope.mADSR1 = value;
      break;

   case SD_VP_ADSR2:
      pVoiceState->mADSREnvelope.mADSR2 = value;
      break;

   case SD_P_EVOLL:
      gReverbDepthPS2 = value;
      break;

   case SD_P_EVOLR:
      BPE_VERIFY(value == gReverbDepthPS2, false, "L/R reverb settings should be the same.");
      NVitaSound::apply_system_reverb();
      break;

   default:
      bpe_debugger_printf( "Warning: unimplemented sceSdSetParam(%d).\n", reg);
      BP_TODO_BREAK;
      break;
   }
}

//----------------------------------------------------------------------------
unsigned short BP_sceSdGetParam(unsigned short entry)
{
   int reg = entry & 0xffc0;
   int core = entry & 1;
   int voice = ( entry & ~0xffc0 ) >> 1;
   BPE_ASSERT_NO_MSG(voice < NVitaSound::kNumSpuVoices);

   SceNgsVoiceInfo voiceInfo;
   int returnCode = sceNgsVoiceGetInfo(gNgsState.mpSpuVoiceState[core][voice]->mVoiceHandle, &voiceInfo);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to get voice info. for SPU voice.");

   switch (reg)
   {
      case SD_VP_ENVX:
         if (voiceInfo.uVoiceState & SCE_NGS_VOICE_STATE_ACTIVE)
         {
            if (voiceInfo.uVoiceState & SCE_NGS_VOICE_STATE_PENDING)
            {
               return 0xffff;
            }
            else
            {
               float currEnv = gNgsState.mpSpuVoiceState[core][voice]->mADSREnvelope.mEnvelopeScale;
               unsigned short releaseLevel = (unsigned short)(currEnv * 0xffff);
               return releaseLevel;
            }
         }
         else
         {
            return 0;
         }

      default:
         BPE_VERIFYA(false, "Warning: unimplemented sceSdGetParam");
         break;
   }

   return 0;
}

//----------------------------------------------------------------------------
unsigned int BP_sceSdGetAddr(unsigned short entry)
{
   int reg = entry & 0xffc0;
   int core = entry & 1;
   int voice = ( entry & ~0xffc0 ) >> 1;
   BPE_ASSERT_NO_MSG(voice < NVitaSound::kNumSpuVoices);

   switch (reg)
   {
      case SD_VA_NAX:
         return NVitaSound::get_spu_stream_byte_pos(gNgsState.mpSpuVoiceState[core][voice]);
      
      default:
         BPE_VERIFYA(false, "Unimplemented sceSdGetAddr.");
   }

   return 0;
}

//----------------------------------------------------------------------------
void BP_sceSdSetAddr(unsigned short entry, unsigned int value)
{
   int reg = entry & 0xffc0;
   int core = entry & 1;
   int voice = ( entry & ~0xffc0 ) >> 1;
   BPE_ASSERT_NO_MSG(voice < NVitaSound::kNumSpuVoices);

   NVitaSound::SSpuVoiceState* pVoiceState = gNgsState.mpSpuVoiceState[core][voice];
   switch (reg)
   {
      case SD_VA_SSA:
         pVoiceState->mSSA = value;
         pVoiceState->mpMemoryStream = NULL;   //cancel any memory stream setting
         pVoiceState->mMemoryStreamMuteVolume = 1;
         break;

      case SD_VA_LSAX:
         pVoiceState->mLSAX = value;         
         break;

      default:
        BPE_VERIFYA(false, "Unimplemented sceSdSetAddr.");
        break;
   }   
}

//----------------------------------------------------------------------------
void BP_sceSdSetSwitch(unsigned short entry, unsigned int value)
{
   int reg = entry & 0xffc0;
   int core = entry & 1;
   int returnCode = SCE_OK;

   BPE_ASSERT_NO_MSG(core < NVitaSound::kNumSpuVoiceChannels);

   switch (reg)
   {
      case SD_S_KOFF:
         for (int i = 0; i < NVitaSound::kNumSpuVoices; i++)
         {
            int voiceBit = 1 << i;
            if (value & voiceBit)
            {
               if (gNgsState.mpSpuVoiceState[core][i]->mADSREnvelope.mADSRState != kADSRState_Off)
                  gNgsState.mpSpuVoiceState[core][i]->mADSREnvelope.mADSRState = kADSRState_Release;
            }
         }
         break;

      case SD_S_KON:
         for (int i = 0; i < NVitaSound::kNumSpuVoices; i++)
         {
            int voiceBit = 1 << i;
            if (value & voiceBit)
            {
               if (gBP_BGMDebugVoiceNum != -1)
               {
                  if (core == 0 && i == gBP_BGMDebugVoiceNum)
                     NVitaSound::key_on_spu_voice_stream(gNgsState.mpSpuVoiceState[core][i]);
               }
               else
               {
                  NVitaSound::key_on_spu_voice_stream(gNgsState.mpSpuVoiceState[core][i]);
               }
            }
         }
         break;

      case SD_S_NON:
         //allocation to noise generator
         if (value != 0)
         {
            //TODO: Support noise generator...
         }
         break;

      case SD_S_PMON:
         //pitch modulation per voice
         if( value != 0 )
         {
            // TODO: Support pitch modulation...
         }
         break;

      case SD_S_VMIXEL:
      case SD_S_VMIXER:
         {
            //Voice output mixing flag per voice (wet left)
            //Voice output mixing flag per voice (wet right)
            for (int i = 0; i < NVitaSound::kNumSpuVoices; ++i)
            {
               NVitaSound::SSpuVoiceState* pVoiceState = gNgsState.mpSpuVoiceState[core][i];

               int voiceBit = 1 << i;
               bool bReverbEnabled = ( value & voiceBit ) != 0;
               if( reg == SD_S_VMIXEL )
               {
                  pVoiceState->mReverbEnabledL = bReverbEnabled;
               }
               else
               {
                  //TED - they always set SD_S_VMIXEL / SD_S_VMIXER together, in that order,
                  //and to the same value.  Apply once as a flag for the entire channel
                  //rather than "properly" implementing via separate L/R wet volumes.
                  BPE_VERIFY(bReverbEnabled == pVoiceState->mReverbEnabledL, false, "Should not be different values.");
                  pVoiceState->mReverbEnabledR = bReverbEnabled;
                  NVitaSound::apply_spu_voice_reverb(pVoiceState);
               }
            }
         }
         break;

      default:
         BPE_VERIFYA(false, "Unimplemented BP_sceSdSetSwitch.");
         break;
   }
}

//----------------------------------------------------------------------------
void BP_sceSdSetCoreAttr(unsigned short entry, unsigned short value)
{
   int reg = entry & 0xfffe;
   switch (reg)
   {
      case SD_C_NOISE_CLK:
         break;

      case SD_C_EFFECT_ENABLE:
         break;
   
      default:
         BPE_VERIFYA(false, "Unimplemented BP_sceSdSetCoreAttr(%d).");
         break;
   }
}

//----------------------------------------------------------------------------
unsigned short BP_sceSdNote2Pitch (unsigned short center_note, unsigned short center_fine, unsigned short note, 
   short fine)
{
   const float BaseA4 = 4096.0f;
   const float NumOctaves = 1.0f / 12.0f;

   float noteDiff = (float)(note - center_note);
   float centerFine = center_fine / 128.0f;
   float noteFine = fine / 128.0f;
   float fineDiff = fabs(centerFine - noteFine);

   unsigned short pitch = (unsigned short)(BP_Float_Floor(BaseA4 * powf(2.0f, (noteDiff + fineDiff) * NumOctaves) 
      + 0.5f));

   return pitch;
}

//----------------------------------------------------------------------------
void BP_SoundSupport_InitDebugMenuPlatformSpecific(int soundMenu, int reverbMenu)
{
   static const char* kReverbPresetModes[] = 
   {
      "Not Overridden",
      "Padded Cell",
      "Room",
      "BathRoom",
      "Living Room",
      "Stone Room",
      "Auditorium",
      "Concert Hall",
      "Cave",
      "Arena",
      "Hanger",
      "Carpeted Hallway",
      "Hallway",
      "Stone Corridor",
      "Alley",
      "Forest",
      "City",
      "Mountains",
      "Quarry",
      "Plain",
      "Parking Lot",
      "Underwater",
      "Small Room",
      "Medium Room",
      "Large Room",
      "Medium Hall",
      "Large Hall",
   };

   struct CallbackReveb
   {
      static int ResetReverb(int data)
      {
         gBP_UseCustomReverbSettings = 0;
         sceNgsSystemLock(gNgsState.mNgsSysHandle);
         NVitaSound::apply_system_reverb();
         sceNgsSystemUnlock(gNgsState.mNgsSysHandle);
         return 0;
      }

      static int SetCustomReverb(int data)
      {
         gBP_UseCustomReverbSettings = 1;
         sceNgsSystemLock(gNgsState.mNgsSysHandle);
         NVitaSound::apply_system_reverb();
         sceNgsSystemUnlock(gNgsState.mNgsSysHandle);
         return 0;
      }

      static int SetReverbVol(int data)
      {
         sceNgsSystemLock(gNgsState.mNgsSysHandle);
         NVitaSound::apply_system_reverb();
         sceNgsSystemUnlock(gNgsState.mNgsSysHandle);
         return 0;
      }
   };

#if BP_ENABLE_DEBUG_MENU
   int item = BP_DebugMenu_AddBool(reverbMenu, "Enable Reverb", &gBP_EnableAudioReverb);
   BP_DebugMenu_SetCallback(item, &CallbackReveb::SetReverbVol, NULL);

   item = BP_DebugMenu_AddFloat(reverbMenu, "Reverb Strength", &gBP_MasterReverbStrength, 0.0f, 4.5f, 0.1f, 0.1f);
   BP_DebugMenu_SetCallback(item, &CallbackReveb::SetReverbVol, NULL);

   item = BP_DebugMenu_AddFloat(reverbMenu, "room (main) volume control in mB -10000 to 0 mB ( 100 mB == 1 dB )", 
      &gNgsState.mpCustomReverbParams->mRoom, -10000.0f, 0.0f, 100.0f, 1000.0f);
   BP_DebugMenu_SetCallback(item, &CallbackReveb::SetCustomReverb, NULL);

   item = BP_DebugMenu_AddFloat(reverbMenu, "high frequency content input in mB, at fHF_reference  -10000 to 0 mB", 
      &gNgsState.mpCustomReverbParams->mRoomHF, -10000.0f, 0.0f, 100.0f, 1000.0f);
   BP_DebugMenu_SetCallback(item, &CallbackReveb::SetCustomReverb, NULL);

   item = BP_DebugMenu_AddFloat(reverbMenu, "decay time of late reverberation in seconds 0.1 to 20 seconds", 
      &gNgsState.mpCustomReverbParams->mDecayTime, 0.1f, 20.0f, 0.1f, 1.0f);
   BP_DebugMenu_SetCallback(item, &CallbackReveb::SetCustomReverb, NULL);

   item = BP_DebugMenu_AddFloat(reverbMenu, "ratio of high frequency decay in late reverb 0.1 to 2.0", 
      &gNgsState.mpCustomReverbParams->mDecayHFRatio, 0.1f, 2.0f, 0.1f, 0.5f);
   BP_DebugMenu_SetCallback(item, &CallbackReveb::SetCustomReverb, NULL);
   
   item = BP_DebugMenu_AddFloat(reverbMenu, "volume of early reflections in mB -10000 to 1000 mB", 
      &gNgsState.mpCustomReverbParams->mReflections, -10000.0f, 1000.0f, 100.0f, 1000.0f);
   BP_DebugMenu_SetCallback(item, &CallbackReveb::SetCustomReverb, NULL);

   item = BP_DebugMenu_AddFloat(reverbMenu, "delay from input signal until first reflection 0.0 to 0.3 seconds", 
      &gNgsState.mpCustomReverbParams->mReflectionsDelay, 0.0f, 0.3f, 0.01f, 0.1f);
   BP_DebugMenu_SetCallback(item, &CallbackReveb::SetCustomReverb, NULL);

   item = BP_DebugMenu_AddFloat(reverbMenu, "volume of late reverb in mB -10000 to 2000 mB ", 
      &gNgsState.mpCustomReverbParams->mReverb, -10000.0f, 2000.0f, 100.0f, 1000.0f);
   BP_DebugMenu_SetCallback(item, &CallbackReveb::SetCustomReverb, NULL);

   item = BP_DebugMenu_AddFloat(reverbMenu, "delay from first reflection until late reverb 0.0 to 0.1 seconds", 
      &gNgsState.mpCustomReverbParams->mReverbDelay, 0.0f, 0.1f, 0.001f, 0.01f);
   BP_DebugMenu_SetCallback(item, &CallbackReveb::SetCustomReverb, NULL);

   item =  BP_DebugMenu_AddFloat(reverbMenu, "echo density in late reverberation 0 to 100 % ", 
      &gNgsState.mpCustomReverbParams->mDiffusion, 0.0f, 100.0f, 1.0f, 10.0f);
   BP_DebugMenu_SetCallback(item, &CallbackReveb::SetCustomReverb, NULL);

   item = BP_DebugMenu_AddFloat(reverbMenu, "modal density in late reverberation 0 to 100 %", 
      &gNgsState.mpCustomReverbParams->mDensity, 0.0f, 100.0f, 1.0f, 10.0f);
   BP_DebugMenu_SetCallback(item, &CallbackReveb::SetCustomReverb, NULL);

   item = BP_DebugMenu_AddFloat(reverbMenu, "reference frequency for fRoom_HF and fDecay_HF_ratio 20 to 20000 Hertz", 
      &gNgsState.mpCustomReverbParams->mHFReference, 20.0f, 20000.0f, 100.0f, 1000.0f);
   BP_DebugMenu_SetCallback(item, &CallbackReveb::SetCustomReverb, NULL);

   item = BP_DebugMenu_AddFloat(reverbMenu, "for early reflections. 0 = single reflection 100 = widely spread reflections", 
      &gNgsState.mpCustomReverbParams->mEarlyReflectionScalar, 0.0f, 100.0f, 1.0f, 10.0f);
   BP_DebugMenu_SetCallback(item, &CallbackReveb::SetCustomReverb, NULL);

   item = BP_DebugMenu_AddFloat(reverbMenu, "reference low frequency 20 to 20000 Hertz ", 
      &gNgsState.mpCustomReverbParams->mLFReference, 20.0f, 20000.0f, 100.0f, 1000.0f);
   BP_DebugMenu_SetCallback(item, &CallbackReveb::SetCustomReverb, NULL);

   item = BP_DebugMenu_AddFloat(reverbMenu, "low frequency content input in mB, at fLF_reference   -10000 to 0 mB", 
      &gNgsState.mpCustomReverbParams->mRoomLF, -10000.0f, 0.0f, 100.0f, 1000.0f);
   BP_DebugMenu_SetCallback(item, &CallbackReveb::SetCustomReverb, NULL);

   item = BP_DebugMenu_AddFloat(reverbMenu, "dry volume in mB -10000 to 0 mB", 
      &gNgsState.mpCustomReverbParams->mDryMB, -10000.0f, 0.0f, 100.0f, 1000.0f);
   BP_DebugMenu_SetCallback(item, &CallbackReveb::SetCustomReverb, NULL);


   item = BP_DebugMenu_AddEnum(reverbMenu, "Reverb Preset", kReverbPresetModes, &gBP_OverriddenVitaReverbMode, 0, sizeof(kReverbPresetModes) / sizeof(char*) - 1);
   BP_DebugMenu_SetCallback(item, &CallbackReveb::ResetReverb, NULL);


   BP_DebugMenu_AddInt(soundMenu, "Ping Pitch", &gBP_WavFormPitch, 24000, 48000, 10, 100);
   BP_DebugMenu_AddFloat(soundMenu, "Ping Volume", &gBP_WavFormVolume, 0.0f, 1.0f, 0.1f, 0.1f);
   BP_DebugMenu_AddInt(soundMenu, "VoiceNum", &gBP_BGMDebugVoiceNum, -1, 23, 1, 1);
#endif
}

//----------------------------------------------------------------------------
extern "C" void mtast_debug_print();
void BP_SoundSupport_ShowStreamStats_PlatformSpecific()
{
   /*
   for(int i = 0; i < BP_DOSTREAM_COUNT; ++i)
   {
      gNgsState.mAudioStreams[i]->ShowStreamStats();
   }
   */
   NVitaSound::CAudioStream* pAudioStream = gNgsState.mAudioStreams[gBP_MTAChannel];
   if (!pAudioStream->IsPlaying())
      return;

   BP_DebugText_Print("BGM Stream: NumTracks = %d", pAudioStream->GetTrackCount());
   for (int i = 0; i < pAudioStream->GetTrackCount(); i++)
   {
      BP_DebugText_Print("Track(%d): Vol: %.2f", i, pAudioStream->GetAudioTrack(i)->mVolume);
   }

#if MGS_VERSION == 3
//   mtast_debug_print();
#endif
}

//----------------------------------------------------------------------------
void BP_SoundSupport_ShowVoiceStats_PlatformSpecific()
{
}

//----------------------------------------------------------------------------
void BP_SoundSupport_ApplyReverbSettings_PlatformSpecific()
{
   NVitaSound::apply_system_reverb();
}

//----------------------------------------------------------------------------
void BP_SoundSupport_ApplyDirectOutputStreamsPaused_PlatformSpecific()
{
   for (int ch=0; ch < BP_DOSTREAM_COUNT; ++ch)
      gNgsState.mAudioStreams[ch]->ApplyPause();
}

//----------------------------------------------------------------------------
unsigned int BP_PlayWaveform( const unsigned char * const waveformData, const int size, const int frequency, const int channelCount )
{
   SceNgsBufferInfo bufferInfo;
   SceNgsVoiceInfo voiceInfo;
   int returnCode = SCE_OK;

   BPE_VERIFY(gNgsState.mStandaloneWavVoice, false, "Invalid voice handle.");
   BPE_VERIFY(channelCount == 1, false, "Unsupported channel count.");

   sceNgsVoiceGetInfo(gNgsState.mStandaloneWavVoice, &voiceInfo);
   if (voiceInfo.uVoiceState != SCE_NGS_VOICE_STATE_AVAILABLE)
      sceNgsVoiceKill(gNgsState.mStandaloneWavVoice);

   returnCode = sceNgsVoiceInit(gNgsState.mStandaloneWavVoice, NULL, SCE_NGS_VOICE_INIT_BASE);
   BPE_VERIFY(returnCode == SCE_OK, false, "Error initializing voice before playback.");

   returnCode = sceNgsVoiceLockParams(gNgsState.mStandaloneWavVoice, SCE_NGS_VOICE_T1_PCM_PLAYER, 
      SCE_NGS_PLAYER_PARAMS_STRUCT_ID, &bufferInfo);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to lock voice parameters.");

   SceNgsPlayerParams* pPcmParams = (SceNgsPlayerParams*)bufferInfo.data;
   pPcmParams->fPlaybackScalar = 1.0f;
   pPcmParams->fPlaybackFrequency = frequency;
   pPcmParams->nLeadInSamples = 0;
   pPcmParams->nLimitNumberOfSamplesPlayed = 0;
   pPcmParams->nStartBuffer = 0;
   pPcmParams->nStartByte = 0;
   pPcmParams->nChannelMap[0] = SCE_NGS_PLAYER_LEFT_CHANNEL;
   pPcmParams->nChannelMap[1] = SCE_NGS_PLAYER_LEFT_CHANNEL;

   pPcmParams->buffs[0].pBuffer = waveformData;
   pPcmParams->buffs[0].nNumBytes = size;
   pPcmParams->buffs[0].nLoopCount = 0;
   pPcmParams->buffs[0].nNextBuff = SCE_NGS_PLAYER_NO_NEXT_BUFFER;

   returnCode = sceNgsVoiceUnlockParams(gNgsState.mStandaloneWavVoice, SCE_NGS_VOICE_T1_PCM_PLAYER);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to unlock voice parameters on SPU stream.");

   SceNgsPatchRouteInfo patchInfo;
   sceNgsPatchGetInfo(gNgsState.mStandAloneWavVoicePatchHandleDry, &patchInfo, NULL);

   patchInfo.vols.m[0][0] = gBP_WavFormVolume;
   patchInfo.vols.m[0][1] = gBP_WavFormVolume;

   returnCode = sceNgsVoicePatchSetVolumesMatrix(gNgsState.mStandAloneWavVoicePatchHandleDry, &patchInfo.vols);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to set voice volumes.");

   returnCode = sceNgsVoicePlay(gNgsState.mStandaloneWavVoice);
   BPE_VERIFY(returnCode == SCE_OK, false, "Unable to play wav stream.");

   return 0;
}

//----------------------------------------------------------------------------
void BP_StopWaveform(const unsigned int handle)
{
   // Was only used for the TGS demo...
   BPE_VERIFY(false, false, "Function should not have been called.");
}

//----------------------------------------------------------------------------
void BP_SetWaveformVolume(const unsigned int handle, const float volume)
{
   // Was only used for the TGS demo...
   BPE_VERIFY(false, false, "Function should not have been called.");
}
