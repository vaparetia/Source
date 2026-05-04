/*
  c_edge.c
  無線画面における顔表示の水平同期ずれ表現

  $Id: c_edge.c,v 1.1.1.3 2002/11/19 11:45:03 Yoshizawa1 Exp $
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
#include "libmt.h"
#include "libutl.h"
#include "dmapack.h"
#include "def_dma.h"

#define _c_edge_c_
#include "c_edge.h"
#include "codecmem.h"

#include "sprite_2d.h"

#include "codec_config.h"

#include "edge.h"

#ifdef _DEBUG_
#define DBG(args...)  printf(args)
#else
#define DBG(args...)
#endif /* _DEBUG_ */

typedef struct {
  DG_GIFTAG   giftag;
  struct _fs_data {
    DG_GSREG    frame;
  } data;
} FRAME_SET ALIGN16;

typedef struct {
  DG_GIFTAG   giftag;
  struct _sd_data {
    DG_GSREG   alpha;
    DG_GSREG   tex0;
    DG_GSREG   test1;
    DG_GSREG   prim;
    DG_GSREG   rgbq;

    DG_GSREG  uv0;
    DG_GSREG  xyz0;
    DG_GSREG  uv1;
    DG_GSREG  xyz1;

    DG_GSREG  test2;
  } data;
} SCRN_DRAW ALIGN16;


/*
 * 各種フィードバックエフェクトを実現するパケット
 */
typedef struct {
  DG_DMATAG   dmatag;
  struct all_gif {
    /* 初期化パケット */
    struct fb_init {
      DG_GIFTAG   giftag;
      struct fbinit_data {
	DG_GSREG  offset;
	DG_GSREG  clamp;
      } data;
    } init_packet;
    
    /* メイン描画パケット */
    struct _draw_packet {
      FRAME_SET  frame_set0; /* バックバッファをフレームバッファに選択       */
      SCRN_DRAW  scrn_draw0; /* フレームバッファの内容をバックバッファに転送 */
      
      /* yet */
      FRAME_SET  frame_set1; /* フレームバッファをメインに設定               */
      SCRN_DRAW  scrn_draw1; /* バックバッファをフレームバッファに転送       */

      FaceEdge   edge;       /* 顔ふちぼかし用パケット */

    } main_draw;
    
    
    /* 環境の復元 */
    struct fb_recov {
      DG_GIFTAG    giftag;
      struct fbrecov_data {
	DG_GSREG   test;
      } data;
    } recov_packet;
  } gif_packet;

  DG_DMATAG   dmatag_offset;  /* オフセット環境リカバリ */
  DG_DMATAG   dmatag_end;     /* RET tag.               */

} FB_Packet ALIGN16;

/* FB = FeedBack の意 */

typedef struct {
  GV_ACT_EX    actor;

  int          side;            /* 0: 左の顔用  /  1:右の顔用  */

  DG_DMAPACK * dmapack;

  FB_Packet * fb_packet;   /* フィードバック書き込みパケットのポインタ */

  /* 対応描画領域の基準点とサイズを記録しておく領域 */
  int x_base;
  int y_base;

  int u_base;
  int v_base;

  int w, h;

} Work;



#define Qsize(type)   (sizeof(type) / sizeof(u_long128))

/*
 * 効果パケットの設定
 */
static int setup_effect_packet(Work * work, int flip)
{
  FaceEdge * packet;
  int chanl;
  u_long tex0;

  packet = &work->fb_packet[flip].gif_packet.main_draw.edge;
  chanl = work->side + 2;
  tex0 = SCE_GS_SET_TEX0(BUFFER_PAGE(flip) / 64,
			 BUFFER_WIDTH / 64,
			 FRAME_BUFFER_COLOR_MODE(),
			 10, 10, 0, 0,
			 0, 0, 0, 0, 0);

  edgeInitFaceEdgePacket(packet, chanl, tex0,
			 work->x_base, work->y_base,
			 work->u_base, work->v_base,
			 0, 0, work->w - 1, work->h - 1, 0);

  return 0;
}


/*
 * フレームバッファ選択パケットの初期化
 */
