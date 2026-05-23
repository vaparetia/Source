/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_POST_MASK_H__
#define __EDGE_POST_MASK_H__

#include <spu_intrinsics.h>
#include "edge/edge_stdint.h"
#include "edge/post/edgepost.h"

extern "C"
{
	// Build a mask image from a single channel floating point image
	void edgePostMakeMaskFromFloats( void* output, void* input, uint32_t count );

	// Apply a mask to an FX16 image
	void edgePostApplyMaskFX16( void* output, const void* input, const void* mask, uint32_t count );
}

#endif // __EDGE_POST_MASK_H__
