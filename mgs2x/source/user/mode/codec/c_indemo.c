//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  c_indemo.c
  無線モードに出入りする際のデモンストレーション
  ゲーム中の画面を背景に加工する。

  $Id: c_indemo.c,v 1.4 2002/11/23 12:16:40 Yoshizawa1 Exp $
*/
/*
  無線モードに入る際に、ゲーム中の実画面を加工、拡大し、
  無線モードにおける背景にする。

  また、無線モードを終える時に、背景としていた画面をゲーム中の画面に
  再びつなぐ演出をする。
*/

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

#include "BP_RenderBufferTypes.h"
#include "BP_Renderer.h"

// defined in BP_Renderer, but only used here
extern void DG_FreeLinerTexture2( DG_TEX_LIN *tex );

void *DG_MakeLoadImagePacket( void *tag_addr, int fmt, int width, int height, int vram_addr, int vram_width, void *tex_addr );

#undef _DEBUG_

#define DBG

// AS(JM) - Disables printf for whole CPP. If it's already disabled, redisable it
#ifdef printf
#  undef printf
#endif

#define printf

#define PRI_Z   0
#define BG_CHANL 4

#define TIMEBASE (BP_BASE_TICK())

#define TEX_16BPP     /* テクスチャをRGBA16 として格納するときに定義 */

#ifndef TEX_16BPP
#define TEX_PSM  SCE_GS_PSMCT32
#else
#define TEX_PSM  SCE_GS_PSMCT16
#endif /* TEX_16BPP */


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

typedef ALIGN16_DECL(struct) {
  DG_DMATAG dmatag0;
  DG_GIFTAG giftag0;
  DG_GSREG  gsregs[4];
  DG_GIFTAG giftag1;
  DG_DMATAG dmatag1;
} tex_packet ;

/*
 * ゲーム画面を加工した結果のテクスチャを背景として転送する DMA パケット構造体
 */
typedef ALIGN16_DECL(struct) {
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
} codecBG_DMA ;

typedef struct _c_indemo_work
{
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

  DG_TEX_LIN *ltex ;/* 線形テクスチャ */
  DG_TEX_LIN ltexbody ;/* 線形テクスチャ */
  // BP_Render
  int             needPreviousFrameCopy;
}
SCodecInDemoWork;

static void *_c_indemo_buffer_callback(void *pParam, DG_DMAPACK *pDstDmaPack)
{
   void *dst_work = DG_AS_SceneBufferAllocCopy( pParam, sizeof( SCodecInDemoWork ) );

   // Always set this to false after the first execution
   // ONLY has effect if scene buffering. Make sure to do similar work in the 
   // other callback
   ((SCodecInDemoWork *) pParam)->needPreviousFrameCopy = FALSE;

   return dst_work;
}

