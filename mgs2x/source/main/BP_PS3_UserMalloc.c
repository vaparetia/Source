/*
 *   SCE CONFIDENTIAL
 *   PlayStation(R)3 Programmer Tool Runtime Library 360.001 
 *   Copyright (C) 2008 Sony Computer Entertainment Inc.
 *   All Rights Reserved. 
 */

/*E
 * File: user_malloc.c
 * Description:
 *   A simple example to replace malloc(). This sample shows an implementation of
 *   functions which are related to malloc() by using mspace_malloc() series. Because
 *   of the specification of mspace_malloc(), the heap area for these malloc() series 
 *   can not be enlarged after creating the area.
 *   You can also implement your own malloc() here, instead of these ones.
 *
 *   To replace malloc() series, you should redefine all of the functions below.
 *
 *     void   _malloc_init(void)
 *     void   _malloc_finalize(void)
 *     void*  malloc(size_t size)
 *     void   free(void *ptr)
 *     void*  calloc(size_t nelem, size_t size)
 *     void*  realloc(void *ptr, size_t size)
 *     void*  memalign(size_t boundary, size_t size)
 *     void*  reallocalign(void *ptr, size_t size, size_t boundary)
 *     int    malloc_stats(struct malloc_managed_size *mmsize)
 *     size_t malloc_usable_size(void *ptr)
 *
 *   Notice that _malloc_init() is called at the beginning of the program, and 
 *   _malloc_finalize() is called at the end of the program. Therefore, no global
 *   constructor is run before executing _malloc_init() and no global destructor
 *   (also the function which is registered by atexit()) is run after executing
 *   _malloc_finalize(). Furthermore, no stream can be accessed in these two functions,
 *   i.e., printf() is not available in these functions.
 */

#include <stdlib.h>
#include <mspace.h>
#include <sys/memory.h>
#include <sys/synchronization.h>

#define UM_DO_LIST 1
#define UM_WALK_FREE 0
#define UM_WALK_ALLOC 0
#define UM_FREE_WIPE 1

#define HEAP_SIZE (1024 * 1024 * 180)
static sys_addr_t heap_area;
static mspace global_mspace;

struct _SMemoryHeader;

struct _SMemoryHeader
{
   void *base;
#if UM_DO_LIST
   struct _SMemoryHeader *mpPrev;
   struct _SMemoryHeader *mpNext;
#endif
   int external_size;
   int internal_size;
   int alloc_number;
   unsigned int cookie;
};

typedef struct _SMemoryHeader SMemoryHeader;

typedef struct _SMemoryAllocationHelper
{
   int mAlign;
   int mHeaderSize;
   int mInternalSize;
   int mExternalSize;
} SMemoryAllocationParams;

static unsigned skGuardCookie = 0xd789e123;
static unsigned char skGuardByte = 0x1D;
static unsigned char skClearByte = 0xF1;
static int sAllocNumber = 0;
static int const skMinGuardBegin = 64 - sizeof( SMemoryHeader );
static int const skMinGuardEnd = 32;
static char *spMemoryHeapBegin = NULL;
static char *spMemoryHeapEnd = NULL;
static sys_lwmutex_t sMutex;
extern int gForcePS3MallocWalk;
int gForcePS3MallocWalk = 0;

#if UM_DO_LIST
static SMemoryHeader *spListHead = NULL;
#endif

static void *return_and_validate_alloc( void *alloced );

static int is_memory_in_heap( void *mem )
{
   if ( (char *) mem >= spMemoryHeapBegin && (char *) mem < spMemoryHeapEnd )
   {
      return 1;
   }
   else
   {
      return 0;
   }
}

static void mem_lock_mutex()
{
   int ret = sys_lwmutex_lock( &sMutex, 0 );

   if ( CELL_OK != ret )
   {
      printf( "Error locking memory mutex: %d %8.8x\n", ret, ret );

      abort();
   }
}

static void mem_unlock_mutex()
{
   int ret = sys_lwmutex_unlock( &sMutex );

   if ( CELL_OK != ret )
   {
      printf( "Error unlocking memory mutex: %d %8.8x\n", ret, ret );

      abort();
   }
}

#if UM_DO_LIST

