/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <string.h>
#include <ppu_intrinsics.h>
#include <sys/process.h>
#include <cell/spurs/task.h>
#include <cell/spurs/lfqueue.h>

#include "edge/edge_printf.h"
#include "edge/edge_assert.h"
#include "edge/zlib/edgezlib_ppu.h"
#include "edge/zlib/edgezlib_deflate_queue_element.h"

//======================================================================================================

extern char _binary_task_edgezlib_deflate_task_spu_elf_start[];

//======================================================================================================

/** 
 * @brief	Create a SPURS Task for performing compression on one SPU.
 *			The task will pull work work off the Deflate Queue when work exists.
 *			If there is no work to do the task will sleep. It will store its context to the location
 *			specified by pTaskContext.
 *
 * @note	Create one Deflate Task for each SPU you want to run. So, if you want compression to be
 *			able to run in parallel on 6 SPUs (and your SPURS Instance has 6 SPUs in it) then you
 *			should creat 6 Deflate Tasks - all in the same taskset, and all working on the same
 *			Deflate Queue.
 *
 * @param	pTaskSet			The taskset that this Deflate Task should be attached to.
 * @param	pTaskContext		The main memory buffer that the task uses for storing its context to.
 *								The required size of this buffer can be queried by calling
 *								"edgeZlibGetDeflateTaskContextSaveSize()" or
 *								"EDGE_ZLIB_GET_DEFLATE_TASK_CONTEXT_SAVE_SIZE()".
 *								Buffer must be 16 byte aligned.
 * @param	handle				The handle for the Deflate Queue that this task will be pulling from.
 *
 * @return						The task id of the created SPURS Task.
 */

CellSpursTaskId edgeZlibCreateDeflateTask(
		CellSpursTaskset*		pTaskSet,
		void*					pTaskContext,
		EdgeZlibDeflateQHandle	handle )
{
	CellSpursTaskId taskId;
	CellSpursTaskArgument taskArgument;
	taskArgument.u32[0] = (uint32_t)handle;

	static const CellSpursTaskLsPattern s_deflateTaskLsPattern = { { 0x00000000, 0x00000000, 0x00000000, 0x00000001 } };	// last 2K for stack

	EDGE_ASSERT( EDGE_ZLIB_GET_DEFLATE_TASK_CONTEXT_SAVE_SIZE() == edgeZlibGetDeflateTaskContextSaveSize() );	//Just checking our macro is correct

	int ret = cellSpursCreateTask(	pTaskSet,
									&taskId,
									_binary_task_edgezlib_deflate_task_spu_elf_start,
									pTaskContext,
									EDGE_ZLIB_GET_DEFLATE_TASK_CONTEXT_SAVE_SIZE(),
									&s_deflateTaskLsPattern,
									&taskArgument );
	EDGE_ASSERT( CELL_OK == ret );
	(void) ret;

	return taskId;
}

//======================================================================================================

/** 
 * @brief	Returns the required buffer size needed by one Deflate Task for storing its context into.
 *
 * @note	The allocated buffer must be 16 byte aligned.
 *
 * @return						The size of the buffer needed for storing the context data of one
 *								Deflate Task.
 */

uint32_t edgeZlibGetDeflateTaskContextSaveSize( void )
{
	return CELL_SPURS_TASK_CONTEXT_SIZE( 1/*#bits in s_taskLsPattern above*/ * 2048 );
}

//======================================================================================================

/** 
 * @brief	Create a Deflate Queue. This will hold the list of work that is queued up for the
 *			Deflate Task(s) to work on.
 *
 * @note	The queue is a FIFO and will stall as necessary if work is pushed onto a full queue,
 *			so the queue size can safely be lower than the actual maximum number of elements needed.
 *
 * @param	pSpurs				The SPURS instance that this Deflate Queue will be associated with.
 * @param	maxNumQueueEntries	The maximum number of entries this queue will hold.
 *								(maximum: 32767)
 * @param	pBuffer				The buffer in main memory to be used for this Deflate Queue.
 *								Must be aligned to 128 bytes.
 * @param	bufferSize			The size of the provided buffer in main memory.
 *								The required size of this buffer for a given number of queue elements
 *								can be queried by calling "edgeZlibGetDeflateQueueSize" or
 *								"EDGE_ZLIB_GET_DEFLATE_QUEUE_SIZE".
 *
 * @return						The handle of the created Deflate Queue.
 */

