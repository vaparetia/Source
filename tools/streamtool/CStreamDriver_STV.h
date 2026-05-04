//----------------------------------------------------------------------------

#pragma once

#include <vector>

#include "streamtool.h"

#include "CStreamDriver.h"

//----------------------------------------------------------------------------

class CStreamDriver_STV : public CStreamDriver
{
public:
   CStreamDriver_STV();

   virtual void ProcessPacket( const STREAM_TAG & packet, const void * const pBody );
   virtual void EndStream();

private:
   bool mbInitialized;

   int mFrequency;
   int mSize;
   int mChannel;
   int mDuration;
   int mPacketSize;

   std::vector< unsigned char >  mStvBuffer;
};

int DecodeADPCM(const signed char* inputData,int inputDataSize,char* outputData,int nrChannels, int interleave);

//----------------------------------------------------------------------------