static void list_add_header( SMemoryHeader *pItem )
{
   mem_lock_mutex();

   pItem->mpPrev = NULL;
   pItem->mpNext = spListHead;

   if ( spListHead )
   {
      spListHead->mpPrev = pItem;
   }

   spListHead = pItem;

   mem_unlock_mutex();
}

static void list_remove_header( SMemoryHeader *pItem )
{
   mem_lock_mutex();

   if ( spListHead == pItem )
   {
      spListHead = pItem->mpNext;
   }

   if ( pItem->mpPrev )
   {
      pItem->mpPrev->mpNext = pItem->mpNext;
   }

   if ( pItem->mpNext )
   {
      pItem->mpNext->mpPrev = pItem->mpPrev;
   }

   mem_unlock_mutex();
}

static void list_walk_headers()
{
   SMemoryHeader *current;

   mem_lock_mutex();

   for ( current = spListHead; current; current = current->mpNext )
   {
      return_and_validate_alloc( current + 1 ); // memory alloc is right after header
   }

   mem_unlock_mutex();
}

#endif

static void init_alloc_params( SMemoryAllocationParams *pParamsOut, int const align, int const size, int const minGuardBegin, int const minGuardEnd )
{
   int const headerAndGuardSize = minGuardBegin + sizeof( SMemoryHeader );
   int const alignedHeaderAndGuardSize = ( headerAndGuardSize + align - 1 ) & ( ~( align - 1 ) );

   pParamsOut->mAlign = align;
   pParamsOut->mInternalSize = size;
   pParamsOut->mHeaderSize = alignedHeaderAndGuardSize;
   pParamsOut->mExternalSize = 
      alignedHeaderAndGuardSize + // beginning
      size +                      // actual memory
      minGuardEnd;                   // end guard
}

static void *precondition_memory( void *alloced, SMemoryAllocationParams const *pParams )
{
   char *alloced_char = (char *) alloced;

#if UM_WALK_ALLOC
   list_walk_headers();
#else
   if ( gForcePS3MallocWalk )
   {
      list_walk_headers();
   }
#endif

   memset( alloced_char, skGuardByte, pParams->mExternalSize );

   SMemoryHeader *pHeader = (SMemoryHeader *) ( alloced_char + pParams->mHeaderSize - sizeof( SMemoryHeader ) );
   pHeader->base = alloced;
   pHeader->external_size = pParams->mExternalSize;
   pHeader->internal_size = pParams->mInternalSize;
   pHeader->alloc_number = sAllocNumber++;
   pHeader->cookie = skGuardCookie;
#if UM_DO_LIST
   list_add_header( pHeader );
#endif
   return alloced_char + pParams->mHeaderSize;
}

static void __attribute__((noinline)) do_abort()
{
   abort();
}

static void memory_error( SMemoryHeader *pHeader, void *alloced )
{
   printf( "MEMORY ERROR!  Details above....\n" );
   printf( "Alloc at: %8.8x, alloc_num? %d\n", alloced, pHeader->alloc_number );
   printf( "Internal size? %d External size? %d\n", pHeader->internal_size, pHeader->external_size);
   do_abort();
}

static SMemoryHeader const *memory_header_from_alloc( void *alloced )
{
   return ((SMemoryHeader *) alloced ) - 1;
}

