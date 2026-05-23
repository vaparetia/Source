//------------------------------------------------------------------------------------------
// CGameObjectPropertiesPackageEntry.cpp
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/GameObjectSystem/CGameObjectPropertiesPackageEntry.h"
#include "Engine/GameObjectSystem/IGameObjectFactory.h"

//------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------

CGameObjectPropertiesPackageEntry::CGameObjectPropertiesPackageEntry(CInputStream &inStream, IGameObjectFactory * const pFactory)
{
   mProperties = CGameObjectProperties::FChildGameObjectProperties(inStream, pFactory);
}

//------------------------------------------------------------------------------------------

CGameObjectPropertiesPackageEntry::~CGameObjectPropertiesPackageEntry()
{
}

//------------------------------------------------------------------------------------------
