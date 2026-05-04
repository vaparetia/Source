//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  photo_disp.c
  画像転送端末 画像表示

  2001/07/06  Y.Kira
  $Id: photo_disp.c,v 1.1.1.3 2002/11/19 11:45:19 Yoshizawa1 Exp $
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "g_struct.h"
#include "g_define.h"
#include "libdg.h"
#include "font.h"
#include "libfs.h"

#include "sprite_2d.h"
#include "../../kira/2D_action/layout_2d.h"
#define _photo_disp_c_
#include "photo_config.h"
#include "photo_term.h"
#include "photo_pic.h"
#include "photo_itp.h"
#include "photo_panel.h"
#include "photo_puppet.h"
#include "photo_disp.h"
#include "photo_codes.h"

#include "../codec/codecmem.h"

#include "BP_Renderer.h"


#ifdef DEBUG
#define DBG
#else
#define DBG
#endif  /* DEBUG */


typedef struct _photo_disp_Work {
  GV_ACT_EX      actor;

  DG_DMAPACK   * dmapack;

  unsigned int * pict_buf;     /* 画像展開用バッファ               */
  unsigned int * tex_buf;      /* 縮小した画像のバッファ           */

  ALIGN16_PRE PictDraw       ALIGN16_POST pict_draw;    /* 描画用 DMA パケット実体          */

  int            alpha;        /* クロスフェード用アルファ値       */
  int            fade_cnt;     /* フェード時間カウンタ             */
  int            step;         /* 処理進行                         */
  
  int            l2d_handle;   /* 表示されているレイアウトのハンドル */

  float          x0, x1, y0, y1 ;
  void			*packet_mem;
  void          *prim ;
#if 1 // BP_Render
  int photoNum;
#endif
} Work;


enum {
  STEP_FDIN,
  STEP_DISP,
  STEP_FDOUT,
  // STEP_EXIT
};

#define FADE_TIME    100

/*
 * 画像をフェードアウトさせながら消去する
 */
void PictFinishDisp(void * workp)
{
  Work * work = workp;

  if(NULL == work) return;

}


static void Act(Work * work)
{
	work->prim = work->packet_mem ;

	switch(work->step) {
    case STEP_FDIN:
      work->fade_cnt += TIME_BASE;
      if(work->fade_cnt >= FADE_TIME) {
		  work->fade_cnt = FADE_TIME;
		  work->step = STEP_DISP;
	  }
     work->alpha = 128 * work->fade_cnt / FADE_TIME;

	  work->prim = DG_SetDmapackAlpha( work->prim, SCE_GS_SET_ALPHA(0, 1, 2, 1, work->alpha) ) ;
      printf("alpha = %d\n", work->alpha);
      break;
    case STEP_DISP:

	  work->prim = DG_SetDmapackAlpha( work->prim, SCE_GS_SET_ALPHA(0, 1, 2, 1, work->alpha) ) ;
      work->fade_cnt = 0;
      break;
    case STEP_FDOUT:
      work->fade_cnt += TIME_BASE;
      work->alpha = 128 * (FADE_TIME - work->fade_cnt) / FADE_TIME;

      work->prim = DG_SetDmapackAlpha( work->prim, SCE_GS_SET_ALPHA(0, 1, 2, 1, work->alpha) ) ;
      printf("alpha = %d\n", work->alpha);
      if(work->fade_cnt >= FADE_TIME) GV_DestroyActor(work);
      break;
    }

   work->prim = DG_SetDmapackTextureDynamic01( work->prim, NULL, BP_GetRenderTarget_c(kRT_TankerSS0+work->photoNum) );
	work->prim = DG_SetDmapackSprt( work->prim,
								   work->x0, work->y0, 0.0f, 0.0f,
								   work->x1, work->y1, 1.0f, 1.0f,
								   DG_MakeDmaPackColorFromInt(0x80808080) ) ;
	//printf("Need %d packet phototerm/photo_disp.c!!\n" , (int)work->prim - (int)work->packet_mem );

   work->prim = DG_SetDmapackEnd( work->prim ) ;
}

static void Die(Work * work)
{
	/*DMAパケットの開放*/
  if ( work->packet_mem )
	codecDelayedFree( work->packet_mem ) ;

  DG_DequeueDmapack(work->dmapack);
  DG_FreeDmapack(work->dmapack);
  codecDelayedFree(work->tex_buf);
  // Die.
  printf("Photo Picture Actor Bye.\n");
  GV_SetActorFreeFunc(work, GV_DelayedFree);
}


