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
#include "edge/zlib/edgezlib_inflate_queue_element.h"

//======================================================================================================

extern char _binary_task_edgezlib_inflate_task_spu_elf_start[];

//======================================================================================================

/** 
 * @brief	Create a SPURS Task for performing decompression on one SPU.
 *			The task will pull work work off the Inflate Queue when work exists.
 *			If there is no work to do the task will sleep.  It will store its context to the location
 *			specified by pTaskContext.
 *
 * @note	Create one Inflate Task for each SPU you want to run.  So, if you want decompression to be
 *			able to run in parallel on 6 SPUs (and your SPURS Instance has 6 SPUs in it) then you
 *			should creat 6 Inflate Tasks - all in the same taskset, and all working on the same
 *			Inflate Queue.
 *
 * @param	pTaskSet			The taskset that this Inflate Task should be attached to.
 * @param	pTaskContext		The main memory buffer that the task uses for storing its context to.
 *								The required size of this buffer can be queried by calling
 *								"edgeZlibGetInflateTaskContextSaveSize()" or
 *								"EDGE_ZLIB_GET_INFLATE_TASK_CONTEXT_SAVE_SIZE()".
 *								Buffer must be 16 byte aligned.
 * @param	handle				The handle for the Inflate Queue that this task will be pulling from.
 *
 * @return						The task id of the created SPURS Task.
 */

CellSpursTaskId edgeZlibCreateInflateTask(
		CellSpursTaskset*		pTaskSet,
		void*					pTaskContext,
		EdgeZlibInflateQHandle	handle )
{
	CellSpursTaskId taskId;
	CellSpursTaskArgument taskArgument;
	taskArgument.u32[0] = (uint32_t)handle;

	static const CellSpursTaskLsPattern s_inflateTaskLsPattern = { { 0x00000000, 0x00000000, 0x00000000, 0x00000001 } };	// last 2K for stack

	EDGE_ASSERT( EDGE_ZLIB_GET_INFLATE_TASK_CONTEXT_SAVE_SIZE() == edgeZlibGetInflateTaskContextSaveSize() );	//Just checking our macro is correct

	int ret = cellSpursCreateTask(	pTaskSet,
									&taskId,
									_binary_task_edgezlib_inflate_task_spu_elf_start,
									pTaskContext,
									EDGE_ZLIB_GET_INFLATE_TASK_CONTEXT_SAVE_SIZE(),
									&s_inflateTaskLsPattern,
									&taskArgument );
	EDGE_ASSERT( CELL_OK == ret );
	(void) ret;

	return taskId;
}

//======================================================================================================

/** 
 * @brief	Returns the required buffer size needed by one Inflate Task for storing its context into.
 *
 * @note	The allocated buffer must be 16 byte aligned.
 *
 * @return						The size of the buffer needed for storing the context data of one
 *								Inflate Task.
 */

uint32_t edgeZlibGetInflateTaskContextSaveSize( void )
{
	return CELL_SPURS_TASK_CONTEXT_SIZE( 1/*#bits in s_taskLsPattern above*/ * 2048 );
}

uint32_t edgeZlibGetTaskContextSaveSize( void )	//This function is deprecated
{
	return CELL_SPURS_TASK_CONTEXT_SIZE( 1/*#bits in s_taskLsPattern above*/ * 2048 );
}

//======================================================================================================

/** 
 * @brief	Create an Inflate Queue.  This will hold the list of work that is queued up for the
 *			Inflate Task(s) to work on.
 *
 * @note	The queue is a FIFO and will stall as necessary if work is pushed onto a full queue,
 *			so the queue size can safely be lower than the actual maximum number of elements needed.
 *
 * @param	pSpurs				The SPURS instance that this Inflate Queue will be associated with.
 * @param	maxNumQueueEntries	The maximum number of entries this queue will hold.
 *								(maximum: 32767)
 * @param	pBuffer				The buffer in main memory to be used for this Inflate Queue.
 *								Must be aligned to 128 bytes.
 * @param	bufferSize			The size of the provided buffer in main memory.
 *								The required size of this buffer for a given number of queue elements
 *								can be queried by calling "edgeZlibGetInflateQueueSize" or
 *								"EDGE_ZLIB_GET_INFLATE_QUEUE_SIZE".
 *
 * @return						The handle of the created Inflate Queue.
 */

