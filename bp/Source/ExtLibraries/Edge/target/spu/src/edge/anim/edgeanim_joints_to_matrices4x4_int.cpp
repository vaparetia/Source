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
 * @brief   Convert joint transforms (R/T/S) to 4x4 matrices
 *          SPU intrinisics implementation.
 *
 * @code
 * Matrix = | Sx * (0.5 - Rx*Ry - Rz*Rz)   Sx * (Rx*Ry + Rw*Rz)         Sx * (Rx*Rz - Rw*Ry)         0 |
 *          | Sy * (Rx*Ry - Rw*Rz)         Sy * (0.5 - Rx*Rx - Rz*Rz)   Sy * (Ry*Rz + Rw*Rz)         0 |
 *          | Sz * (Rx*Rz + Rw*Ry)         Sz * (Ry*Rz - Rw*Rx)         Sz * (0.5 - Rx*Rx - Ry*Ry)   0 |
 *          | Tx                           Ty                           Tz                           1 |
 *
 * @endcode
 *
 * @note    A reference C++ implementation of this function is available in
 *          target/reference/src/edge/anim/edgeanim_joints_to_matrices4x4.cpp
 *
 * @note    This function is not called unless you change the makefile which 
 *          uses asm implementation by default.
 *
 * @param   outputMatrices  Pointer to output matrix array.
 *                          Must be aligned to 16 bytes.
 *                          Due to the SIMD nature of the processing, the 
 *                          outputMatrices array must be a multiple of 4 entries 
 *                          in size. 
 * @param   inputJoints     Pointer to input joint transform array.
 *                          Must be aligned to 16 bytes.
 * @param   count           Joint/matrices count. 
 */

