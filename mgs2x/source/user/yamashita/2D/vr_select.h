/*******************************************************************************
 * vr_select - vr_select.h
 * ＶＲセレクト
 * 2002/06/03 S.Yamashita
 * $Id: vr_select.h,v 1.2 2002/12/12 14:25:40 takaki Exp $
 */

#ifndef __INC_VR_SELECT__
#define __INC_VR_SELECT__

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

#define __CHARA_NAME__ "VR Select"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
//#define SY_PRINTF3_DEBUG
#include "../sy_util/sy_util.h"

/*******************************************************************************
 * defines
 */

/* コールバックタイプ */
enum {
	/* 画面：プレイヤー選択 */
	VRSEL_CB_PLA_RAIDEN_UP = 1,					/* プレイヤー上で上下操作が行われた */
	VRSEL_CB_PLA_RAIDEN_DOWN,
	VRSEL_CB_PLA_NINJA_UP,
	VRSEL_CB_PLA_NINJA_DOWN,
	VRSEL_CB_PLA_X_RAIDEN_UP,
	VRSEL_CB_PLA_X_RAIDEN_DOWN,
	VRSEL_CB_PLA_SNAKE_UP,
	VRSEL_CB_PLA_SNAKE_DOWN,
	VRSEL_CB_PLA_PLISKIN_UP,
	VRSEL_CB_PLA_PLISKIN_DOWN,
	VRSEL_CB_PLA_TUXEDO_UP,
	VRSEL_CB_PLA_TUXEDO_DOWN,
	VRSEL_CB_PLA_PREVIOUS_UP,
	VRSEL_CB_PLA_PREVIOUS_DOWN,
	VRSEL_CB_PLA_SAVE_UP,
	VRSEL_CB_PLA_SAVE_DOWN,
	VRSEL_CB_PLA_EXIT_UP,
	VRSEL_CB_PLA_EXIT_DOWN,

	VRSEL_CB_PLA_RAIDEN_OK,						/* プレイヤー上で決定操作が行われた */
	VRSEL_CB_PLA_NINJA_OK,
	VRSEL_CB_PLA_X_RAIDEN_OK,
	VRSEL_CB_PLA_SNAKE_OK,
	VRSEL_CB_PLA_PLISKIN_OK,
	VRSEL_CB_PLA_TUXEDO_OK,
	VRSEL_CB_PLA_PREVIOUS_OK,
	VRSEL_CB_PLA_SAVE_OK,
	VRSEL_CB_PLA_EXIT_OK,

	VRSEL_CB_PLA_CANCEL,						/* プレイヤー上でキャンセル操作が行われた */

	VRSEL_CB_L1_OPENED,							/* レイアウト１が開いた */
	VRSEL_CB_L1_CLOSED,							/* レイアウト１が閉じた */

	/* 画面：ミッション選択 */
	VRSEL_CB_MIS_VR_UP,							/* ミッション上で上下操作が行われた */
	VRSEL_CB_MIS_VR_DOWN,
	VRSEL_CB_MIS_ALT_UP,
	VRSEL_CB_MIS_ALT_DOWN,

	VRSEL_CB_MIS_VR_OK,							/* ミッション上で決定操作が行われた */
	VRSEL_CB_MIS_ALT_OK,

	VRSEL_CB_MIS_CANCEL,						/* ミッション上でキャンセル操作が行われた */

	/* 画面：モード選択（ＶＲ） */
	VRSEL_CB_MOD_SNEAKING_UP,					/* モード上で上下操作が行われた */
	VRSEL_CB_MOD_SNEAKING_DOWN,
	VRSEL_CB_MOD_WEAPON_UP,
	VRSEL_CB_MOD_WEAPON_DOWN,
	VRSEL_CB_MOD_FIRST_PERSON_VIEW_UP,
	VRSEL_CB_MOD_FIRST_PERSON_VIEW_DOWN,
	VRSEL_CB_MOD_VARIETY_UP,
	VRSEL_CB_MOD_VARIETY_DOWN,
	VRSEL_CB_MOD_STREAKING_UP,
	VRSEL_CB_MOD_STREAKING_DOWN,

	VRSEL_CB_MOD_SNEAKING_OK,					/* モード上で決定操作が行われた */
	VRSEL_CB_MOD_WEAPON_OK,
	VRSEL_CB_MOD_FIRST_PERSON_VIEW_OK,
	VRSEL_CB_MOD_VARIETY_OK,
	VRSEL_CB_MOD_STREAKING_OK,

	VRSEL_CB_MOD_VR_CANCEL,						/* モード上でキャンセル操作が行われた */

	/* 画面：モード選択（ＡＬＴ） */
	VRSEL_CB_MOD_BOMB_DISPOSAL_UP,				/* モード上で上下操作が行われた */
	VRSEL_CB_MOD_BOMB_DISPOSAL_DOWN,
	VRSEL_CB_MOD_ELIMINATE_UP,
	VRSEL_CB_MOD_ELIMINATE_DOWN,
	VRSEL_CB_MOD_HOLD_UP_UP,
	VRSEL_CB_MOD_HOLD_UP_DOWN,
	VRSEL_CB_MOD_PHOTOGRAPH_UP,
	VRSEL_CB_MOD_PHOTOGRAPH_DOWN,

	VRSEL_CB_MOD_BOMB_DISPOSAL_OK,				/* モード上で決定操作が行われた */
	VRSEL_CB_MOD_ELIMINATE_OK,
	VRSEL_CB_MOD_HOLD_UP_OK,
	VRSEL_CB_MOD_PHOTOGRAPH_OK,

	VRSEL_CB_MOD_ALT_CANCEL,					/* モード上でキャンセル操作が行われた */

	/* 画面：武器選択（ＳＮＥＡＫＩＮＧ） */
	VRSEL_CB_WEA_SNEAKING_UP,					/* 武器上で上下操作が行われた */
	VRSEL_CB_WEA_SNEAKING_DOWN,
	VRSEL_CB_WEA_ELIMINATE_ALL_UP,
	VRSEL_CB_WEA_ELIMINATE_ALL_DOWN,

	VRSEL_CB_WEA_SNEAKING_OK,					/* 武器上で決定操作が行われた */
	VRSEL_CB_WEA_ELIMINATE_ALL_OK,

	VRSEL_CB_WEA_SNE_CANCEL,					/* 武器上でキャンセル操作が行われた */

	/* 画面：武器選択（ＷＥＡＰＯＮ） */
	VRSEL_CB_WEA_HANDGUN_UP,					/* 武器上で上下操作が行われた */
	VRSEL_CB_WEA_HANDGUN_DOWN,
	VRSEL_CB_WEA_ASSAULT_RIFLE_UP,
	VRSEL_CB_WEA_ASSAULT_RIFLE_DOWN,
	VRSEL_CB_WEA_C4_CLAYMORE_UP,
	VRSEL_CB_WEA_C4_CLAYMORE_DOWN,
	VRSEL_CB_WEA_GRENADE_UP,
	VRSEL_CB_WEA_GRENADE_DOWN,
	VRSEL_CB_WEA_PSG1_UP,
	VRSEL_CB_WEA_PSG1_DOWN,
	VRSEL_CB_WEA_STINGER_UP,
	VRSEL_CB_WEA_STINGER_DOWN,
	VRSEL_CB_WEA_NIKITA_UP,
	VRSEL_CB_WEA_NIKITA_DOWN,
	VRSEL_CB_WEA_HF_BLADE_UP,
	VRSEL_CB_WEA_HF_BLADE_DOWN,

