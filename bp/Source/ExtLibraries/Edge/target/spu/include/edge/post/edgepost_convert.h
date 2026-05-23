/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_POST_CONVERT_H__
#define __EDGE_POST_CONVERT_H__

#include <spu_intrinsics.h>
#include "edge/edge_stdint.h"
#include "edge/post/edgepost.h"

/*
	Summary of supported pixel formats :

	* argb8, it's the common 32 bit format compatible with RSX
	* floats4, it's four expanded floating points
	* fx16, it's 16 bit per channel, each channel is a fixed point 0:5:11
	* fp16, it's the common FP16 compatible with RSX
	* Log(L)uv, it's 32 bit, first 2 bytes are Log of luminance stored as 8:8 fixed point, last two are normalized 0..1 chroma values
	* FP16Luv, it's 32 bit, first 2 bytes are luminance stored as one FP16, last two are normalized 0..1 chroma values
	* Luv, it's 32 bit, first 2 bytes are luminance stored as fixed point 0:5:11, last two are normalized 0..1 chroma values

*/

extern "C"
{

	// convert a depth buffer ( rsx format) to linear floating point 0..1
	void edgePostExtractDepth( void* output, const void* input, uint32_t count, float near, float far );

	// convert single channel float to argb gray scale
	void edgePostFloatToGrayscale( void* output, const void* input, uint32_t count );

	// convert argb to fp32 x 4
	void edgePostArgb8ToFloats( void* output, const void* input, uint32_t count );

	// convert fp32 x 4 to argb
	void edgePostFloatsToArgb8( void* output, const void* input, uint32_t count );

	// convert 4 x Fixed 0:5:11 to fp32 x 4
	void edgePostFX16ToFloats( void* output, const void* input, uint32_t count );

	// convert 4 x Fixed 0:5:11 to argb 32 bit
	void edgePostFX16ToArgb8( void* output, const void* input, uint32_t count );
	

	//////////////////////////////////////////////////////////////////////////
	//	FP16

	// convert lo fp16 of each 32bit input word to a single channel floating point
	void edgePostFP16LoToFloats( void* output, const void* input, uint32_t count );

	// convert hi fp16 of each 32bit input word to a single channel floating point
	void edgePostFP16hiToFloats( void* output, const void* input, uint32_t count );

	// convert 4 x fp16 to 4 x floats
	void edgePostFP16ToFloats( void* output, const void* input, uint32_t count );

	// convert 4 x floats to 4 x fp16
	void edgePostFloatsToFP16( void* output, const void* input, uint32_t count );

	// convert 4 x fx16 to 4 x fp16
	void edgePostFX16ToFP16( void* output, const void* input, uint32_t count );


	//////////////////////////////////////////////////////////////////////////
	//	Log(L)uv

	// convert Log(L)uv to fp32 x 4
	void edgePostLogLuvToFloats( void* output, const void* input, uint32_t count );

	// convert Log(L)uv to 4 x Fixed 0:5:11
	void edgePostLogLuvToFX16( void* output, const void* input, uint32_t count );

	// convert Log(L)uv to argb 32 bit
	void edgePostLogLuvToArgb( void* output, const void* input, uint32_t count );

	// convert fp32 x 4 to Log(L)uv
	void edgePostFloatsToLogLuv( void* output, const void* input, uint32_t count );

	// convert 4 x Fixed 0:5:11 to Log(L)uv
	void edgePostFX16ToLogLuv( void* output, const void* input, uint32_t count );


	//////////////////////////////////////////////////////////////////////////
	//	Luv
	
	// convert fp32 x 4 to Luv
	void edgePostFloatsToLuv( void* output, const void* input, uint32_t count );
	
	// convert 4 x Fixed 0:5:11 to Luv
	void edgePostFX16ToLuv( void* output, const void* input, uint32_t count );
	
	// convert Luv to fp32 x 4
	void edgePostLuvToFloats( void* output, const void* input, uint32_t count );
	
	// convert Luv to 4 x Fixed 0:5:11
	void edgePostLuvToFX16( void* output, const void* input, uint32_t count );
	
	// convert Luv to argb 32 bit
	void edgePostLuvToArgb( void* output, const void* input, uint32_t count );
	

	//////////////////////////////////////////////////////////////////////////
	//	FP16Luv

	// convert FP16(L)uv to fp32 x 4
	void edgePostFP16LuvToFloats( void* output, const void* input, uint32_t count );

	// convert FP16(L)uv to 4 x Fixed 0:5:11
	void edgePostFP16LuvToFX16( void* output, const void* input, uint32_t count );

	// convert fp32 x 4 to FP16(L)uv
	void edgePostFloatsToFP16Luv( void* output, const void* input, uint32_t count );

	// convert 4 x Fixed 0:5:11 to FP16(L)uv
	void edgePostFX16ToFP16Luv( void* output, const void* input, uint32_t count );
}



#endif // __EDGE_POST_CONVERT_H__
