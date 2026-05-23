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
 * @brief   Convert joint transforms (R/T/S) from local-space to world-space
 *          SPU intrinisics implementation.
 *
 * Rw = Rp * Rl
 * Sw = Sp * Sl (if no parent scale compensation)
 * Sw = Sl (if parent scale compensation)
 * Tw = Tp + (Sp * Tl) + 2 * Rp.xyz x [Rp.xyz x (Sp * Tl) + Rp.w * Sp * Tl]
 *
 * Parent scale compensation is enabled if the high bit of the parent index is set
 *
 * @note    A reference C++ implementation of this function is available in
 *          target/reference/src/edge/anim/edgeanim_local_joints_to_world_joints.cpp
 *
 * @note    This function is not called unless you change the makefile which 
 *          uses asm implementation by default.
 *
 * @param   outputJoints    Pointer to output world joint transform array.
 *                          Must be aligned to 16 bytes.
 *                          Due to the SIMD nature of the processing, 
 *                          the outputJoints array must be a multiple of 4 entries in size. 
 * @param   inputJoints     Pointer to input local joint transform array.
 *                          Must be aligned to 16 bytes.
 * @param   rootJoint       Pointer to root joint transform.
 *                          Must be aligned to 16 bytes.
 * @param   jointLinkage    Pointer to simd hierarchy (usually the address 
 *                          address of the simdHierarchy array in EdgeAnimSkeleton)
 *                          Must be aligned to 16 bytes.
 * @param   count           Number of entries in simd hierarchy - NOT number of joints. 
 *                          Can be calculated from the skeleton as 4 * numSimdHierarchyQuads. 
 */

