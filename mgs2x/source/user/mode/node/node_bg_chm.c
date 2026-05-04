//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  node_bg_chm.c
  ノード画面背景: ベンゼン君

  2001/07/09 Y.Kira
  $Id: node_bg_chm.c,v 1.1.1.3 2002/11/19 11:45:17 Yoshizawa1 Exp $
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

#define MOMENT_MIN  0.0625F
#define MOMENT_RMIN 1.13F

#define MOMENT_MAX  2.8F

#if 0
#define ACC_VAL     0.2F
#define ACC_VAL2    0.14142F
#else
#define ACC_VAL     0.18F
#define ACC_VAL2    0.127279221F
#endif


#define ADJ_POW_PARAM	0.003f


#define ACC_REGIST	0.98F


#define TIMER_LIMIT	6000
#if 0
#define MIN_RATE	1.0f
#define MAX_RATE	0.3f
#else
#define MIN_RATE	0.5f
#define MAX_RATE	1.5f
#endif


#if 0
#define SCR_X_MARGIN	160.0f
#define SCR_Y_MARGIN	96.0f
#else
#define SCR_X_MARGIN	290.0f
#define SCR_Y_MARGIN	160.0f
#endif


// #define N_L2DS		4
#define N_L2DS		3


#define FABS(_f)   (((_f) < 0.0F) ? -(_f) : (_f))

#define MAX_ALPHA  60

typedef struct {
    int           l2d_name;
    int           l2d_handle;

    SPR_OBJ     * root;

    SPR_POS       pos;

    /* 速度 */
    float         x_moment;
    float         y_moment;

    /* フェードイン/アウト関連 */
    int           fade_mode;
    int           fade_time;
    int           fade_cnt;

    /* 表示開始ディレイ */
    int           step;
    int           delay_time;

    /* 奥行き計算用 */
    int		timer;
} L2DWork;

typedef struct {
    GV_ACT_EX	actor;

	int sample_num;

    L2DWork		l2dwork[N_L2DS];

    SPR_POS		home_pos;

	int			index;

	float		x_moment;
	float		y_moment;

    /* 加速度 */
    float		x_acc;
    float		y_acc;

	float color[3];
} Work;


enum {
    STEP_INIT,
    STEP_DISP,
	STEP_FINISH,
};


#define ADJ_COLOR		0.1f
#define ADJ_COLOR2		0.3f


static const float sample_rgb[2][5][3]={

#if 0
	{ 128/2, 10/2,  0/2, }, /* None */
	{   0/2,112/2,  0/2, }, /* U    */
	{ 112/2, 32/2,128/2, }, /* D    */
	{ 192/2,160/2,  0/2, }, /* L    */
	{ 180/2, 48/2, 32/2, }, /* R    */
#else
	{ /* snake */
		{ 124, 10,  0, }, /* None */
		{  32, 36, 34, }, /* U    */
		{  44, 14, 48, }, /* D    */
		{  80, 64,  0, }, /* L    */
		{   0,  8, 56, }, /* R    */
	},
	{ /* raiden */
		{  12, 48, 24, }, /* None */
		{  32, 36, 34, }, /* U    */
		{  52, 14, 48, }, /* D    */
		{  80, 64,  0, }, /* L    */
		{   0,  8, 56, }, /* R    */
	},
#endif

};


/*
 * 指定されたオブジェクト以下のオブジェクトのカラー値に,
 * 指定された値を設定して回る。元の値は無視する。
 */
