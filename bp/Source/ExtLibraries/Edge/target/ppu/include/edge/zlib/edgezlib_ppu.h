/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_ZLIB_PPU_H__
#define __EDGE_ZLIB_PPU_H__

#include "edge/edge_stdint.h"

#include "edge/zlib/edgezlib_inflate_queue_element.h"
#include "edge/zlib/edgezlib_deflate_queue_element.h"

#ifdef __cplusplus
extern "C" 
{
#endif // __cplusplus

//////////////////////////////////////////////////////////////////////////

typedef void* EdgeZlibInflateQHandle;
typedef void* EdgeZlibDeflateQHandle;
struct CellSpursEventFlag;
struct CellSpurs;
struct CellSpursTaskset;
typedef unsigned CellSpursTaskId;

//////////////////////////////////////////////////////////////////////////

extern CellSpursTaskId edgeZlibCreateInflateTask(
			CellSpursTaskset*				pTaskSet,
			void*							pTaskContext,
			EdgeZlibInflateQHandle			handle );

extern uint32_t edgeZlibGetInflateQueueSize(
			uint32_t						maxNumQueueEntries );

extern EdgeZlibInflateQHandle edgeZlibCreateInflateQueue(
			CellSpurs*						pSpurs,
			uint32_t						maxNumQueueEntries,
			void*							pBuffer,
			uint32_t						bufferSize );

extern void edgeZlibAddInflateQueueElement(
			EdgeZlibInflateQHandle			handle,
			const void*						pInputCompressedData,
			uint32_t						compressedSize,
			void*							pOutputUncompressed,
			uint32_t						expectedUncompressedSize,
			uint32_t*						pWorkToDoCounter,
			CellSpursEventFlag*				pEventFlag,
			uint16_t						eventFlagBits,
			EdgeZlibInflateTaskProcessing	processing );

extern bool edgeZlibTryAddInflateQueueElement(
			EdgeZlibInflateQHandle			handle,
			const void*						pInputCompressedData,
			uint32_t						compressedSize,
			void*							pOutputUncompressed,
			uint32_t						expectedUncompressedSize,
			uint32_t*						pWorkToDoCounter,
			CellSpursEventFlag*				pEventFlag,
			uint16_t						eventFlagBits,
			EdgeZlibInflateTaskProcessing	processing );

extern void edgeZlibAddInflateQueueElementPartialCopyOut(
			EdgeZlibInflateQHandle			handle,
			const void*						pInputCompressedData,
			uint32_t						compressedSize,
			void*							pOutputUncompPartialBuff,
			uint16_t						outputUncompSkipBeginSize,
			uint32_t						outputUncompPartialBuffSize,
			uint16_t						outputUncompSkipEndSize,
			uint32_t*						pWorkToDoCounter,
			CellSpursEventFlag*				pEventFlag,
			uint16_t						eventFlagBits,
			EdgeZlibInflateTaskProcessing	processing );

extern bool edgeZlibTryAddInflateQueueElementPartialCopyOut(
			EdgeZlibInflateQHandle			handle,
			const void*						pInputCompressedData,
			uint32_t						compressedSize,
			void*							pOutputUncompPartialBuff,
			uint16_t						outputUncompSkipBeginSize,
			uint32_t						outputUncompPartialBuffSize,
			uint16_t						outputUncompSkipEndSize,
			uint32_t*						pWorkToDoCounter,
			CellSpursEventFlag*				pEventFlag,
			uint16_t						eventFlagBits,
			EdgeZlibInflateTaskProcessing	processing );

extern void edgeZlibShutdownInflateQueue(
			EdgeZlibInflateQHandle	handle );

//"edgeZlibGetTaskContextSaveSize()" is deprecated and will be deleted.
//Please used "edgeZlibGetInflateTaskContextSaveSize()" instead.
extern uint32_t edgeZlibGetTaskContextSaveSize( void );

extern uint32_t edgeZlibGetInflateTaskContextSaveSize( void );

//////////////////////////////////////////////////////////////////////////

extern CellSpursTaskId edgeZlibCreateDeflateTask(
			CellSpursTaskset*				pTaskSet,
			void*							pTaskContext,
			EdgeZlibDeflateQHandle			handle );

extern uint32_t edgeZlibGetDeflateQueueSize(
			uint32_t						maxNumQueueEntries );

extern EdgeZlibDeflateQHandle edgeZlibCreateDeflateQueue(
			CellSpurs*						pSpurs,
			uint32_t						maxNumQueueEntries,
			void*							pBuffer,
			uint32_t						bufferSize );

extern void edgeZlibAddDeflateQueueElement(
			EdgeZlibDeflateQHandle			handle,
			const void*						pInputUncompressedData,
			uint32_t						uncompressedSize,
			void*							pOutputCompressedData,
			uint32_t						maxCompressedOutputSize,
			uint32_t*						pOutputCompressedSize,
			uint32_t*						pWorkToDoCounter,
			CellSpursEventFlag*				pEventFlag,
			uint16_t						eventFlagBits,
			uint32_t						level,
			EdgeZlibDeflateTaskProcessing	processing );

extern bool edgeZlibTryAddDeflateQueueElement(
			EdgeZlibDeflateQHandle			handle,
			const void*						pInputUncompressedData,
			uint32_t						uncompressedSize,
			void*							pOutputCompressedData,
			uint32_t						maxCompressedOutputSize,
			uint32_t*						pOutputCompressedSize,
			uint32_t*						pWorkToDoCounter,
			CellSpursEventFlag*				pEventFlag,
			uint16_t						eventFlagBits,
			uint32_t						level,
			EdgeZlibDeflateTaskProcessing	processing );

extern void edgeZlibShutdownDeflateQueue(
			EdgeZlibDeflateQHandle	handle );

extern uint32_t edgeZlibGetDeflateTaskContextSaveSize( void );

//////////////////////////////////////////////////////////////////////////

#define EDGE_ZLIB_GET_INFLATE_QUEUE_SIZE( maxNumQueueEntries )	(128 + (maxNumQueueEntries * 32))
#define EDGE_ZLIB_GET_DEFLATE_QUEUE_SIZE( maxNumQueueEntries )	(128 + (maxNumQueueEntries * 32))

#define EDGE_ZLIB_GET_INFLATE_TASK_CONTEXT_SAVE_SIZE()			(1024+2048)
#define EDGE_ZLIB_GET_DEFLATE_TASK_CONTEXT_SAVE_SIZE()			(1024+2048)

//////////////////////////////////////////////////////////////////////////

//The Inflate/Deflate Queue must have at least this alignment
#define EDGE_ZLIB_INFLATE_QUEUE_ALIGN							(128)
#define EDGE_ZLIB_DEFLATE_QUEUE_ALIGN							(128)
enum {	kEdgeZlibInflateQueueAlign = 128 };
enum {	kEdgeZlibDeflateQueueAlign = 128 };

//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif // __cplusplus

#endif	//__EDGE_ZLIB_PPU_H__
