/*******************************************************************************
 * CHARA.c
 * キャラ  *NewCHARA
 * 2002/XX/XX S.Yamashita
 * $Id: CHARA.c,v 1.1.1.3 2002/11/19 11:51:54 Yoshizawa1 Exp $
 */

/*******************************************************************************
 * include
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include "libdg.h"
#include "libdg.cnf"
#include "libmt.h"
#include "gameheader.h"
#include "def_dma.h"
#include "libutl.h"

#define __CHARA_NAME__ "CHARA"
#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
#include "../sy_util/sy_util.h"

/*******************************************************************************
 * defines
 */

#define MEM_ADDR1 ((void*)(SCRPAD_ADDR + 0x0000))
#define MEM_ADDR2 ((void*)(SCRPAD_ADDR + 0x2000))

/*******************************************************************************
 * work
 */

/* ワーク */
typedef struct tagWORK
{
	GV_ACT_EX actor;	/* アクター */
	int       name;		/* シナリオ名 */
	int       where;	/* マップＩＤ */
}
WORK;

/*******************************************************************************
 * actor
 */
/*******************************************************************************
 * シグナル処理
 */
static int ReceiveSignal(	/* 処理結果 */
	void *pwork,	/* ワーク */
	int  signal,	/* シグナル */
	int  value)		/* 付加情報 */
{
	int  ret;
	WORK *work  = (WORK *)pwork;

	SY_PRINTF3("ReceiveSignal\n");
	SY_PRINTF2("Signal: %d\n", signal);

	switch(signal)
	{
	case 0:
		ret = 0;
		break;

	case 1:
		ret = 1;
		break;

	default:
		return GV_DefaultSignalFunc(work, signal, value);
	}

	return ret;
}

/*******************************************************************************
 * メッセージ処理
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
		case 0:
			ret = 0;
			break;

		case 1:
			ret = 1;
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
	ReceiveMessage(work);
}

/******************************************************************************
 * 終了処理
 */
static void Die(
	WORK *work)		/* ワーク */
{
	SY_PRINTF3("Die\n");
}

/******************************************************************************
 * 初期化処理

chara キャラ[NewCHARA_Scn] $s:名前
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	WORK *work,		/* ワーク */
	int  name,		/* シナリオ名 */
	int  where)		/* マップＩＤ */
{
	SY_PRINTF3("GetResources\n");

	work->name  = name;
	work->where = where;

	return 1;
}

/******************************************************************************
 * 起動処理
 */
void *NewCHARA(void)
{
	WORK *work;

	SY_PRINTF3("NewCHARA\n");
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

		/* シグナル処理関数登録 */
		GV_SetActorSignalFunc(work, ReceiveSignal);

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

void *NewCHARA_Scn(
	int name,	/* シナリオ名 */
	int where)	/* マップＩＤ */
{
	WORK *work;

	SY_PRINTF3("NewCHARA_Scn\n");
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

		/* シグナル処理関数登録 */
		GV_SetActorSignalFunc(work, ReceiveSignal);

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
