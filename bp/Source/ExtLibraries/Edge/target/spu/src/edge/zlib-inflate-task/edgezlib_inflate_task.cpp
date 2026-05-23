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

#include "edge/zlib/edgezlib_inflate_queue_element.h"
#include "edge/zlib/edgezlib_spu.h"

//////////////////////////////////////////////////////////////////////////

enum
{
	kEdgeZlibOverAllocateInputBufferSize	= 128,	//Need to reserve more than maximum buffer size due to mis-alignment of incoming data
	kEdgeZlibOverAllocateOutputBufferSize	= 128,	//Need to reserve more than maximum buffer size due to mis-alignment of outgoing data
};

unsigned char gEdgeZlibInflateInputBuf[kEdgeZlibInflateInputBufferMaxSize + kEdgeZlibOverAllocateInputBufferSize]  __attribute__((aligned(128)));
unsigned char gEdgeZlibInflateOutputBuf[kEdgeZlibInflateOutputBufferMaxSize + kEdgeZlibOverAllocateOutputBufferSize] __attribute__((aligned(128)));

unsigned char gEdgeZlibAtomicBuffer[128] __attribute__((aligned(128)));	//Only used temporarily for atomic transactions

enum
{
	kDmaTagId = 0,
};

//////////////////////////////////////////////////////////////////////////

