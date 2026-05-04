/*
	tst_prm2.c
	新プリミティブ表示実験プログラム

	1999/12/24 K.Takabe
	$Id: tst_prm2.c,v 1.1.1.3 2002/11/19 11:51:32 Yoshizawa1 Exp $

*/

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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"


#include	"utl_dma.h"

/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define _RND(n)	( ( (rand()>>16) * n ) >> 15 )

/* ---------------------------------------------------------------- */
	/*
		プログラム使用定数の定義
	*/

/* ポリゴン＆ラインの最大頂点数は６４まで！！ */
#define N_PRIMS1	(5)
#define N_VERTS1	(4)

/* ポリゴン＆ラインの最大頂点数は６４まで！！ */
#define N_PRIMS2	(21)
#define N_VERTS2	(16)

/* 通常スプライトの最大頂点数は３２まで！！ */
/* 回転スプライトの最大頂点数は１６まで！！ */
#define USE_ROTATE
//#define N_PRIMS3	(5)
//#define N_VERTS3	(4)
#define N_PRIMS3	(2)
#define N_VERTS3	(1)


#define MEM_ADDR1	((void*)( SCRPAD_ADDR + 0x0000 ))
#define MEM_ADDR2	((void*)( SCRPAD_ADDR + 0x2000 ))


/* ---------------------------------------------------------------- */
	/*
		プログラムワーク
	*/
typedef	struct	{
	GV_ACT		actor ;

	DG_PRIM2	*prim1 ;
	DG_PRIM2	*prim2 ;
	DG_PRIM2	*prim3 ;

	int			map ;
	int			count ;
} Work ;

/* ---------------------------------------------------------------- */
	/*
		プログラム使用サブルーチン
	*/

/* スクラッチパッドからメインメモリへ転送 */
static void _MemCopy( void *dst, void *src, int size, int num )
{
	UTL_StartSprToMem( dst, src, size * num / sizeof(u_long128) );
	UTL_EndSprToMem() ;
}
/* メインメモリからスクラッチパッドへ転送 */
static void _MemCopy2( void *dst, void *src, int size, int num )
{
	UTL_StartMemToSpr( dst, src, size * num / sizeof(u_long128) );
	UTL_EndMemToSpr() ;
}

/* ---------------------------------------------------------------- */
	/*
		プログラムメイン処理
	*/
static void Act( Work *work )
{
#ifdef USE_ROTATE
	{
		DG_PRIM2_UVRGBWH	*uvrgbwh ;
		float				angle ;
		int					i, k ;

		/*
			DG_PRIM2のアニメーション手順
		*/
		/* 頂点やＵＶをアニメーションさせる場合には */
		/* DG_SwitchBufferPirm2()を呼んだ上で、prim->buffer_clockメンバの */
		/* 差すバッファに対してデータ更新を行なう。 */
		DG_SwitchBuffPrim2( work->prim3 );
		uvrgbwh = work->prim3->uvrgb[ work->prim3->buffer_clock ] ;
		for ( i = 0 ; i < N_PRIMS3 ; i++ ){
			for ( k = 0 ; k < N_VERTS3 ; k++ ){
				angle = (float)M_PI * ( ( ( i*13+k*20 + work->count ) % 360 ) - 180 ) / 180.0f ;
				uvrgbwh[k].w = 1000*cosf( angle ) ;/* L/2*cos(ang) */
				uvrgbwh[k].h = 1000*sinf( angle ) ;/* L/2*sin(ang) */
			}
			uvrgbwh += N_VERTS3 ;
		}

		work->count++ ;
	}
#endif

	{
		FVECTOR	*dst, *src, *pos ;
		int		i ;
		/*
			DG_PRIM2のアニメーション手順
		*/
		/* 頂点やＵＶをアニメーションさせる場合には */
		/* DG_SwitchBufferPrim2()を呼んだ上で、prim->buffer_clockメンバの */
		/* 差すバッファに対してデータ更新を行なう。 */
		DG_SwitchBuffPrim2( work->prim2 );
		dst = work->prim2->pos[ work->prim2->buffer_clock ] ;
		src = work->prim2->pos[ 1 - work->prim2->buffer_clock ] ;
		pos = MEM_ADDR1 ;
		for ( i = 0 ; i < N_PRIMS2 ; i++ ){
			_MemCopy2( pos+1,  src, sizeof(FVECTOR), N_VERTS2-1 ) ;
			pos[0].vx = GM_PlayerBody->objs->objs[i].world.m[3][0] ;
			pos[0].vy = GM_PlayerBody->objs->objs[i].world.m[3][1] ;
			pos[0].vz = GM_PlayerBody->objs->objs[i].world.m[3][2] ;
			_MemCopy( dst,  MEM_ADDR1, sizeof(FVECTOR), N_VERTS2 ) ;
			dst += N_VERTS2 ;
			src += N_VERTS2 ;
		}

	}

}

