//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  node_bg_log.c
  ノード画面背景: ログ表示

  2001/07/09 Y.Kira
  $Id: node_bg.c,v 1.1.1.3 2002/11/19 11:45:17 Yoshizawa1 Exp $
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

#define _node_bg_c_
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


void * NewNodeBG_log(int name,    int value);
void * NewNodeBG_chm(int name,    int value);
void * NewNodeBG_nymap(int name,  int value);
void * NewNodeBG_scroll(int name, int value);
void * NewNodeBG_fog(int name,    int value);
void * NewNodeBG_grd(int name,    int value);
void * NewNodeBG_plmap(int name,  int value);
void * NewNodeBG_ray(int name, int value);
void * NewNodeBG_wall(int name, int value);
void * NewNodeBG_genzi(int name, int value);


static struct {
  void * (*Constructor)(int name, int value);
  int    l2d_name;
  int    value;
} child_act_list[] = {
  { NewNodeBG_log,    NODE_BG_L2D_log,       0 },
  { NewNodeBG_scroll, NODE_BG_L2D_scroll,    0 },
  { NewNodeBG_nymap,  NODE_BG_L2D_ny_map_bg, 0 },
  { NewNodeBG_nymap,  NODE_BG_L2D_ny_map,    1 },
  { NewNodeBG_fog,    NODE_BG_L2D_fog1,      0 },
  { NewNodeBG_fog,    NODE_BG_L2D_fog2,      1 },
  { NewNodeBG_fog,    NODE_BG_L2D_fog3,      2 },
  { NewNodeBG_chm,    0,                     0 },
  { NewNodeBG_grd,    NODE_BG_L2D_grd,       0 },
  { NewNodeBG_plmap,  NODE_BG_L2D_plant_map, 0 },
  { NewNodeBG_ray,    NODE_BG_L2D_ray_wall,  0 },
  { NewNodeBG_wall,   NODE_BG_L2D_loop_wall, 0 },
  { NewNodeBG_genzi,  NODE_BG_L2D_genzi,     0 },

  { NULL,             -1 }
};


#define CHILD_ACTS (sizeof(child_act_list)/sizeof(child_act_list[0]))


typedef struct _node_bg_Work {
  GV_ACT_EX    actor;

  SPR_OBJ    * back;
  void       * child_works[ CHILD_ACTS ];

  int          old_flag;
  int          old_bg_flag;
  int		   sample_num;

  CVECTOR      old_fog_color;

  float		   color[3];

  int          finish:1;
} Work;


#define SET_COLOR_2DPRIM(_prim,_col) (*(int*)&(_prim)->sprite.col = (_col))
#define SET_COLOR_2DPRIM2(_prim,_r,_g,_b,_a) \
	SET_COLOR_2DPRIM((_prim),(((_a)<<24) | ((_b)<<16)| ((_g)<<8) | (_r)))

#if 0

#define BACK_R		(18 +4)
#define BACK_G		(22 +4)
#define BACK_B		(20 +4)

#define BACK_U_R	(90/2 +12)
#define BACK_U_G	(24/2 +12)
#define BACK_U_B	(16/2 +12)

#define BACK_D_R	(96/2 +12)
#define BACK_D_G	(80/2 +12)
#define BACK_D_B	( 0/2 +12)

#define BACK_L_R	( 0/2 +12)
#define BACK_L_G	(56/2 +12)
#define BACK_L_B	( 0/2 +12)

#define BACK_R_R	(56/2 +12)
#define BACK_R_G	(16/2 +12)
#define BACK_R_B	(64/2 +12)

#else

#define BACK_R		22
#define BACK_G		26
#define BACK_B		24

#define BACK_R2		18
#define BACK_G2		26
#define BACK_B2		28

#define BACK_U_R	52
#define BACK_U_G	18
#define BACK_U_B	20

#define BACK_D_R	52
#define BACK_D_G	38
#define BACK_D_B	12

#define BACK_L_R	28
#define BACK_L_G	38
#define BACK_L_B	22

#define BACK_R_R	32
#define BACK_R_G	16
#define BACK_R_B	36

#endif