EdgeZlibInflateQHandle edgeZlibCreateInflateQueue(
			CellSpurs*				    pSpurs,
			uint32_t				    maxNumQueueEntries,
			void*					    pBuffer,
			uint32_t				    bufferSize )
{
	EDGE_ASSERT( bufferSize == edgeZlibGetInflateQueueSize( maxNumQueueEntries ) );
	EDGE_ASSERT( bufferSize == EDGE_ZLIB_GET_INFLATE_QUEUE_SIZE( maxNumQueueEntries ) );	//Just checking our macro is correct
	EDGE_ASSERT( (((uint32_t)pBuffer) & 0x7F) == 0 );

	memset( pBuffer, 0, bufferSize );

	//Passed in buffer is used for the CellSpursLFQueue first, followed by the LFQueue buffer
	CellSpursLFQueue* pLfQueue	= (CellSpursLFQueue*) pBuffer;
	void* pLfQueueBuffer		= &pLfQueue[1];

	int ret = cellSpursLFQueueInitializeIWL(	pSpurs,
												pLfQueue,
												pLfQueueBuffer,
												sizeof(EdgeZlibInflateQueueElement),
												maxNumQueueEntries,
												CELL_SPURS_LFQUEUE_ANY2ANY );
	EDGE_ASSERT( CELL_OK == ret );

	ret = cellSpursLFQueueAttachLv2EventQueue( pLfQueue );
	EDGE_ASSERT( CELL_OK == ret );

	return pLfQueue;
}

//======================================================================================================

/** 
 * @brief	Returns the required buffer size needed for an Inflate Queue which can hold at most the
 *			specified number of elements at one time.
 *
 * @note	The allocated buffer must be 128 byte aligned.
 *
 * @param	maxNumQueueEntries	The maximum number of entries the queue will hold at one time.
 *								(maximum: 32767)
 *
 * @return						The size of the buffer needed for storing the Inflate Queue.
 */

uint32_t edgeZlibGetInflateQueueSize( uint32_t maxNumQueueEntries )
{
	EDGE_ASSERT( maxNumQueueEntries > 0 );
	uint32_t size = sizeof(CellSpursLFQueue) + (maxNumQueueEntries * sizeof(EdgeZlibInflateQueueElement));
	return size;
}

//======================================================================================================

/** 
 * @brief	Add a new piece of work to the Inflate Queue.  The work added to this queue will be taken
 *			by one of the Inflate Tasks at the next chance they get.
 *
 * @note	If the queue is full (ie. reached maxNumQueueEntries), and another item is pushed onto the
 *			queue, then this function will block until there is space.
 *
 * @note	The work-to-do-counter can be used to track completion of the processing on a collection
 *			of segments by initializing it to however many segments are being waited on.  Then when
 *			each item completes it will decrement by one, and so when the value reaches zero all
 *			segments will have been done.  At this point the specified event flag will be set.
 *			Note that if the SPU has an error with compressed data it will set the high bit of the
 *			counter.
 *
 * @note	If pWorkToDoCounter is NULL, but pEventFlag is valid, then the event flag will be set
 *			after this segment is done.
 *
 * @note	If the data was stored uncompressed and merely needs moving to its destination, then the
 *			Inflate Task can be used for this by passing the appropriate value for 'processing'.
 *
 * @note	This function expects a pointer to the raw compressed data without any header.
 *
 * @param	handle						The handle of the Inflate Queue that the entry will be pushed
 *										onto.
 * @param	pInputCompressedData		The Effective Address of the input data which is to be
 *										decompressed.
 *										This should be a pointer to the raw data without any header.
 * @param	compressedSize				The size of the compressed input data.
 * @param	pOutputUncompressedData		The Effective Address of the output buffer for the
 *										uncompressed data.
 * @param	expectedUncompressedSize	The expected size of the uncompressed data.
 *										The SPU will assert if this value is incorrect.
 * @param	pWorkToDoCounter			A counter in main memory which will be atomically decremented
 *										after this item has been decompressed.
 *										If the SPU has an error with the compressed data it will set the
 *										high bit of the counter to alert the fact that an error occurred.
 *										Can be NULL if you're not interested.
 * @param	pEventFlag					The event flag to set.
 *										Can be NULL if you're not interested.
 * @param	eventFlagBits				The value to set to the event flag.
 * @param	processing					Choose what kind of processing the task has to do on the data.
 *										Can perform decompression, or merely move the raw data from
 *										pInputCompressedData to pOutputUncompressedData.
 */

