//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * outline - stgoutline.c
 * ステージアウトライン  *NewStageOutline
 * 2002/02/28 S.Yamashita
 * $Id: stgoutline.c,v 1.1.1.3 2002/11/19 11:51:52 Yoshizawa1 Exp $
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

#define __CHARA_NAME__ "Stage Outline"
#define SY_PRINTF1_DEBUG
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
	int       name;		/* シナリオ名 */
	int       where;	/* マップＩＤ */
	int       cache;	/* キャッシュＩＤ */
	OUTLINE   outline;	/* アウトライン */
}
WORK;

/*******************************************************************************
 * actor
 */
/*******************************************************************************
 * メッセージ処理

mesg ステージアウトライン $s:名前 非表示[0]
mesg ステージアウトライン $s:名前 表示[1]
mesg ステージアウトライン $s:名前 色の設定[2] $b:色番号 $b:Ｒ $b:Ｇ $b:Ｂ $b:Ａ
 */
static int ReceiveMessage(	/* 処理結果 */
	WORK *work)		/* ワーク */
{
	int     i;
	int     msg_count;
	int     ret = -1;
	GV_MSG  *msg;
	CVECTOR col;

	msg_count = GV_ReceiveMessage(work->name, &msg);
	for(i = 0; i < msg_count; i++, msg++)
	{
		SY_PRINTF2("Message received.\n");

		switch(msg->message[0])
		{
		case 0:		// 非表示
			OutlineShow(&work->outline, 0);
			ret = 0;
			break;

		case 1:		// 表示
			OutlineShow(&work->outline, 1);
			ret = 1;
			break;

		case 2:		// 色の設定
			col.r  = msg->message[2];
			col.g  = msg->message[3];
			col.b  = msg->message[4];
			col.cd = msg->message[5];
			OutlineSetColor(&work->outline, msg->message[1], &col);
			ret = 2;
			break;

		default:
			ret = -1;
			break;
		}
	}

	return ret;
}

/*******************************************************************************
 * 毎フレーム処理
 */
static void Act(
	WORK *work)		/* ワーク */
{
//	static float f[2] = {0.0f, 0.0f};
//	static float x[2] = {-10000.0f, -13000.0f};
//	static float z[2] = {-15000.0f, -18000.0f};

//	f[1] = (f[1] += 0.002f) < 1.002f ? f[1] : 0.0f;
//	OutlineSetPRange(&work->outline, 4, f);

//	x[0] = (x[0] += 250.f) < 10250.0f ? x[0] : -10000.0f;
//	x[1] = (x[1] += 250.f) < 10250.0f ? x[1] : -10000.0f;
//	OutlineSetXYZRange(&work->outline, 0, x);

//	z[0] = (z[0] += 500.f) < 15500.0f ? z[0] : -15000.0f;
//	z[1] = (z[1] += 500.f) < 15500.0f ? z[1] : -15000.0f;
//	OutlineSetXYZRange(&work->outline, 2, z);

	ReceiveMessage(work);
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

chara ステージアウトライン[NewStageOutline_Scn] $s:名前 \
	-file    $s:rolファイル名 \
	// 以下オプション
	-a_color $b:Ｒ $b:Ｇ $b:Ｂ $b:Ａ ...4 \
	-g       $i:フラグ \
	-center  $i:Ｘ $i:Ｙ $i:Ｚ

// -g フラグ用
#define OL_ANTIALIASING  0x00010000	// アンチエイリアス
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	WORK *work,		/* ワーク */
	int  name,		/* シナリオ名 */
	int  where)		/* マップＩＤ */
{
	int     i;
	int     flag;
	CVECTOR col[4];
	FVECTOR offset = {0.0, 0.0, 0.0, 1.0};

	SY_PRINTF3("GetResources\n");

	work->name  = name;
	work->where = where;
	flag        = 0;
	memset(col, 0xff, sizeof(col));

	/* シナリオオプション */
	{
		/* a_color */
		if(GCL_GetOption('a') != NULL)
		{
			for(i = 0; i < 4; i++)
			{
				if(GCL_NextStr() != NULL)
				{
					col[i].r  = GCL_GetNextInt();
					col[i].g  = GCL_GetNextInt();
					col[i].b  = GCL_GetNextInt();
					col[i].cd = GCL_GetNextInt();
				}
			}
		}

		/* a_color */
		if(GCL_GetOption('a') != NULL)
		{
			for(i = 0; i < 4; i++)
			{
				if(GCL_NextStr() != NULL)
				{
					col[i].r  = GCL_GetNextInt();
					col[i].g  = GCL_GetNextInt();
					col[i].b  = GCL_GetNextInt();
					col[i].cd = GCL_GetNextInt();
				}
			}
		}

		/* center */
		if(GCL_GetOption('c') != NULL)
		{
			offset.vx = GCL_GetNextInt();
			offset.vy = GCL_GetNextInt();
			offset.vz = GCL_GetNextInt();
		}

		/* g */
		if(GCL_GetOption('g') != NULL)
			flag = GCL_GetNextInt();

		SY_PRINTF1("Get Scn Option Succeeded.\n");
	}

	/* file */
	if(GCL_GetOption('f') == NULL)
	{
		SY_PRINTF2("No rol file info.\n");
		return 0;
	}
	work->cache = GV_CacheID(GCL_GetNextInt(), 'r');

	/* アウトラインの初期化 */
	{
      int was_swapped = 0;
      void *mem_le = GV_GetCacheMarkSwapped( work->cache, &was_swapped );

      if(CreateOutline(&work->outline,
			mem_le, NULL, col, &offset, flag, was_swapped ) == 0)
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
void *NewStageOutline_Scn(
	int name,	/* シナリオ名 */
	int where)	/* マップＩＤ */
{
	WORK *work;

	SY_PRINTF3("NewStageOutline_Scn\n");

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
