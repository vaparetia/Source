/*
  写真ネームエントリ＆色変更

  2001/04/10 M.Kobayashi
  $Id: xphotoedit.c,v 1.4 2002/11/23 12:42:25 Yoshizawa1 Exp $
 */

#ifdef KP_XBOX //BP

#include	"gameheader.h"
#include	"xmcman.h"
#include 	"private.h"
#include	"../../kira/2D_action/layout_2d.h"
#include	"../../mode/codec/codecmem.h"
#include	"../../mode/codec/codec_signal.h"
#include	"../../kano/titlescr/titlescr.h"
#include	"../../kano/titlescr/subtitle.h"
#include	"../../kano/resman/resman.h"
#undef SE_START
#include	"../../skoba/test/etc.h"

#define CODE_OPENEDIT (504421)
#define CODE_CLOSEEDIT (14326336)
#define CODE_SELECTTITLE (8757611)
#define CODE_SELECTCOLOR (7897559)
#define CODE_SELECTEXIT (2942896)
#define CODE_OBJ_MARUBATSU (12637854)
#define CODE_OBJ_MARUBATSU_RECT (11297303)
#define CODE_OBJ_MARUBATSU_RGB (1925804)
#define CODE_OBJ_TOP_MARUBATSU (12747845)


#define NAME_X	33
#define NAME_Y	316
#define NAME_W	18
#define NAME_H	14

#define YES_X	390
#define YES_Y	48
#define SLASH_X	(YES_X + 16 * 3)
#define SLASH_Y	YES_Y
#define NO_X	(YES_X + 16 * 4)
#define NO_Y	YES_Y

#define RGBNUM_H	20
#define RGBNUM_X	210
#define RGBNUM_Y	245

#define STR_COLOR		0x80555a50
#define NOFOCUS_A		0x30

#define OVER_WRITE_X	31
#define OVER_WRITE_Y	33
#define OVER_WRITE_U	0
#define OVER_WRITE_V	(SUBTITLE_FONT_HEIGHT * 15/4)
#define OVER_WRITE_UW	(SUBTITLE_FONT_WIDTH * 30)
#define OVER_WRITE_VH 	(SUBTITLE_FONT_HEIGHT * 5/4)
#define OVER_WRITE_W	( OVER_WRITE_UW * SUBTITLE_FONT_DISP_WIDTH / SUBTITLE_FONT_WIDTH )
#define OVER_WRITE_H 	( OVER_WRITE_VH * SUBTITLE_FONT_DISP_HEIGHT / SUBTITLE_FONT_HEIGHT )

#define SAVE_X			OVER_WRITE_X
#define SAVE_Y			OVER_WRITE_Y
#define SAVE_U			0
#define SAVE_V			(OVER_WRITE_V + OVER_WRITE_VH)
#define SAVE_UW			(SUBTITLE_FONT_WIDTH * 30)
#define SAVE_VH 		(SUBTITLE_FONT_HEIGHT * 5 / 4)
#define SAVE_W			( SAVE_UW * SUBTITLE_FONT_DISP_WIDTH / SUBTITLE_FONT_WIDTH )
#define SAVE_H 			( SAVE_VH * SUBTITLE_FONT_DISP_HEIGHT / SUBTITLE_FONT_HEIGHT )

#define SAVECOMP_X		OVER_WRITE_X
#define SAVECOMP_Y		OVER_WRITE_Y
#define SAVECOMP_U		0
#define SAVECOMP_V		(SAVE_V + SAVE_VH)
#define SAVECOMP_UW		(SUBTITLE_FONT_WIDTH * 30)
#define SAVECOMP_VH 	(SUBTITLE_FONT_HEIGHT * 5 / 4)
#define SAVECOMP_W		( SAVECOMP_UW * SUBTITLE_FONT_DISP_WIDTH / SUBTITLE_FONT_WIDTH )
#define SAVECOMP_H 		( SAVECOMP_VH * SUBTITLE_FONT_DISP_HEIGHT / SUBTITLE_FONT_HEIGHT )

#define PHOTO_NORMAL_X	20
#define PHOTO_NORMAL_Y	29
#define PHOTO_NORMAL_W	471
#define PHOTO_NORMAL_H	281

#define PHOTO_FULL_X	0
#define PHOTO_FULL_Y	0
#define PHOTO_FULL_W	640
#define PHOTO_FULL_H	480

#define PHOTO_TEX_W		JPEG_W
#define PHOTO_TEX_H		JPEG_H

#define JPEG_W		(DRAW_WIDTH)
#define JPEG_H		(DRAW_HEIGHT)

#define STR_TOTAL_LEN	(MCX_PHOTO_NAME_MAX + 3 + 1 + 2 + 3 * 3)
#define TEX_STRCODE	GV_StrCode( "node_font_alp_ovl" )


#if 0
#define RESOURCE_JAPAN (16727656) /* セーブゲームリソース日本  */
#define RESOURCE_ENGLISH (16727656) /* セーブゲームリソースアメリカ */
#define RESOURCE_FRANCE (16570686) /* セーブゲームリソースフランス語 */
#define RESOURCE_GERMAN (262847) /* セーブゲームリソースドイツ語 */
#define RESOURCE_ITARY (3895998) /* セーブゲームリソースイタリア語 */
#define RESOURCE_SPAIN (15842681) /* セーブゲームリソーススペイン語 */
#else
#define RESOURCE_JAPAN   	(0xf8a3c7) /* セーブゲームリソース日本語 */
#define RESOURCE_ENGLISH 	(0xebcfd2) /* セーブゲームリソース英語 */
#define RESOURCE_FRANCE 	(0xfcd93e) /* セーブゲームリソースフランス語 */
#define RESOURCE_GERMAN 	(0x0402bf) /* セーブゲームリソースドイツ語 */
#define RESOURCE_SPAIN  	(0xf1bd79) /* セーブゲームリソーススペイン語 */
#define RESOURCE_ITARY 		(0x3b72be) /* セーブゲームリソースイタリア語 */
#endif
static const int resource_name[] =
{
	RESOURCE_ENGLISH,
	RESOURCE_FRANCE,
	RESOURCE_GERMAN,
	RESOURCE_ITARY,
	RESOURCE_SPAIN,
	0,
	RESOURCE_JAPAN,
};
//#define SAVE_GAME_RESOURCE	5
#define SAVE_GAME_RESOURCE	(resource_name[ GM_Language - GM_LANG_ENGLISH ] )

