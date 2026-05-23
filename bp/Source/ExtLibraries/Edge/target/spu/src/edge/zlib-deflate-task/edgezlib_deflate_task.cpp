/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/edge_stdint.h"
#include <spu_intrinsics.h>
#include <string.h>

#include <cell/spurs/types.h>
#include <cell/spurs/task.h>
#include <cell/spurs/lfqueue.h>
#include <cell/spurs/event_flag.h>

#include "edge/edge_atomic.h"
#include "edge/edge_printf.h"
#include "edge/edge_assert.h"

#include "edge/zlib/edgezlib_deflate_queue_element.h"
#include "edge/zlib/edgezlib_spu.h"

//////////////////////////////////////////////////////////////////////////

enum
{
	kEdgeZlibOverAllocateInputBufferSize	= 128,	//Need to reserve more than maximum buffer size due to mis-alignment of incoming data
	kEdgeZlibOverAllocateOutputBufferSize	= 128,	//Need to reserve more than maximum buffer size due to mis-alignment of outgoing data
};

unsigned char gEdgeZlibDeflateInputBuf[kEdgeZlibDeflateInputBufferMaxSize + kEdgeZlibOverAllocateInputBufferSize]		__attribute__((aligned(128)));
unsigned char gEdgeZlibDeflateOutputBuf[kEdgeZlibDeflateOutputBufferMaxSize + kEdgeZlibOverAllocateOutputBufferSize]	__attribute__((aligned(128)));

unsigned char gEdgeZlibAtomicBuffer[128] __attribute__((aligned(128)));	//Only used temporarily for atomic transactions

enum
{
	kDmaTagId = 0,
};

//////////////////////////////////////////////////////////////////////////

