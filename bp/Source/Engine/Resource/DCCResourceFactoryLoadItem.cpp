//------------------------------------------------------------------------------------------
// DCCResourceFactoryLoadItem.cpp
// Dreamcast resource factory load item — synchronous KOS stdio I/O
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Resource/IResourcePool.h"
#include "Engine/Basics/CStringExtras.h"
#include "Engine/System/COsContext.h"
#include BPE_PLATFORM_SPECIFIC2( Engine/Resource, CResourceFactoryLoadItem.h )

#include <stdio.h>

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

CResourceFactoryLoadItem::CResourceFactoryLoadItem(CResId const & resource,
                                                   void ** ppResource,
                                                   CResourceFactory::SFactoryEntry const & factory,
                                                   IResourcePool * pResourcePool)
: CBaseResourceFactoryLoadItem(resource, ppResource, factory, pResourcePool)
, mSize(0)
, mLoadCompleted(false)
{
}

//----------------------------------------------------------------------------

CResourceFactoryLoadItem::~CResourceFactoryLoadItem()
{
   if (IsDone() && !IsCancelled())
   {
      ResourcePool()->NotifyResourceBuilt(GetResourceId());
   }
}

//----------------------------------------------------------------------------

int32 CResourceFactoryLoadItem::GetAsyncLoadingFileSize() const
{
   return mSize;
}

//----------------------------------------------------------------------------

void CResourceFactoryLoadItem::StartAsyncLoad()
{
   mIsAsyncLoadingStarted = true;
   if (IsDone() || IsCancelled())
      return;

   bool const bConstructFromMemory = (mFactory.mFlags & CResourceFactory::SFactoryEntry::kFF_ConstructUsingResourcePath) == 0;

   if (bConstructFromMemory)
   {
      string const filePath = CResourceManager::GetLocalPath(mResourceId);

      FILE * pFile = fopen(filePath.c_str(), "rb");
      if (!pFile)
      {
         string const error(CStringExtras::Stringize("Asset: %s could not be loaded\n", filePath.c_str()));
         bpe_debugger_printf("%s", error.c_str());
         BPE_VERIFYA(false, error.c_str());
         *mpTargetResourcePtr = NULL;
         mIsDone = true;
         return;
      }

      fseek(pFile, 0, SEEK_END);
      mSize = (uint32)ftell(pFile);
      fseek(pFile, 0, SEEK_SET);

      mData.resize(mSize, 0);
      fread(&mData[0], 1, mSize, pFile);
      fclose(pFile);

      mLoadCompleted = true;
   }
   else
   {
      SFactoryReturnResource returnResource;
      mFactory.BuildResource(mResourceId, NULL, 0, *mpResourcePool, returnResource);
      *mpTargetResourcePtr = returnResource.mpResource;
      mIsDone = true;
   }
}

//----------------------------------------------------------------------------

void CResourceFactoryLoadItem::Update()
{
   if (IsDone() || IsCancelled())
      return;

   if (!HasAsyncLoadingStarted() || !mLoadCompleted)
      return;

   SFactoryReturnResource returnResource;
   mFactory.BuildResource(mResourceId, &mData[0], mSize, *mpResourcePool, returnResource);
   *mpTargetResourcePtr = returnResource.mpResource;
   mData = vector<char>();

   mIsDone = true;
}

//----------------------------------------------------------------------------