#define ADJ_COLOR		0.025f
#define ADJ_COLOR2		0.15f


static const float sample_rgb[2][5][3]={
	{
		{ BACK_R  ,BACK_G  ,BACK_B  , }, /* None */
		{ BACK_U_R,BACK_U_G,BACK_U_B, }, /* U    */
		{ BACK_D_R,BACK_D_G,BACK_D_B, }, /* D    */
		{ BACK_L_R,BACK_L_G,BACK_L_B, }, /* L    */
		{ BACK_R_R,BACK_R_G,BACK_R_B, }, /* R    */
	},
	{
		{ BACK_R2 ,BACK_G2 ,BACK_B2 , }, /* None */
		{ BACK_U_R,BACK_U_G,BACK_U_B, }, /* U    */
		{ BACK_D_R,BACK_D_G,BACK_D_B, }, /* D    */
		{ BACK_L_R,BACK_L_G,BACK_L_B, }, /* L    */
		{ BACK_R_R,BACK_R_G,BACK_R_B, }, /* R    */
	},
};


/*
 * シグナルハンドラ
 */
static int ReceiveSignal(void * workp, int signal, int value)
{
  Work * work = workp;
  int    ret = 0;

  switch(signal)
    {
    case  NODE_BG_SIGNAL_RIGHT:
    case  NODE_BG_SIGNAL_LEFT:
    case  NODE_BG_SIGNAL_UPPER:
    case  NODE_BG_SIGNAL_LOWER:
    case  NODE_BG_SIGNAL_RIGHTUP:
    case  NODE_BG_SIGNAL_LEFTDOWN:
	case  NODE_BG_SIGNAL_STOP:
	case  NODE_BG_SIGNAL_OPENGRD:
	case  NODE_BG_SIGNAL_CLOSEGRD:
	case  NODE_BG_SIGNAL_OPENPLMAP:
	case  NODE_BG_SIGNAL_CLOSEPLMAP:
	case  NODE_BG_SIGNAL_OPENRAY:
	case  NODE_BG_SIGNAL_CLOSERAY:
	case  NODE_BG_SIGNAL_THUNDER:
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

      /* 全ての子に対し,シグナルを発行する */
      GV_CallChildSignalFunc(work, signal, value);
      ret = 0;
      break;
	case NODE_BG_SIGNAL_CHGFACE:
      /* 全ての子に対し,シグナルを発行する */
	  work->sample_num=value;
      GV_CallChildSignalFunc(work, signal, value);
      ret = 0;
	  break;
    case NODE_BG_SIGNAL_KILL:
      work->finish = 1;
      ret = 0;
      break;

    default:
      ret = GV_DefaultSignalFunc(workp, signal, value);
      break;
    }

  return ret;
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
			if(dx<=MARGIN){
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
		printf("node_bg : %d %d %d\n",(int)(work->color[0]),(int)(work->color[1]),(int)(work->color[2]));
	}
#endif

}

/* ------------------------------------------------------------------------- */

static void Act(Work * work)
{
  ColorChange(work);

  SET_COLOR_2DPRIM2(work->back,
					(int)(work->color[0]),
					(int)(work->color[1]),
					(int)(work->color[2]),
					128);

  if(work->finish)
    {
      GV_CallParentSignalFunc(work, NODE_BG_SIGNAL_DEAD, 0);
      GV_DestroyActor(work);
    }
}

static void Die(Work * work)
{
  DG_Chanls[ NODE_BG_CHANL ].flag = work->old_flag;
  DG_Chanls[ NODE_BG_CHANL ].bg_clear_flag = work->old_bg_flag;

  printf("node_bg die: chanl %d = %d\n", NODE_BG_CHANL, work->old_flag);

  DG_FogColorMaster = work->old_fog_color;

  if(work->back){
    SPR_Destroy_2D_Object(work->back);
    work->back=NULL;
  }
}

