//----------------------------------------------------------------------------

#pragma once

#include <vector>

#include "streamtool.h"

#include "CStreamDriver.h"

//----------------------------------------------------------------------------

class CStreamDriver_PCM : public CStreamDriver
{
public:
   CStreamDriver_PCM();

   void ProcessPacket( const STREAM_TAG & packet, const void * const pBody );
   void EndStream();

private:
   bool mbInitialized;

   int mFrequency;
   int mSize;
   int mChannel;
   int mFormat;

   std::vector< unsigned char >  mAdpcmBuffer;  //for adpcm streams
   std::vector< short >          mTrackBuffer;  //for str3 streams
};

//----------------------------------------------------------------------------
