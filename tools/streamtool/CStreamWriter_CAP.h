//----------------------------------------------------------------------------

#pragma once

#include <vector>

#include "CStreamWriter.h"

//----------------------------------------------------------------------------

class CStreamWriter_CAP : public CStreamWriter
{
public:
   CStreamWriter_CAP( const bool bDumpText, const int dumpLanguage );

   virtual bool ReplacementFileExistsForCurrStream(EPlatform curPlatform) const;
   virtual void WriteOnePacket(FILE * const streamfp, EPlatform curPlatform, const int currTick);
   virtual void EndStream(FILE * const streamfp, EPlatform curPlatform, STREAM_TAG* pEndTag);
   virtual bool ReplacesDataForCurrStream( const unsigned int type );
   virtual unsigned int GetRebuildPlatformMask() const;

private:
   bool didWriteData;
   bool mbDumpText;
   int  mDumpLanguage;
};

//----------------------------------------------------------------------------
