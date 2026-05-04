/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <cell/spurs/lfqueue.h>
#include <stdlib.h>
#include "edge/edge_printf.h"
#include "edge/edge_assert.h"
#include "edge/lzo/edgelzo1x_inflate_queue_element.h"
#include "edge/lzo/edgelzo1x_deflate_queue_element.h"
#include "edge/lzo/edgelzo_spu.h"


//======================================================================================================

/** 
 * @brief	Add a new piece of work to the Inflate Queue. The work added to this queue will be taken by
 *			one of the Inflate Tasks at the next chance they get.
 *
 * @note	This is the SPU equivalent of the PPU function of the same name.
 *
 * @note	If the queue is full (ie. reached maxNumQueueEntries), and another item is pushed onto the
 *			queue, then this function will block until there is space.
 *
 * @note	The work-to-do-counter can be used to track completion of the processing on a collection
 *			of segments by initializing it to however many segments are being waited on. Then when
 *			each item completes it will decrement by one, and so when the value reaches zero all
 *			segments will have been done.  At this point the specified event flag will be set.
 *			Note that if the SPU has an error with compressed data it will set the high bit of the
 *			counter.
 *
 * @note	If eaWorkToDoCounter is NULL, but eaEventFlag is valid, then the event flag will be set
 *			after this segment is done.
 *
 * @note	If the data was stored uncompressed and merely needs moving to its destination, then the
 *			Inflate Task can be used for this by passing the appropriate value for 'processing'.
 *
 * @param	handle						The handle of the Inflate Queue that the entry will be pushed
 *										onto.
 * @param	eaInputCompressedData		The Effective Address of the input data which is to be
 *										decompressed.
 * @param	compressedSize				The size of the compressed input data.
 * @param	eaOutputUncompressedData	The Effective Address of the output buffer for the
 *										uncompressed data.
 * @param	expectedUncompressedSize	The expected size of the uncompressed data.
 *										The SPU will assert if this value is incorrect.
 * @param	eaWorkToDoCounter			A counter in main memory which will be atomically decremented
 *										after this item has been decompressed.
 *										If the SPU has an error with the compressed data it will set the
 *										high bit of the counter to alert the fact that an error occurred.
 *										Can be NULL if you're not interested.
 * @param	eaEventFlag					The event flag to set.
 *										Can be NULL if you're not interested.
 * @param	eventFlagBits				The value to set to the event flag.
 * @param	processing					Choose what kind of processing the task has to do on the data.
 *										Can perform decompression, or merely move the data from
 *										pInputCompressedData to pOutputUncompressedData.
 * @param	dmaTag						The DMA tag to use when adding the element to the queue
 */

void edgeLzo1xAddInflateQueueElement(
		register EdgeLzo1xInflateQHandle		handle,
		register uint32_t						eaInputCompressedData,
		register uint32_t						compressedSize,
		register uint32_t						eaOutputUncompressedData,
		register uint32_t						expectedUncompressedSize,
		register uint32_t						eaWorkToDoCounter,
		register uint32_t						eaEventFlag,
		register uint16_t						eventFlagBits,
		register EdgeLzo1xInflateTaskProcessing	processing,
		register uint32_t						dmaTag )
{
	edgeLzo1xAddInflateQueueElementPartialCopyOut(	handle,
													eaInputCompressedData,
													compressedSize,
													eaOutputUncompressedData,
													0,
													expectedUncompressedSize,
													0,
													eaWorkToDoCounter,
													eaEventFlag,
													eventFlagBits,
													processing,
													dmaTag );
}

//======================================================================================================

