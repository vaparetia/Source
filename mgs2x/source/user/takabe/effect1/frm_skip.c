//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	frm_skip.c
	フレームスキップエフェクト

	2001/02/15 K.Takabe
	$Id: frm_skip.c,v 1.1.1.3 2002/11/19 11:51:10 Yoshizawa1 Exp $

*/
/*


*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"def_dma.h"
#include	"gameheader.h"
#include	"../../mode/demo/libdemo.h"


#define ALIGNSIZE1(_n) (_n)
#define ALIGNSIZE16(_n) (((_n)+15)&0xfffffff0)
#define ALIGNSIZE64(_n) (((_n)+63)&0xffffffc0)
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)

#define DRAW_DIV	(16)		/* 描画の分割数 */
#define RASTER_NUM	(112)

#if BP_VITA
#define BP_RENDERTARGET_WIDTH (960.0f)
#define BP_RENDERTARGET_HEIGHT (544.0f)
#else
#define BP_RENDERTARGET_WIDTH (1280.0f)
#define BP_RENDERTARGET_HEIGHT (720.0f)
#endif
/* ---------------------------------------------------------------- */
/* プログラムワーク */
typedef	struct	{
	GV_ACT_EX	actor ;
	DG_DMAPACK	*dmapack ;
	int			time ;
	int			count ;
	int			start_flag ;
	void		*texture_memory ;
	int			mode ;
	/* 拡張ワーク */
	int			capture_interval ;		/* 画像取り込み間隔 */
	int			capture_count ;			/*  */
} Work ;

/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
// BP_Render - Use Xbox path
#if defined(PSX2) && 0
/*
	ＰＳ２用ルーチン
*/
typedef struct _screen_draw {
	DG_GIFTAG		giftag ;
	struct _screen_draw_data{
		DG_GSREG	alpha ;
		DG_GSREG	tex0 ;
		DG_GSREG	test1 ;
		DG_GSREG	prim ;
		DG_GSREG	rgbq ;
		struct _sprt_parts{
			DG_GSREG	uv0 ;
			DG_GSREG	xyz0 ;
			DG_GSREG	uv1 ;
			DG_GSREG	xyz1 ;
		} sprt[ DRAW_DIV ];
		DG_GSREG	test2 ;
	} data ;
} SCREEN_DRAW ;

typedef struct {
	DG_MOVEIMAGE		moveimage[4] ;
	DG_DMATAG			dmatag_end ;		/* RETタグ */
} ALL_PACKET ;

/* ---------------------------------------------------------------- */
/* ＤＭＡＰＡＣＫ用パケット初期化 */
static void InitDmaPacket( ALL_PACKET *packet, int which )
{
	packet->dmatag_end.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 ) ;
	packet->dmatag_end.vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 ) ;
	packet->dmatag_end.vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{

	/* テクスチャ取り込み要求とプリミティブの表示を同時に行う */
	if ( work->start_flag ){
		work->dmapack->flag &= ~DG_DMAPACK_INVISIBLEMENU ;
		work->start_flag = 0 ;
	}

	/* 非ポーズ中のみカウンタを進ませる */
	if ( GV_PauseLevel == 0 ){
		work->count -= TIME_BASE + DM_FrameSkip * TIME_BASE ;
		work->capture_count += TIME_BASE + DM_FrameSkip * TIME_BASE ;

		if ( work->count < 0 ) work->count = 0 ;
		if ( work->capture_count > work->capture_interval ){
			work->capture_count -= work->capture_interval ;
			work->dmapack->flag |= DG_DMAPACK_INVISIBLEMENU ;
			work->start_flag = 1 ;
		}

	}

	/* 終了チェック */
	if ( work->count == 0 ) GV_DestroyActor( work );

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	/* ＤＭＡパケットオブジェクト開放 */
	if ( work->dmapack != NULL ){
		/* パケットメモリ開放 */
		if ( work->dmapack->packet[0] != NULL ) GV_DelayedFree( work->dmapack->packet[0] );
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int time )
{
	DG_DMAPACK	*dmapack ;
	ALL_PACKET	*packet ;
	u_int		i ;

	work->time = time ;
	work->count = time ;
	work->start_flag = 1 ;

	/* ＤＭＡパケット型オブジェクト作成 */
	work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU
											 |DG_DMAPACK_INVISIBLEMENU
											 , DG_DMAPACK_PHASE_AFTER, 127 );
	if ( dmapack == NULL ) return ( -1 );
	DG_QueueDmapack( dmapack );

	/* パケットメモリ割り当て */
	packet = GV_Malloc( sizeof(ALL_PACKET) * 2 );
	if ( packet == NULL ) return ( -1 );
	GV_ZeroMemory( packet, sizeof(ALL_PACKET) * 2 );
	dmapack->packet[0] = &packet[0] ;
	dmapack->packet[1] = &packet[1] ;

	for ( i = 0 ; i < 2 ; i++ ){
		DG_MakeMoveImagePacket( &packet[i].moveimage[0],
							   BUFFER_PAGE(i), 64, SCE_GS_PSMCT32, 0, 0,
							   BUFFER_PAGE(1-i), 64, SCE_GS_PSMCT32, 0, 0,
							   64, DRAW_WIDTH*DRAW_HEIGHT/64/4, 0 );
		DG_MakeMoveImagePacket( &packet[i].moveimage[1],
							   BUFFER_PAGE(i) + DRAW_WIDTH*DRAW_HEIGHT/4*1, 64, SCE_GS_PSMCT32, 0, 0,
							   BUFFER_PAGE(1-i) + DRAW_WIDTH*DRAW_HEIGHT/4*1, 64, SCE_GS_PSMCT32, 0, 0,
							   64, DRAW_WIDTH*DRAW_HEIGHT/64/4, 0 );
		DG_MakeMoveImagePacket( &packet[i].moveimage[2],
							   BUFFER_PAGE(i) + DRAW_WIDTH*DRAW_HEIGHT/4*2, 64, SCE_GS_PSMCT32, 0, 0,
							   BUFFER_PAGE(1-i) + DRAW_WIDTH*DRAW_HEIGHT/4*2, 64, SCE_GS_PSMCT32, 0, 0,
							   64, DRAW_WIDTH*DRAW_HEIGHT/64/4, 0 );
		DG_MakeMoveImagePacket( &packet[i].moveimage[3],
							   BUFFER_PAGE(i) + DRAW_WIDTH*DRAW_HEIGHT/4*3, 64, SCE_GS_PSMCT32, 0, 0,
							   BUFFER_PAGE(1-i) + DRAW_WIDTH*DRAW_HEIGHT/4*3, 64, SCE_GS_PSMCT32, 0, 0,
							   64, DRAW_WIDTH*DRAW_HEIGHT/64/4, 0 );
		InitDmaPacket( &packet[i], i );
	}

	return (0);
}
#else
/* ---------------------------------------------------------------- */
/*
	ＸＢＯＸ用ルーチン
*/

