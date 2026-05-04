/*
	xdgd.c
	ライブラリ制御デーモン

	2002/02/07 K.Takabe
	$Id: wdgd.c,v 1.58 2003/01/07 05:05:17 takaki Exp $

*/
/*

	void		DG_ResetSystem()

		システムリセット（ステージ切替え時など）

	void		DG_StartDaemon()

		システム起動（ゲームスタート時一度だけ）
*/
#define	__WDGD_C__

#ifndef _XBOX
#include <d3dx8.h>
// BP #include <xgraphics.h>
#else
#include <xtl.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//#include	"mts.h"
//#include	"libgv.cnf"
//#include	"libgv.h"
//#include	"break.h"
//#include	"libdg.h"
//#include	"libdg.cnf"
//#include	"def_dma.h"
#include "libgv.h"
#include "libdg.h"
#include "private.h"
#include "dgconf.h"
#include "libgv.cnf"
#include "shader.h"
#include "wexebuffer.h"
#include "x2w_app_config.h"

//#include	"game.h"

#define	__BACKBUFFER_BY_RENDTEX__			(TRUE)	// RenderingTextureによりBackBuffer実現
#define	__FRAMEBUFFER_USE_DEPTH__			(TRUE)	// FrameBufferもZ-Buffer使用
#define	__FRAMEBUFFER_LOCKABLE__			(!__DG_STOREIMAGE_BY_RENDBUFFER_ONLY__)

extern void DG_SetDisplayOffset( int x, int y, int flag );
extern DG_VIEWPORT DG_CurrentViewport;

static void DG_ReleaseBackBufferManager(void) ;

static UINT DG_SelectRefreshRateInHz(void) ;		// Refresh Rate選択
static UINT DG_SelectPresentationInterval(void) ;	// フレーム切り替え選択

typedef	struct	DG_DispVibrationPend_
{
	float	ofs ;

	float	ang ;
	float	ang_spd ;
} DG_DispVibrationPend ;

static	DG_DispVibrationPend	vib_pend[2] ;

static void DG_InitDispVibration(void) ;
static void DG_ManageDispVibration(void) ;

/*----------------------------------------------------------------*/
/* DirectX8 依存 */
LPDIRECT3D8       g_pD3D = NULL;
LPDIRECT3DDEVICE8 g_pd3dDevice = NULL;
//D3DPRESENT_PARAMETERS g_d3dpp;

D3DFORMAT			DG_ShadowDepthFormat ;				// 影レンダリング用DepthFormat
D3DFORMAT			DG_ShadowFormat ;					// 影レンダリング用テクスチャFormat
DWORD				DG_ShadowWidth ;					// 影レンダリング用テクスチャサイズ
DWORD				DG_ShadowHeight ;
LPDIRECT3DTEXTURE8	DG_ShadowTexture = NULL;			// 影レンダリング用テクスチャ
LPDIRECT3DTEXTURE8	DG_BackBuffer2Texture = NULL;		// BackBuffer2用テクスチャ
LPDIRECT3DSURFACE8	DG_ShadowSurface = NULL ;			// 影用バックバッファサーフェス
LPDIRECT3DSURFACE8	DG_ShadowDepthSurface = NULL ;		// 影用Ｚバッファサーフェス

LPDIRECT3DTEXTURE8	DG_BackBuffer2DepthTexture = NULL;	// BackBuffer2用Zバッファテクスチャ
LPDIRECT3DSURFACE8	DG_BackBuffer2DepthSurface = NULL ;	// BackBuffer2用Zバッファサーフェス

D3DFORMAT			DG_BackBufferFormat ;		// BackBufferFormat
D3DFORMAT			DG_BackBufferDepthFormat ;	// BackBufferDepthFormat
DWORD				DG_BackBufferWidth ;		// BackBufferサイズ
DWORD				DG_BackBufferHeight ;
DWORD				DG_BackBufferRendWidth ;	// 描画に使用する範囲
DWORD				DG_BackBufferRendHeight ;
float				DG_BackBufferWidthAdjustCoef ;	// RendWidth/Width
float				DG_BackBufferHeightAdjustCoef ;	// RendHeight/Height
FVECTOR				DG_BackBufferTexScaleParam[2] ;	// VertexShader用定数CV_TEX0_SCALE
LPDIRECT3DTEXTURE8	DG_BackBufferTexture[3] ;
LPDIRECT3DSURFACE8	DG_BackBufferSurface[3] ;
LPDIRECT3DSURFACE8	DG_BackBufferDepthSurface ;
BOOL				DG_RendToBackBuffer ;			// BackBuffer描画処理中フラグ
DWORD				DG_FrameSkipCount ;				// FrameSkip数(0～)
DWORD				DG_FrameSkipRemainCount ;		// 残りFrameSkip数
DWORD				DG_FramePresentCount ;			// Frame切替カウンタ
BOOL				DG_DeviceLost = FALSE ;					// DeviceLost中

float	DG_DispVibration1 ;
float	DG_DispVibration2 ;

FMATRIX	DG_SSE_CurrentMatrix ;

static FVECTOR			BackBufferTexScaleParamDef[2] =
{
	{
		((float)DRAW_WIDTH/(float)DISPLAY_WIDTH)/16384.0f,
		((float)DRAW_HEIGHT/(float)DISPLAY_HEIGHT)/16384.0f,
		1,1
	},	/* テクスチャスケール */
	{
		((float)(DISPLAY_WIDTH-DRAW_WIDTH)/2.0f)/(float)DISPLAY_WIDTH,
		((float)(DISPLAY_HEIGHT-DRAW_HEIGHT)/2.0f)/(float)DISPLAY_HEIGHT,
		0,0
	}		/* テクスチャオフセット */
} ;

#ifdef _XBOX
#define AddStr(a,b) (pstrOut += wsprintf( pstrOut, a, b ))
#define MB	(1024*1024)

void PrintGlobalMemoryStatus(void)
{
    MEMORYSTATUS stat;

    // Get the memory status.
    GlobalMemoryStatus( &stat );

	printf("-------- Memory Status --------\n");
    printf( "%4d total MB of virtual memory.\n", stat.dwTotalVirtual / MB );
    printf( "%4d  free MB of virtual memory.\n", stat.dwAvailVirtual / MB );
    printf( "%4d total MB of physical memory.\n", stat.dwTotalPhys / MB );
    printf( "%4d  free MB of physical memory.\n", stat.dwAvailPhys / MB );
    printf( "%4d total MB of paging file.\n", stat.dwTotalPageFile / MB );
    printf( "%4d  free MB of paging file.\n", stat.dwAvailPageFile / MB );
    printf( "%4d  percent of memory is in use.\n", stat.dwMemoryLoad );
	printf("-------------------------------\n");		
}
#endif

/*----------------------------------------------------------------*/
/* PS2適当エミュレーション用 */

/* スクラッチパッド(16K) */
#if 0
static u_long SCRPAD_WORK[16 * 1024 / sizeof(u_long)];
unsigned char *SCRPAD_ADDR = (unsigned char *)SCRPAD_WORK;
#endif

/* FPUアキュムレータ */
float DG_ACC;

/*----------------------------------------------------------------*/

/* １回の処理にかかったフレーム数記録カウンタ */
static volatile int		DG_PassageFrame = 0 ;

/* VSync割り込みカウンタ（累計） */
int		DG_TickCount = 0 ;
static int DG_OldTickCount = 0 ;
int		DG_PassageTick = 0 ;		/* １アクト中に発生したVSync割り込み回数 */

#ifdef LIBDG_PERFORMANCE
DG_PERFORMANCE_DATA	DG_PerformanceData ;
#endif

/* VBlank中かどうか。VBlank中なら１ */
int		DG_VSyncStatus = 0;

/* フレーム切替えモード。0なら1frameをオーバーした時に表示中に切替え */
/* １以上なら、そのフレーム数は待って、ひきつったらその次まで待つ */
int		DG_FrameCount = 0;

/*----------------------------------------------------------------*/

extern int DG_Field ;

/*----------------------------------------------------------------*/

/* ＸＢＯＸ追加ワーク */
static volatile int		flip_flag = 1 ;			/* フリップフラグ */
static volatile int		draw_end_flag = 1 ;
static int				draw_mark_count = 0 ;	/* 描画マークカウンタ */

/*----------------------------------------------------------------*/

typedef	struct	{
	GV_ACT		actor ;
} Work ;

/*----------------------------------------------------------------*/


static unsigned int DG_BackColor = D3DCOLOR_XRGB(0x0, 0x0, 0x0);

/* BG クリア色をセット */
void DG_SetBgColor(unsigned int color)
{
	DG_BackColor = color;
}


//-----------------------------------------------------------------------------
// タイマー関係
//-----------------------------------------------------------------------------

#ifdef PAL
static const int HSYNC_PER_VSYNC = 312;
static const float VSYNC_TIME = 20.00000000f;
#else
// 1frame == 262 HSync == 16.66msec
static const int HSYNC_PER_VSYNC = 262;
static const float VSYNC_TIME = 16.66666666f;
#endif

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
	LONGLONG llDiffTime;
	// 現在時刻を取得
	QueryPerformanceCounter(&qwTime);
	llDiffTime = qwTime.QuadPart - DG_HSyncTimerBase.QuadPart;
			
	return llDiffTime * DG_HSyncParam;
}

// フレーム開始時間からの差をmsec単位で返す
float DG_GetMsecTimer(void)
{
	LARGE_INTEGER qwTime;
	LONGLONG llDiffTime;
	// 現在時刻を取得
	QueryPerformanceCounter(&qwTime);
	llDiffTime = qwTime.QuadPart - DG_HSyncTimerBase.QuadPart;

	return llDiffTime * 1000.0f / DG_QpfTickPerSec;
}

/*----------------------------------------------------------------*/
/* Present→Presentまでの時間をHSync単位で計測する */
static int DG_FrameTimer = 0;
int DG_GetFrameTimer(void)
{
	return DG_FrameTimer;
}

/*----------------------------------------------------------------*/

/* VSync割り込み */
typedef	struct	VSyncEmuSys_
{
	LARGE_INTEGER	timer_freq ;
	LARGE_INTEGER	pre_timer ;

	DWORD	cntr ;				// VSync Count
	DWORD	callback_cntr ;		// callback処理を行った回数
} VSyncEmuSys ;

static	VSyncEmuSys	_vse_sys ;

static void VSyncInCallback(void)
{
	DG_PassageFrame++ ;
	DG_TickCount++ ;
	DG_VSyncStatus = 1;	// 帰線期間 // ?
}

static void  DG_InitVSyncCount(void)
{
	memset(&_vse_sys, 0x00, sizeof(VSyncEmuSys)) ;

	/*-- 時間計測初期化 ----------------------------------------*/

	{
		LARGE_INTEGER	timer_freq ;
		LARGE_INTEGER	timer ;

		QueryPerformanceFrequency(&timer_freq) ;
#ifdef PAL
		timer_freq.QuadPart /= 50 ;		// 50Hz
#else
		timer_freq.QuadPart /= 60 ;		// 60Hz
#endif

		QueryPerformanceCounter(&timer) ;

		_vse_sys.timer_freq = timer_freq  ;
		_vse_sys.pre_timer  = timer  ;

	}
	/*----------------------------------------------------------*/
}

void  DG_ResetVSyncCount(void)
{
#if FALSE	// 完全Reset

	LARGE_INTEGER	timer ;

	QueryPerformanceCounter(&timer) ;
	_vse_sys.pre_timer = timer  ;

#else	// 前回の余剰を残す

	_vse_sys.pre_timer.QuadPart += _vse_sys.timer_freq.QuadPart * _vse_sys.cntr ;
#endif
	_vse_sys.cntr = 0 ;
	_vse_sys.callback_cntr = 0 ;
}

DWORD  DG_GetVSyncCount(void)
{
	DWORD	vs_cntr ;
	DWORD	cb_times ;

	/*-- VSyncカウント計算 -------------------------------------*/

	{
		LARGE_INTEGER	tm ;

		QueryPerformanceCounter(&tm) ;
		tm.QuadPart = tm.QuadPart - _vse_sys.pre_timer.QuadPart ;

		vs_cntr = 0 ;
		if( tm.QuadPart >= _vse_sys.timer_freq.QuadPart )
		{
			LARGE_INTEGER	freq ;

			freq = _vse_sys.timer_freq ;
			vs_cntr++ ;

			tm.QuadPart = tm.QuadPart - freq.QuadPart ;

			while( tm.QuadPart > freq.QuadPart )
			{
				vs_cntr++ ;
				tm.QuadPart -= freq.QuadPart ;
			}

			/*-- Callback処理 ---------------------------------*/

			cb_times = vs_cntr - _vse_sys.callback_cntr ;	// コールバック処理回数	
			if( cb_times )
			{
				for(; cb_times; cb_times--){ VSyncInCallback() ; }
				_vse_sys.callback_cntr = vs_cntr ;
			}
			/*--------------------------------------------------*/	
		}
	}
	/*----------------------------------------------------------*/

	/*-- 処理終了 ----------------------------------------------*/	

	_vse_sys.cntr = vs_cntr ;
	return(vs_cntr) ;
	/*----------------------------------------------------------*/	
}

void DG_WaitVSync(void)
{
	DWORD pre ;

	pre = DG_GetVSyncCount() ;
	while( DG_GetVSyncCount() == pre ) ;
}

void  DG_SleepWaitVSync(void)
{
	LARGE_INTEGER	tm ;
	int				sleep_time ;

	/*-- 次のVSyncまでの時間を計算 -----------------------------*/

	QueryPerformanceCounter(&tm) ;
	tm.QuadPart = tm.QuadPart - _vse_sys.pre_timer.QuadPart ;
	if( tm.QuadPart < _vse_sys.timer_freq.QuadPart )
	{
		tm.QuadPart = _vse_sys.timer_freq.QuadPart - tm.QuadPart ;

		sleep_time = (int)(((float)tm.QuadPart / (float)_vse_sys.timer_freq.QuadPart)
					* (60.0f/1000.0f)) ;
		sleep_time-- ;	// ちょっと短めに ;

		if( sleep_time > 0 ){ Sleep(sleep_time) ; }
	}
	/*----------------------------------------------------------*/	
}

DWORD DG_GetNextVSyncTime(void)
{
	LARGE_INTEGER	tm ;
	int				cntr ;
	
	QueryPerformanceCounter(&tm) ;
	tm.QuadPart = tm.QuadPart - _vse_sys.pre_timer.QuadPart ;

	cntr = 16 ;		// Time Out Counter
	while( tm.QuadPart > _vse_sys.timer_freq.QuadPart )
	{
		tm.QuadPart = tm.QuadPart - _vse_sys.timer_freq.QuadPart ;
		cntr-- ;
		if( !cntr ){ break ; }
	}
	if( !cntr ){ return(_vse_sys.timer_freq.LowPart) ; }	// 安全策
	
	tm.QuadPart = _vse_sys.timer_freq.QuadPart - tm.QuadPart ;
	return(tm.LowPart) ;
}

void DG_GetVSyncPastTime(LARGE_INTEGER *time)
{
	LARGE_INTEGER	tm ;

	QueryPerformanceCounter(&tm) ;

	time->QuadPart = tm.QuadPart - _vse_sys.pre_timer.QuadPart ;	// 経過時間
}

void DG_StopVSyncEmulation(void)
{
	printf("DG_StopVSyncEmulation()\n") ;
}

void DG_RestartVSyncEmulation(void)
{
	LARGE_INTEGER	timer ;

	printf("DG_RestartVSyncEmulation()\n") ;

	QueryPerformanceCounter(&timer) ;

	if( _vse_sys.cntr ){ timer.QuadPart -= _vse_sys.timer_freq.QuadPart * _vse_sys.cntr ; }
	_vse_sys.pre_timer = timer  ;
}

#if 0
/* XBOX用Presendコールバック */
static void PresentCallback( D3DSWAPDATA *pData )
{
	flip_flag = 1 ;
}
#endif

/* プッシュバッファコールバック */
#ifndef _WINDOWS
static void PushBufferStartCallback( int n )
{
	GV_PROFILE_DRAW_CLK( 0, n >> 16 );
}
static void PushBufferMarkCallback( int n )
{
	GV_PROFILE_DRAW_CLK( n & 0xFFFF, n >> 16 );
}

static void PushBufferEndCallback( int n )
{
	GV_PROFILE_DRAW_CLK( n & 0xFFFF, n >> 16 );
	GV_PROFILE_DRAW_END_CLK( n & 0xFFFF, n >> 16 );
	draw_end_flag = 1 ;
}
#endif

/* 描画マーク記録（プロファイル用） */
void DG_SetDrawMark( void )
{
//#ifdef DEBUG_MODE
	draw_mark_count += 1 ;
#ifdef _WINDOWS
	GV_PROFILE_DRAW_CLK( draw_mark_count, GV_ProfileClock );
#else
	IDirect3DDevice8_InsertCallback( g_pd3dDevice, D3DCALLBACK_READ, PushBufferMarkCallback
									, draw_mark_count | ( GV_ProfileClock << 16 ) );
#endif
//#endif
}

/* 描画開始マーク */
void DG_SetDrawStartMark( void )
{
	draw_mark_count = 0 ;
	draw_end_flag = 0 ;

#ifdef _WINDOWS
	GV_PROFILE_DRAW_CLK( 0, GV_ProfileClock  );
#else
	IDirect3DDevice8_InsertCallback( g_pd3dDevice, D3DCALLBACK_READ, PushBufferStartCallback
									, 0  | ( GV_ProfileClock << 16 ) );
#endif
}
/* 描画終了マーク */
void DG_SetDrawEndMark( void )
{
	draw_mark_count += 1 ;
#ifdef _WINDOWS
	GV_PROFILE_DRAW_CLK( draw_mark_count, GV_ProfileClock );
	GV_PROFILE_DRAW_END_CLK( draw_mark_count, GV_ProfileClock );
	draw_end_flag = 1 ;
#else
	IDirect3DDevice8_InsertCallback( g_pd3dDevice, D3DCALLBACK_WRITE, PushBufferEndCallback
									, draw_mark_count  | ( GV_ProfileClock << 16 ) );
#endif
}

