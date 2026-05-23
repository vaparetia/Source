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
 * @brief   Convert 3x4 (transposed) matrices to joint transforms (R/T/S)
 *          SPU intrinisics implementation.
 *
 * @code
 * R = MatrixToQuat(m)      // TODO: elaborate
 * Tx = m03
 * Ty = m13
 * Tz = m23
 * Sx = sqrt(m00*m00 + m10*m10 + m20*m20)
 * Sy = sqrt(m01*m01 + m11*m11 + m21*m21)
 * Sz = sqrt(m02*m02 + m12*m12 + m22*m22)
 * @endcode
 *
 * @note    A reference C++ implementation of this function is available in
 *          target/reference/src/edge/anim/edgeanim_matrices_3x4_to_joints.cpp
 *
 * @note    This function is not called unless you change the makefile which 
 *          uses asm implementation by default.
 *
 * @param   outputJoints    Pointer to output joint transform array.
 *                          Must be aligned to 16 bytes.
 *                          Due to the SIMD nature of the processing, 
 *                          the outputJoints array must be a multiple of 4 entries in size. 
 * @param   inputMatrices   Pointer to input matrix array.
 *                          Must be aligned to 16 bytes.
 * @param   count           Joint/matrices count.
 */

extern "C"
void edgeAnimMatrices3x4ToJoints(EdgeAnimJointTransform* outputJoints, 
								 const void* inputMatrices, 
								 unsigned int count)
{
	EDGE_BOOKMARK(EDGE_BOOKMARK_ANIM_MATRICES_3X4_TO_JOINTS);

	EDGE_ASSERT(EDGE_IS_ALIGNED(outputJoints, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(inputMatrices, 16U));   
	EDGE_ASSERT(count);

	// Constants: Misc
	const vec_float4 one = {1.0f, 1.0f, 1.0f, 1.0f};
	const vec_uchar16 selABcd  = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	const vec_uint4  signMask = {0x80000000, 0x80000000, 0x80000000, 0x80000000};

	// Constants: Shuffle masks
	const vec_uchar16 shufAaBb = {0x00,0x01,0x02,0x03,0x10,0x11,0x12,0x13,0x04,0x05,0x06,0x07,0x14,0x15,0x16,0x17};
	const vec_uchar16 shufBbAa = {0x04,0x05,0x06,0x07,0x14,0x15,0x16,0x17,0x00,0x01,0x02,0x03,0x10,0x11,0x12,0x13};
	const vec_uchar16 shufCcDd = {0x08,0x09,0x0A,0x0B,0x18,0x19,0x1A,0x1B,0x0C,0x0D,0x0E,0x0F,0x1C,0x1D,0x1E,0x1F};
	const vec_uchar16 shufDdCc = {0x0C,0x0D,0x0E,0x0F,0x1C,0x1D,0x1E,0x1F,0x08,0x09,0x0A,0x0B,0x18,0x19,0x1A,0x1B};
	const vec_uchar16 shufCDab = {0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17};
	const vec_uchar16 shufABca = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x18,0x19,0x1A,0x1B,0x10,0x11,0x12,0x13};
	const vec_uchar16 shufCDda = {0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x1C,0x1D,0x1E,0x1F,0x10,0x11,0x12,0x13};
	const vec_uchar16 shufABac = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x10,0x11,0x12,0x13,0x18,0x19,0x1A,0x1B};
	const vec_uchar16 shufCDbc = {0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B};

	// Constants: Shuffle mask table
	const vec_uchar16 shufABCD = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
	const vec_uchar16 shufBADC = {0x04,0x05,0x06,0x07,0x00,0x01,0x02,0x03,0x0C,0x0D,0x0E,0x0F,0x08,0x09,0x0A,0x0B};
	const vec_uchar16 shufCDAB = {0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07};
	const vec_uchar16 shufDCBA = {0x0C,0x0D,0x0E,0x0F,0x08,0x09,0x0A,0x0B,0x04,0x05,0x06,0x07,0x00,0x01,0x02,0x03};
	static const vec_uchar16 shufTable[8] =
	{
		shufABCD,
		shufABCD,
		shufABCD,
		shufABCD,
		shufBADC,
		shufCDAB,
		shufDCBA,
		shufDCBA
	};

	// Pointers
	vec_float4* __restrict__ output = (vec_float4* __restrict__) outputJoints;
	const vec_float4* __restrict__ input = (const vec_float4* __restrict__) inputMatrices;

	// Vectorised loop count (4 iterations at a time, rounded up)
	int count4 = (count + 3) >> 2;

	for (; count4; count4--) {
		// Load matrices
		const vec_float4 m0row0 = input[0];
		const vec_float4 m0row1 = input[1];
		const vec_float4 m0row2 = input[2];
		const vec_float4 m1row0 = input[3];
		const vec_float4 m1row1 = input[4];
		const vec_float4 m1row2 = input[5];
		const vec_float4 m2row0 = input[6];
		const vec_float4 m2row1 = input[7];
		const vec_float4 m2row2 = input[8];
		const vec_float4 m3row0 = input[9];
		const vec_float4 m3row1 = input[10];
		const vec_float4 m3row2 = input[11];

		// Swizzle matrices
		const vec_float4 s00 = spu_shuffle(m0row0, m1row0, shufAaBb);
		const vec_float4 s01 = spu_shuffle(m0row0, m1row0, shufCcDd);
		const vec_float4 s02 = spu_shuffle(m2row0, m3row0, shufBbAa);
		const vec_float4 s03 = spu_shuffle(m2row0, m3row0, shufDdCc);
		const vec_float4 m00 = spu_sel(s00, s02, selABcd);		
		const vec_float4 m10 = spu_shuffle(s00, s02, shufCDab);
		const vec_float4 m20 = spu_sel(s01, s03, selABcd);

		const vec_float4 s10 = spu_shuffle(m0row1, m1row1, shufAaBb);
		const vec_float4 s11 = spu_shuffle(m0row1, m1row1, shufCcDd);
		const vec_float4 s12 = spu_shuffle(m2row1, m3row1, shufBbAa);
		const vec_float4 s13 = spu_shuffle(m2row1, m3row1, shufDdCc);
		const vec_float4 m01 = spu_sel(s10, s12, selABcd);
		const vec_float4 m11 = spu_shuffle(s10, s12, shufCDab);
		const vec_float4 m21 = spu_sel(s11, s13, selABcd);

		const vec_float4 s20 = spu_shuffle(m0row2, m1row2, shufAaBb);
		const vec_float4 s21 = spu_shuffle(m0row2, m1row2, shufCcDd);
		const vec_float4 s22 = spu_shuffle(m2row2, m3row2, shufBbAa);
		const vec_float4 s23 = spu_shuffle(m2row2, m3row2, shufDdCc);
		const vec_float4 m02 = spu_sel(s20, s22, selABcd);
		const vec_float4 m12 = spu_shuffle(s20, s22, shufCDab);
		const vec_float4 m22 = spu_sel(s21, s23, selABcd);

		const vec_float4 th0 = spu_shuffle(s01, s11, shufCcDd);
		const vec_float4 th1 = spu_shuffle(s03, s13, shufAaBb);
		const vec_float4 th2 = spu_sel(one, s21, selABcd);
		const vec_float4 th3 = spu_sel(s23, one, selABcd);
		const vec_float4 t0 = spu_shuffle(th0, th2, shufABca);
		const vec_float4 t1 = spu_shuffle(th0, th2, shufCDda);
		const vec_float4 t2 = spu_shuffle(th1, th3, shufABac);
		const vec_float4 t3 = spu_shuffle(th1, th3, shufCDbc);

		// Extract quats
		// Components require shuffling - this occurs after deswizzling
		vec_float4 trTmp = spu_add(m00, m11);
		vec_uint4 cmp1tmp = spu_cmpgt(m00, m11);
		vec_uint4 cmp1tmp2 = spu_cmpgt(m00, m22);
		vec_uint4 cmp2 = spu_cmpgt(m11, m22);
		vec_uint4 cmp1 = spu_and(cmp1tmp, cmp1tmp2);
		vec_float4 trTmp2 = spu_add(trTmp, m22);
		vec_uint4 neg1tmp = spu_orc(cmp1, cmp2);
		vec_uint4 neg2tmp = spu_or(cmp1, cmp2);
		vec_uint4 cmp0 = spu_and((vec_uint4)trTmp2, signMask);
		vec_uint4 neg0 = spu_andc(cmp0, cmp1);
		vec_uint4 neg1 = spu_and(neg1tmp, cmp0);
		vec_uint4 neg2 = spu_and(neg2tmp, cmp0);
		vec_float4 m00Tmp = (vec_float4)spu_xor((vec_uint4)m00, neg0);
		vec_float4 m11Tmp = (vec_float4)spu_xor((vec_uint4)m11, neg1);
		vec_float4 m22Tmp = (vec_float4)spu_xor((vec_uint4)m22, neg2);
		vec_float4 m21Tmp = (vec_float4)spu_xor((vec_uint4)m21, neg0);
		vec_float4 m02Tmp = (vec_float4)spu_xor((vec_uint4)m02, neg1);
		vec_float4 m10Tmp = (vec_float4)spu_xor((vec_uint4)m10, neg2);
		vec_float4 qa = spu_sub(m12, m21Tmp);
		vec_float4 qb = spu_sub(m20, m02Tmp);
		vec_float4 qc = spu_sub(m01, m10Tmp);
		vec_float4 tr = spu_add(m00Tmp, one);
		vec_float4 tmTmp = spu_add(m11Tmp, m22Tmp);
		vec_float4 qd = spu_add(tr, tmTmp);

		// Normalise quats
		vec_float4 q_sqr = spu_mul(qa, qa);
		q_sqr = spu_madd(qb, qb, q_sqr);
		q_sqr = spu_madd(qc, qc, q_sqr);
		q_sqr = spu_madd(qd, qd, q_sqr);
		vec_float4 q_rinv = spu_rsqrte(q_sqr);
		qa = spu_mul(qa, q_rinv);
		qb = spu_mul(qb, q_rinv);
		qc = spu_mul(qc, q_rinv);
		qd = spu_mul(qd, q_rinv);

		// Lookup shuffles
		vec_uint4 cnt0tmp = spu_rlmask(cmp0, -29);	  //-25
		vec_uint4 cnt1tmp = spu_rlmask(cmp1, -30);	  //-26
		vec_uint4 cnt2tmp = spu_rlmask(cmp2, -31);	  //-27
		vec_uint4 cnt0tmp2 = spu_or(cnt0tmp, cnt1tmp);
		vec_uint4 cnt0 = spu_or(cnt0tmp2, cnt2tmp);
		vec_uint4 cnt1 = spu_slqwbyte(cnt0, 4);
		vec_uint4 cnt2 = spu_slqwbyte(cnt0, 8);
		vec_uint4 cnt3 = spu_slqwbyte(cnt0, 12);
		vec_uchar16 s_shuffle0 = shufTable[si_to_int((vec_char16)cnt0)];
		vec_uchar16 s_shuffle1 = shufTable[si_to_int((vec_char16)cnt1)];
		vec_uchar16 s_shuffle2 = shufTable[si_to_int((vec_char16)cnt2)];
		vec_uchar16 s_shuffle3 = shufTable[si_to_int((vec_char16)cnt3)];

		// Deswizzle quats
		// (X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3 -> X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3)
		vec_float4 qab0ab1 = spu_shuffle(qa, qb, shufAaBb);
		vec_float4 qab2ab3 = spu_shuffle(qa, qb, shufCcDd);
		vec_float4 qcd1cd0 = spu_shuffle(qc, qd, shufBbAa);
		vec_float4 qcd3cd2 = spu_shuffle(qc, qd, shufDdCc);
		vec_float4 q0tmp = spu_sel(qab0ab1, qcd1cd0, selABcd);
		vec_float4 q1tmp = spu_shuffle(qab0ab1, qcd1cd0, shufCDab);
		vec_float4 q2tmp = spu_sel(qab2ab3, qcd3cd2, selABcd);
		vec_float4 q3tmp = spu_shuffle(qab2ab3, qcd3cd2, shufCDab);

		// Shuffle quats
		vec_float4 q0 = spu_shuffle(q0tmp, q0tmp, s_shuffle0);
		vec_float4 q1 = spu_shuffle(q1tmp, q1tmp, s_shuffle1);
		vec_float4 q2 = spu_shuffle(q2tmp, q2tmp, s_shuffle2);
		vec_float4 q3 = spu_shuffle(q3tmp, q3tmp, s_shuffle3);

		// Now for scales
		vec_float4 sx_sqr = spu_mul(m00, m00);
		sx_sqr = spu_madd(m01, m01, sx_sqr);
		sx_sqr = spu_madd(m02, m02, sx_sqr);
		vec_float4 sx_rinv = spu_rsqrte(sx_sqr);
		vec_float4 ssx = spu_mul(sx_rinv, sx_sqr);

		vec_float4 sy_sqr = spu_mul(m10, m10);
		sy_sqr = spu_madd(m11, m11, sy_sqr);
		sy_sqr = spu_madd(m12, m12, sy_sqr);
		vec_float4 sy_rinv = spu_rsqrte(sy_sqr);
		vec_float4 ssy = spu_mul(sy_rinv, sy_sqr);

		vec_float4 sz_sqr = spu_mul(m20, m20);
		sz_sqr = spu_madd(m21, m21, sz_sqr);
		sz_sqr = spu_madd(m22, m22, sz_sqr);
		vec_float4 sz_rinv = spu_rsqrte(sz_sqr);
		vec_float4 ssz = spu_mul(sz_rinv, sz_sqr);

		// Deswizzle scales
		// (X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3  -> X0Y0Z0(1) X1Y1Z1(1) X2Y2Z2(1) X3Y3Z3(1))
		const vec_float4 sh0 = spu_shuffle(ssx, ssy, shufAaBb);
		const vec_float4 sh1 = spu_shuffle(ssx, ssy, shufCcDd);
		const vec_float4 sh2 = spu_shuffle(ssz, one, shufBbAa);
		const vec_float4 sh3 = spu_shuffle(ssz, one, shufDdCc);
		const vec_float4 s0 = spu_sel(sh0, sh2, selABcd);
		const vec_float4 s1 = spu_shuffle(sh0, sh2, shufCDab);
		const vec_float4 s2 = spu_sel(sh1, sh3, selABcd);
		const vec_float4 s3 = spu_shuffle(sh1, sh3, shufCDab);

		// Write results
		output[0] = q0;
		output[1] = t0;
		output[2] = s0;
		output[3] = q1;
		output[4] = t1;
		output[5] = s1;
		output[6] = q2;
		output[7] = t2;
		output[8] = s2;
		output[9] = q3;
		output[10] = t3;
		output[11] = s3;

		// Advance output pointers
		output += 12;
		input += 12;
	}
}
