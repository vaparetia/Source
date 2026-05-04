//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ir_mode.c
	赤外線モードエフェクト

	2000/01/23 K.Takabe
	$Id: ir_mode.c,v 1.11 2002/12/16 00:58:10 takaki Exp $

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
#ifdef KP_XBOX
#include <xtl.h>
// BP #include <xgraphics.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"def_dma.h"
#include	"gameheader.h"

#include "BP_RenderFX.h"
#include "BP_RendererDebug.h"
#include "BP_RenderBufferTypes.h"

void *NewMonochromaticEffect( int phase, int prio, u_int *pal );

#ifdef KP_WINDOWS
extern DWORD	DG_BackBufferRendHeight ;
#endif
#define NEW_VERSION_FOR_PS2

#define ALIGNSIZE1(_n) (_n)
#define ALIGNSIZE16(_n) (((_n)+15)&0xfffffff0)
#define ALIGNSIZE64(_n) (((_n)+63)&0xffffffc0)
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)

#define DRAW_DIV	(16)		/* 描画の分割数 */
#define RASTER_NUM	(DRAW_HEIGHT/4)	/* 112 */

#define DMAPACK_PRIO	(140)	/* ２Ｄシステムより優先を低くする */


/* ---------------------------------------------------------------- */
extern void *NewGrayClut( int time, u_int add_col );

/* ---------------------------------------------------------------- */
typedef	struct _irmode_Work {
	GV_ACT_EX	actor ;
	void		*actor_grayclut ;
	DG_DMAPACK	*dmapack ;
	int			mode ;
	int			count ;
	int			color ;
	void		*alloc_addr ;
#ifndef PSX2
	DG_TEX_LIN	*color_tex ;
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

typedef struct _raster_draw {
#if 0 // BP_PS2 (Swapped for XBOX version)
	DG_GIFTAG		giftag ;
	struct _raster_draw_data{
		DG_GSREG	alpha ;
		DG_GSREG	prim ;
		DG_GSREG	rgbq ;
		struct _raster_parts{
			DG_GSREG	xyz0 ;
			DG_GSREG	xyz1 ;
		} sprt[ RASTER_NUM ];
	} data ;
#else
   DG_DMAPACK_ALPHA alpha ;
   DG_DMAPACK_BOX box[ RASTER_NUM ];
#endif
} RASTER_DRAW ;

typedef struct _frame_mask {				/* 本当はあまりこれを多用してはいけない */
	DG_GIFTAG	giftag ;
	DG_GSREG	frame ;
} FRAME_MASK ;

#if 0 // BP_PS2
typedef struct {
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

		/* 走査線パケット */
		FRAME_MASK	frame3 ;
		RASTER_DRAW	raster_draw ;

		/* 変更環境の復元パケット */
		struct _end_packet {
			DG_GIFTAG		giftag ;
			struct _end_gif_data{
				DG_GSREG	test ;
				DG_GSREG	texflush ;
			} data ;
		} end_packet ;

	} gif_packet ;
	DG_DMATAG			dmatag_drawenv ;	/* 描画環境復元用 */
	DG_DMATAG			dmatag_offset ;		/* オフセット環境復元用 */
	DG_DMATAG			dmatag_end ;		/* RETタグ */
} ALL_PACKET ;
#else
typedef struct _ir_mode_ALL_PACKET {
   RASTER_DRAW	raster_draw ;
   DG_DMAPACK_PARAM end ;
} ALL_PACKET;
#endif

/* ---------------------------------------------------------------- */
static void SetFrameMaskPacket( FRAME_MASK *frame, int which, int mask )
{
	frame->giftag.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(DG_GSREG), 1, 0, 0, 0, 1);
	frame->giftag.regs = GS_REGS_AD ;
	frame->frame.reg = SCE_GS_FRAME_1 ;
	frame->frame.data =
	  SCE_GS_SET_FRAME( BUFFER_PAGE(which)/2048, BUFFER_WIDTH/64, FRAME_BUFFER_COLOR_MODE(), mask ) ;
}

/* ---------------------------------------------------------------- */
#if 0 // BP_PS2
static void InitDmaPacket( ALL_PACKET *packet, int which )
{
	packet->dmatag.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(struct _gif_packet) ) ;
	packet->dmatag.vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 ) ;
	packet->dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(struct _gif_packet), 0) ;
	packet->dmatag_drawenv.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_DRAWENV) ) ;
	packet->dmatag_drawenv.addr = &( DG_Chanl( 0 )->draw_env[ which ] ) ;
	packet->dmatag_drawenv.vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 ) ;
	packet->dmatag_drawenv.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_DRAWENV), 0) ;
	packet->dmatag_offset.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_DRAWOFFSET) ) ;
	packet->dmatag_offset.addr = &( DG_Chanl( 0 )->draw_offset[ which ] ) ;
	packet->dmatag_offset.vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 ) ;
	packet->dmatag_offset.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_DRAWOFFSET), 0) ;
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
#endif

static void SetScreenDrawPacket( SCREEN_DRAW *screen_draw, int flag,
						 int test,
						 u_long64 alpha,
						 int tex_page,
						 int rgba )
{
   BP_TODO_BREAK;
#if 0 //BP_GCC
	static SCREEN_DRAW	def_screen_draw = {
		.giftag = { .tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _screen_draw_data), 1, 0, 0, 0, 1),
					  .regs = GS_REGS_AD },
		.data = {
			.alpha = { .reg = SCE_GS_ALPHA_1, .data = SCE_GS_SET_ALPHA(0,1,2,1,64) },
			.tex0  = { .reg = SCE_GS_TEX0_1,  .data = SCE_GS_SET_TEX0(0,8,0,9,8,0,0,0,0,0,0,0) },
			.test1 = { .reg = SCE_GS_TEST_1,  .data = SCE_GS_SET_TEST( 1, 7, 64, 1, 0, 0, 1, 3 ) },
			.prim  = { .reg = SCE_GS_PRIM,    .data = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) },
			.rgbq  = { .reg = SCE_GS_RGBAQ,   .data = SCE_GS_SET_RGBAQ(128,128,128,128,0) },
		}
	} ;
	int		i ;

	*screen_draw = def_screen_draw ;
	/* ピクセルテスト設定 */
	screen_draw->data.test1.data = test ;
	/* アルファ設定 */
	screen_draw->data.alpha.data = alpha ;
	/* テクスチャページ設定(flag = 0x4:rgba_tex) */
	screen_draw->data.tex0.data = SCE_GS_SET_TEX0( BUFFER_PAGE(tex_page) / 64 ,
							  BUFFER_WIDTH/64,FRAME_BUFFER_COLOR_MODE(),10,10,((flag&4)!=0),0,0,0,0,0,0) ;
	/* プリミティブ属性指定（flag = 0x1:alpha disable, 0x2:tex disable） */
	screen_draw->data.prim.data = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,((flag&2)==0),0,((flag&1)==0),0,1,0,0) ;
	/* 色設定 */
	screen_draw->data.rgbq.data = rgba ;
	/* 座標＆ＵＶ用レジスタ設定 */
	if ( !( flag & 0x80000000 ) ){
		for ( i = 0 ; i < DRAW_DIV ; i++ ){
			screen_draw->data.sprt[ i ].uv0.reg = SCE_GS_UV ;
			screen_draw->data.sprt[ i ].uv1.reg = SCE_GS_UV ;
			screen_draw->data.sprt[ i ].xyz0.reg = SCE_GS_XYZ2 ;
			screen_draw->data.sprt[ i ].xyz1.reg = SCE_GS_XYZ2 ;
		}
	} else {
		for ( i = 0 ; i < DRAW_DIV ; i++ ){
			screen_draw->data.sprt[ i ].uv0.reg = SCE_GS_NOP ;
			screen_draw->data.sprt[ i ].uv1.reg = SCE_GS_NOP ;
			screen_draw->data.sprt[ i ].xyz0.reg = SCE_GS_NOP ;
			screen_draw->data.sprt[ i ].xyz1.reg = SCE_GS_NOP ;
		}
	}
	/* 描画領域＆ＵＶ座標設定 */
	{/* 描画領域が重ならないように描画方向も考慮してプリミティブ座標とＵＶ座標を設定する */
		int		x, y, w, h, u, v, uw, vh ;
		x = ( 2048 - DRAW_WIDTH  / 2 ) * 16 ;
		w =   ( DRAW_WIDTH  ) * 16 / DRAW_DIV ;
		y = ( 2048 - DRAW_HEIGHT / 2 ) * 16 ;
		h =   ( DRAW_HEIGHT ) * 16 ;
		u = 8 ;
		uw =  ( DRAW_WIDTH  ) * 16 / DRAW_DIV ;
		v = 8 ;
		vh =  ( DRAW_HEIGHT ) * 16 ;
		if ( flag & 0x80 ){
			x -= 16 * 3 ;
			y -= 16 * 3 ;
		}
		for ( i = 0 ; i < DRAW_DIV ; i++ ){
			screen_draw->data.sprt[ i ].xyz0.data = SCE_GS_SET_XYZ( x  , y  , DRAW_Z_MAX ) ;
			screen_draw->data.sprt[ i ].xyz1.data = SCE_GS_SET_XYZ( x+w, y+h, DRAW_Z_MAX ) ;
			screen_draw->data.sprt[ i ].uv0.data = SCE_GS_SET_UV( u   , v    ) ;
			screen_draw->data.sprt[ i ].uv1.data = SCE_GS_SET_UV( u+uw, v+vh ) ;
			x += w ;
			u += uw ;
		}
	}
