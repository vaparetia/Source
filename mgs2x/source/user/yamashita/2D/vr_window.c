//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * vr_window - vr_window.c
 * ＶＲウィンドウ  *NewVRWindow
 * 2002/04/19 S.Yamashita
 * $Id: vr_window.c,v 1.1.1.3 2002/11/19 11:51:49 Yoshizawa1 Exp $
 */

/*******************************************************************************
 * include
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libdg.cnf"
#include "libmt.h"
#include "gameheader.h"
#include "def_dma.h"
#include "libutl.h"

#include "../../sigeno/vr/vr.h"
#include "msn.h"
#include "vr_def.h"
#include "layoutman.h"

#include "BP_BuildDefines.h"
#include "BP_Font.h"

#define __CHARA_NAME__ "VR Window"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
//#define SY_PRINTF3_DEBUG
#include "../sy_util/sy_util.h"

#include "mode/menu/xtextscn.h"

/*******************************************************************************
 * defines
 */

/* テキストボックスタイプ */
enum {
	TEXT_BOX_A = 0,		/* 全体 */
	TEXT_BOX_1,			/* ボックス１ */
	TEXT_BOX_2,			/* ボックス２ */
};
enum {
	ORIGIN = 0,			/* 原点 */
	RANGE,				/* 範囲 */
};

/* コールバックタイプ */
enum {
	VRWIN_CB_OPEN_7 = 0,	/* 開く　　７ */
	VRWIN_CB_CLOSE_7,		/* 閉じる　７ */
	VRWIN_CB_OPEN_6,		/* 開く　　６ */
	VRWIN_CB_CLOSE_6,		/* 閉じる　６ */
	VRWIN_CB_OPEN_4,		/* 開く　　４ */
	VRWIN_CB_CLOSE_4,		/* 閉じる　４ */
	VRWIN_CB_HIDE_IDLE,		/* 非表示アイドル */
};

/* メッセージタイプ */
enum {
	VRWIN_MSG_HIDE = 0,		/* 非表示 */
	VRWIN_MSG_SHOW,			/* 表示 */
	VRWIN_MSG_CLOSE,		/* 閉じる */
	VRWIN_MSG_OPEN,			/* 開く */
	VRWIN_MSG_COMPACT,		/* コンパクト */
	VRWIN_MSG_FULL,			/* 完全 */
	VRWIN_MSG_HIDE_PICT,	/* 写真非表示 */
	VRWIN_MSG_SHOW_PICT,	/* 写真表示 */
};

/* フェーズタイプ */
enum {
	VRWIN_PHASE_INIT = 0,						/* 初期化 */

	VRWIN_PHASE_WINDOW_OPEN,					/* ウィンドウオープン */
	VRWIN_PHASE_TEXT_SHOW_ANIME,				/* テキスト一文字ずつ表示 */
	VRWIN_PHASE_PICT_SHOW_ANIME,				/* 写真表示アニメ */
	VRWIN_PHASE_SHOW_IDLE,						/* 表示アイドル */

	VRWIN_PHASE_PICT_HIDE_ANIME,				/* 写真消去アニメ */
	VRWIN_PHASE_TEXT_HIDE_ANIME,				/* テキストフェードアウト */
	VRWIN_PHASE_WINDOW_CLOSE,					/* ウィンドウクローズ */
	VRWIN_PHASE_HIDE_IDLE,						/* 非表示アイドル */
};
#define VRWIN_PHASE_WINDOW_OPEN_COUNT     (DIRECT_TICK(80))		/* フェーズのカウント値 */
#define VRWIN_PHASE_PICT_SHOW_ANIME_COUNT (DIRECT_TICK(15))		/* フェーズのカウント値 */
#define VRWIN_BLINK_COUNT                 (DIRECT_TICK(30))		/* 点滅のカウント数 */
#define VRWIN_PHASE_PICT_HIDE_ANIME_COUNT (DIRECT_TICK(15))		/* フェーズのカウント値 */
#define VRWIN_PHASE_TEXT_HIDE_ANIME_COUNT (DIRECT_TICK(15))		/* フェーズのカウント値 */

/* フラグ */
#define VRWIN_FLAG_JAPANESE    (0x0001)		/* 日本語モード */
#define VRWIN_FLAG_NO_L_SPACE  (0x0002)		/* 行間なしモード */
#define VRWIN_FLAG_COMPACT     (0x0004)		/* コンパクトモード */
#define VRWIN_FLAG_7LINES      (0x0008)		/* ７行モード */
#define VRWIN_FLAG_TRIAL       (0x0010)		/* 体験版モード（Ｌ２Ｄファイルが変わる） */
#define VRWIN_FAST_MODE        (0x0020)		/* 高速モード */
#define VRWIN_SNAKE_TALES      (0x0040)		/* スネークテイルズモード */
#define VRWIN_HIDE_START       (0x0080)		/* ステージ開始時に表示しないでスタートモード */
#define VRWIN_STREAM_STOP      (0x0100)		/* ストリーム停止予約 */
#define VRWIN_SNAKE_TALES6     (0x0200)		/* スネークテイルズ６行モード */
#define VRWIN_FLAG_BLINK_FO    (0x1000)		/* 点滅フェードアウト */
#define VRWIN_FLAG_PROC_CALLED (0x2000)		/* プロックの呼び出し */
#define VRWIN_FLAG_INIT        (0x8000)		/* 初期化 */

/* 設定値 */
#define VRWIN_PICT_POS_X        (-17.0f)	/* 写真 右上の相対位置 */
#define VRWIN_PICT_POS_Y        (7.0f)		/* 写真 右上の相対位置 */
#define VRWIN_PICT_WIDTH        (128.0f*TARGET_ASPECT_X)	/* 写真 幅 */
#define VRWIN_PICT_HEIGHT       (96.0f)		/* 写真 高さ */
#define VRWIN_PICT_COLOR_R      (128)		/* 写真 色Ｒ */
#define VRWIN_PICT_COLOR_G      (128)		/* 写真 色Ｇ */
#define VRWIN_PICT_COLOR_B      (128)		/* 写真 色Ｂ */
#define VRWIN_PICT_COLOR_A      (128)		/* 写真 色Ａ */

//Scales text down in order for the 4th line of explanation not to be cut out on Vita.
#define VRWIN_VITA_FONT_EXTRA_SCALE (0.95f)
#define VRWIN_FONT_SCALE_H_ENG  (0.65f * VRWIN_VITA_FONT_EXTRA_SCALE)		/* フォント スケール横（英語） */
#define VRWIN_FONT_SCALE_V_ENG  (0.65f * VRWIN_VITA_FONT_EXTRA_SCALE)		/* フォント スケール縦（英語） */
#define VRWIN_FONT_SCALE_H_JPN  (0.69f * VRWIN_VITA_FONT_EXTRA_SCALE)		/* フォント スケール横（日本語） */
#define VRWIN_FONT_SCALE_V_JPN  (0.69f * VRWIN_VITA_FONT_EXTRA_SCALE)		/* フォント スケール縦（日本語） */
#define VRWIN_FONT_TEX_WIDTH    (18.0f)		/* フォント テクスチャ幅 */
#define VRWIN_FONT_TEX_HEIGHT   (12.0f)		/* フォント テクスチャ高さ */
#define VRWIN_FONT_TEX_HEIGHT2  (14.0f)		/* フォント テクスチャ高さ */
#define VRWIN_FONT_PITCH        (0.0f)		/* フォント 文字ピッチ */

#define VRWIN_TEXT_1_LENGTH     (400)		/* テキスト１の長さ制限 */
#define VRWIN_TEXT_2_LENGTH     (60)		/* テキスト２の長さ制限 */

#define VRWIN_TEXT_BOX_MARGIN_L (17.0f)		/* 左マージン */
#define VRWIN_TEXT_BOX_MARGIN_T ( 9.0f)		/* 上マージン */
#define VRWIN_TEXT_BOX_MARGIN_M ( 8.0f)		/* 中マージン */
#define VRWIN_TEXT_BOX_MARGIN_R ( 0.0f)		/* 右マージン */
#define VRWIN_TEXT_BOX_MARGIN_B ( 5.0f)		/* 下マージン */

/* Ｌ２Ｄオブジェクト */
#define L2D_FILENAME          ( 4343564)		/* vr_window */
#define L2D_FILENAME_T        (  816226)		/* vr_window_trial */

#define ACTION_WINDOW_OPEN_7  ( 7126734)		/* vr_window_open_7 */
#define ACTION_WINDOW_CLOSE_7 ( 3830781)		/* vr_window_close_7 */
#define ACTION_LINE_MOVE_7    ( 9940853)		/* line_move */
#define ACTION_WINDOW_OPEN_6  ( 7126733)		/* vr_window_open_6 */
#define ACTION_WINDOW_CLOSE_6 ( 3830780)		/* vr_window_close_6 */
#define ACTION_LINE_MOVE_6    (12444276)		/* line_move_6 */
#define ACTION_WINDOW_OPEN_4  ( 7126731)		/* vr_window_open_4 */
#define ACTION_WINDOW_CLOSE_4 ( 3830778)		/* vr_window_close_4 */
#define ACTION_LINE_MOVE_4    (12444274)		/* line_move_4 */
#define ACTION_DUMMY          ( 8143391)		/* dummy */
#define ACTION_DUMMY2         ( 8930337)		/* dummy2 */

#define OBJECT_LEVEL_1        ( 3507603)		/* level_num_1 */
#define OBJECT_LEVEL_2        ( 3507604)		/* level_num_2 */
#define OBJECT_MODE           (11327235)		/* vr_window_mode */
#define OBJECT_MISSION        ( 5314114)		/* vr_window_missions */
#define OBJECT_WEAPON         (12241172)		/* vr_window_weapon */
#define OBJECT_HYPHEN         (  411418)		/* minus */
#define OBJECT_LEVEL          (16016658)		/* level */
#define OBJECT_BG_3           ( 2238483)		/* BG_3 */
#define OBJECT_BG_2           ( 2238482)		/* BG_2 */
#define OBJECT_BG_1           ( 2238481)		/* BG_1 */
#define OBJECT_RECT           (10172196)		/* move_rect */
#define OBJECT_MOVE_LINE      ( 9940853)		/* line_move */

#define OBJECT_HYPHEN2        ( 1864747)		/* minus_2 */
#define OBJECT_LEVEL_A        ( 3507651)		/* level_num_a */
#define OBJECT_LEVEL_B        ( 3507652)		/* level_num_b */

/*******************************************************************************
 * work
 */

