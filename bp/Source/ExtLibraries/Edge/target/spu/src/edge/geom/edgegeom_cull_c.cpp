/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <cell/gcm_spu.h>
#include "edge/geom/edgegeom.h"
#include "edge/geom/edgegeom_internal.h"

#if !TRANSFORMVERTEXESFORCULL_INTRINSICS
void transformVertexesForCull(EdgeGeomSpuContext *ctx)
{
	//Decompresses 4 at a time
	float *pInput = (float *)ctx->positionTable;
	float *pTransVerts = (float*)ctx->uniformTables[ctx->spuConfigInfo.flagsAndUniformTableCount & 0xF];

	const uint32_t numIterations = (ctx->spuConfigInfo.numVertexes + 3) & ~3;

	const uint16_t *pScissorArea = ctx->viewportInfo.scissorArea;
	const uint32_t scissorXMaxi = pScissorArea[0] + pScissorArea[2];
	const uint32_t scissorYMaxi = pScissorArea[1] + pScissorArea[3];
	//convert to float
	const float frustumMaxX = (float)scissorXMaxi;
	const float frustumMaxY = (float)scissorYMaxi;
	const float frustumMinX = (float)pScissorArea[0];
	const float frustumMinY = (float)pScissorArea[1];

	const float *depthRange = ((float*)pScissorArea) + 2;
	const bool flipDepthRange = depthRange[0] > depthRange[1];
	const float frustumMaxZ = (flipDepthRange) ? depthRange[0] : depthRange[1];
	const float frustumMinZ = (flipDepthRange) ? depthRange[1] : depthRange[0];

	float localToWorldMatrix[16];
	float viewProjectionMatrix[16];
	for (uint32_t i = 0; i < 16; i++) {
		localToWorldMatrix[i] = (i < 12) ? ctx->localToWorldMatrix.matrixData[i] : 0.0f;
		viewProjectionMatrix[i] = ctx->viewportInfo.viewProjectionMatrix[i];
	}
	localToWorldMatrix[15] = 1.0f;

	float localToProjMatrix[16];
	matrixMul(localToProjMatrix, viewProjectionMatrix, localToWorldMatrix);

	
	float viewportScaleX  = ctx->viewportInfo.viewportScales[0];
	float viewportScaleY  = ctx->viewportInfo.viewportScales[1];
	float viewportScaleZ  = ctx->viewportInfo.viewportScales[2];
	float viewportOffsetX = ctx->viewportInfo.viewportOffsets[0];
	float viewportOffsetY = ctx->viewportInfo.viewportOffsets[1];
	float viewportOffsetZ = ctx->viewportInfo.viewportOffsets[2];

	// Each output qword contains the following data:
	// word 0: screen-space X coordinate (floating-point)
	// word 1: screen-space Y coordinate (floating-point)
	// word 2: flags
	//         word2 & (1<<0): unused (always zero)
	//         word2 & (1<<1): wSafe   (set if the vertex's projected W value was > 0)
	//         word2 & (1<<2): Z Under (set if the vertex was below the negative Z plane)
	//         word2 & (1<<3): Z Over  (set if the vertex was above the positive Z plane)
	//         word2 & (1<<4): Y Under (set if the vertex was below the negative Y plane)
	//         word2 & (1<<5): Y Over  (set if the vertex was above the positive Y plane)
	//         word2 & (1<<6): X Under (set if the vertex was below the negative X plane)
	//         word2 & (1<<7): X Over  (set if the vertex was above the positive X plane)
	// word3: unused (always zero)
	for (uint32_t iVertex = 0; iVertex < numIterations; iVertex++) {
        //transform 
		float transX = localToProjMatrix[0] * pInput[0] + localToProjMatrix[1] * pInput[1] + localToProjMatrix[2] * pInput[2] + localToProjMatrix[3];
		float transY = localToProjMatrix[4] * pInput[0] + localToProjMatrix[5] * pInput[1] + localToProjMatrix[6] * pInput[2] + localToProjMatrix[7];
		float transZ = localToProjMatrix[8] * pInput[0] + localToProjMatrix[9] * pInput[1] + localToProjMatrix[10] * pInput[2] + localToProjMatrix[11];
		float transW = localToProjMatrix[12] * pInput[0] + localToProjMatrix[13] * pInput[1] + localToProjMatrix[14] * pInput[2] + localToProjMatrix[15];

		transX /= transW;
		transY /= transW;
		transZ /= transW;

		if (transW < 0.0f) {
			transX *= -1.0f;
			transY *= -1.0f;
			transZ *= -1.0f;
		}

		transX = (transX * viewportScaleX) + viewportOffsetX;
		transY = (transY * viewportScaleY) + viewportOffsetY;
		transZ = (transZ * viewportScaleZ) + viewportOffsetZ;

		const uint32_t overX = (transX > frustumMaxX) ? 1 : 0;
		const uint32_t overY = (transY > frustumMaxY) ? 1 : 0;
		const uint32_t overZ = (transZ > frustumMaxZ) ? 1 : 0;

		const uint32_t underX = (frustumMinX > transX) ? 1 : 0;
		const uint32_t underY = (frustumMinY > transY) ? 1 : 0;
		const uint32_t underZ = (frustumMinZ > transZ) ? 1 : 0;

		// test w against 0 (which will invalidate some tests)
		const uint32_t wIsSafe = (transW > 0.0f) ? 1 : 0;
		const uint32_t flags =
			(wIsSafe << 1) |
			(underZ  << 2) |
			(overZ   << 3) |
			(underY  << 4) |
			(overY   << 5) |
			(underX  << 6) |
			(overX   << 7);
		
		//store
		pTransVerts[0] = transX;
		pTransVerts[1] = transY;
		((uint32_t *)pTransVerts)[2] = flags;
		((uint32_t *)pTransVerts)[3] = 0;

		pInput += 4;
		pTransVerts += 4;
	}
}
#endif

