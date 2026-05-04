//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  spin_model.c
  軸回転モデル

  2001/07/30  Yoshihito Kira
  $Id: spin_model.c,v 1.1.1.3 2002/11/19 11:43:52 Yoshizawa1 Exp $
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
#define _spin_model_c_
#include "cylinder.h"
#include "spin_model.h"
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

  /*
   * 軸の基底点に対する,モデルの Z 方向オフセット
   */
  float             model_z;  /* z方向のオフセット値(cylinderの高さに当たる)*/
  
  
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

  /*
   * モデル関連
   */
  OBJECT            obj;        /* 表示するモデルのオブジェクト構造体 */
  int               model;      /* 表示するモデル名称                 */


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
    for(y = 0; y < 4; y++)
      scale.m[x][y] = mat->m[x][y] = (float)(x == y);
  
  /* 軸回り回転角の設定 */
  _sceVu0RotMatrixZ(mat, mat, work->an_spin); /* z軸廻りの回転角として与える */

  /* 軸沿い移動アニメーション */
  mat->m[3][2] = work->an_slide;         /* z 方向の平行移動値として与える */

  /* z 方向の平行移動値として、model_z を与える */
  scale.m[3][2] = work->model_z;

  _sceVu0MulMatrix(mat, &scale, mat);
}

static void calc_matrix(Work * work)
{
#ifdef __GNUC__	
  SVECTOR rot = (SVECTOR){0, 0, 0, 0};
  FVECTOR mov = (FVECTOR){0.0F, 0.0F, 0.0F, 0.0F};
#else
  SVECTOR rot = {0, 0, 0, 0};
  FVECTOR mov = DG_ZeroVector;
#endif  
  OBJECT * obj = &(work->obj);
  FMATRIX mat;
  int i;
  
  /* アニメーションマトリクスに軸マトリクスをかけた結果を、
     オブジェクトを構成するパーツに設定してまわる */
  _sceVu0MulMatrix(&mat, &(work->pole_mat), &(work->mov_mat));
  DG_SetPos2(&mov, &rot);
  GM_ActObject2(obj);
  _sceVu0MulMatrix(&mat, &obj->objs->world, &mat);

  obj->objs->world = mat;

  /* 回転マトリクスを, オブジェクトのマトリクスにかける */
  // _sceVu0MulMatrix(&obj->objs->world, &obj->objs->world, &mat);

  /*
  for(i = 0; i < obj->objs->def->n_x_models; i++)
    obj->objs->objs[i].world = mat;
  */
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
	  break;
	case 1:   /* アルファ値リセット */
	  break;
	}
    }
}

static void Act(Work * work)
{
  /* メッセージの処理 */
  message_proc(work);

  /* プレイヤに連動するのであれば、連動させる */
  player_sync(work);
  
  /* 移動アニメーション       */
  anim_move(work);

  /* アニメーション結果から、移動アニメーションマトリクスを作成 */
  anim_matrix(work);

  /* 軸マトリクスと移動アニメーションマトリクスから、
     表示マトリクスを生成してプリミティブに渡す */
  calc_matrix(work);
}

static void Die(Work * work)
{
  DBG("cylinder.c: Die\n");
  GM_FreeObject(&(work->obj));
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
  work->base     = DG_ZeroVector;  /* 軸の起点   */
  work->pole_rot = DG_ZeroVector;  /* 軸の回転角 */
#endif
  
  /* 表示位置アニメーションパラメータの初期化 */
  work->an_spin = 0.0F;
  work->add_spin = work->tgt_spin = work->min_spin = 0.0F;
  work->an_slide = 0.0F;   work->tm_slide = 0;

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
  work->model = 0;

  /* モデルの指定 */
  if(GCL_GetOption('m'))   /* 'model' */
    work->model = GCL_GetNextInt();  /* モデル名称 */
    
  ASSERT(work->model);

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

  /* モデルの z 方向位置 */
  if(GCL_GetOption('p'))  /* 'posz'   */
    work->model_z = (float)GCL_GetNextInt();

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

  if(GCL_GetOption('l'))    /* 'lookup' */
    work->player_lookup = 1;
}

/*
 * 初期化: プリミティブの用意
 */
#define OBJ_FLAGS (DG_FLAG_ONEPIECE|DG_FLAG_PAINT)

static void setup_model(Work * work)
{
  GM_InitObject(&(work->obj), work->model, OBJ_FLAGS);
  DG_MakePreshade(work->obj.objs, GM_GetMap(GM_CurrentMap)->light);
  
  /* 余分なデータは NULL にする */
  work->obj.evmobj = NULL;
  work->obj.m_ctrl = NULL;

  /*
   * その他初期化
   */
  
  
  
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
      break;

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
  setup_model(work);    /* モデルの生成             */
  DBG("<3>\n");
  setup_matrix(work);   /* 移動マトリクスの初期化         */
  DBG("<4>\n");
  setup_pole(work);     /* 軸マトリクス                   */

  /* 親 Actor がつくものとして、シグナルを受け取るハンドルを設定する */
  GV_SetActorSignalFunc(work, RecieveSignal);

  return 0;
}

/*
 * プログラムインタフェース用パラメータ設定
 */
static void setup_param(Work * work, CYLINDER_PARAM * param)
{
  work->base     = param->O;      /* 起点ベクトル          */
  work->pole_rot = param->rot;   /* 中心軸回転角          */
  work->model_z  = param->posy;   /* イメージの y 方向位置 */

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

  work->player_lookup = param->player_lookup;
}

static int GetResourcesP(Work * work, CYLINDER_PARAM * param)
{
  work->name = -1;
  
  DBG("<0>\n");
  setup_default(work);  /* デフォルト値の設定             */
  DBG("<1>\n");
  setup_param(work, param);  /* プログラムインタフェースに
				与えられたパラメータの設定 */
  DBG("<3>\n");
  setup_model(work);     /* プリミティブの生成             */
  DBG("<4>\n");
  setup_matrix(work);   /* 移動マトリクスの初期化         */
  DBG("<6>\n");
  setup_pole(work);     /* 軸マトリクス                   */

  /* 親 Actor がつくものとして、シグナルを受け取るハンドルを設定する */
  GV_SetActorSignalFunc(work, RecieveSignal);
  
  return 0;
}

void * NewSpinModelP(CYLINDER_PARAM * param)
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


void * NewSpinModel(int name, int where)
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
