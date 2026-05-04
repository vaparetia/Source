/*
  ファイル選択機能

  2001/04/10 M.Kobayashi
  $Id: xfilesel.c,v 1.1.1.3 2002/11/19 11:43:46 Yoshizawa1 Exp $
 */

#include	"gameheader.h"
#include	"xmcman.h"
#include 	"private.h"
#include	"../../kira/2D_action/layout_2d.h"
#include	"../../mode/codec/codec_signal.h"
#include	"../../mode/codec/codecmem.h"
#include	"../../kano/titlescr/titlescr.h"
#include	"../../kano/titlescr/subtitle.h"

#define TEX_STRCODE	GV_StrCode( "node_font_alp_ovl" )

#define DISPLAY_FILE_MAX		10

#define FILENAME_X		39
#define FILENAME_Y		86
#define FILENAME_YSTEP	18

#define YES_X			386
#define YES_Y			322
#define SLASH_X			434
#define SLASH_Y			YES_Y
#define NO_X			450
#define NO_Y			YES_Y
#define PAGE_X			20
#define PAGE_Y			42
#define	PAGENUM_X		112
#define	PAGENUM_Y		PAGE_Y

#define PAGESLASH_X		112

#define	DATASAVE_X		30
#define	DATASAVE_Y		(310+12)

enum {	// 内部使用gui_state
	GUI_STATE_WAIT = MCX_GUI_STATE_SYSTEMMAX,	// 終了時ウェイト
	GUI_STATE_PAGE_FADEOUT,
	GUI_STATE_YES_NO,
};

#define FLAG_NO_NEWDATA		0x01		// newdata 表示なし
#define FLAG_OVERWRITE		0x02		// overwrite

#define CurSelect2Y( sel )	( ((sel) % DISPLAY_FILE_MAX) * FILENAME_YSTEP + FILENAME_Y )

#define COL_FOCUS 	GM_FOCUS_COLOR
#define STR_COLOR	0x80555a50


