//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	dust_cone.c
	ライト内のダスト円錐
	2001/07/28 S.Okajima
	$Id: dust_cone.c,v 1.1.1.3 2002/11/19 11:47:30 Yoshizawa1 Exp $

	pack_trnall stage_effect.tri \
		debug/shibata/staffroll/alpha03_alp_ovl.bmp

	pack_trnall stage_effect.tri \
		effect/bakuha/bombsphere01_alp.bmp \
		effect/bakuha/bombsphere03_alp.bmp \
		effect/bakuha/bombsphere04_alp.bmp

*/


#ifdef PSX2	///
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

#include	"../etc/ok_util.h"


#define P0	 (0)
#define P1	 (1)
#define P2	 (6)
#define P3	 (1)
#define PHASE0	 (P0)
#define PHASE1	 (P0 + P1)
#define PHASE2	 (P0 + P1 + P2)
#define PHASE3	 (P0 + P1 + P2 + P3)
#define PHASE_TOTAL	 (PHASE3)
#define PHASE0_RATIO	 (0.0f)
#define PHASE1_RATIO	 (1.0f)
#define PHASE2_RATIO	 (0.5f)
#define PHASE3_RATIO	 (0.0f)



#define RAISE		 (0)

#define	SCR_POS0		(SCRPAD_ADDR + 0x0000)
#define	SCR_POS1		(SCRPAD_ADDR + 0x2000)

//	 4* 4	 3* 8	  24
//	 8* 8	 7*16	 112
//	16*16	15*32	 480
//	32*32	31*64	1984

#define	N_SIDES0			(16)	/*  */
#define	N_SIDES1			(32)	/* 円周分割数 */
#define	N_TOTAL_SIDES	(N_SIDES0 * N_SIDES1)

#define	N_LOOP0		(N_SIDES0 - 1)
#define	N_LOOP1		(2 * N_SIDES1)
#define	N_TOTAL		( N_LOOP0 * N_LOOP1 )

#define	N_VERTS		(N_LOOP1)
#define	N_PRIMS		(N_TOTAL/N_VERTS)

#define	SCROLL_DIV		(2048)

#define	NORM_LIMT		(0.1f)

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;

	DG_PRIM2	*prim;

	FVECTOR		cone[N_SIDES0*N_SIDES1];

	FVECTOR		center;
	FVECTOR		direction;
	FMATRIX		world;

	float		offset;
	float		shift_max;

	float		min_len;
	float		max_len;
	float		max_radius;


	int			col;
	int			value;
	int			scroll_count;
} Work ;

static	ALIGN16_PRE float	ALIGN16_POST	OK_WS_WorkSin[N_SIDES1];
static	ALIGN16_PRE float	ALIGN16_POST	OK_WS_WorkCos[N_SIDES1];

static	FVECTOR	WorkTemp[N_SIDES1];

/* ---------------------------------------------------------------- */
static int ReceiveSignal( void *pwork, int signal, int value )
{
	Work *work = pwork;

	switch(signal){
	  case 0:
		work->value = value;
		break;
	  default:
		return GV_DefaultSignalFunc( pwork, signal, value );// Must do this!!!
	}
	return 0;
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2		*prim;
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	FVECTOR		*latice_pos;
	FVECTOR		*fvp0;
	FVECTOR		*fvp1;
	FVECTOR		fv_inner;
	FMATRIX		fmat0;
	FMATRIX		fmat1;
	int		clock;
	int		i,j;
	float	alpha;
	float	alpha_max;
	float	shift;
	float	ftemp1;
	float	ftemp;

	prim     = work->prim;

	DG_VisiblePrim2( prim );
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	shift = work->shift_max * (float)(work->scroll_count) / (float)SCROLL_DIV
	      + work->offset;

	uvrgb0 = prim->uvrgb[clock];
	uvrgb1 = uvrgb0;
	uvrgb1++;


	fv_inner.vx = 0.0f;
	fv_inner.vy = 0.0f;
	fv_inner.vz = 1.0f;
	DG_SetPos( &DG_Chanls->eye );
	DG_RotVector( &fv_inner, &fv_inner, 1 );




	//-----------------------------------------------------計算用マトリクス
	DG_COPY_MAT( &fmat0, &DG_UnitMatrix );
	fmat0.m[2][2] = 0.0f;

	DG_COPY_MAT( &fmat1, &work->world );
	DG_COPY_VEC( (FVECTOR *)fmat1.m[3], &DG_ZeroVector );
	_sceVu0MulMatrix( &fmat0, &fmat1, &fmat0 );

	DG_COPY_MAT( &fmat1, &DG_Chanls->eye_inv );
	DG_COPY_VEC( (FVECTOR *)fmat1.m[3], &DG_ZeroVector );
	_sceVu0MulMatrix( &fmat1, &fmat1, &fmat0 );

	//-----------------------------------------------------計算用マトリクス
	// ワールドに置いてからカメラ前に変換
	// α計算用
	_sceVu0MulMatrix( &fmat0, &DG_Chanls->eye_inv, &work->world );	// fmat0:カメラ前座標系（ゼロ原点）

	alpha_max = (float)(work->col&255) * (float)work->value / 4096.0f;

	latice_pos = work->cone;
	for ( i=0; i<N_SIDES0; i++ ){
		ftemp1 = work->shift_max * (float)i / (float)(N_SIDES0) + shift;
		alpha = alpha_max * (float)(N_SIDES0-i) / (float)(N_SIDES0);

		if( i==0 ){
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				uvrgb0->a = 0;
				uvrgb0->u = FTOI12( ftemp1 );
				uvrgb0+= 2;
			}
			latice_pos+= N_SIDES1;
		}else if( i==N_SIDES0-1 ){
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				uvrgb1->a = 0;
				uvrgb1->u = FTOI12( ftemp1 );
				uvrgb1+= 2;
			}
			latice_pos+= N_SIDES1;
		}else{
			DG_SetPos( &fmat0 );
			DG_PutVector( latice_pos, SCR_POS0, N_SIDES1 );	// カメラ前頂点
			DG_SetPos( &fmat1 );
			DG_PutVector( latice_pos, SCR_POS1, N_SIDES1 );	// カメラ前法線
			fvp0 = SCR_POS0;
			fvp1 = SCR_POS1;
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				_sceVu0Normalize( fvp0, fvp0 );		// カメラから見た頂点までのベクトル
				_sceVu0Normalize( fvp1, fvp1 );		// 法線ベクトル
//if(i==4 && GV_Time%500==0)printf("%d:%f %f %f\n",j,fvp0->vx,fvp0->vy,fvp0->vz);
//if(GV_Time%500==0)printf("%d:%f %f %f\n",j,fvp1->vx,fvp1->vy,fvp1->vz);
//if(i==4 && j==0)printf("%f %f %f :: %f %f %f\n",fvp0->vx,fvp0->vy,fvp0->vz,fvp1->vx,fvp1->vy,fvp1->vz);

#if 1
				ftemp = DG_FABS( _sceVu0InnerProduct( fvp0, fvp1 ) );
				if( ftemp < NORM_LIMT ){
					ftemp = 0.0f;
				}else{
					ftemp = (ftemp - NORM_LIMT) / (1.0f - NORM_LIMT);
				}
#else
				ftemp = _sceVu0InnerProduct( fvp0, fvp1 );
				ftemp*= ftemp;
#endif
				uvrgb0->a = uvrgb1->a = (int)(alpha * ftemp );
//				uvrgb0->a = uvrgb1->a = 255;
				uvrgb0->u = uvrgb1->u = FTOI12( ftemp1 );
				uvrgb0+= 2;
				uvrgb1+= 2;
				fvp0++;
				fvp1++;
			}
			latice_pos+= N_SIDES1;
		}

	}

	work->scroll_count++;
	if( work->scroll_count > SCROLL_DIV ) work->scroll_count = 0;


}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim     = OK_FreePrim2( work->prim );
}

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	int		i, k ;
	int		col_r;
	int		col_g;
	int		col_b;
	float	margine_offset;
	float	margine_scale;
	float	ftemp0;
	float	ftemp1;
	float	ftemp2;
	float	angle;
	float	*p_sin;
	float	*p_cos;

	col_r = (work->col>>24)&255;
	col_g = (work->col>>16)&255;
	col_b = (work->col>> 8)&255;


	work->offset    = tex->u_offset;
	work->shift_max = tex->u_scale*0.5f;


	p_sin = OK_WS_WorkSin;
	p_cos = OK_WS_WorkCos;
	for ( k = 0 ; k < N_SIDES1 ; k++ ){
		angle = TPI * (float)k / (float)(N_SIDES1-1);
		(*p_sin++) = sinf( angle );
		(*p_cos++) = cosf( angle );
	}

	margine_offset = tex->v_offset;
	margine_scale  = tex->v_scale;

	uvrgb0 = prim->uvrgb[ 0 ];
	uvrgb1 = prim->uvrgb[ 1 ];
	ftemp0 = margine_scale * 2.0f / (float)(N_LOOP1-2);
	for ( i = 0 ; i < N_LOOP0 ; i++ ){
		ftemp1 = tex->u_scale * (float)(i  )/(float)N_LOOP0;
		ftemp2 = tex->u_scale * (float)(i+1)/(float)N_LOOP0;
		for ( k = 0 ; k < N_LOOP1 ; k++ ){
			if( k&1 ){
				uvrgb1->u = uvrgb0->u = FTOI12( ftemp2 + tex->u_offset );
			}else{
				uvrgb1->u = uvrgb0->u = FTOI12( ftemp1 + tex->u_offset );
			}
			uvrgb1->v = uvrgb0->v = FTOI12((float)(k>>1) * ftemp0 + margine_offset );
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = (k==0)? 0x8fff: 0x0fff;
			uvrgb1->r = uvrgb0->r = col_r;
			uvrgb1->g = uvrgb0->g = col_g;
			uvrgb1->b = uvrgb0->b = col_b;
			uvrgb1->a = uvrgb0->a = 0;
			uvrgb0++;
			uvrgb1++;
		}
	}
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	SVECTOR 	svtemp;
	FVECTOR		*base0;
	FVECTOR		*base1;
	FVECTOR		*latice_pos;
	FVECTOR		*fvp;
	int		i,j,k;
	float	ratio;
	float	len_ratio;
	float	radius;
	float	length;
	float	angle;




	work->scroll_count = 0;

	OK_DirVecXY( &DG_ZeroVector, &work->direction, &svtemp );
	DG_SetPos2( &work->center, &svtemp );
	DG_GetPos( &work->world );