#endif
}

static void SetRasterDrawPacket( RASTER_DRAW *raster_draw, int flag,
								u_long64 alpha,
								int rgba, int y_offset )
{
#if 0 // BP_PS2 (swapped for XBOX version)
	static RASTER_DRAW	def_raster_draw = {
#if 1 //BP_GCC

      { SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _raster_draw_data), 1, 0, 0, 0, 1), GS_REGS_AD }, //giftag
      //data
      {
         { SCE_GS_SET_ALPHA(0,1,2,1,64), SCE_GS_ALPHA_1 }, //alpha
         { SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0), SCE_GS_PRIM }, //prim
         { SCE_GS_SET_RGBAQ(128,128,128,128,0), SCE_GS_RGBAQ } //rgbq
         //sprt[ RASTER_NUM ]
      }

#else
		.giftag = { .tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _raster_draw_data), 1, 0, 0, 0, 1),
					  .regs = GS_REGS_AD },
		.data = {
			.alpha = { .reg = SCE_GS_ALPHA_1, .data = SCE_GS_SET_ALPHA(0,1,2,1,64) },
			.prim  = { .reg = SCE_GS_PRIM,    .data = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) },
			.rgbq  = { .reg = SCE_GS_RGBAQ,   .data = SCE_GS_SET_RGBAQ(128,128,128,128,0) },
		}
#endif
	} ;
	int		i ;

	*raster_draw = def_raster_draw ;
	/* アルファ設定 */
	raster_draw->data.alpha.data = alpha ;
	/* プリミティブ属性指定（flag = 0x1:alpha disable, 0x2:tex disable） */
	raster_draw->data.prim.data = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,((flag&2)==0),0,((flag&1)==0),0,1,0,0) ;
	/* 色設定 */
	raster_draw->data.rgbq.data = rgba ;
	/* 座標＆ＵＶ用レジスタ設定 */
	for ( i = 0 ; i < RASTER_NUM ; i++ ){
		raster_draw->data.sprt[ i ].xyz0.reg = SCE_GS_XYZ2 ;
		raster_draw->data.sprt[ i ].xyz1.reg = SCE_GS_XYZ2 ;
	}
	/* 描画領域＆ＵＶ座標設定 */
	{/* 描画領域が重ならないように描画方向も考慮してプリミティブ座標とＵＶ座標を設定する */
		int		x, y, w, h ;
		x = ( 2048 - DRAW_WIDTH  / 2 ) * 16 ;
		w =   ( DRAW_WIDTH  ) * 16 ;
		y = ( 2048 - DRAW_HEIGHT / 2 ) * 16 + y_offset ;
		h =   ( 3 ) * 16 ;
		for ( i = 0 ; i < RASTER_NUM ; i++ ){
			raster_draw->data.sprt[ i ].xyz0.data = SCE_GS_SET_XYZ( x  , y  , DRAW_Z_MAX ) ;
			raster_draw->data.sprt[ i ].xyz1.data = SCE_GS_SET_XYZ( x+w, y+h, DRAW_Z_MAX ) ;
			y += ( 4 ) * 16 ;
		}
	}
#else
   int		i ;

   /* アルファ設定 */
   DG_SetDmapackAlpha( &raster_draw->alpha, alpha );

   {
      int		x, y, w, h ;
      x = 0 ;
      w = DRAW_WIDTH ;
      y = y_offset ;
      h = 1 ;
      for ( i = 0 ; i < RASTER_NUM ; i++ ){
         DG_SetDmapackBox( &raster_draw->box[ i ], x, y, x+w, y+h, DG_MakeDmaPackColorFromInt(rgba) );
         y += 4 ;
      }
   }
#endif
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	ALL_PACKET	*packet ;

	if ( work->mode == 0 && GV_PauseLevel == 0 ){
		int		r, g, b, a, col ;
		/* ＰＳ２ */
#ifndef NEW_VERSION_FOR_PS2
		packet = work->dmapack->packet[DG_Clock] ;
		col = packet->gif_packet.screen_draw0.data.rgbq.data ;
		col = 0x802020c0 ;
		col = work->color ;
		r = col & 0xff ;
		g = ( col >> 8 ) & 0xff ;
		b = ( col >> 16 ) & 0xff ;
		a = ( col >> 24 ) & 0xff ;
#if 0
		r += work->count ;			/* 揺らぎ分を追加 */
		if ( BP_PS2_rand() >> (15+16-1) ){
			work->count++ ;
			if ( work->count > 16 ) work->count = 16 ;
		} else {
			work->count-- ;
			if ( work->count < -16 ) work->count = -16 ;
		}
#endif

		col = ( a << 24 ) | ( b << 16 ) | ( g << 8 ) | r ;
		packet->gif_packet.screen_draw0.data.rgbq.data = col ;
#endif
	}
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	/* パケットメモリ開放 */
	if ( work->alloc_addr != NULL ){
		GV_DelayedFree( work->alloc_addr );
	}
	/* ＤＭＡパケットオブジェクト開放 */
	if ( work->dmapack != NULL ){
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
	/* 赤外線モード終了 */
	if ( work->mode == 0 ){
		DG_DisplayStatus &= ~DG_STATE_IR_MODE ;
	}
}

