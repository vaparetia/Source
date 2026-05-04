//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  node_bg_nymap.c
  ノード画面背景: NY地図

  2001/07/09 Y.Kira
  $Id: node_bg_nymap.c,v 1.1.1.3 2002/11/19 11:45:18 Yoshizawa1 Exp $
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
#include "libutl.h"

#include "g_define.h"
#include "font.h"
#include "libfs.h"

#include "sprite_2d.h"
#include "../../kira/2D_action/layout_2d.h"
#include "stream.h"

#define _node_bg_scroll_c_
#include "node_config.h"
#include "node.h"
#include "node_map.h"
#include "node_bg_def.h"
#include "node_bg.h"


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

typedef struct {
	GV_ACT_EX     actor;

	int           l2d_name;
	int           l2d_handle;

	int           layer;
	int           mode;

	SPR_OBJ     * root;

	SPR_POS       pos;

	float         x_v,y_v;
	float         x_acc,y_acc;
} Work;


#define X_LIMIT		(50.0f-3.0f)
#define Y_LIMIT		(35.0f-3.0f)

#if 0

#define ACC_VAL     0.1F
#define ACC_VAL2    0.07071F

#else

#define ACC_VAL     0.07F
#define ACC_VAL2    0.049497F

#endif

#define POW_PARAM	0.005f

#define REGIST_PARAM	0.97f
#define POW_PARAM2	0.0001f

#define ADJ_POW_PARAM	0.001f


#if 0
#define MAX_V		2.0f
#else
#define MAX_V		1.4f
#endif

#define LAYER_PARAM	0.5f

#define NODE_BG_NymapDemo		GV_StrCode("Action-1")


static const float init_pos[][2]={
    { 5.0f,2.0f, },
    { 0.0f,0.0f, },
};

static int ReceiveSignal(void * workp, int signal, int value)
{
	Work * work = workp;
	int ret = 0;
	switch(signal){
    case NODE_BG_SIGNAL_RIGHT:
		work->mode=1;
		work->x_v=0.0f;
		work->y_v=0.0f;
		work->x_acc=ACC_VAL;
		work->y_acc=0.0f;
		if(!work->layer){
			work->x_acc*=LAYER_PARAM;
			work->y_acc*=LAYER_PARAM;
		}
		break;
    case NODE_BG_SIGNAL_LEFT:
		work->mode=1;
		work->x_v=0.0f;
		work->y_v=0.0f;
		work->x_acc=-ACC_VAL;
		work->y_acc=0.0f;
		if(!work->layer){
			work->x_acc*=LAYER_PARAM;
			work->y_acc*=LAYER_PARAM;
		}
		break;
    case NODE_BG_SIGNAL_UPPER:
		work->mode=1;
		work->x_v=0.0f;
		work->y_v=0.0f;
		work->x_acc=0.0f;
		work->y_acc=-ACC_VAL;
		if(!work->layer){
			work->x_acc*=LAYER_PARAM;
			work->y_acc*=LAYER_PARAM;
		}
		break;
    case NODE_BG_SIGNAL_LOWER:
		work->mode=1;
		work->x_v=0.0f;
		work->y_v=0.0f;
		work->x_acc=0.0f;
		work->y_acc=ACC_VAL;
		if(!work->layer){
			work->x_acc*=LAYER_PARAM;
			work->y_acc*=LAYER_PARAM;
		}
		break;
    case NODE_BG_SIGNAL_RIGHTUP:
		work->mode=1;
		work->x_v=0.0f;
		work->y_v=0.0f;
		work->x_acc=ACC_VAL2;
		work->y_acc=-ACC_VAL2;
		if(!work->layer){
			work->x_acc*=LAYER_PARAM;
			work->y_acc*=LAYER_PARAM;
		}
		break;
    case NODE_BG_SIGNAL_LEFTDOWN:
		work->mode=1;
		work->x_v=0.0f;
		work->y_v=0.0f;
		work->x_acc=-ACC_VAL2;
		work->y_acc=ACC_VAL2;
		if(!work->layer){
			work->x_acc*=LAYER_PARAM;
			work->y_acc*=LAYER_PARAM;
		}
		break;
    case NODE_BG_SIGNAL_STOP:
		work->mode=0;
		if(work->pos.x+work->x_v<-X_LIMIT ||
		   work->pos.x+work->x_v>X_LIMIT) work->x_v=0.0f;
		if(work->pos.y+work->y_v<-Y_LIMIT ||
		   work->pos.y+work->y_v>Y_LIMIT) work->y_v=0.0f;
		work->x_acc=0.0f;
		work->y_acc=0.0f;
		break;
    default:
		ret = GV_DefaultSignalFunc(workp, signal, value);
		break;
    }

	return ret;
}


