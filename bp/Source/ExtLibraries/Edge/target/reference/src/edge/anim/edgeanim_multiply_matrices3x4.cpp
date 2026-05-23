/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/anim/edgeanim_structs_ps3.h"

using namespace Vectormath::Aos;
/*
 * Reference C++ implementation using vectormath.
 * This function is meant be used by PS3 unit tests and PC implementation, not by PS3 runtime.
 */

extern "C"
void edgeAnimMultiplyMatrices3x4(void* outputMatrices, 
								 const void* leftMatrices,
								 const void* rightMatrices,
                                 unsigned int count)
{
	Vector4* output = (Vector4*)outputMatrices;
	Vector4* leftm = (Vector4*)leftMatrices;
	Vector4* rightm = (Vector4*)rightMatrices;

    for (; count; count--) {
		Matrix4 l(leftm[0], leftm[1], leftm[2], Vector4(0,0,0,1));
		Matrix4 r(rightm[0], rightm[1], rightm[2], Vector4(0,0,0,1));

		// note: all matrices are transposed so multiplication order is reversed
		Matrix4 out = r * l;

		output[0] = out.getCol(0);
		output[1] = out.getCol(1);
		output[2] = out.getCol(2);

		output += 3;
		leftm += 3;
		rightm += 3;
    }
}
