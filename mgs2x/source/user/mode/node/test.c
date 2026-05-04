/*
  ノード起動テスト
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

#include "g_define.h"
#include "font.h"
#include "libfs.h"

#include "node.h"

typedef struct {
  GV_ACT_EX     actor;
} Work;


static void Act(Work * work)
{
  if(GV_PadData[1].press & PAD_R2) NewNode_for_Prog(-1, 0);  /* ノード起動 */
}

static void Die(Work * work)
{
}

static int GetResources(Work * work, int name, int where)
{
  return 0;
}

void * NewNodeTerminalTest(int name, int where)
{
  Work * work;

  if(NULL == (work = GV_NewActor(GV_ACTOR_USER, sizeof(Work))))
    return NULL;

  GV_SetActor(work, Act, Die);
  GV_ActorEX(&(work->actor));

  if(GetResources(work, name, where))
    {
      GV_DestroyActor(work);
      return NULL;
    }
  
  return work;
}
