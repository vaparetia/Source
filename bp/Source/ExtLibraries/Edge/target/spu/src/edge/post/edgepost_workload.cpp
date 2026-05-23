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
#include "edge/edge_atomic.h"
#include "edge/edge_dma.h"

static EdgePostSpuConfig	g_configuration;		// configuration
static EdgePostWorkload		g_workload;				// cached workload in LS
static EdgePostProcessStage	g_stage;				// current processing stage descriptor
static EdgePostContext		g_context;				// current memory layout
static uint32_t				g_workloadEa;			// workload effective address
static uint16_t				g_stageIndex;			// current process stage index
static uint64_t				g_atomicLine[16] __attribute__(( aligned(128)));

void edgePostSetSpuConfig( const EdgePostSpuConfig* config )
{
	g_configuration = *config;
}

// Get current configuration
EdgePostSpuConfig& _edgePostGetSpuConfig()
{
	return g_configuration;
}

// Get current memory layout
EdgePostContext& _edgePostGetCurrentContext()
{
	return g_context;
}

// Get current stage
EdgePostProcessStage* edgePostGetCurrentStage()
{
	return &g_stage;
}

// fetch postprocessing stage
void _edgePostFetchStage( uint16_t stage  )
{
	const uint32_t controlTag = g_configuration.controlDmaTag;

	if ( g_stageIndex != stage )
	{
		const uint64_t ea = g_workload.processStageArrayEa + ( sizeof(EdgePostProcessStage) * stage);
		_edgePostDmaGet( &g_stage, ea, sizeof(EdgePostProcessStage), controlTag );
		_edgePostDmaStall( controlTag );
		g_stageIndex = stage;
	}
}

// initialize workload
void _edgePostInitializeWorkload( uint32_t workloadEa )
{
	EDGE_ASSERT( ( workloadEa & 0xf) == 0);
	const uint32_t controlTag = g_configuration.controlDmaTag;

	// Get workload data
	_edgePostDmaGet( &g_workload, workloadEa, sizeof(g_workload), controlTag);
	_edgePostDmaStall( controlTag );
	g_stageIndex = EDGEPOST_INVALID_STAGE;
	g_workloadEa = workloadEa;
}

// called to allocate first tile to work on
uint16_t _edgePostAllocateFirstTile()
{
	union 
	{
		uint64_t packed;
		EdgePostWorkload workload;
	};

	// disable interrupts
	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	// Allocate tile
	uint16_t tile;
	do
	{
		// get data
		EDGE_ASSERT( ( g_workloadEa & 0xf) == 0);
		packed = EDGE_ATOMIC_LOCK_LINE_64( (uint64_t*)g_atomicLine, g_workloadEa);

		// check it's not finished
		if ( workload.stage == EDGEPOST_INVALID_STAGE )
		{
			// re-enable interrupts
			if (interruptsEnabled)
			{
				spu_ienable();
			}

			return EDGEPOST_INVALID_STAGE;
		}

		// if there are no more tiles ready
		if ( workload.tile == workload.tileCount )
		{
			// re-enable interrupts
			if (interruptsEnabled)
			{
				spu_ienable();
			}

			return EDGEPOST_TILE_NOT_READY;
		}

		// allocate tile
		tile = workload.tile++;

		// try store
	} while( EDGE_ATOMIC_STORE_CONDITIONAL_64( (uint64_t*)g_atomicLine, g_workloadEa, packed ) );

	// re-enable interrupts
	if (interruptsEnabled)
	{
		spu_ienable();
	}

	// fetch stage informations
	_edgePostFetchStage( workload.stage  );

	// select memory layout
	_edgePostInitializeContext( g_stage, g_context );

	// tile allocation went good
	return tile;
}

// called to allocate a tile to work on
uint16_t _edgePostAllocateNextTile()
{
	union 
	{
		uint64_t packed;
		EdgePostWorkload workload;
	};

	// disable interrupts
	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	uint16_t tile;
	do
	{
		// get data
		EDGE_ASSERT( ( g_workloadEa & 0xf) == 0);
		packed = EDGE_ATOMIC_LOCK_LINE_64( (uint64_t*)g_atomicLine, g_workloadEa);

		// validate some assumptions
		EDGE_ASSERT( workload.stage == g_stageIndex );
		
		// if there are no more tiles ...
		if ( workload.tile == workload.tileCount )
		{
			if (interruptsEnabled)
			{
				spu_ienable();
			}

			return EDGEPOST_INVALID_TILE;
		}

		// allocate tile
		tile = workload.tile++;

		// try store
	} while( EDGE_ATOMIC_STORE_CONDITIONAL_64( (uint64_t*)g_atomicLine, g_workloadEa, packed ) );

	// re-enable interrupts
	if (interruptsEnabled)
	{
		spu_ienable();
	}

	// tile allocation went good
	return tile;
}

// called to signal tile completed
void _edgePostCompleteTile()
{
	union 
	{
		uint64_t packed;
		EdgePostWorkload workload;
	};

	bool bWriteRsxLabel, bLastWorkloadTile;
	bool bNeedRsxLabel = g_stage.flags & EDGE_POST_WRITE_RSX_LABEL;
	uint32_t rsxLabeladdress = g_stage.rsxLabelAddress;
	uint32_t rsxLabelValue = g_stage.rsxLabelValue;
	const uint32_t controlTag = g_configuration.controlDmaTag;

	// disable interrupts
	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	do
	{
		// reset
		bLastWorkloadTile = false;
		bWriteRsxLabel = false;

		// get data
		packed = EDGE_ATOMIC_LOCK_LINE_64( (uint64_t*)g_atomicLine, g_workloadEa);

		// increment completion count
		workload.tileCompleted++;

		// check if stage is finished
		if ( workload.tileCompleted == workload.tileCount )
		{
			// check if RSX label needs to be written
			bWriteRsxLabel = true;

			// advance stage
			workload.stage++;

			// check for last stage
			if ( workload.stage != g_workload.stageCount )
			{
				// prefetch stage descriptor
				_edgePostFetchStage( workload.stage );

				// 	reset tile counters
				workload.tile = 0;
				workload.tileCompleted = 0;
				workload.tileCount = g_stage.numTileX * g_stage.numTileY;
			} else {
				
				// workload is finished and we have processed the last tile
				bLastWorkloadTile = true;

				//	set current stage as invalid, signal termination
				workload.stage = EDGEPOST_INVALID_STAGE;
			}
		}

		// try store
	} while( EDGE_ATOMIC_STORE_CONDITIONAL_64( (uint64_t*)g_atomicLine, g_workloadEa, packed ) );

	// write RSX label if needed
	if ( bWriteRsxLabel && bNeedRsxLabel )
	{
		EDGE_ASSERT( ( rsxLabeladdress & 0xf) == 0);
		vec_uint4 data = spu_splats( rsxLabelValue );
		EDGE_DMA_SMALL_PUT( &data, rsxLabeladdress, 4, controlTag, 0, 0);
		EDGE_DMA_WAIT_TAG_STATUS_ALL( 1 << controlTag );
	}

	// re-enable interrupts
	if (interruptsEnabled)
	{
		spu_ienable();
	}

	// invoke callback
	if (bLastWorkloadTile && g_configuration.workloadEndCallback)
	{
		g_configuration.workloadEndCallback();
	}
}
