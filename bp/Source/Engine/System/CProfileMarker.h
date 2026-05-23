//----------------------------------------------------------------------------
// CProfileMarker.cpp
// Bluepoint
// Copyright 2007
// Platform independent class for adding events for ps3 tuner.
//----------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

//#define BPE_USE_PROFILE_MARKER

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//------------------------------------------------------------------------------------------
// Make instances of this class global or static members

class ENGINE_API CProfileMarker
{
public:
   CProfileMarker(char const * const pName);
   void  StartMarker() const;
   void  StopMarker() const;

   int  const           mId;
   char const * const   mpName;   
};

//------------------------------------------------------------------------------------------
// Helper class that activates/deactivates marker via scope
class ENGINE_API CProfileMarkerHelper
{
public:
   CProfileMarkerHelper(CProfileMarker const * const pProfileMarker)
      : mpProfileMarker(pProfileMarker)
   {
      mpProfileMarker->StartMarker();
   };

   ~CProfileMarkerHelper()
   {
      mpProfileMarker->StopMarker();
   };

   CProfileMarker const * const mpProfileMarker;
};

//------------------------------------------------------------------------------------------
// Helper for profile marker

#ifdef BPE_USE_PROFILE_MARKER

#define BPE_ADD_SCOPED_PROFILE_MARKER(desc)     static CProfileMarker const sPM( desc ); \
                                                CProfileMarkerHelper const pmHelper(&sPM);
#else

#define BPE_ADD_SCOPED_PROFILE_MARKER(desc) (desc);

#endif

//------------------------------------------------------------------------------------------

