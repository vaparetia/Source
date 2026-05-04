//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  floor.c
  プレイヤがいる周辺の床が光る

  2001/04/09  Yoshihito Kira
  $Id: floor.c,v 1.1.1.3 2002/11/19 11:43:53 Yoshizawa1 Exp $
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
#include "debugmenu.h"
#include "g_struct.h"

#include "font.h"
#include "libfs.h"
#include "libhzx.h"
#include "libdg.h"
#include "libmt.h"
#include "libutl.h"
#include "utl_dma.h"
#include "vu0_pack.h"
#include "fpu_pack.h"
#include "fmt_demo.h"
#include "dmapack.h"
#include "def_dma.h"
#define _floor_c_
#include "floor.h"

#ifdef __GNUC__
#ifdef DEBUG_MODEb 
#define DBG(args...)   printf(args)
#else
#define DBG(args...)
#endif /* DEBUG */
#else

#define DBG

#endif

/* スクラッチパッド先頭 */
#define SCR_POS   ((FVECTOR *)SCRPAD_ADDR)




/* -------------------------------------------------------------------------
   デフォルト値
  ------------------------------------------------------------------------- */
#define N_VERTS  10   /* 頂点数                   */
#define N_PRIMS  128  /* プリミティブ数           */

#define P_PRIMS  64   /* プレイヤ用プリミティブ数 */

#define DEF_MAX_TIME  300
#define DEF_MAX_ALPHA 16
#define DEF_HEX_SIZE  200      /* デフォルトの正六角形外接円の半径 */
#define DEF_MAX_POINT   6      /* デフォルトの発生点数             */
#define DEF_SPAN       15

#define DEF_COLOR_R   100
#define DEF_COLOR_G   255
#define DEF_COLOR_B   128

/* -------------------------------------------------------------------------
   
   ------------------------------------------------------------------------- */
#define MIN_LIFE  4   /* 最短寿命(最小頂点数) */

#define X_STEP  (SQRT_3 / 2.0F)
#define Y_STEP  (0.5F)
#if defined(PSX2)
#define RAND(_m)    ((BP_PS2_rand() >> 16) % (_m))
#else
#define RAND(_m)    ((irnd() >> 16) % (_m))
#endif
#define AlphaFunc(alpha, t_cnt, max_time)  \
                ((alpha) *       \
		 ((max_time) - (t_cnt)) / (max_time) * \
		 ((max_time) - (t_cnt)) / (max_time) * \
		 ((max_time) - (t_cnt)) / (max_time))

#define NEW_LIMIT()  (20 + RAND(17))

/* -------------------------------------------------------------------------
   
   ------------------------------------------------------------------------- */
typedef struct {
  int   alpha;
  int   t_cnt;
  int   life;
} AlphaBuf;

typedef struct {
  GV_ACT_EX       actor;

  /* 未使用領域の先頭は、(use_begin + act_num) % n_prims になる。    */

  /*
   * 六角描画関連
   */
  /* 共通パラメータ */
  unsigned char   r, g, b;     /* 6角形の色                          */
  unsigned char   max_alpha;   /* 最大アルファ値                     */
  int             span;        /* 0～この値未満の乱数値が 0 でないかぎり
				  あらたな線の追加を行わない */
  float            hex_r;      /* 六角形の大きさ(外接円の半径)       */
  int             max_time;    /* 減衰所要時間                       */
  int             max_point;   /* 一箇所の Hex からの、最大発生原数  */
  int             n_verts;     /* 頂点数                             */

  FVECTOR         center;      /* リージョン中心                     */
  float           region_r;    /* リージョン半径                     */

  /* プレイヤ用パラメータ */
  int             p_prims;     /* プリミティブ数                     */
  int             p_act_num;   /* 表示中線の本数                     */
  int             p_begin;     /* 使用開始点                         */


  /* 敵兵その他用パラメータ */
  int             n_prims;     /* プリミティブ数                     */
  int             act_num;     /* 表示中の線の本数                   */
  int             use_begin;   /* 使用済領域開始点                   */


  FVECTOR          add[6];     /* 六各形の中心から、各頂点までの相対ベクトル */

  AlphaBuf       * alpha_list; /* 時間経過に伴う時間カウンタと
				  アルファ最大値の記録               */

  AlphaBuf       * player_alpha; /* 時間経過に伴う時間カウンタと
				    アルファ最大値の記録(プレイヤ用) */


  DG_PRIM2       * prim;       /* LineStrip 用 DG_PRIM2              */
  HEX_FLOOR_INFO * info;       /* 足元に亀甲を発生させる CONTROL のリスト */

  DG_PRIM2       * player_prim; /* プレイヤ用 DG_PRIM2              */
  HEX_FLOOR_INFO   player;      /* プレイヤ用 info                  */
 
  int              region_check:1;  /* リージョンチェックを
				       するかどうかのフラグ */
  int              floor_check:2;   /* 床チェックをするかどうかのフラグ */
} Work;


