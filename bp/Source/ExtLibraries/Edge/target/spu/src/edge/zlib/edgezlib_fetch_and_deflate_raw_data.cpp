/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/zlib/edgezlib_spu.h"
#include <algorithm>

#include "edge/edge_dma.h"
#include "edge/edge_assert.h"

#include "edgezlib_zutil.h"

//////////////////////////////////////////////////////////////////////////

enum
{
	kLsMemEnd = 0x40000,
};

//////////////////////////////////////////////////////////////////////////

/** 
 * @brief	Compresses a buffer of uncompressed data in main memory to
 *			another main memory buffer.
 *
 * @note	This function fetches data into LS and sends data out of LS
 *			itself as necessary.
 *
 * @note	This function requires 2 temporary buffers in LS to work in.
 *			These can be specified as parameters ("pLsInputTempBuffer" and
 *			"pLsOutputTempBuffer").
 *
 * @return	Zero on success. Non-zero on failure.
 *
 * @param	eaOutputCompressedData		Effective Address of where the compressed data will be written to.
 * @param	maxCompressedOutputSize		The maximum space available for the compressed data.
 * @param	eaCompressedSize			The Effective Address of the uint32_t into which the output
 *										compressed size will be written. The top bit of the output
 *										size indicates whether the data was stored compressed or if the
 *										uncompressed original data was chosen for storing.
 * @param	eaInputUncompressedData		Effective Address of where the uncompressed data will be read from.
 * @param	uncompressedSize			Size of the uncompressed data buffer to be read.
 * @param	dmaTag						The DMA tag to be used for DMAs performed in this function.
 * @param	pLsInputTempBuffer			Pointer to the temporary buffer in Local Store to be used for
 *										reading input data into.
 *										The size of this buffer is specified by inputTempBuffSize.
 * @param	inputTempBuffSize			Size of the temporary buffer pointed to by pLsInputTempBuffer.
 * @param	pLsOutputTempBuffer			Pointer to the temporary buffer in Local Store to be used for
 *										computing the output data into and sending out from.
 *										The size of this buffer is specified by outputTempBuffSize.
 * @param	outputTempBuffSize			Size of the temporary buffer pointed to by pLsOutputTempBuffer.
 * @param	level						Choose the compression level between 0 and 9
 *											0 - no compression
 *											1 - fastest
 *											9 - smallest
 * @param	taskProcessing				If compression makes the data bigger, choose whether to store
 *										the compressed or the original data.
 *										Also, when compressing, choose whether to store the data with
 *										zlib header and footer, or whether to store just the 'raw'
 *										compressed data without any header or footer.
 */
