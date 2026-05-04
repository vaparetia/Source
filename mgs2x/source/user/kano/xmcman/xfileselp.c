/*
  ファイル選択機能
  写真画面部分差分

  2001/04/10 M.Kobayashi
  $Id: xfileselp.c,v 1.1.1.3 2002/11/19 11:43:47 Yoshizawa1 Exp $
 */

#include	"gameheader.h"
#include	"xmcman.h"
#include 	"private.h"
#include	"../../kira/2D_action/layout_2d.h"
#include	"../../mode/codec/codecmem.h"
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

#define NAME_X			(FILENAME_X + 16 * 3)

#define PAGESLASH_X		112

#define	DATASAVE_X		30
#define	DATASAVE_Y		(310+12)

#define PAGE_W			14
#define PAGE_H			10
#define PAGENUM_W		18
#define PAGENUM_H		14


#define OVER_WRITE_X	20
#define OVER_WRITE_Y	318
#define OVER_WRITE_U	0
#define OVER_WRITE_V	0
#define OVER_WRITE_UW	(SUBTITLE_FONT_WIDTH * 30)
#define OVER_WRITE_VH 	(SUBTITLE_FONT_HEIGHT * 4 / 3)
#define OVER_WRITE_W	( OVER_WRITE_UW * SUBTITLE_FONT_DISP_WIDTH / SUBTITLE_FONT_WIDTH )
#define OVER_WRITE_H 	( OVER_WRITE_VH * SUBTITLE_FONT_DISP_HEIGHT / SUBTITLE_FONT_HEIGHT )


#define STR_COLOR		0x80555a50
#define NOFOCUS_A		0x30

#define CODE_PHOTOSAVE_CURSOR	9100063				/* cursor */
#define CODE_SHOWSLOT			0x002cb484			/* showBtm */
#define CODE_SHOWFILE			0x002cfbe7			/* showTop */
#define CODE_HIDEALL			0x0073acc4			/* hideAll */

#define SAVE_GAME_RESOURCE	5
#define RES_NEWDATA			2
#define RES_OVERWRITE		3

static short Select2Y( int sel ) {
	return ((sel) % DISPLAY_FILE_MAX) * FILENAME_YSTEP + FILENAME_Y;
}

enum {	// 内部使用gui_state
	GUI_STATE_PAGE_FADEOUT = MCX_GUI_STATE_SYSTEMMAX,
	GUI_STATE_YES_NO,
};


static char*  str_table[] = {
//	 1---5----0----5----0----5
//	"00 PHOTO 001 ",		// 13,
//	"00 DAMAGEDDATA",		// 14,
#if 1	//AREA_EU_BP_IGNORE()
	"00 NEW PHOTO DATA",	// 17,	
#endif	
	"DATA SAVE/HARD DISK",	// 19,
	"YES",					// 3	
	"/",					// 1
	"NO",					// 2
	"PAGE",					// 4
	"00/00",				// 5
};

//#define STR_TOTAL_LEN	( 14 * DISPLAY_FILE_MAX + 19 + 3 + 1 + 2 + 4 + 5)
#define STR_TOTAL_LEN	( 17 * DISPLAY_FILE_MAX + 19 + 3 + 1 + 2 + 4 + 5)

#define FLAG_NO_NEWDATA		0x01		// newdata 表示なし
#define FLAG_OVERWRITE		0x02		// overwrite


typedef struct MCX_FILE_SELECT_PHOTO{
	MCX_FILE_SELECT	fs;
	MCX_STRINGS	str_filename[ DISPLAY_FILE_MAX ];
	MCX_STRINGS str_datasave;
	int	newfile_id;
} MCX_FILE_SELECT_PHOTO;

