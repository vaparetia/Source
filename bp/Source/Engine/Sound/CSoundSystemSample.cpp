//----------------------------------------------------------------------------
// CSoundSystemSample.cpp
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CSoundSystemSample.h"

//----------------------------------------------------------------------------

#include "Engine/Resource/CResourceManager.h"
#include "Engine/Resource/CResourceFactory.h"
#include "Engine/Sound/CSoundSystem.h"

#ifdef BPE_FMOD
#include "fmod.hpp"
#endif

//----------------------------------------------------------------------------

CSoundSystemSample::CSoundSystemSample( void * pData, int const size, bool const isStreamed )
: mpSound(NULL)
{
#ifdef BPE_FMOD
   
   FMOD_CREATESOUNDEXINFO info = { 0 };
   info.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
   info.length = size;

   FMOD_RESULT res = FMOD_OK;

   if( isStreamed )
   {
      res = CSoundSystem::GlobalInstance()->FModSystem()->createSound( (const char*)pData, FMOD_CREATESTREAM|FMOD_SOFTWARE|FMOD_3D_LOGROLLOFF, &info, &mpSound );   
   }
   else
   {
      res = CSoundSystem::GlobalInstance()->FModSystem()->createSound( (const char*)pData, FMOD_OPENMEMORY|FMOD_CREATECOMPRESSEDSAMPLE|FMOD_SOFTWARE|FMOD_3D_LOGROLLOFF, &info, &mpSound );   
   }

   BPE_VERIFY(res == FMOD_OK, false, "Couldn't create sound");

#endif
}

//----------------------------------------------------------------------------

CSoundSystemSample::~CSoundSystemSample()
{
#ifdef BPE_FMOD
   if( mpSound )
   {
      mpSound->release();
   }
#endif
}

//----------------------------------------------------------------------------

void CSoundSystemSample::FFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource)
{
   // if we have a memory buffer passed in, then we're loading from memory
   if( buildData.mpMemory )
   {
      BPE_VERIFY( buildData.mSize != 0, false, "Invalid arguments" );
      returnResource.mpResource = new CSoundSystemSample(buildData.mpMemory, buildData.mSize, false);
      return;
   }
   // otherwise we're creating a stream directly from the file path
   else
   {
      // whenever we get true pak files (if we still use fmod then) we can make the following calls:
      // 1) If we leave audio streams outside the pak files, this will work just as is
      // 2) If we decide to put audio streams into the pak files, then we can use the file callsbacks to load from our pak file instead
      std::string const filePath = CResourceManager::GetLocalPath(buildData.mResourceId);
      returnResource.mpResource = new CSoundSystemSample((void*)filePath.c_str(), 0, true);
   }
}

//----------------------------------------------------------------------------