static int DecompressInflateQueueElement( const EdgeZlibInflateQueueElement* queueEntry )
{
	uint32_t eaCompressed					= queueEntry->m_eaCompressed;
	uint32_t eaUncompressed					= queueEntry->m_eaUncompressed;
	uint32_t compressedSize					= queueEntry->m_compressedSize;
	uint16_t outputUncompSkipBeginSize		= queueEntry->m_outputUncompSkipBeginSize;
	uint32_t outputUncompPartialBuffSize	= queueEntry->m_outputUncompPartialBuffSize;
	uint16_t outputUncompSkipEndSize		= queueEntry->m_outputUncompSkipEndSize;
	uint32_t isCompressed					= ((queueEntry->m_eaWorkToDoCounter & kEdgeZlibInflateTask_ProcessingMask) == kEdgeZlibInflateTask_Inflate);

	uint32_t expectedUncompSize = outputUncompSkipBeginSize + outputUncompPartialBuffSize + outputUncompSkipEndSize;

	//eaCompressed and eaUncompressed need not be qword aligned
	//compressedSize and expectedUncompSize need not be a qword multiples

	if ( outputUncompSkipBeginSize + outputUncompSkipEndSize > expectedUncompSize )
	{
		EDGE_PRINTF( "EDGE ZLIB ERROR: outputUncompSkipBeginSize(%d) + outputUncompSkipEndSize(%d) > expectedUncompSize(%d)\n",
				outputUncompSkipBeginSize, outputUncompSkipEndSize, expectedUncompSize );
		EDGE_ASSERT( false );
		return 1;
	}

	if ( ( compressedSize > kEdgeZlibInflateInputBufferMaxSize )
		|| ( expectedUncompSize > kEdgeZlibInflateOutputBufferMaxSize ) )
	{
		EDGE_ASSERT( isCompressed == true );
		EDGE_ASSERT( outputUncompSkipBeginSize == 0 );

		int err = edgeZlibFetchAndInflateRawData(	eaUncompressed, expectedUncompSize,
												eaCompressed, compressedSize,
												kDmaTagId,
												gEdgeZlibInflateInputBuf,
												sizeof(gEdgeZlibInflateInputBuf),
												gEdgeZlibInflateOutputBuf,
												sizeof(gEdgeZlibInflateOutputBuf) );
		if ( 0 != err )
		{
			EDGE_ZLIB_DATA_ASSERT_MSG( false, ( "EDGE ZLIB ERROR: edgeZlibFetchAndInflateLargeRawData failed (%d)\n", err ) );
			return err;
		}
	}
	else
	{

		//////////////////////////////////////////////////////////////////////////
		//
		//	Fetch the input data for decompression (round up to a 128 byte multiple)
		//
		//////////////////////////////////////////////////////////////////////////

		uint32_t eaCompressedRoundDown	= eaCompressed & ~0x7F;
		uint32_t inputAlignOffset		= eaCompressed & 0x7F;
		uint32_t compressedSizeRoundUp	= compressedSize + inputAlignOffset;
		compressedSizeRoundUp = (compressedSizeRoundUp + 0x7F) & ~0x7F;

		EDGE_ASSERT( inputAlignOffset <= kEdgeZlibOverAllocateInputBufferSize );
		EDGE_ASSERT( (eaCompressedRoundDown + inputAlignOffset) == eaCompressed );
		EDGE_ASSERT( compressedSizeRoundUp <= sizeof(gEdgeZlibInflateInputBuf) );

		// DMA in input compressed data and wait for completion
		// (We aren't actually interested in the first "inputAlignOffset" bytes)
		EDGE_DMA_LARGE_GET( gEdgeZlibInflateInputBuf, eaCompressedRoundDown, compressedSizeRoundUp, kDmaTagId, 0, 0 );
		// wait for DMA to finish
		EDGE_DMA_WAIT_TAG_STATUS_ALL( 1 << kDmaTagId );

		const unsigned char* pInputBuffer = &gEdgeZlibInflateInputBuf[inputAlignOffset];

		//////////////////////////////////////////////////////////////////////////
		//
		//	Perform the decompression within LS
		//
		//////////////////////////////////////////////////////////////////////////

		//Compute the decompressed output into the LS buffer at the same offset as the main mem buffer has
		uint32_t outputAlignOffset		= (eaUncompressed - outputUncompSkipBeginSize) & 0x7F;
		EDGE_ASSERT( outputAlignOffset < kEdgeZlibOverAllocateOutputBufferSize );

		unsigned char* pLsOutputBuffer;

		if ( isCompressed )
		{
			pLsOutputBuffer							= &gEdgeZlibInflateOutputBuf[outputAlignOffset];

			//Perform the decompression
			int err = edgeZlibInflateRawData(	pLsOutputBuffer,
												expectedUncompSize,
												pInputBuffer,
												compressedSize );
			if ( 0 != err )
			{
				EDGE_ZLIB_DATA_ASSERT_MSG( false,
					( "EDGE ZLIB ERROR: edgeZlibInflateRawData failed (%d)\n",
						err ) );
				return 1;
			}
		}
		else
		{
			//If the data was stored uncompressed, then we just need to move it from one EA to another
			//but we don't want to run the inflate code on it.
			if ( compressedSize != expectedUncompSize )
			{
				//If the data was stored uncompressed, the compressedSize and expectedUncompSize should have been equal
				EDGE_ASSERT( false );
				return 1;
			}

			if ( inputAlignOffset != outputAlignOffset )
			{
				//We will move the input so that when it is output (after outputUncompSkipBeginSize)
				//the low 7 bits will be the same as the destination
				const void* pLsBufferSource				= pInputBuffer;
				pLsOutputBuffer							= &gEdgeZlibInflateOutputBuf[outputAlignOffset];
				memcpy( pLsOutputBuffer, pLsBufferSource, compressedSize );
			}
			else
			{
				//They're already both at the same alignment within 128 bytes
				pLsOutputBuffer		 		 		 	= &gEdgeZlibInflateInputBuf[inputAlignOffset];
			}
		}


		//////////////////////////////////////////////////////////////////////////
		//
		//	Send the results out to the specified Effective Address
		//
		//////////////////////////////////////////////////////////////////////////

		_edgeZlibUnalignedLargeDmaPut(	pLsOutputBuffer + outputUncompSkipBeginSize,
										eaUncompressed,
										expectedUncompSize - outputUncompSkipBeginSize - outputUncompSkipEndSize,
										kDmaTagId );
		EDGE_DMA_WAIT_TAG_STATUS_ALL( 1 << kDmaTagId );
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////

static void ProcessInflateQueueElement( const EdgeZlibInflateQueueElement* queueEntry )
{
	//////////////////////////////////////////////////////////////////////////
	//
	//	Fetch the input data, decompress it, and send it out again
	//
	//////////////////////////////////////////////////////////////////////////

	int err = DecompressInflateQueueElement( queueEntry );
	EDGE_ZLIB_DATA_ASSERT_MSG( 0 == err, ( "EDGE ZLIB ERROR: DecompressInflateQueueElement returned (%d)\n", err ) );


	//////////////////////////////////////////////////////////////////////////
	//
	//	Once the decompression is done, decrement the counter and if
	//	necessary set the event flag
	//
	//////////////////////////////////////////////////////////////////////////

	uint32_t eaWorkToDoCounter				= queueEntry->m_eaWorkToDoCounter & ~kEdgeZlibInflateTask_ProcessingMask;
	uint32_t eaEventFlag					= queueEntry->m_eaEventFlag;
	uint16_t eventFlagBits					= queueEntry->m_eventFlagBits;

	// decrement counter and maybe set event flag	
	bool setEventFlag = true;
	if ( eaWorkToDoCounter )
	{
		setEventFlag = false;

		if ( 0 != err )
		{	// Set hi bit of countdown value so PPU can see there was an error
			EDGE_ATOMIC_OR_32( (uint32_t*) gEdgeZlibAtomicBuffer, eaWorkToDoCounter, kEdgeZlibInflateTask_ErrorDuringDecompression );
		}

		uint32_t old = EDGE_ATOMIC_DECR_32( (uint32_t*) gEdgeZlibAtomicBuffer, eaWorkToDoCounter );

		old = old & ~kEdgeZlibInflateTask_DecompressionErrorMask;

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
	EdgeZlibInflateQueueElement queueEntry;
	CellSpursLFQueuePopContainer popContainer;
	cellSpursLFQueuePopContainerInitialize(	&popContainer, &queueEntry, kDmaTagId );

	while ( 1 )
	{
		//////////////////////////////////////////////////////////////////////////
		//
		//	Read an element off the Inflate Queue into queueEntry
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

		ProcessInflateQueueElement( &queueEntry );


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
