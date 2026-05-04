//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * layout manager - lom_get.c
 * レイアウトマネージャ
 * 2002/04/16 S.Yamashita
 * $Id: lom_get.c,v 1.1.1.3 2002/11/19 11:51:39 Yoshizawa1 Exp $
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
 * ＬＯＭアクションリストインデックスの取得
 */
int LOM_GetActlistIndex(	/* ＬＯＭアクションリストインデックス */
							/* -1: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       lom_actlist_name)		/* ＬＯＭアクションリスト名 */
{
	int i;

	SY_PRINTF3("LOM_GetActlistIndex\n");

	ASSERT(layoutman != NULL);

	/* ＬＯＭアクションリストの検索 */
	for(i = 0; i < layoutman->n_lom_actlist; i++)
	{
		if(layoutman->lom_actlist[i].actlist_name == lom_actlist_name)
		{
			return i;
		}
	}

	return -1;
}

/*******************************************************************************
 * ＬＯＭアクションリストの取得
 */
LOM_ACTLIST *LOM_GetActlist(	/* ＬＯＭアクションリスト */
								/* NULL: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       lom_actlist_name)		/* ＬＯＭアクションリスト名 */
{
	int lom_actlist_index;

	SY_PRINTF3("LOM_GetActlist\n");

	ASSERT(layoutman != NULL);

	if((lom_actlist_index = LOM_GetActlistIndex(layoutman, lom_actlist_name)) == -1)
	{
		SY_PRINTF2("Cannot Find LOM Actlist[%d].\n", lom_actlist_name);
		return NULL;
	}

	return &layoutman->lom_actlist[lom_actlist_index];
}

/*******************************************************************************
 * ＬＯＭアクションリストの取得
 */
LOM_ACTLIST *LOM_GetActlistI(	/* ＬＯＭアクションリスト */
								/* NULL: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       lom_actlist_index)	/* ＬＯＭアクションリストインデックス */
{
	SY_PRINTF3("LOM_GetActlistI\n");

	ASSERT(layoutman != NULL);

	if((lom_actlist_index < 0) || (layoutman->n_lom_actlist - 1 < lom_actlist_index))
		return NULL;

	return &layoutman->lom_actlist[lom_actlist_index];
}

/*******************************************************************************
 * ＬＯＭアクションインデックスの取得
 */
int LOM_GetActIndex(	/* ＬＯＭアクションインデックス */
						/* -1: 失敗 */
	LOM_ACTLIST *lom_actlist,	/* ＬＯＭアクションリスト */
	int         lom_act_name)	/* ＬＯＭアクション名 */
{
	int i;

	SY_PRINTF3("LOM_GetActIndex\n");

	if(lom_actlist == NULL)
		return -1;

	/* ＬＯＭアクションリストの検索 */
	for(i = 0; i < lom_actlist->n_lom_act; i++)
	{
		if(lom_actlist->lom_act[i].act_name == lom_act_name)
		{
			return i;
		}
	}

	return -1;
}

/*******************************************************************************
 * ＬＯＭアクションの取得
 */
LOM_ACT *LOM_GetAct(	/* ＬＯＭアクション */
						/* NULL: 失敗 */
	LOM_ACTLIST *lom_actlist,	/* ＬＯＭアクションリスト */
	int         lom_act_name)	/* ＬＯＭアクション名 */
{
	int lom_act_index;

	SY_PRINTF3("LOM_GetAct\n");

	if(lom_actlist == NULL)
		return NULL;

	if((lom_act_index = LOM_GetActIndex(lom_actlist, lom_act_name)) == -1)
	{
		SY_PRINTF2("Cannot Find LOM Act[%d].\n", lom_act_name);
		return NULL;
	}

	return &lom_actlist->lom_act[lom_act_index];
}

/*******************************************************************************
 * ＬＯＭアクションの取得
 */
LOM_ACT *LOM_GetAct2(	/* ＬＯＭアクション */
						/* NULL: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       lom_actlist_name,		/* ＬＯＭアクションリスト名 */
	int       lom_act_name)			/* ＬＯＭアクション名 */
{
	SY_PRINTF3("LOM_GetAct2\n");

	ASSERT(layoutman != NULL);

	return LOM_GetAct(LOM_GetActlist(layoutman, lom_actlist_name), lom_act_name);
}

/*******************************************************************************
 * ＬＯＭアクションの取得
 */
LOM_ACT *LOM_GetActI(	/* ＬＯＭアクション */
						/* NULL: 失敗 */
	LOM_ACTLIST *lom_actlist,	/* ＬＯＭアクションリスト */
	int         lom_act_index)	/* ＬＯＭアクションインデックス */
{
	SY_PRINTF3("LOM_GetActI\n");

	if(lom_actlist == NULL)
		return NULL;

	if((lom_act_index < 0) || (lom_actlist->n_lom_act - 1 < lom_act_index))
		return NULL;

	return &lom_actlist->lom_act[lom_act_index];
}

/*******************************************************************************
 * ＬＯＭアクションの取得
 */
LOM_ACT *LOM_GetActI2(	/* ＬＯＭアクション */
						/* NULL: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       lom_actlist_index,	/* ＬＯＭアクションリストインデックス */
	int       lom_act_index)		/* ＬＯＭアクションインデックス */
{
	SY_PRINTF3("LOM_GetActI2\n");

	ASSERT(layoutman != NULL);

	return LOM_GetActI(LOM_GetActlistI(layoutman, lom_actlist_index), lom_act_index);
}

/*******************************************************************************
 * ＬＯＭアイコンリストインデックスの取得
 */
int LOM_GetIcolistIndex(	/* ＬＯＭアイコンリストインデックス */
							/* -1: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       lom_icolist_name)		/* ＬＯＭアイコンリスト名 */
{
	int i;

	SY_PRINTF3("LOM_GetIcolistIndex\n");

	ASSERT(layoutman != NULL);

	/* ＬＯＭアイコンリストの検索 */
	for(i = 0; i < layoutman->n_lom_icolist; i++)
	{
		if(layoutman->lom_icolist[i].icolist_name == lom_icolist_name)
		{
			return i;
		}
	}

	return -1;
}

/*******************************************************************************
 * ＬＯＭアイコンリストの取得
 */
LOM_ICOLIST *LOM_GetIcolist(	/* ＬＯＭアイコンリスト */
								/* NULL: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       lom_icolist_name)		/* ＬＯＭアイコンリスト名 */
{
	int lom_icolist_index;

	SY_PRINTF3("LOM_GetIcolist\n");

	ASSERT(layoutman != NULL);

	if((lom_icolist_index = LOM_GetIcolistIndex(layoutman, lom_icolist_name)) == -1)
	{
		SY_PRINTF2("Cannot Find LOM Icolist[%d].\n", lom_icolist_name);
		return NULL;
	}

	return &layoutman->lom_icolist[lom_icolist_index];
}

/*******************************************************************************
 * ＬＯＭアイコンリストの取得
 */
LOM_ICOLIST *LOM_GetIcolistI(	/* ＬＯＭアイコンリスト */
								/* NULL: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       lom_icolist_index)	/* ＬＯＭアイコンリストインデックス */
{
	SY_PRINTF3("LOM_GetIcolistI\n");

	ASSERT(layoutman != NULL);

	if((lom_icolist_index < 0) || (layoutman->n_lom_icolist - 1 < lom_icolist_index))
		return NULL;

	return &layoutman->lom_icolist[lom_icolist_index];
}

/*******************************************************************************
 * ＬＯＭアイコンインデックスの取得
 */
int LOM_GetIcoIndex(	/* ＬＯＭアイコンインデックス */
						/* -1: 失敗 */
	LOM_ICOLIST *lom_icolist,	/* ＬＯＭアイコンリスト */
	int         lom_ico_name)	/* ＬＯＭアイコン名 */
{
	int i;

	SY_PRINTF3("LOM_GetIcoIndex\n");

	if(lom_icolist == NULL)
		return -1;

	/* ＬＯＭアイコンリストの検索 */
	for(i = 0; i < lom_icolist->n_lom_ico; i++)
	{
		if(lom_icolist->lom_ico[i].ico_name == lom_ico_name)
		{
			return i;
		}
	}

	return -1;
}

/*******************************************************************************
 * ＬＯＭアイコンの取得
 */
LOM_ICO *LOM_GetIco(	/* ＬＯＭアイコン */
						/* NULL: 失敗 */
	LOM_ICOLIST *lom_icolist,	/* ＬＯＭアイコンリスト */
	int         lom_ico_name)	/* ＬＯＭアイコン名 */
{
	int lom_ico_index;

	SY_PRINTF3("LOM_GetIco\n");

	if(lom_icolist == NULL)
		return NULL;

	if((lom_ico_index = LOM_GetIcoIndex(lom_icolist, lom_ico_name)) == -1)
	{
		SY_PRINTF2("Cannot Find LOM Ico[%d].\n", lom_ico_name);
		return NULL;
	}

	return &lom_icolist->lom_ico[lom_ico_index];
}

/*******************************************************************************
 * ＬＯＭアイコンの取得
 */
LOM_ICO *LOM_GetIco2(	/* ＬＯＭアイコン */
						/* NULL: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       lom_icolist_name,		/* ＬＯＭアイコンリスト名 */
	int       lom_ico_name)			/* ＬＯＭアイコン名 */
{
	SY_PRINTF3("LOM_GetIco2\n");

	ASSERT(layoutman != NULL);

	return LOM_GetIco(LOM_GetIcolist(layoutman, lom_icolist_name), lom_ico_name);
}

/*******************************************************************************
 * ＬＯＭアイコンの取得
 */
LOM_ICO *LOM_GetIcoI(	/* ＬＯＭアイコン */
						/* NULL: 失敗 */
	LOM_ICOLIST *lom_icolist,	/* ＬＯＭアイコンリスト */
	int         lom_ico_index)	/* ＬＯＭアイコンインデックス */
{
	SY_PRINTF3("LOM_GetIcoI\n");

	if(lom_icolist == NULL)
		return NULL;

	if((lom_ico_index < 0) || (lom_icolist->n_lom_ico - 1 < lom_ico_index))
		return NULL;

	return &lom_icolist->lom_ico[lom_ico_index];
}

/*******************************************************************************
 * ＬＯＭアイコンの取得
 */
LOM_ICO *LOM_GetIcoI2(	/* ＬＯＭアイコン */
						/* NULL: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       lom_icolist_index,	/* ＬＯＭアイコンリストインデックス */
	int       lom_ico_index)		/* ＬＯＭアイコンインデックス */
{
	SY_PRINTF3("LOM_GetIcoI2\n");

	ASSERT(layoutman != NULL);

	return LOM_GetIcoI(LOM_GetIcolistI(layoutman, lom_icolist_index), lom_ico_index);
}

/*******************************************************************************
 * ＬＯＭオブジェクトリストインデックスの取得
 */
int LOM_GetObjlistIndex(	/* ＬＯＭオブジェクトリストインデックス */
							/* -1: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       lom_objlist_name)		/* ＬＯＭオブジェクトリスト名 */
{
	int i;

	SY_PRINTF3("LOM_GetObjlistIndex\n");

	ASSERT(layoutman != NULL);

	/* ＬＯＭオブジェクトリストの検索 */
	for(i = 0; i < layoutman->n_lom_objlist; i++)
	{
		if(layoutman->lom_objlist[i].objlist_name == lom_objlist_name)
		{
			return i;
		}
	}

	return -1;
}

/*******************************************************************************
 * ＬＯＭオブジェクトリストの取得
 */
LOM_OBJLIST *LOM_GetObjlist(	/* ＬＯＭオブジェクトリスト */
								/* NULL: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       lom_objlist_name)		/* ＬＯＭオブジェクトリスト名 */
{
	int lom_objlist_index;

	SY_PRINTF3("LOM_GetObjlist\n");

	ASSERT(layoutman != NULL);

	if((lom_objlist_index = LOM_GetObjlistIndex(layoutman, lom_objlist_name)) == -1)
	{
		SY_PRINTF2("Cannot Find LOM Objlist[%d].\n", lom_objlist_name);
		return NULL;
	}

	return &layoutman->lom_objlist[lom_objlist_index];
}

/*******************************************************************************
 * ＬＯＭオブジェクトリストの取得
 */
LOM_OBJLIST *LOM_GetObjlistI(	/* ＬＯＭオブジェクトリスト */
								/* NULL: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       lom_objlist_index)	/* ＬＯＭオブジェクトリストインデックス */
{
	SY_PRINTF3("LOM_GetObjlistI\n");

	ASSERT(layoutman != NULL);

	if((lom_objlist_index < 0) || (layoutman->n_lom_objlist - 1 < lom_objlist_index))
		return NULL;

	return &layoutman->lom_objlist[lom_objlist_index];
}

/*******************************************************************************
 * ＬＯＭオブジェクトインデックスの取得
 */
int LOM_GetObjIndex(	/* ＬＯＭオブジェクトインデックス */
						/* -1: 失敗 */
	LOM_OBJLIST *lom_objlist,	/* ＬＯＭオブジェクトリスト */
	int         lom_obj_name)	/* ＬＯＭオブジェクト名 */
{
	int i;

	SY_PRINTF3("LOM_GetObjIndex\n");

	if(lom_objlist == NULL)
		return -1;

	/* ＬＯＭオブジェクトリストの検索 */
	for(i = 0; i < lom_objlist->n_lom_obj; i++)
	{
		if(lom_objlist->lom_obj[i].obj_name == lom_obj_name)
		{
			return i;
		}
	}

	return -1;
}

/*******************************************************************************
 * ＬＯＭオブジェクトの取得
 */
LOM_OBJ *LOM_GetObj(	/* ＬＯＭオブジェクト */
						/* NULL: 失敗 */
	LOM_OBJLIST *lom_objlist,	/* ＬＯＭオブジェクトリスト */
	int         lom_obj_name)	/* ＬＯＭオブジェクト名 */
{
	int lom_obj_index;

	SY_PRINTF3("LOM_GetObj\n");

	if(lom_objlist == NULL)
		return NULL;

	if((lom_obj_index = LOM_GetObjIndex(lom_objlist, lom_obj_name)) == -1)
	{
		SY_PRINTF2("Cannot Find LOM Obj[%d].\n", lom_obj_name);
		return NULL;
	}

	return &lom_objlist->lom_obj[lom_obj_index];
}

/*******************************************************************************
 * ＬＯＭオブジェクトの取得
 */
LOM_OBJ *LOM_GetObj2(	/* ＬＯＭオブジェクト */
						/* NULL: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       lom_objlist_name,		/* ＬＯＭオブジェクトリスト名 */
	int       lom_obj_name)			/* ＬＯＭオブジェクト名 */
{
	SY_PRINTF3("LOM_GetObj2\n");

	ASSERT(layoutman != NULL);

	return LOM_GetObj(LOM_GetObjlist(layoutman, lom_objlist_name), lom_obj_name);
}

/*******************************************************************************
 * ＬＯＭオブジェクトの取得
 */
LOM_OBJ *LOM_GetObjI(	/* ＬＯＭオブジェクト */
						/* NULL: 失敗 */
	LOM_OBJLIST *lom_objlist,	/* ＬＯＭオブジェクトリスト */
	int         lom_obj_index)	/* ＬＯＭオブジェクトインデックス */
{
	SY_PRINTF3("LOM_GetObjI\n");

	if(lom_objlist == NULL)
		return NULL;

	if((lom_obj_index < 0) || (lom_objlist->n_lom_obj - 1 < lom_obj_index))
		return NULL;

	return &lom_objlist->lom_obj[lom_obj_index];
}

/*******************************************************************************
 * ＬＯＭオブジェクトの取得
 */
LOM_OBJ *LOM_GetObjI2(	/* ＬＯＭオブジェクト */
						/* NULL: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       lom_objlist_index,	/* ＬＯＭオブジェクトリストインデックス */
	int       lom_obj_index)		/* ＬＯＭオブジェクトインデックス */
{
	SY_PRINTF3("LOM_GetObjI2\n");

	ASSERT(layoutman != NULL);

	return LOM_GetObjI(LOM_GetObjlistI(layoutman, lom_objlist_index), lom_obj_index);
}

/*******************************************************************************
 * ＬＯＭパッドリストインデックスの取得
 */
int LOM_GetPadlistIndex(	/* ＬＯＭパッドリストインデックス */
							/* -1: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       lom_padlist_name)		/* ＬＯＭパッドリスト名 */
{
	int i;

	SY_PRINTF3("LOM_GetPadlistIndex\n");

	ASSERT(layoutman != NULL);

	/* ＬＯＭパッドリストの検索 */
	for(i = 0; i < layoutman->n_lom_padlist; i++)
	{
		if(layoutman->lom_padlist[i].padlist_name == lom_padlist_name)
		{
			return i;
		}
	}

	return -1;
}

/*******************************************************************************
 * ＬＯＭパッドリストの取得
 */
LOM_PADLIST *LOM_GetPadlist(	/* ＬＯＭパッドリスト */
								/* NULL: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       lom_padlist_name)		/* ＬＯＭパッドリスト名 */
{
	int lom_padlist_index;

	SY_PRINTF3("LOM_GetPadlist\n");

	ASSERT(layoutman != NULL);

	if((lom_padlist_index = LOM_GetPadlistIndex(layoutman, lom_padlist_name)) == -1)
	{
		SY_PRINTF2("Cannot Find LOM Padlist[%d].\n", lom_padlist_name);
		return NULL;
	}

	return &layoutman->lom_padlist[lom_padlist_index];
}

/*******************************************************************************
 * ＬＯＭパッドリストの取得
 */
LOM_PADLIST *LOM_GetPadlistI(	/* ＬＯＭパッドリスト */
								/* NULL: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       lom_padlist_index)	/* ＬＯＭパッドリストインデックス */
{
	SY_PRINTF3("LOM_GetPadlistI\n");

	ASSERT(layoutman != NULL);

	if((lom_padlist_index < 0) || (layoutman->n_lom_padlist - 1 < lom_padlist_index))
		return NULL;

	return &layoutman->lom_padlist[lom_padlist_index];
}

/*******************************************************************************
 * ＬＯＭパッドの取得
 */
LOM_PAD *LOM_GetPadI(	/* ＬＯＭパッド */
						/* NULL: 失敗 */
	LOM_PADLIST *lom_padlist,	/* ＬＯＭパッドリスト */
	int         lom_pad_index)	/* ＬＯＭパッドインデックス */
{
	SY_PRINTF3("LOM_GetPadI\n");

	if(lom_padlist == NULL)
		return NULL;

	if((lom_pad_index < 0) || (lom_padlist->n_lom_pad - 1 < lom_pad_index))
		return NULL;

	return &lom_padlist->lom_pad[lom_pad_index];
}

/*******************************************************************************
 * ＬＯＭパッドの取得
 */
LOM_PAD *LOM_GetPadI2(	/* ＬＯＭパッド */
						/* NULL: 失敗 */
	LAYOUTMAN *layoutman,			/* レイアウトマネージャ */
	int       lom_padlist_index,	/* ＬＯＭパッドリストインデックス */
	int       lom_pad_index)		/* ＬＯＭパッドインデックス */
{
	SY_PRINTF3("LOM_GetPadI2\n");

	ASSERT(layoutman != NULL);

	return LOM_GetPadI(LOM_GetPadlistI(layoutman, lom_padlist_index), lom_pad_index);
}

/*******************************************************************************
 * ＬＯＭモードインデックスの取得
 */
int LOM_GetModeIndex(	/* ＬＯＭモードインデックス */
						/* -1: 失敗 */
	LAYOUTMAN *layoutman,		/* レイアウトマネージャ */
	int       lom_mode_name)	/* ＬＯＭモード名 */
{
	int i;

	SY_PRINTF3("LOM_GetModeIndex\n");

	ASSERT(layoutman != NULL);

	/* ＬＯＭモードの検索 */
	for(i = 0; i < layoutman->n_lom_mode; i++)
	{
		if(layoutman->lom_mode[i].mode_name == lom_mode_name)
		{
			return i;
		}
	}

	return -1;
}

/*******************************************************************************
 * ＬＯＭモードの取得
 */
LOM_MODE *LOM_GetMode(	/* ＬＯＭモード */
						/* NULL: 失敗 */
	LAYOUTMAN *layoutman,		/* レイアウトマネージャ */
	int       lom_mode_name)	/* ＬＯＭモード名 */
{
	int lom_mode_index;

	SY_PRINTF3("LOM_GetMode\n");

	ASSERT(layoutman != NULL);

	if((lom_mode_index = LOM_GetModeIndex(layoutman, lom_mode_name)) == -1)
	{
		SY_PRINTF2("Cannot Find LOM Mode[%d].\n", lom_mode_name);
		return NULL;
	}

	return &layoutman->lom_mode[lom_mode_index];
}

/*******************************************************************************
 * ＬＯＭモードの取得
 */
LOM_MODE *LOM_GetModeI(		/* ＬＯＭモード */
							/* NULL: 失敗 */
	LAYOUTMAN *layoutman,		/* レイアウトマネージャ */
	int       lom_mode_index)	/* ＬＯＭモードインデックス */
{
	SY_PRINTF3("LOM_GetModeI\n");

	ASSERT(layoutman != NULL);

	if((lom_mode_index < 0) || (layoutman->n_lom_mode - 1 < lom_mode_index))
		return NULL;

	return &layoutman->lom_mode[lom_mode_index];
}
