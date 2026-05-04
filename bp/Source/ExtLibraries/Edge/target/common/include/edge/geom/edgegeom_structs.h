/* SCE CONFIDENTIAL 
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef EDGEGEOMSTRUCTS_H
#define EDGEGEOMSTRUCTS_H

#include "edge/edge_stdint.h"
#include "edge/edge_stdbool.h"
#include "edge/edge_version.h"
#include "edge/geom/edgegeom_attributes.h"

// We want certain structures to be aligned on the PS3 (for DMA purposes) without padding
// their size.  GCC's attribute-align directive allows this; Visual C++'s declspec-align
// directive automatically pads the structures.  Since we really only care about defining
// the structures in VC so we can use sizeof(), we opt to just not align them in VC at all.
#if !defined(_MSC_VER)
#define EDGE_ALIGN_NO_PAD(x) __attribute__((__aligned__(x)))
#else
#define EDGE_ALIGN_NO_PAD(x)
#endif

#define EDGE_GEOM_CULL_NONE                   0
#define EDGE_GEOM_CULL_FRUSTUM                1
#define EDGE_GEOM_CULL_BACKFACES_AND_FRUSTUM  2
#define EDGE_GEOM_CULL_FRONTFACES_AND_FRUSTUM 3

#define EDGE_GEOM_SKIN_NONE                0
#define EDGE_GEOM_SKIN_NO_SCALING          1
#define EDGE_GEOM_SKIN_UNIFORM_SCALING     2
#define EDGE_GEOM_SKIN_NON_UNIFORM_SCALING 3
#define EDGE_GEOM_SKIN_SINGLE_BONE_NO_SCALING 4
#define EDGE_GEOM_SKIN_SINGLE_BONE_UNIFORM_SCALING 5
#define EDGE_GEOM_SKIN_SINGLE_BONE_NON_UNIFORM_SCALING 6

#define EDGE_GEOM_INDEXES_U16_TRIANGLE_LIST_CW          0
#define EDGE_GEOM_INDEXES_U16_TRIANGLE_LIST_CCW         1
#define EDGE_GEOM_INDEXES_COMPRESSED_TRIANGLE_LIST_CW   2
#define EDGE_GEOM_INDEXES_COMPRESSED_TRIANGLE_LIST_CCW  3

#define EDGE_GEOM_MATRIX_3x4_ROW_MAJOR 0    // This is Edge's "native" format for skinning matrices
#define EDGE_GEOM_MATRIX_4x4_ROW_MAJOR 1    // DirectX-style
#define EDGE_GEOM_MATRIX_4x4_COLUMN_MAJOR 2 // OpenGL-style

#define EDGE_GEOM_SPU_VERTEX_FORMAT_F32c3                                        0
#define EDGE_GEOM_SPU_VERTEX_FORMAT_F32c3_X11Y11Z10N_X11Y11Z10N                  1
#define EDGE_GEOM_SPU_VERTEX_FORMAT_F32c3_X11Y11Z10N_I16Nc4                      2
#define EDGE_GEOM_SPU_VERTEX_FORMAT_F32c3_X11Y11Z10N_X11Y11Z10N_X11Y11Z10N       3
#define EDGE_GEOM_SPU_VERTEX_FORMAT_FIXED_UNITVEC_UNITVEC                        4
#define EDGE_GEOM_SPU_VERTEX_FORMAT_FIXED_UNITVEC_UNITVEC_UNITVEC                5
#define EDGE_GEOM_SPU_VERTEX_FORMAT_EMPTY                                        6

#define EDGE_GEOM_RSX_VERTEX_FORMAT_F32c3                                        0
#define EDGE_GEOM_RSX_VERTEX_FORMAT_F32c3_X11Y11Z10N_X11Y11Z10N                  1
#define EDGE_GEOM_RSX_VERTEX_FORMAT_F32c3_X11Y11Z10N_I16Nc4                      2
#define EDGE_GEOM_RSX_VERTEX_FORMAT_F32c3_X11Y11Z10N_X11Y11Z10N_X11Y11Z10N       3
#define EDGE_GEOM_RSX_VERTEX_FORMAT_EMPTY                                        4

// The first 7 entries here parallel the CELL_GCM_VERTEX_* defines in <cell/gcm/gcm_enum.h>
#define EDGE_GEOM_ATTRIBUTE_FORMAT_I16N        0x01 // -1..1
#define EDGE_GEOM_ATTRIBUTE_FORMAT_F32         0x02 // 32-bit float
#define EDGE_GEOM_ATTRIBUTE_FORMAT_F16         0x03 // 16-bit float, format is s10e5
#define EDGE_GEOM_ATTRIBUTE_FORMAT_U8N         0x04 // 0..1
#define EDGE_GEOM_ATTRIBUTE_FORMAT_I16         0x05 // -32K..32K
#define EDGE_GEOM_ATTRIBUTE_FORMAT_X11Y11Z10N  0x06 // 32bit 10:11:11 bits (z,y,x), each -1..1
#define EDGE_GEOM_ATTRIBUTE_FORMAT_U8          0x07 // 0..255
// SPU-only formats with no GCM equivalent
#define EDGE_GEOM_ATTRIBUTE_FORMAT_FIXED_POINT 0x08 // arbitrary-precision fixed point
#define EDGE_GEOM_ATTRIBUTE_FORMAT_UNIT_VECTOR 0x09 // smallest two components stored with 10 bits apiece.  3 more bits determine which component is missing and its sign.  The final bit gives the sign of the W component (-1.0 or 1.0)

/* These masks are used to interpret the return value of
 * edgeGeomValidateBufferOrder().  Each buffer has a bit which is set if
 * that buffer isn't in a valid location.  In some cases, a misplaced
 * buffer is a fatal error; in others, it's simply a performance
 * warning. See the LibEdge-Overview file for more information on the
 * correct buffer ordering.
 */
