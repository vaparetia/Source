/*
	wave5_ex.c
	海面オブジェクト（ハリアー戦専用）

	2001/07/04 K.Takabe
	$Id: wave5_ex.c,v 1.1.1.3 2002/11/19 11:51:17 Yoshizawa1 Exp $

*/
/*

chara プラント海面ハリアー戦[NewSeaSurfaceHarrierSet] $s:name \
	-single_mode \	// シングルテクスチャモード
	-bumpmap_mode \	// バンプマップモード
	-light_enable \		// ステージライト反映
	-tex $s:ＴＲＩファイル名 $s:テクスチャ名 $s:テクスチャ名 \
	-pos $v:中心座標 \
	-wave $w:隣接影響スピード $w:減衰スピード $w:最小振幅力 \
	-details_wave $w:隣接影響スピード $w:減衰スピード $w:最小振幅力 \
	-invisible $v:非表示バウンディング最小値 $v:非表示バウンディング最大値 \
	-rotate $w:環境マップ基準の回転 \
	-alpha_blend \	// 強制５０％半透明
	-flag $w:フラグ
mesg プラント海面ハリアー戦 $s:名前 表示状態[2] $b:ＯＮ・ＯＦＦ
mesg プラント海面ハリアー戦 $s:名前 slepp[3]
mesg プラント海面ハリアー戦 $s:名前 wake[4]
mesg プラント海面ハリアー戦 $s:名前 表示位置移動[256] $v:座標
mesg プラント海面ハリアー戦 $s:名前 テクスチャ変更[257] $s:テクスチャ名 ...
// ※ハリアー戦専用のプラント海面です※
// テクスチャはこの波プログラム専用のテクスチャ単体でPACK_ALLしたTRIファイルが必要なので注意
// -waveオプションで指定する波アニメーションパラメータはデバッグモードで調整ができる
// -details_waveオプションは-waveより細かく指定可能(1~10000)
// テクスチャ変更は基本的にはデモ用です。
// 「sleep」メッセージを送ることで計算処理を止めることが出来ます。
// 「wake」メッセージか他のメッセージを送ることで目覚めます。

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



/* ---------------------------------------------------------------- */
	/*
		外部参照
	*/
extern int DM_FrameSkip ;
extern float GM_WaterLevel;
extern int	Vu1DrawReflectSurface2[] ;
extern void	*DG_CurveSurfaceVuCode[ 2 ] ;
/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#define _RND(n)	( ( (BP_PS2_rand()>>16) * n ) >> 15 )

/* コンパイラのレジスタ割り当て最適化を当てにしたＦＰＵ演算ルーチン */
#define INNER_PRODUCT(_v0,_v1)	({DG_MULA((_v0)->vx,(_v1)->vx);\
									DG_MADDA((_v0)->vy,(_v1)->vy);\
									DG_MADD((_v0)->vz,(_v1)->vz);})
#define OUTER_PRODUCT(_r,_v0,_v1)	{\
									   float	x, y, z ;\
									   DG_MULA( (_v0)->vy, (_v1)->vz );\
									   x = DG_MSUB( (_v0)->vz, (_v1)->vx );\
									   DG_MULA( (_v0)->vz, (_v1)->vx );\
									   y = DG_MSUB( (_v0)->vx, (_v1)->vz );\
									   DG_MULA( (_v0)->vx, (_v1)->vy );\
									   z = DG_MSUB( (_v0)->vy, (_v1)->vx );\
									   (_r)->vx = x ; (_r)->vy = y ; (_r)->vz = z ;\
								 }
#define GET_VECLEN(_v)	({\
							DG_MULA( (_v)->vx, (_v)->vx );\
							DG_MADDA( (_v)->vy, (_v)->vy );\
							DG_MADD( (_v)->vz, (_v)->vz );})

/* ---------------------------------------------------------------- */
	/*
		プログラム使用定数の定義
	*/

#define	MESH_X	( 16 )				/* Ｘ頂点数（ 4 < MESH_X*MESH_Z < 256 ） */
#define MESH_Z	( 16 )				/* Ｚ頂点数（ 4 < MESH_X*MESH_Z < 256 ） */
#define	MESH_DIV_X	( MESH_X - 1 )	/* パッチの分割数（Ｘ軸方向） */
#define MESH_DIV_Z	( MESH_Z - 1 )	/* パッチの分割数（Ｚ軸方向） */
#define MESH_WIDTH_X	( 8000*2 )	/* メッシュ間隔（Ｘ軸方向） */
#define MESH_WIDTH_Z	( 8000*2 )	/* メッシュ間隔（Ｚ軸方向） */
#define PATCH_X	(3)					/* パッチ配置数 */
#define PATCH_Z	(3)					/* パッチ配置数 */

#define MAX_PLANE	(3)

#define MAX_PARAM_INDEX	(64)

#define TRI_ID	(GV_StrCode("wave"))
#define TEX_ID	(GV_StrCode("wave06"))

/* ---------------------------------------------------------------- */
/* 波ブロック管理構造体 */
typedef ALIGN16_DECL(struct) _wave_perts{
	/* 表示管理用 */
	FVECTOR		offset ;
	DG_PATCH	*patch[MAX_PLANE] ;
	/* ブロックにおける波アニメーションパラメータ */
	float		height[ MESH_DIV_X * MESH_DIV_Z ] ;
	float		force[ MESH_DIV_X * MESH_DIV_Z ] ;
	//char		index[ MESH_DIV_X * MESH_DIV_Z ] ;
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

#ifdef DEBUG_MODE
//void GM_AddDebugMenu( GM_DEBUG_MENU *menu );
static int	Wave5_DebugFlag ;
static int	Wave5_Influence_Speed ;
static int	Wave5_Min_WaveForce ;
static int	Wave5_Lose_Speed ;
static GM_DEBUG_MENU debug_debugflag = {
  class:	"WAVE5",
  menu:		"DEBUG MODE",
  max:		2,
  items:	( char *[] ){ "ON", "OFF" },
  values:	( int [] ){ 1, 0 },
  target:	&Wave5_DebugFlag,
  type:		GM_DEBUG_MENU_FLAG,
};
static GM_DEBUG_MENU debug_influence_speed = {
  class:	"WAVE5",
  menu:		"INFLUENCE SPEED",
  max:		200,
  items:	NULL,
  values:	NULL,
  target:	&Wave5_Influence_Speed,
  type:		GM_DEBUG_MENU_FLAG|GM_DEBUG_MENU_DEC,
};
static GM_DEBUG_MENU debug_lose_speed = {
  class:	"WAVE5",
  menu:		"LOSE SPEED",
  max:		200,
  items:	NULL,
  values:	NULL,
  target:	&Wave5_Lose_Speed,
  type:		GM_DEBUG_MENU_FLAG|GM_DEBUG_MENU_DEC,
};
static GM_DEBUG_MENU debug_min_waveforce = {
  class:	"WAVE5",
  menu:		"MIN WAVEFORCE",
  max:		200,
  items:	NULL,
  values:	NULL,
  target:	&Wave5_Min_WaveForce,
  type:		GM_DEBUG_MENU_FLAG|GM_DEBUG_MENU_DEC,
};
#endif

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

/* キャラクタ監視機構用構造体 */
typedef struct _conflict_control{
	int			flag ;
	float		range ;
	float		fall ;
	int			pad ;
	FVECTOR		pos ;
} CONFLICT_CONTROL ;
#define MAX_CONFLICT		(32)

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
	int			flag ;
	int			tri_id ;
	int			tex_id[MAX_PLANE] ;
	DG_TEX		*tex[MAX_PLANE] ;
	DG_TEX		local_tex[3] ;
	int			light_enable ;
	int			envmap_rotate ;
	int			alpha_blend_flag ;				/* 強制半透明設定フラグ */

	int			mesh_x, mesh_z ;
	int			div_x, div_y ;
	int			mesh_size_x, mesh_size_z ;
	FVECTOR		invisible_bound[2]	;			/* 非表示領域指定 */
	int			invisible_enabled ;				/* 非表示領域指定許可フラグ */
	int			count ;
	int			water_ctrl_flag ;
	int			max_patch_plane ;				/* 最大重ね合わせ数 */
	int			visible_mode ;					/* 表示・非表示モード */

	/* 波制御パラメータ */
	float		influence_speed ;				/* 高さ変化に対するフォースの影響スピード */
	float		min_waveforce ;					/* 減衰許容力 */
	float		lose_speed ;					/* 減衰スピード */

	WaveParts	w_parts[ PATCH_X * PATCH_Z ];
	float		ave_height ;
	DG_PATCH_VERT	verts[2][ PATCH_X * PATCH_Z * ( MESH_X * MESH_Z ) ] ;		/* 共有頂点データ */
	DG_PATCH_VERT	verts2[2][ PATCH_X * PATCH_Z * ( MESH_X * MESH_Z ) ] ;		/* 共有頂点データ */
	DG_PATCH_VERT	*verts3[2] ;
	DG_PATCH_PARTS	parts[ PATCH_X * PATCH_Z * ( MESH_DIV_X * MESH_DIV_Z ) ] ;	/* 共有パッチデータ */
	int			rotate_x[MAX_PLANE] ;
	int			rotate_z[MAX_PLANE] ;
	int			speed_x[MAX_PLANE] ;
	int			speed_z[MAX_PLANE] ;
	float		offset_u[MAX_PLANE] ;
	float		offset_v[MAX_PLANE] ;
	SVECTOR		alpha_count[ MAX_PARAM_INDEX/4 ];
	SVECTOR		alpha_add[ MAX_PARAM_INDEX/4 ];
	SVECTOR		alpha_table[ MAX_PARAM_INDEX/4 ];

	/* キャラクタ監視用 */
	KEEP_CONTROL	keep_list[MAX_KEEP] ;

	/* 当たり判定用 */
	TARGET			target ;
	POWER_TARGET	power_target ;
	FVECTOR			target_force ;
	FVECTOR			damage_pos ;
	int				target_damage_flag ;
	int				invinsible_time ;

	int				debug_count ;

#if 0
	/* debug */
	DG_OBJS				*ray_objs ;
	FMATRIX				ray_light[2] ;
	CONFLICT_CONTROL	ray_conflict_list[ 32 ];
	FVECTOR				ray_rots[ 38 ];
#endif

} Work ;

static Work		*work_ptr = NULL ;

#define GET_VERTS_VOFFSET( __vx, __vz )	( (__vx) + MESH_X * (__vz) )
#define GET_VERTS_POFFSET( __px, __pz )	( ( (__pz) * PATCH_X + (__px) ) * MESH_X * MESH_Z )
#define GET_PATCH_VERTS( _verts, _px, _pz, _vx, _vz ) \
( &( (_verts)[ GET_VERTS_POFFSET( _px, _pz ) + GET_VERTS_VOFFSET( _vx, _vz )  ] ) )
#define GET_PARTS_VOFFSET( __vx, __vz )	( (__vx) + MESH_DIV_X * (__vz) )
#define GET_PARTS_POFFSET( __px, __pz )	( ( (__pz) * PATCH_X + (__px) ) * MESH_DIV_X * MESH_DIV_Z )


