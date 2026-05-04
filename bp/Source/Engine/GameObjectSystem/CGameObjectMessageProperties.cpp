//------------------------------------------------------------------------------------------
// CGameObjectMessageProperties.cpp
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#include "StdAfx.h"

//------------------------------------------------------------------------------------------

#include "CGameObjectMessageProperties.h"

//------------------------------------------------------------------------------------------
CGameObjectMessageProperties::CGameObjectMessageProperties()
{
}

//------------------------------------------------------------------------------------------

CGameObjectMessageProperties::~CGameObjectMessageProperties()
{
}

//------------------------------------------------------------------------------------------

CGameObjectMessageProperties * const CGameObjectMessageProperties::ProcessUnknownMessageType(uint32 const messageType, uint16 const messageSize, CInputStream &inStream)
{
   // Check to see if it's a message without any properties
   uint32 const kMessageWithoutProperties = 0xFFFFFFFF;
   if (messageType == kMessageWithoutProperties)
   {
      uint32 const emptyMessageType = inStream.ReadUint32();
      return new CGameObjectMessagePropertiesEmpty(emptyMessageType);
   }
   // Unknown message properties, skip data
   inStream.Get(NULL, messageSize);
   bpe_debugger_and_console_printf("Skipping data for unknown message type: 0x%08x\n", messageType);

   // Higher level code must handle NULL properties.
   return NULL;
}

//------------------------------------------------------------------------------------------
