//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  cylinder.c
  円筒配置イメージ

  2001/04/18  Yoshihito Kira
  $Id: cylinder.c,v 1.1.1.3 2002/11/19 11:43:52 Yoshizawa1 Exp $
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
#define _cylinder_c_
#include "cylinder.h"
#include "blink.h"

#ifdef __GNUC__
#ifdef _DEBUG_
#define DBG(args...)   printf(args)
#else
#define DBG(args...)
#endif /* DEBUG */
#else
#define DBG
#endif


/* スクラッチパッド先頭 */
#define SCR_POS   ((FVECTOR *)SCRPAD_ADDR)
#define FTOI12(_f)       ( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define MAX_SPD   40.0F

typedef struct {
  GV_ACT_EX         actor;
  int               name;     /* chara 名                             */

  /*
   * ユーザから与えられるパラメータ
   */
  FVECTOR           pre_player; /* 直前のプレイヤ位置(プレイヤ反応時のみ使用)*/
  FVECTOR           base;     /* 軸の起点                             */
  FVECTOR           pole_rot; /* 軸自体の傾きをあらわす回転角         */

  FMATRIX           mov_mat;  /* 移動アニメーション用マトリクス       */
  FMATRIX           pole_mat; /* 軸マトリクス                         */

  float             image_r;  /* イメージの軸からの半径               */
  float             image_w;  /* イメージの弧の長さ(幅)               */
  float             image_h;  /* イメージの高さ                       */
  float             image_y;  /* イメージを配置する起点からの y 位置  */
  int               tri_name; /* tri ファイル名 strcode               */
  int               tex_name; /* tri 内 texture 名称                  */
  
  /*
   * ユーザパラメータから導かれるパラメータ
   */
  DG_TEX          * dgtex;    /* テクスチャ                           */
  float             img_angle;/* イメージの弧の角度(image_w から計算) */
  int               part_num; /* イメージの分割趨(降り曲げる点の数)   */


  /* テクスチャの UV/サイズ値 */
  float             tx_w;     /* テクスチャ自体の大きさ               */
  float             tx_h;     /* テクスチャ自体の大きさ               */

  /* TRI 参照結果の UV 値 */
  float             org_u, org_v;
  float             org_w, org_h;

  /* 描画 UV 値 */
  float             u, v;     /* テクスチャの uv 値                   */
  float             uw, vh;   /* 使用するテクスチャのテクセル幅、高さ */

  float             u_rate;   /* 指定されたテクスチャに対するu値のレート */
  float             v_rate;   /* 指定されたテクスチャに対するv値のレート */
  float             uw_rate;  /* 指定されたテクセル幅に対するレート   */
  float             vh_rate;  /* 指定されたテクセル高さに対するレート */

  int               inner;    /* オプションで指定された内角(4096=2π[rad]) */

  /*
   * アニメーション関連のパラメータ
   */

  /* 軸回り回転アニメーション */
  float             an_spin;    /* 軸まわり回転角                     */

  float             add_spin;   /* 1フレームあたりの、現在の角速度    */
  float             tgt_spin;   /* 目標角速度                         */
  float             min_spin;   /* 最低角速度(動かなければこの絶対値で
				   ゆっくり回転)                      */

  /* 軸沿い移動アニメーション */
  float             an_slide;   /* 軸方向上下位置                     */
  float             mv_slStart; /* 上下位置始点                       */
  float             mv_slEnd;   /* 上下位置終点                       */
  int               tm_slide;   /* 上下所要時間(1/300秒単位)          */
  int               ct_slide;   /* 時間カウンタ                       */

  /* 半径拡大縮小アニメーション */
  float             an_scale;   /* 半径拡大レート                     */
  float             mv_scStart; /* 開始レート                         */
  float             mv_scEnd;   /* 終了レート                         */
  int               tm_scale;   /* 所要時間(1/300秒単位)              */
  int               ct_scale;   /* 時間カウンタ                       */

  /* 上下幅伸長 */
  float             an_stretch; /* 上下幅拡大縮小レート               */
  float             mv_stStart; /* 開始レート                         */
  float             mv_stEnd;   /* 終了レート                         */
  int               tm_stretch; /* 所要時間(1/300秒単位)              */
  int               ct_stretch; /* 時間カウンタ                       */


  /* テクスチャ UV アニメーション */
  float             mv_uStart;  /* 開始正規化 U 値                    */
  float             mv_vStart;  /* 開始正規化 V 値                    */
  float             mv_uwStart; /* 開始正規化 UW 値                   */
  float             mv_vhStart; /* 開始正規化 VH 値                   */

  float             mv_uEnd;    /* 開始正規化 U 値                    */
  float             mv_vEnd;    /* 開始正規化 V 値                    */
  float             mv_uwEnd;   /* 開始正規化 UW 値                   */
  float             mv_vhEnd;   /* 開始正規化 VH 値                   */

  int               tm_uv;      /* UV アニメーション所要時間          */
  int               ct_uv;      /* UV アニメーションカウンタ          */

  /*
   * プリミティブ関連
   */
  DG_PRIM2        * prim;             /* プリミティブ                 */

  int               n_verts;          /* 頂点数                       */
  int               n_prims;          /* プリミティブ数               */
  u_long64     alpha;            /* アルファブレンディング特性   */
  int               alpha_rate;       /* アルファ値                   */
  int               alpha_max;        /* 最大アルファ値               */
  int               alpha_fix;

  /* 点滅関連 */
  int               cnt_time;
  int               cont_time;

  int               min_blight_time;  /* 最短点灯時間                 */
  int               max_blight_time;  /* 最長点灯時間                 */

  int               min_dark_time;    /* 最短消灯時間                 */
  int               max_dark_time;    /* 最長消灯時間                 */

  int               player_lookup:1;  /* プレイヤー連動               */
  int               blink:1;          /* 自動点滅のフラグ             */
  int               mode:1;           /* 1:点灯 / 0:消灯              */

  int               plate:1;          /* 平板モード                   */

} Work;

