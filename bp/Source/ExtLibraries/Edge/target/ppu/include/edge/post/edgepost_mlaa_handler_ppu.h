/* SCE CONFIDENTIAL
* PlayStation(R)Edge 1.2.0
* Copyright (C) 2010 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#ifndef EDGE_POST_MLAA_HANDLER_H
#define EDGE_POST_MLAA_HANDLER_H

// PPU side management code for MLAA. Feel free to roll your own.

#include <cell/spurs.h>
#include <cell/spurs/task.h>
#include <edge/post/edgepost_mlaa.h>

struct EdgePostMlaaTaskParameters;

struct EdgePostMlaaContext
{
	uint32_t							spuCount;
	CellSpurs*							spurs;
	CellSpursTaskset*					taskSet;
	CellSpursTaskId*					taskIds;
	CellSpursTaskArgument*				taskArguments;
	EdgePostMlaaTaskParameters*			taskParameters;
	CellSpursTaskSaveConfig*			saveConfigs;
	CellSpursBarrier*					barrier;
	int32_t*							directionLock;
	uint32_t							rsxLabel;
	volatile uint32_t*					rsxLabelAddress;
	uint32_t							tasksReady;
};

int edgePostMlaaInitializeContext(
								   EdgePostMlaaContext* context, 
								   uint32_t spus, 
								   CellSpurs* spurs, 
								   const uint8_t *priorities, 
								   uint32_t rsxLabel,
								   void* memBlock,
								   size_t memBlockSize);

int edgePostMlaaDestroyContext(EdgePostMlaaContext* context);

void edgePostMlaaPrepareWithRelativeThreshold(
								   EdgePostMlaaContext* context,
								   const void *src, 
								   void *dst,
								   uint32_t width, 
								   uint32_t height,
								   uint32_t pitch,
								   uint8_t base,
								   uint8_t scale,
								   uint32_t mode,
								   uint32_t rsxLabelValue);

// We use an RSX->PPU interrupt to wake up the tasks.
// This is a valid way of doing it if you are using MLAA only a few
// times per frame. Otherwise, you may have to do something more sophisticated,
// like using the SPURS workload flags to wake up a workload that wakes up your
// tasks.

void edgePostMlaaKickTasks(EdgePostMlaaContext* context);	// start the SPUs.

// This method will spin the PPU until mlaa work has finished

void edgePostMlaaWait(EdgePostMlaaContext* context);

// this is the canonical value to pass into the RSX user callback function to indicate that
// an MLAA processing is required. You still need to write that callback, check for this value,
// and kick the tasks.
#define EDGE_POST_MLAA_USER_COMMAND 0xF5AAC0DE				// Anyone know a hex way of writing 'M'?

#define EDGE_POST_MLAA_ALIGN(alignment,value) (( (uint32_t)value + (alignment - 1)) & ~(alignment - 1))

#define EDGE_POST_MLAA_HANDLER_TASK_SAVE_SIZE 3072

#define EDGE_POST_MLAA_HANDLER_BUFFER_ALIGN 128

#define EDGE_POST_MLAA_HANDLER_BASE_BUFFER_SIZE ( \
	EDGE_POST_MLAA_ALIGN(CELL_SPURS_BARRIER_ALIGN,CELL_SPURS_TASKSET_SIZE) + \
	EDGE_POST_MLAA_ALIGN(CELL_SPURS_BARRIER_SIZE,128) + \
	128)

#define EDGE_POST_MLAA_HANDLER_SPU_BUFFER_SIZE(spus) ( \
	EDGE_POST_MLAA_ALIGN(128, \
		EDGE_POST_MLAA_ALIGN(16,spus * (sizeof(CellSpursTaskSaveConfig) + sizeof(CellSpursTaskArgument) + sizeof(CellSpursTaskId))) + \
		EDGE_POST_MLAA_ALIGN(CELL_SPURS_TASK_ATTRIBUTE_ALIGN,spus * sizeof(EdgePostMlaaTaskParameters))) + \
	spus * EDGE_POST_MLAA_HANDLER_TASK_SAVE_SIZE)

#endif //EDGE_POST_MLAA_HANDLER_H
