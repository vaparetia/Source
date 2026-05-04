//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
/*
  cdc_mind.c
  心の声が聞こえる。

  2001/03/26  Y.Kira
  $Id: cdc_mind.c,v 1.1.1.3 2002/11/19 11:44:58 Yoshizawa1 Exp $
*/
#endif

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

#ifdef KP_XBOX
#include <xtl.h>
#endif

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

#if defined(BP_VITA)
#include "gesture_vta.h"
#endif

#define _cdc_mind_c_
#include "cdc_mind.h"

#ifdef PSX2
#ifdef DEBUG
#define DBG(...)  printf(__VA_ARGS__)
#else
#define DBG(...)
#endif /* DEBUG */
#endif
#ifdef KP_XBOX
#define DBG
#endif

#if BP_VITA
// On Vita, the mind voice is time-controlled.  This means
// that you hold down L or R to think positive or negative thoughts for up 
// to one second, rather than relying on pressure.

#  define AS_THINK_PRESSURE_SUPPORT 0
#else
#  define AS_THINK_PRESSURE_SUPPORT 1
#endif


#define MAX_PRESSURE 50
#if AS_THINK_PRESSURE_SUPPORT
#  define PRESS_TIMEOUT  30 /* 90 */
#else
// Press timeout is much higher without pressure sensitivity
// so that time can be the variable
#  define PRESS_TIMEOUT  300 /* 90 */
#endif

static int as_pressure_from_count( int const cnt )
{
   // Gets a clamped "pressure value" from count.  
   // Transforms 0..PRESS_TIMEOUT to 0..255

   if ( cnt < 0 )
   {
      return 0;
   }
   else if ( cnt > PRESS_TIMEOUT )
   {
      return 255;
   }
   else
   {
      return cnt * 255 / PRESS_TIMEOUT;
   }
}

#ifdef PSX2
#define PAD_POSITIVE_THOUGHT  PAD_R1
#if AS_THINK_PRESSURE_SUPPORT
#  define PAD_PRESS_POSITIVE_THOUGHT PAD_PRESS_R1
#endif

#define PAD_NEGATIVE_THOUGHT PAD_L1
#if AS_THINK_PRESSURE_SUPPORT
#  define PAD_PRESS_NEGATIVE_THOUGHT PAD_PRESS_L1
#endif
#else
/* XBOXではR2ボタンが変な所にあるので変更 2002.07.26 Yano */
#define PAD_POSITIVE_THOUGHT  PAD_R1
#define PAD_PRESS_POSITIVE_THOUGHT PAD_PRESS_R1
#define PAD_NEGATIVE_THOUGHT PAD_L1
#define PAD_PRESS_NEGATIVE_THOUGHT PAD_PRESS_L1
#endif

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

  int           max_pressure; /* ボタンが押されてから離される、
				 もしくはタイムアウトまでの最大強度 */
  int           press_cnt;    /* ボタンが押されてから経過した時間   */
  long64          press;        /* 押され始めのボタン */

  int           enable:1;     /* 0 になったら自殺 */
  int           final:1;      /* 1 になったら、最後の心の声 */
  int           mask:1;       /* 1 のときは、ボタンが押されても反応しない */
} Work;


static int def_pressure[4] = { 64, 128, 196, 255 };


static Work * now_work = NULL;


static void Waiting(Work * work);
static void Playing(Work * work);

/* -------------------------------------------------------------------------
   対外インタフェース
   ------------------------------------------------------------------------- */

void CDC_DisableMindVoice(void)
{
  Work * work = now_work;

  if(NULL == work) return;
  work->mask = 1;
}

