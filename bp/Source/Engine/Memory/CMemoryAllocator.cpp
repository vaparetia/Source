//----------------------------------------------------------------------------
// CMemoryAllocator.cpp
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CMemoryAllocator.h"
#include "Engine/Basics/CStringExtras.h"

#if BPE_TARGET == BPE_TARGET_PS3
#include "sys/memory.h"
#include <stdlib.h>
#elif BPE_TARGET == BPE_TARGET_X360
#include "xtl.h"
#endif

//----------------------------------------------------------------------------

SMemoryStats sMemoryStats;

//----------------------------------------------------------------------------

SMemoryStats const & CMemoryAllocator::GetMemoryStatistics(EMemoryStats const memoryStats)
{
   if (memoryStats == kMS_None)
   {
      return sMemoryStats;
   }

#if BPE_TARGET == BPE_TARGET_PS3

   sys_memory_info_t memInfo = { 0 };
   sys_memory_get_user_memory_size(&memInfo);

   sMemoryStats.mMaxMemoryAllocated = bpe::max_val(sMemoryStats.mMaxMemoryAllocated, memInfo.total_user_memory);
   sMemoryStats.mLeastMemoryFree = bpe::min_val(sMemoryStats.mLeastMemoryFree, memInfo.available_user_memory);

   sMemoryStats.mMemoryAllocated = memInfo.total_user_memory;
   sMemoryStats.mMemoryFree = memInfo.available_user_memory;
   sMemoryStats.mMemInfoString.clear();

   if (memoryStats == kMS_Detailed)
   {
      std::malloc_managed_size memStats;
      malloc_stats(&memStats);

      real64 const kOneMeg = 1024.0 * 1024.0;
      sMemoryStats.mMemInfoString = CStringExtras::Stringize("malloc_stats: %0.2f current_inuse_size / %0.2f current_system_size / %0.2f max_system_size.\n",
         memStats.current_inuse_size / kOneMeg,
         memStats.current_system_size / kOneMeg, 
         memStats.max_system_size / kOneMeg);
   }

#elif BPE_TARGET == BPE_TARGET_X360
   MEMORYSTATUS memStat; 
   memStat.dwLength = sizeof(memStat); 
   GlobalMemoryStatus(&memStat);

   size_t memoryAllocated = memStat.dwTotalPhys - memStat.dwAvailPhys;
   sMemoryStats.mMaxMemoryAllocated = bpe::max_val(sMemoryStats.mMaxMemoryAllocated, memoryAllocated);
   sMemoryStats.mLeastMemoryFree = bpe::min_val(sMemoryStats.mLeastMemoryFree, (size_t)memStat.dwAvailPhys);
   
   sMemoryStats.mMemoryAllocated = memoryAllocated;
   sMemoryStats.mMemoryFree = memStat.dwAvailPhys;

   

   sMemoryStats.mMemInfoString.clear();

   if (memoryStats == kMS_Detailed)
   {
      real64 const kOneMeg = 1024.0f * 1024.0f;

      sMemoryStats.mMemInfoString = CStringExtras::Stringize("malloc: %0.2f allocated / %0.2f available / %0.2f total.\n %0.2f Max Allocated / %0.2f Least Free.\n",
         (memStat.dwTotalPhys - memStat.dwAvailPhys) / kOneMeg,
         memStat.dwAvailPhys / kOneMeg, 
         memStat.dwTotalPhys / kOneMeg,
         sMemoryStats.mMaxMemoryAllocated / kOneMeg,
         sMemoryStats.mLeastMemoryFree / kOneMeg);
   }

#endif

   return sMemoryStats;
}

//----------------------------------------------------------------------------

void CMemoryAllocator::ResetFrameCounters()
{
   sMemoryStats.mAllocationsThisFrame = 0;
}

//----------------------------------------------------------------------------

