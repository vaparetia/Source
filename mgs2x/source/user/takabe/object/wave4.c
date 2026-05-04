//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	wave4.c
	新波オブジェクト

	2001/01/18 K.Takabe
	$Id: wave4.c,v 1.1.1.3 2002/11/19 11:51:17 Yoshizawa1 Exp $

*/
/*

chara キラキラ水面２[NewReflectionWaterSurface2Set] $s:name \
	-tex $s:ＴＲＩファイル名 $s:テクスチャ名 \
	-pos $v:中心座標 \
	-force $w:波影響倍率（デフォルト＝１０） \
	-ctrl \	// これをつけると水面コントロールキャラによる水面高さの変更が可能に
	-alpha_blend \	// 強制５０％半透明
	-invisible $v:非表示バウンディング最小値 $v:非表示バウンディング最大値
mesg キラキラ水面２ $s:名前 波発生源追加[0] $s:キャラ名
mesg キラキラ水面２ $s:名前 波発生源削除[1] $s:キャラ名
mesg キラキラ水面２ $s:名前 表示位置移動[256] $v:座標
mesg キラキラ水面２ $s:名前 波影響倍率[257] $w:倍率（デフォルト＝１０）
// 室内用水面オブジェクト
// テクスチャはこの波プログラム専用のテクスチャ単体でPACK_ALLしたTRIファイルが必要なので注意
// メッセージを送るとそのキャラの動きによって水面に波が立つ。

・ループ接続処理

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



/* ---------------------------------------------------------------- */
	/*
		外部参照
	*/
extern float GM_WaterLevel;
/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#ifdef PSX2
#define _RND(n)	( ( (BP_PS2_rand()>>16) * n ) >> 15 )
#else
#define _RND(_n)		( (int)( ( (unsigned int)rand()*(_n) ) / ((unsigned int)RAND_MAX+1) ) )
#endif

/* ---------------------------------------------------------------- */
	/*
		プログラム使用定数の定義
	*/

#define	MESH_X	( 16 )				/* Ｘ頂点数（ 4 < MESH_X*MESH_Z < 256 ） */
#define MESH_Z	( 16 )				/* Ｚ頂点数（ 4 < MESH_X*MESH_Z < 256 ） */
#define	MESH_DIV_X	( MESH_X - 1 )	/* パッチの分割数（Ｘ軸方向） */
#define MESH_DIV_Z	( MESH_Z - 1 )	/* パッチの分割数（Ｚ軸方向） */
#define MESH_WIDTH_X	( 700 )	/* メッシュ間隔（Ｘ軸方向） */
#define MESH_WIDTH_Z	( 700 )	/* メッシュ間隔（Ｚ軸方向） */
#define PATCH_X	(3)					/* パッチ配置数 */
#define PATCH_Z	(3)					/* パッチ配置数 */

#define TRI_ID	(GV_StrCode("wave"))
#define TEX_ID	(GV_StrCode("wave06"))

/* ---------------------------------------------------------------- */
/* 波ブロック管理構造体 */
typedef ALIGN16_DECL(struct) _wave_perts{
	/* 表示管理用 */
	FVECTOR		offset ;
	DG_PATCH	*patch ;
	/* ブロックにおける波アニメーションパラメータ */
	float		height[ MESH_DIV_X * MESH_DIV_Z ] ;
	float		force[ MESH_DIV_X * MESH_DIV_Z ] ;
	/* メモリアクセス高速化用１次元データバッファ（高さのみ） */
	float		line_px[ MESH_DIV_X ];		/* x- */
	float		line_nx[ MESH_DIV_X ];		/* x+ */
	/* 隣接波ブロックへのポインタ */
	struct _wave_perts	*link_x[2] ;
	struct _wave_perts	*link_z[2] ;
	struct _wave_perts	*link_corner[4] ;
	/* 固定ビットフラグ */
} WaveParts ;

/* 加速度計算用スクラッチパッドワーク */
typedef struct __scrpad_calc_force{
	float	height[ MESH_DIV_Z + 2 ][ MESH_DIV_X + 2  ];
	float	force[ MESH_DIV_Z + 2 ][ MESH_DIV_X + 2 ] ;

	WaveParts	local_waveparts ;
} Scrpad_CaclForce ;

/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */

/* キャラクタ監視機構用構造体 */
typedef struct _keep_control{
	int			flag ;
	int			name ;
	CONTROL		*ctrl ;
	int			count ;
	/* 以下各プログラム毎の拡張 */
	FVECTOR		old_pos ;
} KEEP_CONTROL ;
#define MAX_KEEP		(8)

/* ---------------------------------------------------------------- */
	/*
		プログラムワーク
	*/
typedef	struct	{
	GV_ACT_EX	actor ;

	FMATRIX		light[2] ;
	FVECTOR		center ;
	int			name ;
	int			map ;
	int			tri_id ;
	int			tex_id ;
	DG_TEX		*tex ;
	int			mesh_x, mesh_z ;
	int			div_x, div_y ;
	int			mesh_size_x, mesh_size_z ;
	FVECTOR		invisible_bound[2]	;			/* 非表示領域指定 */
	int			invisible_enabled ;				/* 非表示領域指定許可フラグ */
	int			count ;
	int			water_ctrl_flag ;
	int			alpha_blend_flag ;				/* 強制半透明設定フラグ */

	WaveParts	w_parts[ PATCH_X * PATCH_Z ];
	float		ave_height ;
	float		drop_force ;

	/* キャラクタ監視用 */
	KEEP_CONTROL	keep_list[MAX_KEEP] ;

} Work ;

