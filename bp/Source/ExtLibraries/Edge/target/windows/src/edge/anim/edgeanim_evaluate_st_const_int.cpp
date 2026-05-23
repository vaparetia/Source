/* SCE CONFIDENTIAL
* PlayStation(R)Edge 1.2.0
* Copyright (C) 2010 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#include "edge/anim/edgeanim_structs_ps3.h"
#include "edge/anim/edgeanim_windows.h"

extern "C"
void _edgeAnimEvaluateSTConst( uint32_t constCount,
							   const void* constData,
							   const uint16_t* constTable,
							   __m128* output )
{
	const __m128 k1_0f = _mm_set1_ps( 1.0f );

	// Constant Channels
	const __m128* constVData = (__m128*) constData;

	for( uint32_t count4 = (constCount + 3) >> 2; count4; count4-- )
	{
		// load packed data
		__m128 ja = constVData[0];	// X1 Z0 Y0 X0
		__m128 jb = constVData[1];	// Y2 X2 Z1 Y1
		__m128 jc = constVData[2];	// Z3 Y3 X3 Z2

		// shuffle-unpack & set w=1.0f
		__m128 j0, j1, j2, j3;
		j0 = _mm_shuffle_ps( ja, k1_0f, _MM_SHUFFLE( 3, 3, 2, 2 ) );	// 1.0 1.0 Z0 Z0
		j0 = _mm_shuffle_ps( j0, ja, _MM_SHUFFLE( 0, 1, 0, 3 ) );		// X0 Y0 Z0 1.0
		j0 = _mm_shuffle_ps( j0, j0, _MM_SHUFFLE( 0, 1, 2, 3 ) );		// 1.0 Z0 Y0 X0

		j1 = _mm_shuffle_ps( ja, k1_0f, _MM_SHUFFLE( 3, 3, 3, 3 ) );	// 1.0 1.0 X1 X1
		j1 = _mm_shuffle_ps( j1, jb, _MM_SHUFFLE( 0, 1, 0, 3 ) );		// Y1 Z1 X1 1.0
		j1 = _mm_shuffle_ps( j1, j1, _MM_SHUFFLE( 0, 2, 3, 1 ) );		// 1.0 Z1 Y1 X1

		j2 = _mm_shuffle_ps( jc, k1_0f, _MM_SHUFFLE( 3, 3, 0, 0 ) );	// 1.0 1.0 Z2 Z2
		j2 = _mm_shuffle_ps( j2, jb, _MM_SHUFFLE( 2, 3, 0, 3 ) );		// X2 Y2 Z2 1.0
		j2 = _mm_shuffle_ps( j2, j2, _MM_SHUFFLE( 0, 1, 2, 3 ) );		// 1.0 Z2 Y2 X2

		j3 = _mm_shuffle_ps( jc, k1_0f, _MM_SHUFFLE( 3, 3, 3, 3 ) );	// 1.0 1.0 Z3 Z3
		j3 = _mm_shuffle_ps( j3, jc, _MM_SHUFFLE( 1, 2, 0, 3 ) );		// X3 Y3 Z3 1.0
		j3 = _mm_shuffle_ps( j3, j3, _MM_SHUFFLE( 0, 1, 2, 3 ) );		// 1.0 Z3 Y3 X3

		// write out
		output[ constTable[0] * 3 ] = j0;
		output[ constTable[1] * 3 ] = j1;
		output[ constTable[2] * 3 ] = j2;
		output[ constTable[3] * 3 ] = j3;

		// Advance pointers
		constTable += 4;
		constVData += 3;
	}
}
