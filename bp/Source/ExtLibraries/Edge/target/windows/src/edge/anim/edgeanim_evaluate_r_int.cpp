/* SCE CONFIDENTIAL
* PlayStation(R)Edge 1.2.0
* Copyright (C) 2010 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#include "edge/anim/edgeanim_structs_ps3.h"
#include "edge/anim/edgeanim_windows.h"

extern "C"
void _edgeAnimEvaluateR( uint32_t animCount,
						 const uint16_t* animTable,
						 uintptr_t initialAdr,
						 uintptr_t intraAdr, 
						 uintptr_t finalAdr,
						 uintptr_t intraBitsAdr,
						 const __m128i & bitMask,
						 const __m128i & prevBitMask,
						 uint32_t intraFrameCount,
						 uint32_t frameInteger,
						 float frameFraction,
						 __m128* output )
{
	const float kSqrt2 = 1.41421356237309504880168872420970f;
	const float kInvSqrt2 = 0.70710678118654752440084436210485f;

	// Constants
	const uint32_t kStride = 6;
	const __m128i k31 = _mm_set1_epi32( 31 );
	const __m128i k32 = _mm_set1_epi32( 32 );
	const __m128 k1_0f = _mm_set1_ps( 1.0f );
	const __m128 kNeg1_0f = _mm_set1_ps( -1.0f );
	const __m128 oneMinusEpsilon = _mm_set1_ps( 0.999f );

	// Constants: Quaternion Fixed -> Floating Point
	const __m128 kQuatOffset = _mm_set1_ps( -kInvSqrt2 );
	const __m128 kQuatScale = _mm_set1_ps( kSqrt2 / ( ( 1 << 15 ) - 1 ) );

	// Constants: Element Orders
	const __m128i kDABC = _mm_set1_epi32( 0 );
	const __m128i kADBC = _mm_set1_epi32( 1 );
	const __m128i kABDC = _mm_set1_epi32( 2 );
	const __m128i kABCD = _mm_set1_epi32( 3 );

	// Constants : MacLaurin - Acos
	const __m128 acos0 = _mm_set1_ps( 1.5707963050f );
	const __m128 acos1 = _mm_set1_ps( 0.2145988016f );
	const __m128 acos2 = _mm_set1_ps( 0.0889789874f );
	const __m128 acos3 = _mm_set1_ps( 0.0501743046f );

	// Constants : Taylor - Sin
	const __m128 sin0 = _mm_set1_ps( -0.1666666664f );
	const __m128 sin1 = _mm_set1_ps( 0.00833333150f );
	const __m128 sin2 = _mm_set1_ps( -0.0001984090f );
	const __m128 sin3 = _mm_set1_ps( 0.00000275260f );

	// animated channels
	uint32_t intraBitsOfs = 0;
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
		jointOfs = _mm_add_epi32( _mm_slli_epi32( jointOfs, 2 ), _mm_slli_epi32( jointOfs, 1 ) ); // *kStride = *6
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

		// load keys and swizzle to get A + BCI data
		__m128i abci00 = _mm_loadu_si128( (__m128i*) key0adr0 );
		__m128i abci10 = _mm_loadu_si128( (__m128i*) key0adr1 );
		__m128i abci20 = _mm_loadu_si128( (__m128i*) key0adr2 );
		__m128i abci30 = _mm_loadu_si128( (__m128i*) key0adr3 );

		__m128i abci01 = _mm_loadu_si128( (__m128i*) key1adr0 );
		__m128i abci11 = _mm_loadu_si128( (__m128i*) key1adr1 );
		__m128i abci21 = _mm_loadu_si128( (__m128i*) key1adr2 );
		__m128i abci31 = _mm_loadu_si128( (__m128i*) key1adr3 );

		__m128i bci00 = _mm_srli_si128( abci00, 2 );
		__m128i bci10 = _mm_srli_si128( abci10, 2 );
		__m128i bci20 = _mm_srli_si128( abci20, 2 );
		__m128i bci30 = _mm_srli_si128( abci30, 2 );

		__m128i bci01 = _mm_srli_si128( abci01, 2 );
		__m128i bci11 = _mm_srli_si128( abci11, 2 );
		__m128i bci21 = _mm_srli_si128( abci21, 2 );
		__m128i bci31 = _mm_srli_si128( abci31, 2 );

		__m128i leftRotAi = _mm_unpackx_epi32( abci00, abci10, abci20, abci30 );
		__m128i rightRotAi = _mm_unpackx_epi32( abci01, abci11, abci21, abci31 );

		__m128i leftRotBCIi = _mm_unpackx_epi32( bci00, bci10, bci20, bci30 );
		__m128i rightRotBCIi = _mm_unpackx_epi32( bci01, bci11, bci21, bci31 );

		// Unpack 15-bit components
		leftRotAi = _mm_or_si128( _mm_slli_epi16( leftRotAi, 8 ), _mm_srli_epi16( leftRotAi, 8 ) ); // endian swap
		leftRotAi = _mm_srli_epi32( _mm_slli_epi32( leftRotAi, 17 ), 17 ); // 15-bit mask

		rightRotAi = _mm_or_si128( _mm_slli_epi16( rightRotAi, 8 ), _mm_srli_epi16( rightRotAi, 8 ) ); // endian swap
		rightRotAi = _mm_srli_epi32( _mm_slli_epi32( rightRotAi, 17 ), 17 ); // 15-bit mask

		__m128i leftRotBi = leftRotBCIi;
		leftRotBi = _mm_or_si128( _mm_slli_epi16( leftRotBi, 8 ), _mm_srli_epi16( leftRotBi, 8 ) ); // endian swap
		leftRotBi = _mm_srli_epi32( _mm_slli_epi32( leftRotBi, 16 ), 17 ); // 1-bit rshift + 15-bit mask

		__m128i rightRotBi = rightRotBCIi;
		rightRotBi = _mm_or_si128( _mm_slli_epi16( rightRotBi, 8 ), _mm_srli_epi16( rightRotBi, 8 ) ); // endian swap
		rightRotBi = _mm_srli_epi32( _mm_slli_epi32( rightRotBi, 16 ), 17 ); // 1-bit rshift + 15-bit mask

		__m128i leftRotCi = leftRotBCIi;
		leftRotCi = _mm_or_si128( _mm_slli_epi32( leftRotCi, 8 ), _mm_srli_epi32( leftRotCi, 24 ) ); // 8-bit rshift + endian swap
		leftRotCi = _mm_or_si128( _mm_slli_epi16( leftRotCi, 8 ), _mm_srli_epi16( leftRotCi, 8 ) ); // endian swap
		leftRotCi = _mm_srli_epi32( _mm_slli_epi32( leftRotCi, 7 ), 17 ); // 10-bit rshift + 15-bit mask

		__m128i rightRotCi = rightRotBCIi;
		rightRotCi = _mm_or_si128( _mm_slli_epi32( rightRotCi, 8 ), _mm_srli_epi32( rightRotCi, 24 ) ); // 8-bit rshift + endian swap
		rightRotCi = _mm_or_si128( _mm_slli_epi16( rightRotCi, 8 ), _mm_srli_epi16( rightRotCi, 8 ) ); // endian swap
		rightRotCi = _mm_srli_epi32( _mm_slli_epi32( rightRotCi, 7 ), 17 ); // 10-bit rshift + 15-bit mask

		__m128i leftIndex = _mm_srli_epi32( _mm_slli_epi32( leftRotBCIi, 6 ), 30 ); // 24-bit rshift + 2-bit mask
		__m128i rightIndex = _mm_srli_epi32( _mm_slli_epi32( rightRotBCIi, 6 ), 30 ); // 24-bit rshift + 2-bit mask

		// convert to floating point
		__m128 leftRotA, leftRotB, leftRotC, rightRotA, rightRotB, rightRotC;
		leftRotA = _mm_madd_ps( _mm_cvtepi32_ps( leftRotAi ), kQuatScale, kQuatOffset );
		leftRotB = _mm_madd_ps( _mm_cvtepi32_ps( leftRotBi ), kQuatScale, kQuatOffset );
		leftRotC = _mm_madd_ps( _mm_cvtepi32_ps( leftRotCi ), kQuatScale, kQuatOffset );
		rightRotA = _mm_madd_ps( _mm_cvtepi32_ps( rightRotAi ), kQuatScale, kQuatOffset );
		rightRotB = _mm_madd_ps( _mm_cvtepi32_ps( rightRotBi ), kQuatScale, kQuatOffset );
		rightRotC = _mm_madd_ps( _mm_cvtepi32_ps( rightRotCi ), kQuatScale, kQuatOffset );

		// squared length of abc
		__m128 leftRotDsqr = k1_0f;
		leftRotDsqr = _mm_nmsub_ps( leftRotA, leftRotA, leftRotDsqr );
		leftRotDsqr = _mm_nmsub_ps( leftRotB, leftRotB, leftRotDsqr );
		leftRotDsqr = _mm_nmsub_ps( leftRotC, leftRotC, leftRotDsqr );

		__m128 rightRotDsqr = k1_0f;
		rightRotDsqr = _mm_nmsub_ps( rightRotA, rightRotA, rightRotDsqr );
		rightRotDsqr = _mm_nmsub_ps( rightRotB, rightRotB, rightRotDsqr );
		rightRotDsqr = _mm_nmsub_ps( rightRotC, rightRotC, rightRotDsqr );

		// d = sqrt( 1 - a^2 - b^2 - c^2 )
		__m128 leftRotD = _mm_sqrt_ps( leftRotDsqr );
		__m128 rightRotD = _mm_sqrt_ps( rightRotDsqr );

		// Now shuffle components to insert the derived value.
		__m128i leftSelDABC = _mm_cmpeq_epi32( leftIndex, kDABC );
		__m128i leftSelADBC = _mm_cmpeq_epi32( leftIndex, kADBC );
		__m128i leftSelABDC = _mm_cmpeq_epi32( leftIndex, kABDC );
		__m128i leftSelABCD = _mm_cmpeq_epi32( leftIndex, kABCD );

		__m128i rightSelDABC = _mm_cmpeq_epi32( rightIndex, kDABC );
		__m128i rightSelADBC = _mm_cmpeq_epi32( rightIndex, kADBC );
		__m128i rightSelABDC = _mm_cmpeq_epi32( rightIndex, kABDC );
		__m128i rightSelABCD = _mm_cmpeq_epi32( rightIndex, kABCD );

		__m128 leftRotX, leftRotY, leftRotZ, leftRotW, rightRotX, rightRotY, rightRotZ, rightRotW;
		leftRotX = _mm_sel_ps( leftRotA, leftRotD, *(__m128*)&leftSelDABC );
		leftRotY = _mm_sel_ps( _mm_sel_ps( leftRotB, leftRotA, *(__m128*)&leftSelDABC ), leftRotD, *(__m128*)&leftSelADBC );
		leftRotZ = _mm_sel_ps( _mm_sel_ps( leftRotB, leftRotC, *(__m128*)&leftSelABCD ), leftRotD, *(__m128*)&leftSelABDC );
		leftRotW = _mm_sel_ps( leftRotC, leftRotD, *(__m128*)&leftSelABCD );

		rightRotX = _mm_sel_ps( rightRotA, rightRotD, *(__m128*)&rightSelDABC );
		rightRotY = _mm_sel_ps( _mm_sel_ps( rightRotB, rightRotA, *(__m128*)&rightSelDABC ), rightRotD, *(__m128*)&rightSelADBC );
		rightRotZ = _mm_sel_ps( _mm_sel_ps( rightRotB, rightRotC, *(__m128*)&rightSelABCD ), rightRotD, *(__m128*)&rightSelABDC );
		rightRotW = _mm_sel_ps( rightRotC, rightRotD, *(__m128*)&rightSelABCD );

		// Slerp each (left, right) rotation pair with the tweening factors computed earlier

		// Slerp rotations : compute dot product
		__m128 dotRot = _mm_mul_ps( leftRotX, rightRotX );
		dotRot = _mm_madd_ps( leftRotY, rightRotY, dotRot );
		dotRot = _mm_madd_ps( leftRotZ, rightRotZ, dotRot );
		dotRot = _mm_madd_ps( leftRotW, rightRotW, dotRot );

		// Slerp rotations : determine target rotation (right or -right)
		__m128 selNegDotRot = _mm_cmpgt_ps( _mm_setzero_ps(), dotRot );
		__m128 targetSign = _mm_sel_ps( k1_0f, kNeg1_0f, selNegDotRot );
		__m128 cosAngle = _mm_mul_ps( dotRot, targetSign );

		// Slerp rotations : evaluate angle = acos(cosAngle) (MacLaurin)
		__m128 oneMinusCosAngle = _mm_sub_ps( k1_0f, cosAngle );
		__m128 acosEval3 = _mm_mul_ps( acos3, cosAngle );
		__m128 acosEval2 = _mm_nmsub_ps( acosEval3, cosAngle, acos2 );
		__m128 acosEval1 = _mm_nmsub_ps( acosEval2, cosAngle, acos1 );
		__m128 acosEval0 = _mm_nmsub_ps( acosEval1, cosAngle, acos0 );
		__m128 rsqTmp = _mm_rsqrt_ps( oneMinusCosAngle );
		__m128 angle = _mm_mul_ps( acosEval0, _mm_nmsub_ps( cosAngle, rsqTmp, rsqTmp ) );

		// Slerp rotations : evaluate sina = sin(angle) (Taylor)
		__m128 sqAngle = _mm_mul_ps( angle, angle );
		__m128 sinaEval3 = _mm_madd_ps( sin3, sqAngle, sin2 );
		__m128 sinaEval2 = _mm_madd_ps( sinaEval3, sqAngle, sin1 );
		__m128 sinaEval1 = _mm_madd_ps( sinaEval2, sqAngle, sin0 );
		__m128 sinaEval0 = _mm_madd_ps( sinaEval1, sqAngle, k1_0f );
		__m128 sina = _mm_mul_ps( sinaEval0, angle );

		// Slerp rotations : evaluate sinb = sin(t*angle) (Taylor)
		__m128 tAngle = _mm_mul_ps( t, angle );
		__m128 sqTAngle = _mm_mul_ps( tAngle, tAngle );
		__m128 sinbEval3 = _mm_madd_ps( sin3, sqTAngle, sin2 );
		__m128 sinbEval2 = _mm_madd_ps( sinbEval3, sqTAngle, sin1 );
		__m128 sinbEval1 = _mm_madd_ps( sinbEval2, sqTAngle, sin0 );
		__m128 sinbEval0 = _mm_madd_ps( sinbEval1, sqTAngle, k1_0f );
		__m128 sinb = _mm_mul_ps( sinbEval0, tAngle );

		// Slerp rotations : evaluate sinc = sin((1-t)*angle) (Taylor)
		__m128 oneMinusT = _mm_sub_ps( k1_0f, t );
		__m128 oneMinusTAngle = _mm_mul_ps( oneMinusT, angle );
		__m128 sqOneMinusTAngle = _mm_mul_ps( oneMinusTAngle, oneMinusTAngle );
		__m128 sincEval3 = _mm_madd_ps( sin3, sqOneMinusTAngle, sin2 );
		__m128 sincEval2 = _mm_madd_ps( sincEval3, sqOneMinusTAngle, sin1 );
		__m128 sincEval1 = _mm_madd_ps( sincEval2, sqOneMinusTAngle, sin0 );
		__m128 sincEval0 = _mm_madd_ps( sincEval1, sqOneMinusTAngle, k1_0f );
		__m128 sinc = _mm_mul_ps( sincEval0, oneMinusTAngle );   

		// Select either sinc/sina or sinb/sina depending on the quadrant
		__m128 invSina = _mm_rcp_ps( sina );
		invSina = _mm_madd_ps( _mm_nmsub_ps( sina, invSina, k1_0f ), invSina, invSina );

		__m128 fLeftTmp = _mm_mul_ps( sinc, invSina );
		__m128 fRightTmp = _mm_mul_ps( sinb, invSina );
		__m128 selRange = _mm_cmpgt_ps( cosAngle, oneMinusEpsilon );
		__m128 fLeft = _mm_mul_ps( _mm_sel_ps( fLeftTmp, oneMinusT, selRange ), targetSign );
		__m128 fRight = _mm_sel_ps( fRightTmp, t, selRange );

		// Actual blend
		__m128 outRotX = _mm_madd_ps( rightRotX, fRight, _mm_mul_ps( leftRotX, fLeft ) );
		__m128 outRotY = _mm_madd_ps( rightRotY, fRight, _mm_mul_ps( leftRotY, fLeft ) );
		__m128 outRotZ = _mm_madd_ps( rightRotZ, fRight, _mm_mul_ps( leftRotZ, fLeft ) );
		__m128 outRotW = _mm_madd_ps( rightRotW, fRight, _mm_mul_ps( leftRotW, fLeft ) );

		// Unswizzle output rotation
		// (X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3 -> X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3)
		__m128 outRot0 = outRotX, outRot1 = outRotY, outRot2 = outRotZ, outRot3 = outRotW;
		_MM_TRANSPOSE4_PS( outRot0, outRot1, outRot2, outRot3 );

		// output
		output[ animTable[0] * 3 ] = outRot0;
		output[ animTable[1] * 3 ] = outRot1;
		output[ animTable[2] * 3 ] = outRot2;
		output[ animTable[3] * 3 ] = outRot3;

		// kStride(6) * sumBits
		__m128i intraInc = _mm_add_epi32( numBits, _mm_shuffle_epi32( numBits, _MM_SHUFFLE( 2, 3, 0, 1 ) ) );
		intraInc = _mm_add_epi32( intraInc, _mm_shuffle_epi32( intraInc, _MM_SHUFFLE( 1, 0, 3, 2 ) ) );
		intraInc = _mm_add_epi32( _mm_slli_epi32( intraInc, 2 ), _mm_slli_epi32( intraInc, 1 ) );

		// Advance pointers
		intraBitsOffset = _mm_add_epi32( intraBitsOffset, intraBitsOffsetInc );

		intraOfs = _mm_add_epi32( intraOfs, intraInc );
		boundsOfs = _mm_add_epi32( boundsOfs, strideInc );
		animTable += 4;
	}
}
