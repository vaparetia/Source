//----------------------------------------------------------------------------
// BP_AudioStream.cpp
// Support for BP format audio streams
//----------------------------------------------------------------------------

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/StlExtras/BPEStlExtras.h"
#include "Engine/System/CStopWatch.h"

#include "MGS_Common.h"
#include "BP_SoundSupport.h"
#include "BP_AudioStream.h"

extern "C" int gBP_MTAChannel;
//----------------------------------------------------------------------------

namespace
{
   const int skAudioStreamPlaybackBufferSize = BP_DOSTREAM_BUFFER_SIZE;
   struct SAudioStream
   {
      BP_AudioStreamHeader mHeader;
      int                  mStreamType;
      int                  mStatus;
      int                  mReadPosition;
      int                  mWritePosition;
      int                  mPlayedBytes;
      int                  mDoneBytes;
      int                  mReadBytes;
      unsigned int         mPauseBits;
      bool                 mbPlaying;
      //Voice mode volume
      int                  mVoiceVolsStereo[2][2]; //L/R for each of channel 0/1
      float                mVoiceVol3d;
      float                mVoicePan3d;
      int                  mVoiceSurroundType;
      //Direct mode volume
      float                mDirectVolumeBase;
      float                mDirectVolumeTarget;
      float                mDirectVolumeCurr;
      int                  mFadeTimeStart;   //ms, relative to playback start.
      int                  mFadeTimeEnd;
   };

   SAudioStream sStreamState[BP_DOSTREAM_COUNT];
}

//----------------------------------------------------------------------------

static void bp_audiostream_update_read_position( const int channel )
{
   BPE_VERIFY( channel >= 0 && channel < BP_DOSTREAM_COUNT, false, "channel out of range" );

   SAudioStream & audioStream = sStreamState[channel];

   if( !audioStream.mbPlaying )
   {
      return;
   }

   int readPosition = BP_GetDirectOutputStreamPosition(channel);

   if( readPosition < 0 || readPosition >= skAudioStreamPlaybackBufferSize )
   {
      //Playback buffer set up wrong!
      BP_BREAK;
   }

   int readDiff;
   if( readPosition < audioStream.mReadPosition )
   {
      //Read position has wrapped around.
      readDiff = skAudioStreamPlaybackBufferSize - audioStream.mReadPosition + readPosition;
   }
   else
   {
      //Read position has just advanced.
      readDiff = readPosition - audioStream.mReadPosition;
   }

   audioStream.mReadPosition = readPosition;
   audioStream.mPlayedBytes += readDiff;
   audioStream.mReadBytes += readDiff;

   if( BP_IsDirectOutputStreamDone( channel ) )
   {
      audioStream.mbPlaying = false;
      audioStream.mStatus = KCEJSD_STST_NOP;
      audioStream.mDirectVolumeBase = audioStream.mDirectVolumeCurr = audioStream.mDirectVolumeTarget = 0.f;
   }
}

static void bp_audiostream_set_volume( const int channel )
{
   BPE_VERIFY( channel >= 0 && channel < BP_DOSTREAM_COUNT, false, "channel out of range" );
   SAudioStream & audioStream = sStreamState[channel];

   // MTA audio volume when running under MGS3 is controlled using mtast_vita.c
#if MGS_VERSION == 3
   if (channel != gBP_MTAChannel)
#endif
   {
      if( audioStream.mStreamType == BP_DOSTREAM_TYPE_DIRECT )
      {
         //There's no reason one of these streams should go outside the normal 0...1 range of PS2.
         BPE_ASSERT( audioStream.mDirectVolumeCurr >= 0 && audioStream.mDirectVolumeCurr <= 1.f, "volume out of range" );
         const int track = 0;
         BP_SetDirectOutputStreamVolume_Direct( channel, track, audioStream.mDirectVolumeCurr );
      }
      else
      {
         int volsStereo[2][2] = { 0 };
         float vol3d = 0.f;
         if( BP_EnableVagStreams )
         {
            memcpy( volsStereo, audioStream.mVoiceVolsStereo, sizeof( volsStereo ) );
            vol3d = audioStream.mVoiceVol3d;
         }
         BP_SetDirectOutputStreamVolume_Voice( channel, volsStereo, audioStream.mVoicePan3d, vol3d, (BP_SURROUND_VOICE_TYPE)audioStream.mVoiceSurroundType );
      }
   }
}