/* ---------------------------------------------------------------- */
	/*
		プログラム終了処理
	*/
static void Die( Work *work )
{
	GM_FreePrim2( work->prim1 );
	GM_FreePrim2( work->prim2 );
	GM_FreePrim2( work->prim3 );
}

/* ---------------------------------------------------------------- */
	/*
		ワーク初期化処理
	*/
static int GetResources( Work *work, int name, int where )
{
	DG_TEX		*tex ;
	DG_PRIM2	*prim ;
	DG_PRIM2_UVRGB	*uvrgb ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	FVECTOR			*pos ;
	int		i, k ;

	work->map = where ;
	GM_CurrentMap = where ;

	/* テクスチャ取得 */
	tex = DG_GetTexture( GV_StrCode("bomb1_fl") );

	/*
		ポリゴンプリミティブ使用実験
	*/
	/* ４角形ポリゴン実験 *//* ポリゴン＆ラインの最大頂点数は６４まで！！ */
	//prim = work->prim1 = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS1, N_VERTS1 );
	prim = work->prim1 = GM_MakePrim2( DG_PRIM2_CULLPOLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS1, N_VERTS1 );
	DG_ConfigPrim2Tex( prim, tex );

	/* スクラッチパッド上で初期化したデータを実際のワークにコピーする */
	pos = MEM_ADDR1 ;
	uvrgb = MEM_ADDR2 ;
	for ( i = 0 ; i < N_PRIMS1 ; i++ ){
		pos[0].vx = -1000.0F + i * 5000.0F ;
		pos[0].vy = 3000.0F ;
		pos[0].vz = 1000.0F + i * 5000.0F ;
		pos[1].vx = 1000.0F + i * 5000.0F ;
		pos[1].vy = 3000.0F ;
		pos[1].vz = 1000.0F + i * 5000.0F ;
		pos[2].vx = -1000.0F + i * 5000.0F ;
		pos[2].vy = -1000.0F ;
		pos[2].vz = 1000.0F + i * 5000.0F ;
		pos[3].vx = 1000.0F + i * 5000.0F ;
		pos[3].vy = -1000.0F ;
		pos[3].vz = 1000.0F + i * 5000.0F ;

		/* ＵＶ値は1.3.12の固定小数点なので注意！ */
		uvrgb[0].u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb[0].v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb[0].q = 4096 ;
		uvrgb[0].f = 0x8fff ;
		uvrgb[1].u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb[1].v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb[1].q = 4096 ;
		uvrgb[1].f = 0x8fff ;
		uvrgb[2].u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb[2].v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb[2].q = 4096 ;
		//uvrgb[2].f = 0x0fff ;/* 両面表示 */
		uvrgb[2].f = 0x0000 ;/* 左回り表示 */
		uvrgb[3].u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb[3].v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb[3].q = 4096 ;
		//uvrgb[3].f = 0x0fff ;/* 両面表示 */
		uvrgb[3].f = 0x0020 ;/* 右回り表示 */

		for ( k = 0 ; k < 4 ; k++ ){
			uvrgb[k].r = 128 ;
			uvrgb[k].g = 128 ;
			uvrgb[k].b = 128 ;
			uvrgb[k].a = 128 ;
		}

		pos += N_VERTS1 ;
		uvrgb += N_VERTS1 ;
	}
	_MemCopy( prim->pos[ 0 ],  MEM_ADDR1, sizeof(FVECTOR), N_VERTS1 * N_PRIMS1 ) ;
	_MemCopy( prim->uvrgb[ 0 ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_VERTS1 * N_PRIMS1 ) ;
	_MemCopy( prim->pos[ 1 ],  MEM_ADDR1, sizeof(FVECTOR), N_VERTS1 * N_PRIMS1 ) ;
	_MemCopy( prim->uvrgb[ 1 ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_VERTS1 * N_PRIMS1 ) ;


	/*
		ラインプリミティブ使用実験
	*/
	/* ４点接続ライン実験 *//* ポリゴン＆ラインの最大頂点数は６４まで！！ */
	prim = work->prim2 = 
	  GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_ANTIALIASING, N_PRIMS2, N_VERTS2 );
	DG_ConfigPrim2Tex( prim, tex );
	//prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
	prim->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;

	/* スクラッチパッド上で初期化したデータを実際のワークにコピーする */
	pos = MEM_ADDR1 ;
	uvrgb = MEM_ADDR2 ;
	for ( i = 0 ; i < N_PRIMS2 ; i++ ){
		for ( k = 0 ; k < N_VERTS2 ; k++ ){
			pos[k].vx = (float)_RND(3000) ;
			pos[k].vy = (float)_RND(3000) + 5000.0f ;
			pos[k].vz = (float)_RND(3000) + i * 5000.0f ;
			/* ＵＶ値は1.3.12の固定小数点なので注意！ */
			if ( k & 1 ){
				uvrgb[k].u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb[k].v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			} else {
				uvrgb[k].u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb[k].v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			}
			uvrgb[k].q = 4096 ;
			if ( k == 0 ){
				uvrgb[k].f = 0x8fff ;
			} else {
				uvrgb[k].f = 0x0fff ;
			}
			//uvrgb[k].r = _RND(128) ;
			//uvrgb[k].g = _RND(128) ;
			//uvrgb[k].b = _RND(128) ;
			uvrgb[k].r = ( ( i & 1 ) != 0 ) * 128 ;
			uvrgb[k].g = ( ( i & 2 ) != 0 ) * 128 ;
			uvrgb[k].b = ( ( i & 4 ) != 0 ) * 128 ;
			uvrgb[k].a = 128 ;/* アンチエイリアスを使用するときには１２８にする */
		}
		pos += N_VERTS2 ;
		uvrgb += N_VERTS2 ;
	}
	_MemCopy( prim->pos[ 0 ],  MEM_ADDR1, sizeof(FVECTOR), N_VERTS2 * N_PRIMS2 ) ;
	_MemCopy( prim->uvrgb[ 0 ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_VERTS2 * N_PRIMS2 ) ;
	_MemCopy( prim->pos[ 1 ],  MEM_ADDR1, sizeof(FVECTOR), N_VERTS2 * N_PRIMS2 ) ;
	_MemCopy( prim->uvrgb[ 1 ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_VERTS2 * N_PRIMS2 ) ;



	/*
		スプライトプリミティブ使用実験
	*/
#ifndef USE_ROTATE
	/* スプライト実験 *//* スプライトの最大頂点数は３２まで！！ */
	prim = work->prim3 = 
	  GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_FOG|DG_PRIM2_ALPHA, N_PRIMS3, N_VERTS3 );
#else
	/* スプライト実験 *//* 回転スプライトの最大頂点数は１６まで！！ */
	prim = work->prim3 = 
//	  GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_FOG|DG_PRIM2_ALPHA, N_PRIMS3, N_VERTS3 );
	  GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_FOG|DG_PRIM2_ALPHA, N_PRIMS3, N_VERTS3 );
#endif
	DG_ConfigPrim2Tex( prim, tex );
	//prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
	prim->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;

	/* スクラッチパッド上で初期化したデータを実際のワークにコピーする */
	pos = MEM_ADDR1 ;
	uvrgbwh = MEM_ADDR2 ;
	for ( i = 0 ; i < N_PRIMS3 ; i++ ){
		for ( k = 0 ; k < N_VERTS3 ; k++ ){
			pos[k].vx = 0.0f ;
			pos[k].vy = 0.0f - k * 2000.0f ;
			pos[k].vz = 0.0f + i * 2000.0f ;
			/* ＵＶ値は1.3.12の固定小数点なので注意！ */
			uvrgbwh[k].u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh[k].v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh[k].u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh[k].v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh[k].q0 = 4096 ;
			uvrgbwh[k].q1 = 4096 ;
			uvrgbwh[k].f0 = 0x0fff ;
			uvrgbwh[k].f1 = 0x0fff ;
			/* ＷＨ値は整数なので注意！ */
#ifndef USE_ROTATE
			uvrgbwh[k].w = 600 ;/* 幅／２ */
			uvrgbwh[k].h = 1000 ;/* 高さ／２ */
#else
			/* 回転スプライトでは幅と高さで指定するのではなく予め */
			/* 辺の１／２にそれぞれcos()とsin()を乗算した定数を用いる */
			/* （正方形のみサポート） */
			uvrgbwh[k].w = 1000*cosf( (float)M_PI*(i*13+k*20)/180.0f ) ;/* L/2*cos(ang) */
			uvrgbwh[k].h = 1000*sinf( (float)M_PI*(i*13+k*20)/180.0f ) ;/* L/2*sin(ang) */
#endif
			uvrgbwh[k].r = 128 ;
			uvrgbwh[k].g = 128 ;
			uvrgbwh[k].b = 128 ;
			uvrgbwh[k].a = 128 ;
		}
		pos += N_VERTS3 ;
		uvrgbwh += N_VERTS3 ;
	}
	_MemCopy( prim->pos[ 0 ],  MEM_ADDR1, sizeof(FVECTOR), N_VERTS3 * N_PRIMS3 ) ;
	_MemCopy( prim->uvrgb[ 0 ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGBWH), N_VERTS3 * N_PRIMS3 ) ;
	_MemCopy( prim->pos[ 1 ],  MEM_ADDR1, sizeof(FVECTOR), N_VERTS3 * N_PRIMS3 ) ;
	_MemCopy( prim->uvrgb[ 1 ], MEM_ADDR2, sizeof(DG_PRIM2_UVRGBWH), N_VERTS3 * N_PRIMS3 ) ;

	return (0);
}


/* ---------------------------------------------------------------- */
	/*
		シナリオ起動キャラ定義
	*/
void *NewPrim2TestProgram( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