static int ReceiveSignal(void * workp, int signal, int value)
{
  Work * work = workp;
  int ret = 0;

  switch(signal)
    {
    case KILL_PICT:
      work->fade_cnt = 0;
      work->alpha    = 128;
      work->step     = STEP_FDOUT;
      ret = 0;
      break;
    case KILL_FORCE_PICT:
      GV_DestroyActor(work);
      ret = 0;
      break;
    default:
      ret = GV_DefaultSignalFunc(workp, signal, value);
      break;
    }
  return ret;
}

#define ALL_MASK (DG_DMAPACK_INVISIBLE0|DG_DMAPACK_INVISIBLE1| \
                  DG_DMAPACK_INVISIBLE2|DG_DMAPACK_INVISIBLE3| \
                  DG_DMAPACK_INVISIBLEMENU)

static int setup_picture(Work * work, int num)
{
  PIC_PTR * pic;
  DG_DMAPACK * dmapack;

  pic = PIC_GetPicture(num);   /* 指定された画像の圧縮データのポインタを得る */
  printf("XXXXXXXXX: num = %d   pic = %d\n", num, pic);
  if(NULL == pic) return -1;

  /* 展開画像の画像ポインタを描画するパケットを作成する */
  work->photoNum = num;
  if ( !work->packet_mem )
	if ( !(work->packet_mem = codecMalloc( 64 )) ) {
		printf("ERR!! MALLOC phototerm/console.c!!\n");
		return -1 ;
  }
  work->prim = work->packet_mem ;
  DG_SetDmapackEnd( work->prim ) ;

  /*
   * 大きさ指定のオブジェクトがあれば,表示領域の位置とサイズを設定しなおす 
   */
  {
    SPR_OBJ * obj;
    if(NULL != (obj = L2D_GetObject(work->l2d_handle, OBJ_PhotoArea)))
      {
	float x0, y0;
	float x1, y1;
	float home_x, home_y;

	home_x = 0 ;
	home_y = 0 ;

	x0 = obj->box.rect.begin.x;
	y0 = obj->box.rect.begin.y;
	x1 = obj->box.rect.end.x;
	y1 = obj->box.rect.end.y;

	x0 = home_x + x0 * DRAW_WIDTH / SPR_SCRN_WIDTH;
	y0 = home_y + y0 * DRAW_HEIGHT / SPR_SCRN_HEIGHT;
	x1 = home_x + x1 * DRAW_WIDTH / SPR_SCRN_WIDTH;
	y1 = home_y + y1 * DRAW_HEIGHT / SPR_SCRN_HEIGHT;

	work->x0 = x0 ;
	work->x1 = x1 ;
	work->y0 = y0 ;
	work->y1 = y1 ;
      }
  }
  
  /* 転送パケットを DG_DMAPACK 構造体に設定する */
  dmapack = work->dmapack = 
    DG_MakeDmapack2(DG_DMAPACK_MENU | ALL_MASK | DG_DMAPACK_PRIVILEGE,
		    DG_DMAPACK_PHASE_AFTER, 145);
  dmapack->flag &= ~DG_DMAPACK_INVISIBLEMENU;
  dmapack->autopacket = work->packet_mem ;
  DG_QueueDmapack(dmapack);

  return 0;
}


static int GetResources(Work * work, int num, int l2d_handle)
{
  int ret ;

  work->step     = STEP_FDIN; /* フェードインから始める */
  work->alpha    = 0;         /* 開始時は完全な透明     */
  work->fade_cnt = 0;         /* フェードの経過時間は 0 */
  work->l2d_handle = l2d_handle;   /* 表示されている l2d のハンドル */

  printf("---------0\n");

  work->tex_buf  = codecMalloc(PHOTO_TEXSIZE);  /* 1/4 縮小サイズ */
  work->pict_buf = codecMalloc(PHOTO_BUFSIZE);

  GV_SetActorSignalFunc(work, ReceiveSignal);

  printf("---------1\n");

  ret = setup_picture(work, num);
  printf("---------2\n");
  if(NULL != work->pict_buf) codecFree(work->pict_buf);
  printf("---------3\n");
  return ret;
}

void * NewPhotoTermPicture(int num, int l2d_handle)
{
  Work * work;

  work = codecActorPrio(PHOTO_ACT_MODE, sizeof(Work), 0xf0);

  if(NULL == work) return NULL;

  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));

  if(GetResources(work, num, l2d_handle))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  printf("Photo Picture Actor Success.\n");
  return work;
}