	VRSEL_CB_WEA_HANDGUN_OK,					/* 武器上で決定操作が行われた */
	VRSEL_CB_WEA_ASSAULT_RIFLE_OK,
	VRSEL_CB_WEA_C4_CLAYMORE_OK,
	VRSEL_CB_WEA_GRENADE_OK,
	VRSEL_CB_WEA_PSG1_OK,
	VRSEL_CB_WEA_STINGER_OK,
	VRSEL_CB_WEA_NIKITA_OK,
	VRSEL_CB_WEA_HF_BLADE_OK,

	VRSEL_CB_WEA_WEA_CANCEL,					/* 武器上でキャンセル操作が行われた */

	/* 画面：レベル選択 */
	VRSEL_CB_LEV_01_UP,							/* レベル上で上下操作が行われた */
	VRSEL_CB_LEV_01_DOWN,
	VRSEL_CB_LEV_02_UP,
	VRSEL_CB_LEV_02_DOWN,
	VRSEL_CB_LEV_03_UP,
	VRSEL_CB_LEV_03_DOWN,
	VRSEL_CB_LEV_04_UP,
	VRSEL_CB_LEV_04_DOWN,
	VRSEL_CB_LEV_05_UP,
	VRSEL_CB_LEV_05_DOWN,
	VRSEL_CB_LEV_06_UP,
	VRSEL_CB_LEV_06_DOWN,
	VRSEL_CB_LEV_07_UP,
	VRSEL_CB_LEV_07_DOWN,
	VRSEL_CB_LEV_08_UP,
	VRSEL_CB_LEV_08_DOWN,
	VRSEL_CB_LEV_09_UP,
	VRSEL_CB_LEV_09_DOWN,
	VRSEL_CB_LEV_10_UP,
	VRSEL_CB_LEV_10_DOWN,
	VRSEL_CB_LEV_CONTROL_UP,
	VRSEL_CB_LEV_CONTROL_DOWN,

	VRSEL_CB_LEV_01_OK,							/* レベル上で決定操作が行われた */
	VRSEL_CB_LEV_02_OK,
	VRSEL_CB_LEV_03_OK,
	VRSEL_CB_LEV_04_OK,
	VRSEL_CB_LEV_05_OK,
	VRSEL_CB_LEV_06_OK,
	VRSEL_CB_LEV_07_OK,
	VRSEL_CB_LEV_08_OK,
	VRSEL_CB_LEV_09_OK,
	VRSEL_CB_LEV_10_OK,
	VRSEL_CB_LEV_CONTROL_OK,
	VRSEL_CB_LEV_CLEAR_CODE,

	VRSEL_CB_LEV_CANCEL,						/* レベル上でキャンセル操作が行われた */

	VRSEL_CB_L2_OPENED_1,						/* レイアウト２が開いた（ミッション選択） */
	VRSEL_CB_L2_OPENED_2,						/* レイアウト２が開いた（レベル選択） */
	VRSEL_CB_L2_FADEOUT,						/* レイアウト２でテキストがフェードアウトした */
	VRSEL_CB_L2_CLOSED_1,						/* レイアウト２が閉じた（ミッション選択） */
	VRSEL_CB_L2_CLOSED_2,						/* レイアウト２が閉じた（レベル選択） */



	VRSEL_CB_L1,									/* Ｌ１キーが押された */
	VRSEL_CB_R1,									/* Ｒ１キーが押された */
	VRSEL_CB_L2,									/* Ｌ２キーが押された */
	VRSEL_CB_R2,									/* Ｒ２キーが押された */
	VRSEL_CB_L3,									/* Ｌ３キーが押された */
	VRSEL_CB_R3,									/* Ｒ３キーが押された */
};

/* フェーズタイプ */
enum {
	VRSEL_PHASE_PLA = 1,
	VRSEL_PHASE_L1_OPEN,
	VRSEL_PHASE_L1_CLOSE,
	VRSEL_PHASE_MIS,
	VRSEL_PHASE_MOD_VR,
	VRSEL_PHASE_MOD_ALT,
	VRSEL_PHASE_WEA_SNEAKING,
	VRSEL_PHASE_WEA_WEAPON,
	VRSEL_PHASE_LEV_SNEAKING,
	VRSEL_PHASE_LEV_ELIMINATE_ALL,
	VRSEL_PHASE_LEV_HANDGUN,
	VRSEL_PHASE_LEV_ASSAULT_RIFLE,
	VRSEL_PHASE_LEV_C4_CLAYMORE,
	VRSEL_PHASE_LEV_GRENADE,
	VRSEL_PHASE_LEV_PSG1,
	VRSEL_PHASE_LEV_STINGER,
	VRSEL_PHASE_LEV_NIKITA,
	VRSEL_PHASE_LEV_HF_BLADE,
	VRSEL_PHASE_LEV_FIRST_PERSON_VIEW,
	VRSEL_PHASE_LEV_VARIETY,
	VRSEL_PHASE_LEV_STREAKING,
	VRSEL_PHASE_LEV_BOMB_DISPOSAL,
	VRSEL_PHASE_LEV_ELIMINATE,
	VRSEL_PHASE_LEV_HOLD_UP,
	VRSEL_PHASE_LEV_PHOTOGRAPH,
	VRSEL_PHASE_HIDE_SNEAKING,
	VRSEL_PHASE_HIDE_ELIMINATE_ALL,
	VRSEL_PHASE_HIDE_HANDGUN,
	VRSEL_PHASE_HIDE_ASSAULT_RIFLE,
	VRSEL_PHASE_HIDE_C4_CLAYMORE,
	VRSEL_PHASE_HIDE_GRENADE,
	VRSEL_PHASE_HIDE_PSG1,
	VRSEL_PHASE_HIDE_STINGER,
	VRSEL_PHASE_HIDE_NIKITA,
	VRSEL_PHASE_HIDE_HF_BLADE,
	VRSEL_PHASE_HIDE_FIRST_PERSON_VIEW,
	VRSEL_PHASE_HIDE_VARIETY,
	VRSEL_PHASE_HIDE_STREAKING,
	VRSEL_PHASE_HIDE_BOMB_DISPOSAL,
	VRSEL_PHASE_HIDE_ELIMINATE,
	VRSEL_PHASE_HIDE_HOLD_UP,
	VRSEL_PHASE_HIDE_PHOTOGRAPH,
	VRSEL_PHASE_L2_OPEN_1,
	VRSEL_PHASE_L2_OPEN_2,
	VRSEL_PHASE_L2_CLOSE_1,
	VRSEL_PHASE_L2_CLOSE_2,
};

/* プレイヤータイプ */
enum {
	VRSEL_PLA_NO_PLAYER = -1,	/* 未選択 */
	VRSEL_PLA_RAIDEN,			/* ライデン */
	VRSEL_PLA_NINJA,			/* 忍者ライデン */
	VRSEL_PLA_X_RAIDEN,			/* Ｘ－ライデン */
	VRSEL_PLA_SNAKE,			/* スネーク */
	VRSEL_PLA_PLISKIN,			/* プリスキン */
	VRSEL_PLA_TUXEDO,			/* タキシード・スネーク */
	VRSEL_PLA_PREVIOUS,			/* 前作スネーク */
	VRSEL_PLA_MAX,
	VRSEL_SAVE = VRSEL_PLA_MAX,
	VRSEL_EXIT,
};

/* レベルタイプ */
enum {
	VRSEL_LEVEL_01 = 0,
	VRSEL_LEVEL_02,
	VRSEL_LEVEL_03,
	VRSEL_LEVEL_04,
	VRSEL_LEVEL_05,
	VRSEL_LEVEL_06,
	VRSEL_LEVEL_07,
	VRSEL_LEVEL_08,
	VRSEL_LEVEL_09,
	VRSEL_LEVEL_10,
	VRSEL_CONTROLS,
};

