//----------------------------------------------------------------------------
// CTCPClientSocket.h
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//----------------------------------------------------------------------------

class myTcpSocket;
class CTCPOutputStream;
class CTCPInputStream;

//----------------------------------------------------------------------------

class CTCPClientSocket
{
   friend class CTCPInputStream;
   friend class CTCPOutputStream;

public:
   enum EType
   {
      kType_Address,
      kType_HostName
   };

public:
   ENGINE_API explicit CTCPClientSocket( std::string const & server, int const port, EType const type );
   ENGINE_API ~CTCPClientSocket();

   ENGINE_API bool const HasData() const;
   bool const IsConnected() const { return mpSocket.get() != NULL; }

   ENGINE_API CInputStream & InputStream();
   ENGINE_API COutputStream & OutputStream();

private:
   static void InitSockets();

private:
   std::auto_ptr<myTcpSocket>       mpSocket;
   std::auto_ptr<CTCPInputStream>   mpInStream;
   std::auto_ptr<CTCPOutputStream>  mpOutStream;
};

//----------------------------------------------------------------------------

