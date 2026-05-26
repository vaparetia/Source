// Mock dc/vblank.h — host syntax-check stub for KallistiOS vblank API
#pragma once
#include <stdint.h>

typedef void (*vblank_handler_t)(uint32_t code, void *data);

#ifdef __cplusplus
extern "C" {
#endif

int  vblank_handler_add(vblank_handler_t handler, void *data);
void vblank_handler_remove(int handle);

#ifdef __cplusplus
}
#endif
