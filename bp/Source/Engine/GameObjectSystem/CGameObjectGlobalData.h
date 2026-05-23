//------------------------------------------------------------------------------------------
// CGameObjectGlobalData.h
// Bluepoint
// Copyright 2007
//
// Used for sharing common data between multiple game object components.
// Derive from this for your own data type.
// WARNING: Accessing global data isn't thread safe!
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Mechanics/FastDelegate/FastDelegate.h"
#include "boost/shared_ptr.hpp"

//------------------------------------------------------------------------------------------

class CBaseGameManager;

//------------------------------------------------------------------------------------------

class ENGINE_API CGameObjectGlobalData
{
public:
   CGameObjectGlobalData();
   virtual ~CGameObjectGlobalData();

   typedef fastdelegate::FastDelegate1<CBaseGameManager const &, boost::shared_ptr<CGameObjectGlobalData> >   TGlobalDataFactoryDelegate;

   // Use this function to retrieve your global data. This function will create global data if it doesn't exist for the given ids.
   // e.g. CGameObjectGlobalData::GetGlobalData<CMyGlobalData>(0x1234, 0x1234, baseGameManager, fastdelegate::MakeDelegate(myFunc));
   template<class T>
   static boost::shared_ptr<T> GetGlobalData(uint32 const id1, 
                                             uint32 const id2, 
                                             CBaseGameManager &baseGameManager, 
                                             TGlobalDataFactoryDelegate const &factoryDelegate)
   {
      boost::shared_ptr<T> ptr(boost::static_pointer_cast<T>(GetGlobalData_Untyped(id1, id2, baseGameManager, factoryDelegate)));
      return ptr;
   };

   // Use this function to retrieve your global data, will return a NULL shared pointer if data doesn't exist.
   template<class T>
   static boost::shared_ptr<T> GetGlobalData(uint32 const id1, 
                                             uint32 const id2, 
                                             CBaseGameManager &baseGameManager)
   {
      boost::shared_ptr<T> ptr(boost::static_pointer_cast<T>(GetGlobalData_Untyped(id1, id2, baseGameManager)));
      return ptr;
   };

   // WARNING: Use untyped internal functions at own risk!
   static boost::shared_ptr<CGameObjectGlobalData> GetGlobalData_Untyped(uint32 const id1, 
                                                                         uint32 const id2, 
                                                                         CBaseGameManager &baseGameManager, 
                                                                         TGlobalDataFactoryDelegate const &factoryDelegate);   

   static boost::shared_ptr<CGameObjectGlobalData> GetGlobalData_Untyped(uint32 const id1, 
                                                                         uint32 const id2, 
                                                                         CBaseGameManager &baseGameManager);
};

//------------------------------------------------------------------------------------------

