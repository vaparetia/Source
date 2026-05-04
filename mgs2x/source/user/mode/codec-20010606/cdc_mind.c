/*
  cdc_mind.c
  心の声が聞こえる。

  2001/03/26  Y.Kira
  $Id: cdc_mind.c,v 1.1.1.3 2002/11/19 11:45:05 Yoshizawa1 Exp $
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
#include "codec_config.h"
#include "codec.h"

#include "g_define.h"
#include "font.h"
#include "cjimaku.h"
#include "libfs.h"

#include "cdc_face.h"
#include "codecmem.h"
#include "c_indemo.h"
#include "c_layout.h"
#include "cdc_load.h"
#include "stream.h"
#include "codec_signal.h"

#define _cdc_mind_c_
#include "cdc_mind.h"

#ifdef DEBUG
#define DBG(args...)  printf(args)
#else
#define DBG(args...)
#endif /* DEBUG */


#define MAX_PRESSURE 50

#define PAD_TRUE  PAD_R1
#define PAD_PRESS_TRUE PAD_PRESS_R1

#define PAD_FALSE PAD_R2
#define PAD_PRESS_FALSE PAD_PRESS_R2

typedef struct Work {
  GV_ACT_EX     actor;

  GV_PAD      * pad;
  int           step;         /* 処理ステップ */
  void (*now_act)(struct Work * work);


  int           play_vox;     /* 再生予定音声の ID */
  int           vox_handle;   /* 再生中のストリームのハンドル */

  int           vox_true[4];  /* 肯定音声の ID列 */
  int           vox_false[4]; /* 否定音声の ID列 */

  int           level[3];     /* 押下強度レベルの閾値(lev2～4) */

  int           enable:1;     /* 0 になったら自殺 */
  int           final:1;      /* 1 になったら、最後の心の声 */

} Work;


static Work * now_work = NULL;


static void Waiting(Work * work);
static void Playing(Work * work);

/* -------------------------------------------------------------------------
   対外インタフェース
   ------------------------------------------------------------------------- */

/* 心の声 Actor を殺す */
void CDC_KillMindVoice(void)
{
  Work * work = now_work;

  if(NULL == work) return;
  DBG("============================ Kill MindVoice\n");
  work->final = 1;
}

/* 心の声の vox を設定する */
void CDC_SetupMindVoice(int side, int *vox_array)
{
  Work * work = now_work;
  int * arr;
  int i;

  if(NULL == work) return;
  arr = (!side) ? work->vox_true : work->vox_false;
  for(i = 0; i < 4; i++) arr[i] = vox_array[i];
}

/* 心の声のボタン押下閾値を設定する */
void CDC_MindVoicePressure(int * press_levels)
{
  Work * work = now_work;
  int i;

  if(NULL == work) return;
  for(i = 0; i < 3; i++)  work->level[i] = press_levels[i];
}

/* -------------------------------------------------------------------------
   内部処理
   ------------------------------------------------------------------------- */
static void SetupAct(Work * work, void (*func)(Work *work))
{
  work->now_act = func;
  work->step = 0;
}

/*
 * 再生中のフレーム処理
 */
static void Playing(Work * work)
{
  enum {
    STAT_INIT,
    STAT_PLAYING,
    STAT_FINISH
  };

  switch(work->step)
    {
    case STAT_INIT:
      if(work->play_vox < 0)
	{
	  /* 音声が設定されていなければ入力待ちに戻る */
	  SetupAct(work, Waiting);
	  work->play_vox = -1;
	  break;
	}

      /* 再生を開始する(チャネル 1 を使用) */
      work->vox_handle = GM_VoxStream(work->play_vox, GM_STREAM_CHANNEL_1);
      work->step = STAT_PLAYING;
      break;
    case STAT_PLAYING:
      /* 再生状態を監視する */
      if(GM_StreamStatus(work->vox_handle) == GM_STREAM_STATE_END)
	work->step = STAT_FINISH;
      break;

    case STAT_FINISH:
      GM_StreamStop(work->vox_handle);  /* 再生を停止 */
      work->vox_handle = -1;    /* 再生していないことを示す MagicNumber */
      work->play_vox = -1;
      SetupAct(work, Waiting);  /* 待機状態に復帰する */
      if(work->final) work->enable = 0;
      break;
    }
}


