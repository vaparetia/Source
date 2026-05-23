//----------------------------------------------------------------------------
// CResourceFactory.cpp
// Bluepoint
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CResourceFactory.h"
#include "CResourceManager.h"
#include "CBaseResourceFactoryLoadItem.h"

//----------------------------------------------------------------------------

#include "Engine/Streams/CInputStream.h"
#include "Engine/Streams/COutputStream.h"
#include "Engine/Streams/CMemoryInputStream.h"
#include "Engine/System/CFileUtils.h"
#include "Engine/System/CSystemVar.h"
#include "Engine/System/CStopWatch.h"
#include "Engine/System/COsContext.h"
#include "Engine/Basics/CStringExtras.h"
#include "Engine/Mechanics/TinyXml/tinyxml.h"
#include "Engine/Mechanics/TinyXml/xpath_processor.h"

#if BPE_TARGET == BPE_TARGET_PS3
#include <cell/cell_fs.h>
#endif

//----------------------------------------------------------------------------

using namespace std;
using namespace bpe;

//----------------------------------------------------------------------------

static bool                               sAutoCookOnLoad      = false;
static CResourceFactory::EProfileLoading  sProfileLoadingType  = CResourceFactory::kPL_None;


//----------------------------------------------------------------------------

SFactoryReturnResource::SFactoryReturnResource()
: mpResource(NULL)
{
}

//----------------------------------------------------------------------------

SFactoryReturnResource::~SFactoryReturnResource()
{
   // Intentionally empty, resource system manages memory allocations.
}

//----------------------------------------------------------------------------

CResourceFactory::SFactoryEntry::SFactoryEntry(uint32 const factoryFlags, TFactoryFn pFn, CFactoryUserData &userData)
 :  mFlags(factoryFlags)
 ,  mpFactoryFn(pFn)
 ,  mpUserData(userData)
 {
 }

//----------------------------------------------------------------------------

CResourceFactory::SFactoryEntry::SFactoryEntry(uint32 const factoryFlags, TFactoryFn pFn)
 :  mFlags(factoryFlags)
 ,  mpFactoryFn(pFn)
 ,  mpUserData(CFactoryUserData::MakeNull())
 {
 }

//----------------------------------------------------------------------------

      
 void CResourceFactory::SFactoryEntry::BuildResource( CResId const & tag, 
                                                      void * pMemory, 
                                                      int const size, 
                                                      IResourcePool &resourcePool,
                                                      SFactoryReturnResource &returnResource)
{
   real64 const startTime = CStopWatch::gGlobalTime.GetElapsedTime64();
   if (sProfileLoadingType == CResourceFactory::kPL_Verbose)
   {
      bpe_debugger_and_console_printf("%s - 8_StartBuild: %0.3f : %d\n", tag.c_str(), startTime, size);
   }

   SFactoryResourceBuildData buildData(tag, pMemory, size, mpUserData.get(), resourcePool);

   // Build resource
   mpFactoryFn(buildData, returnResource);

   if (sProfileLoadingType == CResourceFactory::kPL_Verbose)
   {
      real64 const endTime = CStopWatch::gGlobalTime.GetElapsedTime64();      
      bpe_debugger_and_console_printf("%s - 9_EndBuild: %0.3f : %0.3fms\n", tag.c_str(), endTime, (endTime - startTime) * 1000.0);
   }

   return;
}

//----------------------------------------------------------------------------

void CResourceFactory::SetAutoCookOnLoad(bool const bAutoCook)
{
   sAutoCookOnLoad = bAutoCook;
}

//----------------------------------------------------------------------------

bool CResourceFactory::GetAutoCookOnLoad()
{
   return sAutoCookOnLoad;
}

//----------------------------------------------------------------------------

void CResourceFactory::SetProfileLoading(EProfileLoading const profileType)
{
   sProfileLoadingType = profileType;
}

//----------------------------------------------------------------------------

CResourceFactory::EProfileLoading CResourceFactory::GetProfileLoading()
{
   return sProfileLoadingType;
}

//----------------------------------------------------------------------------

CResourceFactory::CResourceFactory()
{
#if BPE_TARGET == BPE_TARGET_PS3
   // Initialize async loading
   int ret = cellFsAioInit(CResourceManager::GetRepositoryPath());
   if( ret != CELL_OK )
   {
      // If we couldn't successfully initialize fios, we quit out.
      bpe_debugger_printf("cellFsAioInit failed: %d\n", ret);

      // AndyO: The code below is WRONG for disc builds, we should be waiting for termination processing.
      // However, as we're not using async IO for CP4, I'm disabling the termination flag, as
      // this causes problems with renderer initialization.

      //gpOsContext->mShouldTerminateApplication = true;
   }
#endif
}

//----------------------------------------------------------------------------

CResourceFactory::~CResourceFactory()
{
   foreach(boost::shared_ptr<CBaseResourceFactoryLoadItem> & item, mLoadItems)
   {
      // Cancel all resource loads.
      item->Cancel();
   }
}

