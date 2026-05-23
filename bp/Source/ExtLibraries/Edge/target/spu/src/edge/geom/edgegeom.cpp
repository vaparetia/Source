/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <spu_intrinsics.h>
#include <cell/spurs/common.h>
#include <sdk_version.h>

#include "edge/geom/edgegeom.h"
#include "edge/geom/edgegeom_internal.h"
#include "edge/geom/edgegeom_masks.h"


/**
 *	Get a pointer to the start of free memory in the IOBuffer
 *  @param ctx EdgeGeom job context
 * 	@return the free pointer
 */
void *edgeGeomGetFreePtr(EdgeGeomSpuContext *ctx)
{
#ifdef EDGE_GEOM_DEBUG
	uintptr_t addr = (uintptr_t) ctx->freePtr;
	(void)addr;
	EDGE_ASSERT_MSG( (((addr<ctx->ioBufferBase) || (addr>=ctx->ioBufferEnd) || (addr&0x0F)) == false),
						("Edge error: IO buffer overflow in edgeGeomGetFreePtr = %06x [iobuffer=%06x-%06x]",
							addr, ctx->ioBufferBase, ctx->ioBufferEnd) );
#endif//EDGE_GEOM_DEBUG
	return ctx->freePtr;
}

/**
 * 	Sets the free pointer to a specified address.  Care should be
 *  taken not to set this below data needed by EdgeGeom.
 *  @param ctx EdgeGeom job context
 *  @param ptr the new free pointer
 */
void edgeGeomSetFreePtr(EdgeGeomSpuContext *ctx, const void *ptr)
{
#ifdef EDGE_GEOM_DEBUG
	uintptr_t addr = (uintptr_t) ptr;
	(void)addr;
	EDGE_ASSERT_MSG( (((addr<ctx->ioBufferBase) || (addr>ctx->ioBufferEnd) || (addr&0x0F)) == false),
						("Edge error: IO buffer overflow in edgeGeomSetFreePtr = %06x [iobuffer=%06x-%06x]", 
							addr, ctx->ioBufferBase, ctx->ioBufferEnd) );
#endif//EDGE_GEOM_DEBUG
	ctx->freePtr = (void*)ptr;
}


#ifdef EDGE_GEOM_DEBUG
uint32_t edgeGeomValidateBufferOrder(
	const void *pOutputStreamDesc, 
	const void *pIndexes, 
	const void *pSkinMatrices, 
	const void *pSkinWeights, 
	const void *pVertexesA, 
	const void *pVertexesB,	
	const void *pViewportInfo, 
	const void *pLocalToWorld, 
	const void *pSpuConfigInfo, 
	const void *pFixedOffsetsA, 
	const void *pFixedOffsetsB, 
	const void *pInputStreamDescA,
	const void *pInputStreamDescB)
{
	// cast input pointers to ints, so we can compare them.
	const uint32_t outputStreamDesc	= (uint32_t)pOutputStreamDesc;
	const uint32_t indexes			= (uint32_t)pIndexes;
	const uint32_t viewportInfo		= (uint32_t)pViewportInfo;
	const uint32_t localToWorld		= (uint32_t)pLocalToWorld;
	const uint32_t skinMatrices		= (uint32_t)pSkinMatrices;
	const uint32_t skinWeights		= (uint32_t)pSkinWeights;
	const uint32_t vertexesA		= (uint32_t)pVertexesA;
	const uint32_t vertexesB		= (uint32_t)pVertexesB;
	const uint32_t spuConfigInfo	= (uint32_t)pSpuConfigInfo;
	const uint32_t fixedOffsetsA	= (uint32_t)pFixedOffsetsA;
	const uint32_t fixedOffsetsB	= (uint32_t)pFixedOffsetsB;
	const uint32_t inputStreamDescA	= (uint32_t)pInputStreamDescA;
	const uint32_t inputStreamDescB	= (uint32_t)pInputStreamDescB;

	uint32_t errorCode = 0;

	// Remember that 0 is a perfectly valid pointer target on the SPU,
	// so we can't simply test the pointers against 0 to see if
	// they're valid. Instead, we use the SpuConfigInfo fields to
	// determine what data we're expecting.
	const EdgeGeomSpuConfigInfo *pSpuInfo = (const EdgeGeomSpuConfigInfo*)pSpuConfigInfo;
	bool hasCustomInputFormat = (pSpuInfo->inputVertexFormatId == 0xFF && 
		(inputStreamDescA != 0 || inputStreamDescB != 0));
	bool hasCustomOutputFormat = (pSpuInfo->outputVertexFormatId == 0xFF && outputStreamDesc != 0);
	bool isSkinning = ((pSpuInfo->indexesFlavorAndSkinningFlavor & 0xF) != EDGE_GEOM_SKIN_NONE);
	bool hasIndexes = (pSpuInfo->numIndexes > 0);
	bool isFastPath = (pSpuInfo->flagsAndUniformTableCount & EDGE_GEOM_FLAG_STATIC_GEOMETRY_FAST_PATH);

	// Test alignment of all buffers; they all need to be 16-byte aligned!
	if (hasCustomOutputFormat && (outputStreamDesc & 0xF))
		errorCode |= EDGE_GEOM_VALIDATE_ERROR_CUSTOM_OUTPUT_FORMAT;
	if (hasIndexes && (indexes & 0xF))
		errorCode |= EDGE_GEOM_VALIDATE_ERROR_INDEXES;
	if (isSkinning && (skinMatrices & 0xF))
		errorCode |= EDGE_GEOM_VALIDATE_ERROR_SKINNING_MATRICES;
	if (isSkinning && (skinWeights & 0xF))
		errorCode |= EDGE_GEOM_VALIDATE_ERROR_SKINNING_WEIGHTS;
	if ((vertexesA & 0xF) || (vertexesB & 0xF))
		errorCode |= EDGE_GEOM_VALIDATE_ERROR_VERTEXES;
	if (hasCustomInputFormat && (inputStreamDescA & 0xF))
		errorCode |= EDGE_GEOM_VALIDATE_ERROR_CUSTOM_INPUT_FORMAT;
	if (hasCustomInputFormat && (inputStreamDescB & 0xF))
		errorCode |= EDGE_GEOM_VALIDATE_ERROR_CUSTOM_INPUT_FORMAT;

	// Input stream descriptions, viewport info, local-to-world matrix,
	// SpuConfigInfo and fixed offsets can be in any order and can
	// technically appear anywhere in the buffer, although it's
	// pointlessly wasteful to put them anywhere except after the
	// vertex buffer.
	if (hasCustomInputFormat != 0 && vertexesA > inputStreamDescA)
		errorCode |= EDGE_GEOM_VALIDATE_WARNING_CUSTOM_INPUT_FORMAT;
	if (hasCustomInputFormat != 0 && vertexesB > inputStreamDescB)
		errorCode |= EDGE_GEOM_VALIDATE_WARNING_CUSTOM_INPUT_FORMAT;
	if (viewportInfo != 0 && vertexesA > viewportInfo)
		errorCode |= EDGE_GEOM_VALIDATE_WARNING_VIEWPORT_INFO;
	if (localToWorld != 0 && vertexesA > localToWorld)
		errorCode |= EDGE_GEOM_VALIDATE_WARNING_LOCAL_TO_WORLD;
	if (vertexesA > spuConfigInfo)
		errorCode |= EDGE_GEOM_VALIDATE_WARNING_SPU_CONFIG_INFO;
	if (fixedOffsetsA != 0 && vertexesA > fixedOffsetsA)
		errorCode |= EDGE_GEOM_VALIDATE_WARNING_FIXED_OFFSETS;
	if (fixedOffsetsB != 0 && vertexesB > fixedOffsetsB)
		errorCode |= EDGE_GEOM_VALIDATE_WARNING_FIXED_OFFSETS;

	// Keep track of the ealiest buffer we've found so far.
	uint32_t vertexes = vertexesB ? ((vertexesA < vertexesB) ? vertexesA : vertexesB) : vertexesA;
	uint32_t earliest = vertexes;

	// If we have skinning matrices or weights, we must have both.  It
	// doesn't matter which comes before the other. Both must come
	// before everything above.
	if (isSkinning)
	{
		if (skinWeights > earliest)
			errorCode |= EDGE_GEOM_VALIDATE_ERROR_SKINNING_WEIGHTS;
		if (skinMatrices > earliest)
			errorCode |= EDGE_GEOM_VALIDATE_ERROR_SKINNING_MATRICES;
		earliest = (skinMatrices < skinWeights) ? skinMatrices : skinWeights;
	}
	
	// If we're using the fast path, certain buffers shouldn't be
	// present (warning) and certain operations must not be performed
	// (error)
	if (isFastPath)
	{		
		if (isSkinning)
			errorCode |= EDGE_GEOM_VALIDATE_WARNING_FAST_PATH;

		if (vertexesB != 0)
			errorCode |= EDGE_GEOM_VALIDATE_WARNING_FAST_PATH;
	}

	// If we have indexes, they must come before the earliest table
	// found above (either the input vertex stream or the earlier of
	// the two skinning tables).
	if (hasIndexes)
	{
		if(indexes > earliest)
			errorCode |= EDGE_GEOM_VALIDATE_ERROR_INDEXES;
		earliest = indexes;
	}

	// If there's custom output code, it must be the first item in the buffer.
	if (hasCustomOutputFormat != 0)
	{
		if (outputStreamDesc > earliest)
			errorCode |= EDGE_GEOM_VALIDATE_ERROR_CUSTOM_OUTPUT_FORMAT;
		earliest = outputStreamDesc;
	}

	return errorCode;
}
#endif

