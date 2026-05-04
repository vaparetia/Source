/*
  codeccrt.c
  無線画面の顔表示を「無線らしい」表示にする。

  2000/07/28 Y.Kira
  $Id: codeccrt.c,v 1.1.1.3 2002/11/19 11:45:06 Yoshizawa1 Exp $
*/
/*
  無線画面で表示される人物の顔に、エフェクトをかける。
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
#include "libdg.cnf"
#include "dmapack.h"
#include "def_dma.h"

#define _codeccrt_c_
#include "codeccrt.h"
#include "codecmem.h"

#include "codec_config.h"

#ifndef _DEBUG_
#define DBG(args...)
#else
#define DBG(args...) printf(args)
#endif /* _DEBUG_ */

#define  MAX_LASTERS  (DRAW_HEIGHT / 4)

#define SIZEOF_QWORD(_v)  (sizeof(_v) / sizeof(u_long128))

#define LASTER_ALPHA    110
#define UNSYNC_ALPHA_1  100
#define UNSYNC_ALPHA_2  100
#define UNSYNC_SPD      ((int)(0.5 * 16))

/* 画面効果用 DMA packet 構造体 */
typedef struct {
  DG_DMATAG dmatag;
  struct _gif_ef {
    DG_GIFTAG giftag;
    struct _data_ef {
      /*
       * CRT風の青白い発光
       */
      DG_GSREG  blight_alpha;
      DG_GSREG  blight_prim;

      DG_GSREG  blight_rgbq0;
      DG_GSREG  blight_xyz0;

      DG_GSREG  blight_rgbq1;
      DG_GSREG  blight_xyz1;

      /*
       * 走査線
       */
      DG_GSREG  laster_alpha;
      DG_GSREG  laster_prim;
    } data;
  } gif;
} EfCrtHeader ALIGN16;

typedef struct {
  DG_GSREG  rgbq0;
  DG_GSREG  xyz0;
  DG_GSREG  rgbq1;
  DG_GSREG  xyz1;  /* kick! */
} EfCrtEdge ALIGN16;

typedef struct {
  EfCrtHeader  head;
  EfCrtEdge    edge[0];  
} EfCrtDMA ALIGN16;

typedef struct EfCrtUnSync {
  DG_DMATAG   dmatag;    /* DMAtag */
  DG_GIFTAG   giftag;    /* GIFtag */

  DG_GSREG    alpha;
  DG_GSREG    prim;

  DG_GSREG    rgbq0;
  DG_GSREG    xyz0;

  DG_GSREG    rgbq1;
  DG_GSREG    xyz1;

} EfCrtUnSync;

/* 初期状態 */
static EfCrtDMA def_effect_dma = {
  head:
  {
    dmatag:
    {
      qwc: DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_ef)),
      vifcode:
      {
	SCE_VIF1_SET_NOP(0),
	SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(struct _gif_ef), 0)
      }
    }, /* dmatag */
    
    gif:
    {
      giftag: 
      {
	tag: SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _data_ef), 1, 0, 0, 0, 1),
	regs: GS_REGS_AD
      }, /* giftag */
      data: {
	blight_alpha:{
	  reg: SCE_GS_ALPHA_1,
	  data: SCE_GS_SET_ALPHA(1, 2, 2, 0, 128)
	},
	blight_prim: {
	  reg: SCE_GS_PRIM,
	  data: SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 0, 0, 1, 0, 1, 0, 0)
	},
	blight_rgbq0: {
	  reg: SCE_GS_RGBAQ,
	  data: SCE_GS_SET_RGBAQ(32, 32, 32, 12, 0)
	},
	blight_xyz0: {
	  reg: SCE_GS_XYZ2,
	  data: SCE_GS_SET_XYZ(0, 0, -1)
	},
	blight_rgbq1: {
	  reg: SCE_GS_RGBAQ,
	  data: SCE_GS_SET_RGBAQ(32, 32, 32, 12, 0)
	},
	blight_xyz1: {
	  reg: SCE_GS_XYZ2,
	  data: SCE_GS_SET_XYZ(0, 0, -1)
	},
	laster_alpha:{
	  reg: SCE_GS_ALPHA_1,
	  data: SCE_GS_SET_ALPHA(1, 2, 2, 2, LASTER_ALPHA)
	},
	laster_prim: {
	  reg: SCE_GS_PRIM,
	  data: SCE_GS_SET_PRIM(SCE_GS_PRIM_LINE, 0, 0, 0, 1, 0, 1, 0, 0)
	}
      } /* data */
    } /* gif */
  } /* head */
};