#if 0 //BP_PS2
/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int mode )
{
	DG_DMAPACK	*dmapack ;
	ALL_PACKET	*packet ;
	u_int		col = 0, mask, col2, i ;

	work->mode= mode ;

	/* ＤＭＡパケット型オブジェクト作成 */
	work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_NORMAL|
//											 DG_DMAPACK_INVISIBLE0|
											 DG_DMAPACK_INVISIBLE1|
											 DG_DMAPACK_INVISIBLE2|
											 DG_DMAPACK_INVISIBLE3
											 , DG_DMAPACK_PHASE_AFTER, DMAPACK_PRIO );
	if ( dmapack == NULL ) return ( -1 );
	DG_QueueDmapack( dmapack );

#ifdef NEW_VERSION_FOR_PS2
	//dmapack->flag |= DG_DMAPACK_INVISIBLE0 ;
	if ( mode == 0 ){
		/* 赤外線 */
		static u_int	color_table[8] = {
			0x80ff8000,0x80ffff00,0x8080ff00,0x8000ff00,
			0x8000ff80,0x8000ffff,0x800080ff,0x800000ff
		};
		u_int		pal[256], *data ;
		data = pal ;
		for ( i = 0 ; i < 256 ; i++, data++ ){
			*data = color_table[ i / 32 ] ;
		}

		work->actor_grayclut = NewMonochromaticEffect( DG_DMAPACK_PHASE_AFTER, DMAPACK_PRIO-1, pal );
		if ( work->actor_grayclut == NULL ) return ( -1 );
		GV_SetActorChild( &work->actor, work->actor_grayclut );
		DG_DisplayStatus |= DG_STATE_IR_MODE ;
	} else {
		/* 暗視ゴーグル */
		u_int		pal[256], *data ;
		data = pal ;
#if BP_VITA
		for ( i = 0 ; i < 64 ; i++, data++ ){
			*data = 0x80000000 | ( ( 240 * i / 63 ) << 8 ) ;
		}
		for ( i = 0 ; i < 64 ; i++, data++ ){
			int		tmp = ( 240 * i / 63 ) ;
			*data = 0x8000f000 | ( tmp << 16 ) | ( tmp ) ;
		}
		for ( i = 0 ; i < 128 ; i++, data++ ){
			*data = 0x80f0f0f0 ;
		}
#else
		for ( i = 0 ; i < 32 ; i++, data++ ){
			*data = 0x80000000 | ( ( 240 * i / 31 ) << 8 ) ;
		}
		for ( i = 0 ; i < 32 ; i++, data++ ){
			int		tmp = ( 240 * i / 31 ) ;
			*data = 0x8000f000 | ( tmp << 16 ) | ( tmp ) ;
		}
		for ( i = 0 ; i < 64 ; i++, data++ ){
			*data = 0x80f0f0f0 ;
		}
		for ( i = 0 ; i < 128 ; i++, data++ ){
			*data = 0x80f0f0f0 ;
		}
#endif
		work->actor_grayclut = NewMonochromaticEffect( DG_DMAPACK_PHASE_AFTER, DMAPACK_PRIO-1, pal );
		if ( work->actor_grayclut == NULL ) return ( -1 );
		GV_SetActorChild( &work->actor, work->actor_grayclut );
	}

	work->color = col ;

	/* パケットメモリ割り当て */
	work->alloc_addr = packet = GV_Malloc( sizeof(ALL_PACKET) * 2 );
	if ( packet == NULL ) return ( -1 );
	GV_ZeroMemory( packet, sizeof(ALL_PACKET) * 2 );
	dmapack->packet[0] = &packet[0] ;
	dmapack->packet[1] = &packet[1] ;

	for ( i = 0 ; i < 2 ; i++ ){
		InitDmaPacket( &packet[i], i );

		/* ラスタの加算半透明描画 */
		SetFrameMaskPacket( &packet[i].gif_packet.frame3, i, 0x00000000 );
		SetRasterDrawPacket( &packet[i].gif_packet.raster_draw, 0x02, SCE_GS_SET_ALPHA(0,1,2,1,64), 0x80000000, 0 );

	}

#else
	if ( mode == 0 ){
		/* 赤外線 */
#if 0
		col = 0x802020c0 ;	/* モジュレートカラー */
		mask = 0x00e0e0e0 ;	/* ビットマスク指定 */
		col2 = 0x00303030 ;	/* ラスタ加算カラー */
#else
		col = 0x800018ac ;	/* モジュレートカラー */
		mask = 0x00e0e0e8 ;	/* ビットマスク指定 */
		col2 = 0x00263e26 ;	/* ラスタ加算カラー */
		col2 = 0x004c7c4c ;	/* ラスタ加算カラー */
		//col2 = 0x0072ba72 ;	/* ラスタ加算カラー */
		work->actor_grayclut = NewGrayClut( -1, 0x000030 );	/* パレット変更処理 */
		if ( work->actor_grayclut == NULL ) return ( -1 );
		GV_SetActorChild( &work->actor, work->actor_grayclut );
#endif
		DG_DisplayStatus |= DG_STATE_IR_MODE ;
	} else {
		/* 暗視ゴーグル */
#if 0
		col = 0x8020f020 ;	/* モジュレートカラー */
		mask = 0x00e0ffe0 ;	/* ビットマスク指定 */
		col2 = 0x00303030 ;	/* ラスタ加算カラー */
		work->actor_grayclut = NewGrayClut( -1, 0x404040 );	/* パレット変更処理 */
#else
		col = 0x804bc017 ;	/* モジュレートカラー */
		mask = 0x00e0ffe0 ;	/* ビットマスク指定 */
		col2 = 0x00083010 ;	/* ラスタ加算カラー */
		work->actor_grayclut = NewGrayClut( -1, 0x303030 );	/* パレット変更処理 */
#endif
		if ( work->actor_grayclut == NULL ) return ( -1 );
		GV_SetActorChild( &work->actor, work->actor_grayclut );
	}

	work->color = col ;

	/* パケットメモリ割り当て */
	work->alloc_addr = packet = GV_Malloc( sizeof(ALL_PACKET) * 2 );
	if ( packet == NULL ) return ( -1 );
	GV_ZeroMemory( packet, sizeof(ALL_PACKET) * 2 );
	dmapack->packet[0] = &packet[0] ;
	dmapack->packet[1] = &packet[1] ;

	for ( i = 0 ; i < 2 ; i++ ){
		InitDmaPacket( &packet[i], i );
		/* フレームバッファモジュレート */
		SetFrameMaskPacket( &packet[i].gif_packet.frame0, i, 0x00000000 );
		SetScreenDrawPacket( &packet[i].gif_packet.screen_draw0,
							0x01,
							SCE_GS_SET_TEST( 0, 1, 64, 1, 0, 0, 1, 1 ),
							SCE_GS_SET_ALPHA(0,1,2,1,128),
							i,
							col );
		/* 階調落とし描画 */
		SetFrameMaskPacket( &packet[i].gif_packet.frame1, i, mask );
		SetScreenDrawPacket( &packet[i].gif_packet.screen_draw1,
							0x03,
							SCE_GS_SET_TEST( 0, 1, 64, 1, 0, 0, 1, 1 ),
							SCE_GS_SET_ALPHA(0,1,2,1,128),
							i,
							0x00000000 );

		/* エンボス処理（赤外線時のみ有効） */
		SetFrameMaskPacket( &packet[i].gif_packet.frame2, i, 0x007f7f7f );
		SetFrameMaskPacket( &packet[i].gif_packet.frame2, i, 0x00000000 );
		SetScreenDrawPacket( &packet[i].gif_packet.screen_draw2,
							0x80 | (mode?0x80000000:0),
							SCE_GS_SET_TEST( 0, 1, 64, 1, 0, 0, 1, 1 ),
							SCE_GS_SET_ALPHA(0,1,2,0,128),
							i,
							0x00808080 );

		/* ラスタの加算半透明描画 */
		SetFrameMaskPacket( &packet[i].gif_packet.frame3, i, 0x00000000 );
		if ( mode == 0 ){
			SetRasterDrawPacket( &packet[i].gif_packet.raster_draw, 0x02, SCE_GS_SET_ALPHA(0,1,2,1,40), col2, 0 );
		} else {
			SetRasterDrawPacket( &packet[i].gif_packet.raster_draw, 0x02, SCE_GS_SET_ALPHA(0,2,2,1,128), col2, 0 );
		}

	}
