//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * layout manager - lom_new.c
 * レイアウトマネージャ
 * 2002/04/16 S.Yamashita
 * $Id: lom_new.c,v 1.1.1.3 2002/11/19 11:51:40 Yoshizawa1 Exp $
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
 * 新規ＬＯＭアクションリスト
 */
int LOM_NewActlist(		/* ＬＯＭアクションリスト配列の新規要素のインデックス */
						/* -1: 失敗 */
	LAYOUTMAN *layoutman,		/* レイアウトマネージャ */
	int       actlist_name,		/* アクションリスト名 */
	int       n_lom_act)		/* ＬＯＭアクション数 */
{
	LOM_ACTLIST *lom_actlist;

	SY_PRINTF3("LOM_NewActlist\n");

	ASSERT(layoutman != NULL);

	if(n_lom_act <= 0)
	{
		SY_PRINTF2("n_lom_act Is 0 Or Less.\n");
		return -1;
	}

	/* ＬＯＭアクションリスト配列のメモリの確保 */
	if((lom_actlist = (LOM_ACTLIST *)GV_Malloc(
		sizeof(LOM_ACTLIST) * (layoutman->n_lom_actlist + 1))) == NULL)
	{
		SY_PRINTF2("No Memory.\n");
		return -1;
	}
	memset(lom_actlist, 0x00, sizeof(LOM_ACTLIST) * (layoutman->n_lom_actlist + 1));

	/* ＬＯＭアクション配列のメモリの確保 */
	if((lom_actlist[layoutman->n_lom_actlist].lom_act = (LOM_ACT *)GV_Malloc(
		sizeof(LOM_ACT) * n_lom_act)) == NULL)
	{
		SY_PRINTF2("No Memory.\n");
		GV_Free(lom_actlist);
		return -1;
	}
	memset(lom_actlist[layoutman->n_lom_actlist].lom_act, 0x00, sizeof(LOM_ACT) * n_lom_act);

	/* ＬＯＭアクションリスト配列に新規要素の追加 */
	if(layoutman->lom_actlist)
	{
		memcpy(lom_actlist, layoutman->lom_actlist,
			sizeof(LOM_ACTLIST) * layoutman->n_lom_actlist);
		GV_Free(layoutman->lom_actlist);
	}
	layoutman->lom_actlist = lom_actlist;
	layoutman->n_lom_actlist++;

	/* 新規ＬＯＭアクションリストの初期化 */
	lom_actlist = &layoutman->lom_actlist[layoutman->n_lom_actlist - 1];
	lom_actlist->actlist_name = actlist_name;
	lom_actlist->n_lom_act    = n_lom_act;

	return layoutman->n_lom_actlist - 1;
}

/*******************************************************************************
 * 新規ＬＯＭアイコンリスト
 */
int LOM_NewIcolist(		/* ＬＯＭアイコンリスト配列の新規要素のインデックス */
						/* -1: 失敗 */
	LAYOUTMAN *layoutman,		/* レイアウトマネージャ */
	int       icolist_name,		/* アイコンリスト名 */
	int       n_lom_ico)		/* ＬＯＭアイコン数 */
{
	LOM_ICOLIST *lom_icolist;

	SY_PRINTF3("LOM_NewIcolist\n");

	ASSERT(layoutman != NULL);

	if(n_lom_ico <= 0)
	{
		SY_PRINTF2("n_lom_ico Is 0 Or Less.\n");
		return -1;
	}

	/* ＬＯＭアイコンリスト配列のメモリの確保 */
	if((lom_icolist = (LOM_ICOLIST *)GV_Malloc(
		sizeof(LOM_ICOLIST) * (layoutman->n_lom_icolist + 1))) == NULL)
	{
		SY_PRINTF2("No Memory.\n");
		return -1;
	}
	memset(lom_icolist, 0x00, sizeof(LOM_ICOLIST) * (layoutman->n_lom_icolist + 1));

	/* ＬＯＭアイコン配列のメモリの確保 */
	if((lom_icolist[layoutman->n_lom_icolist].lom_ico = (LOM_ICO *)GV_Malloc(
		sizeof(LOM_ICO) * n_lom_ico)) == NULL)
	{
		SY_PRINTF2("No Memory.\n");
		GV_Free(lom_icolist);
		return -1;
	}
	memset(lom_icolist[layoutman->n_lom_icolist].lom_ico, 0x00, sizeof(LOM_ICO) * n_lom_ico);

	/* ＬＯＭアイコンリスト配列に新規要素の追加 */
	if(layoutman->lom_icolist)
	{
		memcpy(lom_icolist, layoutman->lom_icolist,
			sizeof(LOM_ICOLIST) * layoutman->n_lom_icolist);
		GV_Free(layoutman->lom_icolist);
	}
	layoutman->lom_icolist = lom_icolist;
	layoutman->n_lom_icolist++;

	/* 新規ＬＯＭアイコンリストの初期化 */
	lom_icolist = &layoutman->lom_icolist[layoutman->n_lom_icolist - 1];
	lom_icolist->icolist_name = icolist_name;
	lom_icolist->n_lom_ico    = n_lom_ico;

	return layoutman->n_lom_icolist - 1;
}

