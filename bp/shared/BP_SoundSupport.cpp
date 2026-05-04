//----------------------------------------------------------------------------
// BP_SoundSupport.cpp
//----------------------------------------------------------------------------

#include "Engine/StdAfx.h"

#include "Engine/System/CStopWatch.h"
#include "Engine/Graphics/CColor.h"
#include "BP_RendererDebug.h"

#include "BP_BuildDefines.h"

#include "BP_EndianSupport.h"
#include "BP_SoundSupport.h"
#include "BP_SurroundSound.h"
#include "BP_Debug.h"
#include "BP_AudioStream.h"

#include "libsd.h"

#include "MGS_Common.h"

extern "C" int DG_TickCount;
unsigned char gBP_SpuMemory[BP_SPU_MEMORY_SIZE];

int              BP_EnableMtaStreams = 1;

extern "C"
{
   int           gBP_DecodeSpuMemoryOnTransfer = 1;

   int   BP_EnableVagStreams = 1;
   int   BP_EnableMidiBgm = 1;
   int   BP_EnableSfx = 1;
#ifdef TED
   int   BP_SpewSfxPlayed = 1;
#else
   int   BP_SpewSfxPlayed = 0;
#endif
}

int               gBP_DirectOutputStreamSpeed = 1;
int               gBP_DirectOutputStreamIsDemo = 0;

//MGS3: multi-track support is only for BGM's benefit.  BGM is always channel 2 (the last channel).
//makes it convenient for buffer organization!  The first two buffers are for channel 0 and 1,
//the last three buffers are for channel 2.
unsigned char     gDirectOutputStreamPlaybackBuffer[BP_DOSTREAM_COUNT+BP_DOSTREAM_TRACK_COUNT-1][BP_DOSTREAM_BUFFER_SIZE];

int               gBP_ShowSoundStats = 0;
int               gBP_ShowStreamStats = 0;
int               gBP_ShowVoiceStats = 0;
int               gBP_Sound_CreatedVoices = 0;
int               gBP_Sound_CreatedMemStreams = 0;
int               gBP_Sound_CreatedLoopBuffers = 0;
int               gBP_EnableSfxWavReplacements = 1;

int gbSoundSupportSurroundSound = 0;
int gSoundSupportSurroundSoundChannelCount = 2;
int gDirectOutputStreamsPaused = 0;

//6-channel surround upmixes
static SBP2_8ChannelUpmix gBP_SeUpmix_6 = { 0.707f, 0.5f, 0.5f, 0.4f };
static SBP2_8ChannelUpmix gBP_StreamStereoUpmix_6 = { 0.707f, 0.5f, 0.4f, 0.f };

//8-channel surround upmixes
static SBP2_8ChannelUpmix gBP_SeUpmix_8 = { 0.707f, 0.5f, 0.5f, 0.4f };
static SBP2_8ChannelUpmix gBP_StreamStereoUpmix_8 = { 0.707f, 0.5f, 0.4f, 0.3f };
static SBP6_8ChannelUpmix gBP_Stream51Upmix_8 = { 1.f, (float)(0.4 / 0.3) };

SBP2_8ChannelUpmix * gpBP_SeUpmix = &gBP_SeUpmix_8;
SBP2_8ChannelUpmix * gpBP_StreamStereoUpmix = &gBP_StreamStereoUpmix_8;
SBP6_8ChannelUpmix * gpBP_Stream51Upmix = &gBP_Stream51Upmix_8;

int gReverbModePS2 = 0;
int gReverbDepthPS2 = 0;

unsigned int gBP_XMPBgmOverrideStreams = 0;
unsigned int gBP_PlayBgmVoices = 1;

static const bool gkReverbModePS2Supported[] =
{
   true,
   false,
   false,
   false,
   true,    //STUDIO_C - Many indoor areas in MGS3
   true,    //HALL - MGS2's only reverb mode
   true,    //SPACE - Sorrow fight (s141a) in MGS3
   false,
   false,
   false,
};

static const char * const skReverbModeNamesPS2[] =
{
   "OFF",
   "ROOM",
   "STUDIO_A",
   "STUDIO_B",
   "STUDIO_C",
   "HALL",
   "SPACE",
   "ECHO",
   "DELAY",
   "PIPE"
};

extern void BP_SoundSupport_Init_PlatformSpecific();
extern void BP_SoundSupport_InitDebugMenuPlatformSpecific( int soundMenu, int reverbMenu );
extern void BP_SoundSupport_ShowStreamStats_PlatformSpecific();
extern void BP_SoundSupport_ShowVoiceStats_PlatformSpecific();
extern void BP_SoundSupport_ApplyReverbSettings_PlatformSpecific();
extern void BP_SoundSupport_ApplyDirectOutputStreamsPaused_PlatformSpecific();

//----------------------------------------------------------------------------

int BP_SoundSupport_IsSurroundSound()
{
   return gbSoundSupportSurroundSound;
}

int BP_SoundSupport_GetSurroundSoundChannelCount()
{
   return gSoundSupportSurroundSoundChannelCount;
}

//----------------------------------------------------------------------------

void BP_SoundSupport_Init()
{
   BP_AudioStreamInit();
   BP_SoundSupport_Init_PlatformSpecific();

   if( gSoundSupportSurroundSoundChannelCount == 6 )
   {
      //Update mix tweaks.
      gpBP_SeUpmix = &gBP_SeUpmix_6;
      gpBP_StreamStereoUpmix = &gBP_StreamStereoUpmix_6;
   }
}

void * AllocSysMemory( int type, unsigned long size, void * addr )
{
   return malloc( size );
}

int FreeSysMemory( void *area )
{
   free(area);
   return 0;
}

#if !defined(BP_360)

int BP_sceSdVoiceTrans( short channel, unsigned short mode, unsigned char *m_addr, unsigned int s_addr, unsigned int size )
{
   if( mode != ( SD_TRANS_MODE_WRITE | SD_TRANS_BY_DMA ) )
      BP_TODO_BREAK;

   //Just a memcpy!
   void * const pDst = &gBP_SpuMemory[0]+s_addr;
   memcpy( pDst, m_addr, size );
   return size;
}

