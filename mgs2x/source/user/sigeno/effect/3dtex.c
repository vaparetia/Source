//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	3dtex.c
	ＶＲステージ空中文字
	2002/03/25 K.Sigeno
	$Id: 3dtex.c,v 1.1.1.3 2002/11/19 11:49:40 Yoshizawa1 Exp $
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

#define KASAN (SCE_GS_SET_ALPHA(0,2,0,1,0))

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

//extern void PosBox(FVECTOR * ,float ,SVECTOR * );


#define Y_DEF (9)
typedef	struct _Work {
	GV_ACT_EX	actor ;
	int		time ;
	int		mode ;
	int		limit ;
	FVECTOR	pos;
	FVECTOR	shift;
	DG_PRIM2		*prim_sprt ;
} Work ;

static void Act(Work *work)
{
	FVECTOR *pos ;
#if 0
	{
		SVECTOR	rgb ;
		rgb.vx = 120;
		rgb.vy = 120;
		rgb.vz = 0;
		PosBox(&work->pos,30.0f ,&rgb);
	}
#endif
//	work->pos = *work->posbuf ;
//	GV_MatToVec( work->posbuf, &work->pos ) ;

//	work->pos.vx += work->shift.vx ;
//	work->pos.vy += work->shift.vy ;

	pos = work->prim_sprt->pos[work->prim_sprt->buffer_clock] ;
	pos->vx = work->pos.vx ;
	pos->vy = work->pos.vy ;
	pos->vz = work->pos.vz ;

	{
		DG_PRIM2_UVRGBWH *uvrgbwh ;
		short tex_w,tex_h ;
		uvrgbwh = (DG_PRIM2_UVRGBWH *)work->prim_sprt->uvrgb[work->prim_sprt->buffer_clock] ;
//		uvrgbwh->w = 500 ;/* 幅／２ */
//		uvrgbwh->h = 500 ;/* 高さ／２ */
		uvrgbwh->r  = 127;
		uvrgbwh->g  = 127;
		uvrgbwh->b  = 127;
		uvrgbwh->a  = (255* (work->limit - work->time) )/ work->limit ;
	}


	work->time++ ;
	if(work->limit <= work->time){
		GV_DestroyActor(work) ;
	}
}
static void Die(Work *work)
{
//	DG_FreeMenu2Prim( work->prim ) ;
	DG_DequeuePrim2( work->prim_sprt );
	DG_FreePrim2( work->prim_sprt );
}
static int GetResources(Work *work,FVECTOR	*pos ,int mode,int time , int code)
{
	DG_TEX		*name_tex ;
	DG_PRIM2		*prim ;
	int i,j,k,l;


	work->limit = time ;
	work->time = 0 ;
	work->pos = *pos ;
	work->mode = mode ;
	work->shift.vx = 0.0f;
	work->shift.vy = 0.0f;
	work->shift.vz = 0.0f;


	name_tex = DG_GetTexture( code );
	prim = work->prim_sprt = 
	  GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_FOG|DG_PRIM2_ALPHA, 1, 1 );
	DG_ConfigPrim2Tex( work->prim_sprt, name_tex );
	DG_VisiblePrim2(work->prim_sprt) ;
	DG_SetPrim2Alpha( work->prim_sprt, KASAN );
	for(i= 0 ;i<2;i++){
		DG_PRIM2_UVRGBWH *uvrgbwh ;
		short tex_w,tex_h ;
		for(k= 0 ; k<1;k++){
			pos = work->prim_sprt->pos[i] ;
			pos->vx = work->pos.vx ;
			pos->vy = work->pos.vy ;
			pos->vz = work->pos.vz ;
			uvrgbwh = (DG_PRIM2_UVRGBWH *)work->prim_sprt->uvrgb[i] ;
			uvrgbwh += k*1 ;
			tex_w = (u_short) name_tex->u_scale ;
			tex_h = (u_short) name_tex->v_scale ;
			for (j= 0 ; j<1 ;j++){
				uvrgbwh->u0 = FTOI12( 0.0F * name_tex->u_scale + name_tex->u_offset ) ;/* 左上 */
				uvrgbwh->v0 = FTOI12( 0.0F * name_tex->v_scale + name_tex->v_offset ) ;/* 左上 */
				uvrgbwh->u1 = FTOI12( 1.0F * name_tex->u_scale + name_tex->u_offset ) ;/* 右下 */
				uvrgbwh->v1 = FTOI12( 1.0F * name_tex->v_scale + name_tex->v_offset ) ;/* 右下 */
				uvrgbwh->q0 = 4096 ;
				uvrgbwh->q1 = 4096 ;
				uvrgbwh->f0 = 0x0fff ;
				uvrgbwh->f1 = 0x0fff ;
			/* ＷＨ値は整数なので注意！ */
				uvrgbwh->w = 1000 ;/* 幅／２ */
				uvrgbwh->h = 1000 ;/* 高さ／２ */
				uvrgbwh->r  = 255;
				uvrgbwh->g  = 255;
				uvrgbwh->b  = 255;
				uvrgbwh->a  = 64;
				uvrgbwh++ ;
			}
		}
	}


	return 1 ;
}
void NewSIG_3DTex(FVECTOR *pos ,int mode,int time,int code)
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources(work,pos ,mode,time,code)){
			GV_DestroyActor(work) ;
		}
	}
}

