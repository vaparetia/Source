/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_POST_REFC_H__
#define __EDGE_POST_REFC_H__

#include <spu_intrinsics.h>
#include "edge/edge_stdint.h"
#include "edge/post/edgepost.h"


extern "C"
{

	// some *unsupported* reference implementations
	void edgePostDof_C( void* output, const void* input, uint32_t stride, uint32_t count, const vec_float4* tapOffsetTable );
	void edgePostDofPremultiply_C( void* output, const void* colors, const void* fuzziness, uint32_t count );
	void edgePostExtractNearFuzziness_C( void* output, const void* input, uint32_t count, float nearFuzzy, float nearSharp, float maxFuzziness );
	void edgePostExtractFarFuzziness_C( void* output, const void* input, const void* nearFuzziness, uint32_t count, float farSharp, float farFuzzy, float maxFuzziness, float defaultLeakFactor = .4f );
	void edgePostMotionblur_C( void* output, const void* input, const void* motion, uint32_t stride, uint32_t count );

}



#endif // __EDGE_POST_REFC_H__
