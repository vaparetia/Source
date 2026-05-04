//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	d_splash_camera.c
	カメラ前水飛沫
	2000/04/09 S.Okajima
	$Id: d_splash_camera.c,v 1.1.1.3 2002/11/19 11:46:54 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include	"gameheader.h"
#include	"libmt.h"
#include	"libutl.h"
#include	"utl_dma.h"
#include	"../etc/ok_util.h"

/* 画面前バンダリ */
#define	BOUND_WIDTH	(2000.0f)
/* バウンドの中心と、視点との距離 */
#define	CENTER_DISTANCE	(1800.0f)
//#define	CENTER_DISTANCE	(3000.0f)

#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_VEC			(SCRPAD_ADDR + 0x1000)
#define	SCR_UVS			(SCRPAD_ADDR + 0x2000)

#define	SCR_POS_W0		(SCRPAD_ADDR)
#define	SCR_POS_W1		(SCRPAD_ADDR + 0x10)
#define	SCR_POS_W2		(SCRPAD_ADDR + 0x20)
#define	SCR_POS_W3		(SCRPAD_ADDR + 0x30)
#define	SCR_POS_W4		(SCRPAD_ADDR + 0x30)

#define	scw_fvcalc0	((FVECTOR*)SCR_POS_W0)
#define	scw_fvtemp	((FVECTOR*)SCR_POS_W1)

#define	SIZE_MIN	(10.0f)

#define	WIN_MIN			(200.0f)


#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/4)

#define	BASE_RGB		(128)
//#define	BASE_RGB		(160)
#define	MAX_ALPHA		(96)

#define	COUNT_MIN		(8)
#define	COUNT_RND		(4)

//#define	COUNT_MIN		(32)
//#define	COUNT_RND		(16)

#define	HEIGHT			(50.0f)
#define	SIZE			(300.0f)
//#define	SIZE			(200.0f)

int		OK_CameraSplash_flag=0;
extern	SVECTOR	G_wind_rot;
extern	int		OK_rain_on_off_flag;
extern	float	OK_rain_percentage;

/*----------------------------------------------------------------*/
extern void *NewSplashParts_Demo( FVECTOR *center, SVECTOR *rot, float intense );

/*----------------------------------------------------------------*/
typedef	struct	{
	int			count;
	int			count_max;
	FVECTOR		vec[N_VERTS];
} Unit ;

typedef	struct	{
	GV_ACT_EX	actor ;

	DG_PRIM2	*prim ;

	FVECTOR		cm_bound0;
	FVECTOR		cm_bound1;
	FVECTOR		bound0;
	FVECTOR		bound1;
	FVECTOR		diff;
	SVECTOR		vec_rot;
	SVECTOR		rot;

	int			*flag;

	int			repeat_num;
	Unit		unit[0];
} Work ;

