//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	huge_sea.c
	巨大海面オブジェクト

	2001/05/24 K.Takabe
	$Id: huge_sea.c,v 1.1.1.3 2002/11/19 11:51:14 Yoshizawa1 Exp $

*/
/*

chara 巨大海面[NewHugeSeaSurfaceSet] $s:name \
	-mipmap $b:最大ＬＯＤレベル $i:基準Ｚ値 $b:ＬＯＤ変化補正 \
	-tex $s:ＴＲＩファイル名 $s:テクスチャ名 ...1 \	// テクスチャ名は指定したＬＯＤレベル分必要！
	-pos $v:中心座標 \
	-size $i:基本サイズＸ $i:基本サイズＺ \	// 省略すると400000 400000
	-offset $w:パッチ曲面用ＬＯＤオフセット \	// -2~+2ぐらいの範囲で指定すること
	-light_enable \	// ステージライト反映
	-flag $w:フラグ
mesg 巨大海面 $s:名前 表示状態[2] $b:ＯＮ・ＯＦＦ
mesg 巨大海面 $s:名前 表示位置移動[256] $v:座標
// テクスチャには手動で変換を行ったTRIファイルが必要なので注意（dpackでは構築できない）
// ミップマップテクスチャは256x256以下のサイズで、基本サイズに対して１６回リピートして
// 貼り付けられる。
// 従って256x256のテクスチャがNxN四方にマッピングされる場合には基準Ｚ値2N、
// 基本サイズ16N,16Nとなるような値を設定するのが望ましい。

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
/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#define _RND(n)	( ( (BP_PS2_rand()>>16) * n ) >> 15 )

/* ---------------------------------------------------------------- */
	/*
		プログラム使用定数の定義
	*/

#define	MESH_X	( 16 )				/* Ｘ頂点数（ 4 < MESH_X*MESH_Z < 256 ） */
#define MESH_Z	( 16 )				/* Ｚ頂点数（ 4 < MESH_X*MESH_Z < 256 ） */
#define	MESH_DIV_X	( MESH_X - 1 )	/* パッチの分割数（Ｘ軸方向） */
#define MESH_DIV_Z	( MESH_Z - 1 )	/* パッチの分割数（Ｚ軸方向） */
#define MESH_WIDTH_X	( work->mesh_size_x )	/* メッシュ間隔（Ｘ軸方向） */
#define MESH_WIDTH_Z	( work->mesh_size_z )	/* メッシュ間隔（Ｚ軸方向） */
#define PATCH_X	(1)					/* パッチ配置数 */
#define PATCH_Z	(1)					/* パッチ配置数 */

#define MAX_PLANE	(1)

#define MAX_PARAM_INDEX	(64)

#define TRI_ID	(GV_StrCode("wave"))
#define TEX_ID	(GV_StrCode("wave06"))

/* ---------------------------------------------------------------- */
/* 波ブロック管理構造体 */
typedef ALIGN16_DECL(struct) _wave_perts{
	/* 表示管理用 */
	FVECTOR		offset ;
	DG_PATCH	*patch[MAX_PLANE] ;
} WaveParts ;

/* ---------------------------------------------------------------- */

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
	int			visible_mode ;
	int			tri_id ;
	int			max_mip_level ;
	int			mip_base_z ;
	int			mip_lod_l ;
	int			tex_id[MAX_PLANE][8] ;
	DG_TEX		*tex[MAX_PLANE] ;
#ifdef PSX2
	DG_MIPMAP	mipmap ;
#endif
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

	int			lod_offset ;					/* パッチ曲面ＬＯＤオフセット */

	WaveParts	w_parts[ PATCH_X * PATCH_Z ];
	float		ave_height ;

	int				debug_count ;

} Work ;

//static Work		*work_ptr = NULL ;

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

	patch = DG_MakePatch( 0, MESH_DIV_X * MESH_DIV_Z, MESH_DIV_X * MESH_DIV_Z * 4 );
	if ( patch == NULL ) return ( NULL );

	for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
		for ( j = 0 ; j < MESH_DIV_X ; j++ ){
			int		parts_index, verts_index ;
			parts_index = i * MESH_DIV_Z + j ;
			parts = &patch->parts[ parts_index ] ;
			/* 頂点インデックス設定 */
			verts_index = parts_index * 4 ;
			parts->v_index[ 0 ] = verts_index + 0 ;
			parts->v_index[ 1 ] = verts_index + 1 ;
			parts->v_index[ 2 ] = verts_index + 2 ;
			parts->v_index[ 3 ] = verts_index + 3 ;
			/* その他設定 */
			parts->flag = 0 ;
		}
	}

	return ( patch );

}

