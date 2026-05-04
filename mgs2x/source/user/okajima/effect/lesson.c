//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	lesson.c
	練習用雛形
	2000/04/17 S.Okajima
	$Id: lesson.c,v 1.1.1.3 2002/11/19 11:47:07 Yoshizawa1 Exp $

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

#define	COL_R			(32)
#define	COL_G			(128)
#define	COL_B			(255)
#define	ALPHA			(64)

#define N_PRIMS		(8)
#define N_VERTS		(16)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)

#define	LIFE		(3000)

#define	RAISE				(0)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;

	int			name;
	int			where;


	DG_PRIM2	*prim ;
	FVECTOR		center;
	FVECTOR		force;
	FVECTOR		vec[ N_PRIMS * N_VERTS ];
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
	FVECTOR	*pos;
	FVECTOR	*vec;
	FVECTOR	*pos_before;
	FVECTOR	diff;
	float	param = 5000.0f;
	float	len2;


   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;

	alpha = ALPHA * work->life / work->life_max;
	if( alpha < 0 ) alpha=0;

	vec            = work->vec;
	uvrgbwh        = work->prim->uvrgb[clock];
	pos            = work->prim->pos[clock];
	pos_before     = work->prim->pos[1-clock];
	for( i=0; i<N_PRIMS; i++ ){
		for( j=0; j<N_VERTS; j++ ){
			pos->vx  = pos_before->vx + vec->vx;
			pos->vy  = pos_before->vy + vec->vy;
			pos->vz  = pos_before->vz + vec->vz;

#if 1
			/* プレイヤーに重力 */
			diff.vx = GM_PlayerPosition.vx - pos->vx;
			diff.vy = GM_PlayerPosition.vy - pos->vy;
			diff.vz = GM_PlayerPosition.vz - pos->vz;
			diff.vw = 0.0f;
			len2 = _sceVu0InnerProduct( &diff, &diff ) ;
			if( len2 < 0.01f ){
				DG_COPY_VEC( pos, &work->center );
			}else{
				vec->vx += param * (diff.vx) / len2 ;
				vec->vy += param * (diff.vy) / len2 ;
				vec->vz += param * (diff.vz) / len2 ;
			}
#endif

			uvrgbwh->a = alpha;
			if( alpha == 0 ) uvrgbwh->h = uvrgbwh->w = 0;

			pos++;
			pos_before++;
			vec++;
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

static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *center, SVECTOR *pole_rot, float size, float speed )
{
	FVECTOR		*pos;
	FVECTOR		*vec;
	FVECTOR		fvtemp;
	SVECTOR		svtemp;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i, k ;
	float		angle;
	float		ftemp;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;
	vec       = work->vec;
	fvtemp.vy = 0.0f;
	svtemp.vx = 0;
	svtemp.vy = 0;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			DG_COPY_VEC( pos, center );

			/* 速度ランダマイズ */
			ftemp       = speed*rnd();
			angle       = rnd()*TPI*0.03f;
			fvtemp.vz   = ftemp * cosf( angle );
			fvtemp.vx   = ftemp * sinf( angle );
			svtemp.vz   = irnd()%4096;
			DG_SetPos2( &DG_ZeroVector, &svtemp );
			DG_RotVector( &fvtemp, vec, 1 );

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
			uvrgbwh->g  = COL_G ;
			uvrgbwh->b  = COL_B ;
			uvrgbwh->a  = ALPHA ;


			pos ++;
			vec ++;
			uvrgbwh ++ ;
		}
	}

	DG_SetPos2( &DG_ZeroVector, pole_rot );
	DG_RotVector( work->vec, work->vec, N_PRIMS*N_VERTS );


	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );

	return 1;
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work, int name, int where )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	SVECTOR		svtemp;

	work->name  = name;
	work->where = where;

	/* 初期位置きめきめ */
	work->center.vx = 0.0f;
	work->center.vy = 0.0f;
	work->center.vz = 0.0f;

#if 0
	/* 速度ベクトルきめきめ */
	work->force.vx = frnd() * 1000.0f;
	work->force.vy = 2000.0f;
	work->force.vz = 3000.0f;
	work->length = GV_VecLen3F( &work->force );
	if( work->length != 0.0f ){
		OK_DirVecXY( &DG_ZeroVector, &work->force, &svtemp );
	}else{
		svtemp.vx = 0;
		svtemp.vy = 0;
		svtemp.vz = 0;
	}
#else
	svtemp.vx = irnd()%4096;
	svtemp.vy = irnd()%2048;
	svtemp.vz = 0;
#endif

	/* 回転スプライト */
	prim = work->prim = GM_MakePrim2(
				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				N_PRIMS,
				N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	work->life_max = work->life = LIFE;

//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
	InitPacket( work, prim, tex, &work->center, &svtemp,500.0f, 60.0f + frnd()*30.0f );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}


void *NewLesson( int name, int where )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