static void *return_and_validate_alloc( void *alloced )
{
   char *alloced_char = (char *) alloced;
   int had_error = 0;
   // First let's get the memory header and make sure it's ok
   SMemoryHeader *pHeader = (SMemoryHeader *) ( alloced_char - sizeof( SMemoryHeader ) );

   if ( !is_memory_in_heap( alloced ) )
   {
      printf( "Can't work with memory %8.8x\n", alloced );
      memory_error( pHeader, alloced );
   }

   if ( pHeader->cookie != skGuardCookie )
   {
      printf( "Guard cookie was %8.8x, wanted %8.8x\n", pHeader->cookie, skGuardCookie );
      had_error = 1;
   }

   if ( pHeader->alloc_number >= sAllocNumber )
   {
      printf( "Alloc num %d too big (max is %d)\n", pHeader->alloc_number, sAllocNumber );
      had_error = 1;
   }

   if ( !is_memory_in_heap( pHeader->base ) )
   {
      printf( "Base ptr %8.8x not in heap %8.8x-%8.8x\n", pHeader->base, spMemoryHeapBegin, spMemoryHeapEnd );
   }

#if UM_DO_LIST
   if ( pHeader->mpNext && !is_memory_in_heap( pHeader->mpNext ) )
   {
      printf( "Next ptr %8.8x not in heap %8.8x-%8.8x\n", pHeader->mpNext, spMemoryHeapBegin, spMemoryHeapEnd );
   }

   if ( pHeader->mpPrev && !is_memory_in_heap( pHeader->mpPrev ) )
   {
      printf( "Prev ptr %8.8x not in heap %8.8x-%8.8x\n", pHeader->mpPrev, spMemoryHeapBegin, spMemoryHeapEnd );
   }
#endif

   if ( had_error )
   {
      memory_error( pHeader, alloced );
   }

   // Check the guard blocks
   {
      unsigned char *begin = (unsigned char *) pHeader->base;
      unsigned char *end = (unsigned char *) pHeader;
      unsigned char *at;

      for ( at = begin; at < end; ++at )
      {
         if ( *at != skGuardByte )
         {
            printf( "Guard begin failed at %8.8x (offset %d)\n", at, at - begin );
            memory_error( pHeader, alloced );
         }
      }
   }

   {
      unsigned char *begin = (unsigned char *) alloced_char + pHeader->internal_size;
      unsigned char *end = ( (unsigned char *) pHeader->base ) + pHeader->external_size;
      unsigned char *at;

      for ( at = begin; at < end; ++at )
      {
         if ( *at != skGuardByte )
         {
            printf( "Guard end failed at %8.8x (offset %d)\n", at, at - begin );
            memory_error( pHeader, alloced );
         }
      }
   }

   return pHeader->base;
}

/*E Function prototypes */
void _malloc_init(void);
void _malloc_finalize(void);

/*E 
 * void _malloc_init()
 * Initialize function for malloc series. This function is called at the beginning
 * of the program, before executing `main()' and also all of the global constructors.
 * Notice that there are some limitation below for this function.
 *   - CANNOT access any streams (i.e., no printf(), no fprintf(), etc..).
 *   - CANNOT access any global objects which need the constructor to initialize.
 */
void _malloc_init()
{
   sys_lwmutex_attribute_t attrib;
   sys_lwmutex_attribute_initialize(attrib);

   attrib.attr_recursive = SYS_SYNC_RECURSIVE;

   if ( CELL_OK != sys_lwmutex_create( &sMutex, &attrib ) )
   {
      abort();
   }

	/*E Allocate a heap area from the system */
	if (CELL_OK != sys_memory_allocate(HEAP_SIZE,
									   SYS_MEMORY_PAGE_SIZE_1M,
									   (sys_addr_t *)(&heap_area))) {
		/*E Fail to get a memory block from the system */
		abort();
	}

	/*E Create a heap area */
	global_mspace = mspace_create((void *)heap_area, HEAP_SIZE);

   spMemoryHeapBegin = (char *) heap_area;
   spMemoryHeapEnd = spMemoryHeapBegin + HEAP_SIZE;
}

/*E 
 * void _malloc_finalize()
 * Finalize function for malloc series. This function is called at the end
 * of the program, after executing `main()' and also all of the global destructors.
 * Notice that there are some limitation below for this function.
 *   - CANNOT access any streams (i.e., no printf(), no fprintf(), etc..).
 *   - CANNOT access any global objects which need the destructor to finalize.
 */
void _malloc_finalize() 
{
	/*E Destroy a heap area */
	mspace_destroy(global_mspace);

	/*E Return a memory block for heap area to the system */
	sys_memory_free(heap_area);

   {
      int ret = EBUSY;

      while ( ret == EBUSY )
      {
         ret = sys_lwmutex_destroy( &sMutex );
      }
   }
}

/*E 
 * void *malloc(size_t size)
 * User defined malloc().
 */
void *malloc(size_t size) 
{
	/*E Just call mspace_malloc() which uses the global mspace. */
   SMemoryAllocationParams params;
   init_alloc_params( &params, ( size > 1024 ) ? 32 : 4, size, skMinGuardBegin, skMinGuardEnd );
   return precondition_memory( mspace_malloc( global_mspace, params.mExternalSize ), &params );
}

