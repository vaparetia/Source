//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  photo_puppet.c
  オタコン人形制御

  2001/07/04  Y.Kira
  $Id: photo_puppet.c,v 1.1.1.3 2002/11/19 11:45:20 Yoshizawa1 Exp $
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
#include "g_struct.h"
#include "g_define.h"
#include "libdg.h"
#include "font.h"
#include "libfs.h"

#include "sprite_2d.h"
#include "../../kira/2D_action/layout_2d.h"
#define _photo_term_c_
#include "photo_config.h"
#include "photo_term.h"
#include "photo_pic.h"
#include "photo_itp.h"
#include "photo_panel.h"
#include "photo_puppet.h"
#include "photo_codes.h"

#include "../codec/codecmem.h"
#include "../codec/cdc_load.h"

#ifdef DEBUG
#define DBG
#else
#define DBG
#endif  /* DEBUG */

typedef struct {
  GV_ACT_EX      actor;

  int            l2d_handle;

  int            init_action;
  int            action_default;
  int            action_tired;
  int            time_tired;
  int            cnt_tired;

  int            puppet_action; /* 再生アクション                     */
  int            puppet_time;   /* アニメーション継続時間             */
  int            puppet_cnt;    /* アニメーション再生から経過した時間 */

  int            step;

  int            finish:1;

} Work;

enum {
  STEP_INIT,
  STEP_INIT2,
  STEP_WAIT,
  STEP_ACTION,
  STEP_ANIM,
  STEP_IDOL,
  STEP_TIRED,
  STEP_EXIT,
  STEP_FINISH
};

static Work * now_work = NULL;

int PHOTO_SetDefaultAction(int def_action, int tired_action, int tired_time)
{
  Work * work = now_work;
  if(NULL == work) return -1;

  printf("default action = %d\n");
  work->action_default = def_action;
  work->action_tired = tired_action;
  work->time_tired = tired_time;
  work->cnt_tired = 0;
  return 0;
}

int PHOTO_PuppetAction(int action_name, int act_time)
{
  Work * work = now_work;
  if(NULL == work) return -1;
  
  /* アクション起動条件 */
  work->puppet_action = action_name;
  work->puppet_time   = act_time;
  work->step = STEP_ACTION;
  printf("puppet action : act 0x00%06x  time %d\n", action_name, act_time);
  return 0;
}

int PHOTO_PuppetAck(void)
{
  Work * work = now_work;
  if(NULL == work) return -1;

  if((work->step == STEP_INIT) || (work->step == STEP_INIT2)) return -1;

  return 0;
}


static void Act(Work * work)
{
  switch(work->step)
    {
    case STEP_INIT:
      printf("<photo_puppet.c> STEP_INIT\n");
      /*
       * 初期化アクションが終ったら待機状態に
       */      
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle)) break;
      
      L2D_EvokeAction(work->l2d_handle, work->init_action);
      work->step = STEP_INIT2;
      /* break は不要 */
    case STEP_INIT2:
      printf("<photo_puppet.c> STEP_INIT2\n");
      /*
       * 初期化アクションが終ったら待機状態に
       */      
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle)) break;
      
      work->step = STEP_WAIT;
      /* break は不要 */
    case STEP_WAIT:
      /*
       * アニメーション起動待機状態(デフォルトアニメーション再生)
       */
      if(work->action_default < 0) break;

      /* 何かボタンが押されたら疲れカウンタをリセットする */
      if(GV_PadDataDirect[0].press) work->cnt_tired = 0;

      work->cnt_tired += TIME_BASE;
      /* 他のアクションが起動されるまで、
	 デフォルトアクションを再生しつづける */
      if(L2D_STAT_BUSY != L2D_ActionStatus(work->l2d_handle))
	{
	  if(work->action_tired >= 0)
	    if(work->cnt_tired >= work->time_tired)
	      {
		L2D_EvokeAction(work->l2d_handle, work->action_tired);
		work->step = STEP_TIRED;
		break;
	      }
	  L2D_EvokeAction(work->l2d_handle, work->action_default);
	}
      break;
    case STEP_ACTION:
      /*
       * 特殊アクション起動
       */
      /*
	再生時間の値によって挙動が異なる

	 -1     ワンショット(1ループのみ)
	 0      無限ループ
	 1以上  指定時間の間ループ
      */
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle))
	L2D_BreakAction(work->l2d_handle);
      L2D_EvokeAction(work->l2d_handle, work->puppet_action);

      work->step = STEP_ANIM;
      work->puppet_cnt = 0;
      
      break;

    case STEP_ANIM:
      {
	int stat;

	stat = L2D_ActionStatus(work->l2d_handle);

	if(work->puppet_time > 0)
	  {
	    /* 再生時間チェック */
	    if((work->puppet_cnt += TIME_BASE) >= work->puppet_time)
	      {
		if(L2D_STAT_BUSY == stat) L2D_BreakAction(work->l2d_handle);
		work->cnt_tired = 0;
		work->step = STEP_WAIT;
		break;
	      }
	  }

	if(L2D_STAT_BUSY != stat)
	  {
	    if(work->puppet_time < 0)
	      {
		/* ワンショット */
		work->step = STEP_IDOL;
		work->cnt_tired = 0;
		break;
	      }
	    
	    /* アニメーションをループする */
	    L2D_EvokeAction(work->l2d_handle, work->puppet_action);
	    work->cnt_tired = 0;
	  }
      }
      break;
    case STEP_IDOL:
      if(work->action_tired >= 0)
	{
	  work->cnt_tired += TIME_BASE;
	  if(work->cnt_tired >= work->time_tired)
	    if(L2D_STAT_BUSY != L2D_ActionStatus(work->l2d_handle))
	      work->step = STEP_TIRED;
	}
      break;
    case STEP_TIRED:
      if(L2D_STAT_BUSY != L2D_ActionStatus(work->l2d_handle))
	L2D_EvokeAction(work->l2d_handle, work->action_tired);
      break;
    case STEP_EXIT:
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle))
	L2D_BreakAction(work->l2d_handle);
      L2D_EvokeAction(work->l2d_handle, ACT_exit);
      work->step = STEP_FINISH;

      break;
    case STEP_FINISH:
      if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle)) break;
      work->finish = 1;
      break;
    }
  if(work->finish) GV_DestroyActor(work);

}

