// Mock dc/pvr.h — host syntax-check stub for KallistiOS PowerVR API
#pragma once
#include <stdint.h>

typedef uint32_t uint32;

// List types
#define PVR_LIST_OP_POLY    0
#define PVR_LIST_TR_POLY    1

// Vertex flags
#define PVR_CMD_VERTEX      0x04000000u
#define PVR_CMD_VERTEX_EOL  0x05000000u

typedef struct {
    uint32 flags;
    float x, y, z;
    float u, v;
    uint32 argb;
    uint32 oargb;
} pvr_vertex_t;

typedef struct { uint32 _[8]; } pvr_poly_cxt_t;
typedef struct { uint32 _[8]; } pvr_poly_hdr_t;

#ifdef __cplusplus
extern "C" {
#endif

void pvr_init_defaults(void);
void pvr_shutdown(void);
void pvr_wait_ready(void);
void pvr_scene_begin(void);
void pvr_scene_finish(void);
void pvr_list_begin(int list);
void pvr_list_finish(void);
void pvr_poly_cxt_col(pvr_poly_cxt_t *cxt, int list);
void pvr_poly_compile(pvr_poly_hdr_t *hdr, pvr_poly_cxt_t *cxt);
void pvr_prim(void *data, int size);

#ifdef __cplusplus
}
#endif