//----------------------------------------------------------------------------

void CResourceFactory::BuildSync(CResourceReference &resourceReference )
{
   // See if we're already building this
   CBaseResourceFactoryLoadItem * pItem = NULL;
   list< boost::shared_ptr< CBaseResourceFactoryLoadItem > >::iterator it;
   for( it = mLoadItems.begin(); it != mLoadItems.end(); ++it )
   {
      boost::shared_ptr<CBaseResourceFactoryLoadItem> & item = *it;
      item = *it;
      
      if( item->GetResourceId() == resourceReference.GetResId())
      {
         if (item->IsCancelled())
         {
            item->Uncancel();
         }
         pItem = &(*item);
         break;
      }
   }

   void * pResourcePtr =  NULL;
   if (pItem == NULL)
   {
      // Nope, build it
      BuildAsync(&resourceReference);
      for( it = mLoadItems.begin(); it != mLoadItems.end(); ++it )
      {
         boost::shared_ptr<CBaseResourceFactoryLoadItem> & item = *it;
      
         if ( item->GetResourceId() == resourceReference.GetResId())
         {
            pItem = &(*item);
            break;
         }
      }
   }

   bpe_debugger_printf("Sync build: %s\n",resourceReference.GetResId().c_str());

   // Update until loaded and built
   if (!pItem->HasAsyncLoadingStarted())
   {
      pItem->StartAsyncLoad();
   }

   while( !pItem->IsDone() )
   {
      pItem->Update();
   }
   resourceReference.mpResource = pItem->TargetResourcePtr();

   // Remove built item from 'items to build' list.
   // If we don't do this, then items that are destructed before ProcessLoadingResources is called
   // will not get cached and will also return a NULL resource reference if locked again this immediately.
   mLoadItems.erase(it);

   return;
}

//----------------------------------------------------------------------------

void CResourceFactory::BuildAsync(CResourceReference *pResourceReference)
{
   if (sProfileLoadingType != kPL_None)
   {
      bpe_debugger_and_console_printf("%s - 3_AddToBuildList: %0.3f\n", pResourceReference->GetResId().c_str(), CStopWatch::gGlobalTime.GetElapsedTime64());
   }

   uint32 extension4CC;
   CResId const &resourceId = pResourceReference->GetResId();
   {
      // Calculate uint32 extension type.
      char const * const pSrcExtension4CC = resourceId.c_str() + (resourceId.size() - 4);

      // Make sure we've got at least 3 characters for the extension, otherwise 4CC's will include part of filename.
      BPE_VERIFY((pSrcExtension4CC[2] != '.') && (pSrcExtension4CC[3] != '.'), false, "Invalid type extension for resource");

      // Copy last 4 bytes of filename, assumes a 3 or 4 character extension.
      memcpy(&extension4CC, pSrcExtension4CC, 4);
      NEndian::Swap4Bytes(&extension4CC);
   }

   TFactoryMap::const_iterator found = mFactories.find(extension4CC);
   
   if (found != mFactories.end())
   {
      boost::shared_ptr<CBaseResourceFactoryLoadItem> const loadItem(CBaseResourceFactoryLoadItem::Factory(resourceId, &pResourceReference->mpResource, found->second, pResourceReference->ResourcePool()));

      mLoadItems.push_back(loadItem);
   }
   else
   {
      bpe_debugger_and_console_printf("No factory for asset %s\n", pResourceReference->GetResId().c_str());
      if (!bpe_debugger_is_active())
      {
         // exit rather than crash!
         bpe_message_box( pResourceReference->GetResId().c_str(), "No factory for asset!" );
         COsContext::TerminateProcess();
      }

      BPE_VERIFYA( false, "No factory for asset" );
   }
}

//----------------------------------------------------------------------------

void CResourceFactory::CancelAsyncBuild(CResourceReference *pResourceReference)
{
   for( list< boost::shared_ptr< CBaseResourceFactoryLoadItem > >::iterator it = mLoadItems.begin(); it != mLoadItems.end(); ++it )
   {
      boost::shared_ptr< CBaseResourceFactoryLoadItem > & item = *it;

      if( item->GetResourceId() == pResourceReference->GetResId() )
      {
         item->Cancel();
      }
   }
}

//----------------------------------------------------------------------------

bool CResourceFactory::CanBuild( CResId const & resourceId )
{
   return true;
}

//----------------------------------------------------------------------------

void CResourceFactory::AddFactory(uint32 const resIdExtension4CC, SFactoryEntry const &entry)
{
   mFactories.insert( TFactoryMap::value_type(resIdExtension4CC, entry) );
}

//----------------------------------------------------------------------------
// Might need to take another look at this later to be sure that this works correctly with async builds.
// I think it should work as items make a copy of the factory entries.
void CResourceFactory::RemoveFactory(uint32 const resIdExtension4CC)
{
   TFactoryMap::iterator found = mFactories.find(resIdExtension4CC);
   if (found != mFactories.end())
   {
      mFactories.erase(found);
   }
}

