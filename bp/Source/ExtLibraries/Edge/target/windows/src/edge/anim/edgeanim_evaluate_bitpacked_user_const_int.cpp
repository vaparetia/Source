/* SCE CONFIDENTIAL
* PlayStation(R)Edge 1.2.0
* Copyright (C) 2010 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#include "edge/anim/edgeanim_structs_ps3.h"
#include "edge/anim/edgeanim_windows.h"

extern "C"
void _edgeAnimEvaluateBitPackedUserConst( uint32_t constCount,
										  const void* constData,
										  const uint16_t* constTable,
										  const uint32_t* packingSpec,
										  float* output )
{
	// Constants
	const __m128i k1 = _mm_set1_epi32( 1 );
	const __m128i k23 = _mm_set1_epi32( 23 );
	const __m128i k32 = _mm_set1_epi32( 32 );
	const __m128i k128 = _mm_set1_epi32( 128 );
	const __m128i kAll = _mm_set1_epi32( -1 );
	const __m128 k1_0f = _mm_set1_ps( 1.0f );
	const __m128i kXXX32 = _mm_srli_si128( k32, 12 );

	// load packing spec
	__m128i spec = _mm_set1_epi32( *packingSpec );
	__m128i specSx = _mm_srli_epi32( spec, 31 );
	__m128i specEx = _mm_srli_epi32( _mm_slli_epi32( spec,  1 ), 28 );
	__m128i specMx = _mm_srli_epi32( _mm_slli_epi32( spec,  5 ), 27 );
	__m128i sumSpecX = _mm_add_epi32( _mm_add_epi32( specSx, specEx ), specMx );
	__m128i keyStride = sumSpecX;

	// spec - float - bias exponent ( 128 - ( 1U << ( numExponentBits - 1 ) )
	__m128i expBiasX = _mm_srli_si128( _mm_sub_epi32( specEx, k1 ), 12 );
	expBiasX = _mm_sll_epi32( k1, expBiasX );
	expBiasX = _mm_sub_epi32( k128, expBiasX );

	// spec - float - mantissa keyshift by 23 - numBitsMantissa
	__m128i shiftMx = _mm_srli_si128( _mm_sub_epi32( k23, specMx ), 12 );

	// spec - fixed point - scale factor
	__m128i scaleFactorZeroX = _mm_cmpeq_epi32( specMx, _mm_setzero_si128() );
	__m128i scaleFactorIntX = _mm_srl_epi32( kAll, _mm_srli_si128( _mm_sub_epi32( k32, specMx ), 12 ) );
	__m128 scaleFactorX = _mm_cvtepi32_ps( scaleFactorIntX );

	// spec - fixed point - invert and divide by zero guard
	__m128 invScaleFactorX = _mm_rcp_ps( scaleFactorX );
	invScaleFactorX = _mm_madd_ps( _mm_nmsub_ps( scaleFactorX, invScaleFactorX, k1_0f ), invScaleFactorX, invScaleFactorX );
	scaleFactorX = _mm_andnot_ps( *(__m128*)&scaleFactorZeroX, invScaleFactorX );

	// spec - select integer if num exponent bits = 0, otherwise float
	__m128i selIntX = _mm_cmpeq_epi32( specEx, _mm_setzero_si128() );

	// spec - spec shifts
	specSx = _mm_srli_si128( specSx, 12 );
	specEx = _mm_srli_si128( specEx, 12 );
	specMx = _mm_srli_si128( specMx, 12 );
	sumSpecX = _mm_srli_si128( sumSpecX, 12 );

	__m128i specSx32 = _mm_sub_epi32( kXXX32, specSx );
	__m128i specEx32 = _mm_sub_epi32( kXXX32, specEx );
	__m128i specMx32 = _mm_sub_epi32( kXXX32, specMx );

	//
	uintptr_t constDataAdr = (uintptr_t) constData;

	__m128i constDataBitOfs;
	constDataBitOfs = _mm_slli_si128( keyStride, 4 );
	constDataBitOfs = _mm_add_epi32( constDataBitOfs, _mm_slli_si128( keyStride, 8 ) );
	constDataBitOfs = _mm_add_epi32( constDataBitOfs, _mm_slli_si128( keyStride, 12 ) );

	for( uint32_t count4 = (constCount + 3) >> 2; count4; count4-- )
	{
		// load the bitpacked keyframes
		__m128i byteOfs = _mm_srli_epi32( constDataBitOfs, 3 );
		__m128i bitOfs = _mm_srli_epi32( _mm_slli_epi32( constDataBitOfs, 29 ), 29 );
		__m128i bitOfs32 = _mm_sub_epi32( k32, bitOfs );

		uintptr_t adr0 = constDataAdr + _MM_EXTRACT_EPI32_X( byteOfs );
		uintptr_t adr1 = constDataAdr + _MM_EXTRACT_EPI32_Y( byteOfs );
		uintptr_t adr2 = constDataAdr + _MM_EXTRACT_EPI32_Z( byteOfs );
		uintptr_t adr3 = constDataAdr + _MM_EXTRACT_EPI32_W( byteOfs );

		__m128i key0 = _mm_loadu_si128( (__m128i*)( adr0 ) );
		__m128i key1 = _mm_loadu_si128( (__m128i*)( adr1 ) );
		__m128i key2 = _mm_loadu_si128( (__m128i*)( adr2 ) );
		__m128i key3 = _mm_loadu_si128( (__m128i*)( adr3 ) );

		key0 = _mm_or_si128( _mm_slli_epi16( key0,  8 ), _mm_srli_epi16( key0,  8 ) );
		key1 = _mm_or_si128( _mm_slli_epi16( key1,  8 ), _mm_srli_epi16( key1,  8 ) );
		key2 = _mm_or_si128( _mm_slli_epi16( key2,  8 ), _mm_srli_epi16( key2,  8 ) );
		key3 = _mm_or_si128( _mm_slli_epi16( key3,  8 ), _mm_srli_epi16( key3,  8 ) );
		key0 = _mm_or_si128( _mm_slli_epi32( key0, 16 ), _mm_srli_epi32( key0, 16 ) );
		key1 = _mm_or_si128( _mm_slli_epi32( key1, 16 ), _mm_srli_epi32( key1, 16 ) );
		key2 = _mm_or_si128( _mm_slli_epi32( key2, 16 ), _mm_srli_epi32( key2, 16 ) );
		key3 = _mm_or_si128( _mm_slli_epi32( key3, 16 ), _mm_srli_epi32( key3, 16 ) );

		// swizzle
		__m128i keyQwx = key0, keyQwy = key1, keyQwz = key2, keyQww = key3;
		_MM_TRANSPOSE4_EPI32( keyQwx, keyQwy, keyQwz, keyQww );

		// shift packed x,y,z into preferred word
		__m128i keyx = _mm_or_si128( _mm_slle_epi32( keyQwx, bitOfs ), _mm_srle_epi32( keyQwy, bitOfs32 ) );

		// extract + mask s,e,m
		__m128i keySx = _mm_srl_epi32( keyx, specSx32 );
		keyx = _mm_sll_epi32( keyx, specSx );
		__m128i keyEx = _mm_srl_epi32( keyx, specEx32 );
		keyx = _mm_sll_epi32( keyx, specEx );
		__m128i keyMx = _mm_srl_epi32( keyx, specMx32 );

		//----------------------------------------------------------------
		// expand as float
		//----------------------------------------------------------------

		// bias exponent
		__m128i keyEx0 = _mm_add_epi32( keyEx, expBiasX );
		keyEx0 = _mm_slli_epi32( keyEx0, 23 );

		// mantissa keyshift
		__m128i keyMx0 = _mm_sll_epi32( keyMx, shiftMx );

		// sign
		__m128i keySx0 = _mm_slli_epi32( keySx, 31 );

		// combine
		__m128i keyFloatX = _mm_or_si128( keySx0, _mm_or_si128( keyEx0, keyMx0 ) );

		//----------------------------------------------------------------
		// expand as signed integer
		//----------------------------------------------------------------

		// sign extend
		__m128i keySignExtendX = _mm_cmpgt_epi32( keySx, _mm_setzero_si128() );
		keySignExtendX = _mm_sll_epi32( keySignExtendX, specMx );

		__m128i keySignedIntLX = _mm_or_si128( keySignExtendX, keyMx );
		__m128 keyIntX = _mm_cvtepi32_ps( keySignedIntLX );

		// scale
		keyIntX = _mm_mul_ps( keyIntX, scaleFactorX );

		//----------------------------------------------------------------
		// select integer if num exponent bits = 0, otherwise float
		//----------------------------------------------------------------
		__m128 outX = _mm_sel_ps( *(__m128*)&keyFloatX, keyIntX, *(__m128*)&selIntX );

		// store
		_mm_store_ss( output + constTable[0], outX );
		_mm_store_ss( output + constTable[1], _mm_shuffle_ps( outX, outX, _MM_SHUFFLE( 1, 1, 1, 1 ) ) );
		_mm_store_ss( output + constTable[2], _mm_shuffle_ps( outX, outX, _MM_SHUFFLE( 2, 2, 2, 2 ) ) );
		_mm_store_ss( output + constTable[3], _mm_shuffle_ps( outX, outX, _MM_SHUFFLE( 3, 3, 3, 3 ) ) );

		constDataBitOfs = _mm_add_epi32( constDataBitOfs, _mm_slli_epi32( keyStride, 2 ) );
		constTable += 4;
	}
}

