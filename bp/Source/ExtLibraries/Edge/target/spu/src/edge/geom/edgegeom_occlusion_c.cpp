/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <math.h>

#include "edge/geom/edgegeom_config.h"
#include "edge/geom/edgegeom.h"
#include "edge/geom/edgegeom_internal.h"

#ifndef FLT_MAX
#define FLT_MAX 3.40282347e+38f
#endif

#if !CULLOCCLUDEDTRIANGLES_INTRINSICS

static inline void cross3f(float *c, float *a, float *b)
{
	c[0] = a[1] * b[2] - a[2] * b[1];
	c[1] = a[2] * b[0] - a[0] * b[2];
	c[2] = a[0] * b[1] - a[1] * b[0];
}

static inline uint32_t dot3u(float *a, float *b)
{
	union {float f; uint32_t u32;} u;
	u.f = a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
	return u.u32;
}

static inline uint32_t cross2u(float *a, float *b)
{
	union {float f; uint32_t u32;} u;
	u.f = a[0] * b[1] - a[1] * b[0];
	return u.u32;
}

static inline void transCross(float *cp, float *x0, float *y0, float *x1, float *y1)
{
	cp[0] = -((y0[0] * x1[0]) - (x0[0] * y1[0]));
	cp[1] = -((y0[1] * x1[1]) - (x0[1] * y1[1]));
	cp[2] = -((y0[2] * x1[2]) - (x0[2] * y1[2]));
	cp[3] = -((y0[3] * x1[3]) - (x0[3] * y1[3]));
}

static inline void sub4f(float *o, float *a, float *b)
{
	o[0] = a[0] - b[0];
	o[1] = a[1] - b[1];
	o[2] = a[2] - b[2];
	o[3] = a[3] - b[3];
}

static void bbTest(float *ov0, float *ov1, float *ov2, float *ov3,
				   float *oc0, float *oc1, float *oc2, float *oc3,
				   float *bbmin, float *bbmax,
				   uint32_t *inFront, uint32_t *behind, uint32_t *inside)
{
	//transpose for the sake of sanity
	float boxx[4] = {bbmin[0], bbmin[0], bbmax[0], bbmax[0]};
	float boxy[4] = {bbmin[1], bbmax[1], bbmax[1], bbmin[1]};

	float zmin[4] = {bbmin[2], bbmin[2], bbmin[2], bbmin[2]};
	float zmax[4] = {bbmax[2], bbmax[2], bbmax[2], bbmax[2]};

	float ozzzz[4] = {ov0[2], ov1[2], ov2[2], ov3[2]};

	float wx0[4] = {ov0[0], ov0[0], ov0[0], ov0[0]};
	float wx1[4] = {ov1[0], ov1[0], ov1[0], ov1[0]};
	float wx2[4] = {ov2[0], ov2[0], ov2[0], ov2[0]};
	float wx3[4] = {ov3[0], ov3[0], ov3[0], ov3[0]};

	float wy0[4] = {ov0[1], ov0[1], ov0[1], ov0[1]};
	float wy1[4] = {ov1[1], ov1[1], ov1[1], ov1[1]};
	float wy2[4] = {ov2[1], ov2[1], ov2[1], ov2[1]};
	float wy3[4] = {ov3[1], ov3[1], ov3[1], ov3[1]};

	float pbX0[4], pbX1[4], pbX2[4], pbX3[4];
	float pbY0[4], pbY1[4], pbY2[4], pbY3[4];

	sub4f(pbX0, boxx, wx0);
	sub4f(pbX1, boxx, wx1);
	sub4f(pbX2, boxx, wx2);
	sub4f(pbX3, boxx, wx3);

	sub4f(pbY0, boxy, wy0);
	sub4f(pbY1, boxy, wy1);
	sub4f(pbY2, boxy, wy2);
	sub4f(pbY3, boxy, wy3);

	float wvx0[4] = {oc0[0], oc0[0], oc0[0], oc0[0]};
	float wvx1[4] = {oc1[0], oc1[0], oc1[0], oc1[0]};
	float wvx2[4] = {oc2[0], oc2[0], oc2[0], oc2[0]};
	float wvx3[4] = {oc3[0], oc3[0], oc3[0], oc3[0]};

	float wvy0[4] = {oc0[1], oc0[1], oc0[1], oc0[1]};
	float wvy1[4] = {oc1[1], oc1[1], oc1[1], oc1[1]};
	float wvy2[4] = {oc2[1], oc2[1], oc2[1], oc2[1]};
	float wvy3[4] = {oc3[1], oc3[1], oc3[1], oc3[1]};

	//cross products
	float cp0[4], cp1[4], cp2[4], cp3[4];
	//4 way cross product, times 4
	transCross(cp0, pbX0, pbY0, wvx0, wvy0);
	transCross(cp1, pbX1, pbY1, wvx1, wvy1);
	transCross(cp2, pbX2, pbY2, wvx2, wvy2);
	transCross(cp3, pbX3, pbY3, wvx3, wvy3);

	//do they all have the same sign?
	bool cc = 	(cp0[0] < 0.f && cp0[1] < 0.f && cp0[2] < 0.f && cp0[3] < 0.f &&
				cp1[0] < 0.f && cp1[1] < 0.f && cp1[2] < 0.f && cp1[3] < 0.f &&
				cp2[0] < 0.f && cp2[1] < 0.f && cp2[2] < 0.f && cp2[3] < 0.f &&
				cp3[0] < 0.f && cp3[1] < 0.f && cp3[2] < 0.f && cp3[3] < 0.f) ||
				(cp0[0] > 0.f && cp0[1] > 0.f && cp0[2] > 0.f && cp0[3] > 0.f &&
				cp1[0] > 0.f && cp1[1] > 0.f && cp1[2] > 0.f && cp1[3] > 0.f &&
				cp2[0] > 0.f && cp2[1] > 0.f && cp2[2] > 0.f && cp2[3] > 0.f &&
				cp3[0] > 0.f && cp3[1] > 0.f && cp3[2] > 0.f && cp3[3] > 0.f);

	//is z out of screen?
	bool isInFront = (zmax[0] < ozzzz[0]) && (zmax[1] < ozzzz[1]) && (zmax[2] < ozzzz[2]) && (zmax[3] < ozzzz[3]);
	bool notBehind  = (ozzzz[0] > zmin[0]) || (ozzzz[1] > zmin[1]) || (ozzzz[2] > zmin[2]) || (ozzzz[3] > zmin[3]);

	*inFront = isInFront;
	*inside = cc;
	*behind = !notBehind;
}

