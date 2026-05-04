//----------------------------------------------------------------------------
// CResourcePool.cpp
// Bluepoint
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CResourcePool.h"
#include "CResourceCache.h"
#include "CResourceFactory.h"

//----------------------------------------------------------------------------

#include "Engine/System/CFileUtils.h"
#include "Engine/System/CSystemVar.h"
#include "Engine/System/CStopWatch.h"
#include "Engine/Basics/CStringExtras.h"
#include "Engine/Mechanics/TinyXml/TinyXml.h"
#include "Engine/Mechanics/TinyXml/xpath_processor.h"

//----------------------------------------------------------------------------

using namespace std;
using namespace bpe;

//----------------------------------------------------------------------------

CResourcePool::CResourcePool( IResourceFactory * pFactory )
:  mpFactory( pFactory )
{
}

//----------------------------------------------------------------------------

CResourcePool::~CResourcePool()
{
}

//----------------------------------------------------------------------------

CResource const CResourcePool::GetResource( CResId const & resId )
{
   TResourceMap::const_iterator found = mResources.find( resId );

   if( found != mResources.end() )
      return found->second;

   IObject *pDummyIObject = NULL;
   // TODO: Add check for arbitrary binary data constructor, currently assumes all resources have a virtual destructor.
   CResource const resource( new CResourceReference( pDummyIObject, this, resId ) );

   mResources.insert( TResourceMap::value_type( resId, resource ) );

   return resource;
}
void CResourcePool::ManuallyAddToResourceMap(CResId const & resId, CResource& resource)
{
	mResources.insert( TResourceMap::value_type( resId, resource ) );
}

//----------------------------------------------------------------------------
   
CResourceWithDependencies const CResourcePool::GetResourceWithDependencies( CResId const & resId )
{
   return CResourceWithDependencies(GetResource(resId));
}

//----------------------------------------------------------------------------

IResourcePool::TResourceChildDependencies const & CResourcePool::GetChildDependencies(CResId const &resId)
{
   real64 const startTime = CStopWatch::gGlobalTime.GetElapsedTime64();
   if (CResourceFactory::GetProfileLoading() == CResourceFactory::kPL_Verbose)
   {
      bpe_debugger_and_console_printf("%s - 1_StartGetChildDependencies: %0.3f\n", resId.c_str(), startTime);
   }

   {
      TResourceDependenciesMap::const_iterator it = mDependencies.find(resId);
      if (it != mDependencies.end()) return it->second;
   }

   TResourceChildDependencies &deps = mDependencies.insert(pair<CResId, TResourceChildDependencies>( resId, TResourceChildDependencies() ) ).first->second;
   string const dependencesFilename(CStringExtras::Stringize("%s.assets", CResourceManager::GetLocalPath(resId).c_str()));
   std::string dependencesXml;
   int64 const fileSize = CFileUtils::LoadFileAsString(dependencesFilename.c_str(), dependencesXml);
   if (fileSize > 0)
   {
      TiXmlDocument doc;
      doc.Parse(dependencesXml.c_str());

      // Make sure you use the 'RootElement' not the document for queries.
      TinyXPath::xpath_processor xpProc(doc.RootElement(), "/PackageAssets/Asset");
      int const numNodes = xpProc.u_compute_xpath_node_set();
      deps.reserve(numNodes);
      for (int loop = 0; loop < numNodes; loop++)
      {
         TiXmlElement const * pElement = xpProc.XNp_get_xpath_node(loop)->ToElement();
         // Check for 'packageOnly' attribute
         if (pElement->Attribute("packageOnly") != NULL)
         {
            // Don't want to load this, skip
            continue;
         }
         CResId const depId(pElement->Attribute("cookedAsset"));
         deps.push_back(depId);
      }
   }

   for (TResourceChildDependencies::const_iterator it = deps.begin(); it != deps.end(); ++it)
   {
      // Recurse all other dependencies so they already cached.
      CResId const &dep = *it;
      GetChildDependencies(dep);
   }

   if (CResourceFactory::GetProfileLoading() == CResourceFactory::kPL_Verbose)
   {
      real64 const endTime = CStopWatch::gGlobalTime.GetElapsedTime64();
      bpe_debugger_and_console_printf("%s - 2_EndGetChildDependencies: %0.3f : %0.3fms\n", resId.c_str(), endTime, (endTime - startTime) * 1000.0);
   }

   return deps;
}

//----------------------------------------------------------------------------

void CResourcePool::NotifyResourceBuilt(CResId const &resId)
{
   // We can add code for user callbacks here
   TResourceMap::const_iterator found = mResources.find( resId );

   if( found != mResources.end() )
   {
      CResource const &resource = found->second;
      if (mpResourceCache)
      {
         mpResourceCache->CacheResource(resource);
      }
   }
}

//----------------------------------------------------------------------------

void CResourcePool::SetResourceCache(IResourceCache * pResourceCache)
{
   mpResourceCache.reset(pResourceCache);
}

//----------------------------------------------------------------------------

