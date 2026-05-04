//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	sprt_hola.c
	オーラ

	2001/07/08 S.Okajima
	$Id: sprt_hola.c,v 1.1.1.3 2002/11/19 11:47:37 Yoshizawa1 Exp $
*/


#ifdef PSX2 ///
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libdev.h>
#include <libgraph.h>
#include <libdma.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"
#include	"../../mode/demo/eft_con.h"

#define	P_RGB_MAX		(128)
#define	P_ALPHA_MAX		(64)

#define	RAISE			(0)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)

#define	N_VERTS		(16)
#define	N_PRIMS		(0x2000/16/N_VERTS)

#define	NEXT_COUNT_MIN	(10)
#define	NEXT_COUNT_RND	(30)

#define	FADE_OUT_TIME	(120)

#define	SIZE			(20.0f)
#define	SPREAD_RATIO	(1.02f)

#define	SHIFT_UPPER	(200.0f)

#define	ANGLE	(TPI*1.5f)

#define	LIFE_MAX	(60)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;

	int			fade_out_started;
	int			fade_out_count;

	int			next_count;
	int			now_num;

	int			cycle;

	int			col_dat[LIFE_MAX];

	float		ratio[N_VERTS];

	float		radius[N_PRIMS];
	int			life[N_PRIMS];
	FVECTOR		pole[N_PRIMS];
	FVECTOR		pole2[N_PRIMS];

	FMATRIX		world;
	FVECTOR		*center;
	FVECTOR		*edge0;
	FVECTOR		*edge1;
	DG_PRIM2	*prim;
} Work ;

/* ---------------------------------------------------------------- */
static int ReceiveSignal( void *pwork, int signal, int value )
{
	Work *work = pwork;

	switch(signal){
	  case 0:
		work->fade_out_started = 1;
		work->fade_out_count = FADE_OUT_TIME;
		break;
	  default:
		return GV_DefaultSignalFunc( pwork, signal, value );// Must do this!!!
	}
	return 0;
}

/*----------------------------------------------------------------*/
static void Act( Work *work )
{
	FVECTOR	*sc_pos;
	FVECTOR	*sc_pos1;
	FVECTOR	*sc_pos2;
	int	clock;
	int	alpha;
	int	num;
	int	*life;
	int	*col_dat;
	int	j,k;
	float		fade_ratio;
	float		scale;
	float		*ratio;
	float		*radius;
	DG_PRIM2	*prim ;
	FVECTOR		fvtemp;
	FVECTOR		width;
	FVECTOR		cam;
	DG_PRIM2_UVRGBWH	*uvrgbwh;


	if( --work->next_count < 0 ){
		num = work->now_num;
		sc_pos1 = work->pole;
		sc_pos2 = work->pole2;
		radius  = work->radius;
		life    = work->life;
		sc_pos1+= num;
		sc_pos2+= num;
		radius += num;
		life   += num;

		if( (*life)>= LIFE_MAX ){
//printf("INIT:%d:%d:\n",num,(*life));
			(*life) = 0;
			work->next_count = NEXT_COUNT_MIN + (irnd()>>8)%NEXT_COUNT_RND;
			DG_COPY_VEC( sc_pos1, work->edge0 );	// vw をフラグとして使用する
			DG_COPY_VEC( sc_pos2, work->edge1 );
			_sceVu0SubVector( &width, sc_pos1, sc_pos2 );
			_sceVu0ScaleVector( &width, &width, 1.0f + rnd()*0.25f );
			_sceVu0SubVector( sc_pos2, sc_pos2, &width );
			_sceVu0AddVector( sc_pos1, sc_pos1, &width );
			(*radius) = GV_VecLen3F( &width );
			work->now_num++;
			if( work->now_num>= N_PRIMS ) work->now_num = 0;
		}
	}



//AN_Test_Eye2( work->edge0, 2 );
//AN_Test_Eye2( work->edge1, 2 );


	prim = work->prim;
	DG_VisiblePrim2( prim );
//	DG_InvisiblePrim2( prim );
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	if( work->fade_out_started ){
		fade_ratio = (float)work->fade_out_count / (float)FADE_OUT_TIME;
		work->fade_out_count--;
		if( work->fade_out_count < 0 ){
			GV_DestroyActor( work ) ;
			DG_InvisiblePrim2( prim );
			return;
		}
	}else{
		fade_ratio = 1.0f;
	}

	DG_COPY_VEC( &cam, (FVECTOR *)DG_Chanls->eye.m[3] );


	sc_pos  = prim->pos[clock];
	sc_pos1 = work->pole;
	sc_pos2 = work->pole2;
	life    = work->life;
	col_dat = work->col_dat;
	radius  = work->radius;
	uvrgbwh = prim->uvrgb[clock];
	for ( k = 0 ; k < N_PRIMS ; k++ ){
		_sceVu0SubVector( &fvtemp, sc_pos1, sc_pos2 );
		if( ++(*life) < LIFE_MAX ){
			alpha  = col_dat[(*life)];
			ratio  = work->ratio;
			for ( j = 0 ; j < N_VERTS; j++ ){
				scale  = (float)j/(float)(N_VERTS-1);
				_sceVu0ScaleVector( sc_pos, &fvtemp, scale );
				_sceVu0AddVector( sc_pos, sc_pos, sc_pos2 );

				sc_pos->vy+= SHIFT_UPPER + (*radius)*(*ratio);

				(uvrgbwh++)->a = alpha;
				sc_pos++;
				ratio++;
			}
			(*radius)*= SPREAD_RATIO;
		}else{
			sc_pos+= N_VERTS;
			for ( j=0; j<N_VERTS; j++ ){
				(uvrgbwh++)->a = 0;
			}
		}

		sc_pos1++;
		sc_pos2++;
		radius++;
		life++;
	}

}

/*----------------------------------------------------------------*/
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/* ---------------------------------------------------------------- */
static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		j, k ;
	float	size;
	float	angle;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 1, 2, 0, 1, 0x00 ) );

	//-------------------------------
	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for ( j = 0 ; j < N_PRIMS ; j++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			uvrgbwh1->u0 = uvrgbwh0->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh1->v0 = uvrgbwh0->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh1->u1 = uvrgbwh0->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh1->v1 = uvrgbwh0->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh1->q0 = uvrgbwh0->q0 = 4096 ;
			uvrgbwh1->q1 = uvrgbwh0->q1 = 4096 ;
			uvrgbwh1->f0 = uvrgbwh0->f0 = 0x0fff ;
			uvrgbwh1->f1 = uvrgbwh0->f1 = 0x0fff ;

			size = SIZE * (1.0f + rnd()) * 0.5f;
			angle = TPI*rnd();
			uvrgbwh1->w = uvrgbwh0->w = (int)(cosf( angle ) * size);
			uvrgbwh1->h = uvrgbwh0->h = (int)(sinf( angle ) * size);

			uvrgbwh1->r = uvrgbwh0->r = P_RGB_MAX;
			uvrgbwh1->g = uvrgbwh0->g = P_RGB_MAX;
			uvrgbwh1->b = uvrgbwh0->b = P_RGB_MAX;
			uvrgbwh1->a = uvrgbwh0->a = 0;

			uvrgbwh0 ++ ;
			uvrgbwh1 ++ ;
		}
	}
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int			j;
	int			*col_dat;
	int			*life;
	float		*ratio;
	float		*radius;
	float		ftemp;
	FVECTOR		*p_fv;
	FVECTOR		*p_fv2;

	work->next_count = 0;
	work->now_num = 0;

	col_dat = work->col_dat;
	for ( j=0; j<LIFE_MAX; j++ ){
		(*col_dat++) = (int)((float)P_ALPHA_MAX * sinf( PI * (float)j / (float)(LIFE_MAX-1) )   );
	}


	radius  = work->radius;
	life    = work->life;
	p_fv    = work->pole;
	p_fv2   = work->pole2;
	for ( j = 0 ; j < N_PRIMS ; j++ ){
		DG_COPY_VEC( p_fv++,  &DG_ZeroVector );	// vw をフラグとして使用する
		DG_COPY_VEC( p_fv2++, &DG_ZeroVector );
		(*life++) = LIFE_MAX;
		(*radius++) = 0.0f;
	}

	ratio  = work->ratio;
	for ( j = 0 ; j < N_VERTS ; j++ ){
		ftemp      = (float)j/(float)(N_VERTS-1);
		(*ratio++)  = sinf( PI * ftemp ) + sinf( ANGLE * ftemp )*0.25f;
	}


	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex );

	work->fade_out_started = 0;

	return 0 ;
}


void *NewSprtHola( FVECTOR *edge0, FVECTOR *edge1 )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->edge0  = edge0;
		work->edge1  = edge1;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		GV_SetActorSignalFunc( work, ReceiveSignal ) ;
	}
	return (void *)work ;
}
