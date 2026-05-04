/*
  ファイル選択機能
  ゲームセーブ画面部分差分

  2001/04/10 M.Kobayashi
  $Id: xfileselg.c,v 1.21 2002/09/17 03:58:22 usr03700 Exp $
 */

#include	"gameheader.h"
#include	"xmcman.h"
#include 	"private.h"
#include	"../../kira/2D_action/layout_2d.h"
#include	"../../mode/codec/codecmem.h"
#include	"../../kano/titlescr/subtitle.h"
#include	"../../kano/titlescr/titlescr.h"
#include	"../../kano/mcman/mcman.h"
#include	"font.h"

#define MAX_LEVEL				5

#define MAX_FILE_NORMAL			50
#define MAX_FILE_ST				5
#define MAX_FILE_VR				1

#define SAVE_GAME_RESOURCE		5
#define STAGENAME_RESOURCE		6
#define DIFFICULTY_RESOURCE		7		/* 難易度名 */
//#define LOAD_GAME_RESOURCE		0x00873e66		/* ロードゲームリソース */
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

#define E_STAGENAME_RESOURCE	0x73d521
#define J_STAGENAME_RESOURCE	0x1e51e5
static const int stagename_resource_name[] = {
	E_STAGENAME_RESOURCE,	
	E_STAGENAME_RESOURCE,	
	E_STAGENAME_RESOURCE,	
	E_STAGENAME_RESOURCE,	
	E_STAGENAME_RESOURCE,
	0,
	J_STAGENAME_RESOURCE,
};

#define RES_NEWDATA			2
#define RES_OVERWRITE		3
#define RES_CONFIRM			2

#define TEX_STRCODE	GV_StrCode( "node_font_alp_ovl" )

#define DISPLAY_FILE_MAX		10
// 通常ゲームセーブ・ＶＲ用
#define FONT_UW				(FONT_SIZE_W)
#define FONT_VH				(FONT_SIZE_H + 3)

#define	FONT_DISP_WIDTH		16
#define	FONT_DISP_HEIGHT	18


#define FILENAME_Y		118
#define FILENAME_YSTEP	21
#define FILENAME_H		(FONT_DISP_HEIGHT)

#define CURSOR_Y_OFFSET	3

#define ID_X			56
#define TEXT_X			108
#define DATE_X			240
#define TIME_X			384

#define	ID_W			(FONT_DISP_WIDTH * 3)
#define	TEXT_W			(FONT_DISP_WIDTH * 10)
#define	DATE_W			(FONT_DISP_WIDTH * 10)
#define	TIME_W			(FONT_DISP_WIDTH * 10)

//#define FILENAME_WIDTH	(FONT_DISP_WIDTH * 30)
#define FILENAME_V		( 512 - FONT_VH * DISPLAY_FILE_MAX )
#define FILENAME_VH		FONT_VH

#define ID_U			0
#define TEXT_U			( ID_U + ID_UW )
#define DATE_U			( TEXT_U + TEXT_UW )
#define TIME_U			( DATE_U + DATE_UW )

#define	ID_UW			(FONT_UW * 3)
#define	TEXT_UW			(FONT_UW * 10)
#define	DATE_UW			(FONT_UW * 10)
#define	TIME_UW			(FONT_UW * 10)

// スネークテイルズ用

#define	ST_SUBTITLE_X	TEXT_X
#define	ST_CLEARFLAG_X	TIME_X

#define ST_SUBTITLE_W	(FONT_DISP_WIDTH * 25)
#define ST_CLEARFLAG_W	(FONT_DISP_WIDTH * 6)

#define ST_SUBTITLE_U	TEXT_U
#define ST_CLEARFLAG_U	(ST_SUBTITLE_U + ST_SUBTITLE_UW)

#define ST_SUBTITLE_UW	(FONT_UW * 25)
#define ST_CLEARFLAG_UW	(FONT_UW * 6 )




#define STAGE_X			56
#define STAGE_Y			92
#define	STAGE_W			(FONT_DISP_WIDTH * 30)
#define	STAGE_H			FONT_DISP_HEIGHT
#define STAGE_U			0
#define STAGE_V			( 512 - FONT_VH * (DISPLAY_FILE_MAX + 2) )
#define STAGE_UW		(FONT_UW * 30)
#define STAGE_VH		FONT_VH

#define YES_X			386
#define YES_Y			(118 + 21 * 9 + 4 + 5)
#define SLASH_X			434
#define SLASH_Y			YES_Y
#define NO_X			450
#define NO_Y			YES_Y
#define PAGE_X			324
#define PAGE_Y			72
#define	PAGENUM_X		429
#define	PAGENUM_Y		PAGE_Y

#define PAGESLASH_X		429

#define PAGE_W			14
#define PAGE_H			10
#define PAGENUM_W		18
#define PAGENUM_H		14


