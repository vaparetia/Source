/*
  c_h_sync.c
  水平同期ずれを表現する、無線画面用エフェクト
  2000/09/13

  $Id: c_h_sync.c,v 1.1.1.3 2002/11/19 11:44:56 Yoshizawa1 Exp $
*/
/*
  無線画面において、任意の領域を少しずらす、水平方向に拡大/縮小などして
  フィードバック書き込みを行うことで、水平同期ずれを表現する。
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
#include "libdg.h"
#include "libdg.cnf"
#include "dmapack.h"
#include "def_dma.h"

#define _c_h_sync_c_
#include "c_h_sync.h"
#include "codecmem.h"


typedef struct dmaHSync {
  DG_DMATAG   dmatag;
  DG_GIFTAG   giftag;

  DG_GSREG    extbuf;
  DG_GSREG    extdata;
} dmaHSync;










typedef struct Work {
  GV_ACT_EX   actor;

} Work;


static void Act(Work * work)
{
}


static void Die(Work * work)
{
}

static int GetResources(Work * work)
{
  return 0;
}

void * NewHSyncError(void)
{
  Work * work;

  OPERATOR();

  if(NULL == (work = codecMalloc(ACT_MODE, sizeof(Work), 0xf0))) return NULL;

  GV_SetActor(&work->actor, Act, Die);
  GV_ActorEX(&work->actor);

  if(GetResources(work))
    {
      GV_DestroyActor(work);
      return NULL;
    }

  return work;
}


