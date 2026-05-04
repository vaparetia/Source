//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mono_scn.c
	画面モノクロ化エフェクト（ＰＳ２専用、サブスタンスからの追加プログラム）

	2002/09/17 K.Takabe
	$Id: mono_scn.c,v 1.2 2002/11/25 01:54:46 Yoshizawa1 Exp $

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


#define ALIGNSIZE1(_n) (_n)
#define ALIGNSIZE16(_n) (((_n)+15)&0xfffffff0)
#define ALIGNSIZE64(_n) (((_n)+63)&0xffffffc0)
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)

#define DRAW_DIV	(16)		/* 描画の分割数 */
#define RASTER_NUM	(DRAW_HEIGHT/4)	/* 112 */

#define DMAPACK_PRIO	(140)	/* ２Ｄシステムより優先を低くする */


/* ---------------------------------------------------------------- */
extern qword Blue2RGBVu0 ;
extern qword Blue2RGBVu0_Func ;

/* ---------------------------------------------------------------- */
typedef	struct _mono_scn_Work {
	GV_ACT_EX	actor ;
	DG_DMAPACK	*dmapack ;
	int			mode ;
	int			count ;
	int			color ;
	void		*alloc_addr ;
} Work ;

static ALIGN16_PRE u_char		green_clut[16*16][3] ALIGN16_POST ;
static ALIGN16_PRE u_char		effect_clut[16*16][3] ALIGN16_POST ;
/* ---------------------------------------------------------------- */
/*
	ＰＳ２用ルーチン
*/

/* ＶＵ１セットアップ用ＤＭＡパケット */
typedef struct _vu1_setup_packet {
	DG_DMATAG	dmatag0 ;
	struct {
		DG_VIFCODE	vifcode0 ;
		struct {
			/* +00h */
			u_int			param[4] ;		/* vu1へ渡すデータ */
			/* +01h */
			DG_GIFTAG		giftag0 ;
			struct {
				DG_GSREG	frame ;
				DG_GSREG	tex0 ;
				DG_GSREG	prim ;
			} data0 ;
			/* +05h */
			DG_GIFTAG		giftag1 ;
		} pack0 ;
		DG_VIFCODE	vifcode1 ;
		struct {  
			/* +06h */
			struct {
				struct {
					u_long64		uv0 ;
					u_long64		xyz0 ;
					u_long64		uv1 ;
					u_long64		xyz1 ;
				} sprt[ 64*32/(8*2) ] ;
			} data1 ;
		} pack1 ;
		DG_VIFCODE	vifcode2 ;
		struct {  
			/* +106h */
			DG_GIFTAG		giftag2 ;
			struct {
				DG_GSREG	scissor ;
				DG_GSREG	offset ;
				DG_GSREG	texflush ;
				DG_GSREG	clamp ;
				DG_GSREG	tex1 ;
				DG_GSREG	alpha ;
				DG_GSREG	rgba ;
			} data2 ;
		} pack2 ;
	} init_data ;
	DG_DMATAG	dmatag1 ;		/* ＶＵ１マイクロプログラム読み込み＆実行 */
} VU1_SETUP_PACKET ;

