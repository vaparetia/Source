//------------------------------------------------------------------------------------------
// CGameObjectRegisteredMessageObjectFactories.cpp
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#include "StdAfx.h"

//------------------------------------------------------------------------------------------

#include "CGameObjectRegisteredMessageObjectFactories.h"

//------------------------------------------------------------------------------------------

CGameObjectRegisteredMessageObjectFactories::CGameObjectRegisteredMessageObjectFactories()
{
   // Initialize if not initialized already.
   if (mInitializedToken != 0x1234567812345678ULL)
   {
      mFactories = new TFactories();
      mInitializedToken = 0x1234567812345678ULL;
   }
}

//------------------------------------------------------------------------------------------

CGameObjectRegisteredMessageObjectFactories::~CGameObjectRegisteredMessageObjectFactories()
{
   mInitializedToken = 0;
   delete mFactories;
}

//------------------------------------------------------------------------------------------

void CGameObjectRegisteredMessageObjectFactories::RegisterMessageObjectFactory(  uint32 const messageType, 
                                                                                 CGameObjectMessageProperties * (*pMessageObjectFactory)(void))
{
   // Ughh! Force initialization because of static linkage
   // Constructor will check for if it was previously initialized
   new(this) CGameObjectRegisteredMessageObjectFactories();

   // Add factory
   BPE_ASSERT(mFactories->find(messageType) == mFactories->end(), "Duplicate message properties factory added.");
   
   (*mFactories)[messageType] = pMessageObjectFactory;
}

//------------------------------------------------------------------------------------------

void CGameObjectRegisteredMessageObjectFactories::RemoveRegisteredMessageObjectFactory(uint32 const messageType)
{
   if (mInitializedToken != 0x1234567812345678ULL) return;

   TFactories::iterator iter = mFactories->find(messageType);
   if (iter == mFactories->end())
   {
      // Not registered
      return;
   }
   
   mFactories->erase(iter);
}

//------------------------------------------------------------------------------------------
CGameObjectMessageProperties * CGameObjectRegisteredMessageObjectFactories::BuildGameObjectMessageProperties(uint32 const messageType) const
{
   TFactories::const_iterator iter = mFactories->find(messageType);
   if (iter == mFactories->end())
   {
      return NULL;
   }
   
   // Call function to build uninitialized properties
   CGameObjectMessageProperties * pMessageProperties = iter->second();
   return pMessageProperties;
}

//------------------------------------------------------------------------------------------

CGameObjectRegisterMessageObjectFactory::CGameObjectRegisterMessageObjectFactory(CGameObjectRegisteredMessageObjectFactories &registration, 
                                                                                 uint32 const messageType, 
                                                                                 CGameObjectMessageProperties * (*pMessageObjectFactory)(void))
 : mRegistration(registration)
 , mMessageType(messageType)
{
   registration.RegisterMessageObjectFactory(messageType, pMessageObjectFactory);
}

//------------------------------------------------------------------------------------------

CGameObjectRegisterMessageObjectFactory::~CGameObjectRegisterMessageObjectFactory()
{
   mRegistration.RemoveRegisteredMessageObjectFactory(mMessageType);
}

//------------------------------------------------------------------------------------------

