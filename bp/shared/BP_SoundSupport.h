//----------------------------------------------------------------------------
// BP_SoundSupport.h
//----------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <sdmacro.h>

//----------------------------------------------------------------------------

#define SMEM_Low	(0)
#define SMEM_High	(1)
#define SMEM_Addr	(2)

#define  BP_SPU_MEMORY_SIZE (2*1024*1024)

// On Xbox there could be up to 3 streams playing at a time and under the BP_Audio system, MTA streams could possibly
// be playing at the same time as the other streams. (note, MTA streams are forced to stream channel 2)
#if MGS_VERSION == 3

#define  BP_DOSTREAM_COUNT 3
//N.B. MTA stream format in MGS3 supported to 8 tracks, but none have more than 3.
#define  BP_DOSTREAM_MAX_TRACK_VALUE 8
#define  BP_DOSTREAM_TRACK_COUNT 3

#if defined(BP_VITA)
#define BP_DOSTREAM_BUFFER_SIZE (512 * 1024)
#else
#define BP_DOSTREAM_BUFFER_SIZE (80*0x800)
#endif

#elif MGS_VERSION == 2

#define  BP_DOSTREAM_COUNT 2
#define  BP_DOSTREAM_MAX_TRACK_VALUE 1
#define  BP_DOSTREAM_TRACK_COUNT 1

#if defined(BP_VITA)
#define BP_DOSTREAM_BUFFER_SIZE (512 * 1024) // Vita sound playback uses it's own buffer, this size is just used for stream advancement
#else
#define  BP_DOSTREAM_BUFFER_SIZE (64*0x400)
#endif

#endif

#define	VAG_LOOPSTART	(0x04)
#define	VAG_LOOPBLOCK	(0x02)
#define	VAG_LOOPEND		(0x01)

typedef enum BP_DOSTREAM_FORMAT
{
   BP_DOSTREAM_FORMAT_PCM,
   BP_DOSTREAM_FORMAT_VAG,

   BP_DOSTREAM_FORMAT_COUNT
} BP_DOSTREAM_FORMAT;

typedef enum _BP_DOSTREAM_TYPE
{
   BP_DOSTREAM_TYPE_VOICE, //replaces a VAG stream that went through PS2 voice hardware (str2 or stv)
   BP_DOSTREAM_TYPE_DIRECT //replaces a stream that went straight to PS2 sound output (str3/lnr8 or mta)
} BP_DOSTREAM_TYPE;

//How to calculate surround speaker volumes per voice when running surround mode
typedef enum BP_SURROUND_VOICE_TYPE
{
   BP_SURROUND_VOICE_TYPE_STEREO,   //spread stereo L/R volumes to surround speakers
   BP_SURROUND_VOICE_TYPE_SURROUND, //ignore stereo L/R volumes and use 3d pan / vol
   BP_SURROUND_VOICE_TYPE_FLFCFR,   //spread stereo L/R volumes to FL/FC/FR speakers.
   BP_SURROUND_VOICE_TYPE_FLFR,     //send stereo L/R volumes directly to FL/FR speakers.
   BP_SURROUND_VOICE_TYPE_CENTER,   //send only to center speaker (for mono speech).  Take average of L/R stereo volumes.
   BP_SURROUND_VOICE_TYPE_CENTER_ABS,  //send only to center speaker, Take average of abs(L)/abs(R) stereo volumes.
} BP_SURROUND_VOICE_TYPE;

//N.B. matches PS3 multistream speaker ordering since that's not configurable.
enum BP_SPEAKERS
{
   BP_SPEAKER_FL,
   BP_SPEAKER_FR,
   BP_SPEAKER_FC,
   BP_SPEAKER_LFE,
   BP_SPEAKER_RL,
   BP_SPEAKER_RR,
   BP_SPEAKER_EL,
   BP_SPEAKER_ER,

   BP_SPEAKER_COUNT,
};

//How much of L or R stereo channel to mix into each side's 8-channel mix
typedef struct _SBP2_8ChannelUpmix
{
   float mFront;
   float mCenter;
   float mRear;
   float mExtend;
} SBP2_8ChannelUpmix;

//How much of RL or RR channel to mix into each side's Rear/Extend mix
//(doesn't seem useful to mix any other channels)
typedef struct _SBP6_8ChannelUpmix
{
   float mRear;
   float mExtend;
} SBP6_8ChannelUpmix;

typedef struct _DirectOutputStreamInitData
{
   int channel;
   int trackCount;
   int channelCount;
   int totalStreamSize;
   int playbackBufferSize;
   int freq;
   int format;
   int avgBytesPerSec;
   int blockAlign;
   int streamType;
   int streamTop;
   unsigned char configData[4]; // Only used on Vita
   int totalStreamSamples; // Only used on Vita
   unsigned char mMTAHeader[2048]; // Only used on Vita
} DirectOutputStreamInitData;

extern SBP2_8ChannelUpmix * gpBP_SeUpmix;
extern SBP2_8ChannelUpmix * gpBP_StreamStereoUpmix;
extern SBP6_8ChannelUpmix * gpBP_Stream51Upmix;

extern void BP_SoundSupport_InitDebugMenu();
extern void BP_SoundSupport_BeginFrame();
extern void BP_SoundSupport_EndFrame();

extern void BP_SoundSupport_Init();
extern void BP_SoundSupport_Shutdown();
extern void BP_SoundSupport_SetMasterVolume( float volume );
extern void BP_SoundSupport_Update();

extern int BP_SoundSupport_IsSurroundSound();
extern int BP_SoundSupport_GetSurroundSoundChannelCount();