/** 
 * @brief	Try to add a new piece of work to the Inflate Queue. The work added to this queue will be
 *			taken by one of the Inflate Tasks at the next chance they get.
 *
 * @note	This is the SPU equivalent of the PPU function of the same name.
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
 * @note	If eaWorkToDoCounter is NULL, but eaEventFlag is valid, then the event flag will be set
 *			after this segment is done.
 *
 * @note	If the data was stored uncompressed and merely needs moving to its destination, then the
 *			Inflate Task can be used for this by passing the appropriate value for 'processing'.
 *
 * @param	handle						The handle of the Inflate Queue that the entry will be pushed
 *										onto.
 * @param	eaInputCompressedData		The Effective Address of the input data which is to be
 *										decompressed.
 * @param	compressedSize				The size of the compressed input data.
 * @param	eaOutputUncompressedData	The Effective Address of the output buffer for the
 *										uncompressed data.
 * @param	expectedUncompressedSize	The expected size of the uncompressed data.
 *										The SPU will assert if this value is incorrect.
 * @param	eaWorkToDoCounter			A counter in main memory which will be atomically decremented
 *										after this item has been decompressed.
 *										If the SPU has an error with the compressed data it will set the
 *										high bit of the counter to alert the fact that an error occurred.
 *										Can be NULL if you're not interested.
 * @param	eaEventFlag					The event flag to set.
 *										Can be NULL if you're not interested.
 * @param	eventFlagBits				The value to set to the event flag.
 * @param	processing					Choose what kind of processing the task has to do on the data.
 *										Can perform decompression, or merely move the data from
 *										pInputCompressedData to pOutputUncompressedData.
 * @param	dmaTag						The DMA tag to use when adding the element to the queue
 * @return								'true' if the item was added.  'false' if it failed to add.
 */

bool edgeLzo1xTryAddInflateQueueElement(
		register EdgeLzo1xInflateQHandle		handle,
		register uint32_t						eaInputCompressedData,
		register uint32_t						compressedSize,
		register uint32_t						eaOutputUncompressedData,
		register uint32_t						expectedUncompressedSize,
		register uint32_t						eaWorkToDoCounter,
		register uint32_t						eaEventFlag,
		register uint16_t						eventFlagBits,
		register EdgeLzo1xInflateTaskProcessing	processing,
		register uint32_t						dmaTag )
{
	return edgeLzo1xTryAddInflateQueueElementPartialCopyOut(handle,
															eaInputCompressedData,
															compressedSize,
															eaOutputUncompressedData,
															0,
															expectedUncompressedSize,
															0,
															eaWorkToDoCounter,
															eaEventFlag,
															eventFlagBits,
															processing,
															dmaTag );
}

//======================================================================================================

/** 
 * @brief	Add a new piece of work to the Inflate Queue. The work added to this queue will be taken by
 *			one of the Inflate Tasks at the next chance they get.
 *
 * @note	This is the SPU equivalent of the PPU function of the same name.
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
 * @note	If eaWorkToDoCounter is NULL, but eaEventFlag is valid, then the event flag will be set
 *			after this segment is done.
 *
 * @note	If the data was stored uncompressed and merely needs moving to its destination, then the
 *			Inflate Task can be used for this by passing the appropriate value for 'processing'.
 *
 * @note	This function is very similar to edgeLzo1xAddInflateQueueElement. The additional parameters
 *			skipOutputBeginSize and skipOutputEndSize exist so that decompression of a segment can
 *			be done, but only a portion of the output may need to be written.
 *
 * @note	The sum of outputUncompSkipBeginSize, outputUncompPartialBuffSize and outputUncompSkipEndSize
 *			gives the expected uncompressed side of the compressed data.  The SPU will assert if this
 *			value is incorrect.
 *
 * @param	handle						The handle of the Inflate Queue that the entry will be pushed
 *										onto.
 * @param	eaInputCompressedData		The Effective Address of the input data which is to be
 *										decompressed.
 * @param	compressedSize				The size of the compressed input data.
 * @param	eaOutputUncompPartialBuff	The Effective Address of the output buffer for the
 *										uncompressed data.
 * @param	outputUncompSkipBeginSize	Don't output the first N bytes of the uncompressed output
 * @param	outputUncompPartialBuffSize	The size of the uncompressed data which will be DMAed out.
 * @param	outputUncompSkipEndSize		Don't output the last  N bytes of the uncompressed output
 * @param	eaWorkToDoCounter			A counter in main memory which will be atomically decremented
 *										after this item has been decompressed.
 *										If the SPU has an error with the compressed data it will set the
 *										high bit of the counter to alert the fact that an error occurred.
 *										Can be NULL if you're not interested.
 * @param	eaEventFlag					The event flag to set.
 *										Can be NULL if you're not interested.
 * @param	eventFlagBits				The value to set to the event flag.
 * @param	processing					Choose what kind of processing the task has to do on the data.
 *										Can perform decompression, or merely move the data from
 *										pInputCompressedData to pOutputUncompressedData.
 * @param	dmaTag						The DMA tag to use when adding the element to the queue
 */

