//------------------------------------------------------------------------------------------
// GameObjectUtils.h (Engine)
// Bluepoint
// Helper functions for game objects
// Copyright 2006
//------------------------------------------------------------------------------------------

#pragma once

#include "Engine/BPEEngineAPI.h"
#include "Engine/GameObjectSystem/CBaseGameManager.h"

//------------------------------------------------------------------------------------------

namespace GameObjectUtils
{
   ENGINE_API void AddSimpleLinkToComponent(CGameObjectComponent &comp, int32 const linkId, TComponentId const &destId);

   // Helper functions so we can access global components easily.
   // If you use 'AddGlobalComponent' you must call 'RemoveGlobalComponent' when the object is deleted.
   ENGINE_API void AddGlobalComponent(CBaseGameManager &manager, CGameObjectComponent &comp);
   ENGINE_API void RemoveGlobalComponent(CBaseGameManager &manager, CGameObjectComponent &comp);
   ENGINE_API CBaseGameManager::TComponentGroupAndTypeToComponentMapConstIterPair 
                  GetGlobalComponent(CBaseGameManager const &manager, uint32 const componentGroupType, uint32 const componentType);
   ENGINE_API CBaseGameManager::TComponentGroupAndTypeToComponentMapIterPair 
                  GetGlobalComponent(CBaseGameManager &manager, uint32 const componentGroupType, uint32 const componentType);
   ENGINE_API CGameObjectComponent const *
                  GetFirstGlobalComponent(CBaseGameManager const &manager, uint32 const componentGroupType, uint32 const componentType);
   ENGINE_API CGameObjectComponent *
                  GetFirstGlobalComponent(CBaseGameManager &manager, uint32 const componentGroupType, uint32 const componentType);

   ENGINE_API uint32 const GenerateMessageTypeFromString(char const *messageName);
}

//----------------------------------------------------------------------------
