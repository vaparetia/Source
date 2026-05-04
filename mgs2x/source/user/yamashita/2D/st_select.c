//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * st_select - st_select.c
 * スネークテールズセレクト  *NewSTSelect
 * 2002/07/09 S.Yamashita
 * $Id: st_select.c,v 1.1.1.3 2002/11/19 11:51:42 Yoshizawa1 Exp $
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

#define __CHARA_NAME__ "Snake Tales Select"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
//#define SY_PRINTF3_DEBUG
#include "../sy_util/sy_util.h"

/*******************************************************************************
 * defines
 */

/* コールバックタイプ */
enum {
	STS_CB_ITEM_1_UP = 1,
	STS_CB_ITEM_1_DN,
	STS_CB_ITEM_1_OK,
	STS_CB_ITEM_2_UP,
	STS_CB_ITEM_2_DN,
	STS_CB_ITEM_2_OK,
	STS_CB_ITEM_3_UP,
	STS_CB_ITEM_3_DN,
	STS_CB_ITEM_3_OK,
	STS_CB_ITEM_4_UP,
	STS_CB_ITEM_4_DN,
	STS_CB_ITEM_4_OK,
	STS_CB_ITEM_5_UP,
	STS_CB_ITEM_5_DN,
	STS_CB_ITEM_5_OK,
	STS_CB_ITEM_6_UP,
	STS_CB_ITEM_6_DN,
	STS_CB_ITEM_6_OK,

	STS_CB_CANCELED,
	STS_CB_OPENED,
	STS_CB_CLOSED,
};

/* フラグ */
#define STS_INIT        (0x01)	/* 初期化時 */
#define STS_INIT2       (0x02)	/* 初期化時２ */
#define STS_INPUT_OK    (0x04)	/* 入力ＯＫ */

/* 設定値 */
#define STS_ICO_R     (-27)		/* アイコン相対位置Ｘ */
#define STS_ICO_T     (  2)		/* アイコン相対位置Ｙ */

#define STS_OPENFRAME_COUNT (32)	/* フレーム開くカウント */
#define STS_INTERVAL_COUNT  ( 8)	/* 入力インターバルカウント */

/* Ｌ２Ｄオブジェクト */
#define L2D_FILENAME           ( 1003062)		/* snake_tales_select */

#define ACTION_OPEN_FRAME      (12404571)		/* openFrame_tales */
#define ACTION_CLOSE_FRAME     (16524786)		/* closeFrame_tales */
#define ACTION_DUMMY           ( 8143391)		/* dummy */
#define ACTION_DUMMY2          ( 8930337)		/* dummy2 */

#define OBJECT_ITEM_1            (16020969)		/* tales_A */
#define OBJECT_ITEM_2            (16020970)		/* tales_B */
#define OBJECT_ITEM_3            (16020971)		/* tales_C */
#define OBJECT_ITEM_4            (16020972)		/* tales_D */
#define OBJECT_ITEM_5            (16020973)		/* tales_E */
#define OBJECT_ITEM_6            ( 3435924)		/* exit */

/*******************************************************************************
 * work
 */

/* ワーク */
typedef struct tagWORK
{
	GV_ACT_EX      actor;			/* アクター */

	int            proc[6];			/* プロシージャ */
	int            name;			/* シナリオ名 */
	unsigned short count;			/* カウンター */
	unsigned short count2;			/* カウンター２ */

	LAYOUTMAN      layoutman;		/* レイアウトマネージャ */

	char           clear[5];		/* クリアフラグ */
	char           cursor;			/* カーソル位置 */
	char           flag;			/* フラグ */
	char           index;			/* インデックス */
}
WORK;

/*******************************************************************************
 * local
 */

