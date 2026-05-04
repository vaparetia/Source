//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  node_bg_scroll.c
  ノード画面背景: スクロールグリッド

  2001/07/09 Y.Kira
  $Id: node_bg_scroll.c,v 1.1.1.3 2002/11/19 11:45:18 Yoshizawa1 Exp $
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
  GV_ACT_EX      actor;

  int            l2d_name;
  int            l2d_handle;

  int            mode;

  SPR_OBJ      * root;  /* root オブジェクトのポインタ */
  SPR_POS        pos;   /* 表示位置調整 */

  float          x_v,y_v;
  float          x_acc,y_acc;
} Work;


#define X_LIMIT		(60.0f-3.0f)
#define Y_LIMIT		(25.0f-3.0f)

#define ACC_VAL_X	0.1F
#define ACC_VAL2_X	0.07071F
#define ACC_VAL_Y	(ACC_VAL_X*0.5f)
#define ACC_VAL2_Y	(ACC_VAL2_X*0.5f)

#define POW_PARAM_X	0.005f
#define POW_PARAM_Y	(POW_PARAM_X*0.7f)

#define ADJ_POW_PARAM_X		0.0008f
#define ADJ_POW_PARAM_Y		(ADJ_POW_PARAM_X*0.7f)


#define REGIST_PARAM	0.98f
#define POW_PARAM2	0.0001f

#define MAX_V_X		2.0f
#define MAX_V_Y		1.0f


#define NODE_BG_scroll_act		0x00cb5f80		/* Action-2 */


static int ReceiveSignal(void * workp, int signal, int value)
{
	Work * work = workp;
	int ret = 0;

	switch(signal){
    case NODE_BG_SIGNAL_RIGHT:
		work->mode=1;
		work->x_v=0.0f;
		work->x_acc=ACC_VAL_X;
		work->y_v=0.0f;
		work->y_acc=0.0f;
		break;
    case NODE_BG_SIGNAL_LEFT:
		work->mode=1;
		work->x_v=0.0f;
		work->x_acc=-ACC_VAL_X;
		work->y_v=0.0f;
		work->y_acc=0.0f;
		break;
    case NODE_BG_SIGNAL_UPPER:
		work->mode=1;
		work->x_v=0.0f;
		work->x_acc=0.0f;
		work->y_v=0.0f;
		work->y_acc=-ACC_VAL_Y;
		break;
    case NODE_BG_SIGNAL_LOWER:
		work->mode=1;
		work->x_v=0.0f;
		work->x_acc=0.0f;
		work->y_v=0.0f;
		work->y_acc=ACC_VAL_Y;
		break;
    case NODE_BG_SIGNAL_RIGHTUP:
		work->mode=1;
		work->x_v=0.0f;
		work->x_acc=ACC_VAL2_X;
		work->y_v=0.0f;
		work->y_acc=-ACC_VAL2_Y;
		break;
    case NODE_BG_SIGNAL_LEFTDOWN:
		work->mode=1;
		work->x_v=0.0f;
		work->x_acc=-ACC_VAL2_X;
		work->y_v=0.0f;
		work->y_acc=ACC_VAL2_Y;
		break;
    case NODE_BG_SIGNAL_STOP:
		work->mode=0;
		if(work->pos.x+work->x_v<-X_LIMIT ||
		   work->pos.x+work->x_v>X_LIMIT) work->x_v=0.0f;
		work->x_acc=0.0f;
		if(work->pos.y+work->y_v<-Y_LIMIT ||
		   work->pos.y+work->y_v>Y_LIMIT) work->y_v=0.0f;
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

	if(GV_PadDataDirect[0].analog_input & GV_PAD_ANALOG_R_USE){
		adjx=(float)((int)(GV_PadDataDirect[0].right_dx)-0x80)/8.0f;
		adjy=(float)((int)(GV_PadDataDirect[0].right_dy)-0x80)/8.0f;
	}

	if(L2D_STAT_BUSY != L2D_ActionStatus(work->l2d_handle)){
		printf("Scroll Start\n");
		L2D_EvokeAction(work->l2d_handle,NODE_BG_scroll_act);
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

		ax=0.0f;
		ay=0.0f;

		ax+=adjx*ADJ_POW_PARAM_X;
		ay+=adjy*ADJ_POW_PARAM_Y;

		ax+=-work->pos.x*POW_PARAM_X;
		ay+=-work->pos.y*POW_PARAM_Y;

		work->x_v=work->x_v*REGIST_PARAM+ax+work->x_acc;
		work->y_v=work->y_v*REGIST_PARAM+ay+work->y_acc;
	}
	else{
		float ax,ay;

		ax=0.0f;
		ay=0.0f;

		ax+=adjx*ADJ_POW_PARAM_X;
		ay+=adjy*ADJ_POW_PARAM_Y;

		ax+=-work->pos.x*POW_PARAM2;
		ay+=-work->pos.y*POW_PARAM2;

		work->x_v=work->x_v*REGIST_PARAM+ax;
		work->y_v=work->y_v*REGIST_PARAM+ay;
	}

	if(work->x_v>MAX_V_X) work->x_v=MAX_V_X;
	else if(work->x_v<-MAX_V_X) work->x_v=-MAX_V_X;
	if(work->y_v>MAX_V_Y) work->y_v=MAX_V_Y;
	else if(work->y_v<-MAX_V_Y) work->y_v=-MAX_V_Y;
}

static void Die(Work * work)
{
	L2D_ReleaseLayout(work->l2d_handle);  /* レイアウトを開放して終る */
}

static int GetResources(Work * work, int l2d_name)
{
   float safeZoneOffsetY=0;
	void * l2d_ptr;
	work->l2d_name = l2d_name;


	l2d_ptr = GV_GetCache(GV_CacheID(l2d_name, 'o'));
	if(NULL == l2d_ptr)
		l2d_ptr = CDC_GetFileEntry(l2d_name, 'o');
	ASSERT(NULL != l2d_ptr);
   work->l2d_handle = L2D_SetupLayout2(l2d_ptr,
										NODE_BG_CHANL, NODE_BG_PRI_SCROLL,
										SPR_FLAG_PRIV, 0, safeZoneOffsetY);
	if(work->l2d_handle < 0) return -1;

	/* 基底オブジェクトのポインタを取得 */
	work->root = L2D_GetObject(work->l2d_handle, NODE_BG_ROOT);

	/* デフォルトアクション以外に存在しない */
	L2D_EvokeAction(work->l2d_handle, NODE_BG_DefaultAction);

	/* 初期位置 */
	work->pos.x = 0.0F;
	work->pos.y = 0.0F;
	SPR_SetPosEmpty(work->root, &(work->pos));

	work->y_v=0.0f;
	work->y_acc=0.0f;

	work->mode=0;

	GV_SetActorSignalFunc(work, ReceiveSignal);
  
	return 0;
}

void * NewNodeBG_scroll(int name, int value)
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
