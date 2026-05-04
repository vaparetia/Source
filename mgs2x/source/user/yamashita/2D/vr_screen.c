//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * vr_screen - vr_screen.c
 * ＶＲスクリーン  *NewVRScreen
 * 2002/04/11 S.Yamashita
 * $Id: vr_screen.c,v 1.3 2002/12/11 14:07:51 takaki Exp $
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

#define __CHARA_NAME__ "VR Screen"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
//#define SY_PRINTF3_DEBUG
#include "../sy_util/sy_util.h"

/*******************************************************************************
 * defines
 */

/* フェーズタイプ */
enum {
	VRSCR_PHASE_IDLE = 0,			/* アイドル */
	VRSCR_PHASE_HIDE,				/* 非表示アイドル */
	VRSCR_PHASE_FADE_IN,			/* フェードイン */
	VRSCR_PHASE_FADE_OUT,			/* フェードアウト */
};


extern int BP_AdjustTick2(int);
#define VRSCR_FADE_IN_COUNT  (BP_AdjustTick(15))	/* フェードイン１カウント */
#define VRSCR_FADE_OUT_COUNT (BP_AdjustTick(20))	/* フェードアウトカウント */

/* フラグ */
#define VRSCR_INIT         (0x0001)			/* 初期化時 */
#define VRSCR_INIT2        (0x0002)			/* 初期化時２ */
#define VRSCR_INIT3        (0x0004)			/* 初期化時３ */
#define VRSCR_OPENFRAME    (0x0008)			/* フレームをオープン中 */
#define VRSCR_MENU_ON      (0x0010)			/* メニューを表示中 */

#define VRSCR_HIDE_FRAME   (0x0100)			/* 枠を非表示 */
#define VRSCR_HIDE_COUNTER (0x0200)			/* カウンターを非表示 */
#define VRSCR_FIVE         (0x0400)			/* ５桁モード */
#define VRSCR_WEAPON2      (0x0800)			/* ＷＥＡＰＯＮ２　　　ＭＯＤＥ（ターゲット、　　　　タイム） */
#define VRSCR_WEAPON       (0x1000)			/* ＷＥＡＰＯＮ　　　　ＭＯＤＥ（ターゲット、スコア、タイム） */
#define VRSCR_SNEAKING     (0x2000)			/* ＳＮＥＡＫＩＮＧ　　ＭＯＤＥ（　　　　　　　　　　タイム） */
#define VRSCR_BOMB         (0x4000)			/* ＢＯＭＢ　　　　　　ＭＯＤＥ（ボム、　　　　　　　タイム） */
#define VRSCR_ELIMINATE    (0x8000)			/* ＥＬＩＭＩＮＡＴＥ　ＭＯＤＥ（エネミー、　　　　　タイム） */

/* 設定値 */
#define VRSCR_FRM_X_RANGE     (30.0f)		/* 枠の横方向範囲 */
#define VRSCR_FRM_Y_RANGE     (30.0f)		/* 縦の横方向範囲 */
#define VRSCR_FRM_MOVE_DIV    (60.0f)		/* 枠移動の速度（分母） */
#define VRSCR_TEX_WIDTH       (18.0f)		/* テクスチャ幅 */

/* Ｌ２Ｄオブジェクト */
#define L2D_FILENAME          ( 4523451)	/* vr_system */
#define L2D_FILENAME_2        (11452065)	/* vr_system_five */

#define ACTION_DUMMY          ( 8143391)	/* dummy */
#define ACTION_DUMMY2         ( 8930337)	/* dummy2 */
#define ACTION_OPEN_FRAME     (15586903)	/* open_frame */
#define ACTION_WEAPON_IDLE    ( 8436842)	/* vr_game_weapon_s */
#define ACTION_WEAPON_OFF     (14243052)	/* vr_game_weapon_s_off */
#define ACTION_WEAPON_ON      ( 3590828)	/* vr_game_weapon_s_on */
#define ACTION_SNEAKING_IDLE  ( 9342085)	/* vr_game_sneaking */
#define ACTION_SNEAKING_OFF   ( 9056750)	/* vr_game_sneaking_off */
#define ACTION_SNEAKING_ON    ( 4477332)	/* vr_game_sneaking_on */
#define ACTION_BOMB_IDLE      (11249159)	/* vr_game_bomb_s */
#define ACTION_BOMB_OFF       (11273094)	/* vr_game_bomb_s_off */
#define ACTION_BOMB_ON        (  352289)	/* vr_game_bomb_s_on */
#define ACTION_ELIMINATE_IDLE ( 4146922)	/* vr_game_eliminate_s */
#define ACTION_ELIMINATE_OFF  (13974932)	/* vr_game_eliminate_s_off */
#define ACTION_ELIMINATE_ON   ( 7776753)	/* vr_game_eliminate_s_on */
#define ACTION_WEAPON2_IDLE   ( 1543554)	/* vr_game_weapon_s2 */
#define ACTION_WEAPON2_OFF    ( 5423614)	/* vr_game_weapon_s2_off */
#define ACTION_WEAPON2_ON     (12752404)	/* vr_game_weapon_s2_on */
#define ACTION_COUNTER_OFF    (13150804)	/* vr_game_time_off */

