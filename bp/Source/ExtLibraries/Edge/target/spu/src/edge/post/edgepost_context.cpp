/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/edge_stdint.h"
#include <string.h>
#include <spu_intrinsics.h>
#include "edge/edge_printf.h"
#include "edge/edge_assert.h"
#include "edge/post/edgepost.h"
#include "edgepost_internals.h"

static inline uint8_t* align( const uint8_t* value, uint32_t alignment)
{
	return (uint8_t*)( ( uint32_t(value) + (alignment - 1)) & ~(alignment - 1));
}

static inline uint32_t align( const uint32_t value, uint32_t alignment)
{
	return ( value + (alignment - 1)) & ~(alignment - 1);
}

void _edgePostInitializeContext( const EdgePostProcessStage& stage, EdgePostContext& context )
{
	// access configuration
	const EdgePostSpuConfig& config = _edgePostGetSpuConfig();
	
	// reset context
	memset( &context, 0, sizeof(EdgePostContext));

	// get a heap pointer
	uint8_t* pHeap = (uint8_t*)( config.heapStart );

	// where the effect code is loaded
	EDGE_ASSERT( ( stage.effectCodeSize & 0xf) == 0 );
	context.effectCodeStart = pHeap;
	pHeap += stage.effectCodeSize;

	// reserve space for effect parameters
	EDGE_ASSERT( ( stage.stageParametersSize & 0xf) == 0 );
	context.effectParameters = pHeap;
	pHeap += stage.stageParametersSize;

	// needs double-buffering ?
	const int numBuffers = ( stage.numTileX * stage.numTileY ) > 1 ? 2 : 1;

	// Setup tile pointers
	for ( int i = 0; i < EDGE_POST_MAX_TILES; ++i )
	{
		// get reference to tile info
		const EdgePostSourceTile& tile = stage.sources[i];

		// process if enabled
		if ( tile.addressEa != 0 )
		{
			// calculate occupancy
			uint32_t stride = ( tile.borderWidth * 2 + tile.width ) * tile.bytesPerPixel;
			const uint32_t numScanline = tile.borderHeight * 2 + tile.height;
			const uint32_t dataSize = tile.multiplier * stride * numScanline;
			context.tileDirection[i] = tile.type;

			// double-check datasize is aligned
			EDGE_ASSERT( ( dataSize & 0xf) == 0 );

			// fill context
			for ( int j = 0; j < numBuffers; ++j )
			{
				if ( tile.type == EDGE_POST_TILE_SCRATCH && j != 0 )
				{
					// reuse data area
					context.tileData[j][i] = context.tileData[0][i];

				} else {
					
					// allocate data area
					context.tileData[j][i] = pHeap; 
					pHeap += dataSize;
				}
				
				// select DmaList area based on tile direction
				switch ( tile.type )
				{
				case EDGE_POST_TILE_INPUT:
					// overlap DmaList with tile data
					context.dmaListSize[i] = numScanline * 8;
					context.tileDmaList[j][i] = 
						(EdgeDmaListElement*)( context.tileData[j][i] + ( dataSize - align( context.dmaListSize[i], 16) ));
					break;

				case EDGE_POST_TILE_OUTPUT:
					// allocate separate DmaList area
					context.dmaListSize[i] = numScanline * 8;
					context.tileDmaList[j][i] = (EdgeDmaListElement*)pHeap; 
					pHeap += context.dmaListSize[i]; 
					break;

				case EDGE_POST_TILE_SCRATCH:
					// there is no DmaList area
					break;

				default:
					EDGE_ASSERT( 0 && "Unreacheable" );
				}
				
				// make sure this is aligned
				pHeap = align( pHeap, 16);
			}
		}
	}

	//	Check we haven't overflow
	EDGE_ASSERT( uint32_t( pHeap - uint32_t(config.heapStart)) <= config.heapSize );
}
