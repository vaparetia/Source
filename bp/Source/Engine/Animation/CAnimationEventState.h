//----------------------------------------------------------------------------
// CAnimationEventState.h
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//----------------------------------------------------------------------------

class ENGINE_API CAnimationEventState
{
public:
   struct SEventState
   {
      enum EState
      {
         kState_None       = 0,
         
         kState_Inside     = (1 << 0),
         kState_Begin      = (1 << 1),
         kState_End        = (1 << 2),

         kState_Triggered  = (kState_Begin | kState_Inside | kState_End)

      };

      SEventState(uint32 const eventId, uint16 const jointIndex, uint16 const state)
      :  mEventId(eventId)
      ,  mJointIndex(jointIndex)
      ,  mState(state)
      {
      }

      uint32   mEventId;
      uint16   mJointIndex;
      uint16   mState;
   };

public:
   CAnimationEventState();

   static uint32 GetEventId(const char * const pEventName);

   void AddEvents(SEventState const * pEvents, int const eventCount);

public:
   bpe::reserved_vector<SEventState, 128> mEvents;
};
