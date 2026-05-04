//----------------------------------------------------------------------------
// CSPURSTask.h
// Bluepoint
// Copyright 2006
//
// Wrapper around PS3 libSPURS. Used for running code on PS3 SPUs.
//
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#if defined(SPU)
#include <cell/atomic.h>
#include <cell/dma.h>
#endif

//----------------------------------------------------------------------------

class ENGINE_API CSPURSTask
{
public:
   CSPURSTask();
   ~CSPURSTask();

   enum ETaskStatus
   {
      kTS_None,
      kTS_InTaskQueue,
      kTS_Running,
      kTS_Completed,

      kTS_Count
   };

   uint32      mTaskStatus;
   uint32 *    mpCompletionGate;                 // RSX synchronization with SPU, points to command buffer JTS.
                                          // If non-null then SPU will write a NOP to this address at completion.
   uint32 *    mpTaskCompletedCount;      // Points to counter inside CSPURSManager
   
   void SetTaskInQueue(uint32 * pTaskCompletedCount);
   void WaitForCompletion();
   bool IsCompleted();
   void AddCompletionGate(uint32 * pGateAddress);  // SPU/PPU writes a zero to gate address if task is completed.
   
#if defined(SPU)
   #define BPE_SPURS_TASK_DMA_TAG   31

   static void MoveToRunningStatus(CSPURSTask *pTask, uint32_t* pAtomicBuffer)
   {
      // Must be in kTS_InTaskQueue state, update to kTS_Running.
      cellAtomicIncr32(pAtomicBuffer, (uint64_t) &pTask->mTaskStatus);
   }

   static void MoveToCompletedStatus(CSPURSTask *pTask, uint32_t* pAtomicBuffer)
   {
      // Store the task completed count pointer as it may be reset as soon as the task is flagged as completed.
      uint64_t pTaskCompletedCount = (uint64_t) cellDmaGetUint32((uint64_t) &pTask->mpTaskCompletedCount, BPE_SPURS_TASK_DMA_TAG, 0, 0);

      // Must be in kTS_Running state, update to kTS_Completed.
      cellAtomicIncr32(pAtomicBuffer, (uint64_t) &pTask->mTaskStatus);

      // Read address of RSX gate atomically.
      uint64_t * pGateAddress = (uint64_t *) cellAtomicNop32(pAtomicBuffer, (uint64_t) &pTask->mpCompletionGate);
      if (pGateAddress)
      {
         // We've got an RSX synchronization gate.
         // Write a NOP over the gate JTS.
         uint32_t const kNOP = 0;
         cellDmaPutUint32(kNOP, (uint64_t) pGateAddress, BPE_SPURS_TASK_DMA_TAG, 0, 0);
      }
         
      // We're now completed
      // Update pTaskCompletedCount and increment
      cellAtomicIncr32( pAtomicBuffer, pTaskCompletedCount);
   }
#endif   

};

//----------------------------------------------------------------------------