/*******************************************************************************
 * 新規ＬＯＭオブジェクトリスト
 */
int LOM_NewObjlist(		/* ＬＯＭオブジェクトリスト配列の新規要素のインデックス */
						/* -1: 失敗 */
	LAYOUTMAN *layoutman,		/* レイアウトマネージャ */
	int       objlist_name,		/* オブジェクトリスト名 */
	int       n_lom_obj)		/* ＬＯＭオブジェクト数 */
{
	LOM_OBJLIST *lom_objlist;

	SY_PRINTF3("LOM_NewObjlist\n");

	ASSERT(layoutman != NULL);

	if(n_lom_obj <= 0)
	{
		SY_PRINTF2("n_lom_obj Is 0 Or Less.\n");
		return -1;
	}

	/* ＬＯＭオブジェクトリスト配列のメモリの確保 */
	if((lom_objlist = (LOM_OBJLIST *)GV_Malloc(
		sizeof(LOM_OBJLIST) * (layoutman->n_lom_objlist + 1))) == NULL)
	{
		SY_PRINTF2("No Memory.\n");
		return -1;
	}
	memset(lom_objlist, 0x00, sizeof(LOM_OBJLIST) * (layoutman->n_lom_objlist + 1));

	/* ＬＯＭオブジェクト配列のメモリの確保 */
	if((lom_objlist[layoutman->n_lom_objlist].lom_obj = (LOM_OBJ *)GV_Malloc(
		sizeof(LOM_OBJ) * n_lom_obj)) == NULL)
	{
		SY_PRINTF2("No Memory.\n");
		GV_Free(lom_objlist);
		return -1;
	}
	memset(lom_objlist[layoutman->n_lom_objlist].lom_obj, 0x00, sizeof(LOM_OBJ) * n_lom_obj);

	/* ＬＯＭオブジェクトリスト配列に新規要素の追加 */
	if(layoutman->lom_objlist)
	{
		memcpy(lom_objlist, layoutman->lom_objlist,
			sizeof(LOM_OBJLIST) * layoutman->n_lom_objlist);
		GV_Free(layoutman->lom_objlist);
	}
	layoutman->lom_objlist = lom_objlist;
	layoutman->n_lom_objlist++;

	/* 新規ＬＯＭオブジェクトリストの初期化 */
	lom_objlist = &layoutman->lom_objlist[layoutman->n_lom_objlist - 1];
	lom_objlist->objlist_name = objlist_name;
	lom_objlist->n_lom_obj    = n_lom_obj;

	return layoutman->n_lom_objlist - 1;
}

/*******************************************************************************
 * 新規ＬＯＭパッドリスト
 */
int LOM_NewPadlist(		/* ＬＯＭパッドリスト配列の新規要素のインデックス */
						/* -1: 失敗 */
	LAYOUTMAN *layoutman,		/* レイアウトマネージャ */
	int       padlist_name,		/* パッドリスト名 */
	int       n_lom_pad)		/* ＬＯＭパッド数 */
{
	LOM_PADLIST *lom_padlist;

	SY_PRINTF3("LOM_NewPadlist\n");

	ASSERT(layoutman != NULL);

	if(n_lom_pad <= 0)
	{
		SY_PRINTF2("n_lom_pad Is 0 Or Less.\n");
		return -1;
	}

	/* ＬＯＭパッドリスト配列のメモリの確保 */
	if((lom_padlist = (LOM_PADLIST *)GV_Malloc(
		sizeof(LOM_PADLIST) * (layoutman->n_lom_padlist + 1))) == NULL)
	{
		SY_PRINTF2("No Memory.\n");
		return -1;
	}
	memset(lom_padlist, 0x00, sizeof(LOM_PADLIST) * (layoutman->n_lom_padlist + 1));

	/* ＬＯＭパッド配列のメモリの確保 */
	if((lom_padlist[layoutman->n_lom_padlist].lom_pad = (LOM_PAD *)GV_Malloc(
		sizeof(LOM_PAD) * n_lom_pad)) == NULL)
	{
		SY_PRINTF2("No Memory.\n");
		GV_Free(lom_padlist);
		return -1;
	}
	memset(lom_padlist[layoutman->n_lom_padlist].lom_pad, 0x00, sizeof(LOM_PAD) * n_lom_pad);

	/* ＬＯＭパッドリスト配列に新規要素の追加 */
	if(layoutman->lom_padlist)
	{
		memcpy(lom_padlist, layoutman->lom_padlist,
			sizeof(LOM_PADLIST) * layoutman->n_lom_padlist);
		GV_Free(layoutman->lom_padlist);
	}
	layoutman->lom_padlist = lom_padlist;
	layoutman->n_lom_padlist++;

	/* 新規ＬＯＭパッドリストの初期化 */
	lom_padlist = &layoutman->lom_padlist[layoutman->n_lom_padlist - 1];
	lom_padlist->padlist_name = padlist_name;
	lom_padlist->n_lom_pad    = n_lom_pad;

	return layoutman->n_lom_padlist - 1;
}

/*******************************************************************************
 * 新規ＬＯＭモード
 */
int LOM_NewMode(	/* ＬＯＭモード配列の新規要素のインデックス */
					/* -1: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       mode_name,			/* モード名 */
	int       lom_actlist_index,	/* ＬＯＭアクションリストインデックス */
	int       lom_objlist_index,	/* ＬＯＭオブジェクトリストインデックス */
	int       lom_padlist_index)	/* ＬＯＭパッドリストインデックス */
{
	LOM_MODE *lom_mode;

	SY_PRINTF3("LOM_NewMode\n");

	ASSERT(layoutman != NULL);

	if(    (lom_actlist_index >= layoutman->n_lom_actlist)
		|| (lom_objlist_index >= layoutman->n_lom_objlist)
		|| (lom_padlist_index >= layoutman->n_lom_padlist))
	{
		SY_PRINTF2("Invalid List Index.\n");
		return -1;
	}

	/* ＬＯＭモード配列のメモリの確保 */
	if((lom_mode = (LOM_MODE *)GV_Malloc(
		sizeof(LOM_MODE) * (layoutman->n_lom_mode + 1))) == NULL)
	{
		SY_PRINTF2("No Memory.\n");
		return -1;
	}
	memset(lom_mode, 0x00, sizeof(LOM_MODE) * (layoutman->n_lom_mode + 1));

	/* ＬＯＭモード配列に新規要素の追加 */
	if(layoutman->lom_mode)
	{
		memcpy(lom_mode, layoutman->lom_mode,
			sizeof(LOM_MODE) * layoutman->n_lom_mode);
		GV_Free(layoutman->lom_mode);
	}
	layoutman->lom_mode = lom_mode;
	layoutman->n_lom_mode++;

	/* 新規ＬＯＭモードの初期化 */
	lom_mode = &layoutman->lom_mode[layoutman->n_lom_mode - 1];
	lom_mode->mode_name         = mode_name;
	lom_mode->lom_actlist_index = lom_actlist_index;
	lom_mode->lom_objlist_index = lom_objlist_index;
	lom_mode->lom_padlist_index = lom_padlist_index;

	return layoutman->n_lom_mode - 1;
}

/*******************************************************************************
 * 新規ＬＯＭモード
 */
int LOM_NewMode2(	/* ＬＯＭモード配列の新規要素のインデックス */
					/* -1: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       mode_name,			/* モード名 */
	int       lom_actlist_name,		/* ＬＯＭアクションリスト名 */
	int       lom_objlist_name,		/* ＬＯＭオブジェクトリスト名 */
	int       lom_padlist_name)		/* ＬＯＭパッドリスト名 */
{
	int lom_actlist_index;
	int lom_objlist_index;
	int lom_padlist_index;

	SY_PRINTF3("LOM_NewMode2\n");

	ASSERT(layoutman != NULL);

	/* ＬＯＭアクションリスト */
	if((lom_actlist_index = LOM_GetActlistIndex(layoutman, lom_actlist_name)) == -1)
	{
		SY_PRINTF2("Cannot find LOM Actionlist.\n");
		return -1;
	}

	/* ＬＯＭオブジェクトリスト */
	if((lom_objlist_index = LOM_GetObjlistIndex(layoutman, lom_objlist_name)) == -1)
		lom_objlist_index = LOM_MODE_NOLIST;

	/* ＬＯＭパッドリスト */
	if((lom_padlist_index = LOM_GetPadlistIndex(layoutman, lom_padlist_name)) == -1)
		lom_padlist_index = LOM_MODE_NOLIST;

	/* 新規ＬＯＭモード */
	return LOM_NewMode(layoutman, mode_name, lom_actlist_index, lom_objlist_index, lom_padlist_index);
}
