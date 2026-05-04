//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	crosfade.c
	クロスフェードエフェクト

	2000/02/05 K.Takabe
	$Id: crosfade.c,v 1.5 2002/11/23 12:09:24 Yoshizawa1 Exp $

*/
/*
	void *NewCrossFadeEffect( int time )
	int		time ;	フェード時間（１／３００秒単位）

	キャラを起動したときのフレームを退避し、指定時間かけて
	クロスフェードを行う
	１つ起動するごとにメモリを５１２ＫＢ消費するので注意すること！
	（起動タイミングとしてはカメラが変化したタイミングで起動すると
	切り替え前の最終フレームをクロスフェード用として取り込むことができる）


	void *NewCrossFadeEffectCustom( int time, int capture_interval, int bright_time, int alpha_time, int flag )
	int		time ;				起動時間（１／３００秒単位）
	int		capture_interval ;	フレームバッファ再取り込み間隔（１／３００秒単位）
	int		bright_time ;		輝度のフェード時間（１／３００秒単位）
	int		alpha_time ;		アルファのフェード時間（１／３００秒単位）
	int		flag ;				各種フラグ（現在未使用）
								0x0001:低プライオリティモード（フェードインアウトに負ける）

	特殊な機能をつけたクロスフェードエフェクト
	フレームバッファの再取り込みを行うと自動的に輝度とアルファのフェードが
	初期値に戻る
	輝度ののフェード時間を０にすると常に輝度が１００％になる
	アルファのフェード時間を０にすると常にアルファが１２８になる
	フラグは現在使用されていない


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

/* 通常設定 */
#define NORMAL_PHASE	(DG_DMAPACK_PHASE_AFTER)
#define NORMAL_PRIO		(127)
/* 低プライオリティモード設定（フェードインアウトに負けるように設定） */
#define LOW_PHASE		(DG_DMAPACK_PHASE_NORMAL)
#define LOW_PRIO		(64)

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
	int			alpha_time ;			/* アルファ減衰時間 */
	int			bright_time ;			/* 明るさ減衰時間 */
	int			capture_count ;			/*  */
	int			alpha_count ;			/*  */
	int			bright_count ;			/*  */
	int			alpha ;
	int			bright ;
	int			flag ;			/* 制御フラグ */
#ifndef PSX2
	DG_TEX_LIN	tex ;
#endif
} Work ;

/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
#ifdef PSX2
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
	/* テクスチャ転送 */
	DG_LOADIMAGE		loadimage ;
#ifdef PAL
	DG_LOADIMAGE		loadimage2 ;
#endif

	DG_DMATAG			dmatag ;			/* ＧＩＦ接続ＤＭＡタグ */
	struct _gif_packet {

		/* 初期化パケット */
		struct _init_packet {
			DG_GIFTAG		giftag ;
			struct _init_gif_data{
				DG_GSREG	clamp ;
				DG_GSREG	texflush ;
			} data ;
		} init_packet ;

		/* メイン描画パケット */
		SCREEN_DRAW	screen_draw ;

		/* 変更環境の復元パケット */
		struct _end_packet {
			DG_GIFTAG		giftag ;
			struct _end_gif_data{
				DG_GSREG	test ;
				DG_GSREG	texflush ;
			} data ;
		} end_packet ;

	} gif_packet ;
	DG_DMATAG			dmatag_end ;		/* RETタグ */
} ALL_PACKET ;

