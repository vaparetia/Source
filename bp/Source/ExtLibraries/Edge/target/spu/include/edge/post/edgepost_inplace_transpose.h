/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef EDGEPOST_INPLACE_TRANSPOSE_H
#define EDGEPOST_INPLACE_TRANSPOSE_H

#include <edge/edge_dma.h>
#include "edgepost_direction_lock.h"

#define EDGEPOST_TRANSPOSE_MAX_BLOCK_SIZE 128
#define EDGEPOST_TRANSPOSE_MIN_BLOCK_SIZE 64

struct EdgePostInplaceTranposeMemoryLayout
{
	uint32_t TileBuffer[3][EDGEPOST_TRANSPOSE_MAX_BLOCK_SIZE * EDGEPOST_TRANSPOSE_MAX_BLOCK_SIZE ] __attribute__((aligned(1024))); //one to fetch and store, one to read, one to write.
	EdgeDmaListElement DmaList[6][EDGEPOST_TRANSPOSE_MAX_BLOCK_SIZE] __attribute__((aligned(8)));
	volatile int32_t AtomicBuffer[32] __attribute__((aligned(128)));
	EdgeDirectionLock Lock;
	uint32_t BlockSize;
};

typedef void (*EdgePostInplaceTransposePostOpFunction) (void* param, void* data, size_t dataSize);

#ifdef __cplusplus
extern "C" {
#endif


void edgePostTransposeInPlace(
	EdgePostInplaceTranposeMemoryLayout* layout,
	uint32_t sourceEa, 
	uint32_t destEa, 
	uint32_t imagePitch, 
	uint32_t imageHeight,
	uint32_t blockSize,
	uint32_t spuId, 
	uint32_t lockAddress,
	EdgePostInplaceTransposePostOpFunction postOpFunction,
	void *postOpParameter,
	uint32_t baseDmaTag);

#ifdef __cplusplus
}
#endif //__cplusplus


#endif //EDGEPOST_INPLACE_TRANSPOSE_H
