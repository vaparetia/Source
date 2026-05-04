//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xshdwchn.c
	チャンネル処理ユニット／影オブジェクトＤＭＡ接続ルーチン

	2002/04/02 K.Takabe
	$Id: xshdwchn.c,v 1.1.1.3 2002/11/19 11:42:37 Yoshizawa1 Exp $

*/
/*

	void		DG_ShadowChainChanl( DG_CHANL *cp, int which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	キューされた各オブジェクトをＤＭＡパケットに接続する


*/

#ifdef KP_XBOX //BP

#ifdef KP_XBOX
#include <xtl.h>
//BP_RENDER #include <xgraphics.h>
#else
#include <d3dx8.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include	"libgv.h"
#include	"libdg.h"
#include	"libdg.cnf"

#include	"def_dma.h"

#include	"shader.h"

/*----------------------------------------------------------------*/


/*----------------------------------------------------------------*/
	/*
	*/

/* スポットライトのベーステクスチャイメージへのポインタ */
void	*DG_SpotLightBaseImage = NULL ;


extern LPDIRECT3DTEXTURE9	DG_BackBufferTexture[3] ;		/* バックバッファテクスチャ */
extern LPDIRECT3DSURFACE9	DG_BackBufferSurface[3] ;		/* バックバッファサーフェス */
extern LPDIRECT3DSURFACE9	DG_DepthBufferSurface ;			/* Ｚバッファサーフェス */
extern D3DVIEWPORT9			DG_CurrentViewport ;
extern LPDIRECT3DTEXTURE9	DG_ShadowBackBufferTexture ;	/* 影用バックバッファテクスチャ */
extern LPDIRECT3DSURFACE9	DG_ShadowBackBufferSurface ;	/* 影用バックバッファサーフェス */
extern LPDIRECT3DSURFACE9	DG_ShadowDepthBufferSurface ;	/* 影用Ｚバッファサーフェス */

/*----------------------------------------------------------------*/


/*----------------------------------------------------------------*/
	/*
		各種ローカル構造体定義
	*/


/*----------------------------------------------------------------*/
typedef struct {
	DG_OBJS		*objs ;
	int			flag ;
	int			shadow_id ;
	int			bound_mode ;
	int			group_id ;
} OBJ_LIST ;

typedef struct {
	/* オブジェクト表示用 */
	int			buffer_switch ;
	int			pad0[3] ;
	/* ＤＭＡ生成用バッファ */
	u_long128		dma_buffer[128] ;	/* 汎用ＤＭＡパケット生成バッファ */

	/* 各種処理用 */
	FMATRIX		eye_pers ;
	FMATRIX		eye_pers2 ;
	FMATRIX		light_view ;
	FMATRIX		light_view2 ;
	FMATRIX		light_trans ;
	FMATRIX		light_pers ;
	FMATRIX		light_pers2 ;
	FVECTOR		light_vector ;
	FVECTOR		color_vector ;
	FVECTOR		light_pos ;				/* 光源位置（ＸＢＯＸ拡張） */
	FMATRIX		raise_eye_pers ;		/* 優先上げマトリクス（クリップオブジェクト用） */
	FMATRIX		raise_eye_pers2 ;		/* 優先上げマトリクス（非クリップオブジェクト用） */
	FMATRIX		root ;
	FMATRIX		tmp_mat ;
	FVECTOR		tmp_vec ;
	FVECTOR		verts[16] ;
	int			bound_mode ;
	float		fog_param1 ;
	float		fog_param2 ;
	float		far_range ;				/* 減衰の最大距離 */
	int			invisible_flag ;
	int			pad[2] ;

	/* ChainObj()関数固有ローカル変数 */
	void		*matrix_addr ;

	/* オブジェクト検索用リスト */
	int			max_list ;
	OBJ_LIST	list[128] ;

	/* 計算結果一時保存 */
	FMATRIX		screen ;				/* 表示オブジェクトの透視変換マトリクス */
	FMATRIX		local_light ;			/* 表示オブジェクトのローカルライトマトリクス */
	FMATRIX		local_color ;			/* 表示オブジェクトのローカルカラーマトリクス */
	FMATRIX		env_mat ;				/* 表示オブジェクトのエンベロープ計算マトリクス */
	int			fog ;					/* 表示オブジェクトの固定フォグ値 */
	int			local_bound_mode ;		/* 表示オブジェクトのバウンディングチェック結果 */
	int			para_mode ;				/* テクスチャの平行投影モード */
#ifdef LIBDG_PERFORMANCE
	PERFORMANCE_PACKET_INFO	projection[2] ;
	PERFORMANCE_PACKET_INFO	shadow_draw[2] ;
	int			n_packs ;
	int			n_verts ;
#endif

	/* ローカルワーク */
	u_long128	local_work[0] ;

} ScrpadWork ;

int	DG_ActiveShadowFlag = 0 ;
/*----------------------------------------------------------------*/

#ifndef	NO_PROTOTYPE
#endif

extern void DG_StartBoundingCheckSupport( void );/* screen.c */
extern int DG_WriteObjsPacketInit_WriteShadow( void *tag_addr, DG_CHANL *cp, float fog_param1, float fog_param2 );


