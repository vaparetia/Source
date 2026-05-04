//----------------------------------------------------------------------------
// CRegisteredEvaluatorFactories.h
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "map"

//----------------------------------------------------------------------------
class CInputStream;
class IEvaluator;
class IEvaluatorFactory;
class CEvaluatorAllocator;
//----------------------------------------------------------------------------

class ENGINE_API CRegisteredEvaluatorFactories
{
public:
   typedef IEvaluator * (TEvaluatorFactory)(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);

   CRegisteredEvaluatorFactories();
   ~CRegisteredEvaluatorFactories();

   void           RegisterEvaluatorFactory(uint32 const evaluatorType, TEvaluatorFactory *pFactory);
   void           RemoveRegisteredEvaluatorFactory(uint32 const evaluatorType);

   IEvaluator *   BuildEvaluator(uint32 const evaluatorType,
                                 CInputStream &inStream, 
                                 CEvaluatorAllocator &allocator, 
                                 IEvaluatorFactory const &factory) const;
   
private:
   typedef std::map<uint32, TEvaluatorFactory *>    TFactories;
   
   // Turn this into sorted vector later
   TFactories     mFactories;
};

//----------------------------------------------------------------------------
// Helper class to be used as a static to register factories on app start
class ENGINE_API CRegisterEvaluatorFactory
{
public:
   CRegisterEvaluatorFactory( CRegisteredEvaluatorFactories &registration, 
                              uint32 const evaluatorType, 
                              CRegisteredEvaluatorFactories::TEvaluatorFactory *pFactory);
   ~CRegisterEvaluatorFactory();

   CRegisteredEvaluatorFactories &  mRegistration;
   uint32                           mEvaluatorType;

   BPE_DISABLE_COPY_AND_ASSIGNMENT(CRegisterEvaluatorFactory);
};

//----------------------------------------------------------------------------