static void Die(Work * work)
{
  if(L2D_STAT_BUSY == L2D_ActionStatus(work->l2d_handle))
    L2D_BreakAction(work->l2d_handle);
  L2D_ReleaseLayout(work->l2d_handle);
  if(now_work == work) now_work = NULL;
}

static int  ReceiveSignal(void * workp, int signal, int value)
{
  Work * work = workp;
  int ret = 0;

  switch(signal)
    {
    case PHOTO_SIGNAL_KILL_PUPPET:
      work->finish = 1;
      ret = 0;
    case PHOTO_SIGNAL_EXIT_PUPPET:
      work->step = STEP_EXIT;
      ret = 0;
    default:
      ret = GV_DefaultSignalFunc(workp, signal, value);
      break;
    }
  return ret;
}

static int get_sound(int strcode)
{
  static struct {
    int strcode;
    int sound;
  } snd_list[] = {
    { 0x00de6e5a  /* "SD_A_HA_CMPLT" */, SD_A_HA_CMPLT },
    { 0x00d95fb0  /* "SD_A_HA_SCLWI" */, SD_A_HA_SCLWI },
    { 0x007f8650  /* "SD_A_HA_MOVLI" */, SD_A_HA_MOVLI },
    { 0x00ec7934  /* "SD_A_HA_DISCN" */, SD_A_HA_DISCN },
    { 0x009ca51d  /* "SD_A_OTA_IN_1" */, SD_A_OTA_IN_1 },
    { 0x009fbfbd  /* "SD_A_OTA_OUT1" */, SD_A_OTA_OUT1 },
    { 0x009f991d  /* "SD_A_OTA_OK_1" */, SD_A_OTA_OK_1 },
    { 0x009f091d  /* "SD_A_OTA_NG_1" */, SD_A_OTA_NG_1 },
    { 0x009defdd  /* "SD_A_OTA_LAU1" */, SD_A_OTA_LAU1 },
    { 0x00a18c5d  /* "SD_A_OTA_SHY1" */, SD_A_OTA_SHY1 },
    { 0x00a1ab7d  /* "SD_A_OTA_SPR1" */, SD_A_OTA_SPR1 },
    { -1, -1}
  };

  int i;

  for(i = 0; snd_list[i].strcode > 0; i++)
    if(snd_list[i].strcode == strcode) return snd_list[i].sound;

  printf("Unknown SE by layout key.\n");
  HANGUP();
  return snd_list[0].sound;
}

/*
 * レイアウトのシグナルハンドラ
 */
static void LayoutSignals(void * workp, int signal, int value)
{
  Work * work = workp;

  switch(signal)
    {
    case 0x00000aa5  /* "SE" */:
      {
	int sound;
	/* 受け取った StrCode に対応する SEを発生させる */
	sound = get_sound(value);
	GM_SdSet(sound);
      }
      break;
    }
}


static int GetResources(Work * work, int l2d_name, int init_action)
{
   float safeZoneOffsetY=0;
  if(NULL != now_work) return -1;

  printf("photo puppet act setup...\n");
  now_work = work;

  work->finish = 0;
  {
    void * l2d_ptr;
    if(NULL == (l2d_ptr = GV_GetCache(GV_CacheID(l2d_name, 'o'))))
      l2d_ptr = CDC_GetFileEntry(l2d_name, 'o');
    
    work->l2d_handle = L2D_SetupLayout2(l2d_ptr, 4, 0, SPR_FLAG_PRIV, 0, safeZoneOffsetY);
  }
  ASSERT(work->l2d_handle >= 0);
  work->init_action = init_action;

  /*
   * レイアウトのシグナルハンドラを登録
   */
  L2D_SetSignalHandle(work->l2d_handle, work, LayoutSignals);

  /*
   * 初期アクションを再生
   */
  if(work->l2d_handle > 0)
    L2D_EvokeAction(work->l2d_handle, PHOTO_PUPPET_DefaultAction);


  work->action_default = -1;  /* 初期状態ではデフォルトアクション無し */
  work->step = STEP_INIT;

  /* シグナルハンドラを登録する */
  GV_SetActorSignalFunc(work, ReceiveSignal);
  
  return 0;
}

void * NewPhotoTermPuppet(int l2d_name, int init_action)
{
  Work * work = GV_NewActorPrio(PHOTO_ACT_MODE, sizeof(Work), 0xf0);
  if(NULL == work) return NULL;

  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));
  if(GetResources(work, l2d_name, init_action))
    {
      GV_DestroyActor(work);
      return NULL;
    }

  return work;
}
