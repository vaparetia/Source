//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	raster.c
	走査線エフェクト（ir_mode.cから派生）

	2000/01/23 K.Takabe
	$Id: raster.c,v 1.3 2002/11/23 12:46:55 Yoshizawa1 Exp $

*/
/*
	void *NewRasterEffect( int alpha, int color );
	int		alpha ;		ＰＳ２アルファレジスタ設定値(SCE_GS_SET_ALPHA(a,b,c,d,alp))
	int		color ;		ラスター描画色
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#if 0 //BP_PS2 def PSX2
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

#ifdef KP_WINDOWS
extern DWORD	DG_BackBufferRendHeight ;
#endif

#define ALIGNSIZE1(_n) (_n)
#define ALIGNSIZE16(_n) (((_n)+15)&0xfffffff0)
#define ALIGNSIZE64(_n) (((_n)+63)&0xffffffc0)
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)

#define DRAW_DIV	(16)		/* 描画の分割数 */
#define RASTER_NUM	(DRAW_HEIGHT/4)	/* 112 */

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;
	void		*actor_grayclut ;
	DG_DMAPACK	*dmapack ;
	int			mode ;
	int			count ;
	void		*packet_mem ;
} Work ;

/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
#if 0 //BP_PS2 def PSX2
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
} RASTER_DRAW ;

typedef struct _frame_mask {				/* 本当はあまりこれを多用してはいけない */
	DG_GIFTAG	giftag ;
	DG_GSREG	frame ;
} FRAME_MASK ;

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

#if 0
		/* メイン描画パケット */
		FRAME_MASK	frame0 ;
		SCREEN_DRAW	screen_draw0 ;

		/* 描画パケット */
		FRAME_MASK	frame2 ;
		SCREEN_DRAW	screen_draw2 ;

		/* 描画パケット */
		FRAME_MASK	frame1 ;
		SCREEN_DRAW	screen_draw1 ;
#endif
		/* 走査線パケット */
//		FRAME_MASK	frame3 ;
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

/* ---------------------------------------------------------------- */
#if 0
static void SetFrameMaskPacket( FRAME_MASK *frame, int which, int mask )
{
	frame->giftag.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(DG_GSREG), 1, 0, 0, 0, 1);
	frame->giftag.regs = GS_REGS_AD ;
	frame->frame.reg = SCE_GS_FRAME_1 ;
	frame->frame.data =
	  SCE_GS_SET_FRAME( BUFFER_PAGE(which)/2048, BUFFER_WIDTH/64, FRAME_BUFFER_COLOR_MODE(), mask ) ;
}
#endif
/* ---------------------------------------------------------------- */
static void InitDmaPacket( ALL_PACKET *packet, int which, int chanl )
{
	packet->dmatag.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(struct _gif_packet) ) ;
	packet->dmatag.vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 ) ;
	packet->dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(struct _gif_packet), 0) ;
	packet->dmatag_drawenv.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_DRAWENV) ) ;
	packet->dmatag_drawenv.addr = &( DG_Chanl( chanl )->draw_env[ which ] ) ;
	packet->dmatag_drawenv.vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 ) ;
	packet->dmatag_drawenv.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_DRAWENV), 0) ;
	packet->dmatag_offset.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_DRAWOFFSET) ) ;
	packet->dmatag_offset.addr = &( DG_Chanl( chanl )->draw_offset[ which ] ) ;
	packet->dmatag_offset.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
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
#if 0
static void SetScreenDrawPacket( SCREEN_DRAW *screen_draw, int flag,
						 int test,
						 u_long64 alpha,
						 int tex_page,
						 int rgba )
{
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

}
#endif
static void SetRasterDrawPacket( RASTER_DRAW *raster_draw, int flag,
								u_long64 alpha,
								int rgba, int y_offset )
{
   BP_RENDER_TODO_BREAK;
#if 0 //BP_GCC
	static RASTER_DRAW	def_raster_draw = {
		.giftag = { .tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _raster_draw_data), 1, 0, 0, 0, 1),
					  .regs = GS_REGS_AD },
		.data = {
			.alpha = { .reg = SCE_GS_ALPHA_1, .data = SCE_GS_SET_ALPHA(0,1,2,1,64) },
			.prim  = { .reg = SCE_GS_PRIM,    .data = SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) },
			.rgbq  = { .reg = SCE_GS_RGBAQ,   .data = SCE_GS_SET_RGBAQ(128,128,128,128,0) },
		}
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
#endif
}
#else
/* ---------------------------------------------------------------- */
/*
	ＸＢＯＸ用ルーチン
*/