static CMemoryAllocator::EAllocTraceMode sAllocTraceMode = CMemoryAllocator::kAT_Off;
static size_t sAllocTraceMinSize = 0x0;
static size_t sAllocTraceMaxSize = 0xFFFFFFFF;
static bool sbAllocHookEnabled = false;
static bool sbAllocHooked = false;

//----------------------------------------------------------------------------

CMemoryAllocator::EAllocTraceMode CMemoryAllocator::GetAllocTraceMode()
{
   return sAllocTraceMode;
}


//----------------------------------------------------------------------------
#if BPE_TARGET != BPE_TARGET_WIN32

void CMemoryAllocator::SetAllocTraceMode(EAllocTraceMode const mode)
{
   sAllocTraceMode = mode;
}

#endif
//----------------------------------------------------------------------------

#if BPE_TARGET == BPE_TARGET_WIN32

//----------------------------------------------------------------------------

#include "boost/scoped_ptr.hpp"
#include <windows.h>
#include "ExtLibraries/Detours/detours.h"
#include "Win32StackWalker.h"

//----------------------------------------------------------------------------

static DWORD sTraceThread = 0;

void CMemoryAllocator::SetAllocTraceMode(EAllocTraceMode const mode)
{
   // Only trace allocations on the thread that set the trace
   sTraceThread = GetCurrentThreadId();
   sAllocTraceMode = mode;
}

//----------------------------------------------------------------------------

// Simple implementation of an additional output to the console:
class MyStackWalker : public StackWalker
{
public:
  MyStackWalker(int options) : StackWalker(options) {}
  MyStackWalker(DWORD dwProcessId, HANDLE hProcess) : StackWalker(dwProcessId, hProcess) {}

  virtual void OnOutput(LPCSTR szText)
  {
     mStackOutput += std::string(szText);
  }

  virtual void OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr)
  {
     // Don't care about error spam
     (szFuncName); (gle); (addr);
  }

  std::string  mStackOutput;
};

// Keep a static if we're using this because creating the stackwalker is slow when it loads symbols
static boost::scoped_ptr<MyStackWalker>   spStackWalker;

//----------------------------------------------------------------------------

extern "C" {
   DETOUR_TRAMPOLINE(LPVOID WINAPI Real_HeapAlloc(HANDLE Heap, DWORD Flags, size_t s), HeapAlloc);
   DETOUR_TRAMPOLINE(BOOL WINAPI Real_HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem), HeapFree);
}

//----------------------------------------------------------------------------

CRITICAL_SECTION sAllocSection;

//----------------------------------------------------------------------------

MyStackWalker * GetStackWalker()
{
    if (!spStackWalker)
    {
       int options = StackWalker::RetrieveLine;
       //LPCSTR szSymPath = NULL; 
       //DWORD dwProcessId = GetCurrentProcessId();
       //HANDLE hProcess = GetCurrentProcess();

       spStackWalker.reset(new MyStackWalker(options));
    }
    return spStackWalker.get();
}


//----------------------------------------------------------------------------
// Make sure you've got the windbg debugger as this has the latest dbghelp.dll
// Also, this code doesn't seem to work correctly when remote debugging, so watch out
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/tools/tools/windbg_debugger.asp

void DumpAllocInfo(char * pDescription, void *pAddress, uint32 const size)
{
   if (sTraceThread != GetCurrentThreadId()) return;

   EnterCriticalSection(&sAllocSection);

   static bool sInsideAlloc = false;
   if (!sInsideAlloc)
   {
      sInsideAlloc = true;

      MyStackWalker *pStackWalker = GetStackWalker();
      pStackWalker->ShowCallstack();
      // Ignore sound allocations
      if ((pStackWalker->mStackOutput.find("sound") == -1) && (pStackWalker->mStackOutput.find("Sound") == -1))
      {
         CHAR buffer[1024];
         _snprintf_s(buffer, 1024, "%s%x/%d\n", pDescription, pAddress, size);
         OutputDebugStringA(buffer);
         OutputDebugStringA(pStackWalker->mStackOutput.c_str());
      }
      pStackWalker->mStackOutput.clear();

      sInsideAlloc = false;
   }

   LeaveCriticalSection(&sAllocSection);
}