static void BP_CodexInOutCallback(void* pWork)
{
   int i;

   SCodecInDemoWork *work = (SCodecInDemoWork *)pWork;

   SBP_PFX_CodexInOut* pPacket = (SBP_PFX_CodexInOut*)BP_RB_Alloc(sizeof(SBP_PFX_CodexInOut));
   pPacket->needPreviousFrameCopy = work->needPreviousFrameCopy;

   pPacket->colorWeight[0] = 1.0f; // r
   pPacket->colorWeight[1] = 1.0f; // g
   pPacket->colorWeight[2] = 1.0f; // b
   pPacket->colorWeight[3] = 1.0f; // a

   pPacket->uMinMax_vMinMax[0] = work->u[0]/(DRAW_WIDTH/2);
   pPacket->uMinMax_vMinMax[1] = work->u[1]/(DRAW_WIDTH/2);
   pPacket->uMinMax_vMinMax[2] = work->v[0]/(DRAW_HEIGHT/2);
   pPacket->uMinMax_vMinMax[3] = work->v[1]/(DRAW_HEIGHT/2);

   if( DG_Chanls_Buf[2].flag == 1 || DG_Chanls_Buf[3].flag == 1 )
   {
      float x, y;

      x = (float)DG_Chanls_Buf[2].offset_x + DRAW_WIDTH / 2.0f - (float)DG_Chanls_Buf[2].width / 2.0f;
      y = (float)DG_Chanls_Buf[2].offset_y + DRAW_HEIGHT / 2.0f - (float)DG_Chanls_Buf[2].height / 2.0f;
      pPacket->faceBoxes[0][0] = x;
      pPacket->faceBoxes[0][1] = x+(float)DG_Chanls_Buf[2].width;
      pPacket->faceBoxes[0][2] = y;
      pPacket->faceBoxes[0][3] = y+(float)DG_Chanls_Buf[2].height;

      x = (float)DG_Chanls_Buf[3].offset_x + DRAW_WIDTH / 2.0f - (float)DG_Chanls_Buf[3].width / 2.0f;
      y = (float)DG_Chanls_Buf[3].offset_y + DRAW_HEIGHT / 2.0f - (float)DG_Chanls_Buf[3].height / 2.0f;
      pPacket->faceBoxes[1][0] = x;
      pPacket->faceBoxes[1][1] = x+(float)DG_Chanls_Buf[3].width;
      pPacket->faceBoxes[1][2] = y;
      pPacket->faceBoxes[1][3] = y+(float)DG_Chanls_Buf[3].height;
      // To normalized coordinates
      pPacket->faceBoxes[0][0] = pPacket->faceBoxes[0][0]/(DRAW_WIDTH / 2.0f) - 1.0f;
      pPacket->faceBoxes[0][1] = pPacket->faceBoxes[0][1]/(DRAW_WIDTH / 2.0f) - 1.0f;
      pPacket->faceBoxes[0][2] = pPacket->faceBoxes[0][2]/(-DRAW_HEIGHT / 2.0f) + 1.0f;
      pPacket->faceBoxes[0][3] = pPacket->faceBoxes[0][3]/(-DRAW_HEIGHT / 2.0f) + 1.0f;

      pPacket->faceBoxes[1][0] = pPacket->faceBoxes[1][0]/(DRAW_WIDTH / 2.0f) - 1.0f;
      pPacket->faceBoxes[1][1] = pPacket->faceBoxes[1][1]/(DRAW_WIDTH / 2.0f) - 1.0f;
      pPacket->faceBoxes[1][2] = pPacket->faceBoxes[1][2]/(-DRAW_HEIGHT / 2.0f) + 1.0f;
      pPacket->faceBoxes[1][3] = pPacket->faceBoxes[1][3]/(-DRAW_HEIGHT / 2.0f) + 1.0f;

      pPacket->needFaceBoxes = 1;
   }
   else
   {
      pPacket->needFaceBoxes = 0;
   }

   BP_RB_AddCommand(kCmd_PostFX_CodexInOut, (char*)pPacket);

   // Only has an effect if not scene buffering
   work->needPreviousFrameCopy = FALSE;
}

static SCodecInDemoWork * now_demo = NULL;


static void scrn_shrink(SCodecInDemoWork * work, void * tmp)
{
  unsigned char *src, *dst;
  unsigned char *e, *o;
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
      for(x = 0; x < DRAW_WIDTH / 2; x++)
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
    }
}

/* グレースケール化:
   16階調のグレースケールにする。これにより1pixel を4bit で扱うことができる。
*/
static int scrn_grayscale(SCodecInDemoWork * work, unsigned char * tmp)
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
	even  = *(src++) * 299;
	even += *(src++) * 587;
	even += *(src++) * 114;
	src++;
	even /= rate;
	even *= 0x0421;   /* 結果をRGBA16 形式にまとめる */
	*dst++ = even | 0x8000;
      }

#endif /* TEX_16BPP */

  return max;
}


/* スクリーンショット取得
 * 縮小されたスクリーンショットを取得する */
static int scrn_shot(SCodecInDemoWork * work)
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

#ifdef KP_XBOX
  DG_StoreImage( tmp , 1 /* 表示中バッファ */, 
				 0, 16 /*フレームパッファは480。真中448をとる為*/, 
				 DRAW_WIDTH, DRAW_HEIGHT,
				 0/* 32bit */ );