/* ---------------------------------------------------------------- */
/* ＤＭＡＰＡＣＫ用パケット初期化 */
static void InitDmaPacket( ALL_PACKET *packet, int which )
{
	packet->dmatag.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(struct _gif_packet) ) ;
	packet->dmatag.vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 ) ;
	packet->dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(struct _gif_packet), 0) ;
	packet->dmatag_end.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 ) ;
	packet->dmatag_end.vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 ) ;
	packet->dmatag_end.vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
	/* 初期化パケットの初期化 */
	packet->gif_packet.init_packet.giftag.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _init_gif_data), 1, 0, 0, 0, 1) ;
	packet->gif_packet.init_packet.giftag.regs = GS_REGS_AD ;
	packet->gif_packet.init_packet.data.clamp.reg = SCE_GS_CLAMP_1 ;
	packet->gif_packet.init_packet.data.clamp.data = SCE_GS_SET_CLAMP(2,2,1,DRAW_WIDTH-1,1,DRAW_HEIGHT-2) ;
	packet->gif_packet.init_packet.data.texflush.reg = SCE_GS_TEXFLUSH ;
	packet->gif_packet.init_packet.data.texflush.data = 0 ;
	/* 終了パケットの初期化 */
	packet->gif_packet.end_packet.giftag.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _end_gif_data), 1, 0, 0, 0, 1) ;
	packet->gif_packet.end_packet.giftag.regs = GS_REGS_AD ;
	packet->gif_packet.end_packet.data.test.reg = SCE_GS_TEST_1 ;
	packet->gif_packet.end_packet.data.test.data = SCE_GS_SET_TEST( 1, 7, 0, 0, 0, 0, 1, 1 ) ;
	packet->gif_packet.end_packet.data.texflush.reg = SCE_GS_TEXFLUSH ;
	packet->gif_packet.end_packet.data.texflush.data = 0 ;
}
/* スクリーン描画パケット設定 */
static void SetScreenDrawPacket( SCREEN_DRAW *screen_draw,
						 int test,
						 u_long64 alpha,
						 int rgba )
{
	static SCREEN_DRAW	def_screen_draw = {
#if 1 //BP_GCC
      { SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _screen_draw_data), 1, 0, 0, 0, 1), GS_REGS_AD }, //giftag
      //data
      {
         { SCE_GS_SET_ALPHA(0,1,2,1,64), SCE_GS_ALPHA_1 }, //alpha
         { SCE_GS_SET_TEX0(0,8,0,9,8,0,0,0,0,0,0,0), SCE_GS_TEX0_1 }, //tex0
         { SCE_GS_SET_TEST( 1, 7, 64, 1, 0, 0, 1, 3 ), SCE_GS_TEST_1 }, //test1
         { SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0), SCE_GS_PRIM }, //prim
         { SCE_GS_SET_RGBAQ(128,128,128,128,0), SCE_GS_RGBAQ }, //rgbq
         //sprt[DRAW_DIV]
         {
            0
         },
         //test2
      }
#else
		.giftag = { .tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _screen_draw_data), 1, 0, 0, 0, 1),
					  .regs = GS_REGS_AD },
		.data = {
			.alpha = { .reg = SCE_GS_ALPHA_1, .data = SCE_GS_SET_ALPHA(0,1,2,1,64) },
			.tex0  = { .reg = SCE_GS_TEX0_1,  .data = SCE_GS_SET_TEX0(0,8,0,9,8,0,0,0,0,0,0,0) },
			.test1 = { .reg = SCE_GS_TEST_1,  .data = SCE_GS_SET_TEST( 1, 7, 64, 1, 0, 0, 1, 3 ) },
			.prim  = { .reg = SCE_GS_PRIM,    .data = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) },
			.rgbq  = { .reg = SCE_GS_RGBAQ,   .data = SCE_GS_SET_RGBAQ(128,128,128,128,0) },
		}
