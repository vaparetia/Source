#pragma once

#include "CStreamWriter.h"

// Needed for MTA header information..
#include "CStreamDriver_MTA.h"

class CStreamWriter_Vita_AT9 : public CStreamWriter
{
public:

   // This structure cannot change without changing how the game reads the stream!
   struct At9AudioHeader
   {
      unsigned int mCookie;
      int mFormat;
      int mChannels; // Lower 16 bits has the number of channels, upper 16 bits has the number of tracks.
      int mFrequency;
      int mAT9DataSize; // This is the size in bytes of just the AT9 packet data without padding
      int mStreamDataSize; // This is the size in bytes of all audio packets including padding
      int mTotalStreamSamples;
      unsigned char  mConfigData[4];
   };

   struct AudioFrame 
   {
      AudioFrame() 
         : mAt9BytePos(0)
         , mAt9NumBytes(0)
         , mPcmBytePos(0)
         , mPcmNumBytes(0)
         , mPacketStartTime(0.0f)
         , mPacketEndTime(0.0f)
         , mIsLoopFrame(false)
      {
      }
      
      unsigned int mAt9BytePos;
      unsigned int mAt9NumBytes;
      unsigned int mPcmBytePos;
      unsigned int mPcmNumBytes;
      float mPacketStartTime;
      float mPacketEndTime;
      bool mIsLoopFrame;
   };

   class AudioTrack 
   {
   public:
      AudioTrack() 
         : mpData(NULL)
         , mpAudioDataStart(NULL)
         , mBufferSizeInBytes(0) { }

      ~AudioTrack() 
      {
         delete [] mpData;
      }

      std::vector<AudioFrame> mAudioFrames;
      unsigned char* mpData;
      unsigned char* mpAudioDataStart;
      unsigned int mBufferSizeInBytes;
   };

   class AudioStream
   {
   public:
      AudioStream() 
         : mCurrAudioPacket(0)
         , mTotalAudioPackets(0)
      { 
         memset(&mStreamHeader, 0, sizeof(At9AudioHeader));
      }

      ~AudioStream()
      {
         for (unsigned int i = 0; i < mTracks.size(); i++)
            delete mTracks[i];
      }

      At9AudioHeader mStreamHeader;
      std::vector<AudioTrack*> mTracks;
      unsigned int mCurrAudioPacket;
      unsigned int mTotalAudioPackets;
   };

   CStreamWriter_Vita_AT9();

   virtual bool ReplacementFileExistsForCurrStream(EPlatform curPlatform) const;
   virtual void WriteOnePacket(FILE * const streamfp, EPlatform curPlatform, const int currTick);
   virtual void EndStream(FILE * const streamfp, EPlatform curPlatform, STREAM_TAG* pEndTag);
   virtual bool ReplacesDataForCurrStream(const unsigned int type);
   virtual unsigned int GetRebuildPlatformMask() const;

   // We do not need to actually modify any packets but this interface provides a convenient method by which we can 
   // accumulate the total amount of audio bytes for BGM streams.
   virtual void ModifyCurrentSourcePacket(STREAM_TAG& streamTag, unsigned char * const packetBuf, 
      EPlatform curPlatform);

   //Needed for CStreamWriter_PCM()...
   static bool  ReplacementFileExistsForCurrStream_Static(EPlatform curPlatform);

private:

   void WriteSoundPacket(FILE * streamfp, const int currTick, bool forceWrite);

   AudioStream* mpAudioStream;
   FILE_MTA mMTAHeader; // Used when we are converting BGM streams.
   unsigned int mLoopedSector;
   unsigned int mNumSourceAudioDataBytes; // Just audio data size.
   unsigned int mNumSourceDataBytes; // Includes stream padding and headers.

   bool mIsBGMStream; // All MGS3 BGM streams except for the title_bgm stream have loop regions in which case we need to pad the first packet
   bool mIsMTAHeaderValid;
};
