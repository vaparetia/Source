//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xd3d.cpp

	Direct3D関係ラッパー関数

	2001/05/08 F.Miyauchi / DirectX オリジナル
	$Id: xd3d.c,v 1.6 2002/11/23 11:36:55 Yoshizawa1 Exp $
*/

// 同値設定の回避/コマンド発行数カウント/返値チェック等等のため、
// 頻繁に使用するものはこっちに持ってくる。
// インライン化予定。

#ifdef KP_XBOX //BP

#ifndef KP_XBOX
#include <d3dx8.h>
#else
#include <xtl.h>
#endif

#include <stdio.h>
#include "libgv.h"
#include "libdg.h"
#include "private.h"

#include "shader.h"

//#define WIRE_MODE		/* 強制ワイヤフレームモード有効化 */

typedef struct tagFSAATYPE {
	D3DMULTISAMPLE_TYPE Type;
	char                *Text;
} FSAATYPE;

static int DG_MultiSampleType = 0;

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
#if 0 //BP_RENDER
#ifdef KP_XBOX
	// この方法はXBOXのみ。
	// Windowsでは、Reset(); を呼ぶときに頂点バッファやら何やら
	// 解放する必要がある。
	g_d3dpp.MultiSampleType = (D3DMULTISAMPLE_TYPE)type;
	IDirect3DDevice9_Reset( g_pd3dDevice, &g_d3dpp );
#else
#endif
#endif
}

/* パレット関係 */
void DG_CreatePalette( int/*D3DPALETTESIZE*/ Size, void/*LPDIRECT3DPALETTE8*/ *ppPalette )
{
#if 0 //BP_RENDER
	if ( FAILED( IDirect3DDevice9_CreatePalette( g_pd3dDevice, Size, ppPalette ) ) ) {
		*ppPalette = NULL;
		ASSERT(0);
	}
#endif
}
void DG_SetPalette( DWORD Stage, void/*LPDIRECT3DPALETTE8*/ *pPalette )
{
#if 0 //BP_RENDER
	IDirect3DDevice9_SetPalette( g_pd3dDevice, Stage, pPalette );
#endif
}

/* 可視性テスト関係 */
static DWORD VisibilityTestID = 0;

/* 空きIDを返す */
/* ※1Syncに 4096個以上の可視性テストをしないことが前提。 */
DWORD DG_GetVisibilityTestID(void)
{
#if 0 //BP_RENDER
	VisibilityTestID = (VisibilityTestID + 1) % D3DVISIBILITY_TEST_MAX;
#endif
	return VisibilityTestID;
}

void DG_BeginVisibilityTest(void)
{
#if 0 //BP_RENDER
	if ( FAILED( IDirect3DDevice9_BeginVisibilityTest( g_pd3dDevice ) ) ){
		ASSERT(0);
	}
#endif
}

void DG_EndVisibilityTest( DWORD Index )
{
#if 0 //BP_RENDER
	if ( FAILED( IDirect3DDevice9_EndVisibilityTest( g_pd3dDevice, Index) ) ){
		ASSERT(0);
	}
#endif
}

/* 終了するまで待つ */
void DG_GetVisibilityTestResultBlock( DWORD Index, UINT* pResult, ULONGLONG* pTimeStamp )
{
#if 0 //BP_RENDER
	HRESULT hr;
	do {
		hr = IDirect3DDevice9_GetVisibilityTestResult( g_pd3dDevice, Index, pResult, pTimeStamp );
	} while (hr == D3DERR_TESTINCOMPLETE);
#endif
}

/* DirectX にフォグカラーを設定する */
void DG_SetDxFogColor(void)
{
	CVECTOR color;

	color.r = DG_FogColor.b;
	color.g = DG_FogColor.g;
	color.b = DG_FogColor.r;
   BP_MARCO_BREAK;
#if 0 //BP_RENDER
	// Xbox
	DG_SetRenderState( D3DRS_FOGCOLOR, *(DWORD *)&color );
#endif
}


#define TEXTURE_STAGE_MAX    (4)
static IDirect3DBaseTexture9 *DG_Texture[TEXTURE_STAGE_MAX];

