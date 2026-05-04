//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	strip_dome.c
	炎ストリッパ

	2001/06/15 S.Okajima
	$Id: strip_dome.c,v 1.1.1.3 2002/11/19 11:47:38 Yoshizawa1 Exp $
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
#define	COL_G	(96)
#define	COL_B	(64)
#define	COL_A	(64)

#define	RAISE			(0)

#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)

#define	N_VERTS_STRIP	(64)
#define	N_PRIMS_STRIP	(0x1000/16/N_VERTS_STRIP)
#define	N_DATA			(N_PRIMS_STRIP*N_VERTS_STRIP/2)

#define	N_VERTS_STRIP_HALF	(N_VERTS_STRIP/2)

#define	WIDTH_MAX	(40.0f)

#define	POLE_LENGTH_MIN	(400.0f)
#define	POLE_LENGTH_RND	(100.0f)

#define	INIT_LIFE		(120)
#define	INIT_SCALE_UP	(8.0f)

#define	STEP_RATIO	(0.02f)

#define	FADE_OUT_TIME	(120)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;

	int			fade_out_started;
	int			fade_out_count;

	int			init_count;
	int			cycle;
	float		size[N_VERTS_STRIP_HALF];
	float		ratio[N_PRIMS_STRIP];
	FVECTOR		pole[N_PRIMS_STRIP];
	FVECTOR		pole2[N_PRIMS_STRIP];
	FVECTOR		pos[N_DATA];
	FMATRIX		world;
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
static void MakePole( FVECTOR *fvec, float scale )
{
	FVECTOR		fvtemp;
	SVECTOR		svtemp;

	fvtemp.vx = 0.0f;
	fvtemp.vy = 0.0f;
	fvtemp.vz = (POLE_LENGTH_MIN + POLE_LENGTH_RND*rnd()) * scale;

	svtemp.vx = ( (irnd()>>8)&255 ) - 1024;
	svtemp.vy = ( (irnd()>>8)&4095);
	svtemp.vz = 0;

	DG_SetPos2( &DG_ZeroVector, &svtemp );
	DG_RotVector( &fvtemp, fvec, 1 );
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
	int	i,j,k;
	float		fade_ratio;
	float		scale;
	float		*size;
	float		*ratio;
	DG_PRIM2	*prim ;
	FVECTOR		fvtemp;
	FVECTOR		cam;
	SVECTOR		rot;
	DG_PRIM2_UVRGB	*uvrgb;

	prim = work->prim_strip;
	DG_VisiblePrim2( prim );
//	DG_InvisiblePrim2( prim );
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

#if 0
	if( (irnd()>>8)&3 ){
		DG_SetTmpLight2(
			(FVECTOR *)work->world.m[3],
			1250.0f,
			2500.0f,
			COL_R | COL_G<<8 | COL_B<<16,
			LIT_FLAG_CHARAONLY ) ;
	}
#endif

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

	ratio = work->ratio;
	sc_pos  = SCR_POS;
	sc_pos1 = work->pole;
	sc_pos2 = work->pole2;
	uvrgb   = prim->uvrgb[clock];
	for ( k = 0 ; k < N_PRIMS_STRIP ; k++ ){
		size = work->size;
		_sceVu0SubVector( &fvtemp, sc_pos1, sc_pos2 );

		if( (*ratio)<= 0.0f ){
			if( ((irnd()>>8)&15)==0 ){
				(*ratio) = STEP_RATIO;
				MakePole( sc_pos1, 4.0f*rnd() );
				MakePole( sc_pos2, 4.0f*rnd() );
				if( work->init_count > 0 ){
					sc_pos1->vy*= 1.0f + INIT_SCALE_UP * (float)work->init_count / (float)INIT_LIFE;
				}
			}
		}else{
				(*ratio)+= STEP_RATIO;
		}
		if( (*ratio) > 1.0f ) (*ratio) = 0.0f;

		for ( j = 0 ; j < N_VERTS_STRIP_HALF ; j++ ){
			scale  = (float)(N_VERTS_STRIP_HALF - j)/(float)N_VERTS_STRIP_HALF;
			_sceVu0ScaleVector( sc_pos, &fvtemp, scale );
			_sceVu0AddVector( sc_pos, sc_pos, sc_pos2 );
			_sceVu0ScaleVector( sc_pos, sc_pos, ( 1.0f + (*size) ) * (*ratio) );
			size++;
			sc_pos++;
		}

		alpha = (int)( (float)COL_A * (1.0f - (*ratio)) * fade_ratio );
		for ( j = 0 ; j < N_VERTS_STRIP ; j++ ){
			(uvrgb++)->a = alpha;
		}

		ratio++;
		sc_pos1++;
		sc_pos2++;
	}
	OK_Scr_Mem( work->pos, SCR_POS, sizeof(FVECTOR), N_DATA ) ;


	//-----------------------------------------------------幅生成マトリクス
	DG_COPY_MAT( &fmat, &DG_Chanls->eye_inv );
	DG_COPY_VEC( (FVECTOR *)fmat.m[3], &DG_ZeroVector );	// カメラ前に変換

	rot.vx = 0;
	rot.vy = 0;
	rot.vz = 1024;
	DG_SetPos2( &DG_ZeroVector, &rot );
	DG_GetPos( &fmat1 );
	_sceVu0MulMatrix( &fmat, &fmat1, &fmat );	// ９０度回転

	DG_COPY_MAT( &fmat1, &DG_UnitMatrix );
	fmat1.m[2][2] = 0.0f;					// Ｚを潰す
	_sceVu0MulMatrix( &fmat, &fmat1, &fmat );

	DG_COPY_MAT( &fmat1, &DG_Chanls->eye );
	DG_COPY_VEC( (FVECTOR *)fmat1.m[3], &DG_ZeroVector );
	_sceVu0MulMatrix( &fmat, &fmat1, &fmat );	// ワールドに戻す

	//-----------------------------------------------------STRIP

	sc_pos  = SCR_POS;
	sc_pos1 = SCR_POS;
	sc_pos1+= 1;
	sc_pos2 = SCR_TMP;
	for ( k = 0 ; k < N_PRIMS_STRIP ; k++ ){
		size = work->size;
		for ( j = 0 ; j < N_VERTS_STRIP_HALF ; j++ ){
			_sceVu0SubVector( sc_pos2, sc_pos1++, sc_pos++ );	// 差のリストを生成
			_sceVu0Normalize( sc_pos2, sc_pos2 );
			_sceVu0ScaleVector( sc_pos2, sc_pos2, (*size)*WIDTH_MAX );
			sc_pos2++;
			size++;
		}
	}
	sc_pos2 = SCR_TMP;
	DG_SetPos( &fmat );
	DG_RotVector( sc_pos2, sc_pos2, N_DATA );	// 点の間の距離を「カメラから見て幅を持つように」変換


	pos = prim->pos[clock];
	sc_pos  = SCR_POS;
	sc_pos2 = SCR_TMP;
	for ( i=0; i<N_DATA; i++ ){

//AN_Test_Eye2( sc_pos, 2 );

		_sceVu0SubVector( pos++, sc_pos  , sc_pos2   );
		_sceVu0AddVector( pos++, sc_pos++, sc_pos2++ );
	}


	work->init_count--;

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
			uvrgb1->a = uvrgb0->a = COL_A;
			uvrgb0++;
			uvrgb1++;
		}
	}
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work, FVECTOR *center )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int			j;
	float		*size;
	float		*ratio;
	FVECTOR		*p_fv;
	FVECTOR		*p_fv2;

	work->init_count = INIT_LIFE;

	ratio = work->ratio;
	p_fv  = work->pole;
	p_fv2 = work->pole2;
	for ( j = 0 ; j < N_PRIMS_STRIP ; j++ ){
#if 0
		MakePole( p_fv,  4.0f*rnd() );
		MakePole( p_fv2, 4.0f*rnd() );
		(*ratio) = rnd();
#else
		DG_COPY_VEC( p_fv,  &DG_ZeroVector );
		DG_COPY_VEC( p_fv2, &DG_ZeroVector );
		(*ratio) = 0.0f;
#endif
		ratio++;
		p_fv++;
		p_fv2++;
	}

	size = work->size;
	for ( j = 0 ; j < N_VERTS_STRIP_HALF ; j++ ){
		*size = sinf( PI*j/(N_VERTS_STRIP_HALF) );
		size++;
	}

//	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	prim = work->prim_strip = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS_STRIP, N_VERTS_STRIP );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacketStrip( work, prim, tex );

	DG_COPY_MAT( &work->world, &DG_UnitMatrix );
	DG_COPY_VEC( (FVECTOR *)work->world.m[3], center );
	prim->root = &work->world;


	{
		extern void *NewShortSparkProg( FVECTOR *pos, int life );
		NewShortSparkProg( center, 300 );
	}

	work->fade_out_started = 0;

	return 0 ;
}


void *NewStripDome( FVECTOR *center )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		if ( GetResources( work, center ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		GV_SetActorSignalFunc( work, ReceiveSignal ) ;
	}
	return (void *)work ;
}