#endif

	return (0);
}
#else

extern int g_IR_Blinds_DarkenHeight;
extern int g_IR_Blinds_BlankHeight;
static void BP_IRModeCallback(void* pWork)
{
   int i;

   SBP_PFX_Blinds* pPacket = (SBP_PFX_Blinds*)BP_RB_Alloc(sizeof(SBP_PFX_Blinds));

   pPacket->alphaData = SCE_GS_SET_ALPHA(0,1,2,1,64);
   pPacket->color = 0x80000000;
   pPacket->startY = 0;
   pPacket->darkenHeight = g_IR_Blinds_DarkenHeight;
   pPacket->blankHeight = g_IR_Blinds_BlankHeight;

   BP_RB_AddCommand(kCmd_PostFx_Blinds, (char*)pPacket);
}

static int GetResources( Work *work, int mode )
{
   DG_DMAPACK	*dmapack ;
   ALL_PACKET	*packet ;
   u_int		col = 0, mask, col2, i ;

   work->mode= mode ;

   /* ＤＭＡパケット型オブジェクト作成 */
   work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_NORMAL|
      //											 DG_DMAPACK_INVISIBLE0|
      DG_DMAPACK_INVISIBLE1|
      DG_DMAPACK_INVISIBLE2|
      DG_DMAPACK_INVISIBLE3
      , DG_DMAPACK_PHASE_AFTER, DMAPACK_PRIO );
   if ( dmapack == NULL ) return ( -1 );
   DG_QueueDmapack( dmapack );

   //dmapack->flag |= DG_DMAPACK_INVISIBLE0 ;
   if ( mode == 0 ){
      /* 赤外線 */
      static u_int	color_table[8] = {
         0x80ff8000,0x80ffff00,0x8080ff00,0x8000ff00,
         0x8000ff80,0x8000ffff,0x800080ff,0x800000ff
      };
      u_int		pal[256], *data ;
      data = pal ;
      for ( i = 0 ; i < 256 ; i++, data++ ){
         *data = color_table[ i / 32 ] ;
      }

      work->actor_grayclut = NewMonochromaticEffect( DG_DMAPACK_PHASE_AFTER, DMAPACK_PRIO-1, pal );
      if ( work->actor_grayclut == NULL ) return ( -1 );
      GV_SetActorChild( &work->actor, work->actor_grayclut );
      DG_DisplayStatus |= DG_STATE_IR_MODE ;
   } else {
      /* 暗視ゴーグル */
      u_int		pal[256], *data ;
      data = pal ;
#if BP_VITA
		for ( i = 0 ; i < 64 ; i++, data++ ){
			*data = 0x80000000 | ( ( 240 * i / 63 ) << 8 ) ;
		}
		for ( i = 0 ; i < 64 ; i++, data++ ){
			int		tmp = ( 240 * i / 63 ) ;
			*data = 0x8000f000 | ( tmp << 16 ) | ( tmp ) ;
		}
		for ( i = 0 ; i < 128 ; i++, data++ ){
			*data = 0x80f0f0f0 ;
		}
#else
      for ( i = 0 ; i < 32 ; i++, data++ ){
         *data = 0x80000000 | ( ( 240 * i / 31 ) << 8 ) ;
      }
      for ( i = 0 ; i < 32 ; i++, data++ ){
         int		tmp = ( 240 * i / 31 ) ;
         *data = 0x8000f000 | ( tmp << 16 ) | ( tmp ) ;
      }
      for ( i = 0 ; i < 64 ; i++, data++ ){
         *data = 0x80f0f0f0 ;
      }
      for ( i = 0 ; i < 128 ; i++, data++ ){
         *data = 0x80f0f0f0 ;
      }
#endif
      work->actor_grayclut = NewMonochromaticEffect( DG_DMAPACK_PHASE_AFTER, DMAPACK_PRIO-1, pal );
      if ( work->actor_grayclut == NULL ) return ( -1 );
      GV_SetActorChild( &work->actor, work->actor_grayclut );
   }

   work->color = col ;

   work->alloc_addr = packet = GV_Malloc( sizeof(ALL_PACKET) );
   if ( packet == NULL ) return ( -1 );
   GV_ZeroMemory( packet, sizeof(ALL_PACKET) );
   dmapack->autopacket = packet ;

   SetRasterDrawPacket( &packet->raster_draw, 0x02, SCE_GS_SET_ALPHA(0,1,2,1,64), 0x80000000, 0 );
   DG_SetDmapackEnd( &packet->end );

   DG_AS_DMAPackSetCallbacks(dmapack, BP_IRModeCallback, NULL);
   dmapack->BP_callbackParam = work;

   return (0);
}
#endif

#else
/*
	ＸＢＯＸ用ルーチン
*/
//#define ENABLE_NOISE

typedef struct _screen_draw {
	DG_DMAPACK_ALPHA		alpha ;
	DG_DMAPACK_SPRT			sprt ;
} SCREEN_DRAW ;

typedef struct _noise_filter {
	DG_DMAPACK_PARAM		tex ;
	DG_DMAPACK_ALPHA		alpha ;
	DG_DMAPACK_SPRT			sprt ;
} NOISE_FILTER ;

typedef struct _raster_draw {
	DG_DMAPACK_ALPHA	alpha ;
#ifndef KP_WINDOWS
	DG_DMAPACK_BOX		box[ RASTER_NUM ];
#else
	DG_DMAPACK_LINE_F	line[ RASTER_NUM ];
#endif
} RASTER_DRAW ;

typedef struct {
#ifdef ENABLE_NOISE
	/* ノイズフィルター */
	NOISE_FILTER		noise ;
#endif

	/* 初期化パケット */
	DG_DMAPACK_PARAM	backupframe ;

	/* メイン描画パケット */
	DG_DMAPACK_CALLBACK	callback0 ;

	/* 描画パケット */

	/* 走査線パケット */
	RASTER_DRAW	raster_draw ;

	DG_DMAPACK_PARAM	end ;
} ALL_PACKET ;

