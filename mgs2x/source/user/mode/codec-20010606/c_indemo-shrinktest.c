/*
  c_indemo.c
  無線モードに出入りする際のデモンストレーション
  ゲーム中の画面を背景に加工する。

  $Id: c_indemo-shrinktest.c,v 1.1.1.3 2002/11/19 11:45:04 Yoshizawa1 Exp $
*/
/*
  無線モードに入る際に、ゲーム中の実画面を加工、拡大し、
  無線モードにおける背景にする。

  また、無線モードを終える時に、背景としていた画面をゲーム中の画面に
  再びつなぐ演出をする。
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
#include "g_struct.h"
#include "codec.h"
#include "player.h"

#include "font.h"
#include "cjimaku.h"
#include "libfs.h"
#include "libdg.h"
#include "libmt.h"
#include "libutl.h"
#include "fmt_demo.h"
#include "libdg.cnf"
#include "dmapack.h"
#include "def_dma.h"
#include "sprite_2d.h"

#define _c_indemo_c_
#include "c_indemo.h"
#include "codecmem.h"

#include "codec_config.h"

#include "3d_util.h"

void *DG_MakeLoadImagePacket( void *tag_addr, int fmt, int width, int height, int vram_addr, int vram_width, void *tex_addr );

#ifndef _DEBUG_
#define DBG(args...)
#else
#define DBG(args...)   printf(args)
#endif /* _DEBUG_ */

#define PRI_Z   0
#define BG_CHANL 4

#ifndef PAL
#define TIMEBASE 5   /* NTSC */
#else
#define TIMEBASE 6   /* PAL  */
#endif /* PAL */


#define TEX_16BPP     /* テクスチャをRGBA16 として格納するときに定義 */
/*
#define MONO_R   40
#define MONO_G   100
#define MONO_B   100
#define MONO_BASE 100
*/
#define MONO_R   128
#define MONO_G   128
#define MONO_B   128
#define MONO_BASE 100
/*
#define MONO_R   0
#define MONO_G   60
#define MONO_B   55
#define MONO_BASE 100
*/
typedef enum {
  CodecDemo_Zoom,         /* 背景領域拡大中         */
  CodecDemo_Fix,          /* 背景定着               */
  CodecDemo_Wide,         /* 背景縮小(復帰)中       */
  CodecDemo_Recover       /* 背景復帰完了           */                  
} CodecDemoStep;

typedef struct {
  DG_DMATAG dmatag0;
  DG_GIFTAG giftag0;
  DG_GSREG  gsregs[4];
  DG_GIFTAG giftag1;
  DG_DMATAG dmatag1;
#if 0
  DG_DMATAG dmatag2;
  DG_GIFTAG giftag2;
  DG_GSREG  texflush;
#endif
} tex_packet ALIGN16;

/*
 * ゲーム画面を加工した結果のテクスチャを背景として転送する DMA パケット構造体
 */
typedef struct {
  /* スクリーンショットを加工したものを、ここで転送 */
  tex_packet tex_trans;

  /* 背景の転送 */
  DG_DMATAG  dmatag;
  struct _gif {
    DG_GIFTAG giftag;
    struct _data {

      /* depth test を行なう(z が 0 でない箇所は抜く) */
      DG_GSREG  test0;

      /*
       * 背景としてのプリミティブ描画
       */
      DG_GSREG  alpha;
      DG_GSREG  clamp;

      /* テクスチャ環境: 取得したスクリーンショットを使用する */
      DG_GSREG  texflush;		/* added by K.Takabe 2000.11.28 */
      DG_GSREG  tex0;
      /* DG_GSREG  tex1; */
      DG_GSREG  prim;
      DG_GSREG  uv0;
      DG_GSREG  rgbq0;
      DG_GSREG  xyz0;
      DG_GSREG  uv1;
      DG_GSREG  rgbq1;
      DG_GSREG  xyz1;

      /* depth test 環境を元に戻す */
      DG_GSREG  test1;
    } data;
  } gif;
} codecBG_DMA ALIGN16;

