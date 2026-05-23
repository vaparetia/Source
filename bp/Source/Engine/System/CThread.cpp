//----------------------------------------------------------------------------
// CThread.cpp
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/System/CThread.h"
#include "Engine/System/CSystemVar.h"

#if BPE_TARGET == BPE_TARGET_WIN32
#include <windows.h>
#elif BPE_TARGET == BPE_TARGET_X360
#include <xtl.h>
#elif BPE_TARGET == BPE_TARGET_PS3
#include <sys/ppu_thread.h>
#include <sys/synchronization.h>
#elif BPE_TARGET == BPE_TARGET_VITA
#include <kernel/threadmgr.h>
#include <sceerror.h>
#endif

#if BPE_TARGET == BPE_TARGET_X360 || BPE_TARGET == BPE_TARGET_WIN32
void SetThreadName(DWORD dwThreadID, const char* szThreadName)
{
   typedef struct tagTHREADNAME_INFO
   {
      DWORD dwType;     // must be 0x1000
      LPCSTR szName;    // pointer to name (in user address space)
      DWORD dwThreadID; // thread ID (-1 = caller thread)
      DWORD dwFlags;    // reserved for future use, must be zero
   } THREADNAME_INFO;

   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = szThreadName;
   info.dwThreadID = dwThreadID;
   info.dwFlags = 0;

   __try
   {
      RaiseException( 0x406D1388, 0, sizeof(info) / sizeof(DWORD), (DWORD*)&info );
   }
   __except( EXCEPTION_CONTINUE_EXECUTION ) {
   }
}
#endif

//----------------------------------------------------------------------------
#if BPE_TARGET == BPE_TARGET_VITA
CThread::CThread(TThreadSignal threadFunction, int processor, const char* pThreadName)
#else
CThread::CThread(TThreadSignal::slot_type const & threadFunction, int processor, const char* pThreadName)
#endif
:  mThreadHandle(BPE_INVALID_HANDLE_VALUE)
,  mTerminateEvent(false, false)
{
#if BPE_TARGET != BPE_TARGET_VITA
   mThreadImpl.connect(threadFunction);
#endif

#if BPE_TARGET == BPE_TARGET_WIN32 || BPE_TARGET == BPE_TARGET_X360
   DWORD threadID;
   mThreadHandle = CreateThread( NULL, 
                                 0,
                                 ThreadProc,
                                 this,
                                 0,
                                 &threadID);
   SetThreadName(threadID, pThreadName);
#if BPE_TARGET == BPE_TARGET_X360
   if( processor < 0 || processor > 5 )
   {
      processor = 5; // Default to valid value if passed in random junk
   }
   XSetThreadProcessor(mThreadHandle, processor);
#endif
   ResumeThread(mThreadHandle);
#endif

#if BPE_TARGET == BPE_TARGET_PS3
   sys_event_flag_attribute_t attr;
   sys_event_flag_attribute_initialize(attr);
   sys_event_flag_create(&mThreadEventFlag, &attr, kEF_IsDone);

   int const kThreadPriority = 1001;
   int const kStackSize = 32 * 1024;

   sys_ppu_thread_create(&mThread, CThread::ThreadProc, (uint64_t)this, kThreadPriority, kStackSize, SYS_PPU_THREAD_CREATE_JOINABLE, pThreadName);
#endif

#if BPE_TARGET == BPE_TARGET_VITA
   mThreadCallback = threadFunction;

   mThreadID = sceKernelCreateThread(pThreadName, CThread::ThreadProc, SCE_KERNEL_DEFAULT_PRIORITY_USER, 32 * 1024, 
      0, SCE_KERNEL_CPU_MASK_USER_ALL, NULL);

   SceKernelThreadOptParam threadParam;
   threadParam.attr = reinterpret_cast<SceUInt32>(this);
   threadParam.size = sizeof(SceUInt32);
   sceKernelStartThread(mThreadID, sizeof(threadParam), &threadParam);
#endif
}

//----------------------------------------------------------------------------

CThread::~CThread()
{
   mTerminateEvent.Set();
   
#if BPE_TARGET == BPE_TARGET_WIN32 || BPE_TARGET == BPE_TARGET_X360
   // wait for thread to close
   WaitForSingleObject(mThreadHandle, INFINITE);
   
   CloseHandle(mThreadHandle);
#endif

#if BPE_TARGET == BPE_TARGET_PS3
   uint64_t return_status;

   sys_ppu_thread_join(mThread, &return_status);
#endif

#if BPE_TARGET == BPE_TARGET_VITA
   int threadResult = 0;
   int res = sceKernelWaitThreadEnd(mThreadID, &threadResult, NULL);
   BPE_VERIFY(res == SCE_OK, false, "Error joining bootloader thread.");

   res = sceKernelDeleteThread(mThreadID);
   BPE_VERIFY(res == SCE_OK, false, "Error destroying bootloader thread.");
#endif
}

//----------------------------------------------------------------------------

bool const CThread::ShouldTerminateThread() const
{
   return mTerminateEvent.IsSet();
}

//----------------------------------------------------------------------------

int CThread::GetNumCPUs()
{
#if BPE_TARGET == BPE_TARGET_WIN32
   static int sNumCPUs = -1;
   if (sNumCPUs == -1)
   {
      // Use env var to determine number of CPUs
      CSystemVar var("NUMBER_OF_PROCESSORS");
      if (!var.IsValid())
      {
         sNumCPUs = 1;
      }
      else
      {
         sNumCPUs = atoi(var.GetValue().c_str());
         if (sNumCPUs == 0)
         {
            sNumCPUs = 1;
         }
      }
   }

   return sNumCPUs;
#elif BPE_TARGET == BPE_TARGET_X360
   return 6; // Actually returning number of cores on X360
#elif BPE_TARGET == BPE_TARGET_PS3
   return 1;
#elif BPE_TARGET == BPE_TARGET_VITA
   return 3; // One for each core.
#elif BPE_TARGET == BPE_TARGET_DREAMCAST
   return 1;
#else
#error Unsupported platform.
#endif
}

//----------------------------------------------------------------------------

#if BPE_TARGET == BPE_TARGET_WIN32 || BPE_TARGET == BPE_TARGET_X360
uint32 BPE_WINAPI CThread::ThreadProc(void * lpParam)
{
   CThread* pThis = (CThread*)lpParam;
   
   return pThis->mThreadImpl(*pThis) ? 0 : 1;
}
#endif

#if BPE_TARGET == BPE_TARGET_PS3
void BPE_WINAPI CThread::ThreadProc(uint64 lpParam)
{
   CThread* pThis = (CThread*)lpParam;

   pThis->mThreadImpl(*pThis);
}
#endif

#if BPE_TARGET == BPE_TARGET_VITA
SceInt32 CThread::ThreadProc(SceSize argSize, void* pArgBlock)
{
   SceKernelThreadOptParam* pThreadParam = reinterpret_cast<SceKernelThreadOptParam*>(pArgBlock);
   CThread* pThis = reinterpret_cast<CThread*>(pThreadParam->attr);
   pThis->mThreadCallback(*pThis);
   return 0;
}
#endif

//----------------------------------------------------------------------------
