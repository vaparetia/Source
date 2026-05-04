//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	bomb_gas.c
	爆破エフェクト
	2000/03/03 S.Okajima
	$Id: bomb_gas.c,v 1.1.1.3 2002/11/19 11:47:01 Yoshizawa1 Exp $

*/

#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

#define	RGB_SMOKE		(64)
#define	ALPHA_SMOKE		(32)

#define	RGB_FIRE		(16)
#define	ALPHA_FIRE		(32)

#define N_PRIMS		(1)
#define N_VERTS		(4)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)


#define	LIFE_TIME		(150)
#define	VEC_MIN			(50.0f)
#define	VEC_RAND		(150.0f)
#define	MIN_VEC			(50.0f)
#define	DECAY_RATIO		(0.95f)
#define	SIZE_SCALING	(10.0f)

#define	INIT_MULTIPLE_NUM	(2)
#define	WIN_MIN				(10.0f)
#define	WIN_MAX				(200.0f)
#define	ZOOM_RATIO			(1.002f)
#define	SEARCH_HZX			(2000)

#define	RAISE_SMOKE			(1000)
#define	RAISE_FIRE			(1500)

#define	SHIFT_CENTER		(1000.0f)

extern	FVECTOR G_wind;
extern SVECTOR G_wind_rot;	/* 風向 */
extern int G_wind_intense;	/* 強さ */
extern int G_wind_intense_max;
extern FMATRIX G_wind_matrix;

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;

	DG_PRIM2	*prim ;
	DG_PRIM2	*prim_fire ;

	FVECTOR		center;

	float		angle[ N_PRIMS * N_VERTS ] ;
	int			parts_flg[ N_PRIMS * N_VERTS ];

	FVECTOR		top_vec[ N_PRIMS ];
	float		top_radius[ N_PRIMS ];
	FVECTOR		top_pos[ N_PRIMS ];

	SVECTOR		wind_rot;
	FVECTOR		wind;

	int			life;

//	FVECTOR		temp[4096];

} Work ;
/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	FVECTOR	*pos;
	FVECTOR	*pos_before;
	FVECTOR	*pos_fire;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	DG_PRIM2_UVRGBWH	*uvrgbwh_before;
	DG_PRIM2_UVRGBWH	*uvrgbwh_fire;
	int	i,j;
	int	local_count;
	int		clock;
	int		alpha_smoke;
	int		alpha_fire;
	int		*parts_flg;
	float	*top_radius;
	float	*angle;
	float	ftemp;
	float	angle_temp;
	FVECTOR	*top_vec;
	FVECTOR	*top_pos;
	FVECTOR	local_wind;


	DG_VisiblePrim2( work->prim ) ;
	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
	DG_VisiblePrim2( work->prim_fire ) ;
	GM_GroupPrim2( work->prim_fire, GM_CurrentStageMap ) ;

	alpha_smoke = ALPHA_SMOKE * work->life     / LIFE_TIME;
//	alpha_fire  = ALPHA_FIRE  * work->life * 3 / LIFE_TIME - ALPHA_FIRE*2;
	alpha_fire  = ALPHA_FIRE  * work->life * 2 / LIFE_TIME - ALPHA_FIRE;
	if( alpha_smoke < 0 ) alpha_smoke = 0;
	if( alpha_fire  < 0 ) alpha_fire  = 0;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	DG_SwitchBuffPrim2( work->prim_fire );
	clock = work->prim->buffer_clock;

	work->wind.vx=0.0f;
	work->wind.vz=0.0f;

	ftemp = -(float)G_wind_intense * WIN_MAX / (float)G_wind_intense_max;
	OK_FloatSmoother( &work->wind.vy, &ftemp, 0.97f );

	OK_DirectionSmoother( &work->wind_rot, &G_wind_rot, 0.999f );
	DG_SetPos2( &DG_ZeroVector, &work->wind_rot );
	DG_RotVector( &work->wind, &local_wind, 1 );
	local_wind.vy = 0.0f;



	top_vec    = work->top_vec;
	top_radius = work->top_radius;
	top_pos    = work->top_pos;

	pos            = work->prim->pos[clock];
	pos_before     = work->prim->pos[1-clock];
	uvrgbwh        = work->prim->uvrgb[clock];
	uvrgbwh_before = work->prim->uvrgb[1-clock];
	pos_fire       = work->prim_fire->pos[clock];
	uvrgbwh_fire   = work->prim_fire->uvrgb[clock];

	parts_flg  = work->parts_flg;
	angle      = work->angle;
	for( i=0; i<N_PRIMS; i++ ){
		local_count=irnd()%INIT_MULTIPLE_NUM+1;
		for( j=0; j<N_VERTS; j++ ){
			uvrgbwh->a      = (alpha_smoke - j < 0)? 0: alpha_smoke - j;
			uvrgbwh_fire->a = (alpha_fire  - j < 0)? 0: alpha_fire  - j;
			if( (*parts_flg) == 0 && local_count >= 0 ){
				(*parts_flg) = 1;
				local_count--;
				_sceVu0AddVector( top_pos, top_pos, top_vec );
				DG_COPY_VEC( pos,      top_pos );
				top_vec->vy += P_GRAVITY;

				ftemp = (*top_radius) * (1.0f + rnd());
				angle_temp = (*angle)*TPI;
				uvrgbwh->w = (int)(ftemp * cosf( angle_temp ));
				uvrgbwh->h = (int)(ftemp * sinf( angle_temp ));
				(*top_radius) *= DECAY_RATIO;
			}else if( (*parts_flg) == 1 ){
				pos->vx = pos_before->vx + (*angle)*local_wind.vx;
				pos->vy = pos_before->vy;
				pos->vz = pos_before->vz + (*angle)*local_wind.vz;
				uvrgbwh->w = uvrgbwh_before->w * ZOOM_RATIO;
				uvrgbwh->h = uvrgbwh_before->h * ZOOM_RATIO;
			}
			if( uvrgbwh_fire->a == 0 ){
				uvrgbwh_fire->w = 0;
				uvrgbwh_fire->h = 0;
			}else{
				DG_COPY_VEC( pos_fire, pos );
				uvrgbwh_fire->w = uvrgbwh->w;
				uvrgbwh_fire->h = uvrgbwh->h;
			}
			angle  ++;
			pos    ++;
			pos_before++;
			uvrgbwh++;
			uvrgbwh_before++;
			pos_fire    ++;
			uvrgbwh_fire++;
			parts_flg++;
		}
		top_vec++;
		top_radius++;
		top_pos++;
	}

	if( work->life-- < -1 ) GV_DestroyActor( work ) ;
}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
	work->prim_fire = OK_FreePrim2( work->prim_fire );
}

static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *center, int mode )
{
	FVECTOR		*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i, k ;

	pos        = SCR_POS;
	uvrgbwh    = SCR_UVS;
	DG_ConfigPrim2Tex( prim, tex );
	if( mode==0 ){
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
		prim->raise = RAISE_SMOKE;
		for ( i = 0 ; i < N_PRIMS ; i++ ){
			for ( k = 0 ; k < N_VERTS ; k++ ){
				DG_COPY_VEC( pos, center );
				uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
				uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
				uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
				uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
				uvrgbwh->q0 = 4096 ;
				uvrgbwh->q1 = 4096 ;
				uvrgbwh->f0 = 0x0fff ;
				uvrgbwh->f1 = 0x0fff ;
				uvrgbwh->h  = uvrgbwh->w  = 0;
				uvrgbwh->r  = uvrgbwh->g = uvrgbwh->b = RGB_SMOKE ;
				uvrgbwh->a  = 0 ;
				pos ++;
				uvrgbwh ++ ;
			}
		}
	}else{
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
		prim->raise = RAISE_FIRE;
		for ( i = 0 ; i < N_PRIMS ; i++ ){
			for ( k = 0 ; k < N_VERTS ; k++ ){
				DG_COPY_VEC( pos, center );
				uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
				uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
				uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
				uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
				uvrgbwh->q0 = 4096 ;
				uvrgbwh->q1 = 4096 ;
				uvrgbwh->f0 = 0x0fff ;
				uvrgbwh->f1 = 0x0fff ;
				uvrgbwh->h  = uvrgbwh->w  = 0;
				uvrgbwh->r  = uvrgbwh->g = uvrgbwh->b = RGB_FIRE ;
				uvrgbwh->a  = 0 ;
				pos ++;
				uvrgbwh ++ ;
			}
		}
	}



	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );

	return 1;
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work, FVECTOR *pos, SVECTOR *pole_rot )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	FVECTOR		fvtemp;
	int			i,k;
	SVECTOR		svtemp;
	int		*parts_flg;
	float	*angle;
	float	*top_radius;
	FVECTOR	*top_pos;
	FVECTOR	*top_vec;


	work->prim      = NULL;
	work->prim_fire = NULL;

	DG_COPY_VEC( &work->center, pos );

	//------------------------------------------------------------
	/* 煙 */
	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
	prim = work->prim = GM_MakePrim2(
				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, /* 回転スプライト */
				N_PRIMS,
				N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex, pos, 0 );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);


	/* 炎 */
	tex = DG_GetTexture( 11171645 /*"fire1_alp"*/ );
	prim = work->prim_fire = GM_MakePrim2(
				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, /* 回転スプライト */
				N_PRIMS,
				N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex, pos, 1 );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);




	angle      = work->angle;
	parts_flg  = work->parts_flg;
	top_radius = work->top_radius;
	top_pos    = work->top_pos;
	top_vec    = work->top_vec;
	fvtemp.vx  = 0.0f;
	fvtemp.vz  = 0.0f;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			(*angle)    = rnd();
			(*parts_flg)= 0;

			parts_flg++;
			angle++;
		}
		DG_COPY_VEC( top_pos, pos );
		(*top_radius) = fvtemp.vy = rnd()*VEC_RAND + VEC_MIN;
		(*top_radius)*= SIZE_SCALING;
 		svtemp.vx = irnd()%(512 + 256);
		svtemp.vy = irnd()%4096;
		svtemp.vz = 0;
		DG_SetPos2( &DG_ZeroVector, &svtemp );
		DG_RotVector( &fvtemp, top_vec, 1 );
		top_vec++;
		top_radius++;
		top_pos++;
	}

	DG_SetPos2( &work->center, pole_rot );
	DG_RotVector( work->top_vec, work->top_vec, N_PRIMS );
	fvtemp.vy = SHIFT_CENTER;
	DG_PutVector( work->top_vec, &work->center, 1 );

	work->life = LIFE_TIME;

#if 0
	work->wind_rot.vx = G_wind_rot.vx;
	work->wind_rot.vy = G_wind_rot.vy;
	work->wind_rot.vz = G_wind_rot.vz;
#else
	work->wind_rot.vx = pole_rot->vx;
	work->wind_rot.vy = pole_rot->vy;
	work->wind_rot.vz = pole_rot->vz;
#endif

	work->wind.vx = 0.0f;
	work->wind.vy = 0.0f;
	work->wind.vz = 0.0f;

	return 0 ;
}

void *NewBombGasEffect( FVECTOR *pos, SVECTOR *pole_rot )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, pos, pole_rot ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