void edgeZlibAddInflateQueueElement(
		EdgeZlibInflateQHandle			handle,
		const void*						pInputCompressedData,
		uint32_t						compressedSize,
		void*							pOutputUncompressedData,
		uint32_t						expectedUncompressedSize,
		uint32_t*						pWorkToDoCounter,
		CellSpursEventFlag*				pEventFlag,
		uint16_t						eventFlagBits,
		EdgeZlibInflateTaskProcessing	processing )
{
	edgeZlibAddInflateQueueElementPartialCopyOut(	handle,
													pInputCompressedData,
													compressedSize,
													pOutputUncompressedData,
													0,
													expectedUncompressedSize,
													0,
													pWorkToDoCounter,
													pEventFlag,
													eventFlagBits,
													processing );
}

//======================================================================================================

/** 
 * @brief	Try to add a new piece of work to the Inflate Queue. The work added to this queue will be
 *			taken by one of the Inflate Tasks at the next chance they get.
 *
 * @note	If the item is added, then this function will return true. If the queue is full
 *			(ie. reached maxNumQueueEntries), and another item is pushed onto the queue, then this
 *			function will return false.
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
 * @note	If the data was stored uncompressed and merely needs moving to its destination, then the
 *			Inflate Task can be used for this by passing the appropriate value for 'processing'.
 *
 * @note	This function expects a pointer to the raw compressed data without any header.
 *
 * @param	handle						The handle of the Inflate Queue that the entry will be pushed
 *										onto.
 * @param	pInputCompressedData		The Effective Address of the input data which is to be
 *										decompressed.
 *										This should be a pointer to the raw data without any header.
 * @param	compressedSize				The size of the compressed input data.
 * @param	pOutputUncompressedData		The Effective Address of the output buffer for the
 *										uncompressed data.
 * @param	expectedUncompressedSize	The expected size of the uncompressed data.
 *										The SPU will assert if this value is incorrect.
 * @param	pWorkToDoCounter			A counter in main memory which will be atomically decremented
 *										after this item has been decompressed.
 *										If the SPU has an error with the compressed data it will set the
 *										high bit of the counter to alert the fact that an error occurred.
 *										Can be NULL if you're not interested.
 * @param	pEventFlag					The event flag to set.
 *										Can be NULL if you're not interested.
 * @param	eventFlagBits				The value to set to the event flag.
 * @param	processing					Choose what kind of processing the task has to do on the data.
 *										Can perform decompression, or merely move the raw data from
 *										pInputCompressedData to pOutputUncompressedData.
 * @return								'true' if the item was added.  'false' if it failed to add.
 */

bool edgeZlibTryAddInflateQueueElement(
		EdgeZlibInflateQHandle			handle,
		const void*						pInputCompressedData,
		uint32_t						compressedSize,
		void*							pOutputUncompressedData,
		uint32_t						expectedUncompressedSize,
		uint32_t*						pWorkToDoCounter,
		CellSpursEventFlag*				pEventFlag,
		uint16_t						eventFlagBits,
		EdgeZlibInflateTaskProcessing	processing )
{
	return edgeZlibTryAddInflateQueueElementPartialCopyOut(	handle,
															pInputCompressedData,
															compressedSize,
															pOutputUncompressedData,
															0,
															expectedUncompressedSize,
															0,
															pWorkToDoCounter,
															pEventFlag,
															eventFlagBits,
															processing );
}

//======================================================================================================

