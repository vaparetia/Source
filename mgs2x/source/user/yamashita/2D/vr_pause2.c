//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * vr_pause2 - vr_pause2.c
 * ＶＲポーズ２  *NewVRPause2
 * 2002/07/25 S.Yamashita
 * $Id: vr_pause2.c,v 1.1.1.3 2002/11/19 11:51:44 Yoshizawa1 Exp $
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

#include "debugmenu.h"

#include "../../sigeno/vr/vr.h"
#include "msn.h"
#include "vr_def.h"
#include "layoutman.h"

#include "vr_pause2.h"

#define __CHARA_NAME__ "VR Pause2"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
//#define SY_PRINTF3_DEBUG
#include "../sy_util/sy_util.h"

#include "BP_Misc.h"

extern void ShowExitGameWarning(int *pResult);

/*******************************************************************************
 * defines
 */

/* コールバックタイプ */
enum {
	VRPAU2_CB_CONT_R = 0,		/* ＣＯＮＴＩＮＵＥで右 */
	VRPAU2_CB_REST_L,			/* ＲＥＳＴＡＲＴで左 */
	VRPAU2_CB_REST_R,			/* ＲＥＳＴＡＲＴで右 */
	VRPAU2_CB_EXIT_L,			/* ＥＸＩＴで左 */

	VRPAU2_CB_CONTINUE,			/* ＣＯＮＴＩＮＵＥを実行 */
	VRPAU2_CB_RESTART,			/* ＲＥＳＴＡＲＴを実行 */
	VRPAU2_CB_EXIT,				/* ＥＸＩＴを実行 */

	VRPAU2_CB_CONTINUE_STA,		/* ＣＯＮＴＩＮＵＥを実行（スタートボタン） */
	VRPAU2_CB_RESTART_STA,		/* ＲＥＳＴＡＲＴを実行（スタートボタン） */
	VRPAU2_CB_EXIT_STA,			/* ＥＸＩＴを実行（スタートボタン） */
};

/* フラグ */
#define VRPAU2_INIT        (0x0001)	/* 初期化時 */
#define VRPAU2_WATCH_PAUSE (0x0002)	/* ポーズ状態を監視 */
#define VRPAU2_SHOW        (0x0004)	/* 表示中 */
#define VRPAU2_INPUT_NG    (0x0008)	/* 入力不可 */
#define VRPAU2_WATCH_STA   (0x0010)	/* スタートボタン監視中 */

#define VRPAU2_SELECTED    (0x0700)	/* & で選択されたかチェック */
#define VRPAU2_SEL_CONT    (0x0100)	/* CONTINUE 項目を選択 */
#define VRPAU2_SEL_REST    (0x0200)	/* RESTART 項目を選択 */
#define VRPAU2_SEL_EXIT    (0x0400)	/* EXIT 項目を選択 */

/* Ｌ２Ｄオブジェクト */
#define L2D_FILENAME    (13935504)		/* vr_pause_map */
#define ACTION_SHOW     ( 3878487)		/* show */

#define OBJECT_CONTINUE ( 7961512)		/* continue */
#define OBJECT_RESTART  ( 7057413)		/* restart */
#define OBJECT_EXIT     ( 3435924)		/* exit */

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