/*----------------------------------------------------------------*/
DG_PIXELSHADER	DG_ShadowPixelShader[1] ;
extern DG_VERTEXSHADER	DG_DmapackVertexShader[1] ;
extern DG_VERTEXFORMAT	DG_DmapackVertexFormat[1] ;
DG_VERTEXSHADER	DG_ShadowVertexShader[2];
DG_VERTEXFORMAT	DG_ShadowVertexFormat[1] ;
extern unsigned char VERTEX_SHADER_shdw_mk[];	/* 影生成 */
extern unsigned char VERTEX_SHADER_shdw_wt[];	/* 影投影 */
extern unsigned char PIXEL_SHADER_shdwmap0[];	/* 影投影用 */
void DG_InitShadowVertexShader(void)
{
#if 0 //BP_RENDER
	DWORD dwObjDecl_00[] = {
		/* 通常ライティング */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT3),		/* position */
		D3DVSD_REG(1,  D3DVSDT_NORMSHORT1),	/* blend */
		D3DVSD_REG(2,  D3DVSDT_NORMSHORT3),	/* normal */
		//D3DVSD_REG(10, D3DVSDT_SHORT1),		/* dummy */
		//D3DVSD_REG(7,  D3DVSDT_NORMSHORT2),	/* uv0 */
		//D3DVSD_REG(8,  D3DVSDT_NORMSHORT2),	/* uv1 */
		//D3DVSD_REG(9,  D3DVSDT_NORMSHORT2),	/* uv2 */
		D3DVSD_END()
	};
	/* 頂点フォーマット生成 */
	DG_MakeVertexFormat( &DG_ShadowVertexFormat[ 0 ], dwObjDecl_00 );

	/* 影生成レンダリング */
	DG_MakeVertexShader( &DG_ShadowVertexShader[0], VERTEX_SHADER_shdw_mk, dwObjDecl_00 );
	/* 影投影レンダリング用 */
	DG_MakeVertexShader( &DG_ShadowVertexShader[1], VERTEX_SHADER_shdw_wt, dwObjDecl_00 );

	/* ピクセルシェーダーも生成 */
	DG_MakePixelShader( &DG_ShadowPixelShader[0], PIXEL_SHADER_shdwmap0 );
#endif
}

static void DG_InitChanlRenderState( DG_CHANL *cp, int flag )
{
	static FVECTOR VS_Const[] = {
		{0.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.5f, 0.5f, 0.5f, 0.5f},
		{1.0f/4096.0f, 1.0f/255.0f, 0, 128.0f},
	};
	static FVECTOR	ShadowShiftParam[4] = {
		{ 128.0f + 0.0f, 128.0f + 0.5f, 0, 0 },	/* シャドウマップシフト量（０度） */
		{ 128.0f - 0.25f, 128.0f + 0.433, 0, 0 },	/* シャドウマップシフト量（１２０度） */
		{ 128.0f - 0.25f, 128.0f - 0.433, 0, 0 },	/* シャドウマップシフト量（２４０度） */
		{ 1.0f/4000.0f, 0, 0, 0 }	/* シャドウフォーカス基準距離（未使用） */
	};

	DG_SetRenderState(D3DRS_FOGENABLE, 0 );

	/* 頂点シェーダー用各種定数設定 */
	DG_SetVertexShaderConstant(CV_EYEPERS, &cp->raise_eye_xpers, 4);	/* 優先上げ */
	DG_SetVertexShaderConstant(CV_PERS, &cp->raise_xpers, 4);			/* 優先上げ */

	DG_SetVertexShaderConstant(CV_ZERO, &VS_Const[0], 1);
	DG_SetVertexShaderConstant(CV_ONE, &VS_Const[1], 1);
	DG_SetVertexShaderConstant(CV_HALF, &VS_Const[2], 1);
	DG_SetVertexShaderConstant(CV_SCALE, &VS_Const[3], 1);
	DG_SetVertexShaderConstant(CV_FOG, &DG_FogParam, 1);
	DG_SetVertexShaderConstant(CV_CONST0, &ShadowShiftParam, 4 );

	/* 頂点シェーダー設定 */
	DG_ClearVertexShader();
	DG_LoadVertexShader( &DG_ShadowVertexShader[0] );
	DG_LoadVertexShader( &DG_ShadowVertexShader[1] );
	DG_LoadVertexShader( &DG_DmapackVertexShader[0] );

	/* ピクセルシェーダー設定 */
	DG_SetPixelShader( NULL );
}

/*----------------------------------------------------------------*/
/* ２Ｄ描画補助 */
static void Init2DDraw( void )
{
	static FVECTOR screen_param[2] = {
		{0,-256,1.0f,0},
		{2.0f/256.0f,-2.0f/256.0f,1,1}
	};
	static FVECTOR tex_param[2] = {
		{256.0f/16384.0f,256.0f/16384.0f,1,1},	/* テクスチャスケール */
		{0.0f,0.0f,0,0}		/* テクスチャオフセット */
	};
	DG_SetVertexShaderConstant(CV_TEX0_SCALE, tex_param, 2);
	DG_SetVertexShaderConstant(0, screen_param, 2);

	DG_SetVertexStream( NULL );
	DG_SelectVertexShader( &DG_DmapackVertexShader[0], &DG_DmapackVertexFormat[0] );

}
static void DrawSpot( u_int color )
{
#if 0 //BP_RENDER
	DG_VERTEX_DMAPACK2D		*vertex;
	int		i ;
	float	a, s, c ;

	if ( DG_SpotLightBaseImage == NULL ){
		DG_SetTexture( 0, NULL );
#if 0
		vertex = DG_AllocDynamicVertexBuffer( sizeof(DG_VERTEX_DMAPACK2D), 18 );
		DG_SetVertexFormat( &DG_DmapackVertexFormat[0] );
		vertex->x = 128 ;
		vertex->y = 128 ;
		vertex->rgba = color ;
		vertex++ ;
		for ( i = 1 ; i < 18 ; i++ ){
			a = (float)((i-1)&15) / 8.0f * (float)3.14159265f ; 
			DG_SinCos( &s, &c, a );
			vertex->x = 128.0f + c * 120.0f ;
			vertex->y = 128.0f + s * 120.0f ;
			vertex->rgba = 0x80000000 ;
			vertex++ ;
		}
		DG_DrawVertices( D3DPT_TRIANGLEFAN, 0, 18 );
#else
		vertex = DG_DrawVerticesDirect( D3DPT_TRIANGLEFAN, sizeof(DG_VERTEX_DMAPACK2D), 18 );
		vertex->x = 128 ;
		vertex->y = 128 ;
		vertex->rgba = color ;
		vertex++ ;
		for ( i = 1 ; i < 18 ; i++ ){
			a = (float)((i-1)&15) / 8.0f * (float)3.14159265f ; 
			DG_SinCos( &s, &c, a );
			vertex->x = 128.0f + c * 120.0f ;
			vertex->y = 128.0f + s * 120.0f ;
			vertex->rgba = 0x80000000 ;
			vertex++ ;
		}
		DG_DrawVerticesDirectEnd();
#endif
	} else {
		/* グラデーションサークルの代わりに背景イメージを使用する場合 */
		static D3DBaseTexture	texture ;

		XGSetTextureHeader( 256, 256, 1, 0, D3DFMT_LIN_A8R8G8B8, 0,
						   (LPDIRECT3DTEXTURE8*)&texture, 0, 256*4 );
		IDirect3DTexture8_Register( (LPDIRECT3DTEXTURE8*)&texture, DG_SpotLightBaseImage );
		DG_SetTextureDirect( 0, NULL );
		DG_SetTextureDirect( 0, &texture );

		vertex = DG_DrawVerticesDirect( D3DPT_TRIANGLESTRIP, sizeof(DG_VERTEX_DMAPACK2D), 4 );

		vertex->x = 0 ;
		vertex->y = 0 ;
		vertex->rgba = color ;
		vertex->u = 0 ;
		vertex->v = 0 ;
		vertex++ ;

		vertex->x = 256 ;
		vertex->y = 0 ;
		vertex->rgba = color ;
		vertex->u = 16384 ;
		vertex->v = 0 ;
		vertex++ ;

		vertex->x = 0 ;
		vertex->y = 256 ;
		vertex->rgba = color ;
		vertex->u = 0 ;
		vertex->v = 16384 ;
		vertex++ ;

		vertex->x = 256 ;
		vertex->y = 256 ;
		vertex->rgba = color ;
		vertex->u = 16384 ;
		vertex->v = 16384 ;
		vertex++ ;

		DG_DrawVerticesDirectEnd();
	}
#endif
}
/* ---------------------------------------------------------------- */
	/*
		パケット初期化関連
	*/



	/*
		ＤＭＡに接続する
	*/
