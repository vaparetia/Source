/*
  ファイルチェック機能

  2001/04/10 M.Kobayashi
  $Id: xfilechk.c,v 1.4 2002/11/23 12:42:24 Yoshizawa1 Exp $
 */

#include	"gameheader.h"
#include	"xmcman.h"
#include 	"private.h"
#include	"../../kira/2D_action/layout_2d.h"
#include	"../../mode/codec/codecmem.h"
#include	"../../kano/titlescr/titlescr.h"
#include	"../../kano/titlescr/subtitle.h"
#include	"../../kano/resman/resman.h"


#define	NO_STR		// 早いので文字表示なし

#ifndef NO_STR
#define TEX_STRCODE	GV_StrCode( "node_font_alp_ovl" )


#define	CHECKING_X_P	30
#define	CHECKING_Y_P	(310+12)

#define CHECKING_X_G	58
#define CHECKING_Y_G	72

static char*  str_table[] = {
//	 1---5----0----5----0----5
	"CHECKING HARD DISK...",	//21,
};

#define STR_TOTAL_LEN	( 21 )

#endif

#define	ERR_X		56
#define ERR_Y		72
#define	ERR_X_P		20
#define ERR_Y_P		318
#define ERR_W		( ERR_UW * SUBTITLE_FONT_DISP_WIDTH / SUBTITLE_FONT_WIDTH )
#define ERR_H		( ERR_VH * SUBTITLE_FONT_DISP_HEIGHT / SUBTITLE_FONT_HEIGHT )
#define ERR_U		0
#define ERR_V		( SUBTITLE_FONT_HEIGHT * 2 )
#define ERR_UW		( SUBTITLE_FONT_WIDTH * 30 )
#define ERR_VH		( SUBTITLE_FONT_HEIGHT * 3 )

#define STR_COLOR		0x80555a50


#define SAVE_GAME_RESOURCE	5
#define RES_NOSPACE			1
//#define RES_MANYDIRS		4
#define RES_MANYDIRS		1

#define LOAD_GAME_RESOURCE	(loadgame_resource_name[GM_Language-GM_LANG_ENGLISH])		/* ロードゲームリソース */
//#define LOAD_GAME_RESOURCE	0x00873e66		/* ロードゲームリソース */
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
#define RES_NOFILE			1


static const int tag_part_name[]={
	CODE_SRASH,
	CODE_DOGTAG,
	CODE_ALBUMTAG,
	CODE_DATASAVETAG,
	CODE_DATALOADTAG,
	CODE_UNDERBAR,
};

#define PART_SIZE		(sizeof(tag_part_name)/sizeof(tag_part_name[0]))

enum {
	GUI_STATE_CANCEL_DISP = MCX_GUI_STATE_SYSTEMMAX,
	GUI_STATE_CANCEL_END,
};

enum {
	FLAG_CHECKEND	= 0x01,
	FLAG_SOUNDCODE	= 0x02,
	FLAG_INIT		= 0x04,
};

typedef struct MCX_FILE_CHECK {
	MCX_STATE	state;
#ifndef NO_STR	
	SPR_OBJ		*objbuf[STR_TOTAL_LEN];
	MCX_STRINGS	checking;
#endif
	MCX_MENU_STRINGS	mstr_error;
	int			ctr;		// 1(s) 以上のＴＣＲを満たすためのカウンタ
	u_int		flag;	

	MCX_L2D_MORF	morf[ PART_SIZE ];
	int 		showcode;
	int 		hidecode;
	int			l2d_action;
} MCX_FILE_CHECK;


/////////////
// ファイルソート用関数
#if 0
static MCX_FILEPROP* pFilesLocal;
static int Compare( const u_char* pa, const u_char* pb )
{	// ソート用比較関数
	return (pFilesLocal + *pa)->id - (pFilesLocal + *pb)->id;
}

static void SetOrder( MCX_FILEPROP* pFiles, u_char* pindex, int nData )
{
//	MCX_FILEPROP filestmp[ MAX_FILES ];	// でかくてスタックに取れない
	MCX_FILEPROP *filestmp = (MCX_FILEPROP*)MCX_Malloc( sizeof( MCX_FILEPROP ) * MAX_FILES );
	int i;
	
	for( i = 0 ; i < nData; i++ ) {
		filestmp[ i ] = *(pFiles + *(pindex + i));
	}
	memcpy( pFiles, filestmp, sizeof( MCX_FILEPROP ) * nData );
	MCX_Free( filestmp );
}
#endif
///////////