/* ＬＯＭデータ */
static int _lom_data[] = {
					/*アクションリスト名*/		/*アクション名*/		/*カウント*/	/*フラグ*/
LOM_ACTLIST_DATA,	LOM_ACTLIST_00,				ACTION_DUMMY,			1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_01,				ACTION_OPEN_FRAME,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_DUMMY,			1,				LOM_ACT_FRAME|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_02,				ACTION_DUMMY,			1,				LOM_ACT_FRAME|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_03,				ACTION_CLOSE_FRAME,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_DUMMY,			1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												ACTION_DUMMY2,			1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_DATA_END,

					/*アイコンリスト名*/		/* アイコン名 */	/* テクスチャ名 */	/* 位置(左、上、右、下) */							/* 色 */												/* カウント */	/* フラグ */
LOM_ICOLIST_DATA,	LOM_ICOLIST_00,				LOM_ICO_00,			LOM_ICO_NOTEX,		STS_ICO_R, STS_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H,		VRDEF_CUR_R, VRDEF_CUR_G, VRDEF_CUR_B, VRDEF_CUR_A,		LOM_ICO_LOOP,	LOM_ICO_L_REL|LOM_ICO_T_REL|LOM_ICO_SIZE|LOM_ICO_ALPHA,
												LOM_DATA_END,
					LOM_DATA_END,

					/* オブジェクトリスト名 */	/* オブジェクト名 */	/* アイコンリスト名 */	/* カウント */	/* フラグ */
LOM_OBJLIST_DATA,	LOM_OBJLIST_00,				OBJECT_ITEM_1,			LOM_ICOLIST_00,			8,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_2,			LOM_ICOLIST_00,			8,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_3,			LOM_ICOLIST_00,			8,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_4,			LOM_ICOLIST_00,			8,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_5,			LOM_ICOLIST_00,			8,				LOM_OBJ_INTERP_1,
												OBJECT_ITEM_6,			LOM_ICOLIST_00,			8,				LOM_OBJ_INTERP_1,
												LOM_DATA_END,
					LOM_DATA_END,

					/*パッドリスト名*/			/*アクション名*/		/*オブジェクト名*/	/*入力*/	/*実行番号*/		/*実行引数*/			/*フラグ*/
LOM_PADLIST_DATA,	LOM_PADLIST_00,				LOM_PAD_ALLACT,			OBJECT_ITEM_1,		PAD_U,		LOM_EXE_CALLBACK,	STS_CB_ITEM_1_UP,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_1,		PAD_D,		LOM_EXE_CALLBACK,	STS_CB_ITEM_1_DN,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_2,		PAD_U,		LOM_EXE_CALLBACK,	STS_CB_ITEM_2_UP,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_2,		PAD_D,		LOM_EXE_CALLBACK,	STS_CB_ITEM_2_DN,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_3,		PAD_U,		LOM_EXE_CALLBACK,	STS_CB_ITEM_3_UP,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_3,		PAD_D,		LOM_EXE_CALLBACK,	STS_CB_ITEM_3_DN,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_4,		PAD_U,		LOM_EXE_CALLBACK,	STS_CB_ITEM_4_UP,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_4,		PAD_D,		LOM_EXE_CALLBACK,	STS_CB_ITEM_4_DN,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_5,		PAD_U,		LOM_EXE_CALLBACK,	STS_CB_ITEM_5_UP,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_5,		PAD_D,		LOM_EXE_CALLBACK,	STS_CB_ITEM_5_DN,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_6,		PAD_U,		LOM_EXE_CALLBACK,	STS_CB_ITEM_6_UP,		LOM_PAD_ON_SIG,
												LOM_PAD_ALLACT,			OBJECT_ITEM_6,		PAD_D,		LOM_EXE_CALLBACK,	STS_CB_ITEM_6_DN,		LOM_PAD_ON_SIG,

												LOM_PAD_ALLACT,			OBJECT_ITEM_1,		LOM_PAD_OK,		LOM_EXE_CALLBACK,	STS_CB_ITEM_1_OK,		LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ITEM_2,		LOM_PAD_OK,		LOM_EXE_CALLBACK,	STS_CB_ITEM_2_OK,		LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ITEM_3,		LOM_PAD_OK,		LOM_EXE_CALLBACK,	STS_CB_ITEM_3_OK,		LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ITEM_4,		LOM_PAD_OK,		LOM_EXE_CALLBACK,	STS_CB_ITEM_4_OK,		LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ITEM_5,		LOM_PAD_OK,		LOM_EXE_CALLBACK,	STS_CB_ITEM_5_OK,		LOM_PAD_PRESS,
												LOM_PAD_ALLACT,			OBJECT_ITEM_6,		LOM_PAD_OK,		LOM_EXE_CALLBACK,	STS_CB_ITEM_6_OK,		LOM_PAD_PRESS,

												LOM_PAD_ALLACT,			LOM_PAD_ALLOBJ,		LOM_PAD_CANCEL,	LOM_EXE_CALLBACK,	STS_CB_CANCELED,		LOM_PAD_PRESS,
												LOM_DATA_END,

					/* 開いた後用 */
					LOM_PADLIST_01,				ACTION_DUMMY,			LOM_PAD_ALLOBJ,		LOM_PAD_ANYINPUT,	LOM_EXE_CALLBACK,	STS_CB_OPENED,		0,
												LOM_DATA_END,

					/* 閉じた後用 */
					LOM_PADLIST_02,				ACTION_DUMMY,			LOM_PAD_ALLOBJ,		LOM_PAD_ANYINPUT,	LOM_EXE_CALLBACK,	STS_CB_CLOSED,		0,
												LOM_DATA_END,
					LOM_DATA_END,

					/*モード名*/	/*ＬＯＭアクションリスト*/	/*ＬＯＭオブジェクトリスト*/	/*ＬＯＭパッドリスト*/
LOM_MODE_DATA,		LOM_MODE_00,	LOM_ACTLIST_00,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* hide */
					LOM_MODE_01,	LOM_ACTLIST_01,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* open */
					LOM_MODE_02,	LOM_ACTLIST_02,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* dummy */
					LOM_MODE_03,	LOM_ACTLIST_03,				LOM_MODE_NOLIST,				LOM_PADLIST_02,		/* close */
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
 * スプライトのＲＧＢＡの設定
 */
static void SetSpriteABGR(
	LAYOUTMAN    *layoutman,		/* レイアウトマネージャ */
	int          strcode,			/* 文字列コード */
	int          active,			/* アクティブフラグ  0:非アクティブ  1:アクティブ */
	int          step,				/* 変化度 */
	unsigned int inactive_color,	/* 非アクティブ色 */
	unsigned int active_color)		/* アクティブ色 */
{
	SetSpriteColor(layoutman, strcode, active, step, inactive_color, active_color);
	SetSpriteAlpha(layoutman, strcode, active, step, (unsigned char)(inactive_color >> 24), (unsigned char)(active_color >> 24));
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

	if((param != STS_CB_OPENED) && (param != STS_CB_CLOSED))
	{
		if(!(work->flag & STS_INPUT_OK))
			return 0;
	}

	switch(param)
	{
	case STS_CB_ITEM_1_DN: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_2, 1); break;
	case STS_CB_ITEM_2_DN: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_3, 1); break;
	case STS_CB_ITEM_3_DN: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_4, 1); break;
	case STS_CB_ITEM_4_DN: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_5, 1); break;
	case STS_CB_ITEM_5_DN: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_6, 1); break;
	case STS_CB_ITEM_6_DN: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_1, 1); break;

	case STS_CB_ITEM_1_UP: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_6, 1); break;
	case STS_CB_ITEM_2_UP: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_1, 1); break;
	case STS_CB_ITEM_3_UP: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_2, 1); break;
	case STS_CB_ITEM_4_UP: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_3, 1); break;
	case STS_CB_ITEM_5_UP: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_4, 1); break;
	case STS_CB_ITEM_6_UP: GM_SdSet(SD_S_CUR01); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_5, 1); break;

	case STS_CB_CANCELED :
		if(LOM_GetCurObj(&work->layoutman) == OBJECT_ITEM_6)
			return 0;

		GM_SdSet(SD_S_CUR01);
		LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_6, 1);
		break;

	case STS_CB_ITEM_1_OK: work->index = 0; LOM_SetCurMode(&work->layoutman, LOM_MODE_03, 1, 1, 1); work->count = 0; GM_SdSet(SD_S_WIN01); GM_SdSet(SD_S_WINCLS01); break;
	case STS_CB_ITEM_2_OK: work->index = 1; LOM_SetCurMode(&work->layoutman, LOM_MODE_03, 1, 1, 1); work->count = 0; GM_SdSet(SD_S_WIN01); GM_SdSet(SD_S_WINCLS01); break;
	case STS_CB_ITEM_3_OK: work->index = 2; LOM_SetCurMode(&work->layoutman, LOM_MODE_03, 1, 1, 1); work->count = 0; GM_SdSet(SD_S_WIN01); GM_SdSet(SD_S_WINCLS01); break;
	case STS_CB_ITEM_4_OK: work->index = 3; LOM_SetCurMode(&work->layoutman, LOM_MODE_03, 1, 1, 1); work->count = 0; GM_SdSet(SD_S_WIN01); GM_SdSet(SD_S_WINCLS01); break;
	case STS_CB_ITEM_5_OK: work->index = 4; LOM_SetCurMode(&work->layoutman, LOM_MODE_03, 1, 1, 1); work->count = 0; GM_SdSet(SD_S_WIN01); GM_SdSet(SD_S_WINCLS01); break;
	case STS_CB_ITEM_6_OK: work->index = 5; LOM_SetCurMode(&work->layoutman, LOM_MODE_03, 1, 1, 1); work->count = 0; GM_SdSet(SD_S_START01 ); GM_SdSet(SD_S_WINCLS01); break;

	case STS_CB_OPENED:
		LOM_SetCurMode(&work->layoutman, LOM_MODE_02, 1, 1, 1);
		work->count  = 0;
		work->count2 = 0;
		work->flag |= STS_INPUT_OK;

		/* 初期カーソル位置 */
		switch(work->cursor)
		{
		case 0: LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_1, STS_ICO_R, STS_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_1, 1); break;
		case 1: LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_2, STS_ICO_R, STS_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_2, 1); break;
		case 2: LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_3, STS_ICO_R, STS_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_3, 1); break;
		case 3: LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_4, STS_ICO_R, STS_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_4, 1); break;
		case 4: LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_5, STS_ICO_R, STS_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_5, 1); break;
		case 5: LOM_SetIcoPos(&work->layoutman, OBJECT_ITEM_6, STS_ICO_R, STS_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); LOM_SetCurObj(&work->layoutman, OBJECT_ITEM_6, 1); break;
		}
		return 1;

	case STS_CB_CLOSED:
		/* プロック実行 */
		ASSERT(work->proc[(int)work->index]);
		SY_PRINTF1("PROC CALLED\n");
		GM_ExecProc(work->proc[(int)work->index], NULL);
		return 1;

	default:
		return 0;
	}

	work->flag  &= ~STS_INPUT_OK;
	work->count2 = -STS_INTERVAL_COUNT;

	return 1;
}

