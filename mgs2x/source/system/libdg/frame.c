//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	frame.c
	フレームパラメータ管理ルーチン

	1999/07/07 K.Takabe
	$Id: frame.c,v 1.1.1.3 2002/11/19 11:42:08 Yoshizawa1 Exp $

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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>


#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"

#include	"libgv.h"

#include "BP_RenderBuffer.h"
#include "BP_RenderBufferTypes.h"
#include "BP_Camera.h"
#include "BP_Renderer.h"
#include "BP_Debug.h"
#include "BP_RenderPrim.h"
#include "BP_Memory.h"

#define DG_THREAD_VARS_DEFAULT \
{ \
   0, /* clock */ \
   0, /* undraw */ \
   -1,/* lastwhich */ \
   0, /* light clock */ \
   0, /* DG_PrivilegeMode */ \
   { 0 }, /* pad */ \
   {  /* DG_LightMatrix */ \
      { \
         {0.74f,0.0f,0.0f,0.0f}, \
         {-0.74f,0.0f,0.0f,0.0f}, \
         {0.74f,0.0f,0.0f,0.0f}, \
         {0.0f,0.0f,0.0f,0.0f} \
      } \
   }, \
   {  /* DG_ColorMatrix */                      \
      {                                         \
         {64.0,64.0f,64.0f,0.0f},               \
         {0.0,0.0f,0.0f,0.0f},                  \
         {0.0,0.0f,0.0f,0.0f},                  \
         {64.0f,64.0f,64.0f,0.0f}               \
      }                                         \
   },                                           \
   { 0.0f, 0.0f, 0.0f, 0.0f }, /* BP_FogParam */   \
   0.0f, /* DG_FogParam1 */                     \
   255.0f, /* DG_FogParam2 */                     \
   { 0.0f }, /* pad2 */ \
}

SPerThreadDGVars DG_AS_PerThreadVars[2] = { 
   DG_THREAD_VARS_DEFAULT,
   DG_THREAD_VARS_DEFAULT
};

// If this CTASSERT fails, it means per-thread vars were added
// and not accounted for
KP_CTASSERT( sizeof( SPerThreadDGVars ) == sizeof(int) * 8 + sizeof( FMATRIX ) * 2 + sizeof( float ) * 4 + sizeof( FVECTOR ) ); 


int gAS_UsedBufferSceneNOP = 0;
int DG_Endframe_FrameCount = 0;
int DG_Renderframe_FrameCount = 0;
enum EDGHackArea gAS_DG_HackArea = kDGHA_Unknown;

#if BP_VITA
__thread
#endif
int DG_AS_CurrentThreadVarsIndex = 0;

// THREADING THE ENDFRAME
#if BP_VITA
#  include <kernel/threadmgr.h>
#  include "kernel/processmgr.h"
#  define VITA_THREADED_ENDFRAME 1

#endif

#ifdef VITA_THREADED_ENDFRAME
#  include "Engine/System/VTAThreadPriorities.h"

static const int kEndFrameFlag_Go   = ( 1 << 0 );
static const int kEndFrameFlag_Done = ( 1 << 1 );
static SceUID gEndFrameEventFlagId = -1;
static SceUID gEndFrameThreadId = -1;
static volatile int gEndFrameWhich;
static SceKernelLwMutexWork gEndFrameMutex;

int gEndFrameIsThreaded = TRUE;

static SceInt32 _BP_Endframe_Vita_Thread_Function(SceSize argSize, void *pArgBlock);
static void _BP_EndFrame_GameThread_WaitThreadDoneDoNotPassGo();

#else

int gEndFrameIsThreaded = FALSE;

#endif

int gUseOneThread = 0;

#define GS_REGS_0()
#define GS_REGS_1(r0) \
((r0) << 0x00)
#define GS_REGS_2(r0, r1) \
((r0) << 0x00 | (r1) << 0x04)
#define GS_REGS_3(r0, r1, r2) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08)
#define GS_REGS_4(r0, r1, r2, r3) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08 | (r3) << 0x0c)
#define GS_REGS_5(r0, r1, r2, r3, r4) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08 | (r3) << 0x0c | (r4) << 0x10)
#define GS_REGS_6(r0, r1, r2, r3, r4, r5) \
((r0) << 0x00 | (r1) << 0x04 | (r2) << 0x08 | (r3) << 0x0c | (r4) << 0x10 | (r5) << 0x14)

/*----------------------------------------------------------------*/

	/*
		バッファ切り替えクロック
	*/
//int		DG_Clock = 0 ;

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
		フォグ関連パラメータ
	*/
CVECTOR	DG_FogColorMaster = {0,0,0,0};
CVECTOR	DG_FogColor = {0,0,0,0};
//float	DG_FogParam1 = 0.0f ;
//float	DG_FogParam2 = 255.0f;
//FVECTOR BP_FogParam;

	/*
		クリップ設定
	*/
float	DG_ClipNear = DRAW_NEAR_CLIP ;
float	DG_ClipFar = DRAW_FAR_CLIP ;


#ifdef DEBUG_MODE
	/*
		スクリーンショット記録用強制描画オフセット変更フラグ
	*/
int		DG_OffsetControlFlag = 0 ;
#endif


/*----------------------------------------------------------------*/

#ifndef	NO_PROTOTYPE
#endif

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
void DG_SetFogColor( int r, int g, int b )
{
	DG_FogColorMaster.r = r ;
	DG_FogColorMaster.g = g ;
	DG_FogColorMaster.b = b ;
}	

