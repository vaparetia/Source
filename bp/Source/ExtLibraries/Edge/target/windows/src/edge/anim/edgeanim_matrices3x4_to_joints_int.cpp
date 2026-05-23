/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/anim/edgeanim_structs_ps3.h"
#include "edge/anim/edgeanim_windows.h"

extern "C"
void edgeAnimMatrices3x4ToJoints(EdgeAnimJointTransform* outputJoints, 
                                 const void* inputMatrices, 
                                 unsigned int count)
{
	EDGE_ASSERT(EDGE_IS_ALIGNED(outputJoints, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(inputMatrices, 16U));
	EDGE_ASSERT(count);

	// Constants
	const __m128 k0_5f = _mm_set1_ps( 0.5f );
	const __m128 k1_0f = _mm_set1_ps( 1.0f );

	// Pointers
	__m128* __restrict output = (__m128* __restrict) outputJoints;
	const __m128* __restrict input = (const __m128* __restrict) inputMatrices;

	for( unsigned int count4 = (count + 3) >> 2; count4; count4-- )
	{
		// Load matrices
		const __m128 m0row0 = input[0];
		const __m128 m0row1 = input[1];
		const __m128 m0row2 = input[2];
		const __m128 m1row0 = input[3];
		const __m128 m1row1 = input[4];
		const __m128 m1row2 = input[5];
		const __m128 m2row0 = input[6];
		const __m128 m2row1 = input[7];
		const __m128 m2row2 = input[8];
		const __m128 m3row0 = input[9];
		const __m128 m3row1 = input[10];
		const __m128 m3row2 = input[11];

		// Swizzle matrices
		__m128 m00 = m0row0, m10 = m1row0, m20 = m2row0, m30 = m3row0;
		__m128 m01 = m0row1, m11 = m1row1, m21 = m2row1, m31 = m3row1;
		__m128 m02 = m0row2, m12 = m1row2, m22 = m2row2, m32 = m3row2;
		_MM_TRANSPOSE4_PS( m00, m10, m20, m30 );
		_MM_TRANSPOSE4_PS( m01, m11, m21, m31 );
		_MM_TRANSPOSE4_PS( m02, m12, m22, m32 );

		// Deswizzle + output translations
		__m128 t0 = m30, t1 = m31, t2 = m32, t3 = k1_0f;
		_MM_TRANSPOSE4_PS( t0, t1, t2, t3 );

		output[1] = t0;
		output[4] = t1;
		output[7] = t2;
		output[10] = t3;

		// Now for scales
		__m128 sx_sqr = _mm_mul_ps( m00, m00 );
		__m128 sy_sqr = _mm_mul_ps( m10, m10 );
		__m128 sz_sqr = _mm_mul_ps( m20, m20 );
		sx_sqr = _mm_madd_ps( m01, m01, sx_sqr );
		sy_sqr = _mm_madd_ps( m11, m11, sy_sqr );
		sz_sqr = _mm_madd_ps( m21, m21, sz_sqr );
		sx_sqr = _mm_madd_ps( m02, m02, sx_sqr );
		sy_sqr = _mm_madd_ps( m12, m12, sy_sqr );
		sz_sqr = _mm_madd_ps( m22, m22, sz_sqr );
		__m128 isx = _mm_rsqrt_ps( sx_sqr );
		__m128 isy = _mm_rsqrt_ps( sy_sqr );
		__m128 isz = _mm_rsqrt_ps( sz_sqr );
		isx = _mm_madd_ps( _mm_nmsub_ps( sx_sqr, _mm_mul_ps( isx, isx ), k1_0f ), _mm_mul_ps( isx, k0_5f ), isx );
		isy = _mm_madd_ps( _mm_nmsub_ps( sy_sqr, _mm_mul_ps( isy, isy ), k1_0f ), _mm_mul_ps( isy, k0_5f ), isy );
		isz = _mm_madd_ps( _mm_nmsub_ps( sz_sqr, _mm_mul_ps( isz, isz ), k1_0f ), _mm_mul_ps( isz, k0_5f ), isz );

		// Deswizzle + output scales
		__m128 sx = _mm_mul_ps( isx, sx_sqr );
		__m128 sy = _mm_mul_ps( isy, sy_sqr );
		__m128 sz = _mm_mul_ps( isz, sz_sqr );
		__m128 s0 = sx, s1 = sy, s2 = sz, s3 = k1_0f;
		_MM_TRANSPOSE4_PS( s0, s1, s2, s3 );

		output[2] = s0;
		output[5] = s1;
		output[8] = s2;
		output[11] = s3;

		// Normalize matrix
		m00 = _mm_mul_ps( m00, isx );
		m10 = _mm_mul_ps( m10, isy );
		m20 = _mm_mul_ps( m20, isz );
		m01 = _mm_mul_ps( m01, isx );
		m11 = _mm_mul_ps( m11, isy );
		m21 = _mm_mul_ps( m21, isz );
		m02 = _mm_mul_ps( m02, isx );
		m12 = _mm_mul_ps( m12, isy );
		m22 = _mm_mul_ps( m22, isz );

		// Extract quats
		__m128 xx = m00, yx = m01, zx = m02;
		__m128 xy = m10, yy = m11, zy = m12;
		__m128 xz = m20, yz = m21, zz = m22;
		__m128 trace = _mm_add_ps( _mm_add_ps( xx, yy ), zz );
		__m128 negTrace = _mm_cmplt_ps( trace, _mm_setzero_ps() );
		__m128 ZgtX = _mm_cmpgt_ps( zz, xx );
		__m128 ZgtY = _mm_cmpgt_ps( zz, yy );
		__m128 YgtX = _mm_cmpgt_ps( yy, xx );
		__m128 largestXorY = _mm_andnot_ps( _mm_and_ps( ZgtX, ZgtY ), negTrace );
		__m128 largestYorZ = _mm_and_ps( _mm_or_ps( YgtX, ZgtX ), negTrace );
		__m128 largestZorX = _mm_andnot_ps( _mm_andnot_ps( ZgtY, YgtX ), negTrace );
		zz = _mm_sel_ps( zz, _mm_sub_ps( _mm_setzero_ps(), zz ), largestXorY );
		xy = _mm_sel_ps( xy, _mm_sub_ps( _mm_setzero_ps(), xy ), largestXorY );
		xx = _mm_sel_ps( xx, _mm_sub_ps( _mm_setzero_ps(), xx ), largestYorZ );
		yz = _mm_sel_ps( yz, _mm_sub_ps( _mm_setzero_ps(), yz ), largestYorZ );
		yy = _mm_sel_ps( yy, _mm_sub_ps( _mm_setzero_ps(), yy ), largestZorX );
		zx = _mm_sel_ps( zx, _mm_sub_ps( _mm_setzero_ps(), zx ), largestZorX );
		__m128 radicand = _mm_add_ps( _mm_add_ps( xx, yy ), _mm_add_ps( zz, k1_0f ) );
		__m128 radicand_rsq0 = _mm_rsqrt_ps( radicand );
		__m128 radicand_rsq1 = _mm_mul_ps( radicand_rsq0, radicand_rsq0 );
		__m128 radicand_rsq2 = _mm_nmsub_ps( radicand, radicand_rsq1, k1_0f );
		__m128 radicand_rsq3 = _mm_mul_ps( radicand_rsq0, k0_5f );
		__m128 radicand_rsq = _mm_madd_ps( radicand_rsq2, radicand_rsq3, radicand_rsq0 );
		__m128 scale = _mm_mul_ps( k0_5f, radicand_rsq );
		__m128 tmpx = _mm_mul_ps( _mm_sub_ps( zy, yz ), scale );
		__m128 tmpy = _mm_mul_ps( _mm_sub_ps( xz, zx ), scale );
		__m128 tmpz = _mm_mul_ps( _mm_sub_ps( yx, xy ), scale );
		__m128 tmpw = _mm_mul_ps( radicand, scale );
		__m128 qx = _mm_sel_ps( tmpx, tmpw, largestXorY );
		__m128 qy = _mm_sel_ps( tmpy, tmpz, largestXorY );
		__m128 qz = _mm_sel_ps( tmpz, tmpy, largestXorY );
		__m128 qw = _mm_sel_ps( tmpw, tmpx, largestXorY );
		tmpx = qx;
		tmpz = qz;
		qx = _mm_sel_ps( qx, qy, largestYorZ );
		qy = _mm_sel_ps( qy, tmpx, largestYorZ );
		qz = _mm_sel_ps( qz, qw, largestYorZ );
		qw = _mm_sel_ps( qw, tmpz, largestYorZ );

		// Deswizzle quats
		__m128 q0 = qx, q1 = qy, q2 = qz, q3 = qw;
		_MM_TRANSPOSE4_PS( q0, q1, q2, q3 );

		output[0] = q0;
		output[3] = q1;
		output[6] = q2;
		output[9] = q3;

		// Advance pointers
		input += 12;
		output += 12;
	}
}
