/*
	xshdwchn.c
	チャンネル処理ユニット／影オブジェクトＤＭＡ接続ルーチン

	2002/04/02 K.Takabe
	$Id: wshdwchn.c,v 1.30 2003/01/06 04:39:51 takaki Exp $

*/
/*

	void		DG_ShadowChainChanl( DG_CHANL *cp, int which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	キューされた各オブジェクトをＤＭＡパケットに接続する


*/

#ifdef _XBOX
#include <xtl.h>
#include <xgraphics.h>
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

#include "sse_inline.h"

/*----------------------------------------------------------------*/


/*----------------------------------------------------------------*/
	/*
	*/

/* スポットライトのベーステクスチャイメージへのポインタ */
#ifndef _WINDOWS
void		*DG_SpotLightBaseImage   = NULL ;
#else
DG_TEX_LIN	*DG_SpotLightBaseTexture = NULL ;
#endif

#if FALSE
extern LPDIRECT3DTEXTURE8	DG_BackBufferTexture[3] ;		/* バックバッファテクスチャ */
extern LPDIRECT3DSURFACE8	DG_BackBufferSurface[3] ;		/* バックバッファサーフェス */
extern LPDIRECT3DSURFACE8	DG_DepthBufferSurface ;			/* Ｚバッファサーフェス */
extern DG_VIEWPORT			DG_CurrentViewport ;
extern LPDIRECT3DTEXTURE8	DG_ShadowBackBufferTexture ;	/* 影用バックバッファテクスチャ */
extern LPDIRECT3DSURFACE8	DG_ShadowBackBufferSurface ;	/* 影用バックバッファサーフェス */
extern LPDIRECT3DSURFACE8	DG_ShadowDepthBufferSurface ;	/* 影用Ｚバッファサーフェス */

#else

extern DG_VIEWPORT			DG_CurrentViewport ;

extern LPDIRECT3DTEXTURE8	DG_ShadowTexture;				// 影レンダリング用テクスチャ
extern LPDIRECT3DTEXTURE8	DG_ShadowDepthTexture;			// 影Depthレンダリング用テクスチャ
extern LPDIRECT3DSURFACE8	DG_ShadowSurface ;				// 影用バックバッファサーフェス
extern LPDIRECT3DSURFACE8	DG_ShadowDepthSurface ;			// 影用Ｚバッファサーフェス
extern LPDIRECT3DTEXTURE8	DG_GetFrontBufferTexture;		// GetFrontBuffer用テクスチャ
extern LPDIRECT3DTEXTURE8	DG_BackBuffer2Texture;			// BackBuffer2用テクスチャ

#endif

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

static	void	ChainObj( DG_OBJ *obj, int type ) ;
static	void	ChainObjNIdx( DG_OBJ *obj, int type ) ;		// Index非対応版
static	void	ChainObjNVS( DG_OBJ *obj, int type ) ;		// VertexShader非対応版
static	void	ChainObjNVSNIdx( DG_OBJ *obj, int type ) ;	// VertexShader/Index非対応版
static	void	ChainObjFvf( DG_OBJ *obj, int type ) ;		// FVF頂点版
static	void	ChainObjFvfNIdx( DG_OBJ *obj, int type ) ;	// FVF頂点Index非対応版

static	void	ChainSpotLight(DG_SPOT *spot) ;		// VertexShader対応版
static	void	ChainSpotLightNVS(DG_SPOT *spot) ;	//	VertexShader非対応版

static	void	*_mk_chain_obj_func ;
static	void	*_wt_chain_obj_func ;
static	void	*_chain_spotlight_func ;

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

typedef struct {
	signed short	x, y ;
	signed short	u, v ;
} DG_VERTEX_SHDW ;

typedef struct	DG_VERTEX_PROJSHDW01_
{
	float	x, y, z ;
	float	wt ;
	DWORD	diffuse ;
	float	u0, v0, w0 ;
} DG_VERTEX_PRJSHDW01 ;

typedef struct	DG_VERTEX_PROJSHDW02_
{
	float	x, y, z ;
	float	wt ;
	DWORD	diffuse ;
	float	u0, v0, w0 ;
	float	u1, v1, w1 ;
} DG_VERTEX_PRJSHDW02 ;

typedef struct	DG_VERTEX_PROJSHDW03_
{
	float	x, y, z ;
	float	wt ;
	DWORD	diffuse ;
	float	u0, v0, w0 ;
	float	u1, v1, w1 ;
	float	u2, v2, w2 ;
} DG_VERTEX_PRJSHDW03 ;

#define	DG_VERTEX_PRJSHDW01_FLAG	( D3DFVF_XYZB1		\
									| D3DFVF_DIFFUSE	\
									| D3DFVF_TEX1		\
									| D3DFVF_TEXCOORDSIZE3(0))

#define	DG_VERTEX_PRJSHDW02_FLAG	( D3DFVF_XYZB1		\
									| D3DFVF_DIFFUSE	\
									| D3DFVF_TEX1		\
									| D3DFVF_TEX2		\
									| D3DFVF_TEXCOORDSIZE3(0)	\
									| D3DFVF_TEXCOORDSIZE3(1))

#define	DG_VERTEX_PRJSHDW03_FLAG	( D3DFVF_XYZB1		\
									| D3DFVF_DIFFUSE	\
									| D3DFVF_TEX1		\
									| D3DFVF_TEX2		\
									| D3DFVF_TEX3		\
									| D3DFVF_TEXCOORDSIZE3(0)	\
									| D3DFVF_TEXCOORDSIZE3(1)	\
									| D3DFVF_TEXCOORDSIZE3(2))

int	DG_ActiveShadowFlag = 0 ;

/*----------------------------------------------------------------*/

#ifndef	NO_PROTOTYPE
#endif

extern void DG_StartBoundingCheckSupport( void );/* screen.c */
extern int DG_WriteObjsPacketInit_WriteShadow( void *tag_addr, DG_CHANL *cp, float fog_param1, float fog_param2 );


static	void	CalcKMS2PRJSHDW01(DG_OBJ *obj,DG_VERTEX_PRJSHDW01 *dst, void *src, DWORD stride, int num) ;
static	void	CalcKMSFVF2PRJSHDW01(DG_OBJ *obj,DG_VERTEX_PRJSHDW01 *dst, void *src, DWORD stride, int num) ;