/* ワーク */
typedef struct tagWORK
{
	GV_ACT_EX    actor;							/* アクター */

	int          name;							/* シナリオ名 */
	int          proc_id;						/* プロックＩＤ */
	int          bgmhandler;					/* BGMハンドラー */
	int          bgmvol;						/* BGM音量 */

	char         mission;						/* ミッション */
	char         mode;							/* モード */
	char         weapon;						/* 武器 */
	char         level;							/* レベル */
	float        mis_tex_v0;					/* ミッションテクスチャのＶ値の原点 */
	float        mod_tex_v0;					/* モードテクスチャのＶ値の原点 */
	float        wea_tex_v0;					/* 武器テクスチャのＶ値の原点 */
	float        lev_tex_u0;					/* レベルテクスチャのＵ値の原点 */
	unsigned int font_color;					/* フォント色 */
	float        font_scale_h;					/* フォントスケール横 */
	float        font_scale_v;					/* フォントスケール縦 */

	SPR_POS      pict_pos;						/* 写真表示位置 */
	SPR_OBJ      *pict;							/* 写真スプライト */
	int          pict_tri;						/* 写真 tri ハンドル */

	SPR_POS      text_box[3][2];				/* テキストボックス矩形 */
	char         text2[VRWIN_TEXT_2_LENGTH];	/* テキスト２ */
	char         *text1;						/* テキスト１ */

	void         *text_work;					/* テキストのワーク */
	char         *text_a;						/* 制限時間テキスト */
	char         *text_b;						/* 分テキスト */
	char         *text_c;						/* 秒テキスト */
	char         *text_d;						/* 敵兵数テキスト */
	char         *text_e;						/* 人テキスト */
	char         *text_f;						/* 標的数テキスト */
	char         *text_g;						/* 個テキスト */
	char         *text_h;						/* 爆弾数テキスト */
	char         *text_i;						/* 個テキスト */
	short        flag;							/* フラグ */
	short        count;							/* カウント */
   short        textOffset;
	char         phase;							/* フェーズ */
	char         player;						/* プレイヤー */
	short        sub_level;						/* サブレベル */

	LAYOUTMAN    layoutman;						/* レイアウトマネージャ */
}
WORK;

/*******************************************************************************
 * extern
 */

void *NewTextScreenControlEx(int text_vram_width, int text_vram_height, int prio, int flag, int bufferedTextFlag);
void MENU_ClearTextTexture(void *work);
void MENU_PutTextScreen(void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col);

/*******************************************************************************
 * local
 */

static char _nostring[1] = { '\0' };	/* 長さ０文字列 */

/* ＬＯＭデータ */
static int _lom_data[] = {
					/*アクションリスト名*/		/*アクション名*/		/*カウント*/	/*フラグ*/
LOM_ACTLIST_DATA,	LOM_ACTLIST_00,				ACTION_DUMMY,			1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												LOM_DATA_END,

					LOM_ACTLIST_01,				ACTION_WINDOW_OPEN_7,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_LINE_MOVE_7,		-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_02,				ACTION_WINDOW_CLOSE_7,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_DUMMY,			1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												ACTION_DUMMY2,			1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												LOM_DATA_END,

					LOM_ACTLIST_04,				ACTION_WINDOW_OPEN_6,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_LINE_MOVE_6,		-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_05,				ACTION_WINDOW_CLOSE_6,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_DUMMY,			1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												ACTION_DUMMY2,			1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												LOM_DATA_END,

					LOM_ACTLIST_07,				ACTION_WINDOW_OPEN_4,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_LINE_MOVE_4,		-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_08,				ACTION_WINDOW_CLOSE_4,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_DUMMY,			1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												ACTION_DUMMY2,			1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												LOM_DATA_END,

					LOM_ACTLIST_09,				ACTION_DUMMY2,			1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_DATA_END,

					/*パッドリスト名*/			/*アクション名*/		/*オブジェクト名*/	/*入力*/							/*実行番号*/		/*実行引数*/			/*フラグ*/
LOM_PADLIST_DATA,	LOM_PADLIST_00,				ACTION_LINE_MOVE_7,		LOM_PAD_ALLOBJ,		PAD_SEL|PAD_UDLR|LOM_PAD_OK|LOM_PAD_CANCEL,	LOM_EXE_CALLBACK,	VRWIN_CB_CLOSE_7,		LOM_PAD_PRESS,
												LOM_DATA_END,
					LOM_PADLIST_01,				ACTION_DUMMY,			LOM_PAD_ALLOBJ,		LOM_PAD_ANYINPUT,					LOM_EXE_CALLBACK,	VRWIN_CB_HIDE_IDLE,		0,
												ACTION_DUMMY2,			LOM_PAD_ALLOBJ,		PAD_SEL,							LOM_EXE_CALLBACK,	VRWIN_CB_OPEN_7,		LOM_PAD_PRESS,
												LOM_DATA_END,

					LOM_PADLIST_02,				ACTION_LINE_MOVE_6,		LOM_PAD_ALLOBJ,		PAD_SEL|PAD_UDLR|LOM_PAD_OK|LOM_PAD_CANCEL,	LOM_EXE_CALLBACK,	VRWIN_CB_CLOSE_6,		LOM_PAD_PRESS,
												LOM_DATA_END,
					LOM_PADLIST_03,				ACTION_DUMMY,			LOM_PAD_ALLOBJ,		LOM_PAD_ANYINPUT,					LOM_EXE_CALLBACK,	VRWIN_CB_HIDE_IDLE,		0,
												ACTION_DUMMY2,			LOM_PAD_ALLOBJ,		PAD_SEL,							LOM_EXE_CALLBACK,	VRWIN_CB_OPEN_6,		LOM_PAD_PRESS,
												LOM_DATA_END,

					LOM_PADLIST_04,				ACTION_LINE_MOVE_4,		LOM_PAD_ALLOBJ,		PAD_SEL|PAD_UDLR|LOM_PAD_OK|LOM_PAD_CANCEL,	LOM_EXE_CALLBACK,	VRWIN_CB_CLOSE_4,		LOM_PAD_PRESS,
												LOM_DATA_END,
					LOM_PADLIST_05,				ACTION_DUMMY,			LOM_PAD_ALLOBJ,		LOM_PAD_ANYINPUT,					LOM_EXE_CALLBACK,	VRWIN_CB_HIDE_IDLE,		0,
												ACTION_DUMMY2,			LOM_PAD_ALLOBJ,		PAD_SEL,							LOM_EXE_CALLBACK,	VRWIN_CB_OPEN_4,		LOM_PAD_PRESS,
												LOM_DATA_END,

					LOM_PADLIST_06,				ACTION_DUMMY2,			LOM_PAD_ALLOBJ,		PAD_SEL,							LOM_EXE_CALLBACK,	VRWIN_CB_OPEN_4,		LOM_PAD_PRESS,
												LOM_DATA_END,
					LOM_DATA_END,

					/*モード名*/	/*ＬＯＭアクションリスト*/	/*ＬＯＭオブジェクトリスト*/	/*ＬＯＭパッドリスト*/
LOM_MODE_DATA,		LOM_MODE_00,	LOM_ACTLIST_00,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,		/* 初期状態 */
					LOM_MODE_01,	LOM_ACTLIST_01,				LOM_MODE_NOLIST,				LOM_PADLIST_00,			/* ７行 開く */
					LOM_MODE_02,	LOM_ACTLIST_02,				LOM_MODE_NOLIST,				LOM_PADLIST_01,			/* ７行 閉じる */
					LOM_MODE_04,	LOM_ACTLIST_04,				LOM_MODE_NOLIST,				LOM_PADLIST_02,			/* ６行 開く */
					LOM_MODE_05,	LOM_ACTLIST_05,				LOM_MODE_NOLIST,				LOM_PADLIST_03,			/* ６行 閉じる */
					LOM_MODE_07,	LOM_ACTLIST_07,				LOM_MODE_NOLIST,				LOM_PADLIST_04,			/* ４行 開く */
					LOM_MODE_08,	LOM_ACTLIST_08,				LOM_MODE_NOLIST,				LOM_PADLIST_05,			/* ４行 閉じる */
					LOM_MODE_09,	LOM_ACTLIST_09,				LOM_MODE_NOLIST,				LOM_PADLIST_06,			/* ステージ開始時に表示しないでスタートモード */
					LOM_DATA_END,
LOM_DATA_END,
};

/*******************************************************************************
 * static
 */
/*******************************************************************************
 * テキスト２の設定
 */
