/* SCE CONFIDENTIAL
* PlayStation(R)Edge 1.2.0
* Copyright (C) 2010 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#include "edge/anim/edgeanim_structs_ps3.h"
#include "edge/anim/edgeanim_windows.h"

extern "C"
void _edgeAnimEvaluateUser( uint32_t animCount,
						    const uint16_t* animTable,
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
							float* outputUserChannels )
{
	// constants
	const uint32_t kStride = 4;
	const __m128i k31 = _mm_set1_epi32( 31 );
	const __m128i k32 = _mm_set1_epi32( 32 );

	// animated channels
	const __m128 frameFractionV = _mm_set1_ps( frameFraction );
	const __m128i frameIntegerMinusOne = _mm_set1_epi32( frameInteger - 1 );
	const __m128i frameIntegerMinus128 = _mm_set1_epi32( frameInteger - 128 );
	const __m128i bitMaskNeg = _mm_andnot_si128( bitMask, _mm_cmpeq_epi32( bitMask, bitMask ) );

	const __m128i stride = _mm_set1_epi32( kStride );
	const __m128i strideInc = _mm_set1_epi32( 4*kStride );
	const __m128i intraBitsOffsetInc = _mm_set1_epi32( 4*intraFrameCount );

	__m128i intraBitsOffset = _mm_setr_epi32( intraBitsOfs, intraBitsOfs+intraFrameCount, intraBitsOfs+2*intraFrameCount, intraBitsOfs+3*intraFrameCount );

	__m128i intraOfs = _mm_setzero_si128();
	__m128i boundsOfs = _mm_setu8_epi32( 0, kStride, 2*kStride, 3*kStride );

	for( uint32_t count4 = (animCount + 3) >> 2; count4; count4-- )
	{
		// load bit data + endian swap
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
		__m128i jointOfs = numPrevBits;
		jointOfs = _mm_add_epi32( jointOfs, _mm_slli_si128( numBits,  4 ) );
		jointOfs = _mm_add_epi32( jointOfs, _mm_slli_si128( numBits,  8 ) );
		jointOfs = _mm_add_epi32( jointOfs, _mm_slli_si128( numBits, 12 ) );
		jointOfs = _mm_slli_epi32( jointOfs, 2 ); // *kStride = *4
		jointOfs = _mm_add_epi32( jointOfs, intraOfs );

		uintptr_t key0adr0 = ( _mm_extract_epi16( numPrevBits, 0 ) == 0 ? initialAdr : intraAdr );
		uintptr_t key0adr1 = ( _mm_extract_epi16( numPrevBits, 2 ) == 0 ? initialAdr : intraAdr );
		uintptr_t key0adr2 = ( _mm_extract_epi16( numPrevBits, 4 ) == 0 ? initialAdr : intraAdr );
		uintptr_t key0adr3 = ( _mm_extract_epi16( numPrevBits, 6 ) == 0 ? initialAdr : intraAdr );

		uintptr_t key1adr0 = ( _mm_extract_epi16( numPrevBits, 0 ) == _mm_extract_epi16( numBits, 0 ) ? finalAdr : intraAdr );
		uintptr_t key1adr1 = ( _mm_extract_epi16( numPrevBits, 2 ) == _mm_extract_epi16( numBits, 2 ) ? finalAdr : intraAdr );
		uintptr_t key1adr2 = ( _mm_extract_epi16( numPrevBits, 4 ) == _mm_extract_epi16( numBits, 4 ) ? finalAdr : intraAdr );
		uintptr_t key1adr3 = ( _mm_extract_epi16( numPrevBits, 6 ) == _mm_extract_epi16( numBits, 6 ) ? finalAdr : intraAdr );

		__m128i key0ofs = _mm_sel_si128( _mm_sub_epi32( jointOfs, stride ), boundsOfs, _mm_cmpeq_epi32( numPrevBits, _mm_setzero_si128() ) );
		__m128i key1ofs = _mm_sel_si128( jointOfs, boundsOfs, _mm_cmpeq_epi32( numPrevBits, numBits ) );

		key0adr0 = key0adr0 + _MM_EXTRACT_EPI32_X( key0ofs );
		key0adr1 = key0adr1 + _MM_EXTRACT_EPI32_Y( key0ofs );
		key0adr2 = key0adr2 + _MM_EXTRACT_EPI32_Z( key0ofs );
		key0adr3 = key0adr3 + _MM_EXTRACT_EPI32_W( key0ofs );

		key1adr0 = key1adr0 + _MM_EXTRACT_EPI32_X( key1ofs );
		key1adr1 = key1adr1 + _MM_EXTRACT_EPI32_Y( key1ofs );
		key1adr2 = key1adr2 + _MM_EXTRACT_EPI32_Z( key1ofs );
		key1adr3 = key1adr3 + _MM_EXTRACT_EPI32_W( key1ofs );

		// lerp
		__m128 user0 = _mm_setr_ps(
			*(float*) key0adr0,
			*(float*) key0adr1,
			*(float*) key0adr2,
			*(float*) key0adr3
		);

		__m128 user1 = _mm_setr_ps(
			*(float*) key1adr0,
			*(float*) key1adr1,
			*(float*) key1adr2,
			*(float*) key1adr3
			);

		// lerp
		__m128 user = _mm_madd_ps( t, user1, _mm_nmsub_ps( t, user0, user0 ) );

		// output
		_mm_store_ss( outputUserChannels + animTable[0], user );
		_mm_store_ss( outputUserChannels + animTable[1], _mm_shuffle_ps( user, user, _MM_SHUFFLE( 1, 1, 1, 1 ) ) );
		_mm_store_ss( outputUserChannels + animTable[2], _mm_shuffle_ps( user, user, _MM_SHUFFLE( 2, 2, 2, 2 ) ) );
		_mm_store_ss( outputUserChannels + animTable[3], _mm_shuffle_ps( user, user, _MM_SHUFFLE( 3, 3, 3, 3 ) ) );

		// advance pointers
		// kStride(4) * sumBits
		__m128i intraInc = _mm_add_epi32( numBits, _mm_shuffle_epi32( numBits, _MM_SHUFFLE( 2, 3, 0, 1 ) ) );
		intraInc = _mm_add_epi32( intraInc, _mm_shuffle_epi32( intraInc, _MM_SHUFFLE( 1, 0, 3, 2 ) ) );
		intraInc = _mm_slli_epi32( intraInc, 2 );

		// Advance pointers
		intraBitsOffset = _mm_add_epi32( intraBitsOffset, intraBitsOffsetInc );

		intraOfs = _mm_add_epi32( intraOfs, intraInc );
		boundsOfs = _mm_add_epi32( boundsOfs, strideInc );
		animTable += 4;
	}
}
