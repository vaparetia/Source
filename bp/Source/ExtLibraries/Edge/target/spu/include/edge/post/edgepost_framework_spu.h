/* SCE CONFIDENTIAL
* PlayStation(R)Edge 1.2.0
* Copyright (C) 2009 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#ifndef __EDGE_POST_FRAMEWORK_SPU_H__
#define __EDGE_POST_FRAMEWORK_SPU_H__

#include "edge/edge_stdint.h"
#include "edge/post/edgepost.h"

// edgePostExecuteProcessing return codes
#define EDGEPOST_WORKLOAD_ENDED			0
#define EDGEPOST_WORKLOAD_PREEMPTED		1
#define EDGEPOST_WORKLOAD_IDLE			2

// Edge Post tile callback
struct EdgePostTileInfo;
typedef void (*EdgePostTileCallback)( EdgePostTileInfo* tileInfo );

// Edge Post pool for preemption callback
typedef unsigned int (*EdgePostPollCallback)();


// callback that gets called when a new stage starts
typedef EdgePostTileCallback (*EdgePostStageEnterCallback)( const EdgePostProcessStage* /*stage*/, void* /*effectCodeArea*/ );


// callback that gets called when current stage ends
typedef void (*EdgePostStageExitCallback)( const EdgePostProcessStage* /*stage*/, void* /*effectCodeArea*/ );


// callback that gets called when the current workload is finished, note that this callback is called only on one SPU
typedef void (*EdgePostWorkloadEndCallback)();



// input structure passed as parameter to main tile function
struct EdgePostTileInfo
{
	EdgePostProcessStage* stage;
	uint32_t tile_x;
	uint32_t tile_y;
	uint8_t* tiles[EDGE_POST_MAX_TILES];
	void* parameters;
};


// edgepost configuration
struct EdgePostSpuConfig
{
	void* heapStart;
	uint32_t heapSize;
	uint16_t controlDmaTag;
	uint16_t inputDmaTag;
	uint16_t outputDmaTag;

	// callbacks
	EdgePostPollCallback pollCallback;
	EdgePostStageEnterCallback stageEnterCallback;
	EdgePostStageExitCallback stageExitCallback;
	EdgePostWorkloadEndCallback workloadEndCallback;
} __attribute__((__aligned__(16)));


// Set configuration
void edgePostSetSpuConfig( const EdgePostSpuConfig* config );


// Main loop
int edgePostRunWorkload( uint32_t workloadEa );


// Get currently processed stage
EdgePostProcessStage* edgePostGetCurrentStage();


#endif // __EDGE_POST_SPU_H__