#endif

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
static void scrn_snake_face_position(SCodecInDemoWork * work)
{
  FMATRIX *head, *neck, *eye;
  FVECTOR vhd, vnk, ans_head, ans_neck;
  float x[2], y[2];
  float w, h, siz;
  float cx, cy;
  float dx, dy;

  /*
   * 画面上におけるスネーク/ライデンの顔位置を得る
   *
   *
   * 頭部:
   *      GM_PlayerCBody->objs->objs[ HUMAN21_ATAMA ].world.m[3][0]～[3][2]
   *
   * 首のつけ根:
   *      GM_PlayerCBody->objs->objs[ HUMAN21_KUBI ].world.m[3][0]～[3][2]
   *
   * カメラの透視変換マトリクス:
   *      DG_Chanls[chanl].eye_pers
   */
  if(GM_PlayerBody != NULL)
    {
      head = &GM_PlayerBody->objs->objs[ HUMAN21_ATAMA ].world;
      neck = &GM_PlayerBody->objs->objs[ HUMAN21_KUBI ].world;
      eye = &DG_Chanls[0].raise_eye_pers;
#ifdef _DEBUG_      
      /*
	_disp_matrix(obj);
	_disp_matrix(eye);
      */
#endif /* _DEBUG_ */

      /* 頭のオブジェクト座標を、vhd に設定 */
      vhd.vx = head->m[3][0];
      vhd.vy = head->m[3][1];
      vhd.vz = head->m[3][2];
      vhd.vw = head->m[3][3];

      
      /* 首のつけねのオブジェクト座標を、vnk に設定 */
      vnk.vx = neck->m[3][0];
      vnk.vy = neck->m[3][1];
      vnk.vz = neck->m[3][2];
      vnk.vw = neck->m[3][3];


      /* 取得したベクトル値を、視点マトリクスと乗算する */
      _sceVu0ApplyMatrix(&ans_head, eye, &vhd);
      _sceVu0ApplyMatrix(&ans_neck, eye, &vnk);

      ans_head.vx = ans_head.vx / ans_head.vw * DG_Chanls[0].width / 2;
      ans_head.vy = ans_head.vy / ans_head.vw * DG_Chanls[0].height / 2;
      ans_head.vz = ans_head.vz / ans_head.vw * DRAW_Z_SCALE+DRAW_Z_OFFSET;
      
      ans_neck.vx = ans_neck.vx / ans_neck.vw * DG_Chanls[0].width / 2;
      ans_neck.vy = ans_neck.vy / ans_neck.vw * DG_Chanls[0].height / 2;
      ans_neck.vz = ans_neck.vz / ans_neck.vw * DRAW_Z_SCALE+DRAW_Z_OFFSET;
      
      dx = (ans_neck.vx > ans_head.vx)
	? (ans_neck.vx - ans_head.vx) : (ans_head.vx - ans_neck.vx);
      
      dy = (ans_neck.vy > ans_head.vy)
	? (ans_neck.vy - ans_head.vy) : (ans_head.vy - ans_neck.vy);

      if((dx < 32.0F) && (dy < 32.0F))
	{
	  x[0] = ans_head.vx - 32.0F;
	  x[1] = ans_head.vx + 32.0F;
	  
	  y[0] = ans_head.vy - 32.0F;
	  y[1] = ans_head.vy + 32.0F;
	}
      else
	{
	  if(dy > dx)  /* 顔が画面に対し、縦に近く配置されている */
	    {
	      x[0] = ans_head.vx - dy * 1.25F;
	      x[1] = ans_head.vx + dy * 1.25F;
	      
	      y[0] = ans_head.vy - dy;
	      y[1] = ans_head.vy + dy;
	    }
	  else         /* 顔が画面に対し、横に近く配置されている */
	    {
	      x[0] = ans_head.vx - dx;
	      x[1] = ans_head.vx + dx;
	      
	      y[0] = ans_head.vy - dx * 0.75F;
	      y[1] = ans_head.vy + dx * 0.75F;
	    }
	}

      
#ifdef _DEBUG_
//      _disp_matrix(&mat);   /* M */
#endif /* _DEBUG_ */
    }
  else
    {
      /* デバッグ用に、画面内の一角を指定 */
      x[0] = -32.0F;
      x[1] =  32.0F;
      
      y[0] = -32.0F;
      y[1] =  32.0F;
    }

  x[0] += (float)(DRAW_WIDTH / 2);
  x[1] += (float)(DRAW_WIDTH / 2);
  y[0] += (float)(DRAW_HEIGHT / 2);
  y[1] += (float)(DRAW_HEIGHT / 2);


  /* 領域が明らかに画面外にある場合,画面中央を拡大する */
  if((x[1] < 0.0F) || (x[0] >= DRAW_WIDTH) ||
     (y[1] < 0.0F) || (y[0] >= DRAW_HEIGHT))
    {
      x[0] = (float)(DRAW_WIDTH / 2) - 32.0F;
      x[1] = (float)(DRAW_WIDTH / 2) + 32.0F;
      y[0] = (float)(DRAW_HEIGHT / 2) - 16.0F;
      y[1] = (float)(DRAW_HEIGHT / 2) + 16.0F;
    }
  

  /* 領域があきらかに画面より大きな場合は、画面中央を取得して、
     その領域を拡大する */
  if(((x[0] < 0.0F) && (x[1] >= DRAW_WIDTH)) ||
     ((y[0] < 0.0F) && (y[1] >= DRAW_HEIGHT)))
    {
      x[0] = (float)(DRAW_WIDTH / 2) - 32.0F;
      x[1] = (float)(DRAW_WIDTH / 2) + 32.0F;
      y[0] = (float)(DRAW_HEIGHT / 2) - 16.0F;
      y[1] = (float)(DRAW_HEIGHT / 2) + 16.0F;
    }


  /* 領域が画面端からはみ出ている場合は、大きさはそのままに、
     画面内に収まる位置まで領域を移動する */
  if(x[0] < 0.0F)  x[1] -= x[0], x[0] = 0.0F;
  if(y[0] < 0.0F)  y[1] -= y[0], y[0] = 0.0F;
  if(x[1] >= (float)DRAW_WIDTH)
    x[0] -= x[1] - (float)(DRAW_WIDTH-4), x[1] = (float)(DRAW_WIDTH-4);
  if(y[1] >= (float)DRAW_HEIGHT)
    y[0] -= y[1] - (float)(DRAW_HEIGHT-4), y[1] = (float)(DRAW_HEIGHT-4);
  

  /* 顔の表示されている大きさから、最終的な表示領域を決定する */
  w = x[1] - x[0];
  h = y[1] - y[0];

  if(w < 0) w = -w;
  if(h < 0) h = -h;

  siz = (w < h) ? h : w;

  if(siz >= DRAW_HEIGHT)
    {
      /* 顔が画面からはみ出してしまう場合は、そのままのサイズで描画する */
      w = DRAW_WIDTH;
      h = DRAW_HEIGHT;
      work->x[0] = 0, work->y[0] = 0;
      work->x[1] = w, work->y[1] = h;
    }
  else
    {
      float wx = (float)DRAW_WIDTH / (float)DRAW_HEIGHT;

      if(w < (h * wx))
	h = w / wx;
      else
	w = h * wx;

      cx = (x[0] + x[1]) / 2;
      cy = (y[0] + y[1]) / 2;
      work->x[0] = cx - w / 2;  work->x[1] = cx + w / 2;
      work->y[0] = cy - h / 2;  work->y[1] = cy + h / 2;
    }

  /* 求められた領域の座標は画面サイズに基づくものであるため、
     これをテクスチャサイズに変換する。 */
  work->x[0] /= 2;  work->y[0] /= 2;
  work->x[1] /= 2;  work->y[1] /= 2;

  /* UV 値がテクスチャからはみ出すのを防ぐため、0.5 texel 内側に縮める */
  work->x[0] += 0.5F, work->y[0] += 0.5F;
  work->x[1] -= 0.5F, work->y[1] -= 0.5F;
}