void edgeLzo1xAddInflateQueueElementPartialCopyOut(
		register EdgeLzo1xInflateQHandle		handle,
		register uint32_t						eaInputCompressedData,
		register uint32_t						compressedSize,
		register uint32_t						eaOutputUncompPartialBuff,
		register uint16_t						outputUncompSkipBeginSize,
		register uint32_t						outputUncompPartialBuffSize,
		register uint16_t						outputUncompSkipEndSize,
		register uint32_t						eaWorkToDoCounter,
		register uint32_t						eaEventFlag,
		register uint16_t						eventFlagBits,
		register EdgeLzo1xInflateTaskProcessing	processing,
		register uint32_t						dmaTag )
{
	EdgeLzo1xInflateQueueElement queueEntry;

	uint32_t expectedUncompressedSize = outputUncompSkipBeginSize + outputUncompPartialBuffSize + outputUncompSkipEndSize;

	EDGE_ASSERT_MSG( compressedSize <= kEdgeLzo1xInflateInputBufferMaxSize,
		( "EDGE LZO1X ERROR: Size of compressed data is %d.  Maximum is %d.\n",
			compressedSize, kEdgeLzo1xInflateInputBufferMaxSize ) );

	EDGE_ASSERT_MSG( expectedUncompressedSize <= kEdgeLzo1xInflateOutputBufferMaxSize,
		( "EDGE LZO1X ERROR: Size of uncompressed data is %d.  Maximum is %d.\n",
			expectedUncompressedSize, kEdgeLzo1xInflateOutputBufferMaxSize ) );

	EDGE_ASSERT_MSG( outputUncompSkipBeginSize + outputUncompSkipEndSize <= expectedUncompressedSize,
		( "EDGE LZO1X ERROR: outputUncompSkipBeginSize(%d) + outputUncompSkipEndSize(%d) > expectedUncompressedSize(%d)\n",
			outputUncompSkipBeginSize, outputUncompSkipEndSize, expectedUncompressedSize ) );

	//EDGE_ASSERT( (eaWorkToDoCounter == NULL) || ((*eaWorkToDoCounter) > 0) );	//Requires a DMA to implement this assert

	if ( compressedSize > expectedUncompressedSize )
	{
		//If applying compression makes something bigger, then it'd be better to have just used
		//the uncompressed master data in the first place.
		//Just print a warning to draw people's attention to this and carry on.
		EDGE_PRINTF( "Warning: Compressed data at address 0x%08X is *bigger* than master data (%d > %d).  Pointless?\n",
				eaInputCompressedData,
				compressedSize,
				expectedUncompressedSize );
	}

	EDGE_ASSERT( (eaWorkToDoCounter & 0x3) == 0 );

	queueEntry.m_eaCompressed					= eaInputCompressedData;
	queueEntry.m_eaUncompressed					= eaOutputUncompPartialBuff;
	queueEntry.m_compressedSize					= compressedSize;
	queueEntry.m_outputUncompSkipBeginSize		= outputUncompSkipBeginSize;
	queueEntry.m_outputUncompPartialBuffSize	= outputUncompPartialBuffSize;
	queueEntry.m_outputUncompSkipEndSize		= outputUncompSkipEndSize;
	queueEntry.m_eaEventFlag					= eaEventFlag;
	queueEntry.m_eaWorkToDoCounter				= (eaWorkToDoCounter) | processing;
	queueEntry.m_eventFlagBits					= eventFlagBits;
	queueEntry.m_pad16							= 0;

	//If interrupts are enabled, disable them temporarily while adding to the queue
	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	CellSpursLFQueuePushContainer pushContainer;
	cellSpursLFQueuePushContainerInitialize( &pushContainer, &queueEntry, dmaTag );

	//Note: if the queue is full this routine will wait (yield?)
	int ret = cellSpursLFQueuePushBegin( handle, &pushContainer );
	EDGE_ASSERT( CELL_OK == ret );
	ret = cellSpursLFQueuePushEnd( handle, &pushContainer );
	EDGE_ASSERT( CELL_OK == ret );

	if ( interruptsEnabled )
	{
		spu_ienable();
	}
}

//======================================================================================================

