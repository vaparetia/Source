//------------------------------------------------------------------------------------------
// CGameObjectComponentProperties.h
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

#include "Engine/GameObjectSystem/TGameObjectTypes.h"
#include "Engine/GameObjectSystem/CGameObjectMessage.h"
#include "Engine/GameObjectSystem/CGameObjectApplyPropertiesData.h"
#include "Engine/Evaluators/CEvaluatorAllocator.h"

//------------------------------------------------------------------------------------------

class IGameObjectFactory;
class CInputStream;

//------------------------------------------------------------------------------------------

class ENGINE_API CGameObjectComponentProperties
{
public:
   explicit CGameObjectComponentProperties(TComponentEditorId const &editorId);
   virtual ~CGameObjectComponentProperties();

   //virtual void	ApplyProperties(CInputStream &stream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory) = 0;
   virtual void	ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator) = 0;
   virtual void	PostLoadUpdate() = 0;
   virtual uint32	GetComponentType() const = 0;
   virtual uint32	GetComponentGroupType() const = 0;
   virtual CGameObjectComponentProperties * 
                  clone() const = 0;  // Virtual copy constructor, override this if you start deriving from this class
   virtual char const *
                  GetComponentGroupName(void) const = 0;
   virtual char const *
                  GetComponentTypeName(void) const = 0;

   // Read messages and links after the object has been constructed
   void           AddMessagesAndLinks(CInputStream &inStream, IGameObjectFactory &factory);

   // Helper function used to update existing component properties, returns true if properties were updated
   static bool    UpdateProperties( CGameObjectComponentProperties * pProperties, 
                                    CGameObjectApplyPropertiesData &applyProperties, 
                                    CEvaluatorAllocator * pAllocator);

   TComponentEditorId                  mEditorId;
   std::vector< CGameObjectMessage >   mMessages;
   std::vector< CGameObjectMessage >   mLinks;
   CEvaluatorAllocator                 mEvaluatorAllocator;
};

//------------------------------------------------------------------------------------------