/* 描画終了待ち */
void DG_DrawSync( void )
{
#ifdef _WINDOWS
	// 素通り
#else
	while ( *(volatile int *)&draw_end_flag == 0 ){
		/* 描画が終了し次第次へ */
	}
	IDirect3DDevice8_BlockUntilIdle( g_pd3dDevice );
#endif
}

/*----------------------------------------------------------------*/

//static MTS_EVENTFLAG dg_evflag_body;
//static MTS_EVENTFLAG *dg_evflag;

//#define DG_VSYNC_DONE	0x0001
//#define DG_DMA_DONE		0x0002

static void InitEventFlag(void)
{
//	dg_evflag_body.value = 0;
//	dg_evflag = MTS_CreateEventFlag( &dg_evflag_body );
}

/*----------------------------------------------------------------*/

int DG_FrameStartTime = 0;
int DG_LastActTime = 0;
int DG_LastDrawTime = 0;

	/*
		フレーム開始処理プロセス
	*/
static	void	Act1( Work *work )
{
	OPERATOR() ;

#if 0
	/* PS2 */
	/* フレーム途中切り替え方式 */
	DG_LastActTime = DIFF_HSYNC_TIMER( GET_HSYNC_TIMER(), DG_FrameStartTime );
	/* MTSを使用したスレッドスリープによるウェイト処理 */
	if( DG_FrameCount == 0 ){
		// フレーム途中切替え
		WaitAllDone();
	} else {
		// VSyncまち
		WaitVSync( DG_FrameCount );
	}

	DG_PassageFrame = 0 ;
	/* 経過時間チェック（割り込みを禁止していないので誤差がでるかも） */
	DG_PassageTick = DG_TickCount - DG_OldTickCount ;
	DG_OldTickCount = DG_TickCount ;
	if ( DG_PassageTick < 0 ) DG_PassageTick = 1 ;

	DG_StartFrame() ;

	DG_FrameStartTime = GET_HSYNC_TIMER();
#else
	/* XBOX */
#ifndef _WINDOWS
	if( DG_FrameCount == 0 ){
		/* フレーム途中切替え */
		while ( DG_PassageFrame == 0 ){
			DG_WaitVSync();
		}
		DG_DrawSync();
	} else {
		/* 描画終了待ち */
		DG_DrawSync();
		/* VSyncまち */
		DG_WaitVSync();
	}
#else

	/* 振動制御 */
	if( DG_CheckDispVibrationEnable() )
	{
		DG_ManageDispVibration() ;
	}

	/* フレーム制御 */
	if( DG_FrameCount )
	{
		while( DG_GetVSyncCount() < DG_FrameCount ){ DG_SleepWaitVSync() ; }
	}

	{
		extern BOOL	DM_DemoActNow ;

		DG_PerfEnd(DGPERF_CTGR_PROCESS_ALL) ;

		if( DM_DemoActNow )
		{
			if( DG_UnDrawFrameCount ){ DG_FrameSkipRemainCount = 0 ; }	// Skipキャンセル

			if( DG_FrameSkipRemainCount > 0 )
			{
				/* Skip中 */
				DG_FrameSkipRemainCount-- ;
				while( DG_GetVSyncCount() < (DG_FrameSkipCount - DG_FrameSkipRemainCount))
				{
					DG_SleepWaitVSync();	// 次のVSyncまでSleepして待つ
				}
			}
			else
			{
				/* VSync待ち */
				while( DG_GetVSyncCount() < (DG_FrameSkipCount + 1))
				{
					DG_SleepWaitVSync();	// 次のVSyncまでSleepして待つ
				}
				DG_ResetVSyncCount() ;	// VSync Counter Reset

				DG_FrameSkipCount = DG_CalcSkipFrame() ;

				if( DG_FrameSkipCount ){ DG_FrameSkipCount-- ; }	// 1Flameは許容
				if( DG_FrameSkipCount > 11 ){ DG_FrameSkipCount = 11 ; }
				DG_FrameSkipRemainCount = DG_FrameSkipCount ;
			}
		}
		else
		{
			if( DG_UnDrawFrameCount ){ DG_FrameSkipRemainCount = 0 ; }	// Skipキャンセル

			if( DG_FrameSkipRemainCount > 0 )
			{
				/* Skip中 */
				DG_FrameSkipRemainCount-- ;
				while( DG_GetVSyncCount() < (DG_FrameSkipCount - DG_FrameSkipRemainCount))
				{
					DG_SleepWaitVSync();	// 次のVSyncまでSleepして待つ
				}
			}
			else
			{
				/* VSync待ち */
				while( DG_GetVSyncCount() < (DG_FrameSkipCount + 1))
				{
					DG_SleepWaitVSync();	// 次のVSyncまでSleepして待つ
				}
				DG_ResetVSyncCount() ;	// VSync Counter Reset

				DG_FrameSkipCount = DG_CalcSkipFrame() ;
				if( DG_FrameSkipCount > 5 ){ DG_FrameSkipCount = 5 ; }
				DG_FrameSkipRemainCount = DG_FrameSkipCount ;
			}
		}

		DG_PerfReset(DGPERF_CTGR_PROCESS_ALL) ;
		DG_PerfStart(DGPERF_CTGR_PROCESS_ALL) ;
	}
#endif

	DG_PassageFrame = 0 ;

	//DG_ResetTimer(); // フレーム開始時間を設定
	
	//draw_mark_count = 0 ;
	DG_StartFrame();
#endif
}

	/*
		フレーム終了処理プロセス
	*/
static	void	Act2( Work *work )
{
#if 0
	/* PS2 */
	OPERATOR() ;
	DG_EndFrame() ;
#else

#ifdef _WINDOWS

	/* XBOX */
	// TODO: Present(); と DG_EndFrameの順番 ?

	DG_EndFrame();

#ifndef _WINDOWS	// Windows版ではPresentはDG_StartFrame()に移動
//	DG_ResetTimer();
	DG_Present(NULL, NULL, NULL, NULL);	     // バックバッファ表示
//	DG_FrameTimer = (int)DG_GetHSyncTimer(); // Present()にかかる時間を計測する
#endif


	GV_PROFILE_ACT_END();

#else
	/* XBOX */
	// TODO: Present(); と DG_EndFrameの順番 ?

	DG_EndFrame();

//	DG_ResetTimer();
	//DG_Present(NULL, NULL, NULL, NULL);	     // バックバッファ表示
//	DG_FrameTimer = (int)DG_GetHSyncTimer(); // Present()にかかる時間を計測する

	GV_PROFILE_ACT_END();
#endif
#endif
}

/*----------------------------------------------------------------*/

void		DG_ResetSystem()
{
	DG_InitLightSystem() ;
	DG_InitFrameSystem() ;
	DG_InitVSyncCount() ;
	DG_CurrentGroupID = 0 ;
	DG_FrameCount = 0 ;

	/* XBOX追加 */
	DG_InitRenderState();        // レンダリングステートの初期化
//	DG_ReleaseKmsVertexBuffer(); // KMS用頂点バッファの全解放
//	DG_ReleaseTexture();         // テクスチャを解放
//	DG_Reset2DPrim();

#ifdef DEBUG_MODE
	{
		int	i ;
		DG_CHANL *cp;
		for ( i = 0 ; i < DG_MAX_CHANLS ; i++ ){
			cp = &DG_Chanls[i] ;

			cp->group_id = 0 ;
			if( i == DG_CHANL_MENU ) continue;
			{
				static char *name[] = {
					"OBJS", "ENVM", "PRIM2", "COMDL", "ShadowW", "Spot"
				};
				DG_OBJ_BUFFER *bp;

				bp = &( cp->obj_queue->objs_buffer );
				for( i = 0; i < 6; i++ ){
					if( bp->n_queue > 0 ){
						int	j ;
						printf( "not dequeue %s %d\n", name[ i ], bp->n_queue );
						if ( i == 0 ){
							for ( j = 0 ; j < bp->n_queue ; j++ ){
								//printf(" dequeue objs use tri_id = %d\n", ((DG_OBJS*)(bp->queue[j]))->tri_id );
								printf(" dequeue objs in %s\n", ((DG_OBJS*)(bp->queue[j]))->fname );
							}
						} else if ( i == 2 ){
							for ( j = 0 ; j < bp->n_queue ; j++ ){
								printf(" dequeue prim2 in %s\n", ((DG_PRIM2*)(bp->queue[j]))->fname );
							}
						}
					} else {
						printf("safe dequeue %s\n", name[ i ] );
					}
					bp++;
				}
				bp = cp->obj_queue->user_buffer ;
				for( i = 0; i < DG_MAX_USR_OBJQUEUE; i++ ){
					if( bp->n_queue > 0 ){
						printf( "not dequeue user object buffer(%d) %d\n", i, bp->n_queue );
					} else {
						printf("safe dequeue user object buffer(%d)\n", i );
					}
					bp++;
				}
			}
		}
	}
#endif
	DG_ResetChanlSystem( 0 );
}

void		DG_ResetTexture( void )
{
	//DG_ReleaseKmsVertexBuffer(); // KMS用頂点バッファの全解放
#ifdef _WINDOWS
	DG_ReleaseDGMdlVertexBufferAll() ;	// 全てのObjectが解放されている事前提
#endif
	//DG_ReleaseTexture();         // テクスチャを解放
	DG_InitTextureCache();
	//DG_ResetLocalVideoMemorySystem();	/*  */
}



/* ---------------------------------------------------------------- */
/* 常駐頂点シェーダ/ピクセルシェーダの読み込み */
/* init stage で呼ばれる。 */
void *NewResidentShader(int name, int map)
{
	DG_InitEvmVertexShader();     // evm
	DG_InitObjVertexShader();     // obj
	DG_InitMultiTexObjVertexShader();     // obj
	DG_InitPrim2VertexShader();
	DG_InitShadowVertexShader();  // shadow
	DG_InitComdlVertexShader();   // comdl(Plugin予定あり)
	DG_InitPatchVertexShader();   // patch(Plugin予定あり)
	
	DG_AddPluginComdl();          // test
	DG_AddPluginPatch();          // test

#ifdef DEBUG_MODE
	// この時点(initステージ開始)でのメモリ使用状況を出力
	PrintGlobalMemoryStatus();
#endif
	return NULL;
}

/* ---------------------------------------------------------------- */
/* 常駐頂点シェーダ/ピクセルシェーダの解放 */

void ReleaseResidentShader(void)
{
	DG_ReleasePatchVertexShader();   // patch(Plugin予定あり)
	DG_ReleaseComdlVertexShader();   // comdl(Plugin予定あり)
	DG_ReleaseShadowVertexShader();  // shadow
	DG_ReleasePrim2VertexShader();
	DG_ReleaseMultiTexObjVertexShader();     // obj
	DG_ReleaseObjVertexShader();     // obj
	DG_ReleaseEvmVertexShader();     // evm
}

/* ---------------------------------------------------------------- */
/* libdg の初期化。DirectX8の初期化を含む。 */
extern double exp( double );
extern double log( double );
static int ConvertGamma( int data, float gamma_inv )
{
	float	a ;
	int		ret ;

	if ( data == 0 ) return ( 0 );
	a = (float)( data & 0xff ) / 255.0f ;
	a = (float)exp( gamma_inv * log( a ) ) ;
	ret = (int)(a * 255.0f) ;
	if ( ret > 255 ) ret = 255 ;
	return ( ret );
}

static	D3DFORMAT	_rend_buffer_texfmt_list_16bit[] =
{
	D3DFMT_A1R5G5B5,	// 16bit(Alpha)
	D3DFMT_A4R4G4B4,
	D3DFMT_A8R3G3B2,

	D3DFMT_A2B10G10R10,	// 32bit(Alpha)
	D3DFMT_A8R8G8B8,

	D3DFMT_R5G6B5,		// 16bit
	D3DFMT_X1R5G5B5,

	D3DFMT_R8G8B8,		// 24bit

	D3DFMT_X8R8G8B8,	// 32bit

	D3DFMT_X4R4G4B4,	// 12bit

	D3DFMT_R3G3B2,		// 8bit

	D3DFMT_FORCE_DWORD,	// 終端
} ;

static	D3DFORMAT	_rend_buffer_texfmt_list_32bit[] =
{
	D3DFMT_A2B10G10R10,	// 32bit(Alpha)
	D3DFMT_A8R8G8B8,

	D3DFMT_A1R5G5B5,	// 16bit(Alpha)
	D3DFMT_A4R4G4B4,
	D3DFMT_A8R3G3B2,

	D3DFMT_X8R8G8B8,	// 32bit

	D3DFMT_R8G8B8,		// 24bit

	D3DFMT_R5G6B5,		// 16bit
	D3DFMT_X1R5G5B5,

	D3DFMT_X4R4G4B4,	// 12bit

	D3DFMT_R3G3B2,		// 8bit

	D3DFMT_FORCE_DWORD,	// 終端
} ;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static HWND CreateMainWindow(HINSTANCE hInstance, DWORD dwWidth, DWORD dwHeight);
HWND DG_hWnd = NULL;
static int	DG_SelectDevice(D3DDEVTYPE device_type) ;			// Device選択
static int	DG_CheckD3DDevice(D3DCAPS8 *cap) ;					// Device使用可能検査
static int	DG_CmpD3DDevice(D3DCAPS8 *cap1, D3DCAPS8 *cap2) ;	// Device比較

static int	DG_CheckVertexShaderCap(D3DCAPS8 *cap) ;			// VertexShader使用可能検査
static int	DG_CheckObjVertexShaderCap(D3DCAPS8 *cap) ;			// [OBJ用]
static int	DG_CheckEvmVertexShaderCap(D3DCAPS8 *cap) ;			// [EVM用]

static int	DG_CheckPixelShaderCap(D3DCAPS8 *cap) ;				// PixelShader使用可能検査

static D3DFORMAT	DG_SelectDepthStencilFormat(D3DCAPS8 *cap,
										D3DFORMAT display,
										D3DFORMAT backbuffer,
										D3DRESOURCETYPE RType,
  										D3DFORMAT def_fmt) ;	// Z-Buffer形式選択

static D3DFORMAT	DG_SelectBackBufferFormat(D3DCAPS8 *cap,
										D3DFORMAT display,
										BOOL is32bit) ;			// BackBuffer形式選択

static D3DFORMAT	DG_SelectRenderTargetFormat(D3DCAPS8 *cap,
										D3DFORMAT display,
										D3DFORMAT *list) ;		// RenderTarget形式選択


