//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   ef_spk2.c
   ＲＡＹバルカン用跳弾エフェクト
   
   2001/07/18 K.Takabe
   $Id: ef_spk2.c,v 1.1.1.3 2002/11/19 11:51:22 Yoshizawa1 Exp $
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
#define N_PRIMS		(1)
#define N_VERTS		(4*16)
#define N_UNITS		(N_VERTS/4)

//#define TEXTURE		(14071854)		/* "flare_r1_msk" */
//#define TEXTURE		(7733153)		/* "splash06_alp" */
#define TEXTURE		(11171645)		/* "fire1_alp" */

#define	SET_UVRGB_UV( _p, _u, _v )	{ (_p)->u = (_u) ; (_p)->v = (_v) ; (_p)->q = 4096 ; }
#define	SET_UVRGB_COL( _p, _r, _g, _b, _a )	{ (_p)->r = (_r) ; (_p)->g = (_g) ; (_p)->b = (_b) ; (_p)->a = (_a) ; }

/*------------------------------------------------------------*/
/* 汎用マクロ */
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define FTOI(_f)	( DG_FTOI(_f) )
#define RND(_n)		( (int)( ( (u_long64)BP_PS2_rand()*(_n) ) / ((u_long64)BP_PS2_RAND_MAX+1) ) )
/*------------------------------------------------------------*/
#define ALIVE_TIME	(8*5)
#define BASE_SIZE	(512)

/*------------------------------------------------------------*/

typedef struct	{
    GV_ACT		actor ;
    FMATRIX		world ;
    DG_PRIM2	*prim ;
	FVECTOR		pos[N_UNITS][2] ;
	FVECTOR		vvec[ N_UNITS ] ;
	float		size1, size2 ;
	int			time ;
} Work ;


/*------------------------------------------------------------*/
static void CalcPrimPos( FVECTOR *pos, FVECTOR *from, FVECTOR *to, float size1, float size2 )
{
#if 0
	/* カメラから見て必ず厚みがあるように */
	FVECTOR		vec1, vec2 ;
	FVECTOR		vec ;
	GTE_LoadMatrix( &( DG_Chanl( 0 )->eye_pers ) );
	GTE_PutVector1( &vec1, from );
	GTE_PutVector1( &vec2, to );
	vec.vx = vec2.vy * vec1.vw - vec1.vy * vec2.vw ;
	vec.vy = vec1.vx * vec2.vw - vec2.vx * vec1.vw ;
	vec.vz = 0 ;
	vec.vw = 0 ;
	GTE_Normalize( &vec, &vec );
	GTE_LoadMatrix( &( DG_Chanl( 0 )->eye ) );
	GTE_RotVector1( &vec, &vec );
#if 1
	GTE_ScaleVector( &vec, &vec, size );
	GTE_AddVector( pos + 0, to, &vec );
	GTE_SubVector( pos + 1, to, &vec );
	GTE_AddVector( pos + 2, from, &vec );
	GTE_SubVector( pos + 3, from, &vec );
#else
	/* 画面から見て常に一定の太さの場合 */
	GTE_ScaleVector( &vec1, &vec, vec1.vw / 128 );
	GTE_ScaleVector( &vec2, &vec, vec2.vw / 128 );
	GTE_AddVector( pos + 0, to, &vec2 );
	GTE_SubVector( pos + 1, to, &vec2 );
	GTE_AddVector( pos + 2, from, &vec1 );
	GTE_SubVector( pos + 3, from, &vec1 );
#endif
#else

	/* カメラから見て必ず厚みがあるように */
	FVECTOR		vec1, vec2 ;
	FVECTOR		vec, vec_v, vec_h ;
	GTE_LoadMatrix( &( DG_Chanl( 0 )->eye_pers ) );
	GTE_PutVector1( &vec1, from );
	GTE_PutVector1( &vec2, to );
	vec.vx = vec2.vy * vec1.vw - vec1.vy * vec2.vw ;
	vec.vy = vec1.vx * vec2.vw - vec2.vx * vec1.vw ;
	vec.vz = 0 ;
	vec.vw = 0 ;
	GTE_Normalize( &vec, &vec );
	vec1.vx = -vec.vy ;
	vec1.vy = vec.vx ;
	vec1.vz = 0 ;
	vec1.vw = 0 ;
	GTE_LoadMatrix( &( DG_Chanl( 0 )->eye ) );
	GTE_RotVector1( &vec_h, &vec );
	GTE_RotVector1( &vec_v, &vec1 );

	GTE_ScaleVector( &vec1, &vec_h, size2 );
	GTE_ScaleVector( &vec2, &vec_v, size2 );
	GTE_AddVector( pos + 0, to, &vec1 );
	GTE_SubVector( pos + 1, to, &vec1 );
	//GTE_AddVector( pos + 0, pos + 0, &vec2 );
	//GTE_AddVector( pos + 1, pos + 1, &vec2 );

	GTE_ScaleVector( &vec1, &vec_h, size1 );
	GTE_ScaleVector( &vec2, &vec_v, size1 );
	GTE_AddVector( pos + 2, from, &vec1 );
	GTE_SubVector( pos + 3, from, &vec1 );
	//GTE_SubVector( pos + 2, pos + 2, &vec2 );
	//GTE_SubVector( pos + 3, pos + 3, &vec2 );

#endif
}
/*------------------------------------------------------------*/