/* アルファ 128 系 */
#define OBJECT_TARGETS        ( 5587132)	/* targets */
#define OBJECT_BOMBS          ( 5849273)	/* bombs */
#define OBJECT_ENEMIES        ( 8932149)	/* enemies */
#define OBJECT_TARGET_6       (13025645)	/* target_num_6 */
#define OBJECT_TARGET_5       (13025644)	/* target_num_5 */
#define OBJECT_TARGET_4       (13025643)	/* target_num_4 */
#define OBJECT_SLASH          (14941922)	/* target_num_slash */
#define OBJECT_TARGET_3       (13025642)	/* target_num_3 */
#define OBJECT_TARGET_2       (13025641)	/* target_num_2 */
#define OBJECT_TARGET_1       (13025640)	/* target_num_1 */
#define OBJECT_TIME           ( 3912197)	/* time */
#define OBJECT_TIME_6         (  573994)	/* time_num_6 */
#define OBJECT_TIME_5         (  573993)	/* time_num_5 */
#define OBJECT_COLON2         (12061788)	/* time_colon2 */
#define OBJECT_TIME_4         (  573992)	/* time_num_4 */
#define OBJECT_TIME_3         (  573991)	/* time_num_3 */
#define OBJECT_COLON1         (12061787)	/* time_colon1 */
#define OBJECT_TIME_2         (  573990)	/* time_num_2 */
#define OBJECT_TIME_1         (  573989)	/* time_num_1 */
#define OBJECT_SCORE          ( 6507180)	/* score */
#define OBJECT_SCORE_7        (15657972)	/* score_num_7 */
#define OBJECT_SCORE_6        (15657971)	/* score_num_6 */
#define OBJECT_SCORE_5        (15657970)	/* score_num_5 */
#define OBJECT_SCORE_4        (15657969)	/* score_num_4 */
#define OBJECT_SCORE_3        (15657968)	/* score_num_3 */
#define OBJECT_SCORE_2        (15657967)	/* score_num_2 */
#define OBJECT_SCORE_1        (15657966)	/* score_num_1 */
/* アルファ 42 系 */
#define OBJECT_LN_FRAME1      (14505604)	/* wireFrame01 */
#define OBJECT_LN_FRAME2      (14505605)	/* wireFrame02 */
#define OBJECT_LN_FRAME3      (14505606)	/* wireFrame03 */
#define OBJECT_LN_FRAME4      (14505607)	/* wireFrame04 */
#define OBJECT_LN_FRAME5      (14505608)	/* wireFrame05 */
#define OBJECT_LN_FRAME6      (14505609)	/* wireFrame06 */
#define OBJECT_LN_FRAME7      (14505610)	/* wireFrame07 */
#define OBJECT_LN_FRAME8      (14505611)	/* wireFrame08 */
#define OBJECT_VRSYSTEM       (13576762)	/* vrSystem */
#define OBJECT_FRAME02        ( 4730012)	/* Frame02 */
#define OBJECT_FRAME01        ( 4730011)	/* Frame01 */
#define OBJECT_LN_LINE3       ( 7097141)	/* line01_3 */
#define OBJECT_LN_LINE4       ( 7097142)	/* line01_4 */
#define OBJECT_LN_LINE1       ( 7097139)	/* line01_1 */
#define OBJECT_LN_LINE2       ( 7097140)	/* line01_2 */

#define OBJECT_WIREFRAME      ( 1374036)	/* wireFrame */

/*******************************************************************************
 * work
 */

/* ワーク */
typedef struct tagWORK
{
	GV_ACT_EX actor;		/* アクター */

	int       name;			/* シナリオ名 */
	float     dig_tex_u0;	/* 数値テクスチャのＵ値の原点 */
	short     phase;		/* フェーズ */
	short     count;		/* カウンター */
	short     count2;		/* カウンター２ */
	short     flag;			/* フラグ */

	SPR_POS   frm_org;		/* フレーム原点 */
	SPR_POS   frm_pos;		/* フレーム位置 */

	LAYOUTMAN layoutman;	/* レイアウトマネージャ */
}
WORK;

/*******************************************************************************
 * local
 */

/* ＬＯＭデータ */
static int _lom_data[] = {
					/*アクションリスト名*/		/*アクション名*/		/*カウント*/	/*フラグ*/
LOM_ACTLIST_DATA,	LOM_ACTLIST_00,				ACTION_WEAPON_IDLE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_01,				ACTION_WEAPON_ON,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_WEAPON_IDLE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_02,				ACTION_WEAPON_OFF,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_03,				ACTION_SNEAKING_IDLE,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_04,				ACTION_SNEAKING_ON,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_SNEAKING_IDLE,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_05,				ACTION_SNEAKING_OFF,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_06,				ACTION_BOMB_IDLE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_07,				ACTION_BOMB_ON,			1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BOMB_IDLE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_08,				ACTION_BOMB_OFF,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_09,				ACTION_ELIMINATE_IDLE,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_10,				ACTION_ELIMINATE_ON,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_ELIMINATE_IDLE,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_11,				ACTION_ELIMINATE_OFF,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_12,				ACTION_COUNTER_OFF,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_13,				ACTION_OPEN_FRAME,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_DUMMY,			1,				LOM_ACT_FRAME|LOM_ACT_SHOW,
												ACTION_DUMMY2,			1,				LOM_ACT_FRAME|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_14,				ACTION_DUMMY,			1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												LOM_DATA_END,

					LOM_ACTLIST_15,				ACTION_WEAPON2_IDLE,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_16,				ACTION_WEAPON2_ON,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_WEAPON2_IDLE,	1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_17,				ACTION_WEAPON2_OFF,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_DATA_END,

					/*パッドリスト名*/			/*アクション名*/	/*オブジェクト名*/	/*入力*/			/*実行番号*/		/*実行引数*/	/*フラグ*/
LOM_PADLIST_DATA,	LOM_PADLIST_00,				ACTION_DUMMY,		LOM_PAD_ALLOBJ,		LOM_PAD_ANYINPUT,	LOM_EXE_CALLBACK,	0,				0,
												LOM_DATA_END,
					LOM_DATA_END,

					/*モード名*/	/*ＬＯＭアクションリスト*/	/*ＬＯＭオブジェクトリスト*/	/*ＬＯＭパッドリスト*/
LOM_MODE_DATA,		LOM_MODE_00,	LOM_ACTLIST_00,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,
					LOM_MODE_01,	LOM_ACTLIST_01,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,
					LOM_MODE_02,	LOM_ACTLIST_02,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,
					LOM_MODE_03,	LOM_ACTLIST_03,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,
					LOM_MODE_04,	LOM_ACTLIST_04,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,
					LOM_MODE_05,	LOM_ACTLIST_05,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,
					LOM_MODE_06,	LOM_ACTLIST_06,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,
					LOM_MODE_07,	LOM_ACTLIST_07,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,
					LOM_MODE_08,	LOM_ACTLIST_08,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,
					LOM_MODE_09,	LOM_ACTLIST_09,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,
					LOM_MODE_10,	LOM_ACTLIST_10,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,
					LOM_MODE_11,	LOM_ACTLIST_11,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,
					LOM_MODE_12,	LOM_ACTLIST_12,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,
					LOM_MODE_13,	LOM_ACTLIST_13,				LOM_MODE_NOLIST,				LOM_PADLIST_00,
					LOM_MODE_14,	LOM_ACTLIST_14,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,
					LOM_MODE_15,	LOM_ACTLIST_15,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,
					LOM_MODE_16,	LOM_ACTLIST_16,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,
					LOM_MODE_17,	LOM_ACTLIST_17,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,
					LOM_DATA_END,
LOM_DATA_END,
};

/*******************************************************************************
 * static
 */
/*******************************************************************************
 * ターゲットカウンターの分子の表示設定
 */
