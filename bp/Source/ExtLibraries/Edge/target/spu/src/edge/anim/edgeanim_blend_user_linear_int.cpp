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
 * @brief   Linear blend between user channels
 *          SPU intrinsics implementation.
 *
 * @note    Please refer to LibEdge-Overview_e.pdf for a description of partial
 *          animation logic and blending modes.
 *
 * @note    A reference C++ implementation of this function is available in
 *          target/reference/src/edge/anim/edgeanim_blend_user_linear.cpp
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
 * @param   count           Joint count.
 */

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
	EDGE_BOOKMARK(EDGE_BOOKMARK_ANIM_BLEND_USER_LINEAR);

	EDGE_ASSERT(EDGE_IS_ALIGNED(outputChannels, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(outputWeights, 4U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(leftChannels, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(leftWeights, 4U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(rightChannels, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(rightWeights, 4U));
	EDGE_ASSERT(count);

	const vec_float4 splatAlpha = spu_splats(alpha);

	// Constants : Misc
	const vec_float4 zero = {0.0f, 0.0f, 0.0f, 0.0f};
	const vec_float4 one = {1.0f, 1.0f, 1.0f, 1.0f};
	const vec_uint4 k255U = {255, 255, 255, 255};

	// Constants : Scale for weights (255 = 1.0f)
	const vec_float4 cufltscale = { 1.0039215686f, 1.0039215686f, 1.0039215686f, 1.0039215686f };
	const vec_float4 cfltsscale = {0.99609375f, 0.99609375f, 0.99609375f, 0.99609375f};
	const vec_float4 cfltsbias = {0.001953125f, 0.001953125f, 0.001953125f, 0.001953125f};  

	// Constants : Shuffle masks
	const vec_uchar16 shuf000A000B000C000D = {0x80,0x80,0x80,0x00,0x80,0x80,0x80,0x01,0x80,0x80,0x80,0x02,0x80,0x80,0x80,0x03};
	const vec_uchar16 shuf000X000X000X000X = {0x80,0x80,0x80,0xC0,0x80,0x80,0x80,0xC0,0x80,0x80,0x80,0xC0,0x80,0x80,0x80,0xC0};
	const vec_uchar16 shuf0000000000000000 = {0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80};
	const vec_uchar16 shufDHLPDHLPDHLPDHLP = {0x03,0x07,0x0B,0x0F,0x03,0x07,0x0B,0x0F,0x03,0x07,0x0B,0x0F,0x03,0x07,0x0B,0x0F};
	const vec_uchar16 shufAAAABBBBCCCCDDDD = {0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x03,0x03,0x03,0x03};
	const vec_uchar16 shufABCD = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
	const vec_uchar16 shufabcd = {0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F};

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

	const vec_float4 kClampMax = spu_splats(1.0f);
	const vec_float4 kClampMin = spu_splats(0.0f);

	// Vectorised loop count (4 iterations at a time, rounded up)
	int count4 = (count + 3) >> 2;

	for (; count4; count4--) {
		// Load user data (x4)
		vec_float4 left = leftc[0];
		vec_float4 right = rightc[0];
	
		// Load weights - shuffle mask computed outside of the loop will either 
		// extract bytes or force to ones, if input is NULL. 
		vec_uint4 leftwq = (vec_uint4)si_from_uint(*leftw);
		vec_uint4 rightwq = (vec_uint4)si_from_uint(*rightw);
		vec_uchar16 flags = (vec_uchar16) si_from_uint(*cflags);
		
		// Advance input pointers
		leftc++;
		rightc++;
		leftw++;
		rightw++;
		cflags++;

		// Channel flags
		vec_uchar16 replFlags  = spu_shuffle(flags, flags, shufChannelFlags);
		vec_uchar16 clampFlags = spu_cmpeq(spu_and(replFlags, 
			spu_splats((uint8_t)EDGE_ANIM_USER_CHANNEL_FLAG_CLAMP01)), 
			spu_splats((uint8_t)EDGE_ANIM_USER_CHANNEL_FLAG_CLAMP01));

		// Convert to floats + scale
		vec_uint4 leftWU = (vec_uint4) spu_shuffle(leftwq, leftwq, leftLoadShuffle); 
		vec_uint4 rightWU = (vec_uint4) spu_shuffle(rightwq, rightwq, rightLoadShuffle);
		vec_float4 leftWF = spu_mul(spu_convtf(leftWU, 8), cufltscale);
		vec_float4 rightWF = spu_mul(spu_convtf(rightWU, 8), cufltscale);

		// Partial animation logic
		vec_uchar16 leftNonZeroMask = (vec_uchar16) si_clgti((vec_char16)leftWU, 0); 
		vec_uchar16 rightNonZeroMask = (vec_uchar16) si_clgti((vec_char16)rightWU, 0);
		vec_uchar16 shufCopyLeft = spu_sel(shufabcd, shufABCD, rightNonZeroMask);
		vec_uchar16 shufCopyRight = spu_sel(shufabcd, shufABCD, leftNonZeroMask);

		// Blend factor
		vec_uint4 selRightGreater = spu_cmpgt(rightWF, leftWF);
		vec_float4 divW = spu_sel(leftWF, rightWF, selRightGreater);
		vec_float4 invDivWEst = spu_re(divW);
		vec_float4 invDivW = spu_madd(spu_nmsub(divW, invDivWEst, one), invDivWEst, invDivWEst);		
		vec_float4 tmpBlendF0 = spu_madd(splatAlpha, leftWF, rightWF);
		vec_float4 tmpBlendF1 = spu_sub(tmpBlendF0, leftWF);
		vec_float4 tmpBlendF2 = spu_mul(splatAlpha, rightWF);
		vec_float4 tmpBlendF3 = spu_sel(tmpBlendF2, tmpBlendF1, selRightGreater);
		vec_float4 t = spu_mul(tmpBlendF3, invDivW);

		// Output weight computation	
		vec_float4 outWF0 = spu_madd(rightWF, t, leftWF);
		vec_float4 outWF1 = spu_nmsub(leftWF, t, outWF0);
		vec_float4 outWF = spu_madd(outWF1, cfltsscale, cfltsbias);
		vec_int4 outWU0 = spu_convts(outWF, 8);
		vec_uint4 selOutWUClampUp = spu_cmpgt(outWU0, (vec_int4)k255U);
		vec_uint4 selOutWUClampDown = spu_cmpgt((vec_int4)zero, outWU0);
		vec_int4 outWU1 = spu_sel(outWU0, (vec_int4)k255U, selOutWUClampUp);
		vec_int4 outWU2 = spu_sel(outWU1, (vec_int4)zero, selOutWUClampDown);
		vec_char16 outW = spu_shuffle((vec_char16)outWU2, (vec_char16)outWU2, shufDHLPDHLPDHLPDHLP);

		// Lerp & clamp user channels 
		vec_float4 out0 = spu_madd(right, t, left);
		vec_float4 out1 = spu_nmsub(left, t, out0);
		vec_float4 out2 = spu_shuffle(out1, left, shufCopyLeft);
		vec_float4 out3 = spu_shuffle(out2, right, shufCopyRight);
		vec_uint4 selClampUp = spu_cmpgt(out3, kClampMax);
		vec_uint4 selClampDown = spu_cmpgt(kClampMin, out3);
		vec_float4 out4 = spu_sel(out3, kClampMax, selClampUp);
		vec_float4 out5 = spu_sel(out4, kClampMin, selClampDown);
		vec_float4 out6 = spu_sel(out3, out5, clampFlags);	  

		// Write results (x4) 
		output[0] = out6;
		outputw[0] = si_to_uint(outW);  

		// Advance output pointers
		output += 1;
		outputw++;
	}
}
