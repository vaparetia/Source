//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  static_noise.c
  恒常的ノイズ(たまにパルス的に入るノイズとは違い,恒常的にかかっている)

  2001/07/26  Yoshihito Kira
  $Id: static_noise.c,v 1.6 2002/11/25 14:15:28 takaki Exp $
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
#include "c_layout.h"
#include "cdc_face.h"
#include "cdc_noise.h"
#include "codec_signal.h"

#include "BP_RenderFX.h"

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

#define ALL_MASK  (DG_DMAPACK_INVISIBLE0 | DG_DMAPACK_INVISIBLE1 | \
		   DG_DMAPACK_INVISIBLE2 | DG_DMAPACK_INVISIBLE3 | \
		   DG_DMAPACK_INVISIBLEMENU)

#define TEX_BASE_4	(ZBUFFER_PAGE()/64)	/* = Z Buffer */
#define TEX_BASE_16     (TEXTURE_TOP_PAGE()/64)
#define CLUT_BASE	(0x100000/64-3)
#define QSIZ(_type)   ((sizeof(_type) + 15) / 16)

#define FINISH_TIME  30

#define MAX_ALPHA  24
#define SAND_MODE_16
// #undef SAND_MODE_16

extern char _program_top[];

#ifdef PSX2
#ifdef SAND_MODE_16
#define SAND_ADDR  ((void *)(_program_top + 0x001140))  /* 適当に汚れているメモリを指定 */
#define SAND_PSM   SCE_GS_PSMCT16
#define TEX_BASE   TEX_BASE_16
#else
#define SAND_ADDR  ((void *)(_program_top + 0x001140))  /* 適当に汚れているメモリを指定 */
#define SAND_PSM   SCE_GS_PSMT4HH
#define TEX_BASE   TEX_BASE_4
#endif /* SAND_MODE_32 */
#else //KP_XBOX
#ifndef KP_WINDOWS
#ifdef SAND_MODE_16
//#define SAND_ADDR  ((void *)( (int)(_program_top + 0x00008000) & 0x0fffff00 )) /* 128バイト境界 */
#define SAND_ADDR  ((void *)( (int)(0x11000+0x800) & 0x0fffff00 )) /* 128バイト境界 */
#define SAND_PSM   SCE_GS_PSMCT16
#define TEX_BASE   TEX_BASE_16
#else
//#define SAND_ADDR  ((void *)( (int)(_program_top + 0x00008000) & 0x0fffff00 )) /* 128バイト境界 */
#define SAND_ADDR  ((void *)( (int)(0x11000+0x800) & 0x0fffff00 )) /* 128バイト境界 */
#define SAND_PSM   SCE_GS_PSMT4HH
#define TEX_BASE   TEX_BASE_4
#endif /* SAND_MODE_32 */
#else		// Windows
#ifdef SAND_MODE_16
#define SAND_ADDR  ((void *)setup_tex_trans)	 /* 適当に汚れているメモリを指定(...いいのかな？) */
#define SAND_PSM   SCE_GS_PSMCT16
#define TEX_BASE   TEX_BASE_16
#else
#define SAND_ADDR  ((void *)setup_tex_trans)	 /* 適当に汚れているメモリを指定(...いいのかな？) */
#define SAND_PSM   SCE_GS_PSMT4HH
#define TEX_BASE   TEX_BASE_4
#endif /* SAND_MODE_32 */
#endif /* KP_WINDOWS */
#endif

#define	NOISE_TEX_WIDTH		(128)
#define	NOISE_TEX_HEIGHT	(256)

#ifdef KP_XBOX //BP
extern DWORD	DG_BackBufferRendWidth ;	// 描画に使用する範囲
extern DWORD	DG_BackBufferRendHeight ;
#endif

typedef ALIGN16_DECL(struct) {
  DG_DMATAG  dmatag0;
  DG_GIFTAG  giftag0;
  DG_GSREG   gsregs[4];
  DG_GIFTAG  giftag1;
  DG_DMATAG  dmatag1;
} TexTrans ;