static void Act(Work * work)
{
	float adjx=0.0f,adjy=0.0f;
	float len;

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

	if(work->mode){
		float ax,ay;
		float dx,dy;

		dx=work->pos.x-init_pos[work->layer][0];
		dy=work->pos.y-init_pos[work->layer][1];

		ax=0.0f;
		ay=0.0f;

		ax+=adjx*ADJ_POW_PARAM;
		ay+=adjy*ADJ_POW_PARAM;

		ax+=-dx*POW_PARAM;
		ay+=-dy*POW_PARAM;

		if(!work->layer){
			ax*=LAYER_PARAM;
			ay*=LAYER_PARAM;
		}

		work->x_v=work->x_v*REGIST_PARAM+ax+work->x_acc;
		work->y_v=work->y_v*REGIST_PARAM+ay+work->y_acc;
	}
	else{
		float ax,ay;
		float dx,dy;

		dx=work->pos.x-init_pos[work->layer][0];
		dy=work->pos.y-init_pos[work->layer][1];

		ax=0.0f;
		ay=0.0f;

		ax+=adjx*ADJ_POW_PARAM;
		ay+=adjy*ADJ_POW_PARAM;

		if(!work->layer){
			ax*=LAYER_PARAM;
			ay*=LAYER_PARAM;
		}

		ax+=-dx*POW_PARAM2;
		ay+=-dy*POW_PARAM2;

		work->x_v=work->x_v*REGIST_PARAM+ax;
		work->y_v=work->y_v*REGIST_PARAM+ay;
	}

	len=work->x_v*work->x_v+work->y_v*work->y_v;
	if(len>=MAX_V*MAX_V){
		len=fpu_Rsqrt(len,MAX_V);
		work->x_v*=len;
		work->y_v*=len;
	}

	// L2D_EvokeAction(work->l2d_handle, NODE_BG_NymapDemo);
}

static void Die(Work * work)
{
	L2D_ReleaseLayout(work->l2d_handle);
}

static int GetResources(Work * work, int l2d_name)
{
   float safeZoneOffsetY=0;
	void * l2d_ptr;
	static const int pri[]={
		NODE_BG_PRI_NYMAP_BG,
		NODE_BG_PRI_NYMAP,
	};

	work->l2d_name = l2d_name;
	l2d_ptr = GV_GetCache(GV_CacheID(l2d_name, 'o'));
	if(NULL == l2d_ptr)
		l2d_ptr = CDC_GetFileEntry(l2d_name, 'o');
	ASSERT(NULL != l2d_ptr);
	work->l2d_handle = L2D_SetupLayout2(l2d_ptr,
										NODE_BG_CHANL, pri[work->layer],
										SPR_FLAG_PRIV, 0, safeZoneOffsetY);
	if(work->l2d_handle < 0) return -1;

	/* 基底オブジェクトの取得 */
	work->root = L2D_GetObject(work->l2d_handle, NODE_BG_ROOT);

	L2D_EvokeAction(work->l2d_handle, NODE_BG_DefaultAction);

	work->mode=0;

	/* 初期位置の設定 */
	work->pos.x = init_pos[work->layer][0];
	work->pos.y = init_pos[work->layer][1];

	work->x_v=0.0f;
	work->y_v=0.0f;

	work->x_acc=0.0f;
	work->y_acc=0.0f;

	SPR_SetPosEmpty(work->root, &(work->pos));

	GV_SetActorSignalFunc(work, ReceiveSignal);

	return 0;
}


  
void * NewNodeBG_nymap(int name, int value)
{
	Work * work = codecActorPrio(NODE_ACT_MODE, sizeof(Work), 0xf0);
	if(NULL == work) return NULL;

	work->layer=value;

	GV_SetActor(&(work->actor), Act, Die);
	GV_ActorEX(&(work->actor));

	if(GetResources(work, name)){
		GV_DestroyActor(work);
		return NULL;
    }

	return work;
}