typedef struct {
  GV_ACT_EX       actor;

  CodecDemoMode   mode;   /* モード
			     (CODEC_DEMO_IN:無線IN / CODEC_DEMO_OUT:無線OUT) */

  CodecDemoStep   step;

  void          * scrn;   /* スクリーンショット取得時に使用する
			     バッファへのポインタ */

  void          * org_shot; /* オリジナルのスクリーンショットへのポインタ */

  sceGsStoreImage sp;

  /* 背景画像転送用 DMA パケット */
  codecBG_DMA   * packet;
  DG_DMAPACK    * dmapack;

  /* 背景としてアップにする領域(縮小後のテクスチャデータ上における uv値) */
  float           x[2], y[2];

  /* 表示中の領域 */
  float           u[2], v[2];

  /* フレーム数のカウンタ */
  int             f_cnt;

  /* フェードアウトモードの場合は 1 になる */
  int             fade;

} Work;



static Work * now_demo = NULL;


static void scrn_shrink(Work * work, void * tmp)
{
  unsigned char *src, *dst;
  unsigned char *e, *o, *p;
  int x, y;
  int r, g, b, a;

  src = tmp;
  dst = tmp; /* 縮小画像で、テンポラリバッファをオーバーライドする */

  /*
   * ピクセル数にして、縦横それぞれ半分ずつの縮小なので、
   * 真面目に縮小アルゴリズムを用いたりはしない。
   */

  /* 真面目に RGB 値の補間を行う */
  for(y = 0; y < DRAW_HEIGHT / 2; y++)
    {
      e = src;
      o = src + 4 * DRAW_WIDTH;
      p = src + 8 * DRAW_WIDTH;
      for(x = 0; x < DRAW_WIDTH / 2; x++)
#if 1
	{
	  r  = *e;       g  = *(e + 1); b  = *(e + 2); a  = *(e + 3);
	  r += *(e + 4); g += *(e + 5); b += *(e + 6); a += *(e + 7);
	  r += *(e + 8); g += *(e + 9); b += *(e + 10); a += *(e + 11);

	  r += *o;       g += *(o + 1); b += *(o + 2); a += *(o + 3);
	  r += *(o + 4); g += *(o + 5); b += *(o + 6); a += *(o + 7);
	  r += *(o + 8); g += *(o + 9); b += *(o + 10); a += *(o + 11);

	  r += *p;       g += *(p + 1); b += *(p + 2); a += *(p + 3);
	  r += *(p + 4); g += *(p + 5); b += *(p + 6); a += *(p + 7);
	  r += *(p + 8); g += *(p + 9); b += *(p + 10); a += *(p + 11);

	  r /= 9, g /= 9, b /= 9, a /= 9;
	  *dst++ = r;
	  *dst++ = g;
	  *dst++ = b;
	  *dst++ = a;
	  e += 12;
	  o += 12;
	  p += 12;
	}
      src += DRAW_WIDTH * 12;
#else
      {
	  r  = *e;       g  = *(e + 1); b  = *(e + 2); a  = *(e + 3);
	  r += *(e + 4); g += *(e + 5); b += *(e + 6); a += *(e + 7);

	  r += *o;       g += *(o + 1); b += *(o + 2); a += *(o + 3);
	  r += *(o + 4); g += *(o + 5); b += *(o + 6); a += *(o + 7);

	  r /= 4, g /= 4, b /= 4, a /= 4;
	  *dst++ = r;
	  *dst++ = g;
	  *dst++ = b;
	  *dst++ = a;
	  e += 8;
	  o += 8;
	}
      src += DRAW_WIDTH * 8;
#endif
    }
}