static void Gui( MCX_MAN* pm, MCX_FILE_CHECK* ps )
{
	{ // 共通処理
		if( ps->ctr > 0 ) ps->ctr--;
		if( ps->l2d_action != 0 && L2D_ActionStatus( pm->l2d_handle ) == L2D_STAT_ACK ) {
			L2D_EvokeAction( pm->l2d_handle, ps->l2d_action );
			ps->l2d_action = 0;
		}
	}

	switch ( ps->state.gui_state ) {
	case MCX_GUI_STATE_FADEIN:
	{
		// 終了判定
		if( 
#ifndef NO_STR		
				MCX_StringsIsShow( &ps->checking ) &&
#endif				
				( L2D_ActionStatus( pm->l2d_handle ) == L2D_STAT_ACK ) ) {
			ps->state.gui_state = MCX_GUI_STATE_ACTIVE;
		}
		break;
	}
	case MCX_GUI_STATE_ACTIVE:
	{	// 
		if( ps->flag & FLAG_CHECKEND /*&& ps->ctr <= 0*/ ) {
			// １．ロード時データが無いとき
			// ２．セーブ時データが無く空き領域がないとき
			// ３．ディレクトリが多すぎるとき
			// メッセージを出してキャンセルリターン
			if( pm->nData == 0 ) {
				if( pm->flag & MCX_MAN_FLAG_SAVE ) {
					if( pm->flag & MCX_MAN_FLAG_NOSPACE ) {
						// ２の条件
						MCX_MStringsPrintf( &ps->mstr_error, "%s",
											(char*)GM_GetResource( SAVE_GAME_RESOURCE, RES_NOSPACE ) );
						MCX_MStringsShow( &ps->mstr_error, T_MID );
						pm->result = SAVEGAME_ANS_FAILED_CANCEL;
						ps->state.gui_state = GUI_STATE_CANCEL_DISP;
						break;
					}
					if( pm->flag & MCX_MAN_FLAG_MANYDIRS ) {
						// ３の条件
						MCX_MStringsPrintf( &ps->mstr_error, "%s",
											(char*)GM_GetResource( SAVE_GAME_RESOURCE, RES_MANYDIRS ) );
						MCX_MStringsShow( &ps->mstr_error, T_MID );
						pm->result = SAVEGAME_ANS_FAILED_CANCEL;
						ps->state.gui_state = GUI_STATE_CANCEL_DISP;
						break;
					}
				} else {
					// １の条件
					char buf[ 256 ];
					char* pdatatype;
					if( pm->flag & MCX_MAN_FLAG_PHOTO ) {
						pdatatype = FILE_NAME_PHOTO;
					} else if( pm->flag & MCX_MAN_FLAG_VR ) {
						pdatatype = FILE_NAME_VR;
					} else if( pm->flag & MCX_MAN_FLAG_SNAKE_T ) {
						pdatatype = FILE_NAME_SNAKE_T;
					} else {
						pdatatype = FILE_NAME_GAME;
					}
					sprintf( buf, (char*)GetLocalResource( LOAD_GAME_RESOURCE, RES_NOFILE ), pdatatype );
					
					MCX_MStringsPrintf( &ps->mstr_error, buf );
					MCX_MStringsShow( &ps->mstr_error, T_MID );
					pm->result = SAVEGAME_ANS_FAILED_CANCEL;
					ps->state.gui_state = GUI_STATE_CANCEL_DISP;
					break;
				}
			}
			MCX_ChangeState( &ps->state, MCX_STATE_FILE_SELECT );
			MCX_ClearStart( MCX_STATE_FILE_CHECK );
#if 0			
			{	// データのソート
				// 別スレッド側でやっていたが何故かたまに落ちるので
				int i;
				u_char index[ MAX_FILES ];
				for( i = 0 ; i < MAX_FILES; i++ ) {
					index[ i ] = (u_char)i;
				}
				pFilesLocal = pm->pFiles;
				printf("sort0\n");
				qsort( index, pm->nData, sizeof(u_char), Compare );
				printf("sort1\n");
				SetOrder( pm->pFiles, index, pm->nData );
				printf("sort2\n");
			}
#endif			
		}
		break;
	}
	case MCX_GUI_STATE_FADEOUT:
	{
		if( 
#ifndef NO_STR			
				MCX_StringsIsHide( &ps->checking ) &&
#endif
				( L2D_ActionStatus( pm->l2d_handle ) == L2D_STAT_ACK ) ) {
//			MCX_ClearStart( MCX_STATE_FILE_CHECK );
			MCX_ExitState( &ps->state );
		}
		break;
	}
	case GUI_STATE_CANCEL_DISP:
	{
		if( MCX_MStringsIsShow( &ps->mstr_error ) &&
			L2D_ActionStatus( pm->l2d_handle ) == L2D_STAT_ACK &&
#if 0			
			( pm->press & PAD_OK )
#else	// これで戻らないとだめらしい・・・謎
			( pm->press & PAD_CANCEL )
#endif			
			) {
			ps->l2d_action = CODE_CLOSEALL;
			MCX_MStringsHide( &ps->mstr_error, T_MID );
			ps->state.gui_state = GUI_STATE_CANCEL_END;
			ps->ctr = T_SHORT / 2;
			SE_OK();
			SE_WINCLOSE();
		}
		break;
	}
	case GUI_STATE_CANCEL_END:
	{
		if( MCX_MStringsIsHide( &ps->mstr_error ) &&
			L2D_ActionStatus( pm->l2d_handle ) == L2D_STAT_ACK ) {
			if( --ps->ctr < 0 ) {
				MCX_ChangeState( &ps->state, MCX_STATE_END );
				MCX_ExitState( &ps->state );
			}
		}
		break;
	}
	}
}