int BP_sceSdVoiceTransStatus (short channel, short flag)
{
   return 1;
}

void BP_DecodeSpuMemory( unsigned int _s_addr, unsigned int size )
{
}

#endif

int BP_sceSdSetEffectAttr( int core, sceSdEffectAttr *attr )
{
   BPE_ASSERT( core >= 0 && core < 2, "" );
   int mode = attr->mode & ~SD_REV_MODE_CLEAR_WA;
   BPE_ASSERT( mode >= 0 && mode < SD_REV_MODE_MAX, "" );

   if( !gkReverbModePS2Supported[mode] )
   {
      //Make sure we don't miss any we need to create settings for.
      BP_TODO_BREAK;
   }

   if( attr->depth_R != attr->depth_L )
      BP_TODO_BREAK;

   gReverbModePS2 = mode;
   gReverbDepthPS2 = attr->depth_R;

   BP_SoundSupport_ApplyReverbSettings_PlatformSpecific();

   return SCESD_OK;
}

unsigned int BP_SdGetUTime( void )
{
   //N.B. sdlib source code is aware that 32 bits worth of microseconds wraps around every 71 minutes.
   const uint64 us = CStopWatch::gGlobalTime.GetElapsedMicroSeconds();
   return (unsigned int)us;
}

unsigned int BP_SdGetTime( void )
{
   const uint32 ms = CStopWatch::gGlobalTime.GetElapsedMilliseconds();
   return ms;
}

void BP_SetAllDirectOutputStreamPaused( int paused )
{
   gDirectOutputStreamsPaused = paused;
   BP_SoundSupport_ApplyDirectOutputStreamsPaused_PlatformSpecific();
}

#if !defined(BP_PS3) && !defined(BP_360) && !defined(BP_VITA)

void BP_SoundSupport_Init_PlatformSpecific()
{
}

void BP_SoundSupport_InitDebugMenuPlatformSpecific( int soundMenu, int reverbMenu )
{
}

void BP_SoundSupport_ShowStreamStats_PlatformSpecific()
{
}

void BP_SoundSupport_ShowVoiceStats_PlatformSpecific()
{
}

void BP_SoundSupport_ApplyReverbSettings_PlatformSpecific()
{
}

void BP_SoundSupport_ApplyDirectOutputStreamsPaused_PlatformSpecific()
{
   //handled on the fly.
}

//----------------------------------------------------------------------------

typedef struct _SSpuVoiceStateWin
{
   unsigned int      mSSA;
   unsigned int      mLSAX;
   unsigned int      mBlockBegin;
   unsigned int      mBlockEnd;
   bool              mbBlockLoop;
   unsigned int      mNAX;
   unsigned short    mPitch;
   int               mDGTickCount;
   bool              mbKeyOn;

   //If not NULL, this replaces SSA as the source of the waveform playback.
   const unsigned char * mpMemoryStream;
   int               mMemoryStreamSize;
   int               mMemoryStreamPosition;
} SSpuVoiceState;

typedef struct _SDirectOutputStreamStateWin
{
   int               mTrackCount;
   int               mChannelCount;
   int               mPlaybackBufferSize;
   int               mPlaybackPosition;
   int               mFrequency;
   bool              mbPlaying;
   bool              mbPaused;
   int               mDGTickCount;
   int               mCurrentPlayedDGTicks;
} SDirectOutputStreamState;

static SSpuVoiceState sSpuVoiceState[2][24];
static SDirectOutputStreamState sDirectOutputStreamState[BP_DOSTREAM_COUNT];

//----------------------------------------------------------------------------

static void get_vag_endpoints( SSpuVoiceState & voiceState )
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
         if( voiceState.mpMemoryStream )
         {
            BP_TODO_BREAK; //There shouldn't be such a thing as a looping mem stream...
         }
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
   {
      printf("Warning: could not get vag endpoint!\n");
      BP_TED_BREAK;
   }
}

static void update_voice_nax( SSpuVoiceState & voiceState )
{
   //On non-sound playing platforms, emulate by checking DG_TickCount.
   if( voiceState.mbKeyOn )
   {
      //advance play position only if the key is on.
      int last = voiceState.mDGTickCount;
      voiceState.mDGTickCount = DG_TickCount;
      int diffTicks = voiceState.mDGTickCount - last;
      float fPitch = (float)((48000.0 * voiceState.mPitch)/4096);
      // N.B. VAG packets are 16 bytes / 28 samples.
      int diffBytes = (int)( fPitch * diffTicks * 16 / 28 / (300/BASE_TICK) );
      voiceState.mNAX += diffBytes;

      int loopSize = voiceState.mBlockEnd - voiceState.mBlockBegin;
      while( voiceState.mNAX >= voiceState.mBlockEnd )
      {
         if( !voiceState.mbBlockLoop || (loopSize==0) )
         {
            //non-looping stops now. (muted by hardware)
            voiceState.mNAX = voiceState.mBlockEnd;
            break;
         }

         voiceState.mNAX -= loopSize;
      }
   }
}

//----------------------------------------------------------------------------

void BP_SoundSupport_Shutdown()
{
}

void BP_SoundSupport_SetMasterVolume( float volume )
{
}

void BP_SoundSupport_Update()
{
}

unsigned int BP_PlayWaveform( const unsigned char * const waveformData, const int size, const int frequency, const int channelCount )
{
   return 1;
}

void BP_StopWaveform( const unsigned int handle )
{
}

void BP_SetWaveformVolume( const unsigned int handle, const float volume )
{
}

