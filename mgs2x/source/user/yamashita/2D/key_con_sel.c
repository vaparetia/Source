//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * key_con_sel - key_con_sel.c
 * キーコンフィグ選択  *NewKeyConSel
 * 2002/07/09 S.Yamashita
 * $Id: key_con_sel.c,v 1.4 2002/11/23 12:24:51 Yoshizawa1 Exp $
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

#define __CHARA_NAME__ "Key Config Select"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
//#define SY_PRINTF3_DEBUG
#include "../sy_util/sy_util.h"

/*******************************************************************************
 * defines
 */

/* コールバックタイプ */
enum {
	KCS_CB_JPN_1_L = 1,
	KCS_CB_JPN_1_R,
	KCS_CB_JPN_2_L,
	KCS_CB_JPN_2_R,
	KCS_CB_JPN_3_L,
	KCS_CB_JPN_3_R,
	KCS_CB_JPN_4_L,
	KCS_CB_JPN_4_R,

	KCS_CB_JPN_1_OK,
	KCS_CB_JPN_2_OK,
	KCS_CB_JPN_3_OK,
	KCS_CB_JPN_4_OK,

	KCS_CB_ENG_1_L,
	KCS_CB_ENG_1_R,
	KCS_CB_ENG_2_L,
	KCS_CB_ENG_2_R,
	KCS_CB_ENG_3_L,
	KCS_CB_ENG_3_R,
	KCS_CB_ENG_4_L,
	KCS_CB_ENG_4_R,

	KCS_CB_ENG_1_OK,
	KCS_CB_ENG_2_OK,
	KCS_CB_ENG_3_OK,
	KCS_CB_ENG_4_OK,

	KCS_CB_ENG2_1_L,
	KCS_CB_ENG2_1_R,
	KCS_CB_ENG2_2_L,
	KCS_CB_ENG2_2_R,
	KCS_CB_ENG2_3_L,
	KCS_CB_ENG2_3_R,
	KCS_CB_ENG2_4_L,
	KCS_CB_ENG2_4_R,

	KCS_CB_ENG2_1_OK,
	KCS_CB_ENG2_2_OK,
	KCS_CB_ENG2_3_OK,
	KCS_CB_ENG2_4_OK,

	KCS_CB_CANCELED,
	KCS_CB_CLOSED,
};

/* フラグ */
#define KCS_INIT     (0x0001)	/* 初期化時 */
#define KCS_INPUT_OK (0x0002)	/* 入力ＯＫ */
#define KCS_JAPANESE (0x0004)	/* 日本語モード */
#define KCS_FADE_IN  (0x0010)	/* フェードイン */
#define KCS_FADE_OUT (0x0020)	/* フェードアウト */
#define KCS_CANCELED (0x0040)	/* キャンセル */
#define KCS_DEKACON  (0x0080)	/* デカコントローラ */
#define KCS_INIT2    (0x0100)	/* 初期化時２ */

/* 設定値 */
#define KCS_FADE_COUNT     (25)		/* フェードカウント */
#define KCS_INTERVAL_COUNT (25)		/* インターバルカウント */

/* Ｌ２Ｄオブジェクト */
#ifdef KP_XBOX
#define L2D_FILENAME         (10302309)		/* fpv_controls */
#else
#define L2D_FILENAME         (  654824)		/* fpv_controls_eng_ps2 */
#endif
#define L2D_FILENAME_JPN     ( 7995180)		/* fpv_controls_jpn_ps2 */
#define L2D_FILENAME_2       ( 4486397)		/* option_controls */
#ifdef KP_XBOX
#define L2D_FILENAME_ENG     ( 9106781)		/* fpv_controls_eng */
#define L2D_FILENAME_FRA     ( 9107940)		/* fpv_controls_frn */
#define L2D_FILENAME_GER     ( 9108963)		/* fpv_controls_grm */
#define L2D_FILENAME_ITA     ( 9111074)		/* fpv_controls_itl */
#define L2D_FILENAME_SPA     ( 9121188)		/* fpv_controls_spn */
#else
#define L2D_FILENAME_ENG     (  654824)		/* fpv_controls_eng_ps2 */
#define L2D_FILENAME_FRA     ( 7994928)		/* fpv_controls_frn_ps2 */
#define L2D_FILENAME_GER     ( 6946416)		/* fpv_controls_grm_ps2 */
#define L2D_FILENAME_ITA     ( 5897972)		/* fpv_controls_itl_ps2 */
#define L2D_FILENAME_SPA     ( 7995756)		/* fpv_controls_spn_ps2 */
#endif
#define L2D_FILENAME_ENG_2   (  354679)		/* option_controls_eng */
#define L2D_FILENAME_FRA_2   (  355838)		/* option_controls_frn */
#define L2D_FILENAME_GER_2   (  356861)		/* option_controls_grm */
#define L2D_FILENAME_ITA_2   (  358972)		/* option_controls_itl */
#define L2D_FILENAME_SPA_2   (  369086)		/* option_controls_spn */

