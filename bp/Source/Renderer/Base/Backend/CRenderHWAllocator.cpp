//------------------------------------------------------------------------------------------
// CRenderHWAllocator.cpp
// Copyright 2008
//------------------------------------------------------------------------------------------

#include "StdAfx.h"

//------------------------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderBackend.h"

#include "Renderer/Base/Backend/CRenderHWAllocator.h"
#include "Engine/Memory/CMemoryAllocator.h"

#if BPE_TARGET==BPE_TARGET_PS3
#include "sys/synchronization.h"
#endif

#if BPE_TARGET==BPE_TARGET_VITA
#include <kernel.h>
#endif

//------------------------------------------------------------------------------------------

namespace NRenderHWAllocator
{

struct SHandleData
{
   // First entries must match SHandle as we alias between them internally
   uint8 *     mpAddress;   
   uint32      mUserFlags;
   TBlockIndex mBlockEntryIndex;
};

struct SBlockEntry
{
   // Block entries are linked in mOffset order
   TBlockIndex    mNextEntry;
   TBlockIndex    mPrevEntry;
   TBlockIndex    mNextSizeEntry;
   TBlockIndex    mPrevSizeEntry;
   uint32         mSize;
private:
   uint32         mOffset;

public:
   static const uint32 skOffsetFlagsMask =         0xFFFFFFFC;
   static const uint32 skOffsetFlagsAlignment =    0x00000003;
   static const uint32 skOffsetFlagsAllocated =    0x00000002;

   BPE_FORCEINLINE void SetOffset(uint32 const offset)
   {
      BPE_ASSERT((offset & (~skOffsetFlagsMask)) == 0, "Unaligned offset.");
      mOffset = offset | (mOffset & (~skOffsetFlagsMask));
   }

   BPE_FORCEINLINE void AddToOffset(uint32 const offsetDelta)
   {
      BPE_ASSERT((offsetDelta & skOffsetFlagsAlignment) == 0, "Unaligned offset delta.");
      mOffset += offsetDelta;
   }

   BPE_FORCEINLINE uint32 GetOffset()
   {
      return (mOffset & skOffsetFlagsMask);
   }

   BPE_FORCEINLINE void MarkAsAllocated()
   {
      mOffset |= skOffsetFlagsAllocated;
   }

   BPE_FORCEINLINE void MarkAsFree()
   {
      mOffset &= ~skOffsetFlagsAllocated;
   }

   BPE_FORCEINLINE bool IsAllocated()
   {
      bool const isAllocated = ((mOffset & skOffsetFlagsAllocated) != 0);
      return isAllocated;
   }
};

struct SPendingFreeItem
{
   uint32         mFlushCount;
   TBlockIndex    mBlockEntryIndex;
};

static TBlockIndex const kBlockEntryTerminator = 0xFFFF;

#if ENABLE_SEPERATE_RENDER_THREAD

#  if BPE_TARGET==BPE_TARGET_PS3
sys_lwmutex_t  gAllocatorMutex;
#  elif BPE_TARGET==BPE_TARGET_VITA
SceKernelLwMutexWork __attribute__((aligned (16))) gAllocatorMutex;
#  else
#     error
#  endif
#endif

};

//------------------------------------------------------------------------------------------

class CAllocatorMutexLock
{
public:
   CAllocatorMutexLock()
   {
#if ENABLE_SEPERATE_RENDER_THREAD
#  if BPE_TARGET==BPE_TARGET_PS3
      sys_lwmutex_lock(&NRenderHWAllocator::gAllocatorMutex, 0);
#  elif BPE_TARGET==BPE_TARGET_VITA
      sceKernelLockLwMutex( &NRenderHWAllocator::gAllocatorMutex, 1, NULL );
#  else
#     error
#  endif
#endif
   }

