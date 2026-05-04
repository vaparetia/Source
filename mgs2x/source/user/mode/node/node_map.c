//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  node_map.c
  全体マップの表示など、ノードにおけるマップ関係のモジュール

  2001/01/19   Y.Kira
  $Id: node_map.c,v 1.1.1.3 2002/11/19 11:45:18 Yoshizawa1 Exp $
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
#include "node_map.h"

#include "g_define.h"
#include "font.h"
#include "libfs.h"

#include "sprite_2d.h"
#include "../../kira/2D_action/layout_2d.h"
/* #include "layout_2d.h" */
#include "stream.h"

#define _node_map_c_
#include "node_config.h"
#include "node.h"
#include "node_map.h"

#ifdef PSX2
#ifndef DEBUG
#define DBG(...)
#else
#define DBG(...)   printf(__VA_ARGS__)
#endif /* DEBUG */
#else
#define DBG
#endif
typedef struct Work {
  GV_ACT_EX   actor;

  SPR_OBJ   * center;  /* 基準点の位置("基準点" の名称を持つオブジェクト) */

  int         step;
  GV_PAD    * pad;

  int         handle;  /* レイアウト管理ハンドル                          */

  void (*act_func)(struct Work *);

} Work;

/* =========================================================================
   =========================================================================
   ========================================================================= */
static Work * now_work = NULL;

/* =========================================================================
   =========================================================================
   ========================================================================= */
/*
 * フレーム処理の切替え
 */
static void SetupAct(Work * work, void (*func)(Work *))
{
  work->act_func = func;
  work->step = 0;
}


/*
 * 表示中の処理
 */
static void display_func(Work * work)
{
  int   dx, dy;
  float vx, vy;  /* 移動量 */

  /* 中心点オブジェクトが無い場合は無視 */
  if(NULL == work->center) return;

  /* 左アナログスティックの値を得る */
  dx = work->pad->left_dx;
  dy = work->pad->left_dy;
  
  /* 十字ボタンの感圧情報も考慮にいれる */

  
  /* 移動量を計算する */
  vx = (float)((dx - 128) / 32);
  vy = (float)((dy - 128) / 32);

  /* 倒した方向に、倒した重みに比例する速度でマップをスクロールさせる */
  if(NULL != work->center)
    {
      work->center->empty.pos.x += vx;
      work->center->empty.pos.y += vy;
    }
}



/*
 * 最初のフレーム処理での初期描画
 */
static void init_draw_func(Work * work)
{
  enum {
    STEP_INIT,
    STEP_WAIT
  };

  switch(work->step)
    {
    case STEP_INIT:
      /* "初期描画" の名をもつアクションを起動 */
      L2D_EvokeAction(work->handle, NODEMAP_INIT);

      /* "基準点" の名を持つオブジェクトを得る */
      work->center = L2D_GetObject(work->handle, NODEMAP_CENTER);

      work->step = STEP_WAIT;  /* 描画待ちに移行 */
      break;
    case STEP_WAIT:
      /* 初期アクションが終ったら、表示中の処理に切替える */
      if(L2D_ActionStatus(work->handle) == L2D_STAT_ACK)
	SetupAct(work, display_func);
      break;
    }
}

static void Act(Work * work)
{
  work->pad = &GV_PadDataDirect[0];
  (work->act_func)(work);
}

static void Die(Work * work)
{
  if(now_work == work) now_work = NULL;  /* 登録を抹消       */
  L2D_ReleaseLayout(work->handle);                   /* レイアウトを開放 */
}

/* =========================================================================
   =========================================================================
   ========================================================================= */

/*
 * Actor 用ワークの作成と、Actor 登録
 */
static Work * create_actor(void)
{
  Work * work;

  if(NULL != (work = GV_NewActorPrio(NODE_ACT_MODE,
				     sizeof(Work), NODE_ACT_PRIO)))
    {
      GV_SetActor(work, Act, Die);
      GV_ActorEX(&(work->actor));
    }
  return work;
}

/*
 * 最低限の初期化(シナリオ、プログラム双方からの起動で使用)
 */
static int resources_init(Work * work, int strcode)
{
  if(now_work != NULL) return -1;
  work->center = NULL;  /* 中心オブジェクトを未登録に */

  /* 全体マップレイアウトを読み込み、表示の準備をする */
  DBG("resources_init(): [1]\n");
  work->handle = L2D_LoadLayout(strcode, 4, 0, SPR_FLAG_PRIV);

  /* 初期実行プロセスを登録 */
  DBG("resources_init(): [2]\n");
  SetupAct(work, init_draw_func);

  now_work = work;
  return 0;
}

/*
 * シナリオからの起動による初期化
 */
static int GetResources(Work * work, int name, int where)
{
  int strcode;

  GCL_GetOption('m');
  strcode = GCL_GetNextInt();  /* マップ名を取得 */
  return resources_init(work, strcode); 
}

/*
 * シナリオからの起動
 */
void * NewNodeMap(int name, int where)
{
  Work * work;

  if(NULL != (work = create_actor()))
    if(GetResources(work, name, where))
      {
	GV_DestroyActor(work);
	work = NULL;
      }

  return work;
}

/*
 * プログラムからの起動による初期化
 */
static int GetResources_for_prog(Work * work, int strcode)
{
  return resources_init(work, strcode);
}

/*
 * プログラム用 Actor 起動インタフェース
 */
void * NewNodeMap_for_Prog(int strcode)
{
  Work * work;

  if(NULL != (work = create_actor()))
    if(GetResources_for_prog(work, strcode))
      {
	GV_DestroyActor(work);
	work = NULL;
      }
  return work;
}