/** 
 * @brief	Add a new piece of work to the Inflate Queue.  The work added to this queue will be taken
 *			by one of the Inflate Tasks at the next chance they get.
 *
 * @note	If the queue is full (ie. reached maxNumQueueEntries), and another item is pushed onto the
 *			queue, then this function will block until there is space.
 *
 * @note	The work-to-do-counter can be used to track completion of the processing on a collection
 *			of segments by initializing it to however many segments are being waited on.  Then when
 *			each item completes it will decrement by one, and so when the value reaches zero all
 *			segments will have been done.  At this point the specified event flag will be set.
 *			Note that if the SPU has an error with compressed data it will set the high bit of the
 *			counter.
 *
 * @note	If pWorkToDoCounter is NULL, but pEventFlag is valid, then the event flag will be set
 *			after this segment is done.
 *
 * @note	If the data was stored uncompressed and merely needs moving to its destination, then the
 *			Inflate Task can be used for this by passing the appropriate value for 'processing'.
 *
 * @note	This function expects a pointer to the raw compressed data without any header.
 *
 * @note	This function is very similar to edgeZlibAddInflateQueueElement. The additional parameters
 *			skipOutputBeginSize and skipOutputEndSize exist so that decompression of a segment can
 *			be done, but only a portion of the output may need to be written.
 *
 * @note	The sum of outputUncompSkipBeginSize, outputUncompPartialBuffSize and outputUncompSkipEndSize
 *			gives the expected uncompressed side of the compressed data.  The SPU will assert if this
 *			value is incorrect.
 *
 * @param	handle						The handle of the Inflate Queue that the entry will be pushed
 *										onto.
 * @param	pInputCompressedData		The Effective Address of the input data which is to be
 *										decompressed.
 *										This should be a pointer to the raw data without any header.
 * @param	compressedSize				The size of the compressed input data.
 * @param	pOutputUncompPartialBuff	The Effective Address of the output buffer for the
 *										uncompressed data.
 * @param	outputUncompSkipBeginSize	Don't output the first N bytes of the uncompressed output
 * @param	outputUncompPartialBuffSize	The size of the uncompressed data which will be DMAed out.
 * @param	outputUncompSkipEndSize		Don't output the last  N bytes of the uncompressed output
 * @param	pWorkToDoCounter			A counter in main memory which will be atomically decremented
 *										after this item has been decompressed.
 *										If the SPU has an error with the compressed data it will set the
 *										high bit of the counter to alert the fact that an error occurred.
 *										Can be NULL if you're not interested.
 * @param	pEventFlag					The event flag to set.
 *										Can be NULL if you're not interested.
 * @param	eventFlagBits				The value to set to the event flag.
 * @param	processing					Choose what kind of processing the task has to do on the data.
 *										Can perform decompression, or merely move the raw data from
 *										pInputCompressedData to pOutputUncompressedData.
 */

void edgeZlibAddInflateQueueElementPartialCopyOut(
		EdgeZlibInflateQHandle			handle,
		const void*						pInputCompressedData,
		uint32_t						compressedSize,
		void*							pOutputUncompPartialBuff,
		uint16_t						outputUncompSkipBeginSize,
		uint32_t						outputUncompPartialBuffSize,
		uint16_t						outputUncompSkipEndSize,
		uint32_t*						pWorkToDoCounter,
		CellSpursEventFlag*				pEventFlag,
		uint16_t						eventFlagBits,
		EdgeZlibInflateTaskProcessing	processing )
{
	EdgeZlibInflateQueueElement queueEntry;

	uint32_t expectedUncompressedSize = outputUncompSkipBeginSize + outputUncompPartialBuffSize + outputUncompSkipEndSize;

	EDGE_ASSERT_MSG( !sys_process_is_stack((void*)pInputCompressedData),
		("EDGE ZLIB ERROR: Input compressed data is on stack (which is illegal)\n") );
	
	EDGE_ASSERT_MSG( !sys_process_is_stack(pOutputUncompPartialBuff),
		("EDGE ZLIB ERROR: Output uncompressed data is on stack (which is illegal)\n") );

	EDGE_ASSERT_MSG( outputUncompSkipBeginSize + outputUncompSkipEndSize <= expectedUncompressedSize,
		("EDGE ZLIB ERROR: outputUncompSkipBeginSize(%d) + outputUncompSkipEndSize(%d) > expectedUncompressedSize(%d)\n",
			outputUncompSkipBeginSize, outputUncompSkipEndSize, expectedUncompressedSize) );

	EDGE_ASSERT( (pWorkToDoCounter == NULL) || ((*pWorkToDoCounter) > 0) );	//If a counter is passed, then being zero already is clearly a bug

	if ( compressedSize > expectedUncompressedSize )
	{
		//If applying compression makes something bigger, then it'd be better to have just used
		//the uncompressed master data in the first place.
		//Just print a warning to draw people's attention to this and carry on.
		EDGE_PRINTF( "Warning: Compressed data at address 0x%08X is *bigger* than master data (%d > %d).  Pointless?\n",
				(uint32_t)pInputCompressedData,
				compressedSize,
				expectedUncompressedSize );
	}

	EDGE_ASSERT( (((uint32_t)pWorkToDoCounter) & 0x3) == 0 );

	queueEntry.m_eaCompressed					= (uint32_t)pInputCompressedData;
	queueEntry.m_eaUncompressed					= (uint32_t)pOutputUncompPartialBuff;
	queueEntry.m_compressedSize					= compressedSize;
	queueEntry.m_outputUncompSkipBeginSize		= outputUncompSkipBeginSize;
	queueEntry.m_outputUncompPartialBuffSize	= outputUncompPartialBuffSize;
	queueEntry.m_outputUncompSkipEndSize		= outputUncompSkipEndSize;
	queueEntry.m_eaWorkToDoCounter				= ((uint32_t)pWorkToDoCounter) | processing;
	queueEntry.m_eaEventFlag					= (uint32_t)pEventFlag;
	queueEntry.m_eventFlagBits					= eventFlagBits;
	queueEntry.m_pad16							= 0;

	__lwsync();									//Ensure store to queueEntry is complete before task can run

	CellSpursLFQueue* pLfQueue = (CellSpursLFQueue*) handle;

	int ret = cellSpursLFQueuePush( pLfQueue, &queueEntry );
	(void) ret;
	EDGE_ASSERT( CELL_OK == ret );
}

