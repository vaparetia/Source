/*
	codecmem.c
	      無線 GV_Malloc() 系を直接的/間接的に使用している処理を切り分ける
	2000/07/27	Y.Kira
	$Id: codecmem.c,v 1.1.1.3 2002/11/19 11:45:07 Yoshizawa1 Exp $
*/
/*
  無線画面モードにおける各種処理で、現状 GV_Malloc() を呼んでいるものを
  ラップし、ソースを切り分けておく。

  ※現状では、通常と変わらない処理を行う。
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include "gameheader.h"
#include "codec.h"

#include "font.h"
#include "cjimaku.h"
#include "libfs.h"
#include "libdg.h"
#include "memlist.h"

#define _codecmem_c_
#include "codecmem.h"

#ifdef DEBUG
#define _DEBUG_
#endif /* DEBUG */

#ifdef _DEBUG_
#define DBG(args...)  printf(args)
#define DB_CALL(func)    func
#else
#define DBG(args...)
#define DB_CALL(func)
#endif /* _DEBUG_ */

#define USE_MEMLIST

static long size_count = 0;
#ifdef USE_MEMLIST
static void * codec_area = NULL;
static MEMLIST codec_memlist;
static int     codec_mem_ready = 0;
static int     cmem_cnt = 0;
#endif /* USE_MEMLIST */

/*
 * 無線用のメモリ領域を準備する
 */
int  codecMemInit(void)
{
  size_count = 0;
#ifdef USE_MEMLIST
  if(NULL == (codec_area = DG_ResizePacketMemory(CODEC_MEM_SIZE)))
    return -1;

  GV_MlBufferInit(&codec_memlist, "codec",
		  codec_area, (char *)codec_area + CODEC_MEM_SIZE, 16);
#endif /* USE_MEMLIST */

  codec_mem_ready = 1;
  cmem_cnt = 0;
  return 0;
}

void codecMemCheck(void)
{
  MEMTAG * tag = codec_area;
  if(!codec_mem_ready) return;
  //ASSERT(tag->size <= CODEC_MEM_SIZE);
  // printf("codecMemCheck(): %d\n", cmem_cnt);
  cmem_cnt++;
}

void codecMemRecov(void)
{
  DBG("codecMemRecov()\n");
  DG_ResizePacketMemory(0);
  codec_mem_ready = 0;
}

/*
 * 使用したメモリサイズを取得し、カウンタをリセットする
 */
long codecMemCountReset(void)
{
  long siz;
  siz = size_count;
  size_count = 0;
  return siz;
}

/*
 * その時点での使用メモリサイズカウンタ値を取得する
 */
long codecMemGetSize(void)
{
  return size_count;
}

/*
 * DG_EVMOBJ を生成する箇所の処理
 */
DG_EVMOBJ * codecMakeEvmObj(EVM_DEF * def, int flag, int chanl)
{
  /* 物体ハンドラのサイズ */
  return DG_MakeEvmObj(def, flag, chanl);
}

/*
 * Actor を生成している箇所の処理
 *
 * Actor の Work を生成するために GV_Malloc() を使用するため、代替処理が必要。
 */
void * codecActorPrio(long _l, size_t _s, int _p)
{
  return GV_NewActorPrio(_l, _s, _p);
}


/*
 * GV_Malloc() の代用品
 */
void * codecMalloc(long size)
{
#ifndef USE_MEMLIST
  size_count += size;
  return GV_Malloc(size);
#else
  char * ptr;

  ASSERT(codec_mem_ready);

  size += 16;
  ptr = GV_MlMalloc(&codec_memlist, size);
  ASSERT(NULL != ptr);
  if(NULL == ptr) return NULL;
  DBG("\ncodecMalloc(%d) = %p\n", size, ptr);
  DB_CALL(GV_MlPrintDebug(&codec_memlist));
  size_count += size;
  return ptr + 16;
#endif /* USE_MEMLIST */
}

/*
 * codecMalloc() で確保したメモリを開放する
 */
void codecFree(void * ptr)
{
#ifndef USE_MEMLIST
  GV_Free(ptr);
#else
  char * c_ptr;
  int siz;

  ASSERT(codec_mem_ready);

  c_ptr = (char *)ptr - 16;
  siz = GV_MlGetSize(c_ptr);
  DBG("\ncodecFree(%p): size = %d\n", c_ptr, siz);
  // *(int*)c_ptr = 0;
  GV_MlFree(&codec_memlist, c_ptr, siz);
  DB_CALL(GV_MlPrintDebug(&codec_memlist));
  size_count -= siz;
#endif /* USE_MEMLIST */
}
