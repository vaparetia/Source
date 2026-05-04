//----------------------------------------------------------------------------

#pragma once

#include "streamtool.h"

class CStreamDriver
{
public:
   virtual void ProcessPacket( const STREAM_TAG & packet, const void * const pBody ) = 0;
   virtual void EndStream() = 0;
};

//----------------------------------------------------------------------------
