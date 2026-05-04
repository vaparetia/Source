//----------------------------------------------------------------------------
// threading.c
//----------------------------------------------------------------------------

//#define BP_USE_CUSTOM_FIBER_MEMORY_PARTITION 1

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/Basics/BPEAssert.h"
#include "Engine/System/CStopWatch.h"

#include "stdio.h"
#include "string.h"

#include "MGS_Common.h"

#ifdef BP_WIN32
#include "xmmintrin.h"
#include "windows.h"

#endif

#if BP_360
#include <xtl.h>
#endif

#ifdef BP_PS3
#include <stdlib.h>
#include <cell/fiber/ppu_fiber.h>
#include <cell/fiber/ppu_context.h>
#endif //BP_PS3

#ifdef BP_VITA
#  include <stdlib.h>
#  include <libsysmodule.h>
#  include <sce_fiber.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define BP_STACKSIZE_MULTIPLIER 8   //BP Soren: just added a multiplier in case we use more stack space than expected

#define BP_STACKCHECK_FILLER  0xdeadbeaf
#define BP_STACKCHECK_SIZE    4

#include "bp_threading.h"

//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

//#define threadPrintF(x, ...) printf(x, __VA_ARGS__)
#define threadPrintF(x, ...) //

//----------------------------------------------------------------------------

#define WAIT_TYPE_NOT         0
#define WAIT_TYPE_WAKEUP      1
#define WAIT_TYPE_SEMAPHORE   2


//----------------------------------------------------------------------------

struct SBPThread_Debug
{
   char const *      mpThreadName;
   void              (*mThreadFunc)(void *);
   int               mTid;
};

struct SBPThread
{
   struct ThreadParam   mParm;
   int                  mFiber;
   int                  mCurrentSubThreadFiber;
   SBPThread_Debug      mDebug;
   void*                mFiberStackPtr;
   uint64               mDelayThreadWakeupTime;
   SBPThread *          mpNextThreadInQueue;
   bool                 mbAutoDelete;
};

SBPThread gThreads[PS2_MAX_THREADS];

SBPThread* gCurrentThread = NULL;
SBPThread* spThreadReadyQueue = NULL;
SBPThread* gCurrentWaitingForVsyncThread = NULL;

struct SBPSemaphore_Debug
{
   char const *      mpSemaName;
   int               mSid;
};

struct SBPSemaphore
{
   struct SemaParam     mParam;
   int                  mWaitQueue[16];
   int                  mValid;
   SBPSemaphore_Debug   mDebug;
};

SBPSemaphore gSemaphores[PS2_MAX_SEMAPHORES];

//----------------------------------------------------------------------------

#ifdef BP_PS3
static void* sCurrentFiber=NULL;
#endif

#ifdef BP_VITA
static SceFiber *sCurrentFiber = NULL;
#endif

void* bp_getcurrentfiber()
{
#if BP_WIN32 || BP_360
   return GetCurrentFiber();
#else
   return sCurrentFiber;
#endif
}

void bp_switchtofiber(void* fiberID)
{
#if BP_WIN32 || BP_360
   SwitchToFiber(fiberID);
#elif BP_PS3
   CellFiberPpuContext* fiberContext=(CellFiberPpuContext*)fiberID;
   if ( sCurrentFiber != fiberContext )
   {
      sCurrentFiber=fiberContext;
      int ret = cellFiberPpuContextSwitch(fiberContext, NULL, NULL);
      BPE_ASSERT_NO_MSG(ret==CELL_OK);
   }

#elif BP_VITA
   SceFiber *fiberContext = (SceFiber *) fiberID;

   if ( sCurrentFiber != fiberContext )
   {
      sCurrentFiber = fiberContext;
      int ret = sceFiberSwitch( sCurrentFiber, 0, 0 );

      BPE_ASSERT_NO_MSG(ret==SCE_OK);
   }
#else
#  error UNKNOWN PLATFORM
#endif

   //See if the last thread wanted to be deleted on exit.
   for( int i = 0; i < PS2_MAX_THREADS; ++i )
   {
      SBPThread* pThread = gThreads + i;
      if( pThread->mbAutoDelete )
      {
         pThread->mbAutoDelete = false;
         BP_DeleteThread(i);
      }
   }
}

#ifdef BP_PS3

#if BP_USE_CUSTOM_FIBER_MEMORY_PARTITION
#include "iosmalloc.h"
namespace
{
   bool fiberStackPtnInit = false;
   IOSMallocPtn fiberStackPtn;
};

void bp_initFiberStackPtn()
{
   if( fiberStackPtnInit )
      return;

   fiberStackPtnInit = true;

   int const size = 16*1024*1024;
   void* ptr = memalign(16, size);

   iosInitRootPartition( &fiberStackPtn, (uint32)ptr, (uint32)ptr + size, 0);
}
#endif // BP_USE_CUSTOM_FIBER_MEMORY_PARTITION

#endif // BP_PS3