// windows では Window 作成時にhInstance が必要
void DG_StartDaemon(HINSTANCE hInstance)
{
	D3DPRESENT_PARAMETERS	g_d3dpp ;

	static Work Work1, Work2;

	ErrorLogPrintf("/*-- Graphics System Daemon Start --*/\n") ;	// Log

	g_pd3dDevice		 = NULL;
	DG_ResetD3DDeviceReq = FALSE ;	// ResetDevice要求OFF

	/* 実行バッファ初期化 */
	DG_InitExeBuffer() ;

	/* Direct3Dオブジェクトの生成 */
	g_pD3D = Direct3DCreate8(D3D_SDK_VERSION);
	if (g_pD3D == NULL) {
		/* 作成できず */
		ErrorLogPrintf("Direct3DCreate8作成失敗\n");

		X2W_ErrorPrintfID(X2W_MB_ERROR, X2WERR_ID_CANT_CREATE_D3D8) ;
		exit(EXIT_FAILURE);
	}

	{
		D3DDISPLAYMODE	dmode;
		int				adp_id ;
		D3DFORMAT		depth_fmt ;
		DWORD			BehaviorFlags ;
		HRESULT			hr ;

		/*-- デバイスの選択 -------------------------------------------------*/

		adp_id = DG_SelectDevice(D3DDEVTYPE_HAL) ;
		if( adp_id < 0 )
		{
			ErrorLogPrintf("HAL Device選択不可能\n");
			adp_id = DG_SelectDevice(D3DDEVTYPE_REF) ;	// SoftWareで実装
			if( adp_id < 0 )
			{
				/* デバイス作成出来ず */
				ErrorLogPrintf("REF Device選択不可能\n");

				X2W_ErrorPrintfID(X2W_MB_ERROR, X2WERR_ID_CANT_CREATE_D3DEVICE8) ;
				exit(EXIT_FAILURE);
			}
		}
		/*-------------------------------------------------------------------*/

		/*-- 現在のディスプレイモードを得る ---------------------------------*/

		if(FAILED(IDirect3D8_GetAdapterDisplayMode(g_pD3D, adp_id, &dmode)))
		{
			/* ディスプレイモードを得ることが出来なかった */
			X2W_ErrorPrintfID(X2W_MB_ERROR, X2WERR_ID_OTHER_FATALERROR) ;
			exit(EXIT_FAILURE);
		}
		/*-------------------------------------------------------------------*/

		/*-- ディスプレイモード検査------------------------------------------*/

		if( DG_WinApp.fullscreen )
		{
			if( X2W_GetAppConfig(X2WAPPCFG_DISP_FORMAT) )
			{
				DG_WinApp.flag |= M_DG_WINAPP_BACKBUFFER_32BIT ;	// 32bit
			}
		}
		else
		{
			switch( dmode.Format )
			{
			  /* 32bit */
			  case D3DFMT_A8R8G8B8 :
			  case D3DFMT_X8R8G8B8 :
			  case D3DFMT_A2B10G10R10 :
				DG_WinApp.flag |= M_DG_WINAPP_BACKBUFFER_32BIT ;	// 32bit
				break ;

			  /* 16bit */
			  case D3DFMT_R5G6B5 :
			  case D3DFMT_X1R5G5B5 :
			  case D3DFMT_A1R5G5B5 :
			  case D3DFMT_A4R4G4B4 :
				break ;

			  /* その他 */
			  default :
				X2W_ErrorPrintfID(X2W_MB_ERROR, X2WERR_ID_DISPLAY_FORMAT) ;
				//WinExec("Rundll32 Shell32.dll,Control_RunDLL Desk.cpl,,3", SW_SHOW);
				exit(EXIT_FAILURE);
				break ;
			}
		}
		/*-------------------------------------------------------------------*/


		/*-- バックサーフェースのフォーマットをコピーして使用する -----------*/

		ZeroMemory(&g_d3dpp,sizeof(g_d3dpp));

		if( !DG_WinApp.fullscreen )
		{
			g_d3dpp.Windowed = TRUE ;	// Window Mode
		}
		else
		{
			g_d3dpp.Windowed = FALSE ;	// Full Screen Mode

			g_d3dpp.FullScreen_RefreshRateInHz		= DG_SelectRefreshRateInHz() ;
			g_d3dpp.FullScreen_PresentationInterval = DG_SelectPresentationInterval() ;
		}

		g_d3dpp.BackBufferWidth  = DG_WinApp.window_width ;
		g_d3dpp.BackBufferHeight = DG_WinApp.window_height ;

		g_d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE ;
		g_d3dpp.SwapEffect		= D3DSWAPEFFECT_DISCARD;	// 垂直同期でフリップ

		if( !DG_WinApp.fullscreen )
		{
			// 現在のDisplayのフォーマット
			g_d3dpp.BackBufferFormat = dmode.Format;
		}
		else
		{
			// 設定値使用
			g_d3dpp.BackBufferFormat = DG_SelectBackBufferFormat(&DG_WinApp.d3d_cap,
											dmode.Format,
											DG_WinApp.flag & M_DG_WINAPP_BACKBUFFER_32BIT);
		}

#if !__BACKBUFFER_BY_RENDTEX__
#if __DG_STOREIMAGE_BY_RENDBUFFER_ONLY__
		g_d3dpp.BackBufferCount = 1;
#else
		g_d3dpp.BackBufferCount = 2;	// StoreImageが参照する可能性があるので余分に取得
#endif

		switch( X2W_GetAppConfig(X2WAPPCFG_DISP_FORMAT) )
		{
			case 0 :	depth_fmt = D3DFMT_D15S1 ;	break ;	// 16bit
			default :	depth_fmt = D3DFMT_D24S8 ;	break ;	// 32bit
		}
		depth_fmt = DG_SelectDepthStencilFormat(&DG_WinApp.d3d_cap,
										g_d3dpp.BackBufferFormat,
										g_d3dpp.BackBufferFormat,
										D3DRTYPE_SURFACE,
  										depth) ;		// 使用可能フォーマット検査
		if( depth_fmt == D3DFMT_FORCE_DWORD )
		{
			ErrorLogPrintf("使用可能なZBuffer形式がありませんでした...無念\n");

			X2W_ErrorPrintfID(X2W_MB_ERROR, X2WERR_ID_OTHER_FATALERROR) ;
			exit(EXIT_FAILURE);
		}
		g_d3dpp.EnableAutoDepthStencil	= TRUE ;					//ZBuffer使用
		g_d3dpp.AutoDepthStencilFormat	= depth_fmt ;
#if __FRAMEBUFFER_LOCKABLE__
		g_d3dpp.Flags					= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER ;
																	// Backbuffer Lock可能
#endif
#else
#if __DG_STOREIMAGE_BY_RENDBUFFER_ONLY__
		g_d3dpp.BackBufferCount			= 1 ;
#else
		g_d3dpp.BackBufferCount			= 2 ;	// StoreImageが参照する可能性があるので余分に取得
#endif
#if !__FRAMEBUFFER_USE_DEPTH__
		g_d3dpp.EnableAutoDepthStencil	= FALSE;	// ZBuffer使用無し
#else
		switch( X2W_GetAppConfig(X2WAPPCFG_DISP_FORMAT) )
		{
			case 0 :	depth_fmt = D3DFMT_D15S1 ;	break ;	// 16bit
			default :	depth_fmt = D3DFMT_D24S8 ;	break ;	// 32bit
		}

		depth_fmt = DG_SelectDepthStencilFormat(&DG_WinApp.d3d_cap,
										g_d3dpp.BackBufferFormat,
										g_d3dpp.BackBufferFormat,
										D3DRTYPE_SURFACE,
  										depth_fmt) ;		// 使用可能フォーマット検査
		if( depth_fmt == D3DFMT_FORCE_DWORD )
		{
			ErrorLogPrintf("使用可能なZBuffer形式がありませんでした...無念\n");
			X2W_ErrorPrintfID(X2W_MB_ERROR, X2WERR_ID_OTHER_FATALERROR) ;
			exit(EXIT_FAILURE);
		}
		g_d3dpp.EnableAutoDepthStencil	= TRUE ;		// ZBuffer使用(2D系が使用)
		g_d3dpp.AutoDepthStencilFormat	= depth_fmt ;
#endif
#if __FRAMEBUFFER_LOCKABLE__
		g_d3dpp.Flags					= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER ;
													// Backbuffer Lock可能
#endif
#endif
		/*-------------------------------------------------------------------*/

		/*-- デバイス作成 ---------------------------------------------------*/

		BehaviorFlags = 0 ;
		BehaviorFlags |= D3DCREATE_MULTITHREADED ;	// 外せれば良いのですが....

		if( DG_WinApp.d3d_cap.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT )
		{
			if( DG_CheckVertexShaderCap(&DG_WinApp.d3d_cap) )
			{
				BehaviorFlags |= D3DCREATE_HARDWARE_VERTEXPROCESSING ;	// Hardware
				if( DG_WinApp.d3d_cap.DevCaps & D3DDEVCAPS_PUREDEVICE )
				{
					BehaviorFlags |= D3DCREATE_PUREDEVICE ;				// 高速化期待...
				}
			}
			else
			{
				BehaviorFlags |= D3DCREATE_MIXED_VERTEXPROCESSING ;	// Hardware & SoftWare
			}
		}
		else
		{
			BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING ;	// Software
		}

		{
			D3DADAPTER_IDENTIFIER8	adp_iden ;

			/*-- Log出力 ----------------------------------------------------*/
			
			IDirect3D8_GetAdapterIdentifier(g_pD3D,
										DG_WinApp.d3d_cap.AdapterOrdinal,
										D3DENUM_NO_WHQL_LEVEL, &adp_iden) ;

			ErrorLogPuts("Video Device:\n") ;
			ErrorLogPrintf("\tDriver         :%s\n", adp_iden.Driver) ;
			ErrorLogPrintf("\tDescription    :%s\n", adp_iden.Description) ;
			ErrorLogPrintf("\tDriver Version :%08X%08X\n",
										adp_iden.DriverVersion.HighPart,
										adp_iden.DriverVersion.LowPart) ;
			ErrorLogPrintf("\tVendorId       :%08X\n", adp_iden.VendorId) ;
			ErrorLogPrintf("\tDeviceId       :%08X\n", adp_iden.DeviceId) ;
			ErrorLogPrintf("\tSubSysId       :%08X\n", adp_iden.SubSysId) ;
			ErrorLogPrintf("\tRevision       :%08X\n", adp_iden.Revision) ;

			ErrorLogPuts("Create Device:\n") ;
			ErrorLogPrintf("\tDeviceType     :%d\n", DG_WinApp.d3d_cap.DeviceType) ;
			ErrorLogPrintf("\tBehaviorFlags  :%08X\n", BehaviorFlags) ;
			ErrorLogPrintf("\tWindowed       :%d\n", g_d3dpp.Windowed) ;
			ErrorLogPrintf("\tFullScreen_RefreshRateInHz :%d\n",
							g_d3dpp.FullScreen_RefreshRateInHz) ;
			ErrorLogPrintf("\tFullScreen_PresentationInterval :%d\n",
							g_d3dpp.FullScreen_PresentationInterval) ;
			ErrorLogPrintf("\tBackBuffer     :(%4d,%4d)\n",
							g_d3dpp.BackBufferWidth, g_d3dpp.BackBufferHeight) ;
			ErrorLogPrintf("\tMultiSampleType:%d\n", g_d3dpp.MultiSampleType) ;
			ErrorLogPrintf("\tSwapEffect     :%d\n", g_d3dpp.SwapEffect) ;
			ErrorLogPrintf("\tBackBufferFormat:%d\n", g_d3dpp.BackBufferFormat) ;
			ErrorLogPrintf("\tBackBufferCount:%d\n", g_d3dpp.BackBufferCount) ;
			ErrorLogPrintf("\tEnableAutoDepthStencil:%d\n", g_d3dpp.EnableAutoDepthStencil) ;
			ErrorLogPrintf("\tFlags          :%08X\n", g_d3dpp.Flags) ;
			/*---------------------------------------------------------------*/
		}

		hr = IDirect3D8_CreateDevice( g_pD3D,
									DG_WinApp.d3d_cap.AdapterOrdinal,
									DG_WinApp.d3d_cap.DeviceType, // HAL? REF?
									DG_hWnd,
									BehaviorFlags,
									&g_d3dpp, &g_pd3dDevice) ;
		if( FAILED(hr) )
		{
			ErrorLogPrintf("CreateDeviceに失敗、再試行します\n");

			/*-- 条件を変えてCreate -----------------------------------------*/

			BehaviorFlags &= ~(D3DCREATE_SOFTWARE_VERTEXPROCESSING
						   | D3DCREATE_MIXED_VERTEXPROCESSING
						   | D3DCREATE_HARDWARE_VERTEXPROCESSING
						   | D3DCREATE_PUREDEVICE) ;

			BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING ;	// Software
			hr = IDirect3D8_CreateDevice( g_pD3D,
										DG_WinApp.d3d_cap.AdapterOrdinal,
										DG_WinApp.d3d_cap.DeviceType, // HAL? REF?
										DG_hWnd,
										BehaviorFlags,
										&g_d3dpp, &g_pd3dDevice) ;

			if( FAILED(hr) )
			{
				ErrorLogPrintf("CreateDeviceの再試行にも失敗、再試行します\n");

				X2W_ErrorPrintfID(X2W_MB_ERROR, X2WERR_ID_CANT_CREATE_D3DEVICE8) ;
				exit(EXIT_FAILURE);
			}
			/*---------------------------------------------------------------*/
		}
		/*-------------------------------------------------------------------*/

		/*-- レンダリングステートの起動時初期化 -----------------------------*/

		DG_PowerOnInitRenderState() ;
		/*-------------------------------------------------------------------*/

		/*-- Front/BackBuffer作成 -------------------------------------------*/

#if	__BACKBUFFER_BY_RENDTEX__
		{
			DWORD		tex_w, tex_h ;
			D3DFORMAT	fmt ;
			D3DFORMAT	*fmt_list ;


			if( (DG_WinApp.flag & M_DG_WINAPP_BACKBUFFER_32BIT) )
			{
				fmt_list = _rend_buffer_texfmt_list_32bit ;	// 32bit
			}
			else
			{
				fmt_list = _rend_buffer_texfmt_list_16bit ;	// 16bit
			}
			fmt = DG_SelectRenderTargetFormat(&DG_WinApp.d3d_cap,
										g_d3dpp.BackBufferFormat,
										fmt_list) ;

			DG_BackBufferFormat = fmt ;

			DG_BackBufferWidth      = X2W_GetAppConfig(X2WAPPCFG_REND_WIDTH) ;
			DG_BackBufferHeight     = X2W_GetAppConfig(X2WAPPCFG_REND_HEIGHT) ;

			if( DG_BackBufferWidth < g_d3dpp.BackBufferWidth )
			{
				DG_BackBufferRendWidth  = DG_BackBufferWidth ;
			}
			else
			{
				DG_BackBufferRendWidth = g_d3dpp.BackBufferWidth ;
			}


			if( DG_BackBufferHeight < g_d3dpp.BackBufferHeight )
			{
				DG_BackBufferRendHeight = DG_BackBufferHeight ;
			}
			else
			{
				DG_BackBufferRendHeight = g_d3dpp.BackBufferHeight ;
			}

			DG_BackBufferWidthAdjustCoef = (float)DG_BackBufferRendWidth
										 / (float)DG_BackBufferWidth ;
			DG_BackBufferHeightAdjustCoef = (float)DG_BackBufferRendHeight
										  / (float)DG_BackBufferHeight ;

			DG_BackBufferTexScaleParam[0] = BackBufferTexScaleParamDef[0] ;
			DG_BackBufferTexScaleParam[1] = BackBufferTexScaleParamDef[1] ;

			DG_BackBufferTexScaleParam[0].x *= DG_BackBufferWidthAdjustCoef ;
			DG_BackBufferTexScaleParam[0].y *= DG_BackBufferHeightAdjustCoef ;
			DG_BackBufferTexScaleParam[1].x *= DG_BackBufferWidthAdjustCoef ;
			DG_BackBufferTexScaleParam[1].y *= DG_BackBufferHeightAdjustCoef ;

			tex_w = DG_BackBufferWidth ;
			tex_h = DG_BackBufferHeight ;

			DG_CreateTexture(tex_w, tex_h, 1, D3DUSAGE_RENDERTARGET,
						fmt,
						D3DPOOL_DEFAULT, &DG_BackBufferTexture[0]) ;
			DG_CreateTexture(tex_w, tex_h, 1, D3DUSAGE_RENDERTARGET,
						fmt,
						D3DPOOL_DEFAULT, &DG_BackBufferTexture[1]) ;
			DG_CreateTexture(tex_w, tex_h, 1, D3DUSAGE_RENDERTARGET,
						fmt,
						D3DPOOL_DEFAULT, &DG_BackBufferTexture[2]) ;

			hr = IDirect3DTexture8_GetSurfaceLevel(DG_BackBufferTexture[0], 0,
											&DG_BackBufferSurface[0]) ;
			ASSERT( !FAILED(hr) ) ;
			hr = IDirect3DTexture8_GetSurfaceLevel(DG_BackBufferTexture[1], 0,
											&DG_BackBufferSurface[1]) ;
			ASSERT( !FAILED(hr) ) ;
			hr = IDirect3DTexture8_GetSurfaceLevel(DG_BackBufferTexture[2], 0,
											&DG_BackBufferSurface[2]) ;
			ASSERT( !FAILED(hr) ) ;

			switch( X2W_GetAppConfig(X2WAPPCFG_DEPTH_FORMAT) )
			{
				case 0 :	depth_fmt = D3DFMT_D15S1 ;	break ;	// 16bit
				default :	depth_fmt = D3DFMT_D24S8 ;	break ;	// 32bit
			}
			depth_fmt = DG_SelectDepthStencilFormat(&DG_WinApp.d3d_cap,
											g_d3dpp.BackBufferFormat,
											fmt,
											D3DRTYPE_SURFACE,
  											depth_fmt) ;		// 使用可能フォーマット検査

			DG_BackBufferDepthFormat = depth_fmt ;	// BackBufferDepthFormat保存
			if( depth_fmt != D3DFMT_FORCE_DWORD )
			{
				hr = DG_CreateDepthStencilSurface(tex_w, tex_h, depth_fmt,
										D3DMULTISAMPLE_NONE,
										&DG_BackBufferDepthSurface) ;
			}


			/*-- Clear -----------------------------------------------------*/

			DG_ClearTextureImage(DG_BackBufferTexture[0]) ;
			DG_ClearTextureImage(DG_BackBufferTexture[1]) ;
			DG_ClearTextureImage(DG_BackBufferTexture[2]) ;
			/*---------------------------------------------------------------*/

			/*-- Log出力 ----------------------------------------------------*/

			ErrorLogPrintf("BackBufferFormat:\n");
			ErrorLogPrintf("\tFormat     :%d\n", DG_BackBufferFormat);
			ErrorLogPrintf("\tSize       :(%d, %d)\n", DG_BackBufferWidth,
												DG_BackBufferHeight);
			ErrorLogPrintf("\tRendSize   :(%d, %d)\n", DG_BackBufferRendWidth,
												DG_BackBufferRendHeight);
			ErrorLogPrintf("\tDepthFormat:%d\n", DG_BackBufferDepthFormat);
			/*---------------------------------------------------------------*/
		}
#endif	// __BACKBUFFER_BY_RENDTEX__

		/*-------------------------------------------------------------------*/

		/*-- BackBuffer管理初期化 -------------------------------------------*/

		DG_InitBackBufferManager() ;
		/*-------------------------------------------------------------------*/

		/*-- アプリケーション状態作成 ---------------------------------------*/

		DG_WinApp.d3dpp = g_d3dpp ;

		DG_WinApp.flag &= ~(M_DG_WINAPP_ZBUFFER | M_DG_WINAPP_STENCIL) ;
		DG_WinApp.flag |= M_DG_WINAPP_ZBUFFER ;		// Z-Buffer使用可

		switch( depth_fmt )
		{
		  case D3DFMT_D15S1 :
		  case D3DFMT_D24X4S4 :
		  case D3DFMT_D24S8 :
			DG_WinApp.flag |= M_DG_WINAPP_STENCIL ;	// Stencil-Buffer使用可能
			break ;
		}

		if( BehaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING )
		{
			/* Software使用 */
			/* VertexBuffer, VertexShaderをSoftwareで使用 */
			DG_WinApp.flag |= M_DG_WINAPP_VBUFF_SOFTWAREPROCESSING
						    | M_DG_WINAPP_USE_VERTEXSHADER
						    | M_DG_WINAPP_VSH_SOFTWAREPROCESSING
						    | M_DG_WINAPP_OBJ_USE_VERTEXSHADER
						    | M_DG_WINAPP_OBJ_VBUFF_SOFTWAREPROCESSING
						    | M_DG_WINAPP_EVM_USE_VERTEXSHADER
						    | M_DG_WINAPP_EVM_VBUFF_SOFTWAREPROCESSING
							| M_DG_WINAPP_SHADOWWT_USE_VERTEXSHADER
							| M_DG_WINAPP_OPTCMF00_USE_VERTEXSHADER
							| M_DG_WINAPP_PATCH_USE_VERTEXSHADER ;
		}
		else if( BehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING )
		{
			/* Hardware/Software使用(Vertex Shaderは使用できない事が前提) */
			ASSERT( !DG_CheckVertexShaderCap(&DG_WinApp.d3d_cap) ) ;

			/* VertexShaderをSoftwareで使用 */
			DG_WinApp.flag |= M_DG_WINAPP_VSH_SOFTWAREPROCESSING ;

			/* DG_OBJ描画VertexShader使用判定  */
			if( DG_CheckObjVertexShaderCap(&DG_WinApp.d3d_cap) )
			{
				/* この場合はSoftware Vertex Shaderになります */
				DG_WinApp.flag |= M_DG_WINAPP_OBJ_USE_VERTEXSHADER
							    | M_DG_WINAPP_OBJ_VBUFF_SOFTWAREPROCESSING ;

				DG_WinApp.flag |= M_DG_WINAPP_SHADOWWT_USE_VERTEXSHADER ;
				DG_WinApp.flag |= M_DG_WINAPP_OPTCMF00_USE_VERTEXSHADER ;
			}
		}
		else
		{
			/* Harware使用(Vertex Shaderは使用できる事が前提) */
			ASSERT( DG_CheckVertexShaderCap(&DG_WinApp.d3d_cap) ) ;

			/* Vertex Shader使用 */
			if( X2W_GetAppConfig(X2WAPPCFG_PRIM_USE_VERTEXSHADER) )
			{
				DG_WinApp.flag |= M_DG_WINAPP_USE_VERTEXSHADER ;
			}

			/* DG_OBJ描画VertexShader使用判定  */
			if( X2W_GetAppConfig(X2WAPPCFG_OBJ_USE_OBJ_MULTITEX)
			  || DG_CheckObjVertexShaderCap(&DG_WinApp.d3d_cap) )
			{
				DG_WinApp.flag |= M_DG_WINAPP_OBJ_USE_VERTEXSHADER ;
			}
			DG_WinApp.flag |= M_DG_WINAPP_SHADOWWT_USE_VERTEXSHADER ;
			DG_WinApp.flag |= M_DG_WINAPP_OPTCMF00_USE_VERTEXSHADER ;
			DG_WinApp.flag |= M_DG_WINAPP_PATCH_USE_VERTEXSHADER ;

			/* DG_EVMOBJ描画VertexShader使用判定  */
			if(  X2W_GetAppConfig(X2WAPPCFG_EVM_USE_OBJ_MULTITEX) )
			{
				DG_WinApp.flag |= M_DG_WINAPP_EVM_USE_VERTEXSHADER ;
			}
		}

		/* 各種Shader用フラグ作成 */
		if( DG_WinApp.flag & M_DG_WINAPP_VBUFF_SOFTWAREPROCESSING )
		{
			DG_WinApp.vbuff_softwareprocessing = TRUE ;
			if( DG_WinApp.flag & M_DG_WINAPP_USE_VERTEXSHADER )
			{
				DG_WinApp.vbuff_usage = D3DUSAGE_SOFTWAREPROCESSING ;
			}
		}

		if( DG_WinApp.flag & M_DG_WINAPP_OBJ_VBUFF_SOFTWAREPROCESSING )
		{
			DG_WinApp.obj_vbuff_softwareprocessing = TRUE ;
			if( DG_WinApp.flag & M_DG_WINAPP_OBJ_USE_VERTEXSHADER )
			{
				DG_WinApp.obj_vbuff_usage = D3DUSAGE_SOFTWAREPROCESSING ;
			}
		}

		if( DG_WinApp.flag & M_DG_WINAPP_EVM_VBUFF_SOFTWAREPROCESSING )
		{
			DG_WinApp.evm_vbuff_softwareprocessing = TRUE ;
			if( DG_WinApp.flag & M_DG_WINAPP_EVM_USE_VERTEXSHADER )
			{
				DG_WinApp.evm_vbuff_usage = D3DUSAGE_SOFTWAREPROCESSING ;
			}
		}

		/* Index使用可能判定 */
		if( DG_WinApp.d3d_cap.MaxVertexIndex > 0xffff )
		{
			DG_WinApp.flag |= M_DG_WINAPP_INDEXPRIM_USABLE ;
		}

		/* Texture同時使用可能数補正 */
		if( DG_WinApp.d3d_cap.MaxSimultaneousTextures
		  && (DG_WinApp.d3d_cap.MaxSimultaneousTextures < 4) )
		{
			DG_WinApp.multi_tex_max = 1 ;	// 4以上でない場合シングルテクスチャとする
		}
		else
		{
			DG_WinApp.multi_tex_max = DG_WinApp.d3d_cap.MaxSimultaneousTextures ;
		}

		/* Pixel Shader 使用可能判定 */
		if( DG_CheckPixelShaderCap(&DG_WinApp.d3d_cap)
		  && X2W_GetAppConfig(X2WAPPCFG_USE_PIXEL_SHADER) )	// 外部設定
		{
			DG_WinApp.flag |= M_DG_WINAPP_PIXELSHADER_USABLE ;
		}

		/* 赤外線スコープ表現選択 */
		if( !(DG_WinApp.flag & M_DG_WINAPP_PIXELSHADER_USABLE) )
		{
			DG_WinApp.flag |= M_DG_WINAPP_IR_OLDVERSION ;
		}

		/* 光源正規化必要性判定 */
#if FALSE
		if(  !(DG_WinApp.flag & M_DG_WINAPP_OBJ_USE_VERTEXSHADER)
		  || !(DG_WinApp.flag & M_DG_WINAPP_EVM_USE_VERTEXSHADER) )
		{
			DG_WinApp.flag |= M_DG_WINAPP_LGTMTX_NORMALIZE ;
		}
#else
		DG_WinApp.flag |= M_DG_WINAPP_LGTMTX_NORMALIZE ;
#endif
		/*-------------------------------------------------------------------*/

		/*-- 外部設定適用 ---------------------------------------------------*/

		/* 画面振動許可 */
		if( X2W_GetAppConfig(X2WAPPCFG_DISP_VIBRATION_ENABLE) )
		{
			DG_WinApp.flag |= M_DG_WINAPP_DISPVIVRATION_ENABLE ;
		}

		/* 投影Shadowの使用 */
		if( X2W_GetAppConfig(X2WAPPCFG_PROJECTION_SHADOW) )
		{
			DG_WinApp.efc_flag |= M_DG_WINAPPEFC_USE_SHADOW ;
		}

		/* Focus系処理使用 */
		if( X2W_GetAppConfig(X2WAPPCFG_FOCUS_EFFECT) )
		{
			DG_WinApp.efc_flag |= M_DG_WINAPPEFC_USE_FOCUS ;
		}

		/* 無線Focus系処理使用 */
		if( X2W_GetAppConfig(X2WAPPCFG_CODEC_FOCUS_EFFECT) )
		{
			DG_WinApp.efc_flag |= M_DG_WINAPPEFC_USE_CODEC_FOCUS ;
		}

		/* Blur系処理使用 */
		if( X2W_GetAppConfig(X2WAPPCFG_BLUR_EFFECT) )
		{
			DG_WinApp.efc_flag |= M_DG_WINAPPEFC_USE_BLUR ;
		}

		/* 煙型Blur系処理使用 */
		if( X2W_GetAppConfig(X2WAPPCFG_SMKBLUR_EFFECT) )
		{
			DG_WinApp.efc_flag |= M_DG_WINAPPEFC_USE_SMKBLUR ;
		}

		/* 煙型Blur系処理使用 */
		if( X2W_GetAppConfig(X2WAPPCFG_MIRROR_OBJ_EFFECT) )
		{
			DG_WinApp.efc_flag |= M_DG_WINAPPEFC_USE_MIRROR ;
		}

		/* クロスフェード効果使用 */
		if( X2W_GetAppConfig(X2WAPPCFG_CROSSFADE_ENABLE) )
		{
			DWORD	lv ;

			DG_WinApp.efc_flag |= M_DG_WINAPPEFC_USE_CROSSFADE ;

			/* クロスフェード解像度設定 */
			lv = X2W_GetAppConfig(X2WAPPCFG_CROSSFADE_TEXLEVEL) ;
			if( lv > 0 )
			{
				DG_WinApp.crossfade_texlevel_w = (WORD)(lv >> 1) ;			// 0,1,1,2,2,...
				DG_WinApp.crossfade_texlevel_h = (WORD)((lv + 1) >> 1) ;	// 1,1,2,2,3,...
			}
		}

		/* BackBuffer描画オフセット */
		DG_WinApp.backbuffer_rend_ofs_w = -0.5f
										+ ((float)X2W_GetAppConfig(X2WAPPCFG_REND_OFS_W)
										/ (float)0x20000) ;
		DG_WinApp.backbuffer_rend_ofs_h =  -0.5f
										+ ((float)X2W_GetAppConfig(X2WAPPCFG_REND_OFS_H)
										/ (float)0x20000) ;

		/* 水中エフェクトレベル */
		DG_WinApp.inwater_efclevel = X2W_GetAppConfig(X2WAPPCFG_INWATER_EFFECT_LEVEL) ;

		/* 雨エフェクトレベル */
		DG_WinApp.rain_efclevel = X2W_GetAppConfig(X2WAPPCFG_RAIN_EFFECT_LEVEL) ;

		/* VR敵兵体破壊エフェクト許可(TEST) */
		if( X2W_GetAppConfig(X2WAPPCFG_VR_BREAKBODY_ENABLE) )
		{
			DG_WinApp.efc_flag |= M_DG_WINAPPEFC_USE_VRBREAKBODY ;
		}
		/*-------------------------------------------------------------------*/

		/*-- Clear用フラグマスク作成 ----------------------------------------*/
#if !__BACKBUFFER_BY_RENDTEX__

		DG_WinApp.clear_flag_mask = D3DCLEAR_TARGET ;
		if( DG_WinApp.flag & M_DG_WINAPP_ZBUFFER ){ DG_WinApp.clear_flag_mask |= D3DCLEAR_ZBUFFER ; }
		if( DG_WinApp.flag & M_DG_WINAPP_STENCIL ){ DG_WinApp.clear_flag_mask |= D3DCLEAR_STENCIL ; }
#endif
		/*-------------------------------------------------------------------*/

		/*-- RenderTarget情報直接設定 --------------------------------------*/

#if !__BACKBUFFER_BY_RENDTEX__
		DG_SetRenderTargetWH(DG_WinApp.window_width, DG_WinApp.window_height) ;
#endif
		/*------------------------------------------------------------------*/

		/*-- 作業用BackBuffer用テクスチャの取得 -----------------------------*/

#if !__BACKBUFFER_BY_RENDTEX__
		DG_CreateTexture( 640, 480, 1, 0, DG_WinApp.d3dpp.BackBufferFormat,
					D3DPOOL_MANAGED, &DG_BackBuffer2Texture) ;
		DG_ClearTextureImage(DG_BackBuffer2Texture) ;
#else
		DG_BackBuffer2Texture = DG_BackBufferTexture[2] ;
#endif
		/*-------------------------------------------------------------------*/

		/*-- 影用テクスチャ作成 ---------------------------------------------*/

		if( DG_WinApp.efc_flag & M_DG_WINAPPEFC_USE_SHADOW )
		{
			DWORD		tex_w, tex_h ;
			D3DFORMAT	fmt ;
			D3DFORMAT	*fmt_list ;

			tex_w = X2W_GetAppConfig(X2WAPPCFG_SHADOWTEX_WIDTH) ;
			tex_h = X2W_GetAppConfig(X2WAPPCFG_SHADOWTEX_HEIGHT) ;

			/*-- 影描画用テクスチャ作成 -------------------------------------*/

			switch( X2W_GetAppConfig(X2WAPPCFG_DISP_FORMAT) )
			{
			  case 0 :	// 16bit
				fmt_list = _rend_buffer_texfmt_list_16bit ;
				break ;

			  default :	// 32bit
				fmt_list = _rend_buffer_texfmt_list_32bit ;
				break ;
			}
			fmt = DG_SelectRenderTargetFormat(&DG_WinApp.d3d_cap,
										g_d3dpp.BackBufferFormat,
										fmt_list) ;

			DG_CreateTexture( tex_w, tex_h, 1, D3DUSAGE_RENDERTARGET,
									fmt, D3DPOOL_DEFAULT, &DG_ShadowTexture) ;


			DG_ShadowFormat = fmt ;		// 影レンダリング用テクスチャFormat
			DG_ShadowWidth  = tex_w ;	// 影レンダリング用テクスチャサイズ
			DG_ShadowHeight = tex_h ;
			/*---------------------------------------------------------------*/

			/*-- 影用サーフェイス取得 ---------------------------------------*/

			IDirect3DTexture8_GetSurfaceLevel(DG_ShadowTexture, 0, &DG_ShadowSurface) ;
			fmt = DG_SelectDepthStencilFormat(&DG_WinApp.d3d_cap,
											g_d3dpp.BackBufferFormat,
											D3DFMT_A8R8G8B8,
											D3DRTYPE_SURFACE,
  											D3DFMT_D16) ;		// 使用可能フォーマット検査

			DG_ShadowDepthFormat = fmt ;
			if( fmt != D3DFMT_FORCE_DWORD )
			{
				hr = DG_CreateDepthStencilSurface(tex_w, tex_h, fmt,
										D3DMULTISAMPLE_NONE,
										&DG_ShadowDepthSurface) ;
			}
			/*---------------------------------------------------------------*/

			/*-- Log出力 ----------------------------------------------------*/

			ErrorLogPrintf("Shadow:\n");
			ErrorLogPrintf("\tFormat     :%d\n", DG_ShadowFormat);
			ErrorLogPrintf("\tSize       :(%d, %d)\n",
									DG_ShadowWidth, DG_ShadowHeight) ;
			ErrorLogPrintf("\tDepthFormat:%d\n", DG_ShadowDepthFormat);
			/*---------------------------------------------------------------*/
		}
		/*-------------------------------------------------------------------*/

		DG_SystemTexture[0].tex_trans.ptex = DG_BackBufferTexture[0] ;
		DG_SystemTexture[1].tex_trans.ptex = DG_BackBufferTexture[1] ;
		DG_SystemTexture[2].tex_trans.ptex = DG_BackBufferTexture[2] ;
		DG_SystemTexture[3].tex_trans.ptex = DG_BackBufferTexture[2] ;
	}

#if 0
	{/* ガンマ補正実験 */
		D3DGAMMARAMP	Ramp ;
		int				i ;

		for ( i = 0 ; i < 256 ; i++ ){
			Ramp.red[ i ] = ConvertGamma( i, 1.0f/1.3f ) ;
			Ramp.green[ i ] = ConvertGamma( i, 1.0f/1.3f ) ;
			Ramp.blue[ i ] = ConvertGamma( i, 1.0f/1.3f ) ;
		}
		IDirect3DDevice8_SetGammaRamp( g_pd3dDevice, D3DSGR_CALIBRATE, &Ramp );
	}
#endif

	{/* ビューポート指定 */
		DG_VIEWPORT	viewport;
		viewport.lx     = 0.0f ;
		viewport.ly     = 0.0f ;
		viewport.width  = 1.0f ;
		viewport.height = 1.0f ;
		viewport.min_z  = 0.0f ;
		viewport.max_z  = 1.0f ;
		DG_SetViewport( &viewport );
		DG_CurrentViewport   = viewport ;

		/*-- 描画用イベント作成 ---------------------------------------------*/
#if __DG_DRAW_IN_MAIN_LOOP__ || __DG_DRAW_IN_DRAWTHREAD__ || __DG_DRAW_IN_EXEBUFFER__
		DG_RequestDrawEventHandle = CreateEvent(NULL, TRUE, FALSE, NULL) ;
		DG_DrawDoneEventHandle    = CreateEvent(NULL, TRUE, FALSE, NULL) ;
#endif
		/*-------------------------------------------------------------------*/
	}
	DG_Clock = 0 ;

	/* モデル関係初期化 */
	DG_InitDGMdlVertexBufferManager() ;

	/* 使用可能TextureFormat検査(表示状態が変わったら再度実行すること) */
	DG_CheckUsableTextureFormats() ;

	/* Vertex Shader 情報の初期化 */
	DG_InitVertexShader();

	/* Pixel Shader 情報の初期化 */
	DG_InitPixelShader();

	/* アルファブレンディングの設定を初期化する */
	DG_InitAlphaState();

	DG_InitKmsVertexBuffer();	/* KMS用インデックスバッファ初期化（現在はあまり必要ない） */
	DG_InitTimer();				/* タイマ初期化 */
	/* メモリ関連初期化 */
	DG_InitMemorySystem( DG_STATIC_VERTBUF_SIZE, DG_DYNAMIC_VERTBUF_SIZE
						, DG_PUSHBUFFER_SIZE );

	/* スタティックプッシュバッファ関連初期化 */
	DG_DmaReset();


	DG_SetDisplayOffset( 0, 0, 0 );
	DG_InitChanlSystem(0);
	DG_ResetSystem();

	//InitEventFlag();

	//AddIntcHandler(INTC_VBLANK_S, VSyncInCallback, - 1);
	//AddIntcHandler2(INTC_VBLANK_E, VSyncOutCallback, - 1, NULL);
	//EnableIntc(INTC_VBLANK_S);
	//EnableIntc(INTC_VBLANK_E);

	/*
		ロード初期化ルーチンの登録
	*/
	GV_SetLoader('k' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitKms);
	GV_SetLoader('l' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitLt2);
	//GV_SetLoader('b' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitBmp);
	//GV_SetLoader('d' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitBmp); // BMPと同じ関数
	//GV_SetLoader('x' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitXti);
	GV_SetLoader('x' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitTri);
	GV_SetLoader('e' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitEvm);
	GV_SetLoader('c' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitCvd);
	//GV_SetLoader('t' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitTri);
	GV_SetLoader( 'z' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitZar ) ;

	{
		/* カメラ位置の適当初期設定 */
		FVECTOR cam = {0.0f,  0.0f, 128.0f, 0.0f};
		FVECTOR trg = {0.0f,  0.0f, 0.0f};
      BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
		DG_SetCamera2(DG_Chanls, &cam, &trg, 2.0f);
	}

	/*
		ＤＧライブラリデーモンは、２プロセス使用
	*/
	GV_InitActor(GV_ACTOR_DAEMON, &Work1, NULL);  // フレーム開始プロセス
	GV_SetActor(&Work1, Act1, NULL);
	GV_InitActor(GV_ACTOR_DAEMON2, &Work2, NULL); // フレーム終了プロセス
	GV_SetActor(&Work2, Act2, NULL);

	NewResidentShader( 0, 0 );

	/* パフォーマンス検査初期化 */
	DG_PerfInit() ;

	/* 線形テクスチャ生成初期化 */
	DG_MakeLinerTextureInit() ;
}


