/*
	xframe.c
	フレームパラメータ管理ルーチン

	2002/02/07 K.Takabe
	$Id: wframe.c,v 1.38 2002/12/26 11:07:26 Yoshizawa1 Exp $

*/
/*

	void		DG_SetDispEnv( x, y, w, h, shift )
	int		x, y, w, h ;	基本表示位置
	int		shift ;		水平シフト量

		表示エリアを設定する

	void		DG_InitFrameSystem()

		フレーム開始／終了サイクルを初期化する

	void		DG_StartFrame()
	
		フレーム開始処理

	void		DG_EndFrame()

		フレーム終了処理

	------------------------------------------------

	void DG_SetFogColor( int r, int g, int b )
	int			r ;		
	int			g ;		
	int			b ;		

		フォグカラーを設定する


	void DG_SetFogParam( float near, float far )
	float		near ;		フォグのかかりはじめる距離
	float		far ;		完全にフォグに溶け込む距離
		フォグ係数を設定する


	------------------------------------------------

	void		DG_MakeCameraMatrix( FMATRIX *mat, FVECTOR *from, FVECTOR *to )
	FMATRIX		*cam_mat ;	計算したカメラマトリクス
	FVECTOR		*from ;		カメラ位置
	FVECTOR		*to ;		カメラ注目点

		カメラ位置とカメラ注目点からカメラマトリクスを生成する

	void		DG_SetCamera( DG_CHANL *chanl, FMATRIX *mat, float screen )
	DG_CHANL	*chanl ;	チャンネル構造体
	FMATRIX		*cam_mat ;	カメラ注目点
	float		screen ;	スクリーン距離

		指定したマトリクスでカメラを設定する

	void		DG_SetCamera2( chanl, from, to, screen )
	DG_CHANL	*chanl ;	チャンネル構造体
	FVECTOR		*from ;		カメラ注目点
	FVECTOR		*to ;		カメラ位置
	float		screen ;	スクリーン距離

		カメラ行列を、注目点／カメラ位置でセッティング

	------------------------------------------------

	void		DG_FrameChanl( DG_CHANL *cp, int which )
	DG_CHANL	*chanl ;	チャンネル構造体
	int			which ;		バッファ選択

		フレーム初期化パケット接続チャンネル

	------------------------------------------------

	void	DG_AddCurrentGroup( int id )
	int		id ;		表示に追加するグループＩＤ

	指定したグループの表示をＯＮにする（メインチャンネルに対して）


	void	DG_DeleteCurrentGroup( int id )
	int		id ;		表示から削除するグループＩＤ

	指定したグループの表示をＯＦＦにする（メインチャンネルに対して）


	void	DG_AddCurrentGroup2( int chanl, int id )
	void	DG_DeleteCurrentGroup2( int chanl, int id )

	上記関数のチャンネル指定バージョン

	------------------------------------------------

	以下、主にＤＧライブラリ内部で使用する

	//void		DG_SetWindow( window, screen )
	//RECT		*window ;	クリップ領域
	//int		screen ;	スクリーン距離

	//	クリップ領域／スクリーン距離をセットする

*/
// NOTE: 各チャンネルに格納されるカメラパラメータについて
//
//
//  FMATRIX eye_pers;           /* カメラ透視変換マトリクス（=pers*eye_inv）*/
//  FMATRIX eye_inv;            /* カメラ逆行列 */
//  FMATRIX eye;                /* カメラ行列 */
//  FMATRIX pers;               /* 透視変換マトリクス */
//  FMATRIX eye_pers2;          /* カメラ透視変換マトリクス（クリップなし計算用）（現在未使用） */
//  FMATRIX pers2;              /* 透視変換マトリクス（クリップなし計算用）（現在未使用） */
//  FMATRIX raise_pers;         /* 透視変換マトリクス */
//  FMATRIX raise_pers2;        /* 透視変換マトリクス（クリップなし計算用）（現在未使用） */
//  FMATRIX raise_eye_pers;     /* カメラ透視変換マトリクス */
//  FMATRIX raise_eye_pers2;    /* カメラ透視変換マトリクス（クリップなし計算用）（現在未使用） */
//  float   screen;             /* カメラ→スクリーンキョリ（default = 2.0） */
//
//  DirectX8では以下のように呼ばれているもの。
//  ・カメラ行列   → ビュー行列 (D3DXでは、D3DXMatrixLookAtLH)
//  ・透視変換行列 → 射影行列   (D3DXでは、D3DXMatrixPerspectiveLH
//                                           or D3DXMatrixPerspectiveOffCenterLH)
//  

#ifndef _XBOX
#include <windows.h>
#include <d3dx8.h>
#else
#include <xtl.h>
#ifndef _WINDOWS
#include <d3d8perf.h>
#endif
#endif

#include "libgv.h"
#include "libdg.h"
#include "dgconf.h"
#include "shader.h"
#include "wdgperf.h"
#include "wexebuffer.h"

#define	__FPS_TEST__	(TRUE)		// FPSの表示テスト
#define	__DGPERF_TEST__	(TRUE)		// 描画パフォーマンスの表示テスト

#if __FPS_TEST__
#include "game.h"

static	DWORD			_fps_cntr = 0 ;
static	LARGE_INTEGER	_fps_timer_freq ;
static	LARGE_INTEGER	_fps_pre_timer ;
#endif	/* __FPS_TEST__ */

#if __DG_DRAW_IN_DRAWTHREAD__	// Threadを作って描画

static DWORD	_draw_thread_id ;
static HANDLE	_draw_thread_handle = NULL ;
#endif // __DG_DRAW_IN_DRAWTHREAD__
/*----------------------------------------------------------------*/

	/*
		バッファ切り替えクロック
	*/
int		DG_Clock = 0 ;

	/*
		各種画面表示状態
	*/
int		DG_DisplayStatus = 0 ;

	/*
		引きつりフラグ
	*/
int		DG_SkipFrame = 1 ;
int		DG_DelayMode = 0 ;

	/*
		フィールド情報
	*/
int		DG_Field ;
int		DG_CurrentField;

	/*
		表示グループ
	*/
int		DG_CurrentGroupID ;

	/*
		フォグ関連パラメータ
	*/
CVECTOR	DG_FogColorMaster = {0,0,0,0xff};
CVECTOR	DG_FogColor = {0,0,0,0xff};
FVECTOR DG_FogParam = {0.0f, 255.0f, 1.0f / 255.0f, 1.0f};
float	DG_FogParam1 = 0.0f ;
float	DG_FogParam2 = 255.0f;
float	DG_FogStart ;
float	DG_FogEnd ;

	/*
		クリップ設定
	*/
float	DG_ClipNear = DRAW_NEAR_CLIP ;
float	DG_ClipFar = DRAW_FAR_CLIP ;

	/*
		現在アクティブなビューポート設定（システム内で使用）
	*/
