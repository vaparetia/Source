/*
	patch.h
	パッチ曲面オブジェクト用ヘッダ

	2000/07/17 K.Takabe
	$Id: patch.h,v 1.1.1.3 2002/11/19 11:42:17 Yoshizawa1 Exp $

*/


#ifndef __PATCH_H__
#define __PATCH_H__

#ifdef __cplusplus
extern "C" {
#endif



/* ---------------------------------------------------------------- */
typedef struct _dg_patch_vert{
	FVECTOR		pos ;			/* 頂点座標 */
	FVECTOR		pos_ds ;		/* 頂点座標Ｓ軸方向変化量 */
	FVECTOR		pos_dt ;		/* 頂点座標Ｔ軸方向変化量 */
	FVECTOR		uv ;			/* テクスチャ座標 */
} DG_PATCH_VERT ;

typedef struct _dg_patch_parts{
	int				v_index[4] ;	/* 参照頂点へのインデックス */
	int				parts_index[4] ;	/* 隣接するパッチ曲面パーツのインデックス */
	FVECTOR			max, min ;		/* バウンディングチェックサイズ */
	int				flag ;			/* 処理フラグ */
	int				lod_level ;		/*  */
	int				edge_flag ;		/* エッジフラグ */
	int				pad[1] ;		/*  */
} DG_PATCH_PARTS ;

typedef struct _dg_patch {
	FMATRIX			world ;				/* ワールドマトリクス */
	FMATRIX			*root ;				/* 付随マトリクス */
	FMATRIX			*light ;			/* 光源マトリクス */
	int				flag ;				/* 各種フラグ */
	int				n_verts ;			/* 管理頂点数 */
	int				n_patch ;			/* オブジェクト表示数 */
	int				group_id ;			/* グループＩＤ */
	int				chanl ;				/* 処理チャンネル */
	int				tri_id ;			/* 使用テクスチャ */
	float			lod_z_bias ;		/* ＬＯＤ補正距離 */
	int				lod_level_bias ;	/* ＬＯＤ補正レベル */
	int				buffer_clock ;		/* 頂点ダブルバッファ切り替えクロック */
	DG_TEX			*tex ;				/* テクスチャ構造体へのポインタ */
	DG_PATCH_VERT	*verts[2] ;			/* 共有頂点データ */
	DG_PATCH_PARTS	*parts ;			/* パッチ曲面データ配列 */
	DG_MIPMAP		*mipmap ;			/* ミップマップ構造体 */
	FVECTOR			max,min ;			/* 全体バウンディング（未使用） */
	FMATRIX			envmap_correct ;	/* 環境マップ補正マトリクス */
} DG_PATCH ;


/* DG_PATCH.flag 用フラグ */
enum {
	DG_PATCH_INVISIBLE			= 0x0003,		/* 不可視属性（全チャンネル） */
	DG_PATCH_INVISIBLE0			= 0x0001,		/* 不可視属性（チャンネル０のみ） */
	DG_PATCH_INVISIBLE1			= 0x0002,		/* 不可視属性（チャンネル１のみ） */
	DG_PATCH_INVISIBLE2			= 0x0004,		/* 不可視属性（チャンネル２のみ） */
	DG_PATCH_INVISIBLE3			= 0x0008,		/* 不可視属性（チャンネル３のみ） */
	DG_PATCH_SINGLE				= 0x0010,		/* シングルバッファモード（静止オブジェクト用）＜未実装＞ */
	DG_PATCH_NOCHECK			= 0x0020,		/* 各パッチ曲面のＬＯＤ、バウンディングチェック */
												/* （他のパッチ曲面オブジェクトと共有する場合に使用する） */
	DG_PATCH_FOGBLACK			= 0x0040,		/* 強制的にフォグカラーを黒に（加算半透明テクスチャに有効） */
	DG_PATCH_SEMITRANS			= 0x0080,		/* 強制５０％半透明 */
	DG_PATCH_DUMMY				= 0x0000,		/*  */

	DG_PATCH_TYPEMASK			= 0xff00,		/*  */
	DG_PATCH_NORMAL				= 0x0000,		/* 通常曲面サーフェス */
 	DG_PATCH_REFLECTPLANE		= 0x0100,		/* 反射平面型（Ｙ軸０固定・水面反射エミュレーション） */
	DG_PATCH_BUMPMAP			= 0x0200,		/* ＸＺ軸固定ＵＶ摂動バンプマップ曲面サーフェス */
};

/* ---------------------------------------------------------------- */
extern void DG_AddPluginPatch( void );
extern void DG_DeletePluginPatch( void );
extern void DG_AddPluginPatchTrans( void );
extern void DG_DeletePluginPatchTrans( void );
extern void DG_QueuePatchObjs( DG_PATCH *patch );
extern void DG_DequeuePatchObjs( DG_PATCH *patch );
extern DG_PATCH* DG_MakePatch( int flag, int n_patch, int n_verts );
extern void DG_FreePatch( DG_PATCH *patch );

extern DG_PATCH* DG_MakePatchMesh( int flag, int n_vert_s, int n_verts_t );
extern void DG_ConfigPatchLOD( DG_PATCH *patch, int level );
extern void DG_SetupPatchMeshBounding( DG_PATCH *patch );



/* ---------------------------------------------------------------- */
	/*
		頂点バッファ切り替え
	*/
static	inline	void	DG_SwitchBuffPatch( DG_PATCH *patch )
{
   if (!DG_Arm_SkipThisFrame())
   {
   	patch->buffer_clock = 1 - patch->buffer_clock ;
   }
}



#ifdef __cplusplus
}
#endif


#endif