int fiberDeleteCount=0;
void bp_deletefiber(void* fiberID,void* stackPtr)
{
   fiberDeleteCount++;
#if BP_WIN32 || BP_360
   DeleteFiber(fiberID);
#elif BP_VITA
   SceFiber *fiberContext = (SceFiber*)fiberID;

   BPE_ASSERT_NO_MSG(sCurrentFiber!=fiberContext);
   int ret = sceFiberFinalize( fiberContext );
   BPE_ASSERT_NO_MSG(ret==SCE_OK);

   if ( stackPtr )
   {
      free( stackPtr );
   }

   free( fiberContext );

#elif BP_PS3
   {
      CellFiberPpuContext* fiberContext=(CellFiberPpuContext*)fiberID;
#  ifndef FINAL_VERSION
      int i;
      if(stackPtr)
         for(i=0;i<BP_STACKCHECK_SIZE;i++)
            BPE_ASSERT_NO_MSG(((uint32*)stackPtr)[i]==BP_STACKCHECK_FILLER);
#  endif // FINAL_VERSION
      BPE_ASSERT_NO_MSG(sCurrentFiber!=fiberID);
      int ret = cellFiberPpuContextFinalize(fiberContext);
      BPE_ASSERT_NO_MSG(ret==CELL_OK);

#  if BP_USE_CUSTOM_FIBER_MEMORY_PARTITION
      if(stackPtr)
         iosFree(stackPtr);

      iosFree(fiberContext);
#  else // !BP_USE_CUSTOM_FIBER_MEMORY_PARTITION
      if(stackPtr)
         free(stackPtr);

      free(fiberContext);
#  endif // BP_USE_CUSTOM_FIBER_MEMORY_PARTITION
   }
#else
#  error Unknown platform
#endif
}

#ifdef BP_PS3
void bp_dummystartfiber(uint64_t arg, CellFiberPpuContext* fiberFrom)
{

   //decode 64-bit arg into the function pointer and proper arg pointer
   void (*routine)(void *)=(void(*)(void*)) ((arg>>32)&0xffffffff);
//   *(unsigned int*)&routine=(unsigned int)argptr2;
   routine((void*)(arg&0xffffffff));
}
#endif

void* bp_createfiber(int stackSize,void (*routine)(void *), void* arg,void** stackPtrPtr)
{
#if BP_WIN32 || BP_360
   *stackPtrPtr=0;   //not needed on Win32
   return CreateFiber(stackSize*BP_STACKSIZE_MULTIPLIER, (LPFIBER_START_ROUTINE)routine, arg);
#elif BP_VITA
   SceFiber *fiberContext = (SceFiber *) memalign( SCE_FIBER_ALIGNMENT, SCE_FIBER_SIZE );

   BPE_ASSERT_NO_MSG(stackSize>=2048);
   int memorySize = stackSize*BP_STACKSIZE_MULTIPLIER + SCE_FIBER_CONTEXT_MINIMUM_SIZE;

   uint32* fiberStack = (uint32*)memalign(SCE_FIBER_CONTEXT_ALIGNMENT, memorySize);
   BPE_ASSERT_NO_MSG(fiberStack!=0);

   for( int i = 0; i < BP_STACKCHECK_SIZE; ++i )
      fiberStack[i] = BP_STACKCHECK_FILLER;

   *stackPtrPtr=fiberStack;

   SceFiberEntry entryPoint = (SceFiberEntry) routine;
   int ret = sceFiberInitialize( 
      fiberContext, 
      "BP Fiber", 
      entryPoint, 
      (unsigned) arg, 
      fiberStack, 
      memorySize, 
      NULL );
   BPE_ASSERT_NO_MSG(ret == SCE_OK);

   return fiberContext;

#elif BP_PS3
   
#  if BP_USE_CUSTOM_FIBER_MEMORY_PARTITION
   bp_initFiberStackPtn();
#  endif

//   CellFiberPpuContextAttribute* attr=memalign(CELL_FIBER_PPU_CONTEXT_ATTRIBUTE_ALIGN, CELL_FIBER_PPU_CONTEXT_ATTRIBUTE_SIZE);
   CellFiberPpuContextAttribute attr;
   int ret = cellFiberPpuContextAttributeInitialize(&attr);
   BPE_ASSERT_NO_MSG(ret == CELL_OK);
   attr.debuggerSupport = true;
   static int test1=1000;
   sprintf(attr.name,"Bp Fiber %d",test1);
   test1++;

   //BP Soren: Can't use iosMallocAlign, as the ios allocation system is not initialized when we start to use threads!

#  if BP_USE_CUSTOM_FIBER_MEMORY_PARTITION
   CellFiberPpuContext* fiberContext = (CellFiberPpuContext*)iosMallocAlign(&fiberStackPtn, CELL_FIBER_PPU_CONTEXT_SIZE, CELL_FIBER_PPU_CONTEXT_ALIGN);
#  else
   CellFiberPpuContext* fiberContext = (CellFiberPpuContext*)memalign(CELL_FIBER_PPU_CONTEXT_ALIGN,CELL_FIBER_PPU_CONTEXT_SIZE);
#  endif

   BPE_ASSERT_NO_MSG(stackSize>=2048);
   int memorySize = stackSize*BP_STACKSIZE_MULTIPLIER;

#  if BP_USE_CUSTOM_FIBER_MEMORY_PARTITION
   uint32* fiberStack = (uint32*)iosMallocAlign(&fiberStackPtn, memorySize, CELL_FIBER_PPU_CONTEXT_STACK_ALIGN);
#  else
   uint32* fiberStack = (uint32*)memalign(CELL_FIBER_PPU_CONTEXT_STACK_ALIGN,stackSize*BP_STACKSIZE_MULTIPLIER);
#  endif
   BPE_ASSERT_NO_MSG(fiberStack!=0);

   for( int i = 0; i < BP_STACKCHECK_SIZE; ++i )
      fiberStack[i] = BP_STACKCHECK_FILLER;

   //   CellFiberPpuContext* fiberContext = (CellFiberPpuContext*)iosMallocAlign(&rootPtn,CELL_FIBER_PPU_CONTEXT_SIZE,CELL_FIBER_PPU_CONTEXT_ALIGN);
//   void* fiberStack = iosMallocAlign(&rootPtn,stackSize*BP_STACKSIZE_MULTIPLIER,CELL_FIBER_PPU_CONTEXT_STACK_ALIGN);
   *stackPtrPtr=fiberStack;

   //we are adding a "dummy startfiber" function to match the PS3 lib standards, and it takes an int64_t as parameters
   //so we will put the true function pointer in the upper 32 bits and the void pointer arg as the lower 32 bits
   uint64_t mixedArg=(((uint64_t)routine)<<32)|(((uint64_t)arg)&0xffffffff);
   ret = cellFiberPpuContextInitialize(
      fiberContext,
      &bp_dummystartfiber,		// entry function
      mixedArg,	// arg
      fiberStack, stackSize*BP_STACKSIZE_MULTIPLIER,
      &attr
      );
   BPE_ASSERT_NO_MSG(ret == CELL_OK);

   return fiberContext;
#else
#  error Unknown platform
#endif
}

