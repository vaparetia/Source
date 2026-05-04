//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xdgd.c
	ライブラリ制御デーモン

	2002/02/07 K.Takabe
	$Id: xdgd.c,v 1.7 2002/11/23 11:36:55 Yoshizawa1 Exp $

*/
/*

	void		DG_ResetSystem()

		システムリセット（ステージ切替え時など）

	void		DG_StartDaemon()

		システム起動（ゲームスタート時一度だけ）
*/

#ifdef KP_XBOX //BP
asasas
#ifndef KP_XBOX
#include <d3dx8.h>
#include <xgraphics.h>
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

//#include	"game.h"

extern void DG_SetDisplayOffset( int x, int y, int flag );


/*----------------------------------------------------------------*/
/* DirectX8 依存 */
LPDIRECT3D9       g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
D3DPRESENT_PARAMETERS g_d3dpp;
LPDIRECT3DTEXTURE9 DG_ShadowTexture = NULL; // 影レンダリング用テクスチャ
LPDIRECT3DTEXTURE9	DG_BackBufferTexture[4] ;		/* バックバッファテクスチャ */
LPDIRECT3DSURFACE9	DG_BackBufferSurface[4] ;		/* バックバッファサーフェス */
LPDIRECT3DSURFACE9	DG_DepthBufferSurface = NULL ;	/* Ｚバッファサーフェス */
LPDIRECT3DTEXTURE9	DG_ShadowBackBufferTexture ;	/* 影用バックバッファテクスチャ */
LPDIRECT3DSURFACE9	DG_ShadowBackBufferSurface ;	/* 影用バックバッファサーフェス */
LPDIRECT3DSURFACE9	DG_ShadowDepthBufferSurface ;	/* 影用Ｚバッファサーフェス */
void				*DG_BackBufferAddr[3] ;			/* バックバッファアドレス */
void				*DG_DepthBufferAddr ;			/* Ｚバッファアドレス */
void				*DG_ShadowBackBufferAddr ;		/* 影用バックバッファアドレス */
void				*DG_ShadowDepthBufferAddr ;		/* 影用Ｚバッファアドレス */

#if 0 //BP_RENDER
D3DBaseTexture		DG_BackBufferTextureMemory[ 4 ];			/* テクスチャ実体 */
D3DSurface			DG_DepthBufferSurfaceMemory ;
D3DBaseTexture		DG_ShadowBackBufferTextureMemory;			/* テクスチャ実体 */
D3DSurface			DG_ShadowDepthBufferSurfaceMemory ;
#endif

#ifdef KP_XBOX
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
static u_long64 SCRPAD_WORK[16 * 1024 / sizeof(u_long64)];
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
#ifdef PAL     //BP JG - not used as we don't use the XBOX define
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
#if 0 //BP_RENDER
static void VSyncInCallback(D3DVBLANKDATA* pData)
{
	DG_PassageFrame++ ;
	DG_TickCount++ ;
	DG_VSyncStatus = 1;	// 帰線期間 // ?
}
#endif

#if 0
/* XBOX用Presendコールバック */
static void PresentCallback( D3DSWAPDATA *pData )
{
	flip_flag = 1 ;
}
#endif

/* プッシュバッファコールバック */
static void PushBufferStartCallback( int n )
{
#if 0 //BP_RENDER
   GV_PROFILE_DRAW_CLK( 0, n >> 16 );
#endif
}
static void PushBufferMarkCallback( int n )
{
#if 0 //BP_RENDER
   GV_PROFILE_DRAW_CLK( n & 0xFFFF, n >> 16 );
#endif
}

static void PushBufferEndCallback( int n )
{
#if 0 //BP_RENDER
	GV_PROFILE_DRAW_CLK( n & 0xFFFF, n >> 16 );
	GV_PROFILE_DRAW_END_CLK( n & 0xFFFF, n >> 16 );
	draw_end_flag = 1 ;
#endif
}

