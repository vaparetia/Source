#ifndef _codecmem_h_
#define _codecmem_h_

/*
 * 無線用メモリ領域のサイズ
 */
#define CODEC_MEM_SIZE  (1024 * 1024 * 3)

#ifdef _CODEC_WITHOUT_PAUSE_
#define ACT_MODE GV_ACTOR_AFTER
#else
#define ACT_MODE GV_ACTOR_MANAGER
#endif /* _CODEC_WITHOUT_PAUSE_ */

int         codecMemInit(void);
void        codecMemRecov(void);
int         codecMemClean(void);
long        codecMemCountReset(void);
long        codecMemGetSize(void);
void      * codecMalloc(long size);
DG_EVMOBJ * codecMakeEvmObj(EVM_DEF * def, int flag, int chanl);
void      * codecActorPrio(long _l, size_t _s, int _p);
void        codecFree(void * ptr);

#endif /* _codecmem_h_ */
