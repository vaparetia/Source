//------------------------------------------------------------------------------------------
// CGameObjectPropertiesPackageEntry.h
// Container for multiple component properties.
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

#include "Engine/GameObjectSystem/CGameObjectProperties.h"
#include "boost/shared_ptr.hpp"
//------------------------------------------------------------------------------------------

class IGameObjectFactory;

//------------------------------------------------------------------------------------------

class ENGINE_API CGameObjectPropertiesPackageEntry
{
public:
   explicit CGameObjectPropertiesPackageEntry(CInputStream &inStream, IGameObjectFactory * const pFactory);
   ~CGameObjectPropertiesPackageEntry();

   CGameObjectProperties::TChildGameObjectProperties const &   GetPropertiesArray() const  { return mProperties; };

private:
   // Vector of properties for game object (child game object properties are stored in game object)
   CGameObjectProperties::TChildGameObjectProperties     mProperties;
};