/* 描画マーク記録（プロファイル用） */
void DG_SetDrawMark( void )
{
#if 0 //BP_RENDER
//#ifdef DEBUG_MODE
	draw_mark_count += 1 ;
	//IDirect3DDevice8_InsertCallback( g_pd3dDevice, D3DCALLBACK_READ, PushBufferMarkCallback
	//								, draw_mark_count | ( GV_ProfileClock << 16 ) );
	IDirect3DDevice8_InsertCallback( g_pd3dDevice, D3DCALLBACK_WRITE, PushBufferMarkCallback
									, draw_mark_count | ( GV_ProfileClock << 16 ) );
//#endif
#endif
}

/* 描画開始マーク */
void DG_SetDrawStartMark( void )
{
#if 0 //BP_RENDER
   draw_mark_count = 0 ;
	draw_end_flag = 0 ;
	IDirect3DDevice8_InsertCallback( g_pd3dDevice, D3DCALLBACK_READ, PushBufferStartCallback
									, 0  | ( GV_ProfileClock << 16 ) );
#endif
}
/* 描画終了マーク */
void DG_SetDrawEndMark( void )
{
#if 0 //BP_RENDER
	draw_mark_count += 1 ;
	IDirect3DDevice8_InsertCallback( g_pd3dDevice, D3DCALLBACK_WRITE, PushBufferEndCallback
									, draw_mark_count  | ( GV_ProfileClock << 16 ) );
#endif
}

/* 描画終了待ち */
void DG_DrawSync( void )
{
#if 0 //BP_RENDER
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
	if( DG_FrameCount == 0 ){
		/* フレーム途中切替え */
		while ( DG_PassageFrame == 0 )
      {
#if 0 //BP_RENDER
			IDirect3DDevice8_BlockUntilVerticalBlank( g_pd3dDevice );	/* = VSync() */
#endif
		}
		DG_DrawSync();
	} else {
		/* 描画終了待ち */
		DG_DrawSync();
		/* VSyncまち */
		//WaitVSync( DG_FrameCount );
#if 0 //BP_RENDER
		IDirect3DDevice8_BlockUntilVerticalBlank( g_pd3dDevice );	/* = VSync() */
#endif
	}
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
	/* XBOX */
	// TODO: Present(); と DG_EndFrameの順番 ?

	DG_EndFrame();

//	DG_ResetTimer();
	//DG_Present(NULL, NULL, NULL, NULL);	     // バックバッファ表示
//	DG_FrameTimer = (int)DG_GetHSyncTimer(); // Present()にかかる時間を計測する

	GV_PROFILE_ACT_END();
#endif
}

/*----------------------------------------------------------------*/

void		DG_ResetSystem()
{
	DG_InitLightSystem() ;
	DG_InitFrameSystem() ;
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
	
	DG_InitPixelShader();         // pixel shader

	DG_AddPluginComdl();          // test
	DG_AddPluginPatch();          // test

#ifdef DEBUG_MODE
	// この時点(initステージ開始)でのメモリ使用状況を出力
	PrintGlobalMemoryStatus();
#endif
	return NULL;
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
	a = (float)( data & 0xff ) / 255.0 ;
	a = exp( gamma_inv * log( a ) ) ;
	ret = a * 255 ;
	if ( ret > 255 ) ret = 255 ;
	return ( ret );
}

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

#if 0 //BP_RENDER

	/* Direct3Dオブジェクトの生成 */
	g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (g_pD3D == NULL) {
		printf("Direct3DCreate8 Failed\n");
		exit(EXIT_FAILURE);
	}