static int U2D_TreeColor(SPR_OBJ * obj,int red,int green,int blue)
{
	SPR_OBJ * child;
	int cnt = 1;

	switch(obj->head.id){
    case SP_EMPTY: break;
    case SP_POINT:
		obj->point.col.r = red;
		obj->point.col.g = green;
		obj->point.col.b = blue;
		break;
    case SP_LINE:
		obj->line.col[0].r = red;
		obj->line.col[0].g = green;
		obj->line.col[0].b = blue;
		obj->line.col[1].r = red;
		obj->line.col[1].g = green;
		obj->line.col[1].b = blue;
		break;
    case SP_LINESTRIP:
		{
			int i;

			for(i = 0; i < obj->linestrip.pcnt; i++){
				obj->linestrip.plist[i].col.r = red;
				obj->linestrip.plist[i].col.g = green;
				obj->linestrip.plist[i].col.b = blue;
			}
		}
		break;
    case SP_BOX:
    case SP_BOX_F:
		obj->box.col.r = red;
		obj->box.col.g = green;
		obj->box.col.b = blue;
		break;
    case SP_SPRITE:
    case SP_SPRITE_F:
		obj->sprite.col.r = red;
		obj->sprite.col.g = green;
		obj->sprite.col.b = blue;
		break;
      
    case SP_ZOOM:
    case SP_ZOOM_F:
		obj->zoom.col.r = red;
		obj->zoom.col.g = green;
		obj->zoom.col.b = blue;
		break;

    case SP_SPIN:
    case SP_SPIN_F:
		obj->spin.col.r = red;
		obj->spin.col.g = green;
		obj->spin.col.b = blue;
		break;
      
    case SP_POLY:
    case SP_POLY_F:
		{
			int i;
			for(i = 0; i < 4; i++){
				obj->poly.col[i].r = red;
				obj->poly.col[i].g = green;
				obj->poly.col[i].b = blue;
			}
		}
		break;

    case SP_TRIANGLE:
		{
			int i;
			for(i = 0; i < 3; i++){
				obj->triangle.col[i].r = red;
				obj->triangle.col[i].g = green;
				obj->triangle.col[i].b = blue;
			}
		}
		break;
	default:
		break;
    }

	/* 自分に子がいるなら、その子を全て処理する */
	for(child = obj->head.child; NULL != child; child = child->head.next)
		cnt += U2D_TreeColor(child,red,green,blue);
  
	return cnt;
}


#define MARGIN		16
#define INT_ABS(x)	(((x)<0) ? -(x) : (x))


static void ColorChange(Work *work)
{
	int t_color_num=0;
	float rate=0.0f;

	if(GV_PadDataDirect[0].analog_input & GV_PAD_ANALOG_R_USE){
		float adjx,adjy;
		int dx,dy;

		dx=(int)(GV_PadDataDirect[0].right_dx)-0x80;
		dy=(int)(GV_PadDataDirect[0].right_dy)-0x80;

		adjx=(float)dx/8.0f;
		adjy=(float)dy/8.0f;

#if 0
		if(dx>-MARGIN && dx<MARGIN){
			rate=fpu_Abs(adjy);
			if(dy<=-MARGIN){
				t_color_num=1;
			}
			else if(dy>=MARGIN){
				t_color_num=2;
			}
			else{
				t_color_num=0;
			}
		}
		else if(dy>-MARGIN && dy<MARGIN){
			rate=fpu_Abs(adjx);
			if(dx<=-MARGIN){
				t_color_num=3;
			}
			else if(dx>=MARGIN){
				t_color_num=4;
			}
			else{
				t_color_num=0;
			}
		}
		else{
			t_color_num=0;
		}
#else
		if(dx>-MARGIN && dx<MARGIN && dy>-MARGIN && dy<MARGIN){
			t_color_num=0;
		}
		else if(INT_ABS(dx)>INT_ABS(dy)){
			rate=fpu_Abs(adjx);
			if(dx<0){
				t_color_num=3;
			}
			else{
				t_color_num=4;
			}
		}
		else{
			rate=fpu_Abs(adjy);
			if(dy<0){
				t_color_num=1;
			}
			else{
				t_color_num=2;
			}
		}
#endif
	}

	if(t_color_num){
		float r,g,b;
		float len;

		r=sample_rgb[work->sample_num][t_color_num][0]-work->color[0];
		g=sample_rgb[work->sample_num][t_color_num][1]-work->color[1];
		b=sample_rgb[work->sample_num][t_color_num][2]-work->color[2];

		len=r*r+g*g+b*b;
		len=fpu_Rsqrt(len,rate*ADJ_COLOR);
		if(len<1.0f){
			r*=len;
			g*=len;
			b*=len;
		}

		work->color[0]+=r;
		work->color[1]+=g;
		work->color[2]+=b;
	}
	else{
		float r,g,b;
		float len;

		r=sample_rgb[work->sample_num][t_color_num][0]-work->color[0];
		g=sample_rgb[work->sample_num][t_color_num][1]-work->color[1];
		b=sample_rgb[work->sample_num][t_color_num][2]-work->color[2];

		len=r*r+g*g+b*b;
		len=fpu_Rsqrt(len,ADJ_COLOR2);
		if(len<1.0f){
			r*=len;
			g*=len;
			b*=len;
		}

		work->color[0]+=r;
		work->color[1]+=g;
		work->color[2]+=b;
	}

#ifdef DEBUG_MODE
	if(GV_PadDataDirect[0].press & PAD_STA){
		printf("node_bg_chm : %d %d %d\n",
			   (int)(work->color[0]),(int)(work->color[1]),(int)(work->color[2]));
	}
#endif
}

