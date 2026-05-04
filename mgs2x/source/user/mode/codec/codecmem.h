#ifndef _codecmem_h_
#define _codecmem_h_

/*
 * 無線用メモリ領域のサイズ
 */

#ifdef PSX2
#define CODEC_MEM_SIZE  (1024 * 1024 * 3 + 256*1024)
#endif
#ifdef KP_XBOX
#define CODEC_MEM_SIZE  (4*1024*1024)
#endif
#if 1 //BP (need the higher amount as we're running some xbox code now)
#undef CODEC_MEM_SIZE
#define CODEC_MEM_SIZE  (5*1024*1024)
#endif

#ifdef _CODEC_WITHOUT_PAUSE_
#define ACT_MODE GV_ACTOR_AFTER
#else
#define ACT_MODE GV_ACTOR_MANAGER
#endif /* _CODEC_WITHOUT_PAUSE_ */

int         codecMemInit(void);
int         codecMemRecov(void);
int         codecMemClean(void);
long64        codecMemCountReset(void);
long64        codecMemGetSize(void);
void      * codecMalloc(long64 size);
DG_EVMOBJ * codecMakeEvmObj(EVM_DEF * def, int flag, int chanl);
void      * codecActorPrio(long64 _l, size_t _s, int _p);
void        codecFree(void * ptr);
void        codecMemCheck(void);
void        codecDelayedFree(void * ptr);
void        codecDelayFreeMain(void);

extern void *DG_ResizePacketMemory( int size );

#endif /* _codecmem_h_ */