/** 
 * @brief	Try to add a new piece of work to the Inflate Queue. The work added to this queue will be
 *			taken by one of the Inflate Tasks at the next chance they get.
 *
 * @note	This is the SPU equivalent of the PPU function of the same name.
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
 * @note	If eaWorkToDoCounter is NULL, but eaEventFlag is valid, then the event flag will be set
 *			after this segment is done.
 *
 * @note	If the data was stored uncompressed and merely needs moving to its destination, then the
 *			Inflate Task can be used for this by passing the appropriate value for 'processing'.
 *
 * @note	This function is very similar to edgeLzo1xAddInflateQueueElement. The additional parameters
 *			skipOutputBeginSize and skipOutputEndSize exist so that decompression of a segment can
 *			be done, but only a portion of the output may need to be written.
 *
 * @note	The sum of outputUncompSkipBeginSize, outputUncompPartialBuffSize and outputUncompSkipEndSize
 *			gives the expected uncompressed side of the compressed data. The SPU will assert if this
 *			value is incorrect.
 *
 * @param	handle						The handle of the Inflate Queue that the entry will be pushed
 *										onto.
 * @param	eaInputCompressedData		The Effective Address of the input data which is to be
 *										decompressed.
 * @param	compressedSize				The size of the compressed input data.
 * @param	eaOutputUncompPartialBuff	The Effective Address of the output buffer for the
 *										uncompressed data.
 * @param	outputUncompSkipBeginSize	Don't output the first N bytes of the uncompressed output
 * @param	outputUncompPartialBuffSize	The size of the uncompressed data which will be DMAed out.
 * @param	outputUncompSkipEndSize		Don't output the last  N bytes of the uncompressed output
 * @param	eaWorkToDoCounter			A counter in main memory which will be atomically decremented
 *										after this item has been decompressed.
 *										If the SPU has an error with the compressed data it will set the
 *										high bit of the counter to alert the fact that an error occurred.
 *										Can be NULL if you're not interested.
 * @param	eaEventFlag					The event flag to set.
 *										Can be NULL if you're not interested.
 * @param	eventFlagBits				The value to set to the event flag.
 * @param	processing					Choose what kind of processing the task has to do on the data.
 *										Can perform decompression, or merely move the data from
 *										pInputCompressedData to pOutputUncompressedData.
 * @param	dmaTag						The DMA tag to use when adding the element to the queue
 * @return								'true' if the item was added. 'false' if it failed to add.
 */