static int ReceiveSignal(void * workp, int signal, int value)
{
    Work * work = workp;
    int ret = 0;

    switch(signal){
    case NODE_BG_SIGNAL_RIGHT:
		work->x_acc=ACC_VAL;
		work->y_acc=0.0f;
		break;
    case NODE_BG_SIGNAL_LEFT:
		work->x_acc=-ACC_VAL;
		work->y_acc=0.0f;
		break;
    case NODE_BG_SIGNAL_UPPER:
		work->x_acc=0.0f;
		work->y_acc=-ACC_VAL;
		break;
    case NODE_BG_SIGNAL_LOWER:
		work->x_acc=0.0f;
		work->y_acc=ACC_VAL;
		break;
    case NODE_BG_SIGNAL_RIGHTUP:
		work->x_acc=ACC_VAL2;
		work->y_acc=-ACC_VAL2;
		break;
    case NODE_BG_SIGNAL_LEFTDOWN:
		work->x_acc=-ACC_VAL2;
		work->y_acc=ACC_VAL2;
		break;
    case NODE_BG_SIGNAL_STOP:
		work->x_acc=0.0f;
		work->y_acc=0.0f;
		break;
	case NODE_BG_SIGNAL_CHGFACE:
		if(work->sample_num!=value){
			work->color[0]=sample_rgb[value][0][0];
			work->color[1]=sample_rgb[value][0][1];
			work->color[2]=sample_rgb[value][0][2];
		}
		work->sample_num=value;
		break;
    default:
		ret = GV_DefaultSignalFunc(workp, signal, value);
		break;
    }

    return ret;
}

/* 桂馬飛び位置配置
   適当に決めました */