/**
 *	Copies input data to internal variables.  Sets up pointers in scratch 
 * 	memory.  Sets callbacks.
 *  @param ctx EdgeGeom job context
 * 	@param info configuration info for this job
 * 	@param scratchBuffer pointer to scratch memory area
 * 	@param scratchBufferSize (in bytes - CellSpursJobHeader::sizeScratch<<4 with spurs job) - only used for validation
 * 	@param ioBuffer pointer to the base of IO buffer - only used for validation
 * 	@param ioBufferSize (in bytes - CellSpursJobHeader::sizeInOrInOut with spurs job) - only used for validation
 * 	@param dmaTag tag for this job to use with DMAs
 * 	@param inViewportInfo pointer to input viewport info structure to be copied to an internal variable
 * 	@param inLocalToWorldMatrix pointer to input local to world matrix structure to be copied to an internal variable
 *  @param customFormatInfo pointer to custom vertex stream descriptions and callback functions
 */
void edgeGeomInitialize(
	EdgeGeomSpuContext *ctx,
	const EdgeGeomSpuConfigInfo *info, 
	void *scratchBuffer, 
	uint32_t scratchBufferSize,
	void *ioBuffer,
	uint32_t ioBufferSize,
	uint32_t dmaTag,
	const EdgeGeomViewportInfo *inViewportInfo /*= 0*/,
	const EdgeGeomLocalToWorldMatrix *inLocalToWorldMatrix /*= 0*/,
	const EdgeGeomCustomVertexFormatInfo *customFormatInfo /*= 0*/,
    const EdgeGeomCustomTransformVertexesForCullCallbackInfo *customTransformInfo /*= 0*/,
    uint32_t gcmControlEa /* = 0*/)
{
	ctx->freePtr = (uint8_t*)ioBuffer + ioBufferSize; // Overly conservative, but the best we can do for now.
	ctx->indexesLs = 0;
	ctx->vertexesLs = 0;
	ctx->spuConfigInfo = *info;
	ctx->outputDmaTag = dmaTag;
	ctx->inputDmaTag = dmaTag;
	ctx->positionTable = 0;
	ctx->normalTable   = 0;
	ctx->tangentTable  = 0;
	ctx->binormalTable = 0;
	ctx->uniformTableToAttributeIdMapping = si_il(0);
	ctx->scratchBufferBase = (uint32_t) scratchBuffer;
	ctx->scratchBufferEnd = ctx->scratchBufferBase + scratchBufferSize;
	ctx->ioBufferBase = (uint32_t) ioBuffer;
	ctx->ioBufferEnd = ctx->ioBufferBase + ioBufferSize;
	ctx->numVisibleIndexes = (ctx->spuConfigInfo.numIndexes);
    ctx->gcmControlEa = gcmControlEa;

	uint32_t numTables = (ctx->spuConfigInfo.flagsAndUniformTableCount & 0xF) + 1;

	void* curScratch = scratchBuffer;
	const uint32_t numVertexesRounded = (ctx->spuConfigInfo.numVertexes+7) & ~7;
	for(uint32_t i = 0; i < numTables; ++i)
	{
		ctx->uniformTables[i] = (float*) curScratch;
		curScratch = (void*) ((sizeof(qword) * numVertexesRounded) + (uintptr_t)curScratch);
#ifdef EDGE_GEOM_DEBUG
		EDGE_ASSERT_MSG( ((((uint32_t)curScratch<ctx->scratchBufferBase) || ((uint32_t)curScratch>ctx->scratchBufferEnd)) == false),
			("Edge error: Scratch buffer overflow (%06x after table %i) [scratchBuffer=%06x-%06x] \n",
				(uint32_t) curScratch, i, ctx->scratchBufferBase, ctx->scratchBufferEnd) );
#endif//EDGE_GEOM_DEBUG
	}
	if(inViewportInfo != 0)
		ctx->viewportInfo = *inViewportInfo;
	if(inLocalToWorldMatrix != 0)
		ctx->localToWorldMatrix = *inLocalToWorldMatrix;

	if (!customFormatInfo)
	{
		ctx->customFormatInfo.inputStreamDescA = 0;
		ctx->customFormatInfo.inputStreamDescB = 0;
		ctx->customFormatInfo.blendStreamDesc = 0;
		ctx->customFormatInfo.outputStreamDesc = 0;

		ctx->customFormatInfo.decompressInputCallbackA = 0;
		ctx->customFormatInfo.decompressInputCallbackB = 0;
		ctx->customFormatInfo.decompressBlendCallback = 0;
		ctx->customFormatInfo.outputStrideCallback = 0;
		ctx->customFormatInfo.compressOutputCallback = 0;
		ctx->customFormatInfo.setVertexDataArraysCallback = 0;
	}
	else
	{
		ctx->customFormatInfo.inputStreamDescA	= customFormatInfo->inputStreamDescA;
		ctx->customFormatInfo.inputStreamDescB	= customFormatInfo->inputStreamDescB;
		ctx->customFormatInfo.blendStreamDesc	= 0; // filled in per-shape
		ctx->customFormatInfo.outputStreamDesc	= customFormatInfo->outputStreamDesc;
		
		ctx->customFormatInfo.decompressInputCallbackA		= customFormatInfo->decompressInputCallbackA;
		ctx->customFormatInfo.decompressInputCallbackB		= customFormatInfo->decompressInputCallbackB;
		ctx->customFormatInfo.decompressBlendCallback		= customFormatInfo->decompressBlendCallback;
		ctx->customFormatInfo.outputStrideCallback			= customFormatInfo->outputStrideCallback;
		ctx->customFormatInfo.compressOutputCallback		= customFormatInfo->compressOutputCallback;
		ctx->customFormatInfo.setVertexDataArraysCallback	=customFormatInfo->setVertexDataArraysCallback;
	}
    if (!customTransformInfo) 
    {
        ctx->transformForCullCallbackInfo.transformCallback = 0;
        ctx->transformForCullCallbackInfo.transformCallbackUserData = 0;
    }
    else
    {
        ctx->transformForCullCallbackInfo.transformCallback = customTransformInfo->transformCallback;
        ctx->transformForCullCallbackInfo.transformCallbackUserData = customTransformInfo->transformCallbackUserData;
    }
}

