//------------------------------------------------------------------------------------------
// CRegisteredEvaluatorFactories.cpp
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#include "StdAfx.h"

//------------------------------------------------------------------------------------------

#include "Engine/Streams/CInputStream.h"
#include "CRegisteredEvaluatorFactories.h"
#include "IEvaluator.h"
#include "IEvaluatorFactory.h"
#include "CEvaluatorAllocator.h"

//------------------------------------------------------------------------------------------

CRegisteredEvaluatorFactories::CRegisteredEvaluatorFactories()
{
}

//------------------------------------------------------------------------------------------

CRegisteredEvaluatorFactories::~CRegisteredEvaluatorFactories()
{
}

//------------------------------------------------------------------------------------------

void CRegisteredEvaluatorFactories::RegisterEvaluatorFactory(uint32 const evaluatorType, TEvaluatorFactory *pFactory)
{
   // Add factory
   BPE_ASSERT(evaluatorType != 0, "0 is reserved for NULL evaluators");
   BPE_ASSERT(mFactories.find(evaluatorType) == mFactories.end(), "Duplicate evaluator factory added.");
   
   mFactories[evaluatorType] = pFactory;
}

//------------------------------------------------------------------------------------------

void CRegisteredEvaluatorFactories::RemoveRegisteredEvaluatorFactory(uint32 const evaluatorType)
{
   TFactories::iterator entry = mFactories.find(evaluatorType);
   BPE_ASSERT(entry != mFactories.end(), "Removing unregistered factory.");
   mFactories.erase(entry);
}

//------------------------------------------------------------------------------------------

IEvaluator * CRegisteredEvaluatorFactories::BuildEvaluator(uint32 const evaluatorType, CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory) const
{
   // Explicit check for NULL evaluators
   if (evaluatorType == 0) return NULL;

   TFactories::const_iterator iter = mFactories.find(evaluatorType);
   if (iter == mFactories.end())
   {
      return NULL;
   }
   
   // Call function to build evaluator
   IEvaluator * pEvaluator = iter->second(inStream, allocator, factory);
   return pEvaluator;
}

//------------------------------------------------------------------------------------------

CRegisterEvaluatorFactory::CRegisterEvaluatorFactory( CRegisteredEvaluatorFactories &registration, 
                                                      uint32 const evaluatorType,
                                                      CRegisteredEvaluatorFactories::TEvaluatorFactory *pFactory)
 : mRegistration(registration)
 , mEvaluatorType(evaluatorType)
{
   registration.RegisterEvaluatorFactory(evaluatorType, pFactory);
}

//------------------------------------------------------------------------------------------

CRegisterEvaluatorFactory::~CRegisterEvaluatorFactory()
{
   mRegistration.RemoveRegisteredEvaluatorFactory(mEvaluatorType);
}

//------------------------------------------------------------------------------------------

