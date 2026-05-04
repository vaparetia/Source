//----------------------------------------------------------------------------
// BP_SoundSupportX360.cpp
//----------------------------------------------------------------------------

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/StlExtras/BPEStlExtras.h"
#include "Engine/System/CStopWatch.h"
#include "Engine/Graphics/CColor.h"

#include "BP_SoundSupport.h"
#include "BP_SurroundSound.h"
#include "BP_ADSREnvelope.h"

#include "BP_EndianSupport.h"
#include "BP_FileSupport.h"
#include "libsd.h"

#include "BP_Debug.h"


#include "Engine/System/CSyncCriticalSection.h"

#include "BP_RendererDebug.h"

#include <XAudio2.h>
#include <audiodefs.h>
#include <xma2defs.h>
#include <xbox.h>
#include <xtl.h>

#include <xwmadecode.h>
#include <xaudio2fx.h>
#include <xmp.h>

using namespace XAUDIO2;

#include <BP_Memory.h>
#include <stdio.h>
#include <map>

extern void BP_SoundSupport_ApplyDirectOutputStreamsPaused_PlatformSpecific();

extern "C"
{
   extern int gBP_MTAChannel;
};

extern int gBP_Sound_CreatedVoices;
extern int gBP_Sound_CreatedMemStreams;
extern int gBP_Sound_CreatedLoopBuffers;

extern int gbSoundSupportSurroundSound;
extern int gSoundSupportSurroundSoundChannelCount;

extern int gReverbModePS2;
extern int gReverbDepthPS2;

extern unsigned int gBP_XMPBgmOverrideStreams;
extern unsigned int gBP_PlayBgmVoices;
extern int gDirectOutputStreamsPaused;

//----------------------------------------------------------------------------

//#define BP_DUMP_XAUDIO_WAVS

#define BP_DECODE_SPU_AFTER_ALL_TRANSFER
//#define BP_DYNAMIC_LOOP_BUFFERS
#define BP_USE_NEW_ADSR_ENVELOPES
#define BP_XAUDIO_USE_REVERB

// Size of a single vag packet in bytes.
const int BP_VAG_PACKET_SIZE = 16;

const int BP_NUM_VAG_PACKETS = BP_SPU_MEMORY_SIZE / BP_VAG_PACKET_SIZE;

// One vag packet will decompress into 28 mono 16 bit samples.
const int BP_PCM_SIZE_FROM_VAG_SIZE = 28 * sizeof(short);

const int MAX_DECODED_PCM_BUFFERS = 4;
const int MAX_DECODED_PCM_BUFFER_SIZE = 65536;
const int XWMA_NUM_ENCODED_PACKETS = 32;
const int MAX_XWMA_PACKET_DECODE_BUFFER_SIZE = 16384;

#define BP_NATIVE_SAMPLES_PER_SECOND 48000

static XAUDIO2FX_REVERB_I3DL2_PARAMETERS skXAudioReverbMode_OFF = XAUDIO2FX_I3DL2_PRESET_DEFAULT;

static XAUDIO2FX_REVERB_I3DL2_PARAMETERS skXAudioReverbMode_STUDIO_C =
{
   100,     //WetDryMix
   0,       //Room
   -500,    //RoomHF
   0.0f,    //RoomRolloffFactor
   3.92f,   //DecayTime
   0.70f,   //DecayHFRatio
   -1230,   //Reflections
   0.020f,  //ReflectionsDelay
   -2,      //Reverb
   0.029f,  //ReverbDelay
   100.0f,  //Diffusion
   100.0f,  //Density
   5000.0f  //HFReference
};

//MGS2 XBox apparently used a D3D "Concert Hall" setting
//This is based on XAUDIO2FX_I3DL2_PRESET_CONCERTHALL
static XAUDIO2FX_REVERB_I3DL2_PARAMETERS skXAudioReverbMode_HALL =
{
   100,     //WetDryMix
   0,       //Room
   -500,    //RoomHF
   0.0f,    //RoomRolloffFactor
   3.92f,   //DecayTime
   0.70f,   //DecayHFRatio
   -1230,   //Reflections
   0.020f,  //ReflectionsDelay
   -2,      //Reverb
   0.029f,  //ReverbDelay
   100.0f,  //Diffusion
   100.0f,  //Density
   5000.0f  //HFReference
};

static XAUDIO2FX_REVERB_I3DL2_PARAMETERS skXAudioReverbMode_SPACE = XAUDIO2FX_I3DL2_PRESET_MOUNTAINS;

static XAUDIO2FX_REVERB_I3DL2_PARAMETERS * skXAudioReverbModes[] =
{
   &skXAudioReverbMode_OFF,
   NULL,
   NULL,
   NULL,
   &skXAudioReverbMode_STUDIO_C,
   &skXAudioReverbMode_HALL,
   &skXAudioReverbMode_SPACE,
   NULL,
   NULL,
   NULL,
};

static XAUDIO2FX_REVERB_I3DL2_PARAMETERS sCurrXAudioReverbMode = XAUDIO2FX_I3DL2_PRESET_DEFAULT;

//----------------------------------------------------------------------------

// Vag (ADPCM) packets are real-time decompressed into this buffer. All vag addresses are then translated 
// to an offset in this buffer when playback occurs.
unsigned char gDecompressedVagBuffer[BP_NUM_VAG_PACKETS * BP_PCM_SIZE_FROM_VAG_SIZE];
extern unsigned char gBP_SpuMemory[];

#if MGS_VERSION==3
unsigned char     gMTAStreamPlaybackBuffer[BP_DOSTREAM_TRACK_COUNT*BP_DOSTREAM_BUFFER_SIZE];
#endif

//----------------------------------------------------------------------------

namespace
{
   enum ESend
   {
      kSend_Master,
      kSend_Submix,
      kSend_Count
   };

   enum EBgmMusicType
   {
      kBgmMusicType_Not,      //No BGM in this voice; should not be muted by XMP.
      kBgmMusicType_BgmOnly,  //Only BGM in this voice; should be muted by XMP.
      kBgmMusicType_BgmMixed  //Mixed BGM / other in this voice; should mute XMP.
   };

   const int skDefaultReverbMasterVolume = 1.f;

   //Persistent state necessary to detect and properly persist the ADPCM history
   //coefficients across multiple small transfers / decodes of VAG data into the buffer.
   struct SADPCMStreamBufferState
   {
      int            mSpuAddr;
      int            mSpuSize;
      double         mS1;
      double         mS2;
   };

   SADPCMStreamBufferState sADPCMStreamBufferState[2][2];

   enum EDirectOutputStreamState
   {
      kDOSS_Closed,
      kDOSS_Pending,
      kDOSS_Playing
   };

   enum EVoiceBufferType
   {
      kVBT_FirstBuffer,
      kVBT_MemoryBuffer,
   };

   struct SSpuVoiceState
   {
      int         mCore;
      int         mVoice;

      unsigned int mSSA;
      unsigned int mLSAX;
      unsigned int mBlockBegin;
      unsigned int mBlockEnd;
      int   mXAudioPitch;
      float mXAudioVolumeL;
      float mXAudioVolumeR;

      //ADSR envelope support
      SADSREnvelopeState   mADSREnvelope;

      //Surround sound support
      int mSurroundType;
      float mPan3d;
      float mVol3d;
      float mXAudioSurroundVolumes[BP_SPEAKER_COUNT];

      //If not NULL, this replaces SSA as the source of the waveform playback.
      const unsigned char* mpMemoryStream;
      char* mpDecompressedMemStream;
      int mMemoryStreamSize;
      float mMemoryStreamMuteVolume;

      int                  mLastSampledVoicePos;
      EVoiceBufferType mCurrPlayingBufferType;

      IXAudio2SourceVoice* mpAudioVoice;
      bool mbBlockLoop;

      bool                 mbReverbEnabledL;
      bool                 mbReverbEnabledR;

      //Reusing XAudio2 voices is unreliable in terms of resetting their state,
      //and there are playback issues to do with PS2 code keying off then immediately
      //keying on a voice with new settings, so here's a spare voice that can be used to
      //allow the previously playing use of a voice to overlap the new one at key on,
      //and be destroyed before reuse.  Safest way to avoid various problems on X360.
      //In theory it can be expensive to destroy these (it's synchronous wrt X360 audio thread), but in tests it's negligible.
      IXAudio2SourceVoice* mpSpareAudioVoice;
      //Quickly fade this voice out over 1/10s.
      float                mSpareVoiceEnvelope;
      float                mSpareVoiceReleaseSpeed;
   };

#ifdef BP_DYNAMIC_LOOP_BUFFERS
   //To reliably decode looped ADPCM waveforms without pops, we need space for an extra iteration of the loop
   //so that the decode history from the end of the loop can apply to the beginning of the final loop range.
   //In other words, the first few samples of decoded data at the beginning of the loop region when coming from the
   //end of the non-looped region WILL NOT necessarily match the first few samples of decoded data in the loop region
   //when coming from the end of the loop.
   typedef std::vector< char > TSpuLoopBuffer;
   typedef std::map< unsigned int, TSpuLoopBuffer > TSpuLoopBufferMap;

   TSpuLoopBufferMap gSpuLoopBufferMap;
#endif
}

//----------------------------------------------------------------------------
static void get_vag_endpoints(SSpuVoiceState& voiceState)
{
   voiceState.mBlockBegin = 0;
   voiceState.mBlockEnd = 0;
   voiceState.mbBlockLoop = false;

   int begin = voiceState.mpMemoryStream ? 0 : voiceState.mSSA;
   int end = voiceState.mpMemoryStream ? voiceState.mMemoryStreamSize : BP_SPU_MEMORY_SIZE;
   for( int i=begin; i < end; i+=16 )
   {
      const unsigned char * vagPacket = voiceState.mpMemoryStream ? &voiceState.mpMemoryStream[i] : &gBP_SpuMemory[i];
      const unsigned char flag = vagPacket[1];
      if( flag & VAG_LOOPBLOCK )
      {
         voiceState.mbBlockLoop = true;
      }
      if( flag & VAG_LOOPSTART )
      {
         voiceState.mBlockBegin = i;
      }
      if( flag & VAG_LOOPEND )
      {
         voiceState.mBlockEnd = i+16;
         break;
      }
   }
   //LSAX register was manually set.  Here's our block begin.
   if( voiceState.mLSAX )
   {
      voiceState.mBlockBegin = voiceState.mLSAX;
      voiceState.mbBlockLoop = true;
      voiceState.mLSAX = 0;
   }

   if( !voiceState.mBlockEnd )
      printf("Warning: could not get vag endpoint!\n");
}

static EBgmMusicType bp_get_spu_voice_bgm_type( const int core, const int voice )
{
#if MGS_VERSION==2
   //First 32 voices are reserved for midi music (see SNG_TRACK_NUM)
   int spu_num = core*24 + voice;
   if( spu_num < 32 )
   {
      return kBgmMusicType_BgmOnly;
   }
#endif
   //No other cases where spu hardware voices are music
   return kBgmMusicType_Not;
}

static EBgmMusicType bp_get_stream_bgm_type_by_name( const char * const streamFullName )
{
#if MGS_VERSION == 2
#include "BP_BgmStreamListsMGS2.h"
#elif MGS_VERSION==3
#include "BP_BgmStreamListsMGS3.h"
#endif

   char streamName[FILENAME_MAX];
   strcpy( streamName, strrchr( streamFullName, '/' ) + 1 );
   *(strstr( streamName, ".sdt" )) = 0;
   for( int i=0; skBgmOnlyStreamNames[i] != NULL; ++i )
   {
      if( !strcmp( streamName, skBgmOnlyStreamNames[i] ) )
      {
         return kBgmMusicType_BgmOnly;
      }
   }

   for( int i=0; skNoBgmStreamNames[i] != NULL; ++i )
   {
      if( !strcmp( streamName, skNoBgmStreamNames[i] ) )
      {
         return kBgmMusicType_Not;
      }
   }

   //mixed by default.
   return kBgmMusicType_BgmMixed;
}

static EBgmMusicType bp_get_stream_bgm_type( const int top, const int channelCount )
{
   //Simple rule to start with; we can add special cases later if there turn out to be problems with this.
   switch( channelCount )
   {
   default:
      BP_BREAK;
   case 0:
      //Stream has never been played.
   case 1:
      //If a stream has only one channel, there's no way it's music.
      //Most likely vox.
      return kBgmMusicType_Not;
   case 2:
   case 6:
      if( const char * const streamName = BP_FindStreamName( top ) )
      {
         if( strstr( streamName, "/bgm/" ) || strstr( streamName, "/bgm_2/" ) )
         {
            printf("BGM stream music only: %s\n", streamName );
            //All BGM / BGM2 streams in MGS3 are considered to be music only.
            return kBgmMusicType_BgmOnly;
         }

         //Any other special cases are handed by manually-created lists.
         EBgmMusicType bgmType = bp_get_stream_bgm_type_by_name( streamName );
         if( bgmType != kBgmMusicType_BgmMixed )
         {
            printf("stream type special case (%d): %s\n", bgmType, streamName );
         }
         return bgmType;
      }
      //Default.
      return kBgmMusicType_BgmMixed;
   }
}

static void bp_start_stream_xmp_hook( const int streamChannel, const EBgmMusicType type )
{
   if( type == kBgmMusicType_BgmMixed )
   {
      //The only case where we should mute the XMP music.
      int newOverrides = gBP_XMPBgmOverrideStreams | (1 << streamChannel);
      if( newOverrides != gBP_XMPBgmOverrideStreams )
      {
         if( !gBP_XMPBgmOverrideStreams )
         {
            printf("bp_start_stream_xmp_hook: XMPOverrideBackgroundMusic!\n");
            //Started playing first stream that should override background music.
            XMPOverrideBackgroundMusic();
         }
         gBP_XMPBgmOverrideStreams = newOverrides;
      }
   }
}

static void bp_stop_stream_xmp_hook( const int streamChannel, const EBgmMusicType type )
{
   if( type == kBgmMusicType_BgmMixed )
   {
      //The only case where we should mute the XMP music.
      int newOverrides = gBP_XMPBgmOverrideStreams & (~(1 << streamChannel));
      if( newOverrides != gBP_XMPBgmOverrideStreams )
      {
         if( !newOverrides )
         {
            //This was the last stream that should override background music.
            printf("bp_stop_stream_xmp_hook: XMPRestoreBackgroundMusic!\n");
            XMPRestoreBackgroundMusic();
         }
         gBP_XMPBgmOverrideStreams = newOverrides;
      }
   }
}

static void bp_set_bgm_mute_volume( IXAudio2SourceVoice* const pVoice, const EBgmMusicType type, const float masterVolume )
{
   if( pVoice )
   {
      if( type == kBgmMusicType_BgmOnly )
      {
         const float bgmVolume = gBP_PlayBgmVoices ? 1.f : 0.f;
         pVoice->SetVolume(bgmVolume * masterVolume);
      }
   }
}

//----------------------------------------------------------------------------

class AudioDriver
{
public:
   class DirectSoundVoiceCallback;
   class SpuVoiceCallback;

   // This structure stores decoded xWMA audio data. When a buffer is full it is submitted to the XAudio2 voice.
   struct PcmBuffer 
   {
      int usedBytes;
      BYTE data[MAX_DECODED_PCM_BUFFER_SIZE];
   };