static int init_frame_set(FRAME_SET * packet, int page)
{
  static FRAME_SET def_frame_set = {
    giftag:
    {
      tag: SCE_GIF_SET_TAG(Qsize(struct _fs_data), 1, 0, 0, 0, 1),
      regs: GS_REGS_AD
    },
    data:
    {
      frame:
      {
	reg: SCE_GS_FRAME_1,
	data: SCE_GS_SET_FRAME(BUFFER_PAGE(2)/2048,
			       BUFFER_WIDTH/64, SCE_GS_PSMCT32, 0)
      }
    }
  };

  *packet = def_frame_set;
  packet->data.frame.data = 
    SCE_GS_SET_FRAME(BUFFER_PAGE(page)/2048,
		     BUFFER_WIDTH/64, SCE_GS_PSMCT32, 0);
  return 0;
}

/*
 * 描画転送パケットの初期化
 *
 * flag      0x0001  アルファブレンディング禁止(0: ON / 1: OFF)
 *           0x0002  テクスチャマッピング禁止(0: ON / 1: OFF)
 *           0x0004  0:RGB / 1:RGBA
 *
 * test      TEST_1 の値
 * alpha     ALPHA_1 の値
 * tex_page  TEX0_1 に設定するテクスチャベースポイントを得るページ
 * rgba      RGBA 輝度値
 * z         描画 Z 値
 * dir       描画方向
 */
static int init_scrn_draw(SCRN_DRAW * packet, int flag,
			  int test, u_long alpha,
			  int tex_page, int rgba,
			  int z, int dir)
{
  static SCRN_DRAW def_scrn_draw = {
    giftag:
    {
      tag: SCE_GIF_SET_TAG(Qsize(struct _sd_data), 1, 0, 0, 0, 1),
      regs: GS_REGS_AD
    },
    data:
    {
      alpha:
      {
	reg: SCE_GS_ALPHA_1,
	data: SCE_GS_SET_ALPHA(0, 1, 2, 1, 64)
      },
      tex0:
      {
	reg: SCE_GS_TEX0_1,
	data: SCE_GS_SET_TEX0(0, 8, 0, 9, 8, 0, 0, 0, 0, 0, 0, 0)
      },
      test1:
      {
	reg: SCE_GS_TEST_1,
	data: SCE_GS_SET_TEST(1, 7, 64, 1, 0, 0, 1, 1)
      },
      prim:
      {
	reg: SCE_GS_PRIM,
	data: SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 1, 0, 1, 0, 1, 0, 0)
      },
      rgbq:
      {
	reg: SCE_GS_RGBAQ,
	data: SCE_GS_SET_RGBAQ(128, 128, 128, 128, 0)
      },
      uv0:  { reg: SCE_GS_UV },
      xyz0: { reg: SCE_GS_XYZ2 },
      uv1:  { reg: SCE_GS_UV },
      xyz1: { reg: SCE_GS_XYZ2 },

      test2:
      {
	reg: SCE_GS_TEST_1,
	data: SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1)
      }
    }
  };

  *packet = def_scrn_draw;
  
  /* 与えられたパラメタの設定 */
  packet->data.test1.data = test;
  packet->data.alpha.data = alpha;
  packet->data.tex0.data  = SCE_GS_SET_TEX0(BUFFER_PAGE(tex_page) / 64,
					    BUFFER_WIDTH / 64,
					    FRAME_BUFFER_COLOR_MODE(),
					    10, 10, ((flag&4)!=0),
					    0, 0, 0, 0, 0, 0);
  packet->data.prim.data = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,
					   0,
					   ((flag&2) == 0),
					   0,
					   ((flag&1) == 0),
					   0, 1, 0, 0);
  packet->data.rgbq.data = rgba;
  
  {
    int x, y, w, h, u, v, uw, vh;

    if(dir & 1)
      x = (2048 - DRAW_WIDTH / 2) * 16, u = 16,
	w = (DRAW_WIDTH) * 16, uw = (DRAW_WIDTH) * 16;
    else
      x = (2048 + DRAW_WIDTH / 2) * 16 + 16, u = DRAW_WIDTH * 16 + 16,
	w = -(DRAW_WIDTH) * 16, uw = -(DRAW_WIDTH) * 16;

    if(dir & 2)
      y = (2048 - DRAW_HEIGHT / 2) * 16, v = 16,
	h = (DRAW_HEIGHT) * 16, vh = (DRAW_HEIGHT) * 16;
    else
      y = (2048 + DRAW_HEIGHT / 2) * 16 + 16, v = DRAW_HEIGHT * 16 + 16,
	h = -(DRAW_HEIGHT) * 16, vh = -(DRAW_HEIGHT) * 16;

    packet->data.xyz0.data = SCE_GS_SET_XYZ(x, y, z);
    packet->data.xyz1.data = SCE_GS_SET_XYZ(x + w, y + h, z);

    packet->data.uv0.data = SCE_GS_SET_UV(u, v);
    packet->data.uv1.data = SCE_GS_SET_UV(u + uw, v + vh);
  }

  return 0;
}