static void Gui( MCX_MAN* pm, MCX_FILE_SELECT* ps )
{
	if( ps->GuiLocal != NULL ) {
		ps->GuiLocal( pm, ps );
	}
	
	switch ( ps->state.gui_state ) {
	case MCX_GUI_STATE_FADEIN:
	{
		// 終了判定
		if( ps->pIntrpJudge->ctr == 0 
			&& ( L2D_ActionStatus( pm->l2d_handle ) == L2D_STAT_ACK ) ) {
			ps->state.gui_state = MCX_GUI_STATE_ACTIVE;
		}
		break;
	}
	case MCX_GUI_STATE_ACTIVE:
	{
		ps->GuiActive( pm, ps );
		break;
	}
	case MCX_GUI_STATE_FADEOUT:
	{
		if( MCX_StringsIsHide( &ps->str_page )
			&& (L2D_ActionStatus( pm->l2d_handle ) == L2D_STAT_ACK )) {
//			ps->ctr = T_SHORT / 2;
			MCX_ExitState( &ps->state );
		}
		break;
	}
	case GUI_STATE_PAGE_FADEOUT:
	{	// ページフェードアウト中
		// ページが消えきったかどうか判定
		if( ps->pIntrpJudge->ctr == 0 ) {
			ps->ShowCurrentPage( pm, ps );
			ps->state.gui_state = MCX_GUI_STATE_FADEIN;	// 最初の状態に戻る
		}
		break;
	}
	case GUI_STATE_YES_NO:
	{	// Overwrite, create new の yes/no 選択
		int prev_cursor = ps->yn_cursor;
		u_short press = pm->press;

		if( !MCX_StringsIsShow( &ps->str_slash ) ) break;	// 補間中はやらない

#if 0
		{	// Yes 色決定コード
			u_char r, g, b;
			char diff;
			r = COL_FOCUS & 0xff;
			g = (COL_FOCUS >> 8) & 0xff;
			b = (COL_FOCUS >> 16) & 0xff;

			diff = ( ( GV_PadDataDirect[ 3 ].status & PAD_U ) ? 1 : 0 )
					- ( ( GV_PadDataDirect[ 3 ].status & PAD_D ) ? 1 : 0 );
			
			if( GV_PadDataDirect[ 3 ].status & PAD_A ) {
				r += diff;
			}
			if( GV_PadDataDirect[ 3 ].status & PAD_B ) {
				g += diff;
			}
			if( GV_PadDataDirect[ 3 ].status & PAD_Y ) {
				b += diff;
			}

			COL_FOCUS = ((int) r ) | ((int) g << 8 ) | ((int) b << 16 );

			if( diff ) {
				printf("%d, %d, %d\n", r, g, b );
				if( ps->yn_cursor == 0 ) {
					ps->str_yes.col = (ps->str_yes.col & 0xff000000) | ( COL_FOCUS & 0x00ffffff);
				} else {
					ps->str_no.col = (ps->str_no.col & 0xff000000) | ( COL_FOCUS & 0x00ffffff);
				}
			}
		}
#endif		

		// カーソル処理
		if( press & PAD_L ) {
			ps->yn_cursor = 0;
			if( ps->yn_cursor != prev_cursor ) {
				
				MCX_StringsShow( &ps->str_yes, DIRECT_TICK( 10 ) );
				ps->str_yes.col = (ps->str_yes.col & 0xff000000) | ( COL_FOCUS & 0x00ffffff);
				MCX_StringsNofocus( &ps->str_no, DIRECT_TICK( 10 ) );
				ps->str_no.col = (ps->str_no.col & 0xff000000) | ( STR_COLOR & 0x00ffffff);
				SE_SEL();
			}
		} else if ( press & PAD_R ) {
			ps->yn_cursor = 1;
			if( ps->yn_cursor != prev_cursor ) {
				
				MCX_StringsShow( &ps->str_no, DIRECT_TICK( 10 ) );
				ps->str_no.col = (ps->str_no.col & 0xff000000) | ( COL_FOCUS & 0x00ffffff);
				MCX_StringsNofocus( &ps->str_yes, DIRECT_TICK( 10 ) );
				ps->str_yes.col = (ps->str_yes.col & 0xff000000) | ( STR_COLOR & 0x00ffffff);
				SE_SEL();
			}
		} else if ( ps->str_yes.intrp_alpha.ctr == 0 ) { // 補間中でなければ
			// 決定・キャンセル処理
			if( /*(press & PAD_CANCEL) || */((press & PAD_OK) && ps->yn_cursor == 1 ) ) {
				// キャンセル
				MCX_MStringsHide( &ps->mstr_create, DIRECT_TICK(20) );
//				MCX_MStringsHide( &ps->mstr_overwrite, DIRECT_TICK(20) );
				MCX_StringsHide( &ps->str_yes ,DIRECT_TICK(20) );
				MCX_StringsHide( &ps->str_slash ,DIRECT_TICK(20) );
				MCX_StringsHide( &ps->str_no ,DIRECT_TICK(20) );
				ps->ExitYesNo( pm, ps );
				ps->state.gui_state = MCX_GUI_STATE_FADEIN;	// 最初に戻る
				SE_CANCEL();
			} else if( (press & PAD_OK ) && ps->yn_cursor == 0 ) {
				// 決定
				MCX_MStringsHide( &ps->mstr_create, DIRECT_TICK(20) );
//				MCX_MStringsHide( &ps->mstr_overwrite, DIRECT_TICK(20) );
				MCX_StringsHide( &ps->str_yes, DIRECT_TICK(20) );
				MCX_StringsHide( &ps->str_slash, DIRECT_TICK(20) );
				MCX_StringsHide( &ps->str_no, DIRECT_TICK(20) );
				ps->SetFileName( pm, ps );	// 内部ファイル名決定
				MCX_ChangeState( &ps->state, MCX_STATE_SL );// 画面クリアはしないまま
				SE_OK();
			}
		}
		break;
	}
	}
}

