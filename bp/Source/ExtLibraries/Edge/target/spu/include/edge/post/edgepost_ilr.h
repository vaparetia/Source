/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_POST_ILR_H__
#define __EDGE_POST_ILR_H__

#include <spu_intrinsics.h>
#include "edge/edge_stdint.h"
#include "edge/post/edgepost.h"


extern "C"
{
	/*
	* operates on the entire image
	* input/output must be argb8
	*/
	void edgePostIlr( void* output, void* input, uint32_t width, uint32_t height, const vec_float4* tints, const vec_float4* scales );


	/*
	* operates on the entire image
	* input is FX16
	* output is LUV
	*/
	void edgePostIlr_hdr( void* output, void* input, uint32_t width, uint32_t height, const vec_float4* tints, const vec_float4* scales );

	/*
	* operates on the entire image
	* input/output must be argb8
	*/
	void edgePostVignette( void* output, void* input, uint32_t width, uint32_t height );
	
	/*
	* operates on the entire image
	* input/output must be FX16
	*/
	void edgePostVignetteFX16( void* output, void* input, uint32_t width, uint32_t height );
}



#endif // __EDGE_POST_ILR_H__
