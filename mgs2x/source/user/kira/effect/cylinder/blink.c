//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  blink.c
  空中文字点滅オブジェクト
  複数の円筒テクスチャを起動し、同期させて明滅させる。
  枠の部分は動かさず、ウィンドウの文字のみを UV アニメーションさせる。

  2001/04/24  Y.Kira
  $Id: blink.c,v 1.1.1.3 2002/11/19 11:43:52 Yoshizawa1 Exp $
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

#include "gameheader.h"
#include "g_struct.h"

#include "font.h"
#include "libfs.h"
#include "libdg.h"
#include "libmt.h"
#include "libutl.h"
#include "utl_dma.h"
#include "vu0_pack.h"
#include "fpu_pack.h"
#include "fmt_demo.h"
#include "dmapack.h"
#include "def_dma.h"
#define _blink_c_
#include "cylinder.h"
#include "blink.h"

#define BASE_POINT ((float)1000)

#ifdef __GNUC__
#ifdef DEBUG
#define DBG(args...)  printf(args)
#else
#define DBG(args...)
#endif /* DEBUG */
#else
#define DBG
#endif



typedef struct {
  GV_ACT_EX   actor;

  int         name;      /* 名前                                */


  FVECTOR     base;      /* 円柱の軸の起点                      */
  FVECTOR     pole_rot;  /* 軸の回転角                          */

  int         alpha;
  int         pre_alpha; /* 直前のアルファ値 */
  int         max_alpha; /* その時点で最大にとりうるアルファ値  */
  int         fix_alpha; /* その chara が最大にとりうるアルファ値 */

  float       tx_w, tx_h; /* 外枠テクスチャのサイズ */


  float       image_w;   /* 枠イメージの幅(弧の長さに対応)      */
  float       image_h;   /* 枠イメージの高さ                    */
  float       image_r;   /* 円柱半径(軸からの距離)              */
  float       image_y;   /* 枠イメージの底の、起点からの y 位置 */

  /*
    文字イメージの開始位置とサイズ

    枠イメージの幅に対する比率で与えられる。
  */    
  float       chr_x;     /* 枠イメージの左端からの位置(枠イメージに対する比) */
  float       chr_y;     /* 枠イメージの上端からの位置(枠イメージに対する比) */
  float       chr_w;     /* 文字イメージの幅(枠イメージに対する比)   */
  float       chr_h;     /* 文字イメージの高さ(枠イメージに対する比) */
  

  int         tri_name;  /* 使用 tri     */
  int         tex_frame; /* 外枠 texture */
  int         tex_chr;   /* 文字 texture */
  int         tex_bg;    /* 背景 texture */
  

  int         spin_time;        /* 一周所要時間 */
  float       spin_init;        /* 初期角       */
     
  int         slide_time;       /* 軸沿い移動所要時間 */
  float       slide_start;      /* 始端高さ           */
  float       slide_end;        /* 終端高さ           */

  int         scale_time;       /* 半径変更所要時間   */
  float       scale_start;      /* 開始半径           */
  float       scale_end;        /* 終了半径           */

  int         stretch_time;     /* 縦引き伸ばし所要時間 */
  float       stretch_start;    /* 開始高さ             */
  float       stretch_end;      /* 終了高さ             */

  /* テキスト部アニメーションパラメータ */
  int         uv_time;          /* UV アニメーション時間 */
  float       uv_start_u;       /* 開始 U   */
  float       uv_start_v;       /* 開始 V   */
  float       uv_start_w;       /* 開始幅   */
  float       uv_start_h;       /* 開始高さ */
  float       uv_end_u;         /* 終了 U   */
  float       uv_end_v;         /* 終了 V   */
  float       uv_end_w;         /* 終了幅   */
  float       uv_end_h;         /* 終了高さ */

  /* 背景 UV アニメーションパラメータ */
  int         bg_time;          /* 背景UV アニメーション時間 */
  float       bg_start_u;       /* 開始 U   */
  float       bg_start_v;       /* 開始 V   */
  float       bg_start_w;       /* 開始幅   */
  float       bg_start_h;       /* 開始高さ */
  float       bg_end_u;         /* 終了 U   */
  float       bg_end_v;         /* 終了 V   */
  float       bg_end_w;         /* 終了幅   */
  float       bg_end_h;         /* 終了高さ */

  int         min_blight_time;  /* 最短点灯時間 */
  int         max_blight_time;  /* 最長点灯時間 */

  int         min_dark_time;    /* 最短消灯時間 */
  int         max_dark_time;    /* 最長消灯時間 */

  int         cont_time;        /* 現在の状態を保持する時間 */
  int         cnt_time;         /* タイムカウンタ           */
  int         mode:1;

  int         player_lookup:1;  /* プレイヤ同期 */
  int         plate:1;          /* 平板モード */

} Work;

