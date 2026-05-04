/*
  デバイスセレクト機能

  （ハードディスクのみ対応なのでいらない）

  2001/04/10 M.Kobayashi
  $Id: xdevselect.c,v 1.1.1.3 2002/11/19 11:43:46 Yoshizawa1 Exp $
 */

#include	"gameheader.h"
#include	"xmcman.h"
#include 	"private.h"
#include	"../../kira/2D_action/layout_2d.h"
#include	"../titlescr/portsel.h"
#include	"../../mode/codec/codecmem.h"

#define	N_PORT	9
#define TRI_STRCODE	GV_StrCode( "photo_save" )
#define TEX_STRCODE	GV_StrCode( "node_font_alp_ovl" )
#define CODE_PHOTOSAVE_CURSOR	9100063			/* cursor */

#define CODE_SHOWSLOT			0x002cb484			/* showBtm */
#define CODE_HIDEALL			0x0073acc4			/* hideAll */


#define	POS_MU_X		39
#define	POS_MU_Y		86
#define	POS_MU_YSTEP	16

#define STR_COLOR		0x80555a50
#define NOFOCUS_ALPHA	0x30

static char*  str_table[] = {
//	 1---5----0----5----0
	"HARD DISK",		//9,
	"MEMORY UNIT 1A",	//14,
	"MEMORY UNIT 1B",	//14,
	"MEMORY UNIT 2A",	//14,
	"MEMORY UNIT 2B",	//14,
	"MEMORY UNIT 3A",	//14,
	"MEMORY UNIT 3B",	//14,
	"MEMORY UNIT 4A",	//14,
	"MEMORY UNIT 4B",	//14,
};

#define STR_TOTAL_LEN	( 14 * 8 + 9 )

#define Cursor2Y( cur ) ( (cur) * POS_MU_YSTEP + POS_MU_Y )		

typedef struct MCX_DEV_SELECT {
	MCX_STATE	state;
	SPR_OBJ		*objbuf[STR_TOTAL_LEN];
	MCX_STRINGS	str_port[N_PORT];
	int			cursor;			// カーソル位置
	SPR_OBJ		*pSprcursor;	// カーソル表示スプライト
	MCX_INTERP	intrp_cursor_y;	// カーソルｙ座標補間
	MCX_INTERP	intrp_cursor_a;	// カーソルalpha 補間

	int			tex_handle;		// 使用したテクスチャハンドル
} MCX_DEV_SELECT;


static void Gui( MCX_MAN* pm, MCX_DEV_SELECT* ps )
{
	{ // 共通処理
		// カーソル
		ps->pSprcursor->sprite.pos.y = ps->intrp_cursor_y.fCurrent;
		ps->pSprcursor->sprite.col.a = (u_char)ps->intrp_cursor_a.fCurrent;
	}
		
	switch ( ps->state.gui_state ) {
	case MCX_GUI_STATE_FADEIN:
	{
		// 終了判定
		if( (ps->intrp_cursor_a.ctr == 0)
			&& ( L2D_ActionStatus( pm->l2d_handle ) == L2D_STAT_ACK ) ) {
			ps->state.gui_state = MCX_GUI_STATE_ACTIVE;
		}
		break;
	}
	case MCX_GUI_STATE_ACTIVE:
	{
		int i;
		if( ps->intrp_cursor_y.ctr == 0) {
			u_short press = pm->press;
			if( press & (PAD_D|PAD_U) ) {
				// カーソル移動
				if( press & PAD_D ) {
					// 次の候補を探す
					for( i = (ps->cursor + 1) % N_PORT; i != ps->cursor ; i = ( i + 1 ) % N_PORT ) {
						if( MCX_StringsIsShow( &ps->str_port[ i ] ) ) break;
					}
				} else {
					// 前の候補を探す
					for( i = (ps->cursor + N_PORT - 1) % N_PORT; i != ps->cursor ; i = ( i + N_PORT - 1 ) % N_PORT ) {
						if( MCX_StringsIsShow( &ps->str_port[ i ] ) ) break;
					}
				}
				if( i != ps->cursor ) {
					MCX_StringsNofocus( &ps->str_port[ ps->cursor ] , DIRECT_TICK(10) );
					MCX_StringsShow( &ps->str_port[ i ], DIRECT_TICK(10) );
					ps->cursor = i;
					ps->intrp_cursor_y.target = Cursor2Y( i );
					ps->intrp_cursor_y.ctr = DIRECT_TICK( 10 );
					if( i != 0 ) {
						--i;
						pm->curdev = DEV_MU_TOP + ((i & 1) << 2) + ( i >> 1 );
					} else {
						pm->curdev = DEV_HDD;
					}
				}
			}
			// 決定／キャンセル
			if( press & (PAD_OK | PAD_CANCEL) ){
				if( press & PAD_OK ) {
					MCX_ChangeState( &ps->state, MCX_STATE_FILE_CHECK );
				}
				if( press & PAD_CANCEL ) {
					MCX_ChangeState( &ps->state, MCX_STATE_END );
					L2D_EvokeAction( pm->l2d_handle, CODE_HIDEALL );
				}
				MCX_ClearStart( MCX_STATE_DEV_SELECT );
			}
		}
		break;
	}
	case MCX_GUI_STATE_FADEOUT:
	{
		// 終了判定
		if( (ps->intrp_cursor_a.ctr == 0)
			&& ( L2D_ActionStatus( pm->l2d_handle ) == L2D_STAT_ACK ) ) {
			MCX_ExitState( &ps->state );
		}
		break;
	}
	}
}
	

