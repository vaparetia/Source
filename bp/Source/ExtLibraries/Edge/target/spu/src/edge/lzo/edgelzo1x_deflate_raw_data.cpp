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

//////////////////////////////////////////////////////////////////////////

enum
{
	kLsMemEnd = 0x40000,
};

//////////////////////////////////////////////////////////////////////////

/* Work-memory needed for compression. Allocate memory in units
 * of `lzo_align_t' (instead of `char') to make sure it is properly aligned.
 */

#define HEAP_ALLOC(var,size) \
	lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

static HEAP_ALLOC(wrkmem,LZOPRO_LZO1X_1_14_MEM_COMPRESS);

//////////////////////////////////////////////////////////////////////////

/** 
 * @brief	Compresses a buffer of uncompressed data.
 *
 * @note	The input and output buffers are both in LS. Providing the
 *			input data and dealing with the output data is expected to be
 *			handled by the function which calls edgeLzo1xDeflateRawData.
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
 */

extern "C" int edgeLzo1xDeflateRawData(
		const unsigned char*			pUncompr,
		uint32_t						uncompSize,
		unsigned char*					pComprData,
		uint32_t						maxCompressedDataSize,
		uint32_t*						pOutputCompressedSize )
{
	EDGE_ASSERT( ((uint32_t)pUncompr) < kLsMemEnd );		//Double check the user hasn't passed in effective addresses by mistake
	EDGE_ASSERT( ((uint32_t)pComprData) < kLsMemEnd );

	*pOutputCompressedSize = 0;		//Set to zero now, just in case we abort early

	// get compressed allocation, and ensure it is big enough
	uint32_t comprAllocation = maxCompressedDataSize;
	EDGE_ASSERT( comprAllocation >= (uncompSize + (uncompSize / 16) + 64 + 3) );

	if ( 0 )
	{
		// Clear work mem, since Lzopro uses it without initializing it.
		// This isn't done for the assembly version since it has code added to do it.
		// This isn't strictly necessary but without it, output would be non-deterministic.
		uint8_t* pWrkmem = (uint8_t*)wrkmem;
		for ( uint32_t i = 0 ; i < LZOPRO_LZO1X_1_14_MEM_COMPRESS ; i++ )
		{
			pWrkmem[i] = 0;
		}
	}

	// compress data
	lzo_uint comprSize = maxCompressedDataSize;
	int ret = lzopro_lzo1x_1_14_compress_asm( pUncompr, uncompSize, pComprData, (lzo_uint*)&comprSize, wrkmem );
	//int ret = lzopro_lzo1x_1_14_compress( pUncompr, uncompSize, pComprData, (lzo_uint*)&comprSize, wrkmem );

	if ( (ret == LZO_E_OK) && (comprSize <= comprAllocation) )
	{
		*pOutputCompressedSize = comprSize;
		return 0;	// OK
	}
	else
	{
		/* this should NEVER happen */
		EDGE_PRINTF("EDGE LZO1X Deflate Task: internal error - compression failed: %d\n", ret);
		EDGE_ASSERT( false );
		return 1;
	}
}

//////////////////////////////////////////////////////////////////////////
