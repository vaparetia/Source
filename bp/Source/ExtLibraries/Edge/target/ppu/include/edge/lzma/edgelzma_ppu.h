/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_LZMA_PPU_H__
#define __EDGE_LZMA_PPU_H__

#include "edge/edge_stdint.h"

#include "edge/lzma/edgelzma_inflate_queue_element.h"

#ifdef __cplusplus
extern "C" 
{
#endif // __cplusplus

//////////////////////////////////////////////////////////////////////////

typedef void* EdgeLzmaInflateQHandle;
struct CellSpursEventFlag;
struct CellSpurs;
struct CellSpursTaskset;
typedef unsigned CellSpursTaskId;

//////////////////////////////////////////////////////////////////////////

extern CellSpursTaskId edgeLzmaCreateInflateTask(
			CellSpursTaskset*			pTaskSet,
			void*						pTaskContext,
			EdgeLzmaInflateQHandle		handle );

extern uint32_t edgeLzmaGetInflateQueueSize(
			uint32_t					maxNumQueueEntries );

extern EdgeLzmaInflateQHandle edgeLzmaCreateInflateQueue(
			CellSpurs*						pSpurs,
			uint32_t						maxNumQueueEntries,
			void*							pBuffer,
			uint32_t						bufferSize );

extern void edgeLzmaAddInflateQueueElement(
			EdgeLzmaInflateQHandle			handle,
			const unsigned char*			pProperties,
			uint32_t						propertiesSize,
			const void*						pInputCompressedData,
			uint32_t						compressedSize,
			void*							pOutputUncompressed,
			uint32_t						expectedUncompressedSize,
			uint32_t*						pWorkToDoCounter,
			CellSpursEventFlag*				pEventFlag,
			uint16_t						eventFlagBits,
			EdgeLzmaInflateTaskProcessing	processing );

extern bool edgeLzmaTryAddInflateQueueElement(
			EdgeLzmaInflateQHandle			handle,
			const unsigned char*			pProperties,
			uint32_t						propertiesSize,
			const void*						pInputCompressedData,
			uint32_t						compressedSize,
			void*							pOutputUncompressed,
			uint32_t						expectedUncompressedSize,
			uint32_t*						pWorkToDoCounter,
			CellSpursEventFlag*				pEventFlag,
			uint16_t						eventFlagBits,
			EdgeLzmaInflateTaskProcessing	processing );

extern void edgeLzmaAddInflateQueueElementPartialCopyOut(
			EdgeLzmaInflateQHandle			handle,
			const unsigned char*			pProperties,
			uint32_t						propertiesSize,
			const void*						pInputCompressedData,
			uint32_t						compressedSize,
			void*							pOutputUncompPartialBuff,
			uint16_t						outputUncompSkipBeginSize,
			uint32_t						outputUncompPartialBuffSize,
			uint16_t						outputUncompSkipEndSize,
			uint32_t*						pWorkToDoCounter,
			CellSpursEventFlag*				pEventFlag,
			uint16_t						eventFlagBits,
			EdgeLzmaInflateTaskProcessing	processing );

extern bool edgeLzmaTryAddInflateQueueElementPartialCopyOut(
			EdgeLzmaInflateQHandle			handle,
			const unsigned char*			pProperties,
			uint32_t						propertiesSize,
			const void*						pInputCompressedData,
			uint32_t						compressedSize,
			void*							pOutputUncompPartialBuff,
			uint16_t						outputUncompSkipBeginSize,
			uint32_t						outputUncompPartialBuffSize,
			uint16_t						outputUncompSkipEndSize,
			uint32_t*						pWorkToDoCounter,
			CellSpursEventFlag*				pEventFlag,
			uint16_t						eventFlagBits,
			EdgeLzmaInflateTaskProcessing	processing );

extern void edgeLzmaShutdownInflateQueue(
			EdgeLzmaInflateQHandle			handle );

extern uint32_t edgeLzmaGetInflateTaskContextSaveSize( void );

//////////////////////////////////////////////////////////////////////////

#define EDGE_LZMA_GET_INFLATE_QUEUE_SIZE( maxNumQueueEntries )	(128 + (maxNumQueueEntries * 32))
#define EDGE_LZMA_GET_INFLATE_TASK_CONTEXT_SAVE_SIZE()			(1024+2048)

//////////////////////////////////////////////////////////////////////////

//The Inflate Queue must have at least this alignment
#define EDGE_LZMA_INFLATE_QUEUE_ALIGN							(128)
enum {	kEdgeLzmaInflateQueueAlign = 128 };

//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif // __cplusplus

#endif	//__EDGE_LZMA_PPU_H__