/* グレースケール化:
   16階調のグレースケールにする。これにより1pixel を4bit で扱うことができる。
*/
static int scrn_grayscale(Work * work, unsigned char * tmp)
{
  unsigned char * src;
  int x, y;
  int even, max;
  int rate;
  int w;

#ifndef TEX_16BPP
  /*
   * 32bpp(RGBA32) の場合 
   */
  unsigned char * dst;

  dst = work->scrn; /* 16階調グレースケールが格納されるバッファのポインタ */
  src = tmp;        /* オーバーライドで縮小された画像のポインタ           */

  max = 0;
  rate = 1000;
  w = DRAW_WIDTH / 2;

  for(y = 0; y < DRAW_HEIGHT / 2; y++)
    for(x = 0; x < w; x++)
      {
	/* 偶数ピクセルの処理 */
	even  = *(src++) * 299;
	even += *(src++) * 587;
	even += *(src++) * 114;
	src++;
	even /= rate;

	*dst++ = even;
	*dst++ = even;
	*dst++ = even;
	*dst++;
      }
#else
  /*
   * 16bpp (RGBA16) の場合
   */
  unsigned short *dst;

  dst = work->scrn; /* 16階調グレースケールが格納されるバッファのポインタ */
  src = tmp;        /* オーバーライドで縮小された画像のポインタ           */

  max = 0;
  rate = 16000;
  w = DRAW_WIDTH / 2;

  for(y = 0; y < DRAW_HEIGHT / 2; y++)
    for(x = 0; x < w; x++)
      {
	/* 偶数ピクセルの処理 */
#if 0
	even  = *(src++) * 299;
	even += *(src++) * 587;
	even += *(src++) * 114;
	src++;
	even /= rate;
	even *= 0x0421;   /* 結果をRGBA16 形式にまとめる */
#endif
	even  = (*(src++) >> 3) & 0x1f;
	even |= (*(src++) & 0xf8) << 2;
	even |= (*(src++) & 0xf8) << 7;
	src++;
 
	*dst++ = even;
      }

#endif /* TEX_16BPP */

  return max;
}


/* スクリーンショット取得
 * 縮小されたスクリーンショットを取得する */
static int scrn_shot(Work * work)
{
  void * tmp;

  /* スクリーンショット取得用のバッファを確保する */
  /*
   * テンポラリバッファを開放した際のメモリ断片化を防ぐため、
   * 先に結果用バッファを確保しておく。
   */
  
  /* 縮小結果用のバッファを先に確保する */
#ifndef TEX_16BPP
  if(NULL == (work->scrn = codecMalloc(DRAW_WIDTH * DRAW_HEIGHT)))
    return -1;
#else
  if(NULL == (work->scrn = codecMalloc((DRAW_WIDTH * DRAW_HEIGHT) / 2)))
    return -1;
#endif /* TEX_16BPP */

  /* 縮小結果は、16byte バウンダリになければならない */
  ASSERT(!((int)work->scrn & 0x0f));


  /* 縮小前のテンポラリ用バッファを次に確保する */
  if(NULL == (work->org_shot = tmp = codecMalloc(DRAW_WIDTH * DRAW_HEIGHT * 4)))
    {
      codecFree(work->scrn);
      work->scrn = NULL;
      return -1;
    }
  /* 画面を取得 */
  {
    int flip;

    flip = (DG_LastWhich < 0) ? DG_Clock : ( 1 - DG_LastWhich );
    sceGsSetDefStoreImage(&work->sp,
			  BUFFER_PAGE(flip), DRAW_WIDTH / 64,
			  SCE_GS_PSMCT32, 
			  0, 0, DRAW_WIDTH, DRAW_HEIGHT);
  }

  FlushCache(0);

  {
    int stat = 0;
    while(!stat) stat = DG_DmaCheckEnd();
  }

  sceGsSyncPath(0, 0);
  sceGsExecStoreImage(&work->sp, tmp);
  /*
   * 取得した画面を縮小する(縮小画面はテンポラリにオーバーライドされる)
   */
  scrn_shrink(work, tmp);

  /*
   * オーバーライドで作成された縮小画像を、16階調のグレースケールに変換する
   */
  scrn_grayscale(work, tmp);
  /* 通常背景の場合はテンポラリ領域を開放する */
  codecFree(tmp);  DBG("Free BG Temporaly\n");
  return 0;
}

/* 三次元座標を透視変換結果の画面座標に変換する */
static void scrn_3d_to_2d(float *x, float *y, int chanl, FVECTOR *vec)
{
}
#ifdef _DEBUG_
static void _disp_matrix(FMATRIX * mat)
{
  int i;

  for(i = 0; i < 4; i++)
    printf("| %5.2f  %5.2f  %5.2f %5.2f |\n",
	   mat->m[0][i], mat->m[1][i], mat->m[2][i], mat->m[3][i]);
}
#endif /* _DEBUG_ */

/* 画面上におけるプレイヤの顔の表示位置と表示大に合わせて、
   目標領域を設定する */