/******************************************************************************
 * 毎フレーム処理
 */
static void Act(
	WORK *work)		/* ワーク */
{
	static unsigned int col_i[2] = {VRDEF_PLAYABL_I, VRDEF_2ND_3RD_I};
	static unsigned int col_a[2] = {VRDEF_PLAYABL_A, VRDEF_2ND_3RD_A};

	if(work->flag & STS_INIT)
	{
		work->flag &= ~STS_INIT;
		work->flag |=  STS_INIT2;
		return;
	}
	if(work->flag & STS_INIT2)
	{
		/*
		 * レイアウトの初期化
		 */
		work->flag &= ~STS_INIT2;

		/* モードの設定 */
		LOM_SetCurMode(&work->layoutman, LOM_MODE_01, 1, 1, 1);
		work->count = 0;
		GM_SdSet(SD_S_LINEMOV1);
	}

	work->count++;
	work->count2++;
	if(work->count2 == 0)
		work->flag |= STS_INPUT_OK;

	/* レイアウトマネージャ */
	ActLayoutman(&work->layoutman);

	/* 各モードごとの処理 */
	if(LOM_GetCurMode(&work->layoutman) == LOM_MODE_01)
	{
		/* open */
#ifdef PAL
		if(work->count ==  8) GM_SdSet(SD_S_WINOPN01);
#else
		if(work->count == 10) GM_SdSet(SD_S_WINOPN01);
#endif

		SetSpriteColor(&work->layoutman, OBJECT_ITEM_1, 0, 256, col_i[(int)work->clear[0]], col_a[(int)work->clear[0]]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_2, 0, 256, col_i[(int)work->clear[1]], col_a[(int)work->clear[1]]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_3, 0, 256, col_i[(int)work->clear[2]], col_a[(int)work->clear[2]]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_4, 0, 256, col_i[(int)work->clear[3]], col_a[(int)work->clear[3]]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_5, 0, 256, col_i[(int)work->clear[4]], col_a[(int)work->clear[4]]);
		SetSpriteColor(&work->layoutman, OBJECT_ITEM_6, 0, 256, col_i[(int)0             ], col_a[(int)0             ]);
	}
	else if(LOM_GetCurMode(&work->layoutman) == LOM_MODE_02)
	{
		switch(LOM_GetCurObj(&work->layoutman))
		{
		case OBJECT_ITEM_1:
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_1, 1, 16, col_i[(int)work->clear[0]], col_a[(int)work->clear[0]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_2, 0, 16, col_i[(int)work->clear[1]], col_a[(int)work->clear[1]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_3, 0, 16, col_i[(int)work->clear[2]], col_a[(int)work->clear[2]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_4, 0, 16, col_i[(int)work->clear[3]], col_a[(int)work->clear[3]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_5, 0, 16, col_i[(int)work->clear[4]], col_a[(int)work->clear[4]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_6, 0, 16, col_i[(int)0             ], col_a[(int)0             ]);
			break;
		case OBJECT_ITEM_2:
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_1, 0, 16, col_i[(int)work->clear[0]], col_a[(int)work->clear[0]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_2, 1, 16, col_i[(int)work->clear[1]], col_a[(int)work->clear[1]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_3, 0, 16, col_i[(int)work->clear[2]], col_a[(int)work->clear[2]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_4, 0, 16, col_i[(int)work->clear[3]], col_a[(int)work->clear[3]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_5, 0, 16, col_i[(int)work->clear[4]], col_a[(int)work->clear[4]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_6, 0, 16, col_i[(int)0             ], col_a[(int)0             ]);
			break;
		case OBJECT_ITEM_3:
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_1, 0, 16, col_i[(int)work->clear[0]], col_a[(int)work->clear[0]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_2, 0, 16, col_i[(int)work->clear[1]], col_a[(int)work->clear[1]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_3, 1, 16, col_i[(int)work->clear[2]], col_a[(int)work->clear[2]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_4, 0, 16, col_i[(int)work->clear[3]], col_a[(int)work->clear[3]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_5, 0, 16, col_i[(int)work->clear[4]], col_a[(int)work->clear[4]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_6, 0, 16, col_i[(int)0             ], col_a[(int)0             ]);
			break;
		case OBJECT_ITEM_4:
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_1, 0, 16, col_i[(int)work->clear[0]], col_a[(int)work->clear[0]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_2, 0, 16, col_i[(int)work->clear[1]], col_a[(int)work->clear[1]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_3, 0, 16, col_i[(int)work->clear[2]], col_a[(int)work->clear[2]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_4, 1, 16, col_i[(int)work->clear[3]], col_a[(int)work->clear[3]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_5, 0, 16, col_i[(int)work->clear[4]], col_a[(int)work->clear[4]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_6, 0, 16, col_i[(int)0             ], col_a[(int)0             ]);
			break;
		case OBJECT_ITEM_5:
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_1, 0, 16, col_i[(int)work->clear[0]], col_a[(int)work->clear[0]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_2, 0, 16, col_i[(int)work->clear[1]], col_a[(int)work->clear[1]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_3, 0, 16, col_i[(int)work->clear[2]], col_a[(int)work->clear[2]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_4, 0, 16, col_i[(int)work->clear[3]], col_a[(int)work->clear[3]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_5, 1, 16, col_i[(int)work->clear[4]], col_a[(int)work->clear[4]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_6, 0, 16, col_i[(int)0             ], col_a[(int)0             ]);
			break;
		case OBJECT_ITEM_6:
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_1, 0, 16, col_i[(int)work->clear[0]], col_a[(int)work->clear[0]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_2, 0, 16, col_i[(int)work->clear[1]], col_a[(int)work->clear[1]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_3, 0, 16, col_i[(int)work->clear[2]], col_a[(int)work->clear[2]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_4, 0, 16, col_i[(int)work->clear[3]], col_a[(int)work->clear[3]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_5, 0, 16, col_i[(int)work->clear[4]], col_a[(int)work->clear[4]]);
			SetSpriteABGR(&work->layoutman, OBJECT_ITEM_6, 1, 16, col_i[(int)0             ], col_a[(int)0             ]);
			break;
		}
	}
	else if(LOM_GetCurMode(&work->layoutman) == LOM_MODE_03)
	{
		/* close */
#ifdef PAL
		if(work->count ==  8) GM_SdSet(SD_S_LINEMOV1);
#else
		if(work->count == 10) GM_SdSet(SD_S_LINEMOV1);
#endif

		switch(work->index)
		{
		case 0:
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1, 1, 256, col_i[(int)work->clear[0]], col_a[(int)work->clear[0]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2, 0, 256, col_i[(int)work->clear[1]], col_a[(int)work->clear[1]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3, 0, 256, col_i[(int)work->clear[2]], col_a[(int)work->clear[2]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4, 0, 256, col_i[(int)work->clear[3]], col_a[(int)work->clear[3]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5, 0, 256, col_i[(int)work->clear[4]], col_a[(int)work->clear[4]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_6, 0, 256, col_i[(int)0             ], col_a[(int)0             ]);
			break;
		case 1:
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1, 0, 256, col_i[(int)work->clear[0]], col_a[(int)work->clear[0]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2, 1, 256, col_i[(int)work->clear[1]], col_a[(int)work->clear[1]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3, 0, 256, col_i[(int)work->clear[2]], col_a[(int)work->clear[2]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4, 0, 256, col_i[(int)work->clear[3]], col_a[(int)work->clear[3]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5, 0, 256, col_i[(int)work->clear[4]], col_a[(int)work->clear[4]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_6, 0, 256, col_i[(int)0             ], col_a[(int)0             ]);
			break;
		case 2:
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1, 0, 256, col_i[(int)work->clear[0]], col_a[(int)work->clear[0]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2, 0, 256, col_i[(int)work->clear[1]], col_a[(int)work->clear[1]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3, 1, 256, col_i[(int)work->clear[2]], col_a[(int)work->clear[2]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4, 0, 256, col_i[(int)work->clear[3]], col_a[(int)work->clear[3]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5, 0, 256, col_i[(int)work->clear[4]], col_a[(int)work->clear[4]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_6, 0, 256, col_i[(int)0             ], col_a[(int)0             ]);
			break;
		case 3:
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1, 0, 256, col_i[(int)work->clear[0]], col_a[(int)work->clear[0]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2, 0, 256, col_i[(int)work->clear[1]], col_a[(int)work->clear[1]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3, 0, 256, col_i[(int)work->clear[2]], col_a[(int)work->clear[2]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4, 1, 256, col_i[(int)work->clear[3]], col_a[(int)work->clear[3]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5, 0, 256, col_i[(int)work->clear[4]], col_a[(int)work->clear[4]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_6, 0, 256, col_i[(int)0             ], col_a[(int)0             ]);
			break;
		case 4:
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1, 0, 256, col_i[(int)work->clear[0]], col_a[(int)work->clear[0]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2, 0, 256, col_i[(int)work->clear[1]], col_a[(int)work->clear[1]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3, 0, 256, col_i[(int)work->clear[2]], col_a[(int)work->clear[2]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4, 0, 256, col_i[(int)work->clear[3]], col_a[(int)work->clear[3]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5, 1, 256, col_i[(int)work->clear[4]], col_a[(int)work->clear[4]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_6, 0, 256, col_i[(int)0             ], col_a[(int)0             ]);
			break;
		case 5:
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_1, 0, 256, col_i[(int)work->clear[0]], col_a[(int)work->clear[0]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_2, 0, 256, col_i[(int)work->clear[1]], col_a[(int)work->clear[1]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_3, 0, 256, col_i[(int)work->clear[2]], col_a[(int)work->clear[2]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_4, 0, 256, col_i[(int)work->clear[3]], col_a[(int)work->clear[3]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_5, 0, 256, col_i[(int)work->clear[4]], col_a[(int)work->clear[4]]);
			SetSpriteColor(&work->layoutman, OBJECT_ITEM_6, 1, 256, col_i[(int)0             ], col_a[(int)0             ]);
			break;
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

chara スネークテイルズセレクト[NewSTSelect_Scn] $s:名前 \
	-proc   $p:ステージロードプロック ... \
	-fclear $f:クリアフラグ ... \
	-cursor $i:カーソル位置（０～） \
	-exit   $p:ＥＸＩＴプロック
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
	memset(work->clear, 0x00, sizeof(work->clear));

	work->name   = name;
	work->count  = 0;
	work->count2 = 0;
	work->flag   = 0;
	work->index  = -1;
	work->cursor = 0;

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

		/* fclear */
		if(GCL_GetOption('f') == NULL)
		{
			SY_PRINTF2("no fclear parameter.\n");
			return 0;
		}
		for(i = 0; i < 5; i++)
		{
			work->clear[i] = (GCL_GetNextInt() != 0);
		}

		/* cursor */
		if(GCL_GetOption('c') == NULL)
		{
			SY_PRINTF2("no cursor parameter.\n");
			return 0;
		}
		work->cursor = GCL_GetNextInt();
		if((work->cursor < 0) || (5 < work->cursor))
		{
			SY_PRINTF2("cursor parameter: invalid value");
			ASSERT(0);
		}

		/* exit */
		if(GCL_GetOption('e') == NULL)
		{
			SY_PRINTF2("no exit parameter.\n");
			return 0;
		}
		work->proc[5] = GCL_GetNextInt();

		SY_PRINTF1("Get Scn Option Succeeded.\n");
	}

	/* レイアウトマネージャ */
	{
		if(CreateLayoutman3(&work->layoutman, L2D_FILENAME, 4, 0, 0, 0, 0, LOMCallback, NULL, work, 20, 10, LOM_INPUT_DIRECT|LOM_KEY_REP_1) == 0) { SY_PRINTF2("Create Layoutman Failed.\n"); return 0; }
		SY_PRINTF1("Create Layoutman Succeeded.\n");
		if(LoadLOMData(&work->layoutman, _lom_data) == 0) { SY_PRINTF2("Load LOM Data Failed.\n"); return 0; }
		SY_PRINTF1("Load LOM Data Succeeded.\n");
	}

	work->flag |= STS_INIT;

	/* ポーズ無効 */
	GM_GameStatus |= STATE_PAUSE_DISABLE;

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
void *NewSTSelect(void)
{
	WORK *work;

	SY_PRINTF3("NewSTSelect\n");
	OPERATOR();

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
		if(!GetResources(work, 0, GM_CurrentMap))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}

void *NewSTSelect_Scn(
	int name,	/* シナリオ名 */
	int where)	/* マップＩＤ */
{
	WORK *work;

	SY_PRINTF3("NewSTSelect_Scn\n");
	OPERATOR();

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
		if(!GetResources(work, name, where))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}
