/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <spu_intrinsics.h>

#include "edge/edge_assert.h"
#include "edge/anim/edgeanim_spu.h"

#warning sub-optimal version - using intrinisics implementation instead of asm

/** 
 * @brief   Convert joint transforms (R/T/S) from world-space to local-space
 *          SPU intrinisics implementation.
 *
 * @code
 * Rl = Conj(Rp) * Rw
 * Sl = Sw * 1/Sp (if no parent scale compensation)
 * Sl = Sw (if parent scale compensation)
 * Tl = Ts + 2 * Conj(Rp).xyz x [(Conj(Rp).xyz x Ts) + (Rp.w * Ts)]
 * where Ts = (Tw - Tp) * 1/Sp
 * @endcode
 *
 * Parent scale compensation is enabled if the high bit of the parent index is set
 *
 * @note    A reference C++ implementation of this function is available in
 *          target/reference/src/edge/anim/edgeanim_world_joints_to_local_joints.cpp
 *
 * @note    This function is not called unless you change the makefile which 
 *          uses asm implementation by default.
 *
 * @param   outputJoints    Pointer to output local joint transform array.
 *                          Must be aligned to 16 bytes.
 *                          Due to the simd nature of the processing, 
 *                          the outputJoints array must be a multiple of 4 entries in size. 
 * @param   inputJoints     Pointer to input world joint transform array.
 *                          Must be aligned to 16 bytes.
 * @param   rootJoint       Pointer to root joint transform
 *                          Must be aligned to 16 bytes.
 * @param   jointLinkage    Pointer to simd hierarchy (usually the address 
 *                          address of the simdHierarchy array in EdgeAnimSkeleton).
 *                          Must be aligned to 16 bytes.
 * @param   count           Number of entries in simd hierarchy - NOT number of joints. 
 *                          Can be calculated from the skeleton as 4 * numSimdHierarchyQuads. 
 */

