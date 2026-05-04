//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  test.c
  2Dレイアウトモジュールのテスト

  2001/01/12  Y.Kira

  $Id: test.c,v 1.1.1.3 2002/11/19 11:43:51 Yoshizawa1 Exp $
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

#include "font.h"
#include "libfs.h"

#include "sprite_2d.h"  /* 2D モジュールを使用 */
#include "layout_2d.h"  /* 2D レイアウトドライバを使用 */

#ifdef PSX2

#ifdef _DEBUG_
#define DBG(args...)  printf(args)
#else
#define DBG(...)
#endif /* _DEBUG_ */

#else

#include <stdarg.h>
#ifdef _DEBUG_
static inline void DBG( char *f, ... )
{
	va_list		argptr ;
	va_start( argptr, format );
	vsprintf( buff, format, argptr );
	va_end( argptr );
}
#else
static void inline DBG( char *f, ... )
{
	va_list		argptr ;
	va_start( argptr, f );
	//vsprintf( buff, f, argptr );
	va_end( argptr );
}
#endif /* _DEBUG_ */

#endif


typedef struct {
  GV_ACT_EX   actor;

  int         handle;   /* レイアウトハンドル */
  int         layout_name;

  int         init_action;
  int         play_action;
} Work;


static void signal_handle(Work * work, int sign, int value)
{
  switch(sign)
    {
    case 0x4a00fd:  /* 信号1 */
      printf("signal 1: value = %d\n", value);
      break;
    case 0x4a00fe:  /* 信号2 */
      printf("signal 2: value = %d\n", value);
      break;
    }
  printf("---\n");
}


static void Act(Work * work)
{
  if(L2D_ActionStatus(work->handle) == L2D_STAT_BUSY) return;
  L2D_EvokeAction(work->handle, work->play_action);
}

static void Die(Work * work)
{
  L2D_ReleaseLayout(work->handle);
}

static int GetResources(Work * work, int name, int where)
{
  work->layout_name = -1;
  work->handle      = -1;
  work->init_action = -1;
  work->play_action = -1;

  if(GCL_GetOption('l')) /* 'layout' */
    work->layout_name = GCL_GetNextInt();

  if(GCL_GetOption('i')) /* 'init' */
    work->init_action = GCL_GetNextInt();

  if(GCL_GetOption('p')) /* 'play' */
    work->play_action = GCL_GetNextInt();
  
  work->handle = L2D_LoadLayout(work->layout_name, 4, 0, SPR_FLAG_PRIV);

  if(work->handle < 0) return -1;

  L2D_SetSignalHandle(work->handle, work, signal_handle);

  if(L2D_EvokeAction(work->handle, work->init_action) < 0)
    {
      L2D_ReleaseLayout(work->handle);
      return -1;
    }

  return 0;
}

void * NewLayoutTest(int name, int where)
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