static void GuiFirstCheck( MCX_MAN* pm, MCX_FILE_CHECK* ps )
{	// 最初のチェックのときのＧＵＩ
	switch ( ps->state.gui_state ) {
	case MCX_GUI_STATE_FADEIN:
		if( ps->flag & FLAG_CHECKEND /*&& ps->ctr <= 0*/ ) {
			MCX_ChangeState( &ps->state, MCX_STATE_END );
			MCX_ClearStart( MCX_STATE_FILE_CHECK );
			MCX_ExitState( &ps->state );
		}
	default:
		break;
	}

}

static void ClearStart( MCX_MAN* pm, MCX_FILE_CHECK* ps )
{	// フェードアウト開始
#ifndef NO_STR	
	MCX_StringsHide( &ps->checking, DIRECT_TICK( 30 ) );
#endif
}
	

static void Handler( MCX_MAN* pm, MCX_FILE_CHECK* ps, int event )
{	// イベントハンドラ

	switch( event ) {
	case MCX_EVENT_MC_NULL:		// フォーカス中メモリカードが抜けた
	{
		pm->curdev = DEV_HDD;	// 次に移る（ＭＣに対応しないので組まない）
		break;
	}
	case MCX_EVENT_FIND_SUCCESS:
	{
		ps->flag |= FLAG_CHECKEND;
		break;
	}
	default:
		;
	}
}

static void L2dHandler( MCX_MAN* pm, MCX_FILE_CHECK* ps, int sign, int value )
{
	int i;
	switch( sign ) {
	case CODE_HIDE_MODE:
		for( i = 0 ; i < PART_SIZE; i++ ) {
			MCX_L2DMorfSet( &ps->morf[ i ], ps->showcode, ps->hidecode, value / DIV_TICK_VALUE );
		}
		break;
	case CODE_SHOW_MODE:
		for( i = 0 ; i < PART_SIZE; i++ ) {
			MCX_L2DMorfSet( &ps->morf[ i ], ps->hidecode, ps->showcode, value / DIV_TICK_VALUE );
		}
		break;
	case CODE_SOUND:
		if( ps->flag & FLAG_SOUNDCODE ) {
			SE_EXPANDLINE();
		} else {
			SE_WINOPEN();
			ps->flag |= FLAG_SOUNDCODE;
		}
		break;
	}
}