#define DEF_IMAGE_r   3000.0F
#define DEF_IMAGE_w   -1.0F
#define DEF_IMAGE_h   -1.0F
#define DEF_IMAGE_y   0.0F

#define DEF_ALPHA_MAX 64

#define DEF_MAX_PART  32       /* 360度を覆う場合に、正何角形になるか */

#define POINT_BASE   ((float)1000) /* GCLで与えられた値をこの値で割ったものを、
				     正規化 UV 値として扱う */

#ifdef DEBUG
/*
 * デバッグ用マトリクス表示
 */
void mat_disp(FMATRIX * mat)
{
  int y;
  for(y = 0; y < 4; y++)
    DBG("| %8.3f %8.3f %8.3f %8.3f |\n",
	mat->m[0][y], mat->m[1][y], mat->m[2][y], mat->m[3][y]);
  DBG("\n");
}
#endif /* DEBUG */

/* スクラッチパッドからメインメモリへ転送 */
static void CpyScr2Mem(void * dst, void * src, int size, int num)
{
  UTL_StartSprToMem(dst, src, size * num / sizeof(u_long128));
  UTL_EndSprToMem();
}

static void CpyMem2Scr(void * dst, void * src, int size, int num)
{
  UTL_StartMemToSpr(dst, src, size * num / sizeof(u_long128));
  UTL_EndMemToSpr();
}

/*
 * 各フレーム毎の、UV値、アルファ値を設定する
 */
static void setup_UVanim(Work * work)
{
  DG_PRIM2 * prim = work->prim;
  DG_PRIM2_UVRGB * uvrgb = SCRPAD_ADDR;
  int clock;
  float u, v;
  float w, h;
  int i, num;

  clock = prim->buffer_clock;

  u = work->u + work->uw * work->u_rate;
  v = work->v + work->vh * work->v_rate;

  w = work->uw * work->uw_rate;
  h = work->vh * work->vh_rate;

  /* 現在の値をスクラッチパッドに展開 */
  CpyMem2Scr(uvrgb, prim->uvrgb[clock],
	     sizeof(DG_PRIM2_UVRGB), work->n_prims * work->n_verts);

  for(i = 0; i < work->part_num; i++)
    {
      num = (work->part_num - 1 - i) * 2;
      ASSERT((num < work->n_verts) && ((num + 1) < work->n_verts));

      /* UV 値の設定 */
      uvrgb[num].u = FTOI12(u + (w * (float)i) / (float)(work->part_num - 1));
      uvrgb[num + 1].u = uvrgb[num].u;
      uvrgb[num].v = FTOI12(v);
      uvrgb[num + 1].v = FTOI12(v + h);


      // uvrgb[num].q = uvrgb[num + 1].q = 4096;

      /* アルファ値の設定 */
      uvrgb[num].a = uvrgb[num + 1].a = work->alpha_rate;
    }

  /* 設定しおわったら、DG_PRIM2 に転送 */
  CpyScr2Mem(prim->uvrgb[clock], uvrgb,
	     sizeof(DG_PRIM2_UVRGB), work->n_prims * work->n_verts);
}

/*
 * 移動アニメーション
 */
static void anim_move(Work * work)
{
  /* 軸まわり回転 */
  work->an_spin += work->add_spin;

  /* 角度値を -π～πの間に収める */
  if(work->an_spin >= F_PI)
    work->an_spin = -2.0F * F_PI + work->an_spin;
  if(work->an_spin < -F_PI)
    work->an_spin = 2.0F * F_PI + work->an_spin;

  /* 軸沿い移動 */
  if(work->tm_slide)
    {
      work->ct_slide += TIME_BASE;
      if(work->ct_slide >= work->tm_slide) work->ct_slide -= work->tm_slide;

      work->an_slide =
	(work->mv_slEnd - work->mv_slStart) *
	(float)work->ct_slide / (float)work->tm_slide + work->mv_slStart;
    }
  
  
  /* 半径拡大縮小 */
  if(work->tm_scale)
    {
      float r;
      work->ct_scale += TIME_BASE;
      if(work->ct_scale >= work->tm_scale) work->ct_scale -= work->tm_scale;

      r = (work->mv_scEnd - work->mv_scStart) *
	(float)work->ct_scale / (float)work->tm_scale + work->mv_scStart;

      work->an_scale = r / work->image_r;
    }

  /* 上下幅伸長 */
  if(work->tm_stretch)
    {
      float h;

      work->ct_stretch += TIME_BASE;
      if(work->ct_stretch >= work->tm_stretch)
	work->ct_stretch -= work->tm_stretch;

      h = (work->mv_stEnd - work->mv_stStart) *
	(float)work->ct_stretch / (float)work->tm_stretch + work->mv_stStart;

      work->an_stretch = h / work->image_h;
    }
}


