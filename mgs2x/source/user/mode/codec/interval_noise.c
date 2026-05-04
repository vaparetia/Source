//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  interval_noise.c
  普段から乗るノイズ

  2001/07/18   Y.Kira
  $Id: interval_noise.c,v 1.1.1.3 2002/11/19 11:45:02 Yoshizawa1 Exp $
*/
/* ほぼ一定間隔 + ランダムな間隔でノイズを顔に載せます。 */

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
#include "debugmenu.h"
#include "codec.h"

#include "font.h"
#include "cjimaku.h"
#include "libfs.h"
#include "libdg.h"
#include "libmt.h"
#include "libutl.h"
#include "fmt_demo.h"
#include "dmapack.h"
#include "def_dma.h"
#define _interval_noise_c_
#include "cdc_load.h"
#include "cdc_face.h"
#include "codecmem.h"
#include "codeccrt.h"
#include "c_hsync.h"

#include "c_layout.h"
#include "sprite_2d.h"
#include "3d_util.h"
#include "codecmtn.h"
#include "cdcobj.h"
#include "handycam.h"
#include "c_nfocus.h"
#include "c_ffocus.h"

#include "interval_noise.h"

#include "codec_config.h"
#include "codec_signal.h"
#include "cdc_hair.h"
#include "../../kano/hair/hair_called.h"
#include "cdc_noise.h"

#ifdef PSX2
#undef _DEBUG_ 
#ifdef _DEBUG_
#define DBG(...)  printf(__VA_ARGS__)
#else
#define DBG(...)
#endif /* _DEBUG_ */
#endif
#ifdef KP_XBOX
#define DBG
#endif


#define TIME_MAX 250

#define BUG_RATE 10

typedef struct {
  GV_ACT_EX    actor;

  int          side;       /* 表示サイド                 */

  int          noise;      /* 次に表示するノイズの種類   */
  int          len;        /* 次に起動するノイズの長さ   */
  int          wait;       /* 次に起動するまでの時間     */
  int          cnt;        /* 経過時間                   */
  int          snd;        /* ノイズ時のサウンド         */

  int          finish:1;   /* 終了フラグ                 */
} Work;


static Work * now_work[2] = {NULL, NULL};

#ifdef DEBUG_MODE

static int _DEBUG_codec_noise = 1;

#ifdef PSX2
static GM_DEBUG_MENU debug_menu = {
  class:   "CODEC",
  menu:    "NOISE",
  max:     2,
  items:   (char *[]){"ON", "OFF"},
  values:  (int[]){1, 0},
  target:  &_DEBUG_codec_noise,
  mask:    0x00000001
};


void CODEC_AddNoiseDebugMenu(void)
{
  GM_AddDebugMenu(&debug_menu);
}
#endif
#ifdef KP_XBOX
void CODEC_AddNoiseDebugMenu(void)
{
}
#endif

#endif /* DEBUG_MODE */

/*
 * 次に起動するノイズを用意する
 */
static void setup_noise(Work * work)
{
  static struct {
    int rate;    /* 発生確率   */
    int noise;   /* ノイズ種別 */
    int span;    /* 間隔最低値 */
    int min;     /* ノイズの長さ最小 */
    int max;     /* ノイズの長さ最長 */
  } noise_list[] = {
    { 60, -1,              150, 200, 450 },   /* ノイズ無し   */
    { 40, CDC_NOISE_GHOST, 80,   70, 120 },   /* ゴースト     */
#if 0
    { 15, CDC_NOISE_VFAST, 200, 100, 200 },   /* 早送りノイズ */
    { 10, CDC_NOISE_SAND,  50,  20,  40 },    /* サンドノイズ */
#endif
  };
  int dice;
  int i;

#ifdef PSX2
  dice = ((BP_PS2_rand() >> 16) % 100);
#else //XBOX
  dice = (rand() % 100);
#endif

  i = 0;
  while(dice >= noise_list[i].rate)
    {
      dice -= noise_list[i].rate;
      i++;
    }

  work->noise = noise_list[i].noise;
#ifdef PSX2
  work->len   = noise_list[i].min +
    ((BP_PS2_rand() >> 16) % (noise_list[i].max - noise_list[i].min));
  work->wait  = noise_list[i].span + ((BP_PS2_rand() >> 16) % 100) + work->len;
#else //XBOX
  work->len   = noise_list[i].min +
    (rand() % (noise_list[i].max - noise_list[i].min));
  work->wait  = noise_list[i].span + (rand()  % 100) + work->len;
#endif
  work->cnt = 0;
}

static void Act(Work * work)
{
  /* 終了が指定されていればなにもせずに終了 */
  if(work->finish)
    {
      GV_DestroyActor(work);
      return;
    }

  if((work->cnt += TIME_BASE) >= work->wait)
    {
      if(work->noise >= 0)
	{
	  void * noisework;
	  /* ノイズを起動 */
#ifdef DEBUG_MODE
	  if(_DEBUG_codec_noise)
	    {
	      // printf("Noise Evoke %d len = %d\n", work->noise, work->len);
#endif
	      noisework = NewCodecNoise(work->side, work->noise, work->len);
	      if(work->noise == CDC_NOISE_SAND) GM_SdSet(work->snd);
	      if(NULL != noisework);
	      GV_SetActorChild(work, noisework);
#ifdef DEBUG_MODE
	    }
#endif
	}
      setup_noise(work);
    }
}

static void Die(Work * work)
{
}


static int ReceiveSignal(void * workp, int signal, int value)
{
  Work * work = workp;
  int ret = 0;
  switch(signal)
    {
    case CDC_SIGNAL_NOISE_KILL:
      // GV_CallChildSignalFunc(work, signal, value);
      work->finish = 1;
      break;
    default:
      ret = GV_DefaultSignalFunc(workp, signal, value);
      break;
    }
  return ret;
}

static int GetResources(Work * work, int side)
{
  int cnt;
  int i;

  if(NULL != now_work[side]) return -1;

  /* 終了を指定するためのシグナルハンドラを設定する */
  GV_SetActorSignalFunc(work, ReceiveSignal);

  work->len = 0;
  setup_noise(work);

  work->side = side;
  work->finish = 0;
  work->snd = (!side) ? CODEC_SE_CAMERA_CHG_L : CODEC_SE_CAMERA_CHG_R;

  return 0;
}

void * NewIntervalNoise(int side)
{
  Work * work = codecActorPrio(GV_ACTOR_MANAGER, sizeof(Work), 0xf0);
  if(NULL == work) return NULL;

  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));

  if(GetResources(work, side))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  return work;
}