static void scrn_snake_face_position(Work * work)
{
  FMATRIX mat, *obj, *eye;
  FVECTOR vec, ans;
  float x[2], y[2];
  float w, h, siz;
  float cx, cy;

  /*
   * 画面上におけるスネークの顔位置を得る
   *
   * スネークの首のつけ根:
   *      GM_PlayerCBody->objs->objs[ HUMAN21_KUBI ].world.m[3][0]～[3][2]
   *
   * カメラの透視変換マトリクス:
   *      DG_Chanls[chanl].eye_pers
   */
  if(GM_PlayerBody != NULL)
    {
      obj = &GM_PlayerBody->objs->objs[ HUMAN16_KUBI ].world;
      eye = &DG_Chanls[0].eye_pers;
#ifdef _DEBUG_      
      /*
	_disp_matrix(obj);
	_disp_matrix(eye);
      */
#endif /* _DEBUG_ */

      /* 首のつけ根のオブジェクト座標を、vec に設定 */
      vec.vx = obj->m[3][0];
      vec.vy = obj->m[3][1];
      vec.vz = obj->m[3][2];
      vec.vw = 1.0;

      /* オブジェクトのマトリクスと透視変換マトリクスを
	 乗算したマトリクスを作成 */
      _sceVu0MulMatrix(&mat, eye, obj);
#ifdef _DEBUG_
      _disp_matrix(&mat);   /* M */
#endif /* _DEBUG_ */
      _sceVu0ApplyMatrix(&ans, &mat, &vec);
  
      {
	float x, y;
	x = ans.vx / ans.vw * DG_Chanls[0].width  / 2;
	y = ans.vy / ans.vw * DG_Chanls[0].height / 2;
	
	printf("(%8.3f, %8.3f)\n", x, y);
      }
    }



  /* デバッグ用に、画面内の一角を指定 */
  x[0] = DRAW_WIDTH / 2 - 32;
  x[1] = DRAW_WIDTH / 2 + 32;

  y[0] = DRAW_HEIGHT / 2 - 16;
  y[1] = DRAW_HEIGHT / 2 + 16;


  /* 顔の表示されている大きさから、最終的な表示領域を決定する */
  w = x[1] - x[0];
  h = y[1] - y[0];

  if(w < 0) w = -w;
  if(h < 0) h = -h;

  siz = (w < h) ? h : w;

  if(siz > DRAW_HEIGHT)
    {
      /* 顔が画面からはみ出してしまう場合は、そのままのサイズで描画する */
      w = DRAW_WIDTH;
      h = DRAW_HEIGHT;
      work->x[0] = 0, work->y[0] = 0;
      work->x[1] = w, work->y[1] = h;
    }
  else
    {
      h = siz;
      w = siz * DRAW_WIDTH / DRAW_HEIGHT;
      cx = (x[0] + x[1]) / 2;
      cy = (y[0] + y[1]) / 2;
      work->x[0] = cx - w / 2;  work->x[1] = cx + w / 2;
      work->y[0] = cy - h / 2;  work->y[1] = cy + h / 2;
    }

  /* 求められた領域の座標は画面サイズに基づくものであるため、
     これをテクスチャサイズに変換する。 */
  work->x[0] /= 2;  work->y[0] /= 2;
  work->x[1] /= 2;  work->y[1] /= 2;
}

/* 毎フレーム変動するパラメタを設定する */
static void scrn_dma_realtime_setup(Work * work)
{
  int u0, v0, u1, v1;

  u0 = (int)(work->u[0] * 16),  v0 = (int)(work->v[0] * 16);
  u1 = (int)(work->u[1] * 16),  v1 = (int)(work->v[1] * 16);
  
  u0 = v0 = 0;
  u1 = 256 * 2 / 3 * 16;
  v1 = 224 * 2 / 3 * 16;

  /* テクスチャの uv領域と、表示面積の設定 */
  work->packet[DG_Clock].gif.data.uv0.data = SCE_GS_SET_UV(u0, v0);
  work->packet[DG_Clock].gif.data.uv1.data = SCE_GS_SET_UV(u1, v1);
}