/*
 * テクスチャアニメーション
 */
static void anim_tex(Work * work)
{
  float u, v, uw, vh;
  float rate;

  /*
   * UV アニメーション
   */
  if(work->tm_uv)
    {
      if((work->ct_uv += TIME_BASE) >= work->tm_uv) work->ct_uv -= work->tm_uv;
      rate = (float)work->ct_uv / (float)work->tm_uv;
      u = (work->mv_uEnd - work->mv_uStart) * rate + work->mv_uStart;
      v = (work->mv_vEnd - work->mv_vStart) * rate + work->mv_vStart;
      uw = (work->mv_uwEnd - work->mv_uwStart) * rate + work->mv_uwStart;
      vh = (work->mv_vhEnd - work->mv_vhStart) * rate + work->mv_vhStart;
    }
  else
    {
      u = v = 0.0F;
      uw = vh = 1.0F;
    }

  /* 求めた正規化位置と、オリジナルの UV 値から、描画 UV 値を計算する */
  work->u = work->org_u + work->uw * u;
  work->v = work->org_v + work->vh * v;
  work->uw = work->org_w * uw;
  work->vh = work->org_h * vh;
}

static void set_scaling(FMATRIX * mat, float xs, float ys, float zs)
{
  int x, y;

  for(y = 0; y < 4; y++)
    for(x = 0; x < 4; x++)
      mat->m[x][y] = 0.0F;

  mat->m[0][0] = xs;
  mat->m[1][1] = ys;
  mat->m[2][2] = zs;
  mat->m[3][3] = 1.0F;
}


/*
 * 移動アニメーションマトリクスを作成する
 */
static void anim_matrix(Work * work)
{
  FMATRIX * mat = &(work->mov_mat);
  FMATRIX scale;
  int x, y;

  /*
   * 一旦マトリクスを初期化する
   */
  for(x = 0; x < 4; x++)
    for(y = 0; y < 4; y++) mat->m[x][y] = (float)(x == y);
  
  /* 軸回り回転角の設定 */
  _sceVu0RotMatrixY(mat, mat, work->an_spin); /* y軸廻りの回転角として与える */

  /* 軸沿い移動アニメーション */
  mat->m[3][1] = work->an_slide;         /* y 方向の平行移動値として与える */

  /* スケーリング値を設定したマトリクスを作成する */
  set_scaling(&scale, work->an_scale, work->an_stretch, work->an_scale);

  /* y 方向の平行移動値として、image_y を与える */
  scale.m[3][1] = work->image_y;

  _sceVu0MulMatrix(mat, &scale, mat);
}

static void calc_matrix(Work * work)
{
  DG_PRIM2 * prim = work->prim;

  /* アニメーションマトリクスに軸マトリクスをかけた結果を、
     プリミティブに設定 */
  prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
  _sceVu0MulMatrix(&(prim->as_world), &(work->pole_mat), &(work->mov_mat));
}

static void player_sync(Work * work)
{
  float sign = 1.0F;
  FVECTOR * mov;
  FVECTOR tmp;
  float len;
  float max, spd;

  if(!work->player_lookup) return;
  mov = &(GM_PlayerControl->mov);
  
  /* プレイヤが軸の起点より左側に居れば符号を反転する */
  if(mov->vx < work->base.vx) sign = -1.0F;

  /* 角速度は、プレイヤの移動量に比例させる */
  _sceVu0SubVector(&tmp, mov, &(work->pre_player));
  len = fpu_Sqrt(tmp.vx * tmp.vx + tmp.vz * tmp.vz);  /* xz平面上の移動距離 */
  max = work->min_spin * 3.0F;  /* 最大角速度は通常の三倍。但し赤くはない。 */
  spd = (max - work->min_spin) * len / MAX_SPD + work->min_spin;

  /* 求めた角速度に対し、符号を与え、目標角速度とする */
  work->tgt_spin = spd * sign;

  /* 現在の角速度と目標角速度の間に差がある場合、その差を補間する */
  work->add_spin = work->add_spin + (work->tgt_spin - work->add_spin) * 0.1F;

  /* 現在のプレイヤ位置を、次回フレームの直前プレイヤ位置として保存する */
  work->pre_player = *mov;
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
	    work->alpha_max = msg->message[1];
	    if(work->alpha_max > work->alpha_fix)
	      work->alpha_max = work->alpha_fix;
	  }
	  break;
	case 1:   /* アルファ値リセット */
	  work->alpha_max = work->alpha_fix;
	  break;
	}
    }
}

