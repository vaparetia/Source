//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * layout manager - lom_set_cur.c
 * レイアウトマネージャ
 * 2002/04/16 S.Yamashita
 * $Id: lom_set_cur.c,v 1.1.1.3 2002/11/19 11:51:40 Yoshizawa1 Exp $
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

#include "lom.h"

/*******************************************************************************
 * public
 */
/*******************************************************************************
 * カレントＬＯＭアクションの設定
 *
 *   現在の LOM_ACTLIST の指定インデックスの LOM_ACT をカレントとし、カウンタを０にする。
 *   指定インデックスが、
 *     LOM_ACT_SPEED なら即座に指定速度に設定し、次の LOM_ACT を実行する。
 *     LOM_ACT_PAUSE なら一時停止状態にする。
 *     LOM_ACT_CONTI なら一時停止状態を解除する。
 *     LOM_ACT_COUNT か
 *     LOM_ACT_FRAME なら、現在のアクションを停止し、新しいアクションを開始する。
 */
int LOM_SetCurActI(		/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,		/* レイアウトマネージャ */
	int       lom_act_index)	/* ＬＯＭアクションインデックス */
{
	LOM_ACTLIST *lom_actlist;
	LOM_ACT     *lom_act;

	SY_PRINTF3("LOM_SetCurActI\n");

	ASSERT(layoutman != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(2)\n");
		return 0;
	}

	/* カレントＬＯＭアクションリストの取得 */
	if((lom_actlist = LOM_GetActlistI(layoutman, layoutman->c_lom_actlist_index)) == NULL)
	{
		SY_PRINTF2("No Current LOM Actlist.\n");
		return 0;
	}

	/* ＬＯＭアクションの取得 */
	if((lom_act = LOM_GetActI(lom_actlist, lom_act_index)) == NULL)
		return 0;

	/* カレントＬＯＭアクションを設定 */
	lom_actlist->c_lom_act_index = lom_act_index;
	layoutman->flag &= ~LOM_DEFAULT_ACTION;

	/* 実行可能アクションを検索 */
	while(1)
	{
		if((lom_act->flag & LOM_ACT_TYPEMASK) == LOM_ACT_SPEED)
		{
			/* 指定速度に設定 */
			if(L2D_SetActionPlaySpeed(layoutman->layout, lom_act->count) != L2D_STAT_ACK)
				SY_PRINTF1("Set Action Play Speed Failed.\n");
		}
		else if(lom_act->count != 0)
		{
			/* 実行可能アクションを発見 */
			break;
		}

		/* 次のＬＯＭアクション */
		if((lom_act = LOM_GetActI(lom_actlist, lom_actlist->c_lom_act_index + 1)) == NULL)
		{
			SY_PRINTF1("No Executable Action Found.\n");
			return 0;
		}
		lom_actlist->c_lom_act_index++;
	}

	if((lom_act->flag & LOM_ACT_TYPEMASK) == LOM_ACT_PAUSE)
	{
		/* 指定時間一時停止 */
		if(L2D_PauseAction(layoutman->layout, L2D_PAUSE) != L2D_STAT_ACK)
			SY_PRINTF1("Pause Action PAUSE Failed.\n");

		layoutman->act_count = 0;
		layoutman->count     = 0;

		return 1;
	}
	if((lom_act->flag & LOM_ACT_TYPEMASK) == LOM_ACT_CONTI)
	{
		/* 一時停止状態の解除 */
		if(L2D_PauseAction(layoutman->layout, L2D_CONTINUE) != L2D_STAT_ACK)
			SY_PRINTF1("Pause Action CONTINUE Failed.\n");

		layoutman->act_count = 0;
		layoutman->count     = 0;

		return 1;
	}

	/* 現在のアクションを停止 */
	if(L2D_BreakAction(layoutman->layout) != L2D_STAT_ACK)
		SY_PRINTF1("Break Action Failed.\n");

	/* 新しいアクションを開始 */
	if(L2D_EvokeAction(layoutman->layout, lom_act->act_name) != L2D_STAT_ACK)
	{
		SY_PRINTF1("Evoke Action[%d] Failed.\n", lom_act->act_name);
		return 0;
	}
	SY_PRINTF1("Evoke Action: %d\n", lom_act->act_name);

	layoutman->act_count = 0;
	layoutman->count     = 0;

	return 1;
}

/*******************************************************************************
 * カレントＬＯＭアクションの設定
 */