#if 1
	{/* バッファの作成 */
#ifndef KP_WINDOWS
		VOID WINAPI XGSetTextureHeader(
									   UINT Width,
									   UINT Height,
									   UINT Levels,
									   DWORD Usage,
									   D3DFORMAT Format,
									   D3DPOOL Pool,
									   IDirect3DTexture8* pTexture,
									   UINT Data,                  // Offset to the data held by this resource
									   UINT Pitch                  // Texture pitch
									   );
		VOID WINAPI XGSetSurfaceHeader(
									   UINT Width,
									   UINT Height,
									   D3DFORMAT Format,
									   IDirect3DSurface8 *pSurface,
									   UINT Data,
									   UINT Pitch
									   );
#endif	// KP_WINDOWS
		int		i ;
		/* フレームバッファ及びＺバッファ用メモリの確保 */
		DG_BackBufferAddr[0] = D3D_AllocContiguousMemory( DISPLAY_WIDTH * DISPLAY_HEIGHT * 4 * 3, D3DTILE_ALIGNMENT );
		DG_BackBufferAddr[1] = (void*)( (char*)DG_BackBufferAddr[ 0 ] + DISPLAY_WIDTH * DISPLAY_HEIGHT * 4 ) ;
		DG_BackBufferAddr[2] = (void*)( (char*)DG_BackBufferAddr[ 1 ] + DISPLAY_WIDTH * DISPLAY_HEIGHT * 4 ) ;
		GV_ZeroMemory( DG_BackBufferAddr[0], DISPLAY_WIDTH * DISPLAY_HEIGHT * 4 * 3 );
		DG_DepthBufferAddr = D3D_AllocContiguousMemory( DISPLAY_WIDTH * DISPLAY_HEIGHT * 4, D3DTILE_ALIGNMENT );

		/* テクスチャ及びサーフェスの手動作成 */
		for ( i = 0 ; i < 3 ; i++ ){
			DG_BackBufferTexture[ i ] = &DG_BackBufferTextureMemory[ i ];
			XGSetTextureHeader( DISPLAY_WIDTH, DISPLAY_HEIGHT, 1, 0, D3DFMT_LIN_A8R8G8B8, 0, DG_BackBufferTexture[i], 0, 0 );
			IDirect3DTexture8_Register( DG_BackBufferTexture[i], DG_BackBufferAddr[i] );
			IDirect3DTexture8_GetSurfaceLevel( DG_BackBufferTexture[i], 0, &DG_BackBufferSurface[i] );
		}
		DG_DepthBufferSurface = &DG_DepthBufferSurfaceMemory ;
		XGSetSurfaceHeader( DISPLAY_WIDTH, DISPLAY_HEIGHT, D3DFMT_LIN_D24S8, DG_DepthBufferSurface, 0, 0 );
		IDirect3DSurface8_Register( DG_DepthBufferSurface, DG_DepthBufferAddr );

		DG_SystemTexture[0].tex_trans.ptex = DG_BackBufferTexture[0] ;
		DG_SystemTexture[1].tex_trans.ptex = DG_BackBufferTexture[1] ;
		DG_SystemTexture[2].tex_trans.ptex = DG_BackBufferTexture[2] ;
		DG_SystemTexture[3].tex_trans.ptex = DG_BackBufferTexture[2] ;


		/* 影用フレームバッファ及びＺバッファ用メモリの確保 */
		DG_ShadowBackBufferAddr = D3D_AllocContiguousMemory( 256 * 256 * 4, D3DTILE_ALIGNMENT );
		DG_ShadowDepthBufferAddr = D3D_AllocContiguousMemory( 256 * 256 * 4, D3DTILE_ALIGNMENT );
		/* 影用テクスチャ及びサーフェスの手動作成 */
		DG_ShadowBackBufferTexture = &DG_ShadowBackBufferTextureMemory;
		XGSetTextureHeader( 256, 256, 1, 0, D3DFMT_LIN_A8R8G8B8, 0, DG_ShadowBackBufferTexture, 0, 0 );
		//XGSetTextureHeader( 256, 256, 1, 0, D3DFMT_A8R8G8B8, 0, DG_ShadowBackBufferTexture, 0, 0 );
		IDirect3DTexture8_Register( DG_ShadowBackBufferTexture, DG_ShadowBackBufferAddr );
		IDirect3DTexture8_GetSurfaceLevel( DG_ShadowBackBufferTexture, 0, &DG_ShadowBackBufferSurface );
		DG_ShadowDepthBufferSurface = &DG_ShadowDepthBufferSurfaceMemory ;
		XGSetSurfaceHeader( 256, 256, D3DFMT_LIN_D24S8, DG_ShadowDepthBufferSurface, 0, 0 );
		//XGSetSurfaceHeader( 256, 256, D3DFMT_D24S8, DG_ShadowDepthBufferSurface, 0, 0 );
		IDirect3DSurface8_Register( DG_ShadowDepthBufferSurface, DG_ShadowDepthBufferAddr );

	}
