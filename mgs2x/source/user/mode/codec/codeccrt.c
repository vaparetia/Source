//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
  codeccrt.c
  無線画面の顔表示を「無線らしい」表示にする。

  2000/07/28 Y.Kira
  $Id: codeccrt.c,v 1.4 2002/11/23 12:16:41 Yoshizawa1 Exp $
*/
/*
  無線画面で表示される人物の顔に、エフェクトをかける。
*/
#endif

#ifdef PSX2
#define REDEFINEPSX2
#undef PSX2
#endif

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

#include "BP_RenderFX.h"
#include "BP_RendererDebug.h"
#include "BP_RenderBufferTypes.h"

#ifdef KP_XBOX //BP
extern DWORD	DG_BackBufferRendHeight ;
#endif

#ifdef PSX2
#ifndef _DEBUG_
#define DBG(...)
#else
#define DBG(...) printf(__VA_ARGS__)
#endif /* _DEBUG_ */
#endif
#ifdef KP_XBOX
#define DBG
#endif

#define  MAX_LASTERS  (DRAW_HEIGHT / 4)

// #define SIZEOF_QWORD(_v)  (sizeof(_v) / sizeof(u_long128))

#define LASTER_ALPHA    95  /* 110 */
#define UNSYNC_ALPHA_1  100
#define UNSYNC_ALPHA_2  100

#ifdef PSX2
#define UNSYNC_SPD      ((int)(0.5 * 16))
#else
#define UNSYNC_SPD      ((float)(0.5f))
#endif



/* 定義されているときは、白フィルタをかけない */
#define NO_WHITE_FILTER

#ifdef PSX2

/* 画面効果用 DMA packet 構造体 */
typedef ALIGN16_DECL(struct) {
  DG_DMATAG dmatag;
  struct _gif_ef {
    DG_GIFTAG giftag;
    struct _data_ef {
      /*
       * CRT風の青白い発光
       */
#ifndef NO_WHITE_FILTER
      DG_GSREG  blight_test;
      DG_GSREG  blight_alpha;
      DG_GSREG  blight_prim;
      
      DG_GSREG  blight_rgbq0;
      DG_GSREG  blight_xyz0;
      
      DG_GSREG  blight_rgbq1;
      DG_GSREG  blight_xyz1;
#endif /* NO_WHITE_FILTER */
      /*
       * 走査線
       */
      DG_GSREG  laster_alpha;
      DG_GSREG  laster_prim;
    } data;
  } gif;
} EfCrtHeader ;

typedef ALIGN16_DECL(struct) {
  DG_GSREG  rgbq0;
  DG_GSREG  xyz0;
  DG_GSREG  rgbq1;
  DG_GSREG  xyz1;  /* kick! */
} EfCrtEdge ;

typedef ALIGN16_DECL(struct) {
  EfCrtHeader  head;
  EfCrtEdge    edge[0];  
} EfCrtDMA ;

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
#if 1 //BP_GCC

   //head
   {
      //dmatag
      {
         DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_ef)), //qwc
         NULL,
         { SCE_VIF1_SET_NOP(0), SCE_VIF1_SET_DIRECT(SIZEOF_QWORD(struct _gif_ef), 0) } //vifcode
      },
      //gif
      {
         { SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _data_ef), 1, 0, 0, 0, 1), GS_REGS_AD }, //giftag
         //data
         {
            { SCE_GS_SET_ALPHA(1, 2, 2, 2, LASTER_ALPHA), SCE_GS_ALPHA_1 }, //laster_alpha
            { SCE_GS_SET_PRIM(SCE_GS_PRIM_LINE, 0, 0, 0, 1, 0, 1, 0, 0), SCE_GS_PRIM } //laster_prim
         }
      }
   }