/* 使用デバイス選択 */
static int	DG_SelectDevice(D3DDEVTYPE device_type)
{
	UINT		adp_num ;
	int			sel ;
	UINT		i ;
	HRESULT		hr ;
	D3DCAPS8	def_cap ;
	D3DCAPS8	cap ;
	D3DCAPS8	cur_cap ;
	char		devname[64] ;
	D3DADAPTER_IDENTIFIER8	adp_iden ;

	/* 指定デバイス検査 */

	X2W_GetGraphicDeviceName(devname, sizeof(devname) ) ; // 指定デバイス名取得
	ErrorLogPrintf("Selected Device:%s\n", devname) ;

	sel = -1 ;
	adp_num = IDirect3D8_GetAdapterCount(g_pD3D) ;
	for(i=0; i<adp_num; i++)
	{
		/* デバイス名検査 */
		IDirect3D8_GetAdapterIdentifier(g_pD3D, i, D3DENUM_NO_WHQL_LEVEL, &adp_iden) ;

		/* デバイス名前比較 */
		ErrorLogPrintf("Device[%d]:%s\n", i, adp_iden.Description) ;
		if( strcmp(devname, adp_iden.Description) != 0 ){ continue ; }

		/* デバイス取得 */
		hr = IDirect3D8_GetDeviceCaps(g_pD3D,
					D3DADAPTER_DEFAULT, device_type, &cap) ;
		if( FAILED(hr) ){ continue ; }

		/* 検査 */
		if( !DG_CheckD3DDevice(&cap) ){ continue ; }

		/* 使えそうなのでこれで決定 */
		sel = i ;
	}

	if( sel >=0 )
	{
		DG_WinApp.d3d_cap = cap ;
		return(sel) ;
	}

	ErrorLogPrintf("Can't Find Device:%s\n", devname) ;

	/* Defaultデバイス検査 */
	sel = D3DADAPTER_DEFAULT ;
	hr = IDirect3D8_GetDeviceCaps(g_pD3D,
				D3DADAPTER_DEFAULT, device_type, &def_cap) ;
	if( FAILED(hr) ){ return(-1) ; }

	if( DG_CheckD3DDevice(&def_cap) )
	{
		DG_WinApp.d3d_cap = def_cap ;
		return(D3DADAPTER_DEFAULT) ;
	}

	/* Defaultが駄目っぽいのでその他のデバイスを選択 */
	sel = -1 ;
	adp_num = IDirect3D8_GetAdapterCount(g_pD3D) ;
	for(i=1; i<adp_num; i++)
	{
		/* デバイス取得 */
		hr = IDirect3D8_GetDeviceCaps(g_pD3D, i, device_type, &cap) ;
		if( FAILED(hr) ){ continue ; }

		/* 検査 */
		if( !DG_CheckD3DDevice(&cap) ){ continue ; }

		/* 比較 */
		if( (sel < 0)
		  || (DG_CmpD3DDevice(&cap, &cur_cap) > 0) )	// 今の方が良さげ
		{
			sel = i ;
			cur_cap = cap ;
		}
	}

	/* 処理終了 */
	if( sel >= 0 ){ DG_WinApp.d3d_cap = cap ; }
	return(sel) ;
}

