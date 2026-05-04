/*
	mcchkscr.h
		メモリーカード警告画面

	2001/07/12 K.Kano
	$Id: mcchkscr.h,v 1.1.1.3 2002/11/19 11:43:39 Yoshizawa1 Exp $
*/


#ifndef _mcchkscr_h_
#define _mcchkscr_h_

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#include <float.h>
#endif

#include "gameheader.h"
#include "libutl.h"

#include "../../kira/2D_action/layout_2d.h"

#include "../mcman/mcman.h"
#include "../resman/resman.h"

#include "mode/menu/xtextscn.h"

/* text_scn.c */

/* プロトタイプ宣言 */
/* 説明分テキストを初期化する */
void MENU_ClearTextTexture( void *work );
void MENU_ClearPartTextTexture( void *work, int start_line, int height );
/* 説明文を表示する */
void MENU_PutTextScreen( void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col );
/* テキストテクスチャ管理デーモン */
void *NewTextScreenControl( void );
void *NewTextScreenControlForTitle( void );
void *NewTextScreenControlForCodec( void );


#define SE_SEL()		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_CUR01)
#define SE_OK()			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WIN01)


#endif
