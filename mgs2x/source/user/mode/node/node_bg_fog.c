//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  node_bg_fog.c
  ノード画面背景: フォグ

  2001/07/09 Y.Kira
  $Id: node_bg_fog.c,v 1.1.1.3 2002/11/19 11:45:17 Yoshizawa1 Exp $
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
#include "../../kira/2D_lib/util_2d.h"
#include "stream.h"

#define _node_bg_scroll_c_
#include "node_config.h"
#include "node.h"
#include "node_map.h"
#include "node_bg_def.h"
#include "node_bg.h"

#include "libutl.h"


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

#if 0

#define MOMENT_MIN  0.4F
#define MOMENT_MAX  4.0F

#else

#define MOMENT_MIN  0.1F
#define MOMENT_MAX  2.0F

#endif


#define ACC_VAL     0.2F
#define ACC_VAL2    0.14142F
#define ACC_PARAM	0.96f

#define ACC_REGIST	0.98F


#define ADJ_POW_PARAM	0.005f


#define TIMER_LIMIT	6000

#define MIN_RATE	1.0f
#define MAX_RATE	0.3f


#define SCR_X_MARGIN	240.0f
#define SCR_Y_MARGIN	160.0f


/* フォグのルート */
#define NODE_BG_FOG_ROOT	0x0098fc50	/* fogNull */


#define FABS(_f)   (((_f) < 0.0F) ? -(_f) : (_f))

#define MAX_ALPHA  60

typedef struct {
	GV_ACT_EX     actor;

	int           l2d_name;
	int           l2d_handle;

	SPR_OBJ     * root;

	SPR_POS       pos;

	int			  index;

	/* 速度 */
	float         x_moment;
	float         y_moment;

	/* 加速度 */
	float		x_acc;
	float		y_acc;
} Work;


static int ReceiveSignal(void * workp, int signal, int value)
{
	Work * work = workp;
	int ret = 0;
	static const float acc[]={
		ACC_VAL,
		ACC_VAL*ACC_PARAM,
		ACC_VAL*ACC_PARAM*ACC_PARAM,
	};
	static const float acc2[]={
		ACC_VAL2,
		ACC_VAL2*ACC_PARAM,
		ACC_VAL2*ACC_PARAM*ACC_PARAM,
	};

	switch(signal){
    case NODE_BG_SIGNAL_RIGHT:
		work->x_acc=acc[work->index];
		work->y_acc=0.0f;
		break;
    case NODE_BG_SIGNAL_LEFT:
		work->x_acc=-acc[work->index];
		work->y_acc=0.0f;
		break;
    case NODE_BG_SIGNAL_UPPER:
		work->x_acc=0.0f;
		work->y_acc=-acc[work->index];
		break;
    case NODE_BG_SIGNAL_LOWER:
		work->x_acc=0.0f;
		work->y_acc=acc[work->index];
		break;
    case NODE_BG_SIGNAL_RIGHTUP:
		work->x_acc=acc2[work->index];
		work->y_acc=-acc2[work->index];
		break;
    case NODE_BG_SIGNAL_LEFTDOWN:
		work->x_acc=-acc2[work->index];
		work->y_acc=acc2[work->index];
		break;
    case NODE_BG_SIGNAL_STOP:
		work->x_acc=0.0f;
		work->y_acc=0.0f;
		break;
    default:
		ret = GV_DefaultSignalFunc(workp, signal, value);
		break;
    }

	return ret;
}

static void CalcInitPos(Work *work)
{

#if 0
	work->pos.x = (float)((irnd() >> 16) % (int)(SPR_SCRN_WIDTH));
	work->pos.y = (float)((irnd() >> 16) % (int)(SPR_SCRN_HEIGHT));
      
#else
	static const float xy[][2]={
		{   0.0f,  0.0f, },
		{ 500.0f,190.0f, },
		{ 250.0f,380.0f, },
	};

	work->pos.x = xy[work->index][0];
	work->pos.y = xy[work->index][1];
#endif

	SPR_SetPosEmpty(work->root, &(work->pos));

#if 0      
	work->x_moment = (float)((BP_PS2_rand() >> 16) % 15 - 7) / 10.0F;
	work->y_moment = (float)((BP_PS2_rand() >> 16) % 15 - 7) / 10.0F;
#else
	work->x_moment = MOMENT_MIN*frnd();
	work->y_moment = fpu_Sqrt(MOMENT_MIN*MOMENT_MIN-work->x_moment*work->x_moment);
	if(irnd() & 0x00010000) work->y_moment=-work->y_moment;
#endif


}

