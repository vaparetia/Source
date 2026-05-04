//------------------------------------------------------------------------------------------
// VTACStopWatch.cpp
// Bluepoint/Armature
// Copyright 2011
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/System/CStopWatch.h"

#include <rtc.h>
#include <kernel/threadmgr.h>
#include <kernel/processmgr.h>

//------------------------------------------------------------------------------------------
int64             CStopWatch::sTimerFrequency = 1000000LL;
//------------------------------------------------------------------------------------------

CStopWatch::CStopWatch()
{
   Reset();
}

//------------------------------------------------------------------------------------------

void CStopWatch::Sleep(uint32 const milliseconds)
{
   uint32 const microSeconds = milliseconds * 1000;

   sceKernelDelayThread(microSeconds);
}

//------------------------------------------------------------------------------------------

const int64 CStopWatch::GetCurrentTicks() const
{
   SceKernelSysClock clock;

   sceKernelGetProcessTime( &clock );

   return clock.quad;
}