//----------------------------------------------------------------------------

static bool bp_ready_queue_sanity_check()
{
   const SBPThread * pPrevThread = NULL;
   for( const SBPThread * pCurrThread = spThreadReadyQueue; pCurrThread != NULL; pCurrThread = pCurrThread->mpNextThreadInQueue )
   {
      BPE_ASSERT_NO_MSG( pPrevThread == NULL || pPrevThread->mParm.currentPriority <= pCurrThread->mParm.currentPriority );
      BPE_ASSERT_NO_MSG( pCurrThread->mParm.status == THS_READY || pCurrThread->mParm.status == THS_RUN );
      pPrevThread = pCurrThread;
   }
   return true;
}

static void bp_add_to_ready_queue( SBPThread * const pThread )
{
   BPE_ASSERT_NO_MSG( bp_ready_queue_sanity_check() );
   // See section 1.5 of EE Overview - Kernel manual.
   // All threads in READY or RUN state are in the ready queue, ordered by priority
   // then by order in which they were put in the READY state.

   //Shouldn't already be in ready queue
   BPE_ASSERT_NO_MSG( pThread->mpNextThreadInQueue == NULL );

   SBPThread * pPrevReadyThread = NULL;
   SBPThread * pNextReadyThread = NULL;

   //insert pThread in front of the first thread in the list that's null or has a lower (higher number) priority.
   for( pNextReadyThread = spThreadReadyQueue; pNextReadyThread != NULL && pNextReadyThread->mParm.currentPriority <= pThread->mParm.currentPriority; pNextReadyThread = pNextReadyThread->mpNextThreadInQueue )
   {
      pPrevReadyThread = pNextReadyThread;
   }

   pThread->mpNextThreadInQueue = pNextReadyThread;
   if( pPrevReadyThread )
   {
      pPrevReadyThread->mpNextThreadInQueue = pThread;
   }
   else
   {
      //First thread in READY queue.
      spThreadReadyQueue = pThread;
   }
}

//----------------------------------------------------------------------------

static void bp_remove_from_ready_queue( SBPThread * const pThread )
{
   BPE_ASSERT_NO_MSG( spThreadReadyQueue != NULL );  //READY queue should not be empty

   if( spThreadReadyQueue == pThread )
   {
      spThreadReadyQueue = pThread->mpNextThreadInQueue;
   }
   else
   {
      SBPThread * pPrevReadyThread = spThreadReadyQueue;
      while( pPrevReadyThread->mpNextThreadInQueue != pThread )
      {
         pPrevReadyThread = pPrevReadyThread->mpNextThreadInQueue;

         BPE_ASSERT_NO_MSG( pPrevReadyThread != NULL );  //pThread not found in READY queue
      }

      pPrevReadyThread->mpNextThreadInQueue = pThread->mpNextThreadInQueue;
   }

   pThread->mpNextThreadInQueue = NULL;
   BPE_ASSERT_NO_MSG( bp_ready_queue_sanity_check() );
}

//----------------------------------------------------------------------------

