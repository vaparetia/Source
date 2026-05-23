//----------------------------------------------------------------------------
// CSyncEvent.cpp
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CSyncEvent.h"

#if BPE_TARGET == BPE_TARGET_WIN32
#  include <windows.h>
#  define BPE_MAXIMUM_WAIT_OBJECTS MAXIMUM_WAIT_OBJECTS
#elif BPE_TARGET == BPE_TARGET_X360
#  include <xtl.h>
#  define BPE_MAXIMUM_WAIT_OBJECTS MAXIMUM_WAIT_OBJECTS
#elif BPE_TARGET == BPE_TARGET_PS3
#  include <sys/synchronization.h>
#  define BPE_MAXIMUM_WAIT_OBJECTS 64
#  define AS_FLAG(_x) ((uint64_t)1 << (int32)_x)
static sys_event_flag_t mEventFlagHandle = 0;
static uint64_t active_event_flags = 0;
#elif BPE_TARGET==BPE_TARGET_VITA
#  include <kernel/threadmgr.h>
#  include <kernel/error.h>
#  define BPE_MAXIMUM_WAIT_OBJECTS 64
// skWaitFlag has to be up here because Vita user mode 
// events can only be in the 0xFF000000 mask.
static uint32 const skVitaWaitFlag = 0x01000000;

#elif BPE_TARGET==BPE_TARGET_DREAMCAST
#  include <kos/sem.h>
// Dreamcast: ENABLE_SEPARATE_RENDER_THREAD is 0 so events are never
// waited on in practice. A semaphore covers the interface.

#else
#  error Unsupported platform!
#endif

//----------------------------------------------------------------------------

CSyncEvent::CSyncEvent(bool manualReset, bool initialState)
#if BPE_TARGET == BPE_TARGET_WIN32 || BPE_TARGET == BPE_TARGET_X360
:  mEventHandle(CreateEvent(NULL, manualReset, initialState, NULL))
#else
:  mEventHandle(BPE_INVALID_HANDLE_VALUE)
#endif
{
#if BPE_TARGET == BPE_TARGET_PS3
   if (active_event_flags == 0)
   {
      sys_event_flag_t eventFlag = -1;
      sys_event_flag_attribute_t eventAttr = { 0 };
      sys_event_flag_attribute_initialize(eventAttr);
      sys_event_flag_create(&mEventFlagHandle, &eventAttr, initialState);
   }
   BPE_ASSERT(active_event_flags != (uint64_t)-1, "TOO MANY EVENTS!!!");

   uint64_t tempFlag = active_event_flags;
   int id = 0;
   for (uint64_t tempFlag = active_event_flags; (tempFlag & 1) == 1; tempFlag <<= 1)
      id++;

   active_event_flags |= AS_FLAG(id);
   mEventHandle = (BPE_HANDLE)id;
#elif BPE_TARGET==BPE_TARGET_VITA
   SceUInt32 eventFlags = 0;

   eventFlags |= SCE_KERNEL_ATTR_TH_FIFO;
   eventFlags |= manualReset ? SCE_KERNEL_EVENT_ATTR_MANUAL_RESET : SCE_KERNEL_EVENT_ATTR_AUTO_RESET;
   eventFlags |= SCE_KERNEL_ATTR_NOTIFY_CB_WAKEUP_ONLY;

   SceUID handle = sceKernelCreateSimpleEvent( "CSyncEvent", eventFlags, initialState ? skVitaWaitFlag : 0, NULL );

   BPE_ASSERT( handle > 0, "Too many events!!" );

   mEventHandle = (BPE_HANDLE) handle;
#endif
}

//----------------------------------------------------------------------------

CSyncEvent::~CSyncEvent()
{
#if BPE_TARGET == BPE_TARGET_WIN32 || BPE_TARGET == BPE_TARGET_X360
   CloseHandle(mEventHandle);
#elif BPE_TARGET == BPE_TARGET_PS3
   active_event_flags &= ~AS_FLAG(mEventHandle);
   if (active_event_flags == 0)
      sys_event_flag_destroy(mEventFlagHandle);
#elif BPE_TARGET==BPE_TARGET_VITA
   sceKernelDeleteSimpleEvent( (SceUID) mEventHandle );
#elif BPE_TARGET==BPE_TARGET_DREAMCAST
   // DC stub: threading disabled, event is a no-op
#else
#  error Unsupported platform!
#endif
}

//----------------------------------------------------------------------------

void CSyncEvent::Set()
{
#if BPE_TARGET == BPE_TARGET_WIN32 || BPE_TARGET == BPE_TARGET_X360
   SetEvent(mEventHandle);
#elif BPE_TARGET == BPE_TARGET_PS3
   sys_event_flag_set(mEventFlagHandle, AS_FLAG(mEventHandle));
#elif BPE_TARGET==BPE_TARGET_VITA
   sceKernelSetEvent( (SceUID) mEventHandle, skVitaWaitFlag, 0 );
#elif BPE_TARGET==BPE_TARGET_DREAMCAST
   // DC stub: threading disabled, event is a no-op
#else
#  error Unsupported platform!
#endif
}