typedef ALIGN16_DECL(struct) {

  DG_DMATAG    dmatag;

  ALIGN16_DECL(struct) dma_pack {
    /*
     * REGLIST mode で転送てきないレジスタの内容を PACKED mode で転送
     */    
    DG_GIFTAG    giftag0;  /* prim も一緒に送信 */
    struct gif_pack_1 {
      DG_GSREG   texflush;
      DG_GSREG   texa;
      DG_GSREG   test;
      DG_GSREG   alpha;
    } gif_1;

    /*
     * REGLIST mode で主要な描画部分を転送
     */
    DG_GIFTAG    giftag1;
    ALIGN16_DECL(struct) gif_pack_2 {
     
      u_long64  clamp;
      u_long64  tex0;

      u_long64  prim;
      u_long64  rgbq;
      
      u_long64  uv0;
      u_long64  xyz0;
      
      u_long64  uv1;
      u_long64  xyz1;
      
    } gif_2 ;
  }  dmap;

} SpriteDraw ;

/*
 * 砂嵐転送用
 */
typedef ALIGN16_DECL(struct) {
  TexTrans    trans;    /* テクスチャの転送 */
  SpriteDraw  sprite;   /* スプライト描画   */
} sand_trans ;



typedef struct _static_noise_Work {
  GV_ACT_EX    actor;

  int          side;
  int          chanl;

  int          noise_type;  /* ノイズのタイプ               */
  int          noise_time;  /* ノイズ開始からの経過時間     */
  int          noise_len;   /* ノイズを発生させつづける時間 */
  int          finish_time;

  void       (*act_func)(struct Work * work);
  void       (*die_func)(struct Work * work);
  int          step;

  DG_DMAPACK * dmapack;

  union {
    struct {
      sand_trans  * packet[2];

      int         alpha;
      int         step;      /* 表示ステップ
				0   正位置
				1   左右反転
				2   上下反転
				3   上下左右反転  */
    } sand;
  } params;


  int          finish:1;    /* 非 0 で終了 */

#ifdef KP_XBOX
#if 0
  DG_TEX_LIN *ltex ;
#else
  DG_TEX_LIN noise_tex2 ;  
#endif
#endif

} Work;

#if 1 // BP_RENDER
static ALIGN64_PRE DG_TEX_LIN	StaticRandomNoise_Texture ALIGN64_POST = {
   256,
   256,
   DG_TEXLIN_FORMAT_L8,
   0,
   NULL,
   0,
   0
};
#endif


static Work * now_work[2] = {NULL, NULL };


static void setup_tex_trans(TexTrans * trans,
			    void * tex, int width, int height)
{
  TexTrans * p;
  int w;
  extern void *DG_MakeLoadImagePacket( void *tag_addr, int fmt, int width, int height, 
									   int vram_addr, int vram_width, void *tex_addr );

  w = (width + 63) / 64;
  p = DG_MakeLoadImagePacket(trans,
			     SAND_PSM, width, height,
			     TEX_BASE * 64, w * 64, tex);
  trans->dmatag0.vifcode[0] = SCE_VIF1_SET_FLUSHA(0);
}

/*
 * スプライト転送パケットの初期化
 */
static void setup_sprite(SpriteDraw * spr,
			 u_long64 alpha,
			 u_long64 tex0,
			 u_long64 uv0,
			 u_long64 uv1,
			 u_long64 xyz0,
			 u_long64 xyz1)
{
  spr->dmap.gif_1.alpha.reg  = SCE_GS_ALPHA_1;
  spr->dmap.gif_1.alpha.data = alpha;

  spr->dmap.gif_1.texflush.reg  = SCE_GS_TEXFLUSH;
  spr->dmap.gif_1.texflush.data = 0;

  spr->dmap.gif_1.texa.reg = SCE_GS_TEXA;
  spr->dmap.gif_1.texa.data = SCE_GS_SET_TEXA(128, 0, 128);
  
  spr->dmap.gif_1.test.reg = SCE_GS_TEST_1;
  spr->dmap.gif_1.test.data = SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 1);

  spr->dmap.gif_2.clamp = SCE_GS_SET_CLAMP(0, 0, 0, 0, 0, 0);
  spr->dmap.gif_2.tex0 = tex0;
  spr->dmap.gif_2.prim = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,
					 0, 1, 0, 1, 0, 1, 0, 0);

  spr->dmap.gif_2.rgbq = SCE_GS_SET_RGBAQ(128, 128, 128, 128, 0);
  spr->dmap.gif_2.uv0  = uv0;
  spr->dmap.gif_2.uv1  = uv1;
  spr->dmap.gif_2.xyz0 = xyz0;
  spr->dmap.gif_2.xyz1 = xyz1;


  spr->dmap.giftag0.regs = GS_REGS_AD;
  spr->dmap.giftag0.tag =
    SCE_GIF_SET_TAG(QSIZ(struct gif_pack_1), 0, 0, 0, 0, 1);


  spr->dmap.giftag1.regs = 0x53531068;
  spr->dmap.giftag1.tag =
    SCE_GIF_SET_TAG(1, 1, 0, 0, 1, 8);
 
  /* DMAtag の設定 */
  spr->dmatag.qwc = DMATAG_SET_QWC(DMATAG_ID_RET, QSIZ(struct dma_pack));
  spr->dmatag.vifcode[0] = SCE_VIF1_SET_NOP(0);
  spr->dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT(QSIZ(struct dma_pack), 0);
}

