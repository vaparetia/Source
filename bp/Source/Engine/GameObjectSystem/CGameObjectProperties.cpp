//------------------------------------------------------------------------------------------
// CGameObjectProperties.cpp
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/GameObjectSystem/CGameObjectProperties.h"
#include "Engine/GameObjectSystem/IGameObjectFactory.h"

//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------

CGameObjectProperties::CGameObjectProperties(TGameObjectEditorId const & editorId, 
                                             std::string const &editorName, 
                                             uint32 const flags, 
                                             TComponentProperties const &componentProperties)
:  mEditorId(editorId)
,  mEditorName(editorName)
,  mFlags(flags)
,  mComponentProperties(componentProperties)
{
}

//------------------------------------------------------------------------------------------

CGameObjectProperties::~CGameObjectProperties()
{
}

//------------------------------------------------------------------------------------------

CGameObjectProperties * const CGameObjectProperties::FGameObjectProperties(CInputStream &inStream, IGameObjectFactory * const pFactory)
{
   int32 const version = inStream.ReadInt32();
   BPE_VERIFY(version == kVersion, false, "Version mismatch!");

   // Get id
   TGameObjectEditorId const editorId = TGameObjectEditorId(inStream);
   // Editor name, used for debugging and logging
   std::string const editorName(inStream.ReadString());
   // Flags
   uint32 const flags = inStream.ReadUint32();

   CGameObjectProperties * const pGameObjectProperties = new CGameObjectProperties(editorId, editorName, flags, TComponentProperties());

   int numComponents = inStream.ReadInt32();
   pGameObjectProperties->mComponentProperties.reserve(numComponents);

   for (int loop = 0; loop < numComponents; loop++)
   {
      // Use factory to build components
      CGameObjectComponentProperties * const pComponentProperties = pFactory->BuildComponentProperties(inStream);
      if (pComponentProperties != NULL)
      {
         // Add messages and links written after component
         pComponentProperties->AddMessagesAndLinks(inStream, *pFactory);
         pGameObjectProperties->mComponentProperties.push_back(boost::shared_ptr<CGameObjectComponentProperties const>(pComponentProperties));
      }
   }
   pGameObjectProperties->mChildGameObjectProperties = FChildGameObjectProperties(inStream, pFactory);

   return pGameObjectProperties;
}

//------------------------------------------------------------------------------------------

CGameObjectProperties::TChildGameObjectProperties CGameObjectProperties::FChildGameObjectProperties(CInputStream &inStream, IGameObjectFactory * const pFactory)
{
   TChildGameObjectProperties childGameObjectProperties;
    // Now load child properties
   int numChildProperties = inStream.ReadInt32();
   childGameObjectProperties.reserve(numChildProperties);

   for (int loop = 0; loop < numChildProperties; loop++)
   {
      // Use recursion to build child properties
      CGameObjectProperties * const pChildGOProperties = FGameObjectProperties(inStream, pFactory);
      childGameObjectProperties.push_back(boost::shared_ptr<CGameObjectProperties const>(pChildGOProperties));
   }

   return childGameObjectProperties;
}

//------------------------------------------------------------------------------------------

