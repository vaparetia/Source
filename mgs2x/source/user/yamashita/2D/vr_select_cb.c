//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * vr_select - vr_select_cb.c
 * ＶＲセレクト
 * 2002/06/03 S.Yamashita
 * $Id: vr_select_cb.c,v 1.2 2002/12/12 14:25:40 takaki Exp $
 */

/*******************************************************************************
 * include
 */

#include "vr_select.h"

/*******************************************************************************
 * public
 */
/******************************************************************************
 * カーソル位置の設定
 */
void VRSEL_L2_SetCursorPos(
	VRSEL_WORK *work,	/* ワーク */
	int        stages)
{
	switch(work->level)
	{
#ifndef KP_WINDOWS
	case 10: LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_CONTROL, 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_LEV_CONTROL, VRSEL_ICO_R2, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
#endif
	case  0: if(stages > 0) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_01, 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_LEV_01, VRSEL_ICO_R2, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break; }
	case  1: if(stages > 1) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_02, 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_LEV_02, VRSEL_ICO_R2, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break; }
	case  2: if(stages > 2) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_03, 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_LEV_03, VRSEL_ICO_R2, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break; }
	case  3: if(stages > 3) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_04, 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_LEV_04, VRSEL_ICO_R2, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break; }
	case  4: if(stages > 4) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_05, 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_LEV_05, VRSEL_ICO_R2, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break; }
	case  5: if(stages > 5) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_06, 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_LEV_06, VRSEL_ICO_R2, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break; }
	case  6: if(stages > 6) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_07, 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_LEV_07, VRSEL_ICO_R2, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break; }
	case  7: if(stages > 7) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_08, 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_LEV_08, VRSEL_ICO_R2, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break; }
	case  8: if(stages > 8) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_09, 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_LEV_09, VRSEL_ICO_R2, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break; }
	case  9: if(stages > 9) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_10, 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_LEV_10, VRSEL_ICO_R2, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break; }
	default:
		ASSERT(0);
		break;
	}
}

/*******************************************************************************
 * レイアウトマネージャ１のコールバック
 */