//----------------------------------------------------------------------------
// Monitors async loading count and starts new async loading
void CResourceFactory::UpdateAsyncLoading()
{
   int const kMaxAsyncLoadingFiles = 16;
   int const kMaxAsyncLoadingTotalFileSize = 0x200000;   // 2 megs

   int asyncLoadingFileCount = 0;
   int totalLoadingFileSizes = 0;
   int remainingAsyncLoadingTotalFileSize = kMaxAsyncLoadingTotalFileSize;

   for( list< boost::shared_ptr<CBaseResourceFactoryLoadItem> >::iterator it = mLoadItems.begin(); it != mLoadItems.end(); ++it)
   {
      CBaseResourceFactoryLoadItem & item = *(*it);

      if( item.IsDone() || item.IsCancelled() )
      {
         continue;
      }

      if (item.HasAsyncLoadingStarted())
      {
         asyncLoadingFileCount++;
         if (asyncLoadingFileCount >= kMaxAsyncLoadingFiles)
         {
            // Don't need to start loading any more files
            return;
         }
         totalLoadingFileSizes += item.GetAsyncLoadingFileSize();
         if (totalLoadingFileSizes >= kMaxAsyncLoadingTotalFileSize)
         {
            // Already loaded enough
            return;
         }
      }
   }

   // Start loading more files
   for( list< boost::shared_ptr<CBaseResourceFactoryLoadItem> >::iterator it = mLoadItems.begin(); it != mLoadItems.end(); ++it)
   {
      CBaseResourceFactoryLoadItem & item = *(*it);
      if( item.IsDone() || item.IsCancelled() )
      {
         continue;
      }

      if (!item.HasAsyncLoadingStarted())
      {
         item.StartAsyncLoad();
         asyncLoadingFileCount++;
         totalLoadingFileSizes += item.GetAsyncLoadingFileSize();

         if (sProfileLoadingType != kPL_None)
         {
            bpe_debugger_and_console_printf("%s - 4_StartAsyncLoad: %0.3f : %d : %d\n", item.GetResourceId().c_str(), CStopWatch::gGlobalTime.GetElapsedTime64(), asyncLoadingFileCount, totalLoadingFileSizes);
         }

         if (asyncLoadingFileCount >= kMaxAsyncLoadingFiles)
         {
            // Don't need to start loading any more files
            return;
         }

         if (totalLoadingFileSizes >= kMaxAsyncLoadingTotalFileSize)
         {
            // Already loading enough
            return;
         }
      }
   }

}

//----------------------------------------------------------------------------

void CResourceFactory::ProcessLoadingResources(real32 const maxTime)
{
   CStopWatch buildTimer;
   UpdateAsyncLoading();
   for( list< boost::shared_ptr<CBaseResourceFactoryLoadItem> >::iterator it = mLoadItems.begin(); it != mLoadItems.end(); )
   {
      CBaseResourceFactoryLoadItem & item = *(*it);

      if( item.IsDone() || item.IsCancelled() )
      {
         if (sProfileLoadingType != kPL_None)
         {
            if (item.IsDone())
            {
               bpe_debugger_and_console_printf("%s - 5_LoadCompleted: %0.3f\n", item.GetResourceId().c_str(), CStopWatch::gGlobalTime.GetElapsedTime64());
            }
            else
            {
               // Cancelled
               bpe_debugger_and_console_printf("%s - 5_LoadCancelled: %0.3f\n", item.GetResourceId().c_str(), CStopWatch::gGlobalTime.GetElapsedTime64());

            }
         }

         it = mLoadItems.erase( it );
      }
      else
      {
         item.Update();
         // Allow a max of 1ms for resource building
         if (buildTimer.GetElapsedTime() > maxTime) break;
         ++it;
      }
   }
   UpdateAsyncLoading();
}

//----------------------------------------------------------------------------

int CResourceFactory::DecompressResource(uint8 * dest, uint32 *destLen, uint8 * const source, uint32 sourceLen, CResId const &resId)
{
   real64 const startTime = CStopWatch::gGlobalTime.GetElapsedTime64();

   if (sProfileLoadingType == CResourceFactory::kPL_Verbose)
   {
      bpe_debugger_and_console_printf("%s - 6_StartDecompression: %0.3f : %d\n", resId.c_str(), startTime, sourceLen);
   }

   int result = uncompress_gzip(dest, destLen, source, sourceLen);

   if (sProfileLoadingType == CResourceFactory::kPL_Verbose)
   {      
      real64 const endTime = CStopWatch::gGlobalTime.GetElapsedTime64();      
      bpe_debugger_and_console_printf("%s - 7_EndDecompression: %0.3f : %0.3fms : %d\n", resId.c_str(), endTime, (endTime - startTime) * 1000.0, *destLen);
   }

   return result;
}

//----------------------------------------------------------------------------

