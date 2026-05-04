//------------------------------------------------------------------------------------------
// CStopWatch.cpp
// Bluepoint
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/System/CStopWatch.h"

#include <revolution/os.h>

//------------------------------------------------------------------------------------------
int64             CStopWatch::sTimerFrequency = OS_TIMER_CLOCK;
//------------------------------------------------------------------------------------------

CStopWatch::CStopWatch()
{
   Reset();
}
   
//------------------------------------------------------------------------------------------

void CStopWatch::Sleep(uint32 const milliseconds)
{
   OSSleepMilliseconds( milliseconds );
}

//------------------------------------------------------------------------------------------

const int64 CStopWatch::GetCurrentTicks() const
{
   return OSGetTime();
}

