//------------------------------------------------------------------------------------------
// CGameObjectPropertiesPackage.cpp
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/GameObjectSystem/CGameObjectPropertiesPackage.h"
#include "Engine/GameObjectSystem/IGameObjectFactory.h"
#include "Engine/Streams/CMemoryInputStream.h"
#include "Engine/Resource/CResourceFactory.h"

//------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------

CGameObjectPropertiesPackage::CGameObjectPropertiesPackage(CInputStream &inStream, IGameObjectFactory * const pFactory)
{
   // Read header from stream
   uint32 const header = inStream.ReadUint32();
   BPE_VERIFY(header == 'SSOP', false, "Incorrect header in CGameObjectPropertiesPackage constructor.");

   // Read property tree
   mpRootEntry = boost::shared_ptr<CGameObjectPropertiesPackageEntry>(new CGameObjectPropertiesPackageEntry(inStream, pFactory));
}

//------------------------------------------------------------------------------------------

CGameObjectPropertiesPackage::~CGameObjectPropertiesPackage()
{
}

//------------------------------------------------------------------------------------------

void CGameObjectPropertiesPackage::FGameObjectPackage(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource)
{
   CMemoryInputStream inStream(buildData.mpMemory, buildData.mSize, CMemoryInputStream::kOwner_App);
   IGameObjectFactory * const pFactory = reinterpret_cast<IGameObjectFactory *>(buildData.mpFactoryUserData);

   returnResource.mpResource = new CGameObjectPropertiesPackage(inStream, pFactory);
}

//------------------------------------------------------------------------------------------