/*----------------------------------------------------------------*/
DG_PIXELSHADER	DG_ShadowPixelShader[1] ;
extern DG_VERTEXSHADER	DG_DmapackVertexShader[1] ;
DG_VERTEXSHADER	DG_ShadowVertexShader[2];
extern unsigned char VERTEX_SHADER_shdw_mk[];	/* 影生成 */
extern unsigned char VERTEX_SHADER_shdw_wt[];	/* 影投影 */
extern unsigned char PIXEL_SHADER_shdwmap0[];	/* 影投影用 */
void DG_InitShadowVertexShader(void)
{
	DWORD dwObjDecl_00[] = {
		/* 通常ライティング */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_SHORT4),		/* position, pad */
		D3DVSD_REG(2,  D3DVSDT_SHORT4),		/* normal,dummy */
//		D3DVSD_REG(7,  D3DVSDT_SHORT2),		/* uv0(dummy) */
//		D3DVSD_REG(8,  D3DVSDT_SHORT2),		/* uv1(dummy) */
//		D3DVSD_REG(9,  D3DVSDT_SHORT2),		/* uv2(dummy) */
		D3DVSD_END()
	};
	DWORD dwObjDecl_00_Fvf[] = {
		/* 通常ライティング */
		D3DVSD_STREAM(0),
		D3DVSD_REG(0,  D3DVSDT_FLOAT4),		/* position, pad */
		D3DVSD_REG(2,  D3DVSDT_FLOAT3),		/* normal */
//		D3DVSD_REG(7,  D3DVSDT_FLOAT2),		/* uv0(dummy) */
//		D3DVSD_REG(8,  D3DVSDT_FLOAT2),		/* uv1(dummy) */
//		D3DVSD_REG(9,  D3DVSDT_FLOAT2),		/* uv2(dummy) */
		D3DVSD_END()
	};

	if( !DG_CheckUseShadow() ){ return ; }	// 使用しないので終了

	if( DG_CheckObjUseVertexShader() )
	{
		/* 影生成レンダリング */
		DG_MakeVertexShader( &DG_ShadowVertexShader[0], VSHT_wshdw_mk, dwObjDecl_00 );
	}

	if( DG_CheckShadowWtUseVertexShader() )
	{
		DWORD	vsh_id ;
		DWORD	psh_id ;
		DWORD	*decl ;

		/* 影投影レンダリング用 */
		if( DG_GetMultiTexMax() >= 4 )
		{
			/* 豪華版(Texture x3(+裏判定用x1) 使用) */
			if( DG_CheckObjUseVertexShader() )
			{
				vsh_id = VSHT_wshdw_wt ;
				decl = dwObjDecl_00 ;
			}
			else
			{
				vsh_id = VSHT_wshdw_wt_fvf ;
				decl = dwObjDecl_00_Fvf ;
			}
			psh_id = PSHT_shdwmap0 ;
		}
		else
		{
			/* 簡素版(Texture x1 使用) */

			if( DG_CheckObjUseVertexShader() )
			{
				vsh_id = VSHT_wshdw_wt_1tex ;
				decl = dwObjDecl_00 ;
			}
			else
			{
				vsh_id = VSHT_wshdw_wt_fvf_1tex ;
				decl = dwObjDecl_00_Fvf ;
			}
			psh_id = PSHT_wshdwmap0_1tex ;
		}

		/* Shader作成 */
		DG_MakeVertexShader(&DG_ShadowVertexShader[1], vsh_id, decl) ;
		DG_MakePixelShader(&DG_ShadowPixelShader[0],   psh_id );
	}

	/* 影作成描画関数選択 */
	if( DG_CheckObjUseVertexShader() )
	{
		if( DG_CheckIndexPrimitiveUseable() ){ _mk_chain_obj_func = ChainObj ; }
		else{ _mk_chain_obj_func = ChainObjNIdx ; }
	}
	else
	{
		/* VertexShader非対応版 */
		if( DG_CheckIndexPrimitiveUseable() ){ _mk_chain_obj_func = ChainObjNVS ; }
		else{ _mk_chain_obj_func = ChainObjNVSNIdx ; }
	}

	/* 影投影描画関数選択 */
	if( DG_CheckShadowWtUseVertexShader() )
	{
		if( DG_CheckObjUseVertexShader() )
		{
			if( DG_CheckIndexPrimitiveUseable() ){ _wt_chain_obj_func = ChainObj ; }
			else{ _wt_chain_obj_func = ChainObjNIdx ; }
		}
		else
		{
			if( DG_CheckIndexPrimitiveUseable() ){ _wt_chain_obj_func = ChainObjFvf ; }
			else{ _wt_chain_obj_func = ChainObjFvfNIdx ; }
		}
	}
	else
	{
		/* VertexShader非対応版 */
		if( DG_CheckIndexPrimitiveUseable() ){ _wt_chain_obj_func = ChainObjNVS ; }
		else{ _wt_chain_obj_func = ChainObjNVSNIdx ; }
	}

	/* SpotLight描画関数選択 */
	if( DG_CheckUseVertexShader() )
	{
		_chain_spotlight_func = ChainSpotLight ;
	}
	else
	{
		_chain_spotlight_func = ChainSpotLightNVS ;
	}
}

void DG_ReleaseShadowVertexShader(void)
{
	DG_KillVertexShader( &DG_ShadowVertexShader[0] );
	DG_KillVertexShader( &DG_ShadowVertexShader[1] );

	DG_KillPixelShader( &DG_ShadowPixelShader[0] );
}

static void DG_InitChanlRenderState( DG_CHANL *cp, int flag )
{
	static FVECTOR VS_Const[] = {
		{0.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.5f, 0.5f, 0.5f, 0.5f},
#ifndef _WINDOWS
		{1.0f/4096.0f, 1.0f/255.0f, 1.0f/32767.0f, 128.0f},
#else
		{1.0f/4096.0f, 1.0f/255.0f, 1.0f/32767.0f, 128.0f/256.0f},
#endif
	};
	static FVECTOR	ShadowShiftParam[4] = {
#ifndef _WINDOWS
		{ 128.0f + 0.0f, 128.0f + 0.5f, 0, 0 },	/* シャドウマップシフト量（０度） */
		{ 128.0f - 0.25f, 128.0f + 0.433, 0, 0 },	/* シャドウマップシフト量（１２０度） */
		{ 128.0f - 0.25f, 128.0f - 0.433, 0, 0 },	/* シャドウマップシフト量（２４０度） */
		{ 1.0f/4000.0f, 0, 0, 0 }	/* シャドウフォーカス基準距離（未使用） */
#else
		{ (128.0f + 0.0f)/256.0f, (128.0f + 0.5f)/256.0f, 0, 0 },
		{ (128.0f - 0.25f)/256.0f, (128.0f + 0.433f)/256.0f, 0, 0 },
		{ (128.0f - 0.25f)/256.0f, (128.0f - 0.433f)/256.0f, 0, 0 },	
		{ 1.0f/4000.0f, 0, 0, 0 }	/* シャドウフォーカス基準距離（未使用） */
#endif
	};

	DG_SetRenderState(D3DRS_FOGENABLE, 0 );

	/* 頂点シェーダー用各種定数設定 */
	if( DG_CheckObjUseVertexShader() || DG_CheckShadowWtUseVertexShader() )
	{
		DG_SetVertexShaderConstant(CV_EYEPERS, &cp->raise_eye_xpers, 4);	/* 優先上げ */
		DG_SetVertexShaderConstant(CV_PERS, &cp->raise_xpers, 4);			/* 優先上げ */

		DG_SetVertexShaderConstant(CV_ZERO, &VS_Const[0], 1);
		DG_SetVertexShaderConstant(CV_ONE, &VS_Const[1], 1);
		DG_SetVertexShaderConstant(CV_HALF, &VS_Const[2], 1);
		DG_SetVertexShaderConstant(CV_SCALE, &VS_Const[3], 1);
		DG_SetVertexShaderConstant(CV_FOG, &DG_FogParam, 1);
		DG_SetVertexShaderConstant(CV_CONST0, &ShadowShiftParam, 4 );

		/* Fog 初期化 */
		if( DG_CheckObjUseVertexShader() && DG_CheckShadowWtUseVertexShader() )
		{
			float fog_start = 0.0f;
			float fog_end   = 1.0f;
			DG_SetRenderState(D3DRS_FOGSTART, *(DWORD *)&fog_start);
			DG_SetRenderState(D3DRS_FOGEND,   *(DWORD *)&fog_end);
		}
	}

	if( !DG_CheckObjUseVertexShader() || !DG_CheckShadowWtUseVertexShader() )
	{
		DG_SetTransform(D3DTS_PROJECTION, &cp->raise_xpers );
		DG_SetTransform(D3DTS_VIEW,       &cp->eye_inv );

		/* Fog */
		DG_SetRenderState(D3DRS_FOGSTART, *((DWORD *)&DG_FogStart)) ;
		DG_SetRenderState(D3DRS_FOGEND,   *((DWORD *)&DG_FogEnd)) ;
	}

	/* ピクセルシェーダー設定 */
	DG_SetPixelShader( NULL );
}

/*----------------------------------------------------------------*/
/* PS2形式からXBOX形式へ変換 */
#define EXCHG_COLOR(_c)	( ((_c)&0xff00ff00) | (((_c)&0xff)<<16) | (((_c)&0xff0000)>>16) )