static void setup_sand_trans(Work * work, sand_trans * sand)
{
#if 0 //BP_RENDER
  setup_tex_trans(&(sand->trans), SAND_ADDR, FACE_WIDTH, FACE_HEIGHT);
  setup_sprite(&(sand->sprite),
	       SCE_GS_SET_ALPHA(2, 0, 2, 1, MAX_ALPHA),
	       SCE_GS_SET_TEX0(TEX_BASE, (FACE_WIDTH + 63) / 64,
			       SAND_PSM, 9, 9, 0, 0,
			       0, 0, 0, 0, 2),
	       SCE_GS_SET_UV(0, 0),
	       SCE_GS_SET_UV(FACE_WIDTH * 16, FACE_HEIGHT * 16),
	       SCE_GS_SET_XYZ((2048-FACE_WIDTH)*16,(2048-FACE_HEIGHT)*16, -1),
	       SCE_GS_SET_XYZ((2048+FACE_WIDTH)*16,(2048+FACE_HEIGHT)*16, -1));
#endif
}

/* サンドノイズ初期化 */
static void InitSand(Work * work)
{
  int i;
  unsigned int col;

  work->params.sand.alpha = MAX_ALPHA;

  /*
   * 転送パケット領域を用意
   */
#if 0 // 0 // BP_RENDER
#ifdef PSX2
  for(i = 0; i < 2; i++)
    {
      work->params.sand.packet[i] = codecMalloc(sizeof(sand_trans));
      ASSERT(NULL != work->params.sand.packet[i]);
    }
  
  /* 転送パケットを用意 */
  setup_sand_trans(work, work->params.sand.packet[0]);
  setup_sand_trans(work, work->params.sand.packet[1]);

  /* 転送パケットを DMAPACK に関連付ける */
  work->dmapack->packet[0] = work->params.sand.packet[0];
  work->dmapack->packet[1] = work->params.sand.packet[1];
#else //XBOX
  {
	  int packsize ;

	  packsize = 256 ;
	  //	  packsize = sizeof();
      work->params.sand.packet[0] = codecMalloc( packsize );
      ASSERT(NULL != work->params.sand.packet[0]);
	  
	  work->dmapack->autopacket = work->params.sand.packet[0];

     StaticRandomNoise_Texture.BP_TextureHandle = GetRandomNoiseTexture();
  }
#endif
#else
  {
     int packsize ;

     packsize = 256 ;
     //	  packsize = sizeof();
     work->params.sand.packet[0] = codecMalloc( packsize );
     ASSERT(NULL != work->params.sand.packet[0]);

     work->dmapack->autopacket = work->params.sand.packet[0];

     StaticRandomNoise_Texture.BP_TextureHandle = GetRandomNoiseTexture();
  }
#endif
}

/*
 * サンドノイズの主処理
 *
 * いわゆる夜中の砂嵐(最近は夜中でも砂嵐は見掛けない…)
 */
