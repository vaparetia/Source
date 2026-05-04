//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * vr_spark.c
 * ＶＲ跳弾  *NewVRSpark
 * 2002/04/09 S.Yamashita
 * $Id: vr_spark.c,v 1.4 2002/11/23 12:36:06 Yoshizawa1 Exp $
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

#define __CHARA_NAME__ "VR Spark"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
#include "../sy_util/sy_util.h"

/*******************************************************************************
 * defines
 */

#define COMDL_COUNT (10)	/* 跳弾コモデル数 */
#define SPARK_COUNT (12)	/* 最大跳弾数 */
#define ACT_COUNT   (16)	/* 表示時間 */
#define COLOR_R     (80)	/* 色Ｒ */
#define COLOR_G     (30)	/* 色Ｇ */
#define COLOR_B     (10)	/* 色Ｂ */
#define COLOR_A     (90)	/* 色Ａ */
#define SPREAD      (1560)	/* 広がり角度 */
#define SCALE_XY    (2.0f)	/* ＸＹ方向モーションのスケール */
#define SCALE_Z     (2.4f)	/* Ｚ方向モーションのスケール */

/*******************************************************************************
 * work
 */

typedef struct tagSPARK SPARK;
typedef struct tagWORK
{
	GV_ACT_EX actor;				/* アクター */
	int       name;					/* 名前 */
	int       where;				/* マップＩＤ */
	int       flag;					/* フラグ */
	short     n_spark;				/* 最大跳弾数 */
	DG_COMDL  *comdl[COMDL_COUNT];	/* 跳弾コモデル */

	struct tagSPARK
	{
		FVECTOR pos;				/* 位置 */
		FMATRIX world;				/* 回転 */
		SVECTOR rot[COMDL_COUNT];	/* 広がり */
		int     count;				/* 表示時間(カウンター) */
		int     count_max;			/* 表示時間 */
	}
	*spark;							/* 跳弾配列 */
}
WORK;

/*******************************************************************************
 * local
 */

static WORK *wk = NULL;

/*******************************************************************************
 * static
 */
/******************************************************************************
 * 空いている要素を返す
 */
static int GetSpark(	/* インデックス番号 */
						/* -1: 空いている要素がない */
	WORK *work)		/* ワーク */
{
	int i;

	for(i = 0; i < work->n_spark; i++)
	{
		if(work->spark[i].count == 0)
			return i;
	}

	return -1;
}

/*******************************************************************************
 * actor
 */
/******************************************************************************
 * 新規跳弾の登録 - NewVRSpark
 */
int NewVRSpark(		/* 1: 成功 */
					/* 0: 失敗 */
	FMATRIX *world)		/* ワールド */
{
	int   index1, index2;
	SPARK *spark;

	if(wk == NULL)
		return 0;

	if((index2 = GetSpark(wk)) == -1)
		return 0;

	/* マトリクスをダンプ */
	{
		SY_PRINTF1("m[0][0] = %f\n"  , world->m[0][0]);
		SY_PRINTF1("m[0][1] = %f\n"  , world->m[0][1]);
		SY_PRINTF1("m[0][2] = %f\n"  , world->m[0][2]);
		SY_PRINTF1("m[0][3] = %f\n\n", world->m[0][3]);
		SY_PRINTF1("m[1][0] = %f\n"  , world->m[1][0]);
		SY_PRINTF1("m[1][1] = %f\n"  , world->m[1][1]);
		SY_PRINTF1("m[1][2] = %f\n"  , world->m[1][2]);
		SY_PRINTF1("m[1][3] = %f\n\n", world->m[1][3]);
		SY_PRINTF1("m[2][0] = %f\n"  , world->m[2][0]);
		SY_PRINTF1("m[2][1] = %f\n"  , world->m[2][1]);
		SY_PRINTF1("m[2][2] = %f\n"  , world->m[2][2]);
		SY_PRINTF1("m[2][3] = %f\n\n", world->m[2][3]);
		SY_PRINTF1("m[3][0] = %f\n"  , world->m[3][0]);
		SY_PRINTF1("m[3][1] = %f\n"  , world->m[3][1]);
		SY_PRINTF1("m[3][2] = %f\n"  , world->m[3][2]);
		SY_PRINTF1("m[3][3] = %f\n\n", world->m[3][3]);
	}

	/* 跳弾を初期化 */
	spark = &wk->spark[index2];
	spark->pos           = *(FVECTOR *)world->m[3];
	spark->world         = *world;
	spark->world.m[3][0] = 0;
	spark->world.m[3][1] = 0;
	spark->world.m[3][2] = 0;
	spark->count         = ACT_COUNT;
	spark->count_max     = ACT_COUNT;
	for(index1 = 0; index1 < COMDL_COUNT; index1++)
	{
		spark->rot[index1].vx  = (irnd() % SPREAD) - SPREAD / 2 + 768;
		spark->rot[index1].vy  = (irnd() % SPREAD) - SPREAD / 2;
		spark->rot[index1].vz  = irnd();
		spark->rot[index1].pad = 1;
	}

	return 1;
}

