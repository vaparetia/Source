/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_LZO1X_DEFLATE_QUEUE_ELEMENT_H__
#define __EDGE_LZO1X_DEFLATE_QUEUE_ELEMENT_H__

//////////////////////////////////////////////////////////////////////////

// Options to choose what output from compression to store
// This gets ORed into the m_eaWorkToDoCounter of the EdgeLzo1xDeflateQueueElement
typedef enum EdgeLzo1xDeflateTaskProcessing
{
	kEdgeLzo1xDeflateTask_DeflateStoreCompressed			= 0,
	kEdgeLzo1xDeflateTask_DeflateStoreSmallest				= 1,

	kEdgeLzo1xDeflateTask_DeflateStoreWhatMask				= 0x1,
} EdgeLzo1xDeflateTaskProcessing;

// Tell user whether the uncompressed or compressed data was stored
// This gets ORed into the returned size
typedef enum EdgeLzo1xDeflateTaskProcessingStored
{
	kEdgeLzo1xDeflateTask_UncompressedWasStored				= 0,
	kEdgeLzo1xDeflateTask_CompressedWasStored				= 0x80000000,

	kEdgeLzo1xDeflateTask_HowStoredMask						= 0x80000000,
} EdgeLzo1xDeflateTaskProcessingStored;

//This gets ORed into the workToDoCounter
typedef enum EdgeLzo1xDeflateTaskError
{
	kEdgeLzo1xDeflateTask_ErrorDuringCompression			= 0x80000000,
	kEdgeLzo1xDeflateTask_NoErrorDuringCompression			= 0x00000000,

	kEdgeLzo1xDeflateTask_CompressionErrorMask				= 0x80000000,
} EdgeLzo1xDeflateTaskError;

//////////////////////////////////////////////////////////////////////////

/** 
 * @brief	One entry in the queue which tells the Deflate Task to
 *			compress a single segment of data.
 *
 * @note	Uncompressed data must be not more than 64K
 *
 * @note	Compressed data must be not more than
 *			uncomprSize + (uncomprSize/16) + 64 + 3.
 *
 * @note	Warning: The low 16 bits of the address of the input
 *			uncompressed data is used by lzo1x in deciding how to encode
 *			the data, and changing these low 16 bits will thus result in
 *			differently encoded (but completely correct) compressed data.
 *			Therefore the same uncompressed file may compress to different
 *			compressed data than a previous time, but both compressed data
 *			sets will decompress to the same uncompressed data.
 */

typedef struct EdgeLzo1xDeflateQueueElement
{	// align 16, 32 bytes
	uint32_t	m_eaInputUncompressedData;		///< effective address of uncompressed data.
	uint32_t	m_eaOutputCompressedData;		///< effective address where to put compressed data.
	uint32_t	m_uncompressedSize;				///< size of uncompressed data.
	uint32_t	m_maxCompressedOutputSize;		///< max size of output buffer for compressed data.
	uint32_t	m_eaOutputCompressedSize;		///< effective address where to store uint32_t: (size of output data, hi bit = 1 if compressed)
	uint32_t	m_eaWorkToDoCounter;			///< effective address of counter to atomically decrement when decompression done.  May be NULL.
												///< Low bit is EdgeLzo1xDeflateTaskProcessing (independent of upper bits)
	uint32_t	m_eaEventFlag;					///< effective address of event flag.  May be NULL.
	uint16_t	m_eventFlagBits;				///< value to set to the event flag.
	uint16_t	m_pad16;
} EdgeLzo1xDeflateQueueElement __attribute__((aligned(16)));

//////////////////////////////////////////////////////////////////////////

enum
{
	kEdgeLzo1xDeflateInputBufferMaxSize	= 0x10000,
	kEdgeLzo1xDeflateOutputBufferMaxSize	= kEdgeLzo1xDeflateInputBufferMaxSize + kEdgeLzo1xDeflateInputBufferMaxSize/16 + 64 + 3,
};

//////////////////////////////////////////////////////////////////////////

#endif //__EDGE_LZO1X_DEFLATE_QUEUE_ELEMENT_H__
