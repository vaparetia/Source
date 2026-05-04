/*
  セーブロード機能

  2001/04/10 M.Kobayashi
  $Id: xsaveload.c,v 1.1.1.3 2002/11/19 11:43:48 Yoshizawa1 Exp $
 */

#include	"gameheader.h"
#include	"xmcman.h"
#include 	"private.h"
#include	"../../kira/2D_action/layout_2d.h"
#include	"../../mode/codec/codecmem.h"
#include	"../../mode/codec/codec_signal.h"
#include	"../../kano/titlescr/titlescr.h"
#include	"../../kano/titlescr/subtitle.h"
#include	"../../kano/resman/resman.h"

#define STR_COLOR		0x80555a50

#define SAVE_GAME_RESOURCE	5
#define RES_SAVING			5
#define RES_SAVE_FAILED		8
#define RES_SAVE_COMP		7

//#define LOAD_GAME_RESOURCE	0x00873e66		/* ロードゲームリソース */
#define LOAD_GAME_RESOURCE	(loadgame_resource_name[GM_Language-GM_LANG_ENGLISH])		/* ロードゲームリソース */
#define J_LOADGAME_RESOURCE		0x00f8a329		/* ロードゲームリソース日本語 */
#define E_LOADGAME_RESOURCE		0x00c44fd2		/* ロードゲームリソース英語 */
#define F_LOADGAME_RESOURCE		0x001cd935		/* ロードゲームリソースフランス語 */
#define G_LOADGAME_RESOURCE		0x00018abf		/* ロードゲームリソースドイツ語 */
#define I_LOADGAME_RESOURCE		0x005b72b4		/* ロードゲームリソースイタリア語 */
#define S_LOADGAME_RESOURCE		0x0011bd70		/* ロードゲームリソーススペイン語 */
static const int loadgame_resource_name[]={
	E_LOADGAME_RESOURCE,
	F_LOADGAME_RESOURCE,
	G_LOADGAME_RESOURCE,
	I_LOADGAME_RESOURCE,
	S_LOADGAME_RESOURCE,
	0, //韓国語
	J_LOADGAME_RESOURCE,
};
#define RES_LOADING			3
#define RES_LOAD_FAILED		5
#define RES_LOAD_COMP		4


#define	SAVE_X		20
#define SAVE_Y		318
#define SAVE_W		( SAVE_UW * SUBTITLE_FONT_DISP_WIDTH / SUBTITLE_FONT_WIDTH )
#define SAVE_H		( SAVE_VH * SUBTITLE_FONT_DISP_HEIGHT / SUBTITLE_FONT_HEIGHT )
#define SAVE_U		0
#define SAVE_V		( SUBTITLE_FONT_HEIGHT * 3 )
#define SAVE_UW		( SUBTITLE_FONT_WIDTH * 30 )
#define SAVE_VH		( SUBTITLE_FONT_HEIGHT * 5 / 4 )

// photo のメッセージに対する game のメッセージの差分
#define DIFF_X		36	
#define DIFF_Y		(-7)


typedef struct MCX_SAVELOAD {
	MCX_STATE	state;
	MCX_MENU_STRINGS	mstr_saving;
	MCX_MENU_STRINGS	mstr_completed;
	int				ctr;		// TCR 表示秒数要件を満たすためのカウンタ
	int				result;
} MCX_SAVELOAD;


static void Gui( MCX_MAN* pm, MCX_SAVELOAD* ps )
{
	switch ( ps->state.gui_state ) {
	case MCX_GUI_STATE_FADEIN:
	{
		if( ps->ctr++ > DIRECT_TICK( 80 ) ) {
			if( ps->result == RES_SAVE_COMP ) {
				// 成功
				ps->ctr = 0;
				MCX_MStringsShow( &ps->mstr_completed, DIRECT_TICK(20) );
				MCX_MStringsHide( &ps->mstr_saving, DIRECT_TICK(20) );
				// 表示ファイル名を変える
				((MCX_FILE_SELECT*)pm->pState[ MCX_STATE_FILE_SELECT ])
						->FileComplete( pm, pm->pState[ MCX_STATE_FILE_SELECT ] );
				ps->state.gui_state = MCX_GUI_STATE_ACTIVE;
				if( pm->flag & MCX_MAN_FLAG_SAVE ) {
					SE_SAVE_FINISH();
				} else {
					extern void	MC_SetLoaddataTime( FILETIME* pFt );
					MC_SetLoaddataTime( &pm->pCurfile->finddata.ftLastWriteTime );
				}
			} else if( ps->result == RES_SAVE_FAILED ) {
				ps->ctr = 0;
				// メッセージを変える
				if( pm->flag & MCX_MAN_FLAG_SAVE ) {
					MCX_MStringsClearLine( &ps->mstr_completed );
					MCX_MStringsPrintf( &ps->mstr_completed,
										GM_GetResource( SAVE_GAME_RESOURCE, RES_SAVE_FAILED ) );
					
				} else {
					char dataname[ MCX_MAX_GAMENAME ];
					if( pm->flag & MCX_MAN_FLAG_GAME ) {
						sprintf( dataname, "%s %02d", FILE_NAME_GAME, pm->pCurfile->id );
					} else if ( pm->flag & MCX_MAN_FLAG_VR ) {
						strcpy( dataname, FILE_NAME_VR );
					} else {
						sprintf( dataname, "%s %02d", FILE_NAME_SNAKE_T, pm->pCurfile->id );
					}
					MCX_MStringsClearLine( &ps->mstr_completed );
					
					MCX_MStringsPrintf( &ps->mstr_completed,
										GetLocalResource( LOAD_GAME_RESOURCE, RES_LOAD_FAILED ) ,
										dataname );
					ps->mstr_completed.y1 -= SAVE_H * 5 / 4;
				}
				
				MCX_MStringsShow( &ps->mstr_completed, DIRECT_TICK(20) );
				MCX_MStringsHide( &ps->mstr_saving, DIRECT_TICK(20) );
				ps->state.gui_state = MCX_GUI_STATE_ACTIVE;
			}
		}
		break;
	}
	case MCX_GUI_STATE_ACTIVE:
	{	//
		if( ((ps->result == RES_SAVE_COMP && ps->ctr++ > DIRECT_TICK( 200 )) ||
			 (pm->press & PAD_OK)) && MCX_MStringsIsShow( &ps->mstr_completed ) ) {
			if( ps->result == RES_SAVE_COMP ) {
				// 終了
				pm->result = SAVEGAME_ANS_SUCCESS;
			}
			MCX_ClearStart( MCX_STATE_FILE_SELECT );
			MCX_ClearStart( MCX_STATE_SL );
		}
		break;
	}
	case MCX_GUI_STATE_FADEOUT:
	{
		if( MCX_MStringsIsHide( &ps->mstr_completed )
			&& L2D_ActionStatus( pm->l2d_handle ) == L2D_STAT_ACK ) {
			MCX_ExitState( &ps->state );
			if( ps->result == RES_SAVE_COMP ) {
				MCX_ChangeState( &ps->state, MCX_STATE_END );
			} else {
				MCX_ChangeState( &ps->state, MCX_STATE_FILE_CHECK );
			}
		}
		break;
	}
	}
}