/******************************************************************************
 * 毎フレーム処理
 */
static void Act(
	WORK *work)		/* ワーク */
{
	int      index1, index2;
	SPARK    *spark;
	FVECTOR  scale;
	FMATRIX  mat;
	int      a, phaseA1, phaseA2;
	int      b, phaseB1, phaseB2;

	/* 各跳弾 */
	for(index2 = 0; index2 < work->n_spark; index2++)
	{
		spark = &work->spark[index2];
		if(spark->count == 0)
			continue;

		/* フェーズ値 */
		phaseA2 = spark->count_max * 4 / 5;
		phaseA1 = spark->count_max - phaseA2;
		a       = spark->count - phaseA2;
		phaseB2 = spark->count_max * 2 / 3;
		phaseB1 = spark->count_max - phaseB2;
		b      = spark->count - phaseB2;

		/* フェーズＡ */
		if(a > 0) scale.vy = scale.vx = a             * SCALE_XY / phaseA1;		/* フェーズ１ */
		else      scale.vy = scale.vx = (phaseA2 + a) * SCALE_XY / phaseA2;		/* フェーズ２ */
		scale.vz = (spark->count_max - spark->count) * SCALE_Z / spark->count_max;
		scale.vw = 1.0f;

		spark->count--;

		/* 各コモデル */
		for(index1 = 0; index1 < COMDL_COUNT; index1++)
		{
			DG_SetPos(&DG_UnitMatrix);
			DG_ScalePos(&scale);
			DG_GetPos(&mat);

			DG_SetPos(&spark->world);
			DG_RotatePos(&spark->rot[index1]);
			DG_GetPos(&wk->comdl[index1]->pos[index2].world);
			_sceVu0MulMatrix(&wk->comdl[index1]->pos[index2].world, &wk->comdl[index1]->pos[index2].world, &mat);

			wk->comdl[index1]->pos[index2].world.m[3][0] = spark->pos.vx;
			wk->comdl[index1]->pos[index2].world.m[3][1] = spark->pos.vy;
			wk->comdl[index1]->pos[index2].world.m[3][2] = spark->pos.vz;

			if(spark->count == 0) work->comdl[index1]->pos[index2].color.vw = 0;
			else
			{
				/* フェーズＢ */
				if(b > 0) work->comdl[index1]->pos[index2].color.vw = COLOR_A;								/* フェーズ１ */
				else      work->comdl[index1]->pos[index2].color.vw = (phaseB2 + b) * COLOR_A / phaseB2;	/* フェーズ２ */
			}
		}
	}
}

/******************************************************************************
 * 終了処理
 */
static void Die(
	WORK *work)		/* ワーク */
{
	int i;

	for(i = 0; i < COMDL_COUNT; i++)
	{
		if(work->comdl[i])
		{
			DG_DequeueComdlObjs(work->comdl[i]);
			DG_FreeComdl(work->comdl[i]);
		}
	}

	GV_Free(work->spark);
}