static void setup_mode(Work * work, int mode)
{
  int r, min;

  if(mode)
    {
      min = work->min_blight_time;
      r = work->max_blight_time - min;
    }
  else
    {
      min = work->min_dark_time;
      r = work->max_dark_time - min;
    }
  work->mode = mode;
  work->cont_time = min + ((BP_PS2_rand() >> 16) % r);
}



/*
 * 二つの子 Actor のシグナルハンドラを呼び出し、アルファ値を際設定する
 */
static void send_alpha(Work * work)
{
  if(work->alpha != work->pre_alpha)
    {
      GV_CallChildSignalFunc(work, CYLINDER_ALPHA_SIGNAL, work->alpha);
      work->pre_alpha = work->alpha;
    }
}

static void message_proc(Work * work)
{
  int n_msg;
  GV_MSG * msg;
  int i;

  if(work->name < 0) return;
  if(!(n_msg = GV_ReceiveMessage(work->name, &msg))) return;

  for(i = 0; i < n_msg; i++, msg++)
    {
      switch(msg->message[0])
	{
	case 0:   /* アルファ値設定 */
	  {
	    work->max_alpha = msg->message[1];
	    if(work->max_alpha > work->fix_alpha)
	      work->max_alpha = work->fix_alpha;
	  }
	  break;
	case 1:   /* アルファ値リセット */
	  work->max_alpha = work->fix_alpha;
	  break;
	}
    }
}


static void Act(Work * work)
{
  message_proc(work);

  work->cnt_time += TIME_BASE;
  if(work->cnt_time >= work->cont_time)
    {
      int next;

      next = (work->mode) ? 0 : 1;
      work->cnt_time -= work->cont_time;
      setup_mode(work, next);
    }

  if(!work->mode)
    {
      
      // work->alpha = 0;
      float trate, rate = 1.0F;

      trate = 1.0F - (float)work->cnt_time / work->cont_time;
      if(trate < 0.0F) rate = 0.0F;
      work->alpha = (int)((float)work->max_alpha * trate);
    }
  else
    {
      float trate, rate = 1.0F;
      trate = (float)work->cnt_time / (float)work->cont_time;
      if(trate < 0.1F) rate = trate * 10.0F;
      work->alpha = (int)((float)work->max_alpha * rate);
    }

  send_alpha(work);
}

static void Die(Work * work)
{
}

/*
 * パラメータ設定
 */
