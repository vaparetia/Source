//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * vr_clear_code - vr_clear_code.c
 * ＶＲクリアコード  *NewVRClearCode
 * 2002/08/11 S.Yamashita
 * $Id: vr_clear_code.c,v 1.3 2002/11/23 12:46:56 Yoshizawa1 Exp $
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

#define __CHARA_NAME__ "VR Clear Code"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
//#define SY_PRINTF3_DEBUG
#include "../sy_util/sy_util.h"

/*******************************************************************************
 * defines
 */

/* フラグ */
#define VRCLC_INIT  (0x0001)	/* 初期化時 */
#define VRCLC_INIT2 (0x0002)	/* 初期化時 */

#define VRCLC_TEX_HEIGHT (12)	/* テクスチャ高さ */
#define VRCLC_TEX_WIDTH  (18)	/* テクスチャ幅 */

/* Ｌ２Ｄオブジェクト */
#ifdef KP_XBOX
#define L2D_FILENAME       (16393367)	/* vr_clear_code */
#else
#define L2D_FILENAME       (11595995)	/* vr_clear_code_ps2 */
#endif

#define ACTION_BLINK       ( 1690157)	/* blinkDot */
#define ACTION_RAI_VR_OPE  (10565927)	/* RaidenOpenVRClearCode */
#define ACTION_RAI_VR_CLO  (13427018)	/* RaidenCloseVRClearCode */
#define ACTION_RAI_SNE_OPE ( 7440304)	/* RaidenOpenVRClearCodeSneaking */
#define ACTION_RAI_SNE_CLO ( 9745240)	/* RaidenCloseVRClearCodeSneaking */
#define ACTION_RAI_WEA_OPE (13670634)	/* RaidenOpenVRClearCodeWeapon */
#define ACTION_RAI_WEA_CLO (12231093)	/* RaidenCloseVRClearCodeWeapon */
#define ACTION_NIN_VR_OPE  ( 6000716)	/* RaidenNinOpenVRClearCode */
#define ACTION_NIN_VR_CLO  ( 1557986)	/* RaidenNinCloseVRClearCode */
#define ACTION_NIN_SNE_OPE ( 9847303)	/* RaidenNinOpenVRClearCodeSneaking */
#define ACTION_NIN_SNE_CLO ( 2883133)	/* RaidenNinCloseVRClearCodeSneaking */
#define ACTION_NIN_WEA_OPE ( 6709785)	/* RaidenNinOpenVRClearCodeWeapon */
#define ACTION_NIN_WEA_CLO ( 7587720)	/* RaidenNinCloseVRClearCodeWeapon */
#define ACTION_X_R_VR_OPE  (10610983)	/* XRaidenOpenVRClearCode */
#define ACTION_X_R_VR_CLO  (14868810)	/* XRaidenCloseVRClearCode */
#define ACTION_SNA_VR_OPE  (11852216)	/* SnakeOpenVRClearCode */
#define ACTION_SNA_VR_CLO  ( 4256621)	/* SnakeCloseVRClearCode */
#define ACTION_SNA_SNE_OPE (  170833)	/* SnakeOpenVRClearCodeSneaking */
#define ACTION_SNA_SNE_CLO (12003178)	/* SnakeCloseVRClearCodeSneaking */
#define ACTION_SNA_WEA_OPE (12107055)	/* SnakeOpenVRClearCodeWeapon */
#define ACTION_SNA_WEA_CLO (12528210)	/* SnakeCloseVRClearCodeWeapon */
#define ACTION_PLI_VR_OPE  (13287115)	/* PliskinOpenVRClearCode */
#define ACTION_PLI_VR_CLO  (16618959)	/* PliskinCloseVRClearCode */
#define ACTION_PLI_SNE_OPE ( 1421622)	/* PliskinOpenVRClearCodeSneaking */
#define ACTION_PLI_SNE_CLO ( 1696781)	/* PliskinCloseVRClearCodeSneaking */
#define ACTION_PLI_WEA_OPE ( 3277301)	/* PliskinOpenVRClearCodeWeapon */
#define ACTION_PLI_WEA_CLO (15188737)	/* PliskinCloseVRClearCodeWeapon */
#define ACTION_TUX_VR_OPE  (   60715)	/* SnakeTuxedoOpenVRClearCode */
#define ACTION_TUX_VR_CLO  (12804534)	/* SnakeTuxedoCloseVRClearCode */
#define ACTION_TUX_SNE_OPE ( 7661412)	/* SnakeTuxedoOpenVRClearCodeSneaking */
#define ACTION_TUX_SNE_CLO (   43481)	/* SnakeTuxedoCloseVRClearCodeSneaking */
#define ACTION_TUX_WEA_OPE (12425666)	/* SnakeTuxedoOpenVRClearCodeWeapon */
#define ACTION_TUX_WEA_CLO ( 5946547)	/* SnakeTuxedoCloseVRClearCodeWeapon */
#define ACTION_PRE_VR_OPE  (15067347)	/* SnakeMgs1OpenVRClearCode */
#define ACTION_PRE_VR_CLO  ( 6477523)	/* SnakeMgs1CloseVRClearCode */
#define ACTION_PRE_SNE_OPE ( 1952864)	/* SnakeMgs1OpenVRClearCodeSneaking */
#define ACTION_PRE_SNE_CLO ( 1919310)	/* SnakeMgs1CloseVRClearCodeSneaking */
#define ACTION_PRE_WEA_OPE (16548859)	/* SnakeMgs1OpenVRClearCodeWeapon */
#define ACTION_PRE_WEA_CLO ( 3671003)	/* SnakeMgs1CloseVRClearCodeWeapon */

#define OBJECT_MISSION (6933856)	/* missions */
#define OBJECT_MODE    (3688677)	/* mode */
#define OBJECT_WEAPON  (8539970)	/* weapon */
#define OBJECT_LEVEL10 (1851416)	/* num_1 */
#define OBJECT_LEVEL01 (1851417)	/* num_2 */
#define OBJECT_FONT_01 ( 247775)	/* code_font_01 */
#define OBJECT_FONT_02 ( 247776)	/* code_font_02 */
#define OBJECT_FONT_03 ( 247777)	/* code_font_03 */
#define OBJECT_FONT_04 ( 247778)	/* code_font_04 */
#define OBJECT_FONT_05 ( 247779)	/* code_font_05 */
#define OBJECT_FONT_06 ( 247780)	/* code_font_06 */
#define OBJECT_FONT_07 ( 247781)	/* code_font_07 */
#define OBJECT_FONT_08 ( 247782)	/* code_font_08 */
#define OBJECT_FONT_09 ( 247783)	/* code_font_09 */
#define OBJECT_FONT_10 ( 247806)	/* code_font_10 */
#define OBJECT_FONT_11 ( 247807)	/* code_font_11 */
#define OBJECT_FONT_12 ( 247808)	/* code_font_12 */
#define OBJECT_FONT_13 ( 247809)	/* code_font_13 */
#define OBJECT_FONT_14 ( 247810)	/* code_font_14 */
#define OBJECT_FONT_15 ( 247811)	/* code_font_15 */
#define OBJECT_FONT_16 ( 247812)	/* code_font_16 */
#define OBJECT_FONT_17 ( 247813)	/* code_font_17 */
#define OBJECT_FONT_18 ( 247814)	/* code_font_18 */
#define OBJECT_FONT_19 ( 247815)	/* code_font_19 */
#define OBJECT_FONT_20 ( 247838)	/* code_font_20 */

/*******************************************************************************
 * work
 */

/* ワーク */
typedef struct tagWORK
{
	GV_ACT_EX actor;			/* アクター */
	LAYOUTMAN layoutman;		/* レイアウトマネージャ */

	int       stage;			/* ステージＩＤ */
	int       *status;			/* 起動状態 */
	float     tex_mis_v0;		/* ミッションテクスチャのＶ値の原点 */
	float     tex_mod_v0;		/* モードテクスチャのＶ値の原点 */

	float     tex_wea_v0;		/* 武器テクスチャのＶ値の原点 */
	float     tex_lev_u0;		/* レベルテクスチャのＵ値の原点 */
	float     tex_fon_u0;		/* フォントテクスチャのＵ値の原点 */
	short     flag;				/* フラグ */
	char      mission;			/* ミッション */
	char      mode;				/* モード */

	char      weapon;			/* 武器 */
	char      level;			/* レベル */
	char      player;			/* プレイヤー */

	char      clear_code[32];	/* クリアコード */
}
WORK;

/*******************************************************************************
 * local
 */

/* ＬＯＭデータ */
static int _lom_data[] = {
					/*アクションリスト名*/		/*アクション名*/		/*カウント*/	/*フラグ*/
LOM_ACTLIST_DATA,	LOM_ACTLIST_00,				ACTION_RAI_VR_OPE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_01,				ACTION_RAI_SNE_OPE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_02,				ACTION_RAI_WEA_OPE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_03,				ACTION_NIN_VR_OPE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_04,				ACTION_NIN_SNE_OPE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_05,				ACTION_NIN_WEA_OPE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_06,				ACTION_X_R_VR_OPE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_07,				ACTION_SNA_VR_OPE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_08,				ACTION_SNA_SNE_OPE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_09,				ACTION_SNA_WEA_OPE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_10,				ACTION_PLI_VR_OPE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_11,				ACTION_PLI_SNE_OPE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_12,				ACTION_PLI_WEA_OPE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_13,				ACTION_TUX_VR_OPE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_14,				ACTION_TUX_SNE_OPE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_15,				ACTION_TUX_WEA_OPE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_16,				ACTION_PRE_VR_OPE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_17,				ACTION_PRE_SNE_OPE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_18,				ACTION_PRE_WEA_OPE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			-1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_20,				ACTION_RAI_VR_CLO,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_21,				ACTION_RAI_SNE_CLO,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_22,				ACTION_RAI_WEA_CLO,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_23,				ACTION_NIN_VR_CLO,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_24,				ACTION_NIN_SNE_CLO,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_25,				ACTION_NIN_WEA_CLO,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_26,				ACTION_X_R_VR_CLO,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_27,				ACTION_SNA_VR_CLO,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_28,				ACTION_SNA_SNE_CLO,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_29,				ACTION_SNA_WEA_CLO,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_30,				ACTION_PLI_VR_CLO,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_31,				ACTION_PLI_SNE_CLO,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_32,				ACTION_PLI_WEA_CLO,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_33,				ACTION_TUX_VR_CLO,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_34,				ACTION_TUX_SNE_CLO,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_35,				ACTION_TUX_WEA_CLO,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_36,				ACTION_PRE_VR_CLO,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_37,				ACTION_PRE_SNE_CLO,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_38,				ACTION_PRE_WEA_CLO,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_BLINK,			1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_DATA_END,

					/*パッドリスト名*/			/*アクション名*/		/*オブジェクト名*/	/*入力*/			/*実行番号*/		/*実行引数*/	/*フラグ*/
LOM_PADLIST_DATA,	LOM_PADLIST_00,				ACTION_BLINK,			LOM_PAD_ALLOBJ,		LOM_PAD_CANCEL,			LOM_EXE_CALLBACK,	0,				LOM_PAD_PRESS,
												LOM_DATA_END,
					LOM_PADLIST_01,				ACTION_BLINK,			LOM_PAD_ALLOBJ,		LOM_PAD_ANYINPUT,	LOM_EXE_CALLBACK,	1,				0,
												LOM_DATA_END,
					LOM_DATA_END,

					/*モード名*/	/*ＬＯＭアクションリスト*/	/*ＬＯＭオブジェクトリスト*/	/*ＬＯＭパッドリスト*/
LOM_MODE_DATA,		LOM_MODE_00,	LOM_ACTLIST_00,				LOM_MODE_NOLIST,				LOM_PADLIST_00,		/* rai vr open */
					LOM_MODE_01,	LOM_ACTLIST_01,				LOM_MODE_NOLIST,				LOM_PADLIST_00,		/* rai sne open */
					LOM_MODE_02,	LOM_ACTLIST_02,				LOM_MODE_NOLIST,				LOM_PADLIST_00,		/* rai wea open */
					LOM_MODE_03,	LOM_ACTLIST_03,				LOM_MODE_NOLIST,				LOM_PADLIST_00,		/* nin vr open */
					LOM_MODE_04,	LOM_ACTLIST_04,				LOM_MODE_NOLIST,				LOM_PADLIST_00,		/* nin sne open */
					LOM_MODE_05,	LOM_ACTLIST_05,				LOM_MODE_NOLIST,				LOM_PADLIST_00,		/* nin wea open */
					LOM_MODE_06,	LOM_ACTLIST_06,				LOM_MODE_NOLIST,				LOM_PADLIST_00,		/* x-r vr open */
					LOM_MODE_07,	LOM_ACTLIST_07,				LOM_MODE_NOLIST,				LOM_PADLIST_00,		/* sna vr open */
					LOM_MODE_08,	LOM_ACTLIST_08,				LOM_MODE_NOLIST,				LOM_PADLIST_00,		/* sna sne open */
					LOM_MODE_09,	LOM_ACTLIST_09,				LOM_MODE_NOLIST,				LOM_PADLIST_00,		/* sna wea open */
					LOM_MODE_10,	LOM_ACTLIST_10,				LOM_MODE_NOLIST,				LOM_PADLIST_00,		/* pli vr open */
					LOM_MODE_11,	LOM_ACTLIST_11,				LOM_MODE_NOLIST,				LOM_PADLIST_00,		/* pli sne open */
					LOM_MODE_12,	LOM_ACTLIST_12,				LOM_MODE_NOLIST,				LOM_PADLIST_00,		/* pli wea open */
					LOM_MODE_13,	LOM_ACTLIST_13,				LOM_MODE_NOLIST,				LOM_PADLIST_00,		/* tux vr open */
					LOM_MODE_14,	LOM_ACTLIST_14,				LOM_MODE_NOLIST,				LOM_PADLIST_00,		/* tux sne open */
					LOM_MODE_15,	LOM_ACTLIST_15,				LOM_MODE_NOLIST,				LOM_PADLIST_00,		/* tux wea open */
					LOM_MODE_16,	LOM_ACTLIST_16,				LOM_MODE_NOLIST,				LOM_PADLIST_00,		/* pre vr open */
					LOM_MODE_17,	LOM_ACTLIST_17,				LOM_MODE_NOLIST,				LOM_PADLIST_00,		/* pre sne open */
					LOM_MODE_18,	LOM_ACTLIST_18,				LOM_MODE_NOLIST,				LOM_PADLIST_00,		/* pre wea open */

					LOM_MODE_20,	LOM_ACTLIST_20,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* rai vr close */
					LOM_MODE_21,	LOM_ACTLIST_21,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* rai sne close */
					LOM_MODE_22,	LOM_ACTLIST_22,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* rai wea close */
					LOM_MODE_23,	LOM_ACTLIST_23,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* nin vr close */
					LOM_MODE_24,	LOM_ACTLIST_24,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* nin sne close */
					LOM_MODE_25,	LOM_ACTLIST_25,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* nin wea close */
					LOM_MODE_26,	LOM_ACTLIST_26,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* x-r vr close */
					LOM_MODE_27,	LOM_ACTLIST_27,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* sna vr close */
					LOM_MODE_28,	LOM_ACTLIST_28,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* sna sne close */
					LOM_MODE_29,	LOM_ACTLIST_29,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* sna wea close */
					LOM_MODE_30,	LOM_ACTLIST_30,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* pli vr close */
					LOM_MODE_31,	LOM_ACTLIST_31,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* pli sne close */
					LOM_MODE_32,	LOM_ACTLIST_32,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* pli wea close */
					LOM_MODE_33,	LOM_ACTLIST_33,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* tux vr close */
					LOM_MODE_34,	LOM_ACTLIST_34,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* tux sne close */
					LOM_MODE_35,	LOM_ACTLIST_35,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* tux wea close */
					LOM_MODE_36,	LOM_ACTLIST_36,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* pre vr close */
					LOM_MODE_37,	LOM_ACTLIST_37,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* pre sne close */
					LOM_MODE_38,	LOM_ACTLIST_38,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* pre wea close */
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
 * シグナルハンドラ
 */
static void LOMSigHandler(
	void *pWork,	/* ワーク */
	int  sign,		/* シグナル名 */
	int  value)		/* 値 */
{
	if(sign == 3781)	/* se */
		GM_SdSet(SD_S_TYPING01);
	else if(sign == 121042)	/* se2 */
		GM_SdSet(SD_S_TYPING02);
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
	case 0:
		/* モードの設定 */
		switch(work->player)
		{
		case MSN_PLAYER_RAIDEN:
			if     (work->mode == MSN_MODE_STREAKING) { LOM_SetCurMode(&work->layoutman, LOM_MODE_26, 1, 1, 1); SY_PRINTF1("LOM_MODE_26\n"); }
			else if(work->mode == MSN_MODE_SNEAKING ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_21, 1, 1, 1); SY_PRINTF1("LOM_MODE_21\n"); }
			else if(work->mode == MSN_MODE_WEAPON   ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_22, 1, 1, 1); SY_PRINTF1("LOM_MODE_22\n"); }
			else                                      { LOM_SetCurMode(&work->layoutman, LOM_MODE_20, 1, 1, 1); SY_PRINTF1("LOM_MODE_20\n"); }
			break;
		case MSN_PLAYER_NINJA:
			if     (work->mode == MSN_MODE_SNEAKING ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_24, 1, 1, 1); SY_PRINTF1("LOM_MODE_24\n"); }
			else if(work->mode == MSN_MODE_WEAPON   ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_25, 1, 1, 1); SY_PRINTF1("LOM_MODE_25\n"); }
			else                                      { LOM_SetCurMode(&work->layoutman, LOM_MODE_23, 1, 1, 1); SY_PRINTF1("LOM_MODE_23\n"); }
			break;
		case MSN_PLAYER_SNAKE:
			if     (work->mode == MSN_MODE_SNEAKING ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_28, 1, 1, 1); SY_PRINTF1("LOM_MODE_28\n"); }
			else if(work->mode == MSN_MODE_WEAPON   ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_29, 1, 1, 1); SY_PRINTF1("LOM_MODE_29\n"); }
			else                                      { LOM_SetCurMode(&work->layoutman, LOM_MODE_27, 1, 1, 1); SY_PRINTF1("LOM_MODE_27\n"); }
			break;
		case MSN_PLAYER_PLISKIN:
			if     (work->mode == MSN_MODE_SNEAKING ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_31, 1, 1, 1); SY_PRINTF1("LOM_MODE_31\n"); }
			else if(work->mode == MSN_MODE_WEAPON   ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_32, 1, 1, 1); SY_PRINTF1("LOM_MODE_32\n"); }
			else                                      { LOM_SetCurMode(&work->layoutman, LOM_MODE_30, 1, 1, 1); SY_PRINTF1("LOM_MODE_30\n"); }
			break;
		case MSN_PLAYER_TUXEDO:
			if     (work->mode == MSN_MODE_SNEAKING ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_34, 1, 1, 1); SY_PRINTF1("LOM_MODE_34\n"); }
			else if(work->mode == MSN_MODE_WEAPON   ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_35, 1, 1, 1); SY_PRINTF1("LOM_MODE_35\n"); }
			else                                      { LOM_SetCurMode(&work->layoutman, LOM_MODE_33, 1, 1, 1); SY_PRINTF1("LOM_MODE_33\n"); }
			break;
		case MSN_PLAYER_PREVIOUS:
			if     (work->mode == MSN_MODE_SNEAKING ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_37, 1, 1, 1); SY_PRINTF1("LOM_MODE_37\n"); }
			else if(work->mode == MSN_MODE_WEAPON   ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_38, 1, 1, 1); SY_PRINTF1("LOM_MODE_38\n"); }
			else                                      { LOM_SetCurMode(&work->layoutman, LOM_MODE_36, 1, 1, 1); SY_PRINTF1("LOM_MODE_36\n"); }
			break;
		default:
			ASSERT(0);
			break;
		}
		GM_SdSet(SD_S_V_CANS02);
 		break;

	case 1:
		GV_DestroyActor(work);
		break;

	default:
		ASSERT(0);
		return 0;
	}

	return 1;
}

/******************************************************************************
 * 毎フレーム処理
 */
static void Act(
	WORK *work)		/* ワーク */
{
	if(work->flag & VRCLC_INIT)
	{
		work->flag &= ~VRCLC_INIT;
		work->flag |=  VRCLC_INIT2;
		return;
	}
	if(work->flag & VRCLC_INIT2)
	{
		/*
		 * レイアウトの初期化
		 */
		work->flag &= ~VRCLC_INIT2;
		/* 各ＵＶ値の取得 */
		LOM_SprTex_GetV(&work->layoutman, OBJECT_MISSION, &work->tex_mis_v0);
		LOM_SprTex_GetV(&work->layoutman, OBJECT_MODE   , &work->tex_mod_v0);
		LOM_SprTex_GetV(&work->layoutman, OBJECT_WEAPON , &work->tex_wea_v0);
		LOM_SprTex_GetU(&work->layoutman, OBJECT_LEVEL01, &work->tex_lev_u0);
		LOM_SprTex_GetU(&work->layoutman, OBJECT_FONT_01, &work->tex_fon_u0);

		/* モードの設定 */
		switch(work->player)
		{
		case MSN_PLAYER_RAIDEN:
			if     (work->mode == MSN_MODE_STREAKING) { LOM_SetCurMode(&work->layoutman, LOM_MODE_06, 1, 1, 1); SY_PRINTF1("LOM_MODE_06\n"); }
			else if(work->mode == MSN_MODE_SNEAKING ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_01, 1, 1, 1); SY_PRINTF1("LOM_MODE_01\n"); }
			else if(work->mode == MSN_MODE_WEAPON   ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_02, 1, 1, 1); SY_PRINTF1("LOM_MODE_02\n"); }
			else                                      { LOM_SetCurMode(&work->layoutman, LOM_MODE_00, 1, 1, 1); SY_PRINTF1("LOM_MODE_00\n"); }
			break;
		case MSN_PLAYER_NINJA:
			if     (work->mode == MSN_MODE_SNEAKING ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_04, 1, 1, 1); SY_PRINTF1("LOM_MODE_04\n"); }
			else if(work->mode == MSN_MODE_WEAPON   ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_05, 1, 1, 1); SY_PRINTF1("LOM_MODE_05\n"); }
			else                                      { LOM_SetCurMode(&work->layoutman, LOM_MODE_03, 1, 1, 1); SY_PRINTF1("LOM_MODE_03\n"); }
			break;
		case MSN_PLAYER_SNAKE:
			if     (work->mode == MSN_MODE_SNEAKING ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_08, 1, 1, 1); SY_PRINTF1("LOM_MODE_08\n"); }
			else if(work->mode == MSN_MODE_WEAPON   ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_09, 1, 1, 1); SY_PRINTF1("LOM_MODE_09\n"); }
			else                                      { LOM_SetCurMode(&work->layoutman, LOM_MODE_07, 1, 1, 1); SY_PRINTF1("LOM_MODE_07\n"); }
			break;
		case MSN_PLAYER_PLISKIN:
			if     (work->mode == MSN_MODE_SNEAKING ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_11, 1, 1, 1); SY_PRINTF1("LOM_MODE_11\n"); }
			else if(work->mode == MSN_MODE_WEAPON   ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_12, 1, 1, 1); SY_PRINTF1("LOM_MODE_12\n"); }
			else                                      { LOM_SetCurMode(&work->layoutman, LOM_MODE_10, 1, 1, 1); SY_PRINTF1("LOM_MODE_10\n"); }
			break;
		case MSN_PLAYER_TUXEDO:
			if     (work->mode == MSN_MODE_SNEAKING ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_14, 1, 1, 1); SY_PRINTF1("LOM_MODE_14\n"); }
			else if(work->mode == MSN_MODE_WEAPON   ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_15, 1, 1, 1); SY_PRINTF1("LOM_MODE_15\n"); }
			else                                      { LOM_SetCurMode(&work->layoutman, LOM_MODE_13, 1, 1, 1); SY_PRINTF1("LOM_MODE_13\n"); }
			break;
		case MSN_PLAYER_PREVIOUS:
			if     (work->mode == MSN_MODE_SNEAKING ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_17, 1, 1, 1); SY_PRINTF1("LOM_MODE_17\n"); }
			else if(work->mode == MSN_MODE_WEAPON   ) { LOM_SetCurMode(&work->layoutman, LOM_MODE_18, 1, 1, 1); SY_PRINTF1("LOM_MODE_18\n"); }
			else                                      { LOM_SetCurMode(&work->layoutman, LOM_MODE_16, 1, 1, 1); SY_PRINTF1("LOM_MODE_16\n"); }
			break;
		default:
			ASSERT(0);
			break;
		}
	}

	/* レイアウトマネージャ */
	ActLayoutman(&work->layoutman);

	/* 各ＵＶ値の操作 */
	LOM_SprTex_SetV(&work->layoutman, OBJECT_MISSION, work->tex_mis_v0 + VRCLC_TEX_HEIGHT * work->mission);
	LOM_SprTex_SetV(&work->layoutman, OBJECT_MODE   , work->tex_mod_v0 + VRCLC_TEX_HEIGHT * work->mode   );
	if(    (work->weapon == MSN_WEAPON_HF_BLADE)
		&& (   (work->player == MSN_PLAYER_SNAKE   )
			|| (work->player == MSN_PLAYER_PLISKIN )
			|| (work->player == MSN_PLAYER_TUXEDO  )
			|| (work->player == MSN_PLAYER_PREVIOUS)))
	{
		LOM_SprTex_SetV(&work->layoutman, OBJECT_WEAPON , work->tex_wea_v0 + VRCLC_TEX_HEIGHT * (work->weapon + 1));
	}
	else
	{
		LOM_SprTex_SetV(&work->layoutman, OBJECT_WEAPON , work->tex_wea_v0 + VRCLC_TEX_HEIGHT * work->weapon);
	}
	if(work->level == 10)
	{
		LOM_SprTex_SetU(&work->layoutman, OBJECT_LEVEL10, work->tex_lev_u0 + VRCLC_TEX_WIDTH * 1);
		LOM_SprTex_SetU(&work->layoutman, OBJECT_LEVEL01, work->tex_lev_u0 + VRCLC_TEX_WIDTH * 0);
	}
	else
	{
		LOM_SprTex_SetU(&work->layoutman, OBJECT_LEVEL10, work->tex_lev_u0 + VRCLC_TEX_WIDTH * 0);
		LOM_SprTex_SetU(&work->layoutman, OBJECT_LEVEL01, work->tex_lev_u0 + VRCLC_TEX_WIDTH * work->level);
	}
	LOM_SprTex_SetU(&work->layoutman, OBJECT_FONT_01, work->tex_fon_u0 + VRCLC_TEX_WIDTH * work->clear_code[ 0]);
	LOM_SprTex_SetU(&work->layoutman, OBJECT_FONT_02, work->tex_fon_u0 + VRCLC_TEX_WIDTH * work->clear_code[ 1]);
	LOM_SprTex_SetU(&work->layoutman, OBJECT_FONT_03, work->tex_fon_u0 + VRCLC_TEX_WIDTH * work->clear_code[ 2]);
	LOM_SprTex_SetU(&work->layoutman, OBJECT_FONT_04, work->tex_fon_u0 + VRCLC_TEX_WIDTH * work->clear_code[ 3]);
	LOM_SprTex_SetU(&work->layoutman, OBJECT_FONT_05, work->tex_fon_u0 + VRCLC_TEX_WIDTH * work->clear_code[ 4]);
	LOM_SprTex_SetU(&work->layoutman, OBJECT_FONT_06, work->tex_fon_u0 + VRCLC_TEX_WIDTH * work->clear_code[ 5]);
	LOM_SprTex_SetU(&work->layoutman, OBJECT_FONT_07, work->tex_fon_u0 + VRCLC_TEX_WIDTH * work->clear_code[ 6]);
	LOM_SprTex_SetU(&work->layoutman, OBJECT_FONT_08, work->tex_fon_u0 + VRCLC_TEX_WIDTH * work->clear_code[ 7]);
	LOM_SprTex_SetU(&work->layoutman, OBJECT_FONT_09, work->tex_fon_u0 + VRCLC_TEX_WIDTH * work->clear_code[ 8]);
	LOM_SprTex_SetU(&work->layoutman, OBJECT_FONT_10, work->tex_fon_u0 + VRCLC_TEX_WIDTH * work->clear_code[ 9]);
	LOM_SprTex_SetU(&work->layoutman, OBJECT_FONT_11, work->tex_fon_u0 + VRCLC_TEX_WIDTH * work->clear_code[10]);
	LOM_SprTex_SetU(&work->layoutman, OBJECT_FONT_12, work->tex_fon_u0 + VRCLC_TEX_WIDTH * work->clear_code[11]);
	LOM_SprTex_SetU(&work->layoutman, OBJECT_FONT_13, work->tex_fon_u0 + VRCLC_TEX_WIDTH * work->clear_code[12]);
	LOM_SprTex_SetU(&work->layoutman, OBJECT_FONT_14, work->tex_fon_u0 + VRCLC_TEX_WIDTH * work->clear_code[13]);
	LOM_SprTex_SetU(&work->layoutman, OBJECT_FONT_15, work->tex_fon_u0 + VRCLC_TEX_WIDTH * work->clear_code[14]);
	LOM_SprTex_SetU(&work->layoutman, OBJECT_FONT_16, work->tex_fon_u0 + VRCLC_TEX_WIDTH * work->clear_code[15]);
	LOM_SprTex_SetU(&work->layoutman, OBJECT_FONT_17, work->tex_fon_u0 + VRCLC_TEX_WIDTH * work->clear_code[16]);
	LOM_SprTex_SetU(&work->layoutman, OBJECT_FONT_18, work->tex_fon_u0 + VRCLC_TEX_WIDTH * work->clear_code[17]);
	LOM_SprTex_SetU(&work->layoutman, OBJECT_FONT_19, work->tex_fon_u0 + VRCLC_TEX_WIDTH * work->clear_code[18]);
	LOM_SprTex_SetU(&work->layoutman, OBJECT_FONT_20, work->tex_fon_u0 + VRCLC_TEX_WIDTH * work->clear_code[19]);
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

	/* 終了に設定 */
	*work->status = 1;
}

/******************************************************************************
 * 初期化処理
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	WORK *work,		/* ワーク */
	int  name,		/* シナリオ名 */
	int  where,		/* マップＩＤ */
	int  stage,		/* ステージＩＤ */
	int  *status)	/* 起動状態  0:起動中  1:終了 */
{
	int length;

	SY_PRINTF3("GetResources\n");

	work->stage      = stage;
	work->status     = status;
	work->tex_mis_v0 = 0;
	work->tex_mod_v0 = 0;
	work->tex_wea_v0 = 0;
	work->tex_lev_u0 = 0;
	work->tex_fon_u0 = 0;
	work->flag       = 0;

	/* ステージ情報 */
	Msn_GetStageInfo(work->stage, &work->mission, &work->mode, &work->weapon, &work->level, &work->player);
	work->level++;

	/* クリアコード */
	memset(work->clear_code, 0x00, sizeof(work->clear_code));
	length = Msn_GetClearCode(work->stage, work->clear_code, 20);
	ASSERT(length == 20);

	/* レイアウトマネージャ */
	{
		if(CreateLayoutman3(&work->layoutman, L2D_FILENAME, 4, 0, 0, 0, 0, LOMCallback, LOMSigHandler, work, 5, 5, LOM_INPUT_DIRECT) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
		SY_PRINTF1("Create Layoutman Succeeded.\n");
		if(LoadLOMData(&work->layoutman, _lom_data) == 0) { SY_PRINTF2("Load LOM Data Failed.\n"); return 0; }
		SY_PRINTF1("Load LOM Data Succeeded.\n");
	}

	/* 起動中に設定 */
	*work->status = 0;

	work->flag |= VRCLC_INIT;

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
void *NewVRClearCode(
	int stage,		/* ステージＩＤ */
	int *status)	/* 起動状態  0:起動中  1:終了 */
{
	WORK *work;

	SY_PRINTF3("NewVRClearCode\n");
	OPERATOR();

	if(status == NULL)
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
		if(!GetResources(work, 0, GM_CurrentMap, stage, status))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}