#define OVER_WRITE_X	56
#define OVER_WRITE_Y	(118 + 21 * 9 + 4)
#define OVER_WRITE_U	0
#define OVER_WRITE_V	0
#define OVER_WRITE_UW	(SUBTITLE_FONT_WIDTH * 30)
#define OVER_WRITE_VH 	(SUBTITLE_FONT_HEIGHT * 5 / 4)
#define OVER_WRITE_W	( OVER_WRITE_UW * SUBTITLE_FONT_DISP_WIDTH / SUBTITLE_FONT_WIDTH )
#define OVER_WRITE_H 	( OVER_WRITE_VH * SUBTITLE_FONT_DISP_HEIGHT / SUBTITLE_FONT_HEIGHT )



#define STR_COLOR			0x80555a50
#define STR_COLOR_CLEARED	0x80101080
#define NOFOCUS_A		0x30

static short Select2Y( int sel ) {
	return ((sel) % DISPLAY_FILE_MAX) * FILENAME_YSTEP + FILENAME_Y;
}

enum {	// 内部使用gui_state
	GUI_STATE_PAGE_FADEOUT = MCX_GUI_STATE_SYSTEMMAX,
	GUI_STATE_YES_NO,
};

static const char* tales_title_name[] = {
	MCMAN_TALES_TITLE_A,
	MCMAN_TALES_TITLE_B,
	MCMAN_TALES_TITLE_C,
	MCMAN_TALES_TITLE_D,
	MCMAN_TALES_TITLE_E,
};


static char*  str_table[] = {
//	 1---5----0----5----0----5
	"YES",					// 3	
	"/",					// 1
	"NO",					// 2
	"PAGE",					// 4
	"00/00",				// 5
};

#define STR_TOTAL_LEN	( 3 + 1 + 2 + 4 + 5)

#define FLAG_NO_NEWDATA		0x01		// newdata 表示なし
#define FLAG_OVERWRITE		0x02		// overwrite

typedef struct MCX_FILE_SELECT_GAME {
	MCX_FILE_SELECT	fs;
	MCX_MENU_STRINGS	mstr_filename[ DISPLAY_FILE_MAX ][ 4 ];
	MCX_MENU_STRINGS	mstr_stagename[ 2 ];	// ダブルバッファにする
	int	stagename_buf;
	int	newfile_id;

	MCX_INTERP		intrp_filename_y;			// ファイル名補間
} MCX_FILE_SELECT_GAME;


//// ローカル使用関数
static void WriteStageName( MCX_MAN* pm, MCX_FILE_SELECT_GAME* ps )
{	// ステージ名をプリント
	char* pstr;
	char vrname[MCMAN_VR_NAME_MAX * 2];
	
	MCX_MStringsHide( &ps->mstr_stagename[ ps->stagename_buf ], T_SHORT );
	
	if( MCX_GetFileInfo( ps->fs.cursel )->flag & MCX_FILEPROP_FLAG_DAMAGED ) {
		pstr = "Damaged game";
	} else if( pm->flag & MCX_MAN_FLAG_VR ) {
		pstr = vrname;
		GV_ZeroMemory( vrname, sizeof(vrname));
		if( ps->fs.cursel == pm->nData ) {
			strcpy(vrname, "NEW DATA:");
			memcpy( vrname + strlen( vrname ), GM_MyName,
					MCMAN_VR_NAME_MAX );
		} else {
			strcpy(vrname, "Missions Data:");
			memcpy( vrname + strlen( vrname ), MCX_GetFileInfo( ps->fs.cursel )->info.vrinfo.name,
					MCMAN_VR_NAME_MAX );
		}
	} else {
		if( ps->fs.cursel == pm->nData ) {
			pstr = "NEW DATA";
		} else {
			MCMAN_INFODATA* pinfo;
			pinfo = &MCX_GetFileInfo( ps->fs.cursel )->info.gameinfo;
			if( pinfo->stage_num >= 0 && pinfo->stage_num < MC_N_STAGES ) {
				if( pm->flag & MCX_MAN_FLAG_SAVE ) {
					pstr =  (char*)GM_GetResource( STAGENAME_RESOURCE, pinfo->stage_num );
				} else {	// リソース取得の仕方が違う。。。
					pstr =  (char*)GetLocalResource( stagename_resource_name[GM_Language-GM_LANG_ENGLISH],
													 pinfo->stage_num );
				}
			} else {
				pstr = "NO STAGENAME";
			}
		}
	}
	ps->stagename_buf ^= 1;
	MCX_MStringsClearLine( &ps->mstr_stagename[ ps->stagename_buf ] );
	MCX_MStringsPrintf( &ps->mstr_stagename[ ps->stagename_buf ], "%s", pstr );
	MCX_MStringsShow( &ps->mstr_stagename[ ps->stagename_buf ], T_SHORT );
}

static void PrintFileName( MCX_MAN* pm, MCX_MENU_STRINGS* pmstr, MCX_FILEPROP* pFile )
{
	MCX_FILE_INFO* pinfo;
	pinfo = &pFile->info;
	
	MCX_MStringsClearLine( pmstr );
	// ID
	MCX_MStringsPrintf( pmstr , "%02d",
						pFile->id );
	// TEXT
	{
		if( pFile->flag & MCX_FILEPROP_FLAG_DAMAGED ) {
			MCX_MStringsPrintf( pmstr + 1, "Damaged game" );
		} else if( pm->flag & MCX_MAN_FLAG_SNAKE_T ) {
//			MCX_MStringsPrintf( pmstr + 1, "Tales %c", 'A' + pinfo->snake_tales_no - 1 );
			if( pinfo->gameinfo.snake_tales_no >= 1 && pinfo->gameinfo.snake_tales_no <= MCMAN_MAX_TALES ) {
				MCX_MStringsPrintf( pmstr + 1, "%s", tales_title_name[ pinfo->gameinfo.snake_tales_no - 1 ] );
			} else {
				MCX_MStringsPrintf( pmstr + 1, "?" );
			}
		} else if( pm->flag & MCX_MAN_FLAG_VR ) {
			MCX_MStringsPrintf( pmstr + 1, "%3.1f%%", pinfo->vrinfo.achieve_high + 0.1f * pinfo->vrinfo.achieve_low );
		} else {
			char* pStr;
			int diff;
			
			diff = (pinfo->gameinfo.difficulty - GM_LEVEL_VERYEASY ) / 10;
			if( diff >= 0 && diff <= MAX_LEVEL ) {
				pStr = (char*)GM_GetResource( DIFFICULTY_RESOURCE, diff );
			} else {
				pStr = "NO LEVEL";
			}
			MCX_MStringsPrintf( pmstr + 1, "%s", pStr );
		}
	}
	// DATE
	if( !(pFile->flag & MCX_FILEPROP_FLAG_DAMAGED) ) {
		if( pm->flag & MCX_MAN_FLAG_SNAKE_T ) {
			// クリアフラグ表示
			int i;
			int n;
			char str[ MCMAN_MAX_TALES + 1 ];
			for( i = 0, n = 0 ; i < MCMAN_MAX_TALES ; i ++ ) {
				if( pinfo->gameinfo.st_clear_flag & ( 1 << i ) ) {
					str[ n++ ] = 'A' + i;
				} 
			}
			str[ n ] = '\0';
			MCX_MStringsPrintf( pmstr + 2,
								str );
			
		} else {
			FILETIME	localtime;
			SYSTEMTIME	systime;
			
			FileTimeToLocalFileTime( &pFile->finddata.ftLastWriteTime, &localtime );
			FileTimeToSystemTime( &localtime, &systime );

			MCX_MStringsPrintf( pmstr + 2,
								"%04d %02d %02d", systime.wYear, systime.wMonth, systime.wDay );
		}
	}
	// TIME
	if( !(pFile->flag & MCX_FILEPROP_FLAG_DAMAGED) ) {
		if( pm->flag & MCX_MAN_FLAG_GAME ) {
			int ptime = pinfo->gameinfo.playtime / FRAMES_PER_SEC;
			if( ptime >= 60*60*10000) ptime = 60*60*10000 - 1;
			if( ptime < 0 ) ptime = 0;
			
			MCX_MStringsPrintf( pmstr + 3,
								"%04d:%02d:%02d", ptime / 3600, (ptime / 60) % 60,
								ptime % 60 );
		} else if ( pm->flag & MCX_MAN_FLAG_VR ) {
			int ptime = pinfo->vrinfo.playtime / FRAMES_PER_SEC;
			if( ptime >= 60*60*10000) ptime = 60*60*10000 - 1;
			if( ptime < 0 ) ptime = 0;
			
			MCX_MStringsPrintf( pmstr + 3,
								"%04d:%02d:%02d", ptime / 3600, (ptime / 60) % 60,
								ptime % 60 );
		}
	}

	if( (pm->flag & MCX_MAN_FLAG_GAME )){	// クリアしていたら色を変える
		int col;
		int i;
		if( pinfo->gameinfo.clear_count > 0 ) {
			col = STR_COLOR_CLEARED;
		} else {
			col = STR_COLOR;
		}
		for( i = 0 ; i < 4; i++ ) {
			(pmstr + i)->col = col;
		}
	}
}

