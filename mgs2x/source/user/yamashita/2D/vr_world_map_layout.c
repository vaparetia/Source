//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/******************************************************************************
 * vr_pause - vr_world_map_layout.c
 * ＶＲ全体マップ表示３Ｄのにぎやかし  *NewVRWorldMapLayout
 * 2002/07/04 S.Yamashita
 * $Id: vr_world_map_layout.c,v 1.1.1.3 2002/11/19 11:51:50 Yoshizawa1 Exp $
 */

/******************************************************************************
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

#include "debugmenu.h"

#include "../../sigeno/vr/vr.h"
#include "msn.h"
#include "layoutman.h"
#include "vr_def.h"

#include "vr_map_3d.h"
#include "vr_world_map_layout.h"

#define __CHARA_NAME__ "VR Map 3D L2D"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
//#define SY_PRINTF3_DEBUG
#include "../sy_util/sy_util.h"

/******************************************************************************
 * define
 */

/* フラグ */
#define FLAG_ACTION_START (0x1)		/* アクションを開始 */
#define FLAG_ACTION_OK    (0x2)		/* アクション中 */
#define FLAG_SPRITE_INIT  (0x4)		/* スプライトを初期化 */

/* 設定値 */
#define BLOCK_MAX      (3)	/* ブロックスプライト数 */
#define FADE_IN_SPEED  (4)	/* フェードイン */
#define FADE_OUT_SPEED (8)	/* フェードアウト */

/* レイアウト */
#define LAYOUT            (  114832)	/* map */
#define OBJ_ROOT          ( 2770484)	/* ROOT */
#define OBJ_BOMBTEX       (  194173)	/* bombtex */
#define OBJ_BLOCK1        (16325211)	/* block1 */
#define OBJ_CURSOR_TRI    ( 2755675)	/* cursor_tri */

#define ACTION_DEFAULT    (  566267)	/* DefaultAction */
#define ACTION_DISPSTART  (11728985)	/* DispStart */
#define ACTION_DISPLAY    ( 6934130)	/* Display */
#define ACTION_DISPEND    ( 6894589)	/* DispEnd */
#define ACTION_EMPTY      ( 8933119)	/* empty */

/******************************************************************************
 * work
 */

typedef struct _work
{
	GV_ACT_EX actor;					/* アクター */
	Pos       *pos;						/* 位置情報のアドレス */
	int       *position;				/* 現在位置のアドレス */
	int       *bomb_stage;				/* 爆弾ステージ */
	int       handle_2d;				/* レイアウト */

	SPR_OBJ   *block[BLOCK_MAX];		/* ブロックスプライト */
	SPR_OBJ   *bomb[BOMB_MAX];			/* Ｂマークスプライト */
	int       dup_count;				/* 複製数 */
	int       invers;					/* フェードフラグ */

	int       action;					/* 起動アクション */
	int       flag;						/* フラグ */
	void      (*act)(struct _work *);	/* Act 関数 */
} Work;

/******************************************************************************
 * extern
 */

extern void *NewWorldMapSubLayout(int *position);

/******************************************************************************
 * static
 */

static int AnimationAct(Work *work);

/******************************************************************************
 * static
 */
/******************************************************************************
 * シグナル処理
 */
static int ReceiveSignal(
	void *pwork,
	int  signal,
	int  value)
{
	Work *work = (Work *)pwork;
	int  i;

	switch(signal)
	{
	case VRWML_CLOSE:
		work->action = ACTION_DISPEND;
 		work->flag  |=  FLAG_ACTION_START;
		work->flag  &= ~FLAG_ACTION_OK;

		/* アクションの強制ストップ */
		L2D_BreakAction(work->handle_2d);
		work->act = (void *)AnimationAct;

		for(i = 0; i < BOMB_MAX; i++)
		{
			if(work->bomb[i] != NULL)
				SPR_HIDE(work->bomb[i]);
		}
	    GV_CallChildSignalFunc(work, 0x1, 0);
		break;

	case GV_SIGNAL_KILL:
		((GV_ACT *)work)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor(work);
		return 1;
	}

	return 0;
}

