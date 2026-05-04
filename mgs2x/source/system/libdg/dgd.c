//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	dgd.c
	ライブラリ制御デーモン

	1999/07/07 K.Takabe
	$Id: dgd.c,v 1.1.1.3 2002/11/19 11:42:06 Yoshizawa1 Exp $

*/
/*

	void		DG_ResetSystem()

		システムリセット（ステージ切替え時など）

	void		DG_StartDaemon()

		システム起動（ゲームスタート時一度だけ）
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

#include	"mts.h"
#include	"libgv.cnf"
#include	"libgv.h"
//BP_PS2 #include	"break.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"
#include "bp_threading.h"
#include "BP_Renderer.h"

//#include	"game.h"

#ifndef PAL
#define GS_DISP_MODE	(SCE_GS_NTSC)
#else
#define GS_DISP_MODE	(SCE_GS_PAL)
#endif

extern int gEndFrameIsThreaded;

/*----------------------------------------------------------------*/

/* １回の処理にかかったフレーム数記録カウンタ */
static int		DG_PassageFrame = 0 ;
#if 0
static volatile int DG_VSyncCount ;
#endif

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

#ifdef ARMATURE_RENDER_FRAME_SKIP
static int Arm_DrawSkipClock = 0;
int DG_Arm_SkipThisFrame()
{
   return Arm_DrawSkipClock != 0;
}
#else
int DG_Arm_SkipThisFrame()
{
   return 0;
}
#endif

/* 表示位置オフセット */
//static int	DG_DispAdjustX = 0 ;
//static int	DG_DispAdjustY = 0 ;
static u_long64	DG_GSDisplay1 = 0 ;			/* ＧＳレジスタ設定値 */
static u_long64	DG_GSDisplay2 = 0 ;			/* ＧＳレジスタ設定値 */

static int vin_handler;
static int vout_handler;

float DG_ACC;

/*----------------------------------------------------------------*/

extern int DG_Field ;

/*----------------------------------------------------------------*/

typedef	struct	{
	GV_ACT		actor ;
} Work ;

/*----------------------------------------------------------------*/

#ifndef	NO_PROTOTYPE
static	void	Act1( Work * ) ;
static	void	Act2( Work * ) ;
#endif

/* ハイレゾテキストウィンドウ対応 */
//#define HIRESO_TEXT
#ifndef PAL
#define DISP_Y_OFFSET	(50)
#else
#define DISP_Y_OFFSET	(60)
#endif

/*----------------------------------------------------------------*/
void DG_SetDisplayMode( int which ){
	static u_long64	def_dispfb1 = (FRAME_BUFFER_COLOR_MODE()<<15)|((BUFFER_WIDTH>>6)<<9)|(BUFFER_PAGE(0)/2048);
	static u_long64	def_dispfb2 = (FRAME_BUFFER_COLOR_MODE()<<15)|((BUFFER_WIDTH>>6)<<9)|(BUFFER_PAGE(1)/2048);
	//static u_long64	def_display1 = ( (u_long64)(DRAW_HEIGHT*1-1) << 44 ) | ((u_long64)0x9ff<<32)
	//  | ((( 2560+DRAW_WIDTH-1)/DRAW_WIDTH-1) << 23 ) | ((DISP_Y_OFFSET)<<12) | (0x27c+(0*(2560/DRAW_WIDTH))) ;
	//static u_long64	def_display2 = ( (u_long64)(DRAW_HEIGHT*1-1) << 44 ) | ((u_long64)0x9ff<<32)
	//  | ((( 2560+DRAW_WIDTH-1)/DRAW_WIDTH-1) << 23 ) | ((DISP_Y_OFFSET+1)<<12) | (0x27c+(0*(2560/DRAW_WIDTH))) ;
	//static u_long64	def_display3 = ( (u_long64)(DRAW_HEIGHT*1-1) << 44 ) | ((u_long64)0x9ff<<32)
	//  | ((( 2560+DRAW_WIDTH-1)/DRAW_WIDTH-1) << 23 ) | ((DISP_Y_OFFSET-1)<<12) | (0x27c+(0*(2560/DRAW_WIDTH))) ;
	static int LastWhich = 0 ;
	static int flag = 0 ;
	u_long64	dispfb, display1, display2 ;

	if ( which == -1 ) which = LastWhich ;
	dispfb = ( ( which & 1 ) == 0 ) ? def_dispfb1 : def_dispfb2 ;

	DG_Field = 0 ;
	//display1 = def_display1 ;
	//display2 = def_display2 ;
	display1 = DG_GSDisplay1 ;
	display2 = DG_GSDisplay2 ;
#if 0
	if ( GV_PadData[1].status & PAD_B ) {
		display1 = display2 = def_display1 ;
	}
#endif

#if 0 //BP_PS2
	if ( flag == 0 ){
		DPUT_GS_PMODE( 0x0000000000008027 );
		DPUT_GS_SMODE2( 0x01 );
		DPUT_GS_DISPFB1( dispfb );
		DPUT_GS_DISPLAY1( display1 );
		DPUT_GS_DISPFB2( dispfb );
		DPUT_GS_DISPLAY2( display2 );
		DPUT_GS_BGCOLOR( 0 );
		flag = 1 ;
	} else {
		DPUT_GS_DISPFB1( dispfb );
		DPUT_GS_DISPFB2( dispfb );
		DPUT_GS_DISPLAY1( display1 );
		DPUT_GS_DISPLAY2( display2 );
	}
#else
   //BP_RENDER This switches back and forth between two back buffers?
   BP_TRIVIAL_BREAK;
#endif
	LastWhich = which ;
}