//// メンバ関数
static void ShowCurrentPage( MCX_MAN* pm, MCX_FILE_SELECT_GAME* ps )
{	// 現在のページを制作して表示
	int i;
	int start_file = DISPLAY_FILE_MAX * (ps->fs.curpage - 1);
	for( i = start_file ; i < pm->nData && i < start_file + DISPLAY_FILE_MAX; i++ ) {
		PrintFileName( pm, ps->mstr_filename[ i - start_file ], MCX_GetFileInfo( i ) );
		if( ps->fs.cursel == i ) {
			MCX_MStringsShow( &ps->mstr_filename[ i - start_file ][ 0 ] , T_SHORT );
			MCX_MStringsShow( &ps->mstr_filename[ i - start_file ][ 1 ] , T_SHORT );
			MCX_MStringsShow( &ps->mstr_filename[ i - start_file ][ 2 ] , T_SHORT );
			MCX_MStringsShow( &ps->mstr_filename[ i - start_file ][ 3 ] , T_SHORT );
		} else {
			MCX_MStringsNofocus( &ps->mstr_filename[ i - start_file ][ 0 ] , T_SHORT );
			MCX_MStringsNofocus( &ps->mstr_filename[ i - start_file ][ 1 ] , T_SHORT );
			MCX_MStringsNofocus( &ps->mstr_filename[ i - start_file ][ 2 ] , T_SHORT );
			MCX_MStringsNofocus( &ps->mstr_filename[ i - start_file ][ 3 ] , T_SHORT );
		}
	}
	if( ps->fs.curpage == ps->fs.maxpage ) {	// 新規データ
		if( !(ps->fs.flag & FLAG_NO_NEWDATA )) {
			MCX_MStringsClearLine( &ps->mstr_filename[ i - start_file ] [ 0 ] );
			MCX_MStringsPrintf( &ps->mstr_filename[ i - start_file ][ 0 ], "%02d", ps->newfile_id );
			MCX_MStringsPrintf( &ps->mstr_filename[ i - start_file ][ 1 ], "NEW DATA" );
			ps->mstr_filename[ i - start_file ][ 0 ].col = STR_COLOR;
			ps->mstr_filename[ i - start_file ][ 1 ].col = STR_COLOR;

			if( ps->fs.cursel == i ) {
				MCX_MStringsShow( &ps->mstr_filename[ i - start_file ][ 0 ] , T_SHORT );
				MCX_MStringsShow( &ps->mstr_filename[ i - start_file ][ 1 ] , T_SHORT );
				MCX_MStringsShow( &ps->mstr_filename[ i - start_file ][ 2 ] , T_SHORT );
				MCX_MStringsShow( &ps->mstr_filename[ i - start_file ][ 3 ] , T_SHORT );
			} else {
				MCX_MStringsNofocus( &ps->mstr_filename[ i - start_file ][ 0 ] , T_SHORT );
				MCX_MStringsNofocus( &ps->mstr_filename[ i - start_file ][ 1 ] , T_SHORT );
				MCX_MStringsNofocus( &ps->mstr_filename[ i - start_file ][ 2 ] , T_SHORT );
				MCX_MStringsNofocus( &ps->mstr_filename[ i - start_file ][ 3 ] , T_SHORT );
			}
		}
	}

	// ステージ名表示
	WriteStageName( pm, ps );
	
	// ページ番号を表示
	MCX_StringsPrintf(&ps->fs.str_pagenum, "%02d/%02d", ps->fs.curpage, ps->fs.maxpage );
	{	// '/' の位置が合うように調整
		int diff;
		diff = PAGESLASH_X - (int)(ps->fs.str_pagenum.ppObj[ 2 ])->sprite.pos.x;
		for ( i = 0 ; i < ps->fs.str_pagenum.len ; i ++ ){
			(ps->fs.str_pagenum.ppObj[ i ])->sprite.pos.x += diff;
		}
	}
	if( ps->fs.maxpage > 1 ) {
		MCX_StringsShow( &ps->fs.str_pagenum, T_LONG);
	}
	
	ps->fs.pIntrpJudge = &ps->mstr_filename[ 0 ][ 0 ].intrp_alpha;
}

static void HideCurrentPage( MCX_MAN* pm, MCX_FILE_SELECT_GAME* ps )
{
	int i;
	for( i = 0 ; i < DISPLAY_FILE_MAX ; i++ ) {
		MCX_MStringsHide( &ps->mstr_filename[ i ][ 0 ] , T_SHORT );
		MCX_MStringsHide( &ps->mstr_filename[ i ][ 1 ] , T_SHORT );
		MCX_MStringsHide( &ps->mstr_filename[ i ][ 2 ] , T_SHORT );
		MCX_MStringsHide( &ps->mstr_filename[ i ][ 3 ] , T_SHORT );
	}
	ps->fs.pIntrpJudge = &ps->mstr_filename[ 0 ][ 0 ].intrp_alpha;
}

static void GuiLocal( MCX_MAN* pm, MCX_FILE_SELECT_GAME* ps )
{	// ＧＵＩ非共通部分
	int i;
	// カーソル
	SPR_SHOW( ps->fs.pSprcursor[ 0 ] );
	SPR_SHOW( ps->fs.pSprcursor[ 1 ] );
	ps->fs.pSprcursor[ 0 ]->empty.pos.y = ps->fs.intrp_cursor_y.fCurrent + CURSOR_Y_OFFSET;
	ps->fs.pSprcursor[ 1 ]->sprite.col.a = (u_char)ps->fs.intrp_cursor_a.fCurrent;

	// MENU_STR 位置変更
	for ( i = 0 ; i < DISPLAY_FILE_MAX ; i ++ ) {
		short y1 = (short)((FILENAME_Y + FILENAME_YSTEP * i) * ( 1.f - ps->intrp_filename_y.fCurrent )
						   + ps->intrp_filename_y.fCurrent * ( FILENAME_Y + FILENAME_YSTEP * 3 ));
		ps->mstr_filename[ i ][ 0 ].y1 = y1;
		ps->mstr_filename[ i ][ 1 ].y1 = y1;
		ps->mstr_filename[ i ][ 2 ].y1 = y1;
		ps->mstr_filename[ i ][ 3 ].y1 = y1;
	}
}

