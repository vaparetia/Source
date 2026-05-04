//----------------------------------------------------------------------------
// CGameObjectRegisteredMessageObjectFactories.h
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "map"
#include "boost/optional.hpp"

//----------------------------------------------------------------------------
class CInputStream;
class CGameObjectMessageProperties;
//----------------------------------------------------------------------------

class ENGINE_API CGameObjectRegisteredMessageObjectFactories
{
public:
   CGameObjectRegisteredMessageObjectFactories();
   ~CGameObjectRegisteredMessageObjectFactories();

   void  RegisterMessageObjectFactory(uint32 const messageType, CGameObjectMessageProperties * (*pMessageObjectFactory)(void));
   void  RemoveRegisteredMessageObjectFactory(uint32 const messageType);

   CGameObjectMessageProperties * 
         BuildGameObjectMessageProperties(uint32 const messageType) const;
   
private:
   typedef std::map<uint32, CGameObjectMessageProperties* (*)(void)>    TFactories;
   
   TFactories *                  mFactories;
   uint64                        mInitializedToken;      // Hacky method to determine if this has been initialized when linking static libs
};

//----------------------------------------------------------------------------
// Helper class to be used as a static to register factories on app start
class ENGINE_API CGameObjectRegisterMessageObjectFactory
{
public:
   CGameObjectRegisterMessageObjectFactory(  CGameObjectRegisteredMessageObjectFactories &registration, 
                                             uint32 const messageType, 
                                             CGameObjectMessageProperties * (*pMessageObjectFactory)(void));
   ~CGameObjectRegisterMessageObjectFactory();

   CGameObjectRegisteredMessageObjectFactories &   mRegistration;
   uint32                                          mMessageType;

   BPE_DISABLE_COPY_AND_ASSIGNMENT(CGameObjectRegisterMessageObjectFactory);
};

//----------------------------------------------------------------------------