//	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
//	tex = DG_GetTexture( 8038630 /*"wave12_alp_ovl"*/ );
	tex = DG_GetTexture( 8819182 /*"alpha03_alp_ovl"*/ );
//	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("mo:null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	prim->raise = RAISE;
	prim->root = &work->world;

	DG_InvisiblePrim2( prim );


	len_ratio = work->min_len / work->max_len;


	latice_pos = work->cone;
	base0   = SCR_POS0;
	base1   = base0;
	base1++;
	for ( i=0; i<N_SIDES0; i++ ){
		ratio = (float)i     / (float)(N_SIDES0-1);
		radius = work->max_radius * (len_ratio + ( 1.0f - len_ratio ) * ratio);
		length = work->min_len + (work->max_len - work->min_len) * ratio;
		fvp = latice_pos;
		for ( k = 0 ; k < N_SIDES1 ; k++ ){
			angle = TPI * (float)k / (float)(N_SIDES1-1);
			latice_pos->vx = radius * sinf( angle );
			latice_pos->vy = radius * cosf( angle );
			latice_pos->vz = length;
			latice_pos++;
		}

		if( i==0 ){
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				DG_COPY_VEC( base0, fvp );
				base0+=2;
				fvp++;
			}
		}else if( i==N_SIDES0-1 ){
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				DG_COPY_VEC( base1, fvp );
				base1+=2;
				fvp++;
			}
		}else{
			DG_RotVector( SCR_POS0, WorkTemp, N_SIDES1 );
			for ( j = 0 ; j < N_SIDES1 ; j++ ){
				DG_COPY_VEC( base0, fvp );
				DG_COPY_VEC( base1, fvp );
				base0+=2;
				base1+=2;
				fvp++;
			}
		}
	}
	OK_Scr_Mem( prim->pos[0],  SCR_POS0, sizeof(FVECTOR), N_PRIMS*N_VERTS ) ;
	OK_Scr_Mem( prim->pos[1],  SCR_POS0, sizeof(FVECTOR), N_PRIMS*N_VERTS ) ;




	return 0 ;
}

void *NewDustCone( FVECTOR *center, FVECTOR *direction, float min_len, float max_len, float max_radius, int col )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->center,    center );
		DG_COPY_VEC( &work->direction, direction );

		work->min_len    = min_len;
		work->max_len    = max_len;
		work->max_radius = max_radius;
		work->col  = col;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		GV_SetActorSignalFunc( work, ReceiveSignal ) ;

	}
	return (void *)work ;
}