#define ACTION_DUMMY       ( 8143391)	/* dummy */
#define ACTION_DUMMY2      ( 8930337)	/* dummy2 */
#define ACTION_JPN_1_2     ( 1076089)	/* jpn_1_2 */
#define ACTION_JPN_2_3     ( 1077114)	/* jpn_2_3 */
#define ACTION_JPN_3_4     ( 1078139)	/* jpn_3_4 */
#define ACTION_JPN_2_1     ( 1077112)	/* jpn_2_1 */
#define ACTION_JPN_3_2     ( 1078137)	/* jpn_3_2 */
#define ACTION_JPN_4_3     ( 1079162)	/* jpn_4_3 */
#define ACTION_ENG_1_2     (10512948)	/* eng_1_2 */
#define ACTION_ENG_2_3     (10513973)	/* eng_2_3 */
#define ACTION_ENG_3_4     (10514998)	/* eng_3_4 */
#define ACTION_ENG_2_1     (10513971)	/* eng_2_1 */
#define ACTION_ENG_3_2     (10514996)	/* eng_3_2 */
#define ACTION_ENG_4_3     (10516021)	/* eng_4_3 */
#define ACTION_ENG2_1_2    ( 7444855)	/* engBic_1_2 */
#define ACTION_ENG2_2_3    ( 7445880)	/* engBic_2_3 */
#define ACTION_ENG2_3_4    ( 7446905)	/* engBic_3_4 */
#define ACTION_ENG2_2_1    ( 7445878)	/* engBic_2_1 */
#define ACTION_ENG2_3_2    ( 7446903)	/* engBic_3_2 */
#define ACTION_ENG2_4_3    ( 7447928)	/* engBic_4_3 */

#define OBJECT_RIGHT       ( 5646715)	/* right */
#define OBJECT_LEFT        ( 3645748)	/* left */
#define OBJECT_JPN_1       ( 4010910)	/* controls1_jpn */
#define OBJECT_JPN_2       ( 5059486)	/* controls2_jpn */
#define OBJECT_JPN_3       ( 6108062)	/* controls3_jpn */
#define OBJECT_JPN_4       ( 7156638)	/* controls4_jpn */
#define OBJECT_ENG_1       ( 4005719)	/* controls1_eng */
#define OBJECT_ENG_2       ( 5054295)	/* controls2_eng */
#define OBJECT_ENG_3       ( 6102871)	/* controls3_eng */
#define OBJECT_ENG_4       ( 7151447)	/* controls4_eng */
#define OBJECT_ENG2_1      (10742553)	/* controls1_eng2 */
#define OBJECT_ENG2_2      (10742555)	/* controls2_eng2 */
#define OBJECT_ENG2_3      (10742557)	/* controls3_eng2 */
#define OBJECT_ENG2_4      (10742559)	/* controls4_eng2 */
#define OBJECT_ABBUTTON    (  853730)	/* A_B_button */
#define OBJECT_DEKACON_1   (16410168)	/* controls_a */
#define OBJECT_DEKACON_2   (16410169)	/* controls_b */
#define OBJECT_DEKACON_3   (16410170)	/* controls_c */
#define OBJECT_DEKACON_4   (16410171)	/* controls_d */

/*******************************************************************************
 * work
 */

/* ワーク */
typedef struct tagWORK
{
	GV_ACT_EX actor;		/* アクター */
	LAYOUTMAN layoutman;	/* レイアウトマネージャ */
	int       count;		/* カウント */
	int       page;			/* ページ */
	int       *ret;			/* 結果 */
	short     flag;			/* フラグ */
	short     t_pages;		/* 合計ページ数 */
}
WORK;

/*******************************************************************************
 * local
 */

