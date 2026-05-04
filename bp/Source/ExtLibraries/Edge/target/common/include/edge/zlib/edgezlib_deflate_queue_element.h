/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_ZLIB_DEFLATE_QUEUE_ELEMENT_H__
#define __EDGE_ZLIB_DEFLATE_QUEUE_ELEMENT_H__

//////////////////////////////////////////////////////////////////////////

// Options to choose what output from compression to store
// This gets ORed into the m_eaWorkToDoCounter of the EdgeZlibDeflateQueueElement
typedef enum EdgeZlibDeflateTaskProcessing
{
	kEdgeZlibDeflateTask_DeflateStoreCompressed				= 0,
	kEdgeZlibDeflateTask_DeflateStoreSmallest				= 1,
	kEdgeZlibDeflateTask_DeflateStoreCompressedWithHeader	= 2,
	//kEdgeZlibDeflateTask_DeflateStoreSmallestWithHeader	= 3,	//Invalid option - StoreSmallest is going to be raw master data (ie. no zlib header)

	kEdgeZlibDeflateTask_DeflateStoreWhatMask				= 0x1,
	kEdgeZlibDeflateTask_DeflateStoreWithHeaderMask			= 0x2,
	kEdgeZlibDeflateTask_ProcessingMask						= kEdgeZlibDeflateTask_DeflateStoreWhatMask | kEdgeZlibDeflateTask_DeflateStoreWithHeaderMask,
} EdgeZlibDeflateTaskProcessing;

// Tell user whether the uncompressed or compressed data was stored
// This gets ORed into the returned size
typedef enum EdgeZlibDeflateTaskProcessingStored
{
	kEdgeZlibDeflateTask_UncompressedWasStored				= 0x00000000,
	kEdgeZlibDeflateTask_CompressedWasStored				= 0x80000000,

	kEdgeZlibDeflateTask_HowStoredMask						= 0x80000000,
} EdgeZlibDeflateTaskProcessingStored;

//This gets ORed into the workToDoCounter
typedef enum EdgeZlibDeflateTaskError
{
	kEdgeZlibDeflateTask_ErrorDuringCompression		= 0x80000000,
	kEdgeZlibDeflateTask_NoErrorDuringCompression	= 0x00000000,

	kEdgeZlibDeflateTask_CompressionErrorMask		= 0x80000000,
} EdgeZlibDeflateTaskError;

//////////////////////////////////////////////////////////////////////////

/** 
 * @brief	One entry in the queue which tells the Deflate Task to
 *			compress a single segment of data.
 *
 * @note	The address of the compressed and uncompressed data can have
 *			any alignment.
 *
 */

typedef struct EdgeZlibDeflateQueueElement
{	// align 16, 32 bytes
	uint32_t	m_eaInputUncompressedData;		///< effective address of uncompressed data.
	uint32_t	m_eaOutputCompressedData;		///< effective address where to put compressed data.
	uint32_t	m_uncompressedSize;				///< size of uncompressed data.
	uint32_t	m_maxCompressedOutputSize;		///< max size of output buffer for compressed data.
	uint32_t	m_eaOutputCompressedSize;		///< effective address of the compressed size.
	uint32_t	m_eaWorkToDoCounter;			///< effective address of counter to atomically decrement when decompression done.
												///< If the SPU has an error with the uncompressed data it will set the high bit
												///< of the counter to alert the fact that an error occurred.  This EA may be NULL.
												///< The two least significant bits of this give a flag of type EdgeZlibDeflateTaskProcessing
	uint32_t	m_eaEventFlag;					///< effective address of event flag.  May be NULL.
	uint16_t	m_eventFlagBits;				///< value to set to the event flag.
	uint8_t		m_compressionLevel;				///< compression level (1 - fastest, 9 - smallest)
	uint8_t		m_pad8;
} EdgeZlibDeflateQueueElement __attribute__((aligned(16)));

//////////////////////////////////////////////////////////////////////////

enum
{
	kEdgeZlibDeflateInputBufferMaxSize	= 0x10000,

	// Docs say this should be: (12 + 1.001f * kEdgeZlibDeflateInputBufferMaxSize).
	// However, I have seen increases of 12 + 1.0048f, so I'm not sure what's going on
	//	Therefore I'll make it 12 + 1.01f
	//	I further align it up to 128 bound.  That would result in a value of 0x10300.
	kEdgeZlibDeflateOutputBufferMaxSize	= 0x10300, 
};

//////////////////////////////////////////////////////////////////////////

#endif //__EDGE_ZLIB_DEFLATE_QUEUE_ELEMENT_H__
