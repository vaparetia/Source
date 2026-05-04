/*
	xd3d.cpp

	Direct3D関係ラッパー関数

	2001/05/08 F.Miyauchi / DirectX オリジナル
	$Id: wd3d.c,v 1.50 2002/12/25 13:28:57 takaki Exp $
*/

// 同値設定の回避/コマンド発行数カウント/返値チェック等等のため、
// 頻繁に使用するものはこっちに持ってくる。
// インライン化予定。

#ifndef _XBOX
#include <d3dx8.h>
#else
#include <xtl.h>
#endif

#include <stdio.h>
#include "libgv.h"
#include "libdg.h"
#include "libdg.cnf"
#include "private.h"

#include "shader.h"
#include "rand.h"

//#define WIRE_MODE		/* 強制ワイヤフレームモード有効化 */

#define	__X2W_DEBUG_MESS__	(TRUE)	// X-Box --> Windows検査Message出力

#if TRUE
#define	__STATECHK_STREAM_SOUCE__			(TRUE)	// DG_SetStreamSource状態比較を行う
#define	__STATECHK_INDEICES__				(TRUE)	// DG_SetIndicies状態比較を行う
#define	__STATECHK_VERTEXSHADERCONSTANT__	(TRUE)	// DG_SetVertexShaderConstant
#define	__STATECHK_SETTEXTURESTAGESTATE__	(TRUE)	// DG_SetTextureStageState
#define	__STATECHK_SETTRANSFORM__			(TRUE)	// DG_SetTransform(やらない方がいい？)
#define	__STATECHK_SETLIGHT__				(TRUE)	// DG_SetLight
#define	__STATECHK_LIGHTENABLE__			(TRUE)	// DG_LightEnable
#else
#define	__STATECHK_STREAM_SOUCE__			(FALSE)	// DG_SetStreamSource状態比較を行う
#define	__STATECHK_INDEICES__				(FALSE)	// DG_SetIndicies状態比較を行う
#define	__STATECHK_VERTEXSHADERCONSTANT__	(FALSE)	// DG_SetVertexShaderConstantForce
#define	__STATECHK_SETTEXTURESTAGESTATE__	(FALSE)	// DG_SetTextureStageState
#define	__STATECHK_SETTRANSFORM__			(FALSE)	// DG_SetTransform(やらない方がいい？)
#define	__STATECHK_SETLIGHT__				(FALSE)	// DG_SetLight
#define	__STATECHK_LIGHTENABLE__			(FALSE)	// DG_LightEnable
#endif

static void inline DG_SetTextureRaw( int stage, LPDIRECT3DTEXTURE8 pTexture );

#if	!__X2W_DEBUG_MESS__
#undef	dbgErrMessPuts
#define	dbgErrMessPuts(x_, y_)
#endif

typedef struct tagFSAATYPE {
	D3DMULTISAMPLE_TYPE Type;
	char                *Text;
} FSAATYPE;

static int DG_MultiSampleType = 0;

static DWORD _DG_render_target_clear_flag_mask ;

static DWORD DG_render_target_width  = 640 ;
static DWORD DG_render_target_height = 480 ;

static float DG_render_target_view_nrm_w  = 640.0f ;	// Viewport正規化係数
static float DG_render_target_view_nrm_h  = 480.0f ;

#if __DG_DRAW_IN_EXEBUFFER__
static	LPDIRECT3DSURFACE8	_DG_cur_render_target_surface ;
static	LPDIRECT3DSURFACE8	_DG_cur_depth_surface ;
#endif

/*-- ResetDevice関係宣言 ----------------------------------------------------*/

#define	__RESOURCE_MANAGER__	(FALSE)

#if __RESOURCE_MANAGER__
enum
{
	RESOURCE_ELEM_TYPE_NULL,
	RESOURCE_ELEM_TYPE_TEXTURE,
	RESOURCE_ELEM_TYPE_DEPTHSTENCILSURFACE,
	RESOURCE_ELEM_TYPE_VBUFFER,
	RESOURCE_ELEM_TYPE_IBUFFER,
} ;

typedef	struct	RESOURCE_ELEM_PARAM_TEXTURE_
{
	UINT		Width ;
	UINT		Height ;
	UINT		Levels ;
	DWORD		Usage ;
	D3DFORMAT	Format ;
} RESOURCE_ELEM_PARAM_TEXTURE ;

typedef	struct	RESOURCE_ELEM_PARAM_DEPTHSTENCILSURFACE_
{
	UINT				Width ;
	UINT				Height ;
	D3DFORMAT			Format;
	D3DMULTISAMPLE_TYPE	MultiSample ;
} RESOURCE_ELEM_PARAM_DEPTHSTENCILSURFACE ;

typedef	struct	RESOURCE_ELEM_PARAM_VBUFFER_
{
	UINT	Length ;
	DWORD	Usage ;
	DWORD	FVF ;
} RESOURCE_ELEM_PARAM_VBUFFER ;

typedef	struct	RESOURCE_ELEM_PARAM_IBUFFER_
{
	UINT		Length ;
	DWORD		Usage ;
	D3DFORMAT	Format ;
} RESOURCE_ELEM_PARAM_IBUFFER ;


typedef	union	RESOURCE_ELEM_PARAM_
{
	RESOURCE_ELEM_PARAM_TEXTURE				tex ;
	RESOURCE_ELEM_PARAM_DEPTHSTENCILSURFACE	dp_surf ;
	RESOURCE_ELEM_PARAM_VBUFFER				vbuff ;
	RESOURCE_ELEM_PARAM_IBUFFER				ibuff ;
} RESOURCE_ELEM_PARAM ;

typedef	struct	RESOURCE_ELEM_
{
	DWORD	type ;				// Resource Type

	void	*buff ;				// 確保されたアドレス
	void	**buff_addr ;		// 確保されたアドレスを保持するアドレス

	RESOURCE_ELEM_PARAM	param ;	// リソース作成用のパラメータ
} RESOURCE_ELEM ;

typedef	struct	RESOURCE_TBL_
{
	DWORD			elems_num ;
	DWORD			elems_max ;
	RESOURCE_ELEM	*elems ;	// リンクリストにしてもよかったのですが、
								// アプリの安定性の為にテーブルにしました
} RESOURCE_TBL ;

typedef	struct	RESOURCE_MAN_
{
	#define	MAX_DEFAULTPOOL_RESOURCE_NUM	(32)
	RESOURCE_ELEM	defpool_elems[MAX_DEFAULTPOOL_RESOURCE_NUM] ;
	RESOURCE_TBL	defpool_tbl ;
} RESOURCE_MAN ;

static	RESOURCE_MAN	DG_ResourceMan ;

static	void	DG_InitD3DResourceManager(void) ;
static	void	DG_ReleaseD3DResourceManager(void) ;
static	void	DG_InitD3DResourceTbl(RESOURCE_TBL *tbl, RESOURCE_ELEM *elems, DWORD max) ;
static	void	DG_ReleaseD3DResourceTbl(RESOURCE_TBL *tbl) ;
static	void	DG_QueueElemD3DResourceTbl(RESOURCE_TBL *tbl, DWORD type, void **buff_addr,
						RESOURCE_ELEM_PARAM *param, DWORD param_size) ;
static	void	DG_DequeueElemD3DResourceTbl(RESOURCE_TBL *tbl, void *buff) ;
static	void	DG_ResetDeviceReleaseD3DResourceTbl(RESOURCE_TBL *tbl) ;
static	void	DG_ResetDeviceCreateD3DResourceTbl(RESOURCE_TBL *tbl, D3DPOOL Pool) ;
#endif	// __RESOURCE_MANAGER__

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

#if 0
//XBOXでサポートされる予定のアンチエイリアス
//Windowsとは名前が違う…。
D3DMULTISAMPLE_NONE
D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_LINEAR
D3DMULTISAMPLE_2_SAMPLES_MULTISAMPLE_QUINCUNX
D3DMULTISAMPLE_2_SAMPLES_SUPERSAMPLE_HORIZONTAL_LINEAR 
D3DMULTISAMPLE_2_SAMPLES_SUPERSAMPLE_VERTICAL_LINEAR 
D3DMULTISAMPLE_4_SAMPLES_MULTISAMPLE_LINEAR 
D3DMULTISAMPLE_4_SAMPLES_MULTISAMPLE_GAUSSIAN 
D3DMULTISAMPLE_4_SAMPLES_SUPERSAMPLE_LINEAR 
D3DMULTISAMPLE_4_SAMPLES_SUPERSAMPLE_GAUSSIAN 
D3DMULTISAMPLE_9_SAMPLES_MULTISAMPLE_GAUSSIAN 
D3DMULTISAMPLE_9_SAMPLES_SUPERSAMPLE_GAUSSIAN 
D3DMULTISAMPLE_PREFILTER_FORMAT_DEFAULT 
D3DMULTISAMPLE_PREFILTER_FORMAT_X1R5G5B5 
D3DMULTISAMPLE_PREFILTER_FORMAT_R5G6B5 
D3DMULTISAMPLE_PREFILTER_FORMAT_X8R8B8G8 
D3DMULTISAMPLE_PREFILTER_FORMAT_A8R8G8B8 
#endif

/* フルシーンアンチエイリアス設定サンプル関数 */
void DG_SetFSAA(int type)
{
#ifdef _XBOX
#ifndef _WINDOWS
	// この方法はXBOXのみ。
	// Windowsでは、Reset(); を呼ぶときに頂点バッファやら何やら
	// 解放する必要がある。
	g_d3dpp.MultiSampleType = (D3DMULTISAMPLE_TYPE)type;
	IDirect3DDevice8_Reset( g_pd3dDevice, &g_d3dpp );
#endif	// _WINDOWS
#else
#endif
}

/* パレット関係 */
void DG_CreatePalette( D3DPALETTESIZE Size, LPDIRECT3DPALETTE8 *ppPalette )
{
#ifdef _WINDOWS
	dbgErrMessPuts("CreatePalette: no supported",S_OK) ;
#else
	if ( FAILED( IDirect3DDevice8_CreatePalette( g_pd3dDevice, Size, ppPalette ) ) ) {
		*ppPalette = NULL;
		ASSERT(0);
	}
#endif
}
void DG_SetPalette( DWORD Stage, LPDIRECT3DPALETTE8 pPalette )
{
#ifdef _WINDOWS
	dbgErrMessPuts("SetPalette: no supported",S_OK) ;
#else
	IDirect3DDevice8_SetPalette( g_pd3dDevice, Stage, pPalette );
#endif
}

/* 可視性テスト関係 */
static DWORD VisibilityTestID = 0;

/* DirectX にフォグカラーを設定する */
void DG_SetDxFogColor(void)
{
	CVECTOR color;

	color.r = DG_FogColor.b;
	color.g = DG_FogColor.g;
	color.b = DG_FogColor.r;
	// Xbox
	DG_SetRenderState( D3DRS_FOGCOLOR, *(DWORD *)&color );
}


#define TEXTURE_STAGE_MAX    (4)
#define LIGHT_ID_MAX 		   (4)
static void *DG_Texture[TEXTURE_STAGE_MAX];

static int DG_DrawPrimitiveCount = 0;
static int DG_DrawIndexedPrimitiveCount = 0;
static DWORD VertexShaderHandle;
static DWORD PixelShaderHandle;

static FVECTOR	DG_VertexShaderConstant[MAX_VERTEXSHADER_CONSTANT_NUM];

DWORD	DG_RenderStateValue[MAX_RENDERSTATE_VALUE_NUM];

#define	MAX_TEXTURESTAGESTATE_TYPE	(D3DTSS_RESULTARG)	// DirectX8.1ではこんな感じ
DWORD	DG_TextureStageStateValue[TEXTURE_STAGE_MAX][MAX_TEXTURESTAGESTATE_TYPE];

#if __STATECHK_STREAM_SOUCE__
static	IDirect3DVertexBuffer8	*DG_StreamSourceData[8] ;
static	UINT					DG_StreamSourceStride[8] ;
#endif

#if __STATECHK_INDEICES__
static	IDirect3DIndexBuffer8	*DG_IndeciesData ;
static	UINT					DG_IndeciesBaseVertexIndex ;
#endif

#if __STATECHK_SETLIGHT__
static	D3DLIGHT8	DG_SetLightValue[LIGHT_ID_MAX] ;
#endif

#if __STATECHK_LIGHTENABLE__
static	BOOL		DG_LightEnableValue[LIGHT_ID_MAX] ;
#endif

#if	__STATECHK_SETTRANSFORM__
#define	DG_SETTRANSFORM_VIEW_MAX	(4)
#define	DG_SETTRANSFORM_TEX_MAX		(4)
#define	DG_SETTRANSFORM_WORLD_MAX	(4)

static	FMATRIX		DG_SetTransform_View[DG_SETTRANSFORM_VIEW_MAX] ;
static	FMATRIX		DG_SetTransform_Tex[DG_SETTRANSFORM_TEX_MAX] ;
static	FMATRIX		DG_SetTransform_World[DG_SETTRANSFORM_WORLD_MAX] ;
#endif

/* デバイス再構築時のレンダリングステート/内部変数を初期設定する */
void DG_InitRenderState_ResetDevice(void)
{
	int 	i;
	BOOL	sw ;

	/* 内部変数の初期化 */
	for (i = 0; i < TEXTURE_STAGE_MAX; i++) {
		DG_Texture[i] = NULL;
	}
	DG_DrawPrimitiveCount = 0;
	DG_DrawIndexedPrimitiveCount = 0;

	memset(DG_RenderStateValue,       0xff, sizeof(DG_RenderStateValue)) ;
	memset(DG_TextureStageStateValue, 0xff, sizeof(DG_TextureStageStateValue)) ;
#if __STATECHK_SETLIGHT__
	memset(DG_SetLightValue,          0xff, sizeof(DG_SetLightValue)) ;
#endif
#if __STATECHK_LIGHTENABLE__
	memset(DG_LightEnableValue,       0xff, sizeof(DG_LightEnableValue)) ;
#endif
#if	__STATECHK_SETTRANSFORM__
	memset(DG_SetTransform_View,  0xff, sizeof(DG_SetTransform_View)) ;
	memset(DG_SetTransform_Tex,   0xff, sizeof(DG_SetTransform_Tex)) ;
	memset(DG_SetTransform_World, 0xff, sizeof(DG_SetTransform_World)) ;
#endif
									// 0xffffffffで上手く行かない物は個別対応


	/* Vertex Shader Constant 初期化 */
	DG_RestoreVertexShaderConstant() ;

	/* シェーダ関係初期化 */
	if( DG_CheckVertexBufferSoftProcessing() ){	sw = TRUE ; }
	else{ sw = FALSE ; }
	DG_SetRenderState( D3DRS_SOFTWAREVERTEXPROCESSING ,sw );

	//DG_SetRenderState(D3DRS_NORMALIZENORMALS, TRUE) ;

	/* Material初期化 */
	if( !DG_CheckUseVertexShader() )
	{
		D3DMATERIAL8	mat ;

		mat.Diffuse.r = 1.0f ;
		mat.Diffuse.g = 1.0f ;
		mat.Diffuse.b = 1.0f ;
		mat.Diffuse.a = 1.0f ;
		mat.Ambient.r = 1.0f ;
		mat.Ambient.g = 1.0f ;
		mat.Ambient.b = 1.0f ;
		mat.Ambient.a = 1.0f ;
		mat.Specular.r = 0.0f ;
		mat.Specular.g = 0.0f ;
		mat.Specular.b = 0.0f ;
		mat.Specular.a = 0.0f ;
		mat.Emissive.r = 0.0f ;
		mat.Emissive.g = 0.0f ;
		mat.Emissive.b = 0.0f ;
		mat.Emissive.a = 0.0f ;
		mat.Power    = 0.0f ;

		IDirect3DDevice8_SetMaterial(g_pd3dDevice, &mat) ;

		/* 頂点色の設定 */
		DG_SetRenderState(D3DRS_COLORVERTEX, TRUE) ;

		DG_SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE,  D3DMCS_COLOR1) ;
		DG_SetRenderState(D3DRS_AMBIENTMATERIALSOURCE,  D3DMCS_MATERIAL) ;
	}

	/* レンダリングステート/内部変数を初期設定する */
	DG_InitRenderState() ;
}