static int chanl_masks[] = {
  ~DG_DMAPACK_INVISIBLE0,
  ~DG_DMAPACK_INVISIBLE1,
  ~DG_DMAPACK_INVISIBLE2,
  ~DG_DMAPACK_INVISIBLE3,
  ~DG_DMAPACK_INVISIBLEMENU
};

#define ALL_MASK  (DG_DMAPACK_INVISIBLE0 | DG_DMAPACK_INVISIBLE1 | \
		   DG_DMAPACK_INVISIBLE2 | DG_DMAPACK_INVISIBLE3 | \
		   DG_DMAPACK_INVISIBLEMENU)

typedef struct _work {
  GV_ACT_EX    actor;
  int          side;   /* エフェクトをかける顔表示窓 */
  int          chanl;  /* 描画チャネル               */
  int          laster; /* 線を引くラスタ数           */
  int          cnt;

  /* CRT 調表示にする領域の指定 */
  int          x, y;
  int          width;
  int          height;

  EfCrtDMA    * packet[2];
  EfCrtUnSync   un_sync[2][2];  /* CRT を撮影した際に写る黒い帯 */

  int unsync_mode; /* 0: 上から下 / 1: 下から上 */
  int unsync_height;
  int unsync_top;
  int unsync_wait_cnt;
  DG_DMAPACK * dmapack;

} Work;


static Work *now_work[2] = {NULL, NULL};

/*
 * 走査線表示
 */
/*
  走査線は、1ラスタおきに黒いラインを描画することで表現
*/
#ifndef SpFixed
#define SpFixed(n)   ((n) * 16)
#endif /* SpFixed */





static void create_unsync(EfCrtUnSync * unsync, int alpha)
{
  /* DMAtag */
  unsync->dmatag.qwc = DMATAG_SET_QWC(DMATAG_ID_RET, 7);
  unsync->dmatag.vifcode[0] = SCE_VIF1_SET_NOP(0);
  unsync->dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT(7, 0);

  /* GIFtag */
  unsync->giftag.tag = SCE_GIF_SET_TAG(6, 1, 0, 0, 0, 1);
  unsync->giftag.regs = GS_REGS_AD;

  /* 描画命令送出 */
  /* ALPHA_1 */
  unsync->alpha.reg = SCE_GS_ALPHA_1;
  unsync->alpha.data = SCE_GS_SET_ALPHA(1, 2, 2, 2, alpha);

  /* PRIM */
  unsync->prim.reg = SCE_GS_PRIM;
  unsync->prim.data = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,
				      0, 0, 0, 1, 0, 0, 0, 0);

  /* RGBAQ */
  unsync->rgbq0.reg = SCE_GS_RGBAQ;
  unsync->rgbq0.data = SCE_GS_SET_RGBAQ(0, 0, 0, 0, 0);

  /* XYZ2 */
  unsync->xyz0.reg = SCE_GS_XYZ2;
  unsync->xyz0.data = SCE_GS_SET_XYZ(0, 0, -1);

  /* RGBAQ */
  unsync->rgbq1.reg = SCE_GS_RGBAQ;
  unsync->rgbq1.data = SCE_GS_SET_RGBAQ(0, 0, 0, 0, 0);

  /* XYZ2 (kick!)*/
  unsync->xyz1.reg = SCE_GS_XYZ2;
  unsync->xyz1.data = SCE_GS_SET_XYZ(0, 0, -1);
}

static void setup_unsync(Work * work, EfCrtUnSync * unsync, int top)
{
  int x_base, y_base;

  x_base = (2048 - work->width / 2);
  y_base = (2048 - work->height / 2);

  unsync->xyz0.data = SCE_GS_SET_XYZ(x_base * 16, y_base * 16 + top, -1);
  unsync->xyz1.data = SCE_GS_SET_XYZ((x_base + work->width) * 16,
				     top + (y_base + work->unsync_height) * 16, -1);
}