DG_VIEWPORT DG_CurrentViewport;

/* 現在描画を行っているバックバッファ番号 */
int		DG_CurrentBackBuffer = 0 ;

	/* BackBuffer設定 */
extern DWORD				DG_BackBufferWidth ;		// BackBufferサイズ
extern DWORD				DG_BackBufferHeight ;
extern DWORD				DG_BackBufferRendWidth ;	// 描画に使用する範囲
extern DWORD				DG_BackBufferRendHeight ;
extern float				DG_BackBufferWidthAdjustCoef ;	// RendWidth/Width
extern float				DG_BackBufferHeightAdjustCoef ;// RendHeight/Height
extern LPDIRECT3DTEXTURE8	DG_BackBufferTexture[3] ;
extern LPDIRECT3DSURFACE8	DG_BackBufferSurface[3] ;
extern LPDIRECT3DSURFACE8	DG_BackBufferDepthSurface ;
extern BOOL					DG_RendToBackBuffer ;		// BackBuffer描画処理中フラグ
extern DWORD				DG_FrameSkipCount ;			// FrameSkip数(0～)
extern DWORD				DG_FrameSkipRemainCount ;	// 残りFrameSkip数

	/*
		表示制御
	*/
DWORD				DG_SkipRendBackBuffer2Frame     = 0 ;
DWORD				DG_SkipRendBackBuffer2FrameCntr = 0 ;
/*----------------------------------------------------------------*/

#ifndef	NO_PROTOTYPE
#endif

static	void	DG_DrawUnDrawFrame(int which) ;

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
void DG_SetFogColor( int r, int g, int b )
{
	DG_FogColorMaster.r = r ;
	DG_FogColorMaster.g = g ;
	DG_FogColorMaster.b = b ;

	/* Xbox: DirectXにフォグカラーを設定する */
	DG_SetDxFogColor();
}	

void DG_SetFogParam( float var_near, float var_far )
{
#if 0
	DG_FogParam1 = 255.0f / ( var_near - var_far ) ;
	DG_FogParam2 = 255.0f - DG_FogParam1 * var_near ;
//	DG_ClipFar = var_far ;
#else
	float	param1, param2 ;

	/* ＸＢＯＸの頂点シェーダー用パラメータ計算 */
	DG_FogStart = var_near ;
	DG_FogEnd   = var_far ;
	param1 = - 1.0f / ( var_near - var_far ) ;
	param2 = -param1 * var_near ;
	DG_FogParam.x = param1;
	DG_FogParam.y = param2;
	DG_FogParam.z = 1.0f / 255.0f;
	DG_FogParam.w = 1.0f;

	/* 互換性に問題があったのでこの値はそのまま残しておく */
	DG_FogParam1 = 255.0f / ( var_near - var_far ) ;
	DG_FogParam2 = 255.0f - DG_FogParam1 * var_near ;
#endif
}	

void DG_SetClipParam( float var_near, float var_far )
{
	DG_ClipNear = var_near ;
	DG_ClipFar = var_far ;
}

/*----------------------------------------------------------------*/

void		DG_InitFrameSystem()
{
	static FVECTOR cam_def_from = {0,0,0,1};
	static FVECTOR cam_def_to = {0,0,1,1};
	DG_ClearChanlSystem( 1- DG_Clock ) ;
	DG_ClearChanlSystem( DG_Clock ) ;
	//DG_ResetTextureTmpClut() ;
	DG_SetFogColor( 0, 0, 0 );
	DG_SetFogParam( 20*65536.0f, 21*65536.0f );
	DG_SetClipParam( DRAW_NEAR_CLIP, DRAW_FAR_CLIP );
	DG_DisplayStatus = 0 ;
   BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
	DG_SetCamera2( DG_Chanl(0), &cam_def_from, &cam_def_to, 0.0f );
	DG_SetCamera2( DG_Chanl(1), &cam_def_from, &cam_def_to, 0.0f );

	DG_SkipRendBackBuffer2Frame     = 0 ;
	DG_SkipRendBackBuffer2FrameCntr = 0 ;
}

long DG_UnDrawFrameCount=0;
static	BOOL DG_UnDrawFrame1st=TRUE;

#ifdef _WINDOWS
#ifdef DEBUG_MODE
char	DG_UnDrawFrameCountSetInfo[256] = "None";
#endif
#endif

int DG_LastWhich = -1;

extern volatile	DWORD	_dipswitch ;

#if __DG_DRAW_IN_DRAWTHREAD__
void DG_DrawFrame( void )
{
	int		which ;

	which = DG_Clock ;

	/*-- Perf --*/
#if __DGPERF_TEST__
	if( _dipswitch & (1 << 0) ){ DG_DrawPerf() ; }
#endif
	DG_PerfResetDrawDebug() ;
	DG_PerfReset(DGPERF_CTGR_DRAWALL) ;

	/*-- Test --*/
#if __FPS_TEST__
	{
		if( _fps_cntr == 0 )
		{
			QueryPerformanceFrequency(&_fps_timer_freq) ;
		}

		_fps_cntr++ ;
		if( _fps_cntr > 60 )
		{
			LARGE_INTEGER	timer ;
			LARGE_INTEGER	stimer ;
			float			fps_val ;

			QueryPerformanceCounter(&timer) ;
		
			stimer.QuadPart = timer.QuadPart - _fps_pre_timer.QuadPart ;
			_fps_pre_timer = timer ;
			if(  (_dipswitch & (1 << 1)) 
			  && (stimer.QuadPart > 0) )
			{
				fps_val = (float)_fps_timer_freq.QuadPart/(float)stimer.QuadPart ;	

				DEBUG_Locate( 64, 240, 0 );
				DEBUG_Printf( "FPS:%7.3f[Skip:%d]\n", fps_val, DG_FrameSkipCount) ;
			}
		}
	}
#endif
	/*----------*/

	/*-- 描画処理 --*/

	DG_PerfStart(DGPERF_CTGR_DRAWALL) ;		// 描画時間総計計測開始

	DG_PerfStartD(DGPERF_CTGR_PRESENT) ;	// 計測開始
	DG_Present(NULL, NULL, NULL, NULL);	     /* バックバッファ表示 */
	DG_PerfEndD(DGPERF_CTGR_PRESENT) ;		// 計測一旦停止
	/*----------*/

	/* スタティックプッシュバッファのつかえないＰＣではこうなるのかも・・・ */

	DG_SetDrawStartMark();
	DG_BeginScene() ;
	DG_SortChanlSystem( 1- which ); /* 計算&描画をまとめて実行 */
	DG_RendBackBuffer2Frame() ;		/* 描画結果をFrameBufferにレンダリング */
	DG_EndScene() ;
	DG_SetDrawEndMark();

	DG_PerfEnd(DGPERF_CTGR_DRAWALL) ;

	DG_LastWhich = - 1;
}
#endif

