//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * layout manager - lom_get_cur.c
 * レイアウトマネージャ
 * 2002/05/27 S.Yamashita
 * $Id: lom_get_cur.c,v 1.1.1.3 2002/11/19 11:51:40 Yoshizawa1 Exp $
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
 * カレントＬＯＭアクションリストの取得
 */
int LOM_GetCurActList(	/* カレントＬＯＭアクションリスト名 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman)	/* レイアウトマネージャ */
{
	LOM_ACTLIST *lom_actlist;

	SY_PRINTF3("LOM_GetCurActList\n");

	ASSERT(layoutman != NULL);

	if((lom_actlist = LOM_GetActlistI(layoutman, layoutman->c_lom_actlist_index)) == NULL)
	{
		SY_PRINTF2("Cannot Find LOM Actlist[Index: %d].\n", layoutman->c_lom_actlist_index);
		return 0;
	}

	return lom_actlist->actlist_name;
}

/*******************************************************************************
 * カレントＬＯＭアクションリストの取得
 */
int LOM_GetCurActListI(		/* カレントＬＯＭアクションリストインデックス */
							/* -1: 失敗 */
	LAYOUTMAN *layoutman)	/* レイアウトマネージャ */
{
	SY_PRINTF3("LOM_GetCurActListI\n");

	ASSERT(layoutman != NULL);

	if(    (layoutman->c_lom_actlist_index < 0)
		|| (layoutman->n_lom_actlist - 1 < layoutman->c_lom_actlist_index))
		return -1;

	return layoutman->c_lom_actlist_index;
}

/*******************************************************************************
 * カレントアクションの取得
 */
int LOM_GetCurAct(	/* カレントアクション名 */
					/* 0: 失敗 */
	LAYOUTMAN *layoutman)	/* レイアウトマネージャ */
{
	LOM_ACTLIST *lom_actlist;
	LOM_ACT     *lom_act;

	SY_PRINTF3("LOM_GetCurAct\n");

	ASSERT(layoutman != NULL);

	if((lom_actlist = LOM_GetActlistI(layoutman, layoutman->c_lom_actlist_index)) == NULL)
	{
		SY_PRINTF2("Cannot Find LOM Actlist[Index: %d].\n", layoutman->c_lom_actlist_index);
		return 0;
	}

	if((lom_act = LOM_GetActI(lom_actlist, lom_actlist->c_lom_act_index)) == NULL)
	{
		SY_PRINTF2("Cannot Find LOM Act[Index: %d].\n", lom_actlist->c_lom_act_index);
		return 0;
	}

	return lom_act->act_name;
}

/*******************************************************************************
 * カレントＬＯＭアクションの取得
 */
int LOM_GetCurActI(		/* カレントＬＯＭアクションインデックス */
						/* -1: 失敗 */
	LAYOUTMAN *layoutman)	/* レイアウトマネージャ */
{
	LOM_ACTLIST *lom_actlist;

	SY_PRINTF3("LOM_GetCurActI\n");

	ASSERT(layoutman != NULL);

	if((lom_actlist = LOM_GetActlistI(layoutman, layoutman->c_lom_actlist_index)) == NULL)
	{
		SY_PRINTF2("Cannot Find LOM Actlist[Index: %d].\n", layoutman->c_lom_actlist_index);
		return -1;
	}

	if(    (lom_actlist->c_lom_act_index < 0)
		|| (lom_actlist->n_lom_act - 1 < lom_actlist->c_lom_act_index))
		return -1;

	return lom_actlist->c_lom_act_index;
}

/*******************************************************************************
 * カレントＬＯＭオブジェクトリストの取得
 */
int LOM_GetCurObjList(	/* カレントＬＯＭオブジェクトリスト名 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman)	/* レイアウトマネージャ */
{
	LOM_OBJLIST *lom_objlist;

	SY_PRINTF3("LOM_GetCurObjList\n");

	ASSERT(layoutman != NULL);

	if((lom_objlist = LOM_GetObjlistI(layoutman, layoutman->c_lom_objlist_index)) == NULL)
	{
//		SY_PRINTF1("Cannot Find LOM Objlist[Index: %d].\n", layoutman->c_lom_objlist_index);
		return 0;
	}

	return lom_objlist->objlist_name;
}

/*******************************************************************************
 * カレントＬＯＭオブジェクトリストの取得
 */
int LOM_GetCurObjListI(		/* カレントＬＯＭオブジェクトリストインデックス */
							/* -1: 失敗 */
	LAYOUTMAN *layoutman)	/* レイアウトマネージャ */
{
	SY_PRINTF3("LOM_GetCurObjListI\n");

	ASSERT(layoutman != NULL);

	if(    (layoutman->c_lom_objlist_index < 0)
		|| (layoutman->n_lom_objlist - 1 < layoutman->c_lom_objlist_index))
		return -1;

	return layoutman->c_lom_objlist_index;
}

/*******************************************************************************
 * カレントオブジェクトの取得
 */
int LOM_GetCurObj(	/* カレントオブジェクト名 */
					/* 0: 失敗 */
	LAYOUTMAN *layoutman)	/* レイアウトマネージャ */
{
	LOM_OBJLIST *lom_objlist;
	LOM_OBJ     *lom_obj;

	SY_PRINTF3("LOM_GetCurObj\n");

	ASSERT(layoutman != NULL);

	if((lom_objlist = LOM_GetObjlistI(layoutman, layoutman->c_lom_objlist_index)) == NULL)
	{
//		SY_PRINTF1("Cannot Find LOM Objlist[Index: %d].\n", layoutman->c_lom_objlist_index);
		return 0;
	}

	if((lom_obj = LOM_GetObjI(lom_objlist, lom_objlist->c_lom_obj_index)) == NULL)
	{
		SY_PRINTF2("Cannot Find LOM Obj[Index: %d].\n", lom_objlist->c_lom_obj_index);
		return 0;
	}

	return lom_obj->obj_name;
}

/*******************************************************************************
 * カレントＬＯＭオブジェクトの取得
 */
int LOM_GetCurObjI(		/* カレントＬＯＭオブジェクトインデックス */
						/* -1: 失敗 */
	LAYOUTMAN *layoutman)	/* レイアウトマネージャ */
{
	LOM_OBJLIST *lom_objlist;

	SY_PRINTF3("LOM_GetCurObjI\n");

	ASSERT(layoutman != NULL);

	if((lom_objlist = LOM_GetObjlistI(layoutman, layoutman->c_lom_objlist_index)) == NULL)
	{
//		SY_PRINTF1("Cannot Find LOM Objlist[Index: %d].\n", layoutman->c_lom_objlist_index);
		return 0;
	}

	if(    (lom_objlist->c_lom_obj_index < 0)
		|| (lom_objlist->n_lom_obj - 1 < lom_objlist->c_lom_obj_index))
		return -1;

	return lom_objlist->c_lom_obj_index;
}

/*******************************************************************************
 * カレントＬＯＭモードの取得
 */
int LOM_GetCurMode(		/* カレントＬＯＭモード名 */
						/* 0: 失敗 */
	LAYOUTMAN *layoutman)	/* レイアウトマネージャ */
{
	LOM_MODE *lom_mode;

	SY_PRINTF3("LOM_GetCurMode\n");

	ASSERT(layoutman != NULL);

	if((lom_mode = LOM_GetModeI(layoutman, layoutman->c_lom_mode_index)) == NULL)
	{
		SY_PRINTF2("Cannot Find LOM Mode[Index: %d].\n", layoutman->c_lom_mode_index);
		return 0;
	}

	return lom_mode->mode_name;
}

/*******************************************************************************
 * カレントＬＯＭモードの取得
 */
int LOM_GetCurModeI(	/* カレントＬＯＭモードインデックス */
						/* -1: 失敗 */
	LAYOUTMAN *layoutman)	/* レイアウトマネージャ */
{
	SY_PRINTF3("LOM_GetCurModeI\n");

	ASSERT(layoutman != NULL);

	if(    (layoutman->c_lom_mode_index < 0)
		|| (layoutman->n_lom_mode - 1 < layoutman->c_lom_mode_index))
		return -1;

	return layoutman->c_lom_mode_index;
}