/* ゲーム起動時のレンダリングステート/内部変数を初期設定する */
void DG_PowerOnInitRenderState(void)
{
#if	__RESOURCE_MANAGER__
	/* ResetDvice用管理 */
	DG_InitD3DResourceManager() ;
#endif	// __RESOURCE_MANAGER__

	/* 実際の処理はDG_InitRenderState_ResetDevice()に任せる */
	DG_InitRenderState_ResetDevice() ;
}

/* レンダリングステート/内部変数を初期設定する */
void DG_InitRenderState(void)
{
	int 	i;

	/* カリング無し */
	DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	/* フォグ関係 */
	//DG_SetRenderState(D3DRS_FOGENABLE, TRUE);
	DG_SetRenderState(D3DRS_FOGENABLE, FALSE );
	DG_SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
	{
		float fog_start = 0.0f;
		float fog_end = 1.0f;
		DG_SetRenderState(D3DRS_FOGSTART, *(DWORD *)&fog_start);
		DG_SetRenderState(D3DRS_FOGEND, *(DWORD *)&fog_end);
#ifndef _WINDOWS
		DG_SetRenderState(D3DRS_SPECULARENABLE, TRUE);
#else
		DG_SetRenderState(D3DRS_SPECULARENABLE, FALSE);
#endif
	}
	
	/* アルファ関係初期化 */
	DG_InitAlphaMode();

	/* Zバッファ関係 */
	DG_SetRenderState(D3DRS_ZENABLE, TRUE);
	DG_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);	

	/* ライティング無効 */
	DG_SetRenderState(D3DRS_LIGHTING, FALSE);

	/* Z比較式 */
	/* MGSXではZの値が通常のアプリケーションと逆(遠方が0.0f)になる */
	//g_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
	DG_SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);

	/* ステンシル関連 */
	DG_SetRenderState( D3DRS_STENCILENABLE, FALSE );

	/* アンチエイリアス */
	DG_SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, FALSE );
	
	/* 頂点ブレンディング */
	//g_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
	DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);

	/* アルファテスト */
	/* アルファ > 0 の場合のみ描画する */
	//g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	//g_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	//g_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x00000000);
	DG_SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
	DG_SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
	DG_SetRenderState( D3DRS_ALPHAREF, 0x00000000 );

	/* テクスチャ関係 */
	for (i = 0; i < TEXTURE_STAGE_MAX; i++) {
		DG_InitTextureStageState(i);
	}
	/* シェーダ関係初期化 */
	DG_ResetShader();

	DG_SetRenderState( D3DRS_ALPHAREF, 0x00000000 );

#ifdef WIRE_MODE
	DG_SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
#endif
}

