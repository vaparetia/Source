/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_POST_RESAMPLE_H__
#define __EDGE_POST_RESAMPLE_H__

#include <spu_intrinsics.h>
#include "edge/edge_stdint.h"
#include "edge/post/edgepost.h"


extern "C"
{
	// bilinear downsample
	// pixel format is four 8 bit channel
	void edgePostDownsample8( void* output, const void* input, uint32_t stride, uint32_t count );

	// bilinear downsample
	// pixel format is four 16 bit channel
	void edgePostDownsample16( void* output, const void* input, uint32_t stride, uint32_t count );

	// bilinear downsample
	// pixel format is single channel float
	void edgePostDownsampleF( void* output, const void* input, uint32_t stride, uint32_t count );
	
	// nearest downsample
	// any pixel format with 32 bit size
	void edgePostNearestDownsample( void* output, const void* input, uint32_t count );
	
	// candidate downsample ( choose min sample )
	// pixel format is single channel float
	void edgePostDownsampleFloatMin( void* output, const void* input, uint32_t stride, uint32_t count );
	
	// candidate downsample ( choose max sample )
	// pixel format is single channel float
	void edgePostDownsampleFloatMax( void* output, const void* input, uint32_t stride, uint32_t count );

	// bilinear upsample
	// pixel format is four 8 bit channel
	void edgePostUpsample8( void* output, const void* input, uint32_t istride, uint32_t ostride, uint32_t count );

	// bilinear upsample
	// pixel format is four 16 bit channel
	void edgePostUpsample16( void* output, const void* input, uint32_t istride, uint32_t ostride, uint32_t count );
	
	// bilinear upsample
	// pixel format is single channel float
	void edgePostUpsampleF( void* output, const void* input, uint32_t istride, uint32_t ostride, uint32_t count );

}

#endif // __EDGE_POST_SPU_H__