/*----------------------------------------------------------------*/
/* 座標計算データを初期化する */
static	void	InitVectors( Work *work, int prim_num, int active_num, int clock )
{
	int		i;
	int		num;
	float	len;
	SVECTOR	rot;
	FVECTOR	center;
	FVECTOR	*pos,*calc;
	DG_PRIM2_UVRGB	*uvrgb ;
	FVECTOR	*vec;

	work->unit[prim_num].count_max = work->unit[prim_num].count = COUNT_MIN + irnd()%COUNT_RND;

	num   = prim_num*N_VERTS;
	pos   = work->prim->pos[ clock ];
	pos  += num;
	uvrgb = work->prim->uvrgb[ clock ];
	uvrgb+= num;
	vec   = work->unit[prim_num].vec;
#ifdef BP_PSX2_ASM
	asm volatile ("
		lqc2		vf13,0x00(%0)
		"::"r"(&work->bound0):"memory"
	);
#endif

	if( prim_num >= active_num ){
		DG_COPY_VEC( &center, (FVECTOR *)DG_Chanls->eye.m[3] );
		for( i=0; i<N_POLYS; i++ ){
			DG_COPY_VEC( pos, &center );	pos++;
			DG_COPY_VEC( pos, &center );	pos++;
			DG_COPY_VEC( pos, &center );	pos++;
			DG_COPY_VEC( pos, &center );	pos++;
			uvrgb->a = 0;					uvrgb++;
			uvrgb->a = 0;					uvrgb++;
			uvrgb->a = 0;					uvrgb++;
			uvrgb->a = 0;					uvrgb++;
		}
	}else{
		for( i=0; i<N_POLYS; i++ ){
			uvrgb->a = MAX_ALPHA;			uvrgb++;
			uvrgb->a = MAX_ALPHA;			uvrgb++;
			uvrgb->a = MAX_ALPHA;			uvrgb++;
			uvrgb->a = MAX_ALPHA;			uvrgb++;
#if 0
			center.vx = work->bound0.vx + work->diff.vx * rnd();
			center.vy = work->bound0.vy + work->diff.vy * rnd();
			center.vz = work->bound0.vz + work->diff.vz * rnd();

			DG_COPY_VEC( pos, &center );	pos++;
			DG_COPY_VEC( pos, &center );	pos++;
			DG_COPY_VEC( pos, &center );	pos++;
			DG_COPY_VEC( pos, &center );	pos++;
#else
#ifdef BP_PSX2_ASM
			center.vx = work->diff.vx * rnd();
			center.vy = work->diff.vy * rnd();
			center.vz = work->diff.vz * rnd();
			asm volatile ("
				lqc2		vf8 ,0x00(%1)
				vadd.xyz	vf8 ,vf8 ,vf13
				vmove.xyz	vf9 ,vf8
				vmove.xyz	vf10,vf8
				vmove.xyz	vf11,vf9

				sqc2		vf8 ,0x00(%0)
				sqc2		vf9 ,0x10(%0)
				sqc2		vf10,0x20(%0)
				sqc2		vf11,0x30(%0)
				"::"r"(pos),"r"(&center):"memory"
			);
			pos += 4;
#else
			center.vx = work->bound0.vx + work->diff.vx * rnd();
			center.vy = work->bound0.vy + work->diff.vy * rnd();
			center.vz = work->bound0.vz + work->diff.vz * rnd();

			DG_COPY_VEC( pos, &center );	pos++;
			DG_COPY_VEC( pos, &center );	pos++;
			DG_COPY_VEC( pos, &center );	pos++;
			DG_COPY_VEC( pos, &center );	pos++;
#endif
#endif

#ifndef BP_PSX2_ASM
			len   = -SIZE * (rnd() + 1.0f);
			{
				float	width, z0, z1, z2 ;
				width = len ;
				z0 = len * 0.10f ;
				z1 = len * 0.55f ;
				z2 = len ;
				scw_fvcalc0[0].vx = 0.0f ;
				scw_fvcalc0[0].vy = 0.0f ;
				scw_fvcalc0[0].vz = z2 ;
				scw_fvcalc0[1].vx = width ;
				scw_fvcalc0[1].vy = 0.0f ;
				scw_fvcalc0[1].vz = z1 ;
				scw_fvcalc0[2].vx = -width ;
				scw_fvcalc0[2].vy = 0.0f ;
				scw_fvcalc0[2].vz = z1 ;
				scw_fvcalc0[3].vx = 0.0f ;
				scw_fvcalc0[3].vy = 0.0f ;
				scw_fvcalc0[3].vz = z0 ;
			}
#else
			len   = -SIZE * (rnd() + 1.0f);
			scw_fvtemp->vx = len;
			scw_fvtemp->vy = len * 0.55f;
			scw_fvtemp->vz = len ;
			scw_fvtemp->vw = len * 0.10f;
			asm volatile ("
				lqc2		vf12,0x00(%1)

				vmove.xy	vf8 ,vf0
				vaddx.z		vf8 ,vf0 ,vf12

				vaddz.x		vf9 ,vf0 ,vf12
				vmove.y		vf9 ,vf0
				vaddy.z		vf9 ,vf0 ,vf12

				vsubz.x		vf10,vf0,vf12
				vmove.y		vf10,vf0
				vaddy.z		vf10,vf0,vf12

				vmove.xy	vf11,vf0
				vaddw.z		vf11,vf0,vf12

				sqc2		vf8 ,0x00(%0)
				sqc2		vf9 ,0x10(%0)
				sqc2		vf10,0x20(%0)
				sqc2		vf11,0x30(%0)
				"::"r"(scw_fvcalc0),"r"(scw_fvtemp):"memory"
			);
#endif

			rot.vx = work->vec_rot.vx + irnd()%128  + 64;
			rot.vy = work->vec_rot.vy + irnd()%1024 - 512;
			rot.vz = irnd()%4096;
			DG_SetPos2( &DG_ZeroVector, &rot );
			DG_RotVector( scw_fvcalc0, vec, 4 );

/*
			_sceVu0ScaleVector( &vec[0], &vec[0], 0.25f );
			_sceVu0ScaleVector( &vec[1], &vec[1], 0.25f );
			_sceVu0ScaleVector( &vec[2], &vec[2], 0.25f );
			_sceVu0ScaleVector( &vec[3], &vec[3], 0.25f );
*/

			vec+=4;
		}
	}
}