/*
 * メイン描画パケットの初期化
 */
static int  init_main_packet(Work * work, int flip)
{
  int ret = 0;
  int chanl;
  struct _draw_packet * packet;

  chanl = work->side + 2;
  packet = &work->fb_packet[flip].gif_packet.main_draw;

  ret  = init_frame_set(&packet->frame_set0, 2);
  ret |= init_scrn_draw(&packet->scrn_draw0,
			0x01,
			SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1),
			SCE_GS_SET_ALPHA(0, 1, 2, 1, 64),
			flip,
			SCE_GS_SET_RGBAQ(128, 128, 128, 94, 0),
			0, 0);

  ret |= setup_effect_packet(work, flip);
  ret |= init_frame_set(&packet->frame_set1, flip);
  ret |= init_scrn_draw(&packet->scrn_draw1,
			0x02,
			SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1),
			SCE_GS_SET_ALPHA(2, 2, 2, 1, 0),
			0,
			SCE_GS_SET_RGBAQ(128, 128, 128, 0, 0),
			0, 0);
  return ret;
}

/*
 * 初期化/後始末パケットの初期化
 */
static FB_Packet * init_feedback_packet(Work * work, int flip)
{
  int siz;
  int chanl;
  FB_Packet * packet;

  chanl = work->side + 2;
  packet = &work->fb_packet[flip];


  siz = Qsize(struct all_gif);
  /* 描画パケットの tag */
  packet->dmatag.qwc = DMATAG_SET_QWC(DMATAG_ID_CNT, siz);
  packet->dmatag.vifcode[0] = SCE_VIF1_SET_NOP(0);
  packet->dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT(siz, 0);

  /* 描画環境復帰パケットの tag */
  siz = Qsize(DG_DRAWOFFSET);
  packet->dmatag_offset.qwc = DMATAG_SET_QWC(DMATAG_ID_REF, siz);
  packet->dmatag_offset.addr = &DG_Chanls[chanl].draw_offset[flip];
  packet->dmatag_offset.vifcode[0] = SCE_VIF1_SET_NOP(0);
  packet->dmatag_offset.vifcode[1] = SCE_VIF1_SET_DIRECT(siz, 0);

  /* リターン tag */
  packet->dmatag_end.qwc = DMATAG_SET_QWC(DMATAG_ID_RET, 0);
  packet->dmatag_end.vifcode[0] = SCE_VIF1_SET_NOP(0);
  packet->dmatag_end.vifcode[1] = SCE_VIF1_SET_NOP(0);

  /* 初期化パケットの初期化(ややこしい) */
  packet->gif_packet.init_packet.giftag.tag =
    SCE_GIF_SET_TAG(Qsize(struct fbinit_data), 1, 0, 0, 0, 1);
  packet->gif_packet.init_packet.giftag.regs= GS_REGS_AD;
  packet->gif_packet.init_packet.data.offset.reg = SCE_GS_XYOFFSET_1;
  packet->gif_packet.init_packet.data.offset.data =
    SCE_GS_SET_XYOFFSET(((2048 - DRAW_WIDTH / 2 - 0) << 4),
			((2048 - DRAW_HEIGHT / 2 - 0) << 4));
  packet->gif_packet.init_packet.data.clamp.reg = SCE_GS_CLAMP_1;
  packet->gif_packet.init_packet.data.clamp.data =
    SCE_GS_SET_CLAMP(2, 2, 1, DRAW_WIDTH - 1, 1, DRAW_HEIGHT - 2);

  /* 環境復帰パケットの初期化 */
  packet->gif_packet.recov_packet.giftag.tag =
    SCE_GIF_SET_TAG(Qsize(struct fbrecov_data), 1, 0, 0, 0, 1);
  packet->gif_packet.recov_packet.giftag.regs = GS_REGS_AD;
  packet->gif_packet.recov_packet.data.test.reg = SCE_GS_TEST_1;
  packet->gif_packet.recov_packet.data.test.reg =
    SCE_GS_SET_TEST(1, 7, 0, 0, 0, 0, 1, 1);

  /* メイン描画パケットの初期化 */
  init_main_packet(work, flip);

  return packet;
}

