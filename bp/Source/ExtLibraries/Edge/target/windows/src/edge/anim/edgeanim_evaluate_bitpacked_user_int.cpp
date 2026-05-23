/* SCE CONFIDENTIAL
* PlayStation(R)Edge 1.2.0
* Copyright (C) 2010 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#include "edge/anim/edgeanim_structs_ps3.h"
#include "edge/anim/edgeanim_windows.h"

extern "C"
void _edgeAnimEvaluateBitPackedUser( uint32_t animCount,
									 const uint16_t* animTable,
									 const uint32_t* packingSpec,
									 uintptr_t initialAdr,
									 uintptr_t intraAdr, 
									 uintptr_t finalAdr,
									 uintptr_t intraBitsAdr,
									 uint32_t intraBitsOfs, 
									 const __m128i & bitMask,
									 const __m128i & prevBitMask,
									 uint32_t intraFrameCount,
									 uint32_t frameInteger,
									 float frameFraction,
									 float* output )
{
	// Constants
	const __m128i k1 = _mm_set1_epi32( 1 );
	const __m128i k23 = _mm_set1_epi32( 23 );
	const __m128i k31 = _mm_set1_epi32( 31 );
	const __m128i k32 = _mm_set1_epi32( 32 );
	const __m128i k128 = _mm_set1_epi32( 128 );
	const __m128i kAll = _mm_set1_epi32( -1 );
	const __m128 k1_0f = _mm_set1_ps( 1.0f );

	// animated channels
	const __m128 frameFractionV = _mm_set1_ps( frameFraction );
	const __m128i frameIntegerMinus128 = _mm_set1_epi32( frameInteger - 128 );
	const __m128i frameIntegerMinusOne = _mm_set1_epi32( frameInteger - 1 );
	const __m128i bitMaskNeg = _mm_andnot_si128( bitMask, _mm_cmpeq_epi32( bitMask, bitMask ) );

	__m128i intraBitsOffset = _mm_setr_epi32( intraBitsOfs, intraBitsOfs+intraFrameCount, intraBitsOfs+2*intraFrameCount, intraBitsOfs+3*intraFrameCount );
	__m128i intraBitsOffsetInc = _mm_set1_epi32( 4*intraFrameCount );

	__m128i intraOfs = _mm_setzero_si128();
	__m128i boundsOfs = _mm_setzero_si128();

	for( uint32_t count4 = (animCount + 3) >> 2; count4; count4-- )
	{
		//----------------------------------------------------------------
		// load packing specs x 4
		//----------------------------------------------------------------
		__m128i spec = _mm_loadu_si128( (const __m128i*) packingSpec );
		__m128i specSx = _mm_srli_epi32( spec, 31 );
		__m128i specEx = _mm_srli_epi32( _mm_slli_epi32( spec,  1 ), 28 );
		__m128i specMx = _mm_srli_epi32( _mm_slli_epi32( spec,  5 ), 27 );
		__m128i sumSpecX = _mm_add_epi32( _mm_add_epi32( specSx, specEx ), specMx );
		__m128i keyStride = sumSpecX;

		__m128i specSx32 = _mm_sub_epi32( k32, specSx );
		__m128i specEx32 = _mm_sub_epi32( k32, specEx );
		__m128i specMx32 = _mm_sub_epi32( k32, specMx );

		__m128i strideSum = keyStride;
		strideSum = _mm_add_epi32( strideSum, _mm_slli_si128( keyStride,  4 ) );
		strideSum = _mm_add_epi32( strideSum, _mm_slli_si128( keyStride,  8 ) );
		strideSum = _mm_add_epi32( strideSum, _mm_slli_si128( keyStride, 12 ) );

		__m128i keyOfs = _mm_slli_si128( strideSum, 4 );
		__m128i boundsCurOfs = _mm_add_epi32( boundsOfs, keyOfs );

		//----------------------------------------------------------------
		// decode intra bit stream to determine keyframes
		//----------------------------------------------------------------

		__m128i byteOfs = _mm_srli_epi32( intraBitsOffset, 3 );
		__m128i bitOfs = _mm_srli_epi32( _mm_slli_epi32( intraBitsOffset, 29 ), 29 );
		__m128i bitOfs32 = _mm_sub_epi32( k32, bitOfs );

		uintptr_t adr0 = intraBitsAdr + _MM_EXTRACT_EPI32_X( byteOfs );
		uintptr_t adr1 = intraBitsAdr + _MM_EXTRACT_EPI32_Y( byteOfs );
		uintptr_t adr2 = intraBitsAdr + _MM_EXTRACT_EPI32_Z( byteOfs );
		uintptr_t adr3 = intraBitsAdr + _MM_EXTRACT_EPI32_W( byteOfs );

		__m128i d0a = _mm_loadu_si128( (__m128i*)( adr0 ) );
		__m128i d1a = _mm_loadu_si128( (__m128i*)( adr1 ) );
		__m128i d2a = _mm_loadu_si128( (__m128i*)( adr2 ) );
		__m128i d3a = _mm_loadu_si128( (__m128i*)( adr3 ) );
		__m128i d0b = _mm_loadu_si128( (__m128i*)( adr0 + 4 ) );
		__m128i d1b = _mm_loadu_si128( (__m128i*)( adr1 + 4 ) );
		__m128i d2b = _mm_loadu_si128( (__m128i*)( adr2 + 4 ) );
		__m128i d3b = _mm_loadu_si128( (__m128i*)( adr3 + 4 ) );

		d0a = _mm_or_si128( _mm_slli_epi16( d0a,  8 ), _mm_srli_epi16( d0a,  8 ) );
		d1a = _mm_or_si128( _mm_slli_epi16( d1a,  8 ), _mm_srli_epi16( d1a,  8 ) );
		d2a = _mm_or_si128( _mm_slli_epi16( d2a,  8 ), _mm_srli_epi16( d2a,  8 ) );
		d3a = _mm_or_si128( _mm_slli_epi16( d3a,  8 ), _mm_srli_epi16( d3a,  8 ) );
		d0a = _mm_or_si128( _mm_slli_epi32( d0a, 16 ), _mm_srli_epi32( d0a, 16 ) );
		d1a = _mm_or_si128( _mm_slli_epi32( d1a, 16 ), _mm_srli_epi32( d1a, 16 ) );
		d2a = _mm_or_si128( _mm_slli_epi32( d2a, 16 ), _mm_srli_epi32( d2a, 16 ) );
		d3a = _mm_or_si128( _mm_slli_epi32( d3a, 16 ), _mm_srli_epi32( d3a, 16 ) );
		d0b = _mm_slli_epi32( d0b, 24 );
		d1b = _mm_slli_epi32( d1b, 24 );
		d2b = _mm_slli_epi32( d2b, 24 );
		d3b = _mm_slli_epi32( d3b, 24 );

		// bit shift
		d0a = _mm_sll_epi32( d0a, _MM_SHIFT_EPI32_X( bitOfs ) );
		d1a = _mm_sll_epi32( d1a, _MM_SHIFT_EPI32_Y( bitOfs ) );
		d2a = _mm_sll_epi32( d2a, _MM_SHIFT_EPI32_Z( bitOfs ) );
		d3a = _mm_sll_epi32( d3a, _MM_SHIFT_EPI32_W( bitOfs ) );
		d0b = _mm_srl_epi32( d0b, _MM_SHIFT_EPI32_X( bitOfs32 ) );
		d1b = _mm_srl_epi32( d1b, _MM_SHIFT_EPI32_Y( bitOfs32 ) );
		d2b = _mm_srl_epi32( d2b, _MM_SHIFT_EPI32_Z( bitOfs32 ) );
		d3b = _mm_srl_epi32( d3b, _MM_SHIFT_EPI32_W( bitOfs32 ) );

		__m128i ib0 = _mm_or_si128( d0a, d0b );
		__m128i ib1 = _mm_or_si128( d1a, d1b );
		__m128i ib2 = _mm_or_si128( d2a, d2b );
		__m128i ib3 = _mm_or_si128( d3a, d3b );

		// swizzle bits -> xxxx yyyy zzzz wwww
		__m128i ibx = ib0, iby = ib1, ibz = ib2, ibw = ib3;
		_MM_TRANSPOSE4_EPI32( ibx, iby, ibz, ibw );

		// Clear invalid bits (if intra frame count is less than 128)
		__m128i bx = _mm_and_si128( ibx, _mm_shuffle_epi32( bitMask, _MM_SHUFFLE( 0, 0, 0, 0 ) ) );
		__m128i by = _mm_and_si128( iby, _mm_shuffle_epi32( bitMask, _MM_SHUFFLE( 1, 1, 1, 1 ) ) );
		__m128i bz = _mm_and_si128( ibz, _mm_shuffle_epi32( bitMask, _MM_SHUFFLE( 2, 2, 2, 2 ) ) );
		__m128i bw = _mm_and_si128( ibw, _mm_shuffle_epi32( bitMask, _MM_SHUFFLE( 3, 3, 3, 3 ) ) );

		// Get bits prior to current frame
		__m128i pbx = _mm_and_si128( ibx, _mm_shuffle_epi32( prevBitMask, _MM_SHUFFLE( 0, 0, 0, 0 ) ) );
		__m128i pby = _mm_and_si128( iby, _mm_shuffle_epi32( prevBitMask, _MM_SHUFFLE( 1, 1, 1, 1 ) ) );
		__m128i pbz = _mm_and_si128( ibz, _mm_shuffle_epi32( prevBitMask, _MM_SHUFFLE( 2, 2, 2, 2 ) ) );
		__m128i pbw = _mm_and_si128( ibw, _mm_shuffle_epi32( prevBitMask, _MM_SHUFFLE( 3, 3, 3, 3 ) ) );

		// Count enabled bits
		__m128i numBits;
		numBits = _mm_popcnt_epi32( bx );
		numBits = _mm_add_epi32( numBits, _mm_popcnt_epi32( by ) );
		numBits = _mm_add_epi32( numBits, _mm_popcnt_epi32( bz ) );
		numBits = _mm_add_epi32( numBits, _mm_popcnt_epi32( bw ) );

		__m128i numPrevBits;
		numPrevBits = _mm_popcnt_epi32( pbx );
		numPrevBits = _mm_add_epi32( numPrevBits, _mm_popcnt_epi32( pby ) );
		numPrevBits = _mm_add_epi32( numPrevBits, _mm_popcnt_epi32( pbz ) );
		numPrevBits = _mm_add_epi32( numPrevBits, _mm_popcnt_epi32( pbw ) );

		// get slerp alpha (t)
		__m128i tzi0 = _mm_and_si128( pbx, _mm_sub_epi32( _mm_setzero_si128(), pbx ) );
		__m128i tzi1 = _mm_and_si128( pby, _mm_sub_epi32( _mm_setzero_si128(), pby ) );
		__m128i tzi2 = _mm_and_si128( pbz, _mm_sub_epi32( _mm_setzero_si128(), pbz ) );
		__m128i tzi3 = _mm_and_si128( pbw, _mm_sub_epi32( _mm_setzero_si128(), pbw ) );
		__m128i tzc0 = _mm_cmpeq_epi32( tzi0, _mm_setzero_si128() );
		__m128i tzc1 = _mm_cmpeq_epi32( tzi1, _mm_setzero_si128() );
		__m128i tzc2 = _mm_cmpeq_epi32( tzi2, _mm_setzero_si128() );
		__m128i tzc3 = _mm_cmpeq_epi32( tzi3, _mm_setzero_si128() );
		__m128i tz0 = _mm_bsr_epi32( tzi0 );
		__m128i tz1 = _mm_bsr_epi32( tzi1 );
		__m128i tz2 = _mm_bsr_epi32( tzi2 );
		__m128i tz3 = _mm_bsr_epi32( tzi3 );
		tz0 = _mm_sel_si128( tz0, k32, tzc0 );
		tz1 = _mm_sel_si128( tz1, k32, tzc1 );
		tz2 = _mm_sel_si128( tz2, k32, tzc2 );
		tz3 = _mm_sel_si128( tz3, k32, tzc3 );
		__m128i tzm1 = _mm_and_si128( tzc3, tzc2 );
		__m128i tzm2 = _mm_and_si128( tzm1, tzc1 );
		__m128i tzs1 = _mm_add_epi32( tz3, _mm_and_si128( tz2, tzc3 ) );
		__m128i tzs2 = _mm_add_epi32( tzs1, _mm_and_si128( tz1, tzm1 ) );
		__m128i tzs3 = _mm_add_epi32( tzs2, _mm_and_si128( tz0, tzm2 ) );
		__m128 bBits = _mm_cvtepi32_ps( _mm_add_epi32( tzs3, frameIntegerMinus128 ) );

		__m128i lzi0 = _mm_or_si128( _mm_andnot_si128( _mm_shuffle_epi32( prevBitMask, _MM_SHUFFLE( 0, 0, 0, 0 ) ), bx ), _mm_shuffle_epi32( bitMaskNeg, _MM_SHUFFLE( 0, 0, 0, 0 ) ) );
		__m128i lzi1 = _mm_or_si128( _mm_andnot_si128( _mm_shuffle_epi32( prevBitMask, _MM_SHUFFLE( 1, 1, 1, 1 ) ), by ), _mm_shuffle_epi32( bitMaskNeg, _MM_SHUFFLE( 1, 1, 1, 1 ) ) );
		__m128i lzi2 = _mm_or_si128( _mm_andnot_si128( _mm_shuffle_epi32( prevBitMask, _MM_SHUFFLE( 2, 2, 2, 2 ) ), bz ), _mm_shuffle_epi32( bitMaskNeg, _MM_SHUFFLE( 2, 2, 2, 2 ) ) );
		__m128i lzi3 = _mm_or_si128( _mm_andnot_si128( _mm_shuffle_epi32( prevBitMask, _MM_SHUFFLE( 3, 3, 3, 3 ) ), bw ), _mm_shuffle_epi32( bitMaskNeg, _MM_SHUFFLE( 3, 3, 3, 3 ) ) );
		__m128i lzc0 = _mm_cmpeq_epi32( lzi0, _mm_setzero_si128() );
		__m128i lzc1 = _mm_cmpeq_epi32( lzi1, _mm_setzero_si128() );
		__m128i lzc2 = _mm_cmpeq_epi32( lzi2, _mm_setzero_si128() );
		__m128i lzc3 = _mm_cmpeq_epi32( lzi3, _mm_setzero_si128() );
		__m128i lz0 = _mm_bsr_epi32( lzi0 );
		__m128i lz1 = _mm_bsr_epi32( lzi1 );
		__m128i lz2 = _mm_bsr_epi32( lzi2 );
		__m128i lz3 = _mm_bsr_epi32( lzi3 );
		lz0 = _mm_sel_si128( _mm_sub_epi32( k31, lz0 ), k32, lzc0 );
		lz1 = _mm_sel_si128( _mm_sub_epi32( k31, lz1 ), k32, lzc1 );
		lz2 = _mm_sel_si128( _mm_sub_epi32( k31, lz2 ), k32, lzc2 );
		lz3 = _mm_sel_si128( _mm_sub_epi32( k31, lz3 ), k32, lzc3 );
		__m128i lzm1 = _mm_and_si128( lzc0, lzc1 );
		__m128i lzm2 = _mm_and_si128( lzm1, lzc2 );
		__m128i lzs1 = _mm_add_epi32( lz0, _mm_and_si128( lz1, lzc0 ) );
		__m128i lzs2 = _mm_add_epi32( lzs1, _mm_and_si128( lz2, lzm1 ) );
		__m128i lzs3 = _mm_add_epi32( lzs2, _mm_and_si128( lz3, lzm2 ) );
		__m128 aBits = _mm_cvtepi32_ps( _mm_sub_epi32( lzs3, frameIntegerMinusOne ) );

		// Compute tweening factors
		__m128 t = _mm_div_ps( _mm_add_ps( bBits, frameFractionV ), _mm_add_ps( aBits, bBits ) );

		// get compressed joint addresses
		__m128i jointStrideLo = _mm_mul_epu32( numBits, keyStride );
		__m128i jointStrideHi = _mm_mul_epu32( _mm_srli_epi64( numBits, 32 ), _mm_srli_epi64( keyStride, 32 ) );
		__m128i jointStride = _mm_or_si128( jointStrideLo, _mm_slli_epi64( jointStrideHi, 32 ) );

		__m128i jointOfs = jointStride;
		jointOfs = _mm_add_epi32( jointOfs, _mm_slli_si128( jointStride,  4 ) );
		jointOfs = _mm_add_epi32( jointOfs, _mm_slli_si128( jointStride,  8 ) );
		jointOfs = _mm_add_epi32( jointOfs, _mm_slli_si128( jointStride, 12 ) );

		__m128i prevStrideLo = _mm_mul_epu32( numPrevBits, keyStride );
		__m128i prevStrideHi = _mm_mul_epu32( _mm_srli_epi64( numPrevBits, 32 ), _mm_srli_epi64( keyStride, 32 ) );
		__m128i prevStride = _mm_or_si128( prevStrideLo, _mm_slli_epi64( prevStrideHi, 32 ) );

		__m128i rightCurOfs = _mm_slli_si128( jointOfs, 4 );
		rightCurOfs = _mm_add_epi32( prevStride, rightCurOfs );
		rightCurOfs = _mm_add_epi32( intraOfs, rightCurOfs );
		__m128i leftCurOfs = _mm_sub_epi32( rightCurOfs, keyStride );

		uintptr_t left0adr = ( _mm_extract_epi16( numPrevBits, 0 ) == 0 ? initialAdr : intraAdr );
		uintptr_t left1adr = ( _mm_extract_epi16( numPrevBits, 2 ) == 0 ? initialAdr : intraAdr );
		uintptr_t left2adr = ( _mm_extract_epi16( numPrevBits, 4 ) == 0 ? initialAdr : intraAdr );
		uintptr_t left3adr = ( _mm_extract_epi16( numPrevBits, 6 ) == 0 ? initialAdr : intraAdr );

		uintptr_t right0adr = ( _mm_extract_epi16( numPrevBits, 0 ) == _mm_extract_epi16( numBits, 0 ) ? finalAdr : intraAdr );
		uintptr_t right1adr = ( _mm_extract_epi16( numPrevBits, 2 ) == _mm_extract_epi16( numBits, 2 ) ? finalAdr : intraAdr );
		uintptr_t right2adr = ( _mm_extract_epi16( numPrevBits, 4 ) == _mm_extract_epi16( numBits, 4 ) ? finalAdr : intraAdr );
		uintptr_t right3adr = ( _mm_extract_epi16( numPrevBits, 6 ) == _mm_extract_epi16( numBits, 6 ) ? finalAdr : intraAdr );

		__m128i leftOfs = _mm_sel_si128( leftCurOfs, boundsCurOfs, _mm_cmpeq_epi32( numPrevBits, _mm_setzero_si128() ) );
		__m128i rightOfs = _mm_sel_si128( rightCurOfs, boundsCurOfs, _mm_cmpeq_epi32( numPrevBits, numBits ) );

		// load the bitpacked keysframes
		__m128i leftByteOfs = _mm_srli_epi32( leftOfs, 3 );
		left0adr = left0adr + _MM_EXTRACT_EPI32_X( leftByteOfs );
		left1adr = left1adr + _MM_EXTRACT_EPI32_Y( leftByteOfs );
		left2adr = left2adr + _MM_EXTRACT_EPI32_Z( leftByteOfs );
		left3adr = left3adr + _MM_EXTRACT_EPI32_W( leftByteOfs );

		__m128i leftBitOfs = _mm_srli_epi32( _mm_slli_epi32( leftOfs, 29 ), 29 );
		__m128i leftBitOfs32 = _mm_sub_epi32( k32, leftBitOfs );

		__m128i left0 = _mm_loadu_si128( (__m128i*)( left0adr ) );
		__m128i left1 = _mm_loadu_si128( (__m128i*)( left1adr ) );
		__m128i left2 = _mm_loadu_si128( (__m128i*)( left2adr ) );
		__m128i left3 = _mm_loadu_si128( (__m128i*)( left3adr ) );

		left0 = _mm_or_si128( _mm_slli_epi16( left0,  8 ), _mm_srli_epi16( left0,  8 ) );
		left1 = _mm_or_si128( _mm_slli_epi16( left1,  8 ), _mm_srli_epi16( left1,  8 ) );
		left2 = _mm_or_si128( _mm_slli_epi16( left2,  8 ), _mm_srli_epi16( left2,  8 ) );
		left3 = _mm_or_si128( _mm_slli_epi16( left3,  8 ), _mm_srli_epi16( left3,  8 ) );
		left0 = _mm_or_si128( _mm_slli_epi32( left0, 16 ), _mm_srli_epi32( left0, 16 ) );
		left1 = _mm_or_si128( _mm_slli_epi32( left1, 16 ), _mm_srli_epi32( left1, 16 ) );
		left2 = _mm_or_si128( _mm_slli_epi32( left2, 16 ), _mm_srli_epi32( left2, 16 ) );
		left3 = _mm_or_si128( _mm_slli_epi32( left3, 16 ), _mm_srli_epi32( left3, 16 ) );

		__m128i rightByteOfs = _mm_srli_epi32( rightOfs, 3 );
		right0adr = right0adr + _MM_EXTRACT_EPI32_X( rightByteOfs );
		right1adr = right1adr + _MM_EXTRACT_EPI32_Y( rightByteOfs );
		right2adr = right2adr + _MM_EXTRACT_EPI32_Z( rightByteOfs );
		right3adr = right3adr + _MM_EXTRACT_EPI32_W( rightByteOfs );

		__m128i rightBitOfs = _mm_srli_epi32( _mm_slli_epi32( rightOfs, 29 ), 29 );
		__m128i rightBitOfs32 = _mm_sub_epi32( k32, rightBitOfs );

		__m128i right0 = _mm_loadu_si128( (__m128i*)( right0adr ) );
		__m128i right1 = _mm_loadu_si128( (__m128i*)( right1adr ) );
		__m128i right2 = _mm_loadu_si128( (__m128i*)( right2adr ) );
		__m128i right3 = _mm_loadu_si128( (__m128i*)( right3adr ) );

		right0 = _mm_or_si128( _mm_slli_epi16( right0,  8 ), _mm_srli_epi16( right0,  8 ) );
		right1 = _mm_or_si128( _mm_slli_epi16( right1,  8 ), _mm_srli_epi16( right1,  8 ) );
		right2 = _mm_or_si128( _mm_slli_epi16( right2,  8 ), _mm_srli_epi16( right2,  8 ) );
		right3 = _mm_or_si128( _mm_slli_epi16( right3,  8 ), _mm_srli_epi16( right3,  8 ) );
		right0 = _mm_or_si128( _mm_slli_epi32( right0, 16 ), _mm_srli_epi32( right0, 16 ) );
		right1 = _mm_or_si128( _mm_slli_epi32( right1, 16 ), _mm_srli_epi32( right1, 16 ) );
		right2 = _mm_or_si128( _mm_slli_epi32( right2, 16 ), _mm_srli_epi32( right2, 16 ) );
		right3 = _mm_or_si128( _mm_slli_epi32( right3, 16 ), _mm_srli_epi32( right3, 16 ) );

		// swizzle
		__m128i leftQwx = left0, leftQwy = left1, leftQwz = left2, leftQww = left3;
		__m128i rightQwx = right0, rightQwy = right1, rightQwz = right2, rightQww = right3;
		_MM_TRANSPOSE4_EPI32( leftQwx, leftQwy, leftQwz, leftQww );
		_MM_TRANSPOSE4_EPI32( rightQwx, rightQwy, rightQwz, rightQww );

		// bit shift
		__m128i leftx = _mm_or_si128( _mm_slle_epi32( leftQwx, leftBitOfs ), _mm_srle_epi32( leftQwy, leftBitOfs32 ) );
		__m128i rightx = _mm_or_si128( _mm_slle_epi32( rightQwx, rightBitOfs ), _mm_srle_epi32( rightQwy, rightBitOfs32 ) );

		// extract + mask s,e,m
		__m128i leftSx = _mm_srle_epi32( leftx, specSx32 );
		leftx = _mm_slle_epi32( leftx, specSx );
		__m128i leftEx = _mm_srle_epi32( leftx, specEx32 );
		leftx = _mm_slle_epi32( leftx, specEx );
		__m128i leftMx = _mm_srle_epi32( leftx, specMx32 );

		__m128i rightSx = _mm_srle_epi32( rightx, specSx32 );
		rightx = _mm_slle_epi32( rightx, specSx );
		__m128i rightEx = _mm_srle_epi32( rightx, specEx32 );
		rightx = _mm_slle_epi32( rightx, specEx );
		__m128i rightMx = _mm_srle_epi32( rightx, specMx32 );

		//----------------------------------------------------------------
		// expand as float
		//----------------------------------------------------------------

		// bias exponent ( 128 - ( 1U << ( numExponentBits - 1 ) )
		__m128i expBiasX = _mm_sub_epi32( specEx, k1 );
		expBiasX = _mm_slle_epi32( k1, expBiasX );
		expBiasX = _mm_sub_epi32( k128, expBiasX );

		__m128i leftEx0 = _mm_add_epi32( leftEx, expBiasX );
		leftEx0 = _mm_slli_epi32( leftEx0, 23 );

		__m128i rightEx0 = _mm_add_epi32( rightEx, expBiasX );
		rightEx0 = _mm_slli_epi32( rightEx0, 23 );

		// mantissa keyshift by 23 - numBitsMantissa
		__m128i shiftMx = _mm_sub_epi32( k23, specMx );
		__m128i leftMx0 = _mm_slle_epi32( leftMx, shiftMx );
		__m128i rightMx0 = _mm_slle_epi32( rightMx, shiftMx );

		// sign
		__m128i leftSx0 = _mm_slli_epi32( leftSx, 31 );
		__m128i rightSx0 = _mm_slli_epi32( rightSx, 31 );

		// combine
		__m128i leftFloatX = _mm_or_si128( leftSx0, _mm_or_si128( leftEx0, leftMx0 ) );
		__m128i rightFloatX = _mm_or_si128( rightSx0, _mm_or_si128( rightEx0, rightMx0 ) );

		//----------------------------------------------------------------
		// expand as signed integer
		//----------------------------------------------------------------

		// sign extend
		__m128i leftSignExtendX = _mm_srai_epi32( _mm_slli_epi32( leftSx, 31 ), 31 );
		__m128i rightSignExtendX = _mm_srai_epi32( _mm_slli_epi32( rightSx, 31 ), 31 );
		leftSignExtendX = _mm_slle_epi32( leftSignExtendX, specMx );
		rightSignExtendX = _mm_slle_epi32( rightSignExtendX, specMx );
		__m128i leftSignedIntLX = _mm_or_si128( leftSignExtendX, leftMx );
		__m128i rightSignedIntLX = _mm_or_si128( rightSignExtendX, rightMx );
		__m128 leftIntX = _mm_cvtepi32_ps( leftSignedIntLX );
		__m128 rightIntX = _mm_cvtepi32_ps( rightSignedIntLX );

		// scale factor
		__m128i scaleFactorZeroX = _mm_cmpeq_epi32( specMx, _mm_setzero_si128() );
		__m128i scaleFactorIntX = _mm_sub_epi32( k32, specMx );
		scaleFactorIntX = _mm_unpackx_epi32(
			_mm_srl_epi32( kAll, _MM_SHIFT_EPI32_X( scaleFactorIntX ) ),
			_mm_srl_epi32( kAll, _MM_SHIFT_EPI32_Y( scaleFactorIntX ) ),
			_mm_srl_epi32( kAll, _MM_SHIFT_EPI32_Z( scaleFactorIntX ) ),
			_mm_srl_epi32( kAll, _MM_SHIFT_EPI32_W( scaleFactorIntX ) )
			);
		__m128 scaleFactorX = _mm_cvtepi32_ps( scaleFactorIntX );

		// invert and divide by zero guard
		__m128 invScaleFactorX = _mm_rcp_ps( scaleFactorX );
		invScaleFactorX = _mm_madd_ps( _mm_nmsub_ps( scaleFactorX, invScaleFactorX, k1_0f ), invScaleFactorX, invScaleFactorX );
		scaleFactorX = _mm_andnot_ps( *(__m128*)&scaleFactorZeroX, invScaleFactorX );

		// scale
		leftIntX = _mm_mul_ps( leftIntX, scaleFactorX );
		rightIntX = _mm_mul_ps( rightIntX, scaleFactorX );

		// select integer if num exponent bits = 0, otherwise float
		__m128i selIntX = _mm_cmpeq_epi32( specEx, _mm_setzero_si128() );

		__m128 leftX = _mm_sel_ps( *(__m128*)&leftFloatX, leftIntX, *(__m128*)&selIntX );
		__m128 rightX = _mm_sel_ps( *(__m128*)&rightFloatX, rightIntX, *(__m128*)&selIntX );

		// lerp
		__m128 oneMinusT = _mm_sub_ps( k1_0f, t );
		__m128 outX = _mm_madd_ps( t, rightX, _mm_mul_ps( oneMinusT, leftX ) );

		// store
		_mm_store_ss( output + animTable[0], outX );
		_mm_store_ss( output + animTable[1], _mm_shuffle_ps( outX, outX, _MM_SHUFFLE( 1, 1, 1, 1 ) ) );
		_mm_store_ss( output + animTable[2], _mm_shuffle_ps( outX, outX, _MM_SHUFFLE( 2, 2, 2, 2 ) ) );
		_mm_store_ss( output + animTable[3], _mm_shuffle_ps( outX, outX, _MM_SHUFFLE( 3, 3, 3, 3 ) ) );

		// Advance pointers
		animTable += 4;
		packingSpec += 4;

		intraBitsOffset = _mm_add_epi32( intraBitsOffset, intraBitsOffsetInc );

		intraOfs = _mm_add_epi32( intraOfs, _mm_shuffle_epi32( jointOfs, _MM_SHUFFLE( 3, 3, 3, 3 ) ) );
		boundsOfs = _mm_add_epi32( boundsOfs, _mm_shuffle_epi32( strideSum, _MM_SHUFFLE( 3, 3, 3, 3 ) ) );
	}
}