void BP_InitDirectOutputStream( DirectOutputStreamInitData const* pInitInfo )
{
   BPE_ASSERT_NO_MSG( pInitInfo->trackCount > 0 && pInitInfo->trackCount <= BP_DOSTREAM_TRACK_COUNT );
   BPE_ASSERT( pInitInfo->format == BP_DOSTREAM_FORMAT_PCM, "TODO" );
   if( pInitInfo->channel == 2 )
   {
      //MGS3: playback buffer size for BGM varies based on track count.  If there are fewer than 3 tracks,
      //the playback buffer actually spans multiple BP_DOSTREAM_BUFFER_SIZE elements.
      //The entire playback buffer size for MTA streams is BP_DOSTREAM_TRACK_COUNT the normal size.
      BPE_ASSERT_NO_MSG( pInitInfo->playbackBufferSize <= BP_DOSTREAM_BUFFER_SIZE * BP_DOSTREAM_TRACK_COUNT );
   }
   else
   {
      BPE_ASSERT_NO_MSG( pInitInfo->playbackBufferSize <= BP_DOSTREAM_BUFFER_SIZE );
   }

   BP_StopDirectOutputStream(pInitInfo->channel);

   sDirectOutputStreamState[pInitInfo->channel].mTrackCount = pInitInfo->trackCount;
   sDirectOutputStreamState[pInitInfo->channel].mChannelCount = pInitInfo->channelCount;
   sDirectOutputStreamState[pInitInfo->channel].mPlaybackBufferSize = pInitInfo->playbackBufferSize;
   sDirectOutputStreamState[pInitInfo->channel].mPlaybackPosition = 0;
   sDirectOutputStreamState[pInitInfo->channel].mFrequency = pInitInfo->freq;
   sDirectOutputStreamState[pInitInfo->channel].mbPlaying = false;
   sDirectOutputStreamState[pInitInfo->channel].mbPaused = false;
}

void BP_ContinuePlayDirectOutputStream( const int channel )
{
   if( !sDirectOutputStreamState[channel].mbPlaying )
   {
      sDirectOutputStreamState[channel].mbPlaying = true;
      sDirectOutputStreamState[channel].mDGTickCount = DG_TickCount;
      sDirectOutputStreamState[channel].mCurrentPlayedDGTicks = 0;
   }
}

void BP_SetDirectOutputStreamPaused( const int channel, int paused )
{
   sDirectOutputStreamState[channel].mbPaused = paused ? true : false;
}

void BP_StopDirectOutputStream( const int channel )
{
   sDirectOutputStreamState[channel].mbPlaying = false;
}

extern int BP_IsDirectOutputStreamDone( const int channel )
{
   BP_TODO_BREAK;
   return 0;
}

int BP_GetDirectOutputStreamPosition( const int channel )
{
   if( sDirectOutputStreamState[channel].mbPlaying && !sDirectOutputStreamState[channel].mbPaused && !gDirectOutputStreamsPaused )
   {
      //advance play position only if the stream is playing.
      int last = sDirectOutputStreamState[channel].mDGTickCount;
      sDirectOutputStreamState[channel].mDGTickCount = DG_TickCount;
      int diffTicks = sDirectOutputStreamState[channel].mDGTickCount - last;
      sDirectOutputStreamState[channel].mCurrentPlayedDGTicks += diffTicks;
      float streamSpeed = 1.0f;//gBP_DirectOutputStreamSpeed is applied to delta tick count
      int diffSamples = (int)(streamSpeed * sDirectOutputStreamState[channel].mFrequency * diffTicks / (300/BASE_TICK));
      int diffBytes = (diffSamples * sizeof(short) * sDirectOutputStreamState[channel].mChannelCount);
      sDirectOutputStreamState[channel].mPlaybackPosition += diffBytes;

      while( sDirectOutputStreamState[channel].mPlaybackPosition >= sDirectOutputStreamState[channel].mPlaybackBufferSize )
      {
         sDirectOutputStreamState[channel].mPlaybackPosition -= sDirectOutputStreamState[channel].mPlaybackBufferSize;
      }
      return sDirectOutputStreamState[channel].mPlaybackPosition;
   }
   else
   {
      return 0;
   }
}

double BP_GetDirectOutputStreamPlayTime( const int channel )
{
   double playTime = (double)sDirectOutputStreamState[channel].mCurrentPlayedDGTicks / (300/BASE_TICK);
   return playTime;
}

void BP_SetDirectOutputStreamVolume_Direct( const int channel, const int track, const float fVol )
{
   BPE_ASSERT_NO_MSG( channel >= 0 && channel < BP_DOSTREAM_COUNT );
   BPE_ASSERT( track >= 0 && track < BP_DOSTREAM_MAX_TRACK_VALUE, "" );
}

void BP_SetDirectOutputStreamVolume_Voice( const int channel, int stereoVols[2][2],
                                          float pan3d, float vol3d, BP_SURROUND_VOICE_TYPE surroundType)
{
   BPE_ASSERT_NO_MSG( channel >= 0 && channel < BP_DOSTREAM_COUNT );
}

void BP_SetDirectOutputStreamDsp_Voice( const int channel, int bDspEnabled )
{
   BPE_ASSERT_NO_MSG( channel >= 0 && channel < BP_DOSTREAM_COUNT );
}

unsigned char * BP_GetDirectOutputStreamPlaybackBuffer( const int channel )
{
   BPE_ASSERT_NO_MSG( channel >= 0 && channel < BP_DOSTREAM_COUNT );
   //N.B. the last channel "spans" BP_DOSTREAM_TRACK_COUNT buffers worth.
   return gDirectOutputStreamPlaybackBuffer[channel];
}

int BP_GetDirectOutputStreamReadAheadSize( const int channel )
{
   return sDirectOutputStreamState[channel].mPlaybackBufferSize;
}

void BP_NotifyDirectOutputStreamBytesAvailable(int channel, void* pBuffer, int sizeBytes)
{
   (void)channel;
   (void)sizeBytes;
   (void)pBuffer;
}

void BP_PlayMemStream( const int core, const int voice, const void * const playbackBuffer, const int playbackBufferSize )
{
   BPE_ASSERT( core < 2 && voice < 24, "" );
   SSpuVoiceState & voiceState = sSpuVoiceState[core][voice];
   voiceState.mpMemoryStream = (const unsigned char*)playbackBuffer;
   voiceState.mMemoryStreamSize = playbackBufferSize;
   voiceState.mMemoryStreamPosition = 0;
}

