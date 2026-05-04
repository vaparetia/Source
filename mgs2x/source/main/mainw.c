/*
 * MGSX main 
 * 
 * 2001/03/21 M.Kobayashi
 * $Id: mainw.c,v 1.40 2003/02/12 01:08:41 takaki Exp $
 */

#include <stdio.h>

#include <xtl.h>

#include <libgv.h>
#include <libdg.h>

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static HWND CreateMainWindow(HINSTANCE hInstance, DWORD dwWidth, DWORD dwHeight);


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
#include "x2w_app_config.h"
#include "x2w_util.h"

#define	DISPLAY_WIDTH	(640)
#define	DISPLAY_HEIGHT	(480)

static 	MSG  msg;
#include <winCriemu.h>
#include <windecode.h>

void _program_top( void )
{
}


static DWORD	_mtid ;
static HANDLE	_hMainThread ;
static HANDLE	_hMainThreadExitDoneEvent ;
static HANDLE	_hMainThreadKillReqEvent ;
static HANDLE	_hMutex ;
static DWORD WINAPI	MainThread( LPVOID arg );

static void		MainThreadKillSync( void );	// MainThread終了要求＆終了待ち

extern RADAR_InitVertexShader(void) ;		// user/kira/xradar.c
extern RADAR_ReleaseVertexShader(void) ;	// user/kira/xradar.c

static BOOL	CheckMultiExec(void) ;

BOOL	SetWindowsPowerOffSetting(void) ;		// 省電力モード設定変更
BOOL	RestoreWindowsPowerOffSetting(void) ;	// 省電力モード元設定に戻す


#include <signal.h>

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow )
{
	HWND hWnd;
	HRESULT hr;

	/*-- Initialize Private System ------------------------------------------*/

	InitErrorLog() ;
	X2W_InisOSStatus() ;
	/*-----------------------------------------------------------------------*/

	//-------------------------------------------------
	//	ディスクチェックによるコピー対策(簡易すぎで済みません....)

#ifndef DEBUG_MODE
	if( !X2W_InsertDiskCheck() )
	{
		/*-- ダメっぽいので終了 --*/
		X2W_NextProcess() ;
		ExitProcess(0) ;
		return(1) ;
	}
#endif
	//-------------------------------------------------
	//	多重起動禁止
	if( CheckMultiExec() ){ return(0) ; }	// 多重起動っぽいので終了

	// COMの初期化
	hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		DXTRACE_ERR("COMの初期化に失敗", hr);
		return 0;
	}
	//-------------------------------------------------

	//-------------------------------------------------
	//	省電力モード設定変更
	SetWindowsPowerOffSetting() ;

	//	スレッドの初期化
	//	もともとはＸＢＯＸのメインループがスレッドだったので
	//	初期化時にメインループを起動していたが初期化だけに修正
	MTS_BootThread( "MAIN", ( void * )NULL, 0, NULL, 16 * 1024, NULL );
	
	/*-- Initialize Private System (1st) ------------------------------------*/

	FS_StartDaemon();
	X2W_InitAppConfig();	// libfsを使用するのでこのタイミングで実行
	/*-----------------------------------------------------------------------*/

	/*-- Window作成 ---------------------------------------------------------*/

	hWnd = CreateMainWindow(hInstance, DISPLAY_WIDTH, DISPLAY_HEIGHT);
	ASSERT(hWnd) ;
	
//@	SetPriorityClassMacro(HIGH_PRIORITY_CLASS);
	
	/*-----------------------------------------------------------------------*/

	/*
		Initialize Private System
	*/
	GV_StartDaemon();
	DG_StartDaemon(hInstance);

	HZX_StartDaemon();

	GCL_Initialize();
	MT_Initialize();