void DG_InitTextureStageState(int Stage)
{
	DG_SetTextureStageState(Stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	DG_SetTextureStageState(Stage, D3DTSS_COLORARG2, D3DTA_CURRENT);

	DG_SetTextureStageState(Stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	DG_SetTextureStageState(Stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

	if (Stage != 0) { /* 初期状態では、テクスチャステージステート0以外は無効 */
		DG_SetTextureStageState(Stage, D3DTSS_COLOROP, D3DTOP_DISABLE);
		DG_SetTextureStageState(Stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	} else {
		/* PS2に合わせて、TEXTURE * DIFFUSE * 2 を最終的な色にする */
		DG_SetTextureStageState(Stage, D3DTSS_COLOROP,   D3DTOP_MODULATE2X);
		/* アルファは、TEXTURE ... 0x80, DIFFUSE ... 0x80 で テクスチャの色が出るように */
		DG_SetTextureStageState(Stage, D3DTSS_ALPHAOP,   D3DTOP_MODULATE2X);
	}
	/* 拡大フィルタ/縮小フィルタ */
	DG_SetTextureStageState(Stage, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	DG_SetTextureStageState(Stage, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	//DG_SetTextureStageState(Stage, D3DTSS_MIPFILTER, D3DTEXF_NONE );
	//DG_SetTextureStageState(Stage, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
	DG_SetTextureStageState(Stage, D3DTSS_MIPFILTER, D3DTEXF_POINT);
	/* CLAMPモード */
	DG_SetTextureStageState(Stage, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	DG_SetTextureStageState(Stage, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);

	if( !DG_CheckUseVertexShader() )
	{
		DG_SetTextureStageState(Stage,
				D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2) ;		// Texture座標Transform
		DG_SetTextureStageState(Stage,
				D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU | Stage) ;// Texture座標入力Index
	}

#ifndef _WINDOWS
	DG_Texture[Stage] = NULL;
#endif
	DG_SetTextureRaw( Stage, NULL );

	//DG_SetTextureStageState( Stage, D3DTSS_ALPHAKILL, D3DTALPHAKILL_ENABLE );
}

/* ゲーム終了時のDirect3Dのステート/内部変数を終了処理する */
void DG_ReleaseD3DState(void)
{
	int	stage ;

	for(stage=0; stage<TEXTURE_STAGE_MAX; stage++)
	{
		/*-- テクスチャ解除しないとメモリリークを起こすらしい ----------*/

		DG_SetTextureRaw(stage, NULL) ;
		/*--------------------------------------------------------------*/
	}

	/*-- RenderingTargetの解除 -----------------------------------------*/

	DG_SetRenderTarget(NULL, NULL) ;
	/*------------------------------------------------------------------*/
}

/* 全般 */
void DG_Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags,
			  D3DCOLOR Color, float Z, DWORD Stencil)
{
#if !__DG_DRAW_IN_EXEBUFFER__
	HRESULT	hr ;
#endif
	/*-- Flag補正 ------------------------------------------------------*/

	Flags &= _DG_render_target_clear_flag_mask ;
	if( !Flags ){ return ; }
	/*------------------------------------------------------------------*/

#if __DG_DRAW_IN_EXEBUFFER__
	DG_Clear_EB(Count, pRects, Flags, Color, Z, Stencil) ;
#else
	hr = IDirect3DDevice8_Clear( g_pd3dDevice, Count, pRects, Flags, Color, Z, Stencil );
	ASSERT( !FAILED(hr) ) ;
#endif
}

void DG_BeginScene(void)
{
	HRESULT	hr ;

	hr = IDirect3DDevice8_BeginScene( g_pd3dDevice );

	ASSERT(!FAILED(hr)) ;
}

void DG_EndScene(void)
{
	HRESULT	hr ;

	hr = IDirect3DDevice8_EndScene( g_pd3dDevice );
	ASSERT(!FAILED(hr)) ;
}

extern	BOOL	DG_DeviceLost ;		// DeviceLost中

HRESULT DG_Present(CONST RECT* pSourceRect, CONST RECT* pDestRect,
		   HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{
	HRESULT	hr ;

	/*-- 表示Flip ------------------------------------------------------*/

	DG_CopyBackBuffer2Frame() ;	// FrameBufferに内容複写(安全対策)
	DG_EndCurrentBackBuffer() ;	// 現在のBackBuffer使用終了処理

	hr = IDirect3DDevice8_Present( g_pd3dDevice,
						pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion) ;

	if( FAILED(hr) )
	{
		switch( hr )
		{
		  case D3DERR_DEVICELOST :	// Device Lost
			if( !DG_ResetD3DDevice() )	// 復旧作業
			{
				DG_DeviceLost = TRUE ;
				return(E_FAIL) ;	// 復旧失敗
			}
			break ;

		  default :
			ASSERT(0) ;
			break ;
		}
	}
	DG_DeviceLost = FALSE ;

	DG_FlipBackBuffer() ;

	/*------------------------------------------------------------------*/

#if FALSE
	/*-- Clear用マスク設定 ---------------------------------------------*/

	DG_SetClearFlagMask(DG_WinApp.clear_flag_mask) ;
	/*------------------------------------------------------------------*/

	/*-- RenderTarget情報直接設定 --------------------------------------*/

	DG_SetRenderTargetWH(DG_WinApp.window_width, DG_WinApp.window_height) ;
	/*------------------------------------------------------------------*/
#endif

	return(S_OK) ;
}

// α関連

/*
 * SCE_GS_SET_ALPHAについて
 *
 *  アルファブレンディングを行った場合の出力カラー Cv は
 *  以下の式で求めることが出来る。
 *
 *  Cv = (A - B) * C >> 7 + D
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  この式における A, B, C, D は、SCE_GS_SET_ALPHA(a, b, c, d, fix) によって
 *  次のように定まる。
 *
 *  フレームバッファのカラーを Cd, アルファ値を Ad,
 *  描画ソースのカラーを       Cs, アルファ値を As,
 *  出力されるカラーを         Cv と置くと、
 *
 *  a == 0 : Cs, 1 : Cd,  2 : 0
 *  b == 0 : Cs, 1 : Cd,  2 : 0
 *  c == 0 : As, 1 : Ad,  2 : fix
 *  d == 0 : Cs, 1 : Cd,  2 : 0
 *
 *  fix  : c == 2 の時に参照されるアルファ値を設定する。
 *  0～255の値を取り、128の時に 1.0 を表す。
 *
 *  ※ 16bit, 24bit カラーの場合、As は TEXAレジスタを参照して定められる。
 *
 * これに対し、DirectXでは
 * SRCBLEND, DSTBLEND の設定と
 * D3DBLENDOPの設定(ADD, SUBTRACT(src-dst), REVSUBSTRACT(dst-src))
 * を設定することで可能になる。
 * SRC に対する fix 値は ピクセルシェーダでなんとかなるが、
 * DST に対する fix 値は どうしようもない。
 *
 *  D3DBLEND_ZERO         : (0, 0, 0, 0) 
 *  D3DBLEND_ONE          : (1, 1, 1, 1)
 *  D3DBLEND_SRCCOLOR     : (Rs, Gs, Bs, As)
 *  D3DBLEND_INVSRCCOLOR  : (1-Rs, 1-Gs, 1-Bs, 1-As)
 *  D3DBLEND_SRCALPHA     : (As, As, As, As)
 *  D3DBLEND_INVSRCALPHA  : (1-As, 1-As, 1-As, 1-As)
 *  D3DBLEND_DESTALPHA    : (Ad, Ad, Ad, Ad)
 *  D3DBLEND_INVDESTALPHA : (1-Ad, 1-Ad, 1-Ad, 1-Ad)
 *  D3DBLEND_DESTCOLOR    : (Rd, Gd, Bd, Ad)
 *  D3DBLEND_INVDESTCOLOR : (1-Rd, 1-Gd, 1-Bd, 1-Ad)
 *  D3DBLEND_SRCALPHASAT  : (f, f, f, 1), f = min(As, 1-Ad)
 *
 *
 *      SCE_GS_SET_ALPHA(2, 0, 0, 1) ... ( 0 - Cs) *  As + Cd
 *                   Cd - a * Cs     = SRCALPHA, ONE, REVSUBSTRACT
 *      SCE_GS_SET_ALPHA(2, 2, 2, 1)      ... ( 0 -  0) * fix + Cd
 *                   Cd              = ZERO, ONE
 *      SCE_GS_SET_ALPHA(0, 1, 2, 1)      ... (Cs - Cd) * fix + Cd
 *                   無理
 *      SCE_GS_SET_ALPHA(1, 2, 2, 2)      ... (Cd -  0) * fix + 0
 *                   Cd              = ZERO, ONE
 *      SCE_GS_SET_ALPHA(1, 0, 2, 2)      ... (Cd - Cs) * fix + 0
 *                   Cd - Cs         = ONE, ONE, BLENDOP_REVSUBTRACT
 *      SCE_GS_SET_ALPHA(0, 1, 2, 2, 128) ... (Cs - Cd) * 128 + 0
 *                   Cs - Cd         = ONE, ONE, BLENDOP_SUBTRACT
 *      SCE_GS_SET_ALPHA(0, 1, 0, 1, fix)  ... (Cs - Cd) * As  + Cd
 *                 a Cs + (1 - a) Cd = SRCALPHA, INVSRCALPHA
 *      SCE_GS_SET_ALPHA(0, 2, 0, 1, fix)  ... (Cs -  0) * As  + Cd
 *                 a Cs + Cd         = SRCALPHA, ONE
 */

/* アルファブレンディング設定値 */
static unsigned int DG_AlphaBlend;
static unsigned int DG_SrcBlend;
static unsigned int DG_DstBlend;
static unsigned int DG_BlendOp;

/* アルファブレンディング値を初期化する */
/* 起動時に一回呼ばれる */
void DG_InitAlphaMode(void)
{
	DG_AlphaBlend = FALSE;
	DG_SrcBlend = D3DBLEND_ONE;
	DG_DstBlend = D3DBLEND_ZERO;
	DG_BlendOp = D3DBLENDOP_ADD;

	DG_SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );		
	DG_SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	DG_SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
	DG_SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
}

/* アルファブレンディング値切り替え(変更チェック付き/鬱陶しい…) */
static inline void SetAlphaBlend(unsigned int mode)
{
	if (DG_AlphaBlend != mode) {
		DG_AlphaBlend = mode;
		DG_SetRenderState( D3DRS_ALPHABLENDENABLE, mode);
	}
}
static inline void SetSrcBlend(unsigned int mode)
{
	if (DG_SrcBlend != mode) {
		DG_SrcBlend = mode;
		DG_SetRenderState( D3DRS_SRCBLEND, mode);
	}
}
static inline void SetDstBlend(unsigned int mode)
{
	if (DG_DstBlend != mode) {
		DG_DstBlend = mode;
		DG_SetRenderState( D3DRS_DESTBLEND, mode);
	}
}
static inline void SetBlendOp(unsigned int mode)
{
	if (DG_BlendOp != mode) {
		DG_BlendOp = mode;
		DG_SetRenderState( D3DRS_BLENDOP, mode);
	}
}

/* 外部公開(不透明オブジェクト描画時にOFFにする) */
void DG_AlphaBlendDisable(void)
{
	SetAlphaBlend(FALSE);
	SetSrcBlend(D3DBLEND_ONE);
	SetDstBlend(D3DBLEND_ZERO);
	SetBlendOp(D3DBLENDOP_ADD);
}

/* 加算半透明にする */
void DG_SetSemitransAdd(void)
{
	SetAlphaBlend(TRUE);
	SetSrcBlend(D3DBLEND_ONE);
	SetDstBlend(D3DBLEND_ONE);
	SetBlendOp(D3DBLENDOP_ADD);
}

/* 通常の半透明にする */
void DG_SetSemitransNormal(void)
{
	SetAlphaBlend(TRUE);
	SetSrcBlend(D3DBLEND_SRCALPHA);
	SetDstBlend(D3DBLEND_INVSRCALPHA);
	SetBlendOp(D3DBLENDOP_ADD);
}

/* 影テクスチャ描画用の半 透明モード */
void DG_SetSemitransShadow(void)
{
#if 0
	SetAlphaBlend(TRUE);
	SetSrcBlend(D3DBLEND_INVSRCALPHA);
	SetDstBlend(D3DBLEND_SRCALPHA);
	SetBlendOp(D3DBLENDOP_ADD);
#else
	SetAlphaBlend(TRUE);
	SetSrcBlend(D3DBLEND_SRCALPHA);
	SetDstBlend(D3DBLEND_INVSRCALPHA);
	SetBlendOp(D3DBLENDOP_ADD);
#endif
}

/* 減算半透明にする */
void DG_SetSemitransSub(void)
{
	SetAlphaBlend(TRUE);
	SetSrcBlend(D3DBLEND_ONE);
	SetDstBlend(D3DBLEND_ONE);
	SetBlendOp(D3DBLENDOP_REVSUBTRACT);
}

/* アルファブレンディングモードを設定する。 */
/*
	SCE_GS_SET_ALPHAで正直に分岐する。
	※このままだと死ぬ程重いのでは…
	※Windows版のHALでは、D3DBLENDOP サポートありません。
	（ＰＳ２形式のビット並びを直で指定しているので注意！）
*/
#if 0
void DG_SetAlphaMode(u_long type)
{
	unsigned int	alpha = 0x80000000 ;
	int				alpha_fix_mode = 0 ;
	int				param ;
#ifdef WIRE_MODE
	SetAlphaBlend(FALSE);
	return;
#endif
	/* まず0かどうかでαブレンディングのON/OFFを決める */
	if (type == 0) {
		SetAlphaBlend(FALSE);
		return;
	}
	SetAlphaBlend(TRUE);

	/* 固定アルファチェック */
	switch ( ( type >> 4 ) & 3 ){
	  case 2:
		alpha = ( type >> 32 ) & 0xff ;
		break ;
	  default:
		alpha = 128 ;
		break ;
	}

	param = (int)(type & SCE_GS_SET_ALPHA(3,3,3,3,0));
	switch (param) {
	  case SCE_GS_SET_ALPHA(2,2,0,1,0): /* DST */
	  case SCE_GS_SET_ALPHA(2,2,1,1,0): /* DST */
	  case SCE_GS_SET_ALPHA(2,2,2,1,0): /* DST */
		SetSrcBlend(D3DBLEND_ZERO);
		SetDstBlend(D3DBLEND_ONE);
		SetBlendOp(D3DBLENDOP_ADD);
		break ;
	  case SCE_GS_SET_ALPHA(2,2,0,0,0): /* SRC */
	  case SCE_GS_SET_ALPHA(2,2,1,0,0): /* SRC */
	  case SCE_GS_SET_ALPHA(2,2,2,0,0): /* SRC */
		SetSrcBlend(D3DBLEND_ONE);
		SetDstBlend(D3DBLEND_ZERO);
		SetBlendOp(D3DBLENDOP_ADD);
		break ;
	  case SCE_GS_SET_ALPHA(0,2,2,2,0): /* SRC * fix */
		SetSrcBlend(D3DBLEND_SRCALPHA);
		SetDstBlend(D3DBLEND_ZERO);
		SetBlendOp(D3DBLENDOP_ADD);
		alpha_fix_mode = 1 ;
	  case SCE_GS_SET_ALPHA(1,2,2,2,0): /* DST * fix */
		SetSrcBlend(D3DBLEND_ZERO);
		SetDstBlend(D3DBLEND_SRCALPHA);
		SetBlendOp(D3DBLENDOP_ADD);
		alpha_fix_mode = 1 ;
		break;
	  case SCE_GS_SET_ALPHA(1,0,2,2,0): /* ( DST - SRC ) * fix */
		SetSrcBlend(D3DBLEND_SRCALPHA);
		SetDstBlend(D3DBLEND_SRCALPHA);
		SetBlendOp(D3DBLENDOP_REVSUBTRACT);
		alpha_fix_mode = 1 ;
		break;
	  case SCE_GS_SET_ALPHA(0,1,2,2,0): /* ( SRC - DST ) * fix */
		SetSrcBlend(D3DBLEND_SRCALPHA);
		SetDstBlend(D3DBLEND_SRCALPHA);
		SetBlendOp(D3DBLENDOP_SUBTRACT);
		alpha_fix_mode = 1 ;
		break;
	  case SCE_GS_SET_ALPHA(0,1,0,2,0): /* ( SRC - DST ) * alpha */
		SetSrcBlend(D3DBLEND_SRCALPHA);
		SetDstBlend(D3DBLEND_SRCALPHA);
		SetBlendOp(D3DBLENDOP_SUBTRACT);
		break;
	  case SCE_GS_SET_ALPHA(0,2,0,2,0): /* SRC * alpha */
		SetSrcBlend(D3DBLEND_SRCALPHA);
		SetDstBlend(D3DBLEND_SRCALPHA);
		SetBlendOp(D3DBLENDOP_SUBTRACT);
		break;
	  case SCE_GS_SET_ALPHA(0,1,0,1,0): /* 通常の半透明 */
		SetSrcBlend(D3DBLEND_SRCALPHA);
		SetDstBlend(D3DBLEND_INVSRCALPHA);
		SetBlendOp(D3DBLENDOP_ADD);
		break;
	  case SCE_GS_SET_ALPHA(0,2,0,1,0): /* 加算 SRC * alpha + DST */
		SetSrcBlend(D3DBLEND_SRCALPHA);
		SetDstBlend(D3DBLEND_ONE);
		SetBlendOp(D3DBLENDOP_ADD);
		break;
	  case SCE_GS_SET_ALPHA(2,0,0,1,0): /* 減算 DST - alpha * SRC */
		SetSrcBlend(D3DBLEND_SRCALPHA);
		SetDstBlend(D3DBLEND_ONE);
		SetBlendOp(D3DBLENDOP_REVSUBTRACT);
		break;
	  case SCE_GS_SET_ALPHA(0,1,2,1,0): /* SRC * fix + DST * ( 1 - fix ) */
		SetSrcBlend(D3DBLEND_SRCALPHA);
		SetDstBlend(D3DBLEND_INVSRCALPHA);
		SetBlendOp(D3DBLENDOP_ADD);
		alpha_fix_mode = 1 ;
		break;
	  case SCE_GS_SET_ALPHA(0,2,2,1,0): /* 加算 SRC * fix + DST */
		SetSrcBlend(D3DBLEND_SRCALPHA);
		SetDstBlend(D3DBLEND_ONE);
		SetBlendOp(D3DBLENDOP_ADD);
		alpha_fix_mode = 1 ;
		break;
	  case SCE_GS_SET_ALPHA(2,0,2,1,0): /* 減算 DST - fix * SRC */
		SetSrcBlend(D3DBLEND_SRCALPHA);
		SetDstBlend(D3DBLEND_ONE);
		SetBlendOp(D3DBLENDOP_REVSUBTRACT);
		alpha_fix_mode = 1 ;
		break;
	default:
		/* サポートしていないモードはここに来る */
		SetSrcBlend(D3DBLEND_ONE);
		SetDstBlend(D3DBLEND_ZERO);
		SetBlendOp(D3DBLENDOP_ADD);
		printf("SCE_GS_SET_ALPHA : wrong type (%08lx)\n", type);
		break;
	}

	if ( alpha_fix_mode ){
		alpha <<= 1 ;
		if ( alpha > 255 ) alpha = 255 ;
		DG_SetRenderState( D3DRS_TEXTUREFACTOR, alpha << 24 );
		DG_SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR );
		DG_SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );	/* ここで変更するとやばいか？ */
	} else {
		DG_SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		DG_SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	}

}
#endif

/* レンダリングステート設定 */
#if !__SET_RENDER_STATE_INLINE__	/* libdgw.h内でインライン定義 */
void DG_SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	if( DG_RenderStateValue[State] != Value )
	{
#if __DG_DRAW_IN_EXEBUFFER__
		DG_SetRenderState_EB(State, Value) ;
#else
		HRESULT	hr ;

		hr = IDirect3DDevice8_SetRenderState( g_pd3dDevice, State, Value );
		ASSERT( !FAILED(hr) ) ;
#endif
		DG_RenderStateValue[State] = Value ;
	}
}

DWORD DG_GetRenderState(D3DRENDERSTATETYPE State)
{
	return(DG_RenderStateValue[State]) ;
}

void DG_NopRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	DG_RenderStateValue[State] = State ;	// 状態だけ設定
}

#endif

/* 変換行列設定 */
void DG_SetTransform( D3DTRANSFORMSTATETYPE State, CONST FMATRIX *pMatrix)
{
#if	__STATECHK_SETTRANSFORM__
	FMATRIX	*cur_mtx ;

	if( State >= D3DTS_WORLDMATRIX(0) )
	{
		ASSERT( State < (D3DTS_WORLDMATRIX(0) + DG_SETTRANSFORM_WORLD_MAX) ) ;
		cur_mtx = &DG_SetTransform_World[State - D3DTS_WORLDMATRIX(0)] ;
	}
	else if( State >= D3DTS_TEXTURE0 )
	{
		ASSERT( State < (D3DTS_TEXTURE0 + DG_SETTRANSFORM_TEX_MAX) ) ;
		cur_mtx = &DG_SetTransform_Tex[State - D3DTS_TEXTURE0] ;
	}
	else
	{
		ASSERT( State < DG_SETTRANSFORM_VIEW_MAX ) ;
		cur_mtx = &DG_SetTransform_View[State] ;
	}

	if(  (pMatrix->m[0][0] == cur_mtx->m[0][0])
	  && (pMatrix->m[0][1] == cur_mtx->m[0][1])
	  && (pMatrix->m[0][2] == cur_mtx->m[0][2])
	  && (pMatrix->m[0][3] == cur_mtx->m[0][3])
	  && (pMatrix->m[1][0] == cur_mtx->m[1][0])
	  && (pMatrix->m[1][1] == cur_mtx->m[1][1])
	  && (pMatrix->m[1][2] == cur_mtx->m[1][2])
	  && (pMatrix->m[1][3] == cur_mtx->m[1][3])
	  && (pMatrix->m[2][0] == cur_mtx->m[2][0])
	  && (pMatrix->m[2][1] == cur_mtx->m[2][1])
	  && (pMatrix->m[2][2] == cur_mtx->m[2][2])
	  && (pMatrix->m[2][3] == cur_mtx->m[2][3])
	  && (pMatrix->m[3][0] == cur_mtx->m[3][0])
	  && (pMatrix->m[3][1] == cur_mtx->m[3][1])
	  && (pMatrix->m[3][2] == cur_mtx->m[3][2])
	  && (pMatrix->m[3][3] == cur_mtx->m[3][3]) )
	{
		return ;	// 前回と同じ
	}
#endif

#if __DG_DRAW_IN_EXEBUFFER__
	DG_SetTransform_EB(State, (void *)pMatrix) ;
#else
	HRESULT	hr ;

	hr = IDirect3DDevice8_SetTransform( g_pd3dDevice, State, (CONST D3DMATRIX *)pMatrix );
	ASSERT( !FAILED(hr) ) ;
#endif

#if	__STATECHK_SETTRANSFORM__
	memcpy(cur_mtx, pMatrix, sizeof(FMATRIX)) ;
#endif
}

/* 変換行列設定 */
void DG_SetTransformTexCoord( D3DTRANSFORMSTATETYPE State, CONST FMATRIX *pMatrix)
{
#if	__STATECHK_SETTRANSFORM__
	FMATRIX	*cur_mtx ;

	cur_mtx = &DG_SetTransform_Tex[State - D3DTS_TEXTURE0] ;

	if(  (pMatrix->m[0][0] == cur_mtx->m[0][0])
	  && (pMatrix->m[1][1] == cur_mtx->m[1][1])
	  && (pMatrix->m[2][0] == cur_mtx->m[2][0])
	  && (pMatrix->m[2][1] == cur_mtx->m[2][1]) )	// 使用する奴だけ比較
	{
		return ;	// 前回と同じ
	}
#endif

#if __DG_DRAW_IN_EXEBUFFER__
	DG_SetTransform_EB(State, (void *)pMatrix) ;
#else
	HRESULT	hr ;

	hr = IDirect3DDevice8_SetTransform( g_pd3dDevice, State, (CONST D3DMATRIX *)pMatrix );
	ASSERT( !FAILED(hr) ) ;
#endif

#if	__STATECHK_SETTRANSFORM__
	memcpy(cur_mtx, pMatrix, sizeof(FMATRIX)) ;
#endif
}

/* 変換行列転地設定 */
void DG_SetTransposeTransform( D3DTRANSFORMSTATETYPE State, CONST FMATRIX *pMatrix)
{
	FMATRIX	mtx ;

	D3DXMatrixTranspose((void *)&mtx, (void *)pMatrix);
	DG_SetTransform(State, &mtx) ;
}

/* 頂点バッファ作成 */
void DG_CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool,
						   IDirect3DVertexBuffer8** ppVertexBuffer)
{
	HRESULT hr;

	if( !Length )
	{
		*ppVertexBuffer = NULL ;
		return ;
	}

	hr = IDirect3DDevice8_CreateVertexBuffer( g_pd3dDevice, Length, Usage, FVF, Pool,
											  ppVertexBuffer );

	if (FAILED(hr)) {
		/*-- エラーメッセージ ----------------------------------------------*/

		X2W_ErrorPutsHResult(X2W_MB_ERROR, hr) ;
		X2W_ErrorAbort() ;
		/*------------------------------------------------------------------*/

#ifdef DEBUG_MODE
		dbgErrMessPuts("CreateVertexBuffer", hr) ;
		printf("CreateVertexBuffer failed.:%d\n", hr);
		ASSERT(0);
#endif
	}

#if	__RESOURCE_MANAGER__
	/*-- リソース管理 ------------------------------------------------------*/

	if( Pool == D3DPOOL_DEFAULT )
	{
		RESOURCE_ELEM_PARAM_VBUFFER	param ;

		param.Length = Length;
		param.Usage  = Usage ;
		param.FVF    = FVF ;

		DG_QueueElemD3DResourceTbl(&DG_ResourceMan.defpool_tbl,
							RESOURCE_ELEM_TYPE_VBUFFER,
							ppVertexBuffer,
							(void *)&param, sizeof(param)) ;
	}
	/*----------------------------------------------------------------------*/
#endif
}

/* 頂点バッファ解放処理 */
void DG_ReleaseD3DVertexBuffer(IDirect3DVertexBuffer8* pVertexBuffer)
{
	HRESULT hr;
	int		i ;
	IDirect3DVertexBuffer8 **src_data ;

	ASSERT( DG_ExeBufferSync(TRUE) ) ;

#if __STATECHK_STREAM_SOUCE__
	src_data = DG_StreamSourceData ;
	for(i=0; i<8; i++, src_data++)
	{
		if( pVertexBuffer == *src_data )
		{
			*src_data = (void *)0xffffffff ;
		}
	}
#endif

	hr = IDirect3DVertexBuffer8_Release(pVertexBuffer) ;

#if	__RESOURCE_MANAGER__
	/*-- リソース管理 ------------------------------------------------------*/

	DG_DequeueElemD3DResourceTbl(&DG_ResourceMan.defpool_tbl, pVertexBuffer) ;
	/*----------------------------------------------------------------------*/
#endif
}


/* インデックスバッファ作成 */
void DG_CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format,
						  D3DPOOL Pool, IDirect3DIndexBuffer8** ppIndexBuffer)
{
	HRESULT hr;

	if( !Length )
	{
		*ppIndexBuffer = NULL ;
		return ;
	}

	hr = IDirect3DDevice8_CreateIndexBuffer( g_pd3dDevice, Length, Usage, Format, Pool, ppIndexBuffer );
	if (FAILED(hr)) {
		/*-- エラーメッセージ ----------------------------------------------*/

		X2W_ErrorPutsHResult(X2W_MB_ERROR, hr) ;
		X2W_ErrorAbort() ;
		/*------------------------------------------------------------------*/
#ifdef DEBUG_MODE
		printf("CreateIndexBuffer failed.\n");
		ASSERT(0);
#endif
	}

#if	__RESOURCE_MANAGER__
	/*-- リソース管理 ------------------------------------------------------*/

	if( Pool == D3DPOOL_DEFAULT )
	{
		RESOURCE_ELEM_PARAM_IBUFFER	param ;

		param.Length = Length ;
		param.Usage  = Usage  ;
		param.Format = Format ;

		DG_QueueElemD3DResourceTbl(&DG_ResourceMan.defpool_tbl,
							RESOURCE_ELEM_TYPE_IBUFFER,
							ppIndexBuffer,
							(void *)&param, sizeof(param)) ;
	}
	/*----------------------------------------------------------------------*/
#endif
}

