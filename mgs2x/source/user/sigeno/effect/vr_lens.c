//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vr_lens.c
	ＶＲステージレンズフレア
	2002/04/22 K.Sigeno
	$Id: vr_lens.c,v 1.1.1.3 2002/11/19 11:49:49 Yoshizawa1 Exp $
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

#define TEX_FLR_R1	(14071854)
#define TEX_FLR_R2	(15120430)

#define TEX_FLR_G1	(14071832)
#define TEX_FLR_G2	(15120408)

#define TEX_FLR_B1	(14071822)
#define TEX_FLR_B2	(15120398)

//#define ALPHA_MAX	(40)
//#define ALPHA_MAX	(80)
#define ALPHA_MAX	(30)

#if 1
#define	N_FLARES (12)
static int FlareTexCode[N_FLARES]={

	TEX_FLR_R1,
	TEX_FLR_R2,

	TEX_FLR_G1,
	TEX_FLR_G2,

	TEX_FLR_B1,
	TEX_FLR_B2,

	TEX_FLR_R1,
	TEX_FLR_R2,

	TEX_FLR_G1,
	TEX_FLR_G2,

	TEX_FLR_B1,
	TEX_FLR_B2
	
};
static float tex_size[N_FLARES]={
	1.0,
	0.25,
	0.8,
	0.2,
	0.9,
	0.5,
	1.0,
	0.25,
	0.8,
	0.4,
	0.9,
	0.5
};

#else
#define	N_FLARES (2)
static int FlareTexCode[N_FLARES]={

	TEX_FLR_R1,
	TEX_FLR_R2,
};
static float tex_size[N_FLARES]={
	1.0,
	0.25
} ;

#endif



typedef	struct _Work {
	GV_ACT_EX	actor ;
	int		time ;
	int		mode ;
	int		limit ;
	FVECTOR	pos;
	FVECTOR	shift;
	DG_PRIM2		*prim_sprt[N_FLARES] ;

	float size ;
	float len ;

} Work ;



static void SetPrimPos(Work *work)
{
	FVECTOR *pos ;
	FVECTOR shift,shift2,raise ;
	int fls ,alpha;

	shift.vx = 0.0f ;
	shift.vy = 0.0f ;
	shift.vz = -work->len ;
	shift.vw = 1.0f ;

	{
		DG_CHANL  *cp ;
		FVECTOR cm_pos ;
		SVECTOR rot ;
		cp = DG_Chanl(0) ;

		GV_SubVec3F( (FVECTOR *)&cp->eye.m[3], &work->pos, &shift ) ;

#if 1
		raise.vx = shift.vx/1.5f ;
		raise.vy = shift.vy/1.5f ;
		raise.vz = shift.vz/1.5f ;
#else
		/*目標から1500離す*/
		GV_LenVec3F(&shift,&raise,1.0f, 2000.0f) ;
#endif
		_sceVu0Normalize( &shift, &shift ) ;

		rot.vx = GV_VecDir2X( &shift) ;
		rot.vy = GV_VecDir2( &shift) ;
		rot.vz = 0 ;

		DG_RotatePos( &rot ) ;
		DG_SetPos2(&work->pos,&rot) ;

		shift.vx = cosf(( float )M_PI * 1.0f*work->time/work->limit) ;
		shift.vy = 0.0f ;
		shift.vz = sinf(( float )M_PI * 1.0f*work->time/work->limit) ;

		shift2.vx = cosf(( float )-M_PI * (-1.0f)*work->time/work->limit) ;
		shift2.vy = 0.0f ;
		shift2.vz = sinf(( float )-M_PI * (1.0f)*work->time/work->limit) ;

		alpha = ALPHA_MAX* shift.vz ;

		DG_RotVector(&shift,&shift,1) ;
		DG_RotVector(&shift2,&shift2,1) ;
	}

	

	for(fls= 0;fls<N_FLARES;fls++){
		pos = work->prim_sprt[fls]->pos[work->prim_sprt[fls]->buffer_clock] ;
		if(fls&1){
			pos->vx = work->pos.vx + raise.vx + shift.vx*fls*work->len;
			pos->vy = work->pos.vy + raise.vy + shift.vy*fls*work->len;
			pos->vz = work->pos.vz + raise.vz + shift.vz*fls*work->len;
		}else {
			pos->vx = work->pos.vx + raise.vx + shift2.vx*fls*(-work->len);
			pos->vy = work->pos.vy + raise.vy + shift2.vy*fls*(-work->len);
			pos->vz = work->pos.vz + raise.vz + shift2.vz*fls*(-work->len);
		}
		{
			DG_PRIM2_UVRGBWH *uvrgbwh ;
			short tex_w,tex_h ;
			uvrgbwh = (DG_PRIM2_UVRGBWH *)work->prim_sprt[fls]->uvrgb[work->prim_sprt[fls]->buffer_clock] ;
			uvrgbwh->a  = alpha ;
		}
	}
}

