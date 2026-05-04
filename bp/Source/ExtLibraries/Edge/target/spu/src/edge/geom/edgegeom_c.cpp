/* SCE CONFIDENTIAL 
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/geom/edgegeom.h"
#include "edge/geom/edgegeom_internal.h"

#if !EDGEGEOMNORMALIZEUNIFORMTABLE_INTRINSICS
static inline float rsqrt(float f)
{
	return si_to_float(si_fi(si_from_float(f), si_frsqest(si_from_float(f))));
}

void edgeGeomNormalizeUniformTable(qword *pUniform, int32_t vertexCount)
{
	float *vertex = (float *)pUniform;
	const uint32_t roundedVertexCount = (vertexCount + 7) & ~7;
	for (uint32_t i = 0; i < roundedVertexCount; i++) {
		float s = rsqrt(vertex[0] * vertex[0] + vertex[1] * vertex[1] + vertex[2] * vertex[2]);
		vertex[0] *= s;
		vertex[1] *= s;
		vertex[2] *= s;
		vertex += 4;
	}
}
#endif
