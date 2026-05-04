#pragma once

#include "CStreamWriter.h"
#include "vector"
#include "map"

class CStreamWriter_Movies : public CStreamWriter
{
   struct SFrameInfo
   {
      SFrameInfo(int frame, int startOffset, int size)
         : mFrame(frame)
         , mStartOffset(startOffset)
         , mSize(size)
      {
      }

      int mFrame;
      int mStartOffset;
      int mSize;
   };
public:

   CStreamWriter_Movies() 
      : mHasWrittenOutInfoPackets(false)
      , m_pMovieSource__fp(NULL)
      , m_memoryBuffer(NULL)
      , m_bufferPos(0)
      , m_totalBytes(0)
      , m_startWritingPackets(false)
      , mpCurrentReplacementFileName(NULL)
      , maReplacementFileNameIndex(0)
      , IPU_info__fp(NULL)
      , IPU_info__conversionlist_fp(NULL)
      , IPU_info__hasInfo(false) 
      , mMovieLength(0.0f)
      , mLastMovieLength(0.0f)
      , m_prevTime(0.0f)
   {
   }

   virtual bool ReplacementFileExistsForCurrStream(EPlatform curPlatform) const;
   virtual void WriteOnePacket(FILE * const streamfp, EPlatform curPlatform, const int currTick);
   virtual void ModifyCurrentSourcePacket( STREAM_TAG &streamTag, unsigned char * const packetBuf, EPlatform curPlatform );
   virtual void EndStream(FILE * const streamfp, EPlatform curPlatform, STREAM_TAG* pEndTag);
   virtual bool ReplacesDataForCurrStream(const unsigned int type);
   virtual void WriteOutInfo( STREAM_TAG & streamTag);
   virtual void FinishInfo();
   void LoadUpNextReplacementFile(EPlatform curPlatform);
   virtual unsigned int GetRebuildPlatformMask() const;

   virtual ~CStreamWriter_Movies();

private:
   void WriteMoviePacket(FILE * streamfp, EPlatform curPlatform, const int currTick, int const maxPacketSize);

   bool mHasWrittenOutInfoPackets;
   FILE* m_pMovieSource__fp;
   unsigned char* m_memoryBuffer;
   unsigned int m_bufferPos;
   unsigned int m_totalBytes;
   float m_prevTime;

   // In the case of the ending movie, it cannot start playing at the beginning of the stream but rather somewhere in 
   // the middle of the stream. This flag is set to true when writing can begin.
   bool m_startWritingPackets;

   int mMovieDataStartTick;

   mutable std::string* mpCurrentReplacementFileName;
   mutable int maReplacementFileNameIndex;
   mutable std::vector<std::string> maReplacementFileName;
   std::vector<SFrameInfo> mFrameInfos;
   std::vector<int> mVideoFrameByteSizes;
   float mFrameRate;
   float mMovieLength;
   float mLastMovieLength;

   // // IPU info
   bool IPU_info__hasInfo;
   FILE* IPU_info__fp;
   FILE* IPU_info__conversionlist_fp;
   int IPU_info__prevTick;
   int IPU_info__movieDataStartTick;
   struct IPU_INFO_STRUCT
   {
      int count;
      int totalSize;

      IPU_INFO_STRUCT(int _count, int _totalSize)
         : count(_count)
         , totalSize(_totalSize)
      {
      }

      // We need this to use with std::map
      IPU_INFO_STRUCT()
         : count(-1)
         , totalSize(-1)
      {
      }
   };
   std::map<int,IPU_INFO_STRUCT> IPU_info__mapDeltaTicks; // delta ticks to info_struct
};

void ChangeFPS_SingleFile();