typedef struct {
  int next_id;
  float x_step;
  float y_step;
} MvInfo;

static MvInfo mv_list[6][3] = {
  /* 0 */ {{5, -X_STEP,  Y_STEP}, {1,  X_STEP,  Y_STEP}, {3, 0.0F, -1.0F}},
  /* 1 */ {{0, -X_STEP, -Y_STEP}, {2,  0.0F,    1.0F  }, {4, X_STEP, -Y_STEP}},
  /* 2 */ {{1,  0.0F,   -1.0F  }, {3, -X_STEP,  Y_STEP}, {5, X_STEP, Y_STEP}},
  /* 3 */ {{2,  X_STEP, -Y_STEP}, {4, -X_STEP, -Y_STEP}, {0, 0.0F,   1.0F}},
  /* 4 */ {{3,  X_STEP,  Y_STEP}, {5,  0.0F,   -1.0F  }, {1, -X_STEP, Y_STEP}},
  /* 5 */ {{4,  0.0F,    1.0F  }, {0,  X_STEP, -Y_STEP}, {2, -X_STEP, -Y_STEP}}
};

static Work * now_work = NULL;

#ifdef DEBUG_MODE
static int _DEBUG_hexfloor = 1;  /* デフォルトは ON */

static GM_DEBUG_MENU debug_menu = {
#ifdef __GNUC__
  class:  "EFFECT",
  menu:   "HEXAGON PLASMA",
  max:    2,
  items:  (char *[]){ "ON", "OFF" },
  values: (int[]){1, 0},
  target: &_DEBUG_hexfloor,
  mask:   0x00000001
#else
  NULL,
  "EFFECT",
  "HEXAGON PLASMA",
  NULL, //(char *[]){"ON", "OFF" },
  NULL, //(int[]){1, 0},
  &_DEBUG_hexfloor,
  0x00000001,
  NULL,
  0,
  0,
  2,
  0,
#endif  
};

#endif /* DEBUG_MODE */

/* -------------------------------------------------------------------------
   公開関数
   ------------------------------------------------------------------------- */
int HEX_RegistControlInfo(HEX_FLOOR_INFO * info, CONTROL * ctrl)
{
  Work * work = now_work;

  DBG("regist trying...\n");
  if(NULL == work) return -1;
  DBG("regist!!\n");
  info->ctrl = ctrl;
  info->pre_mov = ctrl->mov;  /* 位置をコピー */
  info->flags = 0;   /* フラグをクリア(表示状態に) */

  info->prev = NULL;
  info->next = work->info;
  if(NULL != work->info)
    work->info->prev = info;

  work->info = info;
  return 0;
}

int HEX_RemoveControlInfo(HEX_FLOOR_INFO * info)
{
  Work * work = now_work;

  if(NULL == work) return -1;

  if(info->prev != NULL) info->prev->next = info->next;
  if(info->next != NULL) info->next->prev = info->prev;
  if(work->info == info) work->info = info->next;
  return 0;
}

void HEX_InitFloor(void)
{
  Work * work = now_work;
  if(NULL == work) return;

  /* 使われていないことにする */
  work->p_begin = work->use_begin = 0;
  work->act_num = work->p_act_num = 0;
}

/* -------------------------------------------------------------------------
   
   ------------------------------------------------------------------------- */

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
 * CONTROL 構造体の mov が指す位置の足元を光らせる
 */