/* ２Ｄ描画補助 */
static void Init2DDraw( void )
{
	static FVECTOR screen_param[2] = {
		{-1.0f, 1.0f, 1.0f, 1.0f},
		{2.0f/256.0f,-2.0f/256.0f,0.0f,0.0f}
	};
#ifndef _WINDOWS
	static FVECTOR tex_param[2] = {
		{256.0f/16384.0f,256.0f/16384.0f,1,1},	/* テクスチャスケール */
		{0.0f,0.0f,0,0}		/* テクスチャオフセット */
	};
#else
	static FVECTOR tex_param[2] = {
		{1.0f/16384.0f,1.0f/16384.0f,1,1},	/* テクスチャスケール */
		{0.0f,0.0f,0,0}		/* テクスチャオフセット */
	};
#endif
	DG_SetVertexShaderConstant(CV_TEX0_SCALE, tex_param, 2);
	DG_SetVertexShaderConstant(0, screen_param, 2);

	/* VertexProcessing設定  */
	DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
						DG_CheckVertexBufferSoftProcessing()) ;

	DG_SelectVertexShader( &DG_DmapackVertexShader[0] );
}

static void Init2DDrawNVS( void )
{
	static FMATRIX screen_param_mtx =
	{
		2.0f/256.0f,	0.0f,			0.0f,	0.0f,
		0.0f,			-2.0f/256.0f,	0.0f,	0.0f,
		0.0f,			0.0f,			0.0f,	0.0f,
		-1.0f,			1.0f,			1.0f,	1.0f
	};
#ifndef _WINDOWS
	static FMATRIX tex_param_mtx =
	{
		256.0f/16384.0f,	0.0f,				0.0f,	0.0f,
		0.0f,				256.0f/16384.0f,	0.0f,	0.0f,
		0.0f,				0.0f,				1.0f,	0.0f,
		0.0f,				0.0f,				0.0f,	1.0f
	};
#else
	static FMATRIX tex_param_mtx =
	{
		1.0f/16384.0f,	0.0f,			0.0f,	0.0f,
		0.0f,			1.0f/16384.0f,	0.0f,	0.0f,
		0.0f,			0.0f,			1.0f,	0.0f,
		0.0f,			0.0f,			0.0f,	1.0f
	};
#endif
	D3DXMATRIX	d3dx_mtx ;

	D3DXMatrixOrthoOffCenterLH(&d3dx_mtx, -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f) ;

	DG_SetTransform(D3DTS_PROJECTION, (FMATRIX *)d3dx_mtx.m) ;
	DG_SetTransform(D3DTS_VIEW,       &DG_UnitMatrix) ;
	DG_SetTransform(D3DTS_WORLD,      (void *)&screen_param_mtx) ;

	DG_SetTransformTexCoord(D3DTS_TEXTURE0,   (void *)&tex_param_mtx) ;


	/* VertexProcessing設定  */
	DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
						DG_CheckVertexBufferSoftProcessing()) ;

	DG_SetVertexShader(D3DFVF_DG_VERTEX_DMAPACK2D_FLAG) ;

	DG_SetRenderState(D3DRS_LIGHTING, FALSE) ;	// 光源計算OFF

	DG_SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2) ;
}

static void DrawSpot( u_int color )
{
	DG_VERTEX_DMAPACK2D		*vertex;
	DG_VERTEX_DMAPACK2D		vertex_buff[18];
	int		i ;
	float	a, s, c ;
	DWORD	sofs ;

	if ( DG_SpotLightBaseTexture == NULL ){
		DG_SetTexture( 0, NULL );

		vertex = vertex_buff ;
		vertex->x = 128 ;
		vertex->y = 128 ;
		vertex->rgba = EXCHG_COLOR(color) ;
		vertex++ ;
		for ( i = 1 ; i < 18 ; i++ ){
			a = (float)((i-1)&15) / 8.0f * (float)3.14159265f ; 
			DG_SinCos( &s, &c, a );
			vertex->x = (short)(128.0f + c * 120.0f) ;
			vertex->y = (short)(128.0f + s * 120.0f) ;
			vertex->rgba = 0x80000000 ;
			vertex++ ;
		}

		DG_SetDynamicVertexBuffer(vertex_buff, sizeof(DG_VERTEX_DMAPACK2D), 18, &sofs) ;
		DG_DrawPrimitive(D3DPT_TRIANGLEFAN, sofs, 18 - 2) ;
	} else {
		/* グラデーションサークルの代わりに背景イメージを使用する場合 */

		if( DG_LinerTextureGetImageDirty(DG_SpotLightBaseTexture) )
		{
			DG_LinerTextureFlushImage(DG_SpotLightBaseTexture) ;
			DG_LinerTextureClearImageDirty(DG_SpotLightBaseTexture) ;
		}
		DG_SetTextureDirect( 0, DG_SpotLightBaseTexture->d3dtexture );

		color = EXCHG_COLOR(color) ;

		vertex = vertex_buff ;

		vertex->x    = 0 ;
		vertex->y    = 0 ;
		vertex->rgba = color ;
		vertex->u    = 0 ;
		vertex->v    = 0 ;
		vertex++ ;

		vertex->x    = 256 ;
		vertex->y    = 0 ;
		vertex->rgba = color ;
		vertex->u    = 16384 ;
		vertex->v    = 0 ;
		vertex++ ;

		vertex->x    = 0 ;
		vertex->y    = 256 ;
		vertex->rgba = color ;
		vertex->u    = 0 ;
		vertex->v    = 16384 ;
		vertex++ ;

		vertex->x    = 256 ;
		vertex->y    = 256 ;
		vertex->rgba = color ;
		vertex->u    = 16384 ;
		vertex->v    = 16384 ;
		vertex++ ;

		DG_SetDynamicVertexBuffer(vertex_buff, sizeof(DG_VERTEX_DMAPACK2D), 4, &sofs) ;
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, 2) ;
	}
}

static void DrawSpotNVS( u_int color )
{
	DG_VERTEX_DMAPACK2D_FVF		*vertex;
	DG_VERTEX_DMAPACK2D_FVF		vertex_buff[18];
	int		i ;
	float	a, s, c ;
	DWORD	sofs ;

	if ( DG_SpotLightBaseTexture == NULL ){
		DG_SetTextureNVS( 0, NULL );

		vertex = vertex_buff ;

		vertex->x = 128 ;
		vertex->y = 128 ;
		vertex->z = 0 ;
		vertex->rgba = EXCHG_COLOR(color) ;
		vertex++ ;
		for ( i = 1 ; i < 18 ; i++ ){
			a = (float)((i-1)&15) / 8.0f * (float)3.14159265f ; 
			DG_SinCos( &s, &c, a );
			vertex->x = (short)(128.0f + c * 120.0f) ;
			vertex->y = (short)(128.0f + s * 120.0f) ;
			vertex->z = 0 ;
			vertex->rgba = 0x80000000 ;
			vertex++ ;
		}

		DG_SetDynamicVertexBuffer(vertex_buff,
					sizeof(DG_VERTEX_DMAPACK2D_FVF), 18, &sofs) ;
		DG_DrawPrimitive(D3DPT_TRIANGLEFAN, sofs, 18 - 2) ;
	} else {
		/* グラデーションサークルの代わりに背景イメージを使用する場合 */
		if( DG_LinerTextureGetImageDirty(DG_SpotLightBaseTexture) )
		{
			DG_LinerTextureFlushImage(DG_SpotLightBaseTexture) ;
			DG_LinerTextureClearImageDirty(DG_SpotLightBaseTexture) ;
		}
		DG_SetTextureDirect( 0, DG_SpotLightBaseTexture->d3dtexture );

		color = EXCHG_COLOR(color) ;

		vertex = vertex_buff ;

		vertex->x    = 0.0f ;
		vertex->y    = 0.0f ;
		vertex->z    = 0.0f ;
		vertex->rgba = color ;
		vertex->u    = 0.0f ;
		vertex->v    = 0.0f ;
		vertex++ ;

		vertex->x    = 256.0f ;
		vertex->y    = 0.0f ;
		vertex->z    = 0.0f ;
		vertex->rgba = color ;
		vertex->u    = 16384.0f ;
		vertex->v    = 0.0f ;
		vertex++ ;

		vertex->x    = 0.0f ;
		vertex->y    = 256.0f ;
		vertex->z    = 0.0f ;
		vertex->rgba = color ;
		vertex->u    = 0.0f ;
		vertex->v    = 16384.0f ;
		vertex++ ;

		vertex->x    = 256.0f ;
		vertex->y    = 256.0f ;
		vertex->z    = 0.0f ;
		vertex->rgba = color ;
		vertex->u    = 16384.0f ;
		vertex->v    = 16384.0f ;
		vertex++ ;

		DG_SetDynamicVertexBuffer(vertex_buff, sizeof(DG_VERTEX_DMAPACK2D_FVF), 4, &sofs) ;
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, 2) ;
	}
}


static	void	ChainSpotLight(DG_SPOT *spot)
{
	Init2DDraw();
	DG_SetAlphaMode( 0 );
	DrawSpot( spot->color );
	DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ) );
}

static	void	ChainSpotLightNVS(DG_SPOT *spot)
{
	Init2DDrawNVS();
	DG_SetAlphaMode( 0 );
	DrawSpotNVS( spot->color );
	DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ) );
}

/* ---------------------------------------------------------------- */
	/*
		パケット初期化関連
	*/



	/*
		ChainObj系Material初期化
	*/
static	void	ChainObj_InitMatrial( DG_OBJ *obj, int type )
{
	ScrpadWork	*scrpad = (ScrpadWork *)SCRPAD_ADDR ;

	DG_SetStaticVertexBuffer( 0, obj->d3d_vbuff,  obj->stride );
	DG_SelectVertexShader( &DG_ShadowVertexShader[type] );

	/* マトリクスの設定 */
#if !__DG_OBJ_VERTEX_WEIGHT_REV__
	DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
	if (obj->parent != -1) {
		DG_SetVertexShaderConstant(CV_WORLD1, &obj->inv_mat, 4);
	}
#else
	DG_SetVertexShaderConstant(CV_WORLD1, &obj->world, 4);
	if (obj->parent != -1) {
		if ( !(obj->mdl_type & DG_TYPE_REVWT_WT0) ) {
			DG_SetVertexShaderConstant(CV_WORLD0, &obj->inv_mat, 4);
		} else {
			/* 計算誤差を防ぐ為の処置 */
			DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
		}
	} else {
		DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
	}
#endif
	DG_SetVertexShaderConstant(CV_LIGHTVEC, &scrpad->local_light, 4);
	DG_SetVertexShaderConstant(CV_LIGHTCOL, &scrpad->local_color, 4);
}


	/*
		ＤＭＡに接続する
	*/
static	void	ChainObj( DG_OBJ *obj, int type )
{
	ScrpadWork	*scrpad = (ScrpadWork *)SCRPAD_ADDR ;
	DG_OBJ_PACKET	*pack ;
	int				j ;
	int				index_ofs ;

	pack = obj->packets ;
	if ( pack == NULL ) return ;

	/* Material初期化 */
	ChainObj_InitMatrial(obj, type) ;

	/* モデルの描画処理 */
	DG_SetStaticIndexBuffer(obj->d3d_ibuff, 0) ;
	index_ofs = 0 ;
	for ( j = obj->n_packs; j > 0; j --, pack++ ){
		if ( pack->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
		} else {
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
		}
		DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
						pack->idx_vtx_min, pack->idx_vtx_num,
						index_ofs, pack->n_indices - 2) ;
		index_ofs += pack->n_indices ;
	}

	/* カリング禁止 */
	DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
}

static	void	ChainObjNIdx( DG_OBJ *obj, int type )
{
	ScrpadWork	*scrpad = (ScrpadWork *)SCRPAD_ADDR ;
	DG_OBJ_PACKET	*pack ;
	int				j ;
	int				index_ofs ;

	pack = obj->packets ;
	if ( pack == NULL ) return ;

	/* Material初期化 */
	ChainObj_InitMatrial(obj, type) ;

	/* モデルの描画処理 */
	index_ofs = 0 ;
	for ( j = obj->n_packs; j > 0; j --, pack++ ){
		if ( pack->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
		} else {
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
		}
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP,
						index_ofs, pack->n_indices - 2) ;
		index_ofs += pack->n_indices ;
	}

	/* カリング禁止 */
	DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
}

	/*
		ChainObjNVS系Material初期化
	*/
static _forceinline	void	ChainObjNVS_InitMatrial( DG_OBJ *obj, int type )
{
	ScrpadWork	*scrpad = (ScrpadWork *)SCRPAD_ADDR ;

	/* マトリクスの設定 */
#if !__DG_OBJ_VERTEX_WEIGHT_REV__
	DG_SetTransform(D3DTS_WORLDMATRIX(0), (FMATRIX *)&obj->world) ;
	if (obj->parent != -1) {
		DG_SetTransform(D3DTS_WORLDMATRIX(1), (FMATRIX *)&obj->inv_mat) ;
		DG_SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS ) ;
	} else {
		DG_SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_DISABLE ) ;
	}
#else
	if (obj->parent != -1) {
		DG_SetTransform(D3DTS_WORLDMATRIX(0), (FMATRIX *)&obj->inv_mat) ;
		DG_SetTransform(D3DTS_WORLDMATRIX(1), (FMATRIX *)&obj->world) ;
		DG_SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS ) ;
	} else {
		DG_SetTransform(D3DTS_WORLDMATRIX(0), (FMATRIX *)&obj->world) ;
		DG_SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_DISABLE ) ;
	}
#endif

	/* 光源設定 */
	if( type == 0 )
	{
		/* 影Mask作成 */
		DG_SetLightMatrix2Direct3D(&scrpad->local_light,
								   &scrpad->local_color) ;
	}
}

	/*
		ChainObjNVS系VertexBuffer設定
	*/
static	void	ChainObjNVS_MakeShadow_SetVertexBuffer( DG_OBJ *obj )
{
	DWORD					fvf_flag ;
	LPDIRECT3DVERTEXBUFFER8	vbuff ;
	DWORD					stride ;

	/* 頂点設定 */
	if( obj->stride == sizeof(DG_VERTEX_KMSS) )
	{
		fvf_flag = D3DFVF_DG_VERTEX_KMSS_FLAG ;
		stride   = sizeof(DG_VERTEX_KMSS_FVF) ;
	}
	else
	{
		fvf_flag = D3DFVF_DG_VERTEX_KMSM_FLAG ;
		stride   = sizeof(DG_VERTEX_KMSM_FVF) ;
	}

	if( obj->flag & DG_FLAG_MULTITEX )
	{
		if( obj->rgbs ){ vbuff = obj->d3d_cvbuff ; }
		else{ vbuff = obj->d3d_vbuff ; }
	}
	else
	{
		if( obj->flag & DG_FLAG_PAINT ){ vbuff = obj->d3d_cvbuff ; }
		else{ vbuff = obj->d3d_vbuff ; }
	}

	DG_SetVertexShader(fvf_flag) ;
	DG_SetStaticVertexBuffer(0, vbuff, stride) ;
}


static	DWORD	ChainObjNVS_WriteShadow_SetVertexBuffer( DG_OBJ *obj )
{
	DWORD	fvf_flag ;
	void	*dst ;
	DWORD	sofs ;
	DWORD	n_verts ;
	DWORD	stride ;

	/* 頂点設定 */
	fvf_flag = DG_VERTEX_PRJSHDW01_FLAG ;
	stride   = sizeof(DG_VERTEX_PRJSHDW01) ;

	n_verts = obj->n_verts ;

	ASSERT( DG_GetWorkBufferSize() > (stride * n_verts) ) ;
	dst = DG_GetWorkBuffer() ;	// 作業用メモリ

	/*-- CPUで計算 --*/
	CalcKMS2PRJSHDW01(obj, dst, obj->vbuff, obj->stride, n_verts) ;

	/*-- Stream設定 --*/
	DG_SetVertexShader(fvf_flag) ;
	DG_SetDynamicVertexBuffer(dst, stride, n_verts, &sofs) ;

	return(sofs) ;
}