qword *edgeGeomGetUniformTable(EdgeGeomSpuContext *ctx, uint32_t index)
{
	return (qword *)(ctx->uniformTables[index]);
}

static int32_t GetUniformTableIndex(EdgeGeomSpuContext *ctx, EdgeGeomAttributeId attrId)
{
	const qword s_DDDDDDDDDDDDDDDD = si_ilh(0x0303);
	const qword id = si_shufb(si_from_int(attrId), s_DDDDDDDDDDDDDDDD, s_DDDDDDDDDDDDDDDD);
	const qword matchBits = si_gbb(si_ceqb(id, ctx->uniformTableToAttributeIdMapping));
	const int tableIndex = si_to_int(si_ai(si_clz(matchBits), -16));

	// If the table index is greater than the number of allocated
	// uniform tables (excluding the spare table, if present), then
	// the index is invalid
	uint8_t validTableCount = (ctx->spuConfigInfo.flagsAndUniformTableCount & 0xF) + 1;
	validTableCount = (ctx->spuConfigInfo.flagsAndUniformTableCount & EDGE_GEOM_FLAG_INCLUDES_EXTRA_UNIFORM_TABLE) ? validTableCount : validTableCount-1;
	

	return (tableIndex < validTableCount) ? tableIndex : -1;
}

static int32_t ReplaceUniformTableMapping(EdgeGeomSpuContext *ctx, EdgeGeomAttributeId oldAttrId,
	EdgeGeomAttributeId newAttrId)
{
	int32_t tableIndex = GetUniformTableIndex(ctx, oldAttrId);
	if (tableIndex < 0)
		return -1;

	// Update the mapping table to reflect the table's new attribute ID
	ctx->uniformTableToAttributeIdMapping = si_shufb(si_from_int(newAttrId),
		ctx->uniformTableToAttributeIdMapping,
		si_cbd(si_from_int(tableIndex), 0));
	return tableIndex;
}

qword *edgeGeomGetUniformTableByAttribute(EdgeGeomSpuContext *ctx, EdgeGeomAttributeId attrId)
{
	int32_t tableIndex = GetUniformTableIndex(ctx, attrId);
	if (tableIndex < 0)
		return 0;
	return (qword *)(ctx->uniformTables[tableIndex]);
}

int32_t edgeGeomAssignUniformTable(EdgeGeomSpuContext *ctx, EdgeGeomAttributeId attrId)
{
	return ReplaceUniformTableMapping(ctx, EDGE_GEOM_ATTRIBUTE_ID_UNKNOWN, attrId);
}


int32_t edgeGeomUnassignUniformTable(EdgeGeomSpuContext *ctx, EdgeGeomAttributeId attrId)
{
	return ReplaceUniformTableMapping(ctx, attrId, EDGE_GEOM_ATTRIBUTE_ID_UNKNOWN);
}

EdgeGeomSpuConfigInfo *edgeGeomGetSpuConfigInfo(EdgeGeomSpuContext *ctx)
{
	return &(ctx->spuConfigInfo);
}

EdgeGeomViewportInfo *edgeGeomGetViewportInfo(EdgeGeomSpuContext *ctx)
{
	return &ctx->viewportInfo;
}

EdgeGeomLocalToWorldMatrix *edgeGeomGetLocalToWorldMatrix(EdgeGeomSpuContext *ctx)
{
	return &(ctx->localToWorldMatrix);
}

uint32_t edgeGeomGetUniformTableCount(EdgeGeomSpuContext *ctx)
{
	return (ctx->spuConfigInfo.flagsAndUniformTableCount & 0xF) + 1;
}

qword **edgeGeomGetUniformTables(EdgeGeomSpuContext *ctx)
{
	return (qword **)(void*)(ctx->uniformTables);
}

qword *edgeGeomGetPositionUniformTable(EdgeGeomSpuContext *ctx)
{
	return (qword*)ctx->positionTable;
}

void edgeGeomSetPositionUniformTable(EdgeGeomSpuContext *ctx, qword *table)
{
	ctx->positionTable = (void*)table;
}

/**
 * 	Get the uniform table containing the screen-space transformed vertices.
 * 	For normal EdgeGeom usage, vertices will be in 
 * 	[X, Y, flags, quantized] format as needed for culling.
 *  @param ctx EdgeGeom job context
 * 	@return uniform table containing transformed vertices.
 */
qword *edgeGeomGetTransformUniformTable(EdgeGeomSpuContext *ctx)
{
	return (qword *)(ctx->uniformTables[ctx->spuConfigInfo.flagsAndUniformTableCount & 0xF]);
}

qword *edgeGeomGetNormalUniformTable(EdgeGeomSpuContext *ctx)
{
	return (qword*)ctx->normalTable;
}

void edgeGeomSetNormalUniformTable(EdgeGeomSpuContext *ctx, qword *table)
{
	ctx->normalTable = (void*)table;
}

qword *edgeGeomGetTangentUniformTable(EdgeGeomSpuContext *ctx)
{
	return (qword*)ctx->tangentTable;
}

void edgeGeomSetTangentUniformTable(EdgeGeomSpuContext *ctx, qword *table)
{
	ctx->tangentTable = (void*)table;
}

qword *edgeGeomGetBinormalUniformTable(EdgeGeomSpuContext *ctx)
{
	return (qword*)ctx->binormalTable;
}

void edgeGeomSetBinormalUniformTable(EdgeGeomSpuContext *ctx, qword *table)
{
	ctx->binormalTable = (void*)table;
}

uint16_t *edgeGeomGetIndexTable(EdgeGeomSpuContext *ctx)
{
	return (uint16_t*)(ctx->indexesLs);
}

uint16_t edgeGeomGetIndexCount(const EdgeGeomSpuContext *ctx)
{
	return ctx->numVisibleIndexes;
}

void edgeGeomSetIndexCount(EdgeGeomSpuContext *ctx, uint16_t count)
{
	ctx->numVisibleIndexes = count;
}

uint16_t edgeGeomGetVertexCount(const EdgeGeomSpuContext *ctx)
{
	return ctx->spuConfigInfo.numVertexes;
}

void edgeGeomSetVertexCount(EdgeGeomSpuContext *ctx, uint16_t count)
{
	ctx->spuConfigInfo.numVertexes = count;
}

bool edgeGeomIsAllocatedFromRingBuffer(EdgeGeomAllocationInfo *info)
{
	return (info->rsxLabelEa);
}

