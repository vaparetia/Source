//----------------------------------------------------------------------------
// CTCPOutputStream.h
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Streams/COutputStream.h"

//----------------------------------------------------------------------------

class CTCPClientSocket;

//----------------------------------------------------------------------------

class CTCPOutputStream : public COutputStream
{
public:
   ENGINE_API explicit CTCPOutputStream(CTCPClientSocket & socket, const uint32 bufferSize = 4096);
   ENGINE_API virtual ~CTCPOutputStream();

protected:
   virtual void Write(const void *buffer, const uint32 bufferSize);

private:
   CTCPClientSocket &   mSocket;

private:
   BPE_DISABLE_COPY_AND_ASSIGNMENT( CTCPOutputStream );
};

//----------------------------------------------------------------------------


