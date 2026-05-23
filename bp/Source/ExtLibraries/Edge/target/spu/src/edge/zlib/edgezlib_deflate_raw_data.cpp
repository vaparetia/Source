/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/edge_printf.h"
#include "edge/edge_assert.h"
#include "edge/zlib/edgezlib_spu.h"
#include "edgezlib_deflate.h"

//////////////////////////////////////////////////////////////////////////

enum
{
	kLsMemEnd = 0x40000,
};

//////////////////////////////////////////////////////////////////////////

/** 
 * @brief	Compresses a buffer of uncompressed data.
 *
 * @note	The input and output buffers are both in LS. Providing the
 *			input data and dealing with the output data is expected to be
 *			handled by the function which calls edgeZlibDeflateRawData.
 *			This function doesn’t do any DMAs internally.
 *
 * @return	Zero on success. Non-zero on failure.
 *
 * @param	pUncompr				Pointer to where the uncompressed data will be read from in Local Store.
 * @param	uncompSize				The size of the input uncompressed data which will be compressed.
 * @param	pComprData				Pointer to where the compressed data will be written to in Local Store.
 *									This area will be filled with the raw compressed data without any header.
 * @param	maxCompressedDataSize	Maximum size of the compressed data buffer.
 * @param	pOutputCompressedSize	Returns the output size of the compressed data.
 * @param	level					Choose the compression level between 0 and 9
 *										0 - no compression
 *										1 - fastest
 *										9 - smallest
 */

extern "C" int edgeZlibDeflateRawData(
		const unsigned char*	pUncompr,
		uint32_t				uncompSize,
		unsigned char*			pComprData,
		uint32_t				maxCompressedDataSize,
		uint32_t*				pOutputCompressedSize,
		uint32_t				level )
{
	EDGE_ASSERT( ((uint32_t)pUncompr) < kLsMemEnd );		//Double check the user hasn't passed in effective addresses by mistake
	EDGE_ASSERT( ((uint32_t)pComprData) < kLsMemEnd );

	*pOutputCompressedSize = 0;		//Set to zero now, just in case we abort early

	z_stream stream;
	stream.next_in		= pUncompr;
	stream.avail_in		= uncompSize;

	stream.next_out		= pComprData;
	stream.avail_out	= maxCompressedDataSize;

	int err = deflateInit2( &stream, level, Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY );
	if ( err != Z_OK )
	{
		EDGE_ASSERT( false );
		return err;
	}

	err = deflate( &stream, Z_FINISH );
	if ( err != Z_STREAM_END )
	{
		EDGE_ASSERT( false );
		deflateEnd( &stream );
		return (err == Z_OK ? Z_BUF_ERROR : err);
	}

	EDGE_ASSERT( stream.total_out <= maxCompressedDataSize );

	*pOutputCompressedSize = stream.total_out;

	err = deflateEnd( &stream );
	return err;
}

//////////////////////////////////////////////////////////////////////////