/*----------------------------------------------------------------*/

static MTS_EVENTFLAG dg_evflag_body;
static MTS_EVENTFLAG *dg_evflag;
extern void DG_DumpDmaStatus( void );

#define DG_VSYNC_DONE	0x0001
#define DG_DMA_DONE		0x0002

static void InitEventFlag( void )
{
	dg_evflag_body.value = 0;
	dg_evflag = MTS_CreateEventFlag( &dg_evflag_body );
}

static void WaitAllDone( void )
{
	int res;
	int flag;
	int	value ;

	flag = DG_VSYNC_DONE;
	if( !DG_DmaCheckEnd() ){
		flag |= DG_DMA_DONE;
	}
	MARK( "WAITDV" );
	value = dg_evflag->value ;
	//res = MTS_WaitEventFlag( dg_evflag, flag
	//						 , MTS_EW_MODE_AND | MTS_EW_MODE_CLEAR, 300 * ONE_V_HSYNC );
	res = MTS_WaitEventFlag( dg_evflag, flag
							 , MTS_EW_MODE_AND, 300 * ONE_V_HSYNC );
	value = dg_evflag->value ;
	dg_evflag->value = 0 ;
	dg_evflag->wait_threadid = 0;
	if( res == MTS_EW_NO_WAIT ){
		// すでに終了していてねる必要がない場合でも少しスリープする
		MARK( "WAIT3H" );
		MTS_DelayThread( 3 );
	} else if( res == MTS_EW_TIMEOUT ){

		printf( "TIMEOUT !!\n" );
		DG_DumpDmaStatus();
		// DMA MISS ??
#ifdef DEBUG_MODE
#if 1
		if( flag & DG_DMA_DONE ){
			if( !DG_DmaCheckEnd() ){
				HANGUP();
			}
		}
#endif
#endif

#if 0 //BP_PS2
		DI();
		{
			int stat;
			stat = DGET_D_STAT();
			DPUT_D1_CHCR( stat & ( ~0x100 ) );
			sceDevVif1Reset();
			sceDevVu1Reset();
		}
		EI();
#endif
	}
	MARK( "WAKEUP" );
	//scePrintf("WaitAllDone end\n" );
}