void ps2_threading_emulation_init(void(*primaryfiberfunction)(void* arg))
{
   memset(gThreads, 0, sizeof(gThreads));

   // Thread debug initialization code.
   for (int i = 0; i < PS2_MAX_THREADS; ++i)
   {
      SBPThread* pThread = gThreads + i;
      pThread->mDebug.mTid = i;
      pThread->mDebug.mpThreadName = "Unknown";
   }

   memset(gSemaphores, 0, sizeof(gSemaphores));

   // Sema debug initialization code.
   for (int i = 0; i < PS2_MAX_SEMAPHORES; ++i)
   {
      SBPSemaphore* pSema = gSemaphores + i;
      pSema->mDebug.mSid = i;
      pSema->mDebug.mpSemaName = "Unknown";
   }



   SBPThread* pThread = gThreads;
   pThread->mParm.status = THS_RUN;

   pThread->mParm.initPriority = pThread->mParm.currentPriority = 1;
   pThread->mParm.waitType = WAIT_TYPE_NOT;
   pThread->mParm.waitId = 0;
   gCurrentThread = pThread;

   bp_add_to_ready_queue( pThread );

#if BP_WIN32 || BP_360
   //BP Soren: on win32 we convert this thread to a fiber and keep running with primaryfiberfunction.
   pThread->mFiber = (int)ConvertThreadToFiber(NULL);
   pThread->mParm.entry = (void (__cdecl*)(void*))0xFFFFFFFF;
   primaryfiberfunction(NULL);

#elif BP_VITA
   int ret = sceSysmoduleLoadModule( SCE_SYSMODULE_FIBER );
   BPE_ASSERT_NO_MSG(ret == SCE_OK);

#  define PRIMARY_FIBER_STACK_SIZE (64*1024/BP_STACKSIZE_MULTIPLIER)
   pThread->mFiber = (int) bp_createfiber( PRIMARY_FIBER_STACK_SIZE, primaryfiberfunction, NULL, &pThread->mFiberStackPtr );

   sCurrentFiber = (SceFiber *) pThread->mFiber;

   ret = sceFiberRun( sCurrentFiber, 0, NULL );
   BPE_ASSERT_NO_MSG(ret == SCE_OK);

#elif BP_PS3
   //BP Soren: On PS3 we create the fiber with primaryfiberfunction
   int ret = cellFiberPpuInitialize();
   BPE_ASSERT_NO_MSG(ret == CELL_OK);
#  define PRIMARY_FIBER_STACK_SIZE (64*1024/BP_STACKSIZE_MULTIPLIER)
   pThread->mFiber =(int)bp_createfiber(PRIMARY_FIBER_STACK_SIZE, primaryfiberfunction, NULL,&pThread->mFiberStackPtr);

   sCurrentFiber=(void*)pThread->mFiber;
   int cause;
   ret = cellFiberPpuContextRun((CellFiberPpuContext*)sCurrentFiber, &cause, NULL,	NULL);
   BPE_ASSERT_NO_MSG(ret == CELL_OK);
   BPE_ASSERT_NO_MSG(cause == CELL_OK);
#else
#  error Unknown platform.
#endif

}

//----------------------------------------------------------------------------
// Threads
//----------------------------------------------------------------------------

int bp_find_thread_slot()
{
   for( int i = 0; i < PS2_MAX_THREADS; ++i )
   {
      if( gThreads[i].mFiber == NULL )
         return i;
   }

   return -1;
}

//----------------------------------------------------------------------------

void bp_free_thread_slot(int tid)
{
   gThreads[tid].mFiber = NULL;
   gThreads[tid].mFiberStackPtr = NULL;
}

//----------------------------------------------------------------------------

void bp_wakeup_delayed_threads()
{
   //Wake up any threads that have been delayed and whose timer has expired before rescheduling.
   const uint64 nowMicros = CStopWatch::gGlobalTime.GetElapsedMicroSeconds();

   for( int i = 0; i < PS2_MAX_THREADS; ++i )
   {
      SBPThread* pThread = gThreads + i;

      if( pThread->mFiber != NULL )
      {
         if( pThread->mDelayThreadWakeupTime != 0 )
         {
            if( pThread->mParm.status == THS_WAIT && pThread->mParm.waitType == WAIT_TYPE_WAKEUP )
            {
               if( nowMicros >= pThread->mDelayThreadWakeupTime )
               {
                  //Time to wake it up.
                  pThread->mDelayThreadWakeupTime = 0;

                  pThread->mParm.wakeupCount++;

                  if( pThread->mParm.wakeupCount != 0 )
                     BP_BREAK;

                  if( pThread == gCurrentThread )
                     BP_BREAK;

                  // Make woken up thread ready to run.
                  pThread->mParm.status = THS_READY;
                  pThread->mParm.waitType = WAIT_TYPE_NOT;
                  pThread->mParm.waitId = 0;

                  bp_add_to_ready_queue( pThread );
               }
               else
               {
               }
            }
            else
            {
               BP_BREAK;
            }
         }
      }
   }

   //Threads are about to be rescheduled.
}

//----------------------------------------------------------------------------