extern DG_VERTEXSHADER	DG_DmapackVertexShader[] ;
extern PIXEL_SHADER_ps_irmode[] ;
static int				pixelshader_init = 0 ;
static DG_PIXELSHADER	IRModePixelShader ;
#ifdef ENABLE_NOISE
static DG_TEX_LIN	local_lin_tex ;
#endif
/* ---------------------------------------------------------------- */
/* DMAPACKパケット中で呼ばれるコールバック描画ルーチン */
/* グレイスケール化された退避バッファを１Ｄテクスチャで再マップして描画 */
static void DrawIRMode( int param )
{
	float	u, v, uw, vh, du0, du1, du2, du3, dv0, dv1, dv2, dv3 ;
	Work	*work = (void*)param ;
	DG_SetTextureDirect( 0, DG_SystemTexture[2].tex_trans.ptex );
#ifndef KP_WINDOWS
	DG_SetTextureDirect( 1, &work->color_tex->d3dtexture );
#else
	DG_SetTextureDirect( 1, work->color_tex->d3dtexture );
#endif
#ifndef KP_WINDOWS
	DG_SelectVertexShader( &DG_DmapackVertexShader[1], NULL );
#else
	DG_SelectVertexShader( &DG_DmapackVertexShader[1] );
#endif
	DG_SetPixelShader( &IRModePixelShader );
	//DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ) );
	DG_SetAlphaMode( 0 );

	u = ( DISPLAY_WIDTH - DRAW_WIDTH ) / 2 + 0.5f ;
	v = ( DISPLAY_HEIGHT - DRAW_HEIGHT ) / 2 + 0.5f ;
	uw = DRAW_WIDTH ;
	vh = DRAW_HEIGHT ;
	du0 =  0.0f ; dv0 =  0.0f ;
	//du1 =  1.0f ; dv1 =  0.0f ;
	//du2 =  0.0f ; dv2 = -1.0f ;
	//du3 =  0.0f ; dv3 =  1.0f ;
#ifndef KP_WINDOWS
	IDirect3DDevice8_Begin( g_pd3dDevice, D3DPT_TRIANGLESTRIP );
	IDirect3DDevice8_SetVertexData4ub( g_pd3dDevice, 3, 128, 128, 128, 255 );

	IDirect3DDevice8_SetVertexData2f( g_pd3dDevice,  7, u+du0, v+dv0 );
	//IDirect3DDevice8_SetVertexData2f( g_pd3dDevice,  8, u+du1, v+dv1 );
	//IDirect3DDevice8_SetVertexData2f( g_pd3dDevice,  9, u+du2, v+dv2 );
	//IDirect3DDevice8_SetVertexData2f( g_pd3dDevice, 10, u+du3, v+dv3 );
	IDirect3DDevice8_SetVertexData2s( g_pd3dDevice,  0, 0, 0 );

	IDirect3DDevice8_SetVertexData2f( g_pd3dDevice,  7, u+uw+du0, v+dv0 );
	//IDirect3DDevice8_SetVertexData2f( g_pd3dDevice,  8, u+uw+du1, v+dv1 );
	//IDirect3DDevice8_SetVertexData2f( g_pd3dDevice,  9, u+uw+du2, v+dv2 );
	//IDirect3DDevice8_SetVertexData2f( g_pd3dDevice, 10, u+uw+du3, v+dv3 );
	IDirect3DDevice8_SetVertexData2s( g_pd3dDevice,  0, DRAW_WIDTH, 0 );

	IDirect3DDevice8_SetVertexData2f( g_pd3dDevice,  7, u+du0, v+vh+dv0 );
	//IDirect3DDevice8_SetVertexData2f( g_pd3dDevice,  8, u+du1, v+vh+dv1 );
	//IDirect3DDevice8_SetVertexData2f( g_pd3dDevice,  9, u+du2, v+vh+dv2 );
	//IDirect3DDevice8_SetVertexData2f( g_pd3dDevice, 10, u+du3, v+vh+dv3 );
	IDirect3DDevice8_SetVertexData2s( g_pd3dDevice,  0, 0, DRAW_HEIGHT );

	IDirect3DDevice8_SetVertexData2f( g_pd3dDevice,  7, u+uw+du0, v+vh+dv0 );
	//IDirect3DDevice8_SetVertexData2f( g_pd3dDevice,  8, u+uw+du1, v+vh+dv1 );
	//IDirect3DDevice8_SetVertexData2f( g_pd3dDevice,  9, u+uw+du2, v+vh+dv2 );
	//IDirect3DDevice8_SetVertexData2f( g_pd3dDevice, 10, u+uw+du3, v+vh+dv3 );
	IDirect3DDevice8_SetVertexData2s( g_pd3dDevice,  0, DRAW_WIDTH, DRAW_HEIGHT );
	IDirect3DDevice8_End( g_pd3dDevice );
#else
	{
		DG_VERTEX_DMAPACK2D4	vertex[4] ;
		DWORD					sofs ;

		vertex[0].x    = 0 ;
		vertex[0].y    = 0 ;
		vertex[0].rgba = 0xff808080 ;
		vertex[0].u0   = (short)(u+du0) ;
		vertex[0].v0   = (short)(v+dv0) ;

		vertex[1].x    = DRAW_WIDTH ;
		vertex[1].y    = 0 ;
		vertex[1].rgba = 0xff808080 ;
		vertex[1].u0   = vertex[0].u0 + (short)uw ;
		vertex[1].v0   = vertex[0].v0 ;

		vertex[2].x    = 0 ;
		vertex[2].y    = DRAW_HEIGHT ;
		vertex[2].rgba = 0xff808080 ;
		vertex[2].u0   = vertex[0].u0 ;
		vertex[2].v0   = vertex[0].v0 + (short)vh ;

		vertex[3].x    = DRAW_WIDTH ;
		vertex[3].y    = DRAW_HEIGHT ;
		vertex[3].rgba = 0xff808080 ;
		vertex[3].u0   = vertex[1].u0 ;
		vertex[3].v0   = vertex[2].v0 ;

		DG_SetDynamicVertexBuffer(vertex, sizeof(DG_VERTEX_DMAPACK2D4),
								4, &sofs) ;
		DG_DrawPrimitive( D3DPT_TRIANGLESTRIP, sofs, 2 );
	}
#endif

#if 0 //BP
	DG_SelectVertexShader( &DG_DmapackVertexShader[0] );
	DG_SetPixelShader( NULL );
	DG_SetTextureDirect( 1, NULL );
#endif
}