#endif
	} ;
	int		i ;

	*screen_draw = def_screen_draw ;
	/* ピクセルテスト設定 */
	screen_draw->data.test1.data = test ;
	/* アルファ設定 */
	screen_draw->data.alpha.data = alpha ;
	/* テクスチャページ設定(flag = 0x4:rgba_tex) */
	screen_draw->data.tex0.data = SCE_GS_SET_TEX0( TEXTURE_TOP_PAGE() / 64 ,
							  BUFFER_WIDTH/64,SCE_GS_PSMCT16,10,10,0,0,0,0,0,0,0) ;
	/* プリミティブ属性指定（flag = 0x1:alpha disable, 0x2:tex disable） */
	screen_draw->data.prim.data = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) ;
	/* 色設定 */
	screen_draw->data.rgbq.data = rgba ;
	/* 座標＆ＵＶ用レジスタ設定 */
	for ( i = 0 ; i < DRAW_DIV ; i++ ){
		screen_draw->data.sprt[ i ].uv0.reg = SCE_GS_UV ;
		screen_draw->data.sprt[ i ].uv1.reg = SCE_GS_UV ;
		screen_draw->data.sprt[ i ].xyz0.reg = SCE_GS_XYZ2 ;
		screen_draw->data.sprt[ i ].xyz1.reg = SCE_GS_XYZ2 ;
	}
	/* 描画領域＆ＵＶ座標設定 */
	{
		int		x, y, w, h, u, v, uw, vh ;
		x = ( 2048 - DRAW_WIDTH  / 2 ) * 16 ;
		w =   ( DRAW_WIDTH  ) * 16 / DRAW_DIV ;
		y = ( 2048 - DRAW_HEIGHT / 2 ) * 16 ;
		h =   ( DRAW_HEIGHT ) * 16 ;
		u = 8 ;
		uw =  ( DRAW_WIDTH  ) * 16 / DRAW_DIV ;
		v = 8 ;
		vh =  ( DRAW_HEIGHT ) * 16 ;
		//x += BP_PS2_rand()%24 - 12 ;
		//y += BP_PS2_rand()%24 - 12 ;
		for ( i = 0 ; i < DRAW_DIV ; i++ ){
			screen_draw->data.sprt[ i ].xyz0.data = SCE_GS_SET_XYZ( x  , y  , DRAW_Z_MAX ) ;
			screen_draw->data.sprt[ i ].xyz1.data = SCE_GS_SET_XYZ( x+w, y+h, DRAW_Z_MAX ) ;
			screen_draw->data.sprt[ i ].uv0.data = SCE_GS_SET_UV( u   , v    ) ;
			screen_draw->data.sprt[ i ].uv1.data = SCE_GS_SET_UV( u+uw, v+vh ) ;
			x += w ;
			u += uw ;
		}
	}
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	ALL_PACKET	*packet ;
	int			alpha, bright ;
	int			col ;

	/* テクスチャ取り込み要求とプリミティブの表示を同時に行う */
	if ( work->start_flag ){
		work->dmapack->flag &= ~DG_DMAPACK_INVISIBLEMENU ;
		DG_SetFrameStore( work->texture_memory, 1 );
		work->start_flag = 0 ;
	}

	/* 非ポーズ中のみカウンタを進ませる */
	if ( GV_PauseLevel == 0 ){
		work->count -= TIME_BASE + DM_FrameSkip * TIME_BASE ;
		if ( work->count < 0 ) work->count = 0 ;
	}

	switch ( work->mode ){
	  case 0:
		/* パラメータ設定 */
		alpha = 128 * work->count / work->time ;
		col = 0x80808080 ;
		break ;
	  default:
	  case 1:
		/* 非ポーズ中のみカウンタを進ませる */
		if ( GV_PauseLevel == 0 ){
			work->capture_count += TIME_BASE + DM_FrameSkip * TIME_BASE ;
			work->alpha_count -= TIME_BASE + DM_FrameSkip * TIME_BASE ;
			work->bright_count -= TIME_BASE + DM_FrameSkip * TIME_BASE ;
			/* フレームバッファ取り込みチェック */
			if ( work->capture_count > work->capture_interval ){
				work->capture_count -= work->capture_interval ;
				work->alpha_count = work->alpha_time ;
				work->bright_count = work->bright_time ;
				work->dmapack->flag |= DG_DMAPACK_INVISIBLEMENU ;
				work->start_flag = 1 ;
			}
			/* アルファアニメーションチェック */
			if ( work->alpha_count < 0 ){
				work->alpha_count = 0 ;
			}
			if ( work->alpha_time != 0 ){
				work->alpha = 128 * work->alpha_count / work->alpha_time ;
			} else {
				work->alpha = 128 ;
			}
			/* 輝度アニメーションチェック */
			if ( work->bright_count < 0 ){
				work->bright_count = 0 ;
			}
			if ( work->bright_time != 0 ){
				work->bright = 128 * work->bright_count / work->bright_time ;
			} else {
				work->bright = 128 ;
			}
		}
		/* パラメータ設定 */
		alpha = work->alpha ;
		bright = work->bright ;
		col = ( bright ) | ( bright << 8 ) | ( bright << 16 ) | ( bright << 24 );
		break ;
	}

	/* パケットの設定 */
	packet = work->dmapack->packet[ DG_Clock ] ;
	SetScreenDrawPacket( &packet->gif_packet.screen_draw,
						SCE_GS_SET_TEST( 0, 1, 64, 1, 0, 0, 1, 1 ),
						SCE_GS_SET_ALPHA(0,1,2,1,alpha),
						col );

	/* 終了チェック */
	if ( work->count == 0 ) GV_DestroyActor( work );

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	/* テクスチャ用メモリ開放 */
	if ( work->texture_memory != NULL ) GV_DelayedFree( work->texture_memory );
	/* ＤＭＡパケットオブジェクト開放 */
	if ( work->dmapack != NULL ){
		/* パケットメモリ開放 */
		if ( work->dmapack->packet[0] != NULL ) GV_DelayedFree( work->dmapack->packet[0] );
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int time, int flag )
{
	DG_DMAPACK	*dmapack ;
	ALL_PACKET	*packet ;
	u_int		i ;

	work->time = time ;
	work->count = time ;
	work->start_flag = 1 ;

	/* ＤＭＡパケット型オブジェクト作成 */
	if ( !( flag & 0x0001 ) ){
		work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU
												  |DG_DMAPACK_INVISIBLEMENU
												  , NORMAL_PHASE, NORMAL_PRIO );
	} else {
		work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU
												  |DG_DMAPACK_INVISIBLEMENU
												  , LOW_PHASE, LOW_PRIO );
	}
	if ( dmapack == NULL ) return ( -1 );
	DG_QueueDmapack( dmapack );

	/* ここで512KBのメモリを確保 */
	//work->texture_memory = GV_Malloc( DRAW_WIDTH * DRAW_HEIGHT * 2 );
	work->texture_memory = GV_AllocMemory( GV_NORMAL_MEMORY, NULL,
										  ALIGNSIZE128( DRAW_WIDTH * DRAW_HEIGHT * 2 ) , 128 );
	if ( work->texture_memory == NULL ) return ( -1 );

	/* パケットメモリ割り当て */
	packet = GV_Malloc( sizeof(ALL_PACKET) * 2 );
	if ( packet == NULL ) return ( -1 );
	GV_ZeroMemory( packet, sizeof(ALL_PACKET) * 2 );
	dmapack->packet[0] = &packet[0] ;
	dmapack->packet[1] = &packet[1] ;

	for ( i = 0 ; i < 2 ; i++ ){
#ifndef PAL    //BP JG - unused
#if 0
		/* NTSC(448line) version *//* 転送効率が悪い */
		DG_MakeLoadImagePacket( &packet[i].loadimage,
							   SCE_GS_PSMCT16, DRAW_WIDTH, DRAW_HEIGHT,
							   TEXTURE_TOP_PAGE(), DRAW_WIDTH, work->texture_memory );
#else
		/* NTSC(448line) version *//* 転送効率がいい */
		DG_MakeLoadImagePacket( &packet[i].loadimage,
							   SCE_GS_PSMCT32, 64, DRAW_WIDTH*DRAW_HEIGHT/64/2,
							   TEXTURE_TOP_PAGE(), 64, work->texture_memory );
#endif
#else
#if 0
		/* PAL(512line) version *//* 転送効率が悪い */
		DG_MakeLoadImagePacket( &packet[i].loadimage,
							   SCE_GS_PSMCT16, DRAW_WIDTH, DRAW_HEIGHT/2,
							   TEXTURE_TOP_PAGE(), DRAW_WIDTH, work->texture_memory );
		DG_MakeLoadImagePacket( &packet[i].loadimage2,
							   SCE_GS_PSMCT16, DRAW_WIDTH, (DRAW_HEIGHT/2)|((DRAW_HEIGHT/2)<<16),
							   TEXTURE_TOP_PAGE(), DRAW_WIDTH,
							   (void*)( (int)work->texture_memory + DRAW_WIDTH*DRAW_HEIGHT*2/2 ) );
#else
		/* PAL(512line) version *//* 転送効率がいい */
		DG_MakeLoadImagePacket( &packet[i].loadimage,
							   SCE_GS_PSMCT32, 64, DRAW_WIDTH*DRAW_HEIGHT/64/2/2,
							   TEXTURE_TOP_PAGE(), 64, work->texture_memory );
		DG_MakeLoadImagePacket( &packet[i].loadimage2,
							   SCE_GS_PSMCT32, 64, (DRAW_WIDTH*DRAW_HEIGHT/64/2/2)|((DRAW_WIDTH*DRAW_HEIGHT/64/2/2)<<16),
							   TEXTURE_TOP_PAGE(), 64,
							   (void*)( (int)work->texture_memory + DRAW_WIDTH*DRAW_HEIGHT*2/2 ) );
#endif
#endif

		InitDmaPacket( &packet[i], i );
		/*  */
		SetScreenDrawPacket( &packet[i].gif_packet.screen_draw,
							SCE_GS_SET_TEST( 0, 1, 64, 1, 0, 0, 1, 1 ),
							SCE_GS_SET_ALPHA(0,1,2,1,64),
							0x80808080 );
	}

	return (0);
}
#else
/* ---------------------------------------------------------------- */
/*
	ＸＢＯＸ用ルーチン
*/
typedef struct _screen_draw {
	DG_DMAPACK_PARAM	use_texlin ;
	DG_DMAPACK_ALPHA	alpha ;
	DG_DMAPACK_SPRT		sprt ;
} SCREEN_DRAW ;