static tex_packet * scrn_tex_dma_setup(Work * work, tex_packet * packet)
{
  tex_packet * p;
  int w;

  w = (DRAW_WIDTH / 2 + 63) / 64;
  p = DG_MakeLoadImagePacket(packet,             /* tag address    */
#ifndef TEX_16BPP
			     SCE_GS_PSMCT32,
#else
			     SCE_GS_PSMCT16,     /* Pixel format   */
#endif /* TEX_16BPP */
			     DRAW_WIDTH / 2,     /* Texture width  */
			     DRAW_HEIGHT / 2,    /* Texture height */
			     TEXTURE_TOP_PAGE(),
			     w * 64, work->scrn);

#if 0
  packet->dmatag2.qwc = DMATAG_SET_QWC(DMATAG_ID_CNT, 2);
  packet->dmatag2.vifcode[0] = SCE_VIF1_SET_NOP(0);
  packet->dmatag2.vifcode[1] = SCE_VIF1_SET_DIRECT(2, 0);

  packet->giftag2.tag = SCE_GIF_SET_TAG(1, 1, 0, 0, 0, 1);
  packet->giftag2.regs = GS_REGS_AD;

  packet->texflush.reg = SCE_GS_TEXFLUSH;
  packet->texflush.data = 0;
#endif
  return p;
}

static int scrn_dma_setup(Work * work)
{
  static codecBG_DMA def_bg_dma = {
    dmatag:
    {
      qwc: DMATAG_SET_QWC(DMATAG_ID_RET, sizeof(struct _gif) / 16),
      vifcode:
      {
	SCE_VIF1_SET_NOP(0),
	SCE_VIF1_SET_DIRECT(sizeof(struct _gif) / 16, 0)
      },
    }, /* dmatag */
    gif:
    {
      giftag:
      {
	tag: SCE_GIF_SET_TAG(sizeof(struct _data) / 16, 1, 0, 0, 0, 1),
	regs: GS_REGS_AD
      }, /* giftag */
      data:
      {
	test0:
	{
	  reg: SCE_GS_TEST_1,
	  data: SCE_GS_SET_TEST(0, 0, 0, 0, 0, 0, 1, 2)
	},
	alpha:
	{
	  reg: SCE_GS_ALPHA_1,
	  data: SCE_GS_SET_ALPHA(1, 2, 1, 0, 128)
	},
	clamp:
	{
	  reg: SCE_GS_CLAMP_1,
	  data: SCE_GS_SET_CLAMP(1, 1, 0, DRAW_WIDTH / 2, 0, DRAW_HEIGHT / 2)
	},
	texflush:		/* added by K.Takabe 2000.11.28 */
	{
	  reg: SCE_GS_TEXFLUSH,
	  data: 0
	},
	tex0:
	{
	  reg: SCE_GS_TEX0_1,
	  data: SCE_GS_SET_TEX0(TEXTURE_TOP_PAGE() / 64, DRAW_WIDTH / 2 / 64,
#ifndef TEX_16BPP
				SCE_GS_PSMCT32,
#else
				SCE_GS_PSMCT16,
#endif /* TEX_16BPP */
				10, 10, 1, 0,
				0,    /* CLUT バッファベースアドレス */
				0,/* CLUT Pixel格納フォーマット */
				0,            /* CSM                         */
				0,            /* CLUT エントリオフセット     */
				4)            /* CLUT バッファロード制御     */
	},
	/*
	  tex1:
	  {
	  reg: SCE_GS_TEX1_1,
	  data: SCE_GS_SET_TEX1(0, 0, 1, 1, 0, 0, 0)
	  },
	*/
	prim:
	{
	  reg: SCE_GS_PRIM,
	  data: SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,  /* PRIM */
				0,                   /* IIP  */
				1,                   /* TME  */
				0,                   /* FGE  */
				1,                   /* ABE  */
				0,                   /* AA1  */
				1,                   /* FST  */
				0,                   /* CTXT */
				0)                   /* FIX  */
	},
	uv0:{ reg: SCE_GS_UV,  data: 0 },
	rgbq0:{
	  reg: SCE_GS_RGBAQ,
	  data: SCE_GS_SET_RGBAQ(MONO_R, MONO_G, MONO_B, 128, 0)
	  // data: SCE_GS_SET_RGBAQ(255, 0, 0, 128, 0)
	},
	xyz0:
	{
	  reg: SCE_GS_XYZ2,
	  /*
	    data: SCE_GS_SET_XYZ((2048 - DRAW_WIDTH / 2) * 16,
	    (2048 - DRAW_HEIGHT / 2) * 16, PRI_Z)
	  */
	  data: SCE_GS_SET_XYZ((2048 - DRAW_WIDTH / 4) * 16,
			       (2048 - DRAW_HEIGHT / 4) * 16, PRI_Z)
	},
	uv1:{ reg: SCE_GS_UV,  data: 0 },
	rgbq1:{
	  reg: SCE_GS_RGBAQ,
	  data: SCE_GS_SET_RGBAQ(MONO_R, MONO_G, MONO_B, 128, 0)
	  // data: SCE_GS_SET_RGBAQ(255, 0, 0, 128, 0)
	},
	xyz1:
	{
	  reg: SCE_GS_XYZ2,
	  /*
	    data: SCE_GS_SET_XYZ((2048 + DRAW_WIDTH / 2) * 16,
	    (2048 + DRAW_HEIGHT / 2) * 16, PRI_Z)
	  */
	  data: SCE_GS_SET_XYZ((2048 + DRAW_WIDTH / 4) * 16,
			       (2048 + DRAW_HEIGHT / 4) * 16, PRI_Z)
	},
	test1:
	{
	  reg: SCE_GS_TEST_1,
	  data: 0
	}
      }
    }
  };
  DG_DMAPACK * dmapack;
  codecBG_DMA * packet;

  work->f_cnt = 0;

  if(NULL == (packet = codecMalloc(sizeof(codecBG_DMA) * 2))) return -1;

  {
    unsigned long test;
    
    test = *(unsigned long *)&DG_Chanls[0].draw_env[0].datas.test1;
    def_bg_dma.gif.data.test1.data = test;
  }

  packet[0] = def_bg_dma;
  packet[1] = def_bg_dma;

  scrn_tex_dma_setup(work, &packet[0].tex_trans);
  scrn_tex_dma_setup(work, &packet[1].tex_trans);