typedef struct {
	DG_DMATAG			dmatag_wait ;		/* 描画終了ウェイト用 */
	DG_LOADIMAGE		load_green_clut ;	/* アルファチャンネル＞Ｂチャンネル変換用ＣＬＵＴ読み込み */
	DG_LOADIMAGE		load_effect_clut ;	/* エフェクトＣＬＵＴ */

	DG_DMATAG			dmatag_init ;
	struct {
		/* 初期化パケット */
		struct _init_packet {
			DG_GIFTAG		giftag ;
			struct _init_gif_data{
				DG_GSREG	texflush ;
				DG_GSREG	clamp ;
				DG_GSREG	texa ;
				DG_GSREG	tex1 ;
				DG_GSREG	test ;
			} data ;
		} init_packet ;
	} init_packet ;

	DG_DMATAG			dmatag0 ;			/* ＧＩＦ接続ＤＭＡタグ */
	struct {
		/* モノクロ化のためのモジュレート */
		struct {
			DG_GIFTAG		giftag ;
			struct {
				DG_GSREG	texflush ;
				DG_GSREG	frame ;
				DG_GSREG	scissor ;
				DG_GSREG	offset ;
				DG_GSREG	clamp ;
				DG_GSREG	tex0 ;
				DG_GSREG	alpha ;
				DG_GSREG	prim ;
				DG_GSREG	rgbq ;
				struct {
					DG_GSREG	uv0 ;
					DG_GSREG	xyz0 ;
					DG_GSREG	uv1 ;
					DG_GSREG	xyz1 ;
				} sprt[ DRAW_WIDTH/16 ] ;
			} data ;
		} pass0 ;
	} gif_packet0 ;

	DG_DMATAG			dmatag1 ;
	struct {
		/* 表のR,Gチャンネルを裏のB,Aチャンネルへ移動 */
		struct {
			DG_GIFTAG		giftag ;
			struct {
				DG_GSREG	frame ;
				DG_GSREG	scissor ;
				DG_GSREG	offset ;
				DG_GSREG	texflush ;
				DG_GSREG	clamp ;
				DG_GSREG	tex0 ;
				DG_GSREG	alpha ;
				DG_GSREG	prim ;
				DG_GSREG	rgbq ;
				struct {
					DG_GSREG	uv0 ;
					DG_GSREG	xyz0 ;
					DG_GSREG	uv1 ;
					DG_GSREG	xyz1 ;
				} sprt[ DRAW_WIDTH/16 ] ;
			} data ;
		} pass1 ;
	} gif_packet1 ;

	DG_DMATAG			dmatag2 ;
	struct {
		/* 表のB,Aチャンネルを裏のB,Aチャンネルへ加算 */
		struct {
			DG_GIFTAG		giftag ;
			struct {
				DG_GSREG	texflush ;
				DG_GSREG	frame ;
				DG_GSREG	scissor ;
				DG_GSREG	offset ;
				DG_GSREG	clamp ;
				DG_GSREG	tex0 ;
				DG_GSREG	alpha ;
				DG_GSREG	prim ;
				DG_GSREG	rgbq ;
				struct {
					DG_GSREG	uv0 ;
					DG_GSREG	xyz0 ;
					DG_GSREG	uv1 ;
					DG_GSREG	xyz1 ;
				} sprt[ DRAW_WIDTH/64 ] ;
			} data ;
		} pass2 ;
	} gif_packet2 ;

	DG_DMATAG			dmatag3 ;
	struct {
		/* 裏のA(G)チャンネルをパレットテクスチャとして裏のBチャンネルへ加算 */
		struct {
			DG_GIFTAG		giftag ;
			struct {
				DG_GSREG	texflush ;
				DG_GSREG	frame ;
				DG_GSREG	scissor ;
				DG_GSREG	offset ;
				DG_GSREG	clamp ;
				DG_GSREG	tex0 ;
				DG_GSREG	alpha ;
				DG_GSREG	prim ;
				DG_GSREG	rgbq ;
				struct {
					DG_GSREG	uv0 ;
					DG_GSREG	xyz0 ;
					DG_GSREG	uv1 ;
					DG_GSREG	xyz1 ;
				} sprt[ DRAW_WIDTH/64 ] ;
			} data ;
		} pass3 ;
	} gif_packet3 ;

#if 0	/* メモリ使用量が大きいのでＶＵ１化 */
		/* 裏のBチャンネルを８ビットパレットとして表へ転送 */
		struct {
			DG_GIFTAG		giftag ;
			struct {
				DG_GSREG	texflush ;
				DG_GSREG	frame ;
				DG_GSREG	scissor ;
				DG_GSREG	offset ;
				DG_GSREG	clamp ;
				DG_GSREG	tex0 ;
				DG_GSREG	alpha ;
				DG_GSREG	prim ;
				DG_GSREG	rgbq ;
			} data ;
			DG_GIFTAG		giftag2 ;
			struct {
				struct {
					u_long64		uv0 ;
					u_long64		xyz0 ;
					u_long64		uv1 ;
					u_long64		xyz1 ;
				} sprt[ DRAW_WIDTH*DRAW_HEIGHT/8/2 ] ;
			} data2 ;
		} pass4 ;
#endif

	/* 裏のBチャンネルを８ビットパレットとして表へ転送 */
	VU1_SETUP_PACKET	vu1_setup ;			/*  */

	DG_DMATAG			dmatag_reset ;
	struct _reset_packet{
		DG_GIFTAG		giftag ;
		struct {
			DG_GSREG		scissor ;
			DG_GSREG		offset ;
			DG_GSREG		test ;
			DG_GSREG		tex1 ;
			DG_GSREG		texflush ;
		} data ;
	} reset_packet ;

	DG_DMATAG			dmatag_drawenv ;	/* 描画環境復元用 */
	DG_DMATAG			dmatag_offset ;		/* オフセット環境復元用 */
	DG_DMATAG			dmatag_end ;		/* RETタグ */
} ALL_PACKET ;