   struct XwmPacket 
   {
      // Size of one individual stream packet (includes extra padding at end to align to 16 byte size)
      int packetSize; 

      // Amount of valid data without padding in buffer
      int numEncodedBytes;
      
      // If this is true, then there are no more packets after this in the stream.
      bool lastPacket;
      
      // Data for buffer
      BYTE buffer[MAX_XWMA_PACKET_DECODE_BUFFER_SIZE];
   };

   struct AudioStreamChannel 
   {
      DirectSoundVoiceCallback* m_directSoundCallback[BP_DOSTREAM_TRACK_COUNT];
      IXAudio2SourceVoice* m_directSoundTracks[BP_DOSTREAM_TRACK_COUNT];

      // Used for decoding xWMA streams.
      XWMADECODE* m_pDecoder;
      
      XwmPacket m_xwmPackets[XWMA_NUM_ENCODED_PACKETS];
      
      // decoder is reading from this packet index
      int m_readPacket;
      // sound stream is putting data at this packet
      int m_writePacket;

      // Used for shared data protection on decoded xWMA streams and for interaction between voice callbacks.
      CSyncCriticalSection m_soundSync;
      HANDLE m_soundDestroyedEvent;
      HANDLE m_decodeThread;
      HANDLE m_decodeReadyEvent;

      // Temporary buffer used by the xWMA decoder.
      BYTE m_xwmDecodeBuffer[MAX_XWMA_PACKET_DECODE_BUFFER_SIZE];

      // Used for keeping the main thread and the decode thread from getting to far ahead or behind.

      // Current stream position (total stream size includes padding), only used to tell game about where it currently is.
      // Not used internally.
      int m_xwmReadPos;
      
      // Amount of valid data in ALL packet-buffers without padding.
      int m_numXWMBytesAvail;

      //Number of bytes of xwm waveform data read, not including 16-byte padding on incoming data
      //Will go from 0...m_totalStreamSize over the duration of the stream.
      int m_numXWMBytesReadNoPadding;

      // Array of buffers which are used in a round robin fashion for storing decoded xWMA data
      PcmBuffer m_pcmBuffers[MAX_DECODED_PCM_BUFFERS];
      int m_currPcmBuffer;        

      int m_currTrackCount;
      int m_currBufferSize;
      int m_currFrequency;
      int m_avgBytesPerSecond;
      int m_format;
      int m_streamChannel;
      int m_numAudioChannels;
      int m_streamType;
      int m_streamTop;
      EBgmMusicType m_bgmType;

      // Total XWMA data size without padding (grabbed from header loaded at beginning)
      int m_totalStreamSize;

      // Current stream position (without packet header, with padding)
      int m_bytesNotified;

      int /*EDirectOutputStreamState*/ m_currSoundState;
      int m_isStreamDonePlaying;
      bool m_reachedLastEncodedPacket;
      bool m_didSubmitLastBuffer;

      float m_stereoVolumeMatrix[2*2];  //up to 2 channels; Stereo L/R (need to cache this for reverb purposes)
      bool m_bReverbEnabled;
      bool m_bPaused;
   };

   class DirectSoundVoiceCallback : public IXAudio2VoiceCallback 
   {
   public:
      DirectSoundVoiceCallback(AudioDriver* pAudioDriver, AudioStreamChannel* pAudioStream, int track) : 
         m_pAudioDriver(pAudioDriver), m_pAudioStream(pAudioStream), m_track(track) 
         {
            m_hBufferEndEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
         }

         virtual ~DirectSoundVoiceCallback()
         {
            CloseHandle(m_hBufferEndEvent);
         }

         void OnStreamEnd() 
         {
            if (m_pAudioStream->m_format == BP_DOSTREAM_FORMAT_PCM)
            {
               if (m_pAudioDriver->GetSoundState(m_pAudioStream->m_streamChannel) == kDOSS_Playing)
                  m_pAudioDriver->QueueDirectSound(m_pAudioStream->m_streamChannel, m_track);
            }
            else 
            {
               m_pAudioStream->m_isStreamDonePlaying = 1;
            }
         }

         void OnBufferEnd(void * pBufferContext) 
         { 
            SetEvent(m_hBufferEndEvent); 
         }

         HANDLE GetBufferEndEventHandle() const { return m_hBufferEndEvent; }

         void OnVoiceProcessingPassEnd() {}
         void OnVoiceProcessingPassStart(UINT32 SamplesRequired) { }
         void OnBufferStart(void * pBufferContext) {}
         void OnLoopEnd(void * pBufferContext) { }
         void OnVoiceError(void * pBufferContext, HRESULT Error) { }

   private:
      AudioDriver* m_pAudioDriver;
      AudioStreamChannel* m_pAudioStream;
      HANDLE m_hBufferEndEvent;
      int m_streamChannel;
      int m_track;
   };

   AudioDriver() : m_pXAudio(NULL), m_pMasterVoice(NULL) 
   {
#ifndef GOLD_VERSION
      mbChangedSimulatedFrequency = false;
#endif

      for (int i = 0; i < BP_DOSTREAM_COUNT; i++)
         m_audioStreams[i] = new AudioStreamChannel();
   }

   void Initialize();
   void Shutdown() {}
   bool IsRunning() { return true; }

   void InitSound(DirectOutputStreamInitData const* pInitData);
   void StartSound(int channel);
   void StopSound(int channel);
   void PauseSound(int channel, bool isPaused);
   void ApplyPause( int channel );

   void SetTrackVolume_Direct(int channel, int trackNum, float volume);
   void SetTrackVolume_Voice(const int channel, const float fStereoVols[2][2], const float pan3d, const float vol3d, BP_SURROUND_VOICE_TYPE surroundType);
   void ApplyReverbEnabled_StreamVoice(const int channel);
   int GetSoundPos(int channel);
   void SetMasterVolume(float volume);

   void SetSoundState(int channel, EDirectOutputStreamState newState) 
   { 
      BPE_ASSERT_NO_MSG(channel < BP_DOSTREAM_COUNT);
      m_audioStreams[channel]->m_currSoundState = newState;
   }

   EDirectOutputStreamState GetSoundState(int channel)
   { 
      BPE_ASSERT_NO_MSG(channel < BP_DOSTREAM_COUNT);
      return (EDirectOutputStreamState)m_audioStreams[channel]->m_currSoundState;
   }

   // If track is -1 queue on all tracks for the channel.
   void QueueDirectSound(int channel, int track);

   double GetSoundPlayTime(int channel);

   int IsSoundDone(int channel);

   void NotifySoundBytesAvailable(int channel, void* pBuffer, int sizeBytes);

   void SetVoiceParam(int param, int core, int voice, unsigned short value);
   unsigned short GetVoiceParam(int param, int core, int voice);

   void SetVoiceSourceAddr(int type, int core, int voice, int addr);

   IXAudio2SourceVoice* CreateSourceVoicePCM( int channelCount, int samplesPerSecond, IXAudio2VoiceCallback *pCallback, bool bSendSubmix, EBgmMusicType bgmType );

   void StartVoice(int core, int voice);
   void StopVoice(SSpuVoiceState & voiceState);
   void StopVoiceForReuse( SSpuVoiceState & voiceState );
   void DestroyVoice( SSpuVoiceState & voiceState );
   unsigned int GetVoicePos(int core, int voice);
   void ApplyVoiceReverb( SSpuVoiceState & voiceState );

   void ApplyStereoVolume( SSpuVoiceState & voiceState );
   void GetStereoSpread8Volume( float * const outSpeakerVols, const float stereoVolL, const float stereoVolR );
   void SurroundSoundSetVoiceType( const int core, const int voice, BP_SURROUND_VOICE_TYPE voiceType );
   void SurroundSoundSetPanVol( const int core, const int voice, float pan, float vol);
   void ApplyVolume( SSpuVoiceState & voiceState );
   void ApplyKeyOffVolume( SSpuVoiceState & voiceState, const float dt );
   void ApplyPitch( SSpuVoiceState & voiceState );

   void QueueAudioVoice(SSpuVoiceState& voiceState);

   void PlayMemoryStream(int core, int voice, const void * const playbackBuffer, int playbackBufferSize);
   void MuteMemoryStream(int core, int voice, int muted);

   void ApplyReverbSetting();

public:
   AudioStreamChannel* m_audioStreams[BP_DOSTREAM_COUNT];
   
   SSpuVoiceState m_spuVoiceState[2][24];

   IXAudio2* m_pXAudio;
   IXAudio2MasteringVoice* m_pMasterVoice;   
   IXAudio2SubmixVoice*    m_pSubmixVoice;
   IUnknown*               m_pReverbEffect;
   float                   m_ReverbMasterVolume;

#ifndef GOLD_VERSION
   bool mbChangedSimulatedFrequency;
#endif
};

//----------------------------------------------------------------------------

static HRESULT GetXWMAData(void* pUserData, XWMADECODE_INPUT_BUFFER_INFO* pInfo)
{
   AudioDriver::AudioStreamChannel* pAudioStream = (AudioDriver::AudioStreamChannel*)pUserData;
   
   if (pAudioStream->m_reachedLastEncodedPacket)
   {
      return S_FALSE;
   }

   CSyncCriticalSectionLocker lock(pAudioStream->m_soundSync);

   AudioDriver::XwmPacket* pReadPacket = &pAudioStream->m_xwmPackets[pAudioStream->m_readPacket];

   int const compressedPacketSize = pReadPacket->numEncodedBytes;

   if( compressedPacketSize == 0 )
   {
      BPE_VERIFYA(false, "SOUND: compressedPacketSize == 0: Should not happen.");

      return S_FALSE;
   }

   BPE_VERIFY(compressedPacketSize <= pAudioStream->m_pDecoder->XWMAPacketByteSize, false, "Trying to feed too much data to decoder.");

   pInfo->pInputData = pReadPacket->buffer;
   pInfo->InputDataBytes = compressedPacketSize;
   pInfo->NoMoreInput = pReadPacket->lastPacket;

   // Mark packet as unused.
   pReadPacket->numEncodedBytes = 0;

   pAudioStream->m_xwmReadPos += pReadPacket->packetSize; // increase read pos by size of packet including padding
   pAudioStream->m_numXWMBytesAvail -= compressedPacketSize;   

   pAudioStream->m_reachedLastEncodedPacket = pReadPacket->lastPacket;

   // We're done consuming this read packet, advance to next.
   pAudioStream->m_readPacket = (pAudioStream->m_readPacket + 1) % XWMA_NUM_ENCODED_PACKETS;   
   
   return S_OK;
}

//----------------------------------------------------------------------------

static void WaitForFreeBuffer(AudioDriver::AudioStreamChannel* pAudioStream)
{  
   for(;;)
   {
      XAUDIO2_VOICE_STATE audioState;
      {
         CSyncCriticalSectionLocker lock(pAudioStream->m_soundSync);
         pAudioStream->m_directSoundTracks[0]->GetState(&audioState);
      }

      //printf("Wait For Free Buffer: Queued: %d\n", audioState.BuffersQueued);

      // Wait for at least one buffer to become available.
      if( audioState.BuffersQueued < MAX_DECODED_PCM_BUFFERS )
         break;

      if (pAudioStream->m_currSoundState != kDOSS_Playing)
         break;

      Sleep(1);
   }
}

//----------------------------------------------------------------------------

static int RunDecoder(AudioDriver::AudioStreamChannel* pAudioStream)
{
   if( pAudioStream->m_pDecoder->DecoderState == XWMADECODE_STATE_DONE )
      return 0;

   if( !pAudioStream->m_reachedLastEncodedPacket && pAudioStream->m_numXWMBytesAvail == 0 )
      return 0;

   HRESULT hResult = XWMADecodeProcessData(pAudioStream->m_pDecoder);

   int availPCMBytes = pAudioStream->m_pDecoder->FramesReady * pAudioStream->m_pDecoder->OutputChannels * pAudioStream->m_pDecoder->SampleSize;
   
   //printf("XWMADecodeProcessData: Res: %d avail bytes: %d\n", hResult, availPCMBytes);

   // If the current pcm buffer is full, submit it and use the next.
   AudioDriver::PcmBuffer* pPCMBuffer = &pAudioStream->m_pcmBuffers[pAudioStream->m_currPcmBuffer];

   // If the current buffer does not have enough space to receive the new data, queue it up for playback.
   if ((pPCMBuffer->usedBytes + availPCMBytes) > MAX_DECODED_PCM_BUFFER_SIZE)
   {
      XAUDIO2_BUFFER xAudioBuffer = {0};

      xAudioBuffer.pAudioData = &pPCMBuffer->data[0];
      xAudioBuffer.Flags = 0;
      xAudioBuffer.AudioBytes = pPCMBuffer->usedBytes;

      pAudioStream->m_directSoundTracks[0]->SubmitSourceBuffer(&xAudioBuffer);

      pPCMBuffer->usedBytes = 0;

      pAudioStream->m_currPcmBuffer = (pAudioStream->m_currPcmBuffer + 1) % MAX_DECODED_PCM_BUFFERS;

      // Stall for a buffer to become available.
      WaitForFreeBuffer(pAudioStream);

      // Set up the next buffer and reset it.
      pPCMBuffer = &pAudioStream->m_pcmBuffers[pAudioStream->m_currPcmBuffer];
      memset(&pPCMBuffer->data[0], 0, MAX_DECODED_PCM_BUFFER_SIZE);
   }

   // Copy newly decoded pcm data into the current pcm buffer.
   memcpy(&pPCMBuffer->data[pPCMBuffer->usedBytes], pAudioStream->m_pDecoder->pDecodedData, availPCMBytes);
   pPCMBuffer->usedBytes += availPCMBytes;

   return availPCMBytes;
}

//----------------------------------------------------------------------------

static DWORD XWMADecodeThreadMain(void* pThreadUserData)
{
   AudioDriver::AudioStreamChannel* pAudioStream = (AudioDriver::AudioStreamChannel*)pThreadUserData;
   HRESULT hr;
   bool doneWithStream = false;

   for (;;)
   {
      WaitForSingleObject(pAudioStream->m_decodeReadyEvent, INFINITE);

      BPE_ASSERT_NO_MSG(pAudioStream->m_currTrackCount == 1);
      
      // We now have an audio stream ready for decoding. Continuously decode until the stream is over or told to 
      // stop.

      for (;;)
      {
         EDirectOutputStreamState streamState = (EDirectOutputStreamState)pAudioStream->m_currSoundState;
         
         if (streamState == kDOSS_Playing)
         {
            IXAudio2SourceVoice* pSourceVoice = pAudioStream->m_directSoundTracks[0];
            BPE_ASSERT_NO_MSG(pSourceVoice != NULL);

            RunDecoder(pAudioStream);

            // If we have reached the last packet submitted marking the end of the audio stream.
            if (pAudioStream->m_reachedLastEncodedPacket)
            {
               // Keep decoding until the decoder tells us it's done.
               while (pAudioStream->m_pDecoder->DecoderState != XWMADECODE_STATE_DONE)
               {
                  RunDecoder(pAudioStream);
               }

               if( !pAudioStream->m_didSubmitLastBuffer )
               {
                  printf("SUBMITTING LAST BUFFER!\n");

                  pAudioStream->m_didSubmitLastBuffer = true;

                  AudioDriver::PcmBuffer* pPCMBuffer = &pAudioStream->m_pcmBuffers[pAudioStream->m_currPcmBuffer];

                  XAUDIO2_BUFFER xAudioBuffer = {0};
                  xAudioBuffer.pAudioData = &pPCMBuffer->data[0];
                  xAudioBuffer.Flags = XAUDIO2_END_OF_STREAM;
                  xAudioBuffer.AudioBytes = pPCMBuffer->usedBytes;

                  pSourceVoice->SubmitSourceBuffer(&xAudioBuffer);
               }
            }         
         }
         else if (streamState == kDOSS_Closed)
         {
            printf("STOPPING DECODING SOUND!!!\n");

            CSyncCriticalSectionLocker lock(pAudioStream->m_soundSync);

            // Clean up the audio here since we know we aren't in the middle of decoding or submitting additional audio.
            if( pAudioStream->m_pDecoder != NULL )
            {
               XWMADecodeDestroy(pAudioStream->m_pDecoder);
               pAudioStream->m_pDecoder = NULL;
            }

            if( pAudioStream->m_directSoundTracks[0] != NULL )
            {
               pAudioStream->m_directSoundTracks[0]->DestroyVoice();
               pAudioStream->m_directSoundTracks[0] = NULL;
            }

            ResetEvent(pAudioStream->m_decodeReadyEvent);

            // Finally exit the loop and wait for another sound source to become available.
            SetEvent(pAudioStream->m_soundDestroyedEvent);

            break;
         }
      }
   }

   return 0;
}