static void blight_footprint(Work * work, HEX_FLOOR_INFO * info,
			     DG_PRIM2 * prims, AlphaBuf * alpha_list,
			     int * n_prims, int * act_num, int * use_begin,
			     int floor_check)
{
  CONTROL * ctrl = info->ctrl;
  FVECTOR * pos = &(ctrl->mov);
  FVECTOR mov;
  FVECTOR center;
  FVECTOR point;
  FVECTOR add;
  DG_PRIM2 * prim = prims;
  int max;
  int points;   /* 発生源数   */
  int root;     /* 発生源番号 */
  int alpha;    /* アルファ値 */
  int life;     /* 寿命       */
  int i, j;
  int r;
  int idx, clock;
  int cx, cz;
  float fx;
  FVECTOR * scr_vec = SCRPAD_ADDR;    /* 上半分はベクトル用 */
  DG_PRIM2_UVRGB * scr_rgb = (DG_PRIM2_UVRGB *)(scr_vec + 512);  /* 下半分は属性用     */
  FVECTOR        * pr_pos;
  DG_PRIM2_UVRGB * pr_rgb;
  float bound_min_x = 1.0F, bound_max_x = -1.0F;
  float bound_min_z = 1.0F, bound_max_z = -1.0F;



#ifdef DEBUG_MODE
  /* デバッグメニューで床チェックが禁止されている場合は,
     強制的にチェックをしないようにする */
  if(!_DEBUG_hexfloor) floor_check = 0;
#endif /* DEBUG_MODE */

  /*
   * プリミティブ追加を行わない場合は処理しない
   */
  if(info->flags & HEX_FLAG_UNDRAW) return ;
  
  /*
   * 場合によっては処理しないこともある
   */
  if(!(ctrl->grounded & 1)) return;  /* 接地していない場合は処理しない */

  /* 移動量が一定以上ある場合は、かならず処理 */
  _sceVu0SubVector(&mov, &(ctrl->mov), &(info->pre_mov));
  info->pre_mov = ctrl->mov;  /* 更新 */

  fx = fpu_Sqrt(mov.vx * mov.vx + mov.vz * mov.vz);
  if(fx < 20.0F)
    if(RAND(work->span) > 0) return;

  clock = prim->buffer_clock;

#ifdef FLOOR_CHECK_ENABLE
  /*
   * 足場の床の領域を求める
   */
  if(floor_check)
    {
      int floor_flag = HZX_LevelHazardCheck(ctrl->hzx_id,
					    pos,
					    HZX_CHK_F_FLOOR,
					    HZX_FLOOR_NO_BULLET |
					    HZX_FLOOR_RECOIL_TYPE);
      if( floor_flag & 1)
	{
	  HZX_FLR  flr[2];
	  int flr_atrs[2];

	  HZX_GetLevelHazard(flr, flr_atrs);

	  bound_min_x = DG_MIN(flr->p1.x,   flr->p2.x);
	  bound_min_x = DG_MIN(bound_min_x, flr->p3.x);
	  bound_min_x = DG_MIN(bound_min_x, flr->p4.x);

	  bound_min_z = DG_MIN(flr->p1.z,   flr->p2.z);
	  bound_min_z = DG_MIN(bound_min_z, flr->p3.z);
	  bound_min_z = DG_MIN(bound_min_z, flr->p4.z);

	  bound_max_x = DG_MAX(flr->p1.x,   flr->p2.x);
	  bound_max_x = DG_MAX(bound_max_x, flr->p3.x);
	  bound_max_x = DG_MAX(bound_max_x, flr->p4.x);

	  bound_max_z = DG_MAX(flr->p1.z,   flr->p2.z);
	  bound_max_z = DG_MAX(bound_max_z, flr->p3.z);
	  bound_max_z = DG_MAX(bound_max_z, flr->p4.z);
	}
      else
	{
	  bound_min_x = 1.0F;
	  bound_max_x = -1.0F;
	  bound_min_z = 1.0F;
	  bound_max_z = -1.0F;
	}
    }
#endif


  /*
   * 足場の含まれる正六角形の中心を求める
   */
  /* 中心点の整数位置を求める */
  cz = (int)(pos->vz / (work->hex_r * 1.5F) + 0.5F);
  fx = (float)(1 - (cz & 1)) * 0.5F;
  cx = (int)(pos->vx / (work->hex_r * SQRT_3) + fx);

  /* マップ上の座標に変換する */
  center.vz = work->hex_r * 1.5F * (float)cz;
  center.vx = work->hex_r * SQRT_3 * ((float)(cz & 1) * 0.5F + (float)cx);
  center.vy = pos->vy - ctrl->height;
  center.vw = 1.0F;

  
  max = work->max_point;
  points = RAND(max) + 1;  /* 最低でも一つ */
  add.vy = 0.0F;
  add.vw = 1.0F;

  pr_pos = prim->pos[clock];
  pr_rgb = prim->uvrgb[clock];

  /* 各発生源の処理 */
  for(i = 0; i < points; i++)
    {
      if(*act_num >= *n_prims) break;

      root = RAND(6);  /* 6頂点のどれか    */
      alpha = work->max_alpha; /* とりあえず現状では、最大アルファ値から開始 */
      life  = MIN_LIFE + RAND(work->n_verts - MIN_LIFE);  /* 寿命(最低でも4) */

      /* 開始頂点番号と中心位置から、開始点を求める */

      r = RAND(2) + 1;

      _sceVu0AddVector(&point, &center, &(work->add[root]));
      *scr_vec = point;  /* 最初の頂点を記録 */

      scr_rgb->r = work->r;
      scr_rgb->g = work->g;
      scr_rgb->b = work->b;
      scr_rgb->a = alpha;
      scr_rgb->f = 0x0fff;

      for(j = 1; j < life; j++)
	{
	  /* ベクトルの成長 */
	  /*
	    mv_list には、現在の点から移動できる隣接する点までの、
	    x, y それぞれの距離が外接円の半径を 1 とした値で入っている。
	    また、移動先の点の番号も記録されている。

	    ここでは root は現在の頂点番号(正六角形の頂点に対し、時計まわりに
	    与える) をあらわし、その頂点から移動できる先をランダムに r で
	    選択している。ループ突入時には r は 0 ～ 2 の値だが、
	    ループ2回目からは 0 ～ 1 の値で進行する。
	  */
       	  add.vx = mv_list[root][r].x_step * work->hex_r;
	  add.vy = 0.0f;
	  add.vz = mv_list[root][r].y_step * work->hex_r;
	  root = mv_list[root][r].next_id;
	  _sceVu0AddVector(&point, &point, &add);

#ifdef FLOOR_CHECK_ENABLE
	  if(floor_check)
	    {
	      if(point.vx > bound_max_x) break;
	      if(point.vx < bound_min_x) break;
	      if(point.vz > bound_max_z) break;
	      if(point.vz < bound_min_z) break;
	    }
#endif /* FLOOR_CHECK_ENABLE */


#if 1
	  /* リージョンチェックが必要な場合、中心点からの距離が
	     限界距離以上であれば、それ以上の描画を行わない。 */
	  if(work->region_check)
	    {
	      /* 中心点を原点とした座標を求める */
	      _sceVu0SubVector(&add, &point, &(work->center));
	      
	      /* xz 平面上の距離を求める */
	      fx = fpu_Sqrt(add.vx * add.vx + add.vz * add.vz);
	      
	      /* 距離が work->region_r より大きければ、中断 */
	      if(fx > work->region_r) break;
		  
	    }
#endif

	  *(scr_vec + j) = point;
	  /* 属性の設定 */
	  (scr_rgb + j)->r = work->r;
	  (scr_rgb + j)->g = work->g;
	  (scr_rgb + j)->b = work->b;
	  (scr_rgb + j)->a = alpha - alpha * j / (life - 1);
	  (scr_rgb + j)->f = 0x0fff;
	  r = RAND(2);
	}
      /* あまりの頂点は描画キックを行わない */
      while(j < work->n_verts)
	{
	  (scr_rgb + j)->f = 0x8fff;
	  j++;
	}
      
      /* この時点でスクラッチパッドには、
	 一本分のパラメータが出来上がっている。 */

      /* 未使用領域の先頭を求める */
      idx = (*use_begin + *act_num) % *n_prims;

      CpyScr2Mem(pr_pos + work->n_verts * idx,
		 scr_vec, sizeof(FVECTOR), work->n_verts);

      CpyScr2Mem(pr_rgb + work->n_verts * idx,
		 scr_rgb, sizeof(DG_PRIM2_UVRGB), work->n_verts);

      alpha_list[idx].alpha = alpha;
      alpha_list[idx].t_cnt = 0;
      alpha_list[idx].life = life;

      (*act_num)++;
    }

  /* 使用されていない頂点は全て描画キック無し */
  for(i = 0; i < work->n_verts; i++) (scr_rgb + i)->f = 0x8fff;
  for(i = *act_num; i < *n_prims; i++)
    {
      idx = (*use_begin + i) % *n_prims;
      CpyScr2Mem(pr_rgb + work->n_verts * idx,
		 scr_rgb,  sizeof(DG_PRIM2_UVRGB), work->n_verts);
    }
}

