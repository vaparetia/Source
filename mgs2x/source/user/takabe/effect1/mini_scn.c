//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mini_scn.c
	ゲームオーバー用画面縮小化エフェクト

	2001/06/10 K.Takabe
	$Id: mini_scn.c,v 1.2 2002/12/23 10:42:10 takaki Exp $

*/
/*

	void *NewReduceScreenEffect( void )

		画面縮小化キャラを起動する
		制御に使用するのでワークアドレスを保持する必要があるので注意！


	void TAKABE_SetReduceScreenSize( void *work_ptr, int x1, int y1, int x2, int y2, int color );
	void	*work_ptr ;		確保したキャラのワークアドレス
	int		x1 ;			仮想座標上での左上Ｘ座標
	int		y1 ;			仮想座標上での左上Ｙ座標
	int		x2 ;			仮想座標上での右下Ｘ座標
	int		y2 ;			仮想座標上での右下Ｙ座標
	int		color ;			描画色（ＲＧＢＡ）

		画面縮小キャラに対して描画領域を指定する
		仮想座標は512x384


	void TAKABE_SetReduceScreenSizeF( void *work_ptr, float x1, float y1, float x2, float y2, int color );

		実数指定バージョン

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"

#include "BP_RenderFX.h"
#include "BP_RendererDebug.h"
#include "BP_RenderBufferTypes.h"

#define CLOCK_COUNT	(BP_BASE_TICK())

#define DRAW_DIV	(16)		/* 描画の分割数 */

/* ---------------------------------------------------------------- */
#define MAX_DRAW	(8)		/* ２以上 */

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;

	DG_DMAPACK	*dmapack ;
	void		*packet_mem ;

	float		x1, y1 ;
	float		x2, y2 ;
	int			color ;

} Work ;

/* ---------------------------------------------------------------- */
/*
	ＸＢＯＸ用ルーチン
*/
typedef struct {
	/* 初期化パケット */
	DG_DMAPACK_PARAM		frame_backup ;

#ifdef KP_WINDOWS
	/* Windows用処理 */
	DG_DMAPACK_PARAM		rend_backbuffer2frame ;
#endif

	/* メイン描画パケット */
	DG_DMAPACK_PARAM		disable_atest ;
	DG_DMAPACK_PARAM		use_frametex ;
	DG_DMAPACK_SPRT			sprt ;
	DG_DMAPACK_PARAM		enable_atest ;

	/* 変更環境の復元パケット */

	DG_DMAPACK_PARAM		end ;
} ALL_PACKET ;

#ifdef KP_WINDOWS
extern	DWORD	DG_SkipRendBackBuffer2Frame ;
#endif

/* ---------------------------------------------------------------- */
static void InitDmaPacket( ALL_PACKET *packet, int which )
{
	/* 初期化パケットの初期化 */
	DG_SetDmapackBackupFrame( &packet->frame_backup, 0 );
#ifdef KP_WINDOWS
	DG_SetDmapackRendBackBuffer2Frame( &packet->rend_backbuffer2frame );
#endif
	DG_SetDmapackUseFrameTex( &packet->use_frametex, 2 );
	DG_SetDmapackModeEnable( &packet->disable_atest, DG_DMAPACK_MODE_NO_ALPHATEST );
	DG_SetDmapackModeDisable( &packet->enable_atest, DG_DMAPACK_MODE_NO_ALPHATEST );
	/* 終了パケットの初期化 */
	DG_SetDmapackEnd( &packet->end );
}

/* 入力は実座標系なので注意！ */
static void SetScreenDrawArea( void *screen_draw, float x1, float y1, float x2, float y2, int rgba )
{
	DG_SetDmapackSprt( screen_draw,
					  x1, y1, DG_FRAME_U( 0 ), DG_FRAME_V( 0 ),
					  x2, y2, DG_FRAME_U( DRAW_WIDTH ), DG_FRAME_V( DRAW_HEIGHT ),
					  DG_MakeDmaPackColorFromInt(rgba) );

}

/* ---------------------------------------------------------------- */

