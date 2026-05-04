//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    操作説明説明表示
    cntrl_exp.c

	2000/11/03 T.Shibata

	$Id: ctrl_exp.c,v 1.1.1.3 2002/11/19 11:48:43 Yoshizawa1 Exp $
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

#include 	"libutl.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

//#include	"camera.h"
#include 	"sprite_2d.h"
#include	"../util/ts_util.h"

//#define	ACTOR_PRIO		(254)


#define TRI_CODE		(GV_StrCode("2D_tex"))
//#define TRI_CODE		(GV_StrCode("staff"))

#define SET_COLOR_2DPRIM(_prim,_col) (*(int*)&(_prim)->sprite.col = (_col))
#define SET_COLOR_2DPRIM2(_prim,_r,_g,_b,_a) SET_COLOR_2DPRIM((_prim),(((_a)<<24) | ((_b)<<16)| ((_g)<<8) | (_r)))
#define SET_COLOR_2DPRIM3(_prim,_a,_col) SET_COLOR_2DPRIM((_prim),(((_a)<<24) | (_col)))

#define FADEIN_TIME		(8)
#define FADEOUT_TIME	(8)

#define	TRN2DFIG_Y(_y)	(((_y)*384.0f)/448.0f)

#define CHANL_NUM	(0)

typedef	struct {
	GV_ACT_EX		actor;
	SPR_OBJ   		*sprite[6];

	short			step;
	short			time;
	short	   		flag;
	short			eng;
} Work;

enum {
	STEP_INV_WAIT = 0,
	STEP_FADEIN,
	STEP_VIS_WAIT,
	STEP_FADEOUT,
};

static int Tex_Code[6] = {
	-1,
	-1,
	15452192,
	15517728,
	15583264,
	15648800
};

static Work *g_Explain = NULL;

static void Act( Work *work )
{
	int		alpha = 0;

	if(GM_IsGameOver()){
		GV_DestroyActor( work ) ;
		SPR_HIDE(work->sprite[0]);
		SPR_HIDE(work->sprite[1]);
		SPR_HIDE(work->sprite[2]);
		SPR_HIDE(work->sprite[3]);
		SPR_HIDE(work->sprite[4]);
		SPR_HIDE(work->sprite[5]);
		return;
	}

	switch(work->step){
	case STEP_INV_WAIT:
		if(GV_PauseLevel & GV_PAUSE_PAUSE){
			//ポーズ開始

#ifdef DEBUG_MODE
			if(!GM_DebugModeEnable){
				work->flag = 0;
			}
//			if(work->flag) return;
#else
			work->flag = 0;
#endif

			work->step = STEP_FADEIN;
			work->time = FADEIN_TIME;
			SPR_SHOW(work->sprite[0]);
			SPR_SHOW(work->sprite[1]);
			SPR_SHOW(work->sprite[2]);
			SPR_SHOW(work->sprite[3]);
			SPR_SHOW(work->sprite[4]);
			SPR_SHOW(work->sprite[5]);
		}
		break;
	case STEP_FADEIN:
		alpha = (int)(128.0f*(float)(FADEIN_TIME - work->time)/(float)FADEIN_TIME);

		if(--work->time < 0){
			work->step = STEP_VIS_WAIT;
			work->time = 0;
		}

		break;
	case STEP_VIS_WAIT:
		alpha = 128;

//		if( GV_PadDataDirect[0].press & PAD_SEL ) work->flag |= 1;
		if( GV_PadDataDirect[0].press & PAD_SEL ) work->flag = 1 - work->flag;

		if(!(GV_PauseLevel&GV_PAUSE_PAUSE)){
			work->step = STEP_FADEOUT;
			work->time = FADEOUT_TIME;
		}

		break;
	case STEP_FADEOUT:
		alpha = (int)(128.0f*(float)work->time/(float)FADEOUT_TIME);

		if( --work->time < 0){
			work->step = STEP_INV_WAIT;
			alpha = 0;
			SPR_HIDE(work->sprite[0]);
			SPR_HIDE(work->sprite[1]);
			SPR_HIDE(work->sprite[2]);
			SPR_HIDE(work->sprite[3]);
			SPR_HIDE(work->sprite[4]);
			SPR_HIDE(work->sprite[5]);
#ifdef DEBUG_MODE
			if(!GM_DebugModeEnable){
				work->flag = 0;
			}else{
				work->flag = 1;
			}
#else
			work->flag = 0;
#endif
		}
		break;
	default:
		printf("STEP ERR!!!\n");
	}

	SET_COLOR_2DPRIM3(work->sprite[0],alpha,0x00000000);
	SET_COLOR_2DPRIM3(work->sprite[1],(alpha>>3)*6,0x00171613);
	SET_COLOR_2DPRIM3(work->sprite[2],alpha,0x00d0d0d0);
	SET_COLOR_2DPRIM3(work->sprite[3],alpha,0x00d0d0d0);
	SET_COLOR_2DPRIM3(work->sprite[4],alpha,0x00d0d0d0);
	SET_COLOR_2DPRIM3(work->sprite[5],alpha,0x00d0d0d0);

	if(work->flag&1){
		SPR_HIDE(work->sprite[0]);
		SPR_HIDE(work->sprite[1]);
		SPR_HIDE(work->sprite[2]);
		SPR_HIDE(work->sprite[3]);
		SPR_HIDE(work->sprite[4]);
		SPR_HIDE(work->sprite[5]);
	}else{
		SPR_SHOW(work->sprite[0]);
		SPR_SHOW(work->sprite[1]);
		SPR_SHOW(work->sprite[2]);
		SPR_SHOW(work->sprite[3]);
		SPR_SHOW(work->sprite[4]);
		SPR_SHOW(work->sprite[5]);
	}
	if(work->eng){
		SPR_HIDE(work->sprite[0]);
		SPR_HIDE(work->sprite[2]);
	}
}

static void Die( Work *work )
{
	int i;
	for( i = 0; i < 6; i++ ){
		if(work->sprite[i]) SPR_Destroy_2D_Object(work->sprite[i]);
	}
	g_Explain = NULL;
}

static int GetResources( Work *work )
{
	static FVECTOR Sprite_Data[6] = {
		{  0.0f, TRN2DFIG_Y(  0.0f), 512.0f, TRN2DFIG_Y( 42.0f)},
		{  0.0f, 0.0f, 512.0f, 384.0f},

		{ 512.0f - 169.0f, TRN2DFIG_Y( 18.0f), 140.0f, TRN2DFIG_Y( 16.0f)},
		{ 34.0f, TRN2DFIG_Y( 53.0f), 414.0f, TRN2DFIG_Y( 78.0f)},
		{ 32.0f, TRN2DFIG_Y(158.0f), 456.0f, TRN2DFIG_Y(128.0f)},
		{ 34.0f, TRN2DFIG_Y(313.0f), 448.0f, TRN2DFIG_Y(104.0f)},
	};
	FVECTOR	*data = Sprite_Data;
	SPR_OBJ	*sprite;
	int		i,tri_code;

#ifdef DEBUG_MODE
	if(GM_DebugModeEnable){
		work->flag = 1;
	}else{
		work->flag = 0;
	}
#else
	work->flag = 0;
#endif

	work->eng = GCL_GetOptionValue( 'e', 0 );

	if(work->eng){
		Sprite_Data[3].vx = 16.0f;
		Sprite_Data[3].vy = TRN2DFIG_Y( 18.0f);
		Sprite_Data[3].vz = 482.0f;
		Sprite_Data[3].vw = TRN2DFIG_Y( 98.0f);

		Sprite_Data[4].vx = 16.0f;
		Sprite_Data[4].vy = TRN2DFIG_Y(118.0f);
		Sprite_Data[4].vz = 486.0f;
		Sprite_Data[4].vw = TRN2DFIG_Y(164.0f);

		Sprite_Data[5].vx = 16.0f;
		Sprite_Data[5].vy = TRN2DFIG_Y(296.0f);
		Sprite_Data[5].vz = 482.0f;
		Sprite_Data[5].vw = TRN2DFIG_Y(136.0f);
	}

	work->step = STEP_INV_WAIT;

	tri_code = SPR_LoadTexture(TRI_CODE);

	for( i = 0; i < 6; i++ ){
		sprite = work->sprite[i] = SPR_Create_2D_Object(SP_SPRITE, 4, NULL);
		if(sprite == NULL){ printf("ERR!! Make2DObj[sprite]\n"); return -1; }
		if(Tex_Code[i] > 0 && !(work->eng && i == 2)) SPR_ObjSetTexture(sprite, Tex_Code[i], tri_code);

		SPR_SetPosSprite(sprite, (SPR_POS*)data);
		SPR_SetSizeSprite(sprite, data->vz, data->vw );
		sprite->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
		sprite->head.flags |= SPR_FLAG_ALPHA;
		SET_COLOR_2DPRIM(sprite,0);

		//SPR_SHOW(sprite);

		data++;
	}

	SPR_SetPriority( work->sprite[0], 0);
	SPR_SetPriority( work->sprite[1], 0);
	SPR_SetPriority( work->sprite[2], 1);
	SPR_SetPriority( work->sprite[3], 1);
	SPR_SetPriority( work->sprite[4], 1);
	SPR_SetPriority( work->sprite[5], 1);

	return 0;
}

void *NewOpenCntrlExp( int name, int map )
{
	Work	*work;

	if(g_Explain) return NULL;

	work = (Work*)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
								  sizeof( Work ), 0 ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
        }
	}
	g_Explain = work;

	return work ;
}