/* ---------------------------------------------------------------- */
/* 波ブロックの初期化 */
static void InitWaveParts( Work *work, WaveParts *wave )
{
	DG_PATCH		*patch ;
	DG_PATCH_VERT	*verts ;
	int			i, j ;

	/* オブジェクトの初期化 */
	wave->patch = patch = DG_MakePatchMesh( DG_PATCH_REFLECTPLANE, MESH_X, MESH_Z );
	if ( work->alpha_blend_flag ) patch->flag |= DG_PATCH_SEMITRANS ;
	patch->light = work->light ;
	GM_GroupObject( patch, work->map );
	DG_QueuePatchObjs( patch );
	DG_ConfigPatchLOD( patch, 10 );
	patch->tri_id = work->tri_id ;
	patch->tex = work->tex ;
	/* 各頂点パラメータなどの初期化 */
	/* 加速度初期化 */
	for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
		for ( j = 0 ; j < MESH_DIV_X ; j++ ){
			wave->force[ j + i * ( MESH_DIV_X ) ] = _RND(MESH_WIDTH_Z/32) - MESH_WIDTH_Z/64 ;
		}
	}
	/* スクラッチパッド上で一度構築してから転送 */
	verts = SCRPAD_ADDR ;
	for ( i = 0 ; i < MESH_Z ; i++ ){
		for ( j = 0 ; j < MESH_X ; j++ ){
			verts->pos.vx = j * MESH_WIDTH_X - ( ( MESH_X - 1 ) * MESH_WIDTH_X ) / 2 ;
			verts->pos.vz = i * MESH_WIDTH_Z - ( ( MESH_Z - 1 ) * MESH_WIDTH_Z ) / 2 ;
			verts->pos.vy = 0.0f ;
			verts->pos.vw = 1.0f ;
			verts->pos_ds.vx = MESH_WIDTH_X ;
			verts->pos_ds.vz = 0.0f ;
			verts->pos_ds.vy = 0.0f ;
			verts->pos_ds.vw = 0.0f ;
			verts->pos_dt.vx = 0.0f ;
			verts->pos_dt.vz = MESH_WIDTH_Z ;
			verts->pos_dt.vy = 0.0f ;
			verts->pos_dt.vw = 0.0f ;
			verts->uv.vx = j / 4.0f ;	/* テクスチャは使用しない */
			verts->uv.vy = i / 4.0f ;	/* テクスチャは使用しない */
			verts->uv.vz = 1.0f ;		/* テクスチャは使用しない */
			verts->uv.vw = 128.0f ;		/* テクスチャは使用しない */
			verts++ ;
		}
	}
	verts = SCRPAD_ADDR ;
	FlushCache( 0 );
	DG_StartSprToMem( patch->verts[ 0 ], verts, MESH_X * MESH_Z * 4 );
	DG_EndSprToMem();
	DG_StartSprToMem( patch->verts[ 1 ], verts, MESH_X * MESH_Z * 4 );
	DG_EndSprToMem();

	DG_SetupPatchMeshBounding( patch );

}
/* 波ブロックの開放 */
static void FreeWaveParts( WaveParts *wave )
{
	DG_DequeuePatchObjs( wave->patch );
	DG_FreePatch( wave->patch );
}
/* 波ブロックアニメーション処理(加速度更新) */
static float CalcWaveForce( WaveParts *org_wave )
{
	Scrpad_CaclForce	*scrpad ;
	WaveParts			*link, *wave ;
	int					i, j, x, z ;
	float				*height, *src_height, *force, ave_height ;
	DG_PATCH_VERT		*verts ;

	scrpad= SCRPAD_ADDR ;
	wave = org_wave ;
	//wave = &scrpad->local_waveparts ;
	//FlushCache( 0 );
	//DG_StartMemToSpr( &scrpad->local_waveparts, org_wave, SIZEOF_QWORD(WaveParts) );
	//DG_EndMemToSpr();

	{/* 高さの情報をワークにコピー */
		src_height = &wave->height[ 0 + 0 * MESH_DIV_X ] ;
		height = (float*)scrpad->height + 1 + 1 * ( MESH_DIV_X + 2 ) ;
		for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
			GV_PREFECH( src_height + 16 );
			for ( j = 0 ; j < MESH_DIV_X ; j++ ){
				*height++ = *src_height++ ;
			}
			height += 2 ;
		}
	}

	/* 隣接部分の情報を取得する */
	/* X- */
	height = &scrpad->height[ 1 ][ 0 ] ;
	if ( ( link = wave->link_x[0] ) != NULL ){
		/* リンクするブロックがある場合 */
		src_height = link->line_nx ;
		for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
			*height = *src_height++ ;
			height += ( MESH_DIV_X + 2 ) ;
		}
	} else {
		/* リンクするブロックがない場合 */
		for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
			height[0] = height[1] ;
			height += ( MESH_DIV_X + 2 ) ;
		}
	}
	/* X+ */
	height = &scrpad->height[ 1 ][ MESH_DIV_X + 1 ] ;
	if ( ( link = wave->link_x[1] ) != NULL ){
		/* リンクするブロックがある場合 */
		src_height = link->line_px ;
		for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
			*height = *src_height++ ;
			height += ( MESH_DIV_X + 2 ) ;
		}
	} else {
		/* リンクするブロックがない場合 */
		for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
			height[0] = height[-1] ;
			height += ( MESH_DIV_X + 2 ) ;
		}
	}
	/* Z- */
	height = &scrpad->height[ 0 ][ 1 ] ;
	if ( ( link = wave->link_z[0] ) != NULL ){
		/* リンクするブロックがある場合 */
		src_height = &link->height[ 0 + ( MESH_DIV_Z - 1 ) * MESH_DIV_X ] ;
	} else {
		/* リンクするブロックがない場合 */
		src_height = height + ( MESH_DIV_X + 2 ) ;
	}
	for ( i = 0 ; i < MESH_DIV_X ; i++ ) *height++ = *src_height++ ;
	/* Z+ */
	height = &scrpad->height[ MESH_DIV_Z + 1 ][ 1 ] ;
	if ( ( link = wave->link_z[1] ) != NULL ){
		/* リンクするブロックがある場合 */
		src_height = &link->height[ 0 + ( 0 ) * MESH_DIV_X ] ;
	} else {
		/* リンクするブロックがない場合 */
		src_height = height - ( MESH_DIV_X + 2 );
	}
	for ( i = 0 ; i < MESH_DIV_X ; i++ ) *height++ = *src_height++ ;
	/* X-Z- */
	if ( ( link = wave->link_corner[0] ) != NULL ){
		scrpad->height[ 0 ][ 0 ] = link->height[ ( MESH_DIV_X - 1 ) + ( MESH_DIV_Z - 1 ) * MESH_DIV_X ] ;
	} else {
		scrpad->height[ 0 ][ 0 ] = scrpad->height[ 1 ][ 1 ] ;
	}
	/* X+Z- */
	if ( ( link = wave->link_corner[1] ) != NULL ){
		scrpad->height[ 0 ][ MESH_DIV_X + 1 ] = link->height[ ( 0 ) + ( MESH_DIV_Z - 1 ) * MESH_DIV_X ] ;
	} else {
		scrpad->height[ 0 ][ MESH_DIV_X + 1 ] = scrpad->height[ 1 ][ MESH_DIV_X ] ;
	}
	/* X-Z+ */
	if ( ( link = wave->link_corner[2] ) != NULL ){
		scrpad->height[ MESH_DIV_Z + 1 ][ 0 ] = link->height[ ( MESH_DIV_X - 1 ) + ( 0 ) * MESH_DIV_X ] ;
	} else {
		scrpad->height[ MESH_DIV_Z + 1 ][ 0 ] = scrpad->height[ MESH_DIV_Z ][ 1 ] ;
	}
	/* X+Z+ */
	if ( ( link = wave->link_corner[3] ) != NULL ){
		scrpad->height[ MESH_DIV_Z + 1 ][ MESH_DIV_X + 1 ] = link->height[ ( 0 ) + ( 0 ) * MESH_DIV_X ] ;
	} else {
		scrpad->height[ MESH_DIV_Z + 1 ][ MESH_DIV_X + 1 ] = scrpad->height[ MESH_DIV_Z ][ MESH_DIV_X ] ;
	}

	{/* 加速度の情報をワークにコピー */
		float	*src_force ;
		force = (float*)scrpad->force + 1 + 1 * ( MESH_DIV_X + 2 ) ;
		src_force = &wave->force[ 0 + 0 * MESH_DIV_X ] ;
		for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
			GV_PREFECH( src_force + 16 );
			for ( j = 0 ; j < MESH_DIV_X ; j++ ){
				*force++ = *src_force++ ;
			}
			force += 2 ;
		}
	}

	//wave = org_wave ;

	/* バッファ切り替え */
	DG_SwitchBuffPatch( wave->patch );
	verts = wave->patch->verts[ wave->patch->buffer_clock ];

	ave_height = 0 ;
	{/* 加速度の計算 */
		int	px, nx, pz, nz ;

		height = (float*)scrpad->height ;
		force = (float*)scrpad->force ;
		pz = 0 ;						/* １つ前の頂点オフセット */
		z = 1 * ( MESH_DIV_X + 2 ) ;	/* 処理中の頂点オフセット */
		nz = 2 * ( MESH_DIV_X + 2 ) ;	/* １つ次の頂点オフセット */
		for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
			px = 0 ;		/* １つ前の頂点オフセット */
			x = 1 ;			/* 処理中の頂点オフセット */
			nx = 2 ;		/* １つ次の頂点オフセット */
			for ( j = 0 ; j < MESH_DIV_X ; j++ ){
				float		h, f ;

				/* 隣接する高さからの影響を計算する */
#if !defined( PSX2 )
				{
					float	h1, h2 ;
					h1 = ( height[ x + nz ] + height[ x + pz ] +
						  height[ nx + z ] + height[ px + z ] ) * ( 2.0f/12.0f ) ;
					h2 = ( height[ nx + nz ] + height[ nx + pz ] +
						  height[ px + nz ] + height[ px + pz ] ) * ( 1.0f/12.0f ) ;
					h = h1 + h2 ;
				}
#else
				DG_MULA( 2.0f/12.0f, height[ x + nz ] );		/* 垂直隣接成分 */
				DG_MADDA( 2.0f/12.0f, height[ x + pz ] );		/* 垂直隣接成分 */
				DG_MADDA( 2.0f/12.0f, height[ nx + z ] );		/* 垂直隣接成分 */
				DG_MADDA( 2.0f/12.0f, height[ px + z ] );		/* 垂直隣接成分 */
				DG_MADDA( 1.0f/12.0f, height[ nx + nz ] );		/* 斜め隣接成分 */
				DG_MADDA( 1.0f/12.0f, height[ nx + pz ] );		/* 斜め隣接成分 */
				DG_MADDA( 1.0f/12.0f, height[ px + nz ] );		/* 斜め隣接成分 */
				h = DG_MADD( 1.0f/12.0f, height[ px + pz ] );	/* 斜め隣接成分 */
#endif
				f = h - height[ x + z ] ;
				f = force[ x + z ] + f * 0.20f ;
				if ( f > 3.0f ) f *= 0.80f ;
				force[ x + z ] = f ;

				/* 波の傾き計算 */
				verts->pos.vy = height[ x + z ];
				verts->pos_ds.vy = ( height[ nx + z ] - height[ px + z ] ) * 0.5f ;
				verts->pos_dt.vy = ( height[ x + nz ] - height[ x + pz ] ) * 0.5f ;
				verts++ ;

				/* 平均値取得用計算 */
				ave_height += height[ x + z ] ;

				/* 頂点オフセットインクリメント */
				px++ ;
				x++ ;
				nx++ ;
			}
			verts++ ;	/* スキップ */
			/* 頂点オフセットインクリメント */
			pz += ( MESH_DIV_X + 2 ) ;
			z += ( MESH_DIV_X + 2 ) ;
			nz += ( MESH_DIV_X + 2 ) ;
		}
	}
	ave_height *= 1.0f / ( MESH_DIV_X * MESH_DIV_Z ) ;

	{/* 加速度の情報をメモリに書き戻す */
		float	*src_force ;
		force = (float*)scrpad->force + 1 + 1 * ( MESH_DIV_X + 2 ) ;
		src_force = &wave->force[ 0 + 0 * MESH_DIV_X ] ;
		for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
			GV_PREFECH( src_force + 16 );
			for ( j = 0 ; j < MESH_DIV_X ; j++ ){
				*src_force++ = *force++ ;	/* 表記が逆なので注意 */
			}
			force += 2;
		}
	}

	//FlushCache( 0 );
	//DG_StartSprToMem( org_wave, &scrpad->local_waveparts, SIZEOF_QWORD(WaveParts) );
	//DG_EndSprToMem();

	return ( ave_height );

}
/* 波ブロックアニメーション処理（高さ更新） */
static void CalcWaveHeight( WaveParts *wave, float ave_height )
{
	Scrpad_CaclForce	*scrpad ;
	int					i, j ;
	float				*height, *force ;

	scrpad= SCRPAD_ADDR ;

	{/* 高さの情報をワークにコピー */
		float	*src_height ;
		height = (float*)scrpad->height + 1 + 1 * ( MESH_DIV_X + 2 ) ;
		src_height = &wave->height[ 0 + 0 * MESH_DIV_X ] ;
		for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
			GV_PREFECH( src_height + 16 );
			for ( j = 0 ; j < MESH_DIV_X ; j++ ){
				*height++ = *src_height++ ;
			}
			height += 2 ;
		}
	}

	{/* 加速度の情報をワークにコピー */
		float	*src_force ;
		force = (float*)scrpad->force + 1 + 1 * ( MESH_DIV_X + 2 ) ;
		src_force = &wave->force[ 0 + 0 * MESH_DIV_X ] ;
		for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
			for ( j = 0 ; j < MESH_DIV_X ; j++ ){
				*force++ = *src_force++ ;
			}
			force += 2 ;
		}
	}

	{/* 高さの更新 */
		force = (float*)scrpad->force + 1 + 1 * ( MESH_DIV_X + 2 ) ;
		height = (float*)scrpad->height + 1 + 1 * ( MESH_DIV_X + 2 ) ;
		for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
			for ( j = 0 ; j < MESH_DIV_X ; j++ ){
				*height += *force - ave_height ;
				force++ ;
				height++ ;
			}
			force += 2 ;
			height += 2 ;
		}
	}

	{/* 高さの情報をメモリに書き戻す */
		float	*src_height ;
		height = (float*)scrpad->height + 1 + 1 * ( MESH_DIV_X + 2 ) ;
		src_height = &wave->height[ 0 + 0 * MESH_DIV_X ] ;
		for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
			GV_PREFECH( src_height + 16 );
			for ( j = 0 ; j < MESH_DIV_X ; j++ ){
				*src_height++ = *height++ ;	/* 表記が逆なので注意 */
			}
			height += 2 ;
		}
		src_height = &scrpad->height[ 1 ][ 1 ] ;
		height = wave->line_px ;
		for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
			*height++ = *src_height ;
			src_height += ( MESH_DIV_X + 2 );
		}
		src_height = &scrpad->height[ 1 ][ MESH_DIV_X ] ;
		height = wave->line_nx ;
		for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
			*height++ = *src_height ;
			src_height += ( MESH_DIV_X + 2 );
		}
	}

}
/* 波を立てる */
static void DropWave( Work *work, FVECTOR *pos, float drop_force )
{
	int		i, j ;

	for ( i = 0 ; i < PATCH_Z ; i++ ){
		for ( j = 0 ; j < PATCH_X ; j++ ){
			WaveParts	*wave, *link ;
			float	max_x, max_z, min_x, min_z, x_rate, z_rate ;
			float	*pxpz_h, *nxpz_h, *pxnz_h, *nxnz_h ;
			float	*pxpz_f, *nxpz_f, *pxnz_f, *nxnz_f ;
			FVECTOR	v_pos ;
			int		x, z ;

			wave = &work->w_parts[ j + i * PATCH_X ] ;
			min_x = wave->offset.vx - (float)( MESH_X - 1 ) * 0.5f * MESH_WIDTH_X ;
			max_x = wave->offset.vx + (float)( MESH_X - 1 ) * 0.5f * MESH_WIDTH_X ;
			min_z = wave->offset.vz - (float)( MESH_Z - 1 ) * 0.5f * MESH_WIDTH_Z ;
			max_z = wave->offset.vz + (float)( MESH_Z - 1 ) * 0.5f * MESH_WIDTH_Z ;
			if ( pos->vx < max_x && pos->vx >= min_x && pos->vz < max_z && pos->vz >= min_z ){
				/* 基準座標インデックス＆座標算出 */
				x = DG_FTOI( pos->vx - min_x ) / MESH_WIDTH_X ;
				z = DG_FTOI( pos->vz - min_z ) / MESH_WIDTH_Z ;
				v_pos.vx = min_x + x * MESH_WIDTH_X ;
				v_pos.vy = 0.0f ;
				v_pos.vz = min_z + z * MESH_WIDTH_Z ;
				/* 位置の割合を求める */
				x_rate = ( pos->vx - v_pos.vx ) / MESH_WIDTH_X ;
				z_rate = ( pos->vz - v_pos.vz ) / MESH_WIDTH_Z ;
				//printf("area: %f,%f - %f %f\n", min_x, min_z, max_x, max_z );
				//printf("%f %f %f %f\n", pos->vx, pos->vz, v_pos.vx, v_pos.vz );
				//printf("%d %d %f %f \n", x, z, x_rate, z_rate );
				/* ４点のパラメータアドレス算出 */
				pxpz_h = &wave->height[ x + z * MESH_DIV_X ] ;
				pxpz_f = &wave->force[ x + z * MESH_DIV_X ] ;
				nxpz_h = pxnz_h = nxnz_h = pxpz_h ;
				nxpz_f = pxnz_f = nxnz_f = pxpz_f ;
				if ( (x+1) < MESH_DIV_X ){
					nxpz_h = &wave->height[ (x+1) + z * MESH_DIV_X ] ;
					nxpz_f = &wave->force[ (x+1) + z * MESH_DIV_X ] ;
				} else {
					if ( ( link = wave->link_x[1] ) != NULL ){
						nxpz_h = &link->height[ (0) + z * MESH_DIV_X ] ;
						nxpz_f = &link->force[ (0) + z * MESH_DIV_X ] ;
						}
				}
				if ( (z+1) < MESH_DIV_Z ){
					pxnz_h = &wave->height[ x + (z+1) * MESH_DIV_X ] ;
					pxnz_f = &wave->force[ x + (z+1) * MESH_DIV_X ] ;
				} else {
					if ( ( link = wave->link_z[1] ) != NULL ){
						pxnz_h = &link->height[ x + (0) * MESH_DIV_X ] ;
						pxnz_f = &link->force[ x + (0) * MESH_DIV_X ] ;
					}
				}
				if ( (x+1) < MESH_DIV_X && (z+1) < MESH_DIV_Z ){
					nxnz_h = &wave->height[ (x+1) + (z+1) * MESH_DIV_X ] ;
					nxnz_f = &wave->force[ (x+1) + (z+1) * MESH_DIV_X ] ;
				} else {
					x++ ;
					z++ ;
					if ( x == MESH_DIV_X && z == MESH_DIV_Z ){
						link = wave->link_corner[3] ;
						x = 0 ; z = 0 ;
					} else if ( x == MESH_DIV_X ){
						link = wave->link_x[1] ;
						x = 0 ;
					} else {
						link = wave->link_z[1] ;
						z = 0 ;
					}
					if ( link != NULL ){
						nxnz_h = &link->height[ (x) + (z) * MESH_DIV_X ] ;
						nxnz_f = &link->force[ (x) + (z) * MESH_DIV_X ] ;
					}
				}
				/* ４点に変化量を分配する */
				*pxpz_h -= drop_force * ( 1 - x_rate ) * ( 1 - z_rate );
				*nxpz_h -= drop_force * x_rate * ( 1 - z_rate ) ;
				*pxnz_h -= drop_force * ( 1 - x_rate ) *  z_rate ;
				*nxnz_h -= drop_force * x_rate * z_rate ;
				*pxpz_f = drop_force * ( 1 - x_rate ) * ( 1 - z_rate );
				*nxpz_f = drop_force * x_rate * ( 1 - z_rate ) ;
				*pxnz_f = drop_force * ( 1 - x_rate ) *  z_rate ;
				*nxnz_f = drop_force * x_rate * z_rate ;
			}
		}
	}
}
/* ---------------------------------------------------------------- */
/* 指定した名前のコントロールを検索 */
static CONTROL* SearchControl( int map, int name )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		if ( ( control->map & map ) == 0 ) continue ;
		return ( control ) ;
	}
	printf("%s: search faild control !!\n", __FILE__);
	return ( NULL );
}
/* 指定したコントロールを監視キャラとして見つける */
static void AddKeep( Work *work, int name )
{
	CONTROL	*ctrl ;
	KEEP_CONTROL	*keep ;
	int		i ;

	if ( ( ctrl = SearchControl( work->map, name ) ) == NULL ) return ;
	for ( i = 0 ; i < MAX_KEEP ; i++ ){
		keep = &work->keep_list[ i ] ;
		if ( keep->flag ) continue ;
		keep->flag = 1 ;
		keep->ctrl = ctrl ;
		keep->name = name ;
		keep->old_pos = ctrl->mov ;
		keep->count = _RND(64);
		return ;
	}
	if ( i == MAX_KEEP ){
		printf("%s:not enough keep work(%d)\n", __FILE__, keep->name );
		return ;
	}
}
/* 監視をやめる */
static void DelKeep( Work *work, int name )
{
	KEEP_CONTROL	*keep ;
	int		i ;

	for ( i = 0 ; i < MAX_KEEP ; i++ ){
		keep = &work->keep_list[ i ] ;
		if ( keep->flag == 0 ) continue ;
		keep->flag = 0 ;
		return ;
	}
}
/* 監視キャラが存在するがチェックする */
static void CheckKeep( Work *work )
{
	KEEP_CONTROL	*keep ;
	CONTROL	*ctrl ;
	int		i ;

	for ( i = 0 ; i < MAX_KEEP ; i++ ){
		keep = &work->keep_list[ i ] ;
		if ( keep->flag == 0 ) continue ;
		if ( ( ( ctrl = SearchControl( work->map, keep->name ) ) == NULL ) || ( ctrl != keep->ctrl ) ){
			keep->flag = 0 ;
			printf("%s:lost keep control(%d)\n", __FILE__, keep->name );
			continue ;
		}
	}
}
/* ---------------------------------------------------------------- */
	/*
		プログラム使用サブルーチン
	*/