static void setup_blink_anim(Work * work)
{

  work->cnt_time += TIME_BASE;
  if(work->cnt_time >= work->cont_time)
    {
      int next;
      int min, r;

      next = (work->mode) ? 0 : 1;
      work->cnt_time -= work->cont_time;

      if(next)
	{
	  min = work->min_blight_time;
	  r = work->max_blight_time - min;
	}
      else
	{
	  min = work->min_dark_time;
	  r = work->max_blight_time - min;
	}
      work->mode = next; 
      work->cont_time = min + ((BP_PS2_rand() >> 16) % r);
    }

  if(!work->mode)
    {
      /* 消える場合はアルファ値を減衰させる */
      /* work->alpha_rate = 0; 旧来の処理はいきなり消すだけ */
      float trate, rate = 1.0F;

      trate = 1.0F - (float)work->cnt_time / work->cont_time;
      if(trate < 0.0F) rate = 0.0F;
      work->alpha_rate = (int)((float)work->alpha_max * trate);
    }
  else
    {
      float trate, rate = 1.0F;
      trate = (float)work->cnt_time / (float)work->cont_time;
      if(trate < 0.1F) rate = trate * 10.0F;
      work->alpha_rate = (int)((float)work->alpha_max * rate); 
    }
}

static void Act(Work * work)
{
  /* メッセージの処理 */
  message_proc(work);

  /* 点滅が指示されている場合、自動的に Alpha 値を操作して明滅を行う */
  if(work->blink)
    setup_blink_anim(work);
  else
    work->alpha_rate = work->alpha_max;

  /* DG_PRIM2 を Flip する */
  DG_SwitchBuffPrim2(work->prim);

  /* プレイヤに連動するのであれば、連動させる */
  player_sync(work);

  /* 移動アニメーション       */
  anim_move(work);

  /* テクスチャアニメーション */
  anim_tex(work);

  /* テクスチャの UV 値を指定 */
  setup_UVanim(work);

  /* アニメーション結果から、移動アニメーションマトリクスを作成 */
  anim_matrix(work);

  /* 軸マトリクスと移動アニメーションマトリクスから、
     表示マトリクスを生成してプリミティブに渡す */
  calc_matrix(work);
}

static void Die(Work * work)
{
  DBG("cylinder.c: Die\n");
  GM_FreePrim2(work->prim);
  DBG("cylinder.c: Die...success.\n");
}

/*
 * 初期化: デフォルト値の設定
 */
static void setup_default(Work * work)
{
  /* 軸のデフォルト(原点から地平面に対し垂直に延びる線) */
#ifdef __GNUC__	
  work->base     = (FVECTOR){0.0F, 0.0F, 0.0F, 1.0F};  /* 軸の起点   */
  work->pole_rot = (FVECTOR){0.0F, 0.0F, 0.0F, 1.0F};  /* 軸の回転角 */
#else
  work->pole_rot = work->base = DG_ZeroVector;
#endif  

  work->image_r = DEF_IMAGE_r;
  work->image_w = DEF_IMAGE_w;
  work->image_h = DEF_IMAGE_h;
  work->image_y = DEF_IMAGE_y;
  work->inner   = -1;

  work->tri_name = -1;  /* 現状では無し     */
  work->tex_name = -1;  /* 現状では指定無し */
 
  work->alpha      = SCE_GS_SET_ALPHA(0, 1, 0, 1, 0);
  work->alpha_rate = DEF_ALPHA_MAX;
  work->alpha_max  = DEF_ALPHA_MAX;
  work->alpha_fix  = DEF_ALPHA_MAX;

  work->u_rate = work->v_rate = 0.0F;
  work->uw_rate = work->vh_rate = 1.0F;


  /* 表示位置アニメーションパラメータの初期化 */
  work->an_spin = 0.0F;
  work->add_spin = work->tgt_spin = work->min_spin = 0.0F;
  work->an_slide = 0.0F;   work->tm_slide = 0;
  work->an_scale = 1.0F;   work->tm_scale = 0;
  work->an_stretch = 1.0F; work->tm_stretch = 0;

  /* UV アニメーションパラメータの初期化 */
  work->tm_uv = 0;

  /* 点滅アニメーションパラメータの初期化 */
  work->blink = 0;
  work->min_blight_time = DEF_BLINK_MIN_BLIGHT;
  work->max_blight_time = DEF_BLINK_MAX_BLIGHT;
  
  work->min_dark_time = DEF_BLINK_MIN_BLIGHT;
  work->max_dark_time = DEF_BLINK_MAX_BLIGHT;
  work->cont_time = work->min_blight_time;
  work->cnt_time = 0;   /* タイムカウンタのリセット */

  /* 挙動フラグの初期化 */
  work->player_lookup = 0;  /* プレイヤの動きを見て、方向や挙動を変える */
}

/*
 * 初期化: オプションによって指定されたパラメータの設定
 */
