//------------------------------------------------------------------------------------------
// CResourcePool.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

#include "Engine/Resource/IResourcePool.h"

#include "boost/shared_ptr.hpp"

class IResourceFactory;
class IResourceCache;


//------------------------------------------------------------------------------------------

class ENGINE_API CResourcePool : public IResourcePool
{
public:
   typedef std::map< CResId, CResource >  TResourceMap;
   typedef std::map< CResId, TResourceChildDependencies >  TResourceDependenciesMap;

   explicit CResourcePool( IResourceFactory * pFactory );
   ~CResourcePool();

   //workaround for assets not added to resource map
   void											ManuallyAddToResourceMap(CResId const & resId, CResource& resource);

   CResource const                              GetResource( CResId const & resId );
   CResourceWithDependencies const              GetResourceWithDependencies( CResId const & resId );
   // Returns child dependencies associated with resource.
   // Call multiple times to get full tree.
   virtual TResourceChildDependencies const &   GetChildDependencies(CResId const &resId);
   virtual void                                 NotifyResourceBuilt(CResId const &resId);

   virtual void                                 SetResourceCache(IResourceCache * pResourceCache); // Takes ownership of ptr
   virtual IResourceCache *                     ResourceCache()         { return mpResourceCache.get(); };
   virtual IResourceFactory *                   ResourceFactory()       { return mpFactory; }

   // Get resource map, useful for iterating over loaded resources.
   TResourceMap const &                         GetResourceMap() const  { return mResources; };

private:
   IResourceFactory *                  mpFactory;
   TResourceMap                        mResources;
   TResourceDependenciesMap            mDependencies;
   boost::shared_ptr<IResourceCache>   mpResourceCache;

private:
   BPE_DISABLE_OBJECT_ASSIGN( CResourcePool );
};

//------------------------------------------------------------------------------------------

