//----------------------------------------------------------------------------
// CResourceCache.cpp
// Bluepoint
// Copyright 2006
//
// Used to cache already loaded resources to cut down on loading times.
//
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CResourceCache.h"

//----------------------------------------------------------------------------


#include "Engine/System/CFileUtils.h"
#include "Engine/System/COsContext.h"
#include "Engine/System/CStopWatch.h"
#include "Engine/Basics/CStringExtras.h"

#include "boost/regex.hpp"

//----------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------

CResourceCache::CResourceCache()
{
}

//------------------------------------------------------------------------------------------

CResourceCache::~CResourceCache()
{
}

//------------------------------------------------------------------------------------------

void CResourceCache::RebuildInvalidatedCachedResources()
{
   // Don't check for invalidated resources in shipping mode
   if (gpOsContext->IsReleaseMode()) return;

   // Don't check for invalidated resources in non develop modes
   if (!gpOsContext->IsDevelopMode()) return;

   bool didRebuildResource = false;

   for (TResourceMap::const_iterator it = mCachedResources.begin(); it != mCachedResources.end(); ++it)
   {
      CResId const &resId = it->first;
      didRebuildResource |= CheckForInvalidatedCachedResource(resId);
   }

   if( didRebuildResource )
   {
      for( int i = 0; i < mResourcesChangedDelegates.size(); ++i )
      {
         mResourcesChangedDelegates[i]();
      }
   }
}

//------------------------------------------------------------------------------------------

bool CResourceCache::CheckForInvalidatedCachedResource(CResId const &resId)
{
   if (!resId.IsFileResource()) 
      return false;

   // We can add code for user callbacks here
   TResourceMap::iterator found = mCachedResources.find( resId );

   if( found != mCachedResources.end() )
   {
      CLockedResource const &resource = found->second.second;
      // Trust me, I know what I'm doing!
      CResourceReference & resourceRef = const_cast<CResourceReference &>(static_cast<CResourceReference const &>(resource.GetResourceReference()));

//      CTimestamp const fileTS(CTimestamp::FromFile(CResourceManager::GetLocalPath(resId)));
//      if (fileTS > found->second.first)
	  //soren changing - in GOW we just think everything needs reloading (this is used for almost nothing)
	  if(true)
      {
         bpe_debugger_printf("%s has been updated, reloading.\n", resId.c_str());
         // Newer version exists, rebuild our locked version.
         resourceRef.Rebuild();
		 //soren removing - in GOW we just think everything needs reloading (this is used for almost nothing)
//         found->second.first = fileTS;
         return true;
      }
   }
   return false;
}

//------------------------------------------------------------------------------------------

void CResourceCache::CacheResource(CResource const &resource)
{
   CResId const &resId = *resource.GetResId();
   if (!ResIdPassesFilters(resId))
   {
      // No caching for you!
      return;
   }

   CTimestamp  timestamp(CTimestamp::Null());

   //soren removing - no extra disk access for GOW, buddy!
//   if (resId.IsFileResource())
//   {
//      timestamp = CTimestamp::FromFile(CResourceManager::GetLocalPath(resId));
//   }

   CLockedResource lockedResource(resource);
   mCachedResources.insert(TResourceMap::value_type(resId, pair<CTimestamp, CLockedResource>(timestamp, lockedResource)));
}

//------------------------------------------------------------------------------------------

void CResourceCache::CacheResource(CResource const &resource, CResId const &resId)
{
   if (!ResIdPassesFilters(resId))
   {
      // No caching for you!
      return;
   }

   CTimestamp  timestamp(CTimestamp::Null());
   //soren removing - no extra disk access for GOW, buddy!
//   if (resId.IsFileResource())
//   {
//      timestamp = CTimestamp::FromFile(CResourceManager::GetLocalPath(resId));
//   }

   CLockedResource lockedResource(resource);
   mCachedResources.insert(TResourceMap::value_type(resId, pair<CTimestamp, CLockedResource>(timestamp, lockedResource)));
}

//------------------------------------------------------------------------------------------

CLockedResource const * CResourceCache::FindCachedResource(CResId const &resId) const
{
   TResourceMap::const_iterator found = mCachedResources.find( resId );

   if (found != mCachedResources.end())
   {
      CLockedResource const &resource = found->second.second;
      return &resource;
   }
   return NULL;
}

//------------------------------------------------------------------------------------------

uint32 CResourceCache::GetType() const
{
   return skType;
}

//------------------------------------------------------------------------------------------

bool CResourceCache::ResIdPassesFilters(CResId const &resId) const
{
   foreach(boost::shared_ptr<boost::regex const> const &pRegex, mExcludeFilters)
   {
      if (regex_match(resId.c_str(), *pRegex))
      {
         bpe_debugger_printf("AssetNotCached: %s - %s\n", resId.c_str(), pRegex->str().c_str());
         return false;
      }
   }
   return true;
}

//------------------------------------------------------------------------------------------

void CResourceCache::AddRegexExcludeAssetFilter(std::string const &filter)
{    
   boost::regex const * pRegex = new boost::regex(filter.c_str(), boost::regex::extended|boost::regex::icase);
   mExcludeFilters.push_back(boost::shared_ptr<boost::regex const>(pRegex));
}

//------------------------------------------------------------------------------------------

void CResourceCache::AddResourcesChangedDelegate(TResourcesChangedDelegate const & delegate)
{
   // ensure it's not added twice
   std::vector<TResourcesChangedDelegate>::const_iterator it = std::find(mResourcesChangedDelegates.begin(), mResourcesChangedDelegates.end(), delegate);
   if (it == mResourcesChangedDelegates.end())
   {
      mResourcesChangedDelegates.push_back(delegate);
   }
}

//------------------------------------------------------------------------------------------

void CResourceCache::RemoveResourcesChangedDelegate(TResourcesChangedDelegate const & delegate)
{
   std::vector<TResourcesChangedDelegate>::iterator it = std::find(mResourcesChangedDelegates.begin(), mResourcesChangedDelegates.end(), delegate);
   if (it != mResourcesChangedDelegates.end())
   {
      mResourcesChangedDelegates.erase(it);
   }
}

//------------------------------------------------------------------------------------------
