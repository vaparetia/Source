/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_POST_BLOOM_H__
#define __EDGE_POST_BLOOM_H__

#include <spu_intrinsics.h>
#include "edge/edge_stdint.h"
#include "edge/post/edgepost.h"


extern "C"
{

	// bloom capture
	void edgePostBloomCapture8( void* output, const void* input, uint32_t count, vec_float4 exposureLevel, 
		vec_float4 minLuminance, vec_float4 luminanceRangeRcp );

	void edgePostBloomCaptureFX16( void* output, const void* input, uint32_t count, vec_float4 exposureLevel, 
		vec_float4 minLuminance, vec_float4 luminanceRangeRcp );

	// Tone-mapping
	// NOTE: slow due to gamma correction
	void edgePostTonemapFX16( void* output, void* input, uint32_t count, float avgLuminance, float middleGray, float white );

	// find max and average luminance
	void edgePostAvgLuminanceFX16( const void* input, uint32_t count, vec_float4* avgLuminancePtr, vec_float4* maxLuminancePtr );
}



#endif // __EDGE_POST_BLOOM_H__