/* ムービータイプ */
enum {
	VRSEL_MOV_SNEAKING_RAIDEN = 0,
	VRSEL_MOV_SNEAKING_SNAKE,
	VRSEL_MOV_WEAPON_RAIDEN,
	VRSEL_MOV_WEAPON_SNAKE,
	VRSEL_MOV_FIRST_PERSON_VIEW_RAIDEN,
	VRSEL_MOV_FIRST_PERSON_VIEW_SNAKE,
	VRSEL_MOV_VARIETY_RAIDEN,
	VRSEL_MOV_VARIETY_SNAKE,
	VRSEL_MOV_BOMB_DISPOSAL_RAIDEN,
	VRSEL_MOV_BOMB_DISPOSAL_SNAKE,
	VRSEL_MOV_ELIMINATE_RAIDEN,
	VRSEL_MOV_ELIMINATE_SNAKE,
	VRSEL_MOV_HOLD_UP_RAIDEN,
	VRSEL_MOV_HOLD_UP_SNAKE,
	VRSEL_MOV_PHOTOGRAPH_RAIDEN,
	VRSEL_MOV_PHOTOGRAPH_SNAKE,
};



/* フラグ */
#define VRSEL_FLAG_INIT           (0x0001)		/* 初期化時 */
#define VRSEL_FLAG_INIT2          (0x0002)		/* 初期化時２ */
#define VRSEL_FLAG_INPUT_OK       (0x0004)		/* 入力可能状態 */
#define VRSEL_FLAG_WAIT_ACTION    (0x0008)		/* 入力不可能状態（アクション監視）２つセットで使用 */
#define VRSEL_FLAG_WAIT_ACTION2   (0x0010)		/* 入力不可能状態（アクション監視）２つセットで使用 */
#define VRSEL_FLAG_NO_PLAYER      (0x0020)		/* プレイヤー選択モードで起動 */
#define VRSEL_FLAG_LAYOUT_2       (0x0040)		/* レイアウト２を起動中 */
#define VRSEL_FLAG_LOAD_PLAYER    (0x0080)		/* 新しいプレイヤーをロード */
#define VRSEL_FLAG_REQUEST_MOVIE  (0x0100)		/* ムービーをリクエスト */
#define VRSEL_FLAG_HIDE_RECORD    (0x0200)		/* レコード表示を一時的に消す */
#define VRSEL_FLAG_PHASE_CHANGING (0x0400)		/* フェーズ変更中 */

/* 設定値 */
#define VRSEL_TEX_WIDTH      (18.0f)		/* テクスチャ幅 */
#define VRSEL_ICO_R          (-27)			/* アイコン相対位置Ｘ */
#define VRSEL_ICO_R2         (-27)			/* アイコン相対位置Ｘ */
#define VRSEL_ICO_T          (2)			/* アイコン相対位置Ｙ */
#define VRSEL_INPUT_INTERVAL (8)			/* 入力インターバル */
#define VRSEL_JUSTINCASE     (600)			/* 念のため */
#define VRSEL_KONAMICOMMAND  "UUDDLRLRBA"



/* Ｌ２Ｄオブジェクト１ */
#define L2D_FILENAME_1       (15076103)		/* missions_character_select */

#define ACTION_1_DUMMY       ( 8143391)		/* dummy */
#define ACTION_1_DUMMY2      ( 8930337)		/* dummy2 */
#define ACTION_1_OPEN_FRAME  (  863270)		/* openFrame */
#define ACTION_1_CLOSE_FRAME ( 6956960)		/* closeFrame */

#define OBJECT_1_PIC_PREVIOUS ( 1797836)	/* bg_old_sna */
#define OBJECT_1_PIC_NINJA    ( 7564590)	/* bg_rai_ninjya */
#define OBJECT_1_PIC_TUXEDO   (15430930)	/* bg_sna_txd */
#define OBJECT_1_PIC_PLISKIN  ( 7687801)	/* bg_pliskin */
#define OBJECT_1_PIC_SNAKE    ( 7060554)	/* bg_snake */
#define OBJECT_1_PIC_RAIDEN   (11234744)	/* bg_raiden */
#define OBJECT_1_PIC_X_RAIDEN ( 8976205)	/* bg_x_raiden */

#define OBJECT_1_TXT_RAIDEN     ( 4595192)	/* raiden */
#define OBJECT_1_TXT_NINJA_1    (10650006)	/* raidenNinja */
#define OBJECT_1_TXT_NINJA_2    ( 1459624)	/* ninja */
#define OBJECT_1_TXT_NINJA_Q    (12718318)	/* raiden_ninja_question */
#define OBJECT_1_TXT_X_RAIDEN   ( 4847032)	/* x_raiden */
#define OBJECT_1_TXT_X_RAIDEN_Q (14128918)	/* x_raiden_question */
#define OBJECT_1_TXT_SNAKE      ( 6853068)	/* snake */
#define OBJECT_1_TXT_PLISKIN    (13325932)	/* pliskin */
#define OBJECT_1_TXT_PLISKIN_Q  ( 6800173)	/* pliskin_question */
#define OBJECT_1_TXT_TUXEDO_1   (11670712)	/* snakeTuxedo */
#define OBJECT_1_TXT_TUXEDO_2   ( 9282014)	/* tuxedo */
#define OBJECT_1_TXT_TUXEDO_Q   (15553903)	/* snake_txd_question */
#define OBJECT_1_TXT_PREVIOUS_1 (15643565)	/* snakeMgs1 */
#define OBJECT_1_TXT_PREVIOUS_2 ( 3680913)	/* mgs1 */
#define OBJECT_1_TXT_PREVIOUS_Q (15553215)	/* snake_old_question */
#define OBJECT_1_TXT_SAVE       ( 3871525)	/* save */
#define OBJECT_1_TXT_EXIT       ( 3435924)	/* exit */

#define OBJECT_1_001_RAIDEN   (10731239)	/* raiden_num_1 */
#define OBJECT_1_010_RAIDEN   ( 7855396)	/* raiden_num_10 */
#define OBJECT_1_100_RAIDEN   (16491710)	/* raiden_num_100 */
#define OBJECT_1_001_NINJA    (12363390)	/* raidenNinja_num_1 */
#define OBJECT_1_010_NINJA    ( 9752583)	/* raidenNinja_num_10 */
#define OBJECT_1_100_NINJA    (10092834)	/* raidenNinja_num_100 */
#define OBJECT_1_001_X_RAIDEN (10071784)	/* x_raiden_num_1 */
#define OBJECT_1_010_X_RAIDEN ( 3530051)	/* x_raiden_num_10 */
#define OBJECT_1_100_X_RAIDEN (12298390)	/* x_raiden_num_100 */
#define OBJECT_1_001_SNAKE    ( 4240368)	/* snake_num_1 */
#define OBJECT_1_010_SNAKE    ( 1474104)	/* snake_num_10 */
#define OBJECT_1_100_SNAKE    (13616946)	/* snake_num_100 */
#define OBJECT_1_001_PLISKIN  (15850504)	/* pliskin_num_1 */
#define OBJECT_1_010_PLISKIN  ( 3899726)	/* pliskin_num_10 */
#define OBJECT_1_100_PLISKIN  ( 7350775)	/* pliskin_num_100 */
#define OBJECT_1_001_TUXEDO   (10579714)	/* snakeTuxedo_num_1 */
#define OBJECT_1_010_TUXEDO   ( 3006596)	/* snakeTuxedo_num_10 */
#define OBJECT_1_100_TUXEDO   (12325045)	/* snakeTuxedo_num_100 */
#define OBJECT_1_001_PREVIOUS (13184081)	/* snakeMgs1_num_1 */
#define OBJECT_1_010_PREVIOUS ( 2460265)	/* snakeMgs1_num_10 */
#define OBJECT_1_100_PREVIOUS (11619668)	/* snakeMgs1_num_100 */
#define OBJECT_1_PCT_RAIDEN     ( 8661968)	/* percent_01 */
#define OBJECT_1_PCT_NINJA      ( 8661969)	/* percent_02 */
#define OBJECT_1_PCT_X_RAIDEN   ( 8661970)	/* percent_03 */
#define OBJECT_1_PCT_SNAKE      ( 8661971)	/* percent_04 */
#define OBJECT_1_PCT_PLISKIN    ( 8661972)	/* percent_05 */
#define OBJECT_1_PCT_TUXEDO     ( 8661973)	/* percent_06 */
#define OBJECT_1_PCT_PREVIOUS   ( 8661974)	/* percent_07 */



