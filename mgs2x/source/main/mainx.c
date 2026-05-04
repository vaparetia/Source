/*
 * MGSX main 
 * 
 * 2001/03/21 M.Kobayashi
 * $Id: mainx.c,v 1.3 2002/11/04 11:43:42 takaki Exp $
 */

#include <stdio.h>

#ifdef KP_XBOX
#include <xtl.h>

#ifdef KP_WINDOWS
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static HWND CreateMainWindow(HINSTANCE hInstance, DWORD dwWidth, DWORD dwHeight);
#endif

#else
#define STRICT
#include <windows.h>
#include <d3dx8.h>

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static HWND CreateMainWindow(HINSTANCE hInstance, DWORD dwWidth, DWORD dwHeight);

//#include "../system/libdg/dgconf.h"

#endif

#include "libgv.h"
#include "libfs.h"
#include "libdg.h"
#include "libgcl.h"
#include "libmt.h"
#include "libhzx.h"
#include "g_struct.h"
#include "game.h"
#include "game.x"
//#include "libnt.h"
#include "mts.h"

//BP_TEMP (fix link errors until there's a better place to put this)
FMATRIX	DG_SSE_CurrentMatrix ;

#ifdef KP_WINDOWS
#define	DISPLAY_WIDTH	(640)
#define	DISPLAY_HEIGHT	(480)
#endif

static 	MSG  msg;

void _program_top( void )
{
}

#ifndef KP_WINDOWS	// (#ifdef KP_XBOX)
int Xmain( void )
#else
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow )
#endif
{
#ifdef KP_WINDOWS	// (#ifndef KP_XBOX)
	// メインウィンドウの作成
	extern HWND DG_hWnd;
	DG_hWnd = CreateMainWindow(hInstance, DISPLAY_WIDTH, DISPLAY_HEIGHT);
#endif
	/*
		Initialize Private System
	*/
	GV_StartDaemon();
	FS_StartDaemon();

#ifndef KP_WINDOWS	// (#ifdef KP_XBOX)
	DG_StartDaemon();
#else
	DG_StartDaemon(hInstance);
#endif

	HZX_StartDaemon();

	GCL_Initialize();
	MT_Initialize();

///	NH_StartDaemon();

	{	// サウンドスレッド初期化
		extern void sd_init( void );
		sd_init();
	}
	
	GM_StartDaemon();

	GV_SetSystemResident();	// ここまでにResidentに確保されたメモリをsystemとして固定
#if 0
	{
		// 浮動小数点系のエラーを受けるようにする。
		int value = 0x27B;
		__asm {
			fnclex;
			fldcw	value
		}
	}
#endif
	
#ifndef KP_WINDOWS	// (#ifdef KP_XBOX)
	for( ;; ){
		GV_ExecActorSystem();
	}
#else
	//MSG  msg;
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

#ifndef KP_WINDOWS		// よく分からないので、とりあえずコメントアウト(By Takaki)
	DG_KillDaemon();
	GV_KillDaemon();
#endif

#endif
	
	return 0;
}

#ifndef KP_WINDOWS
int main( void )
{
#if 0 //BP_TODO
	MTS_BootThread( "MAIN", ( void * )Xmain, 0, NULL, 16 * 1024, NULL );
#else
   Xmain();
#endif
}
#endif

#ifdef KP_WINDOWS	// (#ifndef KP_XBOX)
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
#endif // KP_WINDOWS