void bp_reschedule_threads( const bool bCheckWakeupDelayed = true )
{
   if( bCheckWakeupDelayed )
   {
      //prevent reschedule due to a DelayThread() causing the same thread to be woken up
      //if the time was small.
      bp_wakeup_delayed_threads();
   }

   //See if we're waiting for a vsync signal.
   //Allow all the other threads to run and yield before faking the vsync.
   if( gCurrentWaitingForVsyncThread != NULL && spThreadReadyQueue == NULL )
   {
      int waitingTid = gCurrentWaitingForVsyncThread - gThreads;
      gCurrentWaitingForVsyncThread = NULL;
      gCurrentThread = NULL;
      BP_WakeupThread( waitingTid );
      return;
   }

   //Take the head of the ready queue to run.
   BPE_ASSERT_NO_MSG( spThreadReadyQueue != NULL );
   BPE_ASSERT_NO_MSG( spThreadReadyQueue->mParm.status == THS_READY );
   int threadId = (int)(spThreadReadyQueue - gThreads);

   if( threadId == -1)
      BP_BREAK;

   if( threadId != BP_GetThreadId() )
   {
      SBPThread* pThread = gThreads + threadId;
      pThread->mParm.status = THS_RUN;
      
      if( gCurrentThread && gCurrentThread->mParm.status == THS_RUN )
         BP_BREAK;

      threadPrintF("Thd SWITCH: %2d (%s) -> %2d (%s)\n",  gCurrentThread->mDebug.mTid, gCurrentThread->mDebug.mpThreadName, pThread->mDebug.mTid, pThread->mDebug.mpThreadName);

      gCurrentThread = pThread;
      if( pThread->mCurrentSubThreadFiber )
         bp_switchtofiber((void*)pThread->mCurrentSubThreadFiber);
      else
         bp_switchtofiber((void*)pThread->mFiber);
   }
   else
   {
      // Sanity check
      if ((gCurrentThread->mParm.status != THS_RUN) && (gCurrentThread->mParm.status != THS_READY))
         BP_BREAK;

      // Didn't switch threads, make sure that if 'bp_force_reschedule_threads' set the current status to READY we reset it to RUN
      gCurrentThread->mParm.status = THS_RUN;
   }
}


//----------------------------------------------------------------------------

void bp_yield_ps2_thread_to_vsync()
{
   //Replacement for event signal mechanism used on PS2 to sleep the main thread
   //until the vsync happened, which allowed the lower-priority threads to run
   //in the meantime.
   //For us, for now, the lower priority threads will not be interrupted and we'll
   //assume that they all yield within a reasonable time slice.

   BPE_ASSERT_NO_MSG( gCurrentWaitingForVsyncThread == NULL );

   gCurrentWaitingForVsyncThread = gCurrentThread;

   BP_SleepThread();
}

//----------------------------------------------------------------------------

void bp_force_reschedule_threads()
{
   if( !gCurrentThread || gCurrentThread->mParm.status != THS_RUN )
      BP_BREAK;

   gCurrentThread->mParm.status = THS_READY;

   bp_reschedule_threads();
}

//----------------------------------------------------------------------------

void bp_set_subthread_fiber_for_thread(int tid, int subthreadFiber)
{
   SBPThread* pThread = gThreads + tid;
   
   if( pThread->mCurrentSubThreadFiber != NULL && subthreadFiber != NULL )
      BP_BREAK;

   pThread->mCurrentSubThreadFiber = subthreadFiber;
}

//----------------------------------------------------------------------------
// Annotate debug information on thread.
void bp_debug_thread__annotate_thread_func(int tid, void (*icoThreadFunc)(void *))
{
   SBPThread* pThread = gThreads + tid;
   pThread->mDebug.mThreadFunc = icoThreadFunc;
}

//----------------------------------------------------------------------------
// Annotate debug information on thread.
void bp_debug_thread__annotate_thread_name(int tid, char const * pName)
{
   SBPThread* pThread = gThreads + tid;
   pThread->mDebug.mpThreadName = pName;
}

