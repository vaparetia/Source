//----------------------------------------------------------------------------
// CSPURSTask.cpp
// Bluepoint
// Copyright 2006
//
// Helper class for managing tasks. This file is not compiled on SPU.
//
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include <stdlib.h>
#include "Engine/System/CSPURSTask.h"
#include "Engine/System/CSPURSManager.h"

#if BPE_TARGET == BPE_TARGET_PS3
#include <cell/atomic.h>
#endif

//----------------------------------------------------------------------------

CSPURSTask::CSPURSTask()
: mTaskStatus(kTS_None)
, mpTaskCompletedCount(NULL)
, mpCompletionGate(NULL)
{
}

//----------------------------------------------------------------------------

CSPURSTask::~CSPURSTask()
{
   WaitForCompletion();   
}

//----------------------------------------------------------------------------

void CSPURSTask::SetTaskInQueue(uint32 * pTaskCompletedCount)
{
   BPE_VERIFY((mTaskStatus == kTS_None) || CSPURSManager::AtomicCheckForValue((int*)&mTaskStatus, kTS_Completed), false, "Cannot set in queue when task in progress.");
   mTaskStatus = kTS_InTaskQueue;
   mpCompletionGate = NULL;
   mpTaskCompletedCount = pTaskCompletedCount;
}

//----------------------------------------------------------------------------
   

void CSPURSTask::WaitForCompletion()
{
#if BPE_TARGET == BPE_TARGET_PS3
   if (mTaskStatus != kTS_None)
   {
      CSPURSManager::AtomicWaitForValue((int*)&mTaskStatus, kTS_Completed);
   }
#endif   
}

//----------------------------------------------------------------------------

bool CSPURSTask::IsCompleted()
{
#if BPE_TARGET == BPE_TARGET_PS3
   if ((mTaskStatus != kTS_None) && !CSPURSManager::AtomicCheckForValue((int*)&mTaskStatus, kTS_Completed))
   {
      return false;
   }
#endif   
   return true;
}

//----------------------------------------------------------------------------

void CSPURSTask::AddCompletionGate(uint32 * pGateAddress)
{
#if BPE_TARGET == BPE_TARGET_PS3
   // Atomically write updated gate address for RSX JTS.
   cellAtomicStore32((uint32_t*)&this->mpCompletionGate, (uint32_t) pGateAddress);

   // Check to see if task is completed or pending completion
   if (IsCompleted())
   {
      // SPU task is finished, we need to overwrite the JTS with a NOP so the RSX can continue.
      // Note that a double (SPU/PPU) RSX write can occur in this case, but this is fine as they're the same NOP value.
      uint32_t const kNOP = 0;   // NOP on RSX
      *mpCompletionGate = kNOP;
   }
#endif
}

//----------------------------------------------------------------------------


