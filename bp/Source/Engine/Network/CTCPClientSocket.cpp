//----------------------------------------------------------------------------
// CTCPClientSocket.cpp
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CTCPClientSocket.h"

//----------------------------------------------------------------------------

#include "Engine/Network/CTCPInputStream.h"
#include "Engine/Network/CTCPOutputStream.h"
#if BPE_TARGET == BPE_TARGET_WIN32
#include "mySocket.h"
#else
class myTcpSocket {};
#endif

//----------------------------------------------------------------------------

static bool sSocketsInitialized = false;

//----------------------------------------------------------------------------

CTCPClientSocket::CTCPClientSocket( std::string const & server, int const port, EType const type )
{
   InitSockets();

#if BPE_TARGET == BPE_TARGET_WIN32
   mpSocket.reset( new myTcpSocket(port) );

   mpSocket->setLingerOnOff(true);
   mpSocket->setLingerSeconds(10);
   mpSocket->setSocketBlocking(0);

   if( !mpSocket->connectToServer(server,(type == kType_Address) ? ADDRESS : NAME) )
   {
      mpSocket.reset();
   }
#endif
   mpInStream.reset(new CTCPInputStream( *this ) );
   mpOutStream.reset( new CTCPOutputStream( *this ) );
}

//----------------------------------------------------------------------------

CTCPClientSocket::~CTCPClientSocket()
{
}

//----------------------------------------------------------------------------

void CTCPClientSocket::InitSockets()
{
   if( !sSocketsInitialized )
   {
      sSocketsInitialized = true;

#if BPE_TARGET == BPE_TARGET_WIN32
      WSADATA wsaData;
      WSAStartup(0x101, &wsaData);
#endif
   }
}

//----------------------------------------------------------------------------

bool const CTCPClientSocket::HasData() const
{
#if BPE_TARGET == BPE_TARGET_WIN32
   // if there is still remaining data in the stream, return that we have data
   if( mpInStream.get() && mpInStream->HasData() )
   {
      return true;
   }

   // check if there is data on the socket ready for receiving
   if( mpSocket.get() )
   {
      int socketId = mpSocket->getSocketId();
      u_long bytes = 0;
      if( !ioctlsocket( socketId, FIONREAD, &bytes ) )
      {
         return bytes != 0;
      }
   }
#endif
   return false;
}

//----------------------------------------------------------------------------

CInputStream & CTCPClientSocket::InputStream()
{
   return *mpInStream;
}

//----------------------------------------------------------------------------

COutputStream & CTCPClientSocket::OutputStream()
{
   return *mpOutStream;
}

//----------------------------------------------------------------------------