   ~CAllocatorMutexLock()
   {
#if ENABLE_SEPERATE_RENDER_THREAD
#  if BPE_TARGET==BPE_TARGET_PS3
      sys_lwmutex_unlock(&NRenderHWAllocator::gAllocatorMutex);
#  elif BPE_TARGET==BPE_TARGET_VITA
      sceKernelUnlockLwMutex( &NRenderHWAllocator::gAllocatorMutex, 1 );
#  else
#     error
#  endif
#endif
   }

   static void Init()
   {
#if ENABLE_SEPERATE_RENDER_THREAD
#  if BPE_TARGET==BPE_TARGET_PS3
      sys_lwmutex_attribute_t attr = { 0 };
      attr.attr_protocol = SYS_SYNC_FIFO;
      attr.attr_recursive = SYS_SYNC_RECURSIVE;
      sys_lwmutex_attribute_name_set(attr.name, "RA_Mutx");

      int ret = sys_lwmutex_create(&NRenderHWAllocator::gAllocatorMutex, &attr);
      BPE_VERIFY(ret == CELL_OK, false, "Unable to create allocator lwmutex.");
#  elif BPE_TARGET==BPE_TARGET_VITA
      BPE_CHECK_SCE( sceKernelCreateLwMutex( &NRenderHWAllocator::gAllocatorMutex, "BP HWAlloc Mutex", SCE_KERNEL_LW_MUTEX_ATTR_TH_FIFO | SCE_KERNEL_LW_MUTEX_ATTR_RECURSIVE, 0, NULL ) );
#  else
#     error
#  endif
#endif
   }
};

//------------------------------------------------------------------------------------------

using namespace NRenderHWAllocator;

//------------------------------------------------------------------------------------------
// Helper functions
static TBlockIndex _allocate_block_entry(CRenderHWAllocator * pAlloc)
{
   TBlockIndex const blockIndex = pAlloc->mpUnusedBlockEntries[0];
   pAlloc->mNumBlockEntries++;
   // Remove allocated block
   pAlloc->mpUnusedBlockEntries[0] = pAlloc->mpUnusedBlockEntries[pAlloc->mMaxBlockEntries - pAlloc->mNumBlockEntries];

   return blockIndex;
}

static void _deallocate_block_entry(CRenderHWAllocator * pAlloc, TBlockIndex const blockIndex)
{
   BPE_ASSERT(blockIndex != pAlloc->mFirstBlockEntry, "Deleting first block without update!");

   // Add entry handle
   pAlloc->mpUnusedBlockEntries[pAlloc->mMaxBlockEntries - pAlloc->mNumBlockEntries] = blockIndex;
   pAlloc->mNumBlockEntries--;

   return;
}


static THandleIndex _allocate_handle(CRenderHWAllocator * pAlloc)
{
   // Find unused handle
   THandleIndex const handleIndex = pAlloc->mpUnusedHandles[0];
   pAlloc->mNumHandles++;
   // Remove allocated handle
   pAlloc->mpUnusedHandles[0] = pAlloc->mpUnusedHandles[pAlloc->mMaxHandles - pAlloc->mNumHandles];

   return handleIndex;
}

static void _deallocate_handle(CRenderHWAllocator * pAlloc, CRenderHWAllocator::SHandle const * const pHandle)
{
   // Find unused handle
   SHandleData * pHandleData = (SHandleData *) pHandle;
   THandleIndex const handleIndex = pHandleData - pAlloc->mpHandleData;

   // Mark handle as not used
   pHandleData->mpAddress = NULL;
   pHandleData->mBlockEntryIndex = kBlockEntryTerminator;
   // Add free handle
   pAlloc->mpUnusedHandles[pAlloc->mMaxHandles - pAlloc->mNumHandles] = handleIndex;
   pAlloc->mNumHandles--;

   return;
}

