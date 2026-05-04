/*
	packet.h
	packet.c用パケット構造体定義ヘッダ

	2001/07/21	K.Takabe

*/


#ifndef __PACKET_H__
#define __PACKET_H__

#define INIT_PACKET_GIFTAG( _p )	{ (_p).giftag.regs = 0xe ; (_p).giftag.tag = SCE_GIF_SET_TAG(SIZEOF_QWORD(_p)-1, 1, 0, 0, 0, 1); }

enum {
	DG_PACKET_FLAG_CONTEXT2		= 0x0001,	/* コンテキスト２として初期化（default:コンテキスト１） */
	DG_PACKET_FLAG_COLDEPTH16	= 0x0002,	/* １６ビットフレームバッファ使用（default:３２ビット） */
	DG_PACKET_FLAG_ZDEPTH16		= 0x0004,	/* １６ビットＺバッファ使用（default:２４ビット） */
	DG_PACKET_FLAG_NODEPTH		= 0x0008,	/* デプステストを行わない（default:行う） */
	DG_PACKET_FLAG_TEXDEPTH16	= 0x0010,	/* １６ビットテクスチャ使用（default:３２ビット） */
	DG_PACKET_FLAG_USETEXALPHA	= 0x0020,	/* テクスチャのアルファ使用（default:TEXAを使用） */
	DG_PACKET_FLAG_USETEXTURE	= 0x0040,	/* テクスチャマップ有り */
	DG_PACKET_FLAG_ALPHABLEND	= 0x0080,	/* アルファブレンド有り */
	DG_PACKET_FLAG_ADDBLEND		= 0x0100,	/* 加算半透明有り（DG_PACKET_FLAG_ALPHABLENDも必要） */
	DG_PACKET_FLAG_SUBBLEND		= 0x0200,	/* 減算半透明有り（DG_PACKET_FLAG_ALPHABLENDも必要） */
};

/* フレームバッファ設定パケット */
typedef struct _dg_packet_frame_buffer{
	DG_GSREG		frame ;
	DG_GSREG		zbuf ;
	DG_GSREG		xyoffset ;
	DG_GSREG		scissor ;
} DG_PACKET_FRAME_BUFFER ;

/* テクスチャ設定パケット */
typedef struct _dg_packet_texture {
	DG_GSREG		tex0 ;
	DG_GSREG		clamp ;
} DG_PACKET_TEXTURE ;

/* スプライト描画開始パケット */
typedef struct _dg_packet_draw2d{
	DG_GSREG		alpha ;
	DG_GSREG		test ;
	DG_GSREG		prim ;
	DG_GSREG		prmode ;
	DG_GSREG		rgbaq ;
} DG_PACKET_DRAW2D ;

/* ＵＶと頂点パケット */
typedef struct _dg_packet_verts{
	DG_GSREG		uv ;
	DG_GSREG		xyzf2 ;
} DG_PACKET_VERTS ;

/* ---------------------------------------------------------------- */
/* メインフレームバッファからテクスチャ領域への転送準備パケット */
typedef struct _dg_packet_drawscreen {
	DG_GIFTAG					giftag ;
	DG_PACKET_FRAME_BUFFER		frame ;
	DG_PACKET_TEXTURE			texture ;
	DG_PACKET_DRAW2D			draw2d ;
	DG_PACKET_VERTS				verts[8][2] ;
} DG_PACKET_DRAWSCREEN ;


#endif