static	DWORD	ChainObjNVSNIdx_WriteShadow_SetVertexBuffer( DG_OBJ *obj )
{
	DWORD	fvf_flag ;
	void	*dst ;
	DWORD	sofs ;
	DWORD	n_verts ;
	DWORD	stride ;
	void	*tmp_dst ;
	DWORD	n_indices ;

	/* 頂点設定 */
	fvf_flag = DG_VERTEX_PRJSHDW01_FLAG ;
	stride   = sizeof(DG_VERTEX_PRJSHDW01) ;

	n_verts   = obj->n_verts ;
	n_indices = obj->n_indices ;

	ASSERT( DG_GetWorkBufferSize() > (stride * (n_verts + n_indices)) ) ;
	dst = DG_GetWorkBuffer() ;	// 作業用メモリ

	tmp_dst = ((BYTE *)dst) + (stride * n_indices) ;

	/*-- CPUで計算 --*/
	CalcKMS2PRJSHDW01(obj, tmp_dst, obj->vbuff, obj->stride, n_verts) ;

	/*-- 並べ替え --*/
	DG_ArrangeVertexByIndex(dst, tmp_dst, stride,
						obj->index, n_indices) ;	// 並べ替え

	/*-- Stream設定 --*/
	DG_SetVertexShader(fvf_flag) ;
	DG_SetDynamicVertexBuffer(dst, stride, n_indices, &sofs) ;

	return(sofs) ;
}

	/*
		ＤＭＡに接続する(VertexShader非対応版)
	*/
static	void	ChainObjNVS( DG_OBJ *obj, int type )
{
	DG_OBJ_PACKET	*pack ;
	int				j ;
	int				index_ofs ;
	DWORD			sofs ;

	pack = obj->packets ;
	if ( pack == NULL ) return ;
	
	/* マテリアル初期化 */
	ChainObjNVS_InitMatrial(obj, type) ;

	/* 頂点設定 */
	if( type == 0 )
	{
		ChainObjNVS_MakeShadow_SetVertexBuffer(obj) ;
		sofs = 0 ;
	}
	else
	{
		sofs = ChainObjNVS_WriteShadow_SetVertexBuffer(obj) ;
	}

	/* モデルの描画処理 */
	DG_SetStaticIndexBuffer(obj->d3d_ibuff, sofs) ;
	index_ofs = 0 ;
	for ( j = obj->n_packs; j > 0; j --, pack++ ){
		if ( pack->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
		} else {
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
		}
		DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
						pack->idx_vtx_min, pack->idx_vtx_num,
						index_ofs, pack->n_indices - 2) ;
		index_ofs += pack->n_indices ;
	}

	/* カリング禁止 */
	DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );

}

static	void	ChainObjNVSNIdx( DG_OBJ *obj, int type )
{
	DG_OBJ_PACKET	*pack ;
	int				j ;
	int				index_ofs ;

	pack = obj->packets ;
	if ( pack == NULL ) return ;

	/* マテリアル初期化 */
	ChainObjNVS_InitMatrial(obj, type) ;

	/* 頂点設定 */
	if( type == 0 )
	{
		ChainObjNVS_MakeShadow_SetVertexBuffer(obj) ;
		index_ofs =  0 ;
	}
	else
	{
		index_ofs = ChainObjNVSNIdx_WriteShadow_SetVertexBuffer(obj) ;
	}

	/* モデルの描画処理 */
	for ( j = obj->n_packs; j > 0; j --, pack++ ){
		if ( pack->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
		} else {
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
		}
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP,
						index_ofs, pack->n_indices - 2) ;
		index_ofs += pack->n_indices ;
	}

	/* カリング禁止 */
	DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );

}


	/*
		ChainObjFvf系Material初期化
	*/
static	void	ChainObjFvf_InitMatrial( DG_OBJ *obj, int type )
{
	ScrpadWork	*scrpad = (ScrpadWork *)SCRPAD_ADDR ;
	LPDIRECT3DVERTEXBUFFER8	vbuff ;
	DWORD					stride ;

	/* 頂点設定 */
	if( obj->stride == sizeof(DG_VERTEX_KMSS) )
	{
		stride = sizeof(DG_VERTEX_KMSS_FVF) ;
	}
	else
	{
		stride = sizeof(DG_VERTEX_KMSM_FVF) ;
	}

	if( obj->flag & DG_FLAG_MULTITEX )
	{
		if( obj->rgbs ){ vbuff = obj->d3d_cvbuff ; }
		else{ vbuff = obj->d3d_vbuff ; }
	}
	else
	{
		if( obj->flag & DG_FLAG_PAINT ){ vbuff = obj->d3d_cvbuff ; }
		else{ vbuff = obj->d3d_vbuff ; }
	}

	DG_SelectVertexShader( &DG_ShadowVertexShader[type] );
	DG_SetStaticVertexBuffer(0, vbuff, stride) ;

	/* マトリクスの設定 */
#if !__DG_OBJ_VERTEX_WEIGHT_REV__
	DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
	if (obj->parent != -1) {
		DG_SetVertexShaderConstant(CV_WORLD1, &obj->inv_mat, 4);
	}
#else
	DG_SetVertexShaderConstant(CV_WORLD1, &obj->world, 4);
	if (obj->parent != -1) {
		if ( !(obj->mdl_type & DG_TYPE_REVWT_WT0) ) {
			DG_SetVertexShaderConstant(CV_WORLD0, &obj->inv_mat, 4);
		} else {
			/* 計算誤差を防ぐ為の処置 */
			DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
		}
	} else {
		DG_SetVertexShaderConstant(CV_WORLD0, &obj->world, 4);
	}
#endif
	DG_SetVertexShaderConstant(CV_LIGHTVEC, &scrpad->local_light, 4);
	DG_SetVertexShaderConstant(CV_LIGHTCOL, &scrpad->local_color, 4);
}


	/*
		ＤＭＡに接続する(FVF頂点版)
	*/
static	void	ChainObjFvf( DG_OBJ *obj, int type )
{
	ScrpadWork	*scrpad = (ScrpadWork *)SCRPAD_ADDR ;
	DG_OBJ_PACKET	*pack ;
	int				j ;
	int				index_ofs ;

	pack = obj->packets ;
	if ( pack == NULL ) return ;

	/* Material初期化 */
	ChainObjFvf_InitMatrial(obj, type) ;

	/* モデルの描画処理 */
	DG_SetStaticIndexBuffer(obj->d3d_ibuff, 0) ;
	index_ofs = 0 ;
	for ( j = obj->n_packs; j > 0; j --, pack++ ){
		if ( pack->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
		} else {
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
		}
		DG_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,
						pack->idx_vtx_min, pack->idx_vtx_num,
						index_ofs, pack->n_indices - 2) ;
		index_ofs += pack->n_indices ;
	}

	/* カリング禁止 */
	DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
}

static	void	ChainObjFvfNIdx( DG_OBJ *obj, int type )
{
	ScrpadWork	*scrpad = (ScrpadWork *)SCRPAD_ADDR ;
	DG_OBJ_PACKET	*pack ;
	int				j ;
	int				index_ofs ;

	pack = obj->packets ;
	if ( pack == NULL ) return ;

	/* Material初期化 */
	ChainObjFvf_InitMatrial(obj, type) ;

	/* モデルの描画処理 */
	index_ofs = 0 ;
	for ( j = obj->n_packs; j > 0; j --, pack++ ){
		if ( pack->flag & ( DG_PACKFLAG_CULLAUTO | DG_PACKFLAG_CULLON ) ){	/* カリングのチェック */
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW );
		} else {
			DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE );
		}
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP,
						index_ofs, pack->n_indices - 2) ;
		index_ofs += pack->n_indices ;
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
	void		(*chain_obj_func)(DG_OBJ *obj, int type) ;
	DWORD		flag ;

	/* 関数選択 */
	chain_obj_func = _mk_chain_obj_func ;

	/* 強制的に光源をなくして黒で描画 */
	GV_ZeroMemory( &scrpad->local_color, sizeof(FMATRIX) );
