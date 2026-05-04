//----------------------------------------------------------------------------
// CSoundSystemFEV.h
// FMOD designer project
// Asset is platform specific
// Bluepoint
// Copyright 2007
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//----------------------------------------------------------------------------

class CResId;
struct SFactoryResourceBuildData;
struct SFactoryReturnResource;

//----------------------------------------------------------------------------

namespace FMOD
{
   class EventProject;
}

//----------------------------------------------------------------------------

class ENGINE_API CSoundSystemFEV
{
public:
   explicit CSoundSystemFEV( void * pData, int const size, CResId const &resId );
   virtual ~CSoundSystemFEV();

   static void FFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource);

   FMOD::EventProject* FModEventProject() const { return mpProject; }

private:
   FMOD::EventProject*     mpProject;
};

//----------------------------------------------------------------------------