static void bp_audiostream_update( const int channel )
{
   BPE_VERIFY( channel >= 0 && channel < BP_DOSTREAM_COUNT, false, "channel out of range" );
   SAudioStream & audioStream = sStreamState[channel];

   if( !audioStream.mbPlaying )
   {
      return;
   }

   bp_audiostream_update_read_position( channel );

#if MGS_VERSION == 3
   if (channel == gBP_MTAChannel)
      return; // Don't mess with volume or fading for MTA streams. mtast_vita.c handles this for MTA stream simulation.
#endif

   //Update fade in / out.
   if( audioStream.mDirectVolumeTarget != audioStream.mDirectVolumeCurr )
   {
      // If we are trying to fade out but paused, just stop the voice
      if (audioStream.mPauseBits && audioStream.mDirectVolumeTarget == 0.0f)
      {
         audioStream.mDirectVolumeCurr = 0.0f;
      }
      else
      {
         int now = BP_AudioStreamGetPlayTime( channel );

         // In addition to checking to see if we have faded out, make sure there is actually enough data remaining in 
         // the stream to fade.
         if( now >= audioStream.mFadeTimeEnd || audioStream.mReadBytes == audioStream.mDoneBytes)
         {
            //Done fading.
            audioStream.mDirectVolumeCurr = audioStream.mDirectVolumeTarget;
            //         printf("bp_audiostream_update: fade stop!\n");
         }
         else if( now >= audioStream.mFadeTimeStart )
         {
            float t = float( now - audioStream.mFadeTimeStart ) / ( audioStream.mFadeTimeEnd - audioStream.mFadeTimeStart );
            //         printf("bp_audiostream_update: fade interpolate %f!\n", t);
            audioStream.mDirectVolumeCurr = audioStream.mDirectVolumeBase + ( audioStream.mDirectVolumeTarget - audioStream.mDirectVolumeBase ) * t;
         }
      }
   }

   if( audioStream.mDirectVolumeCurr == 0.f && audioStream.mDirectVolumeTarget == 0.f && audioStream.mStatus == KCEJSD_STST_STOP )
   {
      //Finished fading out.
      BP_StopDirectOutputStream(channel);
      audioStream.mbPlaying = false;
      audioStream.mStatus = KCEJSD_STST_NOP;
      audioStream.mDirectVolumeBase = audioStream.mDirectVolumeCurr = audioStream.mDirectVolumeTarget = 0.f;
   }

   //Always update volume.
   bp_audiostream_set_volume( channel );
}

//----------------------------------------------------------------------------
void BP_AudioStreamInit()
{
   memset(&sStreamState[0], 0, sizeof(sStreamState));

   for (int i = 0; i < BP_DOSTREAM_COUNT; i++)
   {
      sStreamState[i].mVoiceVolsStereo[0][0] = -1;
      sStreamState[i].mVoiceVolsStereo[0][1] = -1;
      sStreamState[i].mVoiceVolsStereo[1][0] = -1;
      sStreamState[i].mVoiceVolsStereo[1][1] = -1;
   }
}

//----------------------------------------------------------------------------

