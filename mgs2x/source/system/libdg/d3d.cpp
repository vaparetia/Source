/*
	d3d.cpp

	Direct3D関係ラッパー関数

	2001/05/08 F.Miyauchi / DirectX オリジナル
	$Id: d3d.cpp,v 1.1.1.3 2002/11/19 11:42:05 Yoshizawa1 Exp $
*/

// 同値設定の回避/コマンド発行数カウント/返値チェック等等のため、
// 頻繁に使用するものはこっちに持ってくる。
// インライン化予定。

#ifndef KP_XBOX
#include <d3dx8.h>
#else
#include <xtl.h>
#endif

#include <stdio.h>
#include "libgv.h"
#include "libdg.h"
#include "private.h"

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

// フルシーンアンチエイリアス設定サンプル関数
void DG_SetFSAA(int type)
{
#ifdef KP_XBOX
	// この方法はXBOXのみ。
	// Windowsでは、Reset(); を呼ぶときに頂点バッファやら何やら
	// 解放する必要がある。
	g_d3dpp.MultiSampleType = (D3DMULTISAMPLE_TYPE)type;
	g_pd3dDevice->Reset(&g_d3dpp);
#else
#endif
}

// パレット関係
void DG_CreatePalette(D3DPALETTESIZE Size, LPDIRECT3DPALETTE8 *ppPalette)
{
	if (FAILED(g_pd3dDevice->CreatePalette(Size, ppPalette))) {
		*ppPalette = NULL;
		ASSERT(0);
	}
}
void DG_SetPalette(DWORD Stage, LPDIRECT3DPALETTE8 pPalette)
{
	g_pd3dDevice->SetPalette(Stage, pPalette);
}

// 可視性テスト関係
static DWORD VisibilityTestID = 0;

// 空きIDを返す
// ※1Syncに 4096個以上の可視性テストをしないことが前提。
DWORD DG_GetVisibilityTestID(void)
{
	VisibilityTestID = (VisibilityTestID + 1) % D3DVISIBILITY_TEST_MAX;
	return VisibilityTestID;
}

void DG_BeginVisibilityTest(void)
{
	if (FAILED(g_pd3dDevice->BeginVisibilityTest())) {
		ASSERT(0);
	}
}

void DG_EndVisibilityTest(DWORD Index)
{
	if (FAILED(g_pd3dDevice->EndVisibilityTest(Index))) {
		ASSERT(0);
	}
}

// 終了するまで待つ
void DG_GetVisibilityTestResultBlock(DWORD Index, UINT* pResult, ULONGLONG* pTimeStamp)
{
	HRESULT hr;
	do {
		hr = g_pd3dDevice->GetVisibilityTestResult(Index, pResult, pTimeStamp);
	} while (hr == D3DERR_TESTINCOMPLETE);
}

// DirectX にフォグカラーを設定する
void DG_SetDxFogColor(void)
{
	CVECTOR color;

	color.r = DG_FogColor.b;
	color.g = DG_FogColor.g;
	color.b = DG_FogColor.r;
	// Xbox
	DG_SetRenderState(D3DRS_FOGCOLOR, *(DWORD *)&color);
}


#define TEXTURE_STAGE_MAX    (4)
static IDirect3DBaseTexture8 *DG_Texture[TEXTURE_STAGE_MAX];

static int DG_DrawPrimitiveCount = 0;
static int DG_DrawIndexedPrimitiveCount = 0;
static DWORD VertexShaderHandle;
static DWORD PixelShaderHandle;

// ゲーム起動時のレンダリングステート/内部変数を初期設定する
void DG_InitRenderState(void)
{
	int i;

	// 内部変数の初期化
	for (i = 0; i < TEXTURE_STAGE_MAX; i++) {
		DG_Texture[i] = NULL;
	}
	DG_DrawPrimitiveCount = 0;
	DG_DrawIndexedPrimitiveCount = 0;

	// カリング無し
	DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// フォグ関係
	DG_SetRenderState(D3DRS_FOGENABLE, TRUE);
	DG_SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
	float fog_start = 0.0f;
	float fog_end = 1.0f;
	DG_SetRenderState(D3DRS_FOGSTART, *(DWORD *)&fog_start);
	DG_SetRenderState(D3DRS_FOGEND, *(DWORD *)&fog_end);
    DG_SetRenderState(D3DRS_SPECULARENABLE, TRUE);
	
	// アルファ関係初期化
	DG_InitAlphaMode();

	// Zバッファ関係
	DG_SetRenderState(D3DRS_ZENABLE, TRUE);
	DG_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);	

	// ライティング無効
	DG_SetRenderState(D3DRS_LIGHTING, FALSE);

	// Z比較式
	// MGSXではZの値が通常のアプリケーションと逆(遠方が0.0f)になる
	g_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
	//g_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	
	// 頂点ブレンディング
	g_pd3dDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);

	// アルファテスト
	// アルファ > 0 の場合のみ描画する
	g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x00000000);

	// テクスチャ関係
	for (i = 0; i < TEXTURE_STAGE_MAX; i++) {
		DG_InitTextureStageState(i);
	}
	// シェーダ関係初期化
	DG_ResetShader();
}