/* 毎フレーム変動するパラメタを設定する */
static void scrn_dma_realtime_setup(SCodecInDemoWork * work)
{
  {
	  void* prim;
	  unsigned int col;

	  prim = work->dmapack->autopacket;
	  col = MONO_R | (MONO_G<<8) | (MONO_B<<16) | (128<<24); 
	  
	  /* 顔表示領域をくりぬく */
	  if( DG_Chanls[2].flag == 1 || DG_Chanls[3].flag == 1 ){
		  float x, y;
		  prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(2,2,2,1,128) );
		  x = (float)DG_Chanls[2].offset_x + DRAW_WIDTH / 2.0f - (float)DG_Chanls[2].width / 2.0f + 1.0f;
		  y = (float)DG_Chanls[2].offset_y + DRAW_HEIGHT / 2.0f - (float)DG_Chanls[2].height / 2.0f + 1.0f;
		  prim = DG_SetDmapackSetZ( prim, 1.0f );
		  prim = DG_SetDmapackBox( prim, x, y, x+(float)DG_Chanls[2].width, y+(float)DG_Chanls[2].height, DG_MakeDmaPackColorFromInt(0x80808080) );/* 左 */
		  x = (float)DG_Chanls[3].offset_x + DRAW_WIDTH / 2.0f - (float)DG_Chanls[3].width / 2.0f + 1.0f;
		  y = (float)DG_Chanls[3].offset_y + DRAW_HEIGHT / 2.0f - (float)DG_Chanls[3].height / 2.0f + 1.0f;
		  prim = DG_SetDmapackSetZ( prim, 1.0f );
		  prim = DG_SetDmapackBox( prim, x, y, x+(float)DG_Chanls[3].width, y+(float)DG_Chanls[3].height, DG_MakeDmaPackColorFromInt(0x80808080) );/* 右 */
	  }
	  /* 線形テクスチャの大きさは(DRAW_WIDTH/2)×(DRAW_HEIGHT/2) */
	  prim = DG_SetDmapackTexLin( prim , work->ltex );
	  prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA(1,2,1,0,128) );
	  prim = DG_SetDmapackSetZ( prim, 0.9f );

	  prim = DG_SetDmapackSprt( prim, 0.0f, 0.0f, work->u[0]/320.0f, work->v[0]/224.0f,
							   DRAW_WIDTH, DRAW_HEIGHT, work->u[1]/320.0f, work->v[1]/224.0f, DG_MakeDmaPackColorFromInt(col) );
	  prim = DG_SetDmapackEnd( prim );

     work->dmapack->autopacketSize = (char *) prim - (char *) work->dmapack->autopacket;
  }
}

