//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	strip_hola.c
	オーラ

	2001/06/15 S.Okajima
	$Id: strip_hola.c,v 1.1.1.3 2002/11/19 11:47:38 Yoshizawa1 Exp $
*/



#ifdef PSX2 ///
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"
#include	"../../mode/demo/eft_con.h"

#define	COL_R	(128)
#define	COL_G	(196)
#define	COL_B	(255)
#define	COL_A	(255)

#define	RAISE			(0)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)

#define	N_VERTS_STRIP	(64)
#define	N_PRIMS_STRIP	(0x2000/16/N_VERTS_STRIP)
#define	N_DATA			(N_PRIMS_STRIP*N_VERTS_STRIP/2)

#define	N_VERTS_STRIP_HALF	(N_VERTS_STRIP/2)

#define	NEXT_COUNT_MIN	(10)
#define	NEXT_COUNT_RND	(30)

#define	FADE_OUT_TIME	(120)

#define	WIDTH_MIN		(10.0f)
#define	WIDTH_ADD		(10.0f)
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

	float		ratio[N_VERTS_STRIP_HALF];

	float		radius[N_PRIMS_STRIP];
	int			life[N_PRIMS_STRIP];
	FVECTOR		pole[N_PRIMS_STRIP];
	FVECTOR		pole2[N_PRIMS_STRIP];

	FVECTOR		pos[N_DATA];
	FMATRIX		world;
	FVECTOR		*center;
	FVECTOR		*edge0;
	FVECTOR		*edge1;
	DG_PRIM2	*prim_strip;
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
	FMATRIX	fmat;
	FMATRIX	fmat1;
	FVECTOR	*sc_pos;
	FVECTOR	*sc_pos1;
	FVECTOR	*sc_pos2;
	FVECTOR	*pos;
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
	FVECTOR		local_width;
	FVECTOR		cam;
	SVECTOR		rot;
	DG_PRIM2_UVRGB	*uvrgb;


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
			if( work->now_num>= N_PRIMS_STRIP ) work->now_num = 0;
		}
	}



