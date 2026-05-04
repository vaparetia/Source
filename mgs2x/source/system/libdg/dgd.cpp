/*
  dgd.cpp
  ライブラリ制御デーモン

  1999/07/07 K.Takabe
  $Id: dgd.cpp,v 1.1.1.3 2002/11/19 11:42:06 Yoshizawa1 Exp $

*/

#ifndef KP_XBOX
#include <d3dx8.h>
#else
#include <xtl.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "libgv.h"
#include "libdg.h"
#include "private.h"
#include "dgconf.h"

//-----------------------------------------------------------------------------

// DirectX8 依存
LPDIRECT3D8       g_pD3D = NULL;
LPDIRECT3DDEVICE8 g_pd3dDevice = NULL;
D3DPRESENT_PARAMETERS g_d3dpp;
LPDIRECT3DTEXTURE8 DG_ShadowTexture = NULL; // 影レンダリング用テクスチャ

#ifdef KP_XBOX
#define AddStr(a,b) (pstrOut += wsprintf( pstrOut, a, b ))
#define MB	(1024*1024)

void PrintGlobalMemoryStatus(void)
{
    MEMORYSTATUS stat;
    CHAR strOut[1024], *pstrOut;

    // Get the memory status.
    GlobalMemoryStatus( &stat );

    // Setup the output string.
    pstrOut = strOut;
	printf("-------- Memory Status --------\n");
    AddStr( "%4d total MB of virtual memory.\n", stat.dwTotalVirtual / MB );
    AddStr( "%4d  free MB of virtual memory.\n", stat.dwAvailVirtual / MB );
    AddStr( "%4d total MB of physical memory.\n", stat.dwTotalPhys / MB );
    AddStr( "%4d  free MB of physical memory.\n", stat.dwAvailPhys / MB );
    AddStr( "%4d total MB of paging file.\n", stat.dwTotalPageFile / MB );
    AddStr( "%4d  free MB of paging file.\n", stat.dwAvailPageFile / MB );
    AddStr( "%4d  percent of memory is in use.\n", stat.dwMemoryLoad );
	printf("-------------------------------\n");		

    // Output the string.
    OutputDebugString(strOut);
}
#endif

//-----------------------------------------------------------------------------

// PS2適当エミュレーション用

// スクラッチパッド(16K)
static u_long64 SCRPAD_WORK[16 * 1024 / sizeof(u_long64)];
unsigned char *SCRPAD_ADDR = (unsigned char *)SCRPAD_WORK;

// アキュムレータ
float DG_ACC;

//-----------------------------------------------------------------------------

/* １回の処理にかかったフレーム数記録カウンタ */
int DG_PassageFrame = 0;

/* 引きつりキャンセルに必要なフレームスキップ数 */
int DG_FrameSkip = 0;
int DG_TickCount = 0;

/* VBlank中かどうか。VBlank中なら１ */
int DG_VSyncStatus = 0;

/* フレーム切替えモード。0なら1frameをオーバーした時に表示中に切替え */
/* １以上なら、そのフレーム数は待って、ひきつったらその次まで待つ */
int DG_FrameCount = 0;

//-----------------------------------------------------------------------------

typedef	struct	{
	GV_ACT actor;
} Work ;

//-----------------------------------------------------------------------------

int DG_FrameStartTime = 0;
int DG_LastActTime = 0;
int DG_LastDrawTime = 0;

//static unsigned int DG_BackColor = D3DCOLOR_XRGB(0, 0x20, 0x60);
//static unsigned int DG_BackColor = D3DCOLOR_XRGB(0x20, 0x20, 0x20);
static unsigned int DG_BackColor = D3DCOLOR_XRGB(0x0, 0x0, 0x0);

// BG クリア色をセット
void DG_SetBgColor(unsigned int color)
{
	DG_BackColor = color;
}

//-----------------------------------------------------------------------------
// タイマー関係
//-----------------------------------------------------------------------------


static LARGE_INTEGER DG_HSyncTimerBase;     // フレーム開始時間/毎フレーム初期化される
static LONGLONG      DG_QpfTickPerSec;
static float         DG_HSyncParam = 0.0f;  // HSync算出用パラメータ

// タイマー関係のパラメータを初期化する
static void DG_InitTimer(void)
{
	int bUsingQPF;
	LARGE_INTEGER qwTicksPerSec;
	bUsingQPF = QueryPerformanceFrequency(&qwTicksPerSec);
	if (!bUsingQPF) { // QueryPerformanceFrequencyが使えない環境
		ASSERT(0);
	}
	DG_QpfTickPerSec = qwTicksPerSec.QuadPart;

	// QueryPerformanceCounter から HSYNC への変換用


   int HSYNC_PER_VSYNC;
   float VSYNC_TIME;
   if ( BP_IsPAL() == true )
   {
      HSYNC_PER_VSYNC = 312;
      VSYNC_TIME = 20.00000000f;
   }
   else
   {
      // 1frame == 262 HSync == 16.66msec
      HSYNC_PER_VSYNC = 262;
      VSYNC_TIME = 16.66666666f;
   }
   DG_HSyncParam = 1000.0f * HSYNC_PER_VSYNC / (VSYNC_TIME * DG_QpfTickPerSec);
}

// フレーム開始時間をセットする(毎フレーム開始時に呼ばれる)
static void DG_ResetTimer(void)
{
	QueryPerformanceCounter(&DG_HSyncTimerBase);
}

// フレーム開始時間からの差をHSync単位(PS2互換)で返す
float DG_GetHSyncTimer(void)
{
	LARGE_INTEGER qwTime;
	// 現在時刻を取得
	QueryPerformanceCounter(&qwTime);
	LONGLONG llDiffTime;
	llDiffTime = qwTime.QuadPart - DG_HSyncTimerBase.QuadPart;
			
	return llDiffTime * DG_HSyncParam;
}

// フレーム開始時間からの差をmsec単位で返す
float DG_GetMsecTimer(void)
{
	LARGE_INTEGER qwTime;
	// 現在時刻を取得
	QueryPerformanceCounter(&qwTime);
	LONGLONG llDiffTime;
	llDiffTime = qwTime.QuadPart - DG_HSyncTimerBase.QuadPart;

	return llDiffTime * 1000.0f / DG_QpfTickPerSec;
}

//-----------------------------------------------------------------------------

// Present→Presentまでの時間をHSync単位で計測する
static int DG_FrameTimer = 0;
int DG_GetFrameTimer(void)
{
	return DG_FrameTimer;
}

// フレーム開始プロセス
static void Act1(Work *work)
{
	// TODO: Present(); と DG_StartFrameの順番 ?
	//DG_ResetTimer(); // フレーム開始時間を設定
	
	// フレームバッファクリア
	// NOTE: カメラに近い方が1.0fになるので、バッファは0.0fでクリアしておく
    DG_Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, DG_BackColor, 0.0f, 0);
	DG_StartFrame();
}

// フレーム終了プロセス
static void Act2(Work *work)
{
	// TODO: Present(); と DG_EndFrameの順番 ?
	DG_EndFrame();
#if 0
	// 仮デバッグメータ
	DG_BeginScene();
	{
		DG_2DCOLORVERTEX v[4];
		//float fX = 320.f * DG_GetHSyncTimer() / HSYNC_PER_VSYNC;
		float fX = (640 - 48) * DG_GetHSyncTimer() / HSYNC_PER_VSYNC;
		//float fX = (640 - 48) * DG_GetMsecTimer() / 16.666666666f;
		DWORD dwCol = 0xff00ff00;
		if (fX > (640 - 48)) {
			dwCol = 0xffffff00;
		}
		if (GV_PadData[0].status & PAD_A) {
			printf("DG_GetMsecTimer = %f\n", DG_GetMsecTimer());
		}
		v[0].v = D3DXVECTOR4(24,      480-28, 1.0f, 1.0f); v[0].rgba = dwCol;
		v[1].v = D3DXVECTOR4(fX + 24, 480-28, 1.0f, 1.0f); v[1].rgba = dwCol;
		v[2].v = D3DXVECTOR4(24,      480,    1.0f, 1.0f); v[2].rgba = dwCol;
		v[3].v = D3DXVECTOR4(fX + 24, 480,    1.0f, 1.0f); v[3].rgba = dwCol;
		DG_SetTexture(0, NULL);
		DG_SetRenderState(D3DRS_LIGHTING, FALSE);
		DG_SetAlphaMode(0);		
		DG_SetVertexShader(D3DFVF_2DCOLORVERTEX);
		DG_DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(DG_2DCOLORVERTEX));
	}
	DG_EndScene();
#endif
	DG_ResetTimer();
	DG_Present(NULL, NULL, NULL, NULL);	     // バックバッファ表示
	DG_FrameTimer = (int)DG_GetHSyncTimer(); // Present()にかかる時間を計測する
}

// VSync割り込み
static void VSyncInCallback(D3DVBLANKDATA* pData)
{
	DG_TickCount++;
}


//-----------------------------------------------------------------------------

//static MTS_EVENTFLAG dg_evflag_body;
//static MTS_EVENTFLAG *dg_evflag;

//#define DG_VSYNC_DONE	0x0001
//#define DG_DMA_DONE		0x0002

static void InitEventFlag(void)
{
//	dg_evflag_body.value = 0;
//	dg_evflag = MTS_CreateEventFlag( &dg_evflag_body );
}

// システムのリセット/ステージ切り替え時にも呼ばれる
void DG_ResetSystem(void)
{
	DG_InitLightSystem();
	DG_InitFrameSystem();

	// XBOX追加
	DG_InitRenderState();        // レンダリングステートの初期化
	DG_ReleaseKmsVertexBuffer(); // KMS用頂点バッファの全解放
	DG_ReleaseTexture();         // テクスチャを解放
	DG_Reset2DPrim();
}

void DG_ResetTexture(void)
{
	//DG_InitTextureCache();
}

// Direct3D関係のリソースを全て解放
void DG_KillDaemon(void)
{
	if (DG_ShadowTexture) {
		DG_ShadowTexture->Release();
	}

	DG_FreeTextureAll();
	DG_ReleasePrim2VertexBuffer();
	DG_ReleaseParticleVertexBuffer();
	DG_ReleasePatchVertexBuffer();
	DG_ReleaseKmsVertexBuffer();
	DG_ReleaseShadowVertexBuffer();
	DG_Release2DPrim();
	
	if (g_pD3D) {
		g_pD3D->Release();
	}
	if (g_pd3dDevice) {
		g_pd3dDevice->Release();
	}
}


//-----------------------------------------------------------------------------

// 常駐頂点シェーダ/ピクセルシェーダの読み込み
// init stage で呼ばれる。
extern "C" void *NewResidentShader(int name, int map)
{
	DG_InitEvmVertexShader();     // evm
	DG_InitObjVertexShader();     // obj
	DG_InitShadowVertexShader();  // shadow
	DG_InitComdlVertexShader();   // comdl(Plugin予定あり)
	DG_InitPatchVertexShader();   // patch(Plugin予定あり)
	
	DG_InitPixelShader();         // pixel shader

	DG_AddPluginComdl();          // test
	DG_AddPluginPatch();          // test

#ifdef DEBUG_MODE
	// この時点(initステージ開始)でのメモリ使用状況を出力
	PrintGlobalMemoryStatus();
#endif
	return NULL;
}


//-----------------------------------------------------------------------------

// libdg の初期化。DirectX8の初期化を含む。

#ifndef KP_XBOX
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static HWND CreateMainWindow(HINSTANCE hInstance, DWORD dwWidth, DWORD dwHeight);
HWND DG_hWnd = NULL;

// windows では Window 作成時にhInstance が必要
void DG_StartDaemon(HINSTANCE hInstance)
#else
void DG_StartDaemon(void)
#endif // !KP_XBOX
{
	static Work Work1, Work2;

	g_pD3D = NULL;
	g_pd3dDevice = NULL;

	// Direct3Dオブジェクトの生成
	g_pD3D = Direct3DCreate8(D3D_SDK_VERSION);
	if (g_pD3D == NULL) {
		printf("Direct3DCreate8 Failed\n");
		exit(EXIT_FAILURE);
	}

	// D3D初期化パラメータ設定
	// 決め打ちのため、対応していないデバイスでは失敗する。
	// 正しくはMicrosoftのサンプルを参照。
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	g_d3dpp.BackBufferCount = 1;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;

	g_d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	g_d3dpp.BackBufferWidth = DISPLAY_WIDTH;
	g_d3dpp.BackBufferHeight = DISPLAY_HEIGHT;
	
#ifndef KP_XBOX
	g_d3dpp.Windowed = TRUE;

	// 現在のディスプレイモードを取得する	
	D3DDISPLAYMODE d3ddm;
	if (FAILED(g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm))) {
		exit(EXIT_FAILURE);
	}
	g_d3dpp.BackBufferFormat = d3ddm.Format;

	//if (FAILED(g_pD3D->CreateDevice(0, D3DDEVTYPE_REF, DG_hWnd,
	if (FAILED(g_pD3D->CreateDevice(0, D3DDEVTYPE_HAL, DG_hWnd,
									D3DCREATE_HARDWARE_VERTEXPROCESSING,
									//D3DCREATE_SOFTWARE_VERTEXPROCESSING,
									&g_d3dpp, &g_pd3dDevice))) {
		printf("CreateDevice (on Windows) Failed\n");
		exit(EXIT_FAILURE);
    }
#else
	// GPU転送に使用するバッファサイズ
	// デフォルト値はそれぞれ、512K, 32K。
	g_pD3D->SetPushBufferSize(512 * 1024 * 6, 32 * 1024);
	
	g_d3dpp.Windowed = FALSE;
	//g_d3dpp.FullScreen_RefreshRateInHz = 60;
	//g_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	// XBOX では hWnd = NULL
    if (FAILED(g_pD3D->CreateDevice(0, D3DDEVTYPE_HAL, NULL,
									D3DCREATE_HARDWARE_VERTEXPROCESSING,
									&g_d3dpp, &g_pd3dDevice))) {
		printf("CreateDevice (on XBOX) Failed\n");
		exit(EXIT_FAILURE);
    }
#endif // !KP_XBOX

	// ビューポート指定
	D3DVIEWPORT8 d3dview;
	d3dview.X = 0; // ターゲットサーフェスの左上座標
	d3dview.Y = 0;	
	d3dview.Width = DISPLAY_WIDTH;
	d3dview.Height = DISPLAY_HEIGHT;
	d3dview.MinZ = 0.0f; // クリップボリュームの最小値
	d3dview.MaxZ = 1.0f; // クリップボリュームの最大値
	g_pd3dDevice->SetViewport(&d3dview);

	// 影レンダリング用テクスチャの生成
	// XBOXのサンプルでは D3DPOOL_MANAGED で作成しているのだが、
	// windows ではD3DPOOL_DEFAULT でなければ怒られてしまう。
	if (FAILED(g_pd3dDevice->CreateTexture(
			256, 256, 1, D3DUSAGE_RENDERTARGET,
			D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &DG_ShadowTexture))) {
		printf("CreateShadowTexture Failed\n");
		exit(EXIT_FAILURE);
	}
	
	DG_Clock = 0;

	DG_InitPrim2VertexBuffer();    // PRIM2用頂点バッファの初期化
	DG_InitKmsVertexBuffer();      // KMS用頂点バッファの初期化
	DG_InitParticleVertexBuffer(); // パーティクル用頂点バッファの初期化
	DG_InitShadowVertexBuffer(); // 影用頂点バッファの初期化(仮)
	DG_InitTimer();              // タイマ初期化
	DG_Init2DPrim();             // 2Dプリミティブ関係初期化

	DG_InitChanlSystem(0);
	DG_ResetSystem();

	//InitEventFlag();

	//AddIntcHandler(INTC_VBLANK_S, VSyncInCallback, - 1);
	//AddIntcHandler2(INTC_VBLANK_E, VSyncOutCallback, - 1, NULL);
	//EnableIntc(INTC_VBLANK_S);
	//EnableIntc(INTC_VBLANK_E);

	GV_SetLoader('k' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitKms);
	GV_SetLoader('l' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitLt2);
	GV_SetLoader('b' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitBmp);
	GV_SetLoader('d' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitBmp); // BMPと同じ関数
	GV_SetLoader('x' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitXti);
	GV_SetLoader('e' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitEvm);
	GV_SetLoader('c' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitCvd);
#ifdef LIBGV_ENABLE	
	GV_SetLoader('t' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitTri);
#endif

	{
		// カメラ位置の適当初期設定
		VECTOR cam = {0.0f,  0.0f, 128.0f, 0.0f};
		VECTOR trg = {0.0f,  0.0f, 0.0f};
      BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
		DG_SetCamera2(DG_Chanls, &cam, &trg, 2.0f);
	}

	// VSync割り込み設定
	g_pd3dDevice->SetVerticalBlankCallback(VSyncInCallback);
	
	// DGライブラリデーモンは2プロセス使用する
	GV_InitActor(GV_ACTOR_DAEMON, &Work1, NULL);  // フレーム開始プロセス
	GV_SetActor(&Work1, Act1, NULL);
	GV_InitActor(GV_ACTOR_DAEMON2, &Work2, NULL); // フレーム終了プロセス
	GV_SetActor(&Work2, Act2, NULL);
}


// メインルーチンに移動
#if 0
//-----------------------------------------------------------------------------
// メッセージ処理(static)
//-----------------------------------------------------------------------------
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:   // キーボードからのキー取得
		switch (wParam) {
		case VK_ESCAPE: // 'ESC'
			PostQuitMessage(0);
			return 0;
		}
		break;
	case WM_LBUTTONDOWN:  // マウス左ボタン
		//LOWORD(lParam); // マウスのx位置
		//HIWORD(lParam); // マウスのy位置
		break;
	case WM_RBUTTONDOWN:  // マウス右ボタン
		break;
	case WM_MOUSEMOVE:    // マウスが動いた
		break;
	}
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// window生成
//-----------------------------------------------------------------------------
static HWND CreateMainWindow(HINSTANCE hInstance, DWORD dwWidth, DWORD dwHeight)
{
	WNDCLASS wndClass = {
		0, WndProc, 0, 0, hInstance,
		NULL, NULL, NULL, NULL, "DirectX8 Game",
	};
	HWND hWnd;	

	RegisterClass(&wndClass);
	// window 作成
	hWnd = CreateWindow("DirectX8 Game", "DirectX8 Game Window",
						WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME
						| WS_MINIMIZEBOX | WS_VISIBLE,
						CW_USEDEFAULT, CW_USEDEFAULT,
						dwWidth, dwHeight,
						NULL, NULL, hInstance, NULL);
	return hWnd;
}
#endif // !KP_XBOX