#define RES_OVERWRITE		3
#define RES_SAVING			5
#define RES_SAVE_COMP		7
#define RES_SAVE_FAILED		8

enum {
	SAVE_FLAG_COMP		= 0x01,
	SAVE_FLAG_COMP_DISP	= 0x02,
	SAVE_FLAG_ERROR		= 0x04,
};

extern void MENU_DrawPictureTex( int x, int y, int w, int h, int color, DG_TEX_LIN *tex, int flag );
extern float SK_PrintfNormalWidth( u_char ascci , SPR_OBJ *pObj );

typedef struct MCX_PHOTOEDIT {
	MCX_STATE	state;
	DG_TEX_LIN		texlin;	// 画像表示用テクスチャ
	MCX_STRINGS		str_name;	// ファイル名
	int		l2d_action;
	MCX_INTERP	intrp_photo;
	int		curpos;
	int		prev_curpos;
	
	void*	pPrevLoad;		// 関数保存
	void*	pPrevLoadParam;	// 関数保存
	
	MCX_PHOTO_INFO*	pPhinfo;
	SPR_OBJ**		ppObj;	// スプライト列へ
	int		tri_handle;

	// ネームエントリ用
	int		namepos;			// 名前入力カーソル位置
	MCX_PHOTO_INFO	phinfotmp;
	MCX_INTERP	intrp_cursor_a;	// カーソルアルファ
	MCX_L2D_MORF	morfMaruBatu;
	MCX_L2D_MORF	morfMaruBatuRect;

	// 色変更用
	int		rgbpos;				// カーソル位置
	MCX_STRINGS		str_rgbnum[3];
	MCX_L2D_MORF	morfMaruBatuRGB;

	// 上書き確認用
	int		ynpos;
	MCX_STRINGS	str_yes;
	MCX_STRINGS	str_slash;
	MCX_STRINGS	str_no;
	MCX_MENU_STRINGS	mstr_overwrite;

	// セーブ用
	MCX_MENU_STRINGS	mstr_save;
	MCX_MENU_STRINGS	mstr_savecomp;
	int					save_flag;
	int					save_ctr;

	// フルスクリーン用
	BOOL				bFull;
	
} MCX_PHOTOEDIT;

enum {
	GUI_SUBMODE_FI = MCX_GUI_STATE_SYSTEMMAX,
	GUI_SUBMODE_ACTIVE,
	GUI_SUBMODE_FO,
};


/// 副モード設定
static void EnterNameEntry( MCX_MAN* pm, MCX_PHOTOEDIT* ps );
static void GuiNameEntry( MCX_MAN* pm, MCX_PHOTOEDIT* ps );
static void FadeoutNameEntry( MCX_MAN* pm, MCX_PHOTOEDIT* ps );
static void EnterRgb( MCX_MAN* pm, MCX_PHOTOEDIT* ps );
static void GuiRgb( MCX_MAN* pm, MCX_PHOTOEDIT* ps );
static void FadeoutRgb( MCX_MAN* pm, MCX_PHOTOEDIT* ps );
static void EnterYN( MCX_MAN* pm, MCX_PHOTOEDIT* ps );
static void GuiYN( MCX_MAN* pm, MCX_PHOTOEDIT* ps );
static void FadeoutYN( MCX_MAN* pm, MCX_PHOTOEDIT* ps );
static void EnterSave( MCX_MAN* pm, MCX_PHOTOEDIT* ps );
static void GuiSave( MCX_MAN* pm, MCX_PHOTOEDIT* ps );
static void FadeoutSave( MCX_MAN* pm, MCX_PHOTOEDIT* ps );
static void EnterExit( MCX_MAN* pm, MCX_PHOTOEDIT* ps );
static void EnterFullScreen( MCX_MAN* pm, MCX_PHOTOEDIT* ps );
static void GuiFullScreen( MCX_MAN* pm, MCX_PHOTOEDIT* ps );
static void FadeoutFullScreen( MCX_MAN* pm, MCX_PHOTOEDIT* ps );


typedef struct SUBMODE {
	void	(*Enter)( MCX_MAN* pm, MCX_PHOTOEDIT* ps );
	void	(*Gui)( MCX_MAN* pm, MCX_PHOTOEDIT* ps );
	void	(*FadeOut)( MCX_MAN* pm, MCX_PHOTOEDIT* ps );
	int		CursorCode;
} SUBMODE;

static SUBMODE	submode[] = {
	{ EnterNameEntry, GuiNameEntry, FadeoutNameEntry, CODE_SELECTTITLE },
	{ EnterRgb, GuiRgb, FadeoutRgb, CODE_SELECTCOLOR },
	{ EnterExit, NULL, NULL, CODE_SELECTEXIT },
	{ EnterYN, GuiYN, FadeoutYN },
	{ EnterSave, GuiSave, FadeoutSave },
	{ EnterFullScreen, GuiFullScreen, FadeoutFullScreen },
};
enum {
	SUBMODE_NAMEENTRY,
	SUBMODE_COLOR,
	SUBMODE_EXIT,
	SUBMODE_DISP_MAX,
	SUBMODE_YESNO = SUBMODE_DISP_MAX,
	SUBMODE_SAVING,
	SUBMODE_FULLSCREEN,
	SUBMODE_MAX
};