static void SetText2(
	WORK *work)		/* ワーク */
{
	int min, sec, enemy, target, bomb;

	min    = (VR_TIME / (VRDEF_FRAMERATE * 60)) % 100;
	sec    = (VR_TIME % (VRDEF_FRAMERATE * 60)) / VRDEF_FRAMERATE;
	enemy  = VR_ENEMY_MAX  % 1000;
	target = VR_TARGET_MAX % 1000;
	bomb   = VR_BOMBS_MAX  % 1000;

#if 0
 	if(work->mission == MSN_MISSION_VR)
	{
		if(work->mode == MSN_MODE_SNEAKING)
		{
			if(work->weapon == MSN_WEAPON_SNEAKING)			{ (min == 0) ? sprintf(work->text2, "\n%s %d %s"              , work->text_a, sec, work->text_c)
															: ((sec > 0) ? sprintf(work->text2, "\n%s %d %s %d %s"        , work->text_a, min, work->text_b, sec, work->text_c)
															             : sprintf(work->text2, "\n%s %d %s"              , work->text_a, min, work->text_b)); }

			else											{ (min == 0) ? sprintf(work->text2, "%s %d %s\n%s %d %s"      , work->text_a, sec, work->text_c,                    work->text_d, enemy, work->text_e)
															: ((sec > 0) ? sprintf(work->text2, "%s %d %s %d %s\n%s %d %s", work->text_a, min, work->text_b, sec, work->text_c, work->text_d, enemy, work->text_e)
															             : sprintf(work->text2, "%s %d %s\n%s %d %s"      , work->text_a, min, work->text_b,                    work->text_d, enemy, work->text_e)); }
		}
		else if(work->mode == MSN_MODE_WEAPON)				{ (min == 0) ? sprintf(work->text2, "%s %d %s\n%s %d %s"      , work->text_a, sec, work->text_c,                    work->text_f, target, work->text_g)
															: ((sec > 0) ? sprintf(work->text2, "%s %d %s %d %s\n%s %d %s", work->text_a, min, work->text_b, sec, work->text_c, work->text_f, target, work->text_g)
															             : sprintf(work->text2, "%s %d %s\n%s %d %s"      , work->text_a, min, work->text_b,                    work->text_f, target, work->text_g)); }

		else if(work->mode == MSN_MODE_FIRST_PERSON_VIEW)	{ (min == 0) ? sprintf(work->text2, "\n%s %d %s"              , work->text_a, sec, work->text_c)
															: ((sec > 0) ? sprintf(work->text2, "\n%s %d %s %d %s"        , work->text_a, min, work->text_b, sec, work->text_c)
															             : sprintf(work->text2, "\n%s %d %s"              , work->text_a, min, work->text_b)); }

		else if(work->mode == MSN_MODE_VARIETY)				{ (min == 0) ? sprintf(work->text2, "\n%s %d %s"              , work->text_a, sec, work->text_c)
															: ((sec > 0) ? sprintf(work->text2, "\n%s %d %s %d %s"        , work->text_a, min, work->text_b, sec, work->text_c)
															             : sprintf(work->text2, "\n%s %d %s"              , work->text_a, min, work->text_b)); }

		else												{ (min == 0) ? sprintf(work->text2, "\n%s %d %s"              , work->text_a, sec, work->text_c)
															: ((sec > 0) ? sprintf(work->text2, "\n%s %d %s %d %s"        , work->text_a, min, work->text_b, sec, work->text_c)
															             : sprintf(work->text2, "\n%s %d %s"              , work->text_a, min, work->text_b)); }
	}
	else
	{
		if     (work->mode == MSN_MODE_BOMB_DISPOSAL)		{ (min == 0) ? sprintf(work->text2, "%s %d %s\n%s %d %s"      , work->text_a, sec, work->text_c,                    work->text_h, bomb, work->text_i)
															: ((sec > 0) ? sprintf(work->text2, "%s %d %s %d %s\n%s %d %s", work->text_a, min, work->text_b, sec, work->text_c, work->text_h, bomb, work->text_i)
															             : sprintf(work->text2, "%s %d %s\n%s %d %s"      , work->text_a, min, work->text_b,                    work->text_h, bomb, work->text_i)); }

		else if(work->mode == MSN_MODE_ELIMINATE)			{ (min == 0) ? sprintf(work->text2, "%s %d %s\n%s %d %s"      , work->text_a, sec, work->text_c,                    work->text_d, enemy, work->text_e)
															: ((sec > 0) ? sprintf(work->text2, "%s %d %s %d %s\n%s %d %s", work->text_a, min, work->text_b, sec, work->text_c, work->text_d, enemy, work->text_e)
															             : sprintf(work->text2, "%s %d %s\n%s %d %s"      , work->text_a, min, work->text_b,                    work->text_d, enemy, work->text_e)); }

		else if(work->mode == MSN_MODE_HOLD_UP)				{ (min == 0) ? sprintf(work->text2, "%s %d %s\n%s %d %s"      , work->text_a, sec, work->text_c,                    work->text_d, enemy, work->text_e)
															: ((sec > 0) ? sprintf(work->text2, "%s %d %s %d %s\n%s %d %s", work->text_a, min, work->text_b, sec, work->text_c, work->text_d, enemy, work->text_e)
															             : sprintf(work->text2, "%s %d %s\n%s %d %s"      , work->text_a, min, work->text_b,                    work->text_d, enemy, work->text_e)); }

		else												{ (min == 0) ? sprintf(work->text2, "\n%s %d %s"              , work->text_a, sec, work->text_c)
															: ((sec > 0) ? sprintf(work->text2, "\n%s %d %s %d %s"        , work->text_a, min, work->text_b, sec, work->text_c)
															             : sprintf(work->text2, "\n%s %d %s"              , work->text_a, min, work->text_b)); }
	}
#else
	if(work->flag & VRWIN_FLAG_TRIAL)
	{
		switch(work->level)
		{
		case 1 : sprintf(work->text2, "\n%s %2d %s %02d %s"           , work->text_a, min, work->text_b, sec, work->text_c); break;
		case 2 : sprintf(work->text2,   "%s %2d %s %02d %s\n%s %2d %s", work->text_a, min, work->text_b, sec, work->text_c, work->text_f, target, work->text_g); break;
		case 3 : sprintf(work->text2,   "%s %2d %s %02d %s\n%s %2d %s", work->text_a, min, work->text_b, sec, work->text_c, work->text_d, enemy, work->text_e); break;
		case 4 : sprintf(work->text2,   "%s %2d %s %02d %s\n%s %2d %s", work->text_a, min, work->text_b, sec, work->text_c, work->text_f, target, work->text_g); break;
		case 5 : sprintf(work->text2, "\n%s %2d %s %02d %s"           , work->text_a, min, work->text_b, sec, work->text_c); break;
		default: ASSERT(0);
		}
	}
	else
	{
		if(work->mission == MSN_MISSION_VR)
		{
			if(work->mode == MSN_MODE_SNEAKING)
			{
				if(work->weapon == MSN_WEAPON_SNEAKING)			{ sprintf(work->text2, "\n%s %2d %s %02d %s"           , work->text_a, min, work->text_b, sec, work->text_c); }
				else											{ sprintf(work->text2,   "%s %2d %s %02d %s\n%s %2d %s", work->text_a, min, work->text_b, sec, work->text_c, work->text_d, enemy, work->text_e); }
			}
			else if(work->mode == MSN_MODE_WEAPON)				{ sprintf(work->text2,   "%s %2d %s %02d %s\n%s %2d %s", work->text_a, min, work->text_b, sec, work->text_c, work->text_f, target, work->text_g); }
			else if(work->mode == MSN_MODE_FIRST_PERSON_VIEW)	{ sprintf(work->text2, "\n%s %2d %s %02d %s"           , work->text_a, min, work->text_b, sec, work->text_c); }
			else if(work->mode == MSN_MODE_VARIETY)				{ sprintf(work->text2, "\n%s %2d %s %02d %s"           , work->text_a, min, work->text_b, sec, work->text_c); }
			else												{ sprintf(work->text2, "\n%s %2d %s %02d %s"           , work->text_a, min, work->text_b, sec, work->text_c); }
		}
		else
		{
			if     (work->mode == MSN_MODE_BOMB_DISPOSAL)		{ sprintf(work->text2,   "%s %2d %s %02d %s\n%s %2d %s", work->text_a, min, work->text_b, sec, work->text_c, work->text_h, bomb, work->text_i); }
			else if(work->mode == MSN_MODE_ELIMINATE)			{ sprintf(work->text2,   "%s %2d %s %02d %s\n%s %2d %s", work->text_a, min, work->text_b, sec, work->text_c, work->text_d, enemy, work->text_e); }
			else if(work->mode == MSN_MODE_HOLD_UP)				{ sprintf(work->text2,   "%s %2d %s %02d %s\n%s %2d %s", work->text_a, min, work->text_b, sec, work->text_c, work->text_d, enemy, work->text_e); }
			else												{ sprintf(work->text2, "\n%s %2d %s %02d %s"           , work->text_a, min, work->text_b, sec, work->text_c); }
		}
	}
#endif
}

/*******************************************************************************
 * レベルの表示設定
 */
static void SetLevel(
	WORK *work,		/* ワーク */
	int  value,		/* 値 */
	int  obj_10,	/* １０の位 */
	int  obj_01)	/* １の位 */
{
	int i;

	if(value < 0)
		return;

	/* １０の位 */
	ASSERT(value < 100)
	i = value / 10;
	value %= 10;
	LOM_SprTex_SetU(&work->layoutman, obj_10, work->lev_tex_u0 + VRWIN_FONT_TEX_WIDTH * i);

	/* １の位 */
	LOM_SprTex_SetU(&work->layoutman, obj_01, work->lev_tex_u0 + VRWIN_FONT_TEX_WIDTH * value);
}

/*******************************************************************************
 * レイアウトマネージャのコールバック
 */
static int LOMCallback(		/* 1: 成功:処理を続行 */
							/* 2: 成功:処理を中断 */
							/* 0: 失敗 */
	void *pWork,	/* ワーク */
	int  param)		/* パラメータ */
{
	WORK *work = (WORK *)pWork;

	SY_PRINTF3("LOMCallback\n");
	SY_PRINTF1("Callback: %d\n", param);

	switch(param)
	{
	case VRWIN_CB_OPEN_7:
	case VRWIN_CB_OPEN_6:
	case VRWIN_CB_OPEN_4:
		if(!(GM_VRStatus & GM_VR_IDLE))
		{
			if(!(GM_IsGameOver() || (GM_VRStatus & GM_VR_CLEAR) || (GV_PauseLevel & GV_LEVEL_NORMAL) || (GV_PadData[0].flag & GV_PAD_RELEASE)))
			{
				if     (work->flag & VRWIN_SNAKE_TALES6) LOM_SetCurMode(&work->layoutman, LOM_MODE_04, 1, 1, 1);
				else if(work->flag & VRWIN_FLAG_COMPACT) LOM_SetCurMode(&work->layoutman, LOM_MODE_07, 1, 1, 1);
				else if(work->flag & VRWIN_FLAG_7LINES ) LOM_SetCurMode(&work->layoutman, LOM_MODE_01, 1, 1, 1);
				else                                     LOM_SetCurMode(&work->layoutman, LOM_MODE_04, 1, 1, 1);
				SY_PRINTF1("VRWIN_PHASE_WINDOW_OPEN\n");
				work->phase = VRWIN_PHASE_WINDOW_OPEN;
            work->count = work->textOffset = 0;

				/* ポーズ状態に入る */
				MSN_2DSTATUS |= MSN_2DSTAT_WINDOW_PAUSE;
				GV_PauseOnActorSystem(GV_PAUSE_PAUSE);
				GM_GameStatus |= STATE_PAUSE_DISABLE;

				GM_VRStatus |= GM_VR_WINDOW_OPEN;

				GM_SdSet(SD_S_WINOPN01);
				work->flag &= ~VRWIN_STREAM_STOP;
			}
		}
		return 1;

	case VRWIN_CB_CLOSE_7:
	case VRWIN_CB_CLOSE_6:
	case VRWIN_CB_CLOSE_4:
		if(work->phase == VRWIN_PHASE_SHOW_IDLE)
		{
			SY_PRINTF1("VRWIN_PHASE_PICT_HIDE_ANIME\n");
			work->phase = VRWIN_PHASE_PICT_HIDE_ANIME;
			work->count = work->textOffset = 0;

			GM_SdSet(SD_S_V_CANS02);
		}
		return 1;

	case VRWIN_CB_HIDE_IDLE:
		SY_PRINTF1("VRWIN_PHASE_HIDE_IDLE\n");
		work->phase = VRWIN_PHASE_HIDE_IDLE;
		work->count = work->textOffset = 0;

//		GM_StreamStop( work->bgmhandler ) ;

		/* コンパクトモードに移行 */
		work->flag |= VRWIN_FLAG_COMPACT;
		work->flag &= ~VRWIN_FLAG_7LINES;

		/* ポーズ状態を解除する */
		if(MSN_2DSTATUS & MSN_2DSTAT_WINDOW_PAUSE)
		{
			MSN_2DSTATUS &= ~MSN_2DSTAT_WINDOW_PAUSE;
			GV_PauseOffActorSystem(GV_PAUSE_PAUSE);
			GM_GameStatus &= ~STATE_PAUSE_DISABLE;
		}
		else
		{
			int	hndl ;

			GM_SdSet(INT_PAUSEOFF);
			if ( GM_StreamGetChannelHandler( 0 ) == work->bgmhandler ) {
				GM_SdSet( STR1_FADE_OUT ) ;
printf("str channel --------------- 0 \n") ;
			} else if ( GM_StreamGetChannelHandler( 1 ) == work->bgmhandler ) {
				GM_SdSet( STR2_FADE_OUT ) ;
printf("str channel --------------- 2 \n") ;
			}
		}
		GM_VRStatus &= ~GM_VR_WINDOW_OPEN;

		/* プロック実行 */
		if((work->proc_id != 0) && ((work->flag & VRWIN_FLAG_PROC_CALLED) == 0))
		{
			SY_PRINTF1("PROC CALLED\n");
			work->flag |= VRWIN_FLAG_PROC_CALLED;
			GM_ExecProc(work->proc_id, NULL);
		}

		MSN_2DSTATUS |= MSN_2DSTAT_QUICK_WINDOW;
		return 1;

	default:
		return 1;
	}
}

