//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	3dprint.c
	ＶＲステージ空中文字
	2002/01/31 K.Sigeno
	$Id: 3dprint.c,v 1.1.1.3 2002/11/19 11:49:55 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include	<string.h>

#include "libutl.h"
#include "gameheader.h"
//extern void PosBox(FVECTOR * ,float ,SVECTOR * );

typedef	struct _Work {
	GV_ACT_EX	actor ;
	char	str[16];
	int		time ;
	int		mode ;
	int		limit ;
	int		shift_y ;
	FVECTOR	pos;
} Work ;
static void convpos(Work *work)
{
	float     tmp ;
	FVECTOR   ret ;   
	int       x,y ;   /* 表示座標 */
	DG_CHANL  *cp ;
	u_char	r,g,b,alpha	;
	r = 127;g=127;b=127;
	
	/* 位置決定 */
	cp = DG_Chanl(0) ;
	tmp = work->pos.vw ;
	work->pos.vw = 1.0F ;
	_sceVu0ApplyMatrix(&ret, &cp->eye_pers, &work->pos) ;
	work->pos.vw = tmp ;
	
	 /* カメラの範囲内に入っているかどうか */
	if(ret.vz > ret.vw){
		return ;
	}
	if(ret.vw < 0) ret.vw = - ret.vw ;

	if((ret.vx > ret.vw) || (ret.vx < (-ret.vw))){
		return ;
	}

	if((ret.vy > ret.vw) || (ret.vy < (-ret.vw))){
		return ;
	}

	x = (int)((ret.vx / ret.vw) * (float)(DRAW_WIDTH>>1)) + (DRAW_WIDTH>>1) ;
	y = (int)((ret.vy / ret.vw) * (float)(DRAW_HEIGHT>>1)) + (DRAW_HEIGHT>>1) ;

#if 0
	//文字 上方へ
	y-=(int)(work->time*1) ;
#else
	y -= DIRECT_SCREEN_Y((int)(work->time/2)) ;
#endif

#if 1
//	y -= 16 ;
	y -= DIRECT_SCREEN_Y(work->shift_y) ;
#endif

//	MENU_SetAlphaMode( 0, 1, 0, 1, 0 );
	MENU_SetAlphaMode( 0, 2, 0, 1 ,0 ); //加算
	MENU_Locate( x,y, DG_DMAPACK_MENU );
	alpha = 90 ;
	if((work->limit - work->time)<30){
		alpha = 90- (30-(work->limit - work->time))*3 ;
	}
//	alpha = 0x80 ;

	switch(work->mode){
		case 0 :	/*白 得点*/
			r = 200;g=200;b=200;
			break ;
		case 1 :	/*赤 ＮＧ*/
			r = 200;g=64;b=64;
			break;
		case 2 :	/*黄色 コンボ*/
			r = 200;g=200;b=64;
			break;
		case 3 :	/* コンボリミット*/
			r = 255;g=152;b=26;
			break;
	}
	MENU_Color( r, g, b, alpha );
	MENU_Printf( "%s\n",work->str ) ;
}


static void Act(Work *work)
{
	if(work->time >=work->limit) {
		return ;
	}
	convpos(work ); 
	work->time++;
}
static void Die(Work *work)
{
}

/*
timeは内部でのPAL変換は行わない。
コンボカウントとのからみの問題
*/
void CALL_SIG_3DPrintf(void *w,FVECTOR *pos ,char *str,int mode,int time)
{
	Work *work ;
	work = (Work *) w ;
	if(work == NULL) return ;
   strncpy( work->str, str, sizeof( work->str ) );
   work->str[ sizeof( work->str ) - 1 ] = 0;
// BP_WARNING - Making the strcpy safe
//	strcpy(work->str,str);
	work->limit = time ;
	work->time = 0 ;
	work->pos = *pos ;
	work->mode = mode ;
}
void *NewSIG_3DPrintf(int shift_y)
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		work->shift_y = shift_y ;
	}
	return work ;
}