typedef struct {
	/* テクスチャ転送 */

	/* 初期化パケット */

	/* メイン描画パケット */
	SCREEN_DRAW	screen_draw ;

	/* 変更環境の復元パケット */
	DG_DMAPACK_PARAM	end ;
} ALL_PACKET ;

/* ---------------------------------------------------------------- */
/* ＤＭＡＰＡＣＫ用パケット初期化 */
static void InitDmaPacket( ALL_PACKET *packet, int which )
{
	DG_SetDmapackEnd( &packet->end );
}
/* スクリーン描画パケット設定 */
static void *SetScreenDrawPacket( SCREEN_DRAW *screen_draw,
						 u_long64 alpha,
						 int rgba, DG_TEX_LIN *tex )
{

	/* ピクセルテスト設定 */
	/* アルファ設定 */
	DG_SetDmapackAlpha( &screen_draw->alpha, alpha );
	/* テクスチャページ設定(flag = 0x4:rgba_tex) */
	DG_SetDmapackTexLin( &screen_draw->use_texlin, tex );
	/* 描画領域＆ＵＶ座標設定 */
	{/* ＸＢＯＸでは必ずフレームバッファを退避して描画するので重なりは気にする必要がない */
		float		x, y, w, h, u, v, uw, vh, uof, vof ;
		uof = ( DISPLAY_WIDTH - DRAW_WIDTH ) / 2 ;
		vof = ( DISPLAY_HEIGHT - DRAW_HEIGHT ) / 2 ;
		x = 0 ; w = DRAW_WIDTH ;
		y = 0 ; h = DRAW_HEIGHT ;
		u = (float)( uof + 0.5f ) / tex->width ;
		uw = (float)( DRAW_WIDTH ) / tex->width ;
		v = (float)( vof + 0.5f ) / tex->height ;
		vh = (float)( DRAW_HEIGHT ) / tex->height ;
		DG_SetDmapackSprt( &screen_draw->sprt,
						  x, y, u, v,
						  x+w, y+h, u+uw, v+vh,
						  rgba );
	}

	return ( &screen_draw[1] );
}


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	ALL_PACKET	*packet ;
	int			alpha, bright ;
	int			col ;

	/* テクスチャ取り込み要求とプリミティブの表示を同時に行う */
	if ( work->start_flag ){
		work->dmapack->flag &= ~DG_DMAPACK_INVISIBLEMENU ;
		DG_SetFrameStore( work->texture_memory, 1 );
		work->start_flag = 0 ;
	}

	/* 非ポーズ中のみカウンタを進ませる */
	if ( GV_PauseLevel == 0 ){
		work->count -= TIME_BASE + DM_FrameSkip * TIME_BASE ;
		if ( work->count < 0 ) work->count = 0 ;
	}

	switch ( work->mode ){
	  case 0:
		/* パラメータ設定 */
		alpha = 128 * work->count / work->time ;
		col = 0x80808080 ;
		break ;
	  default:
	  case 1:
		/* 非ポーズ中のみカウンタを進ませる */
		if ( GV_PauseLevel == 0 ){
			work->capture_count += TIME_BASE + DM_FrameSkip * TIME_BASE ;
			work->alpha_count -= TIME_BASE + DM_FrameSkip * TIME_BASE ;
			work->bright_count -= TIME_BASE + DM_FrameSkip * TIME_BASE ;
			/* フレームバッファ取り込みチェック */
			if ( work->capture_count > work->capture_interval ){
				work->capture_count -= work->capture_interval ;
				work->alpha_count = work->alpha_time ;
				work->bright_count = work->bright_time ;
				work->dmapack->flag |= DG_DMAPACK_INVISIBLEMENU ;
				work->start_flag = 1 ;
			}
			/* アルファアニメーションチェック */
			if ( work->alpha_count < 0 ){
				work->alpha_count = 0 ;
			}
			if ( work->alpha_time != 0 ){
				work->alpha = 128 * work->alpha_count / work->alpha_time ;
			} else {
				work->alpha = 128 ;
			}
			/* 輝度アニメーションチェック */
			if ( work->bright_count < 0 ){
				work->bright_count = 0 ;
			}
			if ( work->bright_time != 0 ){
				work->bright = 128 * work->bright_count / work->bright_time ;
			} else {
				work->bright = 128 ;
			}
		}
		/* パラメータ設定 */
		alpha = work->alpha ;
		bright = work->bright ;
		col = ( bright ) | ( bright << 8 ) | ( bright << 16 ) | ( bright << 24 );
		break ;
	}

	/* パケットの設定 */
	packet = work->dmapack->autopacket ;
	SetScreenDrawPacket( &packet->screen_draw,
						SCE_GS_SET_ALPHA(0,1,2,1,alpha),
						col, &work->tex );

	/* 終了チェック */
	if ( work->count == 0 ) GV_DestroyActor( work );

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	/* テクスチャ用メモリ開放 */
#ifdef KP_WINDOWS
	DG_FreeLinerTexture2( &work->tex ) ;
