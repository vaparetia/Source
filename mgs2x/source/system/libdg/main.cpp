//-----------------------------------------------------------------------------
// DirectX8/ゲームメインルーチン
//                                                      F.Miyauchi 2001/02/15
//-----------------------------------------------------------------------------

#ifndef KP_XBOX
// 通常のWindows用ゲーム
#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <basetsd.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <tchar.h>
#include <d3dx8.h>
#else
#include <xtl.h>
#endif // !KP_XBOX

#include "libdg.h" // DirectXゲーム用汎用ヘッダ

//-----------------------------------------------------------------------------
// DirectX関連のグローバル変数実体
//-----------------------------------------------------------------------------

//CDXGame           *g_pGame = NULL;             // DirectXゲームクラス
//LPDIRECT3D8       g_pD3D = NULL;               // D3Dオブジェクト
//LPDIRECT3DDEVICE8 g_pd3dDevice = NULL;         // D3Dデバイス


//-----------------------------------------------------------------------------
//  Windowsアプリケーションのエントリポイント
//-----------------------------------------------------------------------------
#ifdef KP_XBOX
VOID __cdecl main()
#else
INT APIENTRY WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     INT       nCmdShow)
#endif //KP_XBOX
{
#ifdef KP_XBOX	
	DG_StartDaemon();
#else
	DG_StartDaemon(hInstance);	
#endif

#ifdef KP_XBOX
	for (; ;) {
		//MainLoop();
	}
#else
	MSG  msg;
	BOOL bGameEnd = FALSE;

	extern void TEST_Init();
	extern void TEST_MainLoop();
	extern void TEST_Exit();
	
	TEST_Init();
	ZeroMemory(&msg, sizeof(msg));
	
	while (!bGameEnd && msg.message != WM_QUIT) {
		// メッセージ処理
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			if (GetMessage(&msg, NULL, 0, 0)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			} else {
				bGameEnd = TRUE;
			}
		} else {
			//MainLoop();
			extern void DG_Act1(void);
			extern void DG_Act2(void);			
			DG_Act1();
			TEST_MainLoop();
			DG_Act2();
		}
	}
	TEST_Exit();
#endif	

	return EXIT_SUCCESS;

	
#if 0
#ifdef KP_XBOX
	HINSTANCE hInstance = NULL;
#endif //KP_XBOX

	g_pGame = new CDXGame();
	
	// CDXGameクラス初期化。
	// 失敗したらエラー窓を出しておしまい。
	if (FAILED(g_pGame->Init(hInstance, CDXGame::WINDOWED))) {
		//if (FAILED(g_pGame->Init(hInstance, CDXGame::FULLSCREEN))) {
		Xerror("CDXGame Initialize Failed");
#ifndef KP_XBOX
		return EXIT_FAILURE;
#endif
	}

	g_pGame->Exec();    // 実行無限ループ
	delete g_pGame;     // 解放

#ifndef KP_XBOX
	return EXIT_SUCCESS;
#endif
#endif
}
