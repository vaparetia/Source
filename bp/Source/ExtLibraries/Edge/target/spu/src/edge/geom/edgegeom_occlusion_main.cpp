/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/edge_assert.h"
#include "edge/geom/edgegeom.h"
#include "edge/geom/edgegeom_internal.h"
#include "edge/geom/edgegeom_masks.h"


#if EDGEGEOMCULLOCCLUDEDTRIANGLES_INTRINSICS
uint32_t edgeGeomCullOccludedTriangles(EdgeGeomSpuContext * const ctx, const uint32_t occluderCount, const uint32_t occludersEa, int32_t indexBias, EdgeGeomCullingResults *detailedResults)
{
    if (occluderCount == 0)
		return edgeGeomGetIndexCount(ctx);

	EDGE_ASSERT_MSG(occluderCount<=8, ("Edge error: the number of occluders must be less than or equal to 8\n") );

	// Start the DMA of the occluders:
	float s_occluderData[8*(4*4)] __attribute__((__aligned__(16))); // up to 8 occluders (512B)
	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	
	spu_idisable();
	EDGE_DMA_GET(&s_occluderData[0], occludersEa, occluderCount*16*4, ctx->inputDmaTag, 0, 0);
	if( interruptsEnabled )
		spu_ienable();


	const qword s_AAAA = si_ila(0x10203);
	const qword s_BBBB = si_orbi(s_AAAA, 0x04);
	const qword s_CCCC = si_orbi(s_AAAA, 0x08);
	const qword s_DDDD = si_orbi(s_AAAA, 0x0C);
	const qword s_CcDd = si_rotqbyi((qword)s_DdCc, 8);
	const qword s_BbAa = si_rotqbyi((qword)s_AaBb, 8);
	const qword s_ABcd = si_rotqbyi((qword)s_cdAB, 8);

	// Get the projection matrix:
	const qword viewProjectionMatrix = si_from_ptr(ctx->viewportInfo.viewProjectionMatrix);
	const qword viewProjMat0         = si_lqd(viewProjectionMatrix, 0x00);
	const qword viewProjMat1         = si_lqd(viewProjectionMatrix, 0x10);
	const qword viewProjMat2         = si_lqd(viewProjectionMatrix, 0x20);
	const qword viewProjMat3         = si_lqd(viewProjectionMatrix, 0x30);

	// Compute LocalToProj matrix from LocalToWorld and ViewportProjection matrices
	const qword w2hdc00 = si_shufb(viewProjMat0, viewProjMat0, s_AAAA);
	const qword w2hdc10 = si_shufb(viewProjMat1, viewProjMat1, s_AAAA);
	const qword w2hdc20 = si_shufb(viewProjMat2, viewProjMat2, s_AAAA);
	const qword w2hdc30 = si_shufb(viewProjMat3, viewProjMat3, s_AAAA);

	const qword w2hdc01 = si_shufb(viewProjMat0, viewProjMat0, s_BBBB);
	const qword w2hdc11 = si_shufb(viewProjMat1, viewProjMat1, s_BBBB);
	const qword w2hdc21 = si_shufb(viewProjMat2, viewProjMat2, s_BBBB);
	const qword w2hdc31 = si_shufb(viewProjMat3, viewProjMat3, s_BBBB);

	const qword w2hdc02 = si_shufb(viewProjMat0, viewProjMat0, s_CCCC);
	const qword w2hdc12 = si_shufb(viewProjMat1, viewProjMat1, s_CCCC);
	const qword w2hdc22 = si_shufb(viewProjMat2, viewProjMat2, s_CCCC);
	const qword w2hdc32 = si_shufb(viewProjMat3, viewProjMat3, s_CCCC);

	const qword w2hdc03 = si_shufb(viewProjMat0, viewProjMat0, s_DDDD);
	const qword w2hdc13 = si_shufb(viewProjMat1, viewProjMat1, s_DDDD);
	const qword w2hdc23 = si_shufb(viewProjMat2, viewProjMat2, s_DDDD);
	const qword w2hdc33 = si_shufb(viewProjMat3, viewProjMat3, s_DDDD);

	const qword k1_0f    = si_ilhu(0x3F80);

	const qword sel_00FF = si_fsmbi(0x00FF);
	const qword defw     = si_il(0);


	//
	// Loop #1: Transform each vertices into screen space
	//

	float o2hdc_mat[16] __attribute__((__aligned__(16)));
	const qword o2hdcptr = si_from_ptr(o2hdc_mat);

	// Compute the object to hdc matrix:
	const qword o2w  = si_from_ptr(&(ctx->localToWorldMatrix.matrixData));
	{
		const qword o2w0 = si_lqd(o2w, 0x00);
		const qword o2w1 = si_lqd(o2w, 0x10);
		const qword o2w2 = si_lqd(o2w, 0x20);
		const qword o2w3 = (qword)(vec_float4){0.0f,0.0f,0.0f,1.0f};
	
		const qword o2hdc0tmp0 = si_fm(w2hdc00, o2w0);
		const qword o2hdc1tmp0 = si_fm(w2hdc10, o2w0);
		const qword o2hdc2tmp0 = si_fm(w2hdc20, o2w0);
		const qword o2hdc3tmp0 = si_fm(w2hdc30, o2w0);
		
		const qword o2hdc0tmp1 = si_fma(w2hdc01, o2w1, o2hdc0tmp0);
		const qword o2hdc1tmp1 = si_fma(w2hdc11, o2w1, o2hdc1tmp0);
		const qword o2hdc2tmp1 = si_fma(w2hdc21, o2w1, o2hdc2tmp0);
		const qword o2hdc3tmp1 = si_fma(w2hdc31, o2w1, o2hdc3tmp0);

		const qword o2hdc0tmp2 = si_fma(w2hdc02, o2w2, o2hdc0tmp1);
		const qword o2hdc1tmp2 = si_fma(w2hdc12, o2w2, o2hdc1tmp1);
		const qword o2hdc2tmp2 = si_fma(w2hdc22, o2w2, o2hdc2tmp1);
		const qword o2hdc3tmp2 = si_fma(w2hdc32, o2w2, o2hdc3tmp1);

		const qword o2hdc0 = si_fma(w2hdc03, o2w3, o2hdc0tmp2);
		const qword o2hdc1 = si_fma(w2hdc13, o2w3, o2hdc1tmp2);
		const qword o2hdc2 = si_fma(w2hdc23, o2w3, o2hdc2tmp2);
		const qword o2hdc3 = si_fma(w2hdc33, o2w3, o2hdc3tmp2);

		si_stqd(o2hdc0, o2hdcptr, 0x00);
		si_stqd(o2hdc1, o2hdcptr, 0x10);
		si_stqd(o2hdc2, o2hdcptr, 0x20);
		si_stqd(o2hdc3, o2hdcptr, 0x30);
	}

	// Project the vertices in screen space:
	void * const pVertices = ctx->positionTable;
	void * const pVertTemp = ctx->uniformTables[ctx->spuConfigInfo.flagsAndUniformTableCount & 0xF];
	edgeGeomTransformVertexes(ctx->spuConfigInfo.numVertexes, pVertices, pVertTemp, o2hdc_mat);

	// Wait to make sure the occluders data arrived.
	spu_idisable();
	EDGE_DMA_WAIT_TAG_STATUS_ALL(1 << ctx->inputDmaTag);
	if ( interruptsEnabled )
		spu_ienable();

	char * poccData = (char*)s_occluderData;

	//
	// Loop #2: Transform each occluder into screen space
	//

	// Project the occluders to screen:
	for ( uint32_t ondx = 0; ondx < occluderCount; ondx++, poccData+=0x40)
	{
		const qword pOccluders = si_from_ptr(poccData);

		const qword ioccluder0 = si_lqd(pOccluders, 0x00);
		const qword ioccluder1 = si_lqd(pOccluders, 0x10);
		const qword ioccluder2 = si_lqd(pOccluders, 0x20);
		const qword ioccluder3 = si_lqd(pOccluders, 0x30);

		const qword tmp00 = si_shufb(ioccluder0, ioccluder1, (qword)s_AaBb);
		const qword tmp20 = si_shufb(ioccluder0, ioccluder1, (qword)s_CcDd);
		const qword tmp10 = si_shufb(ioccluder2, ioccluder3, (qword)s_BbAa);
		const qword tmp30 = si_shufb(ioccluder2, ioccluder3, (qword)s_DdCc);
		
		const qword xxxx0 = si_shufb(tmp00, tmp10, (qword)s_ABcd);
		const qword yyyy0 = si_shufb(tmp10, tmp00, (qword)s_cdAB);
		const qword zzzz0 = si_shufb(tmp20, tmp30, (qword)s_ABcd);


		const qword tmp01 = si_fma(xxxx0, w2hdc00, w2hdc03);
		const qword tmp11 = si_fma(xxxx0, w2hdc10, w2hdc13);
		const qword tmp21 = si_fma(xxxx0, w2hdc20, w2hdc23);
		const qword tmp31 = si_fma(xxxx0, w2hdc30, w2hdc33);

		const qword tmp02 = si_fma(yyyy0, w2hdc01, tmp01);
		const qword tmp12 = si_fma(yyyy0, w2hdc11, tmp11);
		const qword tmp22 = si_fma(yyyy0, w2hdc21, tmp21);
		const qword tmp32 = si_fma(yyyy0, w2hdc31, tmp31);

		const qword xxxx1 = si_fma(zzzz0, w2hdc02, tmp02);
		const qword yyyy1 = si_fma(zzzz0, w2hdc12, tmp12);
		const qword zzzz1 = si_fma(zzzz0, w2hdc22, tmp22);
		const qword wwww1 = si_fma(zzzz0, w2hdc32, tmp32);


		const qword recipA = si_frest(wwww1);
		const qword recipB = si_fi(wwww1, recipA);
		const qword recipC = si_fnms(recipB, wwww1, k1_0f);
		const qword ooW    = si_fma(recipC, recipB, recipB);

		const qword xxxx2 = si_fm(xxxx1, ooW);
		const qword yyyy2 = si_fm(yyyy1, ooW);
		const qword zzzz2 = si_fm(zzzz1, ooW);

		const qword tmp03 = si_shufb(xxxx2, yyyy2, (qword)s_AaBb);
		const qword tmp13 = si_shufb(zzzz2, defw,  (qword)s_BbAa);
		const qword tmp23 = si_shufb(zzzz2, defw,  (qword)s_DdCc);
		const qword tmp33 = si_shufb(xxxx2, yyyy2, (qword)s_CcDd);

		const qword hvw0 = si_selb (tmp03, tmp13, sel_00FF);
		const qword hvw1 = si_shufb(tmp13, tmp03, (qword)s_cdAB);
		const qword hvw2 = si_selb (tmp33, tmp23, sel_00FF);
		const qword hvw3 = si_shufb(tmp23, tmp33, (qword)s_cdAB);

		si_stqd(hvw0, pOccluders, 0x00);
		si_stqd(hvw1, pOccluders, 0x10);
		si_stqd(hvw2, pOccluders, 0x20);
		si_stqd(hvw3, pOccluders, 0x30);
	}

	const uint32_t remainingIndices = cullOccludedTriangles(ctx, edgeGeomGetIndexCount(ctx), occluderCount, s_occluderData, indexBias, detailedResults);

	edgeGeomSetIndexCount(ctx, remainingIndices);
	edgeGeomSetFreePtr(ctx, (void *)((  ((uint32_t)((uint8_t *)ctx->indexesLs + 2*remainingIndices)) + 0xf) & ~0xf));

	return remainingIndices;

}
#endif