/*
 * 入力待機状態のフレーム処理
 */
static void Waiting(Work * work)
{
  enum {
    STAT_INIT,
    STAT_WAIT
  };

  switch(work->step)
    {
    case STAT_INIT:   /* 初期化         */
      work->step = STAT_WAIT;
      break;
      
    case STAT_WAIT:   /* パッド入力待ち */
      if(work->pad->press & (PAD_TRUE | PAD_FALSE))
	{
	  int pwR1, pwR2, pw, idx, i;
	  int * arr;
	  
	  /* 両方押されている可能性があるので、ルールを設ける。
	     (1) 押されていないほうのボタンは強さ 0 とする。
	     (2) 強さが強いほうのボタンを有効とする。
	     (3) 両者の強さが同じ場合、R1(肯定)ボタンを有効とする */
	  pwR1 = (work->pad->status & PAD_TRUE)
	    ? work->pad->pressure[ PAD_PRESS_TRUE ] : 0;
	  pwR2 = (work->pad->status & PAD_FALSE)
	    ? work->pad->pressure[ PAD_PRESS_FALSE ] : 0;
	  
	  DBG("R1 power: %d   R2 power: %d\n", pwR1, pwR2);
	  
	  arr = (pwR1 >= pwR2) ? work->vox_true : work->vox_false;
	  pw = (pwR1 >= pwR2) ? pwR1 : pwR2;
	  
	  /* 強さによって、メッセージを変える */
	  idx = 0;
	  for(i = 0; i < 3; i++) if(work->level[i] <= pw) idx++;
	  
	  /* 音声を選出する */
	  work->play_vox = arr[idx];
	  
	  SetupAct(work, Playing);  /* 再生中フレーム処理関数に切替え */
	}
      break;
    }
  if(work->final) work->enable = 0;
}

static void Act(Work * work)
{
  work->pad = &GV_PadDataDirect[0];  /* 現在のパッドステータス           */
  (work->now_act)(work);             /* 現在のフレーム処理関数を呼び出す */

  /* 中断指令が出ていたら、死ぬ */
  if(!work->enable)
    {
      /* 親に死ぬ予定であることを告げる */
      GV_CallParentSignalFunc(work, CDC_SIGNAL_MIND_DIE, 0);
      GV_DestroyActor(work);
    }
}


static void Die(Work * work)
{
  if(now_work == work) now_work = NULL;  /* 受付を禁止する*/

  /*
   * 現時点では、enable は GM_StreamStop() を実行後に 0 になるため、
   * Die() が実行される段階では既にストりーミングは停止している
   */
#if 0
  /* 再生中であれば強制的に中断する */
  if(work->vox_handle >= 0) GM_StreamStop(work->vox_handle);
#endif

}

static int GetResources(Work * work)
{
  int i;

  if(NULL != now_work) return -1;
  now_work = work;

  /* 初期化 */
  work->play_vox = -1;
  work->vox_handle = -1;

  /* 音声を設定されていないことにする */
  for(i = 0; i < 4; i++)
    work->vox_true[i] = work->vox_false[i] = -1;

  /* 押下強度レベル閾値の初期値を設定する */
  for(i = 0; i < 3; i++)
    work->level[i] = (i + 1) * (MAX_PRESSURE / 4);

  work->enable = 1;
  work->final = 0;


  SetupAct(work, Waiting);   /* 初期フレーム処理関数を設定 */

  return 0;
}

void * NewCodecMindVoice(void)
{
  Work * work;

  if(NULL != now_work) return NULL;

  if(NULL == (work = GV_NewActorPrio(GV_ACTOR_MANAGER, sizeof(Work), 0xf0)))
    return NULL;
  
  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));
  
  if(GetResources(work))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  
  return work;
}