static int DG_DrawPrimitiveCount = 0;
static int DG_DrawIndexedPrimitiveCount = 0;
static DWORD VertexShaderHandle;
static DWORD PixelShaderHandle;

/* ゲーム起動時のレンダリングステート/内部変数を初期設定する */
void DG_InitRenderState(void)
{
	int i;

   BP_MARCO_BREAK;
#if 0 //BP
	/* 内部変数の初期化 */
	for (i = 0; i < TEXTURE_STAGE_MAX; i++) {
		DG_Texture[i] = NULL;
	}
	DG_DrawPrimitiveCount = 0;
	DG_DrawIndexedPrimitiveCount = 0;

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
		DG_SetRenderState(D3DRS_SPECULARENABLE, TRUE);
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

#ifdef WIRE_MODE
	DG_SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
#endif

#endif //BP
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
#if 0 //BP_RENDER
	/* 拡大フィルタ/縮小フィルタ */
	DG_SetTextureStageState(Stage, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	DG_SetTextureStageState(Stage, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	//DG_SetTextureStageState(Stage, D3DTSS_MIPFILTER, D3DTEXF_NONE );
	//DG_SetTextureStageState(Stage, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
	DG_SetTextureStageState(Stage, D3DTSS_MIPFILTER, D3DTEXF_POINT);
	/* CLAMPモード */
	DG_SetTextureStageState(Stage, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	DG_SetTextureStageState(Stage, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	DG_Texture[Stage] = NULL;
	IDirect3DDevice9_SetTexture( g_pd3dDevice, Stage, NULL );

	//DG_SetTextureStageState( Stage, D3DTSS_ALPHAKILL, D3DTALPHAKILL_ENABLE );
#endif

}


/* 全般 */
void DG_Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags,
			  D3DCOLOR Color, float Z, DWORD Stencil)
{
	IDirect3DDevice9_Clear( g_pd3dDevice, Count, pRects, Flags, Color, Z, Stencil );
}

void DG_BeginScene(void)
{
	IDirect3DDevice9_BeginScene( g_pd3dDevice );
}

void DG_EndScene(void)
{
	IDirect3DDevice9_EndScene( g_pd3dDevice );
}

#ifdef KP_XBOX
void DG_Present(CONST RECT* pSourceRect, CONST RECT* pDestRect,
				void *pUnused0, void *pUnused1)
{
	IDirect3DDevice9_Present( g_pd3dDevice, pSourceRect, pDestRect, pUnused0, pUnused1 );
}
#else
void DG_Present(CONST RECT* pSourceRect, CONST RECT* pDestRect,
		   HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{
	IDirect3DDevice9_Present( g_pd3dDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}
#endif
void DG_KickPushBuffer( void )
{
#if 0 //BP_RENDER
	IDirect3DDevice9_KickPushBuffer( g_pd3dDevice );
#endif
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
 *  0〜255の値を取り、128の時に 1.0 を表す。
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

	IDirect3DDevice9_SetRenderState( g_pd3dDevice, D3DRS_ALPHABLENDENABLE, FALSE );		
	IDirect3DDevice9_SetRenderState( g_pd3dDevice, D3DRS_SRCBLEND, D3DBLEND_ONE );
	IDirect3DDevice9_SetRenderState( g_pd3dDevice, D3DRS_DESTBLEND, D3DBLEND_ZERO );
	IDirect3DDevice9_SetRenderState( g_pd3dDevice, D3DRS_BLENDOP, D3DBLENDOP_ADD );
}

/* アルファブレンディング値切り替え(変更チェック付き/鬱陶しい…) */
static inline void SetAlphaBlend(unsigned int mode)
{
	if (DG_AlphaBlend != mode) {
		DG_AlphaBlend = mode;
		IDirect3DDevice9_SetRenderState( g_pd3dDevice, D3DRS_ALPHABLENDENABLE, mode);
	}
}
static inline void SetSrcBlend(unsigned int mode)
{
	if (DG_SrcBlend != mode) {
		DG_SrcBlend = mode;
		IDirect3DDevice9_SetRenderState( g_pd3dDevice, D3DRS_SRCBLEND, mode);
	}
}
static inline void SetDstBlend(unsigned int mode)
{
	if (DG_DstBlend != mode) {
		DG_DstBlend = mode;
		IDirect3DDevice9_SetRenderState( g_pd3dDevice, D3DRS_DESTBLEND, mode);
	}
}
static inline void SetBlendOp(unsigned int mode)
{
	if (DG_BlendOp != mode) {
		DG_BlendOp = mode;
		IDirect3DDevice9_SetRenderState( g_pd3dDevice, D3DRS_BLENDOP, mode);
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
void DG_SetAlphaMode(u_long64 type)
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
#if 0	/* libdgx.h内でインライン定義 */
void DG_SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	IDirect3DDevice9_SetRenderState( g_pd3dDevice, State, Value );
}
#endif

/* 変換行列設定 */
void DG_SetTransform( D3DTRANSFORMSTATETYPE State, CONST FMATRIX *pMatrix)
{
	IDirect3DDevice9_SetTransform( g_pd3dDevice, State, (CONST D3DMATRIX *)pMatrix );
}

/* 頂点バッファ作成 */
void DG_CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool,
						   IDirect3DVertexBuffer9** ppVertexBuffer)
{
#if 0 //BP_RENDER
	HRESULT hr;
	hr = IDirect3DDevice9_CreateVertexBuffer( g_pd3dDevice, Length, Usage, FVF, Pool, ppVertexBuffer );
	if (FAILED(hr)) {
		printf("CreateVertexBuffer failed.\n");
		ASSERT(0);
	}
#endif
}

/* インデックスバッファ作成 */
void DG_CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format,
						  D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer)
{
#if 0 //BP_RENDER
	HRESULT hr;
	hr = IDirect3DDevice9_CreateIndexBuffer( g_pd3dDevice, Length, Usage, Format, Pool, ppIndexBuffer );
	if (FAILED(hr)) {
		printf("CreateIndexBuffer failed.\n");
		ASSERT(0);
	}
#endif
}

void DG_SetRenderTarget(IDirect3DSurface9* pRenderTarget,
						IDirect3DSurface9* pNewZStencil)
{
#if 0 //BP_RENDER
	HRESULT hr;
	hr = IDirect3DDevice9_SetRenderTarget( g_pd3dDevice, pRenderTarget, pNewZStencil );
	if (FAILED(hr)) {
		printf("SetRenderTarget failed\n");
		ASSERT(0);
	}
#endif
}

void DG_GetRenderTarget(IDirect3DSurface9** ppRenderTarget)
{
#if 0 //BP_RENDER
	HRESULT hr;
	hr = IDirect3DDevice9_GetRenderTarget( g_pd3dDevice, ppRenderTarget );
	if (FAILED(hr)) {
		printf("GetRenderTarget failed\n");
		ASSERT(0);
	}
#endif
}

void DG_GetDepthStencilSurface(IDirect3DSurface9 **ppZStencilSurface)
{
#if 0 //BP_RENDER
	HRESULT hr;
	hr = IDirect3DDevice9_GetDepthStencilSurface( g_pd3dDevice, ppZStencilSurface );
	if (FAILED(hr)) {
		printf("GetDepthStencilSurface failed\n");
		ASSERT(0);
	}
#endif
}

/* テクスチャ関連 */
void DG_CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage,
					  D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture)
{
#if 0 //BP_RENDER
	HRESULT hr;
	hr = IDirect3DDevice9_CreateTexture( g_pd3dDevice, Width, Height, Levels, Usage, Format, Pool, ppTexture);
	if (FAILED(hr)) {
		printf("CreateTexture (%d, %d) failed\n", Width, Height);
		ASSERT(0);
	}
#endif
}

void DG_CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage,
							D3DFORMAT Format, D3DPOOL Pool,
							IDirect3DVolumeTexture9** ppVolumeTexture)
{
#if 0 //BP_RENDER
	HRESULT hr;
	hr = IDirect3DDevice9_CreateVolumeTexture( g_pd3dDevice, Width, Height, Depth, Levels, Usage,
											  Format, Pool, ppVolumeTexture);
	if (FAILED(hr)) {
		printf("CreateVolumeTexture (%d, %d, %d) failed\n", Width, Height, Depth);
		ASSERT(0);
	}
#endif
}


#if 0
void DG_SetTexture(DWORD Stage, IDirect3DBaseTexture9 *pTexture)
{
#ifdef WIRE_MODE
	IDirect3DDevice9_SetTexture( g_pd3dDevice, Stage, NULL );
	return ;
#endif
	/* 同一テクスチャはセットしない */
	if (pTexture != DG_Texture[Stage]) {
		IDirect3DDevice9_SetTexture( g_pd3dDevice, Stage, pTexture );
		DG_Texture[Stage] = pTexture;
		if ( pTexture == NULL ){
			if ( Stage == 0 ){
				/* PS2に合わせて、TEXTURE * DIFFUSE * 2 を最終的な色にする */
				DG_SetTextureStageState( Stage, D3DTSS_COLOROP,   D3DTOP_MODULATE);
				/* アルファは、TEXTURE ... 0x80, DIFFUSE ... 0x80 で テクスチャの色が出るように */
				//DG_SetTextureStageState( Stage, D3DTSS_ALPHAOP,   D3DTOP_MODULATE2X);
			} else {
				DG_SetTextureStageState( Stage, D3DTSS_COLOROP,   D3DTOP_DISABLE);
			}
		} else {
			/* PS2に合わせて、TEXTURE * DIFFUSE * 2 を最終的な色にする */
			DG_SetTextureStageState( Stage, D3DTSS_COLOROP,   D3DTOP_MODULATE2X);
			/* アルファは、TEXTURE ... 0x80, DIFFUSE ... 0x80 で テクスチャの色が出るように */
			//DG_SetTextureStageState( Stage, D3DTSS_ALPHAOP,   D3DTOP_MODULATE2X);
		}
	} else {
		/* 念のため変更されている可能性があるため再設定しておく */
		if ( pTexture == NULL ){
			/* PS2に合わせて、TEXTURE * DIFFUSE * 2 を最終的な色にする */
			DG_SetTextureStageState( Stage, D3DTSS_COLOROP,   D3DTOP_MODULATE);
		} else {
			/* PS2に合わせて、TEXTURE * DIFFUSE * 2 を最終的な色にする */
			DG_SetTextureStageState( Stage, D3DTSS_COLOROP,   D3DTOP_MODULATE2X);
		}
	}
}
#else
void DG_SetTexture( int stage, DG_TEX_TRANS *tex_trans )
{
#if 0 //BP_RENDER
	IDirect3DBaseTexture9 *pTexture ;
#ifdef WIRE_MODE
	IDirect3DDevice9_SetTexture( g_pd3dDevice, Stage, NULL );
	return ;
#endif
	if ( tex_trans == NULL ){
		IDirect3DDevice9_SetTexture( g_pd3dDevice, stage, NULL );
		IDirect3DDevice9_SetPalette( g_pd3dDevice, stage, NULL );
		if ( stage == 0 ){
			DG_SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_MODULATE );
		} else {
			DG_SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_DISABLE );
		}
	} else {
		IDirect3DDevice9_SetTexture( g_pd3dDevice, stage, tex_trans->ptex );
		IDirect3DDevice9_SetPalette( g_pd3dDevice, stage, tex_trans->ppal );
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
#endif
}
void DG_SetTextureDirect( int stage, IDirect3DBaseTexture9 *pTexture )
{
	 ;
#ifdef WIRE_MODE
	IDirect3DDevice9_SetTexture( g_pd3dDevice, stage, NULL );
	return ;
#endif
	 IDirect3DDevice9_SetTexture( g_pd3dDevice, stage, pTexture );
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
#endif

void DG_SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	IDirect3DDevice9_SetTextureStageState( g_pd3dDevice, Stage, Type, Value );
}

/* DrawPrimitive/描画関連 */
void DG_DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	if (PrimitiveCount != 0) {
		IDirect3DDevice9_DrawPrimitive( g_pd3dDevice, PrimitiveType, StartVertex, PrimitiveCount );
	}
}

void DG_DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount,
						CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	IDirect3DDevice9_DrawPrimitiveUP( g_pd3dDevice, PrimitiveType, PrimitiveCount,
									 pVertexStreamZeroData, VertexStreamZeroStride );
}

void DG_DrawIndexedPrimitive(D3DPRIMITIVETYPE Type, UINT MinIndex, UINT NumVertices,
							 UINT StartIndex, UINT PrimitiveCount)
{
#if 0 //BP_RENDER
	IDirect3DDevice9_DrawIndexedPrimitive( g_pd3dDevice, Type, MinIndex, NumVertices,
										  StartIndex, PrimitiveCount);
#endif
}

void DG_DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex,
							   UINT NumVertices, UINT PrimitiveCount,
							   CONST void* pIndexData, D3DFORMAT IndexDataFormat,
							   CONST void* pVertexStreamZeroData,
							   UINT VertexStreamZeroStride)
{
	IDirect3DDevice9_DrawIndexedPrimitiveUP( g_pd3dDevice, 
			PrimitiveType, MinIndex, NumVertices, PrimitiveCount,
			pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride );
}


void DG_DrawVertices( D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT VertexCount )
{
#if 0 //BP_RENDER
   if ( VertexCount != 0) {
		IDirect3DDevice9_DrawVertices( g_pd3dDevice, PrimitiveType, StartVertex, VertexCount );
	}
#endif
}

void DG_DrawIndexedVertices( D3DPRIMITIVETYPE PrimitiveType, UINT VertexCount, CONST WORD *pIndexData )
{
#if 0 //BP_RENDER
   if ( VertexCount != 0) {
		IDirect3DDevice9_DrawIndexedVertices( g_pd3dDevice, PrimitiveType, VertexCount, pIndexData );
	}
#endif
}

void DG_DrawVerticesUP( D3DPRIMITIVETYPE PrimitiveType, UINT VertexCount,
					   CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride )
{
#if 0 //BP_RENDER
	if ( VertexCount != 0) {
		IDirect3DDevice9_DrawVerticesUP( g_pd3dDevice,
										PrimitiveType, VertexCount,
										pVertexStreamZeroData, VertexStreamZeroStride );
	}
#endif
}

void DG_DrawIndexedVerticesUP( D3DPRIMITIVETYPE PrimitiveType,
							  UINT VertexCount,
							  CONST void* pIndexData,
							  CONST void* pVertexStreamZeroData,
							  UINT VertexStreamZeroStride )
{
#if 0 //BP_RENDER
	if ( VertexCount != 0) {
		IDirect3DDevice9_DrawIndexedVerticesUP( g_pd3dDevice,
											   PrimitiveType, VertexCount,
											   pIndexData, pVertexStreamZeroData, VertexStreamZeroStride );
	}
#endif
}

void DG_SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT Stride)
{
#if 0 //BP_RENDER
	IDirect3DDevice9_SetStreamSource( g_pd3dDevice, StreamNumber, pStreamData, Stride );
#endif
}

void DG_SetIndices(IDirect3DIndexBuffer9* pIndexData, UINT BaseVertexIndex)
{
#if 0 //BP_RENDER
	IDirect3DDevice9_SetIndices( g_pd3dDevice, pIndexData, BaseVertexIndex );
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
#ifndef KP_XBOX
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
	
	if( FAILED(	IDirect3DDevice9_CreatePixelShader( g_pd3dDevice, pDef, pHandle ) ) ) {
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
	
	if( FAILED(	IDirect3DDevice9_CreatePixelShader( g_pd3dDevice,  &pDef->Psd, pHandle ) ) ) {
		printf("can't create pixel shader \n");
		HANGUP();
	}
#endif
}
#endif


void	DG_MakePixelShader( DG_PIXELSHADER *shader, void *code )
{
#if 0 //BP_RENDER
#if 0
	if( FAILED(	IDirect3DDevice9_CreatePixelShader( g_pd3dDevice,
												   &((D3DPIXELSHADERDEF_FILE*)code)->Psd,
												   &shader->handle ) ) ) {
		printf("can't create pixel shader \n");
		HANGUP();
	}
#else
	shader->prog_addr = &((D3DPIXELSHADERDEF_FILE*)code)->Psd ;
#endif
#endif
}
void DG_FreePixelShader( DG_PIXELSHADER *shader )
{
#if 0
	if ( FAILED( IDirect3DDevice9_DeletePixelShader( g_pd3dDevice, shader->handle ) ) ){
		printf("can't release vertex shader\n");
		HANGUP();
	}
#endif
	DG_ResetPixelShader();
}


/* シェーダハンドルを初期化する。 */
/* シェーダを消去した場合に呼んでおく必要がある。 */
void DG_ResetShader(void)
{
	DG_ResetVertexShader();
	DG_ResetPixelShader();	
}
#if 0	/* xvshader.cへ移動 */
void DG_ResetVertexShader(void)
{
	/* 適当な固定機能シェーダで初期化 */
	/* なぜか初期化しておかないとマルチテクスチャＫＭＳ描画でおかしくなる */
	IDirect3DDevice9_SetVertexShader( g_pd3dDevice, D3DFVF_XYZ );
	VertexShaderHandle = D3DFVF_XYZ;
}
#endif
void DG_ResetPixelShader(void)
{
	IDirect3DDevice9_SetPixelShader( g_pd3dDevice, 0 );
	PixelShaderHandle = 0;
}

/* 負荷軽減のため、同一ハンドルの時には登録しないように */
void DG_SetVertexShader(DWORD Handle)
{
#if 0 //BP_RENDER
	if (Handle != VertexShaderHandle) {
		IDirect3DDevice9_SetVertexShader( g_pd3dDevice, Handle );
		VertexShaderHandle = Handle;
	}
#endif
}
void DG_SetPixelShader( DG_PIXELSHADER *shader )
{
#if 0 //BP_RENDER
	if ( shader == NULL ){
		IDirect3DDevice9_SetPixelShader( g_pd3dDevice, 0 );
	} else {
		//IDirect3DDevice9_SetPixelShader( g_pd3dDevice, shader->handle );
		IDirect3DDevice9_SetPixelShaderProgram( g_pd3dDevice, shader->prog_addr );
	}
#endif
}

void DG_DeleteVertexShader(DWORD Handle)
{
#if 0 //BP_RENDER
	if ( FAILED( IDirect3DDevice9_DeleteVertexShader( g_pd3dDevice, Handle ) ) ){
		printf("can't release vertex shader\n");
		HANGUP();
	}
#endif
   DG_ResetVertexShader();
}
void DG_DeletePixelShader(DWORD Handle)
{
#if 0 //BP_RENDER
	if ( FAILED( IDirect3DDevice9_DeletePixelShader( g_pd3dDevice, Handle ) ) ){
		printf("can't release vertex shader\n");
		HANGUP();
	}
#endif
   DG_ResetPixelShader();
}

void DG_SetVertexShaderConstant(DWORD Register,
								CONST void* pConstantData,
								DWORD  ConstantCount)
{
#if 0 //BP_RENDER
	IDirect3DDevice9_SetVertexShaderConstant( g_pd3dDevice, Register, pConstantData, ConstantCount );
#endif
}

/* 新設/マトリクスを転置してシェーダに設定 */
void DG_SetVertexShaderConstantMatrix(DWORD Register,
									  CONST MATRIX *Matrix,
									  DWORD nMatrix)
{
#if 0 //BP_RENDER
	int i;
	MATRIX matTmp;

	for (i = nMatrix; i > 0; i--, Matrix++, Register += 4) {
		D3DXMatrixTranspose(&matTmp, Matrix);
		DG_SetVertexShaderConstant(Register, &matTmp, 4);
	}
#endif
}

void DG_SetPixelShaderConstant(DWORD Register,
								CONST void* pConstantData,
								DWORD  ConstantCount)
{
#if 0 //BP_RENDER
	IDirect3DDevice9_SetPixelShaderConstant( g_pd3dDevice, Register, pConstantData, ConstantCount );
#endif
}

#endif