static void Handler( MCX_MAN* pm, MCX_DEV_SELECT* ps, int event )
{	// イベントハンドラ

	printf("mcx ev %d\n", event );
	switch( event ) {
	case MCX_EVENT_MC_STATUS:	// メモリカードステータスチェンジ
	{
		int i;
		for( i = 1 ; i < N_PORT ; i++ ) {
			int mask = (1 << (((( i - 1 ) >> 1) & 3) + ((( i - 1 ) & 1) * 16 )));
			if( pm->devchange & mask ) {
				if( pm->devstate & mask ) {
					MCX_StringsNofocus( &ps->str_port[ i ] , DIRECT_TICK ( 20 ) );
				} else {
					MCX_StringsHide( &ps->str_port[ i ] , DIRECT_TICK ( 20 ) );
				}
			}
		}
		break;
	}
	case MCX_EVENT_MC_NULL:		// フォーカス中メモリカードが抜けた
	{
		pm->curdev = DEV_HDD;
		
		ps->intrp_cursor_y.target = Cursor2Y( ps->cursor = 0 );
		ps->intrp_cursor_y.ctr = DIRECT_TICK( 10 );
		break;
	}
	default:
		;
	}
}

static void Enter( MCX_MAN* pm, MCX_DEV_SELECT* ps )
{	//
	int i;
	
	// カーソル準備
	if( pm->curdev == DEV_HDD ) {
		ps->cursor = 0;
	} else {
		ps->cursor = ((pm->curdev - DEV_MU_TOP) / 4) + ((pm->curdev - DEV_MU_TOP ) % 4) * 2 + 1;
	}
	ps->pSprcursor = L2D_GetObject( pm->l2d_handle, CODE_PHOTOSAVE_CURSOR );
	SPR_SHOW( ps->pSprcursor );
	ps->pSprcursor->sprite.col.a = 0;
	ps->intrp_cursor_y.fCurrent = (float) Cursor2Y( ps->cursor );
	ps->intrp_cursor_y.ctr = 0;
	ps->intrp_cursor_a.fCurrent = 0.f;
	ps->intrp_cursor_a.target = 0x80;
	ps->intrp_cursor_a.ctr = DIRECT_TICK( 30 );

	MCX_AddInterpService( &ps->intrp_cursor_y );
	MCX_AddInterpService( &ps->intrp_cursor_a );

	// 文字準備
	MCX_StringsInitBuffer( ps->objbuf, STR_TOTAL_LEN, pm->font_tri_handle,
						   TEX_STRCODE );
	for( i = 0 ; i < N_PORT ; i++ ) {
		int mask = (1 << (((( i - 1 ) >> 1) & 3) + ((( i - 1 ) & 1) * 16 )));
		
		MCX_StringsCreate( &ps->str_port[ i ], str_table[ i ], POS_MU_X, POS_MU_Y + POS_MU_YSTEP * i, STR_COLOR,
						   NOFOCUS_ALPHA);
		
		if( i == 0 || (mask & pm->devstate) ) {
			if( ps->cursor == i ) {
				MCX_StringsShow( &ps->str_port[ i ] , DIRECT_TICK ( 30 ) );
			} else {
				MCX_StringsNofocus( &ps->str_port[ i ] , DIRECT_TICK ( 30 ) );
			}
		}
	}
	L2D_EvokeAction( pm->l2d_handle, CODE_SHOWSLOT );
}

static void ClearStart( MCX_MAN* pm, MCX_DEV_SELECT* ps )
{	// フェードアウト開始
	int i;
	for( i = 0 ; i < N_PORT ; i++ ) {
		MCX_StringsHide( &ps->str_port[ i ], DIRECT_TICK( 30 ) );
	}
	ps->intrp_cursor_a.target = 0;
	ps->intrp_cursor_a.ctr = DIRECT_TICK( 30 );
}

static void Exit( MCX_MAN* pm, MCX_DEV_SELECT* ps )
{	//
	int i;
	for( i = 0 ; i < N_PORT ; i++ ) {
		MCX_StringsDelete( &ps->str_port[ i ] );
	}
}

MCX_STATE*	MCX_GetDefaultState_DevSelect( void )
{
	MCX_DEV_SELECT* ps;

	ps = (MCX_DEV_SELECT *)MCX_Malloc( sizeof( MCX_DEV_SELECT ) );
	ps->state.GUI = Gui;
	ps->state.Handler = Handler;
	ps->state.Enter = Enter;
	ps->state.ClearStart = ClearStart;
	ps->state.Exit = Exit;

	return &ps->state;
}