static uint32 _get_bucket_entry_for_size(uint32 const size)
{
   BPE_ASSERT(size > 3, "Size too small.");

   // Get log2 value (http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog)
   uint32 v = size;	         // 32-bit value to find the log2 of 
   register uint32 r; // result of log2(v) will go here
   register uint32 shift;

   r =     (v > 0xFFFF) << 4; v >>= r;
   shift = (v > 0xFF  ) << 3; v >>= shift; r |= shift;
   shift = (v > 0xF   ) << 2; v >>= shift; r |= shift;
   shift = (v > 0x3   ) << 1; v >>= shift; r |= shift;
                                           r |= (v >> 1);
   uint32 const bucketIndex = r - 2;
   BPE_ASSERT(bucketIndex < CRenderHWAllocator::skFreeListBucketCount, "Bucket index out of range.");

   return bucketIndex;
}

//------------------------------------------------------------------------------------------

CRenderHWAllocator::CRenderHWAllocator(void * pPool, uint32 const poolSize, uint32 const maxHandles, uint32 const userFlags)
: mpPool((uint8 *)pPool)
, mPoolSize(poolSize)
, mFlushPendingFreeCounter(0)
, mUserFlags(userFlags)
, mMaxHandles(maxHandles)
, mNumHandles(0)
, mNumBlockEntries(0)
, mNumPendingFreeBlocks(0)
, mpPendingFreeItems( NULL )
, mTotalAllocatedSize(0)
, mPeakAllocatedSize( 0 )
{
   BPE_ASSERT(pPool != NULL, "NULL pPool.");
   BPE_ASSERT(poolSize >= 32, "Pool too small.");
   BPE_ASSERT(maxHandles <= 0x7FFE, "Too many handles.");

   CAllocatorMutexLock::Init();

   // Handles
   mpHandleData = (SHandleData *) BPE_MALLOC_ALIGNED(16, sizeof(SHandleData) * mMaxHandles);
   memset(mpHandleData, 0, sizeof(SHandleData) * mMaxHandles);
   for( int i = 0; i < mMaxHandles; ++i )
      mpHandleData[i].mUserFlags = userFlags;

   mpUnusedHandles = (uint16 *) BPE_MALLOC_ALIGNED(16, sizeof(THandleIndex) * mMaxHandles);
   for (int loop = 0; loop < mMaxHandles; loop++) 
      mpUnusedHandles[loop] = loop;

   // Block management
   mMaxBlockEntries = (mMaxHandles * 2) + 1;  // Additional free handle for remaining space.
   mpBlockEntries = (SBlockEntry *) BPE_MALLOC_ALIGNED(16, sizeof(SBlockEntry) * mMaxBlockEntries);
   memset(mpBlockEntries, 0, sizeof(SBlockEntry) * mMaxBlockEntries);

   mpUnusedBlockEntries = (uint16 *) BPE_MALLOC_ALIGNED(16, sizeof(TBlockIndex) * mMaxBlockEntries);
   for (int loop = 0; loop < mMaxBlockEntries; loop++) mpUnusedBlockEntries[loop] = loop;

   for (int loop = 0; loop < skFreeListBucketCount; loop++) mFreeLists[loop] = kBlockEntryTerminator;

   // Initial block entry
   mFirstBlockEntry = _allocate_block_entry(this);  // This will probably always be zero, but this  aids code maintenance
   SBlockEntry &initialFreeBlock = mpBlockEntries[mFirstBlockEntry]; 
   initialFreeBlock.SetOffset(0);
   initialFreeBlock.mSize = mPoolSize;
   initialFreeBlock.mPrevEntry = kBlockEntryTerminator;
   initialFreeBlock.mNextEntry = kBlockEntryTerminator;
   AddToFreeLists(initialFreeBlock);

   // Pending free list
   mpPendingFreeItems = (SPendingFreeItem *) BPE_MALLOC_ALIGNED(16, sizeof(SPendingFreeItem) * mMaxHandles);
   memset(mpPendingFreeItems, 0, sizeof(SPendingFreeItem) * mMaxHandles);

   mPeakArea[0] = '\0';
}

//------------------------------------------------------------------------------------------

