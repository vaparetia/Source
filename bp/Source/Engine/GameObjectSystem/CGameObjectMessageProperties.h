//----------------------------------------------------------------------------
// CGameObjectMessageProperties.h
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Evaluators/CEvaluatorAllocator.h"

//----------------------------------------------------------------------------

class CInputStream;
class IEvaluatorFactory;
class CGameObjectApplyPropertiesData;
class CEvaluatorUpdateData;

//----------------------------------------------------------------------------
class ENGINE_API CGameObjectMessageProperties
{
public:
   CGameObjectMessageProperties();
   virtual ~CGameObjectMessageProperties();

   virtual void   ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator) = 0;
   virtual void   PostLoadUpdate() = 0;
   virtual uint32 GetMessageType() const = 0;
   virtual void   InitializeEvaluators(CEvaluatorUpdateData &updateData) const = 0;

   static CGameObjectMessageProperties * const ProcessUnknownMessageType(uint32 const messageType, uint16 const messageSize, CInputStream &inStream);

   CEvaluatorAllocator                 mEvaluatorAllocator;
};

//----------------------------------------------------------------------------
// This class is used when we have a message that doesn't have any properties.
class ENGINE_API CGameObjectMessagePropertiesEmpty : public CGameObjectMessageProperties
{
public:
   CGameObjectMessagePropertiesEmpty(uint32 const messageType)
   : mMessageType(messageType)
   {
   };

   virtual ~CGameObjectMessagePropertiesEmpty() {};

   virtual void   ApplyProperties(CGameObjectApplyPropertiesData &, CEvaluatorAllocator &) {};
   virtual void   PostLoadUpdate() {};
   virtual uint32 GetMessageType() const { return mMessageType; };
   virtual void   InitializeEvaluators(CEvaluatorUpdateData &) const { };

   uint32         mMessageType;
};

//----------------------------------------------------------------------------