#ifdef KP_WINDOWS
static void DrawIRModeNVS( int param )	// VertexShader非対応版
{
	float	u, v, uw, vh, du0, du1, du2, du3, dv0, dv1, dv2, dv3 ;
	Work	*work = (void*)param ;
	DWORD	color ;

	DG_SetTextureDirect( 0, DG_SystemTexture[2].tex_trans.ptex );
	DG_SetTextureDirect( 1, work->color_tex->d3dtexture );
	DG_SetPixelShader( &IRModePixelShader );

	DG_SetVertexShader(D3DFVF_DG_VERTEX_DMAPACK2D2_FLAG);
	DG_SetTransform(D3DTS_TEXTURE0, &DG_UnitMatrix) ;	// UV直接
	DG_SetTransform(D3DTS_TEXTURE1, &DG_UnitMatrix) ;	// UV直接

	//DG_SetAlphaMode( SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ) );
	DG_SetAlphaMode( 0 );

	u = ((float)( DISPLAY_WIDTH - DRAW_WIDTH ) / 2.0f + 0.5f) * (1.0f / DISPLAY_WIDTH)  ;
	v = ((float)( DISPLAY_HEIGHT - DRAW_HEIGHT ) / 2.0f + 0.5f) * (1.0f / DISPLAY_HEIGHT) ;
	uw = (float)DRAW_WIDTH / (float)DISPLAY_WIDTH ;
	vh = (float)DRAW_HEIGHT / (float)DISPLAY_HEIGHT ;
	{
		extern float	DG_BackBufferWidthAdjustCoef ;	// RendWidth/Width
		extern float	DG_BackBufferHeightAdjustCoef ;	// RendHeight/Height

		u  *= DG_BackBufferWidthAdjustCoef ;
		v  *= DG_BackBufferHeightAdjustCoef ;
		uw *= DG_BackBufferWidthAdjustCoef ;
		vh *= DG_BackBufferHeightAdjustCoef ;
	}

	color = 0xff808080 ;
	du0 =  0.0f ; dv0 =  0.0f ;
	//du1 =  1.0f ; dv1 =  0.0f ;
	//du2 =  0.0f ; dv2 = -1.0f ;
	//du3 =  0.0f ; dv3 =  1.0f ;
	{
		DG_VERTEX_DMAPACK2D2_FVF	vertex[4] ;
		DWORD					sofs ;

		vertex[0].x    = 0.0f ;
		vertex[0].y    = 0.0f ;
		vertex[0].z    = 0.0f ;
		vertex[0].rgba = color ;
		vertex[0].u0   = (u+du0) ;
		vertex[0].v0   = (v+dv0) ;
		vertex[0].u1   = 0.0f ;
		vertex[0].v1   = 0.0f ;

		vertex[1].x    = DRAW_WIDTH ;
		vertex[1].y    = 0.0f ;
		vertex[1].z    = 0.0f ;
		vertex[1].rgba = color ;
		vertex[1].u0   = vertex[0].u0 + uw ;
		vertex[1].v0   = vertex[0].v0 ;
		vertex[1].u1   = 0.0f ;
		vertex[1].v1   = 0.0f ;

		vertex[2].x    = 0.0f ;
		vertex[2].y    = DRAW_HEIGHT ;
		vertex[2].z    = 0.0f ;
		vertex[2].rgba = color ;
		vertex[2].u0   = vertex[0].u0 ;
		vertex[2].v0   = vertex[0].v0 + vh ;
		vertex[2].u1   = 0.0f ;
		vertex[2].v1   = 0.0f ;

		vertex[3].x    = DRAW_WIDTH ;
		vertex[3].y    = DRAW_HEIGHT ;
		vertex[3].z    = 0.0f ;
		vertex[3].rgba = color ;
		vertex[3].u0   = vertex[1].u0 ;
		vertex[3].v0   = vertex[2].v0 ;
		vertex[3].u1   = 0.0f ;
		vertex[3].v1   = 0.0f ;

		DG_SetDynamicVertexBuffer(vertex, sizeof(DG_VERTEX_DMAPACK2D2_FVF),
								4, &sofs) ;
		DG_DrawPrimitive( D3DPT_TRIANGLESTRIP, sofs, 2 );
	}

	DG_SetVertexShader(D3DFVF_DG_VERTEX_DMAPACK2D2_FLAG);
	DG_SetPixelShader( NULL );
	DG_SetTextureDirect( 1, NULL );
}
#endif

#ifdef KP_WINDOWS
static void DrawIRModeNVSNPS( int param )	// VertexShader非対応/PixelShader非対応版
{
	float	u, v, uw, vh, du0, du1, du2, du3, dv0, dv1, dv2, dv3 ;
	Work	*work = (void*)param ;
	DWORD	color ;
	DG_VERTEX_DMAPACK2D2_FVF	vertex[4] ;
	DWORD						sofs ;

	u = ((float)( DISPLAY_WIDTH - DRAW_WIDTH ) / 2.0f + 0.5f) * (1.0f / DISPLAY_WIDTH)  ;
	v = ((float)( DISPLAY_HEIGHT - DRAW_HEIGHT ) / 2.0f + 0.5f) * (1.0f / DISPLAY_HEIGHT) ;
	uw = (float)DRAW_WIDTH / (float)DISPLAY_WIDTH ;
	vh = (float)DRAW_HEIGHT / (float)DISPLAY_HEIGHT ;
	{
		extern float	DG_BackBufferWidthAdjustCoef ;	// RendWidth/Width
		extern float	DG_BackBufferHeightAdjustCoef ;	// RendHeight/Height

		u  *= DG_BackBufferWidthAdjustCoef ;
		v  *= DG_BackBufferHeightAdjustCoef ;
		uw *= DG_BackBufferWidthAdjustCoef ;
		vh *= DG_BackBufferHeightAdjustCoef ;

		du0 = 0.0f ;
		dv0 = 0.0f ;
		du1 = (-4.0f * DG_BackBufferWidthAdjustCoef)/(float)DISPLAY_WIDTH  ;
		dv1 = (-4.0f * DG_BackBufferHeightAdjustCoef)/(float)DISPLAY_HEIGHT ;
	}

	/*-- 描画環境設定 ------------------------------------------*/

	DG_SetTextureDirect( 0, DG_SystemTexture[2].tex_trans.ptex );

	DG_SetVertexShader(D3DFVF_DG_VERTEX_DMAPACK2D2_FLAG);
	DG_SetTransform(D3DTS_TEXTURE0, &DG_UnitMatrix) ;	// UV直接
	DG_SetTransform(D3DTS_TEXTURE1, &DG_UnitMatrix) ;	// UV直接

	DG_SetRenderState( D3DRS_ALPHATESTENABLE, FALSE) ;

	if( work->mode == 0 )
	{
		/*-- 赤外線 --*/
		color = 0xffff0801 ;
		DG_SetTextureDirect( 0, DG_SystemTexture[2].tex_trans.ptex );
	}
	else
	{
		/*-- 暗視 --*/
		color = 0xffc0ffc0 ;

		DG_SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE) ;
		DG_SetRenderState(D3DRS_BLENDOP,          D3DBLENDOP_ADD) ;
		DG_SetRenderState(D3DRS_SRCBLEND,         D3DBLEND_ONE) ;
		DG_SetRenderState(D3DRS_DESTBLEND,        D3DBLEND_ONE) ;
	}
	/*----------------------------------------------------------*/

	/*-- 通常描画 ----------------------------------------------*/

	{
		vertex[0].x    = 0.0f ;
		vertex[0].y    = 0.0f ;
		vertex[0].z    = 0.0f ;
		vertex[0].rgba = color ;
		vertex[0].u0   = (u+du0) ;
		vertex[0].v0   = (v+dv0) ;
		vertex[0].u1   = (u+du1) ;
		vertex[0].v1   = (v+dv1) ;

		vertex[1].x    = DRAW_WIDTH ;
		vertex[1].y    = 0.0f ;
		vertex[1].z    = 0.0f ;
		vertex[1].rgba = color ;
		vertex[1].u0   = vertex[0].u0 + uw ;
		vertex[1].v0   = vertex[0].v0 ;
		vertex[1].u1   = vertex[0].u1 + uw ;
		vertex[1].v1   = vertex[0].v1 ;

		vertex[2].x    = 0.0f ;
		vertex[2].y    = DRAW_HEIGHT ;
		vertex[2].z    = 0.0f ;
		vertex[2].rgba = color ;
		vertex[2].u0   = vertex[0].u0 ;
		vertex[2].v0   = vertex[0].v0 + vh ;
		vertex[2].u1   = vertex[0].u1 ;
		vertex[2].v1   = vertex[0].v1 + vh ;

		vertex[3].x    = DRAW_WIDTH ;
		vertex[3].y    = DRAW_HEIGHT ;
		vertex[3].z    = 0.0f ;
		vertex[3].rgba = color ;
		vertex[3].u0   = vertex[1].u0 ;
		vertex[3].v0   = vertex[2].v0 ;
		vertex[3].u1   = vertex[1].u1 ;
		vertex[3].v1   = vertex[2].v1 ;

		DG_SetDynamicVertexBuffer(vertex, sizeof(DG_VERTEX_DMAPACK2D2_FVF),
								4, &sofs) ;
		DG_DrawPrimitive( D3DPT_TRIANGLESTRIP, sofs, 2 );
	}
	/*----------------------------------------------------------*/

