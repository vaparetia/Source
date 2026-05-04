/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <spu_intrinsics.h>

#include "edge/edge_assert.h"
#include "edge/anim/edgeanim_spu.h"

#warning sub-optimal version - using intrinisics implementation instead of asm

/**
 * @brief   Relative blend between joint transforms (for additive/subtractive animations)
 *          SPU intrinsics implementation.
 *
 * @note    Please refer to LibEdge-Overview_e.pdf for a description of partial
 *          animation logic and blending modes.
 *
 * @note    A reference C++ implementation of this function is available in
 *          target/reference/src/edge/anim/edgeanim_blend_joints_relative.cpp
 *
 * @note    This function is not called unless you change the makefile which 
 *          uses asm implementation by default.
 *
 * @param   outputJoints    Pointer to output joint transform array. 
 *                          Must be aligned to 16 bytes. 
 *                          Due to the SIMD nature of the processing, the 
 *                          outputJoints array must be a multiple of 4 entries 
 *                          in size. 
 * @param   outputWeights   Pointer to output joint weights array.
 *                          Must be aligned to 4 bytes.
 * @param   leftJoints      Pointer to left joint transform array.
 *                          Must be aligned to 16 bytes.
 * @param   leftWeights     Pointer to left joint weights array.
 *                          Must be aligned to 4 bytes.
 * @param   rightJoints     Pointer to right joint transform array.
 *                          Must be aligned to 16 bytes.
 * @param   rightWeights    Pointer to right joint weights array.
 *                          Must be aligned to 4 bytes.
 * @param   alpha           Blend factor (0-1)
 * @param   blendMode       Relative blend/compose mode
 * @param   count           Joint count. 
 */