//======================================================================================================

/** 
 * @brief	Try to add a new piece of work to the Inflate Queue.  The work added to this queue will be
 *			taken by one of the Inflate Tasks at the next chance they get.
 *
 * @note	If the item is added, then this function will return true.  If the queue is full
 *			(ie. reached maxNumQueueEntries), and another item is pushed onto the queue, then this
 *			function will return false.
 *
 * @note	The work-to-do-counter can be used to track completion of the processing on a collection
 *			of segments by initializing it to however many segments are being waited on.  Then when
 *			each item completes it will decrement by one, and so when the value reaches zero all
 *			segments will have been done.  At this point the specified event flag will be set.
 *			Note that if the SPU has an error with compressed data it will set the high bit of the
 *			counter.
 *
 * @note	If pWorkToDoCounter is NULL, but pEventFlag is valid, then the event flag will be set
 *			after this segment is done.
 *
 * @note	If the data was stored uncompressed and merely needs moving to its destination, then the
 *			Inflate Task can be used for this by passing the appropriate value for 'processing'.
 *
 * @note	This function expects a pointer to the raw compressed data without any header.
 *
 * @note	This function is very similar to edgeZlibAddInflateQueueElement. The additional parameters
 *			skipOutputBeginSize and skipOutputEndSize exist so that decompression of a segment can
 *			be done, but only a portion of the output may need to be written.
 *
 * @note	The sum of outputUncompSkipBeginSize, outputUncompPartialBuffSize and outputUncompSkipEndSize
 *			gives the expected uncompressed side of the compressed data.  The SPU will assert if this
 *			value is incorrect.
 *
 * @param	handle						The handle of the Inflate Queue that the entry will be pushed
 *										onto.
 * @param	pInputCompressedData		The Effective Address of the input data which is to be
 *										decompressed.
 *										This should be a pointer to the raw data without any header.
 * @param	compressedSize				The size of the compressed input data.
 * @param	pOutputUncompPartialBuff	The Effective Address of the output buffer for the
 *										uncompressed data.
 * @param	outputUncompSkipBeginSize	Don't output the first N bytes of the uncompressed output
 * @param	outputUncompPartialBuffSize	The size of the uncompressed data which will be DMAed out.
 * @param	outputUncompSkipEndSize		Don't output the last  N bytes of the uncompressed output
 * @param	pWorkToDoCounter			A counter in main memory which will be atomically decremented
 *										after this item has been decompressed.
 *										If the SPU has an error with the compressed data it will set the
 *										high bit of the counter to alert the fact that an error occurred.
 *										Can be NULL if you're not interested.
 * @param	pEventFlag					The event flag to set.
 *										Can be NULL if you're not interested.
 * @param	eventFlagBits				The value to set to the event flag.
 * @param	processing					Choose what kind of processing the task has to do on the data.
 *										Can perform decompression, or merely move the raw data from
 *										pInputCompressedData to pOutputUncompressedData.
 * @return								'true' if the item was added.  'false' if it failed to add.
 */