void DG_SetFogParam( float near, float far )
{
#if 0 //BP_PS2
	DG_FogParam1 = 255.0f / ( near - far ) ;
	DG_FogParam2 = 255.0f - DG_FogParam1 * near ;
//	DG_ClipFar = far ;
#else

   BP_FogParam.vx = -1.0f / (near - far);
   BP_FogParam.vy = -BP_FogParam.vx * near;
   BP_FogParam.vz = 0.0f;
   BP_FogParam.vw = 1.0f;

   //BP - Need to keep original values the same as PS2 because they are being used by certain code to reverse calculate the near/far values for the fog (i.e. d_fog_set.c)
   DG_FogParam1 = 255.0f / ( near - far );
   DG_FogParam2 = 255.0f - DG_FogParam1 * near;

#endif
}	

void DG_SetClipParam( float near, float far )
{
	DG_ClipNear = near ;
	DG_ClipFar = far ;
}

/*----------------------------------------------------------------*/

void		DG_InitFrameSystem()
{
	static FVECTOR cam_def_from = {0,0,0,1};
	static FVECTOR cam_def_to = {0,0,1,1};
   BP_WaitForLastRenderThreadToComplete();

   // Comment copied from MGS3 - may not be accurate
   // BP(JM) - NOTE THAT THIS FUNCTION CAN BE CALLED A NUMBER OF TIMES

   BP_Render_EnableUltFromThisThread( 0 );

#if VITA_THREADED_ENDFRAME
   if ( gEndFrameIsThreaded && gEndFrameThreadId == -1 )
   {
      // It is essential that the mutex and condition variable are woken on a FIFO basis, not thread
      gEndFrameEventFlagId = sceKernelCreateEventFlag(
         "End Frame Event", 
         SCE_KERNEL_EVF_ATTR_TH_PRIO | SCE_KERNEL_EVF_ATTR_MULTI, 
         kEndFrameFlag_Done, 
         NULL);

      gEndFrameThreadId = sceKernelCreateThread(
         "End Frame Thread", 
         _BP_Endframe_Vita_Thread_Function, 
         kPriorityEndFrame, 
         128 * 1024, // Stack size
         0, 
         SCE_KERNEL_CPU_MASK_USER_ALL, NULL);

      sceKernelStartThread(gEndFrameThreadId, 0, NULL);
   }

   _BP_EndFrame_GameThread_WaitThreadDoneDoNotPassGo();

#endif
   BP_WaitForLastRenderThreadToComplete();

   BP_RB_SetCurrentWriterThread();

   BP_WaitForLastRenderThreadToComplete();

   DG_ClearChanlSystem( 0 ) ;
	DG_ClearChanlSystem( 1 ) ;

   BP_RB_UnsetCurrentWriterThread();

   DG_ResetTextureTmpClut() ;
	//DG_SortChanlSystem( 0 ) ;
	//DG_SortChanlSystem( 1 ) ;
	//DG_ClipNear = DRAW_NEAR_CLIP ;
	//DG_ClipFar = DRAW_FAR_CLIP ;
	DG_SetFogColor( 0, 0, 0 );
	DG_SetFogParam( 20*65536.0f, 21*65536.0f );
	DG_SetClipParam( DRAW_NEAR_CLIP, DRAW_FAR_CLIP );
	DG_DisplayStatus = 0 ;
   BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
	DG_SetCamera2( DG_Chanl(0), &cam_def_from, &cam_def_to, 2.0f );
	DG_SetCamera2( DG_Chanl(1), &cam_def_from, &cam_def_to, 2.0f );
}

//long64 DG_UnDrawFrameCount=0;

//int DG_LastWhich = -1;