EdgeZlibDeflateQHandle edgeZlibCreateDeflateQueue(
			CellSpurs*					pSpurs,
			uint32_t					maxNumQueueEntries,
			void*						pBuffer,
			uint32_t					bufferSize )
{
	EDGE_ASSERT( bufferSize == edgeZlibGetDeflateQueueSize( maxNumQueueEntries ) );
	EDGE_ASSERT( bufferSize == EDGE_ZLIB_GET_DEFLATE_QUEUE_SIZE( maxNumQueueEntries ) );	//Just checking our macro is correct
	EDGE_ASSERT( (((uint32_t)pBuffer) & 0x7F) == 0 );

	memset( pBuffer, 0, bufferSize );

	//Passed in buffer is used for the CellSpursLFQueue first, followed by the LFQueue buffer
	CellSpursLFQueue* pLfQueue	= (CellSpursLFQueue*) pBuffer;
	void* pLfQueueBuffer		= &pLfQueue[1];

	int ret = cellSpursLFQueueInitializeIWL(	pSpurs,
												pLfQueue,
												pLfQueueBuffer,
												sizeof(EdgeZlibDeflateQueueElement),
												maxNumQueueEntries,
												CELL_SPURS_LFQUEUE_ANY2ANY );
	EDGE_ASSERT( CELL_OK == ret );

	ret = cellSpursLFQueueAttachLv2EventQueue( pLfQueue );
	EDGE_ASSERT( CELL_OK == ret );

	return pLfQueue;
}

//======================================================================================================

/** 
 * @brief	Returns the required buffer size needed for a Deflate Queue which can hold at most the
 *			specified number of elements at one time.
 *
 * @note	The allocated buffer must be 128 byte aligned.
 *
 * @param	maxNumQueueEntries	The maximum number of entries the queue will hold at one time.
 *								(maximum: 32767)
 *
 * @return						The size of the buffer needed for storing the Deflate Queue.
 */

uint32_t edgeZlibGetDeflateQueueSize( uint32_t maxNumQueueEntries )
{
	EDGE_ASSERT( maxNumQueueEntries > 0 );
	uint32_t size = sizeof(CellSpursLFQueue) + (maxNumQueueEntries * sizeof(EdgeZlibDeflateQueueElement));
	return size;
}

//======================================================================================================

/** 
 * @brief	Add a new piece of work to the Deflate Queue. The work added to this queue will be taken
 *			by one of the Deflate Tasks at the next chance they get.
 *
 * @note	If the queue is full (ie. reached maxNumQueueEntries), and another item is pushed onto the
 *			queue, then this function will block until there is space.
 *
 * @note	The work-to-do-counter can be used to track completion of the processing on a collection
 *			of segments by initializing it to however many segments are being waited on. Then when
 *			each item completes it will decrement by one, and so when the value reaches zero all
 *			segments will have been done. At this point the specified event flag will be set.
 *			Note that if the SPU has an error with compressed data it will set the high bit of the
 *			counter.
 *
 * @note	If pWorkToDoCounter is NULL, but pEventFlag is valid, then the event flag will be set
 *			after this segment is done.
 *
 * @param	handle						The handle of the Deflate Queue that the entry will be pushed
 *										onto.
 * @param	pInputUncompressedData		The Effective Address of the input data which is to be
 *										compressed.
 * @param	uncompressedSize			The size of the uncompressed input data.
 * @param	pOutputCompressedData		The Effective Address of the output buffer for the
 *										compressed data.
 * @param	maxCompressedOutputSize		The maximum space available for the compressed data.
 * @param	pOutputCompressedSize		The Effective Address of the uint32_t into which the output
 *										compressed size will be written. The top bit of the output
 *										size indicates whether the data was stored compressed or if the
 *										uncompressed original data was chosen for storing.
 * @param	pWorkToDoCounter			A counter in main memory which will be atomically decremented
 *										after this item has been compressed.
 *										If the SPU has an error with the compression it will set the
 *										high bit of the counter to alert the fact that an error occurred.
 *										Can be NULL if you're not interested.
 * @param	pEventFlag					The event flag to set.
 *										Can be NULL if you're not interested.
 * @param	eventFlagBits				The value to set to the event flag.
 * @param	level						Choose the compression level between 0 and 9
 *											0 - no compression
 *											1 - fastest
 *											9 - smallest
 * @param	processing					If compression makes the data bigger, choose whether to store
 *										the compressed or the original data.
 *										Also, when compressing, choose whether to store the data with
 *										zlib header and footer, or whether to store just the 'raw'
 *										compressed data without any header or footer.
 */