/* インデックスバッファ解放 */
void DG_ReleaseD3DIndexBuffer(IDirect3DIndexBuffer8* pIndexBuffer)
{
	HRESULT hr;

	ASSERT( DG_ExeBufferSync(TRUE) ) ;

#if __STATECHK_INDEICES__
	if( pIndexBuffer == DG_IndeciesData ){ DG_IndeciesData = (void *)0xffffffff ; }
#endif

	hr = IDirect3DIndexBuffer8_Release(pIndexBuffer) ;

#if	__RESOURCE_MANAGER__
	/*-- リソース管理 ------------------------------------------------------*/

	DG_DequeueElemD3DResourceTbl(&DG_ResourceMan.defpool_tbl, pIndexBuffer) ;
	/*----------------------------------------------------------------------*/
#endif
}

void DG_SetRenderTarget2(IDirect3DSurface8* pRenderTarget,
						IDirect3DSurface8* pNewZStencil,
						DWORD ClearFlagMask)
{
#if !__DG_DRAW_IN_EXEBUFFER__
	HRESULT hr;
#endif

#if __DG_DRAW_IN_EXEBUFFER__
	DG_SetRenderTarget_EB(pRenderTarget, pNewZStencil) ;
#else
	hr = IDirect3DDevice8_SetRenderTarget( g_pd3dDevice, pRenderTarget, pNewZStencil );
#ifdef DEBUG_MODE
	if (FAILED(hr)) {
		printf("SetRenderTarget failed\n");
		dbgErrMessPuts("SetRenderTarget", hr) ;
		ASSERT(0);
	}
#endif
#endif

	/*-- Clear用マスク設定 ---------------------------------------------*/

	_DG_render_target_clear_flag_mask = ClearFlagMask ;
	/*------------------------------------------------------------------*/

	/*-- Surface記憶 ---------------------------------------------------*/

#if __DG_DRAW_IN_EXEBUFFER__
	_DG_cur_render_target_surface = pRenderTarget ;
	_DG_cur_depth_surface = pNewZStencil ;
#endif
	/*------------------------------------------------------------------*/
}

void DG_SetRenderTarget(IDirect3DSurface8* pRenderTarget,
						IDirect3DSurface8* pNewZStencil)
{
	DWORD			mask ;
	D3DSURFACE_DESC	desc ;
	HRESULT			hr ;

	/*-- Clear用マスク作成 ---------------------------------------------*/

	mask = 0 ;
	if( pRenderTarget )
	{
		mask |= D3DCLEAR_TARGET ;

		hr = IDirect3DSurface8_GetDesc(pRenderTarget, &desc) ;
		ASSERT( !FAILED(hr) ) ;
		DG_SetRenderTargetWH(desc.Width, desc.Height) ;
	}
	else
	{
		DG_SetRenderTargetWH(0, 0) ;
	}

	if( pNewZStencil )
	{
		mask |= D3DCLEAR_ZBUFFER ;

		IDirect3DSurface8_GetDesc(pNewZStencil, &desc) ;
		switch( desc.Format )
		{
		  case D3DFMT_D15S1 :
		  case D3DFMT_D24S8 :
		  case D3DFMT_D24X8 :
		  case D3DFMT_D24X4S4 :
			mask |= D3DCLEAR_STENCIL ;
			break ;
		}
	}
	/*------------------------------------------------------------------*/

	DG_SetRenderTarget2(pRenderTarget, pNewZStencil, mask) ;
}

void DG_SetRenderTargetWH(DWORD width, DWORD height)
{
	DG_render_target_width  = width ;
	DG_render_target_height = height ;
	DG_render_target_view_nrm_w = (float)width ;
	DG_render_target_view_nrm_h = (float)height ;
}

void DG_GetRenderTargetWH(DWORD *width, DWORD *height)
{
	*width  = DG_render_target_width ;
	*height = DG_render_target_height ;
}

void	DG_SetClearFlagMask(DWORD mask)
{
	_DG_render_target_clear_flag_mask = mask ;
}

DWORD	DG_GetClearFlagMask(void)
{
	return(_DG_render_target_clear_flag_mask) ;
}

void DG_GetRenderTarget(IDirect3DSurface8** ppRenderTarget)
{
#if !__DG_DRAW_IN_EXEBUFFER__
	HRESULT hr;
	hr = IDirect3DDevice8_GetRenderTarget( g_pd3dDevice, ppRenderTarget );
#ifdef DEBUG_MODE
	if (FAILED(hr)) {
		printf("GetRenderTarget failed\n");
		ASSERT(0);
	}
#endif
#else
	*ppRenderTarget = _DG_cur_render_target_surface ;
#endif
}

void DG_GetDepthStencilSurface(IDirect3DSurface8 **ppZStencilSurface)
{
#if !__DG_DRAW_IN_EXEBUFFER__
	HRESULT hr;
	hr = IDirect3DDevice8_GetDepthStencilSurface( g_pd3dDevice, ppZStencilSurface );
#ifdef DEBUG_MODE
	if (FAILED(hr)) {
		printf("GetDepthStencilSurface failed\n");

		ASSERT(0);
	}
#endif
#else
	*ppZStencilSurface = _DG_cur_depth_surface ;
#endif
}

#if __DG_DRAW_IN_EXEBUFFER__
void DG_SetCurrentRenderTarget(void)
{
	HRESULT hr;

	/* 現在のRenderTargetを記録 */
	hr = IDirect3DDevice8_GetRenderTarget( g_pd3dDevice,
								&_DG_cur_render_target_surface );
	if (FAILED(hr)) {
		printf("GetRenderTarget failed\n");
		ASSERT(0);
	}

	hr = IDirect3DDevice8_GetDepthStencilSurface( g_pd3dDevice,
								&_DG_cur_depth_surface );
	if (FAILED(hr)) {
		printf("GetDepthStencilSurface failed\n");

		ASSERT(0);
	}
}
#endif


/* サーフェイス作成 */
HRESULT DG_CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format,
						D3DMULTISAMPLE_TYPE MultiSample,IDirect3DSurface8** ppSurface)
{
	HRESULT hr;

	hr = IDirect3DDevice8_CreateDepthStencilSurface(g_pd3dDevice,
							Width, Height, Format, MultiSample, ppSurface) ;

	if (FAILED(hr)) {
		/*-- エラーメッセージ ----------------------------------------------*/

		X2W_ErrorPutsHResult(X2W_MB_ERROR, hr) ;
		X2W_ErrorAbort() ;
		/*------------------------------------------------------------------*/
#ifdef DEBUG_MODE
		dbgErrMessPuts("", hr) ;
		printf("CreateDepthStencilSurface (%d, %d) failed\n", Width, Height);
		ASSERT(0);
#endif
		return(hr) ;
	}

#if	__RESOURCE_MANAGER__
	/*-- リソース管理 ------------------------------------------------------*/

	{
		// D3DPOOL_DEFAULTと決まっている

		RESOURCE_ELEM_PARAM_DEPTHSTENCILSURFACE	param ;

		param.Width  = Width ;
		param.Height = Height ;
		param.Format = Format ;
		param.MultiSample = MultiSample ;

		DG_QueueElemD3DResourceTbl(&DG_ResourceMan.defpool_tbl,
							RESOURCE_ELEM_TYPE_DEPTHSTENCILSURFACE,
							ppSurface,
							(void *)&param, sizeof(param)) ;
	}
	/*----------------------------------------------------------------------*/
#endif

	return(hr) ;
}

/* Surface破棄 */
void DG_DestroySurface(IDirect3DSurface8 *pSurface)
{
	IDirect3DSurface8_Release(pSurface) ;

#if	__RESOURCE_MANAGER__
	/*-- リソース管理 ------------------------------------------------------*/

	DG_DequeueElemD3DResourceTbl(&DG_ResourceMan.defpool_tbl, pSurface) ;
	/*----------------------------------------------------------------------*/
#endif
}

/* テクスチャ関連 */
void DG_CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage,
					  D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture8** ppTexture)
{
	HRESULT hr;

	hr = IDirect3DDevice8_CreateTexture( g_pd3dDevice, Width, Height,
								Levels, Usage, Format, Pool, ppTexture);

	if (FAILED(hr)) {
		/*-- エラーメッセージ ----------------------------------------------*/

		X2W_ErrorPutsHResult(X2W_MB_ERROR, hr) ;
		X2W_ErrorAbort() ;
		/*------------------------------------------------------------------*/
#ifdef DEBUG_MODE
		dbgErrMessPuts("", hr) ;
		printf("CreateTexture (%d, %d) failed\n", Width, Height);
		ASSERT(0);
#endif
	}

#if	__RESOURCE_MANAGER__
	/*-- リソース管理 ------------------------------------------------------*/

	if( Pool == D3DPOOL_DEFAULT )
	{
		// D3DPOOL_DEFAULTと決まっている

		RESOURCE_ELEM_PARAM_TEXTURE	param ;

		param.Width  = Width ;
		param.Height = Height ;
		param.Levels = Levels ;
		param.Usage  = Usage ;
		param.Format = Format ;
		DG_QueueElemD3DResourceTbl(&DG_ResourceMan.defpool_tbl,
							RESOURCE_ELEM_TYPE_TEXTURE,
							ppTexture,
							(void *)&param, sizeof(param)) ;
	}
	/*----------------------------------------------------------------------*/
#endif
}

void DG_CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage,
							D3DFORMAT Format, D3DPOOL Pool,
							IDirect3DVolumeTexture8** ppVolumeTexture)
{
	HRESULT hr;
	hr = IDirect3DDevice8_CreateVolumeTexture( g_pd3dDevice, Width, Height, Depth, Levels, Usage,
											  Format, Pool, ppVolumeTexture);
	if (FAILED(hr)) {
		/*-- エラーメッセージ ----------------------------------------------*/

		X2W_ErrorPutsHResult(X2W_MB_ERROR, hr) ;
		X2W_ErrorAbort() ;
		/*------------------------------------------------------------------*/
#ifdef DEBUG_MODE
		printf("CreateVolumeTexture (%d, %d, %d) failed\n", Width, Height, Depth);
		ASSERT(0);
#endif
	}
}

static void inline DG_SetTextureRaw( int stage, LPDIRECT3DTEXTURE8 pTexture )
{
#if !__DG_DRAW_IN_EXEBUFFER__
	HRESULT	hr ;
#endif
	if( DG_Texture[stage] != (void *)pTexture )	// 比較テスト
	{
#if __DG_DRAW_IN_EXEBUFFER__
		DG_SetTexture_EB(stage, pTexture) ;
#else
		hr = IDirect3DDevice8_SetTexture( g_pd3dDevice, stage, pTexture );
		ASSERT( !FAILED(hr) ) ;
#endif
		DG_Texture[stage] = (void *)pTexture ;
	}
}

void DG_SetTexture( int stage, DG_TEX_TRANS *tex_trans )
{
#ifdef WIRE_MODE
	DG_SetTextureRaw( Stage, NULL );
	return ;
#endif

	/* PixelShaderが使えない時はstage0のみ */
	if( (stage > 0) && !DG_CheckPixelShaderUseable() ){ return ; }

	if ( tex_trans == NULL ){
		DG_SetTextureRaw( stage, NULL );
		if ( stage == 0 ){
			DG_SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_MODULATE );
		} else {
			DG_SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_DISABLE );
		}
	} else {
		DG_SetTextureRaw( stage, tex_trans->ptex );

		DG_SetTextureStageState( stage, D3DTSS_COLOROP,   D3DTOP_MODULATE2X);
		if ( stage == 0 ){
			DG_SetAlphaMode( tex_trans->alpha.data );
		}
#ifdef UVADJUST
		if ( stage < 3 ){
			/* CV_TEX?_SCALE,CV_TEX?_OFFSETそれぞれにパラメータを転送 */
			DG_SetVertexShaderConstant( CV_TEX0_SCALE + stage * 2, &tex_trans->vec1, 2);
		}
#endif
	}
}

void DG_SetTextureNVS( int stage, DG_TEX_TRANS *tex_trans )
{
#ifdef WIRE_MODE
	DG_SetTextureRaw( Stage, NULL );
	return ;
#endif

	/* PixelShaderが使えない時はstage0のみ(仮) */
	if( (stage > 0) && !DG_CheckPixelShaderUseable() ){ return ; }

	if ( tex_trans == NULL ){
		DG_SetTextureRaw( stage, NULL );
		if ( stage == 0 ){
			DG_SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_MODULATE );
		} else {
			DG_SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_DISABLE );
		}
	} else {
		DG_SetTextureRaw( stage, tex_trans->ptex );

		DG_SetTextureStageState( stage, D3DTSS_COLOROP,   D3DTOP_MODULATE2X);
		if ( stage == 0 ){
			DG_SetAlphaMode( tex_trans->alpha.data );
		}
#ifdef UVADJUST
		if ( stage < 3 ){
			/* CV_TEX?_SCALE,CV_TEX?_OFFSETそれぞれにパラメータを転送 */
			static FMATRIX	mtx =
			{
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			} ;

			mtx.m[0][0] = tex_trans->vec1.x ;
			mtx.m[2][0] = tex_trans->vec2.x ;
			mtx.m[1][1] = tex_trans->vec1.y ;
			mtx.m[2][1] = tex_trans->vec2.y ;
			DG_SetTransformTexCoord( D3DTS_TEXTURE0 + stage, (void *)&mtx) ;
		}
#endif
	}
}

