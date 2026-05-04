//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  allmap.c
  ポーズ時全体マップ表示 3D表示版

  2001/04/03  Y.Kira
  $Id: allmap.c,v 1.1.1.3 2002/11/19 11:43:54 Yoshizawa1 Exp $
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"

#include "g_define.h"
#include "font.h"
#include "libfs.h"
#include "libgv.h"

#include "sprite_2d.h"
#include "../2D_action/layout_2d.h"
#include "map_config.h"

#ifdef _DEBUG_
#define DBG(args) printf(args)
#else
#define DBG(args)
#endif  /* _DEBUG_ */



typedef struct Work {
  GV_ACT_EX   actor;

  int         name;         /* actor 名                               */

  GV_PAD    * pad;          /* パッドデータ                           */

  int         layout;       /* 2D レイアウトハンドル                  */
  int         pos_action;   /* プレイヤ位置に合わせたアクションの名称 */

  int         next_layout;  /* 設定待ちレイアウト                     */
  int         next_action;  /* 設定待ちアクション                     */

  SPR_OBJ   * mask;         /* 画面のトーンを落すためのスプライト     */
  int         fade_cnt;     /* フェードイン/アウトのカウンタ          */ 
  int         step;         /* 処理ステップ                           */

  /*
   * 全体マップ用モデル
   * 多分右スティックでぐりぐり回る。
   */
  
  
  
  
  
  
  
  
  

  /* Act 処理関数のポインタ */
  void  (* ActFunc)(struct Work * work);

  int         quit:1;       /* 終了フラグ                             */

} Work;

static void map_fadein(Work * work);
static Work * now_work = NULL;


void ALLMAP_Quit(void)
{
  Work * work = now_work;

  if(NULL == work) return;

  work->quit = 1;
}


/*
 * Act 処理関数の切替え
 */
static void act_setup(Work * work, void (*func)(Work * work))
{
  work->ActFunc = func;
  work->step = 0;  
}

/*
 * レイアウトの切替え
 */
static void change_layout(Work * work, int layout_name)
{
  if(work->layout >= 0) L2D_ReleaseLayout(work->layout);
  if(layout_name >= 0)
    work->layout = L2D_LoadLayout(layout_name,
				  MAP_CHANL, MAP_PRIORITY, MAP_FLAGS);
  else
    work->layout = -1;
}

/*
 * アクションの切替え
 */
static void change_action(Work * work, int action_name)
{
  work->pos_action = action_name;
}


/*
 * メッセージを受け取り、内容によって使用するレイアウトと、
 * 位置表示アクションを変更する
 */
static void check_message(Work * work)
{
  int n_msg;
  GV_MSG * msg;

  if(n_msg = GV_ReceiveMessage(work->name, &msg))
    while(n_msg)
      {
	switch(msg->message[0])
	  {
	  case 0:   /* レイアウト変更 */
	    work->next_layout = msg->message[1];
	    break;
	  case 1:   /* アクション変更 */
	    work->next_action = msg->message[1];
	    break;
	  case 2:   /* レイアウト、アクション同時設定 */
	    work->next_layout = msg->message[1];
	    work->next_action = msg->message[2];
	    break;
	  }
	n_msg--, msg++;
      }

  if(work->layout >= 0)
    if(L2D_ActionStatus(work->layout) == L2D_STAT_BUSY) return;

  if(work->next_layout >= 0)
    {
      change_layout(work, work->next_layout);
      work->next_layout = -1;
    }

  if(work->next_action >= 0)
    {
      change_action(work, work->next_action);
      work->next_action = -1;
    }
}


/*
 * 親 Actor からのシグナルの受け口
 */
static int RecieveSignal(void * workp, int signal, int value)
{
  Work * work = workp;
  int ret = 0;

  switch(signal)
    {
    case MAP_SIGNAL_QUIT:  /* 終了処理開始シグナル */
      work->quit = 1;
      break;
    default:
      ret = GV_DefaultSignalFunc(workp, signal, value);
      break;
    }
  return ret;
}