int LOM_SetCurAct(	/* 1: 成功 */
					/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       act_name)		/* アクション名 */
{
	int lom_act_index;

	SY_PRINTF3("LOM_SetCurAct\n");

	ASSERT(layoutman != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(3)\n");
		return 0;
	}

	if((lom_act_index = LOM_GetActIndex(
		LOM_GetActlistI(layoutman, layoutman->c_lom_actlist_index), act_name)) == -1)
	{
		SY_PRINTF2("Cannot Find LOM Act[%d] On The Current LOM Actlist.\n", act_name);
		return 0;
	}

	/* カレントＬＯＭアクションの設定 */
	return LOM_SetCurActI(layoutman, lom_act_index);
}

/*******************************************************************************
 * カレントＬＯＭアイコンの設定
 *
 *   現在の LOM_ICOLIST の指定インデックスの LOM_ICO をカレントとし、カウンタを０にする。
 *   前の LOM_ICO を非表示状態にし、新しい LOM_ICO を表示状態にする。
 */
int LOM_SetCurIcoI(		/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,		/* レイアウトマネージャ */
	int       lom_ico_index)	/* ＬＯＭアイコンインデックス */
{
	LOM_ICOLIST *lom_icolist;
	LOM_ICO     *lom_ico;
	LOM_ICO     *lom_ico_prev;

	SY_PRINTF3("LOM_SetCurIcoI\n");

	ASSERT(layoutman != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(4)\n");
		return 0;
	}

	/* カレントＬＯＭアイコンリストの取得 */
	if((lom_icolist = LOM_GetIcolistI(layoutman, layoutman->c_lom_icolist_index)) == NULL)
	{
		SY_PRINTF2("No Current LOM Icolist.\n");
		return 0;
	}

	/* ＬＯＭアイコンの取得 */
	if((lom_ico = LOM_GetIcoI(lom_icolist, lom_ico_index)) == NULL)
		return 0;

	/* 前のＬＯＭアイコンを非表示 */
	if((lom_ico_prev = LOM_GetIcoI(lom_icolist, lom_icolist->c_lom_ico_index)) == NULL)
	{
		SY_PRINTF2("Invalid LOM Ico Index.\n");
		return 0;
	}
	lom_ico_prev->sprite->sprite.head.head.flags |= SPR_FLAG_HIDDEN;

	/* カレントＬＯＭアイコンを設定 */
	lom_icolist->c_lom_ico_index = lom_ico_index;

	/* カレントＬＯＭアイコンを表示 */
	lom_ico->sprite->sprite.head.head.flags &= ~SPR_FLAG_HIDDEN;

	layoutman->ico_count = 0;

	return 1;
}

/*******************************************************************************
 * カレントＬＯＭアイコンの設定
 */
int LOM_SetCurIco(	/* 1: 成功 */
					/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       ico_name)		/* アイコン名 */
{
	int lom_ico_index;

	SY_PRINTF3("LOM_SetCurIco\n");

	ASSERT(layoutman != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(5)\n");
		return 0;
	}

	if((lom_ico_index = LOM_GetIcoIndex(
		LOM_GetIcolistI(layoutman, layoutman->c_lom_icolist_index), ico_name)) == -1)
	{
		SY_PRINTF2("Cannot Find LOM Ico[%d] On The Current LOM Icolist.\n", ico_name);
		return 0;
	}

	/* カレントＬＯＭアイコンの設定 */
	return LOM_SetCurIcoI(layoutman, lom_ico_index);
}

/*******************************************************************************
 * カレントＬＯＭオブジェクトの設定
 *
 *   現在の LOM_OBJLIST の指定インデックスの LOM_OBJ をカレントとし、カウンタを０にする。
 *   前の LOM_ICO を非表示状態にし、あたらしい LOM_ICOLIST と LOM_ICO をカレントとし、
 *   表示状態にし、カウンタを０にする。
 */