//----------------------------------------------------------------------------
void AudioDriver::Initialize()
{
   for (int i = 0; i < BP_DOSTREAM_COUNT; i++)
   {
      m_audioStreams[i] = new AudioStreamChannel();

      memset(&m_audioStreams[i]->m_directSoundTracks[0], 0, sizeof(IXAudio2SourceVoice*) * BP_DOSTREAM_TRACK_COUNT);     
      
      m_audioStreams[i]->m_pDecoder = NULL;

      m_audioStreams[i]->m_currBufferSize = 0;
      m_audioStreams[i]->m_currFrequency = 0;
      m_audioStreams[i]->m_currTrackCount = 0;
      m_audioStreams[i]->m_currSoundState = kDOSS_Closed;
      m_audioStreams[i]->m_streamChannel = i;
      m_audioStreams[i]->m_numAudioChannels = 0;

      memset(&m_audioStreams[i]->m_xwmDecodeBuffer[0], 0, MAX_DECODED_PCM_BUFFER_SIZE);
      m_audioStreams[i]->m_xwmReadPos = 0;
      m_audioStreams[i]->m_readPacket = 0;
      m_audioStreams[i]->m_writePacket = 0;
      m_audioStreams[i]->m_numXWMBytesAvail = 0;
      m_audioStreams[i]->m_bytesNotified = 0;
      m_audioStreams[i]->m_numXWMBytesReadNoPadding = 0;
      m_audioStreams[i]->m_soundDestroyedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
      m_audioStreams[i]->m_decodeReadyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
      m_audioStreams[i]->m_isStreamDonePlaying = 0;
      m_audioStreams[i]->m_reachedLastEncodedPacket = false;
      m_audioStreams[i]->m_didSubmitLastBuffer = false;
      memset( m_audioStreams[i]->m_stereoVolumeMatrix, 0, sizeof( m_audioStreams[i]->m_stereoVolumeMatrix ) );
      m_audioStreams[i]->m_bReverbEnabled = false;
      m_audioStreams[i]->m_bPaused = false;

      DWORD threadID;
      m_audioStreams[i]->m_decodeThread = CreateThread(NULL, 0, &XWMADecodeThreadMain, m_audioStreams[i], 
         CREATE_SUSPENDED, &threadID);

      // Put each decode thread on the same hardware thread as all three of them being active at once doesn't happen 
      // all that often.
      XSetThreadProcessor(m_audioStreams[i]->m_decodeThread, 5);

      // Resume the thread so it is ready as soon as we have audio data to decode. Note, however, it will be put to sleep
      // immediately since we use an event to signal when there is data available.
      ResumeThread(m_audioStreams[i]->m_decodeThread);

      m_audioStreams[i]->m_currPcmBuffer = 0;
   }

   memset(&m_spuVoiceState[0][0], 0, sizeof(m_spuVoiceState));   

   HRESULT hr;
   if (FAILED(hr = XAudio2Create( &m_pXAudio, 0, XAUDIO2_DEFAULT_PROCESSOR)))
   {
      printf("Failed to initialize XAudio2. Error code: %d", hr);
      BP_BREAK;
   }


   XAUDIOSPEAKERCONFIG speakerConfig;
   if( FAILED(hr=XGetSpeakerConfig( &speakerConfig ) ))
   {
      printf("Failed to get speaker configuration. Error code: %d", hr);
      BP_BREAK;
   }

   if( speakerConfig & XAUDIOSPEAKERCONFIG_DIGITAL_DOLBYDIGITAL )
   {
      //5.1 support
      gSoundSupportSurroundSoundChannelCount = 6;
      gbSoundSupportSurroundSound = 1;
   }
   else
   {
      //Stereo
      gSoundSupportSurroundSoundChannelCount = 2;
      gbSoundSupportSurroundSound = 0;
   }

   printf("AudioDriver::Initialize(): output channels: %d\n", gSoundSupportSurroundSoundChannelCount );
   //Mastering voice
   if (FAILED(hr = m_pXAudio->CreateMasteringVoice( &m_pMasterVoice, gSoundSupportSurroundSoundChannelCount, 
      XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, NULL )))
   {
      printf("Failed to create master voice. Error code: %d", hr);
      BP_BREAK;
   }

   m_ReverbMasterVolume = skDefaultReverbMasterVolume;
#ifdef BP_XAUDIO_USE_REVERB
   //Reverb effect
   if( FAILED( hr = XAudio2CreateReverb( &m_pReverbEffect, 0 ) ) )
   {
      printf("Failed to create reverb effect. Error code: %d", hr);
      BP_BREAK;
   }

   //Effect chain
   XAUDIO2_EFFECT_DESCRIPTOR effects[] =
   {
      { m_pReverbEffect, TRUE, gSoundSupportSurroundSoundChannelCount }
   };
   XAUDIO2_EFFECT_CHAIN effectChain = { BPE_ARRAY_SIZE(effects), effects };

   //Submix voice
   if( FAILED( hr = m_pXAudio->CreateSubmixVoice( &m_pSubmixVoice, 2,
      BP_NATIVE_SAMPLES_PER_SECOND, 0, 0,
      NULL, &effectChain ) ) )
   {
      printf("Failed to create submix voice. Error code: %d", hr);
      BP_BREAK;
   }
#else
   m_pReverbEffect = NULL;
   m_pSubmixVoice = NULL;
#endif

   ApplyReverbSetting();

   for (int currStream = 0; currStream < BP_DOSTREAM_COUNT; currStream++)
   {
      for (int currTrack = 0; currTrack < BP_DOSTREAM_TRACK_COUNT; currTrack++)
      {
         m_audioStreams[currStream]->m_directSoundCallback[currTrack] = new DirectSoundVoiceCallback(
            this, m_audioStreams[currStream], currTrack);
         memset( m_audioStreams[currStream]->m_stereoVolumeMatrix, 0, sizeof( m_audioStreams[currStream]->m_stereoVolumeMatrix ) );
      }
   }

   //Create an XAudio2 voice for each PS2 SPU hardware voice.
   for( int core=0; core < 2; ++core )
   {
      for( int voice=0; voice < 24; ++voice )
      {
         SSpuVoiceState & voiceState = m_spuVoiceState[core][voice];
         memset( &voiceState, 0, sizeof( voiceState ) );

         voiceState.mCore = core;
         voiceState.mVoice = voice;

         IXAudio2VoiceCallback *pCallback = NULL;
         bool bSendSubmix = true;
         EBgmMusicType bgmType = bp_get_spu_voice_bgm_type( voiceState.mCore, voiceState.mVoice );
         voiceState.mpAudioVoice = CreateSourceVoicePCM( 1, BP_NATIVE_SAMPLES_PER_SECOND, pCallback, bSendSubmix, bgmType );
         ++gBP_Sound_CreatedVoices;
         voiceState.mpSpareAudioVoice = NULL;
      }
   }
}

//----------------------------------------------------------------------------
void AudioDriver::SetMasterVolume(float volume)
{
   if (!m_pMasterVoice)
      return;

   m_pMasterVoice->SetVolume(volume);
}

//----------------------------------------------------------------------------
void AudioDriver::InitSound(DirectOutputStreamInitData const* pInitData)
{
   BPE_ASSERT_NO_MSG(pInitData->channel < BP_DOSTREAM_COUNT);

   printf("STOPPING SOUND BEFORE INITING SOUND\n");

   // Make sure to stop any sounds playing on the requested channel before creating a new one.
   StopSound(pInitData->channel);

   printf("INITING SOUND ON CHANNEL: %d\n", pInitData->channel);

   AudioStreamChannel* pAudioStream = m_audioStreams[pInitData->channel];

   pAudioStream->m_format = pInitData->format;
   pAudioStream->m_streamType = pInitData->streamType;
   pAudioStream->m_streamTop = pInitData->streamTop;
   BPE_ASSERT_NO_MSG(pInitData->format == BP_DOSTREAM_FORMAT_PCM || 
      pInitData->format == WAVE_FORMAT_WMAUDIO2 || pInitData->format == WAVE_FORMAT_WMAUDIO3);
 
   pAudioStream->m_currTrackCount = pInitData->trackCount;
   pAudioStream->m_currBufferSize = pInitData->playbackBufferSize;
   pAudioStream->m_currFrequency = pInitData->freq;
   pAudioStream->m_avgBytesPerSecond = pInitData->avgBytesPerSec;
   pAudioStream->m_totalStreamSize = pInitData->totalStreamSize;
   pAudioStream->m_numAudioChannels = pInitData->channelCount;
   pAudioStream->m_xwmReadPos = 0;
   pAudioStream->m_readPacket = 0;
   pAudioStream->m_writePacket = 0;
   pAudioStream->m_currPcmBuffer = 0;
   pAudioStream->m_numXWMBytesAvail = 0;
   pAudioStream->m_bytesNotified = 0;
   pAudioStream->m_numXWMBytesReadNoPadding = 0;
   pAudioStream->m_isStreamDonePlaying = 0;
   pAudioStream->m_reachedLastEncodedPacket = false;
   pAudioStream->m_didSubmitLastBuffer = false;
   pAudioStream->m_bPaused = false;
   memset(&pAudioStream->m_pcmBuffers[0], 0, sizeof(PcmBuffer) * MAX_DECODED_PCM_BUFFERS);
   memset(&pAudioStream->m_xwmPackets[0], 0, sizeof(XwmPacket) * XWMA_NUM_ENCODED_PACKETS);
   memset(&pAudioStream->m_xwmDecodeBuffer[0], 0, MAX_XWMA_PACKET_DECODE_BUFFER_SIZE);

   for (int i = 0; i < BP_DOSTREAM_TRACK_COUNT; i++)
   {
      if (pAudioStream->m_directSoundTracks[i] != NULL)
      {
         pAudioStream->m_directSoundTracks[i]->DestroyVoice();
         pAudioStream->m_directSoundTracks[i] = NULL;
      }

      BPE_ASSERT(pAudioStream->m_directSoundTracks[i] == NULL, "Sound was not stopped.");
   }

   pAudioStream->m_bgmType = bp_get_stream_bgm_type( pAudioStream->m_streamTop, pInitData->channelCount );
   bp_start_stream_xmp_hook( pInitData->channel, pAudioStream->m_bgmType );

   for (int i = 0; i < pAudioStream->m_currTrackCount; i++)
   {
      //Don't send to submix voice for Direct-type streams; only streams emulating those played back on PS2 hardware voices
      //should have effects (reverb) applied to them.
      bool bSendSubmix = ( pAudioStream->m_streamType != BP_DOSTREAM_TYPE_DIRECT );
      pAudioStream->m_directSoundTracks[i] = CreateSourceVoicePCM( pInitData->channelCount, pInitData->freq, pAudioStream->m_directSoundCallback[i], bSendSubmix, pAudioStream->m_bgmType );
      pAudioStream->m_directSoundTracks[i]->Start();
      pAudioStream->m_currSoundState = kDOSS_Pending;
   }
   ApplyReverbEnabled_StreamVoice( pInitData->channel );

   // If this stream contains xWMA data then create a decoder.
   if (pAudioStream->m_format == WAVE_FORMAT_WMAUDIO2 || pAudioStream->m_format == WAVE_FORMAT_WMAUDIO3)
   {
      WAVEFORMATEXTENSIBLE wfx = {0};

      wfx.Format.wFormatTag = pInitData->format;
      wfx.Format.nChannels = pInitData->channelCount;
      wfx.Format.nSamplesPerSec = pInitData->freq;
      wfx.Format.nAvgBytesPerSec = pInitData->avgBytesPerSec;
      wfx.Format.nBlockAlign = pInitData->blockAlign;
      wfx.Format.wBitsPerSample = 16;

      DWORD cBufferSize;
      cBufferSize = XWMADecodeGetRequiredBufferSize(&wfx.Format);
      HRESULT hr =  XWMADecodeCreate(&wfx.Format, GetXWMAData, pAudioStream, &pAudioStream->m_xwmDecodeBuffer[0], 
         cBufferSize, &pAudioStream->m_pDecoder);

      BPE_ASSERT_NO_MSG(pAudioStream->m_streamChannel != gBP_MTAChannel);

      printf("CREATED DECODER.\n");

      if (FAILED(hr))
         printf("BPSOUND: Failed to create xWMA decoder.\n");
   }
   else
   {
      pAudioStream->m_pDecoder = NULL;
   }
}

//----------------------------------------------------------------------------
void AudioDriver::StartSound(int channel)
{
   BPE_ASSERT_NO_MSG(channel < BP_DOSTREAM_COUNT);
   EDirectOutputStreamState soundState = GetSoundState(channel);

   if (soundState == kDOSS_Pending)
   {
      printf("STARTING SOUND\n");

      QueueDirectSound(channel, -1);

      // If the format is xWMA setting the state to playing will allow the decoding to start.
      SetSoundState(channel, kDOSS_Playing);
   }
}

//----------------------------------------------------------------------------

void AudioDriver::StopSound(int channel)
{
   BPE_ASSERT_NO_MSG(channel < BP_DOSTREAM_COUNT);

   AudioStreamChannel* pAudioStream = m_audioStreams[channel];

   printf("SETTING SOUND STATE TO CLOSED ON CHANNEL: (%d) (WAS : %d)\n", channel, pAudioStream->m_currSoundState);

   SetSoundState(channel, kDOSS_Closed);
   
   // If we are not decoding xWMA data go ahead and stop and kill the xaudio2 voice. If we ARE decoding xWMA data,
   // let the decoder thread finish what it's currently doing and kill the sound itself. The decode thread also 
   // takes care of destroying the decoder context as well.
   if (pAudioStream->m_format == BP_DOSTREAM_FORMAT_PCM)
   {
      for (int i = 0; i < pAudioStream->m_currTrackCount; i++)
      {
         if (pAudioStream->m_directSoundTracks[i])
         {
            pAudioStream->m_directSoundTracks[i]->Stop();
            pAudioStream->m_directSoundTracks[i]->DestroyVoice();
            pAudioStream->m_directSoundTracks[i] = NULL;
         }
      }
   }
   else
   {
      if( pAudioStream->m_pDecoder )
      {
         // Wake up thread in case it's still waiting to start.
         SetEvent(pAudioStream->m_decodeReadyEvent);

         // Now wait for thread to finish destroying itself.
         printf("WAITING FOR DECODER TO FINISH ON CHANNEL(%d).\n", channel);
         WaitForSingleObject(pAudioStream->m_soundDestroyedEvent, INFINITE);
         BPE_ASSERT_NO_MSG(pAudioStream->m_pDecoder == NULL);    
      }
   }

   printf("DONE WITH STOP SOUND ON CHANNEL(%d).\n", channel);
   bp_stop_stream_xmp_hook( channel, pAudioStream->m_bgmType );
}