#define RND( _n )	( ( ( BP_PS2_rand() >> 16 ) * (_n) ) >> 15 )
#define RNDS( _n )	( ( ( ( BP_PS2_rand() >> 16 ) * (_n) ) >> 14 ) - ( _n ) )
/* ---------------------------------------------------------------- */
static DG_PATCH* MakePatchMeshEx( Work *work, int s, int t )
{
	DG_PATCH	*patch ;
	DG_PATCH_PARTS	*parts ;
	int			i, j ;

	patch = DG_MakePatch( 0, 0, 0 );
	if ( patch == NULL ) return ( NULL );
	patch->n_patch = MESH_DIV_X * MESH_DIV_Z ;
	patch->parts = &work->parts[ GET_PARTS_POFFSET( s, t ) ];
	patch->verts[ 0 ] = GET_PATCH_VERTS( work->verts[0], s, t, 0, 0 );
	patch->verts[ 1 ] = GET_PATCH_VERTS( work->verts[1], s, t, 0, 0 );

	for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
		for ( j = 0 ; j < MESH_DIV_X ; j++ ){
			parts = &patch->parts[ GET_PARTS_VOFFSET( j, i ) ] ;
			/* 頂点インデックス設定 */
			parts->v_index[ 0 ] = GET_VERTS_VOFFSET( j + 0, i + 0 );
			parts->v_index[ 1 ] = GET_VERTS_VOFFSET( j + 1, i + 0 );
			parts->v_index[ 2 ] = GET_VERTS_VOFFSET( j + 0, i + 1 );
			parts->v_index[ 3 ] = GET_VERTS_VOFFSET( j + 1, i + 1 );
			/* 隣接パッチインデックス設定 */
			if ( j > 0 ){
				parts->parts_index[0] = GET_PARTS_VOFFSET( j - 1, i ) ;
			} else {
				if ( s > 0 ){
					parts->parts_index[0] = GET_PARTS_VOFFSET( MESH_DIV_X - 1, i ) + GET_PARTS_POFFSET( -1, 0 ) ;
				} else {
					parts->parts_index[0] = -1 ;
				}
			}
			if ( j < ( MESH_DIV_X - 1 ) ){
				parts->parts_index[1] = GET_PARTS_VOFFSET( j + 1, i ) ;
			} else {
				if ( s < ( PATCH_X - 1 ) ){
					parts->parts_index[1] = GET_PARTS_VOFFSET( 0, i ) + GET_PARTS_POFFSET( 1, 0 );
				} else {
					parts->parts_index[1] = -1 ;
				}
			}
			if ( i > 0 ){
				parts->parts_index[2] = GET_PARTS_VOFFSET( j, i - 1 ) ;
			} else {
				if ( t > 0 ){
					parts->parts_index[2] = GET_PARTS_VOFFSET( j, MESH_DIV_Z - 1 ) + GET_PARTS_POFFSET( 0, -1 ) ;
				} else {
					parts->parts_index[2] = -1 ;
				}
			}
			if ( i < ( MESH_DIV_Z - 1 ) ){
				parts->parts_index[3] = GET_PARTS_VOFFSET( j, i + 1 ) ;
			} else {
				if ( t < ( PATCH_Z - 1 ) ){
					parts->parts_index[3] = GET_PARTS_VOFFSET( j, 0 ) + GET_PARTS_POFFSET( 0, 1 ) ;
				} else {
					parts->parts_index[3] = -1 ;
				}
			}
			/* その他設定 */
			parts->flag = 0 ;
		}
	}

	return ( patch );

}

/* 波ブロックの初期化 */
static void InitWaveParts( Work *work, WaveParts *wave, int s, int t )
{
	DG_PATCH		*patch ;
	DG_PATCH_VERT	*verts ;
	int			i, j, k ;

	/* 各頂点パラメータなどの初期化 */
	/* 加速度初期化 */
	for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
		for ( j = 0 ; j < MESH_DIV_X ; j++ ){
			wave->force[ j + i * ( MESH_DIV_X ) ] = _RND(MESH_WIDTH_Z/64) - MESH_WIDTH_Z/128 ;
			//wave->index[ j + i * ( MESH_DIV_X ) ] = RND( MAX_PARAM_INDEX );
		}
	}
	/* オブジェクトの初期化 */
	for ( i = 0 ; i < work->max_patch_plane ; i++ ){
		patch = MakePatchMeshEx( work, s, t );
		//patch = DG_MakePatchMesh( 0/*DG_PATCH_REFLECTPLANE*/, MESH_X, MESH_Z );
		switch ( i ){
		  case 0:
			wave->patch[i] = patch ;
			break ;
		  case 1:
			patch->flag |= DG_PATCH_NOCHECK ;	/* １枚目と共有するので各局面のチェックをしないように設定 */
			patch->verts[ 0 ] = GET_PATCH_VERTS( work->verts2[0], s, t, 0, 0 );
			patch->verts[ 1 ] = GET_PATCH_VERTS( work->verts2[1], s, t, 0, 0 );
			if ( work->max_patch_plane != 3 ){
				/* 環境マップモード設定 */
				SVECTOR	rot ;
				patch->flag |= DG_PATCH_REFLECTPLANE ;
				/* 環境マップ補正用マトリクス生成 */
				DG_SetPos( &DG_UnitMatrix );
				rot.vx = rot.vy = 0 ;
				rot.vz = work->envmap_rotate ;
				DG_RotatePos( &rot );
				DG_GetPos( &patch->envmap_correct );
			}
			wave->patch[i] = patch ;
			/* テクスチャが加算ＯＲ減算半透明であれば強制黒フォグフラグを立てる */
			if ( ( work->tex[1]->tex_trans.alpha.data & 0xf ) == SCE_GS_SET_ALPHA( 0, 2, 0, 0, 0 ) ){
				patch->flag |= DG_PATCH_FOGBLACK ;
			}
			if ( ( work->tex[1]->tex_trans.alpha.data & 0xf ) == SCE_GS_SET_ALPHA( 2, 1, 0, 0, 0 ) ){
				patch->flag |= DG_PATCH_FOGBLACK ;
			}
			if ( work->max_patch_plane == 3 ) patch->flag |= DG_PATCH_FOGBLACK ;
			break ;
		  case 2:
			patch->flag |= DG_PATCH_NOCHECK ;	/* １枚目と共有するので各局面のチェックをしないように設定 */
			patch->verts[ 0 ] = GET_PATCH_VERTS( work->verts3[0], s, t, 0, 0 );
			patch->verts[ 1 ] = GET_PATCH_VERTS( work->verts3[1], s, t, 0, 0 );
			//patch->flag |= DG_PATCH_BUMPMAP ;
			wave->patch[i] = patch ;
			/* テクスチャが加算ＯＲ減算半透明であれば強制黒フォグフラグを立てる */
			if ( ( work->tex[1]->tex_trans.alpha.data & 0xf ) == SCE_GS_SET_ALPHA( 0, 2, 0, 0, 0 ) ){
				patch->flag |= DG_PATCH_FOGBLACK ;
			}
			if ( ( work->tex[1]->tex_trans.alpha.data & 0xf ) == SCE_GS_SET_ALPHA( 2, 1, 0, 0, 0 ) ){
				patch->flag |= DG_PATCH_FOGBLACK ;
			}
			if ( work->max_patch_plane == 3 ) patch->flag |= DG_PATCH_FOGBLACK ;
			break ;
		}
		if ( work->alpha_blend_flag ) patch->flag |= DG_PATCH_SEMITRANS ;
		patch->light = work->light ;
		GM_GroupObject( patch, work->map );
		DG_QueuePatchObjs( patch );
		DG_ConfigPatchLOD( patch, 13 );
		patch->tri_id = work->tri_id ;
		patch->tex = work->tex[i] ;
	}
	/* スクラッチパッド上で一度構築してから転送 */
	verts = SCRPAD_ADDR ;
	for ( i = 0 ; i < MESH_Z ; i++ ){
		for ( j = 0 ; j < MESH_X ; j++ ){
			if ( i == 0 || j == 0 || i == (MESH_Z-1) || j == (MESH_X-1) ){
				verts->pos.vx = j * MESH_WIDTH_X - ( MESH_DIV_X * MESH_WIDTH_X ) / 2 ;
				verts->pos.vz = i * MESH_WIDTH_Z - ( MESH_DIV_Z * MESH_WIDTH_Z ) / 2 ;
			} else {
				verts->pos.vx = j * MESH_WIDTH_X - ( MESH_DIV_X * MESH_WIDTH_X ) / 2 ;
				verts->pos.vz = i * MESH_WIDTH_Z - ( MESH_DIV_Z * MESH_WIDTH_Z ) / 2 ;
				//verts->pos.vx = j * MESH_WIDTH_X - ( MESH_DIV_X * MESH_WIDTH_X ) / 2 + RNDS(MESH_WIDTH_X/4) ;
				//verts->pos.vz = i * MESH_WIDTH_Z - ( MESH_DIV_Z * MESH_WIDTH_Z ) / 2 + RNDS(MESH_WIDTH_Z/4) ;
			}
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
			//verts->uv.vx = verts->pos.vx / ( MESH_WIDTH_X * MESH_X / 4.0f) ;
			//verts->uv.vy = verts->pos.vz / ( MESH_WIDTH_Z * MESH_Z / 4.0f) ;
			verts->uv.vx = ( j * MESH_WIDTH_X - ( ( MESH_X - 1 ) * MESH_WIDTH_X ) / 2 ) / ( MESH_WIDTH_X * 4.0f) ;
			verts->uv.vy = ( i * MESH_WIDTH_Z - ( ( MESH_Z - 1 ) * MESH_WIDTH_Z ) / 2 ) / ( MESH_WIDTH_Z * 4.0f) ;
			verts->uv.vz = 1.0f ;
			verts->uv.vw = 128.0f ;
			verts++ ;
		}
	}
	verts = SCRPAD_ADDR ;
	for ( i = 0 ; i < work->max_patch_plane ; i++ ){
		patch = wave->patch[i] ;
		FlushCache( 0 );
		DG_StartSprToMem( patch->verts[ 0 ], verts, MESH_X * MESH_Z * 4 );
		DG_EndSprToMem();
		DG_StartSprToMem( patch->verts[ 1 ], verts, MESH_X * MESH_Z * 4 );
		DG_EndSprToMem();
		DG_SetupPatchMeshBounding( patch );
	}

	/* パッチ曲面バウンディング補正 */
	for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
		for ( j = 0 ; j < MESH_DIV_X ; j++ ){
			DG_PATCH_PARTS	*parts ;
			for ( k = 0 ; k < work->max_patch_plane ; k++ ){
				patch = wave->patch[k] ;
				parts = &patch->parts[ GET_PARTS_VOFFSET( j, i ) ] ;
				parts->min.vx = j * MESH_WIDTH_X - ( MESH_DIV_X * MESH_WIDTH_X ) / 2 ;
				parts->min.vz = i * MESH_WIDTH_Z - ( MESH_DIV_Z * MESH_WIDTH_Z ) / 2 ;
				parts->max.vx = ( j + 1 ) * MESH_WIDTH_X - ( MESH_DIV_X * MESH_WIDTH_X ) / 2 ;
				parts->max.vz = ( i + 1 ) * MESH_WIDTH_Z - ( MESH_DIV_Z * MESH_WIDTH_Z ) / 2 ;
				//parts->max.vx = MESH_WIDTH_X * 3 / 2 ;
				//parts->min.vx = -MESH_WIDTH_X * 3 / 2 ;
				//parts->max.vz = MESH_WIDTH_Z * 3 / 2 ;
				//parts->min.vz = -MESH_WIDTH_Z * 3 / 2 ;
				parts->max.vy = 2000.0f ;
				parts->min.vy = -2000.0f ;
				if ( k != 0 ){
					wave->patch[ k ]->parts = wave->patch[ 0 ]->parts ;
				}
			}
		}
	}

}
/* 波ブロックの開放 */
static void FreeWaveParts( WaveParts *wave, int max_patch_plane )
{
	int		i ;
	for ( i = 0 ; i < max_patch_plane ; i++ ){
		DG_DequeuePatchObjs( wave->patch[i] );
		DG_FreePatch( wave->patch[i] );
	}
}
/* 波ブロックアニメーション処理(加速度更新) */
static float CalcWaveForce( Work *work, WaveParts *org_wave )
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

	///* バッファ切り替え */
	//for ( i = 0 ; i < work->max_patch_plane ; i++ ){
	//	DG_SwitchBuffPatch( wave->patch[ i ] );
	//}
	verts = wave->patch[0]->verts[ wave->patch[0]->buffer_clock ];

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
#if 0
				f = h - height[ x + z ] ;
				f = force[ x + z ] + f * 0.01f ;
				if ( f > 3.0f ) f *= 0.99f ;