static int xy[][2]={
	{ SPR_SCRN_WIDTH*2.0f/4.0f,SPR_SCRN_HEIGHT*2.0f/4.0f, },
	{ SPR_SCRN_WIDTH*0.0f/4.0f,SPR_SCRN_HEIGHT*3.0f/4.0f, },

	{ SPR_SCRN_WIDTH*1.0f/4.0f,SPR_SCRN_HEIGHT*1.0f/4.0f, },
	{ SPR_SCRN_WIDTH*3.0f/4.0f,SPR_SCRN_HEIGHT*0.0f/4.0f, },
	{ SPR_SCRN_WIDTH*2.0f/4.0f,SPR_SCRN_HEIGHT*3.0f/4.0f, },

	{ SPR_SCRN_WIDTH*0.0f/4.0f,SPR_SCRN_HEIGHT*0.0f/4.0f, },
	{ SPR_SCRN_WIDTH*1.0f/4.0f,SPR_SCRN_HEIGHT*2.0f/4.0f, },
	{ SPR_SCRN_WIDTH*3.0f/4.0f,SPR_SCRN_HEIGHT*3.0f/4.0f, },
	{ SPR_SCRN_WIDTH*0.0f/4.0f,SPR_SCRN_HEIGHT*1.0f/4.0f, },

	{ SPR_SCRN_WIDTH*2.0f/4.0f,SPR_SCRN_HEIGHT*0.0f/4.0f, },
	{ SPR_SCRN_WIDTH*3.0f/4.0f,SPR_SCRN_HEIGHT*2.0f/4.0f, },
	{ SPR_SCRN_WIDTH*1.0f/4.0f,SPR_SCRN_HEIGHT*3.0f/4.0f, },
	{ SPR_SCRN_WIDTH*2.0f/4.0f,SPR_SCRN_HEIGHT*1.0f/4.0f, },

	{ SPR_SCRN_WIDTH*0.0f/4.0f,SPR_SCRN_HEIGHT*2.0f/4.0f, },
	{ SPR_SCRN_WIDTH*1.0f/4.0f,SPR_SCRN_HEIGHT*0.0f/4.0f, },
};

