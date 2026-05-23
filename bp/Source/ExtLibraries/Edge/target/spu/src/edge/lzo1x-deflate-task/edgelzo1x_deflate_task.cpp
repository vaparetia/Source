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
#include "edge/edge_dma.h"
#include "edge/edge_printf.h"
#include "edge/edge_assert.h"

#include "edge/lzo/edgelzo1x_deflate_queue_element.h"
#include "edge/lzo/edgelzo_spu.h"

//////////////////////////////////////////////////////////////////////////

enum
{
	kEdgeLzo1xOverAllocateInputBufferSize  = 128,	//Need to reserve more than maximum buffer size due to mis-alignment of incoming data
	kEdgeLzo1xOverAllocateOutputBufferSize = 128,	//Need to reserve more than maximum buffer size due to mis-alignment of outgoing data
};

unsigned char gEdgeLzo1xDeflateInputBuf [kEdgeLzo1xDeflateInputBufferMaxSize  + kEdgeLzo1xOverAllocateInputBufferSize]	__attribute__((aligned(128)));
unsigned char gEdgeLzo1xDeflateOutputBuf[kEdgeLzo1xDeflateOutputBufferMaxSize + kEdgeLzo1xOverAllocateOutputBufferSize]	__attribute__((aligned(128)));

unsigned char gEdgeLzo1xAtomicBuffer[128] __attribute__((aligned(128)));	//Only used temporarily for atomic transactions

enum
{
	kDmaTagId = 0,
};

//////////////////////////////////////////////////////////////////////////

static int CompressDeflateQueueElement( EdgeLzo1xDeflateQueueElement* queueEntry )
{
	uint32_t	eaInputUncompressedData		= queueEntry->m_eaInputUncompressedData;
	uint32_t	eaOutputCompressedData		= queueEntry->m_eaOutputCompressedData;
	uint32_t	uncompressedSize			= queueEntry->m_uncompressedSize;
	uint32_t	maxCompressedOutputSize		= queueEntry->m_maxCompressedOutputSize;
	uint32_t	eaOutputCompressedSize		= queueEntry->m_eaOutputCompressedSize;
	bool storeSmallest						=
		((queueEntry->m_eaWorkToDoCounter & kEdgeLzo1xDeflateTask_DeflateStoreWhatMask) == kEdgeLzo1xDeflateTask_DeflateStoreSmallest);

	if ( uncompressedSize > kEdgeLzo1xDeflateInputBufferMaxSize )
	{
		EDGE_LZO1X_DATA_ASSERT_MSG( false, ( "EDGE LZO1X ERROR: Input data for compression is too big (%d > %d)\n", uncompressedSize, kEdgeLzo1xDeflateInputBufferMaxSize) );
		return 1;
	}


	//////////////////////////////////////////////////////////////////////////
	//
	//	Fetch the input data for compression (round up to a 128 byte multiple)
	//
	//////////////////////////////////////////////////////////////////////////

	uint32_t eaUncompressedRoundDown	= eaInputUncompressedData & ~0x7F;
	uint32_t inputAlignOffset			= eaInputUncompressedData & 0x7F;
	uint32_t uncompressedSizeRoundUp	= uncompressedSize + inputAlignOffset;
	uncompressedSizeRoundUp = (uncompressedSizeRoundUp + 0x7F) & ~0x7F;

	EDGE_ASSERT( inputAlignOffset <= kEdgeLzo1xOverAllocateInputBufferSize );
	EDGE_ASSERT( (eaUncompressedRoundDown + inputAlignOffset) == eaInputUncompressedData );
	EDGE_ASSERT( uncompressedSizeRoundUp <= sizeof(gEdgeLzo1xDeflateInputBuf) );

	// DMA in input uncompressed data and wait for completion
	// (We aren't actually interested in the first "inputAlignOffset" bytes)
	EDGE_DMA_LARGE_GET( gEdgeLzo1xDeflateInputBuf, eaUncompressedRoundDown, uncompressedSizeRoundUp, kDmaTagId, 0, 0 );
	// wait for DMA to finish
	EDGE_DMA_WAIT_TAG_STATUS_ALL( 1 << kDmaTagId );


	//////////////////////////////////////////////////////////////////////////
	//
	//	Perform the compression within LS
	//
	//////////////////////////////////////////////////////////////////////////

	//Compute the compressed output into the LS buffer at the same offset as the main mem buffer has
	uint32_t outputAlignOffset		= eaOutputCompressedData & 0x7F;
	EDGE_ASSERT( outputAlignOffset < kEdgeLzo1xOverAllocateOutputBufferSize );

	unsigned char* pLsOutputBuffer;
	pLsOutputBuffer							= &gEdgeLzo1xDeflateOutputBuf[outputAlignOffset];

	//Perform the compression
	uint32_t compressedSize = kEdgeLzo1xDeflateOutputBufferMaxSize;
	int err = edgeLzo1xDeflateRawData(	&gEdgeLzo1xDeflateInputBuf[inputAlignOffset],
										uncompressedSize,
										pLsOutputBuffer,
										kEdgeLzo1xDeflateOutputBufferMaxSize,
										&compressedSize );
	if ( 0 != err )
	{
		EDGE_LZO1X_DATA_ASSERT_MSG( false, ( "EDGE LZO1X ERROR: edgeLzo1xDeflateRawData failed (%d)\n", err ) );
		return 1;
	}


	//////////////////////////////////////////////////////////////////////////
	//
	//	Send the results out to the specified Effective Address,
	//	and set the output size & compressed flag
	//
	//////////////////////////////////////////////////////////////////////////

	bool outputCompressed;
	const uint8_t* pOutputData;
	uint32_t outputSize;

	if( compressedSize >= uncompressedSize  &&  storeSmallest )
	{	// compressed data is bigger, and in that case user wants to output uncompressed data

		outputCompressed = false;
		pOutputData = &gEdgeLzo1xDeflateInputBuf[0] + inputAlignOffset;
		outputSize = uncompressedSize;
	}
	else
	{	// compressed data is smaller, or user wants to output compressed data anyway

		outputCompressed = true;
		pOutputData = pLsOutputBuffer;
		outputSize = compressedSize;
	}

	if( !outputCompressed  &&  (eaInputUncompressedData == eaOutputCompressedData) )
	{
		// we are outputting uncompressed data into the same ea that it came from
		// so we don't have to output anything
	}
	else
	{
		// don't allow write beyond allocation
		EDGE_ASSERT( outputSize <= kEdgeLzo1xDeflateOutputBufferMaxSize );

		if ( outputSize > maxCompressedOutputSize )
		{
			EDGE_PRINTF( "EDGE LZO1X ERROR: Size of compressed data is %d.  Maximum is %d.\n", outputSize, maxCompressedOutputSize );
			EDGE_ASSERT( false );
			return 1;
		}

		_edgeLzoUnalignedLargeDmaPut(	pOutputData, eaOutputCompressedData, outputSize, kDmaTagId );
		// We don't have to wait here since we do a wait further below
		//EDGE_DMA_WAIT_TAG_STATUS_ALL( 1 << kDmaTagId );
	}

	// output compressed size, and compressed flag (hi bit), back to main memory
	if( outputCompressed )
		outputSize |= kEdgeLzo1xDeflateTask_CompressedWasStored;

	uint32_t tempOutputSize[4] __attribute__((aligned(16)));
	uint32_t* pTempOutputSize = (uint32_t*)( (uint32_t)&tempOutputSize[0] | (eaOutputCompressedSize & 0xF) );
	*pTempOutputSize = outputSize;

	EDGE_DMA_SMALL_PUT( pTempOutputSize, eaOutputCompressedSize, 4, kDmaTagId, 0, 0 );
	EDGE_DMA_WAIT_TAG_STATUS_ALL( 1 << kDmaTagId );

	return 0;
}