/* 最低限のD3Dデバイス能力があるか検査 */
static int	DG_CheckD3DDevice(D3DCAPS8 *cap)
{
	/* 最低限のDirect3Dのサポートがあるか検査する */

	/* DevCap */
	#define	DEVCAPS_HAL_MASK	(D3DDEVCAPS_DRAWPRIMTLVERTEX	\
								|D3DDEVCAPS_HWRASTERIZATION		\
								|D3DDEVCAPS_HWTRANSFORMANDLIGHT	\
								|D3DDEVCAPS_PUREDEVICE)

	if( cap->DeviceType == D3DDEVTYPE_HAL )
	{
		if( (cap->DevCaps & DEVCAPS_HAL_MASK) != DEVCAPS_HAL_MASK )
		{
			return(FALSE) ;	// 機能を満たさず
		}
	}

	/* PrimitiveMiscCaps */
	#define	PRIMITIVEMISCCAPS_MASK	(D3DPMISCCAPS_BLENDOP			\
									|D3DPMISCCAPS_COLORWRITEENABLE	\
									|D3DPMISCCAPS_CULLCCW			\
									|D3DPMISCCAPS_CULLCW			\
									|D3DPMISCCAPS_CULLNONE			\
									|D3DPMISCCAPS_MASKZ)

	if( (cap->PrimitiveMiscCaps & PRIMITIVEMISCCAPS_MASK) != PRIMITIVEMISCCAPS_MASK )
	{
		return(FALSE) ;	// 機能を満たさず
	}

	/* ZCmpCaps */
	#define	ZCMPCAPS_MASK	(D3DPCMPCAPS_ALWAYS			\
							|D3DPCMPCAPS_GREATEREQUAL	\
							|D3DPCMPCAPS_LESSEQUAL)
	if( (cap->ZCmpCaps & ZCMPCAPS_MASK) != ZCMPCAPS_MASK )
	{
		return(FALSE) ;	// 機能を満たさず
	}

	/* SrcBlendCaps */
	#define	SRCBLENDAPS_MASK	(D3DPBLENDCAPS_DESTALPHA	\
								|D3DPBLENDCAPS_DESTCOLOR 	\
								|D3DPBLENDCAPS_INVDESTALPHA \
								|D3DPBLENDCAPS_INVDESTCOLOR \
								|D3DPBLENDCAPS_INVSRCALPHA 	\
								|D3DPBLENDCAPS_INVSRCCOLOR 	\
								|D3DPBLENDCAPS_ONE 			\
								|D3DPBLENDCAPS_SRCALPHA 	\
								|D3DPBLENDCAPS_SRCCOLOR 	\
								|D3DPBLENDCAPS_ZERO)
	if( (cap->SrcBlendCaps & SRCBLENDAPS_MASK) != SRCBLENDAPS_MASK )
	{
		return(FALSE) ;	// 機能を満たさず
	}

	/* DestBlendCaps */
	#define	DESTBLENDAPS_MASK	(D3DPBLENDCAPS_DESTALPHA	\
								|D3DPBLENDCAPS_DESTCOLOR 	\
								|D3DPBLENDCAPS_INVDESTALPHA \
								|D3DPBLENDCAPS_INVDESTCOLOR \
								|D3DPBLENDCAPS_INVSRCALPHA 	\
								|D3DPBLENDCAPS_INVSRCCOLOR 	\
								|D3DPBLENDCAPS_ONE 			\
								|D3DPBLENDCAPS_SRCALPHA 	\
								|D3DPBLENDCAPS_SRCCOLOR 	\
								|D3DPBLENDCAPS_ZERO)
	if( (cap->DestBlendCaps & DESTBLENDAPS_MASK) != DESTBLENDAPS_MASK )
	{
		return(FALSE) ;	// 機能を満たさず
	}

	/* AlphaCmpCaps */
	#define	ALPHACMPCAPS_MASK	(D3DPCMPCAPS_GREATER)
	if( (cap->AlphaCmpCaps & ALPHACMPCAPS_MASK) != ALPHACMPCAPS_MASK )
	{
		return(FALSE) ;	// 機能を満たさず
	}


	/* その他のテクスチャ マッピング能力(TextureCaps) */
	#define	TEXTURECAPS_MASK		(D3DPTEXTURECAPS_ALPHA		\
									|D3DPTEXTURECAPS_PROJECTED)
	#define	TEXTURECAPS_NMASK		(D3DPTEXTURECAPS_SQUAREONLY)


	if( (cap->TextureCaps & TEXTURECAPS_MASK) != TEXTURECAPS_MASK )
	{
		return(FALSE) ;	// 機能を満たさず
	}

	if( (cap->TextureCaps & TEXTURECAPS_NMASK) )
	{
		return(FALSE) ;	// 機能制限により使用できない
	}

	/* テクスチャ アドレッシング能力(TextureAddressCaps) */
	#define	TEXTUREADDRESSCAPS_MASK		(D3DPTADDRESSCAPS_CLAMP		\
										|D3DPTADDRESSCAPS_WRAP)

	if( (cap->TextureAddressCaps & TEXTUREADDRESSCAPS_MASK) != TEXTUREADDRESSCAPS_MASK )
	{
		return(FALSE) ;	// 機能を満たさず
	}

	/* 線描画プリミティブの能力(LineCaps) */
	#define	LINECAPS_MASK		(D3DLINECAPS_ZTEST)

	if( (cap->LineCaps & LINECAPS_MASK) != LINECAPS_MASK )
	{
		return(FALSE) ;	// 機能を満たさず
	}

	/* 最大のテクスチャ幅(MaxTextureWidth) */
	if( cap->MaxTextureWidth < 2048 )
	{
		return(FALSE) ;	// 機能を満たさず
	}

	/* 最大のテクスチャ高(MaxTextureHeight) */
	if( cap->MaxTextureHeight < 2048 )
	{
		return(FALSE) ;	// 機能を満たさず
	}

	/* 柔軟な頂点フォーマットの能力(FVFCaps) */
	if( (cap->FVFCaps & D3DFVFCAPS_TEXCOORDCOUNTMASK) < 3 )	// Texture座標数
	{
		return(FALSE) ;	// 機能を満たさず
	}

	/* TextureOpCaps */
	#define	TEXTUREOPCAPS_MASK		(D3DTEXOPCAPS_DISABLE		\
									|D3DTEXOPCAPS_MODULATE2X	\
									|D3DTEXOPCAPS_SELECTARG2	\
									|D3DTEXOPCAPS_MODULATE)		

	if( (cap->TextureOpCaps & TEXTUREOPCAPS_MASK) != TEXTUREOPCAPS_MASK )
	{
		return(FALSE) ;	// 機能を満たさず
	}

	/* 頂点の処理能力(VertexProcessingCaps) */
	#define	VERTEXPROCESSINGCAPS_MASK	(D3DVTXPCAPS_DIRECTIONALLIGHTS 	\
										|D3DVTXPCAPS_TEXGEN)		

	if( (cap->VertexProcessingCaps & VERTEXPROCESSINGCAPS_MASK) != VERTEXPROCESSINGCAPS_MASK)
	{
		return(FALSE) ;	// 機能を満たさず
	}

	/* 判定終了 */
	return(TRUE) ;	// 使用可能
}

/* PixelShader使用可能検査 */
static int	DG_CheckPixelShaderCap(D3DCAPS8 *cap)
{
	/* PixelShaderVersion(ps.1.1以上の能力が必要) */
	if( cap->PixelShaderVersion < D3DPS_VERSION(1, 1) ){ return(FALSE) ; }

	/* アプリケーション的都合により、Multi Texture x4 は必要 */
	if( cap->MaxSimultaneousTextures < 4 ){ return(FALSE) ; }

	/* 使用可能 */
	return(TRUE) ;
}

/* VertexShader使用可能検査 */
static int	DG_CheckVertexShaderCap(D3DCAPS8 *cap)
{
	/* PixelShaderが使用できない場合は使用できないと見なします(GeForce4 MX対策) */
	if( !DG_CheckPixelShaderCap(cap) ){ return(FALSE) ; }

	/* VertexShaderVersion(vs.1.1以上の能力が必要) */
	if( cap->VertexShaderVersion < D3DVS_VERSION(1, 1) ){ return(FALSE) ; }

	/* VertexBuffer最大Stride */
	#ifndef MAX
	#define	MAX(a_, b_)	((a_)>(b_)?(a_):(b_))
	#endif
	#define	MAX_STRIDE	MAX(sizeof(DG_VERTEX_DMAPACK2D),		\
						MAX(sizeof(DG_VERTEX_DMAPACK2D4),		\
						MAX(sizeof(DG_VERTEX_EVM),				\
						MAX(sizeof(DG_VERTEX_DMAPACK2D),		\
						MAX(sizeof(DG_VERTEX_KMSS),				\
						    sizeof(DG_VERTEX_KMSM))))))

	if( cap->MaxStreamStride < MAX_STRIDE ){ return(FALSE) ; }


	/* VertexShader入力Stream最大数(必要数２) */
	if( cap->MaxStreams < 2 ){ return(FALSE) ; }

	/* VertexShader定数レジスタ必要数検査 */
	if( cap->MaxVertexShaderConst < MAX_VERTEXSHADER_CONSTANT_NUM ){ return(FALSE) ; }

	return(TRUE) ;
}

/* DG_OBJ描画にVertexShaderを使用するか判定 */
static int	DG_CheckObjVertexShaderCap(D3DCAPS8 *cap)
{
	return( (cap->MaxVertexBlendMatrices < 2) ) ;	// 頂点Blend数判定
}

/* DG_EVMOBJ描画にVertexShaderを使用するか判定 */
static int	DG_CheckEvmVertexShaderCap(D3DCAPS8 *cap)
{
	return(TRUE) ;	// 現在はVertexShaderのみの描画ルーチンのみ
}

/* D3Dデバイス能力の比較 */
static int	DG_CmpD3DDevice(D3DCAPS8 *cap1, D3DCAPS8 *cap2)
{
	int	val ;

	val = 0 ;

	/* MaxVertexIndex */
	if( cap1->MaxVertexIndex > 0xffff ){ val-- ; }
	if( cap2->MaxVertexIndex > 0xffff ){ val++ ; }

	/* VertexShaderVersion */
	val += (int)(cap2->VertexShaderVersion & 0xffff)
		-(int)(cap1->VertexShaderVersion & 0xffff) ;

	/* PixelSixelShaderVersion ;

	/* 判定終了 */
	return(0) ;	// 同程度の能力とする
}

/* Z-Buffer形式選択 */
static	D3DFORMAT	_sel_depth_fmt_tbl[] =
{
    D3DFMT_D15S1,			// 低機能な物から選択
    D3DFMT_D16,
    D3DFMT_D16_LOCKABLE,
    D3DFMT_D24X8,
    D3DFMT_D24X4S4,
    D3DFMT_D24S8,
    D3DFMT_D32,

	D3DFMT_FORCE_DWORD,	// 終端
} ;

static	D3DFORMAT	_sel_depthstencil_fmt_tbl[] =	
{
    D3DFMT_D15S1,			// 低機能な物から選択
    D3DFMT_D24X4S4,
    D3DFMT_D24S8,
    D3DFMT_D16,				// ここから下はStencil機能は無い....
    D3DFMT_D16_LOCKABLE,
    D3DFMT_D24X8,
    D3DFMT_D32,

	D3DFMT_FORCE_DWORD,	// 終端
} ;