#if FALSE
	/*-- 描画環境設定 ------------------------------------------*/

	{
		/*-- 2nd Stage: 差分 -----------------------------------*/

		DG_SetTextureDirect( 1, DG_SystemTexture[2].tex_trans.ptex );
		DG_SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SUBTRACT) ;
		/*------------------------------------------------------*/

		/*-- 3rd Stage ２倍化 ----------------------------------*/

		DG_SetTextureDirect(2, NULL) ;
		DG_SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_ADD) ;
		DG_SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_CURRENT) ;
		DG_SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_CURRENT) ;

		DG_SetTextureStageState(2, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1) ;
		DG_SetTextureStageState(2, D3DTSS_ALPHAARG1, D3DTA_CURRENT) ;
		/*------------------------------------------------------*/
	}

	DG_SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE) ;
	DG_SetRenderState(D3DRS_BLENDOP,   D3DBLENDOP_ADD) ;

	DG_SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ZERO) ;
	DG_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR) ;

	DG_SetRenderState( D3DRS_ALPHATESTENABLE, TRUE) ;
	/*----------------------------------------------------------*/

	/*-- 輪郭描画 ----------------------------------------------*/

	color = 0xff808080 ;
	{
		vertex[0].rgba = color ;
		vertex[1].rgba = color ;
		vertex[2].rgba = color ;
		vertex[3].rgba = color ;

		DG_SetDynamicVertexBuffer(vertex, sizeof(DG_VERTEX_DMAPACK2D2_FVF),
								4, &sofs) ;
		DG_DrawPrimitive( D3DPT_TRIANGLESTRIP, sofs, 2 );
	}
	/*----------------------------------------------------------*/
#endif
	/*-- 描画環境復帰処理 --------------------------------------*/

	DG_SetVertexShader(D3DFVF_DG_VERTEX_DMAPACK2D_FLAG);

	DG_SetTextureDirect( 1, NULL );
	DG_SetAlphaMode( 0 );

	DG_InitTextureStageState(2) ;
	/*----------------------------------------------------------*/
}
#endif
/* ---------------------------------------------------------------- */
/* パケット初期化 */
static void InitDmaPacket( ALL_PACKET *packet, Work *work )
{
	DG_SetDmapackBackupFrame( &packet->backupframe, 0|0x04 );
#ifndef KP_WINDOWS
	DG_SetDmapackPacketCallback( &packet->callback0, DrawIRMode, (int)work );
#else
	{
		void	*callback_func ;


		if( DG_CheckUseVertexShader() )
		{
			callback_func = (void *)DrawIRMode ;
		}
		else
		{
			if( DG_CheckPixelShaderUseable() )
			{
				callback_func = (void *)DrawIRModeNVS ;
			}
			else
			{
				callback_func = (void *)DrawIRModeNVSNPS ;
			}
		}
		DG_SetDmapackPacketCallback( &packet->callback0, callback_func, (int)work );
	}
#endif
	DG_SetDmapackEnd( &packet->end );
	//printf("%p %p %p\n", &packet->backupframe, &packet->callback0, &packet->raster_draw.alpha, &packet->end );
	//while( 1 );
}
#ifdef ENABLE_NOISE
/* ノイズパケット生成 */
static void SetNoisePacket( NOISE_FILTER *packet )
{
	float	u, v, uw, vh ;
	DG_SetDmapackTexLin( &packet->tex, &local_lin_tex );
	DG_SetDmapackAlpha( &packet->alpha, SCE_GS_SET_ALPHA( 0, 1, 2, 1, 32 ) );
	
	u = ( BP_PS2_rand() * 128 ) / (BP_PS2_RAND_MAX+1) ;
	v = ( BP_PS2_rand() * 128 ) / (BP_PS2_RAND_MAX+1) ;
	uw = 512 / 2 ;
	vh = 512 / 2 ;
	u /= 512 ;
	v /= 512 ;
	uw /= 512 ;
	vh /= 512 ;
	DG_SetDmapackSprt( &packet->sprt,
					  0, 0, u, v,
					  DRAW_WIDTH, DRAW_HEIGHT, u+uw, v+vh, 0x80808080 );
}
#endif
/* 走査線パケット生成 */
static void SetRasterDrawPacket( RASTER_DRAW *raster, int flag,
								u_long64 alpha,
								int rgba, int y_offset )
{
	int		i ;

	/* アルファ設定 */
	DG_SetDmapackAlpha( &raster->alpha, alpha );

#ifndef KP_WINDOWS
	{
		int		x, y, w, h ;
		x = 0 ;
		w = DRAW_WIDTH ;
		y = y_offset ;
		h = 1 ;
		for ( i = 0 ; i < RASTER_NUM ; i++ ){
			DG_SetDmapackBox( &raster->box[ i ], x, y, x+w, y+h, rgba );
			y += 4 ;
		}
	}
#else
	{
		float	x, y, w ;
		float	ofs ;
		int		iofs ;

		/* ライン描画設定 */
		iofs = (DG_BackBufferRendHeight + (RASTER_NUM-1))/RASTER_NUM ;
		if( iofs < 2 ){ iofs = 2 ; }
		ofs = (float)(DRAW_HEIGHT * iofs)/(float)DG_BackBufferRendHeight ;

		x = 0 ;
		w = (float)DRAW_WIDTH ;
		y = (float)y_offset ;
		for ( i = 0 ; i < RASTER_NUM ; i++ ){
			DG_SetDmapackLineF( &raster->line[ i ], x, y, rgba, x+w, y, rgba );
			y += ofs ;
		}
	}
#endif
}
/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	ALL_PACKET	*packet ;

	if ( work->mode == 0 && GV_PauseLevel == 0 ){
		int		r, g, b, a, col ;
	}
#ifdef ENABLE_NOISE
	{
		ALL_PACKET *packet = work->dmapack->autopacket ;
		SetNoisePacket( &packet->noise );
	}
