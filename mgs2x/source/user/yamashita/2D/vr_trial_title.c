/*******************************************************************************
 * vr_trial_title - vr_trial_title.c
 * ＶＲ体験版タイトル  *NewVRTrialTitle
 * 2002/06/17 S.Yamashita
 * $Id: vr_trial_title.c,v 1.2 2002/12/05 18:41:56 takaki Exp $
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

#define __CHARA_NAME__ "VR Trial Title"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
//#define SY_PRINTF3_DEBUG
#include "../sy_util/sy_util.h"

/*******************************************************************************
 * defines
 */

/* コールバックタイプ */
enum {
	VRTRT_CB_ITEM_1_UP = 1,
	VRTRT_CB_ITEM_1_DN,
	VRTRT_CB_ITEM_1_OK,
	VRTRT_CB_ITEM_2_UP,
	VRTRT_CB_ITEM_2_DN,
	VRTRT_CB_ITEM_2_OK,
	VRTRT_CB_ITEM_3_UP,
	VRTRT_CB_ITEM_3_DN,
	VRTRT_CB_ITEM_3_OK,
	VRTRT_CB_ITEM_4_UP,
	VRTRT_CB_ITEM_4_DN,
	VRTRT_CB_ITEM_4_OK,

	VRTRT_CB_ITEM_5_UP,
	VRTRT_CB_ITEM_5_DN,
	VRTRT_CB_ITEM_5_OK,

	VRTRT_CB_CLOSE_PICT,
	VRTRT_CB_CLOSE_PICT2,
	VRTRT_CB_CLOSED,
};

/* フラグ */
#define VRTRT_INIT        (0x01)	/* 初期化時 */
#define VRTRT_INPUT_OK    (0x02)	/* 入力ＯＫ */

/* 設定値 */
#define VRTRT_TEX_WIDTH (18.0f)		/* テクスチャ幅 */
#define VRTRT_ICO_R     (-27)		/* アイコン相対位置Ｘ */
#define VRTRT_ICO_T     (  0)		/* アイコン相対位置Ｙ */

/* Ｌ２Ｄオブジェクト */
#define L2D_FILENAME       (16712935)	/* vr_training_menu */
#define ACTION_OPEN_FRAME  (  863270)	/* openFrame */
#define ACTION_CLOSE_FRAME ( 6956960)	/* closeFrame */
#define ACTION_SHOW_PICT   (12870753)	/* show_controls */
#define ACTION_HIDE_PICT   ( 8631475)	/* hide_controls */
#define ACTION_SHOW_PICT2  (5073005)	/* show_controls_eng */
#define ACTION_HIDE_PICT2  ( 6905202)	/* hide_controls_eng */
#define ACTION_DUMMY       ( 8143391)	/* dummy */
#define ACTION_DUMMY2      ( 8930337)	/* dummy2 */

#define OBJECT_ITEM_1      ( 2661433)	/* new_game */
#define OBJECT_ITEM_2      ( 9741075)	/* load_game */
#define OBJECT_ITEM_3      ( 3912499)	/* option */
#define OBJECT_ITEM_4      ( 2148199)	/* controls_font */
#define OBJECT_ITEM_5      ( 3435924)	/* exit */

/*******************************************************************************
 * work
 */

/* ワーク */
typedef struct tagWORK
{
	GV_ACT_EX      actor;		/* アクター */

	int            proc[4];		/* プロシージャ */

	int            name;		/* シナリオ名 */
	char           index;		/* インデックス */
	char           flag;		/* フラグ */
	unsigned short count;		/* カウンター */
	char           prev;		/* 前の選択アイテム番号 */
	unsigned short count2;		/* カウンター２ */

	LAYOUTMAN      layoutman;	/* レイアウトマネージャ */
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

					LOM_ACTLIST_03,				ACTION_SHOW_PICT,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_04,				ACTION_HIDE_PICT,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_05,				ACTION_SHOW_PICT2,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_06,				ACTION_HIDE_PICT2,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,

					LOM_ACTLIST_07,				ACTION_CLOSE_FRAME,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_DUMMY,			1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												ACTION_DUMMY2,			1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_DATA_END,

					/*アイコンリスト名*/		/* アイコン名 */	/* テクスチャ名 */	/* 位置(左、上、右、下) */								/* 色 */												/* カウント */	/* フラグ */
LOM_ICOLIST_DATA,	LOM_ICOLIST_00,				LOM_ICO_00,			LOM_ICO_NOTEX,		VRTRT_ICO_R, VRTRT_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H,		VRDEF_CUR_R, VRDEF_CUR_G, VRDEF_CUR_B, VRDEF_CUR_A,		LOM_ICO_LOOP,	LOM_ICO_L_REL|LOM_ICO_T_REL|LOM_ICO_SIZE|LOM_ICO_ALPHA,
												LOM_DATA_END,
					LOM_DATA_END,

#ifdef AREA_EU	// #ifdef PAL
					/* オブジェクトリスト名 */	/* オブジェクト名 */	/* アイコンリスト名 */	/* カウント */	/* フラグ */
LOM_OBJLIST_DATA,	LOM_OBJLIST_00,				OBJECT_ITEM_1,			LOM_ICOLIST_00,			7,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_2,			LOM_ICOLIST_00,			7,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_3,			LOM_ICOLIST_00,			7,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_4,			LOM_ICOLIST_00,			7,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_5,			LOM_ICOLIST_00,			7,				LOM_OBJ_INTERP_1,
												LOM_DATA_END,
					LOM_DATA_END,
#else
					/* オブジェクトリスト名 */	/* オブジェクト名 */	/* アイコンリスト名 */	/* カウント */	/* フラグ */
LOM_OBJLIST_DATA,	LOM_OBJLIST_00,				OBJECT_ITEM_1,			LOM_ICOLIST_00,			8,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_2,			LOM_ICOLIST_00,			8,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_3,			LOM_ICOLIST_00,			8,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_4,			LOM_ICOLIST_00,			8,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_5,			LOM_ICOLIST_00,			8,				LOM_OBJ_INTERP_1,
												LOM_DATA_END,
					LOM_DATA_END,
#endif

					/*パッドリスト名*/			/*アクション名*/		/*オブジェクト名*/	/*入力*/	/*実行番号*/		/*実行引数*/			/*フラグ*/
LOM_PADLIST_DATA,	LOM_PADLIST_00,				LOM_PAD_ALLACT,			OBJECT_ITEM_1,		PAD_U,		LOM_EXE_CALLBACK,	VRTRT_CB_ITEM_1_UP,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_1,		PAD_D,		LOM_EXE_CALLBACK,	VRTRT_CB_ITEM_1_DN,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_2,		PAD_U,		LOM_EXE_CALLBACK,	VRTRT_CB_ITEM_2_UP,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_2,		PAD_D,		LOM_EXE_CALLBACK,	VRTRT_CB_ITEM_2_DN,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_3,		PAD_U,		LOM_EXE_CALLBACK,	VRTRT_CB_ITEM_3_UP,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_3,		PAD_D,		LOM_EXE_CALLBACK,	VRTRT_CB_ITEM_3_DN,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_4,		PAD_U,		LOM_EXE_CALLBACK,	VRTRT_CB_ITEM_4_UP,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_4,		PAD_D,		LOM_EXE_CALLBACK,	VRTRT_CB_ITEM_4_DN,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_5,		PAD_U,		LOM_EXE_CALLBACK,	VRTRT_CB_ITEM_5_UP,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_5,		PAD_D,		LOM_EXE_CALLBACK,	VRTRT_CB_ITEM_5_DN,		LOM_PAD_ON_SIG,

												LOM_PAD_ALLACT,			OBJECT_ITEM_1,		PAD_OK,		LOM_EXE_CALLBACK,	VRTRT_CB_ITEM_1_OK,		LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ITEM_2,		PAD_OK,		LOM_EXE_CALLBACK,	VRTRT_CB_ITEM_2_OK,		LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ITEM_3,		PAD_OK,		LOM_EXE_CALLBACK,	VRTRT_CB_ITEM_3_OK,		LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ITEM_4,		PAD_OK,		LOM_EXE_CALLBACK,	VRTRT_CB_ITEM_4_OK,		LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ITEM_5,		PAD_OK,		LOM_EXE_CALLBACK,	VRTRT_CB_ITEM_5_OK,		LOM_PAD_PRESS,
												LOM_DATA_END,

					LOM_PADLIST_01,				ACTION_SHOW_PICT,		LOM_PAD_ALLOBJ,		PAD_OK|PAD_CANCEL,	LOM_EXE_CALLBACK,	VRTRT_CB_CLOSE_PICT,	LOM_PAD_PRESS,
												LOM_DATA_END,

					LOM_PADLIST_02,				ACTION_SHOW_PICT2,		LOM_PAD_ALLOBJ,		PAD_OK|PAD_CANCEL,	LOM_EXE_CALLBACK,	VRTRT_CB_CLOSE_PICT2,	LOM_PAD_PRESS,
												LOM_DATA_END,

					/* 閉じた後用 */
					LOM_PADLIST_03,				ACTION_DUMMY,			LOM_PAD_ALLOBJ,		LOM_PAD_ANYINPUT,	LOM_EXE_CALLBACK,	VRTRT_CB_CLOSED,		0,
												LOM_DATA_END,
					LOM_DATA_END,

					/*モード名*/	/*ＬＯＭアクションリスト*/	/*ＬＯＭオブジェクトリスト*/	/*ＬＯＭパッドリスト*/
LOM_MODE_DATA,		LOM_MODE_00,	LOM_ACTLIST_00,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* init */
					LOM_MODE_01,	LOM_ACTLIST_01,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* open */
					LOM_MODE_02,	LOM_ACTLIST_02,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* dummy */
					LOM_MODE_03,	LOM_ACTLIST_03,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* show */
					LOM_MODE_04,	LOM_ACTLIST_04,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* hide */
					LOM_MODE_05,	LOM_ACTLIST_05,				LOM_MODE_NOLIST,				LOM_PADLIST_02,		/* show2 */
					LOM_MODE_06,	LOM_ACTLIST_06,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* hide2 */
					LOM_MODE_07,	LOM_ACTLIST_07,				LOM_MODE_NOLIST,				LOM_PADLIST_03,		/* close */
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

	if(param != VRTRT_CB_CLOSED)
	{
		if(!(work->flag & VRTRT_INPUT_OK))
			return 0;
		work->flag &= ~VRTRT_INPUT_OK;
		work->count2 = 0;
	}

	switch(param)
	{
	case VRTRT_CB_ITEM_1_DN: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_2, 1); return 1;
	case VRTRT_CB_ITEM_2_DN: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_3, 1); return 1;
	case VRTRT_CB_ITEM_3_DN: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_4, 1); return 1;
	case VRTRT_CB_ITEM_4_DN: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_5, 1); return 1;
	case VRTRT_CB_ITEM_5_DN: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_1, 1); return 1;

	case VRTRT_CB_ITEM_1_UP: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_5, 1); return 1;
	case VRTRT_CB_ITEM_2_UP: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_1, 1); return 1;
	case VRTRT_CB_ITEM_3_UP: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_2, 1); return 1;
	case VRTRT_CB_ITEM_4_UP: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_3, 1); return 1;
	case VRTRT_CB_ITEM_5_UP: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_4, 1); return 1;

	case VRTRT_CB_ITEM_1_OK:
		work->index = 0;
		LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_1, VRTRT_ICO_R, VRTRT_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
		LOM_SetCurMode(&work->layoutman, LOM_MODE_07, 1, 1, 1);
		work->count = 0;
		GM_SdSet(SD_S_WIN01  );
		GM_SdSet(SD_S_WINCLS01);
		return 1;
	case VRTRT_CB_ITEM_2_OK:
		work->index = 1;
		LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_2, VRTRT_ICO_R, VRTRT_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
		LOM_SetCurMode(&work->layoutman, LOM_MODE_07, 1, 1, 1);
		work->count = 0;
		GM_SdSet(SD_S_WIN01  );
		GM_SdSet(SD_S_WINCLS01);
		return 1;
	case VRTRT_CB_ITEM_3_OK:
		work->index = 2;
		LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_3, VRTRT_ICO_R, VRTRT_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
		LOM_SetCurMode(&work->layoutman, LOM_MODE_07, 1, 1, 1);
		work->count = 0; GM_SdSet(SD_S_WIN01  );
		GM_SdSet(SD_S_WINCLS01);
		return 1;
	case VRTRT_CB_ITEM_5_OK:
		work->index = 3;
		LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_5, VRTRT_ICO_R, VRTRT_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
		LOM_SetCurMode(&work->layoutman, LOM_MODE_07, 1, 1, 1);
		work->count = 0;
		GM_SdSet(SD_S_START01);
		GM_SdSet(SD_S_WINCLS01);
		return 1;

	case VRTRT_CB_ITEM_4_OK:
		SY_PRINTF1("GM_Language %d\n", GM_Language);
		LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_4, VRTRT_ICO_R, VRTRT_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); 
		if(GM_Language == 7) { LOM_SetCurMode(&work->layoutman, LOM_MODE_03, 1, 1, 0); SY_PRINTF1("LOM_MODE_03\n"); }
		else                 { LOM_SetCurMode(&work->layoutman, LOM_MODE_05, 1, 1, 0); SY_PRINTF1("LOM_MODE_05\n"); }
		work->count = 0;
		GM_SdSet(SD_S_WIN01);
		return 1;

	case VRTRT_CB_CLOSE_PICT:
		LOM_SetCurMode(&work->layoutman, LOM_MODE_04, 1, 1, 0);
		work->count = 0;
		GM_SdSet(SD_S_V_CANS02);
		return 1;

	case VRTRT_CB_CLOSE_PICT2:
		LOM_SetCurMode(&work->layoutman, LOM_MODE_06, 1, 1, 0);
		work->count = 0;
		GM_SdSet(SD_S_V_CANS02);
		return 1;

	case VRTRT_CB_CLOSED:
		/* プロック実行 */
		ASSERT(work->proc[(int)work->index]);
		SY_PRINTF1("PROC CALLED\n");
		GM_ExecProc(work->proc[(int)work->index], NULL);
		return 1;
	default:
		return 1;
	}
}

