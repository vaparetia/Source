//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   ef_spl2.c
   水しぶきエフェクト
   
   2001/07/10 K.Takabe
   $Id: ef_spl2.c,v 1.1.1.3 2002/11/19 11:51:22 Yoshizawa1 Exp $
   */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

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

#include	"../other/vec_util.h"

/*------------------------------------------------------------*/

//#define	PRIM_FLAG	(DG_PRIM2_LINE | DG_PRIM2_SHADE | DG_PRIM2_ANTIALIASING)
#define	PRIM_FLAG	(DG_PRIM2_POLY | DG_PRIM2_SHADE | DG_PRIM2_TEX | DG_PRIM2_ALPHA )
#define N_PRIMS		(5)
#define N_VERTS		(4*16)
#define N_UNITS		(N_PRIMS*N_VERTS/4)

//#define TEXTURE		(14071854)		/* "flare_r1_msk" */
//#define TEXTURE		(4458455)		/* "muzzlefrash_01_alp" */
//#define TEXTURE		(7733153)		/* "splash06_alp" */
//#define TEXTURE		(15638432)		/* "blood_1bw_msk" */
#define TEXTURE		(13872561)			/* "blood_1e_msk" */

#define	SET_UVRGB_UV( _p, _u, _v )	{ (_p)->u = (_u) ; (_p)->v = (_v) ; (_p)->q = 4096 ; }
#define	SET_UVRGB_COL( _p, _r, _g, _b, _a )	{ (_p)->r = (_r) ; (_p)->g = (_g) ; (_p)->b = (_b) ; (_p)->a = (_a) ; }

/*------------------------------------------------------------*/
/* 汎用マクロ */
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define FTOI(_f)	( DG_FTOI(_f) )
#define RND(_n)		( (int)( ( (u_long64)BP_PS2_rand()*(_n) ) / ((u_long64)BP_PS2_RAND_MAX+1) ) )
/*------------------------------------------------------------*/
#define ALIVE_TIME	(60*5)
#define SPLASH_SIZE	(2000)
#define SPLASH_ARC	(200)
#define SPLASH_WIDTH	(30.0f/180.0f)

extern float BP_AdjustTick4(float);
#define G_ACCEL		(BP_AdjustTick4(10))


/*------------------------------------------------------------*/

typedef struct	{
    GV_ACT_EX	actor ;
    FMATRIX		world ;
    DG_PRIM2	*prim ;
	FVECTOR		vvec[ N_UNITS * 4 ] ;
	int			time ;
} Work ;


/*------------------------------------------------------------*/

static	void	Act( Work *work )
{
	FVECTOR			*pos, *old_pos, *vvec ;
	DG_PRIM2_UVRGB	*uvrgb ;
	int		i, alpha ;
	float	t ;

	t = (float)work->time / (float)ALIVE_TIME ;
	t = 1.0f - t ;
	t *= t ;
	alpha = DG_FTOI( 64 * t ) ;

	/* 以前の座標データアドレスを取得 */
	old_pos = work->prim->pos[ work->prim->buffer_clock ] ;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	pos = work->prim->pos[ work->prim->buffer_clock ] ;
	uvrgb = work->prim->uvrgb[ work->prim->buffer_clock ] ;

	vvec = work->vvec ;

	for ( i = 0 ; i < N_UNITS ; i++ ){
		/* プリミティブ座標設定 */
		GTE_AddVector( &pos[ 0 ], &old_pos[ 0 ], &vvec[0] );
		GTE_AddVector( &pos[ 1 ], &old_pos[ 1 ], &vvec[1] );
		GTE_AddVector( &pos[ 2 ], &old_pos[ 2 ], &vvec[2] );
		GTE_AddVector( &pos[ 3 ], &old_pos[ 3 ], &vvec[3] );
		/* スピード減衰＆落下加速 */
		vvec[0].vy -= G_ACCEL ;
		vvec[1].vy -= G_ACCEL ;
		if ( pos[ 0 ].vy < 0 ){
			GTE_ScaleVector( &vvec[ 0 ], &vvec[ 0 ], 0.8f );
			GTE_ScaleVector( &vvec[ 1 ], &vvec[ 1 ], 0.8f );
			GTE_ScaleVector( &vvec[ 2 ], &vvec[ 2 ], 0.8f );
			GTE_ScaleVector( &vvec[ 3 ], &vvec[ 3 ], 0.8f );
			vvec[ 0 ].vy = - vvec[ 0 ].vy ;
			vvec[ 1 ].vy = - vvec[ 1 ].vy ;
		}

		SET_UVRGB_COL( &uvrgb[0], 128, 128, 128, alpha );
		SET_UVRGB_COL( &uvrgb[1], 128, 128, 128, alpha );
		SET_UVRGB_COL( &uvrgb[2], 128, 128, 128, alpha );
		SET_UVRGB_COL( &uvrgb[3], 128, 128, 128, alpha );

		old_pos += 4 ;
		pos += 4 ;
		uvrgb += 4 ;
		vvec += 4 ;
	}

	DG_VisiblePrim2( work->prim ) ;

	work->time += TIME_BASE ;
	if ( work->time > ALIVE_TIME ){
		GV_DestroyActor( work );
	}
}

