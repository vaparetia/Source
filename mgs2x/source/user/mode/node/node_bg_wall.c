//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  node_bg_wall.c
  ノード画面背景: スクロールバックグラウンド

  2001/07/09 Y.Kira
  $Id: node_bg_wall.c,v 1.1.1.3 2002/11/19 11:45:18 Yoshizawa1 Exp $
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


#define CODE_LOOP_ACTION	0x00be4cac		/* roop_action */


typedef struct {
	GV_ACT_EX      actor;

	int            l2d_name;
	int            l2d_handle;

	int			   action_strcode;
	int            busy_flag;
} Work;


static void Act(Work * work)
{
	if(L2D_ActionStatus(work->l2d_handle)==L2D_STAT_ACK){
		work->busy_flag=0;
		if(work->action_strcode!=0){
			int stat;

			stat=L2D_EvokeAction(work->l2d_handle,work->action_strcode);

#ifdef DEBUG_MODE
			// printf("<grd>L2D Stat = %d\n",stat);
#endif

			work->action_strcode=0;
			work->busy_flag=1;
		}
	}
	else{
		work->busy_flag=1;
	}

	if(!work->busy_flag){
		work->action_strcode=CODE_LOOP_ACTION;
	}
}

static void Die(Work * work)
{
	L2D_ReleaseLayout(work->l2d_handle);
}

static int GetResources(Work * work, int l2d_name)
{
   float safeZoneOffsetY = 0;
	void * l2d_ptr;

	// printf("node_bg/grd.l2d: l2d_name = %d\n", l2d_name);
	work->l2d_name   = l2d_name;

	l2d_ptr = GV_GetCache(GV_CacheID(l2d_name, 'o'));
	if(NULL == l2d_ptr)
		l2d_ptr = CDC_GetFileEntry(l2d_name, 'o');
	ASSERT(NULL != l2d_ptr);
	work->l2d_handle = L2D_SetupLayout2(l2d_ptr,
										NODE_BG_CHANL, NODE_BG_PRI_WALL,
										SPR_FLAG_PRIV, 0, safeZoneOffsetY);
  
	// printf("node_bg_grd.c: l2d handle = %d\n", work->l2d_handle);
	if(work->l2d_handle < 0) return -1;

	L2D_EvokeAction(work->l2d_handle, NODE_BG_DefaultAction);

	work->action_strcode=0;
	work->busy_flag=0;

	return 0;
}

void * NewNodeBG_wall(int name, int value)
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