static void effect_laster(Work * work)
{
  int left, right, y, y_base, y_pos;
  int i;

  work->cnt = ++(work->cnt) & 3;

  left = (2048 - DG_Chanls[work->chanl].width / 2 + work->x - 1) * 16;
  right = left + (work->width + 1 ) * 16;

  /* y = 0; */
#ifndef HIGHRESO_FFI
  y = ((work->cnt & 2) != 0);
  y_base = 2048 - DG_Chanls[work->chanl].height / 2 + work->y;

  for(i = 0; i < work->laster; i++, y += 2)
    {
      y_pos = (y_base + y) * 16;

      work->packet[DG_Clock]->edge[i].xyz0.data = SCE_GS_SET_XYZ(left, y_pos, -1);
      work->packet[DG_Clock]->edge[i].xyz1.data = SCE_GS_SET_XYZ(right, y_pos, -1);
      /*
	work->packet[1]->edge[i].xyz0.data = SCE_GS_SET_XYZ(left, y_pos, -1);
	work->packet[1]->edge[i].xyz1.data = SCE_GS_SET_XYZ(right, y_pos, -1);
      */
    }
#else
  y = ((work->cnt & 2) != 0) * 2;
  y_base = 2048 - DG_Chanls[work->chanl].height / 2 + work->y;

  for(i = 0; (i + 1) < work->laster; i+=2, y += 4)
    {
      y_pos = (y_base + y) * 16;

      work->packet[DG_Clock]->edge[i].xyz0.data = SCE_GS_SET_XYZ(left, y_pos, -1);
      work->packet[DG_Clock]->edge[i].xyz1.data = SCE_GS_SET_XYZ(right, y_pos, -1);
      work->packet[DG_Clock]->edge[i+1].xyz0.data=SCE_GS_SET_XYZ(left,  y_pos+16, -1);
      work->packet[DG_Clock]->edge[i+1].xyz1.data=SCE_GS_SET_XYZ(right, y_pos+16, -1);
      /*
	work->packet[1]->edge[i].xyz0.data = SCE_GS_SET_XYZ(left, y_pos, -1);
	work->packet[1]->edge[i].xyz1.data = SCE_GS_SET_XYZ(right, y_pos, -1);
	work->packet[1]->edge[i+1].xyz0.data=SCE_GS_SET_XYZ(left,  y_pos+16, -1);
	work->packet[1]->edge[i+1].xyz1.data=SCE_GS_SET_XYZ(right, y_pos+16, -1);
      */
    }

  /* 総ラスタ数が奇数であった場合の処置 */
  if(i < work->laster)
    {
      // DBG("i = %d\n", i);
      y_pos = (y_base + y) * 16;
      work->packet[DG_Clock]->edge[i].xyz0.data = SCE_GS_SET_XYZ(left, y_pos, -1);
      work->packet[DG_Clock]->edge[i].xyz1.data = SCE_GS_SET_XYZ(right, y_pos, -1);
      /*
	work->packet[1]->edge[i].xyz0.data = SCE_GS_SET_XYZ(left, y_pos, -1);
	work->packet[1]->edge[i].xyz1.data = SCE_GS_SET_XYZ(right, y_pos, -1);
      */
    }

#endif /* HIGHRESO_FFI */

}


