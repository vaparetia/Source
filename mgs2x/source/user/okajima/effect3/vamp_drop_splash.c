//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vamp_drop_splash.c
	体からの水飛沫：水面付近で発生

	2000/04/29 S.Okajima
	$Id: vamp_drop_splash.c,v 1.1.1.3 2002/11/19 11:47:38 Yoshizawa1 Exp $
*/

#ifdef PSX2 ///
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

#include <libutl.h>
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"utl_dma.h"
#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"


#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_VEC		(SCRPAD_ADDR + 0x2000)

#define	N_VERTS		(32)
#define	N_PRIMS		(16)
#define	N_LOOPS		(8)

#define	N_PARTS		(N_PRIMS*N_VERTS)

#if 1
#define	TEX_NAME		(3594043 /*"drop01_msk"*/)
#define	P_RGB_MAX		(255)
#define	P_ALPHA_MAX		(64)
#define	SIZE			(8.0f)
#else
#define	TEX_NAME		(8781729 /*"splash07_alp"*/)
#define	P_RGB_MAX		(255)
#define	P_ALPHA_MAX		(32)
#define	SIZE			(1000.0f)
#endif

#define	LIFE		(120)

#define	VEC_LEN		(100.0f)
#define	VEC_MIN_Y	(50.0f)

#define	RADIUS_RATIO	(0.1f)
#define	OPOSIT_RATIO	(0.01f)

#define	ORIGIN_WEIGHT	(4.0f)

#define	WIDE_RATIO		(0.1f)
#define	HEIGHT_RATIO	(16.0f)

#define	RIPPLE_NEAR_LIMIT	(250.0f)

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
extern void OK_PutKirariCross( FVECTOR *pos, float width_ratio );
/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX		actor ;

	FVECTOR		center;
	FVECTOR		master_vec;
	FVECTOR		unit_master_vec;
	float		radius;

	int			life;
	int			life_max;
	int			loop_num;

	DG_PRIM2	*prim ;
	FVECTOR		vec[N_VERTS*N_PRIMS*N_LOOPS];

} Work ;


/* ---------------------------------------------------------------- */
static void InitVecs( Work *work, int clock )
{
	FVECTOR				*sc_vec;
	FVECTOR				*vec;
	FVECTOR				*sc_pos;
	FVECTOR				*pos;
	DG_PRIM2_UVRGBWH	*sc_uvr ;	/* スプライト用 */
	int		j ;
	float	ftemp;
	float	radius;
	DG_PRIM2	*prim ;

	prim = work->prim;

	//-------------------------------
	sc_vec  = work->vec;
	sc_pos  = prim->pos[clock];
	sc_vec += N_VERTS*N_PRIMS*work->loop_num;
	sc_pos += N_VERTS*N_PRIMS*work->loop_num;

	pos     = SCR_POS;
	vec     = SCR_VEC;
	for ( j=0; j<N_PRIMS*N_VERTS; j++ ){

		radius = work->radius*WIDE_RATIO*rnd();
		ftemp  = TPI*rnd();
		pos->vx = sinf(ftemp);
		pos->vy = 0.0f;
		pos->vz = cosf(ftemp);

		ftemp = _sceVu0InnerProduct( &work->unit_master_vec, pos );
		if( ftemp < 0.0f ){
			vec->vx = work->master_vec.vx - radius*pos->vx;
			vec->vy = work->master_vec.vy * rnd();
			vec->vz = work->master_vec.vz - radius*pos->vz;
		}else if( ftemp < 0.25f ){
			vec->vx = work->master_vec.vx * 2.0f + radius*pos->vx;
			vec->vy = work->master_vec.vy * rnd();
			vec->vz = work->master_vec.vz * 2.0f + radius*pos->vz;
		}else{
			vec->vx = work->master_vec.vx + radius*pos->vx;
			vec->vy = work->master_vec.vy * (( ftemp+1.0f)*5.0f + rnd() )*0.1f;
			vec->vz = work->master_vec.vz + radius*pos->vz;
		}

		_sceVu0ScaleVector( vec, vec, rnd() );

		radius = work->radius*rnd();
		pos->vx = work->center.vx + radius*pos->vx;
		pos->vy = GM_WaterLevel + work->radius*rnd();
		pos->vz = work->center.vz + radius*pos->vz;

		pos++;
		vec++;
	}
	OK_Scr_Mem( sc_pos, SCR_POS, sizeof(FVECTOR),N_VERTS*N_PRIMS ) ;
	OK_Scr_Mem( sc_vec, SCR_VEC, sizeof(FVECTOR),N_VERTS*N_PRIMS ) ;


	//-------------------------------
	sc_uvr = prim->uvrgb[clock];
	sc_uvr+= N_VERTS*N_PRIMS*work->loop_num;
	for ( j=0; j<N_PRIMS*N_VERTS; j++ ){
		sc_uvr->w = sc_uvr->h = SIZE;
		sc_uvr++ ;
	}

	work->loop_num++;


}


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2	*prim ;
	FVECTOR		*pos;
	FVECTOR		*sc_pos0;
	FVECTOR		*sc_pos1;
	FVECTOR		*vec;
	FVECTOR		fvtemp;
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int			i,j;
	int			clock;
	int			count;

	prim = work->prim;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;

	vec     = work->vec;
	sc_pos0 = prim->pos[  1-clock ];
	sc_pos1 = prim->pos[    clock ];
	uvrgbwh0= prim->uvrgb[1-clock];
	uvrgbwh1= prim->uvrgb[clock];
	count = 0;
	for ( i=0; i<N_LOOPS; i++ ){
		OK_Mem_Scr( SCR_POS, sc_pos0, sizeof(FVECTOR), N_PRIMS*N_VERTS);
		pos = SCR_POS;
		for ( j=0; j<N_PRIMS*N_VERTS; j++ ){
			if( pos->vy >= GM_WaterLevel ){
				if( (irnd()>>8)&1 )OK_PutKirariCross( pos, 1.0f );
				_sceVu0AddVector( pos, pos, vec );
				vec->vy+= P_GRAVITY;
				uvrgbwh1->w = uvrgbwh1->h = uvrgbwh0->w;
				uvrgbwh1->a = uvrgbwh0->a;
			}else{
				if( uvrgbwh1->a!=0 ){
					count++;
					if( (count&15)==0 ){
						_sceVu0SubVector( &fvtemp, &work->center, pos ) ;
						if( GV_VecLen3F( &fvtemp ) > RIPPLE_NEAR_LIMIT ){
							extern int OK_PutRipple( FVECTOR *center );
							OK_PutRipple( pos );
						}
					}
				}
				uvrgbwh1->w = uvrgbwh1->h = 0;
				uvrgbwh1->a = 0;
			}

			pos++;
			vec++;
			uvrgbwh0++;
			uvrgbwh1++;
		}
		OK_Scr_Mem( sc_pos1, SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS);
		sc_pos0+= N_PRIMS*N_VERTS;
		sc_pos1+= N_PRIMS*N_VERTS;
	}

	if( work->loop_num < N_LOOPS ) InitVecs( work, clock );
	if( work->life-- < 0 ) GV_DestroyActor( work ) ;

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/* ---------------------------------------------------------------- */
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*sc_uvr0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*sc_uvr1 ;	/* スプライト用 */
	int		i,j ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );


// 初期ベクトルを中心とした円周を生成し、Ｙを傾斜させる

	work->unit_master_vec.vx = work->master_vec.vx;
	work->unit_master_vec.vy = work->master_vec.vy * HEIGHT_RATIO;
	work->unit_master_vec.vz = work->master_vec.vz;
	_sceVu0Normalize( &work->unit_master_vec, &work->unit_master_vec );
	_sceVu0ScaleVector( &work->master_vec, &work->unit_master_vec, VEC_LEN );

	work->unit_master_vec.vy = 0.0f;
	_sceVu0Normalize( &work->unit_master_vec, &work->unit_master_vec );

	//-------------------------------
	sc_uvr0 = prim->uvrgb[0];
	sc_uvr1 = prim->uvrgb[1];
	for ( i=0; i<N_LOOPS; i++ ){
		uvrgbwh = SCR_POS ;
		for ( j=0; j<N_PRIMS*N_VERTS; j++ ){
			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

			uvrgbwh->w = uvrgbwh->h = 0;

			uvrgbwh->r = P_RGB_MAX ;
			uvrgbwh->g = P_RGB_MAX ;
			uvrgbwh->b = P_RGB_MAX ;
			uvrgbwh->a = P_ALPHA_MAX ;

			uvrgbwh++ ;

		}
		OK_Scr_Mem( sc_uvr0, SCR_POS, sizeof(DG_PRIM2_UVRGBWH), N_VERTS*N_PRIMS ) ;
		OK_Scr_Mem( sc_uvr1, SCR_POS, sizeof(DG_PRIM2_UVRGBWH), N_VERTS*N_PRIMS ) ;
		sc_uvr0+= N_VERTS*N_PRIMS;
		sc_uvr1+= N_VERTS*N_PRIMS;
	}

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * N_LOOPS * N_VERTS * N_PRIMS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * N_LOOPS * N_VERTS * N_PRIMS );


	InitVecs( work, prim->buffer_clock );
}


static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	FVECTOR		fvtemp;

	work->loop_num = 0;

	tex = DG_GetTexture( TEX_NAME );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_LOOPS*N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );
	DG_VisiblePrim2( work->prim );

	work->life = work->life_max = LIFE;


	if( GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE) ){
		_sceVu0SubVector( &fvtemp, &work->center, (FVECTOR *)DG_Chanls->eye.m[3] );
		if( GV_VecLen3F( &fvtemp ) < 5000.0f ){
			extern void *NewWaterDropsMany( int life );
			extern void *NewScrDrop( int life );
//			NewWaterDropsMany( -1 );
			NewScrDrop( -1 );
		}
	}

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewVampDropSplash( FVECTOR *center, FVECTOR *vec, float radius )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActorPrio( GV_ACTOR_EFFECT, sizeof( Work ), GV_PRIO_MAX-32 ) ;
//	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->center, center );
		work->center.vy = GM_WaterLevel;

		DG_COPY_VEC( &work->master_vec, vec );

		work->radius = (radius > 1.0f)? radius: 1.0f;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