typedef struct _raster_draw {
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
} RASTER_DRAW ;

#ifdef KP_WINDOWS
typedef union {
	DG_DMAPACK_BOX_F	box ;
	DG_DMAPACK_LINE_F	line ;
} RASTER_PACKET ;
#endif

typedef struct {
	DG_DMAPACK_ALPHA	alpha ;
#ifndef KP_WINDOWS
	DG_DMAPACK_BOX		box[ RASTER_NUM ];
#else
	RASTER_PACKET		raster[ RASTER_NUM ];
#endif
	DG_DMAPACK_PARAM	end ;
} ALL_PACKET ;

/* ---------------------------------------------------------------- */
static void InitDmaPacket( ALL_PACKET *packet, int which, int chanl )
{
}
static void SetRasterDrawPacket( void *prim, int flag,
								u_long64 alpha,
								int rgba, int y_offset )
{
	int		i ;

	/* アルファ設定 */
	prim = DG_SetDmapackAlpha( prim, alpha );
	if ( ( rgba & 0xff000000 ) == 0 ) rgba |= 0x80000000 ;
#ifndef KP_WINDOWS
	{
		int		x, y, w, h ;
		x = 0 ;
		w = DRAW_WIDTH ;
		y = y_offset ;
		h = 3 ;
		for ( i = 0 ; i < RASTER_NUM ; i++ ){
			prim = DG_SetDmapackBox( prim, x, y, x+w, y+h, DG_MakeDmaPackColorFromInt(rgba) );
			y += 4 ;
		}
	}
#else
	{
		float	x, y, w, h ;
		float	ofs ;
		int		iofs ;
		int		rend_h ;

		/* ライン描画設定 */
		x = 0 ;
		w = (float)DRAW_WIDTH ;
		y = (float)y_offset ;

		rend_h = DG_BackBufferRendHeight ;
		iofs   = (rend_h + (RASTER_NUM-1))/RASTER_NUM ;
		if( iofs < 2 )
		{
			/* h == 1[pixel] */
			ofs = (float)(DRAW_HEIGHT * 2) / (float)rend_h ;

			for ( i = 0 ; (i < RASTER_NUM) && (y < DRAW_HEIGHT) ; i++ ){
				prim = DG_SetDmapackLineF( prim, x, y, rgba, x+w, y, rgba );
				y += ofs ;
			}
		}
		else
		{
			/* h > 1[pixel] */
			ofs = (float)(DRAW_HEIGHT * iofs) / (float)rend_h ;
			h   = (float)(DRAW_HEIGHT * (iofs-1)) / (float)rend_h ;

			for ( i = 0 ; (i < RASTER_NUM) && (y < DRAW_HEIGHT) ; i++ ){
				prim = DG_SetDmapackBox_F( prim, x, y, x+w, y+h, rgba );
				y += ofs ;
			}
		}
	}
#endif
	prim = DG_SetDmapackEnd( prim );
}
#endif


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	/* パケットメモリ開放 */
	if ( work->packet_mem != NULL ) GV_DelayedFree( work->packet_mem );
	/* ＤＭＡパケットオブジェクト開放 */
	if ( work->dmapack != NULL ){
		DG_DequeueDmapack( work->dmapack );
		DG_FreeDmapack( work->dmapack );
	}
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int alpha, int color, int chanl )
{
	DG_DMAPACK	*dmapack ;
	ALL_PACKET	*packet ;
	u_int		i, invisible_flag ;

#if 0
	/* ＤＭＡパケット型オブジェクト作成 */
	work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_NORMAL|
//											 DG_DMAPACK_INVISIBLE0|
											 DG_DMAPACK_INVISIBLE1|
											 DG_DMAPACK_INVISIBLE2|
											 DG_DMAPACK_INVISIBLE3
											 , DG_DMAPACK_PHASE_AFTER, 250 );
#else
	/* ＤＭＡパケット型オブジェクト作成 */
	invisible_flag =  ( (DG_DMAPACK_INVISIBLE0|DG_DMAPACK_INVISIBLE1|DG_DMAPACK_INVISIBLE2|DG_DMAPACK_INVISIBLE3) &
					   ~(DG_DMAPACK_INVISIBLE0 << chanl ) ) ;
	work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_NORMAL|invisible_flag, DG_DMAPACK_PHASE_AFTER, 250 );
	dmapack->flag &= ~(DG_DMAPACK_INVISIBLE0 << chanl );
