/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_POST_MOTIONBLUR_H__
#define __EDGE_POST_MOTIONBLUR_H__

#include <spu_intrinsics.h>
#include "edge/edge_stdint.h"
#include "edge/post/edgepost.h"

extern "C"
{
	// apply motion blur to one pixel scanline
	// input format is expected to be four floats per pixel, order is argb
	// motion holds motion vectors. It's 4 byte per pixel format with motin vector in 'A' and 'R' conponents
	// output is argb 32 bit
	void edgePostMotionblur( void* output, const void* input, const void* motion, uint32_t stride, uint32_t count );

	// apply motion blur to one pixel scanline
	// input format is expected to be four floats per pixel, order is argb
	// motion holds motion vectors. It's 4 byte per pixel format with motin vector in 'A' and 'R' conponents
	// output is argb 64 bit, each component is fixed 0:5:11
	void edgePostMotionblur_FX16( void* output, const void* input, const void* motion, uint32_t stride, uint32_t count );
}

#endif // __EDGE_POST_MOTIONBLUR_H__
