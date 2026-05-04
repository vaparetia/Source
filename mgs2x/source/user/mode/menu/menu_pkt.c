//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	menu_sub.c
	メニュー用スタティックＰＳ２パケット定義ファイル

	2000/10/18	K.Takabe
	$Id: menu_pkt.c,v 1.1.1.3 2002/11/19 11:45:13 Yoshizawa1 Exp $
*/

/* ---------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"def_dma.h"
#include	"libdg.cnf"
#include	"dmapack.h"
#include "menu.h"

#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))

/* ---------------------------------------------------------------- */
#define POSX(_x)	( (_x)*DRAW_WIDTH*16/VR_WIDTH )
#define POSY(_y)	( (_y)*DRAW_HEIGHT*16/VR_HEIGHT )


/* ---------------------------------------------------------------- */
typedef struct _normal_gifpackt {
	DG_DMATAG	dmatag ;
	DG_GIFTAG	giftag ;
	DG_GSREG	datas[0] ;
} NORMAL_GIFPACKT ;

static DG_GSREG		Menu_Message_Window_Packet[] = {
	/* メッセージ背景の正方形プリミティブ（内側） */
	{SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 0, 0, 1, 0, 1, 0, 0), SCE_GS_PRIM},
	{SCE_GS_SET_RGBAQ( 0x08, 0x08, 0x10, 0x40, 0 ), SCE_GS_RGBAQ},
	{SCE_GS_SET_XYZF2( POSX( 0+EXP_WIN_EDGE_X ), POSY( 0+EXP_WIN_EDGE_Y ), 0, 0), SCE_GS_XYZF2},
	{SCE_GS_SET_XYZF2( POSX( EXP_WIN_W-EXP_WIN_EDGE_X ), POSY( EXP_WIN_H-EXP_WIN_EDGE_Y ), 0, 0), SCE_GS_XYZF2},
	/* メッセージ背景の正方形プリミティブ（外枠部分） */
	{SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 0, 0, 1, 0, 1, 0, 0), SCE_GS_PRIM},
	{SCE_GS_SET_RGBAQ( 0x01, 0x01, 0x01, 0x80, 0 ), SCE_GS_RGBAQ},
	{SCE_GS_SET_XYZF2( POSX( 0 ), POSY( 0 ), 0, 0), SCE_GS_XYZF2},
	{SCE_GS_SET_XYZF2( POSX( EXP_WIN_W ), POSY( 0+EXP_WIN_EDGE_Y ), 0, 0), SCE_GS_XYZF2},
	{SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 0, 0, 1, 0, 1, 0, 0), SCE_GS_PRIM},
	{SCE_GS_SET_RGBAQ( 0x01, 0x01, 0x01, 0x80, 0 ), SCE_GS_RGBAQ},
	{SCE_GS_SET_XYZF2( POSX( 0 ), POSY( EXP_WIN_H-EXP_WIN_EDGE_Y ), 0, 0), SCE_GS_XYZF2},
	{SCE_GS_SET_XYZF2( POSX( EXP_WIN_W ), POSY( EXP_WIN_H ), 0, 0), SCE_GS_XYZF2},
	{SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 0, 0, 1, 0, 1, 0, 0), SCE_GS_PRIM},
	{SCE_GS_SET_RGBAQ( 0x01, 0x01, 0x01, 0x80, 0 ), SCE_GS_RGBAQ},
	{SCE_GS_SET_XYZF2( POSX( 0 ), POSY( 0 ), 0, 0), SCE_GS_XYZF2},
	{SCE_GS_SET_XYZF2( POSX( 0+EXP_WIN_EDGE_X ), POSY( EXP_WIN_H ), 0, 0), SCE_GS_XYZF2},
	{SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE, 0, 0, 0, 1, 0, 1, 0, 0), SCE_GS_PRIM},
	{SCE_GS_SET_RGBAQ( 0x01, 0x01, 0x01, 0x80, 0 ), SCE_GS_RGBAQ},
	{SCE_GS_SET_XYZF2( POSX( EXP_WIN_W-EXP_WIN_EDGE_X ), POSY( 0 ), 0, 0), SCE_GS_XYZF2},
	{SCE_GS_SET_XYZF2( POSX( EXP_WIN_W ), POSY( EXP_WIN_H ), 0, 0), SCE_GS_XYZF2},
	/* 下側のタグ形プリミティブ */
	{SCE_GS_SET_PRIM(SCE_GS_PRIM_TRISTRIP, 0, 0, 0, 1, 0, 1, 0, 0), SCE_GS_PRIM},
	{SCE_GS_SET_XYZF2( POSX( 0 ), POSY( EXP_WIN_H ), 0, 0), SCE_GS_XYZF2},
	{SCE_GS_SET_XYZF2( POSX( 0 ), POSY( EXP_WIN_H+EXP_TAG_H ), 0, 0), SCE_GS_XYZF2},
	{SCE_GS_SET_XYZF2( POSX( 0+EXP_TAG_W2 ), POSY( EXP_WIN_H ), 0, 0), SCE_GS_XYZF2},
	{SCE_GS_SET_XYZF2( POSX( 0+EXP_TAG_W1 ), POSY( EXP_WIN_H+EXP_TAG_H ), 0, 0), SCE_GS_XYZF2},
	
};


/* 指定したバッファ領域に指定オフセットずらしたパケットを生成する */
int MENU_SetExplainWindowPacket( void *buffer, int x, int y )
{
#if 0
	int			size, i ;
	u_long64		xy ;
	DG_GSREG	*gsreg ;
	NORMAL_GIFPACKT	*packet ;
	u_long128	*tmp_buffer = SCRPAD_ADDR ;

	size = SIZEOF_QWORD(Menu_Message_Window_Packet) + 2 ;


	/* データを一度スクラッチパッドへ転送する */
	DG_StartMemToSpr( tmp_buffer+2, Menu_Message_Window_Packet, size - 2 );

	/* パケットヘッダを生成 */
	packet = tmp_buffer ;
	packet->dmatag.qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, size - 1 );
	packet->dmatag.vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	packet->dmatag.vifcode[1] = SCE_VIF1_SET_DIRECT( size - 1, 0 );
	packet->giftag.tag = SCE_GIF_SET_TAG( size - 2, 1, 0, 0, 0, 1 );
	packet->giftag.regs = 0x000000000000000e ;

	/* ＤＭＡ転送終了待ち */
	DG_EndMemToSpr();

	/* パケットデータを検索してオフセットを加える */
	x = POSX( x ) + ( 2048 - DRAW_WIDTH / 2 ) * 16 ;
	y = POSY( y ) + ( 2048 - DRAW_HEIGHT / 2 ) * 16 ;
	xy = SCE_GS_SET_XYZF2( x, y, 0, 0 );
	gsreg = packet->datas ;
	for ( i = 0 ; i < size - 2 ; gsreg++, i++ ){
		if ( gsreg->reg == SCE_GS_XYZF2 ) gsreg->data += xy ;
	}

	/* スクラッチパッド上のデータをメインメモリに転送する */
	FlushCache( 0 );
	DG_StartSprToMem( buffer, tmp_buffer, size );
	DG_EndSprToMem();

	return ( size );
#else
	int			size, i ;
	u_long64		xy ;
	DG_GSREG	*gsreg ;
	DG_GIFTAG	*giftag ;
	u_long128	*tmp_buffer = SCRPAD_ADDR ;

	size = SIZEOF_QWORD(Menu_Message_Window_Packet) + 1 ;

	/* MENU_Printf系用パケットからパケットメモリを確保（文字表示との優先問題解決のため） */
	buffer = MENU_PutPacket( size * sizeof(u_long128) );

	/* データを一度スクラッチパッドへ転送する */
	DG_StartMemToSpr( tmp_buffer+1, Menu_Message_Window_Packet, size - 1 );

	/* パケットヘッダを生成 */
	giftag = (DG_GIFTAG*)tmp_buffer ;
	giftag->tag = SCE_GIF_SET_TAG( size - 1, 1, 0, 0, 0, 1 );
	giftag->regs = 0x000000000000000e ;

	/* ＤＭＡ転送終了待ち */
	DG_EndMemToSpr();

	/* パケットデータを検索してオフセットを加える */
	x = POSX( x ) + ( 2048 - DRAW_WIDTH / 2 ) * 16 ;
	y = POSY( y ) + ( 2048 - DRAW_HEIGHT / 2 ) * 16 ;
	xy = SCE_GS_SET_XYZF2( x, y, 0, 0 );
	gsreg = (DG_GSREG*)( giftag + 1 ) ;
	for ( i = 0 ; i < size - 1 ; gsreg++, i++ ){
		if ( gsreg->reg == SCE_GS_XYZF2 ) gsreg->data += xy ;
	}

	/* スクラッチパッド上のデータをメインメモリに転送する */
	FlushCache( 0 );
	DG_StartSprToMem( buffer, tmp_buffer, size );
	DG_EndSprToMem();

	return ( 0 );
#endif
}