#if __DG_DRAW_IN_DRAWTHREAD__
static DWORD WINAPI	DG_DrawFrameThread(LPVOID arg)
{
	while( TRUE )
	{
		WaitForSingleObject(DG_RequestDrawEventHandle, INFINITE) ;	// 描画開始待ち
		ResetEvent(DG_RequestDrawEventHandle) ;

		DG_DrawFrame() ;					// 描画

		SetEvent(DG_DrawDoneEventHandle) ;	// 描画終了通達
	}
}
#endif

void		DG_StartFrame()
{
	int		which ;
#if 1
	GV_PROFILE_FLIP();
	GV_PROFILE_START( GV_PROFILE_DRAW_START );
	GV_PROFILE_START( GV_PROFILE_ACT_START );
#endif

	DG_Clock = 1 - DG_Clock ;
	which = DG_Clock ;

	/* 非描画処理 */
	if( DG_UnDrawFrameCount > 0 ){
		if( DG_LastWhich < 0 ){
			DG_LastWhich = which;
		}
		DG_UnDrawFrameCount--;
		DG_UnDrawChanlSystem( 1 - which );

		if( DG_UnDrawFrame1st )
		{
			DG_DrawUnDrawFrame(which) ;
			DG_UnDrawFrame1st = FALSE ;
		}

#ifdef DEBUG_MODE
#if FALSE
		printf("Skip by DG_UnDrawFrameCount![%d] %s\n",
					(DWORD)DG_UnDrawFrameCount,
					DG_UnDrawFrameCountSetInfo) ;
#endif
#endif
	    goto Skip;
	}
	DG_UnDrawFrame1st = TRUE;

#ifndef _WINDOWS
	if( DG_LastWhich >= 0 ){
		/* ＸＢＯＸでは前のフレームで発行した描画コマンドをキャンセルできないため */
		/* 前のフレームをコピーすることで描画を反映させないようにする */
		IDirect3DDevice8_CopyRects( g_pd3dDevice,
								   DG_BackBufferSurface[ DG_CurrentBackBuffer ], NULL, 0,
								   DG_BackBufferSurface[ 1 - DG_CurrentBackBuffer ], NULL );
	}
#endif

	/* FlameSkip中断処理 */
	if( DG_FrameSkipRemainCount )
	{
		if( DG_CheckSkipFrameCancelOK() )
		{
			DG_FrameSkipCount      -= DG_FrameSkipRemainCount ;
			DG_FrameSkipRemainCount = 0 ;
		}
	}

	/* 以降描画処理 */
	DG_PerfEnd(DGPERF_CTGR_PROCESS_ALL) ;	// 計測一旦停止

	if(  !DG_FrameSkipRemainCount		// フレームスキップ中ではない
	  && DG_CheckAppDisplay() )			// 実際にWindowアプリは表示中
	{
#if __DG_DRAW_IN_MAIN_LOOP__	// Main Loop中で描画
		SetEvent(DG_RequestDrawEventHandle) ;	// 描画開始要求

		WaitForSingleObject(DG_DrawDoneEventHandle, INFINITE) ;	// 描画終了待ち
		ResetEvent(DG_DrawDoneEventHandle) ;	

#else
#if __DG_DRAW_IN_DRAWTHREAD__	// Threadを作って描画

		/*-- Thread作成 --*/
		if( !_draw_thread_handle )
		{
			_draw_thread_handle = CreateThread( 0, 0, &DG_DrawFrameThread,
										0, 0, &_draw_thread_id) ;
			SetEvent(DG_DrawDoneEventHandle) ;
		}

		/*-- 前回の描画終了待ち --*/
		WaitForSingleObject(DG_DrawDoneEventHandle, INFINITE) ;
		ResetEvent(DG_DrawDoneEventHandle) ;

		/*-- 描画開始要求 --*/
		SetEvent(DG_RequestDrawEventHandle) ;	// 描画開始要求

		/*-- Test描画終了待ち --*/
#if TRUE
		WaitForSingleObject(DG_DrawDoneEventHandle, INFINITE) ;
		ResetEvent(DG_DrawDoneEventHandle) ;
		SetEvent(DG_DrawDoneEventHandle) ;
#endif
#else
							// ここで描画
		/*-- Perf --*/
#if __DGPERF_TEST__
		if( _dipswitch & (1 << 0) ){ DG_DrawPerf() ; }
#endif
		DG_PerfResetDrawDebug() ;
#if !__DG_DRAW_IN_EXEBUFFER__
		DG_PerfReset(DGPERF_CTGR_DRAWALL) ;
#endif

		/*-- Test --*/
#if __FPS_TEST__
		{
			if( _fps_cntr == 0 )
			{
				QueryPerformanceFrequency(&_fps_timer_freq) ;
			}

			_fps_cntr++ ;
			if( _fps_cntr > 60 )
			{
				LARGE_INTEGER	timer ;
				LARGE_INTEGER	stimer ;
				float			fps_val ;

				QueryPerformanceCounter(&timer) ;
		
				stimer.QuadPart = timer.QuadPart - _fps_pre_timer.QuadPart ;
				_fps_pre_timer = timer ;
				if(  (_dipswitch & (1 << 1)) 
				  && (stimer.QuadPart > 0) )
				{
					fps_val = (float)_fps_timer_freq.QuadPart/(float)stimer.QuadPart ;	

					DEBUG_Locate( 64, 240, 0 );
					DEBUG_Printf( "FPS:%7.3f[Skip:%d]\n", fps_val, DG_FrameSkipCount) ;
				}
			}
		}
	
		if( _dipswitch & (1 << 2) ){ SoundDebugOutYoshizawa() ; }
		if( _dipswitch & (1 << 3) ){ SoundDebugOutYoshizawa2() ; }
#endif
		/*----------*/

#if __DG_DRAW_IN_EXEBUFFER__
		DG_FlipExeBuffer() ;
#endif
		/*-- 描画処理 --*/

		DG_PerfStart(DGPERF_CTGR_DRAWALL) ;		// 描画時間総計計測開始
		DG_PerfStartD(DGPERF_CTGR_PRESENT) ;	// 計測開始

		/*-- UnDrawFrameの直後はPresent処理をしない --------*/

		if( DG_LastWhich < 0 )
		{
			HRESULT	hr ;

			hr = DG_Present(NULL, NULL, NULL, NULL);	     /* バックバッファ表示 */
			if( FAILED(hr) ){ goto ResetAndClear ; }
		}
		else
		{
			/* BackBuffer切替処理のみ */
			DG_EndCurrentBackBuffer() ;	// 現在のBackBuffer使用終了処理
			DG_FlipBackBuffer() ;
		}
		/*--------------------------------------------------*/

		DG_PerfEndD(DGPERF_CTGR_PRESENT) ;		// 計測停止
#if __DG_DRAW_IN_EXEBUFFER__
		DG_PerfEnd(DGPERF_CTGR_DRAWALL) ;		// 描画時間総計計測開始
#endif
		/*----------*/

		/*-- 描画バッファと表示バッファの切替制御 ----------*/

		DG_SkipRendBackBuffer2FrameCntr = DG_SkipRendBackBuffer2Frame ;
		/*--------------------------------------------------*/

		/* スタティックプッシュバッファのつかえないＰＣではこうなるのかも・・・ */

#if !__DG_DRAW_IN_EXEBUFFER__
		DG_SetDrawStartMark();
		DG_SortChanlSystem( 1- which ); /* 計算&描画をまとめて実行 */
		DG_RendBackBuffer2Frame() ;		/* 描画結果をFrameBufferにレンダリング */
		DG_SetDrawEndMark();
#else

		DG_PerfStart(DGPERF_CTGR_DRAWALL) ;
		DG_SortChanlSystem( 1- which ); /* 計算&描画をまとめて実行 */
		DG_RendBackBuffer2Frame() ;		/* 描画結果をFrameBufferにレンダリング */
		DG_PerfEnd(DGPERF_CTGR_DRAWALL) ;

#if FALSE
		DG_PerfStart(DGPERF_CTGR_DRAWALL) ;
		DG_SetDrawStartMark();
		DG_BeginScene() ;
		DG_RunExeBuffer() ;
		DG_EndScene() ;
		DG_SetDrawEndMark();
		DG_PerfEnd(DGPERF_CTGR_DRAWALL) ;
#else
		DG_KickExeBuffer() ;	// Thread動作

#endif

#endif
#if !__DG_DRAW_IN_EXEBUFFER__
		DG_PerfEnd(DGPERF_CTGR_DRAWALL) ;
#endif
		DG_LastWhich = - 1;
#endif
#endif

		DG_FramePresentCount++ ;	// Frame切替カウンタ増加
		goto ResetAndClear;
	}

Skip:
	//GV_PROFILE_FLIP();
	//GV_PROFILE_START( GV_PROFILE_DRAW_START );
	//GV_PROFILE_START( GV_PROFILE_ACT_START );

ResetAndClear:
	DG_PerfStart(DGPERF_CTGR_PROCESS_ALL) ;	// 計測再開

	//DG_ResetDynamicVertexBuffer( which );	/* 動的頂点バッファのリセット */
	DG_ResetMemorySystem( which );	/* ＤＧ用バッファメモリのリセット */
	DG_ClearTmpLight();            // 一時光源バッファのクリア
	DG_ClearChanlSystem(which);

}