static int CompressDeflateQueueElement( const EdgeZlibDeflateQueueElement* queueEntry )
{
	uint32_t	eaInputUncompressedData		= queueEntry->m_eaInputUncompressedData;
	uint32_t	eaOutputCompressedData		= queueEntry->m_eaOutputCompressedData;
	uint32_t	uncompressedSize			= queueEntry->m_uncompressedSize;
	uint32_t	maxCompressedOutputSize		= queueEntry->m_maxCompressedOutputSize;
	uint32_t	eaOutputCompressedSize		= queueEntry->m_eaOutputCompressedSize;
	uint32_t	level						= queueEntry->m_compressionLevel;
	EdgeZlibDeflateTaskProcessing taskProcessing =
					(EdgeZlibDeflateTaskProcessing)(queueEntry->m_eaWorkToDoCounter & kEdgeZlibDeflateTask_ProcessingMask);

	bool storeSmallest = ((taskProcessing & kEdgeZlibDeflateTask_DeflateStoreWhatMask) == kEdgeZlibDeflateTask_DeflateStoreSmallest);
	if ( storeSmallest && (uncompressedSize > kEdgeZlibDeflateInputBufferMaxSize) )
	{
		EDGE_ZLIB_DATA_ASSERT_MSG( false, ( "EDGE ZLIB ERROR: Since uncompressed size (%d) is > SPU buffer size (%d), you can NOT tell SPU to storeSmallest of uncompressed & compressed data\n", uncompressedSize, kEdgeZlibDeflateInputBufferMaxSize) );
		return 1;
	}

	int err = edgeZlibFetchAndDeflateRawData(
			eaOutputCompressedData,				//	uint32_t		eaOutputCompressedData,
			maxCompressedOutputSize,			//	uint32_t		maxCompressedOutputSize,
			eaOutputCompressedSize,				//	uint32_t		eaOutputCompressedSize,
			eaInputUncompressedData,			//	uint32_t		eaInputUncompressedData,
			uncompressedSize,					//	uint32_t		uncompressedSize,
			kDmaTagId,							//	uint32_t		dmaTag,
			gEdgeZlibDeflateInputBuf,			//	unsigned char*	pLsInputTempBuffer,
			sizeof(gEdgeZlibDeflateInputBuf),	//	uint32_t		inputTempBuffSize,
			gEdgeZlibDeflateOutputBuf,			//	unsigned char*	pLsOutputTempBuffer,
			sizeof(gEdgeZlibDeflateOutputBuf),	//	uint32_t		outputTempBuffSize,
			level,								//	uint32_t		level,
			taskProcessing );					//	EdgeZlibDeflateTaskProcessing taskProcessing

	if ( 0 != err )
	{
		EDGE_ZLIB_DATA_ASSERT_MSG( false, ( "EDGE ZLIB ERROR: edgeZlibFetchAndDeflateRawData failed (%d)\n", err ) );
		return err;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////

static void ProcessDeflateQueueElement( const EdgeZlibDeflateQueueElement* queueEntry )
{
	//////////////////////////////////////////////////////////////////////////
	//
	//	Fetch the input data, compress it, and send it out again
	//
	//////////////////////////////////////////////////////////////////////////

	int err = CompressDeflateQueueElement( queueEntry );
	EDGE_ZLIB_DATA_ASSERT_MSG( 0 == err, ( "EDGE ZLIB ERROR: CompressDeflateQueueElement returned (%d)\n", err ) );


	//////////////////////////////////////////////////////////////////////////
	//
	//	Once the compression is done, decrement the counter and if
	//	necessary set the event flag
	//
	//////////////////////////////////////////////////////////////////////////

	uint32_t eaWorkToDoCounter	= queueEntry->m_eaWorkToDoCounter & ~kEdgeZlibDeflateTask_ProcessingMask;
	uint32_t eaEventFlag		= queueEntry->m_eaEventFlag;
	uint16_t eventFlagBits		= queueEntry->m_eventFlagBits;

	// decrement counter and maybe set event flag
	bool setEventFlag = true;
	if ( eaWorkToDoCounter )
	{
		setEventFlag = false;

		if ( 0 != err )
		{	// Set hi bit of countdown value so PPU can see there was an error
			EDGE_ATOMIC_OR_32( (uint32_t*) gEdgeZlibAtomicBuffer, eaWorkToDoCounter, kEdgeZlibDeflateTask_ErrorDuringCompression );
		}

		uint32_t old = EDGE_ATOMIC_DECR_32( (uint32_t*) gEdgeZlibAtomicBuffer, eaWorkToDoCounter );

		old = old & ~kEdgeZlibDeflateTask_CompressionErrorMask;

		if ( 0 == old )
		{
			EDGE_PRINTF( "EDGE ZLIB ERROR: Attempted to decrement a work-to-do counter already at zero\n" );
			EDGE_ASSERT( false );
			return;
		}

		if ( 1 == old ) //if just decremented to zero
		{
			setEventFlag = true;
		}
	}

	if ( setEventFlag )
	{
		if ( eaEventFlag )
		{
			int ret = cellSpursEventFlagSet( eaEventFlag, eventFlagBits );
			EDGE_ASSERT( CELL_OK == ret );
			(void) ret;
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void cellSpursMain( qword argTask, uint64_t )
{
	// get adrs of queue
	uint32_t eaQueue = spu_extract( (vec_uint4)argTask, 0 );

	// initialize popContainer to queueEntry
	EdgeZlibDeflateQueueElement queueEntry;
	CellSpursLFQueuePopContainer popContainer;
	cellSpursLFQueuePopContainerInitialize( &popContainer, &queueEntry, kDmaTagId );

	while ( 1 )
	{
		//////////////////////////////////////////////////////////////////////////
		//
		//	Read an element off the Deflate Queue into queueEntry
		//
		//////////////////////////////////////////////////////////////////////////

		int ret = cellSpursLFQueuePopBegin( eaQueue, &popContainer );		//This will sleep if there's no work in the queue
		EDGE_ASSERT( CELL_OK == ret );
		ret = cellSpursLFQueuePopEnd( eaQueue, &popContainer );
		EDGE_ASSERT( CELL_OK == ret );


		//////////////////////////////////////////////////////////////////////////
		//
		// Get parameters from the queue entry and process this element
		//
		//////////////////////////////////////////////////////////////////////////

		ProcessDeflateQueueElement( &queueEntry );


		//////////////////////////////////////////////////////////////////////////
		//
		//	Yield to other SPURS job or task of higher priority
		//
		//////////////////////////////////////////////////////////////////////////

		uint32_t pollVal = cellSpursTaskPoll();
		if ( pollVal & CELL_SPURS_TASK_POLL_FOUND_WORKLOAD )	//Don't switch to other tasks in this task set, only higher priority workloads
		{
			ret = cellSpursYield();
			EDGE_ASSERT( CELL_OK == ret );
		}
	}

	// never reach here
}

//////////////////////////////////////////////////////////////////////////