uint32_t cullOccludedTriangles(EdgeGeomSpuContext *ctx, const uint32_t indexCount, const uint32_t occluderCount, float occluderData[8*4*4], int32_t indexBias, EdgeGeomCullingResults *detailedResults)
{
    const uint32_t unroundedVertexCount = ctx->spuConfigInfo.numVertexes;
	const uint32_t vertexCount = (unroundedVertexCount + 3) & ~3;
	float *pUniform = ctx->uniformTables[ctx->spuConfigInfo.flagsAndUniformTableCount & 0xF];

	float qmin[3] = {FLT_MAX, FLT_MAX, FLT_MAX};
	float qmax[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

	//1) homogenize vertices because the transform call in edgeGeomCullOccludedTriangles doesn't
	float *vIter = pUniform;
	for (uint32_t i = 0; i < vertexCount; i++) {
		float absw = fabsf(vIter[3]);
		vIter[0] /= absw;
		vIter[1] /= absw;
		vIter[2] /= absw;
		vIter[3] = 0.f;

		//we write out to the extra verts in the intrinsics code, but we don't consider them in the BB
		if (i < unroundedVertexCount) {
			qmin[0] = vIter[0] < qmin[0] ? vIter[0] : qmin[0];
			qmin[1] = vIter[1] < qmin[1] ? vIter[1] : qmin[1];
			qmin[2] = vIter[2] < qmin[2] ? vIter[2] : qmin[2];
	
			qmax[0] = vIter[0] > qmax[0] ? vIter[0] : qmax[0];
			qmax[1] = vIter[1] > qmax[1] ? vIter[1] : qmax[1];
			qmax[2] = vIter[2] > qmax[2] ? vIter[2] : qmax[2];
		}

		vIter += 4;
	}

	// Loop #2: Set a bit in the Nth bit of the W for each vertex if the vertex is behind the occluder
	uint32_t occmask = 1;

	uint32_t occluded = 0;
	uint32_t fullyHidden = 0;

    const uint32_t numTriangles = indexCount / 3;

	for (uint32_t iOcc = 0; iOcc < occluderCount; iOcc++) {
		float *occluder = &occluderData[16 * iOcc];
		float *ov0 = occluder;
		float *ov1 = occluder + 4;
		float *ov2 = occluder + 8;
		float *ov3 = occluder + 12;

		float oc0[4] = {ov1[0] - ov0[0], ov1[1] - ov0[1], ov1[2] - ov0[2], 0.f};
		float oc1[4] = {ov2[0] - ov1[0], ov2[1] - ov1[1], ov2[2] - ov1[2], 0.f};
		float oc2[2/*4*/] = {ov3[0] - ov2[0], ov3[1] - ov2[1]};//, ov3[2] - ov2[2], 0.f};
		float oc3[2/*4*/] = {ov0[0] - ov3[0], ov0[1] - ov3[1]};//, ov0[2] - ov3[2], 0.f};

		float normal[3];
		cross3f(normal, oc0, oc1);
		union {float f; uint32_t u32;} un;
		un.f = normal[2];
		const uint32_t zsat = ((int32_t)un.u32) >> 31;

		//bounding box test
		uint32_t behind = 0, inside = 0, inFront = 0;
		bbTest(ov0, ov1, ov2, ov3,
			   oc0, oc1, oc2, oc3,
			   qmin, qmax,
			   &inFront, &behind, &inside);

		if (behind && inside) {
#ifdef EDGE_GEOM_DEBUG
            if (detailedResults != 0) {
                detailedResults->numOccludedTriangles += numTriangles;
                detailedResults->totalNumCulledTriangles += numTriangles;
            }
#endif
			return 0;
		}

		uint32_t fullyHiddenBit = occmask;

		//if the mesh isn't in front of the occluder
		if (!inFront) {
			for (uint32_t iVert = 0; iVert < vertexCount; iVert++) {
				float *v = pUniform + 4 * iVert;
				//subtract off the occluder corners
				float dv0[3] = {v[0] - ov0[0], v[1] - ov0[1], v[2] - ov0[2]};
				float dv1[2] = {v[0] - ov1[0], v[1] - ov1[1]};
				float dv2[2] = {v[0] - ov2[0], v[1] - ov2[1]};
				float dv3[2] = {v[0] - ov3[0], v[1] - ov3[1]};
	
				//cross products to determine if the verts are inside the quad
				uint32_t c0 = cross2u(dv0, oc0);
				uint32_t c1 = cross2u(dv1, oc1);
				uint32_t c2 = cross2u(dv2, oc2);
				uint32_t c3 = cross2u(dv3, oc3);
	
				const uint32_t cc = (c0 & c1 & c2 & c3) | (~c0 & ~c1 & ~c2 & ~c3);
				const uint32_t inside = (uint32_t)(((int32_t)cc)>>31);
	
				const uint32_t dp = dot3u(dv0, normal);
				const uint32_t bitsat = ((int32_t)dp) >> 31;
				const uint32_t behind = (bitsat & zsat) | (~bitsat & ~zsat);
	
				const uint32_t hidden = inside & behind;
				const uint32_t hidebit = hidden & occmask;
				((uint32_t *)v)[3] |= hidebit;
	
				fullyHiddenBit &= hidebit;
				occluded |= hidebit;
			}
		}
		else{
			fullyHiddenBit = 0;
		}

		fullyHidden |= fullyHiddenBit;
		occmask <<= 1;
	}

	//if all triangles are hidden by a single occluder
	if (fullyHidden) {
#ifdef EDGE_GEOM_DEBUG
        if (detailedResults != 0) {
            detailedResults->numOccludedTriangles += numTriangles;
            detailedResults->totalNumCulledTriangles += numTriangles;
        }
#endif
		return 0;
	}

	const uint32_t numIndexes = indexCount;
	
	if (!occluded)
	{ 
		//if no vertices are occluded, don't move the free pointer
        //and nothing to update in the culling results
		return numIndexes;
	}
	else if (numIndexes == 0) {
#ifdef EDGE_GEOM_DEBUG        
        if (detailedResults != 0) {
            detailedResults->numOccludedTriangles += numTriangles;
            detailedResults->totalNumCulledTriangles += numTriangles;
        }
#endif
		return 0;
	}

    uint16_t *pStartingIndices = (uint16_t *)ctx->indexesLs;
	uint16_t *pIndices = pStartingIndices;
	uint16_t *pOutIndexes = pIndices;

    float *pVertices = pUniform;
    pVertices = (float *)((uint32_t)pVertices + indexBias * sizeof(qword));

	for (uint32_t iTriangle = 0; iTriangle < numTriangles; iTriangle++) {
		uint16_t index0 = pIndices[0];
		uint16_t index1 = pIndices[1];
		uint16_t index2 = pIndices[2];

		uint32_t *v0 = (uint32_t *)(pVertices + 4 * index0);
		uint32_t *v1 = (uint32_t *)(pVertices + 4 * index1);
		uint32_t *v2 = (uint32_t *)(pVertices + 4 * index2);

		uint32_t cull = v0[3] & v1[3] & v2[3];

		if (!cull) {
			pOutIndexes[0] = index0;
			pOutIndexes[1] = index1;
			pOutIndexes[2] = index2;
			pOutIndexes += 3;
		}

		pIndices += 3;
	}

	uint32_t toRet = pOutIndexes - pStartingIndices;
#ifdef EDGE_GEOM_DEBUG
    if (detailedResults != 0) {
        const uint32_t culledTriangles = numTriangles - (toRet / 3);
        detailedResults->numOccludedTriangles += culledTriangles;
        detailedResults->totalNumCulledTriangles += culledTriangles;
    }
#endif
	(void)detailedResults; // avoids "unreferenced parameter" warning in non-debug builds
	return toRet;
}
#endif

#if !EDGEGEOMCULLOCCLUDEDTRIANGLES_INTRINSICS
uint32_t edgeGeomCullOccludedTriangles(EdgeGeomSpuContext * const ctx, const uint32_t occluderCount, const uint32_t occludersEa, int32_t indexBias, EdgeGeomCullingResults *detailedResults)
{
	EDGE_ASSERT_MSG(occluderCount<=8, ("Edge error: the number of occluders must be less than or equal to 8\n") );

	// Start the DMA of the occluders:
	float s_occluderData[8*(4*4)] __attribute__((__aligned__(16))); // up to 8 occluders (512B)
	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;

	spu_idisable();
	EDGE_DMA_GET(&s_occluderData[0], occludersEa, occluderCount*16*4, ctx->inputDmaTag, 0, 0);
	if( interruptsEnabled )
		spu_ienable();

	float localToWorldMatrix[16];
	float viewProjectionMatrix[16];
	for (uint32_t i = 0; i < 16; i++) {
		localToWorldMatrix[i] = (i < 12) ? ctx->localToWorldMatrix.matrixData[i] : 0.0f;
		viewProjectionMatrix[i] = ctx->viewportInfo.viewProjectionMatrix[i];
	}
	localToWorldMatrix[15] = 1.0f;

	float localToProjMatrix[16];
	matrixMul(localToProjMatrix, viewProjectionMatrix, localToWorldMatrix);

	// Project the vertices in screen space:
	void * const pVertices = ctx->positionTable;
	void * const pVertTemp = ctx->uniformTables[ctx->spuConfigInfo.flagsAndUniformTableCount & 0xF];
	edgeGeomTransformVertexes(ctx->spuConfigInfo.numVertexes, pVertices, pVertTemp, localToProjMatrix);

	// Wait to make sure the occluders data arrived.
	spu_idisable();
	EDGE_DMA_WAIT_TAG_STATUS_ALL(1 << ctx->inputDmaTag);
	if ( interruptsEnabled )
		spu_ienable();

	//project the occluders to the screen
	float *pOccluder = s_occluderData;
	for (uint32_t i = 0; i < occluderCount; i++) {
		//transpose pOccluders
		float trans[16] = {	pOccluder[0], pOccluder[4], pOccluder[8], pOccluder[12],
							pOccluder[1], pOccluder[5], pOccluder[9], pOccluder[13],
							pOccluder[2], pOccluder[6], pOccluder[10], pOccluder[14],
							pOccluder[3], pOccluder[7], pOccluder[11], pOccluder[15]};
		matrixMul(trans, viewProjectionMatrix, trans);

		//divide by W to homogenize
		pOccluder[0] = trans[0] / trans[12];
		pOccluder[1] = trans[4] / trans[12];
		pOccluder[2] = trans[8] / trans[12];
		pOccluder[3] = 0.f;

		pOccluder[4] = trans[1] / trans[13];
		pOccluder[5] = trans[5] / trans[13];
		pOccluder[6] = trans[9] / trans[13];
		pOccluder[7] = 0.f;

		pOccluder[8] = trans[2] / trans[14];
		pOccluder[9] = trans[6] / trans[14];
		pOccluder[10] = trans[10] / trans[14];
		pOccluder[11] = 0.f;

		pOccluder[12] = trans[3] / trans[15];
		pOccluder[13] = trans[7] / trans[15];
		pOccluder[14] = trans[11] / trans[15];
		pOccluder[15] = 0.f;

		pOccluder += 16;
	}

	const uint32_t remainingIndices = cullOccludedTriangles(ctx, edgeGeomGetIndexCount(ctx), occluderCount, s_occluderData, indexBias, detailedResults);

	edgeGeomSetIndexCount(ctx, remainingIndices);
	edgeGeomSetFreePtr(ctx, (void *)((  ((uint32_t)((uint8_t *)ctx->indexesLs + 2*remainingIndices)) + 0xf) & ~0xf));

	return remainingIndices;
}
#endif
