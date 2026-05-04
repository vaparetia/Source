//----------------------------------------------------------------------------

#pragma once

#include "streamtool.h"

class CStreamWriter
{
public:
   virtual bool ReplacementFileExistsForCurrStream(EPlatform curPlatform) const = 0;
   virtual void WriteOnePacket(FILE * const streamfp, EPlatform curPlatform, const int currTick) = 0;
   virtual void ModifyCurrentSourcePacket( STREAM_TAG & /*streamTag*/, unsigned char * const /*packetBuf*/, EPlatform /*curPlatform*/ ) {}

   virtual void EndStream(FILE * const streamfp, EPlatform curPlatform, STREAM_TAG* pEndTag) = 0;
   virtual bool ReplacesDataForCurrStream( const unsigned int type ) = 0;

   virtual void WriteOutInfo( STREAM_TAG & streamTag) { streamTag; }
   virtual void FinishInfo() {}
   //For what platforms does this stream writer type force a build?
   virtual unsigned int GetRebuildPlatformMask() const = 0;

   virtual ~CStreamWriter() {}
};

//----------------------------------------------------------------------------