void DG_SetTextureDirect( int stage, LPDIRECT3DTEXTURE8 pTexture )
{
#ifdef WIRE_MODE
	DG_SetTextureRaw( stage, NULL );
	return ;
#endif

	/* PixelShaderが使えない時はstage0のみ(仮) */
	if( (stage > 0) && !DG_CheckPixelShaderUseable() ){ return ; }

	DG_SetTextureRaw( stage, pTexture );
	if ( pTexture == NULL ){
		if ( stage == 0 ){
			DG_SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_MODULATE );
		} else {
			DG_SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_DISABLE );
		}
	} else {
		DG_SetTextureStageState( stage, D3DTSS_COLOROP,   D3DTOP_MODULATE2X);
	}
}

void DG_ReleaseD3DTexture( LPDIRECT3DTEXTURE8 pTexture )
{
	HRESULT	hr ;
	int		i ;
	void	**tex ;

	tex = DG_Texture ;
	for(i=0; i<TEXTURE_STAGE_MAX; i++, tex++)
	{
		if( *tex == pTexture ){ DG_SetTextureRaw(i, NULL) ;	}	// 指定解除
	}

	hr = IDirect3DTexture8_Release(pTexture) ;	// 解放
	ASSERT( !FAILED(hr) ) ;

#if	__RESOURCE_MANAGER__
	/*-- リソース管理 ------------------------------------------------------*/

	DG_DequeueElemD3DResourceTbl(&DG_ResourceMan.defpool_tbl, pTexture) ;
	/*----------------------------------------------------------------------*/
#endif
}


void DG_SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
#if !__DG_DRAW_IN_EXEBUFFER__
	HRESULT	hr ;
#endif

	if( Stage >= DG_WinApp.d3d_cap.MaxTextureBlendStages )	// 実装機能判定
	{
		ASSERT(0) ;
		return ;
	}

#if __STATECHK_SETTEXTURESTAGESTATE__
	if( DG_TextureStageStateValue[Stage][Type] == Value ){ return ; }	// 同設定
#endif

#if __DG_DRAW_IN_EXEBUFFER__
	DG_SetTextureStageState_EB(Stage, Type, Value) ;
#else
	hr = IDirect3DDevice8_SetTextureStageState( g_pd3dDevice, Stage, Type, Value );
	ASSERT( !FAILED(hr) ) ;
#endif

#if __STATECHK_SETTEXTURESTAGESTATE__
	DG_TextureStageStateValue[Stage][Type] = Value ;
#endif
}

/* DrawPrimitive/描画関連 */
#if	!__DRAW_PRIMITIVE_INLINE__	
void DG_DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	/*-- Primitive数チェック ---------------------------------------------------*/

	if( PrimitiveCount > DG_WinApp.d3d_cap.MaxPrimitiveCount )
	{
#ifdef DEBUG_MODE
		printf("[ Fatal Warning ] PrimitiveCount Max Over (%d/%d)\n",
						PrimitiveCount,
						DG_WinApp.d3d_cap.MaxPrimitiveCount) ;
		ASSERT(0) ;
#endif
		PrimitiveCount = DG_WinApp.d3d_cap.MaxPrimitiveCount ;
	}
	/*--------------------------------------------------------------------------*/


	if (PrimitiveCount != 0) {
#if __DG_DRAW_IN_EXEBUFFER__
		DG_DrawPrimitive_EB(PrimitiveType, StartVertex, PrimitiveCount) ;
#else
		HRESULT	hr ;
		hr = IDirect3DDevice8_DrawPrimitive( g_pd3dDevice, PrimitiveType,
										StartVertex, PrimitiveCount );
		ASSERT( !FAILED(hr) ) ;
#endif
	}
}
#endif

#if	!__DRAW_PRIMITIVE_INLINE__	
void DG_DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount,
						CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	HRESULT	hr ;

	/*-- Primitive数チェック ---------------------------------------------------*/

	if( PrimitiveCount > DG_WinApp.d3d_cap.MaxPrimitiveCount )
	{
#ifdef DEBUG_MODE
		printf("[ Fatal Warning ] PrimitiveCount Max Over (%d/%d)\n",
						PrimitiveCount,
						DG_WinApp.d3d_cap.MaxPrimitiveCount) ;
		ASSERT(0) ;
#endif
		PrimitiveCount = DG_WinApp.d3d_cap.MaxPrimitiveCount ;
	}
	/*--------------------------------------------------------------------------*/

#if __DG_DRAW_IN_EXEBUFFER__
	ASSERT(0) ;
#else
	hr = IDirect3DDevice8_DrawPrimitiveUP( g_pd3dDevice, PrimitiveType, PrimitiveCount,
									 pVertexStreamZeroData, VertexStreamZeroStride );
	ASSERT( !FAILED(hr) ) ;
#endif
}
#endif

#if	!__DRAW_PRIMITIVE_INLINE__	
void DG_DrawIndexedPrimitive(D3DPRIMITIVETYPE Type, UINT MinIndex, UINT NumVertices,
							 UINT StartIndex, UINT PrimitiveCount)
{
	HRESULT	hr ;

	if( !DG_CheckIndexPrimitiveUseable() ){ ASSERT(0) ; return ; }

	/*-- Primitive数チェック ---------------------------------------------------*/

	if( PrimitiveCount > DG_WinApp.d3d_cap.MaxPrimitiveCount )
	{
#ifdef DEBUG_MODE
		printf("[ Fatal Warning ] PrimitiveCount Max Over (%d/%d)\n",
						PrimitiveCount,
						DG_WinApp.d3d_cap.MaxPrimitiveCount) ;
		ASSERT(0) ;
#endif
		PrimitiveCount = DG_WinApp.d3d_cap.MaxPrimitiveCount ;
	}
	/*--------------------------------------------------------------------------*/

	/*-- 描画 ------------------------------------------------------------------*/

#if __DG_DRAW_IN_EXEBUFFER__
	DG_DrawIndexedPrimitive_EB(Type, MinIndex, NumVertices, StartIndex, PrimitiveCount) ;
{#else
	hr = IDirect3DDevice8_DrawIndexedPrimitive( g_pd3dDevice, Type, MinIndex, NumVertices,
										  StartIndex, PrimitiveCount);
	ASSERT( !FAILED(hr) ) ;
#endif
	/*--------------------------------------------------------------------------*/
}
#endif

#if	!__DRAW_PRIMITIVE_INLINE__
void DG_DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex,
							   UINT NumVertices, UINT PrimitiveCount,
							   CONST void* pIndexData, D3DFORMAT IndexDataFormat,
							   CONST void* pVertexStreamZeroData,
							   UINT VertexStreamZeroStride)
{
	HRESULT	hr ;

	if( !DG_CheckIndexPrimitiveUseable() ){ ASSERT(0) ; return ; }

	/*-- Primitive数チェック ---------------------------------------------------*/

	if( PrimitiveCount > DG_WinApp.d3d_cap.MaxPrimitiveCount )
	{
#ifdef DEBUG_MODE
		printf("[ Fatal Warning ] PrimitiveCount Max Over (%d/%d)\n",
						PrimitiveCount,
						DG_WinApp.d3d_cap.MaxPrimitiveCount) ;
		ASSERT(0) ;
#endif
		PrimitiveCount = DG_WinApp.d3d_cap.MaxPrimitiveCount ;
	}
	/*--------------------------------------------------------------------------*/

	/*-- 描画 ------------------------------------------------------------------*/

#if __DG_DRAW_IN_EXEBUFFER__
	ASSERT(0) ;
#else
	hr = IDirect3DDevice8_DrawIndexedPrimitiveUP( g_pd3dDevice, 
			PrimitiveType, MinIndex, NumVertices, PrimitiveCount,
			pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride );
	ASSERT( !FAILED(hr) ) ;
#endif
	/*--------------------------------------------------------------------------*/
}
#endif

void DG_SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer8* pStreamData, UINT Stride)
{
#if !__DG_DRAW_IN_EXEBUFFER__
	HRESULT	hr ;
#endif

#if __STATECHK_STREAM_SOUCE__
	if(  (DG_StreamSourceData[StreamNumber]   == pStreamData)
	  && (DG_StreamSourceStride[StreamNumber] == Stride) )
	{
		return ;
	}
#endif

#if __DG_DRAW_IN_EXEBUFFER__
	DG_SetStreamSource_EB(StreamNumber, pStreamData, Stride) ;
#else
	hr = IDirect3DDevice8_SetStreamSource( g_pd3dDevice, StreamNumber, pStreamData, Stride );
	if( FAILED(hr) )
	{
		dbgErrMessPuts("SetStreamSource", hr) ;
		ASSERT(0);
	}
#endif

#if __STATECHK_STREAM_SOUCE__
	DG_StreamSourceData[StreamNumber]   = pStreamData ;
	DG_StreamSourceStride[StreamNumber] = Stride ;
#endif
}

void DG_SetIndices(IDirect3DIndexBuffer8* pIndexData, UINT BaseVertexIndex)
{
#if !__DG_DRAW_IN_EXEBUFFER__
	HRESULT	hr ;
#endif

#if __STATECHK_INDEICES__
	if(  (DG_IndeciesData == pIndexData)
	  && (DG_IndeciesBaseVertexIndex == BaseVertexIndex) )
	{
		return ;
	}
#endif

#if __DG_DRAW_IN_EXEBUFFER__
	DG_SetIndices_EB(pIndexData, BaseVertexIndex) ;
#else
	hr = IDirect3DDevice8_SetIndices( g_pd3dDevice, pIndexData, BaseVertexIndex );
#ifdef DEBUG_MODE
	if( FAILED(hr) )
	{
		dbgErrMessPuts("SetIndices", hr) ;
		ASSERT(0);
	}
#endif
#endif

#if __STATECHK_INDEICES__
	DG_IndeciesData            = pIndexData ;
	DG_IndeciesBaseVertexIndex = BaseVertexIndex ;
#endif
}

/*
 add 2002/02/04 M.Kobayashi
 シェーダをリンクする構造にした
 この部分自動生成にしたほうがいい
*/
typedef struct SHADER_TABLE{
	int id;
	unsigned char* pShader;
} SHADER_TABLE;

#if 0
extern unsigned char VERTEX_SHADER_kms_sl[];
extern unsigned char VERTEX_SHADER_comdl[];
extern unsigned char VERTEX_SHADER_evm[];
extern unsigned char VERTEX_SHADER_evml[];
extern unsigned char VERTEX_SHADER_obj[];
extern unsigned char VERTEX_SHADER_obja[];
extern unsigned char VERTEX_SHADER_objal[];
extern unsigned char VERTEX_SHADER_objaw[];
extern unsigned char VERTEX_SHADER_objawl[];
extern unsigned char VERTEX_SHADER_objl[];
extern unsigned char VERTEX_SHADER_objp[];
extern unsigned char VERTEX_SHADER_objw[];
extern unsigned char VERTEX_SHADER_objwl[];
extern unsigned char VERTEX_SHADER_patch[];
extern unsigned char VERTEX_SHADER_patchbl[];
extern unsigned char VERTEX_SHADER_patchl[];
extern unsigned char VERTEX_SHADER_shadow[];
extern unsigned char PIXEL_SHADER_patchbl[];
extern unsigned char PIXEL_SHADER_plight[];
extern unsigned char PIXEL_SHADER_test[];

static SHADER_TABLE table_vsh[] = {
	723881, VERTEX_SHADER_kms_sl,	/* kms_sl */
	6897906, VERTEX_SHADER_comdl,
	107309,	 VERTEX_SHADER_evm,
	3433996, VERTEX_SHADER_evml,
	116906,	 VERTEX_SHADER_obj,
	3741089, VERTEX_SHADER_obja,
	2274451, VERTEX_SHADER_objal,
	2274462, VERTEX_SHADER_objaw,
	5674032, VERTEX_SHADER_objawl,
	3741100, VERTEX_SHADER_objl,
	3741104, VERTEX_SHADER_objp,
	3741111, VERTEX_SHADER_objw,
	2275155, VERTEX_SHADER_objwl,
	3300559, VERTEX_SHADER_patch,
	7555445, VERTEX_SHADER_patchbl,
	4954706, VERTEX_SHADER_patchl,
	11673411,VERTEX_SHADER_shadow,
};


static SHADER_TABLE table_psh[] = {
	7555445, PIXEL_SHADER_patchbl,
	16132698,PIXEL_SHADER_plight,
	3908308, PIXEL_SHADER_test,
};
#endif

/*
 add 2001/05/10 M.Kobayashi
 ピクセルシェーダのための拡張
 win32 と x でフォーマット,API が違いやがる
*/
#if 0
void	DG_CreatePixelShader(int id, DWORD* pHandle)
{
#ifndef _XBOX
//	DWORD* pDef = (DWORD*)GV_GetCache( GV_CacheID(id, 'u') );
	DWORD* pDef = NULL;
	int i;
	SHADER_TABLE* pTab;
	for( i = sizeof( table_psh ) / sizeof(SHADER_TABLE), pTab = table_psh; i > 0 ; i--, pTab++ ) {
		if( pTab->id == id ) {
			pDef = (DWORD*)pTab->pShader;
			break;
		}
	}
	ASSERT( pDef != NULL );
	
	if( FAILED(	IDirect3DDevice8_CreatePixelShader( g_pd3dDevice, pDef, pHandle ) ) ) {
		printf("can't create pixel shader \n");
		HANGUP();
	}
#else
//	D3DPIXELSHADERDEF_FILE* pDef = (D3DPIXELSHADERDEF_FILE*)GV_GetCache( GV_CacheID(id, 'u') );
	D3DPIXELSHADERDEF_FILE* pDef = NULL;
	int i;
	SHADER_TABLE* pTab;
	for( i = sizeof( table_psh ) / sizeof(SHADER_TABLE), pTab = table_psh; i > 0 ; i--, pTab++ ) {
		if( pTab->id == id ) {
			pDef = (D3DPIXELSHADERDEF_FILE*)pTab->pShader;
			break;
		}
	}
	ASSERT( pDef != NULL );
	
	if( FAILED(	IDirect3DDevice8_CreatePixelShader( g_pd3dDevice,  &pDef->Psd, pHandle ) ) ) {
		printf("can't create pixel shader \n");
		HANGUP();
	}
#endif
}
#endif


#ifndef _WINDOWS
void	DG_MakePixelShader( DG_PIXELSHADER *shader, void *code )
{
#if 0
	if( FAILED(	IDirect3DDevice8_CreatePixelShader( g_pd3dDevice,
												   &((D3DPIXELSHADERDEF_FILE*)code)->Psd,
												   &shader->handle ) ) ) {
		printf("can't create pixel shader \n");
		HANGUP();
	}
#else
	shader->prog_addr = &((D3DPIXELSHADERDEF_FILE*)code)->Psd ;
#endif
}