/* Ｌ２Ｄオブジェクト２ */
#ifdef PSX2
#define L2D_FILENAME_2_1 (10248738)		/* missions_raiden_ps2 */
#define L2D_FILENAME_2_2 ( 5740307)		/* missions_raiden_ninja_ps2 */
#define L2D_FILENAME_2_3 ( 8171912)		/* missions_x_raiden_ps2 */
#define L2D_FILENAME_2_4 ( 3750989)		/* missions_snake_def_ps2 */
#define L2D_FILENAME_2_5 (13908501)		/* missions_pliskin_ps2 */
#define L2D_FILENAME_2_6 ( 1654899)		/* missions_snake_txd_ps2 */
#define L2D_FILENAME_2_7 ( 1654555)		/* missions_snake_old_ps2 */
#else
#define L2D_FILENAME_2_1 (11614470)		/* missions_raiden */
#define L2D_FILENAME_2_2 ( 6588434)		/* missions_raiden_ninja */
#define L2D_FILENAME_2_3 (11939684)		/* missions_x_raiden */
#define L2D_FILENAME_2_4 ( 8313776)		/* missions_snake_def */
#define L2D_FILENAME_2_5 ( 3061818)		/* missions_pliskin */
#define L2D_FILENAME_2_6 ( 8330766)		/* missions_snake_txd */
#define L2D_FILENAME_2_7 ( 8325262)		/* missions_snake_old */
#endif
																										/*	rai	nin	x-r	sna	pli	tux	pre	*/
#define ACTION_2_DUMMY                       ( 8143391)		/* dummy */									/*	o	o	o	o	o	o	o	*/
#define ACTION_2_DUMMY2                      ( 8930337)		/* dummy2 */								/*	o	o	o	o	o	o	o	*/

#define ACTION_2_OPEN_FRAME                  (  863270)		/* openFrame */								/*	o	o	o	o	o	o	o	*/
#define ACTION_2_CLOSE_FRAME                 ( 6956960)		/* closeFrame */							/*	o	o	o	o	o	o	o	*/

#define ACTION_2_VR_MISSIONS                 ( 9709399)		/* move_vr */								/*	o	o	o	o	o	o	o	*/
#define ACTION_2_VR_MISSIONS_RET             (13023225)		/* move_vr_return */						/*	o	o	o	o	o	o	o	*/
#define ACTION_2_ALT_MISSIONS                ( 8689318)		/* move_alt */								/*	o	o		o	o	o	o	*/
#define ACTION_2_ALT_MISSIONS_RET            ( 4271997)		/* move_alt_return */						/*	o	o		o	o	o	o	*/

#define ACTION_2_SNEAKING                    (11335500)		/* sneakingmode */							/*	o	o		o	o	o	o	*/
#define ACTION_2_SNEAKING_RET                ( 4612288)		/* sneakingmode_return */					/*	o	o		o	o	o	o	*/
#define ACTION_2_WEAPON                      ( 6319577)		/* weaponmode */							/*	o	o		o	o	o	o	*/
#define ACTION_2_WEAPON_RET                  (16434779)		/* weaponmode_return */						/*	o	o		o	o	o	o	*/
#define ACTION_2_FIRST_PERSON_VIEW           ( 8964546)		/* first_person_view_mode */				/*	o			o				*/
#define ACTION_2_FIRST_PERSON_VIEW_RET       (14290846)		/* first_person_view_mode_return */			/*	o			o				*/
#define ACTION_2_VARIETY                     ( 6139129)		/* varietymode */							/*	o	o		o	o	o	o	*/
#define ACTION_2_VARIETY_RET                 (15976005)		/* varietymode_return */					/*	o	o		o	o	o	o	*/
#define ACTION_2_STREAKING                   (10758106)		/* streaking_mode */						/*			o					*/
#define ACTION_2_STREAKING_RET               (13291641)		/* streaking_mode_return */					/*			o					*/
#define ACTION_2_BOMB_DISPOSAL               (15475422)		/* bomb_disposal_mode */					/*	o	o		o	o	o	o	*/
#define ACTION_2_BOMB_DISPOSAL_RET           (10678969)		/* bomb_disposal_mode_return */				/*	o	o		o	o	o	o	*/
#define ACTION_2_ELIMINATE                   ( 9069815)		/* eliminate_mode */						/*	o	o		o	o	o	o	*/
#define ACTION_2_ELIMINATE_RET               (11777963)		/* eliminate_mode_return */					/*	o	o		o	o	o	o	*/
#define ACTION_2_HOLD_UP                     ( 1021141)		/* hold_up_mode */							/*	o			o	o	o	o	*/
#define ACTION_2_HOLD_UP_RET                 ( 3318741)		/* hold_up_mode_return */					/*	o			o	o	o	o	*/
#define ACTION_2_PHOTOGRAPH                  ( 5544353)		/* photograph_mode */						/*	o			o				*/
#define ACTION_2_PHOTOGRAPH_RET              ( 5834237)		/* photograph_mode_return */				/*	o			o				*/

#define ACTION_2_SNEAKING_SNEAKING           ( 9651014)		/* sneaking_sneaking */						/*	o	o		o	o	o	o	*/
#define ACTION_2_SNEAKING_SNEAKING_RET       (10891250)		/* sneaking_sneaking_return */				/*	o	o		o	o	o	o	*/
#define ACTION_2_SNEAKING_ELIMINATE_ALL      ( 2415723)		/* sneaking_eliminate_all */				/*	o	o		o	o	o	o	*/
#define ACTION_2_SNEAKING_ELIMINATE_ALL_RET  ( 7296127)		/* sneaking_eliminate_all_return */			/*	o	o		o	o	o	o	*/
#define ACTION_2_WEAPON_HANDGUN              ( 5849126)		/* weapon_handgun */						/*	o			o	o	o	o	*/
#define ACTION_2_WEAPON_HANDGUN_RET          ( 9252386)		/* weapon_handgun_return */					/*	o			o	o	o	o	*/
#define ACTION_2_WEAPON_ASSAULT_RIFLE        ( 5472980)		/* weapon_assault_rifle */					/*	o			o	o	o	o	*/
#define ACTION_2_WEAPON_ASSAULT_RIFLE_RET    (10657268)		/* weapon_assault_rifle_return */			/*	o			o	o	o	o	*/
#define ACTION_2_WEAPON_C4_CLAYMORE          (10409894)		/* weapon_c4_claymore */					/*	o			o	o	o	o	*/
#define ACTION_2_WEAPON_C4_CLAYMORE_RET      ( 4796495)		/* weapon_c4_claymore_return */				/*	o			o	o	o	o	*/
#define ACTION_2_WEAPON_GRENADE              (13510110)		/* weapon_grenade */						/*	o			o	o	o	o	*/
#define ACTION_2_WEAPON_GRENADE_RET          (12251593)		/* weapon_grenade_return */					/*	o			o	o	o	o	*/
#define ACTION_2_WEAPON_PSG1                 (13564278)		/* weapon_psg_1 */							/*	o			o	o	o	o	*/
#define ACTION_2_WEAPON_PSG1_RET             ( 5747152)		/* weapon_psg_1_return */					/*	o			o	o	o	o	*/
#define ACTION_2_WEAPON_STINGER              (  934160)		/* weapon_stinger */						/*	o			o	o	o	o	*/
#define ACTION_2_WEAPON_STINGER_RET          ( 9731018)		/* weapon_stinger_return */					/*	o			o	o	o	o	*/
#define ACTION_2_WEAPON_NIKITA               ( 4284310)		/* weapon_nikita */							/*	o			o	o	o	o	*/
#define ACTION_2_WEAPON_NIKITA_RET           ( 8957283)		/* weapon_nikita_return */					/*	o			o	o	o	o	*/
#define ACTION_2_WEAPON_HF_BLADE             (  668946)		/* weapon_hf_blade */						/*	o	o		o	o	o	o	*/
#define ACTION_2_WEAPON_HF_BLADE_RET         ( 3443626)		/* weapon_hf_blade_return */				/*	o	o		o	o	o	o	*/