static void setup_param(Work * work)
{
  work->min_blight_time = DEF_BLINK_MIN_BLIGHT;
  work->max_blight_time = DEF_BLINK_MAX_BLIGHT;

  work->min_dark_time = DEF_BLINK_MIN_DARK;
  work->max_dark_time = DEF_BLINK_MAX_DARK;

  work->player_lookup = 0;

  if(GCL_GetOption('O'))   /* 'O'      */
    {
      work->base.vx = (float)GCL_GetNextInt();
      work->base.vy = (float)GCL_GetNextInt();
      work->base.vz = (float)GCL_GetNextInt();
      work->base.vw = 0.0F;
    }


  if(GCL_GetOption('R'))  /* 'Rot' */
    {
      int x, y, z;

      x = GCL_GetNextInt();
      y = GCL_GetNextInt();
      z = GCL_GetNextInt();

      if(x > 2048) x -= 4096;
      if(y > 2048) y -= 4096;
      if(z > 2048) z -= 4096;

      work->pole_rot.vx = (float)x * F_PI / 2048.0F;
      work->pole_rot.vy = (float)y * F_PI / 2048.0F;
      work->pole_rot.vz = (float)z * F_PI / 2048.0F;
    }

  work->max_alpha = 64;
  if(GCL_GetOption('a'))   /* 'alpha'  */
    work->max_alpha = GCL_GetNextInt();

  work->fix_alpha = work->max_alpha;

  if(GCL_GetOption('r'))   /* 'r'      */
    work->image_r = (float)GCL_GetNextInt();

  work->image_w = work->image_h = -1.0F;

  if(GCL_GetOption('w'))   /* 'width'  */
    work->image_w = (float)GCL_GetNextInt();
  else
    if(GCL_GetOption('i'))  /* 'inner' */
      {
	float inner;
	inner = (float)GCL_GetNextInt();
	work->image_w = work->image_r * inner * 2.0F * F_PI / 4096.0F;
      }

  if(GCL_GetOption('h'))   /* 'height' */
    work->image_h = (float)GCL_GetNextInt();
  
  if(GCL_GetOption('p'))   /* 'posy'   */
    work->image_y = (float)GCL_GetNextInt();

  if(GCL_GetOption('c'))   /* 'chr'    */
    {
      /* テキスト部の位置と面積を、枠に対する比で取得 */
      work->chr_x = (float)GCL_GetNextInt() / BASE_POINT;
      work->chr_y = (float)GCL_GetNextInt() / BASE_POINT;
      work->chr_w = (float)GCL_GetNextInt() / BASE_POINT;
      work->chr_h = (float)GCL_GetNextInt() / BASE_POINT;
    }

  if(GCL_GetOption('t'))   /* 'tex' */
    {
      work->tri_name  = GCL_GetNextInt();  /* tri 名称             */ 
      work->tex_frame = GCL_GetNextInt();  /* 枠用テクスチャ名称   */
      work->tex_chr   = GCL_GetNextInt();  /* 文字用テクスチャ名称 */
    }
  
  /*
   * アニメーションパラメータ
   */
  if(GCL_GetOption('s'))  /* 'spin' */
    {
      int s;
      s = GCL_GetNextInt() & 4095;
      if(s > 2048) s = -4096 + s;
      work->spin_init = (float)s * F_PI / 2048;
      work->spin_time = GCL_GetNextInt();
    }

  if(GCL_GetOption('v'))  /* vertical */
    {
      work->slide_start = (float)GCL_GetNextInt();
      work->slide_end   = (float)GCL_GetNextInt();
      work->slide_time  = GCL_GetNextInt();
    }

  if(GCL_GetOption('x'))  /* 'xzsize' */
    {
      work->scale_start = (float)GCL_GetNextInt();
      work->scale_end   = (float)GCL_GetNextInt();
      work->scale_time  =  GCL_GetNextInt();
    }

  if(GCL_GetOption('y'))  /* 'yscale' */
    {
      work->stretch_start = (float)GCL_GetNextInt();
      work->stretch_end   = (float)GCL_GetNextInt();
      work->stretch_time  = GCL_GetNextInt();
    }

  if(GCL_GetOption('u'))  /* 'uvanim' */
    {
      work->uv_time = GCL_GetNextInt();

      work->uv_start_u = (float)GCL_GetNextInt() / BASE_POINT;
      work->uv_start_w = (float)GCL_GetNextInt() / BASE_POINT;
      work->uv_start_v = (float)GCL_GetNextInt() / BASE_POINT;
      work->uv_start_h = (float)GCL_GetNextInt() / BASE_POINT;

      work->uv_end_u = (float)GCL_GetNextInt() / BASE_POINT;
      work->uv_end_w = (float)GCL_GetNextInt() / BASE_POINT;
      work->uv_end_v = (float)GCL_GetNextInt() / BASE_POINT;
      work->uv_end_h = (float)GCL_GetNextInt() / BASE_POINT;
    }

  work->tex_bg = -1;
  if(GCL_GetOption('T'))  /* 'bgTex' */
    work->tex_bg    = GCL_GetNextInt();  /* 背景用テクスチャ名称 */

  work->bg_time = 0;
  if(GCL_GetOption('b'))  /* 'bguv' */
    {
      work->bg_time = GCL_GetNextInt();

      work->bg_start_u = (float)GCL_GetNextInt() / BASE_POINT;
      work->bg_start_w = (float)GCL_GetNextInt() / BASE_POINT;
      work->bg_start_v = (float)GCL_GetNextInt() / BASE_POINT;
      work->bg_start_h = (float)GCL_GetNextInt() / BASE_POINT;

      work->bg_end_u = (float)GCL_GetNextInt() / BASE_POINT;
      work->bg_end_w = (float)GCL_GetNextInt() / BASE_POINT;
      work->bg_end_v = (float)GCL_GetNextInt() / BASE_POINT;
      work->bg_end_h = (float)GCL_GetNextInt() / BASE_POINT;
    } 

  if(GCL_GetOption('l'))  /* 'lookup' */
    work->player_lookup = 1;

  work->plate = 0;
  if(GCL_GetOption('L'))  /* 'pLate' */
    work->plate = 1;

  work->cnt_time = 0;
  setup_mode(work, 0);
}