CRenderHWAllocator::~CRenderHWAllocator()
{
   BPE_FREE_ALIGNED(mpPendingFreeItems);
   BPE_FREE_ALIGNED(mpUnusedBlockEntries);
   BPE_FREE_ALIGNED(mpHandleData);
   BPE_FREE_ALIGNED(mpBlockEntries);
   BPE_FREE_ALIGNED(mpUnusedHandles);
}

//------------------------------------------------------------------------------------------

CRenderHWAllocator::SHandle const * const CRenderHWAllocator::Alloc(uint32 const size, uint32 const alignment)
{
   CAllocatorMutexLock mutexLock;

   BPE_ASSERT(size != 0, "No support for zero sized allocations.");

   if (mNumHandles == mMaxHandles)
   {
      // Out of handles
      return NULL;
   }
   
   BPE_ASSERT(alignment > 3, "Minimum alignment is 4 bytes.");

   // Assumes power of 2 alignment
   uint32 const alignmentMask = alignment - 1;

   // Find best fit free block
   uint32 bestFitSizeRequiredSize = 0;
   uint32 bestFitSizeAlignmentPadding = 0;
   TBlockIndex bestFitBlockIndex = kBlockEntryTerminator;

   {
      uint32 bestFitSizeDelta = 0xFFFFFFFF;
      uint32 const paddedSize = ((size + (SBlockEntry::skOffsetFlagsAlignment)) & ~SBlockEntry::skOffsetFlagsAlignment);
      uint32 bucketIndex = _get_bucket_entry_for_size(paddedSize);

      while (bucketIndex < skFreeListBucketCount)
      {
         TBlockIndex currentBlockIndex = mFreeLists[bucketIndex];
         uint32 bestFitOffset = 0xFFFFFFFF;  // Want fit at lowest offset.
         while (currentBlockIndex != kBlockEntryTerminator)
         {
            SBlockEntry & blockEntry = mpBlockEntries[currentBlockIndex];

            // Sanity check
            BPE_ASSERT(bucketIndex == _get_bucket_entry_for_size(blockEntry.mSize), "Free block in wrong bucket.");
            BPE_ASSERT(blockEntry.IsAllocated() == false, "Allocated block in free list.");

            uint32 alignmentPadding = blockEntry.GetOffset() & alignmentMask;
            if (alignmentPadding > 0)
            {
               alignmentPadding = alignment - alignmentPadding;
            }
            // Pad up size for alignment
            uint32 const requiredSize = paddedSize + alignmentPadding;
            if (blockEntry.mSize >= requiredSize)
            {
               uint32 const sizeRemaining = blockEntry.mSize - requiredSize;
               // Find smaller entry or same size entry but with a smaller offset
               if ((sizeRemaining < bestFitSizeDelta) || ((sizeRemaining == bestFitSizeDelta) && (blockEntry.GetOffset() < bestFitOffset)))
               {
                  bestFitSizeDelta = sizeRemaining;
                  bestFitSizeRequiredSize = requiredSize;
                  bestFitOffset = blockEntry.GetOffset();
                  bestFitSizeAlignmentPadding = alignmentPadding;
                  bestFitBlockIndex = currentBlockIndex;
               }
            }
            currentBlockIndex = blockEntry.mNextSizeEntry;
         }
         if (currentBlockIndex != kBlockEntryTerminator)
         {
            // We've found a fit within this bucket.
            // Break, as next bucket will have larger sizes.
            break;
         }
         bucketIndex++;
      }
   }

   if (bestFitBlockIndex == kBlockEntryTerminator)
   {
      // Couldn't find block
      return NULL;
   }

   THandleIndex const handleIndex = _allocate_handle(this);   
   SHandleData * pHandleData = mpHandleData + handleIndex;

   THandleIndex allocBlockIndex = bestFitBlockIndex;  // Initialize to best fit in case we use the entire block

   {
      SBlockEntry & freeBlockEntry = mpBlockEntries[bestFitBlockIndex];

      // Set address pointer for handle
      pHandleData->mpAddress = mpPool + freeBlockEntry.GetOffset() + bestFitSizeAlignmentPadding;

      uint32 const freeBlockSizeRemaining = freeBlockEntry.mSize - bestFitSizeRequiredSize;                     
      if (freeBlockSizeRemaining > 0)
      {
         // Size is remaining for free block, split memory and use a new block for allocated chunk.
         allocBlockIndex = _allocate_block_entry(this);   
         pHandleData->mBlockEntryIndex = allocBlockIndex;

         // Optimization Note:
         // We should add support for splitting blocks to reduce overhead of very large alignment parameters such as 64k RSX tiles.
         SBlockEntry & allocBlockEntry = mpBlockEntries[allocBlockIndex];
         allocBlockEntry.SetOffset(freeBlockEntry.GetOffset());
         allocBlockEntry.mSize = bestFitSizeRequiredSize;
         allocBlockEntry.mPrevEntry = freeBlockEntry.mPrevEntry;
         allocBlockEntry.mNextEntry = bestFitBlockIndex;
         allocBlockEntry.mPrevSizeEntry = kBlockEntryTerminator;  // reset
         allocBlockEntry.mNextSizeEntry = kBlockEntryTerminator;  // reset
         allocBlockEntry.MarkAsAllocated();

         // Check to if split block was the first entry
         if (freeBlockEntry.mPrevEntry == kBlockEntryTerminator)
         {
            // Yep, set new allocated block as the first entry
            mFirstBlockEntry = allocBlockIndex;
         }

         // Could optimize removal and re-insertion into free lists as might remain in same bucket.
         UpdateInFreeListsAndModifySize(freeBlockEntry, freeBlockSizeRemaining);

         // Update free block size, and point to newly allocated block
         freeBlockEntry.AddToOffset(bestFitSizeRequiredSize);
         freeBlockEntry.mSize = freeBlockSizeRemaining;
         if (freeBlockEntry.mPrevEntry != kBlockEntryTerminator)
         {
            // Update prev prev entry index to new allocated block
            SBlockEntry & prevPrevEntry = mpBlockEntries[freeBlockEntry.mPrevEntry];
            prevPrevEntry.mNextEntry = allocBlockIndex;
         }

         freeBlockEntry.mPrevEntry = allocBlockIndex;
      }
      else
      {
         // We're using up the entire space of the block, change to an allocated block.
         RemoveFromFreeLists(freeBlockEntry);
         freeBlockEntry.MarkAsAllocated();
         pHandleData->mBlockEntryIndex = bestFitBlockIndex;
      }
   }   

   // Stats
   mTotalAllocatedSize += bestFitSizeRequiredSize;

   if ( mTotalAllocatedSize > mPeakAllocatedSize )
   {
      mPeakAllocatedSize = mTotalAllocatedSize;
      strcpy(mPeakArea, RenderBackend()->GetCurrentAreaDebugName());
   }

   return (SHandle const*) pHandleData;
}

