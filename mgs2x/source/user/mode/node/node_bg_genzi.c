//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  node_bg_genzi.c
  ノード画面背景: 源氏紋

  2001/07/09 Y.Kira
  $Id: node_bg_genzi.c,v 1.1.1.3 2002/11/19 11:45:17 Yoshizawa1 Exp $
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

#include "gameheader.h"

#include "g_define.h"
#include "font.h"
#include "libfs.h"

#include "sprite_2d.h"
#include "../../kira/2D_action/layout_2d.h"
#include "stream.h"

#define _node_bg_log_c_
#include "node_config.h"
#include "node.h"
#include "node_map.h"

#include "node_bg_def.h"
#include "node_bg.h"
#include "node_bg_log.h"

/* 車輪の再発明を防ぐ: 無線のロード機構を流用 */
#include "../codec/cdc_load.h"
#include "../codec/codecmem.h"

#ifdef PSX2
#ifdef DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif  /* DEBUG */
#else
#define DBG
#endif


#define CODE_SHOW_MAIN				0x00e9bbd3		/* showGenziMain */
#define CODE_HIDE_MAIN				0x00a90c25		/* hideGenziMain */
#define CODE_SHOW_OPTION			0x002f57c6		/* showGenziOpt */
#define CODE_HIDE_OPTION			0x00bd5248		/* hideGenziOpt */
#define CODE_SHOW_NEWGAME			0x002f5269		/* showGenziNew */
#define CODE_HIDE_NEWGAME			0x00bd4ceb		/* hideGenziNew */
#define CODE_SHOW_DIF				0x002f2ad8		/* showGenziDif */
#define CODE_HIDE_DIF				0x00bd255a		/* hideGenziDif */
#define CODE_SHOW_QUEST				0x002f6026		/* showGenziQst */
#define CODE_HIDE_QUEST				0x00bd5aa8		/* hideGenziQst */


typedef struct {
	GV_ACT_EX      actor;

	int            l2d_name;
	int            l2d_handle;

	int			   action_strcode;
	int            busy_flag;
} Work;


static int ReceiveSignal(void * workp, int signal, int value)
{
  Work * work = workp;
  int ret = 0;

  static int genzi_act_list[] = {
    CODE_SHOW_MAIN,
    CODE_SHOW_OPTION,
    CODE_SHOW_NEWGAME,
    CODE_SHOW_DIF,
    CODE_SHOW_QUEST,
    CODE_HIDE_MAIN,
    CODE_HIDE_OPTION,
    CODE_HIDE_NEWGAME,
    CODE_HIDE_DIF,
    CODE_HIDE_QUEST
  };

  switch(signal)
    {
    case NODE_BG_SIGNAL_GZ_SHOW_MAIN:
    case NODE_BG_SIGNAL_GZ_SHOW_OPTION:
    case NODE_BG_SIGNAL_GZ_SHOW_NEWGAME:
    case NODE_BG_SIGNAL_GZ_SHOW_DIF:
    case NODE_BG_SIGNAL_GZ_SHOW_QUEST:
    case NODE_BG_SIGNAL_GZ_HIDE_MAIN:
    case NODE_BG_SIGNAL_GZ_HIDE_OPTION:
    case NODE_BG_SIGNAL_GZ_HIDE_NEWGAME:
    case NODE_BG_SIGNAL_GZ_HIDE_DIF:
    case NODE_BG_SIGNAL_GZ_HIDE_QUEST:
      work->action_strcode =
	genzi_act_list[ signal - NODE_BG_SIGNAL_GZ_SHOW_MAIN ];
      break;
    default:
      ret = GV_DefaultSignalFunc(workp, signal, value);
      break;
    }
  return ret;
}

static void Act(Work * work)
{
	if(L2D_ActionStatus(work->l2d_handle)==L2D_STAT_ACK){
		work->busy_flag=0;
		if(work->action_strcode!=0){
			int stat;

			stat=L2D_EvokeAction(work->l2d_handle,work->action_strcode);

#ifdef DEBUG_MODE
			// printf("<genzi>L2D Stat = %d\n",stat);
#endif

			work->action_strcode=0;
			work->busy_flag=1;
		}
	}
	else{
		work->busy_flag=1;
	}
	/*
	  if(!work->busy_flag){
	  work->action_strcode=CODE_SHOW_NEWGAME;
	  }
	*/
}

static void Die(Work * work)
{
	L2D_ReleaseLayout(work->l2d_handle);
}

static int GetResources(Work * work, int l2d_name)
{
	void * l2d_ptr;
   float safeZoneOffsetY=0;

	printf("node_bg/genzi.l2d: l2d_name = %d\n", l2d_name);
	work->l2d_name   = l2d_name;

	l2d_ptr = GV_GetCache(GV_CacheID(l2d_name, 'o'));
	if(NULL == l2d_ptr)
		l2d_ptr = CDC_GetFileEntry(l2d_name, 'o');

	if(NULL == l2d_ptr) return -1;

	work->l2d_handle = L2D_SetupLayout2(l2d_ptr,
										NODE_BG_CHANL, 6 /* NODE_BG_PRI_GENZI */ ,
										SPR_FLAG_PRIV, 0, safeZoneOffsetY);
  
	// printf("node_bg_genzi.c: l2d handle = %d\n", work->l2d_handle);
	if(work->l2d_handle < 0) return -1;

	L2D_EvokeAction(work->l2d_handle, NODE_BG_DefaultAction);

	work->action_strcode=0;
	work->busy_flag=0;
	GV_SetActorSignalFunc(work, ReceiveSignal);
	return 0;
}

void * NewNodeBG_genzi(int name, int value)
{
	Work * work = codecActorPrio(NODE_ACT_MODE, sizeof(Work), 0xf0);
	if(NULL == work) return NULL;

	GV_SetActor(&(work->actor), Act, Die);
	GV_ActorEX(&(work->actor));
	if(GetResources(work, name)){
		GV_DestroyActor(work);
		return NULL;
    }

	return work;
}