//----------------------------------------------------------------------------
void AudioDriver::QueueDirectSound(int channel, int track)
{
   BPE_ASSERT_NO_MSG(channel < BP_DOSTREAM_COUNT);
   
   AudioStreamChannel* pAudioStream = m_audioStreams[channel];
   if (pAudioStream->m_format == BP_DOSTREAM_FORMAT_PCM)
   {
      XAUDIO2_BUFFER playBuffer;

      playBuffer.Flags = XAUDIO2_END_OF_STREAM;
      playBuffer.AudioBytes = pAudioStream->m_currBufferSize;
      playBuffer.PlayBegin = 0;
      playBuffer.PlayLength = 0;
      playBuffer.LoopBegin = 0;
      playBuffer.LoopLength = 0;
      playBuffer.LoopCount = 0;
      playBuffer.pContext = NULL;

      // If track is negative queue on all tracks.
      if (track < 0)
      {
         for (int currTrack = 0; currTrack < pAudioStream->m_currTrackCount; currTrack++)
         {
            BPE_ASSERT(pAudioStream->m_directSoundTracks[currTrack] != NULL, "Invalid sound track.");
            playBuffer.pAudioData = (BYTE*)BP_GetDirectOutputStreamPlaybackBuffer(channel) + currTrack * playBuffer.AudioBytes;
            BPE_ASSERT_NO_MSG( playBuffer.pAudioData != NULL );   //non-BGM stream still going through the straight PCM interface?
            pAudioStream->m_directSoundTracks[currTrack]->SubmitSourceBuffer(&playBuffer);
         }
      }
      else
      {
         BPE_ASSERT_NO_MSG(track >= 0 && track < BP_DOSTREAM_TRACK_COUNT);
         BPE_ASSERT_NO_MSG(pAudioStream->m_directSoundTracks[track] != NULL);

         playBuffer.pAudioData = (BYTE*)BP_GetDirectOutputStreamPlaybackBuffer(channel) + track * playBuffer.AudioBytes;
         BPE_ASSERT_NO_MSG( playBuffer.pAudioData != NULL );   //non-BGM stream still going through the straight PCM interface?
         pAudioStream->m_directSoundTracks[track]->SubmitSourceBuffer(&playBuffer);
      }
   }
   else if (pAudioStream->m_format == WAVE_FORMAT_WMAUDIO2 || pAudioStream->m_format == WAVE_FORMAT_WMAUDIO3)
   {
      // When we go to play a xWMA formatted stream inform the decode thread there is now a valid stream available to 
      // decode.
      printf("STARTING DECODER ON CHANNEL(%d)\n", channel);
      SetEvent(pAudioStream->m_decodeReadyEvent);
   }   
}

//----------------------------------------------------------------------------
int AudioDriver::GetSoundPos(int channel)
{
   if (GetSoundState(channel) == kDOSS_Playing)
   {
#ifndef GOLD_VERSION

      if( ( gBP_DirectOutputStreamSpeed != 1 ) && ( gBP_DirectOutputStreamIsDemo != 0 ) )
      {
         if (m_audioStreams[channel]->m_directSoundTracks[0] )
         {
            m_audioStreams[channel]->m_directSoundTracks[0]->SetFrequencyRatio(gBP_DirectOutputStreamSpeed);
            mbChangedSimulatedFrequency = true;
         }
      }
      else
      {
         if( mbChangedSimulatedFrequency )
         {
            if( m_audioStreams[channel]->m_directSoundTracks[0] )
            {
               m_audioStreams[channel]->m_directSoundTracks[0]->SetFrequencyRatio(gBP_DirectOutputStreamSpeed);
            }
            mbChangedSimulatedFrequency = false;
         }
      }
#endif

      if (m_audioStreams[channel]->m_format == BP_DOSTREAM_FORMAT_PCM)
      {
         int numBytes = 0;
         XAUDIO2_VOICE_STATE voiceState;
         {
            CSyncCriticalSectionLocker lock(m_audioStreams[channel]->m_soundSync);
            m_audioStreams[channel]->m_directSoundTracks[0]->GetState(&voiceState);
         }

         int byteOffset = voiceState.SamplesPlayed * m_audioStreams[channel]->m_numAudioChannels * sizeof(short);
         if (byteOffset < m_audioStreams[channel]->m_currBufferSize)
         {
            numBytes = byteOffset;
         }
         else 
         {
            printf("DEBUG: Sound buffer overflow!!! Offset: %d BufferSize: %d NumTracks: %d\n", byteOffset, 
               m_audioStreams[channel]->m_currBufferSize, m_audioStreams[channel]->m_currTrackCount);
         }

         return numBytes;
      }
      else
      {
         //xWMA audio stream.
         BPE_ASSERT_NO_MSG(m_audioStreams[channel]->m_format == WAVE_FORMAT_WMAUDIO2 || m_audioStreams[channel]->m_format == WAVE_FORMAT_WMAUDIO3);
         return m_audioStreams[channel]->m_xwmReadPos % BP_DOSTREAM_BUFFER_SIZE;
      }
   }
   
   return 0;
}

//----------------------------------------------------------------------------
// This function 
void AudioDriver::NotifySoundBytesAvailable(int channel, void* pBuffer, int sizeBytes)
{
   if( m_audioStreams[channel]->m_format == WAVE_FORMAT_WMAUDIO2 || m_audioStreams[channel]->m_format == WAVE_FORMAT_WMAUDIO3 )
   {
      {
         CSyncCriticalSectionLocker lock(m_audioStreams[channel]->m_soundSync);

         AudioStreamChannel* pAudioStream = m_audioStreams[channel];

         XwmPacket* pWritePacket = &pAudioStream->m_xwmPackets[pAudioStream->m_writePacket];

         // Ensure packet we're about to recycle is consumed.
         BPE_VERIFY(pWritePacket->numEncodedBytes == 0, false, "SOUND: NotifySoundBytesAvailable: Writing data faster than we're reading.");

         // Copy data into local buffer
         memcpy(pWritePacket->buffer, pBuffer, sizeBytes);

         // encoded bytes is sttream header packet size 
         pWritePacket->numEncodedBytes = pAudioStream->m_pDecoder->XWMAPacketByteSize;

         // This size includes padding
         pWritePacket->packetSize = sizeBytes;

         // Keep track of global amount of data we have received from the streaming system (including padding)
         pAudioStream->m_bytesNotified += sizeBytes;
         pAudioStream->m_numXWMBytesReadNoPadding += pAudioStream->m_pDecoder->XWMAPacketByteSize;

         // Reached the end of the stream, tag packet as such.
         if (m_audioStreams[channel]->m_numXWMBytesReadNoPadding >= m_audioStreams[channel]->m_totalStreamSize)
            pWritePacket->lastPacket = true;
         else
            pWritePacket->lastPacket = false;

         // Keep track of global amount of data ready for consumption
         pAudioStream->m_numXWMBytesAvail += pWritePacket->numEncodedBytes;

         // Advance to next packet.
         pAudioStream->m_writePacket = (pAudioStream->m_writePacket + 1) % XWMA_NUM_ENCODED_PACKETS;

      }

      // If we got encoded data go ahead and start the sound. 
      StartSound(channel);
   }
}

//----------------------------------------------------------------------------
void AudioDriver::PauseSound(int channel, bool isPaused)
{
   AudioStreamChannel* pChannel = m_audioStreams[channel];
   pChannel->m_bPaused = isPaused;
   ApplyPause( channel );
}

//----------------------------------------------------------------------------
void AudioDriver::ApplyPause( int channel )
{
   AudioStreamChannel* pChannel = m_audioStreams[channel];

   CSyncCriticalSectionLocker lock(pChannel->m_soundSync);

   for (int i = 0; i < pChannel->m_currTrackCount; i++)
   {
      IXAudio2SourceVoice* pVoice = pChannel->m_directSoundTracks[i];

      if( pVoice != NULL )
      {
         if ( pChannel->m_bPaused || gDirectOutputStreamsPaused )
            pVoice->Stop();
         else
            pVoice->Start();
      }
   }
}

//----------------------------------------------------------------------------
void AudioDriver::SetTrackVolume_Direct(int channel, int trackNum, float volume)
{
   if (GetSoundState(channel) != kDOSS_Playing)
      return;

   BPE_ASSERT_NO_MSG(trackNum < BP_DOSTREAM_TRACK_COUNT);
   AudioStreamChannel* pAudioStream = m_audioStreams[channel];
   BPE_ASSERT( pAudioStream->m_streamType == BP_DOSTREAM_TYPE_DIRECT, "Wrong volume function for this stream type!" );

   float fCalcVols[64];
   BP_GetDirectOutputStreamSpeakerVolumes_Direct( pAudioStream->m_numAudioChannels, volume, fCalcVols );
   //Transpose when copying out: on X360 rows are speakers and columns are source channels.
   float fSpeakerVols[64] = { 0 };
   for( int ch=0; ch<pAudioStream->m_numAudioChannels; ++ch )
   {
      for( int speaker=0; speaker<gSoundSupportSurroundSoundChannelCount; ++speaker )
      {
         fSpeakerVols[speaker*pAudioStream->m_numAudioChannels+ch] = fCalcVols[ch*BP_SPEAKER_COUNT+speaker];
      }
   }

   CSyncCriticalSectionLocker lock(pAudioStream->m_soundSync);
   if (pAudioStream->m_directSoundTracks[trackNum])
   {
      //N.B. this type of "direct output" stream never goes through effect processing so there's no need to specify pDestinationVoice.
      //It always goes straight to the mastering voice.
      pAudioStream->m_directSoundTracks[trackNum]->SetOutputMatrix(NULL,pAudioStream->m_numAudioChannels, gSoundSupportSurroundSoundChannelCount, fSpeakerVols);
   }
}

//----------------------------------------------------------------------------
void AudioDriver::SetTrackVolume_Voice(const int channel, const float fStereoVols[2][2], const float pan3d,
                                       const float vol3d, BP_SURROUND_VOICE_TYPE surroundType)
{
   if (GetSoundState(channel) != kDOSS_Playing)
      return;

   float const stereoLeft0 = fStereoVols[0][0];
   float const stereoRight0 = fStereoVols[0][1];
   float const stereoLeft1 = fStereoVols[1][0];
   float const stereoRight1 = fStereoVols[1][1];

   AudioStreamChannel* pAudioStream = m_audioStreams[channel];
   BPE_ASSERT_NO_MSG(pAudioStream->m_format == WAVE_FORMAT_WMAUDIO2 || pAudioStream->m_format == WAVE_FORMAT_WMAUDIO3);

   //voice type streams can only be mono or stereo.
   BPE_ASSERT_NO_MSG(pAudioStream->m_numAudioChannels <= 2);

   CSyncCriticalSectionLocker lock(pAudioStream->m_soundSync);
   if (pAudioStream->m_directSoundTracks[0])
   {
      float speakerVolumes[2 * BP_SPEAKER_COUNT] = { 0 };
      if( pAudioStream->m_numAudioChannels == 2 )
      {
         //Use a fixed volume mix.
         //This configuration ignores all pan information.  All L volume goes to channel 0 and all R volume goes to channel 1.
         if( BP_SoundSupport_IsSurroundSound() )
         {
            float speakerVolumes0[BP_SPEAKER_COUNT];
            float speakerVolumes1[BP_SPEAKER_COUNT];
            BP_GetVoiceSpeakerVolumes( stereoLeft0, stereoRight0, pan3d, vol3d, surroundType, speakerVolumes0 );
            BP_GetVoiceSpeakerVolumes( stereoRight0, stereoRight1, pan3d, vol3d, surroundType, speakerVolumes1 );
            //Transpose when copying out: on X360 rows are speakers and columns are source channels.
            for( int i=0; i < BP_SPEAKER_COUNT; ++i )
            {
               speakerVolumes[i*2+0] = speakerVolumes0[i];
               speakerVolumes[i*2+1] = speakerVolumes1[i];
            }
         }
         else
         {
            speakerVolumes[0] = stereoLeft0;
            speakerVolumes[1] = stereoLeft1;
            speakerVolumes[2] = stereoRight0;
            speakerVolumes[3] = stereoRight1;
         }
         //Regardless of master volume mix, store stereo volume mix for submix voice (reverb).
         //first row is stereo left.
         pAudioStream->m_stereoVolumeMatrix[0] = stereoLeft0;
         pAudioStream->m_stereoVolumeMatrix[1] = stereoLeft1;
         //second row is stereo right.
         pAudioStream->m_stereoVolumeMatrix[2] = stereoRight0;
         pAudioStream->m_stereoVolumeMatrix[3] = stereoRight1;
      }
      else
      {
         //Mono.
         //In this case, the original vag stream used two hardware voices playing back the same data.
         //We only have one source channel in this replacement type so add the volumes for the same effect.
         //Left volume is set on channel 0 and right volume is set on channel 1.
         if( BP_SoundSupport_IsSurroundSound() )
         {
            BP_GetVoiceSpeakerVolumes( stereoLeft0+stereoLeft1, stereoRight0+stereoRight1, pan3d, vol3d, surroundType, speakerVolumes );
         }
         else
         {
            speakerVolumes[0] = stereoLeft0 + stereoLeft1;
            speakerVolumes[1] = stereoRight0 + stereoRight1;
         }
         //Regardless of master volume mix, store stereo volume mix for submix voice (reverb).
         //first row is stereo left, stereo right.
         pAudioStream->m_stereoVolumeMatrix[0] = stereoLeft0 + stereoLeft1;
         pAudioStream->m_stereoVolumeMatrix[1] = stereoRight0 + stereoRight1;
         //second row is unused.
         pAudioStream->m_stereoVolumeMatrix[2] = 0.f;
         pAudioStream->m_stereoVolumeMatrix[3] = 0.f;
      }

      pAudioStream->m_directSoundTracks[0]->SetOutputMatrix(m_pMasterVoice, pAudioStream->m_numAudioChannels, gSoundSupportSurroundSoundChannelCount, speakerVolumes);
   }

   ApplyReverbEnabled_StreamVoice( channel );
}