/*E 
 * void free(void *ptr)
 * User defined free().
 */
void free(void *ptr) 
{
	/*E Just call mspace_free() which uses the global mspace. */
   if ( ptr )
   {
      void *real_alloc_ptr = return_and_validate_alloc( ptr );
      SMemoryHeader *pHeader = ((SMemoryHeader *) ptr ) - 1;
      int const real_alloc_size = pHeader->external_size;

#if UM_DO_LIST
      list_remove_header( pHeader );

#  if UM_WALK_FREE
      list_walk_headers();
#  else
      if ( gForcePS3MallocWalk )
      {
         list_walk_headers();
      }
#  endif

#endif

#if UM_FREE_WIPE
      memset( real_alloc_ptr, skClearByte, real_alloc_size );
#endif

      return mspace_free(global_mspace, real_alloc_ptr );
   }
}

/*E 
 * void *calloc(size_t nelem, size_t size)
 * User defined calloc().
 */
void *calloc(size_t nelem, size_t size) 
{
	/*E Just call mspace_calloc() which uses the global mspace. */
   size_t const totalSize = nelem * size;
   void *ptr = malloc( totalSize );
   memset( ptr, 0, totalSize );

   return ptr;
}

/*E 
 * void *realloc(void *ptr, size_t size)
 * User defined realloc().
 */
void *realloc(void *ptr, size_t size) 
{
   if ( ptr == NULL )
   {
      return malloc( size );
   }
   else if ( size == 0 )
   {
      free( ptr );
      return NULL;
   }
   else
   {
      void *original_ptr = return_and_validate_alloc( ptr );
      SMemoryHeader const *pHeader = memory_header_from_alloc( ptr );
      void *newalloc = malloc( size );
      int const copy_size = ( size < pHeader->internal_size ) ? size : pHeader->internal_size;

      if ( copy_size > 0 )
      {
         memcpy( newalloc, ptr, copy_size );
      }

      free( ptr );

      return newalloc;
   }
}

/*E 
 * void *memalign(size_t boundary, size_t size)
 * User defined memalign().
 */
void *memalign(size_t boundary, size_t size) 
{
	/*E Just call mspace_memalign() which uses the global mspace. */
   SMemoryAllocationParams params;
   init_alloc_params( &params, boundary, size, skMinGuardBegin, skMinGuardEnd );
   return precondition_memory( mspace_memalign( global_mspace, boundary, params.mExternalSize ), &params );
}

/*E 
 * void *reallocalign(void *ptr, size_t size, size_t boundary)
 * User defined reallocalign.
 */
void *reallocalign(void *ptr, size_t size, size_t boundary) 
{
	/*E Just call mspace_reallocalign() which uses the global mspace. */

   if ( ptr == NULL )
   {
      return memalign( boundary, size );
   }
   else if ( size == 0 )
   {
      free( ptr );
      return NULL;
   }
   else
   {
      void *original_ptr = return_and_validate_alloc( ptr );
      SMemoryHeader const *pHeader = memory_header_from_alloc( ptr );
      void *newalloc = memalign( boundary, size );
      int const copy_size = ( size < pHeader->internal_size ) ? size : pHeader->internal_size;

      if ( copy_size > 0 )
      {
         memcpy( newalloc, ptr, copy_size );
      }

      free( ptr );

      return newalloc;
   }
}

/*E 
 * int malloc_stats(struct malloc_managed_size *mmsize)
 * User defined malloc_stats().
 */
int malloc_stats(struct malloc_managed_size *mmsize) 
{
	/*E Just call mspace_malloc_stats() which uses the global mspace. */
	return mspace_malloc_stats(global_mspace, mmsize);
}

/*E 
 * size_t malloc_usable_size(void *ptr)
 * User defined malloc_usable_size().
 */
size_t malloc_usable_size(void *ptr) 
{
	/*E Just call mspace_malloc_usable_size(). */

   return_and_validate_alloc( ptr );
   SMemoryHeader *pHeader = ( (SMemoryHeader *)( ptr ) ) - 1;

   return pHeader->internal_size;
}