/*
 * 子 Actor の起動。
 * 三つの cylinder を起動し、一つを枠、一つを文字,一つを背景に割り当てる。
 */
static void create_child(Work * work)
{
  CYLINDER_PARAM param;
  void * child_workp;
  float y_plus;

  param.plate    = work->plate;  /* 平版 */

  /* 起点は共通 */
  param.O        = work->base;       /* 起点設定       */
  param.rot      = work->pole_rot;   /* 中心軸傾き     */
  /*
   * 外枠のパラメータ設定
   * 基本的に、この actor のパラメータは外枠を基準にとる。
   */
  param.alpha    = work->max_alpha;  /* 最大アルファ値 */
  param.r        = work->image_r;
  param.width    = work->image_w;
  param.height   = work->image_h;
  param.posy     = work->image_y;
  param.tri_name = work->tri_name;
  param.tex_name = work->tex_frame;  /* 枠テクスチャ   */

  /* アニメーションパラメータ */
  param.tm_spin  = work->spin_time;
  param.mv_spin  = work->spin_init;

  param.tm_slide   = work->slide_time;
  param.mv_slStart = work->slide_start;
  param.mv_slEnd   = work->slide_end;

  param.tm_scale   = work->scale_time;
  param.mv_scStart = work->scale_start;
  param.mv_scEnd   = work->scale_end;

  param.tm_stretch = work->stretch_time;
  param.mv_stStart = work->stretch_start;
  param.mv_stEnd   = work->stretch_end;

  param.tm_uv      = 0;  /* 外枠は UV アニメーション無し */
  param.player_lookup = work->player_lookup;

  /* 外枠に対応する円柱イメージを作成(平板モード時は平板) */
  child_workp = NewCylinderImageP(&param);
  GV_SetActorChild(work, child_workp);   /* 子 Actor として登録 */

  /* ----------------------------------------------------------------------- */
  /*
   * 背景分のパラメータ設定。サイズは外枠と同じ。
   * 背景は,オプションとして -bgTex が指定されている場合のみ生成される。
   */
  if(work->tex_bg >= 0)  /* 背景が指定されている場合 */
    {
      param.tex_name = work->tex_bg;  /* 背景テクスチャ   */

      /* 与えられている 背景UVアニメーション値は、そのまま背景部に与えられる */
      if(work->bg_time > 0)
	{
	  param.tm_uv      = work->bg_time;
      
	  param.mv_uStart  = work->bg_start_u;
	  param.mv_vStart  = work->bg_start_v;
	  param.mv_uwStart = work->bg_start_w;
	  param.mv_vhStart = work->bg_start_h;
	  
	  param.mv_uEnd    = work->bg_end_u;
	  param.mv_vEnd    = work->bg_end_v;
	  param.mv_uwEnd   = work->bg_end_w;
	  param.mv_vhEnd   = work->bg_end_h;
	}
      /* 背景に対応する円柱イメージを生成 */
      child_workp = NewCylinderImageP(&param);
      GV_SetActorChild(work, child_workp);   /* 子 Actor として登録 */
    }

  /* ----------------------------------------------------------------------- */

  /*
   * 文字スクロール部のパラメータ設定
   *
   * 文字スクロール部のパラメータは、外枠を元に、若干加工したものを与える
   */
  /* 縦横の長さは、外枠の大きさにそれぞれの比をかけたもの */
  param.width  = work->image_w * work->chr_w;
  param.height = work->image_h * work->chr_h;

  /* 底の位置を上げてやる */
  y_plus = work->image_h - (work->image_h * (work->chr_y + work->chr_h));
  param.posy   = work->image_y + y_plus;

  /* テクスチャは文字用 */
  param.tex_name = work->tex_chr;

  /* アニメーションパラメータ */
  /* 軸廻り回転の初期角を、枠に対する x 位置に合わせて修正する */
  /* 初期角を、枠が描く弧の角度に対する比から求める */
  {
    float a, b, x;

    a = work->image_w / work->image_r;  /* 枠のしめる角度   */
    b = a * work->chr_w;                /* 文字のしめる角度 */
    x = a * work->chr_x;                /* 枠の端から
					   文字の開始される位置までの角度 */
    param.mv_spin -= x + (b - a) / 2.0F;   /* 中心位置の角度差を引く */
  }

  /* 軸沿い移動のパラメータは、底を上げた分を開始点、終了点に追加する */
  param.mv_slStart += y_plus;
  param.mv_slEnd   += y_plus;

  /* 半径変更のパラメータは変わらない */
  
  /* 縦方向引き伸ばしの場合は、開始高さ、終了高さに対し、
     それぞれ文字イメージの比をかける。 */
  param.mv_stStart *= work->chr_h;
  param.mv_stEnd   *= work->chr_h;

  /* 与えられている UVアニメーション値は、そのまま文字部に与えられる */
  param.tm_uv      = work->uv_time;

  param.mv_uStart  = work->uv_start_u;
  param.mv_vStart  = work->uv_start_v;
  param.mv_uwStart = work->uv_start_w;
  param.mv_vhStart = work->uv_start_h;

  param.mv_uEnd    = work->uv_end_u;
  param.mv_vEnd    = work->uv_end_v;
  param.mv_uwEnd   = work->uv_end_w;
  param.mv_vhEnd   = work->uv_end_h;

  /* 文字部に対応する円柱イメージを作成 */
  child_workp = NewCylinderImageP(&param);
  GV_SetActorChild(work, child_workp);   /* 子 Actor として登録 */
}