static tex_packet * scrn_tex_dma_setup(SCodecInDemoWork * work, tex_packet * packet)
{
   return NULL;
}

static int scrn_dma_setup(SCodecInDemoWork * work)
{
  DG_DMAPACK * dmapack;
  void* packet;
  int packsize;

  work->f_cnt = 0;
  {
	  /* 線形テクスチャ作り */
	  DG_TEX_LIN *prim;
	  int i;
	  extern void DG_MakeLinerTexture2( DG_TEX_LIN *tex, int width, int height, int format, void *addr );

	  prim = &work->ltexbody;
	  
	  DG_MakeLinerTexture2( prim, DRAW_WIDTH /2, DRAW_HEIGHT /2
						   , DG_TEXLIN_FORMAT_A1R5G5B5, work->scrn );
//	  for( i = 0; i < DRAW_WIDTH /2 * DRAW_HEIGHT /2; i++ ){
//		  ((unsigned short*)work->scrn)[i] |= (1<<15); /* alpha = 1 を代入 */
//	  }
//	  memcpy( prim->image, work->scrn, DRAW_WIDTH /2 * DRAW_HEIGHT /2 * sizeof(short) );
	  work->ltex = prim ; 
  }
  work->u[0] = 0.5F;
  work->v[0] = 0.5F;
  work->u[1] = (float)(DRAW_WIDTH / 2) - 0.5F;
  work->v[1] = (float)(DRAW_HEIGHT / 2) - 0.5F;

  packsize = sizeof(DG_DMAPACK_TEX)+sizeof(DG_DMAPACK_ALPHA)+sizeof(DG_DMAPACK_SPRT)+sizeof(DG_DMAPACK_PARAM)
	+sizeof(DG_DMAPACK_TAG) 
	  +sizeof(DG_DMAPACK_ALPHA)+sizeof(DG_DMAPACK_PARAM)*2+sizeof(DG_DMAPACK_BOX)*2;/* くり抜き用 */
  //printf("c_indemo packsize =%d\n",packsize);
  packet = codecMalloc( packsize );
  
  
  dmapack = work->dmapack = DG_MakeDmapack2(DG_DMAPACK_MENU |
					    DG_DMAPACK_PRIVILEGE,
					    DG_DMAPACK_PHASE_AFTER, 0);

  /* フェードアウトモードでなければ、最初から転送を開始する */
  dmapack->flag &= ~DG_DMAPACK_INVISIBLEMENU;

  dmapack->autopacket = packet ;
  packet = DG_SetDmapackEnd( packet );
  dmapack->autopacketSize = (char *) packet - (char *) dmapack->autopacket;

#ifndef NO_BACKGROUND
   DG_QueueDmapack(dmapack);

   DG_AS_DMAPackSetCallbacks(dmapack, BP_CodexInOutCallback, _c_indemo_buffer_callback);
   dmapack->BP_callbackParam = work;
#endif /* NO_BACKGROUND */

   work->packet = dmapack->autopacket;

  scrn_dma_realtime_setup(work); 

  return 0;
}

