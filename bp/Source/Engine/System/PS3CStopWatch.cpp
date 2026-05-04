//------------------------------------------------------------------------------------------
// CStopWatch.cpp
// Bluepoint
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/System/CStopWatch.h"

#include <sys/sys_time.h>
#include <sys/timer.h>

//------------------------------------------------------------------------------------------

static bool       gTimerInitialized = false;
int64             CStopWatch::sTimerFrequency = 0;
//------------------------------------------------------------------------------------------

CStopWatch::CStopWatch()
{
   if( !gTimerInitialized )
   {
      sTimerFrequency = sys_time_get_timebase_frequency();
      gTimerInitialized = true;
   }

   Reset();
}

//------------------------------------------------------------------------------------------

void CStopWatch::Sleep(uint32 const milliseconds)
{
   sys_timer_usleep(1000LL * milliseconds);
}

//------------------------------------------------------------------------------------------

const int64 CStopWatch::GetCurrentTicks() const
{
   uint64_t ticks;
   do
   {
#ifdef __SNC__
      ticks = __builtin_mftb();
#else
      asm volatile ("mftb %0" : "=r"(ticks));
#endif
   }
   while ((ticks & 0xFFFFFFFFUL) == 0);     // Bug fix for timer wrapping (https://ps3.scedev.net/forums/nodejump/35093)

   return (int64) ticks;
}