/* ＬＯＭデータ */
static int _lom_data[] = {
					/*アクションリスト名*/		/*アクション名*/	/*カウント*/	/*フラグ*/
LOM_ACTLIST_DATA,	LOM_ACTLIST_00,				ACTION_SHOW,		1,				LOM_ACT_COUNT|LOM_ACT_HIDE,
												LOM_DATA_END,
					LOM_ACTLIST_01,				ACTION_SHOW,		1,				LOM_ACT_COUNT|LOM_ACT_SHOW,
												LOM_DATA_END,
					LOM_DATA_END,

					/*アイコンリスト名*/		/* アイコン名 */	/* テクスチャ名 */	/* 位置(左、上、右、下) */	/* 色 */	/* カウント */	/* フラグ */
LOM_ICOLIST_DATA,	LOM_ICOLIST_00,				LOM_ICO_00,			LOM_ICO_NOTEX,		0, 0, 0, 0,					0, 0, 0, 0,	LOM_ICO_LOOP,	LOM_ICO_ALPHA,
												LOM_DATA_END,
					LOM_DATA_END,

					/* オブジェクトリスト名 */	/* オブジェクト名 */	/* アイコンリスト名 */	/* カウント */	/* フラグ */
LOM_OBJLIST_DATA,	LOM_OBJLIST_00,				OBJECT_CONTINUE,		LOM_ICOLIST_00,			1,				LOM_OBJ_INTERP_1,
												OBJECT_RESTART,			LOM_ICOLIST_00,			1,				LOM_OBJ_INTERP_1,
												OBJECT_EXIT,			LOM_ICOLIST_00,			1,				LOM_OBJ_INTERP_1,
												LOM_DATA_END,
					LOM_DATA_END,

					/*パッドリスト名*/			/*アクション名*/	/*オブジェクト名*/	/*入力*/			/*実行番号*/		/*実行引数*/			/*フラグ*/
LOM_PADLIST_DATA,	LOM_PADLIST_00,				ACTION_SHOW,		OBJECT_CONTINUE,	PAD_R,				LOM_EXE_CALLBACK,	VRPAU2_CB_CONT_R,		LOM_PAD_ON_SIG,
												ACTION_SHOW,		OBJECT_RESTART,		PAD_L,				LOM_EXE_CALLBACK,	VRPAU2_CB_REST_L,		LOM_PAD_ON_SIG,
												ACTION_SHOW,		OBJECT_RESTART,		PAD_R,				LOM_EXE_CALLBACK,	VRPAU2_CB_REST_R,		LOM_PAD_ON_SIG,
												ACTION_SHOW,		OBJECT_EXIT,		PAD_L,				LOM_EXE_CALLBACK,	VRPAU2_CB_EXIT_L,		LOM_PAD_ON_SIG,

												ACTION_SHOW,		OBJECT_CONTINUE,	LOM_PAD_OK,				LOM_EXE_CALLBACK,	VRPAU2_CB_CONTINUE,		LOM_PAD_RELEASE,
												ACTION_SHOW,		OBJECT_RESTART,		LOM_PAD_OK,				LOM_EXE_CALLBACK,	VRPAU2_CB_RESTART,		LOM_PAD_RELEASE,
												ACTION_SHOW,		OBJECT_EXIT,		LOM_PAD_OK,				LOM_EXE_CALLBACK,	VRPAU2_CB_EXIT,			LOM_PAD_RELEASE,

												ACTION_SHOW,		OBJECT_CONTINUE,	PAD_STA,			LOM_EXE_CALLBACK,	VRPAU2_CB_CONTINUE_STA,	LOM_PAD_RELEASE,
												ACTION_SHOW,		OBJECT_RESTART,		PAD_STA,			LOM_EXE_CALLBACK,	VRPAU2_CB_RESTART_STA,	LOM_PAD_RELEASE,
												ACTION_SHOW,		OBJECT_EXIT,		PAD_STA,			LOM_EXE_CALLBACK,	VRPAU2_CB_EXIT_STA,		LOM_PAD_RELEASE,
												LOM_DATA_END,
					LOM_DATA_END,

					/*モード名*/	/*ＬＯＭアクションリスト*/	/*ＬＯＭオブジェクトリスト*/	/*ＬＯＭパッドリスト*/
LOM_MODE_DATA,		LOM_MODE_00,	LOM_ACTLIST_00,				LOM_MODE_NOLIST,				LOM_MODE_NOLIST,	/* init */
					LOM_MODE_01,	LOM_ACTLIST_01,				LOM_OBJLIST_00,					LOM_PADLIST_00,		/* open */
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
 * スプライトの色の設定
 */
static void SetSpriteColor(
	LAYOUTMAN    *layoutman,		/* レイアウトマネージャ */
	int          strcode,			/* 文字列コード */
	int          active,			/* アクティブフラグ  0:非アクティブ  1:アクティブ */
	int          step,				/* 変化度 */
	unsigned int inactive_color,	/* 非アクティブ色 */
	unsigned int active_color)		/* アクティブ色 */
{
	SPR_COLOR    sprcol;
	unsigned int color;
	int          add;

	{
		SPR_OBJ *spr;
		if((spr = L2D_GetObject(layoutman->layout, strcode)) == NULL)
			return;
	}

	if(active == 0) color = inactive_color;
	else            color = active_color;

	LOM_Spr_GetColor(layoutman, strcode, &sprcol);
	{
		if((abs(add = (color & 0x000000ff) - sprcol.r)) <= step)
		                 sprcol.r  = (color & 0x000000ff);
		else if(add > 0) sprcol.r += step;
		else             sprcol.r -= step;

		if((abs(add = ((color & 0x0000ff00) >> 8) - sprcol.g)) <= step)
		                 sprcol.g  = ((color & 0x0000ff00) >> 8);
		else if(add > 0) sprcol.g += step;
		else             sprcol.g -= step;

		if((abs(add = ((color & 0x00ff0000) >> 16) - sprcol.b)) <= step)
		                 sprcol.b  = ((color & 0x00ff0000) >> 16);
		else if(add > 0) sprcol.b += step;
		else             sprcol.b -= step;
	}
	LOM_Spr_SetColor(layoutman, strcode, &sprcol);
}

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

	if((work->flag & VRPAU2_WATCH_STA) || (work->flag & VRPAU2_INPUT_NG) || (work->flag & VRPAU2_SELECTED))
		return 1;

	switch(param)
	{
	case VRPAU2_CB_CONT_R:
		LOM_SetCurObj (&work->layoutman, OBJECT_RESTART, 0);
		GM_SdSet(SD_S_CUR01);
		return 1;
	case VRPAU2_CB_REST_L:
		LOM_SetCurObj (&work->layoutman, OBJECT_CONTINUE, 0);
		GM_SdSet(SD_S_CUR01);
		return 1;
	case VRPAU2_CB_REST_R:
		LOM_SetCurObj (&work->layoutman, OBJECT_EXIT, 0);
		GM_SdSet(SD_S_CUR01);
		return 1;
	case VRPAU2_CB_EXIT_L:
		LOM_SetCurObj (&work->layoutman, OBJECT_RESTART, 0);
		GM_SdSet(SD_S_CUR01);
		return 1;

#ifdef PSX2
	case VRPAU2_CB_CONTINUE_STA:
	case VRPAU2_CB_RESTART_STA:
	case VRPAU2_CB_EXIT_STA:
#endif
#ifndef PSX2
	case VRPAU2_CB_CONTINUE_STA:
#endif
	case VRPAU2_CB_CONTINUE:
		work->flag &= ~VRPAU2_SELECTED;
		work->flag |=  VRPAU2_SEL_CONT;
		work->count = 0;
		return 1;

#ifndef PSX2
	case VRPAU2_CB_RESTART_STA:
#endif
	case VRPAU2_CB_RESTART:
      {
         //BP - added system dialog on X360 to prevent destructive action
         //without confirmation.
         //NOTE: this pause menu is for Alternative missions.
         int confirmRet;
         ShowExitGameWarning( &confirmRet );
         if( confirmRet == 0 )   //yes
         {
		      work->flag &= ~VRPAU2_SELECTED;
		      work->flag |=  VRPAU2_SEL_REST;
		      work->count = 0;

		      GM_SdSet( SE_ALL_STOP );
         }
         else
         {
            //Pretend the user did not ever hit the button.
         }
      }
		return 1;

#ifndef PSX2
	case VRPAU2_CB_EXIT_STA:
#endif
	case VRPAU2_CB_EXIT:
      {
         //BP - added system dialog on X360 to prevent destructive action
         //without confirmation.
         //NOTE: this pause menu is for Alternative missions.
         int confirmRet;
         ShowExitGameWarning( &confirmRet );
         if( confirmRet == 0 )   //yes
         {
            work->flag &= ~VRPAU2_SELECTED;
            work->flag |=  VRPAU2_SEL_EXIT;
            work->count = 0;

            GM_SdSet( SE_ALL_STOP );
         }
         else
         {
            //Pretend the user did not ever hit the button.
         }
      }
		return 1;

	default:
		return 1;
	}
}

/*******************************************************************************
 * シグナル処理
	VRPAU2_OFF - value: 0
	VRPAU2_ON  - value: 0
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
	case VRPAU2_OFF:
		/* ポーズ解除 */
		if(work->flag & VRPAU2_SHOW)
		{
			work->flag &= ~VRPAU2_SHOW;
			LOM_SetCurMode(&work->layoutman, LOM_MODE_00, 1, 1, 1);
			work->count = 0;
			work->flag &= ~VRPAU2_SELECTED;
			return 1;
		}
		else return 0;

	case VRPAU2_ON:
		/* ポーズ */
		if(!(work->flag & VRPAU2_SHOW))
		{
			/* スタートボタン監視開始 */
			work->flag |= VRPAU2_WATCH_STA;

			work->flag |= VRPAU2_SHOW;
			GM_SdSet(SD_S_WIN01);
			LOM_SetCurMode(&work->layoutman, LOM_MODE_01, 1, 1, 1);
			work->count = 0;
			work->flag |= VRPAU2_INPUT_NG;
			return 1;
		}
		else return 0;

	default:
		/* 更に子供に送る */
		return GV_DefaultSignalFunc(work, signal, value);
	}
}

/*******************************************************************************
 * 毎フレーム処理
 */
static void Act(
	WORK *work)		/* ワーク */
{
	if(work->flag & VRPAU2_INIT)
	{
		/*
		 * レイアウトの初期化
		 */
		work->flag &= ~VRPAU2_INIT;

		/* モードの設定 */
		LOM_SetCurMode(&work->layoutman, LOM_MODE_00, 1, 1, 1);
	}

	work->count++;

	/* レイアウトマネージャ */
	ActLayoutman(&work->layoutman);

	/* テキストの色 */
	if(LOM_GetCurMode(&work->layoutman) == LOM_MODE_01)
	{
		if(work->flag & VRPAU2_INPUT_NG)
		{
			SetSpriteColor(&work->layoutman, OBJECT_CONTINUE, 1, 256, VRDEF_UNPLAYA_I, VRDEF_PLAYABL_A);
			SetSpriteColor(&work->layoutman, OBJECT_RESTART , 0, 256, VRDEF_UNPLAYA_I, VRDEF_PLAYABL_A);
			SetSpriteColor(&work->layoutman, OBJECT_EXIT    , 0, 256, VRDEF_UNPLAYA_I, VRDEF_PLAYABL_A);
		}
		else
		{
			switch(LOM_GetCurObjI(&work->layoutman))
			{
			case 0:
				SetSpriteColor(&work->layoutman, OBJECT_CONTINUE, 1, 32, VRDEF_UNPLAYA_I, VRDEF_PLAYABL_A);
				SetSpriteColor(&work->layoutman, OBJECT_RESTART , 0, 32, VRDEF_UNPLAYA_I, VRDEF_PLAYABL_A);
				SetSpriteColor(&work->layoutman, OBJECT_EXIT    , 0, 32, VRDEF_UNPLAYA_I, VRDEF_PLAYABL_A);
				break;

			case 1:
				SetSpriteColor(&work->layoutman, OBJECT_CONTINUE, 0, 32, VRDEF_UNPLAYA_I, VRDEF_PLAYABL_A);
				SetSpriteColor(&work->layoutman, OBJECT_RESTART , 1, 32, VRDEF_UNPLAYA_I, VRDEF_PLAYABL_A);
				SetSpriteColor(&work->layoutman, OBJECT_EXIT    , 0, 32, VRDEF_UNPLAYA_I, VRDEF_PLAYABL_A);
				break;

			case 2:
				SetSpriteColor(&work->layoutman, OBJECT_CONTINUE, 0, 32, VRDEF_UNPLAYA_I, VRDEF_PLAYABL_A);
				SetSpriteColor(&work->layoutman, OBJECT_RESTART , 0, 32, VRDEF_UNPLAYA_I, VRDEF_PLAYABL_A);
				SetSpriteColor(&work->layoutman, OBJECT_EXIT    , 1, 32, VRDEF_UNPLAYA_I, VRDEF_PLAYABL_A);
				break;

			default:
				ASSERT(0);
				break;
			}
		}
	}

	if(work->flag & VRPAU2_SELECTED)
	{
		if(work->count == 1)
		{
			if(work->flag & VRPAU2_SEL_CONT)
			{
				/* ポーズを解除する */
//				GM_SdSet(SD_S_WIN01);
				GM_GameStatus &= ~STATE_PAUSE_DISABLE;
				GM_COM_PauseOff();

				work->flag &= ~VRPAU2_SHOW;
				LOM_SetCurMode(&work->layoutman, LOM_MODE_00, 1, 1, 1);
				work->count = 0;
				work->flag &= ~VRPAU2_SELECTED;
			}
			else if(work->flag & VRPAU2_SEL_REST)
			{
				if(work->proc_restart != 0)
				{
					GM_GameStatus &= ~STATE_PAUSE_DISABLE;
					GM_COM_PauseOff();

					SY_PRINTF1("Proc RESTART");
					GM_SdSet( SNG_STOP );	/* BGM停止 */
					GM_SdSet( SE_EXP_STOP );	/* 拡張ＳＥ停止 */
					GM_SdSet(SD_S_WIN01);
					MSN_RETRY_COUNT++;
					SY_PRINTF1("RETRY %d\n", VR_RetryCount);
					GM_ExecProc(work->proc_restart, NULL);

					// リスタート処理
					{
//						extern	int		GM_DiazepamCount ;
						/* ポーズ解除 */
//						GV_PauseOffActorSystem( GV_PAUSE_PAUSE | GV_PAUSE_MENU | GV_PAUSE_READERROR ) ;
//						GM_SdSet( SNG_PAUSEOFF ) ;
						
//						GM_ContinueCount ++ ;
						GM_StartAlertMode = ALERT_MODE_SNEAK ;	/* コンティニュー後は潜入モード */
						GM_DiazepamCount = 0 ;
						/* いくつかのリンク変数は強制セーブ */
						GCL_SaveLinkVar( &GM_StartAlertMode, sizeof( GM_StartAlertMode ) ) ;
						GCL_SaveLinkVar( &GM_PlayTime, sizeof( GM_PlayTime ) ) ;
						GCL_SaveLinkVar( &GM_SaveCount, sizeof( GM_SaveCount ) ) ;
						GCL_SaveLinkVar( &GM_ContinueCount, sizeof( GM_ContinueCount ) ) ;
						GCL_SaveLinkVar( &GM_ShootCount, sizeof( GM_ShootCount ) ) ;		
						GCL_SaveLinkVar( &GM_AlertCount, sizeof( GM_AlertCount ) ) ;
						GCL_SaveLinkVar( &GM_KillCount, sizeof( GM_KillCount ) ) ;
						GCL_SaveLinkVar( &GM_DamageCount, sizeof( GM_DamageCount ) ) ;
						GCL_SaveLinkVar( &GM_RationUseCount, sizeof( GM_RationUseCount ) ) ;
						GCL_SaveLinkVar( &GM_ClearingCount, sizeof( GM_ClearingCount ) ) ;
						GCL_SaveLinkVar( &GM_RedFindCount, sizeof( GM_RedFindCount ) ) ;
						GCL_SaveLinkVar( &GM_ClearCodeFlag, sizeof( GM_ClearCodeFlag ) ) ;
						GCL_SaveLinkVar( &GM_MecaKillCount, sizeof( GM_MecaKillCount ) ) ;
//						GM_CallCallbackProc( GM_CALLBACK_CONTINUE ) ;
						if ( GM_SaveArea != *( int * )GCL_GetSavedLinkVar( &GM_SaveArea ) ) {
							/* セーブしたエリアが今と異なる場合 */
							GM_LoadRestart( 0 );
						} else {
							/* 同じ */
							GM_LoadRequest = GM_REQ_RESET ;
						}
						GM_LoadRequest |= GM_REQ_RESTORE_VAR ;
					}
				}
			}
			else
			{
				if(work->proc_exit != 0)
				{
					GM_GameStatus &= ~STATE_PAUSE_DISABLE;
					GM_COM_PauseOff();

					SY_PRINTF1("Proc EXIT");
					GM_SdSet( SNG_STOP );	/* BGM停止 */
					GM_SdSet( SE_EXP_STOP );	/* 拡張ＳＥ停止 */
					GM_SdSet(SD_S_START01);
					GM_DiazepamCount = 0 ;
					GM_ExecProc(work->proc_exit, NULL);
				}
			}
		}
		return;
	}

	/* ポーズ状態を監視 */
	if(work->flag & VRPAU2_WATCH_PAUSE)
	{
		if(    (GV_PauseLevelNoXMB == GV_PAUSE_PAUSE)   //BP_PAUSE - exclude XMB from pause check
			&& !(MSN_2DSTATUS & MSN_2DSTAT_WINDOW_PAUSE)
			&& !(GV_PadData[0].flag & GV_PAD_RELEASE))
		{
			/* ポーズ中 */
			if(!(work->flag & VRPAU2_SHOW))
			{
				/* ポーズ解除をできなくする */
				GM_GameStatus |= STATE_PAUSE_DISABLE;

				/* スタートボタン監視開始 */
				work->flag |= VRPAU2_WATCH_STA;

				work->flag |= VRPAU2_SHOW;
				GM_SdSet(SD_S_WIN01);
				LOM_SetCurMode(&work->layoutman, LOM_MODE_01, 1, 1, 1);
				work->count = 0;
				work->flag |= VRPAU2_INPUT_NG;
			}
		}
		else
		{
			/* ポーズ解除中 */
			if(work->flag & VRPAU2_SHOW)
			{
				work->flag &= ~VRPAU2_SHOW;
				GM_SdSet(SD_S_V_CANS02);
				LOM_SetCurMode(&work->layoutman, LOM_MODE_00, 1, 1, 1);
				work->count = 0;
				work->flag &= ~VRPAU2_SELECTED;
			}
		}
	}

   if( (BP_IsPAL()==TRUE && work->count == 16) ||
      (BP_IsPAL()!=TRUE && work->count == 20) )
	{
		work->flag &= ~VRPAU2_INPUT_NG;
	}
	if(work->count > 1000)
		work->count = 1000;

	/* スタートボタン監視 */
	if(work->flag & VRPAU2_WATCH_STA)
	{
		if(!(GV_PadDataDirect[0].status & PAD_STA))
			work->flag &= ~VRPAU2_WATCH_STA;
	}
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

#ifdef KP_XBOX
	GM_ReleasePadDisconnect();
#endif
}