static void SetTarget1(
	WORK *work,		/* ワーク */
	int  value)		/* 値 */
{
	int i;

	if(value < 0)
		return;

	if(work->flag & VRSCR_FIVE)
	{
		/* 値 １０の位 */
		if(value > 99)
		{
			/* カンスト */
			LOM_SprTex_SetU(&work->layoutman, OBJECT_TARGET_5, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			LOM_SprTex_SetU(&work->layoutman, OBJECT_TARGET_4, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			return;
		}
		i = value / 10;
		value %= 10;
		LOM_SprTex_SetU(&work->layoutman, OBJECT_TARGET_5, work->dig_tex_u0 + VRSCR_TEX_WIDTH * i);

		/* 値 １の位 */
		LOM_SprTex_SetU(&work->layoutman, OBJECT_TARGET_4, work->dig_tex_u0 + VRSCR_TEX_WIDTH * value);
	}
	else
	{
		/* １００の位 */
		if(value > 999)
		{
			/* カンスト */
			LOM_SprTex_SetU(&work->layoutman, OBJECT_TARGET_6, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			LOM_SprTex_SetU(&work->layoutman, OBJECT_TARGET_5, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			LOM_SprTex_SetU(&work->layoutman, OBJECT_TARGET_4, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			return;
		}
		i = value / 100;
		value %= 100;
		LOM_SprTex_SetU(&work->layoutman, OBJECT_TARGET_6, work->dig_tex_u0 + VRSCR_TEX_WIDTH * i);

		/* 値 １０の位 */
		i = value / 10;
		value %= 10;
		LOM_SprTex_SetU(&work->layoutman, OBJECT_TARGET_5, work->dig_tex_u0 + VRSCR_TEX_WIDTH * i);

		/* 値 １の位 */
		LOM_SprTex_SetU(&work->layoutman, OBJECT_TARGET_4, work->dig_tex_u0 + VRSCR_TEX_WIDTH * value);
	}
}

/*******************************************************************************
 * ターゲットカウンターの分母の表示設定
 */
static void SetTarget2(
	WORK *work,		/* ワーク */
	int  value)		/* 値 */
{
	int i;

	if(value < 0)
		return;

	if(work->flag & VRSCR_FIVE)
	{
		/* １０の位 */
		if(value > 99)
		{
			/* カンスト */
			LOM_SprTex_SetU(&work->layoutman, OBJECT_TARGET_3, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			LOM_SprTex_SetU(&work->layoutman, OBJECT_TARGET_2, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			return;
		}
		i = value / 10;
		value %= 10;
		LOM_SprTex_SetU(&work->layoutman, OBJECT_TARGET_3, work->dig_tex_u0 + VRSCR_TEX_WIDTH * i);

		/* １の位 */
		LOM_SprTex_SetU(&work->layoutman, OBJECT_TARGET_2, work->dig_tex_u0 + VRSCR_TEX_WIDTH * value);
	}
	else
	{
		/* １００の位 */
		if(value > 999)
		{
			/* カンスト */
			LOM_SprTex_SetU(&work->layoutman, OBJECT_TARGET_3, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			LOM_SprTex_SetU(&work->layoutman, OBJECT_TARGET_2, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			LOM_SprTex_SetU(&work->layoutman, OBJECT_TARGET_1, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			return;
		}
		i = value / 100;
		value %= 100;
		LOM_SprTex_SetU(&work->layoutman, OBJECT_TARGET_3, work->dig_tex_u0 + VRSCR_TEX_WIDTH * i);

		/* １０の位 */
		i = value / 10;
		value %= 10;
		LOM_SprTex_SetU(&work->layoutman, OBJECT_TARGET_2, work->dig_tex_u0 + VRSCR_TEX_WIDTH * i);

		/* １の位 */
		LOM_SprTex_SetU(&work->layoutman, OBJECT_TARGET_1, work->dig_tex_u0 + VRSCR_TEX_WIDTH * value);
	}
}

/*******************************************************************************
 * スコアカウンターの表示設定
 */
static void SetScore(
	WORK *work,		/* ワーク */
	int  value)		/* 値 */
{
	int i;

	if(value < 0)
		return;

	if(work->flag & VRSCR_FIVE)
	{
		/* １００００の位 */
		if(value > 99999)
		{
			/* カンスト */
			LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_7, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_6, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_5, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_4, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_3, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			return;
		}
		i = value / 10000;
		value %= 10000;
		LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_7, work->dig_tex_u0 + VRSCR_TEX_WIDTH * i);

		/* １０００の位 */
		i = value / 1000;
		value %= 1000;
		LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_6, work->dig_tex_u0 + VRSCR_TEX_WIDTH * i);

		/* １００の位 */
		i = value / 100;
		value %= 100;
		LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_5, work->dig_tex_u0 + VRSCR_TEX_WIDTH * i);

		/* １０の位 */
		i = value / 10;
		value %= 10;
		LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_4, work->dig_tex_u0 + VRSCR_TEX_WIDTH * i);

		/* １の位 */
		LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_3, work->dig_tex_u0 + VRSCR_TEX_WIDTH * value);
	}
	else
	{
		/* １００００００の位 */
		if(value > 9999999)
		{
			/* カンスト */
			LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_7, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_6, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_5, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_4, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_3, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_2, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_1, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
			return;
		}
		i = value / 1000000;
		value %= 1000000;
		LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_7, work->dig_tex_u0 + VRSCR_TEX_WIDTH * i);

		/* １０００００の位 */
		i = value / 100000;
		value %= 100000;
		LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_6, work->dig_tex_u0 + VRSCR_TEX_WIDTH * i);

		/* １００００の位 */
		i = value / 10000;
		value %= 10000;
		LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_5, work->dig_tex_u0 + VRSCR_TEX_WIDTH * i);

		/* １０００の位 */
		i = value / 1000;
		value %= 1000;
		LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_4, work->dig_tex_u0 + VRSCR_TEX_WIDTH * i);

		/* １００の位 */
		i = value / 100;
		value %= 100;
		LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_3, work->dig_tex_u0 + VRSCR_TEX_WIDTH * i);

		/* １０の位 */
		i = value / 10;
		value %= 10;
		LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_2, work->dig_tex_u0 + VRSCR_TEX_WIDTH * i);

		/* １の位 */
		LOM_SprTex_SetU(&work->layoutman, OBJECT_SCORE_1, work->dig_tex_u0 + VRSCR_TEX_WIDTH * value);
	}
}

/*******************************************************************************
 * タイムカウンターの表示設定
 */
