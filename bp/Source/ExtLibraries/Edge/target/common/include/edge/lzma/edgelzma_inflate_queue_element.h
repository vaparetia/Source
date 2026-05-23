/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_LZMA_INFLATE_QUEUE_ELEMENT_H__
#define __EDGE_LZMA_INFLATE_QUEUE_ELEMENT_H__

//////////////////////////////////////////////////////////////////////////

//This gets ORed into the m_eaWorkToDoCounter of the EdgeLzmaInflateQueueElement
typedef enum EdgeLzmaInflateTaskProcessing
{
	kEdgeLzmaInflateTask_Memcpy						= 0,
	kEdgeLzmaInflateTask_Inflate					= 1,

	kEdgeLzmaInflateTask_ProcessingMask				= 1,
} EdgeLzmaInflateTaskProcessing;

//This gets ORed into the workToDoCounter
typedef enum EdgeLzmaInflateTaskError
{
	kEdgeLzmaInflateTask_ErrorDuringDecompression	= 0x80000000,
	kEdgeLzmaInflateTask_NoErrorDuringDecompression	= 0x00000000,

	kEdgeLzmaInflateTask_DecompressionErrorMask		= 0x80000000,
} EdgeLzmaInflateTaskError;

//////////////////////////////////////////////////////////////////////////

/** 
 * @brief	One entry in the queue which tells the Inflate Task to
 *			decompress a single segment of data.
 *
 * @note	Compressed data must be not more than 64K
 *
 * @note	Uncompressed data must be not more than 64K
 *
 * @note	The address of the compressed and uncompressed data can have
 *			any alignment.
 *
 * @note	The sum of m_outputUncompSkipBeginSize, m_outputUncompPartialBuffSize
 *			and m_outputUncompSkipEndSize gives the expected size that the
 *			compressed data will be decompress to.  If this value is incorrect
 *			then the SPU code will assert.
 */

typedef struct EdgeLzmaInflateQueueElement
{	// align 16, 32 bytes
	uint32_t	m_eaCompressed;					///< effective address of compressed data.
	uint32_t	m_eaUncompressed;				///< effective address where to put uncompressed data.
	uint32_t	m_compressedSize;				///< size of compressed data.
	uint32_t	m_outputUncompPartialBuffSize;	///< size of uncompressed data to be DMAed out.
	uint32_t	m_eaWorkToDoCounter;			///< effective address of counter to atomically decrement when decompression done.
												///< If the SPU has an error with the compressed data it will set the high bit
												///< of the counter to alert the fact that an error occurred.  This EA may be NULL.
												///< LSB of this EA is a flag to say if segment was stored compressed (1) or uncompressed (0).
	uint32_t	m_eaEventFlag;					///< effective address of event flag.  May be NULL.
	uint16_t	m_eventFlagBits;				///< value to set to the event flag.
	uint16_t	m_outputUncompSkipBeginSize;	///< Don't output the first N bytes of the uncompressed output
	uint16_t	m_outputUncompSkipEndSize;		///< Don't output the last  N bytes of the uncompressed output.
	uint8_t		m_properties0;					///< Pass in the first byte of the "properties" data
	uint8_t		m_pad8;
} EdgeLzmaInflateQueueElement __attribute__((aligned(16)));

//////////////////////////////////////////////////////////////////////////

enum
{
	kEdgeLzmaInflateInputBufferMaxSize	= 0x10000,
	kEdgeLzmaInflateOutputBufferMaxSize	= 0x10000,
};

//////////////////////////////////////////////////////////////////////////

#endif //__EDGE_LZMA_INFLATE_QUEUE_ELEMENT_H__