int edgeZlibFetchAndDeflateRawData(
		uint32_t						eaOutputCompressedData,
		uint32_t						maxCompressedOutputSize,
		uint32_t						eaCompressedSize,
		uint32_t						eaInputUncompressedData,
		uint32_t						uncompressedSize,
		uint32_t						dmaTag,
		unsigned char*					pLsInputTempBuffer,
		uint32_t						inputTempBuffSize,
		unsigned char*					pLsOutputTempBuffer,
		uint32_t						outputTempBuffSize,
		uint32_t						level,
		EdgeZlibDeflateTaskProcessing	taskProcessing )
{
	EDGE_ASSERT( ((uint32_t)eaInputUncompressedData) >= kLsMemEnd );			//Double check the user hasn't passed in LS addresses by mistake
	EDGE_ASSERT( ((uint32_t)eaOutputCompressedData) >= kLsMemEnd );
	EDGE_ASSERT( ((uint32_t)pLsInputTempBuffer) < kLsMemEnd );		//Double check the user hasn't passed in effective addresses by mistake
	EDGE_ASSERT( ((uint32_t)pLsOutputTempBuffer) < kLsMemEnd );

	const uint32_t kEdgeZlibOverAllocateInputBufferSize = 128;		//Need to reserve more than maximum buffer size due to potential mis-alignment of incoming data
	const uint32_t kEdgeZlibOverAllocateOutputBufferSize = 128;		//Need to reserve more than maximum buffer size due to potential mis-alignment of outgoing data

	EDGE_ASSERT( (((uint32_t)pLsInputTempBuffer) & 0x7F) == 0 );
	EDGE_ASSERT( (inputTempBuffSize & 0x7F) == 0 );
	EDGE_ASSERT( (((uint32_t)pLsOutputTempBuffer) & 0x7F) == 0 );
	EDGE_ASSERT( (outputTempBuffSize & 0x7F) == 0 );
	uint32_t singleInputMaxSize				= inputTempBuffSize - kEdgeZlibOverAllocateInputBufferSize;
	EDGE_ASSERT( singleInputMaxSize > 0 );
	uint32_t singleOutputMaxSize			= outputTempBuffSize - kEdgeZlibOverAllocateOutputBufferSize;
	EDGE_ASSERT( singleOutputMaxSize > 0 );

	bool storeSmallest	= ((taskProcessing & kEdgeZlibDeflateTask_DeflateStoreWhatMask) == kEdgeZlibDeflateTask_DeflateStoreSmallest);
	bool storeHeader	= ((taskProcessing & kEdgeZlibDeflateTask_DeflateStoreWithHeaderMask) == kEdgeZlibDeflateTask_DeflateStoreCompressedWithHeader);

	// if size is > 1 SPU buffer, you can't store the smaller of the uncompressed & compressed data
	EDGE_ASSERT( !(storeSmallest  &&  uncompressedSize > kEdgeZlibDeflateInputBufferMaxSize) );

	uint32_t currEaInput					= eaInputUncompressedData;
	uint32_t currEaOutput					= eaOutputCompressedData;
	uint32_t remainingInputData				= uncompressedSize;
	uint32_t remainingOutputSpace;

	if( storeSmallest )
		remainingOutputSpace			= kEdgeZlibDeflateOutputBufferMaxSize;
	else
		remainingOutputSpace			= maxCompressedOutputSize;

	z_stream strm;

	strm.avail_in							= 0;
	strm.next_in							= Z_NULL;

	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;

	int  windowBits;
	if ( storeHeader )
		windowBits = MAX_WBITS;
	else
		windowBits = -MAX_WBITS;	//if windowBits < 0 then it suppresses the zlib wrapper

	//int ret = deflateInit( &strm, level );
	int ret = deflateInit2( &strm, level, Z_DEFLATED, windowBits, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY );
	if ( ret != Z_OK )
		return ret;

	uint32_t totalOutputSize = 0;
	int32_t flush = Z_NO_FLUSH;

	// assume we will have stored the compressed data
	bool storedCompressed = true;

	/* decompress until deflate stream ends or end of file */
	do
	{
		if ( remainingInputData == 0 )
			break;

		//Firstly, fetch up to "singleInputMaxSize" worth of input data
		uint32_t thisInputSize				= std::min( remainingInputData, singleInputMaxSize );

		uint32_t inputAlignOffset;
		{
			uint32_t eaInputRoundDown		= currEaInput & ~0x7F;
			inputAlignOffset				= currEaInput & 0x7F;
			uint32_t inputSizeRoundUp		= thisInputSize + inputAlignOffset;
			inputSizeRoundUp = (inputSizeRoundUp + 0x7F) & ~0x7F;
			EDGE_ASSERT( inputAlignOffset <= kEdgeZlibOverAllocateInputBufferSize );
			EDGE_ASSERT( (eaInputRoundDown + inputAlignOffset) == currEaInput );
			EDGE_ASSERT( inputSizeRoundUp <= inputTempBuffSize );

			spu_idisable();
			EDGE_DMA_LARGE_GET( pLsInputTempBuffer, eaInputRoundDown, inputSizeRoundUp, dmaTag, 0, 0 );
			EDGE_DMA_WAIT_TAG_STATUS_ALL( 1 << dmaTag );
			if ( interruptsEnabled )
				spu_ienable();

			strm.next_in					= &pLsInputTempBuffer[inputAlignOffset];
			strm.avail_in					= thisInputSize;
		}

		//We have consumed "thisInputSize" worth of the input data
		currEaInput							+= thisInputSize;
		remainingInputData					-= thisInputSize;

		if( remainingInputData == 0 )
		{
			flush = Z_FINISH;
		}
		EDGE_ASSERT( !(storeSmallest  &&  remainingInputData > 0) );

		//Set up our output buffer
		uint32_t thisOutputAlignOffset1		= currEaOutput & 0x7F;
		EDGE_ASSERT( thisOutputAlignOffset1 < kEdgeZlibOverAllocateOutputBufferSize );
		unsigned char* pThisOutputBase		= &pLsOutputTempBuffer[thisOutputAlignOffset1];
		strm.next_out						= pThisOutputBase;
		strm.avail_out						= singleOutputMaxSize;

		// Keep running deflate() on input and sending out data to main memory until we've consumed all input data
		while ( true )
		{
			ret = deflate( &strm, flush );
			EDGE_ASSERT( ret != Z_STREAM_ERROR ); // no bad return value for data

			uint32_t thisOutputSize			= singleOutputMaxSize - strm.avail_out;
			EDGE_ASSERT( thisOutputSize <= singleOutputMaxSize );
			if ( thisOutputSize > remainingOutputSpace )
			{
				//Ran out of buffer space in main memory
				EDGE_ASSERT_MSG( false, ( "EDGE ZLIB ERROR: buffer for output compressed data not big enough\n" ) );
				deflateEnd( &strm );
				return Z_BUF_ERROR;
			}

			if ( thisOutputSize )
			{
				//Flush the LS buffer

				EDGE_ASSERT( !(storeSmallest  &&  ret != Z_STREAM_END) );

				if( storeSmallest  &&  thisOutputSize >= uncompressedSize )
				{	// output uncompressed data

					storedCompressed = false;
					thisOutputSize = uncompressedSize;

					if( eaOutputCompressedData == eaInputUncompressedData )
					{	// user overlaid input & output, so the output is already valid
						// and we don't have to do a DMA!
					}
					else
					{	// DMA uncompressed data
						_edgeZlibUnalignedLargeDmaPut(
								&pLsInputTempBuffer[inputAlignOffset],
								eaOutputCompressedData, uncompressedSize, dmaTag );
						spu_idisable();
						EDGE_DMA_WAIT_TAG_STATUS_ALL( 1 << dmaTag );
						if ( interruptsEnabled )
							spu_ienable();
					}
				}
				else
				{	// output compressed data
					_edgeZlibUnalignedLargeDmaPut( pThisOutputBase, currEaOutput, thisOutputSize, dmaTag );
					spu_idisable();
					EDGE_DMA_WAIT_TAG_STATUS_ALL( 1 << dmaTag );
					if ( interruptsEnabled )
						spu_ienable();
				}

				remainingOutputSpace		-= thisOutputSize;
				currEaOutput				+= thisOutputSize;

				totalOutputSize				+= thisOutputSize;
			}

			if ( strm.avail_out )
			{
				//If there was still output buffer space available
				//then we must have run out of input data, so break out
				//of the inner while loop and fetch more input data
				break;
			}

			if ( thisOutputSize )
			{
				//Start filling the output buffer from the start again
				uint32_t thisOutputAlignOffset2	= currEaOutput & 0x7F;
				EDGE_ASSERT( thisOutputAlignOffset2 < kEdgeZlibOverAllocateOutputBufferSize );
				pThisOutputBase				= &pLsOutputTempBuffer[thisOutputAlignOffset2];

				strm.next_out				= pThisOutputBase;
				strm.avail_out				= singleOutputMaxSize;
			}
		}

		// Keep fetching more input data until deflate tells us we've reached the end of the stream

	} while (flush != Z_FINISH);
	EDGE_ASSERT(ret == Z_STREAM_END);			/* stream will be complete */

	EDGE_ASSERT( totalOutputSize <= maxCompressedOutputSize );
	
	// clean up and return
	deflateEnd( &strm );

	// output compressed size, and compressed flag (hi bit), back to PPU
	if( storedCompressed )
		totalOutputSize |= kEdgeZlibDeflateTask_CompressedWasStored;

	uint32_t tempOutputSize[4] __attribute__((aligned(16)));
	uint32_t* pTempOutputSize = (uint32_t*)( (uint32_t)&tempOutputSize[0] | (eaCompressedSize & 0xF) );
	*pTempOutputSize = totalOutputSize;

	spu_idisable();
	EDGE_DMA_SMALL_PUT( pTempOutputSize, eaCompressedSize, 4, dmaTag, 0, 0 );
	EDGE_DMA_WAIT_TAG_STATUS_ALL( 1 << dmaTag );
	if ( interruptsEnabled )
		spu_ienable();

	return ((ret == Z_STREAM_END) ? Z_OK : Z_DATA_ERROR);
}

//////////////////////////////////////////////////////////////////////////