static D3DFORMAT	DG_SelectDepthStencilFormat(D3DCAPS8 *cap,
										D3DFORMAT display,
										D3DFORMAT backbuffer,
										D3DRESOURCETYPE RType,
  										D3DFORMAT def_fmt)
{
	HRESULT		hr ;
	D3DFORMAT	fmt ;
	D3DFORMAT	*fmt_ptr ;

	/*-- 指定されたFormatの検査 ---------------------------------------------*/

	hr = IDirect3D8_CheckDeviceFormat(g_pD3D,
							cap->AdapterOrdinal, 	// 使用中のアダプタ番号
							cap->DeviceType,
							display,				// Display Format
							D3DUSAGE_DEPTHSTENCIL,
							RType,					// Texture使用指定
							def_fmt) ;				// 検査するFormat
	if( !FAILED(hr) )
	{
	    hr = IDirect3D8_CheckDepthStencilMatch(g_pD3D,
								cap->AdapterOrdinal,
								cap->DeviceType,
								display,	// Display Format
								backbuffer,	// BackBuffer Format
								def_fmt) ;	// Match検査
		
		if( !FAILED(hr) ){ return(def_fmt) ; }	// 使用可能
	}
#ifdef DEBUG_MODE
	printf("can't use DepthStencil Buffer %d(Resource Type %d)\n", def_fmt, RType) ;
#endif
	/*-----------------------------------------------------------------------*/

	/*-- 代替Format検索 -----------------------------------------------------*/

	switch( def_fmt )
	{
	  case D3DFMT_D15S1 :
	  case D3DFMT_D24X4S4 :
	  case D3DFMT_D24S8 :
		fmt_ptr = _sel_depthstencil_fmt_tbl ; 	// Stencil使用
		break ;

	  default :
		fmt_ptr = _sel_depth_fmt_tbl ; 			// Stencil使用
		break ;
	}

	while( (fmt = *fmt_ptr) != D3DFMT_FORCE_DWORD )
	{
#ifdef DEBUG_MODE
		printf("\tcheck DepthStencil Buffer %d\n", fmt) ;
#endif

		hr = IDirect3D8_CheckDeviceFormat(g_pD3D,
								cap->AdapterOrdinal, 	// 使用中のアダプタ番号
								cap->DeviceType,
								display,				// Display Format
								D3DUSAGE_DEPTHSTENCIL,
								RType,					// Texture使用指定
								fmt) ;					// 検査するFormat

		if( !FAILED(hr) )
		{
		    hr = IDirect3D8_CheckDepthStencilMatch(g_pD3D,
									cap->AdapterOrdinal,
									cap->DeviceType,
									display,	// Display Format
									backbuffer,	// BackBuffer Format
									fmt) ;	// Match検査
			if( !FAILED(hr) ){ break ; }		// これに決定
		}

		fmt_ptr++ ;
	}
	/*-----------------------------------------------------------------------*/

#ifdef DEBUG_MODE
	printf("Select DepthStencil Buffer %d(Resource Type %d)\n", fmt, RType) ;
#endif
	return(fmt) ;
}

/* BackBuffer形式選択 */
static	D3DFORMAT	_sel_backbuffer_fmt_list_16bit[] =
{
    D3DFMT_R5G6B5,
    D3DFMT_X8R8G8B8,

	D3DFMT_FORCE_DWORD,	// 終端
} ;

static	D3DFORMAT	_sel_backbuffer_fmt_list_32bit[] =
{
    D3DFMT_X8R8G8B8,
    D3DFMT_R5G6B5,

	D3DFMT_FORCE_DWORD,	// 終端
} ;

static D3DFORMAT	DG_SelectBackBufferFormat(D3DCAPS8 *cap, D3DFORMAT display,
										BOOL is32bit)
{
	D3DFORMAT	fmt ;
	D3DFORMAT	*list ;
	HRESULT		hr ;

	switch( is32bit )
	{
	  case 0 :	// 16bit
		list = _sel_backbuffer_fmt_list_16bit ;
		break ;

	  default :	// 32bit
		list = _sel_backbuffer_fmt_list_32bit ;
		break ;
	}

	/*-- 使用可能判定 -------------------------------------------------------*/

	while( TRUE )
	{
		fmt = *list ;
		if( fmt == D3DFMT_FORCE_DWORD ){ break ; }	// 終端

		hr = IDirect3D8_CheckDeviceType(g_pD3D,
								cap->AdapterOrdinal, 	// 使用中のアダプタ番号
								cap->DeviceType,
								display,				// Display Format
								fmt,
								FALSE) ;
		if( !FAILED(hr) ){ break ; }
		list++ ;
	}
	/*-----------------------------------------------------------------------*/
printf("SELECT(%d)\n", fmt) ;
	return( fmt ) ;
}

/* RenderTarget形式選択 */
static D3DFORMAT	DG_SelectRenderTargetFormat(D3DCAPS8 *cap, D3DFORMAT display,
										D3DFORMAT *list)
{
	HRESULT	hr ;
	D3DFORMAT	fmt ;

	while( TRUE )
	{
		fmt = *list ;
		if( fmt == D3DFMT_FORCE_DWORD ){ break ; }	// 終端

		hr = IDirect3D8_CheckDeviceFormat(g_pD3D,
									cap->AdapterOrdinal, 	// 使用中のアダプタ番号
									cap->DeviceType,
									display,				// Display Format
									D3DUSAGE_RENDERTARGET,
									D3DRTYPE_TEXTURE,		// Texture使用指定
									fmt) ;					// 検査するFormat
		if( !FAILED(hr) ){ break ; }	// 使用可能
		list++ ;
	}

	return(fmt) ;
}

/* Direct3D関係のリソースを全て解放 */
void DG_KillDaemon(void)
{

	/*-- 描画用イベント解放 -------------------------------------------------*/
#if __DG_DRAW_IN_MAIN_LOOP__ || __DG_DRAW_IN_DRAWTHREAD__ || __DG_DRAW_IN_EXEBUFFER__
	CloseHandle(DG_RequestDrawEventHandle) ;
	CloseHandle(DG_DrawDoneEventHandle) ;
#endif
	/*-----------------------------------------------------------------------*/

	if( !DG_DeviceLost )
	{
		if (DG_ShadowSurface) {
			DG_DestroySurface( DG_ShadowSurface );
			DG_ShadowSurface = NULL ;
		}
		if (DG_ShadowDepthSurface) {
			DG_DestroySurface( DG_ShadowDepthSurface );
			DG_ShadowDepthSurface = NULL ;
		}
#if !__BACKBUFFER_BY_RENDTEX__
		if (DG_BackBuffer2Texture) {
			DG_ReleaseD3DTexture( DG_BackBuffer2Texture );
			DG_BackBuffer2Texture = NULL ;
		}
#endif
		if (DG_ShadowTexture) {
			DG_ReleaseD3DTexture( DG_ShadowTexture );
			DG_ShadowTexture = NULL ;
		}

#if __BACKBUFFER_BY_RENDTEX__
		{
			int	i ;
			LPDIRECT3DTEXTURE8	*tex ;
			LPDIRECT3DSURFACE8	*sur ;

			tex = DG_BackBufferTexture ;
			sur = DG_BackBufferSurface ;
			for(i=3; i>0; i--, tex++, sur++)
			{
				if( *sur) {
					IDirect3DSurface8_Release( *sur );
					*sur = NULL ;
				}
				if( *tex) {
					DG_ReleaseD3DTexture( *tex );
					*tex = NULL ;
				}
			}
			if (DG_BackBufferDepthSurface) {
				DG_DestroySurface( DG_BackBufferDepthSurface );
				DG_BackBufferDepthSurface = NULL ;
			}
		}
#endif

		ReleaseResidentShader() ;
		DG_ReleaseD3DState() ;
		DG_ReleaseExeBuffer() ;
//		DG_FreeTextureAll();
		//DG_ReleaseParticleVertexBuffer();
		DG_ReleasePatchVertexBuffer();
		DG_ReleaseKmsVertexBuffer();
		DG_ReleasePixelShader() ;
		DG_ReleaseVertexShader() ;
		DG_ReleaseDynamicVertexBufferSystem() ;
		DG_ReleaseMemorySystem() ;
		DG_ReleaseTexture() ;
		DG_ReleaseDGMdlVertexBufferManager() ;
		DG_ReleaseBackBufferManager() ;
	}

	if (g_pD3D) {
		IDirect3D8_Release( g_pD3D );
		g_pD3D = NULL ;
	}
	if (g_pd3dDevice) {
		IDirect3DDevice8_Release( g_pd3dDevice );
		g_pd3dDevice = NULL ;
	}
}

/* ---------------------------------------------------------------- */
void DG_SetDisplayOffset( int x, int y, int flag )
{
}

/*--------------------------------------------------------------*/
/*	DG_InitBackBufferManager	BackBuffer管理初期化			*/
/*--------------------------------------------------------------*/
typedef	struct	DGBackBufferManager_
{
	LPDIRECT3DSURFACE8	render_target ;			// 元のRenderTarget
#if __FRAMEBUFFER_USE_DEPTH__
	LPDIRECT3DSURFACE8	render_depth_target ;	// 元のDepthSurface
#endif
	BOOL				backbuff2flame ;	// FrameBuffer転送済みフラグ

	DWORD	flag ;
	#define	M_DGBACKBUFF_COPY_NEED_MODIFY	(1 << 0)	// Copyにはデータ改変が必要
} DGBackBufferManager ;

static	DGBackBufferManager	_bbman ;

void DG_InitBackBufferManager(void)
{
#if __BACKBUFFER_BY_RENDTEX__
	int		i ;
	HRESULT	hr ;

	ZeroMemory(&_bbman, sizeof(DGBackBufferManager)) ;
	DG_CurrentBackBuffer = 0 ;

	/*-- Clear -------------------------------------------------*/

	for(i=4; i>0; i--)	// 数回やっておく
	{
		hr = IDirect3DDevice8_Present(g_pd3dDevice, NULL, NULL, NULL, NULL);
#ifdef DEBUG_MODE
		if( FAILED(hr) )
		{
			dbgErrMessPuts("", hr) ;
			ASSERT(0) ;
		}
#endif

		hr = IDirect3DDevice8_BeginScene(g_pd3dDevice) ;
		ASSERT( !FAILED(hr) ) ;
		hr = IDirect3DDevice8_Clear(g_pd3dDevice, 0, NULL, D3DCLEAR_TARGET,
						 0x00000000, 0.0f, 0) ;
		ASSERT( !FAILED(hr) ) ;
		hr = IDirect3DDevice8_EndScene(g_pd3dDevice) ;
		ASSERT( !FAILED(hr) ) ;
	}
	/*----------------------------------------------------------*/

	/*-- 本来のRenderTarget取得 --------------------------------*/

#if __DG_DRAW_IN_EXEBUFFER__
	DG_SetCurrentRenderTarget() ;
#endif

	DG_GetRenderTarget(&_bbman.render_target) ;
#if __FRAMEBUFFER_USE_DEPTH__
	DG_GetDepthStencilSurface(&_bbman.render_depth_target) ;
#endif
	/*----------------------------------------------------------*/

	/*-- Surface比較 -------------------------------------------*/

	{
		D3DSURFACE_DESC	src_desc ;
		D3DSURFACE_DESC	dst_desc ;

		IDirect3DSurface8_GetDesc(DG_BackBufferSurface[0], &src_desc) ;
		IDirect3DSurface8_GetDesc(_bbman.render_target, &dst_desc) ;

		if(  (src_desc.Format != dst_desc.Format)	// Formatが違う
		  || (src_desc.Width  != dst_desc.Width)	// 幅が違う
		  || (src_desc.Height != dst_desc.Height) )	// 高さが違う
		{
			_bbman.flag |= M_DGBACKBUFF_COPY_NEED_MODIFY ;
												// Copyにはデータ改変が必要
		}
	}
	/*----------------------------------------------------------*/

	/*-- RenderTarget初期設定 ----------------------------------*/

	_bbman.backbuff2flame = FALSE ;
#if FALSE
	DG_SetRenderTarget(DG_BackBufferSurface[0], DG_BackBufferDepthSurface) ;
	DG_RendToBackBuffer = TRUE ;
	DG_Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
				 0x00000000, 0.0f, 0) ;
#else
	DG_EndCurrentBackBuffer() ;
#endif
	/*----------------------------------------------------------*/
#endif
}

/*--------------------------------------------------------------*/
/*	DG_ReleaseBackBufferManager	BackBuffer管理終了処理			*/
/*--------------------------------------------------------------*/

static void DG_ReleaseBackBufferManager(void)
{
#if __BACKBUFFER_BY_RENDTEX__

#if __FRAMEBUFFER_USE_DEPTH__
	if( _bbman.render_target || _bbman.render_depth_target )
	{
		DG_SetRenderTarget(_bbman.render_target, _bbman.render_depth_target) ;
		DG_RendToBackBuffer = FALSE ;

		if( _bbman.render_target )
		{
			IDirect3DSurface8_Release(_bbman.render_target) ;
			_bbman.render_target = NULL ;
		} 
		if( _bbman.render_depth_target )
		{
			IDirect3DSurface8_Release(_bbman.render_depth_target) ;
			_bbman.render_depth_target = NULL ;
		} 
	}
#else
	if( _bbman.render_target )
	{
		DG_SetRenderTarget(_bbman.render_target, NULL) ;
		DG_RendToBackBuffer = FALSE ;
		IDirect3DSurface8_Release(_bbman.render_target) ;
		_bbman.render_target = NULL ;
	}
#endif

#endif
}

/*--------------------------------------------------------------*/
/*	DG_CopyBackBuffer2Frame		BackBufferをFrameにコピー		*/
/*--------------------------------------------------------------*/

void DG_CopyBackBuffer2Frame(void)
{
#if __BACKBUFFER_BY_RENDTEX__
#if __FRAMEBUFFER_LOCKABLE__
	HRESULT	hr ;

	if( _bbman.backbuff2flame ){ return ; }

	if( !(_bbman.flag & M_DGBACKBUFF_COPY_NEED_MODIFY) )
	{
		hr = IDirect3DDevice8_CopyRects(g_pd3dDevice,
							DG_BackBufferSurface[DG_CurrentBackBuffer], NULL, 0,
							_bbman.render_target, NULL) ;
		ASSERT( !FAILED(hr) ) ;
	}
#if FALSE
	else
	{
		D3DSURFACE_DESC		src_desc ;
		D3DLOCKED_RECT		lrect ;
		LPDIRECT3DSURFACE8	src ;
		RECT				src_rect ;
		RECT				dst_rect ;

		src = DG_BackBufferSurface[DG_CurrentBackBuffer] ;
		hr = IDirect3DSurface8_LockRect(src, &lrect, NULL, D3DLOCK_READONLY) ;
		ASSERT( !FAILED(hr) ) ;
		hr = IDirect3DSurface8_GetDesc(src, &src_desc) ;
		ASSERT( !FAILED(hr) ) ;

		src_rect.left   = 0 ;
		src_rect.top    = 0 ;
		src_rect.right  = src_desc.Width ;
		src_rect.bottom = src_desc.Height ;

		dst_rect = src_rect ;

		hr= D3DXLoadSurfaceFromMemory(_bbman.render_target,
								NULL,			// CONST PALETTEENTRY*
								&dst_rect,		// CONST RECT* pDestRect,
								lrect.pBits,	// LPCVOID pSrcMemory,
								src_desc.Format,// D3DFORMAT SrcFormat,
								lrect.Pitch,	// UINT SrcPitch,
								NULL,			// CONST PALETTEENTRY* pSrcPalette,
								&src_rect,		// CONST RECT* pSrcRect,
								D3DX_FILTER_TRIANGLE,	//DWORD Filter,(Dither無し)
								0);				//D3DCOLOR ColorKey
		ASSERT( !FAILED(hr) ) ;

		IDirect3DSurface8_UnlockRect(src) ;
	}
#endif

	_bbman.backbuff2flame = TRUE ;	// 転送済み
#endif
#endif
}

/*--------------------------------------------------------------*/
/*	DG_RendBackBuffer2Frame		BackBufferをFrameにRendering	*/
/*--------------------------------------------------------------*/

/* DG_VERTEX_DMAPACK2D頂点へのデータ設定(Warning対策) */
#define	setVertexDMAPack2DXY(dst_, x_, y_)	\
				{(dst_)->x = (short)(x_);(dst_)->y = (short)(y_);}
#define	setVertexDMAPack2DUV(dst_, u_, v_)	\
				{(dst_)->u = (short)(u_);(dst_)->v = (short)(v_);}
#define	setVertexDMAPack2DRGBA(dst_, rgba_)	\
				{(dst_)->rgba = (rgba_);}

/* DG_VERTEX_DMAPACK2D頂点へのデータ設定(VertexShader非対応版)(Warning対策) */
#define	setVertexDMAPack2DFVFXY(dst_, x_, y_)	\
				{(dst_)->x = (float)(x_);(dst_)->y = (float)(y_);(dst_)->z=(float)(0.0f);}
#define	setVertexDMAPack2DFVFUV(dst_, u_, v_)	\
				{(dst_)->u = (float)(u_);(dst_)->v = (float)(v_);}
#define	setVertexDMAPack2DFVFRGBA(dst_, rgba_)	\
				{(dst_)->rgba = (rgba_);}


static void DG_RendBackBuffer2Frame_Normal(void) ;
static void DG_RendBackBuffer2Frame_NVS(void) ;