static void EnterYesNo( MCX_MAN* pm, MCX_FILE_SELECT_GAME* ps )
{
	int i;
	for ( i = 0 ; i < DISPLAY_FILE_MAX; i++ ) {
		if( i != (ps->fs.cursel % DISPLAY_FILE_MAX ) ) {
			MCX_MStringsHide( &ps->mstr_filename[ i ][ 0 ], T_SHORT );
			MCX_MStringsHide( &ps->mstr_filename[ i ][ 1 ], T_SHORT );
			MCX_MStringsHide( &ps->mstr_filename[ i ][ 2 ], T_SHORT );
			MCX_MStringsHide( &ps->mstr_filename[ i ][ 3 ], T_SHORT );
		}
	}
	ps->intrp_filename_y.target = 1;
	ps->intrp_filename_y.ctr = T_SHORT;

	ps->fs.intrp_cursor_a.target = 0;
	ps->fs.intrp_cursor_a.ctr = T_SHORT;
	
	ps->fs.pIntrpJudge = &ps->intrp_filename_y;
}

static void ExitYesNo( MCX_MAN* pm, MCX_FILE_SELECT_GAME* ps )
{
	int i;
	int max = (ps->fs.curpage == ps->fs.maxpage ) ? (ps->fs.nselection % DISPLAY_FILE_MAX) : DISPLAY_FILE_MAX;
	if( max == 0 ) max = DISPLAY_FILE_MAX;
	for ( i = 0 ; i < max; i++ ) {
		if( i != (ps->fs.cursel % DISPLAY_FILE_MAX ) ) {
			MCX_MStringsNofocus( &ps->mstr_filename[ i ][ 0 ], T_SHORT );
			MCX_MStringsNofocus( &ps->mstr_filename[ i ][ 1 ], T_SHORT );
			MCX_MStringsNofocus( &ps->mstr_filename[ i ][ 2 ], T_SHORT );
			MCX_MStringsNofocus( &ps->mstr_filename[ i ][ 3 ], T_SHORT );
		}
	}
	ps->intrp_filename_y.target = 0;
	ps->intrp_filename_y.ctr = T_SHORT;

	ps->fs.intrp_cursor_a.target = 0x80;
	ps->fs.intrp_cursor_a.ctr = T_SHORT;
	
	ps->fs.pIntrpJudge = &ps->intrp_filename_y;
}

static void SetFileName( MCX_MAN* pm, MCX_FILE_SELECT_GAME* ps )
{	// セーブする直前に内部使用ファイル名（ファイル情報）を決める
	pm->pCurfile = MCX_GetFileInfo( ps->fs.cursel );
	if( pm->flag & MCX_MAN_FLAG_SAVE ) {
		ZeroMemory( &pm->pCurfile->info, sizeof( MCX_FILE_INFO ) );
		if( pm->flag & MCX_MAN_FLAG_VR ) {
			SaveVRInfo( &pm->pCurfile->info.vrinfo );
		} else {
			SaveGameInfo( &pm->pCurfile->info.gameinfo );
		}
		// 拡張されたデータ部分
		if( pm->flag & MCX_MAN_FLAG_SNAKE_T ) {
			int i;
			
			pm->pCurfile->info.gameinfo.snake_tales_no
					= GCL_ReadVarRef( &TTL_ReferenceVariable[0], 0 );
			pm->pCurfile->info.gameinfo.st_clear_flag = 0;
			for( i = 0 ; i < MCMAN_MAX_TALES ; i++ ) {
				pm->pCurfile->info.gameinfo.st_clear_flag |=
						GCL_ReadVarRef( &TTL_ReferenceVariable[1 + i], 0 ) ?
						( 1 << i ) : 0;
			}
		} 
		if( !(ps->fs.flag & FLAG_OVERWRITE) ) {
			pm->nData++;
			// 新規セーブ・ファイル名を作成
			MCX_SetFileName( pm->pCurfile, ps->newfile_id );
		} else {
			MCX_SetFileName( pm->pCurfile, pm->pCurfile->id);
		}

	}
}

static void FileComplete( MCX_MAN* pm, MCX_FILE_SELECT_GAME* ps )
{	// ファイル操作が成功したときに呼ばれる
	if( pm->flag & MCX_MAN_FLAG_SAVE ) {
		// 必要なファイル情報を作成
		SYSTEMTIME systime;

		GetSystemTime( &systime );
		SystemTimeToFileTime( &systime, &pm->pCurfile->finddata.ftLastWriteTime );
		
		PrintFileName( pm, ps->mstr_filename[ ps->fs.cursel % DISPLAY_FILE_MAX ], pm->pCurfile );
		WriteStageName( pm, ps );
	}
	if( ps->fs.flag & FLAG_OVERWRITE ) {
		if( pm->flag & MCX_MAN_FLAG_GAME ) {
			MCX_SetLoaddataID( MCMAN_FILE_KIND_GAME, pm->pCurfile->id );
		} else if( pm->flag & MCX_MAN_FLAG_SNAKE_T ) {
			MCX_SetLoaddataID( MCMAN_FILE_KIND_SNAKE_TALES, pm->pCurfile->id );
		}
	} else {
		MCX_ResetLoaddataID();
	}
}

