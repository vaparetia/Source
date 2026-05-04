//----------------------------------------------------------------------------
// BP_AudioStream.h
// Support for BP format audio streams
//----------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------

// Define a Vita specific audio stream header. There are a few unnecessary fields or fields that have different meaning
// so instead of using the same struct and interpreting fields to be different data just make a Vita struct for easier 
// code readability.
#if !defined(BP_VITA)
typedef struct _BP_AudioStreamHeader
{
   unsigned int   mCookie;
   int            mFormat;
   int            mChannels;
   int            mFrequency;
   int            mSampleSize;
   int            mAvgBytesPerSecond;
   int            mBlockAlign;
   int            mPad[1];
} BP_AudioStreamHeader;
#else
typedef struct _BP_AudioStreamHeader
{
   unsigned int   mCookie;
   int            mFormat;
   int            mChannels;
   int            mFrequency;
   int            mAT9DataSize; // This is the size in bytes of just the AT9 packet data without padding
   int            mStreamDataSize; // This is the size in bytes of all audio packets including padding
   int            mTotalStreamSamples;
   unsigned char  mConfigData[4];
   
   // If we are simulating BGM streams in MGS3 this data points to the MTA header. If we are not, this data is ignored.
   // We need this data in the stream audio header so the necessary audio data structures can be initialized when 
   // the stream is first loaded.
   unsigned char mMTAHeader[2048];
} BP_AudioStreamHeader;
#endif

#if MGS_VERSION==2
enum
{
     KCEJSD_STST_ERROR		= -1
   , KCEJSD_STST_NOP
   , KCEJSD_STST_IDLE
   , KCEJSD_STST_OPEN
   , KCEJSD_STST_READY
   , KCEJSD_STST_PLAY
   , KCEJSD_STST_PAUSE
   , KCEJSD_STST_STOP
   , KCEJSD_STST_CLOSE
   , KCEJSD_STST_PAUSEIN
   , KCEJSD_STST_PAUSEOUT
};
#endif

typedef enum _BP_AUDIOSTREAM_PAUSE
{
   BP_AUDIOSTREAM_PAUSE_STREAM   = 1<<0,  //stream paused directly
   BP_AUDIOSTREAM_PAUSE_GLOBAL   = 1<<1,  //all sounds "paused"
} BP_AUDIOSTREAM_PAUSE;

//----------------------------------------------------------------------------

void BP_AudioStreamInit();
void BP_AudioStreamOpen( int channel, const void *header, int streamType, int surroundType, int streamTop );
int BP_AudioStreamGetChannelCount( int channel );
int BP_AudioStreamGetStatus( int channel );
void BP_AudioStreamPlay( int channel );
void BP_AudioStreamStop( int channel, int time );
void BP_AudioStreamPause( int channel, int command, unsigned int pause );
void BP_AudioStreamSetVolume_Voice( int channel, int volsStereo[2][2], float pan3d, float vol3d );
void BP_AudioStreamGetVolume_Voice( int* pVolumes, int channel);
int BP_AudioStreamGetPlayTime( int channel );   //ms
int BP_AudioStreamGetRequest( int channel ); //bytes
int BP_AudioStreamGetDoneCount( int channel ); //bytes
void BP_AudioStreamSetPacket( int channel, void *buffer, int sizeBytes );
void BP_AudioStream_SetDspEnabled( int channel, int enabled );
void BP_AudioStreamUpdate();

//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif
