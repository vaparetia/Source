//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  node_bg_ray.c
  ノード画面背景: メタルギアレイ壁紙

  2001/07/09 Y.Kira
  $Id: node_bg_ray.c,v 1.1.1.3 2002/11/19 11:45:18 Yoshizawa1 Exp $
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


#define CODE_OPENRAY		0x002cf230		/* showRay */
#define CODE_CLOSERAY		0x0073ef71		/* hideRay */


#define X_LIMIT		(10.0f)
#define Y_LIMIT		(10.0f)


#define REGIST_PARAM	0.98f

#define POW_PARAM		0.0010f
#define ADJ_POW_PARAM	0.0008f


typedef struct {
	GV_ACT_EX      actor;

	int            l2d_name;
	int            l2d_handle;

	int			   action_strcode;
	int            busy_flag;

	SPR_OBJ     * root;

	SPR_POS       pos;

	float         x_v,y_v;
} Work;


static int ReceiveSignal(void * workp, int signal, int value)
{
	Work * work = workp;
	int  ret = 0;

	switch(signal){
	case NODE_BG_SIGNAL_OPENRAY:
#if 1
		if(work->l2d_handle>=0){
			if(L2D_EvokeAction(work->l2d_handle,CODE_OPENRAY)!=L2D_STAT_ACK){
				work->action_strcode=CODE_OPENRAY;
			}
		}
#endif
		break;
	case NODE_BG_SIGNAL_CLOSERAY:
#if 1
		if(work->l2d_handle>=0){
			if(L2D_EvokeAction(work->l2d_handle,CODE_CLOSERAY)!=L2D_STAT_ACK){
				work->action_strcode=CODE_CLOSERAY;
			}
		}
#endif
		break;
	default:
	  ret = GV_DefaultSignalFunc(workp, signal, value);
	  break;
	}
	return ret;
}


static void Act(Work * work)
{
	if(work->l2d_handle>=0){

#if 1
		float adjx=0.0f,adjy=0.0f;
		float ax,ay;

		if(GV_PadDataDirect[0].analog_input & GV_PAD_ANALOG_R_USE){
			adjx=(float)((int)(GV_PadDataDirect[0].right_dx)-0x80)/8.0f;
			adjy=(float)((int)(GV_PadDataDirect[0].right_dy)-0x80)/8.0f;
		}

		work->pos.x+=work->x_v;
		work->pos.y+=work->y_v;

		if(work->pos.x<-X_LIMIT) work->pos.x=-X_LIMIT;
		else if(work->pos.x>X_LIMIT) work->pos.x=X_LIMIT;
		if(work->pos.y<-Y_LIMIT) work->pos.y=-Y_LIMIT;
		else if(work->pos.y>Y_LIMIT) work->pos.y=Y_LIMIT;

		/* 処理後の位置を設定 */
		SPR_SetPosEmpty(work->root, &(work->pos));


		ax=0.0f;
		ay=0.0f;

		ax+=-work->pos.x*POW_PARAM;
		ay+=-work->pos.y*POW_PARAM;

		ax+=adjx*ADJ_POW_PARAM;
		ay+=adjy*ADJ_POW_PARAM;

		work->x_v=work->x_v*REGIST_PARAM+ax;
		work->y_v=work->y_v*REGIST_PARAM+ay;
#endif

		if(L2D_ActionStatus(work->l2d_handle)==L2D_STAT_ACK){
			work->busy_flag=0;
			if(work->action_strcode!=0){
				int stat;

				stat=L2D_EvokeAction(work->l2d_handle,work->action_strcode);

#ifdef DEBUG_MODE
				// printf("<plmap>L2D Stat = %d\n",stat);
#endif

				work->action_strcode=0;
				work->busy_flag=1;
			}
		}
		else{
			work->busy_flag=1;
		}
	}
}

static void Die(Work * work)
{
	L2D_ReleaseLayout(work->l2d_handle);
}

static int GetResources(Work * work, int l2d_name)
{
	void * l2d_ptr;

	// printf("node_bg_ray.l2d: l2d_name = %d\n", l2d_name);

	work->l2d_name   = l2d_name;

	l2d_ptr = GV_GetCache(GV_CacheID(l2d_name, 'o'));
	if(NULL == l2d_ptr)
		l2d_ptr = CDC_GetFileEntry(l2d_name, 'o');
	ASSERT(NULL != l2d_ptr);

	work->l2d_handle = -1;

#if 1
	if((GM_TitleMenuStatus & TITLE_MENU_TANKER_CLEARD) &&
	   (GM_TitleMenuStatus & TITLE_MENU_PLANT_CLEARD))
#endif

	{

      float safeZoneOffsetY=0;
		work->l2d_handle = L2D_SetupLayout2(l2d_ptr,
											NODE_BG_CHANL, NODE_BG_PRI_RAY,
											SPR_FLAG_PRIV, 0, safeZoneOffsetY);

		// printf("node_bg_ray.c: l2d handle = %d\n", work->l2d_handle);

		if(work->l2d_handle < 0) return -1;

		/* 基底オブジェクトの取得 */
		work->root = L2D_GetObject(work->l2d_handle, NODE_BG_ROOT);

		L2D_EvokeAction(work->l2d_handle, NODE_BG_DefaultAction);
	}

	// work->action_strcode=CODE_OPENRAY;
	work->action_strcode=0;

	return 0;
}

void * NewNodeBG_ray(int name, int value)
{
	Work * work = codecActorPrio(NODE_ACT_MODE, sizeof(Work), 0xf0);
	if(NULL == work) return NULL;

	GV_SetActor(&(work->actor), Act, Die);
	GV_ActorEX(&(work->actor));
	if(GetResources(work, name)){
		GV_DestroyActor(work);
		return NULL;
	}

	GV_SetActorSignalFunc(work, ReceiveSignal);

	return work;
}