static void ShowCurrentPage( MCX_MAN* pm, MCX_FILE_SELECT_PHOTO* ps )
{	// 現在のページを制作して表示
	int start_file = DISPLAY_FILE_MAX * (ps->fs.curpage - 1);
	int i;
	for( i = start_file ; i < pm->nData && i < start_file + DISPLAY_FILE_MAX; i++ ) {
		if( MCX_GetFileInfo( i )->flag & MCX_FILEPROP_FLAG_DAMAGED ) {
			MCX_StringsPrintf( &ps->str_filename[ i - start_file ], "%02d DAMAGEDDATA",
							   MCX_GetFileInfo( i )->id );
		} else {
			MCX_StringsPrintf( &ps->str_filename[ i - start_file ],
							   "%02d %s", MCX_GetFileInfo( i )->id,
							   MCX_GetFileInfo( i )->info.photoinfo.name );
		}
		MCX_StringsMove( &ps->str_filename[ i - start_file ], 3, NAME_X );
		if( ps->fs.cursel == i ) {
			MCX_StringsShow( &ps->str_filename[ i - start_file ], DIRECT_TICK(10) );
		} else {
			MCX_StringsNofocus( &ps->str_filename[ i - start_file ], DIRECT_TICK(10) );
		}
	}
	if( ps->fs.curpage == ps->fs.maxpage ) {
		if( !(ps->fs.flag & FLAG_NO_NEWDATA )) {
			MCX_StringsPrintf( &ps->str_filename[ i - start_file ],
							   "%02d %s", ps->newfile_id,
							   "NEW PHOTO DATA" );
			MCX_StringsMove( &ps->str_filename[ i - start_file ], 3, NAME_X );
			if( ps->fs.cursel == i ) {
				MCX_StringsShow( &ps->str_filename[ i - start_file ], DIRECT_TICK(10) );
			} else {
				MCX_StringsNofocus( &ps->str_filename[ i - start_file ], DIRECT_TICK(10) );
			}
		}
	}
	// ページ番号を表示
	MCX_StringsPrintf(&ps->fs.str_pagenum, "%02d/%02d", ps->fs.curpage, ps->fs.maxpage );
	{	// '/' の位置が合うように調整
		int diff;
		diff = PAGESLASH_X - (int)(ps->fs.str_pagenum.ppObj[ 2 ])->sprite.pos.x;
		for ( i = 0 ; i < ps->fs.str_pagenum.len ; i ++ ){
			(ps->fs.str_pagenum.ppObj[ i ])->sprite.pos.x += diff;
		}
	}
	MCX_StringsShow( &ps->fs.str_pagenum, DIRECT_TICK( 30 ) );
	ps->fs.pIntrpJudge = &ps->str_filename[ 0 ].intrp_alpha;
}

static void HideCurrentPage( MCX_MAN* pm, MCX_FILE_SELECT_PHOTO* ps )
{
	int i;
	for( i = 0 ; i < DISPLAY_FILE_MAX ; i++ ) {
		MCX_StringsHide( &ps->str_filename[ i ] , DIRECT_TICK( 10 ) );
	}

	ps->fs.pIntrpJudge = &ps->str_filename[ 0 ].intrp_alpha;
}

static void GuiLocal( MCX_MAN* pm, MCX_FILE_SELECT_PHOTO* ps )
{	// ＧＵＩ非共通部分
	// カーソル
	ps->fs.pSprcursor[ 0 ]->sprite.col.a = (u_char)ps->fs.intrp_cursor_a.fCurrent;
	ps->fs.pSprcursor[ 0 ]->sprite.pos.y = ps->fs.intrp_cursor_y.fCurrent;
}

static void EnterYesNo( MCX_MAN* pm, MCX_FILE_SELECT_PHOTO* ps, int bNewdata )
{
	MCX_StringsHide( &ps->str_datasave, DIRECT_TICK( 20 ) );
	
	MCX_MStringsClearLine( &ps->fs.mstr_create );
	if( bNewdata ) {
		if( pm->flag & MCX_MAN_FLAG_SAVE ) {
			switch( GM_Language ) {
			case GM_LANG_FRENCH:
			case GM_LANG_SPANISH:
				ps->fs.mstr_create.y1 = OVER_WRITE_Y - OVER_WRITE_H - 3;
				break;
			default:
				ps->fs.mstr_create.y1 = OVER_WRITE_Y;
				break;
			}
		} else {
			ps->fs.mstr_create.y1 = OVER_WRITE_Y;
		}
		MCX_MStringsPrintf( &ps->fs.mstr_create, GM_GetResource( SAVE_GAME_RESOURCE, RES_NEWDATA ) );
	} else {
		MCX_MStringsPrintf( &ps->fs.mstr_create, GM_GetResource( SAVE_GAME_RESOURCE, RES_OVERWRITE ) );
		ps->fs.mstr_create.y1 = OVER_WRITE_Y;
	}
}