//----------------------------------------------------------------------------

void CSyncEvent::Reset()
{
#if BPE_TARGET == BPE_TARGET_WIN32 || BPE_TARGET == BPE_TARGET_X360
   ResetEvent(mEventHandle);
#elif BPE_TARGET == BPE_TARGET_PS3
   sys_event_flag_clear(mEventFlagHandle, AS_FLAG(mEventHandle));
#elif BPE_TARGET==BPE_TARGET_VITA
   sceKernelClearEvent( (SceUID) mEventHandle, 0 );
#elif BPE_TARGET==BPE_TARGET_DREAMCAST
   // DC stub: threading disabled, event is a no-op
#else
#  error Unsupported platform!
#endif
}

//----------------------------------------------------------------------------

bool CSyncEvent::IsSet() const
{
#if BPE_TARGET == BPE_TARGET_WIN32
   return WaitForSingleObject(mEventHandle, 0) == WAIT_OBJECT_0;
#elif BPE_TARGET == BPE_TARGET_PS3
   uint64_t eventValue = 0;
   sys_event_flag_get(mEventFlagHandle, &eventValue);
   return (eventValue & AS_FLAG(mEventHandle)) == AS_FLAG(mEventHandle);
#elif BPE_TARGET==BPE_TARGET_VITA
   SceUInt32 bitPattern = 0;
   SceUInt64 userData = 0;

   // If the ERROR_EVENT_COND is returned, this means that the event would have caused 
   // a wait condition, which means the event's not set.
   if ( SCE_KERNEL_ERROR_EVENT_COND == sceKernelPollEvent( (SceUID) mEventHandle, skVitaWaitFlag, &bitPattern, &userData ) )
   {
      return false;
   }
   else
   {
      return ( bitPattern & skVitaWaitFlag ) == skVitaWaitFlag;
   }
#elif BPE_TARGET==BPE_TARGET_DREAMCAST
   // DC stub: threading disabled, event is a no-op
#else
#  error Unsupported platform!
#endif
}

//----------------------------------------------------------------------------

BPE_HANDLE CSyncEvent::GetHandle() const
{
   return mEventHandle;
}

//----------------------------------------------------------------------------

bool CSyncEvent::Wait(int timeoutMS) const
{
#if BPE_TARGET == BPE_TARGET_WIN32 || BPE_TARGET == BPE_TARGET_X360
   return WaitForSingleObject(mEventHandle, timeoutMS) == WAIT_OBJECT_0;
#elif BPE_TARGET == BPE_TARGET_PS3
   return sys_event_flag_wait(mEventFlagHandle, AS_FLAG(mEventHandle), SYS_EVENT_FLAG_WAIT, NULL, timeoutMS);
#elif BPE_TARGET==BPE_TARGET_VITA
   SceUInt64 userData = 0;
   SceUInt32 timeoutUS = ( timeoutMS == -1 ) ? 0 : ( timeoutMS * 1000 );

   SceInt32 const result = sceKernelWaitEvent( (SceUID) mEventHandle, skVitaWaitFlag, NULL, &userData, timeoutMS == -1 ? NULL : &timeoutUS );

   if ( SCE_KERNEL_ERROR_WAIT_TIMEOUT == result )
   {
      return false;
   }
   else
   {
      return true;
   }
#elif BPE_TARGET==BPE_TARGET_DREAMCAST
   // DC stub: threading disabled, event is a no-op
#else
#  error Unsupported platform!
#endif
}

//----------------------------------------------------------------------------

#if CSYNCEVENT_SUPPORTS_MULTIPLE_WAITS

bool CSyncEvent::WaitForAllEvents(std::vector<CSyncEvent const*> const & events, int timeoutMS)
{
   BPE_ASSERT( events.size() < BPE_MAXIMUM_WAIT_OBJECTS, "Too many events!" );

#if BPE_TARGET == BPE_TARGET_WIN32 || BPE_TARGET == BPE_TARGET_X360
   BPE_HANDLE handles[BPE_MAXIMUM_WAIT_OBJECTS];

   for( int i = 0; i < events.size(); ++i )
   {
      handles[i] = events[i]->GetHandle();
   }

   DWORD res = WaitForMultipleObjects(events.size(), handles, true, timeoutMS);
   return res != WAIT_TIMEOUT;
#elif BPE_TARGET == BPE_TARGET_PS3
   BPE_ASSERT( events.size() < BPE_MAXIMUM_WAIT_OBJECTS, "Too many events!" );

   int64 eventFlags = 0;
   for( int i = 0; i < events.size(); ++i )
   {
      eventFlags |= AS_FLAG(events[i]->GetHandle());
   }
   int res = sys_event_flag_wait(mEventFlagHandle, eventFlags, SYS_EVENT_FLAG_WAIT_AND, NULL, timeoutMS);

   return res != ETIMEDOUT;
#elif BPE_TARGET == BPE_TARGET_VITA
   SceKernelWaitEvent eventList[BPE_MAXIMUM_WAIT_OBJECTS];
   
   for ( int i = 0; i < events.size(); ++i )
   {
      eventList[i].eventId = (SceUID)events[i]->GetHandle();
      eventList[i].eventPattern = skVitaWaitFlag;
   }

   // pTimeout is NULL for no timeout, otherwise it's 
   // an in/out with the timeout

   SceUInt32 timeoutUS;
   SceUInt32 *pTimeout;

   if ( timeoutMS == -1 )
   {
      timeoutUS = 0;
      pTimeout = NULL;
   }
   else
   {
      timeoutUS = timeoutMS * 1000;
      pTimeout = &timeoutUS;
   }

   int const res = sceKernelWaitMultipleEvents(&eventList[0], events.size(), SCE_KERNEL_EVENT_WAIT_MODE_AND, NULL, 
      pTimeout);

   return res != SCE_KERNEL_ERROR_WAIT_TIMEOUT;
#else
#  error Unsupported platform
#endif
}