#define ACTION_2_SHOW_FIRST_PERSON_VIEW      ( 5923637)		/* show_first_person_view_mode */			/*	o			o				*/
#define ACTION_2_HIDE_FIRST_PERSON_VIEW      (13252425)		/* hide_first_person_view_mode */			/*	o			o				*/
#define ACTION_2_SHOW_VARIETY                ( 4411436)		/* show_varietymode */						/*	o	o		o	o	o	o	*/
#define ACTION_2_HIDE_VARIETY                ( 7090132)		/* hide_varietymode */						/*	o	o		o	o	o	o	*/
#define ACTION_2_SHOW_STREAKING              ( 4037292)		/* show_streaking_mode */					/*			o					*/
#define ACTION_2_HIDE_STREAKING              ( 1158940)		/* hide_streaking_mode */					/*			o					*/
#define ACTION_2_SHOW_BOMB_DISPOSAL          (  375308)		/* show_bomb_disposal_mode */				/*	o	o		o	o	o	o	*/
#define ACTION_2_HIDE_BOMB_DISPOSAL          (  195411)		/* hide_bomb_disposal_mode */				/*	o	o		o	o	o	o	*/
#define ACTION_2_SHOW_ELIMINATE              ( 2349001)		/* show_eliminate_mode */					/*	o	o		o	o	o	o	*/
#define ACTION_2_HIDE_ELIMINATE              (16247864)		/* hide_eliminate_mode */					/*	o	o		o	o	o	o	*/
#define ACTION_2_SHOW_HOLD_UP                (12843825)		/* show_hold_up_mode */						/*	o			o	o	o	o	*/
#define ACTION_2_HIDE_HOLD_UP                (14676022)		/* hide_hold_up_mode */						/*	o			o	o	o	o	*/
#define ACTION_2_SHOW_PHOTOGRAPH             ( 8582100)		/* show_photograph_mode */					/*	o			o				*/
#define ACTION_2_HIDE_PHOTOGRAPH             (  360911)		/* hide_photograph_mode */					/*	o			o				*/
#define ACTION_2_SHOW_SNEAKING_SNEAKING      (16519167)		/* show_sneaking_sneaking */				/*	o	o		o	o	o	o	*/
#define ACTION_2_HIDE_SNEAKING_SNEAKING      ( 3406346)		/* hide_sneaking_sneaking */				/*	o	o		o	o	o	o	*/
#define ACTION_2_SHOW_SNEAKING_ELIMINATE_ALL (16152029)		/* show_sneaking_eliminate_all */			/*	o	o		o	o	o	o	*/
#define ACTION_2_HIDE_SNEAKING_ELIMINATE_ALL ( 6703602)		/* hide_sneaking_eliminate_all */			/*	o	o		o	o	o	o	*/
#define ACTION_2_SHOW_WEAPON_HANDGUN         (15905527)		/* show_weapon_handgun */					/*	o			o	o	o	o	*/
#define ACTION_2_HIDE_WEAPON_HANDGUN         (13027175)		/* hide_weapon_handgun */					/*	o			o	o	o	o	*/
#define ACTION_2_SHOW_WEAPON_ASSAULT_RIFLE   (11548474)		/* show_weapon_assault_rifle */				/*	o			o	o	o	o	*/
#define ACTION_2_HIDE_WEAPON_ASSAULT_RIFLE   (11883311)		/* hide_weapon_assault_rifle */				/*	o			o	o	o	o	*/
#define ACTION_2_SHOW_WEAPON_C4_CLAYMORE     (12086995)		/* show_weapon_c4_claymore */				/*	o			o	o	o	o	*/
#define ACTION_2_HIDE_WEAPON_C4_CLAYMORE     (11907098)		/* hide_weapon_c4_claymore */				/*	o			o	o	o	o	*/
#define ACTION_2_SHOW_WEAPON_GRENADE         ( 6789296)		/* show_weapon_grenade */					/*	o			o	o	o	o	*/
#define ACTION_2_HIDE_WEAPON_GRENADE         ( 3910944)		/* hide_weapon_grenade */					/*	o			o	o	o	o	*/
#define ACTION_2_SHOW_WEAPON_PSG1            ( 1317583)		/* show_weapon_psg1 */						/*	o			o	o	o	o	*/
#define ACTION_2_HIDE_WEAPON_PSG1            ( 3996279)		/* hide_weapon_psg1 */						/*	o			o	o	o	o	*/
#define ACTION_2_SHOW_WEAPON_STINGER         (10990561)		/* show_weapon_stinger */					/*	o			o	o	o	o	*/
#define ACTION_2_HIDE_WEAPON_STINGER         ( 8112209)		/* hide_weapon_stinger */					/*	o			o	o	o	o	*/
#define ACTION_2_SHOW_WEAPON_NIKITA          (13511468)		/* show_weapon_nikita */					/*	o			o	o	o	o	*/
#define ACTION_2_HIDE_WEAPON_NIKITA          ( 5032912)		/* hide_weapon_nikita */					/*	o			o	o	o	o	*/
#define ACTION_2_SHOW_WEAPON_HF_BLADE        ( 3706693)		/* show_weapon_hf_blade */					/*	o	o		o	o	o	o	*/
#define ACTION_2_HIDE_WEAPON_HF_BLADE        (12262719)		/* hide_weapon_hf_blade */					/*	o	o		o	o	o	o	*/
#define ACTION_2_CLOSE_FRAME_2               (13743941)		/* closeFrame_02 */							/*	o	o	o	o	o	o	o	*/