static void SetTime(
	WORK *work,		/* ワーク */
	int  value)		/* 値 */
{
	int i;
	int min;
	int sec;

	if(value < 0)
		return;

	min = value / (VRDEF_FRAMERATE * 60);
	value %= VRDEF_FRAMERATE * 60;
	sec = value / VRDEF_FRAMERATE;
	value %= VRDEF_FRAMERATE;

	/* 分 １０の位 */
	i = min / 10;
	min %= 10;

	if(i > 9)
	{
		/* カンスト */
		LOM_SprTex_SetU(&work->layoutman, OBJECT_TIME_6, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
		LOM_SprTex_SetU(&work->layoutman, OBJECT_TIME_5, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
		LOM_SprTex_SetU(&work->layoutman, OBJECT_TIME_4, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 5);
		LOM_SprTex_SetU(&work->layoutman, OBJECT_TIME_3, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
		LOM_SprTex_SetU(&work->layoutman, OBJECT_TIME_2, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
		LOM_SprTex_SetU(&work->layoutman, OBJECT_TIME_1, work->dig_tex_u0 + VRSCR_TEX_WIDTH * 9);
		return;
	}
	LOM_SprTex_SetU(&work->layoutman, OBJECT_TIME_6, work->dig_tex_u0 + VRSCR_TEX_WIDTH * i);

	/* 分 １の位 */
	LOM_SprTex_SetU(&work->layoutman, OBJECT_TIME_5, work->dig_tex_u0 + VRSCR_TEX_WIDTH * min);

	/* 秒 １０の位 */
	i = sec / 10;
	sec %= 10;
	LOM_SprTex_SetU(&work->layoutman, OBJECT_TIME_4, work->dig_tex_u0 + VRSCR_TEX_WIDTH * i);

	/* 秒 １の位 */
	LOM_SprTex_SetU(&work->layoutman, OBJECT_TIME_3, work->dig_tex_u0 + VRSCR_TEX_WIDTH * sec);

	/* １／１００秒 １０の位 */
	value = (value * 100) / VRDEF_FRAMERATE;
	i = value / 10;
	value %= 10;
	LOM_SprTex_SetU(&work->layoutman, OBJECT_TIME_2, work->dig_tex_u0 + VRSCR_TEX_WIDTH * i);

	/* １／１００秒 １の位 */
	LOM_SprTex_SetU(&work->layoutman, OBJECT_TIME_1, work->dig_tex_u0 + VRSCR_TEX_WIDTH * value);
}

/*******************************************************************************
 * 
 */
static void Init3(
	WORK *work)		/* ワーク */
{
	/* モードの設定 */
	if     (work->flag & VRSCR_HIDE_COUNTER) { LOM_SetCurMode(&work->layoutman, LOM_MODE_12, 1, 1, 1); SY_PRINTF1("VRSCR_HIDE_COUNTER\n"); }
	else if(work->flag & VRSCR_WEAPON      ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_00, 1, 1, 1); SY_PRINTF1("VRSCR_WEAPON\n");       }
	else if(work->flag & VRSCR_SNEAKING    ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_03, 1, 1, 1); SY_PRINTF1("VRSCR_SNEAKING\n");     }
	else if(work->flag & VRSCR_BOMB        ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_06, 1, 1, 1); SY_PRINTF1("VRSCR_BOMB\n");         }
	else if(work->flag & VRSCR_ELIMINATE   ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_09, 1, 1, 1); SY_PRINTF1("VRSCR_ELIMINATE\n");    }
	else if(work->flag & VRSCR_WEAPON2     ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_15, 1, 1, 1); SY_PRINTF1("VRSCR_WEAPON2\n");      }
	else
	{
		SY_PRINTF2("No VR Screen Mode.\n");
		ASSERT(0);
	}
	SY_PRINTF1("VRSCR_PHASE_IDLE\n");
	work->phase  = VRSCR_PHASE_IDLE;
	work->count  = 0;

	work->flag &= ~VRSCR_OPENFRAME;
	work->flag &= ~VRSCR_INIT2;
	work->flag |=  VRSCR_INIT3;
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

	Init3(work);

	return 1;
}

/*******************************************************************************
 * メッセージ処理

mesg ＶＲスクリーン $s:名前 非表示[0]
mesg ＶＲスクリーン $s:名前 表示[1]
 */