#else
	{
		int		i ;
		/* フレームバッファ、バックバッファ及び画面退避バッファをテクスチャとして作成 */
		for ( i = 0 ; i < 3 ; i++ ){
			DG_BackBufferTexture[i] = malloc( sizeof(D3DBaseTexture) );
			IDirect3DDevice8_CreateTexture( NULL, DISPLAY_WIDTH, DISPLAY_HEIGHT, 1, 0, D3DFMT_LIN_A8R8G8B8,
										   D3DPOOL_DEFAULT, &DG_BackBufferTexture[i] );
			IDirect3DTexture8_GetSurfaceLevel( DG_BackBufferTexture[i], 0, &DG_BackBufferSurface[i] );
		}
		IDirect3DDevice8_CreateDepthStencilSurface( NULL, DISPLAY_WIDTH, DISPLAY_HEIGHT, D3DFMT_LIN_D24S8,
												   D3DMULTISAMPLE_NONE , &DG_DepthBufferSurface );
		DG_SystemTexture[0].tex_trans.ptex = DG_BackBufferTexture[0] ;
		DG_SystemTexture[1].tex_trans.ptex = DG_BackBufferTexture[1] ;
		DG_SystemTexture[2].tex_trans.ptex = DG_BackBufferTexture[2] ;
		DG_SystemTexture[3].tex_trans.ptex = DG_BackBufferTexture[2] ;
	}
#endif

	/* D3D初期化パラメータ設定 */
	/* 決め打ちのため、対応していないデバイスでは失敗する。 */
	/* 正しくはMicrosoftのサンプルを参照。 */
	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));

   g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_LIN_D24S8;
	g_d3dpp.BackBufferCount = 1;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;

	g_d3dpp.BackBufferFormat = D3DFMT_LIN_A8R8G8B8;
	g_d3dpp.BackBufferWidth = DISPLAY_WIDTH;
	g_d3dpp.BackBufferHeight = DISPLAY_HEIGHT;
#ifndef KP_WINDOWS
	g_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE_OR_IMMEDIATE ;
	g_d3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE ;
	g_d3dpp.Flags = D3DPRESENTFLAG_10X11PIXELASPECTRATIO ;
	g_d3dpp.BufferSurfaces[0] = DG_BackBufferSurface[1] ;	/* バックバッファ */
	g_d3dpp.BufferSurfaces[1] = DG_BackBufferSurface[0] ;	/* フロントバッファ */
	g_d3dpp.DepthStencilSurface = DG_DepthBufferSurface ;	/* バックバッファ */