//----------------------------------------------------------------------------
// Annotate debug information on thread.
void bp_debug_thread__dump_global_thread_info(void)
{
//   SBPThread* pThread = gThreads + tid;
//   pThread->mDebug.mIcoThreadFunc = icoThreadFunc;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void BP_ThreadExit(void)
{
   // Switch current thread state to DORMANT and switch to next thread.
   BPE_ASSERT_NO_MSG( gCurrentThread->mParm.status == THS_RUN );
   bp_remove_from_ready_queue( gCurrentThread );

   gCurrentThread->mParm.status = THS_DORMANT;

   bp_reschedule_threads();
}

//----------------------------------------------------------------------------

int BP_CreateThread(struct ThreadParam * parm)
{
   int tid = bp_find_thread_slot();
   if( tid != -1 )
   {
      SBPThread* pThread = gThreads + tid;

      // AndyO: Suspect that some parm state reset should be moved to 'StartThread'.
      pThread->mParm = *parm;
      pThread->mParm.wakeupCount = 0;
      pThread->mParm.status = THS_DORMANT;
      pThread->mParm.currentPriority = pThread->mParm.initPriority;
      pThread->mFiber = 0;
      pThread->mFiberStackPtr = NULL;
      pThread->mDelayThreadWakeupTime = 0;
      pThread->mpNextThreadInQueue = NULL;
   }

   return tid;
}

//----------------------------------------------------------------------------

int BP_DeleteThread(int tid)
{
   BPE_ASSERT_NO_MSG(BP_GetThreadId() != tid);
   SBPThread* pThread = gThreads + tid;
   BPE_ASSERT_NO_MSG(pThread->mFiber!=0);
   BPE_ASSERT_NO_MSG(pThread->mParm.status == THS_DORMANT);
   bp_deletefiber((void*)pThread->mFiber,pThread->mFiberStackPtr);
   bp_free_thread_slot(tid);

   return 0;
}

//----------------------------------------------------------------------------

int BP_StartThread(int tid, void *arg)
{
   if( tid != -1 )
   {
      int const currentTid = BP_GetThreadId();
      
      SBPThread* pThread = gThreads + tid;

      if( pThread == gCurrentThread )
         BP_BREAK;

      // Currently unknown which is the correct behavior

      // Make thread ready for execution but don't schedule anything.
      if( pThread->mParm.status != THS_DORMANT)
         BP_BREAK;

      // Now create thread
      pThread->mFiber = (int)bp_createfiber(pThread->mParm.stackSize, pThread->mParm.entry, arg,&pThread->mFiberStackPtr);
      pThread->mParm.status = THS_READY;

      bp_add_to_ready_queue( pThread );

      bp_force_reschedule_threads();  // allow threads to run
   }

   return tid;
}

//----------------------------------------------------------------------------

int BP_SuspendThread(int tid)
{
   //BP_TODO - Goes from READY (and RUN?) to SUSPENDED state
   BP_BREAK;
   return -1;
}

//----------------------------------------------------------------------------

int BP_ResumeThread(int tid)
{
   //BP_TODO - Goes from SUSPENDED to READY state
   BP_BREAK;
   return -1;
}

//----------------------------------------------------------------------------

void BP_ExitDeleteThread(void)
{
   // Switch current thread state to DORMANT and switch to next thread.
   BPE_ASSERT_NO_MSG( gCurrentThread->mParm.status == THS_RUN );
   bp_remove_from_ready_queue( gCurrentThread );

   gCurrentThread->mParm.status = THS_DORMANT;

   // Mark this thread for immediate deletion after the switch.
   gCurrentThread->mbAutoDelete = true;

   bp_reschedule_threads();
}

//----------------------------------------------------------------------------
// Forcibly terminates the thread specified by thid and places it in DORMANT state.
// If the thread had been enqueued in a queue (such as a semaphore wait), it will be deleted from that queue.
int BP_TerminateThread(int tid)
{
   BPE_ASSERT_NO_MSG(BP_GetThreadId() != tid);
   SBPThread* pThread = gThreads + tid;

   if ((pThread->mParm.status == THS_WAIT) && 
       (pThread->mParm.waitType == WAIT_TYPE_SEMAPHORE))
   {
      SBPSemaphore* pSema = gSemaphores + pThread->mParm.waitId;
      BPE_ASSERT_NO_MSG(pSema->mValid!=0);

      // Remove from Sema wait queue
      for(int i = 0; i < pSema->mParam.numWaitThreads; ++i )
      {          
         if (pSema->mWaitQueue[i] == tid )
         {
            // Remove this thread from wait queue if not last entry
            if (i < (pSema->mParam.numWaitThreads - 1))
            {
               memmove(&pSema->mWaitQueue[i], &pSema->mWaitQueue[i + 1], sizeof(pSema->mWaitQueue[0]) * (pSema->mParam.numWaitThreads - i));
            }
            pSema->mParam.numWaitThreads--;
            goto DELETED_FROM_QUEUE;
         }
      }
      BPE_ASSERT_NO_MSG(FALSE); // Couldn't find this thread in wait queue
DELETED_FROM_QUEUE:
      ;
   }

   if( pThread->mParm.status == THS_RUN || pThread->mParm.status == THS_READY )
   {
      bp_remove_from_ready_queue( pThread );
   }

   // Set as DORMANT
   pThread->mParm.status = THS_DORMANT;
   pThread->mParm.waitType = WAIT_TYPE_NOT;
   pThread->mParm.waitId = 0;
   pThread->mParm.wakeupCount = 0;

   return 0;
}

//----------------------------------------------------------------------------

int BP_ChangeThreadPriority(int tid, int prio )
{
   if( tid != -1 )
   {
      SBPThread* pThread = gThreads + tid;
      if( pThread->mParm.status == THS_RUN || pThread->mParm.status == THS_READY )
      {
         bp_remove_from_ready_queue( pThread );
         pThread->mParm.currentPriority = prio;
         bp_add_to_ready_queue( pThread );
      }
      else
      {
         pThread->mParm.currentPriority = prio;
      }

      bp_force_reschedule_threads();  // allow change in priority to trigger thread rescheduling
   }
   
   return tid;
}

//----------------------------------------------------------------------------

int BP_GetThreadId(void)
{
   for( int i = 0; i < PS2_MAX_THREADS; ++i )
   {
      // Use cached current thread ptr, the reason this is done instead of looking for the current active "fiber" is because
      // subthreading needs to use fibers as well (in that case we'll have a fiber id that doesn't match ANY thread)
      if( gThreads + i == gCurrentThread )
         return i;
   }
   return -1;
}

//----------------------------------------------------------------------------

int BP_ReferThreadStatus(int tid, struct ThreadParam * parm)
{
   if( parm )
   {
      BP_TODO_BREAK;
   }

   SBPThread* pThread = gThreads + tid;
   if( pThread->mFiber != NULL )
      return 1;
   return 0;
}

//----------------------------------------------------------------------------

int BP_SleepThread(void)
{
   int tid = BP_GetThreadId();
   
   if( tid != -1 )
   {
      SBPThread* pThread = gThreads + tid;

      pThread->mParm.wakeupCount--;

      // just put to sleep?
      if( pThread->mParm.wakeupCount == -1)
      {
         bp_remove_from_ready_queue( pThread );

         pThread->mParm.status = THS_WAIT;
         pThread->mParm.waitType = WAIT_TYPE_WAKEUP;
         bp_reschedule_threads();
      }
   }

   return tid;
}

//----------------------------------------------------------------------------

int BP_WakeupThread(int tid)
{
   if( tid != -1 )
   {
      SBPThread* pThread = gThreads + tid;
      pThread->mParm.wakeupCount++;

      if( pThread->mParm.status == THS_WAIT && pThread->mParm.waitType == WAIT_TYPE_WAKEUP )
      {
         BPE_ASSERT_NO_MSG( pThread->mpNextThreadInQueue == NULL );
         // Just became awake?
         if( pThread->mParm.wakeupCount >= 0 )
         {
            if( pThread == gCurrentThread )
               BP_BREAK;

            // Make woken up thread ready to run.
            pThread->mParm.status = THS_READY;
            pThread->mParm.waitType = WAIT_TYPE_NOT;
            pThread->mParm.waitId = 0;

            bp_add_to_ready_queue( pThread );

            // switch current into ready state from running.
            int currentTid = BP_GetThreadId();
            SBPThread* pCurrentThread = gThreads + currentTid;
            if( pCurrentThread->mParm.status == THS_RUN )
            {
               pCurrentThread->mParm.status = THS_READY;
            }

            bp_reschedule_threads();
         }
      }
   }

   return tid;
}

//----------------------------------------------------------------------------

int BP_iWakeupThread(int tid)
{
   return BP_WakeupThread(tid);
}

//----------------------------------------------------------------------------

int BP_CancelWakeupThread(int tid)
{
// Description
// Reads and clears the wakeup request count for the specified thread, canceling all wakeup requests.
// Return value
// If termination is normal, returns the wakeup request count.
// If there is an error, returns -1.

   SBPThread* pThread = gThreads + tid;

   int const prevWakeupCount = pThread->mParm.wakeupCount;
   pThread->mParm.wakeupCount = 0;

   return prevWakeupCount;
}

//----------------------------------------------------------------------------

int BP_DelayThread( u_int usec )
{
   int tid = BP_GetThreadId();

#ifdef BP_ICO
   if( tid != 6 ) //iosCdvdManager
#else
   if( tid != 9
      && tid != 6
      && tid != 11 //mcManager
      )

#endif
   {
      //Make super careful that we don't accidentally call this from a thread
      //that isn't supposed to be put to sleep in this manner (such as performing
      //a nonblocking file read from iScheduler)
      BP_BREAK;
   }

   if( tid != -1 )
   {
      SBPThread* pThread = gThreads + tid;
      if( pThread->mDelayThreadWakeupTime != 0 )
         BP_BREAK;

      const uint64 nowMicros = CStopWatch::gGlobalTime.GetElapsedMicroSeconds();
      pThread->mDelayThreadWakeupTime = nowMicros + usec;

      //Put to sleep.
      if( pThread->mParm.wakeupCount != 0 )
         BP_BREAK;   //only expect threads to put *themselves* to sleep.

      bp_remove_from_ready_queue( pThread );

      pThread->mParm.wakeupCount--;
      pThread->mParm.status = THS_WAIT;
      pThread->mParm.waitType = WAIT_TYPE_WAKEUP;
      bp_reschedule_threads( false );  //don't check for delayed thread wakeup.

      return tid;
   }
   else
   {
      BP_BREAK;
   }

   return 0;
}

//----------------------------------------------------------------------------
// Semaphore
//----------------------------------------------------------------------------

int bp_find_sema_slot()
{
   for( int i = 0; i < PS2_MAX_SEMAPHORES; ++i )
   {
      if( !gSemaphores[i].mValid )
         return i;
   }

   return -1;
}

//----------------------------------------------------------------------------

void bp_free_sema_slot(int sid)
{
   BPE_ASSERT_NO_MSG(gSemaphores[sid].mValid!=0);

   gSemaphores[sid].mValid = NULL;
}

//----------------------------------------------------------------------------
// Annotate debug information on thread.
void bp_debug_sema__annotate_sema_name(int sid, char const * pName)
{
   SBPSemaphore* pSema = gSemaphores + sid;
   pSema->mDebug.mpSemaName = pName;
}


//----------------------------------------------------------------------------

int CreateSema(struct SemaParam * parm, char const *pBP_DebugName)
{
   int sid = bp_find_sema_slot();
   
   if( sid != -1 )
   {
      SBPSemaphore* pSema = gSemaphores + sid;
      pSema->mValid = 1;
      pSema->mParam = *parm;
      pSema->mParam.currentCount = pSema->mParam.initCount;
      pSema->mParam.numWaitThreads = 0;
      pSema->mParam.attr = 0;
      pSema->mParam.option = 0;

      //Debug
      pSema->mDebug.mpSemaName = pBP_DebugName;
   }

   return sid;
}

//----------------------------------------------------------------------------

int DeleteSema(int sid)
{
   bp_free_sema_slot(sid);
   return sid;
}

//----------------------------------------------------------------------------

int SignalSema(int sid)
{
   if( sid != -1 )
   {
      SBPSemaphore* pSema = gSemaphores + sid;

      if( pSema->mValid )
      {
         pSema->mParam.currentCount++;

         //if( sid == 0 )
         {
            threadPrintF("Thd SIGNAL: %2d (%s) signaling semaphore %d (%s) - cnt: %d\n", 
               BP_GetThreadId(), gThreads[BP_GetThreadId()].mDebug.mpThreadName, sid, pSema->mDebug.mpSemaName, pSema->mParam.currentCount);
         }

         if( pSema->mParam.numWaitThreads > 0 )
         {
            int threadToWakeUp = pSema->mWaitQueue[0];

            memmove(pSema->mWaitQueue, pSema->mWaitQueue + 1, sizeof(int) * pSema->mParam.numWaitThreads - 1);
            pSema->mParam.numWaitThreads--;

            SBPThread* pThread = gThreads + threadToWakeUp;


            if( pThread->mParm.status != THS_WAIT || pThread->mParm.waitType != WAIT_TYPE_SEMAPHORE || pThread->mParm.waitId != sid )
               BP_BREAK;

            //if( sid == 0 )
            {
               threadPrintF("Thd WAKEUP: %2d (%s) waiting on semaphore %d (%s) woke up...\n", 
                  threadToWakeUp, gThreads[threadToWakeUp].mDebug.mpThreadName, sid, pSema->mDebug.mpSemaName);
            }

            pThread->mParm.status = THS_READY;
            pThread->mParm.waitType = WAIT_TYPE_NOT;
            pThread->mParm.waitId = 0;

            bp_add_to_ready_queue( pThread );

            // Should have a current thread and current thread should be running
            if( gCurrentThread == NULL || gCurrentThread->mParm.status != THS_RUN)
               BP_BREAK;
            
            // Current thread is no longer running (but is ready to run if it needs to!)
            gCurrentThread->mParm.status = THS_READY;

            bp_reschedule_threads();
         }
      }
   }

   return sid;
}

//----------------------------------------------------------------------------

int WaitSema(int sid)
{
   if( sid != -1 )
   {
      SBPSemaphore* pSema = gSemaphores + sid;
      
      if( pSema->mValid )
      {
         int currentTid = BP_GetThreadId();

         pSema->mParam.currentCount--;

         //if( sid == 0 )
         {
            threadPrintF("Thd WAITIN: %2d (%s) waiting on semaphore %d (%s) - cnt: %d\n", 
               currentTid, gThreads[currentTid].mDebug.mpThreadName, sid, pSema->mDebug.mpSemaName, pSema->mParam.currentCount);
         }

         if( pSema->mParam.currentCount < 0 )
         {
            int i = 0;
            
            for( ; i < pSema->mParam.numWaitThreads; ++i )
            {
               if( pSema->mWaitQueue[i] == currentTid )
               {
                  break;
               }
            }

            // Only add to waiting queue if we're not already waiting for this semaphore.
            if( i == pSema->mParam.numWaitThreads )
            {
               pSema->mWaitQueue[pSema->mParam.numWaitThreads] = currentTid;
               pSema->mParam.numWaitThreads++;

               SBPThread* pThread = gThreads + currentTid;

               pThread->mParm.status = THS_WAIT;
               pThread->mParm.waitType = WAIT_TYPE_SEMAPHORE;
               pThread->mParm.waitId = sid;

               bp_remove_from_ready_queue( pThread );

               bp_reschedule_threads();
            }
            else
            {
               return sid;
            }
         }
      }
   }

   return sid;
}

//
// Gets the status of the specified semaphore.
//
// Return value
// If termination is normal,returns the semaphore ID.
// If there is an error, returns -1.
//

//----------------------------------------------------------------------------

int ReferSemaStatus(int sid, struct SemaParam * pSceSema_out)
{
   if( sid == -1 )
   {
      return -1;
   }
   SBPSemaphore* pSema = gSemaphores + sid;
   if( !pSema->mValid )
   {
      return -1;
   }
   // Return status of semaphore
   *pSceSema_out = pSema->mParam;

   return sid;   
}

//----------------------------------------------------------------------------
// Interrupts
//----------------------------------------------------------------------------

int EIntr(void)
{
   return 0;
}

//----------------------------------------------------------------------------

int DIntr(void)
{
   return 0;
}

//----------------------------------------------------------------------------
// Misc other SCE functions
//----------------------------------------------------------------------------
// From eekernel.h
//
// Enable interrupts from 'cause' / device
// INTC_VBLANK_S  - V-blank start
// INTC_GS - GS
//  etc.
//
// Returns 1 if already enabled, 0 if not currently enabled.
//
// Looks like it's only used for the vblank on ICO
//

int EnableIntc(int cause)
{
   return 1;
}

//----------------------------------------------------------------------------
// From eekernel.h
//
// Adds interrupt handler function for 'cause' / device
// INTC_VBLANK_S  - V-blank start
// INTC_GS - GS
//  etc.
//
// Returns handler ID (or -1 for error)
//
// Looks like it's only used for the vblank on ICO
//

int AddIntcHandler(int cause, int (*handler)(int ca), int next)
{
   return 0;   
}

//----------------------------------------------------------------------------
// From libgraph.h
//
// Wait for vblank
//
// 'mode' should always be zero.
//
// Return value
//  0 - Even field
//  1 - Odd field (1 is always returned for non-interlaced mode)
//
int sceGsSyncV(int mode)
{
   return 1;   // progressive
}

//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif
