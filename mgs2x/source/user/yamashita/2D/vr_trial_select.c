/*******************************************************************************
 * vr_trial_select - vr_trial_select.c
 * ＶＲ体験版セレクト  *NewVRTrialSelect
 * 2002/06/04 S.Yamashita
 * $Id: vr_trial_select.c,v 1.2 2002/12/05 18:41:57 takaki Exp $
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

#define __CHARA_NAME__ "VR Trial Select"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
//#define SY_PRINTF3_DEBUG
#include "../sy_util/sy_util.h"

/*******************************************************************************
 * defines
 */

/* コールバックタイプ */
enum {
	VRTRS_CB_ITEM_1_UP = 1,
	VRTRS_CB_ITEM_1_DN,
	VRTRS_CB_ITEM_1_OK,
	VRTRS_CB_ITEM_2_UP,
	VRTRS_CB_ITEM_2_DN,
	VRTRS_CB_ITEM_2_OK,
	VRTRS_CB_ITEM_3_UP,
	VRTRS_CB_ITEM_3_DN,
	VRTRS_CB_ITEM_3_OK,
	VRTRS_CB_ITEM_4_UP,
	VRTRS_CB_ITEM_4_DN,
	VRTRS_CB_ITEM_4_OK,
	VRTRS_CB_ITEM_5_UP,
	VRTRS_CB_ITEM_5_DN,
	VRTRS_CB_ITEM_5_OK,
	VRTRS_CB_ITEM_6_UP,
	VRTRS_CB_ITEM_6_DN,
	VRTRS_CB_ITEM_6_OK,

	VRTRS_CB_ITEM_7_UP,
	VRTRS_CB_ITEM_7_DN,
	VRTRS_CB_ITEM_7_OK,

	VRTRS_CB_CANCELED,
	VRTRS_CB_CLOSED,
};

/* フラグ */
#define VRTRS_INIT        (0x01)	/* 初期化時 */
#define VRTRS_INPUT_OK    (0x02)	/* 入力ＯＫ */

/* 設定値 */
#define VRTRS_TEX_WIDTH (18.0f)		/* テクスチャ幅 */
#define VRTRS_ICO_R     (-27)		/* アイコン相対位置Ｘ */
#define VRTRS_ICO_T     (  0)		/* アイコン相対位置Ｙ */

/* Ｌ２Ｄオブジェクト */
#define L2D_FILENAME           ( 7041366)		/* vr_training */
#define ACTION_OPEN_FRAME      (  863270)		/* openFrame */
#define ACTION_CLOSE_FRAME     ( 6956960)		/* closeFrame */
#define ACTION_SHOW_RECORD     (16129319)		/* show_record */
#define ACTION_HIDE_RECORD     (  691452)		/* hide_record */
#define ACTION_DUMMY           ( 8143391)		/* dummy */
#define ACTION_DUMMY2          ( 8930337)		/* dummy2 */
#define ACTION_STAGE_1_0       ( 5282964)		/* hide_sneaking */
#define ACTION_STAGE_0_1       ( 9522242)		/* show_sneaking */
#define ACTION_STAGE_2_1       (13027175)		/* hide_weapon_handgun */
#define ACTION_STAGE_0_2       ( 8223627)		/* show_weapon_handgun_02 */
#define ACTION_STAGE_1_2       (15905527)		/* show_weapon_handgun */
#define ACTION_STAGE_3_2       (10456838)		/* hide_eliminate_all */
#define ACTION_STAGE_0_3       ( 3347113)		/* show_eliminate_all_02 */
#define ACTION_STAGE_2_3       ( 2158179)		/* show_eliminate_all */
#define ACTION_STAGE_4_3       (12262719)		/* hide_weapon_hf_blade */
#define ACTION_STAGE_0_4       (10755705)		/* show_weapon_hf_blade_02 */
#define ACTION_STAGE_3_4       ( 3706693)		/* show_weapon_hf_blade */
#define ACTION_STAGE_5_4       ( 9148776)		/* hide_vraiety_01 */
#define ACTION_STAGE_4_5       (10515601)		/* show_vraiety */
#define ACTION_STAGE_0_5       ( 4870764)		/* show_vraiety_02 */
#define ACTION_STAGE_5_0       ( 9148777)		/* hide_vraiety_02 */

#define ACTION_STAGE_1R_0      (2832407)		/* hide_record_sneaking */
#define ACTION_STAGE_0_1R      (11053596)		/* show_record_sneaking */
#define ACTION_STAGE_0_5R      (7417728)		/* show_record_vraiety */
#define ACTION_STAGE_5R_0      (4539376)		/* hide_record_vraiety */

#define OBJECT_BG_1            ( 7819999)		/* bg_sna_02 */
#define OBJECT_BG_2            (15413940)		/* bg_sna_def */
#define OBJECT_BG_3            (7025243)		/* bg_rai_def */
#define OBJECT_BG_4            ( 11227868)		/* bg_rai_02 */

#define OBJECT_ITEM_1          ( 10991609)		/* stage_01 */
#define OBJECT_ITEM_1_1        ( 16306266)		/* stage_num_01_1 */
#define OBJECT_ITEM_1_10       ( 1706895)		/* stage_num_01_10 */
#define OBJECT_ITEM_2          ( 10991610)		/* stage_02 */
#define OBJECT_ITEM_2_1        ( 16307290)		/* stage_num_02_1 */
#define OBJECT_ITEM_2_10       ( 1739663)		/* stage_num_02_10 */
#define OBJECT_ITEM_3          ( 10991611)		/* stage_03 */
#define OBJECT_ITEM_3_1        ( 16308314)		/* stage_num_03_1 */
#define OBJECT_ITEM_3_10       ( 1772431)		/* stage_num_03_10 */
#define OBJECT_ITEM_4          ( 10991612)		/* stage_04 */
#define OBJECT_ITEM_4_1        ( 16309338)		/* stage_num_04_1 */
#define OBJECT_ITEM_4_10       ( 1805199)		/* stage_num_04_10 */
#define OBJECT_ITEM_5          ( 10991613)		/* stage_05 */
#define OBJECT_ITEM_5_1        ( 16310362)		/* stage_num_05_1 */
#define OBJECT_ITEM_5_10       ( 1837967)		/* stage_num_05_10 */
#define OBJECT_ITEM_6          ( 3871525)		/* save */
#define OBJECT_ITEM_7          ( 3435924)		/* exit */

#define OBJECT_REC_RECORD      ( 8604558)		/* record */
#define OBJECT_REC_BAR         (12403282)		/* record_underBar */
#define OBJECT_REC_1ST         (   53972)		/* 1st */
#define OBJECT_REC_1ST_1       ( 5305814)		/* 1st_num_1 */
#define OBJECT_REC_1ST_10      ( 2013946)		/* 1st_num_10 */
#define OBJECT_REC_1ST_100     (14114675)		/* 1st_num_100 */
#define OBJECT_REC_1ST_1000    (15462058)		/* 1st_num_1000 */
#define OBJECT_REC_1ST_10000   ( 8246669)		/* 1st_num_10000 */
#define OBJECT_REC_1ST_100000  (12235231)		/* 1st_num_100000 */
#define OBJECT_REC_1ST_1000000 ( 5651495)		/* 1st_num_1000000 */
#define OBJECT_REC_2ND         (   54820)		/* 2nd */
#define OBJECT_REC_2ND_1       ( 5360086)		/* 2nd_num_1 */
#define OBJECT_REC_2ND_10      ( 3750650)		/* 2nd_num_10 */
#define OBJECT_REC_2ND_100     ( 2580343)		/* 2nd_num_100 */
#define OBJECT_REC_2ND_1000    (15462164)		/* 2nd_num_1000 */
#define OBJECT_REC_2ND_10000   ( 8250061)		/* 2nd_num_10000 */
#define OBJECT_REC_2ND_100000  (12343775)		/* 2nd_num_100000 */
#define OBJECT_REC_2ND_1000000 ( 9124903)		/* 2nd_num_1000000 */
#define OBJECT_REC_3RD         (   55972)		/* 3rd */
#define OBJECT_REC_3RD_1       ( 5433814)		/* 3rd_num_1 */
#define OBJECT_REC_3RD_10      ( 6109946)		/* 3rd_num_10 */
#define OBJECT_REC_3RD_100     (10968955)		/* 3rd_num_100 */
#define OBJECT_REC_3RD_1000    (15462308)		/* 3rd_num_1000 */
#define OBJECT_REC_3RD_10000   ( 8254669)		/* 3rd_num_10000 */
#define OBJECT_REC_3RD_100000  (12491231)		/* 3rd_num_100000 */
#define OBJECT_REC_3RD_1000000 (13843495)		/* 3rd_num_1000000 */

/*******************************************************************************
 * work
 */

/* ワーク */
typedef struct tagWORK
{
	GV_ACT_EX      actor;			/* アクター */

	int            proc[7];			/* プロシージャ */
	int            name;			/* シナリオ名 */

	unsigned int   inactive[6];		/* 非アクティブ時色  0:未クリア  1:１位  2:２位  3:３位  4:クリア  5:不可 */
	unsigned int   active[5];		/* アクティブ時色    0:未クリア  1:１位  2:２位  3:３位  4:クリア         */
	float          tex_u0;			/* テクスチャのＵ原点 */

	unsigned char  inactive_a[6];	/* 非アクティブ時アルファ  0:未クリア  1:１位  2:２位  3:３位  4:クリア  5:不可 */
	unsigned char  active_a[5];		/* アクティブ時アルファ    0:未クリア  1:１位  2:２位  3:３位  4:クリア         */
	char           index;			/* インデックス */
	char           flag;			/* フラグ */
	char           prev;			/* 前の選択アイテム番号 */
	unsigned short count;			/* カウンター */

	LAYOUTMAN      layoutman;		/* レイアウトマネージャ */

	unsigned short count2;			/* カウンター２ */
}
WORK;

/*******************************************************************************
 * local
 */

static void *_work = NULL;	/* 多重起動防止 */

/* ＬＯＭデータ */
static int _lom_data[] = {
					/*アクションリスト名*/		/*アクション名*/		/*カウント*/	/*フラグ*/
LOM_ACTLIST_DATA,	LOM_ACTLIST_00,				ACTION_DUMMY,			1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												LOM_DATA_END,

					LOM_ACTLIST_01,				ACTION_OPEN_FRAME,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_02,				ACTION_DUMMY,			1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_03,				ACTION_HIDE_RECORD,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_04,				ACTION_SHOW_RECORD,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_05,				ACTION_CLOSE_FRAME,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_DUMMY,			1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												ACTION_DUMMY2,			1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												LOM_DATA_END,

					LOM_ACTLIST_06,				ACTION_STAGE_1_0,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_07,				ACTION_STAGE_0_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_08,				ACTION_STAGE_2_1,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_09,				ACTION_STAGE_0_2,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_10,				ACTION_STAGE_1_2,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_11,				ACTION_STAGE_3_2,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_12,				ACTION_STAGE_0_3,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_13,				ACTION_STAGE_2_3,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_14,				ACTION_STAGE_4_3,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_15,				ACTION_STAGE_0_4,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_16,				ACTION_STAGE_3_4,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_17,				ACTION_STAGE_5_4,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_18,				ACTION_STAGE_4_5,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_19,				ACTION_STAGE_0_5,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_20,				ACTION_STAGE_5_0,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_21,				ACTION_STAGE_1R_0,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_22,				ACTION_STAGE_0_1R,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_23,				ACTION_STAGE_0_5R,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_24,				ACTION_STAGE_5R_0,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_DATA_END,

					/*アイコンリスト名*/		/* アイコン名 */	/* テクスチャ名 */	/* 位置(左、上、右、下) */								/* 色 */												/* カウント */	/* フラグ */
LOM_ICOLIST_DATA,	LOM_ICOLIST_00,				LOM_ICO_00,			LOM_ICO_NOTEX,		VRTRS_ICO_R, VRTRS_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H,		VRDEF_CUR_R, VRDEF_CUR_G, VRDEF_CUR_B, VRDEF_CUR_A,		LOM_ICO_LOOP,	LOM_ICO_L_REL|LOM_ICO_T_REL|LOM_ICO_SIZE|LOM_ICO_ALPHA,
												LOM_DATA_END,
					LOM_DATA_END,

#ifdef AREA_EU	// #ifdef PAL
					/* オブジェクトリスト名 */	/* オブジェクト名 */	/* アイコンリスト名 */	/* カウント */	/* フラグ */
LOM_OBJLIST_DATA,	LOM_OBJLIST_00,				OBJECT_ITEM_1,			LOM_ICOLIST_00,			7,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_2,			LOM_ICOLIST_00,			7,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_3,			LOM_ICOLIST_00,			7,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_4,			LOM_ICOLIST_00,			7,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_5,			LOM_ICOLIST_00,			7,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_6,			LOM_ICOLIST_00,			7,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_7,			LOM_ICOLIST_00,			7,				LOM_OBJ_INTERP_1,
												LOM_DATA_END,
					LOM_DATA_END,
#else
					/* オブジェクトリスト名 */	/* オブジェクト名 */	/* アイコンリスト名 */	/* カウント */	/* フラグ */
LOM_OBJLIST_DATA,	LOM_OBJLIST_00,				OBJECT_ITEM_1,			LOM_ICOLIST_00,			8,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_2,			LOM_ICOLIST_00,			8,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_3,			LOM_ICOLIST_00,			8,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_4,			LOM_ICOLIST_00,			8,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_5,			LOM_ICOLIST_00,			8,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_6,			LOM_ICOLIST_00,			8,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_7,			LOM_ICOLIST_00,			8,				LOM_OBJ_INTERP_1,
												LOM_DATA_END,
					LOM_DATA_END,
#endif

					/*パッドリスト名*/			/*アクション名*/		/*オブジェクト名*/	/*入力*/	/*実行番号*/		/*実行引数*/			/*フラグ*/
LOM_PADLIST_DATA,	LOM_PADLIST_00,				LOM_PAD_ALLACT,			OBJECT_ITEM_1,		PAD_U,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_1_UP,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_1,		PAD_D,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_1_DN,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_2,		PAD_U,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_2_UP,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_2,		PAD_D,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_2_DN,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_3,		PAD_U,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_3_UP,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_3,		PAD_D,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_3_DN,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_4,		PAD_U,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_4_UP,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_4,		PAD_D,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_4_DN,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_5,		PAD_U,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_5_UP,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_5,		PAD_D,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_5_DN,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_6,		PAD_U,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_6_UP,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_6,		PAD_D,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_6_DN,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_7,		PAD_U,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_7_UP,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_7,		PAD_D,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_7_DN,		LOM_PAD_ON_SIG,

												LOM_PAD_ALLACT,			OBJECT_ITEM_1,		PAD_OK,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_1_OK,		LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ITEM_2,		PAD_OK,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_2_OK,		LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ITEM_3,		PAD_OK,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_3_OK,		LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ITEM_4,		PAD_OK,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_4_OK,		LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ITEM_5,		PAD_OK,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_5_OK,		LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ITEM_6,		PAD_OK,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_6_OK,		LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ITEM_7,		PAD_OK,		LOM_EXE_CALLBACK,	VRTRS_CB_ITEM_7_OK,		LOM_PAD_PRESS,

												LOM_PAD_ALLACT,			LOM_PAD_ALLOBJ,		PAD_CANCEL,	LOM_EXE_CALLBACK,	VRTRS_CB_CANCELED,		LOM_PAD_PRESS,
												LOM_DATA_END,

					/* 閉じた後用 */
					LOM_PADLIST_01,				ACTION_DUMMY,			LOM_PAD_ALLOBJ,		LOM_PAD_ANYINPUT,	LOM_EXE_CALLBACK,	VRTRS_CB_CLOSED,		0,
												LOM_DATA_END,
					LOM_DATA_END,

					/*モード名*/	/*ＬＯＭアクションリスト*/	/*ＬＯＭオブジェクトリスト*/	/*ＬＯＭパッドリスト*/
LOM_MODE_DATA,		LOM_MODE_00,	LOM_ACTLIST_00,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* init */
					LOM_MODE_01,	LOM_ACTLIST_01,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* open */
					LOM_MODE_02,	LOM_ACTLIST_02,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* dummy */
					LOM_MODE_03,	LOM_ACTLIST_03,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* hide */
					LOM_MODE_04,	LOM_ACTLIST_04,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* show */
					LOM_MODE_05,	LOM_ACTLIST_05,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* close */

					LOM_MODE_06,	LOM_ACTLIST_06,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* stage 1->0 */
					LOM_MODE_07,	LOM_ACTLIST_07,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* stage 0->1 */
					LOM_MODE_08,	LOM_ACTLIST_08,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* stage 2->1 */
					LOM_MODE_09,	LOM_ACTLIST_09,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* stage 0->2 */
					LOM_MODE_10,	LOM_ACTLIST_10,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* stage 1->2 */
					LOM_MODE_11,	LOM_ACTLIST_11,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* stage 3->2 */
					LOM_MODE_12,	LOM_ACTLIST_12,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* stage 0->3 */
					LOM_MODE_13,	LOM_ACTLIST_13,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* stage 2->3 */
					LOM_MODE_14,	LOM_ACTLIST_14,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* stage 4->3 */
					LOM_MODE_15,	LOM_ACTLIST_15,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* stage 0->4 */
					LOM_MODE_16,	LOM_ACTLIST_16,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* stage 3->4 */
					LOM_MODE_17,	LOM_ACTLIST_17,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* stage 5->4 */
					LOM_MODE_18,	LOM_ACTLIST_18,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* stage 4->5 */
					LOM_MODE_19,	LOM_ACTLIST_19,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* stage 0->5 */
					LOM_MODE_20,	LOM_ACTLIST_20,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* stage 5->0 */

					LOM_MODE_21,	LOM_ACTLIST_21,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* stage 1r->0  */
					LOM_MODE_22,	LOM_ACTLIST_22,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* stage 0 ->1r */
					LOM_MODE_23,	LOM_ACTLIST_23,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* stage 0 ->5r */
					LOM_MODE_24,	LOM_ACTLIST_24,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* stage 5r->0  */
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
 * スプライトの色の設定
 */
static void SetSpriteColor(
	LAYOUTMAN    *layoutman,		/* レイアウトマネージャ */
	int          strcode,			/* 文字列コード */
	int          active,			/* アクティブフラグ  0:非アクティブ  1:アクティブ */
	int          step,				/* 変化度 */
	unsigned int inactive_color,	/* 非アクティブ色 */
	unsigned int active_color)		/* アクティブ色 */
{
	SPR_COLOR    sprcol;
	unsigned int color;
	int          add;

	{
		SPR_OBJ *spr;
		if((spr = L2D_GetObject(layoutman->layout, strcode)) == NULL)
			return;
	}

	if(active == 0) color = inactive_color;
	else            color = active_color;

	LOM_Spr_GetColor(layoutman, strcode, &sprcol);
	{
		if((abs(add = (color & 0x000000ff) - sprcol.r)) <= step)
		                 sprcol.r  = (color & 0x000000ff);
		else if(add > 0) sprcol.r += step;
		else             sprcol.r -= step;

		if((abs(add = ((color & 0x0000ff00) >> 8) - sprcol.g)) <= step)
		                 sprcol.g  = ((color & 0x0000ff00) >> 8);
		else if(add > 0) sprcol.g += step;
		else             sprcol.g -= step;

		if((abs(add = ((color & 0x00ff0000) >> 16) - sprcol.b)) <= step)
		                 sprcol.b  = ((color & 0x00ff0000) >> 16);
		else if(add > 0) sprcol.b += step;
		else             sprcol.b -= step;
	}
	LOM_Spr_SetColor(layoutman, strcode, &sprcol);
}

/*******************************************************************************
 * スプライトのアルファの設定
 */
static void SetSpriteAlpha(
	LAYOUTMAN     *layoutman,		/* レイアウトマネージャ */
	int           strcode,			/* 文字列コード */
	int           active,			/* アクティブフラグ  0:非アクティブ  1:アクティブ */
	int           step,				/* 変化度 */
	unsigned char inactive_alpha,	/* 非アクティブアルファ */
	unsigned char active_alpha)		/* アクティブアルファ */
{
	SPR_COLOR     sprcol;
	unsigned char alpha;
	int           add;

	{
		SPR_OBJ *spr;
		if((spr = L2D_GetObject(layoutman->layout, strcode)) == NULL)
			return;
	}

	if(active == 0) alpha = inactive_alpha;
	else            alpha = active_alpha;

	LOM_Spr_GetColor(layoutman, strcode, &sprcol);
	{
		if(abs(add = alpha - sprcol.a) <= step)
		                 sprcol.a  = alpha;
		else if(add > 0) sprcol.a += step;
		else             sprcol.a -= step;
	}
	LOM_Spr_SetColor(layoutman, strcode, &sprcol);

	if(sprcol.a == 0) ShowSprite(layoutman, strcode, 0);
	else              ShowSprite(layoutman, strcode, 1);
}

/*******************************************************************************
 * スコアカウンターの表示設定
 */
static void SetScore(
	WORK *work,		/* ワーク */
	int  object_7,	/* １００００００の位 */
	int  object_6,	/* １０００００の位 */
	int  object_5,	/* １００００の位 */
	int  object_4,	/* １０００の位 */
	int  object_3,	/* １００の位 */
	int  object_2,	/* １０の位 */
	int  object_1,	/* １の位 */
	int  value)		/* 値 */
{
	int i;

	if(value < 0)
		return;

	/* １００００００の位 */
	i = value / 1000000;
	value %= 1000000;
	if(i > 9) i = 9;
	LOM_SprTex_SetU(&work->layoutman, object_7, work->tex_u0 + VRTRS_TEX_WIDTH * i);

	/* １０００００の位 */
	i = value / 100000;
	value %= 100000;
	LOM_SprTex_SetU(&work->layoutman, object_6, work->tex_u0 + VRTRS_TEX_WIDTH * i);

	/* １００００の位 */
	i = value / 10000;
	value %= 10000;
	LOM_SprTex_SetU(&work->layoutman, object_5, work->tex_u0 + VRTRS_TEX_WIDTH * i);

	/* １０００の位 */
	i = value / 1000;
	value %= 1000;
	LOM_SprTex_SetU(&work->layoutman, object_4, work->tex_u0 + VRTRS_TEX_WIDTH * i);

	/* １００の位 */
	i = value / 100;
	value %= 100;
	LOM_SprTex_SetU(&work->layoutman, object_3, work->tex_u0 + VRTRS_TEX_WIDTH * i);

	/* １０の位 */
	i = value / 10;
	value %= 10;
	LOM_SprTex_SetU(&work->layoutman, object_2, work->tex_u0 + VRTRS_TEX_WIDTH * i);

	/* １の位 */
	LOM_SprTex_SetU(&work->layoutman, object_1, work->tex_u0 + VRTRS_TEX_WIDTH * value);
}

/*******************************************************************************
 * スコアカウンターの表示設定
 */
static void SetScores(
	WORK *work,		/* ワーク */
	int  stage_id)
{
   BP_TODO_BREAK;
#if 0 //BP
	SetScore(work,
		OBJECT_REC_1ST_1000000,
		OBJECT_REC_1ST_100000,
		OBJECT_REC_1ST_10000,
		OBJECT_REC_1ST_1000,
		OBJECT_REC_1ST_100,
		OBJECT_REC_1ST_10,
		OBJECT_REC_1ST_1,
		MSN_HISCORE2(stage_id, 1));
	SetScore(work,
		OBJECT_REC_2ND_1000000,
		OBJECT_REC_2ND_100000,
		OBJECT_REC_2ND_10000,
		OBJECT_REC_2ND_1000,
		OBJECT_REC_2ND_100,
		OBJECT_REC_2ND_10,
		OBJECT_REC_2ND_1,
		MSN_HISCORE2(stage_id, 2));
	SetScore(work,
		OBJECT_REC_3RD_1000000,
		OBJECT_REC_3RD_100000,
		OBJECT_REC_3RD_10000,
		OBJECT_REC_3RD_1000,
		OBJECT_REC_3RD_100,
		OBJECT_REC_3RD_10,
		OBJECT_REC_3RD_1,
		MSN_HISCORE2(stage_id, 3));
#endif
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

	if(param != VRTRS_CB_CLOSED)
	{
		if(!(work->flag & VRTRS_INPUT_OK))
			return 0;
		work->flag &= ~VRTRS_INPUT_OK;
		work->count2 = 0;
	}

	switch(param)
	{
	case VRTRS_CB_ITEM_1_DN:
		GM_SdSet(SD_S_CUR01);
		LOM_SetCurMode(&work->layoutman, LOM_MODE_10, 1, 1, 0);
		work->count = 0;
		LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_2, 1);
		return 1;
	case VRTRS_CB_ITEM_2_DN:
		GM_SdSet(SD_S_CUR01);
		LOM_SetCurMode(&work->layoutman, LOM_MODE_13, 1, 1, 0);
		work->count = 0;
		LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_3, 1);
		return 1;
	case VRTRS_CB_ITEM_3_DN:
		GM_SdSet(SD_S_CUR01);
		LOM_SetCurMode(&work->layoutman, LOM_MODE_16, 1, 1, 0);
		work->count = 0;
		LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_4, 1);
		return 1;
	case VRTRS_CB_ITEM_4_DN:
		GM_SdSet(SD_S_CUR01);
		LOM_SetCurMode(&work->layoutman, LOM_MODE_18, 1, 1, 0);
		work->count = 0;
		LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_5, 1);
		return 1;
	case VRTRS_CB_ITEM_5_DN:
		GM_SdSet(SD_S_CUR01);
		LOM_SetCurMode(&work->layoutman, LOM_MODE_24, 1, 1, 0);
		work->count = 0;
		LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_6, 1);
		return 1;
	case VRTRS_CB_ITEM_6_DN:
		GM_SdSet(SD_S_CUR01);
		LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_7, 1);
		return 1;
	case VRTRS_CB_ITEM_7_DN:
		GM_SdSet(SD_S_CUR01);
		LOM_SetCurMode(&work->layoutman, LOM_MODE_22, 1, 1, 0);
		work->count = 0;
		LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_1, 1);
		return 1;

	case VRTRS_CB_ITEM_1_UP:
		GM_SdSet(SD_S_CUR01);
		LOM_SetCurMode(&work->layoutman, LOM_MODE_21, 1, 1, 0);
		work->count = 0;
		LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_7, 1);
		return 1;
	case VRTRS_CB_ITEM_2_UP:
		GM_SdSet(SD_S_CUR01);
		LOM_SetCurMode(&work->layoutman, LOM_MODE_08, 1, 1, 0);
		work->count = 0;
		LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_1, 1);
		return 1;
	case VRTRS_CB_ITEM_3_UP:
		GM_SdSet(SD_S_CUR01);
		LOM_SetCurMode(&work->layoutman, LOM_MODE_11, 1, 1, 0);
		work->count = 0;
		LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_2, 1);
		return 1;
	case VRTRS_CB_ITEM_4_UP:
		GM_SdSet(SD_S_CUR01);
		LOM_SetCurMode(&work->layoutman, LOM_MODE_14, 1, 1, 0);
		work->count = 0;
		LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_3, 1);
		return 1;
	case VRTRS_CB_ITEM_5_UP:
		GM_SdSet(SD_S_CUR01);
		LOM_SetCurMode(&work->layoutman, LOM_MODE_17, 1, 1, 0);
		work->count = 0;
		LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_4, 1);
		return 1;
	case VRTRS_CB_ITEM_6_UP:
		GM_SdSet(SD_S_CUR01);
		SY_PRINTF1("LOM_MODE_04\n");
		LOM_SetCurMode(&work->layoutman, LOM_MODE_23, 1, 1, 0);
		work->count = 0;
		LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_5, 1);
		return 1;
	case VRTRS_CB_ITEM_7_UP:
		GM_SdSet(SD_S_CUR01);
		LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_6, 1);
		return 1;

	case VRTRS_CB_ITEM_1_OK: work->index = 0; SY_PRINTF1("LOM_MODE_05\n"); LOM_SetCurMode(&work->layoutman, LOM_MODE_05, 1, 1, 1); work->count = 0; GM_SdSet(SD_S_START001); GM_SdSet(SD_S_WINCLS01); return 1;
	case VRTRS_CB_ITEM_2_OK: work->index = 1; SY_PRINTF1("LOM_MODE_05\n"); LOM_SetCurMode(&work->layoutman, LOM_MODE_05, 1, 1, 1); work->count = 0; GM_SdSet(SD_S_START001); GM_SdSet(SD_S_WINCLS01); return 1;
	case VRTRS_CB_ITEM_3_OK: work->index = 2; SY_PRINTF1("LOM_MODE_05\n"); LOM_SetCurMode(&work->layoutman, LOM_MODE_05, 1, 1, 1); work->count = 0; GM_SdSet(SD_S_START001); GM_SdSet(SD_S_WINCLS01); return 1;
	case VRTRS_CB_ITEM_4_OK: work->index = 3; SY_PRINTF1("LOM_MODE_05\n"); LOM_SetCurMode(&work->layoutman, LOM_MODE_05, 1, 1, 1); work->count = 0; GM_SdSet(SD_S_START001); GM_SdSet(SD_S_WINCLS01); return 1;
	case VRTRS_CB_ITEM_5_OK: work->index = 4; SY_PRINTF1("LOM_MODE_05\n"); LOM_SetCurMode(&work->layoutman, LOM_MODE_05, 1, 1, 1); work->count = 0; GM_SdSet(SD_S_START001); GM_SdSet(SD_S_WINCLS01); return 1;
	case VRTRS_CB_ITEM_6_OK: work->index = 5; SY_PRINTF1("LOM_MODE_05\n"); LOM_SetCurMode(&work->layoutman, LOM_MODE_05, 1, 1, 1); work->count = 0; GM_SdSet(SD_S_WIN01   ); GM_SdSet(SD_S_WINCLS01); work->prev = 6; return 1;
	case VRTRS_CB_ITEM_7_OK: work->index = 6; SY_PRINTF1("LOM_MODE_05\n"); LOM_SetCurMode(&work->layoutman, LOM_MODE_05, 1, 1, 1); work->count = 0; GM_SdSet(SD_S_START01 ); GM_SdSet(SD_S_WINCLS01); work->prev = 7; return 1;

	case VRTRS_CB_CANCELED:
		work->prev = LOM_GetCurObjI(&work->layoutman) + 1;

		work->index = 6;
		SY_PRINTF1("LOM_MODE_05\n");
		LOM_SetCurMode(&work->layoutman, LOM_MODE_05, 1, 1, 1);
		work->count = 0;
		GM_SdSet(SD_S_START01);
		GM_SdSet(SD_S_WINCLS01);
		return 1;

	case VRTRS_CB_CLOSED:
		/* プロック実行 */
		ASSERT(work->proc[(int)work->index]);
		SY_PRINTF1("PROC CALLED\n");
		if((0 <= work->index) && (work->index <= 4))
		{
			MSN_RETRY_COUNT    = 0;
			MSN_CONTINUE_COUNT = 0;
			MSN_2DSTATUS &= ~MSN_2DSTAT_QUICK_WINDOW;
		}
		GM_ExecProc(work->proc[(int)work->index], NULL);
		return 1;
	default:
		return 1;
	}
}

/*******************************************************************************
 * メッセージ処理

mesg ＶＲ体験版セレクト $s:名前 表示[0] $i:カーソル位置	// 1～
mesg ＶＲ体験版セレクト $s:名前 非表示[1]
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
		case 0:		/* 表示 */
			SY_PRINTF1("LOM_MODE_01 - %d\n", msg->message[1]);
			LOM_SetCurMode(&work->layoutman, LOM_MODE_01, 1, 1, 0);
			work->prev = msg->message[1];
			work->count = 0;
			GM_SdSet(SD_S_LINEMOV1);
			ret = 0;
			break;

		case 1:		/* 非表示 */
			SY_PRINTF1("LOM_MODE_00\n");
			LOM_SetCurMode(&work->layoutman, LOM_MODE_00, 1, 1, 1);
			work->count = 0;
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
   BP_TODO_BREAK;
#if 0 //BP
	int item_1, item_2, item_3, item_4, item_5;
	int clear[3];

	if(work->flag & VRTRS_INIT)
	{
		/*
		 * レイアウトの初期化
		 */
		work->flag &= ~VRTRS_INIT;

		/* 数字テクスチャのＵＶ関連の値の取得 */
		LOM_SprTex_GetU(&work->layoutman, OBJECT_REC_1ST_1000000, &work->tex_u0);
		SY_PRINTF1("u0: %f\n", work->tex_u0);

		/* モードの設定 */
		SY_PRINTF1("LOM_MODE_00\n"); 
		LOM_SetCurMode(&work->layoutman, LOM_MODE_00, 1, 1, 1);
	}

	work->count++;
	work->count2++;

	/* メッセージ処理 */
	ReceiveMessage(work);

	/* レイアウトマネージャ */
	ActLayoutman(&work->layoutman);

	/* 各ステージのクリア状況 */
	item_1 = Msn_GetClearLevel(VRDEF_TRIAL_1);
	item_2 = Msn_GetClearLevel(VRDEF_TRIAL_2);// if (item_1 == 0)                   item_2 = 5;
	item_3 = Msn_GetClearLevel(VRDEF_TRIAL_3);// if((item_2 == 0) || (item_2 == 5)) item_3 = 5;
	item_4 = Msn_GetClearLevel(VRDEF_TRIAL_4);// if((item_3 == 0) || (item_3 == 5)) item_4 = 5;
	item_5 = Msn_GetClearLevel(VRDEF_TRIAL_5);// if((item_4 == 0) || (item_4 == 5)) item_5 = 5;

	/* 各モードごとの処理 */
	if(LOM_GetCurMode(&work->layoutman) == LOM_MODE_00)
	{
		/* 背景 */
		SetSpriteColor(&work->layoutman, OBJECT_BG_1, 1, 256, VRDEF_ACTIVE, VRDEF_ACTIVE);
		SetSpriteColor(&work->layoutman, OBJECT_BG_2, 1, 256, VRDEF_ACTIVE, VRDEF_ACTIVE);
		SetSpriteColor(&work->layoutman, OBJECT_BG_3, 1, 256, VRDEF_ACTIVE, VRDEF_ACTIVE);
		SetSpriteColor(&work->layoutman, OBJECT_BG_4, 1, 256, VRDEF_ACTIVE, VRDEF_ACTIVE);
		SetSpriteAlpha(&work->layoutman, OBJECT_BG_1, 0, 256, 0, VRDEF_ACTIVE_A);
		SetSpriteAlpha(&work->layoutman, OBJECT_BG_2, 0, 256, 0, VRDEF_ACTIVE_A);
		SetSpriteAlpha(&work->layoutman, OBJECT_BG_3, 0, 256, 0, VRDEF_ACTIVE_A);
		SetSpriteAlpha(&work->layoutman, OBJECT_BG_4, 0, 256, 0, VRDEF_ACTIVE_A);

		/* 選択項目 */
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_1   , 0, 256, work->inactive[item_1], work->active[item_1]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_1 , 0, 256, work->inactive[item_1], work->active[item_1]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_10, 0, 256, work->inactive[item_1], work->active[item_1]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_2   , 0, 256, work->inactive[item_2], work->active[item_2]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_1 , 0, 256, work->inactive[item_2], work->active[item_2]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_10, 0, 256, work->inactive[item_2], work->active[item_2]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_3   , 0, 256, work->inactive[item_3], work->active[item_3]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_1 , 0, 256, work->inactive[item_3], work->active[item_3]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_10, 0, 256, work->inactive[item_3], work->active[item_3]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_4   , 0, 256, work->inactive[item_4], work->active[item_4]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_1 , 0, 256, work->inactive[item_4], work->active[item_4]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_10, 0, 256, work->inactive[item_4], work->active[item_4]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_5   , 0, 256, work->inactive[item_5], work->active[item_5]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_1 , 0, 256, work->inactive[item_5], work->active[item_5]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_10, 0, 256, work->inactive[item_5], work->active[item_5]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_6   , 0, 256, VRDEF_INACTIVE        , VRDEF_ACTIVE        );
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_7   , 0, 256, VRDEF_INACTIVE        , VRDEF_ACTIVE        );

		/* ハイスコア */
		SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1000000, 0, 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_100000 , 0, 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_10000  , 0, 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1000   , 0, 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_100    , 0, 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_10     , 0, 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1      , 0, 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1000000, 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_100000 , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_10000  , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1000   , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_100    , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_10     , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1      , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1000000, 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_100000 , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_10000  , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1000   , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_100    , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_10     , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1      , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
	}
	else if(LOM_GetCurMode(&work->layoutman) == LOM_MODE_01)
	{
#ifdef PAL
		if(work->count == 8) GM_SdSet(SD_S_WINOPN01);
#else
		if(work->count == 10) GM_SdSet(SD_S_WINOPN01);
#endif

		/* 背景 */
		SetSpriteColor(&work->layoutman, OBJECT_BG_1, 1, 256, VRDEF_ACTIVE, VRDEF_ACTIVE);
		SetSpriteColor(&work->layoutman, OBJECT_BG_2, 1, 256, VRDEF_ACTIVE, VRDEF_ACTIVE);
		SetSpriteColor(&work->layoutman, OBJECT_BG_3, 1, 256, VRDEF_ACTIVE, VRDEF_ACTIVE);
		SetSpriteColor(&work->layoutman, OBJECT_BG_4, 1, 256, VRDEF_ACTIVE, VRDEF_ACTIVE);
		SetSpriteAlpha(&work->layoutman, OBJECT_BG_1, 0, 256, 0, VRDEF_ACTIVE_A);
		SetSpriteAlpha(&work->layoutman, OBJECT_BG_2, 0, 256, 0, VRDEF_ACTIVE_A);
		SetSpriteAlpha(&work->layoutman, OBJECT_BG_3, 0, 256, 0, VRDEF_ACTIVE_A);
		SetSpriteAlpha(&work->layoutman, OBJECT_BG_4, 0, 256, 0, VRDEF_ACTIVE_A);

		/* 選択項目 */
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_1   , 0, 256, work->inactive[item_1], work->active[item_1]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_1 , 0, 256, work->inactive[item_1], work->active[item_1]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_10, 0, 256, work->inactive[item_1], work->active[item_1]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_2   , 0, 256, work->inactive[item_2], work->active[item_2]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_1 , 0, 256, work->inactive[item_2], work->active[item_2]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_10, 0, 256, work->inactive[item_2], work->active[item_2]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_3   , 0, 256, work->inactive[item_3], work->active[item_3]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_1 , 0, 256, work->inactive[item_3], work->active[item_3]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_10, 0, 256, work->inactive[item_3], work->active[item_3]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_4   , 0, 256, work->inactive[item_4], work->active[item_4]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_1 , 0, 256, work->inactive[item_4], work->active[item_4]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_10, 0, 256, work->inactive[item_4], work->active[item_4]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_5   , 0, 256, work->inactive[item_5], work->active[item_5]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_1 , 0, 256, work->inactive[item_5], work->active[item_5]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_10, 0, 256, work->inactive[item_5], work->active[item_5]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_6   , 0, 256, VRDEF_INACTIVE        , VRDEF_ACTIVE        );
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_7   , 0, 256, VRDEF_INACTIVE        , VRDEF_ACTIVE        );

		/* ハイスコア */
		SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1000000, 0, 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_100000 , 0, 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_10000  , 0, 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1000   , 0, 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_100    , 0, 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_10     , 0, 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1      , 0, 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
		SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1000000, 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_100000 , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_10000  , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1000   , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_100    , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_10     , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1      , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1000000, 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_100000 , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_10000  , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1000   , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_100    , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_10     , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1      , 0, 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);

		if((work->prev == 6) || (work->prev == 7))
		{
			ShowSprite(&work->layoutman, OBJECT_REC_RECORD, 0);
			ShowSprite(&work->layoutman, OBJECT_REC_BAR   , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_1ST   , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_2ND   , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_3RD   , 0);

			ShowSprite(&work->layoutman, OBJECT_REC_1ST_1000000, 0);
			ShowSprite(&work->layoutman, OBJECT_REC_1ST_100000 , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_1ST_10000  , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_1ST_1000   , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_1ST_100    , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_1ST_10     , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_1ST_1      , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_2ND_1000000, 0);
			ShowSprite(&work->layoutman, OBJECT_REC_2ND_100000 , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_2ND_10000  , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_2ND_1000   , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_2ND_100    , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_2ND_10     , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_2ND_1      , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_3RD_1000000, 0);
			ShowSprite(&work->layoutman, OBJECT_REC_3RD_100000 , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_3RD_10000  , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_3RD_1000   , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_3RD_100    , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_3RD_10     , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_3RD_1      , 0);
		}

#ifdef PAL
		if(work->count == 25)
#else
		if(work->count == 30)
#endif
		{
			switch(work->prev)
			{
			case 1:
				LOM_SetCurMode(&work->layoutman, LOM_MODE_07, 1, 1, 0);
				work->count = 0;
				LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_1, VRTRS_ICO_R, VRTRS_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
				break;
			case 2:
				LOM_SetCurMode(&work->layoutman, LOM_MODE_09, 1, 1, 0);
				work->count = 0;
				LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_2, 1);
				LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_2, VRTRS_ICO_R, VRTRS_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
				break;
			case 3:
				LOM_SetCurMode(&work->layoutman, LOM_MODE_12, 1, 1, 0);
				work->count = 0;
				LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_3, 1);
				LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_3, VRTRS_ICO_R, VRTRS_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
				break;
			case 4:
				LOM_SetCurMode(&work->layoutman, LOM_MODE_15, 1, 1, 0);
				work->count = 0;
				LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_4, 1);
				LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_4, VRTRS_ICO_R, VRTRS_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
				break;
			case 5:
				LOM_SetCurMode(&work->layoutman, LOM_MODE_19, 1, 1, 0);
				work->count = 0;
				LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_5, 1);
				LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_5, VRTRS_ICO_R, VRTRS_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
				break;
			case 6:
				LOM_SetCurMode(&work->layoutman, LOM_MODE_02, 1, 1, 0);
				work->count = 0;
				LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_6, 1);
				LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_6, VRTRS_ICO_R, VRTRS_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
				break;
			case 7:
				LOM_SetCurMode(&work->layoutman, LOM_MODE_02, 1, 1, 0);
				work->count = 0;
				LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_7, 1);
				LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_7, VRTRS_ICO_R, VRTRS_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
				break;
			default:
				LOM_SetCurMode(&work->layoutman, LOM_MODE_07, 1, 1, 0);
				work->count = 0;
				LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_1, VRTRS_ICO_R, VRTRS_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
				break;
			}
			work->flag |= VRTRS_INPUT_OK;
		}
	}
	else if(LOM_GetCurMode(&work->layoutman) == LOM_MODE_05)
	{
#ifdef PAL
		if(work->count == 8) GM_SdSet(SD_S_LINEMOV1);
#else
		if(work->count == 10) GM_SdSet(SD_S_LINEMOV1);
#endif

		/* 背景 */
		if(work->prev != 6)
		{
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_1, 0, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_2, 0, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_3, 0, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_4, 0, 16, 0, VRDEF_ACTIVE_A);

			if(work->prev != 1)        ShowSprite(&work->layoutman, OBJECT_BG_1, 0);
			if(  !((work->prev == 2)
				|| (work->prev == 5))) ShowSprite(&work->layoutman, OBJECT_BG_2, 0);
			if(work->prev != 3)        ShowSprite(&work->layoutman, OBJECT_BG_3, 0);
			if(work->prev != 4)        ShowSprite(&work->layoutman, OBJECT_BG_4, 0);
		}

		/* 選択項目 */
		if(work->prev == 1)
		{
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1   , 1, 256, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_1 , 1, 256, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_10, 1, 256, work->inactive[item_1], work->active[item_1]);

			SetScores(work, VRDEF_TRIAL_1);
			clear[0] = MSN_HISCORE(VRDEF_TRIAL_1, 1) & MSN_OVERWRITE ? 1 : 0;
			clear[1] = MSN_HISCORE(VRDEF_TRIAL_1, 2) & MSN_OVERWRITE ? 1 : 0;
			clear[2] = MSN_HISCORE(VRDEF_TRIAL_1, 3) & MSN_OVERWRITE ? 1 : 0;
		}
		else
		{
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1   , 0, 256, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_1 , 0, 256, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_10, 0, 256, work->inactive[item_1], work->active[item_1]);
		}
		if(work->prev == 2)
		{
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2   , 1, 256, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_1 , 1, 256, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_10, 1, 256, work->inactive[item_2], work->active[item_2]);

			SetScores(work, VRDEF_TRIAL_2);
			clear[0] = MSN_HISCORE(VRDEF_TRIAL_2, 1) & MSN_OVERWRITE ? 1 : 0;
			clear[1] = MSN_HISCORE(VRDEF_TRIAL_2, 2) & MSN_OVERWRITE ? 1 : 0;
			clear[2] = MSN_HISCORE(VRDEF_TRIAL_2, 3) & MSN_OVERWRITE ? 1 : 0;
		}
		else
		{
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2   , 0, 256, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_1 , 0, 256, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_10, 0, 256, work->inactive[item_2], work->active[item_2]);
		}
		if(work->prev == 3)
		{
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3   , 1, 256, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_1 , 1, 256, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_10, 1, 256, work->inactive[item_3], work->active[item_3]);

			SetScores(work, VRDEF_TRIAL_3);
			clear[0] = MSN_HISCORE(VRDEF_TRIAL_3, 1) & MSN_OVERWRITE ? 1 : 0;
			clear[1] = MSN_HISCORE(VRDEF_TRIAL_3, 2) & MSN_OVERWRITE ? 1 : 0;
			clear[2] = MSN_HISCORE(VRDEF_TRIAL_3, 3) & MSN_OVERWRITE ? 1 : 0;
		}
		else
		{
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3   , 0, 256, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_1 , 0, 256, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_10, 0, 256, work->inactive[item_3], work->active[item_3]);
		}
		if(work->prev == 4)
		{
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4   , 1, 256, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_1 , 1, 256, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_10, 1, 256, work->inactive[item_4], work->active[item_4]);

			SetScores(work, VRDEF_TRIAL_4);
			clear[0] = MSN_HISCORE(VRDEF_TRIAL_4, 1) & MSN_OVERWRITE ? 1 : 0;
			clear[1] = MSN_HISCORE(VRDEF_TRIAL_4, 2) & MSN_OVERWRITE ? 1 : 0;
			clear[2] = MSN_HISCORE(VRDEF_TRIAL_4, 3) & MSN_OVERWRITE ? 1 : 0;
		}
		else
		{
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4   , 0, 256, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_1 , 0, 256, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_10, 0, 256, work->inactive[item_4], work->active[item_4]);
		}
		if(work->prev == 5)
		{
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5   , 1, 256, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_1 , 1, 256, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_10, 1, 256, work->inactive[item_5], work->active[item_5]);

			SetScores(work, VRDEF_TRIAL_5);
			clear[0] = MSN_HISCORE(VRDEF_TRIAL_5, 1) & MSN_OVERWRITE ? 1 : 0;
			clear[1] = MSN_HISCORE(VRDEF_TRIAL_5, 2) & MSN_OVERWRITE ? 1 : 0;
			clear[2] = MSN_HISCORE(VRDEF_TRIAL_5, 3) & MSN_OVERWRITE ? 1 : 0;
		}
		else
		{
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5   , 0, 256, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_1 , 0, 256, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_10, 0, 256, work->inactive[item_5], work->active[item_5]);
		}
		if((work->prev == 6) || (work->prev == 7))
		{
			if(work->prev == 6)
			{
				SetSpriteColor(&work->layoutman, OBJECT_ITEM_6, 1, 256, VRDEF_INACTIVE, VRDEF_ACTIVE);
				SetSpriteColor(&work->layoutman, OBJECT_ITEM_7, 0, 256, VRDEF_INACTIVE, VRDEF_ACTIVE);
			}
			else
			{
				SetSpriteColor(&work->layoutman, OBJECT_ITEM_6, 0, 256, VRDEF_INACTIVE, VRDEF_ACTIVE);
				SetSpriteColor(&work->layoutman, OBJECT_ITEM_7, 1, 256, VRDEF_INACTIVE, VRDEF_ACTIVE);
			}

			ShowSprite(&work->layoutman, OBJECT_REC_RECORD     , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_BAR        , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_1ST        , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_2ND        , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_3RD        , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_1ST_1000000, 0);
			ShowSprite(&work->layoutman, OBJECT_REC_1ST_100000 , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_1ST_10000  , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_1ST_1000   , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_1ST_100    , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_1ST_10     , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_1ST_1      , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_2ND_1000000, 0);
			ShowSprite(&work->layoutman, OBJECT_REC_2ND_100000 , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_2ND_10000  , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_2ND_1000   , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_2ND_100    , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_2ND_10     , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_2ND_1      , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_3RD_1000000, 0);
			ShowSprite(&work->layoutman, OBJECT_REC_3RD_100000 , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_3RD_10000  , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_3RD_1000   , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_3RD_100    , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_3RD_10     , 0);
			ShowSprite(&work->layoutman, OBJECT_REC_3RD_1      , 0);
		}
		else
		{
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_6, 0, 256, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_7, 0, 256, VRDEF_INACTIVE, VRDEF_ACTIVE);

			/* ハイスコア */
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1000000, clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_100000 , clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_10000  , clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1000   , clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_100    , clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_10     , clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1      , clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1000000, clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_100000 , clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_10000  , clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1000   , clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_100    , clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_10     , clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1      , clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1000000, clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_100000 , clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_10000  , clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1000   , clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_100    , clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_10     , clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1      , clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
		}
	}
	else
	{
		/* カーソルの位置に対する処理 */
		switch(LOM_GetCurObj(&work->layoutman))
		{
		case OBJECT_ITEM_1:
			work->prev = 1;

			/* 背景 */
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_1, 1, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_2, 0, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_3, 0, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_4, 0, 16, 0, VRDEF_ACTIVE_A);

			/* 選択項目 */
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1   , 1, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_1 , 1, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_10, 1, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2   , 0, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_1 , 0, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_10, 0, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3   , 0, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_1 , 0, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_10, 0, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4   , 0, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_1 , 0, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_10, 0, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5   , 0, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_1 , 0, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_10, 0, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_6   , 0, 16, VRDEF_INACTIVE        , VRDEF_ACTIVE        );
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_7   , 0, 16, VRDEF_INACTIVE        , VRDEF_ACTIVE        );

			/* ハイスコア */
			SetScores(work, VRDEF_TRIAL_1);
			clear[0] = MSN_HISCORE(VRDEF_TRIAL_1, 1) & MSN_OVERWRITE ? 1 : 0;
			clear[1] = MSN_HISCORE(VRDEF_TRIAL_1, 2) & MSN_OVERWRITE ? 1 : 0;
			clear[2] = MSN_HISCORE(VRDEF_TRIAL_1, 3) & MSN_OVERWRITE ? 1 : 0;

			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1000000, clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_100000 , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_10000  , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1000   , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_100    , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_10     , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1      , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1000000, clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_100000 , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_10000  , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1000   , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_100    , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_10     , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1      , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1000000, clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_100000 , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_10000  , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1000   , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_100    , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_10     , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1      , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			break;

		case OBJECT_ITEM_2:
			work->prev = 2;

			/* 背景 */
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_1, 0, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_2, 1, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_3, 0, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_4, 0, 16, 0, VRDEF_ACTIVE_A);

			/* 選択項目 */
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1   , 0, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_1 , 0, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_10, 0, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2   , 1, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_1 , 1, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_10, 1, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3   , 0, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_1 , 0, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_10, 0, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4   , 0, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_1 , 0, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_10, 0, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5   , 0, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_1 , 0, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_10, 0, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_6   , 0, 16, VRDEF_INACTIVE        , VRDEF_ACTIVE        );
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_7   , 0, 16, VRDEF_INACTIVE        , VRDEF_ACTIVE        );

			/* ハイスコア */
			SetScores(work, VRDEF_TRIAL_2);
			clear[0] = MSN_HISCORE(VRDEF_TRIAL_2, 1) & MSN_OVERWRITE ? 1 : 0;
			clear[1] = MSN_HISCORE(VRDEF_TRIAL_2, 2) & MSN_OVERWRITE ? 1 : 0;
			clear[2] = MSN_HISCORE(VRDEF_TRIAL_2, 3) & MSN_OVERWRITE ? 1 : 0;

			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1000000, clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_100000 , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_10000  , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1000   , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_100    , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_10     , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1      , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1000000, clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_100000 , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_10000  , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1000   , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_100    , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_10     , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1      , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1000000, clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_100000 , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_10000  , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1000   , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_100    , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_10     , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1      , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			break;

		case OBJECT_ITEM_3:
			work->prev = 3;

			/* 背景 */
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_1, 0, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_2, 0, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_3, 1, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_4, 0, 16, 0, VRDEF_ACTIVE_A);

			/* 選択項目 */
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1   , 0, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_1 , 0, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_10, 0, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2   , 0, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_1 , 0, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_10, 0, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3   , 1, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_1 , 1, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_10, 1, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4   , 0, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_1 , 0, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_10, 0, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5   , 0, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_1 , 0, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_10, 0, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_6   , 0, 16, VRDEF_INACTIVE        , VRDEF_ACTIVE        );
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_7   , 0, 16, VRDEF_INACTIVE        , VRDEF_ACTIVE        );

			/* ハイスコア */
			SetScores(work, VRDEF_TRIAL_3);
			clear[0] = MSN_HISCORE(VRDEF_TRIAL_3, 1) & MSN_OVERWRITE ? 1 : 0;
			clear[1] = MSN_HISCORE(VRDEF_TRIAL_3, 2) & MSN_OVERWRITE ? 1 : 0;
			clear[2] = MSN_HISCORE(VRDEF_TRIAL_3, 3) & MSN_OVERWRITE ? 1 : 0;

			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1000000, clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_100000 , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_10000  , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1000   , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_100    , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_10     , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1      , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1000000, clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_100000 , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_10000  , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1000   , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_100    , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_10     , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1      , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1000000, clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_100000 , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_10000  , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1000   , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_100    , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_10     , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1      , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			break;

		case OBJECT_ITEM_4:
			work->prev = 4;

			/* 背景 */
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_1, 0, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_2, 0, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_3, 0, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_4, 1, 16, 0, VRDEF_ACTIVE_A);

			/* 選択項目 */
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1   , 0, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_1 , 0, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_10, 0, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2   , 0, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_1 , 0, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_10, 0, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3   , 0, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_1 , 0, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_10, 0, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4   , 1, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_1 , 1, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_10, 1, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5   , 0, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_1 , 0, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_10, 0, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_6   , 0, 16, VRDEF_INACTIVE        , VRDEF_ACTIVE        );
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_7   , 0, 16, VRDEF_INACTIVE        , VRDEF_ACTIVE        );

			/* ハイスコア */
			SetScores(work, VRDEF_TRIAL_4);
			clear[0] = MSN_HISCORE(VRDEF_TRIAL_4, 1) & MSN_OVERWRITE ? 1 : 0;
			clear[1] = MSN_HISCORE(VRDEF_TRIAL_4, 2) & MSN_OVERWRITE ? 1 : 0;
			clear[2] = MSN_HISCORE(VRDEF_TRIAL_4, 3) & MSN_OVERWRITE ? 1 : 0;

			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1000000, clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_100000 , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_10000  , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1000   , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_100    , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_10     , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1      , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1000000, clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_100000 , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_10000  , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1000   , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_100    , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_10     , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1      , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1000000, clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_100000 , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_10000  , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1000   , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_100    , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_10     , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1      , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			break;

		case OBJECT_ITEM_5:
			work->prev = 5;

			/* 背景 */
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_1, 0, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_2, 1, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_3, 0, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_4, 0, 16, 0, VRDEF_ACTIVE_A);

			/* 選択項目 */
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1   , 0, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_1 , 0, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_10, 0, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2   , 0, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_1 , 0, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_10, 0, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3   , 0, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_1 , 0, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_10, 0, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4   , 0, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_1 , 0, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_10, 0, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5   , 1, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_1 , 1, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_10, 1, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_6   , 0, 16, VRDEF_INACTIVE        , VRDEF_ACTIVE        );
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_7   , 0, 16, VRDEF_INACTIVE        , VRDEF_ACTIVE        );

			/* ハイスコア */
			SetScores(work, VRDEF_TRIAL_5);
			clear[0] = MSN_HISCORE(VRDEF_TRIAL_5, 1) & MSN_OVERWRITE ? 1 : 0;
			clear[1] = MSN_HISCORE(VRDEF_TRIAL_5, 2) & MSN_OVERWRITE ? 1 : 0;
			clear[2] = MSN_HISCORE(VRDEF_TRIAL_5, 3) & MSN_OVERWRITE ? 1 : 0;

			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1000000, clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_100000 , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_10000  , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1000   , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_100    , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_10     , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1      , clear[0], 16, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1000000, clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_100000 , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_10000  , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1000   , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_100    , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_10     , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1      , clear[1], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1000000, clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_100000 , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_10000  , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1000   , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_100    , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_10     , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1      , clear[2], 16, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			break;

		case OBJECT_ITEM_6:

			/* 背景 */
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_1, 0, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_2, 0, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_3, 0, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_4, 0, 16, 0, VRDEF_ACTIVE_A);

			/* 選択項目 */
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1   , 0, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_1 , 0, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_10, 0, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2   , 0, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_1 , 0, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_10, 0, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3   , 0, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_1 , 0, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_10, 0, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4   , 0, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_1 , 0, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_10, 0, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5   , 0, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_1 , 0, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_10, 0, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_6   , 1, 16, VRDEF_INACTIVE        , VRDEF_ACTIVE        );
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_7   , 0, 16, VRDEF_INACTIVE        , VRDEF_ACTIVE        );

			/* ハイスコア */
			if(work->prev == 1)
			{
				SetScores(work, VRDEF_TRIAL_1);
				clear[0] = MSN_HISCORE(VRDEF_TRIAL_1, 1) & MSN_OVERWRITE ? 1 : 0;
				clear[1] = MSN_HISCORE(VRDEF_TRIAL_1, 2) & MSN_OVERWRITE ? 1 : 0;
				clear[2] = MSN_HISCORE(VRDEF_TRIAL_1, 3) & MSN_OVERWRITE ? 1 : 0;
			}
			else if(work->prev == 2)
			{
				SetScores(work, VRDEF_TRIAL_2);
				clear[0] = MSN_HISCORE(VRDEF_TRIAL_2, 1) & MSN_OVERWRITE ? 1 : 0;
				clear[1] = MSN_HISCORE(VRDEF_TRIAL_2, 2) & MSN_OVERWRITE ? 1 : 0;
				clear[2] = MSN_HISCORE(VRDEF_TRIAL_2, 3) & MSN_OVERWRITE ? 1 : 0;
			}
			else if(work->prev == 3)
			{
				SetScores(work, VRDEF_TRIAL_3);
				clear[0] = MSN_HISCORE(VRDEF_TRIAL_3, 1) & MSN_OVERWRITE ? 1 : 0;
				clear[1] = MSN_HISCORE(VRDEF_TRIAL_3, 2) & MSN_OVERWRITE ? 1 : 0;
				clear[2] = MSN_HISCORE(VRDEF_TRIAL_3, 3) & MSN_OVERWRITE ? 1 : 0;
			}
			else if(work->prev == 4)
			{
				SetScores(work, VRDEF_TRIAL_4);
				clear[0] = MSN_HISCORE(VRDEF_TRIAL_4, 1) & MSN_OVERWRITE ? 1 : 0;
				clear[1] = MSN_HISCORE(VRDEF_TRIAL_4, 2) & MSN_OVERWRITE ? 1 : 0;
				clear[2] = MSN_HISCORE(VRDEF_TRIAL_4, 3) & MSN_OVERWRITE ? 1 : 0;
			}
			else if(work->prev == 5)
			{
				SetScores(work, VRDEF_TRIAL_5);
				clear[0] = MSN_HISCORE(VRDEF_TRIAL_5, 1) & MSN_OVERWRITE ? 1 : 0;
				clear[1] = MSN_HISCORE(VRDEF_TRIAL_5, 2) & MSN_OVERWRITE ? 1 : 0;
				clear[2] = MSN_HISCORE(VRDEF_TRIAL_5, 3) & MSN_OVERWRITE ? 1 : 0;
			}

			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1000000, clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_100000 , clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_10000  , clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1000   , clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_100    , clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_10     , clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1      , clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1000000, clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_100000 , clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_10000  , clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1000   , clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_100    , clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_10     , clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1      , clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1000000, clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_100000 , clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_10000  , clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1000   , clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_100    , clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_10     , clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1      , clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			break;

		case OBJECT_ITEM_7:

			/* 背景 */
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_1, 0, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_2, 0, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_3, 0, 16, 0, VRDEF_ACTIVE_A);
			SetSpriteAlpha(&work->layoutman, OBJECT_BG_4, 0, 16, 0, VRDEF_ACTIVE_A);

			/* 選択項目 */
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1   , 0, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_1 , 0, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1_10, 0, 16, work->inactive[item_1], work->active[item_1]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2   , 0, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_1 , 0, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2_10, 0, 16, work->inactive[item_2], work->active[item_2]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3   , 0, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_1 , 0, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3_10, 0, 16, work->inactive[item_3], work->active[item_3]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4   , 0, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_1 , 0, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4_10, 0, 16, work->inactive[item_4], work->active[item_4]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5   , 0, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_1 , 0, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5_10, 0, 16, work->inactive[item_5], work->active[item_5]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_6   , 0, 16, VRDEF_INACTIVE        , VRDEF_ACTIVE        );
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_7   , 1, 16, VRDEF_INACTIVE        , VRDEF_ACTIVE        );

			/* ハイスコア */
			if(work->prev == 1)
			{
				SetScores(work, VRDEF_TRIAL_1);
				clear[0] = MSN_HISCORE(VRDEF_TRIAL_1, 1) & MSN_OVERWRITE ? 1 : 0;
				clear[1] = MSN_HISCORE(VRDEF_TRIAL_1, 2) & MSN_OVERWRITE ? 1 : 0;
				clear[2] = MSN_HISCORE(VRDEF_TRIAL_1, 3) & MSN_OVERWRITE ? 1 : 0;
			}
			else if(work->prev == 2)
			{
				SetScores(work, VRDEF_TRIAL_2);
				clear[0] = MSN_HISCORE(VRDEF_TRIAL_2, 1) & MSN_OVERWRITE ? 1 : 0;
				clear[1] = MSN_HISCORE(VRDEF_TRIAL_2, 2) & MSN_OVERWRITE ? 1 : 0;
				clear[2] = MSN_HISCORE(VRDEF_TRIAL_2, 3) & MSN_OVERWRITE ? 1 : 0;
			}
			else if(work->prev == 3)
			{
				SetScores(work, VRDEF_TRIAL_3);
				clear[0] = MSN_HISCORE(VRDEF_TRIAL_3, 1) & MSN_OVERWRITE ? 1 : 0;
				clear[1] = MSN_HISCORE(VRDEF_TRIAL_3, 2) & MSN_OVERWRITE ? 1 : 0;
				clear[2] = MSN_HISCORE(VRDEF_TRIAL_3, 3) & MSN_OVERWRITE ? 1 : 0;
			}
			else if(work->prev == 4)
			{
				SetScores(work, VRDEF_TRIAL_4);
				clear[0] = MSN_HISCORE(VRDEF_TRIAL_4, 1) & MSN_OVERWRITE ? 1 : 0;
				clear[1] = MSN_HISCORE(VRDEF_TRIAL_4, 2) & MSN_OVERWRITE ? 1 : 0;
				clear[2] = MSN_HISCORE(VRDEF_TRIAL_4, 3) & MSN_OVERWRITE ? 1 : 0;
			}
			else if(work->prev == 5)
			{
				SetScores(work, VRDEF_TRIAL_5);
				clear[0] = MSN_HISCORE(VRDEF_TRIAL_5, 1) & MSN_OVERWRITE ? 1 : 0;
				clear[1] = MSN_HISCORE(VRDEF_TRIAL_5, 2) & MSN_OVERWRITE ? 1 : 0;
				clear[2] = MSN_HISCORE(VRDEF_TRIAL_5, 3) & MSN_OVERWRITE ? 1 : 0;
			}

			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1000000, clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_100000 , clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_10000  , clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1000   , clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_100    , clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_10     , clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_1ST_1      , clear[0], 256, VRDEF_INACTIVE2, VRDEF_A_1STPLAC);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1000000, clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_100000 , clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_10000  , clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1000   , clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_100    , clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_10     , clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_2ND_1      , clear[1], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1000000, clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_100000 , clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_10000  , clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1000   , clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_100    , clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_10     , clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			SetSpriteColor(&work->layoutman, OBJECT_REC_3RD_1      , clear[2], 256, VRDEF_INACTIVE2, VRDEF_A_CLEARED);
			break;

		default:
			break;
		}
#ifdef PAL
		if(work->count2 == 7)
#else
		if(work->count2 == 8)
#endif
			work->flag |= VRTRS_INPUT_OK;
	}

	if(work->count > 1000)
		work->count = 1000;
#endif
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

	/* 多重起動防止 */
	_work = NULL;
}

/******************************************************************************
 * 初期化処理

chara ＶＲ体験版セレクト[NewVRTrialSelect_Scn] $s:名前 \
	-proc $p:ステージロードプロック ... \
	-save $p:ＳＡＶＥプロック \
	-exit $p:ＥＸＩＴプロック \
	-init // フラグ初期化
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	WORK *work,		/* ワーク */
	int  name,		/* シナリオ名 */
	int  where)		/* マップＩＤ */
{
	int i;

	SY_PRINTF3("GetResources\n");

	memset(work->proc, 0x00, sizeof(work->proc));
	work->name   = name;
	work->tex_u0 = 0.0f;
	work->index  = -1;
	work->flag   = 0;
	work->count  = 0;
	work->prev   = 0;
	work->count2 = 0;

	/* テキストの色の準備 */
	work->inactive[0] = VRDEF_INACTIVE;
	work->inactive[1] = VRDEF_I_1STPLAC;
	work->inactive[2] = VRDEF_I_CLEARED;
	work->inactive[3] = VRDEF_I_CLEARED;
	work->inactive[4] = VRDEF_INACTIVE2;
	work->inactive[5] = VRDEF_I_UNPABLE;

	work->inactive_a[0] = VRDEF_INACTIVE_A;
	work->inactive_a[1] = VRDEF_I_1STPLAC_A;
	work->inactive_a[2] = VRDEF_I_CLEARED_A;
	work->inactive_a[3] = VRDEF_I_CLEARED_A;
	work->inactive_a[4] = VRDEF_INACTIVE2_A;
	work->inactive_a[5] = VRDEF_I_UNPABLE_A;

	work->active[0]   = VRDEF_ACTIVE;
	work->active[1]   = VRDEF_A_1STPLAC;
	work->active[2]   = VRDEF_A_CLEARED;
	work->active[3]   = VRDEF_A_CLEARED;
	work->active[4]   = VRDEF_ACTIVE;

	work->active_a[0]   = VRDEF_ACTIVE_A;
	work->active_a[1]   = VRDEF_A_1STPLAC_A;
	work->active_a[2]   = VRDEF_A_CLEARED_A;
	work->active_a[3]   = VRDEF_A_CLEARED_A;
	work->active_a[4]   = VRDEF_ACTIVE_A;

	/* シナリオオプション */
	if(name != 0)
	{
		/* proc */
		if(GCL_GetOption('p') == NULL)
		{
			SY_PRINTF2("no proc parameter.\n");
			return 0;
		}
		for(i = 0; i < 5; i++)
			work->proc[i] = GCL_GetNextInt();

		/* save */
		if(GCL_GetOption('s') != NULL)
			work->proc[5] = GCL_GetNextInt();

		/* exit */
		if(GCL_GetOption('e') != NULL)
			work->proc[6] = GCL_GetNextInt();

		/* init */
		if(GCL_GetOption('i') != NULL)
			Msn_SetDefaultData();

		SY_PRINTF1("Get Scn Option Succeeded.\n");
	}

	/* レイアウトマネージャ */
	{
#ifdef PAL
		if(CreateLayoutman3(&work->layoutman, L2D_FILENAME, 4, 0, 0, 0, 0, LOMCallback, NULL, work, 17, 9, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
#else
		if(CreateLayoutman3(&work->layoutman, L2D_FILENAME, 4, 0, 0, 0, 0, LOMCallback, NULL, work, 20, 10, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
#endif
		SY_PRINTF1("Create Layoutman Succeeded.\n");
		if(LoadLOMData(&work->layoutman, _lom_data) == 0) { SY_PRINTF2("Load LOM Data Failed.\n"); return 0; }
		SY_PRINTF1("Load LOM Data Succeeded.\n");
	}

	work->flag |= VRTRS_INIT;

	/* ポーズ無効 */
	GM_GameStatus |= STATE_PAUSE_DISABLE;

	/* 多重起動防止 */
	_work = work;

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
void *NewVRTrialSelect(void)
{
	WORK *work;

	SY_PRINTF3("NewVRTrialSelect\n");
	OPERATOR();

	/* 多重起動防止 */
	if(_work != NULL)
		return(_work);

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

void *NewVRTrialSelect_Scn(
	int name,	/* シナリオ名 */
	int where)	/* マップＩＤ */
{
	WORK *work;

	SY_PRINTF3("NewVRTrialSelect_Scn\n");
	OPERATOR();

	/* 多重起動防止 */
	if(_work != NULL)
		return(_work);

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