#else
void	DG_MakePixelShader( DG_PIXELSHADER *shader, int type )
{
	HRESULT	hr ;
	PShInfo	*psh_info ;
	void	*buffer ;

	/*-- PixselShader実装判定 ----------------------------------*/

	if( !DG_CheckPixelShaderUseable() )
	{
		shader->handle = (int)NULL ;
		return ;
	}
	/*----------------------------------------------------------*/

	/*-- すでに初期化されていそうならしない --------------------*/

	if( shader->psh_type != PSHT_NULL ){ return ; }
	/*----------------------------------------------------------*/

	/*-- 使用可能か判定 ----------------------------------------*/

	psh_info = getPixelShaderInfo(type) ;
	if( !psh_info->act ){ return ; }
	/*----------------------------------------------------------*/

	/*-- ハンドル作成 ------------------------------------------*/

	shader->psh_type = type ;

	buffer = (psh_info->buff)->lpVtbl->GetBufferPointer(psh_info->buff) ;
	hr = IDirect3DDevice8_CreatePixelShader( g_pd3dDevice, buffer,
					&shader->handle) ;

	if( FAILED(	hr ) )
	{
#ifdef DEBUG_MODE
		char	strbuff[256] ;
		sprintf(strbuff, "can't create pixel shader[%d:%s]", type, _psh_name_tbl[type]) ;
		dbgErrMessPuts(strbuff, hr) ;
#endif
		/*-- エラーメッセージ ----------------------------------------------*/

		X2W_ErrorPutsHResult(X2W_MB_ERROR, hr) ;
		X2W_ErrorAbort() ;
		/*------------------------------------------------------------------*/
		return ;
	}
	/*----------------------------------------------------------*/
}
#endif

void	DG_KillPixelShader( DG_PIXELSHADER *shader )
{
	/*-- ハンドル解放 ------------------------------------------*/

	if( shader->handle ){ DG_DeletePixelShader(shader->handle) ; }

	shader->psh_type = PSHT_NULL ;
	shader->handle   = (DWORD)NULL ;
	/*----------------------------------------------------------*/
}

void DG_FreePixelShader( DG_PIXELSHADER *shader )
{
#ifndef _WINDOWS
#if 0
	if ( FAILED( IDirect3DDevice8_DeletePixelShader( g_pd3dDevice, shader->handle ) ) ){
		printf("can't release vertex shader\n");
		HANGUP();
	}
#endif
	DG_ResetPixelShader();
#endif
}


/* シェーダハンドルを初期化する。 */
/* シェーダを消去した場合に呼んでおく必要がある。 */
void DG_ResetShader(void)
{
	DG_ResetVertexShader();
	DG_ResetPixelShader();	
}

void DG_ResetPixelShader(void)
{
	DG_SetPixelShader( NULL );
}

/* 負荷軽減のため、同一ハンドルの時には登録しないように */
void DG_SetVertexShader(DWORD Handle)
{
#if !__DG_DRAW_IN_EXEBUFFER__
	HRESULT	hr ;
#endif

	if( VertexShaderHandle != Handle )
	{
#if __DG_DRAW_IN_EXEBUFFER__
		DG_SetVertexShader_EB(Handle) ;
#else
		hr = IDirect3DDevice8_SetVertexShader( g_pd3dDevice, Handle );
		ASSERT( !FAILED(hr) ) ;
#endif
		VertexShaderHandle = Handle ;
	}
}

void DG_SetVertexShaderForce(DWORD Handle)
{
#if !__DG_DRAW_IN_EXEBUFFER__
	HRESULT	hr ;
#endif

#if __DG_DRAW_IN_EXEBUFFER__
	DG_SetVertexShader_EB(Handle) ;
#else
	hr = IDirect3DDevice8_SetVertexShader( g_pd3dDevice, Handle );	// 内部状態比較無し
	ASSERT( !FAILED(hr) ) ;
#endif
	VertexShaderHandle = Handle ;
}

DWORD DG_GetVertexShader(void)
{
#if __DG_DRAW_IN_EXEBUFFER__
	return(VertexShaderHandle) ;
#else
	ASSERT(0) ;
	return(NULL) ;
#endif
}


void DG_SetPixelShader( DG_PIXELSHADER *shader )
{
#if !__DG_DRAW_IN_EXEBUFFER__
	HRESULT	hr ;
#endif

	if( !DG_CheckPixelShaderUseable() ){ return ; }

	if ( shader == NULL ){
		if( PixelShaderHandle ){
#if __DG_DRAW_IN_EXEBUFFER__
			DG_SetPixelShader_EB(0) ;
#else
			hr = IDirect3DDevice8_SetPixelShader(g_pd3dDevice, 0) ;
			ASSERT( !FAILED(hr) ) ;
#endif
			PixelShaderHandle = 0 ;
		}
	} else {
		if( PixelShaderHandle != shader->handle )
		{
#if __DG_DRAW_IN_EXEBUFFER__
			DG_SetPixelShader_EB(shader->handle) ;
#else
			hr = IDirect3DDevice8_SetPixelShader(g_pd3dDevice, shader->handle) ;
			ASSERT( !FAILED(hr) ) ;
#endif
			PixelShaderHandle = shader->handle;
		}
	}
}

void DG_DeleteVertexShader(DWORD Handle)
{
	if ( FAILED( IDirect3DDevice8_DeleteVertexShader( g_pd3dDevice, Handle ) ) ){
		printf("can't release vertex shader\n");
		HANGUP();
	}
	DG_ResetVertexShader();
}
void DG_DeletePixelShader(DWORD Handle)
{
	if ( FAILED( IDirect3DDevice8_DeletePixelShader( g_pd3dDevice, Handle ) ) ){
		printf("can't release vertex shader\n");
		HANGUP();
	}
	DG_ResetPixelShader();
}

void DG_RestoreVertexShaderConstant(void)
{
#if __STATECHK_VERTEXSHADERCONSTANT__

#if !__DG_DRAW_IN_EXEBUFFER__
	HRESULT	hr ;
#endif

#if __DG_DRAW_IN_EXEBUFFER__
	DG_SetVertexShaderConstant_EB(0, DG_VertexShaderConstant,
							MAX_VERTEXSHADER_CONSTANT_NUM) ;
#else
	hr = IDirect3DDevice8_SetVertexShaderConstant(0, DG_VertexShaderConstant,
							MAX_VERTEXSHADER_CONSTANT_NUM);
	ASSERT( !FAILED(hr)) ;
#endif

#endif	// __STATECHK_VERTEXSHADERCONSTANT__
}

void DG_SetVertexShaderConstantForce(DWORD Register,
								CONST void* pConstantData,
								DWORD  ConstantCount)
{
#if !__DG_DRAW_IN_EXEBUFFER__
	HRESULT	hr ;
#endif

#if __DG_DRAW_IN_EXEBUFFER__
	DG_SetVertexShaderConstant_EB( Register, pConstantData, ConstantCount );
#else
	hr = IDirect3DDevice8_SetVertexShaderConstant( g_pd3dDevice, Register, pConstantData, ConstantCount );
	ASSERT( !FAILED(hr)) ;
#endif

#if __STATECHK_VERTEXSHADERCONSTANT__
	memcpy(&DG_VertexShaderConstant[Register], pConstantData,
			sizeof(FVECTOR) * ConstantCount) ;
#endif
}

void DG_SetVertexShaderConstant(DWORD Register,
								CONST void* pConstantData,
								DWORD  ConstantCount)
{
	int		i ;
	float	*set ;
	float	*cur ;
	DWORD	sofs ;
#if !__DG_DRAW_IN_EXEBUFFER__
	HRESULT	hr ;
#endif

#if !__STATECHK_VERTEXSHADERCONSTANT__

#if __DG_DRAW_IN_EXEBUFFER__
	DG_SetVertexShaderConstant_EB(Register, pConstantData, ConstantCount) ;
#else
	hr = IDirect3DDevice8_SetVertexShaderConstant( g_pd3dDevice,
							Register, pConstantData, ConstantCount );
#endif
#else
	/* 前回の状態と比較 */
	set = (float *)pConstantData ;
	cur = (float *)&DG_VertexShaderConstant[Register] ;

	for(i=ConstantCount; i>0; i--, set+=4, cur+=4)
	{
		if(  (set[0] != cur[0])
		  || (set[1] != cur[1])
		  || (set[2] != cur[2])
		  || (set[3] != cur[3]) )
		{
			break ;	// 設定の必要あり
		}
	}
	if( !i ){ return ; }	// 設定の必要無し

	/* 設定 */
	sofs = ConstantCount - i ;
	if( sofs )
	{
		Register                 += sofs ;
		(FVECTOR *)pConstantData += sofs ;
		ConstantCount            -= sofs ;
	}

#if __DG_DRAW_IN_EXEBUFFER__
	DG_SetVertexShaderConstant_EB(Register, pConstantData, ConstantCount) ;
#else
	hr = IDirect3DDevice8_SetVertexShaderConstant( g_pd3dDevice,
										Register, pConstantData, ConstantCount);
#endif
	memcpy(&DG_VertexShaderConstant[Register], pConstantData,
				sizeof(FVECTOR) * ConstantCount) ;
#endif

#if !__DG_DRAW_IN_EXEBUFFER__
	ASSERT( !FAILED(hr) ) ;
#endif
}

void DG_GetVertexShaderConstant(DWORD Register,	void* pConstantData,
								DWORD  ConstantCount)
{
#if !__STATECHK_VERTEXSHADERCONSTANT__
	ASSERT(0) ;		// 数値は保存されていません
#else
	memcpy(pConstantData, &DG_VertexShaderConstant[Register],
				sizeof(FVECTOR) * ConstantCount) ;
#endif
}

/* 新設/マトリクスを転置してシェーダに設定 */
void DG_SetVertexShaderConstantMatrix(DWORD Register,
									  CONST MATRIX *Matrix,
									  DWORD nMatrix)
{
	int i;
	MATRIX matTmp;

	for (i = nMatrix; i > 0; i--, Matrix++, Register += 4) {
		D3DXMatrixTranspose((void *)&matTmp, (void *)Matrix);
		DG_SetVertexShaderConstant(Register, &matTmp, 4);
	}
}

void DG_SetPixelShaderConstant(DWORD Register,
								CONST void* pConstantData,
								DWORD  ConstantCount)
{
#if __DG_DRAW_IN_EXEBUFFER__
	DG_SetPixelShaderConstant_EB(Register, pConstantData, ConstantCount) ;
#else
	HRESULT	hr ;

	hr = IDirect3DDevice8_SetPixelShaderConstant( g_pd3dDevice, Register, pConstantData, ConstantCount );
	ASSERT( !FAILED(hr) ) ;
#endif
}

void DG_SetViewportDirect(D3DVIEWPORT8 *viewport) // 内部補正無し
{
#if __DG_DRAW_IN_EXEBUFFER__
	DG_SetViewport_EB(viewport) ;

#else
	HRESULT	hr ;

	hr = IDirect3DDevice8_SetViewport( g_pd3dDevice, viewport ) ;
#ifdef DEBUG_MODE
	if( FAILED(hr) )
	{
		printf("viewport(%d,%d,%d,%d)\n", viewport->X, viewport->Y,
							viewport->Width, viewport->Height) ;

		ASSERT( 0 ) ;
	}
#endif
#endif
}

void DG_SetViewport(DG_VIEWPORT *viewport)
{
	D3DVIEWPORT8	vp ;
	float			nrm_w, nrm_h ;

	nrm_w = DG_render_target_view_nrm_w ;
	nrm_h = DG_render_target_view_nrm_h ;

	vp.X     = (int)(viewport->lx * nrm_w) ;
	vp.Y     = (int)(viewport->ly * nrm_h) ;
	vp.Width = (int)(viewport->width * nrm_w) ;
	vp.Height= (int)(viewport->height * nrm_h) ;
	vp.MinZ  = viewport->min_z ;
	vp.MaxZ  = viewport->max_z ;
	DG_SetViewportDirect(&vp) ;
}

void DG_BeginExeScene(void)
{
	DG_InExeScene = TRUE ;
}

void DG_EndExeScene(void)
{
	DG_InExeScene = FALSE ;
}

void DG_SetLight(DWORD Index, D3DLIGHT8 *pLight)
{
#if __STATECHK_SETLIGHT__
	D3DLIGHT8	*cur_lgt ;

	cur_lgt = &DG_SetLightValue[Index] ;
	if(  (cur_lgt->Direction.x == pLight->Direction.x)
	  && (cur_lgt->Direction.y == pLight->Direction.y)
	  && (cur_lgt->Direction.z == pLight->Direction.z)
	  && (cur_lgt->Diffuse.r   == pLight->Diffuse.r)
	  && (cur_lgt->Diffuse.g   == pLight->Diffuse.g)
	  && (cur_lgt->Diffuse.b   == pLight->Diffuse.b)
	  && (cur_lgt->Diffuse.a   == pLight->Diffuse.a)
	  &&  (cur_lgt->Type        == pLight->Type) )
	{
		/* MGS2で使用する物のみの比較 */

		return ;	// 変更無し
	}
#endif	// __STATECHK_SETLIGHT__

#if __DG_DRAW_IN_EXEBUFFER__
	DG_SetLight_EB(Index, pLight) ;
#else
	HRESULT	hr ;

	hr = IDirect3DDevice8_SetLight(g_pd3dDevice, Index, pLight) ;
	ASSERT( !FAILED(hr) ) ;
#endif

#if __STATECHK_SETLIGHT__
	*cur_lgt = *pLight ;
#endif
}

void DG_LightEnable(DWORD Index, BOOL bEnable)
{
#if __STATECHK_LIGHTENABLE__
	if( DG_LightEnableValue[Index] == bEnable ){ return ; }
#endif

#if __DG_DRAW_IN_EXEBUFFER__
	DG_LightEnable_EB(Index, bEnable) ;
#else
	HRESULT	hr ;

	hr = IDirect3DDevice8_LightEnable(g_pd3dDevice, Index, bEnable) ;
	ASSERT( !FAILED(hr) ) ;
#endif

#if __STATECHK_LIGHTENABLE__
	DG_LightEnableValue[Index] = bEnable ;
#endif
}

void DG_LightEnableAll( BOOL bEnable)
{
#if !__STATECHK_LIGHTENABLE__
	int		i ;
	for(i=0; i<LIGHT_ID_MAX; i++){ DG_LightEnable(i, bEnable) ; }
#else
	int		i ;
	BOOL	*cur_val ;

	cur_val = DG_LightEnableValue ;
	for(i=0; i<LIGHT_ID_MAX; i++, cur_val++)
	{
		if( *cur_val != bEnable )
		{
#if __DG_DRAW_IN_EXEBUFFER__
			DG_LightEnable_EB(i, bEnable) ;
#else
			HRESULT	hr ;

			hr = IDirect3DDevice8_LightEnable(g_pd3dDevice, i, bEnable) ;
			ASSERT( !FAILED(hr) ) ;
#endif
			*cur_val = bEnable ;
		}
	}
#endif
}