/* テクスチャ情報をリージョンリピートテクスチャに設定する */
static void TextureRepeatSet( DG_TEX *tex )
{
	int		tw, th, w, h, tx, ty, ofx, ofy, tmp ;
	tw = ( tex->tex_trans.tex0.data >> 26 ) & 0x0f ;
	th = ( tex->tex_trans.tex0.data >> 30 ) & 0x0f ;
	w = 1 << tw ;
	h = 1 << th ;
	tmp = w * tex->u_scale ;
	tx = 1 ; while ( tx < tmp ) tx <<= 1 ;
	tmp = h * tex->v_scale ;
	ty = 1 ; while ( ty < tmp ) ty <<= 1 ;
	ofx = w * tex->u_offset ;
	ofx &= ~( tx - 1 );
	ofy = h * tex->v_offset ;
	ofy &= ~( ty - 1 );
	tex->tex_trans.clamp.data = SCE_GS_SET_CLAMP( 3, 3, tx-1, ofx, ty-1, ofy );
}
#if 0
/* テクスチャのスケーリング値を取得する */
static void GetTextureScale( DG_TEX *tex, FVECTOR *scale )
{
	int		tw, th, w, h, tx, ty, tmp ;
	tw = ( tex->tex_trans.tex0.data >> 26 ) & 0x0f ;
	th = ( tex->tex_trans.tex0.data >> 30 ) & 0x0f ;
	w = 1 << tw ;
	h = 1 << th ;
	tmp = w * tex->u_scale ;
	tx = 1 ; while ( tx < tmp ) tx <<= 1 ;
	tmp = h * tex->v_scale ;
	ty = 1 ; while ( ty < tmp ) ty <<= 1 ;
	scale->vx = 1.0f / ( w / tx ) ;
	scale->vy = 1.0f / ( h / ty ) ;
}
#endif