//----------------------------------------------------------------------------
void AudioDriver::ApplyReverbEnabled_StreamVoice(const int channel)
{
#ifdef BP_XAUDIO_USE_REVERB
   AudioStreamChannel* pAudioStream = m_audioStreams[channel];
   //Send either full or no volume to submix voice.
   float reverbVol = pAudioStream->m_bReverbEnabled ? 1.f : 0.f;
   float finalVols[2*2];
   //If this is a stereo stream, the first two are stereo left and the second two are stereo right.
   //If this is a mono stream, the first one is stereo left and the second one is stereo right and the last two are ignored.
   finalVols[0] = pAudioStream->m_stereoVolumeMatrix[0] * reverbVol;
   finalVols[1] = pAudioStream->m_stereoVolumeMatrix[1] * reverbVol;
   finalVols[2] = pAudioStream->m_stereoVolumeMatrix[2] * reverbVol;
   finalVols[3] = pAudioStream->m_stereoVolumeMatrix[3] * reverbVol;

   CSyncCriticalSectionLocker lock(pAudioStream->m_soundSync);
   if( pAudioStream->m_directSoundTracks[0] && pAudioStream->m_streamType == BP_DOSTREAM_TYPE_VOICE )
   {
//      printf("ApplyReverbEnabled_StreamVoice: %d -> %f %f %f %f\n", pAudioStream->m_numAudioChannels, finalVols[0],finalVols[1],finalVols[2],finalVols[3]);
      pAudioStream->m_directSoundTracks[0]->SetOutputMatrix(m_pSubmixVoice, pAudioStream->m_numAudioChannels, 2, finalVols);
   }
#endif
}

//----------------------------------------------------------------------------
double AudioDriver::GetSoundPlayTime(int channel)
{
   CSyncCriticalSectionLocker lock(m_audioStreams[channel]->m_soundSync);

   EDirectOutputStreamState currSoundState = GetSoundState(channel);
   
   if (currSoundState == kDOSS_Playing )
   {
      XAUDIO2_VOICE_STATE voiceState;
      m_audioStreams[channel]->m_directSoundTracks[0]->GetState(&voiceState);

      return voiceState.SamplesPlayed / (double)m_audioStreams[channel]->m_currFrequency;
   }
   else 
   {
      return 0.0;
   }
}

//----------------------------------------------------------------------------
int AudioDriver::IsSoundDone(int channel)
{
   if (m_audioStreams[channel]->m_totalStreamSize == 0)
      return 0;

   BPE_ASSERT_NO_MSG(m_audioStreams[channel]->m_format == WAVE_FORMAT_WMAUDIO2 ||
      m_audioStreams[channel]->m_format == WAVE_FORMAT_WMAUDIO3);

   if( m_audioStreams[channel]->m_isStreamDonePlaying )
   { 
      printf("SOUND IS DONE!\n");
      StopSound(channel);
      return 1;
   }
   else
   {
      return 0;
   }
}

//----------------------------------------------------------------------------
void AudioDriver::SetVoiceParam(int param, int core, int voice, unsigned short value)
{
   SSpuVoiceState& voiceState = m_spuVoiceState[core][voice];

   switch(param)
   {
   case SD_VP_VOLL:
      {
         //voice volume (left)
         voiceState.mXAudioVolumeL = BP_PS2VolToFVol( value );
         //Changed to explicitly apply volume all at once in BP_SoundSupport_ApplyVoiceVolume()
         //so there's only one call to SetOutputMatrix, just to be sure everything stays in sync.
      }
      break;

   case SD_VP_VOLR:
      {
         //voice volume (right)
         voiceState.mXAudioVolumeR = BP_PS2VolToFVol( value );
         //Changed to explicitly apply volume all at once in BP_SoundSupport_ApplyVoiceVolume()
         //so there's only one call to SetOutputMatrix, just to be sure everything stays in sync.
      }
      break;

   case SD_VP_PITCH:
      {
         //pitch when sound is generated
         voiceState.mXAudioPitch = (((int)value*48000)+2048)/4096;
         if (voiceState.mXAudioPitch < XAUDIO2_MIN_SAMPLE_RATE )
         {
            printf("INVALID SOUND PITCH! Pitch was %d\n", voiceState.mXAudioPitch);
            voiceState.mXAudioPitch = XAUDIO2_MIN_SAMPLE_RATE;
         }
         else if( voiceState.mXAudioPitch > XAUDIO2_MAX_SAMPLE_RATE )
         {
            printf("INVALID SOUND PITCH! Pitch was %d\n", voiceState.mXAudioPitch);
            voiceState.mXAudioPitch = XAUDIO2_MAX_SAMPLE_RATE;
         }

         //Change pitch of an already playing voice.
         ApplyPitch( voiceState );
      }
      break;

   case SD_VP_ADSR1:
      {
         //envelope
         voiceState.mADSREnvelope.mADSR1 = value;
      }
      break;

   case SD_VP_ADSR2:
      {
         //envelope (2)
         voiceState.mADSREnvelope.mADSR2 = value;
      }
      break;

   case SD_P_EVOLL:
      {
         //Effect return volume (left)
         //NOTE: MGS2/3 always sets L/R together and to the same value.
         //Set it here and verify that the value matches the R setting below.
         //Then apply it once there.
         gReverbDepthPS2 = value;
         float fDepth = (float)value / 0x7fff;
         //printf("SD_P_EVOLL: %d -> %f\n", value, fDepth );
      }
      break;
   case SD_P_EVOLR:
      {
         //Effect return volume (right)
         //NOTE: MGS2/3 always L/R together and to the same value.
         //See above.
         if( value != gReverbDepthPS2 )
            BP_TODO_BREAK;

         ApplyReverbSetting();
      }
      break;

   default:
      printf("Unimplemented set parameter: %d\n", param);
      break;
   }
}

//----------------------------------------------------------------------------
unsigned short AudioDriver::GetVoiceParam(int param, int core, int voice)
{
   SSpuVoiceState& voiceState = m_spuVoiceState[core][voice];

   switch (param)
   {
   case SD_VP_ENVX:
      {
         XAUDIO2_VOICE_STATE xAudioVoiceState;
         voiceState.mpAudioVoice->GetState(&xAudioVoiceState, XAUDIO2_VOICE_NOSAMPLESPLAYED);

         if( xAudioVoiceState.BuffersQueued == 0 )
         {
            return 0;
         }

#ifndef BP_USE_NEW_ADSR_ENVELOPES
         return 0xffff;
#else
         const float currEnv = voiceState.mADSREnvelope.mEnvelopeScale;
         unsigned short releaseLevel = (unsigned short)(currEnv * 0xffff);
         return releaseLevel;
#endif
      }
      break;

   default:
      printf("Unsupported voice parameter: %d\n", param);
      break;
   }
   return 0;
}

//----------------------------------------------------------------------------
void AudioDriver::SetVoiceSourceAddr(int type, int core, int voice, int addr)
{
   SSpuVoiceState&voiceState = m_spuVoiceState[core][voice];

   switch(type)
   {
   case SD_VA_SSA:
      {
         //waveform data starting address
         StopVoice( voiceState );
         voiceState.mSSA = addr;
         voiceState.mpMemoryStream = NULL;
         voiceState.mMemoryStreamMuteVolume = 1;
      }
      break;

   case SD_VA_LSAX:
      {
         //loop point address
         //manually setting loop block point for this key
         voiceState.mLSAX = addr;
      }
      break;

   default:
      printf("Unimplemented setaddr type: %d\n", type);
      break;
   }
}

//----------------------------------------------------------------------------
IXAudio2SourceVoice* AudioDriver::CreateSourceVoicePCM( int channelCount, int samplesPerSecond, IXAudio2VoiceCallback *pCallback, bool bSendSubmix, EBgmMusicType bgmType )
{
   const XAUDIO2_EFFECT_CHAIN *pEffectChain = NULL;
   const XAUDIO2_VOICE_SENDS *pSendList = NULL;

#ifdef BP_XAUDIO_USE_REVERB
   XAUDIO2_SEND_DESCRIPTOR sendDescriptors[kSend_Count];
   sendDescriptors[kSend_Master].Flags = XAUDIO2_SEND_USEFILTER; // LPF direct-path
   sendDescriptors[kSend_Master].pOutputVoice = m_pMasterVoice;
   sendDescriptors[kSend_Submix].Flags = XAUDIO2_SEND_USEFILTER; // LPF reverb-path -- omit for better performance at the cost of less realistic occlusion
   sendDescriptors[kSend_Submix].pOutputVoice = m_pSubmixVoice;
   const XAUDIO2_VOICE_SENDS sendList = { BPE_ARRAY_SIZE(sendDescriptors), sendDescriptors };
   pSendList = &sendList;
#endif

   if( !bSendSubmix )
   {
      //Default to just sending to mastering voice.
      pSendList = NULL;
   }

   WAVEFORMATEX soundFormat;
   soundFormat.wFormatTag = WAVE_FORMAT_PCM;
   soundFormat.nChannels = channelCount;
   soundFormat.wBitsPerSample = 16;
   soundFormat.nSamplesPerSec = samplesPerSecond;
   soundFormat.nBlockAlign = soundFormat.nChannels * sizeof(short);
   soundFormat.nAvgBytesPerSec = soundFormat.nSamplesPerSec * soundFormat.nBlockAlign;
   soundFormat.cbSize = 0;

   UINT32 flags = 0;
   if( bgmType == kBgmMusicType_BgmOnly )
   {
      //this voice will be muted if XMP music is playing.
      flags |= XAUDIO2_VOICE_MUSIC;
   }
   IXAudio2SourceVoice* pSourceVoice;
   HRESULT hr;
   if (FAILED(hr=m_pXAudio->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&soundFormat, flags, 
      XAUDIO2_MAX_FREQ_RATIO, pCallback, pSendList, pEffectChain)))
   {
      printf("Failed to create new audio voice. Error code: %d", hr);
      BP_BREAK;
   }

   const float masterVolume = 1.f;
   bp_set_bgm_mute_volume( pSourceVoice, bgmType, masterVolume );
   return pSourceVoice;
}

//----------------------------------------------------------------------------
void AudioDriver::StartVoice(int core, int voice)
{
   SSpuVoiceState& voiceState = m_spuVoiceState[core][voice];

   StopVoiceForReuse(voiceState);

   get_vag_endpoints(voiceState);
   voiceState.mCurrPlayingBufferType = kVBT_FirstBuffer;

   BP_InitADSREnvelope( &voiceState.mADSREnvelope );
   ApplyVolume( voiceState );
   ApplyPitch( voiceState );

   QueueAudioVoice(voiceState);
   voiceState.mpAudioVoice->Start();
}

//----------------------------------------------------------------------------
void AudioDriver::QueueAudioVoice(SSpuVoiceState& voiceState)
{
   XAUDIO2_BUFFER firstBuffer = { 0 };

   // Set address and size of data to play
   firstBuffer.Flags = XAUDIO2_END_OF_STREAM;

   int sb_size = voiceState.mBlockEnd - voiceState.mSSA;
   int loop_size = voiceState.mBlockEnd - voiceState.mBlockBegin;

   int num_sb_packets = sb_size / BP_VAG_PACKET_SIZE;
   int num_loop_packets = loop_size / BP_VAG_PACKET_SIZE;

   sb_size = num_sb_packets * BP_PCM_SIZE_FROM_VAG_SIZE;
   loop_size = num_loop_packets * BP_PCM_SIZE_FROM_VAG_SIZE;

   int sb_offset = voiceState.mSSA * BP_PCM_SIZE_FROM_VAG_SIZE / BP_VAG_PACKET_SIZE;
   int loop_offset = voiceState.mBlockBegin * BP_PCM_SIZE_FROM_VAG_SIZE / BP_VAG_PACKET_SIZE;

   voiceState.mCurrPlayingBufferType = kVBT_FirstBuffer;
   firstBuffer.pAudioData = (BYTE*)(&gDecompressedVagBuffer[0] + sb_offset);
   firstBuffer.AudioBytes = sb_size;
   firstBuffer.PlayBegin = 0;
   firstBuffer.PlayLength = sb_size / sizeof(short);

   if(voiceState.mbBlockLoop)
   {
      BPE_ASSERT(voiceState.mpMemoryStream == NULL, "Memory streams should not be valid in loops.");
#ifndef BP_DYNAMIC_LOOP_BUFFERS
      firstBuffer.LoopBegin = (loop_offset-sb_offset) / sizeof(short);
      firstBuffer.LoopLength = loop_size / sizeof(short);
      firstBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
#else
      firstBuffer.Flags = 0;
#endif
   }
   else if( voiceState.mpMemoryStream )
   {
      voiceState.mCurrPlayingBufferType = kVBT_MemoryBuffer;

      int numVagPackets = voiceState.mMemoryStreamSize / BP_VAG_PACKET_SIZE;
      int numPCMBytes = numVagPackets * BP_PCM_SIZE_FROM_VAG_SIZE;

      BPE_ASSERT_NO_MSG( voiceState.mpDecompressedMemStream == NULL );
      voiceState.mpDecompressedMemStream = (char*)BP_Memory_Alloc(numPCMBytes, 4, kMT_Permanent, kMC_Sound);
      ++gBP_Sound_CreatedMemStreams;
      BP_DecompressVAG(voiceState.mpMemoryStream, numVagPackets, voiceState.mpDecompressedMemStream, NULL, NULL);

      firstBuffer.pAudioData = (BYTE*)voiceState.mpDecompressedMemStream;
      firstBuffer.AudioBytes = numPCMBytes;
      firstBuffer.PlayLength = numPCMBytes / sizeof(short);
   }

#ifdef BP_DUMP_XAUDIO_WAVS
   {
      char dumpWavFilename[FILENAME_MAX];
      static int dumpCount = 0;
      sprintf( dumpWavFilename, "E:\\%d_%08x_%d%s.wav", dumpCount++, firstBuffer.pAudioData, firstBuffer.AudioBytes, voiceState.mbBlockLoop ? "_l" : "" );
      short * swappedBuf = new short[firstBuffer.AudioBytes];
      memcpy( swappedBuf, firstBuffer.pAudioData, firstBuffer.AudioBytes );
      BP_LE_SwapSShortArray_Inp( swappedBuf, firstBuffer.AudioBytes / sizeof(short) );
      BP_SaveLPCM16WavFile( dumpWavFilename, 1, voiceState.mXAudioPitch, firstBuffer.AudioBytes, (const unsigned char*)swappedBuf );
      delete swappedBuf;

   }
#endif
   voiceState.mpAudioVoice->SubmitSourceBuffer(&firstBuffer);
#ifdef BP_DYNAMIC_LOOP_BUFFERS
   if( voiceState.mbBlockLoop )
   {
      const TSpuLoopBuffer & loopBuffer = gSpuLoopBufferMap[voiceState.mSSA];
      BPE_ASSERT( !loopBuffer.empty(), "Should be a loop buffer for this ssa!" );
      XAUDIO2_BUFFER secondBuffer = { 0 };
      secondBuffer.Flags = XAUDIO2_END_OF_STREAM;
      secondBuffer.pAudioData = (BYTE*)(&loopBuffer[0]);
      secondBuffer.AudioBytes = loopBuffer.size();
      secondBuffer.PlayBegin = 0;
      secondBuffer.PlayLength = loopBuffer.size() / sizeof(short);
      secondBuffer.LoopBegin = 0;
      secondBuffer.LoopLength = loopBuffer.size() / sizeof(short);
      secondBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
      voiceState.mpAudioVoice->SubmitSourceBuffer(&secondBuffer);
   }
#endif
}

//----------------------------------------------------------------------------
void AudioDriver::StopVoice(SSpuVoiceState & voiceState)
{
   voiceState.mpAudioVoice->Stop(XAUDIO2_PLAY_TAILS);  //always allow tails
}