static FB_Packet * setup_feedback_packet(Work * work)
{
  FB_Packet * packet;

  if(NULL == (packet = codecMalloc(sizeof(FB_Packet) * 2))) return NULL;

  work->fb_packet = packet;

  init_feedback_packet(work, 0);
  init_feedback_packet(work, 1);

  return packet;
}


static void calc_base(Work * work)
{
  int x, y;

  /* 描画領域のサイズ取得 */
  work->w = DG_Chanls[work->side + 2].width;
  work->h = DG_Chanls[work->side + 2].height;

  /* 描画領域左上の点が、画面中央からどの位置にあるかを取得する */
  x = DG_Chanls[work->side + 2].offset_x - work->w / 2;
  y = DG_Chanls[work->side + 2].offset_y - work->h / 2;

  /* 描画領域の XY 基準位置取得 */
  work->x_base = 2048 + x;
  work->y_base = 2048 + y;

  /* 描画領域の UV 基準位置取得 */
  work->u_base = DRAW_WIDTH  / 2 + x;
  work->v_base = DRAW_HEIGHT / 2 + y;
}

static void Act(Work * work)
{
  /* 画面の加工に必要な各描画基準点を算出する */
  calc_base(work);
  setup_effect_packet(work, DG_Clock);
}

static void Die(Work * work)
{
  DG_DequeueDmapack(work->dmapack);
  DG_FreeDmapack(work->dmapack);
  codecFree(work->fb_packet);
}

static int GetResources(Work * work, int side)
{
  DG_DMAPACK * dmapack;
  int flags;

  work->side = side;

  if(NULL == (work->fb_packet = setup_feedback_packet(work)))
    return -1;

  flags = (DG_DMAPACK_INVISIBLE  | DG_DMAPACK_INVISIBLE2 |
	   DG_DMAPACK_INVISIBLE3 | DG_DMAPACK_INVISIBLEMENU |
	   DG_DMAPACK_NORMAL     | DG_DMAPACK_PRIVILEGE);

  if(!side)
    flags &= ~DG_DMAPACK_INVISIBLE2;
  else
    flags &= ~DG_DMAPACK_INVISIBLE3;

  if(NULL == (dmapack = DG_MakeDmapack2(flags, DG_DMAPACK_PHASE_LAST, 130)))
    {
      codecFree(work->fb_packet);
      return -1;
    }
  dmapack->flag = flags;
  dmapack->packet[0] = &work->fb_packet[0];
  dmapack->packet[1] = &work->fb_packet[1];
  work->dmapack = dmapack;
  DG_QueueDmapack(dmapack);
  return 0;
}

void * NewEffectCodecFaceEdgeP(int side)
{
  Work * work;

  if(NULL == (work = codecActorPrio(ACT_MODE, sizeof(Work), 0xf0)))
    return NULL;

  GV_SetActor(&work->actor, Act, Die);
  GV_ActorEX(&work->actor);
  work->side = side;

  if(GetResources(work, side))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  
  return work;
}
