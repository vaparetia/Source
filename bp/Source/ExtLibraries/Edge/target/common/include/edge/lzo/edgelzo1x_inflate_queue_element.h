/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_LZO1X_INFLATE_QUEUE_ELEMENT_H__
#define __EDGE_LZO1X_INFLATE_QUEUE_ELEMENT_H__

//////////////////////////////////////////////////////////////////////////

//This gets ORed into the m_eaWorkToDoCounter of the EdgeLzo1xInflateQueueElement
typedef enum EdgeLzo1xInflateTaskProcessing
{
	kEdgeLzo1xInflateTask_Memcpy						= 0,
	kEdgeLzo1xInflateTask_Inflate						= 1,

	kEdgeLzo1xInflateTask_ProcessingMask				= 1,
} EdgeLzo1xInflateTaskProcessing;

//This gets ORed into the workToDoCounter
typedef enum EdgeLzo1xInflateTaskError
{
	kEdgeLzo1xInflateTask_ErrorDuringDecompression		= 0x80000000,
	kEdgeLzo1xInflateTask_NoErrorDuringDecompression	= 0x00000000,

	kEdgeLzo1xInflateTask_DecompressionErrorMask		= 0x80000000,
} EdgeLzo1xInflateTaskError;

//////////////////////////////////////////////////////////////////////////

/** 
 * @brief	One entry in the queue which tells the Inflate Task to
 *			decompress a single segment of data.
 *
 * @note	Compressed data must be not more than 64K
 *
 * @note	Uncompressed data must be not more than 64K
 *
 * @note	The sum of m_outputUncompSkipBeginSize, m_outputUncompPartialBuffSize
 *			and m_outputUncompSkipEndSize gives the expected size that the
 *			compressed data will be decompress to.  If this value is incorrect
 *			then the SPU code will assert.
 */

typedef struct EdgeLzo1xInflateQueueElement
{	// align 16, 32 bytes
	uint32_t	m_eaCompressed;					///< effective address of compressed data.
	uint32_t	m_eaUncompressed;				///< effective address where to put uncompressed data.
	uint32_t	m_compressedSize;				///< size of compressed data.
	uint32_t	m_outputUncompPartialBuffSize;	///< size of uncompressed data to be DMAed out.
	uint32_t	m_eaWorkToDoCounter;			///< effective address of counter to atomically decrement when decompression done.  May be NULL.
												///< LSB is a flag to say if segment was stored compressed (1) or uncompressed (0).
	uint32_t	m_eaEventFlag;					///< effective address of event flag.  May be NULL.
	uint16_t	m_eventFlagBits;				///< value to set to the event flag.
	uint16_t	m_outputUncompSkipBeginSize;	///< Don't output the first N bytes of the uncompressed output
	uint16_t	m_outputUncompSkipEndSize;		///< Don't output the last  N bytes of the uncompressed output.
	uint16_t	m_pad16;
} EdgeLzo1xInflateQueueElement __attribute__((aligned(16)));

//////////////////////////////////////////////////////////////////////////

enum
{
	kEdgeLzo1xInflateOutputBufferMaxSize	= 0x10000,
	kEdgeLzo1xInflateInputBufferMaxSize	= kEdgeLzo1xInflateOutputBufferMaxSize + kEdgeLzo1xInflateOutputBufferMaxSize / 16 + 64 + 3,
};

//////////////////////////////////////////////////////////////////////////

#endif //__EDGE_LZO1X_INFLATE_QUEUE_ELEMENT_H__