//----------------------------------------------------------------------------
void AudioDriver::DestroyVoice( SSpuVoiceState & voiceState )
{
   BPE_ASSERT( voiceState.mpAudioVoice != NULL, "active voice should always exist!");
   //Destroy *active* voice.
   voiceState.mpAudioVoice->DestroyVoice();
   voiceState.mpAudioVoice = NULL;
   --gBP_Sound_CreatedVoices;

   //Free to destroy the memory stream buffer for this voice.
   if (voiceState.mpDecompressedMemStream)
   {
      BP_Memory_Free(voiceState.mpDecompressedMemStream);
      --gBP_Sound_CreatedMemStreams;
      voiceState.mpDecompressedMemStream = NULL;
   }
}

void AudioDriver::StopVoiceForReuse(SSpuVoiceState & voiceState)
{
   if( voiceState.mpDecompressedMemStream )
   {
      //No playback problem to immediately destroy in this case, and we don't want to have to manage
      //two memory stream buffers at once.
      DestroyVoice( voiceState );
   }

   std::swap( voiceState.mpAudioVoice, voiceState.mpSpareAudioVoice );

   //Voice just stopped (and possibly destroyed) is now the spare audio voice.
   if( voiceState.mpSpareAudioVoice )
   {
      //Spare audio voice now needs to die gracefully.
      if( voiceState.mADSREnvelope.mADSRState != kADSRState_Off && voiceState.mADSREnvelope.mEnvelopeScale > 0.f )
      {
         //Key off from current envelope scale with release.
         voiceState.mSpareVoiceEnvelope = 1.f;
         const float kSpareVoiceReleaseSpeed = 10.f;  //1/10s release for a full envelope.
         voiceState.mSpareVoiceReleaseSpeed = kSpareVoiceReleaseSpeed / voiceState.mADSREnvelope.mEnvelopeScale;
         //printf("StopVoiceForReuse: Key off spare audio voice [%d][%2d]: %f -> %f\n", voiceState.mCore, voiceState.mVoice, voiceState.mADSREnvelope.mEnvelopeScale, voiceState.mSpareVoiceReleaseSpeed );
      }
      else
      {
         //already muted, make sure envelope scale is zapped and voice is immediately stopped.
         voiceState.mSpareVoiceEnvelope = 0.f;
         voiceState.mSpareVoiceReleaseSpeed = 0.f;
         voiceState.mpSpareAudioVoice->Stop(XAUDIO2_PLAY_TAILS);  //always allow tails
         //printf("StopVoiceForReuse: Stop spare audio voice [%d][%2d]\n", voiceState.mCore, voiceState.mVoice );
      }
   }
   else
   {
      //if just-stopped voice was destroyed, nothing more to do.
   }

   //What *was* the spare voice is now the voice to be reused.  It must be destroyed and re-created on the spot.
   if( voiceState.mpAudioVoice )
   {
      DestroyVoice( voiceState );
   }
   //Recreate this voice.
   {
      IXAudio2VoiceCallback *pCallback = NULL;
      bool bSendSubmix = true;
      EBgmMusicType bgmType = bp_get_spu_voice_bgm_type( voiceState.mCore, voiceState.mVoice );
      voiceState.mpAudioVoice = CreateSourceVoicePCM( 1, BP_NATIVE_SAMPLES_PER_SECOND, pCallback, bSendSubmix, bgmType );
      ++gBP_Sound_CreatedVoices;
   }

   //Restore surround type to default setting.
   voiceState.mLastSampledVoicePos = 0;
}

//----------------------------------------------------------------------------
void AudioDriver::ApplyVoiceReverb( SSpuVoiceState & voiceState )
{
#ifdef BP_XAUDIO_USE_REVERB
   float masterVolume = voiceState.mpMemoryStream ? voiceState.mMemoryStreamMuteVolume : 1.f;
   masterVolume *= voiceState.mADSREnvelope.mEnvelopeScale;
   float stereoVolL = voiceState.mXAudioVolumeL * masterVolume;
   float stereoVolR = voiceState.mXAudioVolumeR * masterVolume;
   //Send either full or no volume to submix voice.
   float reverbVol = voiceState.mbReverbEnabledR ? 1.f : 0.f;

   float finalVols[2] = { stereoVolL * reverbVol, stereoVolR * reverbVol };
   voiceState.mpAudioVoice->SetOutputMatrix( m_pSubmixVoice, 1, 2, finalVols );
#endif
}

//----------------------------------------------------------------------------
void AudioDriver::PlayMemoryStream(int core, int voice, const void * const playbackBuffer, int playbackBufferSize)
{
   BPE_ASSERT(core < 2 && voice < 24, "");

   SSpuVoiceState&voiceState = m_spuVoiceState[core][voice];
   StopVoice( voiceState );
   voiceState.mpMemoryStream = (const unsigned char*)playbackBuffer;
   voiceState.mMemoryStreamSize = playbackBufferSize;
   voiceState.mMemoryStreamMuteVolume = 1;
}

//----------------------------------------------------------------------------
void AudioDriver::MuteMemoryStream(int core, int voice, int muted)
{
   BPE_ASSERT(core < 2 && voice < 24, "");
   SSpuVoiceState& voiceState = m_spuVoiceState[core][voice];
   
   if(voiceState.mpMemoryStream)
   {
      voiceState.mMemoryStreamMuteVolume = muted ? 0.0f : 1.0f;
      ApplyVolume( voiceState );
   }
}

void AudioDriver::ApplyReverbSetting()
{
#ifdef BP_XAUDIO_USE_REVERB
   XAUDIO2FX_REVERB_PARAMETERS native;
   sCurrXAudioReverbMode = *skXAudioReverbModes[gReverbModePS2];
   ReverbConvertI3DL2ToNative( &sCurrXAudioReverbMode, &native );
   m_pSubmixVoice->SetEffectParameters( 0, &native, sizeof( native ) );
   const float ps2Depth = (float)gReverbDepthPS2 / 0x7fff;
   const float reverbVol = ps2Depth * m_ReverbMasterVolume;

   m_pSubmixVoice->SetVolume( reverbVol );

#endif
}

//----------------------------------------------------------------------------
unsigned int AudioDriver::GetVoicePos(int core, int voice)
{
   SSpuVoiceState& voiceState = m_spuVoiceState[core][voice];

   XAUDIO2_VOICE_STATE xAudioVoiceState;
   voiceState.mpAudioVoice->GetState(&xAudioVoiceState);

   if( xAudioVoiceState.BuffersQueued == 0 )
   {
      return 0;
   }

   int slotAddress;
   int numPCMSamples;
   switch (voiceState.mCurrPlayingBufferType)
   {
   case kVBT_FirstBuffer:
      {
         const int playLength = ( voiceState.mBlockEnd - voiceState.mSSA ) * 28 / 16;
         numPCMSamples = xAudioVoiceState.SamplesPlayed;
         if( numPCMSamples > playLength )
         {
            if( voiceState.mbBlockLoop )
            {
               const int loopBegin = ( voiceState.mBlockBegin - voiceState.mSSA ) * 28 / 16;
               const int loopSize = ( voiceState.mBlockEnd - voiceState.mBlockBegin ) * 28 / 16;
               //After playing the full playLength once, it starts looping within the loop range.
               int loopPos = numPCMSamples - playLength;
               //Wrap within loop range.
               loopPos %= loopSize;
               //Actual current position.
               numPCMSamples = loopBegin + loopPos;
            }
            else
            {
               //should stop at the end of the playback buffer when not looping.
            }
         }
         slotAddress = (int)(voiceState.mSSA);
      }
      break;

   default:
      BP_TODO_BREAK;
   case kVBT_MemoryBuffer:
      //Used by BP_GetMemStreamPosition().  Return relative to buffer head.
      numPCMSamples = xAudioVoiceState.SamplesPlayed;
      slotAddress = 0;
      break;
   }

   // Each vag packet has 28 samples so using the number of pcm samples played, we can determine the number
   // of vag packets played giving us a byte offset.
   int spuOffset = numPCMSamples * 16 / 28;

   voiceState.mLastSampledVoicePos = slotAddress + spuOffset;
   return voiceState.mLastSampledVoicePos;
}

void AudioDriver::SurroundSoundSetVoiceType( const int core, const int voice, BP_SURROUND_VOICE_TYPE voiceType )
{
   SSpuVoiceState & voiceState = m_spuVoiceState[core][voice];
   voiceState.mSurroundType = voiceType;
   //Changed to explicitly apply volume all at once in BP_SoundSupport_ApplyVoiceVolume()
   //so there's only one call to SetOutputMatrix, just to be sure everything stays in sync.
}

void AudioDriver::SurroundSoundSetPanVol( const int core, const int voice, float pan, float vol )
{
   SSpuVoiceState & voiceState = m_spuVoiceState[core][voice];
   voiceState.mPan3d = pan;
   voiceState.mVol3d = vol;
   //Changed to explicitly apply volume all at once in BP_SoundSupport_ApplyVoiceVolume()
   //so there's only one call to SetOutputMatrix, just to be sure everything stays in sync.
}

void AudioDriver::ApplyVolume( SSpuVoiceState & voiceState )
{
   const EADSRState prevADSRState = voiceState.mADSREnvelope.mADSRState;
#ifndef BP_USE_NEW_ADSR_ENVELOPES
   voiceState.mADSREnvelope.mEnvelopeScale = 1.f;
#else
   if( prevADSRState != kADSRState_Off )
   {
      BP_UpdateADSREnvelope( &voiceState.mADSREnvelope );
   }
#endif
   const bool bKeyOffAfterRelease = ( prevADSRState != kADSRState_Off ) && ( voiceState.mADSREnvelope.mADSRState == kADSRState_Off );

   float masterVolume = voiceState.mpMemoryStream ? voiceState.mMemoryStreamMuteVolume : 1.f;
   masterVolume *= voiceState.mADSREnvelope.mEnvelopeScale;
   float stereoVolL = voiceState.mXAudioVolumeL * masterVolume;
   float stereoVolR = voiceState.mXAudioVolumeR * masterVolume;
   float vol3d = voiceState.mVol3d * masterVolume;
   BP_GetVoiceSpeakerVolumes( stereoVolL, stereoVolR, voiceState.mPan3d, vol3d, (BP_SURROUND_VOICE_TYPE)voiceState.mSurroundType, voiceState.mXAudioSurroundVolumes );

   voiceState.mpAudioVoice->SetOutputMatrix(m_pMasterVoice, 1, gSoundSupportSurroundSoundChannelCount, voiceState.mXAudioSurroundVolumes );

   if( bKeyOffAfterRelease )
   {
      //Full stop.
      StopVoice(voiceState);
   }

   ApplyVoiceReverb( voiceState );
}

void AudioDriver::ApplyKeyOffVolume( SSpuVoiceState & voiceState, const float dt )
{
   if( voiceState.mpSpareAudioVoice )
   {
      if( voiceState.mSpareVoiceEnvelope > 0.f )
      {
         const float prevEnvelope = voiceState.mSpareVoiceEnvelope;
         const float releaseDelta = voiceState.mSpareVoiceReleaseSpeed * dt;
         voiceState.mSpareVoiceEnvelope -= releaseDelta;
         //printf("ApplyKeyOffVolume [%d][%2d]: %f->%f\n", voiceState.mCore, voiceState.mVoice, prevEnvelope, voiceState.mSpareVoiceEnvelope );
         if( voiceState.mSpareVoiceEnvelope <= 0.f )
         {
            voiceState.mSpareVoiceEnvelope = 0.f;
            voiceState.mSpareVoiceReleaseSpeed = 0.f;
            voiceState.mpSpareAudioVoice->Stop(XAUDIO2_PLAY_TAILS);
         }
         else
         {
            EBgmMusicType bgmType = bp_get_spu_voice_bgm_type( voiceState.mCore, voiceState.mVoice );
            bp_set_bgm_mute_volume( voiceState.mpSpareAudioVoice, bgmType, voiceState.mSpareVoiceEnvelope );
         }
      }
   }
}

void AudioDriver::ApplyPitch( SSpuVoiceState & voiceState )
{
   float frequencyRatio = (float)voiceState.mXAudioPitch / BP_NATIVE_SAMPLES_PER_SECOND;
   if( frequencyRatio > XAUDIO2_MAX_FREQ_RATIO )
   {
      frequencyRatio = XAUDIO2_MAX_FREQ_RATIO;
   }
   else if( frequencyRatio < XAUDIO2_MIN_FREQ_RATIO )
   {
      frequencyRatio = XAUDIO2_MIN_FREQ_RATIO;
   }
//   printf("PITCH [%d][%2d] %d %d -> %f\n", core, voice, value, voiceState.mXAudioPitch, frequencyRatio );
   voiceState.mpAudioVoice->SetFrequencyRatio( frequencyRatio );
}

//----------------------------------------------------------------------------
AudioDriver* g_pAudio;

//----------------------------------------------------------------------------
static int BP_DebugMenu_ActionCallback_ReverbSettings_Changed(int data)
{
   *skXAudioReverbModes[gReverbModePS2] = sCurrXAudioReverbMode;

   g_pAudio->ApplyReverbSetting();
   return 0;
}

void BP_SoundSupport_InitDebugMenuPlatformSpecific( int soundMenu, int reverbMenu )
{
#if BP_ENABLE_DEBUG_MENU
   int item;
   item = BP_DebugMenu_AddFloat(reverbMenu, "WetVolume", &g_pAudio->m_ReverbMasterVolume, 0.0f, 8.f, 0.05f, 0.2f);
   BP_DebugMenu_SetCallback( item, BP_DebugMenu_ActionCallback_ReverbSettings_Changed, 0 );

   item = BP_DebugMenu_AddInt(reverbMenu, "Room", &sCurrXAudioReverbMode.Room, -10000, 0, 10, 100);
   BP_DebugMenu_SetCallback( item, BP_DebugMenu_ActionCallback_ReverbSettings_Changed, 0 );

   item = BP_DebugMenu_AddInt(reverbMenu, "RoomHF", &sCurrXAudioReverbMode.RoomHF, -10000, 0, 10, 100);
   BP_DebugMenu_SetCallback( item, BP_DebugMenu_ActionCallback_ReverbSettings_Changed, 0 );

   item = BP_DebugMenu_AddFloat(reverbMenu, "DecayTime", &sCurrXAudioReverbMode.DecayTime, 0.1f, 20.f, 0.1f, 1.f);
   BP_DebugMenu_SetCallback( item, BP_DebugMenu_ActionCallback_ReverbSettings_Changed, 0 );

   item = BP_DebugMenu_AddFloat(reverbMenu, "DecayHFRatio", &sCurrXAudioReverbMode.DecayHFRatio, 0.1f, 20.f, 0.1f, 1.f);
   BP_DebugMenu_SetCallback( item, BP_DebugMenu_ActionCallback_ReverbSettings_Changed, 0 );

   item = BP_DebugMenu_AddInt(reverbMenu, "Reflections", &sCurrXAudioReverbMode.Reflections, -10000, 1000, 10, 100);
   BP_DebugMenu_SetCallback( item, BP_DebugMenu_ActionCallback_ReverbSettings_Changed, 0 );

   item = BP_DebugMenu_AddFloat(reverbMenu, "ReflectionsDelay", &sCurrXAudioReverbMode.ReflectionsDelay, 0.f, 0.3f, 0.01f, 0.1f);
   BP_DebugMenu_SetCallback( item, BP_DebugMenu_ActionCallback_ReverbSettings_Changed, 0 );

   item = BP_DebugMenu_AddInt(reverbMenu, "Reverb", &sCurrXAudioReverbMode.Reverb, -10000, 2000, 10, 100);
   BP_DebugMenu_SetCallback( item, BP_DebugMenu_ActionCallback_ReverbSettings_Changed, 0 );

   item = BP_DebugMenu_AddFloat(reverbMenu, "ReverbDelay", &sCurrXAudioReverbMode.ReverbDelay, 0.f, 0.1f, 0.01f, 0.1f);
   BP_DebugMenu_SetCallback( item, BP_DebugMenu_ActionCallback_ReverbSettings_Changed, 0 );

   item = BP_DebugMenu_AddFloat(reverbMenu, "Diffusion", &sCurrXAudioReverbMode.Diffusion, 0.f, 100.f, 1.f, 10.f);
   BP_DebugMenu_SetCallback( item, BP_DebugMenu_ActionCallback_ReverbSettings_Changed, 0 );

   item = BP_DebugMenu_AddFloat(reverbMenu, "Density", &sCurrXAudioReverbMode.Density, 0.f, 100.f, 1.f, 10.f);
   BP_DebugMenu_SetCallback( item, BP_DebugMenu_ActionCallback_ReverbSettings_Changed, 0 );

   item = BP_DebugMenu_AddFloat(reverbMenu, "HFReference", &sCurrXAudioReverbMode.HFReference, 20.f, 20000.f, 10.f, 100.f);
   BP_DebugMenu_SetCallback( item, BP_DebugMenu_ActionCallback_ReverbSettings_Changed, 0 );
#endif
}