#else
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
#ifndef NO_WHITE_FILTER
	blight_alpha:{
	  reg: SCE_GS_ALPHA_1,
	  data: SCE_GS_SET_ALPHA(1, 2, 2, 0, 128)
	},
	blight_test: {
	  reg: SCE_GS_TEST_1,
	  data: SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1)
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
#endif /* NO_WHITE_FILTER */
	laster_alpha:{
	  reg: SCE_GS_ALPHA_1,
	  data: SCE_GS_SET_ALPHA(1, 2, 2, 2, LASTER_ALPHA)
	},
	laster_prim: {
	  reg: SCE_GS_PRIM,
	  data: SCE_GS_SET_PRIM(SCE_GS_PRIM_LINE,
				0, 0, 0, 1, 0, 1, 0, 0)
	}
      } /* data */
    } /* gif */
  } /* head */

#endif //BP_GCC
};
#endif	/* PSX2 */

#if defined(KP_XBOX) || 1
typedef ALIGN16_DECL(struct) {
	int dum;
} EfCrtHeader ;

typedef ALIGN16_DECL(struct) {
	int dum;
} EfCrtEdge ;

typedef ALIGN16_DECL(struct) {
	int dum;
} EfCrtDMA ;

typedef struct EfCrtUnSync {
	int dum;
} EfCrtUnSync;

/* 初期状態 */
static EfCrtDMA def_effect_dma = {
	0
};
#endif


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

typedef struct _codeccrt_work {
  GV_ACT_EX    actor;
  int          side;   /* エフェクトをかける顔表示窓 */
  int          chanl;  /* 描画チャネル               */
  int          laster; /* 線を引くラスタ数           */
  int          cnt;

  /* CRT 調表示にする領域の指定 */
  int          x, y;
  int          width;
  int          height;

#ifdef PSX2
  EfCrtDMA    * packet[2];
#else  //XBOX
  void *packet[1];
  void *xbox_prim ; /* XBOX用dmapackのaddr記憶変数 */
#endif
  EfCrtUnSync   un_sync[2][2];  /* CRT を撮影した際に写る黒い帯 */

  int unsync_mode; /* 0: 上から下 / 1: 下から上 */

#ifdef PSX2
  int unsync_height;
  int   unsync_top;
#else
  float unsync_height;
  float unsync_top;
#endif

  int unsync_wait_cnt;
#if 1
  DG_DMAPACK * dmapack_LasterLines;
#endif
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
#ifdef PSX2
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
#endif
}

static void setup_unsync(Work * work, EfCrtUnSync * unsync, int top)
{
#ifdef _PSX2
  int x_base, y_base;

  x_base = (2048 - work->width / 2);
  y_base = (2048 - work->height / 2);
  unsync->xyz0.data = SCE_GS_SET_XYZ(x_base * 16, y_base * 16 + top, -1);
  unsync->xyz1.data = SCE_GS_SET_XYZ((x_base + work->width) * 16,
				     top + (y_base + work->unsync_height) * 16, -1);
#endif
}

extern int g_CCRTLaster__Blinds_DarkenHeight;
extern int g_CCRTLaster__Blinds_BlankHeight;
static void BP_CCRTLaster_ModeCallback(void* pWork)
{
   int i;

   Work *work = (Work *)pWork;

   SBP_PFX_Blinds* pPacket = (SBP_PFX_Blinds*)BP_RB_Alloc(sizeof(SBP_PFX_Blinds));

   pPacket->alphaData = SCE_GS_SET_ALPHA(1, 2, 2, 2, LASTER_ALPHA);
   pPacket->color = 0x80000000;
   pPacket->startY = ((work->cnt & 2) != 0) ? g_CCRTLaster__Blinds_DarkenHeight : 0;
   pPacket->darkenHeight = g_CCRTLaster__Blinds_DarkenHeight;
   pPacket->blankHeight = g_CCRTLaster__Blinds_BlankHeight;

   BP_RB_AddCommand(kCmd_PostFx_Blinds, (char*)pPacket);
}

