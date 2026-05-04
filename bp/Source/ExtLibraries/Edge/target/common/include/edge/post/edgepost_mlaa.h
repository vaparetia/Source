/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef EDGEPOST_MLAA_H
#define EDGEPOST_MLAA_H

#include <edge/edge_stdint.h>

#define EDGE_POST_MLAA_MODE_ENABLED					(1<<0)
#define EDGE_POST_MLAA_MODE_SHOW_EDGES				(1<<1)
#define EDGE_POST_MLAA_MODE_SINGLE_SPU_TRANSPOSE	(1<<2)
#define EDGE_POST_MLAA_MODE_TRANSPOSE_64			(1<<3)

typedef __attribute__((aligned(16))) struct EdgePostMlaaTaskParameters
{
	uint32_t	rsxLabelValue;				// If you want the task to write an RSX label when done, use
	uint32_t	rsxLabelAddress;			// these two parameters. You'll only want to set this for one SPU.
	uint32_t	taskCounterAddress;			// If you want each SPU to atomically increment a counter when done, set this.
	uint32_t	imageAddress;				// Source address of the image.
	uint32_t	destAddress;				// The target address can be identical to imageAddress for in-place MLAA.
	uint32_t	barrierAddress;				// Address to a SPURS barrier used to synchronize multi-SPU operation.
	uint32_t	directionLockAddress;		// Address of an EdgeDirectionLock, used during the transpose passes.
	uint16_t	imageWidth;					// Width of the image.
	uint16_t	imageHeight;				// Height of the image, not including padding.
	uint16_t	imagePitch;					// Pitch in bytes.
	uint8_t		mode;						// Combination of EDGE_POST_MLAA_MODE_* flags.
	uint8_t		spuId : 4;					// Number of this SPU.
	uint8_t		spuCount : 4;				// Total number of SPUs.
	uint16_t	parameter0;					// Edge detection parameter0. See documentation.
	uint16_t	parameter1;					// Edge detection parameter1. See documentation.
	uint8_t		reserved[24];				// Reserved space for future extensions.	
} EdgePostMlaaTaskParameters;

#ifdef __SPU__

struct EdgePostMlaaMemoryLayout;			// from edgepost_mlaa_memory_layout.h
struct EdgePostInplaceTranposeMemoryLayout;	// from edgepost_inplace_transpose.h

#ifdef __cplusplus
extern "C" {
#endif

void edgePostTransposePass(EdgePostInplaceTranposeMemoryLayout* layout, const EdgePostMlaaTaskParameters* parameter, uint32_t first, uint32_t maxSpus, uint32_t baseDmaTag);
void edgePostMlaaPass(EdgePostMlaaMemoryLayout* layout, const EdgePostMlaaTaskParameters* parameter, uint32_t pass, uint32_t first, uint32_t baseDmaTag);

#ifdef __cplusplus
}
#endif //__cplusplus


#endif

#endif //EDGEPOST_MLAA_H
