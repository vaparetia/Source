//----------------------------------------------------------------------------
// CMemoryAllocator.h
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

#if BPE_TARGET == BPE_TARGET_WIN32
// WIN32 alloca
#ifndef alloca
   #define alloca(x) _alloca(x)
#endif

#elif BPE_TARGET == BPE_TARGET_RVL
#define alloca(x) __alloca(x)
#elif BPE_TARGET == BPE_TARGET_X360
extern "C" void* __cdecl _alloca(size_t);
#define alloca(x) _alloca(x)
#else
#include <alloca.h>
#endif

#if BPE_TARGET==BPE_TARGET_WIN32 || BPE_TARGET == BPE_TARGET_X360
#  define BPE_MALLOC_ALIGNED(a, s)    _aligned_malloc(s, a)
#  define BPE_FREE_ALIGNED(a)         _aligned_free(a)
#elif BPE_TARGET==BPE_TARGET_PS3 || BPE_TARGET==BPE_TARGET_VITA
#  define BPE_MALLOC_ALIGNED(a, s)    memalign(a, s)
#  define BPE_FREE_ALIGNED(a)         free(a)
#elif BPE_TARGET==BPE_TARGET_RVL

extern "C"
{
   void *bpe_rvl_alloc_aligned( size_t alignment, size_t size );
   void *bpe_rvl_alloc_aligned_mem2( size_t alignment, size_t size );
}

#  define BPE_MALLOC_ALIGNED(a, s)      bpe_rvl_alloc_aligned(a,s)
#  define BPE_MALLOC_ALIGNED_MEM2(a, s) bpe_rvl_alloc_aligned_mem2(a,s)
#  define BPE_FREE_ALIGNED(a)           free(a)
#else
#  error Unknown platform!
#endif


//----------------------------------------------------------------------------

struct SMemoryStats
{
   SMemoryStats()
      : mMemoryAllocated(0)
      , mMemoryFree(0)
      , mNumAllocations(0)
      , mAllocationsThisFrame(0)
      , mMaxMemoryAllocated(0)
#if BPE_TARGET == BPE_TARGET_X360
      , mLeastMemoryFree(512*1024*1024)
#elif BPE_TARGET == BPE_TARGET_PS3
       , mLeastMemoryFree(256*1024*1024)
#elif BPE_TARGET == BPE_TARGET_WIN32
      , mLeastMemoryFree(1000*1024*1024)
#elif BPE_TARGET == BPE_TARGET_VITA
      , mLeastMemoryFree(282*1024*1024)
#else
#error unknown platform
#endif
   {
   };

   size_t      mMemoryAllocated;
   size_t      mMemoryFree;
   int         mNumAllocations;

   int         mAllocationsThisFrame;
   std::string mMemInfoString;

   size_t      mMaxMemoryAllocated;
   size_t      mLeastMemoryFree;
};

//----------------------------------------------------------------------------

class ENGINE_API CMemoryAllocator
{
public:

   enum EMemoryStats
   {
      kMS_None,
      kMS_Fast,
      kMS_Detailed,

      kMS_Count
   };

   static SMemoryStats const & GetMemoryStatistics(EMemoryStats const memoryStats);
   static void ResetFrameCounters();

   enum EAllocTraceMode
   {
      kAT_Off,
      kAT_StackTrace,

      kAT_Count
   };

   static void             SetAllocTraceMode(EAllocTraceMode const mode);
   static EAllocTraceMode  GetAllocTraceMode();
};

//----------------------------------------------------------------------------