/* ---------------------------------------------------------------- */
/* 指定した数のＧＳパケットを転送するためのＧＩＦパケットを設定 */
static void SetGifPacket( DG_GIFTAG	*giftag, int num )
{
	giftag->tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(DG_GSREG) * num, 1, 0, 0, 0, 1);
	giftag->regs = GS_REGS_AD ;
}
/* GSのレジスタを設定する */
static void SetGSREG( DG_GSREG *reg, int reg_num, u_long64 data )
{
	reg->reg = reg_num ;
	reg->data = data ;
}
/* ---------------------------------------------------------------- */
static void InitVu1SetupPacket( VU1_SETUP_PACKET *packet, int which )
{
#if 0 //BP_RENDER
	int		i, j, x, y, vumem_offset = 0 ;
	union { int	i ; float f ; } var_mem0, var_mem1 ;

	packet->dmatag0.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(packet->init_data) );
	packet->dmatag0.vifcode[0] = SCE_VIF1_SET_FLUSHE( 0 );
	packet->dmatag0.vifcode[1] = SCE_VIF1_SET_STCYCL( 1, 1, 0 );

	packet->init_data.vifcode0.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->init_data.vifcode0.vifcode[1] = SCE_VIF1_SET_NOP( 0 );
	packet->init_data.vifcode0.vifcode[2] = SCE_VIF1_SET_NOP( 0 );
	packet->init_data.vifcode0.vifcode[3] = SCE_VIF1_SET_UNPACK( vumem_offset, SIZEOF_QWORD(packet->init_data.pack0), VIF_DATA128, 0 );
	vumem_offset += SIZEOF_QWORD(packet->init_data.pack0) ;

	/* フレーム、テクスチャページ設定パケット生成 */
	SetGifPacket( &packet->init_data.pack0.giftag0, SIZEOF_QWORD(packet->init_data.pack0.data0) );
	SetGSREG( &packet->init_data.pack0.data0.frame,
			 SCE_GS_FRAME_1,
			 SCE_GS_SET_FRAME( BUFFER_PAGE(which)/2048, BUFFER_WIDTH/64, SCE_GS_PSMCT32, 0x00000000 ) );
	SetGSREG( &packet->init_data.pack0.data0.tex0,
			 SCE_GS_TEX0_1,
			 SCE_GS_SET_TEX0( BUFFER_PAGE(2)/64, DRAW_WIDTH*2/64, SCE_GS_PSMT8, 10, 10, 0, 1,
							 (BUFFER_PAGE(3)+256)/64, 1, SCE_GS_PSMCT32, 0, 1 ) );
	SetGSREG( &packet->init_data.pack0.data0.prim,
			 SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );
	//SetGSREG( &packet->init_data.pack0.data0.texflush, SCE_GS_TEXFLUSH, 0 );

	/* １ページ分のＢ成分コピー用スプライトプリミティブを生成 */
	packet->init_data.pack0.giftag1.tag = SCE_GIF_SET_TAG( 64*32/(8*2), 1, 0, 0, 1, 4);
	packet->init_data.pack0.giftag1.regs = 0x5353;/* UV,XYZ,UV,XYZ */

	packet->init_data.vifcode1.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->init_data.vifcode1.vifcode[1] = SCE_VIF1_SET_NOP( 0 );
	packet->init_data.vifcode1.vifcode[2] = SCE_VIF1_SET_NOP( 0 );
	packet->init_data.vifcode1.vifcode[3] = SCE_VIF1_SET_UNPACK( vumem_offset, SIZEOF_QWORD(packet->init_data.pack1)&0xff, VIF_DATA128, 0 );
	vumem_offset += SIZEOF_QWORD(packet->init_data.pack1) ;
	for ( j = 0, y = 0 ; y < 32 ; y += 2 ){
		for ( x = 0 ; x < 64 ; x += 8, j++ ){
			int		u0, v0, u1, v1 ;
			u0 = x * 2 + 8 ; if ( y & 2 ) u0 -= 4 ;
			u1 = u0 + 8 ;
			v0 = y * 2 + 0 ;
			v1 = y * 2 + 2 ;
			packet->init_data.pack1.data1.sprt[j].uv0 = SCE_GS_SET_UV( u0*16+8, v0*16+8 );
			packet->init_data.pack1.data1.sprt[j].uv1 = SCE_GS_SET_UV( u1*16+8, v1*16+8 );
			packet->init_data.pack1.data1.sprt[j].xyz0 = SCE_GS_SET_XYZ( (x)*16, (y)*16, DRAW_Z_MAX );
			packet->init_data.pack1.data1.sprt[j].xyz1 = SCE_GS_SET_XYZ( (x+8)*16, (y+2)*16, DRAW_Z_MAX );
			//if ( j == 0 ){
			//	packet->init_data.pack1.data1.sprt[j].uv0 = SCE_GS_SET_UV( u1*16+8, v1*16+8 );
			//	packet->init_data.pack1.data1.sprt[j].xyz0 = SCE_GS_SET_XYZ( (0)*16, (0)*16, DRAW_Z_MAX );
			//	packet->init_data.pack1.data1.sprt[j].xyz1 = SCE_GS_SET_XYZ( (640)*16, (320)*16, DRAW_Z_MAX );
			//} else {
			//	//packet->init_data.pack1.data1.sprt[j].xyz1 = SCE_GS_SET_XYZ( (x+0)*16, (y+2)*16, DRAW_Z_MAX );
			//}
			//packet->init_data.pack1.data1.sprt[j].xyz1 = SCE_GS_SET_XYZ( (x+16)*16, (y+4)*16, DRAW_Z_MAX );
		}
	}

	/* 初期化パケット生成 */
	packet->init_data.vifcode2.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->init_data.vifcode2.vifcode[1] = SCE_VIF1_SET_NOP( 0 );
	packet->init_data.vifcode2.vifcode[2] = SCE_VIF1_SET_NOP( 0 );
	packet->init_data.vifcode2.vifcode[3] = SCE_VIF1_SET_UNPACK( vumem_offset, SIZEOF_QWORD(packet->init_data.pack2), VIF_DATA128, 0 );
	vumem_offset += SIZEOF_QWORD(packet->init_data.pack2) ;
	SetGifPacket( &packet->init_data.pack2.giftag2, SIZEOF_QWORD(packet->init_data.pack2.data2) );
	//SetGSREG( &packet->init_data.pack2.data2.scissor,
	//		 SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR( 0, DRAW_WIDTH, 0, DRAW_HEIGHT ) );
	SetGSREG( &packet->init_data.pack2.data2.scissor,
			 SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR( 0, 64, 0, 32 ) );
	SetGSREG( &packet->init_data.pack2.data2.offset,
			 SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET( 0, 0 ) );
	SetGSREG( &packet->init_data.pack2.data2.texflush,
			 SCE_GS_TEXFLUSH, 0 );
	SetGSREG( &packet->init_data.pack2.data2.clamp,
			 SCE_GS_CLAMP_1, SCE_GS_SET_CLAMP( 3, 0, 0x3f7, 0x08, 0x000, 0x000 ) );
	SetGSREG( &packet->init_data.pack2.data2.tex1,
			 SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 ) );
	SetGSREG( &packet->init_data.pack2.data2.alpha,
			 SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA( 2, 2, 2, 0, 128 ) );
	SetGSREG( &packet->init_data.pack2.data2.rgba,
			 SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ( 128, 128, 128, 128, 0 ) );

	/* パラメータ設定 */
	packet->init_data.pack0.param[0] = DRAW_WIDTH*DRAW_HEIGHT/(64*32) ;	/* 繰り返し回数 */
	packet->init_data.pack0.data0.frame.data |= 0x40000000 ;
	var_mem0.i = packet->init_data.pack0.data0.frame.data & 0xffffffff ;
	var_mem1.i = var_mem0.i + 1 ;
	var_mem1.f = var_mem1.f - var_mem0.f ;
	packet->init_data.pack0.param[1] = var_mem1.i ;	/* frame差分 */
	var_mem0.i = packet->init_data.pack0.data0.tex0.data & 0xffffffff ;
	var_mem1.i = var_mem0.i + 32 ;
	var_mem1.f = var_mem1.f - var_mem0.f ;
	packet->init_data.pack0.param[2] = var_mem1.i ;	/* tex0差分 */
	packet->init_data.pack0.param[3] = 0 ;

	packet->dmatag1 = *( (DG_DMATAG*)Blue2RGBVu0 );
	//packet->dmatag2 = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(struct _gif_packet) );
