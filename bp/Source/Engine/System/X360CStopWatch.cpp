//------------------------------------------------------------------------------------------
// CStopWatch.cpp
// Bluepoint
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/System/CStopWatch.h"

#include <Xtl.h>

//------------------------------------------------------------------------------------------

static bool       gTimerInitialized = false;
int64             CStopWatch::sTimerFrequency = 0;

//------------------------------------------------------------------------------------------

CStopWatch::CStopWatch()
{
   if( !gTimerInitialized )
   {
      LARGE_INTEGER ticksPerSecond;
      QueryPerformanceFrequency( &ticksPerSecond );
      sTimerFrequency = ticksPerSecond.QuadPart;

      gTimerInitialized = true;
   }

   Reset();
}
   
//------------------------------------------------------------------------------------------

void CStopWatch::Sleep(uint32 const milliseconds)
{
   ::Sleep(milliseconds);
}

//------------------------------------------------------------------------------------------

const int64 CStopWatch::GetCurrentTicks() const
{
   LARGE_INTEGER currentTime;
   QueryPerformanceCounter( &currentTime );
   return currentTime.QuadPart;
}


//------------------------------------------------------------------------------------------