bool edgeLzo1xTryAddInflateQueueElementPartialCopyOut(
		register EdgeLzo1xInflateQHandle		handle,
		register uint32_t						eaInputCompressedData,
		register uint32_t						compressedSize,
		register uint32_t						eaOutputUncompPartialBuff,
		register uint16_t						outputUncompSkipBeginSize,
		register uint32_t						outputUncompPartialBuffSize,
		register uint16_t						outputUncompSkipEndSize,
		register uint32_t						eaWorkToDoCounter,
		register uint32_t						eaEventFlag,
		register uint16_t						eventFlagBits,
		register EdgeLzo1xInflateTaskProcessing	processing,
		register uint32_t						dmaTag )
{
	EdgeLzo1xInflateQueueElement queueEntry;

	uint32_t expectedUncompressedSize = outputUncompSkipBeginSize + outputUncompPartialBuffSize + outputUncompSkipEndSize;

	EDGE_ASSERT_MSG( compressedSize <= kEdgeLzo1xInflateInputBufferMaxSize,
		( "EDGE LZO1X ERROR: Size of compressed data is %d.  Maximum is %d.\n",
			compressedSize, kEdgeLzo1xInflateInputBufferMaxSize ) );

	EDGE_ASSERT_MSG( expectedUncompressedSize <= kEdgeLzo1xInflateOutputBufferMaxSize,
		( "EDGE LZO1X ERROR: Size of uncompressed data is %d.  Maximum is %d.\n",
			expectedUncompressedSize, kEdgeLzo1xInflateOutputBufferMaxSize ) );

	EDGE_ASSERT_MSG( outputUncompSkipBeginSize + outputUncompSkipEndSize <= expectedUncompressedSize,
		( "EDGE LZO1X ERROR: outputUncompSkipBeginSize(%d) + outputUncompSkipEndSize(%d) > expectedUncompressedSize(%d)\n",
			outputUncompSkipBeginSize, outputUncompSkipEndSize, expectedUncompressedSize ) );

	//EDGE_ASSERT( (eaWorkToDoCounter == NULL) || ((*eaWorkToDoCounter) > 0) );	//Requires a DMA to implement this assert

	if ( compressedSize > expectedUncompressedSize )
	{
		//If applying compression makes something bigger, then it'd be better to have just used
		//the uncompressed master data in the first place.
		//Just print a warning to draw people's attention to this and carry on.
		EDGE_PRINTF( "Warning: Compressed data at address 0x%08X is *bigger* than master data (%d > %d).  Pointless?\n",
				eaInputCompressedData,
				compressedSize,
				expectedUncompressedSize );
	}

	EDGE_ASSERT( (eaWorkToDoCounter & 0x3) == 0 );

	queueEntry.m_eaCompressed					= eaInputCompressedData;
	queueEntry.m_eaUncompressed					= eaOutputUncompPartialBuff;
	queueEntry.m_compressedSize					= compressedSize;
	queueEntry.m_outputUncompSkipBeginSize		= outputUncompSkipBeginSize;
	queueEntry.m_outputUncompPartialBuffSize	= outputUncompPartialBuffSize;
	queueEntry.m_outputUncompSkipEndSize		= outputUncompSkipEndSize;
	queueEntry.m_eaEventFlag					= eaEventFlag;
	queueEntry.m_eaWorkToDoCounter				= (eaWorkToDoCounter) | processing;
	queueEntry.m_eventFlagBits					= eventFlagBits;
	queueEntry.m_pad16							= 0;

	//If interrupts are enabled, disable them temporarily while adding to the queue
	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	CellSpursLFQueuePushContainer pushContainer;
	cellSpursLFQueuePushContainerInitialize( &pushContainer, &queueEntry, dmaTag );

	//Test to see if we can push onto the queue
	int ret = cellSpursLFQueueTryPushBegin( handle, &pushContainer );
	if ( CELL_SPURS_TASK_ERROR_AGAIN == ret )
	{
		//No space
		if ( interruptsEnabled )
		{
			spu_ienable();
		}
		return false;
	}
	EDGE_ASSERT( CELL_OK == ret );

	ret = cellSpursLFQueuePushEnd( handle, &pushContainer );
	EDGE_ASSERT( CELL_OK == ret );

	if ( interruptsEnabled )
	{
		spu_ienable();
	}

	return true;
}

//======================================================================================================

/** 
 * @brief	Add a new piece of work to the Deflate Queue. The work added to this queue will be taken by
 *			one of the Deflate Tasks at the next chance they get.
 *
 * @note	This is the SPU equivalent of the PPU function of the same name.
 *
 * @note	If the queue is full (ie. reached maxNumQueueEntries), and another item is pushed onto the
 *			queue, then this function will block until there is space.
 *
 * @note	The work-to-do-counter can be used to track completion of the processing on a collection
 *			of segments by initializing it to however many segments are being waited on. Then when
 *			each item completes it will decrement by one, and so when the value reaches zero all
 *			segments will have been done. At this point the specified event flag will be set.
 *			Note that if the SPU has an error with compression it will set the high bit of the
 *			counter.
 *
 * @note	If eaWorkToDoCounter is NULL, but eaEventFlag is valid, then the event flag will be set
 *			after this segment is done.
 *
 * @param	handle						The handle of the Deflate Queue that the entry will be pushed
 *										onto.
 * @param	eaInputUncompressedData		The Effective Address of the input data which is to be
 *										compressed.
 * @param	uncompressedSize			The size of the uncompressed input data.
 * @param	eaOutputCompressedData		The Effective Address of the output buffer for the
 *										compressed data.
 * @param	maxCompressedOutputSize		The maximum space available for the compressed data.
 * @param	eaOutputCompressedSize		The Effective Address of the uint32_t into which the output
 *										compressed size will be written. The top bit of the output
 *										size indicates whether the data was stored compressed or if the
 *										uncompressed original data was chosen for storing.
 * @param	eaWorkToDoCounter			A counter in main memory which will be atomically decremented
 *										after this item has been compressed.
 *										If the SPU has an error with the compression it will set the
 *										high bit of the counter to alert the fact that an error occurred.
 *										Can be NULL if you're not interested.
 * @param	eaEventFlag					The event flag to set.
 *										Can be NULL if you're not interested.
 * @param	eventFlagBits				The value to set to the event flag.
 * @param	processing					If compression makes the data bigger, choose whether to store
 *										the compressed or the original data.
 * @param	dmaTag						The DMA tag to use when adding the element to the queue
 */