/******************************************************************************
 * 初期化処理

chara ＶＲ跳弾[NewVRSparkBase] $s:名前
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	WORK *work,		/* ワーク */
	int  name,		/* シナリオ名 */
	int  where)		/* マップＩＤ */
{
	int    i, j;
	DG_DEF *def;

	work->name    = name;
	work->where   = where;
	work->flag    = 0;
	work->n_spark = SPARK_COUNT;

	if((work->spark = GV_Malloc(sizeof(SPARK) * work->n_spark)) == NULL)
		ASSERT(0);
	memset(work->spark, 0x00, sizeof(SPARK) * work->n_spark);

	for(i = 0; i < COMDL_COUNT; i++)
	{
		/* 各コモデル kms */
		switch(i)
		{
		case 0 : def = (DG_DEF*)GV_GetCache(GV_CacheID(16719133, 'k')); break;	/* vr2_frg1_cm */
		case 1 : def = (DG_DEF*)GV_GetCache(GV_CacheID(16751901, 'k')); break;	/* vr2_frg2_cm */
		case 2 : def = (DG_DEF*)GV_GetCache(GV_CacheID(    7454, 'k')); break;	/* vr2_frg1_cm */
		case 3 : def = (DG_DEF*)GV_GetCache(GV_CacheID(   40222, 'k')); break;	/* vr2_frg4_cm */
		case 4 : def = (DG_DEF*)GV_GetCache(GV_CacheID(   72990, 'k')); break;	/* vr2_frg5_cm */
		case 5 : def = (DG_DEF*)GV_GetCache(GV_CacheID(16719133, 'k')); break;	/* vr2_frg1_cm */
		case 6 : def = (DG_DEF*)GV_GetCache(GV_CacheID(16751901, 'k')); break;	/* vr2_frg2_cm */
		case 7 : def = (DG_DEF*)GV_GetCache(GV_CacheID(    7454, 'k')); break;	/* vr2_frg1_cm */
		case 8 : def = (DG_DEF*)GV_GetCache(GV_CacheID(   40222, 'k')); break;	/* vr2_frg4_cm */
		case 9 : def = (DG_DEF*)GV_GetCache(GV_CacheID(   72990, 'k')); break;	/* vr2_frg5_cm */
		default: continue;
		}

		/* コモデル生成 */
		/* 各コモデルを跳弾数分生成する */
		if((work->comdl[i] = DG_MakeComdl(def->models[0].packs, DG_COMDL_SEMITRANS, work->n_spark, 0)) == NULL)
			ASSERT(0);
		DG_QueueComdlObjs(work->comdl[i]);
		work->comdl[i]->group_id = GM_GetDGGroupID(where);

		/* 各跳弾を初期化 */
		for(j = 0; j < work->n_spark; j++)
		{
			DG_COPY_MAT(&work->comdl[i]->pos[j].world, &DG_UnitMatrix);
			work->comdl[i]->pos[j].color.vx = COLOR_R;
			work->comdl[i]->pos[j].color.vy = COLOR_G;
			work->comdl[i]->pos[j].color.vz = COLOR_B;
			work->comdl[i]->pos[j].color.vw = 0;
		}
	}

	/* ワークを static に保管 */
	wk = work;

	return 1;
}

/******************************************************************************
 * 起動処理
 */
void *NewVRSparkBase(
	int name,	/* シナリオ名 */
	int where)	/* マップＩＤ */
{
	WORK *work;

	SY_PRINTF3("NewVRSparkBase\n");
	OPERATOR();

	/* アクター生成 */
	work = (WORK *)GV_NewActor(
		GV_ACTOR_USER,
		sizeof(WORK));
	if(work != NULL)
	{
		/* アクター登録 */
		GV_SetActor(&work->actor, Act, Die);

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