static void ExitYesNo( MCX_MAN* pm, MCX_FILE_SELECT_PHOTO* ps )
{
	MCX_StringsShow( &ps->str_datasave, DIRECT_TICK( 20 ) );
}

static void FileComplete( MCX_MAN* pm, MCX_FILE_SELECT_PHOTO* ps )
{	// ファイル操作が成功したときに呼ばれる

	// 名前を反映
	MCX_StringsPrintf( &ps->str_filename[ ps->fs.cursel % DISPLAY_FILE_MAX ],
					   "%02d %s", pm->pCurfile->id,
					   pm->pCurfile->info.photoinfo.name );
	MCX_StringsMove( &ps->str_filename[ ps->fs.cursel % DISPLAY_FILE_MAX ], 3, NAME_X );
}

static void SetFileName( MCX_MAN* pm, MCX_FILE_SELECT_PHOTO* ps )
{
	// ファイル名決定
	int id;
	pm->pCurfile = MCX_GetFileInfo( ps->fs.cursel );
	if( !(ps->fs.flag & FLAG_OVERWRITE )) {
		// 新規ファイル
		id = ps->newfile_id;
	} else {
		id = pm->pCurfile->id;
	}
	ZeroMemory( &pm->pCurfile->info, sizeof( MCX_FILE_INFO ) );
	sprintf( pm->pCurfile->info.photoinfo.name, "PHOTO %03d", id );
	MCX_SetFileName( pm->pCurfile, id );
	// デフォルト色記録
	pm->pCurfile->info.photoinfo.col[ 0 ] = pm->pCurfile->info.photoinfo.col[ 1 ]
			= pm->pCurfile->info.photoinfo.col[ 2 ] = 0x80;
}

static void MoveFocus( MCX_MAN* pm, MCX_FILE_SELECT_PHOTO* ps, int prevsel )
{
	MCX_StringsShow( &ps->str_filename[ ps->fs.cursel % DISPLAY_FILE_MAX ], DIRECT_TICK( 10 ) );
	MCX_StringsNofocus( &ps->str_filename[ prevsel % DISPLAY_FILE_MAX ], DIRECT_TICK( 10 ) );
}


static void ClearStart( MCX_MAN* pm, MCX_FILE_SELECT_PHOTO* ps )
{	// フェードアウト開始
	int i;
	for ( i = 0 ; i < DISPLAY_FILE_MAX ; i++ ) {
		MCX_StringsHide( &ps->str_filename[ i ], DIRECT_TICK(20) );
	}
	MCX_StringsHide( &ps->str_datasave, DIRECT_TICK(20) );
	MCX_StringsHide( &ps->fs.str_yes, DIRECT_TICK(20) );
	MCX_StringsHide( &ps->fs.str_slash, DIRECT_TICK(20) );
	MCX_StringsHide( &ps->fs.str_no, DIRECT_TICK(20) );
	MCX_StringsHide( &ps->fs.str_page, DIRECT_TICK(20) );
	MCX_StringsHide( &ps->fs.str_pagenum, DIRECT_TICK(20) );

	MCX_MStringsHide( &ps->fs.mstr_create, DIRECT_TICK(20) );
//	MCX_MStringsHide( &ps->fs.mstr_overwrite, DIRECT_TICK(20) );

	L2D_EvokeAction( pm->l2d_handle, pm->l2d_tab.a_hide );
	
	ps->fs.intrp_cursor_a.target = 0;
	ps->fs.intrp_cursor_a.ctr = DIRECT_TICK(20) ;
}
	
