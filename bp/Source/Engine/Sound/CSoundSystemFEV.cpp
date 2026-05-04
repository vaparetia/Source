//----------------------------------------------------------------------------
// CSoundSystemFEV.cpp
// FMOD designer project
// Asset is platform specific
// Bluepoint
// Copyright 2007
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CSoundSystemFEV.h"

//----------------------------------------------------------------------------

#include "Engine/Sound/CSoundSystem.h"
#include "Engine/System/CFileUtils.h"
#include "Engine/System/CStopWatch.h"
#include "Engine/Resource/CResourceFactory.h"
#include "Engine/Resource/CResourceManager.h"

#ifdef BPE_FMOD
#include "fmod.hpp"
#include "fmod_event.h"
#endif

//----------------------------------------------------------------------------

CSoundSystemFEV::CSoundSystemFEV( void * pData, int const size, CResId const &resId )
: mpProject(NULL)
{
#ifdef BPE_FMOD
   
   CSoundSystem  * pSoundSystem = CSoundSystem::GlobalInstance();
   FMOD_RESULT res = FMOD_OK;

   // Now set media directory to current asset directory
   // strip filename
   std::string localPath(CResourceManager::GetLocalPath(resId));
   std::string assetDirectory(CFileUtils::GetFilePath(localPath));     
   res = pSoundSystem->FModEventSystem()->setMediaPath(assetDirectory.c_str());

   // Fake out loading for .fev
   pSoundSystem->SetFMODMemoryFile(pData, size);
   res = pSoundSystem->FModEventSystem()->load( "DontCare", NULL, &mpProject);
   pSoundSystem->ResetFMODMemoryFile();

   // Load all groups via FMOD
   int numGroups = 0;
   res = mpProject->getNumGroups(&numGroups);
   for (int loop = 0; loop < numGroups; loop++)
   {
      FMOD::EventGroup *pEventGroup = NULL;
      bool const bCacheEvents = true;
      res = mpProject->getGroupByIndex(loop, bCacheEvents, &pEventGroup);      
      // Only load samples, don't open streams otherwise we'll run out of file handles on PS3.
      res = pEventGroup->loadEventData(FMOD::EVENT_RESOURCE_SAMPLES, EVENT_DEFAULT);      
   }

#endif
}

//----------------------------------------------------------------------------

CSoundSystemFEV::~CSoundSystemFEV()
{
#ifdef BPE_FMOD
   if( mpProject )
   {
      FMOD_RESULT res = FMOD_OK;

      // Looks like you have to explicitly free all groups for clean shutdown
      int numGroups = 0;
      res = mpProject->getNumGroups(&numGroups);
      for (int loop = 0; loop < numGroups; loop++)
      {
         FMOD::EventGroup *pEventGroup = NULL;
         bool const bCacheEvents = true;
         res = mpProject->getGroupByIndex(loop, bCacheEvents, &pEventGroup);
         res = pEventGroup->freeEventData();
      }      
      
      res = mpProject->release();
      mpProject = NULL;
   }
#endif
}

//----------------------------------------------------------------------------

void CSoundSystemFEV::FFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource)
{
   BPE_VERIFY( buildData.mpMemory && buildData.mSize, false, "Invalid arguments" );

   returnResource.mpResource = new CSoundSystemFEV( buildData.mpMemory, buildData.mSize, buildData.mResourceId );
}

//----------------------------------------------------------------------------