void		DG_StartFrame()
{
	int		which ;
   int      isEndFrameThread = 0;

   // JM(AS) - DG_StartFrame changes its behavior depending on the thread
   // it's called from!  If it's called from a non-render thread, it just updates its 
   // internal variables. If it's called from a render thread, it flips buffers and such

   if ( !gEndFrameIsThreaded || DG_AS_CurrentThreadVarsIndex == 1 )
   {
      isEndFrameThread = 1;
   }

   if ( DG_AS_CurrentThreadVarsIndex == 0 )
   {
      gAS_DG_HackArea = kDGHA_Unknown;
      if ( !strcmp( GM_GetArea(), "w32a"  ) )
      {
         gAS_DG_HackArea = kDGHA_w32a;
      }
   }

   if (isEndFrameThread)
   {
	   GV_PROFILE_FLIP();
	   GV_PROFILE_START( GV_PROFILE_DRAW_START );
	   GV_PROFILE_START( GV_PROFILE_ACT_START );
   }

   DG_Clock = 1 - DG_Clock ;
	which = DG_Clock ;

	if( DG_UnDrawFrameCount > 0 ){
		if( DG_LastWhich < 0 ){
			DG_LastWhich = which;
		}
		DG_UnDrawFrameCount--;
		if ( isEndFrameThread )
      {
         DG_UnDrawChanlSystem( 1 - which );
      }
	    goto Skip;
	} else {
		if( DG_LastWhich >= 0 ){
			if( which == DG_LastWhich ){
            if ( isEndFrameThread )
            {
   				DG_UnDrawChanlSystem( 1 - which );
            }
				goto Skip;
			}
		}
	}
	/*
		表示画面を切りかえる
	*/
   if ( isEndFrameThread )
   {
   	DG_SetDisplayMode( which );
   }

	/*
		描画開始
	*/
#ifdef HIGHRESO_I
#  error THIS NEVER HAPPENS

	{
		DG_CHANL	*cp ;
		int			i, field = DG_Field ;
		{
			static int	no_delay_count ;
			extern int DG_LastActTime;	// in dgd.c
			extern int DG_LastDrawTime;

			if ( ( DG_LastActTime >= 260 ) || ( DG_LastDrawTime >= 260 ) ){
				no_delay_count = 0 ;
			} else {
				no_delay_count++ ;
			}
			if ( !( no_delay_count > 2 ) ) field = 0 ;
		}
#ifdef DEBUG_MODE
		/* スクリーンショット撮影用強制オフセットコントロール処理 */
		if ( DG_OffsetControlFlag != 0 ) field = DG_Clock ;
#endif
		for ( i = 0 ; i < 5 ; i++ ){
			int		f ;
			cp = DG_Chanl(i) ;
			f = ( cp->high_reso ) ? field : 1 ;
			if ( cp->flag ){
				*(u_long64*)&cp->draw_offset[1-DG_Clock].datas.xyoffset1 = 
				  SCE_GS_SET_XYOFFSET( ( ( 2048 - DRAW_WIDTH / 2 - cp->offset_x ) << 4 ),
									  ( ( 2048 - DRAW_HEIGHT / 2 - cp->offset_y ) << 4 ) + ( f ? 0 : 8 ) ) ;
			}
		}
		DG_CurrentField = field;
	}
#else
   if ( isEndFrameThread )
   {
   	DG_CurrentField = DG_Field;
   }
#endif

	{
		extern int DG_VSyncStatus;	// dgd.c
		if( DG_VSyncStatus == 0 ){
			/* 表示期間中 */
			/* 次のHSYNCの頭まで待つ */
			WAIT_HSYNC( 0 );
		}
	}

   BP_Debug_PushCPUMarker("DG_StartFrame");

   if ( isEndFrameThread )
   {
   	DG_DrawChanlSystem( 1 - which ) ;
   }

	DG_LastWhich = -1;

   BP_Debug_PopCPUMarker();

Skip:
	/*
		フレーム開始処理
	*/
   if ( isEndFrameThread )
   {
      BP_RB_SetCurrentWriterThread();
      
      DG_ClearChanlSystem( which ) ;
      
      BP_RB_UnsetCurrentWriterThread();
   }

   if ( DG_AS_CurrentThreadVarsIndex == 0 )
   {
      DG_ClearTmpLight();
   }
}

static void debug_put_info( char *str, PERFORMANCE_PACKET_INFO *info )
{
		printf("%s : %d objcts  %d packs  %d verts\n", str, info->n_obj, info->n_packs, info->n_verts );
}


static void end_frame_internal(int which)
{
   BP_RB_SetCurrentWriterThread();

   // this will be in another thread
   DG_SortChanlSystem( which ) ;

   /* 一時ＣＬＵＴバッファのリセット */
   DG_ResetTextureTmpClut();

   BP_Debug_PushCPUMarker( "DrainUlt+Fixup" );
   BP_Render_HelpDrainUltWork();
   BP_Prim_Update_FixupPrimRenderPackets();
   BP_Debug_PopCPUMarker();

   BP_RB_UnsetCurrentWriterThread();

   if (gEndFrameIsThreaded)
   {
      DG_StartFrame();
   }
}

#ifdef VITA_THREADED_ENDFRAME

static void _BP_EndFrame_EndFrameThread_WaitToGo()
{
   sceKernelWaitEventFlag(
      gEndFrameEventFlagId, 
      kEndFrameFlag_Go, 
      SCE_KERNEL_EVF_WAITMODE_AND | SCE_KERNEL_EVF_WAITMODE_CLEAR_PAT, 
      NULL, 
      NULL);
}

static void _BP_EndFrame_EndFrameThread_MarkDone()
{
   // Marks the end frame thread as done by or'ing done

   sceKernelSetEventFlag(
      gEndFrameEventFlagId, 
      kEndFrameFlag_Done);

}

static void _BP_EndFrame_GameThread_WaitThreadDone()
{
   // This waits for the other thread to be done.
   // Note that event flags are not cleared when this happens.
   // "Done" gets cleared the next time the thread is marked to go.

   sceKernelWaitEventFlag(
      gEndFrameEventFlagId, 
      kEndFrameFlag_Done, 
      SCE_KERNEL_EVF_WAITMODE_AND, 
      NULL, 
      NULL);
}

static void _BP_EndFrame_GameThread_WaitThreadDoneDoNotPassGo()
{
   // This waits for the other thread to be done, and it makes
   // damn sure that there is no pending "Go" state.  This is called
   // when re-initting the frame.

   _BP_EndFrame_GameThread_WaitThreadDone();

   sceKernelClearEventFlag(
      gEndFrameEventFlagId, 
      ~kEndFrameFlag_Go);

}

static void _BP_EndFrame_GameThread_MarkThreadGo()
{
   // Marks the end frame to go, which unsets done.
   _BP_EndFrame_GameThread_WaitThreadDone();

   sceKernelClearEventFlag(
      gEndFrameEventFlagId, 
      ~kEndFrameFlag_Done);

   sceKernelSetEventFlag(
      gEndFrameEventFlagId, 
      kEndFrameFlag_Go);
}

