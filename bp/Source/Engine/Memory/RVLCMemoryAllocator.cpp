//----------------------------------------------------------------------------
// RVLCMemoryAllocator.cpp
// Bluepoint
// Copyright 2009
//----------------------------------------------------------------------------

#include <stdafx.h>
#include "CMemoryAllocator.h"
#include <string.h>
#include <revolution/mem.h>

//#define LOG_ALLOCATIONS
//#define CHECK_HEAPS

namespace
{
   bool sMemInitialized = false;

   MEMHeapHandle sMEM1Heap;
   MEMHeapHandle sMEM2Heap;

   void init_heaps()
   {
      void *mem1ArenaLo = OSGetMEM1ArenaLo();
      void *mem1ArenaHi = OSGetMEM1ArenaHi();

      void *mem2ArenaLo = OSGetMEM2ArenaLo();
      void *mem2ArenaHi = OSGetMEM2ArenaHi();

      OSSetMEM1ArenaLo( mem1ArenaHi );
      OSSetMEM2ArenaLo( mem2ArenaHi );

      sMEM1Heap = MEMCreateExpHeap( mem1ArenaLo, (char *) mem1ArenaHi - (char *) mem1ArenaLo );
      sMEM2Heap = MEMCreateExpHeap( mem2ArenaLo, (char *) mem2ArenaHi - (char *) mem2ArenaLo );

      sMemInitialized = true;
   }

#ifdef CHECK_HEAPS

   void check_memory_before( MEMHeapHandle heap, char const *checkAssert, void *ptr, size_t size )
   {
      if ( heap != sMEM2Heap )
      {
         return;
      }
      
      bool ret = MEMCheckExpHeap( heap, MEM_HEAP_ERROR_PRINT );
      bpe_debugger_printf( "Before %d %s, MEM1? %d, ptr: %8.8x, size: %d\n", ret, checkAssert, heap == sMEM1Heap, ptr, size );
      if ( !ret )
      {
         BPE_SET_BREAKPOINT;
      }
   }

   void check_memory_after( MEMHeapHandle heap, char const *checkAssert, void *ptr, size_t size )
   {
      if ( heap != sMEM2Heap )
      {
         return;
      }

      bpe_debugger_printf( "After %s, MEM1? %d, ptr: %8.8x, size: %d\n", checkAssert, heap == sMEM1Heap, ptr, size );
   }

#else
#define check_memory_before(x,y,z,w) ((void)0)
#define check_memory_after(x,y,z,w) ((void)0)
#endif
}

extern "C"
{

   __declspec(force_export) void *	__sys_alloc(__std(size_t) blocksize )
   {
      if ( !sMemInitialized )
      {
         init_heaps();
      }

      check_memory_before( sMEM1Heap, "__sys_alloc", NULL, blocksize );
      void *mem = MEMAllocFromExpHeap( sMEM1Heap, blocksize );
      check_memory_after( sMEM1Heap, "__sys_alloc", mem, blocksize );

      if ( mem == NULL )
      {
         check_memory_before( sMEM2Heap, "__sys_alloc", NULL, blocksize );
         mem = MEMAllocFromExpHeap( sMEM2Heap, blocksize );
         check_memory_after( sMEM2Heap, "__sys_alloc", mem, blocksize );
      }

#ifdef LOG_ALLOCATIONS
      OSReport( "Alloc 0x%8.8x - %d\n", mem, blocksize );
#endif

      return mem;
   }

   __declspec(force_export) void	__sys_free(void *  block)
   {
      if ( sMemInitialized && block )
      {
         MEMHeapHandle heap = MEMFindContainHeap( block );
         check_memory_before( heap, "__sys_free", block, 0 );

#ifdef LOG_ALLOCATIONS
         OSReport( "Free 0x%8.8x\n", block );
#endif
         MEMFreeToExpHeap( heap, block );

         check_memory_after( heap, "__sys_free", block, 0 );
      }
   }

   __declspec(force_export) void *bpe_rvl_alloc_aligned( size_t alignment, size_t size )
   {
      if ( !sMemInitialized )
      {
         init_heaps();
      }

      check_memory_before( sMEM1Heap, "bpe_rvl_alloc_aligned", NULL, size );
      void *mem = MEMAllocFromExpHeapEx( sMEM1Heap, size, (int) alignment );
      check_memory_after( sMEM1Heap, "bpe_rvl_alloc_aligned", mem, size );

      return mem;
   }

   __declspec(force_export) void *bpe_rvl_alloc_aligned_mem2( size_t alignment, size_t size )
   {
      if ( !sMemInitialized )
      {
         init_heaps();
      }

      check_memory_before( sMEM2Heap, "bpe_rvl_alloc_aligned_mem2", NULL, size );
      void *mem = MEMAllocFromExpHeapEx( sMEM2Heap, size, (int) alignment );
      check_memory_after( sMEM2Heap, "bpe_rvl_alloc_aligned_mem2", mem, size );

      return mem;
   }

#if 1
   __declspec(force_export) extern void *malloc( __std(size_t) blocksize )
   {
      return __sys_alloc( blocksize );
   }

   __declspec(force_export) extern void free( void *block )
   {
      __sys_free( block );
   }

   __declspec(force_export) extern void *realloc( void *ptr, __std(size_t) size )
   {
      if ( !sMemInitialized )
      {
         init_heaps();
      }

      if ( size == 0 )
      {
         free( ptr );
         return NULL;
      }
      else if ( ptr == NULL )
      {
         return malloc( size );
      }
      else
      {
         MEMHeapHandle heap = MEMFindContainHeap( ptr );

         u32 resized = MEMResizeForMBlockExpHeap( heap, ptr, size );

         if ( resized > 0 )
         {
            return ptr;
         }
         else
         {
            // Resize failed, we need to allocate a new block and kill this one
            u32 oldAllocationSize = MEMGetSizeForMBlockExpHeap( ptr );
            u32 const toCopy = ( oldAllocationSize < size ) ? oldAllocationSize : size;
            void *pNewAlloc = malloc( size );
            memcpy( pNewAlloc, ptr, toCopy );
            free( ptr );
            return pNewAlloc;
         }
      }
   }
#endif
}