static void Enter( MCX_MAN* pm, MCX_FILE_CHECK* ps )
{	//
	// 文字準備
#ifndef NO_STR
	MCX_StringsInitBuffer( ps->objbuf, STR_TOTAL_LEN, pm->font_tri_handle,
						   TEX_STRCODE );
#endif	
	ps->flag &= FLAG_INIT;
	ps->l2d_action = 0;
	
	if( pm->flag & MCX_MAN_FLAG_PHOTO ) {
#ifndef NO_STR
		MCX_StringsCreate( &ps->checking, str_table[ 0 ], CHECKING_X_P, CHECKING_Y_P, STR_COLOR,
						   0 );
#endif		
		ps->l2d_action = pm->l2d_tab.a_check_show;
		SE_EXPANDLINE();
	} else {
		int i;
#ifndef NO_STR			
		MCX_StringsCreate( &ps->checking, str_table[ 0 ], CHECKING_X_G, CHECKING_Y_G, STR_COLOR,
						   0 );
#endif		
		if( pm->flag & MCX_MAN_FLAG_SAVE ) {
			ps->showcode = CODE_SHOW_SAVE;
			ps->hidecode = CODE_HIDE_SAVE;
		} else if( pm->flag & MCX_MAN_FLAG_DOGTAG ) {
			ps->showcode = CODE_SHOW_DOGTAGLOAD;
			ps->hidecode = CODE_HIDE_DOGTAGLOAD;
		} else {
			ps->showcode = CODE_SHOW_LOAD;
			ps->hidecode = CODE_HIDE_LOAD;
		}

		if( !( ps->flag & FLAG_INIT ) ) {
			for( i = 0; i < PART_SIZE ; i++ ) {
				MCX_L2DMorfCreate( &ps->morf[ i ], pm->l2d_handle, tag_part_name[ i ] );
			}
			ps->flag |= FLAG_INIT;
		}
		
		if( pm->l2d_handle >= 0 ) {
			if( (ps->l2d_action = pm->l2d_tab.a_check_show) != 0 ) {
				SE_EXPANDLINE();
			} else {
				ps->flag |= FLAG_SOUNDCODE;
			}
		}
	}
#ifndef NO_STR
	MCX_StringsShow( &ps->checking , DIRECT_TICK ( 30 ) );
#endif	

	ps->ctr = DIRECT_TICK( 66 );

	if( pm->flag & MCX_MAN_FLAG_FIRSTCHECK ) {
		ps->state.GUI = GuiFirstCheck;
	} 
	pm->nData = 0;

	{
		int err_x;
		int err_y;
		if( (pm->flag & MCX_MAN_FLAG_SAVE) &&
			(pm->flag & MCX_MAN_FLAG_PHOTO ) ) {
			err_x = ERR_X_P;
			err_y = ERR_Y_P;
			if( GM_Language == GM_LANG_SPANISH ) {
				err_y -= SUBTITLE_FONT_WIDTH * 5 / 4;
			}
		} else {
			err_x = ERR_X;
			err_y = ERR_Y;
		}
		MCX_MStringsCreate( &ps->mstr_error, " ",
							err_x, err_y, ERR_W, ERR_H,
							ERR_U, ERR_V, ERR_UW, ERR_VH, STR_COLOR );
	}
	
	MCX_QueueRequest( MCX_REQUEST_FIND_GAME );
}

static void Exit( MCX_MAN* pm, MCX_FILE_CHECK* ps )
{	//
#ifndef NO_STR
	MCX_StringsDelete( &ps->checking );
#endif	
	MCX_MStringsDelete( &ps->mstr_error );

}

MCX_STATE*	MCX_GetDefaultState_FileCheck( void )
{
	MCX_FILE_CHECK* ps;

	ps = (MCX_FILE_CHECK *)MCX_Malloc( sizeof( MCX_FILE_CHECK) );
	ps->state.GUI = Gui;
	ps->state.ClearStart = ClearStart;
	ps->state.Handler = Handler;
	ps->state.Enter = Enter;
	ps->state.Exit = Exit;
	ps->state.L2dHandler = L2dHandler;
	ps->flag = 0;
	
	return &ps->state;
}