static void DG_SetLightMatrix2Direct3D_SetDirLgt(FMATRIX *dir_mtx, FMATRIX *col_mtx)
{
	int			i ;
	float		*dir_ptr ;
	FVECTOR		*col_vec ;
	D3DLIGHT8	lgt ;

	/*-- Directional Light -----------------------------------------------------*/

	ZeroMemory(&lgt, sizeof(D3DLIGHT8)) ;
	lgt.Type = D3DLIGHT_DIRECTIONAL ;

	dir_ptr = (float *)(dir_mtx->m[0]) ;
	col_vec = (FVECTOR *)(col_mtx->m[0]) ;
	for(i=0; i<3; i++, dir_ptr++, col_vec++)
	{
		lgt.Diffuse.r = col_vec->vx ;
		lgt.Diffuse.g = col_vec->vy ;
		lgt.Diffuse.b = col_vec->vz ;
		if(  (lgt.Diffuse.r == 0.0f)
		  && (lgt.Diffuse.g == 0.0f) 
		  && (lgt.Diffuse.b == 0.0f))
		{
			DG_LightEnable(i, FALSE) ;	// 使用しない
			continue ;
		}

		lgt.Direction.x = -dir_ptr[0] ;
		lgt.Direction.y = -dir_ptr[4] ;
		lgt.Direction.z = -dir_ptr[8] ;
		lgt.Diffuse.r  *= (1.0f/255.0f) ;
		lgt.Diffuse.g  *= (1.0f/255.0f) ;
		lgt.Diffuse.b  *= (1.0f/255.0f) ;

#if TRUE
#ifdef DEBUG_MODE
		if( ((lgt.Direction.x*lgt.Direction.x)
			+(lgt.Direction.y*lgt.Direction.y)
			+(lgt.Direction.z*lgt.Direction.z)) < 0.5f )
		{
			float	coef ;

			lgt.Direction.x = frnd() ;	// デバッグ点滅
			lgt.Direction.y = frnd() ;
			lgt.Direction.z = frnd() ;

			coef = 1.0f/DG_SQRT(((lgt.Direction.x*lgt.Direction.x)
					+(lgt.Direction.y*lgt.Direction.y)
					+(lgt.Direction.z*lgt.Direction.z))) ;
			lgt.Direction.x *= coef ;
			lgt.Direction.y *= coef ;
			lgt.Direction.z *= coef ;

			lgt.Diffuse.r = 1.0f ;
			lgt.Diffuse.g = -0.5f ;
			lgt.Diffuse.b = -0.5f ;
		}
#endif
#endif
		DG_SetLight(i,    &lgt) ;
		DG_LightEnable(i, TRUE) ;
	}
	/*--------------------------------------------------------------------------*/
}

void DG_SetLightMatrix2Direct3D(FMATRIX *dir_mtx, FMATRIX *col_mtx)
{
	DWORD		d3dcol ;
	FVECTOR		vec ;

	/*-- Ambient ---------------------------------------------------------------*/

	vec = *(FVECTOR *)(col_mtx->m[3]) ;	// Ambient
	d3dcol = (0xff << 24)
		| (((DWORD)vec.vx) << 16)
		| (((DWORD)vec.vy) << 8)
		| (((DWORD)vec.vz) << 0) ;

	DG_SetRenderState(D3DRS_AMBIENT, d3dcol) ;
	/*--------------------------------------------------------------------------*/

	/*-- Directional Light -----------------------------------------------------*/

	DG_SetLightMatrix2Direct3D_SetDirLgt(dir_mtx, col_mtx) ;
	/*--------------------------------------------------------------------------*/

}

/*------------------------------------------------------------------------------*/
/* VertexShader用FogParamをRenderStateに設定									*/
/*------------------------------------------------------------------------------*/
void DG_SetRenderStateFogParam(float param1, float param2)
{
	float	start ;
	float	end ;
	float	coef ;

	coef  = 1.0f/ param1 ;
	start = -param2 * coef ;
	end   = (1.0f - param2) * coef ;

	DG_SetRenderState(D3DRS_FOGSTART, *((DWORD *)&start)) ;
	DG_SetRenderState(D3DRS_FOGEND,   *((DWORD *)&end)) ;
}

//-----------------------------------------------------------------------------
// window生成
//-----------------------------------------------------------------------------
#define	WND_CLASS_NAME	"MGS2"
#define	WND_TITLE_NAME	"Metal Gear Solid 2 : SUBSTANCE"

#include "x2w_app_config.h"


static int	DG_SelectDevide(void) ;								// Device選択
static int	DG_CheckD3DDevide(D3DCAPS8 *cap) ;					// Device使用可能検査
static int	DG_CmpD3DDevide(D3DCAPS8 *cap1, D3DCAPS8 *cap2) ;	// Device比較

HWND	DG_CreateMainWindow(HINSTANCE hInstance, WNDPROC WndProc)
{
	WNDCLASSEX wndclass;
	RECT 	rect;
	HWND 	hWnd;	
	DWORD	wnd_style ;
	DWORD	wnd_style_ex ;
	BOOL	fullscreen ;
	DWORD	dwWidth, dwHeight ;
	DWORD	create_width, create_height ;

	if( X2W_GetAppConfig(X2WAPPCFG_WINDOWED) )
	{
		fullscreen = FALSE ;
	}
	else
	{
		fullscreen = TRUE ;
	}
	dwWidth  = X2W_GetAppConfig(X2WAPPCFG_DISP_WIDTH) ;
	dwHeight = X2W_GetAppConfig(X2WAPPCFG_DISP_HEIGHT) ;

	/*-- ウィンドウクラスの登録 ---------------------------------------------*/

	wndclass.cbSize 		= sizeof(WNDCLASSEX);
	wndclass.style			= CS_CLASSDC;
	wndclass.lpfnWndProc	= WndProc;
	wndclass.cbClsExtra		= 0;
	wndclass.cbWndExtra		= 0;
	wndclass.hInstance		= hInstance;
	wndclass.hIcon			= NULL;
	wndclass.hCursor		= LoadCursor(NULL,IDC_ARROW);
	wndclass.hbrBackground	= NULL;
	wndclass.lpszMenuName	= NULL;
	wndclass.lpszClassName	= WND_CLASS_NAME;
	wndclass.hIconSm		= NULL;

	if(!RegisterClassEx(&wndclass)){ return NULL; }
	/*-----------------------------------------------------------------------*/

	/*-- メインウィンドウの作成 ---------------------------------------------*/

	if( !fullscreen )
	{
#if FALSE
		wnd_style    = WS_OVERLAPPEDWINDOW;
#else
		wnd_style    = WS_OVERLAPPED
					 | WS_CAPTION
					 | WS_SYSMENU
					 | WS_THICKFRAME
					 | WS_MINIMIZEBOX
					 | WS_MAXIMIZEBOX ;
#endif
		wnd_style_ex = 0 ;
	}
	else
	{
		wnd_style    = WS_POPUP ;
		wnd_style_ex = WS_EX_TOPMOST ;
	}

	rect.left   = 0;
	rect.top    = 0;
	rect.right  = dwWidth;
	rect.bottom = dwHeight;
	AdjustWindowRectEx(&rect,wnd_style,FALSE,wnd_style_ex);

	create_width  = rect.right - rect.left ;
	create_height = rect.bottom - rect.top ;

	hWnd = CreateWindowEx(wnd_style_ex,
							WND_CLASS_NAME, WND_TITLE_NAME,
							wnd_style,
							CW_USEDEFAULT,CW_USEDEFAULT,
							create_width, create_height,
							(HWND)NULL,(HMENU)NULL,
							hInstance,(LPSTR)NULL);
	/*-----------------------------------------------------------------------*/

	/*-- カーソル操作 -------------------------------------------------------*/

	DG_InitWindowCursor(FALSE) ;	// 消去状態初期化
	/*-----------------------------------------------------------------------*/

	/*-- 環境初期化 ---------------------------------------------------------*/

	ZeroMemory(&DG_WinApp, sizeof(DG_WINAPP)) ;

	DG_hWnd = hWnd;
	DG_WinApp.fullscreen           = fullscreen ;
	DG_WinApp.window_width         = dwWidth ;
	DG_WinApp.window_height        = dwHeight ;
	DG_WinApp.create_window_width  = create_width ;
	DG_WinApp.create_window_height = create_height ;
	DG_WinApp.window_edge_w        = create_width - dwWidth ;
	DG_WinApp.window_edge_h        = create_height - dwHeight ;
	/*-----------------------------------------------------------------------*/

	return hWnd;
}

//-----------------------------------------------------------------------------
// window表示処理
//-----------------------------------------------------------------------------
void	DG_ShowMainWindow(int nCmdShow)
{
	HWND 	hWnd ;

	hWnd = DG_hWnd ;

	/*-- Window表示開始 -----------------------------------------------------*/

	ShowWindow(hWnd, nCmdShow) ;
	if( !DG_WinApp.fullscreen )
	{
		SetActiveWindow(hWnd) ;
		SetForegroundWindow(hWnd) ;
	}
	else
	{
		SetForegroundWindow(hWnd) ;
		SetActiveWindow(hWnd) ;

		while( !SetActiveWindow(hWnd) )				// Window Active
		{
			if( !SetForegroundWindow(hWnd) )
			{
				/*-- Foreground処理に変更できなかった場合の処理 -------------*/	

				ShowWindow(hWnd, SW_MINIMIZE) ;				// とりあえずアイコン化
				while( GetForegroundWindow() != hWnd ) ;	// Foreground待ち

				ShowWindow(hWnd, SW_RESTORE) ;	// アイコン化解除
				/*-----------------------------------------------------------*/
			}
		}
	}
	/*-----------------------------------------------------------------------*/

	DG_WinApp.app_flag |= M_DG_WINAPPAPP_DISPLAY ;	// Display On(安全策)
}

//-----------------------------------------------------------------------------
// windowメッセージ処理
//-----------------------------------------------------------------------------
static void DG_WndProc_Sizing(WPARAM wParam, LPARAM lParam) ;

BOOL DG_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL	ret ;

#ifdef DEBUG_MODE
	//printf("WM[%04X]\n", msg) ;
#endif

	ret = FALSE ;
	if( DG_WinApp.fullscreen )
	{
		/*-- Full Screen ----------------------------------------------------*/

		switch( msg )
		{
		  /* アクティブ状態変更 */
		  case WM_ACTIVATE:
			switch( wParam )
			{
			  case 0 :
				ShowWindow(hWnd, SW_MINIMIZE) ;	// アイコン化して退避
				break ;

			  case 1 :
			  case 2 :
				ShowWindow(hWnd, SW_RESTORE) ;		// 復帰
				break ;
			}
			break ;

		  /* フォーカス消去 */
		  case WM_KILLFOCUS:
			/* Full Screenの場合、アイコン化して退避 */
			ShowWindow(hWnd, SW_MINIMIZE) ;
			break ;

		  /* フォーカスを得た場合はサイズを元に戻す */
		  case WM_SETFOCUS:
			ShowWindow(hWnd, SW_RESTORE) ;			// 復帰
			break ;

		  /* サイズ変更 */
		  case WM_SIZE :
			switch( wParam )
			{
			  case SIZE_RESTORED :	// 元のサイズに復元された
			  case SIZE_MAXIMIZED :	// 最大化された
				DG_WinApp.app_flag |= M_DG_WINAPPAPP_DISPLAY ;	// Display On
				break ;

			  case SIZE_MINIMIZED :	// アイコン化された
				DG_WinApp.app_flag &= ~M_DG_WINAPPAPP_DISPLAY ;	// Display Off
				break ;
			}
			break ;

		  /* カーソルがWindow上に存在(移動)している */
		  case WM_SETCURSOR:
			if( X2W_GetErrorDisplayStatus() )
			{
				DG_ShowWindowCursor(TRUE) ;
			}
			else
			{
				BOOL	show_req ;

				if( !DG_CheckAppDisplay() ){ show_req = TRUE ; }
				else{ show_req = FALSE ; }

				DG_ShowWindowCursor(show_req) ;
			}
			break ;
		}
		/*-------------------------------------------------------------------*/
	}
	else
	{
		/*-- Window Mode ----------------------------------------------------*/

		switch( msg )
		{
		  /* ウィンドウサイズ変更の最大/最小サイズの取得 */
		  case WM_GETMINMAXINFO:
			if( DG_WinApp.window_height )
			{
				MINMAXINFO	*mminfo ;
				LONG		w, h ;

				mminfo = (MINMAXINFO *)lParam ;

				/*-- 最小幅に合わせて最小サイズ補正 -------------------------*/

				w = mminfo->ptMinTrackSize.x - DG_WinApp.window_edge_w ;

				h = (DG_WinApp.window_height * w) / DG_WinApp.window_width ;
				h += DG_WinApp.window_edge_h ;

				mminfo->ptMinTrackSize.y = h ;
				/*-----------------------------------------------------------*/
			}
			break ;

		  /* WINDOWSサイズの変更中 */
		  case WM_SIZING :
			DG_WndProc_Sizing(wParam, lParam) ;
			break ;

		  /* サイズ変更 */
		  case WM_SIZE :
			switch( wParam )
			{
			  case SIZE_RESTORED :	// 元のサイズに復元された
			  case SIZE_MAXIMIZED :	// 最大化された
				DG_WinApp.app_flag |= M_DG_WINAPPAPP_DISPLAY ;	// Display On
				break ;

			  case SIZE_MINIMIZED :	// アイコン化された
				DG_WinApp.app_flag &= ~M_DG_WINAPPAPP_DISPLAY ;	// Display Off
				break ;
			}
			break ;

		  /* カーソルがWindow上に存在(移動)している */
		  case WM_SETCURSOR:
			if( X2W_GetErrorDisplayStatus() )
			{
				DG_ShowWindowCursor(TRUE) ;
			}
			else
			{
				BOOL	show_req ;


				if( !DG_CheckAppDisplay() )
				{
					show_req = TRUE ;
				}
				else
				{
					if( (lParam & 0xffff) != 1 ){ show_req = TRUE ; }
					else{ show_req = FALSE ; }
				}

				DG_ShowWindowCursor(show_req) ;
			}
			break ;

		  
		  case WM_PAINT:
			if( DG_CheckAppDisplay() && X2W_GetErrorDisplayStatus() )
			{
				/* 表示中でメッセージボックス表示中は、切り替えでごまかす */
				IDirect3DDevice8_Present(g_pd3dDevice, NULL, NULL, NULL, NULL) ;
			}
			break ;
		}
		/*-------------------------------------------------------------------*/
	}
	return(ret) ;
}


//-----------------------------------------------------------------------------
// windowサイズ変更処理
//-----------------------------------------------------------------------------
#define	DG_WINPROC_SIZEING_ADJ_WIDTH	(32)
#define	DG_WINPROC_SIZEING_ADJ_HEIGHT	(32)

