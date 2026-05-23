//----------------------------------------------------------------------------
// CSoundSystemSample.h
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//----------------------------------------------------------------------------

namespace FMOD
{
   class Sound;
}

//----------------------------------------------------------------------------

class CResId;
struct SFactoryResourceBuildData;
struct SFactoryReturnResource;

//----------------------------------------------------------------------------

class CSoundSystemSample
{
public:
   ENGINE_API explicit CSoundSystemSample( void * pData, int const size, bool const isStreamed );
   ENGINE_API virtual ~CSoundSystemSample();

   ENGINE_API static void FFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource);

   FMOD::Sound* FModSound() const { return mpSound; }

private:
   FMOD::Sound*   mpSound;
};

//----------------------------------------------------------------------------