/* ＬＯＭデータ */
static int _lom_data[] = {
					/*アクションリスト名*/		/*アクション名*/		/*カウント*/	/*フラグ*/
LOM_ACTLIST_DATA,	LOM_ACTLIST_00,				ACTION_DUMMY,			1,				LOM_ACT_FRAME|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_01,				ACTION_JPN_1_2,			1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_02,				ACTION_JPN_2_3,			1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_03,				ACTION_JPN_3_4,			1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_04,				ACTION_JPN_2_1,			1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_05,				ACTION_JPN_3_2,			1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_06,				ACTION_JPN_4_3,			1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_11,				ACTION_ENG_1_2,			1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_12,				ACTION_ENG_2_3,			1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_13,				ACTION_ENG_3_4,			1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_14,				ACTION_ENG_2_1,			1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_15,				ACTION_ENG_3_2,			1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_16,				ACTION_ENG_4_3,			1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_31,				ACTION_ENG2_1_2,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_32,				ACTION_ENG2_2_3,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_33,				ACTION_ENG2_3_4,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_34,				ACTION_ENG2_2_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_35,				ACTION_ENG2_3_2,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_36,				ACTION_ENG2_4_3,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_20,				ACTION_DUMMY2,			KCS_FADE_COUNT,	LOM_ACT_FRAME|LOM_ACT_SHOW,
												ACTION_DUMMY,			1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												ACTION_DUMMY2,			1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_DATA_END,

					/*アイコンリスト名*/		/* アイコン名 */	/* テクスチャ名 */	/* 位置(左、上、右、下) */	/* 色 */		/* カウント */	/* フラグ */
LOM_ICOLIST_DATA,	LOM_ICOLIST_00,				LOM_ICO_00,			LOM_ICO_NOTEX,		0, 0, 0, 0,					0, 0, 0, 0,		LOM_ICO_LOOP,	LOM_ICO_ALPHA,
												LOM_DATA_END,
					LOM_DATA_END,

					/* オブジェクトリスト名 */	/* オブジェクト名 */	/* アイコンリスト名 */	/* カウント */	/* フラグ */
LOM_OBJLIST_DATA,	LOM_OBJLIST_00,				OBJECT_JPN_1,			LOM_ICOLIST_00,			1,				LOM_OBJ_INTERP_1,
												OBJECT_JPN_2,			LOM_ICOLIST_00,			1,				LOM_OBJ_INTERP_1,
												OBJECT_JPN_3,			LOM_ICOLIST_00,			1,				LOM_OBJ_INTERP_1,
												OBJECT_JPN_4,			LOM_ICOLIST_00,			1,				LOM_OBJ_INTERP_1,
												LOM_DATA_END,
					LOM_OBJLIST_01,				OBJECT_ENG_1,			LOM_ICOLIST_00,			1,				LOM_OBJ_INTERP_1,
												OBJECT_ENG_2,			LOM_ICOLIST_00,			1,				LOM_OBJ_INTERP_1,
												OBJECT_ENG_3,			LOM_ICOLIST_00,			1,				LOM_OBJ_INTERP_1,
												OBJECT_ENG_4,			LOM_ICOLIST_00,			1,				LOM_OBJ_INTERP_1,
												LOM_DATA_END,
					LOM_OBJLIST_03,				OBJECT_ENG2_1,			LOM_ICOLIST_00,			1,				LOM_OBJ_INTERP_1,
												OBJECT_ENG2_2,			LOM_ICOLIST_00,			1,				LOM_OBJ_INTERP_1,
												OBJECT_ENG2_3,			LOM_ICOLIST_00,			1,				LOM_OBJ_INTERP_1,
												OBJECT_ENG2_4,			LOM_ICOLIST_00,			1,				LOM_OBJ_INTERP_1,
												LOM_DATA_END,
					LOM_DATA_END,

					/*パッドリスト名*/			/*アクション名*/		/*オブジェクト名*/	/*入力*/			/*実行番号*/		/*実行引数*/			/*フラグ*/
LOM_PADLIST_DATA,	LOM_PADLIST_00,				LOM_PAD_ALLACT,			OBJECT_JPN_1,		PAD_L,				LOM_EXE_CALLBACK,	KCS_CB_JPN_1_L,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_JPN_1,		PAD_R,				LOM_EXE_CALLBACK,	KCS_CB_JPN_1_R,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_JPN_2,		PAD_L,				LOM_EXE_CALLBACK,	KCS_CB_JPN_2_L,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_JPN_2,		PAD_R,				LOM_EXE_CALLBACK,	KCS_CB_JPN_2_R,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_JPN_3,		PAD_L,				LOM_EXE_CALLBACK,	KCS_CB_JPN_3_L,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_JPN_3,		PAD_R,				LOM_EXE_CALLBACK,	KCS_CB_JPN_3_R,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_JPN_4,		PAD_L,				LOM_EXE_CALLBACK,	KCS_CB_JPN_4_L,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_JPN_4,		PAD_R,				LOM_EXE_CALLBACK,	KCS_CB_JPN_4_R,		LOM_PAD_ON_SIG,

												LOM_PAD_ALLACT,			OBJECT_JPN_1,		LOM_PAD_OK,				LOM_EXE_CALLBACK,	KCS_CB_JPN_1_OK,	LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_JPN_2,		LOM_PAD_OK,				LOM_EXE_CALLBACK,	KCS_CB_JPN_2_OK,	LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_JPN_3,		LOM_PAD_OK,				LOM_EXE_CALLBACK,	KCS_CB_JPN_3_OK,	LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_JPN_4,		LOM_PAD_OK,				LOM_EXE_CALLBACK,	KCS_CB_JPN_4_OK,	LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			LOM_PAD_ALLOBJ,		LOM_PAD_CANCEL,			LOM_EXE_CALLBACK,	KCS_CB_CANCELED,	LOM_PAD_PRESS,
												LOM_DATA_END,

					LOM_PADLIST_01,				LOM_PAD_ALLACT,			OBJECT_ENG_1,		PAD_L,				LOM_EXE_CALLBACK,	KCS_CB_ENG_1_L,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ENG_1,		PAD_R,				LOM_EXE_CALLBACK,	KCS_CB_ENG_1_R,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ENG_2,		PAD_L,				LOM_EXE_CALLBACK,	KCS_CB_ENG_2_L,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ENG_2,		PAD_R,				LOM_EXE_CALLBACK,	KCS_CB_ENG_2_R,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ENG_3,		PAD_L,				LOM_EXE_CALLBACK,	KCS_CB_ENG_3_L,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ENG_3,		PAD_R,				LOM_EXE_CALLBACK,	KCS_CB_ENG_3_R,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ENG_4,		PAD_L,				LOM_EXE_CALLBACK,	KCS_CB_ENG_4_L,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ENG_4,		PAD_R,				LOM_EXE_CALLBACK,	KCS_CB_ENG_4_R,		LOM_PAD_ON_SIG,

												LOM_PAD_ALLACT,			OBJECT_ENG_1,		LOM_PAD_OK,				LOM_EXE_CALLBACK,	KCS_CB_ENG_1_OK,	LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ENG_2,		LOM_PAD_OK,				LOM_EXE_CALLBACK,	KCS_CB_ENG_2_OK,	LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ENG_3,		LOM_PAD_OK,				LOM_EXE_CALLBACK,	KCS_CB_ENG_3_OK,	LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ENG_4,		LOM_PAD_OK,				LOM_EXE_CALLBACK,	KCS_CB_ENG_4_OK,	LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			LOM_PAD_ALLOBJ,		LOM_PAD_CANCEL,			LOM_EXE_CALLBACK,	KCS_CB_CANCELED,	LOM_PAD_PRESS,
												LOM_DATA_END,

					LOM_PADLIST_03,				LOM_PAD_ALLACT,			OBJECT_ENG2_1,		PAD_L,				LOM_EXE_CALLBACK,	KCS_CB_ENG2_1_L,	LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ENG2_1,		PAD_R,				LOM_EXE_CALLBACK,	KCS_CB_ENG2_1_R,	LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ENG2_2,		PAD_L,				LOM_EXE_CALLBACK,	KCS_CB_ENG2_2_L,	LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ENG2_2,		PAD_R,				LOM_EXE_CALLBACK,	KCS_CB_ENG2_2_R,	LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ENG2_3,		PAD_L,				LOM_EXE_CALLBACK,	KCS_CB_ENG2_3_L,	LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ENG2_3,		PAD_R,				LOM_EXE_CALLBACK,	KCS_CB_ENG2_3_R,	LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ENG2_4,		PAD_L,				LOM_EXE_CALLBACK,	KCS_CB_ENG2_4_L,	LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ENG2_4,		PAD_R,				LOM_EXE_CALLBACK,	KCS_CB_ENG2_4_R,	LOM_PAD_ON_SIG,

												LOM_PAD_ALLACT,			OBJECT_ENG2_1,		LOM_PAD_OK,				LOM_EXE_CALLBACK,	KCS_CB_ENG2_1_OK,	LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ENG2_2,		LOM_PAD_OK,				LOM_EXE_CALLBACK,	KCS_CB_ENG2_2_OK,	LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ENG2_3,		LOM_PAD_OK,				LOM_EXE_CALLBACK,	KCS_CB_ENG2_3_OK,	LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ENG2_4,		LOM_PAD_OK,				LOM_EXE_CALLBACK,	KCS_CB_ENG2_4_OK,	LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			LOM_PAD_ALLOBJ,		LOM_PAD_CANCEL,			LOM_EXE_CALLBACK,	KCS_CB_CANCELED,	LOM_PAD_PRESS,
												LOM_DATA_END,
					/* 閉じた後用 */
					LOM_PADLIST_02,				ACTION_DUMMY,			LOM_PAD_ALLOBJ,		LOM_PAD_ANYINPUT,	LOM_EXE_CALLBACK,	KCS_CB_CLOSED,		0,
												LOM_DATA_END,
					LOM_DATA_END,

					/*モード名*/	/*ＬＯＭアクションリスト*/	/*ＬＯＭオブジェクトリスト*/	/*ＬＯＭパッドリスト*/
LOM_MODE_DATA,		LOM_MODE_20,	LOM_ACTLIST_00,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* init */
					LOM_MODE_21,	LOM_ACTLIST_00,				LOM_OBJLIST_01,					LOM_PADLIST_01,		/* init */
					LOM_MODE_22,	LOM_ACTLIST_20,				LOM_MODE_NOLIST,				LOM_PADLIST_02,		/* end */
					LOM_MODE_23,	LOM_ACTLIST_00,				LOM_OBJLIST_03,					LOM_PADLIST_03,		/* init */

					LOM_MODE_01,	LOM_ACTLIST_01,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* jpn_1_2 */
					LOM_MODE_02,	LOM_ACTLIST_02,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* jpn_2_3 */
					LOM_MODE_03,	LOM_ACTLIST_03,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* jpn_3_4 */
					LOM_MODE_04,	LOM_ACTLIST_04,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* jpn_2_1 */
					LOM_MODE_05,	LOM_ACTLIST_05,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* jpn_3_2 */
					LOM_MODE_06,	LOM_ACTLIST_06,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* jpn_4_3 */

					LOM_MODE_11,	LOM_ACTLIST_11,				LOM_OBJLIST_01,					LOM_PADLIST_01,		/* eng_1_2 */
					LOM_MODE_12,	LOM_ACTLIST_12,				LOM_OBJLIST_01,					LOM_PADLIST_01,		/* eng_2_3 */
					LOM_MODE_13,	LOM_ACTLIST_13,				LOM_OBJLIST_01,					LOM_PADLIST_01,		/* eng_3_4 */
					LOM_MODE_14,	LOM_ACTLIST_14,				LOM_OBJLIST_01,					LOM_PADLIST_01,		/* eng_2_1 */
					LOM_MODE_15,	LOM_ACTLIST_15,				LOM_OBJLIST_01,					LOM_PADLIST_01,		/* eng_3_2 */
					LOM_MODE_16,	LOM_ACTLIST_16,				LOM_OBJLIST_01,					LOM_PADLIST_01,		/* eng_4_3 */

					LOM_MODE_31,	LOM_ACTLIST_31,				LOM_OBJLIST_03,					LOM_PADLIST_03,		/* engBic_1_2 */
					LOM_MODE_32,	LOM_ACTLIST_32,				LOM_OBJLIST_03,					LOM_PADLIST_03,		/* engBic_2_3 */
					LOM_MODE_33,	LOM_ACTLIST_33,				LOM_OBJLIST_03,					LOM_PADLIST_03,		/* engBic_3_4 */
					LOM_MODE_34,	LOM_ACTLIST_34,				LOM_OBJLIST_03,					LOM_PADLIST_03,		/* engBic_2_1 */
					LOM_MODE_35,	LOM_ACTLIST_35,				LOM_OBJLIST_03,					LOM_PADLIST_03,		/* engBic_3_2 */
					LOM_MODE_36,	LOM_ACTLIST_36,				LOM_OBJLIST_03,					LOM_PADLIST_03,		/* engBic_4_3 */
					LOM_DATA_END,
LOM_DATA_END,
};