void DG_RendBackBuffer2Frame(void)
{
#if __BACKBUFFER_BY_RENDTEX__

	DG_VIEWPORT		viewport ;

	if( _bbman.backbuff2flame ){ return ; }	// 転送済だったらやらない

	/*-- 本当のBackBufferをRenderTargetに戻す ------------------*/

#if __FRAMEBUFFER_USE_DEPTH__
	DG_SetRenderTarget(_bbman.render_target, _bbman.render_depth_target) ;
	DG_Clear(0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
				 0x00000000, 0.0f, 0) ;
#else
	DG_SetRenderTarget(_bbman.render_target, NULL) ;
#endif
	DG_RendToBackBuffer = FALSE ;
	if( _bbman.render_target )
	{
		IDirect3DSurface8_Release(_bbman.render_target) ;
		_bbman.render_target = NULL ;
	} ;
#if __FRAMEBUFFER_USE_DEPTH__
	if( _bbman.render_depth_target )
	{
		IDirect3DSurface8_Release(_bbman.render_depth_target) ;
		_bbman.render_depth_target = NULL ;
	} ;
#endif

	/*----------------------------------------------------------*/

	/*-- Vireport指定 ------------------------------------------*/

	viewport.lx     = (float)(( DISPLAY_WIDTH - DRAW_WIDTH ) / 2 + 0)
					/ (float)DISPLAY_WIDTH ;
	viewport.ly     = (float)(( DISPLAY_HEIGHT - DRAW_HEIGHT ) / 2 + 0)
					/ (float)DISPLAY_HEIGHT ;
	viewport.width  = (float)DRAW_WIDTH / (float)DISPLAY_WIDTH ;
	viewport.height = (float)DRAW_HEIGHT / (float)DISPLAY_HEIGHT ;
	viewport.min_z  = 0.0f; /* クリップボリュームの最小値 */
	viewport.max_z  = 1.0f; /* クリップボリュームの最大値 */
	DG_SetViewport( &viewport );
	/*----------------------------------------------------------*/

	/*-- RenderState設定 ---------------------------------------*/

	DG_SetPixelShader( NULL );

	DG_SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	DG_SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	DG_SetRenderState( D3DRS_FOGENABLE, FALSE );
	DG_SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	DG_SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

	DG_SetTextureDirect( 0, DG_BackBufferTexture[DG_CurrentBackBuffer] );
	DG_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	DG_SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
	DG_SetAlphaMode( 0 );
	DG_SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	/*----------------------------------------------------------*/

	/*-- VertexProcessing設定 ----------------------------------*/

	DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
						DG_CheckVertexBufferSoftProcessing()) ;
	/*----------------------------------------------------------*/

	/*-- 描画設定 ----------------------------------------------*/

	if( DG_CheckUseVertexShader() )
	{
		DG_RendBackBuffer2Frame_Normal() ;
	}
	else
	{
		DG_RendBackBuffer2Frame_NVS() ;	// VertexShader非対応版
	}
	/*----------------------------------------------------------*/

	_bbman.backbuff2flame = TRUE ;	// 転送済みフラグ

	/*-- 終了処理 ----------------------------------------------*/

	DG_SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
	DG_SetRenderState( D3DRS_ZWRITEENABLE,    TRUE );
	DG_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE2X );
	DG_SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE2X );

	DG_SetViewport( &DG_CurrentViewport );	// 補正無しViewport
	/*----------------------------------------------------------*/
#endif
}

static void DG_RendBackBuffer2Frame_Normal(void)
{
	int				col ;
#ifndef _WINDOWS
	static FVECTOR full_screen_param[2] = {
		{0,-DRAW_HEIGHT,1.0f,0},
		{2.0f/DRAW_WIDTH,-2.0f/DRAW_HEIGHT,1,1}
	};
#else
	static FVECTOR full_screen_param[2] = {
		{-1.0f, 1.0f, 1.0f, 1.0f},
		{2.0f/DRAW_WIDTH,-2.0f/DRAW_HEIGHT, 0.0f, 0.0f}
	};
#endif
	static FVECTOR VS_Const[] = {
		{0.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
			{0.5f, 0.5f, 0.5f, 0.5f},
		{1.0f/4096.0f, 1.0f/255.0f, 0, 0},
	};
	extern DG_VERTEXSHADER	DG_DmapackVertexShader[2] ;

	/*-- Vertex Shader設定 -------------------------------------*/

	DG_SetVertexShaderConstant(CV_ZERO,  &VS_Const[0], 1);
	DG_SetVertexShaderConstant(CV_ONE,   &VS_Const[1], 1);
	DG_SetVertexShaderConstant(CV_HALF,  &VS_Const[2], 1);
	DG_SetVertexShaderConstant(CV_SCALE, &VS_Const[3], 1);
#if FALSE
	DG_SetVertexShaderConstant(CV_TEX0_SCALE, tex_param_lintex, 2);
#else
	DG_SetVertexShaderConstant(CV_TEX0_SCALE, DG_BackBufferTexScaleParam, 2);
#endif
	DG_SetVertexShaderConstant(0, full_screen_param, 2);

	DG_SelectVertexShader( &DG_DmapackVertexShader[0] );
	/*----------------------------------------------------------*/

	/*-- Primitive描画 -----------------------------------------*/

	col = 0xffffffff ;
	{
		DG_VERTEX_DMAPACK2D	Vertex[4] ;
		DWORD				sofs ;
		float				coef ;
		float				lu, lv, ru, rv ;

		/*-- UV値の計算 ----------------------------------------*/


		#define	REND_OFS_U_COEF	(DG_WinApp.backbuffer_rend_ofs_w)
												// -0.5fでクリアに描画されます
		#define	REND_OFS_V_COEF	(DG_WinApp.backbuffer_rend_ofs_h)


		coef = (float)DISPLAY_WIDTH / (float)DG_BackBufferRendWidth ;
		coef *= REND_OFS_U_COEF ;
		lu = DG_FRAME_U(coef) * 16384.0f ;
		ru = DG_FRAME_U((float)DRAW_WIDTH + coef) * 16384.0f ;

		coef = (float)DISPLAY_HEIGHT / (float)DG_BackBufferRendHeight ;
		coef *= REND_OFS_V_COEF ;
		lv = DG_FRAME_V(coef) * 16384.0f ;
		rv = DG_FRAME_V((float)DRAW_HEIGHT + coef) * 16384.0f ;
		/*------------------------------------------------------*/

		setVertexDMAPack2DXY(  &Vertex[0], 0, 0) ;
		setVertexDMAPack2DRGBA(&Vertex[0], col) ;
		setVertexDMAPack2DUV(  &Vertex[0], lu, lv) ;
		setVertexDMAPack2DXY(  &Vertex[1], DRAW_WIDTH, 0) ;
		setVertexDMAPack2DRGBA(&Vertex[1], col) ;
		setVertexDMAPack2DUV(  &Vertex[1], ru, lv) ;
		setVertexDMAPack2DXY(  &Vertex[2], 0, DRAW_HEIGHT) ;
		setVertexDMAPack2DRGBA(&Vertex[2], col) ;
		setVertexDMAPack2DUV(  &Vertex[2], lu, rv) ;
		setVertexDMAPack2DXY(  &Vertex[3], DRAW_WIDTH, DRAW_HEIGHT) ;
		setVertexDMAPack2DRGBA(&Vertex[3], col) ;
		setVertexDMAPack2DUV(  &Vertex[3], ru, rv) ;

		DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D), 4, &sofs) ;
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, 2) ;
	}
	/*----------------------------------------------------------*/
}

static void DG_RendBackBuffer2Frame_NVS(void)
{
	int				col ;
	FMATRIX			mtx ;
	static FMATRIX full_screen_mtx =
	{
		2.0f/DRAW_WIDTH,	0.0f,				0.0f,	0.0f,
		0.0f,				-2.0f/DRAW_HEIGHT,	0.0f,	0.0f,
		0.0f,				0.0f,				0.0f,	0.0f,
		-1.0f,				1.0f,				1.0f,	1.0f
	} ;

	/*-- Shader設定 --------------------------------------------*/

	DG_SetRenderState(D3DRS_LIGHTING, FALSE) ;

	{
		D3DXMATRIX	d3dx_mtx ;

		D3DXMatrixOrthoOffCenterLH(&d3dx_mtx, -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f) ;

		DG_SetTransform(D3DTS_PROJECTION, (FMATRIX *)d3dx_mtx.m) ;
		DG_SetTransform(D3DTS_VIEW,       &DG_UnitMatrix) ;
		DG_SetTransform(D3DTS_WORLD,	  &full_screen_mtx) ;
	}

	mtx = DG_UnitMatrix ;
	mtx.m[0][0] = DG_BackBufferTexScaleParam[0].vx ;
	mtx.m[1][1] = DG_BackBufferTexScaleParam[0].vy ;
	mtx.m[2][0] = DG_BackBufferTexScaleParam[1].vx ;
	mtx.m[2][1] = DG_BackBufferTexScaleParam[1].vy ;
	DG_SetTransformTexCoord( D3DTS_TEXTURE0, (void *)&mtx) ;

	DG_SetVertexShader( D3DFVF_DG_VERTEX_DMAPACK2D_FLAG ) ;
	/*----------------------------------------------------------*/

	/*-- Primitive描画 -----------------------------------------*/

	col = 0xffffffff ;
	{
		DG_VERTEX_DMAPACK2D_FVF	Vertex[4] ;
		DWORD					sofs ;
		float					coef ;
		float					lu, lv, ru, rv ;

		/*-- UV値の計算 ----------------------------------------*/

		#define	REND_OFS_U_COEF	(DG_WinApp.backbuffer_rend_ofs_w)
												// -0.5fでクリアに描画されます
		#define	REND_OFS_V_COEF	(DG_WinApp.backbuffer_rend_ofs_h)

		coef = (float)DISPLAY_WIDTH / (float)DG_BackBufferRendWidth ;
		coef *= REND_OFS_U_COEF ;
		lu = DG_FRAME_U(coef) * 16384.0f ;
		ru = DG_FRAME_U((float)DRAW_WIDTH + coef) * 16384.0f ;

		coef = (float)DISPLAY_HEIGHT / (float)DG_BackBufferRendHeight ;
		coef *= REND_OFS_V_COEF ;
		lv = DG_FRAME_V(coef) * 16384.0f ;
		rv = DG_FRAME_V((float)DRAW_HEIGHT + coef) * 16384.0f ;
		/*------------------------------------------------------*/

		setVertexDMAPack2DFVFXY(  &Vertex[0], 0, 0) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[0], col) ;
		setVertexDMAPack2DFVFUV(  &Vertex[0], lu, lv) ;
		setVertexDMAPack2DFVFXY(  &Vertex[1], DRAW_WIDTH, 0) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[1], col) ;
		setVertexDMAPack2DFVFUV(  &Vertex[1], ru, lv) ;
		setVertexDMAPack2DFVFXY(  &Vertex[2], 0, DRAW_HEIGHT) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[2], col) ;
		setVertexDMAPack2DFVFUV(  &Vertex[2], lu, rv) ;
		setVertexDMAPack2DFVFXY(  &Vertex[3], DRAW_WIDTH, DRAW_HEIGHT) ;
		setVertexDMAPack2DFVFRGBA(&Vertex[3], col) ;
		setVertexDMAPack2DFVFUV(  &Vertex[3], ru, rv) ;

		DG_SetDynamicVertexBuffer(Vertex, sizeof(DG_VERTEX_DMAPACK2D_FVF), 4, &sofs) ;
		DG_DrawPrimitive(D3DPT_TRIANGLESTRIP, sofs, 2) ;
	}
	/*----------------------------------------------------------*/
}

/*--------------------------------------------------------------*/
/*	DG_FlipBackBuffer		BackBuffer切り替え処理				*/
/*--------------------------------------------------------------*/
void DG_FlipBackBuffer(void)
{
#if __DG_DRAW_IN_EXEBUFFER__
	DG_SetCurrentRenderTarget() ;
#endif

#if __BACKBUFFER_BY_RENDTEX__
	DG_GetRenderTarget(&_bbman.render_target) ;	// 元のRenderTarget保存
#if __FRAMEBUFFER_USE_DEPTH__
	DG_GetDepthStencilSurface(&_bbman.render_depth_target) ;
#endif

	DG_CurrentBackBuffer ^= 1 ;

	_bbman.backbuff2flame = FALSE ;
	DG_SetRenderTarget(DG_BackBufferSurface[DG_CurrentBackBuffer],
						DG_BackBufferDepthSurface) ;
	DG_RendToBackBuffer = TRUE ;
#endif

}

/*--------------------------------------------------------------*/
/*	DG_EndCurrentBackBuffer		現BackBufferに対する終了処理	*/
/*--------------------------------------------------------------*/
void DG_EndCurrentBackBuffer(void)
{
#if __BACKBUFFER_BY_RENDTEX__

#if __FRAMEBUFFER_USE_DEPTH__
	if( _bbman.render_target || _bbman.render_depth_target )
	{
		DG_SetRenderTarget(_bbman.render_target, _bbman.render_depth_target) ;
		DG_RendToBackBuffer = FALSE ;

		if( _bbman.render_target )
		{
			IDirect3DSurface8_Release(_bbman.render_target) ;
			_bbman.render_target = NULL ;
		} 
		if( _bbman.render_depth_target )
		{
			IDirect3DSurface8_Release(_bbman.render_depth_target) ;
			_bbman.render_depth_target = NULL ;
		} 
	}
#else
	if( _bbman.render_target )
	{
		DG_SetRenderTarget(_bbman.render_target, NULL) ;
		DG_RendToBackBuffer = FALSE ;
		IDirect3DSurface8_Release(_bbman.render_target) ;
		_bbman.render_target = NULL ;
	}
#endif

#endif
}

/*--------------------------------------------------------------*/
/*	DG_SelectRefreshRateInHz	Refresh Rate選択				*/
/*--------------------------------------------------------------*/
static UINT DG_SelectRefreshRateInHz(void)
{
	return(D3DPRESENT_RATE_DEFAULT) ;
}

/*--------------------------------------------------------------*/
/*	DG_SelectPresentationInterval	フレーム切り替え機能選択	*/
/*--------------------------------------------------------------*/

static	UINT _sel_present_interval_tbl[] =
{
	D3DPRESENT_INTERVAL_ONE,
	D3DPRESENT_INTERVAL_IMMEDIATE,
	D3DPRESENT_INTERVAL_TWO,
	D3DPRESENT_INTERVAL_THREE,
	D3DPRESENT_INTERVAL_FOUR,
} ;
#define	SEL_PRESENT_INTERVAL_TBL_NUM	(sizeof(_sel_present_interval_tbl)/sizeof(_sel_present_interval_tbl[0]))

static UINT DG_SelectPresentationInterval(void)
{
	int		i ;
	UINT	*tbl ;
	UINT	flag ;

	flag = DG_WinApp.d3d_cap.PresentationIntervals ;

	tbl = _sel_present_interval_tbl ;
	for(i=SEL_PRESENT_INTERVAL_TBL_NUM-1; i>0; i--, tbl++)
	{
		if( flag & *tbl ){ break ; }	// 指定機能有り
	}

	return(*tbl) ;
}

/*--------------------------------------------------------------*/
/*	DG_InitDispVibration	画面振動初期化						*/
/*--------------------------------------------------------------*/
static void DG_InitDispVibration(void)
{
	ZeroMemory(vib_pend, sizeof(vib_pend)) ;

	DG_SinCos(&vib_pend[0].ofs, &vib_pend[1].ofs, 0.0f) ;
}

/*--------------------------------------------------------------*/
/*	DG_ManageDispVibration	画面振動制御						*/
/*--------------------------------------------------------------*/
static void DG_ManageDispVibration(void)
{
	DG_DispVibrationPend	*pend ;
	float					ofs, dmy ;

	/* 第一振動子 */
	pend = &vib_pend[0] ;

	pend->ang     += pend->ang_spd ;
	pend->ang_spd *= 0.8f ;

	DG_SinCos(&ofs, &dmy, pend->ang) ;
	DG_DispVibration1 = (6.0f/(float)DRAW_WIDTH) * (ofs - pend->ofs) ;	// 振動値
	pend->ofs = ofs ;

	/* 第二振動子 */
	pend = &vib_pend[1] ;

	pend->ang     += pend->ang_spd ;
	pend->ang_spd *= 0.8f ;

	DG_SinCos(&dmy, &ofs, pend->ang) ;
	DG_DispVibration2 = (6.0f/(float)DRAW_HEIGHT) * (ofs - pend->ofs) ;	// 振動値
	pend->ofs = ofs ;
}

/*--------------------------------------------------------------*/
/*	DG_SetDispVibration1	画面振動(PAD振動代替)				*/
/*--------------------------------------------------------------*/
void DG_SetDispVibration1(int value)
{
	DG_DispVibrationPend	*pend ;

	pend = &vib_pend[0] ;
	pend->ang_spd += (((float)value * (1.0f/24.0f)) - pend->ang_spd) * 0.5f ;
}

/*--------------------------------------------------------------*/
/*	DG_SetDispVibration2	画面振動(PAD振動代替)				*/
/*--------------------------------------------------------------*/
void DG_SetDispVibration2(int value)
{
	DG_DispVibrationPend	*pend ;

	pend = &vib_pend[1] ;
	pend->ang_spd += (((float)value * (1.0f/20.0f)) - pend->ang_spd) * 0.5f ;
}

/*--------------------------------------------------------------*/
/*	DG_DeviceReset_ReleaseBackBuffer							*/
/*				DeviceReset時のBackBuffer一時解放処理			*/
/*--------------------------------------------------------------*/
static	void	DG_DeviceReset_ReleaseBackBuffer(void)
{
	/*-- 取得サーフェイス解放 ----------------------------------*/

	IDirect3DSurface8_Release(DG_ShadowSurface) ;

	IDirect3DSurface8_Release(DG_BackBufferSurface[2]) ;
	IDirect3DSurface8_Release(DG_BackBufferSurface[1]) ;
	IDirect3DSurface8_Release(DG_BackBufferSurface[0]) ;
	/*----------------------------------------------------------*/

	/*-- 影テクスチャ解放 --------------------------------------*/

	if( DG_ShadowDepthSurface ){ IDirect3DSurface8_Release(DG_ShadowDepthSurface) ; }
	if( DG_ShadowTexture ){ DG_ReleaseD3DTexture(DG_ShadowTexture) ; }
	/*----------------------------------------------------------*/

	/*-- バックバッファ解放 ------------------------------------*/

	if( DG_BackBufferDepthSurface ){ IDirect3DSurface8_Release(DG_BackBufferDepthSurface) ; }
	DG_ReleaseD3DTexture(DG_BackBufferTexture[2]) ;
	DG_ReleaseD3DTexture(DG_BackBufferTexture[1]) ;
	DG_ReleaseD3DTexture(DG_BackBufferTexture[0]) ;
	/*----------------------------------------------------------*/
}

