//----------------------------------------------------------------------------

#pragma once

#include <vector>

#include "CStreamWriter.h"

//----------------------------------------------------------------------------

class CStreamWriter_Demo : public CStreamWriter
{
public:
   CStreamWriter_Demo();

   virtual bool ReplacementFileExistsForCurrStream(EPlatform curPlatform) const;
   virtual void WriteOnePacket(FILE * const streamfp, EPlatform curPlatform, const int currTick);
   virtual void ModifyCurrentSourcePacket( STREAM_TAG & streamTag, unsigned char * const packetBuf, EPlatform curPlatform );

   virtual void EndStream(FILE * const streamfp);
   virtual bool ReplacesDataForCurrStream( const unsigned int type );
   virtual unsigned int GetRebuildPlatformMask() const;

private:

   enum EWritingState
   {
      kWritingState_None,
      kWritingState_Pending,
      kWritingState_Finished
   };

private:
   void         DoWriteOnePacket( FILE * const streamfp, const int currTick );

private:
   FILE *               mpSourceFile;

   EWritingState        mWritingState;
};

//----------------------------------------------------------------------------