static BOOL CursorMove( MCX_MAN* pm, MCX_FILE_SELECT* ps, u_short press )
{	// カーソル移動処理
	// 動いたら TRUE　を返す
	int prevsel = ps->cursel;
	if( press & PAD_D ) {
		if( ps->cursel < ps->nselection - 1 ) {
			ps->cursel++;
			SE_SEL();
		}
	} else if( press & PAD_U ) {
		if( ps->cursel > 0 ) {
			ps->cursel--;
			SE_SEL();
		}
	} else if( press & PAD_R ) {
		if( ps->curpage < ps->maxpage ) {
			ps->cursel += DISPLAY_FILE_MAX;
			if( ps->cursel >= ps->nselection ) {
				ps->cursel = ps->nselection - 1;
			}
			SE_SEL();
		}
	} else if( press & PAD_L ) {
		if( ps->curpage > 1 ) { 
			ps->cursel -= DISPLAY_FILE_MAX;
			if( ps->cursel < 0 ) {
				ps->cursel = 0;
			}
			SE_SEL();
		}
	}	
	// カーソル移動
	if( prevsel != ps->cursel ) {
		ps->intrp_cursor_y.target = ps->Select2Y( ps->cursel );
		ps->intrp_cursor_y.ctr = DIRECT_TICK( 10 );
		// ページ移動判定
		if( (prevsel / DISPLAY_FILE_MAX) != ( ps->cursel / DISPLAY_FILE_MAX ) ) {
			ps->curpage = (ps->cursel / DISPLAY_FILE_MAX) + 1;
			ps->HideCurrentPage( pm, ps );
			ps->state.gui_state = GUI_STATE_PAGE_FADEOUT;
		} else {
			// フォーカス移動処理
			if( ps->MoveFocus != NULL ) ps->MoveFocus( pm, ps, prevsel );
		}
		return TRUE;
	}
	return FALSE;
}

static void GuiActive( MCX_MAN* pm, MCX_FILE_SELECT* ps )
{	// 	MCX_GUI_STATE_ACTIVE 時の処理
	//  photo load が特殊なために仮想化
	if( ps->intrp_cursor_y.ctr == 0) {	// カーソルが移動中でない条件
		u_short press = pm->press;
		if( ps->CursorMove( pm, ps, press ) ) return;
		// ファイル決定判定
		if( press & PAD_OK ) {
			// new data か overwrite か
			if( ps->cursel == pm->nData ) {
				// カーソル位置がデータ数に等しい＝new data
				ps->yn_cursor = 0;	// Yes デフォルト
//				MCX_MStringsShow( &ps->mstr_create, DIRECT_TICK( 20 ) );
				ps->flag &= ~FLAG_OVERWRITE;
				MCX_StringsShow( &ps->str_yes, DIRECT_TICK( 20 ) );
				ps->str_yes.col = (ps->str_yes.col & 0xff000000) | ( COL_FOCUS & 0x00ffffff);
				MCX_StringsNofocus( &ps->str_no, DIRECT_TICK( 20 ) );
				ps->str_no.col = (ps->str_no.col & 0xff000000) | ( STR_COLOR & 0x00ffffff);
			} else {
				// Overwrite
				ps->yn_cursor = 1;	// No デフォルト
				ps->flag |= FLAG_OVERWRITE;
//				MCX_MStringsShow( &ps->mstr_overwrite, DIRECT_TICK( 20 ) );
				MCX_StringsShow( &ps->str_no, DIRECT_TICK( 20 ) );
				ps->str_no.col = (ps->str_no.col & 0xff000000) | ( COL_FOCUS & 0x00ffffff);
				MCX_StringsNofocus( &ps->str_yes, DIRECT_TICK( 20 ) );
				ps->str_yes.col = (ps->str_yes.col & 0xff000000) | ( STR_COLOR & 0x00ffffff);
			}
			SE_OK();
			ps->EnterYesNo( pm, ps, (ps->cursel == pm->nData) );
			MCX_MStringsShow( &ps->mstr_create, DIRECT_TICK( 20 ) );
			MCX_StringsShow( &ps->str_slash, DIRECT_TICK( 20 ) );
			ps->state.gui_state = GUI_STATE_YES_NO;
		} else if ( press & PAD_CANCEL ) {
			// キャンセル
			pm->result = SAVEGAME_ANS_FAILED_CANCEL;
			MCX_ClearStart( MCX_STATE_FILE_SELECT );
			MCX_ChangeState( &ps->state, MCX_STATE_END );
			SE_CANCEL();
		}
	}
}


static void Handler( MCX_MAN* pm, MCX_FILE_SELECT* ps, int event )
{	// イベントハンドラ

	switch( event ) {
	case MCX_EVENT_MC_NULL:		// フォーカス中メモリカードが抜けた
	{
		pm->curdev = DEV_HDD;	// 次に移る（ＭＣに対応しないので組まない）
		break;
	}
	default:
		;
	}
}

void	MCX_SetFileSelect( MCX_FILE_SELECT* ps )
{
	ps->state.GUI = Gui;
	ps->state.Handler = Handler;
	ps->state.L2dHandler = NULL;
	ps->GuiActive = GuiActive;
	ps->CursorMove = CursorMove;
}