static void MoveFocus( MCX_MAN* pm, MCX_FILE_SELECT_GAME* ps, int prevsel )
{
	int i;
	for( i = 0 ; i < 4 ; i++ ) {
		MCX_MStringsShow( &ps->mstr_filename[ ps->fs.cursel % DISPLAY_FILE_MAX ][ i ], T_SHORT );
		MCX_MStringsNofocus( &ps->mstr_filename[ prevsel % DISPLAY_FILE_MAX ][ i ], T_SHORT );
		
	}
	WriteStageName( pm, ps );
}

static void ClearStart( MCX_MAN* pm, MCX_FILE_SELECT_GAME* ps )
{	// フェードアウト開始
	int i;
	for ( i = 0 ; i < DISPLAY_FILE_MAX ; i++ ) {
		MCX_MStringsHide( &ps->mstr_filename[ i ][ 0 ], T_SHORT );
		MCX_MStringsHide( &ps->mstr_filename[ i ][ 1 ], T_SHORT );
		MCX_MStringsHide( &ps->mstr_filename[ i ][ 2 ], T_SHORT );
		MCX_MStringsHide( &ps->mstr_filename[ i ][ 3 ], T_SHORT );
	}
	MCX_StringsHide( &ps->fs.str_yes, T_SHORT );
	MCX_StringsHide( &ps->fs.str_slash, T_SHORT );
	MCX_StringsHide( &ps->fs.str_no, T_SHORT );
	MCX_StringsHide( &ps->fs.str_page, T_SHORT );
	MCX_StringsHide( &ps->fs.str_pagenum, T_SHORT );

	MCX_MStringsHide( &ps->mstr_stagename[ 0 ], T_SHORT );
	MCX_MStringsHide( &ps->mstr_stagename[ 1 ], T_SHORT );

	MCX_MStringsHide( &ps->fs.mstr_create, T_SHORT );
	MCX_MStringsHide( &ps->fs.mstr_overwrite, T_SHORT );

	if( ( pm->flag & MCX_MAN_FLAG_DOGTAG )
		&& pm->result == SAVEGAME_ANS_SUCCESS ) {
		L2D_EvokeAction( pm->l2d_handle, CODE_TODOGTAG );
	} else {
		L2D_EvokeAction( pm->l2d_handle, CODE_CLOSEALL );
		SE_WINCLOSE();
	}
	
	ps->fs.intrp_cursor_a.target = 0;
	ps->fs.intrp_cursor_a.ctr = T_SHORT ;
}
	
