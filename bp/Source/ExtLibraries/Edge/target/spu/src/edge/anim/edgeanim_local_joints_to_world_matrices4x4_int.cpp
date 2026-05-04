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
* @brief   Convert local-space joint transforms (R/T/S) to world-space 4x4 matrices
*          SPU intrinisics implementation.
*
* @code
* localScale = 
*          | Sx    0     0  |
*          | 0     Sy    0  |
*          | 0     0     Sz |
*          | 0     0     0  |
*
* localRotation =
* Matrix = | (0.5 - Rx*Ry - Rz*Rz)   (Rx*Ry + Rw*Rz)         (Rx*Rz - Rw*Ry)       |
*          | (Rx*Ry - Rw*Rz)         (0.5 - Rx*Rx - Rz*Rz)   (Ry*Rz + Rw*Rz)       |
*          | (Rx*Rz + Rw*Ry)         (Ry*Rz - Rw*Rx)         (0.5 - Rx*Rx - Ry*Ry) |
*          | 0                       0                       0                     |
*
* localScaleCompensate =
*          | 1/PSx    0        0     |
*          | 0        1/PSy    0     |
*          | 0        0        1/PSz |
*          | 0        0        0     |
*
* localTranslate = 
*          | 1     0     0  |
*          | 0     1     0  |
*          | Tx    Ty    Tz |
*
* result = parent * localTranslate * localScaleCompensate * localRotation * localScale
* if no parent scale compensation - ignore (equal the Identity)
* @endcode
*
* Parent scale compensation is enabled if the high bit of the parent index is set
*
* @note    A reference C++ implementation of this function is available in
*          target/reference/src/edge/anim/edgeanim_local_joints_to_world_matrices4x4.cpp
*
* @note    This function is not called unless you change the makefile which 
*          uses asm implementation by default.
*
* @param   outputMatrices  Pointer to output matrix array.
*                          Must be aligned to 16 bytes. 
*                          Due to the SIMD nature of the processing, the 
*                          outputMatrices array must be a multiple of 4 entries 
*                          in size. 
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

#define SHUFFLE_PACK3(A,B,C,D,X,Y,Z)          \
do {                                          \
	vec_float4 tmp0, tmp1, tmp2, tmp3;        \
	tmp0 = spu_shuffle( (A), (B), shufAaBb ); \
	tmp1 = spu_shuffle( (A), (B), shufCcDd ); \
	tmp2 = spu_shuffle( (C), (D), shufBbAa ); \
	tmp3 = spu_shuffle( (C), (D), shufDdCc ); \
	(X) = spu_sel( tmp0, tmp2, mask_00FF );   \
	(Y) = spu_shuffle( tmp0, tmp2, shufCDab );\
	(Z) = spu_sel( tmp1, tmp3, mask_00FF );   \
} while(0)

#define SHUFFLE_PACK4(A,B,C,D,X,Y,Z,W)        \
do {                                          \
	vec_float4 tmp0, tmp1, tmp2, tmp3;        \
	tmp0 = spu_shuffle( (A), (B), shufAaBb ); \
	tmp1 = spu_shuffle( (A), (B), shufCcDd ); \
	tmp2 = spu_shuffle( (C), (D), shufBbAa ); \
	tmp3 = spu_shuffle( (C), (D), shufDdCc ); \
	(X) = spu_sel( tmp0, tmp2, mask_00FF );   \
	(Y) = spu_shuffle( tmp0, tmp2, shufCDab );\
	(Z) = spu_sel( tmp1, tmp3, mask_00FF );   \
	(W) = spu_shuffle( tmp1, tmp3, shufCDab );\
} while(0)