void BP_AudioStreamOpen( int channel, const void *header, int streamType, int surroundType, int streamTop )
{
   BPE_VERIFY( channel >= 0 && channel < BP_DOSTREAM_COUNT, false, "channel out of range" );
   SAudioStream & audioStream = sStreamState[channel];
   memcpy( &audioStream.mHeader, header, sizeof( audioStream.mHeader ) );
   audioStream.mStreamType = streamType;
   audioStream.mVoiceSurroundType = surroundType;

#if defined(BP_PS3)
   BPE_VERIFY( audioStream.mHeader.mCookie == 'MSFC', false, "invalid MSF stream" );
#elif defined(BP_360)
   BPE_VERIFY( audioStream.mHeader.mCookie == 'XWMA', false, "invalid xbox audio stream" );
#elif defined(BP_VITA)
   BPE_VERIFY( audioStream.mHeader.mCookie == 'VAT9', false, "invalid Vita audio stream");
#else
   BP_TODO_BREAK; //no platform-specific audio streams exist for any other platforms!
#endif

   const int trackCount = 1;

   DirectOutputStreamInitData initData = { 0 };

   initData.channel = channel;

#if defined(BP_VITA)
   initData.trackCount = (audioStream.mHeader.mChannels >> 16) & 0xffff;
   if (!initData.trackCount)
      initData.trackCount = 1;
   initData.channelCount = audioStream.mHeader.mChannels & 0xffff;

   // Need to set the header channels to be the actual channel count so other systems work correctly.
   audioStream.mHeader.mChannels = initData.channelCount;
#else
   initData.trackCount = trackCount;
   initData.channelCount = audioStream.mHeader.mChannels;
#endif

   initData.playbackBufferSize = skAudioStreamPlaybackBufferSize;
   initData.freq = audioStream.mHeader.mFrequency;
   initData.format = audioStream.mHeader.mFormat;
   initData.streamType = streamType;
   initData.streamTop = streamTop;

#if !defined(BP_VITA)   
   initData.totalStreamSize = audioStream.mHeader.mSampleSize;
   initData.avgBytesPerSec = audioStream.mHeader.mAvgBytesPerSecond;
   initData.blockAlign = audioStream.mHeader.mBlockAlign;   
#else
   initData.totalStreamSamples = audioStream.mHeader.mTotalStreamSamples;
   initData.totalStreamSize = audioStream.mHeader.mAT9DataSize;
   memcpy(&initData.configData[0], &audioStream.mHeader.mConfigData[0], sizeof(unsigned int));

   // This data will not be valid unless the stream is a MGS3 bgm stream.
   memcpy(&initData.mMTAHeader[0], &audioStream.mHeader.mMTAHeader[0], sizeof(initData.mMTAHeader));
#endif

   audioStream.mDirectVolumeBase = 1.f;
   audioStream.mDirectVolumeTarget = 1.f;
   audioStream.mDirectVolumeCurr = 1.f;
   audioStream.mFadeTimeStart = audioStream.mFadeTimeEnd = 0;

   BP_InitDirectOutputStream(&initData);

   audioStream.mStatus = KCEJSD_STST_READY;
   audioStream.mReadPosition = 0;
   audioStream.mWritePosition = 0;
   audioStream.mPlayedBytes = 0;
   audioStream.mDoneBytes = 0;
   audioStream.mReadBytes = 0;
   audioStream.mPauseBits = 0;
   audioStream.mbPlaying = false;
}

int BP_AudioStreamGetChannelCount( int channel )
{
   BPE_VERIFY( channel >= 0 && channel < BP_DOSTREAM_COUNT, false, "channel out of range" );
   return sStreamState[channel].mHeader.mChannels & 0x0000ffff;
}

int BP_AudioStreamGetStatus( int channel )
{
   BPE_VERIFY( channel >= 0 && channel < BP_DOSTREAM_COUNT, false, "channel out of range" );
   return sStreamState[channel].mStatus;
}

void BP_AudioStreamPlay( int channel )
{
   BPE_VERIFY( channel >= 0 && channel < BP_DOSTREAM_COUNT, false, "channel out of range" );
   SAudioStream & audioStream = sStreamState[channel];
   audioStream.mbPlaying = true;
   audioStream.mStatus = KCEJSD_STST_PLAY;
}

void BP_AudioStreamStop( int channel, int time )
{
   BPE_VERIFY( channel >= 0 && channel < BP_DOSTREAM_COUNT, false, "channel out of range" );
   SAudioStream & audioStream = sStreamState[channel];

#if MGS_VERSION==2
   //Full immediate stop on both kinds of streams.
   BP_StopDirectOutputStream(channel);
   audioStream.mbPlaying = false;
   audioStream.mStatus = KCEJSD_STST_STOP;
#else
   if ( audioStream.mStreamType == BP_DOSTREAM_TYPE_DIRECT && channel != gBP_MTAChannel)
   {
      //Fade out.
      //N.B. a stopReq in mtast.c results in a minimum stopFadeTime of 3.
      time = std::max( time, 3 );
      int fadeTimeMs = time * 100 + 1; //from mtast.c; time is apparently measured in 10ths of a second
      audioStream.mDirectVolumeBase = audioStream.mDirectVolumeCurr;
      audioStream.mDirectVolumeTarget = 0.f;
      audioStream.mStatus = KCEJSD_STST_STOP;
      audioStream.mFadeTimeStart = BP_AudioStreamGetPlayTime(channel);
      audioStream.mFadeTimeEnd = audioStream.mFadeTimeStart + fadeTimeMs;
   }
   else
   {
      //vagstreams stop immediately in MGS3.
      BP_StopDirectOutputStream(channel);
      audioStream.mbPlaying = false;
      audioStream.mStatus = KCEJSD_STST_NOP;
      audioStream.mDirectVolumeBase = audioStream.mDirectVolumeCurr = audioStream.mDirectVolumeTarget = 0.f;
   }
#endif
}