static void Enter( MCX_MAN* pm, MCX_FILE_SELECT_GAME* ps )
{	//
	int i;
	
	// 文字準備
	ps->fs.ppobjbuf = (SPR_OBJ**)MCX_Malloc( sizeof(SPR_OBJ*) * STR_TOTAL_LEN );
	
	MCX_StringsInitBuffer( ps->fs.ppobjbuf, STR_TOTAL_LEN, pm->font_tri_handle,
						   TEX_STRCODE );
	for( i = 0 ; i < DISPLAY_FILE_MAX ; i++ ) {
		// ID
		u_int col;
		if( pm->flag & MCX_MAN_FLAG_VR ) {
			col = 0;
		} else {
			col = STR_COLOR;
		}
		MCX_MStringsCreate( &ps->mstr_filename[ i ][ 0 ], "00",
							ID_X, FILENAME_Y + FILENAME_YSTEP * i, ID_W, FILENAME_H, 
							ID_U, FILENAME_V + FILENAME_VH * i, ID_UW, FILENAME_VH, col );
		if( pm->flag & MCX_MAN_FLAG_SNAKE_T ) {
			// SUBTITLE
			MCX_MStringsCreate( &ps->mstr_filename[ i ][ 1 ], "--------------------",
								ST_SUBTITLE_X, FILENAME_Y + FILENAME_YSTEP * i, ST_SUBTITLE_W, FILENAME_H,
								ST_SUBTITLE_U, FILENAME_V + FILENAME_VH * i, ST_SUBTITLE_UW, FILENAME_VH, STR_COLOR );
			// CLEARFLAG
			MCX_MStringsCreate( &ps->mstr_filename[ i ][ 2 ], "-----",
								ST_CLEARFLAG_X, FILENAME_Y + FILENAME_YSTEP * i, ST_CLEARFLAG_W, FILENAME_H,
								ST_CLEARFLAG_U, FILENAME_V + FILENAME_VH * i, ST_CLEARFLAG_UW, FILENAME_VH, STR_COLOR );
			// DUMMY
			MCX_MStringsCreate( &ps->mstr_filename[ i ][ 3 ], " ",
								TIME_X, FILENAME_Y + FILENAME_YSTEP * i, 0, 0,
								TIME_U, FILENAME_V + FILENAME_VH * i, TIME_UW, FILENAME_VH, STR_COLOR );
		} else {
			// TEXT
			MCX_MStringsCreate( &ps->mstr_filename[ i ][ 1 ], "----------",
								TEXT_X, FILENAME_Y + FILENAME_YSTEP * i, TEXT_W, FILENAME_H,
								TEXT_U, FILENAME_V + FILENAME_VH * i, TEXT_UW, FILENAME_VH, STR_COLOR );
			// DATE
			MCX_MStringsCreate( &ps->mstr_filename[ i ][ 2 ], "2000 01 01",
								DATE_X, FILENAME_Y + FILENAME_YSTEP * i, DATE_W, FILENAME_H,
								DATE_U, FILENAME_V + FILENAME_VH * i, DATE_UW, FILENAME_VH, STR_COLOR );
			// TIME
			MCX_MStringsCreate( &ps->mstr_filename[ i ][ 3 ], "0000:00:00",
								TIME_X, FILENAME_Y + FILENAME_YSTEP * i, TIME_W, FILENAME_H,
								TIME_U, FILENAME_V + FILENAME_VH * i, TIME_UW, FILENAME_VH, STR_COLOR );
		}
	}
	for( i = 0 ; i < 2 ; i++ ) {
		MCX_MStringsCreate( &ps->mstr_stagename[ i ], "-------------------",
							STAGE_X, STAGE_Y, STAGE_W, STAGE_H,
							STAGE_U, STAGE_V + STAGE_VH * i, STAGE_UW, STAGE_VH, STR_COLOR );
	}
	ps->stagename_buf = 0;
	
	MCX_StringsCreate( &ps->fs.str_yes, str_table[ 0 ], YES_X, YES_Y, STR_COLOR, NOFOCUS_A );
	MCX_StringsCreate( &ps->fs.str_slash, str_table[ 1 ], SLASH_X, SLASH_Y, STR_COLOR, NOFOCUS_A );
	MCX_StringsCreate( &ps->fs.str_no, str_table[ 2 ], NO_X, NO_Y, STR_COLOR, NOFOCUS_A );
	MCX_StringsCreate( &ps->fs.str_page, str_table[ 3 ], PAGE_X, PAGE_Y, STR_COLOR, NOFOCUS_A );
	MCX_StringsCreate( &ps->fs.str_pagenum, str_table[ 4 ], PAGENUM_X, PAGENUM_Y, STR_COLOR, NOFOCUS_A );

	MCX_MStringsCreate( &ps->fs.mstr_create, GM_GetResource( SAVE_GAME_RESOURCE, RES_NEWDATA ),
						OVER_WRITE_X, OVER_WRITE_Y, OVER_WRITE_W, OVER_WRITE_H,
						OVER_WRITE_U, OVER_WRITE_V, OVER_WRITE_UW, OVER_WRITE_VH, STR_COLOR );
	if( pm->flag & MCX_MAN_FLAG_SAVE ) {
		MCX_MStringsCreate( &ps->fs.mstr_overwrite, GM_GetResource( SAVE_GAME_RESOURCE, RES_OVERWRITE ),
							OVER_WRITE_X, OVER_WRITE_Y, OVER_WRITE_W, OVER_WRITE_H,
							OVER_WRITE_U, OVER_WRITE_V + OVER_WRITE_VH,
							OVER_WRITE_UW, OVER_WRITE_VH, STR_COLOR );
	} else {
		// リソース取得の関数が違う。。。
		MCX_MStringsCreate( &ps->fs.mstr_overwrite, GetLocalResource( LOAD_GAME_RESOURCE, RES_CONFIRM ),
							OVER_WRITE_X, OVER_WRITE_Y, OVER_WRITE_W, OVER_WRITE_H,
							OVER_WRITE_U, OVER_WRITE_V + OVER_WRITE_VH,
							OVER_WRITE_UW, OVER_WRITE_VH, STR_COLOR );
	}
	// str_page の大きさを変える
	(*ps->fs.str_page.ppObj)->sprite.dw = PAGE_W;
	(*ps->fs.str_page.ppObj)->sprite.dh = PAGE_H;
	MCX_StringsPrintf( &ps->fs.str_page, "%s", str_table[ 3 ] );
	// str_pagenum の大きさを変える
	(*ps->fs.str_pagenum.ppObj)->sprite.dw = PAGENUM_W;
	(*ps->fs.str_pagenum.ppObj)->sprite.dh = PAGENUM_H;

	ps->fs.flag = 0;

	// ページ数計算
	if( (pm->flag & MCX_MAN_FLAG_SAVE) &&
		!(pm->flag & ( MCX_MAN_FLAG_NOSPACE | MCX_MAN_FLAG_MANYDIRS ) ) ) {
		int maxdata;
		if( pm->flag & MCX_MAN_FLAG_GAME ) {
			maxdata = MAX_FILE_NORMAL;
		} else if( pm->flag & MCX_MAN_FLAG_SNAKE_T ) {
			maxdata = MAX_FILE_ST;
		} else if( pm->flag & MCX_MAN_FLAG_VR ) {
			maxdata = MAX_FILE_VR;
		}
		if( pm->nData < maxdata ) {
			ps->fs.maxpage = pm->nData / DISPLAY_FILE_MAX + 1;
		} else {
			ps->fs.maxpage = (pm->nData - 1) / DISPLAY_FILE_MAX + 1; // nDataは 1 以上（そうでないものは前ステージでキャンセル）
			ps->fs.flag |= FLAG_NO_NEWDATA;
		}
	} else {
		ps->fs.maxpage = (pm->nData - 1) / DISPLAY_FILE_MAX + 1; // nDataは 1 以上（そうでないものは前ステージでキャンセル）
		ps->fs.flag |= FLAG_NO_NEWDATA;
	}
	
	if( ps->fs.maxpage > 1 ) {
		MCX_StringsShow( &ps->fs.str_page , T_LONG );
	}
	
	ps->fs.nselection = (ps->fs.flag & FLAG_NO_NEWDATA) ? pm->nData : pm->nData + 1;
	ps->fs.cursel = ps->fs.nselection - 1;
	
	if( pm->flag & MCX_MAN_FLAG_SAVE ) {
		// セーブ時
		// 前に save/load complete したファイルがあればそこにフォーカス
		// なければ NEW FILE
		int id = -1;
		if( pm->flag & MCX_MAN_FLAG_GAME ) {
			id = MCX_GetLoaddataID( MCMAN_FILE_KIND_GAME );
		} else if( pm->flag & MCX_MAN_FLAG_SNAKE_T ) {
			id = MCX_GetLoaddataID( MCMAN_FILE_KIND_SNAKE_TALES );
		}
		for( i = 0 ; i < pm->nData; i++ ) {
			if( pm->pFiles[ i ].id == id ) {
				ps->fs.cursel = i;
				break;
			}
		}
	} else {
		ps->fs.cursel = pm->latestfile;
	}
	ps->fs.curpage = (ps->fs.cursel / DISPLAY_FILE_MAX) + 1;

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
	
	ShowCurrentPage( pm, ps );
	L2D_EvokeAction( pm->l2d_handle, CODE_OPENSELFILE1 );

	// カーソル準備
	ps->fs.pSprcursor[ 0 ] = L2D_GetObject( pm->l2d_handle, CODE_SELCURSNULL );
	ps->fs.pSprcursor[ 1 ] = L2D_GetObject( pm->l2d_handle, CODE_SELCURS );
	SPR_SHOW( ps->fs.pSprcursor[ 0 ] );
	SPR_SHOW( ps->fs.pSprcursor[ 1 ] );
	ps->fs.pSprcursor[ 1 ]->sprite.col.a = 0;
	ps->fs.intrp_cursor_y.fCurrent = (float) Select2Y( ps->fs.cursel );
	ps->fs.intrp_cursor_y.ctr = 0;
	ps->fs.intrp_cursor_a.fCurrent = 0.f;
	ps->fs.intrp_cursor_a.target = 0x80;
	ps->fs.intrp_cursor_a.ctr = T_LONG;
	MCX_AddInterpService( &ps->fs.intrp_cursor_y );
	MCX_AddInterpService( &ps->fs.intrp_cursor_a );

	// ファイル名位置補間
	ps->intrp_filename_y.fCurrent = 0.f;
	ps->intrp_filename_y.ctr = 0;
	ps->intrp_filename_y.target = 0;
	MCX_AddInterpService( &ps->intrp_filename_y );

	MCX_SetAutoRepeat( 10 );
}

