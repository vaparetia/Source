//------------------------------------------------------------------------------------------
// CGameObjectComponentMethods.h
// Bluepoint
// All CGameObjectComponentMethods* derive from this in gamecode.
// Allows CGameObjectComponent to pass a castable version game side to components.
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------

class ENGINE_API CGameObjectComponentMethods
{
public:
   CGameObjectComponentMethods() {};
   virtual ~CGameObjectComponentMethods() {};
};

//------------------------------------------------------------------------------------------
