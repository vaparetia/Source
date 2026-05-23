/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/anim/edgeanim_structs_ps3.h"
#include "edge/anim/edgeanim_windows.h"

extern "C"
void edgeAnimJointsToMatrices3x4(void* outputMatrices, 
                                 const struct EdgeAnimJointTransform* inputJoints, 
                                 unsigned int count)
{
	EDGE_ASSERT(EDGE_IS_ALIGNED(outputMatrices, 16U));
	EDGE_ASSERT(EDGE_IS_ALIGNED(inputJoints, 16U));
	EDGE_ASSERT(count);

	// Constants: Misc
	const __m128 k0_5f = _mm_set1_ps( 0.5f );

	// Pointers
	__m128* __restrict output = (__m128* __restrict) outputMatrices;
	const __m128* __restrict input = (const __m128* __restrict) inputJoints;

    for( unsigned int count4 = (count + 3) >> 2; count4; count4-- )
	{
		// Swizzle quats
		// (X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3 - > X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3)
		__m128 rx = input[0];
		__m128 ry = input[3];
		__m128 rz = input[6];
		__m128 rw = input[9];
		_MM_TRANSPOSE4_PS( rx, ry, rz, rw );

		// Swizzle scales
		// (X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3 - > X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3)
		__m128 sx = input[2];
		__m128 sy = input[5];
		__m128 sz = input[8];
		__m128 sw = input[11];
		_MM_TRANSPOSE4_PS( sx, sy, sz, sw );

		// Pre-multiply by two
		sx = _mm_add_ps( sx, sx );
		sy = _mm_add_ps( sy, sy );
		sz = _mm_add_ps( sz, sz );

		// Construct scaled rotation matrix
		__m128 mrxx = _mm_nmsub_ps( rx, rx, k0_5f );
		__m128 mryy = _mm_nmsub_ps( ry, ry, k0_5f );
		__m128 rxy = _mm_mul_ps( rx, ry );
		__m128 rxz = _mm_mul_ps( rx, rz );
		__m128 ryz = _mm_mul_ps( ry, rz );
		__m128 m00 = _mm_mul_ps( _mm_nmsub_ps( rz, rz, mryy ), sx );
		__m128 m01 = _mm_mul_ps( _mm_madd_ps( rw, rz, rxy ), sx );
		__m128 m02 = _mm_mul_ps( _mm_nmsub_ps( rw, ry, rxz ), sx );
		__m128 m10 = _mm_mul_ps( _mm_nmsub_ps( rw, rz, rxy ), sy );
		__m128 m11 = _mm_mul_ps( _mm_nmsub_ps( rz, rz, mrxx ), sy );
		__m128 m12 = _mm_mul_ps( _mm_madd_ps( rw, rx, ryz ), sy );
		__m128 m20 = _mm_mul_ps( _mm_madd_ps( rw, ry, rxz ), sz );
		__m128 m21 = _mm_mul_ps( _mm_nmsub_ps( rw, rx, ryz ), sz );
		__m128 m22 = _mm_mul_ps( _mm_nmsub_ps( ry, ry, mrxx ), sz );

		// Swizzle translations
		// (X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3 - > X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3)
		__m128 t0 = input[1];
		__m128 t1 = input[4];
		__m128 t2 = input[7];
		__m128 t3 = input[10];
		__m128 m30 = t0, m31 = t1, m32 = t2, m33 = t3;
		_MM_TRANSPOSE4_PS( m30, m31, m32, m33 );

		// Deswizzle & store row 0 (for each of the 4 matrices)
		// (X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3 -> X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3)
		__m128 mat0row0 = m00, mat1row0 = m10, mat2row0 = m20, mat3row0 = m30;
		_MM_TRANSPOSE4_PS( mat0row0, mat1row0, mat2row0, mat3row0 );
		output[0] = mat0row0;
		output[3] = mat1row0;
		output[6] = mat2row0;
		output[9] = mat3row0;

		// Deswizzle & store row 1 (for each of the 4 matrices)
		// (X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3 -> X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3)
		__m128 mat0row1 = m01, mat1row1 = m11, mat2row1 = m21, mat3row1 = m31;
		_MM_TRANSPOSE4_PS( mat0row1, mat1row1, mat2row1, mat3row1 );
		output[1] = mat0row1;
		output[4] = mat1row1;
		output[7] = mat2row1;
		output[10] = mat3row1;

		// Deswizzle & store row 2 (for each of the 4 matrices)
		// (X0X1X2X3 Y0Y1Y2Y3 Z0Z1Z2Z3 W0W1W2W3 -> X0Y0Z0W0 X1Y1Z1W1 X2Y2Z2W2 X3Y3Z3W3)
		__m128 mat0row2 = m02, mat1row2 = m12, mat2row2 = m22, mat3row2 = m32;
		_MM_TRANSPOSE4_PS( mat0row2, mat1row2, mat2row2, mat3row2 );
		output[2] = mat0row2;
		output[5] = mat1row2;
		output[8] = mat2row2;
		output[11] = mat3row2;

		// Advance pointers
		input += 12;
		output += 12;
	}
}