void edgeLzo1xAddDeflateQueueElement(
		EdgeLzo1xDeflateQHandle			handle,
		uint32_t						eaInputUncompressedData,
		uint32_t						uncompressedSize,
		uint32_t						eaOutputCompressedData,
		uint32_t						maxCompressedOutputSize,
		uint32_t						eaOutputCompressedSize,
		uint32_t						eaWorkToDoCounter,
		uint32_t						eaEventFlag,
		uint16_t						eventFlagBits,
		EdgeLzo1xDeflateTaskProcessing	processing,
		uint32_t						dmaTag )
{
	EdgeLzo1xDeflateQueueElement queueEntry;

	EDGE_ASSERT_MSG ( uncompressedSize <= kEdgeLzo1xDeflateInputBufferMaxSize,
		( "EDGE LZO1X ERROR: Size of uncompressed data is %d.  Maximum is %d.\n", uncompressedSize, kEdgeLzo1xDeflateInputBufferMaxSize ) );

	EDGE_ASSERT_MSG ( maxCompressedOutputSize <= kEdgeLzo1xDeflateOutputBufferMaxSize,
		( "EDGE LZO1X ERROR: Size of compressed data is %d.  Maximum is %d.\n", maxCompressedOutputSize, kEdgeLzo1xDeflateOutputBufferMaxSize ) );

	//EDGE_ASSERT( (eaWorkToDoCounter == NULL) || ((*eaWorkToDoCounter) > 0) );	//Requires a DMA to implement this assert

	EDGE_ASSERT( (eaWorkToDoCounter & 0x3) == 0 );

	queueEntry.m_eaInputUncompressedData		= eaInputUncompressedData;
	queueEntry.m_eaOutputCompressedData			= eaOutputCompressedData;
	queueEntry.m_uncompressedSize				= uncompressedSize;
	queueEntry.m_maxCompressedOutputSize		= maxCompressedOutputSize;
	queueEntry.m_eaOutputCompressedSize			= eaOutputCompressedSize;
	queueEntry.m_eaWorkToDoCounter				= (eaWorkToDoCounter) | processing;
	queueEntry.m_eaEventFlag					= eaEventFlag;
	queueEntry.m_eventFlagBits					= eventFlagBits;
	queueEntry.m_pad16							= 0;

	//If interrupts are enabled, disable them temporarily while adding to the queue
	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	CellSpursLFQueuePushContainer pushContainer;
	cellSpursLFQueuePushContainerInitialize( &pushContainer, &queueEntry, dmaTag );

	//Note: if the queue is full this routine will wait (yield?)
	int ret = cellSpursLFQueuePushBegin( handle, &pushContainer );
	EDGE_ASSERT( CELL_OK == ret );
	ret = cellSpursLFQueuePushEnd( handle, &pushContainer );
	EDGE_ASSERT( CELL_OK == ret );

	if ( interruptsEnabled )
	{
		spu_ienable();
	}
}

//======================================================================================================