static void ActSand(Work * work)
{
  int r;
  int u[2], v[2];
  int w, h;
  sand_trans * pack = work->params.sand.packet[ DG_Clock ];

  /* 終了していたら、表示を徐々に薄くする。 */
  work->params.sand.alpha = MAX_ALPHA;
  if(work->finish)
    {
      int left_t = work->finish_time;
      int alpha;
      
      alpha = (MAX_ALPHA * (FINISH_TIME - left_t)) / FINISH_TIME;
      if(alpha < 0) alpha = 0;
      work->params.sand.alpha = alpha;
    }

  /* 直前の表示以外の三種のうちから、ランダムに選択する */
#ifdef PSX2
  r = (BP_PS2_rand() >> 16) % 3;
#else //XBOX
  r = rand() % 3;
#endif
  if(r >= work->params.sand.step) r++;
  work->params.sand.step = r;

#ifdef PSX2
  w = FACE_WIDTH - ((BP_PS2_rand() >> 16) % 32);
  h = FACE_HEIGHT - ((BP_PS2_rand() >> 16) % 32);
#else //XBOX
  w = FACE_WIDTH - (rand() % 32);
  h = FACE_HEIGHT - (rand() % 32);
#endif
  u[0] = 8;
  v[0] = 8;
#ifndef KP_WINDOWS
  u[1] = w * 16 - 8;
  v[1] = h * 16 - 8;
#else
  u[1] = (((w * DG_WinApp.window_width) << 4)/DISPLAY_WIDTH) - 8;
  v[1] = (((h * DG_WinApp.window_height) << 4)/DISPLAY_HEIGHT) - 8;
#endif

  /* 状態にあわせて上下左右の反転をかける */
  if(r & 1) u[1] ^= u[0], u[0] ^= u[1], u[1] ^= u[0];
  if(r & 2) v[1] ^= v[0], v[0] ^= v[1], v[1] ^= v[0];

#if 0 // BP_RENDER
#ifdef PSX2

  pack->sprite.dmap.gif_1.alpha.data =
    SCE_GS_SET_ALPHA(0, 1, 2, 1, work->params.sand.alpha);
  pack->sprite.dmap.gif_2.uv0 = SCE_GS_SET_UV(u[0], v[0]);
  pack->sprite.dmap.gif_2.uv1 = SCE_GS_SET_UV(u[1], v[1]);

#else //XBOX
  {
	  void *prim ;
	  unsigned int col = 0x80808080 ;
	  float w, h;

	  w = (float)(DG_Chanls[work->chanl].width);
	  h = (float)(DG_Chanls[work->chanl].height);
	  
	  prim = work->dmapack->autopacket;
	  prim = DG_SetDmapackTexLin( prim , &work->noise_tex2 );
	  prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(0, 1, 2, 1, work->params.sand.alpha));

	  prim = DG_SetDmapackSprt( prim, w/2.0f-(float)FACE_WIDTH, h/2.0f-(float)FACE_HEIGHT,
							    (u[0]/16.0f)/(float)FACE_WIDTH, (v[0]/16.0f)/(float)FACE_HEIGHT,
							    w/2.0f+(float)FACE_WIDTH, h/2.0f+(float)FACE_HEIGHT,
							    (u[1]/16.0f)/(float)FACE_WIDTH, (v[1]/16.0f)/(float)FACE_HEIGHT, col );

	  prim = DG_SetDmapackEnd( prim );
  }
#endif
#else
  {
     void *prim ;
     unsigned int col = 0x80808080 ;
     float w, h;

     w = (float)(DG_Chanls[work->chanl].width);
     h = (float)(DG_Chanls[work->chanl].height);

     prim = work->dmapack->autopacket;
     prim = DG_SetDmapackTexLin( prim , &StaticRandomNoise_Texture );
     prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(0, 1, 2, 1, work->params.sand.alpha));

     prim = DG_SetDmapackSprt( prim, w/2.0f-(float)FACE_WIDTH, h/2.0f-(float)FACE_HEIGHT,
        (u[0]/16.0f)/(float)FACE_WIDTH, (v[0]/16.0f)/(float)FACE_HEIGHT,
        w/2.0f+(float)FACE_WIDTH, h/2.0f+(float)FACE_HEIGHT,
        (u[1]/16.0f)/(float)FACE_WIDTH, (v[1]/16.0f)/(float)FACE_HEIGHT, DG_MakeDmaPackColorFromInt(col) );

     prim = DG_SetDmapackEnd( prim );
  }