#endif // CSYNCEVENT_SUPPORTS_MULTIPLE_WAITS

//----------------------------------------------------------------------------

#if CSYNCEVENT_SUPPORTS_MULTIPLE_WAITS

BPE_HANDLE CSyncEvent::WaitForOneOfMultipleEvents(std::vector<CSyncEvent const*> const & events, int timeoutMS)
{
#if BPE_TARGET == BPE_TARGET_WIN32 || BPE_TARGET == BPE_TARGET_X360
   BPE_ASSERT( events.size() < BPE_MAXIMUM_WAIT_OBJECTS, "Too many events!" );
   BPE_HANDLE handles[BPE_MAXIMUM_WAIT_OBJECTS];

   for( int i = 0; i < events.size(); ++i )
   {
      handles[i] = events[i]->GetHandle();
   }

   DWORD res = WaitForMultipleObjects(events.size(), handles, false, timeoutMS);
   
   if( res == WAIT_TIMEOUT )
      return BPE_INVALID_HANDLE_VALUE;

   int handleIndex = res - WAIT_OBJECT_0;
   
   BPE_ASSERT(handleIndex >= 0 && handleIndex < events.size(),  "Invalid BPE_HANDLE index" );

   return handles[handleIndex];
#elif BPE_TARGET == BPE_TARGET_PS3
   BPE_ASSERT( events.size() < BPE_MAXIMUM_WAIT_OBJECTS, "Too many events!" );

   int64 eventFlags = 0;
   for( int i = 0; i < events.size(); ++i )
   {
      eventFlags |= AS_FLAG(events[i]->GetHandle());
   }
   uint64_t eventFlag = 0;
   int res = sys_event_flag_wait(mEventFlagHandle, eventFlags, SYS_EVENT_FLAG_WAIT_OR, &eventFlag, timeoutMS);

   for( int i = 0; i < events.size(); ++i )
   {
      if ((eventFlag & AS_FLAG(events[i]->GetHandle())) == AS_FLAG(events[i]->GetHandle()))
         return events[i]->mEventHandle;
   }

   return BPE_INVALID_HANDLE_VALUE;
#elif BPE_TARGET == BPE_TARGET_VITA
   SceKernelWaitEvent eventList[BPE_MAXIMUM_WAIT_OBJECTS];
   SceKernelResultEvent resultList[BPE_MAXIMUM_WAIT_OBJECTS];

   for ( int i = 0; i < events.size(); ++i )
   {
      eventList[i].eventId = (SceUID)events[i]->GetHandle();
      eventList[i].eventPattern = skVitaWaitFlag;
   }

   // pTimeout is NULL for no timeout, otherwise it's 
   // an in/out with the timeout

   SceUInt32 timeoutUS;
   SceUInt32 *pTimeout;

   if ( timeoutMS == -1 )
   {
      timeoutUS = 0;
      pTimeout = NULL;
   }
   else
   {
      timeoutUS = timeoutMS * 1000;
      pTimeout = &timeoutUS;
   }

   int const res = sceKernelWaitMultipleEvents(&eventList[0], events.size(), SCE_KERNEL_EVENT_WAIT_MODE_OR, resultList, 
      pTimeout);

   // Returned result is the count of items in resultList
      
   if ( res < 0 )
   {
      return BPE_INVALID_HANDLE_VALUE;
   }
   else
   {
      for (int i = 0; i < res; ++i )
      {
         if (resultList[i].result == SCE_OK )
         {
            return BPE_HANDLE( resultList[i].eventId );
         }
      }

      return BPE_INVALID_HANDLE_VALUE;
   }
#else
#  error Unsupported platform
#endif
}

#endif // CSYNCEVENT_SUPPORTS_MULTIPLE_WAITS

//----------------------------------------------------------------------------

                                