void		DG_EndFrame()
{
	int		which ;
	int		start_addr, end_addr ;

	/*
		オブジェクトのセットアップ＆ＤＭＡ登録
	*/
	which = DG_Clock ;

	/* 赤外線ゴーグル対応処理 */
	DG_FogColor = DG_FogColorMaster ;
	if ( DG_DisplayStatus & DG_STATE_IR_MODE ){
		DG_FogColor.r >>= 1 ;
		DG_FogColor.g >>= 1 ;
		DG_FogColor.b >>= 1 ;
	}

	/* 一時ＣＬＵＴバッファの設定 */
	//DG_SetTextureTmpClut( which );
	/* ライト変更に伴う再プリシェード（チャンネル０のオブジェクトに対してのみ） */
	DG_LightReshadeChanl( DG_Chanl( 0 ), which );
	/* プリシェード更新（チャンネル０のオブジェクトに対してのみ） */
	DG_TmpLightPreshadeChanl( DG_Chanl( 0 ), which );
	/* 頂点アニメの復元（チャンネル０のオブジェクトに対してのみ） */
	DG_RefreshVAnimeChanl( DG_Chanl( 0 ), which );

	/* 描画（１フレーム分のディスプレイリスト作成） */
#ifndef _WINDOWS
	if ( DG_UnDrawFrameCount == 0 ){
		DG_SortChanlSystem( which );
	}
#endif

	/* 一時ＣＬＵＴバッファのリセット */
	//DG_ResetTextureTmpClut();

}

/*----------------------------------------------------------------*/
#ifndef _WINDOWS
static FVECTOR prev_vx;
#else
static FVECTOR prev_vx = { 0.0F, 0.0F, 0.0F, 1.0F };
#endif
void		DG_MakeCameraMatrix( FMATRIX *mat, FVECTOR *from, FVECTOR *to )
{
	// ※PS2版とY軸方向を逆にする
	//static	FVECTOR	Lower = { 0.0F, 1.0F, 0.0F, 1.0F } ;
	static	FVECTOR	Lower = { 0.0F, -1.0F, 0.0F, 1.0F } ;		/* ＰＳ２版と同じ */
#ifndef _WINDOWS
	FVECTOR		z_vec, y_vec, x_vec ;
#else
	FVECTOR		z_vec = { 0.0F, 0.0F, 0.0F, 1.0F } ;	// NAN対策
	FVECTOR		y_vec = { 0.0F, 0.0F, 0.0F, 1.0F } ;
	FVECTOR		x_vec = { 0.0F, 0.0F, 0.0F, 1.0F } ;
#endif

	mat->m[3][ 0 ] = from->vx ;
	mat->m[3][ 1 ] = from->vy ;
	mat->m[3][ 2 ] = from->vz ;
	mat->m[3][ 3 ] = 1.0 ;
	/*
		カメラ行列を計算する
		Ｚ軸は、「カメラ位置→注目点」ベクトルを正規化
		Ｘ軸は、「下向きベクトル」×Ｚ軸の結果を正規化
		Ｙ軸は、Ｚ軸×Ｘ軸
	*/
	z_vec.vx = ( to->vx - from->vx ) ;
	z_vec.vy = ( to->vy - from->vy ) ;
	z_vec.vz = ( to->vz - from->vz ) ;
	_sceVu0OuterProduct( &x_vec, &Lower, &z_vec ) ;
	if( x_vec.vx == 0.0F && x_vec.vy == 0.0F && x_vec.vz == 0.0F ){
		x_vec = prev_vx;
	} else {
		prev_vx = x_vec;
	}

	_sceVu0Normalize( &x_vec, &x_vec ) ;
	_sceVu0Normalize( &z_vec, &z_vec ) ;
	_sceVu0OuterProduct( &y_vec, &z_vec, &x_vec ) ;
	mat->m[ 0 ][ 0 ] = x_vec.vx ;
	mat->m[ 0 ][ 1 ] = x_vec.vy ;
	mat->m[ 0 ][ 2 ] = x_vec.vz ;
	mat->m[ 0 ][ 3 ] = 0.0F ;
	mat->m[ 1 ][ 0 ] = y_vec.vx ;
	mat->m[ 1 ][ 1 ] = y_vec.vy ;
	mat->m[ 1 ][ 2 ] = y_vec.vz ;
	mat->m[ 1 ][ 3 ] = 0.0F ;
	mat->m[ 2 ][ 0 ] = z_vec.vx ;
	mat->m[ 2 ][ 1 ] = z_vec.vy ;
	mat->m[ 2 ][ 2 ] = z_vec.vz ;
	mat->m[ 2 ][ 3 ] = 0.0F ;

}
/* sceVu0ViewScreenMatrix 互換 */
void DG_ViewScreenMatrix(
		FMATRIX *res,
		float scrz,              /* スクリーンまでの距離 */
		float ax, float ay,      /* アスペクト比 */
		float cx, float cy,      /* スクリーンの中心座標 */
		float zmin, float zmax,  /* Zバッファ最小値/最大値 */
		float nearz, float farz) /* ニアクリップ面のZ,ファークリップ面のZ */
{
	float	az, cz;
	FMATRIX  m, mt;

	cz = (- zmax * nearz + zmin * farz) / (farz - nearz);
	az  = farz * nearz * (- zmin + zmax) / (farz - nearz);

	//     | scrz    0  0 0 |
	// m = |    0 scrz  0 0 | 
	//     |    0    0  0 1 |
	//     |    0    0  1 0 |
	_sceVu0UnitMatrix( &m );
	m.m[0][0] = scrz; // スクリーンまでの距離
	m.m[1][1] = scrz;
	m.m[2][2] = 0.0f;
	m.m[3][3] = 0.0f;
	m.m[3][2] = 1.0f;
	m.m[2][3] = 1.0f;

	//      | ax  0  0 cx |
	// mt = |  0 ay  0 cy | 
	//      |  0  0 az cz |
	//      |  0  0  0  1 |
	_sceVu0UnitMatrix( &mt );
	mt.m[0][0] = ax;
	mt.m[1][1] = ay;
	mt.m[2][2] = az;
	mt.m[3][0] = cx;
	mt.m[3][1] = cy;
	mt.m[3][2] = cz;

	_sceVu0MulMatrix( res, &mt, &m );
}

