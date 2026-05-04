/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <spu_intrinsics.h>

#include "edge/anim/edgeanim_spu.h"

/**
 * Intrinsics implementation
 *
 * Note: This function is not called unless you change the makefile which 
 *       uses asm implementation by default.
 */

#warning sub-optimal version - intrinisics implementation instead of asm

void _edgeAnimEvaluateRConst(vec_float4* output, const vec_uint4* constRData, uint32_t count, 
							 const uint16_t* constRTable, const vec_uchar16* shufTable)
{
	const vec_uchar16 shuf0A0D0G0b = {0x80,0x80,0x00,0x01,0x80,0x80,0x06,0x07,0x80,0x80,0x0c,0x0d,0x80,0x80,0x12,0x13};
	const vec_uchar16 shuf0E0H0c0f = {0x80,0x80,0x08,0x09,0x80,0x80,0x0e,0x0f,0x80,0x80,0x14,0x15,0x80,0x80,0x1a,0x1b};
	const vec_uchar16 shufBCEFHacd = {0x02,0x03,0x04,0x05,0x08,0x09,0x0a,0x0b,0x0e,0x0f,0x10,0x11,0x14,0x15,0x16,0x17};
	const vec_uchar16 shufFGabdegh = {0x0a,0x0b,0x0c,0x0d,0x10,0x11,0x12,0x13,0x16,0x17,0x18,0x19,0x1c,0x1d,0x1e,0x1f};
	const vec_uchar16 shufAaBb = {0x00,0x01,0x02,0x03,0x10,0x11,0x12,0x13,0x04,0x05,0x06,0x07,0x14,0x15,0x16,0x17};
	const vec_uchar16 shufBbAa = {0x04,0x05,0x06,0x07,0x14,0x15,0x16,0x17,0x00,0x01,0x02,0x03,0x10,0x11,0x12,0x13};
	const vec_uchar16 shufCcDd = {0x08,0x09,0x0A,0x0B,0x18,0x19,0x1A,0x1B,0x0C,0x0D,0x0E,0x0F,0x1C,0x1D,0x1E,0x1F};
	const vec_uchar16 shufDdCc = {0x0C,0x0D,0x0E,0x0F,0x1C,0x1D,0x1E,0x1F,0x08,0x09,0x0A,0x0B,0x18,0x19,0x1A,0x1B};
	const vec_uchar16 shufCDab = {0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17};
	const vec_uchar16 selABcd   = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

	const vec_float4 fOne = {1.0f, 1.0f, 1.0f, 1.0f};
	const vec_float4 fHalf = {0.5f, 0.5f, 0.5f, 0.5f};
	static const float sqrt2 = 1.4142135624f;
	const vec_float4 quatScale = {sqrt2, sqrt2, sqrt2, sqrt2};
	const vec_float4 quatBias = {-0.5*sqrt2, -0.5*sqrt2, -0.5*sqrt2, -0.5*sqrt2};

	uint32_t count8 = (count + 7) >> 3;

	for(; count8; count8--) {
		// Decompress the 8 rotation quaternions to float 4
		// Compression is 'smallest 3' (15.15.15.2)
		vec_uint4 qw0 = constRData[0];
		vec_uint4 qw1 = constRData[1];
		vec_uint4 qw2 = constRData[2];

		// Unpack 15-bit components
		vec_uint4 a0i = spu_shuffle(qw0, qw1, shuf0A0D0G0b);
		vec_uint4 a4i = spu_shuffle(qw1, qw2, shuf0E0H0c0f);
		vec_uint4 bc0i = spu_shuffle(qw0, qw1, shufBCEFHacd);
		vec_uint4 bc4i = spu_shuffle(qw1, qw2, shufFGabdegh);
		vec_uint4 b0i = spu_and(spu_rlmask(bc0i, -17), 0x7fff);
		vec_uint4 b4i = spu_and(spu_rlmask(bc4i, -17), 0x7fff);
		vec_uint4 c0i = spu_and(spu_rlmask(bc0i, -2), 0x7fff);
		vec_uint4 c4i = spu_and(spu_rlmask(bc4i, -2), 0x7fff);

		vec_float4 a0 = spu_madd(spu_convtf(a0i, 15), quatScale, quatBias);
		vec_float4 a4 = spu_madd(spu_convtf(a4i, 15), quatScale, quatBias);
		vec_float4 b0 = spu_madd(spu_convtf(b0i, 15), quatScale, quatBias);
		vec_float4 b4 = spu_madd(spu_convtf(b4i, 15), quatScale, quatBias);
		vec_float4 c0 = spu_madd(spu_convtf(c0i, 15), quatScale, quatBias);
		vec_float4 c4 = spu_madd(spu_convtf(c4i, 15), quatScale, quatBias);

		// Derived value, d = sqrt(1 - a^2 - b^2 - c^2)
		vec_float4 d0sqr0 = spu_nmsub(a0, a0, fOne);
		vec_float4 d0sqr1 = spu_nmsub(b0, b0, d0sqr0);
		vec_float4 d0sqr2 = spu_nmsub(c0, c0, d0sqr1);
		vec_float4 d0tmp0 = (vec_float4)si_frsqest((qword)d0sqr2);
		vec_float4 d0tmp1 = (vec_float4)si_fi((qword)d0sqr2, (qword)d0tmp0);
		vec_float4 d0tmp2 = spu_mul(d0sqr2, d0tmp1);
		vec_float4 d0tmp3 = spu_mul(d0tmp1, fHalf);
		vec_float4 d0tmp4 = spu_nmsub(d0tmp2, d0tmp1, fOne);
		vec_float4 d0inv = spu_madd(d0tmp3, d0tmp4, d0tmp1);
		vec_float4 d0 = spu_mul(d0inv, d0sqr2);

		vec_float4 d4sqr0 = spu_nmsub(a4, a4, fOne);
		vec_float4 d4sqr1 = spu_nmsub(b4, b4, d4sqr0);
		vec_float4 d4sqr2 = spu_nmsub(c4, c4, d4sqr1);
		vec_float4 d4tmp0 = (vec_float4)si_frsqest((qword)d4sqr2);
		vec_float4 d4tmp1 = (vec_float4)si_fi((qword)d4sqr2, (qword)d4tmp0);
		vec_float4 d4tmp2 = spu_mul(d4sqr2, d4tmp1);
		vec_float4 d4tmp3 = spu_mul(d4tmp1, fHalf);
		vec_float4 d4tmp4 = spu_nmsub(d4tmp2, d4tmp1, fOne);
		vec_float4 d4inv = spu_madd(d4tmp3, d4tmp4, d4tmp1);
		vec_float4 d4 = spu_mul(d4inv, d4sqr2);

		// Deswizzle
		vec_float4 s0 = spu_shuffle(a0, b0, shufAaBb);
		vec_float4 s1 = spu_shuffle(a0, b0, shufCcDd);
		vec_float4 s2 = spu_shuffle(c0, d0, shufBbAa);
		vec_float4 s3 = spu_shuffle(c0, d0, shufDdCc);
		vec_float4 q0s = spu_sel(s0, s2, selABcd);
		vec_float4 q1s = spu_shuffle(s0, s2, shufCDab);
		vec_float4 q2s = spu_sel(s1, s3, selABcd);
		vec_float4 q3s = spu_shuffle(s1, s3, shufCDab);

		vec_float4 s4 = spu_shuffle(a4, b4, shufAaBb);
		vec_float4 s5 = spu_shuffle(a4, b4, shufCcDd);
		vec_float4 s6 = spu_shuffle(c4, d4, shufBbAa);
		vec_float4 s7 = spu_shuffle(c4, d4, shufDdCc);
		vec_float4 q4s = spu_sel(s4, s6, selABcd);
		vec_float4 q5s = spu_shuffle(s4, s6, shufCDab);
		vec_float4 q6s = spu_sel(s5, s7, selABcd);
		vec_float4 q7s = spu_shuffle(s5, s7, shufCDab);

		// Get 2-bit derived value index
		vec_uint4 idx0 = spu_and(bc0i, 0x3);
		vec_uint4 idx4 = spu_and(bc4i, 0x3);

		// Now shuffle components to insert the derived value.
		uint32_t sofs0 = si_to_uint((vec_char16)idx0);
		uint32_t sofs1 = si_to_uint((vec_char16)spu_slqwbyte(idx0, 4));
		uint32_t sofs2 = si_to_uint((vec_char16)spu_slqwbyte(idx0, 8));
		uint32_t sofs3 = si_to_uint((vec_char16)spu_slqwbyte(idx0, 12));
		uint32_t sofs4 = si_to_uint((vec_char16)idx4);
		uint32_t sofs5 = si_to_uint((vec_char16)spu_slqwbyte(idx4, 4));
		uint32_t sofs6 = si_to_uint((vec_char16)spu_slqwbyte(idx4, 8));
		uint32_t sofs7 = si_to_uint((vec_char16)spu_slqwbyte(idx4, 12));

		const vec_uchar16 shuf0 = shufTable[sofs0];
		const vec_uchar16 shuf1 = shufTable[sofs1];
		const vec_uchar16 shuf2 = shufTable[sofs2];
		const vec_uchar16 shuf3 = shufTable[sofs3];
		const vec_uchar16 shuf4 = shufTable[sofs4];
		const vec_uchar16 shuf5 = shufTable[sofs5];
		const vec_uchar16 shuf6 = shufTable[sofs6];
		const vec_uchar16 shuf7 = shufTable[sofs7];

		vec_float4 q0 = spu_shuffle(q0s, q0s, shuf0);
		vec_float4 q1 = spu_shuffle(q1s, q1s, shuf1);
		vec_float4 q2 = spu_shuffle(q2s, q2s, shuf2);
		vec_float4 q3 = spu_shuffle(q3s, q3s, shuf3);
		vec_float4 q4 = spu_shuffle(q4s, q4s, shuf4);
		vec_float4 q5 = spu_shuffle(q5s, q5s, shuf5);
		vec_float4 q6 = spu_shuffle(q6s, q6s, shuf6);
		vec_float4 q7 = spu_shuffle(q7s, q7s, shuf7);

		// Write results (x4) 
		vec_float4* outAdr0 = output + constRTable[0] * 3;
		vec_float4* outAdr1 = output + constRTable[1] * 3;
		vec_float4* outAdr2 = output + constRTable[2] * 3;
		vec_float4* outAdr3 = output + constRTable[3] * 3;
		vec_float4* outAdr4 = output + constRTable[4] * 3;
		vec_float4* outAdr5 = output + constRTable[5] * 3;
		vec_float4* outAdr6 = output + constRTable[6] * 3;
		vec_float4* outAdr7 = output + constRTable[7] * 3;

		*outAdr0 = q0;
		*outAdr1 = q1;
		*outAdr2 = q2;
		*outAdr3 = q3;
		*outAdr4 = q4;
		*outAdr5 = q5;
		*outAdr6 = q6;
		*outAdr7 = q7;

		// Advance pointers
		constRTable += 8;
		constRData += 3;
	}
}
