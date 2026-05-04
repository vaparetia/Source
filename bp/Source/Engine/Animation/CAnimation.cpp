//----------------------------------------------------------------------------
// CAnimation.cpp
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CAnimation.h"

//----------------------------------------------------------------------------

#include "Engine/Resource/CResourceFactory.h"
#include "Engine/Animation/CAnimationEventState.h"
#include "Engine/Math/CQuaternion.h"
#include "Engine/Math/CVector4.h"

#include "edge/anim/edgeanim_structs.h"

//----------------------------------------------------------------------------

namespace
{
   struct SAnimationCustomHeader
   {
      uint32   mVersion;
      uint32   mEventDefsOffset;
      uint32   mEventDefsCount;
      uint32   mEventsOffset;
      uint32   mEventsCount;
      uint32   mRootNodeInfoOffset;
   };

   struct SEventDef
   {
      uint32   mEventId;
      uint32   mJointIndex;
   };

   struct SEvent
   {
      uint32   mEventIndex;
      uint16   mEventTime;
      uint16   mEventDuration;
   };

   struct SRootNodeInfo
   {
      CQuaternion mRootDeltaRotation;
      CVector4    mRootDeltaTranslation;
   };
}

//----------------------------------------------------------------------------

CAnimation::~CAnimation()
{
   EdgeAnimAnimation* pAnimation = (EdgeAnimAnimation*)mpData;
   BPE_FREE_ALIGNED(pAnimation);
}

//----------------------------------------------------------------------------

real32 const CAnimation::GetDuration() const
{
   EdgeAnimAnimation* pAnimation = (EdgeAnimAnimation*)mpData;
   return pAnimation->duration;
}

//----------------------------------------------------------------------------

void CAnimation::GetEvents( real32 const previousTime, real32 const currentTime, CAnimationEventState &eventState ) const
{
   EdgeAnimAnimation const * pEdgeAnimation = (EdgeAnimAnimation const*)mpData;

   SAnimationCustomHeader const * pCustomData = EDGE_OFFSET_GET_POINTER(SAnimationCustomHeader, pEdgeAnimation->offsetCustomData);
   BPE_ASSERT(pCustomData->mVersion == 2, "Invalid custom data version");

   int const eventDefCount = pCustomData->mEventDefsCount;

   if( eventDefCount > 0 )
   {
      SEventDef const * pEventDefs = EDGE_OFFSET_GET_POINTER(SEventDef, pCustomData->mEventDefsOffset);

      CAnimationEventState::SEventState* animationEventState = (CAnimationEventState::SEventState*)alloca(sizeof(CAnimationEventState::SEventState) * eventDefCount);
      for(int i = 0; i < eventDefCount; ++i)
      {
         animationEventState[i].mEventId = pEventDefs[i].mEventId;
         animationEventState[i].mJointIndex = (uint16)pEventDefs[i].mJointIndex;
         animationEventState[i].mState = CAnimationEventState::SEventState::kState_None;
      }

      SEvent const * pEvents = EDGE_OFFSET_GET_POINTER(SEvent, pCustomData->mEventsOffset);

      int const currentFrame = int(previousTime * BP_FRAMES_PER_SEC());
      int const nextFrame = int(currentTime * BP_FRAMES_PER_SEC());

      for( int i = 0; i < pCustomData->mEventsCount; ++i )
      {
         SEvent const & event = pEvents[i];

         if( event.mEventDuration == 0 )
         {
            if( event.mEventTime >= currentFrame && event.mEventTime < nextFrame )
            {
               animationEventState[event.mEventIndex].mState = CAnimationEventState::SEventState::kState_Triggered;
            }
         }
         else
         {
            int const eventStartFrame = event.mEventTime;
            int const eventEndFrame = event.mEventTime + event.mEventDuration;

            bool const isInside = eventStartFrame <= currentFrame && eventEndFrame >= nextFrame;
            // just return inside. when all animations have been queried, the state is processed to determine begin/inside/end
            animationEventState[event.mEventIndex].mState |= isInside ? CAnimationEventState::SEventState::kState_Inside : 0;
         }
      }

      eventState.AddEvents(animationEventState, eventDefCount);
   }
}

//----------------------------------------------------------------------------

bool const CAnimation::GetTimingOfEvent( int const jointIndex, uint32 const eventId, SAnimationEventTiming &eventTiming ) const
{
   EdgeAnimAnimation const * pEdgeAnimation = (EdgeAnimAnimation const*)mpData;

   SAnimationCustomHeader const * pCustomData = EDGE_OFFSET_GET_POINTER(SAnimationCustomHeader, pEdgeAnimation->offsetCustomData);
   BPE_ASSERT(pCustomData->mVersion == 2, "Invalid custom data version");

   int const eventDefCount = pCustomData->mEventDefsCount;

   if( eventDefCount > 0 )
   {
      SEventDef const * pEventDefs = EDGE_OFFSET_GET_POINTER(SEventDef, pCustomData->mEventDefsOffset);

      int eventIndex = 0;
      for( ; eventIndex < eventDefCount; ++eventIndex)
      {
         if (pEventDefs[eventIndex].mEventId == eventId && (jointIndex == kAnyJoint || pEventDefs[eventIndex].mJointIndex == jointIndex))
         {
            break;
         }
      }

      if (eventIndex != eventDefCount)
      {
         SEvent const * pEvents = EDGE_OFFSET_GET_POINTER(SEvent, pCustomData->mEventsOffset);
         for( int i = 0; i < pCustomData->mEventsCount; ++i )
         {
            if (pEvents[i].mEventIndex == eventIndex)
            {
               eventTiming.mTime = pEvents[i].mEventTime;
               eventTiming.mDuration = pEvents[i].mEventDuration;
               eventTiming.mJointIndex = pEventDefs[eventIndex].mJointIndex;
               return true;
            }
         }
      }
   }

   return false;
}

//----------------------------------------------------------------------------

void CAnimation::GetRootNodeAnimationDelta(CQuaternion & rotationDelta, CVector4 & translationDelta) const
{
   EdgeAnimAnimation const * pEdgeAnimation = (EdgeAnimAnimation const*)mpData;

   SAnimationCustomHeader const * pCustomData = EDGE_OFFSET_GET_POINTER(SAnimationCustomHeader, pEdgeAnimation->offsetCustomData);
   BPE_ASSERT(pCustomData->mVersion == 2, "Invalid custom data version");

   SRootNodeInfo const * pRootNodeInfo = EDGE_OFFSET_GET_POINTER(SRootNodeInfo, pCustomData->mRootNodeInfoOffset);
   rotationDelta = pRootNodeInfo->mRootDeltaRotation;
   translationDelta = pRootNodeInfo->mRootDeltaTranslation;
}

//----------------------------------------------------------------------------

void CAnimation::FAnimationFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource)
{
   BPE_VERIFY( buildData.mpMemory != NULL, false, "must have memory" );

   EdgeAnimAnimation* pAnimation = (EdgeAnimAnimation*)BPE_MALLOC_ALIGNED(128, buildData.mSize);
   memcpy(pAnimation, buildData.mpMemory, buildData.mSize);

   returnResource.mpResource = new CAnimation(pAnimation);
}

