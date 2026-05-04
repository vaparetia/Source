/* SCE CONFIDENTIAL
* PlayStation(R)Edge 1.2.0
* Copyright (C) 2010 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#include "edge/anim/edgeanim_structs_ps3.h"
#include "edge/anim/edgeanim_windows.h"

extern "C"
void edgeAnimBlendJointsLinear(struct EdgeAnimJointTransform* outputJoints, 
							   uint8_t* outputWeights,
							   const struct EdgeAnimJointTransform* leftJoints,
							   const uint8_t* leftWeights,
							   const struct EdgeAnimJointTransform* rightJoints, 
							   const uint8_t* rightWeights,
							   float alpha,
							   unsigned int count)
{   
	EDGE_ASSERT(EDGE_IS_ALIGNED(outputJoints, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(outputWeights, 4U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(leftJoints, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(leftWeights, 4U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(rightJoints, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(rightWeights, 4U));
	EDGE_ASSERT(count);

	const __m128 splatAlpha = _mm_set1_ps( alpha );

	// Constants : Misc
	const __m128i k255 = _mm_set1_epi32( 255 );
	const __m128 k1_0f = _mm_set1_ps( 1.0f );
	const __m128 kNeg1_0f = _mm_set1_ps( -1.0f );
	const __m128 oneMinusEpsilon = _mm_set1_ps( 0.999f );

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

	// Constants : Scale for weights (255 = 1.0f)
	const __m128 weightScaleF2I = _mm_set1_ps( 255.0f );
	const __m128 weightScaleI2F = _mm_set1_ps( 1.0f / 255.0f );

	// Pointers
	const __m128* leftJ = (const __m128*) leftJoints;
	const __m128* rightJ = (const __m128*) rightJoints;
	__m128* outputJ = (__m128*) outputJoints;

	const uint32_t* leftW = (const uint32_t*) leftWeights;
	const uint32_t* rightW = (const uint32_t*) rightWeights;
	uint32_t* outputW = (uint32_t*) outputWeights;

	// Weight Select Mask - Uses outputWeights as 'safe' read if not in use
	const __m128i selLeftNonNull = _mm_set1_epi32( leftW != 0 ? 0 : 255 );
	const __m128i selRightNonNull = _mm_set1_epi32( rightW != 0 ? 0 : 255 );
	leftW = ( leftW != 0 ? leftW : outputW );
	rightW = ( rightW != 0 ? rightW : outputW );

	// Vectorised loop count (4 iterations at a time, rounded up)
	for( unsigned int count4 = (count + 3) >> 2; count4; count4-- )
	{
		// Load blend weights
		__m128i leftByte  = _mm_loadu_si128( (__m128i*) leftW );
		__m128i rightByte = _mm_loadu_si128( (__m128i*) rightW );

		// Unpack bytes to words
		leftByte = _mm_unpacklo_epi8( leftByte, leftByte );
		leftByte = _mm_unpacklo_epi16( leftByte, leftByte );
		leftByte = _mm_srli_epi32( leftByte, 24 );

		rightByte = _mm_unpacklo_epi8( rightByte, rightByte );
		rightByte = _mm_unpacklo_epi16( rightByte, rightByte );
		rightByte = _mm_srli_epi32( rightByte, 24 );

		// Set to defaults if don't exist
		leftByte = _mm_or_si128( _mm_andnot_si128( selLeftNonNull, leftByte ), selLeftNonNull );
		rightByte = _mm_or_si128( _mm_andnot_si128( selRightNonNull, rightByte ), selRightNonNull );

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

		// Valid (> 0) weights
		__m128i leftValid = _mm_cmpgt_epi32( leftByte, _mm_setzero_si128() );
		__m128i rightValid = _mm_cmpgt_epi32( rightByte, _mm_setzero_si128() );
		__m128i bothValidI = _mm_and_si128( leftValid, rightValid );
		__m128i anyValidI = _mm_or_si128( leftValid, rightValid );
		__m128 bothValid = *(__m128*)&bothValidI;
		__m128 anyValid = *(__m128*)&anyValidI;

		// Force left / right
		__m128i forceLeftI = _mm_andnot_si128( bothValidI, leftValid );
		__m128i forceRightI = _mm_andnot_si128( bothValidI, rightValid );
		__m128 forceLeft = *(__m128*)&forceLeftI;
		__m128 forceRight = *(__m128*)&forceRightI;

		// Blend weights + clamp
		__m128 blendedWeight0 = _mm_madd_ps( rightWeight, t, _mm_mul_ps( leftWeight, oneMinusT ) );
		__m128 blendedWeight1 = _mm_mul_ps( blendedWeight0, weightScaleF2I );
		__m128i blendedWeight2 = _mm_cvtps_epi32( blendedWeight1 );
		__m128i blendedWeight3 = _mm_min_epi16( blendedWeight2, k255 );
		__m128i blendedWeight = _mm_max_epi16( blendedWeight3, _mm_setzero_si128() );

		// Determine output weights
		__m128i outputWeight = _mm_and_si128( blendedWeight, bothValidI );
		outputWeight = _mm_or_si128( outputWeight, _mm_and_si128( leftByte, forceLeftI ) );
		outputWeight = _mm_or_si128( outputWeight, _mm_and_si128( rightByte, forceRightI ) );

		// Packs weights into lowest word
		outputWeight = _mm_packus_epi16( outputWeight, outputWeight );
		outputWeight = _mm_packus_epi16( outputWeight, outputWeight );

		// Output weights
		*outputW = *(uint32_t*)(&outputWeight);

		// Load + swizzle translations
		__m128 leftTransX =  leftJ[1]; __m128 rightTransX =  rightJ[1];
		__m128 leftTransY =  leftJ[4]; __m128 rightTransY =  rightJ[4];
		__m128 leftTransZ =  leftJ[7]; __m128 rightTransZ =  rightJ[7];
		__m128 leftTransW = leftJ[10]; __m128 rightTransW = rightJ[10];
		_MM_TRANSPOSE4_PS( leftTransX, leftTransY, leftTransZ, leftTransW );
		_MM_TRANSPOSE4_PS( rightTransX, rightTransY, rightTransZ, rightTransW );

		// Blend translations
		__m128 blendedTransX = _mm_madd_ps( rightTransX, t, _mm_mul_ps( leftTransX, oneMinusT ) );
		__m128 blendedTransY = _mm_madd_ps( rightTransY, t, _mm_mul_ps( leftTransY, oneMinusT ) );
		__m128 blendedTransZ = _mm_madd_ps( rightTransZ, t, _mm_mul_ps( leftTransZ, oneMinusT ) );

		// Select, swizzle + output translations
		__m128 outputTransX = _mm_and_ps( blendedTransX, bothValid );
		__m128 outputTransY = _mm_and_ps( blendedTransY, bothValid );
		__m128 outputTransZ = _mm_and_ps( blendedTransZ, bothValid );
		__m128 outputTransW = k1_0f;
		outputTransX = _mm_or_ps( outputTransX, _mm_and_ps( leftTransX, forceLeft ) );
		outputTransY = _mm_or_ps( outputTransY, _mm_and_ps( leftTransY, forceLeft ) );
		outputTransZ = _mm_or_ps( outputTransZ, _mm_and_ps( leftTransZ, forceLeft ) );
		outputTransX = _mm_or_ps( outputTransX, _mm_and_ps( rightTransX, forceRight ) );
		outputTransY = _mm_or_ps( outputTransY, _mm_and_ps( rightTransY, forceRight ) );
		outputTransZ = _mm_or_ps( outputTransZ, _mm_and_ps( rightTransZ, forceRight ) );
		_MM_TRANSPOSE4_PS( outputTransX, outputTransY, outputTransZ, outputTransW );
		outputJ[1]  = outputTransX;
		outputJ[4]  = outputTransY;
		outputJ[7]  = outputTransZ;
		outputJ[10] = outputTransW;

		// Load + swizzle scales
		__m128 leftScaleX =  leftJ[2]; __m128 rightScaleX =  rightJ[2];
		__m128 leftScaleY =  leftJ[5]; __m128 rightScaleY =  rightJ[5];
		__m128 leftScaleZ =  leftJ[8]; __m128 rightScaleZ =  rightJ[8];
		__m128 leftScaleW = leftJ[11]; __m128 rightScaleW = rightJ[11];
		_MM_TRANSPOSE4_PS( leftScaleX, leftScaleY, leftScaleZ, leftScaleW );
		_MM_TRANSPOSE4_PS( rightScaleX, rightScaleY, rightScaleZ, rightScaleW );

		// Blend scales
		__m128 blendedScaleX = _mm_madd_ps( rightScaleX, t, _mm_mul_ps( leftScaleX, oneMinusT ) );
		__m128 blendedScaleY = _mm_madd_ps( rightScaleY, t, _mm_mul_ps( leftScaleY, oneMinusT ) );
		__m128 blendedScaleZ = _mm_madd_ps( rightScaleZ, t, _mm_mul_ps( leftScaleZ, oneMinusT ) );

		// Select, swizzle + output scales
		__m128 outputScaleX = _mm_and_ps( blendedScaleX, bothValid );
		__m128 outputScaleY = _mm_and_ps( blendedScaleY, bothValid );
		__m128 outputScaleZ = _mm_and_ps( blendedScaleZ, bothValid );
		__m128 outputScaleW = k1_0f;
		outputScaleX = _mm_or_ps( outputScaleX, _mm_and_ps( leftScaleX, forceLeft ) );
		outputScaleY = _mm_or_ps( outputScaleY, _mm_and_ps( leftScaleY, forceLeft ) );
		outputScaleZ = _mm_or_ps( outputScaleZ, _mm_and_ps( leftScaleZ, forceLeft ) );
		outputScaleX = _mm_or_ps( outputScaleX, _mm_and_ps( rightScaleX, forceRight ) );
		outputScaleY = _mm_or_ps( outputScaleY, _mm_and_ps( rightScaleY, forceRight ) );
		outputScaleZ = _mm_or_ps( outputScaleZ, _mm_and_ps( rightScaleZ, forceRight ) );
		outputScaleX = _mm_or_ps( outputScaleX, _mm_andnot_ps( anyValid, k1_0f ) );
		outputScaleY = _mm_or_ps( outputScaleY, _mm_andnot_ps( anyValid, k1_0f ) );
		outputScaleZ = _mm_or_ps( outputScaleZ, _mm_andnot_ps( anyValid, k1_0f ) );
		_MM_TRANSPOSE4_PS( outputScaleX, outputScaleY, outputScaleZ, outputScaleW );
		outputJ[2]  = outputScaleX;
		outputJ[5]  = outputScaleY;
		outputJ[8]  = outputScaleZ;
		outputJ[11] = outputScaleW;

		// Load + swizzle rotations
		__m128 leftRotX = leftJ[0]; __m128 rightRotX = rightJ[0];
		__m128 leftRotY = leftJ[3];	__m128 rightRotY = rightJ[3];
		__m128 leftRotZ = leftJ[6];	__m128 rightRotZ = rightJ[6];
		__m128 leftRotW = leftJ[9];	__m128 rightRotW = rightJ[9];
		_MM_TRANSPOSE4_PS( leftRotX, leftRotY, leftRotZ, leftRotW );
		_MM_TRANSPOSE4_PS( rightRotX, rightRotY, rightRotZ, rightRotW );

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

		// Do the actual blend
		__m128 blendedRotX = _mm_madd_ps( rightRotX, fRight, _mm_mul_ps( leftRotX, fLeft ) );
		__m128 blendedRotY = _mm_madd_ps( rightRotY, fRight, _mm_mul_ps( leftRotY, fLeft ) );
		__m128 blendedRotZ = _mm_madd_ps( rightRotZ, fRight, _mm_mul_ps( leftRotZ, fLeft ) );
		__m128 blendedRotW = _mm_madd_ps( rightRotW, fRight, _mm_mul_ps( leftRotW, fLeft ) );

		// Select, swizzle + output rotations
		__m128 outputRotX = _mm_and_ps( blendedRotX, bothValid );
		__m128 outputRotY = _mm_and_ps( blendedRotY, bothValid );
		__m128 outputRotZ = _mm_and_ps( blendedRotZ, bothValid );
		__m128 outputRotW = _mm_and_ps( blendedRotW, bothValid );
		outputRotX = _mm_or_ps( outputRotX, _mm_and_ps( leftRotX, forceLeft ) );
		outputRotY = _mm_or_ps( outputRotY, _mm_and_ps( leftRotY, forceLeft ) );
		outputRotZ = _mm_or_ps( outputRotZ, _mm_and_ps( leftRotZ, forceLeft ) );
		outputRotW = _mm_or_ps( outputRotW, _mm_and_ps( leftRotW, forceLeft ) );
		outputRotX = _mm_or_ps( outputRotX, _mm_and_ps( rightRotX, forceRight ) );
		outputRotY = _mm_or_ps( outputRotY, _mm_and_ps( rightRotY, forceRight ) );
		outputRotZ = _mm_or_ps( outputRotZ, _mm_and_ps( rightRotZ, forceRight ) );
		outputRotW = _mm_or_ps( outputRotW, _mm_and_ps( rightRotW, forceRight ) );
		outputRotW = _mm_or_ps( outputRotW, _mm_andnot_ps( anyValid, k1_0f ) );
		_MM_TRANSPOSE4_PS( outputRotX, outputRotY, outputRotZ, outputRotW );
		outputJ[0] = outputRotX;
		outputJ[3] = outputRotY;
		outputJ[6] = outputRotZ;
		outputJ[9] = outputRotW;

		// Advance pointers
		leftJ += 12;
		rightJ += 12;
		outputJ += 12;
		leftW++;
		rightW++;
		outputW++;
	}
}
