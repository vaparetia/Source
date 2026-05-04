//------------------------------------------------------------------------------------------
// CEvaluatorContainerProperty.h
// Container class for holding user specified lists of evaluators
// Also has target guids properties per evaluator
// Used for sending evaluator properties to components
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/System/CGuid.h"
#include "Engine/GameObjectSystem/TGameObjectTypes.h"
#include <vector>
#include "boost/scoped_ptr.hpp"
#include "boost/smart_ptr.hpp"
#include "CEvaluatorAllocator.h"

//------------------------------------------------------------------------------------------
class CInputStream;
class IEvaluator;
class IEvaluatorFactory;
class CEvaluatorUpdateData;
class CGameObjectApplyPropertiesData;
//------------------------------------------------------------------------------------------
class ENGINE_API CEvaluatorContainerEntry
{
public:
   enum EEvaluatorType
   {
      kET_Float,
      kET_Bool,
      kET_Vector,
      kET_Color,

      kET_Count
   };

   typedef std::vector<uint8>   TTargetProperty;  // memstream for property

   CEvaluatorContainerEntry(  EEvaluatorType const type, 
                              std::vector<TComponentEditorId> const &targetComponents, 
                              TTargetProperty const &targetProperty, 
                              IEvaluator * const pEvaluator);  // Takes ownership
   ~CEvaluatorContainerEntry();

   EEvaluatorType                   mType;
   std::vector<TComponentEditorId>  mTargetComponents;
   TTargetProperty                  mTargetProperty;
   boost::scoped_ptr<IEvaluator>    mpEvaluator;

   static boost::shared_ptr<CEvaluatorContainerEntry>  FFactory(CInputStream &stream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
};

//------------------------------------------------------------------------------------------

class ENGINE_API CEvaluatorContainerProperty
{
public:
   CEvaluatorContainerProperty();
   ~CEvaluatorContainerProperty();

   void ApplyProperties(CGameObjectApplyPropertiesData &applyProperties);
   void Initialize(CEvaluatorUpdateData &updateData) const;
   
   CEvaluatorAllocator                                         mEvaluatorAllocator;
   std::vector<boost::shared_ptr<CEvaluatorContainerEntry> >   mEntries;
};

//------------------------------------------------------------------------------------------