//----------------------------------------------------------------------------
void BP_SoundSupport_Init_PlatformSpecific()
{
   g_pAudio = new AudioDriver();
   g_pAudio->Initialize();
}

//----------------------------------------------------------------------------
void BP_SoundSupport_ApplyReverbSettings_PlatformSpecific()
{
   g_pAudio->ApplyReverbSetting();
}

//----------------------------------------------------------------------------
void BP_SoundSupport_ApplyDirectOutputStreamsPaused_PlatformSpecific()
{
   for( int ch=0; ch<BP_DOSTREAM_COUNT; ++ch )
   {
      g_pAudio->ApplyPause(ch);
   }
}

//----------------------------------------------------------------------------
void BP_SoundSupport_Shutdown()
{
   g_pAudio->Shutdown();
}

//----------------------------------------------------------------------------
void BP_SoundSupport_SetMasterVolume( float volume )
{
   g_pAudio->SetMasterVolume(volume);
}

//----------------------------------------------------------------------------
void BP_SoundSupport_Update()
{
   static CStopWatch sSoundUpdateStw;
   float soundUpdateDeltaTime = sSoundUpdateStw.GetElapsedTime();
   sSoundUpdateStw.Reset();
   //Must keep track of XMP playback state (polling is easier than event notification)
   //If XMP playback is already going, created SourceVoices aren't automatically muted when started.
   //So we get to do some manual work here!
   BOOL bPlaybackControl = 0;
   if( ERROR_SUCCESS == XMPTitleHasPlaybackControl( &bPlaybackControl ) )
   {
      BOOL bPlayBgmVoices = bPlaybackControl || gBP_XMPBgmOverrideStreams;
      if( bPlayBgmVoices != gBP_PlayBgmVoices )
      {
         //Toggle all voices on or off.
         gBP_PlayBgmVoices = bPlayBgmVoices;

         //Apply changed setting immediately to all.
         for( int core=0; core < 2; ++core )
         {
            for( int voice=0; voice < 24; ++voice )
            {
               SSpuVoiceState & voiceState = g_pAudio->m_spuVoiceState[core][voice];
               EBgmMusicType bgmType = bp_get_spu_voice_bgm_type( core, voice );
               const float masterVolume = 1.f;
               bp_set_bgm_mute_volume( voiceState.mpAudioVoice, bgmType, masterVolume );
               bp_set_bgm_mute_volume( voiceState.mpSpareAudioVoice, bgmType, voiceState.mSpareVoiceEnvelope );
            }
         }

         for( int channel=0; channel < BP_DOSTREAM_COUNT; ++channel )
         {
            AudioDriver::AudioStreamChannel* pAudioStream = g_pAudio->m_audioStreams[channel];

            CSyncCriticalSectionLocker lock(pAudioStream->m_soundSync);
            for( int tr=0; tr<BP_DOSTREAM_TRACK_COUNT; ++tr)
            {
               if (pAudioStream->m_directSoundTracks[tr])
               {
                  const float masterVolume = 1.f;
                  bp_set_bgm_mute_volume( pAudioStream->m_directSoundTracks[tr], pAudioStream->m_bgmType, masterVolume );
               }
            }
         }
      }
   }
#ifdef BP_USE_NEW_ADSR_ENVELOPES
   //tick all ADSR envelopes.
   for( int core=0; core < 2; ++core )
   {
      for( int voice=0; voice < 24; ++voice )
      {
         SSpuVoiceState & voiceState = g_pAudio->m_spuVoiceState[core][voice];
         if( voiceState.mADSREnvelope.mADSRState != kADSRState_Off )
         {
            g_pAudio->ApplyVolume( voiceState );
         }
         g_pAudio->ApplyKeyOffVolume( voiceState, soundUpdateDeltaTime );
      }
   }
#endif
}

//----------------------------------------------------------------------------
unsigned int BP_PlayWaveform( const unsigned char * const waveformData, const int size, const int frequency, const int channelCount )
{
   bool bSendSubmix = false;
   IXAudio2SourceVoice* pSourceVoice = g_pAudio->CreateSourceVoicePCM( channelCount, frequency, NULL, bSendSubmix, kBgmMusicType_Not );

   XAUDIO2_BUFFER firstBuffer = { 0 };

   // Set address and size of data to play
   firstBuffer.Flags = XAUDIO2_END_OF_STREAM;

   firstBuffer.pAudioData = (BYTE*)(waveformData);
   firstBuffer.AudioBytes = size;
   firstBuffer.PlayBegin = 0;
   firstBuffer.PlayLength = size / sizeof(short) / channelCount;

   pSourceVoice->SubmitSourceBuffer(&firstBuffer);
   pSourceVoice->Start();

   return (unsigned int)pSourceVoice;
}

void BP_StopWaveform( const unsigned int handle )
{
   IXAudio2SourceVoice* pSourceVoice = (IXAudio2SourceVoice*)handle;
   pSourceVoice->Stop();
   pSourceVoice->DestroyVoice();

   printf("BP_StopWaveform: %08x\n", handle );
}

void BP_SetWaveformVolume( const unsigned int handle, const float volume )
{
   //NOTE: this supports stereo playback only!
   IXAudio2SourceVoice* pSourceVoice = (IXAudio2SourceVoice*)handle;
   float vols[2*2] = { 0 };
   vols[0] = volume; //channel 0, full volume left.
   vols[3] = volume; //channel 1, full volume rignt.
   pSourceVoice->SetOutputMatrix( NULL, 2, 2, vols );
}

//----------------------------------------------------------------------------
void BP_InitDirectOutputStream(DirectOutputStreamInitData const* pInitInfo)
{
   // WMAUDIO3 is the format for 5.1 compressed xbox audio, WMAUDIO2 is the format for stereo compressed xbox audio
   BPE_ASSERT_NO_MSG( pInitInfo->trackCount > 0 && pInitInfo->trackCount <= BP_DOSTREAM_TRACK_COUNT);
   BPE_ASSERT_NO_MSG( pInitInfo->format == BP_DOSTREAM_FORMAT_PCM || pInitInfo->format == WAVE_FORMAT_WMAUDIO2 ||
      pInitInfo->format == WAVE_FORMAT_WMAUDIO3);
   if( pInitInfo->channel == 2 )
   {
      BPE_ASSERT_NO_MSG( pInitInfo->format == BP_DOSTREAM_FORMAT_PCM );
      //MGS3: playback buffer size for BGM varies based on track count.  If there are fewer than 3 tracks,
      //the playback buffer actually spans multiple BP_DOSTREAM_BUFFER_SIZE elements.
      //The entire playback buffer size for MTA streams is BP_DOSTREAM_TRACK_COUNT the normal size.
      BPE_ASSERT_NO_MSG( pInitInfo->playbackBufferSize <= BP_DOSTREAM_BUFFER_SIZE * BP_DOSTREAM_TRACK_COUNT );
   }
   else
   {
      BPE_ASSERT_NO_MSG( pInitInfo->playbackBufferSize <= BP_DOSTREAM_BUFFER_SIZE );
   }
   g_pAudio->InitSound(pInitInfo);
}

//----------------------------------------------------------------------------
void BP_ContinuePlayDirectOutputStream( const int channel )
{
   g_pAudio->StartSound(channel);
}

//----------------------------------------------------------------------------
void BP_SetDirectOutputStreamPaused( const int channel, int paused)
{
   bool isPaused = paused ? true : false;
   g_pAudio->PauseSound(channel, isPaused);
}

//----------------------------------------------------------------------------
void BP_StopDirectOutputStream( const int channel )
{
   g_pAudio->StopSound(channel);
}

//----------------------------------------------------------------------------
int BP_IsDirectOutputStreamDone( const int channel )
{
   BPE_ASSERT_NO_MSG(channel < BP_DOSTREAM_COUNT);
   return g_pAudio->IsSoundDone(channel);
}

//----------------------------------------------------------------------------
int BP_GetDirectOutputStreamPosition( const int channel )
{
   return g_pAudio->GetSoundPos(channel);
}

//----------------------------------------------------------------------------
double BP_GetDirectOutputStreamPlayTime( const int channel )
{
   BPE_ASSERT_NO_MSG(channel < BP_DOSTREAM_COUNT);
   return g_pAudio->GetSoundPlayTime(channel);
}

//----------------------------------------------------------------------------
void BP_SetDirectOutputStreamVolume_Direct( const int channel, const int track, const float fVol )
{
   BPE_ASSERT( track >= 0 && track < BP_DOSTREAM_MAX_TRACK_VALUE, "" );

   if( track >= BP_DOSTREAM_TRACK_COUNT )
      return;

   g_pAudio->SetTrackVolume_Direct(channel, track, fVol);
}

//----------------------------------------------------------------------------
void BP_SetDirectOutputStreamVolume_Voice( const int channel, int stereoVols[2][2],
                                          float pan3d, float vol3d, BP_SURROUND_VOICE_TYPE surroundType)
{
   float fStereoVols[2][2];
   for( int ch=0; ch<2; ++ch )
   {
      for( int lr=0; lr<2; ++lr )
      {
         fStereoVols[ch][lr] = BP_PS2VolToFVol(stereoVols[ch][lr]);
      }
   }
   g_pAudio->SetTrackVolume_Voice(channel, fStereoVols, pan3d, vol3d, surroundType);
}

void BP_SetDirectOutputStreamDsp_Voice( const int channel, int bDspEnabled )
{
   AudioDriver::AudioStreamChannel* pAudioStream = g_pAudio->m_audioStreams[channel];
   pAudioStream->m_bReverbEnabled = bDspEnabled!=0;
   g_pAudio->ApplyReverbEnabled_StreamVoice( channel );
}

//----------------------------------------------------------------------------
unsigned char * BP_GetDirectOutputStreamPlaybackBuffer( const int channel )
{
   BPE_ASSERT_NO_MSG( channel >= 0 && channel < BP_DOSTREAM_COUNT );
#if MGS_VERSION==3
   if( channel == 2 )
   {
      //BGM MTA stream.
      return gMTAStreamPlaybackBuffer;
   }
#endif
   //N.B. this case is for BP_AudioStreams, which are on X360 the only direct output streams that
   //should be on a channel other than 2.  Because all of these streams use the vbr compressed audio
   //now, they do not actually use this playback buffer.
   return NULL;
}

//----------------------------------------------------------------------------

int BP_GetDirectOutputStreamReadAheadSize( const int channel )
{
   AudioDriver::AudioStreamChannel* pStream = g_pAudio->m_audioStreams[channel];

   if(pStream->m_format == BP_DOSTREAM_FORMAT_PCM)
   {
      return pStream->m_currBufferSize;
   }
   else
   {
      int const internalReadAheadSize = (XWMA_NUM_ENCODED_PACKETS / 2) * pStream->m_pDecoder->XWMAPacketByteSize;
      int const audioStreamPlaybackBufferSize = pStream->m_currBufferSize;

      return (internalReadAheadSize < audioStreamPlaybackBufferSize) ? internalReadAheadSize : audioStreamPlaybackBufferSize;
   }
}

//----------------------------------------------------------------------------
void BP_NotifyDirectOutputStreamBytesAvailable(int channel, void* pBuffer, int sizeBytes)
{
   g_pAudio->NotifySoundBytesAvailable(channel, pBuffer, sizeBytes);
}

//----------------------------------------------------------------------------
int BP_sceSdVoiceTrans(short channel, unsigned short mode, unsigned char *m_addr, unsigned int _s_addr, 
   unsigned int size)
{
   // On the xbox we need to decode the vag data into linear pcm in addition to copying into the SPU buffer.
   // We use the emulated spu memory buffer to support the other vag related functionality.
   void* const pDst = &gBP_SpuMemory[0] + _s_addr;
   memcpy(pDst, m_addr, size);

   BPE_ASSERT( (size & (BP_VAG_PACKET_SIZE-1)) == 0, "unaligned waveform data transfer size" );
   int numVagPackets = size / BP_VAG_PACKET_SIZE;
   int decompressedSize = numVagPackets * BP_PCM_SIZE_FROM_VAG_SIZE;

   //N.B. address does not have to be aligned to 16 bytes, but the address must at least convert to decompressed
   //address cleanly.  (e.g. an odd address * (28*sizeof(short))/16 doesn't work.)
   int pcmAddress = _s_addr * BP_PCM_SIZE_FROM_VAG_SIZE / BP_VAG_PACKET_SIZE;
   BPE_ASSERT( pcmAddress * BP_VAG_PACKET_SIZE / BP_PCM_SIZE_FROM_VAG_SIZE == _s_addr, "unaligned waveform data transfer address" );

   char* pUnCompressBufferStart = (char*)(&gDecompressedVagBuffer[0] + pcmAddress);

   BPE_ASSERT((pUnCompressBufferStart+decompressedSize) < ((char*)(&gDecompressedVagBuffer[0]) + sizeof(gDecompressedVagBuffer)),
      "Buffer overflow detected while decompressing vag packets.");

   //Fetch decompression history pointers from buffer state.  This is needed to properly decode a continuous
   //stream across multiple transfers.
   double * pS1 = NULL;
   double * pS2 = NULL;
   for( int i=0; i < 2; ++i )
   {
      for( int lr=0; lr < 2; ++lr )
      {
         SADPCMStreamBufferState & bufferState = sADPCMStreamBufferState[i][lr];
         if( _s_addr >= bufferState.mSpuAddr && (_s_addr + size) <= bufferState.mSpuAddr+bufferState.mSpuSize )
         {
            pS1 = &bufferState.mS1;
            pS2 = &bufferState.mS2;
            break;
         }
      }
   }

#ifdef BP_DECODE_SPU_AFTER_ALL_TRANSFER
   if( gBP_DecodeSpuMemoryOnTransfer )
#endif
   {
      //This is presumably a streaming buffer.
      //Decompress this entire block of memory as a single waveform using the appropriate history values.
      BP_DecompressVAG(m_addr, numVagPackets, pUnCompressBufferStart, pS1, pS2);
   }

//   printf("BP_sceSdVoiceTrans: %d -> %d (sz %d)\n", _s_addr, _s_addr+size, size );
   return size;
}