/*
 * CONTROL 構造体の mov が指す位置の足元を光らせる
 */
static void blight_floor(Work * work, HEX_FLOOR_INFO * info)
{
  /* 敵兵その他 NPC 用 */
  blight_footprint(work, info,
		   work->prim, work->alpha_list,
		   &(work->n_prims), &(work->act_num), &(work->use_begin),
		   work->floor_check);
}

/*
 * プレイヤだけ特別扱い
 */
static void blight_player(Work * work)
{
  blight_footprint(work, &(work->player),
		   work->player_prim, work->player_alpha,
		   &(work->p_prims), &(work->p_act_num), &(work->p_begin),
		   work->floor_check);
}


/*
 * その他の足元を発光
 */
static void other_footprint(Work * work)
{
  HEX_FLOOR_INFO * info;

  for(info = work->info; info != NULL; info = info->next)
    {
      blight_floor(work, info);
    }
}

static void blight_down(Work * work,
			DG_PRIM2 * prim, AlphaBuf * alpha_list,
			int * n_prims, int * act_num, int * use_begin)
{
  int idx, i, j;
  int end_cnt;
  int old_clock, clock;
  FVECTOR * scr_vec = SCRPAD_ADDR;    /* 上半分はベクトル用 */
  DG_PRIM2_UVRGB * scr_rgb = (DG_PRIM2_UVRGB *)(scr_vec + 512);  /* 下半分は属性用     */
  FVECTOR        * pr_pos;
  FVECTOR        * old_pos;
  DG_PRIM2_UVRGB * pr_rgb;
  DG_PRIM2_UVRGB * old_rgb;
  int alpha, t_cnt;

  old_clock = prim->buffer_clock;
  DG_SwitchBuffPrim2(prim);
  clock = prim->buffer_clock;

  pr_pos = prim->pos[clock];
  pr_rgb = prim->uvrgb[clock];
  old_pos = prim->pos[old_clock];
  old_rgb = prim->uvrgb[old_clock];

  /* 既に表示されているもののタイムカウンタを減衰させ、
     0 になったものから順次開放して再利用可能にする */
  end_cnt = 0;
  for(i = 0; i < *act_num; i++)
    {
      idx = (*use_begin + i) % *n_prims;
      alpha_list[idx].t_cnt += TIME_BASE;
      if(alpha_list[idx].t_cnt >= work->max_time)
	{
	  /* 表示時間が終った */
	  end_cnt++;
	  continue;
	}

      /* 終っていなければ、各頂点のアルファ値を、
	 減衰関数に従い再設定する。 */

      /* 属性値をスクラッチパッドに読み出す */
      CpyMem2Scr(scr_vec, old_pos + work->n_verts * idx,
		 sizeof(FVECTOR), work->n_verts);
      CpyMem2Scr(scr_rgb, old_rgb + work->n_verts * idx,
		 sizeof(DG_PRIM2_UVRGB), work->n_verts);

      /* アルファ値を関数に従い再設定 */
      for(j = 0; j < alpha_list[idx].life; j++)
	{
	  alpha = alpha_list[idx].alpha;
	  t_cnt = alpha_list[idx].t_cnt;
	  alpha = AlphaFunc(alpha, t_cnt, work->max_time);
	  (scr_rgb + j)->a = alpha;
	}

      /* アルファを再設定した属性を、あらたな属性として描き戻す */
      CpyScr2Mem(pr_pos + work->n_verts * idx, scr_vec,
		 sizeof(FVECTOR), work->n_verts);
      CpyScr2Mem(pr_rgb + work->n_verts * idx, scr_rgb,
		 sizeof(DG_PRIM2_UVRGB), work->n_verts);
    }

  /* 終った分を順次開放 */
  *use_begin = (*use_begin + end_cnt) % *n_prims;
  *act_num -= end_cnt;
}