#if EDGEGEOMNORMALIZEUNIFORMTABLE_INTRINSICS
void edgeGeomNormalizeUniformTable(qword *pUniform, int32_t vertexCount)
{
	const int32_t roundedVertexCount = (vertexCount + 7) & ~7;

	const qword sq_AAAa = (qword)s_AAAa;
	const qword sq_AaBb = (qword)s_AaBb;
	const qword sq_BbAa = si_rotqbyi(sq_AaBb, 8);;
	const qword sq_cdAB = (qword)s_cdAB;
	const qword sq_CcCc = (qword)s_CcCc;
	const qword k1_0f = si_ilhu(0x3f80);
	const qword m_00FF = si_fsmbi(0x00FF);
	const qword sq_BBBb = si_orbi(sq_AAAa, 0x04);
	const qword sq_CCCc = si_orbi(sq_AAAa, 0x08);
	const qword sq_DDDd = si_orbi(sq_AAAa, 0x0C);

	const qword pInVerts1  = si_from_ptr(pUniform + roundedVertexCount + 24);
	const qword pInVerts2  = si_from_ptr(pUniform + roundedVertexCount + 25);
	const qword pInVerts3  = si_from_ptr(pUniform + roundedVertexCount + 26);
	const qword pInVerts4  = si_from_ptr(pUniform + roundedVertexCount + 27);
	const qword pInVerts5  = si_from_ptr(pUniform + roundedVertexCount + 28);
	const qword pInVerts6  = si_from_ptr(pUniform + roundedVertexCount + 29);
	const qword pInVerts7  = si_from_ptr(pUniform + roundedVertexCount + 30);
	const qword pInVerts8  = si_from_ptr(pUniform + roundedVertexCount + 31);
	const qword pOutVerts1 = si_from_ptr(pUniform + roundedVertexCount + 0);
	const qword pOutVerts2 = si_from_ptr(pUniform + roundedVertexCount + 1);
	const qword pOutVerts3 = si_from_ptr(pUniform + roundedVertexCount + 2);
	const qword pOutVerts4 = si_from_ptr(pUniform + roundedVertexCount + 3);
	const qword pOutVerts5 = si_from_ptr(pUniform + roundedVertexCount + 4);
	const qword pOutVerts6 = si_from_ptr(pUniform + roundedVertexCount + 5);
	const qword pOutVerts7 = si_from_ptr(pUniform + roundedVertexCount + 6);
	const qword pOutVerts8 = si_from_ptr(pUniform + roundedVertexCount + 7);

	qword offset = si_from_int(-roundedVertexCount * 16);

	// Loop preamble
	qword in1__ = *(pUniform + 0);
	qword in2__ = *(pUniform + 1);
	qword in3__ = *(pUniform + 2);
	qword in4__ = *(pUniform + 3);
	qword in5__ = *(pUniform + 4);
	qword in6__ = *(pUniform + 5);
	qword in7__ = *(pUniform + 6);
	qword in8__ = *(pUniform + 7);
	qword temp1 = si_shufb(in1__, in2__, sq_AaBb);
	qword temp2 = si_shufb(in3__, in4__, sq_BbAa);
	qword temp3 = si_shufb(in1__, in2__, sq_CcCc);
	qword temp4 = si_shufb(in3__, in4__, sq_CcCc);
	qword temp5 = si_shufb(in5__, in6__, sq_AaBb);
	qword temp6 = si_shufb(in7__, in8__, sq_BbAa);
	qword temp7 = si_shufb(in5__, in6__, sq_CcCc);
	qword temp8 = si_shufb(in7__, in8__, sq_CcCc);
	qword xxxx1 = si_selb(temp1, temp2, m_00FF);
	qword yyyy1 = si_shufb(temp2, temp1, sq_cdAB);
	qword zzzz1 = si_selb(temp3, temp4, m_00FF);
	qword xxxx2 = si_selb(temp5, temp6, m_00FF);
	qword yyyy2 = si_shufb(temp6, temp5, sq_cdAB);
	qword zzzz2 = si_selb(temp7, temp8, m_00FF);
	qword partialSum1 = si_fma(yyyy1, yyyy1, si_fm(xxxx1, xxxx1));
	qword partialSum2 = si_fma(yyyy2, yyyy2, si_fm(xxxx2, xxxx2));
	qword sum1  = si_fma(zzzz1, zzzz1, partialSum1);
	qword sum2  = si_fma(zzzz2, zzzz2, partialSum2);
	qword rsqrt1= si_fi(sum1, si_frsqest(sum1));
	qword rsqrt2= si_fi(sum2, si_frsqest(sum2));

	qword in1_  = *(pUniform + 8);
	qword in2_  = *(pUniform + 9);
	qword in3_  = *(pUniform + 10);
	qword in4_  = *(pUniform + 11);
	qword in5_  = *(pUniform + 12);
	qword in6_  = *(pUniform + 13);
	qword in7_  = *(pUniform + 14);
	qword in8_  = *(pUniform + 15);
	temp1 = si_shufb(in1_, in2_, sq_AaBb);
	temp2 = si_shufb(in3_, in4_, sq_BbAa);
	temp3 = si_shufb(in1_, in2_, sq_CcCc);
	temp4 = si_shufb(in3_, in4_, sq_CcCc);
	temp5 = si_shufb(in5_, in6_, sq_AaBb);
	temp6 = si_shufb(in7_, in8_, sq_BbAa);
	temp7 = si_shufb(in5_, in6_, sq_CcCc);
	temp8 = si_shufb(in7_, in8_, sq_CcCc);
	xxxx1 = si_selb(temp1, temp2, m_00FF);
	yyyy1 = si_shufb(temp2, temp1, sq_cdAB);
	zzzz1 = si_selb(temp3, temp4, m_00FF);
	xxxx2 = si_selb(temp5, temp6, m_00FF);
	yyyy2 = si_shufb(temp6, temp5, sq_cdAB);
	zzzz2 = si_selb(temp7, temp8, m_00FF);
	partialSum1 = si_fma(yyyy1, yyyy1, si_fm(xxxx1, xxxx1));
	partialSum2 = si_fma(yyyy2, yyyy2, si_fm(xxxx2, xxxx2));

	qword in1 = *(pUniform + 16);
	qword in2 = *(pUniform + 17);
	qword in3 = *(pUniform + 18);
	qword in4 = *(pUniform + 19);
	qword in5 = *(pUniform + 20);
	qword in6 = *(pUniform + 21);
	qword in7 = *(pUniform + 22);
	qword in8 = *(pUniform + 23);

	do
	{
		// part 4 -- scale input verts by inverse sqrt and write them back to the uniform table
		si_stqx(si_fm(in1__, si_shufb(rsqrt1, k1_0f, sq_AAAa)), pOutVerts1, offset);
		si_stqx(si_fm(in2__, si_shufb(rsqrt1, k1_0f, sq_BBBb)), pOutVerts2, offset);
		si_stqx(si_fm(in3__, si_shufb(rsqrt1, k1_0f, sq_CCCc)), pOutVerts3, offset);
		si_stqx(si_fm(in4__, si_shufb(rsqrt1, k1_0f, sq_DDDd)), pOutVerts4, offset);
		si_stqx(si_fm(in5__, si_shufb(rsqrt2, k1_0f, sq_AAAa)), pOutVerts5, offset);
		si_stqx(si_fm(in6__, si_shufb(rsqrt2, k1_0f, sq_BBBb)), pOutVerts6, offset);
		si_stqx(si_fm(in7__, si_shufb(rsqrt2, k1_0f, sq_CCCc)), pOutVerts7, offset);
		si_stqx(si_fm(in8__, si_shufb(rsqrt2, k1_0f, sq_DDDd)), pOutVerts8, offset);

		// part 3 -- compute reciprocal square root of all four sums
		sum1 = si_fma(zzzz1, zzzz1, partialSum1);
		sum2 = si_fma(zzzz2, zzzz2, partialSum2);
		rsqrt1 = si_fi(sum1, si_frsqest(sum1));
		rsqrt2 = si_fi(sum2, si_frsqest(sum2));
		in1__ = in1_;
		in2__ = in2_;
		in3__ = in3_;
		in4__ = in4_;
		in5__ = in5_;
		in6__ = in6_;
		in7__ = in7_;
		in8__ = in8_;

		// part 2 -- sum squares of vertex components
		temp1 = si_shufb(in1, in2, sq_AaBb);
		temp2 = si_shufb(in3, in4, sq_BbAa);
		temp3 = si_shufb(in1, in2, sq_CcCc);
		temp4 = si_shufb(in3, in4, sq_CcCc);
		temp5 = si_shufb(in5, in6, sq_AaBb);
		temp6 = si_shufb(in7, in8, sq_BbAa);
		temp7 = si_shufb(in5, in6, sq_CcCc);
		temp8 = si_shufb(in7, in8, sq_CcCc);
		xxxx1 = si_selb(temp1, temp2, m_00FF);
		yyyy1 = si_shufb(temp2, temp1, sq_cdAB);
		zzzz1 = si_selb(temp3, temp4, m_00FF);
		xxxx2 = si_selb(temp5, temp6, m_00FF);
		yyyy2 = si_shufb(temp6, temp5, sq_cdAB);
		zzzz2 = si_selb(temp7, temp8, m_00FF);
		partialSum1 = si_fma(yyyy1, yyyy1, si_fm(xxxx1, xxxx1));
		partialSum2 = si_fma(yyyy2, yyyy2, si_fm(xxxx2, xxxx2));
		in1_ = in1;
		in2_ = in2;
		in3_ = in3;
		in4_ = in4;
		in5_ = in5;
		in6_ = in6;
		in7_ = in7;
		in8_ = in8;

		// part 1 -- load input verts
		in1 = si_lqx(pInVerts1, offset);
		in2 = si_lqx(pInVerts2, offset);
		in3 = si_lqx(pInVerts3, offset);
		in4 = si_lqx(pInVerts4, offset);
		in5 = si_lqx(pInVerts5, offset);
		in6 = si_lqx(pInVerts6, offset);
		in7 = si_lqx(pInVerts7, offset);
		in8 = si_lqx(pInVerts8, offset);

		offset = si_ai(offset, 0x80);
	} while(si_to_uint(offset) != 0);
}
#endif