static BOOL Save( MCX_MAN* pm );
	
//// ローカル使用関数
static void PhotoDisp( MCX_MAN* pm, MCX_PHOTOEDIT* ps ) // 写真の描画
{
	u_int col;
	
//	memcpy( ps->pTexlin->image, pm->pDataBuffer, JPEG_W * JPEG_H * 4 );
	col = (((int) ps->intrp_photo.fCurrent) << 24) 
			| ps->phinfotmp.col[ 0 ]
			| ( ps->phinfotmp.col[ 1 ] << 8 )
			| ( ps->phinfotmp.col[ 2 ] << 16 ) ;
	if( ps->bFull ) {
		MENU_DrawPictureTex( 0 , 0, 512, 384, col, &ps->texlin, 0 );
	} else {
		MENU_DrawPictureTex( PHOTO_NORMAL_X , PHOTO_NORMAL_Y, PHOTO_NORMAL_W, PHOTO_NORMAL_H, col, &ps->texlin, 0 );
	}
}
////

static void Gui( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{
	if( ps->l2d_action != 0 && L2D_ActionStatus( pm->l2d_handle ) == L2D_STAT_ACK ) {
		L2D_EvokeAction( pm->l2d_handle, ps->l2d_action );
		ps->l2d_action = 0;
	}

	PhotoDisp( pm, ps );
	
	switch ( ps->state.gui_state ) {
	case MCX_GUI_STATE_FADEIN:
	{
		if( L2D_ActionStatus( pm->l2d_handle ) == L2D_STAT_ACK ) {
			ps->intrp_photo.target = 0x80;
			ps->intrp_photo.ctr = T_SHORT;
			ps->state.gui_state = MCX_GUI_STATE_ACTIVE;
			ps->l2d_action = CODE_SELECTTITLE;
			MCX_StringsShow( &ps->str_name, T_MID );
		}
		break;
	}
	case MCX_GUI_STATE_ACTIVE:
	{	// 通常
		if( L2D_ActionStatus( pm->l2d_handle ) == L2D_STAT_ACK ) {
			if( pm->press & PAD_L ) {
				if( ps->curpos > 0 ) {
					ps->curpos--;
					ps->l2d_action = submode[ ps->curpos ].CursorCode;
					SE_SEL();
				}
			} else if( pm->press & PAD_R ) {
				if( ps->curpos < SUBMODE_DISP_MAX - 1 ) {
					ps->curpos++;
					ps->l2d_action = submode[ ps->curpos ].CursorCode;
					SE_SEL();
				}
			} else if( pm->press & PAD_OK ) {
				ps->state.gui_state = GUI_SUBMODE_FI;
				submode[ ps->curpos ].Enter( pm, ps );
			} else if( pm->press & PAD_CANCEL ) {
				EnterExit( pm, ps );
			} else if( pm->press & PAD_R1 ) {
				ps->prev_curpos = ps->curpos;
				ps->curpos = SUBMODE_FULLSCREEN;
				ps->state.gui_state = GUI_SUBMODE_FI;
				EnterFullScreen( pm, ps );
			}
		}
		break;
	}
	case MCX_GUI_STATE_FADEOUT:
	{
		if( MCX_StringsIsHide( &ps->str_name ) ) {
			MCX_ExitState( &ps->state );
		}
		break;
	}
	case GUI_SUBMODE_FI:
	{
		if( L2D_ActionStatus( pm->l2d_handle ) == L2D_STAT_ACK ) {
			ps->state.gui_state = GUI_SUBMODE_ACTIVE;
		}
		break;
	}
	case GUI_SUBMODE_ACTIVE:
	{
		submode[ ps->curpos ].Gui( pm, ps );
		break;
	}
	case GUI_SUBMODE_FO:
	{
		submode[ ps->curpos ].FadeOut( pm, ps );
		break;
	}
	}
}

static void ClearStart( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{	// フェードアウト開始
	ps->intrp_photo.target = 0;
	ps->intrp_photo.ctr = T_MID;
	MCX_StringsHide( &ps->str_name, T_MID );
}
	

static void Handler( MCX_MAN* pm, MCX_PHOTOEDIT* ps, int event )
{	// イベントハンドラ
	switch( event ) {
	case MCX_EVENT_MC_NULL:		// フォーカス中メモリカードが抜けた
	{
		pm->curdev = DEV_HDD;	// 次に移る（ＭＣに対応しないので組まない）
		break;
	}
	case MCX_EVENT_SL_SUCCESS:
	{
		ps->save_flag |= SAVE_FLAG_COMP;
		break;
	}
	case MCX_EVENT_SL_ERROR:
	{
		ps->save_flag |= SAVE_FLAG_ERROR;
		break;
	}
	default:
		;
	}
}


static void Enter( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{	//
	int i;
	
	ps->l2d_action = CODE_OPENEDIT;
	DG_MakeLinerTexture2( &ps->texlin, PHOTO_TEX_W, PHOTO_TEX_H, DG_TEXLIN_FORMAT_A8R8G8B8,
						  (void*)(((long64)pm->pDataBuffer + 0xff) & ~0xff) );

	ps->intrp_photo.fCurrent = 0.f;
	ps->intrp_photo.ctr = 0;

	ps->curpos = 0;

	ps->pPhinfo = &pm->pCurfile->info.photoinfo;
	ps->phinfotmp = *ps->pPhinfo;
	
#if 1	// 仮処理
	if( ps->phinfotmp.col[ 0 ] == 0 && ps->phinfotmp.col[ 1 ]  == 0 && ps->phinfotmp.col[ 2 ] == 0 ) {
		ps->phinfotmp.col[ 0 ] = ps->phinfotmp.col[ 1 ] = ps->phinfotmp.col[ 2 ] = 0x80;
	}
#endif
	
	// 文字準備
	ps->ppObj = (SPR_OBJ**)MCX_Malloc( sizeof(SPR_OBJ*) * STR_TOTAL_LEN );
	MCX_StringsInitBuffer( ps->ppObj, STR_TOTAL_LEN, ps->tri_handle = SPR_LoadTexture( GV_StrCode( "font1" ) ),
						   TEX_STRCODE );
	MCX_StringsCreate( &ps->str_name, "123456789abcd", NAME_X, NAME_Y, STR_COLOR, NOFOCUS_A );
	MCX_StringsCreate( &ps->str_yes, "YES", YES_X, YES_Y, STR_COLOR, NOFOCUS_A );
	MCX_StringsCreate( &ps->str_slash, "/", SLASH_X, SLASH_Y, STR_COLOR, NOFOCUS_A );
	MCX_StringsCreate( &ps->str_no, "NO", NO_X, NO_Y, STR_COLOR, NOFOCUS_A );

	for( i = 0 ; i < 3 ; i++ ) {
		MCX_StringsCreate( &ps->str_rgbnum[ i ], "000", RGBNUM_X, RGBNUM_Y + RGBNUM_H * i, STR_COLOR, NOFOCUS_A );
	}


	MCX_MStringsCreate( &ps->mstr_overwrite, //GM_GetResource( SAVE_GAME_RESOURCE, RES_OVERWRITE ),
						GetLocalResource( SAVE_GAME_RESOURCE, RES_OVERWRITE ),
						OVER_WRITE_X, OVER_WRITE_Y, OVER_WRITE_W, OVER_WRITE_H,
						OVER_WRITE_U, OVER_WRITE_V, // + OVER_WRITE_VH,
						OVER_WRITE_UW, OVER_WRITE_VH, STR_COLOR );
	MCX_MStringsCreate( &ps->mstr_save, //GM_GetResource( SAVE_GAME_RESOURCE, RES_SAVING ),
						GetLocalResource( SAVE_GAME_RESOURCE, RES_SAVING ),
						SAVE_X, SAVE_Y, SAVE_W, SAVE_H,
						SAVE_U, SAVE_V,//+ SAVE_VH,
						SAVE_UW, SAVE_VH, STR_COLOR );
	MCX_MStringsCreate( &ps->mstr_savecomp, //GM_GetResource( SAVE_GAME_RESOURCE, RES_SAVE_COMP ),
						GetLocalResource( SAVE_GAME_RESOURCE, RES_SAVE_COMP ),
						SAVECOMP_X, SAVECOMP_Y, SAVECOMP_W, SAVECOMP_H,
						SAVECOMP_U, SAVECOMP_V, //+ SAVECOMP_VH,
						SAVECOMP_UW, SAVECOMP_VH, STR_COLOR );
	
	// str_name の大きさを変える
	(*ps->str_name.ppObj)->sprite.dw = NAME_W;
	(*ps->str_name.ppObj)->sprite.dh = NAME_H;
	MCX_StringsPrintf( &ps->str_name, "%s", ps->phinfotmp.name );
	
	MCX_AddInterpService( &ps->intrp_photo );
	MCX_AddInterpService( &ps->intrp_cursor_a );

	// 読み込み関数の保存
	ps->pPrevLoad = pm->SaveLoad;
	ps->pPrevLoadParam = pm->pSlParam;
	MCX_SetSaveLoadFunc( Save, pm );

	ps->bFull = FALSE;

	// モーフィング準備
	MCX_L2DMorfCreate( &ps->morfMaruBatu, pm->l2d_handle, CODE_OBJ_MARUBATSU );
	MCX_L2DMorfCreate( &ps->morfMaruBatuRect, pm->l2d_handle, CODE_OBJ_MARUBATSU_RECT );
	MCX_L2DMorfCreate( &ps->morfMaruBatuRGB, pm->l2d_handle, CODE_OBJ_MARUBATSU_RGB );
	
	MCX_SetAutoRepeat( 0 );
}

static void Exit( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{	//
	int i;
	
	ps->l2d_action = CODE_CLOSEEDIT;
//	DG_FreeLinerTexture( ps->pTexlin );
#ifdef KP_WINDOWS
	DG_FreeLinerTexture2( &ps->texlin );
#endif
	MCX_DeleteInterp( &ps->intrp_photo );
	MCX_DeleteInterp( &ps->intrp_cursor_a );
	MCX_StringsDelete( &ps->str_name );
	MCX_StringsDelete( &ps->str_yes);
	MCX_StringsDelete( &ps->str_slash );
	MCX_StringsDelete( &ps->str_no );
	for( i = 0 ; i < 3 ; i++ ) {
		MCX_StringsDelete( &ps->str_rgbnum[ i ] );
	}
	MCX_MStringsDelete( &ps->mstr_overwrite );
	MCX_MStringsDelete( &ps->mstr_save );
	MCX_MStringsDelete( &ps->mstr_savecomp );
	
	MCX_L2DMorfDelete( &ps->morfMaruBatu );
	MCX_L2DMorfDelete( &ps->morfMaruBatuRect );
	MCX_L2DMorfDelete( &ps->morfMaruBatuRGB );
	
	SPR_KillTexture( ps->tri_handle );
	MCX_Free( ps->ppObj );
}

MCX_STATE*	MCX_GetPhotoEditState( void )
{
	MCX_PHOTOEDIT* ps;

	ps = (MCX_PHOTOEDIT *)MCX_Malloc( sizeof( MCX_PHOTOEDIT) );
	ps->state.GUI = Gui;
	ps->state.ClearStart = ClearStart;
	ps->state.Handler = Handler;
	ps->state.Enter = Enter;
	ps->state.Exit = Exit;
	ps->state.L2dHandler = NULL;
	return &ps->state;
}


////////////////////////////////////////
// 名前変更部分

#define CODE_ALBAM_MARUBATSU (12637854)
#define CODE_ALBAM_MARUBATSU_RGB (1925804)
#define CODE_FONT_HILIGHT (1666338)

#define CODE_SHOW_NAME_FILTER (15610570)	// show_alb_filter
#define CODE_HIDE_NAME_FILTER (3111368)		// hide_alb_filter
#define CODE_SHOW_NAME_MARUBATU (14265529)
#define CODE_HIDE_NAME_MARUBATU (16097726)

#define NAME_ENTRY_MAX	10

#define CHAR_OK			'a'
#define CHAR_BACK		'`'

#define NAMEEDIT_PAD_OK			PAD_B
#define NAMEEDIT_PAD_CANCEL		PAD_A

static void ClearNameEntry( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{
	ps->intrp_cursor_a.ctr = T_SHORT;
	ps->intrp_cursor_a.target = 0;
	// Ｌ２Ｄ
	MCX_L2DMorfSet( &ps->morfMaruBatu, CODE_SHOW_NAME_MARUBATU, CODE_HIDE_NAME_MARUBATU, T_MID );
	MCX_L2DMorfSet( &ps->morfMaruBatuRect, CODE_SHOW_NAME_FILTER, CODE_HIDE_NAME_FILTER, T_MID );
}

static void EnterNameEntry( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{
	SPR_OBJ* pspr;
	pspr = L2D_GetObject( pm->l2d_handle , CODE_ALBAM_MARUBATSU );
	SPR_SHOW( pspr );
	pspr = L2D_GetObject( pm->l2d_handle , CODE_ALBAM_MARUBATSU_RGB );
	SPR_SHOW( pspr );
	ps->namepos = strlen( ps->phinfotmp.name ) - 1;

	pspr = L2D_GetObject( pm->l2d_handle , CODE_FONT_HILIGHT );
	pspr->sprite.col.a = 0;
	SPR_SHOW( pspr );

	// Ｌ２Ｄ
	MCX_L2DMorfSet( &ps->morfMaruBatu, CODE_HIDE_NAME_MARUBATU, CODE_SHOW_NAME_MARUBATU, T_MID );
	MCX_L2DMorfSet( &ps->morfMaruBatuRect, CODE_HIDE_NAME_FILTER, CODE_SHOW_NAME_FILTER, T_MID );
	
	// カーソルアルファ
	ps->intrp_cursor_a.fCurrent = 0.f;
	ps->intrp_cursor_a.target = 0x50;
	ps->intrp_cursor_a.ctr = T_SHORT;

	MCX_SetAutoRepeat( 6 );

	SE_OK();
}

static void GuiNameEntry( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{
	SPR_OBJ* pspr;
	u_short press;
	static const char chtable[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_!&./:?`a ";

	pspr = L2D_GetObject( pm->l2d_handle, CODE_OBJ_TOP_MARUBATSU );
	SPR_SHOW( pspr );
	
	pspr = L2D_GetObject( pm->l2d_handle , CODE_FONT_HILIGHT );
	SPR_SHOW( pspr );

	// 文字入力処理
	press = pm->press;

	if( ps->namepos == NAME_ENTRY_MAX ) {	// 最終文字の時の例外
		press &= ~(PAD_L | PAD_R);
	}

	if( press & PAD_L ) {	// 文字を変える
		int curpos = strrchr( chtable, ps->phinfotmp.name[ ps->namepos ] ) - chtable;
		ps->phinfotmp.name[ ps->namepos ] = chtable[ curpos - 1 ];
		SE_SEL();
	} else if( press & PAD_R ) {	// 文字を変える
		int curpos = strchr( chtable, ps->phinfotmp.name[ ps->namepos ] ) - chtable;
		ps->phinfotmp.name[ ps->namepos ] = chtable[ curpos + 1 ];
		SE_SEL();
	} else if( press & NAMEEDIT_PAD_OK ) {	// 文字が入力された
		char* pchcur = &ps->phinfotmp.name[ ps->namepos ];
		if( *pchcur == CHAR_OK ) {
			press = PAD_STA;
			if( ps->namepos > 0 ) {
				*pchcur = '\0';
				ps->namepos--;
			} else {
				*pchcur = '?';	// 名前がないときは'?' にする
			}
		} else if ( *pchcur == CHAR_BACK ) {
			press = NAMEEDIT_PAD_CANCEL;
			*pchcur = '\0';
		} else {	// 通常の文字入力
			press = 0;
			ps->namepos++;
			pchcur++;
			if( ps->namepos == NAME_ENTRY_MAX ) {
				*pchcur = CHAR_OK;
			} else {
				*pchcur = ' ';
			}
			SE_TYPE();
		}
	}
	if ( press & NAMEEDIT_PAD_CANCEL ) {	// 文字を消してカーソル移動
		SE_CANCEL();
		if( ps->namepos > 0 ) {
			ps->phinfotmp.name[ ps->namepos ] = '\0';
			ps->namepos--;
		} else {
			ps->phinfotmp.name[ ps->namepos ] = ' ';
		}
	} else if ( press & PAD_STA ) {
		// 名前の正当性チェック
		int i;
		SE_OK();
		for ( i = 0 ; i <= ps->namepos ; i++ ) {
			if( ps->phinfotmp.name[ i ] != ' ' ) break;
		}
		if( i == ps->namepos + 1 ) {	// 全部空白だった
			ZeroMemory( ps->phinfotmp.name, sizeof( ps->phinfotmp.name ) );
			ps->phinfotmp.name[ 0 ] = '?';
			ps->namepos = 0;
		}
		
		// 上書きするかどうか聞く
		ps->prev_curpos = ps->curpos;
		ps->curpos = SUBMODE_YESNO;
		submode[ ps->curpos ].Enter( pm, ps );
		ps->state.gui_state = GUI_SUBMODE_FI;
		SPR_HIDE( pspr );
	} else if ( press & PAD_SEL ) {
		// 名前を元に戻してキャンセル
		ps->phinfotmp = *ps->pPhinfo;
		ClearNameEntry( pm, ps );
		ps->state.gui_state = GUI_SUBMODE_FO;
		MCX_SetAutoRepeat( 0 );
		SE_CANCEL();
	}

	// カーソルスプライトの処理
	MCX_StringsPrintf( &ps->str_name, "%s", ps->phinfotmp.name );
	pspr->sprite.pos.x = ps->str_name.ppObj[ ps->namepos ]->sprite.pos.x;
	pspr->sprite.pos.y = ps->str_name.ppObj[ ps->namepos ]->sprite.pos.y - 1;
	pspr->sprite.dh = ps->str_name.ppObj[ ps->namepos ]->sprite.dh;
	{	// exponential 幅補間
		int nextw = (int)SK_PrintfNormalWidth( ps->phinfotmp.name[ ps->namepos ], NULL );
		nextw = ( nextw == 0xff ) ? 14 : nextw;
		pspr->sprite.dw = (pspr->sprite.dw * 3 + nextw ) / 4;
	}
	pspr->sprite.col.a = (u_char)ps->intrp_cursor_a.fCurrent;

}

static void FadeoutNameEntry( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{
	SPR_OBJ* pspr;
	pspr = L2D_GetObject( pm->l2d_handle , CODE_FONT_HILIGHT );
	pspr->sprite.col.a = (u_char)ps->intrp_cursor_a.fCurrent;
	if( ps->intrp_cursor_a.ctr == 0 ) {
		SPR_HIDE( pspr );
		ps->state.gui_state = MCX_GUI_STATE_ACTIVE;
	}
}


////////////////////////////////////////
// 色変更部分
#define CODE_OPENRGB	8941000
#define CODE_CLOSERGB (6751495)
#define CODE_OBJ_RGBSELECT (15807010)
#define CODE_OBJ_RBAR      (5313689)
#define CODE_OBJ_GBAR      (10556568)
#define CODE_OBJ_BBAR      (5400341)
#define CODE_OBJ_RMASK	 (7842518)
#define CODE_OBJ_GMASK	(7842519)
#define CODE_OBJ_BMASK	 (7842520)
#define CODE_SHOW_RGB_FILTER (602829)
#define CODE_HIDE_RGB_FILTER (4880842)
#define CODE_SHOW_RGB_MARUBATU (14267685)
#define CODE_HIDE_RGB_MARUBATU (16099882)

#define LINE_Y 			(275)
#define BAR_MAX			128

static const code_bar[] = { CODE_OBJ_RBAR, CODE_OBJ_GBAR, CODE_OBJ_BBAR };
static const code_mask[] = { CODE_OBJ_RMASK, CODE_OBJ_GMASK, CODE_OBJ_BMASK };

static void ClearRgb( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{	// ＲＧＢ画面を消去開始
	int i;
	for( i = 0 ; i < 3 ; i++ ) {
		MCX_StringsHide( &ps->str_rgbnum[ i ], T_MID );
	}
	ps->state.gui_state = GUI_SUBMODE_FO;
	ps->l2d_action = CODE_CLOSERGB;

	MCX_L2DMorfSet( &ps->morfMaruBatuRGB, CODE_SHOW_RGB_MARUBATU, CODE_HIDE_RGB_MARUBATU, T_MID );
	MCX_L2DMorfSet( &ps->morfMaruBatuRect, CODE_SHOW_RGB_FILTER, CODE_HIDE_RGB_FILTER, T_MID );
}


static void EnterRgb( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{
	int i;
	ps->l2d_action = CODE_OPENRGB;
	ps->rgbpos = 0;
	for( i = 0 ; i < 3 ; i++ ) {
		MCX_StringsPrintf( &ps->str_rgbnum[i], "%d", ps->phinfotmp.col[i] );
		if( i == 0 ) MCX_StringsShow( &ps->str_rgbnum[ i ], T_MID );
		else MCX_StringsNofocus( &ps->str_rgbnum[ i ], T_MID );
	}
	MCX_L2DMorfSet( &ps->morfMaruBatuRGB, CODE_HIDE_RGB_MARUBATU, CODE_SHOW_RGB_MARUBATU, T_MID );
	MCX_L2DMorfSet( &ps->morfMaruBatuRect, CODE_HIDE_RGB_FILTER, CODE_SHOW_RGB_FILTER, T_MID );
	
	MCX_SetAutoRepeat( 2 );
	SE_OK();
}

static void GuiRgb( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{
	u_short press;
	SPR_OBJ* pspr;
	int i;

	pspr = L2D_GetObject( pm->l2d_handle, CODE_OBJ_TOP_MARUBATSU );
	SPR_SHOW( pspr );

	press = pm->press;

	if( press & PAD_L ) {
		u_char* pcol = &ps->phinfotmp.col[ps->rgbpos];
		if( *pcol > 0 ) {
			--*pcol;
			SE_RGB_TUNE();
		}
	} else if( press & PAD_R ) {
		u_char* pcol = &ps->phinfotmp.col[ps->rgbpos];
		if( *pcol < 0xff ) {
			++*pcol;
			SE_RGB_TUNE();
		}
	} else if( press & PAD_U ) {
		if( ps->rgbpos > 0 ) {
			MCX_StringsNofocus( &ps->str_rgbnum[ ps->rgbpos ], T_SHORT );
			--ps->rgbpos;
			MCX_StringsShow( &ps->str_rgbnum[ ps->rgbpos ], T_SHORT );
			SE_SEL();
		}
	} else if( press & PAD_D ) {
		if( ps->rgbpos < 2 ) {
			MCX_StringsNofocus( &ps->str_rgbnum[ ps->rgbpos ], T_SHORT );
			++ps->rgbpos;
			MCX_StringsShow( &ps->str_rgbnum[ ps->rgbpos ], T_SHORT );
			SE_SEL();
		}
	} else if( press & PAD_CANCEL ) {
		// キャンセル
		ps->phinfotmp = *ps->pPhinfo;	// 色を元に戻す
		MCX_SetAutoRepeat( 0 );
		ClearRgb( pm, ps );
		SE_CANCEL();
	} else if( press & PAD_OK ) {
		// 決定
		// 上書きするかどうか聞く
		ps->prev_curpos = ps->curpos;
		ps->curpos = SUBMODE_YESNO;
		submode[ ps->curpos ].Enter( pm, ps );
		ps->state.gui_state = GUI_SUBMODE_FI;
		SE_OK();
	}

	// 画面更新
	for( i = 0 ; i < 3 ; i++ ) {
		SPR_OBJ* pspr2;
		// 文字を書く
		MCX_StringsPrintf( &ps->str_rgbnum[ i ], "%d", ps->phinfotmp.col[ i ] );
		// バー表示
		pspr = L2D_GetObject( pm->l2d_handle, code_bar[ i ] );
		pspr2 = pspr->head.child;
		pspr->line.pos[ 1 ].x = pspr->line.pos[ 0 ].x + ps->phinfotmp.col[ i ] * BAR_MAX / 256.f;
		pspr2->line.pos[ 0 ].x = pspr->line.pos[ 1 ].x - 95.f;
		pspr2->line.pos[ 1 ].x = pspr2->line.pos[ 0 ].x;
		// マスク処理
		pspr = L2D_GetObject( pm->l2d_handle, code_mask[ i ] );
		if( i == ps->rgbpos ) SPR_HIDE( pspr );
		else SPR_SHOW( pspr );
	}
	pspr = L2D_GetObject( pm->l2d_handle, CODE_OBJ_RGBSELECT );
	pspr->line.pos[ 0 ].y = (float)(LINE_Y + RGBNUM_H * ps->rgbpos);
	pspr->line.pos[ 1 ].y = pspr->line.pos[ 0 ].y;
	
}

static void FadeoutRgb( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{
	int i;
	for( i = 0 ; i < 3; i++ ) {
		SPR_OBJ* pspr;
		pspr = L2D_GetObject( pm->l2d_handle, code_mask[ i ] );
		SPR_HIDE( pspr );
	}
	if( L2D_ActionStatus( pm->l2d_handle) == L2D_STAT_ACK
		&& MCX_StringsIsHide( &ps->str_rgbnum[ 0 ]) ) {
		ps->state.gui_state = MCX_GUI_STATE_ACTIVE;
	}
}

////////////////////////////////////////
// 上書きしていいか聞く
#define CODE_OPENOVERWRITE	(12887728)
#define CODE_CLOSEOVERWRITE	(6977128)
static void EnterYN( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{
	ps->l2d_action = CODE_OPENOVERWRITE;
	ps->ynpos	= 1;	// No がデフォルト
	MCX_StringsShow( &ps->str_slash, T_MID );
	MCX_StringsShow( &ps->str_no, T_MID );
	ps->str_no.col = (ps->str_no.col & 0xff000000) | ( GM_FOCUS_COLOR & 0x00ffffff);
	MCX_StringsNofocus( &ps->str_yes, T_MID );
	ps->str_yes.col = (ps->str_yes.col & 0xff000000) | ( STR_COLOR & 0x00ffffff);
	MCX_MStringsShow( &ps->mstr_overwrite, T_MID );
}

static void GuiYN( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{
	u_short press = pm->press;

	if( ps->str_yes.intrp_alpha.ctr != 0 ) return ;	// 補間中は判定なし

	// カーソル判定
	if( press & PAD_L ) {
		if( ps->ynpos == 1 ) {
			ps->ynpos = 0;
			MCX_StringsShow( &ps->str_yes, T_SHORT );
			ps->str_yes.col = (ps->str_yes.col & 0xff000000) | ( GM_FOCUS_COLOR & 0x00ffffff);
			MCX_StringsNofocus( &ps->str_no, T_SHORT );
			ps->str_no.col = (ps->str_no.col & 0xff000000) | ( STR_COLOR & 0x00ffffff);
			SE_SEL();
		}
	} else if( press & PAD_R ){
		if( ps->ynpos == 0 ) {
			ps->ynpos = 1;
			MCX_StringsShow( &ps->str_no, T_SHORT );
			ps->str_no.col = (ps->str_no.col & 0xff000000) | ( GM_FOCUS_COLOR & 0x00ffffff);
			MCX_StringsNofocus( &ps->str_yes, T_SHORT );
			ps->str_yes.col = (ps->str_yes.col & 0xff000000) | ( STR_COLOR & 0x00ffffff);
			SE_SEL();
		}
	} else if( press & (PAD_OK | PAD_CANCEL) ) {
		MCX_StringsHide( &ps->str_yes, T_MID );
		MCX_StringsHide( &ps->str_slash, T_MID );
		MCX_StringsHide( &ps->str_no, T_MID );
		MCX_MStringsHide( &ps->mstr_overwrite, T_MID );
		if( ps->ynpos == 1 || (press & PAD_CANCEL)) {
			// キャンセルで元に戻る
			ps->l2d_action = CODE_CLOSEOVERWRITE;
			ps->state.gui_state = GUI_SUBMODE_FO;
			SE_CANCEL();
		} else {
			// 上書きセーブする
			pm->pCurfile->info.photoinfo = ps->phinfotmp;
			ps->curpos = SUBMODE_SAVING;
			submode[ ps->curpos ].Enter( pm, ps );
			ps->state.gui_state = GUI_SUBMODE_FI;
			SE_OK();
		}
	}
}

static void FadeoutYN( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{
	if( MCX_StringsIsHide( &ps->str_yes ) ) {
		ps->curpos = ps->prev_curpos;
		ps->state.gui_state = GUI_SUBMODE_FI;
	}
}

////////////////////////////////////////
// Full Screen Mode
#define CODE_PRE_SCREEN_TO_FULL_SCREEN_OK (10624623)
#define CODE_FULL_SCREEN_TO_PRE_SCREEN_OK (13667387)

static void EnterFullScreen( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{
	ps->l2d_action = CODE_PRE_SCREEN_TO_FULL_SCREEN_OK;
	ps->intrp_photo.ctr = T_MID;
	ps->intrp_photo.target = 0;
	MCX_StringsHide( &ps->str_name, T_MID );
}

static void GuiFullScreen( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{
	u_int press = pm->press ;

	if( ps->intrp_photo.target == 0 ) { // F.O.中
		if( ps->intrp_photo.ctr == 0 ) { // F.O.終了
			ps->intrp_photo.target = 0x80;
			ps->intrp_photo.ctr = T_MID;
			ps->bFull = TRUE;
		}
	} else {
		if( press ) {
			ps->l2d_action = CODE_FULL_SCREEN_TO_PRE_SCREEN_OK;
			ps->state.gui_state = GUI_SUBMODE_FO;
			ps->intrp_photo.target = 0;
			ps->intrp_photo.ctr = T_MID;
			MCX_StringsShow( &ps->str_name, T_MID );
		}
	}
}

static void FadeoutFullScreen( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{
	if( L2D_ActionStatus( pm->l2d_handle ) == L2D_STAT_ACK
		&& ps->intrp_photo.ctr == 0 ) {
		ps->state.gui_state = MCX_GUI_STATE_ACTIVE;
		ps->curpos = ps->prev_curpos;
		ps->bFull = FALSE;
		ps->intrp_photo.target = 0x80;
		ps->intrp_photo.ctr = T_MID;
		ps->l2d_action = submode[ ps->curpos ].CursorCode;
	}
}

///////////////////////////////////////
// 上書きセーブ

static BOOL Save( MCX_MAN* pm )
{
	HANDLE hFile;
	int size;
	
	if( ( hFile = MCX_Open( GENERIC_WRITE )) == INVALID_HANDLE_VALUE ) {
		return FALSE;
	}
//	size = pm->sign.dwFileLength - sizeof( MCX_SIGNATURE ) - MCX_FILE_INFO_SIZE;
	size = pm->sign.dwFileLength - sizeof( MCX_SIGNATURE );
	if( ( MCX_Write( hFile, pm->pLoadBuffer, size ) )
		!= size ) {
		MCX_Close( hFile );
		return FALSE;
	}
	if( MCX_Close( hFile ) != 0 ) {
		return FALSE;
	}
	
	return TRUE;
}

static void EnterSave( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{	// セーブ開始
	MCX_MStringsShow( &ps->mstr_save, T_MID );
	MCX_QueueRequest( MCX_REQUEST_SAVE_GAME );
	ps->save_flag = 0;
	ps->save_ctr = DIRECT_TICK( 40 );
}

static void GuiSave( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{
	if( !(ps->save_flag & SAVE_FLAG_COMP_DISP) ) {
		// 前半・セーブ中
		if( ps->save_ctr > 0 ) {
			ps->save_ctr--;
		} else if( ps->save_flag & ( SAVE_FLAG_COMP | SAVE_FLAG_ERROR) ) {
			if( ps->save_flag & SAVE_FLAG_ERROR ) {
				MCX_MStringsClearLine( &ps->mstr_savecomp );
				MCX_MStringsPrintf( &ps->mstr_savecomp, // GM_GetResource( SAVE_GAME_RESOURCE, RES_SAVE_FAILED )
									"%s", GetLocalResource( SAVE_GAME_RESOURCE, RES_SAVE_FAILED ) );
			} else {
				SE_SAVE_FINISH();
			}
			ps->save_ctr = DIRECT_TICK( 40 );
			MCX_MStringsHide( &ps->mstr_save, T_MID );
			MCX_MStringsShow( &ps->mstr_savecomp, T_MID );
			ps->save_flag |= SAVE_FLAG_COMP_DISP;
		} 
	} else {
		// 後半・セーブ完了表示中
		if( --ps->save_ctr < 0 ) {
			MCX_MStringsHide( &ps->mstr_savecomp, T_MID );
			ps->l2d_action = CODE_CLOSEOVERWRITE;
			ps->state.gui_state = GUI_SUBMODE_FO;
		}
	}
}

static void FadeoutSave( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{
	if( MCX_MStringsIsHide( &ps->mstr_savecomp ) ) {
		ps->curpos = ps->prev_curpos;
		MCX_SetAutoRepeat( 0 );
		if( ps->curpos == SUBMODE_NAMEENTRY ) {
			// NameEntry モードの画面を消す
			ClearNameEntry( pm, ps );
		} else {
			// ＲＧＢ変更モードの画面を消す
			ClearRgb( pm, ps );
		}
	}
}

//////////////////////////////////////////////
// エディットモードから抜ける
static void EnterExit( MCX_MAN* pm, MCX_PHOTOEDIT* ps )
{
	SE_CANCEL();
	MCX_SetSaveLoadFunc( ps->pPrevLoad, ps->pPrevLoadParam );
	MCX_ClearStart( MCX_STATE_SL );
	MCX_ChangeState( &ps->state, MCX_STATE_FILE_SELECT );
}

#endif