/*******************************************************************************
 * メッセージ処理

mesg ＶＲウィンドウ $s:名前 非表示[0]
mesg ＶＲウィンドウ $s:名前 表示[1]
mesg ＶＲウィンドウ $s:名前 開く[3]
mesg ＶＲウィンドウ $s:名前 写真非表示[6]
mesg ＶＲウィンドウ $s:名前 写真表示[7]
 */
static int ReceiveMessage(	/* 処理結果 */
	WORK *work)		/* ワーク */
{
	int    i;
	int    msg_count;
	int    ret = -1;
	int    flags;
	GV_MSG *msg;

	msg_count = GV_ReceiveMessage(work->name, &msg);
	for(i = 0; i < msg_count; i++, msg++)
	{
		SY_PRINTF2("Message received.\n");

		switch(msg->message[0])
		{
		case VRWIN_MSG_HIDE:		/* 非表示 */
			LOM_Emp_GetFlags(&work->layoutman, 2770484 /* ROOT */, &flags);
			flags |= SPR_FLAG_HIDDEN;
			LOM_Emp_SetFlags(&work->layoutman, 2770484 /* ROOT */, flags);
			ret = 0;
			break;

		case VRWIN_MSG_SHOW:		/* 表示 */
			LOM_Emp_GetFlags(&work->layoutman, 2770484 /* ROOT */, &flags);
			flags &= ~SPR_FLAG_HIDDEN;
			LOM_Emp_SetFlags(&work->layoutman, 2770484 /* ROOT */, flags);
			ret = 0;
			break;

		case VRWIN_MSG_OPEN:		/* 開く */
			if(!(GM_IsGameOver() || (GM_VRStatus & GM_VR_CLEAR) || (GV_PauseLevel & GV_LEVEL_NORMAL)))
			{
				if     (work->flag & VRWIN_SNAKE_TALES6) LOM_SetCurMode(&work->layoutman, LOM_MODE_04, 1, 1, 1);
				else if(work->flag & VRWIN_FLAG_COMPACT) LOM_SetCurMode(&work->layoutman, LOM_MODE_07, 1, 1, 1);
				else if(work->flag & VRWIN_FLAG_7LINES ) LOM_SetCurMode(&work->layoutman, LOM_MODE_01, 1, 1, 1);
				else                                     LOM_SetCurMode(&work->layoutman, LOM_MODE_04, 1, 1, 1);
				SY_PRINTF1("VRWIN_PHASE_WINDOW_OPEN\n");
				work->phase = VRWIN_PHASE_WINDOW_OPEN;
				work->count = work->textOffset = 0;

				GM_SdSet(INT_PAUSEON);
				work->flag |= VRWIN_STREAM_STOP;
			}
			ret = 0;
			break;

		case VRWIN_MSG_HIDE_PICT:	/* 写真非表示 */
			if(work->pict != NULL)
				work->pict->head.flags |= SPR_FLAG_HIDDEN;
			ret = 0;
			break;

		case VRWIN_MSG_SHOW_PICT:	/* 写真表示 */
			if(work->pict != NULL)
				work->pict->head.flags &= ~SPR_FLAG_HIDDEN;
			ret = 0;
			break;

		default:
			ret = -1;
			break;
		}
	}

	return ret;
}

/******************************************************************************
 * 毎フレーム処理
 */