/*******************************************************************************
 * メッセージ処理

mesg ＶＲ体験版タイトル $s:名前 表示[0] $i:カーソル位置	// 1～
mesg ＶＲ体験版タイトル $s:名前 非表示[1]
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
			LOM_SetCurMode(&work->layoutman, LOM_MODE_01, 1, 1, 0);
			work->prev = msg->message[1];
			work->count = 0;
			GM_SdSet(SD_S_LINEMOV1);
			ret = 0;
			break;

		case 1:		/* 非表示 */
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
	if(work->flag & VRTRT_INIT)
	{
		/*
		 * レイアウトの初期化
		 */
		work->flag &= ~VRTRT_INIT;

		/* モードの設定 */
		LOM_SetCurMode(&work->layoutman, LOM_MODE_00, 1, 1, 1);
	}

	work->count++;
	work->count2++;

	/* メッセージ処理 */
	ReceiveMessage(work);

	/* レイアウトマネージャ */
	ActLayoutman(&work->layoutman);

	/* 各モードごとの処理 */
	if(LOM_GetCurMode(&work->layoutman) == LOM_MODE_00)
	{
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_1, 0, 256, VRDEF_INACTIVE, VRDEF_ACTIVE);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_2, 0, 256, VRDEF_INACTIVE, VRDEF_ACTIVE);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_3, 0, 256, VRDEF_INACTIVE, VRDEF_ACTIVE);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_4, 0, 256, VRDEF_INACTIVE, VRDEF_ACTIVE);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_5, 0, 256, VRDEF_INACTIVE, VRDEF_ACTIVE);
	}
	else if(LOM_GetCurMode(&work->layoutman) == LOM_MODE_01)
	{
#ifdef PAL
		if(work->count == 8) GM_SdSet(SD_S_WINOPN01);
#else
		if(work->count == 10) GM_SdSet(SD_S_WINOPN01);
#endif

		SetSpriteColor(&work->layoutman, OBJECT_ITEM_1, 0, 256, VRDEF_INACTIVE, VRDEF_ACTIVE);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_2, 0, 256, VRDEF_INACTIVE, VRDEF_ACTIVE);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_3, 0, 256, VRDEF_INACTIVE, VRDEF_ACTIVE);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_4, 0, 256, VRDEF_INACTIVE, VRDEF_ACTIVE);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_5, 0, 256, VRDEF_INACTIVE, VRDEF_ACTIVE);