static	void	ChainObj( DG_OBJ *obj, int type )
{
	ScrpadWork	*scrpad = SCRPAD_ADDR ;
	DG_OBJ_PACKET	*pack ;
	int			i, j ;
	unsigned short	*index_addr ;

	pack = obj->packets ;
	if ( pack == NULL ) return ;

	DG_SetVertexBuffer( 0, obj->vbuff, obj->stride );
	DG_SetVertexStream( &DG_ShadowVertexFormat[ 0 ] );
	DG_SelectVertexShader( &DG_ShadowVertexShader[type], &DG_ShadowVertexFormat[ 0 ] );

	/* マトリクスの設定 */
	DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
	if (obj->parent != -1) {
		DG_SetVertexShaderConstant(CV_WORLD1, &obj->inv_mat, 4);
	}
	DG_SetVertexShaderConstant(CV_LIGHTVEC, &scrpad->local_light, 4);
	DG_SetVertexShaderConstant(CV_LIGHTCOL, &scrpad->local_color, 4);

	/* モデルの描画処理 */
	index_addr = obj->index ;
	for ( j = obj->n_packs; j > 0; j --, pack++ ){
		if ( pack->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
		} else {
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
		}
		DG_DrawIndexedVertices( D3DPT_TRIANGLESTRIP, pack->n_indices, index_addr );
		index_addr += pack->n_indices ;
	}

	/* カリング禁止 */
	DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );

}


/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

static int BoundCheck( FMATRIX *mat, FVECTOR *bound, int mode )
{
	int		and_flag, or_flag, prim_over_flag, flag ;
	FVECTOR		verts, tmp_v ;
	float		w, total_w ;
	int			i ;

	and_flag = 0xffffffff ;
	or_flag = 0 ;
	prim_over_flag = 0 ;
	total_w = 0 ;

	for ( i = 8 ; i > 0 ; i-- ){
		verts.vx = ( i & 1 ) ? bound[0].vx : bound[1].vx ;
		verts.vy = ( i & 2 ) ? bound[0].vy : bound[1].vy ;
		verts.vz = ( i & 4 ) ? bound[0].vz : bound[1].vz ;
		verts.vw = 1.0F ;
		_sceVu0ApplyMatrix( &tmp_v, mat, &verts );
		total_w += tmp_v.vw ;
		w = DG_FABS( tmp_v.vw );

		/* クリップチェック */
		flag = 0 ;
		if ( tmp_v.vx >  w ) flag |= CLIP_X0_FLAG;
		if ( tmp_v.vx < -w ) flag |= CLIP_X1_FLAG;
		if ( tmp_v.vy >  w ) flag |= CLIP_Y0_FLAG;
		if ( tmp_v.vy < -w ) flag |= CLIP_Y1_FLAG;
		if ( tmp_v.vz >  w ) flag |= CLIP_Z0_FLAG;
		if ( tmp_v.vz < -w ) flag |= CLIP_Z1_FLAG;
		//if ( tmp_v.vz < 0.0f ) flag |= CLIP_Z1_FLAG;	/* XBOXはPS2とは違うので注意 */
		and_flag &= flag ;
		or_flag |= flag ;

#if 0
		/* ＸＹ軸において余裕を持った大きさでチェック */
		tmp_v.vx *= (SCALE)->vx / 640.0f ;
		tmp_v.vy *= (SCALE)->vy / 512.0f ;
		flag = 0 ;
		if ( tmp_v.vx >  w ) flag |= CLIP_X0_FLAG;
		if ( tmp_v.vx < -w ) flag |= CLIP_X1_FLAG;
		if ( tmp_v.vy >  w ) flag |= CLIP_Y0_FLAG;
		if ( tmp_v.vy < -w ) flag |= CLIP_Y1_FLAG;
		prim_over_flag |= flag ;
#endif

	}
	/* バウンディング中央値のＺ座標取得 */
	//SORT_Z = total_w / 8.0f ;

#if 0
	/* バウンディング判定 */
	if ( mode == 0 ){
		if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
		if ( or_flag & 0x30 ) return ( 1 );	/* クリップ必要 */
		if ( prim_over_flag ) return ( 1 );	/* クリップ必要 */
	} else {
		if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
		if ( or_flag & 0x3f ) return ( 1 );	/* クリップ必要 */
	}
#else
	/* ＸＢＯＸではクリップによるコストは無視しても大丈夫そうなので */
	if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
	if ( or_flag & 0x3f ) return ( 1 );	/* クリップ必要 */
#endif

	return ( 0 );	/* 完全画面内 */

}

/*----------------------------------------------------------------*/

	/*
		ＤＭＡに接続する
	*/