static void Act(
	WORK *work)		/* ワーク */
{
	int          flag;
	int          flags;
	int          draw_text = 0;
	char         text1[VRWIN_TEXT_1_LENGTH];
	char         text2[VRWIN_TEXT_2_LENGTH];
	SPR_COLOR    col;
	unsigned int font_color = 0;
	unsigned int alpha;
	SPR_POS      pict_pos;

	/* テキストボックス位置 */
	if(work->phase != VRWIN_PHASE_HIDE_IDLE)
	{
		LOM_Spr_GetPosition(&work->layoutman, OBJECT_BG_3, &work->text_box[TEXT_BOX_A][ORIGIN]  , 1);
		LOM_Spr_GetWidth   (&work->layoutman, OBJECT_BG_3, &work->text_box[TEXT_BOX_A][RANGE ].x, 1);
		LOM_Spr_GetHeight  (&work->layoutman, OBJECT_BG_3, &work->text_box[TEXT_BOX_A][RANGE ].y, 1);

		work->text_box[TEXT_BOX_A][ORIGIN].x += VRWIN_TEXT_BOX_MARGIN_L;
		work->text_box[TEXT_BOX_A][ORIGIN].y += VRWIN_TEXT_BOX_MARGIN_T;
		work->text_box[TEXT_BOX_A][RANGE ].x -= VRWIN_TEXT_BOX_MARGIN_L + VRWIN_TEXT_BOX_MARGIN_R;
		work->text_box[TEXT_BOX_A][RANGE ].y -= VRWIN_TEXT_BOX_MARGIN_T + VRWIN_TEXT_BOX_MARGIN_B;

		if(LOM_GetCurMode(&work->layoutman) == LOM_MODE_07)
		{
			/* ４行 開くモード */
			work->text_box[TEXT_BOX_1][ORIGIN].x = work->text_box[TEXT_BOX_A][ORIGIN].x;
			work->text_box[TEXT_BOX_1][ORIGIN].y = work->text_box[TEXT_BOX_A][ORIGIN].y;
			work->text_box[TEXT_BOX_1][RANGE ].x = work->text_box[TEXT_BOX_A][RANGE ].x; 
			work->text_box[TEXT_BOX_1][RANGE ].y = work->text_box[TEXT_BOX_A][RANGE ].y;
		}
		else if(LOM_GetCurMode(&work->layoutman) == LOM_MODE_04)
		{
			/* ６行 開くモード */
			if(work->flag & VRWIN_SNAKE_TALES6)
			{
				work->text_box[TEXT_BOX_1][ORIGIN].x = work->text_box[TEXT_BOX_A][ORIGIN].x;
				work->text_box[TEXT_BOX_1][ORIGIN].y = work->text_box[TEXT_BOX_A][ORIGIN].y;
				work->text_box[TEXT_BOX_1][RANGE ].x = work->text_box[TEXT_BOX_A][RANGE ].x; 
				work->text_box[TEXT_BOX_1][RANGE ].y = work->text_box[TEXT_BOX_A][RANGE ].y;
			}
			else
			{
				work->text_box[TEXT_BOX_1][ORIGIN].x =  work->text_box[TEXT_BOX_A][ORIGIN].x;
				work->text_box[TEXT_BOX_1][ORIGIN].y =  work->text_box[TEXT_BOX_A][ORIGIN].y;
				work->text_box[TEXT_BOX_1][RANGE ].x =  work->text_box[TEXT_BOX_A][RANGE ].x; 
				work->text_box[TEXT_BOX_1][RANGE ].y = (work->text_box[TEXT_BOX_A][RANGE ].y - VRWIN_TEXT_BOX_MARGIN_M) * 4 / 6;

				work->text_box[TEXT_BOX_2][ORIGIN].x = work->text_box[TEXT_BOX_A][ORIGIN].x;
				work->text_box[TEXT_BOX_2][ORIGIN].y = work->text_box[TEXT_BOX_1][ORIGIN].y + work->text_box[TEXT_BOX_1][RANGE ].y + VRWIN_TEXT_BOX_MARGIN_M;
				work->text_box[TEXT_BOX_2][RANGE ].x = work->text_box[TEXT_BOX_A][RANGE ].x; 
				work->text_box[TEXT_BOX_2][RANGE ].y = work->text_box[TEXT_BOX_A][RANGE ].y - work->text_box[TEXT_BOX_1][RANGE ].y - VRWIN_TEXT_BOX_MARGIN_M;
			}
		}
		else
		{
			if(work->flag & VRWIN_SNAKE_TALES6)
			{
				work->text_box[TEXT_BOX_1][ORIGIN].x =  work->text_box[TEXT_BOX_A][ORIGIN].x;
				work->text_box[TEXT_BOX_1][ORIGIN].y =  work->text_box[TEXT_BOX_A][ORIGIN].y;
				work->text_box[TEXT_BOX_1][RANGE ].x =  work->text_box[TEXT_BOX_A][RANGE ].x; 
				work->text_box[TEXT_BOX_1][RANGE ].y = (work->text_box[TEXT_BOX_A][RANGE ].y - VRWIN_TEXT_BOX_MARGIN_M) * 6 / 7;

				work->text_box[TEXT_BOX_2][ORIGIN].x = work->text_box[TEXT_BOX_A][ORIGIN].x;
				work->text_box[TEXT_BOX_2][ORIGIN].y = work->text_box[TEXT_BOX_1][ORIGIN].y + work->text_box[TEXT_BOX_1][RANGE ].y + VRWIN_TEXT_BOX_MARGIN_M;
				work->text_box[TEXT_BOX_2][RANGE ].x = work->text_box[TEXT_BOX_A][RANGE ].x; 
				work->text_box[TEXT_BOX_2][RANGE ].y = work->text_box[TEXT_BOX_A][RANGE ].y - work->text_box[TEXT_BOX_1][RANGE ].y - VRWIN_TEXT_BOX_MARGIN_M;
			}
			else
			{
				work->text_box[TEXT_BOX_1][ORIGIN].x =  work->text_box[TEXT_BOX_A][ORIGIN].x;
				work->text_box[TEXT_BOX_1][ORIGIN].y =  work->text_box[TEXT_BOX_A][ORIGIN].y;
				work->text_box[TEXT_BOX_1][RANGE ].x =  work->text_box[TEXT_BOX_A][RANGE ].x; 
				work->text_box[TEXT_BOX_1][RANGE ].y = (work->text_box[TEXT_BOX_A][RANGE ].y - VRWIN_TEXT_BOX_MARGIN_M) * 5 / 7;

				work->text_box[TEXT_BOX_2][ORIGIN].x = work->text_box[TEXT_BOX_A][ORIGIN].x;
				work->text_box[TEXT_BOX_2][ORIGIN].y = work->text_box[TEXT_BOX_1][ORIGIN].y + work->text_box[TEXT_BOX_1][RANGE ].y + VRWIN_TEXT_BOX_MARGIN_M;
				work->text_box[TEXT_BOX_2][RANGE ].x = work->text_box[TEXT_BOX_A][RANGE ].x; 
				work->text_box[TEXT_BOX_2][RANGE ].y = work->text_box[TEXT_BOX_A][RANGE ].y - work->text_box[TEXT_BOX_1][RANGE ].y - VRWIN_TEXT_BOX_MARGIN_M;
			}
		}
	}

	if(work->flag & VRWIN_FLAG_INIT)
	{
		/*
		 * ＶＲウィンドウの初期化
		 */
		work->flag &= ~VRWIN_FLAG_INIT;

		/* テクスチャのＵＶ値の設定 */
		LOM_SprTex_GetV(&work->layoutman, OBJECT_MISSION, &work->mis_tex_v0);
		LOM_SprTex_GetV(&work->layoutman, OBJECT_MODE   , &work->mod_tex_v0);
		LOM_SprTex_GetV(&work->layoutman, OBJECT_WEAPON , &work->wea_tex_v0);
		LOM_SprTex_GetU(&work->layoutman, OBJECT_LEVEL_2, &work->lev_tex_u0);
		SY_PRINTF1("mis v0: %f\n", work->mis_tex_v0);
		SY_PRINTF1("mod v0: %f\n", work->mod_tex_v0);
		SY_PRINTF1("wea v0: %f\n", work->wea_tex_v0);
		SY_PRINTF1("lev u0: %f\n", work->lev_tex_u0);

		/* テキストのワーク */
		if     (work->flag & VRWIN_FLAG_NO_L_SPACE) flag = 0x0001;	/* 行間なし */
		else if(work->flag & VRWIN_FLAG_JAPANESE  ) flag = 0x0000;	/* 行間大 */
		else                                        flag = 0x0004;	/* 行間小 */
		if((work->text_work = NewTextScreenControlEx(
			(((((int)(work->text_box[TEXT_BOX_A][RANGE].x / work->font_scale_h)) + 63) / 64) * 64),
			work->text_box[TEXT_BOX_A][RANGE].y / work->font_scale_v, 240, flag, 0/*bufferedTextFlag*/)) == NULL)
		{
			SY_PRINTF2("Create Text Screen Control Failed.\n");
			ASSERT(0);
		}
		GV_SetActorChild(work, work->text_work);

		/* レイアウトマネージャ */
		if(work->flag & VRWIN_HIDE_START) LOM_SetCurMode(&work->layoutman, LOM_MODE_09, 1, 1, 1);
		else                              LOM_SetCurMode(&work->layoutman, LOM_MODE_00, 1, 1, 1);
		work->phase = VRWIN_PHASE_HIDE_IDLE;
	}

	/* メッセージ処理 */
	ReceiveMessage(work);

	if(work->phase != VRWIN_PHASE_HIDE_IDLE)
	{
		/* スキップモード */
		if(    (work->phase < VRWIN_PHASE_SHOW_IDLE)
			&& (GV_PadDataDirect[0].press & (PAD_UDLR|PAD_CANCEL))
			&& (MSN_2DSTATUS & MSN_2DSTAT_QUICK_WINDOW))
		{
			/* 写真を非表示に */
			if(work->pict != NULL)
				work->pict->head.flags |= SPR_FLAG_HIDDEN;

			GM_SdSet(SD_S_V_CANS02);

			SY_PRINTF1("VRWIN_PHASE_WINDOW_CLOSE\n");
			work->phase = VRWIN_PHASE_WINDOW_CLOSE;
			work->count = work->textOffset = 0;

			/* レイアウトマネージャ */
			if     (LOM_GetCurMode(&work->layoutman) == LOM_MODE_07) LOM_SetCurMode(&work->layoutman, LOM_MODE_08, 1, 1, 1);
			else if(LOM_GetCurMode(&work->layoutman) == LOM_MODE_04) LOM_SetCurMode(&work->layoutman, LOM_MODE_05, 1, 1, 1);
			else                                                     LOM_SetCurMode(&work->layoutman, LOM_MODE_02, 1, 1, 1);
		}
	}

	/* レイアウトマネージャ */
	ActLayoutman(&work->layoutman);

	if(work->phase != VRWIN_PHASE_HIDE_IDLE)
	{
		if(work->flag & VRWIN_SNAKE_TALES)
		{
			/* ＵＶの設定 */
			LOM_SprTex_SetV     (&work->layoutman, OBJECT_MISSION, work->mis_tex_v0 + VRWIN_FONT_TEX_HEIGHT * MSN_MISSION_MAX);
			LOM_SprTex_SetV     (&work->layoutman, OBJECT_MODE   , work->mod_tex_v0 + VRWIN_FONT_TEX_HEIGHT2 * (MSN_MODE_MAX + work->level));
			LOM_SprTex_SetHeight(&work->layoutman, OBJECT_MODE   , VRWIN_FONT_TEX_HEIGHT2);

			/* 表示の設定 */
			LOM_Spr_GetFlags(&work->layoutman, OBJECT_HYPHEN , &flags); flags |= SPR_FLAG_HIDDEN;
			LOM_Spr_SetFlags(&work->layoutman, OBJECT_HYPHEN , flags);
			LOM_Spr_GetFlags(&work->layoutman, OBJECT_WEAPON , &flags); flags |= SPR_FLAG_HIDDEN;
			LOM_Spr_SetFlags(&work->layoutman, OBJECT_WEAPON , flags);
			LOM_Spr_GetFlags(&work->layoutman, OBJECT_LEVEL  , &flags); flags |= SPR_FLAG_HIDDEN;
			LOM_Spr_SetFlags(&work->layoutman, OBJECT_LEVEL  , flags);
			LOM_Spr_GetFlags(&work->layoutman, OBJECT_LEVEL_1, &flags); flags |= SPR_FLAG_HIDDEN;
			LOM_Spr_SetFlags(&work->layoutman, OBJECT_LEVEL_1, flags);
			LOM_Spr_GetFlags(&work->layoutman, OBJECT_LEVEL_2, &flags); flags |= SPR_FLAG_HIDDEN;
			LOM_Spr_SetFlags(&work->layoutman, OBJECT_LEVEL_2, flags);
		}
		else
		{
			/* ＵＶの設定 */
			LOM_SprTex_SetV     (&work->layoutman, OBJECT_MISSION, work->mis_tex_v0 + VRWIN_FONT_TEX_HEIGHT * work->mission);
			LOM_SprTex_SetV     (&work->layoutman, OBJECT_MODE   , work->mod_tex_v0 + VRWIN_FONT_TEX_HEIGHT2 * work->mode   );
			LOM_SprTex_SetHeight(&work->layoutman, OBJECT_MODE   , VRWIN_FONT_TEX_HEIGHT2);
			if(    (work->weapon == MSN_WEAPON_HF_BLADE)
				&& (   (work->player == MSN_PLAYER_SNAKE)
					|| (work->player == MSN_PLAYER_PLISKIN)
					|| (work->player == MSN_PLAYER_TUXEDO)
					|| (work->player == MSN_PLAYER_PREVIOUS)))
			{
				LOM_SprTex_SetV(&work->layoutman, OBJECT_WEAPON , work->wea_tex_v0 + VRWIN_FONT_TEX_HEIGHT * (work->weapon + 1));
			}
			else
			{
				LOM_SprTex_SetV(&work->layoutman, OBJECT_WEAPON , work->wea_tex_v0 + VRWIN_FONT_TEX_HEIGHT * work->weapon);
			}
			SetLevel(work, work->level, OBJECT_LEVEL_2, OBJECT_LEVEL_1);

			/* 表示の設定 */
			if(    (!((work->mode == MSN_MODE_SNEAKING) || (work->mode == MSN_MODE_WEAPON)))
				|| (work->flag & VRWIN_FLAG_TRIAL))
			{
				LOM_Spr_GetFlags(&work->layoutman, OBJECT_HYPHEN, &flags); flags |= SPR_FLAG_HIDDEN;
				LOM_Spr_SetFlags(&work->layoutman, OBJECT_HYPHEN, flags);
				LOM_Spr_GetFlags(&work->layoutman, OBJECT_WEAPON, &flags); flags |= SPR_FLAG_HIDDEN;
				LOM_Spr_SetFlags(&work->layoutman, OBJECT_WEAPON, flags);
			}

			/* サブレベル */
			if(work->sub_level != -1)
			{
				LOM_Spr_GetFlags(&work->layoutman, OBJECT_HYPHEN2 , &flags); flags &= ~SPR_FLAG_HIDDEN;
				LOM_Spr_SetFlags(&work->layoutman, OBJECT_HYPHEN2 , flags);
				LOM_Spr_GetFlags(&work->layoutman, OBJECT_LEVEL_A , &flags); flags &= ~SPR_FLAG_HIDDEN;
				LOM_Spr_SetFlags(&work->layoutman, OBJECT_LEVEL_A , flags);
				LOM_Spr_GetFlags(&work->layoutman, OBJECT_LEVEL_B , &flags); flags &= ~SPR_FLAG_HIDDEN;
				LOM_Spr_SetFlags(&work->layoutman, OBJECT_LEVEL_B , flags);
				SetLevel(work, work->sub_level, OBJECT_LEVEL_B, OBJECT_LEVEL_A);
			}
		}

		/* 高速モード */
		if(GV_PadDataDirect[0].status & (PAD_SEL|PAD_UDLR|PAD_OK|PAD_CANCEL))
		{
			work->flag |= VRWIN_FAST_MODE;
		}
	}

	/* 各フェーズ処理 */
   {
#if BP_USE_NEW_FONT_SYSTEM()
      unsigned char* current = NULL;
      unsigned char* original = NULL;
      int text1Length = strlen(work->text1);
      int text2Length = strlen(work->text2);

      if(work->textOffset < text1Length)
      {
         current = original = work->text1 + work->textOffset;
      }
      else if(work->textOffset < (text1Length + text2Length) )
      {
         current = original = work->text2 + (work->textOffset - text1Length);
      }

      if( current )
      {
         int dummy;
         current = BP_font_decode_utf8_character(&dummy, current);
         work->textOffset += (current - original);
      }
#endif

      work->count++;
   }

	switch(work->phase)
	{
	case VRWIN_PHASE_INIT:				/* 初期化 */
		break;

	case VRWIN_PHASE_WINDOW_OPEN:		/* ウィンドウオープン */
		/* テキスト２の準備 */
		if(!(work->flag & VRWIN_FLAG_COMPACT))
			SetText2(work);

		if(work->count == VRWIN_PHASE_WINDOW_OPEN_COUNT)
		{
			SY_PRINTF1("VRWIN_PHASE_TEXT_SHOW_ANIME\n");
			work->phase = VRWIN_PHASE_TEXT_SHOW_ANIME;
			work->count = work->textOffset = 0;

			MENU_ClearTextTexture(work->text_work);

			/* 高速モードのリセット */
			work->flag &= ~VRWIN_FAST_MODE;
		}
		break;

	case VRWIN_PHASE_TEXT_SHOW_ANIME:	/* テキスト一文字ずつ表示 */
		draw_text  = 1;
		font_color = work->font_color;

		if(    (LOM_GetCurMode(&work->layoutman) != LOM_MODE_07)		/* ４行 開くモードでないか */
			&& ((work->flag & VRWIN_SNAKE_TALES6) == 0))
		{
			if(work->flag & VRWIN_FAST_MODE)
				work->count = work->textOffset = strlen(work->text1) + strlen(work->text2);

			GM_SdSet(SD_S_GO_TYPE1);

			/* テキスト１とテキスト２を順序良くアニメーション */
			if(work->textOffset > strlen(work->text1))
			{
				/* テキスト１は全て表示。テキスト２をアニメーション */
				MENU_CreateTextTexture(work->text_work,
					(work->text_box[TEXT_BOX_1][ORIGIN].x - work->text_box[TEXT_BOX_A][ORIGIN].x) / work->font_scale_h,
					(work->text_box[TEXT_BOX_1][ORIGIN].y - work->text_box[TEXT_BOX_A][ORIGIN].y) / work->font_scale_v,
					 work->text_box[TEXT_BOX_1][RANGE ].x / work->font_scale_h,
					 work->text_box[TEXT_BOX_1][RANGE ].y / work->font_scale_v,
					 VRWIN_FONT_PITCH, 0, 0, work->text1);

#if !BP_USE_NEW_FONT_SYSTEM()
				if(work->text2[(work->textOffset - strlen(work->text1)) - 1] & 0x80)
					work->textOffset++;
#endif

				memset(text2, 0x00, sizeof(text2));
				strncpy(text2, work->text2, work->textOffset - strlen(work->text1));

				MENU_CreateTextTexture(work->text_work,
					(work->text_box[TEXT_BOX_2][ORIGIN].x - work->text_box[TEXT_BOX_A][ORIGIN].x) / work->font_scale_h,
					(work->text_box[TEXT_BOX_2][ORIGIN].y - work->text_box[TEXT_BOX_A][ORIGIN].y) / work->font_scale_v,
					 work->text_box[TEXT_BOX_2][RANGE ].x / work->font_scale_h,
					 work->text_box[TEXT_BOX_2][RANGE ].y / work->font_scale_v,
					 VRWIN_FONT_PITCH, 0, 0, text2);
			}
			else
			{
				/* テキスト１をアニメーション */
#if !BP_USE_NEW_FONT_SYSTEM()
				if(work->text1[work->textOffset - 1] & 0x80)
					work->textOffset++;
#endif

				memset(text1, 0x00, sizeof(text1));
				strncpy(text1, work->text1, work->textOffset);

				MENU_CreateTextTexture(work->text_work,
					(work->text_box[TEXT_BOX_1][ORIGIN].x - work->text_box[TEXT_BOX_A][ORIGIN].x) / work->font_scale_h,
					(work->text_box[TEXT_BOX_1][ORIGIN].y - work->text_box[TEXT_BOX_A][ORIGIN].y) / work->font_scale_v,
					 work->text_box[TEXT_BOX_1][RANGE ].x / work->font_scale_h,
					 work->text_box[TEXT_BOX_1][RANGE ].y / work->font_scale_v,
					 VRWIN_FONT_PITCH, 0, 0, text1);
			}

			if(work->textOffset >= strlen(work->text1) + strlen(work->text2))
			{
				SY_PRINTF1("VRWIN_PHASE_PICT_SHOW_ANIME\n");
				work->phase = VRWIN_PHASE_PICT_SHOW_ANIME;
				work->count = work->textOffset = 0;

				/* 写真を表示に */
				if(work->pict != NULL)
				{
					pict_pos.x = (work->text_box[TEXT_BOX_A][ORIGIN].x + work->text_box[TEXT_BOX_A][RANGE ].x + VRWIN_TEXT_BOX_MARGIN_R - VRWIN_PICT_WIDTH ) + work->pict_pos.x;
					pict_pos.y = (work->text_box[TEXT_BOX_A][ORIGIN].y - VRWIN_TEXT_BOX_MARGIN_T) + work->pict_pos.y;
					SPR_SetPosSprite(work->pict, &pict_pos);
					SY_PRINTF1("Pict Pos: x=%f, Y=%f\n", pict_pos.x, pict_pos.y);
					work->pict->head.flags &= ~SPR_FLAG_HIDDEN;
					SPR_SetColorSprite(work->pict, VRWIN_PICT_COLOR_R, VRWIN_PICT_COLOR_G, VRWIN_PICT_COLOR_B, 0);
				}
			}
		}
		else
		{
			if(work->flag & VRWIN_FAST_MODE)
				work->count = VRWIN_PHASE_PICT_HIDE_ANIME_COUNT;

			/* テキストがフェードイン */
			alpha = work->font_color >> 24;
			alpha = (alpha * work->count) / VRWIN_PHASE_TEXT_HIDE_ANIME_COUNT;
			font_color = (work->font_color & 0x00ffffff) | (alpha << 24);

			MENU_CreateTextTexture(work->text_work,
				(work->text_box[TEXT_BOX_1][ORIGIN].x - work->text_box[TEXT_BOX_A][ORIGIN].x) / work->font_scale_h,
				(work->text_box[TEXT_BOX_1][ORIGIN].y - work->text_box[TEXT_BOX_A][ORIGIN].y) / work->font_scale_v,
				 work->text_box[TEXT_BOX_1][RANGE ].x / work->font_scale_h,
				 work->text_box[TEXT_BOX_1][RANGE ].y / work->font_scale_v,
				 VRWIN_FONT_PITCH, 0, 0, work->text1);

			if(work->count == VRWIN_PHASE_PICT_HIDE_ANIME_COUNT)
			{
				SY_PRINTF1("VRWIN_PHASE_PICT_SHOW_ANIME\n");
				work->phase = VRWIN_PHASE_PICT_SHOW_ANIME;
				work->count = 0;

				/* 写真を表示に */
				if(work->pict != NULL)
				{
					pict_pos.x = (work->text_box[TEXT_BOX_A][ORIGIN].x + work->text_box[TEXT_BOX_A][RANGE ].x + VRWIN_TEXT_BOX_MARGIN_R - VRWIN_PICT_WIDTH ) + work->pict_pos.x;
					pict_pos.y = (work->text_box[TEXT_BOX_A][ORIGIN].y - VRWIN_TEXT_BOX_MARGIN_T) + work->pict_pos.y;
					SPR_SetPosSprite(work->pict, &pict_pos);
					SY_PRINTF1("Pict Pos: x=%f, Y=%f\n", pict_pos.x, pict_pos.y);
					work->pict->head.flags &= ~SPR_FLAG_HIDDEN;
					SPR_SetColorSprite(work->pict, VRWIN_PICT_COLOR_R, VRWIN_PICT_COLOR_G, VRWIN_PICT_COLOR_B, 0);
				}
			}
		}
		break;

	case VRWIN_PHASE_PICT_SHOW_ANIME:	/* 写真表示アニメ */
		if(work->flag & VRWIN_FAST_MODE)
			work->count = VRWIN_PHASE_PICT_SHOW_ANIME_COUNT;

		draw_text  = 1;
		font_color = work->font_color;

		if(work->pict == NULL)
		{
			work->count = VRWIN_PHASE_PICT_SHOW_ANIME_COUNT;
		}
		else
		{
			/* 写真がフェードイン */
			SPR_SetColorSprite(work->pict, VRWIN_PICT_COLOR_R, VRWIN_PICT_COLOR_G, VRWIN_PICT_COLOR_B,
				(VRWIN_PICT_COLOR_A * work->count) / VRWIN_PHASE_PICT_SHOW_ANIME_COUNT);
		}

		if(work->count == VRWIN_PHASE_PICT_SHOW_ANIME_COUNT)
		{
			SY_PRINTF1("VRWIN_PHASE_SHOW_IDLE\n");
			work->phase = VRWIN_PHASE_SHOW_IDLE;
			work->count = 0;
		}
		break;

	case VRWIN_PHASE_SHOW_IDLE:			/* 表示アイドル */
		draw_text  = 1;
		font_color = work->font_color;

		/* 点滅 */
		if(work->flag & VRWIN_FLAG_BLINK_FO)
		{
			work->count -= 2;
			if(work->count <= 0)
				work->flag &= ~VRWIN_FLAG_BLINK_FO;
		}
		else
		{
			if(work->count >= VRWIN_BLINK_COUNT)
				work->flag |= VRWIN_FLAG_BLINK_FO;
		}
		LOM_Spr_GetColor(&work->layoutman, OBJECT_RECT, &col);
		col.a = (128 * work->count) / VRWIN_BLINK_COUNT;
		LOM_Spr_SetColor(&work->layoutman, OBJECT_RECT, &col);
		break;

	case VRWIN_PHASE_PICT_HIDE_ANIME:	/* 写真消去アニメ */
		draw_text  = 1;
		font_color = work->font_color;

		if(work->pict == NULL)
		{
			work->count = VRWIN_PHASE_PICT_HIDE_ANIME_COUNT;
		}
		else
		{
			/* 写真がフェードアウト */
			SPR_SetColorSprite(work->pict, VRWIN_PICT_COLOR_R, VRWIN_PICT_COLOR_G, VRWIN_PICT_COLOR_B,
				(VRWIN_PICT_COLOR_A * (VRWIN_PHASE_PICT_SHOW_ANIME_COUNT - work->count)) / VRWIN_PHASE_PICT_SHOW_ANIME_COUNT);
		}

		if(work->count == VRWIN_PHASE_PICT_HIDE_ANIME_COUNT)
		{
			SY_PRINTF1("VRWIN_PHASE_TEXT_HIDE_ANIME\n");
			work->phase = VRWIN_PHASE_TEXT_HIDE_ANIME;
			work->count = 0;

			/* 写真を非表示に */
			if(work->pict != NULL)
				work->pict->head.flags |= SPR_FLAG_HIDDEN;
		}
		break;

	case VRWIN_PHASE_TEXT_HIDE_ANIME:	/* テキストフェードアウト */
		draw_text  = 1;

		/* テキストがフェードアウト */
		alpha = work->font_color >> 24;
		alpha = (alpha * (VRWIN_PHASE_TEXT_HIDE_ANIME_COUNT - work->count)) / VRWIN_PHASE_TEXT_HIDE_ANIME_COUNT;
		font_color = (work->font_color & 0x00ffffff) | (alpha << 24);

		if(work->count == VRWIN_PHASE_TEXT_HIDE_ANIME_COUNT)
		{
			SY_PRINTF1("VRWIN_PHASE_WINDOW_CLOSE\n");
			work->phase = VRWIN_PHASE_WINDOW_CLOSE;
			work->count = 0;

			/* レイアウトマネージャ */
			if     (LOM_GetCurMode(&work->layoutman) == LOM_MODE_07) LOM_SetCurMode(&work->layoutman, LOM_MODE_08, 1, 1, 1);
			else if(LOM_GetCurMode(&work->layoutman) == LOM_MODE_04) LOM_SetCurMode(&work->layoutman, LOM_MODE_05, 1, 1, 1);
			else                                                     LOM_SetCurMode(&work->layoutman, LOM_MODE_02, 1, 1, 1);
		}
		break;

	case VRWIN_PHASE_WINDOW_CLOSE:		/* ウィンドウクローズ */
		work->bgmvol -= GM_MAX_VOL / 10 ;
		if ( work->bgmvol < 0 ) work->bgmvol = 0 ;
//		GM_VoxStreamSetPan( work->bgmhandler, work->bgmvol, GM_PAN_CENTER ) ;
		break;

	case VRWIN_PHASE_HIDE_IDLE:			/* 非表示アイドル */
		if(work->flag & VRWIN_STREAM_STOP)
		{
			if ( work->count == DIRECT_TICK(15)  ) {
				GM_StreamStop( work->bgmhandler ) ;
			}
		}
		break;
	}

	/* テキストの描画 */
	if(draw_text != 0)
	{
		if(    (LOM_GetCurMode(&work->layoutman) != LOM_MODE_07)		/* ４行 開くモードでないか */
			&& ((work->flag & VRWIN_SNAKE_TALES6) == 0))
		{
			MENU_PutTextScreen(work->text_work,
				work->text_box[TEXT_BOX_A][ORIGIN].x,
				work->text_box[TEXT_BOX_A][ORIGIN].y,
				work->text_box[TEXT_BOX_A][ORIGIN].x + work->text_box[TEXT_BOX_A][RANGE ].x,
				work->text_box[TEXT_BOX_A][ORIGIN].y + work->text_box[TEXT_BOX_A][RANGE ].y,
				0,
				0,
				work->text_box[TEXT_BOX_A][RANGE ].x / work->font_scale_h,
				work->text_box[TEXT_BOX_A][RANGE ].y / work->font_scale_v,
				font_color);
		}
		else
		{
			MENU_PutTextScreen(work->text_work,
				work->text_box[TEXT_BOX_1][ORIGIN].x,
				work->text_box[TEXT_BOX_1][ORIGIN].y,
				work->text_box[TEXT_BOX_1][ORIGIN].x + work->text_box[TEXT_BOX_1][RANGE ].x,
				work->text_box[TEXT_BOX_1][ORIGIN].y + work->text_box[TEXT_BOX_1][RANGE ].y,
				0,
				0,
				work->text_box[TEXT_BOX_1][RANGE ].x / work->font_scale_h,
				work->text_box[TEXT_BOX_1][RANGE ].y / work->font_scale_v,
				font_color);
		}
	}
}