static void Act(Work * work)
{
  /* 既に描いてある六角形の頂点アルファ値を、時間経過で減衰させる */
  blight_down(work, work->prim, work->alpha_list,
	      &(work->n_prims), &(work->act_num), &(work->use_begin));
  blight_down(work, work->player_prim, work->player_alpha,
	      &(work->p_prims), &(work->p_act_num), &(work->p_begin));

  /* プレイヤの足元 */
  blight_player(work);

  /* 登録されている CONTROL に対し、足元の発光を試みる */
  other_footprint(work);
}

static void Die(Work * work)
{
  GV_Free(work->alpha_list);
  GV_Free(work->player_alpha);
  GM_FreePrim2(work->prim);
  GM_FreePrim2(work->player_prim);
  if(now_work == work) now_work = NULL;
}


/*
 * プリミティブの全頂点を描画キック無しにする(初期化)
 */
static void init_prim_vertex(DG_PRIM2 * prim, int clock,
			     int n_prims, int n_verts)
{
  FVECTOR * scr_vec = (FVECTOR *)SCRPAD_ADDR;
  FVECTOR * vec     = prim->pos[clock];
  DG_PRIM2_UVRGB * scr_uvrgb = (DG_PRIM2_UVRGB *)(scr_vec + 512);
  DG_PRIM2_UVRGB * uvrgb = prim->uvrgb[clock];
  int i;

  /* 頂点キック無しを、頂点数分作成 */
  /* 座標原点設定を頂点数分作成 */
  for(i = 0; i < n_verts; i++)
    {
#ifdef __GNUC__
      *(scr_vec + i) = (FVECTOR){0.0F, 0.0F, 0.0F, 1.0F};
#else
      *(scr_vec + i) = DG_ZeroVector;
#endif	  
      (uvrgb + i)->f = 0x8fff;
    }
  
  /* プリミティブ数分、スクラッチパッドの内容を頂点属性に転送する */
  for(i = 0; i < n_prims; i++)
    {
      CpyScr2Mem(vec + i * n_verts, scr_vec, sizeof(FVECTOR), n_verts);
      CpyScr2Mem(uvrgb + i * n_verts, scr_uvrgb,
		 sizeof(DG_PRIM2_UVRGB), n_verts);
    }
}