extern "C"
void edgeAnimWorldJointsToLocalJoints(EdgeAnimJointTransform* outputJoints, 
									  const EdgeAnimJointTransform* inputJoints,
									  const EdgeAnimJointTransform* rootJoint,
									  const unsigned short* jointLinkage, 
									  unsigned int count)
{
	EDGE_BOOKMARK(EDGE_BOOKMARK_ANIM_WORLD_JOINTS_TO_LOCAL_JOINTS);

	EDGE_ASSERT(outputJoints != inputJoints);
	EDGE_ASSERT(EDGE_IS_ALIGNED(outputJoints, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(rootJoint, 16U));   
	EDGE_ASSERT(EDGE_IS_ALIGNED(inputJoints, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(jointLinkage, 16U));
	EDGE_ASSERT(count);

	// Constants: Misc
	const vec_float4 one = {1.0f, 1.0f, 1.0f, 1.0f};
	const vec_float4 two = {2.0f, 2.0f, 2.0f, 2.0f};
	const vec_uint4 idxMask = {0x00007FFF, 0x00007FFF, 0x00007FFF, 0x00007FFF};
	const vec_uint4 negParent = {0x00003FFF, 0x00003FFF, 0x00003FFF, 0x00003FFF};
	const vec_uint4 conjMask = {0x80000000, 0x80000000, 0x80000000, 0x80000000};

	// Constants: Shuffle masks
	const vec_uchar16 shufAAAA = {0x00,0x01,0x02,0x03,0x00,0x01,0x02,0x03,0x00,0x01,0x02,0x03,0x00,0x01,0x02,0x03};
	const vec_uchar16 shufBBBB = {0x04,0x05,0x06,0x07,0x04,0x05,0x06,0x07,0x04,0x05,0x06,0x07,0x04,0x05,0x06,0x07};
	const vec_uchar16 shufCCCC = {0x08,0x09,0x0A,0x0B,0x08,0x09,0x0A,0x0B,0x08,0x09,0x0A,0x0B,0x08,0x09,0x0A,0x0B};
	const vec_uchar16 shufDDDD = {0x0C,0x0D,0x0E,0x0F,0x0C,0x0D,0x0E,0x0F,0x0C,0x0D,0x0E,0x0F,0x0C,0x0D,0x0E,0x0F};
	const vec_uchar16 shufAaBb = {0x00,0x01,0x02,0x03,0x10,0x11,0x12,0x13,0x04,0x05,0x06,0x07,0x14,0x15,0x16,0x17};
	const vec_uchar16 shufBbAa = {0x04,0x05,0x06,0x07,0x14,0x15,0x16,0x17,0x00,0x01,0x02,0x03,0x10,0x11,0x12,0x13};
	const vec_uchar16 shufCcDd = {0x08,0x09,0x0A,0x0B,0x18,0x19,0x1A,0x1B,0x0C,0x0D,0x0E,0x0F,0x1C,0x1D,0x1E,0x1F};
	const vec_uchar16 shufDdCc = {0x0C,0x0D,0x0E,0x0F,0x1C,0x1D,0x1E,0x1F,0x08,0x09,0x0A,0x0B,0x18,0x19,0x1A,0x1B};
	const vec_uchar16 shufCDab = {0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17};
	const vec_uchar16 selABcd   = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

	// Input base pointers
	const vec_float4* __restrict__ inputBaseR = ((const vec_float4*) inputJoints) + 0;
	const vec_float4* __restrict__ inputBaseT = ((const vec_float4*) inputJoints) + 1;
	const vec_float4* __restrict__ inputBaseS = ((const vec_float4*) inputJoints) + 2;
	const vec_uint4*  __restrict__ linkageInput = ( const vec_uint4* ) jointLinkage;
	const vec_uint4 rootJointAdr = spu_splats((unsigned int) rootJoint);
	const vec_uint4 inputAdr = spu_splats((unsigned int) inputJoints);
	const vec_uint4 outputAdr = spu_splats((unsigned int) outputJoints);
	
	// Output base pointers
	const vec_float4* __restrict__ outputBaseR = ((const vec_float4* __restrict__) outputJoints) + 0;
	const vec_float4* __restrict__ outputBaseT = ((const vec_float4* __restrict__) outputJoints) + 1;
	const vec_float4* __restrict__ outputBaseS = ((const vec_float4* __restrict__) outputJoints) + 2;

	// Vectorised loop count (4 iterations at a time, rounded up)
	int count4 = (count + 3) >> 2;

	for (; count4; count4--) {
		const vec_uint4 indices = linkageInput[0];

		// Local indices
		const vec_uint4 localIdx = spu_rlmask(indices, -16);
		const vec_uint4 localOfs = spu_add(spu_sl(localIdx, 4), spu_sl(localIdx, 5));
		const vec_uint4 localOfs0 = localOfs;
		const vec_uint4 localOfs1 = spu_slqwbyte(localOfs, 4);
		const vec_uint4 localOfs2 = spu_slqwbyte(localOfs, 8);
		const vec_uint4 localOfs3 = spu_slqwbyte(localOfs, 12);

		// Parent indices
		const vec_uint4 parentIdx = spu_and(indices, idxMask);
		const vec_uint4 parentUseRoot = spu_cmpgt(parentIdx, negParent);
		const vec_uint4 parentOfs = spu_add(spu_sl(parentIdx, 4), spu_sl(parentIdx, 5));
		const vec_uint4 parentWorldAdrI = spu_add(inputAdr, parentOfs);
		const vec_uint4 parentLocalAdrI = spu_add(outputAdr, parentOfs);
		const vec_uint4 parentWorldAdr = spu_sel(parentWorldAdrI, rootJointAdr, parentUseRoot);
		const vec_uint4 parentLocalAdr = spu_sel(parentLocalAdrI, rootJointAdr, parentUseRoot);

		const vec_uint4 parentWorldAdr0 = parentWorldAdr;
		const vec_uint4 parentWorldAdr1 = spu_slqwbyte(parentWorldAdr, 4);
		const vec_uint4 parentWorldAdr2 = spu_slqwbyte(parentWorldAdr, 8);
		const vec_uint4 parentWorldAdr3 = spu_slqwbyte(parentWorldAdr, 12);

		const vec_uint4 parentLocalAdr0 = parentLocalAdr;
		const vec_uint4 parentLocalAdr1 = spu_slqwbyte(parentLocalAdr, 4);
		const vec_uint4 parentLocalAdr2 = spu_slqwbyte(parentLocalAdr, 8);
		const vec_uint4 parentLocalAdr3 = spu_slqwbyte(parentLocalAdr, 12);

		// World joints (lqx)
		const vec_float4* __restrict__ inputR0 = (const vec_float4*) 
			((uintptr_t) inputBaseR + si_to_uint((vec_char16) localOfs0));
		const vec_float4* __restrict__ inputR1 = (const vec_float4*) 
			((uintptr_t) inputBaseR + si_to_uint((vec_char16) localOfs1));
		const vec_float4* __restrict__ inputR2 = (const vec_float4*) 
			((uintptr_t) inputBaseR + si_to_uint((vec_char16) localOfs2));
		const vec_float4* __restrict__ inputR3 = (const vec_float4*) 
			((uintptr_t) inputBaseR + si_to_uint((vec_char16) localOfs3));
		const vec_float4* __restrict__ inputT0 = (const vec_float4*) 
			((uintptr_t) inputBaseT + si_to_uint((vec_char16) localOfs0));
		const vec_float4* __restrict__ inputT1 = (const vec_float4*) 
			((uintptr_t) inputBaseT + si_to_uint((vec_char16) localOfs1));
		const vec_float4* __restrict__ inputT2 = (const vec_float4*) 
			((uintptr_t) inputBaseT + si_to_uint((vec_char16) localOfs2));
		const vec_float4* __restrict__ inputT3 = (const vec_float4*) 
			((uintptr_t) inputBaseT + si_to_uint((vec_char16) localOfs3));
		const vec_float4* __restrict__ inputS0 = (const vec_float4*) 
			((uintptr_t) inputBaseS + si_to_uint((vec_char16) localOfs0));
		const vec_float4* __restrict__ inputS1 = (const vec_float4*) 
			((uintptr_t) inputBaseS + si_to_uint((vec_char16) localOfs1));
		const vec_float4* __restrict__ inputS2 = (const vec_float4*) 
			((uintptr_t) inputBaseS + si_to_uint((vec_char16) localOfs2));
		const vec_float4* __restrict__ inputS3 = (const vec_float4*) 
			((uintptr_t) inputBaseS + si_to_uint((vec_char16) localOfs3));
		const vec_float4 wr0 = *inputR0;
		const vec_float4 wr1 = *inputR1;
		const vec_float4 wr2 = *inputR2;
		const vec_float4 wr3 = *inputR3;
		const vec_float4 wt0 = *inputT0;
		const vec_float4 wt1 = *inputT1;
		const vec_float4 wt2 = *inputT2;
		const vec_float4 wt3 = *inputT3;
		const vec_float4 ws0 = *inputS0;
		const vec_float4 ws1 = *inputS1;
		const vec_float4 ws2 = *inputS2;
		const vec_float4 ws3 = *inputS3;

		// Parent joints - warning: aliasing between loop iterations
		const vec_float4 pwr0 = ((const vec_float4* __restrict__) si_to_uint((vec_char16) parentWorldAdr0))[0];
		const vec_float4 pwr1 = ((const vec_float4* __restrict__) si_to_uint((vec_char16) parentWorldAdr1))[0];
		const vec_float4 pwr2 = ((const vec_float4* __restrict__) si_to_uint((vec_char16) parentWorldAdr2))[0];
		const vec_float4 pwr3 = ((const vec_float4* __restrict__) si_to_uint((vec_char16) parentWorldAdr3))[0];
		const vec_float4 pwt0 = ((const vec_float4* __restrict__) si_to_uint((vec_char16) parentWorldAdr0))[1];
		const vec_float4 pwt1 = ((const vec_float4* __restrict__) si_to_uint((vec_char16) parentWorldAdr1))[1];
		const vec_float4 pwt2 = ((const vec_float4* __restrict__) si_to_uint((vec_char16) parentWorldAdr2))[1];
		const vec_float4 pwt3 = ((const vec_float4* __restrict__) si_to_uint((vec_char16) parentWorldAdr3))[1];
		const vec_float4 pws0 = ((const vec_float4* __restrict__) si_to_uint((vec_char16) parentWorldAdr0))[2];
		const vec_float4 pws1 = ((const vec_float4* __restrict__) si_to_uint((vec_char16) parentWorldAdr1))[2];
		const vec_float4 pws2 = ((const vec_float4* __restrict__) si_to_uint((vec_char16) parentWorldAdr2))[2];
		const vec_float4 pws3 = ((const vec_float4* __restrict__) si_to_uint((vec_char16) parentWorldAdr3))[2];

		const vec_float4 pls0 = ((const vec_float4* __restrict__) si_to_uint((vec_char16) parentLocalAdr0))[2];
		const vec_float4 pls1 = ((const vec_float4* __restrict__) si_to_uint((vec_char16) parentLocalAdr1))[2];
		const vec_float4 pls2 = ((const vec_float4* __restrict__) si_to_uint((vec_char16) parentLocalAdr2))[2];
		const vec_float4 pls3 = ((const vec_float4* __restrict__) si_to_uint((vec_char16) parentLocalAdr3))[2];

		// Swizzle world quats
		// (X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3 - > X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3)
		const vec_float4 wrs0 = spu_shuffle(wr0, wr1, shufAaBb);
		const vec_float4 wrs1 = spu_shuffle(wr0, wr1, shufCcDd);
		const vec_float4 wrs2 = spu_shuffle(wr2, wr3, shufBbAa);
		const vec_float4 wrs3 = spu_shuffle(wr2, wr3, shufDdCc);
		const vec_float4 wrx = spu_sel(wrs0, wrs2, selABcd);
		const vec_float4 wry = spu_shuffle(wrs0, wrs2, shufCDab);
		const vec_float4 wrz = spu_sel(wrs1, wrs3, selABcd);
		const vec_float4 wrw = spu_shuffle(wrs1, wrs3, shufCDab);

		// Swizzle parent world quats
		// (X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3 - > X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3)
		const vec_float4 pwrs0 = spu_shuffle(pwr0, pwr1, shufAaBb);
		const vec_float4 pwrs1 = spu_shuffle(pwr0, pwr1, shufCcDd);
		const vec_float4 pwrs2 = spu_shuffle(pwr2, pwr3, shufBbAa);
		const vec_float4 pwrs3 = spu_shuffle(pwr2, pwr3, shufDdCc);
		const vec_float4 pwrx = spu_sel(pwrs0, pwrs2, selABcd);
		const vec_float4 pwry = spu_shuffle(pwrs0, pwrs2, shufCDab);
		const vec_float4 pwrz = spu_sel(pwrs1, pwrs3, selABcd);
		const vec_float4 pwrw = spu_shuffle(pwrs1, pwrs3, shufCDab);

		// Invert parent world quats
		const vec_float4 pwrix = (vec_float4)spu_xor((vec_uint4)pwrx, conjMask);
		const vec_float4 pwriy = (vec_float4)spu_xor((vec_uint4)pwry, conjMask);
		const vec_float4 pwriz = (vec_float4)spu_xor((vec_uint4)pwrz, conjMask);
		const vec_float4 pwriw = pwrw;

		// Local quat = inverse parent world quat * world quat
		const vec_float4 x0 = spu_mul(pwriw, wrx);
		const vec_float4 x1 = spu_madd(pwrix, wrw, x0);
		const vec_float4 x2 = spu_madd(pwriy, wrz, x1);
		const vec_float4 lrx = spu_nmsub(pwriz, wry, x2);
		const vec_float4 y0 = spu_mul(pwriw, wry);
		const vec_float4 y1 = spu_nmsub(pwrix, wrz, y0);
		const vec_float4 y2 = spu_madd(pwriy, wrw, y1);
		const vec_float4 lry = spu_madd(pwriz, wrx, y2);
		const vec_float4 z0 = spu_mul(pwriw, wrz);
		const vec_float4 z1 = spu_madd(pwrix, wry, z0);
		const vec_float4 z2 = spu_nmsub(pwriy, wrx, z1);
		const vec_float4 lrz = spu_madd(pwriz, wrw, z2);
		const vec_float4 w0 = spu_mul(pwriw, wrw);
		const vec_float4 w1 = spu_nmsub(pwrix, wrx, w0);
		const vec_float4 w2 = spu_nmsub(pwriy, wry, w1);
		const vec_float4 lrw = spu_nmsub(pwriz, wrz, w2);

		// Deswizzle local quats
		// (X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3 -> X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3)
		const vec_float4 s0 = spu_shuffle(lrx, lry, shufAaBb);
		const vec_float4 s1 = spu_shuffle(lrx, lry, shufCcDd);
		const vec_float4 s2 = spu_shuffle(lrz, lrw, shufBbAa);
		const vec_float4 s3 = spu_shuffle(lrz, lrw, shufDdCc);
		const vec_float4 lr0 = spu_sel(s0, s2, selABcd);
		const vec_float4 lr1 = spu_shuffle(s0, s2, shufCDab);
		const vec_float4 lr2 = spu_sel(s1, s3, selABcd);
		const vec_float4 lr3 = spu_shuffle(s1, s3, shufCDab);

		// Store rotation (stqx)
		vec_float4* outputR0 = (vec_float4* __restrict__) 
			((uintptr_t) outputBaseR + si_to_uint((vec_char16) localOfs0));
		vec_float4* outputR1 = (vec_float4* __restrict__) 
			((uintptr_t) outputBaseR + si_to_uint((vec_char16) localOfs1));
		vec_float4* outputR2 = (vec_float4* __restrict__) 
			((uintptr_t) outputBaseR + si_to_uint((vec_char16) localOfs2));
		vec_float4* outputR3 = (vec_float4* __restrict__) 
			((uintptr_t) outputBaseR + si_to_uint((vec_char16) localOfs3));
		(*outputR0) = lr0;
		(*outputR1) = lr1;
		(*outputR2) = lr2;
		(*outputR3) = lr3;

		// 1 / parent world scale
		vec_float4 pwsi0 = spu_re(pws0);
		vec_float4 pwsi1 = spu_re(pws1);
		vec_float4 pwsi2 = spu_re(pws2);
		vec_float4 pwsi3 = spu_re(pws3);
		// nr refinement adds 1 cycle/joint
		vec_float4 tmp;

		tmp = spu_nmsub(pws0, pwsi0, one);
		pwsi0 = spu_madd(tmp, pwsi0, pwsi0);

		tmp = spu_nmsub(pws1, pwsi1, one);
		pwsi1 = spu_madd(tmp, pwsi1, pwsi1);

		tmp = spu_nmsub(pws2, pwsi2, one);
		pwsi2 = spu_madd(tmp, pwsi2, pwsi2);

		tmp = spu_nmsub(pws3, pwsi3, one);
		pwsi3 = spu_madd(tmp, pwsi3, pwsi3);

		// Scale compensate
		const vec_uint4 compensateBit = spu_sl(indices, 16);
		const vec_uint4 compensateMask = spu_rlmaska(compensateBit, -31);
		const vec_uint4 compensateMask0 = spu_shuffle(compensateMask, compensateMask, shufAAAA);
		const vec_uint4 compensateMask1 = spu_shuffle(compensateMask, compensateMask, shufBBBB);
		const vec_uint4 compensateMask2 = spu_shuffle(compensateMask, compensateMask, shufCCCC);
		const vec_uint4 compensateMask3 = spu_shuffle(compensateMask, compensateMask, shufDDDD);
		const vec_float4 scale0 = spu_sel(one, pls0, compensateMask0);
		const vec_float4 scale1 = spu_sel(one, pls1, compensateMask1);
		const vec_float4 scale2 = spu_sel(one, pls2, compensateMask2);
		const vec_float4 scale3 = spu_sel(one, pls3, compensateMask3);

		// Final scale
		const vec_float4 ls0 = spu_mul(spu_mul(ws0, scale0), pwsi0);
		const vec_float4 ls1 = spu_mul(spu_mul(ws1, scale1), pwsi1);
		const vec_float4 ls2 = spu_mul(spu_mul(ws2, scale2), pwsi2);
		const vec_float4 ls3 = spu_mul(spu_mul(ws3, scale3), pwsi3);

		// Store scale (stqx)
		vec_float4* outputS0 = (vec_float4* __restrict__) 
			((uintptr_t) outputBaseS + si_to_uint((vec_char16) localOfs0));
		vec_float4* outputS1 = (vec_float4* __restrict__) 
			((uintptr_t) outputBaseS + si_to_uint((vec_char16) localOfs1));
		vec_float4* outputS2 = (vec_float4* __restrict__) 
			((uintptr_t) outputBaseS + si_to_uint((vec_char16) localOfs2));
		vec_float4* outputS3 = (vec_float4* __restrict__) 
			((uintptr_t) outputBaseS + si_to_uint((vec_char16) localOfs3));
		(*outputS0) = ls0;
		(*outputS1) = ls1;
		(*outputS2) = ls2;
		(*outputS3) = ls3;
				
		// Local translation offset from parent world translation
		const vec_float4 lt0 = spu_sub(wt0, pwt0);
		const vec_float4 lt1 = spu_sub(wt1, pwt1);
		const vec_float4 lt2 = spu_sub(wt2, pwt2);
		const vec_float4 lt3 = spu_sub(wt3, pwt3);

		// Swizzle local translation
		// (X0Y0Z0* X1Y1Z1* X2Y2Z2* X3Y3Z3* - > X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3)
		const vec_float4 lth0 = spu_shuffle(lt0, lt1, shufAaBb);
		const vec_float4 lth1 = spu_shuffle(lt0, lt1, shufCcDd);
		const vec_float4 lth2 = spu_shuffle(lt2, lt3, shufBbAa);
		const vec_float4 lth3 = spu_shuffle(lt2, lt3, shufDdCc);
		const vec_float4 ltx = spu_sel(lth0, lth2, selABcd);
		const vec_float4 lty = spu_shuffle(lth0, lth2, shufCDab);
		const vec_float4 ltz = spu_sel(lth1, lth3, selABcd);

		// c = Cross(pwr, lt) + pwriw*lt = 
		// (pwriy*ltz-pwriz*lty, pwriz*ltx-pwrix*ltz, pwrix*lty-pwriy*ltx) + pwriw*lt
		const vec_float4 cx0 = spu_mul(pwriy, ltz);
		const vec_float4 cx1 = spu_nmsub(pwriz, lty, cx0);
		const vec_float4 cx = spu_madd(pwriw, ltx, cx1);
		const vec_float4 cy0 = spu_mul(pwriz, ltx);
		const vec_float4 cy1 = spu_nmsub(pwrix, ltz, cy0);
		const vec_float4 cy = spu_madd(pwriw, lty, cy1);
		const vec_float4 cz0 = spu_mul(pwrix, lty);
		const vec_float4 cz1 = spu_nmsub(pwriy, ltx, cz0);
		const vec_float4 cz = spu_madd(pwriw, ltz, cz1);

		// d = lt + 2 * Cross(pwr, c)
		const vec_float4 dx0 = spu_mul(pwriy, cz);
		const vec_float4 dx1 = spu_nmsub(pwriz, cy, dx0);
		const vec_float4 dx = spu_madd(two, dx1, ltx);
		const vec_float4 dy0 = spu_mul(pwriz, cx);
		const vec_float4 dy1 = spu_nmsub(pwrix, cz, dy0);
		const vec_float4 dy = spu_madd(two, dy1, lty);
		const vec_float4 dz0 = spu_mul(pwrix, cy);
		const vec_float4 dz1 = spu_nmsub(pwriy, cx, dz0);
		const vec_float4 dz = spu_madd(two, dz1, ltz);

		// Deswizzle rotated translation
		// (X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3 -> X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3)
		const vec_float4 s4 = spu_shuffle(dx, dy, shufAaBb);
		const vec_float4 s5 = spu_shuffle(dx, dy, shufCcDd);
		const vec_float4 s6 = spu_shuffle(dz, one, shufBbAa);
		const vec_float4 s7 = spu_shuffle(dz, one, shufDdCc);
		const vec_float4 rt0 = spu_sel(s4, s6, selABcd);
		const vec_float4 rt1 = spu_shuffle(s4, s6, shufCDab);
		const vec_float4 rt2 = spu_sel(s5, s7, selABcd);
		const vec_float4 rt3 = spu_shuffle(s5, s7, shufCDab);

		// Scale local translation
		const vec_float4 lrts0 = spu_mul(rt0, pwsi0);
		const vec_float4 lrts1 = spu_mul(rt1, pwsi1);
		const vec_float4 lrts2 = spu_mul(rt2, pwsi2);
		const vec_float4 lrts3 = spu_mul(rt3, pwsi3);

		// Store translation (stqx)
		vec_float4* outputT0 = (vec_float4* __restrict__) 
			((uintptr_t) outputBaseT + si_to_uint((vec_char16) localOfs0));
		vec_float4* outputT1 = (vec_float4* __restrict__) 
			((uintptr_t) outputBaseT + si_to_uint((vec_char16) localOfs1));
		vec_float4* outputT2 = (vec_float4* __restrict__) 
			((uintptr_t) outputBaseT + si_to_uint((vec_char16) localOfs2));
		vec_float4* outputT3 = (vec_float4* __restrict__) 
			((uintptr_t) outputBaseT + si_to_uint((vec_char16) localOfs3));
		(*outputT0) = lrts0;
		(*outputT1) = lrts1;
		(*outputT2) = lrts2;
		(*outputT3) = lrts3;

		linkageInput++;
	}
}