static void setup_option(Work * work)
{

  /* 中心軸の回転角 */
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

  /* 最大アルファ値の変更 */
  if(GCL_GetOption('a'))  /* 'alpha' */
    {
      work->alpha_fix = GCL_GetNextInt();
      work->alpha_max = work->alpha_fix;
      work->alpha_rate = work->alpha_max;
    }

  /* 自動ランダム点滅機能(仮) */
  if(GCL_GetOption('b'))  /* 'blink' */
    work->blink = 1;
  
  /* 起点の変更 */
  if(GCL_GetOption('O'))  /* 'O' */
    {
      work->base.vx = (float)GCL_GetNextInt();
      work->base.vy = (float)GCL_GetNextInt();
      work->base.vz = (float)GCL_GetNextInt();
    }

  /* 軸からイメージの半径指定 */
  if(GCL_GetOption('r'))  /* 'r' */
    work->image_r = (float)GCL_GetNextInt();

  /* イメージの弧の長さ */
  if(GCL_GetOption('w'))  /* 'width' */
    work->image_w = (float)GCL_GetNextInt();
  else
    /* イメージの弧の中心角(幅が指定されている場合は無視) */
    if(GCL_GetOption('i')) /* 'inner' */
      work->inner = GCL_GetNextInt();
  
  /* イメージの高さ */
  if(GCL_GetOption('h'))  /* 'height' */
    work->image_h = (float)GCL_GetNextInt();

  /* イメージの y 方向位置 */
  if(GCL_GetOption('p'))  /* 'posy'   */
    work->image_y = (float)GCL_GetNextInt();

  /* テクスチャの指定 */
  if(GCL_GetOption('t'))  /* 'tex' */
    {
      work->tri_name = GCL_GetNextInt();  /* tri ファイル名 */
      work->tex_name = GCL_GetNextInt();  /* texture 名     */
    }

  work->plate = 0;
  if(GCL_GetOption('L'))  /* 'pLate' */
    work->plate = 1;

  /*
   * アニメーションの設定
   */
  /* 軸廻り回転 */
  if(GCL_GetOption('s'))  /* 'spin' */
    {
      int st;
      int tm;

      st = GCL_GetNextInt();
      tm = GCL_GetNextInt();

      st &= 4095;
      if(st > 2048) st -= 4096;
      work->an_spin = (float)st * F_PI / 2048.0F; /* 初期角   */
      if(tm)
	work->min_spin = (2.0F * F_PI) / ((float)tm / (float)TIME_BASE);
      else
	work->min_spin = 0.0F;

      work->tgt_spin = work->add_spin = work->min_spin;
    }

  /* 軸沿い移動 */
  if(GCL_GetOption('v'))  /* 'vertical' */
    {
      work->mv_slStart = (float)GCL_GetNextInt();   /* 開始位置(起点相対) */
      work->mv_slEnd   = (float)GCL_GetNextInt();   /* 終了位置(起点相対) */
      work->tm_slide   = GCL_GetNextInt();          /* 移動所要時間       */
    }

  /* 半径変更 */
  if(GCL_GetOption('x'))  /* 'xzscale' */
    {
      work->mv_scStart = (float)GCL_GetNextInt();   /* 開始半径 */
      work->mv_scEnd   = (float)GCL_GetNextInt();   /* 終了半径 */
      work->tm_scale   = GCL_GetNextInt();          /* 移動所要時間       */
    }

  if(GCL_GetOption('y'))  /* 'yscale' */
    {
      work->mv_stStart = (float)GCL_GetNextInt();   /* 開始テクスチャ高さ */
      work->mv_stEnd   = (float)GCL_GetNextInt();   /* 終了テクスチャ高さ */
      work->tm_stretch = GCL_GetNextInt();          /* 所要時間           */
    }

  if(GCL_GetOption('u')) /* 'uvanim' */
    {
      work->ct_uv = 0;                    /* カウンタをリセット */
      work->tm_uv = GCL_GetNextInt();     /* 所要時間           */
      work->mv_uStart = (float)GCL_GetNextInt() / POINT_BASE;
      work->mv_uwStart = (float)GCL_GetNextInt() / POINT_BASE;
      work->mv_vStart = (float)GCL_GetNextInt() / POINT_BASE;
      work->mv_vhStart = (float)GCL_GetNextInt() / POINT_BASE;

      work->mv_uEnd = (float)GCL_GetNextInt() / POINT_BASE;
      work->mv_uwEnd = (float)GCL_GetNextInt() / POINT_BASE;
      work->mv_vEnd = (float)GCL_GetNextInt() / POINT_BASE;
      work->mv_vhEnd = (float)GCL_GetNextInt() / POINT_BASE;
    }

  if(GCL_GetOption('l'))    /* 'lookup' */
    work->player_lookup = 1;
}

/*
 * 初期化: テクスチャの用意
 */
