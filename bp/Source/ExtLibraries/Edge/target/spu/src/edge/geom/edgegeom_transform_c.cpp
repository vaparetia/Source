/* SCE CONFIDENTIAL 
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/geom/edgegeom.h"
#include "edge/geom/edgegeom_internal.h"

#if !EDGEGEOMTRANSFORMVERTEXES_INTRINSICS
void edgeGeomTransformVertexes(uint32_t numVertexes, void *inVertexes, void *outVertexes, void *matrix)
{
	//since the intrinsics version does 8 at a time, we'll round to a multiple of 8, and then divide by 4 since
	//this code does 4 at a time

	const uint32_t numIterations = ((numVertexes + 7) & ~7) >> 2;

	float *mat1 = (float*)matrix;
	float *mat2 = (float*)inVertexes;
	float *pOut = (float*)outVertexes;

	for (uint32_t i = 0; i < numIterations; i++) {
		static float out[16];
		out[0*4+0] = mat1[0*4+0]*mat2[0*4+0] + mat1[0*4+1]*mat2[0*4+1] + mat1[0*4+2]*mat2[0*4+2] + mat1[0*4+3]*mat2[0*4+3];
		out[0*4+1] = mat1[1*4+0]*mat2[0*4+0] + mat1[1*4+1]*mat2[0*4+1] + mat1[1*4+2]*mat2[0*4+2] + mat1[1*4+3]*mat2[0*4+3];
		out[0*4+2] = mat1[2*4+0]*mat2[0*4+0] + mat1[2*4+1]*mat2[0*4+1] + mat1[2*4+2]*mat2[0*4+2] + mat1[2*4+3]*mat2[0*4+3];
		out[0*4+3] = mat1[3*4+0]*mat2[0*4+0] + mat1[3*4+1]*mat2[0*4+1] + mat1[3*4+2]*mat2[0*4+2] + mat1[3*4+3]*mat2[0*4+3];

		out[1*4+0] = mat1[0*4+0]*mat2[1*4+0] + mat1[0*4+1]*mat2[1*4+1] + mat1[0*4+2]*mat2[1*4+2] + mat1[0*4+3]*mat2[1*4+3];
		out[1*4+1] = mat1[1*4+0]*mat2[1*4+0] + mat1[1*4+1]*mat2[1*4+1] + mat1[1*4+2]*mat2[1*4+2] + mat1[1*4+3]*mat2[1*4+3];
		out[1*4+2] = mat1[2*4+0]*mat2[1*4+0] + mat1[2*4+1]*mat2[1*4+1] + mat1[2*4+2]*mat2[1*4+2] + mat1[2*4+3]*mat2[1*4+3];
		out[1*4+3] = mat1[3*4+0]*mat2[1*4+0] + mat1[3*4+1]*mat2[1*4+1] + mat1[3*4+2]*mat2[1*4+2] + mat1[3*4+3]*mat2[1*4+3];

		out[2*4+0] = mat1[0*4+0]*mat2[2*4+0] + mat1[0*4+1]*mat2[2*4+1] + mat1[0*4+2]*mat2[2*4+2] + mat1[0*4+3]*mat2[2*4+3];
		out[2*4+1] = mat1[1*4+0]*mat2[2*4+0] + mat1[1*4+1]*mat2[2*4+1] + mat1[1*4+2]*mat2[2*4+2] + mat1[1*4+3]*mat2[2*4+3];
		out[2*4+2] = mat1[2*4+0]*mat2[2*4+0] + mat1[2*4+1]*mat2[2*4+1] + mat1[2*4+2]*mat2[2*4+2] + mat1[2*4+3]*mat2[2*4+3];
		out[2*4+3] = mat1[3*4+0]*mat2[2*4+0] + mat1[3*4+1]*mat2[2*4+1] + mat1[3*4+2]*mat2[2*4+2] + mat1[3*4+3]*mat2[2*4+3];

		out[3*4+0] = mat1[0*4+0]*mat2[3*4+0] + mat1[0*4+1]*mat2[3*4+1] + mat1[0*4+2]*mat2[3*4+2] + mat1[0*4+3]*mat2[3*4+3];
		out[3*4+1] = mat1[1*4+0]*mat2[3*4+0] + mat1[1*4+1]*mat2[3*4+1] + mat1[1*4+2]*mat2[3*4+2] + mat1[1*4+3]*mat2[3*4+3];
		out[3*4+2] = mat1[2*4+0]*mat2[3*4+0] + mat1[2*4+1]*mat2[3*4+1] + mat1[2*4+2]*mat2[3*4+2] + mat1[2*4+3]*mat2[3*4+3];
		out[3*4+3] = mat1[3*4+0]*mat2[3*4+0] + mat1[3*4+1]*mat2[3*4+1] + mat1[3*4+2]*mat2[3*4+2] + mat1[3*4+3]*mat2[3*4+3];

		pOut[ 0] = out[ 0];
		pOut[ 1] = out[ 1];
		pOut[ 2] = out[ 2];
		pOut[ 3] = out[ 3];
		pOut[ 4] = out[ 4];
		pOut[ 5] = out[ 5];
		pOut[ 6] = out[ 6];
		pOut[ 7] = out[ 7];
		pOut[ 8] = out[ 8];
		pOut[ 9] = out[ 9];
		pOut[10] = out[10];
		pOut[11] = out[11];
		pOut[12] = out[12];
		pOut[13] = out[13];
		pOut[14] = out[14];
		pOut[15] = out[15];

		mat2 += 16;
		pOut += 16;
	}
}
#endif
