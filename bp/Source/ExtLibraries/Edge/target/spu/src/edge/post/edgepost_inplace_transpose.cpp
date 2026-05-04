/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <stdint.h>
#include <spu_intrinsics.h>
#include <cell/spurs.h>
#include <cell/sync/barrier.h>

#include <edge/edge_assert.h>
#include <edge/edge_dma.h>
#include <edge/post/edgepost_inplace_transpose.h>
#include <edge/post/edgepost_direction_lock.h>


/*
 * This code does not do a full transpose of an image, but instead transposes 128^2 pixel blocks
 * in-place. This allows us to treat the image as fully transposed through the help of dma-lists.
 * As we treat the image as block of 128^2 size, IMAGE HEIGHT AND WIDTH MUST BE A MULTIPLE OF 128!
 * (Sorry for the screaming...)
 *
 * If you want to have the lowest total SPU time, run this code on just one SPU. If you want to have
 * the lowest latency, take two.
 */

#define ALL_DMA_TAGS (7<<baseDmaTag)

extern "C" void edgePostTranspose(uint32_t* src, uint32_t* src, uint32_t size);

__attribute__((noinline)) static void _edgePostTransposeDmaWaitMayStall(uint32_t tag)
{
	EDGE_DMA_WAIT_TAG_STATUS_ALL(tag);
}

__attribute__((noinline)) static void _edgePostTransposeDmaWaitShouldNotStall(uint32_t tag)
{
	EDGE_DMA_WAIT_TAG_STATUS_ALL(tag);
}

static void _edgePostInplaceTransposePostOperation(
	EdgePostInplaceTranposeMemoryLayout* const memory,
	const register uint32_t index,
	EdgePostInplaceTransposePostOpFunction postOpFunction,
	void *postOpParameter)
{
	if(postOpFunction)
	{
		const uint32_t buffer_index = index%3;
		postOpFunction(postOpParameter,memory->TileBuffer[buffer_index],memory->BlockSize*memory->BlockSize*sizeof(uint32_t));
	}
}

static inline void _edgePostFillEvenSizedDmaListAligned(
	register EdgeDmaListElement* list,
	register const uint32_t elements,
	register const uint32_t startEa,
	register const uint32_t eaUpdate,
	register const uint32_t fetchSize)
{
	register const vec_uint4 element_update = {0,2*eaUpdate,0,2*eaUpdate};
	register vec_uint4 two_elements         = {fetchSize,startEa,fetchSize,startEa+eaUpdate};
	

	register vec_uint4 buffer = {(uint32_t)list,0,0,0};


	for(register int32_t i = elements; i > 0; i-=2)
	{
		si_stqd((qword)two_elements,(qword)buffer,0);
		two_elements+=element_update;
		buffer=(vec_uint4)si_ai((qword)buffer,0x10);
	}
}

static void _edgePostPrepareFetchBlock(
	register EdgePostInplaceTranposeMemoryLayout* const memory,
	const register uint32_t index,
	const register uint32_t imageEa,
	const register uint32_t imagePitch,
	const register uint32_t shear)
{
	const register uint32_t slot_index=index%3;

	const register uint32_t blocks_per_line = imagePitch / (memory->BlockSize * 4);
	const register uint32_t real_index = 2*index+shear;		// because we now interleave the blocks between SPUs.
	const register uint32_t ea =
		imageEa +
		(real_index / blocks_per_line) * memory->BlockSize * imagePitch + //vertical position
		(real_index % blocks_per_line) * memory->BlockSize * 4; //horizontal position


	_edgePostFillEvenSizedDmaListAligned(
		memory->DmaList[2*slot_index+1],
		memory->BlockSize,
		ea,
		imagePitch,
		memory->BlockSize*4);
}


