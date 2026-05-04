//------------------------------------------------------------------------------------------
// GameObjectUtils.cpp (Engine)
// Bluepoint
// Helper functions for game objects
// Copyright 2006
//------------------------------------------------------------------------------------------

#include "StdAfx.h"

#include "GameObjectUtils.h"
#include "Engine/GameObjectSystem/CGameObjectComponent.h"
#include "Engine/GameObjectSystem/CGameObjectMessageProperties.h"
#include "Engine/GameObjectSystem/IGameObjectSharedComponentData.h"  // Included for interface class DLL export.

#include "Engine/Mechanics/CCRC.h"

//------------------------------------------------------------------------------------------

void GameObjectUtils::AddSimpleLinkToComponent(CGameObjectComponent &comp, int32 const linkId, TComponentId const &destId)
{
   comp.DuplicateMessagesAndLinks();

   boost::shared_ptr<CGameObjectMessageProperties const> pProperties( new CGameObjectMessagePropertiesEmpty(linkId) );
   CGameObjectMessage message(linkId, destId, pProperties);
   comp.RemappedLinks().push_back(message);
}

//------------------------------------------------------------------------------------------

uint64 component_and_group_type_to_uint64(uint32 const componentGroupType, uint32 const componentType)
{
   uint64 const componentAndGroupType = (uint64) componentGroupType + (((uint64) componentType) << 32ULL);
   return componentAndGroupType;
}

//------------------------------------------------------------------------------------------

void GameObjectUtils::AddGlobalComponent(CBaseGameManager &manager, CGameObjectComponent &comp)
{
   uint64 const key = component_and_group_type_to_uint64(comp.GetProperties()->GetComponentGroupType(), comp.GetProperties()->GetComponentType());
   manager.mComponentGroupAndTypeToComponentMap.insert(CBaseGameManager::TComponentGroupAndTypeToComponentMap::value_type(key, &comp));
}

//------------------------------------------------------------------------------------------

void GameObjectUtils::RemoveGlobalComponent(CBaseGameManager &manager, CGameObjectComponent &comp)
{
   uint64 const key = component_and_group_type_to_uint64(comp.GetProperties()->GetComponentGroupType(), comp.GetProperties()->GetComponentType());
   CBaseGameManager::TComponentGroupAndTypeToComponentMapIterPair componentsIt = manager.mComponentGroupAndTypeToComponentMap.equal_range(key);
   for (CBaseGameManager::TComponentGroupAndTypeToComponentMap::iterator iter = componentsIt.first; iter != componentsIt.second; ++iter)
   {
      CGameObjectComponent * pComponent = (*iter).second;
      if (pComponent->GetId() == comp.GetId())
      {
         // Unique id's match
         manager.mComponentGroupAndTypeToComponentMap.erase(iter);
         break;
      }
   }
}

//------------------------------------------------------------------------------------------

CBaseGameManager::TComponentGroupAndTypeToComponentMapConstIterPair GameObjectUtils::GetGlobalComponent(CBaseGameManager const &manager, uint32 const componentGroupType, uint32 const componentType)
{
   uint64 const key = component_and_group_type_to_uint64(componentGroupType, componentType);
   return manager.mComponentGroupAndTypeToComponentMap.equal_range(key);
}

//------------------------------------------------------------------------------------------

CBaseGameManager::TComponentGroupAndTypeToComponentMapIterPair GameObjectUtils::GetGlobalComponent(CBaseGameManager &manager, uint32 const componentGroupType, uint32 const componentType)
{
   uint64 const key = component_and_group_type_to_uint64(componentGroupType, componentType);
   return manager.mComponentGroupAndTypeToComponentMap.equal_range(key);
}

//------------------------------------------------------------------------------------------

CGameObjectComponent const * GameObjectUtils::GetFirstGlobalComponent(CBaseGameManager const &manager, uint32 const componentGroupType, uint32 const componentType)
{
   uint64 const key = component_and_group_type_to_uint64(componentGroupType, componentType);
   CBaseGameManager::TComponentGroupAndTypeToComponentMap::const_iterator it = manager.mComponentGroupAndTypeToComponentMap.find(key);
   if (it != manager.mComponentGroupAndTypeToComponentMap.end())
   {
      return (*it).second;
   }
   return NULL;
}

//------------------------------------------------------------------------------------------

CGameObjectComponent *  GameObjectUtils::GetFirstGlobalComponent(CBaseGameManager &manager, uint32 const componentGroupType, uint32 const componentType)
{
   uint64 const key = component_and_group_type_to_uint64(componentGroupType, componentType);
   CBaseGameManager::TComponentGroupAndTypeToComponentMap::const_iterator it = manager.mComponentGroupAndTypeToComponentMap.find(key);
   if (it != manager.mComponentGroupAndTypeToComponentMap.end())
   {
      return (*it).second;
   }
   return NULL;
}

//------------------------------------------------------------------------------------------

uint32 const GameObjectUtils::GenerateMessageTypeFromString(char const *messageName)
{
   return CCRC::CalculateCRC32AsString(messageName);
}

//------------------------------------------------------------------------------------------