#endif
}

static void DieSand(Work * work)
{
#if 1 //BP_RENDER
   BP_CODEC_RENDER_TODO_BREAK;
#else
  codecDelayedFree(work->params.sand.packet[0]);
#ifdef PSX2
  codecDelayedFree(work->params.sand.packet[1]);
#endif
#endif

#ifdef KP_XBOX
#ifndef KP_WINDOWS
#if 0 //DG_MakeLinerTexture2 に移行したため不要
  DG_FreeLinerTexture( work->ltex );
#else
  DG_FreeLinerTexture2( &work->noise_tex2 );
#endif
#endif //KP_WINDOWS
#endif //KP_XBOX
}

/* ------------------------------------------------------------------------- */

static void set_noise_type(Work * work, int type)
{
  static int chanl_masks[] = {
    DG_DMAPACK_INVISIBLE0,
    DG_DMAPACK_INVISIBLE1,
    DG_DMAPACK_INVISIBLE2,
    DG_DMAPACK_INVISIBLE3,
    DG_DMAPACK_INVISIBLEMENU
  };
  work->dmapack = NULL;
  if((type < 0) || (type >= CDC_NOISE_MAX)) return;

  /* とりあえず転送用の DMAPACK を用意する */
  work->dmapack = DG_MakeDmapack2(DG_DMAPACK_PRIVILEGE |
				  DG_DMAPACK_NORMAL | ALL_MASK,
				  DG_DMAPACK_PHASE_AFTER, 150);
  work->dmapack->flag &= ~chanl_masks[ work->chanl ];

  work->step = 0;
  work->act_func = ActSand;
  work->die_func = DieSand;
  work->noise_type = type;

  InitSand(work);
  DG_QueueDmapack(work->dmapack);
}

static void Act(Work * work)
{
  if(NULL != work->act_func) (work->act_func)(work);

  work->noise_time += TIME_BASE;
  if(work->finish)
    {
      /* 終了所要時間を過ぎていたら自殺 */
      if(work->finish_time >= FINISH_TIME)
	GV_DestroyActor(work);
      else
	work->finish_time += TIME_BASE;
    }
  else
    {
      if(work->noise_len > 0)
	if(work->noise_time > work->noise_len)
	  {
	    work->finish = 1;
	    work->finish_time = work->noise_time - work->noise_len;
	  }
      
    }
}


static void Die(Work * work)
{
  if(work == now_work[work->side]) now_work[work->side] = NULL;

  if(NULL != work->dmapack)
    {
      DG_DequeueDmapack(work->dmapack);
      DG_FreeDmapack(work->dmapack);
    }
  if(NULL != work->die_func) (work->die_func)(work);
}

static int ReceiveSignal(void * workp, int signal, int value)
{
  Work * work = workp;
  int ret = 0;
  switch(signal)
    {
    case CDC_SIGNAL_NOISE_KILL:
      work->finish = 1;
      work->finish_time = 0;
      break;

    default:
      ret = GV_DefaultSignalFunc(workp, signal, value);
      break;
    }
  return ret;
}

static int GetResources(Work * work, int side, int type, int t_len)
{
  work->side = side;
  work->chanl = side + 2;   /* 無線の顔は、左が 2, 右が 3 ch. */
  work->noise_time = 0;
  work->noise_len = t_len;
  work->finish = 0;

  set_noise_type(work, type);  

  GV_SetActorSignalFunc(work, ReceiveSignal);

  now_work[side] = work;
  return 0;
}

void * NewCodecStaticNoise(int side)
{
  Work * work;

  /* 多重起動禁止 */
  if(now_work[side] != NULL) return NULL;

  if(NULL == (work = codecActorPrio(ACT_MODE, sizeof(Work), 0xef)))
    return NULL;

  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));

  printf("Noise Side = %d\n", side);
  if(GetResources(work, side, CDC_NOISE_SAND, 0))
    {
      GV_DestroyActor(work);
      return NULL;
    }

  return work;
}
