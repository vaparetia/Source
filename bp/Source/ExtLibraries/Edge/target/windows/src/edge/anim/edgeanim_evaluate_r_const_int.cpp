/* SCE CONFIDENTIAL
* PlayStation(R)Edge 1.2.0
* Copyright (C) 2010 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#include "edge/anim/edgeanim_structs_ps3.h"
#include "edge/anim/edgeanim_windows.h"

extern "C"
void _edgeAnimEvaluateRConst( uint32_t constCount,
							  const void* constData,
							  const uint16_t* constTable,
							  __m128* output )
{
	const float kSqrt2 = 1.41421356237309504880168872420970f;
	const float kInvSqrt2 = 0.70710678118654752440084436210485f;

	// Constants
	const __m128i* constRData = (__m128i*) constData;
	const __m128 k1_0f = _mm_set1_ps( 1.0f );

	// Constants: Quaternion Fixed -> Floating Point
	const __m128 kQuatOffset = _mm_set1_ps( -kInvSqrt2 );
	const __m128 kQuatScale = _mm_set1_ps( kSqrt2 / ( ( 1 << 15 ) - 1 ) );

	// Constants: Element Orders
	const __m128i kDABC = _mm_set1_epi32( 0 );
	const __m128i kADBC = _mm_set1_epi32( 1 );
	const __m128i kABDC = _mm_set1_epi32( 2 );
	const __m128i kABCD = _mm_set1_epi32( 3 );

	for( uint32_t count8 = (constCount + 7) >> 3; count8; count8-- )
	{
		// Decompress the 8 rotation quaternions to float 4
		// Compression is 'smallest 3' (15.15.15.2)
		__m128i qw0 = constRData[0];
		__m128i qw1 = constRData[1];
		__m128i qw2 = constRData[2];

		// Unpack 15-bit components
		__m128i a0i, a4i;
		a0i = _mm_srli_si128( _mm_slli_si128( qw0, 14 ), 14 ); // a0 qw0.u8[0,1]
		a0i = _mm_or_si128( a0i, _mm_slli_si128( _mm_srli_si128( _mm_slli_si128( qw0,  8 ), 14 ),  4 ) ); // a1 qw0.u8[6,7]
		a0i = _mm_or_si128( a0i, _mm_srli_si128( _mm_srli_si128( _mm_slli_si128( qw0,  2 ), 14 ),  8 ) ); // a2 qw0.u8[12,13]
		a0i = _mm_or_si128( a0i, _mm_slli_si128( _mm_srli_si128( _mm_slli_si128( qw1, 12 ), 14 ), 12 ) ); // a3 qw1.u8[2,3]

		a4i = _mm_srli_si128( _mm_slli_si128( qw1,  6 ), 14 ); // a4 qw1.u8[8,9]
		a4i = _mm_or_si128( a4i, _mm_slli_si128( _mm_srli_si128( qw1, 14 ),  4 ) ); // a5 qw1.u8[14,15]
		a4i = _mm_or_si128( a4i, _mm_slli_si128( _mm_srli_si128( _mm_slli_si128( qw2, 10 ), 14 ),  8 ) ); // a6 qw2.u8[4,5]
		a4i = _mm_or_si128( a4i, _mm_slli_si128( _mm_srli_si128( _mm_slli_si128( qw2,  4 ), 14 ), 12 ) ); // a7 qw2.u8[10,11]

		__m128i bc0i, bc4i;
		bc0i = _mm_srli_si128( _mm_slli_si128( qw0, 10 ), 12 ); // b0 qw0.u8[2,3,4,5]
		bc0i = _mm_or_si128( bc0i, _mm_slli_si128( _mm_srli_si128( _mm_slli_si128( qw0, 4 ), 12 ),  4 ) ); // b1 qw0.u8[8,9,10,11]
		bc0i = _mm_or_si128( bc0i, _mm_slli_si128( _mm_or_si128( _mm_srli_si128( qw0, 14 ), _mm_srli_si128( _mm_slli_si128( qw1, 14 ), 12 ) ), 8 ) ); // b2 qw0.u8[14,15] qw1.u8[0,1]
		bc0i = _mm_or_si128( bc0i, _mm_slli_si128( _mm_srli_si128( _mm_slli_si128( qw1, 8 ), 12 ), 12 ) ); // b3 qw1.u8[4,5,6,7]

		bc4i = _mm_srli_si128( _mm_slli_si128( qw1,  2 ), 12 ); // b4 qw1.u8[10,11,12,13]
		bc4i = _mm_or_si128( bc4i, _mm_srli_si128( _mm_slli_si128( qw2, 12 ),  8 ) ); // b5 qw2.u8[0,1,2,3]
		bc4i = _mm_or_si128( bc4i, _mm_slli_si128( _mm_srli_si128( _mm_slli_si128( qw2, 6 ), 12 ), 8 ) ); // b6 qw2.u8[6,7,8,9]
		bc4i = _mm_or_si128( bc4i, _mm_slli_si128( _mm_srli_si128( qw2, 12 ), 12 ) ); // b7 qw2.u8[12,13,14,15]

		// endian swap
		a0i = _mm_or_si128( _mm_slli_epi16( a0i, 8 ), _mm_srli_epi16( a0i, 8 ) );
		a4i = _mm_or_si128( _mm_slli_epi16( a4i, 8 ), _mm_srli_epi16( a4i, 8 ) );
		bc0i = _mm_or_si128( _mm_slli_epi16( bc0i, 8 ),  _mm_srli_epi16( bc0i, 8 ) );
		bc4i = _mm_or_si128( _mm_slli_epi16( bc4i, 8 ),  _mm_srli_epi16( bc4i, 8 ) );
		bc0i = _mm_or_si128( _mm_slli_epi32( bc0i, 16 ), _mm_srli_epi32( bc0i, 16 ) );
		bc4i = _mm_or_si128( _mm_slli_epi32( bc4i, 16 ), _mm_srli_epi32( bc4i, 16 ) );

		// get b+c components
		__m128i b0i = _mm_srli_epi32( bc0i, 17 );
		__m128i b4i = _mm_srli_epi32( bc4i, 17 );
		__m128i c0i = _mm_srli_epi32( _mm_slli_epi32( bc0i, 15 ), 17 );
		__m128i c4i = _mm_srli_epi32( _mm_slli_epi32( bc4i, 15 ), 17 );

		// convert to floating point
		__m128 a0 = _mm_madd_ps( _mm_cvtepi32_ps( a0i ), kQuatScale, kQuatOffset );
		__m128 a4 = _mm_madd_ps( _mm_cvtepi32_ps( a4i ), kQuatScale, kQuatOffset );
		__m128 b0 = _mm_madd_ps( _mm_cvtepi32_ps( b0i ), kQuatScale, kQuatOffset );
		__m128 b4 = _mm_madd_ps( _mm_cvtepi32_ps( b4i ), kQuatScale, kQuatOffset );
		__m128 c0 = _mm_madd_ps( _mm_cvtepi32_ps( c0i ), kQuatScale, kQuatOffset );
		__m128 c4 = _mm_madd_ps( _mm_cvtepi32_ps( c4i ), kQuatScale, kQuatOffset );
													   
		// Derived value, d = sqrt(1 - a^2 - b^2 - c^2)
		__m128 d0sqr0 = _mm_nmsub_ps( a0, a0, k1_0f  );
		__m128 d4sqr0 = _mm_nmsub_ps( a4, a4, k1_0f  );
		__m128 d0sqr1 = _mm_nmsub_ps( b0, b0, d0sqr0 );
		__m128 d4sqr1 = _mm_nmsub_ps( b4, b4, d4sqr0 );
		__m128 d0sqr2 = _mm_nmsub_ps( c0, c0, d0sqr1 );
		__m128 d4sqr2 = _mm_nmsub_ps( c4, c4, d4sqr1 );
		__m128 d0 = _mm_sqrt_ps( d0sqr2 );
		__m128 d4 = _mm_sqrt_ps( d4sqr2 );

		// Get 2-bit derived value index
		__m128i idx0 = _mm_srli_epi32( _mm_slli_epi32( bc0i, 30 ), 30 ); // 2-bit mask
		__m128i idx4 = _mm_srli_epi32( _mm_slli_epi32( bc4i, 30 ), 30 ); // 2-bit mask

		// Now shuffle components to insert the derived value.
		__m128i selDABC0 = _mm_cmpeq_epi32( idx0, kDABC );
		__m128i selDABC4 = _mm_cmpeq_epi32( idx4, kDABC );

		__m128i selADBC0 = _mm_cmpeq_epi32( idx0, kADBC );
		__m128i selADBC4 = _mm_cmpeq_epi32( idx4, kADBC );

		__m128i selABDC0 = _mm_cmpeq_epi32( idx0, kABDC );
		__m128i selABDC4 = _mm_cmpeq_epi32( idx4, kABDC );

		__m128i selABCD0 = _mm_cmpeq_epi32( idx0, kABCD );
		__m128i selABCD4 = _mm_cmpeq_epi32( idx4, kABCD );

		__m128 q0x, q0y, q0z, q0w;
		__m128 q4x, q4y, q4z, q4w;

		q0x = _mm_sel_ps( a0, d0, *(__m128*)&selDABC0 );
		q4x = _mm_sel_ps( a4, d4, *(__m128*)&selDABC4 );

		q0y = _mm_sel_ps( _mm_sel_ps( b0, a0, *(__m128*)&selDABC0 ), d0, *(__m128*)&selADBC0 );
		q4y = _mm_sel_ps( _mm_sel_ps( b4, a4, *(__m128*)&selDABC4 ), d4, *(__m128*)&selADBC4 );

		q0z = _mm_sel_ps( _mm_sel_ps( b0, c0, *(__m128*)&selABCD0 ), d0, *(__m128*)&selABDC0 );
		q4z = _mm_sel_ps( _mm_sel_ps( b4, c4, *(__m128*)&selABCD4 ), d4, *(__m128*)&selABDC4 );

		q0w = _mm_sel_ps( c0, d0, *(__m128*)&selABCD0 );
		q4w = _mm_sel_ps( c4, d4, *(__m128*)&selABCD4 );

		// Deswizzle
		__m128 q0 = q0x, q1 = q0y, q2 = q0z, q3 = q0w;
		__m128 q4 = q4x, q5 = q4y, q6 = q4z, q7 = q4w;
		_MM_TRANSPOSE4_PS( q0, q1, q2, q3 );
		_MM_TRANSPOSE4_PS( q4, q5, q6, q7 );

		// Write results (x4)
		output[ constTable[0] * 3 ] = q0;
		output[ constTable[1] * 3 ] = q1;
		output[ constTable[2] * 3 ] = q2;
		output[ constTable[3] * 3 ] = q3;
		output[ constTable[4] * 3 ] = q4;
		output[ constTable[5] * 3 ] = q5;
		output[ constTable[6] * 3 ] = q6;
		output[ constTable[7] * 3 ] = q7;

		// Advance pointers
		constTable += 8;
		constRData += 3;
	}
}
