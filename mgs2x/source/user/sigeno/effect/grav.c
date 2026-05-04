//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	grav.c
	重力に引かれていろいろ飛んでくる
	2002/04/24 K.Sigeno
	$Id: grav.c,v 1.1.1.3 2002/11/19 11:49:46 Yoshizawa1 Exp $
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

#define TEX_FLR_0	(6318117)

#define ALPHA_MAX	(40)
#define	N_FLARES (3)

//#define	TEST_R	(127)
//#define	TEST_G	(127)
//#define	TEST_B	(127)

#define	TEST_R	(37)
#define	TEST_G	(110)
#define	TEST_B	(127)

#define	TEX_VR_NOISE_A0	(11632395)	/*vr2_noise_a0_alp.bmp*/
#define	TEX_VR_NOISE_A1	(12680971)	/*vr2_noise_a1_alp.bmp*/
#define	TEX_VR_NOISE_A2	(13729547)	/*vr2_noise_a2_alp.bmp*/
#define	TEX_VR_NOISE_B0	(11632397)	/*vr2_noise_b0_alp.bmp*/
#define	TEX_VR_NOISE_B1	(12680973)	/*vr2_noise_b1_alp.bmp*/
#define	TEX_VR_NOISE_B2	(13729549)	/*vr2_noise_b2_alp.bmp*/

static int BonboriTexCode[N_FLARES]={

	TEX_FLR_R1,

	TEX_FLR_R1,

	TEX_FLR_R1,

//	TEX_FLR_R1,

//	TEX_FLR_G1,

//	TEX_FLR_B1
	
};
extern void PosBox(FVECTOR * ,float ,SVECTOR * );


typedef	struct _Work {
	GV_ACT_EX	actor ;
	int			time ;
	int			mode ;
	int			limit ;
	FVECTOR		pos;
	FVECTOR		shift;
	DG_PRIM2	*prim_sprt[N_FLARES] ;
	FVECTOR		vel[N_FLARES] ;
	float size ;
	int len ;

	float	lens[N_FLARES] ;
	short	dir[N_FLARES] ;
	short	dir_v[N_FLARES] ;

} Work ;

//#define START_VEL	(50.0f)
#define START_VEL	(90.0f)
//#define START_VEL	(0.0f)
//#define	G_RATE	(0.0000100f)
//#define	G_RATE	(0.0000500f)
//#define	G_RATE	(0.0001000f)
//#define	G_RATE	(0.0000010f)
//#define	G_RATE	(0.0000005f)
//#define	G_RATE	(0.005f)
//#define	G_RATE	(0.05f)
#define	G_RATE	(0.010f)

//#define	DECAY_RATE	(0.99985f)
//#define	DECAY_RATE	(0.99f)
//#define	DECAY_RATE	(0.985f) /*120*/
//#define	DECAY_RATE	(0.900f)
//#define	DECAY_RATE	(0.990f)
//#define	DECAY_RATE	(0.9800f)
#define	DECAY_RATE	(0.9850f)

//#define FIX_G_VEL (5.000f)
//#define FIX_G_VEL (4.000f)
//#define FIX_G_VEL (2.000f)
#define FIX_G_VEL (3.000f)

#if 0
static void randgr(float *vel,float pos){
	if(pos> 0.0f){
		*vel -= (pos * pos)*G_RATE;
	}else {
		*vel += (pos * pos)*G_RATE;
	}
	*vel *= DECAY_RATE;

}
#else
static void randgr(float *vel,float pos){
	if(pos> 0.0f){
		*vel -= FIX_G_VEL;
	}else {
		*vel += FIX_G_VEL;
	}
	*vel *= DECAY_RATE;
}
#endif

/*現在の方向ベクトルを中心向けに回転させる*/
static void SetGvec(FVECTOR *vel, FVECTOR *pos) {
	SVECTOR	to_cent_rot ,vel_rot,sub_rot ;
	FVECTOR	to_cent ;
	to_cent.vx = -pos->vx ;
	to_cent.vy = -pos->vy ;
	to_cent.vz = -pos->vz ;
	to_cent.vw = 0 ;

#if 0
//	GV_SubVec3F( (FVECTOR *)&cp->eye.m[3], &work->pos, &shift ) ;

	GV_VecToRot(&to_cent ,&to_cent_rot) ;
	GV_VecToRot(vel ,&vel_rot) ;

//int		GV_DiffDirS( from, to )
	sub_rot.vx = (GV_DiffDirS( vel_rot.vx, to_cent_rot.vx )) ;
	sub_rot.vy = (GV_DiffDirS( vel_rot.vy, to_cent_rot.vy )) ;
	sub_rot.vz = 0 ;

	DG_SetPos2(&DG_ZeroVector ,&sub_rot) ;

	DG_RotVector( vel, vel, 1 ) ;
#else 
	_sceVu0Normalize( &to_cent, &to_cent ) ;
	vel->vx += (to_cent.vx*60.0) ;
	vel->vy += (to_cent.vy*60.0) ;
	vel->vz += (to_cent.vz*60.0) ;
#endif

}

