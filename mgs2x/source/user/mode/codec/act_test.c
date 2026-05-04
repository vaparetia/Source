/*
  無線画面アクションのテスト用モジュール
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
#include "codec.h"

#include "font.h"
#include "cjimaku.h"
#include "libfs.h"

#include "cdc_face.h"
#include "codecmem.h"
#include "c_indemo.h"

#define _c_layout_c_
#include "c_layout.h"

typedef struct {
  GV_ACT_EX   actor;

  void      * work;   /* 画面アクションの Work へのポインタ */

  int         freq;

} Work;

static void Act(Work * work)
{
  static int cnt = 0;
  int light;
  if(GV_PadData[1].press & PAD_OK) codecOpenFrame();
  if(GV_PadData[1].press & PAD_CANCEL) codecCloseFrame();

  codecSetLevel(3);
  light = 0;
  if(GV_PadData[1].status & PAD_X) light |= CODEC_LIGHT_PTT;
  if(GV_PadData[1].status & PAD_D) light |= CODEC_LIGHT_MEMORY;
  if(GV_PadData[1].status & PAD_L)
    {
      light |= CODEC_LIGHT_TUNING | CODEC_LIGHT_LEFT;
      if(++(work->freq) > 999) work->freq = 0;
    }
  if(GV_PadData[1].status & PAD_R)
    {
      light |= CODEC_LIGHT_TUNING | CODEC_LIGHT_RIGHT;
      if(--(work->freq) < 0) work->freq = 999;
    }
  
  codecSetFreq(work->freq);
  codecSetLight(light);
}

static void Die(Work * work)
{
  GV_DestroyActor(work->work);
}

static int GetResources(Work * work)
{
  if(NULL == (work->work = NewCodecLayout()))
    {
      printf("Could not create codec layout.\n");
      return -1;
    }
  work->freq = 0;
  return 0;
}

void * NewCodecActionTest(int name, int where)
{
  Work * work;

  OPERATOR();

  if(NULL == (work = codecActorPrio(ACT_MODE, sizeof(Work), 0xf0)))
    return NULL;

  GV_SetActor(&work->actor, Act, Die);
  GV_ActorEX(&work->actor);

  if(GetResources(work))
    {
      GV_DestroyActor(work);
      return NULL;
    }

  return work;
}
