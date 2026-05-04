/*
    dma_set.h
	DMAやらGIFやら

	2000/07/11 T.Shibata

	$Id: dma_set.h,v 1.1.1.3 2002/11/19 11:48:53 Yoshizawa1 Exp $
	
*/
#include "dmapack.h"
#ifndef _DMA_SET_H
#define _DMA_SET_H

///////////////////////////////////////////////////
//
// DMAPACKET 構造体
//

//頂点
typedef struct _spr_vart_data {
	DG_GSREG	st;
	DG_GSREG	xyz;
}SVERT_DATA;

//モデル頂点
typedef struct _mdl_vart_data {
	DG_GSREG	rgbq;
	DG_GSREG	uv;
	DG_GSREG	xyz;
}MVERT_DATA;

//テクスチャー無し頂点
typedef struct _not_vart_data {
	DG_GSREG	rgbq;
	DG_GSREG	xyz;
}NOTVERT_DATA;

//モデル頂点
typedef struct _mdl_vart_data_f {
	DG_GSREG	uv;
	DG_GSREG	xyz;
}MVERT_DATA_F;

//テクスチャー無しライン頂点
typedef struct _line_vart_data {
	DG_GSREG	rgbq0;
	DG_GSREG	xyz0;
	DG_GSREG	rgbq1;
	DG_GSREG	xyz1;
}LINE_VERT_DATA;

//四角ポリゴン頂点
typedef struct _poly_vart_data {
	DG_GSREG	rgbq0;
	DG_GSREG	uv0;
	DG_GSREG	xyz0;
	DG_GSREG	rgbq1;
	DG_GSREG	uv1;
	DG_GSREG	xyz1;
	DG_GSREG	rgbq2;
	DG_GSREG	uv2;
	DG_GSREG	xyz2;
	DG_GSREG	rgbq3;
	DG_GSREG	uv3;
	DG_GSREG	xyz3;
}POLY_VERT_DATA;

typedef struct _drow_flush {
	DG_GIFTAG		giftag ;
	struct _drow_flush_data{
		DG_GSREG	texflush0;
	} data;
} DROW_FLUSH;

//フレーム退避
typedef struct _tr_buffer {
	DG_GIFTAG		giftag ;
	struct _tr_buffer_data{
		DG_GSREG	texflush0;
		DG_GSREG	bitbltbuf;
		DG_GSREG	trxpos;
		DG_GSREG	trxreg;
		DG_GSREG	trxdir;
		DG_GSREG	texflush1;
	} data ;
} TR_BUFFER;

//フレームクリアー
typedef struct _frame_clear{
	DG_GIFTAG		giftag ;
	struct _frame_clear_data{
		DG_GSREG	test;
		DG_GSREG	alpha;
		DG_GSREG	rgbq;
		DG_GSREG	prim;
		DG_GSREG	xyz0;
		DG_GSREG	xyz1;
	} data;
}FRAME_CLEAR;

//モデル描画の設定
typedef struct {
	DG_GIFTAG		giftag ;
	struct _mdl_data{
		DG_GSREG			test;
		DG_GSREG			clamp;
		DG_GSREG			tex0;
		DG_GSREG			alpha;
		DG_GSREG			rgbq;
		DG_GSREG			prim;
	} data;
} MDL_DRAW;

//テクスチャーの設定
typedef struct {
	DG_GIFTAG		giftag ;
	struct _tex_draw_data{
		DG_GSREG	texflush;
		DG_GSREG	clamp;
		DG_GSREG	test;
		DG_GSREG	tex2;
		DG_GSREG	tex0;
		DG_GSREG	alpha;
		DG_GSREG	rgbq;
		DG_GSREG	prim;
	} data;
} TEX_DRAW;

//ピクセルテスト復元
typedef struct {
	DG_GIFTAG		giftag ;
	struct _packet_end_data {
		DG_GSREG	test;
	} data;
} PACKET_END;

//extern DG_DMAPACK* DG_MakeDmapack( int flag, int phase );
//extern void DG_FreeDmapack( DG_DMAPACK *dmapack );
extern DG_TEX_PACKET* DG_GetTexturePacket( unsigned int code );
extern int DG_WriteTextureChangePacks( void *tag_addr, void *tex_packet );
extern DG_TEX* DG_GetTexture2( unsigned int tri_code, unsigned int code );
extern DG_TEXTURE_LIST* DG_GetTextureList( unsigned int code );

extern int GetMdlVerts(DG_DEF *def);		// モデルの頂点数
extern int GetMdlVerts_CV2(CV2_DEF *def);		// モデルの頂点数
extern int GetMdlVertsJoint( DG_DEF *def, int joint );


#if 0 //BP_PS2
extern void InitDrowFlush( DROW_FLUSH *drow_frush );
extern void InitTrBuffer( TR_BUFFER *tr_buffer, int which );		//画面退避
extern void InitFrameClear( FRAME_CLEAR *frame_clear,
							u_long64 test, u_long64 rgba, u_long64 alpha, long64 z );		//フレームクリアー
extern void InitMdlDraw(MDL_DRAW *mdl_draw,
						u_long64 test, u_long64 alpha, u_long64 prim );//退避したフレームバッファーをテクスチャーとしたモデル描画の設定
#else
extern void *InitDrowFlush( void *packet );
extern void *InitTrBuffer( void *packet, int which );		//画面退避
extern void *InitFrameClear( void *packet,
							u_long64 test, u_long64 rgba, u_long64 alpha, long64 z );		//フレームクリアー
extern void *InitMdlDraw( void *packet,
						u_long64 test, u_long64 alpha, u_long64 prim );//退避したフレームバッファーをテクスチャーとしたモデル描画の設定
#endif

extern void InitSetTexDraw( TEX_DRAW *tex_draw, u_long64 rgba, u_long64 prim );	//テクスチャー初期化
extern void InitEndPacket( PACKET_END *packet );//ピクセルテスト復元
extern void TexDataSet( void *dst, DG_TEX_PACKET *src,
						TEX_DRAW *tex_draw, DG_TEX_TRANS *tex_trans,
						u_long64 alpha );//テクスチャー転送パケット設定

#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))

#ifdef PSX2

#define MAX_U		((DRAW_WIDTH<<4)-(1<<3))
#define MAX_V		((DRAW_HEIGHT<<4)-(1<<3))
#define MIN_UV		(1<<3)
#define	MAX_X		((2048+DRAW_WIDTH/2)<<4)
#define	MAX_Y		((2048+DRAW_HEIGHT/2)<<4)
#define	MIN_X		((2048-DRAW_WIDTH/2)<<4)
#define	MIN_Y		((2048-DRAW_HEIGHT/2)<<4)

#define DRAW_WIDTH_HALF		((float)(DRAW_WIDTH>>1))
#define DRAW_HEIGHT_HALF	((float)(DRAW_HEIGHT>>1))

#define SHIBATA_FSHIFT			(16.0f)

#else

#define MAX_U		(1.0f)
#define MAX_V		(1.0f)
#define MIN_UV		(0.0f)
#define	MAX_X		(DRAW_WIDTH )
#define	MAX_Y		(DRAW_HEIGHT)
#define	MIN_X		0
#define	MIN_Y		0

#define DRAW_WIDTH_HALF		((float)(DRAW_WIDTH>>1))
#define DRAW_HEIGHT_HALF	((float)(DRAW_HEIGHT>>1))

#define SHIBATA_FSHIFT			(1.0f)

#endif /*PSX2*/

#endif
