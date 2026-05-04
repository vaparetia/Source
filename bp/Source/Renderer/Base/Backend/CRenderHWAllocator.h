//------------------------------------------------------------------------------------------
// CRenderHWAllocator.h
// Copyright 2008
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"


//------------------------------------------------------------------------------------------

namespace NRenderHWAllocator
{
   // Typedefs to make handle and block indices explicit.
   typedef  uint16   THandleIndex;
   typedef  uint16   TBlockIndex;

   struct SHandleData;     // Internal private handle data, aliases to SHandle
   struct SBlockEntry;
   struct SPendingFreeItem;
}

struct SRenderHWAllocatorStats
{
   // size currently allocated (pending free not included)
   int   mAllocatedSize;
   // largest size ever allocated
   int   mPeakAllocatedSize;
   char  mPeakArea[256];
   // size currently pending for free
   int   mPendingFreeSize;
   // total size of memory for allocator
   int   mTotalSize;

   // number of handles allocated (pending free not included)
   int   mAllocatedHandles;
   // number of handles pending free
   int   mPendingFreeBlocks;
   // total number of handles
   int   mTotalHandles;
};

class RENDERER_API CRenderHWAllocator
{
public:
   CRenderHWAllocator(void * pPool, uint32 const poolSize, uint32 const maxHandles, uint32 const userFlags);
   ~CRenderHWAllocator();

   struct SHandle
   {
      SHandle(): mpAddress( 0 ), mUserFlags( 0 ) {}
      uint8 *  mpAddress;
      uint32   mUserFlags;
   };

   SHandle const * const   Alloc(uint32 const size, uint32 const alignment);
   void                    PendingFree(SHandle const * const pHandle, uint32 const flushWaitCount);
   void                    ImmediateFree(SHandle const * const pHandle);
   void                    FlushPendingFree();

   bool                    IsHandleInPool(SHandle const * pHandle) const;

   void                    GetStats(SRenderHWAllocatorStats * pStats) const;

private:
   void                    ImmediateFree_Internal(NRenderHWAllocator::TBlockIndex freedBlockIndex);
   void                    AddToFreeLists(NRenderHWAllocator::SBlockEntry &entry);
   void                    RemoveFromFreeLists(NRenderHWAllocator::SBlockEntry &entry);
   void                    UpdateInFreeListsAndModifySize(NRenderHWAllocator::SBlockEntry &entry_SizeModified, uint32 const newSize);  // Modifies entry.mSize also.

public:
   uint8  *                               mpPool;
   uint32                                 mPoolSize;
   uint32                                 mFlushPendingFreeCounter;
   uint32                                 mUserFlags;

   // Handles
   uint32                                 mMaxHandles;
   uint32                                 mNumHandles;
   NRenderHWAllocator::THandleIndex *     mpUnusedHandles;        // Array of indices to free handles, max handles in size
   NRenderHWAllocator::SHandleData *      mpHandleData;           // Handle data, max handles in size, aliases to SHandle

   // Free and allocated block info
   uint32                                 mMaxBlockEntries;
   uint32                                 mNumBlockEntries;
   NRenderHWAllocator::TBlockIndex *      mpUnusedBlockEntries;   // Array of indices to free block entries, max handles in size
   NRenderHWAllocator::SBlockEntry *      mpBlockEntries;         // Bookkeeping, 2x max handles + 1
   NRenderHWAllocator::TBlockIndex        mFirstBlockEntry;

   // Free list buckets, 4 bytes to 2^31
   static const int skFreeListBucketCount = 29;
   NRenderHWAllocator::TBlockIndex        mFreeLists[skFreeListBucketCount];

   uint32                                 mNumPendingFreeBlocks;
   NRenderHWAllocator::SPendingFreeItem * mpPendingFreeItems;


   // Stat tracking
   uint32                                 mTotalAllocatedSize;
   uint32                                 mPeakAllocatedSize;
   char                                   mPeakArea[256];
};

//------------------------------------------------------------------------------------------
