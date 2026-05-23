/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_LZO_PPU_H__
#define __EDGE_LZO_PPU_H__

#include "edge/edge_stdint.h"

#include "edge/lzo/edgelzo1x_inflate_queue_element.h"
#include "edge/lzo/edgelzo1x_deflate_queue_element.h"

#ifdef __cplusplus
extern "C" 
{
#endif // __cplusplus

//////////////////////////////////////////////////////////////////////////

typedef void* EdgeLzo1xInflateQHandle;
typedef void* EdgeLzo1xDeflateQHandle;
struct CellSpursEventFlag;
struct CellSpurs;
struct CellSpursTaskset;
typedef unsigned CellSpursTaskId;

//////////////////////////////////////////////////////////////////////////

extern CellSpursTaskId edgeLzo1xCreateInflateTask(
			CellSpursTaskset*				pTaskSet,
			void*							pTaskContext,
			EdgeLzo1xInflateQHandle			handle );

extern uint32_t edgeLzo1xGetInflateQueueSize(
			uint32_t						maxNumQueueEntries );

extern EdgeLzo1xInflateQHandle edgeLzo1xCreateInflateQueue(
			CellSpurs*						pSpurs,
			uint32_t						maxNumQueueEntries,
			void*							pBuffer,
			uint32_t						bufferSize );

extern void edgeLzo1xAddInflateQueueElement(
			EdgeLzo1xInflateQHandle			handle,
			const void*						pInputCompressedData,
			uint32_t						compressedSize,
			void*							pOutputUncompressed,
			uint32_t						expectedUncompressedSize,
			uint32_t*						pWorkToDoCounter,
			CellSpursEventFlag*				pEventFlag,
			uint16_t						eventFlagBits,
			EdgeLzo1xInflateTaskProcessing	processing );

extern bool edgeLzo1xTryAddInflateQueueElement(
			EdgeLzo1xInflateQHandle			handle,
			const void*						pInputCompressedData,
			uint32_t						compressedSize,
			void*							pOutputUncompressed,
			uint32_t						expectedUncompressedSize,
			uint32_t*						pWorkToDoCounter,
			CellSpursEventFlag*				pEventFlag,
			uint16_t						eventFlagBits,
			EdgeLzo1xInflateTaskProcessing	processing );

extern void edgeLzo1xAddInflateQueueElementPartialCopyOut(
			EdgeLzo1xInflateQHandle			handle,
			const void*						pInputCompressedData,
			uint32_t						compressedSize,
			void*							pOutputUncompPartialBuff,
			uint16_t						outputUncompSkipBeginSize,
			uint32_t						outputUncompPartialBuffSize,
			uint16_t						outputUncompSkipEndSize,
			uint32_t*						pWorkToDoCounter,
			CellSpursEventFlag*				pEventFlag,
			uint16_t						eventFlagBits,
			EdgeLzo1xInflateTaskProcessing	processing );

extern bool edgeLzo1xTryAddInflateQueueElementPartialCopyOut(
			EdgeLzo1xInflateQHandle			handle,
			const void*						pInputCompressedData,
			uint32_t						compressedSize,
			void*							pOutputUncompPartialBuff,
			uint16_t						outputUncompSkipBeginSize,
			uint32_t						outputUncompPartialBuffSize,
			uint16_t						outputUncompSkipEndSize,
			uint32_t*						pWorkToDoCounter,
			CellSpursEventFlag*				pEventFlag,
			uint16_t						eventFlagBits,
			EdgeLzo1xInflateTaskProcessing	processing );

extern void edgeLzo1xShutdownInflateQueue(
			EdgeLzo1xInflateQHandle			handle );

extern uint32_t edgeLzo1xGetInflateTaskContextSaveSize( void );

//////////////////////////////////////////////////////////////////////////

extern CellSpursTaskId edgeLzo1xCreateDeflateTask(
			CellSpursTaskset*				pTaskSet,
			void*							pTaskContext,
			EdgeLzo1xDeflateQHandle			handle );

extern uint32_t edgeLzo1xGetDeflateQueueSize(
			uint32_t						maxNumQueueEntries );

extern EdgeLzo1xDeflateQHandle edgeLzo1xCreateDeflateQueue(
			CellSpurs*						pSpurs,
			uint32_t						maxNumQueueEntries,
			void*							pBuffer,
			uint32_t						bufferSize );

extern void edgeLzo1xAddDeflateQueueElement(
			EdgeLzo1xDeflateQHandle			handle,
			const void*						pInputUncompressedData,
			uint32_t						uncompressedSize,
			void*							pOutputCompressed,
			uint32_t						maxCompressedOutputSize,
			uint32_t*						pOutputCompressedSize,
			uint32_t*						pWorkToDoCounter,
			CellSpursEventFlag*				pEventFlag,
			uint16_t						eventFlagBits,
			EdgeLzo1xDeflateTaskProcessing	processing );

extern bool edgeLzo1xTryAddDeflateQueueElement(
			EdgeLzo1xDeflateQHandle			handle,
			const void*						pInputUncompressedData,
			uint32_t						uncompressedSize,
			void*							pOutputCompressed,
			uint32_t						maxCompressedOutputSize,
			uint32_t*						pOutputCompressedSize,
			uint32_t*						pWorkToDoCounter,
			CellSpursEventFlag*				pEventFlag,
			uint16_t						eventFlagBits,
			EdgeLzo1xDeflateTaskProcessing	processing );

extern void edgeLzo1xShutdownDeflateQueue(
			EdgeLzo1xDeflateQHandle			handle );

extern uint32_t edgeLzo1xGetDeflateTaskContextSaveSize( void );

//////////////////////////////////////////////////////////////////////////

#define EDGE_LZO1X_GET_INFLATE_QUEUE_SIZE( maxNumQueueEntries )	(128 + (maxNumQueueEntries * 32))
#define EDGE_LZO1X_GET_DEFLATE_QUEUE_SIZE( maxNumQueueEntries )	(128 + (maxNumQueueEntries * 32))
#define EDGE_LZO1X_GET_INFLATE_TASK_CONTEXT_SAVE_SIZE()			(1024+2048)
#define EDGE_LZO1X_GET_DEFLATE_TASK_CONTEXT_SAVE_SIZE()			(1024+2048)

//////////////////////////////////////////////////////////////////////////

//The Inflate/Deflate Queue must have at least this alignment
#define EDGE_LZO1X_INFLATE_QUEUE_ALIGN							(128)
#define EDGE_LZO1X_DEFLATE_QUEUE_ALIGN							(128)

enum {	kEdgeLzo1xInflateQueueAlign = 128 };
enum {	kEdgeLzo1xDeflateQueueAlign = 128 };

//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif // __cplusplus

#endif	//__EDGE_LZO_PPU_H__