#define EDGE_GEOM_VALIDATE_WARNING_VIEWPORT_INFO       0x00000001
#define EDGE_GEOM_VALIDATE_WARNING_LOCAL_TO_WORLD      0x00000002
#define EDGE_GEOM_VALIDATE_WARNING_SPU_CONFIG_INFO     0x00000004
#define EDGE_GEOM_VALIDATE_WARNING_FIXED_OFFSETS       0x00000008
#define EDGE_GEOM_VALIDATE_WARNING_CUSTOM_INPUT_FORMAT 0x00000010
#define EDGE_GEOM_VALIDATE_WARNING_FAST_PATH           0x00000020
#define EDGE_GEOM_VALIDATE_ERROR_SKINNING_MATRICES     0x00010000
#define EDGE_GEOM_VALIDATE_ERROR_SKINNING_WEIGHTS      0x00020000
#define EDGE_GEOM_VALIDATE_ERROR_INDEXES               0x00040000
#define EDGE_GEOM_VALIDATE_ERROR_CUSTOM_OUTPUT_FORMAT  0x00080000
#define EDGE_GEOM_VALIDATE_ERROR_FAST_PATH             0x00100000
#define EDGE_GEOM_VALIDATE_ERROR_CUSTOM_INPUT_FORMAT   0x00200000
#define EDGE_GEOM_VALIDATE_ERROR_VERTEXES              0x00400000

/* These masks can be used to quickly check whether any errors or
 * warnings were reported.
 */
#define EDGE_GEOM_VALIDATE_ERROR_MASK                  0xFFFF0000
#define EDGE_GEOM_VALIDATE_WARNING_MASK                0x0000FFFF

/*
 * These flags define the various processing options for the SPU.
 * They exist in the upper 4 bits of the first byte of EdgeGeomSpuConfigInfo.
 */
#define EDGE_GEOM_FLAG_STATIC_GEOMETRY_FAST_PATH  0x10 // Currently unused.  Do not set!
#define EDGE_GEOM_FLAG_INCLUDES_EXTRA_UNIFORM_TABLE 0x80 // this bit must be set in order for culling or custom blend shape flavors to be used

/*
 * These values should be masked into the address specified for direct output.
 */
#define EDGE_GEOM_DIRECT_OUTPUT_TO_LOCAL_MEMORY		0x00000002
#define EDGE_GEOM_DIRECT_OUTPUT_TO_MAIN_MEMORY 		0x00000003

struct EDGE_ALIGN_NO_PAD(16) EdgeGeomSpuConfigInfo
{
	uint8_t flagsAndUniformTableCount;		// flags in high nibble.  uniform count - 1 in low nibble (to make the effective range 1-16).
	uint8_t commandBufferHoleSize;			// >> 4
	uint8_t inputVertexFormatId;			// Custom is 0xFF
	uint8_t secondaryInputVertexFormatId;	// Custom is 0xFF
	uint8_t outputVertexFormatId;			// Custom is 0xFF
	uint8_t vertexDeltaFormatId;			// Custom is 0xFF. Unused as of 0.4.2 (all blend shapes use custom formats)
	uint8_t indexesFlavorAndSkinningFlavor;	// indexes flavor in high nibble.  Skinning flavor in low nibble.
	uint8_t skinningMatrixFormat;
	uint16_t numVertexes;
	uint16_t numIndexes;
	uint32_t indexesOffset;					// Low bit is location
};


/*
 * This structure is not *really* part of the Edge Geometry API; it is
 * not referenced by any Edge function.  It is only provided as an
 * example structure used to package up all the data required for a
 * single segment.  Feel free to replace it with your own custom
 * structure.
 *
 * All sizes given are in bytes, unless otherwise specified
 */