int VRSEL_LOMCallback_1(	/* 1: 成功:処理を続行 */
							/* 2: 成功:処理を中断 */
							/* 0: 失敗 */
	void *pWork,	/* ワーク */
	int  param)		/* パラメータ */
{
	VRSEL_WORK *work = (VRSEL_WORK *)pWork;

	SY_PRINTF3("LOMCallback_1\n");
	SY_PRINTF1("Callback: %d\n", param);

	switch(param)
	{
	case VRSEL_CB_PLA_RAIDEN_UP    : if(VRSEL_Pla_MoveUp  (work, 0) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_PLA_RAIDEN_DOWN  : if(VRSEL_Pla_MoveDown(work, 0) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_PLA_NINJA_UP     : if(VRSEL_Pla_MoveUp  (work, 1) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_PLA_NINJA_DOWN   : if(VRSEL_Pla_MoveDown(work, 1) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_PLA_X_RAIDEN_UP  : if(VRSEL_Pla_MoveUp  (work, 2) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_PLA_X_RAIDEN_DOWN: if(VRSEL_Pla_MoveDown(work, 2) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_PLA_SNAKE_UP     : if(VRSEL_Pla_MoveUp  (work, 3) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_PLA_SNAKE_DOWN   : if(VRSEL_Pla_MoveDown(work, 3) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_PLA_PLISKIN_UP   : if(VRSEL_Pla_MoveUp  (work, 4) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_PLA_PLISKIN_DOWN : if(VRSEL_Pla_MoveDown(work, 4) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_PLA_TUXEDO_UP    : if(VRSEL_Pla_MoveUp  (work, 5) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_PLA_TUXEDO_DOWN  : if(VRSEL_Pla_MoveDown(work, 5) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_PLA_PREVIOUS_UP  : if(VRSEL_Pla_MoveUp  (work, 6) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_PLA_PREVIOUS_DOWN: if(VRSEL_Pla_MoveDown(work, 6) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_PLA_SAVE_UP      : if(VRSEL_Pla_MoveUp  (work, 7) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_PLA_SAVE_DOWN    : if(VRSEL_Pla_MoveDown(work, 7) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_PLA_EXIT_UP      : if(VRSEL_Pla_MoveUp  (work, 8) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_PLA_EXIT_DOWN    : if(VRSEL_Pla_MoveDown(work, 8) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;

	case VRSEL_CB_PLA_RAIDEN_OK    : if(VRSEL_Pla_Select_1(work, VRSEL_PLA_RAIDEN)   == 1) { VRSEL_InputNG(work); } break;
	case VRSEL_CB_PLA_NINJA_OK     : if(VRSEL_Pla_Select_1(work, VRSEL_PLA_NINJA)    == 1) { VRSEL_InputNG(work); } break;
	case VRSEL_CB_PLA_X_RAIDEN_OK  : if(VRSEL_Pla_Select_1(work, VRSEL_PLA_X_RAIDEN) == 1) { VRSEL_InputNG(work); } break;
	case VRSEL_CB_PLA_SNAKE_OK     : if(VRSEL_Pla_Select_1(work, VRSEL_PLA_SNAKE)    == 1) { VRSEL_InputNG(work); } break;
	case VRSEL_CB_PLA_PLISKIN_OK   : if(VRSEL_Pla_Select_1(work, VRSEL_PLA_PLISKIN)  == 1) { VRSEL_InputNG(work); } break;
	case VRSEL_CB_PLA_TUXEDO_OK    : if(VRSEL_Pla_Select_1(work, VRSEL_PLA_TUXEDO)   == 1) { VRSEL_InputNG(work); } break;
	case VRSEL_CB_PLA_PREVIOUS_OK  : if(VRSEL_Pla_Select_1(work, VRSEL_PLA_PREVIOUS) == 1) { VRSEL_InputNG(work); } break;
	case VRSEL_CB_PLA_SAVE_OK      : if(VRSEL_Pla_Select_2(work)                     == 1) { VRSEL_InputNG(work); } break;
	case VRSEL_CB_PLA_EXIT_OK      : if(VRSEL_Pla_Select_3(work)                     == 1) { VRSEL_InputNG(work); } break;

	case VRSEL_CB_PLA_CANCEL:
		if(VRSEL_Pla_Cancel(work) == 1)
		{
			VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL);
		}
		break;

	case VRSEL_CB_L1_OPENED:
		VRSEL_LOM_SetCurMode(work, &work->layoutman_1, LOM_MODE_02, 1, 1, 1);

		switch(work->player)
		{
		case VRSEL_PLA_NO_PLAYER:
		case VRSEL_PLA_RAIDEN   : LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_RAIDEN    , 0); LOM_SetIcoPos(&work->layoutman_1, OBJECT_1_TXT_RAIDEN    , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case VRSEL_PLA_NINJA    : LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_NINJA_1   , 0); LOM_SetIcoPos(&work->layoutman_1, OBJECT_1_TXT_NINJA_1   , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case VRSEL_PLA_X_RAIDEN : LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_X_RAIDEN  , 0); LOM_SetIcoPos(&work->layoutman_1, OBJECT_1_TXT_X_RAIDEN  , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case VRSEL_PLA_SNAKE    : LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_SNAKE     , 0); LOM_SetIcoPos(&work->layoutman_1, OBJECT_1_TXT_SNAKE     , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case VRSEL_PLA_PLISKIN  : LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_PLISKIN   , 0); LOM_SetIcoPos(&work->layoutman_1, OBJECT_1_TXT_PLISKIN   , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case VRSEL_PLA_TUXEDO   : LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_TUXEDO_1  , 0); LOM_SetIcoPos(&work->layoutman_1, OBJECT_1_TXT_TUXEDO_1  , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case VRSEL_PLA_PREVIOUS : LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_PREVIOUS_1, 0); LOM_SetIcoPos(&work->layoutman_1, OBJECT_1_TXT_PREVIOUS_1, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case VRSEL_SAVE         : LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_SAVE      , 0); LOM_SetIcoPos(&work->layoutman_1, OBJECT_1_TXT_SAVE      , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case VRSEL_EXIT         : LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_EXIT      , 0); LOM_SetIcoPos(&work->layoutman_1, OBJECT_1_TXT_EXIT      , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		default:
			ASSERT(0);
			break;
		}

		/* 入力可 */
		VRSEL_InputOK(work);
		break;

	case VRSEL_CB_L1_CLOSED:
		if(work->flag & VRSEL_FLAG_LOAD_PLAYER)
		{
			/* プレイヤーの常駐をロードするためにシナリオに返す */

			/* プロック呼び出し */
			GCL_ARGS args;
			int      buf[1];
			args.argv = buf;
			args.argc = 1;
			buf[0]    = work->player;
			ASSERT(work->proc_player);
			SY_PRINTF1("PROC CALLED\n");
			GM_ExecProc(work->proc_player, &args);
		}
		else if(work->player == VRSEL_SAVE)
		{
			/* セーブするためにシナリオに返す */

			/* プロック呼び出し */
			ASSERT(work->proc_save);
			SY_PRINTF1("PROC CALLED\n");
			GM_ExecProc(work->proc_save, NULL);
		}
		else if(work->player == VRSEL_EXIT)
		{
			/* タイトルステージをロードするためにシナリオに返す */

			/* プロック呼び出し */
			ASSERT(work->proc_title);
			SY_PRINTF1("PROC CALLED\n");
			GM_ExecProc(work->proc_title, NULL);
		}
		else
		{
			/* ミッションセレクトへ移行 */
			VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_01, 1, 1, 1);
			work->flag |= VRSEL_FLAG_LAYOUT_2;
			GM_SdSet(SD_S_LINEMOV1);
		}
		break;

	case VRSEL_CB_L1: VRSEL_Dbg_L1(work); break;
	case VRSEL_CB_R1: VRSEL_Dbg_R1(work); break;
	case VRSEL_CB_L2: VRSEL_Dbg_L2(work); break;
	case VRSEL_CB_R2: VRSEL_Dbg_R2(work); break;
	case VRSEL_CB_L3: VRSEL_Dbg_L3(work); break;
	case VRSEL_CB_R3: VRSEL_Dbg_R3(work); break;

	default:
		ASSERT(0);
		break;
	}
	return 1;
}

/*******************************************************************************
 * レイアウトマネージャ２のコールバック
 */
int VRSEL_LOMCallback_2(	/* 1: 成功:処理を続行 */
							/* 2: 成功:処理を中断 */
							/* 0: 失敗 */
	void *pWork,	/* ワーク */
	int  param)		/* パラメータ */
{
	VRSEL_WORK *work = (VRSEL_WORK *)pWork;

	SY_PRINTF3("LOMCallback_2\n");
	SY_PRINTF1("Callback: %d\n", param);

	switch(param)
	{
	case VRSEL_CB_MIS_VR_UP   : if(VRSEL_Mis_Move_Up  (work, 0) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MIS_VR_DOWN : if(VRSEL_Mis_Move_Down(work, 0) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MIS_ALT_UP  : if(VRSEL_Mis_Move_Up  (work, 1) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MIS_ALT_DOWN: if(VRSEL_Mis_Move_Down(work, 1) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MIS_VR_OK   : if(VRSEL_Mis_Select(work, MSN_MISSION_VR         ) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_MIS_ALT_OK  : if(VRSEL_Mis_Select(work, MSN_MISSION_ALTERNATIVE) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_MIS_CANCEL  : if(VRSEL_Mis_Cancel(work) == 1) { VRSEL_InputNG(work); } break;

	case VRSEL_CB_MOD_SNEAKING_UP           : if(VRSEL_Mod_MoveUp_Vr  (work, 0) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MOD_SNEAKING_DOWN         : if(VRSEL_Mod_MoveDown_Vr(work, 0) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MOD_WEAPON_UP             : if(VRSEL_Mod_MoveUp_Vr  (work, 1) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MOD_WEAPON_DOWN           : if(VRSEL_Mod_MoveDown_Vr(work, 1) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MOD_FIRST_PERSON_VIEW_UP  : if(VRSEL_Mod_MoveUp_Vr  (work, 2) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MOD_FIRST_PERSON_VIEW_DOWN: if(VRSEL_Mod_MoveDown_Vr(work, 2) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MOD_VARIETY_UP            : if(VRSEL_Mod_MoveUp_Vr  (work, 3) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MOD_VARIETY_DOWN          : if(VRSEL_Mod_MoveDown_Vr(work, 3) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MOD_STREAKING_UP          : if(VRSEL_Mod_MoveUp_Vr  (work, 4) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MOD_STREAKING_DOWN        : if(VRSEL_Mod_MoveDown_Vr(work, 4) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MOD_SNEAKING_OK           : if(VRSEL_Mod_Select(work, MSN_MODE_SNEAKING         ) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_MOD_WEAPON_OK             : if(VRSEL_Mod_Select(work, MSN_MODE_WEAPON           ) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_MOD_FIRST_PERSON_VIEW_OK  : if(VRSEL_Mod_Select(work, MSN_MODE_FIRST_PERSON_VIEW) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_MOD_VARIETY_OK            : if(VRSEL_Mod_Select(work, MSN_MODE_VARIETY          ) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_MOD_STREAKING_OK          : if(VRSEL_Mod_Select(work, MSN_MODE_STREAKING        ) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_MOD_VR_CANCEL             : if(VRSEL_Mod_Cancel_1(work) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;

	case VRSEL_CB_MOD_BOMB_DISPOSAL_UP      : if(VRSEL_Mod_MoveUp_Alt  (work, 0) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MOD_BOMB_DISPOSAL_DOWN    : if(VRSEL_Mod_MoveDown_Alt(work, 0) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MOD_ELIMINATE_UP          : if(VRSEL_Mod_MoveUp_Alt  (work, 1) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MOD_ELIMINATE_DOWN        : if(VRSEL_Mod_MoveDown_Alt(work, 1) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MOD_HOLD_UP_UP            : if(VRSEL_Mod_MoveUp_Alt  (work, 2) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MOD_HOLD_UP_DOWN          : if(VRSEL_Mod_MoveDown_Alt(work, 2) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MOD_PHOTOGRAPH_UP         : if(VRSEL_Mod_MoveUp_Alt  (work, 3) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MOD_PHOTOGRAPH_DOWN       : if(VRSEL_Mod_MoveDown_Alt(work, 3) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_MOD_BOMB_DISPOSAL_OK      : if(VRSEL_Mod_Select(work, MSN_MODE_BOMB_DISPOSAL) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_MOD_ELIMINATE_OK          : if(VRSEL_Mod_Select(work, MSN_MODE_ELIMINATE    ) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_MOD_HOLD_UP_OK            : if(VRSEL_Mod_Select(work, MSN_MODE_HOLD_UP      ) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_MOD_PHOTOGRAPH_OK         : if(VRSEL_Mod_Select(work, MSN_MODE_PHOTOGRAPH   ) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_MOD_ALT_CANCEL            : if(VRSEL_Mod_Cancel_2(work) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;

	case VRSEL_CB_WEA_SNEAKING_UP       : if(VRSEL_Wea_MoveUp_Sne  (work, 0) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_WEA_SNEAKING_DOWN     : if(VRSEL_Wea_MoveDown_Sne(work, 0) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_WEA_ELIMINATE_ALL_UP  : if(VRSEL_Wea_MoveUp_Sne  (work, 1) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_WEA_ELIMINATE_ALL_DOWN: if(VRSEL_Wea_MoveDown_Sne(work, 1) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_WEA_SNEAKING_OK       : if(VRSEL_Wea_Select(work, MSN_WEAPON_SNEAKING     ) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_WEA_ELIMINATE_ALL_OK  : if(VRSEL_Wea_Select(work, MSN_WEAPON_ELIMINATE_ALL) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_WEA_SNE_CANCEL        : if(VRSEL_Wea_Cancel_1(work) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;

	case VRSEL_CB_WEA_HANDGUN_UP        : if(VRSEL_Wea_MoveUp_Wea  (work, 0) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_WEA_HANDGUN_DOWN      : if(VRSEL_Wea_MoveDown_Wea(work, 0) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_WEA_ASSAULT_RIFLE_UP  : if(VRSEL_Wea_MoveUp_Wea  (work, 1) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_WEA_ASSAULT_RIFLE_DOWN: if(VRSEL_Wea_MoveDown_Wea(work, 1) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_WEA_C4_CLAYMORE_UP    : if(VRSEL_Wea_MoveUp_Wea  (work, 2) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_WEA_C4_CLAYMORE_DOWN  : if(VRSEL_Wea_MoveDown_Wea(work, 2) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_WEA_GRENADE_UP        : if(VRSEL_Wea_MoveUp_Wea  (work, 3) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_WEA_GRENADE_DOWN      : if(VRSEL_Wea_MoveDown_Wea(work, 3) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_WEA_PSG1_UP           : if(VRSEL_Wea_MoveUp_Wea  (work, 4) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_WEA_PSG1_DOWN         : if(VRSEL_Wea_MoveDown_Wea(work, 4) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_WEA_STINGER_UP        : if(VRSEL_Wea_MoveUp_Wea  (work, 5) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_WEA_STINGER_DOWN      : if(VRSEL_Wea_MoveDown_Wea(work, 5) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_WEA_NIKITA_UP         : if(VRSEL_Wea_MoveUp_Wea  (work, 6) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_WEA_NIKITA_DOWN       : if(VRSEL_Wea_MoveDown_Wea(work, 6) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_WEA_HF_BLADE_UP       : if(VRSEL_Wea_MoveUp_Wea  (work, 7) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_WEA_HF_BLADE_DOWN     : if(VRSEL_Wea_MoveDown_Wea(work, 7) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_WEA_HANDGUN_OK        : if(VRSEL_Wea_Select(work, MSN_WEAPON_HANDGUN      ) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_WEA_ASSAULT_RIFLE_OK  : if(VRSEL_Wea_Select(work, MSN_WEAPON_ASSAULT_RIFLE) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_WEA_C4_CLAYMORE_OK    : if(VRSEL_Wea_Select(work, MSN_WEAPON_C4_CLAYMORE  ) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_WEA_GRENADE_OK        : if(VRSEL_Wea_Select(work, MSN_WEAPON_GRENADE      ) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_WEA_PSG1_OK           : if(VRSEL_Wea_Select(work, MSN_WEAPON_PSG1         ) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_WEA_STINGER_OK        : if(VRSEL_Wea_Select(work, MSN_WEAPON_STINGER      ) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_WEA_NIKITA_OK         : if(VRSEL_Wea_Select(work, MSN_WEAPON_NIKITA       ) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_WEA_HF_BLADE_OK       : if(VRSEL_Wea_Select(work, MSN_WEAPON_HF_BLADE     ) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;
	case VRSEL_CB_WEA_WEA_CANCEL        : if(VRSEL_Wea_Cancel_2(work) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;

	case VRSEL_CB_LEV_01_UP       : if(VRSEL_Lev_MoveUp  (work,  0) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_01_DOWN     : if(VRSEL_Lev_MoveDown(work,  0) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_02_UP       : if(VRSEL_Lev_MoveUp  (work,  1) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_02_DOWN     : if(VRSEL_Lev_MoveDown(work,  1) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_03_UP       : if(VRSEL_Lev_MoveUp  (work,  2) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_03_DOWN     : if(VRSEL_Lev_MoveDown(work,  2) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_04_UP       : if(VRSEL_Lev_MoveUp  (work,  3) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_04_DOWN     : if(VRSEL_Lev_MoveDown(work,  3) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_05_UP       : if(VRSEL_Lev_MoveUp  (work,  4) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_05_DOWN     : if(VRSEL_Lev_MoveDown(work,  4) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_06_UP       : if(VRSEL_Lev_MoveUp  (work,  5) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_06_DOWN     : if(VRSEL_Lev_MoveDown(work,  5) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_07_UP       : if(VRSEL_Lev_MoveUp  (work,  6) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_07_DOWN     : if(VRSEL_Lev_MoveDown(work,  6) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_08_UP       : if(VRSEL_Lev_MoveUp  (work,  7) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_08_DOWN     : if(VRSEL_Lev_MoveDown(work,  7) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_09_UP       : if(VRSEL_Lev_MoveUp  (work,  8) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_09_DOWN     : if(VRSEL_Lev_MoveDown(work,  8) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_10_UP       : if(VRSEL_Lev_MoveUp  (work,  9) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_10_DOWN     : if(VRSEL_Lev_MoveDown(work,  9) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_CONTROL_UP  : if(VRSEL_Lev_MoveUp  (work, 10) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_CONTROL_DOWN: if(VRSEL_Lev_MoveDown(work, 10) == 1) { VRSEL_InputStop(work, VRSEL_INPUT_INTERVAL); } break;
	case VRSEL_CB_LEV_01_OK       : if(VRSEL_Lev_Select_1(work,  0) == 1) { VRSEL_InputNG(work); } break;
	case VRSEL_CB_LEV_02_OK       : if(VRSEL_Lev_Select_1(work,  1) == 1) { VRSEL_InputNG(work); } break;
	case VRSEL_CB_LEV_03_OK       : if(VRSEL_Lev_Select_1(work,  2) == 1) { VRSEL_InputNG(work); } break;
	case VRSEL_CB_LEV_04_OK       : if(VRSEL_Lev_Select_1(work,  3) == 1) { VRSEL_InputNG(work); } break;
	case VRSEL_CB_LEV_05_OK       : if(VRSEL_Lev_Select_1(work,  4) == 1) { VRSEL_InputNG(work); } break;
	case VRSEL_CB_LEV_06_OK       : if(VRSEL_Lev_Select_1(work,  5) == 1) { VRSEL_InputNG(work); } break;
	case VRSEL_CB_LEV_07_OK       : if(VRSEL_Lev_Select_1(work,  6) == 1) { VRSEL_InputNG(work); } break;
	case VRSEL_CB_LEV_08_OK       : if(VRSEL_Lev_Select_1(work,  7) == 1) { VRSEL_InputNG(work); } break;
	case VRSEL_CB_LEV_09_OK       : if(VRSEL_Lev_Select_1(work,  8) == 1) { VRSEL_InputNG(work); } break;
	case VRSEL_CB_LEV_10_OK       : if(VRSEL_Lev_Select_1(work,  9) == 1) { VRSEL_InputNG(work); } break;
	case VRSEL_CB_LEV_CONTROL_OK  : if(VRSEL_Lev_Select_2(work)     == 1) { VRSEL_InputNG(work); } break;
	case VRSEL_CB_LEV_CLEAR_CODE  : if(VRSEL_Lev_Select_3(work)     == 1) { VRSEL_InputNG(work); } break;
	case VRSEL_CB_LEV_CANCEL      : if(VRSEL_Lev_Cancel(work) == 1) { VRSEL_InputStop2(work, VRSEL_JUSTINCASE); } break;

	case VRSEL_CB_L2_OPENED_1:
		/* ミッション選択に移行 */
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_21, 1, 1, 1); break;
		case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_21, 1, 1, 1); break;
		case VRSEL_PLA_X_RAIDEN: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_24, 1, 1, 1); break;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_21, 1, 1, 1); break;
		case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_21, 1, 1, 1); break;
		case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_21, 1, 1, 1); break;
		case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_21, 1, 1, 1); break;
		default:
			ASSERT(0);
			break;
		}
		/* カーソル位置の設定 */
		switch(work->missions)
		{
		case MSN_MISSION_VR         : LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MIS_VR         , 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MIS_VR         , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case MSN_MISSION_ALTERNATIVE: LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MIS_ALTERNATIVE, 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MIS_ALTERNATIVE, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		default:
			ASSERT(0);
			break;
		}

		/* 入力可 */
		VRSEL_InputOK(work);
		break;

	case VRSEL_CB_L2_OPENED_2:
		/* ミッション選択に移行・カーソル位置の設定 */
		if(work->missions == MSN_MISSION_VR)
		{
			if(work->mode == MSN_MODE_SNEAKING)
			{
				if(work->weapon == MSN_WEAPON_SNEAKING)
				{
					VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_86, 1, 1, 1);
					VRSEL_L2_SetCursorPos(work, 10);
				}
				else if(work->weapon == MSN_WEAPON_ELIMINATE_ALL)
				{
					VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_88, 1, 1, 1);
					VRSEL_L2_SetCursorPos(work, 10);
				}
				else
					ASSERT(0);
			}
			else if(work->mode == MSN_MODE_WEAPON)
			{
				if(work->weapon == MSN_WEAPON_HANDGUN)
				{
					VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_90, 1, 1, 1);
					VRSEL_L2_SetCursorPos(work, 5);
				}
				else if(work->weapon == MSN_WEAPON_ASSAULT_RIFLE)
				{
					VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_92, 1, 1, 1);
					VRSEL_L2_SetCursorPos(work, 5);
				}
				else if(work->weapon == MSN_WEAPON_C4_CLAYMORE)
				{
					VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_94, 1, 1, 1);
					VRSEL_L2_SetCursorPos(work, 5);
				}
				else if(work->weapon == MSN_WEAPON_GRENADE)
				{
					VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_96, 1, 1, 1);
					VRSEL_L2_SetCursorPos(work, 5);
				}
				else if(work->weapon == MSN_WEAPON_PSG1)
				{
					VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_98, 1, 1, 1);
					VRSEL_L2_SetCursorPos(work, 5);
				}
				else if(work->weapon == MSN_WEAPON_STINGER)
				{
					VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_100, 1, 1, 1);
					VRSEL_L2_SetCursorPos(work, 5);
				}
				else if(work->weapon == MSN_WEAPON_NIKITA)
				{
					VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_102, 1, 1, 1);
					VRSEL_L2_SetCursorPos(work, 5);
				}
				else if(work->weapon == MSN_WEAPON_HF_BLADE)
				{
					VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_104, 1, 1, 1);
					VRSEL_L2_SetCursorPos(work, 5);
				}
				else
					ASSERT(0);
			}
			else if(work->mode == MSN_MODE_FIRST_PERSON_VIEW)
			{
				VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_64, 1, 1, 1);
				VRSEL_L2_SetCursorPos(work, 5);
			}
			else if(work->mode == MSN_MODE_VARIETY)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_66, 1, 1, 1); VRSEL_L2_SetCursorPos(work, 7); break;
				case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_68, 1, 1, 1); VRSEL_L2_SetCursorPos(work, 1); break;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_66, 1, 1, 1); VRSEL_L2_SetCursorPos(work, 7); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_70, 1, 1, 1); VRSEL_L2_SetCursorPos(work, 2); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_70, 1, 1, 1); VRSEL_L2_SetCursorPos(work, 2); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_72, 1, 1, 1); VRSEL_L2_SetCursorPos(work, 3); break;
				default:
					ASSERT(0);
					break;
				}
			}
			else if(work->mode == MSN_MODE_STREAKING)
			{
				VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_74, 1, 1, 1);
				VRSEL_L2_SetCursorPos(work, 1);
			}
			else
				ASSERT(0);
		}
		else if(work->missions == MSN_MISSION_ALTERNATIVE)
		{
			if(work->mode == MSN_MODE_BOMB_DISPOSAL)
			{
				VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_76, 1, 1, 1);
				VRSEL_L2_SetCursorPos(work, 5);
			}
			else if(work->mode == MSN_MODE_ELIMINATE)
			{
				VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_78, 1, 1, 1);
				VRSEL_L2_SetCursorPos(work, 10);
			}
			else if(work->mode == MSN_MODE_HOLD_UP)
			{
				VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_80, 1, 1, 1);
				VRSEL_L2_SetCursorPos(work, 10);
			}
			else if(work->mode == MSN_MODE_PHOTOGRAPH)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_82, 1, 1, 1); VRSEL_L2_SetCursorPos(work, 6); break;
				case VRSEL_PLA_SNAKE : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_84, 1, 1, 1); VRSEL_L2_SetCursorPos(work, 7); break;
				default:
					ASSERT(0);
					break;
				}
			}
			else
				ASSERT(0);
		}
		else
			ASSERT(0);

		/* 入力可 */
		VRSEL_InputOK(work);
		break;

	case VRSEL_CB_L2_FADEOUT:
		VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_20, 1, 1, 1);
		break;

	case VRSEL_CB_L2_CLOSED_1:
		/* プレイヤーセレクトへ移行 */
		VRSEL_LOM_SetCurMode(work, &work->layoutman_1, LOM_MODE_01, 1, 1, 1);
		work->flag &= ~VRSEL_FLAG_LAYOUT_2;
		GM_SdSet(SD_S_LINEMOV1);
		break;

	case VRSEL_CB_L2_CLOSED_2:
		/* ステージをロードするためにシナリオに返す */
		{
			/* プロック呼び出し */
			GCL_ARGS args;
			int      buf[5];
			args.argv = buf;
			args.argc = 5;
			buf[0]    = work->player;
			buf[1]    = work->missions;
			buf[2]    = work->mode;
			buf[3]    = work->weapon;
			buf[4]    = work->level;

			SY_PRINTF2(
				"player  : %d\n"
				"missions: %d\n"
				"mode    : %d\n"
				"weapon  : %d\n"
				"level   : %d\n",
				work->player, work->missions, work->mode, work->weapon, work->level);

			MSN_RETRY_COUNT    = 0;
			MSN_CONTINUE_COUNT = 0;
			MSN_2DSTATUS &= ~MSN_2DSTAT_QUICK_WINDOW;
			GM_DiazepamCount   = 0;

			ASSERT(work->proc_stage);
			SY_PRINTF1("PROC CALLED\n");
			GM_ExecProc(work->proc_stage, &args);
		}
		break;

	case VRSEL_CB_L1: VRSEL_Dbg_L1(work); break;
	case VRSEL_CB_R1: VRSEL_Dbg_R1(work); break;
	case VRSEL_CB_L2: VRSEL_Dbg_L2(work); break;
	case VRSEL_CB_R2: VRSEL_Dbg_R2(work); break;
	case VRSEL_CB_L3: VRSEL_Dbg_L3(work); break;
	case VRSEL_CB_R3: VRSEL_Dbg_R3(work); break;

	default:
		ASSERT(0);
		break;
	}
	return 1;
}