static int ReceiveMessage(	/* 処理結果 */
	WORK *work)		/* ワーク */
{
	int    i;
	int    msg_count;
	int    ret = -1;
	GV_MSG *msg;

	msg_count = GV_ReceiveMessage(work->name, &msg);
	for(i = 0; i < msg_count; i++, msg++)
	{
		SY_PRINTF2("Message received.\n");
		switch(msg->message[0])
		{
		case 0:		/* 非表示 */
			work->flag |=  VRSCR_INIT2;
			LOM_SetCurMode(&work->layoutman, LOM_MODE_14, 1, 1, 1);
			SY_PRINTF1("VRSCR_PHASE_HIDE\n");
			work->phase  = VRSCR_PHASE_HIDE;
			work->count  = 0;
			work->count2 = 0;
			break;

		case 1:		/* 表示 */
			if(work->phase == VRSCR_PHASE_HIDE)
			{
				if(work->flag & VRSCR_HIDE_FRAME)
				{
					Init3(work);
				}
				else
				{
					SY_PRINTF1("LOM_MODE_13\n");
					LOM_SetCurMode(&work->layoutman, LOM_MODE_13, 1, 1, 1);
					SY_PRINTF1("VRSCR_PHASE_IDLE\n");
					work->phase  = VRSCR_PHASE_IDLE;
					work->count  = 0;

					work->flag |= VRSCR_OPENFRAME;
				}
			}
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
	if(work->flag & VRSCR_INIT)
	{
		/*
		 * レイアウトの初期化
		 */
		work->flag &= ~VRSCR_INIT;
		work->flag |=  VRSCR_INIT2;

		/* 数字テクスチャのＵＶ関連の値の取得 */
		LOM_SprTex_GetU(&work->layoutman, OBJECT_TIME_6, &work->dig_tex_u0);
		SY_PRINTF1("u0: %f\n", work->dig_tex_u0);

		/* フレーム原点の取得 */
		LOM_Emp_GetPosition(&work->layoutman, OBJECT_WIREFRAME, &work->frm_org, 1);
		work->frm_pos = work->frm_org;

		/* モードの設定 */
		if     (work->flag & VRSCR_HIDE_COUNTER) { LOM_SetCurMode(&work->layoutman, LOM_MODE_12, 1, 1, 1); SY_PRINTF1("VRSCR_HIDE_COUNTER\n"); }
		else if(work->flag & VRSCR_WEAPON      ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_00, 1, 1, 1); SY_PRINTF1("VRSCR_WEAPON\n");       }
		else if(work->flag & VRSCR_SNEAKING    ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_03, 1, 1, 1); SY_PRINTF1("VRSCR_SNEAKING\n");     }
		else if(work->flag & VRSCR_BOMB        ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_06, 1, 1, 1); SY_PRINTF1("VRSCR_BOMB\n");         }
		else if(work->flag & VRSCR_ELIMINATE   ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_09, 1, 1, 1); SY_PRINTF1("VRSCR_ELIMINATE\n");    }
		else if(work->flag & VRSCR_WEAPON2     ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_15, 1, 1, 1); SY_PRINTF1("VRSCR_WEAPON2\n");      }
		else
		{
			SY_PRINTF2("No VR Screen Mode.\n");
			ASSERT(0);
		}

		/* 起動時は非表示 */
		SY_PRINTF1("VRSCR_PHASE_HIDE\n");
		work->phase  = VRSCR_PHASE_HIDE;
		work->count  = 0;
		work->count2 = 0;
	}

	/* メッセージ処理 */
	ReceiveMessage(work);

	/* レイアウトマネージャ */
	ActLayoutman(&work->layoutman);

	/* ＵＶ操作 */
	/* ターゲット（ボム、エネミー） */
	if    ((work->flag & VRSCR_WEAPON   )
		|| (work->flag & VRSCR_WEAPON2  )) { SetTarget1(work, VR_TARGET_NUM); SetTarget2(work, VR_TARGET_MAX); }
	else if(work->flag & VRSCR_BOMB     )  { SetTarget1(work, VR_BOMBS_NUM ); SetTarget2(work, VR_BOMBS_MAX ); }
	else if(work->flag & VRSCR_ELIMINATE)  { SetTarget1(work, VR_ENEMY_NUM ); SetTarget2(work, VR_ENEMY_MAX ); }

	/* スコア */
	if(work->flag & VRSCR_WEAPON) { SetScore(work, VR_SCORE); }

	/* タイム */
	SetTime(work, VR_TIME);

	/* 残り１０秒 */
	if(VR_TIME <= VRDEF_FRAMERATE * 10)
	{
		SPR_COLOR col;
		col.r = 240;
		col.g =  60;
		col.b =  60;
		col.a = 48 + ((80 * (VR_TIME % VRDEF_FRAMERATE)) / VRDEF_FRAMERATE);
		LOM_Spr_SetColor(&work->layoutman, OBJECT_TIME_6, &col);
		LOM_Spr_SetColor(&work->layoutman, OBJECT_TIME_5, &col);
		LOM_Spr_SetColor(&work->layoutman, OBJECT_TIME_4, &col);
		LOM_Spr_SetColor(&work->layoutman, OBJECT_TIME_3, &col);
		LOM_Spr_SetColor(&work->layoutman, OBJECT_TIME_2, &col);
		LOM_Spr_SetColor(&work->layoutman, OBJECT_TIME_1, &col);
	}

	/* 枠の表示の設定 */

	if(work->flag & VRSCR_HIDE_FRAME)
	{
		int flags;

		LOM_Lin_GetFlags(&work->layoutman, OBJECT_LN_LINE1, &flags); flags |= SPR_FLAG_HIDDEN;
		LOM_Lin_SetFlags(&work->layoutman, OBJECT_LN_LINE1, flags);
		LOM_Lin_GetFlags(&work->layoutman, OBJECT_LN_LINE2, &flags); flags |= SPR_FLAG_HIDDEN;
		LOM_Lin_SetFlags(&work->layoutman, OBJECT_LN_LINE2, flags);
		LOM_Lin_GetFlags(&work->layoutman, OBJECT_LN_LINE3, &flags); flags |= SPR_FLAG_HIDDEN;
		LOM_Lin_SetFlags(&work->layoutman, OBJECT_LN_LINE3, flags);
		LOM_Lin_GetFlags(&work->layoutman, OBJECT_LN_LINE4, &flags); flags |= SPR_FLAG_HIDDEN;
		LOM_Lin_SetFlags(&work->layoutman, OBJECT_LN_LINE4, flags);
		LOM_Spr_GetFlags(&work->layoutman, OBJECT_FRAME01, &flags); flags |= SPR_FLAG_HIDDEN;
		LOM_Spr_SetFlags(&work->layoutman, OBJECT_FRAME01, flags);
		LOM_Spr_GetFlags(&work->layoutman, OBJECT_FRAME02, &flags); flags |= SPR_FLAG_HIDDEN;
		LOM_Spr_SetFlags(&work->layoutman, OBJECT_FRAME02, flags);
		LOM_Spr_GetFlags(&work->layoutman, OBJECT_VRSYSTEM, &flags); flags |= SPR_FLAG_HIDDEN;
		LOM_Spr_SetFlags(&work->layoutman, OBJECT_VRSYSTEM, flags);
		LOM_Emp_GetFlags(&work->layoutman, OBJECT_WIREFRAME, &flags); flags |= SPR_FLAG_HIDDEN;
		LOM_Emp_SetFlags(&work->layoutman, OBJECT_WIREFRAME, flags);
	}

	if(    !(work->flag & VRSCR_INIT2)
		&& !(work->flag & VRSCR_OPENFRAME))
	{
		/* ゲーム状態監視 */
		if(!(work->flag & VRSCR_INIT3))
		{
			/* メニューチェック */
			if(GM_CheckMenuStatus(MENU_MENU_OPEN))
			{
				if((work->flag & VRSCR_MENU_ON) == 0)
				{
					/* メニューがＯＮになった瞬間 */
					work->flag |= VRSCR_MENU_ON;

					/* モードの設定 */
					if     (work->flag & VRSCR_HIDE_COUNTER) { }
					else if(work->flag & VRSCR_WEAPON      ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_02, 1, 1, 1); SY_PRINTF1("LOM_MODE_02\n"); }
					else if(work->flag & VRSCR_SNEAKING    ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_05, 1, 1, 1); SY_PRINTF1("LOM_MODE_05\n"); }
					else if(work->flag & VRSCR_BOMB        ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_08, 1, 1, 1); SY_PRINTF1("LOM_MODE_08\n"); }
					else if(work->flag & VRSCR_ELIMINATE   ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_11, 1, 1, 1); SY_PRINTF1("LOM_MODE_11\n"); }
					else if(work->flag & VRSCR_WEAPON2     ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_17, 1, 1, 1); SY_PRINTF1("LOM_MODE_17\n"); }
				}
			}
			else
			{
				if(work->flag & VRSCR_MENU_ON)
				{
					/* メニューがＯＦＦになった瞬間 */
					work->flag &= ~VRSCR_MENU_ON;

					/* モードの設定 */
					if     (work->flag & VRSCR_HIDE_COUNTER) { }
					else if(work->flag & VRSCR_WEAPON      ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_01, 1, 1, 1); SY_PRINTF1("LOM_MODE_01\n"); }
					else if(work->flag & VRSCR_SNEAKING    ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_04, 1, 1, 1); SY_PRINTF1("LOM_MODE_04\n"); }
					else if(work->flag & VRSCR_BOMB        ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_07, 1, 1, 1); SY_PRINTF1("LOM_MODE_07\n"); }
					else if(work->flag & VRSCR_ELIMINATE   ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_10, 1, 1, 1); SY_PRINTF1("LOM_MODE_10\n"); }
					else if(work->flag & VRSCR_WEAPON2     ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_16, 1, 1, 1); SY_PRINTF1("LOM_MODE_16\n"); }
				}
			}
		}

		/* ポーズ時は非動作 */
		if((GV_PauseLevel & GV_LEVEL_NORMAL) == 0)
		{
			/* 枠の移動 */
			if(GV_PadData[0].status & PAD_L)
			{
				work->frm_pos.x += ((work->frm_org.x - VRSCR_FRM_X_RANGE) - work->frm_pos.x) / VRSCR_FRM_MOVE_DIV;
			}
			else if(GV_PadData[0].status & PAD_R)
			{
				work->frm_pos.x += ((work->frm_org.x + VRSCR_FRM_X_RANGE) - work->frm_pos.x) / VRSCR_FRM_MOVE_DIV;
			}
			else
			{
				work->frm_pos.x += (work->frm_org.x - work->frm_pos.x) / VRSCR_FRM_MOVE_DIV;
			}
			if(GV_PadData[0].status & PAD_U)
			{
				work->frm_pos.y += ((work->frm_org.y - VRSCR_FRM_Y_RANGE) - work->frm_pos.y) / VRSCR_FRM_MOVE_DIV;
			}
			else if(GV_PadData[0].status & PAD_D)
			{
				work->frm_pos.y += ((work->frm_org.y + VRSCR_FRM_Y_RANGE) - work->frm_pos.y) / VRSCR_FRM_MOVE_DIV;
			}
			else
			{
				work->frm_pos.y += (work->frm_org.y - work->frm_pos.y) / VRSCR_FRM_MOVE_DIV;
			}
			LOM_Emp_SetPosition(&work->layoutman, OBJECT_WIREFRAME, &work->frm_pos, 1);
		}

		/* ポーズチェック */
		if(GV_PauseLevel & GV_LEVEL_NORMAL)
		{
			if(work->count2 > 0)
			{
				/* 武器・アイテムメニュー表示ではないポーズ中にフェードアウトへ移行 */
				if(GM_CheckMenuStatus(MENU_MENU_OPEN) == 0)
				{
					if(work->phase == VRSCR_PHASE_IDLE)
					{
						SY_PRINTF1("VRSCR_PHASE_FADE_OUT\n");
						work->phase = VRSCR_PHASE_FADE_OUT;
						work->count = 0;
					}
				}
			}

			/* ポーズ時間 */
			work->count2 = 1;
		}
		else
		{
			/* ポーズ時間をリセット */
			work->count2 = 0;

			/* ゲームオーバーフラグ・クリアフラグ チェック */
			if(GM_IsGameOver() || (GM_VRStatus & GM_VR_CLEAR))
			{
				if(work->phase == VRSCR_PHASE_IDLE)
				{
					SY_PRINTF1("VRSCR_PHASE_FADE_OUT\n");
					work->phase = VRSCR_PHASE_FADE_OUT;
					work->count = 0;
				}
			}
			else
			{
				if(work->phase == VRSCR_PHASE_HIDE)
				{
					SY_PRINTF1("VRSCR_PHASE_FADE_IN\n");
					work->phase = VRSCR_PHASE_FADE_IN;
					work->count = 0;
				}
			}
		}

		if(work->flag & VRSCR_INIT3)
		{
			work->flag &= ~VRSCR_INIT3;
		}
	}

	/* 各フェーズ処理 */
	{
		unsigned char a1 = 128;
		unsigned char a2 =  42;
		int           flags;
		SPR_COLOR     col;

		work->count++;
		switch(work->phase)
		{
		case VRSCR_PHASE_IDLE:			/* アイドル */
			break;

		case VRSCR_PHASE_HIDE:			/* 非表示アイドル */
			LOM_Emp_GetFlags(&work->layoutman, 2770484 /* ROOT */, &flags);
			flags |= SPR_FLAG_HIDDEN;
			LOM_Emp_SetFlags(&work->layoutman, 2770484 /* ROOT */, flags);
			break;

		case VRSCR_PHASE_FADE_IN:		/* フェードイン */
			/* 表示 */
			LOM_Emp_GetFlags(&work->layoutman, 2770484 /* ROOT */, &flags);
			flags &= ~SPR_FLAG_HIDDEN;
			LOM_Emp_SetFlags(&work->layoutman, 2770484 /* ROOT */, flags);

			a1 = (128 * work->count) / VRSCR_FADE_IN_COUNT;
			a2 = ( 42 * work->count) / VRSCR_FADE_IN_COUNT;
			if(work->count == VRSCR_FADE_IN_COUNT)
			{
				SY_PRINTF1("VRSCR_PHASE_IDLE\n");
				work->phase = VRSCR_PHASE_IDLE;
				work->count = 0;
			}
			break;

		case VRSCR_PHASE_FADE_OUT:		/* フェードアウト */

			a1 = (128 * (VRSCR_FADE_OUT_COUNT - work->count)) / VRSCR_FADE_OUT_COUNT;
			a2 = ( 42 * (VRSCR_FADE_OUT_COUNT - work->count)) / VRSCR_FADE_OUT_COUNT;
			if(work->count == VRSCR_FADE_OUT_COUNT)
			{
				SY_PRINTF1("VRSCR_PHASE_HIDE\n");
				work->phase = VRSCR_PHASE_HIDE;
				work->count = 0;
			}
			break;
		}

		LOM_Emp_GetFlags(&work->layoutman, 2770484 /* ROOT */, &flags);
		if(    ((flags & SPR_FLAG_HIDDEN) == 0)
			&& (work->phase != VRSCR_PHASE_IDLE))
		{
			if((work->flag & VRSCR_WEAPON) || (work->flag & VRSCR_WEAPON2))
			{
				LOM_Spr_GetColor (&work->layoutman, OBJECT_TARGETS  , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_TARGETS  , &col);
			}
			else if(work->flag & VRSCR_BOMB)
			{
				LOM_Spr_GetColor (&work->layoutman, OBJECT_BOMBS    , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_BOMBS    , &col);
			}
			else if(work->flag & VRSCR_ELIMINATE)
			{
				LOM_Spr_GetColor (&work->layoutman, OBJECT_ENEMIES  , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_ENEMIES  , &col);
			}
			if(!(work->flag & VRSCR_FIVE))
			{
				LOM_Spr_GetColor (&work->layoutman, OBJECT_TARGET_6 , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_TARGET_6 , &col);
			}
			LOM_Spr_GetColor (&work->layoutman, OBJECT_TARGET_5 , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_TARGET_5 , &col);
			LOM_Spr_GetColor (&work->layoutman, OBJECT_TARGET_4 , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_TARGET_4 , &col);
			LOM_Spr_GetColor (&work->layoutman, OBJECT_SLASH    , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_SLASH    , &col);
			LOM_Spr_GetColor (&work->layoutman, OBJECT_TARGET_3 , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_TARGET_3 , &col);
			LOM_Spr_GetColor (&work->layoutman, OBJECT_TARGET_2 , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_TARGET_2 , &col);
			if(!(work->flag & VRSCR_FIVE))
			{
				LOM_Spr_GetColor (&work->layoutman, OBJECT_TARGET_1 , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_TARGET_1 , &col);
			}
			LOM_Spr_GetColor (&work->layoutman, OBJECT_TIME     , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_TIME     , &col);
			LOM_Spr_GetColor (&work->layoutman, OBJECT_TIME_6   , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_TIME_6   , &col);
			LOM_Spr_GetColor (&work->layoutman, OBJECT_TIME_5   , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_TIME_5   , &col);
			LOM_Spr_GetColor (&work->layoutman, OBJECT_COLON2   , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_COLON2   , &col);
			LOM_Spr_GetColor (&work->layoutman, OBJECT_TIME_4   , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_TIME_4   , &col);
			LOM_Spr_GetColor (&work->layoutman, OBJECT_TIME_3   , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_TIME_3   , &col);
			LOM_Spr_GetColor (&work->layoutman, OBJECT_COLON1   , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_COLON1   , &col);
			LOM_Spr_GetColor (&work->layoutman, OBJECT_TIME_2   , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_TIME_2   , &col);
			LOM_Spr_GetColor (&work->layoutman, OBJECT_TIME_1   , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_TIME_1   , &col);
			LOM_Spr_GetColor (&work->layoutman, OBJECT_SCORE    , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_SCORE    , &col);
			LOM_Spr_GetColor (&work->layoutman, OBJECT_SCORE_7  , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_SCORE_7  , &col);
			LOM_Spr_GetColor (&work->layoutman, OBJECT_SCORE_6  , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_SCORE_6  , &col);
			LOM_Spr_GetColor (&work->layoutman, OBJECT_SCORE_5  , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_SCORE_5  , &col);
			LOM_Spr_GetColor (&work->layoutman, OBJECT_SCORE_4  , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_SCORE_4  , &col);
			LOM_Spr_GetColor (&work->layoutman, OBJECT_SCORE_3  , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_SCORE_3  , &col);
			if(!(work->flag & VRSCR_FIVE))
			{
				LOM_Spr_GetColor (&work->layoutman, OBJECT_SCORE_2  , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_SCORE_2  , &col);
				LOM_Spr_GetColor (&work->layoutman, OBJECT_SCORE_1  , &col); col.a = a1; LOM_Spr_SetColor (&work->layoutman, OBJECT_SCORE_1  , &col);
			}

			LOM_Spr_GetColor (&work->layoutman, OBJECT_VRSYSTEM , &col); col.a = a2; LOM_Spr_SetColor (&work->layoutman, OBJECT_VRSYSTEM , &col);
			LOM_Spr_GetColor (&work->layoutman, OBJECT_FRAME02  , &col); col.a = a2; LOM_Spr_SetColor (&work->layoutman, OBJECT_FRAME02  , &col);
			LOM_Spr_GetColor (&work->layoutman, OBJECT_FRAME01  , &col); col.a = a2; LOM_Spr_SetColor (&work->layoutman, OBJECT_FRAME01  , &col);

			LOM_Lin_GetColor0(&work->layoutman, OBJECT_LN_FRAME1, &col); col.a = a2; LOM_Lin_SetColor0(&work->layoutman, OBJECT_LN_FRAME1, &col);
			LOM_Lin_GetColor1(&work->layoutman, OBJECT_LN_FRAME1, &col); col.a = a2; LOM_Lin_SetColor1(&work->layoutman, OBJECT_LN_FRAME1, &col);
			LOM_Lin_GetColor0(&work->layoutman, OBJECT_LN_FRAME2, &col); col.a = a2; LOM_Lin_SetColor0(&work->layoutman, OBJECT_LN_FRAME2, &col);
			LOM_Lin_GetColor1(&work->layoutman, OBJECT_LN_FRAME2, &col); col.a = a2; LOM_Lin_SetColor1(&work->layoutman, OBJECT_LN_FRAME2, &col);
			LOM_Lin_GetColor0(&work->layoutman, OBJECT_LN_FRAME3, &col); col.a = a2; LOM_Lin_SetColor0(&work->layoutman, OBJECT_LN_FRAME3, &col);
			LOM_Lin_GetColor1(&work->layoutman, OBJECT_LN_FRAME3, &col); col.a = a2; LOM_Lin_SetColor1(&work->layoutman, OBJECT_LN_FRAME3, &col);
			LOM_Lin_GetColor0(&work->layoutman, OBJECT_LN_FRAME4, &col); col.a = a2; LOM_Lin_SetColor0(&work->layoutman, OBJECT_LN_FRAME4, &col);
			LOM_Lin_GetColor1(&work->layoutman, OBJECT_LN_FRAME4, &col); col.a = a2; LOM_Lin_SetColor1(&work->layoutman, OBJECT_LN_FRAME4, &col);
			LOM_Lin_GetColor0(&work->layoutman, OBJECT_LN_FRAME5, &col); col.a = a2; LOM_Lin_SetColor0(&work->layoutman, OBJECT_LN_FRAME5, &col);
			LOM_Lin_GetColor1(&work->layoutman, OBJECT_LN_FRAME5, &col); col.a = a2; LOM_Lin_SetColor1(&work->layoutman, OBJECT_LN_FRAME5, &col);
			LOM_Lin_GetColor0(&work->layoutman, OBJECT_LN_FRAME6, &col); col.a = a2; LOM_Lin_SetColor0(&work->layoutman, OBJECT_LN_FRAME6, &col);
			LOM_Lin_GetColor1(&work->layoutman, OBJECT_LN_FRAME6, &col); col.a = a2; LOM_Lin_SetColor1(&work->layoutman, OBJECT_LN_FRAME6, &col);
			LOM_Lin_GetColor0(&work->layoutman, OBJECT_LN_FRAME7, &col); col.a = a2; LOM_Lin_SetColor0(&work->layoutman, OBJECT_LN_FRAME7, &col);
			LOM_Lin_GetColor1(&work->layoutman, OBJECT_LN_FRAME7, &col); col.a = a2; LOM_Lin_SetColor1(&work->layoutman, OBJECT_LN_FRAME7, &col);
			LOM_Lin_GetColor0(&work->layoutman, OBJECT_LN_FRAME8, &col); col.a = a2; LOM_Lin_SetColor0(&work->layoutman, OBJECT_LN_FRAME8, &col);
			LOM_Lin_GetColor1(&work->layoutman, OBJECT_LN_FRAME8, &col); col.a = a2; LOM_Lin_SetColor1(&work->layoutman, OBJECT_LN_FRAME8, &col);
			LOM_Lin_GetColor0(&work->layoutman, OBJECT_LN_LINE3 , &col); col.a = a2; LOM_Lin_SetColor0(&work->layoutman, OBJECT_LN_LINE3 , &col);
			LOM_Lin_GetColor1(&work->layoutman, OBJECT_LN_LINE3 , &col); col.a = a2; LOM_Lin_SetColor1(&work->layoutman, OBJECT_LN_LINE3 , &col);
			LOM_Lin_GetColor0(&work->layoutman, OBJECT_LN_LINE4 , &col); col.a = a2; LOM_Lin_SetColor0(&work->layoutman, OBJECT_LN_LINE4 , &col);
			LOM_Lin_GetColor1(&work->layoutman, OBJECT_LN_LINE4 , &col); col.a = a2; LOM_Lin_SetColor1(&work->layoutman, OBJECT_LN_LINE4 , &col);
			LOM_Lin_GetColor0(&work->layoutman, OBJECT_LN_LINE1 , &col); col.a = a2; LOM_Lin_SetColor0(&work->layoutman, OBJECT_LN_LINE1 , &col);
			LOM_Lin_GetColor1(&work->layoutman, OBJECT_LN_LINE1 , &col); col.a = a2; LOM_Lin_SetColor1(&work->layoutman, OBJECT_LN_LINE1 , &col);
			LOM_Lin_GetColor0(&work->layoutman, OBJECT_LN_LINE2 , &col); col.a = a2; LOM_Lin_SetColor0(&work->layoutman, OBJECT_LN_LINE2 , &col);
			LOM_Lin_GetColor1(&work->layoutman, OBJECT_LN_LINE2 , &col); col.a = a2; LOM_Lin_SetColor1(&work->layoutman, OBJECT_LN_LINE2 , &col);
		}
	}

	if(work->count > 1000)
		work->count = 1000;
}