static	void	MakeShadowChainObjs( DG_OBJS *objs )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJ		*obj ;
	int			i ;
	int			bound ;
	DG_OBJS		*org_objs = objs ;

	/* 強制的に光源をなくして黒で描画 */
	GV_ZeroMemory( &scrpad->local_color, sizeof(FMATRIX) );
	//scrpad->local_color.m[3][0] = 128 ;
	//scrpad->local_color.m[3][1] = 256 ;
	//scrpad->local_color.m[3][2] = 256 ;
	/* 影処理ではローモデルがあればそれを使う */
	if ( objs->low != NULL ) objs = objs->low ;

	/* テクスチャをなくす */
	DG_SetTexture( 0, NULL );

	/* 投影マトリクスの設定 */
	DG_SetVertexShaderConstant(CV_EYEPERS, &scrpad->light_pers2, 4);

	obj = objs->objs ;
	for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
		/* クリップアウトチェック */
		_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &obj->world );
		if ( ( bound = scrpad->bound_mode ) == 1 ){
			bound = BoundCheck( &scrpad->tmp_mat, &obj->bound_min, 0 ) ;
		}
		if ( bound == 2 ) continue ;
		/* その他スキップ条件チェック */
		if ( objs->flag & ( DG_FLAG_SHADOWMAKE & DG_FLAG_PLUGINMASK ) ){
			/* 影生成フラグがついている場合のみ関節の表示・非表示を見るようにする */
			if ( obj->flag & scrpad->invisible_flag ) continue ;
		}
		if ( obj->model->type & DG_TYPE_TRANS ) continue ;

		scrpad->env_mat = obj->inv_mat ;
		scrpad->fog = 0x8fff ;
		scrpad->local_bound_mode = bound ;

#ifdef LIBDG_PERFORMANCE
		scrpad->n_packs = 0 ;
		scrpad->n_verts = 0 ;
#endif
		ChainObj( obj, 0 );
#ifdef LIBDG_PERFORMANCE
		scrpad->shadow_draw[bound].n_obj++ ;
		scrpad->shadow_draw[bound].n_packs += scrpad->n_packs ;
		scrpad->shadow_draw[bound].n_verts += scrpad->n_verts ;
#endif
	}

}

static	void	WriteShadowChainObjs( DG_OBJS *objs )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJ		*obj ;
	int			i, bound ;

	obj = objs->objs ;
	for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
		/* クリップアウトチェック */
		if ( obj->bound_mode & 2 ) continue ;
		/* その他スキップ条件チェック */
		if ( obj->flag & scrpad->invisible_flag ) continue ;
		if ( obj->model->type & ( DG_TYPE_TRANS | DG_TYPE_OVERLAY0 | DG_TYPE_OVERLAY1 | DG_TYPE_OVERLAY2 ) ) continue ;
		/* スポットライトから見たバウンディングをチェック */
		_sceVu0MulMatrix( &scrpad->local_light, &scrpad->light_pers, &obj->world );
		bound = BoundCheck( &scrpad->local_light, &obj->bound_min, 0 ) ;
		if ( bound == 2 ) continue ;
		if ( scrpad->para_mode == 0 ){
			/* スポットライト時 */
			scrpad->local_bound_mode = obj->bound_mode | bound ;
			scrpad->local_light = scrpad->light_pers2 ;
		} else {
			/* 平行投影時 */
			/* 現在は投影によるニアクリップを行っていないため */
			scrpad->local_bound_mode = obj->bound_mode ;
			/* 並行投影でシャドウマッピング描画を行う場合にテクスチャ射影マトリクスに細工をする */
			//scrpad->tmp_mat = scrpad->light_pers2 ;
			////scrpad->tmp_mat.m[2][3] = 1.0f ;
			////scrpad->tmp_mat.m[3][3] = 0.0f ;
			//_sceVu0MulMatrix( &scrpad->local_light, &scrpad->tmp_mat, &obj->world );
			scrpad->local_light = scrpad->light_pers2 ;
		}

		/* オブジェクト描画パラメータ構造体にデータをセット */
		scrpad->env_mat = obj->inv_mat ;
#if 0
		{/* フォグ計算 */
			float	f ;
			int		ifog ;
			f = scrpad->fog_param1 * obj->sort_z + scrpad->fog_param2 ;
			//f =  DG_FogParam1 * obj->sort_z + DG_FogParam2 ;
			f = DG_MAX( f, 0.0f );
			f = DG_MIN( f, 255.0f );
			ifog = DG_FTOI( f ) ;
			scrpad->fog = ( ifog << 4 ) | 0x8000 ;
		}
#endif
		*(FVECTOR*)scrpad->local_color.m[0] = scrpad->color_vector ;
		scrpad->local_color.m[1][3] = scrpad->far_range ;
		/* 光源ベクトルをモデル座標相対に変換 */
		*(FVECTOR*)scrpad->local_color.m[2] = scrpad->light_vector ;
		*(FVECTOR*)scrpad->local_color.m[3] = scrpad->light_pos ;
		//{
		//	FMATRIX	tmp_mat ;
		//	_sceVu0InversMatrix( &tmp_mat, &obj->world );
		//	scrpad->light_vector.vw = 0.0f ;
		//	_sceVu0ApplyMatrix( (FVECTOR*)scrpad->local_color.m[2], &tmp_mat, &scrpad->light_vector );
		//}

#ifdef LIBDG_PERFORMANCE
		scrpad->n_packs = 0 ;
		scrpad->n_verts = 0 ;
#endif
		ChainObj( obj, 1 );
#ifdef LIBDG_PERFORMANCE
		scrpad->projection[scrpad->local_bound_mode].n_obj++ ;
		scrpad->projection[scrpad->local_bound_mode].n_packs += scrpad->n_packs ;
		scrpad->projection[scrpad->local_bound_mode].n_verts += scrpad->n_verts ;
#endif
	}

}

/*----------------------------------------------------------------*/


/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

