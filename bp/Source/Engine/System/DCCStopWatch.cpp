//------------------------------------------------------------------------------------------
// DCCStopWatch.cpp
// Bluepoint
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/System/CStopWatch.h"

#include <arch/timer.h>
#include <kos/thread.h>

//------------------------------------------------------------------------------------------
// timer_us_gettime64 returns microseconds; frequency is therefore 1 MHz.
int64 CStopWatch::sTimerFrequency = 1000000LL;
//------------------------------------------------------------------------------------------

CStopWatch::CStopWatch()
{
   Reset();
}

//------------------------------------------------------------------------------------------

void CStopWatch::Sleep(uint32 const milliseconds)
{
   thd_sleep(milliseconds);
}

//------------------------------------------------------------------------------------------

const int64 CStopWatch::GetCurrentTicks() const
{
   return static_cast<int64>(timer_us_gettime64());
}

//------------------------------------------------------------------------------------------