/*--------------------------------------------------------------*/
/*	DG_DeviceReset_CreateBackBuffer								*/
/*				DeviceReset時のBackBuffer再構築処理				*/
/*--------------------------------------------------------------*/
static	void	DG_DeviceReset_CreateBackBuffer(void)
{
	D3DFORMAT	fmt ;
	DWORD		tex_w, tex_h ;
	HRESULT		hr ;

	/*-- BackBufferテクスチャ作成 ------------------------------*/

	fmt   = DG_BackBufferFormat ;
	tex_w = DG_BackBufferWidth ;
	tex_h = DG_BackBufferHeight ;

	DG_CreateTexture(tex_w, tex_h, 1, D3DUSAGE_RENDERTARGET,
				fmt,
				D3DPOOL_DEFAULT, &DG_BackBufferTexture[0]) ;
	DG_CreateTexture(tex_w, tex_h, 1, D3DUSAGE_RENDERTARGET,
				fmt,
				D3DPOOL_DEFAULT, &DG_BackBufferTexture[1]) ;
	DG_CreateTexture(tex_w, tex_h, 1, D3DUSAGE_RENDERTARGET,
				fmt,
				D3DPOOL_DEFAULT, &DG_BackBufferTexture[2]) ;

	IDirect3DTexture8_GetSurfaceLevel(DG_BackBufferTexture[0], 0,
									&DG_BackBufferSurface[0]) ;
	IDirect3DTexture8_GetSurfaceLevel(DG_BackBufferTexture[1], 0,
									&DG_BackBufferSurface[1]) ;
	IDirect3DTexture8_GetSurfaceLevel(DG_BackBufferTexture[2], 0,
									&DG_BackBufferSurface[2]) ;

	fmt = DG_BackBufferDepthFormat ;
	if( fmt != D3DFMT_FORCE_DWORD )
	{
		hr = DG_CreateDepthStencilSurface(tex_w, tex_h, fmt,
							D3DMULTISAMPLE_NONE,
							&DG_BackBufferDepthSurface) ;
	}
	/*----------------------------------------------------------*/

	/*-- 影用テクスチャ再作成 ----------------------------------*/

	fmt   = DG_ShadowFormat ;	// 影レンダリング用テクスチャFormat
	tex_w = DG_ShadowWidth ;	// 影レンダリング用テクスチャサイズ
	tex_h = DG_ShadowHeight ;

	DG_CreateTexture( tex_w, tex_h, 1, D3DUSAGE_RENDERTARGET,
							fmt, D3DPOOL_DEFAULT, &DG_ShadowTexture) ;
	IDirect3DTexture8_GetSurfaceLevel(DG_ShadowTexture, 0, &DG_ShadowSurface) ;

	fmt = DG_ShadowDepthFormat ;
	if( fmt != D3DFMT_FORCE_DWORD )
	{
		hr = DG_CreateDepthStencilSurface(tex_w, tex_h, fmt,
								D3DMULTISAMPLE_NONE,
								&DG_ShadowDepthSurface) ;
	}
	/*----------------------------------------------------------*/

	/*-- システムテクスチャ設定 --------------------------------*/

	DG_SystemTexture[0].tex_trans.ptex = DG_BackBufferTexture[0] ;
	DG_SystemTexture[1].tex_trans.ptex = DG_BackBufferTexture[1] ;
	DG_SystemTexture[2].tex_trans.ptex = DG_BackBufferTexture[2] ;
	DG_SystemTexture[3].tex_trans.ptex = DG_BackBufferTexture[2] ;
	/*----------------------------------------------------------*/
}

/*--------------------------------------------------------------*/
/*	DG_ResetD3DDevice		デバイスリセット処理				*/
/*--------------------------------------------------------------*/
static	BOOL	_ResetDeviceResult ;

BOOL	DG_ResetD3DDevice(void)
{
	extern	BOOL	GM_AppExit_ConfReq ;	// 終了処理問い合わせ要求

	DG_ResetD3DDeviceReq = TRUE ;	// 要求ON

	while( DG_ResetD3DDeviceReq )
	{
		/* デバイスリセット処理終了待ち */
		if( GM_AppExit_ConfReq ){ return(FALSE) ; }

		/* デバイスリセット処理終了待ち */
		Sleep(1000) ;
	}
	return(_ResetDeviceResult) ;
}

/*--------------------------------------------------------------*/
/*	DG_ResetD3DDeviceMain	デバイスリセット処理本体(WinMain側)	*/
/*--------------------------------------------------------------*/
enum
{
	D3DDEVRESET_STAT_IDLE,				// 停止中
	D3DDEVRESET_STAT_START,				// 開始処理
	D3DDEVRESET_STAT_TESTCOOPERATIVE,	// TestCooperativeLevel
	D3DDEVRESET_STAT_RESET,				// DeviceReset
	D3DDEVRESET_STAT_PRESENTTEST,		// 復帰検査
	D3DDEVRESET_STAT_RESTORE,			// 復帰作業

	D3DDEVRESET_STAT_DISPLAYFORMAT_ERR,	// DisplayFormat変更の為に復帰が出来ない
	D3DDEVRESET_STAT_WAIT_ACTIVE,		// WindowがActiveになるのを待つ

	D3DDEVRESET_STAT_ASK_RETRY,			// 再度復帰作業を行うかを問う

	D3DDEVRESET_STAT_END,				// 終了
} ;

static	int	_ResetD3DDeviceStat = D3DDEVRESET_STAT_IDLE ;

void	DG_ResetD3DDeviceMain(void)
{
	HRESULT	hr ;
	int		sel ;

	if( !DG_ResetD3DDeviceReq ){ return ; }

	switch( _ResetD3DDeviceStat )
	{
	  case D3DDEVRESET_STAT_IDLE :		// 停止中
		if( DG_ExeBufferSync(TRUE) ){ _ResetD3DDeviceStat = D3DDEVRESET_STAT_START ; }
		break ;

	  case D3DDEVRESET_STAT_START :
		_ResetD3DDeviceStat = D3DDEVRESET_STAT_TESTCOOPERATIVE ;

		/*-- Actor処理 -----------------------------------------*/

		GV_ActorResetDeviceReleaseAll() ;
		/*------------------------------------------------------*/

		/*-- Backbuffer解放 ------------------------------------*/

		DG_DeviceReset_ReleaseBackBuffer() ;
		/*------------------------------------------------------*/

		/*-- Resetの前にやっておく解放処理 ---------------------*/

		DG_ReleaseDynamicVertexBufferSystem() ;
		DG_ReleaseBackBufferManager() ;
		DG_ResetDevice_ReleaseD3DRenderingState() ;

		DG_SetRenderTarget(NULL, NULL) ;
		/*------------------------------------------------------*/

	  case D3DDEVRESET_STAT_TESTCOOPERATIVE :	// TestCooperativeLevel

		/*-- 協調レベル検査 ------------------------------------*/

		hr = IDirect3DDevice8_TestCooperativeLevel(g_pd3dDevice) ;
		switch( hr )
		{
		  case D3DERR_DEVICELOST :
			printf("IDirect3DDevice8_TestCooperativeLevel:D3DERR_DEVICELOST\n") ;
			break ;

		  case S_OK :
			printf("IDirect3DDevice8_TestCooperativeLevel:OK\n") ;
			_ResetD3DDeviceStat = D3DDEVRESET_STAT_RESET ;
			break ;

		  case D3DERR_DEVICENOTRESET :
			printf("IDirect3DDevice8_TestCooperativeLevel:Not Reset\n") ;
			_ResetD3DDeviceStat = D3DDEVRESET_STAT_RESET ;
			break ;

		  default :
			ASSERT(0) ;
			break ;
		}
		/*------------------------------------------------------*/
		break ;

	  case D3DDEVRESET_STAT_RESET :				// DeviceReset

		/*-- DisplayFormat検査(Windowedの場合のみ) -------------*/

		if( DG_WinApp.d3dpp.Windowed )
		{
			D3DDISPLAYMODE	dmode ;

			hr = IDirect3D8_GetAdapterDisplayMode(g_pD3D,
													DG_WinApp.d3d_cap.AdapterOrdinal,
													&dmode) ;
			if( FAILED(hr) )
			{
				X2W_ErrorPrintfID(X2W_MB_ERROR, X2WERR_ID_OTHER_FATALERROR) ;
				exit(EXIT_FAILURE) ;
			}

			/*-- 作成時と現在のフォーマットを比較する ----------*/

			if( DG_WinApp.d3dpp.BackBufferFormat != dmode.Format )
			{
				_ResetD3DDeviceStat = D3DDEVRESET_STAT_DISPLAYFORMAT_ERR ;
				break ;
			}
			/*--------------------------------------------------*/
		}
		/*------------------------------------------------------*/

		/*-- Device Reset --------------------------------------*/

		hr = IDirect3DDevice8_Reset(g_pd3dDevice, &DG_WinApp.d3dpp) ;
		if( !FAILED(hr) )
		{
			printf("IDirect3DDevice8_Reset(): OK\n") ;
			_ResetD3DDeviceStat = D3DDEVRESET_STAT_PRESENTTEST ;
		}
		else
		{
			switch( hr )
			{
			  case D3DERR_INVALIDCALL :		// 不正呼び出し
				printf("IDirect3DDevice8_Reset():D3DERR_INVALIDCALL\n") ;

				_ResetD3DDeviceStat = D3DDEVRESET_STAT_END ;
				_ResetDeviceResult = FALSE ;	// 致命的エラー
				break ;

			  case D3DERR_OUTOFVIDEOMEMORY :	// メモリ不足
			  case E_OUTOFMEMORY :
				printf("IDirect3DDevice8_Reset(): D3DERR_OUTOFVIDEOMEMORY or E_OUTOFMEMORY\n") ;
				/* 再試行意思確認 */
				_ResetD3DDeviceStat = D3DDEVRESET_STAT_ASK_RETRY ;
				break ;

			  case D3DERR_DEVICELOST :	// ここに来たら、D3DPOOL_DEFAULTの物が残って
										// しまっている様です。
				/* 再試行意思確認 */
				_ResetD3DDeviceStat = D3DDEVRESET_STAT_ASK_RETRY ;
				break ;

			  default :
				dbgErrMessPuts("IDirect3DDevice8_Reset():", hr) ;
				_ResetD3DDeviceStat = D3DDEVRESET_STAT_END ;
				_ResetDeviceResult = FALSE ;	// 致命的エラー
				break ;
			}
		}
		/*------------------------------------------------------*/
		break ;

	  /* 復帰検査 */
	  case D3DDEVRESET_STAT_PRESENTTEST :
		hr = IDirect3DDevice8_Present(g_pd3dDevice, NULL, NULL, NULL, NULL) ;

		if( !FAILED(hr) )
		{
			printf("IDirect3DDevice8_Present():OK\n") ;
			_ResetD3DDeviceStat = D3DDEVRESET_STAT_RESTORE ;	// OK
		}
		else
		{
			printf("IDirect3DDevice8_Present():Failed\n") ;
			switch( hr )
			{
			  case D3DERR_DEVICELOST :	// Device Lost
				/* 再試行意思確認 */
				_ResetD3DDeviceStat = D3DDEVRESET_STAT_ASK_RETRY ;
				break ;

			  default :
				_ResetD3DDeviceStat = D3DDEVRESET_STAT_END ;
				_ResetDeviceResult = FALSE ;	// 致命的エラー
				break ;
			}
		}
		break ;

	  case D3DDEVRESET_STAT_RESTORE : 	// 復帰作業

		/*-- リソース関係の再確保 ------------------------------*/

		DG_ResetDevice_CreateD3DRenderingState() ;
		/*------------------------------------------------------*/

		/*-- DeviceReset時のBackBuffer再構築処理 ---------------*/

		DG_DeviceReset_CreateBackBuffer() ;
		/*------------------------------------------------------*/

		/*-- 管理初期化 ----------------------------------------*/

		DG_InitRenderState_ResetDevice() ;
		DG_InitBackBufferManager() ;
		DG_InitDynamicVertexBufferSystem() ;
		/*------------------------------------------------------*/

		/*-- Actor処理 -----------------------------------------*/

		GV_ActorResetDeviceCreateAll() ;
		/*------------------------------------------------------*/

		_ResetD3DDeviceStat = D3DDEVRESET_STAT_END ;
		_ResetDeviceResult  = TRUE ;	// 正常終了
		break ;

	  /* DisplayFormat変更の為に復帰が出来ない */
	  case D3DDEVRESET_STAT_DISPLAYFORMAT_ERR :
		
		/*-- Window退避 ----------------------------------------*/

		ShowWindow(DG_hWnd, SW_SHOWMINIMIZED) ;	// アイコン化して退避
		/*------------------------------------------------------*/

		/*-- 注意表示 ------------------------------------------*/

		if( DG_WinApp.flag & M_DG_WINAPP_BACKBUFFER_32BIT )
		{
			/* 32bit */
			X2W_ErrorPrintfID(X2W_MB_WARNING, X2WERR_ID_SET_DISPLAY_32BIT) ;
			WinExec("Rundll32 Shell32.dll,Control_RunDLL Desk.cpl,,3", SW_SHOW);
		}
		else
		{
			/* 16bit */
			X2W_ErrorPrintfID(X2W_MB_WARNING, X2WERR_ID_SET_DISPLAY_16BIT) ;
			WinExec("Rundll32 Shell32.dll,Control_RunDLL Desk.cpl,,3", SW_SHOW);
		}
		/*------------------------------------------------------*/

		/* Activeになるのを待ちます */
		_ResetD3DDeviceStat = D3DDEVRESET_STAT_WAIT_ACTIVE ;
		break ;

	  case D3DDEVRESET_STAT_WAIT_ACTIVE :		// WindowがActiveになるのを待つ

		if( DG_CheckAppDisplay() )	// 表示されたら
		{
			/* もう一度最初から復旧を試みる */
			_ResetD3DDeviceStat = D3DDEVRESET_STAT_TESTCOOPERATIVE ;
		}
		break ;

	  case D3DDEVRESET_STAT_ASK_RETRY :			// 再度復帰作業を行うかを問う

		/*-- Retry? --------------------------------------------*/

		sel = X2W_ErrorPrintfID(X2W_MB_RETRYCANCEL, X2WERR_ID_ASK_RETRY_RESET_DEVICE) ;

		switch( sel )
		{
		  case IDCANCEL :	// キャンセル
			/* 本当に止めるのか意思確認 */
			sel = X2W_ErrorPrintfID(X2W_MB_OKCANCEL | MB_DEFBUTTON2,	// デフォルトCANCEL
								X2WERR_ID_ASK_ERROR_EXIT) ;
			switch( sel )
			{
			  case IDOK :	// OK選択
				X2W_ErrorAbort() ;			// 異常終了
				break ;
			}
			break ;

		  case IDRETRY :	// 再試行
		  default :
			_ResetD3DDeviceStat = D3DDEVRESET_STAT_TESTCOOPERATIVE ;
			break ;
		}
		/*------------------------------------------------------*/
		break ;

	  case D3DDEVRESET_STAT_END :		// 終了
		_ResetD3DDeviceStat = D3DDEVRESET_STAT_IDLE ;

		DG_ResetD3DDeviceReq = FALSE ;
		break ;
	}

}

/*--------------------------------------------------------------*/
/*	DG_InitWindowCursor		Window Cursor 表示/非表示処理初期化	*/
/*--------------------------------------------------------------*/
void DG_InitWindowCursor(BOOL show)
{
	int	disp_cntr ;

	ZeroMemory(&DG_WinAppCursor, sizeof(DG_WinAppCursor)) ;

	if( show )
	{
		while( (disp_cntr = ShowCursor(TRUE)) < 0 ) ;
		DG_WinAppCursor.disp_cntr = disp_cntr ;
		DG_WinAppCursor.show      = TRUE ;
	}
	else
	{
		while( (disp_cntr = ShowCursor(FALSE)) >= 0 ) ;
		DG_WinAppCursor.disp_cntr = disp_cntr ;
		DG_WinAppCursor.show      = FALSE ;
	}

}

/*--------------------------------------------------------------*/
/*	DG_ShowWindowCursor		Window Cursor 表示/非表示処理		*/
/*--------------------------------------------------------------*/
void DG_ShowWindowCursor(BOOL show)
{
	int	disp_cntr ;

	if( show )
	{
		if( !DG_WinAppCursor.show )
		{
			while( (disp_cntr = ShowCursor(TRUE)) < 0 ) ;
			DG_WinAppCursor.disp_cntr = disp_cntr ;
			DG_WinAppCursor.show      = TRUE ;
		}
	}
	else
	{
		if( DG_WinAppCursor.show )
		{
			while( (disp_cntr = ShowCursor(FALSE)) >= 0 ) ;
			DG_WinAppCursor.disp_cntr = disp_cntr ;
			DG_WinAppCursor.show      = FALSE ;
		}
	}
}

void DG_ShowWindowCursorForce(BOOL show)
{
	int	disp_cntr ;

	if( show )
	{
		while( (disp_cntr = ShowCursor(TRUE)) < 0 ) ;
		DG_WinAppCursor.disp_cntr = disp_cntr ;
		DG_WinAppCursor.show      = TRUE ;
	}
	else
	{
		while( (disp_cntr = ShowCursor(FALSE)) >= 0 ) ;
		DG_WinAppCursor.disp_cntr = disp_cntr ;
		DG_WinAppCursor.show      = FALSE ;
	}
}
