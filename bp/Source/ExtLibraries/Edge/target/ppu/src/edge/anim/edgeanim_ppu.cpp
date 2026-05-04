/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <stdlib.h>
#include <string.h>

#include "edge/edge_assert.h"
#ifdef __PPU__
#include "edge/anim/edgeanim_ppu.h"
#else
#include "edge/anim/edgeanim_windows.h"
#endif
#include "edge/anim/edgeanim_common.h"

void edgeAnimPpuInitialize(EdgeAnimPpuContext* ppuContext, unsigned int numSpus, int spuExternalStorageMask,
                            size_t sizeExternalStoragePerSpu, void* externalStorageBlock)
{
    /* EdgeAnimPpuContext objects must be aligned to 16 bytes boundaries (SPU DMA restriction) */
    EDGE_ASSERT(ppuContext);
    EDGE_ASSERT(EDGE_IS_ALIGNED(ppuContext, 16U));
    EDGE_ASSERT(numSpus <= (sizeof(ppuContext->poseCacheArray) / sizeof(ppuContext->poseCacheArray[0])));
    
    /* If we have an external storage mask defined, we must have some 16 byte aligned external storage. */
    EDGE_ASSERT((spuExternalStorageMask != 0) == (externalStorageBlock != NULL));
    EDGE_ASSERT((externalStorageBlock != NULL) == (sizeExternalStoragePerSpu != 0));
    EDGE_ASSERT((spuExternalStorageMask != 0) == (sizeExternalStoragePerSpu != 0));
    EDGE_ASSERT(!(0x0FU & (uintptr_t)externalStorageBlock));
    
    memset(ppuContext, 0, sizeof(*ppuContext));

    uint8_t* currentExternalStorage = (uint8_t*)externalStorageBlock;
    size_t alignedExternalStoragePerSpu = EDGE_ALIGN(sizeExternalStoragePerSpu, 16);
    
    for (unsigned int spuLoop = 0; spuLoop < numSpus; spuLoop++) {
        if (spuExternalStorageMask & (1 << spuLoop)) {
            ppuContext->poseCacheArray[spuLoop].sizePoseCache = (uint32_t) alignedExternalStoragePerSpu;
            ppuContext->poseCacheArray[spuLoop].eaPoseCache = (uintptr_t) currentExternalStorage;
            currentExternalStorage += alignedExternalStoragePerSpu;
        }
    }
}

void edgeAnimPpuFinalize(EdgeAnimPpuContext* ppuContext)
{
    /* validation */
    EDGE_ASSERT(ppuContext);
    EDGE_ASSERT(EDGE_IS_ALIGNED(ppuContext, 16U));

    /* reset */
    for(int spuLoop = (sizeof(ppuContext->poseCacheArray) / sizeof(ppuContext->poseCacheArray[0])) - 1; 
        spuLoop >= 0; 
        spuLoop--) {
        if (ppuContext->poseCacheArray[spuLoop].sizePoseCache) {
            ppuContext->poseCacheArray[spuLoop].eaPoseCache = 0;
            ppuContext->poseCacheArray[spuLoop].sizePoseCache = 0;
        }
    }
}

size_t edgeAnimComputeExternalStorageSize(unsigned int numSpus, int spuExternalStorageMask, size_t sizeExternalStoragePerSpu)
{
    EDGE_ASSERT(numSpus <= EDGE_ANIM_MAX_SPUS);

    size_t  externalStorageSize = 0;

    /* We need a 16-byte aligned block of storage for each SPU that has a bit set in spuExternalStorageMask */
    for (unsigned int spuLoop = 0; spuLoop < numSpus; spuLoop++) {
        if (spuExternalStorageMask & (1 << spuLoop)) {
            externalStorageSize += EDGE_ALIGN(sizeExternalStoragePerSpu, 16);
        }
    }
    
    return externalStorageSize;
}

void edgeAnimSetExternalJointWeights(EdgeAnimAnimation* animation, uint8_t* jointWeights)
{
    /* Validation */
    EDGE_ASSERT(animation);
    EDGE_ASSERT(EDGE_IS_ALIGNED(animation, 16U));
    EDGE_ASSERT(jointWeights);
    EDGE_ASSERT(EDGE_IS_ALIGNED(jointWeights, 16U));

    /* Replace existing joint weights - note that the existing joint weight array is lost because
     * size information has been overwritten */
    animation->sizeJointsWeightArray = (uint32_t) EDGE_ALIGN(animation->numJoints, 16U);
    animation->eaUserJointWeightArray = (uintptr_t) jointWeights;
    animation->offsetJointsWeightArray = 0;
}
