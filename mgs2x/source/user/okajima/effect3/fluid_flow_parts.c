//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	fluid_flow_parts.c
	テクスチャスクロール
	ストリップで曲面形成

	2001/06/06 S.Okajima
	$Id: fluid_flow_parts.c,v 1.1.1.3 2002/11/19 11:47:31 Yoshizawa1 Exp $
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
#include	"utl_dma.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../etc/ok_util.h"

#define RAISE		 (0)

#define	SCR_POS0	(SCRPAD_ADDR + 0x00)
#define	SCR_POS1	(SCRPAD_ADDR + 0x10)
#define	SCR_POS2	(SCRPAD_ADDR + 0x20)
#define	SCR_POS3	(SCRPAD_ADDR + 0x30)

#define COL_R (64)
#define COL_G (255)
#define COL_B (255)
#define COL_A (16)

//#define	N_VERTS		(32)
#define	N_PRIMS		(1)
//#define	N_TOTAL		(N_PRIMS*N_VERTS)
//#define	N_STRIP		(N_PRIMS*N_VERTS/2-1)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;

	DG_TEX		*tex ;
	DG_PRIM2	*prim;
	int			side;

	int			num;
	int			life;
	int			life_max;

	int			*col;

} Work ;

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *edge0, FVECTOR *edge1 )
{
	FVECTOR			*pos0;
	FVECTOR			*pos1;
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	int		k, l ;
	int		n_verts ;
	int		n_verts_half ;
	int		side ;
	int		col_r;
	int		col_g;
	int		col_b;

	col_r = (((*work->col)>>24)&255);
	col_g = (((*work->col)>>16)&255);
	col_b = (((*work->col)>> 8)&255);

	n_verts_half = N_PRIMS*work->num;
	n_verts      = n_verts_half*2;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	pos0   = prim->pos[ 0 ];
	pos1   = prim->pos[ 1 ];
	uvrgb0 = prim->uvrgb[ 0 ];
	uvrgb1 = prim->uvrgb[ 1 ];
	side = work->side&1;
	if( work->side&2 ){
		for ( k = 0 ; k < n_verts ; k++ ){
			l = k+side;
			if(k&1){
				DG_COPY_VEC( pos0, edge0++ );
				DG_COPY_VEC( pos1++, pos0++ );
			}else{
				DG_COPY_VEC( pos0, edge1++ );
				DG_COPY_VEC( pos1++, pos0++ );
			}

			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = (k<2)? 0x8fff: 0x0fff;
			uvrgb1->r = uvrgb0->r = col_r ;
			uvrgb1->g = uvrgb0->g = col_g ;
			uvrgb1->b = uvrgb0->b = col_b ;
			uvrgb0++;
			uvrgb1++;
		}
	}else{
		for ( k = 0 ; k < n_verts ; k++ ){
			l = k+side;
			if(k&1){
				DG_COPY_VEC( pos0, edge0++ );
				DG_COPY_VEC( pos1++, pos0++ );
			}else{
				DG_COPY_VEC( pos0, edge1++ );
				DG_COPY_VEC( pos1++, pos0++ );
			}

			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = (k<2)? 0x8fff: 0x0fff;
			uvrgb1->r = uvrgb0->r = col_r ;
			uvrgb1->g = uvrgb0->g = col_g ;
			uvrgb1->b = uvrgb0->b = col_b ;
			uvrgb0++;
			uvrgb1++;
		}
	}

}


/*----------------------------------------------------------------*/
#pragma optimize("",off) /*yano 2002.03.15*/
static void Update_UV( Work *work, int clock )
{
	DG_PRIM2_UVRGB	*uvrgb;
	DG_TEX *tex;
	int		k, l ;
	int		side ;
	float	fnum ;
	float	start ;
	float	ftemp ;
	int		col_a ;
	int		n_verts ;
	int		n_verts_half ;

	col_a = (((*work->col)>> 0)&255) / 2;

	n_verts_half = N_PRIMS*work->num;
	n_verts      = n_verts_half*2;

//if(GV_Time%30==0)printf("col_a:%d\n",col_a);

	// -1.0 ～ 1.0
	start = -1.0f + (float)(work->life*2)/(float)work->life_max;

	tex = work->tex;
	fnum = (float)n_verts;
	uvrgb = work->prim->uvrgb[ clock ];
	side = work->side&1;
//printf("\n");
	if( work->side&2 ){
		for ( k = 0 ; k < n_verts ; k++ ){
			ftemp = start
			      + (float)(k/2) * 2.0f/(fnum-2.0f);
//printf("ftemp:%f\n",ftemp);
			if( ftemp < 0.0f ){
				ftemp = 0.0f;
			}else if( ftemp > 1.0f ){
				ftemp = 1.0f;
			}

			l = k+side;
			uvrgb->u = FTOI12( ftemp * tex->u_scale + tex->u_offset );
			uvrgb->v = (l&1)?FTOI12( 1.0f * tex->v_scale + tex->v_offset ):FTOI12( tex->v_offset );
			if( k < n_verts_half ){
				uvrgb->a = col_a * k           / (n_verts_half-1);
			}else{
				uvrgb->a = col_a * (n_verts-k-1) / (n_verts_half-1);
			}
			uvrgb++;
		}
	}else{
		for ( k = 0 ; k < n_verts ; k++ ){
			ftemp = start
			      + (float)(k/2) * 2.0f/(fnum-2.0f);
			if( ftemp < 0.0f ){
				ftemp = 0.0f;
			}else if( ftemp > 1.0f ){
				ftemp = 1.0f;
			}

			l = k+side;
			uvrgb->v = FTOI12( ftemp * tex->v_scale + tex->v_offset );
			uvrgb->u = (l&1)?FTOI12( 1.0f * tex->u_scale + tex->u_offset ):FTOI12( tex->u_offset );
			if( k < n_verts_half ){
				uvrgb->a = col_a * k           / (n_verts_half-1);
			}else{
				uvrgb->a = col_a * (n_verts-k-1) / (n_verts_half-1);
			}
			uvrgb++;
		}
	}

}
#pragma optimize("",on) /*yano 2002.03.15*/



/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2		*prim ;
	int		clock;

	prim = work->prim;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );
	DG_VisiblePrim2( work->prim );
	clock = prim->buffer_clock;

	Update_UV( work, clock );

	if( work->life-- <= 0 ) GV_DestroyActor( work ) ;
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int GetResources( Work *work, FVECTOR *edge0, FVECTOR *edge1 )
{
	DG_PRIM2	*prim ;

	work->side = (irnd()>>8)&3;

//	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );
//	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
//	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
	work->tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
//	work->tex = DG_GetTexture( 8819182 /*"alpha03_alp_ovl"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, work->num*2 );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, work->tex, edge0, edge1 );

	DG_InvisiblePrim2( work->prim );

	return 0 ;
}

/* ---------------------------------------------------------------- */
/*
座標のポインタ先を毎フレーム参照
*/
// 注）num は 最大（32/2）= 16
// 増やす時は N_PRIMS を 増やす
void *FluidFlowParts( FVECTOR *edge0, FVECTOR *edge1, int num, int life, int *col )
{
	Work		*work ;

	OPERATOR() ;

	// FVECTOR列 二つ分確保
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		work->life = work->life_max = life;
		work->num  = num;
		work->col = col;

		if ( GetResources( work, edge0, edge1 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

