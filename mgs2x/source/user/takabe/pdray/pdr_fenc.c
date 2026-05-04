//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	pdr_fenc.c
	量産型ＲＡＹ戦ステージＶＲ柵キャラ

	2001/07/25 K.Takabe
	$Id: pdr_fenc.c,v 1.1.1.3 2002/11/19 11:51:24 Yoshizawa1 Exp $

*/
/*

chara	ＲＡＹステージ環境設定[NewPDRayStageEnvironmentSet] $s:name \
	-flag $w:フラグ


*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"
#include	"libmt.h"
#include	"gameheader.h"
#include	"debugmenu.h"
#include	"camera.h"

#include	"../other/vec_util.h"

/* ---------------------------------------------------------------- */
/* 汎用マクロ */
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define FTOI(_f)	( DG_FTOI(_f) )
#define RND(_n)		( (int)( ( (u_long64)BP_PS2_rand()*(_n) ) / ((u_long64)BP_PS2_RAND_MAX+1) ) )
/* ---------------------------------------------------------------- */
#define DEFAULT_NAME	(1)	/* "" */
/* ---------------------------------------------------------------- */

//#define PRIM_FLAG	(DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define PRIM_FLAG	(DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_ALPHA)
#define N_PRIMS		(6)
#define N_VERTS		(64)

#define TEXTURE		(3182109)		/* "smoke_lp3_alp" */

#define	SET_UVRGB_UV( _p, _u, _v )	{ (_p)->u = (_u) ; (_p)->v = (_v) ; (_p)->q = 4096 ; }
#define	SET_UVRGB_COL( _p, _r, _g, _b, _a )	{ (_p)->r = (_r) ; (_p)->g = (_g) ; (_p)->b = (_b) ; (_p)->a = (_a) ; }

/* ---------------------------------------------------------------- */
typedef struct {
	FVECTOR				pos[ N_PRIMS * N_VERTS ];
	DG_PRIM2_UVRGB		uvrgb[ N_PRIMS * N_VERTS ];
} ScrpadWork ;

/* ---------------------------------------------------------------- */
typedef struct _work{
	GV_ACT_EX		actor ;
	int				name ;
	int				map ;

	DG_TEX			*tex ;
	DG_PRIM2		*prim ;

	int				count ;
	int				count_add ;
} Work ;

//static FVECTOR	base_pos[6] = {
//	
//};

/* ---------------------------------------------------------------- */
	/*
		プログラム使用サブルーチン
	*/

/* メインメモリからスクラッチパッドへ転送 */
static void SprLoad( void *scrpad, void *mem, int num )
{
	/* ＤＭＡを使用しているので使用には細心の注意を払うこと！！ */
	//FlushCache( 0 );
	DG_StartMemToSpr( scrpad, mem, num );
	DG_EndMemToSpr() ;
}
/* スクラッチパッドからメインメモリへ転送 */
static void SprStore( void *scrpad, void *mem, int num )
{
	/* ＤＭＡを使用しているので使用には細心の注意を払うこと！！ */
	//FlushCache( 0 );
	DG_StartSprToMem( mem, scrpad, num );
	DG_EndSprToMem() ;
}

/* ---------------------------------------------------------------- */
	/*
		メイン処理
	*/
static void Act( Work *work )
{
	DG_TEX			*tex ;
	DG_PRIM2		*prim ;
	FVECTOR			*pos ;
	DG_PRIM2_UVRGB	*uvrgb ;
	int				i, j ;
	ScrpadWork		*scrpad = SCRPAD_ADDR ;

	GM_CurrentMap = work->map ;

	tex = work->tex ;

	prim = work->prim ;
	DG_SwitchBuffPrim2( prim );
	pos = prim->pos[ prim->buffer_clock ] ;
	uvrgb = prim->uvrgb[ prim->buffer_clock ] ;

	FlushCache( 0 );
	SprLoad( scrpad->pos, pos, SIZEOF_QWORD(FVECTOR) * N_PRIMS * N_VERTS );
	SprLoad( scrpad->uvrgb, uvrgb, SIZEOF_QWORD(DG_PRIM2_UVRGB) * N_PRIMS * N_VERTS );
	pos = scrpad->pos ;
	uvrgb = scrpad->uvrgb ;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		float		offset ;
		offset = sinf( GTE_PS2RAD( work->count ) ) * 0.5f + 0.5f ;
		for ( j = 0 ; j < N_VERTS/2 ; j++ ){
			FVECTOR		tmp_vec ;
			float		len, alpha ;

			GTE_SubVector( &tmp_vec, pos, &GM_PlayerPosition );
			tmp_vec.vy = 0.0f ;
			len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
			alpha = 140 - 128 * len / 8000.0f ;
			alpha = DG_MIN( 128.0f, alpha );
			alpha = DG_MAX( 0.0f, alpha );
			alpha *= offset ;

			uvrgb[0].a = FTOI( alpha );
			//uvrgb[1].a = FTOI( alpha );
			uvrgb[1].a = 0 ;
#if 0
			{
				float	t, v ;
				t = (float)j / (float)(N_VERTS/2) + offset / (N_VERTS/2) ;
				v = t * tex->v_scale + tex->v_offset ;
				uvrgb[1].v = uvrgb[0].v = FTOI12( v ) ;
			}
#endif

			pos += 2 ;
			uvrgb += 2 ;
		}
	}

	FlushCache( 0 );
	SprStore( scrpad->uvrgb, prim->uvrgb[ prim->buffer_clock ], SIZEOF_QWORD(DG_PRIM2_UVRGB) * N_PRIMS * N_VERTS );

	work->count += 64 ;
}
/* ---------------------------------------------------------------- */
	/*
		終了
	*/