#if 0
/* 逆行列算出（正規化回転マトリクスのみ対応） */
static void InversMatrix( FMATRIX *res, FMATRIX *mat )
{
	FMATRIX		m ;
	FVECTOR		v ;
	_sceVu0UnitMatrix( &m );
	m.m[0][0] = mat->m[0][0] ;
	m.m[1][1] = mat->m[1][1] ;
	m.m[2][2] = mat->m[2][2] ;
	m.m[0][1] = mat->m[1][0] ;
	m.m[0][2] = mat->m[2][0] ;
	m.m[1][0] = mat->m[0][1] ;
	m.m[1][2] = mat->m[2][1] ;
	m.m[2][0] = mat->m[0][2] ;
	m.m[2][1] = mat->m[1][2] ;
	v.vx = - mat->m[3][0] ;
	v.vy = - mat->m[3][1] ;
	v.vz = - mat->m[3][2] ;
	v.vw = 1.0f ;
	_sceVu0ApplyMatrix( &m.m[3][0], &m, &v );
}
#endif

/* カメラマトリクスを用いてカメラを設定 */
void		DG_SetCamera( DG_CHANL *chanl, FMATRIX *mat, float screen )
{
	chanl->screen = screen ;
	chanl->eye = *mat ;
	//_sceVu0InversMatrix( &chanl->eye_inv, &chanl->eye );
	DG_InversMatrix( &chanl->eye_inv, &chanl->eye );

	/*
		透視変換行列を生成する
	*/
	DG_ViewScreenMatrix( &chanl->pers,
						chanl->screen,
						ASPECT_X(), ASPECT_Y() * chanl->width / chanl->height, /* アスペクト比 */
						0.0F, 0.0F, /* ＸＹオフセット値 */
						(float)-1.0F, (float)1.0F, /* 変換後Ｚ範囲 */
						(float)DG_ClipNear, (float)DG_ClipFar); /* Ｚクリップ */
	DG_ViewScreenMatrix( &chanl->raise_pers,
						chanl->screen,
						ASPECT_X(), ASPECT_Y() * chanl->width / chanl->height, /* アスペクト比 */
						0.0F, 0.0F, /* ＸＹオフセット値 */
						(float)-1.0F, (float)1.0F, /* 変換後Ｚ範囲 */
						(float)DG_ClipNear+0.05f, (float)DG_ClipFar+0.0f); /* Ｚクリップ */
	/* DirectX用マトリクスも生成する */
	DG_ViewScreenMatrix( &chanl->xpers,
						chanl->screen,
						ASPECT_X(), -ASPECT_Y() * chanl->width / chanl->height, /* アスペクト比 */
						0.0F, 0.0F, /* ＸＹオフセット値 */
						//(float)-1.0F, (float)1.0F, /* 変換後Ｚ範囲 */
						(float)0.0F, (float)1.0F, /* 変換後Ｚ範囲 */
						(float)DG_ClipNear, (float)DG_ClipFar); /* Ｚクリップ */
	DG_ViewScreenMatrix( &chanl->raise_xpers,
						chanl->screen,
						ASPECT_X(), -ASPECT_Y() * chanl->width / chanl->height, /* アスペクト比 */
						0.0F, 0.0F, /* ＸＹオフセット値 */
						//(float)-1.0F, (float)1.0F, /* 変換後Ｚ範囲 */
						(float)0.0F, (float)1.0F, /* 変換後Ｚ範囲 */
						(float)DG_ClipNear+0.05f, (float)DG_ClipFar+0.0f); /* Ｚクリップ */

	/*
		カメラ透視変換行列を生成する
	*/
	_sceVu0MulMatrix( &chanl->eye_pers, &chanl->pers, &chanl->eye_inv );
	_sceVu0MulMatrix( &chanl->raise_eye_pers, &chanl->raise_pers, &chanl->eye_inv );
	_sceVu0MulMatrix( &chanl->eye_xpers, &chanl->xpers, &chanl->eye_inv );
	_sceVu0MulMatrix( &chanl->raise_eye_xpers, &chanl->raise_xpers, &chanl->eye_inv );
#if 0
	_sceVu0MulMatrix( &chanl->eye_pers2, &chanl->pers2, &chanl->eye_inv );
	_sceVu0MulMatrix( &chanl->raise_eye_pers2, &chanl->raise_pers2, &chanl->eye_inv );
#endif

}
void		DG_SetCamera2( DG_CHANL *chanl, FVECTOR *from, FVECTOR *to, float screen )
{
	FMATRIX		mat, mat2 ;

#ifdef _XBOX
	if( from->vx == to->vx && from->vy == to->vy && from->vz == to->vz ){
		return;
	}
#endif

	DG_MakeCameraMatrix( &mat, from, to );
	/*
		カメラ逆行列を計算する
	*/
	//_sceVu0InversMatrix( &mat2, &mat );
	DG_InversMatrix( &mat2, &mat );

	/* Ｚ回転 */
	_sceVu0RotMatrixZ( &mat2, &mat2, from->vw ) ;
	//_sceVu0InversMatrix( &mat, &mat2 ) ;
	DG_InversMatrix( &mat, &mat2 ) ;
	/* カメラ設定 */
	DG_SetCamera( chanl, &mat, screen );

}
/*----------------------------------------------------------------*/

	/*
		フレーム初期化パケット接続チャンネル
	*/
