//------------------------------------------------------------------------------------------
// CGameObjectComponentProperties.cpp
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#include "StdAfx.h"

//------------------------------------------------------------------------------------------

#include "Engine/GameObjectSystem/CGameObjectComponentProperties.h"
#include "Engine/GameObjectSystem/CGameObjectMessage.h"
#include "Engine/GameObjectSystem/IGameObjectFactory.h"
#include "Engine/GameObjectSystem/CGameObjectApplyPropertiesData.h"

//------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------

CGameObjectComponentProperties::CGameObjectComponentProperties(TComponentEditorId const &editorId)
:  mEditorId(editorId)
{
}

//------------------------------------------------------------------------------------------

CGameObjectComponentProperties::~CGameObjectComponentProperties()
{
}

//------------------------------------------------------------------------------------------

void CGameObjectComponentProperties::AddMessagesAndLinks(CInputStream &inStream, IGameObjectFactory &factory)
{
   // Read messages and links from stream
   int const numMessages = inStream.ReadInt32();
   mMessages.reserve(numMessages);
   for (int loop = 0; loop < numMessages; loop++)
   {
      CGameObjectMessage const message(CGameObjectMessage::FMessageFactory(inStream, factory));

      // Check for valid message properties in case message type was unknown.
      if (message.mpProperties)
      {
         // Message valid.
         mMessages.push_back(message);
      }
   }

   // Links are the same as messages internally
   int const numLinks = inStream.ReadInt32();
   mLinks.reserve(numLinks);
   for (int loop = 0; loop < numLinks; loop++)
   {
      mLinks.push_back(CGameObjectMessage::FMessageFactory(inStream, factory));
   }
}

//------------------------------------------------------------------------------------------

bool CGameObjectComponentProperties::UpdateProperties(CGameObjectComponentProperties * pProperties, 
                                                      CGameObjectApplyPropertiesData &applyProperties, 
                                                      CEvaluatorAllocator * pAllocator)
{
   uint32 const type = applyProperties.mStream.ReadUint32();
   uint16 const size = applyProperties.mStream.ReadUint16();

   if (!pProperties)
   {
      // NULL component, don't update
      applyProperties.mStream.Get(NULL, size);
      return false;
   }

   if (pProperties->GetComponentType() != type)
   {
      // Skip data
      applyProperties.mStream.Get(NULL, size);
      return false;
   }

   // If passed in allocator is null, use the component properties allocator
   pAllocator = pAllocator ? pAllocator : &pProperties->mEvaluatorAllocator;
   pProperties->ApplyProperties(applyProperties, *pAllocator);

   return true;
}

//------------------------------------------------------------------------------------------