static int setup_dma_packet(Work * work)
{
  DG_DMAPACK * dmapack;
  unsigned char * packet;
  EfCrtDMA  * packet_ptr[2];
  size_t       siz, dmasiz, gifsiz;
  int i, j, x, y;

  DBG("Setup Sample Object.\n");

  work->dmapack = NULL;
  work->packet[0] = NULL;
  work->packet[1] = NULL;

  DBG("Size Setup.\n");

  siz = sizeof(EfCrtHeader) + sizeof(EfCrtEdge) * work->laster;
  dmasiz = (siz - sizeof(DG_DMATAG)) / sizeof(u_long128);
  gifsiz = dmasiz - 1;

  if(NULL == (packet = codecMalloc(siz * 2))) return -1;

  def_effect_dma.head.dmatag.qwc = DMATAG_SET_QWC(DMATAG_ID_RET, dmasiz);

  def_effect_dma.head.dmatag.vifcode[0] = SCE_VIF1_SET_NOP(0);
  def_effect_dma.head.dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT(dmasiz, 0);
  def_effect_dma.head.gif.giftag.tag = SCE_GIF_SET_TAG(gifsiz, 1, 0, 0, 0, 1);

  x = (2048 - DG_Chanls[work->chanl].width / 2 + work->x) * 16;
  y = (2048 - DG_Chanls[work->chanl].height / 2 + work->y) * 16;
  def_effect_dma.head.gif.data.blight_xyz0.data = SCE_GS_SET_XYZ(x, y, 0);

  x += work->width * 16;
  y += work->height * 16;
  def_effect_dma.head.gif.data.blight_xyz1.data = SCE_GS_SET_XYZ(x, y, 0);

  packet_ptr[0] = (EfCrtDMA *)packet;
  packet_ptr[1] = (EfCrtDMA *)(packet + siz);

  /* パケットの初期値を設定 */
  memcpy(packet_ptr[0], &def_effect_dma, sizeof(EfCrtHeader));
  memcpy(packet_ptr[1], &def_effect_dma, sizeof(EfCrtHeader));

  for(j = 0; j < 2; j++)
    for(i = 0; i < work->laster; i++)
      {
	packet_ptr[j]->edge[i].xyz0.reg = SCE_GS_XYZ2;
	packet_ptr[j]->edge[i].xyz1.reg = SCE_GS_XYZ2;

	packet_ptr[j]->edge[i].rgbq0.reg = SCE_GS_RGBAQ;
	packet_ptr[j]->edge[i].rgbq0.data = SCE_GS_SET_RGBAQ(0, 0, 0, 128, 0);

	packet_ptr[j]->edge[i].rgbq1.reg = SCE_GS_RGBAQ;
	packet_ptr[j]->edge[i].rgbq1.data = SCE_GS_SET_RGBAQ(0, 0, 0, 128, 0);
      }

  if(NULL == (dmapack = DG_MakeDmapack(DG_DMAPACK_NORMAL|DG_DMAPACK_PRIVILEGE,
				       DG_DMAPACK_PHASE_LAST)))
    {
      codecFree(packet);
      return -1;
    }

  dmapack->packet[0] = packet_ptr[0];
  dmapack->packet[1] = packet_ptr[1];

  DBG("packet[0] = %p, packet[1] = %p\n",
      dmapack->packet[0], dmapack->packet[1]);

  DG_QueueDmapack( dmapack );
  DBG("DG_QueueDmapack() is ** COMPLETED!!!!!! ** \n");

  work->dmapack = dmapack;
  work->packet[0] = packet_ptr[0];
  work->packet[1] = packet_ptr[1];
  dmapack->flag = DG_DMAPACK_NORMAL | DG_DMAPACK_PRIVILEGE | ALL_MASK;
  dmapack->flag &= chanl_masks[work->chanl];
  DBG("Sample OBJECT Setup completed.\n");

  effect_laster(work);
 
  return 0;
}

/*
 * 黒い帯の初期化
 */
static void init_black_zone(Work * work)
{
  int i;

#if 0 
  /* 上下方向はランダム */
  work->unsync_mode = (rand() & 4) >> 2;
#else
  work->unsync_mode = 0;
#endif

  /* 幅は領域の最大 2/5 */
  work->unsync_height = work->height / 4 + rand() % (work->height / 3);
  work->unsync_wait_cnt = work->unsync_height / UNSYNC_SPD;

  if(!work->unsync_mode)
    work->unsync_top= -(work->unsync_height + 1);
  else
    work->unsync_top = work->height;
  work->unsync_top *= 16;

  for(i = 0; i < 2; i++)
    {
      create_unsync(&work->un_sync[0][i], UNSYNC_ALPHA_1);
      create_unsync(&work->un_sync[1][i], UNSYNC_ALPHA_2);
    }
}

/*
 * CRT をビデオで撮影した際に映る黒い帯
 */