void BP_AudioStreamPause( int channel, int command, unsigned int pause )
{
   BPE_VERIFY( channel >= 0 && channel < BP_DOSTREAM_COUNT, false, "channel out of range" );
   SAudioStream & audioStream = sStreamState[channel];

#if MGS_VERSION==2

   if( command )
   {
      audioStream.mPauseBits |= pause;
   }
   else
   {
      audioStream.mPauseBits &= (~pause);
   }

#else

   switch( command )
   {
   case KCEJSD_SYST_PAUSE_ON:
      audioStream.mPauseBits |= pause;
      break;
   case KCEJSD_SYST_PAUSE_OFF:
      audioStream.mPauseBits &= ~pause;
      break;
   default:
      BP_TODO_BREAK;
   }
#endif

   if( !audioStream.mbPlaying )
   {
      return;
   }

   if( audioStream.mPauseBits )
   {
      BP_SetDirectOutputStreamPaused( channel, 1 );
   }
   else
   {
      BP_SetDirectOutputStreamPaused( channel, 0 );
   }
}

void BP_AudioStreamSetVolume_Voice( int channel, int volsStereo[2][2], float pan3d, float vol3d)
{
   SAudioStream & audioStream = sStreamState[channel];
   //Game is allowed to set these even if the stream is not playing.
   memcpy( audioStream.mVoiceVolsStereo, volsStereo, sizeof( audioStream.mVoiceVolsStereo ) );
   audioStream.mVoicePan3d = pan3d;
   audioStream.mVoiceVol3d = vol3d;
   bp_audiostream_set_volume( channel );
}

void BP_AudioStreamGetVolume_Voice( int* pVolumes, int channel)
{
   BPE_VERIFY(pVolumes, false, false);
   memcpy(pVolumes, &sStreamState[channel].mVoiceVolsStereo[0][0], sizeof(int) * 4);
}

int BP_AudioStreamGetPlayTime( int channel )
{
   BPE_VERIFY( channel >= 0 && channel < BP_DOSTREAM_COUNT, false, "channel out of range" );
   bp_audiostream_update_read_position( channel );
   double playTime = BP_GetDirectOutputStreamPlayTime( channel );
   int ms = (int)( playTime * 1000 );
   return ms;
}

int BP_AudioStreamGetRequest( int channel )
{
   BPE_VERIFY( channel >= 0 && channel < BP_DOSTREAM_COUNT, false, "channel out of range" );
   bp_audiostream_update_read_position( channel );
   
   SAudioStream & audioStream = sStreamState[channel];
   
   int const readAheadSize = BP_GetDirectOutputStreamReadAheadSize(channel);
   
   if (BP_GetDirectOutputStreamPlaybackBuffer(channel))
   {
      // Only need to verify this size if using BP_SoundSupport's streaming buffer. If this buffer is null, the
      // platform is responsible for managing it's own buffers.
      BPE_VERIFY(readAheadSize <= skAudioStreamPlaybackBufferSize, false, "Read Ahead size too big");
   }

   int request = audioStream.mReadBytes + readAheadSize;
//   printf("BP_AudioStreamGetRequest: %d = %d + %d\n", request, audioStream.mReadBytes, readAheadSize );
   return request;
}

int BP_AudioStreamGetDoneCount( int channel )
{
   //Count of packets (bytes) that we're done with.  Data is copied out immediately
   //for this type of stream.
   BPE_VERIFY( channel >= 0 && channel < BP_DOSTREAM_COUNT, false, "channel out of range" );
   SAudioStream & audioStream = sStreamState[channel];
   return audioStream.mDoneBytes;
}