bool edgeZlibTryAddInflateQueueElementPartialCopyOut(
		EdgeZlibInflateQHandle			handle,
		const void*						pInputCompressedData,
		uint32_t						compressedSize,
		void*							pOutputUncompPartialBuff,
		uint16_t						outputUncompSkipBeginSize,
		uint32_t						outputUncompPartialBuffSize,
		uint16_t						outputUncompSkipEndSize,
		uint32_t*						pWorkToDoCounter,
		CellSpursEventFlag*				pEventFlag,
		uint16_t						eventFlagBits,
		EdgeZlibInflateTaskProcessing	processing )
{
	EdgeZlibInflateQueueElement queueEntry;

	uint32_t expectedUncompressedSize = outputUncompSkipBeginSize + outputUncompPartialBuffSize + outputUncompSkipEndSize;

	EDGE_ASSERT_MSG( !sys_process_is_stack((void*)pInputCompressedData),
		("EDGE ZLIB ERROR: Input compressed data is on stack (which is illegal)\n") );
	
	EDGE_ASSERT_MSG( !sys_process_is_stack(pOutputUncompPartialBuff),
		("EDGE ZLIB ERROR: Output uncompressed data is on stack (which is illegal)\n") );


	EDGE_ASSERT_MSG( outputUncompSkipBeginSize + outputUncompSkipEndSize <= expectedUncompressedSize,
		("EDGE ZLIB ERROR: outputUncompSkipBeginSize(%d) + outputUncompSkipEndSize(%d) > expectedUncompressedSize(%d)\n",
				outputUncompSkipBeginSize, outputUncompSkipEndSize, expectedUncompressedSize) );

	EDGE_ASSERT( (pWorkToDoCounter == NULL) || ((*pWorkToDoCounter) > 0) );	//If a counter is passed, then being zero already is clearly a bug

	if ( compressedSize > expectedUncompressedSize )
	{
		//If applying compression makes something bigger, then it'd be better to have just used
		//the uncompressed master data in the first place.
		//Just print a warning to draw people's attention to this and carry on.
		EDGE_PRINTF( "Warning: Compressed data at address 0x%08X is *bigger* than master data (%d > %d).  Pointless?\n",
				(uint32_t)pInputCompressedData,
				compressedSize,
				expectedUncompressedSize );
	}

	EDGE_ASSERT( (((uint32_t)pWorkToDoCounter) & 0x3) == 0 );

	queueEntry.m_eaCompressed					= (uint32_t)pInputCompressedData;
	queueEntry.m_eaUncompressed					= (uint32_t)pOutputUncompPartialBuff;
	queueEntry.m_compressedSize					= compressedSize;
	queueEntry.m_outputUncompSkipBeginSize		= outputUncompSkipBeginSize;
	queueEntry.m_outputUncompPartialBuffSize	= outputUncompPartialBuffSize;
	queueEntry.m_outputUncompSkipEndSize		= outputUncompSkipEndSize;
	queueEntry.m_eaWorkToDoCounter				= ((uint32_t)pWorkToDoCounter) | processing;
	queueEntry.m_eaEventFlag					= (uint32_t)pEventFlag;
	queueEntry.m_eventFlagBits					= eventFlagBits;
	queueEntry.m_pad16							= 0;

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
 * @brief	Shuts down the Inflate Queue.
 *
 * @param	handle						The handle of the Inflate Queue to shutdown.
 */

void edgeZlibShutdownInflateQueue( EdgeZlibInflateQHandle handle )
{
	CellSpursLFQueue* pLfQueue = (CellSpursLFQueue*) handle;

	int ret = cellSpursLFQueueDetachLv2EventQueue( pLfQueue );
	EDGE_ASSERT( CELL_OK == ret );
	(void) ret;
}

//======================================================================================================