#ifdef PAL
		if(work->count == 25)
#else
		if(work->count == 30)
#endif
		{
			LOM_SetCurMode(&work->layoutman, LOM_MODE_02, 1, 1, 0);
			work->count = 0;

			switch(work->prev)
			{
			case 1:
				LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_1, VRTRT_ICO_R, VRTRT_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
				break;
			case 2:
				LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_2, 1);
				LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_2, VRTRT_ICO_R, VRTRT_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
				break;
			case 3:
				LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_3, 1);
				LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_3, VRTRT_ICO_R, VRTRT_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
				break;
			case 4:
				LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_4, 1);
				LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_4, VRTRT_ICO_R, VRTRT_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
				break;
			case 5:
				LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_5, 1);
				LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_5, VRTRT_ICO_R, VRTRT_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
				break;
			default:
				LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_1, VRTRT_ICO_R, VRTRT_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
				break;
			}
		}
		work->flag |= VRTRT_INPUT_OK;
	}
	else if(LOM_GetCurMode(&work->layoutman) == LOM_MODE_07)
	{
#ifdef PAL
		if(work->count == 8) GM_SdSet(SD_S_LINEMOV1);
#else
		if(work->count == 10) GM_SdSet(SD_S_LINEMOV1);
#endif

		if(work->prev == 1) { SetSpriteColor(&work->layoutman, OBJECT_ITEM_1, 1, 256, VRDEF_INACTIVE, VRDEF_ACTIVE); }
		else                { SetSpriteColor(&work->layoutman, OBJECT_ITEM_1, 0, 256, VRDEF_INACTIVE, VRDEF_ACTIVE); }
		if(work->prev == 2) { SetSpriteColor(&work->layoutman, OBJECT_ITEM_2, 1, 256, VRDEF_INACTIVE, VRDEF_ACTIVE); }
		else                { SetSpriteColor(&work->layoutman, OBJECT_ITEM_2, 0, 256, VRDEF_INACTIVE, VRDEF_ACTIVE); }
		if(work->prev == 3) { SetSpriteColor(&work->layoutman, OBJECT_ITEM_3, 1, 256, VRDEF_INACTIVE, VRDEF_ACTIVE); }
		else                { SetSpriteColor(&work->layoutman, OBJECT_ITEM_3, 0, 256, VRDEF_INACTIVE, VRDEF_ACTIVE); }
		if(work->prev == 4) { SetSpriteColor(&work->layoutman, OBJECT_ITEM_4, 1, 256, VRDEF_INACTIVE, VRDEF_ACTIVE); }
		else                { SetSpriteColor(&work->layoutman, OBJECT_ITEM_4, 0, 256, VRDEF_INACTIVE, VRDEF_ACTIVE); }
		if(work->prev == 5) { SetSpriteColor(&work->layoutman, OBJECT_ITEM_5, 1, 256, VRDEF_INACTIVE, VRDEF_ACTIVE); }
		else                { SetSpriteColor(&work->layoutman, OBJECT_ITEM_5, 0, 256, VRDEF_INACTIVE, VRDEF_ACTIVE); }
	}
	else
	{
		/* カーソルの位置に対する処理 */
		switch(LOM_GetCurObj(&work->layoutman))
		{
		case OBJECT_ITEM_1:
			work->prev = 1;

			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1, 1, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2, 0, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3, 0, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4, 0, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5, 0, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			break;

		case OBJECT_ITEM_2:
			work->prev = 2;

			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1, 0, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2, 1, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3, 0, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4, 0, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5, 0, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			break;

		case OBJECT_ITEM_3:
			work->prev = 3;

			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1, 0, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2, 0, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3, 1, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4, 0, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5, 0, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			break;

		case OBJECT_ITEM_4:
			work->prev = 4;

			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1, 0, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2, 0, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3, 0, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4, 1, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5, 0, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			break;

		case OBJECT_ITEM_5:
			work->prev = 5;

			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1, 0, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2, 0, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3, 0, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4, 0, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5, 1, 16, VRDEF_INACTIVE, VRDEF_ACTIVE);
			break;

		default:
			break;
		}
