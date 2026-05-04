//------------------------------------------------------------------------------------------
// CGameObjectRegisteredComponentGroupObjectFactories.cpp
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#include "StdAfx.h"

//------------------------------------------------------------------------------------------

#include "CGameObjectRegisteredComponentGroupObjectFactories.h"
#include "Engine/GameObjectSystem/CGameObjectApplyPropertiesData.h"

//------------------------------------------------------------------------------------------

CGameObjectRegisteredComponentGroupObjectFactories::CGameObjectRegisteredComponentGroupObjectFactories()
{
   // Initialize if not initialized already.
   if (mInitializedToken != 0x1234567812345678ULL)
   {
      mFactories = new TFactoryMap();
      mInitializedToken = 0x1234567812345678ULL;
   }
}

//------------------------------------------------------------------------------------------

CGameObjectRegisteredComponentGroupObjectFactories::~CGameObjectRegisteredComponentGroupObjectFactories()
{
   mInitializedToken = 0;
   delete mFactories;
   //BPE_ASSERT(mFactories->empty(), "Not all factories unregistered before destroying CGameObjectRegisteredComponentGroupObjectFactories");
}

//------------------------------------------------------------------------------------------

void CGameObjectRegisteredComponentGroupObjectFactories::RegisterComponentGroupObjectFactory(uint32 const componentGroupType,
                                                                                             CGameObjectComponentProperties * (*pComponentPropertiesFactory)(CGameObjectApplyPropertiesData &, CEvaluatorAllocator *),
                                                                                             CGameObjectComponent * const     (*pComponentFactory)(boost::shared_ptr<CGameObjectComponentProperties const> const &))
{
   // Ughh! Force initialization because of static linkage
   // Constructor will check for if it was previously initialized
   new(this) CGameObjectRegisteredComponentGroupObjectFactories();

   // Add factory
   BPE_ASSERT(mFactories->find(componentGroupType) == mFactories->end(), "Duplicate componentGroup properties factory added.");
   
   (*mFactories)[componentGroupType] = TFactoryPair(pComponentPropertiesFactory, pComponentFactory);
}

//------------------------------------------------------------------------------------------

void CGameObjectRegisteredComponentGroupObjectFactories::RemoveRegisteredComponentGroupObjectFactory(uint32 const componentGroupType)
{
   if (mInitializedToken != 0x1234567812345678ULL) return;

   TFactoryMap::iterator iter = mFactories->find(componentGroupType);
   if (iter == mFactories->end())
   {
      // Not registered.
      return;
   }

   mFactories->erase(iter);
}

//------------------------------------------------------------------------------------------

CGameObjectComponentProperties * CGameObjectRegisteredComponentGroupObjectFactories::BuildGameObjectComponentProperties(uint32 const componentGroupType, CGameObjectApplyPropertiesData &applyProperties) const
{
   TFactoryMap::const_iterator iter = mFactories->find(componentGroupType);
   if (iter == mFactories->end())
   {
      return NULL;
   }
   
   // Call function to build properties
   CGameObjectComponentProperties * pComponentProperties = iter->second.first(applyProperties, NULL);
   return pComponentProperties;
}

//------------------------------------------------------------------------------------------

CGameObjectComponent * CGameObjectRegisteredComponentGroupObjectFactories::BuildGameObjectComponent(uint32 const componentGroupType, boost::shared_ptr<CGameObjectComponentProperties const> const &pComponentProperties) const
{
   TFactoryMap::const_iterator iter = mFactories->find(componentGroupType);
   if (iter == mFactories->end())
   {
      return NULL;
   }
   
   // Call function to build uninitialized properties
   CGameObjectComponent * pComponent= iter->second.second(pComponentProperties);
   return pComponent;
}

//------------------------------------------------------------------------------------------

bool CGameObjectRegisteredComponentGroupObjectFactories::ComponentGroupFactoryExists(uint32 const componentGroupType)
{
   TFactoryMap::const_iterator iter = mFactories->find(componentGroupType);
   if (iter == mFactories->end())
   {
      return false;
   }
   return true;
}

//------------------------------------------------------------------------------------------

CGameObjectRegisterComponentGroupObjectFactory::CGameObjectRegisterComponentGroupObjectFactory( CGameObjectRegisteredComponentGroupObjectFactories &registration,                                                 
                                                                                                uint32 const componentGroupType,                                                                                  
                                                                                                CGameObjectComponentProperties * (*pComponentPropertiesFactory)(CGameObjectApplyPropertiesData &, CEvaluatorAllocator *),
                                                                                                CGameObjectComponent * const     (*pComponentFactory)(boost::shared_ptr<CGameObjectComponentProperties const> const &))
 : mRegistration(registration)
 , mComponentGroupType(componentGroupType)
{
   registration.RegisterComponentGroupObjectFactory(componentGroupType, pComponentPropertiesFactory, pComponentFactory);
}

//------------------------------------------------------------------------------------------

CGameObjectRegisterComponentGroupObjectFactory::~CGameObjectRegisterComponentGroupObjectFactory()
{
   mRegistration.RemoveRegisteredComponentGroupObjectFactory(mComponentGroupType);
}

//------------------------------------------------------------------------------------------