static void Act(Work *work)
{
	FVECTOR *pos ;
	FVECTOR shift,shift2,raise ;
	int fls ,alpha;

#ifdef PSX2
	{
		if(((work->time &3)==1)&&(((irnd()>>8)&3)==1)){
			int rgb ,d;
			d = 80.0f *sinf(( float )M_PI * 1.0f*work->time/work->limit) ;
			rgb = d ;
			rgb |= d << 8;
			rgb |= d << 16;
			DG_SetTmpLight2( &work->pos , 1000.0f,2000.0f ,rgb,(0x0100|0x0200)) ;
		}
	}
#endif

	SetPrimPos(work) ;
	work->time++ ;
	if(work->limit <= work->time){
		GV_DestroyActor(work) ;
	}
}
static void Die(Work *work)
{
	int fls ;
//	DG_FreeMenu2Prim( work->prim ) ;
	for(fls= 0;fls<N_FLARES;fls++){
		DG_DequeuePrim2( work->prim_sprt[fls] );
		DG_FreePrim2( work->prim_sprt[fls] );
	}
}
static int GetResources(Work *work,FVECTOR	*pos ,int mode,int time )
{
	DG_TEX		*name_tex ;
	DG_PRIM2		*prim ;
	int i,j,k,l,fls;


	work->limit = DIRECT_TICK(time) ;
	work->time = 0 ;
	work->pos = *pos ;
	work->pos.vw = 1.0f ;

	work->mode = mode ;
	work->shift.vx = 0.0f;
	work->shift.vy = 0.0f;
	work->shift.vz = 0.0f;

	for(fls= 0;fls<N_FLARES;fls++){
		name_tex = DG_GetTexture( FlareTexCode[fls] );
		prim = work->prim_sprt[fls] = 
//		GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_FOG|DG_PRIM2_ALPHA, 1, 1 );
//NO_FOG
		GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, 1 );
		DG_ConfigPrim2Tex( work->prim_sprt[fls], name_tex );
		DG_VisiblePrim2(work->prim_sprt[fls]) ;
		DG_SetPrim2Alpha( work->prim_sprt[fls], KASAN );
		for(i= 0 ;i<2;i++){
			DG_PRIM2_UVRGBWH *uvrgbwh ;
			short tex_w,tex_h ;
			for(k= 0 ; k<1;k++){
				pos = work->prim_sprt[fls]->pos[i] ;
				pos->vx = work->pos.vx ;
				pos->vy = work->pos.vy ;
				pos->vz = work->pos.vz ;
				uvrgbwh = (DG_PRIM2_UVRGBWH *)work->prim_sprt[fls]->uvrgb[i] ;
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
//					uvrgbwh->w = work->size/2 + fls * (work->size/4) ;/* 幅／２ */
//					uvrgbwh->h = work->size/2 + fls * (work->size/4) ;/* 高さ／２ */
					uvrgbwh->w = work->size/2 * tex_size[fls] ;
					uvrgbwh->h = work->size/2 * tex_size[fls] ;
					uvrgbwh->r  = 127;
					uvrgbwh->g  = 127;
					uvrgbwh->b  = 127;
					uvrgbwh->a  = 0;
					uvrgbwh++ ;
				}
			}
		}
	}

	SetPrimPos(work) ;

	return 1 ;
}
void NewVR_Lens(FVECTOR *pos ,int mode,int time ,float size,float len)
{
	Work *work ;

	work = (Work *)GV_CreateActor( GV_ACTOR_AFTER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		work->size = size ;
		work->len = len ;
		if(!GetResources(work,pos ,mode,time)){
			GV_DestroyActor(work) ;
		}
	}
}

//flag 0x0100:キャラクタに反映 0x0200:背景に反映
#if 0
void VR_SetTmpLight(FVECTOR *pos , float r_range ,float e_range , int r,int g,int b,int flag)
{
	int color ;

	color = r ;//r
	color |= g << 8 ;//g
	color |= b << 16 ;//b

	DG_SetTmpLight2( pos, r_range, e_range, color, flag ) ;

}
#endif
