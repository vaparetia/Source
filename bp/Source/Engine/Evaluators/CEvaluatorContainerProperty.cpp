//----------------------------------------------------------------------------
// CEvaluatorContainerProperty.cpp
// Interface class for building engine evaluators
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#include "StdAfx.h"

#include "CEvaluatorContainerProperty.h"
#include "IEvaluator.h"
#include "IEvaluatorFactory.h"
#include "Engine/Streams/CInputStream.h"
#include "Engine/GameObjectSystem/CGameObjectApplyPropertiesData.h"

//----------------------------------------------------------------------------

CEvaluatorContainerEntry::CEvaluatorContainerEntry(EEvaluatorType const type, 
                                                   std::vector<TComponentEditorId> const &targetComponents, 
                                                   TTargetProperty const &targetProperty, 
                                                   IEvaluator * const pEvaluator)
: mType(type)
, mTargetComponents(targetComponents)
, mTargetProperty(targetProperty)
, mpEvaluator(pEvaluator)
{
}

//----------------------------------------------------------------------------

CEvaluatorContainerEntry::~CEvaluatorContainerEntry()
{
}

//----------------------------------------------------------------------------

boost::shared_ptr<CEvaluatorContainerEntry> CEvaluatorContainerEntry::FFactory(CInputStream &stream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   EEvaluatorType const type = static_cast<EEvaluatorType>(stream.ReadUint32());
   std::vector<TComponentEditorId> const targetComponents = bpe::vector_s<TComponentEditorId>(stream);
   TTargetProperty const targetProperty = bpe::vector_s<uint8>(stream);
   IEvaluator * pEvaluator = NULL;
   uint16 const size = stream.ReadUint16();
   uint32 const evaluatorType = stream.ReadUint32();
   switch (type)
   {
      case kET_Float:
         pEvaluator = factory.BuildFloatEvaluator(evaluatorType, size, stream, allocator);
         break;
      case kET_Bool:
         pEvaluator = factory.BuildBoolEvaluator(evaluatorType, size, stream, allocator);
         break;
      case kET_Vector:
         pEvaluator = factory.BuildVectorEvaluator(evaluatorType, size, stream, allocator);
         break;
      case kET_Color:
         pEvaluator = factory.BuildColorEvaluator(evaluatorType, size, stream, allocator);
         break;
   }

   BPE_ASSERT( (evaluatorType == 0) || (pEvaluator), "Couldn't build evaluator.");
   return boost::shared_ptr<CEvaluatorContainerEntry>(new CEvaluatorContainerEntry(type, targetComponents, targetProperty, pEvaluator));
}

//----------------------------------------------------------------------------

CEvaluatorContainerProperty::CEvaluatorContainerProperty()
{
}

//----------------------------------------------------------------------------

CEvaluatorContainerProperty::~CEvaluatorContainerProperty()
{
}

//----------------------------------------------------------------------------

void CEvaluatorContainerProperty::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties)
{
   mEvaluatorAllocator = CEvaluatorAllocator();
   mEntries.clear();
   // Read number of properties
   int const numEvaluators = applyProperties.mStream.ReadInt32();
   mEntries.reserve(numEvaluators);
   for (int loop = 0; loop < numEvaluators; loop++)
   {
      mEntries.push_back(CEvaluatorContainerEntry::FFactory(applyProperties.mStream, mEvaluatorAllocator, applyProperties.mFactory));
   }
}

//----------------------------------------------------------------------------

void CEvaluatorContainerProperty::Initialize(CEvaluatorUpdateData &updateData) const
{
   for (int loop = 0; loop < mEntries.size(); ++loop)
   {
      mEntries[loop]->mpEvaluator->Initialize(updateData);
   }
}
   
//----------------------------------------------------------------------------
   
