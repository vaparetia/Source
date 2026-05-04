//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	codecmem.c
	      無線 GV_Malloc() 系を直接的/間接的に使用している処理を切り分ける
	2000/07/27	Y.Kira
	$Id: codecmem.c,v 1.3 2002/11/23 12:42:26 Yoshizawa1 Exp $
*/

#ifdef PSX2
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
#endif

#ifdef KP_XBOX
#include <xtl.h>
#endif

#include "gameheader.h"
#include "codec.h"

#include "g_define.h"
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

#undef _DEBUG_

#ifdef PSX2
#ifdef _DEBUG_
#define DBG(...)  printf(__VA_ARGS__)
#define DB_CALL(func)    func
#else
#define DBG(...)
#define DB_CALL(func)
#endif /* _DEBUG_ */
#endif // PSX2
#ifdef KP_XBOX
#define DBG
#define DB_CALL(func)
#endif

#define USE_MEMLIST

typedef struct mem_top_block {
  unsigned int size;
  struct mem_top_block * next;
} mem_top_block;

static long64 size_count = 0;
#ifdef USE_MEMLIST

typedef struct {
  GV_ACT_EX    actor;

  /* 単にガベージコレクタを毎フレーム呼ぶだけのキャラ。 */
} Work;


static Work * now_work = NULL;
static void * codec_area = NULL;
static MEMLIST codec_memlist;
static int     codec_mem_ready = 0;
static int     cmem_cnt = 0;
static mem_top_block * delay_link[2] = { NULL, NULL };

#endif /* USE_MEMLIST */


static void * NewGarbageCollector(void);


/*
 * 無線用のメモリ領域を準備する
 */
int  codecMemInit(void)
{
  size_count = 0;
#ifdef USE_MEMLIST
  if(NULL == (now_work = NewGarbageCollector()))
    {
      DG_ResizePacketMemory(0);
      return -1;
    }
#endif /* USE_MEMLIST */

  codec_mem_ready = 1;
  cmem_cnt = 0;
  return 0;
}

void codecMemCheck(void)
{
  if(!codec_mem_ready) return;
  //ASSERT(tag->size <= CODEC_MEM_SIZE);
  // printf("codecMemCheck(): %d\n", cmem_cnt);
  cmem_cnt++;
}

int codecMemRecov(void)
{
  DBG("codecMemRecov()\n");

  if(!codec_mem_ready) return 0;

  /* 遅延開放リストにまだポインタが残っていたら,非0 を返す */
  if((delay_link[0] != NULL) || (delay_link[1] != NULL)) return -1;
 
  GV_DestroyActor(now_work);
  DG_ResizePacketMemory(0);
  codec_mem_ready = 0;
  now_work = NULL;

  return 0;   /* 0 が返ったら全開放終了 */
}

/*
 * 使用したメモリサイズを取得し、カウンタをリセットする
 */
long64 codecMemCountReset(void)
{
  long64 siz;
  siz = size_count;
  size_count = 0;
  return siz;
}

/*
 * その時点での使用メモリサイズカウンタ値を取得する
 */
long64 codecMemGetSize(void)
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
void * codecActorPrio(long64 _l, size_t _s, int _p)
{
  return GV_NewActorPrio(_l, _s, _p);
}


/*
 * GV_Malloc() の代用品
 */
void * codecMalloc(long64 size)
{
#ifndef USE_MEMLIST
  size_count += size;
  return GV_Malloc(size);
#else
  char * ptr;

  ASSERT(codec_mem_ready);

  printf("codecMalloc size = %ld(0x%x)\n", size, ( int )size);
  size += 16;
  ptr = GV_MlMalloc(&codec_memlist, size);
#ifdef DEBUG_MODE
  if(NULL == ptr)
    {
      int free_area = ( char * )codec_memlist.now_bottom - ( char * )codec_memlist.top;

      /* メモリが足りない場合,どのぐらい足りないかを表示する。 */
      GV_MlPrintDebug(&codec_memlist);
      printf("request size: %08x(%d)\n", size, size);
    }
#endif
  ASSERT(NULL != ptr);
  if(NULL == ptr) return NULL;
  // printf("codecMalloc(%d) = %p\n", size, ptr);
  DB_CALL(GV_MlPrintDebug(&codec_memlist));
  size_count += GV_MlGetSize(ptr);
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
  // printf("codecFree(%p): size = %d\n", c_ptr, siz);
  // *(int*)c_ptr = 0;
  GV_MlFree(&codec_memlist, c_ptr, siz);
  DB_CALL(GV_MlPrintDebug(&codec_memlist));
  size_count -= siz;
#endif /* USE_MEMLIST */
}


void codecDelayedFree(void * ptr)
{
#ifndef USE_MEMLIST
  GV_DelayedFree(ptr);
#else
  mem_top_block * c_ptr;

  ASSERT(codec_mem_ready);

  c_ptr = (mem_top_block *)((char *)ptr - 16);

  /* 遅れ開放リストに追加 */
  c_ptr->next = delay_link[ DG_Clock ];
  delay_link[ DG_Clock ] = c_ptr;

#endif /* USE_MEMLIST */
}

void codecDelayFreeMain(void)
{
  mem_top_block * ptr;
  mem_top_block * next;
  int siz;
#ifndef USE_MEMLIST
#else
  /* まだ codecmem.c の機能を使っていなければ何もしない */
  if(!codec_mem_ready) return;

  /*
   * 1 フレーム前に遅延開放リストに追加されたものを全て開放する
   */
  ptr = delay_link[ 1 - DG_Clock ];
  while(NULL != ptr)
    {
      next = ptr->next;
      siz = ptr->size;
      // printf("codecDelayedFree(%p) = %d\n", ptr, siz);
      GV_MlFree(&codec_memlist, ptr, siz);
      size_count -= siz;
      ptr = next;
    }
  delay_link[ 1 - DG_Clock ] = NULL;  /* 開放したのでリセット */
#endif
}

static void Act(Work * work)
{
  codecDelayFreeMain();
}

static void Die(Work * work)
{
  printf("Finish Garbage Collector.\n");
}

static int GetResources(Work * work)
{
  if(NULL == (codec_area = DG_ResizePacketMemory(CODEC_MEM_SIZE)))
    return -1;

  GV_MlBufferInit(&codec_memlist, "codec",
		  codec_area, (char *)codec_area + CODEC_MEM_SIZE, 16);

  delay_link[0] = delay_link[1] = NULL;

  return 0;
}

static void * NewGarbageCollector(void)
{
  Work * work = codecActorPrio(GV_ACTOR_MANAGER, sizeof(Work), 0xf0);
  if(NULL == work) return NULL;

  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));

  if(GetResources(work))
    {
      GV_DestroyActor(work);
      return NULL;
    }

  return work;
}