///	NH_StartDaemon();

	{	// サウンドスレッド初期化
		extern void sd_init( void );
		sd_init();
	}
	
	GM_StartDaemon();

	RADAR_InitVertexShader() ;		// RADAR用のVertexShaderは最初に作成しておく

	GV_SetSystemResident();	// ここまでにResidentに確保されたメモリをsystemとして固定
	
	ErrorLogFlush() ;	// ここまでのLogを出力しておく

	/*-- Window表示開始 -----------------------------------------------------*/

	DG_ShowMainWindow(nCmdShow) ;
	/*-----------------------------------------------------------------------*/

	_hMainThreadExitDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL) ;
	_hMainThreadKillReqEvent  = CreateEvent(NULL, TRUE, FALSE, NULL) ;
	_hMainThread = CreateThread( 0, 0, &MainThread, 0, 0, &_mtid) ;	// MainThread

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
		}
		else
		{
			DG_ResetD3DDeviceMain() ;	// Device Reset処理

#if __DG_DRAW_IN_MAIN_LOOP__
			{
				DWORD	ret ;

				ret = WaitForSingleObject(DG_RequestDrawEventHandle, 0) ;	// 描画要求
				if ( ret != WAIT_TIMEOUT ) {
					ResetEvent(DG_RequestDrawEventHandle) ;	

					DG_DrawFrame() ;
					SetEvent(DG_DrawDoneEventHandle) ;	// 描画終了
				}
			}
#endif
			Sleep(0) ;
		}
	}


	// 終了処理
	MainThreadKillSync() ;

	if( _hMainThreadExitDoneEvent ){ while( !CloseHandle(_hMainThreadExitDoneEvent) ) ; }
	if( _hMainThreadKillReqEvent  ){ while( !CloseHandle(_hMainThreadKillReqEvent) ) ; }

	RestoreWindowsPowerOffSetting() ;	// 省電力モードを元設定に戻す

	// 終了処理その２
	GV_KillActorSystem() ;			// プロセス管理ルーチンの終了処理
	RADAR_ReleaseVertexShader() ;	// user/kira/xradar.cより...
	{
		extern int sd_release (void);
		sd_release ();
	}
	DG_KillDaemon();
	X2W_DestAppConfig();
	GV_KillDaemon();
	X2W_ReleaseOSStatus() ;
	ReleaseErrorLog() ;

	CoUninitialize() ;	// COM終了

	// 多重起動判定用のMutex解放
	if( _hMutex	){ CloseHandle(_hMutex) ; }

	// 次のプロセスの実行開始
	X2W_NextProcess() ;

	// 終了
	ExitProcess(0) ;
	return(0) ;
}

static void		MainThreadKillSync( void )
{
	if( _hMainThread )
	{
		SetEvent(_hMainThreadKillReqEvent) ;				// Main Thread Kill 要求
		WaitForSingleObject(_hMainThreadExitDoneEvent,
								INFINITE) ;					// Main Thread 終了待ち

		CloseHandle(_hMainThread) ;
		_hMainThread = NULL ;
	}
}

#ifndef _WINDOWS
int main( void )
{
	MTS_BootThread( "MAIN", ( void * )Xmain, 0, NULL, 16 * 1024, NULL );
}
#endif

DWORD WINAPI	MainThread( LPVOID arg )
{
	DWORD	ret ;
	HRESULT hr;

	// COMの初期化
	hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		DXTRACE_ERR("COMの初期化に失敗", hr);
		return 0;
	}
	WindowsMpegInit();			// MPEGフィルター初期化
	while(1)
	{
		DG_PerfStart(DGPERF_CTGR_PROCESS_ALL) ;
		GV_ExecActorSystem() ;
		GM_WndApp() ;			// Gameモジュール
		DG_PerfEnd(DGPERF_CTGR_PROCESS_ALL) ;

		ret = WaitForSingleObject(_hMainThreadKillReqEvent, 0) ;
		if( ret != WAIT_TIMEOUT ){ break ; }	// 終了要求判定
	}
	DG_ExeBufferSync(FALSE) ;					// 描画終了待ち

	WindowsMpegRelease();	// MPEGフィルター解放
	CoUninitialize() ;	// COM終了

	PostMessage(DG_hWnd, WM_CLOSE, 0, 0) ;

	SetEvent(_hMainThreadExitDoneEvent) ;		// MainThread終了

	ExitThread(0) ;
	return(0) ;
}