extern "C"
void edgeAnimLocalJointsToWorldJoints(EdgeAnimJointTransform* outputJoints, 
									  const EdgeAnimJointTransform* inputJoints,
									  const EdgeAnimJointTransform* rootJoint,
									  const unsigned short* jointLinkage, 
									  unsigned int count)
{   
	EDGE_BOOKMARK(EDGE_BOOKMARK_ANIM_LOCAL_JOINTS_TO_WORLD_JOINTS);

	EDGE_ASSERT(outputJoints != inputJoints);
	EDGE_ASSERT(EDGE_IS_ALIGNED(outputJoints, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(rootJoint, 16U));   
	EDGE_ASSERT(EDGE_IS_ALIGNED(inputJoints, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(jointLinkage, 16U));
	EDGE_ASSERT(count);

	// Constants: Misc
	const vec_float4 one = {1.0f, 1.0f, 1.0f, 1.0f};
	const vec_float4 two = {2.0f, 2.0f, 2.0f, 2.0f};
	const vec_float4 half = {0.5f, 0.5f, 0.5f, 0.5f};
	const vec_uchar16 selABcd = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	const vec_uint4 negParent = {0x00003FFF, 0x00003FFF, 0x00003FFF, 0x00003FFF};   
	const vec_uint4 idxMask = {0x00007FFF, 0x00007FFF, 0x00007FFF, 0x00007FFF};

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

	// Input base pointers
	const vec_float4* __restrict__ inputBaseR = ((const vec_float4* __restrict__) inputJoints) + 0;
	const vec_float4* __restrict__ inputBaseT = ((const vec_float4* __restrict__) inputJoints) + 1;
	const vec_float4* __restrict__ inputBaseS = ((const vec_float4* __restrict__) inputJoints) + 2;
	const vec_uint4*  __restrict__ linkageInput = ( const vec_uint4* ) jointLinkage;
	const vec_uint4 rootJointAdr = spu_splats((unsigned int) rootJoint);
	const vec_uint4 outputAdr = spu_splats((unsigned int) outputJoints);
	const vec_uint4 inputAdr = spu_splats((unsigned int) inputJoints);

	// Output base pointers 
	const vec_float4* outputBaseR = ((const vec_float4*) outputJoints) + 0;
	const vec_float4* outputBaseT = ((const vec_float4*) outputJoints) + 1;
	const vec_float4* outputBaseS = ((const vec_float4*) outputJoints) + 2;

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
		const vec_uint4 parentLocalAdrI = spu_add(inputAdr, parentOfs);
		const vec_uint4 parentWorldAdrI = spu_add(outputAdr, parentOfs);
		const vec_uint4 parentLocalAdr = spu_sel(parentLocalAdrI, rootJointAdr, parentUseRoot);
		const vec_uint4 parentWorldAdr = spu_sel(parentWorldAdrI, rootJointAdr, parentUseRoot);

		const vec_uint4 parentWorldAdr0 = parentWorldAdr;
		const vec_uint4 parentWorldAdr1 = spu_slqwbyte(parentWorldAdr, 4);
		const vec_uint4 parentWorldAdr2 = spu_slqwbyte(parentWorldAdr, 8);
		const vec_uint4 parentWorldAdr3 = spu_slqwbyte(parentWorldAdr, 12);

		const vec_uint4 parentLocalAdr0 = parentLocalAdr;
		const vec_uint4 parentLocalAdr1 = spu_slqwbyte(parentLocalAdr, 4);
		const vec_uint4 parentLocalAdr2 = spu_slqwbyte(parentLocalAdr, 8);
		const vec_uint4 parentLocalAdr3 = spu_slqwbyte(parentLocalAdr, 12);

		// Local joints (lqx)
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
		const vec_float4 lr0 = *inputR0;
		const vec_float4 lr1 = *inputR1;
		const vec_float4 lr2 = *inputR2;
		const vec_float4 lr3 = *inputR3;
		const vec_float4 lt0 = *inputT0;
		const vec_float4 lt1 = *inputT1;
		const vec_float4 lt2 = *inputT2;
		const vec_float4 lt3 = *inputT3;
		const vec_float4 ls0 = *inputS0;
		const vec_float4 ls1 = *inputS1;
		const vec_float4 ls2 = *inputS2;
		const vec_float4 ls3 = *inputS3;

		// Parent joints - warning: aliasing between loop iterations
		const vec_float4 pwr0 = ((const vec_float4*) si_to_uint((vec_char16) parentWorldAdr0))[0];
		const vec_float4 pwr1 = ((const vec_float4*) si_to_uint((vec_char16) parentWorldAdr1))[0];
		const vec_float4 pwr2 = ((const vec_float4*) si_to_uint((vec_char16) parentWorldAdr2))[0];
		const vec_float4 pwr3 = ((const vec_float4*) si_to_uint((vec_char16) parentWorldAdr3))[0];
		const vec_float4 pwt0 = ((const vec_float4*) si_to_uint((vec_char16) parentWorldAdr0))[1];
		const vec_float4 pwt1 = ((const vec_float4*) si_to_uint((vec_char16) parentWorldAdr1))[1];
		const vec_float4 pwt2 = ((const vec_float4*) si_to_uint((vec_char16) parentWorldAdr2))[1];
		const vec_float4 pwt3 = ((const vec_float4*) si_to_uint((vec_char16) parentWorldAdr3))[1];
		const vec_float4 pws0 = ((const vec_float4*) si_to_uint((vec_char16) parentWorldAdr0))[2];
		const vec_float4 pws1 = ((const vec_float4*) si_to_uint((vec_char16) parentWorldAdr1))[2];
		const vec_float4 pws2 = ((const vec_float4*) si_to_uint((vec_char16) parentWorldAdr2))[2];
		const vec_float4 pws3 = ((const vec_float4*) si_to_uint((vec_char16) parentWorldAdr3))[2];

		const vec_float4 pls0 = ((const vec_float4* __restrict__) si_to_uint((vec_char16) parentLocalAdr0))[2];
		const vec_float4 pls1 = ((const vec_float4* __restrict__) si_to_uint((vec_char16) parentLocalAdr1))[2];
		const vec_float4 pls2 = ((const vec_float4* __restrict__) si_to_uint((vec_char16) parentLocalAdr2))[2];
		const vec_float4 pls3 = ((const vec_float4* __restrict__) si_to_uint((vec_char16) parentLocalAdr3))[2];

		// Swizzle local quats
		// (X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3 - > X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3)
		const vec_float4 lrs0 = spu_shuffle(lr0, lr1, shufAaBb);
		const vec_float4 lrs1 = spu_shuffle(lr0, lr1, shufCcDd);
		const vec_float4 lrs2 = spu_shuffle(lr2, lr3, shufBbAa);
		const vec_float4 lrs3 = spu_shuffle(lr2, lr3, shufDdCc);
		const vec_float4 lrx = spu_sel(lrs0, lrs2, selABcd);
		const vec_float4 lry = spu_shuffle(lrs0, lrs2, shufCDab);
		const vec_float4 lrz = spu_sel(lrs1, lrs3, selABcd);
		const vec_float4 lrw = spu_shuffle(lrs1, lrs3, shufCDab);

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

		// World quat = parent world quat * local quat
		const vec_float4 x0 = spu_mul(pwrw, lrx);
		const vec_float4 x1 = spu_madd(pwrx, lrw, x0);
		const vec_float4 x2 = spu_madd(pwry, lrz, x1);
		vec_float4 wrx = spu_nmsub(pwrz, lry, x2);
		const vec_float4 y0 = spu_mul(pwrw, lry);
		const vec_float4 y1 = spu_nmsub(pwrx, lrz, y0);
		const vec_float4 y2 = spu_madd(pwry, lrw, y1);
		vec_float4 wry = spu_madd(pwrz, lrx, y2);
		const vec_float4 z0 = spu_mul(pwrw, lrz);
		const vec_float4 z1 = spu_madd(pwrx, lry, z0);
		const vec_float4 z2 = spu_nmsub(pwry, lrx, z1);
		vec_float4 wrz = spu_madd(pwrz, lrw, z2);
		const vec_float4 w0 = spu_mul(pwrw, lrw);
		const vec_float4 w1 = spu_nmsub(pwrx, lrx, w0);
		const vec_float4 w2 = spu_nmsub(pwry, lry, w1);
		vec_float4 wrw = spu_nmsub(pwrz, lrz, w2);

		// Normalise world quats
		vec_float4 wrsqr = spu_mul(wrx, wrx);
		wrsqr = spu_madd(wry, wry, wrsqr);
		wrsqr = spu_madd(wrz, wrz, wrsqr);
		wrsqr = spu_madd(wrw, wrw, wrsqr);
		vec_float4 ntmp0 = (vec_float4)si_frsqest((qword)wrsqr);
		vec_float4 ntmp1 = (vec_float4)si_fi((qword)wrsqr, (qword)ntmp0);
		vec_float4 ntmp2 = spu_mul(wrsqr, ntmp1);
		vec_float4 ntmp3 = spu_mul(ntmp1, half);
		vec_float4 ntmp4 = spu_nmsub(ntmp2, ntmp1, one);
		vec_float4 inv = spu_madd(ntmp3, ntmp4, ntmp1);
		wrx = spu_mul(wrx, inv);
		wry = spu_mul(wry, inv);
		wrz = spu_mul(wrz, inv);
		wrw = spu_mul(wrw, inv);

		// Deswizzle world quats
		// (X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3 -> X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3)
		const vec_float4 s0 = spu_shuffle(wrx, wry, shufAaBb);
		const vec_float4 s1 = spu_shuffle(wrx, wry, shufCcDd);
		const vec_float4 s2 = spu_shuffle(wrz, wrw, shufBbAa);
		const vec_float4 s3 = spu_shuffle(wrz, wrw, shufDdCc);
		const vec_float4 wr0 = spu_sel(s0, s2, selABcd);
		const vec_float4 wr1 = spu_shuffle(s0, s2, shufCDab);
		const vec_float4 wr2 = spu_sel(s1, s3, selABcd);
		const vec_float4 wr3 = spu_shuffle(s1, s3, shufCDab);
	
		// Store rotation (stqx)
		vec_float4* outputR0 = (vec_float4*) ((uintptr_t) outputBaseR 
			+ si_to_uint((vec_char16) localOfs0));
		vec_float4* outputR1 = (vec_float4*) ((uintptr_t) outputBaseR 
			+ si_to_uint((vec_char16) localOfs1));
		vec_float4* outputR2 = (vec_float4*) ((uintptr_t) outputBaseR 
			+ si_to_uint((vec_char16) localOfs2));
		vec_float4* outputR3 = (vec_float4*) ((uintptr_t) outputBaseR 
			+ si_to_uint((vec_char16) localOfs3));
		(*outputR0) = wr0;
		(*outputR1) = wr1;
		(*outputR2) = wr2;
		(*outputR3) = wr3;

		// 1 / parent local scale
		vec_float4 plsi0 = spu_re(pls0);
		vec_float4 plsi1 = spu_re(pls1);
		vec_float4 plsi2 = spu_re(pls2);
		vec_float4 plsi3 = spu_re(pls3);
		// nr refinement adds 1 cycle/joint
		vec_float4 tmp;

		tmp = spu_nmsub(pls0, plsi0, one);
		plsi0 = spu_madd(tmp, plsi0, plsi0);

		tmp = spu_nmsub(pls1, plsi1, one);
		plsi1 = spu_madd(tmp, plsi1, plsi1);

		tmp = spu_nmsub(pls2, plsi2, one);
		plsi2 = spu_madd(tmp, plsi2, plsi2);

		tmp = spu_nmsub(pls3, plsi3, one);
		plsi3 = spu_madd(tmp, plsi3, plsi3);

		// Scale compensate
		const vec_uint4 compensateBit = spu_sl(indices, 16);
		const vec_uint4 compensateMask = spu_rlmaska(compensateBit, -31);
		const vec_uint4 compensateMask0 = spu_shuffle(compensateMask, 
			compensateMask, shufAAAA);
		const vec_uint4 compensateMask1 = spu_shuffle(compensateMask, 
			compensateMask, shufBBBB);
		const vec_uint4 compensateMask2 = spu_shuffle(compensateMask, 
			compensateMask, shufCCCC);
		const vec_uint4 compensateMask3 = spu_shuffle(compensateMask, 
			compensateMask, shufDDDD);
		const vec_float4 scale0 = spu_sel(one, plsi0, compensateMask0);
		const vec_float4 scale1 = spu_sel(one, plsi1, compensateMask1);
		const vec_float4 scale2 = spu_sel(one, plsi2, compensateMask2);
		const vec_float4 scale3 = spu_sel(one, plsi3, compensateMask3);

		// Final scale
		const vec_float4 lss0 = spu_mul(spu_mul(ls0, pws0), scale0);
		const vec_float4 lss1 = spu_mul(spu_mul(ls1, pws1), scale1);
		const vec_float4 lss2 = spu_mul(spu_mul(ls2, pws2), scale2);
		const vec_float4 lss3 = spu_mul(spu_mul(ls3, pws3), scale3);

		// Store scale (stqx)
		vec_float4* outputS0 = (vec_float4*) ((uintptr_t) outputBaseS 
			+ si_to_uint((vec_char16) localOfs0));
		vec_float4* outputS1 = (vec_float4*) ((uintptr_t) outputBaseS 
			+ si_to_uint((vec_char16) localOfs1));
		vec_float4* outputS2 = (vec_float4*) ((uintptr_t) outputBaseS 
			+ si_to_uint((vec_char16) localOfs2));
		vec_float4* outputS3 = (vec_float4*) ((uintptr_t) outputBaseS 
			+ si_to_uint((vec_char16) localOfs3));
		(*outputS0) = lss0;
		(*outputS1) = lss1;
		(*outputS2) = lss2;
		(*outputS3) = lss3;

		// Scale and swizzle local translation
		const vec_float4 lts0 = spu_mul(lt0, pws0);
		const vec_float4 lts1 = spu_mul(lt1, pws1);
		const vec_float4 lts2 = spu_mul(lt2, pws2);
		const vec_float4 lts3 = spu_mul(lt3, pws3);
		const vec_float4 ltsh0 = spu_shuffle(lts0, lts1, shufAaBb);
		const vec_float4 ltsh1 = spu_shuffle(lts0, lts1, shufCcDd);
		const vec_float4 ltsh2 = spu_shuffle(lts2, lts3, shufBbAa);
		const vec_float4 ltsh3 = spu_shuffle(lts2, lts3, shufDdCc);
		const vec_float4 ltsx = spu_sel(ltsh0, ltsh2, selABcd);
		const vec_float4 ltsy = spu_shuffle(ltsh0, ltsh2, shufCDab);
		const vec_float4 ltsz = spu_sel(ltsh1, ltsh3, selABcd);

		// c = Cross(pwr, lt) + pwrw*lts = 
		// (pwry*ltsz-pwrz*ltsy, pwrz*ltsx-pwrx*ltsz, pwrx*ltsy-pwry*ltsx) + pwrw*lts
		const vec_float4 cx0 = spu_mul(pwry, ltsz);
		const vec_float4 cx1 = spu_nmsub(pwrz, ltsy, cx0);
		const vec_float4 cx = spu_madd(pwrw, ltsx, cx1);
		const vec_float4 cy0 = spu_mul(pwrz, ltsx);
		const vec_float4 cy1 = spu_nmsub(pwrx, ltsz, cy0);
		const vec_float4 cy = spu_madd(pwrw, ltsy, cy1);
		const vec_float4 cz0 = spu_mul(pwrx, ltsy);
		const vec_float4 cz1 = spu_nmsub(pwry, ltsx, cz0);
		const vec_float4 cz = spu_madd(pwrw, ltsz, cz1);

		// d = lt + 2 * Cross(pwr, c)
		const vec_float4 dx0 = spu_mul(pwry, cz);
		const vec_float4 dx1 = spu_nmsub(pwrz, cy, dx0);
		const vec_float4 dx = spu_madd(two, dx1, ltsx);
		const vec_float4 dy0 = spu_mul(pwrz, cx);
		const vec_float4 dy1 = spu_nmsub(pwrx, cz, dy0);
		const vec_float4 dy = spu_madd(two, dy1, ltsy);
		const vec_float4 dz0 = spu_mul(pwrx, cy);
		const vec_float4 dz1 = spu_nmsub(pwry, cx, dz0);
		const vec_float4 dz = spu_madd(two, dz1, ltsz);

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

		// Add parent translation
		const vec_float4 wt0 = spu_add(rt0, pwt0);
		const vec_float4 wt1 = spu_add(rt1, pwt1);
		const vec_float4 wt2 = spu_add(rt2, pwt2);
		const vec_float4 wt3 = spu_add(rt3, pwt3);
		
		// Store translation (stqx)
		vec_float4* outputT0 = (vec_float4*) ((uintptr_t) outputBaseT 
			+ si_to_uint((vec_char16) localOfs0));
		vec_float4* outputT1 = (vec_float4*) ((uintptr_t) outputBaseT 
			+ si_to_uint((vec_char16) localOfs1));
		vec_float4* outputT2 = (vec_float4*) ((uintptr_t) outputBaseT 
			+ si_to_uint((vec_char16) localOfs2));
		vec_float4* outputT3 = (vec_float4*) ((uintptr_t) outputBaseT 
			+ si_to_uint((vec_char16) localOfs3));
		(*outputT0) = wt0;
		(*outputT1) = wt1;
		(*outputT2) = wt2;
		(*outputT3) = wt3;

		linkageInput++;
	}
}