extern "C"
void edgeAnimLocalJointsToWorldMatrices4x4(void* outputMatrices,
										   const EdgeAnimJointTransform* inputJoints,
										   const EdgeAnimJointTransform* rootJoint,
										   const unsigned short* jointLinkage, 
										   unsigned int count)
{
	EDGE_BOOKMARK(EDGE_BOOKMARK_ANIM_LOCAL_JOINTS_TO_WORLD_MATRICES_4X4);

	EDGE_ASSERT(EDGE_IS_ALIGNED(outputMatrices, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(inputJoints, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(rootJoint, 16U));   
	EDGE_ASSERT(EDGE_IS_ALIGNED(jointLinkage, 16U));
	EDGE_ASSERT(count);

	// Constants: Misc
	const vec_float4 zero = {0.0f, 0.0f, 0.0f, 0.0f};
	const vec_float4 half = {0.5f, 0.5f, 0.5f, 0.5f};
	const vec_float4 one  = {1.0f, 1.0f, 1.0f, 1.0f};
	const vec_uint4 negParent = {0x00003FFF, 0x00003FFF, 0x00003FFF, 0x00003FFF};   
	const vec_uint4 idxMask = {0x00007FFF, 0x00007FFF, 0x00007FFF, 0x00007FFF};
	const vec_uint4 mask_00FF = (vec_uint4)spu_maskb(0x00FF);

	// Constants: Shuffle masks
	const vec_uchar16 shufAAAA = {0x00,0x01,0x02,0x03,0x00,0x01,0x02,0x03,0x00,0x01,0x02,0x03,0x00,0x01,0x02,0x03};
	const vec_uchar16 shufBBBB = {0x04,0x05,0x06,0x07,0x04,0x05,0x06,0x07,0x04,0x05,0x06,0x07,0x04,0x05,0x06,0x07};
	const vec_uchar16 shufCCCC = {0x08,0x09,0x0A,0x0B,0x08,0x09,0x0A,0x0B,0x08,0x09,0x0A,0x0B,0x08,0x09,0x0A,0x0B};
	const vec_uchar16 shufDDDD = {0x0C,0x0D,0x0E,0x0F,0x0C,0x0D,0x0E,0x0F,0x0C,0x0D,0x0E,0x0F,0x0C,0x0D,0x0E,0x0F};
	const vec_uchar16 shufAaBb = {0x00,0x01,0x02,0x03,0x10,0x11,0x12,0x13,0x04,0x05,0x06,0x07,0x14,0x15,0x16,0x17};
	const vec_uchar16 shufBbAa = {0x04,0x05,0x06,0x07,0x14,0x15,0x16,0x17,0x00,0x01,0x02,0x03,0x10,0x11,0x12,0x13};
	const vec_uchar16 shufCcDd = {0x08,0x09,0x0A,0x0B,0x18,0x19,0x1A,0x1B,0x0C,0x0D,0x0E,0x0F,0x1C,0x1D,0x1E,0x1F};
	const vec_uchar16 shufCDab = {0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17};
	const vec_uchar16 shufDdCc = {0x0C,0x0D,0x0E,0x0F,0x1C,0x1D,0x1E,0x1F,0x08,0x09,0x0A,0x0B,0x18,0x19,0x1A,0x1B};

	// Base pointers
	const vec_float4* __restrict__ inputBaseR = (( const vec_float4* ) inputJoints) + 0;
	const vec_float4* __restrict__ inputBaseT = (( const vec_float4* ) inputJoints) + 1;
	const vec_float4* __restrict__ inputBaseS = (( const vec_float4* ) inputJoints) + 2;
	const vec_uint4*  __restrict__ linkageInput = ( const vec_uint4* ) jointLinkage;
	vec_float4* __restrict__ outputBaseX = (( vec_float4* ) outputMatrices) + 0;
	vec_float4* __restrict__ outputBaseY = (( vec_float4* ) outputMatrices) + 1;
	vec_float4* __restrict__ outputBaseZ = (( vec_float4* ) outputMatrices) + 2;
	vec_float4* __restrict__ outputBaseW = (( vec_float4* ) outputMatrices) + 3;

	// Compute Root Matrix (Splatted across all 4 components)
	const vec_float4 rr = ((const vec_float4*) rootJoint)[0];
	const vec_float4 rt = ((const vec_float4*) rootJoint)[1];
	const vec_float4 rs = ((const vec_float4*) rootJoint)[2];

	const vec_float4 rrx = spu_shuffle( rr, rr, shufAAAA );
	const vec_float4 rry = spu_shuffle( rr, rr, shufBBBB );
	const vec_float4 rrz = spu_shuffle( rr, rr, shufCCCC );
	const vec_float4 rrw = spu_shuffle( rr, rr, shufDDDD );
	const vec_float4 rtx = spu_shuffle( rt, rt, shufAAAA );
	const vec_float4 rty = spu_shuffle( rt, rt, shufBBBB );
	const vec_float4 rtz = spu_shuffle( rt, rt, shufCCCC );
	const vec_float4 rsx = spu_shuffle( rs, rs, shufAAAA );
	const vec_float4 rsy = spu_shuffle( rs, rs, shufBBBB );
	const vec_float4 rsz = spu_shuffle( rs, rs, shufCCCC );

	vec_float4 root_m00, root_m10, root_m20;
	vec_float4 root_m01, root_m11, root_m21;
	vec_float4 root_m02, root_m12, root_m22;
	{
		vec_float4 rsx2 = spu_add(rsx, rsx);
		vec_float4 rsy2 = spu_add(rsy, rsy);
		vec_float4 rsz2 = spu_add(rsz, rsz);
		vec_float4 mrxx = spu_nmsub(rrx, rrx, half);
		vec_float4 mryy = spu_nmsub(rry, rry, half);
		vec_float4 rxy = spu_mul(rrx, rry);
		vec_float4 rxz = spu_mul(rrx, rrz);
		vec_float4 ryz = spu_mul(rry, rrz);
		root_m00 = spu_nmsub(rrz, rrz, mryy);
		root_m01 = spu_madd(rrw, rrz, rxy);
		root_m02 = spu_nmsub(rrw, rry, rxz);
		root_m10 = spu_nmsub(rrw, rrz, rxy);
		root_m11 = spu_nmsub(rrz, rrz, mrxx);
		root_m12 = spu_madd(rrw, rrx, ryz);
		root_m20 = spu_madd(rrw, rry, rxz);
		root_m21 = spu_nmsub(rrw, rrx, ryz);
		root_m22 = spu_nmsub(rry, rry, mrxx);
		root_m00 = spu_mul(root_m00, rsx2);
		root_m01 = spu_mul(root_m01, rsx2);
		root_m02 = spu_mul(root_m02, rsx2);
		root_m10 = spu_mul(root_m10, rsy2);
		root_m11 = spu_mul(root_m11, rsy2);
		root_m12 = spu_mul(root_m12, rsy2);
		root_m20 = spu_mul(root_m20, rsz2);
		root_m21 = spu_mul(root_m21, rsz2);
		root_m22 = spu_mul(root_m22, rsz2);
	}

	int count4 = (count + 3) >> 2;

	for(; count4; count4--, linkageInput++) {

		const vec_uint4 indices = linkageInput[0];

		// Joint offsets
		const vec_uint4 jointIdx = spu_rlmask(indices, -16);
		const vec_uint4 jointInOfs = spu_add(spu_sl(jointIdx, 4), spu_sl(jointIdx, 5));
		const vec_uint4 jointInOfs0 = jointInOfs;
		const vec_uint4 jointInOfs1 = spu_slqwbyte(jointInOfs, 4);
		const vec_uint4 jointInOfs2 = spu_slqwbyte(jointInOfs, 8);
		const vec_uint4 jointInOfs3 = spu_slqwbyte(jointInOfs, 12);
		const vec_uint4 jointOutOfs = spu_sl(jointIdx, 6);
		const vec_uint4 jointOutOfs0 = jointOutOfs;
		const vec_uint4 jointOutOfs1 = spu_slqwbyte(jointOutOfs, 4);
		const vec_uint4 jointOutOfs2 = spu_slqwbyte(jointOutOfs, 8);
		const vec_uint4 jointOutOfs3 = spu_slqwbyte(jointOutOfs, 12);

		// Parent offsets (if use root - set to 0 - will be ignored anyhow)
		const vec_uint4 parentIdx = spu_and(indices, idxMask);
		const vec_uint4 parentUseRoot = spu_cmpgt(parentIdx, negParent);
		const vec_uint4 parentJointOfs = spu_sel(spu_add(spu_sl(parentIdx, 4), spu_sl(parentIdx, 5)),(vec_uint4)zero,parentUseRoot);
		const vec_uint4 parentJointOfs0 = parentJointOfs;
		const vec_uint4 parentJointOfs1 = spu_slqwbyte(parentJointOfs, 4);
		const vec_uint4 parentJointOfs2 = spu_slqwbyte(parentJointOfs, 8);
		const vec_uint4 parentJointOfs3 = spu_slqwbyte(parentJointOfs, 12);
		const vec_uint4 parentMatOfs = spu_sel(spu_sl(parentIdx, 6),(vec_uint4)zero,parentUseRoot);
		const vec_uint4 parentMatOfs0 = parentMatOfs;
		const vec_uint4 parentMatOfs1 = spu_slqwbyte(parentMatOfs, 4);
		const vec_uint4 parentMatOfs2 = spu_slqwbyte(parentMatOfs, 8);
		const vec_uint4 parentMatOfs3 = spu_slqwbyte(parentMatOfs, 12);

		// Local Joint Transform Components (lqx)
		const vec_float4 lr0 = *((const vec_float4*) ((uintptr_t) inputBaseR + si_to_uint((vec_char16) jointInOfs0)));
		const vec_float4 lr1 = *((const vec_float4*) ((uintptr_t) inputBaseR + si_to_uint((vec_char16) jointInOfs1)));
		const vec_float4 lr2 = *((const vec_float4*) ((uintptr_t) inputBaseR + si_to_uint((vec_char16) jointInOfs2)));
		const vec_float4 lr3 = *((const vec_float4*) ((uintptr_t) inputBaseR + si_to_uint((vec_char16) jointInOfs3)));
		const vec_float4 lt0 = *((const vec_float4*) ((uintptr_t) inputBaseT + si_to_uint((vec_char16) jointInOfs0)));
		const vec_float4 lt1 = *((const vec_float4*) ((uintptr_t) inputBaseT + si_to_uint((vec_char16) jointInOfs1)));
		const vec_float4 lt2 = *((const vec_float4*) ((uintptr_t) inputBaseT + si_to_uint((vec_char16) jointInOfs2)));
		const vec_float4 lt3 = *((const vec_float4*) ((uintptr_t) inputBaseT + si_to_uint((vec_char16) jointInOfs3)));
		const vec_float4 ls0 = *((const vec_float4*) ((uintptr_t) inputBaseS + si_to_uint((vec_char16) jointInOfs0)));
		const vec_float4 ls1 = *((const vec_float4*) ((uintptr_t) inputBaseS + si_to_uint((vec_char16) jointInOfs1)));
		const vec_float4 ls2 = *((const vec_float4*) ((uintptr_t) inputBaseS + si_to_uint((vec_char16) jointInOfs2)));
		const vec_float4 ls3 = *((const vec_float4*) ((uintptr_t) inputBaseS + si_to_uint((vec_char16) jointInOfs3)));

		// Local Parent Scale (lqx)
		const vec_float4 lps0 = *((const vec_float4*) ((uintptr_t) inputBaseS + si_to_uint((vec_char16) parentJointOfs0)));
		const vec_float4 lps1 = *((const vec_float4*) ((uintptr_t) inputBaseS + si_to_uint((vec_char16) parentJointOfs1)));
		const vec_float4 lps2 = *((const vec_float4*) ((uintptr_t) inputBaseS + si_to_uint((vec_char16) parentJointOfs2)));
		const vec_float4 lps3 = *((const vec_float4*) ((uintptr_t) inputBaseS + si_to_uint((vec_char16) parentJointOfs3)));

		// Swizzle Joint Transform Components
		vec_float4 lrx, lry, lrz, lrw;
		SHUFFLE_PACK4( lr0, lr1, lr2, lr3, lrx, lry, lrz, lrw );

		vec_float4 ltx, lty, ltz;
		SHUFFLE_PACK3( lt0, lt1, lt2, lt3, ltx, lty, ltz );

		vec_float4 lsx, lsy, lsz;
		SHUFFLE_PACK3( ls0, ls1, ls2, ls3, lsx, lsy, lsz );
		const vec_float4 lsx2 = spu_add( lsx, lsx );
		const vec_float4 lsy2 = spu_add( lsy, lsy );
		const vec_float4 lsz2 = spu_add( lsz, lsz );

		vec_float4 lpsx, lpsy, lpsz;
		SHUFFLE_PACK3( lps0, lps1, lps2, lps3, lpsx, lpsy, lpsz );
		lpsx = spu_sel( lpsx, rsx, parentUseRoot );
		lpsy = spu_sel( lpsy, rsy, parentUseRoot );
		lpsz = spu_sel( lpsz, rsz, parentUseRoot );

		// Invert Parent Scale
		const vec_float4 lpisx = spu_re( lpsx );
		const vec_float4 lpisy = spu_re( lpsy );
		const vec_float4 lpisz = spu_re( lpsz );

		// Parent Scale Compensation
		const vec_uint4 compensateBit = spu_sl( indices, 16 );
		const vec_uint4 compensateMask = spu_rlmaska( compensateBit, -31 );
		const vec_float4 lscx = spu_sel( one, lpisx, compensateMask );
		const vec_float4 lscy = spu_sel( one, lpisy, compensateMask );
		const vec_float4 lscz = spu_sel( one, lpisz, compensateMask );

		// Compute World Matrix - Start with Parent World Matrix
		vec_float4 world0Row0 = *((vec_float4*) ((uintptr_t) outputBaseX + si_to_uint((vec_char16) parentMatOfs0)));
		vec_float4 world1Row0 = *((vec_float4*) ((uintptr_t) outputBaseX + si_to_uint((vec_char16) parentMatOfs1)));
		vec_float4 world2Row0 = *((vec_float4*) ((uintptr_t) outputBaseX + si_to_uint((vec_char16) parentMatOfs2)));
		vec_float4 world3Row0 = *((vec_float4*) ((uintptr_t) outputBaseX + si_to_uint((vec_char16) parentMatOfs3)));
		vec_float4 world0Row1 = *((vec_float4*) ((uintptr_t) outputBaseY + si_to_uint((vec_char16) parentMatOfs0)));
		vec_float4 world1Row1 = *((vec_float4*) ((uintptr_t) outputBaseY + si_to_uint((vec_char16) parentMatOfs1)));
		vec_float4 world2Row1 = *((vec_float4*) ((uintptr_t) outputBaseY + si_to_uint((vec_char16) parentMatOfs2)));
		vec_float4 world3Row1 = *((vec_float4*) ((uintptr_t) outputBaseY + si_to_uint((vec_char16) parentMatOfs3)));
		vec_float4 world0Row2 = *((vec_float4*) ((uintptr_t) outputBaseZ + si_to_uint((vec_char16) parentMatOfs0)));
		vec_float4 world1Row2 = *((vec_float4*) ((uintptr_t) outputBaseZ + si_to_uint((vec_char16) parentMatOfs1)));
		vec_float4 world2Row2 = *((vec_float4*) ((uintptr_t) outputBaseZ + si_to_uint((vec_char16) parentMatOfs2)));
		vec_float4 world3Row2 = *((vec_float4*) ((uintptr_t) outputBaseZ + si_to_uint((vec_char16) parentMatOfs3)));
		vec_float4 world0Row3 = *((vec_float4*) ((uintptr_t) outputBaseW + si_to_uint((vec_char16) parentMatOfs0)));
		vec_float4 world1Row3 = *((vec_float4*) ((uintptr_t) outputBaseW + si_to_uint((vec_char16) parentMatOfs1)));
		vec_float4 world2Row3 = *((vec_float4*) ((uintptr_t) outputBaseW + si_to_uint((vec_char16) parentMatOfs2)));
		vec_float4 world3Row3 = *((vec_float4*) ((uintptr_t) outputBaseW + si_to_uint((vec_char16) parentMatOfs3)));

		// Swizzle World Matrices
		vec_float4 world_m00, world_m01, world_m02, world_tx;
		vec_float4 world_m10, world_m11, world_m12, world_ty;
		vec_float4 world_m20, world_m21, world_m22, world_tz;
		SHUFFLE_PACK3( world0Row0, world1Row0, world2Row0, world3Row0, world_m00, world_m01, world_m02 );
		SHUFFLE_PACK3( world0Row1, world1Row1, world2Row1, world3Row1, world_m10, world_m11, world_m12 );
		SHUFFLE_PACK3( world0Row2, world1Row2, world2Row2, world3Row2, world_m20, world_m21, world_m22 );
		SHUFFLE_PACK3( world0Row3, world1Row3, world2Row3, world3Row3, world_tx,  world_ty,  world_tz  );

		// Parent or Root?
		world_m00 = spu_sel( world_m00, root_m00, parentUseRoot );
		world_m01 = spu_sel( world_m01, root_m01, parentUseRoot );
		world_m02 = spu_sel( world_m02, root_m02, parentUseRoot );
		world_m10 = spu_sel( world_m10, root_m10, parentUseRoot );
		world_m11 = spu_sel( world_m11, root_m11, parentUseRoot );
		world_m12 = spu_sel( world_m12, root_m12, parentUseRoot );
		world_m20 = spu_sel( world_m20, root_m20, parentUseRoot );
		world_m21 = spu_sel( world_m21, root_m21, parentUseRoot );
		world_m22 = spu_sel( world_m22, root_m22, parentUseRoot );
		world_tx = spu_sel( world_tx, rtx, parentUseRoot );
		world_ty = spu_sel( world_ty, rty, parentUseRoot );
		world_tz = spu_sel( world_tz, rtz, parentUseRoot );

		// Apply localTranslate
		world_tx = spu_madd( ltx, world_m00, world_tx );
		world_tx = spu_madd( lty, world_m10, world_tx );
		world_tx = spu_madd( ltz, world_m20, world_tx );
		world_ty = spu_madd( ltx, world_m01, world_ty );
		world_ty = spu_madd( lty, world_m11, world_ty );
		world_ty = spu_madd( ltz, world_m21, world_ty );
		world_tz = spu_madd( ltx, world_m02, world_tz );
		world_tz = spu_madd( lty, world_m12, world_tz );
		world_tz = spu_madd( ltz, world_m22, world_tz );

		// Apply localCompensateScale
		world_m00 = spu_mul( world_m00, lscx );
		world_m01 = spu_mul( world_m01, lscx );
		world_m02 = spu_mul( world_m02, lscx );
		world_m10 = spu_mul( world_m10, lscy );
		world_m11 = spu_mul( world_m11, lscy );
		world_m12 = spu_mul( world_m12, lscy );
		world_m20 = spu_mul( world_m20, lscz );
		world_m21 = spu_mul( world_m21, lscz );
		world_m22 = spu_mul( world_m22, lscz );

		// Determine localRotationScale
		vec_float4 lcrs_m00, lcrs_m01, lcrs_m02;
		vec_float4 lcrs_m10, lcrs_m11, lcrs_m12;
		vec_float4 lcrs_m20, lcrs_m21, lcrs_m22;
		{
			vec_float4 mlrxx = spu_nmsub(lrx, lrx, half);
			vec_float4 mlryy = spu_nmsub(lry, lry, half);
			vec_float4 lrxy = spu_mul(lrx, lry);
			vec_float4 lrxz = spu_mul(lrx, lrz);
			vec_float4 lryz = spu_mul(lry, lrz);
			lcrs_m00 = spu_nmsub(lrz, lrz, mlryy);
			lcrs_m01 = spu_madd(lrw, lrz, lrxy);
			lcrs_m02 = spu_nmsub(lrw, lry, lrxz);
			lcrs_m10 = spu_nmsub(lrw, lrz, lrxy);
			lcrs_m11 = spu_nmsub(lrz, lrz, mlrxx);
			lcrs_m12 = spu_madd(lrw, lrx, lryz);
			lcrs_m20 = spu_madd(lrw, lry, lrxz);
			lcrs_m21 = spu_nmsub(lrw, lrx, lryz);
			lcrs_m22 = spu_nmsub(lry, lry, mlrxx);

			lcrs_m00 = spu_mul(lcrs_m00, lsx2);
			lcrs_m01 = spu_mul(lcrs_m01, lsx2);
			lcrs_m02 = spu_mul(lcrs_m02, lsx2);
			lcrs_m10 = spu_mul(lcrs_m10, lsy2);
			lcrs_m11 = spu_mul(lcrs_m11, lsy2);
			lcrs_m12 = spu_mul(lcrs_m12, lsy2);
			lcrs_m20 = spu_mul(lcrs_m20, lsz2);
			lcrs_m21 = spu_mul(lcrs_m21, lsz2);
			lcrs_m22 = spu_mul(lcrs_m22, lsz2);
		}

		// Apply localRotationScale
		vec_float4 world_m00_t, world_m10_t, world_m20_t;
		vec_float4 world_m01_t, world_m11_t, world_m21_t;
		vec_float4 world_m02_t, world_m12_t, world_m22_t;
		world_m00_t =  spu_mul( lcrs_m00, world_m00 );
		world_m00_t = spu_madd( lcrs_m01, world_m10, world_m00_t );
		world_m00_t = spu_madd( lcrs_m02, world_m20, world_m00_t );
		world_m01_t =  spu_mul( lcrs_m00, world_m01 );
		world_m01_t = spu_madd( lcrs_m01, world_m11, world_m01_t );
		world_m01_t = spu_madd( lcrs_m02, world_m21, world_m01_t );
		world_m02_t =  spu_mul( lcrs_m00, world_m02 );
		world_m02_t = spu_madd( lcrs_m01, world_m12, world_m02_t );
		world_m02_t = spu_madd( lcrs_m02, world_m22, world_m02_t );

		world_m10_t =  spu_mul( lcrs_m10, world_m00 );
		world_m10_t = spu_madd( lcrs_m11, world_m10, world_m10_t );
		world_m10_t = spu_madd( lcrs_m12, world_m20, world_m10_t );
		world_m11_t =  spu_mul( lcrs_m10, world_m01 );
		world_m11_t = spu_madd( lcrs_m11, world_m11, world_m11_t );
		world_m11_t = spu_madd( lcrs_m12, world_m21, world_m11_t );
		world_m12_t =  spu_mul( lcrs_m10, world_m02 );
		world_m12_t = spu_madd( lcrs_m11, world_m12, world_m12_t );
		world_m12_t = spu_madd( lcrs_m12, world_m22, world_m12_t );

		world_m20_t =  spu_mul( lcrs_m20, world_m00 );
		world_m20_t = spu_madd( lcrs_m21, world_m10, world_m20_t );
		world_m20_t = spu_madd( lcrs_m22, world_m20, world_m20_t );
		world_m21_t =  spu_mul( lcrs_m20, world_m01 );
		world_m21_t = spu_madd( lcrs_m21, world_m11, world_m21_t );
		world_m21_t = spu_madd( lcrs_m22, world_m21, world_m21_t );
		world_m22_t =  spu_mul( lcrs_m20, world_m02 );
		world_m22_t = spu_madd( lcrs_m21, world_m12, world_m22_t );
		world_m22_t = spu_madd( lcrs_m22, world_m22, world_m22_t );

		// Deswizzle Matrices
		SHUFFLE_PACK4( world_m00_t, world_m01_t, world_m02_t, zero, world0Row0, world1Row0, world2Row0, world3Row0 );
		SHUFFLE_PACK4( world_m10_t, world_m11_t, world_m12_t, zero, world0Row1, world1Row1, world2Row1, world3Row1 );
		SHUFFLE_PACK4( world_m20_t, world_m21_t, world_m22_t, zero, world0Row2, world1Row2, world2Row2, world3Row2 );
		SHUFFLE_PACK4( world_tx,    world_ty,    world_tz,    one,  world0Row3, world1Row3, world2Row3, world3Row3 );

		// Output
		*((vec_float4*) ((uintptr_t) outputBaseX + si_to_uint((vec_char16) jointOutOfs0))) = world0Row0;
		*((vec_float4*) ((uintptr_t) outputBaseX + si_to_uint((vec_char16) jointOutOfs1))) = world1Row0;
		*((vec_float4*) ((uintptr_t) outputBaseX + si_to_uint((vec_char16) jointOutOfs2))) = world2Row0;
		*((vec_float4*) ((uintptr_t) outputBaseX + si_to_uint((vec_char16) jointOutOfs3))) = world3Row0;

		*((vec_float4*) ((uintptr_t) outputBaseY + si_to_uint((vec_char16) jointOutOfs0))) = world0Row1;
		*((vec_float4*) ((uintptr_t) outputBaseY + si_to_uint((vec_char16) jointOutOfs1))) = world1Row1;
		*((vec_float4*) ((uintptr_t) outputBaseY + si_to_uint((vec_char16) jointOutOfs2))) = world2Row1;
		*((vec_float4*) ((uintptr_t) outputBaseY + si_to_uint((vec_char16) jointOutOfs3))) = world3Row1;

		*((vec_float4*) ((uintptr_t) outputBaseZ + si_to_uint((vec_char16) jointOutOfs0))) = world0Row2;
		*((vec_float4*) ((uintptr_t) outputBaseZ + si_to_uint((vec_char16) jointOutOfs1))) = world1Row2;
		*((vec_float4*) ((uintptr_t) outputBaseZ + si_to_uint((vec_char16) jointOutOfs2))) = world2Row2;
		*((vec_float4*) ((uintptr_t) outputBaseZ + si_to_uint((vec_char16) jointOutOfs3))) = world3Row2;

		*((vec_float4*) ((uintptr_t) outputBaseW + si_to_uint((vec_char16) jointOutOfs0))) = world0Row3;
		*((vec_float4*) ((uintptr_t) outputBaseW + si_to_uint((vec_char16) jointOutOfs1))) = world1Row3;
		*((vec_float4*) ((uintptr_t) outputBaseW + si_to_uint((vec_char16) jointOutOfs2))) = world2Row3;
		*((vec_float4*) ((uintptr_t) outputBaseW + si_to_uint((vec_char16) jointOutOfs3))) = world3Row3;
	}
}