//------------------------------------------------------------------------------------------

void CRenderHWAllocator::PendingFree(SHandle const * const pHandle, uint32 const flushWaitCount)
{
   CAllocatorMutexLock mutexLock;

   BPE_ASSERT(pHandle != NULL, "NULL handle.");
   BPE_VERIFY(mNumPendingFreeBlocks < mMaxHandles, false, "Too many pending frees!");

   SHandleData * pHandleData = (SHandleData *) pHandle;
   mpPendingFreeItems[mNumPendingFreeBlocks].mFlushCount = mFlushPendingFreeCounter + flushWaitCount;
   mpPendingFreeItems[mNumPendingFreeBlocks].mBlockEntryIndex = pHandleData->mBlockEntryIndex;
   mNumPendingFreeBlocks++;
   _deallocate_handle(this, pHandle);
}

//------------------------------------------------------------------------------------------

void CRenderHWAllocator::ImmediateFree(SHandle const * const pHandle)
{
   CAllocatorMutexLock mutexLock;

   BPE_ASSERT(pHandle != NULL, "NULL handle.");

   // Mark block as free
   SHandleData * pHandleData = (SHandleData *) pHandle;
   TBlockIndex const freedBlockIndex = pHandleData->mBlockEntryIndex;
   _deallocate_handle(this, pHandle);

   ImmediateFree_Internal(freedBlockIndex);
}