#endif
	if ( work->texture_memory != NULL ) GV_DelayedFree( work->texture_memory );
	/* ＤＭＡパケットオブジェクト開放 */
	if ( work->dmapack != NULL ){
		/* パケットメモリ開放 */
		if ( work->dmapack->autopacket != NULL ) GV_DelayedFree( work->dmapack->autopacket );
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int time, int flag )
{
	DG_DMAPACK	*dmapack ;
	ALL_PACKET	*packet ;
	u_int		i ;

	work->time = time ;
	work->count = time ;
	work->start_flag = 1 ;

	/* ＤＭＡパケット型オブジェクト作成 */
	if ( !( flag & 0x0001 ) ){
		work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU
												  |DG_DMAPACK_INVISIBLEMENU
												  , NORMAL_PHASE, NORMAL_PRIO );
	} else {
		work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU
												  |DG_DMAPACK_INVISIBLEMENU
												  , LOW_PHASE, LOW_PRIO );
	}
	if ( dmapack == NULL ) return ( -1 );
	DG_QueueDmapack( dmapack );

	/* ここで512KBのメモリを確保 */
	//work->texture_memory = GV_Malloc( DRAW_WIDTH * DRAW_HEIGHT * 2 );
	/* システムの関係で確保するメモリ量は描画に使われているサイズではなく、フレームバッファの実サイズになるので注意！ */
	work->texture_memory = GV_AllocMemory( GV_NORMAL_MEMORY, NULL,
										  ALIGNSIZE128( DISPLAY_WIDTH * DISPLAY_HEIGHT * 4 ) , 128 );
	if ( work->texture_memory == NULL ) return ( -1 );
	/* 本当はDG_MakeLinerTexture()を使いたかったが、メモリ領域が足りなかったため例外的にこうしているので注意！ */
	DG_MakeLinerTexture2( &work->tex, DISPLAY_WIDTH, DISPLAY_HEIGHT, DG_TEXLIN_FORMAT_A8R8G8B8, work->texture_memory );
	/* 一時的にレンダリングカラー深度を１６ビットに設定できればこっちが使えるのだが・・・ */
	//DG_MakeLinerTexture2( &work->tex, DISPLAY_WIDTH, DISPLAY_HEIGHT, DG_TEXLIN_FORMAT_R5G6B5, work->texture_memory );

	/* パケットメモリ割り当て */
	packet = GV_Malloc( sizeof(ALL_PACKET) );
	if ( packet == NULL ) return ( -1 );
	GV_ZeroMemory( packet, sizeof(ALL_PACKET) );
	dmapack->autopacket = packet ;
	//dmapack->packet[0] = &packet[0] ;
	//dmapack->packet[1] = &packet[1] ;


	InitDmaPacket( packet, 0 );
	/*  */
	SetScreenDrawPacket( &packet->screen_draw,
						SCE_GS_SET_ALPHA(0,1,2,1,64),
						0x80808080, &work->tex );

	return (0);
}
#endif

