/* SCE CONFIDENTIAL
* PlayStation(R)Edge 1.2.0
* Copyright (C) 2010 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#include <sys/timer.h>
#include "edge/edge_assert.h"
#include "edge/post/edgePost.h"
#include "edge/post/edgePost_ppu.h"


// Initialise a workload
void edgePostInitializeWorkload( EdgePostWorkload* pWorkload, const EdgePostProcessStage* pStages, uint16_t stageCount )
{
	EDGE_ASSERT( stageCount >= 1 );
	EDGE_ASSERT( pStages );
	EDGE_ASSERT( pWorkload );

	pWorkload->stage = 0;
	pWorkload->tile = 0;
	pWorkload->tileCompleted = 0;
	pWorkload->tileCount = pStages[0].numTileX * pStages[0].numTileY;
	pWorkload->processStageArrayEa = (uint32_t)pStages;
	pWorkload->stageCount = stageCount;
}


// Query if workload is finished
bool edgePostIsWorkloadFinished( EdgePostWorkload* pWorkload )
{
	EDGE_ASSERT( pWorkload );

	const volatile uint16_t* p = (const volatile uint16_t*)&pWorkload->stage;
	return *p == 0xffff;
}


// Stall for a workload
void edgePostStallForWorkload( EdgePostWorkload* pWorkload )
{
	while( !edgePostIsWorkloadFinished( pWorkload) )
		sys_timer_usleep( 30 );
}