/* 基本 */																/*	rai	nin	x-r	sna	pli	tux	pre	*/
#define OBJECT_2_BOX_MOVIE    (13147979)	/* window */				/*	o	o	o	o	o	o	o	*/
/* クリア％ */
#define OBJECT_2_PCT_PERCENT (5455621)	/* percent */
#define OBJECT_2_PCT_100     (  26273)	/* num_100 */
#define OBJECT_2_PCT_010     (8913715)	/* num_10 */
#define OBJECT_2_PCT_001     (1851416)	/* num_1 */
/* ミッションテキスト */															/*	rai	nin	x-r	sna	pli	tux	pre	*/
#define OBJECT_2_MIS_VR          (10145135)		/* vrMissions */					/*	o	o	o	o	o	o	o	*/
#define OBJECT_2_MIS_ALTERNATIVE (15805603)		/* alternative_missions */			/*	o	o		o	o	o	o	*/
/* モードテキスト */															/*	rai	nin	x-r	sna	pli	tux	pre	*/
#define OBJECT_2_MOD_SNEAKING   (10286924)	/* sneakingMode */					/*	o	o		o	o	o	o	*/
#define OBJECT_2_MOD_WEAPON     ( 5271001)	/* weaponMode */					/*	o			o	o	o	o	*/
#define OBJECT_2_MOD_FSV_1      (14861973)	/* first_person_view_mode_01 */		/*	o			o	o	o	o	*/
#define OBJECT_2_MOD_FSV_2      (14861974)	/* first_person_view_mode_02 */		/*	o			o	o	o	o	*/
#define OBJECT_2_MOD_VARIETY    (13945711)	/* variety_mode */					/*	o	o		o	o	o	o	*/
#define OBJECT_2_MOD_STREAKING  (10758106)	/* streaking_mode */				/*			o					*/
#define OBJECT_2_MOD_BOMB_D     (15475422)	/* bomb_disposal_mode */			/*	o	o		o	o	o	o	*/
#define OBJECT_2_MOD_ELIMINATE  ( 9069815)	/* eliminate_mode */				/*	o	o		o	o	o	o	*/
#define OBJECT_2_MOD_HOLD_UP    ( 1021141)	/* hold_up_mode */					/*	o			o	o	o	o	*/
#define OBJECT_2_MOD_PHOTOGRAPH ( 5544353)	/* photograph_mode */				/*	o			o				*/
#define OBJECT_2_MOD_QUESTION_1 (  457130)	/* question_mode */					/*	o	o		o	o	o	o	*/
#define OBJECT_2_MOD_QUESTION_2 (14058926)	/* question_mode_02 */				/*	o	o		o	o	o	o	*/
/* 武器テキスト */														/*	rai	nin	x-r	sna	pli	tux	pre	*/
#define OBJECT_2_WEA_SNEAKING      (15982338)		/* no_weapon */			/*	o	o		o	o	o	o	*/
#define OBJECT_2_WEA_ELIMINATE_ALL ( 9708236)		/* eliminate_all */		/*	o	o		o	o	o	o	*/
#define OBJECT_2_WEA_HANDGUN       ( 1295831)		/* handgun */			/*	o			o	o	o	o	*/
#define OBJECT_2_WEA_ASSAULT_R     (16051970)		/* assault_rifle */		/*	o			o	o	o	o	*/
#define OBJECT_2_WEA_C4_CLAYMORE   ( 8028161)		/* c4/claymore */		/*	o			o	o	o	o	*/
#define OBJECT_2_WEA_GRENADE       ( 8956815)		/* grenade */			/*	o			o	o	o	o	*/
#define OBJECT_2_WEA_PSG1          ( 3876888)		/* psg_1 */				/*	o			o	o	o	o	*/
#define OBJECT_2_WEA_STINGER       (13158080)		/* stinger */			/*	o			o	o	o	o	*/
#define OBJECT_2_WEA_NIKITA        (13054915)		/* nikita */			/*	o			o	o	o	o	*/
#define OBJECT_2_WEA_HF_BLADE      ( 5958441)		/* hf_blade */			/*	o			o	o	o	o	*/
/* 武器アイコン */												/*	rai	nin	x-r	sna	pli	tux	pre	*/
#define OBJECT_2_ICO_HANDGUN_1       (   3545)	/* m9 */		/*	o			o	o	o	o	*/
#define OBJECT_2_ICO_HANDGUN_2       (6917057)	/* usp_sp */	/*	o			o	o	o	o	*/
#define OBJECT_2_ICO_ASSAULT_RIFLE_1 (7473930)	/* m4a_nm */	/*	o			o	o	o	o	*/
#define OBJECT_2_ICO_ASSAULT_RIFLE_2 ( 102867)	/* aks */		/*	o			o	o	o	o	*/
#define OBJECT_2_ICO_C4_CLAYMORE_1   ( 104754)	/* cfr */		/*	o			o	o	o	o	*/
#define OBJECT_2_ICO_C4_CLAYMORE_2   (3363993)	/* cray */		/*	o			o	o	o	o	*/
#define OBJECT_2_ICO_GRENADE_1       ( 120130)	/* rgb */		/*	o			o	o	o	o	*/
#define OBJECT_2_ICO_GRENADE_2       ( 109221)	/* gre */		/*	o			o	o	o	o	*/
#define OBJECT_2_ICO_PSG1            (3791121)	/* psg1 */		/*	o			o	o	o	o	*/
#define OBJECT_2_ICO_STINGER         (3890574)	/* stin */		/*	o			o	o	o	o	*/
#define OBJECT_2_ICO_NIKITA          (3715529)	/* niki */		/*	o			o	o	o	o	*/
#define OBJECT_2_ICO_HF_BLADE        ( 109858)	/* hfb */		/*	o			o	o	o	o	*/
/* ステージレベル */											/*	rai	nin	x-r	sna	pli	tux	pre	*/
#define OBJECT_2_LEV_01      (7992353)	/* LEVEL_01 */			/*	o	o	o	o	o	o	o	*/
#define OBJECT_2_LEV_02      (7992354)	/* LEVEL_02 */			/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_03      (7992355)	/* LEVEL_03 */			/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_04      (7992356)	/* LEVEL_04 */			/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_05      (7992357)	/* LEVEL_05 */			/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_06      (7992358)	/* LEVEL_06 */			/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_07      (7992359)	/* LEVEL_07 */			/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_08      (7992360)	/* LEVEL_08 */			/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_09      (7992361)	/* LEVEL_09 */			/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_10      (7992384)	/* LEVEL_10 */			/*	o	o		o	o	o	o	*/
#ifndef KP_WINDOWS
#define OBJECT_2_LEV_CONTROL (8257174)	/* controls */
#endif
#define OBJECT_2_LEV_01_01   (1342735)	/* LEVEL_num_1 */		/*	o	o	o	o	o	o	o	*/
#define OBJECT_2_LEV_01_10   (8934991)	/* LEVEL_num_10_1 */	/*	o	o	o	o	o	o	o	*/
#define OBJECT_2_LEV_02_01   (1342736)	/* LEVEL_num_2 */		/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_02_10   (8934992)	/* LEVEL_num_10_2 */	/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_03_01   (1342737)	/* LEVEL_num_3 */		/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_03_10   (8934993)	/* LEVEL_num_10_3 */	/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_04_01   (1342738)	/* LEVEL_num_4 */		/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_04_10   (8934994)	/* LEVEL_num_10_4 */	/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_05_01   (1342739)	/* LEVEL_num_5 */		/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_05_10   (8934995)	/* LEVEL_num_10_5 */	/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_06_01   (1342740)	/* LEVEL_num_6 */		/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_06_10   (8934996)	/* LEVEL_num_10_6 */	/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_07_01   (1342741)	/* LEVEL_num_7 */		/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_07_10   (8934997)	/* LEVEL_num_10_7 */	/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_08_01   (1342742)	/* LEVEL_num_8 */		/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_08_10   (8934998)	/* LEVEL_num_10_8 */	/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_09_01   (1342743)	/* LEVEL_num_9 */		/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_09_10   (8934999)	/* LEVEL_num_10_9 */	/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_10_01   (9413138)	/* LEVEL_num_10 */		/*	o	o		o	o	o	o	*/
#define OBJECT_2_LEV_10_10   ( 707105)	/* LEVEL_num_10_10 */	/*	o	o		o	o	o	o	*/
/* ランキング */
#define OBJECT_2_REC_RECORD      ( 8604558)		/* record */
#define OBJECT_2_REC_BAR         (12403282)		/* record_underBar */
#define OBJECT_2_REC_1ST         (   53972)		/* 1st */
#define OBJECT_2_REC_1ST_1       ( 5305814)		/* 1st_num_1 */
#define OBJECT_2_REC_1ST_10      ( 2013946)		/* 1st_num_10 */
#define OBJECT_2_REC_1ST_100     (14114675)		/* 1st_num_100 */
#define OBJECT_2_REC_1ST_1000    (15462058)		/* 1st_num_1000 */
#define OBJECT_2_REC_1ST_10000   ( 8246669)		/* 1st_num_10000 */
#define OBJECT_2_REC_1ST_100000  (12235231)		/* 1st_num_100000 */
#define OBJECT_2_REC_1ST_1000000 ( 5651495)		/* 1st_num_1000000 */
#define OBJECT_2_REC_2ND         (   54820)		/* 2nd */
#define OBJECT_2_REC_2ND_1       ( 5360086)		/* 2nd_num_1 */
#define OBJECT_2_REC_2ND_10      ( 3750650)		/* 2nd_num_10 */
#define OBJECT_2_REC_2ND_100     ( 2580343)		/* 2nd_num_100 */
#define OBJECT_2_REC_2ND_1000    (15462164)		/* 2nd_num_1000 */
#define OBJECT_2_REC_2ND_10000   ( 8250061)		/* 2nd_num_10000 */
#define OBJECT_2_REC_2ND_100000  (12343775)		/* 2nd_num_100000 */
#define OBJECT_2_REC_2ND_1000000 ( 9124903)		/* 2nd_num_1000000 */
#define OBJECT_2_REC_3RD         (   55972)		/* 3rd */
#define OBJECT_2_REC_3RD_1       ( 5433814)		/* 3rd_num_1 */
#define OBJECT_2_REC_3RD_10      ( 6109946)		/* 3rd_num_10 */
#define OBJECT_2_REC_3RD_100     (10968955)		/* 3rd_num_100 */
#define OBJECT_2_REC_3RD_1000    (15462308)		/* 3rd_num_1000 */
#define OBJECT_2_REC_3RD_10000   ( 8254669)		/* 3rd_num_10000 */
#define OBJECT_2_REC_3RD_100000  (12491231)		/* 3rd_num_100000 */
#define OBJECT_2_REC_3RD_1000000 (13843495)		/* 3rd_num_1000000 */