#else
				/* プラント海面プログラムに適したパラメータに変更 */
				f = h - height[ x + z ] ;
				//f = force[ x + z ] + f * 0.03f ;	/* 波持続係数 */
				//if ( f > 50.0f ) f *= 0.97f ;	/* 減衰スピード */
				/* 高さの差に影響スピードで補正 */
				f = force[ x + z ] + f * work->influence_speed ;
				/* 指定した減衰許容力以上のフォースがある場合には減衰させる */
				if ( DG_FABS( f ) > work->min_waveforce ) f *= work->lose_speed ;
#endif
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
#if 0
/* 波を盛り上げる */
static int RiseWave( Work *work, CONFLICT_CONTROL *cf_ctrl )
{
	int		i, j, x, z ;
	float	r_range, rangerange4 ;

	r_range = 1.0f / cf_ctrl->range ;
	rangerange4 = cf_ctrl->range * cf_ctrl->range * 2.0f * 2.0f ;
	for ( i = 0 ; i < PATCH_Z ; i++ ){
		for ( j = 0 ; j < PATCH_X ; j++ ){
			WaveParts	*wave ;
			DG_PATCH_VERT	*verts ;
			FVECTOR	pos ;

			wave = &work->w_parts[ j + i * PATCH_X ] ;
			{/* 大域領域チェック */
				float	dx, dz ;
				dx = DG_FABS( cf_ctrl->pos.vx - wave->offset.vx ) ;
				dz = DG_FABS( cf_ctrl->pos.vz - wave->offset.vz ) ;
				if ( dx > ( MESH_WIDTH_X * MESH_DIV_X + cf_ctrl->range * 2.0f ) ) continue ;
				if ( dz > ( MESH_WIDTH_Z * MESH_DIV_Z + cf_ctrl->range * 2.0f ) ) continue ;
			}

			{/* 相対座標取得 */
				FMATRIX	inv_mat ;
				pos = cf_ctrl->pos ;
				pos.vw = 1.0f ;
				_sceVu0InversMatrix( &inv_mat, &wave->patch[0]->world );
				_sceVu0ApplyMatrix( &pos, &inv_mat, &pos );
			}

			/* 判定スキップチェック＆制御点の落下加速補正 */
			if ( cf_ctrl->flag == 0 ){
				if ( pos.vy < ( -cf_ctrl->range ) ){
					continue ;
				}
				/* 制御点の落下補正 */
				if ( pos.vy > 0.0f ){
					cf_ctrl->fall += 9.6f / 60.0f * 1000.0f ;
				}
			} else {
				if ( pos.vy > ( cf_ctrl->range ) ){
					continue ;
				}
				/* 制御点の落下補正 */
				if ( pos.vy < 0.0f ){
					cf_ctrl->fall += 9.6f / 60.0f * 1000.0f ;
				}
			}

			/* 各頂点とのあたり判定 */
			for ( z = 0 ; z < MESH_DIV_Z ; z++ ){
				verts = wave->patch[0]->verts[ wave->patch[0]->buffer_clock ] ;
				verts += z * MESH_X ;
				for ( x = 0 ; x < MESH_DIV_X ; verts++, x++ ){
					FVECTOR		vec ;
					float		height, len, sq ;
					int			offset ;
					offset = x + z * MESH_DIV_X ;
					vec.vx = verts->pos.vx - pos.vx ;
					vec.vz = verts->pos.vz - pos.vz ;
					sq = vec.vx * vec.vx + vec.vz * vec.vz ;
					if ( sq > ( rangerange4 ) ) continue ;

					//vec.vy = wave->height[ offset ] - pos.vy ;
					len = sq * r_range ;

					/* 頂点が有効範囲内だった場合 */
					if ( cf_ctrl->flag == 0 ){
						/* 対象物が水面下の場合 */
						height = pos.vy - len - cf_ctrl->fall ;	/* その地点のあたり球の高さを求める */
						if ( height > wave->height[ offset ] ){
							wave->force[ offset ] = 0 ;
							wave->height[ offset ] = height ;
						}
						
					} else {
						/* 対象物が水面上の場合 */
						height = pos.vy + len + cf_ctrl->fall ;	/* その地点のあたり球の高さを求める */
						if ( height < wave->height[ offset ] ){
							wave->force[ offset ] = 0 ;
							wave->height[ offset ] = height ;
						}
						
					}
				}
			}
		}
	}

	/* 突き抜けチェック */
	if ( cf_ctrl->flag == 0 ){
		if ( ( cf_ctrl->pos.vy - work->center.vy ) > ( 1.5f * cf_ctrl->range ) ){
			/* チェック方向反転 */
			cf_ctrl->flag = 1 ;
			/* 制御点の落下補正 */
			cf_ctrl->fall = 0.0f ;
		}
	} else {
		if ( ( work->center.vy - cf_ctrl->pos.vy ) > ( 1.5f * cf_ctrl->range ) ){
			/* チェック方向反転 */
			cf_ctrl->flag = 0 ;
			/* 制御点の落下補正 */
			cf_ctrl->fall = 0.0f ;
		}
	}
	return ( 0 );
}
#endif
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
	tex->u_scale = (float)tx / w ;
	tex->v_scale = (float)ty / h ;
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
static void TargetCallback( TARGET *offence, TARGET *defence, Work *work )
{
	POWER_TARGET	*power ;

	if ( work->invinsible_time ) return ;
	if ( offence->power == NULL ) return ;
	power = offence->power ;
	switch ( power->type & POWER_CLASS ){
	  case POWER_EXPLODE:
		if ( !( offence->weapon_type & WP_CLAYMORE ) ){
			work->damage_pos = offence->center ;
			work->invinsible_time = 60 ;
			work->target_damage_flag = 1 ;
		}
		break ;
	  case POWER_ONCE:
	  case POWER_DECREASE:
	  case POWER_THRESHOLD:
	  case POWER_CONST:
	  default:
		break ;
	}
}

/* ---------------------------------------------------------------- */
	/*
		プログラムメイン処理
	*/