/** 
 * @brief	Try to add a new piece of work to the Deflate Queue. The work added to this queue will be taken by
 *			one of the Deflate Tasks at the next chance they get.
 *
 * @note	This is the SPU equivalent of the PPU function of the same name.
 *
 * @note	If the queue is full (ie. reached maxNumQueueEntries), and another item is pushed onto the
 *			queue, then this function will return and not block.
 *
 * @note	The work-to-do-counter can be used to track completion of the processing on a collection
 *			of segments by initializing it to however many segments are being waited on. Then when
 *			each item completes it will decrement by one, and so when the value reaches zero all
 *			segments will have been done. At this point the specified event flag will be set.
 *			Note that if the SPU has an error with compression it will set the high bit of the
 *			counter.
 *
 * @note	If eaWorkToDoCounter is NULL, but eaEventFlag is valid, then the event flag will be set
 *			after this segment is done.
 *
 * @param	handle						The handle of the Deflate Queue that the entry will be pushed
 *										onto.
 * @param	eaInputUncompressedData		The Effective Address of the input data which is to be
 *										compressed.
 * @param	uncompressedSize			The size of the uncompressed input data.
 * @param	eaOutputCompressedData		The Effective Address of the output buffer for the
 *										compressed data.
 * @param	maxCompressedOutputSize		The maximum space available for the compressed data.
 * @param	eaOutputCompressedSize		The Effective Address of the uint32_t into which the output
 *										compressed size will be written. The top bit of the output
 *										size indicates whether the data was stored compressed or if the
 *										uncompressed original data was chosen for storing.
 * @param	eaWorkToDoCounter			A counter in main memory which will be atomically decremented
 *										after this item has been compressed.
 *										If the SPU has an error with the compression it will set the
 *										high bit of the counter to alert the fact that an error occurred.
 *										Can be NULL if you're not interested.
 * @param	eaEventFlag					The event flag to set.
 *										Can be NULL if you're not interested.
 * @param	eventFlagBits				The value to set to the event flag.
 * @param	processing					If compression makes the data bigger, choose whether to store
 *										the compressed or the original data.
 * @param	dmaTag						The DMA tag to use when adding the element to the queue
 * @return								'true' if the item was added. 'false' if it failed to add.
 */

bool edgeLzo1xTryAddDeflateQueueElement(
		EdgeLzo1xDeflateQHandle			handle,
		uint32_t						eaInputUncompressedData,
		uint32_t						uncompressedSize,
		uint32_t						eaOutputCompressedData,
		uint32_t						maxCompressedOutputSize,
		uint32_t						eaOutputCompressedSize,
		uint32_t						eaWorkToDoCounter,
		uint32_t						eaEventFlag,
		uint16_t						eventFlagBits,
		EdgeLzo1xDeflateTaskProcessing	processing,
		uint32_t						dmaTag )
{
	EdgeLzo1xDeflateQueueElement queueEntry;

	EDGE_ASSERT_MSG ( uncompressedSize <= kEdgeLzo1xDeflateInputBufferMaxSize,
		( "EDGE LZO1X ERROR: Size of uncompressed data is %d.  Maximum is %d.\n", uncompressedSize, kEdgeLzo1xDeflateInputBufferMaxSize ) );

	EDGE_ASSERT_MSG ( maxCompressedOutputSize <= kEdgeLzo1xDeflateOutputBufferMaxSize,
		( "EDGE LZO1X ERROR: Size of compressed data is %d.  Maximum is %d.\n", maxCompressedOutputSize, kEdgeLzo1xDeflateOutputBufferMaxSize ) );

	//EDGE_ASSERT( (eaWorkToDoCounter == NULL) || ((*eaWorkToDoCounter) > 0) );	//Requires a DMA to implement this assert
	EDGE_ASSERT( (eaWorkToDoCounter & 0x3) == 0 );

	queueEntry.m_eaInputUncompressedData		= eaInputUncompressedData;
	queueEntry.m_eaOutputCompressedData			= eaOutputCompressedData;
	queueEntry.m_uncompressedSize				= uncompressedSize;
	queueEntry.m_maxCompressedOutputSize		= maxCompressedOutputSize;
	queueEntry.m_eaOutputCompressedSize			= eaOutputCompressedSize;
	queueEntry.m_eaWorkToDoCounter				= (eaWorkToDoCounter) | processing;
	queueEntry.m_eaEventFlag					= eaEventFlag;
	queueEntry.m_eventFlagBits					= eventFlagBits;
	queueEntry.m_pad16							= 0;

	//If interrupts are enabled, disable them temporarily while adding to the queue
	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	CellSpursLFQueuePushContainer pushContainer;
	cellSpursLFQueuePushContainerInitialize( &pushContainer, &queueEntry, dmaTag );

	//Test to see if we can push onto the queue
	int ret = cellSpursLFQueueTryPushBegin( handle, &pushContainer );
	if ( CELL_SPURS_TASK_ERROR_AGAIN == ret )
	{
		//No space
		if ( interruptsEnabled )
		{
			spu_ienable();
		}
		return false;
	}
	EDGE_ASSERT( CELL_OK == ret );

	ret = cellSpursLFQueuePushEnd( handle, &pushContainer );
	EDGE_ASSERT( CELL_OK == ret );

	if ( interruptsEnabled )
	{
		spu_ienable();
	}

	return true;
}

//======================================================================================================
