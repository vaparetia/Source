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
 * @brief   Relative blend between user channels (for additive/subtractive animations)
 *          SPU intrinsics implementation.
 * 
 * @note    Please refer to LibEdge-Overview_e.pdf for a description of partial 
 *          animation logic and blending modes.
 *
 * @note    A reference C++ implementation of this function is available in
 *          target/reference/src/edge/anim/edgeanim_blend_user_relative.cpp
 *
 * @note    This function is not called unless you change the makefile which 
 *          uses asm implementation by default.
 *
 * @param   outputChannels  Pointer to output user channel array. 
 *                          Must be aligned to 16 bytes.
 *                          Due to the SIMD nature of the processing, the 
 *                          outputChannels array must be a multiple of 4 entries 
 *                          in size. 
 * @param   outputWeights   Pointer to output user channel weights array.
 *                          Must be aligned to 4 bytes.
 * @param   leftChannels    Pointer to left user channel array.
 *                          Must be aligned to 16 bytes.
 * @param   leftWeights     Pointer to left user channel weights array.
 *                          Must be aligned to 4 bytes.
 * @param   rightChannels   Pointer to right user channel array.
 *                          Must be aligned to 16 bytes.
 * @param   rightWeights    Pointer to right user channel weights array.
 *                          Must be aligned to 4 bytes.
 * @param   alpha           Blend factor (0-1)
 * @param   blendMode       Relative blend/compose mode
 * @param   count           Joint count.
 */

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
	EDGE_BOOKMARK(EDGE_BOOKMARK_ANIM_BLEND_USER_ADDITIVE);

	EDGE_ASSERT(EDGE_IS_ALIGNED(outputChannels, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(outputWeights, 4U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(leftChannels, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(leftWeights, 4U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(rightChannels, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(rightWeights, 4U));
	EDGE_ASSERT(count);

	const vec_float4 splatAlpha = spu_splats(alpha);
	
	// Constants : Misc 
	const vec_uint4 zero = {0U, 0U, 0U, 0U};	
	const vec_float4 fone = {1.0f, 1.0f, 1.0f, 1.0f};
	const vec_float4 kClampMax = spu_splats(1.0f);
	const vec_float4 kClampMin = spu_splats(0.0f);
	const vec_uint4 kClampWU = spu_splats(0xFFU);

	// Constants : Scale for weights (255 = 1.0f)
	const vec_float4 cufltscale = {1.0039215686f, 1.0039215686f, 1.0039215686f, 1.0039215686f};

	// Constants : Shuffle masks
	const vec_uint4 shufABCD = {0x00010203,0x04050607,0x08090A0B,0x0C0D0E0F};
	const vec_uint4 shufabcd = {0x10111213,0x14151617,0x18191A1B,0x1C1D1E1F};
	const vec_uchar16 shuf000A000B000C000D = {0x80,0x80,0x80,0x00,0x80,0x80,0x80,0x01,0x80,0x80,0x80,0x02,0x80,0x80,0x80,0x03};
	const vec_uchar16 shuf000X000X000X000X = {0x80,0x80,0x80,0xC0,0x80,0x80,0x80,0xC0,0x80,0x80,0x80,0xC0,0x80,0x80,0x80,0xC0};
	const vec_uchar16 shuf0000000000000000 = {0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80};
	const vec_uchar16 shufDHLPDHLPDHLPDHLP = {0x03,0x07,0x0B,0x0F,0x03,0x07,0x0B,0x0F,0x03,0x07,0x0B,0x0F,0x03,0x07,0x0B,0x0F};
	const vec_uchar16 shufAAAABBBBCCCCDDDD = {0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x03,0x03,0x03,0x03};
 
	// Pointers
	const vec_float4* __restrict__ leftc = (const vec_float4* __restrict__) leftChannels;
	const vec_float4* __restrict__ rightc = (const vec_float4* __restrict__) rightChannels;
	const uint32_t* __restrict__ leftw = (const uint32_t* __restrict__) leftWeights;
	const uint32_t* __restrict__ rightw = (const uint32_t* __restrict__) rightWeights;
	const uint32_t* __restrict__ cflags = (const uint32_t* __restrict__) channelFlags;
	vec_float4* __restrict__ output = (vec_float4* __restrict__) outputChannels;
	uint32_t* __restrict__ outputw = (uint32_t* __restrict__) outputWeights;

	// Compute shuffle masks to load weights. If NULL they will force to ones, otherwise zero extend bytes to ints
	vec_uint4 selLeftNonNull = (vec_uint4)si_clgti((vec_char16) spu_splats((uintptr_t) leftWeights), 0);
	vec_uint4 selRightNonNull = (vec_uint4)si_clgti((vec_char16) spu_splats((uintptr_t) rightWeights), 0);
	vec_uint4 selChannelFlagsNonNull = (vec_uint4)si_clgti((vec_char16) spu_splats((uintptr_t) channelFlags), 0);
	vec_uchar16 leftLoadShuffle = spu_sel(shuf000X000X000X000X, shuf000A000B000C000D, (vec_uchar16)selLeftNonNull);
	vec_uchar16 rightLoadShuffle = spu_sel(shuf000X000X000X000X, shuf000A000B000C000D, (vec_uchar16)selRightNonNull);
	vec_uchar16 shufChannelFlags = spu_sel(shuf0000000000000000, shufAAAABBBBCCCCDDDD, (vec_uchar16)selChannelFlagsNonNull);

	// Constants computed outside of the loop for various flavours of additive/substractive blending mode
	vec_uint4 splatBlendMode = spu_splats((unsigned int)blendMode);
	vec_uint4 selSubtract = spu_cmpgt(spu_and(splatBlendMode, 0x01U), spu_splats(0U));
	vec_uint4 selCompose = spu_cmpgt(spu_and(splatBlendMode, 0x10U), spu_splats(0U));
	vec_uint4 selNotCompose = spu_nand(selCompose, selCompose);
	const vec_uint4 maskSignNegate = {0x80000000, 0x80000000, 0x80000000, 0x80000000};
	vec_uint4 shufCompose = spu_sel(shufABCD, shufabcd, selCompose);
	vec_uint4 maskNegate = spu_sel(zero, maskSignNegate, selSubtract);

	// Vectorised loop count (4 iterations at a time, rounded up)
	int count4 = (count + 3) >> 2;

	for (; count4; count4--) {
		// Load user data (x4)
		vec_float4 left = leftc[0];
		vec_float4 right = rightc[0];

		// Load weights - shuffle mask computed outside of the loop will either 
		// extract bytes or force to ones, if input is NULL. 
		vec_uint4 leftwq = (vec_uint4) si_from_uint(*leftw);
		vec_uint4 rightwq = (vec_uint4) si_from_uint(*rightw);
		vec_uchar16 flags = (vec_uchar16) si_from_uint(*cflags);
		vec_uint4 leftwu = (vec_uint4) spu_shuffle(leftwq, leftwq, leftLoadShuffle); 
		vec_uint4 rightwu = (vec_uint4) spu_shuffle(rightwq, rightwq, rightLoadShuffle);

		// Advance input pointers
		leftc += 1;
		rightc += 1;
		leftw++;
		rightw++;
		cflags++;

		// Channel flags
		vec_uchar16 replFlags  = spu_shuffle(flags, flags, shufChannelFlags);
		vec_uchar16 clampFlags = spu_cmpeq(spu_and(replFlags, 
			spu_splats((uint8_t)EDGE_ANIM_USER_CHANNEL_FLAG_CLAMP01)), 
			spu_splats((uint8_t)EDGE_ANIM_USER_CHANNEL_FLAG_CLAMP01));
		vec_uchar16 minmaxFlags = spu_cmpeq(spu_and(replFlags, 
			spu_splats((uint8_t)EDGE_ANIM_USER_CHANNEL_FLAG_MINMAX)), 
			spu_splats((uint8_t)EDGE_ANIM_USER_CHANNEL_FLAG_MINMAX));

		// Blend factor / partial animation logic
		vec_uint4 leftZeroMask = (vec_uint4) si_ceqi((vec_char16) leftwu, 0);
		vec_uint4 rightZeroMask = (vec_uint4) si_ceqi((vec_char16) rightwu, 0);
		vec_uint4 neitherZeroMask = spu_nor(leftZeroMask, rightZeroMask);
		vec_uint4 maskZeroW0 = spu_and(selNotCompose, leftZeroMask);
		vec_uint4 maskZeroW1 = spu_or(leftZeroMask, rightZeroMask);
		vec_uint4 maskZeroW2 = spu_and(maskZeroW1, selSubtract);
		vec_uint4 maskZeroW = spu_nor(maskZeroW0, maskZeroW2);
		vec_uint4 selForceRight = spu_and(leftZeroMask, selCompose);
		vec_uint4 shufCopyRight = spu_sel((vec_uint4)shufABCD, (vec_uint4)shufabcd, selForceRight);
		vec_uint4 shufCopyLeft = spu_sel((vec_uint4)shufABCD, (vec_uint4)shufabcd, rightZeroMask);
		vec_float4 rightwf = spu_mul(spu_convtf(rightwu, 8), cufltscale);
		vec_float4 tmpAlpha0 = spu_mul(splatAlpha, rightwf);
		vec_float4 t = spu_shuffle(tmpAlpha0, fone, (vec_uchar16) shufCompose); 

		// Minmax mode
		vec_uint4 selLeftGreater = spu_cmpgt(left, right);
		vec_uint4 selMinMax = spu_xor(selLeftGreater, selSubtract);
		vec_float4 outMinMax = spu_sel(right, left, selMinMax);
		vec_uint4 selMinMaxOutput = spu_and(neitherZeroMask, (vec_uint4)minmaxFlags);

		// Lerp & clamp user channels 
		vec_uint4 maskNegateTmp = spu_sel(maskNegate, zero, minmaxFlags);
		vec_float4 tmpRight = (vec_float4) spu_xor((vec_uint4)right, (vec_uint4)maskNegateTmp);
		vec_float4 out0 = spu_madd(tmpRight, t, left);
		vec_float4 out1 = (vec_float4)spu_shuffle((vec_uchar16)out0, (vec_uchar16)tmpRight, (vec_uchar16)shufCopyRight);
		vec_float4 out2 = spu_sel(out1, outMinMax, selMinMaxOutput);
		vec_float4 out3 = spu_shuffle(out2, left, (vec_uchar16) shufCopyLeft);
		vec_float4 out4 = spu_shuffle(out3, tmpRight, (vec_uchar16) shufCopyRight);
		vec_uint4 selClampUp = spu_cmpgt(out4, kClampMax);
		vec_uint4 selClampDown = spu_cmpgt(kClampMin, out4);
		vec_float4 out5 = spu_sel(out4, kClampMax, selClampUp);
		vec_float4 out6 = spu_sel(out5, kClampMin, selClampDown);
		vec_float4 out7 = spu_sel(out4, out6, clampFlags);	  
		
		// Output weight / partial animation logic
		vec_uint4 outWU0 = spu_add(leftwu, rightwu);
		vec_uint4 selClampOutWU = spu_cmpgt(outWU0, kClampWU);
		vec_uint4 outWU1 = spu_sel(outWU0, kClampWU, selClampOutWU);
		vec_uint4 outWU2 = spu_and(outWU1, maskZeroW);  
		uint32_t outW = si_to_uint((vec_char16) spu_shuffle((vec_uchar16)outWU2, (vec_uchar16)outWU2, (vec_uchar16)shufDHLPDHLPDHLPDHLP));

		// Write results (x4) 
		output[0] = out7;
		outputw[0] = outW;

		// Advance output pointers
		output += 1;
		outputw++;
	}
}
