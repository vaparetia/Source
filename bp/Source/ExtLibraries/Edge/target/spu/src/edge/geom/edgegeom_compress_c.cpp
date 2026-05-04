/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/geom/edgegeom.h"
#include "edge/geom/edgegeom_internal.h"
#include "edge/geom/edgegeom_compress.h"

#if !COMPRESSVERTEXESBYDESCRIPTION_INTRINSICS
#define EDGEMIN(x,y) ((x)<(y)?(x):(y))
#define EDGEMAX(x,y) ((x)>(y)?(x):(y))

void *CompressVertexesByDescription(EdgeGeomSpuContext *ctx,
	const EdgeGeomVertexStreamDescription *streamDesc)
{
	// Re-compressing vertex data is forbidden in the static geometry fast path; you shouldn't
	// be outputting any vertex data at all!
	if (ctx->spuConfigInfo.flagsAndUniformTableCount & EDGE_GEOM_FLAG_STATIC_GEOMETRY_FAST_PATH)
		return (void*)ctx->vertexesLs;

	void *outVertexes = ctx->freePtr;
	ctx->vertexesLs = (uint32_t)outVertexes;

	if(ctx->spuConfigInfo.numVertexes == 0)
		return outVertexes;

	const uint32_t vertexStride = streamDesc->stride;
	const uint32_t numVertexes = (ctx->spuConfigInfo.numVertexes + 3) & ~3;

	const EdgeGeomGenericBlock *nextAttribute = streamDesc->blocks;
	// Process each attribute sequentially.
	for(uint32_t iAttr=0; iAttr<streamDesc->numAttributes; ++iAttr)
	{
		EdgeGeomAttributeBlock attribute = nextAttribute->attributeBlock;
		nextAttribute++;

		uint8_t *pOut = (uint8_t *)outVertexes;

		//offset into vertex
		const uint8_t attribOffset = attribute.offset;
		const uint32_t vertexInc = vertexStride - attribute.size;

		pOut += attribOffset;

		//which uniform
		const uint8_t id = attribute.edgeAttributeId;
		float *src = 0;
		for (uint32_t i = 0; i < 16; i++) {
			if (ctx->uniformTableToAttributeIdMapping[i] == id) {
				src = (float *)edgeGeomGetUniformTable(ctx, i);
				break;
			}
		}

		const int32_t attribFormat = attribute.format;
		switch (attribFormat) {
		case EDGE_GEOM_ATTRIBUTE_FORMAT_F32:
			for (uint32_t i = 0; i < numVertexes; i++) {
				for (uint32_t j = 0; j < attribute.componentCount; j++) {
					union {float f; uint8_t b[4];} u;
					u.f = src[i*4 + j];
					//copy bytes
					pOut[0] = u.b[0];
					pOut[1] = u.b[1];
					pOut[2] = u.b[2];
					pOut[3] = u.b[3];

					pOut += 4;
				}
				pOut += vertexInc;
			}
			break;
		case EDGE_GEOM_ATTRIBUTE_FORMAT_F16:
			for (uint32_t i = 0; i < numVertexes; i++) {
				for (uint32_t j = 0; j < attribute.componentCount; j++) {
					union {uint16_t u16; uint8_t b[2];} u;

					const uint32_t f = *(uint32_t*)(&src[i*4 + j]);
					const uint32_t signbit  = (f & 0x80000000) >> 16;
					const int32_t  exponent = ((f & 0x7F800000) >> 23) - (127 - 15);
					const uint32_t mantissa = (f & 0x007FFFFF);

					if (exponent <= 0)
						u.u16 =  0;
					else if (exponent > 30)
						u.u16 = (uint16_t)(signbit | 0x7BFF);
					else
						u.u16 = (uint16_t)(signbit | (exponent << 10) | (mantissa >> 13));

					pOut[0] = u.b[0];
					pOut[1] = u.b[1];

					pOut += 2;
				}
				pOut += vertexInc;
			}
			break;
		case EDGE_GEOM_ATTRIBUTE_FORMAT_U8:
			for (uint32_t i = 0; i < numVertexes; i++) {
				for (uint32_t j = 0; j < attribute.componentCount; j++) {
					//bias to prevent rounding down
					float f = src[i*4 + j] + .5f;
					const uint8_t b = (uint8_t)(EDGEMAX(0.0f,EDGEMIN(255.0f,f)));
					pOut[0] = b;
					pOut += 1;
				}
				pOut += vertexInc;
			}
			break;
		case EDGE_GEOM_ATTRIBUTE_FORMAT_I16:
			for (uint32_t i = 0; i < numVertexes; i++) {
				for (uint32_t j = 0; j < attribute.componentCount; j++) {
					union {int16_t i16; uint8_t b[2];} u;
					float f = src[i*4 + j];

					u.i16 = (int16_t)(EDGEMAX(-32768.0f,EDGEMIN(32767.0f,f)));

					pOut[0] = u.b[0];
					pOut[1] = u.b[1];

					pOut += 2;
				}
				pOut += vertexInc;
			}
			break;
		case EDGE_GEOM_ATTRIBUTE_FORMAT_U8N:
			for (uint32_t i = 0; i < numVertexes; i++) {
				for (uint32_t j = 0; j < attribute.componentCount; j++) {
					float f = src[i*4 + j];

					const uint8_t b = (uint8_t)(EDGEMAX(0.0f,EDGEMIN(1.0f,f))*255.0f);

					pOut[0] = b;
					pOut += 1;
				}
				pOut += vertexInc;
			}
			break;
		case EDGE_GEOM_ATTRIBUTE_FORMAT_I16N:
			for (uint32_t i = 0; i < numVertexes; i++) {
				for (uint32_t j = 0; j < attribute.componentCount; j++) {
					union {int16_t i16; uint8_t b[2];} u;
					float f = src[i*4 + j];

					u.i16 = (int16_t)((((EDGEMAX(-1.0f,EDGEMIN(1.0f,f))+1.0f)/2.0f)*65535.0f)-32768.0f);

					pOut[0] = u.b[0];
					pOut[1] = u.b[1];

					pOut += 2;
				}
				pOut += vertexInc;
			}
			break;
		case EDGE_GEOM_ATTRIBUTE_FORMAT_X11Y11Z10N:
			#define X11Y11Z10N_X_MASK 0x000007FF
			#define X11Y11Z10N_X_BITS 11
			#define X11Y11Z10N_X_SHIFT 0

			#define X11Y11Z10N_Y_MASK 0x003FF800
			#define X11Y11Z10N_Y_BITS 11
			#define X11Y11Z10N_Y_SHIFT 11
		
			#define X11Y11Z10N_Z_MASK 0xFFC00000
			#define X11Y11Z10N_Z_BITS 10
			#define X11Y11Z10N_Z_SHIFT 22

			for (uint32_t i = 0; i < numVertexes; i++) {
				union {uint32_t u32; uint8_t b[4];} u;				

				float x = src[i*4 + 0];
				float y = src[i*4 + 1];
				float z = src[i*4 + 2];

				const uint32_t uX = ((int32_t(EDGEMAX(EDGEMIN(((x)*2047.f - 1.f)*0.5f, 1023.f), -1024.f)) & (X11Y11Z10N_X_MASK >> X11Y11Z10N_X_SHIFT)) << X11Y11Z10N_X_SHIFT);
				const uint32_t uY = ((int32_t(EDGEMAX(EDGEMIN(((y)*2047.f - 1.f)*0.5f, 1023.f), -1024.f)) & (X11Y11Z10N_Y_MASK >> X11Y11Z10N_Y_SHIFT)) << X11Y11Z10N_Y_SHIFT);
				const uint32_t uZ = ((int32_t(EDGEMAX(EDGEMIN(((z)*1023.f - 1.f)*0.5f,  511.f), -512.f )) & (X11Y11Z10N_Z_MASK >> X11Y11Z10N_Z_SHIFT)) << X11Y11Z10N_Z_SHIFT);
				u.u32 = uX | uY | uZ;

				pOut[0] = u.b[0];
				pOut[1] = u.b[1];
				pOut[2] = u.b[2];
				pOut[3] = u.b[3];

				pOut += vertexStride;
			}
			break;
		}
	}
	return outVertexes;
}
#endif