static void setup_texture(Work * work)
{
  DG_TEX * tex;
  float w, h;

  work->dgtex = NULL;

  /* テクスチャがロードされていなければ何もしない */
  if((work->tri_name < 0) || (work->tex_name < 0)) return;

  tex = work->dgtex = DG_GetTexture2(work->tri_name, work->tex_name);
  if(NULL == tex) return;

  /* テクスチャ自体の幅、高さを得る */
  work->tx_w = (float)(1 << ((tex->tex_trans.tex0.data >> 26) & 0x0f));
  work->tx_h = (float)(1 << ((tex->tex_trans.tex0.data >> 30) & 0x0f));

  work->org_u = tex->u_offset;
  work->org_v = tex->v_offset;
  work->org_w = tex->u_scale;
  work->org_h = tex->v_scale;

  w = tex->u_scale * work->tx_w;
  h = tex->v_scale * work->tx_h;

  /* 幅と高さが未定義である場合、内角と半径から幅を求める。*/
  if((work->image_w < 0.0F) && (work->inner >= 0))
    {
      ASSERT(work->inner > 0);
      work->image_w = work->image_r *
	(float)work->inner * 2.0F * F_PI / 4096.0F;
      /* あとは、以下の処理で縦の高さは時動的に求められる */
    }

  /* 幅、もしくは高さのいずれかが未定義であった場合、
     もう一方の値と縦横比を元に、未定義になっている側の値を計算する。 */
  if(work->image_w < 0.0F)
    /* 横幅が未定義なので、縦の高さから横幅を計算する */
    work->image_w = work->image_h * w / h;

  if(work->image_h < 0.0F)
    /* 高さが未定義なので、横幅から高さを計算する */
    work->image_h = work->image_w * h / w;
}

/*
 * 初期化: プリミティブの用意
 */
static void setup_prim(Work * work)
{
  /* イメージの幅と、軸からの半径から、弧の内角を求める */

  if(!work->plate)
    {
      /* 角度を [rad] で扱っているので計算が楽。*/
      work->img_angle = work->image_w / work->image_r;
      
      /* 弧の内角から、テクスチャの分割数を求める。
	 具体的には、両端と途中の頂点を合わせた数。 */
      work->part_num = (int)(work->img_angle * (float)DEF_MAX_PART * 2.0f / F_PI ) + 1;
      if(work->part_num < 2) work->part_num = 2;
      if(work->part_num > DEF_MAX_PART) work->part_num = DEF_MAX_PART;
      
    }
  else
    {
      /* 平板モードの場合は,指定された幅と高さを持つ平面を
	 円筒の接面の一部として描画する */

      work->img_angle = 0;
      work->part_num = 2;  /* 平板なので、分割数は 1 (2頂点x1対 = 4頂点) */

    }

  work->n_verts = work->part_num * 2;  /* 頂点数は、分割数の2倍 */
  work->n_prims = 1;
  DBG("n_prims = %d,  n_verts = %d\n", work->n_prims, work->n_verts);

  work->prim = GM_MakePrim2(DG_PRIM2_POLY | DG_PRIM2_ALPHA |
			    DG_PRIM2_SHADE | DG_PRIM2_TEX ,
			    work->n_prims, work->n_verts);
  DG_SetPrim2Alpha(work->prim, work->alpha);

  /* テクスチャの設定 */
  if(NULL != work->dgtex)
    {
      DBG("Texture SET!!!!!!!!!!!!!!!!!!!!\n");
      DG_ConfigPrim2Tex(work->prim, work->dgtex);
    }
}

/*
 * 初期化: 頂点の初期化
 */
