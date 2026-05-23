//------------------------------------------------------------------------------------------
// CGameObjectGlobalData.cpp
// Bluepoint
// Copyright 2005
//
// Used for sharing common data between multiple game object components.
// Derive from this for your own data types.
//------------------------------------------------------------------------------------------

#include "StdAfx.h"

#include "Engine/GameObjectSystem/CGameObjectGlobalData.h"
#include "Engine/GameObjectSystem/CBaseGameManager.h"
#include "boost/weak_ptr.hpp"

//------------------------------------------------------------------------------------------

CGameObjectGlobalData::CGameObjectGlobalData()
{
}

//------------------------------------------------------------------------------------------

CGameObjectGlobalData::~CGameObjectGlobalData()
{
}

//------------------------------------------------------------------------------------------
// Will create global data if data doesn't exist for id1/2
boost::shared_ptr<CGameObjectGlobalData> CGameObjectGlobalData::GetGlobalData_Untyped(uint32 const id1, 
                                                                                       uint32 const id2, 
                                                                                       CBaseGameManager &baseGameManager, 
                                                                                       TGlobalDataFactoryDelegate const &factoryDelegate)
{
   CBaseGameManager::TGlobalDataMap &globalDataMap = baseGameManager.GlobalDataMap();

   boost::shared_ptr<CGameObjectGlobalData> data;
   uint64 const mapKey = (uint64) id1 + (uint64) id2 * 0xFFFFFFFFUL;

   CBaseGameManager::TGlobalDataMap::iterator it = globalDataMap.find(mapKey);
   if (it != globalDataMap.end())
   {
      data = it->second.lock();
   }

   // Check to see if we've already got data
   if (!data)
   {
      // NULL, need to create data
      data = factoryDelegate(baseGameManager);
      std::pair<CBaseGameManager::TGlobalDataMap::iterator , bool> insertIt = globalDataMap.insert(std::pair<uint64, boost::weak_ptr<CGameObjectGlobalData> >(mapKey, data));
      // Reset map entry weak ptr as it might already exist in map with NULL ptr.
      insertIt.first->second = data; 
   }

   return data;
}

//------------------------------------------------------------------------------------------
// Will return a NULL shared pointer if data doesn't exist for id1/2
boost::shared_ptr<CGameObjectGlobalData> CGameObjectGlobalData::GetGlobalData_Untyped(uint32 const id1, 
                                                                                       uint32 const id2, 
                                                                                       CBaseGameManager &baseGameManager)                                                                                       
{
   CBaseGameManager::TGlobalDataMap &globalDataMap = baseGameManager.GlobalDataMap();

   boost::shared_ptr<CGameObjectGlobalData> data;
   uint64 const mapKey = (uint64) id1 + (uint64) id2 * 0xFFFFFFFFUL;

   CBaseGameManager::TGlobalDataMap::iterator it = globalDataMap.find(mapKey);
   if (it != globalDataMap.end())
   {
      data = it->second.lock();
   }

   // Returns 'NULL' data if doesn't already exist.
   return data;
}

//------------------------------------------------------------------------------------------
