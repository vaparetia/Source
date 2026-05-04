//----------------------------------------------------------------------------
// CProfileMarker.cpp
// Bluepoint
// Copyright 2007
// Platform independent class for adding events for ps3 tuner.
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include <stdio.h>
#include "CProfileMarker.h"

//----------------------------------------------------------------------------

#if BPE_TARGET == BPE_TARGET_PS3

#if defined(BPE_USE_PROFILE_MARKER)
#include "libsn.h"
#include "libsntuner.h"
#endif
#else

// profile markers do not work in non PS3 builds
#undef BPE_USE_PROFILE_MARKER

#endif

//----------------------------------------------------------------------------
// Global count for marker events
static int sMarkerId = 0;

//----------------------------------------------------------------------------

CProfileMarker::CProfileMarker(char const * const pName)
: mId(sMarkerId)
, mpName(pName)
{
#if defined(BPE_USE_PROFILE_MARKER)
   if (sMarkerId == 0)
   {
      snTunerInit();
   }
   BPE_VERIFY(sMarkerId < 128, false, "Too many markers.");
   sMarkerId++;
#endif
}

//----------------------------------------------------------------------------

void CProfileMarker::StartMarker() const
{
#if defined(BPE_USE_PROFILE_MARKER)
   snStartMarker(mId, mpName);
#endif
}

//----------------------------------------------------------------------------

void CProfileMarker::StopMarker() const
{
#if defined(BPE_USE_PROFILE_MARKER)
   snStopMarker(mId);
#endif
}

//----------------------------------------------------------------------------