//------------------------------------------------------------------------------------------

void CRenderHWAllocator::ImmediateFree_Internal(TBlockIndex const originalFreedBlockIndex)
{
   SBlockEntry *pFreedBlock = &mpBlockEntries[originalFreedBlockIndex];
   TBlockIndex freedBlockIndex = originalFreedBlockIndex;

   // Stats
   mTotalAllocatedSize -= pFreedBlock->mSize;

   // Check for merge with previous block
   if (pFreedBlock->mPrevEntry != kBlockEntryTerminator)
   {
      SBlockEntry &prevBlock = mpBlockEntries[pFreedBlock->mPrevEntry];
      if (!prevBlock.IsAllocated())
      {
         uint32 const endOffset = prevBlock.GetOffset() + prevBlock.mSize;
         if (endOffset == pFreedBlock->GetOffset())
         {
            // We can merge these blocks
            RemoveFromFreeLists(prevBlock);
            prevBlock.mSize += pFreedBlock->mSize;
            prevBlock.mNextEntry = pFreedBlock->mNextEntry;
            if (pFreedBlock->mNextEntry != kBlockEntryTerminator)
            {
               // Update next block indexing this as a previous block as we're about to delete it.               
               SBlockEntry &nextBlock = mpBlockEntries[pFreedBlock->mNextEntry];
               nextBlock.mPrevEntry = pFreedBlock->mPrevEntry;
            }

            _deallocate_block_entry(this, freedBlockIndex);

            freedBlockIndex = pFreedBlock->mPrevEntry;

            // Update this as block to merge with next entry if possible
            pFreedBlock = &prevBlock;
         }
      }
   }

   // Check for merge with next block
   if (pFreedBlock->mNextEntry != kBlockEntryTerminator)
   {
      TBlockIndex const nextEntryIndex = pFreedBlock->mNextEntry;
      SBlockEntry &nextBlock = mpBlockEntries[nextEntryIndex];
      if (!nextBlock.IsAllocated())
      {
         uint32 const endOffset = pFreedBlock->GetOffset() + pFreedBlock->mSize;
         if (endOffset == nextBlock.GetOffset())
         {
            // We can merge these blocks
            RemoveFromFreeLists(nextBlock);
            pFreedBlock->mSize += nextBlock.mSize;
            pFreedBlock->mNextEntry = nextBlock.mNextEntry;
            if (nextBlock.mNextEntry != kBlockEntryTerminator)
            {
               // Update next block indexing this as a previous block as we're about to delete it.               
               SBlockEntry &nextNextBlock = mpBlockEntries[nextBlock.mNextEntry];
               nextNextBlock.mPrevEntry = freedBlockIndex;
            }
            
            _deallocate_block_entry(this, nextEntryIndex);
         }
      }
   }

   // Flag as deallocated
   pFreedBlock->MarkAsFree();
   // Add to free list
   AddToFreeLists(*pFreedBlock);
}

//------------------------------------------------------------------------------------------