static void Enter( MCX_MAN* pm, MCX_FILE_SELECT_PHOTO* ps )
{	//
	int i;
	
	// 文字準備
	ps->fs.ppobjbuf = (SPR_OBJ**)MCX_Malloc( sizeof(SPR_OBJ*) * STR_TOTAL_LEN );
	
	MCX_StringsInitBuffer( ps->fs.ppobjbuf, STR_TOTAL_LEN, pm->font_tri_handle,
						   TEX_STRCODE );
	for( i = 0 ; i < DISPLAY_FILE_MAX ; i++ ) {
		MCX_StringsCreate( &ps->str_filename[ i ], str_table[ 0 ], FILENAME_X, FILENAME_Y + FILENAME_YSTEP * i, STR_COLOR,
						   NOFOCUS_A );
	}
	MCX_StringsCreate( &ps->str_datasave, str_table[ 1 ], DATASAVE_X, DATASAVE_Y, STR_COLOR, NOFOCUS_A );
	MCX_StringsCreate( &ps->fs.str_yes, str_table[ 2 ], YES_X, YES_Y, STR_COLOR, NOFOCUS_A );
	MCX_StringsCreate( &ps->fs.str_slash, str_table[ 3 ], SLASH_X, SLASH_Y, STR_COLOR, NOFOCUS_A );
	MCX_StringsCreate( &ps->fs.str_no, str_table[ 4 ], NO_X, NO_Y, STR_COLOR, NOFOCUS_A );
	MCX_StringsCreate( &ps->fs.str_page, str_table[ 5 ], PAGE_X, PAGE_Y, STR_COLOR, NOFOCUS_A );
	MCX_StringsCreate( &ps->fs.str_pagenum, str_table[ 6 ], PAGENUM_X, PAGENUM_Y, STR_COLOR, NOFOCUS_A );
	
	MCX_MStringsCreate( &ps->fs.mstr_create, GM_GetResource( SAVE_GAME_RESOURCE, RES_NEWDATA ),
						OVER_WRITE_X, OVER_WRITE_Y, OVER_WRITE_W, OVER_WRITE_H * 2,
						OVER_WRITE_U, OVER_WRITE_V, OVER_WRITE_UW, OVER_WRITE_VH * 2, STR_COLOR );
#if 0	
	MCX_MStringsCreate( &ps->fs.mstr_overwrite, GM_GetResource( SAVE_GAME_RESOURCE, RES_OVERWRITE ),
						OVER_WRITE_X, OVER_WRITE_Y, OVER_WRITE_W, OVER_WRITE_H,
						OVER_WRITE_U, OVER_WRITE_V + OVER_WRITE_VH * 2,
						OVER_WRITE_UW, OVER_WRITE_VH, STR_COLOR );
#endif	

	// str_page の大きさを変える
	(*ps->fs.str_page.ppObj)->sprite.dw = PAGE_W;
	(*ps->fs.str_page.ppObj)->sprite.dh = PAGE_H;
	MCX_StringsPrintf( &ps->fs.str_page, str_table[ 5 ] );
	// str_pagenum の大きさを変える
	(*ps->fs.str_pagenum.ppObj)->sprite.dw = PAGENUM_W;
	(*ps->fs.str_pagenum.ppObj)->sprite.dh = PAGENUM_H;

	MCX_StringsShow( &ps->str_datasave , DIRECT_TICK ( 30 ) );
	MCX_StringsShow( &ps->fs.str_page , DIRECT_TICK ( 30 ) );

	ps->fs.flag = 0;

	// ページ数計算
	if( (pm->flag & MCX_MAN_FLAG_SAVE) &&
		!(pm->flag & ( MCX_MAN_FLAG_NOSPACE | MCX_MAN_FLAG_MANYDIRS) ) ) {
		ps->fs.maxpage = pm->nData / 10 + 1;
		if( ps->fs.maxpage > 10 ) {
			ps->fs.maxpage = 10;
			ps->fs.flag |= FLAG_NO_NEWDATA;
		}
	} else {
		ps->fs.maxpage = (pm->nData - 1) / 10 + 1;
		ps->fs.flag |= FLAG_NO_NEWDATA;
	}
	
	// 新規の場合のファイル名を準備しておく
	for( i = 0 ; i < 100 ; i++ ) {
		int j;
		// かぶった名前がないか調査
		for ( j = 0 ; j < pm->nData ; j++ ) {
			if( MCX_GetFileInfo( j )->id == i ) {
				break;
			}
		}
		if( j == pm->nData ) break;
	}
	ps->newfile_id = i;
	
	
	ps->fs.nselection = (ps->fs.flag & FLAG_NO_NEWDATA) ? pm->nData : pm->nData + 1;
	// 常にNEW DATA へ
	ps->fs.cursel = ps->fs.nselection - 1;
	ps->fs.curpage = (ps->fs.cursel / DISPLAY_FILE_MAX) + 1;

	ShowCurrentPage( pm, ps );
	L2D_EvokeAction( pm->l2d_handle, CODE_SHOWFILE );

	// カーソル準備
	ps->fs.pSprcursor[ 0 ] = L2D_GetObject( pm->l2d_handle, CODE_PHOTOSAVE_CURSOR );
	ps->fs.pSprcursor[ 1 ] = NULL;
	SPR_SHOW( ps->fs.pSprcursor[ 0 ] );
	ps->fs.pSprcursor[ 0 ] ->sprite.col.a = 0;
	ps->fs.intrp_cursor_y.fCurrent = (float) Select2Y( ps->fs.cursel );
	ps->fs.intrp_cursor_y.ctr = 0;
	ps->fs.intrp_cursor_a.fCurrent = 0.f;
	ps->fs.intrp_cursor_a.target = 0x80;
	ps->fs.intrp_cursor_a.ctr = DIRECT_TICK( 30 );
	MCX_AddInterpService( &ps->fs.intrp_cursor_y );
	MCX_AddInterpService( &ps->fs.intrp_cursor_a );

	MCX_SetAutoRepeat( 10 );
}