void edgeZlibAddDeflateQueueElement(
		EdgeZlibDeflateQHandle			handle,
		const void*						pInputUncompressedData,
		uint32_t						uncompressedSize,
		void*							pOutputCompressedData,
		uint32_t						maxCompressedOutputSize,
		uint32_t*						pOutputCompressedSize,
		uint32_t*						pWorkToDoCounter,
		CellSpursEventFlag*				pEventFlag,
		uint16_t						eventFlagBits,
		uint32_t						level,
		EdgeZlibDeflateTaskProcessing	processing )
{
	EdgeZlibDeflateQueueElement queueEntry;

	EDGE_ASSERT_MSG( !sys_process_is_stack((void*)pInputUncompressedData),
		("EDGE ZLIB ERROR: Input uncompressed data is on stack (which is illegal)\n") );
	
	EDGE_ASSERT_MSG( !sys_process_is_stack(pOutputCompressedData),
		("EDGE ZLIB ERROR: Output compressed data is on stack (which is illegal)\n") );

	EDGE_ASSERT( (pWorkToDoCounter == NULL) || ((*pWorkToDoCounter) > 0) );	//If a counter is passed, then being zero already is clearly a bug

	EDGE_ASSERT( (((uint32_t)pWorkToDoCounter) & 0x3) == 0 );

	queueEntry.m_eaInputUncompressedData		= (uint32_t)pInputUncompressedData;
	queueEntry.m_eaOutputCompressedData			= (uint32_t)pOutputCompressedData;
	queueEntry.m_uncompressedSize				= uncompressedSize;
	queueEntry.m_maxCompressedOutputSize		= maxCompressedOutputSize;
	queueEntry.m_eaOutputCompressedSize			= (uint32_t)pOutputCompressedSize;
	queueEntry.m_eaWorkToDoCounter				= ((uint32_t)pWorkToDoCounter) | processing;
	queueEntry.m_eaEventFlag					= (uint32_t)pEventFlag;
	queueEntry.m_eventFlagBits					= eventFlagBits;
	queueEntry.m_compressionLevel				= level;
	queueEntry.m_pad8							= 0;

	__lwsync();									//Ensure store to queueEntry is complete before task can run

	CellSpursLFQueue* pLfQueue = (CellSpursLFQueue*) handle;

	int ret = cellSpursLFQueuePush( pLfQueue, &queueEntry );
	(void) ret;
	EDGE_ASSERT( CELL_OK == ret );
}

//======================================================================================================

/** 
 * @brief	Try to add a new piece of work to the Deflate Queue. The work added to this queue will be taken
 *			by one of the Deflate Tasks at the next chance they get.
 *
 * @note	If the queue is full (ie. reached maxNumQueueEntries), and another item is pushed onto the
 *			queue, then this function will return with status alerting you to this.
 *
 * @note	The work-to-do-counter can be used to track completion of the processing on a collection
 *			of segments by initializing it to however many segments are being waited on. Then when
 *			each item completes it will decrement by one, and so when the value reaches zero all
 *			segments will have been done. At this point the specified event flag will be set.
 *			Note that if the SPU has an error with compressed data it will set the high bit of the
 *			counter.
 *
 * @note	If pWorkToDoCounter is NULL, but pEventFlag is valid, then the event flag will be set
 *			after this segment is done.
 *
 * @param	handle						The handle of the Deflate Queue that the entry will be pushed
 *										onto.
 * @param	pInputUncompressedData		The Effective Address of the input data which is to be
 *										compressed.
 * @param	uncompressedSize			The size of the uncompressed input data.
 * @param	pOutputCompressedData		The Effective Address of the output buffer for the
 *										compressed data.
 * @param	maxCompressedOutputSize		The maximum space available for the compressed data.
 * @param	pOutputCompressedSize		The Effective Address of the uint32_t into which the output
 *										compressed size will be written. The top bit of the output
 *										size indicates whether the data was stored compressed or if the
 *										uncompressed original data was chosen for storing.
 * @param	pWorkToDoCounter			A counter in main memory which will be atomically decremented
 *										after this item has been compressed.
 *										If the SPU has an error with the compression it will set the
 *										high bit of the counter to alert the fact that an error occurred.
 *										Can be NULL if you're not interested.
 * @param	pEventFlag					The event flag to set.
 *										Can be NULL if you're not interested.
 * @param	eventFlagBits				The value to set to the event flag.
 * @param	level						Choose the compression level between 0 and 9
 *											0 - no compression
 *											1 - fastest
 *											9 - smallest
 * @param	processing					If compression makes the data bigger, choose whether to store
 *										the compressed or the original data.
 *										Also, when compressing, choose whether to store the data with
 *										zlib header and footer, or whether to store just the 'raw'
 *										compressed data without any header or footer.
 *
 * @return								'true' if the item was added. 'false' if it failed to add.
 */

