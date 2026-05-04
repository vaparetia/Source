//------------------------------------------------------------------------------------------
// CGameObjectPropertiesPackage.h
// Container for multiple component properties.
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

#include "TGameObjectTypes.h"
#include "CGameObjectPropertiesPackageEntry.h"

//------------------------------------------------------------------------------------------

class CResId;
struct SFactoryResourceBuildData;
struct SFactoryReturnResource;

class IGameObjectFactory;

//------------------------------------------------------------------------------------------

class ENGINE_API CGameObjectPropertiesPackage
{
public:
   explicit CGameObjectPropertiesPackage(CInputStream &inStream, IGameObjectFactory * const pFactory);
   virtual ~CGameObjectPropertiesPackage();

   CGameObjectPropertiesPackageEntry const &    GetRootEntry() const  { return *mpRootEntry; };

   static void FGameObjectPackage(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource);

private:
   // Using shared ptr to make copying/construction easier
   boost::shared_ptr<CGameObjectPropertiesPackageEntry>     mpRootEntry;
};

