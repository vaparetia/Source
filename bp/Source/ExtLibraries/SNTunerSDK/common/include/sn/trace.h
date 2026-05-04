/* SCE CONFIDENTIAL
PLAYSTATION(R)3 Programmer Tool Runtime Library 240.000
* Copyright (C) 2008 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#ifndef __CELL_TRACE_H__
#define __CELL_TRACE_H__

#include <sys/types.h>
#include <cell/error.h>

CDECL_BEGIN

#ifndef SPU

/*
 * Error codes
 */

/* No module is loaded by host tool */
#define CELL_TRACE_ERROR_MODULE_NOT_LOADED CELL_ERROR_CAST(0x80520101)
/* Not registered */
#define CELL_TRACE_ERROR_NOT_REGISTERED    CELL_ERROR_CAST(0x80520102)
/* Resource shortage */
#define CELL_TRACE_ERROR_NO_RESOURCE       CELL_ERROR_CAST(0x80520103)

/*
 * Trace attribute
 */

typedef struct {
	uint32_t moduleId; /* Module ID */
	uint32_t moduleVersion; /* Module software version */
	uint64_t reserved[2]; /* Reserved for future extension */
} CellTraceAttr;

/*
 * Trace mode
 */
typedef enum {
	CELL_TRACE_MODE_STOP = 0,
	CELL_TRACE_MODE_OVERWRITE = 1,
} CellTraceMode;

/*
 * Trace module open parameter
 */
typedef struct {
	CellTraceMode mode; /* Trace mode */
	uint32_t traceBufferSize; /* Trace buffer size */
	uint32_t customParamSize; /* Custom parameter size */
} CellTraceOpenParams;

/*
 * Callback functions
 */
typedef int (*CellTraceOpenCallback)(CellTraceAttr *attr,
									 CellTraceOpenParams *openParam,
									 sys_addr_t traceBuf,
									 sys_addr_t customParamBuf);

typedef int (*CellTraceCloseCallback)(CellTraceAttr *attr);

typedef int (*CellTraceStartCallback)(CellTraceAttr *attr);

typedef int (*CellTraceStopCallback)(CellTraceAttr *attr);

typedef struct {
	CellTraceOpenCallback open;
	CellTraceCloseCallback close;
	CellTraceStartCallback start;
	CellTraceStopCallback stop;
} CellTraceCallbacks;

/* Invalid registered ID */
#define CELL_TRACE_INVALID_REGISTERED_ID   0xffffffff

/* Trace module definitions */
#define CELL_TRACE_MODULE_SPU_MANAGER      0x20001002

/* Prototypes */

int cellTraceIsModuleLoaded(void);
int cellTraceRegisterSpuManager(uint32_t *registeredId, CellTraceCallbacks *callback);
int cellTraceUnregister(uint32_t registeredId);

#endif // !SPU


/* Trace buffer control area */

#define CELL_TRACE_CONTROL_SIZE 128
#define CELL_TRACE_SPU_MANAGER_CONTROL_SIZE CELL_TRACE_CONTROL_SIZE
#define CELL_TRACE_SPU_MANAGER_MAX_NUM_OF_SPU_THREADS 8

typedef struct {
	uint32_t threadId[CELL_TRACE_SPU_MANAGER_MAX_NUM_OF_SPU_THREADS];
	uint32_t count[CELL_TRACE_SPU_MANAGER_MAX_NUM_OF_SPU_THREADS];
	uint32_t reserved1;
	uint32_t numOfTraces;
} CellTraceSpuManagerControl;


/*
 * Trace packet header
 */

/* Packet TAG definitions */
#define CELL_TRACE_TAG_LOAD     0x50
#define CELL_TRACE_TAG_MAP      0x51
#define CELL_TRACE_TAG_DISPATCH 0x52
#define CELL_TRACE_TAG_RESUME   0x53
#define CELL_TRACE_TAG_EXIT     0x54
#define CELL_TRACE_TAG_YIELD    0x55
#define CELL_TRACE_TAG_SLEEP    0x56
#define CELL_TRACE_TAG_USER     0x57
#define CELL_TRACE_TAG_GUID     0x58

/*
 * Trace packet data for SPU manager trace
 */

/* LOAD packet data */
typedef struct {
	uint32_t ea;
	uint16_t lsa;
	uint16_t size;
} CellTraceSpuManagerLoad;

/* MAP packet data */
typedef struct {
	uint32_t eaOffset;
	uint16_t lsa;
	uint16_t size;
} CellTraceSpuManagerMap;

/* START packet data */
typedef struct {
	uint32_t name;
	uint16_t level;
	uint16_t lsa;
} CellTraceSpuManagerStart;

/* STOP packet data */
typedef struct {
	uint64_t guid;
} CellTraceSpuManagerStop;

/* USER packet data */
typedef struct {
	uint64_t data;
} CellTraceSpuManagerUser;

/* GUID packet data */
typedef struct {
	uint64_t guid;
} CellTraceSpuManagerGuid;

/* SPU manager packet data */
typedef union {
	CellTraceSpuManagerLoad load;
	CellTraceSpuManagerMap map;
	CellTraceSpuManagerStart start;
	CellTraceSpuManagerStop stop;
	CellTraceSpuManagerUser user;
	CellTraceSpuManagerGuid guid;
	uint64_t data64;
} CellTraceSpuManagerData;

/* Trace packet header */
typedef struct __attribute__((aligned(8))) CellTraceHeader8 {
	uint8_t tag;    /* packet tag */
	uint8_t length; /* length of payload in word */
	uint8_t cpu;    /* processor ID */
	uint8_t thread; /* thread ID */
	uint32_t time;  /* lower 32bit timebase */
} CellTraceHeader8;

/* SPU manager packet */
typedef struct __attribute__((aligned(16))) {
	CellTraceHeader8 header;
	CellTraceSpuManagerData data;
} CellTraceSpuManagerPacket;

CDECL_END

#endif /*__CELL_TRACE_H__*/