extern void BP_SoundSupport_SurroundSoundSetVoiceType( const int core, const int voice, BP_SURROUND_VOICE_TYPE voiceType );
extern void BP_SoundSupport_SurroundSoundSetVoicePanVol( const int core, const int voice, float pan, float vol );
extern void BP_SoundSupport_ApplyVoiceVolume( const int core, const int voice );

extern void BP_SoundSupport_InitADPCMStreamingBuffer( const int channel, const int lr, const int spuAddr, const int spuSize );

extern float BP_PS2VolToFVol( const int ps2_vol );
extern void BP_GetDirectOutputStreamSpeakerVolumes_Direct( const int channelCount, const float fVol, float * const outSpeakerVolumes );
extern void BP_Downmix7_1To5_1( float * const outVolumes6, const float * const inVolumes8 );
extern void BP_GetVoiceSpeakerVolumes( const float stereoVolL, const float stereoVolR, const float pan3d, const float vol3d,
                                      BP_SURROUND_VOICE_TYPE surroundType, float * const outSpeakerVolumes );

//Raw waveform playback support.  Used to play a wav file at end of TGS demo.
extern unsigned int BP_PlayWaveform( const unsigned char * const waveformData, const int size, const int frequency, const int channelCount );
extern void BP_StopWaveform( const unsigned int handle );
extern void BP_SetWaveformVolume( const unsigned int handle, const float volume );

//"Direct output" streaming support for streams that were DMA'ed directly to PS2 sound output hardware
//MTA streams in MGS3 and str3 streams in MGS2 use this.
//Also, this is now our interface to rebuilt / re-encoded platform-specific audio streams.
extern void BP_InitDirectOutputStream(DirectOutputStreamInitData const* pInitInfo);
extern void BP_ContinuePlayDirectOutputStream( const int channel );
extern void BP_SetDirectOutputStreamPaused( const int channel, int paused );
extern void BP_StopDirectOutputStream( const int channel );
extern int BP_IsDirectOutputStreamDone( const int channel );
extern int BP_GetDirectOutputStreamPosition( const int channel );
extern double BP_GetDirectOutputStreamPlayTime( const int channel );
extern void BP_SetDirectOutputStreamVolume_Direct( const int channel, const int track, const float fVol );
extern void BP_SetDirectOutputStreamVolume_Voice( const int channel, int stereoVols[2][2],
                                                 float pan3d, float vol3d, BP_SURROUND_VOICE_TYPE surroundType);
extern void BP_SetDirectOutputStreamDsp_Voice( const int channel, int bDspEnabled );

extern unsigned char * BP_GetDirectOutputStreamPlaybackBuffer( const int channel );
extern int BP_GetDirectOutputStreamReadAheadSize( const int channel );
extern void BP_SetAllDirectOutputStreamPaused( int paused );
extern void BP_NotifyDirectOutputStreamBytesAvailable(int channel, void* buffer, int sizeBytes);

//Memory stream support (playback from outside fake SPU memory)
extern void BP_PlayMemStream( const int core, const int voice, const void * const playbackBuffer, const int playbackBufferSize );
extern int BP_GetMemStreamPosition(const int core, const int voice);
extern void BP_SetMemStreamMuted( const int core, const int voice, const int muted );

//PS2 API emulation
extern void *AllocSysMemory(int type, unsigned long size, void *addr);
extern int FreeSysMemory(void *area);

extern void             BP_sceSdSetParam( unsigned short entry, unsigned short value );
extern unsigned short   BP_sceSdGetParam( unsigned short entry );
extern unsigned int     BP_sceSdGetAddr( unsigned short entry );
extern void             BP_sceSdSetAddr( unsigned short entry, unsigned int value );
extern void             BP_sceSdSetSwitch( unsigned short entry, unsigned int value );
extern void             BP_sceSdSetCoreAttr( unsigned short entry, unsigned short value );
extern unsigned short   BP_sceSdNote2Pitch ( unsigned short center_note, unsigned short center_fine, unsigned short note, short fine);
extern int              BP_sceSdVoiceTrans( short channel, unsigned short mode, unsigned char *m_addr, unsigned int _s_addr, unsigned int size );
extern int              BP_sceSdVoiceTransStatus (short channel, short flag);
extern int              BP_sceSdSetEffectAttr( int core, sceSdEffectAttr *attr );

//Flag whether sound system should decode waveform data on the fly when transferring into emulated SPU memory
extern int              gBP_DecodeSpuMemoryOnTransfer;

//Decode a range of memory all at once when all transfers are complete
extern void             BP_DecodeSpuMemory( unsigned int _s_addr, unsigned int size );

extern unsigned int BP_SdGetUTime( void );
extern unsigned int BP_SdGetTime( void );

extern void             BP_SaveLPCM16WavFile( const char * const filename, const unsigned int channelCount, const unsigned int frequency,
                                              const unsigned int dataSize, const unsigned char * const data );

extern void             BP_DecompressVAG(const unsigned char* pVagPacketStart, int numVagPackets, char* pDstBuffer, double * const pPersistentS1,
                                         double * const pPersistentS2 );

extern void             BP_SanitizeVAG( unsigned char * const pData, const int sz );

extern int              BP_EnableVagStreams;
extern int              BP_EnableMtaStreams;
#if MGS_VERSION==3
#elif MGS_VERSION==2
#endif

extern int              gBP_DirectOutputStreamSpeed;
extern int              gBP_DirectOutputStreamIsDemo;

//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif
