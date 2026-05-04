/*
 * MGSX main 
 * 
 * 2001/03/21 M.Kobayashi
 * $Id: main.cpp,v 1.1.1.3 2002/11/19 11:42:00 Yoshizawa1 Exp $
 */

#include <stdio.h>

#ifdef KP_XBOX
#include <xtl.h>
#else
#define STRICT
#include <windows.h>
#include <d3dx8.h>

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static HWND CreateMainWindow(HINSTANCE hInstance, DWORD dwWidth, DWORD dwHeight);

//#include "../system/libdg/dgconf.h"

#endif

#include "libgv.h"
//#include "libfs.h"
//#include "libdg.h"
//#include "libgcl.h"
//#include "libmt.h"
//#include "libhzx.h"
//#include "libsd.h"
//#include "g_struct.h"
//#include "game.h"
//#include "libnt.h"

#ifdef KP_XBOX
int main( void )
#else
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow )
#endif
{
#ifndef	KP_XBOX
	// メインウィンドウの作成
	extern HWND DG_hWnd;
	DG_hWnd = CreateMainWindow(hInstance, DISPLAY_WIDTH, DISPLAY_HEIGHT);
#endif
	/*
		Initialize Private System
	*/
	GV_StartDaemon();
///	FS_StartDaemon();

#ifdef KP_XBOX
///	DG_StartDaemon();
#else
///	DG_StartDaemon(hInstance);
#endif

///	HZX_StartDaemon();

///	GCL_Initialize();
///	MT_Initialize();

///	NH_StartDaemon();

	
///	GM_StartDaemon();


//	sd_init();
	
#ifdef KP_XBOX
	for( ;; ){
		GV_ExecActorSystem();
	}
#else
	MSG  msg;	
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT) {
		// メッセージ処理
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			if (GetMessage(&msg, NULL, 0, 0)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			} else {
				break;
			}
		} else {
			GV_ExecActorSystem();
		}
	}

	DG_KillDaemon();
	GV_KillDaemon();
#endif
	
	return 0;
}

#ifndef KP_XBOX
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