int BP_GetMemStreamPosition(const int core, const int voice)
{
   BPE_ASSERT( core < 2 && voice < 24, "" );
   SSpuVoiceState & voiceState = sSpuVoiceState[core][voice];
   if( voiceState.mpMemoryStream && voiceState.mbKeyOn )
   {
      //advance play position only if the key is on.
      int last = voiceState.mDGTickCount;
      voiceState.mDGTickCount = DG_TickCount;
      int diffTicks = voiceState.mDGTickCount - last;
      float fPitch = (float)((48000.0 * voiceState.mPitch)/4096);
      // N.B. VAG packets are 16 bytes / 28 samples.
      int diffBytes = (int)( fPitch * diffTicks * 16 / 28 / (300/BASE_TICK) );
      voiceState.mMemoryStreamPosition += diffBytes;

      int loopSize = voiceState.mBlockEnd - voiceState.mBlockBegin;
      while( voiceState.mMemoryStreamPosition >= voiceState.mBlockEnd )
      {
         if( !voiceState.mbBlockLoop || (loopSize==0) )
         {
            //non-looping stops now. (muted by hardware)
            voiceState.mMemoryStreamPosition = voiceState.mBlockEnd;
            break;
         }

         voiceState.mMemoryStreamPosition -= loopSize;
      }

      return voiceState.mMemoryStreamPosition;
   }
   else
   {
      return 0;
   }
}

void BP_SetMemStreamMuted( const int core, const int voice, const int muted )
{
   BPE_ASSERT( core < 2 && voice < 24, "" );
}

void BP_sceSdSetParam( unsigned short entry, unsigned short value )
{
   int reg = entry & 0xffc0;
   int core = entry & 1;
   int voice = ( entry & ~0xffc0 ) >> 1;
   BPE_ASSERT( voice < 24, "" );

   SSpuVoiceState & voiceState = sSpuVoiceState[core][voice];

   switch( reg )
   {
   case SD_VP_VOLL:
      {
         //voice volume (left)
      }
      break;
   case SD_VP_VOLR:
      {
         //voice volume (right)
      }
      break;
   case SD_VP_PITCH:
      {
         //pitch when sound is generated
         voiceState.mPitch = value;
         BPE_ASSERT_NO_MSG( voiceState.mPitch > 0 );
      }
      break;
   case SD_VP_ADSR1:
      {
         //envelope
      }
      break;
   case SD_VP_ADSR2:
      {
         //envelope (2)
      }
      break;
   case SD_P_MVOLL:
   case SD_P_MVOLR:
      {
         //master volume
         if( value != 0 && value != 0x3fff )
         {
            //these are only set during initialization to zero then max
            //safe to ignore!
            BP_TODO_BREAK;
         }
      }
      break;
   case SD_P_EVOLL:
   case SD_P_EVOLR:
      {
         //Effect return volume (left)
         //Effect return volume (right)
      }
      break;
   default:
      BP_TED_BREAK;
   }
}

unsigned short BP_sceSdGetParam( unsigned short entry )
{
   int reg = entry & 0xffc0;
   int core = entry & 1;
   int voice = ( entry & ~0xffc0 ) >> 1;
   BPE_ASSERT( voice < 24, "" );

   SSpuVoiceState & voiceState = sSpuVoiceState[core][voice];

   switch( reg )
   {
   case SD_VP_ENVX:
      {
         //envelope value
         update_voice_nax( voiceState );
         if( voiceState.mbKeyOn )
         {
            if( voiceState.mNAX == voiceState.mBlockEnd )
            {
               return 0;
            }
            else
            {
               return 0xffff; //max
            }
         }
         else
         {
            return 0;
         }
      }
      break;
   default:
      BP_TED_BREAK;
      return 0;
   }
}

void BP_sceSdSetAddr( unsigned short entry, unsigned int value )
{
   int reg = entry & 0xffc0;
   int core = entry & 1;
   int voice = ( entry & ~0xffc0 ) >> 1;
   BPE_ASSERT( voice < 24, "" );

   SSpuVoiceState & voiceState = sSpuVoiceState[core][voice];

   switch( reg )
   {
   case SD_VA_SSA:
      {
         //waveform data starting address
         voiceState.mSSA = value;
         voiceState.mNAX = value;
         voiceState.mpMemoryStream = NULL;   //cancel any memory stream setting
      }
      break;
   case SD_VA_LSAX:
      {
         //loop point address
         //manually setting loop block point for this key
         voiceState.mLSAX = value;
      }
      break;
   default:
      BP_TED_BREAK;
   }
   
}

unsigned int BP_sceSdGetAddr( unsigned short entry )
{
   int reg = entry & 0xffc0;
   int core = entry & 1;
   int voice = ( entry & ~0xffc0 ) >> 1;
   BPE_ASSERT( voice < 24, "" );

   SSpuVoiceState & voiceState = sSpuVoiceState[core][voice];

   switch( reg )
   {
   case SD_VA_NAX:
      {
         //address of waveform data that should be read next
         update_voice_nax( voiceState );

         return voiceState.mNAX;
      }
      break;
   default:
      BP_TED_BREAK;
      return 0;
   }
}