/* 加工プロセス
 * 以下の加工を行う
 *
 * (1) グレースケール化
 * (2) 4値化などまで色を落す
 * (3) テクスチャとして設定できる形式に変換 */
static void scrn_process(SCodecInDemoWork * work)
{
  /* テクスチャとして転送する準備を行う */
  scrn_snake_face_position(work);
  scrn_dma_setup(work);
}

static void Act(SCodecInDemoWork * work)
{
  int n, step;
  int w, h;

  if(work->step == CodecDemo_Fix ||
     work->step == CodecDemo_Recover)
    {
      scrn_dma_realtime_setup(work);
      return;
    }

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

static void Die(SCodecInDemoWork * work)
{
  if(work == now_demo) now_demo = NULL;

#ifndef NO_BACKGROUND
  DG_DequeueDmapack(work->dmapack);
#endif /* NO_BACKGROUND */
  DG_FreeLinerTexture2(&work->ltexbody);
  DG_FreeDmapack(work->dmapack);

  codecDelayedFree(work->packet);
  codecDelayedFree(work->scrn);

  DBG("codec_into_demo is die.\n");
}

static int ReceiveSignal(void * workp, int signal,int value)
{
  SCodecInDemoWork * work = workp;
  int ret = 0;
  switch(signal)
    {
    case GV_SIGNAL_KILL:
      DBG("c_indemo.c: recieve kill signal.\n");
    default:
      ret = GV_DefaultSignalFunc(workp, signal, value);
      break;
    }
  return ret;
}

static int GetResources(SCodecInDemoWork * work, CodecDemoMode mode, int fade)
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
  GV_SetActorSignalFunc(work, ReceiveSignal);

  work->needPreviousFrameCopy = 1;

  return 0;
}

/*
 * プログラムインタフェース
 */
void * NewCodecIntoDemo(CodecDemoMode mode, int fade)
{
  SCodecInDemoWork * work;

  ASSERT(now_demo == NULL);

  if(NULL == (work = codecActorPrio(ACT_MODE, sizeof(SCodecInDemoWork), 0xf0)))
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
int CodecBG_Recover(void * workp, int fade)
{
  SCodecInDemoWork * work = workp;

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
