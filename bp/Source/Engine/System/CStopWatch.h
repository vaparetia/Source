//------------------------------------------------------------------------------------------
// CStopWatch.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Basics/BPEEnvironment.h"

//------------------------------------------------------------------------------------------

class ENGINE_API CStopWatch
{
public:
   CStopWatch();
   
   // returns elapsed time since last reset in seconds
   const real32 GetElapsedTime() const;
   const real64 GetElapsedTime64() const;
   const uint32 GetElapsedMilliseconds() const;
   const uint64 GetElapsedMicroSeconds() const; //N.B. wraps around every 71 minutes if a uint32!

   // resets the timer back to zero
   void Reset();

   static void Sleep(uint32 const milliseconds);
   static CStopWatch gGlobalTime;

private:
   int64 const GetCurrentTicks() const;

private:
   int64    mInitTicks;

   static int64 sTimerFrequency;
};

//------------------------------------------------------------------------------------------
// Use this to time blocks of code.
// Note that QueryPerformanceCounter has ~1.85us overhead on my A64x2 3800+
class ENGINE_API CTimeBlock
{
public:
   CTimeBlock(char const * const pLabel, real64 const debuggerPrintfThreshold = 0.0, real64 const consolePrintfThreshold = 1.0);
   ~CTimeBlock();

   char const *   mpLabel;
   real64         mDebuggerPrintfThreshold;
   real64         mConsolePrintfThreshold;
   CStopWatch     mSW;
};

#define BPE_TIMEFUNCTION     CTimeBlock const tf(__FUNCTION__)

//------------------------------------------------------------------------------------------
