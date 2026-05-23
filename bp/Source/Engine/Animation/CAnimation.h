//----------------------------------------------------------------------------
// CAnimation.h
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//----------------------------------------------------------------------------

class CResId;
struct SFactoryResourceBuildData;
struct SFactoryReturnResource;
class CAnimationEventState;
class CInputStream;
class CQuaternion;
class CVector4;

//----------------------------------------------------------------------------

struct SAnimationEventTiming
{
   enum EConstructionFlags
   {
      kConstructUninitialized
   };
   SAnimationEventTiming(EConstructionFlags const) {}
   real32 const GetTimeFloat() const { return mTime / BP_FRAMES_PER_SEC(); }
   real32 const GetDurationFloat() const { return mDuration / BP_FRAMES_PER_SEC(); }
   uint16 mTime;
   uint16 mDuration;
   int mJointIndex;
};

//----------------------------------------------------------------------------

class CAnimation
{
public:
   virtual ~CAnimation();

   enum EJointIndex
   {
        kAnyJoint  = -2
      , kNoJoint   = -1
      , kRootJoint =  0
   };

   ENGINE_API real32 const GetDuration() const;
   ENGINE_API void GetEvents( real32 const previousTime, real32 const currentTime, CAnimationEventState &eventState ) const;
   ENGINE_API bool const GetTimingOfEvent(int const jointIndex, uint32 const eventId, SAnimationEventTiming &eventTiming) const;

   // This function fills in the rotation/translation deltas of the root node over the duration of this animation.
   ENGINE_API void GetRootNodeAnimationDelta(CQuaternion & rotationDelta, CVector4 & translationDelta) const;

   ENGINE_API static void FAnimationFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource);

protected:
   CAnimation(void* pData)
   {
      mpData = pData;
   }

public:
   void * mpData; 

private:
   BPE_DISABLE_COPY_AND_ASSIGNMENT(CAnimation);
};