#ifdef DEBUG_MODE
#if FALSE
	scrpad->local_color.m[3][0] = 128 ;
	scrpad->local_color.m[3][1] = 256 ;
	scrpad->local_color.m[3][2] = 256 ;
#endif
#endif
	if( !DG_CheckObjUseVertexShader() )
	{
		DG_SetRenderState(D3DRS_LIGHTING, TRUE) ;	// 光源計算ON
	}
	
	/* 影処理ではローモデルがあればそれを使う */
	if ( objs->low != NULL ) objs = objs->low ;

	/* テクスチャをなくす */
	DG_SetTexture( 0, NULL );

	obj = objs->objs ;
	for ( i = 0 ; i < objs->n_models ; i++, obj++ ){
#ifndef _WINDOWS
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
#else
		if ( obj->model->type & DG_TYPE_TRANS ) continue ;

		/* その他スキップ条件チェック */
		flag = objs->flag ;
		if ( flag & ( DG_FLAG_SHADOWMAKE & DG_FLAG_PLUGINMASK ) ){
			/* 影生成フラグがついている場合のみ関節の表示・非表示を見るようにする */
			if ( flag & scrpad->invisible_flag ) continue ;
		}

		/* クリップアウトチェック */
		_sceVu0MulMatrix( &scrpad->tmp_mat, &scrpad->light_pers, &obj->world );
		if ( ( bound = scrpad->bound_mode ) == 1 ){
			bound = BoundCheck( &scrpad->tmp_mat, &obj->bound_min, 0 ) ;
		}
		if ( bound == 2 ) continue ;
#endif

		scrpad->env_mat = obj->inv_mat ;
		scrpad->fog = 0x8fff ;
		scrpad->local_bound_mode = bound ;

#ifdef LIBDG_PERFORMANCE
		scrpad->n_packs = 0 ;
		scrpad->n_verts = 0 ;
#endif
		chain_obj_func( obj, 0 );
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
	void		(*chain_obj_func)(DG_OBJ *obj, int type) ;
	DWORD		invisible_flag ;

	/* 関数選択 */
	chain_obj_func = _wt_chain_obj_func ;

	/* 光源設定 */
	if( !DG_CheckShadowWtUseVertexShader() )
	{
		/* Texture投影 */
		DG_SetTransformTexCoord(D3DTS_TEXTURE0, &DG_UnitMatrix) ;
		DG_SetTextureStageState(0,
				D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_PROJECTED|D3DTTFF_COUNT3) ;
	}

	/* 描画 */
	invisible_flag = scrpad->invisible_flag ;
	obj = objs->objs ;
	for ( i = objs->n_models ; i > 0 ; i--, obj++ ){
		/* クリップアウトチェック */
		if ( obj->bound_mode & 2 ) continue ;
		/* その他スキップ条件チェック */
		if ( obj->flag & invisible_flag ) continue ;
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
			scrpad->local_light = scrpad->light_pers2 ;
		}

		/* オブジェクト描画パラメータ構造体にデータをセット */
		scrpad->env_mat = obj->inv_mat ;

		*(FVECTOR*)scrpad->local_color.m[0] = scrpad->color_vector ;
		scrpad->local_color.m[1][3] = scrpad->far_range ;
		/* 光源ベクトルをモデル座標相対に変換 */
		*(FVECTOR*)scrpad->local_color.m[2] = scrpad->light_vector ;
		*(FVECTOR*)scrpad->local_color.m[3] = scrpad->light_pos ;

#ifdef LIBDG_PERFORMANCE
		scrpad->n_packs = 0 ;
		scrpad->n_verts = 0 ;
#endif
		chain_obj_func( obj, 1 );
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
	ScrpadWork	*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJS		/* **oque, */*objs, **oque2 ;
	DG_SPOT		**spot_queue, *spot ;
	DG_OBJ_QUEUE	*que ;
	DG_OBJ_BUFFER	*obj_buff ;
	int			j, k, shadow_id, enable, gid, c_gid ;
	//int		time1, time2, time3, debug_count ;
	OBJ_LIST	*list ;
	LPDIRECT3DSURFACE8	cur_front_surface ;
	LPDIRECT3DSURFACE8	cur_depth_surface ;
	BOOL				mk_use_vertexshader ;
	BOOL				wt_use_vertexshader ;
	void				(*chain_spotlight_func)(DG_SPOT *) ;

	if( !DG_CheckUseShadow() ){ return ; }

	if ( DG_ActiveShadowFlag != 2 ) return ;

	DG_PerfStartD(DGPERF_CTGR_SPOT) ;

	que = cp->obj_queue ;
	if ( que == NULL ) return ;

	mk_use_vertexshader = DG_CheckObjUseVertexShader() ;
	wt_use_vertexshader = DG_CheckShadowWtUseVertexShader() ;

	/*-- 描画関数選択 -------------------------------------------------------*/

	chain_spotlight_func = _chain_spotlight_func ;	// 描画関数選択
	/*-----------------------------------------------------------------------*/

	/*-- カレントのレンダリングターゲット保存 -------------------------------*/

	DG_GetRenderTarget(&cur_front_surface) ;
	DG_GetDepthStencilSurface(&cur_depth_surface) ;
	/*-----------------------------------------------------------------------*/

	obj_buff = &que->objs_buffer ;

	c_gid = DG_CurrentGroupID ;

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

		/*-- 影マスク描画 ---------------------------------------------------*/

		DG_SetRenderTarget( DG_ShadowSurface, DG_ShadowDepthSurface );
		DG_Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
					0x00000000, 0.0f, 0 );
		DG_SetRenderState( D3DRS_ZWRITEENABLE, 1 );		

		/* Vertex Processing設定 */
		DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
							DG_CheckObjVertexBufferSoftProcessing()) ;

		/* 投影マトリクスの設定 */
		if( mk_use_vertexshader )
		{
			DG_SetVertexShaderConstant( CV_EYEPERS, &scrpad->light_pers2, 4 );
		}
		else
		{
			DG_SetTransform(D3DTS_PROJECTION, &scrpad->light_pers2 );
			DG_SetTransform(D3DTS_VIEW,       &DG_UnitMatrix);
		}
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
		/*-------------------------------------------------------------------*/

		/*-- ライトの描画 ---------------------------------------------------*/

		DG_SetRenderState( D3DRS_ZWRITEENABLE, 0 );		
		chain_spotlight_func(spot) ;
		/*-------------------------------------------------------------------*/

		/*-- 影の生成 -------------------------------------------------------*/

		/* Vertex Processing設定 */
		DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
							DG_CheckObjVertexBufferSoftProcessing()) ;

		if( mk_use_vertexshader )
		{
			DG_SetVertexShaderConstant( CV_EYEPERS, &scrpad->light_pers2, 4 );
		}
		else
		{
			DG_SetTransform(D3DTS_PROJECTION, &scrpad->light_pers2 );
			DG_SetTransform(D3DTS_VIEW,       &DG_UnitMatrix);
		}

		//DG_SetRenderState( D3DRS_EDGEANTIALIAS, TRUE );
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
		//DG_SetRenderState( D3DRS_EDGEANTIALIAS, FALSE );
		/*-------------------------------------------------------------------*/

		{/* テクスチャ暈し */
		}

		/*-- 影(投影モデル)描画 ---------------------------------------------*/

		DG_SetRenderTarget( cur_front_surface, cur_depth_surface );

		DG_SetRenderState( D3DRS_ZWRITEENABLE, FALSE );		

		DG_SetViewport( &DG_CurrentViewport );

		if( !wt_use_vertexshader )
		{
			/* Vertex Processing変更 */
			DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
								DG_CheckVertexBufferSoftProcessing()) ;
		}

