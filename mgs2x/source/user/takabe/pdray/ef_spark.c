//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ef_spark.c
	量産型ＲＡＹ用ＶＲ床壊れエフェクト（跳弾）

	2001/06/06 K.Takabe
	$Id: ef_spark.c,v 1.1.1.3 2002/11/19 11:51:22 Yoshizawa1 Exp $

*/
/*



*/
/* !NEED_PAL_READJUST! */

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


#include	"../other/vec_util.h"

/* ---------------------------------------------------------------- */
/* 汎用マクロ */
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define FTOI(_f)	( DG_FTOI(_f) )
#define RND(_n)		( (int)( ( (u_long64)BP_PS2_rand()*(_n) ) / ((u_long64)BP_PS2_RAND_MAX+1) ) )
/* ---------------------------------------------------------------- */
/* シグナル関連 */
#define SIGNAL_RAYMISSILESMOKE			(0x4665dc00)	/* GV_StrCode("RAYMissileSmoke")<<8 */
#define SIGNAL_RAYMISSILESMOKE_STOP		(SIGNAL_RAYMISSILESMOKE|0x01)
/* ローカル用再定義 */
#define SIGNAL_THIS_EFFECT	(SIGNAL_RAYMISSILESMOKE)
#define SIGNAL_STOP			(SIGNAL_THIS_EFFECT|0x01)
/* ---------------------------------------------------------------- */
/* エフェクトパラメータ */
#define ALIVE_FRAME		(20)	/* 生存フレーム数 */
#define PRIM_SCALE		(200)	/* ヘックスのスケール */


extern float BP_AdjustTick4(float);
#define FALL_ACCEL		(BP_AdjustTick4(10))

/* ---------------------------------------------------------------- */
/* プリミティブパラメータ */
#define PRIM_FLAG	(DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define N_BASE_OBJS		(7)
#define N_HEX_POLY_VERTS	(6)
#define N_HEX_LINE_VERTS	(7)
#define N_VERTS_POLY	(N_HEX_POLY_VERTS * N_BASE_OBJS)
#define N_VERTS_LINE	(N_HEX_LINE_VERTS * N_BASE_OBJS)
#define N_PRIMS			(1)
#define TOTAL_OBJS		( N_PRIMS * N_BASE_OBJS )
//#define TEXTURE		(4625891)		/* "bombgas1_alp" */
#define TEXTURE		(9868771)		/* "bombgas6_alp" */

#define COLOR_R		(80)
#define COLOR_G		(160)
#define COLOR_B		(128)

/* ---------------------------------------------------------------- */
/* スクラッチパッド使用状況 */
typedef struct _scrpad{/* 合計１６ＫＢ以下であること！ */
	FVECTOR				pos[ 256 ] ;
	FVECTOR				vvec[ 256 ] ;
	DG_PRIM2_UVRGB		uvrgb[ 256 ] ;
} ScrpadWork ;

/* ---------------------------------------------------------------- */
typedef struct {
	GV_ACT_EX	actor ;
	int			map ;

	FMATRIX		world ;						/* 出現位置マトリクス */
	float		scale ;						/* スケール */
	DG_PRIM2	*prim_poly ;				/* ポリゴン描画用プリミティブ */
	DG_PRIM2	*prim_line ;				/* ライン描画用プリミティブ */

	FVECTOR		vvec[ TOTAL_OBJS ] ;		/* 頂点速度 */
	FMATRIX		mats[ TOTAL_OBJS ];			/* 各破片マトリクス */
	FMATRIX		add_rot[ TOTAL_OBJS ];		/* 回転補正量 */

	int			count ;
} Work ;

/* ---------------------------------------------------------------- */
FVECTOR		poly_base_pos[N_HEX_POLY_VERTS] = {
	{ 1.0f, 0.0f, 0.0f, 1.0f},
	{ 0.5f, 0.0f, 0.8660254f, 1.0f},
	{ 0.5f, 0.0f, -0.8660254f, 1.0f},
	{ -0.5f, 0.0f, 0.8660254f, 1.0f},
	{ -0.5f, 0.0f, -0.8660254f, 1.0f},
	{ -1.0f, 0.0f, 0.0f, 1.0f},
};
FVECTOR		line_base_pos[7] = {
	{ 1.0f, 0.0f, 0.0f, 1.0f},
	{ 0.5f, 0.0f, 0.8660254f, 1.0f},
	{ -0.5f, 0.0f, 0.8660254f, 1.0f},
	{ -1.0f, 0.0f, 0.0f, 1.0f},
	{ -0.5f, 0.0f, -0.8660254f, 1.0f},
	{ 0.5f, 0.0f, -0.8660254f, 1.0f},
	{ 1.0f, 0.0f, 0.0f, 1.0f}
};
FVECTOR		offset_base_pos[N_BASE_OBJS] = {
	{ 0.0f, 0.0f, 0.0f, 1.0f},
	{ 1.0f+0.5f, 0.0f, 0.0f+0.8660254f, 1.0f},
	{ 1.0f+0.5f, 0.0f, 0.0f-0.8660254f, 1.0f},
	{ 0.5f-0.5f, 0.0f, 0.8660254f+0.8660254f, 1.0f},
	{ 0.5f-0.5f, 0.0f, -0.8660254f-0.8660254f, 1.0f},
	{ -1.0f-0.5f, 0.0f, 0.0f+0.8660254f, 1.0f},
	{ -1.0f-0.5f, 0.0f, 0.0f-0.8660254f, 1.0f}
};