/*******************************************************************************
 * static
 */
/*******************************************************************************
 * スプライトの表示設定
 */
static void ShowSprite(
	LAYOUTMAN     *layoutman,	/* レイアウトマネージャ */
	int           strcode,		/* 文字列コード */
	int           show)			/* 表示フラグ */
{
	int flags;

	if(show == 0)
	{
		LOM_Spr_GetFlags(layoutman, strcode, &flags); flags |= SPR_FLAG_HIDDEN;
		LOM_Spr_SetFlags(layoutman, strcode,  flags);
	}
	else
	{
		LOM_Spr_GetFlags(layoutman, strcode, &flags); flags &= ~SPR_FLAG_HIDDEN;
		LOM_Spr_SetFlags(layoutman, strcode,  flags);
	}
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

	if(param != KCS_CB_CLOSED)
	{
		if(!(work->flag & KCS_INPUT_OK))
			return 0;
	}

	switch(param)
	{
	case KCS_CB_JPN_1_L:	return 0;
	case KCS_CB_JPN_1_R:	LOM_SetCurMode(&work->layoutman, LOM_MODE_01, 1, 1, 1); LOM_SetCurObj(&work->layoutman, OBJECT_JPN_2, 1); work->page = 2; GM_SdSet(SD_S_TYPING03); break;
	case KCS_CB_JPN_2_L:	LOM_SetCurMode(&work->layoutman, LOM_MODE_04, 1, 1, 1); LOM_SetCurObj(&work->layoutman, OBJECT_JPN_1, 1); work->page = 1; GM_SdSet(SD_S_TYPING03); break;
	case KCS_CB_JPN_2_R:	if(work->t_pages > 2)
							{
							LOM_SetCurMode(&work->layoutman, LOM_MODE_02, 1, 1, 1); LOM_SetCurObj(&work->layoutman, OBJECT_JPN_3, 1); work->page = 3; GM_SdSet(SD_S_TYPING03); break;
							}
							else
							return 0;
	case KCS_CB_JPN_3_L:	LOM_SetCurMode(&work->layoutman, LOM_MODE_05, 1, 1, 1); LOM_SetCurObj(&work->layoutman, OBJECT_JPN_2, 1); work->page = 2; GM_SdSet(SD_S_TYPING03); break;
	case KCS_CB_JPN_3_R:	if(work->t_pages > 3)
							{
							LOM_SetCurMode(&work->layoutman, LOM_MODE_03, 1, 1, 1); LOM_SetCurObj(&work->layoutman, OBJECT_JPN_4, 1); work->page = 4; GM_SdSet(SD_S_TYPING03); break;
							}
							else
							return 0;
	case KCS_CB_JPN_4_L:	LOM_SetCurMode(&work->layoutman, LOM_MODE_06, 1, 1, 1); LOM_SetCurObj(&work->layoutman, OBJECT_JPN_3, 1); work->page = 3; GM_SdSet(SD_S_TYPING03); break;
	case KCS_CB_JPN_4_R:	return 0;



	case KCS_CB_ENG_1_L:	return 0;
	case KCS_CB_ENG_1_R:	LOM_SetCurMode(&work->layoutman, LOM_MODE_11, 1, 1, 1); LOM_SetCurObj(&work->layoutman, OBJECT_ENG_2, 1); work->page = 2; GM_SdSet(SD_S_TYPING03); break;
	case KCS_CB_ENG_2_L:	LOM_SetCurMode(&work->layoutman, LOM_MODE_14, 1, 1, 1); LOM_SetCurObj(&work->layoutman, OBJECT_ENG_1, 1); work->page = 1; GM_SdSet(SD_S_TYPING03); break;
	case KCS_CB_ENG_2_R:	if(work->t_pages > 2)
							{
							LOM_SetCurMode(&work->layoutman, LOM_MODE_12, 1, 1, 1); LOM_SetCurObj(&work->layoutman, OBJECT_ENG_3, 1); work->page = 3; GM_SdSet(SD_S_TYPING03); break;
							}
							else
							return 0;
	case KCS_CB_ENG_3_L:	LOM_SetCurMode(&work->layoutman, LOM_MODE_15, 1, 1, 1); LOM_SetCurObj(&work->layoutman, OBJECT_ENG_2, 1); work->page = 2; GM_SdSet(SD_S_TYPING03); break;
	case KCS_CB_ENG_3_R:	if(work->t_pages > 3)
							{
							LOM_SetCurMode(&work->layoutman, LOM_MODE_13, 1, 1, 1); LOM_SetCurObj(&work->layoutman, OBJECT_ENG_4, 1); work->page = 4; GM_SdSet(SD_S_TYPING03); break;
							}
							else
							return 0;
	case KCS_CB_ENG_4_L:	LOM_SetCurMode(&work->layoutman, LOM_MODE_16, 1, 1, 1); LOM_SetCurObj(&work->layoutman, OBJECT_ENG_3, 1); work->page = 3; GM_SdSet(SD_S_TYPING03); break;
	case KCS_CB_ENG_4_R:	return 0;



	case KCS_CB_ENG2_1_L:	return 0;
	case KCS_CB_ENG2_1_R:	LOM_SetCurMode(&work->layoutman, LOM_MODE_31, 1, 1, 1); LOM_SetCurObj(&work->layoutman, OBJECT_ENG2_2, 1); work->page = 2; GM_SdSet(SD_S_TYPING03); break;
	case KCS_CB_ENG2_2_L:	LOM_SetCurMode(&work->layoutman, LOM_MODE_34, 1, 1, 1); LOM_SetCurObj(&work->layoutman, OBJECT_ENG2_1, 1); work->page = 1; GM_SdSet(SD_S_TYPING03); break;
	case KCS_CB_ENG2_2_R:	if(work->t_pages > 2)
							{
							LOM_SetCurMode(&work->layoutman, LOM_MODE_32, 1, 1, 1); LOM_SetCurObj(&work->layoutman, OBJECT_ENG2_3, 1); work->page = 3; GM_SdSet(SD_S_TYPING03); break;
							}
							else
							return 0;
	case KCS_CB_ENG2_3_L:	LOM_SetCurMode(&work->layoutman, LOM_MODE_35, 1, 1, 1); LOM_SetCurObj(&work->layoutman, OBJECT_ENG2_2, 1); work->page = 2; GM_SdSet(SD_S_TYPING03); break;
	case KCS_CB_ENG2_3_R:	if(work->t_pages > 3)
							{
							LOM_SetCurMode(&work->layoutman, LOM_MODE_33, 1, 1, 1); LOM_SetCurObj(&work->layoutman, OBJECT_ENG2_4, 1); work->page = 4; GM_SdSet(SD_S_TYPING03); break;
							}
							else
							return 0;
	case KCS_CB_ENG2_4_L:	LOM_SetCurMode(&work->layoutman, LOM_MODE_36, 1, 1, 1); LOM_SetCurObj(&work->layoutman, OBJECT_ENG2_3, 1); work->page = 3; GM_SdSet(SD_S_TYPING03); break;
	case KCS_CB_ENG2_4_R:	return 0;



	case KCS_CB_JPN_1_OK :
	case KCS_CB_ENG_1_OK :
	case KCS_CB_ENG2_1_OK:
	case KCS_CB_JPN_2_OK :
	case KCS_CB_ENG_2_OK :
	case KCS_CB_ENG2_2_OK:
	case KCS_CB_JPN_3_OK :
	case KCS_CB_ENG_3_OK :
	case KCS_CB_ENG2_3_OK:
	case KCS_CB_JPN_4_OK :
	case KCS_CB_ENG_4_OK :
	case KCS_CB_ENG2_4_OK:	work->flag |= KCS_FADE_OUT; LOM_SetCurMode(&work->layoutman, LOM_MODE_22, 1, 1, 1); GM_SdSet(SD_S_WIN01); break;
	case KCS_CB_CANCELED :	work->flag |= KCS_FADE_OUT; LOM_SetCurMode(&work->layoutman, LOM_MODE_22, 1, 1, 1); work->flag |= KCS_CANCELED; GM_SdSet(SD_S_V_CANS02); break;

	case KCS_CB_CLOSED:
		GV_DestroyActor(work);
		break;

	default:
		return 0;
	}

	work->flag &= ~KCS_INPUT_OK;
	if(work->flag & KCS_FADE_OUT) work->count = -KCS_FADE_COUNT;
	else                          work->count = -KCS_INTERVAL_COUNT;

	return 1;
}

