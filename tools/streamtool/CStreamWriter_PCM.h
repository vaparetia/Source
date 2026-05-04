//----------------------------------------------------------------------------

#pragma once

#include <vector>

#include "CStreamWriter.h"

//----------------------------------------------------------------------------

class CStreamWriter_PCM : public CStreamWriter
{
public:
   CStreamWriter_PCM();

   virtual bool ReplacementFileExistsForCurrStream(EPlatform curPlatform) const;
   virtual void WriteOnePacket(FILE * const streamfp, EPlatform curPlatform, const int currTick);
   virtual void EndStream(FILE * const streamfp, EPlatform curPlatform, STREAM_TAG* pEndTag);
   virtual bool ReplacesDataForCurrStream( const unsigned int type );
   virtual unsigned int GetRebuildPlatformMask() const;

private:
   void DoWritePacket( FILE * const streamfp );
   void UpdateTick();

private:

   bool                          mbInitialized;
   int                           mOriginalSize;
   int                           mNewSize;
   int                           mFrequency;
   int                           mChannels;
   int                           mFormat;
   std::vector< unsigned char >  mSampleBuffer;
   int                           mSampleBytesWritten;
   int                           mTick;
   int                           mStreamType;
};

//----------------------------------------------------------------------------
