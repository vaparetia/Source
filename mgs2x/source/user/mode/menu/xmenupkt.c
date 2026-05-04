//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xmenupkt.c
	メニュー用スタティックＰＳ２パケット定義ファイル

	2002/03/22	K.Takabe
	$Id: xmenupkt.c,v 1.1.1.3 2002/11/19 11:45:14 Yoshizawa1 Exp $
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
#define POSX(_x)	( (float)(_x)*((float)DRAW_WIDTH/VR_WIDTH) )
#define POSY(_y)	( (float)(_y)*((float)DRAW_HEIGHT/VR_HEIGHT) )


/* ---------------------------------------------------------------- */

/* 指定したバッファ領域に指定オフセットずらしたパケットを生成する */
int MENU_SetExplainWindowPacket( void *buffer, int x, int y )
{
	int			size, i ;
	void		*prim ;

	prim = MENU_OpenPrim();	/* 詳細はgame/xmenuprm.cを参照・・・ */

	/* ＸＢＯＸ版は面倒だったのでベタでパケット生成 */
	prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );

	/* メッセージ背景の正方形プリミティブ（内側） */
	prim = DG_SetDmapackBox( prim,
							POSX( 0+EXP_WIN_EDGE_X + x ), POSY( 0+EXP_WIN_EDGE_Y + y ),
							POSX( EXP_WIN_W-EXP_WIN_EDGE_X + x ), POSY( EXP_WIN_H-EXP_WIN_EDGE_Y + y ),
							DG_MakeDmaPackColorFromInt(0x40100808) );
	/* メッセージ背景の正方形プリミティブ（外枠部分） */
	prim = DG_SetDmapackBox( prim,
							POSX( 0 + x ), POSY( 0 + y ),
							POSX( EXP_WIN_W + x ), POSY( 0+EXP_WIN_EDGE_Y + y ),
							DG_MakeDmaPackColorFromInt(0x80010101) );
	prim = DG_SetDmapackBox( prim,
							POSX( 0 + x ), POSY( EXP_WIN_H-EXP_WIN_EDGE_Y + y ),
							POSX( EXP_WIN_W + x ), POSY( EXP_WIN_H + y ),
							DG_MakeDmaPackColorFromInt(0x80010101) );
	prim = DG_SetDmapackBox( prim,
							POSX( 0 + x ), POSY( 0 + y ),
							POSX( 0+EXP_WIN_EDGE_X + x ), POSY( EXP_WIN_H + y ),
							DG_MakeDmaPackColorFromInt(0x80010101) );
	prim = DG_SetDmapackBox( prim,
							POSX( EXP_WIN_W-EXP_WIN_EDGE_X + x ), POSY( 0 + y ),
							POSX( EXP_WIN_W + x ), POSY( EXP_WIN_H + y ),
							DG_MakeDmaPackColorFromInt(0x80010101) );
	/* 下側のタグ形プリミティブ */
	prim = DG_SetDmapackQuad( prim,
							 POSX( 0 + x ), POSY( EXP_WIN_H + y ), DG_MakeDmaPackColorFromInt(0x80010101),
							 POSX( 0 + x ), POSY( EXP_WIN_H+EXP_TAG_H + y ), DG_MakeDmaPackColorFromInt(0x80010101),
							 POSX( 0+EXP_TAG_W1 + x ), POSY( EXP_WIN_H+EXP_TAG_H + y ), DG_MakeDmaPackColorFromInt(0x80010101),
							 POSX( 0+EXP_TAG_W2 + x ), POSY( EXP_WIN_H + y ), DG_MakeDmaPackColorFromInt(0x80010101) );

	MENU_ClosePrim( prim );	/* 詳細はgame/xmenuprm.cを参照・・・ */

	return ( 0 );
}
