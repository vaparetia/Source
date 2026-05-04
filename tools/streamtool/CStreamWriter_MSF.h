//----------------------------------------------------------------------------

#pragma once

#include <vector>

#include "CStreamWriter.h"

//----------------------------------------------------------------------------

class CStreamWriter_MSF : public CStreamWriter
{
public:
   CStreamWriter_MSF();

   virtual bool ReplacementFileExistsForCurrStream(EPlatform curPlatform) const;
   virtual void WriteOnePacket(FILE * const streamfp, EPlatform curPlatform, const int currTick);
   virtual void EndStream(FILE * const streamfp, EPlatform curPlatform, STREAM_TAG* pEndTag);
   virtual bool ReplacesDataForCurrStream( const unsigned int type );
   virtual unsigned int GetRebuildPlatformMask() const;

   //Needed for CStreamWriter_PCM()...
   static bool  ReplacementFileExistsForCurrStream_Static(EPlatform curPlatform);
private:

   bool                          mbInitialized;
   int                           mCompressionType;
   int                           mChannels;
   int                           mFrequency;
   std::vector< unsigned char >  mSampleBuffer;
   int                           mSampleBytesWritten;
};

//----------------------------------------------------------------------------