void		DG_FrameChanl( DG_CHANL *cp, int which )
{
	MARK( "frame.c" );
	DG_PerfStartD(DGPERF_CTGR_FRAME_START) ;

	/*
		初期化パケットの設定
	*/

	/* スタティックプッシュバッファへの記録開始 */
	DG_OpenDmaTask();

	/* 描画終了待ち */
	
#if 1 /* 最終的にはいらないはず */
	{/* ビューポート指定 */

#ifndef _WINDOWS
		D3DVIEWPORT8 d3dview;
		ZeroMemory( &d3dview, sizeof(D3DVIEWPORT8) );
		//d3dview.X = DRAW_WIDTH / 2 + cp->offset_x - cp->width / 2 ; /* ターゲットサーフェスの左上座標 */
		//d3dview.Y = DRAW_HEIGHT / 2 + cp->offset_y - cp->height / 2 ;	
		d3dview.X = DISPLAY_WIDTH / 2 + cp->offset_x - cp->width / 2 ; /* ターゲットサーフェスの左上座標 */
		d3dview.Y = DISPLAY_HEIGHT / 2 + cp->offset_y - cp->height / 2 ;	
		d3dview.Width = cp->width ;
		d3dview.Height = cp->height ;
		d3dview.MinZ = 0.0f; /* クリップボリュームの最小値 */
		d3dview.MaxZ = 1.0f; /* クリップボリュームの最大値 */

		DG_SetViewport( &d3dview );
#else

		DG_VIEWPORT	viewport;

		viewport.lx = (((float)DISPLAY_WIDTH - (float)cp->width) / 2.0f)
				+ (float)cp->offset_x ;
		viewport.ly = (((float)DISPLAY_HEIGHT - (float)cp->height) / 2.0f)
				+ (float)cp->offset_y ;
		viewport.width  = (float)cp->width ;
		viewport.height = (float)cp->height ;
		viewport.min_z  = 0.0f; /* クリップボリュームの最小値 */
		viewport.max_z  = 1.0f; /* クリップボリュームの最大値 */

		viewport.lx     *= (DG_BackBufferWidthAdjustCoef/(float)DISPLAY_WIDTH) ;
		viewport.ly     *= (DG_BackBufferHeightAdjustCoef/(float)DISPLAY_HEIGHT) ;
		viewport.width  *= (DG_BackBufferWidthAdjustCoef/(float)DISPLAY_WIDTH) ;
		viewport.height *= (DG_BackBufferHeightAdjustCoef/(float)DISPLAY_HEIGHT) ;

		DG_SetViewport( &viewport );
		DG_CurrentViewport = viewport ;
#endif
	}
#endif

	/* カメラ/透視変換マトリクスの設定 */
	//DG_SetTransform( D3DTS_VIEW, &cp->eye_inv );
	//DG_SetTransform( D3DTS_PROJECTION, &cp->pers );

	/* アルファブレンディングOFF */
	DG_AlphaBlendDisable();

	DG_SetPixelShader(0);


	/* デフォルトの描画環境設定 */
	//dma = DG_PopDefaultDrawEnv( cp, top_addr );

	/* 仮面消去パケット生成 */
	if ( cp->bg_clear_flag ){
		/*
			画面消去付き
		*/
		DG_SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
		if ( cp->bg_clear_flag == 2 ){
			/* Ｚバッファのみ消去する */
			DG_Clear(0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
					 D3DCOLOR_XRGB( DG_FogColor.r, DG_FogColor.g, DG_FogColor.b ), 0.0f, 0);
		} else {
			/* 通常の画面消去 */
			DG_Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
					 D3DCOLOR_XRGB( DG_FogColor.r, DG_FogColor.g, DG_FogColor.b ), 0.0f, 0);
		}

		/* 強制５０％半透明用ステンシルの生成 */
		//DG_MakeStencilMask();
	} else {
		/*
			画面消去なし
		*/
		DG_SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
#if 0
		DG_SetRenderState( D3DRS_STENCILENABLE, TRUE );
		DG_SetRenderState( D3DRS_STENCILFUNC, D3DCMP_ALWAYS );
		DG_SetRenderState( D3DRS_STENCILWRITEMASK, 0xFF);
		DG_SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_ZERO );
		DG_SetRenderState( D3DRS_STENCILENABLE, FALSE );
#endif

	}
	/* フォグの設定 */
	DG_SetDxFogColor();

	/* スタティックプッシュバッファへの記録終了 */
	DG_CloseDmaTask();

	/* パフォーマンス検査終了 */
	DG_PerfEndD(DGPERF_CTGR_FRAME_START) ;
}
/* ---------------------------------------------- */

/* 表示グループユーティリティ */

/* 表示グループに追加 */
void	DG_AddCurrentGroup( int id )
{
	//DG_CurrentGroupID |= id ;
	DG_Chanl( 0 )->group_id |= id ;
	DG_Chanl( 1 )->group_id |= id ;
	DG_Chanl( 2 )->group_id |= id ;
	DG_Chanl( 3 )->group_id |= id ;
}

/* 表示グループから削除 */
void	DG_DeleteCurrentGroup( int id )
{
	DG_Chanl( 0 )->group_id &= ~id ;
	DG_Chanl( 1 )->group_id &= ~id ;
	DG_Chanl( 2 )->group_id &= ~id ;
	DG_Chanl( 3 )->group_id &= ~id ;
}

/* 表示グループに追加 */
void	DG_AddCurrentGroup2( int chanl, int id )
{
	DG_Chanl( chanl )->group_id |= id ;
}

/* 表示グループから削除 */
void	DG_DeleteCurrentGroup2( int chanl, int id )
{
	DG_Chanl( chanl )->group_id &= ~id ;
}


/* ---------------------------------------------------------------- */
/* XBOXにて強制５０％半透明を実現するためにステンシルマスクの作成を行う */
extern void DG_MakeStencilMaskNVS( void ) ;