static void Act(Work *work)
{
	FVECTOR *pos ;
	FVECTOR shift,shift2,raise ;
	SVECTOR rot ;
	int fls ,alpha;
	float len_rate ;

	{
		DG_CHANL  *cp ;
		FVECTOR tmp;
		SVECTOR rot ;
		cp = DG_Chanl(0) ;

		GV_SubVec3F( (FVECTOR *)&cp->eye.m[3], &work->pos, &shift ) ;
		raise.vx = shift.vx/2.0f ;
		raise.vy = shift.vy/2.0f ;
		raise.vz = shift.vz/2.0f ;

		tmp.vx = raise.vx + work->pos.vx ;
		tmp.vy = raise.vy + work->pos.vy ;
		tmp.vz = raise.vz + work->pos.vz ;

		_sceVu0Normalize( &shift, &shift ) ;

		GV_VecToRot(&shift ,&rot) ;

//printf("TO CAM ROT X [%d] Y[%d] \n",rot.vx,rot.vy) ;
//		DG_RotatePos( &rot ) ;
//		DG_SetPos2(&work->pos,&rot) ;


		DG_SetPos2(&tmp,&rot) ;
//		DG_SetPos2(&DG_ZeroVector,&DG_ZeroSVector) ;


#if 1

		shift.vx = cosf(( float )M_PI * 1.0f*work->time/work->limit) ;
		shift.vy = 0.0f ;
		shift.vz = sinf(( float )M_PI * 1.0f*work->time/work->limit) ;

		shift2.vx = cosf(( float )M_PI * (-1.0f)*work->time/work->limit) ;
		shift2.vy = 0.0f ;
		shift2.vz = sinf(( float )M_PI * (1.0f)*work->time/work->limit) ;

//		alpha = 127* shift.vz ;
//		alpha = 80* shift.vz ;
//		alpha = 32* shift.vz ;
		alpha = ALPHA_MAX* shift.vz ;

		DG_RotVector(&shift,&shift,1) ;
		DG_RotVector(&shift2,&shift2,1) ;
#endif
	}



	for(fls= 0;fls<N_FLARES;fls++){
		DG_GetPos(&work->prim_sprt[fls]->as_world ) ;
	}
	for(fls= 0;fls<N_FLARES;fls++){
		pos = work->prim_sprt[fls]->pos[work->prim_sprt[fls]->buffer_clock] ;
#if 0
		randgr(&(work->vel[fls].vx),pos->vx) ;
		randgr(&(work->vel[fls].vy),pos->vy) ;
		randgr(&(work->vel[fls].vz),pos->vz) ;

		pos->vx += work->vel[fls].vx ;
		pos->vy += work->vel[fls].vy ;
		pos->vz += work->vel[fls].vz ;
#else

//		SetGvec(&(work->vel[fls]),pos) ;

//		pos->vx += work->vel[fls].vx ;
//		pos->vy += work->vel[fls].vy ;
//		pos->vz += work->vel[fls].vz ;


		len_rate = (float)(work->limit-work->time)/(float)work->limit  ;
//		len_rate = len_rate*len_rate*len_rate ;
		len_rate = len_rate*len_rate ;
		pos->vx = work->lens[fls]*len_rate ;
		pos->vy = pos->vz = 0.0f;

		work->dir[fls] += work->dir_v[fls] ;
//		work->dir_v[fls]++ ;

		rot.vx = rot.vy = 0;
		rot.vz = work->dir[fls] ;
		DG_SetPos2( &DG_ZeroVector, &rot ) ;

		DG_RotVector( pos, pos, 1 ) ;

#endif
#if 0
		if(fls&1){
			pos->vx = work->pos.vx + raise.vx + shift.vx*fls*work->len;
			pos->vy = work->pos.vy + raise.vy + shift.vy*fls*work->len;
			pos->vz = work->pos.vz + raise.vz + shift.vz*fls*work->len;
		}else {
			pos->vx = work->pos.vx + raise.vx + shift2.vx*fls*(-work->len);
			pos->vy = work->pos.vy + raise.vy + shift2.vy*fls*(-work->len);
			pos->vz = work->pos.vz + raise.vz + shift2.vz*fls*(-work->len);
		}
#endif
		{
			DG_PRIM2_UVRGBWH *uvrgbwh ;
			short tex_w,tex_h ;
			uvrgbwh = (DG_PRIM2_UVRGBWH *)work->prim_sprt[fls]->uvrgb[work->prim_sprt[fls]->buffer_clock] ;
//			uvrgbwh->a  = alpha ;
		uvrgbwh->w = (work->size/2 + fls * (work->size/4)) * (work->limit - work->time) / work->limit ;
		uvrgbwh->h = (work->size/2 + fls * (work->size/4))*	(work->limit - work->time) / work->limit ; 

			uvrgbwh->a  = 80 *	(work->limit - work->time) / work->limit ;

		}
	}

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

static void SetRandVel(Work *work)
{
	FVECTOR	vel,len,*pos;
	SVECTOR rot ;
	int i;

	vel.vx= 0.0f ;
	vel.vy= START_VEL ;
	vel.vz= 0.0f ;

	len.vy= 0.0f ;
	len.vz= 0.0f ;

	for(i=0 ; i<N_FLARES; i++){
//		len.vx = (float)((irnd()>>8) %  work->len ) ;
		len.vx = work->len -  ((irnd()>>8) % (work->len/2) );

//		rot.vx = (irnd()>>8) & 4095 ;
		rot.vx = 0 ;
//		rot.vy = (irnd()>>8) & 4095 ;
		rot.vy = 0 ;
//		rot.vz = 0 ;
		rot.vz = (irnd()>>8) & 4095 ;

		DG_SetPos2( &DG_ZeroVector, &rot ) ;
		DG_RotVector( &vel, &work->vel[i], 1 ) ;
		pos = work->prim_sprt[i]->pos[0] ;
		DG_RotVector( &len, pos, 1 ) ;
		pos = work->prim_sprt[i]->pos[1] ;
		DG_RotVector( &len, pos, 1 ) ;
	}

	for(i=0 ; i<N_FLARES; i++){
//		len.vx = (float)((irnd()>>8) %  work->len ) ;
		work->lens[i] = work->len -  ((irnd()>>8) % (work->len/2) );
		work->dir[i] = (irnd()>>8)& 4095 ;
		if(i&1){
			work->dir_v[i] = (irnd()>>8) % 25 ;
		}else {
			work->dir_v[i] = -((irnd()>>8) % 25) ;
		}
		rot.vx = rot.vy = 0;
		rot.vz = work->dir[i] ;
		DG_SetPos2( &DG_ZeroVector, &rot ) ;

		pos = work->prim_sprt[i]->pos[0] ;

		pos->vx = work->lens[i] ;
		pos->vy = pos->vz = 0.0f ;

		DG_RotVector( pos, pos, 1 ) ;

		pos = work->prim_sprt[i]->pos[1] ;

		pos->vx = work->lens[i] ;
		pos->vy = pos->vz = 0.0f ;

		DG_RotVector( pos, pos, 1 ) ;

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
		name_tex = DG_GetTexture( BonboriTexCode[fls] );
		prim = work->prim_sprt[fls] = 
		   GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_FOG|DG_PRIM2_ALPHA, 1, 1 );

      prim->flag |= DG_PRIM_AS_CUSTOMWORLD;

		DG_ConfigPrim2Tex( work->prim_sprt[fls], name_tex );
		DG_VisiblePrim2(work->prim_sprt[fls]) ;
		DG_SetPrim2Alpha( work->prim_sprt[fls], KASAN );
		for(i= 0 ;i<2;i++){
			DG_PRIM2_UVRGBWH *uvrgbwh ;
			short tex_w,tex_h ;
			for(k= 0 ; k<1;k++){
				pos = work->prim_sprt[fls]->pos[i] ;
#if 0
				pos->vx = work->pos.vx ;
				pos->vy = work->pos.vy ;
				pos->vz = work->pos.vz ;
#else
				*pos = DG_ZeroVector ;
#endif
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
					uvrgbwh->w = work->size/2 + fls * (work->size/4) ;/* 幅／２ */
					uvrgbwh->h = work->size/2 + fls * (work->size/4) ;/* 高さ／２ */
					uvrgbwh->r  = TEST_R;
					uvrgbwh->g  = TEST_G;
					uvrgbwh->b  = TEST_B;
					uvrgbwh->a  = 0;
					uvrgbwh++ ;
				}
			}
		}
	}

	SetRandVel(work) ;

	return 1 ;
}
void NewGravEf(FVECTOR *pos ,int mode,int time ,float size,int len)
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
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