static void _edgePostFetchBlock(
	register EdgePostInplaceTranposeMemoryLayout* const memory,
	const register uint32_t index,
	const register uint32_t imageEa,
	const register uint32_t imagePitch,
	const register uint32_t shear,
	const register uint32_t baseDmaTag)
{
	(void)imageEa;
	(void)imagePitch;
	(void)shear;

	const register uint32_t slot_index=index%3;

	{
		const bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
		spu_idisable();
		_edgePostTransposeDmaWaitMayStall(ALL_DMA_TAGS); //we don't want to mix, so just make sure everyone else is done.

		if( interrupts_enabled ) 
		{
			spu_ienable();
		}
	}

	_edgeDirectionLockSetToRead(&memory->Lock,true);

	{
		const bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
		spu_idisable();
		EDGE_DMA_LIST_GETF(
			memory->TileBuffer[slot_index],
			0,
			memory->DmaList[2*slot_index+1],
			memory->BlockSize * sizeof(EdgeDmaListElement),
			slot_index+baseDmaTag,0,0);
		if( interrupts_enabled ) 
		{
			spu_ienable();
		}
	}
}

static void _edgePostPrepareStoreBlock(
	      register EdgePostInplaceTranposeMemoryLayout* const memory,
	const register uint32_t index,
	const register uint32_t imageEa,
	const register uint32_t imagePitch,
	const register uint32_t shear)
{
	const register uint32_t slot_index=(index+2)%3;

	const register uint32_t blocks_per_line = imagePitch / (memory->BlockSize * 4);
	const register uint32_t real_index = 2*index+shear;		// because we now interleave the blocks between SPUs.
	const register uint32_t ea =
		imageEa +
		(real_index / blocks_per_line) * memory->BlockSize * imagePitch + //vertical position
		(real_index % blocks_per_line) * memory->BlockSize * 4; //horizontal position


	_edgePostFillEvenSizedDmaListAligned(
		memory->DmaList[2*slot_index],
		memory->BlockSize,
		ea,
		imagePitch,
		memory->BlockSize*4);
}


static void _edgePostStoreBlock(
	      register EdgePostInplaceTranposeMemoryLayout* const memory,
	const register uint32_t index,
	const register uint32_t imageEa,
	const register uint32_t imagePitch,
	const register uint32_t shear,
	const register uint32_t baseDmaTag)
{
	(void)imageEa;
	(void)imagePitch;
	(void)shear;

	const register uint32_t slot_index=(index+2)%3;
	const register uint32_t prev_fetch_index=(index+1)%3;

	{
		const bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
		spu_idisable();

		// first we make sure the previous fetch is done,
		// to prevent XDR port contention
		_edgePostTransposeDmaWaitShouldNotStall(1<<(prev_fetch_index+baseDmaTag));

		// now make sure all stores are done, globally
		_edgeDirectionLockSetToWrite(&memory->Lock,true);

		// only when we are sure there are no more gets in-flight, do we store.
		EDGE_DMA_LIST_PUTF(
			memory->TileBuffer[slot_index],
			0,
			memory->DmaList[2*slot_index],
			memory->BlockSize * sizeof(EdgeDmaListElement),
			slot_index+baseDmaTag,0,0);
		if( interrupts_enabled )
		{
			spu_ienable();
		}
	}

}

static void _edgePostTransposeBlock(
	EdgePostInplaceTranposeMemoryLayout* const memory,
	const register uint32_t index,
	const register uint32_t baseDmaTag)
{
	const register uint32_t src_index=index%3;
	const register uint32_t dst_index=(index+2)%3;

	{
		const bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1; 
		spu_idisable();
		_edgePostTransposeDmaWaitShouldNotStall(1<<(src_index+baseDmaTag));	//make sure our source data has arrived.
		if( interrupts_enabled ) 
		{
			spu_ienable();
		}
	}
	

	edgePostTranspose(
		memory->TileBuffer[src_index],
		memory->TileBuffer[dst_index],
		memory->BlockSize);

}

#define PREPARE_FETCH_BLOCK(a) _edgePostPrepareFetchBlock(memory, a, sourceEa, imagePitch, shear)
#define PREPARE_STORE_BLOCK(a) _edgePostPrepareStoreBlock(memory, a, destEa, imagePitch, shear)
#define FETCH_BLOCK(a) _edgePostFetchBlock(memory, a, sourceEa, imagePitch, shear, baseDmaTag)
#define STORE_BLOCK(a) _edgePostStoreBlock(memory, a, destEa, imagePitch, shear, baseDmaTag)
#define TRANSPOSE_BLOCK(a) _edgePostTransposeBlock(memory, a, baseDmaTag)