int LOM_SetCurObjI(		/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,		/* レイアウトマネージャ */
	int       lom_obj_index,	/* ＬＯＭオブジェクトインデックス */
	int       default_ico)		/* 1: ＬＯＭアイコンをデフォルトに設定する  0: しない */
{
	int         lom_icolist_index;
	LOM_OBJLIST *lom_objlist;
	LOM_OBJ     *lom_obj;
	LOM_ICOLIST *lom_icolist;
	LOM_ICO     *lom_ico;
	LOM_ICOLIST *lom_icolist_prev;
	LOM_ICO     *lom_ico_prev;

	SY_PRINTF3("LOM_SetCurObjI\n");

	ASSERT(layoutman != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(6)\n");
		return 0;
	}

	/* カレントＬＯＭオブジェクトリストの取得 */
	if((lom_objlist = LOM_GetObjlistI(layoutman, layoutman->c_lom_objlist_index)) == NULL)
	{
		SY_PRINTF2("No Current LOM Objlist.\n");
		return 0;
	}

	/* ＬＯＭオブジェクトの取得 */
	if((lom_obj = LOM_GetObjI(lom_objlist, lom_obj_index)) == NULL)
		return 0;

	/* ＬＯＭアイコンリストインデックスの取得 */
	if((lom_icolist_index = LOM_GetIcolistIndex(layoutman, lom_obj->icolist_name)) == -1)
	{
		SY_PRINTF2("Cannot Find LOM Icolist[%d].\n", lom_obj->icolist_name);
		return 0;
	}

	/* ＬＯＭアイコンリストの取得 */
	if((lom_icolist = LOM_GetIcolistI(layoutman, lom_icolist_index)) == NULL)
	{
		SY_PRINTF2("No Next LOM Icolist.\n");
		return 0;
	}

	/* ＬＯＭアイコンの取得 */
	if(default_ico == 1)
	{
		if((lom_ico = LOM_GetIcoI(lom_icolist, 0)) == NULL)
		{
			SY_PRINTF2("Invalid LOM Ico Index.\n");
			return 0;
		}
	}
	else
	{
		if((lom_ico = LOM_GetIcoI(lom_icolist, lom_icolist->c_lom_ico_index)) == NULL)
		{
			SY_PRINTF2("Invalid LOM Ico Index.\n");
			return 0;
		}
	}

	/* 前のＬＯＭアイコンリストの取得 */
	if((lom_icolist_prev = LOM_GetIcolistI(layoutman, layoutman->c_lom_icolist_index)) != NULL)
	{
		/* 前のＬＯＭアイコンを非表示 */
		if((lom_ico_prev = LOM_GetIcoI(lom_icolist_prev, lom_icolist_prev->c_lom_ico_index)) == NULL)
		{
			SY_PRINTF2("Invalid LOM Ico Index.\n");
			return 0;
		}
		lom_ico_prev->sprite->sprite.head.head.flags |= SPR_FLAG_HIDDEN;
	}

	/* カレントＬＯＭオブジェクトを設定 */
	lom_objlist->c_lom_obj_index = lom_obj_index;

	/* カレントＬＯＭアイコンリストを設定 */
	layoutman->c_lom_icolist_index = lom_icolist_index;

	/* カレントＬＯＭアイコンを設定 */
	if(default_ico == 1)
		lom_icolist->c_lom_ico_index = 0;

	/* カレントＬＯＭアイコンを表示 */
	lom_ico->sprite->sprite.head.head.flags &= ~SPR_FLAG_HIDDEN;

	/* 移動元位置を変更 */
	layoutman->ico_pos[0][LOM_LEFT  ] = layoutman->ico_pos[1][LOM_LEFT  ];
	layoutman->ico_pos[0][LOM_TOP   ] = layoutman->ico_pos[1][LOM_TOP   ];
	layoutman->ico_pos[0][LOM_RIGHT ] = layoutman->ico_pos[1][LOM_RIGHT ];
	layoutman->ico_pos[0][LOM_BOTTOM] = layoutman->ico_pos[1][LOM_BOTTOM];

	layoutman->ico_count = 0;
	layoutman->obj_count = 0;

	return 1;
}

/*******************************************************************************
 * カレントＬＯＭオブジェクトの設定
 */
int LOM_SetCurObj(	/* 1: 成功 */
					/* 0: 失敗 */
	LAYOUTMAN *layoutman,	/* レイアウトマネージャ */
	int       obj_name,		/* オブジェクト名 */
	int       default_ico)	/* 1: ＬＯＭアイコンをデフォルトに設定する  0: しない */
{
	int lom_obj_index;

	SY_PRINTF3("LOM_SetCurObj\n");

	ASSERT(layoutman != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(7)\n");
		return 0;
	}

	if((lom_obj_index = LOM_GetObjIndex(
		LOM_GetObjlistI(layoutman, layoutman->c_lom_objlist_index), obj_name)) == -1)
	{
		SY_PRINTF2("Cannot Find LOM Obj[%d] On The Current LOM Objlist.\n", obj_name);
		return 0;
	}

	/* カレントＬＯＭオブジェクトの設定 */
	return LOM_SetCurObjI(layoutman, lom_obj_index, default_ico);
}

