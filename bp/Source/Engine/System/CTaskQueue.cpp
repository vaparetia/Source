//----------------------------------------------------------------------------
// CTaskQueue.cpp
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/System/CTaskQueue.h"

//----------------------------------------------------------------------------

#include "Engine/System/CThread.h"

#include "boost/bind.hpp"

#if BPE_TARGET == BPE_TARGET_WIN32
#include <windows.h>
#endif
//----------------------------------------------------------------------------

CTaskQueue* gpTaskQueue = NULL;

//----------------------------------------------------------------------------

CTaskQueue::STask::STask(TCallback const & callback, uint32 const userData1, uint32 const userData2)
: mCallback(callback)
, mUserData1(userData1)
, mUserData2(userData2)
, mTaskSyncCount(skDontWaitForSync)
{
}

//----------------------------------------------------------------------------

CTaskQueue::STask::STask(uint32 const syncCount, TCallback const & callback, uint32 const userData1, uint32 const userData2)
: mCallback(callback)
, mUserData1(userData1)
, mUserData2(userData2)
, mTaskSyncCount(syncCount)
{
   BPE_ASSERT(mTaskSyncCount != 0, "syncCounter should never be zero.");
}

//----------------------------------------------------------------------------

CTaskQueue::CTaskQueue(int workerThreadCount, int* aProcessor, const char** apThreadName)
:  mTasksLock()
,  mTasks()
,  mTasksArrivedEvent(false, false)
,  mWorkerThreads()
,  mAwakeThreadCount(0)
,  mSyncCounter(1)
{
   gpTaskQueue = this;

   mWorkerThreads.reserve(workerThreadCount);
   for( int i = 0; i < workerThreadCount; ++i )
   {
#if BPE_TARGET == BPE_TARGET_VITA
      boost::shared_ptr<CThread> threadPtr(new CThread(CTaskQueue::StaticProcessTasksThreadFunc, aProcessor[i], apThreadName[i]));
#else
      boost::shared_ptr<CThread> threadPtr(new CThread(boost::bind(&CTaskQueue::ProcessTasksThreadFunc, this, _1), aProcessor[i], apThreadName[i]));
#endif
      mWorkerThreads.push_back(threadPtr);
   }
}

//----------------------------------------------------------------------------

CTaskQueue::~CTaskQueue()
{
   gpTaskQueue = NULL;
}

//----------------------------------------------------------------------------

void CTaskQueue::AddTask(TCallback const &callback, uint32 const userData1, uint32 const userData2)
{
   CSyncCriticalSectionLocker lock(mTasksLock);
   mTasks.push_back(STask(callback, userData1, userData2));

   mTasksArrivedEvent.Set();
}

//----------------------------------------------------------------------------

void CTaskQueue::AddTaskSync(TCallback const &callback, uint32 const userData1, uint32 const userData2)
{
   CSyncCriticalSectionLocker lock(mTasksLock);
   mTasks.push_back(STask(mSyncCounter, callback, userData1, userData2));

   mTasksArrivedEvent.Set();
}

//----------------------------------------------------------------------------

void CTaskQueue::FlushTasks()
{
   // This function isn't thread safe!
   // Call from main thread only.

   // Just in case it wasn't incremented before.
   IncrementSyncCounter();

   // since we're just waiting for all tasks to be finished we (main controller thread) will start processing tasks ourself
   ProcessTasks();

   // need to wait until all other threads are done processing

   // have to use volatile ptr to thread count, otherwise compiler assumes that variable 
   // can't change and caches it, resulting in an endless loop here.
   int32 volatile* pThreadCount = &mAwakeThreadCount;
   while(*pThreadCount > 0)
   {
      // nothing
   }

   // Reset, as nothing should add tasks whilst we're flushing.
   mSyncCounter = 1;
}

//----------------------------------------------------------------------------

int CTaskQueue::GetCurrentNumberOfTasks()
{
   CSyncCriticalSectionLocker lock(mTasksLock);
   return mTasks.size();
}

//----------------------------------------------------------------------------

int CTaskQueue::IsDoneWithAllTasks()
{
   CSyncCriticalSectionLocker lock(mTasksLock);
   return mTasks.size() == 0 && mAwakeThreadCount == 0;
}

//----------------------------------------------------------------------------
void CTaskQueue::IncrementSyncCounter()
{
#if BPE_TARGET == BPE_TARGET_WIN32 || BPE_TARGET == BPE_TARGET_X360
   InterlockedIncrement((long *) &mSyncCounter);
   BPE_ASSERT(mSyncCounter != 0, "mSyncCounter wrapped! mSyncCounter should never be zero.");
#else
   mSyncCounter++;
#endif
}

//----------------------------------------------------------------------------

bool CTaskQueue::HasMultipleCPUs()
{
   if (CThread::GetNumCPUs() > 1)
   {
      return true;
   }
   return false;
}

//----------------------------------------------------------------------------

bool CTaskQueue::ProcessTasksThreadFunc(CThread & thread)
{
   std::vector<CSyncEvent const*> events;
   events.push_back(&thread.TerminateThreadEvent());
   events.push_back(&mTasksArrivedEvent);

#if BPE_TARGET == BPE_TARGET_WIN32 || BPE_TARGET == BPE_TARGET_X360
   while(true)
   {
      HANDLE signalledEventHandle = CSyncEvent::WaitForOneOfMultipleEvents(events);

      // if signalled handle is the thread terminate handle, we're being told to terminate
      if( signalledEventHandle == thread.TerminateThreadEvent().GetHandle() )
      {
         break;
      }

      InterlockedIncrement(&mAwakeThreadCount);

      ProcessTasks();

      InterlockedDecrement(&mAwakeThreadCount);
   }
#endif

   return true;
}

//----------------------------------------------------------------------------
bool CTaskQueue::StaticProcessTasksThreadFunc(CThread & thread)
{
   BPE_VERIFY(gpTaskQueue != NULL, false, "Invalid task queue.");
   return gpTaskQueue->ProcessTasksThreadFunc(thread);
}

//----------------------------------------------------------------------------

void CTaskQueue::ProcessTasks(int count)
{
   // process as many tasks as possible
   bool hadTask;
   do
   {
      hadTask = false;

      // I bet the overhead of copying boost::function is significant.
      // We should take a look at changing this later.
      boost::optional<STask> task;
      // other wise it must mean there is work, grab it
      {
         CSyncCriticalSectionLocker lock(mTasksLock);      
         if( mTasks.size() > count )
         {
            // could still be empty in case another thread was faster than us and there simply wasn't enough work left over for us
            task = mTasks.front();
            mTasks.erase(mTasks.begin());
         }
      }

      if( task )
      {
         hadTask = true;
         if (task->mTaskSyncCount != STask::skDontWaitForSync)
         {
            WaitForSyncCount(*task);
         }

         task->mCallback(task->mUserData1, task->mUserData2);
      }
   }
   while( hadTask );
}

//----------------------------------------------------------------------------

BPE_NOINLINE void CTaskQueue::WaitForSyncCount(STask const &task)
{
   // I put this in a different function so it will show up in a profile.

   // We need to block waiting for sync counter to increment.
   // It would be kind of useful to have some debugging metrics for this.
   while (mSyncCounter <= task.mTaskSyncCount);
}

//----------------------------------------------------------------------------