#if 0 //BP_PS2
static void WaitVSync( int frame )
{
	int res, flag;

	flag = DG_VSYNC_DONE;
	for( ;; ){
		MTS_ClearEventFlag( dg_evflag );		// 次のVSyncまで待つ
		MARK( "WAITV" );
		res = MTS_WaitEventFlag( dg_evflag, flag
							 , MTS_EW_MODE_AND | MTS_EW_MODE_CLEAR, 120 * ONE_V_HSYNC );
		if( res == MTS_EW_TIMEOUT ){
			printf( "TIMEOUT !!\n" );
			DG_DumpDmaStatus();
			// DMA MISS ??

#if 0 //BP_PS2
			DI();
			{
				int stat;
				stat = DGET_D_STAT();
				DPUT_D1_CHCR( stat & ( ~0x100 ) );
				sceDevVif1Reset();
				sceDevVu1Reset();
			}
			EI();
#endif
			break;
		}
		if( DG_PassageFrame >= frame && DG_DmaCheckEnd() ) break;
	}
	MARK( "WAKEUP" );
}
#endif

void DG_iVsyncDone( void )
{
#if 0 //BP_PS2
	MTS_iSetEventFlag( dg_evflag, DG_VSYNC_DONE );
	//scePrintf("iVsyncDone\n" );
#endif
}

void DG_iDmaDone( void )
{
#if 0 //BP_PS2

	MTS_iSetEventFlag( dg_evflag, DG_DMA_DONE );
	//scePrintf("iDmaDone\n" );
#endif
}

/*----------------------------------------------------------------*/

//static long64 timeout;

static	int	VSyncInCallback( int cause )
{
#if 0
	if ( cause != INTC_VBON ){
		ExitHandler();	/* EEバグ回避用 */
		return (0) ;
	}
#endif
	//scePrintf("vsync\n" );
	//DG_Field ^= 1 ;

	DG_Field = ((DGET_GS_CSR() >> 13) & 0x1) ;

	DG_SetDisplayMode( -1 );

	DG_PassageFrame++ ;
#if 0
	DG_VSyncCount++ ;
#endif
	DG_TickCount++ ;

	DG_iVsyncDone();

	DG_VSyncStatus = 1;	// 帰線期間

#if 0 //BP_PS2
	ExitHandler();	/* EEバグ回避用 */
#endif
	return (0);
}

#include "cdbios.h"
#include "sd_ee.h"

static int VSyncOutCallback( int cause, void *arg, void *addr )
{
	DG_VSyncStatus = 0;	// 表示期間
#ifdef DEBUG_MODE
	if( GV_PadHangupCheck() ){
		excep_printf( "PAD HANGUP in %08X CD %X SD %X PAD %X\n", addr, cdbios_get_status(), sd_status()[1], GV_PadData[ 0 ].flag );
		HANGUP();
	}
#endif
#if 0 //BP_PS2
	ExitHandler();
#endif
	return 0;
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

#ifdef ARMATURE_RENDER_FRAME_SKIP
   if ( BP_Renderer_MGS2_3060_IsAt60() )
   {
      // Make sure to force the skip clock to 0 when we transition to 60, otherwise
      // we could always be drawing "skip" frames!
      Arm_DrawSkipClock = 0;
   }
   else
   {
      Arm_DrawSkipClock = 1 - Arm_DrawSkipClock;
      if ( Arm_DrawSkipClock != 0 )
      {
   //      DG_StartFrame_Skip();
         return;
      }
   }
#endif // ARMATURE_RENDER_FRAME_SKIP

#if 0	// B

	/* 処理落ち対処なし */
	do {
		DG_VSyncCount = 0 ;
		while ( (volatile)DG_VSyncCount == 0 );
		//DG_Field = sceGsSyncV( 0 ) ;/* ＶＳｙｎｃ割り込みと一緒に使用すると落ちる！！ */
	} while ( DG_DmaCheckEnd() == 0 );

#else	// B

	/* フレーム途中切り替え方式 */
	DG_LastActTime = DIFF_HSYNC_TIMER( GET_HSYNC_TIMER(), DG_FrameStartTime );
#if 0	// POLLING version
	/* 通常のウェイト処理 */
	if ( DG_PassageFrame == 0 ){
		while( *(volatile int *)&DG_PassageFrame == 0 );
	}
	while ( DG_DmaCheckEnd() == 0 ) ;
#else	// SLEEP VERSION

#if 0 //BP_RENDER
	/* MTSを使用したスレッドスリープによるウェイト処理 */
	if( DG_FrameCount == 0 ){
		// フレーム途中切替え
		WaitAllDone();
	} else {
		// VSyncまち
		WaitVSync( DG_FrameCount );
	}
#else
   bp_yield_ps2_thread_to_vsync();
#endif

#endif	// SLEEP VERSION

#endif	// B

	DG_PassageFrame = 0 ;
	/* 経過時間チェック（割り込みを禁止していないので誤差がでるかも） */
	DG_PassageTick = DG_TickCount - DG_OldTickCount ;
	DG_OldTickCount = DG_TickCount ;
	if ( DG_PassageTick < 0 ) DG_PassageTick = 1 ;

	DG_StartFrame() ;

	DG_FrameStartTime = GET_HSYNC_TIMER();
}

	/*
		フレーム終了処理プロセス
	*/