#if 0
  {
    void * nxt[2];
#ifdef CODEC_BACKGROUND
    SPR_SetPreDrawTag(BG_CHANL, &nxt[0], &nxt[1], &packet[0], &packet[1]);
#endif /* CODEC_BACKGROUND */
    packet[0].dmatag.addr = nxt[0];
    packet[1].dmatag.addr = nxt[1];
  }
#endif
  
  dmapack = work->dmapack = DG_MakeDmapack2(DG_DMAPACK_MENU |
					    DG_DMAPACK_PRIVILEGE,
					    DG_DMAPACK_PHASE_FIRST, 0);

  /* フェードアウトモードでなければ、最初から転送を開始する */
  dmapack->flag &= ~DG_DMAPACK_INVISIBLEMENU;
  dmapack->packet[0] = &packet[0];
  dmapack->packet[1] = &packet[1];
#ifndef NO_BACKGROUND
  DG_QueueDmapack(dmapack); 
#endif /* NO_BACKGROUND */
  work->packet = packet;

  scrn_dma_realtime_setup(work); 

  return 0;
}

/* 加工プロセス
 * 以下の加工を行う
 *
 * (1) グレースケール化
 * (2) 4値化などまで色を落す
 * (3) テクスチャとして設定できる形式に変換 */
static void scrn_process(Work * work)
{
  int max;

#if 0
  /* グレースケール化する */
  max = scrn_grayscale(work);

  /* 4値化する */
  scrn_four_colors(work, max);
#endif

  /* テクスチャとして転送する準備を行う */
  scrn_snake_face_position(work);
  scrn_dma_setup(work);
}

