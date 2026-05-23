/* SCE CONFIDENTIAL
* PlayStation(R)Edge 1.2.0
* Copyright (C) 2007 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#include <spu_intrinsics.h>

#include "edge/edge_assert.h"
#include "edge/anim/edgeanim_spu.h"

#warning sub-optimal version - using intrinisics implementation instead of asm

/** 
 * @brief   Multiply two arrays of 3x4 (transposed) matrices
 *          SPU intrinisics implementation.
 *
 *          This function is typically used for skinning, to multiply the world-space array 
 *          of matrices by the skinning inverse-bind-pose.
 *
 * @note    A reference C++ implementation of this function is available in
 *          target/reference/src/edge/anim/edgeanim_multiply_matrices_3x4.cpp
 *
 * @note    This function is not called unless you change the makefile which 
 *          uses asm implementation by default.
 *
 * @param   outputJoints    Pointer to output joint transform array.
 *                          Must be aligned to 16 bytes.
 *                          Due to the simd nature of the processing, 
 *                          the outputJoints array must be a multiple of 4 entries in size. 
 * @param   inputMatrices   Pointer to input matrix array.
 *                          Must be aligned to 16 bytes.
 * @param   count           Joint/matrices count. 
 */

extern "C"
void edgeAnimMultiplyMatrices3x4(void* outputMatrices, 
								 const void* leftMatrices, 
								 const void* rightMatrices, 
								 unsigned int count)
{
	EDGE_ASSERT(EDGE_IS_ALIGNED(outputMatrices, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(leftMatrices, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(rightMatrices, 16U));
	EDGE_ASSERT(count);

	const vec_float4 w1 = {0, 0, 0, 1};
	const vec_uchar16 shufAAAA = {0x00,0x01,0x02,0x03,0x00,0x01,0x02,0x03,0x00,0x01,0x02,0x03,0x00,0x01,0x02,0x03};
	const vec_uchar16 shufBBBB = {0x04,0x05,0x06,0x07,0x04,0x05,0x06,0x07,0x04,0x05,0x06,0x07,0x04,0x05,0x06,0x07};
	const vec_uchar16 shufCCCC = {0x08,0x09,0x0a,0x0b,0x08,0x09,0x0a,0x0b,0x08,0x09,0x0a,0x0b,0x08,0x09,0x0a,0x0b};

	vec_float4* __restrict__ output = (vec_float4* __restrict__) outputMatrices;
	const vec_float4* __restrict__ leftm = (const vec_float4* __restrict__) leftMatrices;
	const vec_float4* __restrict__ rightm = (const vec_float4* __restrict__) rightMatrices;

	for (; count; count--) {

		// load
		const vec_float4 l0 = leftm[0];
		const vec_float4 l1 = leftm[1];
		const vec_float4 l2 = leftm[2];
		const vec_float4 r0 = rightm[0];
		const vec_float4 r1 = rightm[1];
		const vec_float4 r2 = rightm[2];

		// splat left components
		const vec_float4 l0x = spu_shuffle(l0, l0, shufAAAA);
		const vec_float4 l0y = spu_shuffle(l0, l0, shufBBBB);
		const vec_float4 l0z = spu_shuffle(l0, l0, shufCCCC);
		const vec_float4 l1x = spu_shuffle(l1, l1, shufAAAA);
		const vec_float4 l1y = spu_shuffle(l1, l1, shufBBBB);
		const vec_float4 l1z = spu_shuffle(l1, l1, shufCCCC);
		const vec_float4 l2x = spu_shuffle(l2, l2, shufAAAA);
		const vec_float4 l2y = spu_shuffle(l2, l2, shufBBBB);
		const vec_float4 l2z = spu_shuffle(l2, l2, shufCCCC);

		// row 0
		vec_float4 out0 = spu_mul(r0, l0x);
		out0 = spu_madd(r1, l0y, out0);
		out0 = spu_madd(r2, l0z, out0);
		out0 = spu_madd(w1, l0, out0);

		// row 1
		vec_float4 out1 = spu_mul(r0, l1x);
		out1 = spu_madd(r1, l1y, out1);
		out1 = spu_madd(r2, l1z, out1);
		out1 = spu_madd(w1, l1, out1);

		// row 2
		vec_float4 out2 = spu_mul(r0, l2x);
		out2 = spu_madd(r1, l2y, out2);
		out2 = spu_madd(r2, l2z, out2);
		out2 = spu_madd(w1, l2, out2);

		// store
		output[0] = out0;
		output[1] = out1;
		output[2] = out2;

		output += 3;
		leftm += 3;
		rightm += 3;
	}
}
