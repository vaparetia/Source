//----------------------------------------------------------------------------
// CAnimationEventState.cpp
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CAnimationEventState.h"

//----------------------------------------------------------------------------

#include "Engine/Mechanics/HashUtils.h"

//----------------------------------------------------------------------------

CAnimationEventState::CAnimationEventState()
{
}

//----------------------------------------------------------------------------

uint32 CAnimationEventState::GetEventId(const char * const pEventName)
{
   return HashUtils::CalculateAsStringFNV1a(pEventName);
}

//----------------------------------------------------------------------------

void CAnimationEventState::AddEvents(SEventState const * pEvents, int const eventCount)
{
   for( int inputEventIndex = 0; inputEventIndex < eventCount; ++inputEventIndex )
   {
      SEventState const & event = pEvents[inputEventIndex];

      if( event.mState != SEventState::kState_None )
      {
         int const outputEventCount = mEvents.size();
         int outputEventIndex = 0;

         for( ; outputEventIndex < outputEventCount; ++outputEventIndex )
         {
            SEventState & outputEvent = mEvents[outputEventIndex];
            if( outputEvent.mEventId == event.mEventId && outputEvent.mJointIndex == event.mJointIndex)
            {
               if( event.mState == outputEvent.mState )
               {
                  break;
               }
               else
               {
                  int const combinedState = event.mState | outputEvent.mState;
                  if( combinedState & SEventState::kState_Inside )
                  {
                     outputEvent.mState = SEventState::kState_Inside;
                     break;
                  }
               }
            }
         }

         if( outputEventIndex == outputEventCount )
         {
            mEvents.push_back(event);
         }
      }
   }
}