static SceInt32 _BP_Endframe_Vita_Thread_Function(SceSize argSize, void *pArgBlock)
{
   // Use the "endframe" thread vars
   DG_AS_CurrentThreadVarsIndex = 1;

   BP_Render_EnableUltFromThisThread( DG_AS_CurrentThreadVarsIndex );
   GV_AS_EnableScratchpadFromThisThread();
   do
   {
      _BP_EndFrame_EndFrameThread_WaitToGo();
      {
         int which = gEndFrameWhich;
         SceUInt64 frameStartTime, frameEndTime;

         frameStartTime = sceKernelGetProcessTimeWide();
         end_frame_internal(which);

         frameEndTime = sceKernelGetProcessTimeWide();

         BP_Renderer_MGS2_3060_SetEndframeTimeUS( frameEndTime - frameStartTime );
      }
      _BP_EndFrame_EndFrameThread_MarkDone();
   }
   while (TRUE);   
}

#endif

void DG_AS_EndframeThread_WaitDone()
{
#ifdef VITA_THREADED_ENDFRAME
   if ( gEndFrameIsThreaded )
   {
      _BP_EndFrame_GameThread_WaitThreadDone();
   }
#endif
}

#if VITA_THREADED_ENDFRAME
static uint64_t sPriorFrameProcessTimeFor60FPS = 0;
#endif

void DG_EndFrame_Skip()
{
   // Setting the prior frame process time back here ensures
   // that when we are doing our 60-fps determination, we only
   // take the non-skip frame into account.
#ifdef VITA_THREADED_ENDFRAME
   sPriorFrameProcessTimeFor60FPS = sceKernelGetProcessTimeWide();
#endif
}

void		DG_EndFrame()
{

	int		which ;

   BP_Debug_PushCPUMarker("DG_EndFrame");

#ifdef VITA_THREADED_ENDFRAME
   if (gEndFrameIsThreaded)
   {
      extern int GM_MenuStatusStackLevel;
      // JM(AS) - Check on the framerate for the game thread before waiting on the endframe thread 
      // as that thread's time shouldn't impact this one really
      uint64_t currentFrameProcessTime = sceKernelGetProcessTimeWide();
      int const valid60fpsGameMode = 
         GM_MenuStatusStackLevel == 0 && // If we have a menu up, dump us to 30fps
         DG_PrivilegeMode == 0 /* PrivilegeMode means full screen something, dump to 30 if not */ ;
      
      // We're in the codec, say it's a full second to force 30fps
      BP_Renderer_MGS2_3060_SetGameTimeUS( currentFrameProcessTime - sPriorFrameProcessTimeFor60FPS, valid60fpsGameMode );

      _BP_EndFrame_GameThread_WaitThreadDone();

      // Copy the clock and such to the endframe thread
      DG_AS_PerThreadVars[1] = DG_AS_PerThreadVars[0];

      // JM(AS) - Start the frame time counter here after we wait for thread done
      BP_Renderer_MGS2_3060_GameHeartbeat();
      sPriorFrameProcessTimeFor60FPS = sceKernelGetProcessTimeWide();
   }
#endif // VITA_THREADED_ENDFRAME

   ++DG_Endframe_FrameCount;

	/*
		オブジェクトのセットアップ＆ＤＭＡ登録
	*/
	which = DG_Clock ;

#ifdef LIBDG_PERFORMANCE
	if ( GV_PadData[0].press & PAD_AL ){
		debug_put_info( "normal model 0", &DG_PerformanceData.normal_model[0] );
		debug_put_info( "normal model 1", &DG_PerformanceData.normal_model[1] );
		debug_put_info( "trans model 0", &DG_PerformanceData.trans_model[0] );
		debug_put_info( "trans model 1", &DG_PerformanceData.trans_model[1] );
		debug_put_info( "multex model 0", &DG_PerformanceData.multex_model[0] );
		debug_put_info( "multex model 1", &DG_PerformanceData.multex_model[1] );
		debug_put_info( "mulwt model", &DG_PerformanceData.mulwt_model );
		debug_put_info( "prim poly", &DG_PerformanceData.prim_poly );
		debug_put_info( "prim line", &DG_PerformanceData.prim_line );
		debug_put_info( "prim sprt", &DG_PerformanceData.prim_sprt );
		debug_put_info( "prim rsprt", &DG_PerformanceData.prim_rsprt );
		debug_put_info( "shadow prejection 0", &DG_PerformanceData.shadow_projection[0] );
		debug_put_info( "shadow prejection 1", &DG_PerformanceData.shadow_projection[1] );
		debug_put_info( "spot prejection 0", &DG_PerformanceData.spot_projection[0] );
		debug_put_info( "spot prejection 1", &DG_PerformanceData.spot_projection[1] );
		debug_put_info( "shadow draw 0", &DG_PerformanceData.shadow_draw[0] );
		debug_put_info( "shadow draw 1", &DG_PerformanceData.shadow_draw[1] );
		debug_put_info( "comodel", &DG_PerformanceData.comodel );
		printf("trans texture size %d KB\n", DG_PerformanceData.use_tex_size / 1024 );
	}
	GV_ZeroMemory( &DG_PerformanceData, sizeof(DG_PERFORMANCE_DATA) ) ;
#endif

	/* 赤外線ゴーグル対応処理 */
	DG_FogColor = DG_FogColorMaster ;
	if ( DG_DisplayStatus & DG_STATE_IR_MODE ){
		DG_FogColor.r >>= 1 ;
		DG_FogColor.g >>= 1 ;
		DG_FogColor.b >>= 1 ;
	}

	/* 一時ＣＬＵＴバッファの設定 */
	DG_SetTextureTmpClut( which );
	/* ライト変更に伴う再プリシェード（チャンネル０のオブジェクトに対してのみ） */
	DG_LightReshadeChanl( DG_Chanl( 0 ), which );
	/* プリシェード更新（チャンネル０のオブジェクトに対してのみ） */
	DG_TmpLightPreshadeChanl( DG_Chanl( 0 ), which );
	/* 頂点アニメの復元（チャンネル０のオブジェクトに対してのみ） */
	DG_RefreshVAnimeChanl( DG_Chanl( 0 ), which );

   BP_Render_HelpDrainUltWork();

#ifdef VITA_THREADED_ENDFRAME
   if (gEndFrameIsThreaded)
   {
      char const *area = GM_GetArea();
      int use_one_thread = 0;

      if ( !strcmp( area, "w04a" ) || !strcmp( area, "w04b" ) || !strcmp( area, "w04c" ) )
      {
         use_one_thread = 1;
      }
      else
         use_one_thread = gUseOneThread;

      if ( use_one_thread )
      {
         gAS_UsedBufferSceneNOP = 1;
         
         AS_BufferSceneNOP( which );

         gEndFrameWhich = which;

         _BP_EndFrame_GameThread_MarkThreadGo();
         _BP_EndFrame_GameThread_WaitThreadDone();

         gAS_UsedBufferSceneNOP = 0;
      }
      else
      {
         AS_BufferScene(which);
         gEndFrameWhich = which;

         _BP_EndFrame_GameThread_MarkThreadGo();
      }
   }
   else
   {
      AS_BufferScene(which);
      end_frame_internal(which);
   }
#else
   AS_BufferScene(which);
   end_frame_internal(which);
#endif
   
#ifdef DEBUG_MODE
	{/* スクリーンショット撮影用処理 */
		extern void DG_CheckSaveScreen( void );
		DG_CheckSaveScreen();
	}
#endif

   BP_Debug_PopCPUMarker();
}