/* ---------------------------------------------------------------- */
	/*
		プログラムメイン処理
	*/
static void Act( Work *work )
{
	GV_MSG		*msg ;
	int			n_msg, i, j ;
	float		total_ave_height, ave_height ;

	//AN_Test_Eye2( &DG_ZeroVector, 3 );

	/* 消滅チェック */
	CheckKeep( work );
	/* メッセージチェック */
	if ( ( n_msg = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
		for ( ; n_msg > 0 ; n_msg--, msg++ ){
			switch ( msg->message[0] ){
			  case 0:/* 監視に追加 */
				AddKeep( work, msg->message[1] );
				break ;
			  case 1:/* 監視から削除 */
				DelKeep( work, msg->message[1] );
				break ;
			  case 256:/* 表示座標の移動 */
				work->center.vx = msg->message[1] ;
				work->center.vy = msg->message[2] ;
				work->center.vz = msg->message[3] ;
				break ;
			  case 257:
				work->drop_force = msg->message[1] ;
				break ;
			}
		}
	}
	for ( i = 0 ; i < MAX_KEEP ; i++ ){
		KEEP_CONTROL	*keep ;
		keep = &work->keep_list[ i ] ;
		if ( keep->flag == 0 ) continue ;

		{/* 固有処理 */
			FVECTOR	*pos, step ;
			float	len, h ;

			pos = &keep->ctrl->mov ;
			_sceVu0SubVector( &step, pos, &keep->old_pos );
			len = DG_SQRT( step.vx * step.vx + step.vy * step.vy + step.vz * step.vz );
			if ( len > 100.0f ) len = 100.0f ;
			h = pos->vy - work->center.vy ;
			if ( h < 1000.0f && h > -1000.0f ){
				keep->count = ( keep->count + 1 ) & 15 ;
				if ( keep->count == 0 ) DropWave( work, &keep->old_pos, len * work->drop_force );
			}
			keep->old_pos = *pos ;
		}
		
	}

	/* 表示位置変更 */
	if ( work->water_ctrl_flag ){
		work->center.vy = GM_WaterLevel ;
		for ( i = 0 ; i < PATCH_Z ; i++ ){
			for ( j = 0 ; j < PATCH_X ; j++ ){
				WaveParts	*wave ;
				wave = &work->w_parts[ j + i * PATCH_X ] ;
				wave->offset.vx = work->center.vx + (float)( MESH_WIDTH_X * MESH_DIV_X ) * ( j - PATCH_X * 0.5f + 0.5f ) ;
				wave->offset.vy = GM_WaterLevel ;	/* これは使ってないので意味なし */
				wave->offset.vz = work->center.vz + (float)( MESH_WIDTH_Z * MESH_DIV_Z ) * ( i - PATCH_Z * 0.5f + 0.5f ) ;
				wave->patch->world.m[3][0] = wave->offset.vx ;
				wave->patch->world.m[3][1] = wave->offset.vy ;
				wave->patch->world.m[3][2] = wave->offset.vz ;
			}
		}
	}

	total_ave_height = 0 ;
	for ( i = 0 ; i < PATCH_Z ; i++ ){
		for ( j = 0 ; j < PATCH_X ; j++ ){
			WaveParts	*wave ;
			wave = &work->w_parts[ j + i * PATCH_X ] ;
			CalcWaveHeight(  wave, work->ave_height );
		}
	}
	for ( i = 0 ; i < PATCH_Z ; i++ ){
		for ( j = 0 ; j < PATCH_X ; j++ ){
			WaveParts	*wave ;
			wave = &work->w_parts[ j + i * PATCH_X ] ;
			ave_height = CalcWaveForce( wave );
			total_ave_height += ave_height ;
		}
	}
	work->ave_height = ave_height / ( PATCH_X * PATCH_Z ) ;

	/* 隣接するオブジェクトにエッジの傾き情報をコピーする */
	for ( i = 0 ; i < PATCH_Z ; i++ ){
		for ( j = 0 ; j < PATCH_X ; j++ ){
			WaveParts	*wave, *link ;
			DG_PATCH_VERT	*dst_verts, *src_verts ;
			int			k ;
			wave = &work->w_parts[ j + i * PATCH_X ] ;
			if ( ( link = wave->link_x[0] ) != NULL ){
				src_verts = wave->patch->verts[ wave->patch->buffer_clock ];
				dst_verts = link->patch->verts[ link->patch->buffer_clock ];
				for ( k = 0 ; k < MESH_DIV_Z ; k++ ){
					dst_verts[ MESH_DIV_X + k * MESH_X ].pos.vy = src_verts[ 0 + k * MESH_X ].pos.vy ;
					dst_verts[ MESH_DIV_X + k * MESH_X ].pos_ds = src_verts[ 0 + k * MESH_X ].pos_ds ;
					dst_verts[ MESH_DIV_X + k * MESH_X ].pos_dt = src_verts[ 0 + k * MESH_X ].pos_dt ;
				}
			}
			if ( ( link = wave->link_z[0] ) != NULL ){
				src_verts = wave->patch->verts[ wave->patch->buffer_clock ];
				dst_verts = link->patch->verts[ link->patch->buffer_clock ];
				for ( k = 0 ; k < MESH_DIV_X ; k++ ){
					dst_verts[ k + MESH_DIV_Z * MESH_X ].pos.vy = src_verts[ k + 0 * MESH_X ].pos.vy ;
					dst_verts[ k + MESH_DIV_Z * MESH_X ].pos_ds = src_verts[ k + 0 * MESH_X ].pos_ds ;
					dst_verts[ k + MESH_DIV_Z * MESH_X ].pos_dt = src_verts[ k + 0 * MESH_X ].pos_dt ;
				}
			}
			if ( ( link = wave->link_corner[0] ) != NULL ){
				src_verts = wave->patch->verts[ wave->patch->buffer_clock ];
				dst_verts = link->patch->verts[ link->patch->buffer_clock ];
				dst_verts[ MESH_DIV_X + MESH_DIV_Z * MESH_X ].pos.vy = src_verts[ 0 + 0 * MESH_X ].pos.vy ;
				dst_verts[ MESH_DIV_X + MESH_DIV_Z * MESH_X ].pos_ds.vy = src_verts[ 0 + 0 * MESH_X ].pos_ds.vy ;
				dst_verts[ MESH_DIV_X + MESH_DIV_Z * MESH_X ].pos_dt.vy = src_verts[ 0 + 0 * MESH_X ].pos_dt.vy ;
			}
		}
	}

#if 0
	/* プレイヤー反応処理 */
	if ( GM_PlayerBody != NULL ){
		FVECTOR	*pos, step ;
		float	force, len, h ;
		static int	count ;
		static FVECTOR	old_pos ;

		pos = &GM_PlayerControl->mov ;
		_sceVu0SubVector( &step, pos, &old_pos );
		len = DG_SQRT( step.vx * step.vx + step.vy * step.vy + step.vz * step.vz );
		if ( len > 100.0f ) len = 100.0f ;
		h = pos->vy - work->center.vy ;
		if ( h < 1000.0f && h > -1000.0f ){
			if ( count == 0 ) DropWave( work, &old_pos, len * 10 );
			count = ( count + 1 ) & 15 ;
		}
		old_pos = *pos ;
	}
#endif

	
}

/* ---------------------------------------------------------------- */
	/*
		プログラム終了処理
	*/
static void Die( Work *work )
{
	int		i, j ;
	for ( i = 0 ; i < PATCH_Z ; i++ ){
		for ( j = 0 ; j < PATCH_X ; j++ ){
			WaveParts	*wave ;
			wave = &work->w_parts[ j + i * PATCH_X ] ;
			FreeWaveParts( wave );
		}
	}
}

/* ---------------------------------------------------------------- */
	/*
		ワーク初期化処理
	*/
static int GetResources( Work *work, int name, int where )
{
	DG_TEX		*tex ;
	int		i, j ;
	FVECTOR		pos ;

	work->name = name ;
	work->map = where ;
	GM_CurrentMap = where ;

	work->light[1].m[3][0] = 128 ;
	work->light[1].m[3][1] = 128 ;
	work->light[1].m[3][2] = 128 ;
	work->light[1].m[3][3] = 128 ;

	/* テクスチャ設定読み込み */
	if ( GCL_GetOption( 't' ) != NULL ) {
		work->tri_id = GCL_GetNextInt() ;
		work->tex_id = GCL_GetNextInt() ;
	} else {
		work->tri_id = TRI_ID ;
		work->tex_id = TEX_ID ;
	}

	/* 設置座標読み込み */
	if ( GCL_GetOption( 'p' ) != NULL ) {
		pos.vx = (float)GCL_GetNextInt() ;
		pos.vy = (float)GCL_GetNextInt() ;
		pos.vz = (float)GCL_GetNextInt() ;
	} else {
		pos = DG_ZeroVector ;
	}
	work->center = pos ;

	/* 波影響倍率設定読み込み */
	if ( GCL_GetOption( 'f' ) != NULL ) {
		work->drop_force = GCL_GetNextInt() ;
	} else {
		work->drop_force = 10.0f ;
	}

	if ( GCL_GetOption( 'c' ) != NULL ){
		/* 水面コントロールキャラによる水面位置の制御を許可する */
		work->water_ctrl_flag = 1 ;
	}

	/* 非表示領域設定 */
	if ( GCL_GetOption( 'i' ) != NULL ) {
		work->invisible_bound[0].vx = (float)GCL_GetNextInt() ;
		work->invisible_bound[0].vy = (float)GCL_GetNextInt() ;
		work->invisible_bound[0].vz = (float)GCL_GetNextInt() ;
		work->invisible_bound[1].vx = (float)GCL_GetNextInt() ;
		work->invisible_bound[1].vy = (float)GCL_GetNextInt() ;
		work->invisible_bound[1].vz = (float)GCL_GetNextInt() ;
		work->invisible_bound[0].vw = work->invisible_bound[1].vw = 1.0f ;
		work->invisible_enabled = 1 ;
	}

	/* 強制５０％半透明設定 */
	if ( GCL_GetOption( 'a' ) != NULL ){
		work->alpha_blend_flag = 1 ;
	}

	/* テクスチャ取得 */
	work->tex = tex = DG_GetTexture2( work->tri_id, work->tex_id );
	/* 強制的にリピートテクスチャにする */
	//tex->tex_trans.clamp.data = SCE_GS_SET_CLAMP( 3, 3, 0x7f, 0x00, 0x7f, 0x00 );
	TextureRepeatSet( tex );
	//tex->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 2, 1, 48 );


	for ( i = 0 ; i < PATCH_Z ; i++ ){
		for ( j = 0 ; j < PATCH_X ; j++ ){
			WaveParts	*wave ;
			wave = &work->w_parts[ j + i * PATCH_X ] ;
			InitWaveParts( work, wave );
			wave->offset.vx = pos.vx + (float)( MESH_WIDTH_X * MESH_DIV_X ) * ( j - PATCH_X * 0.5f + 0.5f ) ;
			wave->offset.vy = pos.vy ;
			wave->offset.vz = pos.vz + (float)( MESH_WIDTH_Z * MESH_DIV_Z ) * ( i - PATCH_Z * 0.5f + 0.5f ) ;
			wave->offset.vw = 1.0f ;
			wave->patch->world.m[3][0] = wave->offset.vx ;
			wave->patch->world.m[3][1] = wave->offset.vy ;
			wave->patch->world.m[3][2] = wave->offset.vz ;
			/* 隣接オブジェクトとのリンク処理 */
			if ( j > 0 )
			  wave->link_x[0] = &work->w_parts[ j - 1 + i * PATCH_X ] ;
			if ( j < (PATCH_X-1) )
			  wave->link_x[1] = &work->w_parts[ j + 1 + i * PATCH_X ] ;
			if ( i > 0 )
			  wave->link_z[0] = &work->w_parts[ j + ( i - 1 ) * PATCH_X ] ;
			if ( i < (PATCH_Z-1) )
			  wave->link_z[1] = &work->w_parts[ j + ( i + 1 ) * PATCH_X ] ;
			if ( j > 0 && i > 0 )
			  wave->link_corner[0] = &work->w_parts[ j - 1 + ( i - 1 ) * PATCH_X ] ;
			if ( j < (PATCH_X-1) && i > 0 )
			  wave->link_corner[1] = &work->w_parts[ j + 1 + ( i - 1 ) * PATCH_X ] ;
			if ( j > 0 && i < (PATCH_Z-1) )
			  wave->link_corner[2] = &work->w_parts[ j - 1 + ( i + 1 ) * PATCH_X ] ;
			if ( j < (PATCH_X-1) && i < (PATCH_Z-1) )
			  wave->link_corner[3] = &work->w_parts[ j + 1 + ( i + 1 ) * PATCH_X ] ;
			
		}
	}

	return (0);
}


/* ---------------------------------------------------------------- */
	/*
		シナリオ起動インターフェイス
	*/
void *NewReflectionWaterSurface2Set( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

#if 0
	/*
		プログラム起動インターフェイス
	*/
void *NewStormyRiverWave( int name, FMATRIX *world, int model )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, GM_CurrentMap, 1, world, model ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
#endif