// NOTE: This function assumes that interrupts have already been
// disabled by the caller.  If they haven't, then this code isn't 100%
// safe
static bool allocateOutputSpaceFromSharedBuffer(EdgeGeomSpuContext *ctx,
	uint32_t sharedBufferInfoEa, uint32_t size,	EdgeGeomAllocationInfo *info)
{
	(void) ctx;

	uint32_t currentEa, result;
	uint8_t sharedInfoBuffer[0x80] __attribute__((__aligned__(128)));
	EdgeGeomSharedBufferInfo volatile *sharedInfo = (EdgeGeomSharedBufferInfo volatile *)sharedInfoBuffer;

	// Get the shared output buffer address and update it in an atomic operation.
	do 
	{
		// Atomic-get the shared buffer info
		EDGE_DMA_GETLLAR(sharedInfo, sharedBufferInfoEa, 0, 0);
		result = EDGE_DMA_WAIT_ATOMIC_STATUS();
		
		// Check to see if memory is available for this allocation.
		currentEa = sharedInfo->currentEa;
		if (currentEa + size > sharedInfo->endEa)
		{
			// If not, inform the system of an allocation failure.
			sharedInfo->failedAllocSize += size;
			currentEa = 0;
		}
		else
		{
			// Otherwise update the shared buffer info with the new address from which allocate.
			sharedInfo->currentEa = currentEa + size;
			info->location = sharedInfo->locationId;
		}
		
		// Write the shared buffer info back to main memory, making
		// sure nothing it hasn't been changed since it was read.
		EDGE_DMA_PUTLLC(sharedInfo, sharedBufferInfoEa, 0, 0);
		result = EDGE_DMA_WAIT_ATOMIC_STATUS();
	} while (result & 1);
	
	info->ea = currentEa;
	info->offset = sharedInfo->startOffset + (currentEa - sharedInfo->startEa);

	return (currentEa != 0);
}

static bool allocateOutputSpaceFromRingBuffer(EdgeGeomSpuContext *ctx, EdgeGeomRingBufferInfo *ringInfo, uint32_t size,
	bool waitForGpu, EdgeGeomAllocationInfo *info)
{
	// Grab the start and end pointers from the mutex.
	uint32_t labelEa			= ringInfo->rsxLabelEa;
	uint32_t start				= ringInfo->startEa;
	uint32_t end				= ringInfo->endEa;
	uint32_t current			= ringInfo->currentEa;
	uint32_t currentPlusSize	= current+size;
	uint32_t startPlusSize		= start+size;

	// If the ring buffer is not at least twice as big as the
	// allocation size, there's a possibility for deadlock.  In that
	// case, we just fail.
	if (__builtin_expect((2*size > (end-start)), false)) {
		return false;
	}

	// Add validation for ring buffer free cache since it's a new feature
	EDGE_ASSERT_MSG( ((ringInfo->cachedFree&&((ringInfo->cachedFree<ringInfo->startEa)||(ringInfo->cachedFree>ringInfo->endEa))) == false),
						("Edge error: Invalid ringInfo->cachedFree (must be either NULL or within the ring buffer - 0x%08x ring buffer is 0x%08x - 0x%08x)\n",
							ringInfo->cachedFree, ringInfo->startEa, ringInfo->endEa) );

	uint8_t labelBuffer[0x80] __attribute__((__aligned__(128)));
	uint32_t volatile *pLabelValue = (uint32_t volatile *) ((uintptr_t) labelBuffer + (0x7FU & (uintptr_t) labelEa));
	bool success = false;
	bool getLabel = ringInfo->cachedFree? false: true;

#ifdef EDGE_GEOM_DEBUG
    uint8_t gcmControlBuffer[0x80] __attribute__((__aligned__(128)));
    uint32_t volatile *pGcmControlValue = (uint32_t volatile *) ((uintptr_t) gcmControlBuffer + (0x7FU & (uintptr_t) ctx->gcmControlEa));
    uint32_t getValue = 0;
    uint32_t oldGetValue = 0;
    uint32_t getCount = 0;
#endif //EDGE_GEOM_DEBUG
	do
	{
		if (getLabel)
		{
			// Get the RSX label into LS.
			EDGE_DMA_SMALL_GET(pLabelValue, labelEa, 4, ctx->inputDmaTag, 0, 0);
			
			// Wait for the DMA to complete
			EDGE_DMA_WAIT_TAG_STATUS_ALL(1 << ctx->inputDmaTag);

			ringInfo->cachedFree = *pLabelValue;
		}

		// Free
		uint32_t freeEnd = ringInfo->cachedFree;

		// Test to see if the allocation will fit.
		if (__builtin_expect((((freeEnd <= current) || (currentPlusSize < freeEnd)) &&
							  ((currentPlusSize <= end) || (startPlusSize < freeEnd))), true))
		{
			success = true;
			// Check whether we need to flip back to the beginning of the ring buffer
			if (currentPlusSize > end)
			{
				current = start;
				currentPlusSize = startPlusSize;
                info->wrapped = true;
			}
		}
		// Only exit based on the "real" free position
		else if (__builtin_expect((!waitForGpu && getLabel), false))
		{
			return false;
		}
		// Now we must read the label to know where the RSX really is
		else if (!getLabel)
		{
			getLabel = true;
		}
		// May want to wait for a bit if this fails...
		else
		{
            // Read GcmControl structure
            // Compare to previous
            // If getCount > N, warning
#ifdef EDGE_GEOM_DEBUG			
            if (ctx->gcmControlEa) {
                // Get the control register into LS.
				EDGE_DMA_GET(pGcmControlValue, ctx->gcmControlEa, 16, ctx->inputDmaTag, 0, 0);
			
                // Wait for the DMA to complete
				EDGE_DMA_WAIT_TAG_STATUS_ALL(1 << ctx->inputDmaTag);

                getValue = pGcmControlValue[1];

                if (oldGetValue && oldGetValue == getValue) {
                    getCount++;
                }
                else{
                    getCount = 0;
                }
                oldGetValue = getValue;

                if (getCount == EDGE_GEOM_WARN_GET_POINTER_UNCHANGED_COUNT) {
                    EDGE_PRINTF("Edge warning: Get pointer unchanged: (get, put) (0x%08x, 0x%08x)\n", getValue, pGcmControlValue[0]);
                    //count = 0;
                }
            }
#endif //EDGE_GEOM_DEBUG

			// Wait ~96,000 cycles to avoid flooding the GPU with label
			// reads.  Each read of the SPU decrementer takes 40
			// cycles
			for(uint32_t i=0; i<2400; ++i)
			{
				spu_readch(SPU_RdDec);
			}
		}
	}
	while(__builtin_expect((!success),false));
	
	ringInfo->currentEa = currentPlusSize;
	
	info->ea = current;
	info->offset = ringInfo->startOffset + (current - start);
	info->location = ringInfo->locationId;

	return true;
}