static	void	Die( Work *work )
{
	GM_FreePrim2( work->prim ) ;
}

/*------------------------------------------------------------*/

static	int	GetResources( Work *work, FMATRIX *world )
{
	DG_PRIM2		*prim ;
	DG_PRIM2_UVRGB	*uvrgb ;
	DG_TEX			*tex ;
	FVECTOR			*pos, *vvec ;
	int				i, j ;
	float			u0, v0, u1, v1 ;

	work->world = *world ;


	tex = DG_GetTexture( TEXTURE );
	u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset ) ;
	v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset ) ;
	u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset ) ;
	v1 = FTOI12( 0.8f * tex->v_scale + tex->v_offset ) ;
	v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset ) ;

	prim = work->prim = GM_MakePrim2( PRIM_FLAG, N_PRIMS, N_VERTS ) ;
	if ( prim == NULL ) return -1 ;
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) ) ;
   prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
	prim->as_world = *world ;
	for ( i = 0; i < 2; i ++ ) {
		uvrgb = prim->uvrgb[ i ] ;
		pos = prim->pos[ i ] ;
		for ( j = 0 ; j < N_PRIMS * N_VERTS ; j += 4 ){
			int		index ;
			index = RND( 4 );
			pos[ 0 ] = pos[ 1 ] = pos[ 2 ] = pos[ 3 ] = DG_ZeroVector ;
			SET_UVRGB_COL( &uvrgb[0], 128, 128, 128, 128 );
			SET_UVRGB_COL( &uvrgb[1], 128, 128, 128, 128 );
			SET_UVRGB_COL( &uvrgb[2], 128, 128, 128, 128 );
			SET_UVRGB_COL( &uvrgb[3], 128, 128, 128, 128 );
			SET_UVRGB_UV( uvrgb + 0, u0, v0 );	/* 左上 */
			SET_UVRGB_UV( uvrgb + 1, u1, v0 );	/* 右上 */
			SET_UVRGB_UV( uvrgb + 2, u0, v1 );	/* 左下 */
			SET_UVRGB_UV( uvrgb + 3, u1, v1 );	/* 右下 */
			uvrgb[ 0 ].f = 0x8fff ;
			uvrgb[ 1 ].f = 0x8fff ;
			uvrgb[ 2 ].f = 0x0fff ;
			uvrgb[ 3 ].f = 0x0fff ;
			uvrgb += 4 ;
			pos += 4 ;
		}
	}
	DG_InvisiblePrim2( prim ) ;

	{/* 座標初期化 */
		FVECTOR		*pos0, *pos1 ;
		pos0 = prim->pos[ 0 ] ;
		pos1 = prim->pos[ 1 ] ;
		vvec = work->vvec ;
		for ( i = 0 ; i < N_UNITS ; i++ ){
			FVECTOR		sincos ;
			float		len, height ;
			int			angle ;

			GTE_SinCos( &sincos, GTE_PS2RAD( RND(768)+256 ) );
			angle = RND(4096) ;
			sincos.vx *= sincos.vx ;
			height = sincos.vx * SPLASH_SIZE * 2 * 0.1f ;
			len = sincos.vy * SPLASH_SIZE * 0.1f ;

			GTE_SinCos( &sincos, GTE_PS2RAD( angle ) );
			vvec[0].vx = sincos.vy * len ;
			vvec[0].vy = height ;
			vvec[0].vz = sincos.vx * len ;
			vvec[2].vx = sincos.vy * len * 0.5f ;
			vvec[2].vy = 0.0f ;
			vvec[2].vz = sincos.vx * len * 0.5f ;

			GTE_SinCos( &sincos, GTE_PS2RAD( angle + 368 ) );
			vvec[1].vx = sincos.vy * len ;
			vvec[1].vy = height ;
			vvec[1].vz = sincos.vx * len ;
			vvec[3].vx = sincos.vy * len * 0.5f;
			vvec[3].vy = 0.0f ;
			vvec[3].vz = sincos.vx * len * 0.5f ;


			pos0 += 4 ;
			pos1 += 4 ;
			vvec += 4 ;
		}
	}


	return 0 ;
}

void	*NewRayFootSplash( FMATRIX *world )
{
	Work	*work ;

	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, world ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
	//if ( GV_PadData[ 0 ].press & PAD_A ){
	//	extern void	*NewRayFootSplash( FMATRIX *world );
	//	NewRayFootSplash( &GM_PlayerBody->objs->world );
	//}