/* ---------------------------------------------------------------- */
	/*
		プログラム使用サブルーチン
	*/

#if 0
/* メインメモリからスクラッチパッドへ転送 */
static void SprLoad( void *scrpad, void *mem, int num )
{
	/* ＤＭＡを使用しているので使用には細心の注意を払うこと！！ */
	//FlushCache( 0 );
	DG_StartMemToSpr( scrpad, mem, num );
	DG_EndMemToSpr() ;
}
#endif
/* スクラッチパッドからメインメモリへ転送 */
static void SprStore( void *scrpad, void *mem, int num )
{
	/* ＤＭＡを使用しているので使用には細心の注意を払うこと！！ */
	//FlushCache( 0 );
	DG_StartSprToMem( mem, scrpad, num );
	DG_EndSprToMem() ;
}

/* ---------------------------------------------------------------- */
static void InitWork( Work *work )
{
	int		i ;
	FVECTOR	*vvec ;
	FMATRIX	*mat, *rot ;

	/* マトリクスの初期化 */
	mat = work->mats ;
	rot = work->add_rot ;
	for ( i = 0 ; i < TOTAL_OBJS ; i++ ){
		FVECTOR		tmp_vec ;

		GTE_UnitMatrix( mat );
		mat->m[0][0] = PRIM_SCALE ;
		mat->m[1][1] = PRIM_SCALE ;
		mat->m[2][2] = PRIM_SCALE ;
		//mat->m[3][0] = RND( 200 ) - 100 ;
		//mat->m[3][1] = RND( 200 ) - 100 ;
		//mat->m[3][2] = RND( 200 ) - 100 ;
		GTE_ScaleVector( (FVECTOR*)mat->m[3], &offset_base_pos[ i ], PRIM_SCALE );
		GTE_UnitMatrix( rot );
		tmp_vec.vx = RND( 256 ) - 128 ;
		tmp_vec.vy = RND( 256 ) - 128 ;
		tmp_vec.vz = RND( 256 ) - 128 ;
		GTE_Normalize( &tmp_vec, &tmp_vec );
		GTE_MakeRotateAxis( rot, &tmp_vec, GTE_PS2RAD( RND( 256 ) - 128 ) );
		mat++ ;
		rot++ ;
	}

	/* 速度ベクトルの初期化 */
	vvec = work->vvec ;
	for ( i = 0 ; i < TOTAL_OBJS ; i++ ){
		vvec->vx = RND( 70 ) - 35 ;
		vvec->vz = RND( 70 ) - 35 ;
		vvec->vy = RND( 100 ) + 50 ;
		vvec->vw = 0.0f ;
		GTE_ScaleVector( vvec, vvec, work->scale );
		vvec++ ;
	}

}
static void InitPolyPrims( Work *work )
{
	DG_PRIM2		*prim ;
	FVECTOR			*pos ;
	DG_PRIM2_UVRGB	*uvrgb ;
	int				i, j ;

	prim = work->prim_poly ;
	pos = SCRPAD_ADDR ;
	for ( i = 0 ; i < TOTAL_OBJS ; i++ ){
		GTE_LoadMatrix( &work->mats[ i ] );
		GTE_PutVector( pos, poly_base_pos, N_HEX_POLY_VERTS );
		pos += N_HEX_POLY_VERTS ;
	}
	FlushCache( 0 );
	SprStore( SCRPAD_ADDR, prim->pos[ 0 ], SIZEOF_QWORD(FVECTOR) * N_PRIMS * N_VERTS_POLY );
	SprStore( SCRPAD_ADDR, prim->pos[ 1 ], SIZEOF_QWORD(FVECTOR) * N_PRIMS * N_VERTS_POLY );

	uvrgb = SCRPAD_ADDR ;
	for ( i = 0 ; i < TOTAL_OBJS ; i++ ){
		for ( j = 0 ; j < N_HEX_POLY_VERTS ; j++ ){
			uvrgb->f = ( j < 2 ) ? ( 0x8fff ) : ( 0x0fff ) ;
			uvrgb->r = COLOR_R ;
			uvrgb->g = COLOR_G ;
			uvrgb->b = COLOR_B ;
			uvrgb->a = 48 ;
			uvrgb++ ;
		}
	}
	FlushCache( 0 );
	SprStore( SCRPAD_ADDR, prim->uvrgb[ 0 ], SIZEOF_QWORD(DG_PRIM2_UVRGB) * N_PRIMS * N_VERTS_POLY );
	SprStore( SCRPAD_ADDR, prim->uvrgb[ 1 ], SIZEOF_QWORD(DG_PRIM2_UVRGB) * N_PRIMS * N_VERTS_POLY );

}