static	void	Act2( Work *work )
{
	OPERATOR() ;
#ifdef ARMATURE_RENDER_FRAME_SKIP
   if ( Arm_DrawSkipClock != 0 )
   {
      extern void DG_EndFrame_Skip();

      DG_EndFrame_Skip();
      return ;
   }
#endif // ARMATURE_RENDER_FRAME_SKIP
	DG_EndFrame() ;
}

/*----------------------------------------------------------------*/

void		DG_ResetSystem()
{
	DG_InitLightSystem() ;
	DG_InitFrameSystem() ;
	DG_FrameCount = 0 ;

	//DG_ReloadPalette();
	//DG_ResetPaletteEffect();
	//DG_SetBackGroundColor( 0, 0, 0 );
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
	//DG_InitTextureSystem() ;
	//DG_ResetResidentTexture();
	DG_InitTextureCache();
}

void		DG_StartDaemon()
{
	static	Work	Work1, Work2 ;

	OPERATOR() ;
	DG_Clock = 0 ;

#if 0 //BP_PS2

#ifndef HIGHRESO_FFI
#ifndef HALFRESO_NI
	sceGsResetGraph(0, SCE_GS_INTERLACE, GS_DISP_MODE, SCE_GS_FRAME);
#else
	sceGsResetGraph(0, SCE_GS_NOINTERLACE, GS_DISP_MODE, SCE_GS_FRAME);
#endif
#else
	sceGsResetGraph(0, SCE_GS_INTERLACE, GS_DISP_MODE, SCE_GS_FIELD);
#endif

#endif

	DG_SetDisplayOffset( 0, 0, 0 );
	DG_DmaReset();
	DG_InitChanlSystem( 0 ) ;
	DG_ResetSystem() ;
	InitEventFlag();

#if 0 //BP_PS2
//	sceGsSyncVCallback( VSyncInCallback ) ;
	vin_handler = AddIntcHandler( INTC_VBLANK_S, VSyncInCallback, -1 );
	vout_handler = AddIntcHandler2( INTC_VBLANK_E, VSyncOutCallback, -1, NULL );
	EnableIntc( INTC_VBLANK_S );
	EnableIntc( INTC_VBLANK_E );
#endif
	//sceMpegInit();
	/*
		ロード初期化ルーチンの登録
	*/
	GV_SetLoader( 'k' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitKms ) ;
	GV_SetLoader( 't' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitTri ) ;
	GV_SetLoader( 'l' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitLt2 ) ;
	GV_SetLoader( 'c' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitCvd ) ;
	GV_SetLoader( 'e' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitEvm ) ;
	GV_SetLoader( 'z' | LOADER_CALL_RESIDENT, (GV_LOADFUNC)DG_LoadInitZar ) ;

	/*
		ＤＧライブラリデーモンは、２プロセス使用
	*/
	GV_InitActor( GV_ACTOR_DAEMON, &Work1, NULL ) ;
	GV_SetActor( &Work1, Act1, NULL ) ;
	GV_InitActor( GV_ACTOR_DAEMON2, &Work2, NULL ) ;
	GV_SetActor( &Work2, Act2, NULL ) ;

	{/* デバッグルーチン */
#if 0 //BP_PS2
		extern qword Vu1DrawObject2 ;
		extern qword Vu1DrawObject3 ;
		extern qword Vu1DrawShadowObject2 ;
		extern int		Vu1DrawMultiWeight3[] ;
		printf("vu1 micro program size(normal model)       : %08x\n", ((DG_DMATAG*)Vu1DrawObject2)->qwc & 0x7fff );
		printf("vu1 micro program size(multi texture model): %08x\n", ((DG_DMATAG*)Vu1DrawObject3)->qwc & 0x7fff );
		printf("vu1 micro program size(shadow object model): %08x\n", ((DG_DMATAG*)Vu1DrawShadowObject2)->qwc & 0x7fff );
		printf("vu1 micro program size(multi weight model) : %08x\n", ((DG_DMATAG*)Vu1DrawMultiWeight3)->qwc & 0x7fff );
#endif
	}

}