void CRenderHWAllocator::AddToFreeLists(SBlockEntry &entry)
{
   uint32 const bucketIndex = _get_bucket_entry_for_size(entry.mSize);

   // Put this free block at start of free entry linked list for this bucket.
   TBlockIndex const entryIndex = &entry - mpBlockEntries;

   entry.mPrevSizeEntry = kBlockEntryTerminator;

   TBlockIndex const prevEntryIndex = mFreeLists[bucketIndex];
   mFreeLists[bucketIndex] = entryIndex;
   if (prevEntryIndex != kBlockEntryTerminator)
   {
      // Update previous first entry
      SBlockEntry &prevEntry = mpBlockEntries[prevEntryIndex];
      // Sanity check
      BPE_ASSERT(bucketIndex == _get_bucket_entry_for_size(prevEntry.mSize), "Free block in wrong bucket.");
      BPE_ASSERT(prevEntry.IsAllocated() == false, "Allocated block in free list.");

      prevEntry.mPrevSizeEntry = entryIndex;
      BPE_ASSERT((prevEntry.mPrevSizeEntry != prevEntry.mNextSizeEntry) || (prevEntry.mNextSizeEntry == kBlockEntryTerminator), "Prev/Next size entries are the same.");
   }
   entry.mNextSizeEntry = prevEntryIndex;
   BPE_ASSERT((entry.mPrevSizeEntry != entry.mNextSizeEntry) || (entry.mNextSizeEntry == kBlockEntryTerminator), "Prev/Next size entries are the same.");
}

//------------------------------------------------------------------------------------------

void CRenderHWAllocator::RemoveFromFreeLists(SBlockEntry &entry)
{
   // Remove from free lists
   if (entry.mPrevSizeEntry == kBlockEntryTerminator)
   {
      // First entry in bucket, update bucket
      uint32 const bucketIndex = _get_bucket_entry_for_size(entry.mSize);
      mFreeLists[bucketIndex] = entry.mNextSizeEntry;
   }
   else
   {
      SBlockEntry &prevEntry = mpBlockEntries[entry.mPrevSizeEntry];
      BPE_ASSERT(_get_bucket_entry_for_size(entry.mSize) == _get_bucket_entry_for_size(prevEntry.mSize), "Free block in wrong bucket.");
      BPE_ASSERT(prevEntry.IsAllocated() == false, "Allocated block in free list.");
      BPE_ASSERT((prevEntry.mPrevSizeEntry != prevEntry.mNextSizeEntry) || (prevEntry.mNextSizeEntry == kBlockEntryTerminator), "Prev/Next size entries are the same.");

      prevEntry.mNextSizeEntry = entry.mNextSizeEntry;
   }

   // Update following entry in linked size list
   if (entry.mNextSizeEntry != kBlockEntryTerminator)
   {
      SBlockEntry &nextEntry = mpBlockEntries[entry.mNextSizeEntry];
      BPE_ASSERT(_get_bucket_entry_for_size(entry.mSize) == _get_bucket_entry_for_size(nextEntry.mSize), "Free block in wrong bucket.");
      BPE_ASSERT(nextEntry.IsAllocated() == false, "Allocated block in free list.");
      BPE_ASSERT((nextEntry.mPrevSizeEntry != nextEntry.mNextSizeEntry) || (nextEntry.mNextSizeEntry == kBlockEntryTerminator), "Prev/Next size entries are the same.");

      nextEntry.mPrevSizeEntry = entry.mPrevSizeEntry;
   }
}

//------------------------------------------------------------------------------------------
// Must be called before size is updated.
// Has the size effect of updating the block entry size.
void CRenderHWAllocator::UpdateInFreeListsAndModifySize(NRenderHWAllocator::SBlockEntry &entry_SizeModified, uint32 const newSize)
{
   BPE_ASSERT(entry_SizeModified.IsAllocated() == false, "Can't free an allocated block.");
   if (_get_bucket_entry_for_size(entry_SizeModified.mSize) != _get_bucket_entry_for_size(newSize))
   {
      // Bucket has changed, update
      RemoveFromFreeLists(entry_SizeModified);
      entry_SizeModified.mSize = newSize;
      AddToFreeLists(entry_SizeModified);
   }
}

//------------------------------------------------------------------------------------------