//-----------------------------------------------------------------------------
// メッセージ処理(static)
//-----------------------------------------------------------------------------
volatile	DWORD	_dipswitch = 0x00000000 ;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int	ret ;

	/* SLEEP対処 */
    switch (msg) {
	case WM_POWERBROADCAST:
		switch (wParam) {
#ifdef DEBUG_MODE
		case PBT_APMBATTERYLOW:			// バッテリ電力が低下した
		case PBT_APMOEMEVENT:			// OEM定義のイベントが発生した
		case PBT_APMPOWERSTATUSCHANGE:	// パワー状態が変化した
		case PBT_APMQUERYSUSPENDFAILED:	// 待機要求が拒否された
		case PBT_APMRESUMEAUTOMATIC:	// システムが自動的に復帰しようとしている
		case PBT_APMRESUMECRITICAL:		// 致命的な待機状態からシステムが復帰しようとしている
		case PBT_APMRESUMESUSPEND:		// 待機状態から復帰しようとしている
			printf("WM_POWERBROADCAST[%X]\n", wParam) ;
			break ;
#endif
		case PBT_APMQUERYSUSPEND:		// 待機要求をする
		case PBT_APMSUSPEND:			// システムが待機状態になろうとしている

			printf("PBT_APMQUERYSUSPEND or PBT_APMSUSPEND Requested\n") ;
			return(BROADCAST_QUERY_DENY) ;	// 要求拒否
			break ;
		}
		break ;
	}

	/* 各モジュールのメッセージ処理 */
	ret = GM_WndProc(hWnd, msg, wParam, lParam) ;
	if( ret >=0 ){ return(ret) ; }

	DG_WndProc(hWnd, msg, wParam, lParam) ;

	/* メインモジュールメッセージ処理 */
    switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

#ifdef DEBUG_MODE
	case WM_KEYDOWN:   // キーボードからのキー取得
		switch (wParam) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			_dipswitch ^= 1 << (wParam - '0') ;			// Dipswitchもどき
			printf("DIPSITCH:0x%08X\n", _dipswitch) ;
			break ;

		}
		SoundDebugMainYoshizawa(wParam);
		break ;
#endif
	}
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

//-----------------------------------------------------------------------------
// window生成
//-----------------------------------------------------------------------------
static HWND CreateMainWindow(HINSTANCE hInstance, DWORD dwWidth, DWORD dwHeight)
{
	return DG_CreateMainWindow(hInstance, (WNDPROC)WndProc);
}

//-----------------------------------------------------------------------------
// CheckMultiExec	多重起動検査
//-----------------------------------------------------------------------------
#define	WND_CLASS_NAME	"MGS2"
#define	MULTIEXEC_MUTEX_NAME	"Metal Gear Solid 2 : SUBSTANCE"
#define	WND_TITLE_NAME			"Metal Gear Solid 2 : SUBSTANCE"	// libdg/wd3d.cとあわせる事

static BOOL	CheckMultiExec(void)
{
	BOOL	chk ;

	//-------------------------------------------------
	//	多重起動判定

	chk = FALSE ;
	_hMutex = CreateMutex(NULL, TRUE, MULTIEXEC_MUTEX_NAME) ;
	switch( GetLastError() )
	{
	  case ERROR_ALREADY_EXISTS :	// 既に存在している
		chk = TRUE ;
		break ;
	}

	//-------------------------------------------------

	//	多重起動の場合、既に立ち上がっているアプリを前面に
	if( chk )
	{
		HWND	hWnd ;

		hWnd = FindWindow(NULL, WND_TITLE_NAME) ;	// 既に起動しているアプリ検索
		if( hWnd )
		{
			ShowWindow(hWnd, SW_RESTORE) ;
			SetForegroundWindow(hWnd) ;	// 前面に
		}
	}

	//-------------------------------------------------
	//	検査終了
	return(chk) ;
}

//-----------------------------------------------------------------------------
// SetWindowsPowerOffSetting	省電力モード設定変更
//-----------------------------------------------------------------------------
static DWORD	_PowerOffActiveOrg ;		// 電力停止モード元設定
static DWORD	_LowPowerActiveOrg ;		// 電力停止モード元設定


BOOL	SetWindowsPowerOffSetting(void)
{
	//-------------------------------------------------
	//	現在の設定値を取得
	SystemParametersInfo(SPI_GETPOWEROFFACTIVE, 0, &_PowerOffActiveOrg, 0);
	SystemParametersInfo(SPI_GETLOWPOWERACTIVE, 0, &_LowPowerActiveOrg, 0);

	//-------------------------------------------------
	//	設定
	SystemParametersInfo(SPI_SETPOWEROFFACTIVE, 0, NULL, 0);
	SystemParametersInfo(SPI_SETLOWPOWERACTIVE, 0, NULL, 0);

	return(TRUE) ;
}

//-----------------------------------------------------------------------------
// RestoreWindowsPowerOffSetting	省電力モード元設定に戻す
//-----------------------------------------------------------------------------
BOOL	RestoreWindowsPowerOffSetting(void)
{
	SystemParametersInfo(SPI_SETPOWEROFFACTIVE, _PowerOffActiveOrg, NULL, 0);
	SystemParametersInfo(SPI_SETLOWPOWERACTIVE, _LowPowerActiveOrg, NULL, 0);

	return(TRUE) ;
}