#if FALSE
		DG_SetTextureDirect( 0, DG_ShadowTexture );
		DG_SetTextureDirect( 1, DG_ShadowTexture );
		DG_SetTextureDirect( 2, DG_ShadowTexture );
#endif
		if( wt_use_vertexshader )
		{
			DG_SetTextureDirect( 0, DG_ShadowTexture );
			if( DG_GetMultiTexMax() > 3 )
			{
				DG_SetTextureDirect( 1, DG_ShadowTexture );
				DG_SetTextureDirect( 2, DG_ShadowTexture );
			}
			DG_SetPixelShader( &DG_ShadowPixelShader[0] );
		}
		else
		{
			DG_SetTextureDirect(0, DG_ShadowTexture) ;
			DG_SetTextureDirect(1, NULL) ;

			DG_SetPixelShader( NULL );

			DG_SetRenderState(D3DRS_LIGHTING, FALSE) ;	// 光源計算OFF
		}
		DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 2, 0, 1, 128 ) );

		/* 投影マトリクスの設定 */
		if( wt_use_vertexshader )
		{
			DG_SetVertexShaderConstant( CV_EYEPERS, &scrpad->raise_eye_pers, 4 );
		}
		else
		{
			DG_SetTransform(D3DTS_PROJECTION, &scrpad->raise_eye_pers );
			DG_SetTransform(D3DTS_VIEW,       &DG_UnitMatrix );
		}

		list = scrpad->list ;
		for ( j = scrpad->max_list ; j > 0 ; j--, list++ ){
			if ( !( list->flag & ( DG_FLAG_SHADOWWRITE & DG_FLAG_PLUGINMASK ) ) ) continue ;
			if ( list->shadow_id != shadow_id ) continue ;
			WriteShadowChainObjs( list->objs );
		}
		/*-------------------------------------------------------------------*/

		DG_SetPixelShader( NULL );
		DG_InitTextureStageState( 0 );
		DG_InitTextureStageState( 1 );
		DG_InitTextureStageState( 2 );
		DG_SetTexture( 0, NULL );
		DG_SetTexture( 1, NULL );
		DG_SetTexture( 2, NULL );
		DG_SetRenderState( D3DRS_ZWRITEENABLE, 1 );		

#if FALSE
		if ( DG_ISCHECK_PACKETBUFFER() ) break ;	/* メモリが足りなくなったら強制終了 */
#endif
	}

	/* 終了パケットの設定 */
	/* 描画環境復元 */
	DG_SetRenderState(D3DRS_FOGENABLE, TRUE);

	if( !wt_use_vertexshader )
	{
		/* Texture投影後始末 */
		DG_SetTextureStageState(0,
				D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2) ;
	}

	/* スタティックプッシュバッファへの記録終了 */
	DG_CloseDmaTask();

#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.spot_projection[0] = scrpad->projection[0] ;
	DG_PerformanceData.spot_projection[1] = scrpad->projection[1] ;
	DG_PerformanceData.shadow_draw[0] = scrpad->shadow_draw[0] ;
	DG_PerformanceData.shadow_draw[1] = scrpad->shadow_draw[1] ;
#endif

	/*-- レンダリングターゲットRelease --------------------------------------*/

#if !__DG_DRAW_IN_EXEBUFFER__	// 実行バッファの場合は解放する必要は無い
	IDirect3DSurface8_Release(cur_front_surface) ;
	IDirect3DSurface8_Release(cur_depth_surface) ;
#endif
	/*-----------------------------------------------------------------------*/

	DG_PerfEndD(DGPERF_CTGR_SPOT) ;
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
	ScrpadWork			*scrpad = (ScrpadWork*)SCRPAD_ADDR ;
	DG_OBJS				/* **oque, */*objs, **oque2 ;
	DG_SPOT				**spot_queue, *spot ;
	DG_OBJ_QUEUE		*que ;
	DG_OBJ_BUFFER		*obj_buff ;
	int					j, k, shadow_id, enable, gid, c_gid ;
	//int				time1, time2, time3, debug_count ;
	OBJ_LIST			*list ;
	LPDIRECT3DSURFACE8	cur_front_surface ;
	LPDIRECT3DSURFACE8	cur_depth_surface ;
	BOOL				mk_use_vertexshader ;
	BOOL				wt_use_vertexshader ;

	if( !DG_CheckUseShadow() ){ return ; }
	if ( DG_ActiveShadowFlag != 1 ) return ;

	DG_PerfStartD(DGPERF_CTGR_SHADOW) ;

	/*-- カレントのレンダリングターゲット保存 -------------------------------*/

	DG_GetRenderTarget(&cur_front_surface) ;
	DG_GetDepthStencilSurface(&cur_depth_surface) ;
	/*-----------------------------------------------------------------------*/

	que = cp->obj_queue ;
	if ( que == NULL ) return ;
	obj_buff = &que->objs_buffer ;

	c_gid = DG_CurrentGroupID ;

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

	mk_use_vertexshader = DG_CheckObjUseVertexShader() ;
	wt_use_vertexshader = DG_CheckShadowWtUseVertexShader() ;

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

		/*-- 影マスク描画 ---------------------------------------------------*/

		/* VertexProcessing設定  */
		DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
							DG_CheckObjVertexBufferSoftProcessing()) ;

		DG_SetRenderTarget( DG_ShadowSurface, DG_ShadowDepthSurface );
		DG_Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
				0x0000000, 0.0f, 0 );
		DG_SetRenderState( D3DRS_ZWRITEENABLE, 1 );		

		/* 投影マトリクスの設定 */
		if( mk_use_vertexshader )
		{
			DG_SetVertexShaderConstant( CV_EYEPERS, &scrpad->light_pers2, 4 );
		}
		else
		{
			DG_SetTransform(D3DTS_PROJECTION, &scrpad->light_pers2 );
			DG_SetTransform(D3DTS_VIEW,       &DG_UnitMatrix);
		}
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

		/*-------------------------------------------------------------------*/

		/*-- ライトの描画 ---------------------------------------------------*/

		DG_SetRenderState( D3DRS_ZWRITEENABLE, 0 );		
		DG_Clear(0, NULL, D3DCLEAR_TARGET, 0x00000000, 0.0f, 0 );
		DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ) );
		/*-------------------------------------------------------------------*/

		/*-- 影の生成 -------------------------------------------------------*/

		/* VertexProcessing設定  */
		DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
							DG_CheckObjVertexBufferSoftProcessing()) ;

		if( mk_use_vertexshader )
		{
			DG_SetVertexShaderConstant( CV_EYEPERS, &scrpad->light_pers2, 4 );
		}
		else
		{
			DG_SetTransform(D3DTS_PROJECTION, &scrpad->light_pers2 );
			DG_SetTransform(D3DTS_VIEW,       &DG_UnitMatrix);
		}

		//DG_SetRenderState( D3DRS_EDGEANTIALIAS, TRUE );
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
						MakeShadowChainObjs( objs );	/* オブジェクトをＤＭＡに登録 */
						objs = objs->next ;				/* 関連付けされた他のオブジェクトも処理 */
						scrpad->bound_mode = 1 ;
					}
				}
			}
		}
		//DG_SetRenderState( D3DRS_EDGEANTIALIAS, FALSE );
		/*-------------------------------------------------------------------*/

		{/* テクスチャ暈し */
		}

		/*-- 影の描画 -------------------------------------------------------*/

		DG_SetRenderTarget( cur_front_surface, cur_depth_surface );
		//DG_Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0x00000000, 0.0f, 0 );
		DG_SetRenderState( D3DRS_ZWRITEENABLE, 0 );		
		DG_SetViewport( &DG_CurrentViewport );

		if( !wt_use_vertexshader )
		{
			/* Vertex Processing変更 */
			DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
								DG_CheckVertexBufferSoftProcessing()) ;
		}