#if !EDGEGEOMCULLTRIANGLES_INTRINSICS
uint32_t edgeGeomCullTriangles(EdgeGeomSpuContext *ctx, uint32_t cullingFlavor, int32_t indexBias,
	EdgeGeomCullingResults *detailedResults)
{
	if(cullingFlavor == EDGE_GEOM_CULL_NONE)
	{
		// If the indexesOffset isn't -1, then the index buffer should
		// be considered "constant" (we can't modify or output it),
		// and we can reclaim that space for other calculations.
		if (ctx->spuConfigInfo.indexesOffset != 0xFFFFFFFF) {
			edgeGeomSetFreePtr(ctx, (void*)ctx->indexesLs);
		}
		return ctx->numVisibleIndexes;
	}

	// Culling uses a spare uniform table for intermediate
	// calculations.  If the spare uniform table bit is not set in the
	// SpuConfigInfo flags, then the spare uniform table does not
	// exist!  This is an error!
	if ((ctx->spuConfigInfo.flagsAndUniformTableCount & EDGE_GEOM_FLAG_INCLUDES_EXTRA_UNIFORM_TABLE) == 0)
	{
		EDGE_PRINTF("ERROR: attempt to use culling without allocating an extra uniform table!\n");
		return ctx->numVisibleIndexes;
	}

    if (ctx->transformForCullCallbackInfo.transformCallback) {
        ctx->transformForCullCallbackInfo.transformCallback(ctx, ctx->transformForCullCallbackInfo.transformCallbackUserData);
    }
    else{
        transformVertexesForCull(ctx);
    }

	//intrinsics code
	//does 4 triangles at a time
	const uint32_t nearMask = (1<<2); // corresponds to the Z Under frustum test

	float *pVertices = (float *)ctx->uniformTables[ctx->spuConfigInfo.flagsAndUniformTableCount & 0xF];

    pVertices = (float *)((uint32_t)pVertices + indexBias * sizeof(qword));

    const uint32_t numIterations = ((ctx->numVisibleIndexes / 3));

	bool flipBit = true;
	bool useFace = false;

	const uint8_t vpSampleFlavor = ctx->viewportInfo.sampleFlavor;
	const bool useRgms = vpSampleFlavor == CELL_GCM_SURFACE_SQUARE_ROTATED_4;

	float pixelCenterBias =  0.5f;
	float pixelCenterScale = 1.0f;
	switch(ctx->viewportInfo.sampleFlavor)
	{
	case CELL_GCM_SURFACE_CENTER_1:				pixelCenterScale = 1.0f; break;
	case CELL_GCM_SURFACE_DIAGONAL_CENTERED_2:	
	case CELL_GCM_SURFACE_SQUARE_CENTERED_4:	pixelCenterScale = 2.0f; break;
	case CELL_GCM_SURFACE_SQUARE_ROTATED_4:		break; // irrelevant for 4xRGMS
	}

	switch (cullingFlavor) 
	{
	case EDGE_GEOM_CULL_BACKFACES_AND_FRUSTUM: //BF and Frustum
		flipBit = !flipBit;
		/* intentional fall-through */
	case EDGE_GEOM_CULL_FRONTFACES_AND_FRUSTUM: //FF and Frustum
		useFace = true;
		/* intentional fall-through */
	case EDGE_GEOM_CULL_FRUSTUM:
		uint32_t indexesFlavor = (ctx->spuConfigInfo.indexesFlavorAndSkinningFlavor >> 4) & 0xF;
		flipBit = 	(indexesFlavor == EDGE_GEOM_INDEXES_U16_TRIANGLE_LIST_CW ||
					 indexesFlavor == EDGE_GEOM_INDEXES_COMPRESSED_TRIANGLE_LIST_CW) ? 
					!flipBit : flipBit;
	}

	uint16_t *pIndices = (uint16_t *)(ctx->indexesLs);
	uint16_t *pStartingIndices = pIndices;
	uint16_t *pOutIndexes = pIndices;

	for (uint32_t iTriangle = 0; iTriangle < numIterations; iTriangle++) {
        uint16_t index0 = pIndices[0];
		uint16_t index1 = pIndices[1];
		uint16_t index2 = pIndices[2];

		float *v0 = pVertices + 4 * index0;
		float *v1 = pVertices + 4 * index1;
		float *v2 = pVertices + 4 * index2;

		//frustum test
		uint32_t cullFlags0 = (((uint32_t *)v0)[2] & ((uint32_t *)v1)[2]) & ((uint32_t *)v2)[2];
		bool wSafe = (cullFlags0 & (1<<1));
		uint32_t cullFrustum0Tmp1 = (cullFlags0 & 0xFC);
		bool cullFrustum0 = (cullFrustum0Tmp1 > (uint32_t)0);

		uint32_t crossesNear0 = (((uint32_t *)v0)[2] | ((uint32_t *)v1)[2]) | ((uint32_t *)v2)[2];
		bool doesNotIntersectNear0 = (crossesNear0 & nearMask) == 0;

		float e00[2] = {((float *)v1)[0] - ((float *)v0)[0], ((float *)v1)[1] - ((float *)v0)[1]};
		float e01[2] = {((float *)v2)[0] - ((float *)v0)[0], ((float *)v2)[1] - ((float *)v0)[1]};

		float cross = e00[0] * e01[1] - e00[1] * e01[0];

		cross *= flipBit ? -1.f : 1.f;
		bool isBackFacing = cross > 0.f;
		bool passesNearPlaneTest = doesNotIntersectNear0 && wSafe;
		bool cullBackFace = (isBackFacing && passesNearPlaneTest);

		//RGMS test
		//get bb
		bool noPixel = false;
		if (useRgms)
		{
			const int32_t rgmsX[16] = {   1, 0, 3, 2,
									3, 1, 1, 0,
									0, 3, 2, 1,
									1, 1, 0, 2};

			const int32_t rgmsY[16] = {   2, 0, 1, 1,
									1, 3, 2, 0,
									0, 1, 1, 3,
									3, 1, 0, 2};

			float tmpX0 = v0[0] > v1[0] ? v0[0] : v1[0];
			float maxX = tmpX0 > v2[0] ? tmpX0 : v2[0];
	
			float tmpY0 = v0[1] > v1[1] ? v0[1] : v1[1];
			float maxY = tmpY0 > v2[1] ? tmpY0 : v2[1];
	
			float tmpX1 = v0[0] > v1[0] ? v1[0] : v0[0];
			float minX = tmpX1 > v2[0] ? v2[0] : tmpX1;
	
			float tmpY1 = v0[1] > v1[1] ? v1[1] : v0[1];
			float minY = tmpY1 > v2[1] ? v2[1] : tmpY1;
	
			int32_t maxXQuant = (int32_t)(maxX * (float)(1 << 2));
			int32_t maxYQuant = (int32_t)(maxY * (float)(1 << 2));

			int32_t minXQuant = (int32_t)(minX * (float)(1 << 2));
			int32_t minYQuant = (int32_t)(minY * (float)(1 << 2));

			int32_t dxQuant = maxXQuant - minXQuant;
			int32_t dyQuant = maxYQuant - minYQuant;

			//int32_t yCoord = minYQuant << 2;
			//int32_t rgmsCoord = (minXQuant & 0x3) | (yCoord & 0xC);
			int32_t rgmsCoord = (minXQuant & 0x3);

			bool xMiss = rgmsX[rgmsCoord] > dxQuant;
			bool yMiss = rgmsY[rgmsCoord] > dyQuant;
			noPixel = xMiss && yMiss;
		}
		else
		{
			int32_t xInt0 = (int32_t)(v0[0] * pixelCenterScale + pixelCenterBias);
			int32_t xInt1 = (int32_t)(v1[0] * pixelCenterScale + pixelCenterBias);
			int32_t xInt2 = (int32_t)(v2[0] * pixelCenterScale + pixelCenterBias);
			int32_t yInt0 = (int32_t)(v0[1] * pixelCenterScale + pixelCenterBias);
			int32_t yInt1 = (int32_t)(v1[1] * pixelCenterScale + pixelCenterBias);
			int32_t yInt2 = (int32_t)(v2[1] * pixelCenterScale + pixelCenterBias);

			noPixel = (xInt0 == xInt1 && xInt1 == xInt2 && xInt2 == xInt0) ||
				(yInt0 == yInt1 && yInt1 == yInt2 && yInt2 == yInt0);
		}
		noPixel = noPixel && passesNearPlaneTest; // noPixel test is invalid if the triangle crosses the near plane

		bool cull0 = 	cullFrustum0 ||
			            noPixel ||
			            (useFace && cullBackFace);
		if (!cull0) {
			pOutIndexes[0] = index0;
			pOutIndexes[1] = index1;
			pOutIndexes[2] = index2;
			pOutIndexes += 3;
		}
#ifdef EDGE_GEOM_DEBUG
        else{
            if (detailedResults){
                if (cullFrustum0)
                    detailedResults->numOutsideFrustumTriangles++;
                if (noPixel) 
                    detailedResults->numNoPixelTriangles++;
                if (useFace && cullBackFace)
                    detailedResults->numBackFacingTriangles++;
                detailedResults->totalNumCulledTriangles++;
            }
		}
#endif
		(void)detailedResults; // avoids "unreferenced parameter" warning in non-debug builds
		pIndices += 3;
	}

	uint32_t toRet = pOutIndexes - pStartingIndices;
	ctx->numVisibleIndexes = toRet;
	edgeGeomSetFreePtr(ctx, (void *)((  ((uint32_t)((uint8_t *)ctx->indexesLs + 2*toRet)) + 0xf) & ~0xf));
	return toRet;
}
#endif