#endif
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	/* パケットメモリ開放 */
	if ( work->alloc_addr != NULL ){
		GV_DelayedFree( work->alloc_addr );
	}
	/* ＤＭＡパケットオブジェクト開放 */
	if ( work->dmapack != NULL ){
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
	/* 赤外線モード終了 */
	if ( work->mode == 0 ){
		DG_DisplayStatus &= ~DG_STATE_IR_MODE ;
	}
	/* ピクセルシェーダー開放 */
	if ( pixelshader_init == 1 ){
		DG_FreePixelShader( &IRModePixelShader );
	}
	pixelshader_init-- ;
	if ( work->color_tex != NULL ){
		DG_FreeLinerTexture( work->color_tex );
	}

#ifdef KP_WINDOWS
#ifdef ENABLE_NOISE
	DG_FreeLinerTexture2( &local_lin_tex );
#endif
#endif
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int mode )
{
	DG_DMAPACK	*dmapack ;
	ALL_PACKET	*packet ;
	u_int		col, mask, col2, i ;

	work->mode= mode ;

	/* ＤＭＡパケット型オブジェクト作成 */
	work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_NORMAL|
//											 DG_DMAPACK_INVISIBLE0|
											 DG_DMAPACK_INVISIBLE1|
											 DG_DMAPACK_INVISIBLE2|
											 DG_DMAPACK_INVISIBLE3
											 , DG_DMAPACK_PHASE_AFTER, DMAPACK_PRIO );
	if ( dmapack == NULL ) return ( -1 );
	DG_QueueDmapack( dmapack );

	if ( mode == 0 ){
		/* 赤外線 */
		col = 0x800018ac ;	/* モジュレートカラー */
		mask = 0x00e0e0e8 ;	/* ビットマスク指定 */
		col2 = 0x00263e26 ;	/* ラスタ加算カラー */
		col2 = 0x804c7c4c ;	/* ラスタ加算カラー */
#if 0
#if 1
		work->actor_grayclut = NewGrayClut( -1, 0x000030 );	/* パレット変更処理 */
#else
		work->actor_grayclut = NewGrayClut2( -1, 0x000060, col );	/* パレット変更処理 */
#endif
		if ( work->actor_grayclut == NULL ) return ( -1 );
		GV_SetActorChild( &work->actor, work->actor_grayclut );
#endif
		DG_DisplayStatus |= DG_STATE_IR_MODE ;
	} else {
		/* 暗視ゴーグル */
		col = 0x804bc017 ;	/* モジュレートカラー */
		mask = 0x00e0ffe0 ;	/* ビットマスク指定 */
		col2 = 0x80083010 ;	/* ラスタ加算カラー */
#if 0
#if 1
//		work->actor_grayclut = NewGrayClut( -1, 0x303030 );	/* パレット変更処理 */
		work->actor_grayclut = NewGrayClut( -1, 0x000000 );	/* パレット変更処理 */
#else
		work->actor_grayclut = NewGrayClut2( -1, 0x106030, col );	/* パレット変更処理 */
#endif
		if ( work->actor_grayclut == NULL ) return ( -1 );
		GV_SetActorChild( &work->actor, work->actor_grayclut );
#endif
	}
	work->color = col ;

	/* ピクセルシェーダー初期化 */
   BP_RENDER_TODO_BREAK;
#if 0//BP
	if ( pixelshader_init == 0 ){
		DG_MakePixelShader( &IRModePixelShader, PSHT_ps_irmode );
	}
#endif
	pixelshader_init++ ;

	/* パケットメモリ割り当て */
	work->alloc_addr = packet = GV_Malloc( sizeof(ALL_PACKET) );
	if ( packet == NULL ) return ( -1 );
	GV_ZeroMemory( packet, sizeof(ALL_PACKET) );
	dmapack->autopacket = packet ;
	//dmapack->packet[0] = &packet[0] ;
	//dmapack->packet[1] = &packet[1] ;

	InitDmaPacket( packet, work );
#ifndef KP_WINDOWS
	SetRasterDrawPacket( &packet->raster_draw, 0x02, SCE_GS_SET_ALPHA(0,1,2,1,64), 0x80000000, 0 );
#else
	{
		DWORD	alpha ;

		/* 実際の描画面積がX-BOX版より大きい場合は補正 */
		if( DG_BackBufferRendHeight < DRAW_HEIGHT )
		{
			alpha = (DWORD)(((float)DG_BackBufferRendHeight * 64.0f)/(float)DRAW_HEIGHT) ;
		}
		else
		{
			alpha = 64 ;
		}
		SetRasterDrawPacket( &packet->raster_draw, 0x02,
						SCE_GS_SET_ALPHA(0,1,2,1,alpha), 0x80000000, 0 );
	}
#endif

	work->color_tex = DG_MakeLinerTexture( 128, 1, DG_TEXLIN_FORMAT_A8R8G8B8 );
	{/* テクスチャ生成 */
		u_int	*data = work->color_tex->image ;
		*data = work->color_tex->image ;
		if ( mode == 0 ){
			/* 赤外線ゴーグル */
			static u_int	color_table[8] = {
				0x800080ff,0x8000ffff,0x8000ff80,0x8000ff00,
				0x8080ff00,0x80ffff00,0x80ff8000,0x80ff0000
			};
			for ( i = 0 ; i < 128 ; i++, data++ ){
				*data = color_table[ i / 16 ] ;
			}
		} else {
			/* 暗視ゴーグル */
#if 0
			static u_int	color_table[8] = {
				0x80000000,0x80005500,0x8000aa00,0x8000ff00,
				0x8080ff80,0x80c0ffc0,0x80ffffff,0x80ffffff
			};
			for ( i = 0 ; i < 128 ; i++, data++ ){
				*data = color_table[ i / 16 ] ;
			}
#else
			static u_int	color_table[16] = {
				0x80000000,0x80005500,0x8000aa00,0x8000ff00,
				0x8080ff80,0x80c0ffc0,0x80ffffff,0x80ffffff,
				0x80ffffff,0x80ffffff,0x80ffffff,0x80ffffff,
				0x80ffffff,0x80ffffff,0x80ffffff,0x80ffffff
			};
#if 0
			for ( i = 0 ; i < 64 ; i++, data++ ){
				*data = color_table[ i / 4 ] ;
			}
			for ( i = 0 ; i < 64 ; i++, data++ ){
				*data = color_table[ 15 ] ;
			}
#else
			for ( i = 0 ; i < 16 ; i++, data++ ){
				*data = 0x80000000 | ( ( 240 * i / 15 ) << 8 ) ;
			}
			for ( i = 0 ; i < 16 ; i++, data++ ){
				int		tmp = ( 240 * i / 15 ) ;
				*data = 0x8000f000 | ( tmp << 16 ) | ( tmp ) ;
			}
			for ( i = 0 ; i < 32 ; i++, data++ ){
				*data = 0x80f0f0f0 ;
			}
			for ( i = 0 ; i < 64 ; i++, data++ ){
				*data = 0x80f0f0f0 ;
			}
#endif
#endif
		}
	}
	/* 強制的に通常のテクスチャフォーマットに置き換える */
#ifndef KP_WINDOWS

#if 0 //BP
	XGSetTextureHeader( 128, 1, 1, 0, D3DFMT_A8R8G8B8, 0,
					   (LPDIRECT3DTEXTURE8*)&work->color_tex->d3dtexture, 0, 512 );
	IDirect3DTexture8_Register( (LPDIRECT3DTEXTURE8*)&work->color_tex->d3dtexture, work->color_tex->image );
#endif

#endif

	/* 作成したImageをTextureに適用させる */
#ifdef KP_WINDOWS
	DG_LinerTextureFlushImage(work->color_tex);
#endif

#ifdef ENABLE_NOISE
	{/* ノイズフィルター用ノイズテクスチャ生成 */
		extern char _program_top[];
		void	*addr = (void *)( (int)(_program_top + 0x001140) & 0x0fffff00 );
		DG_MakeLinerTexture2( &local_lin_tex, 512, 512, DG_TEXLIN_FORMAT_P8, addr );
	}
	SetNoisePacket( &packet->noise );
#endif


	return (0);
}
#endif


/* ---------------------------------------------------------------- */
	/*
		赤外線モードＯＮ
	*/
void *NewIRMode( void )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, 0 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

	/*
		暗視ゴーグルモードＯＮ
	*/
void *NewThermalMode( void )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, 1 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