/******************************************************************************
 * 終了処理
 */
static void Die(
	WORK *work)		/* ワーク */
{
	SY_PRINTF3("Die\n");

	/* レイアウトマネージャ */
	DestroyLayoutman(&work->layoutman);
}

/******************************************************************************
 * 初期化処理

chara ＶＲスクリーン[NewVRScreen_Scn] $s:名前 \
	-flag $i:フラグ

#define VRSCR_HIDE_FRAME   (0x0100)		// 枠を非表示
#define VRSCR_HIDE_COUNTER (0x0200)		// カウンターを非表示
#define VRSCR_FIVE         (0x0400)		// ５桁モード
#define VRSCR_WEAPON2      (0x0800)		// ＷＥＡＰＯＮ２　　　ＭＯＤＥ（ターゲット、　　　　タイム）
#define VRSCR_WEAPON       (0x1000)		// ＷＥＡＰＯＮ　　　　ＭＯＤＥ（ターゲット、スコア、タイム）
#define VRSCR_SNEAKING     (0x2000)		// ＳＮＥＡＫＩＮＧ　　ＭＯＤＥ（　　　　　　　　　　タイム）
#define VRSCR_BOMB         (0x4000)		// ＢＯＭＢ　　　　　　ＭＯＤＥ（ボム、　　　　　　　タイム）
#define VRSCR_ELIMINATE    (0x8000)		// ＥＬＩＭＩＮＡＴＥ　ＭＯＤＥ（エネミー、　　　　　タイム）
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	WORK *work,		/* ワーク */
	int  name,		/* シナリオ名 */
	int  where)		/* マップＩＤ */
{
	SY_PRINTF3("GetResources\n");

	work->name = name;
	work->flag = VRSCR_HIDE_COUNTER;

	/* シナリオオプション */
	if(name != 0)
	{
		/* flag */
		if(GCL_GetOption('f') == NULL)
		{
			SY_PRINTF2("no flag parameter.\n");
			return 0;
		}
		work->flag = GCL_GetNextInt();

		SY_PRINTF1("Get Scn Option Succeeded.\n");
	}

	/* レイアウトマネージャ */
	{
		if(work->flag & VRSCR_FIVE)
		{
			if(CreateLayoutman4(&work->layoutman, L2D_FILENAME_2, 4, 0, 0, 0, 0, 0, LOMCallback, NULL, work, 0, 0, LOM_INPUT_DIRECT) == 0)
			{
				SY_PRINTF2("Create Layoutman Failed.\n");
				return 0;
			}
			SY_PRINTF1("Create Layoutman Succeeded.\n");
		}
		else
		{
			SY_PRINTF2("Need to set VRSCR_FIVE flag.\n");
			ASSERT(0);
			return 0;
#if 0
			if(CreateLayoutman4(&work->layoutman, L2D_FILENAME, 4, 0, 0, 0, 0, 0, LOMCallback, NULL, work, 0, 0, LOM_INPUT_DIRECT) == 0)
			{
				SY_PRINTF2("Create Layoutman Failed.\n");
				return 0;
			}
			SY_PRINTF1("Create Layoutman Succeeded.\n");
#endif
		}

		if(LoadLOMData(&work->layoutman, _lom_data) == 0)
		{
			SY_PRINTF2("Load LOM Data Failed.\n");
			return 0;
		}
		SY_PRINTF1("Load LOM Data Succeeded.\n");
	}

	work->flag |= VRSCR_INIT;

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
void *NewVRScreen(void)
{
	WORK *work;

	SY_PRINTF3("NewVRScreen\n");
	OPERATOR();

	/* アクター生成 */
	work = (WORK *)GV_CreateActor(
		GV_ACTOR_AFTER2,			/* プロセスの優先順位 */
		GV_CLASS_CHARA,				/* プロセスクラス */
		sizeof(WORK),				/* メモリ確保サイズ */
		0);							/* 実行プライオリティ */
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

void *NewVRScreen_Scn(
	int name,	/* シナリオ名 */
	int where)	/* マップＩＤ */
{
	WORK *work;

	SY_PRINTF3("NewVRScreen_Scn\n");
	OPERATOR();

	/* アクター生成 */
	work = (WORK *)GV_CreateActor(
		GV_ACTOR_AFTER2,			/* プロセスの優先順位 */
		GV_CLASS_CHARA,				/* プロセスクラス */
		sizeof(WORK),				/* メモリ確保サイズ */
		0);							/* 実行プライオリティ */
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