extern "C"
void edgeAnimJointsToMatrices4x4(void* outputMatrices, 
								 const EdgeAnimJointTransform* inputJoints, 
								 unsigned int count)
{
	EDGE_BOOKMARK(EDGE_BOOKMARK_ANIM_JOINTS_TO_MATRICES_3X4);

	EDGE_ASSERT(EDGE_IS_ALIGNED(outputMatrices, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(inputJoints, 16U));
	EDGE_ASSERT(count);

	// Constants: Misc
	const vec_float4 half = {0.5f, 0.5f, 0.5f, 0.5f};
	const vec_float4 one = {1.0f, 1.0f, 1.0f, 1.0f};
	const vec_float4 two = {2.0f, 2.0f, 2.0f, 2.0f};
	const vec_float4 zero = {0.0f, 0.0f, 0.0f, 0.0f};
	const vec_uchar16 selABcd = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	const vec_uchar16 selABCd = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF};

	// Constants: Shuffle masks
	const vec_uchar16 shufAaBb = {0x00,0x01,0x02,0x03,0x10,0x11,0x12,0x13,0x04,0x05,0x06,0x07,0x14,0x15,0x16,0x17};
	const vec_uchar16 shufBbAa = {0x04,0x05,0x06,0x07,0x14,0x15,0x16,0x17,0x00,0x01,0x02,0x03,0x10,0x11,0x12,0x13};
	const vec_uchar16 shufCcDd = {0x08,0x09,0x0A,0x0B,0x18,0x19,0x1A,0x1B,0x0C,0x0D,0x0E,0x0F,0x1C,0x1D,0x1E,0x1F};
	const vec_uchar16 shufDdCc = {0x0C,0x0D,0x0E,0x0F,0x1C,0x1D,0x1E,0x1F,0x08,0x09,0x0A,0x0B,0x18,0x19,0x1A,0x1B};
	const vec_uchar16 shufCDab = {0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17};

	// Pointers
	vec_float4* __restrict__ output = (vec_float4* __restrict__) outputMatrices;
	const vec_float4* __restrict__ input = (const vec_float4* __restrict__) inputJoints;

	// Vectorised loop count (4 iterations at a time, rounded up)
	int count4 = (count + 3) >> 2;

	for (; count4; count4--) {
		// Swizzle quats
		// (X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3 - > X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3)
		const vec_float4 r0 = input[0];
		const vec_float4 r1 = input[3];
		const vec_float4 r2 = input[6];
		const vec_float4 r3 = input[9];
		const vec_float4 rs0 = spu_shuffle(r0, r1, shufAaBb);
		const vec_float4 rs1 = spu_shuffle(r0, r1, shufCcDd);
		const vec_float4 rs2 = spu_shuffle(r2, r3, shufBbAa);
		const vec_float4 rs3 = spu_shuffle(r2, r3, shufDdCc);
		const vec_float4 rx = spu_sel(rs0, rs2, selABcd);
		const vec_float4 ry = spu_shuffle(rs0, rs2, shufCDab);
		const vec_float4 rz = spu_sel(rs1, rs3, selABcd);
		const vec_float4 rw = spu_shuffle(rs1, rs3, shufCDab);

		// Swizzle scales
		// (X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3 - > X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3)
		const vec_float4 s0 = input[2];
		const vec_float4 s1 = input[5];
		const vec_float4 s2 = input[8];
		const vec_float4 s3 = input[11];
		const vec_float4 ss0 = spu_shuffle(s0, s1, shufAaBb);
		const vec_float4 ss1 = spu_shuffle(s0, s1, shufCcDd);
		const vec_float4 ss2 = spu_shuffle(s2, s3, shufBbAa);
		const vec_float4 ss3 = spu_shuffle(s2, s3, shufDdCc);
		vec_float4 sx = spu_sel(ss0, ss2, selABcd);
		vec_float4 sy = spu_shuffle(ss0, ss2, shufCDab);
		vec_float4 sz = spu_sel(ss1, ss3, selABcd);
		vec_float4 sw = spu_shuffle(ss1, ss3, shufCDab);
		sx = spu_mul(sx, two);
		sy = spu_mul(sy, two);
		sz = spu_mul(sz, two);
		sw = spu_mul(sw, two);

		// Construct rotation matrix
		vec_float4 mrxx = spu_nmsub(rx, rx, half);
		vec_float4 mryy = spu_nmsub(ry, ry, half);
		vec_float4 rxy = spu_mul(rx, ry);
		vec_float4 rxz = spu_mul(rx, rz);
		vec_float4 ryz = spu_mul(ry, rz);
		vec_float4 m00 = spu_nmsub(rz, rz, mryy);
		vec_float4 m01 = spu_madd(rw, rz, rxy);
		vec_float4 m02 = spu_nmsub(rw, ry, rxz);
		vec_float4 m10 = spu_nmsub(rw, rz, rxy);
		vec_float4 m11 = spu_nmsub(rz, rz, mrxx);
		vec_float4 m12 = spu_madd(rw, rx, ryz);
		vec_float4 m20 = spu_madd(rw, ry, rxz);
		vec_float4 m21 = spu_nmsub(rw, rx, ryz);
		vec_float4 m22 = spu_nmsub(ry, ry, mrxx);
		m00 = spu_mul(m00, sx);
		m01 = spu_mul(m01, sx);
		m02 = spu_mul(m02, sx);
		m10 = spu_mul(m10, sy);
		m11 = spu_mul(m11, sy);
		m12 = spu_mul(m12, sy);
		m20 = spu_mul(m20, sz);
		m21 = spu_mul(m21, sz);
		m22 = spu_mul(m22, sz);

		// Deswizzle & store row 0 (for each of the 4 matrices)
		// (X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3 -> X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3)
		const vec_float4 smx0 = spu_shuffle(m00, m01, shufAaBb);
		const vec_float4 smx1 = spu_shuffle(m00, m01, shufCcDd);
		const vec_float4 smx2 = spu_shuffle(m02, zero, shufBbAa);
		const vec_float4 smx3 = spu_shuffle(m02, zero, shufDdCc);
		const vec_float4 mat0row0 = spu_sel(smx0, smx2, selABcd);
		const vec_float4 mat1row0 = spu_shuffle(smx0, smx2, shufCDab);
		const vec_float4 mat2row0 = spu_sel(smx1, smx3, selABcd);
		const vec_float4 mat3row0 = spu_shuffle(smx1, smx3, shufCDab);
		output[0] = mat0row0;
		output[4] = mat1row0;
		output[8] = mat2row0;
		output[12] = mat3row0;

		// Deswizzle & store row 1 (for each of the 4 matrices)
		// (X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3 -> X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3)
		const vec_float4 smy0 = spu_shuffle(m10, m11, shufAaBb);
		const vec_float4 smy1 = spu_shuffle(m10, m11, shufCcDd);
		const vec_float4 smy2 = spu_shuffle(m12, zero, shufBbAa);
		const vec_float4 smy3 = spu_shuffle(m12, zero, shufDdCc);
		const vec_float4 mat0row1 = spu_sel(smy0, smy2, selABcd);
		const vec_float4 mat1row1 = spu_shuffle(smy0, smy2, shufCDab);
		const vec_float4 mat2row1 = spu_sel(smy1, smy3, selABcd);
		const vec_float4 mat3row1 = spu_shuffle(smy1, smy3, shufCDab);
		output[1] = mat0row1;
		output[5] = mat1row1;
		output[9] = mat2row1;
		output[13] = mat3row1;

		// Deswizzle & store row 2 (for each of the 4 matrices)
		// (X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3 -> X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3)
		const vec_float4 smz0 = spu_shuffle(m20, m21, shufAaBb);
		const vec_float4 smz1 = spu_shuffle(m20, m21, shufCcDd);
		const vec_float4 smz2 = spu_shuffle(m22, zero, shufBbAa);
		const vec_float4 smz3 = spu_shuffle(m22, zero, shufDdCc);
		const vec_float4 mat0row2 = spu_sel(smz0, smz2, selABcd);
		const vec_float4 mat1row2 = spu_shuffle(smz0, smz2, shufCDab);
		const vec_float4 mat2row2 = spu_sel(smz1, smz3, selABcd);
		const vec_float4 mat3row2 = spu_shuffle(smz1, smz3, shufCDab);
		output[2] = mat0row2;
		output[6] = mat1row2;
		output[10] = mat2row2;
		output[14] = mat3row2;

		// Translations & store row 3
		const vec_float4 t0 = input[1];
		const vec_float4 t1 = input[4];
		const vec_float4 t2 = input[7];
		const vec_float4 t3 = input[10];
		const vec_float4 mat0row3 = spu_sel(t0, one, selABCd);
		const vec_float4 mat1row3 = spu_sel(t1, one, selABCd);
		const vec_float4 mat2row3 = spu_sel(t2, one, selABCd);
		const vec_float4 mat3row3 = spu_sel(t3, one, selABCd);
		output[3] = mat0row3;
		output[7] = mat1row3;
		output[11] = mat2row3;
		output[15] = mat3row3;

		// Advance pointers
		input += 12;
		output += 16;
	}
}
