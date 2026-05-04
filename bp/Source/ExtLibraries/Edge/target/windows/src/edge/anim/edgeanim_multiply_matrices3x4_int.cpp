/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/anim/edgeanim_structs_ps3.h"
#include "edge/anim/edgeanim_windows.h"

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

	const __m128 w = _mm_setr_ps( 0.0f, 0.0f, 0.0f, 1.0f );

	__m128* __restrict output = (__m128* __restrict) outputMatrices;
	const __m128* __restrict left = (const __m128* __restrict) leftMatrices;
	const __m128* __restrict right = (const __m128* __restrict) rightMatrices;

    for( ; count; count-- )
	{
		// load
		const __m128 r0 = right[0];
		const __m128 r1 = right[1];
		const __m128 r2 = right[2];
		const __m128 l0 = left[0];
		const __m128 l1 = left[1];
		const __m128 l2 = left[2];

		// row 0
		__m128 out0 = _mm_mul_ps( r0, _mm_shuffle_ps( l0, l0, _MM_SHUFFLE( 0, 0, 0, 0 ) ) );
		out0 = _mm_madd_ps( r1, _mm_shuffle_ps( l0, l0, _MM_SHUFFLE( 1, 1, 1, 1 ) ), out0 );
		out0 = _mm_madd_ps( r2, _mm_shuffle_ps( l0, l0, _MM_SHUFFLE( 2, 2, 2, 2 ) ), out0 );
		out0 = _mm_madd_ps( w, l0, out0 );
		output[0] = out0;
																																					 
		// row 1
		__m128 out1 = _mm_mul_ps( r0, _mm_shuffle_ps( l1, l1, _MM_SHUFFLE( 0, 0, 0, 0 ) ) );
		out1 = _mm_madd_ps( r1, _mm_shuffle_ps( l1, l1, _MM_SHUFFLE( 1, 1, 1, 1 ) ), out1 );
		out1 = _mm_madd_ps( r2, _mm_shuffle_ps( l1, l1, _MM_SHUFFLE( 2, 2, 2, 2 ) ), out1 );
		out1 = _mm_madd_ps( w, l1, out1 );
		output[1] = out1;

		// row 2
		__m128 out2 = _mm_mul_ps( r0, _mm_shuffle_ps( l2, l2, _MM_SHUFFLE( 0, 0, 0, 0 ) ) );
		out2 = _mm_madd_ps( r1, _mm_shuffle_ps( l2, l2, _MM_SHUFFLE( 1, 1, 1, 1 ) ), out2 );
		out2 = _mm_madd_ps( r2, _mm_shuffle_ps( l2, l2, _MM_SHUFFLE( 2, 2, 2, 2 ) ), out2 );
		out2 = _mm_madd_ps( w, l2, out2 );
		output[2] = out2;

		output += 3;
		right += 3;
		left += 3;
    }
}
