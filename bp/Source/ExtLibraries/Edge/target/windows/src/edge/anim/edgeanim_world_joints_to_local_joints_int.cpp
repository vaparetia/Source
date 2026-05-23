/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/anim/edgeanim_structs_ps3.h"
#include "edge/anim/edgeanim_windows.h"

extern "C"
void edgeAnimWorldJointsToLocalJoints(EdgeAnimJointTransform* outputJoints, 
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

		// Load + Swizzle World Joint Transform Components
		__m128 wrx = inputBase[ _mm_extract_epi16( jointIdx, 0 ) ];
		__m128 wry = inputBase[ _mm_extract_epi16( jointIdx, 2 ) ];
		__m128 wrz = inputBase[ _mm_extract_epi16( jointIdx, 4 ) ];
		__m128 wrw = inputBase[ _mm_extract_epi16( jointIdx, 6 ) ];
		_MM_TRANSPOSE4_PS( wrx, wry, wrz, wrw );

		__m128 wtx = inputBase[ _mm_extract_epi16( jointIdx, 0 ) + 1 ];
		__m128 wty = inputBase[ _mm_extract_epi16( jointIdx, 2 ) + 1 ];
		__m128 wtz = inputBase[ _mm_extract_epi16( jointIdx, 4 ) + 1 ];
		__m128 wtw = inputBase[ _mm_extract_epi16( jointIdx, 6 ) + 1 ];
		_MM_TRANSPOSE4_PS( wtx, wty, wtz, wtw );

		__m128 wsx = inputBase[ _mm_extract_epi16( jointIdx, 0 ) + 2 ];
		__m128 wsy = inputBase[ _mm_extract_epi16( jointIdx, 2 ) + 2 ];
		__m128 wsz = inputBase[ _mm_extract_epi16( jointIdx, 4 ) + 2 ];
		__m128 wsw = inputBase[ _mm_extract_epi16( jointIdx, 6 ) + 2 ];
		_MM_TRANSPOSE4_PS( wsx, wsy, wsz, wsw );

		// Load + Swizzle Parent World Joint (or Root) Transform Components
		__m128 pwrx = inputBase[ _mm_extract_epi16( parentIdx, 1 ) ];
		__m128 pwry = inputBase[ _mm_extract_epi16( parentIdx, 3 ) ];
		__m128 pwrz = inputBase[ _mm_extract_epi16( parentIdx, 5 ) ];
		__m128 pwrw = inputBase[ _mm_extract_epi16( parentIdx, 7 ) ];
		_MM_TRANSPOSE4_PS( pwrx, pwry, pwrz, pwrw );

		pwrx = _mm_sel_ps( pwrx, rootJointRx, parentUseRoot );
		pwry = _mm_sel_ps( pwry, rootJointRy, parentUseRoot );
		pwrz = _mm_sel_ps( pwrz, rootJointRz, parentUseRoot );
		pwrw = _mm_sel_ps( pwrw, rootJointRw, parentUseRoot );

		__m128 pwtx = inputBase[ _mm_extract_epi16( parentIdx, 1 ) + 1 ];
		__m128 pwty = inputBase[ _mm_extract_epi16( parentIdx, 3 ) + 1 ];
		__m128 pwtz = inputBase[ _mm_extract_epi16( parentIdx, 5 ) + 1 ];
		__m128 pwtw = inputBase[ _mm_extract_epi16( parentIdx, 7 ) + 1 ];
		_MM_TRANSPOSE4_PS( pwtx, pwty, pwtz, pwtw );

		pwtx = _mm_sel_ps( pwtx, rootJointTx, parentUseRoot );
		pwty = _mm_sel_ps( pwty, rootJointTy, parentUseRoot );
		pwtz = _mm_sel_ps( pwtz, rootJointTz, parentUseRoot );

		__m128 pwsx = inputBase[ _mm_extract_epi16( parentIdx, 1 ) + 2 ];
		__m128 pwsy = inputBase[ _mm_extract_epi16( parentIdx, 3 ) + 2 ];
		__m128 pwsz = inputBase[ _mm_extract_epi16( parentIdx, 5 ) + 2 ];
		__m128 pwsw = inputBase[ _mm_extract_epi16( parentIdx, 7 ) + 2 ];
		_MM_TRANSPOSE4_PS( pwsx, pwsy, pwsz, pwsw );

		pwsx = _mm_sel_ps( pwsx, rootJointSx, parentUseRoot );
		pwsy = _mm_sel_ps( pwsy, rootJointSy, parentUseRoot );
		pwsz = _mm_sel_ps( pwsz, rootJointSz, parentUseRoot );

		// Load + Swizzle Parent Local Joint (or Root) Scale Components
		__m128 plsx = outputBase[ _mm_extract_epi16( parentIdx, 1 ) + 2 ];
		__m128 plsy = outputBase[ _mm_extract_epi16( parentIdx, 3 ) + 2 ];
		__m128 plsz = outputBase[ _mm_extract_epi16( parentIdx, 5 ) + 2 ];
		__m128 plsw = outputBase[ _mm_extract_epi16( parentIdx, 7 ) + 2 ];
		_MM_TRANSPOSE4_PS( plsx, plsy, plsz, plsw );

		plsx = _mm_sel_ps( plsx, rootJointSx, parentUseRoot );
		plsy = _mm_sel_ps( plsy, rootJointSy, parentUseRoot );
		plsz = _mm_sel_ps( plsz, rootJointSz, parentUseRoot );

		// Invert parent quats
		__m128 pwrix = _mm_sub_ps( _mm_setzero_ps(), pwrx );
		__m128 pwriy = _mm_sub_ps( _mm_setzero_ps(), pwry );
		__m128 pwriz = _mm_sub_ps( _mm_setzero_ps(), pwrz );
		__m128 pwriw = pwrw;

		// Local quat = inverse parent world quat * world quat
		__m128 lrx = _mm_nmsub_ps( pwriz, wry, _mm_madd_ps( pwriy, wrz, _mm_madd_ps( pwrix, wrw, _mm_mul_ps( pwriw, wrx ) ) ) );
		__m128 lry = _mm_madd_ps( pwriz, wrx, _mm_madd_ps( pwriy, wrw, _mm_nmsub_ps( pwrix, wrz, _mm_mul_ps( pwriw, wry ) ) ) );
		__m128 lrz = _mm_madd_ps( pwriz, wrw, _mm_nmsub_ps( pwriy, wrx, _mm_madd_ps( pwrix, wry, _mm_mul_ps( pwriw, wrz ) ) ) );
		__m128 lrw = _mm_nmsub_ps( pwriz, wrz, _mm_nmsub_ps( pwriy, wry, _mm_nmsub_ps( pwrix, wrx, _mm_mul_ps( pwriw, wrw ) ) ) );

		// Normalise local quats
		__m128 lrsqr = _mm_madd_ps( lrx, lrx, _mm_madd_ps( lry, lry, _mm_madd_ps( lrz, lrz, _mm_mul_ps( lrw, lrw ) ) ) );
		__m128 inv = _mm_rsqrt_ps( lrsqr );
		inv = _mm_madd_ps( _mm_nmsub_ps( lrsqr, _mm_mul_ps( inv, inv ), k1_0f ), _mm_mul_ps( inv, k0_5f ), inv );
		lrx = _mm_mul_ps( lrx, inv );
		lry = _mm_mul_ps( lry, inv );
		lrz = _mm_mul_ps( lrz, inv );
		lrw = _mm_mul_ps( lrw, inv );

		// Deswizzle & store rotation
		__m128 lr0 = lrx, lr1 = lry, lr2 = lrz, lr3 = lrw;
		_MM_TRANSPOSE4_PS( lr0, lr1, lr2, lr3 );

		outputBase[ _mm_extract_epi16( jointIdx, 0 ) ] = lr0;
		outputBase[ _mm_extract_epi16( jointIdx, 2 ) ] = lr1;
		outputBase[ _mm_extract_epi16( jointIdx, 4 ) ] = lr2;
		outputBase[ _mm_extract_epi16( jointIdx, 6 ) ] = lr3;

		// 1 / parent world scale
		__m128 pwsix = _mm_rcp_ps( pwsx );
		__m128 pwsiy = _mm_rcp_ps( pwsy );
		__m128 pwsiz = _mm_rcp_ps( pwsz );
		pwsix = _mm_madd_ps( _mm_nmsub_ps( pwsx, pwsix, k1_0f ), pwsix, pwsix );
		pwsiy = _mm_madd_ps( _mm_nmsub_ps( pwsy, pwsiy, k1_0f ), pwsiy, pwsiy );
		pwsiz = _mm_madd_ps( _mm_nmsub_ps( pwsz, pwsiz, k1_0f ), pwsiz, pwsiz );

		// Scale compensate
		__m128 scalex = _mm_sel_ps( k1_0f, plsx, compensateMask );
		__m128 scaley = _mm_sel_ps( k1_0f, plsy, compensateMask );
		__m128 scalez = _mm_sel_ps( k1_0f, plsz, compensateMask );

		// Final scale
		__m128 lsx = _mm_mul_ps( _mm_mul_ps( wsx, scalex ), pwsix );
		__m128 lsy = _mm_mul_ps( _mm_mul_ps( wsy, scaley ), pwsiy );
		__m128 lsz = _mm_mul_ps( _mm_mul_ps( wsz, scalez ), pwsiz );

		// Deswizzle + Store scale
		__m128 ls0 = lsx, ls1 = lsy, ls2 = lsz, ls3 = k1_0f;
		_MM_TRANSPOSE4_PS( ls0, ls1, ls2, ls3 );

		outputBase[ _mm_extract_epi16( jointIdx, 0 ) + 2 ] = ls0;
		outputBase[ _mm_extract_epi16( jointIdx, 2 ) + 2 ] = ls1;
		outputBase[ _mm_extract_epi16( jointIdx, 4 ) + 2 ] = ls2;
		outputBase[ _mm_extract_epi16( jointIdx, 6 ) + 2 ] = ls3;

		// Offset translation
		wtx = _mm_sub_ps( wtx, pwtx );
		wty = _mm_sub_ps( wty, pwty );
		wtz = _mm_sub_ps( wtz, pwtz );

		// c = Cross(pwir, wt) + pwriw*wt = 
		// (pwriy*wtz-pwriz*wty, pwriz*wtx-pwrix*wtz, pwrix*wty-pwriy*wtx) + pwriw*wt
		__m128 cx = _mm_madd_ps( pwriw, wtx, _mm_nmsub_ps( pwriz, wty, _mm_mul_ps( pwriy, wtz ) ) );
		__m128 cy = _mm_madd_ps( pwriw, wty, _mm_nmsub_ps( pwrix, wtz, _mm_mul_ps( pwriz, wtx ) ) );
		__m128 cz = _mm_madd_ps( pwriw, wtz, _mm_nmsub_ps( pwriy, wtx, _mm_mul_ps( pwrix, wty ) ) );

		// d = wt + 2 * Cross(pwr, c)
		__m128 dx = _mm_nmsub_ps( pwriz, cy, _mm_mul_ps( pwriy, cz ) );
		__m128 dy = _mm_nmsub_ps( pwrix, cz, _mm_mul_ps( pwriz, cx ) );
		__m128 dz = _mm_nmsub_ps( pwriy, cx, _mm_mul_ps( pwrix, cy ) );
		dx = _mm_add_ps( wtx, _mm_add_ps( dx, dx ) );
		dy = _mm_add_ps( wty, _mm_add_ps( dy, dy ) );
		dz = _mm_add_ps( wtz, _mm_add_ps( dz, dz ) );

		// Scale + deswizzle rotated translation
		__m128 lt0 = _mm_mul_ps( pwsix, dx );
		__m128 lt1 = _mm_mul_ps( pwsiy, dy );
		__m128 lt2 = _mm_mul_ps( pwsiz, dz );
		__m128 lt3 = k1_0f;
		_MM_TRANSPOSE4_PS( lt0, lt1, lt2, lt3 );

		// Store translation 
		outputBase[ _mm_extract_epi16( jointIdx, 0 ) + 1 ] = lt0;
		outputBase[ _mm_extract_epi16( jointIdx, 2 ) + 1 ] = lt1;
		outputBase[ _mm_extract_epi16( jointIdx, 4 ) + 1 ] = lt2;
		outputBase[ _mm_extract_epi16( jointIdx, 6 ) + 1 ] = lt3;
	}
}