static void Exit( MCX_MAN* pm, MCX_FILE_SELECT_PHOTO* ps )
{	//
	int i;
	for ( i = 0 ; i < DISPLAY_FILE_MAX ; i++ ) {
		MCX_StringsDelete( &ps->str_filename[ i ] );
	}
	MCX_StringsDelete( &ps->str_datasave );
	MCX_StringsDelete( &ps->fs.str_yes );
	MCX_StringsDelete( &ps->fs.str_slash );
	MCX_StringsDelete( &ps->fs.str_no );
	MCX_StringsDelete( &ps->fs.str_page );
	MCX_StringsDelete( &ps->fs.str_pagenum );

	MCX_MStringsDelete( &ps->fs.mstr_create );
//	MCX_MStringsDelete( &ps->fs.mstr_overwrite );

	MCX_DeleteInterp( &ps->fs.intrp_cursor_y );
	MCX_DeleteInterp( &ps->fs.intrp_cursor_a );
	MCX_Free( ps->fs.ppobjbuf );
}

MCX_STATE*	MCX_GetFileSelectPhoto( void )
{
	MCX_FILE_SELECT_PHOTO* ps;

	ps = (MCX_FILE_SELECT_PHOTO *)MCX_Malloc( sizeof( MCX_FILE_SELECT_PHOTO ) );

	MCX_SetFileSelect( &ps->fs );

	// override
	ps->fs.state.Enter = Enter;
	ps->fs.state.Exit = Exit;
	ps->fs.state.ClearStart = ClearStart;
	ps->fs.ShowCurrentPage = ShowCurrentPage;
	ps->fs.HideCurrentPage = HideCurrentPage;
	ps->fs.EnterYesNo = EnterYesNo;
	ps->fs.ExitYesNo = ExitYesNo;
	ps->fs.SetFileName = SetFileName;
	ps->fs.MoveFocus = MoveFocus;
	ps->fs.GuiLocal = GuiLocal;
	ps->fs.FileComplete = FileComplete;
	ps->fs.Select2Y = Select2Y;

	return &ps->fs.state;
}



