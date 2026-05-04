/* SCE CONFIDENTIAL
* PlayStation(R)Edge 1.2.0
* Copyright (C) 2010 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#include <stdlib.h>
#include <string.h>
#include <cell/gcm.h>

#include <edge/edge_assert.h>
#include <edge/post/edgepost_mlaa_handler_ppu.h>
#include <edge/post/edgepost_mlaa.h>


extern char _binary_task_edgepost_mlaa_task_spu_elf_start[];					// MLAA code 
extern char _binary_task_edgepost_mlaa_task_spu_elf_size[];						// MLAA code size

// Note: If you modify the SPU code, depending on the change, the minimal context yield 
// might not be enough and a larger context area might need to be stored.
static const CellSpursTaskLsPattern g_LsPattern = { { 0x00000000, 0x00000000, 0x00000000, 0x00000001 } };	// last 2K for stack

int edgePostMlaaDestroyContext(EdgePostMlaaContext* context)
{
	if(context->taskSet)
	{
		int ret = cellSpursShutdownTaskset(context->taskSet);
		EDGE_ASSERT( CELL_OK == ret );
		if(CELL_OK != ret ) return ret;
		ret = cellSpursJoinTaskset(context->taskSet);
		EDGE_ASSERT( CELL_OK == ret );
		if(CELL_OK != ret ) return ret;
	}
	memset(context,0,sizeof(context));
	return CELL_OK;
}


int edgePostMlaaInitializeContext(
	EdgePostMlaaContext* context, 
	uint32_t spus, 
	CellSpurs* spurs, 
	const uint8_t *priorities, 
	uint32_t rsxLabel,
	void* memblock,
	size_t memblockSize)
{
	char *freePointer = (char*)memblock;
	memset(context,0,sizeof(context)); // Just so that if anything goes wrong, you can destroy the context.
	context->spuCount = spus;
	context->spurs = spurs;

	context->taskSet = (CellSpursTaskset*)freePointer;
	freePointer += CELL_SPURS_TASKSET_SIZE;	

	CellSpursTasksetAttribute attributeTaskset;
	cellSpursTasksetAttributeInitialize( 
		&attributeTaskset,
		0,
		priorities,
		context->spuCount);

	cellSpursTasksetAttributeSetName(&attributeTaskset,"EDGE MLAA");

	int ret = cellSpursCreateTasksetWithAttribute( context->spurs, context->taskSet, &attributeTaskset );
	if(CELL_OK != ret)
	{
		EDGE_ASSERT(!"MLAA: task set creation failed");
		return ret;
	}

	freePointer = (char*)EDGE_POST_MLAA_ALIGN(CELL_SPURS_BARRIER_ALIGN,freePointer);
	context->barrier = (CellSpursBarrier*)freePointer;
	freePointer += CELL_SPURS_BARRIER_SIZE;

	ret = cellSpursBarrierInitialize(context->taskSet,context->barrier,context->spuCount);
	if(CELL_OK != ret)
	{
		EDGE_ASSERT(!"MLAA: barrier creation failed");
		return ret;
	}

	// initialize direction lock used during the transpose pass.
	freePointer = (char*)EDGE_POST_MLAA_ALIGN(128,freePointer);
	context->directionLock = (int32_t*)freePointer;
	context->directionLock[0] = 0;
	freePointer += 128;

	EDGE_ASSERT(EDGE_POST_MLAA_HANDLER_BASE_BUFFER_SIZE+(char*)memblock==freePointer);

	// now, on to the per SPU setup.
	// In your game, you want this all to be statically allocated. There is little use in being able to set the
	// spu count at runtime.
	context->saveConfigs     = (CellSpursTaskSaveConfig*)freePointer;
	freePointer += context->spuCount*sizeof(CellSpursTaskSaveConfig);

	context->taskArguments   = (CellSpursTaskArgument*)freePointer;
	freePointer += context->spuCount*sizeof(CellSpursTaskArgument);

	context->taskIds         = (CellSpursTaskId*)freePointer;
	freePointer += context->spuCount*sizeof(CellSpursTaskId);

	freePointer = (char*)EDGE_POST_MLAA_ALIGN(16, freePointer);
	context->taskParameters  = (EdgePostMlaaTaskParameters*)freePointer;
	freePointer += context->spuCount*sizeof(EdgePostMlaaTaskParameters);

	// Create spurs tasks
	for ( uint32_t t = 0; t < context->spuCount; ++t )
	{
		// context save area
		ret = cellSpursTaskGetContextSaveAreaSize( &context->saveConfigs[t].sizeContext, &g_LsPattern);
		if(CELL_OK != ret)
		{
			EDGE_ASSERT(!"MLAA: context save area computation failed");
			return ret;
		}
		EDGE_ASSERT(EDGE_POST_MLAA_HANDLER_TASK_SAVE_SIZE == context->saveConfigs[t].sizeContext);

		freePointer = (char*)EDGE_POST_MLAA_ALIGN(128, freePointer);
		void *context_save_buffer = freePointer;
		context->saveConfigs[t].eaContext = context_save_buffer;
		context->saveConfigs[t].lsPattern = &g_LsPattern;

		freePointer += context->saveConfigs[t].sizeContext;

		

		context->taskArguments[t].u32[0]        = (uint32_t)&context->taskParameters[t];
		CellSpursTaskAttribute taskAttribute;
		cellSpursTaskAttributeInitialize(
			&taskAttribute,
			_binary_task_edgepost_mlaa_task_spu_elf_start,
			&context->saveConfigs[t],
			&context->taskArguments[t]
			);

		ret = cellSpursCreateTaskWithAttribute(
			context->taskSet, 
			&context->taskIds[t], 
			&taskAttribute);

		if(CELL_OK != ret)
		{
			EDGE_ASSERT(!"MLAA: task creation failed");
			return ret;
		}
	}

	// initialize rsx label value
	context->rsxLabel = rsxLabel;
	context->rsxLabelAddress = cellGcmGetLabelAddress( rsxLabel);

	context->tasksReady = 1; // to prevent lock-up during the first call to Wait()

	EDGE_ASSERT(memblockSize+(char*)memblock>=freePointer); // verify that the buffer is big enough.

	return CELL_OK;
}

void edgePostMlaaPrepareWithRelativeThreshold(
	EdgePostMlaaContext* context,
	const void *src, 
	void *dst,
	uint32_t width, 
	uint32_t height,
	uint32_t pitch,
	uint8_t thresholdBase,
	uint8_t thresholdScale,
	uint32_t mode,
	uint32_t rsxLabelValue)
{

	for ( uint32_t t = 0; t < context->spuCount; ++t )
	{
		context->taskParameters[t].rsxLabelAddress         = 0;
		context->taskParameters[t].taskCounterAddress      = 0;
		context->taskParameters[t].imageAddress            = (uint32_t)src;
		context->taskParameters[t].destAddress             = (uint32_t)dst;
		context->taskParameters[t].imageWidth              = width;
		context->taskParameters[t].imageHeight             = height;		
		context->taskParameters[t].imagePitch              = pitch;
		context->taskParameters[t].barrierAddress          = (uint32_t)context->barrier;
		context->taskParameters[t].directionLockAddress    = (uint32_t)context->directionLock;
		context->taskParameters[t].mode                    = mode;
		context->taskParameters[t].spuId                   = t;
		context->taskParameters[t].spuCount                = context->spuCount;
		context->taskParameters[t].parameter0              = thresholdBase;
		context->taskParameters[t].parameter1              = thresholdScale;
	}

	context->taskParameters[0].taskCounterAddress = (uint32_t)&context->tasksReady;
	context->taskParameters[0].rsxLabelAddress    = (uint32_t)context->rsxLabelAddress;
	context->taskParameters[0].rsxLabelValue      = rsxLabelValue;
	context->tasksReady= 0;
}

void edgePostMlaaKickTasks(EdgePostMlaaContext* context)
{
	for(uint32_t i = 0; i < context->spuCount; ++i)
	{
		int ret = cellSpursSendSignal(context->taskSet,context->taskIds[i]);
		if(CELL_OK != ret)
		{
			EDGE_ASSERT(!"MLAA: failed to kick task.");
		}
	}
}

void edgePostMlaaWait(EdgePostMlaaContext* context)
{
	volatile EdgePostMlaaContext* ctx = context;
	while (!ctx->tasksReady)
	{
		sys_timer_usleep(30);
	}
}