/*----------------------------------------------------------------*/
/* ポリゴンの座標データを更新する */
static	void	UpdateVectors( Work *work, int prim_num, int active_num, int clock )
{
	int	j;
	int	num;
	FVECTOR			*d_pos;
	DG_PRIM2_UVRGB	*d_uvrgb ;
	FVECTOR			*pos;
	DG_PRIM2_UVRGB	*uvrgb ;
	FVECTOR			*vec;
	u_char	col;


	work->unit[prim_num].count--;

	num     = prim_num*N_VERTS;
	if( prim_num >= active_num ){
		d_uvrgb = work->prim->uvrgb[clock];
		d_uvrgb+= num;
		uvrgb = SCR_UVS;
		OK_Mem_Scr( SCR_UVS, d_uvrgb, sizeof(DG_PRIM2_UVRGB), N_VERTS);
		for( j=0; j<N_POLYS; j++ ){
			/* α */
			uvrgb->a = 0;	uvrgb++;
			uvrgb->a = 0;	uvrgb++;
			uvrgb->a = 0;	uvrgb++;
			uvrgb->a = 0;	uvrgb++;
		}
		OK_Scr_Mem( d_uvrgb, SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_VERTS);
	}else{
		col=(u_char)( work->unit[prim_num].count * MAX_ALPHA / work->unit[prim_num].count_max );
		d_pos   = work->prim->pos[1-clock];
		d_pos  += num;
		d_uvrgb = work->prim->uvrgb[clock];
		d_uvrgb+= num;
		vec     = work->unit[prim_num].vec;
		OK_Mem_Scr( SCR_POS, d_pos,   sizeof(FVECTOR),        N_VERTS);
		OK_Mem_Scr( SCR_UVS, d_uvrgb, sizeof(DG_PRIM2_UVRGB), N_VERTS);
		OK_Mem_Scr( SCR_VEC, vec,     sizeof(FVECTOR),        N_VERTS);
		vec   = SCR_VEC;
		pos   = SCR_POS;
		uvrgb = SCR_UVS;
		for( j=0; j<N_POLYS; j++ ){
			/* α */
			uvrgb->a = col;	uvrgb++;
			uvrgb->a = col;	uvrgb++;
			uvrgb->a = col;	uvrgb++;
			uvrgb->a = col;	uvrgb++;
#if 0
			/* 座標更新 */
			pos->vx += vec->vx;
			pos->vy += vec->vy;
			pos->vz += vec->vz;
			pos++;
			vec++;
			pos->vx += vec->vx;
			pos->vy += vec->vy;
			pos->vz += vec->vz;
			pos++;
			vec++;
			pos->vx += vec->vx;
			pos->vy += vec->vy;
			pos->vz += vec->vz;
			pos++;
			vec++;
			pos->vx += vec->vx;
			pos->vy += vec->vy;
			pos->vz += vec->vz;
			pos++;
			vec++;
#else
#ifdef BP_PSX2_ASM
			asm volatile ("
				lqc2		vf8 ,0x00(%0)
				lqc2		vf9 ,0x00(%1)
				lqc2		vf10,0x10(%0)
				lqc2		vf11,0x10(%1)
				lqc2		vf12,0x20(%0)
				lqc2		vf13,0x20(%1)
				lqc2		vf14,0x30(%0)
				lqc2		vf15,0x30(%1)

				vadd.xyz	vf8 ,vf8 ,vf9
				vadd.xyz	vf10,vf10,vf11
				vadd.xyz	vf12,vf12,vf13
				vadd.xyz	vf14,vf14,vf15

				sqc2		vf8 ,0x00(%0)
				sqc2		vf10,0x10(%0)
				sqc2		vf12,0x20(%0)
				sqc2		vf14,0x30(%0)
				"::"r"(pos),"r"(vec):"memory"
			);
			pos+=4;
			vec+=4;
#else
			pos->vx += vec->vx;
			pos->vy += vec->vy;
			pos->vz += vec->vz;
			pos++;
			vec++;
			pos->vx += vec->vx;
			pos->vy += vec->vy;
			pos->vz += vec->vz;
			pos++;
			vec++;
			pos->vx += vec->vx;
			pos->vy += vec->vy;
			pos->vz += vec->vz;
			pos++;
			vec++;
			pos->vx += vec->vx;
			pos->vy += vec->vy;
			pos->vz += vec->vz;
			pos++;
			vec++;
#endif
#endif
		}
		d_pos   = work->prim->pos[clock];
		d_pos  += num;
		OK_Scr_Mem( d_pos,   SCR_POS, sizeof(FVECTOR),        N_VERTS);
		OK_Scr_Mem( d_uvrgb, SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_VERTS);
	}
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	FVECTOR	fvtemp;
	FVECTOR	b_dis;
	int		i;
	int		clock;
	Unit	*unit;
	int		active_num;

	if( work->flag!=NULL ){
		if( !(*work->flag) ){
			DG_InvisiblePrim2( work->prim ) ;
			return;
		}
	}

	active_num = (int)((float)work->repeat_num * OK_rain_percentage);

	if( (OK_rain_on_off_flag==1)
	 || (OK_CameraSplash_flag==1)
	 || (GM_CheckPlayerStatus( PLAYER_CB_BOX ) && GM_CheckPlayerStatus( PLAYER_WATCH ) )
	  ){
		DG_InvisiblePrim2( work->prim ) ;
		return;
	}else{
		DG_VisiblePrim2( work->prim ) ;
	}

	/* カメラのまえのバンダリをつくる */
	b_dis.vx=0.0f;
	b_dis.vy=0.0f;
	b_dis.vz=CENTER_DISTANCE;
	DG_SetPos( &DG_Chanls->eye );
	DG_PutVector( &b_dis, &b_dis, 1 );
	_sceVu0AddVector( &work->bound0, &b_dis, &work->cm_bound0 ) ;
	_sceVu0AddVector( &work->bound1, &b_dis, &work->cm_bound1 ) ;


	fvtemp.vx = 0.0f;
	fvtemp.vz = 0.0f;
	fvtemp.vy = 4096.0f;
	OK_DirectionSmoother( &work->rot, &G_wind_rot, 0.993f );
//printf("%d %d %d\n",work->rot.vx,work->rot.vy,work->rot.vz);
	DG_SetPos2( &DG_ZeroVector, &work->rot );
	DG_PutVector( &fvtemp, &fvtemp, 1 );
	fvtemp.vy = -HEIGHT;
	OK_DirVecXY( &DG_ZeroVector, &fvtemp, &work->vec_rot );

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;
	unit = work->unit;
	for ( i = 0 ; i < work->repeat_num ; i++ ){
		if( unit->count==0 ){
			InitVectors( work, i, active_num, clock );
		}else{
			UpdateVectors( work, i, active_num, clock );
		}
		unit++;
	}

}


static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}


/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB	*uvrgb0 ;
	DG_PRIM2_UVRGB	*uvrgb1 ;
	int		i, k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	uvrgb0 = prim->uvrgb[ 0 ] ;
	uvrgb1 = prim->uvrgb[ 1 ] ;
	for ( i = 0 ; i < work->repeat_num ; i++ ){
		for ( k = 0 ; k < N_POLYS ; k++ ){
			uvrgb1->u = uvrgb0->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb1->v = uvrgb0->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x8fff ;
			uvrgb1->r = uvrgb0->r = BASE_RGB ;
			uvrgb1->g = uvrgb0->g = BASE_RGB ;
			uvrgb1->b = uvrgb0->b = BASE_RGB ;
			uvrgb1->a = uvrgb0->a = MAX_ALPHA ;
			uvrgb1++; uvrgb0++;

			uvrgb1->u = uvrgb0->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb1->v = uvrgb0->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x8fff ;
			uvrgb1->r = uvrgb0->r = BASE_RGB ;
			uvrgb1->g = uvrgb0->g = BASE_RGB ;
			uvrgb1->b = uvrgb0->b = BASE_RGB ;
			uvrgb1->a = uvrgb0->a = MAX_ALPHA ;
			uvrgb1++; uvrgb0++;

			uvrgb1->u = uvrgb0->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb1->v = uvrgb0->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x0fff ;
			uvrgb1->r = uvrgb0->r = BASE_RGB ;
			uvrgb1->g = uvrgb0->g = BASE_RGB ;
			uvrgb1->b = uvrgb0->b = BASE_RGB ;
			uvrgb1->a = uvrgb0->a = MAX_ALPHA ;
			uvrgb1++; uvrgb0++;

			uvrgb1->u = uvrgb0->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb1->v = uvrgb0->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x0fff ;
			uvrgb1->r = uvrgb0->r = BASE_RGB ;
			uvrgb1->g = uvrgb0->g = BASE_RGB ;
			uvrgb1->b = uvrgb0->b = BASE_RGB ;
			uvrgb1->a = uvrgb0->a = MAX_ALPHA ;
			uvrgb1++; uvrgb0++;
		}
	}


	return 1;
}

