//------------------------------------------------------------------------------------------
// IGameObjectSharedComponentData.h
// Bluepoint
// Copyright 2009
//
// Interface class used for data that is unique per game object but can be shared between 
// it's components without worry about ownership.
// Derive from this for your own data types.
//
// Current only used in CGameObjectGame but might move core functionality into CGameObject,
// hence it's location in Engine.
//
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//------------------------------------------------------------------------------------------

class CBaseGameManager;

//------------------------------------------------------------------------------------------

class ENGINE_API IGameObjectSharedComponentData
{
public:   
   virtual ~IGameObjectSharedComponentData() {}
};

//------------------------------------------------------------------------------------------

