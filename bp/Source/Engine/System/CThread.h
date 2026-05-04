//----------------------------------------------------------------------------
// CThread.h
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

#if BPE_TARGET != BPE_TARGET_RVL

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/System/CSyncEvent.h"

#include "Boost/signals.hpp"

//----------------------------------------------------------------------------

class ENGINE_API CThread
{
public:
#if BPE_TARGET == BPE_TARGET_VITA
   typedef bool (*TThreadSignal)(CThread&);
#else
   typedef boost::signal<bool (CThread&)> TThreadSignal;
#endif
   
public:
#if BPE_TARGET == BPE_TARGET_VITA
   CThread(TThreadSignal threadFunction, int processor=-1, const char* pThreadName=NULL);
#else
   CThread(TThreadSignal::slot_type const & threadFunction, int processor=-1, const char* pThreadName=NULL);
#endif
   ~CThread();

   CSyncEvent const & TerminateThreadEvent() const { return mTerminateEvent; }
   bool const ShouldTerminateThread() const;

   static int GetNumCPUs();

private:
#if BPE_TARGET != BPE_TARGET_VITA
   TThreadSignal     mThreadImpl;
#endif
   BPE_HANDLE        mThreadHandle;
   CSyncEvent        mTerminateEvent;

private:
   BPE_DISABLE_COPY_AND_ASSIGNMENT(CThread);

#if BPE_TARGET == BPE_TARGET_WIN32 || BPE_TARGET == BPE_TARGET_X360
   static uint32 BPE_WINAPI ThreadProc(void * lpParam);
#endif

#if BPE_TARGET == BPE_TARGET_PS3
   enum EEventFlag
   {
      kEF_IsReady = (1 << 0),
      kEF_IsDone  = (1 << 1)
   };

   sys_event_flag_t              mThreadEventFlag;
   sys_ppu_thread_t              mThread;
   static void BPE_WINAPI      ThreadProc(uint64 lpParam);
#endif

#if BPE_TARGET == BPE_TARGET_VITA
   SceUID mThreadID;
   TThreadSignal mThreadCallback;
   static SceInt32 ThreadProc(SceSize argSize, void* pArgBlock);
#endif
};

#endif // TARGET != RVL

//----------------------------------------------------------------------------
