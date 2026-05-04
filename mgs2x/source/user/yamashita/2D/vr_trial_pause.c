/*******************************************************************************
 * vr_trial_pause - vr_trial_pause.c
 * ＶＲポーズ  *NewVRPause
 * 2002/07/03 S.Yamashita
 * $Id: vr_trial_pause.c,v 1.1.1.3 2002/11/19 11:51:49 Yoshizawa1 Exp $
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

#include "vr_pause.h"

#define __CHARA_NAME__ "VR Pause"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
//#define SY_PRINTF3_DEBUG
#include "../sy_util/sy_util.h"

/*******************************************************************************
 * defines
 */

/* コールバックタイプ */
enum {
	VRPAU_CB_CONT_DN = 0,	/* ＣＯＮＴＩＮＵＥで下 */
	VRPAU_CB_REST_UP,		/* ＲＥＳＴＡＲＴで上 */
	VRPAU_CB_REST_DN,		/* ＲＥＳＴＡＲＴで下 */
	VRPAU_CB_EXIT_UP,		/* ＥＸＩＴで上 */

	VRPAU_CB_CONTINUE,		/* ＣＯＮＴＩＮＵＥを実行 */
	VRPAU_CB_RESTART,		/* ＲＥＳＴＡＲＴを実行 */
	VRPAU_CB_EXIT,			/* ＥＸＩＴを実行 */
	VRPAU_CB_CANCEL,		/* キャンセル */

	VRPAU_CB_CLOSED,		/* 閉じた */
};

/* フラグ */
#define VRPAU_INIT        (0x0001)	/* 初期化時 */
#define VRPAU_WATCH_PAUSE (0x0002)	/* ポーズ状態を監視 */
#define VRPAU_SHOW        (0x0004)	/* 表示中 */
#define VRPAU_INPUT_NG    (0x0008)	/* 入力不可 */

/* 設定値 */
#define VRPAU_ICO_R     (-18)		/* アイコン相対位置Ｘ */
#define VRPAU_ICO_T     (  2)		/* アイコン相対位置Ｙ */

/* Ｌ２Ｄオブジェクト */
#define L2D_FILENAME    ( 3551242)		/* vr_pause */
#define ACTION_OPEN     (10812646)		/* openPause */
#define ACTION_CLOSE    (  129121)		/* closePause */
#define ACTION_DUMMY    ( 8143391)		/* dummy */
#define ACTION_DUMMY2   ( 8930337)		/* dummy2 */
#define OBJECT_CURSOR   ( 9100063)		/* cursor */
#define OBJECT_EXIT     ( 3435924)		/* exit */
#define OBJECT_RESTART  ( 7057413)		/* restart */
#define OBJECT_CONTINUE ( 7961512)		/* continue */
#define OBJECT_PAUSE    (16024243)		/* pause_tag */
#define OBJECT_BOTTOM   (13898726)		/* rectBottom */
#define OBJECT_TOP      ( 7044000)		/* rectTop */

/*******************************************************************************
 * work
 */

/* ワーク */
typedef struct tagWORK
{
	GV_ACT_EX actor;			/* アクター */

	int       name;				/* シナリオ名 */
	int       proc_restart;		/* ＲＥＳＴＡＲＴプロック */
	int       proc_exit;		/* ＥＸＩＴプロック */
	short     flag;				/* フラグ */
	short     count;			/* カウンター */

	LAYOUTMAN layoutman;		/* レイアウトマネージャ */
}
WORK;

/*******************************************************************************
 * local
 */

static void *_work = NULL;	/* 多重起動防止 */

