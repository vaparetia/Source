/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_ANIM_SSE_H__
#define __EDGE_ANIM_SSE_H__

#include <intrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>

// Bit Selection
#define _mm_sel_ps( a, b, mask )	( _mm_or_ps( _mm_and_ps( (mask), (b) ), _mm_andnot_ps( (mask), (a) ) ) )
#define _mm_sel_si128( a, b, mask )	( _mm_or_si128( _mm_and_si128( (mask), (b) ), _mm_andnot_si128( (mask), (a) ) ) )

// Multiply-Accumulate
#define _mm_madd_ps( a, b, c )		( _mm_add_ps( _mm_mul_ps( (a), (b) ), (c) ) )
#define _mm_msub_ps( a, b, c )		( _mm_sub_ps( _mm_mul_ps( (a), (b) ), (c) ) )
#define _mm_nmadd_ps( a, b, c )		( _mm_sub_ps( _mm_setzero_ps(), _mm_add_ps( _mm_mul_ps( (a), (b) ), (c) ) ) )
#define _mm_nmsub_ps( a, b, c )		( _mm_sub_ps( (c), _mm_mul_ps( (a), (b) ) ) )

// Packing
#define _mm_setu8_epi32( a, b, c, d )	( _mm_unpacklo_epi16( _mm_unpacklo_epi8( _mm_set1_epi32( ( ((d) << 24) | ((c) << 16) | ((b) << 8) | (a)) ), _mm_setzero_si128() ), _mm_setzero_si128() ) )
#define _mm_unpackx_ps(x0,x1,x2,x3)		( _mm_shuffle_ps( _mm_shuffle_ps( (x0), (x1), 0x44 ), _mm_shuffle_ps( (x2), (x3), 0x44 ), 0x88 ) )
#define _mm_unpackx_epi32(x0,x1,x2,x3)	( _mm_unpacklo_epi64( _mm_unpacklo_epi32( (x0), (x1) ), _mm_unpacklo_epi32( (x2), (x3) ) ) )

// Shift Values
#define _MM_SHIFT_EPI32_X(x)	_mm_srli_si128( _mm_slli_si128( x, 12 ), 12 )
#define _MM_SHIFT_EPI32_Y(x)	_mm_srli_si128( _mm_slli_si128( x,  8 ), 12 )
#define _MM_SHIFT_EPI32_Z(x)	_mm_srli_si128( _mm_slli_si128( x,  4 ), 12 )
#define _MM_SHIFT_EPI32_W(x)	_mm_srli_si128( x, 12 )

// Logical Shift by Element
__forceinline __m128i _mm_srle_epi32( __m128i v, __m128i s )
{
	return _mm_unpackx_epi32(
		_mm_srl_epi32( _mm_shuffle_epi32( v, _MM_SHUFFLE(0,0,0,0) ), _MM_SHIFT_EPI32_X( s ) ),
		_mm_srl_epi32( _mm_shuffle_epi32( v, _MM_SHUFFLE(1,1,1,1) ), _MM_SHIFT_EPI32_Y( s ) ),
		_mm_srl_epi32( _mm_shuffle_epi32( v, _MM_SHUFFLE(2,2,2,2) ), _MM_SHIFT_EPI32_Z( s ) ),
		_mm_srl_epi32( _mm_shuffle_epi32( v, _MM_SHUFFLE(3,3,3,3) ), _MM_SHIFT_EPI32_W( s ) )
		);
}

// Logical Shift by Element
__forceinline __m128i _mm_slle_epi32( __m128i v, __m128i s )
{
	return _mm_unpackx_epi32(
		_mm_sll_epi32( _mm_shuffle_epi32( v, _MM_SHUFFLE(0,0,0,0) ), _MM_SHIFT_EPI32_X( s ) ),
		_mm_sll_epi32( _mm_shuffle_epi32( v, _MM_SHUFFLE(1,1,1,1) ), _MM_SHIFT_EPI32_Y( s ) ),
		_mm_sll_epi32( _mm_shuffle_epi32( v, _MM_SHUFFLE(2,2,2,2) ), _MM_SHIFT_EPI32_Z( s ) ),
		_mm_sll_epi32( _mm_shuffle_epi32( v, _MM_SHUFFLE(3,3,3,3) ), _MM_SHIFT_EPI32_W( s ) )
		);
}

// Extractions
#define _MM_EXTRACT_EPI32_X(x)	_mm_cvtsi128_si32( x )
#define _MM_EXTRACT_EPI32_Y(x)	_mm_cvtsi128_si32( _mm_srli_si128( x,  4 ) )
#define _MM_EXTRACT_EPI32_Z(x)	_mm_cvtsi128_si32( _mm_srli_si128( x,  8 ) )
#define _MM_EXTRACT_EPI32_W(x)	_mm_cvtsi128_si32( _mm_srli_si128( x, 12 ) )

#define _MM_TRANSPOSE4_EPI32(row0, row1, row2, row3) {    \
	__m128i tmp3, tmp2, tmp1, tmp0;                       \
	\
	tmp0   = _mm_unpacklo_epi32((row0), (row1));          \
	tmp1   = _mm_unpacklo_epi32((row2), (row3));          \
	tmp2   = _mm_unpackhi_epi32((row0), (row1));          \
	tmp3   = _mm_unpackhi_epi32((row2), (row3));          \
	\
	(row0) = _mm_unpacklo_epi64(tmp0, tmp1);              \
	(row1) = _mm_unpackhi_epi64(tmp0, tmp1);              \
	(row2) = _mm_unpacklo_epi64(tmp2, tmp3);              \
	(row3) = _mm_unpackhi_epi64(tmp2, tmp3);              \
}

// get index of first set bit in each word
#pragma intrinsic(_BitScanReverse)
__forceinline __m128i _mm_bsr_epi32( __m128i bits )
{
	union {
		__m128i m128i;
		unsigned long ul[4];
	} v, i;

	v.m128i = bits;
	_BitScanReverse( &i.ul[0], v.ul[0] );
	_BitScanReverse( &i.ul[1], v.ul[1] );
	_BitScanReverse( &i.ul[2], v.ul[2] );
	_BitScanReverse( &i.ul[3], v.ul[3] );

	__m128i pos = i.m128i;
	return pos;
}

// count set bits in each word
__forceinline __m128i _mm_popcnt_epi32( __m128i bits )
{
	__m128i count;
	count = _mm_sub_epi32( bits, _mm_and_si128( _mm_srli_epi32( bits, 1 ), _mm_set1_epi32( 0x55555555 ) ) );
	count = _mm_add_epi32( _mm_and_si128( _mm_srli_epi32( count,  2 ), _mm_set1_epi32( 0x33333333 ) ), _mm_and_si128( count, _mm_set1_epi32( 0x33333333 ) ) );
	count = _mm_and_si128( _mm_add_epi32( _mm_srli_epi32( count,  4 ), count ), _mm_set1_epi32( 0x0F0F0F0F ) );
	count = _mm_and_si128( _mm_add_epi32( _mm_srli_epi32( count,  8 ), count ), _mm_set1_epi32( 0x00FF00FF ) );
	count = _mm_and_si128( _mm_add_epi32( _mm_srli_epi32( count, 16 ), count ), _mm_set1_epi32( 0x0000FFFF ) );
	return count;
}

#endif // __EDGE_ANIM_SSE_H__
