/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef EDGEGEOM_H
#define EDGEGEOM_H

#include <spu_intrinsics.h>
#include <stdint.h>
#include <stdbool.h>
#include <cell/gcm_spu.h>
#include "edge/geom/edgegeom_structs.h"
#include "edge/edge_assert.h"
#include "edge/edge_dma.h"
#include "edge/edge_printf.h"

#ifdef __cplusplus
extern "C" 
{
#endif

struct EdgeGeomSpuContext;

typedef uint32_t (*EdgeGeomGetOutputVertexStrideCallback)(EdgeGeomSpuContext *ctx,
	uint32_t outputFormatId);
typedef uint32_t (*EdgeGeomDecompressVertexStreamCallback)(EdgeGeomSpuContext *ctx,
	const void *vertexes, uint32_t numVertexes, const void *fixedOffsets,
	uint32_t inputFormatId);
typedef uint32_t (*EdgeGeomBlendVertexStreamCallback)(EdgeGeomSpuContext *ctx, const void *vertexes,
	uint32_t numVertexes, const void *fixedOffsets, uint32_t deltaFormatId, float alpha);
typedef uint32_t (*EdgeGeomCompressVertexStreamCallback)(EdgeGeomSpuContext *ctx, uint32_t numVertexes,
	void *outVertexes, uint32_t outputFormatId);
typedef uint32_t (*EdgeGeomSetVertexDataArraysCallback)(EdgeGeomSpuContext *ctx, uint32_t outputFormatId,
	CellGcmContextData *gcmCtx, uint32_t outputLocation, uint32_t vertexOffset);
struct EdgeGeomCustomVertexFormatInfo
{
	// Pointers to stream descriptions for input, blending and output.
	// If built-in vertex formats are being used, leave the appropriate
	// fields as NULL.
	EdgeGeomVertexStreamDescription *inputStreamDescA; // for primary input stream
	EdgeGeomVertexStreamDescription *inputStreamDescB; // for secondary input stream
	EdgeGeomVertexStreamDescription *outputStreamDesc;
	EdgeGeomVertexStreamDescription *blendStreamDesc;
	
	// Alternately, individual functions can be overloaded with custom
	// SPU code that's linked in with the edge SPU elf.  If present,
	// these functions override the custom code blocks specified
	// above.  Otherwise, set them to NULL.
	EdgeGeomDecompressVertexStreamCallback decompressInputCallbackA; // for primary input stream
	EdgeGeomDecompressVertexStreamCallback decompressInputCallbackB; // for secondary input stream
	EdgeGeomBlendVertexStreamCallback decompressBlendCallback;
	EdgeGeomGetOutputVertexStrideCallback outputStrideCallback;
	EdgeGeomCompressVertexStreamCallback compressOutputCallback;
	EdgeGeomSetVertexDataArraysCallback setVertexDataArraysCallback;
};
typedef void (*EdgeGeomTransformVertexesForCullCallback)(EdgeGeomSpuContext *ctx, 
                                                         void *userData);
struct EdgeGeomCustomTransformVertexesForCullCallbackInfo
{
    EdgeGeomTransformVertexesForCullCallback transformCallback;
    void *transformCallbackUserData;
};

struct __attribute__((__aligned__(16))) EdgeGeomSpuContext
{
	EdgeGeomSpuConfigInfo spuConfigInfo;
	float *uniformTables[16];
	uint32_t indexesLs;
	uint32_t vertexesLs;
	void *freePtr;
	void *positionTable;
	void *normalTable;
	void *tangentTable;
	void *binormalTable;
	EdgeGeomCustomVertexFormatInfo customFormatInfo;
	EdgeGeomLocalToWorldMatrix localToWorldMatrix;
	EdgeGeomViewportInfo viewportInfo;
	uint32_t outputDmaTag;
	uint32_t inputDmaTag;
	qword uniformTableToAttributeIdMapping; // byte N contains the attribute ID in uniform table N
	uint32_t scratchBufferBase;
	uint32_t scratchBufferEnd;
	uint32_t ioBufferBase;
	uint32_t ioBufferEnd;
    uint32_t gcmControlEa;
    EdgeGeomCustomTransformVertexesForCullCallbackInfo transformForCullCallbackInfo;
    EdgeGeomCullingResults *cullingResults;
	uint16_t numVisibleIndexes;
};


void edgeGeomInitialize(EdgeGeomSpuContext *ctx, const EdgeGeomSpuConfigInfo *spuConfigInfo, 
						void *scratchBuffer, uint32_t scratchBufferSize,
						void *ioBuffer, uint32_t ioBufferSize,
						uint32_t dmaTag, 
						const EdgeGeomViewportInfo *inViewportInfo = 0,
						const EdgeGeomLocalToWorldMatrix *inLocalToWorldMatrix = 0,
						const EdgeGeomCustomVertexFormatInfo *customFormatInfo = 0,
                        const EdgeGeomCustomTransformVertexesForCullCallbackInfo *customTransformInfo = 0, 
                        uint32_t gcmControlEa = 0);
void edgeGeomFinalize(EdgeGeomSpuContext *ctx);

EdgeGeomSpuConfigInfo *edgeGeomGetSpuConfigInfo(EdgeGeomSpuContext *ctx);
EdgeGeomViewportInfo *edgeGeomGetViewportInfo(EdgeGeomSpuContext *ctx);
EdgeGeomLocalToWorldMatrix *edgeGeomGetLocalToWorldMatrix(EdgeGeomSpuContext *ctx);
qword *edgeGeomGetUniformTable(EdgeGeomSpuContext *ctx, uint32_t index);
qword *edgeGeomGetUniformTableByAttribute(EdgeGeomSpuContext *ctx, EdgeGeomAttributeId attrId);
qword **edgeGeomGetUniformTables(EdgeGeomSpuContext *ctx);
uint32_t edgeGeomGetUniformTableCount(EdgeGeomSpuContext *ctx);
int32_t edgeGeomAssignUniformTable(EdgeGeomSpuContext *ctx, EdgeGeomAttributeId attrId);
int32_t edgeGeomUnassignUniformTable(EdgeGeomSpuContext *ctx, EdgeGeomAttributeId attrId);
qword *edgeGeomGetPositionUniformTable(EdgeGeomSpuContext *ctx);
void edgeGeomSetPositionUniformTable(EdgeGeomSpuContext *ctx, qword *table);
qword *edgeGeomGetTransformUniformTable(EdgeGeomSpuContext *ctx);
qword *edgeGeomGetNormalUniformTable(EdgeGeomSpuContext *ctx);
void edgeGeomSetNormalUniformTable(EdgeGeomSpuContext *ctx, qword *table);
qword *edgeGeomGetTangentUniformTable(EdgeGeomSpuContext *ctx);
void edgeGeomSetTangentUniformTable(EdgeGeomSpuContext *ctx, qword *table);
qword *edgeGeomGetBinormalUniformTable(EdgeGeomSpuContext *ctx);
void edgeGeomSetBinormalUniformTable(EdgeGeomSpuContext *ctx, qword *table);
uint16_t *edgeGeomGetIndexTable(EdgeGeomSpuContext *ctx);
uint16_t edgeGeomGetIndexCount(const EdgeGeomSpuContext *ctx);
void edgeGeomSetIndexCount(EdgeGeomSpuContext *ctx, uint16_t count);
uint16_t edgeGeomGetVertexCount(const EdgeGeomSpuContext *ctx);
void edgeGeomSetVertexCount(EdgeGeomSpuContext *ctx, uint16_t count);
void *edgeGeomGetFreePtr(EdgeGeomSpuContext *ctx);
void edgeGeomSetFreePtr(EdgeGeomSpuContext *ctx, const void *ptr);
bool edgeGeomIsAllocatedFromRingBuffer(EdgeGeomAllocationInfo *info);
uint32_t edgeGeomGetOutputVertexStride(EdgeGeomSpuContext *ctx, uint32_t outputFormatId);
void edgeGeomProcessBlendShapes(EdgeGeomSpuContext *ctx, uint32_t numShapes, uint32_t shapeInfosEa);	
void edgeGeomNormalizeUniformTable(qword *pUniform, int32_t vertexCount);
void edgeGeomSkinVertexes(EdgeGeomSpuContext *ctx, void *matrices, uint32_t matrixCount, void *indexesAndWeights);
uint32_t edgeGeomCalculateDefaultOutputSize(EdgeGeomSpuContext *ctx, uint32_t numIndexes);
bool edgeGeomAllocateOutputSpace(EdgeGeomSpuContext *ctx, uint32_t outputBufferInfoEa, uint32_t allocSize, EdgeGeomAllocationInfo *outInfo, uint32_t spuId);
void edgeGeomOutputIndexes(EdgeGeomSpuContext *ctx, uint32_t numIndexes, EdgeGeomAllocationInfo *info, EdgeGeomLocation *outLoc);
void edgeGeomOutputVertexes(EdgeGeomSpuContext *ctx, EdgeGeomAllocationInfo *info, EdgeGeomLocation *outLoc);
void edgeGeomBeginCommandBufferHole(EdgeGeomSpuContext *ctx, CellGcmContextData *gcmCtx, uint32_t holeEa, EdgeGeomAllocationInfo *infos, uint32_t numInfos);
void edgeGeomSetVertexDataArrays(EdgeGeomSpuContext *ctx, CellGcmContextData *gcmCtx, EdgeGeomLocation *vtxLoc);
void edgeGeomEndCommandBufferHole(EdgeGeomSpuContext *ctx, CellGcmContextData *gcmCtx, uint32_t holeEa, EdgeGeomAllocationInfo *infos, uint32_t numInfos);

inline void edgeGeomUseOutputSpace(EdgeGeomAllocationInfo *info, uint32_t size, uint32_t *ea, uint32_t *offset) __attribute__((always_inline));
inline void edgeGeomUseOutputSpace(EdgeGeomAllocationInfo *info, uint32_t size, uint32_t *ea, uint32_t *offset) 
{
    *ea = info->ea;
    *offset = info->offset;

    info->offset += size;
    info->ea += size;
}

#ifdef EDGE_GEOM_DEBUG
uint32_t edgeGeomValidateBufferOrder(
	const void *pOutputStreamDesc, const void *pIndexes, 
	const void *pSkinMatrices, const void *pSkinWeights, 
	const void *pVertexesA, const void *pVertexesB, 
	const void *pViewportInfo, const void *pLocalToWorld, 
	const void *pSpuConfigInfo, const void *pFixedOffsetsA,
	const void *pFixedOffsetsB,	const void *pInputStreamDescA,
	const void *pInputStreamDescA);
#else
static inline uint32_t edgeGeomValidateBufferOrder(
	const void *, const void *, const void *, 
	const void *, const void *, const void *, 
	const void *, const void *, const void *,
	const void *, const void *, const void *,
	const void *)
{
	return 0;
}
#endif

// In edgegeom_decompress.cpp
void edgeGeomDecompressVertexes(EdgeGeomSpuContext *ctx, const void *vertexesA, const void *fixedOffsetsA, const void *vertexesB = 0, const void *fixedOffsetsB = 0);
void edgeGeomDecompressIndexes(EdgeGeomSpuContext *ctx, const void *indexes);

// In edgegeom_compress.cpp
void *edgeGeomCompressVertexes(EdgeGeomSpuContext *ctx);

// In edgegeom_cull.cpp
uint32_t edgeGeomCullTriangles(EdgeGeomSpuContext *ctx, uint32_t cullingFlavor, int32_t indexBias = 0,
	EdgeGeomCullingResults *detailedResults = 0);

// In edgegeom_transform.cpp
void edgeGeomTransformVertexes(uint32_t numVertexes, void *inVertexes, void *outVertexes, void *matrix);

// In edgegeom_occlusion.cpp / edgegeom_occlusion_main.cpp
uint32_t edgeGeomCullOccludedTriangles(EdgeGeomSpuContext *ctx, const uint32_t occludersCount, const uint32_t occludersEa,
	int32_t indexBias = 0, EdgeGeomCullingResults *detailedResults = 0);

#ifdef __cplusplus
}
#endif


#endif // EDGEGEOM_H