/******************************************************************************
 * 終了処理
 */
static void Die(
	WORK *work)		/* ワーク */
{
	SY_PRINTF3("Die\n");

	/* スプライトの解放 */
	if(work->pict != NULL)
		if(SPR_Destroy_2D_Object(work->pict) == -1)
			SY_PRINTF2("Destroy Sprite Failed.\n");

	/* tri の解放 */
	if(work->pict_tri != -1)
		if(SPR_KillTexture(work->pict_tri) == -1)
			SY_PRINTF2("Kill TRI Failed.\n");

	/* レイアウトマネージャ */
	DestroyLayoutman(&work->layoutman);
}

/******************************************************************************
 * 初期化処理

chara ＶＲウィンドウ[NewVRWindow_Scn] $s:名前 \
	-message $d:メッセージ ... \
	// 以下オプション
	-text_A      $d:制限時間 ... \
	-text_B      $d:分 ... \
	-text_C      $d:秒 ... \
	-text_D      $d:敵兵数 ... \
	-text_E      $d:人 ... \
	-text_F      $d:標的数 ... \
	-text_G      $d:個 ... \
	-text_H      $d:爆弾数 ... \
	-text_I      $d:個 ... \
	-picT        $s:tri名  $s:テクスチャ名 \											// 写真用のテクスチャ
	-x_pict      $i:写真表示位置Ｘ（右上からの相対） \									// 1/100 単位
	-y_pict      $i:写真表示位置Ｙ（右上からの相対） \									// 1/100 単位
	-color       $b:フォント色Ｒ  $b:フォント色Ｇ  $b:フォント色Ｂ  $b:フォント色Ａ \
	-h_scale     $i:フォントスケール横 \												// 1/100 単位
	-v_scale     $i:フォントスケール縦 \												// 1/100 単位
	-Proc        $p:閉じたときプロック \
	-flag        $w:フラグ \
	-bgM         $t:スタートＢＧＭ \
	-snake_tales $i:ステージ番号（０～４） \
	-subLevel    $i:サブレベル

#define VRWIN_FLAG_JAPANESE   (0x0001)	// 日本語モード
#define VRWIN_FLAG_NO_L_SPACE (0x0002)	// 行間なしモード
#define VRWIN_FLAG_COMPACT    (0x0004)	// コンパクトモード
#define VRWIN_FLAG_7LINES     (0x0008)	// ７行モード
#define VRWIN_FLAG_TRIAL      (0x0010)	// 体験版モード（Ｌ２Ｄファイルが変わる）
#define VRWIN_HIDE_START      (0x0080)	// ステージ開始時に表示しないでスタートモード
#define VRWIN_SNAKE_TALES6    (0x0200)	// スネークテイルズ６行モード
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	WORK *work,		/* ワーク */
	int  name,		/* シナリオ名 */
	int  where)		/* マップＩＤ */
{
	int tri = 0;
	int tex = 0;

	SY_PRINTF3("GetResources\n");

	/* 初期化 */
	work->name         = name;
	work->proc_id      = 0;

	work->mission      = 0;
	work->mode         = 0;
	work->weapon       = 0;
	work->level        = 0;
	work->mis_tex_v0   = 0;
	work->mod_tex_v0   = 0;
	work->wea_tex_v0   = 0;
	work->lev_tex_u0   = 0;
	work->font_color   = VRDEF_CLEARED_A;
	work->font_scale_h = VRWIN_FONT_SCALE_H_ENG;
	work->font_scale_v = VRWIN_FONT_SCALE_V_ENG;

	work->pict_pos.x   = VRWIN_PICT_POS_X;
	work->pict_pos.y   = VRWIN_PICT_POS_Y;
	work->pict         = NULL;
	work->pict_tri     = -1;

	memset(&work->text_box[0][0], 0x00, sizeof(work->text_box));
	work->text_work    = NULL;
	work->text1        = NULL;
	memset(&work->text2, 0x00, sizeof(work->text2));
	work->text_a       = _nostring;
	work->text_b       = _nostring;
	work->text_c       = _nostring;
	work->text_d       = _nostring;
	work->text_e       = _nostring;
	work->text_f       = _nostring;
	work->text_g       = _nostring;
	work->text_h       = _nostring;
	work->text_i       = _nostring;
	work->flag         = 0;
	work->phase        = VRWIN_PHASE_INIT;
	work->count        = 0;
	work->sub_level    = -1;

	/* シナリオオプション */
	if(name != 0)
	{
		int i, j;

		/* 言語の設定 */
		j = GM_Language;
		if((j < 0) || (j > 6))
			j = 0;
		j++;
		SY_PRINTF1("Current Language: %d\n", j);

		/* message */
		if(GCL_GetOption('m') == NULL)
		{
			SY_PRINTF2("no message parameter.\n");
			return 0;
		}
		for(i = 0; i < j; i++) work->text1 = BP_GCL_LOOKUP_NEW_FONT_STRING(GCL_GetNextString());
		if(strlen(work->text1) >= VRWIN_TEXT_1_LENGTH)
		{
			SY_PRINTF2("Text 1 is too long.\n");
			ASSERT(0);
			return 0;
		}

		if(GCL_GetOption('A') != NULL) for(i = 0; i < j; i++) work->text_a = BP_GCL_LOOKUP_NEW_FONT_STRING(GCL_GetNextString());
		if(GCL_GetOption('B') != NULL) for(i = 0; i < j; i++) work->text_b = BP_GCL_LOOKUP_NEW_FONT_STRING(GCL_GetNextString());
		if(GCL_GetOption('C') != NULL) for(i = 0; i < j; i++) work->text_c = BP_GCL_LOOKUP_NEW_FONT_STRING(GCL_GetNextString());
		if(GCL_GetOption('D') != NULL) for(i = 0; i < j; i++) work->text_d = BP_GCL_LOOKUP_NEW_FONT_STRING(GCL_GetNextString());
		if(GCL_GetOption('E') != NULL) for(i = 0; i < j; i++) work->text_e = BP_GCL_LOOKUP_NEW_FONT_STRING(GCL_GetNextString());
		if(GCL_GetOption('F') != NULL) for(i = 0; i < j; i++) work->text_f = BP_GCL_LOOKUP_NEW_FONT_STRING(GCL_GetNextString());
		if(GCL_GetOption('G') != NULL) for(i = 0; i < j; i++) work->text_g = BP_GCL_LOOKUP_NEW_FONT_STRING(GCL_GetNextString());
		if(GCL_GetOption('H') != NULL) for(i = 0; i < j; i++) work->text_h = BP_GCL_LOOKUP_NEW_FONT_STRING(GCL_GetNextString());
		if(GCL_GetOption('I') != NULL) for(i = 0; i < j; i++) work->text_i = BP_GCL_LOOKUP_NEW_FONT_STRING(GCL_GetNextString());

		/* picT */
		if(GCL_GetOption('T') != NULL)
		{
			tri = GCL_GetNextInt();
			tex = GCL_GetNextInt();
		}

		/* flag */
		if(GCL_GetOption('f') != NULL)
		{
			work->flag = GCL_GetNextInt();

			/* 念のため補正 */
			if(work->flag & VRWIN_FLAG_COMPACT)
			{
				work->flag &= ~VRWIN_FLAG_7LINES;
			}

			if(work->flag & VRWIN_HIDE_START)
			{
				work->flag &= ~VRWIN_FLAG_7LINES;
				work->flag |=  VRWIN_FLAG_COMPACT|VRWIN_FLAG_PROC_CALLED;
				MSN_2DSTATUS |= MSN_2DSTAT_QUICK_WINDOW;
			}

			if(work->flag & VRWIN_FLAG_JAPANESE)
			{
				work->font_scale_h = VRWIN_FONT_SCALE_H_JPN;
				work->font_scale_v = VRWIN_FONT_SCALE_V_JPN;
			}
		}

		/* x_pict */
		if(GCL_GetOption('x') != NULL)
			work->pict_pos.x = GCL_GetNextInt() / 100.0f;

		/* y_pict */
		if(GCL_GetOption('y') != NULL)
			work->pict_pos.y = GCL_GetNextInt() / 100.0f;

		/* color */
		if(GCL_GetOption('c') != NULL)
		{
			work->font_color  = GCL_GetNextInt();
			work->font_color |= GCL_GetNextInt() << 8;
			work->font_color |= GCL_GetNextInt() << 16;
			work->font_color |= GCL_GetNextInt() << 24;
		}

		/* h_scale */
		if(GCL_GetOption('h') != NULL)
			work->font_scale_h = GCL_GetNextInt() / 100.0f;

		/* v_scale */
		if(GCL_GetOption('v') != NULL)
			work->font_scale_v = GCL_GetNextInt() / 100.0f * VRWIN_VITA_FONT_EXTRA_SCALE;

		/* Proc */
		if(GCL_GetOption('P') != NULL)
			work->proc_id = GCL_GetNextInt();

		/* Start BGM */
		if(GCL_GetOption('M') != NULL) {
			int val ;
			
			val = GCL_GetNextInt() ;
			work->bgmhandler = GM_VoxStream( val, 0 ) ;
			work->bgmvol = GM_MAX_VOL ;
			GM_VoxStreamSetPan( work->bgmhandler, work->bgmvol, GM_PAN_CENTER, 0.f ) ;
		}

		/* snake_tales */
		if(GCL_GetOption('s') != NULL)
		{
			work->level = GCL_GetNextInt();
			if((work->level < 0) || (4 < work->level))
			{
				SY_PRINTF2("Wrong Stage Number Value.\n");
				ASSERT(0);
			}
			work->flag |= VRWIN_SNAKE_TALES|VRWIN_HIDE_START|VRWIN_FLAG_COMPACT|VRWIN_FLAG_PROC_CALLED;
			work->flag &= ~VRWIN_FLAG_7LINES;
			MSN_2DSTATUS |= MSN_2DSTAT_QUICK_WINDOW;
		}
		else
		{
			work->flag &= ~VRWIN_SNAKE_TALES6;
		}

		/* subLevel */
		if(GCL_GetOption('L') != NULL)
		{
			work->sub_level = GCL_GetNextInt();
		}

		SY_PRINTF1("Get Scn Option Succeeded.\n");
	}

	/* 表示状態で起動 */
	if(!(work->flag & VRWIN_HIDE_START))
	{
		GM_VRStatus |= GM_VR_WINDOW_OPEN;
	}

	/* レイアウトマネージャ */
	{
		if(work->flag & VRWIN_FLAG_TRIAL)
		{
			SY_PRINTF1("Trial Mode.\n");
			if(CreateLayoutman4(&work->layoutman, L2D_FILENAME_T, 4, 0, 0, 0, 0, 0, LOMCallback, NULL, work, 0, 0, LOM_INPUT_DIRECT) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
		}
		else
		{
			if(CreateLayoutman4(&work->layoutman, L2D_FILENAME, 4, 0, 0, 0, 0, 0, LOMCallback, NULL, work, 0, 0, LOM_INPUT_DIRECT) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
		}
		SY_PRINTF1("Create Layoutman Succeeded.\n");

		if(LoadLOMData(&work->layoutman, _lom_data) == 0) { SY_PRINTF2("Load LOM Data Failed.\n"); return 0; }
		SY_PRINTF1("Load LOM Data Succeeded.\n");
	}

	/* ステージモードの取得 */
	{
		if(work->flag & VRWIN_FLAG_TRIAL)
		{
			work->mission = 0;
			work->weapon  = 0;
			switch(VR_STAGE_ID)
			{
			case VRDEF_TRIAL_1: work->level = 1; work->mode = 0; break;
			case VRDEF_TRIAL_2: work->level = 2; work->mode = 1; break;
			case VRDEF_TRIAL_3: work->level = 3; work->mode = 2; break;
			case VRDEF_TRIAL_4: work->level = 4; work->mode = 3; break;
			case VRDEF_TRIAL_5: work->level = 5; work->mode = 4; break;
			default           : ASSERT(0); break;
			}
		}
		else if(!(work->flag & VRWIN_SNAKE_TALES))
		{
			Msn_GetCurrentStageInfo(&work->mission, &work->mode, &work->weapon, &work->level, &work->player);
			work->level++;
		}
		SY_PRINTF1("Current Stage Info: %d-%d-%d-%d.\n", work->mission, work->mode, work->weapon, work->level);
	}

	/* メッセージ２ */
	{
		if(    ((strlen(work->text_a) + 4 + strlen(work->text_b) + 4 + strlen(work->text_c) + 1 + strlen(work->text_d) + 5 + strlen(work->text_e)) + 1 > sizeof(work->text2))
			|| ((strlen(work->text_a) + 4 + strlen(work->text_b) + 4 + strlen(work->text_c) + 1 + strlen(work->text_f) + 5 + strlen(work->text_g)) + 1 > sizeof(work->text2))
			|| ((strlen(work->text_a) + 4 + strlen(work->text_b) + 4 + strlen(work->text_c) + 1 + strlen(work->text_h) + 5 + strlen(work->text_i)) + 1 > sizeof(work->text2)))
		{
			SY_PRINTF2("Text 2 is too long.\n");
			return 0;
		}
		SetText2(work);
		SY_PRINTF1("Text 2 Set.\n");
	}

	/* 写真のロード */
	if((tri != 0) && (tex != 0))
	{
		if((work->pict_tri = SPR_LoadTexture(tri)) == -1)
		{
			SY_PRINTF2("Load TRI[%d] Failed.\n", tri);
			return 0;
		}
		if((work->pict = SPR_Create_2D_Object(SP_SPRITE, 4, NULL)) == NULL)
		{
			SY_PRINTF2("Create Sprite Failed.\n");
			return 0;
		}
		if(SPR_ObjSetTexture(work->pict, tex, work->pict_tri) == -1)
		{
			SY_PRINTF2("Set Texture Failed.\n");
			return 0;
		}
		SPR_SetPriority(work->pict, SPR_PRI_MOST_NEAR);
		SPR_SetPosSprite  (work->pict, &work->pict_pos);	// 仮の位置
		SPR_SetSizeSprite (work->pict, VRWIN_PICT_WIDTH, VRWIN_PICT_HEIGHT);
		SPR_SetColorSprite(work->pict, VRWIN_PICT_COLOR_R, VRWIN_PICT_COLOR_G, VRWIN_PICT_COLOR_B, 0);
		work->pict->head.flags |= SPR_FLAG_HIDDEN | SPR_FLAG_ALPHA;
		work->pict->head.alpha  = SCE_GS_SET_ALPHA(0, 1, 0, 1, 0x00);

		SY_PRINTF1("Set Picture Succeeded.\n");
	}

	work->flag |= VRWIN_FLAG_INIT;

	return 1;
}

/*******************************************************************************
 * public
 */
/******************************************************************************
 * 起動処理
enum
{
	GV_ACTOR_DAEMON,	// システムデーモン
	GV_ACTOR_MANAGER,	// ゲーム進行プロセス
	GV_ACTOR_ASSIST,	// ゲーム補助プロセス

	GV_ACTOR_PREV2,		// メインより前に実行されるキャラクタ (ポーズ時も動作)
	GV_ACTOR_PREV,		// メインより前に実行されるキャラクタ

	GV_ACTOR_PLAYER,	// 通常キャラクタ (プレイヤー)
	GV_ACTOR_USER,		// 通常キャラクタ
	GV_ACTOR_CAMERA,	// カメラ (ポーズ時も動作)

	GV_ACTOR_AFTER,		// メインより後に実行されるキャラクタ
	GV_ACTOR_EFFECT,	// エフェクト
	GV_ACTOR_AFTER2,	// メインより後に実行されるキャラクタ (ポーズ時も動作)

	GV_ACTOR_DAEMON2,	// システムデーモン２
	GV_ACTOR_LEVEL_MAX
};
 */
void *NewVRWindow(void)
{
	WORK *work;

	SY_PRINTF3("NewVRWindow\n");
	OPERATOR();

	/* アクター生成 */
	work = (WORK *)GV_CreateActor(
		GV_ACTOR_MANAGER,			/* プロセスの優先順位 */
		GV_CLASS_CHARA,				/* プロセスクラス */
		sizeof(WORK),				/* メモリ確保サイズ */
		0xff);						/* 実行プライオリティ */
	if(work != NULL)
	{
		/* アクター登録 */
		GV_SetActor(&work->actor, Act, Die);
		GV_ActorEX(&work->actor);

		/* ワークの初期化 */
		if(!GetResources(work, 0, GM_CurrentMap))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}

void *NewVRWindow_Scn(
	int name,	/* シナリオ名 */
	int where)	/* マップＩＤ */
{
	WORK *work;

	SY_PRINTF3("NewVRWindow_Scn\n");
	OPERATOR();

	/* アクター生成 */
	work = (WORK *)GV_CreateActor(
		GV_ACTOR_MANAGER,			/* プロセスの優先順位 */
		GV_CLASS_CHARA,				/* プロセスクラス */
		sizeof(WORK),				/* メモリ確保サイズ */
		0xff);						/* 実行プライオリティ */
	if(work != NULL)
	{
		/* アクター登録 */
		GV_SetActor(&work->actor, Act, Die);
		GV_ActorEX(&work->actor);

		/* ワークの初期化 */
		if(!GetResources(work, name, where))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}
