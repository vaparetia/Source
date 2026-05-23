/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_POST_ROPS_H__
#define __EDGE_POST_ROPS_H__

#include <spu_intrinsics.h>
#include "edge/edge_stdint.h"
#include "edge/post/edgepost.h"


extern "C"
{

	// output = shuffle(input0) * shuffle(input1)
	// 4 channels, 8 bit per channel
	void edgePostModulate8( void* output, const void* input0, qword shuffle0, const void* input1, qword shuffle1, uint32_t count );

	// output = shuffle(input0) * shuffle(input1)
	// 4 channels, 16 bit per channel 0:5:11
	void edgePostModulateFX16( void* output, const void* input0, qword shuffle0, const void* input1, qword shuffle1, uint32_t count );

	// output = shuffle(input0) * constant
	// 4 channels, 8 bit per channel
	void edgePostConstantModulate8( void* output, const void* input0, qword shuffle0, qword constant, uint32_t count );

	// output = input0 + ( input1 * multiplier )
	// 4 channels, 8 bit per channel
	void edgePostAddSat8( void* output, const void* input0, const void* input1, vec_uint4 multiplier, uint32_t count );

	// output = input0 + input1
	// 4 channels, 16 bit per channel
	void edgePostAddSatFX16( void* output, const void* input0, const void* input1, vec_uint4 multiplier, uint32_t count );

	// r = p0 * ( 1 - p1.a ) + p1
	// 4 channels, 8 bit per channel
	void edgePostBlend8( void* output, const void* input0, const void* input1, uint32_t count );

	// r = p0 * ( 1 - p1.a ) + p1
	// 4 channels, 16 bit per channel 0:5:11
	void edgePostBlendFX16( void* output, const void* input0, const void* input1, uint32_t count );
	
	// r = p0 * p0.a
	// 4 channels, 16 bit per channel 0:5:11
	void edgePostPremultiplyFX16( void* output, const void* input0, uint32_t count );

	//	r = shuffle( p0, p1)
	//	process 16 byte a time
	void edgePostCombine( void* output, const void* input0, const void* input1, qword shuffleMask, uint32_t count );

}

#endif // __EDGE_POST_ROPS_H__