bool edgeZlibTryAddDeflateQueueElement(
		EdgeZlibDeflateQHandle			handle,
		const void*						pInputUncompressedData,
		uint32_t						uncompressedSize,
		void*							pOutputCompressedData,
		uint32_t						maxCompressedOutputSize,
		uint32_t*						pOutputCompressedSize,
		uint32_t*						pWorkToDoCounter,
		CellSpursEventFlag*				pEventFlag,
		uint16_t						eventFlagBits,
		uint32_t						level,
		EdgeZlibDeflateTaskProcessing	processing )
{
	EdgeZlibDeflateQueueElement queueEntry;

	EDGE_ASSERT_MSG( !sys_process_is_stack((void*)pInputUncompressedData),
		("EDGE ZLIB ERROR: Input uncompressed data is on stack (which is illegal)\n") );
	
	EDGE_ASSERT_MSG( !sys_process_is_stack(pOutputCompressedData),
		("EDGE ZLIB ERROR: Output compressed data is on stack (which is illegal)\n") );

	EDGE_ASSERT( (pWorkToDoCounter == NULL) || ((*pWorkToDoCounter) > 0) );	//If a counter is passed, then being zero already is clearly a bug

	EDGE_ASSERT( (((uint32_t)pWorkToDoCounter) & 0x3) == 0 );

	queueEntry.m_eaInputUncompressedData		= (uint32_t)pInputUncompressedData;
	queueEntry.m_eaOutputCompressedData			= (uint32_t)pOutputCompressedData;
	queueEntry.m_uncompressedSize				= uncompressedSize;
	queueEntry.m_maxCompressedOutputSize		= maxCompressedOutputSize;
	queueEntry.m_eaOutputCompressedSize			= (uint32_t)pOutputCompressedSize;
	queueEntry.m_eaWorkToDoCounter				= ((uint32_t)pWorkToDoCounter) | processing;
	queueEntry.m_eaEventFlag					= (uint32_t)pEventFlag;
	queueEntry.m_eventFlagBits					= eventFlagBits;
	queueEntry.m_compressionLevel				= level;
	queueEntry.m_pad8							= 0;

	__lwsync();									//Ensure store to queueEntry is complete before task can run

	CellSpursLFQueue* pLfQueue = (CellSpursLFQueue*) handle;

	int ret = cellSpursLFQueueTryPush( pLfQueue, &queueEntry );

	if ( CELL_SPURS_TASK_ERROR_AGAIN == ret )
	{
		return false;
	}

	EDGE_ASSERT( CELL_OK == ret );
	return true;
}

//======================================================================================================

/** 
 * @brief	Shuts down the Deflate Queue.
 *
 * @param	handle						The handle of the Deflate Queue to shutdown.
 */

void edgeZlibShutdownDeflateQueue( EdgeZlibDeflateQHandle handle )
{
	CellSpursLFQueue* pLfQueue = (CellSpursLFQueue*) handle;

	int ret = cellSpursLFQueueDetachLv2EventQueue( pLfQueue );
	EDGE_ASSERT( CELL_OK == ret );
	(void) ret;
}

//======================================================================================================