static void DG_WndProc_Sizing(WPARAM wParam, LPARAM lParam) 
{
	RECT	rect ;
	LONG	w, h ;
	LONG	adj_w, adj_h ;
	LONG	mp ;

	rect = *(RECT *)lParam ;

	w = (rect.right  - rect.left) - DG_WinApp.window_edge_w ;
	h = (rect.bottom - rect.top)  - DG_WinApp.window_edge_h ;

	switch( wParam )
	{
	  case WMSZ_TOPLEFT :
	  case WMSZ_TOPRIGHT :
	  case WMSZ_BOTTOMLEFT :
	  case WMSZ_BOTTOMRIGHT :

		adj_w = ((DG_WinApp.window_width * h) / DG_WinApp.window_height) ;
		adj_h = ((DG_WinApp.window_height * w) / DG_WinApp.window_width) ;
	
		/*-- 隅補正 ---------------------------------------------------------*/

		if( adj_w > w )
		{
			/*-- X軸補正 ----------------------------------------------------*/

			adj_w += DG_WinApp.window_edge_w ;

			switch( wParam )
			{
			  case WMSZ_RIGHT :
			  case WMSZ_TOPRIGHT :
			  case WMSZ_BOTTOMRIGHT :
				rect.right = rect.left + adj_w ;
				break ;

			  case WMSZ_LEFT :
			  case WMSZ_TOPLEFT :
			  case WMSZ_BOTTOMLEFT :
				rect.left = rect.right - adj_w ;
				break ;

			  default :
				mp         = (rect.left + rect.right) >> 1 ;
				rect.left  = mp - (adj_w >> 1) ;
				rect.right = rect.left + adj_w ;
				break ;
			}
			/*---------------------------------------------------------------*/
		}
		else
		{
			/*-- Y軸補正 ----------------------------------------------------*/

			adj_h += DG_WinApp.window_edge_h ;

			switch( wParam )
			{
			  case WMSZ_TOPLEFT :
			  case WMSZ_TOPRIGHT :
				rect.top = rect.bottom - adj_h ;
				break ;

			  case WMSZ_BOTTOMLEFT :
			  case WMSZ_BOTTOMRIGHT :
				rect.bottom = rect.top + adj_h ;
				break ;

			  default :
				mp          = (rect.top + rect.bottom) >> 1 ;
				rect.top    = mp - (adj_h >> 1) ;
				rect.bottom = rect.top + adj_h ;
				break ;
			}
			/*---------------------------------------------------------------*/
		}
		/*-------------------------------------------------------------------*/
		break ;

	  default :
		/*-- 辺補正 ---------------------------------------------------------*/

		if(  (wParam == WMSZ_TOP)
		  || (wParam == WMSZ_BOTTOM) )
		{
			/*-- X軸補正 ----------------------------------------------------*/

			w = (DG_WinApp.window_width * h) / DG_WinApp.window_height ;

			w += DG_WinApp.window_edge_w ;
			mp = (rect.left + rect.right) >> 1 ;

			rect.left  = mp - (w >> 1) ;
			rect.right = rect.left + w ;
			/*---------------------------------------------------------------*/
		}
		else
		{
			/*-- Y軸補正 ----------------------------------------------------*/

			h = (DG_WinApp.window_height * w) / DG_WinApp.window_width ;
			h += DG_WinApp.window_edge_h ;

			switch( wParam )
			{
			  case WMSZ_TOPLEFT :
			  case WMSZ_TOPRIGHT :
				rect.top = rect.bottom - h ;
				break ;

			  case WMSZ_BOTTOMLEFT :
			  case WMSZ_BOTTOMRIGHT :
				rect.bottom = rect.top + h ;
				break ;

			  default :
				mp = (rect.top + rect.bottom) >> 1 ;

				rect.top    = mp - (h >> 1) ;
				rect.bottom = rect.top + h ;
				break ;
			}
			/*---------------------------------------------------------------*/
		}
		/*-------------------------------------------------------------------*/
		break ;
	}

	w = (rect.right  - rect.left) ;
	h = (rect.bottom - rect.top) ;

	/*-- 標準サイズに近い場合はそれに合わせる -------------------------------*/

	if(  (abs(w - DG_WinApp.create_window_width) < DG_WINPROC_SIZEING_ADJ_WIDTH)
	  && (abs(h - DG_WinApp.create_window_height) < DG_WINPROC_SIZEING_ADJ_HEIGHT) )
	{
		switch( wParam )
		{
		  case WMSZ_LEFT :
		  case WMSZ_TOPLEFT :
		  case WMSZ_BOTTOMLEFT :
			rect.left = rect.right - DG_WinApp.create_window_width ;
			break ;

		  case WMSZ_RIGHT :
		  case WMSZ_TOPRIGHT :
		  case WMSZ_BOTTOMRIGHT :
			rect.right = rect.left + DG_WinApp.create_window_width ;
			break ;

		  default :
			mp = (rect.left + rect.right) >> 1 ;

			rect.left  = mp - (DG_WinApp.create_window_width >> 1) ;
			rect.right = rect.left + DG_WinApp.create_window_width ;
			break ;
		}

		switch( wParam )
		{
		  case WMSZ_TOP :
		  case WMSZ_TOPLEFT :
		  case WMSZ_TOPRIGHT :
			rect.top = rect.bottom - DG_WinApp.create_window_height ;
			break ;

		  case WMSZ_BOTTOM :
		  case WMSZ_BOTTOMLEFT :
		  case WMSZ_BOTTOMRIGHT :
			rect.bottom = rect.top + DG_WinApp.create_window_height ;
			break ;

		  default :
			mp = (rect.top + rect.bottom) >> 1 ;

			rect.top    = mp - (DG_WinApp.create_window_height >> 1) ;
			rect.bottom = rect.top + DG_WinApp.create_window_height ;
			break ;
		}
	}
	/*-----------------------------------------------------------------------*/

	*(RECT *)lParam = rect ;
}

#if	__RESOURCE_MANAGER__
/*--------------------------------------------------------------------------*/
/*	DG_InitD3DResourceManager	リソース管理初期化							*/
/*--------------------------------------------------------------------------*/
static	void	DG_InitD3DResourceManager(void)
{
	ZeroMemory(&DG_ResourceMan, sizeof(DG_ResourceMan)) ;

	/*-- テーブル初期化 ----------------------------------------------------*/

	DG_InitD3DResourceTbl(&DG_ResourceMan.defpool_tbl,
						DG_ResourceMan.defpool_elems,
						MAX_DEFAULTPOOL_RESOURCE_NUM) ;
	/*----------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------*/
/*	DG_ReleaseD3DResourceManager	リソース管理終了処理					*/
/*--------------------------------------------------------------------------*/
static	void	DG_ReleaseD3DResourceManager(void)
{
	DG_ReleaseD3DResourceTbl(&DG_ResourceMan.defpool_tbl) ;
}

/*--------------------------------------------------------------------------*/
/*	DG_InitD3DResourceTbl		リソーステーブル初期化						*/
/*--------------------------------------------------------------------------*/
static	void	DG_InitD3DResourceTbl(RESOURCE_TBL *tbl, RESOURCE_ELEM *elems, DWORD max)
{
	tbl->elems_num = 0 ;
	tbl->elems_max = max ;
	tbl->elems = elems;
}

/*--------------------------------------------------------------------------*/
/*	DG_ReleaseD3DResourceTbl	リソーステーブル終了処理					*/
/*--------------------------------------------------------------------------*/
static	void	DG_ReleaseD3DResourceTbl(RESOURCE_TBL *tbl)
{
}

/*--------------------------------------------------------------------------*/
/*	DG_QueueElemD3DResourceTbl	リソーステーブル登録						*/
/*--------------------------------------------------------------------------*/
static	void	DG_QueueElemD3DResourceTbl(RESOURCE_TBL *tbl, DWORD type, void **buff_addr,
						RESOURCE_ELEM_PARAM *param, DWORD param_size)
{
	RESOURCE_ELEM	*elem ;

	if( tbl->elems_num >= tbl->elems_max ){ ASSERT(0) ; return ; }

	/*-- 空要素取得 --------------------------------------------------------*/

	elem = &tbl->elems[tbl->elems_num] ;
	tbl->elems_num++ ;
	/*----------------------------------------------------------------------*/

printf("Queue %d(0x%08X, 0x%08X)\n", type, *buff_addr, buff_addr) ;

	/*-- 要素設定 ----------------------------------------------------------*/

	elem->type = type ;
	elem->buff      = *buff_addr ;
	elem->buff_addr = buff_addr ;
	memcpy(&elem->param, param, param_size) ;
	/*----------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------*/
/*	DG_QueueElemD3DResourceTbl	リソーステーブル登録解除					*/
/*--------------------------------------------------------------------------*/
static	void	DG_DequeueElemD3DResourceTbl(RESOURCE_TBL *tbl, void *buff)
{
	DWORD			i ;
	DWORD			num ;
	RESOURCE_ELEM	*elem ;

	if( !tbl->elems_num ){ return ; }

	/*-- 要素検索 ----------------------------------------------------------*/

	elem = tbl->elems ;
	num  = tbl->elems_num ;
	for(i=num; i>0; i--, elem++)
	{
		if( elem->buff == buff ){ break ; }
	}
	if( !i ){ return ; }	// 検索失敗
	/*----------------------------------------------------------------------*/

	/*-- 要素切り詰め ------------------------------------------------------*/
	i-- ;
	for(; i>0; i--, elem++)
	{
		*elem = *(elem+1) ;
	}
	/*----------------------------------------------------------------------*/

	tbl->elems_num-- ;
}


/*--------------------------------------------------------------------------*/
/*	DG_ResetDeviceReleaseD3DResourceTbl										*/
/*				ResetDeviceの為、リソースを一旦Release						*/
/*--------------------------------------------------------------------------*/
static	void	DG_ResetDeviceReleaseD3DResourceTbl(RESOURCE_TBL *tbl)
{
	DWORD			i ;
	RESOURCE_ELEM	*elem ;
	HRESULT			hr ;
	
	/*-- 最新の物から解放 --------------------------------------------------*/

	elem = &tbl->elems[tbl->elems_num-1] ;
	for(i=tbl->elems_num; i>0; i--, elem--)
	{
		if( !elem->buff ){ continue ; }
printf("Release %d(0x%08X, 0x%08X)\n", elem->type, elem->buff, elem->buff_addr) ;
		switch( elem->type )
		{
		  case RESOURCE_ELEM_TYPE_TEXTURE :
			{
				LPDIRECT3DTEXTURE8	tex ;

				tex = (LPDIRECT3DTEXTURE8)elem->buff ;

				hr = IDirect3DTexture8_Release(tex) ;
				ASSERT( !FAILED(hr) ) ;
			}
			break ;

		  case RESOURCE_ELEM_TYPE_DEPTHSTENCILSURFACE :
			{
				LPDIRECT3DSURFACE8	sur ;

				sur = (LPDIRECT3DSURFACE8)elem->buff ;

				hr = IDirect3DSurface8_Release(sur) ;
				ASSERT( !FAILED(hr) ) ;
			}
			break ;

		  case RESOURCE_ELEM_TYPE_VBUFFER :
			{
				IDirect3DVertexBuffer8*	vbuff ;

				vbuff = (IDirect3DVertexBuffer8 *)elem->buff ;

				hr = IDirect3DVertexBuffer8_Release(vbuff) ;
				ASSERT( !FAILED(hr) ) ;
			}
			break ;

		  case RESOURCE_ELEM_TYPE_IBUFFER :
			{
				IDirect3DIndexBuffer8*	ibuff ;

				ibuff = (IDirect3DIndexBuffer8 *)elem->buff ;

				hr = IDirect3DIndexBuffer8_Release(ibuff) ;
				ASSERT( !FAILED(hr) ) ;
			}
			break ;
		}

		elem->buff = NULL ;
	}
	/*----------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------*/
/*	DG_ResetDeviceReleaseD3DResourceTbl										*/
/*				ResetDeviceの為リソースを再確保								*/
/*--------------------------------------------------------------------------*/
static	void	DG_ResetDeviceCreateD3DResourceTbl(RESOURCE_TBL *tbl, D3DPOOL Pool)
{
	DWORD			i ;
	RESOURCE_ELEM	*elem ;
	HRESULT			hr ;
	
	/*-- 古いの物から作成 --------------------------------------------------*/

	elem = tbl->elems ;
	for(i=tbl->elems_num; i>0; i--, elem++)
	{
		switch( elem->type )
		{
		  case RESOURCE_ELEM_TYPE_TEXTURE :
			{
				LPDIRECT3DTEXTURE8			*tex ;
				RESOURCE_ELEM_PARAM_TEXTURE	*param ;

				param = &elem->param.tex ;

				tex = (LPDIRECT3DTEXTURE8 *)elem->buff_addr ;
				hr = IDirect3DDevice8_CreateTexture( g_pd3dDevice,
								param->Width,
								param->Height,
								param->Levels,
								param->Usage,
								param->Format,
								Pool,
								tex);
				elem->buff = (void *)*tex ;
			}
			break ;

		  case RESOURCE_ELEM_TYPE_DEPTHSTENCILSURFACE :
			{
				LPDIRECT3DSURFACE8						*sur ;
				RESOURCE_ELEM_PARAM_DEPTHSTENCILSURFACE	*param ;

				param = &elem->param.dp_surf ;

				sur = (LPDIRECT3DSURFACE8 *)elem->buff_addr ;
				hr = IDirect3DDevice8_CreateDepthStencilSurface(g_pd3dDevice,
								param->Width,
								param->Height,
								param->Format,
								param->MultiSample,
								sur) ;
				elem->buff = (void *)*sur ;
			}
			break ;

		  case RESOURCE_ELEM_TYPE_VBUFFER :
			{
				IDirect3DVertexBuffer8		**vbuff ;
				RESOURCE_ELEM_PARAM_VBUFFER	*param ;

				param = &elem->param.vbuff ;

				vbuff = (IDirect3DVertexBuffer8 **)elem->buff_addr ;
				hr = IDirect3DDevice8_CreateVertexBuffer(g_pd3dDevice,
									param->Length,
									param->Usage,
									param->FVF,
									Pool,
									vbuff) ;
				elem->buff = (void *)*vbuff ;
			}
			break ;

		  case RESOURCE_ELEM_TYPE_IBUFFER :
			{
				IDirect3DIndexBuffer8		**ibuff ;
				RESOURCE_ELEM_PARAM_IBUFFER	*param ;

				param = &elem->param.ibuff ;

				ibuff = (IDirect3DIndexBuffer8 **)elem->buff_addr ;
				hr = IDirect3DDevice8_CreateIndexBuffer(g_pd3dDevice,
									param->Length,
									param->Usage,
									param->Format,
									Pool,
									ibuff) ;
				elem->buff = (void *)*ibuff ;
			}
			break ;
		}
printf("Create %d(0x%08X, 0x%08X)\n", elem->type, elem->buff, elem->buff_addr) ;
	}
	/*----------------------------------------------------------------------*/
}
#endif	// __RESOURCE_MANAGER__

/*--------------------------------------------------------------------------*/
/*	DG_ResetDevice_ReleaseD3DRenderingState									*/
/*--------------------------------------------------------------------------*/
void	DG_ResetDevice_ReleaseD3DRenderingState(void)
{
	/*-- Default Pool Resource の解放 --------------------------------------*/
#if	__RESOURCE_MANAGER__
	DG_ResetDeviceReleaseD3DResourceTbl(&DG_ResourceMan.defpool_tbl) ;
#endif
	/*----------------------------------------------------------------------*/

	/*-- レンダリングステートの初期化 --------------------------------------*/

	DG_InitRenderState_ResetDevice() ;
	/*----------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------*/
/*	DG_ResetDevice_CreateD3DRenderingState									*/
/*--------------------------------------------------------------------------*/
void	DG_ResetDevice_CreateD3DRenderingState(void)
{
	/*-- Default Pool Resource の再確保 ------------------------------------*/
#if	__RESOURCE_MANAGER__
	DG_ResetDeviceCreateD3DResourceTbl(&DG_ResourceMan.defpool_tbl, D3DPOOL_DEFAULT) ;
#endif
	/*----------------------------------------------------------------------*/
	
}
