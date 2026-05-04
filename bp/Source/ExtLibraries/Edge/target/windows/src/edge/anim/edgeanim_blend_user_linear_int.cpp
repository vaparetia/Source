/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/anim/edgeanim_structs_ps3.h"
#include "edge/anim/edgeanim_windows.h"

extern "C"
void edgeAnimBlendUserLinear(float* outputChannels,
                             uint8_t* outputWeights,
                             const float* leftChannels,
                             const uint8_t* leftWeights,
                             const float* rightChannels,
                             const uint8_t* rightWeights,
                             const uint8_t* channelFlags,
                             float alpha, 
                             unsigned int count)
{
	EDGE_ASSERT(EDGE_IS_ALIGNED(outputChannels, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(outputWeights, 4U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(leftChannels, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(leftWeights, 4U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(rightChannels, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(rightWeights, 4U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(channelFlags, 4U));
	EDGE_ASSERT(count);

	const __m128 splatAlpha = _mm_set1_ps( alpha );
	
	// Constants
	const __m128i k255 = _mm_set1_epi32( 255 );
	const __m128 k1_0f = _mm_set1_ps( 1.0f );
	const __m128 kNeg1_0f = _mm_set1_ps( -1.0f );
	
	// Constants : Scale for weights (255 = 1.0f)
	const __m128 weightScaleF2I = _mm_set1_ps( 255.0f );
	const __m128 weightScaleI2F = _mm_set1_ps( 1.0f / 255.0f );

	// Channel Pointers
	const __m128* __restrict leftC = (const __m128* __restrict) leftChannels;
	const __m128* __restrict rightC = (const __m128* __restrict) rightChannels;
	__m128* __restrict outputC = (__m128* __restrict) outputChannels;

	const uint32_t* inputFlags = (const uint32_t*) channelFlags;
	const uint32_t* leftW = (const uint32_t*) leftWeights;
	const uint32_t* rightW = (const uint32_t*) rightWeights;
	uint32_t* outputW = (uint32_t*) outputWeights;

	// Weight Select Mask - Uses outputWeights as 'safe' read if not in use
	const __m128i selLeftNonNull = _mm_set1_epi32( leftW != 0 ? -1 : 0 );
	const __m128i selRightNonNull = _mm_set1_epi32( rightW != 0 ? -1 : 0 );
	const __m128i leftDefaultByte = _mm_andnot_si128( selLeftNonNull, k255 );
	const __m128i rightDefaultByte = _mm_andnot_si128( selRightNonNull, k255 );
	leftW = ( leftW != 0 ? leftW : outputW );
	rightW = ( rightW != 0 ? rightW : outputW );

	// Flags Select Mask - Uses outputWeights as 'safe' read if not in use
	const __m128i flagClamp01Mask = _mm_set1_epi32( EDGE_ANIM_USER_CHANNEL_FLAG_CLAMP01 );
	const __m128i selFlagsNonNull = _mm_set1_epi32( inputFlags != 0 ? -1 : 0 );
	inputFlags = ( inputFlags != 0 ? inputFlags : outputW );

	for( unsigned int count4 = (count + 3) >> 2; count4; count4-- )
	{
		// Load flags + blend weights
		__m128i flags = _mm_loadu_si128( (__m128i*) inputFlags );
		__m128i leftByte  = _mm_loadu_si128( (__m128i*) leftW );
		__m128i rightByte = _mm_loadu_si128( (__m128i*) rightW );

		// Unpack bytes to words
		flags = _mm_unpacklo_epi8( flags, flags );
		flags = _mm_unpacklo_epi16( flags, flags );

		leftByte = _mm_unpacklo_epi8( leftByte, leftByte );
		leftByte = _mm_unpacklo_epi16( leftByte, leftByte );

		rightByte = _mm_unpacklo_epi8( rightByte, rightByte );
		rightByte = _mm_unpacklo_epi16( rightByte, rightByte );

		// 8-bit mask
		leftByte = _mm_srli_epi32( _mm_slli_epi32( leftByte, 24 ), 24 );
		rightByte = _mm_srli_epi32( _mm_slli_epi32( rightByte, 24 ), 24 );

		// Set to defaults if don't exist
		flags = _mm_and_si128( flags, selFlagsNonNull );
		leftByte = _mm_or_si128( _mm_and_si128( leftByte, selLeftNonNull ), leftDefaultByte );
		rightByte = _mm_or_si128( _mm_and_si128( rightByte, selRightNonNull ), rightDefaultByte );

		// Scale weights
		__m128 leftWeight = _mm_mul_ps( _mm_cvtepi32_ps( leftByte ), weightScaleI2F );
		__m128 rightWeight =_mm_mul_ps( _mm_cvtepi32_ps( rightByte ), weightScaleI2F ); ;

		// Blend factor
		__m128 selRightGreater = _mm_cmpgt_ps( rightWeight, leftWeight );
		__m128 tmpBlendLeft = _mm_mul_ps( splatAlpha, rightWeight );
		__m128 tmpBlendRight = _mm_sub_ps( _mm_madd_ps( splatAlpha, leftWeight, rightWeight ), leftWeight );
		__m128 tmpBlend = _mm_sel_ps( tmpBlendLeft, tmpBlendRight, selRightGreater );
		__m128 divW = _mm_max_ps( leftWeight, rightWeight );
		__m128 invDivWEst = _mm_rcp_ps( divW );
		__m128 invDivW0 = _mm_mul_ps( divW, kNeg1_0f );
		__m128 invDivW1 = _mm_mul_ps( invDivW0, invDivWEst );
		__m128 invDivW2 = _mm_mul_ps( invDivW1, invDivWEst );
		__m128 invDivW3 = _mm_add_ps( invDivW2, invDivWEst );
		__m128 invDivW  = _mm_add_ps( invDivW3, invDivWEst );
		__m128 t = _mm_mul_ps( tmpBlend, invDivW );
		__m128 oneMinusT = _mm_sub_ps( k1_0f, t );

		// Blend mode
		__m128i modeClampI = _mm_cmpeq_epi32( _mm_and_si128( flags, flagClamp01Mask ), flagClamp01Mask );
		__m128 modeClamp = *(__m128*)&modeClampI;

		// Load Values
		__m128 leftValue = *leftC;
		__m128 rightValue = *rightC;

		// Valid (> 0) weights
		__m128i leftValid = _mm_cmpgt_epi32( leftByte, _mm_setzero_si128() );
		__m128i rightValid = _mm_cmpgt_epi32( rightByte, _mm_setzero_si128() );
		__m128i bothValidI = _mm_and_si128( leftValid, rightValid );
		__m128 bothValid = *(__m128*)&bothValidI;

		// Force left / right
		__m128i forceLeftI = _mm_andnot_si128( bothValidI, leftValid );
		__m128i forceRightI = _mm_andnot_si128( bothValidI, rightValid );
		__m128 forceLeft = *(__m128*)&forceLeftI;
		__m128 forceRight = *(__m128*)&forceRightI;

		// Blended value + weight
		__m128 blendedValue = _mm_madd_ps( rightValue, t, _mm_mul_ps( leftValue, oneMinusT ) );

		__m128 blendedWeight0 = _mm_madd_ps( rightWeight, t, _mm_mul_ps( leftWeight, oneMinusT ) );
		__m128 blendedWeight1 = _mm_mul_ps( blendedWeight0, weightScaleF2I );
		__m128i blendedWeight2 = _mm_cvtps_epi32( blendedWeight1 );
		__m128i blendedWeight3 = _mm_min_epi16( blendedWeight2, k255 );
		__m128i blendedWeight = _mm_max_epi16( blendedWeight3, _mm_setzero_si128() );

		// Determine output value + weight
		__m128 outputValue = _mm_and_ps( blendedValue, bothValid );
		outputValue = _mm_or_ps( outputValue, _mm_and_ps( leftValue, forceLeft ) );
		outputValue = _mm_or_ps( outputValue, _mm_and_ps( rightValue, forceRight ) );

		__m128i outputWeight = _mm_and_si128( blendedWeight, bothValidI );
		outputWeight = _mm_or_si128( outputWeight, _mm_and_si128( leftByte, forceLeftI ) );
		outputWeight = _mm_or_si128( outputWeight, _mm_and_si128( rightByte, forceRightI ) );

		// Clamp to [0..1]
		__m128 clampedValue = outputValue;
		clampedValue = _mm_max_ps( clampedValue, _mm_setzero_ps() );
		clampedValue = _mm_min_ps( clampedValue, k1_0f );
		outputValue = _mm_sel_ps( outputValue, clampedValue, modeClamp );

		// Packs weights into lowest word
		outputWeight = _mm_packus_epi16( outputWeight, outputWeight );
		outputWeight = _mm_packus_epi16( outputWeight, outputWeight );

		// Output value + weight
		*outputC = outputValue;
		*outputW = *(uint32_t*)(&outputWeight);

		// Advance pointers
		leftC++;
		rightC++;
		outputC++;
		leftW++;
		rightW++;
		outputW++;
		inputFlags++;
	}
}