struct EDGE_ALIGN_NO_PAD(16) EdgeGeomPpuConfigInfo
{
	EdgeGeomSpuConfigInfo spuConfigInfo;				// 000
	voidptr32_t indexes;								// 016 // actual pointer type depends on index flavor
	uint16_t indexesSizes[2];							// 020
	voidptr32_t spuVertexes[2];							// 024 // actual pointer type depends on vertex format
	uint16_t spuVertexesSizes[6];						// 032
	voidptr32_t rsxOnlyVertexes;						// 044 // actual pointer type depends on vertex format
	uint32_t rsxOnlyVertexesSize;						// 048
	uint16_t skinMatricesByteOffsets[2];				// 052
	uint16_t skinMatricesSizes[2];						// 056
	uint16_t skinIndexesAndWeightsSizes[2];				// 060
	voidptr32_t skinIndexesAndWeights;					// 064 // actual pointer type is uint8_t*
	uint32_t ioBufferSize;								// 068
	uint32_t scratchSizeInQwords;						// 072 
	uint32_t numBlendShapes;							// 076
	uint16ptr32_t blendShapeSizes;						// 080 // Actual pointer type is uint16_t* -- NULL if no blend shapes
	uint32ptr32_t blendShapes;							// 084 // Actual pointer type is uint32_t* -- NULL if no blend shapes
	uint32_t fixedOffsetsSize[2];						// 088
	voidptr32_t fixedOffsets[2];						// 096 // actual pointer type is uint32_t*
	voidptr32_t spuInputStreamDescs[2];					// 104 // actual pointer type is EdgeGeomVertexStreamDescription*
	voidptr32_t spuOutputStreamDesc;					// 112 // actual pointer type is EdgeGeomVertexStreamDescription*
	voidptr32_t rsxOnlyStreamDesc;						// 116 // actual pointer type is EdgeGeomVertexStreamDescription*
	uint16_t spuInputStreamDescSizes[2];				// 1202
	uint16_t spuOutputStreamDescSize;					// 124
	uint16_t rsxOnlyStreamDescSize;						// 126
 														// 128
};


struct EdgeGeomAttributeBlock
{
	uint8_t offset;					// byte offset of attribute within vertex
	uint8_t format;					// EDGE_GEOM_ATTRIBUTE_FORMAT_*
	uint8_t componentCount;			// 1-4
	uint8_t edgeAttributeId;		// EDGE_GEOM_ATTRIBUTE_ID_*
	uint8_t size;					// size of attribute in bytes
	uint8_t vertexProgramSlotIndex;	// destination vertex program slot on the RSX (valid values are 0-15)
	uint8_t fixedBlockOffset;		// offset (from start of attribute blocks) of the fixed point
									// block for this attribute, if its format is
	                                // EDGE_GEOM_ATTRIBUTE_FORMAT_FIXED_POINT.  Otherwise, unused.
	uint8_t padding;				// unused
};

struct EdgeGeomAttributeFixedBlock
{
	uint8_t integer0;
	uint8_t mantissa0;
	uint8_t integer1;
	uint8_t mantissa1;
	uint8_t integer2;
	uint8_t mantissa2;
	uint8_t integer3;
	uint8_t mantissa3;
};

union EdgeGeomGenericBlock
{
	EdgeGeomAttributeBlock attributeBlock;
	EdgeGeomAttributeFixedBlock fixedBlock;
};

struct EDGE_ALIGN_NO_PAD(16) EdgeGeomVertexStreamDescription
{
	uint8_t numAttributes;
	uint8_t stride;
	uint8_t numBlocks; // For the true size of the structure, use (numBlocks+1)*8
	uint8_t padding[5];
	EdgeGeomGenericBlock blocks[0];
};

struct EDGE_ALIGN_NO_PAD(16) EdgeGeomBlendShapeInfo
{
	uint64_t dmaTag;
	float alpha;
	uint32_t padding;
};

struct EDGE_ALIGN_NO_PAD(16) EdgeGeomViewportInfo
{
	uint16_t scissorArea[4];
	float depthRange[2];
	float viewProjectionMatrix[16];
	float viewportScales[4];
	float viewportOffsets[4];
	uint8_t sampleFlavor;
	uint8_t pad[15];
};

struct EDGE_ALIGN_NO_PAD(16) EdgeGeomLocalToWorldMatrix
{
	float matrixData[12];
};

