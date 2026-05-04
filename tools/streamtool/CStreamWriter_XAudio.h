#pragma once

#include "CStreamWriter.h"

class XWMADECODE;

namespace
{
   unsigned int const kMaxAudioTracks = 8;
}

class CStreamWriter_XAudio : public CStreamWriter
{
public:

   // This structure cannot change without changing how the game reads the stream!
   struct XAudioStreamHeader
   {
      unsigned int   mCookie;
      int            mFormat;
      int            mChannels;
      int            mFrequency;
      int            mSampleSize;
      int            mAvgBytesPerSec;
      int            mBlockAlign;
      int            mUnused;
   };

   struct AudioFrame 
   {
      AudioFrame() : xwmBytePos(0), xwmLen(0), pcmBytePos(0), pcmLen(0), packetStartTime(0.0f), packetEndTime(0.0f) { }
      unsigned int xwmBytePos;
      unsigned int xwmLen;
      unsigned int pcmBytePos;
      unsigned int pcmLen;
      float packetStartTime;
      float packetEndTime;
   };

   struct AudioTrack 
   {
      AudioTrack() : pBuffer(NULL), bufferSize(0), currAudioPacket(0) {}

      ~AudioTrack() 
      {
         delete [] pBuffer;
         audioFrames.clear();
      }

      bool HasRemainingPackets() const {
         return currAudioPacket < audioFrames.size();
      }

      XAudioStreamHeader audioHeader;
      std::vector<AudioFrame> audioFrames;
      unsigned char* pBuffer;
      unsigned int bufferSize;
      unsigned int currAudioPacket;
   };

   CStreamWriter_XAudio();

   virtual bool ReplacementFileExistsForCurrStream(EPlatform curPlatform) const;
   virtual void WriteOnePacket(FILE * const streamfp, EPlatform curPlatform, const int currTick);
   virtual void EndStream(FILE * const streamfp, EPlatform curPlatform, STREAM_TAG* pEndTag);
   virtual bool ReplacesDataForCurrStream(const unsigned int type);
   virtual unsigned int GetRebuildPlatformMask() const;

   //Needed for CStreamWriter_PCM()...
   static bool  ReplacementFileExistsForCurrStream_Static(EPlatform curPlatform);
private:

   void WriteSoundPacket(FILE * streamfp, const int currTick);

   // Only supports a single track, but a track can have multiple channels.
   AudioTrack* m_pAudioTrack;
};