static void GetSpotViewport( DG_SPOT *spot )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	float		angle, near ;

	near = 100.0f ;
	angle = 2.0f ;
	angle = spot->angle ;
	/* ライトのビューポートマトリクスを生成 */
	scrpad->color_vector.vx = 128.0f ;
	scrpad->color_vector.vy = 128.0f ;
	scrpad->color_vector.vz = 128.0f ;
	scrpad->color_vector.vw = 128.0f ;
	scrpad->light_vector = *(FVECTOR*)spot->world.m[2] ;
	scrpad->light_pos = *(FVECTOR*)spot->world.m[3] ;
	scrpad->far_range = spot->range * 2 - near ;
	scrpad->para_mode = 0 ;
	/* パース付き投影 */
	DG_ViewScreenMatrix( &scrpad->light_view,
						   angle,
						   1.0f, -1.0f, 0.0f, 0.0f,
						   -1.0f, 1.0f, near, spot->range * 2 );
	DG_ViewScreenMatrix( &scrpad->light_view2,
						   angle,
						   1.0f, -1.0f, 0.0f, 0.0f,
						   0.0f, 1.0f, near, spot->range * 2 );		/* 頂点シェーダー用 */
	/* ライト用の透視変換マトリクスの作成 */
	_sceVu0InversMatrix( &scrpad->tmp_mat, &spot->world );
	_sceVu0MulMatrix( &scrpad->light_pers, &scrpad->light_view, &scrpad->tmp_mat );
	_sceVu0MulMatrix( &scrpad->light_pers2, &scrpad->light_view2, &scrpad->tmp_mat );

}

	/*
		キューされた各オブジェクトをＤＭＡパケットに接続する
	*/
void DG_SpotChainChanl( DG_CHANL *cp, int which )
{
#if 0 //BP_RENDER
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJS		/* **oque, */*objs, **oque2 ;
	DG_SPOT		**spot_queue, *spot ;
	DG_OBJ_QUEUE	*que ;
	DG_OBJ_BUFFER	*obj_buff ;
	int		/*i, */j, k, size, shadow_id, enable, gid, c_gid ;
	//int		time1, time2, time3, debug_count ;
	OBJ_LIST	*list ;

	if ( DG_ActiveShadowFlag != 2 ) return ;

	que = cp->obj_queue ;
	if ( que == NULL ) return ;
	obj_buff = &que->objs_buffer ;

	c_gid = cp->group_id ;

	scrpad->invisible_flag = DG_FLAG_INVISIBLE0 << cp->chanl_num ;
	scrpad->buffer_switch = 0 ;
	scrpad->eye_pers = cp->eye_pers ;
	scrpad->raise_eye_pers = cp->raise_eye_xpers ;
	scrpad->fog_param1 = 0.0f ;
	scrpad->fog_param2 = 255.0f ;

#ifdef LIBDG_PERFORMANCE
	scrpad->projection[0] = DG_PerformanceData.spot_projection[0] ;
	scrpad->projection[1] = DG_PerformanceData.spot_projection[1] ;
	scrpad->shadow_draw[0] = DG_PerformanceData.shadow_draw[0] ;
	scrpad->shadow_draw[1] = DG_PerformanceData.shadow_draw[1] ;
#endif

	{/* オブジェクト検索高速化用リスト生成 */
		scrpad->max_list = 0 ;
		list = scrpad->list ;
		oque2 = (DG_OBJS**)obj_buff->queue ;
		for ( j = obj_buff->n_queue ; j > 0 ; j--, oque2++ ){
			objs = *oque2 ;
			//if ( objs->flag & DG_FLAG_INVISIBLE ) continue ;
			if ( !( objs->flag & ( ( DG_FLAG_SHADOWWRITE | DG_FLAG_SHADOWMAKE ) & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
			list->objs = objs ;
			list->flag = objs->flag ;
			list->shadow_id = objs->shadow_id ;
			list->bound_mode = objs->bound_mode ;
			list->group_id = objs->group_id ;
			list++ ;
			scrpad->max_list++ ;
		}
	}

	/* スタティックプッシュバッファへの記録開始 */
	DG_OpenDmaTask();

	/* レンダリング初期化 */
	DG_InitChanlRenderState( cp, 0 );

	spot_queue = (DG_SPOT**)que->spot_buffer.queue ;
	for ( k = que->spot_buffer.n_queue ; k > 0 ; k--, spot_queue++ ){
		spot = *spot_queue ;
		if ( spot->flag & DG_FLAG_INVISIBLE ) continue ;

		shadow_id = spot->shadow_id ;
		enable = 0 ;

		gid = 0 ;
		/* 画面内表示チェック */
		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			if ( !( list->flag & ( DG_FLAG_SHADOWWRITE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( list->flag & scrpad->invisible_flag ) continue ;
			/* 同じＩＤのスポットライト投影モデルが存在する場合に処理を行なう */
			if ( list->shadow_id == shadow_id ){
				if ( list->bound_mode != 2 ){
					enable = 1 ;
					gid |= list->group_id ;
				}
			}
		}
		if ( enable == 0 ) continue ;

		GetSpotViewport( spot );

		/* 影マスク描画 */
		DG_SetRenderTarget( DG_ShadowBackBufferSurface, DG_ShadowDepthBufferSurface );
		DG_Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0x0000000, 0.0f, 0 );
		DG_SetRenderState( D3DRS_ZWRITEENABLE, 1 );		
		/* 投影マトリクスの設定 */
		DG_SetVertexShaderConstant( CV_EYEPERS, &scrpad->light_pers2, 4 );
		DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ) );
		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			if ( !( list->flag & ( DG_FLAG_SHADOWWRITE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( list->shadow_id != shadow_id ) continue ;
			objs = list->objs ;
			scrpad->root = objs->world ;
			if ( list->flag & DG_FLAG_ONEPIECE ){
				scrpad->root.m[3][0] += objs->def->tx ;
				scrpad->root.m[3][1] += objs->def->ty ;
				scrpad->root.m[3][2] += objs->def->tz ;
			}
			_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &scrpad->root );
			if ( ( scrpad->bound_mode = BoundCheck( &scrpad->tmp_mat, &objs->bound_min, 0 ) ) == 2 ) continue ;
			MakeShadowChainObjs( objs );	/* オブジェクトをＤＭＡに登録 */
		}

		DG_SetRenderState( D3DRS_ZWRITEENABLE, 0 );		
		{/* ライトの描画 */
			//DG_Clear(0, NULL, D3DCLEAR_TARGET, 0x80ffffff, 0.0f, 0 );
			//DG_Clear(0, NULL, D3DCLEAR_TARGET, 0x00000000, 0.0f, 0 );
			Init2DDraw();
			DG_SetAlphaMode( 0 );
			DrawSpot( spot->color );
			DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ) );
		}
		//size = DG_WriteObjsPacketInit_MakeSpot( scrpad->dma_buffer, cp, spot->color, DG_SpotLightBaseImage, 0 );
		//DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );

		/* 影の生成 */
		DG_SetRenderState( D3DRS_EDGEANTIALIAS, TRUE );
		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			//if ( list->flag & DG_FLAG_INVISIBLE ) continue ;
			if ( !( list->flag & ( DG_FLAG_SHADOWMAKE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( !( list->group_id & gid ) ) continue ;
			objs = list->objs ;
			scrpad->root = objs->world ;
			if ( list->flag & DG_FLAG_ONEPIECE ){
				scrpad->root.m[3][0] += objs->def->tx ;
				scrpad->root.m[3][1] += objs->def->ty ;
				scrpad->root.m[3][2] += objs->def->tz ;
			}
			_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &scrpad->root );
			if ( ( scrpad->bound_mode = BoundCheck( &scrpad->tmp_mat, &objs->bound_min, 1 ) ) == 2 ) continue ;
			while ( objs != NULL ){
				MakeShadowChainObjs( objs );	/* オブジェクトをＤＭＡに登録 */
				objs = objs->next ;				/* 関連付けされた他のオブジェクトも処理 */
				scrpad->bound_mode = 1 ;
			}
		}
		DG_SetRenderState( D3DRS_EDGEANTIALIAS, FALSE );

		{/* テクスチャ暈し */
		}

		/* 影の描画 */
		DG_SetRenderTarget( DG_BackBufferSurface[ DG_CurrentBackBuffer ], DG_DepthBufferSurface );
		//DG_Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0x00000000, 0.0f, 0 );
		//DG_SetRenderState( D3DRS_ZWRITEENABLE, 0 );		
		IDirect3DDevice8_SetViewport( g_pd3dDevice, &DG_CurrentViewport );
		DG_SetTextureDirect( 0, DG_ShadowBackBufferTexture );
		DG_SetTextureDirect( 1, DG_ShadowBackBufferTexture );
		DG_SetTextureDirect( 2, DG_ShadowBackBufferTexture );
		DG_SetPixelShader( &DG_ShadowPixelShader[0] );
		DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 2, 0, 1, 128 ) );
		/* 投影マトリクスの設定 */
		DG_SetVertexShaderConstant( CV_EYEPERS, &scrpad->raise_eye_pers, 4 );
		//size = DG_WriteObjsPacketInit_WriteShadow( scrpad->dma_buffer, cp, scrpad->fog_param1, scrpad->fog_param2 );
		//DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			if ( !( list->flag & ( DG_FLAG_SHADOWWRITE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( list->shadow_id != shadow_id ) continue ;
			WriteShadowChainObjs( list->objs );
		}
		DG_SetPixelShader( NULL );
		DG_InitTextureStageState( 0 );
		DG_InitTextureStageState( 1 );
		DG_InitTextureStageState( 2 );
		DG_SetTexture( 0, NULL );
		DG_SetTexture( 1, NULL );
		DG_SetTexture( 2, NULL );
		DG_SetRenderState( D3DRS_ZWRITEENABLE, 1 );		

		if ( DG_ISCHECK_PACKETBUFFER() ) break ;	/* メモリが足りなくなったら強制終了 */
	}

	/* 終了パケットの設定 */
	/* 描画環境復元 */
	DG_SetRenderState(D3DRS_FOGENABLE, TRUE);

	/* スタティックプッシュバッファへの記録終了 */
	DG_CloseDmaTask();

