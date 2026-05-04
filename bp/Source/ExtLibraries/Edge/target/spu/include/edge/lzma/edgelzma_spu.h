/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_LZMA_SPU_H__
#define __EDGE_LZMA_SPU_H__

#include "edge/edge_stdint.h"
#include "edge/edge_assert.h"

#include "edge/lzma/edgelzma_inflate_queue_element.h"

#ifdef __cplusplus
extern "C" 
{
#endif // __cplusplus

//////////////////////////////////////////////////////////////////////////

// EDGE_LZMA_ENABLE_DATA_ASSERTS can be set to either 0 or 1, to
// respectively disable or enable the optional asserts for bad data.
// By default they are disabled.
//
// If enabled, the SPU will assert as soon as it sees an error which
// might be caused by bad data.
// When disabled, if the SPU sees an error cause by bad data, instead of
// asserting, it will report the error by setting the top bit of the
// work-to-do-counter.  The PPU can then, say, re-load the data and
// re-submit it for decompression.
// Note that if the data asserts are disabled, then we still output the
// message anyway, but leave the SPU to carry on and recover.

#ifndef EDGE_LZMA_ENABLE_DATA_ASSERTS
	#define EDGE_LZMA_ENABLE_DATA_ASSERTS 0
#endif

#if EDGE_LZMA_ENABLE_DATA_ASSERTS
	#define EDGE_LZMA_DATA_ASSERT_MSG( condition, format )			\
		do															\
		{															\
			if ( __builtin_expect( !(condition), 0 ) )				\
			{														\
				EDGE_PRINTF format ;								\
				_edgeAssertionFailure( __FILE__, __LINE__ );		\
			}														\
		} while ( 0 )
#else
	#define EDGE_LZMA_DATA_ASSERT_MSG( condition, format )			\
		do															\
		{															\
			if ( __builtin_expect( !(condition), 0 ) )				\
			{														\
				EDGE_PRINTF format ;								\
			}														\
		} while ( 0 )
#endif

//////////////////////////////////////////////////////////////////////////

typedef uint32_t EdgeLzmaInflateQHandle;

//////////////////////////////////////////////////////////////////////////

extern int edgeLzmaInflateRawData(
		unsigned char*					pUncompr,
		uint32_t						expectedUncompSize,
		const unsigned char*			pProperties,
		uint32_t						propertiesSize,
		const unsigned char*			pComprData,
		uint32_t						comprDataSize );

extern void edgeLzmaAddInflateQueueElement(
		EdgeLzmaInflateQHandle			handle,
		const unsigned char*			pProperties,
		uint32_t						propertiesSize,
		uint32_t						eaInputCompressedData,
		uint32_t						compressedSize,
		uint32_t						eaOutputUncompressedData,
		uint32_t						expectedUncompressedSize,
		uint32_t						eaWorkToDoCounter,
		uint32_t						eaEventFlag,
		uint16_t						eventFlagBits,
		EdgeLzmaInflateTaskProcessing	processing,
		uint32_t						dmaTag );

extern bool edgeLzmaTryAddInflateQueueElement(
		EdgeLzmaInflateQHandle			handle,
		const unsigned char*			pProperties,
		uint32_t						propertiesSize,
		uint32_t						eaInputCompressedData,
		uint32_t						compressedSize,
		uint32_t						eaOutputUncompressedData,
		uint32_t						expectedUncompressedSize,
		uint32_t						eaWorkToDoCounter,
		uint32_t						eaEventFlag,
		uint16_t						eventFlagBits,
		EdgeLzmaInflateTaskProcessing	processing,
		uint32_t						dmaTag );

extern void edgeLzmaAddInflateQueueElementPartialCopyOut(
		EdgeLzmaInflateQHandle			handle,
		const unsigned char*			pProperties,
		uint32_t						propertiesSize,
		uint32_t						eaInputCompressedData,
		uint32_t						compressedSize,
		uint32_t						eaOutputUncompPartialBuff,
		uint16_t						outputUncompSkipBeginSize,
		uint32_t						outputUncompPartialBuffSize,
		uint16_t						outputUncompSkipEndSize,
		uint32_t						eaWorkToDoCounter,
		uint32_t						eaEventFlag,
		uint16_t						eventFlagBits,
		EdgeLzmaInflateTaskProcessing	processing,
		uint32_t						dmaTag );

extern bool edgeLzmaTryAddInflateQueueElementPartialCopyOut(
		EdgeLzmaInflateQHandle			handle,
		const unsigned char*			pProperties,
		uint32_t						propertiesSize,
		uint32_t						eaInputCompressedData,
		uint32_t						compressedSize,
		uint32_t						eaOutputUncompPartialBuff,
		uint16_t						outputUncompSkipBeginSize,
		uint32_t						outputUncompPartialBuffSize,
		uint16_t						outputUncompSkipEndSize,
		uint32_t						eaWorkToDoCounter,
		uint32_t						eaEventFlag,
		uint16_t						eventFlagBits,
		EdgeLzmaInflateTaskProcessing	processing,
		uint32_t						dmaTag );

//////////////////////////////////////////////////////////////////////////

extern void _edgeLzmaUnalignedLargeDmaPut( const void* ls, uint32_t ea, uint32_t size, uint32_t tag );

//////////////////////////////////////////////////////////////////////////


#ifdef __cplusplus
}
#endif // __cplusplus

#endif	//__EDGE_LZMA_SPU_H__