static void CalcInitPos(Work *mwork,L2DWork *work)
{
	int i=mwork->index;

	work->pos.x=mwork->home_pos.x+(float)(xy[i][0]);
	work->pos.y=mwork->home_pos.y+(float)(xy[i][1]);

	if(work->pos.x>=SPR_SCRN_WIDTH) work->pos.x-=SPR_SCRN_WIDTH;
	if(work->pos.y>=SPR_SCRN_HEIGHT) work->pos.y-=SPR_SCRN_HEIGHT;

	mwork->index++;
	if(mwork->index>=15) mwork->index=0;

#if 0
    work->pos.x = (float)((irnd() >> 16) % (int)(SPR_SCRN_WIDTH));
    work->pos.y = (float)((irnd() >> 16) % (int)(SPR_SCRN_HEIGHT));
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

/*
 * レイアウトからのシグナルハンドラ
 */
static void LayoutSignal(void * workp, int signal, int value)
{
    L2DWork * work = workp;

    switch(signal){
    case KEY_fdin:
		work->fade_cnt = 0;
		work->fade_time = value;
		work->fade_mode = 1;
		break;
      
    case KEY_fdout:
		work->fade_cnt = 0;
		work->fade_time = value;
		work->fade_mode = -1;
		break;
    case KEY_actEnd:
		work->step=STEP_FINISH;
		break;
    }
}


static void LayoutAct(Work *work,L2DWork *l2dw,float adjx,float adjy)
{
	float ax,ay;

	ax=0.0f;
	ay=0.0f;

	ax+=adjx*ADJ_POW_PARAM;
	ay+=adjy*ADJ_POW_PARAM;

    switch(l2dw->step){
    case STEP_INIT:
		U2D_TreeAlpha(l2dw->root, 0);
		l2dw->delay_time -= TIME_BASE;
		if(l2dw->delay_time <= 0) l2dw->step = STEP_DISP;
		break;
	case STEP_FINISH:
		CalcInitPos(work,l2dw);
		l2dw->step=STEP_DISP;

    case STEP_DISP:
		/* アクションを実行していなければ、規定のアクションを実行する */
		if(L2D_STAT_BUSY != L2D_ActionStatus(l2dw->l2d_handle)){
			L2D_EvokeAction(l2dw->l2d_handle, ACTION_nodebg_chmAct);
			l2dw->timer=0;
		}

		/* フェードイン/アウト中であれば実行 */
		if(l2dw->fade_mode){
			int alpha;
	  
			l2dw->fade_cnt += TIME_BASE;
			alpha = l2dw->fade_cnt * MAX_ALPHA / l2dw->fade_time;
			if(alpha > MAX_ALPHA) alpha = MAX_ALPHA;
			if(l2dw->fade_mode < 0) alpha = MAX_ALPHA - alpha;
	  
			/* ROOT 以下全てのオブジェクトのアルファ値を設定 */
			U2D_TreeAlpha(l2dw->root, alpha);
			if(l2dw->fade_cnt >= l2dw->fade_time)
				l2dw->fade_mode = 0;
		}

		U2D_TreeColor(l2dw->root,
					  (int)(work->color[0]),
					  (int)(work->color[1]),
					  (int)(work->color[2]));

		l2dw->pos.x += l2dw->x_moment;
		l2dw->pos.y += l2dw->y_moment;
      

		/* 上下左右スクロール */
		if(l2dw->pos.x < -SCR_X_MARGIN){
			l2dw->pos.x += (SPR_SCRN_WIDTH+SCR_X_MARGIN*2.0f);
		}
		if(l2dw->pos.y < -SCR_Y_MARGIN){
			l2dw->pos.y += (SPR_SCRN_HEIGHT+SCR_Y_MARGIN*2.0f);
		}
		if(l2dw->pos.x >= (SPR_SCRN_WIDTH+SCR_X_MARGIN)){
			l2dw->pos.x -= (SPR_SCRN_WIDTH+SCR_X_MARGIN*2.0f);
		}
		if(l2dw->pos.y >= (SPR_SCRN_HEIGHT+SCR_Y_MARGIN)){
			l2dw->pos.y -= (SPR_SCRN_HEIGHT+SCR_Y_MARGIN*2.0f);
		}

		{
			float rate;
			rate=MIN_RATE+(MAX_RATE-MIN_RATE)*(float)(l2dw->timer)/(float)TIMER_LIMIT;

			l2dw->x_moment = l2dw->x_moment * ACC_REGIST + (work->x_acc+ax)*rate;
			l2dw->y_moment = l2dw->y_moment * ACC_REGIST + (work->y_acc+ay)*rate;
		}

		{
			float len;
			len=l2dw->x_moment*l2dw->x_moment;
			len+=l2dw->y_moment*l2dw->y_moment;
			if(len<=MOMENT_MIN*MOMENT_MIN){
				float a=1.0f;  //fpu_Rsqrt(len,MOMENT_MIN);
				l2dw->x_moment*=a;
				l2dw->y_moment*=a;
			}
			else if(len>=MOMENT_MAX*MOMENT_MAX){
				float a=1.0f;//fpu_Rsqrt(len,MOMENT_MAX);
				l2dw->x_moment*=a;
				l2dw->y_moment*=a;
			}
		}
      
		SPR_SetPosEmpty(l2dw->root, &(l2dw->pos));

		l2dw->timer+=TIME_BASE;
		break;
    }
}

static void Act(Work * work)
{
	float adjx=0.0f,adjy=0.0f;
    int i;

	ColorChange(work);

	if(GV_PadDataDirect[0].analog_input & GV_PAD_ANALOG_R_USE){
		adjx=(float)((int)(GV_PadDataDirect[0].right_dx)-0x80)/8.0f;
		adjy=(float)((int)(GV_PadDataDirect[0].right_dy)-0x80)/8.0f;
	}


    for(i=0;i<N_L2DS;i++){
		LayoutAct(work,&(work->l2dwork[i]),adjx,adjy);
    }

	work->home_pos.x+=work->x_moment;
	work->home_pos.y+=work->y_moment;
      
	/* 上下左右スクロール */
	if(work->home_pos.x<0.0f){
		work->home_pos.x+=SPR_SCRN_WIDTH;
	}
	if(work->home_pos.y<0.0f){
		work->home_pos.y+=SPR_SCRN_HEIGHT;
	}
	if(work->home_pos.x>=SPR_SCRN_WIDTH){
		work->home_pos.x-=SPR_SCRN_WIDTH;
	}
	if(work->home_pos.y>=SPR_SCRN_HEIGHT){
		work->home_pos.y-=SPR_SCRN_HEIGHT;
	}

	work->x_moment=work->x_moment*ACC_REGIST+work->x_acc;
	work->y_moment=work->y_moment*ACC_REGIST+work->y_acc;

	{
		float len;
		len=work->x_moment*work->x_moment;
		len+=work->y_moment*work->y_moment;
		if(len<=MOMENT_MIN*MOMENT_MIN){
			float a=1.0f;  //fpu_Rsqrt(len,MOMENT_MIN);
			work->x_moment*=a;
			work->y_moment*=a;
		}
		else if(len>=MOMENT_MAX*MOMENT_MAX){
			float a=1.0f;  //fpu_Rsqrt(len,MOMENT_MAX);
			work->x_moment*=a;
			work->y_moment*=a;
		}
	}
}

static void Die(Work * work)
{
    int i;
    for(i=0;i<N_L2DS;i++){
		if(work->l2dwork[i].l2d_handle>=0){
			L2D_ReleaseLayout(work->l2dwork[i].l2d_handle);
		}
    }
}

static void all_show(void * workp, SPR_OBJ * obj)
{
    SPR_SHOW(obj);
}

static int L2DInit(Work *mwork,L2DWork *work,int l2d_name,int value)
{
    void * l2d_ptr;
    float safeZoneOffsetY=0;

    work->fade_mode = 0;
    work->step = STEP_INIT;
    work->delay_time = 600 * value;

    work->l2d_name = l2d_name;

    l2d_ptr = GV_GetCache(GV_CacheID(l2d_name, 'o'));
    if(NULL == l2d_ptr)
		l2d_ptr = CDC_GetFileEntry(l2d_name, 'o');
    ASSERT(NULL != l2d_ptr);
    work->l2d_handle = L2D_SetupLayout2(l2d_ptr,
					NODE_BG_CHANL, NODE_BG_PRI_CHM,
					SPR_FLAG_PRIV, 0, safeZoneOffsetY);
    if(work->l2d_handle < 0) return -1;

    work->root = L2D_GetObject(work->l2d_handle, OBJ_Prog);

    CalcInitPos(mwork,work);

    L2D_SetSignalHandle(work->l2d_handle,work,LayoutSignal);

    L2D_EvokeAction(work->l2d_handle, NODE_BG_DefaultAction);
    {
		SPR_OBJ * root = L2D_GetObject(work->l2d_handle, OBJ_ROOT);
    
		U2D_TreeAlpha(root, 0);
		U2D_TreeProc(work, root, all_show);
    }

    return 0;
}

static int GetResources(Work * work, int l2d_name, int sig_value)
{
    static const int l2d_strcode[]={
		NODE_BG_L2D_chm1,
		NODE_BG_L2D_chm2,
		NODE_BG_L2D_chm3,
		NODE_BG_L2D_chm2,
    };
    static const int value[]={
		// 0,2,5,8,
		0,3,7,
    };
    int i;

    for(i=0;i<N_L2DS;i++){
		work->l2dwork[i].l2d_handle=-1;
    }

    for(i=0;i<N_L2DS;i++){
		if(L2DInit(work,&(work->l2dwork[i]),l2d_strcode[i],value[i])<0) return -1;
    }

	work->index=0;

	work->x_moment=0.0f;
	work->y_moment=0.0f;

    work->x_acc=0.0f;
    work->y_acc=0.0f;

	work->sample_num=sig_value;

	work->color[0]=sample_rgb[work->sample_num][0][0];
	work->color[1]=sample_rgb[work->sample_num][0][1];
	work->color[2]=sample_rgb[work->sample_num][0][2];

    GV_SetActorSignalFunc(work, ReceiveSignal);
  
    return 0;
}

void * NewNodeBG_chm(int name, int value)
{
    Work * work = codecActorPrio(NODE_ACT_MODE, sizeof(Work), 0xf0);
    if(NULL == work) return NULL;

    GV_SetActor(&(work->actor), Act, Die);
    GV_ActorEX(&(work->actor));

    if(GetResources(work, name, value)){
		GV_DestroyActor(work);
		return NULL;
    }

    return work;
}