//////////////////////////////////////////////////////////////////////////

static void ProcessDeflateQueueElement( EdgeLzo1xDeflateQueueElement* queueEntry )
{
	//////////////////////////////////////////////////////////////////////////
	//
	//	Fetch the input data, compress it, and send it out again
	//
	//////////////////////////////////////////////////////////////////////////

	int err = CompressDeflateQueueElement( queueEntry );
	EDGE_LZO1X_DATA_ASSERT_MSG( 0 == err, ( "EDGE LZO1X ERROR: CompressDeflateQueueElement returned (%d)\n", err ) );


	//////////////////////////////////////////////////////////////////////////
	//
	//	Once the compression is done, decrement the counter and if
	//	necessary set the event flag
	//
	//////////////////////////////////////////////////////////////////////////

	uint32_t eaWorkToDoCounter	= queueEntry->m_eaWorkToDoCounter & ~kEdgeLzo1xDeflateTask_DeflateStoreWhatMask;
	uint32_t eaEventFlag		= queueEntry->m_eaEventFlag;
	uint16_t eventFlagBits		= queueEntry->m_eventFlagBits;

	// decrement counter and maybe set event flag
	bool setEventFlag = true;
	if ( eaWorkToDoCounter )
	{
		setEventFlag = false;

		if ( 0 != err )
		{	// Set hi bit of countdown value so PPU can see there was an error
			EDGE_ATOMIC_OR_32( (uint32_t*) gEdgeLzo1xAtomicBuffer, eaWorkToDoCounter, kEdgeLzo1xDeflateTask_ErrorDuringCompression );
		}

		uint32_t old = EDGE_ATOMIC_DECR_32( (uint32_t*) gEdgeLzo1xAtomicBuffer, eaWorkToDoCounter );

		old = old & ~kEdgeLzo1xDeflateTask_CompressionErrorMask;

		if ( 0 == old )
		{
			EDGE_PRINTF( "EDGE LZO1X ERROR: Attempted to decrement a work-to-do counter already at zero\n" );
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
	EdgeLzo1xDeflateQueueElement queueEntry;
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