extern "C"
void edgeAnimBlendJointsRelative(EdgeAnimJointTransform* outputJoints, 
								 uint8_t* outputWeights,
								 const EdgeAnimJointTransform* leftJoints,
								 const uint8_t* leftWeights,
								 const EdgeAnimJointTransform* rightJoints, 
								 const uint8_t* rightWeights,
								 float alpha, 
								 EdgeAnimRelativeBlendMode blendMode,
								 unsigned int count)
{
	EDGE_BOOKMARK(EDGE_BOOKMARK_ANIM_BLEND_JOINTS_ADDITIVE);

	EDGE_ASSERT(EDGE_IS_ALIGNED(outputJoints, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(outputWeights, 4U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(leftJoints, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(leftWeights, 4U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(rightJoints, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(rightWeights, 4U));
	EDGE_ASSERT(count);

	const vec_float4 splatAlpha = spu_splats(alpha);	

	// Constants : Misc
	const vec_float4 oneMinusEpsilon = {0.999f, 0.999f, 0.999f, 0.999f};
	const vec_uint4 zero = {0U, 0U, 0U, 0U};	
	const vec_float4 fone = {1.0f, 1.0f, 1.0f, 1.0f};
	const vec_float4 minusfone = {-1.0f, -1.0f, -1.0f, -1.0f};  
	const vec_uint4 kClampWU = {0xFFU, 0xFFU, 0xFFU, 0xFFU};

	// Constants : MacLaurin - Acos
	const vec_float4 acos0 = {1.5707963050, 1.5707963050, 1.5707963050, 1.5707963050};
	const vec_float4 acos1 = {0.2145988016, 0.2145988016, 0.2145988016, 0.2145988016};
	const vec_float4 acos2 = {0.0889789874, 0.0889789874, 0.0889789874, 0.0889789874};
	const vec_float4 acos3 = {0.0501743046, 0.0501743046, 0.0501743046, 0.0501743046};
	
	// Constants : Taylor - Sin
	const vec_float4 sin0 = {-0.1666666664, -0.1666666664, -0.1666666664, -0.1666666664};
	const vec_float4 sin1 = {0.0083333315, 0.0083333315, 0.0083333315, 0.0083333315};
	const vec_float4 sin2 = {-0.000198409, -0.000198409, -0.000198409, -0.000198409};
	const vec_float4 sin3 = {0.0000027526, 0.0000027526, 0.0000027526, 0.0000027526};

	// Constants : Scale for weights (255 = 1.0f)
	const vec_float4 cufltscale = {1.0039215686f, 1.0039215686f, 1.0039215686f, 1.0039215686f};

	// Constants : Shuffle masks
	const vec_uchar16 shufAAAA = {0x0,0x1,0x2,0x3,0x0,0x1,0x2,0x3,0x0,0x1,0x2,0x3,0x0,0x1,0x2,0x3};
	const vec_uchar16 shufBBBB = {0x4,0x5,0x6,0x7,0x4,0x5,0x6,0x7,0x4,0x5,0x6,0x7,0x4,0x5,0x6,0x7};
	const vec_uchar16 shufCCCC = {0x8,0x9,0xA,0xB,0x8,0x9,0xA,0xB,0x8,0x9,0xA,0xB,0x8,0x9,0xA,0xB};
	const vec_uchar16 shufDDDD = {0xC,0xD,0xE,0xF,0xC,0xD,0xE,0xF,0xC,0xD,0xE,0xF,0xC,0xD,0xE,0xF};
	const vec_uchar16 shufAaBb = {0x00,0x01,0x02,0x03,0x10,0x11,0x12,0x13,0x04,0x05,0x06,0x07,0x14,0x15,0x16,0x17};
	const vec_uchar16 shufBbAa = {0x04,0x05,0x06,0x07,0x14,0x15,0x16,0x17,0x00,0x01,0x02,0x03,0x10,0x11,0x12,0x13};
	const vec_uchar16 shufCcDd = {0x08,0x09,0x0A,0x0B,0x18,0x19,0x1A,0x1B,0x0C,0x0D,0x0E,0x0F,0x1C,0x1D,0x1E,0x1F};
	const vec_uchar16 shufDdCc = {0x0C,0x0D,0x0E,0x0F,0x1C,0x1D,0x1E,0x1F,0x08,0x09,0x0A,0x0B,0x18,0x19,0x1A,0x1B};
	const vec_uchar16 shufABcd = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F};
	const vec_uchar16 shufCDab = {0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17};
	const vec_uchar16 shufABCD = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
	const vec_uchar16 shufabcd = {0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F};
	const vec_uchar16 shuf000A000B000C000D = {0x80,0x80,0x80,0x00,0x80,0x80,0x80,0x01,0x80,0x80,0x80,0x02,0x80,0x80,0x80,0x03};
	const vec_uchar16 shuf000X000X000X000X = {0x80,0x80,0x80,0xC0,0x80,0x80,0x80,0xC0,0x80,0x80,0x80,0xC0,0x80,0x80,0x80,0xC0};
	const vec_uchar16 shufDHLPDHLPDHLPDHLP = {0x03,0x07,0x0B,0x0F,0x03,0x07,0x0B,0x0F,0x03,0x07,0x0B,0x0F,0x03,0x07,0x0B,0x0F};
	
	// Pointers
	const vec_float4* __restrict__ leftj = (const vec_float4* __restrict__) leftJoints;
	const vec_float4* __restrict__ rightj = (const vec_float4* __restrict__) rightJoints;
	const uint32_t* __restrict__ leftw = (const uint32_t* __restrict__) leftWeights;
	const uint32_t* __restrict__ rightw = (const uint32_t* __restrict__) rightWeights;
	vec_float4* __restrict__ outputj = (vec_float4* __restrict__) outputJoints;
	uint32_t* __restrict__ outputw = (uint32_t* __restrict__) outputWeights;

	// Compute shuffle masks to load weights. If NULL they will force to ones, otherwise zero extend bytes to ints
	vec_uint4 selLeftNonNull = (vec_uint4)si_clgti((vec_char16) spu_splats((uintptr_t) leftWeights), 0);
	vec_uint4 selRightNonNull = (vec_uint4)si_clgti((vec_char16) spu_splats((uintptr_t) rightWeights), 0);
	vec_uchar16 leftLoadShuffle = spu_sel(shuf000X000X000X000X, shuf000A000B000C000D, (vec_uchar16)selLeftNonNull);
	vec_uchar16 rightLoadShuffle = spu_sel(shuf000X000X000X000X, shuf000A000B000C000D, (vec_uchar16)selRightNonNull);

	// Constants computed outside of the loop for various flavours of additive/substractive blending mode
	vec_uint4 splatBlendMode = spu_splats((unsigned int)blendMode);
	vec_uint4 selSubtract = spu_cmpgt(spu_and(splatBlendMode, 0x01U), spu_splats(0U));
	vec_uint4 selCompose = spu_cmpgt(spu_and(splatBlendMode, 0x10U), spu_splats(0U));
	vec_uint4 selNotCompose = spu_nand(selCompose, selCompose);
	const vec_uint4 maskSignNegate = {0x80000000, 0x80000000, 0x80000000, 0x80000000};
	const vec_float4 mulNoWPos = {1.0f, 1.0f, 1.0f, 0.0f};
	const vec_float4 mulNoWNeg = {-1.0f, -1.0f, -1.0f, 0.0f};
	vec_uint4 maskConjugate = spu_sel(zero, maskSignNegate, selSubtract); 
	vec_float4 mulDeltaTrans = spu_sel(mulNoWPos, mulNoWNeg, selSubtract);
	vec_uint4 shufCompose = spu_sel((vec_uint4) shufABCD, (vec_uint4) shufabcd, selCompose);	
	vec_uchar16 shufSwapRots = spu_sel((vec_uchar16) shufabcd, (vec_uchar16) shufABCD, (vec_uchar16) selSubtract);
	 
	// Vectorised loop count (4 iterations at a time, rounded up)
	int count4 = (count + 3) >> 2;

	for (; count4; count4--) {
		// Load joint data (x4)
		vec_float4 leftRot0 = leftj[0];
		vec_float4 leftRot1 = leftj[3];
		vec_float4 leftRot2 = leftj[6];
		vec_float4 leftRot3 = leftj[9];
		vec_float4 rightRot0 = rightj[0];
		vec_float4 rightRot1 = rightj[3];
		vec_float4 rightRot2 = rightj[6];
		vec_float4 rightRot3 = rightj[9];
		vec_float4 leftTrans0 = leftj[1];
		vec_float4 leftScale0 = leftj[2];
		vec_float4 rightTrans0 = rightj[1];
		vec_float4 rightScale0 = rightj[2];
		vec_float4 leftTrans1 = leftj[4];
		vec_float4 leftScale1 = leftj[5];
		vec_float4 rightTrans1 = rightj[4];
		vec_float4 rightScale1 = rightj[5];
		vec_float4 leftTrans2 = leftj[7];
		vec_float4 leftScale2 = leftj[8];
		vec_float4 rightTrans2 = rightj[7];
		vec_float4 rightScale2 = rightj[8];
		vec_float4 leftTrans3 = leftj[10];
		vec_float4 leftScale3 = leftj[11];
		vec_float4 rightTrans3 = rightj[10];
		vec_float4 rightScale3 = rightj[11];
	
		// Load weights - shuffle mask computed outside of the loop will either 
		// extract bytes or force to ones, if input is NULL. 
		vec_uint4 leftwq = (vec_uint4) si_from_uint(*leftw);
		vec_uint4 rightwq = (vec_uint4) si_from_uint(*rightw);
		vec_uint4 leftwu = (vec_uint4) spu_shuffle(leftwq, leftwq, leftLoadShuffle); 
		vec_uint4 rightwu = (vec_uint4) spu_shuffle(rightwq, rightwq, rightLoadShuffle);
		
		// Advance input pointers
		leftj += 12;
		rightj += 12;
		leftw++;
		rightw++;
   
		// Blend factor / partial animation logic
		vec_uint4 leftZeroMask = (vec_uint4) si_ceqi((vec_char16) leftwu, 0);
		vec_uint4 rightZeroMask = (vec_uint4) si_ceqi((vec_char16) rightwu, 0);
		vec_uint4 maskZeroW0 = spu_and(selNotCompose, leftZeroMask);
		vec_uint4 maskZeroW1 = spu_or(leftZeroMask, rightZeroMask);
		vec_uint4 maskZeroW2 = spu_and(maskZeroW1, selSubtract);
		vec_uint4 maskZeroW = spu_nor(maskZeroW0, maskZeroW2);
		vec_uint4 selForceRight = spu_and(leftZeroMask, selCompose);
		vec_float4 rightwf = spu_mul(spu_convtf(rightwu, 8), cufltscale);
		vec_float4 tmpAlpha0 = spu_mul(splatAlpha, rightwf);
		vec_float4 t = spu_shuffle(tmpAlpha0, fone, (vec_uchar16) shufCompose); 

		// Splat t (for non vectorised portions of this loop)
		vec_float4 t0 = spu_shuffle(t, t, shufAAAA);
		vec_float4 t1 = spu_shuffle(t, t, shufBBBB);
		vec_float4 t2 = spu_shuffle(t, t, shufCCCC);
		vec_float4 t3 = spu_shuffle(t, t, shufDDDD);

		// Create "force right" shuffle masks (shuffles, to reduce even pipe load)
		vec_uint4 selForceRight0 = spu_shuffle(selForceRight, selForceRight, shufAAAA);
		vec_uint4 selForceRight1 = spu_shuffle(selForceRight, selForceRight, shufBBBB);
		vec_uint4 selForceRight2 = spu_shuffle(selForceRight, selForceRight, shufCCCC);
		vec_uint4 selForceRight3 = spu_shuffle(selForceRight, selForceRight, shufDDDD);
		vec_uchar16 shufCopyRight0 = spu_sel(shufABCD, shufabcd, (vec_uchar16) selForceRight0);
		vec_uchar16 shufCopyRight1 = spu_sel(shufABCD, shufabcd, (vec_uchar16) selForceRight1);
		vec_uchar16 shufCopyRight2 = spu_sel(shufABCD, shufabcd, (vec_uchar16) selForceRight2); 
		vec_uchar16 shufCopyRight3 = spu_sel(shufABCD, shufabcd, (vec_uchar16) selForceRight3);
		vec_uchar16 shufCopyRight = spu_sel(shufABCD, shufabcd, (vec_uchar16) selForceRight);

		// Create "force left" shuffle masks (shuffles, to reduce even pipe load)
		vec_uint4 selForceLeft0 = spu_shuffle(rightZeroMask, rightZeroMask, shufAAAA);
		vec_uint4 selForceLeft1 = spu_shuffle(rightZeroMask, rightZeroMask, shufBBBB);
		vec_uint4 selForceLeft2 = spu_shuffle(rightZeroMask, rightZeroMask, shufCCCC);
		vec_uint4 selForceLeft3 = spu_shuffle(rightZeroMask, rightZeroMask, shufDDDD);
		vec_uchar16 shufCopyLeft0 = spu_sel(shufABCD, shufabcd, (vec_uchar16) selForceLeft0);
		vec_uchar16 shufCopyLeft1 = spu_sel(shufABCD, shufabcd, (vec_uchar16) selForceLeft1);
		vec_uchar16 shufCopyLeft2 = spu_sel(shufABCD, shufabcd, (vec_uchar16) selForceLeft2); 
		vec_uchar16 shufCopyLeft3 = spu_sel(shufABCD, shufabcd, (vec_uchar16) selForceLeft3);
		vec_uchar16 shufCopyLeft = spu_sel(shufABCD, shufabcd, (vec_uchar16) rightZeroMask);

		// Swizzle left rotations
		// (X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3 - > X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3)
		vec_float4 leftRotX0X1Y0Y1 = spu_shuffle(leftRot0, leftRot1, shufAaBb);
		vec_float4 leftRotY2Y3X2X3 = spu_shuffle(leftRot2, leftRot3, shufBbAa);
		vec_float4 leftRotX = spu_shuffle(leftRotX0X1Y0Y1, leftRotY2Y3X2X3, shufABcd);
		vec_float4 leftRotY = spu_shuffle(leftRotX0X1Y0Y1, leftRotY2Y3X2X3, shufCDab);
		vec_float4 leftRotZ0Z1W0W1 = spu_shuffle(leftRot0, leftRot1, shufCcDd);
		vec_float4 leftRotW2W3Z2Z3 = spu_shuffle(leftRot2, leftRot3, shufDdCc);
		vec_float4 leftRotZ = spu_shuffle(leftRotZ0Z1W0W1, leftRotW2W3Z2Z3, shufABcd);
		vec_float4 leftRotW = spu_shuffle(leftRotZ0Z1W0W1, leftRotW2W3Z2Z3, shufCDab);

		// Swizzle right rotations
		// (X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3 - > X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3)
		vec_float4 rightRotX0X1Y0Y1 = spu_shuffle(rightRot0, rightRot1, shufAaBb);
		vec_float4 rightRotY2Y3X2X3 = spu_shuffle(rightRot2, rightRot3, shufBbAa);
		vec_float4 rightRotXtmp = spu_shuffle(rightRotX0X1Y0Y1, rightRotY2Y3X2X3, shufABcd);
		vec_float4 rightRotYtmp = spu_shuffle(rightRotX0X1Y0Y1, rightRotY2Y3X2X3, shufCDab);
		vec_float4 rightRotZ0Z1W0W1 = spu_shuffle(rightRot0, rightRot1, shufCcDd);
		vec_float4 rightRotW2W3Z2Z3 = spu_shuffle(rightRot2, rightRot3, shufDdCc);
		vec_float4 rightRotZtmp = spu_shuffle(rightRotZ0Z1W0W1, rightRotW2W3Z2Z3, shufABcd);
		vec_float4 rightRotW = spu_shuffle(rightRotZ0Z1W0W1, rightRotW2W3Z2Z3, shufCDab);

		// Conjugate (subtract mode only: negate x y z) 
		vec_float4 rightRotX = (vec_float4) spu_xor((vec_uint4) rightRotXtmp, maskConjugate);
		vec_float4 rightRotY = (vec_float4) spu_xor((vec_uint4) rightRotYtmp, maskConjugate);
		vec_float4 rightRotZ = (vec_float4) spu_xor((vec_uint4) rightRotZtmp, maskConjugate);
		
		// Swap left & right rotations for subtract mode
		vec_float4	aRotX = spu_shuffle(leftRotX, rightRotX, shufSwapRots);
		vec_float4	aRotY = spu_shuffle(leftRotY, rightRotY, shufSwapRots);
		vec_float4	aRotZ = spu_shuffle(leftRotZ, rightRotZ, shufSwapRots);
		vec_float4	aRotW = spu_shuffle(leftRotW, rightRotW, shufSwapRots);
		vec_float4	bRotX = spu_shuffle(rightRotX, leftRotX, shufSwapRots);
		vec_float4	bRotY = spu_shuffle(rightRotY, leftRotY, shufSwapRots);
		vec_float4	bRotZ = spu_shuffle(rightRotZ, leftRotZ, shufSwapRots);
		vec_float4	bRotW = spu_shuffle(rightRotW, leftRotW, shufSwapRots); 

		// Compose target rotation 
		vec_float4 composedRotXa = spu_mul(aRotX, bRotW);
		vec_float4 composedRotYa = spu_mul(aRotY, bRotW);
		vec_float4 composedRotZa = spu_mul(aRotZ, bRotW);
		vec_float4 composedRotWa = spu_mul(aRotW, bRotW);
		
		vec_float4 composedRotXb = spu_madd(aRotW, bRotX, composedRotXa);
		vec_float4 composedRotYb = spu_nmsub(aRotZ, bRotX, composedRotYa);
		vec_float4 composedRotZb = spu_madd(aRotY, bRotX, composedRotZa);
		vec_float4 composedRotWb = spu_nmsub(aRotX, bRotX, composedRotWa);
		
		vec_float4 composedRotXc = spu_madd(aRotZ, bRotY, composedRotXb);
		vec_float4 composedRotYc = spu_madd(aRotW, bRotY, composedRotYb);
		vec_float4 composedRotZc = spu_nmsub(aRotX, bRotY, composedRotZb);
		vec_float4 composedRotWc = spu_nmsub(aRotY, bRotY, composedRotWb);

		vec_float4 composedRotX = spu_nmsub(aRotY, bRotZ, composedRotXc);
		vec_float4 composedRotY = spu_madd(aRotX, bRotZ, composedRotYc);
		vec_float4 composedRotZ = spu_madd(aRotW, bRotZ, composedRotZc);
		vec_float4 composedRotW = spu_nmsub(aRotZ, bRotZ, composedRotWc);

		// Slerp rotations : compute dot product
		vec_float4 dotRotX = spu_mul(leftRotX, composedRotX);
		vec_float4 dotRotXY = spu_madd(leftRotY, composedRotY, dotRotX);
		vec_float4 dotRotXYZ = spu_madd(leftRotZ, composedRotZ, dotRotXY);
		vec_float4 dotRot = spu_madd(leftRotW, composedRotW, dotRotXYZ);  

		// Slerp rotations : determine target rotation (right or -right)
		vec_float4 floatSign = spu_sel(fone, minusfone, spu_cmpgt((vec_float4) zero, dotRot));
		vec_float4 targetRotX = spu_mul(composedRotX, floatSign);
		vec_float4 targetRotY = spu_mul(composedRotY, floatSign);
		vec_float4 targetRotZ = spu_mul(composedRotZ, floatSign);
		vec_float4 targetRotW = spu_mul(composedRotW, floatSign);
		vec_float4 cosAngle = spu_mul(dotRot, floatSign);
 
		// Slerp rotations : evaluate angle = acos(cosAngle) (MacLaurin)
		vec_float4 oneMinusCosAngle = spu_sub(fone, cosAngle);
		vec_float4 rsqTmp = spu_rsqrte(oneMinusCosAngle);	   
		vec_float4 evalTmp = spu_nmsub(rsqTmp, cosAngle, rsqTmp);
		vec_float4 acosEval3 = spu_mul(cosAngle, acos3);
		vec_float4 acosEval2 = spu_nmsub(cosAngle, acosEval3, acos2);
		vec_float4 acosEval1 = spu_nmsub(cosAngle, acosEval2, acos1);
		vec_float4 acosEval0 = spu_nmsub(cosAngle, acosEval1, acos0);
		vec_float4 angle = spu_mul(acosEval0, evalTmp);
				
		// Slerp rotations : evaluate sina = sin(angle) (Taylor)
		vec_float4 sqAngle = spu_mul(angle, angle);
		vec_float4 sinaEval3 = spu_madd(sin3, sqAngle, sin2);
		vec_float4 sinaEval2 = spu_madd(sinaEval3, sqAngle, sin1);
		vec_float4 sinaEval1 = spu_madd(sinaEval2, sqAngle, sin0);
		vec_float4 sinaEval0 = spu_madd(sinaEval1, sqAngle, fone);
		vec_float4 sina = spu_mul(sinaEval0, angle);

		// Slerp rotations : evaluate sinb = sin(t*angle) (Taylor)
		vec_float4 tAngle = spu_mul(t, angle);
		vec_float4 sqTAngle = spu_mul(tAngle, tAngle);
		vec_float4 sinbEval3 = spu_madd(sin3, sqTAngle, sin2);
		vec_float4 sinbEval2 = spu_madd(sinbEval3, sqTAngle, sin1);
		vec_float4 sinbEval1 = spu_madd(sinbEval2, sqTAngle, sin0);
		vec_float4 sinbEval0 = spu_madd(sinbEval1, sqTAngle, fone);
		vec_float4 sinb = spu_mul(sinbEval0, tAngle);
		
		// Slerp rotations : evaluate sinc = sin((1-t)*angle) (Taylor)
		vec_float4 oneMinusT = spu_sub(fone, t);
		vec_float4 oneMinusTAngle = spu_mul(oneMinusT, angle);
		vec_float4 sqOneMinusTAngle = spu_mul(oneMinusTAngle, oneMinusTAngle);
		vec_float4 sincEval3 = spu_madd(sin3, sqOneMinusTAngle, sin2);
		vec_float4 sincEval2 = spu_madd(sincEval3, sqOneMinusTAngle, sin1);
		vec_float4 sincEval1 = spu_madd(sincEval2, sqOneMinusTAngle, sin0);
		vec_float4 sincEval0 = spu_madd(sincEval1, sqOneMinusTAngle, fone);
		vec_float4 sinc = spu_mul(sincEval0, oneMinusTAngle);   

		// Select either sinc/sina or sinb/sina depending on the quadrant
		vec_float4 invSina = spu_re(sina);
		vec_float4 fLeftTmp = spu_mul(sinc, invSina);
		vec_float4 fRightTmp = spu_mul(sinb, invSina);	
		vec_uchar16 shufRange = spu_sel(shufABCD, shufabcd, (vec_uchar16)spu_cmpgt(cosAngle, oneMinusEpsilon));
		vec_float4 fLeft = spu_shuffle(fLeftTmp, oneMinusT, shufRange);
		vec_float4 fRight = spu_shuffle(fRightTmp, t, shufRange);
	
		// The actual blend
		vec_float4 tmpRotX = spu_mul(leftRotX, fLeft);
		vec_float4 tmpRotY = spu_mul(leftRotY, fLeft);
		vec_float4 tmpRotZ = spu_mul(leftRotZ, fLeft);
		vec_float4 tmpRotW = spu_mul(leftRotW, fLeft);		
		vec_float4 outRotX = spu_madd(targetRotX, fRight, tmpRotX);
		vec_float4 outRotY = spu_madd(targetRotY, fRight, tmpRotY);
		vec_float4 outRotZ = spu_madd(targetRotZ, fRight, tmpRotZ);
		vec_float4 outRotW = spu_madd(targetRotW, fRight, tmpRotW);

		// Bypass - required because input values can be NaN -> can't just force t to 0 or 1
		// (using shuffles instead of selects to reduce even pipe load)
		outRotX = (vec_float4) spu_shuffle(outRotX, leftRotX, shufCopyLeft);
		outRotY = (vec_float4) spu_shuffle(outRotY, leftRotY, shufCopyLeft);
		outRotZ = (vec_float4) spu_shuffle(outRotZ, leftRotZ, shufCopyLeft);
		outRotW = (vec_float4) spu_shuffle(outRotW, leftRotW, shufCopyLeft);	
		outRotX = (vec_float4) spu_shuffle(outRotX, rightRotX, shufCopyRight);
		outRotY = (vec_float4) spu_shuffle(outRotY, rightRotY, shufCopyRight);
		outRotZ = (vec_float4) spu_shuffle(outRotZ, rightRotZ, shufCopyRight);
		outRotW = (vec_float4) spu_shuffle(outRotW, rightRotW, shufCopyRight);

		// Deswizzle output rotations 
		// (X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3 -> X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3)
		vec_float4 outRotX0Y0X1Y1 = spu_shuffle(outRotX, outRotY, shufAaBb);
		vec_float4 outRotZ1W1Z0W0 = spu_shuffle(outRotZ, outRotW, shufBbAa);
		vec_float4 outRot0 = spu_shuffle(outRotX0Y0X1Y1, outRotZ1W1Z0W0, shufABcd);
		vec_float4 outRot1 = spu_shuffle(outRotX0Y0X1Y1, outRotZ1W1Z0W0, shufCDab);
		vec_float4 outRotX2Y2X3Y3 = spu_shuffle(outRotX, outRotY, shufCcDd);
		vec_float4 outRotZ3W3Z2W2 = spu_shuffle(outRotZ, outRotW, shufDdCc);
		vec_float4 outRot2 = spu_shuffle(outRotX2Y2X3Y3, outRotZ3W3Z2W2, shufABcd);
		vec_float4 outRot3 = spu_shuffle(outRotX2Y2X3Y3, outRotZ3W3Z2W2, shufCDab);	

		// Lerp translation 0
		vec_float4 rightTrans0b = spu_mul(rightTrans0, mulDeltaTrans);
		vec_float4 tgtTrans0 = spu_add(rightTrans0b, leftTrans0);
		vec_float4 outTrans0Tmp0 = spu_madd(tgtTrans0, t0, leftTrans0);
		vec_float4 outTrans0Tmp1 = spu_nmsub(leftTrans0, t0, outTrans0Tmp0);
		vec_float4 outTrans0Tmp2 = (vec_float4) spu_shuffle(outTrans0Tmp1, leftTrans0, shufCopyLeft0);
		vec_float4 outTrans0 = (vec_float4) spu_shuffle(outTrans0Tmp2, rightTrans0b, shufCopyRight0);

		// Lerp translation 1
		vec_float4 rightTrans1b = spu_mul(rightTrans1, mulDeltaTrans);
		vec_float4 tgtTrans1 = spu_add(rightTrans1b, leftTrans1);
		vec_float4 outTrans1Tmp0 = spu_madd(tgtTrans1, t1, leftTrans1);
		vec_float4 outTrans1Tmp1 = spu_nmsub(leftTrans1, t1, outTrans1Tmp0);
		vec_float4 outTrans1Tmp2 = (vec_float4) spu_shuffle(outTrans1Tmp1, leftTrans1, shufCopyLeft1);
		vec_float4 outTrans1 = (vec_float4) spu_shuffle(outTrans1Tmp2, rightTrans1b, shufCopyRight1);

		// Lerp translation 2
		vec_float4 rightTrans2b = spu_mul(rightTrans2, mulDeltaTrans);
		vec_float4 tgtTrans2 = spu_add(rightTrans2b, leftTrans2);
		vec_float4 outTrans2Tmp0 = spu_madd(tgtTrans2, t2, leftTrans2);
		vec_float4 outTrans2Tmp1 = spu_nmsub(leftTrans2, t2, outTrans2Tmp0);
		vec_float4 outTrans2Tmp2 = (vec_float4) spu_shuffle(outTrans2Tmp1, leftTrans2, shufCopyLeft2);
		vec_float4 outTrans2 = (vec_float4) spu_shuffle(outTrans2Tmp2, rightTrans2b, shufCopyRight2);

		// Lerp translation 3
		vec_float4 rightTrans3b = spu_mul(rightTrans3, mulDeltaTrans);
		vec_float4 tgtTrans3 = spu_add(rightTrans3b, leftTrans3);
		vec_float4 outTrans3Tmp0 = spu_madd(tgtTrans3, t3, leftTrans3);
		vec_float4 outTrans3Tmp1 = spu_nmsub(leftTrans3, t3, outTrans3Tmp0);
		vec_float4 outTrans3Tmp2 = (vec_float4) spu_shuffle(outTrans3Tmp1, leftTrans3, shufCopyLeft3);
		vec_float4 outTrans3 = (vec_float4) spu_shuffle(outTrans3Tmp2, rightTrans3b, shufCopyRight3);

		// Lerp scale 0
		vec_float4 invDeltaScaleEst0 = spu_re(rightScale0);
		vec_float4 invDeltaScale0 = spu_madd(spu_nmsub(rightScale0, invDeltaScaleEst0, fone), invDeltaScaleEst0, invDeltaScaleEst0);		
		vec_float4 rightScale0b = spu_sel(rightScale0, invDeltaScale0, selSubtract);
		vec_float4 tgtScale0 = spu_mul(rightScale0b, leftScale0);
		vec_float4 outScale0Tmp0 = spu_madd(tgtScale0, t0, leftScale0);
		vec_float4 outScale0Tmp1 = spu_nmsub(leftScale0, t0, outScale0Tmp0);
		vec_float4 outScale0Tmp2 = (vec_float4) spu_shuffle(outScale0Tmp1, leftScale0, shufCopyLeft0);
		vec_float4 outScale0 = (vec_float4) spu_shuffle(outScale0Tmp2, rightScale0b, shufCopyRight0);

		// Lerp scale 1
		vec_float4 invDeltaScaleEst1 = spu_re(rightScale1);
		vec_float4 invDeltaScale1 = spu_madd(spu_nmsub(rightScale1, invDeltaScaleEst1, fone), invDeltaScaleEst1, invDeltaScaleEst1);		
		vec_float4 rightScale1b = spu_sel(rightScale1, invDeltaScale1, selSubtract);
		vec_float4 tgtScale1 = spu_mul(rightScale1b, leftScale1);
		vec_float4 outScale1Tmp0 = spu_madd(tgtScale1, t1, leftScale1);
		vec_float4 outScale1Tmp1 = spu_nmsub(leftScale1, t1, outScale1Tmp0);
		vec_float4 outScale1Tmp2 = (vec_float4) spu_shuffle(outScale1Tmp1, leftScale1, shufCopyLeft1);
		vec_float4 outScale1 = (vec_float4) spu_shuffle(outScale1Tmp2, rightScale1b, shufCopyRight1);

		// Lerp scale 2
		vec_float4 invDeltaScaleEst2 = spu_re(rightScale2);
		vec_float4 invDeltaScale2 = spu_madd(spu_nmsub(rightScale2, invDeltaScaleEst2, fone), invDeltaScaleEst2, invDeltaScaleEst2);		
		vec_float4 rightScale2b = spu_sel(rightScale2, invDeltaScale2, selSubtract);
		vec_float4 tgtScale2 = spu_mul(rightScale2b, leftScale2);
		vec_float4 outScale2Tmp0 = spu_madd(tgtScale2, t2, leftScale2);
		vec_float4 outScale2Tmp1 = spu_nmsub(leftScale2, t2, outScale2Tmp0);
		vec_float4 outScale2Tmp2 = (vec_float4) spu_shuffle(outScale2Tmp1, leftScale2, shufCopyLeft2);
		vec_float4 outScale2 = (vec_float4) spu_shuffle(outScale2Tmp2, rightScale2b, shufCopyRight2);

		// Lerp scale 3
		vec_float4 invDeltaScaleEst3 = spu_re(rightScale3);
		vec_float4 invDeltaScale3 = spu_madd(spu_nmsub(rightScale3, invDeltaScaleEst3, fone), invDeltaScaleEst3, invDeltaScaleEst3);		
		vec_float4 rightScale3b = spu_sel(rightScale3, invDeltaScale3, selSubtract);
		vec_float4 tgtScale3 = spu_mul(rightScale3b, leftScale3);
		vec_float4 outScale3Tmp0 = spu_madd(tgtScale3, t3, leftScale3);
		vec_float4 outScale3Tmp1 = spu_nmsub(leftScale3, t3, outScale3Tmp0);
		vec_float4 outScale3Tmp2 = (vec_float4) spu_shuffle(outScale3Tmp1, leftScale3, shufCopyLeft3);
		vec_float4 outScale3 = (vec_float4) spu_shuffle(outScale3Tmp2, rightScale3b, shufCopyRight3);

		// Write results (x4) 
		outputj[0]  = outRot0;
		outputj[1]  = outTrans0;
		outputj[2]  = outScale0;
		outputj[3]  = outRot1;
		outputj[4]  = outTrans1;
		outputj[5]  = outScale1;
		outputj[6]  = outRot2;
		outputj[7]  = outTrans2;
		outputj[8]  = outScale2;
		outputj[9]  = outRot3;
		outputj[10] = outTrans3;
		outputj[11] = outScale3;

		// Output weight / partial animation logic
		vec_uint4 outWU0 = spu_add(leftwu, rightwu);
		vec_uint4 selClampOutWU = spu_cmpgt(outWU0, kClampWU);
		vec_uint4 outWU1 = spu_sel(outWU0, kClampWU, selClampOutWU);
		vec_uint4 outWU2 = spu_and(outWU1, maskZeroW);  
		outputw[0]  = si_to_uint((vec_char16) spu_shuffle((vec_uchar16)outWU2, (vec_uchar16)outWU2, (vec_uchar16)shufDHLPDHLPDHLPDHLP));

		// Advance output pointers
		outputj += 12;
		outputw++;
	}
}