/* ---------------------------------------------------------------- */
	/*
		通常クロスフェード
	*/
void *NewCrossFadeEffect( int time )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, time, 0 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

	/*
		特殊クロスフェード
	*/
void *NewCrossFadeEffectCustom( int time, int capture_interval, int bright_time, int alpha_time, int flag )
{
	Work		*work ;
	OPERATOR() ;

	if ( bright_time == 0 && alpha_time == 0 && flag == 0 ){
		extern void *NewFrameSkipEffect( int time, int interval );
		return NewFrameSkipEffect( time, capture_interval );
	}

	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, time, flag ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		{
			work->alpha = 128 ;
			work->bright = 128 ;
			work->mode = 1 ;
			work->alpha_time = alpha_time ;
			work->bright_time = bright_time ;
			work->capture_interval = capture_interval ;
			work->capture_count = 0 ;
			work->alpha_count = alpha_time ;
			work->bright_count = bright_time ;
			work->flag = flag ;
		}
	}
	return (void *)work ;
}

#if 0
	{/* クロスフェードエフェクト実験用 */
		static FVECTOR	old_pos ;
		static int	count = 0 ;
		float	len ;
		FVECTOR	vec ;

		_sceVu0SubVector( &vec, (FVECTOR*)&DG_Chanl(0)->eye.m[3], &old_pos );
		old_pos = *(FVECTOR*)(DG_Chanl(0)->eye.m[3]) ;
		len = bp_sqrtf( vec.vx * vec.vx + vec.vy * vec.vy + vec.vz * vec.vz );  //BP_MATH - emulate PS2 sqrtf
		if ( count == 0 ){
			if ( len > 1000 ){
				NewCrossFadeEffect( 150 );
				count = 30 ;
			}
		} else {
			count-- ;
		}
	}
	if ( GV_PadData[1].press & PAD_A ) NewCrossFadeEffect( 300 );
#endif