#endif

	/* GPU転送に使用するバッファサイズ */
	/* デフォルト値はそれぞれ、512K, 32K。 */
	IDirect3D8_SetPushBufferSize( g_pD3D, 128 * 1024, 32 * 1024 );
	//IDirect3D8_SetPushBufferSize( g_pD3D, 1024 * 1024 * 4, 16 * 1024 );

	g_d3dpp.Windowed = FALSE;
	/* XBOX では hWnd = NULL */
	if (FAILED( IDirect3D9_CreateDevice( g_pD3D, 0, D3DDEVTYPE_HAL, NULL,
										D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_PUREDEVICE,
										&g_d3dpp, &g_pd3dDevice))) {
		printf("CreateDevice (on XBOX) Failed\n");
		exit(EXIT_FAILURE);
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

#if 1 /* 最終的にはいらないはず */
	{/* ビューポート指定 */
		D3DVIEWPORT9 d3dview;
		d3dview.X = 0; /* ターゲットサーフェスの左上座標 */
		d3dview.Y = 0;	
		d3dview.Width = DISPLAY_WIDTH;
		d3dview.Height = DISPLAY_HEIGHT;
		d3dview.MinZ = 0.0f; /* クリップボリュームの最小値 */
		d3dview.MaxZ = 1.0f; /* クリップボリュームの最大値 */
		IDirect3DDevice9_SetViewport( g_pd3dDevice, &d3dview );
	}
#endif
	//IDirect3DDevice8_SetShaderConstantMode( g_pd3dDevice, 1 );

	/* 影レンダリング用テクスチャの生成 */
	/* XBOXのサンプルでは D3DPOOL_MANAGED で作成しているのだが、 */
	/* windows ではD3DPOOL_DEFAULT でなければ怒られてしまう。 */
	if (FAILED( IDirect3DDevice9_CreateTexture( g_pd3dDevice,
											   256, 256, 1, D3DUSAGE_RENDERTARGET,
											   D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &DG_ShadowTexture))) {
		printf("CreateShadowTexture Failed\n");
		exit(EXIT_FAILURE);
	}

#ifndef KP_WINDOWS
	{/* バッファのタイリング設定 */
		D3DTILE	tile ;
		ZeroMemory( &tile, sizeof(tile) );
		tile.Flags = 0 ;
		tile.pMemory = DG_BackBufferAddr[0] ;
		tile.Size = DISPLAY_WIDTH * DISPLAY_HEIGHT * 4 * 3 ;
		tile.Pitch = D3DTILE_PITCH_0A00 ;
		IDirect3DDevice8_SetTile( NULL, 0, &tile );

		ZeroMemory( &tile, sizeof(tile) );
		tile.Flags = D3DTILE_FLAGS_ZBUFFER | D3DTILE_FLAGS_ZCOMPRESS ;
		tile.pMemory = DG_DepthBufferAddr ;
		tile.Size = DISPLAY_WIDTH * DISPLAY_HEIGHT * 4 ;
		tile.Pitch = D3DTILE_PITCH_0A00 ;
		IDirect3DDevice8_SetTile( NULL, 1, &tile );

#if 1
		/* 影用 */
		ZeroMemory( &tile, sizeof(tile) );
		tile.Flags = 0 ;
		tile.pMemory = DG_ShadowBackBufferAddr ;
		tile.Size = 256 * 256 * 4 ;
		tile.Pitch = D3DTILE_PITCH_0400 ;
		IDirect3DDevice8_SetTile( NULL, 2, &tile );

		ZeroMemory( &tile, sizeof(tile) );
		tile.Flags = D3DTILE_FLAGS_ZBUFFER /*| D3DTILE_FLAGS_ZCOMPRESS*/ ;
		tile.pMemory = DG_ShadowDepthBufferAddr ;
		tile.Size = 256 * 256 * 4 ;
		tile.Pitch = D3DTILE_PITCH_0400 ;
		IDirect3DDevice8_SetTile( NULL, 3, &tile );
#endif
	}
#endif	/* KP_WINDOWS */
#endif //BP

	DG_Clock = 0 ;

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

	/* 割り込み関連初期化 */
#if 0 //BP_RENDER
	IDirect3DDevice8_SetVerticalBlankCallback( g_pd3dDevice, VSyncInCallback );
	//IDirect3DDevice8_SetSwapCallback( g_pd3dDevice, PresentCallback );
#endif

	/*
		ＤＧライブラリデーモンは、２プロセス使用
	*/
	GV_InitActor(GV_ACTOR_DAEMON, &Work1, NULL);  // フレーム開始プロセス
	GV_SetActor(&Work1, Act1, NULL);
	GV_InitActor(GV_ACTOR_DAEMON2, &Work2, NULL); // フレーム終了プロセス
	GV_SetActor(&Work2, Act2, NULL);

	NewResidentShader( 0, 0 );
}


/* Direct3D関係のリソースを全て解放 */
void DG_KillDaemon(void)
{
	if (DG_ShadowTexture) 
   {
		IDirect3DTexture9_Release( DG_ShadowTexture );
	}

//	DG_FreeTextureAll();
	//DG_ReleaseParticleVertexBuffer();
	DG_ReleasePatchVertexBuffer();
	DG_ReleaseKmsVertexBuffer();
	
	if (g_pD3D) {
		IDirect3D9_Release( g_pD3D );
	}
	if (g_pd3dDevice) {
		IDirect3DDevice9_Release( g_pd3dDevice );
	}
}

/* ---------------------------------------------------------------- */
void DG_SetDisplayOffset( int x, int y, int flag )
{
}

#endif
