//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * outline - objoutline.c
 * オブジェクトアウトライン  *NewObjectOutline
 * 2002/03/11 S.Yamashita
 * $Id: objoutline.c,v 1.1.1.3 2002/11/19 11:51:51 Yoshizawa1 Exp $
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

#define __CHARA_NAME__ "Object Outline"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
#include "../sy_util/sy_util.h"

#include "outline.h"

/*******************************************************************************
 * work
 */

/* ワーク */
typedef struct tagWORK
{
	GV_ACT_EX actor;	/* アクター */
	int       cache;	/* キャッシュＩＤ */
	OUTLINE   outline;	/* アウトライン */
}
WORK;

/*******************************************************************************
 * actor
 */
/*******************************************************************************
 * シグナル処理
	OL_SIG_SetColorA = 0,	 CVECTOR[4] へのポインタ
	OL_SIG_SetColorB,		 CVECTOR[4] へのポインタ
	OL_SIG_SetColorAB,		 OL_COLOR へのポインタ
	OL_SIG_SetColor,		 OL_COLOR へのポインタ
	OL_SIG_SetRange,		 OL_RANGE へのポインタ
 */
static int ReceiveSignal(	/* 処理結果 */
	void *pwork,	/* ワーク */
	int  signal,	/* シグナル */
	int  value)		/* 付加情報 */
{
	int  ret;
	WORK *work  = (WORK *)pwork;

	SY_PRINTF3("ReceiveSignal\n");
	SY_PRINTF1("Signal: %d\n", signal);

	switch(signal)
	{
	case OL_SIG_SetColorA:
		OutlineSetColorA(&work->outline, (CVECTOR *)value);
		ret = OL_SIG_SetColorA;
		break;

	case OL_SIG_SetColorB:
		OutlineSetColorB(&work->outline, (CVECTOR *)value);
		ret = OL_SIG_SetColorB;
		break;

	case OL_SIG_SetColorAB:
		OutlineSetColorAB(&work->outline,
			((OL_COLOR *)value)->col_num, ((OL_COLOR *)value)->col);
		ret = OL_SIG_SetColorAB;
		break;

	case OL_SIG_SetColor:

printf("SetColor %d, %d, %d, %d\n", ((OL_COLOR *)value)->col[0].r, ((OL_COLOR *)value)->col[0].g, ((OL_COLOR *)value)->col[0].b, ((OL_COLOR *)value)->col[0].cd);

		OutlineSetColor(&work->outline,
			((OL_COLOR *)value)->col_num, ((OL_COLOR *)value)->col);
		ret = OL_SIG_SetColor;
		break;

	case OL_SIG_SetRange:
		if(((OL_RANGE *)value)->mode < 3)
			OutlineSetXYZRange(&work->outline,
				((OL_RANGE *)value)->mode, ((OL_RANGE *)value)->range);
		else
			OutlineSetPRange(&work->outline,
				((OL_RANGE *)value)->mode, ((OL_RANGE *)value)->range);
		ret = OL_SIG_SetRange;
		break;

	default:
		return GV_DefaultSignalFunc(work, signal, value);
	}

	return ret;
}

/*******************************************************************************
 * 毎フレーム処理
 */
static void Act(
	WORK *work)		/* ワーク */
{
	ActOutline(&work->outline);
}

/*******************************************************************************
 * 終了処理
 */
static void Die(
	WORK *work)		/* ワーク */
{
	SY_PRINTF3("Die\n");

	DestroyOutline(&work->outline);
}

/*******************************************************************************
 * 初期化処理
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	WORK    *work,		/* ワーク */
	int     cache,		/* キャッシュＩＤ */
	DG_OBJS *dg_objs,	/* ＤＧオブジェクト */
	int     flag)		/* フラグ */
{
	work->cache = cache;

	/* アウトラインの初期化 */
	{
      int was_swapped = 0;
      void *mem_le = GV_GetCacheMarkSwapped( work->cache, &was_swapped );

		if(CreateOutline(&work->outline,
			mem_le, dg_objs, NULL, NULL, flag, was_swapped) == 0)
		{
			SY_PRINTF2("Create Outline Failed.\n");
			return 0;
		}

		SY_PRINTF1("Create Outline Succeeded.\n");
	}

	return 1;
}

/*******************************************************************************
 * 起動処理
 */
void *NewObjectOutline(
	int     cache,		/* キャッシュＩＤ */
	DG_OBJS *dg_objs,	/* ＤＧオブジェクト */
	int     flag)		/* フラグ */
{
	WORK *work;

	SY_PRINTF3("NewObjectOutline\n");

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

		/* シグナル処理関数登録 */
		GV_SetActorSignalFunc(work, ReceiveSignal);

		/* ワークの初期化 */
		if(!GetResources(work, cache, dg_objs, flag))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}