static void ClearStart( MCX_MAN* pm, MCX_SAVELOAD* ps )
{	// フェードアウト開始
	MCX_MStringsHide( &ps->mstr_saving, DIRECT_TICK( 20 ) );
	MCX_MStringsHide( &ps->mstr_completed, DIRECT_TICK( 20 ) );
}
	

static void Handler( MCX_MAN* pm, MCX_SAVELOAD* ps, int event )
{	// イベントハンドラ

	switch( event ) {
	case MCX_EVENT_MC_NULL:		// フォーカス中メモリカードが抜けた
	{
		pm->curdev = DEV_HDD;	// 次に移る（ＭＣに対応しないので組まない）
		break;
	}
	case MCX_EVENT_SL_SUCCESS:
	{
		ps->result = RES_SAVE_COMP;
//		ps->result = RES_SAVE_FAILED;
		break;
	}
	case MCX_EVENT_SL_ERROR:
	{
		ps->result = RES_SAVE_FAILED;
		break;
	}
	default:
		;
	}
}


static void Enter( MCX_MAN* pm, MCX_SAVELOAD* ps )
{	//
	// 文字準備
	int diff_x;
	int diff_y;
	char* res0;
	char* res1;
	
	if( pm->flag & MCX_MAN_FLAG_PHOTO ) {
		diff_x = 0;
		diff_y = 0;
	} else {
		diff_x = DIFF_X;
		diff_y = DIFF_Y;
	}
	if( pm->flag & MCX_MAN_FLAG_SAVE ) {
		res0 = GM_GetResource( SAVE_GAME_RESOURCE, RES_SAVING );
		res1 = GM_GetResource( SAVE_GAME_RESOURCE, RES_SAVE_COMP );
	} else {
		res0 = GetLocalResource( LOAD_GAME_RESOURCE, RES_LOADING );
		res1 = GetLocalResource( LOAD_GAME_RESOURCE, RES_LOAD_COMP );
	}
	
	MCX_MStringsCreate( &ps->mstr_saving, res0,
						SAVE_X + diff_x, SAVE_Y + diff_y, SAVE_W, SAVE_H,
						SAVE_U, SAVE_V, SAVE_UW, SAVE_VH, STR_COLOR );
	MCX_MStringsCreate( &ps->mstr_completed, res1,
						SAVE_X + diff_x, SAVE_Y + diff_y, SAVE_W, SAVE_H * 2,
						SAVE_U, SAVE_V + SAVE_VH, SAVE_UW, SAVE_VH * 2, STR_COLOR );
	MCX_MStringsClearLine( &ps->mstr_completed );
	MCX_MStringsPrintf( &ps->mstr_completed, res1 );
	
	MCX_MStringsShow( &ps->mstr_saving, DIRECT_TICK ( 30 ) );

	ps->ctr = 0;
	ps->result = RES_SAVING;
	
	MCX_QueueRequest( MCX_REQUEST_SAVE_GAME );
}

static void Exit( MCX_MAN* pm, MCX_SAVELOAD* ps )
{	//
	MCX_MStringsDelete( &ps->mstr_saving );
	MCX_MStringsDelete( &ps->mstr_completed );
}

MCX_STATE*	MCX_GetDefaultState_SaveLoad( void )
{
	MCX_SAVELOAD* ps;

	ps = (MCX_SAVELOAD *)MCX_Malloc( sizeof( MCX_SAVELOAD) );
	ps->state.GUI = Gui;
	ps->state.ClearStart = ClearStart;
	ps->state.Handler = Handler;
	ps->state.Enter = Enter;
	ps->state.Exit = Exit;
	ps->state.L2dHandler = NULL;

	return &ps->state;
}
