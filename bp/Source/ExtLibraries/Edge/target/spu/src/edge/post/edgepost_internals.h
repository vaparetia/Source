/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGEPOST_INTERNALS_H__
#define __EDGEPOST_INTERNALS_H__

#include "edge/edge_stdint.h"
#include "edge/edge_assert.h"
#include "edge/post/edgepost_framework_spu.h"

// internal return codes
#define EDGEPOST_INVALID_TILE			0xffff
#define EDGEPOST_INVALID_STAGE			0xffff
#define EDGEPOST_TILE_NOT_READY			( 0xffff - 1)

// forward
struct EdgePostProcessStage;
struct EdgePostContext;

struct mfc_list_element;
typedef struct mfc_list_element EdgeDmaListElement;

// Define SPU post-processing memory layout for one stage
typedef struct EdgePostContext
{
	uint8_t* tileData[2][EDGE_POST_MAX_TILES];					// (double-buffered) pointer to tile pixel data
	EdgeDmaListElement* tileDmaList[2][EDGE_POST_MAX_TILES];	// (double-buffered) pointer to DmaList areas for source tile input
	uint32_t dmaListSize[EDGE_POST_MAX_TILES];					// DmaList size for each input
	uint8_t tileDirection[EDGE_POST_MAX_TILES];					// direction of tile ( in, out ... )
	void* effectCodeStart;										// tile main entry point
	void* effectParameters;										// points to effect parameters
} __attribute__((__aligned__(16))) EdgePostContext;

// Initialize edgePost state for workload
void _edgePostInitializeWorkload( uint32_t workloadEa );

// Get current configuration
EdgePostSpuConfig& _edgePostGetSpuConfig();

// Get current memory layout
EdgePostContext& _edgePostGetCurrentContext();

// Allocate first tile to work on
uint16_t _edgePostAllocateFirstTile();

// Allocate next tile to work on
uint16_t _edgePostAllocateNextTile();

// Complete processing of tile
void _edgePostCompleteTile();

// Initialize memory layout for a particular stage
void _edgePostInitializeContext( const EdgePostProcessStage& stage, EdgePostContext& context );

// Build a DMA list to bring tile data from main memory to LS (or vice versa)
void _edgePostBuildTileDmaList( EdgeDmaListElement* pDmaList, uint32_t tileX, uint32_t tileY, uint32_t sourceIndex, const EdgePostProcessStage& stage) ;

// Process one tile
void _edgePostExpandBorders( uint32_t tileX, uint32_t tileY, const EdgePostProcessStage& stage, 
							uint8_t** pInput, uint8_t** pBorderCorrectedInput );

// DMA functions (interrupt safe)
void _edgePostDmaStall( int tag );
void _edgePostDmaLargeGet( void *ls, uint64_t ea, uint32_t size, uint32_t tag );
void _edgePostDmaGet( void *ls, uint64_t ea, uint32_t size, uint32_t tag );
void _edgePostDmaListGet( void *ls, const EdgeDmaListElement *list,	uint32_t listSize, uint32_t tag );
void _edgePostDmaListPut( const void *ls, const EdgeDmaListElement *list, uint32_t listSize, uint32_t tag );

#endif /* __EDGEPOST_INTERNALS_H__ */