/* 波ブロックの初期化 */
static void InitWaveParts( Work *work, WaveParts *wave, int s, int t )
{
	DG_PATCH		*patch = NULL ;
	DG_PATCH_VERT	*verts ;
	int			i, j, k, l ;
	float		u_scale, v_scale ;

	/* 各頂点パラメータなどの初期化 */
	/* オブジェクトの初期化 */
	for ( i = 0 ; i < work->max_patch_plane ; i++ ){
		patch = MakePatchMeshEx( work, s, t );
		//patch = DG_MakePatchMesh( 0/*DG_PATCH_REFLECTPLANE*/, MESH_X, MESH_Z );
		switch ( i ){
		  case 0:
			wave->patch[i] = patch ;
			break ;
		}
		if ( work->alpha_blend_flag ) patch->flag |= DG_PATCH_SEMITRANS ;
		patch->light = work->light ;
		GM_GroupObject( patch, work->map );
		DG_QueuePatchObjs( patch );
		//DG_ConfigPatchLOD( patch, 13 );
		{
			int	lod_level ;
			lod_level = logf( MESH_WIDTH_X ) / logf( 2.0f );
			lod_level += work->lod_offset ;
			DG_ConfigPatchLOD( patch, lod_level );
		}
		patch->tri_id = work->tri_id ;
		patch->tex = work->tex[i] ;
#ifdef PSX2
		patch->mipmap = &work->mipmap ;
#endif
	}
	/* スクラッチパッド上で一度構築してから転送 */
	u_scale = work->tex[0]->u_scale ;
	v_scale = work->tex[0]->v_scale ;
	for ( l = 0 ; l < 2 ; l++ ){
		verts = patch->verts[ l ] ;
		for ( i = 0 ; i < MESH_DIV_Z ; i++ ){
			for ( j = 0 ; j < MESH_DIV_X ; j++ ){
				{/* 頂点座標設定 */
					float	base_x, base_z ;
					base_x = j * MESH_WIDTH_X - ( MESH_DIV_X * MESH_WIDTH_X ) / 2 ;
					base_z = i * MESH_WIDTH_Z - ( MESH_DIV_Z * MESH_WIDTH_Z ) / 2 ;
					verts[0].pos.vx = base_x ;
					verts[0].pos.vy = 0.0f ;
					verts[0].pos.vz = base_z ;
					verts[0].pos.vw = 1.0f ;
					verts[1].pos.vx = base_x + MESH_WIDTH_X ;
					verts[1].pos.vy = 0.0f ;
					verts[1].pos.vz = base_z ;
					verts[1].pos.vw = 1.0f ;
					verts[2].pos.vx = base_x ;
					verts[2].pos.vy = 0.0f ;
					verts[2].pos.vz = base_z + MESH_WIDTH_Z ;
					verts[2].pos.vw = 1.0f ;
					verts[3].pos.vx = base_x + MESH_WIDTH_X ;
					verts[3].pos.vy = 0.0f ;
					verts[3].pos.vz = base_z + MESH_WIDTH_Z ;
					verts[3].pos.vw = 1.0f ;
				}
				/* 頂点変化パラメータ設定 */
				for ( k = 0 ; k < 4 ; k++ ){
					verts[ k ].pos_ds.vx = MESH_WIDTH_X ;
					verts[ k ].pos_ds.vy = 0.0f ;
					verts[ k ].pos_ds.vz = 0.0f ;
					verts[ k ].pos_ds.vw = 0.0f ;
					verts[ k ].pos_dt.vx = 0.0f ;
					verts[ k ].pos_dt.vy = 0.0f ;
					verts[ k ].pos_dt.vz = MESH_WIDTH_Z ;
					verts[ k ].pos_dt.vw = 0.0f ;
				}
				/* ＵＶ値初期化 */
				verts[0].uv.vx = -8.0f * u_scale ;
				verts[0].uv.vy = -8.0f * v_scale ;
				verts[0].uv.vz = 1.0f ;
				verts[0].uv.vw = 128.0f ;
				verts[1].uv.vx = 8.0f * u_scale ;
				verts[1].uv.vy = -8.0f * v_scale ;
				verts[1].uv.vz = 1.0f ;
				verts[1].uv.vw = 128.0f ;
				verts[2].uv.vx = -8.0f * u_scale ;
				verts[2].uv.vy = 8.0f * v_scale ;
				verts[2].uv.vz = 1.0f ;
				verts[2].uv.vw = 128.0f ;
				verts[3].uv.vx = 8.0f * u_scale ;
				verts[3].uv.vy = 8.0f * v_scale ;
				verts[3].uv.vz = 1.0f ;
				verts[3].uv.vw = 128.0f ;
				verts += 4 ;
			}
		}
	}
	for ( i = 0 ; i < work->max_patch_plane ; i++ ){
		patch = wave->patch[i] ;
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
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
	/*
		プログラム使用サブルーチン
	*/

/* テクスチャ情報をリージョンリピートテクスチャに設定する */
static void TextureRepeatSet( DG_TEX *tex )
{
#ifdef PSX2
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
#endif
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

/* ---------------------------------------------------------------- */
	/*
		プログラムメイン処理
	*/
static void Act( Work *work )
{
	GV_MSG		*msg ;
	int			n_msg, i, j, k ;
	int			move_flag = 0 ;

	//AN_Test_Eye2( &DG_ZeroVector, 3 );

	/* メッセージチェック */
	if ( ( n_msg = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
		for ( ; n_msg > 0 ; n_msg--, msg++ ){
			switch ( msg->message[0] ){
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
			  case 256:/* 表示座標の移動 */
				work->center.vx = msg->message[1] ;
				work->center.vy = msg->message[2] ;
				work->center.vz = msg->message[3] ;
				move_flag = 1 ;
				break ;
			}
		}
	}

	/* 表示位置変更 */
	if ( move_flag ){
		for ( i = 0 ; i < PATCH_Z ; i++ ){
			for ( j = 0 ; j < PATCH_X ; j++ ){
				WaveParts	*wave ;
				wave = &work->w_parts[ j + i * PATCH_X ] ;
				wave->offset.vx = work->center.vx + (float)( MESH_WIDTH_X * MESH_DIV_X ) * ( j - PATCH_X * 0.5f + 0.5f ) ;
				//wave->offset.vy = GM_WaterLevel ;	/* これは使ってないので意味なし */
				wave->offset.vz = work->center.vz + (float)( MESH_WIDTH_Z * MESH_DIV_Z ) * ( i - PATCH_Z * 0.5f + 0.5f ) ;
				for ( k = 0 ; k < work->max_patch_plane ; k++ ){
					wave->patch[k]->world.m[3][0] = wave->offset.vx ;
					wave->patch[k]->world.m[3][1] = wave->offset.vy ;
					wave->patch[k]->world.m[3][2] = wave->offset.vz ;
				}
			}
		}
	}

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

#ifdef DEBUG_MODE
#if 0
	{
		static int	data[3], flag ;
		if ( GV_PadData[0].press & PAD_A ){
			printf("%s:%08x\n", __FILE__, data );
		}
		if ( flag == 0 ){
			data[0] = work->mipmap.max_level ;
			data[1] = work->mipmap.z0 ;
			data[2] = work->mipmap.lod_l ;
			flag = 1 ;
		}
		work->mipmap.max_level = data[0] ;
		work->mipmap.z0 = data[1] ;
		work->mipmap.lod_l = data[2] ;
	}
#endif
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

	for ( i = 0 ; i < PATCH_Z ; i++ ){
		for ( j = 0 ; j < PATCH_X ; j++ ){
			WaveParts	*wave ;
			wave = &work->w_parts[ j + i * PATCH_X ] ;
			FreeWaveParts( wave, work->max_patch_plane );
		}
	}
	//work_ptr = NULL ;
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
	work->mesh_size_x = 400000 ;
	work->mesh_size_z = 400000 ;
	work->max_patch_plane = 1 ;

	if ( GCL_GetOption( 'f' ) != NULL ){
		work->flag = GCL_GetNextInt() ;
	}

	if ( GCL_GetOption( 'm' ) != NULL ){
		work->max_mip_level = GCL_GetNextInt() ;
		work->mip_base_z = GCL_GetNextInt() ;
		work->mip_lod_l = GCL_GetNextInt() ;
	}
#ifdef PSX2
	work->mipmap.max_level = work->max_mip_level ;
	work->mipmap.z0 = work->mip_base_z ;
	work->mipmap.lod_l = work->mip_lod_l ;
	//work->mipmap.z0 = (float)MESH_WIDTH_X * (float)MESH_DIV_X / 16.0f * 256 / 512 ;
	work->mipmap.lod_l = 0 ;
#endif

	if ( GCL_GetOption( 's' ) != NULL ){
		work->mesh_size_x = GCL_GetNextInt() ;
		work->mesh_size_z = GCL_GetNextInt() ;
	}

	if ( GCL_GetOption( 'o' ) != NULL ){
		work->lod_offset = GCL_GetNextInt() ;
	}

	if ( GCL_GetOption( 'l' ) != NULL ){
		work->light_enable = 1 ;
		//DG_GetLightMatrix( &DG_ZeroVector, work->light );
		work->light[0] = DG_LightMatrix ;
		work->light[1] = DG_ColorMatrix ;
	}

	/* テクスチャ設定読み込み */
	if ( GCL_GetOption( 't' ) != NULL ) {
#ifdef PSX2
		work->mipmap.tri_id = work->tri_id = GCL_GetNextInt() ;
		for ( i = 0 ; i <= work->max_mip_level ; i++ ){
			work->mipmap.tex_id[i] = work->tex_id[0][i] = GCL_GetNextInt() ;
		}
#else
		work->tri_id = GCL_GetNextInt() ;
		for ( i = 0 ; i <= work->max_mip_level ; i++ ){
			work->tex_id[0][i] = GCL_GetNextInt() ;
		}
#endif
	} else {
		return (-1);
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
printf("huge sea : tri_id = %d, tex_id = %d\n", work->tri_id, work->tex_id[ 0 ][ 0 ] );
	work->tex[0] = tex = DG_GetTexture2( work->tri_id, work->tex_id[0][0] );
	/* 強制的にリピートテクスチャにする */
	TextureRepeatSet( tex );
	//tex->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 2, 1, 128 );
	//tex->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 2, 2, 2, 0, 128 );
	//work->local_tex = *tex ;
	//work->local_tex.tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 2, 1, 64 );
	//work->local_tex.tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 64 );
	//work->local_tex.tex_trans.alpha.data = SCE_GS_SET_ALPHA( 2, 0, 2, 1, 48 );
#if 0
	work->mipmap.max_level = 6 ;
	work->mipmap.tri_id = work->tri_id ;
	work->mipmap.tex_id[0] = work->tex_id[0] ;
	work->mipmap.tex_id[1] = GV_StrCode("oil_alp_ovl_1") ;
	work->mipmap.tex_id[2] = GV_StrCode("oil_alp_ovl_2") ;
	work->mipmap.tex_id[3] = GV_StrCode("oil_alp_ovl_3") ;
	work->mipmap.tex_id[4] = GV_StrCode("oil_alp_ovl_4") ;
	work->mipmap.tex_id[5] = GV_StrCode("oil_alp_ovl_5") ;
	work->mipmap.tex_id[6] = GV_StrCode("oil_alp_ovl_6") ;
	work->mipmap.z0 = (float)MESH_WIDTH_X * (float)MESH_DIV_X / 16.0f * 256 / 512 ;
	work->mipmap.lod_l = 0 ;
#endif
#ifdef PSX2
	DG_MakeMipmap( &work->mipmap );
#endif

	//work->tex[1] = tex = DG_GetTexture2( work->tri_id, work->tex_id[1] );
	/* 強制的にリピートテクスチャにする */
	//TextureRepeatSet( tex );
	//tex->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 2, 1, 128 );
	//tex->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 1, 1, 128 );
	/*tex->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 1, 1, 128 );*/
	//tex->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 2, 1, 48 );
	//work->local_tex = *tex ;
	//work->local_tex.tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 64 );


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
			
		}
	}

	return (0);
}


/* ---------------------------------------------------------------- */
	/*
		シナリオ起動インターフェイス
	*/
void *NewHugeSeaSurfaceSet( int name, int where )
{
	Work		*work ;

	//if ( work_ptr != NULL ) return ( 0 );

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	//work_ptr = work ;
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


/* ---------------------------------------------------------------- */