static	void	Act( Work *work )
{
	FVECTOR		tmp_vec, from ;
	FVECTOR			*pos ;
	DG_PRIM2_UVRGB	*uvrgb ;
	int		i, alpha ;
	float	t ;

	t = (float)work->time / (float)ALIVE_TIME ;
	t = 1.0f - t ;
	t *= t ;
	alpha = DG_FTOI( 128 * t ) ;

	DG_SwitchBuffPrim2( work->prim ) ;
	pos = work->prim->pos[ work->prim->buffer_clock ] ;
	uvrgb = work->prim->uvrgb[ work->prim->buffer_clock ] ;
	GTE_ZeroVector( &tmp_vec );
	GTE_ApplyMatrix( &from, &work->world, &tmp_vec );
	for ( i = 0 ; i < N_UNITS ; i++ ){
		FVECTOR		tmp_vec ;
		float		size ;

		size = work->pos[ i ][ 1 ].vw ;
		/* プリミティブ座標設定 */
		CalcPrimPos( pos, &work->pos[ i ][ 0 ], &work->pos[ i ][ 1 ], size/20.0f, size/30.0f );
		/* 座標更新 */
		GTE_AddVector( &work->pos[ i ][ 1 ], &work->pos[ i ][ 1 ], &work->vvec[ i ] );
		GTE_ScaleVector( &tmp_vec, &work->vvec[ i ], 0.5f );
		GTE_AddVector( &work->pos[ i ][ 0 ], &work->pos[ i ][ 0 ], &tmp_vec );
		/* スピード減衰＆落下加速 */
		GTE_ScaleVector( &work->vvec[ i ], &work->vvec[ i ], 0.7f );

#if 0
		SET_UVRGB_COL( &uvrgb[0], 128, 128, 128, alpha );
		SET_UVRGB_COL( &uvrgb[1], 128, 128, 128, alpha );
		SET_UVRGB_COL( &uvrgb[2], 128, 128, 128, alpha );
		SET_UVRGB_COL( &uvrgb[3], 128, 128, 128, alpha );
#else
		{
			int		r, g, b ;
			r = 150 ;
			g = 120 ;
			b = 40 ;
			SET_UVRGB_COL( &uvrgb[0], r, g, b, alpha );
			SET_UVRGB_COL( &uvrgb[1], r, g, b, alpha );
			SET_UVRGB_COL( &uvrgb[2], r, g, b, alpha );
			SET_UVRGB_COL( &uvrgb[3], r, g, b, alpha );
		}
#endif

		pos += 4 ;
		uvrgb += 4 ;
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
	FVECTOR			*pos ;
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
	for ( i = 0; i < 2; i ++ ) {
		uvrgb = prim->uvrgb[ i ] ;
		pos = prim->pos[ i ] ;
		for ( j = 0 ; j < N_PRIMS * N_VERTS ; j += 4 ){
			int		index ;
			index = RND( 4 );
			pos[ 0 ] = pos[ 1 ] = pos[ 2 ] = pos[ 3 ] = DG_ZeroVector ;
			SET_UVRGB_COL( &uvrgb[0], 128, 128, 128, 128 );
			SET_UVRGB_COL( &uvrgb[1], 128, 128, 128, 128 );
			SET_UVRGB_COL( &uvrgb[2], 128, 128, 128, 0 );
			SET_UVRGB_COL( &uvrgb[3], 128, 128, 128, 0 );
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

	/* 座標初期化 */
	for ( i = 0 ; i < N_UNITS ; i++ ){
		static FVECTOR		base_vec = {0,1,0,1};
		FMATRIX		mat ;
		FVECTOR		tmp_vec ;
		int			angle_x, angle_y ;
		float		size ;

		/* MGS1の跳弾パラメータをそのまま使用した */
		angle_x = RND(256) ;
		angle_y = RND(4096) ;
		size =  ( 64 * BASE_SIZE ) / (float)( angle_x + 64 ) ;
		GTE_ScaleVector( &tmp_vec, &base_vec, size );
		tmp_vec.vw = 1.0f ;

		GTE_UnitMatrix( &mat );
		_sceVu0RotMatrixX( &mat, &mat, GTE_PS2RAD( angle_x ) );
		_sceVu0RotMatrixY( &mat, &mat, GTE_PS2RAD( angle_y ) );

		GTE_ZeroVector( &work->pos[i][0] );
		GTE_ApplyMatrix( &work->pos[i][1], &mat, &tmp_vec );
		//work->vvec[ i ] = work->pos[i][1] ;

		GTE_LoadMatrix( &work->world );
		GTE_RotVector1( &work->vvec[ i ], &work->pos[i][1] );
		GTE_PutVector( &work->pos[i][0], &work->pos[i][0], 2 );
		work->pos[i][1].vw = size ;
		work->vvec[i].vw = size ;
	}

	return 0 ;
}

void	*NewRayValcanSpark( FMATRIX *world )
{
	Work	*work ;

	work = ( Work * )GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, world ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
