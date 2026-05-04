/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/edge_stdint.h"
#include <spu_intrinsics.h>
#include <string.h>
#include "edge/edge_printf.h"
#include "edge/edge_assert.h"
#include "edge/post/edgePost.h"
#include "edgepost_internals.h"


//	for easier debugging
//	#define USE_STALLING_DMA


// integer division ...
static inline void GetTileXY( const EdgePostProcessStage& stage, uint16_t tileIndex, uint16_t& x, uint16_t& y )
{
	y = tileIndex / stage.numTileX;
	x = tileIndex % stage.numTileX;
}

// main loop
int edgePostRunWorkload( uint32_t workloadEa )
{
	// access configuration
	const EdgePostSpuConfig& config = _edgePostGetSpuConfig();

	// dma tags
	const uint32_t inputDmaTag = config.inputDmaTag;
	const uint32_t outputDmaTag = config.outputDmaTag;

	//
	uint16_t tileIndex, nextTileIndex;
	uint16_t nextTileX = 0, nextTileY = 0;
	uint16_t tileX, tileY;

	// current effect code initialized to a very unlikely pointer
	uint32_t effectCodeEa = 1;

	// Initialise state
	_edgePostInitializeWorkload( workloadEa );
	int returnCode = EDGEPOST_WORKLOAD_ENDED;

	// Loop on stages
	while ( 1 )
	{

		// check for preemption
		if ( config.pollCallback() )
		{
			returnCode = EDGEPOST_WORKLOAD_PREEMPTED;
			break;
		} 

		// allocate first Tile
		tileIndex = _edgePostAllocateFirstTile();

		// check for an idle situation ( another worker is finishing off last tile of current stage )
		if ( tileIndex == EDGEPOST_TILE_NOT_READY)
		{
			returnCode = EDGEPOST_WORKLOAD_IDLE;
			break;
		}

		// check for termination
		if ( tileIndex == EDGEPOST_INVALID_STAGE)
			break;

		// get current memory layout ( _edgePostAllocateFirstTile sets this up )
		EdgePostContext& context = _edgePostGetCurrentContext();
		EdgePostProcessStage& stage = *edgePostGetCurrentStage();

		// dma stage code from XDR ( if provided )
		if ( stage.effectCodeEa && ( stage.effectCodeEa != effectCodeEa ))
		{
			EDGE_ASSERT( ( stage.effectCodeEa & 0xf ) == 0);
			_edgePostDmaLargeGet( context.effectCodeStart, stage.effectCodeEa, stage.effectCodeDmaSize, inputDmaTag );

#ifdef USE_STALLING_DMA
			_edgePostDmaStall( inputDmaTag );
#endif
			effectCodeEa = stage.effectCodeEa;
		}

		// dma parameters ( if present )
		if ( stage.stageParametersEa != 0 )
		{
			EDGE_ASSERT( stage.stageParametersSize != 0);
			_edgePostDmaLargeGet( context.effectParameters, stage.stageParametersEa, stage.stageParametersSize, inputDmaTag );

#ifdef USE_STALLING_DMA
			_edgePostDmaStall( inputDmaTag );
#endif

		}

		// Calculate tile coordinate
		GetTileXY( stage, tileIndex, tileX, tileY );

		// Start first dma input
		for ( int si = 0; si < EDGE_POST_MAX_TILES; ++si )
		{
			if ( context.dmaListSize[si] && ( context.tileDirection[si] == EDGE_POST_TILE_INPUT))
			{
				// Build dmalist for nextTileIndex
				_edgePostBuildTileDmaList( context.tileDmaList[0][si], tileX, tileY, si, stage);

				// Start next DMA input
				_edgePostDmaListGet( context.tileData[0][si], context.tileDmaList[0][si], context.dmaListSize[si], inputDmaTag );

#ifdef USE_STALLING_DMA
				_edgePostDmaStall( inputDmaTag );
#endif
			}
		}

		// Wait on first DMA input to end
		_edgePostDmaStall( inputDmaTag );

		// Call StageStart callback and get the entry point to main tile processing function
		EDGE_ASSERT( config.stageEnterCallback );
		EdgePostTileCallback tileMainCallback = config.stageEnterCallback( &stage, context.effectCodeStart );

		// Double buffering index
		uint32_t doubleBufferIndex = 0;
		uint32_t processedTile = 0;

		// Loop on Tile
		while( 1 )
		{
			// Allocate next tile
			nextTileIndex = _edgePostAllocateNextTile();

			// Wait for previous DMA input to end
			_edgePostDmaStall( inputDmaTag );
		
			// Start next tile DMA input
			if ( nextTileIndex != EDGEPOST_INVALID_TILE )
			{
				// Calculate tile coordinate
				GetTileXY( stage, nextTileIndex, nextTileX, nextTileY );

				// loop on sources
				uint32_t inputBufferIndex = doubleBufferIndex ^ 1;
				for ( int si = 0; si < EDGE_POST_MAX_TILES; ++si )
				{
					if ( context.dmaListSize[si] && ( context.tileDirection[si] == EDGE_POST_TILE_INPUT) )
					{
						// Build dmalist for nextTileIndex
						_edgePostBuildTileDmaList( context.tileDmaList[inputBufferIndex][si], 
							nextTileX, nextTileY, si, stage);

						// Start next DMA input
						_edgePostDmaListGet( context.tileData[inputBufferIndex][si],
							context.tileDmaList[inputBufferIndex][si], 
							context.dmaListSize[si], inputDmaTag);
#ifdef USE_STALLING_DMA
						_edgePostDmaStall( inputDmaTag );
#endif
					}
				}
			}

			// Fill in current tile info
			EdgePostTileInfo tileInfo;
			tileInfo.tile_x = tileX;
			tileInfo.tile_y = tileY;
			tileInfo.stage = &stage;
			tileInfo.parameters = context.effectParameters;

			// Process current tile borders
			_edgePostExpandBorders( tileX, tileY, stage, context.tileData[doubleBufferIndex], tileInfo.tiles );

			// Check for debug flag and breakpoint
			if ( __builtin_expect( stage.flags & EDGE_POST_BREAKPOINT, 0) )
				spu_stop( 0x3fff );

			// call tile callback
			tileMainCallback( &tileInfo );

			// Build output DmaLists
			for ( int oi = 0; oi < EDGE_POST_MAX_TILES; ++oi )
			{
				if ( context.dmaListSize[oi] && ( context.tileDirection[oi] == EDGE_POST_TILE_OUTPUT) )
				{
					_edgePostBuildTileDmaList( context.tileDmaList[doubleBufferIndex][oi], tileX, tileY, oi, stage);
				}
			}

			// Wait for previous DMA output
			if ( processedTile++ != 0 )
			{
				// Wait for previous DMA output to end before starting new ones
				_edgePostDmaStall( outputDmaTag );

				// Terminate tile
				_edgePostCompleteTile();
			}

			// Start new DmaList output
			for ( int oi = 0; oi < EDGE_POST_MAX_TILES; ++oi )
			{
				if ( context.dmaListSize[oi] && ( context.tileDirection[oi] == EDGE_POST_TILE_OUTPUT) )
				{
					_edgePostDmaListPut( context.tileData[doubleBufferIndex][oi], 
						context.tileDmaList[doubleBufferIndex][oi], 
						context.dmaListSize[oi], outputDmaTag);
				}
			}
		
#ifdef USE_STALLING_DMA
			// Stall
			_edgePostDmaStall( outputDmaTag );
#endif

			// Check for completion
			if ( nextTileIndex == EDGEPOST_INVALID_TILE )
			{
				// Wait for current DMA output to end
				_edgePostDmaStall( outputDmaTag );

				// Terminate tile
				_edgePostCompleteTile();

				// exit
				break;
			}

			// next tile
			tileIndex = nextTileIndex;
			tileX = nextTileX;
			tileY = nextTileY;

			// Switch buffer
			doubleBufferIndex ^= 1;
		}

		// call StageEnd callback
		EDGE_ASSERT( config.stageExitCallback );
		config.stageExitCallback( &stage, context.effectCodeStart );
	}

	return returnCode;
}
