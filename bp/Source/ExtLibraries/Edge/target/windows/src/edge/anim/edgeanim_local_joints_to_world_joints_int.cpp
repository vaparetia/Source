 /* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/anim/edgeanim_structs_ps3.h"
#include "edge/anim/edgeanim_windows.h"

extern "C"
void edgeAnimLocalJointsToWorldJoints(EdgeAnimJointTransform* outputJoints, 
                                      const EdgeAnimJointTransform* inputJoints,
                                      const EdgeAnimJointTransform* rootJoint,
                                      const uint16_t* jointLinkage, 
                                      unsigned int count)
{
	EDGE_ASSERT( inputJoints != outputJoints ); // Joint Aliasing Not Permitted
	EDGE_ASSERT(EDGE_IS_ALIGNED(outputJoints, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(rootJoint, 16U));   
	EDGE_ASSERT(EDGE_IS_ALIGNED(inputJoints, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(jointLinkage, 16U));
	EDGE_ASSERT(count);

	// Constants: Misc
	const __m128 k0_5f = _mm_set1_ps( 0.5f );
	const __m128 k1_0f = _mm_set1_ps( 1.0f );

	// Base pointers
	const __m128* __restrict inputBase = (( const __m128* __restrict ) inputJoints);
	__m128* __restrict outputBase = (( __m128* __restrict ) outputJoints);

	// Root Joint
	const __m128* rootJointBase = (( const __m128* ) rootJoint);

	const __m128 rootJointR = rootJointBase[0];
	const __m128 rootJointRx = _mm_shuffle_ps( rootJointR, rootJointR, _MM_SHUFFLE( 0, 0, 0, 0 ) );
	const __m128 rootJointRy = _mm_shuffle_ps( rootJointR, rootJointR, _MM_SHUFFLE( 1, 1, 1, 1 ) );
	const __m128 rootJointRz = _mm_shuffle_ps( rootJointR, rootJointR, _MM_SHUFFLE( 2, 2, 2, 2 ) );
	const __m128 rootJointRw = _mm_shuffle_ps( rootJointR, rootJointR, _MM_SHUFFLE( 3, 3, 3, 3 ) );

	const __m128 rootJointT = rootJointBase[1];
	const __m128 rootJointTx = _mm_shuffle_ps( rootJointT, rootJointT, _MM_SHUFFLE( 0, 0, 0, 0 ) );
	const __m128 rootJointTy = _mm_shuffle_ps( rootJointT, rootJointT, _MM_SHUFFLE( 1, 1, 1, 1 ) );
	const __m128 rootJointTz = _mm_shuffle_ps( rootJointT, rootJointT, _MM_SHUFFLE( 2, 2, 2, 2 ) );

	const __m128 rootJointS = rootJointBase[2];
	const __m128 rootJointSx = _mm_shuffle_ps( rootJointS, rootJointS, _MM_SHUFFLE( 0, 0, 0, 0 ) );
	const __m128 rootJointSy = _mm_shuffle_ps( rootJointS, rootJointS, _MM_SHUFFLE( 1, 1, 1, 1 ) );
	const __m128 rootJointSz = _mm_shuffle_ps( rootJointS, rootJointS, _MM_SHUFFLE( 2, 2, 2, 2 ) );

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

		// Load + Swizzle Parent World Joint (or Root) Transform Components
		__m128 pwrx = outputBase[ _mm_extract_epi16( parentIdx, 1 ) ];
		__m128 pwry = outputBase[ _mm_extract_epi16( parentIdx, 3 ) ];
		__m128 pwrz = outputBase[ _mm_extract_epi16( parentIdx, 5 ) ];
		__m128 pwrw = outputBase[ _mm_extract_epi16( parentIdx, 7 ) ];
		_MM_TRANSPOSE4_PS( pwrx, pwry, pwrz, pwrw );

		pwrx = _mm_sel_ps( pwrx, rootJointRx, parentUseRoot );
		pwry = _mm_sel_ps( pwry, rootJointRy, parentUseRoot );
		pwrz = _mm_sel_ps( pwrz, rootJointRz, parentUseRoot );
		pwrw = _mm_sel_ps( pwrw, rootJointRw, parentUseRoot );

		__m128 pwtx = outputBase[ _mm_extract_epi16( parentIdx, 1 ) + 1 ];
		__m128 pwty = outputBase[ _mm_extract_epi16( parentIdx, 3 ) + 1 ];
		__m128 pwtz = outputBase[ _mm_extract_epi16( parentIdx, 5 ) + 1 ];
		__m128 pwtw = outputBase[ _mm_extract_epi16( parentIdx, 7 ) + 1 ];
		_MM_TRANSPOSE4_PS( pwtx, pwty, pwtz, pwtw );

		pwtx = _mm_sel_ps( pwtx, rootJointTx, parentUseRoot );
		pwty = _mm_sel_ps( pwty, rootJointTy, parentUseRoot );
		pwtz = _mm_sel_ps( pwtz, rootJointTz, parentUseRoot );

		__m128 pwsx = outputBase[ _mm_extract_epi16( parentIdx, 1 ) + 2 ];
		__m128 pwsy = outputBase[ _mm_extract_epi16( parentIdx, 3 ) + 2 ];
		__m128 pwsz = outputBase[ _mm_extract_epi16( parentIdx, 5 ) + 2 ];
		__m128 pwsw = outputBase[ _mm_extract_epi16( parentIdx, 7 ) + 2 ];
		_MM_TRANSPOSE4_PS( pwsx, pwsy, pwsz, pwsw );

		pwsx = _mm_sel_ps( pwsx, rootJointSx, parentUseRoot );
		pwsy = _mm_sel_ps( pwsy, rootJointSy, parentUseRoot );
		pwsz = _mm_sel_ps( pwsz, rootJointSz, parentUseRoot );

		// Load + Swizzle Parent Local Joint (or Root) Scale Components
		__m128 plsx = inputBase[ _mm_extract_epi16( parentIdx, 1 ) + 2 ];
		__m128 plsy = inputBase[ _mm_extract_epi16( parentIdx, 3 ) + 2 ];
		__m128 plsz = inputBase[ _mm_extract_epi16( parentIdx, 5 ) + 2 ];
		__m128 plsw = inputBase[ _mm_extract_epi16( parentIdx, 7 ) + 2 ];
		_MM_TRANSPOSE4_PS( plsx, plsy, plsz, plsw );

		plsx = _mm_sel_ps( plsx, rootJointSx, parentUseRoot );
		plsy = _mm_sel_ps( plsy, rootJointSy, parentUseRoot );
		plsz = _mm_sel_ps( plsz, rootJointSz, parentUseRoot );

		// World quat = parent world quat * local quat
		__m128 wrx = _mm_nmsub_ps( pwrz, lry, _mm_madd_ps( pwry, lrz, _mm_madd_ps( pwrx, lrw, _mm_mul_ps( pwrw, lrx ) ) ) );
		__m128 wry = _mm_madd_ps( pwrz, lrx, _mm_madd_ps( pwry, lrw, _mm_nmsub_ps( pwrx, lrz, _mm_mul_ps( pwrw, lry ) ) ) );
		__m128 wrz = _mm_madd_ps( pwrz, lrw, _mm_nmsub_ps( pwry, lrx, _mm_madd_ps( pwrx, lry, _mm_mul_ps( pwrw, lrz ) ) ) );
		__m128 wrw = _mm_nmsub_ps( pwrz, lrz, _mm_nmsub_ps( pwry, lry, _mm_nmsub_ps( pwrx, lrx, _mm_mul_ps( pwrw, lrw ) ) ) );

		// Normalise world quats
		__m128 wrsqr = _mm_madd_ps( wrx, wrx, _mm_madd_ps( wry, wry, _mm_madd_ps( wrz, wrz, _mm_mul_ps( wrw, wrw ) ) ) );
		__m128 inv = _mm_rsqrt_ps( wrsqr );
		inv = _mm_madd_ps( _mm_nmsub_ps( wrsqr, _mm_mul_ps( inv, inv ), k1_0f ), _mm_mul_ps( inv, k0_5f ), inv );
		wrx = _mm_mul_ps( wrx, inv );
		wry = _mm_mul_ps( wry, inv );
		wrz = _mm_mul_ps( wrz, inv );
		wrw = _mm_mul_ps( wrw, inv );

		// Deswizzle & store rotation
		__m128 wr0 = wrx, wr1 = wry, wr2 = wrz, wr3 = wrw;
		_MM_TRANSPOSE4_PS( wr0, wr1, wr2, wr3 );

		outputBase[ _mm_extract_epi16( jointIdx, 0 ) ] = wr0;
		outputBase[ _mm_extract_epi16( jointIdx, 2 ) ] = wr1;
		outputBase[ _mm_extract_epi16( jointIdx, 4 ) ] = wr2;
		outputBase[ _mm_extract_epi16( jointIdx, 6 ) ] = wr3;

		// 1 / parent local scale
		__m128 plsix = _mm_rcp_ps( plsx );
		__m128 plsiy = _mm_rcp_ps( plsy );
		__m128 plsiz = _mm_rcp_ps( plsz );
		plsix = _mm_madd_ps( _mm_nmsub_ps( plsx, plsix, k1_0f ), plsix, plsix );
		plsiy = _mm_madd_ps( _mm_nmsub_ps( plsy, plsiy, k1_0f ), plsiy, plsiy );
		plsiz = _mm_madd_ps( _mm_nmsub_ps( plsz, plsiz, k1_0f ), plsiz, plsiz );

		// Scale compensate
		__m128 scalex = _mm_sel_ps( k1_0f, plsix, compensateMask );
		__m128 scaley = _mm_sel_ps( k1_0f, plsiy, compensateMask );
		__m128 scalez = _mm_sel_ps( k1_0f, plsiz, compensateMask );

		// Final scale
		__m128 ws0 = _mm_mul_ps( _mm_mul_ps( lsx, pwsx ), scalex );
		__m128 ws1 = _mm_mul_ps( _mm_mul_ps( lsy, pwsy ), scaley );
		__m128 ws2 = _mm_mul_ps( _mm_mul_ps( lsz, pwsz ), scalez );
		__m128 ws3 = k1_0f;
		_MM_TRANSPOSE4_PS( ws0, ws1, ws2, ws3 );

		// Store scale
		outputBase[ _mm_extract_epi16( jointIdx, 0 ) + 2 ] = ws0;
		outputBase[ _mm_extract_epi16( jointIdx, 2 ) + 2 ] = ws1;
		outputBase[ _mm_extract_epi16( jointIdx, 4 ) + 2 ] = ws2;
		outputBase[ _mm_extract_epi16( jointIdx, 6 ) + 2 ] = ws3;

		// Scale local translation
		ltx = _mm_mul_ps( ltx, pwsx );
		lty = _mm_mul_ps( lty, pwsy );
		ltz = _mm_mul_ps( ltz, pwsz );

		// c = Cross(pr, lt) + prw*lt = 
		// (pry*ltz-prz*lty, prz*ltx-prx*ltz, prx*lty-pry*ltx) + prw*lt
		__m128 cx = _mm_madd_ps( pwrw, ltx, _mm_nmsub_ps( pwrz, lty, _mm_mul_ps( pwry, ltz ) ) );
		__m128 cy = _mm_madd_ps( pwrw, lty, _mm_nmsub_ps( pwrx, ltz, _mm_mul_ps( pwrz, ltx ) ) );
		__m128 cz = _mm_madd_ps( pwrw, ltz, _mm_nmsub_ps( pwry, ltx, _mm_mul_ps( pwrx, lty ) ) );

		// wt = pwt + lt + 2 * Cross(pr, c)
		__m128 wtx = _mm_nmsub_ps( pwrz, cy, _mm_mul_ps( pwry, cz ) );
		__m128 wty = _mm_nmsub_ps( pwrx, cz, _mm_mul_ps( pwrz, cx ) );
		__m128 wtz = _mm_nmsub_ps( pwry, cx, _mm_mul_ps( pwrx, cy ) );
		wtx = _mm_add_ps( _mm_add_ps( pwtx, ltx ), _mm_add_ps( wtx, wtx ) );
		wty = _mm_add_ps( _mm_add_ps( pwty, lty ), _mm_add_ps( wty, wty ) );
		wtz = _mm_add_ps( _mm_add_ps( pwtz, ltz ), _mm_add_ps( wtz, wtz ) );

		// Deswizzle + store rotated translation
		__m128 wt0 = wtx, wt1 = wty, wt2 = wtz, wt3 = k1_0f;
		_MM_TRANSPOSE4_PS( wt0, wt1, wt2, wt3 );

		// Store translation
		outputBase[ _mm_extract_epi16( jointIdx, 0 ) + 1 ] = wt0;
		outputBase[ _mm_extract_epi16( jointIdx, 2 ) + 1 ] = wt1;
		outputBase[ _mm_extract_epi16( jointIdx, 4 ) + 1 ] = wt2;
		outputBase[ _mm_extract_epi16( jointIdx, 6 ) + 1 ] = wt3;
    }
}
