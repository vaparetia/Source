//----------------------------------------------------------------------------
// CTCPOutputStream.cpp
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CTCPOutputStream.h"

//----------------------------------------------------------------------------

#include "Engine/Network/CTCPClientSocket.h"
#if BPE_TARGET == BPE_TARGET_WIN32
#include "Engine/Network/mySocket.h"
#endif

//----------------------------------------------------------------------------

CTCPOutputStream::CTCPOutputStream(CTCPClientSocket & socket, const uint32 bufferSize)
:  COutputStream(bufferSize)
,  mSocket( socket )
{
}

//----------------------------------------------------------------------------

CTCPOutputStream::~CTCPOutputStream()
{
   Flush();
}

//----------------------------------------------------------------------------

void CTCPOutputStream::Write(const void *buffer, const uint32 bufferSize)
{
   if( mSocket.IsConnected() )
   {
#if BPE_TARGET == BPE_TARGET_WIN32
      int const socketId = mSocket.mpSocket->getSocketId();
      int const numBytes = send(socketId, reinterpret_cast<char const*>(buffer), bufferSize, 0 );

      //BPE_VERIFY(numBytes == bufferSize, false, "Couldn't send data");
#endif
   }
}

//----------------------------------------------------------------------------