static void Act( Work *work )
{
	GV_MSG		*msg ;
	int			n_msg, i, j, k ;
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
			  case 2:/* 表示・非表示設定 */
				work->visible_mode = msg->message[1] ;
				{
					for ( i = 0 ; i < PATCH_Z ; i++ ){
						for ( j = 0 ; j < PATCH_X ; j++ ){
							WaveParts	*wave ;
							wave = &work->w_parts[ j + i * PATCH_X ] ;
							for ( k = 0 ; k < work->max_patch_plane ; k++ ){
								if ( work->visible_mode ){
									wave->patch[k]->flag &= ~DG_PATCH_INVISIBLE ;
								} else {
									wave->patch[k]->flag |= DG_PATCH_INVISIBLE ;
								}
							}
						}
					}
				}
				break ;
			  case 3:/* 待機状態へ（スリープ） */
				GV_WaitMessage( work, work->name );
				break ;
			  case 256:/* 表示座標の移動 */
				work->center.vx = msg->message[1] ;
				work->center.vy = msg->message[2] ;
				work->center.vz = msg->message[3] ;
				break ;
			  case 257:/* テクスチャの変更 */
				{
					int		i, j, k ;
					for ( k = 0 ; k < work->max_patch_plane ; k++ ){
						/* テクスチャの取得 */
						work->tex_id[ k ] = msg->message[ k + 1 ];
						work->tex[ k ] = DG_GetTexture2( work->tri_id, work->tex_id[ k ] );
						TextureRepeatSet( work->tex[ k ] );
						/* パッチ曲面オブジェクトへのテクスチャ反映 */
						for ( i = 0 ; i < PATCH_Z ; i++ ){
							for ( j = 0 ; j < PATCH_X ; j++ ){
								WaveParts	*wave ;
								wave = &work->w_parts[ j + i * PATCH_X ] ;
								wave->patch[ k ]->tex = work->tex[ k ];
							}
						}
					}
				}
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
				if ( keep->count == 0 ) DropWave( work, &keep->old_pos, len * 10 );
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
				for ( k = 0 ; k < work->max_patch_plane ; k++ ){
					wave->patch[k]->world.m[3][0] = wave->offset.vx ;
					wave->patch[k]->world.m[3][1] = wave->offset.vy ;
					wave->patch[k]->world.m[3][2] = wave->offset.vz ;
				}
			}
		}
	}

	{/* アルファテーブルアニメーション（４つ分をまとめて処理） */
		static FVECTOR	sub = {2048.0f,2048.0f,2048.0f,2048.0f} ;
		static FVECTOR	scale = {3.14159265f/2048.0f,64.0f,0.0f,0.0f} ;
		SVECTOR	*svec, *alpha, *add ;
		FVECTOR	vec ;
		svec = work->alpha_count ;
		add = work->alpha_add ;
		alpha = work->alpha_table ;
		for ( i = 0 ; i < MAX_PARAM_INDEX/4 ; svec++, alpha++, add++ , i++ ){
#ifndef PSX2
			/* カウンタ変化＆ＰＳ角度からラジアンへ */
			svec->vx = ( svec->vx + add->vx ) & 4095 ;
			svec->vy = ( svec->vy + add->vy ) & 4095 ;
			svec->vz = ( svec->vz + add->vz ) & 4095 ;
			svec->pad = ( svec->pad + add->pad ) & 4095 ;
			vec.vx = ( svec->vx - 2048 ) 3.14159265f / 2048.0f ;
			vec.vy = ( svec->vy - 2048 ) 3.14159265f / 2048.0f ;
			vec.vz = ( svec->vz - 2048 ) 3.14159265f / 2048.0f ;
			vec.vw = ( svec->pad - 2048 ) 3.14159265f / 2048.0f ;
#else
			/* カウンタ変化＆ＰＳ角度からラジアンへ */
			asm volatile ("
				ld			$4,0(%0)		# $4 = svec
				ld			$5,0(%4)		# $4 = svec
				lqc2		vf2,0(%1)		# vf2 = scale
				lqc2		vf3,0(%2)		# vf3 = sub
				paddsh		$4,$4,$5		# $4 = svec + add
				pand		$4,$4,%5		# $4 = svec & 4095
				sd			$4,0(%0)		# svec = $4
				pextlh		$4,$4,$0		# $4 = svec << 16 (符号拡張３２ビット化・ステップ１)
				psraw		$4,$4,16		# $4 = svec (符号拡張３２ビット化・ステップ２)
				qmtc2		$4,vf1			# vf1 = svec
				vitof0.xyzw	vf1,vf1			# vf1 = (float)svec
				vmulax.xyzw	ACC,vf1,vf2		# ACC = svec * scale.x
				vmsubx.xyzw	vf1,vf3,vf2		# vf1 = svec * scale.x - 2048 * scale.x = ( svec - 2048 ) * scale.x
				sqc2		vf1,0(%3)		# vec = vf1 (PS angle > radian amgle )
			"::"r"(svec),"r"(&scale),"r"(&sub), "r"(&vec),"r"(add),"r"(0x0fff0fff0fff0fff):"$4","$5","memory" );
#endif
			/* サイン値計算 */
			MT_SinX4( &vec, &vec );
#ifndef PSX2
			/* サイン値からアルファ値を算出 */
			alpha->vx = vec.vx * 64 + 64 ;
			alpha->vy = vec.vy * 64 + 64 ;
			alpha->vz = vec.vz * 64 + 64 ;
			alpha->pad = vec.vw * 64 + 64 ;
#else
			/* サイン値からアルファ値を算出 */
			asm volatile ("
				lqc2		vf1,0(%0)		# 
				lqc2		vf2,0(%1)		# 
				vadday.xyzw	ACC,vf0,vf2		# ACC = 64
				vmaddy.xyzw	vf1,vf1,vf2		# vf1 = 64 + vec * 64
				vftoi0.xyzw	vf1,vf1			# vf1 = (int)vf1
				qmfc2		$4,vf1			# $4 = vf1
				ppach		$4,$0,$4		# $4 = ivec2svec( $4 )
				sd			$4,0(%2)		# 
			"::"r"(&vec),"r"(&scale),"r"(alpha):"$4","memory");
#endif
		}
	}

#if 1
	{/* 波が減衰しきらないように端の頂点に対し随時フォースを設定する */
		WaveParts	*wave ;
		switch ( RND(40) ){
		  case 0:
			wave = &work->w_parts[ RND(PATCH_X) ] ;
			wave->force[ RND(MESH_DIV_X) ] =
			  RND(MESH_WIDTH_Z/64) - MESH_WIDTH_Z/128 ;
			break ;
		  case 1:
			wave = &work->w_parts[ RND(PATCH_X) + ( PATCH_Z -1 ) * PATCH_X ] ;
			wave->force[ RND(MESH_DIV_X) + ( MESH_DIV_Z - 1 ) * MESH_DIV_X ] =
			  RND(MESH_WIDTH_Z/64) - MESH_WIDTH_Z/128 ;
			break ;
		  case 2:
			wave = &work->w_parts[ 0 + RND(PATCH_Z) * PATCH_X ] ;
			wave->force[ 0 + RND(MESH_DIV_Z) * MESH_DIV_X ] =
			  RND(MESH_WIDTH_Z/64) - MESH_WIDTH_Z/128 ;
			break ;
		  case 3:
			wave = &work->w_parts[ ( PATCH_X - 1 ) + RND(PATCH_Z) * PATCH_X ] ;
			wave->force[ ( PATCH_X - 1 ) + RND(MESH_DIV_Z) * MESH_DIV_X ] =
			  RND(MESH_WIDTH_Z/64) - MESH_WIDTH_Z/128 ;
			break ;
		}
	}
#else
	{/* 波が減衰しきらないように端の頂点に対し随時フォースを設定する */
		WaveParts	*wave ;
		switch ( RND(40) ){
		  case 0:
			wave = &work->w_parts[ RND(PATCH_X) ] ;
			wave->height[ RND(MESH_DIV_X) ] =
			  RND(MESH_WIDTH_Z/64) - MESH_WIDTH_Z/128 ;
			break ;
		  case 1:
			wave = &work->w_parts[ RND(PATCH_X) + ( PATCH_Z -1 ) * PATCH_X ] ;
			wave->height[ RND(MESH_DIV_X) + ( MESH_DIV_Z - 1 ) * MESH_DIV_X ] =
			  RND(MESH_WIDTH_Z/64) - MESH_WIDTH_Z/128 ;
			break ;
		  case 2:
			wave = &work->w_parts[ 0 + RND(PATCH_Z) * PATCH_X ] ;
			wave->height[ 0 + RND(MESH_DIV_Z) * MESH_DIV_X ] =
			  RND(MESH_WIDTH_Z/64) - MESH_WIDTH_Z/128 ;
			break ;
		  case 3:
			wave = &work->w_parts[ ( PATCH_X - 1 ) + RND(PATCH_Z) * PATCH_X ] ;
			wave->height[ ( MESH_DIV_X - 1 ) + RND(MESH_DIV_Z) * MESH_DIV_X ] =
			  RND(MESH_WIDTH_Z/64) - MESH_WIDTH_Z/128 ;
			break ;
		  default:
			break ;
		}
	}
#endif

	/* バッファ切り替え */
	for ( i = 0 ; i < PATCH_Z ; i++ ){
		for ( j = 0 ; j < PATCH_X ; j++ ){
			WaveParts	*wave ;
			wave = &work->w_parts[ j + i * PATCH_X ] ;
			for ( k = 0 ; k < work->max_patch_plane ; k++ ){
				DG_SwitchBuffPatch( wave->patch[ k ] );
			}
		}
	}

	for ( k = 0 ; k <= DM_FrameSkip ; k++ ){
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
				ave_height = CalcWaveForce( work, wave );
				total_ave_height += ave_height ;
			}
		}
		work->ave_height = ave_height / ( PATCH_X * PATCH_Z ) ;

		/* テクスチャアニメーション計算 */
		for ( i = 0 ; i < work->max_patch_plane ; i++ ){
			work->rotate_x[i] = ( work->rotate_x[i] + work->speed_x[i] ) & 65535 ;
			work->rotate_z[i] = ( work->rotate_z[i] + work->speed_z[i] ) & 65535 ;
			work->offset_u[i] = sinf( ( work->rotate_x[i] - 32768 ) * 3.14159265f / 32738.0f ) / 5.0f ;
			work->offset_v[i] = sinf( ( work->rotate_z[i] - 32768 ) * 3.14159265f / 32738.0f ) / 5.0f ;
		}

	}

	/* 隣接するオブジェクトにエッジの傾き情報をコピーする */
	for ( i = 0 ; i < PATCH_Z ; i++ ){
		for ( j = 0 ; j < PATCH_X ; j++ ){
			WaveParts	*wave, *link ;
			DG_PATCH_VERT	*dst_verts, *src_verts ;
			int			k ;
			wave = &work->w_parts[ j + i * PATCH_X ] ;
			if ( ( link = wave->link_x[0] ) != NULL ){
				src_verts = wave->patch[0]->verts[ wave->patch[0]->buffer_clock ];
				dst_verts = link->patch[0]->verts[ link->patch[0]->buffer_clock ];
				for ( k = 0 ; k < MESH_DIV_Z ; k++ ){
					dst_verts[ MESH_DIV_X + k * MESH_X ].pos.vx = src_verts[ 0 + k * MESH_X ].pos.vx + MESH_WIDTH_X * MESH_DIV_X ;
					dst_verts[ MESH_DIV_X + k * MESH_X ].pos.vz = src_verts[ 0 + k * MESH_X ].pos.vz ;
					dst_verts[ MESH_DIV_X + k * MESH_X ].pos.vy = src_verts[ 0 + k * MESH_X ].pos.vy ;
					dst_verts[ MESH_DIV_X + k * MESH_X ].pos_ds = src_verts[ 0 + k * MESH_X ].pos_ds ;
					dst_verts[ MESH_DIV_X + k * MESH_X ].pos_dt = src_verts[ 0 + k * MESH_X ].pos_dt ;
				}
			}
			if ( ( link = wave->link_z[0] ) != NULL ){
				src_verts = wave->patch[0]->verts[ wave->patch[0]->buffer_clock ];
				dst_verts = link->patch[0]->verts[ link->patch[0]->buffer_clock ];
				for ( k = 0 ; k < MESH_DIV_X ; k++ ){
					dst_verts[ k + MESH_DIV_Z * MESH_X ].pos.vx = src_verts[ k + 0 * MESH_X ].pos.vx ;
					dst_verts[ k + MESH_DIV_Z * MESH_X ].pos.vz = src_verts[ k + 0 * MESH_X ].pos.vz + MESH_WIDTH_Z * MESH_DIV_Z ;
					dst_verts[ k + MESH_DIV_Z * MESH_X ].pos.vy = src_verts[ k + 0 * MESH_X ].pos.vy ;
					dst_verts[ k + MESH_DIV_Z * MESH_X ].pos_ds = src_verts[ k + 0 * MESH_X ].pos_ds ;
					dst_verts[ k + MESH_DIV_Z * MESH_X ].pos_dt = src_verts[ k + 0 * MESH_X ].pos_dt ;
				}
			}
			if ( ( link = wave->link_corner[0] ) != NULL ){
				src_verts = wave->patch[0]->verts[ wave->patch[0]->buffer_clock ];
				dst_verts = link->patch[0]->verts[ link->patch[0]->buffer_clock ];
				dst_verts[ MESH_DIV_X + MESH_DIV_Z * MESH_X ].pos.vx = src_verts[ 0 + 0 * MESH_X ].pos.vx + MESH_WIDTH_X * MESH_DIV_X ;
				dst_verts[ MESH_DIV_X + MESH_DIV_Z * MESH_X ].pos.vz = src_verts[ 0 + 0 * MESH_X ].pos.vz + MESH_WIDTH_Z * MESH_DIV_Z ;
				dst_verts[ MESH_DIV_X + MESH_DIV_Z * MESH_X ].pos.vy = src_verts[ 0 + 0 * MESH_X ].pos.vy ;
				dst_verts[ MESH_DIV_X + MESH_DIV_Z * MESH_X ].pos_ds.vy = src_verts[ 0 + 0 * MESH_X ].pos_ds.vy ;
				dst_verts[ MESH_DIV_X + MESH_DIV_Z * MESH_X ].pos_dt.vy = src_verts[ 0 + 0 * MESH_X ].pos_dt.vy ;
			}
		}
	}

	/* ＵＶアニメーション処理 */
	for ( i = 0 ; i < PATCH_Z ; i++ ){
		for ( j = 0 ; j < PATCH_X ; j++ ){
			WaveParts	*wave ;
			short		*alpha ;

			wave = &work->w_parts[ j + i * PATCH_X ] ;
			alpha = (short*)work->alpha_table ;

			FlushCache( 0 );
			DG_StartMemToSpr( SCRPAD_ADDR, wave->patch[0]->verts[ wave->patch[0]->buffer_clock ],
							 SIZEOF_QWORD(DG_PATCH_VERT) * MESH_X * MESH_Z );
			DG_EndMemToSpr();
			for ( k = 0 ; k < work->max_patch_plane ; k++ ){
				DG_PATCH_VERT	*verts ;
				DG_PATCH		*patch ;
				int				x, z ;
				float			u, v ;
				float		u_scale, v_scale ;

				patch = wave->patch[ k ] ;

				u_scale = work->tex[ k ]->u_scale ;
				v_scale = work->tex[ k ]->v_scale ;
				u = work->offset_u[ k ] ;
				v = work->offset_v[ k ] ;
				if ( k == 0 ){
					u += 0.5f ;
					v += 0.5f ;
				}
				u *= u_scale ;
				v *= v_scale ;

				if ( !( patch->flag & DG_PATCH_REFLECTPLANE ) ){
					/* 環境マップ式でない場合にはＵＶのアニメーションを行う */
					verts = SCRPAD_ADDR ;
					for ( z = 0 ; z < MESH_Z ; z++ ){
						for ( x = 0 ; x < MESH_X ; x++ ){
							//verts->uv.vx = verts->pos.vx / ( MESH_WIDTH_X * MESH_X / 4.0f) + u ;
							//verts->uv.vy = verts->pos.vz / ( MESH_WIDTH_Z * MESH_Z / 4.0f) + v ;
#if 0
							verts->uv.vx = ( x * MESH_WIDTH_X - ( MESH_DIV_X * MESH_WIDTH_X ) / 2 )
							  / ( MESH_WIDTH_X * MESH_DIV_X / 4.0f) * u_scale + u ;
							verts->uv.vy = ( z * MESH_WIDTH_Z - ( MESH_DIV_Z * MESH_WIDTH_Z ) / 2 )
							  / ( MESH_WIDTH_Z * MESH_DIV_Z / 4.0f) * v_scale + v ;
#else
#if 0
							verts->uv.vx = ( x * MESH_WIDTH_X + verts->pos_ds.vx - ( MESH_DIV_X * MESH_WIDTH_X ) / 2 )
							  / ( MESH_WIDTH_X * MESH_DIV_X / 4.0f) * u_scale + u ;
							verts->uv.vy = ( z * MESH_WIDTH_Z + verts->pos_dt.vz - ( MESH_DIV_Z * MESH_WIDTH_Z ) / 2 )
							  / ( MESH_WIDTH_Z * MESH_DIV_Z / 4.0f) * v_scale + v ;
#else
							{
								float	tmp_u, tmp_v, tmp_scale_u, tmp_scale_v ;
								tmp_scale_u = u_scale / ( MESH_WIDTH_X * MESH_DIV_X / 4.0f);
								tmp_scale_v = v_scale / ( MESH_WIDTH_Z * MESH_DIV_Z / 4.0f);
								DG_MULA( (float)x, (float)MESH_WIDTH_X );
								DG_MADDA( verts->pos_ds.vx, 1.0f );
								tmp_u = DG_MSUB( (float)( ( MESH_DIV_X * MESH_WIDTH_X ) / 2 ), 1.0f );
								DG_MULA( (float)z, (float)MESH_WIDTH_Z );
								DG_MADDA( verts->pos_ds.vz, 1.0f );
								tmp_v = DG_MSUB( (float)( ( MESH_DIV_Z * MESH_WIDTH_Z ) / 2 ), 1.0f );
								DG_MULA( tmp_u, tmp_scale_u );
								verts->uv.vx = DG_MADD( u, 1.0f );
								DG_MULA( tmp_v, tmp_scale_v );
								verts->uv.vy = DG_MADD( v, 1.0f );
							}
						
#endif
#endif
							verts++ ;
						}
					}

				}
				DG_StartSprToMem( patch->verts[ patch->buffer_clock ], SCRPAD_ADDR,
								 SIZEOF_QWORD(DG_PATCH_VERT) * MESH_X * MESH_Z );
				DG_EndSprToMem();
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

	/* 当たり判定処理 */
	if ( work->target_damage_flag ){
		extern int TAKABE_AddWaveForce( FVECTOR *pos, float drop_force );
		//TAKABE_AddWaveForce( &work->damage_pos, -800.0f );
		TAKABE_AddWaveForce( &work->damage_pos, 500.0f );
		work->target_damage_flag = 0 ;
	}
	if ( work->invinsible_time ) work->invinsible_time-- ;
	GM_ClearTargetDamage( &work->target );


#ifdef DEBUG_MODE
	if ( Wave5_DebugFlag ){
		work->influence_speed = (float)Wave5_Influence_Speed / 200.0f ;
		work->lose_speed = (float)Wave5_Lose_Speed / 200.0f ;
		work->min_waveforce = (float)Wave5_Min_WaveForce ;
	}
#if 0
	if ( 1 ){
		static int	data[3] ;
		DEBUG_Locate( 32, 350, 0 );
		DEBUG_Printf("wave5: addr = %08x\n", data );
		work->influence_speed = (float)data[0] / 10000.0f ;
		work->lose_speed = (float)data[1] / 10000.0f ;
		work->min_waveforce = (float)data[2] ;
	}
#endif
#endif
#if defined( DEBUG_MODE ) && (0)
	/* 波盛り上がり実験 */
	if ( GV_PadData[0].status & PAD_A ){
		FVECTOR		pos ;
		float		r, y_offset ;

		r = 5000.0f ;
		y_offset =( 128 - GV_PadData[0].right_dy ) * 200.0f ;
		pos = work->center ;
		pos.vy += y_offset ;
		DG_SetPos2( &pos, &DG_ZeroSVector );
		DG_PutObjs( work->ray_objs );

		{
			static int		data[19][2] = {
				{0,3000},
				{1,4000},
				{2,5000},
				{3,4000},
				{5,5000},
				{6,4000},
				{7,3000},
				{8,4000},
				{10,5000},
				{11,4000},
				{12,3000},
				{22,4000},
				{23,4000},
				{24,4000},
				{26,4000},
				{28,4000},
				{29,4000},
				{30,4000},
				{32,4000},
			};
			for ( i = 0 ; i < 19 ; i++ ){
				CONFLICT_CONTROL	*cf_ctrl ;
				cf_ctrl = &work->ray_conflict_list[ i ];
				cf_ctrl->pos = *(FVECTOR*)&work->ray_objs->objs[ data[ i ][ 0 ] ].world.m[3] ;
				cf_ctrl->range = data[ i ][ 1 ];
				cf_ctrl->pos.vy += cf_ctrl->range ;
			}
		}

		for ( i = 0 ; i < 19 ; i++ ){
			RiseWave( work, &work->ray_conflict_list[ i ] );
		}

	}
#endif

	//if ( work->light_enable ){
	//	DG_GetLightMatrix( &DG_ZeroVector, work->light );
	//}
}

/* ---------------------------------------------------------------- */
	/*
		プログラム終了処理
	*/
static void Die( Work *work )
{
	int		i, j ;

	{
		extern int	Vu1DrawReflectSurface[] ;
		DG_CurveSurfaceVuCode[1] = Vu1DrawReflectSurface ;
	}
	//if ( work->verts3[ 0 ] ) GV_Free( work->verts3[ 0 ] );
	//if ( work->verts3[ 1 ] ) GV_Free( work->verts3[ 1 ] );
	GM_FreeTarget( &work->target );
	for ( i = 0 ; i < PATCH_Z ; i++ ){
		for ( j = 0 ; j < PATCH_X ; j++ ){
			WaveParts	*wave ;
			wave = &work->w_parts[ j + i * PATCH_X ] ;
			FreeWaveParts( wave, work->max_patch_plane );
		}
	}
	work_ptr = NULL ;
#if defined( DEBUG_MODE ) && (0)
	DG_DequeueObjs( work->ray_objs );
	DG_FreeObjs( work->ray_objs );
#endif
}

/* ---------------------------------------------------------------- */
	/*
		ワーク初期化処理
	*/
static int GetResources( Work *work, int name, int where )
{
	DG_TEX		*tex ;
	int		i, j, k ;
	FVECTOR		pos ;

	work->name = name ;
	work->map = where ;
	GM_CurrentMap = where ;
	work->visible_mode = 1 ;

	work->light[1].m[3][0] = 128 ;
	work->light[1].m[3][1] = 128 ;
	work->light[1].m[3][2] = 128 ;
	work->light[1].m[3][3] = 128 ;

	if ( GCL_GetOption( 'f' ) != NULL ){
		work->flag = GCL_GetNextInt() ;
	}

	if ( GCL_GetOption( 's' ) != NULL ){
		work->max_patch_plane = 1 ;
	} else {
		work->max_patch_plane = 2 ;
	}

	if ( GCL_GetOption( 'b' ) != NULL ){
		work->max_patch_plane = 3 ;
	}

	if ( GCL_GetOption( 'l' ) != NULL ){
		work->light_enable = 1 ;
		//DG_GetLightMatrix( &DG_ZeroVector, work->light );
		work->light[0] = DG_LightMatrix ;
		work->light[1] = DG_ColorMatrix ;
#if 0
		printf("%s:light enable\n", __FILE__ );
		printf("dir 0:%f %f %f\n", work->light[0].m[0][0], work->light[0].m[1][0], work->light[0].m[2][0] );
		printf("col 0:%f %f %f\n", work->light[1].m[0][0], work->light[1].m[0][1], work->light[1].m[0][2] );
		printf("dir 1:%f %f %f\n", work->light[0].m[0][1], work->light[0].m[1][1], work->light[0].m[2][1]);
		printf("col 1:%f %f %f\n", work->light[1].m[1][0], work->light[1].m[1][1], work->light[1].m[1][2]);
		printf("dir 2:%f %f %f\n", work->light[0].m[0][2], work->light[0].m[1][2], work->light[0].m[2][2]);
		printf("col 2:%f %f %f\n", work->light[1].m[2][0], work->light[1].m[2][1], work->light[1].m[2][2]);
		printf("amb 2:%f %f %f\n", work->light[1].m[3][0], work->light[1].m[3][1], work->light[1].m[3][2]);
#endif
	}

	if ( GCL_GetOption( 'r' ) != NULL ){
		work->envmap_rotate = GCL_GetNextInt() ;
	}

	/* テクスチャ設定読み込み */
	if ( GCL_GetOption( 't' ) != NULL ) {
		work->tri_id = GCL_GetNextInt() ;
		work->tex_id[0] = GCL_GetNextInt() ;
		work->tex_id[1] = GCL_GetNextInt() ;
	} else {
		work->tri_id = TRI_ID ;
		work->tex_id[0] = TEX_ID ;
		work->tex_id[1] = TEX_ID ;
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

	/* 波パラメータ設定 */
	if ( GCL_GetOption( 'w' ) != NULL ){
		work->influence_speed = (float)GCL_GetNextInt() / 200.0f ;
		work->lose_speed = (float)GCL_GetNextInt() / 200.0f ;
		work->min_waveforce = (float)GCL_GetNextInt() ;
		/*
			2,138,188
			6,196,21 (*)
			4,188,69
		*/
	} else {
		/* 波パラメータ初期設定 */
		work->influence_speed = 0.03f ;
		work->lose_speed = 0.97f ;
		work->min_waveforce = 50.0f ;
	}

	/* 波パラメータ設定 */
	if ( GCL_GetOption( 'd' ) != NULL ){
		work->influence_speed = (float)GCL_GetNextInt() / 10000.0f ;
		work->lose_speed = (float)GCL_GetNextInt() / 10000.0f ;
		work->min_waveforce = (float)GCL_GetNextInt() ;
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
	work->tex[0] = tex = DG_GetTexture2( work->tri_id, work->tex_id[0] );
	/* 強制的にリピートテクスチャにする */
	TextureRepeatSet( tex );
	//tex->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 2, 1, 128 );
	//tex->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 2, 2, 2, 0, 128 );
	//work->local_tex = *tex ;
	//work->local_tex.tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 2, 1, 64 );
	//work->local_tex.tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 64 );
	//work->local_tex.tex_trans.alpha.data = SCE_GS_SET_ALPHA( 2, 0, 2, 1, 48 );

	work->tex[1] = tex = DG_GetTexture2( work->tri_id, work->tex_id[1] );
	/* 強制的にリピートテクスチャにする */
	TextureRepeatSet( tex );
	//tex->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 2, 1, 128 );
	//tex->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 1, 1, 128 );
	/*tex->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 1, 1, 128 );*/
	//tex->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 2, 1, 48 );
	//work->local_tex = *tex ;
	//work->local_tex.tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 64 );

#if 0 /* バンプマップ実験用 */
	if ( work->max_patch_plane == 3 ){
		/* バンプマッピングテクスチャ時の設定（テクスチャシフト量パラメータ） */
		int		t_size ;
		//work->verts3[ 0 ] = GV_Malloc( sizeof(DG_PATCH_VERT) * PATCH_X * PATCH_Z * ( MESH_X * MESH_Z ) );
		//work->verts3[ 1 ] = GV_Malloc( sizeof(DG_PATCH_VERT) * PATCH_X * PATCH_Z * ( MESH_X * MESH_Z ) );
		work->verts3[ 0 ] = work->verts2[ 0 ] ;
		work->verts3[ 1 ] = work->verts2[ 1 ] ;
#if 1
		work->local_tex[0] = *( work->tex[ 0 ] );
		work->local_tex[1] = *( work->tex[ 1 ] );
		work->local_tex[2] = *( work->tex[ 1 ] );
		work->local_tex[1].tex_trans.alpha.data = SCE_GS_SET_ALPHA( 1, 2, 2, 0, 64 );/* バンプマップ１ */
		work->local_tex[2].tex_trans.alpha.data = SCE_GS_SET_ALPHA( 1, 0, 2, 2, 255 );/* バンプマップ２ */
		work->tex[ 1 ] = &work->local_tex[ 1 ];
		work->tex[ 2 ] = &work->local_tex[ 2 ];
		/* バンプ用シフト量設定 */
		t_size = 2 << ( ( work->local_tex[ 2 ].tex_trans.tex0.data >> 26 ) & 15 ) ;
		work->local_tex[ 2 ].tex_trans.vec1.vx = -16.0f / (float)t_size ;
		t_size = 2 << ( ( work->local_tex[ 2 ].tex_trans.tex0.data >> 30 ) & 15 ) ;
		work->local_tex[ 2 ].tex_trans.vec1.vy = -16.0f / (float)t_size ;
		//work->local_tex[ 2 ].tex_trans.vec1.vx = 1.0f / 256.0f ;
		//work->local_tex[ 2 ].tex_trans.vec1.vy = 1.0f / 256.0f ;
#endif
	}
#endif


	for ( i = 0 ; i < PATCH_Z ; i++ ){
		for ( j = 0 ; j < PATCH_X ; j++ ){
			WaveParts	*wave ;
			wave = &work->w_parts[ j + i * PATCH_X ] ;
			InitWaveParts( work, wave, j, i );
			wave->offset.vx = pos.vx + (float)( MESH_WIDTH_X * MESH_DIV_X ) * ( j - PATCH_X * 0.5f + 0.5f ) ;
			wave->offset.vy = pos.vy ;
			wave->offset.vz = pos.vz + (float)( MESH_WIDTH_Z * MESH_DIV_Z ) * ( i - PATCH_Z * 0.5f + 0.5f ) ;
			wave->offset.vw = 1.0f ;
			for ( k = 0 ; k < work->max_patch_plane ; k++ ){
				wave->patch[k]->world.m[3][0] = wave->offset.vx ;
				wave->patch[k]->world.m[3][1] = wave->offset.vy ;
				wave->patch[k]->world.m[3][2] = wave->offset.vz ;
			}
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

	work->speed_x[ 0 ] = 60/2 ;
	work->speed_z[ 0 ] = 43/2 ;
	work->speed_x[ 1 ] = 32/2 ;
	work->speed_z[ 1 ] = 17/2 ;
	work->speed_x[ 2 ] = 32/2 ;
	work->speed_z[ 2 ] = 17/2 ;

	{/* アルファテーブル初期化 */
		short	*s ;
		s = (short*)&work->alpha_count[ 0 ];
		for ( i = 0 ; i < MAX_PARAM_INDEX ; i++ ){
			*s++ = RND( 4096 );
		}
		s = (short*)&work->alpha_add[ 0 ];
		for ( i = 0 ; i < MAX_PARAM_INDEX ; i++ ){
			*s++ = RNDS( 30 );
		}
	}

	/* 例外的に環境マップ式曲面パッチ描画ＶＵプログラムを差し替える */
	DG_CurveSurfaceVuCode[1] = Vu1DrawReflectSurface2 ;

	{/* あたり判定設定 */
		FVECTOR		size, trg_pos ;
		int			class ;

		class = TARGET_POWER|TARGET_CHILD_ALWAYS|TARGET_DEFENSE|TARGET_ROTATE ;
		class = TARGET_POWER|TARGET_DEFENSE ;
		size.vx = ( PATCH_X * MESH_DIV_X * MESH_WIDTH_X ) * 0.5f ;
		size.vy = 1000.0f ;
		size.vz = ( PATCH_Z * MESH_DIV_Z * MESH_WIDTH_Z ) * 0.5f ;
		trg_pos = pos ;
		trg_pos.vy -= 1000.0f + 1000.0f ;
		GM_SetTarget( &work->target, class, work->map, ENEMY_SIDE, &size, &DG_ZeroVector );
		GM_SetPowerTarget( &work->target, &work->power_target, POWER_CONST, 1, 0, 0, &work->target_force );
		GM_MoveTarget( &work->target, &trg_pos );
		GM_PutTarget( &work->target );
		GM_SetTargetCallBack( &work->target, (TARGET_CALLBACK)TargetCallback, work );
	}


#ifdef DEBUG_MODE
	GM_AddDebugMenu( &debug_debugflag );
	GM_AddDebugMenu( &debug_influence_speed );
	GM_AddDebugMenu( &debug_lose_speed );
	GM_AddDebugMenu( &debug_min_waveforce );
	Wave5_Influence_Speed = work->influence_speed * 200.0f + 0.5 ;
	Wave5_Lose_Speed = work->lose_speed * 200.0f + 0.5 ;
	Wave5_Min_WaveForce = work->min_waveforce ;
#endif

	for ( i = 0 ; i < 10 ; i++ ){
		Act( work );
	}

#if defined( DEBUG_MODE ) && (0)
	{
		DG_DEF		*def ;
		def = GV_GetCache( GV_CacheID( GV_StrCode( "pdray_def_mt" ), 'k' ) );
		work->ray_objs = DG_MakeObjs( def, DG_FLAG_QUATROT, 0 );
		//DG_SetLightMatrix( work->ray_objs, work->ray_light );
		DG_QueueObjs( work->ray_objs );
		work->ray_objs->rots = work->ray_rots ;
		for ( i = 0 ; i < 38 ; i++ ){
			work->ray_rots[ i ] = DG_ZeroVector ;
		}
	}
#endif

	return (0);
}


/* ---------------------------------------------------------------- */
	/*
		シナリオ起動インターフェイス
	*/
void *NewSeaSurfaceHarrierSet( int name, int where )
{
	Work		*work ;

	if ( work_ptr != NULL ) return ( 0 );

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	work_ptr = work ;
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
/* ---------------------------------------------------------------- */
	/*
		マイクロプログラム読み込み
	*/
static void LoadVu0MicroProgram( void )
{
	extern DG_DMATAG	CalcPatchRayPickVu[] ;
	
	DG_DmaStartCheck( 0 );
	DPUT_D0_MADR( CalcPatchRayPickVu + 1 ) ;
	DPUT_D0_QWC( ( CalcPatchRayPickVu[0].qwc ) & 0x7fff ) ;
	//DPUT_D0_MADR( ((DG_DMATAG*)CalcPatchRayPickVu)->addr ) ;
	//DPUT_D0_QWC( ( ((DG_DMATAG*)CalcPatchRayPickVu)->qwc ) & 0x7fff ) ;
	//DPUT_D0_CHCR( 0x0141 ) ; /* STR:1 TIE:0 TTE:1 ASP:0 MOD:0 DIR:1 */
	asm volatile( "sync.l;sw %1, 0(%0)" :: "r"(D0_CHCR),"r"(0x141) );
	DG_WaitDma( 0 );
}


/* エルミート補間用マトリクス生成 */
static void MakeHermiteMatrix( FMATRIX *res, FVECTOR *q0, FVECTOR *q1, FVECTOR *d0, FVECTOR *d1 )
{
	static FMATRIX c_mat2 = {{
		{ 2.0f,-2.0f, 1.0f, 1.0f},
		{-3.0f, 3.0f,-2.0f,-1.0f},
		{ 0.0f, 0.0f, 1.0f, 0.0f},
		{ 1.0f, 0.0f, 0.0f, 0.0f}
	}};
	FMATRIX	data_mat ;
	*(FVECTOR*)data_mat.m[0] = *q0 ;
	*(FVECTOR*)data_mat.m[1] = *q1 ;
	*(FVECTOR*)data_mat.m[2] = *d0 ;
	*(FVECTOR*)data_mat.m[3] = *d1 ;
	_sceVu0MulMatrix( res, &data_mat, &c_mat2 );
	//_sceVu0ApplyMatrix( &ans, res, &tvec );
}
static void MakeDHermiteMatrix( FMATRIX *res, FVECTOR *q0, FVECTOR *q1, FVECTOR *d0, FVECTOR *d1 )
{
	static FMATRIX c_mat2 = {{
		{ 0.0f, 0.0f, 0.0f, 0.0f},
		{ 6.0f,-6.0f, 3.0f, 3.0f},
		{-6.0f, 6.0f,-4.0f,-2.0f},
		{ 0.0f, 0.0f, 1.0f, 0.0f}
	}};
	FMATRIX	data_mat ;
	*(FVECTOR*)data_mat.m[0] = *q0 ;
	*(FVECTOR*)data_mat.m[1] = *q1 ;
	*(FVECTOR*)data_mat.m[2] = *d0 ;
	*(FVECTOR*)data_mat.m[3] = *d1 ;
	_sceVu0MulMatrix( res, &data_mat, &c_mat2 );
	//_sceVu0ApplyMatrix( &ans, res, &tvec );
}
static void LerpVecFpu( FVECTOR *res, FVECTOR *v1, FVECTOR *v2, float t )
{
#if 0
	res->vx = ( v2->vx - v1->vx ) * t + v1->vx ;
	res->vy = ( v2->vy - v1->vy ) * t + v1->vy ;
	res->vz = ( v2->vz - v1->vz ) * t + v1->vz ;
#else
	/* X = x1 - ( x1 * t ) + ( x2 * t ) = ( x2 - x1 ) * t + x1 */
	DG_ADDA( v1->vx, 0.0f );
	DG_MSUBA( v1->vx, t );
	res->vx = DG_MADD( v2->vx, t );
	/* Y */
	DG_ADDA( v1->vy, 0.0f );
	DG_MSUBA( v1->vy, t );
	res->vy = DG_MADD( v2->vy, t );
	/* Z */
	DG_ADDA( v1->vz, 0.0f );
	DG_MSUBA( v1->vz, t );
	res->vz = DG_MADD( v2->vz, t );
#endif
}

/* 指定した座標における水面のおおよその位置を取得する */
static int GetWaveHeight( FVECTOR *res, FVECTOR *norm, FMATRIX *world, DG_PATCH_PARTS *parts, DG_PATCH_VERT *wverts, FVECTOR *org_pos )
{
	static FVECTOR	t_vec[] = {
		1.0,            1.0       , 1.0   , 1.0,		/* t=16/16*/
		0.823974609375, 0.87890625, 0.9375, 1.0,		/* t=15/16 */
		0.669921875   , 0.765625  , 0.875 , 1.0,		/* t=14/16 */
		0.536376953125, 0.66015625, 0.8125, 1.0,		/* t=13/16 */
		0.421875      , 0.5625    , 0.75  , 1.0,		/* t=12/16 */
		0.324951171875, 0.47265625, 0.6875, 1.0,		/* t=11/16 */
		0.244140625   , 0.390625  , 0.625 , 1.0,		/* t=10/16 */
		0.177978515625, 0.31640625, 0.5625, 1.0,		/* t= 9/16 */
		0.125         , 0.25      , 0.5   , 1.0,		/* t= 8/16 */
		0.083740234375, 0.19140625, 0.4375, 1.0,		/* t= 7/16 */
		0.052734375   , 0.140625  , 0.375 , 1.0,		/* t= 6/16 */
		0.030517578125, 0.09765625, 0.3125, 1.0,		/* t= 5/16 */
		0.015625      , 0.0625    , 0.25  , 1.0,		/* t= 4/16 */
		0.006591796875, 0.03515625, 0.1875, 1.0,		/* t= 3/16 */
		0.001953125   , 0.015625  , 0.125 , 1.0,		/* t= 2/16 */
		0.000244140625, 0.00390625, 0.0625, 1.0,		/* t= 1/16 */
		0.0           , 0.0       , 0.0   , 1.0,		/* t= 0/16 */
	};
	int			i, j, ts, tt ;
	DG_PATCH_VERT	*v0, *v1, *v2, *v3 ;
	FVECTOR		pv0, pv1, pv2, pv3 ;
	FVECTOR		p0, p1, dp0, dp1, *t0, *t1 ;
	FMATRIX		t0_interp_mat, t1_interp_mat, s0_interp_mat, s1_interp_mat ;
	FVECTOR		pos ;

	{/* 相対座標取得 */
		FMATRIX	inv_mat ;
		pos = *org_pos ;
		pos.vw = 1.0f ;
		_sceVu0InversMatrix( &inv_mat, world );
		_sceVu0ApplyMatrix( &pos, &inv_mat, &pos );
	}
	for ( i = 0 ; i < ( MESH_Z - 1 ) ; i++ ){
		v0 = &wverts[ parts->v_index[0] ] ;
		v1 = &wverts[ parts->v_index[1] ] ;
		v2 = &wverts[ parts->v_index[2] ] ;
		v3 = &wverts[ parts->v_index[3] ] ;
		for ( j = 0 ; j < ( MESH_X - 1 ) ; parts++, v0++, v1++, v2++, v3++, j++ ){
			/* バウンディングチェック */
			if ( parts->max.vx >= pos.vx /* && parts->max.vy > pos.vy */ && parts->max.vz >= pos.vz &&
				parts->min.vx <= pos.vx /* && parts->min.vy < pos.vy */ && parts->min.vz <= pos.vz ){
				/* ある程度の粗さで分割した各微少面毎に判定を行なう */
#if 0
				for ( tt = 0 ; tt <= 16 ; tt+=4 ){
					s0_interp_mat = s1_interp_mat ;
					MakeHermiteMatrix( &t0_interp_mat, &v0->pos, &v2->pos, &v0->pos_dt, &v2->pos_dt );
					MakeHermiteMatrix( &t1_interp_mat, &v1->pos, &v3->pos, &v1->pos_dt, &v3->pos_dt );
					t0 = &t_vec[ tt ] ;
					_sceVu0ApplyMatrix( &p0, &t0_interp_mat, t0 );/* エルミート補間 */
					_sceVu0ApplyMatrix( &p1, &t1_interp_mat, t0 );/* エルミート補間 */
					LerpVecFpu( &dp0, &v0->pos_ds, &v2->pos_ds, t0->vz );
					LerpVecFpu( &dp1, &v1->pos_ds, &v3->pos_ds, t0->vz );
					MakeHermiteMatrix( &s1_interp_mat, &p0, &p1, &dp0, &dp1 );
					if ( tt == 0 ) continue ;
					for ( ts = 0 ; ts <= 16 ; ts+=4 ){
						float	f ;
						FVECTOR	l0, l1, n ;
						t1 = &t_vec[ ts ] ;
						pv0 = pv1 ;
						pv2 = pv3 ;
						_sceVu0ApplyMatrix( &pv1, &s0_interp_mat, t1 );/* エルミート補間 */
						_sceVu0ApplyMatrix( &pv3, &s1_interp_mat, t1 );/* エルミート補間 */
						if ( ts == 0 ) continue ;
						/* ＸＺ平面におけるチェックを行なう（手抜き処理） */
#if 0
						_sceVu0SubVector( &l0, &pv1, &pv0 );
						_sceVu0SubVector( &l1, &pos, &pv0 );
						f = l1.vz * l0.vx - l1.vx * l0.vz ;
						if ( f < 0 ) continue ;
						_sceVu0SubVector( &l0, &pv3, &pv1 );
						_sceVu0SubVector( &l1, &pos, &pv1 );
						f = l1.vz * l0.vx - l1.vx * l0.vz ;
						if ( f < 0 ) continue ;
						_sceVu0SubVector( &l0, &pv2, &pv3 );
						_sceVu0SubVector( &l1, &pos, &pv3 );
						f = l1.vz * l0.vx - l1.vx * l0.vz ;
						if ( f < 0 ) continue ;
						_sceVu0SubVector( &l0, &pv0, &pv2 );
						_sceVu0SubVector( &l1, &pos, &pv2 );
						f = l1.vz * l0.vx - l1.vx * l0.vz ;
						if ( f < 0 ) continue ;
						_sceVu0SubVector( &n, &pv1, &pv0 );
						_sceVu0OuterProduct( &n, &n, &l0 );
						_sceVu0Normalize( &n, &n );
						f = _sceVu0InnerProduct( &n, &l1 );
#else
						asm volatile ("
							lqc2			vf3,0(%4)		# vf3 = pos
							lqc2			vf4,0(%0)		# vf4 = pv0
							lqc2			vf5,0(%1)		# vf5 = pv1
							lqc2			vf6,0(%2)		# vf6 = pv2
							lqc2			vf7,0(%3)		# vf7 = pv3
						"::"r"(&pv0),"r"(&pv1),"r"(&pv2),"r"(&pv3),"r"(&pos));
						/* 外積を用いて領域チェック（垂直方向のみ） */
						asm volatile ("
							vsub.xyz		vf8,vf5,vf4		# vf8 = pv1 - pv0 = l0
							vsub.xyz		vf9,vf3,vf4		# vf9 = pos - pv0
							vsub.xyz		vf10,vf7,vf5	# vf10 = pv3 - pv1
							vsub.xyz		vf11,vf3,vf5	# vf11 = pos - pv1
							vsub.xyz		vf12,vf6,vf7	# vf12 = pv2 - pv3
							vsub.xyz		vf13,vf3,vf7	# vf13 = pos - pv3
							vsub.xyz		vf14,vf4,vf6	# vf14 = pv0 - pv2 = l1
							vsub.xyz		vf15,vf3,vf6	# vf15 = pos - pv2 = l2
							vopmula.xyz		ACC,vf8,vf9		# 
							vopmsub.xyz		vf16,vf9,vf8	# vf16 = op( vf8, vf9 )
							vopmula.xyz		ACC,vf10,vf11	# 
							vopmsub.xyz		vf17,vf11,vf10	# vf17 = op( vf8, vf9 )
							vopmula.xyz		ACC,vf12,vf13	# 
							vopmsub.xyz		vf18,vf13,vf12	# vf18 = op( vf8, vf9 )
							vopmula.xyz		ACC,vf14,vf15	# 
							vopmsub.xyz		vf19,vf15,vf14	# vf19 = op( vf8, vf9 )
						");
						{/* 各外積値のＹの符号をチェック */
							long		flag ;
							asm volatile ("
								qmfc2	$4,vf16
								qmfc2	$5,vf17
								and		%0,$4,$5
								qmfc2	$4,vf18
								qmfc2	$5,vf19
								and		%0,%0,$4
								and		%0,%0,$5
							":"=r"(flag)::"$4","$5");
							if ( !( flag & 0x8000000000000000 ) ) continue ;
						}
						/* 法線の算出＆Ｙ位置計算 */
						asm volatile( "
							vopmula.xyz		ACC,vf8,vf14	# 
							vopmsub.xyz		vf2,vf14,vf8	# vf2 = op( l0, l1 ) = n * len
							vmul.xyz		vf3,vf2,vf2		# vf3 = n^2
							vmulax.w		ACC,vf0,vf3		# 
							vmadday.w		ACC,vf0,vf3		# 
							vmaddz.w		vf2,vf0,vf3		# vf2.w = lenlen
							vmul.xyz		vf1,vf15,vf2	# 
							vrsqrt			Q,vf0w,vf2w		# 
							vmulax.w		ACC,vf0,vf1		# 
							vmadday.w		ACC,vf0,vf1		# 
							vmaddz.w		vf2,vf0,vf1		# vf2.w = l2 * n * len
							vwaitq							# Q = 1/len
							vmulq.xyzw		vf2,vf2,Q		# 
							sqc2			vf2,0(%0)		# 
						"::"r"(&n));
						f = n.vw ;
#endif
						pos.vy -= f ;
						pos.vw = 1.0f ;
						_sceVu0ApplyMatrix( res, world, &pos );
						*norm = n ;
						//printf("wave2.c: f = %f\n", f);
						if ( f < 0 ){
							/* 水面下 */
							return ( 1 );
						} else {
							/* 水面上 */
							return ( 0 );
						}
					}
				}
#else
				{
					FVECTOR	tmp_pos, tmp_norm ;
					int		flag ;

					asm volatile ("
						lqc2			vf3,0(%0)
						lqc2			vf4,0(%1)
						lqc2			vf5,0(%2)
						lqc2			vf6,0(%3)
						lqc2			vf7,0(%4)
						lqc2			vf8,16(%1)
						lqc2			vf9,16(%2)
						lqc2			vf10,16(%3)
						lqc2			vf11,16(%4)
						lqc2			vf12,32(%1)
						lqc2			vf13,32(%2)
						lqc2			vf14,32(%3)
						lqc2			vf15,32(%4)
						vcallms			0x00			# ＶＵ０マイクロコードで判定を行う
					"::"r"(&pos),"r"(v0),"r"(v1),"r"(v2),"r"(v3) );

					GV_PREFECH( parts + 1 );

					asm volatile ("
						cfc2.i			$4,$15
						sqc2			vf3,0(%1)
						sqc2			vf2,0(%2)
						move			%0,$4
					":"=r"(flag):"r"(&tmp_pos),"r"(&tmp_norm):"$4" );

					/* 終了チェック */
					if ( flag ){
						tmp_pos.vw = 1.0f ;
						_sceVu0ApplyMatrix( res, world, &tmp_pos );
						*norm = tmp_norm ;
						if ( pos.vy < tmp_pos.vy ) return ( 0 );	/* 水面上 */
						return ( 1 );								/* 水面下 */
					} else {
					}
				}
#endif
			}
		}
	}
	printf("wave2.c:region error\n");
	return ( 0 );
}

/* ---------------------------------------------------------------- */
int TAKABE_GetSeaSurfaceHeight( FVECTOR *pos, FVECTOR *norm, int n_pos )
{
	Work		*work = work_ptr ;
	DG_PATCH_VERT	*verts, *last_addr = NULL ;
	int			ret, i, j, k ;

	if ( work_ptr == NULL ) return ( 0 );

	LoadVu0MicroProgram();
	for ( k = 0 ; k < n_pos ; k++, pos++, norm++ ){
		for ( i = 0 ; i < PATCH_Z ; i++ ){
			for ( j = 0 ; j < PATCH_X ; j++ ){
				WaveParts	*wave ;
				float		x, z ;

				wave = &work->w_parts[ j + i * PATCH_X ] ;
				x = DG_FABS( pos->vx - wave->offset.vx );
				z = DG_FABS( pos->vz - wave->offset.vz );
				if ( x > MESH_WIDTH_X * MESH_DIV_X * 0.5f || z > MESH_WIDTH_Z * MESH_DIV_Z * 0.5f ) continue ;

#if 0
				/* 必要であれば頂点データをスクラッチパッドへコピー */
				verts = wave->patch[0]->verts[ wave->patch[0]->buffer_clock ] ;
				if ( last_addr != verts ){
					last_addr = verts ;
					FlushCache( 0 );
					DG_StartMemToSpr( SCRPAD_ADDR, verts, SIZEOF_QWORD(DG_PATCH_VERT) * MESH_X * MESH_Z );
					DG_EndMemToSpr();
				}

				ret = GetWaveHeight( pos, norm, &wave->patch[0]->world,
									wave->patch[0]->parts,
									SCRPAD_ADDR,
									pos );
#else
				verts = wave->patch[0]->verts[ wave->patch[0]->buffer_clock ] ;
				ret = GetWaveHeight( pos, norm, &wave->patch[0]->world,
									wave->patch[0]->parts,
									verts,
									pos );
#endif
				norm->vw = ret ;
				i = PATCH_Z ;
				j = PATCH_X ;
			}
		}
	}
}


int TAKABE_AddWaveForce( FVECTOR *pos, float drop_force )
{
	Work		*work = work_ptr ;

	if ( work_ptr == NULL ) return ( 0 );

	DropWave( work, pos, drop_force );
}

int TAKABE_RiseWave( CONFLICT_CONTROL *cf_ctrl )
{
	Work		*work = work_ptr ;

	if ( work_ptr == NULL ) return ( 0 );

	return RiseWave( work, cf_ctrl );
}

void TAKABE_GetWaveCenter( FVECTOR *wave_center )
{
	Work		*work = work_ptr ;

	if ( work_ptr == NULL ) return ( 0 );

	*wave_center = work->center ;
}


/* ---------------------------------------------------------------- */
/* 波を盛り上げる */
/* とっても遅い。デモ用 */
static int RiseWaveEx( Work *work, FVECTOR *org_start, FVECTOR *org_end, float range, int flag )
{
	int		i, j, x, z, new_flag ;
	float	r_range, rangerange4 ;
	/* 線分関連パラメータ */
	FVECTOR	start, end, dir ;
	float	dir_len, base_offset ;

	r_range = 1.0f / range ;
	rangerange4 = range * range * 2.0f * 2.0f ;
	for ( i = 0 ; i < PATCH_Z ; i++ ){
		for ( j = 0 ; j < PATCH_X ; j++ ){
			WaveParts	*wave, *link ;
			DG_PATCH_VERT	*verts ;
			FVECTOR	pos ;

			wave = &work->w_parts[ j + i * PATCH_X ] ;

			{/* 相対座標取得 */
				FMATRIX	inv_mat ;
				start = *org_start ;
				end = *org_end ;
				start.vw = 1.0f ;
				end.vw = 1.0f ;
				_sceVu0InversMatrix( &inv_mat, &wave->patch[0]->world );
				_sceVu0ApplyMatrix( &start, &inv_mat, &start );
				_sceVu0ApplyMatrix( &end, &inv_mat, &end );
			}
			/* 線分のパラメータを計算する */
			_sceVu0SubVector( &dir, &end, &start );
			dir_len = DG_SQRT( _sceVu0InnerProduct( &dir, &dir ) );
			_sceVu0ScaleVector( &dir, &dir, 1.0f / dir_len );
			base_offset = _sceVu0InnerProduct( &start, &dir );

			/* 各頂点とのあたり判定 */
			for ( z = 0 ; z < MESH_DIV_Z ; z++ ){
				verts = wave->patch[0]->verts[ wave->patch[0]->buffer_clock ] ;
				verts += z * MESH_X ;
				for ( x = 0 ; x < MESH_DIV_X ; verts++, x++ ){
					FVECTOR		vec, near_pos ;
					float		height, len, sq ;
					int			offset ;
					offset = x + z * MESH_DIV_X ;

					/* 最近点の球の座標を求める */
					len = _sceVu0InnerProduct( &verts->pos, &dir );
					if ( len < base_offset ){
						near_pos = start ;
					} else if ( len > ( base_offset + dir_len ) ){
						near_pos = end ;
					} else {
						_sceVu0ScaleVector( &near_pos, &dir, len - base_offset );
						_sceVu0AddVector( &near_pos, &near_pos, &start );
					}
					
					vec.vx = verts->pos.vx - near_pos.vx ;
					vec.vz = verts->pos.vz - near_pos.vz ;
					sq = vec.vx * vec.vx + vec.vz * vec.vz ;
					if ( sq > ( rangerange4 ) ) continue ;

					//vec.vy = wave->height[ offset ] - near_pos.vy ;
					len = sq * r_range ;

					/* 頂点が有効範囲内だった場合 */
					if ( flag == 0 ){
						/* 対象物が水面下の場合 */
						height = near_pos.vy - len ;	/* その地点のあたり球の高さを求める */
						if ( height > wave->height[ offset ] ){
							wave->force[ offset ] = 0 ;
							wave->height[ offset ] = height ;
						}
						
					} else {
						/* 対象物が水面上の場合 */
						height = near_pos.vy + len ;	/* その地点のあたり球の高さを求める */
						if ( height < wave->height[ offset ] ){
							wave->force[ offset ] = 0 ;
							wave->height[ offset ] = height ;
						}
						
					}
				}
			}
		}
	}

	return ( 0 );
}

int TAKABE_RiseWaveEx( FVECTOR *start, FVECTOR *end, float range, int flag )
{
	Work		*work = work_ptr ;

	if ( work_ptr == NULL ) return ( 0 );

	return RiseWaveEx( work, start, end, range, flag );
}

#endif
