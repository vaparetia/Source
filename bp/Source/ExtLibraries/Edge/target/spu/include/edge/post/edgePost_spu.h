/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_POST_SPU_H__
#define __EDGE_POST_SPU_H__

#include <spu_intrinsics.h>
#include "edge/edge_stdint.h"
#include "edge/post/edgepost.h"

#include "edgepost_resample.h"
#include "edgepost_convert.h"
#include "edgepost_rops.h"
#include "edgepost_motionblur.h"
#include "edgepost_dof.h"
#include "edgepost_bloom.h"
#include "edgepost_mask.h"
#include "edgepost_ilr.h"

extern "C"
{
	// gauss filters
	void edgePostGauss7x1_8( void* output, const void* input, uint32_t count, vec_float4 weights );
	void edgePostGauss1x7_8( void* output, const void* input, uint32_t stride, uint32_t count, vec_float4 weights );
	void edgePostGauss7x1F( void* output, const void* input, uint32_t count, vec_float4 weights );
	void edgePostGauss1x7F( void* output, const void* input, uint32_t stride, uint32_t count, vec_float4 weights );
	
	// Utility
	void edgePostMemCopy( void* output, const void* input, uint32_t count );
}

#endif // __EDGE_POST_SPU_H__