static void Act(Work * work)
{
  int n, step;
  int w, h;
  if(work->step == CodecDemo_Fix ||
     work->step == CodecDemo_Recover)
    {
      scrn_dma_realtime_setup(work);
      return;
    }
#if 0
  {
    static int cnt = 300;
    if(cnt < 300)
      {
	cnt += TIME_BASE;
	return;
      }
    cnt -= 300;
  }
#endif

  step = DEMO_TIME / TIMEBASE;  /* 変化の所要フレーム数 */
  n = work->f_cnt;
  w = DRAW_WIDTH / 2;   /* テクスチャのサイズは、画面表示の半分になる */
  h = DRAW_HEIGHT / 2;  /* テクスチャのサイズは、画面表示の半分になる */
  if(work->mode == CODEC_DEMO_IN)
    {
      /*
       * 設定(ゲーム画面を背景に)
       */
      if(work->step == CodecDemo_Zoom) work->f_cnt++;
      if(work->f_cnt >= step)
	{
	  work->f_cnt = step;
	  work->step = CodecDemo_Fix;
	}
    }
  else
    {
      /*
       * 画面復帰(背景をもとのゲーム画面に)
       */
      if(work->step == CodecDemo_Wide) work->f_cnt--;
      if(work->f_cnt <= 0)
	{
	  work->f_cnt = 0;
	  work->step = CodecDemo_Recover;
	}
    }
  n = work->f_cnt;
  work->u[0] = work->x[0] * n / step;
  work->v[0] = work->y[0] * n / step;
  work->u[1] = w - (w - work->x[1]) * n / step;
  work->v[1] = h - (h - work->y[1]) * n / step;

  if(work->u[0] < 0) work->u[0] = 0;
  if(work->u[0] >= DRAW_WIDTH) work->u[0] = DRAW_WIDTH - 1;
  if(work->u[1] < 0) work->u[1] = 0;
  if(work->u[1] >= DRAW_WIDTH) work->u[1] = DRAW_WIDTH - 1;

  if(work->v[0] < 0) work->v[0] = 0;
  if(work->v[0] >= DRAW_HEIGHT) work->v[0] = DRAW_HEIGHT - 1;
  if(work->v[1] < 0) work->v[1] = 0;
  if(work->v[1] >= DRAW_HEIGHT) work->v[1] = DRAW_HEIGHT - 1;

  /* 作った値をDMAパケットに設定 */
  scrn_dma_realtime_setup(work);
}

static void Die(Work * work)
{
  if(work == now_demo) now_demo = NULL;
#if 0
#ifdef CODEC_BACKGROUND
  SPR_InitPreDrawTag(BG_CHANL);
#endif /* CODEC_BACKGROUND */
#endif
#ifndef NO_BACKGROUND
  DG_DequeueDmapack(work->dmapack);
#endif /* NO_BACKGROUND */
  DG_FreeDmapack(work->dmapack);
  codecFree(work->packet);
  codecFree(work->scrn);
  DBG("codec_into_demo is die.\n");
}

static int GetResources(Work * work, CodecDemoMode mode, int fade)
{
  work->fade = fade;   /* フェードアウト開始モードの場合の挙動 */
  work->mode = mode;
  scrn_shot(work);    /* スクリーンショット取得 */
  scrn_process(work); /* 加工、背景に。         */ 
  if(mode == CODEC_DEMO_IN)   /* 無線モードに入る   */
    {
      work->f_cnt = 0;
      if(fade) work->f_cnt = DEMO_TIME / TIMEBASE;
      work->step = CodecDemo_Zoom;
    }
  else                        /* 無線モードから出る */
    {
      work->f_cnt = DEMO_TIME / TIMEBASE;
      work->step = CodecDemo_Wide;
    }

  return 0;
}

/*
 * プログラムインタフェース
 */
void * NewCodecIntoDemo(CodecDemoMode mode, int fade)
{
  Work * work;

  ASSERT(now_demo == NULL);

  if(NULL == (work = codecActorPrio(ACT_MODE, sizeof(Work), 0xf0)))
    return NULL;

  GV_SetActor(&work->actor, Act, Die);
  GV_ActorEX(&work->actor);

  if(GetResources(work, mode, fade))
    {
      GV_DestroyActor(work);
      return NULL;
    }

  if(NULL != now_demo) GV_DestroyActor(now_demo);

  now_demo = work;

  return work;
}

/*
 * 背景の復帰を指示する
 */
int CodecBG_Recover(Work * work, int fade)
{
  if(NULL == work) work = now_demo;
  if(NULL == work) return -1;

  printf("CodecBG_Recover() is called.\n");
  work->mode = CODEC_DEMO_OUT;
  work->step = CodecDemo_Wide;
  work->f_cnt = DEMO_TIME / TIME_BASE;
  if(fade) work->f_cnt = 0;
  return 0;
}


int IsBGsetuped(void)
{
  if(NULL == now_demo) return 1;
  if(now_demo->step == CodecDemo_Fix) return 1;
  return 0;
}

int IsBGrecovered(void)
{
  if(NULL == now_demo) return 1;
  if(now_demo->step == CodecDemo_Recover) return 1;
  return 0;
}