static int GetResources( Work *work )
{
	int		i;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->cm_bound0.vx = -BOUND_WIDTH;
	work->cm_bound0.vy = -BOUND_WIDTH;
	work->cm_bound0.vz = -BOUND_WIDTH;
	work->cm_bound1.vx =  BOUND_WIDTH;
	work->cm_bound1.vy =  BOUND_WIDTH;
	work->cm_bound1.vz =  BOUND_WIDTH;
	work->diff.vx = BOUND_WIDTH * 2.0f;
	work->diff.vy = BOUND_WIDTH * 2.0f;
	work->diff.vz = BOUND_WIDTH * 2.0f;

	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
//	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, work->repeat_num, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );

	for( i=0; i<work->repeat_num; i++ ){
		InitVectors( work, i, work->repeat_num, 0 );
		InitVectors( work, i, work->repeat_num, 1 );
	}
	DG_VisiblePrim2( work->prim ) ;

	return 0 ;
}

void *NewSplashPolygonOnCamera( int *flag )
{
	Work		*work ;
	int			buf_size;
	int			unit_num;

	OPERATOR() ;

//	unit_num=16;
//	unit_num=12;
//	unit_num=8;
	if ( !GM_CheckGameStatus( STATE_DEMO ) ) {
		unit_num=4;
	}else{
		unit_num=8;
	}

	buf_size = sizeof( Work ) + sizeof( Unit ) * unit_num;
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, buf_size ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->flag = flag;
		work->repeat_num = unit_num;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