static void setup_vertex(Work * work)
{
  DG_PRIM2 * prim = work->prim;
  FVECTOR * pos;
  DG_PRIM2_UVRGB * uvrgb;
  float yh, yl, x, z, rad, half;
  int i, v, clock;

  pos = (FVECTOR *)SCRPAD_ADDR;
  uvrgb = (DG_PRIM2_UVRGB *)(pos + 512);
  yl = 0.0F;
  yh = work->image_h;
  
  /*
   * 頂点数と、弧の角度、半径から、各頂点の座標値を求める
   */
  if(!work->plate)
    {
      /* 円弧モード */
      half = work->img_angle / 2.0F;
      
      
      for(clock = 0; clock < 2; clock++)
	{
	  DBG("n_prims * n_verts = %d\n", work->n_prims * work->n_verts);
	  /* 一旦スクラッチパッドに読み出す */
	  CpyMem2Scr(pos, prim->pos[clock],
		     sizeof(FVECTOR), work->n_prims * work->n_verts);
	  CpyMem2Scr(uvrgb, prim->uvrgb[clock],
		     sizeof(DG_PRIM2_UVRGB), work->n_prims * work->n_verts);
	  
	  DBG("part_num = %d\n", work->part_num);
	  for(i = 0; i < work->part_num; i++)
	    {
	      v = i * 2;
	      
	      ASSERT((v < work->n_verts) && ((v + 1) < work->n_verts));
	      
	      rad = work->img_angle * (float)i /
		(float)(work->part_num - 1) - half;
	      
	      x = work->image_r * vu0_Sin(rad);
	      z = work->image_r * vu0_Cos(rad);
	      
	      pos[v].vx = pos[v + 1].vx = x;
	      pos[v].vz = pos[v + 1].vz = z;
	      pos[v].vy = yh;
	      pos[v + 1].vy = yl;
	      pos[v].vw = pos[v + 1].vw = 1.0F;
	      
	      uvrgb[v].r = uvrgb[v].g = uvrgb[v].b = 128;
	      uvrgb[v + 1].r = uvrgb[v + 1].g = uvrgb[v + 1].b = 128;
	      uvrgb[v].f = uvrgb[v + 1].f = 0x0fff;
	      uvrgb[v].q = uvrgb[v + 1].q = 4096;
	      
	      /* UV 値、alpha 値については、ここでは決定しない */
	    }
	  /* スクラッチパッドから、座標値、属性をコピーする */
	  CpyScr2Mem(prim->pos[clock], pos,
		     sizeof(FVECTOR), work->n_prims * work->n_verts);
	  CpyScr2Mem(prim->uvrgb[clock], uvrgb,
		     sizeof(DG_PRIM2_UVRGB), work->n_prims * work->n_verts);
	}
    }
  else
    {
      printf("n_prims = %d,  n_verts = %d\n", work->n_prims, work->n_verts);
      /* 平板モード */
      for(clock = 0; clock < 2; clock++)
	{
	  DBG("n_prims * n_verts = %d\n", work->n_prims * work->n_verts);
	  /* 一旦スクラッチパッドに読み出す */
	  CpyMem2Scr(pos, prim->pos[clock],
		     sizeof(FVECTOR), work->n_prims * work->n_verts);
	  CpyMem2Scr(uvrgb, prim->uvrgb[clock],
		     sizeof(DG_PRIM2_UVRGB), work->n_prims * work->n_verts);
	  
	  x = work->image_w / 2;
	  z = work->image_r;

	  pos[0].vx = pos[1].vx = -x;
	  pos[2].vx = pos[3].vx = x;

	  pos[0].vz = pos[1].vz = pos[2].vz = pos[3].vz = z;
	  pos[0].vy = pos[2].vy = yh;
	  pos[1].vy = pos[3].vy = yl;
	  pos[0].vw = pos[1].vw = pos[2].vw = pos[3].vw = 1.0F;


	  uvrgb[0].r = uvrgb[0].g = uvrgb[0].b = 128;
	  uvrgb[1].r = uvrgb[1].g = uvrgb[1].b = 128;
	  uvrgb[2].r = uvrgb[2].g = uvrgb[2].b = 128;
	  uvrgb[3].r = uvrgb[3].g = uvrgb[3].b = 128;

	  uvrgb[0].f = uvrgb[1].f = uvrgb[2].f = uvrgb[3].f = 0xfff;
	  uvrgb[0].q = uvrgb[1].q = uvrgb[2].q = uvrgb[3].q = 4096;

	  /* スクラッチパッドから、座標値、属性をコピーする */
	  CpyScr2Mem(prim->pos[clock], pos,
		     sizeof(FVECTOR), work->n_prims * work->n_verts);
	  CpyScr2Mem(prim->uvrgb[clock], uvrgb,
		     sizeof(DG_PRIM2_UVRGB), work->n_prims * work->n_verts);
	}
    }
}

/*
 * 初期化: 挙動マトリクスの初期化
 */
static void setup_matrix(Work * work)
{
  int x, y;

  /* 回転、平行移動、スケーリング無しのマトリクスを作成 */
  for(y = 0; y < 4; y++)
    for(x = 0; x < 4; x++)
      work->mov_mat.m[x][y] = (float)(x == y);
}

/*
 * 初期化: 軸位置マトリクスの初期化
 */
static void setup_pole(Work * work)
{
  int x, y;

  /* 回転、平行移動、スケーリング無しのマトリクスを作成 */
  for(y = 0; y < 4; y++)
    for(x = 0; x < 4; x++)
      work->pole_mat.m[x][y] = (float)(x == y);

  /* 軸の回転に合わせてマトリクスを回転させる */
  _sceVu0RotMatrix(&(work->pole_mat), &(work->pole_mat), &(work->pole_rot));
  
  /* 軸の起点位置に合わせてマトリクスを平行移動する */
  _sceVu0TransMatrix(&(work->pole_mat), &(work->pole_mat), &(work->base));

  /* 軸位置は決定したので、今後は操作しない */
}

/*
 * シグナルの反応
 */
static int RecieveSignal(void * workp, int signal, int value)
{
  Work * work = workp;
  int ret = 0;
  switch(signal)
    {
    case CYLINDER_ALPHA_SIGNAL:
      /*
       * アルファ値変更シグナル
       */
      work->alpha_max = value;
      if(work->alpha_max > work->alpha_fix)
	work->alpha_max = work->alpha_fix;
      break;

#if 0
    case CYLINDER_SPIN_CHANGE_SIGNAL:
      /*
       * 回転方向変更シグナル
       */
      
      
      break;
#endif
    default:
      ret = GV_DefaultSignalFunc(workp, signal, value);
      break;
    }
  return ret;
}

