//----------------------------------------------------------------------------
// CGameObjectRegisteredComponentGroupObjectFactories.h
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "boost/shared_ptr.hpp"
#include "boost/optional.hpp"
#include "map"

//----------------------------------------------------------------------------
class CInputStream;
class CGameObjectComponentProperties;
class CGameObjectComponent;
class CGameObjectApplyPropertiesData;
class IEvaluatorFactory;
class CEvaluatorAllocator;
//----------------------------------------------------------------------------

class ENGINE_API CGameObjectRegisteredComponentGroupObjectFactories
{
public:
   CGameObjectRegisteredComponentGroupObjectFactories();
   ~CGameObjectRegisteredComponentGroupObjectFactories();

   void  RegisterComponentGroupObjectFactory(uint32 const componentGroupType,
                                             CGameObjectComponentProperties * (*pComponentPropertiesFactory)(CGameObjectApplyPropertiesData &, CEvaluatorAllocator *),
                                             CGameObjectComponent * const     (*pComponentFactory)(boost::shared_ptr<CGameObjectComponentProperties const> const &));

   void  RemoveRegisteredComponentGroupObjectFactory(uint32 const componentGroupType);

   CGameObjectComponentProperties * 
         BuildGameObjectComponentProperties(uint32 const componentGroupType, CGameObjectApplyPropertiesData &applyProperties) const;
   CGameObjectComponent * 
         BuildGameObjectComponent(uint32 const componentGroupType, boost::shared_ptr<CGameObjectComponentProperties const> const &pComponentProperties) const;

   bool  ComponentGroupFactoryExists(uint32 const componentGroupType);
   
private:
   typedef std::pair<CGameObjectComponentProperties * (*)(CGameObjectApplyPropertiesData &, CEvaluatorAllocator *), CGameObjectComponent * const (*)(boost::shared_ptr<CGameObjectComponentProperties const> const &)>   TFactoryPair;
   typedef std::map<uint32, TFactoryPair>    TFactoryMap;
   
   // Turn this into sorted vector later
   TFactoryMap *  mFactories;
   uint64         mInitializedToken;      // Hacky method to determine if this has been initialized when linking static libs
};

//----------------------------------------------------------------------------
// Helper class to be used as a static to register factories on app start
class ENGINE_API CGameObjectRegisterComponentGroupObjectFactory
{
public:
   CGameObjectRegisterComponentGroupObjectFactory( CGameObjectRegisteredComponentGroupObjectFactories &registration, 
                                                   uint32 const componentGroupType,                                                                                          
                                                   CGameObjectComponentProperties * (*pComponentPropertiesFactory)(CGameObjectApplyPropertiesData &, CEvaluatorAllocator *),                                          
                                                   CGameObjectComponent * const     (*pComponentFactory)(boost::shared_ptr<CGameObjectComponentProperties const> const &)); 

   ~CGameObjectRegisterComponentGroupObjectFactory();

   CGameObjectRegisteredComponentGroupObjectFactories &  mRegistration;
   uint32                                                mComponentGroupType;

   BPE_DISABLE_COPY_AND_ASSIGNMENT(CGameObjectRegisterComponentGroupObjectFactory);
};

//----------------------------------------------------------------------------