void CDC_EnableMindVoice(void)
{
  Work * work = now_work;
  if(NULL == work) return;
  work->mask = 0;
}

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
  for(i = 0; i < 4; i++)
    {
      arr[i] = vox_array[i];
      printf(" mind vox array[%d] = %d\n", i, vox_array[i]);
    }
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
      DBG("Play Vox: %d\n", work->play_vox);
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
      printf("MindVoice Playing...\n");
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
    STAT_WAIT,
    STAT_PRESSING,
    STAT_SPEAK
  };

  switch(work->step)
    {
    case STAT_INIT:   /* 初期化         */
      work->step = STAT_WAIT;
      break;
      
    case STAT_WAIT:   /* パッド入力待ち */
      if(work->mask) break;   /* 開始禁止であれば何もしない */
      if(!(work->pad->press & (PAD_POSITIVE_THOUGHT | PAD_NEGATIVE_THOUGHT)) ) break;
      {
	int pwR1, pwR2;

	work->press_cnt = 0;
	work->step = STAT_PRESSING;

#if !AS_THINK_PRESSURE_SUPPORT
   pwR1 = (work->pad->status & PAD_POSITIVE_THOUGHT)
      ? 1 : 0;
   pwR2 = (work->pad->status & PAD_NEGATIVE_THOUGHT)
      ? 1 : 0;
#else
   /* 両方押されている可能性があるので、ルールを設ける。
	   (1) 押されていないほうのボタンは強さ 0 とする。
	   (2) 強さが強いほうのボタンを有効とする。
	   (3) 両者の強さが同じ場合、R1(肯定)ボタンを有効とする */
	pwR1 = (work->pad->status & PAD_POSITIVE_THOUGHT)
	  ? work->pad->pressure[ PAD_PRESS_POSITIVE_THOUGHT ] : 0;
	pwR2 = (work->pad->status & PAD_NEGATIVE_THOUGHT)
	  ? work->pad->pressure[ PAD_PRESS_NEGATIVE_THOUGHT ] : 0;
#endif

	if(pwR1 >= pwR2)
	  {
	    work->press = PAD_POSITIVE_THOUGHT;
	    work->max_pressure = pwR1;
	  }
	else
	  {
	    work->press = PAD_NEGATIVE_THOUGHT;
	    work->max_pressure = pwR2;
	  }
      }
      break;

    case STAT_PRESSING:
      /* ボタンが離されたら,押されてからの最大値を用いる */
      if(!(work->pad->status & work->press))
	{
	  work->step = STAT_SPEAK;
	  break;
	}
      /* 時間切れの場合,その時間内での最大値を用いる */
      if((work->press_cnt += TIME_BASE) >= PRESS_TIMEOUT)
	{
	  work->step = STAT_SPEAK;
	  break;
	}

      /* 現在の強さを得る */
      {
	int pw;
	
#if AS_THINK_PRESSURE_SUPPORT
   
   pw = (work->press & PAD_POSITIVE_THOUGHT)
	  ? work->pad->pressure[ PAD_PRESS_POSITIVE_THOUGHT ]
	  : work->pad->pressure[ PAD_PRESS_NEGATIVE_THOUGHT ];

#else
   // AS(JM) - Update the pressure from the press count.  
   // The longer held, the higher the pressure

   pw = as_pressure_from_count( work->press_cnt + TIME_BASE );
#endif

	if(pw > work->max_pressure) work->max_pressure = pw;
      }
      break;
    case STAT_SPEAK:
      {
	int pw, idx, i;
	int * arr;
	  
	arr = (work->press & PAD_POSITIVE_THOUGHT) ? work->vox_true : work->vox_false;
	pw = work->max_pressure;
	  
	/* 強さによって、メッセージを変える */
	printf("pressure = %d\n", pw);
	idx = 0;
	for(i = 0; i < 3; i++) if(work->level[i] < pw) idx++;
	  
	/* 音声を選出する */
	work->play_vox = arr[idx];
	if(work->play_vox < 0)
	  {
	    /* 音声が設定されていなければ、何もしない */
	    work->step = STAT_WAIT;
	    break;
	  }

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
  // printf("cdc_mind.c: Act() is done.\n");
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
  for(i = 0; i < 3; i++) work->level[i] = def_pressure[i];

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