/* ＬＯＭデータ */
static int _lom_data[] = {
					/*アクションリスト名*/		/*アクション名*/	/*カウント*/	/*フラグ*/
LOM_ACTLIST_DATA,	LOM_ACTLIST_00,				ACTION_CLOSE,		1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_01,				ACTION_OPEN,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_ACTLIST_02,				ACTION_CLOSE,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												ACTION_DUMMY,		1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												ACTION_DUMMY2,		1,				LOM_ACT_FRAME|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_DATA_END,

					/*アイコンリスト名*/		/* アイコン名 */	/* テクスチャ名 */	/* 位置(左、上、右、下) */								/* 色 */												/* カウント */	/* フラグ */
LOM_ICOLIST_DATA,	LOM_ICOLIST_00,				LOM_ICO_00,			LOM_ICO_NOTEX,		VRPAU_ICO_R, VRPAU_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H,		VRDEF_CUR_R, VRDEF_CUR_G, VRDEF_CUR_B,           0,		LOM_ICO_LOOP,	LOM_ICO_L_REL|LOM_ICO_T_REL|LOM_ICO_SIZE|LOM_ICO_ALPHA,
												LOM_ICO_01,			LOM_ICO_NOTEX,		VRPAU_ICO_R, VRPAU_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H,		VRDEF_CUR_R, VRDEF_CUR_G, VRDEF_CUR_B, VRDEF_CUR_A,		LOM_ICO_LOOP,	LOM_ICO_L_REL|LOM_ICO_T_REL|LOM_ICO_SIZE|LOM_ICO_ALPHA,
												LOM_DATA_END,
					LOM_DATA_END,

					/* オブジェクトリスト名 */	/* オブジェクト名 */	/* アイコンリスト名 */	/* カウント */	/* フラグ */
LOM_OBJLIST_DATA,	LOM_OBJLIST_00,				OBJECT_CONTINUE,		LOM_ICOLIST_00,			4,				LOM_OBJ_INTERP_1,
												OBJECT_RESTART,			LOM_ICOLIST_00,			4,				LOM_OBJ_INTERP_1,
												OBJECT_EXIT,			LOM_ICOLIST_00,			4,				LOM_OBJ_INTERP_1,
												LOM_DATA_END,
					LOM_DATA_END,

					/*パッドリスト名*/			/*アクション名*/	/*オブジェクト名*/	/*入力*/			/*実行番号*/		/*実行引数*/		/*フラグ*/
LOM_PADLIST_DATA,	LOM_PADLIST_00,				ACTION_OPEN,		OBJECT_CONTINUE,	PAD_D,				LOM_EXE_CALLBACK,	VRPAU_CB_CONT_DN,	LOM_PAD_ON_SIG,
												ACTION_OPEN,		OBJECT_RESTART,		PAD_U,				LOM_EXE_CALLBACK,	VRPAU_CB_REST_UP,	LOM_PAD_ON_SIG,
												ACTION_OPEN,		OBJECT_RESTART,		PAD_D,				LOM_EXE_CALLBACK,	VRPAU_CB_REST_DN,	LOM_PAD_ON_SIG,
												ACTION_OPEN,		OBJECT_EXIT,		PAD_U,				LOM_EXE_CALLBACK,	VRPAU_CB_EXIT_UP,	LOM_PAD_ON_SIG,

												ACTION_OPEN,		OBJECT_CONTINUE,	PAD_OK,				LOM_EXE_CALLBACK,	VRPAU_CB_CONTINUE,	LOM_PAD_PRESS,
												ACTION_OPEN,		OBJECT_RESTART,		PAD_OK,				LOM_EXE_CALLBACK,	VRPAU_CB_RESTART,	LOM_PAD_PRESS,
												ACTION_OPEN,		OBJECT_EXIT,		PAD_OK,				LOM_EXE_CALLBACK,	VRPAU_CB_EXIT,		LOM_PAD_PRESS,
												LOM_DATA_END,

					LOM_PADLIST_01,				ACTION_DUMMY,		LOM_PAD_ALLOBJ,		LOM_PAD_ANYINPUT,	LOM_EXE_CALLBACK,	VRPAU_CB_CLOSED,	0,
												LOM_DATA_END,

					LOM_DATA_END,

					/*モード名*/	/*ＬＯＭアクションリスト*/	/*ＬＯＭオブジェクトリスト*/	/*ＬＯＭパッドリスト*/
LOM_MODE_DATA,		LOM_MODE_00,	LOM_ACTLIST_00,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* init */
					LOM_MODE_01,	LOM_ACTLIST_01,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* open */
					LOM_MODE_02,	LOM_ACTLIST_02,				LOM_MODE_NOLIST,				LOM_PADLIST_01,		/* close */
					LOM_DATA_END,
LOM_DATA_END,
};