/*******************************************************************************
 * 初期化処理

chara ＶＲポーズ２[NewVRPause2_Scn] $s:名前 \
	// 以下オプション
	-restart $p:ＲＥＳＴＡＲＴプロック \
	-exit    $p:ＥＸＩＴプロック
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	WORK  *work,			/* ワーク */
	int   name,				/* シナリオ名 */
	int   where,			/* マップＩＤ */
	int   proc_restart,		/* ＲＥＳＴＡＲＴプロック */
	int   proc_exit,		/* ＥＸＩＴプロック */
	short flag)				/* フラグ */
{
	SY_PRINTF3("GetResources\n");

	work->name         = name;
	work->proc_restart = proc_restart;
	work->proc_exit    = proc_exit;
	work->flag         = flag;
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

	work->flag |= VRPAU2_INIT;

#ifdef KP_XBOX
	GM_CreatePadDisconnect();
#endif
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
void *NewVRPause2(
	int proc_restart,	/* ＲＥＳＴＡＲＴプロック */
	int proc_exit)		/* ＥＸＩＴプロック */
{
	WORK *work;

	SY_PRINTF3("NewVRPause2\n");
	OPERATOR();

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
		GV_SetActorSignalFunc(work, ReceiveSignal);

		/* ワークの初期化 */
		if(!GetResources(work, 0, GM_CurrentMap, proc_restart, proc_exit, 0))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}

void *NewVRPause2_Scn(
	int name,	/* シナリオ名 */
	int where)	/* マップＩＤ */
{
	WORK *work;

	SY_PRINTF3("NewVRPause_Scn2\n");
	OPERATOR();

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
		if(!GetResources(work, name, where, 0, 0, VRPAU2_WATCH_PAUSE))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}