void CRenderHWAllocator::FlushPendingFree()
{
   CAllocatorMutexLock mutexLock;

   mFlushPendingFreeCounter++;

   // Flush all pending free items that have waited for enough flushes.
   for (int loop = 0; loop < mNumPendingFreeBlocks; loop++)
   {
      if (mpPendingFreeItems[loop].mFlushCount == mFlushPendingFreeCounter)
      {         
         // We can delete this
         ImmediateFree_Internal(mpPendingFreeItems[loop].mBlockEntryIndex);

         // Copy last over current position
         mNumPendingFreeBlocks--;
         mpPendingFreeItems[loop] = mpPendingFreeItems[mNumPendingFreeBlocks];

         // Stay on current location as it has been updated
         loop--;
      }
   }
}

//------------------------------------------------------------------------------------------

bool CRenderHWAllocator::IsHandleInPool(SHandle const * pHandle) const
{
   CAllocatorMutexLock mutexLock;

   uint32 const handleIndex = ((SHandleData*)pHandle) - mpHandleData;
   
   bool const result = handleIndex < mMaxHandles;
   return result;
}

//------------------------------------------------------------------------------------------

void CRenderHWAllocator::GetStats(SRenderHWAllocatorStats * pStats) const
{
   memset(pStats, 0, sizeof(SRenderHWAllocatorStats));

   pStats->mAllocatedSize = mTotalAllocatedSize;
   pStats->mPeakAllocatedSize = mPeakAllocatedSize;
   strcpy(pStats->mPeakArea, mPeakArea);

   SPendingFreeItem const * freeItem = mpPendingFreeItems;
   for( int i = 0; i < mNumPendingFreeBlocks; ++i )
   {
      SBlockEntry const & block = mpBlockEntries[freeItem->mBlockEntryIndex];
      pStats->mPendingFreeSize += block.mSize;
      ++freeItem;
   }

   pStats->mTotalSize = mPoolSize;

   pStats->mAllocatedHandles = mNumHandles;
   pStats->mPendingFreeBlocks = mNumPendingFreeBlocks;
   pStats->mTotalHandles = mMaxHandles;

   strcpy(pStats->mPeakArea, mPeakArea);
}

//------------------------------------------------------------------------------------------
#if 0 // Simple testing code
#include "Engine/Math/CRandom.h"
#include "Engine/System/CStopWatch.h"
#include "CRenderHWAllocator.h"

   {
      CTimeBlock tb("allocator");
      int const skMaxHandles = 4096;
      int const poolSize = 4096 * 256;
      void * pPool = malloc(poolSize);
      CRenderHWAllocator alloc(pPool, poolSize, skMaxHandles);

      std::vector<CRenderHWAllocator::SHandle const *> mHandles;      
      CRandom random(99);

      for (int loop = 0; loop < 50; loop++)
      {
         CTimeBlock tb("allocatorLoop");
         while (mHandles.size() < skMaxHandles)
         {
            uint32 const size = random.RangeInt(1, 256);
            CRenderHWAllocator::SHandle const * pHandle = alloc.Alloc(size, 4);
            BPE_ASSERT(pHandle != NULL, "NULL handle.");
            mHandles.push_back(pHandle);
         }

         // Random free 50% of the handles
         while (mHandles.size() > skMaxHandles / 2)
         {
            int const index = random.RangeInt(0, (mHandles.size() - 1));
            alloc.ImmediateFree(mHandles[index]);
            mHandles.erase(mHandles.begin() + index);
         }
         bpe_debugger_printf("Handles: %d Size: %d NumBlockEntries: %d\n", alloc.mNumHandles, alloc.mTotalAllocatedSize, alloc.mNumBlockEntries);
      }

      // Free all handles
      while (mHandles.size() > 0)
      {
         int const index = random.RangeInt(0, (mHandles.size() - 1));
         alloc.ImmediateFree(mHandles[index]);
         mHandles.erase(mHandles.begin() + index);
      }
      bpe_debugger_printf("Handles: %d Size: %d NumBlockEntries blocks: %d\n", alloc.mNumHandles, alloc.mTotalAllocatedSize, alloc.mNumBlockEntries);
      free(pPool);
   }

#endif

//------------------------------------------------------------------------------------------


