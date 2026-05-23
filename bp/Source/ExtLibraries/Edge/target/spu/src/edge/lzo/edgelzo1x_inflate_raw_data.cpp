/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/edge_printf.h"
#include "edge/edge_assert.h"
#include "edge/lzo/edgelzo_spu.h"
#include "lzo/edgelzo_lzoconf.h"
#include "lzo/lzopro/edgelzo_lzo1x.h"
#include "minilzo/edgelzo_minilzo.h"

//////////////////////////////////////////////////////////////////////////

enum
{
	kLsMemEnd = 0x40000,
};

//////////////////////////////////////////////////////////////////////////

/** 
 * @brief	Decompresses a buffer of compressed data.
 *
 * @note	The input and output buffers are both in LS. Providing the
 *			input data and dealing with the output data is expected to be
 *			handled by the function which calls edgeLzo1xInflateRawData.
 *			This function doesn’t do any DMAs internally.
 *
 * @note	This function will assert if the expectedUncompSize disagrees
 *			with the actual uncompressed size.
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

extern "C" int edgeLzo1xInflateRawData(
		unsigned char*					pUncompr,
		uint32_t						expectedUncompSize,
		const unsigned char*			pComprData,
		uint32_t						comprDataSize )
{
	EDGE_ASSERT( ((uint32_t)pUncompr) < kLsMemEnd );		//Double check the user hasn't passed in an effective addresses by mistake
	EDGE_ASSERT( ((uint32_t)pComprData) < kLsMemEnd );

	// decompress data
	lzo_uint returnedUncompSize = expectedUncompSize;
	int ret = lzo1x_decompress_safe_asm( pComprData, comprDataSize, pUncompr, &returnedUncompSize, NULL );
	//int ret = lzo1x_decompress_safe( pComprData, comprDataSize, pUncompr, &returnedUncompSize, NULL );

	if (ret == LZO_E_OK  &&  returnedUncompSize == expectedUncompSize)
	{
		return 0; // OK
	}
	else
	{
		/* this should NEVER happen */
		EDGE_LZO1X_DATA_ASSERT_MSG( false,
			("EDGE LZO1X Inflate Task: decompression failed: %d\n Returned uncompressed size = 0x%x, expected size = 0x%x\n",
				ret, (unsigned int)returnedUncompSize, expectedUncompSize ) );

		return 1;
	}
}

//////////////////////////////////////////////////////////////////////////