/*******************************************************************************
 * extern
 */

extern int GM_COM_PauseOff(void);

/*******************************************************************************
 * static
 */
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

	if(work->flag & VRPAU_INPUT_NG)
		return 1;

	switch(param)
	{
	case VRPAU_CB_CONT_DN:
		LOM_SetCurObj (&work->layoutman, OBJECT_RESTART, 0);
		GM_SdSet(SD_S_CUR01);
		return 1;
	case VRPAU_CB_REST_UP:
		LOM_SetCurObj (&work->layoutman, OBJECT_CONTINUE, 0);
		GM_SdSet(SD_S_CUR01);
		return 1;
	case VRPAU_CB_REST_DN:
		LOM_SetCurObj (&work->layoutman, OBJECT_EXIT, 0);
		GM_SdSet(SD_S_CUR01);
		return 1;
	case VRPAU_CB_EXIT_UP:
		LOM_SetCurObj (&work->layoutman, OBJECT_RESTART, 0);
		GM_SdSet(SD_S_CUR01);
		return 1;

	case VRPAU_CB_CONTINUE:
		/* ポーズを解除する */
		GM_SdSet(SD_S_WIN01);
		GM_COM_PauseOff();
		LOM_SetIcoPos(&work->layoutman, OBJECT_CONTINUE, VRPAU_ICO_R, VRPAU_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
		LOM_SetCurMode(&work->layoutman, LOM_MODE_02, 1, 1, 1);
		work->count = 0;
		SY_PRINTF1("LOM MODE 02 (1)\n");
		return 1;

	case VRPAU_CB_RESTART:
		if(work->proc_restart != 0)
		{
			GM_COM_PauseOff();
			SY_PRINTF1("Proc RESTART");
			GM_SdSet(SD_S_WIN01);
			MSN_RETRY_COUNT++;
			SY_PRINTF1("RETRY %d\n", VR_RetryCount);
			GM_DiazepamCount = 0 ;
			GM_ExecProc(work->proc_restart, NULL);
		}
		return 1;

	case VRPAU_CB_EXIT:
		if(work->proc_exit != 0)
		{
			GM_COM_PauseOff();
			SY_PRINTF1("Proc EXIT");
			GM_SdSet( SNG_STOP );	/* BGM停止 */
			GM_SdSet(SD_S_START01);
			GM_DiazepamCount = 0 ;
			GM_ExecProc(work->proc_exit, NULL);
		}
		return 1;

	case VRPAU_CB_CANCEL:
		/* ポーズを解除する */
		GM_SdSet(SD_S_V_CANS02);
		GM_COM_PauseOff();
		LOM_SetIcoPos(&work->layoutman, OBJECT_CONTINUE, VRPAU_ICO_R, VRPAU_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
		LOM_SetCurMode(&work->layoutman, LOM_MODE_02, 1, 1, 1);
		work->count = 0;
		SY_PRINTF1("LOM MODE 02 (2)\n");
		return 1;

	case VRPAU_CB_CLOSED:
		return 1;

	default:
		return 1;
	}
}
#if 0
/*******************************************************************************
 * シグナル処理
	VRPAU_OFF - value: 0
	VRPAU_ON  - value: 0
 */
static int ReceiveSignal(	/* 処理結果 */
	void *pWork,	/* ワーク */
	int  signal,	/* シグナル */
	int  value)		/* 付加情報 */
{
	WORK *work = (WORK *)pWork;

	SY_PRINTF3("ReceiveSignal\n");
	SY_PRINTF1("Signal: %d\n", signal);

	switch(signal)
	{
	case VRPAU_OFF:
		if(LOM_GetCurMode(&work->layoutman) == LOM_MODE_01)
		{
			LOM_SetIcoPos(&work->layoutman, OBJECT_CONTINUE, VRPAU_ICO_R, VRPAU_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
			LOM_SetCurMode(&work->layoutman, LOM_MODE_02, 1, 1, 1);
			SY_PRINTF1("LOM MODE 02 (3)\n");
			return 1;
		}
		else return 0;

	case VRPAU_ON:
		if(LOM_GetCurMode(&work->layoutman) != LOM_MODE_01)
		{
			GM_SdSet(SD_S_WIN01);
			LOM_SetCurMode(&work->layoutman, LOM_MODE_01, 1, 1, 1);
			SY_PRINTF1("LOM MODE 01 (1)\n");
			return 1;
		}
		else return 0;

	default:
		/* 更に子供に送る */
		return GV_DefaultSignalFunc(work, signal, value);
	}
}

/*******************************************************************************
 * メッセージ処理

mesg ＶＲポーズ $s:名前 ＯＦＦ[0]
mesg ＶＲポーズ $s:名前 ＯＮ[1]
 */
static int ReceiveMessage(	/* 処理結果 */
	WORK *work)		/* ワーク */
{
	int    i;
	int    msg_count;
	int    ret = 0;
	GV_MSG *msg;

	if(work->name == 0)
		return ret;

	msg_count = GV_ReceiveMessage(work->name, &msg);
	for(i = 0; i < msg_count; i++, msg++)
	{
		SY_PRINTF2("Message received.\n");

		switch(msg->message[0])
		{
		case 0:		/* ＯＦＦ */
			if(LOM_GetCurMode(&work->layoutman) == LOM_MODE_01)
			{
				LOM_SetIcoPos(&work->layoutman, OBJECT_CONTINUE, VRPAU_ICO_R, VRPAU_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
				LOM_SetCurMode(&work->layoutman, LOM_MODE_02, 1, 1, 1);
				SY_PRINTF1("LOM MODE 02 (4)\n");
				ret = 1;
			}
			break;

		case 1:		/* ＯＮ */
			if(LOM_GetCurMode(&work->layoutman) != LOM_MODE_01)
			{
				GM_SdSet(SD_S_WIN01);
				LOM_SetCurMode(&work->layoutman, LOM_MODE_01, 1, 1, 1);
				SY_PRINTF1("LOM MODE 01 (2)\n");
				ret = 1;
			}
			break;

		default:
			ret = -1;
			break;
		}
	}

	return ret;
}
#endif
/*******************************************************************************
 * 毎フレーム処理
 */
static void Act(
	WORK *work)		/* ワーク */
{
	SPR_COLOR col;

	if(work->flag & VRPAU_INIT)
	{
		/*
		 * レイアウトの初期化
		 */
		work->flag &= ~VRPAU_INIT;

		/* モードの設定 */
		LOM_SetCurMode(&work->layoutman, LOM_MODE_00, 1, 1, 1);
		SY_PRINTF1("LOM MODE 00 (1)\n");
	}

	work->count++;

	/* メッセージ処理 */
//	ReceiveMessage(work);

	/* レイアウトマネージャ */
	ActLayoutman(&work->layoutman);

	/* ポーズ状態を監視 */
	if(work->flag & VRPAU_WATCH_PAUSE)
	{
		if(    (GV_PauseLevel == GV_PAUSE_PAUSE)
			&& !(MSN_2DSTATUS & MSN_2DSTAT_WINDOW_PAUSE)
			&& !(GV_PadData[0].flag & GV_PAD_RELEASE))
		{
			/* ポーズ中 */
			if(!(work->flag & VRPAU_SHOW))
			{
				work->flag |= VRPAU_SHOW;

				GM_SdSet(SD_S_WIN01);
				LOM_SetCurMode(&work->layoutman, LOM_MODE_01, 1, 1, 1);
				work->count = 0;
				work->flag |= VRPAU_INPUT_NG;
				SY_PRINTF1("LOM MODE 01 (3)\n");
			}
		}
		else
		{
			/* ポーズ解除中 */
			if(work->flag & VRPAU_SHOW)
			{
				work->flag &= ~VRPAU_SHOW;

				LOM_SetIcoPos(&work->layoutman, OBJECT_CONTINUE, VRPAU_ICO_R, VRPAU_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
				LOM_SetCurMode(&work->layoutman, LOM_MODE_02, 1, 1, 1);
				work->count = 0;
				SY_PRINTF1("LOM MODE 02 (5)\n");
			}
		}
	}

	/* テキストの色 */
	if(LOM_GetCurMode(&work->layoutman) == LOM_MODE_01)
	{
		switch(LOM_GetCurObjI(&work->layoutman))
		{
		case 0:
			LOM_Spr_GetColor(&work->layoutman, OBJECT_CONTINUE, &col);
			col.r = VRDEF_ACTIVE_R;
			col.g = VRDEF_ACTIVE_G;
			col.b = VRDEF_ACTIVE_B;
			LOM_Spr_SetColor(&work->layoutman, OBJECT_CONTINUE, &col);
			LOM_Spr_GetColor(&work->layoutman, OBJECT_RESTART , &col);
			col.r = VRDEF_INACTIVE_R;
			col.g = VRDEF_INACTIVE_G;
			col.b = VRDEF_INACTIVE_B;
			LOM_Spr_SetColor(&work->layoutman, OBJECT_RESTART , &col);
			LOM_Spr_GetColor(&work->layoutman, OBJECT_EXIT    , &col);
			col.r = VRDEF_INACTIVE_R;
			col.g = VRDEF_INACTIVE_G;
			col.b = VRDEF_INACTIVE_B;
			LOM_Spr_SetColor(&work->layoutman, OBJECT_EXIT    , &col);
			break;

		case 1:
			LOM_Spr_GetColor(&work->layoutman, OBJECT_CONTINUE, &col);
			col.r = VRDEF_INACTIVE_R;
			col.g = VRDEF_INACTIVE_G;
			col.b = VRDEF_INACTIVE_B;
			LOM_Spr_SetColor(&work->layoutman, OBJECT_CONTINUE, &col);
			LOM_Spr_GetColor(&work->layoutman, OBJECT_RESTART , &col);
			col.r = VRDEF_ACTIVE_R;
			col.g = VRDEF_ACTIVE_G;
			col.b = VRDEF_ACTIVE_B;
			LOM_Spr_SetColor(&work->layoutman, OBJECT_RESTART , &col);
			LOM_Spr_GetColor(&work->layoutman, OBJECT_EXIT    , &col);
			col.r = VRDEF_INACTIVE_R;
			col.g = VRDEF_INACTIVE_G;
			col.b = VRDEF_INACTIVE_B;
			LOM_Spr_SetColor(&work->layoutman, OBJECT_EXIT    , &col);
			break;

		case 2:
			LOM_Spr_GetColor(&work->layoutman, OBJECT_CONTINUE, &col);
			col.r = VRDEF_INACTIVE_R;
			col.g = VRDEF_INACTIVE_G;
			col.b = VRDEF_INACTIVE_B;
			LOM_Spr_SetColor(&work->layoutman, OBJECT_CONTINUE, &col);
			LOM_Spr_GetColor(&work->layoutman, OBJECT_RESTART , &col);
			col.r = VRDEF_INACTIVE_R;
			col.g = VRDEF_INACTIVE_G;
			col.b = VRDEF_INACTIVE_B;
			LOM_Spr_SetColor(&work->layoutman, OBJECT_RESTART , &col);
			LOM_Spr_GetColor(&work->layoutman, OBJECT_EXIT    , &col);
			col.r = VRDEF_ACTIVE_R;
			col.g = VRDEF_ACTIVE_G;
			col.b = VRDEF_ACTIVE_B;
			LOM_Spr_SetColor(&work->layoutman, OBJECT_EXIT    , &col);
			break;

		default:
			ASSERT(0);
			break;
		}
	}

#ifdef PAL
	if(work->count == 13)
#else
	if(work->count == 15)
#endif
	{
		work->flag &= ~VRPAU_INPUT_NG;
		LOM_SetCurIcoI(&work->layoutman, 1);
	}
	if(work->count > 1000)
		work->count = 1000;
}

/*******************************************************************************
 * 終了処理
 */
static void Die(
	WORK *work)		/* ワーク */
{
	SY_PRINTF3("Die\n");

	/* レイアウトマネージャ */
	DestroyLayoutman(&work->layoutman);

	/* 多重起動防止 */
	_work = NULL;
}

/*******************************************************************************
 * 初期化処理

chara ＶＲポーズ[NewVRPause_Scn] $s:名前 \
	// 以下オプション
	-restart $p:ＲＥＳＴＡＲＴプロック \
	-exit    $p:ＥＸＩＴプロック
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	WORK *work,			/* ワーク */
	int  name,			/* シナリオ名 */
	int  where,			/* マップＩＤ */
	int  proc_restart,	/* ＲＥＳＴＡＲＴプロック */
	int  proc_exit)		/* ＥＸＩＴプロック */
{
	SY_PRINTF3("GetResources\n");

	work->name         = name;
	work->proc_restart = proc_restart;
	work->proc_exit    = proc_exit;
	work->flag         = VRPAU_WATCH_PAUSE;
	work->count        = 0;

	/* シナリオオプション */
	if(name != 0)
	{
		/* restart */
		if(GCL_GetOption('r') != NULL)
			work->proc_restart = GCL_GetNextInt();

		/* exit */
		if(GCL_GetOption('e') != NULL)
			work->proc_exit = GCL_GetNextInt();

		SY_PRINTF1("Get Scn Option Succeeded.\n");
	}

	/* レイアウトマネージャ */
	{
		if(CreateLayoutman4(&work->layoutman, L2D_FILENAME, 4, 0, 0, 0, 0, 0, LOMCallback, NULL, work, 15, 10, LOM_KEY_REP_1|LOM_INPUT_DIRECT) == 0)
		{
			SY_PRINTF2("Create Layoutman Failed.\n");
			return 0;
		}
		SY_PRINTF1("Create Layoutman Succeeded.\n");

		if(LoadLOMData(&work->layoutman, _lom_data) == 0)
		{
			SY_PRINTF2("Load LOM Data Failed.\n");
			return 0;
		}
		SY_PRINTF1("Load LOM Data Succeeded.\n");
	}

	work->flag |= VRPAU_INIT;

	/* 多重起動防止 */
	_work = work;

	return 1;
}

/*******************************************************************************
 * public
 */
/*******************************************************************************
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
void *NewVRPause(
	int proc_restart,	/* ＲＥＳＴＡＲＴプロック */
	int proc_exit)		/* ＥＸＩＴプロック */
{
	WORK *work;

	SY_PRINTF3("NewVRPause\n");
	OPERATOR();

	/* 多重起動防止 */
	if(_work != NULL)
		return(_work);

	/* アクター生成 */
	work = (WORK *)GV_CreateActor(
		GV_ACTOR_AFTER2,			/* プロセスの優先順位 */
		GV_CLASS_CHARA,				/* プロセスクラス */
		sizeof(WORK),				/* メモリ確保サイズ */
		0);							/* 実行プライオリティ */
	if(work != NULL)
	{
		/* アクター登録 */
		GV_SetActor(&work->actor, Act, Die);
		GV_ActorEX(&work->actor);

		/* シグナル処理関数登録 */
//		GV_SetActorSignalFunc(work, ReceiveSignal);

		/* ワークの初期化 */
		if(!GetResources(work, 0, GM_CurrentMap, proc_restart, proc_exit))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}

void *NewVRPause_Scn(
	int name,	/* シナリオ名 */
	int where)	/* マップＩＤ */
{
	WORK *work;

	SY_PRINTF3("NewVRPause_Scn\n");
	OPERATOR();

	/* 多重起動防止 */
	if(_work != NULL)
		return(_work);

	/* アクター生成 */
	work = (WORK *)GV_CreateActor(
		GV_ACTOR_AFTER2,			/* プロセスの優先順位 */
		GV_CLASS_CHARA,				/* プロセスクラス */
		sizeof(WORK),				/* メモリ確保サイズ */
		0);							/* 実行プライオリティ */
	if(work != NULL)
	{
		/* アクター登録 */
		GV_SetActor(&work->actor, Act, Die);
		GV_ActorEX(&work->actor);

		/* ワークの初期化 */
		if(!GetResources(work, name, where, 0, 0))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}