static void Act(Work * work)
{
	float adjx=0.0f,adjy=0.0f;
	float ax,ay;

	ax=0.0f;
	ay=0.0f;


	if(GV_PadDataDirect[0].analog_input & GV_PAD_ANALOG_R_USE){
		adjx=(float)((int)(GV_PadDataDirect[0].right_dx)-0x80)/8.0f;
		adjy=(float)((int)(GV_PadDataDirect[0].right_dy)-0x80)/8.0f;
	}

	ax+=adjx*ADJ_POW_PARAM;
	ay+=adjy*ADJ_POW_PARAM;


	work->pos.x += work->x_moment;
	work->pos.y += work->y_moment;
      

#if 0
	/* 反射 */
	if(work->pos.x < 0.0F){
		work->x_moment *= -1.0F;
		work->pos.x -= work->pos.x * 2.0F;
    }
	if(work->pos.y < 0.0F){
		work->y_moment *= -1.0F;
		work->pos.y -= work->pos.y * 2.0F;
    }
	if(work->pos.x >= SPR_SCRN_WIDTH){
		work->x_moment *= -1.0F;
		work->pos.x -= (work->pos.x - SPR_SCRN_WIDTH) * 2.0F;
    }
	if(work->pos.y >= SPR_SCRN_HEIGHT){
		work->y_moment *= -1.0F;
		work->pos.y -= (work->pos.y - SPR_SCRN_HEIGHT) * 2.0F;
    }
#else
	/* 上下左右スクロール */
	if(work->pos.x < -SCR_X_MARGIN){
		work->pos.x += (SPR_SCRN_WIDTH+SCR_X_MARGIN*2.0f);
	}
	if(work->pos.y < -SCR_Y_MARGIN){
		work->pos.y += (SPR_SCRN_HEIGHT+SCR_Y_MARGIN*2.0f);
	}
	if(work->pos.x >= (SPR_SCRN_WIDTH+SCR_X_MARGIN)){
		work->pos.x -= (SPR_SCRN_WIDTH+SCR_X_MARGIN*2.0f);
	}
	if(work->pos.y >= (SPR_SCRN_HEIGHT+SCR_Y_MARGIN)){
		work->pos.y -= (SPR_SCRN_HEIGHT+SCR_Y_MARGIN*2.0f);
	}
#endif
	work->x_moment = work->x_moment * ACC_REGIST + work->x_acc+ax;
	work->y_moment = work->y_moment * ACC_REGIST + work->y_acc+ay;

#if 0      
	if(FABS(work->x_moment) <= MOMENT_MIN)
		work->x_moment = work->x_moment / FABS(work->x_moment) * MOMENT_MIN;
      
	if(FABS(work->y_moment) <= MOMENT_MIN)
		work->y_moment = work->y_moment / FABS(work->y_moment) * MOMENT_MIN;
#else
	{
		float len;
		len=work->x_moment*work->x_moment;
		len+=work->y_moment*work->y_moment;
		if(len<=MOMENT_MIN*MOMENT_MIN){
			float a=fpu_Rsqrt(len,MOMENT_MIN);
			work->x_moment*=a;
			work->y_moment*=a;
		}
		else if(len>=MOMENT_MAX*MOMENT_MAX){
			float a=fpu_Rsqrt(len,MOMENT_MAX);
			work->x_moment*=a;
			work->y_moment*=a;
		}
	}
#endif
      
	SPR_SetPosEmpty(work->root, &(work->pos));
}

static void Die(Work * work)
{
	L2D_ReleaseLayout(work->l2d_handle);
}

#if 0

static void all_show(void * workp, SPR_OBJ * obj)
{
	SPR_SHOW(obj);
}

#endif

static int GetResources(Work * work, int l2d_name, int value)
{
   void * l2d_ptr;
   float safeZoneOffsetY=0;

	work->l2d_name = l2d_name;

	l2d_ptr = GV_GetCache(GV_CacheID(l2d_name, 'o'));
	if(NULL == l2d_ptr)
		l2d_ptr = CDC_GetFileEntry(l2d_name, 'o');
	ASSERT(NULL != l2d_ptr);
	work->l2d_handle = L2D_SetupLayout2(l2d_ptr,
										NODE_BG_CHANL, NODE_BG_PRI_FOG,
										SPR_FLAG_PRIV, 0, safeZoneOffsetY);
	if(work->l2d_handle < 0) {
		printf( "L2D not found : mode/node/node_bg_fog.c\n" ) ;
		return -1;
	}

	work->root = L2D_GetObject(work->l2d_handle, NODE_BG_FOG_ROOT);

	CalcInitPos(work);

	work->x_acc=0.0f;
	work->y_acc=0.0f;

	GV_SetActorSignalFunc(work, ReceiveSignal);

	{
		int stat;
		stat=L2D_EvokeAction(work->l2d_handle, NODE_BG_DefaultAction);
		// printf("Fog L2D stat = %d\n",stat);
	}

#if 1
	{
		SPR_OBJ * root = L2D_GetObject(work->l2d_handle, OBJ_ROOT);
		if ( root == NULL ) {
			printf( "OBJ_ROOT<%d> null : mode/node/node_bg_fog.c\n", OBJ_ROOT ) ;
			return -1 ;
		}
		U2D_TreeAlpha(root, 0);
		// U2D_TreeProc(work, root, all_show);
	}
#endif
  
	return 0;
}


  
void * NewNodeBG_fog(int name, int value)
{
	Work * work = codecActorPrio(NODE_ACT_MODE, sizeof(Work), 0xf0);
	if(NULL == work) return NULL;

	work->index=value;

	GV_SetActor(&(work->actor), Act, Die);
	GV_ActorEX(&(work->actor));

	if(GetResources(work, name, value)){
		GV_DestroyActor(work);
		return NULL;
    }

	return work;
}
