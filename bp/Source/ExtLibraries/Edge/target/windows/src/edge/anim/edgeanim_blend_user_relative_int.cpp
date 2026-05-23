/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/anim/edgeanim_structs_ps3.h"
#include "edge/anim/edgeanim_windows.h"

extern "C"
void edgeAnimBlendUserRelative(float* outputChannels, 
                               uint8_t* outputWeights,
                               const float* leftChannels,
                               const uint8_t* leftWeights,
                               const float* rightChannels, 
                               const uint8_t* rightWeights,
                               const uint8_t* channelFlags,
                               float alpha, 
                               EdgeAnimRelativeBlendMode blendMode,                            
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

	// Constants : Scale for weights (255 = 1.0f)
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
	const __m128i flagMinMaxMask = _mm_set1_epi32( EDGE_ANIM_USER_CHANNEL_FLAG_MINMAX );
	const __m128i selFlagsNonNull = _mm_set1_epi32( inputFlags != 0 ? -1 : 0 );
	inputFlags = ( inputFlags != 0 ? inputFlags : outputW );

	// Blendmode
	const __m128i splatBlendMode = _mm_set1_epi32( blendMode );
	const __m128i subtractI = _mm_cmpeq_epi32( splatBlendMode, _mm_set1_epi32( EDGE_ANIM_RELATIVE_COMPOSE_SUB ) );
	const __m128i composeI = _mm_or_si128( subtractI, _mm_cmpeq_epi32( splatBlendMode, _mm_set1_epi32( EDGE_ANIM_RELATIVE_COMPOSE_ADD ) ) );
	const __m128 subtract = *(__m128*)&subtractI;
	const __m128 compose = *(__m128*)&composeI;

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

		// Blend factor
		__m128 rightWeight =_mm_mul_ps( _mm_cvtepi32_ps( rightByte ), weightScaleI2F );
		__m128 blendFactor = _mm_sel_ps( _mm_mul_ps( rightWeight, splatAlpha ), k1_0f, compose );
		blendFactor = _mm_sel_ps( blendFactor, _mm_sub_ps( _mm_setzero_ps(), blendFactor ), subtract );

		// Blend mode
		__m128i modeClampI = _mm_cmpeq_epi32( _mm_and_si128( flags, flagClamp01Mask ), flagClamp01Mask );
		__m128i modeMinMax = _mm_cmpeq_epi32( _mm_and_si128( flags, flagMinMaxMask ), flagMinMaxMask );
		__m128 modeClamp = *(__m128*)&modeClampI;

		// Load Values
		__m128 leftValue = *leftC;
		__m128 rightValue = *rightC;
		__m128 leftGtRight = _mm_cmpgt_ps( leftValue, rightValue );
		__m128 rightGtLeft = _mm_cmpgt_ps( rightValue, leftValue );

		// Valid (> 0) weights
		__m128i leftValid = _mm_cmpgt_epi32( leftByte, _mm_setzero_si128() );
		__m128i rightValid = _mm_cmpgt_epi32( rightByte, _mm_setzero_si128() );
		__m128i bothValidI = _mm_and_si128( leftValid, rightValid );
		__m128 bothValid = *(__m128*)&bothValidI;

		// Force left / right
		__m128i forceWeightLeft = _mm_andnot_si128( bothValidI, _mm_andnot_si128( subtractI, leftValid ) );
		__m128i forceWeightRight = _mm_andnot_si128( bothValidI, _mm_andnot_si128( subtractI, _mm_and_si128( rightValid, composeI ) ) );
		__m128i forceValueLeft = _mm_or_si128( _mm_and_si128( subtractI, *(__m128i*)&rightGtLeft ), _mm_andnot_si128( subtractI, *(__m128i*)&leftGtRight ) );
		__m128i forceValueRight = _mm_andnot_si128( forceValueLeft, _mm_and_si128( modeMinMax, bothValidI ) );
		forceValueLeft = _mm_and_si128( forceValueLeft, _mm_and_si128( modeMinMax, bothValidI ) );
		forceValueLeft = _mm_or_si128( forceValueLeft, forceWeightLeft );
		forceValueRight = _mm_or_si128( forceValueRight, forceWeightRight );

		// Blended value + weight
		__m128 blendedValue = _mm_add_ps( _mm_mul_ps( blendFactor, rightValue ), leftValue );
		__m128i blendedWeight = _mm_min_epi16( _mm_add_epi32( leftByte, rightByte ), k255 );

		// Determine output value + weight
		__m128 outputValue = _mm_and_ps( blendedValue, bothValid );
		outputValue = _mm_or_ps( outputValue, _mm_and_ps( leftValue, *(__m128*)&forceValueLeft ) );
		outputValue = _mm_or_ps( outputValue, _mm_and_ps( rightValue, *(__m128*)&forceValueRight ) );

		__m128i outputWeight = _mm_and_si128( blendedWeight, bothValidI );
		outputWeight = _mm_or_si128( outputWeight, _mm_and_si128( leftByte, forceWeightLeft ) );
		outputWeight = _mm_or_si128( outputWeight, _mm_and_si128( rightByte, forceWeightRight ) );

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