/*----------------------------------------------------------------*/

void DG_WaitForThreadedRenderComplete()
{
#ifdef VITA_THREADED_ENDFRAME
   if (gEndFrameIsThreaded)
   {
      _BP_EndFrame_GameThread_WaitThreadDone();
   }
#endif
}

/*----------------------------------------------------------------*/
static FVECTOR prev_vx;
void		DG_MakeCameraMatrix( FMATRIX *mat, FVECTOR *from, FVECTOR *to )
{
	static	FVECTOR	Lower = { 0.0F, -1.0F, 0.0F, 1.0F } ;
	FVECTOR		z_vec, y_vec, x_vec ;

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
/* カメラマトリクスを用いてカメラを設定 */
void		DG_SetCamera( DG_CHANL *chanl, FMATRIX *mat, float screen )
{
	chanl->screen = screen ;
	chanl->eye = *mat ;
	_sceVu0InversMatrix( &chanl->eye_inv, &chanl->eye );

	/*
		透視変換行列を生成する
	*/
#if 0
	_sceVu0ViewScreenMatrix( &chanl->pers,
						   chanl->screen,
						   ASPECT_X(), ASPECT_Y() * chanl->width / chanl->height, /* アスペクト比 */
						   0.0F, 0.0F, /* ＸＹオフセット値 */
						   (float)-1.0F, (float)1.0F, /* 変換後Ｚ範囲 */
						   (float)DRAW_NEAR_CLIP, (float)DRAW_FAR_CLIP); /* Ｚクリップ */
	_sceVu0ViewScreenMatrix( &chanl->pers2,
						   chanl->screen * ( chanl->width / 2 ),
						   ASPECT_X(), ASPECT_Y(), /* アスペクト比 */
						   2048.0F, 2048.0F, /* ＸＹオフセット値 */
						   (float)DRAW_Z_MIN, (float)DRAW_Z_MAX, /* 変換後Ｚ範囲 */
						   (float)DRAW_NEAR_CLIP, (float)DRAW_FAR_CLIP); /* Ｚクリップ */
	_sceVu0ViewScreenMatrix( &chanl->raise_pers,
						   chanl->screen,
						   ASPECT_X(), ASPECT_Y() * chanl->width / chanl->height, /* アスペクト比 */
						   0.0F, 0.0F, /* ＸＹオフセット値 */
						   (float)-1.0F, (float)1.0F, /* 変換後Ｚ範囲 */
						   (float)DRAW_NEAR_CLIP+0.05f, (float)DRAW_FAR_CLIP+0.0f); /* Ｚクリップ */
	_sceVu0ViewScreenMatrix( &chanl->raise_pers2,
						   chanl->screen * ( chanl->width / 2 ),
						   ASPECT_X(), ASPECT_Y(), /* アスペクト比 */
						   2048.0F, 2048.0F, /* ＸＹオフセット値 */
						   (float)DRAW_Z_MIN, (float)DRAW_Z_MAX, /* 変換後Ｚ範囲 */
						   (float)DRAW_NEAR_CLIP+0.05f, (float)DRAW_FAR_CLIP+0.0f); /* Ｚクリップ */
#else

	_sceVu0ViewScreenMatrix( &chanl->pers,
						   chanl->screen,
						   ASPECT_X(), ASPECT_Y() * chanl->width / chanl->height, /* アスペクト比 */
						   BP_GetScreenOffsetX(), BP_GetScreenOffsetY(), /* ＸＹオフセット値 */    //BP_CAMERA - apply settings
						   (float)-1.0F, (float)1.0F, /* 変換後Ｚ範囲 */
                     (float)DG_ClipNear, (float)DG_ClipFar); /* Ｚクリップ */

	_sceVu0ViewScreenMatrix( &chanl->pers2,
						   chanl->screen * ( chanl->width / 2 ),
						   ASPECT_X(), ASPECT_Y(), /* アスペクト比 */
						   2048.0F, 2048.0F, /* ＸＹオフセット値 */
						   (float)DRAW_Z_MIN, (float)DRAW_Z_MAX, /* 変換後Ｚ範囲 */
						   (float)DG_ClipNear, (float)DG_ClipFar); /* Ｚクリップ */

	_sceVu0ViewScreenMatrix( &chanl->raise_pers,
						   chanl->screen,
						   ASPECT_X(), ASPECT_Y() * chanl->width / chanl->height, /* アスペクト比 */
						   BP_GetScreenOffsetX(), BP_GetScreenOffsetY(), /* ＸＹオフセット値 */    //BP_CAMERA - apply settings
						   (float)-1.0F, (float)1.0F, /* 変換後Ｚ範囲 */
						   (float)DG_ClipNear+0.05f, (float)DG_ClipFar+0.0f); /* Ｚクリップ */

	_sceVu0ViewScreenMatrix( &chanl->raise_pers2,
						   chanl->screen * ( chanl->width / 2 ),
						   ASPECT_X(), ASPECT_Y(), /* アスペクト比 */
						   2048.0F, 2048.0F, /* ＸＹオフセット値 */
						   (float)DRAW_Z_MIN, (float)DRAW_Z_MAX, /* 変換後Ｚ範囲 */
						   (float)DG_ClipNear+0.05f, (float)DG_ClipFar+0.0f); /* Ｚクリップ */
#endif

	/*
		カメラ透視変換行列を生成する
	*/
	_sceVu0MulMatrix( &chanl->eye_pers, &chanl->pers, &chanl->eye_inv );
	_sceVu0MulMatrix( &chanl->eye_pers2, &chanl->pers2, &chanl->eye_inv );
	_sceVu0MulMatrix( &chanl->raise_eye_pers, &chanl->raise_pers, &chanl->eye_inv );
	_sceVu0MulMatrix( &chanl->raise_eye_pers2, &chanl->raise_pers2, &chanl->eye_inv );

   //BP_CAMERA - set extra matrices that do not include camera tweak offset
   _sceVu0ViewScreenMatrix( &chanl->pers_no_offset,
                            chanl->screen,
                            ASPECT_X(), ASPECT_Y() * chanl->width / chanl->height,
                            0.0f, 0.0f,
                            (float)-1.0F, (float)1.0F,
                            (float)DG_ClipNear, (float)DG_ClipFar );
   _sceVu0MulMatrix( &chanl->eye_pers_no_offset, &chanl->pers_no_offset, &chanl->eye_inv );
   //BP_CAMERA - set extra matrices that do not include camera tweak offset
}
void		DG_SetCamera2( DG_CHANL *chanl, FVECTOR *from, FVECTOR *to, float screen )
{
#if 0
	static	FVECTOR	Lower = { 0.0F, -1.0F, 0.0F, 1.0F } ;
	FVECTOR		z_vec, y_vec, x_vec ;
	FMATRIX		*mat, *mat2 ;

	chanl->screen = screen ;

	mat = &( chanl->eye ) ;
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
	/*
		カメラ逆行列を計算する
	*/
	mat2 = &( chanl->eye_inv ) ;
	_sceVu0InversMatrix( mat2, mat );

	/* Ｚ回転 */
	_sceVu0RotMatrixZ( mat2, mat2, from->vw ) ;
	_sceVu0InversMatrix( mat, mat2 ) ;

	/*
		透視変換行列を生成する
	*/
#if 0
	_sceVu0ViewScreenMatrix( &chanl->pers,
						   chanl->screen,
						   ASPECT_X(), ASPECT_Y() * chanl->width / chanl->height, /* アスペクト比 */
						   0.0F, 0.0F, /* ＸＹオフセット値 */
						   (float)-1.0F, (float)1.0F, /* 変換後Ｚ範囲 */
						   (float)DRAW_NEAR_CLIP, (float)DRAW_FAR_CLIP); /* Ｚクリップ */
	_sceVu0ViewScreenMatrix( &chanl->pers2,
						   chanl->screen * ( chanl->width / 2 ),
						   ASPECT_X(), ASPECT_Y(), /* アスペクト比 */
						   2048.0F, 2048.0F, /* ＸＹオフセット値 */
						   (float)DRAW_Z_MIN, (float)DRAW_Z_MAX, /* 変換後Ｚ範囲 */
						   (float)DRAW_NEAR_CLIP, (float)DRAW_FAR_CLIP); /* Ｚクリップ */
	_sceVu0ViewScreenMatrix( &chanl->raise_pers,
						   chanl->screen,
						   ASPECT_X(), ASPECT_Y() * chanl->width / chanl->height, /* アスペクト比 */
						   0.0F, 0.0F, /* ＸＹオフセット値 */
						   (float)-1.0F, (float)1.0F, /* 変換後Ｚ範囲 */
						   (float)DRAW_NEAR_CLIP+0.05f, (float)DRAW_FAR_CLIP+0.0f); /* Ｚクリップ */
	_sceVu0ViewScreenMatrix( &chanl->raise_pers2,
						   chanl->screen * ( chanl->width / 2 ),
						   ASPECT_X(), ASPECT_Y(), /* アスペクト比 */
						   2048.0F, 2048.0F, /* ＸＹオフセット値 */
						   (float)DRAW_Z_MIN, (float)DRAW_Z_MAX, /* 変換後Ｚ範囲 */
						   (float)DRAW_NEAR_CLIP+0.05f, (float)DRAW_FAR_CLIP+0.0f); /* Ｚクリップ */
#else
	_sceVu0ViewScreenMatrix( &chanl->pers,
						   chanl->screen,
						   ASPECT_X(), ASPECT_Y() * chanl->width / chanl->height, /* アスペクト比 */
						   0.0F, 0.0F, /* ＸＹオフセット値 */
						   (float)-1.0F, (float)1.0F, /* 変換後Ｚ範囲 */
						   (float)DG_ClipNear, (float)DG_ClipFar); /* Ｚクリップ */
	_sceVu0ViewScreenMatrix( &chanl->pers2,
						   chanl->screen * ( chanl->width / 2 ),
						   ASPECT_X(), ASPECT_Y(), /* アスペクト比 */
						   2048.0F, 2048.0F, /* ＸＹオフセット値 */
						   (float)DRAW_Z_MIN, (float)DRAW_Z_MAX, /* 変換後Ｚ範囲 */
						   (float)DG_ClipNear, (float)DG_ClipFar); /* Ｚクリップ */
	_sceVu0ViewScreenMatrix( &chanl->raise_pers,
						   chanl->screen,
						   ASPECT_X(), ASPECT_Y() * chanl->width / chanl->height, /* アスペクト比 */
						   0.0F, 0.0F, /* ＸＹオフセット値 */
						   (float)-1.0F, (float)1.0F, /* 変換後Ｚ範囲 */
						   (float)DG_ClipNear+0.05f, (float)DG_ClipFar+0.0f); /* Ｚクリップ */
	_sceVu0ViewScreenMatrix( &chanl->raise_pers2,
						   chanl->screen * ( chanl->width / 2 ),
						   ASPECT_X(), ASPECT_Y(), /* アスペクト比 */
						   2048.0F, 2048.0F, /* ＸＹオフセット値 */
						   (float)DRAW_Z_MIN, (float)DRAW_Z_MAX, /* 変換後Ｚ範囲 */
						   (float)DG_ClipNear+0.05f, (float)DG_ClipFar+0.0f); /* Ｚクリップ */
#endif

	/*
		カメラ透視変換行列を生成する
	*/
	_sceVu0MulMatrix( &chanl->eye_pers, &chanl->pers, &chanl->eye_inv );
	_sceVu0MulMatrix( &chanl->eye_pers2, &chanl->pers2, &chanl->eye_inv );
	_sceVu0MulMatrix( &chanl->raise_eye_pers, &chanl->raise_pers, &chanl->eye_inv );
	_sceVu0MulMatrix( &chanl->raise_eye_pers2, &chanl->raise_pers2, &chanl->eye_inv );
#else
	FMATRIX		mat, mat2 ;
	DG_MakeCameraMatrix( &mat, from, to );
	/*
		カメラ逆行列を計算する
	*/
	_sceVu0InversMatrix( &mat2, &mat );

	/* Ｚ回転 */
	_sceVu0RotMatrixZ( &mat2, &mat2, from->vw ) ;
	_sceVu0InversMatrix( &mat, &mat2 ) ;
	/* カメラ設定 */
	DG_SetCamera( chanl, &mat, screen );
#endif

}
/*----------------------------------------------------------------*/

	/*
		フレーム初期化パケット接続チャンネル
	*/
void		DG_FrameChanl( DG_CHANL *cp, int which )
{
	struct frame_dma {/* ＤＭＡパケット */
		DG_DMATAG				dmatag ;
		struct frame_gif{/* ＧＩＦパケット */
			DG_GIFTAG			giftag ;
			struct frame_data{/* 転送データ本体 */
				sceGsClear		clear ;
				DG_GSREG		fogcol ;
				DG_GSREG		tex2 ;
				DG_GSREG		scanmsk ;
			}data ;
		} gif ;
	} *dma ;
	void	*top_addr ;
	int		size ;
	DG_DMATAG	*dmatag ;

	MARK( "frame.c" );
	/* ＤＭＡバッファオープン(GIF) */
#if 0
	DG_OpenDmaTask( DG_OPEN_DMA_GIF, NULL, 0 );
#else
	DG_OpenDmaTask( DG_OPEN_DMA_VIF1, NULL, 0 );
#endif
	FlushCache( 0 );
	top_addr = SCRPAD_ADDR;

	/*
		初期化パケットの設定
	*/
#if 0
	*(u_long64*)&cp->draw_env[DG_Clock].datas.xyoffset1 = 
	  SCE_GS_SET_XYOFFSET( ( ( 2048 - DRAW_WIDTH / 2 - cp->offset_x ) << 4 ),
						  ( ( 2048 - DRAW_HEIGHT / 2 - cp->offset_y ) << 4 ) + ( DG_Field ? 8 : 0 ) ) ;
#endif

	dmatag = top_addr ;

	/* 描画終了待ち */
	dmatag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 );
	dmatag->vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	dmatag->vifcode[1] = SCE_VIF1_SET_FLUSHA( 0 );

	/* デフォルトの描画環境設定 */
	dma = DG_PopDefaultDrawEnv( cp, top_addr );

   {
      SBP_SetViewport *pCmd = (SBP_SetViewport *) BP_RB_Alloc( sizeof( SBP_SetViewport ) );

      pCmd->x = cp->offset_x; // + DRAW_WIDTH / 2;
      pCmd->y = cp->offset_y; // + DRAW_HEIGHT / 2;
      pCmd->width = cp->width;
      pCmd->height = cp->height;
      BP_RB_AddCommand( kCmd_SetViewport, (char*)pCmd );
   }

	/* 仮面消去パケット生成 */
	if ( cp->bg_clear_flag )
   {
		/*
			画面消去付き
		*/

		/* ＤＭＡタグ設定 */
		dma->dmatag.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD( struct frame_gif ) );
		dma->dmatag.vifcode[0] = SCE_VIF1_SET_NOP(0);
		dma->dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD( struct frame_gif ), 0 );

		/* ＧＩＦタグ設定 */
		dma->gif.giftag.tag = SCE_GIF_SET_TAG( SIZEOF_QWORD(struct frame_data), 1, 0, 0, 0, 1);
		dma->gif.giftag.regs = 0x0e ;

		/* データ本体設定 */
