/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/edge_printf.h"
#include "edge/edge_assert.h"
#include "edge/lzma/edgelzma_spu.h"
#include "edgelzma_LzmaDecode.h"

//////////////////////////////////////////////////////////////////////////

enum
{
	kLsMemEnd = 0x40000,
};

//////////////////////////////////////////////////////////////////////////

#define EDGE_LZMA_CPROB_SIZE	0x10000
#define EDGE_LZMA_CPROB_ALIGN	0x80

static unsigned char edgeLzmaCProbBuf[EDGE_LZMA_CPROB_SIZE]  __attribute__((aligned(EDGE_LZMA_CPROB_ALIGN)));

//////////////////////////////////////////////////////////////////////////

/** 
 * @brief	Decompresses a buffer of compressed data.
 *
 * @note	The input and output buffers are both in LS. Providing the
 *			input data and dealing with the output data is expected to be
 *			handled by the function which calls edgeLzmaInflateRawData.
 *			This function doesn’t do any DMAs internally.
 *
 * @note	This function will assert if the expectedUncompSize disagrees
 *			with the actual uncompressed size.
 *
 * @note	This function expects a pointer to the raw compressed data
 *			without any header.
 *
 * @note	If any data error is encountered, this function will not assert
 *			(unless a conditional define is changed) and instead returns an
 *			error value to its caller.
 *
 * @return	Zero on success. Non-zero on failure.
 *
 * @param	pUncompr			Pointer to where the uncompressed data will be written to in Local Store.
 * @param	expectedUncompSize	The expected output size of the uncompressed data.
 *								The output buffer pointed to by pUncompr must be at least this large.
 * @param	pComprData			Pointer to where the compressed data will be read from in Local Store.
 *								This should be a pointer to the raw data without any header.
 * @param	comprDataSize		Size of the compressed data buffer to be read.
 */

extern "C" int edgeLzmaInflateRawData(
		unsigned char*					pUncompr,
		uint32_t						expectedUncompSize,
		const unsigned char*			pProperties,
		uint32_t						propertiesSize,
		const unsigned char*			pComprData,
		uint32_t						comprDataSize )
{
	EDGE_ASSERT( ((uint32_t)pUncompr) < kLsMemEnd );		//Double check the user hasn't passed in an effective addresses by mistake
	EDGE_ASSERT( ((uint32_t)pComprData) < kLsMemEnd );

	EDGE_ASSERT( propertiesSize >= 1 );

	// decode properties
	CLzmaDecoderState state;
	if ( LzmaDecodeProperties( &state.Properties, pProperties, propertiesSize ) != LZMA_RESULT_OK )
	{
		EDGE_LZMA_DATA_ASSERT_MSG( false,
			("********************\n*edgeLzmaInflateRawData error: Incorrect stream properties\n********************\n*") );
		return LZMA_RESULT_DATA_ERROR;
	}

	uint32_t numProbs	= LzmaGetNumProbs( &state.Properties );
	uint32_t sizeProbs	= numProbs * sizeof(CProb);
	sizeProbs = (sizeProbs + 0xF) & ~0xF;

	if( sizeProbs > EDGE_LZMA_CPROB_SIZE )
	{
		EDGE_LZMA_DATA_ASSERT_MSG( false,
			("********************\n*edgeLzmaInflateRawData error: sizeProbs (%d) must be <= EDGE_LZMA_CPROB_SIZE (0x%x)\n********************\n",
				sizeProbs, EDGE_LZMA_CPROB_SIZE ) );
		return LZMA_RESULT_DATA_ERROR;
	}

	// Allocate a block for internal structures.
	// Note: make this 16 aligned and pad size to 16 bytes as needed by SPU assembly version
#ifdef __SPU__
	state.Probs = (CProb*)&edgeLzmaCProbBuf[0];
#else
#error "target/spu/src/edge/lzma/*" code may not be threadsafe if compiled for non-SPU
#endif

	if( ((UInt32)state.Probs & 0xF) != 0 )
	{
		EDGE_LZMA_DATA_ASSERT_MSG( false,
			("********************\n*edgeLzmaInflateRawData error: state.Probs (0x%x) must have low 4 bits = 0\n********************\n",
				(UInt32)state.Probs ) );
		return LZMA_RESULT_DATA_ERROR;
	}

	// decompress data (ram to ram)
	uint32_t compressedBytesProcessed;
	uint32_t uncompressedBytesProcessed;
	int ret = LzmaDecode_spu_asm( &state, 
		pComprData, comprDataSize, &compressedBytesProcessed,
		pUncompr, expectedUncompSize, &uncompressedBytesProcessed );

	if( ret != LZMA_RESULT_OK )
	{
		EDGE_LZMA_DATA_ASSERT_MSG( false,
			("********************\n*edgeLzmaInflateRawData error: LzmaDecode returned 0x%x\n********************\n",
				ret ) );
		return LZMA_RESULT_DATA_ERROR;
	}

	if( compressedBytesProcessed != comprDataSize )
	{
		EDGE_LZMA_DATA_ASSERT_MSG( false,
			("********************\n*edgeLzmaInflateRawData error: compressed bytes processed (0x%x) is not value expected (0x%x)\n********************\n",
				compressedBytesProcessed, comprDataSize ) );
		return LZMA_RESULT_DATA_ERROR;
	}

	if( uncompressedBytesProcessed != expectedUncompSize )
	{
		EDGE_LZMA_DATA_ASSERT_MSG( false,
			("********************\n*edgeLzmaInflateRawData error: uncompressed bytes processed (0x%x) is not value expected (0x%x)\n********************\n",
				uncompressedBytesProcessed, expectedUncompSize ) );
		return LZMA_RESULT_DATA_ERROR;
	}

	return LZMA_RESULT_OK;
}

//////////////////////////////////////////////////////////////////////////