//----------------------------------------------------------------------------
void BP_DecodeSpuMemory( unsigned int _s_addr, unsigned int size )
{
#ifdef BP_DECODE_SPU_AFTER_ALL_TRANSFER
   int end_addr = _s_addr + size;

//   printf("BP_DecodeSpuMemory: %d -> %d (sz %d)\n", _s_addr, _s_addr+size, size );

   BPE_ASSERT_NO_MSG( ( _s_addr >= 0 ) && (end_addr <= BP_SPU_MEMORY_SIZE ) );

#ifdef BP_DYNAMIC_LOOP_BUFFERS
   //First, destroy all the looped data buffers in this range.
   TSpuLoopBufferMap::iterator beginRange = gSpuLoopBufferMap.lower_bound(_s_addr);
   TSpuLoopBufferMap::iterator endRange = gSpuLoopBufferMap.upper_bound(_s_addr+size);
   for( TSpuLoopBufferMap::iterator it = beginRange; it != endRange; ++it )
   {
      it = gSpuLoopBufferMap.erase( it );
      --gBP_Sound_CreatedLoopBuffers;
   }
#endif

   int last_ssa = _s_addr;
   int last_loop = 0;
   bool bLooping = false;
   for( int curr_addr = _s_addr; curr_addr < end_addr; curr_addr += BP_VAG_PACKET_SIZE )
   {
      const unsigned char * vagPacket = &gBP_SpuMemory[curr_addr];
      const unsigned char flag = vagPacket[1];
      if( flag & VAG_LOOPBLOCK )
      {
         bLooping = true;
      }
      if( flag & VAG_LOOPSTART )
      {
         last_loop = curr_addr;
      }
      if( flag & VAG_LOOPEND )
      {
         //End of a waveform.  Decode this waveform and generate a looped data buffer.
         int next_ssa = curr_addr+BP_VAG_PACKET_SIZE;
         int numVagPackets = (next_ssa - last_ssa) / BP_VAG_PACKET_SIZE;
         const unsigned char * const spuAddress = &gBP_SpuMemory[last_ssa];
         static const unsigned char skEndMarkerPacket[16] = {0x00,0x07,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77};
         if( numVagPackets > 1 || memcmp( spuAddress, skEndMarkerPacket, sizeof(skEndMarkerPacket) ) )
         {
            int pcmAddress = last_ssa * BP_PCM_SIZE_FROM_VAG_SIZE / BP_VAG_PACKET_SIZE;
            BPE_ASSERT( pcmAddress * BP_VAG_PACKET_SIZE / BP_PCM_SIZE_FROM_VAG_SIZE == last_ssa, "unaligned waveform data transfer address" );
            char* pUnCompressBufferStart = (char*)(&gDecompressedVagBuffer[0] + pcmAddress);
            printf(" BP_DecompressVAG: %d -> %d (sz %d) Looping %d\n", last_ssa, next_ssa, next_ssa-last_ssa, bLooping ? 1 : 0 );
            double s1 = 0;
            double s2 = 0;
            BP_DecompressVAG(spuAddress, numVagPackets, pUnCompressBufferStart, &s1, &s2 );
            if( bLooping )
            {
#ifdef BP_DYNAMIC_LOOP_BUFFERS
               //Build a looped data buffer for clean looping playback.
               BPE_ASSERT( last_loop >= last_ssa, "Error: no loop begin marker since last waveform start!" );
               //The entire buffer contains the first loop iteration.  We want at least 256 (ADPCM) bytes of repeating data total
               //too smooth out any history in the loop playback.
               int loop_spu_size = next_ssa - last_loop;
               int loop_spu_data_size = loop_spu_size;
               while( loop_spu_data_size < 256 )
               {
                  loop_spu_data_size += loop_spu_size;
               }

               int loop_pcm_data_size = loop_spu_data_size * BP_PCM_SIZE_FROM_VAG_SIZE / BP_VAG_PACKET_SIZE;
               BPE_ASSERT( gSpuLoopBufferMap.find( last_ssa ) == gSpuLoopBufferMap.end(), "stray loop buffer left over!" );
               TSpuLoopBuffer & loopBuffer = gSpuLoopBufferMap[last_ssa];
               loopBuffer.assign( loop_pcm_data_size, 0 );
               char * pUncompressLoopBufferStart = &loopBuffer[0];
               const unsigned char * const spuLoopAddress = &gBP_SpuMemory[last_loop];
               const int numPacketsPerLoop = loop_spu_size / BP_VAG_PACKET_SIZE;
               for( int loop=0; loop < loop_spu_data_size; loop += loop_spu_size )
               {
                  char * const pUncompressLoopBuffer = pUncompressLoopBufferStart + loop * BP_PCM_SIZE_FROM_VAG_SIZE / BP_VAG_PACKET_SIZE; 
                  BP_DecompressVAG(spuLoopAddress, numPacketsPerLoop, pUncompressLoopBuffer, &s1, &s2 );
               }

               ++gBP_Sound_CreatedLoopBuffers;
#endif
            }
         }
         //else skip over useless magic end marker.

         bLooping = false;
         last_ssa = next_ssa;
      }
   }
#endif
}

//----------------------------------------------------------------------------
int BP_sceSdVoiceTransStatus (short channel, short flag)
{
   return 1;
}

//----------------------------------------------------------------------------
void BP_sceSdSetParam(unsigned short entry, unsigned short value)
{
   int reg = entry & 0xffc0;
   int core = entry & 1;
   int voice = ( entry & ~0xffc0 ) >> 1;
   BPE_ASSERT(voice < 24, "Invalid voice!");

   g_pAudio->SetVoiceParam(reg, core, voice, value);
}

//----------------------------------------------------------------------------
unsigned short BP_sceSdGetParam(unsigned short entry)
{
   int reg = entry & 0xffc0;
   int core = entry & 1;
   int voice = ( entry & ~0xffc0 ) >> 1;
   BPE_ASSERT(voice < 24, "Invalid voice!");

   return g_pAudio->GetVoiceParam(reg, core, voice);
}

//----------------------------------------------------------------------------
void BP_sceSdSetAddr(unsigned short entry, unsigned int value)
{
   int reg = entry & 0xffc0;
   int core = entry & 1;
   int voice = ( entry & ~0xffc0 ) >> 1;
   BPE_ASSERT(voice < 24, "Invalid voice!");

   g_pAudio->SetVoiceSourceAddr(reg, core, voice, value);
}

//----------------------------------------------------------------------------
unsigned int BP_sceSdGetAddr(unsigned short entry)
{
   int reg = entry & 0xffc0;
   int core = entry & 1;
   int voice = ( entry & ~0xffc0 ) >> 1;
   BPE_ASSERT(voice < 24, "Invalid voice!");

   switch (reg)
   {
   case SD_VA_NAX:
      return g_pAudio->GetVoicePos(core, voice);

   default:
      printf("Unimplemented GetAddr(%d)\n", reg);
      return 0;
   }
}

//----------------------------------------------------------------------------
void BP_sceSdSetSwitch( unsigned short entry, unsigned int value )
{
   int reg = entry & 0xffc0;
   int core = entry & 1;

   switch( reg )
   {
   case SD_S_KOFF:
      {
         //key off (end voice generation)
         for(int i=0; i < 24; ++i)
         {
            int voiceBit = 1 << i;
            if(value & voiceBit)
            {
               SSpuVoiceState & voiceState = g_pAudio->m_spuVoiceState[core][i];
#ifndef BP_USE_NEW_ADSR_ENVELOPES
               g_pAudio->StopVoice(voiceState);
#else
               if( voiceState.mADSREnvelope.mADSRState != kADSRState_Off )
               {
                  //Key off with release.
                  voiceState.mADSREnvelope.mADSRState = kADSRState_Release;
               }
#endif
            }
         }
      }
      break;

   case SD_S_KON:
      {
         //key on (start voice generation)
         for(int i=0; i < 24; ++i)
         {
            int voiceBit = 1 << i;
            if( value & voiceBit )
            {
               g_pAudio->StartVoice(core, i);
            }
         }
      }
      break;

   case SD_S_NON:
      {
         //allocation to noise generator
         if( value != 0 )
         {
            //use the noise generator for this voice
            printf("Unimplemented voice switch: SD_S_NON\n");
         }
      }
      break;

   case SD_S_PMON:
      {
         //pitch modulation per voice
         if( value != 0 )
         {
            printf("Pitch modulation is currently unsupported.\n");
         }
      }
      break;

   case SD_S_VMIXEL:
   case SD_S_VMIXER:
      {
         //Voice output mixing flag per voice (wet left)
         //Voice output mixing flag per voice (wet right)
         for( int i=0; i < 24; ++i )
         {
            SSpuVoiceState & voiceState = g_pAudio->m_spuVoiceState[core][i];

            int voiceBit = 1 << i;
            bool bReverbEnabled = ( value & voiceBit ) != 0;
            if( reg == SD_S_VMIXEL )
            {
               voiceState.mbReverbEnabledL = bReverbEnabled;
            }
            else
            {
               //TED - they always set SD_S_VMIXEL / SD_S_VMIXER together, in that order,
               //and to the same value.  Apply once as a flag for the entire channel
               //rather than "properly" implementing via separate L/R wet volumes.
               if( bReverbEnabled != voiceState.mbReverbEnabledL )
                  BP_TODO_BREAK;

               voiceState.mbReverbEnabledR = bReverbEnabled;
               g_pAudio->ApplyVoiceReverb( voiceState );
            }
         }
      }
      break;

   default:
      printf("Unimplemented voice switch flag. %d\n", reg);
      break;
   }
}

//----------------------------------------------------------------------------
void BP_sceSdSetCoreAttr( unsigned short entry, unsigned short value )
{
   int reg = entry & 0xfffe;
   int core = entry & 1;
   switch (reg)
   {
   case SD_C_NOISE_CLK:
      //noise generator
      break;
   case SD_C_EFFECT_ENABLE:
      {
         //Enable / disable writing to effect work area.  Presumably we don't need this hook!
      }
      break;

   default:
      printf("Unimplemented voice SetCoreAttr flag. %d\n", reg);
      break;
   }
}

//----------------------------------------------------------------------------
unsigned short BP_sceSdNote2Pitch (unsigned short center_note, unsigned short center_fine, 
   unsigned short note, short fine)
{
   const float BaseA4 = 4096.0f;
   const float NumSemitones = 128.0f;
   const float NumOctaves = 1.0f / 12.0f;

   float noteDiff = (float)(note - center_note);
   float centerFine = center_fine / 128.0f;
   float noteFine = fine / 128.0f;
   float fineDiff = fabsf(centerFine - noteFine);

   unsigned short pitch = (unsigned short)(BP_Float_Floor(BaseA4 * powf(2.0f, (noteDiff + fineDiff) * NumOctaves) + 0.5f));
   return pitch;
}

//----------------------------------------------------------------------------
void BP_SoundSupport_SurroundSoundSetVoiceType( const int core, const int voice, BP_SURROUND_VOICE_TYPE voiceType )
{
   BPE_ASSERT( core >= 0 && core < 2, "" );
   BPE_ASSERT( voice >= 0 && voice < 24, "" );

   g_pAudio->SurroundSoundSetVoiceType( core, voice, voiceType );
}

//----------------------------------------------------------------------------
void BP_SoundSupport_SurroundSoundSetVoicePanVol( const int core, const int voice, float pan, float vol )
{
   BPE_ASSERT( core >= 0 && core < 2, "" );
   BPE_ASSERT( voice >= 0 && voice < 24, "" );

   g_pAudio->SurroundSoundSetPanVol( core, voice, pan, vol );
}

//----------------------------------------------------------------------------
void BP_SoundSupport_ApplyVoiceVolume( const int core, const int voice )
{
   BPE_ASSERT( core >= 0 && core < 2, "" );
   BPE_ASSERT( voice >= 0 && voice < 24, "" );
   SSpuVoiceState & voiceState = g_pAudio->m_spuVoiceState[core][voice];
   g_pAudio->ApplyVolume( voiceState );
}

//----------------------------------------------------------------------------
void BP_SoundSupport_InitADPCMStreamingBuffer( const int channel, const int lr, const int spuAddr, const int spuSize )
{
   BPE_ASSERT( channel == 0 || channel == 1, "only 2 streaming channels available" );
   BPE_ASSERT( lr == 0 || lr == 1, "only stereo supported" );
   SADPCMStreamBufferState & bufferState = sADPCMStreamBufferState[channel][lr];
   bufferState.mSpuAddr = spuAddr;
   bufferState.mSpuSize = spuSize;
   bufferState.mS1 = 0;
   bufferState.mS2 = 0;
}

//----------------------------------------------------------------------------
void BP_PlayMemStream(const int core, const int voice, const void * const playbackBuffer, const int playbackBufferSize)
{
   g_pAudio->PlayMemoryStream(core, voice, playbackBuffer, playbackBufferSize);
}

//----------------------------------------------------------------------------
int BP_GetMemStreamPosition(const int core, const int voice)
{
   int pos = g_pAudio->GetVoicePos(core, voice);
   return pos;
}

//----------------------------------------------------------------------------
void BP_SetMemStreamMuted( const int core, const int voice, const int muted )
{
   g_pAudio->MuteMemoryStream(core, voice, muted);
}

//----------------------------------------------------------------------------

void BP_SoundSupport_ShowStreamStats_PlatformSpecific()
{
   for( int i = 0; i < BP_DOSTREAM_COUNT; ++i )
   {
      AudioDriver::AudioStreamChannel* pChannel = g_pAudio->m_audioStreams[i];

      BP_DebugText_Print("Stream[%d]: State: %d Read Pos: %d Notified: %d BytesReadNoPadding: %d Total Size: %d (Not-Read): %d XWM Avail: %d", 
         i, 
         pChannel->m_currSoundState, 
         pChannel->m_xwmReadPos, 
         pChannel->m_bytesNotified, 
         pChannel->m_numXWMBytesReadNoPadding,
         pChannel->m_totalStreamSize,
         pChannel->m_bytesNotified - pChannel->m_xwmReadPos, 
         pChannel->m_numXWMBytesAvail);
   }
}

void BP_SoundSupport_ShowVoiceStats_PlatformSpecific()
{
   for( int core=0; core < 2; ++core )
   {
      for( int voice=0; voice < 24; ++voice )
      {
         static const char kADSRChar[] = { '0', 'A', 'D', 'S', 'R' };
         SSpuVoiceState & voiceState = g_pAudio->m_spuVoiceState[core][voice];

         //N.B. we're in the render thread so we can't call GetVoicePos() which calls a thread-unsafe X360 APT.
         BP_DebugText_Print("[%d][%2d]: %8d %c %f\n", core, voice, voiceState.mLastSampledVoicePos, kADSRChar[voiceState.mADSREnvelope.mADSRState], voiceState.mADSREnvelope.mEnvelopeScale );
      }
   }
}