static void vertex_init(Work * work)
{
  init_prim_vertex(work->prim, 0, work->n_prims, work->n_verts);
  init_prim_vertex(work->prim, 1, work->n_prims, work->n_verts);
  
  init_prim_vertex(work->player_prim, 0, work->p_prims, work->n_verts);
  init_prim_vertex(work->player_prim, 1, work->p_prims, work->n_verts);
}


/*
 * 描画用 DG_PRIM2 構造体を生成する
 */
static int make_primitives(Work * work)
{
  int i;

  work->prim = GM_MakePrim2(DG_PRIM2_LINE|DG_PRIM2_ALPHA,
			    work->n_prims, work->n_verts);
  work->player_prim = GM_MakePrim2(DG_PRIM2_LINE|DG_PRIM2_ALPHA,
				   work->p_prims, work->n_verts);
  DG_SetPrim2Alpha(work->prim, SCE_GS_SET_ALPHA(0, 2, 0, 1, 0));
  DG_SetPrim2Alpha(work->player_prim, SCE_GS_SET_ALPHA(0, 2, 0, 1, 0));

  if(NULL == (work->alpha_list = GV_Malloc(sizeof(AlphaBuf) * work->n_prims)))
    return -1;
  if(NULL== (work->player_alpha = GV_Malloc(sizeof(AlphaBuf) * work->p_prims)))
    return -1;

  for(i = 0; i < work->n_prims; i++)
    {
      work->alpha_list[i].alpha = 0;
      work->alpha_list[i].t_cnt = 0;
    }

  for(i = 0; i < work->p_prims; i++)
    {
      work->player_alpha[i].alpha = 0;
      work->player_alpha[i].t_cnt = 0;
    }
  vertex_init(work);   /* 全頂点をキック無しにする。 */
  return 0;
}