void DG_VramClear( void )
{
#if 0 //BP_PS2
   DG_DMATAG	dma_buffer[ 128 ], *tag;
	void		*mem_addr ;
	int			i ;

	printf("vram clear start!!\n");
	while ( sceGsSyncPath( 0, 0 ) ) ;	/* ＤＭＡ終了ウェイト */
	sceDevVif1Reset();
	sceDevVu1Reset();
	sceDmaGetChan( 2 /* GIF */ )->chcr.TTE = 0 ;		/* DMA TAGは転送しない */

	/* メモリの確保 */
	mem_addr = (void*)MEM_ADDR ;	/* ＭＧＳ２システムのワークメモリ領域を使用する */
	GV_ZeroMemory( mem_addr, 256 * 1024 );

	/* ４Ｍのフレームバッファを１６回（２５６ＫＢ単位）に分けてクリア */
	for ( i = 0 ; i < 16 ; i ++ ){
		//printf("%d/16\n", i );
		/* フレームバッファ転送パケット生成 */
		tag = DG_MakeLoadImagePacket( dma_buffer, SCE_GS_PSMCT32, 512, 128, 512*128*i, 512, mem_addr );
		/* ＤＭＡの終端コードの書き込み */
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_END, 0 );
		tag->vifcode[0] = SCE_VIF1_SET_NOP( 0 );
		tag->vifcode[1] = SCE_VIF1_SET_NOP( 0 );

		/* ＶＲＡＭクリアパケットをＧＩＦ経由で転送 */
		FlushCache(0);						/* キャッシュのフラッシュ */
		sceDmaSend( sceDmaGetChan( 2 /* GIF */ ), dma_buffer );

		while ( sceGsSyncPath( 0, 0 ) ) ;	/* ＤＭＡ終了ウェイト */
	}
	printf("vram clear end!!\n");
#endif
}

void DG_SetDisplayOffset( int x, int y, int flag )
{
	DG_GSDisplay1 = ( (u_long64)(DRAW_HEIGHT*1-1) << 44 ) | ((u_long64)0x9ff<<32)
	  | ((( 2560+DRAW_WIDTH-1)/DRAW_WIDTH-1) << 23 ) | ((DISP_Y_OFFSET+y)<<12) | (0x27c+(x*(2560/DRAW_WIDTH))) ;
	DG_GSDisplay2 = ( (u_long64)(DRAW_HEIGHT*1-1) << 44 ) | ((u_long64)0x9ff<<32)
	  | ((( 2560+DRAW_WIDTH-1)/DRAW_WIDTH-1) << 23 ) | ((DISP_Y_OFFSET+y+1)<<12) | (0x27c+(x*(2560/DRAW_WIDTH))) ;

	/* フリッカーフリー禁止 */
	if ( flag & 1 ){
		DG_GSDisplay2 = DG_GSDisplay1 ;
	}
}

void DG_EndDaemon( void )
{
#if 0 //BP_PS2
	RemoveIntcHandler( INTC_VBLANK_S, vin_handler );
	RemoveIntcHandler( INTC_VBLANK_E, vout_handler );

	DG_EndDmaCtrl();
#endif
}