//----------------------------------------------------------------------------

LPVOID WINAPI My_HeapAlloc(HANDLE hHeap, DWORD dwFlags, size_t s)
{
   bool const serialize = (dwFlags & HEAP_NO_SERIALIZE) == 0;
   
   if( serialize )
      EnterCriticalSection(&sAllocSection);
   
   sMemoryStats.mMemoryAllocated += s;
   sMemoryStats.mNumAllocations++;
   sMemoryStats.mAllocationsThisFrame++;
   
   if( serialize )
      LeaveCriticalSection(&sAllocSection);

   LPVOID pPtr = Real_HeapAlloc( hHeap, dwFlags, s );

   if ((sAllocTraceMode == CMemoryAllocator::kAT_StackTrace) && (s >= sAllocTraceMinSize) && (s <= sAllocTraceMaxSize))
   {
      EnterCriticalSection(&sAllocSection);
      DumpAllocInfo("Alloc:", pPtr, s);
      LeaveCriticalSection(&sAllocSection);
   }

   return pPtr;
}

//----------------------------------------------------------------------------

BOOL WINAPI My_HeapFree( HANDLE hHeap, DWORD dwFlags, LPVOID lpMem )
{
   if( lpMem )
   {
      bool const serialize = (dwFlags & HEAP_NO_SERIALIZE) == 0;

      if( serialize )
         EnterCriticalSection(&sAllocSection);

      sMemoryStats.mMemoryAllocated -= (size_t)HeapSize(hHeap, 0, lpMem);
      sMemoryStats.mNumAllocations--;

      if( serialize )
         LeaveCriticalSection(&sAllocSection);
   }

   return Real_HeapFree( hHeap, dwFlags, lpMem );
}

//----------------------------------------------------------------------------

#define HOOK_ALLOCATIONS

//----------------------------------------------------------------------------

void Detour()
{
   InitializeCriticalSection(&sAllocSection);

#ifdef HOOK_ALLOCATIONS
   PBYTE pbRealTrampoline;
   PBYTE pbRealTarget;
   // Don't hook allocations when debugger is attached as this causes intermittent crashes of unknown origin.
   if (!IsDebuggerPresent() && sbAllocHookEnabled)
   {
      sbAllocHooked = true;
      DetourFunctionWithTrampolineEx((PBYTE)Real_HeapAlloc, (PBYTE)My_HeapAlloc, &pbRealTrampoline, &pbRealTarget);
      DetourFunctionWithTrampolineEx((PBYTE)Real_HeapFree, (PBYTE)My_HeapFree, &pbRealTrampoline, &pbRealTarget);
   }
#endif
}

//----------------------------------------------------------------------------

void UnDetour()
{
#ifdef HOOK_ALLOCATIONS
   if (sbAllocHooked)
   {
      DetourRemove((PBYTE)Real_HeapAlloc, (PBYTE)My_HeapAlloc);
      DetourRemove((PBYTE)Real_HeapFree, (PBYTE)My_HeapFree);
   }
#endif
}

//----------------------------------------------------------------------------

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
   switch( ul_reason_for_call )
   {
   case DLL_PROCESS_ATTACH:
      // Only hook allocations if running within 'Game'.
      sbAllocHookEnabled = _stricmp(::GetCommandLine(), "Game.exe") == 0;
      sbAllocHookEnabled = _stricmp(::GetCommandLine(), "Game_Develop.exe") == 0;
      sbAllocHookEnabled = _stricmp(::GetCommandLine(), "Game_Debug.exe") == 0;
      Detour();
      break;
   case DLL_THREAD_ATTACH:
   case DLL_THREAD_DETACH:
      break;
   case DLL_PROCESS_DETACH:
      UnDetour();
      break;
   }
   return TRUE;
}

//----------------------------------------------------------------------------
#endif // WIN32