#endif
	if ( dmapack == NULL ) return ( -1 );
	DG_QueueDmapack( dmapack );

#if 0 //BP_PS2 def PSX2
	/* ＰＳ２用ルーチン */
	/* パケットメモリ割り当て */
	work->packet_mem = packet = GV_Malloc( sizeof(ALL_PACKET) * 2 );
	if ( packet == NULL ) return ( -1 );
	GV_ZeroMemory( packet, sizeof(ALL_PACKET) * 2 );
	dmapack->packet[0] = &packet[0] ;
	dmapack->packet[1] = &packet[1] ;

	for ( i = 0 ; i < 2 ; i++ ){
		InitDmaPacket( &packet[i], i, chanl );
		SetRasterDrawPacket( &packet[i].gif_packet.raster_draw, 0x02, alpha, color, 0 );

	}
#else
	/* ＸＢＯＸ用ルーチン */
	work->packet_mem = packet = GV_Malloc( sizeof(ALL_PACKET) );
	if ( packet == NULL ) return ( -1 );
	GV_ZeroMemory( packet, sizeof(ALL_PACKET) );
	dmapack->autopacket = packet ;
	//dmapack->packet[0] = &packet[0] ;
	//dmapack->packet[1] = &packet[1] ;

	for ( i = 0 ; i < 2 ; i++ ){
		SetRasterDrawPacket( packet, 0x02, alpha, color, 0 );

	}
#endif


	return (0);
}


static int GetResourcesForCodec( Work *work, int alpha, int color, int chanl )
{
	DG_DMAPACK	*dmapack ;
	ALL_PACKET	*packet ;
	u_int		i, invisible_flag ;

#if 0
	/* ＤＭＡパケット型オブジェクト作成 */
	work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_NORMAL|
//											 DG_DMAPACK_INVISIBLE0|
											 DG_DMAPACK_INVISIBLE1|
											 DG_DMAPACK_INVISIBLE2|
											 DG_DMAPACK_INVISIBLE3
											 , DG_DMAPACK_PHASE_AFTER, 250 );
#else
	/* ＤＭＡパケット型オブジェクト作成 */
	invisible_flag =  ( (DG_DMAPACK_INVISIBLE0|DG_DMAPACK_INVISIBLE1|DG_DMAPACK_INVISIBLE2|DG_DMAPACK_INVISIBLE3) &
					   ~(DG_DMAPACK_INVISIBLE0 << chanl ) ) ;
	work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_NORMAL|DG_DMAPACK_PRIVILEGE|invisible_flag, DG_DMAPACK_PHASE_AFTER, 250 );
	dmapack->flag &= ~(DG_DMAPACK_INVISIBLE0 << chanl );
#endif
	DG_QueueDmapack( dmapack );

#if 0 //BP_PS2 def PSX2
	/* パケットメモリ割り当て */
	work->packet_mem = packet = GV_Malloc( sizeof(ALL_PACKET) * 2 );
	if ( packet == NULL ) return ( -1 );
	GV_ZeroMemory( packet, sizeof(ALL_PACKET) * 2 );
	dmapack->packet[0] = &packet[0] ;
	dmapack->packet[1] = &packet[1] ;

	for ( i = 0 ; i < 2 ; i++ ){
		InitDmaPacket( &packet[i], i, chanl );
		SetRasterDrawPacket( &packet[i].gif_packet.raster_draw, 0x02, alpha, color, 0 );

	}
#else
	/* ＸＢＯＸ用ルーチン */
	work->packet_mem = packet = GV_Malloc( sizeof(ALL_PACKET) );
	if ( packet == NULL ) return ( -1 );
	GV_ZeroMemory( packet, sizeof(ALL_PACKET) );
	dmapack->autopacket = packet ;
	//dmapack->packet[0] = &packet[0] ;
	//dmapack->packet[1] = &packet[1] ;

	for ( i = 0 ; i < 2 ; i++ ){
		SetRasterDrawPacket( packet, 0x02, alpha, color, 0 );

	}
#endif


	return (0);
}


/* ---------------------------------------------------------------- */
	/*
		
	*/
void *NewRasterEffect( int alpha, int color )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, alpha, color, 0 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewRasterEffectChanl( int alpha, int color, int chanl )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, alpha, color, chanl ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewRasterEffectChanlForCodec( int alpha, int color, int chanl )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_MANAGER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResourcesForCodec( work, alpha, color, chanl ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