#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.spot_projection[0] = scrpad->projection[0] ;
	DG_PerformanceData.spot_projection[1] = scrpad->projection[1] ;
	DG_PerformanceData.shadow_draw[0] = scrpad->shadow_draw[0] ;
	DG_PerformanceData.shadow_draw[1] = scrpad->shadow_draw[1] ;
#endif

#endif
}




/* ---------------------------------------------------------------- */
static void OrthoScreenMatrix( FMATRIX *res, float width, float height, float scale,
					   float zmin, float zmax, float nearz, float farz )
{
	float	z_width, z_center ;

	_sceVu0UnitMatrix( res );
	/* X */
	res->m[0][0] = 2.0f * scale / width ;
	/* Y */
	res->m[1][1] = 2.0f * scale / height ;
	/* Z */
	z_width = ( farz - nearz ) / ( zmax - zmin ) ;
	//z_center = ( -farz * zmin + nearz * zmax ) / ( zmax - zmin ) ;
	z_center = ( farz * zmax - nearz * zmin ) / ( zmax - zmin ) ;
	res->m[2][2] = -1.0f / z_width ;
	res->m[3][2] = z_center / z_width ;
	/* W */
	res->m[3][3] = 1.0f ;

}
static void GetParallelViewport( DG_SPOT *spot )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	float		angle, near ;

	near = 100.0f ;
	angle = spot->angle ;
	/* ライトのビューポートマトリクスを生成 */
	scrpad->color_vector.vx = 128.0f ;
	scrpad->color_vector.vy = 128.0f ;
	scrpad->color_vector.vz = 128.0f ;
	scrpad->color_vector.vw = (float)( spot->color >> 24 ) ;
	scrpad->light_vector = *(FVECTOR*)spot->world.m[2] ;
	scrpad->light_pos = *(FVECTOR*)spot->world.m[3] ;
	scrpad->far_range = spot->range * 2 - near ;

	if ( angle == 0.0f ){
		OrthoScreenMatrix( &scrpad->light_view, 3000.0f, 3000.0f, 1.0f,
						  -1.0f, 1.0f, near, spot->range * 2 );
#if 0
		OrthoScreenMatrix( &scrpad->light_view2, 3000.0f, 3000.0f, 128.0f,
						  (float)DRAW_Z_MIN, (float)DRAW_Z_MAX, near, spot->range * 2 );
		scrpad->light_view2.m[3][0] = 2048.0f ;
		scrpad->light_view2.m[3][1] = 2048.0f ;
#else
		OrthoScreenMatrix( &scrpad->light_view2, 3000.0f, 3000.0f, 1.0f,
						  0.0f, 1.0f, near, spot->range * 2 );
#endif
		scrpad->para_mode = 1 ;
	} else {
		/* パース付き投影 */
		DG_ViewScreenMatrix( &scrpad->light_view,
							angle,
							1.0f, 1.0f, 0.0f, 0.0f,
							-1.0f, 1.0f, near, spot->range * 2 );
#if 0
		DG_ViewScreenMatrix( &scrpad->light_view2,
								angle*128.0f,
								1.0f, 1.0f, 2048.0f, 2048.0f,
								(float)DRAW_Z_MIN, (float)DRAW_Z_MAX, near, spot->range * 2 );
#else
		DG_ViewScreenMatrix( &scrpad->light_view2,
							angle,
							1.0f, 1.0f, 0.0f, 0.0f,
							0.0f, 1.0f, near, spot->range * 2 );
#endif
		scrpad->para_mode = 0 ;
	}
	/* ライト用の透視変換マトリクスの作成 */
	_sceVu0InversMatrix( &scrpad->tmp_mat, &spot->world );
	_sceVu0MulMatrix( &scrpad->light_pers, &scrpad->light_view, &scrpad->tmp_mat );
	_sceVu0MulMatrix( &scrpad->light_pers2, &scrpad->light_view2, &scrpad->tmp_mat );
}