static int size_adjust(Work * work)
{
  DG_TEX * tex;
  float w, h;

  /* 外枠テクスチャの大きさを求め、それをもとに外枠のサイズを確定する */
  if((work->tri_name < 0) || (work->tex_frame < 0)) return -1;

  tex = DG_GetTexture2(work->tri_name, work->tex_frame);
  work->tx_w = (float)(1 << ((tex->tex_trans.tex0.data >> 26) & 0x0f));
  work->tx_h = (float)(1 << ((tex->tex_trans.tex0.data >> 30) & 0x0f));

  w = tex->u_scale * work->tx_w;
  h = tex->v_scale * work->tx_h;

  /* 幅、もしくは高さのいずれかが未定義であった場合、
     もう一方の値と縦横比を元に、未定義になっている側の値を計算する。 */
  if(work->image_w < 0.0F)
    /* 横幅が未定義なので、縦の高さから横幅を計算する */
    work->image_w = work->image_h * w / h;

  if(work->image_h < 0.0F)
    /* 高さが未定義なので、横幅から高さを計算する */
    work->image_h = work->image_w * h / w;

  return 0;
}

static int GetResources(Work * work, int name, int where)
{
  work->name = name;

  setup_param(work);

  if(size_adjust(work)) return -1;

  create_child(work);

  work->alpha = work->max_alpha;
  work->pre_alpha = work->alpha;
  return 0;
}

void * NewBlinkCylinderTexture(int name, int where)
{
  Work * work;

  if(NULL == (work = GV_NewActor(GV_ACTOR_USER, sizeof(Work)))) return NULL;

  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));

  if(GetResources(work, name, where))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  return work;
}
