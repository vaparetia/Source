//------------------------------------------------------------------------------------------
// IResourcePool.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

#include "Engine/Resource/CResourceManager.h"

class IResourceFactory;
class IResourceCache;

//------------------------------------------------------------------------------------------

class IResourcePool
{
public:
   virtual ~IResourcePool() = 0;

   typedef std::vector<CResId> TResourceChildDependencies;
   //workaround for assets not added to resource map
   virtual void									ManuallyAddToResourceMap(CResId const & resId, CResource& resource) = 0;

   virtual CResource const                      GetResource( CResId const & resId ) = 0;
   virtual CResourceWithDependencies const      GetResourceWithDependencies( CResId const & resId ) = 0;
   // Returns child dependencies associated with resource.
   // Call multiple times to get full tree.
   virtual TResourceChildDependencies const &   GetChildDependencies(CResId const &resId) = 0;

   // Called when a resource has finished building
   virtual void                                 NotifyResourceBuilt(CResId const &resId) = 0;

   virtual void                                 SetResourceCache(IResourceCache * pResourceCache) = 0;
   virtual IResourceCache *                     ResourceCache() = 0;
   virtual IResourceFactory *                   ResourceFactory() = 0;
};

//------------------------------------------------------------------------------------------

// Global pool used by property loading.
// This has been added due to static library linking problems.
// TODO: Investigate passing the pool as an argument to PostLoadUpdate()

extern ENGINE_API IResourcePool *     gpResources;

//------------------------------------------------------------------------------------------