void DG_InitTextureStageState(int Stage)
{
	g_pd3dDevice->SetTextureStageState(Stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	g_pd3dDevice->SetTextureStageState(Stage, D3DTSS_COLORARG2, D3DTA_CURRENT);

	g_pd3dDevice->SetTextureStageState(Stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	g_pd3dDevice->SetTextureStageState(Stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

	if (Stage != 0) { // 初期状態では、テクスチャステージステート0以外は無効
		g_pd3dDevice->SetTextureStageState(Stage, D3DTSS_COLOROP, D3DTOP_DISABLE);
		g_pd3dDevice->SetTextureStageState(Stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	} else {
		// PS2に合わせて、TEXTURE * DIFFUSE * 2 を最終的な色にする
		g_pd3dDevice->SetTextureStageState(Stage, D3DTSS_COLOROP,   D3DTOP_MODULATE2X);
		// アルファは、TEXTURE ... 0x80, DIFFUSE ... 0x80 で テクスチャの色が出るように
		g_pd3dDevice->SetTextureStageState(Stage, D3DTSS_ALPHAOP,   D3DTOP_MODULATE4X);
	}
	// 拡大フィルタ/縮小フィルタ
	g_pd3dDevice->SetTextureStageState(Stage, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	g_pd3dDevice->SetTextureStageState(Stage, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	// CLAMPモード
	g_pd3dDevice->SetTextureStageState(Stage, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	g_pd3dDevice->SetTextureStageState(Stage, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	DG_Texture[Stage] = NULL;
	g_pd3dDevice->SetTexture(Stage, NULL);	
}


// 全般
void DG_Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags,
			  D3DCOLOR Color, float Z, DWORD Stencil)
{
	g_pd3dDevice->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

void DG_BeginScene(void)
{
	g_pd3dDevice->BeginScene();
}

void DG_EndScene(void)
{
	g_pd3dDevice->EndScene();
}

#ifdef KP_XBOX
void DG_Present(CONST RECT* pSourceRect, CONST RECT* pDestRect,
				void *pUnused0, void *pUnused1)
{
	g_pd3dDevice->Present(pSourceRect, pDestRect, pUnused0, pUnused1);
}
#else
void DG_Present(CONST RECT* pSourceRect, CONST RECT* pDestRect,
		   HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{
	g_pd3dDevice->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}
#endif


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

// アルファブレンディング設定値
static unsigned int DG_AlphaBlend;
static unsigned int DG_SrcBlend;
static unsigned int DG_DstBlend;
static unsigned int DG_BlendOp;

// アルファブレンディング値を初期化する
// 起動時に一回呼ばれる
void DG_InitAlphaMode(void)
{
	DG_AlphaBlend = FALSE;
	DG_SrcBlend = D3DBLEND_ONE;
	DG_DstBlend = D3DBLEND_ZERO;
	DG_BlendOp = D3DBLENDOP_ADD;

	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);		
	g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	g_pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
}

// アルファブレンディング値切り替え(変更チェック付き/鬱陶しい…)
static inline void SetAlphaBlend(unsigned int mode)
{
	if (DG_AlphaBlend != mode) {
		DG_AlphaBlend = mode;
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, mode);
	}
}
static inline void SetSrcBlend(unsigned int mode)
{
	if (DG_SrcBlend != mode) {
		DG_SrcBlend = mode;
		g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, mode);
	}
}
static inline void SetDstBlend(unsigned int mode)
{
	if (DG_DstBlend != mode) {
		DG_DstBlend = mode;
		g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, mode);
	}
}
static inline void SetBlendOp(unsigned int mode)
{
	if (DG_BlendOp != mode) {
		DG_BlendOp = mode;
		g_pd3dDevice->SetRenderState(D3DRS_BLENDOP, mode);
	}
}

// 外部公開(不透明オブジェクト描画時にOFFにする)
void DG_AlphaBlendDisable(void)
{
	SetAlphaBlend(FALSE);
	//SetSrcBlend(D3DBLEND_ONE);
	//SetDstBlend(D3DBLEND_ZERO);
	//SetBlendOp(D3DBLENDOP_ADD);
}

// 加算半透明にする
void DG_SetSemitransAdd(void)
{
	SetAlphaBlend(TRUE);
	SetSrcBlend(D3DBLEND_ONE);
	SetDstBlend(D3DBLEND_ONE);
	SetBlendOp(D3DBLENDOP_ADD);
}

// 通常の半透明にする
void DG_SetSemitransNormal(void)
{
	SetAlphaBlend(TRUE);
	SetSrcBlend(D3DBLEND_SRCALPHA);
	SetDstBlend(D3DBLEND_INVSRCALPHA);
	SetBlendOp(D3DBLENDOP_ADD);
}

// 影テクスチャ描画用の半透明モード
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

// 減算半透明にする
void DG_SetSemitransSub(void)
{
	SetAlphaBlend(TRUE);
	SetSrcBlend(D3DBLEND_ONE);
	SetDstBlend(D3DBLEND_ONE);
	SetBlendOp(D3DBLENDOP_REVSUBTRACT);
}

// アルファブレンディングモードを設定する。
// SCE_GS_SET_ALPHAで正直に分岐する。
// ※このままだと死ぬ程重いのでは…
// ※Windows版のHALでは、D3DBLENDOP サポートありません。
void DG_SetAlphaMode(u_long64 type)
{
	// まず0かどうかでαブレンディングのON/OFFを決める
	if (type == 0) {
		SetAlphaBlend(FALSE);
		return;
	}
	SetAlphaBlend(TRUE);
	
	switch (type) {
	case SCE_GS_SET_ALPHA(2,0,0,1,0): // DST - alpha * SRC
		SetSrcBlend(D3DBLEND_SRCALPHA);
		SetDstBlend(D3DBLEND_ONE);
		SetBlendOp(D3DBLENDOP_REVSUBTRACT);
		break;
	case SCE_GS_SET_ALPHA(2,2,2,1,0): // DST
		SetSrcBlend(D3DBLEND_ZERO);
		SetDstBlend(D3DBLEND_ONE);
		SetBlendOp(D3DBLENDOP_ADD);
		break;
	case SCE_GS_SET_ALPHA(1,2,2,2,0): // DST * fix ... 無理
		SetSrcBlend(D3DBLEND_ZERO);
		SetDstBlend(D3DBLEND_ONE);
		SetBlendOp(D3DBLENDOP_ADD);
		break;
	case SCE_GS_SET_ALPHA(1,0,2,2,0): // DST - SRC
		SetSrcBlend(D3DBLEND_ONE);
		SetDstBlend(D3DBLEND_ONE);
		SetBlendOp(D3DBLENDOP_REVSUBTRACT);
		break;
	case SCE_GS_SET_ALPHA(0,1,2,2,0): // SRC - DST
		SetSrcBlend(D3DBLEND_ONE);
		SetDstBlend(D3DBLEND_ONE);
		SetBlendOp(D3DBLENDOP_SUBTRACT);
		break;
	case SCE_GS_SET_ALPHA(0,1,0,1,0): // 通常の半透明
		SetSrcBlend(D3DBLEND_SRCALPHA);
		SetDstBlend(D3DBLEND_INVSRCALPHA);
		SetBlendOp(D3DBLENDOP_ADD);
		break;
	case SCE_GS_SET_ALPHA(0,2,0,1,0): // 加算
		SetSrcBlend(D3DBLEND_SRCALPHA);
		SetDstBlend(D3DBLEND_ONE);
		SetBlendOp(D3DBLENDOP_ADD);
		break;
	default:
		// サポートしていないモードはここに来る
		SetSrcBlend(D3DBLEND_ONE);
		SetDstBlend(D3DBLEND_ZERO);
		SetBlendOp(D3DBLENDOP_ADD);
		printf("SCE_GS_SET_ALPHA : wrong type (%08lx)\n", type);
		break;
	}
}

// レンダリングステート設定
void DG_SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	// TODO: かえって重くなったりしたら止める
	switch (State) {
	case D3DRS_ALPHABLENDENABLE: SetAlphaBlend(Value); break;
	case D3DRS_SRCBLEND:         SetSrcBlend(Value); break;
	case D3DRS_DESTBLEND:        SetDstBlend(Value); break;
	case D3DRS_BLENDOP:          SetBlendOp(Value); break;

	default:
		// TODO: 頻繁に変更されるStateは更新管理する
		g_pd3dDevice->SetRenderState(State, Value);
		break;
	}
}

// 変換行列設定
void DG_SetTransform(D3DTRANSFORMSTATETYPE State, CONST MATRIX *pMatrix)
{
	g_pd3dDevice->SetTransform(State, (CONST D3DMATRIX *)pMatrix);
}

// 頂点バッファ作成
void DG_CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool,
						   IDirect3DVertexBuffer8** ppVertexBuffer)
{
	HRESULT hr;
	hr = g_pd3dDevice->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer);
	if (FAILED(hr)) {
		printf("CreateVertexBuffer failed.\n");
		ASSERT(0);
	}
}

// インデックスバッファ作成
void DG_CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format,
						  D3DPOOL Pool, IDirect3DIndexBuffer8** ppIndexBuffer)
{
	HRESULT hr;
	hr = g_pd3dDevice->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer);
	if (FAILED(hr)) {
		printf("CreateIndexBuffer failed.\n");
		ASSERT(0);
	}
}

void DG_SetRenderTarget(IDirect3DSurface8* pRenderTarget,
						IDirect3DSurface8* pNewZStencil)
{
	HRESULT hr;
	hr = g_pd3dDevice->SetRenderTarget(pRenderTarget, pNewZStencil);
	if (FAILED(hr)) {
		printf("SetRenderTarget failed\n");
		ASSERT(0);
	}
}

void DG_GetRenderTarget(IDirect3DSurface8** ppRenderTarget)
{
	HRESULT hr;
	hr = g_pd3dDevice->GetRenderTarget(ppRenderTarget);
	if (FAILED(hr)) {
		printf("GetRenderTarget failed\n");
		ASSERT(0);
	}
}

void DG_GetDepthStencilSurface(IDirect3DSurface8 **ppZStencilSurface)
{
	HRESULT hr;
	hr = g_pd3dDevice->GetDepthStencilSurface(ppZStencilSurface);
	if (FAILED(hr)) {
		printf("GetDepthStencilSurface failed\n");
		ASSERT(0);
	}
}

// テクスチャ関連
void DG_CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage,
					  D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture8** ppTexture)
{
	HRESULT hr;
	hr = g_pd3dDevice->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture);
	if (FAILED(hr)) {
		printf("CreateTexture (%d, %d) failed\n", Width, Height);
		ASSERT(0);
	}
}

void DG_CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage,
							D3DFORMAT Format, D3DPOOL Pool,
							IDirect3DVolumeTexture8** ppVolumeTexture)
{
	HRESULT hr;
	hr = g_pd3dDevice->CreateVolumeTexture(Width, Height, Depth, Levels, Usage,
										   Format, Pool, ppVolumeTexture);
	if (FAILED(hr)) {
		printf("CreateVolumeTexture (%d, %d, %d) failed\n", Width, Height, Depth);
		ASSERT(0);
	}
}



void DG_SetTexture(DWORD Stage, IDirect3DBaseTexture8 *pTexture)
{
	// 同一テクスチャはセットしない
	if (pTexture != DG_Texture[Stage]) {
		g_pd3dDevice->SetTexture(Stage, pTexture);
		DG_Texture[Stage] = pTexture;
	}
}

void DG_SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	g_pd3dDevice->SetTextureStageState(Stage, Type, Value);
}

// DrawPrimitive/描画関連
void DG_DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	if (PrimitiveCount != 0) {
		g_pd3dDevice->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
	}
}

void DG_DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount,
						CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	g_pd3dDevice->DrawPrimitiveUP(PrimitiveType, PrimitiveCount,
								  pVertexStreamZeroData, VertexStreamZeroStride);
}

void DG_DrawIndexedPrimitive(D3DPRIMITIVETYPE Type, UINT MinIndex, UINT NumVertices,
							 UINT StartIndex, UINT PrimitiveCount)
{
	g_pd3dDevice->DrawIndexedPrimitive(Type, MinIndex, NumVertices,
									   StartIndex, PrimitiveCount);
}

void DG_DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex,
							   UINT NumVertices, UINT PrimitiveCount,
							   CONST void* pIndexData, D3DFORMAT IndexDataFormat,
							   CONST void* pVertexStreamZeroData,
							   UINT VertexStreamZeroStride)
{
	g_pd3dDevice->DrawIndexedPrimitiveUP(
			PrimitiveType, MinIndex, NumVertices, PrimitiveCount,
			pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

void DG_SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer8* pStreamData, UINT Stride)
{
	g_pd3dDevice->SetStreamSource(StreamNumber, pStreamData, Stride);
}

void DG_SetIndices(IDirect3DIndexBuffer8* pIndexData, UINT BaseVertexIndex)
{
	g_pd3dDevice->SetIndices(pIndexData, BaseVertexIndex);
}

// add 2002/02/04 M.Kobayashi
// シェーダをリンクする構造にした
// この部分自動生成にしたほうがいい
typedef struct SHADER_TABLE{
	int id;
	unsigned char* pShader;
} SHADER_TABLE;

extern "C" {
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
}

static SHADER_TABLE table_vsh[] = {
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

//////////////////////


// add 2001/05/10 M.Kobayashi
// ピクセルシェーダのための拡張
// win32 と x でフォーマット,API が違いやがる
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
	
	if( FAILED(	g_pd3dDevice->CreatePixelShader( pDef, pHandle ) ) ) {
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
	
	if( FAILED(	g_pd3dDevice->CreatePixelShader( &pDef->Psd, pHandle ) ) ) {
		printf("can't create pixel shader \n");
		HANGUP();
	}
#endif
}
		
void	DG_CreateVertexShader(int id, const DWORD* pDecl, DWORD* pHandle)
{
	//DWORD* pDef = (DWORD*)GV_GetCache( GV_CacheID(id, 'v') );
	DWORD* pDef = NULL;
	int i;
	SHADER_TABLE* pTab;
	for( i = sizeof( table_vsh ) / sizeof( SHADER_TABLE), pTab = table_vsh; i > 0 ; i--, pTab++ ) {
		if( pTab->id == id ) {
			pDef = (DWORD*)pTab->pShader;
			break;
		}
	}
	ASSERT( pDef != NULL );
	
	if( FAILED(	g_pd3dDevice->CreateVertexShader( pDecl, pDef, pHandle, 0 ) ) ) {
		printf("can't create vertex shader \n");
		HANGUP();
	} 
	// CreateVertexShader の場合、作成したシェーダが有効になる。
	VertexShaderHandle = *pHandle;
}

// シェーダハンドルを初期化する。
// シェーダを消去した場合に呼んでおく必要がある。
void DG_ResetShader(void)
{
	DG_ResetVertexShader();
	DG_ResetPixelShader();	
}
void DG_ResetVertexShader(void)
{
	// 適当な固定機能シェーダで初期化
	g_pd3dDevice->SetVertexShader(D3DFVF_XYZ);
	VertexShaderHandle = D3DFVF_XYZ;
}
void DG_ResetPixelShader(void)
{
	g_pd3dDevice->SetPixelShader(0);
	PixelShaderHandle = 0;
}

// 負荷軽減のため、同一ハンドルの時には登録しないように
void DG_SetVertexShader(DWORD Handle)
{
	if (Handle != VertexShaderHandle) {
		g_pd3dDevice->SetVertexShader(Handle);
		VertexShaderHandle = Handle;
	}
}
void DG_SetPixelShader(DWORD Handle)
{
	if (Handle != PixelShaderHandle) {
		g_pd3dDevice->SetPixelShader(Handle);
		PixelShaderHandle = Handle;
	}
}

void DG_DeleteVertexShader(DWORD Handle)
{
	if (FAILED(g_pd3dDevice->DeleteVertexShader(Handle))) {
		printf("can't release vertex shader\n");
		HANGUP();
	}
	DG_ResetVertexShader();
}
void DG_DeletePixelShader(DWORD Handle)
{
	if (FAILED(g_pd3dDevice->DeletePixelShader(Handle))) {
		printf("can't release vertex shader\n");
		HANGUP();
	}
	DG_ResetPixelShader();
}

void DG_SetVertexShaderConstant(DWORD Register,
								CONST void* pConstantData,
								DWORD  ConstantCount)
{
	g_pd3dDevice->SetVertexShaderConstant(Register, pConstantData, ConstantCount);
}

// 新設/マトリクスを転置してシェーダに設定
void DG_SetVertexShaderConstantMatrix(DWORD Register,
									  CONST MATRIX *Matrix,
									  DWORD nMatrix)
{
	int i;
	MATRIX matTmp;

	for (i = nMatrix; i > 0; i--, Matrix++, Register += 4) {
		D3DXMatrixTranspose(&matTmp, Matrix);
		DG_SetVertexShaderConstant(Register, &matTmp, 4);
	}
}

void DG_SetPixelShaderConstant(DWORD Register,
								CONST void* pConstantData,
								DWORD  ConstantCount)
{
	g_pd3dDevice->SetPixelShaderConstant(Register, pConstantData, ConstantCount);
}
