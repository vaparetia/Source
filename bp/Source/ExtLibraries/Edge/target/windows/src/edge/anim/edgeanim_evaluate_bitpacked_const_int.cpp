/* SCE CONFIDENTIAL
* PlayStation(R)Edge 1.2.0
* Copyright (C) 2010 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#include "edge/anim/edgeanim_structs_ps3.h"
#include "edge/anim/edgeanim_windows.h"

extern "C"
void _edgeAnimEvaluateBitPackedConst( uint32_t constCount,
									  const void* constData,
									  const uint16_t* constTable,
									  const uint32_t* packingSpec,
									  __m128* output,
									  bool isRotation )
{
	// Constants
	const __m128i k1 = _mm_set1_epi32( 1 );
	const __m128i k23 = _mm_set1_epi32( 23 );
	const __m128i k32 = _mm_set1_epi32( 32 );
	const __m128i k128 = _mm_set1_epi32( 128 );
	const __m128i kAll = _mm_set1_epi32( -1 );
	const __m128 k1_0f = _mm_set1_ps( 1.0f );
	const __m128i kXXX32 = _mm_srli_si128( k32, 12 );

	// Constants: quaternion orders
	const __m128i kDABC = _mm_set1_epi32( 0 );
	const __m128i kADBC = _mm_set1_epi32( 1 );
	const __m128i kABDC = _mm_set1_epi32( 2 );
	const __m128i kABCD = _mm_set1_epi32( 3 );

	// load packing spec
	__m128i spec = _mm_set1_epi32( *packingSpec );
	__m128i specIdx = _mm_srli_epi32( _mm_slli_epi32( spec, 30 ), 30 );
	__m128i specSx = _mm_srli_epi32( spec, 31 );
	__m128i specSy = _mm_srli_epi32( _mm_slli_epi32( spec, 10 ), 31 );
	__m128i specSz = _mm_srli_epi32( _mm_slli_epi32( spec, 20 ), 31 );
	__m128i specEx = _mm_srli_epi32( _mm_slli_epi32( spec,  1 ), 28 );
	__m128i specEy = _mm_srli_epi32( _mm_slli_epi32( spec, 11 ), 28 );
	__m128i specEz = _mm_srli_epi32( _mm_slli_epi32( spec, 21 ), 28 );
	__m128i specMx = _mm_srli_epi32( _mm_slli_epi32( spec,  5 ), 27 );
	__m128i specMy = _mm_srli_epi32( _mm_slli_epi32( spec, 15 ), 27 );
	__m128i specMz = _mm_srli_epi32( _mm_slli_epi32( spec, 25 ), 27 );
	__m128i sumSpecX = _mm_add_epi32( _mm_add_epi32( specSx, specEx ), specMx );
	__m128i sumSpecY = _mm_add_epi32( _mm_add_epi32( specSy, specEy ), specMy );
	__m128i sumSpecZ = _mm_add_epi32( _mm_add_epi32( specSz, specEz ), specMz );
	__m128i sumSpecX32 = _mm_sub_epi32( k32, sumSpecX );
	__m128i sumSpecY32 = _mm_sub_epi32( k32, sumSpecY );
	__m128i keyStride = _mm_add_epi32( _mm_add_epi32( sumSpecX, sumSpecY ), sumSpecZ );

	// spec - float - bias exponent ( 128 - ( 1U << ( numExponentBits - 1 ) )
	__m128i expBiasX = _mm_srli_si128( _mm_sub_epi32( specEx, k1 ), 12 );
	__m128i expBiasY = _mm_srli_si128( _mm_sub_epi32( specEy, k1 ), 12 );
	__m128i expBiasZ = _mm_srli_si128( _mm_sub_epi32( specEz, k1 ), 12 );
	expBiasX = _mm_sll_epi32( k1, expBiasX );
	expBiasY = _mm_sll_epi32( k1, expBiasY );
	expBiasZ = _mm_sll_epi32( k1, expBiasZ );
	expBiasX = _mm_sub_epi32( k128, expBiasX );
	expBiasY = _mm_sub_epi32( k128, expBiasY );
	expBiasZ = _mm_sub_epi32( k128, expBiasZ );

	// spec - float - mantissa keyshift by 23 - numBitsMantissa
	__m128i shiftMx = _mm_srli_si128( _mm_sub_epi32( k23, specMx ), 12 );
	__m128i shiftMy = _mm_srli_si128( _mm_sub_epi32( k23, specMy ), 12 );
	__m128i shiftMz = _mm_srli_si128( _mm_sub_epi32( k23, specMz ), 12 );

	// spec - fixed point - scale factor
	__m128i scaleFactorZeroX = _mm_cmpeq_epi32( specMx, _mm_setzero_si128() );
	__m128i scaleFactorZeroY = _mm_cmpeq_epi32( specMy, _mm_setzero_si128() );
	__m128i scaleFactorZeroZ = _mm_cmpeq_epi32( specMz, _mm_setzero_si128() );
	__m128i scaleFactorIntX = _mm_srl_epi32( kAll, _mm_srli_si128( _mm_sub_epi32( k32, specMx ), 12 ) );
	__m128i scaleFactorIntY = _mm_srl_epi32( kAll, _mm_srli_si128( _mm_sub_epi32( k32, specMy ), 12 ) );
	__m128i scaleFactorIntZ = _mm_srl_epi32( kAll, _mm_srli_si128( _mm_sub_epi32( k32, specMz ), 12 ) );
	__m128 scaleFactorX = _mm_cvtepi32_ps( scaleFactorIntX );
	__m128 scaleFactorY = _mm_cvtepi32_ps( scaleFactorIntY );
	__m128 scaleFactorZ = _mm_cvtepi32_ps( scaleFactorIntZ );

	// spec - fixed point - invert and divide by zero guard
	__m128 invScaleFactorX = _mm_rcp_ps( scaleFactorX );
	__m128 invScaleFactorY = _mm_rcp_ps( scaleFactorY );
	__m128 invScaleFactorZ = _mm_rcp_ps( scaleFactorZ );
	invScaleFactorX = _mm_madd_ps( _mm_nmsub_ps( scaleFactorX, invScaleFactorX, k1_0f ), invScaleFactorX, invScaleFactorX );
	invScaleFactorY = _mm_madd_ps( _mm_nmsub_ps( scaleFactorY, invScaleFactorY, k1_0f ), invScaleFactorY, invScaleFactorY );
	invScaleFactorZ = _mm_madd_ps( _mm_nmsub_ps( scaleFactorZ, invScaleFactorZ, k1_0f ), invScaleFactorZ, invScaleFactorZ );
	scaleFactorX = _mm_andnot_ps( *(__m128*)&scaleFactorZeroX, invScaleFactorX );
	scaleFactorY = _mm_andnot_ps( *(__m128*)&scaleFactorZeroY, invScaleFactorY );
	scaleFactorZ = _mm_andnot_ps( *(__m128*)&scaleFactorZeroZ, invScaleFactorZ );

	// spec - select integer if num exponent bits = 0, otherwise float
	__m128i selIntX = _mm_cmpeq_epi32( specEx, _mm_setzero_si128() );
	__m128i selIntY = _mm_cmpeq_epi32( specEy, _mm_setzero_si128() );
	__m128i selIntZ = _mm_cmpeq_epi32( specEz, _mm_setzero_si128() );

	// spec - quaternion orders
	__m128i selDABC = _mm_cmpeq_epi32( specIdx, kDABC );
	__m128i selADBC = _mm_cmpeq_epi32( specIdx, kADBC );
	__m128i selABDC = _mm_cmpeq_epi32( specIdx, kABDC );
	__m128i selABCD = _mm_cmpeq_epi32( specIdx, kABCD );

	// spec - spec shifts
	specSx = _mm_srli_si128( specSx, 12 );
	specSy = _mm_srli_si128( specSy, 12 );
	specSz = _mm_srli_si128( specSz, 12 );
	specEx = _mm_srli_si128( specEx, 12 );
	specEy = _mm_srli_si128( specEy, 12 );
	specEz = _mm_srli_si128( specEz, 12 );
	specMx = _mm_srli_si128( specMx, 12 );
	specMy = _mm_srli_si128( specMy, 12 );
	specMz = _mm_srli_si128( specMz, 12 );

	__m128i specSx32 = _mm_sub_epi32( kXXX32, specSx );
	__m128i specSy32 = _mm_sub_epi32( kXXX32, specSy );
	__m128i specSz32 = _mm_sub_epi32( kXXX32, specSz );
	__m128i specEx32 = _mm_sub_epi32( kXXX32, specEx );
	__m128i specEy32 = _mm_sub_epi32( kXXX32, specEy );
	__m128i specEz32 = _mm_sub_epi32( kXXX32, specEz );
	__m128i specMx32 = _mm_sub_epi32( kXXX32, specMx );
	__m128i specMy32 = _mm_sub_epi32( kXXX32, specMy );
	__m128i specMz32 = _mm_sub_epi32( kXXX32, specMz );

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
		keyQwx = _mm_or_si128( _mm_slle_epi32( keyQwx, bitOfs ), _mm_srle_epi32( keyQwy, bitOfs32 ) );
		keyQwy = _mm_or_si128( _mm_slle_epi32( keyQwy, bitOfs ), _mm_srle_epi32( keyQwz, bitOfs32 ) );
		keyQwz = _mm_or_si128( _mm_slle_epi32( keyQwz, bitOfs ), _mm_srle_epi32( keyQww, bitOfs32 ) );
		__m128i keyx = keyQwx;

		keyQwx = _mm_or_si128( _mm_slle_epi32( keyQwx, sumSpecX ), _mm_srle_epi32( keyQwy, sumSpecX32 ) );
		keyQwy = _mm_or_si128( _mm_slle_epi32( keyQwy, sumSpecX ), _mm_srle_epi32( keyQwz, sumSpecX32 ) );
		__m128i keyy = keyQwx;

		keyQwx = _mm_or_si128( _mm_slle_epi32( keyQwx, sumSpecY ), _mm_srle_epi32( keyQwy, sumSpecY32 ) );
		__m128i keyz = keyQwx;

		// extract + mask s,e,m
		__m128i keySx = _mm_srl_epi32( keyx, specSx32 );
		__m128i keySy = _mm_srl_epi32( keyy, specSy32 );
		__m128i keySz = _mm_srl_epi32( keyz, specSz32 );
		keyx = _mm_sll_epi32( keyx, specSx );
		keyy = _mm_sll_epi32( keyy, specSy );
		keyz = _mm_sll_epi32( keyz, specSz );
		__m128i keyEx = _mm_srl_epi32( keyx, specEx32 );
		__m128i keyEy = _mm_srl_epi32( keyy, specEy32 );
		__m128i keyEz = _mm_srl_epi32( keyz, specEz32 );
		keyx = _mm_sll_epi32( keyx, specEx );
		keyy = _mm_sll_epi32( keyy, specEy );
		keyz = _mm_sll_epi32( keyz, specEz );
		__m128i keyMx = _mm_srl_epi32( keyx, specMx32 );
		__m128i keyMy = _mm_srl_epi32( keyy, specMy32 );
		__m128i keyMz = _mm_srl_epi32( keyz, specMz32 );

		//----------------------------------------------------------------
		// expand as float
		//----------------------------------------------------------------

		// bias exponent
		__m128i keyEx0 = _mm_add_epi32( keyEx, expBiasX );
		__m128i keyEy0 = _mm_add_epi32( keyEy, expBiasY );
		__m128i keyEz0 = _mm_add_epi32( keyEz, expBiasZ );
		keyEx0 = _mm_slli_epi32( keyEx0, 23 );
		keyEy0 = _mm_slli_epi32( keyEy0, 23 );
		keyEz0 = _mm_slli_epi32( keyEz0, 23 );

		// mantissa keyshift
		__m128i keyMx0 = _mm_sll_epi32( keyMx, shiftMx );
		__m128i keyMy0 = _mm_sll_epi32( keyMy, shiftMy );
		__m128i keyMz0 = _mm_sll_epi32( keyMz, shiftMz );

		// sign
		__m128i keySx0 = _mm_slli_epi32( keySx, 31 );
		__m128i keySy0 = _mm_slli_epi32( keySy, 31 );
		__m128i keySz0 = _mm_slli_epi32( keySz, 31 );

		// combine
		__m128i keyFloatX = _mm_or_si128( keySx0, _mm_or_si128( keyEx0, keyMx0 ) );
		__m128i keyFloatY = _mm_or_si128( keySy0, _mm_or_si128( keyEy0, keyMy0 ) );
		__m128i keyFloatZ = _mm_or_si128( keySz0, _mm_or_si128( keyEz0, keyMz0 ) );

		//----------------------------------------------------------------
		// expand as signed integer
		//----------------------------------------------------------------

		// sign extend
		__m128i keySignExtendX = _mm_cmpgt_epi32( keySx, _mm_setzero_si128() );
		__m128i keySignExtendY = _mm_cmpgt_epi32( keySy, _mm_setzero_si128() );
		__m128i keySignExtendZ = _mm_cmpgt_epi32( keySz, _mm_setzero_si128() );
		keySignExtendX = _mm_sll_epi32( keySignExtendX, specMx );
		keySignExtendY = _mm_sll_epi32( keySignExtendY, specMy );
		keySignExtendZ = _mm_sll_epi32( keySignExtendZ, specMz );
		__m128i keySignedIntLX = _mm_or_si128( keySignExtendX, keyMx );
		__m128i keySignedIntLY = _mm_or_si128( keySignExtendY, keyMy );
		__m128i keySignedIntLZ = _mm_or_si128( keySignExtendZ, keyMz );
		__m128 keyIntX = _mm_cvtepi32_ps( keySignedIntLX );
		__m128 keyIntY = _mm_cvtepi32_ps( keySignedIntLY );
		__m128 keyIntZ = _mm_cvtepi32_ps( keySignedIntLZ );

		// scale
		keyIntX = _mm_mul_ps( keyIntX, scaleFactorX );
		keyIntY = _mm_mul_ps( keyIntY, scaleFactorY );
		keyIntZ = _mm_mul_ps( keyIntZ, scaleFactorZ );

		// select integer if num exponent bits = 0, otherwise float
		__m128 keyX0 = _mm_sel_ps( *(__m128*)&keyFloatX, keyIntX, *(__m128*)&selIntX );
		__m128 keyY0 = _mm_sel_ps( *(__m128*)&keyFloatY, keyIntY, *(__m128*)&selIntY );
		__m128 keyZ0 = _mm_sel_ps( *(__m128*)&keyFloatZ, keyIntZ, *(__m128*)&selIntZ );
		__m128 keyW0 = k1_0f;

		// If rotation - convert to quaternion
		if( isRotation )
		{
			// Load + swizzle vectors - only abc components set so far
			__m128 aaaa = keyX0;
			__m128 bbbb = keyY0;
			__m128 cccc = keyZ0;
			__m128 dddd = keyW0;

			// Derived value, d = sqrt(1 - a^2 - b^2 - c^2)
			dddd = _mm_nmsub_ps( aaaa, aaaa, dddd );
			dddd = _mm_nmsub_ps( bbbb, bbbb, dddd );
			dddd = _mm_nmsub_ps( cccc, cccc, dddd );
			dddd = _mm_max_ps( dddd, _mm_setzero_ps() );
			dddd = _mm_min_ps( dddd, k1_0f );
			dddd = _mm_sqrt_ps( dddd );

			// Now shuffle components to insert the derived value 
			keyX0 = _mm_sel_ps( aaaa, dddd, *(__m128*)&selDABC );
			keyY0 = _mm_sel_ps( _mm_sel_ps( bbbb, aaaa, *(__m128*)&selDABC ), dddd, *(__m128*)&selADBC );
			keyZ0 = _mm_sel_ps( _mm_sel_ps( bbbb, cccc, *(__m128*)&selABCD ), dddd, *(__m128*)&selABDC );
			keyW0 = _mm_sel_ps( cccc, dddd, *(__m128*)&selABCD );
		}

		// deswizzle and store
		_MM_TRANSPOSE4_PS( keyX0, keyY0, keyZ0, keyW0 );

		output[ constTable[0] * 3 ] = keyX0;
		output[ constTable[1] * 3 ] = keyY0;
		output[ constTable[2] * 3 ] = keyZ0;
		output[ constTable[3] * 3 ] = keyW0;

		constDataBitOfs = _mm_add_epi32( constDataBitOfs, _mm_slli_epi32( keyStride, 2 ) );
		constTable += 4;
	}
}