#endif
}

/* ---------------------------------------------------------------- */
static void InitDmaPacket( ALL_PACKET *packet, int which )
{
	packet->dmatag_wait.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
	packet->dmatag_wait.vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 );
	packet->dmatag_wait.vifcode[1] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag_init.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(packet->init_packet) ) ;
	packet->dmatag_init.vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 );
	packet->dmatag_init.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(packet->init_packet), 0);
	packet->dmatag0.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(packet->gif_packet0) ) ;
	packet->dmatag0.vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 );
	packet->dmatag0.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(packet->gif_packet0), 0);
	packet->dmatag1.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(packet->gif_packet1) ) ;
	packet->dmatag1.vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 );
	packet->dmatag1.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(packet->gif_packet1), 0);
	packet->dmatag2.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(packet->gif_packet2) ) ;
	packet->dmatag2.vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 );
	packet->dmatag2.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(packet->gif_packet2), 0);
	packet->dmatag3.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(packet->gif_packet3) ) ;
	packet->dmatag3.vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 );
	packet->dmatag3.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(packet->gif_packet3), 0);
	packet->dmatag_reset.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, SIZEOF_QWORD(packet->reset_packet) ) ;
	packet->dmatag_reset.vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 ) ;
	packet->dmatag_reset.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(packet->reset_packet), 0) ;
	packet->dmatag_drawenv.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_DRAWENV) ) ;
	packet->dmatag_drawenv.addr = &( DG_Chanl( 0 )->draw_env[ which ] ) ;
	packet->dmatag_drawenv.vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 ) ;
	packet->dmatag_drawenv.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_DRAWENV), 0) ;
	packet->dmatag_offset.qwc = DMATAG_SET_QWC( DMATAG_ID_REF, SIZEOF_QWORD(DG_DRAWOFFSET) ) ;
	packet->dmatag_offset.addr = &( DG_Chanl( 0 )->draw_offset[ which ] ) ;
	packet->dmatag_offset.vifcode[0] = SCE_VIF1_SET_NOP( 0 ) ;
	packet->dmatag_offset.vifcode[1] = SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(DG_DRAWOFFSET), 0) ;
	packet->dmatag_end.qwc = DMATAG_SET_QWC( DMATAG_ID_RET, 0 ) ;
	packet->dmatag_end.vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 ) ;
	packet->dmatag_end.vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
	/* 初期化パケットの初期化 */
	packet->init_packet.init_packet.giftag.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _init_gif_data), 1, 0, 0, 0, 1) ;
	packet->init_packet.init_packet.giftag.regs = GS_REGS_AD ;
	packet->init_packet.init_packet.data.clamp.reg = SCE_GS_CLAMP_1 ;
	packet->init_packet.init_packet.data.clamp.data = SCE_GS_SET_CLAMP(2,2,1,DRAW_WIDTH-1,1,DRAW_HEIGHT-2) ;
	packet->init_packet.init_packet.data.texflush.reg = SCE_GS_TEXFLUSH ;
	packet->init_packet.init_packet.data.texflush.data = 0 ;
	SetGSREG( &packet->init_packet.init_packet.data.texa,
			 SCE_GS_TEXA, SCE_GS_SET_TEXA( 0, 0, 128 ) );
	SetGSREG( &packet->init_packet.init_packet.data.tex1,
			 SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 1, 0, 0, 0, 0, 0, 0 ) );
	SetGSREG( &packet->init_packet.init_packet.data.test,
			 //SCE_GS_TEST_1, SCE_GS_SET_TEST( 0, 1, 64, 1, 0, 0, 1, 1 ) );
			 SCE_GS_TEST_1, SCE_GS_SET_TEST( 1, 0, 64, 1, 0, 0, 1, 1 ) );	/* Ｚテスト無効＆Ｚバッファ更新しない */
	/* 描画環境の復元パケットの初期化 */
	SetGSREG( &packet->reset_packet.data.scissor,
			 SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR( 0, DRAW_WIDTH, 0, DRAW_HEIGHT ) );
	SetGSREG( &packet->reset_packet.data.offset,
			 SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET( 2048*16, 2048*16 ) );
	SetGSREG( &packet->reset_packet.data.tex1,
			 SCE_GS_TEX1_1, SCE_GS_SET_TEX1( 1, 0, 1, 1, 0, 0, 0 ) );
	SetGSREG( &packet->reset_packet.data.test,
			 SCE_GS_TEST_1, SCE_GS_SET_TEST( 1, 7, 0, 0, 0, 0, 1, 1 ) );
	SetGSREG( &packet->reset_packet.data.texflush,
			 SCE_GS_TEXFLUSH, 0 );
	packet->reset_packet.giftag.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(packet->reset_packet.data), 1, 0, 0, 0, 1);
	packet->reset_packet.giftag.regs = GS_REGS_AD ;

}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	ALL_PACKET	*packet ;

	if ( work->mode == 0 && GV_PauseLevel == 0 ){
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
}