#if FALSE
		DG_SetTextureDirect( 0, DG_ShadowTexture );
		DG_SetTextureDirect( 1, DG_ShadowTexture );
		DG_SetTextureDirect( 2, DG_ShadowTexture );
#endif
		if( wt_use_vertexshader )
		{
			DG_SetTextureDirect( 0, DG_ShadowTexture );
			if( DG_GetMultiTexMax() > 3 )
			{
				DG_SetTextureDirect( 1, DG_ShadowTexture );
				DG_SetTextureDirect( 2, DG_ShadowTexture );
			}
			DG_SetPixelShader( &DG_ShadowPixelShader[0] );

			DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ) );
		}
		else
		{
			DG_SetTextureDirect(0, DG_ShadowTexture) ;
			DG_SetTextureDirect(1, NULL) ;

			DG_SetPixelShader( NULL );

#if FALSE
			DG_SetRenderState(D3DRS_LIGHTING, TRUE) ;	// 光源計算ON
#else
			DG_SetRenderState(D3DRS_LIGHTING, FALSE) ;	// 光源計算ON
#endif
			/* 独自のBlendMode */
#if FALSE
			DG_SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE) ;
			DG_SetRenderState( D3DRS_BLENDOP,          D3DBLENDOP_ADD);
			DG_SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_ZERO) ;
			DG_SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_INVSRCCOLOR);
#else
			DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ) );
#endif
 		}

		/* 投影マトリクスの設定 */
		if( wt_use_vertexshader )
		{
			DG_SetVertexShaderConstant( CV_EYEPERS, &scrpad->raise_eye_pers, 4 );
		}
		else
		{
			DG_SetTransform(D3DTS_PROJECTION, &scrpad->raise_eye_pers );
			DG_SetTransform(D3DTS_VIEW,       &DG_UnitMatrix );
		}

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
		/*-------------------------------------------------------------------*/

		DG_SetPixelShader( NULL );
		DG_SetTexture( 1, NULL );
		DG_SetTexture( 2, NULL );
		DG_InitTextureStageState( 0 );
		DG_InitTextureStageState( 1 );
		DG_InitTextureStageState( 2 );
		DG_SetRenderState( D3DRS_ZWRITEENABLE, 1 );

#if FALSE
		if ( DG_ISCHECK_PACKETBUFFER() ) break ;	/* メモリが足りなくなったら強制終了 */
#endif
	}

	/* 終了パケットの設定 */
	/* 描画環境復元 */
	DG_SetRenderState(D3DRS_FOGENABLE, TRUE);

	if( !wt_use_vertexshader )
	{
		/* Texture投影後始末 */
		DG_SetTextureStageState(0,
				D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2) ;
	}

	/* スタティックプッシュバッファへの記録終了 */
	DG_CloseDmaTask();

	/*-- レンダリングターゲットRelease --------------------------------------*/

#if !__DG_DRAW_IN_EXEBUFFER__	// 実行バッファの場合は解放する必要は無い
	IDirect3DSurface8_Release(cur_front_surface) ;
	IDirect3DSurface8_Release(cur_depth_surface) ;
#endif
	/*-----------------------------------------------------------------------*/

	DG_PerfEndD(DGPERF_CTGR_SHADOW) ;

#ifdef LIBDG_PERFORMANCE
	DG_PerformanceData.spot_projection[0] = scrpad->projection[0] ;
	DG_PerformanceData.spot_projection[1] = scrpad->projection[1] ;
	DG_PerformanceData.shadow_draw[0] = scrpad->shadow_draw[0] ;
	DG_PerformanceData.shadow_draw[1] = scrpad->shadow_draw[1] ;
#endif

}

/*--------------------------------------------------------------------------*/
/*	CalcKMS2PRJSHDW01	PRJSHDW01計算＆出力									*/
/*--------------------------------------------------------------------------*/
static	void	CalcKMS2PRJSHDW01(DG_OBJ *obj, DG_VERTEX_PRJSHDW01 *dst,
							void *src, DWORD stride, int num)
{
	int				i ;
	ScrpadWork		*scrpad = (ScrpadWork *)SCRPAD_ADDR ;
	DG_VERTEX_PRJSHDW01	*dst_vtx ;
	DG_VERTEX_KMSS	*src_vtx ;
	FVECTOR			wpos ;
	FVECTOR			wnrm ;
	FVECTOR			pos ;
	FVECTOR			nrm ;
	float			fo_dist ;
	float			fo_coef ;
	FVECTOR			lgt_dir ;
	FVECTOR			lgt_col ;
	float			coef ;

	fo_dist = scrpad->local_color.m[1][3] ;
	fo_coef = 1.0f/fo_dist ;						// 減衰係数
	lgt_col = *(FVECTOR *)scrpad->local_color.m[0] ;
	lgt_dir = *(FVECTOR *)scrpad->local_color.m[2] ;

	dst_vtx = (DG_VERTEX_PRJSHDW01 *)dst ;
	src_vtx = (DG_VERTEX_KMSS *)src ;
	for(i=num; i>0; i--, dst_vtx++, (BYTE *)src_vtx+=stride)
	{
		/*-- 頂点座標計算 --------------------------------------------------*/

		dst_vtx->wt = (float)src_vtx->wt * (1.0f / 32767.0f) ;

		pos.vx = (float)src_vtx->vx ;
		pos.vy = (float)src_vtx->vy ;
		pos.vz = (float)src_vtx->vz ;
		pos.vw = 1.0f ;
		_sceVu0ApplyMatrix(&wpos, &obj->world, &pos) ;

		// 頂点Weightは無視....
		dst_vtx->x = pos.vx ;
		dst_vtx->y = pos.vy ;
		dst_vtx->z = pos.vz ;
		/*------------------------------------------------------------------*/

		/*-- Texture座標計算 -----------------------------------------------*/

		_sceVu0ApplyMatrix(&pos, &scrpad->local_light, &wpos) ;	// 光源Local射影

		coef = 0.5f * pos.vw ;
		dst_vtx->u0 = (pos.vx *  0.5f) + coef ;
		dst_vtx->v0 = (pos.vy * -0.5f) + coef ;
		dst_vtx->w0 = pos.vw ;

		if( pos.vw <= 0.0f )
		{
			dst_vtx->diffuse = 0x00000000 ;
			continue ;
		}
		/*------------------------------------------------------------------*/

		/*-- 減衰計算 ------------------------------------------------------*/

		if( fo_dist < pos.vw )
		{
			dst_vtx->diffuse = 0x00000000 ;
			continue ;
		}

		nrm.vx = (float)src_vtx->nx ;
		nrm.vy = (float)src_vtx->ny ;
		nrm.vz = (float)src_vtx->nz ;
		nrm.vw = 0.0f ;
		_sceVu0ApplyMatrix(&wnrm, &obj->world, &nrm) ;
		_sceVu0Normalize(&nrm, &wnrm) ;	// 法線正規化

		coef = ((nrm.vx * lgt_dir.vx) + (nrm.vy * lgt_dir.vy) + (nrm.vz * lgt_dir.vz)) ;
		if( coef <= 0.0f )
		{
			dst_vtx->diffuse = 0x00000000 ;
			continue ;
		}
		coef *= (fo_dist - pos.vw) * fo_coef ;

		{
			DWORD	color ;

			color  = ((DWORD)(lgt_col.vx * coef) & 0xff) << 16 ;
			color |= ((DWORD)(lgt_col.vy * coef) & 0xff) << 8 ;
			color |= ((DWORD)(lgt_col.vz * coef) & 0xff) ;
			color |= ((DWORD)(lgt_col.vw * coef) & 0xff) << 24 ;

			dst_vtx->diffuse = color ;
		}
		/*------------------------------------------------------------------*/
	}
}