uint32_t edgeGeomCalculateDefaultOutputSize(EdgeGeomSpuContext *ctx, uint32_t numIndexes)
{
	uint32_t outputStride = edgeGeomGetOutputVertexStride(ctx, ctx->spuConfigInfo.outputVertexFormatId);
	// padded size of indexes
	uint32_t allocSize = ((numIndexes + 7) & ~7) * 2; 
	// padded size of vertexes
	allocSize += (ctx->spuConfigInfo.numVertexes * outputStride + 0xF) & ~0xF; 
	// all rounded up to a multiple of 128 bytes (see below for why this is crucial)
	return (allocSize + 0x7F) & ~0x7F;
}

bool edgeGeomAllocateOutputSpace(EdgeGeomSpuContext *ctx, uint32_t outputBufferInfoEa, uint32_t allocSize, EdgeGeomAllocationInfo *info, uint32_t spuId)
{
	// This will be set to true if this info is passed to OutputIndexes or OutputVertexes
	info->isVertexData = false;
    info->wrapped = false;

	// If the allocator address has the "direct output" bit set
	// (currently, 0x2), then we're allocating from a specific address
	// that's guaranteed to be available, and we don't need to go any
	// further.
	if(outputBufferInfoEa & 0x00000002)
	{
		// direct address
		info->ea = outputBufferInfoEa & 0xFFFFFFFC; // mask off flag bits
		info->offset = -1;
		info->location = outputBufferInfoEa & 0x1;
		info->rsxLabelEa = 0;
		info->endEa = info->ea + allocSize;
		return true;
	}

	// round up to a multiple of 128 bytes.  This is ensures that all
	// output DMAs start on a 128 byte boundary, which is important
	// both for performance (DMA bandwidth is halved if the source and
	// destination addresses aren't at identical mod 128) and
	// correctness (the RSX reads data in 128-byte blocks, and padding
	// the allocations forces the RSX to fetch the data for each draw
	// call separately, avoiding several nasty cache issues).
	allocSize = (allocSize + 0x7F) & ~0x7F; 

	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	// Load the ring buffer info
	static uint8_t ringInfoBuffer[0x80] __attribute__((__aligned__(128)));
	EdgeGeomRingBufferInfo volatile *ringInfo = (EdgeGeomRingBufferInfo volatile *)ringInfoBuffer;
	const uint32_t ringInfoEa = outputBufferInfoEa + sizeof(EdgeGeomSharedBufferInfo)
		+ spuId * sizeof(EdgeGeomRingBufferInfo);

	// Atomic-get the ring buffer info
	// This is an abuse of the atomic DMA as we're interested in SL1, not atomic access
	EDGE_DMA_GETLLAR(ringInfo, ringInfoEa, 0, 0);
	(void)EDGE_DMA_WAIT_ATOMIC_STATUS();

	bool outputToRingBuffer = false;
	bool hasRingBuffer = (ringInfo->endEa - ringInfo->startEa > 0);
	bool success = false;

	if (!hasRingBuffer)
	{
		// Allocate from the shared buffer
		success = allocateOutputSpaceFromSharedBuffer(ctx, outputBufferInfoEa, allocSize, info);
	}
	else
	{
		// Attempt to allocate from the ring buffer once.  waitForGpu
		// is set to false, so if the allocation wouldn't succeed
		// we'll return immediately instead of blocking.
		success = allocateOutputSpaceFromRingBuffer(ctx, (EdgeGeomRingBufferInfo*)ringInfo, allocSize, false, info);
		if (success)
		{
			outputToRingBuffer = true;
		}
		else
		{
			// If the initial ring buffer allocate attempt failed, attempt to
			// allocate from the shared buffer.
			success = allocateOutputSpaceFromSharedBuffer(ctx, outputBufferInfoEa, allocSize, info);
			if (!success)
			{
				// If the overflow allocate attempt failed, go back to
				// the ring buffer and block until it succeeds.
				success = allocateOutputSpaceFromRingBuffer(ctx, (EdgeGeomRingBufferInfo*)ringInfo,
					allocSize, true, info);
				outputToRingBuffer = true;
			}
		}
	}

	if (outputToRingBuffer) 
	{
		// Store the ring buffer info back to main memory
		// Unconditional put from SL1 (this cache line is private - no need for real atomic op)
		EDGE_DMA_PUTLLUC(ringInfo, ringInfoEa, 0, 0);
		(void)EDGE_DMA_WAIT_ATOMIC_STATUS();
	}

	if (interruptsEnabled)
	{
		spu_ienable();
	}

	// the allocation failed?
	if(!success)
	{
		return false; 
	}

	// Set some info if the allocation succeeded and is
	// from a ring buffer.
	if (outputToRingBuffer)
	{
		info->rsxLabelEa = ringInfo->rsxLabelEa;
	}
	else
	{
		info->rsxLabelEa = 0;
	}
	info->endEa = info->ea + allocSize;

	return true;
}

void edgeGeomOutputIndexes(EdgeGeomSpuContext *ctx, uint32_t numIndexes, EdgeGeomAllocationInfo *info, EdgeGeomLocation *outLoc)
{
	if(!numIndexes) {
		return;
	}

	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	
	spu_idisable();
	uint32_t size = ((numIndexes + 7) & ~7) * 2;
	uint32_t ls = ctx->indexesLs;
	uint32_t ea = info->ea;
	uint32_t offset = info->offset;
	EDGE_ASSERT_MSG(ea + size <= info->endEa, ("Edge Geometry error: output index DMA too large for destination") );
	outLoc->ea = ea;
	outLoc->offset = offset;
	outLoc->location = info->location;
	EDGE_DMA_LARGE_PUT(ls, ea, size, ctx->outputDmaTag, 0, 0);
	info->ea = ea + size;
	info->offset = offset + size;
	if(interruptsEnabled)
		spu_ienable();

	info->isVertexData = true;
}

void edgeGeomOutputVertexes(EdgeGeomSpuContext *ctx, EdgeGeomAllocationInfo *info, EdgeGeomLocation *outLoc)
{
	// In the static geometry path, we do not output any vertex data.
	// The static geometry path should not be compressing/outputting vertices!
	if (ctx->spuConfigInfo.flagsAndUniformTableCount & EDGE_GEOM_FLAG_STATIC_GEOMETRY_FAST_PATH)
		return;

	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();
	uint32_t stride = edgeGeomGetOutputVertexStride(ctx, ctx->spuConfigInfo.outputVertexFormatId);
	uint32_t size = (ctx->spuConfigInfo.numVertexes * stride + 0xF) & ~0xF;
	uint32_t ls = ctx->vertexesLs;
	uint32_t ea = info->ea;
	uint32_t offset = info->offset;
	EDGE_ASSERT_MSG(ea + size <= info->endEa, ("Edge Geometry error: output vertex DMA too large for destination") );
	outLoc->ea = ea;
	outLoc->offset = offset;
	outLoc->location = info->location;
	EDGE_DMA_LARGE_PUT(ls, ea, size, ctx->outputDmaTag, 0, 0);
	info->ea = ea + size;
	info->offset = offset + size;
	if(interruptsEnabled)
		spu_ienable();

	info->isVertexData = true;
}

static const qword s_Dabc = {0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b};