/* ---------------------------------------------------------------- */

static void BP_MonoScnCallback(void* pWork)
{
   int i;

   // doesn't use work, so it doesn't need buffered

   SBP_PFX_Mono* pPacket = (SBP_PFX_Mono*)BP_RB_Alloc(sizeof(SBP_PFX_Mono));

   FVECTOR colorWeighting = { 0x26 / 128.0f, 0x4b / 128.0f, 0x0e / 128.0f, 0.0f };
   pPacket->colorWeighting = colorWeighting;

   pPacket->monoType = kMT_Scene;

   for( i = 0; i < 256; ++i )
   {
      unsigned char r = effect_clut[i][0];
      unsigned char g = effect_clut[i][1];
      unsigned char b = effect_clut[i][2];
      unsigned char a = 0x80;

      unsigned int color = r | g << 8 | b << 16 | a << 24;

      pPacket->palette[i] = color;
   }

   BP_RB_AddCommand(kCmd_PostFx_Mono, (char*)pPacket);
}

static int GetResources( Work *work, int phase, int prio, u_int *pal )
{
	DG_DMAPACK	*dmapack ;
	ALL_PACKET	*packet ;
	u_int		col = 0, mask, col2, i, j, x, y ;

	/* ＤＭＡパケット型オブジェクト作成 */
	work->dmapack = dmapack = DG_MakeDmapack2( DG_DMAPACK_NORMAL|
//											 DG_DMAPACK_INVISIBLE0|
											 DG_DMAPACK_INVISIBLE1|
											 DG_DMAPACK_INVISIBLE2|
											 DG_DMAPACK_INVISIBLE3
											 , phase, prio );
	if ( dmapack == NULL ) return ( -1 );

#if BP_DMAPACK_DEBUG_INFO
      dmapack->BP_LabelMask |= kRL_PostFx;
#endif

   DG_AS_DMAPackSetCallbacks(dmapack, BP_MonoScnCallback, NULL);
   dmapack->BP_callbackParam = work;
	DG_QueueDmapack( dmapack );

	work->color = col ;

	/* パケットメモリ割り当て */
	work->alloc_addr = packet = GV_Malloc( sizeof(ALL_PACKET) * 2 );
	if ( packet == NULL ) return ( -1 );
	GV_ZeroMemory( packet, sizeof(ALL_PACKET) * 2 );
	dmapack->packet[0] = &packet[0] ;
	dmapack->packet[1] = &packet[1] ;

	/* 変換用パレットデータの準備 */
	for ( i = 0 ; i < 256 ; i++ ){
		int		index ;
#if BP_VITA
      // VITA palettes are not swizzled like the ps2
      index = i;
#else
		index = ( ( i & 0x08 ) << 1 ) | ( ( i & 0x10 ) >> 1 ) | ( i & 0xe7 ) ;
#endif
#if 0
		green_clut[ index ][0] = 0 ;
		green_clut[ index ][1] = 0 ;
		green_clut[ index ][2] = i ;
#endif
		effect_clut[ index ][0] = pal[i] & 0xff ;
		effect_clut[ index ][1] = (pal[i]>>8) & 0xff ;
		effect_clut[ index ][2] = (pal[i]>>16) & 0xff ;
	}

	for ( i = 0 ; i < 2 ; i++ ){
		ALL_PACKET		*pack ;

		InitDmaPacket( &packet[i], i );
#if 0 //BP_PS2
		DG_MakeLoadImagePacket( &packet[i].load_green_clut,
							   SCE_GS_PSMCT24, 16, 16, BUFFER_PAGE(3), 64, green_clut );
		DG_MakeLoadImagePacket( &packet[i].load_effect_clut,
							   SCE_GS_PSMCT24, 16, 16, BUFFER_PAGE(3)+256, 64, effect_clut );
#endif
		pack = &packet[i] ;

		/* モノクロ化のためのモジュレート */
		SetGifPacket( &pack->gif_packet0.pass0.giftag, SIZEOF_QWORD(pack->gif_packet0.pass0.data) );
		SetGSREG( &pack->gif_packet0.pass0.data.frame,
				 SCE_GS_FRAME_1,
				 SCE_GS_SET_FRAME( BUFFER_PAGE(i)/2048, BUFFER_WIDTH/64, SCE_GS_PSMCT24, 0x00000000 ) );
		SetGSREG( &pack->gif_packet0.pass0.data.scissor,
				 SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR( 0, DRAW_WIDTH, 0, DRAW_HEIGHT ) );
		SetGSREG( &pack->gif_packet0.pass0.data.offset,
				 SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET( 0, 0 ) );
		SetGSREG( &pack->gif_packet0.pass0.data.clamp,
				 SCE_GS_CLAMP_1, SCE_GS_SET_CLAMP( 0, 0, 0, 0, 0, 0 ) );
		SetGSREG( &pack->gif_packet0.pass0.data.tex0,
				 SCE_GS_TEX0_1,
				 SCE_GS_SET_TEX0( BUFFER_PAGE(i)/64, DRAW_WIDTH/64, SCE_GS_PSMCT32, 9, 9, 1, 0,
								 0, 0, 0, 0, 0 ) );
		SetGSREG( &pack->gif_packet0.pass0.data.texflush, SCE_GS_TEXFLUSH, 0 );
		SetGSREG( &pack->gif_packet0.pass0.data.alpha,
				 SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA( 2, 2, 2, 0, 128 ) );
		SetGSREG( &pack->gif_packet0.pass0.data.prim,
				 SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );
		SetGSREG( &pack->gif_packet0.pass0.data.rgbq,
				 SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ( 0x26, 0x4b, 0x0e, 128, 0 ) );
		for ( j = 0, x = 0 ; x < DRAW_WIDTH ; x += 64, j++ ){
			SetGSREG( &pack->gif_packet0.pass0.data.sprt[ j ].uv0,
					 SCE_GS_UV, SCE_GS_SET_UV( (x+0)*16+8, (0)*16+8 ) );
			SetGSREG( &pack->gif_packet0.pass0.data.sprt[ j ].uv1,
					 SCE_GS_UV, SCE_GS_SET_UV( (x+64)*16+8, (DRAW_HEIGHT)*16+8 ) );
			SetGSREG( &pack->gif_packet0.pass0.data.sprt[ j ].xyz0,
					 SCE_GS_XYZ2, SCE_GS_SET_XYZ( (x+0)*16, (0)*16, DRAW_Z_MAX ) );
			SetGSREG( &pack->gif_packet0.pass0.data.sprt[ j ].xyz1,
					 SCE_GS_XYZ2, SCE_GS_SET_XYZ( (x+64)*16, (DRAW_HEIGHT)*16, DRAW_Z_MAX ) );
		}
		//SetFrameMaskPacket( &packet[i].gif_packet.frame0, i, 0x00000000 );
		//SetScreenDrawPacket( &packet[i].gif_packet.screen_draw0,
		//					0x01,
		//					SCE_GS_SET_TEST( 0, 1, 64, 1, 0, 0, 1, 1 ),
		//					SCE_GS_SET_ALPHA(2,2,2,0,128),
		//					i,
		//					0x000e4b26 );

		/* 表のR,Gチャンネルを裏のB,Aチャンネルへ移動 */
		SetGifPacket( &pack->gif_packet1.pass1.giftag, SIZEOF_QWORD(pack->gif_packet1.pass1.data) );
		SetGSREG( &pack->gif_packet1.pass1.data.frame,
				 SCE_GS_FRAME_1,
				 SCE_GS_SET_FRAME( BUFFER_PAGE(2)/2048, BUFFER_WIDTH/64, SCE_GS_PSMCT16, 0 ) );
		SetGSREG( &pack->gif_packet1.pass1.data.scissor,
				 SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR( 0, DRAW_WIDTH, 0, DRAW_HEIGHT*2 ) );
		SetGSREG( &pack->gif_packet1.pass1.data.offset,
				 SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET( 0, 0 ) );
		SetGSREG( &pack->gif_packet1.pass1.data.clamp,
				 SCE_GS_CLAMP_1, SCE_GS_SET_CLAMP( 0, 0, 0, 0, 0, 0 ) );
		SetGSREG( &pack->gif_packet1.pass1.data.tex0,
				 SCE_GS_TEX0_1,
				 SCE_GS_SET_TEX0( BUFFER_PAGE(i)/64, DRAW_WIDTH/64, SCE_GS_PSMCT16, 9, 10, 1, 1,
								 0, 0, 0, 0, 0 ) );
		SetGSREG( &pack->gif_packet1.pass1.data.texflush, SCE_GS_TEXFLUSH, 0 );
		SetGSREG( &pack->gif_packet1.pass1.data.alpha,
				 SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
		SetGSREG( &pack->gif_packet1.pass1.data.prim,
				 SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0/*1*/,0,1,0,0) );/* no alpha */
		SetGSREG( &pack->gif_packet1.pass1.data.rgbq,
				 SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ( 128, 128, 128, 128, 0 ) );
		x = 0 ;
		y = 0 ;
		for ( j = 0 ; j < DRAW_WIDTH ; j += 16 ){
			SetGSREG( &pack->gif_packet1.pass1.data.sprt[ j / 16 ].uv0,
					 SCE_GS_UV, SCE_GS_SET_UV( (j)*16+8, (0)*16+8 ) );
			SetGSREG( &pack->gif_packet1.pass1.data.sprt[ j / 16 ].uv1,
					 SCE_GS_UV, SCE_GS_SET_UV( (j+8)*16+8, (DRAW_HEIGHT*2)*16+8 ) );
			SetGSREG( &pack->gif_packet1.pass1.data.sprt[ j / 16 ].xyz0,
					 SCE_GS_XYZ2, SCE_GS_SET_XYZ( x+(8)*16, y+(0)*16, DRAW_Z_MAX ) );
			SetGSREG( &pack->gif_packet1.pass1.data.sprt[ j / 16 ].xyz1,
					 SCE_GS_XYZ2, SCE_GS_SET_XYZ( x+(16)*16, y+(DRAW_HEIGHT*2)*16, DRAW_Z_MAX ) );
			x += (16)*16 ;
		}

		/* 表のB,Aチャンネルを裏のB,Aチャンネルへ加算 */
		SetGifPacket( &pack->gif_packet2.pass2.giftag, SIZEOF_QWORD(pack->gif_packet2.pass2.data) );
		SetGSREG( &pack->gif_packet2.pass2.data.frame,
				 SCE_GS_FRAME_1,
				 SCE_GS_SET_FRAME( BUFFER_PAGE(2)/2048, BUFFER_WIDTH/64, SCE_GS_PSMCT24, 0x00000000 ) );
		SetGSREG( &pack->gif_packet2.pass2.data.scissor,
				 SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR( 0, DRAW_WIDTH, 0, DRAW_HEIGHT ) );
		SetGSREG( &pack->gif_packet2.pass2.data.offset,
				 SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET( 0, 0 ) );
		SetGSREG( &pack->gif_packet2.pass2.data.clamp,
				 SCE_GS_CLAMP_1, SCE_GS_SET_CLAMP( 0, 0, 0, 0, 0, 0 ) );
		SetGSREG( &pack->gif_packet2.pass2.data.tex0,
				 SCE_GS_TEX0_1,
				 SCE_GS_SET_TEX0( BUFFER_PAGE(i)/64, DRAW_WIDTH/64, SCE_GS_PSMCT32, 9, 9, 1, 0,
								 0, 0, 0, 0, 0 ) );
		SetGSREG( &pack->gif_packet2.pass2.data.texflush, SCE_GS_TEXFLUSH, 0 );
		SetGSREG( &pack->gif_packet2.pass2.data.alpha,
				 SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA( 0, 2, 2, 1, 128 ) );
		SetGSREG( &pack->gif_packet2.pass2.data.prim,
				 SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );
		SetGSREG( &pack->gif_packet2.pass2.data.rgbq,
				 SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ( 0, 0, 128, 0, 0 ) );
		x = 0 ;
		y = 0 ;
		for ( j = 0 ; j < DRAW_WIDTH ; j += 64 ){
			SetGSREG( &pack->gif_packet2.pass2.data.sprt[ j / 64 ].uv0,
					 SCE_GS_UV, SCE_GS_SET_UV( (j)*16+8, (0)*16+8 ) );
			SetGSREG( &pack->gif_packet2.pass2.data.sprt[ j / 64 ].uv1,
					 SCE_GS_UV, SCE_GS_SET_UV( (j+64)*16+8, (DRAW_HEIGHT)*16+8 ) );
			SetGSREG( &pack->gif_packet2.pass2.data.sprt[ j / 64 ].xyz0,
					 SCE_GS_XYZ2, SCE_GS_SET_XYZ( x+(0)*16, y+(0)*16, DRAW_Z_MAX ) );
			SetGSREG( &pack->gif_packet2.pass2.data.sprt[ j / 64 ].xyz1,
					 SCE_GS_XYZ2, SCE_GS_SET_XYZ( x+(64)*16, y+(DRAW_HEIGHT)*16, DRAW_Z_MAX ) );
			x += (64)*16 ;
		}

		/* 裏のA(G)チャンネルをパレットテクスチャとして裏のBチャンネルへ加算 */
		SetGifPacket( &pack->gif_packet3.pass3.giftag, SIZEOF_QWORD(pack->gif_packet3.pass3.data) );
		SetGSREG( &pack->gif_packet3.pass3.data.frame,
				 SCE_GS_FRAME_1,
				 SCE_GS_SET_FRAME( BUFFER_PAGE(2)/2048, BUFFER_WIDTH/64, SCE_GS_PSMCT24, 0x00000000 ) );
		SetGSREG( &pack->gif_packet3.pass3.data.scissor,
				 SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR( 0, DRAW_WIDTH, 0, DRAW_HEIGHT ) );
		SetGSREG( &pack->gif_packet3.pass3.data.offset,
				 SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET( 0, 0 ) );
		SetGSREG( &pack->gif_packet3.pass3.data.clamp,
				 SCE_GS_CLAMP_1, SCE_GS_SET_CLAMP( 0, 0, 0, 0, 0, 0 ) );
		SetGSREG( &pack->gif_packet3.pass3.data.tex0,
				 SCE_GS_TEX0_1,
				 SCE_GS_SET_TEX0( BUFFER_PAGE(2)/64, DRAW_WIDTH/64, SCE_GS_PSMT8H, 9, 9, 0, 0,
								 BUFFER_PAGE(3)/64, 1, SCE_GS_PSMCT32, 0, 1 ) );
		SetGSREG( &pack->gif_packet3.pass3.data.texflush, SCE_GS_TEXFLUSH, 0 );
		SetGSREG( &pack->gif_packet3.pass3.data.alpha,
				 SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA( 0, 2, 2, 1, 128 ) );
		SetGSREG( &pack->gif_packet3.pass3.data.prim,
				 SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,1,0,1,0,0) );
		SetGSREG( &pack->gif_packet3.pass3.data.rgbq,
				 SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ( 128, 128, 128, 0, 0 ) );
		x = 0 ;
		y = 0 ;
		for ( j = 0 ; j < DRAW_WIDTH ; j += 64 ){
			SetGSREG( &pack->gif_packet3.pass3.data.sprt[ j / 64 ].uv0,
					 SCE_GS_UV, SCE_GS_SET_UV( (j)*16+8, (0)*16+8 ) );
			SetGSREG( &pack->gif_packet3.pass3.data.sprt[ j / 64 ].uv1,
					 SCE_GS_UV, SCE_GS_SET_UV( (j+64)*16+8, (DRAW_HEIGHT)*16+8 ) );
			SetGSREG( &pack->gif_packet3.pass3.data.sprt[ j / 64 ].xyz0,
					 SCE_GS_XYZ2, SCE_GS_SET_XYZ( x+(0)*16, y+(0)*16, DRAW_Z_MAX ) );
			SetGSREG( &pack->gif_packet3.pass3.data.sprt[ j / 64 ].xyz1,
					 SCE_GS_XYZ2, SCE_GS_SET_XYZ( x+(64)*16, y+(DRAW_HEIGHT)*16, DRAW_Z_MAX ) );
			x += (64)*16 ;
		}

#if 0
		/* 裏のBチャンネルを８ビットパレットとして表へ転送 */
		SetGifPacket( &gif_packet->pass4.giftag, SIZEOF_QWORD(gif_packet->pass4.data) );
		SetGSREG( &gif_packet->pass4.data.frame,
				 SCE_GS_FRAME_1,
				 SCE_GS_SET_FRAME( BUFFER_PAGE(i)/2048, BUFFER_WIDTH/64, SCE_GS_PSMCT32, 0x00000000 ) );
		SetGSREG( &gif_packet->pass4.data.scissor,
				 SCE_GS_SCISSOR_1, SCE_GS_SET_SCISSOR( 0, DRAW_WIDTH, 0, DRAW_HEIGHT ) );
		SetGSREG( &gif_packet->pass4.data.offset,
				 SCE_GS_XYOFFSET_1, SCE_GS_SET_XYOFFSET( 0, 0 ) );
		SetGSREG( &gif_packet->pass4.data.clamp,
				 SCE_GS_CLAMP_1, SCE_GS_SET_CLAMP( 3, 0, 0x3f7, 0x08, 0x000, 0x000 ) );
		SetGSREG( &gif_packet->pass4.data.tex0,
				 SCE_GS_TEX0_1,
				 SCE_GS_SET_TEX0( BUFFER_PAGE(2)/64, DRAW_WIDTH*2/64, SCE_GS_PSMT8, 10, 10, 0, 0,
								 (BUFFER_PAGE(3)+256)/64, 1, SCE_GS_PSMCT32, 0, 1 ) );
		SetGSREG( &gif_packet->pass4.data.texflush, SCE_GS_TEXFLUSH, 0 );
		SetGSREG( &packet[i].gif_packet.pass4.data.alpha,
				 SCE_GS_ALPHA_1, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 128 ) );
		SetGSREG( &gif_packet->pass4.data.prim,
				 SCE_GS_PRIM, SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,1,0,0,0,1,0,0) );
		SetGSREG( &gif_packet->pass4.data.rgbq,
				 SCE_GS_RGBAQ, SCE_GS_SET_RGBAQ( 128, 128, 128, 128, 0 ) );

		gif_packet->pass4.giftag2.tag = SCE_GIF_SET_TAG( DRAW_WIDTH*DRAW_HEIGHT/8/2, 1, 0, 0, 1, 4);
		gif_packet->pass4.giftag2.regs = 0x5353;/* UV,XYZ,UV,XYZ */
		for ( j = 0, y = 0 ; y < DRAW_HEIGHT ; y += 2 ){
			for ( x = 0 ; x < DRAW_WIDTH ; x += 8, j++ ){
				int		u0, v0, u1, v1 ;
				u0 = x * 2 + 8 ; if ( y & 2 ) u0 -= 4 ;
				u1 = u0 + 8 ;
				v0 = y * 2 + 0 ;
				v1 = y * 2 + 2 ;
				gif_packet->pass4.data2.sprt[j].uv0 = SCE_GS_SET_UV( u0*16+8, v0*16+8 );
				gif_packet->pass4.data2.sprt[j].uv1 = SCE_GS_SET_UV( u1*16+8, v1*16+8 );
				gif_packet->pass4.data2.sprt[j].xyz0 = SCE_GS_SET_XYZ( (x)*16, (y)*16, DRAW_Z_MAX );
				gif_packet->pass4.data2.sprt[j].xyz1 = SCE_GS_SET_XYZ( (x+8)*16, (y+2)*16, DRAW_Z_MAX );
			}
		}
#endif
		InitVu1SetupPacket( &packet[i].vu1_setup, i );




	}


	return (0);
}


/* ---------------------------------------------------------------- */
	/*
		画面のモノクロ化
	*/
void *NewMonochrome( void )
{
	Work		*work ;
	u_int		pal[256], i ;

	for ( i = 0 ; i < 256 ; i++ ) pal[i] = (i)|(i<<8)|(i<<16);

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, DG_DMAPACK_PHASE_AFTER, DMAPACK_PRIO, pal ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewMonochromaticEffect( int phase, int prio, u_int *pal )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, phase, prio, pal ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