void BP_AudioStreamSetPacket( int channel, void *buffer, int sizeBytes )
{
   BPE_VERIFY( channel >= 0 && channel < BP_DOSTREAM_COUNT, false, "channel out of range" );
   SAudioStream & audioStream = sStreamState[channel];
   // Inform the audio system we have new bytes available in the playback buffer.
   BP_NotifyDirectOutputStreamBytesAvailable(channel, buffer, sizeBytes);

   //Copy data into playback buffer.
   unsigned char * playbackBuffer = BP_GetDirectOutputStreamPlaybackBuffer(channel);
#if !defined(BP_360) && !defined(BP_VITA)
   //On X360 and Vita, all of the streams that go through this interface do not use the sound system's
   //direct playback buffer so it has been removed.
   BPE_ASSERT_NO_MSG( playbackBuffer != NULL );
#endif
   if( audioStream.mReadPosition > audioStream.mWritePosition )
   {
      //Read is ahead of Write in the buffer.
      if( playbackBuffer )
      {
         BPE_VERIFY( audioStream.mWritePosition + sizeBytes <= audioStream.mReadPosition, false, "Buffer overlap!" );
         memcpy( playbackBuffer + audioStream.mWritePosition, buffer, sizeBytes );
      }
//      printf("BP_AudioStreamSetPacket B: R %d W %d S %d\n", audioStream.mReadPosition, audioStream.mWritePosition, sizeBytes );
      audioStream.mWritePosition += sizeBytes;
      //Zero out the rest of the consumed buffer.
      if( playbackBuffer )
      {
         memset( playbackBuffer + audioStream.mWritePosition, 0, audioStream.mReadPosition - audioStream.mWritePosition );
      }
   }
   else
   {
      //Read is behind Write in the buffer.
      //Wrap around to the beginning if the write size crosses the end.
      int firstWriteSize = std::min( sizeBytes, skAudioStreamPlaybackBufferSize - audioStream.mWritePosition );
      if( playbackBuffer )
      {
         memcpy( playbackBuffer + audioStream.mWritePosition, buffer, firstWriteSize );
      }
      
//      printf("BP_AudioStreamSetPacket 1: R %d W %d S %d\n", audioStream.mReadPosition, audioStream.mWritePosition, firstWriteSize );
      audioStream.mWritePosition += firstWriteSize;

      int secondWriteSize = sizeBytes - firstWriteSize;
      if( secondWriteSize > 0 )
      {
         //We wrapped.  There must be enough space in front of the read position to copy the remainder
         //at the beginning of the buffer.
         if( playbackBuffer )
         {
            BPE_VERIFY( secondWriteSize <= audioStream.mReadPosition, false, "Buffer overlap!" );
            memcpy( playbackBuffer + 0, (unsigned char*)buffer + firstWriteSize, secondWriteSize );
         }
//         printf("BP_AudioStreamSetPacket 2: R %d W %d S %d\n", audioStream.mReadPosition, audioStream.mWritePosition, secondWriteSize );
         audioStream.mWritePosition = secondWriteSize;
         //Zero out the rest of the consumed buffer.
         if( playbackBuffer )
         {
            memset( playbackBuffer + audioStream.mWritePosition, 0, audioStream.mReadPosition - audioStream.mWritePosition );
         }
      }
      else
      {
         //Didn't wrap yet.
         //Zero out the rest of the consumed buffer.
         if( playbackBuffer )
         {
            memset( playbackBuffer + audioStream.mWritePosition, 0, skAudioStreamPlaybackBufferSize - audioStream.mWritePosition );
            memset( playbackBuffer, 0, audioStream.mReadPosition );
         }
      }

      // Update write position wrapping around, otherwise if this write fills up the buffer 
      // completely (i.e. 65536 on MGS2) the logic that compares the stream read offset which is wrapped (returned 
      // by BP_GetDirectOutputStreamPosition) against the write offset will fail (readPos = 0 AND writePos = bufferEnd -> buffer full)
      audioStream.mWritePosition %= skAudioStreamPlaybackBufferSize;
   }

   //client stream is free to get rid of this data; we've copied it out.
   audioStream.mDoneBytes += sizeBytes;

   if( audioStream.mDoneBytes >= skAudioStreamPlaybackBufferSize / 2 )
   {
      //We can play as soon as there's at least half a full buffer worth filled.
      BP_ContinuePlayDirectOutputStream( channel );
   }
   bp_audiostream_set_volume( channel );
}

void BP_AudioStream_SetDspEnabled( int channel, int enabled )
{
   BPE_VERIFY( channel >= 0 && channel < BP_DOSTREAM_COUNT, false, "channel out of range" );
   SAudioStream & audioStream = sStreamState[channel];
   BP_SetDirectOutputStreamDsp_Voice( channel, enabled );
}

void BP_AudioStreamUpdate()
{
   for( int i=0; i < BP_DOSTREAM_COUNT; ++i )
   {
      bp_audiostream_update( i );
   }
}