void DG_MakeStencilMask( void )
{
	/* 強制５０％半透明用ステンシルの生成 */
	static FVECTOR VS_Const[] = {
		{0.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 1.0f},
		{0.5f, 0.5f, 0.5f, 0.5f},
		{1.0f/4096.0f, 1.0f/255.0f, 0, 0},
	};
	static FVECTOR	screen_param[2] = {
		{-1.0, 1.0f, 1.0f, 1.0f},
		{2.0f/DISPLAY_WIDTH,-2.0f/DISPLAY_HEIGHT,0.0f,0.0f}
	};
	extern DG_VERTEXSHADER	DG_DmapackVertexShader[1] ;
	short	x, y, xmin, ymin, xmax, ymax ;
	//DG_VERTEX_DMAPACK2D	*Vertex ;
	DWORD				rtgt_w, rtgt_h ;

	/* VertexProcessing設定  */
	DG_SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,
					DG_CheckVertexBufferSoftProcessing()) ;

	/* Vertex Shader 非対応版 */
	if( !DG_CheckUseVertexShader() )
	{
		DG_MakeStencilMaskNVS() ;
		return ;
	}

	if( !(DG_WinApp.flag & M_DG_WINAPP_STENCIL) ){ return ; }	// Stencil-Buffer使用不可

	DG_GetRenderTargetWH(&rtgt_w, &rtgt_h) ;


	DG_SetVertexShaderConstant(CV_ZERO, &VS_Const[0], 1);
	DG_SetVertexShaderConstant(CV_ONE, &VS_Const[1], 1);
	DG_SetVertexShaderConstant(CV_HALF, &VS_Const[2], 1);
	DG_SetVertexShaderConstant(CV_SCALE, &VS_Const[3], 1);
	screen_param[1].vx = 2.0f / (DG_CurrentViewport.width * (float)rtgt_w) ;
	screen_param[1].vy = -2.0f /(DG_CurrentViewport.height * (float)rtgt_h) ;
	DG_SetVertexShaderConstant(0, screen_param, 2);

	/* シェーダー設定 */
	DG_SelectVertexShader( &DG_DmapackVertexShader[0] );

	DG_SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	DG_SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	DG_SetRenderState( D3DRS_STENCILENABLE, TRUE );
	DG_SetRenderState( D3DRS_STENCILFUNC, D3DCMP_ALWAYS );
	DG_SetRenderState( D3DRS_STENCILWRITEMASK, 0xFF);
	DG_SetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_INVERT );
	DG_SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_INVERT );
	DG_SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_INVERT );
	DG_SetRenderState( D3DRS_COLORWRITEENABLE , 0 );
	DG_SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_DISABLE ) ;

	DG_SetTexture( 0, NULL );
	{
		#define	MAX_STENCIL_MAX_VTX_NUM	(DG_GetWorkBufferSize()/sizeof(DG_VERTEX_DMAPACK2D))
		DG_VERTEX_DMAPACK2D *vtx_buff ;
		DG_VERTEX_DMAPACK2D *vtx ;
		int					vtx_cntr ;
		DWORD				sofs ;

		xmin = 0 ;
		ymin = 0 ;
		xmax = (short)((float)rtgt_w * DG_CurrentViewport.width) ;
		ymax = (short)((float)rtgt_h * DG_CurrentViewport.height) ;

		vtx_buff = DG_GetWorkBuffer() ;

		vtx = vtx_buff ;
		vtx_cntr = 0 ;
		for(y=ymin; y<ymax ; y+=2 )
		{
			vtx[0].x = xmin ;
			vtx[0].y = y ;
			//vtx[0].rgba = 0xffffffff ;
			vtx[1].x = xmax ;
			vtx[1].y = y ;
			//vtx[1].rgba = 0xffffffff ;

			vtx += 2 ;
			vtx_cntr += 2 ;
			if( vtx_cntr >= MAX_STENCIL_MAX_VTX_NUM )
			{
				DG_SetDynamicVertexBuffer(vtx_buff, sizeof(DG_VERTEX_DMAPACK2D),
									vtx_cntr, &sofs) ;
				DG_DrawPrimitive( D3DPT_LINELIST, sofs, vtx_cntr >> 1 );

				vtx = vtx_buff ;
				vtx_cntr = 0 ;
			}
		}

		for(x=xmin; x<xmax ; x+=2 )
		{
			vtx[0].x = x ;
			vtx[0].y = ymin ;
			//vtx[0].rgba = 0xffffffff ;
			vtx[1].x = x ;
			vtx[1].y = ymax ;
			//vtx[1].rgba = 0xffffffff ;

			vtx += 2 ;
			vtx_cntr += 2 ;
			if( vtx_cntr >= MAX_STENCIL_MAX_VTX_NUM )
			{
				DG_SetDynamicVertexBuffer(vtx_buff, sizeof(DG_VERTEX_DMAPACK2D),
									vtx_cntr, &sofs) ;
				DG_DrawPrimitive( D3DPT_LINELIST, sofs, vtx_cntr >> 1 );

				vtx = vtx_buff ;
				vtx_cntr = 0 ;
			}
		}

		if( vtx_cntr )
		{
			DG_SetDynamicVertexBuffer(vtx_buff, sizeof(DG_VERTEX_DMAPACK2D),
									vtx_cntr, &sofs) ;
			DG_DrawPrimitive( D3DPT_LINELIST, sofs, vtx_cntr >> 1 );
		}

		#undef	MAX_STENCIL_MAX_VTX_NUM
	}

	DG_SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
	DG_SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	DG_SetRenderState( D3DRS_STENCILENABLE, TRUE );
	DG_SetRenderState( D3DRS_STENCILFUNC, D3DCMP_ALWAYS );
	DG_SetRenderState( D3DRS_STENCILWRITEMASK, 0xFF);
	DG_SetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP );
	DG_SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
	DG_SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );
	DG_SetRenderState( D3DRS_STENCILENABLE, FALSE );
	DG_SetRenderState( D3DRS_STENCILREF, 0x00);
#ifndef _WINDOWS
	DG_SetRenderState( D3DRS_COLORWRITEENABLE , D3DCOLORWRITEENABLE_ALL );
#else
	DG_SetRenderState( D3DRS_COLORWRITEENABLE ,   D3DCOLORWRITEENABLE_RED
												| D3DCOLORWRITEENABLE_GREEN
												| D3DCOLORWRITEENABLE_BLUE
												| D3DCOLORWRITEENABLE_ALPHA) ;
#endif
	//DG_SetRenderState( D3DRS_STENCILENABLE, TRUE );
	DG_SetRenderState( D3DRS_STENCILFUNC, D3DCMP_NOTEQUAL );
	DG_SetRenderState( D3DRS_STENCILENABLE, FALSE );
}

/* ---------------------------------------------------------------- */
/* XBOXにて強制５０％半透明を実現するためにステンシルマスクの作成を行う */
/* 			(VertexShader非対応版)									*/