/******************************************************************************
 * 毎フレーム処理
 */
static void Act(
	WORK *work)		/* ワーク */
{
	int       obj = 0;
	SPR_COLOR sprcol;

	if(work->flag & KCS_INIT)
	{
		work->flag &= ~KCS_INIT;
		work->flag |=  KCS_INIT2;
		return;
	}
	if(work->flag & KCS_INIT2)
	{
		/*
		 * レイアウトの初期化
		 */
		work->flag &= ~KCS_INIT2;

		/* 矢印の色修正 */
		{
			SPR_COLOR col_spr;
			col_spr.r = 128;
			col_spr.g = 128;
			col_spr.b = 128;
			col_spr.a =  52;
			LOM_Spr_SetColor(&work->layoutman, OBJECT_LEFT, &col_spr);
			LOM_Spr_SetColor(&work->layoutman, OBJECT_RIGHT, &col_spr);
		}

		/* モードの設定 */
		if     (work->flag & KCS_JAPANESE) LOM_SetCurMode(&work->layoutman, LOM_MODE_20, 1, 1, 1);
		else if(work->flag & KCS_DEKACON ) LOM_SetCurMode(&work->layoutman, LOM_MODE_23, 1, 1, 1);
		else                               LOM_SetCurMode(&work->layoutman, LOM_MODE_21, 1, 1, 1);

		/* フェードイン */
		work->flag |= KCS_FADE_IN;
	}

	work->count++;

	/* レイアウトマネージャ */
	ActLayoutman(&work->layoutman);

	/* フェード */
	if(work->flag & KCS_FADE_IN)
	{
		/* フェードイン */
		if(work->flag & KCS_JAPANESE)
		{
			switch(work->page)
			{
			case 1: obj = OBJECT_JPN_1; break;
			case 2: obj = OBJECT_JPN_2; break;
			case 3: obj = OBJECT_JPN_3; break;
			case 4: obj = OBJECT_JPN_4; break;
			default: ASSERT(0);
			}
		}
		else if(work->flag & KCS_DEKACON)
		{
			switch(work->page)
			{
			case 1: obj = OBJECT_ENG2_1; break;
			case 2: obj = OBJECT_ENG2_2; break;
			case 3: obj = OBJECT_ENG2_3; break;
			case 4: obj = OBJECT_ENG2_4; break;
			default: ASSERT(0);
			}
		}
		else
		{
			switch(work->page)
			{
			case 1: obj = OBJECT_ENG_1; break;
			case 2: obj = OBJECT_ENG_2; break;
			case 3: obj = OBJECT_ENG_3; break;
			case 4: obj = OBJECT_ENG_4; break;
			default: ASSERT(0);
			}
		}
		ShowSprite(&work->layoutman, obj, 1);
		LOM_SetCurObj(&work->layoutman, obj, 1);

 		LOM_Spr_GetColor(&work->layoutman, obj, &sprcol);
		sprcol.a = (128 * (KCS_FADE_COUNT + work->count)) / KCS_FADE_COUNT;
		LOM_Spr_SetColor(&work->layoutman, obj, &sprcol);

		/* ＡＢボタン説明 */
		ShowSprite(&work->layoutman, OBJECT_ABBUTTON, 1);
 		LOM_Spr_GetColor(&work->layoutman, OBJECT_ABBUTTON, &sprcol);
		sprcol.a = (128 * (KCS_FADE_COUNT + work->count)) / KCS_FADE_COUNT;
		LOM_Spr_SetColor(&work->layoutman, OBJECT_ABBUTTON, &sprcol);

#ifdef KP_XBOX
		if(    (((GM_Configuration2 & GM_CONFIG_REGION) >> 12) == GM_CONFIG_REGION_EU)
			&& (GM_Language != GM_LANG_ENGLISH))
		{
			/* フェードイン */
			switch(work->page)
			{
			case 1: obj = OBJECT_DEKACON_1; break;
			case 2: obj = OBJECT_DEKACON_2; break;
			case 3: obj = OBJECT_DEKACON_3; break;
			case 4: obj = OBJECT_DEKACON_4; break;
			default: ASSERT(0);
			}
			ShowSprite(&work->layoutman, obj, 1);

	 		LOM_Spr_GetColor(&work->layoutman, obj, &sprcol);
			sprcol.a = (128 * (KCS_FADE_COUNT + work->count)) / KCS_FADE_COUNT;
			LOM_Spr_SetColor(&work->layoutman, obj, &sprcol);
		}
#endif
	}
	else if(work->flag & KCS_FADE_OUT)
	{
		/*フェードアウト*/
		if(work->flag & KCS_JAPANESE)
		{
			switch(work->page)
			{
			case 1: obj = OBJECT_JPN_1; break;
			case 2: obj = OBJECT_JPN_2; break;
			case 3: obj = OBJECT_JPN_3; break;
			case 4: obj = OBJECT_JPN_4; break;
			default: ASSERT(0);
			}
		}
		else if(work->flag & KCS_DEKACON)
		{
			switch(work->page)
			{
			case 1: obj = OBJECT_ENG2_1; break;
			case 2: obj = OBJECT_ENG2_2; break;
			case 3: obj = OBJECT_ENG2_3; break;
			case 4: obj = OBJECT_ENG2_4; break;
			default: ASSERT(0);
			}
		}
		else
		{
			switch(work->page)
			{
			case 1: obj = OBJECT_ENG_1; break;
			case 2: obj = OBJECT_ENG_2; break;
			case 3: obj = OBJECT_ENG_3; break;
			case 4: obj = OBJECT_ENG_4; break;
			default: ASSERT(0);
			}
		}
		LOM_Spr_GetColor(&work->layoutman, obj, &sprcol);
		sprcol.a = (128 * (-work->count)) / KCS_FADE_COUNT;
		LOM_Spr_SetColor(&work->layoutman, obj, &sprcol);

		/* ＡＢボタン説明 */
		LOM_Spr_GetColor(&work->layoutman, OBJECT_ABBUTTON, &sprcol);
		sprcol.a = (128 * (-work->count)) / KCS_FADE_COUNT;
		LOM_Spr_SetColor(&work->layoutman, OBJECT_ABBUTTON, &sprcol);

#ifdef KP_XBOX
		if(    (((GM_Configuration2 & GM_CONFIG_REGION) >> 12) == GM_CONFIG_REGION_EU)
			&& (GM_Language != GM_LANG_ENGLISH))
		{
			/*フェードアウト*/
			switch(work->page)
			{
			case 1: obj = OBJECT_DEKACON_1; break;
			case 2: obj = OBJECT_DEKACON_2; break;
			case 3: obj = OBJECT_DEKACON_3; break;
			case 4: obj = OBJECT_DEKACON_4; break;
			default: ASSERT(0);
			}
			ShowSprite(&work->layoutman, obj, 1);

			LOM_Spr_GetColor(&work->layoutman, obj, &sprcol);
			sprcol.a = (128 * (-work->count)) / KCS_FADE_COUNT;
			LOM_Spr_SetColor(&work->layoutman, obj, &sprcol);
		}
#endif
	}

	/* 矢印の表示設定 */
	if(work->flag & (KCS_FADE_IN | KCS_FADE_OUT))
	{
		ShowSprite(&work->layoutman, OBJECT_LEFT , 0);
		ShowSprite(&work->layoutman, OBJECT_RIGHT, 0);
	}
	else
	{
		ShowSprite(&work->layoutman, OBJECT_LEFT , 1);
		ShowSprite(&work->layoutman, OBJECT_RIGHT, 1);

		if(work->page == 1)
		{
			if(work->count >= 0)
				ShowSprite(&work->layoutman, OBJECT_LEFT, 0);
		}
		else if(work->page == work->t_pages)
		{
			if(work->count >= 0)
				ShowSprite(&work->layoutman, OBJECT_RIGHT, 0);
		}
	}

	/* カウンター処理 */
	if(work->count == 0)
	{
		work->flag |=  KCS_INPUT_OK;
		work->flag &= ~(KCS_FADE_IN | KCS_FADE_OUT);
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

	/* 結果を代入 */
	if(work->flag & KCS_CANCELED) *work->ret = -1;
	else                          *work->ret = work->page;
}

/******************************************************************************
 * 初期化処理
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	WORK *work,		/* ワーク */
	int  name,		/* シナリオ名 */
	int  where,		/* マップＩＤ */
	int  page_num,	/* ページ数 */
	int  init_page,	/* 初期ページ番号 1～ */
	int  *ret)		/* 結果 -1: キャンセル
					     0: 起動中
					     1: １ページ目を選択
					     2: ２ページ目を選択
					     3: ３ページ目を選択
					     4: ４ページ目を選択 */
{
	SY_PRINTF3("GetResources\n");

	work->count   = -KCS_FADE_COUNT;
	work->page    = init_page;
	work->ret     = ret;
	work->flag    = 0;
	work->t_pages = 0;

	/* フラグ */
#ifdef KP_XBOX
#ifndef KP_WINDOWS
	if(((GM_Configuration2 & GM_CONFIG_REGION) >> 12) == GM_CONFIG_REGION_EU)
	{
		work->flag |= KCS_DEKACON;
	}
	else
	{
		/* コントローラ検知 */
      BP_TODO_BREAK;
#if 0 //BP
		if(GV_GetXPadType(0) != XINPUT_DEVSUBTYPE_GC_GAMEPAD_ALT)
			work->flag |= KCS_DEKACON;
#endif
	}
#endif
#else
	work->flag |= KCS_DEKACON;
#endif

	/* 合計ページ数 */
	if((page_num != 2) && (page_num != 3) && (page_num != 4))
	{
		ASSERT(0);
	}
	work->t_pages = page_num;

	/* 補正 */
	if( !((0 < work->page) && (work->page <= work->t_pages)) )
		work->page = 1;

	/* レイアウトマネージャ */
	if(((GM_Configuration2 & GM_CONFIG_REGION) >> 12) == GM_CONFIG_REGION_EU)
	{
		if((work->t_pages == 2) || (work->t_pages == 3))
		{
			if(GM_Language == GM_LANG_ENGLISH)
			{
				if(CreateLayoutman3(&work->layoutman, L2D_FILENAME_ENG, 4, 7, 0, 0, 0, LOMCallback, NULL, work, 25, 1, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
			}
			else if(GM_Language == GM_LANG_FRENCH)
			{
				if(CreateLayoutman3(&work->layoutman, L2D_FILENAME_FRA, 4, 7, 0, 0, 0, LOMCallback, NULL, work, 25, 1, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
			}
			else if(GM_Language == GM_LANG_GERMANY)
			{
				if(CreateLayoutman3(&work->layoutman, L2D_FILENAME_GER, 4, 7, 0, 0, 0, LOMCallback, NULL, work, 25, 1, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
			}
			else if(GM_Language == GM_LANG_ITALY)
			{
				if(CreateLayoutman3(&work->layoutman, L2D_FILENAME_ITA, 4, 7, 0, 0, 0, LOMCallback, NULL, work, 25, 1, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
			}
			else if(GM_Language == GM_LANG_SPANISH)
			{
				if(CreateLayoutman3(&work->layoutman, L2D_FILENAME_SPA, 4, 7, 0, 0, 0, LOMCallback, NULL, work, 25, 1, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
			}
			else
			{
				ASSERT(0);
				if(CreateLayoutman3(&work->layoutman, L2D_FILENAME_ENG, 4, 7, 0, 0, 0, LOMCallback, NULL, work, 25, 1, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
			}
		}
		else
		{
			if(GM_Language == GM_LANG_ENGLISH)
			{
				if(CreateLayoutman3(&work->layoutman, L2D_FILENAME_ENG_2, 4, 7, 0, 0, 0, LOMCallback, NULL, work, 25, 1, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
			}
			else if(GM_Language == GM_LANG_FRENCH)
			{
				if(CreateLayoutman3(&work->layoutman, L2D_FILENAME_FRA_2, 4, 7, 0, 0, 0, LOMCallback, NULL, work, 25, 1, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
			}
			else if(GM_Language == GM_LANG_GERMANY)
			{
				if(CreateLayoutman3(&work->layoutman, L2D_FILENAME_GER_2, 4, 7, 0, 0, 0, LOMCallback, NULL, work, 25, 1, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
			}
			else if(GM_Language == GM_LANG_ITALY)
			{
				if(CreateLayoutman3(&work->layoutman, L2D_FILENAME_ITA_2, 4, 7, 0, 0, 0, LOMCallback, NULL, work, 25, 1, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
			}
			else if(GM_Language == GM_LANG_SPANISH)
			{
				if(CreateLayoutman3(&work->layoutman, L2D_FILENAME_SPA_2, 4, 7, 0, 0, 0, LOMCallback, NULL, work, 25, 1, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
			}
			else
			{
				ASSERT(0);
				if(CreateLayoutman3(&work->layoutman, L2D_FILENAME_ENG_2, 4, 7, 0, 0, 0, LOMCallback, NULL, work, 25, 1, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
			}
		}
	}
	else
	{
		if(((GM_Configuration2 & GM_CONFIG_REGION) >> 12) == GM_CONFIG_REGION_JAPAN)
		{
			if(CreateLayoutman3(&work->layoutman, L2D_FILENAME_JPN, 4, 7, 0, 0, 0, LOMCallback, NULL, work, 25, 1, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
		}
		else
		{
			if((work->t_pages == 2) || (work->t_pages == 3))
			{
				if(CreateLayoutman3(&work->layoutman, L2D_FILENAME, 4, 7, 0, 0, 0, LOMCallback, NULL, work, 25, 1, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
			}
			else
			{
				if(CreateLayoutman3(&work->layoutman, L2D_FILENAME_2, 4, 7, 0, 0, 0, LOMCallback, NULL, work, 25, 1, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
			}
		}
	}
	SY_PRINTF1("Create Layoutman Succeeded.\n");
	if(LoadLOMData(&work->layoutman, _lom_data) == 0) { SY_PRINTF2("Load LOM Data Failed.\n"); return 0; }
	SY_PRINTF1("Load LOM Data Succeeded.\n");

	/* 起動中に設定 */
	*work->ret = 0;

	work->flag |= KCS_INIT;

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
void *NewKeyConSel(
	int page_num,	/* ページ数 */
	int init_page,	/* 初期ページ番号 1～ */
	int *ret)		/* 結果 -1: キャンセル
					     0: 起動中
					     1: １ページ目を選択
					     2: ２ページ目を選択
					     3: ３ページ目を選択
					     4: ４ページ目を選択 */
{
	WORK *work;

	SY_PRINTF3("NewKeyConSel\n");
	OPERATOR();

	if(ret == NULL)
		ASSERT(0);

	/* アクター生成 */
	work = (WORK *)GV_CreateActor(
		GV_ACTOR_USER,				/* プロセスの優先順位 */
		GV_CLASS_CHARA,				/* プロセスクラス */
		sizeof(WORK),				/* メモリ確保サイズ */
		0);							/* 実行プライオリティ */
	if(work != NULL)
	{
		/* アクター登録 */
		GV_SetActor(&work->actor, Act, Die);
		GV_ActorEX(&work->actor);

		/* ワークの初期化 */
		if(!GetResources(work, 0, GM_CurrentMap, page_num, init_page, ret))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}