#define OBJECT_2_Y_BUT_CLR_CODE  (12022954)		/* y_button_clr_code */

/*******************************************************************************
 * work
 */

/* ワーク */
typedef struct tagVRSEL_WORK
{
	GV_ACT_EX actor;			/* アクター */

	int       name;				/* シナリオ名 */
	short     phase;			/* フェーズ */
	short     count;			/* カウンター */
	short     flag;				/* フラグ */
	char      player;			/* プレイヤー */
	char      missions;			/* ミッション */
	char      mode;				/* モード */
	char      weapon;			/* 武器 */
	char      level;			/* レベル */
	char      pad1;

	int       proc_player;		/* プレイヤー選択プロック */
	int       proc_stage;		/* ステージ選択プロック */
	int       proc_title;		/* タイトルロードプロック */
	int       proc_save;		/* セーブ用プロック */

	int       proc_keyconfig;	/* キーコンフィグプロック */
	int       keyconsel_set;	/* キーコンフィグ選択の設定値 */
	int       keyconsel_ret;	/* キーコンフィグ選択の返り値 */
	int       vrclc_status;		/* ＶＲクリアコード状態 */

	float     tex1_u0;			/* テクスチャ１のＵ原点 */
	float     tex2_u0;			/* テクスチャ２のＵ原点 */
	short     count2;			/* カウント２ */
	short     pad2;
	SPR_OBJ   *fade;			/* フェード用スプライト */

	int       movie[16];		/* ムービーデータ */

	unsigned int i_color[6];	/* 非アクティブ時の色 */
	unsigned int a_color[6];	/* アクティブ時の色 */

	LAYOUTMAN layoutman_1;		/* レイアウトマネージャ１ */
	LAYOUTMAN layoutman_2;		/* レイアウトマネージャ２ */
}
VRSEL_WORK;

/*******************************************************************************
 * extern
 */

/* ステージリスト */
#define SNEAKING_STAGE_NUM   (10)
#define SNEAKING_WEAPON_NUM  ( 2)
#define WEAPON_STAGE_NUM     ( 5)
#define WEAPON_WEAPON_NUM    ( 8)
#define FPV_STAGE_NUM        ( 5)
#define VARIETY_STAGE_NUM    ( 7)
#define BOMB_STAGE_NUM       ( 5)
#define ELIMINATE_STAGE_NUM  (10)
#define HOLDUP_STAGE_NUM     (10)
#define PHOTOGRAPH_STAGE_NUM ( 7)

extern char _sneaking_stage[SNEAKING_WEAPON_NUM][SNEAKING_STAGE_NUM][VRSEL_PLA_MAX];
extern char _weapon_stage[WEAPON_WEAPON_NUM][WEAPON_STAGE_NUM][VRSEL_PLA_MAX];
extern char _fpv_stage[FPV_STAGE_NUM][VRSEL_PLA_MAX];
extern char _variety_stage[VARIETY_STAGE_NUM][VRSEL_PLA_MAX];
extern char _streaking_stage;
extern char _bomb_stage[BOMB_STAGE_NUM][VRSEL_PLA_MAX];
extern char _eliminate_stage[ELIMINATE_STAGE_NUM][VRSEL_PLA_MAX];
extern char _holdup_stage[HOLDUP_STAGE_NUM][VRSEL_PLA_MAX];
extern char _photograph_stage[PHOTOGRAPH_STAGE_NUM][VRSEL_PLA_MAX];

/* デバッグモード */
#ifdef DEBUG_MODE

#define VRSEL_MAX_PAGES       (5)
#define VRSEL_PAGE0_MAX_ITEMS (4)
#define VRSEL_PAGE1_MAX_ITEMS (2)
#define VRSEL_PAGE2_MAX_ITEMS (8)
#define VRSEL_PAGE3_MAX_ITEMS (3)
#define VRSEL_PAGE4_MAX_ITEMS (4)

#define VRSEL_TEXT_BUF_X (768)
#define VRSEL_TEXT_BUF_Y (768)

#define TEXT_X(c) (VRSEL_TEXT_BUF_X/32 * (c) + 1)
#define TEXT_W    (VRSEL_TEXT_BUF_X/32 - 2)
#define TEXT_Y(r) (VRSEL_TEXT_BUF_Y/24 * (r) + 1)
#define TEXT_H    (VRSEL_TEXT_BUF_Y/24 - 2)

extern void    *_text_work;		/* デバッグテキスト ワーク */
extern SPR_OBJ *_background;	/* 背景スプライト */
extern char    _page;			/* デバッグテキスト ページ */
extern char    _item;			/* デバッグテキスト 項目 */
#endif

/*******************************************************************************
 * functions
 */

/* vr_select.c */
int VRSEL_GetStageID(VRSEL_WORK *work);
int VRSEL_SneakingClearLevel2(VRSEL_WORK *work, int weapon, int player, int flag);
int VRSEL_WeaponClearLevel2(VRSEL_WORK *work, int weapon, int player, int flag);
int VRSEL_SneakingClearLevel(VRSEL_WORK *work, int player, int flag);
int VRSEL_WeaponClearLevel(VRSEL_WORK *work, int player, int flag);
int VRSEL_FPVClearLevel(VRSEL_WORK *work, int player, int flag);
int VRSEL_VarietyClearLevel(VRSEL_WORK *work, int player, int flag);
int VRSEL_StreakingClearLevel(VRSEL_WORK *work, int player, int flag);
int VRSEL_BombClearLevel(VRSEL_WORK *work, int player, int flag);
int VRSEL_EliminateClearLevel(VRSEL_WORK *work, int player, int flag);
int VRSEL_HoldupClearLevel(VRSEL_WORK *work, int player, int flag);
int VRSEL_PhotographClearLevel(VRSEL_WORK *work, int player, int flag);
int VRSEL_VRClearLevel(VRSEL_WORK *work, int player, int flag);
int VRSEL_AlternativeClearLevel(VRSEL_WORK *work, int player, int flag);
int VRSEL_PlayerClearLevel(VRSEL_WORK *work, int player, int flag);
int VRSEL_GetClearPercentage(VRSEL_WORK *work, int player);
void VRSEL_SetStageList(VRSEL_WORK *work, int type);

