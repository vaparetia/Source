//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  bomb_child.c
  爆弾センサー検知領域の、トラップ一箇所あたりに対応

  2001/03/14  Y.Kira
  $Id: bomb_child.c,v 1.1.1.3 2002/11/19 11:43:55 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <eekernel.h>
#include <stdlib.h>
#include <stdio.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <math.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "def_dma.h"
#include "gameheader.h"
#include "radsprite.h"
#include "radsize.h"

#include "bomb.h"
#include "bomb_sensor.h"
#include "bomb_child.h"

typedef struct Work {
  GV_ACT_EX   actor;

  struct Work * next;
  HZX_BLOCK   * blk;
  HZX_TRP     * trp;

  float         width;
  float         height;

  int           blight;
  int           change;

  FVECTOR       vec;
  RADAR_BOMB    bomb;
} Work;

static int RecieveSignal(void * workp, int signal, int value)
{
  Work * work = workp;
  int ret = 0;

  switch(signal)
    {
    case BOMB_BLIGHT_SIGNAL:
      work->change = 1;
      work->blight = value;

      break;
    default:
      ret = GV_DefaultSignalFunc(workp, signal, value);
      break;
    }
  return ret;
}

static void Act(Work * work)
{
  if(!work->change) return;
  BOMB_SetBlight(&(work->bomb), work->blight);
  work->change = 0;
}

static void Die(Work * work)
{
  /* 検知エリアをリストから除去 */
  BOMB_Remove(&(work->bomb));
}

static int GetResources(Work * work,
			HZX_BLOCK * blk, HZX_TRP * trp, int r, int g, int b)
{
  float cx, cy;
  float fx, fy, fz;
  float width, height;

  /* シグナルコールバックの登録 */
  GV_SetActorSignalFunc(work, RecieveSignal);

  work->blight = 0;
  work->change = 0;

  work->next = NULL;
  work->blk  = blk;
  work->trp  = trp;
  
  cx = cy = 0.0F;

  /* 基準点を得る */
  fx = blk->tx + trp->b1.vx;
  fy = blk->ty + (trp->b1.vy + trp->b2.vy) / 2;
  fz = blk->tz + trp->b1.vz;

  width  = trp->b2.vx - trp->b1.vx;
  height = trp->b2.vz - trp->b1.vz;

  work->vec.vx = fx;
  work->vec.vy = fy;
  work->vec.vz = fz;
  work->vec.vw = 1.0F;

  work->width  = width;
  work->height = height;

  /* センサー領域の定義 */
  BOMB_InitBomb(&(work->bomb));
  BOMB_SetArea(&(work->bomb), &(work->vec), cx, cy, width, height);
  BOMB_SetBlight(&(work->bomb), 0);  /* 最初は見えない */

  if((r >= 0) && (g >= 0) && (b >= 0))
    {
      work->bomb.r = r;
      work->bomb.g = g;
      work->bomb.b = b;
    }

  return 0;
}

void * NewBombAreaChild(HZX_BLOCK * blk, HZX_TRP * trp, int r, int g, int b , char mode )
{
  Work * work;

  if(NULL == (work = GV_NewActor(GV_ACTOR_USER, sizeof(Work)))) return NULL;

  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));
  // by koba4
  work->bomb.mode = mode;
  if(GetResources(work, blk, trp, r, g, b))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  printf("NewBombAreaChild() is success.\n");
  return work;
}