/******************************************************************************
 * 
 */
static void BombControl(
	Work *work)
{
	int alpha;
	int tmp;
	int i;

	tmp = 0;
	if(    (work->pos     != NULL          )
		&& (work->action  != ACTION_DISPEND)
		&& (work->bomb[0] != NULL          ))
	{
		for(i = 0; i < BOMB_MAX; i++)
		{
			/* 位置の更新 */
			work->bomb[i]->sprite.pos.x = work->pos->all_point[i].vx - 11;
			work->bomb[i]->sprite.pos.y = work->pos->all_point[i].vy - 12;

			/* アルファ値 */
			alpha  = work->bomb[i]->sprite.col.a;
			alpha += FADE_IN_SPEED * work->invers;
			if(work->invers > 0)
			{
				if(alpha >= 200)
				{
					alpha = 200;
					tmp++;
				}
			}
			else
			{
				if(alpha <= 48)
				{
					alpha = 48;
					tmp++;
				}
			}
			work->bomb[i]->sprite.col.a = alpha;

			/* フェード方向 */
			if(tmp == BOMB_MAX)
			{
				work->invers *= -1;
			}

			/* 表示フラグ */
			if(*work->bomb_stage & (0x1 << i)) SPR_SHOW(work->bomb[i]);
			else                               SPR_HIDE(work->bomb[i]);
		}
	}
}

/******************************************************************************
 * 
 */
static void NormalAct(
	Work *work)	/* ワーク */
{
//	GM_CameraSet *pCam;

	/* カメラの取得 */
//	pCam = GM_GetCurrentCameraSet(0);

	BombControl(work);

	AnimationAct(work);
}

/******************************************************************************
 * スプライトの初期化
 */
static int SprInit(
	Work *work)		/* ワーク */
{
	SPR_OBJ *spr;
	int     strcode;
	int     i;

	if(work->flag & FLAG_SPRITE_INIT)
		return 0;

	/* Ｂマークスプライト */
	if((spr = L2D_GetObject(work->handle_2d, OBJ_BOMBTEX)) == NULL)
	{
		SY_PRINTF2("bombtex\n");
		return -1;
	}
	SPR_HIDE(spr);

	/* コピーの生成 */
	for(i = work->dup_count; i < BOMB_MAX; i++)
	{
		work->bomb[i] = SPR_DuplicateTree(spr);

		if(work->bomb[i] == NULL)
		{
			SY_PRINTF2("bombtex\0");
			work->bomb[0] = NULL;
			break;
		}
		else
		{
			work->dup_count++;
		}
	}

	/* ブロックスプライト */
	strcode = OBJ_BLOCK1;
	for(i = 0; i < BLOCK_MAX; i++)
	{
		if((spr = L2D_GetObject(work->handle_2d, strcode)) == NULL)
		{
			SY_PRINTF2("block\0");
			return -1;
		}
		SPR_SHOW(spr);
		work->block[i] = spr;

		strcode++;
	}

	/* 初期化終了 */
	work->flag |= FLAG_SPRITE_INIT;

	return 0;
}

/******************************************************************************
 * 
 */
