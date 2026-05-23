//----------------------------------------------------------------------------
// CTaskQueue.h
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

#if BPE_TARGET!=BPE_TARGET_RVL

#define BPE_TASKQUEUE_DEFINED 1

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/System/CSyncCriticalSection.h"
#include "Engine/System/CSyncEvent.h"
#include "Engine/Mechanics/FastDelegate/FastDelegate.h"

//----------------------------------------------------------------------------

class CThread;

//----------------------------------------------------------------------------

class ENGINE_API CTaskQueue
{
public:
   // Initialize task queue with a specified amount of worker threads
   // aProcessor[workerThreadCount], apThreadName[workerThreadCount] must be valid
   // NOTE: Doing a "Flush" will cause the Main-thread to also process any tasks available.
   CTaskQueue(int workerThreadCount, int* aProcessor, const char** apThreadName);
   ~CTaskQueue();

   typedef fastdelegate::FastDelegate2<uint32, uint32>   TCallback;
   
   // Add task to queue
   void AddTask(TCallback const &callback, uint32 const userData1, uint32 const userData2);
   // Adds task to queue that will stall until sync counter has been incremented.
   void AddTaskSync(TCallback const &callback, uint32 const userData1, uint32 const userData2);

   void ProcessTasks(int count=0);
   // Join worker threads in processing any remaining tasks, 
   // this function will not return until all tasks have been processed
   void FlushTasks();
   // Allows querying current number of remaining tasks
   // Note: this does not include the number of threads currently already processing tasks!
   // Do NOT use this function to chech for completion, use IsDoneWithAllTasks
   int GetCurrentNumberOfTasks();
   // Allows checking if all tasks are completed
   int IsDoneWithAllTasks();
   // Allows 'AddTaskSync' tasks to run.
   void IncrementSyncCounter();

   // Simple helper function
   static bool HasMultipleCPUs();

private:
   bool ProcessTasksThreadFunc(CThread & thread);
   static bool StaticProcessTasksThreadFunc(CThread & thread);
   
private:
   struct STask
   {
      STask(TCallback const & callback, uint32 const userData1, uint32 const userData2);
      STask(uint32 const syncCount, TCallback const & callback, uint32 const userData1, uint32 const userData2);

      static uint32 const skDontWaitForSync = 0;

      TCallback            mCallback;
      uint32               mUserData1;
      uint32               mUserData2;
      uint32               mTaskSyncCount;
   };

   void WaitForSyncCount(STask const &task);

   CSyncCriticalSection       mTasksLock;
   std::vector<STask>         mTasks;
   CSyncEvent                 mTasksArrivedEvent;
   
   std::vector< boost::shared_ptr<CThread> > mWorkerThreads;
   int32                                     mAwakeThreadCount;
   volatile uint32                           mSyncCounter;
};

//----------------------------------------------------------------------------

extern ENGINE_API CTaskQueue* gpTaskQueue;

inline CTaskQueue* TaskQueue()
{
   return gpTaskQueue;
}

#else
// TODO: Replace RVL #define with correct stub implementation.
#define BPE_TASKQUEUE_DEFINED 0
#endif