//void DG_SpotChainChanl( DG_CHANL *cp, int which )
void DG_ShadowChainChanl( DG_CHANL *cp, int which )
{
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJS		/* **oque, */*objs, **oque2 ;
	DG_SPOT		**spot_queue, *spot ;
	DG_OBJ_QUEUE	*que ;
	DG_OBJ_BUFFER	*obj_buff ;
	int		/*i, */j, k, size, shadow_id, enable, gid, c_gid ;
	//int		time1, time2, time3, debug_count ;
	OBJ_LIST	*list ;

	if ( DG_ActiveShadowFlag != 1 ) return ;

	que = cp->obj_queue ;
	if ( que == NULL ) return ;
	obj_buff = &que->objs_buffer ;

	c_gid = cp->group_id ;

	scrpad->invisible_flag = DG_FLAG_INVISIBLE0 << cp->chanl_num ;
	scrpad->buffer_switch = 0 ;
	scrpad->eye_pers = cp->eye_pers ;
	scrpad->raise_eye_pers = cp->raise_eye_xpers ;
	scrpad->fog_param1 = 0.0f ;
	scrpad->fog_param2 = 255.0f ;

#ifdef LIBDG_PERFORMANCE
	scrpad->projection[0] = DG_PerformanceData.spot_projection[0] ;
	scrpad->projection[1] = DG_PerformanceData.spot_projection[1] ;
	scrpad->shadow_draw[0] = DG_PerformanceData.shadow_draw[0] ;
	scrpad->shadow_draw[1] = DG_PerformanceData.shadow_draw[1] ;
#endif

	{/* オブジェクト検索高速化用リスト生成 */
		scrpad->max_list = 0 ;
		list = scrpad->list ;
		oque2 = (DG_OBJS**)obj_buff->queue ;
		for ( j = obj_buff->n_queue ; j > 0 ; j--, oque2++ ){
			objs = *oque2 ;
			//if ( objs->flag & DG_FLAG_INVISIBLE ) continue ;
			if ( !( objs->flag & ( ( DG_FLAG_SHADOWWRITE | DG_FLAG_SHADOWMAKE ) & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( ( gid = objs->group_id ) != 0 && !( gid & c_gid ) ) continue ;
			list->objs = objs ;
			list->flag = objs->flag ;
			list->shadow_id = objs->shadow_id ;
			list->bound_mode = objs->bound_mode ;
			list->group_id = objs->group_id ;
			list++ ;
			scrpad->max_list++ ;
		}
	}

	/* スタティックプッシュバッファへの記録開始 */
	DG_OpenDmaTask();

	/* レンダリング初期化 */
	DG_InitChanlRenderState( cp, 0 );

	spot_queue = (DG_SPOT**)que->spot_buffer.queue ;
	for ( k = que->spot_buffer.n_queue ; k > 0 ; k--, spot_queue++ ){
		spot = *spot_queue ;
		if ( spot->flag & DG_FLAG_INVISIBLE ) continue ;

		shadow_id = spot->shadow_id ;
		enable = 0 ;

		gid = 0 ;
		/* 画面内表示チェック */
		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			if ( !( list->flag & ( DG_FLAG_SHADOWWRITE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( list->flag & scrpad->invisible_flag ) continue ;
			/* 同じＩＤのスポットライト投影モデルが存在する場合に処理を行なう */
			if ( list->shadow_id == shadow_id || shadow_id == 0 ){
				if ( list->bound_mode != 2 ){
					enable = 1 ;
					gid |= list->group_id ;
				}
			}
		}
		if ( enable == 0 ) continue ;

		GetParallelViewport( spot );

		/* 影マスク描画 */
		DG_SetRenderTarget( DG_ShadowBackBufferSurface, DG_ShadowDepthBufferSurface );
		DG_Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0x0000000, 0.0f, 0 );
		DG_SetRenderState( D3DRS_ZWRITEENABLE, 1 );		
		/* 投影マトリクスの設定 */
		DG_SetVertexShaderConstant( CV_EYEPERS, &scrpad->light_pers2, 4 );
		DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ) );
		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			if ( !( list->flag & ( DG_FLAG_SHADOWWRITE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			//if ( list->shadow_id != shadow_id ) continue ;
			if ( ( list->shadow_id != shadow_id ) && ( shadow_id != 0 ) ) continue ;
			objs = list->objs ;
			scrpad->root = objs->world ;
			if ( list->flag & DG_FLAG_ONEPIECE ){
				scrpad->root.m[3][0] += objs->def->tx ;
				scrpad->root.m[3][1] += objs->def->ty ;
				scrpad->root.m[3][2] += objs->def->tz ;
			}
			_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &scrpad->root );
			if ( ( scrpad->bound_mode = BoundCheck( &scrpad->tmp_mat, &objs->bound_min, 0 ) ) == 2 ) continue ;
			MakeShadowChainObjs( objs );	/* オブジェクトをＤＭＡに登録 */
		}

		DG_SetRenderState( D3DRS_ZWRITEENABLE, 0 );		
		{/* ライトの描画 */
			DG_Clear(0, NULL, D3DCLEAR_TARGET, 0x00000000, 0.0f, 0 );
			DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ) );
		}
		//size = DG_WriteObjsPacketInit_MakeSpot( scrpad->dma_buffer, cp, spot->color, DG_SpotLightBaseImage, 1 );
		//DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );

		//debug_count = 0 ;
		/* 影の生成 */
		//DG_SetRenderState( D3DRS_EDGEANTIALIAS, TRUE );
#if 0
		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			//if ( list->flag & DG_FLAG_INVISIBLE ) continue ;
			if ( !( list->flag & ( DG_FLAG_SHADOWMAKE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( !( list->group_id & gid ) ) continue ;
			objs = list->objs ;
			scrpad->root = objs->world ;
			if ( list->flag & DG_FLAG_ONEPIECE ){
				scrpad->root.m[3][0] += objs->def->tx ;
				scrpad->root.m[3][1] += objs->def->ty ;
				scrpad->root.m[3][2] += objs->def->tz ;
			}
			_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &scrpad->root );
			if ( ( scrpad->bound_mode = BoundCheck( &scrpad->tmp_mat, &objs->bound_min, 1 ) ) == 2 ) continue ;
			while ( objs != NULL ){
				//ShadowScreenObjs( objs );		/* 影投影マトリクスの設定 */
				MakeShadowChainObjs( objs );	/* オブジェクトをＤＭＡに登録 */
				objs = objs->next ;				/* 関連付けされた他のオブジェクトも処理 */
				//debug_count++ ;
				scrpad->bound_mode = 1 ;
			}
		}
#else
		if ( ( objs = spot->objs ) != NULL ){
			if ( objs->flag & ( DG_FLAG_SHADOWMAKE & DG_FLAG_PLUGINMASK ) ){
				scrpad->root = objs->world ;
				if ( objs->flag & DG_FLAG_ONEPIECE ){
					scrpad->root.m[3][0] += objs->def->tx ;
					scrpad->root.m[3][1] += objs->def->ty ;
					scrpad->root.m[3][2] += objs->def->tz ;
				}
				_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &scrpad->root );
				if ( ( scrpad->bound_mode = BoundCheck( &scrpad->tmp_mat, &objs->bound_min, 1 ) ) != 2 ){
					while ( objs != NULL ){
						//ShadowScreenObjs( objs );		/* 影投影マトリクスの設定 */
						MakeShadowChainObjs( objs );	/* オブジェクトをＤＭＡに登録 */
						objs = objs->next ;				/* 関連付けされた他のオブジェクトも処理 */
						//debug_count++ ;
						scrpad->bound_mode = 1 ;
					}
				}
			}
		}
#endif
		//DG_SetRenderState( D3DRS_EDGEANTIALIAS, FALSE );

		{/* テクスチャ暈し */
		}

		/* 影の描画 */
		DG_SetRenderTarget( DG_BackBufferSurface[ DG_CurrentBackBuffer ], DG_DepthBufferSurface );
		//DG_Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0x00000000, 0.0f, 0 );
		DG_SetRenderState( D3DRS_ZWRITEENABLE, 0 );		
		IDirect3DDevice9_SetViewport( g_pd3dDevice, &DG_CurrentViewport );
		DG_SetTextureDirect( 0, DG_ShadowBackBufferTexture );
		DG_SetTextureDirect( 1, DG_ShadowBackBufferTexture );
		DG_SetTextureDirect( 2, DG_ShadowBackBufferTexture );
		DG_SetPixelShader( &DG_ShadowPixelShader[0] );
		DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ) );
		/* 投影マトリクスの設定 */
		DG_SetVertexShaderConstant( CV_EYEPERS, &scrpad->raise_eye_pers, 4 );
		//size = DG_WriteObjsPacketInit_WriteShadow( scrpad->dma_buffer, cp, scrpad->fog_param1, scrpad->fog_param2 );
		//DG_SendCacheFIFO( scrpad->local_work, scrpad->dma_buffer, size );
		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			if ( !( list->flag & ( DG_FLAG_SHADOWWRITE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( ( list->shadow_id != shadow_id ) && ( shadow_id != 0 ) ) continue ;
			objs = list->objs ;
			scrpad->root = objs->world ;
			if ( list->flag & DG_FLAG_ONEPIECE ){
				scrpad->root.m[3][0] += objs->def->tx ;
				scrpad->root.m[3][1] += objs->def->ty ;
				scrpad->root.m[3][2] += objs->def->tz ;
			}
			//_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &scrpad->root );
			//if ( ( scrpad->bound_mode = BoundCheck( &scrpad->tmp_mat, &objs->bound_min, 0 ) ) == 2 ) continue ;
			scrpad->bound_mode = objs->bound_mode ;
			WriteShadowChainObjs( list->objs );
		}
		DG_SetPixelShader( NULL );
		DG_SetTexture( 1, NULL );
		DG_SetTexture( 2, NULL );
		DG_InitTextureStageState( 0 );
		DG_InitTextureStageState( 1 );
		DG_InitTextureStageState( 2 );
		DG_SetRenderState( D3DRS_ZWRITEENABLE, 1 );		

		if ( DG_ISCHECK_PACKETBUFFER() ) break ;	/* メモリが足りなくなったら強制終了 */
	}

	/* 終了パケットの設定 */
	/* 描画環境復元 */
	DG_SetRenderState(D3DRS_FOGENABLE, TRUE);

	/* スタティックプッシュバッファへの記録終了 */
	DG_CloseDmaTask();

#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.spot_projection[0] = scrpad->projection[0] ;
	DG_PerformanceData.spot_projection[1] = scrpad->projection[1] ;
	DG_PerformanceData.shadow_draw[0] = scrpad->shadow_draw[0] ;
	DG_PerformanceData.shadow_draw[1] = scrpad->shadow_draw[1] ;
#endif

}

#endif
