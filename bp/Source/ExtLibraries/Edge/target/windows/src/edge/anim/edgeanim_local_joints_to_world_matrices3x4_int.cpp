/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/anim/edgeanim_structs_ps3.h"
#include "edge/anim/edgeanim_windows.h"

extern "C"
void edgeAnimLocalJointsToWorldMatrices3x4(void* outputMatrices,
										   const EdgeAnimJointTransform* inputJoints,
										   const EdgeAnimJointTransform* rootJoint,
										   const uint16_t* jointLinkage, 
										   unsigned int count)
{
	EDGE_ASSERT( inputJoints != outputMatrices ); // Joint Aliasing Not Permitted
	EDGE_ASSERT(EDGE_IS_ALIGNED(outputMatrices, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(inputJoints, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(rootJoint, 16U));   
	EDGE_ASSERT(EDGE_IS_ALIGNED(jointLinkage, 16U));
	EDGE_ASSERT(count);

	// Constants: Misc
	const __m128 k0_5f = _mm_set1_ps( 0.5f );
	const __m128 k1_0f = _mm_set1_ps( 1.0f );

	// Base pointers
	const __m128* __restrict inputBase = (( const __m128* __restrict ) inputJoints);
	__m128* __restrict outputBase = (( __m128* __restrict ) outputMatrices);

	// Compute Root Matrix (Splatted across all 4 components)
	const __m128* rootJointBase = (( const __m128* ) rootJoint);
	const __m128 rr = rootJointBase[0];
	const __m128 rt = rootJointBase[1];
	const __m128 rs = rootJointBase[2];
	const __m128 rrx = _mm_shuffle_ps( rr, rr, _MM_SHUFFLE( 0, 0, 0, 0 ) );
	const __m128 rry = _mm_shuffle_ps( rr, rr, _MM_SHUFFLE( 1, 1, 1, 1 ) );
	const __m128 rrz = _mm_shuffle_ps( rr, rr, _MM_SHUFFLE( 2, 2, 2, 2 ) );
	const __m128 rrw = _mm_shuffle_ps( rr, rr, _MM_SHUFFLE( 3, 3, 3, 3 ) );
	const __m128 rtx = _mm_shuffle_ps( rt, rt, _MM_SHUFFLE( 0, 0, 0, 0 ) );
	const __m128 rty = _mm_shuffle_ps( rt, rt, _MM_SHUFFLE( 1, 1, 1, 1 ) );
	const __m128 rtz = _mm_shuffle_ps( rt, rt, _MM_SHUFFLE( 2, 2, 2, 2 ) );
	const __m128 rsx = _mm_shuffle_ps( rs, rs, _MM_SHUFFLE( 0, 0, 0, 0 ) );
	const __m128 rsy = _mm_shuffle_ps( rs, rs, _MM_SHUFFLE( 1, 1, 1, 1 ) );
	const __m128 rsz = _mm_shuffle_ps( rs, rs, _MM_SHUFFLE( 2, 2, 2, 2 ) );

	__m128 root_m00, root_m10, root_m20;
	__m128 root_m01, root_m11, root_m21;
	__m128 root_m02, root_m12, root_m22;
	{
		const __m128 rsx2 = _mm_add_ps(rsx, rsx);
		const __m128 rsy2 = _mm_add_ps(rsy, rsy);
		const __m128 rsz2 = _mm_add_ps(rsz, rsz);
		const __m128 mrxx = _mm_nmsub_ps(rrx, rrx, k0_5f);
		const __m128 mryy = _mm_nmsub_ps(rry, rry, k0_5f);
		const __m128 rxy = _mm_mul_ps(rrx, rry);
		const __m128 rxz = _mm_mul_ps(rrx, rrz);
		const __m128 ryz = _mm_mul_ps(rry, rrz);
		root_m00 = _mm_nmsub_ps( rrz, rrz, mryy );
		root_m01 = _mm_madd_ps( rrw, rrz, rxy );
		root_m02 = _mm_nmsub_ps( rrw, rry, rxz );
		root_m10 = _mm_nmsub_ps( rrw, rrz, rxy );
		root_m11 = _mm_nmsub_ps( rrz, rrz, mrxx );
		root_m12 = _mm_madd_ps( rrw, rrx, ryz );
		root_m20 = _mm_madd_ps( rrw, rry, rxz );
		root_m21 = _mm_nmsub_ps( rrw, rrx, ryz );
		root_m22 = _mm_nmsub_ps( rry, rry, mrxx );
		root_m00 = _mm_mul_ps(root_m00, rsx2);
		root_m01 = _mm_mul_ps(root_m01, rsx2);
		root_m02 = _mm_mul_ps(root_m02, rsx2);
		root_m10 = _mm_mul_ps(root_m10, rsy2);
		root_m11 = _mm_mul_ps(root_m11, rsy2);
		root_m12 = _mm_mul_ps(root_m12, rsy2);
		root_m20 = _mm_mul_ps(root_m20, rsz2);
		root_m21 = _mm_mul_ps(root_m21, rsz2);
		root_m22 = _mm_mul_ps(root_m22, rsz2);
	}

	for( unsigned int count4 = (count + 3) >> 2; count4; count4--, jointLinkage+=8 )
	{
		__m128i jointLink = _mm_loadu_si128( (const __m128i*) jointLinkage );

		// Joint offsets
		__m128i jointIdx = jointLink;
		jointIdx = _mm_add_epi16( jointIdx, _mm_slli_epi16( jointIdx, 1 ) );

		// Parent offsets
		__m128i parentIdx = _mm_srli_epi16( _mm_slli_epi16( jointLink, 1 ), 1 );
		__m128i parentUseRootI = _mm_srai_epi32( _mm_slli_epi16( jointLink, 1 ), 31 );
		__m128 parentUseRoot = *(__m128*)&parentUseRootI;
		parentIdx = _mm_andnot_si128( parentUseRootI, parentIdx );
		parentIdx = _mm_add_epi16( parentIdx, _mm_slli_epi16( parentIdx, 1 ) );

		// Scale compensation
		__m128i compensateMaskI = _mm_srai_epi32( jointLink, 31 );
		__m128 compensateMask = *(__m128*)&compensateMaskI;

		// Load + Swizzle Local Joint Transform Components
		__m128 lrx = inputBase[ _mm_extract_epi16( jointIdx, 0 ) ];
		__m128 lry = inputBase[ _mm_extract_epi16( jointIdx, 2 ) ];
		__m128 lrz = inputBase[ _mm_extract_epi16( jointIdx, 4 ) ];
		__m128 lrw = inputBase[ _mm_extract_epi16( jointIdx, 6 ) ];
		_MM_TRANSPOSE4_PS( lrx, lry, lrz, lrw );

		__m128 ltx = inputBase[ _mm_extract_epi16( jointIdx, 0 ) + 1 ];
		__m128 lty = inputBase[ _mm_extract_epi16( jointIdx, 2 ) + 1 ];
		__m128 ltz = inputBase[ _mm_extract_epi16( jointIdx, 4 ) + 1 ];
		__m128 ltw = inputBase[ _mm_extract_epi16( jointIdx, 6 ) + 1 ];
		_MM_TRANSPOSE4_PS( ltx, lty, ltz, ltw );

		__m128 lsx = inputBase[ _mm_extract_epi16( jointIdx, 0 ) + 2 ];
		__m128 lsy = inputBase[ _mm_extract_epi16( jointIdx, 2 ) + 2 ];
		__m128 lsz = inputBase[ _mm_extract_epi16( jointIdx, 4 ) + 2 ];
		__m128 lsw = inputBase[ _mm_extract_epi16( jointIdx, 6 ) + 2 ];
		_MM_TRANSPOSE4_PS( lsx, lsy, lsz, lsw );

		__m128 lsx2 = _mm_add_ps( lsx, lsx );
		__m128 lsy2 = _mm_add_ps( lsy, lsy );
		__m128 lsz2 = _mm_add_ps( lsz, lsz );

		// Load + Swizzle Local Parent Scale
		__m128 lpsx = inputBase[ _mm_extract_epi16( parentIdx, 1 ) + 2 ];
		__m128 lpsy = inputBase[ _mm_extract_epi16( parentIdx, 3 ) + 2 ];
		__m128 lpsz = inputBase[ _mm_extract_epi16( parentIdx, 5 ) + 2 ];
		__m128 lpsw = inputBase[ _mm_extract_epi16( parentIdx, 7 ) + 2 ];
		_MM_TRANSPOSE4_PS( lpsx, lpsy, lpsz, lpsw );

		lpsx = _mm_sel_ps( lpsx, rsx, parentUseRoot );
		lpsy = _mm_sel_ps( lpsy, rsy, parentUseRoot );
		lpsz = _mm_sel_ps( lpsz, rsz, parentUseRoot );

		// Invert Parent Scale
		__m128 lpisx = _mm_rcp_ps( lpsx );
		__m128 lpisy = _mm_rcp_ps( lpsy );
		__m128 lpisz = _mm_rcp_ps( lpsz );
		lpisx = _mm_madd_ps( _mm_nmsub_ps( lpsx, lpisx, k1_0f ), lpisx, lpisx );
		lpisy = _mm_madd_ps( _mm_nmsub_ps( lpsy, lpisy, k1_0f ), lpisy, lpisy );
		lpisz = _mm_madd_ps( _mm_nmsub_ps( lpsz, lpisz, k1_0f ), lpisz, lpisz );

		// Parent Scale Compensation
		__m128 lscx = _mm_sel_ps( k1_0f, lpisx, compensateMask );
		__m128 lscy = _mm_sel_ps( k1_0f, lpisy, compensateMask );
		__m128 lscz = _mm_sel_ps( k1_0f, lpisz, compensateMask );

		// Compute World Matrix - Start with Parent World Matrix
		__m128 world0Col0 = outputBase[ _mm_extract_epi16( parentIdx, 1 ) ];
		__m128 world1Col0 = outputBase[ _mm_extract_epi16( parentIdx, 3 ) ];
		__m128 world2Col0 = outputBase[ _mm_extract_epi16( parentIdx, 5 ) ];
		__m128 world3Col0 = outputBase[ _mm_extract_epi16( parentIdx, 7 ) ];
		__m128 world0Col1 = outputBase[ _mm_extract_epi16( parentIdx, 1 ) + 1 ];
		__m128 world1Col1 = outputBase[ _mm_extract_epi16( parentIdx, 3 ) + 1 ];
		__m128 world2Col1 = outputBase[ _mm_extract_epi16( parentIdx, 5 ) + 1 ];
		__m128 world3Col1 = outputBase[ _mm_extract_epi16( parentIdx, 7 ) + 1 ];
		__m128 world0Col2 = outputBase[ _mm_extract_epi16( parentIdx, 1 ) + 2 ];
		__m128 world1Col2 = outputBase[ _mm_extract_epi16( parentIdx, 3 ) + 2 ];
		__m128 world2Col2 = outputBase[ _mm_extract_epi16( parentIdx, 5 ) + 2 ];
		__m128 world3Col2 = outputBase[ _mm_extract_epi16( parentIdx, 7 ) + 2 ];

		// Swizzle World Matrices
		__m128 world_m00 = world0Col0, world_m10 = world1Col0, world_m20 = world2Col0, world_tx = world3Col0;
		__m128 world_m01 = world0Col1, world_m11 = world1Col1, world_m21 = world2Col1, world_ty = world3Col1;
		__m128 world_m02 = world0Col2, world_m12 = world1Col2, world_m22 = world2Col2, world_tz = world3Col2;
		_MM_TRANSPOSE4_PS( world_m00, world_m10, world_m20, world_tx );
		_MM_TRANSPOSE4_PS( world_m01, world_m11, world_m21, world_ty );
		_MM_TRANSPOSE4_PS( world_m02, world_m12, world_m22, world_tz );

		// Parent or Root?
		world_m00 = _mm_sel_ps( world_m00, root_m00, parentUseRoot );
		world_m01 = _mm_sel_ps( world_m01, root_m01, parentUseRoot );
		world_m02 = _mm_sel_ps( world_m02, root_m02, parentUseRoot );
		world_m10 = _mm_sel_ps( world_m10, root_m10, parentUseRoot );
		world_m11 = _mm_sel_ps( world_m11, root_m11, parentUseRoot );
		world_m12 = _mm_sel_ps( world_m12, root_m12, parentUseRoot );
		world_m20 = _mm_sel_ps( world_m20, root_m20, parentUseRoot );
		world_m21 = _mm_sel_ps( world_m21, root_m21, parentUseRoot );
		world_m22 = _mm_sel_ps( world_m22, root_m22, parentUseRoot );
		world_tx = _mm_sel_ps( world_tx, rtx, parentUseRoot );
		world_ty = _mm_sel_ps( world_ty, rty, parentUseRoot );
		world_tz = _mm_sel_ps( world_tz, rtz, parentUseRoot );

		// Apply localTranslate
		world_tx = _mm_madd_ps( ltx, world_m00, world_tx );
		world_tx = _mm_madd_ps( lty, world_m10, world_tx );
		world_tx = _mm_madd_ps( ltz, world_m20, world_tx );
		world_ty = _mm_madd_ps( ltx, world_m01, world_ty );
		world_ty = _mm_madd_ps( lty, world_m11, world_ty );
		world_ty = _mm_madd_ps( ltz, world_m21, world_ty );
		world_tz = _mm_madd_ps( ltx, world_m02, world_tz );
		world_tz = _mm_madd_ps( lty, world_m12, world_tz );
		world_tz = _mm_madd_ps( ltz, world_m22, world_tz );

		// Apply localCompensateScale
		world_m00 = _mm_mul_ps( world_m00, lscx );
		world_m01 = _mm_mul_ps( world_m01, lscx );
		world_m02 = _mm_mul_ps( world_m02, lscx );
		world_m10 = _mm_mul_ps( world_m10, lscy );
		world_m11 = _mm_mul_ps( world_m11, lscy );
		world_m12 = _mm_mul_ps( world_m12, lscy );
		world_m20 = _mm_mul_ps( world_m20, lscz );
		world_m21 = _mm_mul_ps( world_m21, lscz );
		world_m22 = _mm_mul_ps( world_m22, lscz );

		// Determine localRotationScale
		__m128 lcrs_m00, lcrs_m01, lcrs_m02;
		__m128 lcrs_m10, lcrs_m11, lcrs_m12;
		__m128 lcrs_m20, lcrs_m21, lcrs_m22;
		{
			__m128 mlrxx = _mm_nmsub_ps( lrx, lrx, k0_5f );
			__m128 mlryy = _mm_nmsub_ps( lry, lry, k0_5f );
			__m128 lrxy = _mm_mul_ps( lrx, lry );
			__m128 lrxz = _mm_mul_ps( lrx, lrz );
			__m128 lryz = _mm_mul_ps( lry, lrz );

			lcrs_m00 = _mm_nmsub_ps( lrz, lrz, mlryy );
			lcrs_m01 = _mm_madd_ps( lrw, lrz, lrxy );
			lcrs_m02 = _mm_nmsub_ps( lrw, lry, lrxz );
			lcrs_m10 = _mm_nmsub_ps( lrw, lrz, lrxy );
			lcrs_m11 = _mm_nmsub_ps( lrz, lrz, mlrxx );
			lcrs_m12 = _mm_madd_ps( lrw, lrx, lryz );
			lcrs_m20 = _mm_madd_ps( lrw, lry, lrxz );
			lcrs_m21 = _mm_nmsub_ps( lrw, lrx, lryz  );
			lcrs_m22 = _mm_nmsub_ps( lry, lry, mlrxx );

			lcrs_m00 = _mm_mul_ps( lsx2, lcrs_m00 );
			lcrs_m01 = _mm_mul_ps( lsx2, lcrs_m01 );
			lcrs_m02 = _mm_mul_ps( lsx2, lcrs_m02 );
			lcrs_m10 = _mm_mul_ps( lsy2, lcrs_m10 );
			lcrs_m11 = _mm_mul_ps( lsy2, lcrs_m11 );
			lcrs_m12 = _mm_mul_ps( lsy2, lcrs_m12 );
			lcrs_m20 = _mm_mul_ps( lsz2, lcrs_m20 );
			lcrs_m21 = _mm_mul_ps( lsz2, lcrs_m21 );
			lcrs_m22 = _mm_mul_ps( lsz2, lcrs_m22 );
		}

		// Apply localRotationScale
		__m128 world_m00_t, world_m10_t, world_m20_t;
		__m128 world_m01_t, world_m11_t, world_m21_t;
		__m128 world_m02_t, world_m12_t, world_m22_t;

		world_m00_t = _mm_mul_ps( lcrs_m00, world_m00 );
		world_m01_t = _mm_mul_ps( lcrs_m00, world_m01 );
		world_m02_t = _mm_mul_ps( lcrs_m00, world_m02 );
		world_m00_t = _mm_madd_ps( lcrs_m01, world_m10, world_m00_t );
		world_m01_t = _mm_madd_ps( lcrs_m01, world_m11, world_m01_t );
		world_m02_t = _mm_madd_ps( lcrs_m01, world_m12, world_m02_t );
		world_m00_t = _mm_madd_ps( lcrs_m02, world_m20, world_m00_t );
		world_m01_t = _mm_madd_ps( lcrs_m02, world_m21, world_m01_t );
		world_m02_t = _mm_madd_ps( lcrs_m02, world_m22, world_m02_t );

		world_m10_t = _mm_mul_ps( lcrs_m10, world_m00 );
		world_m11_t = _mm_mul_ps( lcrs_m10, world_m01 );
		world_m12_t = _mm_mul_ps( lcrs_m10, world_m02 );
		world_m10_t = _mm_madd_ps( lcrs_m11, world_m10, world_m10_t );
		world_m11_t = _mm_madd_ps( lcrs_m11, world_m11, world_m11_t );
		world_m12_t = _mm_madd_ps( lcrs_m11, world_m12, world_m12_t );
		world_m10_t = _mm_madd_ps( lcrs_m12, world_m20, world_m10_t );
		world_m11_t = _mm_madd_ps( lcrs_m12, world_m21, world_m11_t );
		world_m12_t = _mm_madd_ps( lcrs_m12, world_m22, world_m12_t );

		world_m20_t = _mm_mul_ps( lcrs_m20, world_m00 );
		world_m21_t = _mm_mul_ps( lcrs_m20, world_m01 );
		world_m22_t = _mm_mul_ps( lcrs_m20, world_m02 );
		world_m20_t = _mm_madd_ps( lcrs_m21, world_m10, world_m20_t );
		world_m21_t = _mm_madd_ps( lcrs_m21, world_m11, world_m21_t );
		world_m22_t = _mm_madd_ps( lcrs_m21, world_m12, world_m22_t );
		world_m20_t = _mm_madd_ps( lcrs_m22, world_m20, world_m20_t );
		world_m21_t = _mm_madd_ps( lcrs_m22, world_m21, world_m21_t );
		world_m22_t = _mm_madd_ps( lcrs_m22, world_m22, world_m22_t );

		// Deswizzle Matrices
		world0Col0 = world_m00_t; world1Col0 = world_m10_t; world2Col0 = world_m20_t; world3Col0 = world_tx;
		world0Col1 = world_m01_t; world1Col1 = world_m11_t; world2Col1 = world_m21_t; world3Col1 = world_ty;
		world0Col2 = world_m02_t; world1Col2 = world_m12_t; world2Col2 = world_m22_t; world3Col2 = world_tz;
		_MM_TRANSPOSE4_PS( world0Col0, world1Col0, world2Col0, world3Col0 );
		_MM_TRANSPOSE4_PS( world0Col1, world1Col1, world2Col1, world3Col1 );
		_MM_TRANSPOSE4_PS( world0Col2, world1Col2, world2Col2, world3Col2 );

		// Output
		outputBase[ _mm_extract_epi16( jointIdx, 0 ) ] = world0Col0;
		outputBase[ _mm_extract_epi16( jointIdx, 2 ) ] = world1Col0;
		outputBase[ _mm_extract_epi16( jointIdx, 4 ) ] = world2Col0;
		outputBase[ _mm_extract_epi16( jointIdx, 6 ) ] = world3Col0;

		outputBase[_mm_extract_epi16( jointIdx, 0 ) + 1 ] = world0Col1;
		outputBase[_mm_extract_epi16( jointIdx, 2 ) + 1 ] = world1Col1;
		outputBase[_mm_extract_epi16( jointIdx, 4 ) + 1 ] = world2Col1;
		outputBase[_mm_extract_epi16( jointIdx, 6 ) + 1 ] = world3Col1;

		outputBase[ _mm_extract_epi16( jointIdx, 0 ) + 2 ] = world0Col2;
		outputBase[ _mm_extract_epi16( jointIdx, 2 ) + 2 ] = world1Col2;
		outputBase[ _mm_extract_epi16( jointIdx, 4 ) + 2 ] = world2Col2;
		outputBase[ _mm_extract_epi16( jointIdx, 6 ) + 2 ] = world3Col2;
	}
}