#ifdef PAL
		if(work->count2 == 7)
#else
		if(work->count2 == 8)
#endif
			work->flag |= VRTRT_INPUT_OK;
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

	/* 多重起動防止 */
	_work = NULL;
}

/******************************************************************************
 * 初期化処理

chara ＶＲ体験版タイトル[NewVRTrialTitle_Scn] $s:名前 \
	-new_game  $p:ＮＥＷプロック \
	-load_game $p:ＬＯＡＤプロック \
	-options   $p:ＯＰＴＩＯＮＳプロック \
	-exit      $p:ＥＸＩＴプロック
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	WORK *work,		/* ワーク */
	int  name,		/* シナリオ名 */
	int  where)		/* マップＩＤ */
{
	SY_PRINTF3("GetResources\n");

	memset(work->proc, 0x00, sizeof(work->proc));
	work->name   = name;
	work->index  = -1;
	work->flag   = 0;
	work->count  = 0;
	work->prev   = 0;
	work->count2 = 0;

	/* シナリオオプション */
	if(name != 0)
	{
		/* new_game */
		if(GCL_GetOption('n') != NULL)
			work->proc[0] = GCL_GetNextInt();

		/* load_game */
		if(GCL_GetOption('l') != NULL)
			work->proc[1] = GCL_GetNextInt();

		/* options */
		if(GCL_GetOption('o') != NULL)
			work->proc[2] = GCL_GetNextInt();

		/* exit */
		if(GCL_GetOption('e') != NULL)
			work->proc[3] = GCL_GetNextInt();

		SY_PRINTF1("Get Scn Option Succeeded.\n");
	}

	/* レイアウトマネージャ */
	{
#ifdef AREA_EU	// #ifdef PAL
		if(CreateLayoutman3(&work->layoutman, L2D_FILENAME, 4, 0, 0, 0, 0, LOMCallback, NULL, work, 17, 9, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
#else
		if(CreateLayoutman3(&work->layoutman, L2D_FILENAME, 4, 0, 0, 0, 0, LOMCallback, NULL, work, 20, 10, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
#endif
		SY_PRINTF1("Create Layoutman Succeeded.\n");
		if(LoadLOMData(&work->layoutman, _lom_data) == 0) { SY_PRINTF2("Load LOM Data Failed.\n"); return 0; }
		SY_PRINTF1("Load LOM Data Succeeded.\n");
	}

	work->flag |= VRTRT_INIT;

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
void *NewVRTrialTitle(void)
{
	WORK *work;

	SY_PRINTF3("NewVRTrialTitle\n");
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

void *NewVRTrialTitle_Scn(
	int name,	/* シナリオ名 */
	int where)	/* マップＩＤ */
{
	WORK *work;

	SY_PRINTF3("NewVRTrialTitle_Scn\n");
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