//AN_Test_Eye2( work->edge0, 2 );
//AN_Test_Eye2( work->edge1, 2 );


	prim = work->prim_strip;
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

	//-----------------------------------------------------幅生成マトリクス
	DG_COPY_MAT( &fmat, &DG_Chanls->eye_inv );
	DG_COPY_VEC( (FVECTOR *)fmat.m[3], &DG_ZeroVector );	// カメラ前に変換

	DG_COPY_MAT( &fmat1, &DG_UnitMatrix );
	fmat1.m[2][2] = 0.0f;					// Ｚを潰す
	_sceVu0MulMatrix( &fmat, &fmat1, &fmat );

	rot.vx = 0;
	rot.vy = 0;
	rot.vz = 1024;
	DG_SetPos2( &DG_ZeroVector, &rot );
	DG_GetPos( &fmat1 );
	_sceVu0MulMatrix( &fmat, &fmat1, &fmat );	// ９０度回転

	DG_COPY_MAT( &fmat1, &DG_Chanls->eye );
	DG_COPY_VEC( (FVECTOR *)fmat1.m[3], &DG_ZeroVector );
	_sceVu0MulMatrix( &fmat, &fmat1, &fmat );	// ワールドに戻す

	DG_SetPos( &fmat );

	pos     = prim->pos[clock];
	sc_pos  = SCR_POS;
	sc_pos1 = work->pole;
	sc_pos2 = work->pole2;
	life    = work->life;
	col_dat = work->col_dat;
	radius  = work->radius;
	uvrgb   = prim->uvrgb[clock];
	for ( k = 0 ; k < N_PRIMS_STRIP ; k++ ){
		_sceVu0SubVector( &fvtemp, sc_pos1, sc_pos2 );
		DG_RotVector( &fvtemp, &width, 1 );
		_sceVu0Normalize( &width, &width );
//		_sceVu0ScaleVector( &width, &width, WIDTH );

		if( ++(*life) < LIFE_MAX ){
			alpha  = col_dat[(*life)];
//printf("%d %d %d \n",k,(*life),alpha);
			ratio  = work->ratio;
			for ( j = 0 ; j < N_VERTS_STRIP_HALF; j++ ){
				scale  = (float)j/(float)(N_VERTS_STRIP_HALF-1);
				_sceVu0ScaleVector( sc_pos, &fvtemp, scale );
				_sceVu0AddVector( sc_pos, sc_pos, sc_pos2 );

				sc_pos->vy+= SHIFT_UPPER + (*radius)*(*ratio);

				_sceVu0ScaleVector( &local_width, &width, WIDTH_MIN + WIDTH_ADD*(*ratio) );

//AN_Test_Eye2( sc_pos, 2 );

				_sceVu0SubVector( pos++, sc_pos, &local_width );
				_sceVu0AddVector( pos++, sc_pos, &local_width );

				(uvrgb++)->a = alpha;
				(uvrgb++)->a = alpha;
				sc_pos++;
				ratio++;
			}
			(*radius)*= SPREAD_RATIO;
		}else{
			uvrgb += N_VERTS_STRIP_HALF*2;
			sc_pos+= N_VERTS_STRIP_HALF;
			for ( j=0; j<N_VERTS_STRIP_HALF; j++ ){
				DG_COPY_VEC( pos++, &DG_ZeroVector );
				DG_COPY_VEC( pos++, &DG_ZeroVector );
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
	work->prim_strip = OK_FreePrim2( work->prim_strip );
}

/*----------------------------------------------------------------*/
static void InitPacketStrip( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	int		j, k ;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	uvrgb0 = prim->uvrgb[ 0 ];
	uvrgb1 = prim->uvrgb[ 1 ];
	for ( k = 0 ; k < N_PRIMS_STRIP ; k++ ){
		for ( j = 0 ; j < N_VERTS_STRIP ; j++ ){
			uvrgb1->u = uvrgb0->u = FTOI12((float)(j/2) * 2.0f/(float)(N_VERTS_STRIP) * tex->u_scale + tex->u_offset );
			uvrgb1->v = uvrgb0->v = (j&1)?FTOI12( 1.0f * tex->v_scale + tex->v_offset ):FTOI12( tex->v_offset );
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = (j<2)? 0x8fff: 0x0fff;
			uvrgb1->r = uvrgb0->r = COL_R ;
			uvrgb1->g = uvrgb0->g = COL_G ;
			uvrgb1->b = uvrgb0->b = COL_B ;
			uvrgb1->a = uvrgb0->a = 0;
			uvrgb0++;
			uvrgb1++;
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
		(*col_dat++) = (int)((float)COL_A * sinf( PI * (float)j / (float)(LIFE_MAX-1) )   );
	}


	radius  = work->radius;
	life    = work->life;
	p_fv    = work->pole;
	p_fv2   = work->pole2;
	for ( j = 0 ; j < N_PRIMS_STRIP ; j++ ){
		DG_COPY_VEC( p_fv++,  &DG_ZeroVector );	// vw をフラグとして使用する
		DG_COPY_VEC( p_fv2++, &DG_ZeroVector );
		(*life++) = LIFE_MAX;
		(*radius++) = 0.0f;
	}

	ratio  = work->ratio;
	for ( j = 0 ; j < N_VERTS_STRIP_HALF ; j++ ){
		ftemp      = (float)j/(float)(N_VERTS_STRIP_HALF-1);
		(*ratio++)  = sinf( PI * ftemp ) + sinf( ANGLE * ftemp )*0.25f;
	}


	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	prim = work->prim_strip = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS_STRIP, N_VERTS_STRIP );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacketStrip( work, prim, tex );

	work->fade_out_started = 0;

	return 0 ;
}


void *NewStripHola( FVECTOR *edge0, FVECTOR *edge1 )
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
