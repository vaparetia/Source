//----------------------------------------------------------------------------
// CTCPInputStream.h
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Streams/CInputStream.h"

//----------------------------------------------------------------------------

class CTCPClientSocket;

//----------------------------------------------------------------------------

class CTCPInputStream : public CInputStream
{
public:
   ENGINE_API explicit CTCPInputStream(CTCPClientSocket & socket, const uint32 bufferSize = 4096);

   bool const HasData() const { return GetUnreadBufferLength() > 0; }

protected:
   virtual const uint32 Read(void *pBuffer, const uint32 bufferSize);

private:
   CTCPClientSocket &   mSocket;

private:
   BPE_DISABLE_COPY_AND_ASSIGNMENT( CTCPInputStream );
};

//----------------------------------------------------------------------------