typedef struct {
   DG_DMAPACK_VIEWMAPPING viewmapping;
	DG_DMAPACK_PARAM		use_frametex ;
	DG_DMAPACK_ALPHA		alpha ;
	DG_DMAPACK_SPRT_F			sprt ;
	DG_DMAPACK_PARAM		end ;
} ALL_PACKET ;

/* ---------------------------------------------------------------- */
/* ＤＭＡＰＡＣＫ用パケット初期化 */
static void InitDmaPacket( ALL_PACKET *packet, int which )
{
   DG_SetDmapackViewMapping(&packet->viewmapping, 0.0f, 0.0f, BP_RENDERTARGET_WIDTH, BP_RENDERTARGET_HEIGHT);

	/* 初期化パケットの初期化 */
	DG_SetDmapackUseFrameTex( &packet->use_frametex, 1 );
	DG_SetDmapackAlpha( &packet->alpha, 0 );
	/* 終了パケットの初期化 */
	DG_SetDmapackEnd( &packet->end );
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	/* テクスチャ取り込み要求とプリミティブの表示を同時に行う */
	if ( work->start_flag ){
		work->dmapack->flag &= ~DG_DMAPACK_INVISIBLEMENU ;
		work->start_flag = 0 ;
	}

	/* 非ポーズ中のみカウンタを進ませる */
	if ( GV_PauseLevel == 0 ){
		work->count -= TIME_BASE + DM_FrameSkip * TIME_BASE ;
		work->capture_count += TIME_BASE + DM_FrameSkip * TIME_BASE ;

		if ( work->count < 0 ) work->count = 0 ;
		if ( work->capture_count > work->capture_interval ){
			work->capture_count -= work->capture_interval ;
			work->dmapack->flag |= DG_DMAPACK_INVISIBLEMENU ;
			work->start_flag = 1 ;
		}

	}

	/* 終了チェック */
	if ( work->count == 0 ) GV_DestroyActor( work );

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	/* ＤＭＡパケットオブジェクト開放 */
	if ( work->dmapack != NULL ){
		/* パケットメモリ開放 */
		if ( work->dmapack->autopacket != NULL ) GV_DelayedFree( work->dmapack->autopacket );
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int time )
{
	DG_DMAPACK	*dmapack ;
	ALL_PACKET	*packet ;
	u_int		i ;

	work->time = time ;
	work->count = time ;
	work->start_flag = 1 ;

	/* ＤＭＡパケット型オブジェクト作成 */
	work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU
											 |DG_DMAPACK_INVISIBLEMENU
											 , DG_DMAPACK_PHASE_AFTER, 127 );
	if ( dmapack == NULL ) return ( -1 );
	DG_QueueDmapack( dmapack );

	/* パケットメモリ割り当て */
	packet = GV_Malloc( sizeof(ALL_PACKET) );
	if ( packet == NULL ) return ( -1 );
	GV_ZeroMemory( packet, sizeof(ALL_PACKET) );
	dmapack->autopacket = packet ;
	//dmapack->packet[0] = &packet[0] ;
	//dmapack->packet[1] = &packet[1] ;

	InitDmaPacket( packet, 0 );
	DG_SetDmapackSprt_F( &packet->sprt,
					  0, 0,
					  0.0f, 0.0f,
					  BP_RENDERTARGET_WIDTH, BP_RENDERTARGET_HEIGHT,
                 1.0f, 1.0f,
                        DG_MakeDmaPackColorFromInt( 0x80808080 ), 0 );

	return (0);
}
#endif


/* ---------------------------------------------------------------- */
	/*
		フレームスキップ
	*/
void *NewFrameSkipEffect( int time, int interval )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, time ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->capture_interval = interval ;
      if (interval <= time)
         work->count = 0;
	}
	return (void *)work ;
}