static void *_codeccrt_buffer_callback(void *pParam, DG_DMAPACK *pDstDmaPack)
{
   return DG_AS_SceneBufferAllocCopy( pParam, sizeof( Work ) );      
}

static void effect_laster(Work * work)
{
#if 0 //BP_Render (using BP_CCRTLaster_ModeCallback above)
  int left, right, y, y_base, y_pos;
  int i;

  /* 閉じ/開きアクションがあるので,毎フレーム大きさが変わることを想定する */
  work->width = DG_Chanls[work->chanl].width;
  left = (2048 - DG_Chanls[work->chanl].width / 2 + work->x - 1) * 16;
  right = left + (work->width + 1 ) * 16;

  /* y = 0; */
  y = ((work->cnt & 2) != 0);
  y_base = 2048 - DG_Chanls[work->chanl].height / 2 + work->y;
	{
		void *prim;
		unsigned int col = 0x80000000;

		prim = work->dmapack->autopacket ;
		prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(1, 2, 2, 2, LASTER_ALPHA) );
		for(i = 0; i < work->laster; i++, y += 2){
			y_pos = (y_base + y) * 16;	   
			prim = DG_SetDmapackLine( prim, left, y_pos, col, right, y_pos, col );
		}
		//prim = DG_SetDmapackEnd( prim );
		work->xbox_prim = prim;
	}
#else
   work->cnt = ++(work->cnt) & 3;
   work->xbox_prim = work->dmapack->autopacket;
#endif
}


static int setup_dma_packet(Work * work)
{
  DG_DMAPACK * dmapack;
  unsigned char * packet;
  EfCrtDMA  * packet_ptr[2];
  size_t       siz, dmasiz, gifsiz;
  int i, j, x, y;

  //DBG("Setup Sample Object.\n");
#if 1
  {
     if(NULL == (work->dmapack_LasterLines = DG_MakeDmapack(DG_DMAPACK_NORMAL|DG_DMAPACK_PRIVILEGE,
        DG_DMAPACK_PHASE_AFTER)))
     {
        return -1;
     }
     DG_AS_DMAPackSetCallbacks(work->dmapack_LasterLines, BP_CCRTLaster_ModeCallback, _codeccrt_buffer_callback);
     work->dmapack_LasterLines->BP_callbackParam = work;
     DG_QueueDmapack( work->dmapack_LasterLines );
     work->dmapack_LasterLines->flag |= ALL_MASK;
     work->dmapack_LasterLines->flag &= chanl_masks[work->chanl];
  }
#endif

  work->dmapack = NULL;

  if(NULL == (dmapack = DG_MakeDmapack(DG_DMAPACK_NORMAL|DG_DMAPACK_PRIVILEGE,
				       DG_DMAPACK_PHASE_AFTER)))
    {
      return -1;
    }
  
  {
	  int packsize;

	  packsize = sizeof(DG_DMAPACK_ALPHA)*2 
		+ sizeof(DG_DMAPACK_LINE)*work->laster + sizeof(DG_DMAPACK_BOX)
		+ sizeof(DG_DMAPACK_TAG);

	  work->packet[0] = codecMalloc( packsize );
	  if ( !(work->packet[0]) ) {
		  //printf("in 'cdc_noise.c' cannot make memory!\n");
		  ASSERT( 0 );
	  }
	  dmapack->autopacket = work->packet[0];
	  DG_SetDmapackEnd( work->packet[0] );
  }
  DG_QueueDmapack( dmapack );
  
  work->dmapack = dmapack;
  dmapack->flag |= ALL_MASK;
  dmapack->flag &= chanl_masks[work->chanl];

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
  work->unsync_mode = (BP_PS2_rand() & 4) >> 2;
#else
  work->unsync_mode = 0;
#endif

  /* 幅は領域の最大 2/5 */
  work->unsync_height = work->height / 4 + BP_PS2_rand() % (work->height / 3);

  work->unsync_wait_cnt = (int)(work->unsync_height / UNSYNC_SPD);
  if(!work->unsync_mode)
    work->unsync_top= -(work->unsync_height + 1);
  else
    work->unsync_top = work->height;
#ifdef PSX2
  work->unsync_top *= 16;
#endif

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
  int qw;
#ifdef PSX2
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
#else //XBOX
  if(work->unsync_wait_cnt-- > 0){
	  void *prim = work->xbox_prim;
	  prim = DG_SetDmapackEnd( prim ); /* 黒い帯は表示されないので、ここでdmapackは終了 */
	  return;
  }
  work->unsync_wait_cnt = 0;

  if(!work->unsync_mode){
      work->unsync_top    += UNSYNC_SPD;
      if(work->unsync_top > (float)work->height){
		  work->unsync_wait_cnt = (int)(work->unsync_height / UNSYNC_SPD);
		  work->unsync_top = -work->unsync_height - 1;
	  }
  } else {
      work->unsync_top    -= UNSYNC_SPD;
      if(work->unsync_top < -(work->unsync_height +1) ){
		  work->unsync_wait_cnt = (int)(work->unsync_height / UNSYNC_SPD);
		  work->unsync_top = (float)work->height ;
	  }
  }
#endif
  
#ifdef PSX2
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

#else //XBOX
  {
	  void *prim = work->xbox_prim ;
	  unsigned int col = 0x80000000 ;

	  if( DG_Clock & 1 ){
		  prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(1, 2, 2, 2, UNSYNC_ALPHA_1) );
		  prim = DG_SetDmapackBox( prim,
								   0.0f, (float)work->unsync_top,
								   (float)work->width, (float)(work->unsync_top + work->unsync_height),
								   DG_MakeDmaPackColorFromInt(col) );
	  } else {
		  prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(1, 2, 2, 2, UNSYNC_ALPHA_2) );
		  prim = DG_SetDmapackBox( prim,
								   0.0f, (float)work->unsync_top + 0.9375f/* 15.0f/16.0f */,
								   (float)work->width, 
								   (float)work->unsync_top + (float)work->unsync_height + 0.9375f/* 15.0f/16.0f */,
								   DG_MakeDmaPackColorFromInt(col) );
	  }						   
	  prim = DG_SetDmapackEnd( prim );
  }