static void SetParam( Work *work, ALL_PACKET *packet, int which, int first_flag )
{

	if ( first_flag ){
		InitDmaPacket( packet, which );
	}

	{/* 仮想座標を実座標に変換して描画領域の設定を行う */
		float	x1, y1, x2, y2 ;
		x1 = work->x1 * DRAW_WIDTH / 512.0f ;
		y1 = work->y1 * DRAW_HEIGHT / 384.0f ;
		x2 = work->x2 * DRAW_WIDTH / 512.0f ;
		y2 = work->y2 * DRAW_HEIGHT / 384.0f ;
		SetScreenDrawArea( &packet->sprt, x1, y1, x2, y2, work->color );
	}

}


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	SetParam( work, work->dmapack->autopacket, DG_Clock, 0 );
	GM_BlurDisableFlag = 1 ;	/* 不具合がでるのでブラーを禁止する */
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
#ifdef KP_WINDOWS
	if( DG_SkipRendBackBuffer2Frame ){ DG_SkipRendBackBuffer2Frame-- ; }
#endif
	/* パケットメモリ開放 */
	if ( work->packet_mem != NULL ) GV_DelayedFree( work->packet_mem );
	/* ＤＭＡパケットオブジェクト開放 */
	if ( work->dmapack != NULL ){
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
	GM_BlurDisableFlag = 0 ;	/* ブラー禁止を解除 */
}

/* ---------------------------------------------------------------- */
static void *_reducescreen_buffer_callback(void *pParam, DG_DMAPACK *pDstDmaPack)
{
   return DG_AS_SceneBufferAllocCopy(pParam, sizeof(Work));
}

static void BP_ReduceScreenCallback(void* pWork)
{
   Work *work = (Work*)pWork;
   SBP_ReduceScreen* pPacket = (SBP_ReduceScreen*)BP_RB_Alloc(sizeof(SBP_PFX_Blinds));

   pPacket->x1 = work->x1 * DRAW_WIDTH / 512.0f ;
   pPacket->y1 = work->y1 * DRAW_HEIGHT / 384.0f ;
   pPacket->x2 = work->x2 * DRAW_WIDTH / 512.0f ;
   pPacket->y2 = work->y2 * DRAW_HEIGHT / 384.0f ;
   pPacket->color = work->color;
   BP_RB_AddCommand(kCmd_PostFx_ReduceScreen, (char*)pPacket);
}

static int GetResources( Work *work, int phase, int prio )
{
	DG_DMAPACK	*dmapack ;
	ALL_PACKET	*packet ;
	int		i ;

#ifdef KP_WINDOWS
	DG_SkipRendBackBuffer2Frame++ ;
#endif

	/* ＤＭＡパケット型オブジェクト作成 */
	//work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU, DG_DMAPACK_PHASE_FIRST, 0 );
	work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU, phase, prio );
   
#if BP_VITA
   DG_AS_DMAPackSetCallbacks(dmapack, BP_ReduceScreenCallback, _reducescreen_buffer_callback);
   dmapack->BP_callbackParam = work;
#endif

	if ( dmapack == NULL ) return ( -1 );
	DG_QueueDmapack( dmapack );

	/* パケットメモリ割り当て */
	work->packet_mem = packet = GV_Malloc( sizeof(ALL_PACKET) );
	if ( packet == NULL ) return ( -1 );
	dmapack->autopacket = packet ;
	//dmapack->packet[0] = &packet[0] ;
	//dmapack->packet[1] = &packet[1] ;

	InitDmaPacket( packet, 1 );

	work->color = 0x80808080 ;
	work->x1 = 0 ;
	work->y1 = 0 ;
	work->x2 = 512 ;
	work->y2 = 384 ;
	SetParam( work, packet, 0, 1 );

	Act( work );

	return (0);
}

/* ---------------------------------------------------------------- */
/* プログラム呼び出しインターフェイス */
void *NewReduceScreenEffect( void )
{
	Work		*work ;

	OPERATOR() ;
	//work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, DG_DMAPACK_PHASE_FIRST, 0 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

/* プログラム呼び出しインターフェイス */
void *NewReduceScreenEffectEx( int phase, int prio )
{
	Work		*work ;

	OPERATOR() ;
	//work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, phase, prio ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


/* ---------------------------------------------------------------- */
void TAKABE_SetReduceScreenSize( void *work_ptr, int x1, int y1, int x2, int y2, int color )
{
	Work	*work ;

	if ( work_ptr == NULL ) return ;
	work = work_ptr ;
	work->x1 = x1 ;
	work->y1 = y1 ;
	work->x2 = x2 ;
	work->y2 = y2 ;
	work->color = color ;
}

void TAKABE_SetReduceScreenSizeF( void *work_ptr, float x1, float y1, float x2, float y2, int color )
{
	Work	*work ;

	if ( work_ptr == NULL ) return ;
	work = work_ptr ;
	work->x1 = x1 ;
	work->y1 = y1 ;
	work->x2 = x2 ;
	work->y2 = y2 ;
	work->color = color ;
}
