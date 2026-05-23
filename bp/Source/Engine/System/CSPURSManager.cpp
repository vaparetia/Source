//----------------------------------------------------------------------------
// CSPURSManager.cpp
// Bluepoint
// Copyright 2006
//
// Wrapper around PS3 libSPURS. Used for running code on PS3 SPUs.
//
// Singleton, you can only ever have one of these!
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include <stdlib.h>
#include "Engine/System/CSPURSManager.h"
#include "Engine/System/CStopWatch.h"
#if BPE_TARGET == BPE_TARGET_PS3
#include <sys/ppu_thread.h>
#include <sys/timer.h>
#include <cell/atomic.h>
#include <cell/spurs/types.h>
#include <cell/spurs/control.h>
#include <cell/spurs/task.h>
#include <sys/spu_initialize.h>
#include <sys/spu_thread_group.h>
#include <spu_printf.h>
#endif

//----------------------------------------------------------------------------

CSPURSManager* gpSpursManager = NULL;

//----------------------------------------------------------------------------

CSPURSManager::CSPURSManager()
#if BPE_TARGET == BPE_TARGET_PS3
:  mpSPURS_Main(new(memalign(128, sizeof(CellSpurs2))) CellSpurs2)
,  mpSPURS_System(new(memalign(128, sizeof(CellSpurs2))) CellSpurs2)
#endif
{
   BPE_VERIFY(gpSpursManager == NULL, false, "CSPURSManager initialized twice.");

   gpSpursManager = this;
#if BPE_TARGET == BPE_TARGET_PS3
   int ret;

   // Initialize 6 SPUs, with no raw SPUs
   ret = sys_spu_initialize(6, 0);
   BPE_ASSERT(!ret, "sys_spu_initialize failed.");

   int const kSPUThreadPriority = 100;
   int const kPPUThreadPriority = 10;
   ret = spu_printf_initialize(kPPUThreadPriority, NULL);
   BPE_ASSERT(!ret, "spu_printf_initialize failed.");

   // "Main" spurs instance (5 SPU's)
   {
      int const kNumSPUs_Main = 5;

	   CellSpursAttribute	attributeSpurs;
	   ret = cellSpursAttributeInitialize(&attributeSpurs, kNumSPUs_Main, kSPUThreadPriority, kPPUThreadPriority, 0 /* isExit */);
      BPE_ASSERT(!ret, "cellSpursAttributeInitialize failed for main group.");

      // Save memory by noting that this group will never get switched out.
      ret = cellSpursAttributeSetSpuThreadGroupType(&attributeSpurs, SYS_SPU_THREAD_GROUP_TYPE_EXCLUSIVE_NON_CONTEXT);
      BPE_ASSERT(!ret, "cellSpursAttributeSetSpuThreadGroupType failed for main group.");

      char spursName[] = "BP_Main";
      ret = cellSpursAttributeSetNamePrefix(&attributeSpurs, spursName, sizeof(spursName));
      BPE_ASSERT(!ret, "cellSpursAttributeSetNamePrefix failed for main group.");

      cellSpursAttributeEnableSpuPrintfIfAvailable(&attributeSpurs);
      ret = cellSpursInitializeWithAttribute2(mpSPURS_Main.get(), &attributeSpurs);
      BPE_ASSERT(!ret, "cellSpursInitialize failed for main group.");
   }

   // "System" spurs instance (1 SPU)
   {
      int const kNumSPUs_System = 1;

      CellSpursAttribute	attributeSpurs;
      ret = cellSpursAttributeInitialize(&attributeSpurs, kNumSPUs_System, kSPUThreadPriority, kPPUThreadPriority, 0 /* isExit */);
      BPE_ASSERT(!ret, "cellSpursAttributeInitialize failed for main group.");

      char spursName[] = "BP_System";
      ret = cellSpursAttributeSetNamePrefix(&attributeSpurs, spursName, sizeof(spursName));
      BPE_ASSERT(!ret, "cellSpursAttributeSetNamePrefix failed for main group.");

      cellSpursAttributeEnableSpuPrintfIfAvailable(&attributeSpurs);
      ret = cellSpursInitializeWithAttribute2(mpSPURS_System.get(), &attributeSpurs);
      BPE_ASSERT(!ret, "cellSpursInitialize failed for main group.");
   }

#endif
   
   ResetTaskCounts();
}