// Patches a command buffer in the appropriate places for jump to self sync.
// Note: Will not correctly parse Jumps, Calls, or Returns.
static inline uint32_t patchForLocalStallSync(uint32_t *cmd, uint32_t size)
{
	if(!size)
		return 0;
		
	qword cmdEnd = si_from_ptr(cmd + size / 4);
	qword cmd0 = si_from_ptr(cmd);

	qword cmd1			= (qword)(0);
	qword notPastEnd	= (qword)(0);
	qword startAligned	= (qword)(0);
	qword endAligned	= startAligned;
	qword cmd2			= (qword)(0);
	qword num			= (qword)(0);
	qword get0			= (qword)(0);
	qword sizeQw = si_from_uint(size);

	const qword mask0 = (qword)spu_splats(0x1ffc);
	const qword alignMask = (qword)spu_splats(0x7F);
	const qword getMask = (qword)spu_splats(0xE003FFFF);
	const qword s_Aabc = si_selb(si_ila(0x10203), s_Dabc, si_fsm(si_from_uint(0x7)));

	//note: Tabbing indicates loop iteration.
	do 
	{
			notPastEnd = si_clgt(cmdEnd, cmd1);
		qword get0Input = si_lqd(cmd0, 0x0);
			qword isAligned = si_ceq(startAligned, endAligned);

		startAligned = si_andc(cmd0, alignMask);
		if(__builtin_expect(si_to_uint(notPastEnd) == 0, false))
			break;

			cmd2 = si_ori(cmd1, 0);
		cmd1 = si_ori(cmd0, 0);
			qword get2 = si_ori(get0, 0);

			qword cmd2Added = si_ai(cmd2, 0x80);
		get0 = si_rotqby(get0Input, cmd0);
			qword cmd2Sub = si_ai(cmd2, 4);
			qword get2Rotated = si_rotqmbii(si_rotqmbyi(get2, -3), -4);

			qword splitAt = si_andc(cmd2Added, alignMask);
		qword get0Rotated = si_rotqmbyi(get0, -2);
			qword newNum = si_sf(cmd2Sub, splitAt);

			qword get2Anded1 = si_andi(get2Rotated, 0x4);
			qword num2 = si_sf(newNum, num);
			qword inc = si_xori(get2Anded1, 0x4);
		num = si_and(get0Rotated, mask0);
			qword newNumShifted = si_shli(newNum, 16);
		qword end = si_a(cmd0, num);
			qword incXnewNum = si_mpyu(inc, si_rotqmbii(newNum,-2));
		cmd0 = si_ai(end, 4);
		endAligned = si_andc(end, alignMask);

		if (__builtin_expect(si_to_uint(isAligned), true))
			continue;

		qword newNumIs0 = si_ceqi(newNum,0);

		qword oldCmdValue = si_lqd(cmd2, 0x0);

		qword pInTmp0 = si_sf(splitAt, cmdEnd);
		qword pInTmp2 = si_andi(pInTmp0, ~0x7f);
		qword pIn = si_a(splitAt, pInTmp2);

		qword newCmd = si_selb(newNumShifted, get2, getMask);
		qword s_mask = si_cwd(cmd2, 0x0);

		newCmd = si_andc(newCmd, newNumIs0);

		qword newCmdOut = si_shufb(newCmd, oldCmdValue, s_mask);
		si_stqd(newCmdOut, cmd2, 0x00);

		qword pLoopEnd = si_ai(splitAt, -0x80);

		qword in1 = (qword)spu_splats(0);
		qword loop = (qword)spu_splats(0);

		do{
			qword pInTemp = si_ai(pIn, -0x80);
			loop = si_ceq(pInTemp, pLoopEnd);

			qword in0 = si_lqd(pIn, -0x10);
			in1 = si_lqd(pIn, 0x00);
			qword in2 = si_lqd(pIn, 0x10);
			qword in3 = si_lqd(pIn, 0x20);
			qword in4 = si_lqd(pIn, 0x30);
			qword in5 = si_lqd(pIn, 0x40);
			qword in6 = si_lqd(pIn, 0x50);
			qword in7 = si_lqd(pIn, 0x60);
			qword in8 = si_lqd(pIn, 0x70);

			qword out0 = si_shufb(in0, in1, s_Dabc);
			qword out1 = si_shufb(in1, in2, s_Dabc);
			qword out2 = si_shufb(in2, in3, s_Dabc);
			qword out3 = si_shufb(in3, in4, s_Dabc);
			qword out4 = si_shufb(in4, in5, s_Dabc);
			qword out5 = si_shufb(in5, in6, s_Dabc);
			qword out6 = si_shufb(in6, in7, s_Dabc);
			qword out7 = si_shufb(in7, in8, s_Dabc);

			si_stqd(out0, pIn, 0x00);
			si_stqd(out1, pIn, 0x10);
			si_stqd(out2, pIn, 0x20);
			si_stqd(out3, pIn, 0x30);
			si_stqd(out4, pIn, 0x40);
			si_stqd(out5, pIn, 0x50);
			si_stqd(out6, pIn, 0x60);
			si_stqd(out7, pIn, 0x70);

			pIn = pInTemp;
		} while(!si_to_uint(loop));

		qword num2Shifted = si_shli(num2, 16);
		qword tmp0 = si_selb(num2Shifted, get2, getMask);
		qword newSplitAt = si_a(tmp0, incXnewNum);

		qword splitAtShifted = si_shufb(newSplitAt, in1, s_Aabc);

		si_stqd(splitAtShifted, splitAt, 0x0);

		cmd0 = splitAt;
		cmd1 = cmd0;
		cmdEnd = si_ai(cmdEnd,4);
		sizeQw = si_ai(sizeQw, 4);
		endAligned = startAligned;
	} while(true);

	return si_to_uint(sizeQw);
}

void edgeGeomBeginCommandBufferHole(EdgeGeomSpuContext *ctx, CellGcmContextData *gcmCtx, uint32_t holeEa, EdgeGeomAllocationInfo *infos, uint32_t numInfos)
{
	// Leave space for 4 bytes in case we have to read from video memory to maintain data consistency in VRAM.
	uint32_t syncData = (uint32_t)edgeGeomGetFreePtr(ctx);
	edgeGeomSetFreePtr(ctx, (void*)(16+(uint32_t)syncData));

	// Align the LS address to the EA address
	uint32_t cmdAddr = (uint32_t)edgeGeomGetFreePtr(ctx);
	uint32_t alignedCmdAddr = (cmdAddr & 0xFFFFFF80) | (holeEa & 0x7F);
	cmdAddr = (cmdAddr > alignedCmdAddr) ? alignedCmdAddr + 0x80 : alignedCmdAddr;
	uint32_t *cmd = (uint32_t*)cmdAddr;
	gcmCtx->begin = cmd;
	gcmCtx->current = cmd;
	gcmCtx->end = cmd + (ctx->spuConfigInfo.commandBufferHoleSize << 2) + (ctx->spuConfigInfo.commandBufferHoleSize == 0 ? 1 : 0); 
	gcmCtx->callback = 0;
	edgeGeomSetFreePtr(ctx, (void*)(((uint32_t)gcmCtx->end + 127) & -128));

	/* 	If we've written output data to video memory, we need to make sure
		that those output DMAs are finished before the command buffer hole
		is filled, otherwise the RSX might try to access the data before
		it's been committed to video memory. */
	for (uint32_t i = 0; i < numInfos; i++) {
		if (infos[i].location == CELL_GCM_LOCATION_LOCAL) {

			bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
			spu_idisable();

			/* 	Read 4 bytes from video memory with a fence, to make sure
				that the proceeding output DMAs finish before the command 
				buffer hole is filled. */
			EDGE_DMA_SMALL_GETF(syncData, infos[i].ea, 4, ctx->outputDmaTag, 0, 0);

			if(interruptsEnabled)
				spu_ienable();

			/*  We only need to do this once, since all of the outputs have
				already been started, so break. */
			break;
		}
	}

	bool invalidateVertexCache = false;
	for (uint32_t i = 0; i < numInfos; i++) {
		invalidateVertexCache = invalidateVertexCache || (infos[i].rsxLabelEa && infos[i].wrapped && infos[i].isVertexData);
	}

	if (invalidateVertexCache) {
		cellGcmSetInvalidateVertexCacheUnsafeInline(gcmCtx);
	}
} 