static int GetResourcesP(Work * work)
{
  int i;
  void * child;

  work->back = NULL;
  work->old_fog_color = DG_FogColorMaster;
#ifdef BP_PSX2_GCC
  DG_FogColorMaster = (CVECTOR){NODE_FOG_R, NODE_FOG_G, NODE_FOG_B, 0};
#else
  {
	  CVECTOR dmdm = {NODE_FOG_R, NODE_FOG_G, NODE_FOG_B, 0};
	  DG_FogColorMaster = dmdm;
  }
#endif

  work->sample_num=0;

  work->old_flag = DG_Chanls[ NODE_BG_CHANL ].flag;
  work->old_bg_flag = DG_Chanls[ NODE_BG_CHANL ].bg_clear_flag;
  printf("node_bg GetResources(): old_flag = %d\n", work->old_flag);
  DG_SetDrawEnv(&DG_Chanls[ NODE_BG_CHANL ], 0, 0, DRAW_WIDTH, DRAW_HEIGHT);
  
  DG_Chanls[ NODE_BG_CHANL ].flag = 1;
  // JA(AS) -- force the bg_clear flag for the chanl back on, in case of codecs leaving it clear
  DG_Chanls[ NODE_BG_CHANL ].bg_clear_flag = 1;

  for(i = 0; NULL != child_act_list[i].Constructor; i++)
    {
      child = (child_act_list[i].Constructor)(child_act_list[i].l2d_name,
					      child_act_list[i].value);
      work->child_works[i] = child;     /* 一応ワークを保存 */

      if(NULL == child) continue;		/* 作れなかったら無視 */

      GV_SetActorChild(work, child);	/* 子として登録 */
    }

  /* シグナルハンドラの登録 */
  GV_SetActorSignalFunc(work, ReceiveSignal);

  work->finish = 0;


  work->back = SPR_Create_2D_Object(SP_SPRITE,NODE_BG_CHANL,NULL);
  if(work->back == NULL)
    {
#ifdef DEBUG_MODE
      printf("ERR!! Make2DObj[work->title_konami]\n");
#endif
      return -1;
    }

#ifdef BP_PSX2_GCC
  SPR_SetPosSprite(work->back, &(SPR_POS){ 0.0f, 0.0f});
#else
  SPR_SetPosSprite(work->back, (SPR_POS*)&DG_ZeroVector );
#endif
  SPR_SetSizeSprite(work->back, 512.0f, 384.0f );
  work->back->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
  // work->back->head.flags |= SPR_FLAG_ALPHA;
  SPR_SetPriority(work->back,NODE_BG_PRI_BACK);
  SET_COLOR_2DPRIM2(work->back,BACK_R,BACK_G,BACK_B,128);
  SPR_SHOW(work->back);


  work->color[0]=sample_rgb[work->sample_num][0][0];
  work->color[1]=sample_rgb[work->sample_num][0][1];
  work->color[2]=sample_rgb[work->sample_num][0][2];

  return 0;
}


#define ALPHA_MODE		SCE_GS_SET_ALPHA(0,1,0,1,64)
// #define BLEND_COLOR		0x14000000
#define BLEND_COLOR		((0<<0) | (16<<8) | (0<<16) | (28<<24))

void *NewRasterEffectChanlForCodec( int alpha, int color, int chanl );

void * NewNodeBG_P(void)
{
  Work * work = (Work *)codecActorPrio(NODE_ACT_MODE, sizeof(Work), 0xf0);
  if(NULL == work) return NULL;

  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));
  if(GetResourcesP(work))
    {
      GV_DestroyActor(work);
      return NULL;
    }

  GV_SetActorChild(work,NewRasterEffectChanlForCodec((int)ALPHA_MODE,BLEND_COLOR,NODE_BG_CHANL));

  return work;
}

void * NewNodeBG(void)
{
  Work * work = (Work *)GV_NewActorPrio(NODE_ACT_MODE, sizeof(Work), 0xf0);

  if(NULL == work) return NULL;

  GV_SetActor(&(work->actor), Act, Die);
  GV_ActorEX(&(work->actor));

  if(GetResourcesP(work))
    {
      GV_DestroyActor(work);
      return NULL;
    }

  GV_SetActorChild(work,NewRasterEffectChanlForCodec((int)ALPHA_MODE,BLEND_COLOR,NODE_BG_CHANL));

  return work;
}

void * NewNodeBG_SCN(int name, int where)
{
  return NewNodeBG();
}