void DG_MakeStencilMaskNVS( void )
{
	/* 強制５０％半透明用ステンシルの生成 */
	static FMATRIX	screen_param_mtx =
	{
		2.0f/DISPLAY_WIDTH,	0.0f,					0.0f,	0.0f,
		0.0f,				-2.0f/DISPLAY_HEIGHT,	0.0f,	0.0f,
		0.0f,				0.0f,					0.0f,	0.0f,
		-1.0f,				1.0f,					1.0f,	1.0f
	};
	int					x, y, xmin, ymin, xmax, ymax ;
	DWORD				rtgt_w, rtgt_h ;
	
	if( !(DG_WinApp.flag & M_DG_WINAPP_STENCIL) ){ return ; }	// Stencil-Buffer使用不可

	DG_GetRenderTargetWH(&rtgt_w, &rtgt_h) ;

	{
		D3DXMATRIX	d3dx_mtx ;

		D3DXMatrixOrthoOffCenterLH(&d3dx_mtx, -1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f) ;
		DG_SetTransform(D3DTS_PROJECTION, (FMATRIX *)d3dx_mtx.m) ;

		screen_param_mtx.m[0][0] = 2.0f / (DG_CurrentViewport.width * (float)rtgt_w) ;
		screen_param_mtx.m[1][1] = -2.0f /(DG_CurrentViewport.height * (float)rtgt_h) ;
		DG_SetTransform(D3DTS_VIEW,  &screen_param_mtx) ;
		DG_SetTransform(D3DTS_WORLD, &DG_UnitMatrix) ;
	}

	/* シェーダー設定 */
	DG_SetVertexShader( D3DFVF_DG_VERTEX_DMAPACK2D_FLAG );

	DG_SetRenderState( D3DRS_LIGHTING, FALSE) ;
	DG_SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
	DG_SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	DG_SetRenderState( D3DRS_STENCILENABLE, TRUE );
	DG_SetRenderState( D3DRS_STENCILFUNC, D3DCMP_ALWAYS );
	DG_SetRenderState( D3DRS_STENCILWRITEMASK, 0xFF);
	DG_SetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_INVERT );
	DG_SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_INVERT );
	DG_SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_INVERT );
	DG_SetRenderState( D3DRS_COLORWRITEENABLE , 0 );
	DG_SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_DISABLE ) ;

	DG_SetTextureNVS( 0, NULL );
	{
		#define	MAX_STENCIL_MAX_VTX_NUM	(DG_GetWorkBufferSize()/sizeof(DG_VERTEX_DMAPACK2D_FVF))
		DG_VERTEX_DMAPACK2D_FVF *vtx_buff ;
		DG_VERTEX_DMAPACK2D_FVF *vtx ;
		int					vtx_cntr ;
		DWORD				sofs ;

		xmin = 0 ;
		ymin = 0 ;
		xmax = (short)((float)rtgt_w * DG_CurrentViewport.width) ;
		ymax = (short)((float)rtgt_h * DG_CurrentViewport.height) ;

		vtx_buff = DG_GetWorkBuffer() ;

		vtx = vtx_buff ;
		vtx_cntr = 0 ;
		for(y=ymin; y<ymax ; y+=2 )
		{
			vtx[0].x = (float)xmin ;
			vtx[0].y = (float)y ;
			vtx[0].z = 0.0f ;
			//vtx[0].rgba = 0xffffffff ;
			vtx[1].x = (float)xmax ;
			vtx[1].y = (float)y ;
			vtx[1].z = 0.0f ;
			//vtx[1].rgba = 0xffffffff ;

			vtx += 2 ;
			vtx_cntr += 2 ;
			if( vtx_cntr >= MAX_STENCIL_MAX_VTX_NUM )
			{
				DG_SetDynamicVertexBuffer(vtx_buff, sizeof(DG_VERTEX_DMAPACK2D_FVF),
									vtx_cntr, &sofs) ;
				DG_DrawPrimitive( D3DPT_LINELIST, sofs, vtx_cntr >> 1 );

				vtx = vtx_buff ;
				vtx_cntr = 0 ;
			}
		}

		for(x=xmin; x<xmax ; x+=2 )
		{
			vtx[0].x = (float)x ;
			vtx[0].y = (float)ymin ;
			vtx[0].z = 0.0f ;
			//vtx[0].rgba = 0xffffffff ;
			vtx[1].x = (float)x ;
			vtx[1].y = (float)ymax ;
			vtx[1].z = 0.0f ;
			//vtx[1].rgba = 0xffffffff ;

			vtx += 2 ;
			vtx_cntr += 2 ;
			if( vtx_cntr >= MAX_STENCIL_MAX_VTX_NUM )
			{
				DG_SetDynamicVertexBuffer(vtx_buff, sizeof(DG_VERTEX_DMAPACK2D_FVF),
									vtx_cntr, &sofs) ;
				DG_DrawPrimitive( D3DPT_LINELIST, sofs, vtx_cntr >> 1 );

				vtx = vtx_buff ;
				vtx_cntr = 0 ;
			}
		}

		if( vtx_cntr )
		{
			DG_SetDynamicVertexBuffer(vtx_buff, sizeof(DG_VERTEX_DMAPACK2D_FVF),
									vtx_cntr, &sofs) ;
			DG_DrawPrimitive( D3DPT_LINELIST, sofs, vtx_cntr >> 1 );
		}

		#undef	MAX_STENCIL_MAX_VTX_NUM
	}

	DG_SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
	DG_SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
	DG_SetRenderState( D3DRS_STENCILENABLE, TRUE );
	DG_SetRenderState( D3DRS_STENCILFUNC, D3DCMP_ALWAYS );
	DG_SetRenderState( D3DRS_STENCILWRITEMASK, 0xFF);
	DG_SetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP );
	DG_SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
	DG_SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );
	DG_SetRenderState( D3DRS_STENCILENABLE, FALSE );
	DG_SetRenderState( D3DRS_STENCILREF, 0x00);
	DG_SetRenderState( D3DRS_COLORWRITEENABLE ,   D3DCOLORWRITEENABLE_RED
												| D3DCOLORWRITEENABLE_GREEN
												| D3DCOLORWRITEENABLE_BLUE
												| D3DCOLORWRITEENABLE_ALPHA) ;
	//DG_SetRenderState( D3DRS_STENCILENABLE, TRUE );
	DG_SetRenderState( D3DRS_STENCILFUNC, D3DCMP_NOTEQUAL );
	DG_SetRenderState( D3DRS_STENCILENABLE, FALSE );
}

/* ---------------------------------------------------------------- */
/*
	非描画フレーム描画処理
*/

static	void		DG_DrawUnDrawFrame(int which)
{
	DG_ExeBufferSync(FALSE) ;

	/* 描画バッファは初期化しておく */
	DG_ClearTextureImage(DG_BackBufferTexture[0]) ;
	DG_ClearTextureImage(DG_BackBufferTexture[1]) ;
}
/* ---------------------------------------------------------------- */