// Wrapper around inlined libgcm function of the same name, to make
// sure there's only one copy of this code in the final ELF. We're
// using the inline version in the first place to avoid having to link
// with libgcm, not because we can't spare the performance hit of a
// handful of function calls.
void edgeGeomSetVertexDataArray(CellGcmContextData *thisContext, const uint8_t index,
	const uint16_t frequency, const uint8_t stride, const uint8_t size, const uint8_t type,
	const uint8_t location, const uint32_t offset)
{
	return cellGcmSetVertexDataArrayUnsafeInline(thisContext, index, frequency, stride, size, type,
		location, offset);
}

static void SetVertexDataArraysByDescription(EdgeGeomSpuContext *ctx, CellGcmContextData *gcmCtx,
	EdgeGeomLocation *loc, EdgeGeomVertexStreamDescription *streamDesc)
{
	// Write Attribute Addresses and formats
	uint32_t offset = loc->offset;
	uint32_t location = loc->location;
	uint32_t stride = streamDesc->stride;
	for(uint32_t iAttr=0; iAttr<streamDesc->numAttributes; ++iAttr)
	{
		EdgeGeomAttributeBlock &attr = streamDesc->blocks[iAttr].attributeBlock;
		edgeGeomSetVertexDataArray(gcmCtx, attr.vertexProgramSlotIndex, 0, stride,
			attr.componentCount, attr.format, location, offset + attr.offset);
	}
}

void edgeGeomSetVertexDataArrays(EdgeGeomSpuContext *ctx, CellGcmContextData *gcmCtx, EdgeGeomLocation *loc)
{
	// Write Attribute Addresses and formats
	uint32_t offset = loc->offset;
	uint32_t location = loc->location;
	uint32_t stride = edgeGeomGetOutputVertexStride(ctx, ctx->spuConfigInfo.outputVertexFormatId);
	switch(ctx->spuConfigInfo.outputVertexFormatId)
	{
	case EDGE_GEOM_RSX_VERTEX_FORMAT_F32c3:
		edgeGeomSetVertexDataArray(gcmCtx, 0, 0, stride, 3, CELL_GCM_VERTEX_F, location, offset);
		break;
	case EDGE_GEOM_RSX_VERTEX_FORMAT_F32c3_X11Y11Z10N_X11Y11Z10N:
		edgeGeomSetVertexDataArray(gcmCtx, 0, 0, stride, 3, CELL_GCM_VERTEX_F, location, offset);
		edgeGeomSetVertexDataArray(gcmCtx, 2, 0, stride, 1, CELL_GCM_VERTEX_CMP, location, offset + 12);
		edgeGeomSetVertexDataArray(gcmCtx, 14, 0, stride, 1, CELL_GCM_VERTEX_CMP, location, offset + 16);
		break;
	case EDGE_GEOM_RSX_VERTEX_FORMAT_F32c3_X11Y11Z10N_I16Nc4:
		edgeGeomSetVertexDataArray(gcmCtx, 0, 0, stride, 3, CELL_GCM_VERTEX_F, location, offset);
		edgeGeomSetVertexDataArray(gcmCtx, 2, 0, stride, 1, CELL_GCM_VERTEX_CMP, location, offset + 12);
		edgeGeomSetVertexDataArray(gcmCtx, 14, 0, stride, 4, CELL_GCM_VERTEX_S1, location, offset + 16);
		break;
	case EDGE_GEOM_RSX_VERTEX_FORMAT_F32c3_X11Y11Z10N_X11Y11Z10N_X11Y11Z10N:
		edgeGeomSetVertexDataArray(gcmCtx, 0, 0, stride, 3, CELL_GCM_VERTEX_F, location, offset);
		edgeGeomSetVertexDataArray(gcmCtx, 2, 0, stride, 1, CELL_GCM_VERTEX_CMP, location, offset + 12);
		edgeGeomSetVertexDataArray(gcmCtx, 14, 0, stride, 1, CELL_GCM_VERTEX_CMP, location, offset + 16);
		edgeGeomSetVertexDataArray(gcmCtx, 15, 0, stride, 1, CELL_GCM_VERTEX_CMP, location, offset + 20);
		break;
	case EDGE_GEOM_RSX_VERTEX_FORMAT_EMPTY:
		break;
	default:
		if(ctx->customFormatInfo.setVertexDataArraysCallback != 0)
		{
			(*ctx->customFormatInfo.setVertexDataArraysCallback)(ctx,
				ctx->spuConfigInfo.outputVertexFormatId, gcmCtx, location, offset);
		}
		else if (ctx->customFormatInfo.outputStreamDesc != 0)
		{
			SetVertexDataArraysByDescription(ctx, gcmCtx, loc,
				ctx->customFormatInfo.outputStreamDesc);
		}
		else
		{
			// ERROR
		}
		break;
	}
}

void edgeGeomEndCommandBufferHole(EdgeGeomSpuContext *ctx, CellGcmContextData *gcmCtx, uint32_t holeEa, EdgeGeomAllocationInfo *infos, uint32_t numInfos)
{
	if(!holeEa)
		return;
		
	for(uint32_t i = 0; i < numInfos; ++i)
	{
		// rsxLabelEa is only set when using a ring buffer.
		uint32_t rsxLabelEa = infos[i].rsxLabelEa;

		// If using ring buffers for output, then generate the command 
		// buffer commands that will update the RSX label value once 
		// the data created by this job is completely consumed.
		if(rsxLabelEa)
			cellGcmSetWriteTextureLabelUnsafeInline(gcmCtx, (rsxLabelEa & 0x0FF0) >> 4, infos[i].endEa);
	}

	uint32_t size = (gcmCtx->current - gcmCtx->begin) * 4; // original size of GCM commands
	uint32_t newSize = patchForLocalStallSync(gcmCtx->begin, size); // final size after patching for JTS
	uint32_t holeSize = ctx->spuConfigInfo.commandBufferHoleSize ? (ctx->spuConfigInfo.commandBufferHoleSize << 4) : 4; // available space in the main GCM command buffer
#ifdef EDGE_GEOM_DEBUG
	EDGE_ASSERT_MSG( newSize <= holeSize,
					("EDGE Geometry error: RSX command size (%d) exceeds hole size (%d)\n",
						newSize, holeSize) );
#endif

	gcmCtx->current += ((newSize - size) / 4);

	uint32_t count = gcmCtx->end - gcmCtx->current;

	// Pad the rest of the hole with nops.  SDK 250 provides a more
	// efficient command for this, but we'll keep the old way around
	// until earlier SDK versions are deprecated.
#if CELL_SDK_VERSION < 0x250001
	cellGcmSetNopCommandUnsafeInline(gcmCtx, count);
#else
    if (count) {
        cellGcmSetSkipNopUnsafeInline(gcmCtx, count - 1);
    }
#endif

    bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	if ( holeSize < 16 )
		EDGE_DMA_SMALL_PUTF(gcmCtx->begin, holeEa, holeSize, ctx->outputDmaTag, 0, 0);
	else
		EDGE_DMA_PUTF(gcmCtx->begin, holeEa, holeSize, ctx->outputDmaTag, 0, 0);

	if(interruptsEnabled)
		spu_ienable();
}

void edgeGeomFinalize(EdgeGeomSpuContext *ctx)
{
	(void) ctx;
}
