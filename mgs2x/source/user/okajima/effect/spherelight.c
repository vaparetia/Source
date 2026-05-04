//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	spherelight.c
	球状ライト
	2000/07/28 S.Okajima
	$Id: spherelight.c,v 1.1.1.3 2002/11/19 11:47:14 Yoshizawa1 Exp $

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

#define	COL_R			(4)
#define	COL_G			(16)
#define	COL_B			(32)
#define	ALPHA			(6)

#define N_PRIMS		(4)
#define N_VERTS		(16)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)

#define	RAISE				(0)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;

	int			col_r;
	int			col_g;
	int			col_b;
	float		radius;

	DG_PRIM2	*prim ;
	FVECTOR		center;
	FVECTOR		force;
	int			life;
	int			life_max;
	float		length;
} Work ;
/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int		i,j;
	int		clock;
	int		alpha;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */


	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	alpha = ALPHA * work->life / work->life_max;
	if( alpha < 0 ) alpha=0;

	uvrgbwh        = work->prim->uvrgb[clock];
	for( i=0; i<N_PRIMS; i++ ){
		for( j=0; j<N_VERTS; j++ ){
			uvrgbwh->a = alpha;
			if( alpha == 0 ) uvrgbwh->h = uvrgbwh->w = 0;
			uvrgbwh++;
		}
	}

	work->life--;
	if( work->life < -1 ){
		GV_DestroyActor( work ) ;
	}

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *center, float size )
{
	FVECTOR		*pos;
	FVECTOR		fvtemp;
	SVECTOR		svtemp;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i, k ;
	float		angle;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;
	fvtemp.vy = 0.0f;
	svtemp.vx = 0;
	svtemp.vy = 0;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			DG_COPY_VEC( pos, center );

			angle       = rnd()*TPI;
			uvrgbwh->w  = (int)(size * cosf( angle ));
			uvrgbwh->h  = (int)(size * sinf( angle ));


			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

			uvrgbwh->r  = COL_R ;
			uvrgbwh->g  = COL_R ;
			uvrgbwh->b  = COL_R ;
			uvrgbwh->a  = ALPHA ;


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
static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	/* 回転スプライト */
	prim = work->prim = GM_MakePrim2(
				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				N_PRIMS,
				N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	InitPacket( work, prim, tex, &work->center, work->radius );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}


void *NewSphereLight( FVECTOR *pos, int col_r, int col_g, int col_b, float radius, int time )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->center, pos );
		work->col_r = col_r;
		work->col_g = col_g;
		work->col_b = col_b;
		work->radius = radius;
		work->life     = time;
		work->life_max = time;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