// A simple output buffer that's shared (via atomic locks) between all SPUs.
// When it's full, subsequent allocations just fail.
struct EDGE_ALIGN_NO_PAD(128) EdgeGeomSharedBufferInfo
{
	uint32_t startEa;     // address of buffer start
	uint32_t startOffset; // RSX offset of buffer start 
	uint32_t endEa;       // address of buffer end (start + size)
	uint32_t currentEa;   // address of beginning of buffer free area
	uint32_t locationId;  // CELL_GCM_LOCATION_MAIN or CELL_GCM_LOCATION_LOCAL
	uint32_t failedAllocSize; // total size of allocations that couldn't be fulfilled per frame.

	uint32_t pad1[2]; // pad out to 128 bytes, to facilitate atomic DMA access.
	uint32_t pad2[24]; 
};

// An effectively infinite output buffer that's synchronized with the
// RSX, so that it knows when it's safe to loop back on itself.  If an
// allocation won't fit, the SPU blocks until the RSX consumes enough
// data to free up the required space.
// Important: this structure is now accessed through a GETLLAR and now
// requires 128 byte alignment
struct EDGE_ALIGN_NO_PAD(128) EdgeGeomRingBufferInfo
{
	uint32_t startEa;     // address of buffer start
	uint32_t startOffset; // RSX offset of buffer start
	uint32_t endEa;       // address of buffer end (start + size)
	uint32_t currentEa;   // address of beginning of buffer free area
	uint32_t locationId;  // CELL_GCM_LOCATION_MAIN or CELL_GCM_LOCATION_LOCAL
	uint32_t rsxLabelEa;  // Address of RSX label used for synchronization
	uint32_t cachedFree;  // Cached free pointer (must be initialized to 0) 
	uint32_t pad[25];     // pad out to 128 bytes
};

// Describes the application's output buffering scheme.  A buffer is
// considered empty/nonexistent if its startEa and endEa fields are
// the same.
//
// If only the shared buffer non-empty, single buffering output is used.
//
// If only the ring buffer is non-empty, ring buffering output is used.
//
// If both buffers are non-empty, then ring buffering output is used,
// with the shared buffer providing overflow space when the SPU would
// otherwise have to wait for the RSX.  This is the recommended
// configuration.
struct EDGE_ALIGN_NO_PAD(128) EdgeGeomOutputBufferInfo
{
	EdgeGeomSharedBufferInfo sharedInfo; // shared by all SPUs
	EdgeGeomRingBufferInfo ringInfo[6];  // one per SPU addressable by SPURS
};

// Describes the allocation reserved by edgeGeomAllocateOutputSpace().
struct EDGE_ALIGN_NO_PAD(16) EdgeGeomAllocationInfo
{
	// Effective address of the allocation
	uint32_t ea;
	// RSX offset of the allocation
	uint32_t offset;
	// RSX location of the allocation
	uint32_t location;
	// the end of the allocation (ring buffers only)
	uint32_t endEa;
	// Effective address of an rsx label (ring buffers only)
	uint32_t rsxLabelEa;
    // Set if ring buffer wrapped
    bool wrapped;
	// Set if used in edgeGeomOutputVertexes() or EdgeGeomOutputIndexes()
	bool isVertexData;
	uint8_t pad[10];
};

// Desribes where in memory something is located from both the 
// CELL and RSX perspectives.
struct EDGE_ALIGN_NO_PAD(16) EdgeGeomLocation
{
	// Effective address
	uint32_t ea;
	// RSX offset
	uint32_t offset;
	// RSX location (main or video memory)
	uint32_t location;
	uint32_t pad;
};

// If this structure is passed to Edge culling functions --
// edgeGeomCullTriangles() and edgeGeomCullOccludedTriangles() -- its
// counters will be incremented for each triangle that is culled due to
// each test.
// Some caveats:
// - For performance reasons, the results will only be tallied when
//   the C implementation of culling is used (see edgegeom_config.h),
//   and only in debug builds (EDGE_GEOM_DEBUG is defined)
// - It is the user's responsibility to zero out the counters prior to
//   calling the culling functions; internally, they are only
//   incremented.
// - Each triangle may fail multiple tests, and thus cause multiple
//   counters to increment.  So, the sum of all the counters will
//   almost certainly be greater than totalNumCulledTriangles.
struct EDGE_ALIGN_NO_PAD(16) EdgeGeomCullingResults
{
    uint16_t numOccludedTriangles; // occlusion -- triangles occluded by user-provided occlusion quads
    uint16_t numOutsideFrustumTriangles; // frustum -- triangles outside the view frustum
    uint16_t numNoPixelTriangles; // noPixel -- triangles that do not intersect any pixel sample points
    uint16_t numBackFacingTriangles; // backFace -- triangles which face away from the viewer.
    uint16_t totalNumCulledTriangles;// culled -- triangles which fit one or more of the above conditions
    uint16_t pad[3];
};

#ifdef EDGE_ALIGN_NO_PAD
#undef EDGE_ALIGN_NO_PAD
#endif

#endif // EDGEGEOMSTRUCTS_H

