//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  easylayout.c
  シナリオから呼び出し可能な 2Dレイアウトのマスク

  2001/01/12  Y.Kira

  $Id: easylayout.c,v 1.1.1.3 2002/11/19 11:43:53 Yoshizawa1 Exp $
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

#include "font.h"
#include "libfs.h"

#include "sprite_2d.h"  /* 2D モジュールを使用 */
#include "../../2D_action/layout_2d.h"  /* 2D レイアウトドライバを使用 */

#ifdef __GNUC__
#ifdef DEBUG
#define DBG(args...)  printf(args)
#else
#define DBG(args...)
#endif /* DEBUG */
#else
#define DBG
#endif

#define DEFAULT_CHANL 0   /* 通常はゲーム画面に重ねる */

typedef struct {
  GV_ACT_EX   actor;

  int         name;          /* chara 名称         */

  int         handle;        /* レイアウトハンドル */
  int         layout_name;   /* レイアウトデータ名 */

  int         step;          /* 実行段階           */
  int         chanl;         /* 表示チャネル       */

  int         init_action;   /* 初期化アクション   */
  int         begin_action;  /* 表示開始アクション */
  int         end_action;    /* 終了アクション     */

  int         exec_action;   /* 任意実行アクション */

  int         block_mode:1;  /* 新たなアクションを実行する際に,
				以前のアクションが実行中であれば拒否する */
} Work;


enum {
  STEP_INIT,
  STEP_BEGIN,
  STEP_WAITING,
  STEP_END,
  STEP_EXIT
};

/*
 * アクション関連のメッセージ
 */
static int message_proc(Work * work)
{
  int n_msg;
  GV_MSG * msg;
  int ret = 0;

  if(!(n_msg = GV_ReceiveMessage(work->name, &msg))) return 0;

  while(n_msg)
    {
      switch(msg->message[0])
	{
	case 0:   /* 簡易レイアウトプレイヤの破棄 */
	  DBG("msg: destroy.\n");
	  if(msg->message[1] == 0)
	    work->block_mode = 0;
	  else
	    work->block_mode = 1;
	  work->exec_action = -1;
	  work->step = STEP_END;  /* 自殺前に終了アクションを再生 */
	  ret = 1;
	  break;
	case 1:   /* アクション再生状態の取得 */
	  DBG("msg: get status.\n");
	  if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle))
	    GCL_Status = 1;
	  else
	    GCL_Status = 0;
	  break;
	case 2:   /* 指定アクションの再生(再生中のものが終ってから) */
	  DBG("msg: play action.\n");
	  if(msg->message[1] == 0)
	    work->block_mode = 0;
	  else
	    work->block_mode = 1;
	  work->exec_action = msg->message[2];
	  ret = 0;
	  break;
	}
      msg++;
      n_msg--;
    }
  
  return 1;
}

static void Act(Work * work)
{
  switch(work->step)
    {
    case STEP_INIT:
      if(L2D_ActionStatus(work->handle) == L2D_STAT_BUSY) return;
      L2D_EvokeAction(work->handle, work->begin_action);
      work->step = STEP_BEGIN;
      break;
    case STEP_BEGIN:
      if(L2D_ActionStatus(work->handle) == L2D_STAT_BUSY) return;
      work->step = STEP_WAITING;
      break;
    case STEP_END:
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle))
	{
	  if(work->block_mode) break;
	  else 	  /* ブロックモードでなければ強制停止 */
	    L2D_BreakAction(work->handle);
	}
      L2D_EvokeAction(work->handle, work->end_action);
      work->step = STEP_EXIT;
      break;
    case STEP_EXIT:  /* chara を殺して終了 */ 
      if(L2D_ActionStatus(work->handle) == L2D_STAT_BUSY) return;
      GV_DestroyActor(work);
      break;
    case STEP_WAITING:  /* 開始してから */
      
      /* あらたなアクション起動メッセージがなければ何もしない */
      if(!message_proc(work)) break;
      if(work->exec_action < 0) break;

      if(L2D_STAT_BUSY == L2D_ActionStatus(work->handle))
	{
	  if(work->block_mode) break;
	  else  /* ブロックモードでなければ強制停止 */
	    L2D_BreakAction(work->handle);
	}
      L2D_EvokeAction(work->handle, work->exec_action);
      break;
    }
}

static void Die(Work * work)
{
  L2D_ReleaseLayout(work->handle);
  
}

static int GetResources(Work * work, int name, int where)
{

  work->name = name;

  work->layout_name  = -1;
  work->handle       = -1;
  work->init_action  = -1;
  work->begin_action = -1;
  work->end_action   = -1;
  work->exec_action  = -1;
  work->chanl = DEFAULT_CHANL;

  work->step = STEP_INIT;

  if(GCL_GetOption('c')) /* 'chanl'  */
    work->chanl = GCL_GetNextInt();

  if(GCL_GetOption('l')) /* 'layout' */
    work->layout_name = GCL_GetNextInt();

  if(GCL_GetOption('i')) /* 'init' */
    work->init_action = GCL_GetNextInt();

  if(GCL_GetOption('b')) /* 'begin' */
    work->begin_action = GCL_GetNextInt();

  if(GCL_GetOption('e')) /* 'end'   */
    work->end_action = GCL_GetNextInt();

  work->handle = L2D_LoadLayout(work->layout_name, work->chanl, 0, SPR_FLAG_PRIV);

  if(work->handle < 0) return -1;

  if(L2D_EvokeAction(work->handle, work->init_action) < 0)
    {
      L2D_ReleaseLayout(work->handle);
      return -1;
    }

  return 0;
}

void * NewEasyLayout(int name, int where)
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
