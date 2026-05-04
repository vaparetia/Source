//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  cdc_vib.c
  無線中コントローラ振動キャラ

  2001/07/04  Y.Kira
  $Id: cdc_vib.c,v 1.1.1.3 2002/11/19 11:44:59 Yoshizawa1 Exp $
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
#ifdef KP_XBOX
#include <xtl.h>
#endif

#include "BP_MovieSupport.h"

#include "gameheader.h"
#include "font.h"
#include "libdg.h"
#include "libdg.cnf"
#include "dmapack.h"
#include "def_dma.h"
#include "ipu.h"

#define _cdc_vib_c_
#include "sprite_2d.h"
#include "cdc_vib.h"
#include "codecmem.h"
#include "codec_signal.h"

#ifdef PSX2
#ifdef DEBUG
#define DBG(...)   printf(__VA_ARGS__)
#else
#define DBG(...)
#endif /* DEBUG */
#endif
#ifdef KP_XBOX
#define DBG
#endif

typedef struct {
  GV_ACT_EX      actor;

  int   vib_type;   /* 振動タイプ */
  int   vib_time;   /* 振動時間   */
  int   vib_cnt;    /* 経過時間   */

  int   finish:1;   /* 非 0 になったら終了させる */

} Work;

static int ReceiveSignal(void * workp, int signal, int value)
{
  Work * work = workp;
  int ret = 0;

  switch(signal)
    {
    case CDC_SIGNAL_VIB_KILL:
      work->finish = 1;
      ret = 0;
      break;

    default:
      ret = GV_DefaultSignalFunc(workp, signal, value);
      break;
    }
  return ret;
}

static void Act(Work * work)
{
  GM_PadVibration1 = 1;
  GM_PadVibration2 = work->vib_type;

  if(work->vib_time > 0) work->vib_cnt += TIME_BASE;
  if(work->vib_cnt >= work->vib_time) work->finish = 1;
  if(work->finish)
    GV_DestroyActor(work);
}

static void Die(Work * work)
{
  printf("Vibration end\n");
}

static int GetResources(Work * work, int type, int vib_time)
{
  work->vib_time = vib_time;
  work->vib_cnt = 0;
  work->vib_type = type;
  work->finish = 0;

  GV_SetActorSignalFunc(work, ReceiveSignal);

  return 0;
}

void * NewCodecVibration(int type, int vib_time)
{
  Work * work;

  if(NULL == (work = codecActorPrio(GV_ACTOR_MANAGER, sizeof(Work), 0xf0)))
    return NULL;

  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));

  if(GetResources(work, type, vib_time))
    {
      GV_DestroyActor(work);
      return NULL;
    }

  return work;
}