static void InitLinePrims( Work *work )
{
	DG_PRIM2		*prim ;
	FVECTOR			*pos ;
	DG_PRIM2_UVRGB	*uvrgb ;
	int				i, j ;

	prim = work->prim_line ;
	pos = SCRPAD_ADDR ;
	for ( i = 0 ; i < TOTAL_OBJS ; i++ ){
		GTE_LoadMatrix( &work->mats[ i ] );
		GTE_PutVector( pos, line_base_pos, N_HEX_LINE_VERTS );
		pos += N_HEX_LINE_VERTS ;
	}
	FlushCache( 0 );
	SprStore( SCRPAD_ADDR, prim->pos[ 0 ], SIZEOF_QWORD(FVECTOR) * N_PRIMS * N_VERTS_LINE );
	SprStore( SCRPAD_ADDR, prim->pos[ 1 ], SIZEOF_QWORD(FVECTOR) * N_PRIMS * N_VERTS_LINE );

	uvrgb = SCRPAD_ADDR ;
	for ( i = 0 ; i < TOTAL_OBJS ; i++ ){
		for ( j = 0 ; j < N_HEX_LINE_VERTS ; j++ ){
			uvrgb->f = ( j == 0 ) ? ( 0x8fff ) : ( 0x0fff ) ;
			uvrgb->r = COLOR_R ;
			uvrgb->g = COLOR_G ;
			uvrgb->b = COLOR_B ;
			uvrgb->a = 48 ;
			uvrgb++ ;
		}
	}
	FlushCache( 0 );
	SprStore( SCRPAD_ADDR, prim->uvrgb[ 0 ], SIZEOF_QWORD(DG_PRIM2_UVRGB) * N_PRIMS * N_VERTS_LINE );
	SprStore( SCRPAD_ADDR, prim->uvrgb[ 1 ], SIZEOF_QWORD(DG_PRIM2_UVRGB) * N_PRIMS * N_VERTS_LINE );
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2	*prim ;
	FVECTOR		*pos ;
	FMATRIX		*mat ;
	DG_PRIM2_UVRGB		*uvrgb ;
	int			i ;

	GM_CurrentMap = work->map ;

	{/* マトリクスの更新処理 */
		FVECTOR		*vvec ;
		FMATRIX		*rot ;

		mat = work->mats ;
		rot = work->add_rot ;
		vvec = work->vvec ;
		for ( i  = 0 ; i < TOTAL_OBJS ; i++ ){
			GTE_MulMatrix( mat, mat, rot );
			GTE_AddVector( (FVECTOR*)mat->m[3], (FVECTOR*)mat->m[3], vvec );
			//GTE_ScaleVector( vvec, vvec, 0.95f );
			vvec->vy -= FALL_ACCEL ;
			if ( mat->m[3][1] < 0.0f ){
				vvec->vy = - vvec->vy ;
				GTE_ScaleVector( vvec, vvec, 0.5f );
			}
			mat++ ;
			rot++ ;
			vvec++ ;
		}
	}

	prim = work->prim_poly ;
	prim->flag &= ~DG_PRIM2_INVISIBLE ;

	/* プリミティブ頂点データのダブルバッファ切り替え */
	DG_SwitchBuffPrim2( prim );

	/* 書き込み用頂点データアドレス取得 */
	pos = prim->pos[ prim->buffer_clock ] ;
	mat = work->mats ;
	for ( i  = 0 ; i < TOTAL_OBJS ; i++ ){
		GTE_LoadMatrix( mat );
		GTE_PutVector( pos, poly_base_pos, N_HEX_POLY_VERTS );
		pos += N_HEX_POLY_VERTS ;
		mat++ ;
	}

	uvrgb = prim->uvrgb[ prim->buffer_clock ] ;
	{
		int		alpha ;
		alpha = DG_FTOI( ( 1.0f - (float)work->count / ALIVE_FRAME ) * 24 ) ;
		if ( alpha < 0 ) alpha = 0 ;
		for ( i  = 0 ; i < TOTAL_OBJS ; i++ ){
			uvrgb[0].a = alpha ;
			uvrgb[1].a = alpha ;
			uvrgb[2].a = alpha ;
			uvrgb[3].a = alpha ;
			uvrgb[4].a = alpha ;
			uvrgb[5].a = alpha ;
			uvrgb += 6 ;
		}
	}


	prim = work->prim_line ;
	prim->flag &= ~DG_PRIM2_INVISIBLE ;

	/* プリミティブ頂点データのダブルバッファ切り替え */
	DG_SwitchBuffPrim2( prim );

	/* 書き込み用頂点データアドレス取得 */
	pos = prim->pos[ prim->buffer_clock ] ;
	mat = work->mats ;
	for ( i  = 0 ; i < TOTAL_OBJS ; i++ ){
		GTE_LoadMatrix( mat );
		GTE_PutVector( pos, line_base_pos, N_HEX_LINE_VERTS );
		pos += N_HEX_LINE_VERTS ;
		mat++ ;
	}

	uvrgb = prim->uvrgb[ prim->buffer_clock ] ;
	{
		int		alpha ;
		alpha = DG_FTOI( ( 1.0f - (float)work->count / ALIVE_FRAME ) * 48 ) ;
		if ( alpha < 0 ) alpha = 0 ;
		for ( i  = 0 ; i < TOTAL_OBJS ; i++ ){
			uvrgb[0].a = alpha ;
			uvrgb[1].a = alpha ;
			uvrgb[2].a = alpha ;
			uvrgb[3].a = alpha ;
			uvrgb[4].a = alpha ;
			uvrgb[5].a = alpha ;
			uvrgb[6].a = alpha ;
			uvrgb += 7 ;
		}
	}

	

	work->count++ ;
	if ( work->count > ALIVE_FRAME ) GV_DestroyActor( work );
}
/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	if ( work->prim_poly != NULL ){
		DG_DequeuePrim2( work->prim_poly );
		DG_FreePrim2( work->prim_poly );
	}
	if ( work->prim_line != NULL ){
		DG_DequeuePrim2( work->prim_line );
		DG_FreePrim2( work->prim_line );
	}
}
/* ---------------------------------------------------------------- */
static int GetResources( Work *work, FMATRIX *parent, float scale )
{
	DG_PRIM2	*prim ;
	//ScrpadWork	*scrpad = SCRPAD_ADDR ;

	work->map = GM_CurrentMap ;
	work->world = *parent ;
	work->scale = scale ;

	/* テクスチャの取得 */
	//tex = DG_GetTexture( TEXTURE );
	//if ( tex == NULL ) printf("%s:no texture(%s)\n", __FILE__, TEXTURE );

	/* プリミティブの確保 */
	work->prim_poly = prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS_POLY );
	if ( prim == NULL ) return ( -1 );
	//DG_ConfigPrim2Tex( prim, tex );
	prim->flag |= DG_PRIM2_INVISIBLE ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) );
   prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
	prim->as_world = work->world ;

	work->prim_line = prim = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS_LINE );
	if ( prim == NULL ) return ( -1 );
	//DG_ConfigPrim2Tex( prim, tex );
	prim->flag |= DG_PRIM2_INVISIBLE ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) );
   prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
	prim->as_world = work->world ;

	/* アニメーション用パラメータ初期化 */
	InitWork( work );
	/* ポリゴン用プリミティブ初期化 */
	InitPolyPrims( work );
	/* ライン用プリミティブ初期化 */
	InitLinePrims( work );

	return ( 0 );
}
/* ---------------------------------------------------------------- */
void *NewVRFloorBreak( FMATRIX *parent, float scale )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, parent, scale ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


/* ---------------------------------------------------------------- */
#define W_A	(1.0f)
#define W_B (0.5f)
#define W_C	(0.8660254f)

void *NewVRFloorBreakWide( FMATRIX *parent, float scale )
{
	static FVECTOR	wide_offset = {W_A*4+W_B,0,W_C,1.0f};
	FMATRIX		mat, rot ;
	FVECTOR		offset ;
	int			i ;
	void		*dummy_work  ;

	mat = *parent ;
	dummy_work = NewVRFloorBreak( &mat, scale );
	scale *= 0.8f ;
	for ( i = 0 ; i < 6 ; i++ ){
		GTE_MakeRotate( &rot, 0, 1, 0, GTE_PS2RAD( i*4096/6 ) );
		GTE_ApplyMatrix( &offset, &rot, &wide_offset );
		GTE_ScaleVector( &offset, &offset, PRIM_SCALE );
		GTE_AddVector( (FVECTOR*)mat.m[3], (FVECTOR*)parent->m[3], &offset );
		mat.m[3][3] = 1.0f ;
		NewVRFloorBreak( &mat, scale );
	}

	return ( dummy_work );
}
