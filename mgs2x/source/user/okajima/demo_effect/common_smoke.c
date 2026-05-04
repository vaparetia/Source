//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	common_smoke.c
	汎用の煙
	2000/10/30 S.Okajima
	$Id: common_smoke.c,v 1.1.1.3 2002/11/19 11:46:51 Yoshizawa1 Exp $

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
#include	"../etc/ok_util.h"

#define N_PRIMS		(1)
#define N_VERTS		(16)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)

#define	RAISE			(0)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;

	DG_PRIM2	*prim ;

	int			size_add;
	float		radius;
	FVECTOR		center;

	float		size[N_PRIMS*N_VERTS];
	float		angle[N_PRIMS*N_VERTS];
	int			life[N_PRIMS*N_VERTS];
	int			life_count;
	int			life_max;
	int			init_count;

	int			col_a;
	int			mode;

} Work ;
/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	int	i;
	int		clock;
	int		*life;
	int		alpha;
	float	*angle;
	float	*size;

	DG_VisiblePrim2( work->prim ) ;
	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	life    = work->life;
	angle   = work->angle;
	size    = work->size;
	uvrgbwh = work->prim->uvrgb[clock];
	if( work->life_count > work->init_count ){	// 初期から
		for( i=0; i<N_PRIMS*N_VERTS; i++ ){
			alpha = work->col_a * (work->life_max - work->life_count) / (work->life_max - work->init_count);
			if( alpha < 0 ) alpha = 0;
			uvrgbwh->a = alpha;
			uvrgbwh->w = (int)(cosf( (*angle) ) * (*size));
			uvrgbwh->h = (int)(sinf( (*angle) ) * (*size));
			(*size) += work->size_add;
			size++;
			life++;
			uvrgbwh++;
		}
	}else{	// 後半
		for( i=0; i<N_PRIMS*N_VERTS; i++ ){
			if( work->life_count > (*life) ){
				alpha = work->col_a;
			}else{
				alpha = work->col_a * work->life_count / (*life);
			}
			if( alpha < 0 ) alpha = 0;
			uvrgbwh->a = alpha;
			uvrgbwh->w = (int)(cosf( (*angle) ) * (*size));
			uvrgbwh->h = (int)(sinf( (*angle) ) * (*size));
			(*size) += work->size_add;
			size++;
			life++;
			uvrgbwh++;
		}
	}

	work->life_count--;
	if( work->life_count < 0 ){
		GV_DestroyActor( work ) ;
	}

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int col_r, int col_g, int col_b, int mode )
{
	SVECTOR		svtemp;
	FVECTOR		fvtemp;
	FVECTOR		*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i, k ;
	int		itemp;
	float	*angle ;
	float	*size ;
	int		*life;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );

	switch( mode ){
	  case 0:
	  default:
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
		break;
	  case 1:
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
		break;
	  case 2:
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
		break;
	  case 3:
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 1, 0, 0, 1, 0x00 ) );
		break;
	}

	work->size_add = (int)(work->radius*0.01f)+1;

	itemp     = work->init_count/2+1;
	angle     = work->angle;
	life      = work->life;
	size      = work->size;
	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;
	svtemp.vz = 0;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){

			(*life) = (irnd()>>8)%itemp + itemp;

			(*angle) = rnd()*TPI;

			svtemp.vx = (irnd()>>8)%2048-1024;
			svtemp.vy = (irnd()>>8)%4096;
			fvtemp.vx = 0.0f;
			fvtemp.vy = 0.0f;
			fvtemp.vz = rnd()*work->radius;
			DG_SetPos2( &work->center, &svtemp );
			DG_PutVector( &fvtemp, pos, 1 );

			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

			(*size) = (rnd()*0.5f + 0.5f) * work->radius;
			uvrgbwh->w = (int)(cosf( (*angle) ) * (*size));
			uvrgbwh->h = (int)(sinf( (*angle) ) * (*size));
			uvrgbwh->r  = col_r;
			uvrgbwh->g  = col_g;
			uvrgbwh->b  = col_b;
			uvrgbwh->a  = 0 ;

			size++;
			life++;
			angle++;
			pos ++;
			uvrgbwh ++ ;
		}
	}


	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );

	return 1;
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work, int col_r, int col_g, int col_b, int mode )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->init_count = work->life_max * 4/5 + 1;

	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );


	/* 回転スプライト */
	prim = work->prim = GM_MakePrim2(
				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				N_PRIMS,
				N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex, col_r, col_g, col_b, mode );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

/*
center :発生位置（へのポインタ）
radius :発生させる煙球の半径
col_r  :
col_g  :
col_b  :
col_a  :
life   :消滅までの時間（フレーム）
mode   :０）加算 １）減算 ２）加算平均 ３）減算平均
*/
void *NewCommonSmoke( FVECTOR *center, float radius, int col_r, int col_g, int col_b, int col_a, int life, int mode )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->center, center );
		work->radius = radius;
		work->col_a = col_a;
		work->life_count = work->life_max = life;

		if ( GetResources( work, col_r, col_g, col_b, mode ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