static int AnimationAct(
	Work *work)	/* ワーク */
{
	int status;

	/* アクション状態の取得 */
	status = L2D_ActionStatus(work->handle_2d);

	if((status != L2D_STAT_BUSY) && (work->flag & FLAG_ACTION_OK))
	{
		switch(work->action)
		{
		case ACTION_DEFAULT:
			/* 次のアクション */
			work->action = ACTION_DISPSTART;
			work->flag  |=  FLAG_ACTION_START;
			work->flag  &= ~FLAG_ACTION_OK;
			break;

		case ACTION_DISPSTART:
			/* 次のアクション */
			work->action = ACTION_DISPLAY;
			work->flag  |=  FLAG_ACTION_START;
			work->flag  &= ~FLAG_ACTION_OK;

			/* スプライトの初期化 */
			SprInit(work);

			/* Act 関数の変更 */
			work->act = (void *)NormalAct;
			break;

		case ACTION_DISPLAY:
			/* 同じアクション */
			work->action = ACTION_DISPLAY;
			work->flag  |=  FLAG_ACTION_START;
			work->flag  &= ~FLAG_ACTION_OK;
			break;

		case ACTION_DISPEND:
			/* 終了 */
			GV_CallParentSignalFunc(work, VRMAP_CHILD_END, 0);
			GV_DestroyActor(work);
			break;
		}
	}

	if((status != L2D_STAT_BUSY) && (work->flag & FLAG_ACTION_START))
	{
		/* アクションを開始する */
		if(status == L2D_STAT_ACK)
		{
			L2D_EvokeAction(work->handle_2d, work->action);
			work->flag &= ~FLAG_ACTION_START;
			work->flag |=  FLAG_ACTION_OK;
		}
	}

	return 0;
}

/******************************************************************************
 * 
 */
static void Act(
	Work *work)		/* ワーク */
{
	SPR_OBJ *spr;

	if(    (GM_MenuStatus & MENU_RADIO_ON)
		|| (GM_MenuStatus & MENU_NODE_ON)
		|| (GM_CheckGameStatus(STATE_PLAY_DEMO)))
	{
		// 無線が始まろうとしたら all hide
		if((spr = L2D_GetObject(work->handle_2d, OBJ_ROOT)) == NULL)
			return;
		SPR_HIDE(spr);
	}

	/* Act 関数の実行 */
	work->act(work);
}

/*******************************************************************************
 * 終了処理
 */
static void Die(
	Work *work)		/* ワーク */
{
	/* L2D */
	if(work->handle_2d >= 0)
		L2D_ReleaseLayout(work->handle_2d);
}

/******************************************************************************
 * 初期化処理
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	Work *work,		/* ワーク */
	char mode)		/* モード */
{
	void *ptr;
	int  i;

	/* 初期化 */
	for(i = 0; i < BLOCK_MAX; i++) work->block[i] = NULL;
	for(i = 0; i < BOMB_MAX ; i++) work->bomb[i]  = NULL;
	work->dup_count = 0;
	work->invers    = 1;
	work->action    = ACTION_DEFAULT;
	work->flag      = FLAG_ACTION_START;
	work->act       = (void *)AnimationAct;

	/* L2D */
	if((work->handle_2d = L2D_LoadLayout2(LAYOUT, DG_CHANL_MENU, 0, 0, GV_PAUSE_STOP)) < 0)
		return 0;

	/* Sub Layout */
	if((ptr = NewWorldMapSubLayout(work->position)) == NULL)
		return 0;
	GV_SetActorChild(work, ptr);

	return 1;
}

/******************************************************************************
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
void *NewVRWorldMapLayout(
	Pos  *pos,			/* 位置情報のアドレス */
	int  *position,		/* 位置情報のアドレス */
	char mode,			/* モード */
	int  *bomb_stage)	/* 爆弾ステージ */
{
	Work *work;

	SY_PRINTF3("NewVRWorldMapLayout\n");
	OPERATOR();

	/* アクター生成 */
	if((work = GV_NewActorPrio(GV_ACTOR_MANAGER, sizeof(Work), 200)) == NULL)
		return NULL;

	/* アクター登録 */
	GV_SetActor(&work->actor, Act, Die);
	GV_ActorEX(&work->actor);

	/* シグナル処理関数登録 */
	GV_SetActorSignalFunc(work, ReceiveSignal);

	/* ワークの初期化 */
	work->pos        = pos;
	work->position   = position;
	work->bomb_stage = bomb_stage;

	if(!GetResources(work, mode))
	{
		SY_PRINTF2("GetResources Failed.\n");
		GV_DestroyActor(work);
		return NULL;
	}

	return work;
}