static int GetResources(Work * work, int name, int where)
{
  work->name = name;
  
  DBG("<0>\n");
  setup_default(work);  /* デフォルト値の設定             */
  DBG("<1>\n");
  setup_option(work);   /* ユーザオプション値による上書き */
  DBG("<2>\n");
  setup_texture(work);  /* テクスチャの用意               */
  DBG("<3>\n");
  setup_prim(work);     /* プリミティブの生成             */
  DBG("<4>\n");
  setup_vertex(work);
  DBG("<5>\n");
  setup_matrix(work);   /* 移動マトリクスの初期化         */
  DBG("<6>\n");
  setup_pole(work);     /* 軸マトリクス                   */
  DBG("<7>\n");
  setup_UVanim(work);

  /* 親 Actor がつくものとして、シグナルを受け取るハンドルを設定する */
  GV_SetActorSignalFunc(work, RecieveSignal);

  return 0;
}

/*
 * プログラムインタフェース用パラメータ設定
 */
static void setup_param(Work * work, CYLINDER_PARAM * param)
{
  /* 最大アルファ値 */
  if(param->alpha >= 0) work->alpha_max = param->alpha;

  work->base    = param->O;      /* 起点ベクトル          */
  work->pole_rot = param->rot;   /* 中心軸回転角          */
  work->image_r = param->r;      /* 半径指定              */
  work->image_w = param->width;  /* イメージ幅(弧の長さ)  */
  work->image_h = param->height; /* イメージの高さ        */
  work->image_y = param->posy;   /* イメージの y 方向位置 */

  /* テクスチャ設定 */
  work->tri_name = param->tri_name;
  work->tex_name = param->tex_name;

  /*
   * アニメーション設定
   */
  /* 軸まわり回転 */
  work->an_spin = param->mv_spin;  /* 初期角を現在の角度に設定        */
  if(param->tm_spin)
    work->min_spin = 2.0F * F_PI / ((float)param->tm_spin / (float)TIME_BASE);
  else
    work->min_spin = 0.0F;
  work->tgt_spin = work->add_spin = work->min_spin;

  /* 軸沿い移動 */
  work->tm_slide   = param->tm_slide;  /* 所要時間 */
  work->ct_slide   = 0;
  work->mv_slStart = param->mv_slStart;   /* 開始位置(起点相対) */
  work->mv_slEnd   = param->mv_slEnd;     /* 終了位置(起点相対) */

  /* 半径変更    */
  work->tm_scale   = param->tm_scale;   /* 所要時間 */
  work->ct_scale   = 0;
  work->mv_scStart = param->mv_scStart; /* 開始半径 */
  work->mv_scEnd   = param->mv_scEnd;   /* 終了半径 */

  /* イメージ高さ変更 */
  work->tm_stretch = param->tm_stretch; /* 所要時間 */
  work->ct_stretch = 0;
  work->mv_stStart = param->mv_stStart; /* 開始高さ */
  work->mv_stEnd   = param->mv_stEnd;   /* 終了高さ */

  /* UVアニメ */
  work->tm_uv       = param->tm_uv;      /* 所要時間 */
  work->ct_uv       = 0;
  work->mv_uStart   = param->mv_uStart; 
  work->mv_vStart   = param->mv_vStart;  
  work->mv_uwStart  = param->mv_uwStart; 
  work->mv_vhStart  = param->mv_vhStart; 

  work->mv_uEnd     = param->mv_uEnd;
  work->mv_vEnd     = param->mv_vEnd;
  work->mv_uwEnd    = param->mv_uwEnd;
  work->mv_vhEnd    = param->mv_vhEnd;

  work->player_lookup = param->player_lookup;
  work->plate         = param->plate;
}

static int GetResourcesP(Work * work, CYLINDER_PARAM * param)
{
  work->name = -1;
  
  DBG("<0>\n");
  setup_default(work);  /* デフォルト値の設定             */
  DBG("<1>\n");
  setup_param(work, param);  /* プログラムインタフェースに
				与えられたパラメータの設定 */
  DBG("<2>\n");
  setup_texture(work);  /* テクスチャの用意               */
  DBG("<3>\n");
  setup_prim(work);     /* プリミティブの生成             */
  DBG("<4>\n");
  setup_vertex(work);
  DBG("<5>\n");
  setup_matrix(work);   /* 移動マトリクスの初期化         */
  DBG("<6>\n");
  setup_pole(work);     /* 軸マトリクス                   */
  DBG("<7>\n");
  setup_UVanim(work);

  /* 親 Actor がつくものとして、シグナルを受け取るハンドルを設定する */
  GV_SetActorSignalFunc(work, RecieveSignal);
  
  return 0;
}

void * NewCylinderImageP(CYLINDER_PARAM * param)
{
  Work * work;

  if(NULL == (work = GV_NewActor(GV_ACTOR_USER, sizeof(Work)))) return NULL;

  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));

  if(GetResourcesP(work, param))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  return work;
}


void * NewCylinderImage(int name, int where)
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
