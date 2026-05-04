/*
	2d_prim.cpp

	2Dプリミティブルーチン

	2001/05/08 F.Miyauchi / DirectX オリジナル
	$Id: 2d_prim.cpp,v 1.1.1.3 2002/11/19 11:42:01 Yoshizawa1 Exp $
*/

// 適当な2D描画ルーチン。D3DLOCK_NOOVERWRITE のテストも兼ねる。

#ifndef KP_XBOX
#include <d3dx8.h>
#else
#include <xtl.h>
#endif

#include <stdio.h>
#include "libgv.h"
#include "libdg.h"
#include "private.h"

// TODO:これ以上Primitiveの種類が増えるとうざいので構造体にしよう…。

#define MAX_BUF  (2)
#define MAX_SPRT (1024)
#define MAX_LINE (1024)

static int nWhich = 0;     // ダブルバッファのうちどちらを使うか
static int nAllSprt = 0;   // スプライトの数
static int nAllLine = 0;   // ラインの数

static LPDIRECT3DVERTEXBUFFER8 SprtVertexBuffer[MAX_BUF];
static LPDIRECT3DVERTEXBUFFER8 LineVertexBuffer[MAX_BUF];

// 2次元描画ルーチンの初期化
void DG_Init2DPrim(void)
{
	HRESULT hr;
	int i;

	for (i = 0; i < MAX_BUF; i++) {
		// vertex buffer 作成
		hr = g_pd3dDevice->CreateVertexBuffer(
				MAX_SPRT * 6 * sizeof(DG_2DVERTEX),
				D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
				D3DFVF_2DVERTEX,
				D3DPOOL_DEFAULT,
				&SprtVertexBuffer[i]);
		if (FAILED(hr)) {
			printf("create vertexbuffer failed\n");
			ASSERT(0);
		}
		hr = g_pd3dDevice->CreateVertexBuffer(
				MAX_LINE * 2 * sizeof(DG_2DCOLORVERTEX),
				D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
				D3DFVF_2DCOLORVERTEX,
				D3DPOOL_DEFAULT,
				&LineVertexBuffer[i]);
		if (FAILED(hr)) {
			printf("create vertexbuffer failed\n");
			ASSERT(0);
		}
	}
}

// ラインを描画する
void DG_Draw2DLine(DG_LINE *pLine, int nLine)
{
	int i;
	DG_2DCOLORVERTEX *pData;
	HRESULT hr;

	if (nLine <= 0) return;
	
	if (nLine + nAllLine > MAX_LINE) {
		printf("warning : 2D LINE buffer over ..\n");
		return;
	}
	hr = LineVertexBuffer[nWhich]->Lock(
			nAllLine * 2 * sizeof(DG_2DCOLORVERTEX), // ここから
			nLine * 2 * sizeof(DG_2DCOLORVERTEX),    // このサイズをロック
			(unsigned char **)&pData,
			D3DLOCK_NOOVERWRITE);               // 上書きしないフラグ
	if (FAILED(hr)) {
		printf("Lock Failed\n");
		ASSERT(0);
	}

	for (i = nLine; i > 0; i--) {
		pData[0].v.x = pLine->x0;
		pData[0].v.y = pLine->y0;
		pData[0].v.z = pLine->z;
		pData[0].v.w = 1.0f;
		pData[0].rgba = pLine->rgba0;

		pData[1].v.x = pLine->x1;
		pData[1].v.y = pLine->y1;
		pData[1].v.z = pLine->z;
		pData[1].v.w = 1.0f;
		pData[1].rgba = pLine->rgba1;

		pData += 2;
		pLine++;
	}
	nAllLine += nLine;
	LineVertexBuffer[nWhich]->Unlock();	
}

// スプライトを描画する
void DG_Draw2DSprt(DG_SPRT *pSprt, int nSprt)
{
	int i;
	DG_2DVERTEX *pData;
	HRESULT hr;

	if (nSprt <= 0) return;
	
	if (nSprt + nAllSprt > MAX_SPRT) {
		printf("warning : 2D SPRT buffer over ..\n");
		return;
	}
	
	hr = SprtVertexBuffer[nWhich]->Lock(
			nAllSprt * 6 * sizeof(DG_2DVERTEX), // ここから
			nSprt * 6 * sizeof(DG_2DVERTEX),    // このサイズをロック
			(unsigned char **)&pData,
			D3DLOCK_NOOVERWRITE);               // 上書きしないフラグ
	if (FAILED(hr)) {
		printf("Lock Failed\n");
		ASSERT(0);
	}

	for (i = nSprt; i > 0; i--) {
		pData[0].v.x = pSprt->x;
		pData[0].v.y = pSprt->y;
		pData[0].v.z = pSprt->z;
		pData[0].v.w = 1.0f;
		pData[0].rgba = pSprt->rgba;
		pData[0].tu0 = pSprt->u0;
		pData[0].tv0 = pSprt->v0;

		pData[1].v.x = pSprt->x + pSprt->w;
		pData[1].v.y = pSprt->y + pSprt->h;
		pData[1].v.z = pSprt->z;
		pData[1].v.w = 1.0f;
		pData[1].rgba = pSprt->rgba;
		pData[1].tu0 = pSprt->u1;
		pData[1].tv0 = pSprt->v1;

		pData[2].v.x = pSprt->x;
		pData[2].v.y = pSprt->y + pSprt->h;
		pData[2].v.z = pSprt->z;
		pData[2].v.w = 1.0f;
		pData[2].rgba = pSprt->rgba;
		pData[2].tu0 = pSprt->u0;
		pData[2].tv0 = pSprt->v1;

		pData[3] = pData[0];
		pData[4] = pData[1];

		pData[5].v.x = pSprt->x + pSprt->w;
		pData[5].v.y = pSprt->y;
		pData[5].v.z = pSprt->z;
		pData[5].v.w = 1.0f;
		pData[5].rgba = pSprt->rgba;
		pData[5].tu0 = pSprt->u1;
		pData[5].tv0 = pSprt->v0;
		
		pData += 6;
		pSprt ++;
	}
	nAllSprt += nSprt;
	SprtVertexBuffer[nWhich]->Unlock();	
}

// テクスチャを設定する
// とりあえず1種類限定!
// 最後に設定されたものが有効になる。
static DG_TEX *pSprtTexture;
void DG_Set2DSprtTex(unsigned int code)
{
	if (code == 0) {
		pSprtTexture = NULL;
		return;
	}
	pSprtTexture = DG_GetTexture(code);
}

// 毎フレームの終わりに描画する
// αブレンディングモード/テクスチャは変更出来ない
void DG_Draw2DPrim(void)
{
	//DG_BeginScene();

	if (nAllSprt > 0 || nAllLine > 0) {
		DG_InitAlphaMode();
		DG_SetRenderState(D3DRS_FOGENABLE, FALSE);
	}

	if (nAllSprt > 0) {
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
		DG_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		DG_SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_POINT);
		DG_SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_POINT);
		DG_SetRenderState(D3DRS_LIGHTING, FALSE);
		if (pSprtTexture == NULL) {
			DG_SetTexture(0, NULL);
		} else {
			DG_SetTexture(0, NULL);
			DG_SetTexture(0, pSprtTexture->tex_trans.ptex);
		}
///		DG_SetSemitransNormal();
//		DG_SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
//		DG_SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
//		DG_SetRenderState( D3DRS_ALPHAREF, 0 );
		DG_AlphaBlendDisable();
		DG_SetStreamSource(0, SprtVertexBuffer[nWhich], sizeof(DG_2DVERTEX));
		DG_SetPixelShader(NULL);
		DG_SetVertexShader(D3DFVF_2DVERTEX);
		DG_DrawPrimitive(D3DPT_TRIANGLELIST, 0, nAllSprt * 2);
		DG_SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		DG_SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	}

	if (nAllLine > 0) {
		DG_SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
		g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		DG_SetTexture(0, NULL);
		DG_SetSemitransNormal();
		DG_SetStreamSource(0, LineVertexBuffer[nWhich], sizeof(DG_2DCOLORVERTEX));
		DG_SetVertexShader(D3DFVF_2DCOLORVERTEX);
		DG_DrawPrimitive(D3DPT_LINELIST, 0, nAllLine);
	}
	//DG_EndScene();

	nWhich = (nWhich + 1) % MAX_BUF; // バッファ切り替え
	nAllSprt = 0;
	nAllLine = 0;
}

// リセット
void DG_Reset2DPrim(void)
{
	nAllSprt = 0;
	nAllLine = 0;
}

// 解放
void DG_Release2DPrim(void)
{
	int i;
	for (i = 0; i < MAX_BUF; i++) {
		SprtVertexBuffer[i]->Release();
		LineVertexBuffer[i]->Release();		
	}
}
