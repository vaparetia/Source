//----------------------------------------------------------------------------
// CTCPInputStream.cpp
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CTCPInputStream.h"

//----------------------------------------------------------------------------

#include "Engine/Network/CTCPClientSocket.h"
#if BPE_TARGET == BPE_TARGET_WIN32
#include "Engine/Network/mySocket.h"
#endif
//----------------------------------------------------------------------------

CTCPInputStream::CTCPInputStream(CTCPClientSocket & socket, const uint32 bufferSize)
:  CInputStream(bufferSize)
,  mSocket( socket )
{
}

//----------------------------------------------------------------------------

const uint32 CTCPInputStream::Read(void *pBuffer, const uint32 bufferSize)
{
#if BPE_TARGET == BPE_TARGET_WIN32
   int const socketId = mSocket.mpSocket->getSocketId();
   int const receivedBytes = recv( socketId, (char*)pBuffer, bufferSize, 0 );
   if( receivedBytes > 0 )
   {
      return receivedBytes;
   }
#endif

   return 0;
}

//----------------------------------------------------------------------------

