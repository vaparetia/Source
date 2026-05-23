//------------------------------------------------------------------------------------------
// CStopWatch.cpp
// Bluepoint
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/System/CStopWatch.h"

//------------------------------------------------------------------------------------------

CStopWatch        CStopWatch::gGlobalTime;  // Used to guarentee initialization for global time
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------

const real32 CStopWatch::GetElapsedTime() const
{
   return static_cast<real32>( GetCurrentTicks() - mInitTicks ) / sTimerFrequency;
}

//------------------------------------------------------------------------------------------

const real64 CStopWatch::GetElapsedTime64() const
{
   return static_cast<real64>( GetCurrentTicks() - mInitTicks ) / sTimerFrequency;
}

//------------------------------------------------------------------------------------------

const uint32 CStopWatch::GetElapsedMilliseconds() const
{
   return static_cast<uint32>( ( ( GetCurrentTicks() - mInitTicks ) * 1000) / sTimerFrequency);
}

//------------------------------------------------------------------------------------------

const uint64 CStopWatch::GetElapsedMicroSeconds() const
{
   return ( ( ( GetCurrentTicks() - mInitTicks ) * 1000000) / sTimerFrequency);
}

//------------------------------------------------------------------------------------------

void CStopWatch::Reset()
{
   mInitTicks = GetCurrentTicks();
}

//------------------------------------------------------------------------------------------

CTimeBlock::CTimeBlock(char const * const pLabel, real64 const debuggerPrintfThreshold , real64 const consolePrintfThreshold)
: mpLabel(pLabel)
, mDebuggerPrintfThreshold(debuggerPrintfThreshold)
, mConsolePrintfThreshold(consolePrintfThreshold)
, mSW()
{
}

//------------------------------------------------------------------------------------------

CTimeBlock::~CTimeBlock()
{
   real64 const elapsedTime = mSW.GetElapsedTime64();

   real64 threshhold = 0;

#ifndef GOLD_VERSION
   void (*pOutputFunc)( const char* const, ... ) = NULL;

   if (mDebuggerPrintfThreshold < mConsolePrintfThreshold)
   {
      threshhold = mDebuggerPrintfThreshold;
      pOutputFunc = &bpe_debugger_printf;
   }
   else
   {
      threshhold = mConsolePrintfThreshold;
      pOutputFunc = &bpe_console_printf;
   }

   if (elapsedTime < threshhold) return;

   if (elapsedTime < 1e-6)
   {
      (*pOutputFunc)("%s: %0.4fns\n", mpLabel, elapsedTime * 1e9);
   }
   else if (elapsedTime < 1e-3)
   {
      (*pOutputFunc)("%s: %0.4fus\n", mpLabel, elapsedTime * 1e6);
   }
   else if (elapsedTime < 1)
   {
      (*pOutputFunc)("%s: %0.4fms\n", mpLabel, elapsedTime * 1e3);
   }
   else
   {
      (*pOutputFunc)("%s: %0.4fs\n", mpLabel, elapsedTime);
   }
#endif
}

//------------------------------------------------------------------------------------------

