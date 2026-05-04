//----------------------------------------------------------------------------
// CSyncEvent.h
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/System/ISyncObject.h"

//----------------------------------------------------------------------------

#if BPE_TARGET==BPE_TARGET_WIN32 || BPE_TARGET==BPE_TARGET_X360 || BPE_TARGET==BPE_TARGET_PS3 || BPE_TARGET==BPE_TARGET_VITA
#  define CSYNCEVENT_SUPPORTS_MULTIPLE_WAITS 1
#else
#  define CSYNCEVENT_SUPPORTS_MULTIPLE_WAITS 0
#endif


class ENGINE_API CSyncEvent : public ISyncObject
{
public:
   CSyncEvent(bool manualReset, bool initialState);
   ~CSyncEvent();

   void Set();
   void Reset();

   bool IsSet() const;

   virtual BPE_HANDLE GetHandle() const;

   // This function waits for this event to bet set or for timeout.
   // If the event is set, return value will be true, if timeout occured, it will be false.
   bool Wait(int timeoutMS = -1) const;
   
#if CSYNCEVENT_SUPPORTS_MULTIPLE_WAITS
   // This functions waits for all events to be set or for timeout.
   // If all events are set it will return true, if a timeout occurs, it will return false
   // Note: Default timeout value is infinite.
   static bool WaitForAllEvents(std::vector<CSyncEvent const*> const & events, int timeoutMS = -1);

   // This functions waits for one out of the given events to be set or for a timeout.
   // If the wait succeeds, it will return the HANDLE of the event that got set.
   // If the wait times out, it will return INVALID_HANDLE_VALUE.
   // Note: Default timeout value is infinite.
   static BPE_HANDLE WaitForOneOfMultipleEvents(std::vector<CSyncEvent const*> const & events, int timeoutMS = -1);

#endif

private:
   BPE_HANDLE mEventHandle;

private:
   BPE_DISABLE_COPY_AND_ASSIGNMENT(CSyncEvent);
};

//----------------------------------------------------------------------------