static void proc_black_zone(Work * work)
{
  int i, qw;

  /* 最初に, 走査線プリミティブの転送を単独で行う設定にする */
  qw = work->packet[DG_Clock]->head.dmatag.qwc & 0xffff;
  work->packet[DG_Clock]->head.dmatag.qwc = DMATAG_SET_QWC(DMATAG_ID_RET, qw);

  if(work->unsync_wait_cnt-- > 0) return;
  work->unsync_wait_cnt = 0;

  if(!work->unsync_mode)
    {
      work->unsync_top    += UNSYNC_SPD;
      if(work->unsync_top > (work->height * 16))
	{
	  work->unsync_wait_cnt = work->unsync_height / UNSYNC_SPD;
	  work->unsync_top = (-work->unsync_height - 1) * 16;
	}
    }
  else
    {
      work->unsync_top    -= UNSYNC_SPD;
      if(work->unsync_top < (work->unsync_height +1) * -16)
	{
	  work->unsync_wait_cnt = work->unsync_height / UNSYNC_SPD;
	  work->unsync_top = work->height * 16;
	}
    }

  /* 描画準備 */
  setup_unsync(work, &work->un_sync[0][DG_Clock], work->unsync_top);
  setup_unsync(work, &work->un_sync[1][DG_Clock], work->unsync_top + 15);


  /* DMA パケットを、走査線描画パケットの後につなぐ */

  qw = work->packet[DG_Clock]->head.dmatag.qwc & 0xffff;
  work->packet[DG_Clock]->head.dmatag.qwc = DMATAG_SET_QWC(DMATAG_ID_NEXT, qw);
  work->packet[DG_Clock]->head.dmatag.addr = &work->un_sync[0][DG_Clock];

  qw = work->un_sync[0][DG_Clock].dmatag.qwc & 0xffff;
  work->un_sync[0][DG_Clock].dmatag.qwc = DMATAG_SET_QWC(DMATAG_ID_NEXT, qw);
  work->un_sync[0][DG_Clock].dmatag.addr = &work->un_sync[1][DG_Clock];
  
  qw = work->un_sync[1][DG_Clock].dmatag.qwc & 0xffff;
  work->un_sync[0][DG_Clock].dmatag.qwc = DMATAG_SET_QWC(DMATAG_ID_RET, qw);
}

static void Act(Work * work)
{
  effect_laster(work);

  /* BlackZone */
  proc_black_zone(work);
}

static void Die(Work * work)
{
  if(work->side >= 0)
    if(work == now_work[work->side]) now_work[work->side] = NULL;
  DG_DequeueDmapack(work->dmapack);
  DG_FreeDmapack(work->dmapack);
  codecFree(work->packet[0]);
  
}

static int GetResources(Work * work, int x, int y, int w, int h)
{
  /* 領域の左上位置の指定 */
  if(x < 0) x = 0;
  if(y < 0) y = 0;

  /* 領域の高さ、幅の指定 */
  if(w < 0) w = DG_Chanls[work->chanl].width - x;
  if(h < 0) h = DG_Chanls[work->chanl].height - y;

  work->x = x;
  work->y = y;
  work->width = w;
  work->height = h;
  work->laster = work->height / 2;
  work->cnt = 0;
  if(setup_dma_packet(work)) return -1;

  init_black_zone(work);
  
  return 0;
}

void * NewCodecEffectCRT(int side, int x, int y, int w, int h)
{
  Work * work;

  OPERATOR();

  if(NULL == (work = codecActorPrio(ACT_MODE, sizeof(Work), 0xf0)))
    return NULL;

  GV_SetActor(&work->actor, Act, Die);
  GV_ActorEX(&work->actor);

  /* 同じサイドに割り当てられた Work があれば、それを開放 */
  if(now_work[side] != NULL)
    GV_DestroyActor(now_work[side]);

  /* 表示サイドの設定 */
  work->side = side;
  work->chanl = side + 2;  /* 顔画面は、チャネル2 とチャネル3 を使用 */

  if(GetResources(work, x, y, w, h))
    {
      GV_DestroyActor(work);
      return NULL;
    }
 
  now_work[side] = work;

  return work;
}

void * NewCodecEffectCRT_SCN(int name, int where)
{
  Work * work;
  int x, y, w, h;
  int chanl, side;
  OPERATOR();

  side = x = y = w = h = -1;
  chanl = 0;

  if(GCL_GetOption('c') != NULL) x = GCL_GetNextInt();
  if(GCL_GetOption('c') != NULL) y = GCL_GetNextInt();
  if(GCL_GetOption('c') != NULL) w = GCL_GetNextInt();
  if(GCL_GetOption('c') != NULL) h = GCL_GetNextInt();

  if(NULL == (work = GV_NewActorPrio(ACT_MODE, sizeof(Work), 0xf0)))
    return NULL;

  GV_SetActor(&work->actor, Act, Die);
  GV_ActorEX(&work->actor);

  /* シナリオ中から呼ばれた場合は、チャネル0, サイド無しとする。*/
  work->side = side;
  work->chanl = chanl;

  if(GetResources(work, x, y, w, h))
    {
      GV_DestroyActor(work);
      DBG("Could not play effect.\n");
      return NULL;
    }

  return work;
}