#endif
}

static void resize_area(Work * work)
{
  int x, y;

#ifndef NO_WHITE_FILTER
  x = (2048 - DG_Chanls[work->chanl].width / 2 + work->x) * 16;
  y = (2048 - DG_Chanls[work->chanl].height / 2 + work->y) * 16;
  work->packet[ DG_Clock ]->head.gif.data.blight_xyz0.data =
    SCE_GS_SET_XYZ(x, y, 0);

  x += DG_Chanls[work->chanl].width * 16;
  y += DG_Chanls[work->chanl].height * 16;
  work->packet[ DG_Clock ]->head.gif.data.blight_xyz1.data =
    SCE_GS_SET_XYZ(x, y, 0);
#endif /* NO_WHITE_FILTER */
}


static void Act(Work * work)
{
  resize_area(work);

  effect_laster(work);

  /* BlackZone */
  proc_black_zone(work);
}

static void Die(Work * work)
{
  if(work->side >= 0)
    if(work == now_work[work->side]) now_work[work->side] = NULL;
#if 1 //BP_Render
   DG_DequeueDmapack(work->dmapack_LasterLines);
#endif
  DG_DequeueDmapack(work->dmapack);
  DG_FreeDmapack(work->dmapack);
  codecDelayedFree(work->packet[0]);
  
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
#ifndef KP_WINDOWS
  work->laster = work->height / 2;
#else
  work->laster = (DWORD)(((float)work->height * (float)DG_BackBufferRendHeight)
				/ (2.0f * (float)DISPLAY_HEIGHT)) ;
#endif
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

#ifdef REDEFINEPSX2
#define PSX2
#undef REDEFINEPSX2
#endif