static void Die( Work *work )
{
	GM_FreePrim2( work->prim );
}
/* ---------------------------------------------------------------- */
	/*
		初期化処理
	*/
static int GetResources( Work *work, int name, int where )
{
	DG_PRIM2	*prim ;
	DG_PRIM2_UVRGB	*uvrgb ;
	FVECTOR		*pos, start_pos, end_pos, sincos ;
	DG_TEX		*tex ;
	int			i, j, k ;
	float		size ;

	work->name = name ;
	work->map = where ;
	GM_CurrentMap = where ;

	work->tex = tex = DG_GetTexture( TEXTURE );

	prim = work->prim = GM_MakePrim2( PRIM_FLAG, N_PRIMS, N_VERTS );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 128 ) );
	for ( k = 0 ; k < 2 ; k++ ){
		pos = prim->pos[ k ];
		uvrgb = prim->uvrgb[ k ];
		for ( i = 0 ; i < N_PRIMS ; i++ ){

			size = 25000.0f ;
			GTE_SinCos( &sincos, GTE_PS2RAD( i * 4096 / 6 + 1024 ) );
			start_pos.vx = sincos.vx * size ;
			start_pos.vy = 0.0f ;
			start_pos.vz = sincos.vy * size ;
			start_pos.vw = 1.0f ;
			GTE_SinCos( &sincos, GTE_PS2RAD( (i+1) * 4096 / 6 + 1024 ) );
			end_pos.vx = sincos.vx * size ;
			end_pos.vy = 0.0f ;
			end_pos.vz = sincos.vy * size ;
			end_pos.vw = 1.0f ;

			for ( j = 0 ; j < N_VERTS/2 ; j++, pos+=2, uvrgb+=2 ){
				float		t ;
				t = (float)j / (float)(N_VERTS/2-1) ;
				/* 座標の設定 */
				GTE_InterVector( &pos[0], &start_pos, &end_pos, t );
				pos[1] = pos[0] ;
				pos[0].vy = 4000.0f ;
				pos[1].vy = 5000.0f ;

				/* 頂点情報の設定 */
				SET_UVRGB_COL( &uvrgb[0], 32*2, 46*2, 84*2, 64 );
				SET_UVRGB_COL( &uvrgb[1], 32*2, 46*2, 84*2, 64 );
				{
					float	u, v ;
					u = 0.5f * tex->u_scale + tex->u_offset ;
					v = t * tex->v_scale + tex->v_offset ;
					SET_UVRGB_UV( &uvrgb[0], FTOI12( u ), FTOI12( v ) );
					u = 1.0f * tex->u_scale + tex->u_offset ;
					v = t * tex->v_scale + tex->v_offset ;
					SET_UVRGB_UV( &uvrgb[1], FTOI12( u ), FTOI12( v ) );
				}
				uvrgb[0].f = 0x0fff ;
				uvrgb[1].f = 0x0fff ;
				if ( j == 0 ){
					uvrgb[0].f = 0x8fff ;
					uvrgb[1].f = 0x8fff ;
				}
			}
		}
	}

	work->count = RND( 4096 );
	work->count_add = RND( 8 ) + 32 ;

	return ( 0 );
}

/* ---------------------------------------------------------------- */
	/*
		シナリオ起動インターフェイス
	*/
void *NewPDRayStageFenceSet( int name, int where )
{
	Work *work ;

	OPERATOR();
	work = GV_CreateActor( GV_ACTOR_USER, GV_CLASS_OBJECT, sizeof(Work), 0 );
	if ( work != NULL ){
		GV_SetActor( &work->actor, Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, name, where ) < 0 ){
			GV_DestroyActor( work );
			return ( NULL );
		}
	}
	return ( work );
}

	/*
		プログラム起動インターフェイス
	*/
void *NewPDRayStageFence( void )
{
	Work *work ;

	OPERATOR();
	work = GV_CreateActor( GV_ACTOR_USER, GV_CLASS_OBJECT, sizeof(Work), 0 );
	if ( work != NULL ){
		GV_SetActor( &work->actor, Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, DEFAULT_NAME, GM_CurrentMap ) < 0 ){
			GV_DestroyActor( work );
			return ( NULL );
		}
	}
	return ( work );
}


