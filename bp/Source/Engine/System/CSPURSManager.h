//----------------------------------------------------------------------------
// CSPURSManager.h
// Bluepoint
// Copyright 2006
//
// Wrapper around PS3 libSPURS. Used for running code on PS3 SPUs.
//
// Singleton, you can only ever have one of these!
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "boost/scoped_ptr.hpp"
#include "boost/scoped_array.hpp"
#include "CSPURSTask.h"

struct CellSpurs2;

//----------------------------------------------------------------------------

class ENGINE_API CSPURSManager
{
public:
   CSPURSManager();
   ~CSPURSManager();

   // Temp. Tasksets are now allocated as needed
   enum ETasksetGroup
   {
      kTG_LowPriorityMultiSPUs,
      kTG_HighPrioritySingleSPU,
      
      kTG_Count
   };
   // When more than this number of tasks are added, we will stall and wait for a taskset completion.
   static int32 const kMaxTasksetsPerGroup = 2;

   // Waits for all tasks in this group to complete
   void                 WaitForTasksetGroupCompletion(ETasksetGroup const tasksetGroup);

   // Simple helper function
   static int  GetCurrentPPUThreadPriority();
   static bool AtomicCheckForValue(int *pValue, int const desiredValue);
   static void AtomicWaitForValue(int *pValue, int const desiredValue);
   
public:
#if BPE_TARGET == BPE_TARGET_PS3
   boost::scoped_ptr<CellSpurs2>             mpSPURS_Main;     // 5 SPUs
   boost::scoped_ptr<CellSpurs2>             mpSPURS_System;   // 1 SPU
#endif   
   uint32                                    mTaskCount[kTG_Count * kMaxTasksetsPerGroup];
   uint32                                    mTaskCountTotal[kTG_Count];
   uint32                                    mTaskCompletedCount[kTG_Count * kMaxTasksetsPerGroup];

private:
   void                 ResetTaskCounts();
   int                  GetTasksetIndex(ETasksetGroup const tasksetGroup);
   void                 WaitForTasksetSubGroupCompletion(int const tasksetIndex);
};

//----------------------------------------------------------------------------

extern ENGINE_API CSPURSManager* gpSpursManager;

inline CSPURSManager* SPURSManager()
{
   return gpSpursManager;
}

//----------------------------------------------------------------------------