static void Exit( MCX_MAN* pm, MCX_FILE_SELECT_GAME* ps )
{	//
	int i;
	for ( i = 0 ; i < DISPLAY_FILE_MAX ; i++ ) {
		MCX_MStringsDelete( &ps->mstr_filename[ i ][ 0 ] );
		MCX_MStringsDelete( &ps->mstr_filename[ i ][ 1 ] );
		MCX_MStringsDelete( &ps->mstr_filename[ i ][ 2 ] );
		MCX_MStringsDelete( &ps->mstr_filename[ i ][ 3 ] );
	}
	MCX_MStringsDelete( &ps->mstr_stagename[ 0 ] );
	MCX_MStringsDelete( &ps->mstr_stagename[ 1 ] );
	MCX_StringsDelete( &ps->fs.str_yes );
	MCX_StringsDelete( &ps->fs.str_slash );
	MCX_StringsDelete( &ps->fs.str_no );
	MCX_StringsDelete( &ps->fs.str_page );
	MCX_StringsDelete( &ps->fs.str_pagenum );

	MCX_MStringsDelete( &ps->fs.mstr_create );
	MCX_MStringsDelete( &ps->fs.mstr_overwrite );

	MCX_DeleteInterp( &ps->fs.intrp_cursor_y );
	MCX_DeleteInterp( &ps->fs.intrp_cursor_a );
	MCX_DeleteInterp( &ps->intrp_filename_y );

	MCX_Free( ps->fs.ppobjbuf );
}

MCX_STATE*	MCX_GetFileSelectGame( void )
{
	MCX_FILE_SELECT_GAME* ps;

	ps = (MCX_FILE_SELECT_GAME *)MCX_Malloc( sizeof( MCX_FILE_SELECT_GAME ) );

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