static void _edgePostTranspose(
	EdgePostInplaceTranposeMemoryLayout* const memory,
	const uint32_t sourceEa, 
	const uint32_t destEa, 
	const uint32_t imageLines, 
	const uint32_t imagePitch, 
	const uint32_t shear, 
	const uint32_t directionLockAddress,
	EdgePostInplaceTransposePostOpFunction postOpFunction,
	void *postOpParameter,
	const uint32_t baseDmaTag)
{
	uint32_t blocks=imageLines * imagePitch;
	// if there is an uneven number of blocks, the SPU with shear 0 will have to take care of it.
	if(0 == shear) blocks += (memory->BlockSize * memory->BlockSize * 4) - 1;
	blocks /= (memory->BlockSize * memory->BlockSize * 4);

	// Set up the direction lock. We might lose this when evicted, so do every time.

	_edgeDirectionLockInitialize(
		&memory->Lock,
		directionLockAddress,
		memory->AtomicBuffer);


	_edgeDirectionLockSetToRead(&memory->Lock,true);
	PREPARE_FETCH_BLOCK(0);
	FETCH_BLOCK(0);
	PREPARE_FETCH_BLOCK(1);
	FETCH_BLOCK(1);
	PREPARE_STORE_BLOCK(0);	

	TRANSPOSE_BLOCK(0);

	_edgePostInplaceTransposePostOperation(memory, 2, postOpFunction, postOpParameter);

	PREPARE_STORE_BLOCK(1);
	PREPARE_FETCH_BLOCK(2);

	STORE_BLOCK(0);

	for(uint32_t i = 1; i < blocks - 2; ++i)
	{		
		TRANSPOSE_BLOCK(i);
		FETCH_BLOCK(i + 1);
		// here is a free block of computations time.
		_edgePostInplaceTransposePostOperation(memory, i + 2, postOpFunction,postOpParameter);

		
		PREPARE_STORE_BLOCK(i+1);
		PREPARE_FETCH_BLOCK(i+2);

		STORE_BLOCK(i);
	}	
	PREPARE_STORE_BLOCK(blocks - 2);
	TRANSPOSE_BLOCK(blocks - 2);
	FETCH_BLOCK(blocks - 1);
	_edgePostInplaceTransposePostOperation(memory, blocks, postOpFunction, postOpParameter);
	STORE_BLOCK(blocks - 2);
	PREPARE_STORE_BLOCK(blocks - 1);
	TRANSPOSE_BLOCK(blocks - 1);
	_edgePostInplaceTransposePostOperation(memory, blocks + 1, postOpFunction, postOpParameter);
	STORE_BLOCK(blocks - 1);
	_edgeDirectionLockRelease(&memory->Lock,true);
}

void edgePostTransposeInPlace(
	EdgePostInplaceTranposeMemoryLayout* memory,
	uint32_t sourceEa,
	uint32_t destEa,
	uint32_t imagePitch,
	uint32_t imageHeight,
	uint32_t blockSize,
	uint32_t spuId,
	uint32_t directionLockAddress,
	EdgePostInplaceTransposePostOpFunction postOpFunction,
	void *postOpParameter,
	uint32_t baseDmaTag)
{
	memory->BlockSize = blockSize;
	EDGE_ASSERT(baseDmaTag<30); //make sure we have enough tags available.
	_edgePostTranspose(memory, sourceEa, destEa, imageHeight, imagePitch, spuId, directionLockAddress,postOpFunction,postOpParameter,baseDmaTag);
	{
		const bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
		spu_idisable();
		_edgePostTransposeDmaWaitMayStall(ALL_DMA_TAGS);
		if( interrupts_enabled )
		{
			spu_ienable();
		}
	}
}