/*******************************************************************************
 * カレントＬＯＭモードの設定
 */
int LOM_SetCurModeI(	/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,		/* レイアウトマネージャ */
	int       lom_mode_index,	/* ＬＯＭモードインデックス */
	int       default_act,		/* 1: ＬＯＭアクションを　　デフォルトに設定する  0: しない  -1: 処理をしない */
	int       default_ico,		/* 1: ＬＯＭアイコンを　　　デフォルトに設定する  0: しない */
	int       default_obj)		/* 1: ＬＯＭオブジェクトを　デフォルトに設定する  0: しない */
{
	LOM_MODE    *lom_mode;
	LOM_ICOLIST *lom_icolist;
	LOM_ICO     *lom_ico;

	SY_PRINTF3("LOM_SetCurModeI\n");

	ASSERT(layoutman != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(8)\n");
		return 0;
	}

	/* ＬＯＭモードの取得 */
	if((lom_mode = LOM_GetModeI(layoutman, lom_mode_index)) == NULL)
	{
		SY_PRINTF1("Invalid LOM Mode Index.\n");
		return 0;
	}

	/* カレントＬＯＭオブジェクトリストを設定 */
	layoutman->c_lom_objlist_index = lom_mode->lom_objlist_index;
	if(layoutman->c_lom_objlist_index != LOM_MODE_NOLIST)
	{
		if(default_obj == 1)
		{
			LOM_SetCurObjI(layoutman, 0, default_ico);
		}
		else
		{
			LOM_SetCurObjI(layoutman, layoutman->lom_objlist[layoutman->c_lom_objlist_index].c_lom_obj_index, default_ico);
		}
	}
	else
	{
		/* 前のＬＯＭアイコンリストの取得 */
		if((lom_icolist = LOM_GetIcolistI(layoutman, layoutman->c_lom_icolist_index)) != NULL)
		{
			/* 前のＬＯＭアイコンを非表示 */
			if((lom_ico = LOM_GetIcoI(lom_icolist, lom_icolist->c_lom_ico_index)) == NULL)
			{
				SY_PRINTF2("Invalid LOM Ico Index.\n");
				return 0;
			}
			lom_ico->sprite->sprite.head.head.flags |= SPR_FLAG_HIDDEN;
		}
		layoutman->c_lom_icolist_index = LOM_MODE_NOLIST;
	}

	/* カレントＬＯＭアクションリストを設定 */
	layoutman->act_count = 0;
	layoutman->count     = 0;
	layoutman->c_lom_actlist_index = lom_mode->lom_actlist_index;
	if(default_act == 1)
	{
		LOM_SetCurActI(layoutman, 0);
	}
	else if(default_act == 0)
	{
		LOM_SetCurActI(layoutman, layoutman->lom_actlist[layoutman->c_lom_actlist_index].c_lom_act_index);
	}

	/* カレントＬＯＭパッドリストを設定 */
	layoutman->c_lom_padlist_index = lom_mode->lom_padlist_index;

	/* カレントＬＯＭモードを設定 */
	layoutman->c_lom_mode_index = lom_mode_index;

	return 1;
}

/*******************************************************************************
 * カレントＬＯＭモードの設定
 */
int LOM_SetCurMode(		/* 1: 成功 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman,		/* レイアウトマネージャ */
	int       lom_mode_name,	/* ＬＯＭモード名 */
	int       default_act,		/* 1: ＬＯＭアクションを　　デフォルトに設定する  0: しない  -1: 処理をしない */
	int       default_ico,		/* 1: ＬＯＭアイコンを　　　デフォルトに設定する  0: しない */
	int       default_obj)		/* 1: ＬＯＭオブジェクトを　デフォルトに設定する  0: しない */
{
	int lom_mode_index;

	SY_PRINTF3("LOM_SetCurMode\n");

	ASSERT(layoutman != NULL);

	if((layoutman->flag & LOM_ENABLED) == 0)
	{
		SY_PRINTF2("Layoutman Not Enabled.(9)\n");
		return 0;
	}

	if((lom_mode_index = LOM_GetModeIndex(layoutman, lom_mode_name)) == -1)
	{
		SY_PRINTF2("Cannot Find LOM Mode[%d].\n", lom_mode_name);
		return 0;
	}

	/* カレントＬＯＭモードの設定 */
	return LOM_SetCurModeI(layoutman, lom_mode_index, default_act, default_ico, default_obj);
}