/* vr_select_act.c */
void VRSEL_ShowSprite(LAYOUTMAN *layoutman, int strcode, int show);
void VRSEL_InputOK(VRSEL_WORK *work);
void VRSEL_InputNG(VRSEL_WORK *work);
void VRSEL_InputStop(VRSEL_WORK *work, short stop);
void VRSEL_InputStop2(VRSEL_WORK *work, short stop);
void VRSEL_SetPhase(VRSEL_WORK *work, int phase);
int  VRSEL_LOM_SetCurMode(VRSEL_WORK *work, LAYOUTMAN *layoutman, int lom_mode_name, int default_act, int default_ico, int default_obj);
void VRSEL_Act(VRSEL_WORK *work);

void VRSEL_L2_SetCursorPos(VRSEL_WORK *work, int stages);
int  VRSEL_LOMCallback_1(void *pWork, int param);
int  VRSEL_LOMCallback_2(void *pWork, int param);

void VRSEL_Dbg_L1(VRSEL_WORK *work);
void VRSEL_Dbg_R1(VRSEL_WORK *work);
void VRSEL_Dbg_L2(VRSEL_WORK *work);
void VRSEL_Dbg_R2(VRSEL_WORK *work);
void VRSEL_Dbg_L3(VRSEL_WORK *work);
void VRSEL_Dbg_R3(VRSEL_WORK *work);
void VRSEL_Dbg_Act(VRSEL_WORK *work);

int VRSEL_IsRaidenPlayable  (VRSEL_WORK *work);
int VRSEL_IsNinjaPlayable   (VRSEL_WORK *work);
int VRSEL_IsXRaidenPlayable (VRSEL_WORK *work);
int VRSEL_IsSnakePlayable   (VRSEL_WORK *work);
int VRSEL_IsPliskinPlayable (VRSEL_WORK *work);
int VRSEL_IsTuxedoPlayable  (VRSEL_WORK *work);
int VRSEL_IsPreviousPlayable(VRSEL_WORK *work);
int VRSEL_IsSavePlayable    (VRSEL_WORK *work);
int VRSEL_IsExitPlayable    (VRSEL_WORK *work);
int VRSEL_Pla_MoveUp  (VRSEL_WORK *work, int pos);
int VRSEL_Pla_MoveDown(VRSEL_WORK *work, int pos);
int VRSEL_Pla_Select_1(VRSEL_WORK *work, int player);
int VRSEL_Pla_Select_2(VRSEL_WORK *work);
int VRSEL_Pla_Select_3(VRSEL_WORK *work);
int VRSEL_Pla_Cancel  (VRSEL_WORK *work);

int VRSEL_IsVRPlayable         (VRSEL_WORK *work);
int VRSEL_IsAlternativePlayable(VRSEL_WORK *work);
int VRSEL_Mis_Move_Up          (VRSEL_WORK *work, int pos);
int VRSEL_Mis_Move_Down        (VRSEL_WORK *work, int pos);
int VRSEL_Mis_Select           (VRSEL_WORK *work, int missions);
int VRSEL_Mis_Cancel           (VRSEL_WORK *work);

int VRSEL_SetMovie(VRSEL_WORK *work, int movie_index);
int VRSEL_SetMovie2(VRSEL_WORK *work, int movie_index);
int VRSEL_IsSneakingPlayable       (VRSEL_WORK *work);
int VRSEL_IsWeaponPlayable         (VRSEL_WORK *work);
int VRSEL_IsFirstPersonViewPlayable(VRSEL_WORK *work);
int VRSEL_IsVarietyPlayable        (VRSEL_WORK *work);
int VRSEL_IsStreakingPlayable      (VRSEL_WORK *work);
int VRSEL_IsBombDisposalPlayable   (VRSEL_WORK *work);
int VRSEL_IsEliminatePlayable      (VRSEL_WORK *work);
int VRSEL_IsHoldUpPlayable         (VRSEL_WORK *work);
int VRSEL_IsPhotographPlayable     (VRSEL_WORK *work);
int VRSEL_Mod_MoveUp_Vr   (VRSEL_WORK *work, int pos);
int VRSEL_Mod_MoveDown_Vr (VRSEL_WORK *work, int pos);
int VRSEL_Mod_MoveUp_Alt  (VRSEL_WORK *work, int pos);
int VRSEL_Mod_MoveDown_Alt(VRSEL_WORK *work, int pos);
int VRSEL_Mod_Select      (VRSEL_WORK *work, int mode);
int VRSEL_Mod_Cancel_1    (VRSEL_WORK *work);
int VRSEL_Mod_Cancel_2    (VRSEL_WORK *work);

int VRSEL_IsSneSneakingPlayable    (VRSEL_WORK *work);
int VRSEL_IsSneEliminateAllPlayable(VRSEL_WORK *work);
int VRSEL_IsWeaHandgunPlayable     (VRSEL_WORK *work);
int VRSEL_IsWeaAssaultRiflePlayable(VRSEL_WORK *work);
int VRSEL_IsWeaC4ClaymorePlayable  (VRSEL_WORK *work);
int VRSEL_IsWeaGrenadePlayable     (VRSEL_WORK *work);
int VRSEL_IsWeaPsg1Playable        (VRSEL_WORK *work);
int VRSEL_IsWeaStingerPlayable     (VRSEL_WORK *work);
int VRSEL_IsWeaNikitaPlayable      (VRSEL_WORK *work);
int VRSEL_IsWeaHfBladePlayable     (VRSEL_WORK *work);
int VRSEL_Wea_MoveUp_Sne  (VRSEL_WORK *work, int pos);
int VRSEL_Wea_MoveDown_Sne(VRSEL_WORK *work, int pos);
int VRSEL_Wea_MoveUp_Wea  (VRSEL_WORK *work, int pos);
int VRSEL_Wea_MoveDown_Wea(VRSEL_WORK *work, int pos);
int VRSEL_Wea_Select      (VRSEL_WORK *work, int weapon);
int VRSEL_Wea_Cancel_1    (VRSEL_WORK *work);
int VRSEL_Wea_Cancel_2    (VRSEL_WORK *work);

int VRSEL_IsLevel01Playable(VRSEL_WORK *work);
int VRSEL_IsLevel02Playable(VRSEL_WORK *work);
int VRSEL_IsLevel03Playable(VRSEL_WORK *work);
int VRSEL_IsLevel04Playable(VRSEL_WORK *work);
int VRSEL_IsLevel05Playable(VRSEL_WORK *work);
int VRSEL_IsLevel06Playable(VRSEL_WORK *work);
int VRSEL_IsLevel07Playable(VRSEL_WORK *work);
int VRSEL_IsLevel08Playable(VRSEL_WORK *work);
int VRSEL_IsLevel09Playable(VRSEL_WORK *work);
int VRSEL_IsLevel10Playable(VRSEL_WORK *work);
int VRSEL_IsControlPlayable(VRSEL_WORK *work);
int VRSEL_Lev_MoveUp  (VRSEL_WORK *work, int pos);
int VRSEL_Lev_MoveDown(VRSEL_WORK *work, int pos);
int VRSEL_Lev_Select_1(VRSEL_WORK *work, int level);
int VRSEL_Lev_Select_2(VRSEL_WORK *work);
int VRSEL_Lev_Select_3(VRSEL_WORK *work);
int VRSEL_Lev_Cancel(VRSEL_WORK *work);

/*******************************************************************************
 */

#endif	/* __INC_VR_SELECT__ */