static void setup_params(Work * work)
{
  /* ラインの色 */
  work->act_num   = 0;
  work->use_begin = 0;
  work->info      = NULL;

  work->r = DEF_COLOR_R;
  work->g = DEF_COLOR_G;
  work->b = DEF_COLOR_B;
  work->max_alpha = DEF_MAX_ALPHA; /* 最大アルファ値     */
  work->max_time  = DEF_MAX_TIME;  /* 残像時間           */
  work->hex_r     = DEF_HEX_SIZE;  /* 正六角形外接円半径 */
  work->n_verts = N_VERTS;    /* デフォルトの最大頂点数(最大64) */
  work->n_prims = N_PRIMS;    /* デフォルトのプリミティブ数     */
  work->p_prims = P_PRIMS;    /* プレイヤ用プリミティブ数       */
  work->max_point = DEF_MAX_POINT;
  work->span      = DEF_SPAN;

  work->center.vx = work->center.vy = work->center.vz = 0.0F;
  work->region_r = 0.0F;
  work->region_check = 0;
  work->floor_check = 0;  /* デフォルトでは床チェックは無し(従来どおり) */

  /*
    これらのデフォルトパラメータは、GCL のオプションで変更できる。

    -color   色
    -alpha   最大アルファ
    -time    残像時間
    -vertex  ライン一つあたりの最大頂点数
    -prims   プリミティブ数
    -size    正六角形外接円の半径
    -x       毎フレーム新しい線を追加する確率 (1/x)
    -user    プレイヤ用プリミティブ数
    -region  亀甲エフェクトを発生させる領域の中心点と半径
    -floor   床チェックを行う
  */

  if(GCL_GetOption('c')) /* 'color' */
    {
      work->r = GCL_GetNextInt();
      work->g = GCL_GetNextInt();
      work->b = GCL_GetNextInt();
    }
  if(GCL_GetOption('a')) /* 'alpha' */
    work->max_alpha = GCL_GetNextInt();

  if(GCL_GetOption('t')) /* 'time' */
    work->max_time = GCL_GetNextInt();

  if(GCL_GetOption('v')) /* 'vertex' */
    work->n_verts = GCL_GetNextInt();

  if(GCL_GetOption('p')) /* 'prims' */
    work->n_prims = GCL_GetNextInt();

  if(GCL_GetOption('s')) /* 'size' */
    work->hex_r = (float)GCL_GetNextInt();

  if(GCL_GetOption('m')) /* 'maxpoint' */
    work->max_point = GCL_GetNextInt();

  if(GCL_GetOption('x')) /* 'x' */
    work->span = GCL_GetNextInt();

  if(GCL_GetOption('u')) /* 'user' */
    work->p_prims = GCL_GetNextInt();

  if(GCL_GetOption('r')) /* 'region' */
    {
      work->center.vx = (float)GCL_GetNextInt();
      work->center.vz = (float)GCL_GetNextInt();
      work->region_r = (float)GCL_GetNextInt();
      work->region_check = 1;
    }
  if(GCL_GetOption('f')) /* 'floor' */
    work->floor_check = 1;
}


static void setup_hex_vertex(Work * work)
{
  float xp, yp;
  int i;

  xp = work->hex_r * X_STEP;
  yp = work->hex_r * Y_STEP;

  work->add[0].vx = work->add[3].vx = 0.0F;
  work->add[0].vz = -work->hex_r;
  work->add[3].vz =  work->hex_r;

  work->add[1].vx = work->add[2].vx =  xp;
  work->add[4].vx = work->add[5].vx = -xp;

  work->add[1].vz = work->add[5].vz = -yp;
  work->add[2].vz = work->add[4].vz =  yp;

  for(i = 0; i < 6; i++)
    {
      work->add[i].vy = 0.0F;
      work->add[i].vw = 1.0F;
    }
}


/*
 * 位置とサイズから、床を光らせる領域を得る
 */
static int GetResources(Work * work, int name, int where)
{
  if(NULL != now_work) return -1;
  now_work = work;

#ifdef DEBUG_MODE
  GM_AddDebugMenu(&debug_menu);
#endif /* DEBUG_MODE */

  setup_params(work);      /* オプションの処理、デフォルト値の変更 */
  setup_hex_vertex(work);  /* 頂点までのベクトルを求める           */
  HEX_RegistControlInfo(&(work->player), GM_PlayerControl);
  work->info = NULL;  /* プレイヤは別口なので、リストには含めない */
  return make_primitives(work);   /* DG_PRIM2 の生成                      */
}


void * NewBlinkFloor(int name, int where)
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
