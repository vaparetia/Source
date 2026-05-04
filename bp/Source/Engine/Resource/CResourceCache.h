//------------------------------------------------------------------------------------------
// CResourceCache.h
// Bluepoint
//
// Copyright 2006
//
// Used to cache already loaded resources to cut down on loading times.
//
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Mechanics/FastDelegate/FastDelegate.h"
#include "Engine/Resource/CResourceManager.h"
#include "Engine/System/CTimestamp.h"
#include "boost/shared_ptr.hpp"
#include "boost/regex_fwd.hpp"

//------------------------------------------------------------------------------------------

class ENGINE_API IResourceCache
{
public:
   virtual ~IResourceCache() {};
   virtual void   RebuildInvalidatedCachedResources() = 0;
   virtual void   CacheResource(CResource const &resource) = 0;
   virtual void   CacheResource(CResource const &resource, CResId const &resId) = 0;      // Useful for caching non-file based resources (http)
   virtual uint32 GetType() const = 0;       // Poor mans RTTI
   virtual CLockedResource const * FindCachedResource(CResId const &resId) const = 0;
};

//------------------------------------------------------------------------------------------

class ENGINE_API CResourceCache : public IResourceCache
{
public:
   typedef fastdelegate::FastDelegate0<>  TResourcesChangedDelegate;

public:
   static const uint32 skType = 'SMPL';

   CResourceCache();
   virtual ~CResourceCache();

   virtual void   RebuildInvalidatedCachedResources();
   virtual void   CacheResource(CResource const &resource);
   virtual void   CacheResource(CResource const &resource, CResId const &resId);
   virtual CLockedResource const * FindCachedResource(CResId const &resId) const;

   virtual uint32 GetType() const;

   bool           CheckForInvalidatedCachedResource(CResId const &resId);
   bool           ResIdPassesFilters(CResId const &resId) const;
   void           AddRegexExcludeAssetFilter(std::string const &filter);

   void           AddResourcesChangedDelegate(TResourcesChangedDelegate const & delegate);
   void           RemoveResourcesChangedDelegate(TResourcesChangedDelegate const & delegate);

public:
   typedef std::map< CResId, std::pair<CTimestamp, CLockedResource> >   TResourceMap;
   typedef std::vector<boost::shared_ptr<boost::regex const> >          TExcludeFilters;

   TResourceMap      mCachedResources;

   TExcludeFilters   mExcludeFilters;

   std::vector<TResourcesChangedDelegate> mResourcesChangedDelegates;
};

//------------------------------------------------------------------------------------------


