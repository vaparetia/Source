
/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/geom/edgegeom_config.h"
#include "edge/geom/edgegeom.h"

#ifndef EDGEGEOM_INTERNAL_H
#define EDGEGEOM_INTERNAL_H

// Wait for ~16 ms before warning that RSX get pointer has not changed                                                            
#define EDGE_GEOM_WARN_GET_POINTER_UNCHANGED_COUNT (531)

void *CompressVertexesByDescription(EdgeGeomSpuContext *ctx,	const EdgeGeomVertexStreamDescription *streamDesc);
uint32_t cullOccludedTriangles(EdgeGeomSpuContext *ctx, const uint32_t indexCount, const uint32_t occluderCount, float occluderData[8*4*4], int32_t indexBias = 0, EdgeGeomCullingResults *detailedResults = 0);
void DecompressBlendShapeRunTable(uint16_t *runTable, uint32_t runTableSize,
	uint32_t numDeltas);
void DecompressIndexes(EdgeGeomSpuContext *ctx, const void *indexes);
void DecompressVertexesByDescription(EdgeGeomSpuContext *ctx,
	const void *vertexes, const void *fixedOffsets, const EdgeGeomVertexStreamDescription *streamDesc,
	uint32_t numVertexes, float blendFactor = 0.0f, uint16_t *blendedVertexIndexTable = 0);
void edgeGeomPreprocessMatrices(vec_float4 *matrices, uint32_t matrixCount);
void transformVertexesForCull(EdgeGeomSpuContext *ctx);

#if !EDGEGEOMCULLOCCLUDEDTRIANGLES_INTRINSICS || !EDGEGEOMSKINVERTEXES_INTRINSICS || !TRANSFORMVERTEXESFORCULL_INTRINSICS
static inline void matrixMul(float *mOut, const float *mA, const float *mB)
{
	float tmp[16];
	tmp[0] = mA[0] * mB[0] + mA[1] * mB[4] + mA[2] * mB[8] + mA[3] * mB[12];
	tmp[1] = mA[0] * mB[1] + mA[1] * mB[5] + mA[2] * mB[9] + mA[3] * mB[13];
	tmp[2] = mA[0] * mB[2] + mA[1] * mB[6] + mA[2] * mB[10] + mA[3] * mB[14];
	tmp[3] = mA[0] * mB[3] + mA[1] * mB[7] + mA[2] * mB[11] + mA[3] * mB[15];

	tmp[4] = mA[4] * mB[0] + mA[5] * mB[4] + mA[6] * mB[8] + mA[7] * mB[12];
	tmp[5] = mA[4] * mB[1] + mA[5] * mB[5] + mA[6] * mB[9] + mA[7] * mB[13];
	tmp[6] = mA[4] * mB[2] + mA[5] * mB[6] + mA[6] * mB[10] + mA[7] * mB[14];
	tmp[7] = mA[4] * mB[3] + mA[5] * mB[7] + mA[6] * mB[11] + mA[7] * mB[15];

	tmp[ 8] = mA[8] * mB[0] + mA[9] * mB[4] + mA[10] * mB[8] + mA[11] * mB[12];
	tmp[ 9] = mA[8] * mB[1] + mA[9] * mB[5] + mA[10] * mB[9] + mA[11] * mB[13];
	tmp[10] = mA[8] * mB[2] + mA[9] * mB[6] + mA[10] * mB[10] + mA[11] * mB[14];
	tmp[11] = mA[8] * mB[3] + mA[9] * mB[7] + mA[10] * mB[11] + mA[11] * mB[15];

	tmp[12] = mA[12] * mB[0] + mA[13] * mB[4] + mA[14] * mB[8] + mA[15] * mB[12];
	tmp[13] = mA[12] * mB[1] + mA[13] * mB[5] + mA[14] * mB[9] + mA[15] * mB[13];
	tmp[14] = mA[12] * mB[2] + mA[13] * mB[6] + mA[14] * mB[10] + mA[15] * mB[14];
	tmp[15] = mA[12] * mB[3] + mA[13] * mB[7] + mA[14] * mB[11] + mA[15] * mB[15];

	mOut[0] = tmp[0];
	mOut[1] = tmp[1];
	mOut[2] = tmp[2];
	mOut[3] = tmp[3];
	mOut[4] = tmp[4];
	mOut[5] = tmp[5];
	mOut[6] = tmp[6];
	mOut[7] = tmp[7];
	mOut[8] = tmp[8];
	mOut[9] = tmp[9];
	mOut[10] = tmp[10];
	mOut[11] = tmp[11];
	mOut[12] = tmp[12];
	mOut[13] = tmp[13];
	mOut[14] = tmp[14];
	mOut[15] = tmp[15];
}
#endif

#endif // EDGEGEOM_INTERNAL_H



