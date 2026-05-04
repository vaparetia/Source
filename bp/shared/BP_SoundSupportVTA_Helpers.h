#pragma once
#ifndef __BP_SOUNDSUPPORTVTA_HELPERS__H
#define __BP_SOUNDSUPPORTVTA_HELPERS__H

#include <ngs.h>
#include <ngs/error.h>
#include <audioout.h>

#include "BP_SoundSupport.h"

extern int gBP_EnableAudioReverb;

#define SULPHA_ENABLED 0

namespace NVitaSound
{
   struct SSpuVoiceState;
   struct SReverbParams;
   class CAudioStream;

   // Size of the buffer used to store audio data streamed by the game
   int const kAT9StreamBufferSize = 512 * 1024;

   // Size and count of the number of buffers read by the audio playback system.

   // AS MCampbell - This value was set at 8192 for the longest time with no apparent problems. Recently there have been 
   // stream disk read errors attributed to the audio playback getting starved. I am unable to determine why these cases 
   // cause the problem but increasing the buffer size to 16384 fixes the problems since more audio data is initially 
   // queued up before playing the voice stream.
   // Addresses Bugs: MGSTWO-3240, MGSTWO-3239, MGSTWO-3227, MGSTWO-3226
   int const kAT9PlayBufferSize = 16384;

   int const kNumAT9PlayBuffers = 4;

   int const kNumSpuVoiceChannels = 2;
   int const kNumSpuVoices = 24;

   // Values taken from the NGS SDK sample.
   int const kNumNgsModules = 14;
   int const kNgsSysGranularity = 512;
   int const kNgsSysSampleRate = 48000;
   int const kAudioThreadStackSize = 32 * 1024;

   enum ESoundPlaybackState 
   {
      kDOSS_Closed,
      kDOSS_Pending,
      kDOSS_Queuing,
      kDOSS_Playing,
   };

   enum EAudioFormat
   {
      kAUDIO_FORMAT_UNKNOWN = -1, // Initialization value.
      kAUDIO_FORMAT_PCM = 0,
      kAUDIO_FORMAT_ATRAC9 = 9, // Set to 9 to match the format id of the data created by the stream tool.
   };

   //----------------------------------------------------------------------------
   struct SNgsState
   {
      SceNgsHSynSystem mNgsSysHandle;
      void* mpNgsSysMem;

      // All other voice racks connect to the master rack. The master rack generates the final PCM audio data sent to 
      // the output audio port.
      void* mpMasterRackMem;
      SceNgsHRack mMasterRack;
      SceNgsHVoice mMasterVoice;

      // Reverb is enabled by creating a reverb rack and reverb voice. Audio streams and/or sound effects that have the 
      // reverb effect go through this voice instead of directly through the master voice.
      void* mpReverbRackMem;
      SceNgsHRack mReverbRack;
      SceNgsHVoice mReverbVoice;
      SceNgsHPatch mReverbPatch; // Used to easily control the overall strength of the reverb effect.
      SReverbParams* mpCustomReverbParams;

      // Hardware port id in which output PCM audio constructed by NGS is sent.
      int mAudioHardwarePort;

      // Audio update thread. This thread pulls data from the NGS system and pushes it to the hardware audio port.
      SceUID mAudioOutThread;

      // Audio streams are used to play back direct sound tracks and BP_Audio sound streams. There can be at most
      // BP_DOSTREAM_COUNT audio streams playing at once. All audio streams are encoded in ATrac9 compressed format.
      // Note, each audio stream may have up to 3 tracks. Each track is a separate voice therefore each audio stream 
      // has it's own audio rack.
      CAudioStream* mAudioStreams[BP_DOSTREAM_COUNT];

      // SPU voices are used to play in game sound effects. This is one audio rack per voice channel. Each rack stores 
      // kNumSpuVoices.
      void* mpSpuVoiceRackMem[NVitaSound::kNumSpuVoiceChannels];
      SceNgsHRack mSpuVoiceRack[NVitaSound::kNumSpuVoiceChannels];
      SSpuVoiceState* mpSpuVoiceState[NVitaSound::kNumSpuVoiceChannels][NVitaSound::kNumSpuVoices];

      // A voice set aside for playing back a one off sound effect. (Used for the AP sensor in MGS2).
      void* mpStandaloneWavVoiceRackMem;
      SceNgsHRack mStandaloneWavVoiceRack;
      SceNgsHVoice mStandaloneWavVoice;
      SceNgsHPatch mStandAloneWavVoicePatchHandleDry; // Connects voice directly to master voice for dry signal
      SceNgsHPatch mStandAloneWavVoicePatchHandleWet; // Connects voice to the reverb voice for wet signal.
   };

   void init_audio();

   void shutdown_audio();

   void init_reverb();

   int audio_update_thread_main(SceSize args, void *argc);

   void game_audio_update();

   void ngs_system_error_callback(const SceNgsCallbackInfo *pCallbackInfo);

   void get_vag_endpoints(SSpuVoiceState* voiceState);

   void init_spu_voices();

   void init_standlone_voice();

   int get_spu_stream_byte_pos(SSpuVoiceState* const pSpuVoice);

   void apply_spu_voice_volume(SSpuVoiceState* const pSpuVoice, bool simulateEnvelopes);

   void apply_spu_voice_reverb(SSpuVoiceState* const pSpuVoice);

   void apply_standalone_voice_reverb();

   void apply_spu_pitch(SSpuVoiceState* const pSpuVoice);

   void key_on_spu_voice_stream(SSpuVoiceState* pSpuVoice);

   void apply_spu_voice_keyoff(SSpuVoiceState* pSpuVoice, float deltaTime);

   void apply_system_reverb();

   void get_reverb_params_MGS2(SceNgsBufferInfo* pReverb, float* pStrength);

   void get_reverb_params_MGS3(SceNgsBufferInfo* pReverb, float* pStrength);

   bool is_bgm_stream(int streamTop);

#if SULPHA_ENABLED == 1
   void start_sulpha_tracing();
   void update_sulpha();
#endif
}

#endif