static void Act(Work * work)
{
  /* パッドデータ: 必ず取得できるように、GV_PadDataDirect を参照する */
  work->pad = &GV_PadDataDirect[0];

  check_message(work);  /* 通常時に受け取るメッセージをチェックする */

  (work->ActFunc)(work);   /* Act 処理関数の呼出 */
 
  if(work->quit)
    {
      GV_DestroyActor(work);
    }
}

static void Die(Work * work)
{
  if(now_work == work) now_work = NULL;
  
  /* マスク用スプライト開放 */
  SPR_Destroy_2D_Object(work->mask);
  
  /* レイアウト開放 */
  L2D_ReleaseLayout(work->layout);
}


/*
 * 表示終了中の Act
 */
static void map_finish(Work * work)
{
  enum {
    STEP_INIT,
    STEP_ACT,
    STEP_FADE,
    STEP_FINISH
  };
  int alpha;

  switch(work->step)
    {
    case STEP_INIT:
      if(work->layout >= 0) L2D_EvokeAction(work->layout, MAP_FinishDisp);
      work->step = STEP_ACT;
      break;

    case STEP_ACT:
      if(work->layout >= 0)
	if(L2D_STAT_BUSY == L2D_ActionStatus(work->layout)) break;

      work->step = STEP_FADE;	
      work->fade_cnt = 0;

    case STEP_FADE:
      work->fade_cnt += TIME_BASE;
      alpha = MAP_ALPHA_MAX * work->fade_cnt / MAP_FADE_TIME;

      /*
       * アルファ値を下げる
       */
      if(alpha > MAP_ALPHA_MAX) alpha = MAP_ALPHA_MAX;
      work->mask->sprite.col.a = MAP_ALPHA_MAX - alpha;

      if(work->fade_cnt >= MAP_FADE_TIME) work->step = STEP_FINISH;
      break;
      
    case STEP_FINISH:
      act_setup(work, map_fadein);
      break;
    }
}

/*
 * 表示中の Act
 */
static void map_display(Work * work)
{
  enum {
    STEP_INIT,    /* 表示開始初期化 */
    STEP_DISP,    /* 表示中         */    
    STEP_FINISH,  /* 表示終了       */
    STEP_DIE      /* 死             */
  };

  switch(work->step)
    {
    case STEP_INIT:
      /* 現在位置に対応するアクションを起動 */
      if(work->pos_action >= 0)
	L2D_EvokeAction(work->layout, work->pos_action);
      work->step = STEP_DISP;
      break;

    case STEP_DISP:
      /* 位置表示がある場合、位置表示アクションをループさせる */
      if(work->pos_action >= 0)
	if(L2D_ActionStatus(work->layout) != L2D_STAT_BUSY)
	  L2D_EvokeAction(work->layout, work->pos_action);

      /* START が押されたら終了指示を出す */
      if(work->pad->press & PAD_STA)
	{
	  L2D_BreakAction(work->layout); /* 強制中断 */
	  work->step = STEP_FINISH;
	}
      break;

    case STEP_FINISH:
      act_setup(work, map_finish);
      break;
    }
}


/*
 * マップ表示開始
 */