void BP_sceSdSetSwitch( unsigned short entry, unsigned int value )
{
   int reg = entry & 0xffc0;
   int core = entry & 1;

   switch( reg )
   {
   case SD_S_KOFF:
      {
         //key off (end voice generation)
         for( int i=0; i < 24; ++i )
         {
            int voiceBit = 1 << i;
            if( value & voiceBit )
            {
               SSpuVoiceState & voiceState = sSpuVoiceState[core][i];
               voiceState.mbKeyOn = false;
            }
         }
      }
      break;
   case SD_S_KON:
      {
         //key on (start voice generation)
         for( int i=0; i < 24; ++i )
         {
            int voiceBit = 1 << i;
            if( value & voiceBit )
            {
               SSpuVoiceState & voiceState = sSpuVoiceState[core][i];
               voiceState.mbKeyOn = true;
               voiceState.mDGTickCount = DG_TickCount;

               get_vag_endpoints( sSpuVoiceState[core][i] );
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
            BP_TED_BREAK;
         }
      }
      break;
   case SD_S_PMON:
      {
         //pitch modulation per voice
         if( value != 0 )
         {
            BP_TODO_BREAK;
         }
      }
      break;
   case SD_S_VMIXEL:
   case SD_S_VMIXER:
      {
         //Voice output mixing flag per voice (wet left)
         //Voice output mixing flag per voice (wet right)
      }
      break;
   default:
      BP_TED_BREAK;
   }
}

void BP_sceSdSetCoreAttr( unsigned short entry, unsigned short value )
{
   int reg = entry & 0xfffe;
   int core = entry & 1;
   switch (reg)
   {
   case SD_C_NOISE_CLK:
      {
         //noise generator
         BP_TED_BREAK;
      }
      break;
   case SD_C_EFFECT_ENABLE:
      {
         //Enable / disable writing to effect work area.  Presumably we don't need this hook!
      }
      break;
   default:
      BP_TED_BREAK;
   }
   
}

unsigned short BP_sceSdNote2Pitch ( unsigned short center_note, unsigned short center_fine, unsigned short note, short fine)
{
   //TODO!
   return 0x1000;
}

void BP_SoundSupport_SurroundSoundSetVoiceType( const int core, const int voice, BP_SURROUND_VOICE_TYPE voiceType )
{
   //Stub in sound emulation mode.
}

void BP_SoundSupport_SurroundSoundSetVoicePanVol( const int core, const int voice, float pan, float vol )
{
   //Stub in sound emulation mode.
}

void BP_SoundSupport_ApplyVoiceVolume( const int core, const int voice )
{
   //Stub in sound emulation mode.
}

void BP_SoundSupport_InitADPCMStreamingBuffer( const int channel, const int lr, const int spuAddr, const int spuSize )
{
   //Stub when we're not doing ADPCM decoding on our side.
}

#endif //!BP_PS3 or BP_360

float BP_PS2VolToFVol( const int ps2_vol )
{
   if (ps2_vol == -1) // -1 indicates a volume reset.
      return 0.0f;

   if( ps2_vol & 0x8000 )
   {
      printf("TODO: non-direct volume setting (%d).  BUG to Ted please!\n", ps2_vol);
      BPE_ASSERT_NO_MSG(false);
   }

   //Volume value is in bits 0...14, signed.
   int volValue = (signed short)( (ps2_vol&0x7fff) << 1 );

   float fvol = ((float)volValue)/32768;
   return fvol;
}

void BP_GetDirectOutputStreamSpeakerVolumes_Direct( const int channelCount, const float fVol, float * const outSpeakerVolumes )
{
   BPE_ASSERT_NO_MSG( channelCount <= BP_SPEAKER_COUNT );

   //Assumed that the size of the output array is an 8x8 matrix no matter the speaker count.
   memset( outSpeakerVolumes, 0, sizeof( float ) * BP_SPEAKER_COUNT * BP_SPEAKER_COUNT );
   //The volumes array is indexed by the channel number and speaker number,
   //with indexes 0-7 used for channel 0, 8-15 used for channel 1, etc.

   const int speakerCount = gSoundSupportSurroundSoundChannelCount;
   //Apply / mix channels to speakers.
   //N.B. channels are ordered in the stream data according to the BP_SPEAKERS enum.
   if( channelCount == speakerCount )
   {
      //Send each surround channel directly to the corresponding speaker.
      //No upmixing or downmixing required.
      for( int i = 0; i < channelCount; ++i )
      {
         float * const speakersForThisChannel = &outSpeakerVolumes[i*BP_SPEAKER_COUNT];
         for( int j=0; j < BP_SPEAKER_COUNT; ++j )
         {
            BP_SPEAKERS speaker = (BP_SPEAKERS)j;
            float speakerVol = ( i == j ) ? fVol : 0.f;
            speakersForThisChannel[j] = speakerVol;
         }
      }
   }
   else if( speakerCount == 2 )
   {
      //We're running in legacy stereo mode but the stream has 6 channels.
      //Need to downmix the stream only in this case.
      //Perform same downmix as PS3 system downmixer type A.
      BPE_ASSERT_NO_MSG( channelCount == 6 );
      const float kF = 0.707f;   //front coefficient
      const float kC = 0.5f;     //center coefficient
      const float kR = 0.5f;     //rear coefficient
      //left channel: 0.707 to left speaker
      outSpeakerVolumes[0*BP_SPEAKER_COUNT+BP_SPEAKER_FL] = fVol * kF;
      //right channel: 0.707 to right speaker
      outSpeakerVolumes[1*BP_SPEAKER_COUNT+BP_SPEAKER_FR] = fVol * kF;
      //center channel: 0.5 to each speaker
      outSpeakerVolumes[2*BP_SPEAKER_COUNT+BP_SPEAKER_FL] = fVol * kC;
      outSpeakerVolumes[2*BP_SPEAKER_COUNT+BP_SPEAKER_FR] = fVol * kC;
      //lfe channel: ignored!
      //left rear channel: 0.5 to left speaker
      outSpeakerVolumes[4*BP_SPEAKER_COUNT+BP_SPEAKER_FL] = fVol * kR;
      //right rear channel: 0.5 to right speaker
      outSpeakerVolumes[5*BP_SPEAKER_COUNT+BP_SPEAKER_FR] = fVol * kR;
   }
   else if( channelCount == 2 )
   {
      //Spread stereo to 6- or 8-channel.
      //channel 0 is the left signal in the source stream
      //channel 1 is the right signal in the source stream
      //N.B. if we're upmixing to 6-channel, the last two speakers' volumes will be empty
      //per the tweak setting gpBP_StreamStereoUpmix.
      float speakerVolsL[BP_SPEAKER_COUNT] = { 0 };
      float speakerVolsR[BP_SPEAKER_COUNT] = { 0 };
      speakerVolsL[BP_SPEAKER_FL] = fVol * gpBP_StreamStereoUpmix->mFront;
      speakerVolsR[BP_SPEAKER_FR] = fVol * gpBP_StreamStereoUpmix->mFront;
      //N.B. center speaker cut in half since it gets a contribution from both sides.
      speakerVolsL[BP_SPEAKER_FC] = speakerVolsR[BP_SPEAKER_FC] = fVol * gpBP_StreamStereoUpmix->mCenter * 0.5f;
      //N.B. LFE gets no contribution from a stereo signal.
      speakerVolsL[BP_SPEAKER_LFE] = speakerVolsR[BP_SPEAKER_LFE] = 0.f;
      speakerVolsL[BP_SPEAKER_RL] = fVol * gpBP_StreamStereoUpmix->mRear;
      speakerVolsR[BP_SPEAKER_RR] = fVol * gpBP_StreamStereoUpmix->mRear;
      speakerVolsL[BP_SPEAKER_EL] = fVol * gpBP_StreamStereoUpmix->mExtend;
      speakerVolsR[BP_SPEAKER_ER] = fVol * gpBP_StreamStereoUpmix->mExtend;
      //Copy out the results.
      memcpy( &outSpeakerVolumes[0*BP_SPEAKER_COUNT], speakerVolsL, sizeof( speakerVolsL ) );
      memcpy( &outSpeakerVolumes[1*BP_SPEAKER_COUNT], speakerVolsR, sizeof( speakerVolsR ) );
   }
   else if( channelCount == 6 )
   {
      //Spread 5.1 to 8-channel.
      BPE_ASSERT_NO_MSG( speakerCount == 8 );
      //For the first 4 channels, set volume directly.
      for( int i = 0; i < 4; ++i )
      {
         float * const speakersForThisChannel = &outSpeakerVolumes[i*BP_SPEAKER_COUNT];
         for( int j=0; j < BP_SPEAKER_COUNT; ++j )
         {
            BP_SPEAKERS speaker = (BP_SPEAKERS)j;
            float speakerVol = ( i == j ) ? fVol : 0.f;
            speakersForThisChannel[j] = speakerVol;
         }
      }
      //For RL and RR, spread to RL/EL and RR/ER according to tweaks.
      //channel 4 is the rear-left signal in the source stream
      //channel 5 is the rear-right signal in the source stream
      float speakerVol_Rear = fVol * gpBP_Stream51Upmix->mRear;
      float speakerVol_Extend = fVol * gpBP_Stream51Upmix->mExtend;
      outSpeakerVolumes[4*BP_SPEAKER_COUNT+BP_SPEAKER_RL] = speakerVol_Rear;
      outSpeakerVolumes[4*BP_SPEAKER_COUNT+BP_SPEAKER_EL] = speakerVol_Extend;
      outSpeakerVolumes[5*BP_SPEAKER_COUNT+BP_SPEAKER_RL] = speakerVol_Rear;
      outSpeakerVolumes[5*BP_SPEAKER_COUNT+BP_SPEAKER_EL] = speakerVol_Extend;
   }
   else
   {
      BP_TODO_BREAK;
   }
}

void BP_Downmix7_1To5_1( float * const outVolumes6, const float * const inVolumes8 )
{
   //Same calculation as PS3's downmixer type B
   //L, R, C, LFE will not be affected
   memcpy( outVolumes6, inVolumes8, 4 * sizeof( float ) );
   //Ls (mix) = 1.0 x Ls + 1.0 x Le
   outVolumes6[BP_SPEAKER_RL] = inVolumes8[BP_SPEAKER_RL] + inVolumes8[BP_SPEAKER_EL];
   //Rs (mix) = 1.0 x Rs + 1.0 x Re
   outVolumes6[BP_SPEAKER_RR] = inVolumes8[BP_SPEAKER_RR] + inVolumes8[BP_SPEAKER_ER];
}

static void bp_get_stereo_spread8_volume( float * const outSpeakerVols, const float stereoVolL, const float stereoVolR )
{
   outSpeakerVols[BP_SPEAKER_FL] = stereoVolL * gpBP_StreamStereoUpmix->mFront;
   outSpeakerVols[BP_SPEAKER_FR] = stereoVolR * gpBP_StreamStereoUpmix->mFront;
   //N.B. center speaker cut in half since it gets a contribution from both sides.
   outSpeakerVols[BP_SPEAKER_FC] = ( stereoVolL + stereoVolR ) * gpBP_StreamStereoUpmix->mCenter * 0.5f;
   //N.B. LFE gets no contribution from a stereo signal.
   outSpeakerVols[BP_SPEAKER_LFE] = 0.f;
   outSpeakerVols[BP_SPEAKER_RL] = stereoVolL * gpBP_StreamStereoUpmix->mRear;
   outSpeakerVols[BP_SPEAKER_RR] = stereoVolR * gpBP_StreamStereoUpmix->mRear;
   outSpeakerVols[BP_SPEAKER_EL] = stereoVolL * gpBP_StreamStereoUpmix->mExtend;
   outSpeakerVols[BP_SPEAKER_ER] = stereoVolR * gpBP_StreamStereoUpmix->mExtend;
}

void BP_GetVoiceSpeakerVolumes( const float stereoVolL, const float stereoVolR, const float pan3d, const float vol3d,
                                      BP_SURROUND_VOICE_TYPE surroundType, float * const outSpeakerVolumes )
{
   memset( outSpeakerVolumes, 0, BP_SPEAKER_COUNT * sizeof(float) );

   if( !BP_SoundSupport_IsSurroundSound() )
   {
      //Send L and R channels straight to those speakers and leave the rest empty.
      //Ignore all surround sound settings.
      outSpeakerVolumes[BP_SPEAKER_FL] = stereoVolL;
      outSpeakerVolumes[BP_SPEAKER_FR] = stereoVolR;
   }
   else
   {
      //Apply surround sound volume.
      float speakerVolumes8[BP_SPEAKER_COUNT] = { 0 };

      switch( surroundType )
      {
      case BP_SURROUND_VOICE_TYPE_STEREO:
         {
            //Spread stereo volume to surround speakers according to tweaks.
            bp_get_stereo_spread8_volume( speakerVolumes8, stereoVolL, stereoVolR );
         }
         break;
      case BP_SURROUND_VOICE_TYPE_SURROUND:
         {
            //Apply 3D surround speaker calculation from 3D volume and pan.
            BP_SurroundSoundCalcVols( speakerVolumes8, BP_SPEAKER_COUNT, pan3d, vol3d );
         }
         break;
      case BP_SURROUND_VOICE_TYPE_FLFCFR:
         {
            //Spread stereo volume to FL, FC, and FR speakers.
            //FC gets 1/2 contribution each from L and R.
            speakerVolumes8[BP_SPEAKER_FL] = stereoVolL;
            speakerVolumes8[BP_SPEAKER_FR] = stereoVolR;
            speakerVolumes8[BP_SPEAKER_FC] = (stereoVolL+stereoVolR) * 0.5f;
         }
         break;
      case BP_SURROUND_VOICE_TYPE_FLFR:
         {
            //Send stereo volume directly to FL/FR speakers.
            speakerVolumes8[BP_SPEAKER_FL] = stereoVolL;
            speakerVolumes8[BP_SPEAKER_FR] = stereoVolR;
         }
         break;
      case BP_SURROUND_VOICE_TYPE_CENTER:
         {
            //Apply stereo volume just to center speaker.
            if( stereoVolL != stereoVolR )
            {
               printf("Warning: center channel voice with non-center pan???\n");
               //            BP_TED_BREAK;
            }
            speakerVolumes8[BP_SPEAKER_FC] = (stereoVolL+stereoVolR) * 0.5f;
         }
         break;
      case BP_SURROUND_VOICE_TYPE_CENTER_ABS:
         {
            //take absolute value of volumes to support SD_V_SNA_EATING_S_1 which was requested to be
            //center speaker only in 5.1 by the audio designer but plays the same waveform in both stereo
            //channels but with a negative volume in L so we'd get a volume of 0 if we used BP_SURROUND_VOICE_TYPE_CENTER.
            speakerVolumes8[BP_SPEAKER_FC] = (fabsf(stereoVolL)+fabsf(stereoVolR)) * 0.5f;

         }
         break;
      default:
         BP_TED_BREAK;
      }

      if( gSoundSupportSurroundSoundChannelCount == 6 )
      {
         BP_Downmix7_1To5_1( outSpeakerVolumes, speakerVolumes8 );
      }
      else
      {
         BPE_ASSERT_NO_MSG( gSoundSupportSurroundSoundChannelCount == 8 );
         memcpy( outSpeakerVolumes, speakerVolumes8, sizeof( speakerVolumes8 ) );
      }

   }
}

//----------------------------------------------------------------------------

#if MGS_VERSION==2
static int BP_DebugMenu_ActionCallback_EnableMidiBGM_Changed(int data)
{
   if( BP_EnableMidiBgm )
   {
      //Can't do anything about re-enabling the midi BGM without caching the last one's code.
      //Who cares, the next song started will work.
   }
   else
   {
      //Stop the current midi bgm.
      BP_EnableMidiBgm = 1;
      GM_SdSet( SNG_STOP ) ;
      BP_EnableMidiBgm = 0;
   }

   return 0;
}
#endif

void BP_SoundSupport_InitDebugMenu()
{
#if BP_ENABLE_DEBUG_MENU

   int item;
   int const soundMenu = BP_DebugMenu_AddMenu("Sound", -1);
   {
#if MGS_VERSION==3
      BP_DebugMenu_AddBool(soundMenu, "Enable MTA Streams", &BP_EnableMtaStreams);
      BP_DebugMenu_AddBool(soundMenu, "Enable STV Streams", &BP_EnableVagStreams);
#elif MGS_VERSION==2
      BP_DebugMenu_AddBool(soundMenu, "Enable VAG Streams", &BP_EnableVagStreams);
      item = BP_DebugMenu_AddBool(soundMenu, "Enable Midi BGM", &BP_EnableMidiBgm);
      BP_DebugMenu_SetCallback( item, BP_DebugMenu_ActionCallback_EnableMidiBGM_Changed, 0 );
#endif

      BP_DebugMenu_AddBool(soundMenu, "Enable SFX", &BP_EnableSfx);
      BP_DebugMenu_AddBool(soundMenu, "Show Sound Stats", &gBP_ShowSoundStats);
      BP_DebugMenu_AddBool(soundMenu, "Show Stream Stats", &gBP_ShowStreamStats);
      BP_DebugMenu_AddBool(soundMenu, "Show Voice Stats", &gBP_ShowVoiceStats);
      BP_DebugMenu_AddBool(soundMenu, "Enable SFX Wave Replacements", &gBP_EnableSfxWavReplacements);
      BP_DebugMenu_AddBool(soundMenu, "Spew SFX Played", &BP_SpewSfxPlayed);
   }
   int const reverbMenu = BP_DebugMenu_AddMenu("Reverb", -1);
   {
      BP_SoundSupport_InitDebugMenuPlatformSpecific( soundMenu, reverbMenu );
   }
#endif
}

void BP_SoundSupport_BeginFrame()
{
}

void BP_SoundSupport_EndFrame()
{
   if(gBP_ShowSoundStats)
   {
      BP_DebugText_Print("Active Voices: %d\n", gBP_Sound_CreatedVoices);
      BP_DebugText_Print("Active Memory Streams: %d\n", gBP_Sound_CreatedMemStreams);
      BP_DebugText_Print("Active Loop Buffers: %d\n", gBP_Sound_CreatedLoopBuffers);
      BP_DebugText_Print("Reverb Mode: %s\n", skReverbModeNamesPS2[gReverbModePS2]);
      BP_DebugText_Print("Reverb Depth: %d (%f)\n", gReverbDepthPS2, (float)gReverbDepthPS2 / 0x7fff);
      BP_DebugText_Print("Reverb Depth: %d (%f)\n", gReverbDepthPS2, (float)gReverbDepthPS2 / 0x7fff);
      BP_DebugText_Print("XMP BGM Override: %x\n", gBP_XMPBgmOverrideStreams);
      BP_DebugText_Print("Playing BGM Voices: %d\n", gBP_PlayBgmVoices);
   }
   if( gBP_ShowStreamStats )
   {
      extern void BP_SoundSupport_ShowStreamStats_PlatformSpecific();
      BP_SoundSupport_ShowStreamStats_PlatformSpecific();
   }
   if( gBP_ShowVoiceStats )
   {
      BP_SoundSupport_ShowVoiceStats_PlatformSpecific();
   }
}

//----------------------------------------------------------------------------

struct SIffChunkHeader
{
   char  mId[4];
   int   mSize;
};

struct SRIffChunk
{
   SIffChunkHeader mHeader;
   char  mWavHeader[4];
};

struct SFmtChunk
{
   SIffChunkHeader   mHeader;
   short             mFormatTag;
   unsigned short    mChannelCount;
   unsigned int      mFrequency;
   unsigned int      mAvgBytesPerSec;
   unsigned short    mBlockAlign;
   unsigned short    mBitsPerSample;
};

//----------------------------------------------------------------------------

void BP_SaveLPCM16WavFile( const char * const filename, const unsigned int channelCount, const unsigned int frequency, const unsigned int dataSize, const unsigned char * const data )
{
   FILE * fp = fopen( filename, "wb" );
   if( !fp )
   {
      BP_BREAK;
   }

   const int bytesPerSample = sizeof(short);

   SRIffChunk riffChunk = { { {'R','I','F','F' }, dataSize + 36 }, {'W','A','V','E'} };
   BP_LE_SwapSInt_Inp( &riffChunk.mHeader.mSize );
   fwrite( &riffChunk, sizeof( riffChunk ), 1, fp );

   SFmtChunk fmtChunk = { { {'f','m','t',' '}, sizeof(SFmtChunk)-sizeof(SIffChunkHeader) }, 1, (unsigned short)channelCount, frequency, frequency * channelCount * bytesPerSample, (unsigned short)(channelCount * bytesPerSample), (unsigned short)(bytesPerSample*8) };
   BP_LE_SwapSInt_Inp( &fmtChunk.mHeader.mSize );
   BP_LE_SwapSShort_Inp( &fmtChunk.mFormatTag );
   BP_LE_SwapUShort_Inp( &fmtChunk.mChannelCount );
   BP_LE_SwapUInt_Inp( &fmtChunk.mFrequency );
   BP_LE_SwapUInt_Inp( &fmtChunk.mAvgBytesPerSec );
   BP_LE_SwapUShort_Inp( &fmtChunk.mBlockAlign );
   BP_LE_SwapUShort_Inp( &fmtChunk.mBitsPerSample );
   fwrite( &fmtChunk, sizeof( fmtChunk ), 1, fp );

   SIffChunkHeader dataChunk = { {'d','a','t','a'}, dataSize };
   BP_LE_SwapSInt_Inp( &dataChunk.mSize );
   fwrite( &dataChunk, sizeof( dataChunk ), 1, fp );

   fwrite( data, dataSize, 1, fp );

   fclose( fp );
}

//----------------------------------------------------------------------------

void BP_DecompressVAG(const unsigned char* pVagPacketStart, int numVagPackets, char* pDstBuffer, double * const pPersistentS1, double * const pPersistentS2 )
{
   static const double f[5][2] = { 
      { 0.0, 0.0 },
      { 60.0 / 64.0,  0.0 },
      { 115.0 / 64.0, -52.0 / 64.0 },
      { 98.0 / 64.0, -55.0 / 64.0 },
      { 122.0 / 64.0, -60.0 / 64.0 } 
   };

   double pcmBuffer[28];

   int predict;
   int shiftFactor;

   double s1 = pPersistentS1 ? *pPersistentS1 : 0.0;
   double s2 = pPersistentS2 ? *pPersistentS2 : 0.0;

   for (int i = 0; i < numVagPackets; i++)
   {
      predict = *pVagPacketStart++;
      shiftFactor = predict & 0xf;
      predict >>= 4;

      // Ignore the flags.
      pVagPacketStart++;

      for (int s = 0; s < 28; s += 2 ) 
      {
         int nibble = *pVagPacketStart++;
         int sample = (nibble & 0xf) << 12;

         if (sample & 0x8000)
            sample |= 0xffff0000;

         pcmBuffer[s] = (double)(sample >> shiftFactor);

         sample = (nibble & 0xf0) << 8;
         if (sample & 0x8000)
            sample |= 0xffff0000;

         pcmBuffer[s + 1] = (double)(sample >> shiftFactor);
      }

      for (int s = 0; s < 28; s++)
      {
         pcmBuffer[s] = pcmBuffer[s] + s1 * f[predict][0] + s2 * f[predict][1];
         s2 = s1;
         s1 = pcmBuffer[s];

         int result = (int)(pcmBuffer[s] + 0.5);
         *pDstBuffer++ = result >> 8;
         *pDstBuffer++ = result & 0xff;
      }
   }

   if( pPersistentS1 )
   {
      *pPersistentS1 = s1;
   }
   if( pPersistentS2 )
   {
      *pPersistentS2 = s2;
   }
}

void BP_SanitizeVAG( unsigned char * const pData, const int sz )
{
   unsigned char * p;
   if( sz & 0xf )
      BP_BREAK;
   for( p = pData; p < pData+sz; p += 16 )
   {
      //This was some sort of "stop marker" in PS2 VAG data-- we don't want the pattern in the sample part of the data because it actually
      //plays as waveform data for us.
      static const unsigned char kMagicVagPattern[16] = { 0x00,0x00,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77 };
      if( !memcmp( p+2, kMagicVagPattern+2, sizeof( kMagicVagPattern )-2 ) )
      {
         //Clear out the sample data part of the packet.
         memset( p+2, 0, sizeof( kMagicVagPattern )-2 );
         //Make sure only the "waveform end" bit is set in the flags.
         p[1] = VAG_LOOPEND;
      }
   }
}

//----------------------------------------------------------------------------
