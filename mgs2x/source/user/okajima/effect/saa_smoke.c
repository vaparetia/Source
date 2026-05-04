//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	saa_smoke.c
	ＳＡＡ煙
	2000/04/19 S.Okajima
	$Id: saa_smoke.c,v 1.1.1.3 2002/11/19 11:47:11 Yoshizawa1 Exp $

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

#define	COL_R			(12)
#define	COL_G			(12)
#define	COL_B			(12)
#define	ALPHA			(64)

#define N_PRIMS		(16)
#define N_VERTS		(16)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)

#define	RAISE				(0)

#define	SIZE_MIN		(10.0f)
#define	SIZE_STP		(1.0f)
#define	VEC_MIN			(5.0f)
#define	VEC_RND			(5.0f)

#define	REPEAT_CYCLE	(128)
#define	FADE_PHASE		(REPEAT_CYCLE - 64)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;

	DG_PRIM2	*prim ;
	FMATRIX		*world;
	FVECTOR		now_center;
	FVECTOR		before_center;
	FVECTOR		vec[ N_PRIMS * N_VERTS ];
	int			poly_count[ N_PRIMS * N_VERTS ];
	int			life;
	int			life_max;
} Work ;

static	FVECTOR	saa_shift = { 17.5f, -322.0f, 67.7f, 0.0f };

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int		i,j;
	int		clock;
	int		alpha;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	FVECTOR	*pos;
	FVECTOR	*vec;
	FVECTOR	*pos_before;
	FVECTOR	fvtemp;
	SVECTOR	svtemp;
	FMATRIX	mat;
	int		count;
	int		*poly_count;
	float	ftemp;

	DG_SetPos( work->world ) ;
	DG_MovePos( &saa_shift ) ;
	DG_GetPos( &mat ) ;

	DG_COPY_VEC( &work->now_center, (FVECTOR *)mat.m[3] );

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;

	alpha = ALPHA * work->life / work->life_max;
	if( alpha < 0 ) alpha=0;

	vec        = work->vec;
	uvrgbwh    = work->prim->uvrgb[clock];
	pos        = work->prim->pos[clock];
	pos_before = work->prim->pos[1-clock];
	poly_count = work->poly_count;
	count      = work->life_max - work->life;
	for( i=0; i<N_PRIMS; i++ ){
		for( j=0; j<N_VERTS; j++ ){
			if( (*poly_count) > 0 ){
				pos->vx  = pos_before->vx + vec->vx;
				pos->vy  = pos_before->vy + vec->vy;
				pos->vz  = pos_before->vz + vec->vz;
				vec->vx *= 0.999f+0.001f*rnd();
				vec->vy *= 0.999f+0.001f*rnd();
				vec->vz *= 0.999f+0.001f*rnd();
				vec->vw += 0.01f;
				ftemp = (SIZE_MIN+(float)(*poly_count)*SIZE_STP);
				uvrgbwh->w  = (int)( ftemp * cosf( vec->vw ) );
				uvrgbwh->h  = (int)( ftemp * sinf( vec->vw ) );
				if( (*poly_count) > FADE_PHASE ){
					uvrgbwh->a = alpha * (REPEAT_CYCLE - (*poly_count)) / FADE_PHASE;
				}else{
					uvrgbwh->a = alpha;
				}
				if( (*poly_count) > REPEAT_CYCLE ) (*poly_count) = 0;
				if( alpha == 0 ) uvrgbwh->h = uvrgbwh->w = 0;
			}

			if( (*poly_count) == 0 ){
				ftemp = rnd();
				pos->vx = work->now_center.vx * ftemp + work->before_center.vx * (1.0f - ftemp) + SIZE_MIN*frnd();
				pos->vy = work->now_center.vy * ftemp + work->before_center.vy * (1.0f - ftemp) + SIZE_MIN*frnd();
				pos->vz = work->now_center.vz * ftemp + work->before_center.vz * (1.0f - ftemp) + SIZE_MIN*frnd();

				fvtemp.vx = 0.0f;
				fvtemp.vy = VEC_MIN + VEC_RND*rnd();
				fvtemp.vz = 0.0f;
//				svtemp.vx = (128 + irnd()%32) * work->life / work->life_max ;
				svtemp.vx = (128 + irnd()%32);
				svtemp.vy = (GV_Time*64 + irnd()%16) * work->life / work->life_max ;
				svtemp.vz = 0;
				DG_SetPos2( &DG_ZeroVector, &svtemp );
				DG_PutVector( &fvtemp, vec, 1 );
				vec->vw    = rnd()*TPI;
				uvrgbwh->a = 0;
				uvrgbwh->h = uvrgbwh->w = 0;
			}
			(*poly_count)++;

			poly_count++;
			pos++;
			pos_before++;
			vec++;
			uvrgbwh++;
		}
	}

	work->life--;
	if( work->life < 0 ){
		GV_DestroyActor( work ) ;
	}

	DG_COPY_VEC( &work->before_center, &work->now_center );

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR		*pos;
	FVECTOR		*vec;
	FVECTOR		center;
	FVECTOR		fvtemp;
	FMATRIX		mat;
	SVECTOR		svtemp;
	int			*poly_count;

	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i, k ;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	DG_SetPos( work->world ) ;
	DG_MovePos( &saa_shift ) ;
	DG_GetPos( &mat ) ;
	DG_COPY_VEC( &center, (FVECTOR *)mat.m[3] );

	pos        = SCR_POS;
	uvrgbwh    = SCR_UVS;
	vec        = work->vec;
	fvtemp.vx  = 0.0f;
	fvtemp.vz  = 0.0f;
	svtemp.vz  = 0;
	poly_count = work->poly_count;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			DG_COPY_VEC( pos, &center );

			(*poly_count) = -( irnd()%(REPEAT_CYCLE) );

			uvrgbwh->w  = 0;
			uvrgbwh->h  = 0;

			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

			uvrgbwh->r  = COL_R ;
			uvrgbwh->g  = COL_G ;
			uvrgbwh->b  = COL_B ;
			uvrgbwh->a  = 0 ;

			pos ++;
			vec ++;
			uvrgbwh ++ ;
			poly_count++;
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
	FMATRIX		mat;

	DG_SetPos( work->world ) ;
	DG_MovePos( &saa_shift ) ;
	DG_GetPos( &mat ) ;
	DG_COPY_VEC( &work->before_center, (FVECTOR *)mat.m[3] );

	/* 回転スプライト */
	prim = work->prim = GM_MakePrim2(
				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				N_PRIMS,
				N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	tex = DG_GetTexture( 9998494 /*"powder02_alp"*/ );
	InitPacket( work, prim, tex );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

/*
world  :[ポインタ参照]銃のマトリクス
life   :フレーム指定。その後自殺する
*/
void *NewSAA_Smoke( FMATRIX *world, int life )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		work->world = world;
		work->life_max = work->life  =life;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