static void map_fadein(Work * work)
{
  enum {
    STEP_INIT,
    STEP_WAIT,
    STEP_FADE,
    STEP_FINISH
  };
  int alpha;

  switch(work->step)
    {
    case STEP_INIT:
      work->fade_cnt = 0;
      work->mask->sprite.col.a = 0;
      SPR_SHOW(work->mask);
      work->step = STEP_WAIT;
      break;

    case STEP_WAIT:
      /*
       * 起動待ち
       */
      if(GM_CheckMenuStatus(MENU_RADIO_DISABLE)) break;
      /* if(work->call_block) break; */
      /* 待ち状態 */
      if( !GV_PauseLevel && !GM_StreamIsPlay() &&
	  !GM_CheckGameStatus(STATE_PLAY_DEMO) &&
	  (work->pad->press & PAD_STA) &&
	  !DG_UnDrawFrameCount)
	{
	  work->quit = 0;
	  work->step = STEP_FADE;
	}
      SPR_HIDE(work->mask);
      break;

    case STEP_FADE:

      /*
       * ゲーム画面のトーンダウン
       */
      SPR_SHOW(work->mask);
      work->fade_cnt += TIME_BASE;
      alpha = work->fade_cnt * MAP_ALPHA_MAX / MAP_FADE_TIME;
      if(alpha > MAP_ALPHA_MAX) alpha = MAP_ALPHA_MAX;

      work->mask->sprite.col.a = alpha;
      
      if(work->fade_cnt >= MAP_FADE_TIME)
	{
	  /* 初期表示アクションを起動 */
	  if(work->layout >= 0)
	    L2D_EvokeAction(work->layout, MAP_InitialDisp);
	  work->step = STEP_FINISH;
	}
      break;

    case STEP_FINISH:
      /*
       * 初期表示アクションが終了したら、Act を切替える
       */
      if(work->layout >= 0)
	if(L2D_ActionStatus(work->layout) == L2D_STAT_BUSY)
	  break;
      act_setup(work, map_display);   /* 表示中 Act に設定 */
      
      break;
    }
}

static int GetResourcesP(Work * work, int layout_name, int pos_action)
{
	static SPR_POS pos = { 0.0f, 0.0f } ;

  if(NULL != now_work) return -1;

  now_work = work;

  /* 与えられた名称のレイアウトデータでレイアウトを用意する */
  work->next_layout = layout_name;

  /* 現在位置表示アクション名を記録する */
  work->next_action = pos_action;

  work->mask = SPR_Create_2D_Object(SP_SPRITE, MAP_MASKPRI, NULL);
  ASSERT(NULL != work->mask);
  SPR_SetPosSprite(work->mask, &pos);  /* 画面左上 */
  SPR_SetSizeSprite(work->mask, 512.0F, 384.0F);         /* 画面大   */

  work->mask->sprite.col.r =
  work->mask->sprite.col.g =
  work->mask->sprite.col.b =
  work->mask->sprite.col.a = 0 ;
  work->mask->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 0);
  work->mask->head.flags |= MAP_FLAGS | SPR_FLAG_ALPHA;

  work->quit = 0;  /* 終了フラグをクリア */

  act_setup(work, map_fadein);

  GV_SetActorSignalFunc(work, RecieveSignal);

  return 0;
}

static int GetResources(Work * work)
{
  int layout_name;
  int pos_action;

  /*
   * シナリオインタフェースの場合、レイアウト名は GCL から受けとる
   */
  layout_name = -1;
  pos_action = -1;

  if(NULL != GCL_GetOption('s')) layout_name = GCL_GetNextInt();
  if(NULL != GCL_GetOption('p')) pos_action = GCL_GetNextInt();
  if(layout_name < 0) pos_action = -1;
  /* 後の処理は同じ */
  return GetResourcesP(work, layout_name, pos_action);
}

/*
 * プログラムインタフェース用コンストラクタ
 */
void * NewAllMapChildP(int layout_name, int pos_action)
{
  Work * work;

  if(NULL == (work = GV_NewActor(MAP_ACT_LEVEL, sizeof(Work))))
    return NULL;

  work->name = -1;
  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));

  if(GetResourcesP(work, layout_name, pos_action))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  return work;
}

/*
 * シナリオインタフェース用コンストラクタ
 */
void * NewAllMapChild(int name, int where)
{
  Work * work;

  if(NULL == (work = GV_NewActor(MAP_ACT_LEVEL, sizeof(Work))))
    return NULL;

  work->name = name;

  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));

  if(GetResources(work))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  return work;
}