//----------------------------------------------------------------------------

CSPURSManager::~CSPURSManager()
{
#if BPE_TARGET == BPE_TARGET_PS3
   int ret = 0;

   ret = cellSpursFinalize(mpSPURS_Main.get());
   BPE_ASSERT(!ret, "cellSpursFinalize failed.");

   ret = cellSpursFinalize(mpSPURS_System.get());
   BPE_ASSERT(!ret, "cellSpursFinalize failed.");

   spu_printf_finalize();
#endif
   gpSpursManager = NULL;
}

//----------------------------------------------------------------------------

void CSPURSManager::WaitForTasksetGroupCompletion(ETasksetGroup const tasksetGroup)
{
   for (int loop = 0; loop < kMaxTasksetsPerGroup; loop++)
   {
      WaitForTasksetSubGroupCompletion((tasksetGroup * kMaxTasksetsPerGroup) + loop);
   }
   mTaskCountTotal[tasksetGroup] = 0;
}

//----------------------------------------------------------------------------

void CSPURSManager::WaitForTasksetSubGroupCompletion(int const tasksetIndex)
{
}

//----------------------------------------------------------------------------

int CSPURSManager::GetCurrentPPUThreadPriority()
{
#if BPE_TARGET == BPE_TARGET_PS3
   int ret;
   sys_ppu_thread_t my_ppu_thread_id;
   ret = sys_ppu_thread_get_id(&my_ppu_thread_id);
   BPE_ASSERT(!ret, "sys_ppu_thread_get_id failed");
   int prio;
   ret = sys_ppu_thread_get_priority(my_ppu_thread_id, &prio);
   BPE_ASSERT(!ret, "sys_ppu_thread_get_id failed");

   return prio;
#else
   return -1;
#endif
}

//----------------------------------------------------------------------------

bool CSPURSManager::AtomicCheckForValue(int *pValue, int const desiredValue)
{
#if BPE_TARGET == BPE_TARGET_PS3
   bool const bCheck = (cellAtomicNop32((uint32_t*)pValue) == desiredValue);
   return bCheck;
#else
   return true;
#endif
}

//----------------------------------------------------------------------------

void CSPURSManager::AtomicWaitForValue(int *pValue, int const desiredValue)
{
#if BPE_TARGET == BPE_TARGET_PS3
   CStopWatch guardTimer;
   int waitCount = 1;
   // Wait for value to match
   while (return_true())
   {
      if (cellAtomicNop32((uint32_t*)pValue) == desiredValue)
      {
         break;
      }
      sys_timer_usleep(5);
      if (guardTimer.GetElapsedMilliseconds() > (waitCount * 20))
      {
         bpe_debugger_printf("AtomicWaitForValue %d: %dms\n", desiredValue, guardTimer.GetElapsedMilliseconds());
         waitCount++;
      }
   }
#endif
   return;
}

//----------------------------------------------------------------------------

void CSPURSManager::ResetTaskCounts()
{
   for (int loop = 0; loop < kTG_Count * kMaxTasksetsPerGroup; loop++)
   {
      mTaskCount[loop] = 0;
      mTaskCompletedCount[loop] = 0;
   }

   for (int loop = 0; loop < kTG_Count; loop++)
   {
      mTaskCountTotal[loop] = 0;
   }
}

//----------------------------------------------------------------------------

int CSPURSManager::GetTasksetIndex(ETasksetGroup const tasksetGroup)
{
#if BPE_TARGET == BPE_TARGET_PS3
   int const tasksetIndex = (tasksetGroup * kMaxTasksetsPerGroup) + ((mTaskCountTotal[tasksetGroup] / CELL_SPURS_MAX_TASK) % kMaxTasksetsPerGroup);
   return tasksetIndex;
#else
   return 0;
#endif
}

//----------------------------------------------------------------------------