#if 0 //BP_PS2
		sceGsSetDefClear( &dma->gif.data.clear, SCE_GS_ZGEQUAL,
						 2048-cp->width/2, 2048-cp->height/2, cp->width, cp->height,
						 DG_FogColor.r, DG_FogColor.g, DG_FogColor.b, 0, 0 );
#else
      memset(&dma->gif.data.clear, 0, sizeof(dma->gif.data.clear));
#endif
		if ( cp->bg_clear_flag == 2 )
      {
			/* Ｚバッファのみ更新する */
			dma->gif.data.clear.testa.ATE = 1 ;
			dma->gif.data.clear.testa.ATST = 0 ;/*NEVER*/
			dma->gif.data.clear.testa.AFAIL = 2 ;/*ZB_ONLY*/
		}
		
      dma->gif.data.clear.testb.ATE = 1 ;
		dma->gif.data.clear.testb.ATST = 5/*GEQUAL*/ ;
		dma->gif.data.clear.testb.AREF = 64 ;
		dma->gif.data.clear.testb.AFAIL = 1/*FB_ONLY*/ ;

		dma->gif.data.fogcol.reg = SCE_GS_FOGCOL ;
		dma->gif.data.fogcol.data = *(int*)&DG_FogColor ;
		/* CLUTバッファロード位置初期化用 */
		dma->gif.data.tex2.reg = SCE_GS_TEX2_1 ;
		dma->gif.data.tex2.data = SCE_GS_SET_TEX2(SCE_GS_PSMT8,0,0,0,0,2) ;
		/* 原因不明スキャンマスク設定の設定されっぱなし不具合回避 */
		dma->gif.data.scanmsk.reg = SCE_GS_SCANMSK ;
		dma->gif.data.scanmsk.data = 0 ;

		/* スクラッチパッドからメインメモリへ書き出し */
		size = ( (int)&dma[1] - (int)top_addr  ) / sizeof(u_long128) ;
		DG_StartSprToMem( DG_CurrentDmaAddr, SCRPAD_ADDR, size );
		DG_CurrentDmaAddr += size ;
   
      {
         // This is the format the receiving end expects the data to be in!
         unsigned int fogColor = DG_FogColor.r | DG_FogColor.g << 8 | DG_FogColor.b << 16;

         // Add clear packet
         {
            SBP_ClearViewport* pCmd = (SBP_ClearViewport*)BP_RB_Alloc(sizeof(SBP_ClearViewport));
            pCmd->clearColor = fogColor;
            pCmd->clearFlags = cp->bg_clear_flag;
            BP_RB_AddCommand(kCmd_ClearViewport, (char*)pCmd);
         }

         // Add frame init packet
         {
            SBP_FrameInitPacket* pCmd = (SBP_FrameInitPacket*)BP_RB_Alloc(sizeof(SBP_FrameInitPacket));
            pCmd->testValue = *(u_long64*)&dma->gif.data.clear.testb;
            pCmd->fogColor = fogColor;
            BP_RB_AddCommand(kCmd_FrameInitPacket, (char*)pCmd);
         }
      }
	} 
   else 
   {
		/*
			画面消去なし
		*/

		/* スクラッチパッドからメインメモリへ書き出し */
		size = ( (int)&dma[0] - (int)top_addr  ) / sizeof(u_long128) ;
		DG_StartSprToMem( DG_CurrentDmaAddr, SCRPAD_ADDR, size );
		DG_CurrentDmaAddr += size ;
	}

	/* ＤＭＡ転送終了待ち */
	DG_EndSprToMem();

	/* ＤＭＡバッファクローズ */
	DG_CloseDmaTask();

}
/* ---------------------------------------------- */

/* 表示グループユーティリティ */

/* 表示グループに追加 */
void	DG_AddCurrentGroup( int id )
{
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
