//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * vr_select - vr_select_pla.c
 * ＶＲセレクト
 * 2002/06/03 S.Yamashita
 * $Id: vr_select_pla.c,v 1.1.1.3 2002/11/19 11:51:48 Yoshizawa1 Exp $
 */

/*******************************************************************************
 * include
 */

#include "vr_select.h"

/*******************************************************************************
 * public
 */
/*******************************************************************************
 * プレイ可能か判定
 */
int VRSEL_IsRaidenPlayable(
	VRSEL_WORK *work)	/* ワーク */
{
	int ret;

	ret = VRSEL_PlayerClearLevel(work, VRSEL_PLA_RAIDEN, 1);

	if((0 <= ret) && (ret <= 4))
		return 1;
	else
		return 0;
}
int VRSEL_IsNinjaPlayable(
	VRSEL_WORK *work)	/* ワーク */
{
	int ret;

	ret = VRSEL_PlayerClearLevel(work, VRSEL_PLA_NINJA, 1);

	if((0 <= ret) && (ret <= 4))
		return 1;
	else
		return 0;
}
int VRSEL_IsXRaidenPlayable(
	VRSEL_WORK *work)	/* ワーク */
{
	int ret;

	ret = VRSEL_PlayerClearLevel(work, VRSEL_PLA_X_RAIDEN, 1);

	if((0 <= ret) && (ret <= 4))
		return 1;
	else
		return 0;
}
int VRSEL_IsSnakePlayable(
	VRSEL_WORK *work)	/* ワーク */
{
	int ret;

	ret = VRSEL_PlayerClearLevel(work, VRSEL_PLA_SNAKE, 1);

	if((0 <= ret) && (ret <= 4))
		return 1;
	else
		return 0;
}
int VRSEL_IsPliskinPlayable(
	VRSEL_WORK *work)	/* ワーク */
{
	int ret;

	ret = VRSEL_PlayerClearLevel(work, VRSEL_PLA_PLISKIN, 1);

	if((0 <= ret) && (ret <= 4))
		return 1;
	else
		return 0;
}
int VRSEL_IsTuxedoPlayable(
	VRSEL_WORK *work)	/* ワーク */
{
	int ret;

	ret = VRSEL_PlayerClearLevel(work, VRSEL_PLA_TUXEDO, 1);

	if((0 <= ret) && (ret <= 4))
		return 1;
	else
		return 0;
}
int VRSEL_IsPreviousPlayable(
	VRSEL_WORK *work)	/* ワーク */
{
	int ret;

	ret = VRSEL_PlayerClearLevel(work, VRSEL_PLA_PREVIOUS, 1);

	if((0 <= ret) && (ret <= 4))
		return 1;
	else
		return 0;
}
int VRSEL_IsSavePlayable(
	VRSEL_WORK *work)	/* ワーク */
{
	return 1;
}
int VRSEL_IsExitPlayable(
	VRSEL_WORK *work)	/* ワーク */
{
	return 1;
}

/*******************************************************************************
 * カーソルを上へ移動
 */
int VRSEL_Pla_MoveUp(	/* 1: 処理を行った */
						/* 0: 処理を行わなかった */
	VRSEL_WORK *work,	/* ワーク */
	int        pos)		/* 現在位置 */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	/* 一つ上の項目へ移動 */
	if     ((pos > 7) && (VRSEL_IsSavePlayable(work)    )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_SAVE      , 0); }
	else if((pos > 6) && (VRSEL_IsPreviousPlayable(work))) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_PREVIOUS_1, 0); }
	else if((pos > 5) && (VRSEL_IsTuxedoPlayable(work)  )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_TUXEDO_1  , 0); }
	else if((pos > 4) && (VRSEL_IsPliskinPlayable(work) )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_PLISKIN   , 0); }
	else if((pos > 3) && (VRSEL_IsSnakePlayable(work)   )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_SNAKE     , 0); }
	else if((pos > 2) && (VRSEL_IsXRaidenPlayable(work) )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_X_RAIDEN  , 0); }
	else if((pos > 1) && (VRSEL_IsNinjaPlayable(work)   )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_NINJA_1   , 0); }
	else if((pos > 0) && (VRSEL_IsRaidenPlayable(work)  )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_RAIDEN    , 0); }
	else if((pos < 8) && (VRSEL_IsExitPlayable(work)    )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_EXIT      , 0); }
	else if((pos < 7) && (VRSEL_IsSavePlayable(work)    )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_SAVE      , 0); }
	else if((pos < 6) && (VRSEL_IsPreviousPlayable(work))) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_PREVIOUS_1, 0); }
	else if((pos < 5) && (VRSEL_IsTuxedoPlayable(work)  )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_TUXEDO_1  , 0); }
	else if((pos < 4) && (VRSEL_IsPliskinPlayable(work) )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_PLISKIN   , 0); }
	else if((pos < 3) && (VRSEL_IsSnakePlayable(work)   )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_SNAKE     , 0); }
	else if((pos < 2) && (VRSEL_IsXRaidenPlayable(work) )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_X_RAIDEN  , 0); }
	else if((pos < 1) && (VRSEL_IsNinjaPlayable(work)   )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_NINJA_1   , 0); }
	else return 0;
	work->missions = 0;
	work->mode     = 0;
	work->weapon   = 0;
	work->level    = 0;

	GM_SdSet(SD_S_CUR01);
 	return 1;
}

/*******************************************************************************
 * カーソルを下へ移動
 */
int VRSEL_Pla_MoveDown(
	VRSEL_WORK *work,	/* ワーク */
	int        pos)		/* 現在位置 */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	/* 一つ下の項目へ移動 */
	if     ((pos < 1) && (VRSEL_IsNinjaPlayable(work)   )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_NINJA_1   , 0); }
	else if((pos < 2) && (VRSEL_IsXRaidenPlayable(work) )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_X_RAIDEN  , 0); }
	else if((pos < 3) && (VRSEL_IsSnakePlayable(work)   )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_SNAKE     , 0); }
	else if((pos < 4) && (VRSEL_IsPliskinPlayable(work) )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_PLISKIN   , 0); }
	else if((pos < 5) && (VRSEL_IsTuxedoPlayable(work)  )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_TUXEDO_1  , 0); }
	else if((pos < 6) && (VRSEL_IsPreviousPlayable(work))) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_PREVIOUS_1, 0); }
	else if((pos < 7) && (VRSEL_IsSavePlayable(work)    )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_SAVE      , 0); }
	else if((pos < 8) && (VRSEL_IsExitPlayable(work)    )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_EXIT      , 0); }
	else if((pos > 0) && (VRSEL_IsRaidenPlayable(work)  )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_RAIDEN    , 0); }
	else if((pos > 1) && (VRSEL_IsNinjaPlayable(work)   )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_NINJA_1   , 0); }
	else if((pos > 2) && (VRSEL_IsXRaidenPlayable(work) )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_X_RAIDEN  , 0); }
	else if((pos > 3) && (VRSEL_IsSnakePlayable(work)   )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_SNAKE     , 0); }
	else if((pos > 4) && (VRSEL_IsPliskinPlayable(work) )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_PLISKIN   , 0); }
	else if((pos > 5) && (VRSEL_IsTuxedoPlayable(work)  )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_TUXEDO_1  , 0); }
	else if((pos > 6) && (VRSEL_IsPreviousPlayable(work))) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_PREVIOUS_1, 0); }
	else if((pos > 7) && (VRSEL_IsSavePlayable(work)    )) { LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_SAVE      , 0); }
	else return 0;
	work->missions = 0;
	work->mode     = 0;
	work->weapon   = 0;
	work->level    = 0;

	GM_SdSet(SD_S_CUR01);
	return 1;
}

/*******************************************************************************
 * 項目を選択
 */
int VRSEL_Pla_Select_1(
	VRSEL_WORK *work,	/* ワーク */
	int        player)	/* プレイヤー */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	if     (player == VRSEL_PLA_RAIDEN  ) { if(VRSEL_IsRaidenPlayable(work)   == 0) ASSERT(0); }
	else if(player == VRSEL_PLA_NINJA   ) { if(VRSEL_IsNinjaPlayable(work)    == 0) ASSERT(0); }
	else if(player == VRSEL_PLA_X_RAIDEN) { if(VRSEL_IsXRaidenPlayable(work)  == 0) ASSERT(0); }
	else if(player == VRSEL_PLA_SNAKE   ) { if(VRSEL_IsSnakePlayable(work)    == 0) ASSERT(0); }
	else if(player == VRSEL_PLA_PLISKIN ) { if(VRSEL_IsPliskinPlayable(work)  == 0) ASSERT(0); }
	else if(player == VRSEL_PLA_TUXEDO  ) { if(VRSEL_IsTuxedoPlayable(work)   == 0) ASSERT(0); }
	else if(player == VRSEL_PLA_PREVIOUS) { if(VRSEL_IsPreviousPlayable(work) == 0) ASSERT(0); }
	else ASSERT(0);

	/* プレイヤーを設定 */
	if(work->player != player)
	{
		work->player = player;
		work->flag |= VRSEL_FLAG_LOAD_PLAYER;
	}

	/* レイアウトを閉じる */
	VRSEL_LOM_SetCurMode(work, &work->layoutman_1, LOM_MODE_03, 1, 1, 1);

	GM_SdSet(SD_S_WIN01);
	GM_SdSet(SD_S_WINCLS01);
	return 1;
}

int VRSEL_Pla_Select_2(
	VRSEL_WORK *work)	/* ワーク */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	if(VRSEL_IsSavePlayable(work) == 0)
		ASSERT(0);

	work->player = VRSEL_SAVE;

	/* レイアウトを閉じる */
	VRSEL_LOM_SetCurMode(work, &work->layoutman_1, LOM_MODE_03, 1, 1, 1);

	GM_SdSet(SD_S_WIN01);
	GM_SdSet(SD_S_WINCLS01);
	return 1;
}

int VRSEL_Pla_Select_3(
	VRSEL_WORK *work)	/* ワーク */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	if(VRSEL_IsExitPlayable(work) == 0)
		ASSERT(0);

	work->player = VRSEL_EXIT;

	/* レイアウトを閉じる */
	VRSEL_LOM_SetCurMode(work, &work->layoutman_1, LOM_MODE_03, 1, 1, 1);


	GM_SdSet(SD_S_START01);
	GM_SdSet(SD_S_WINCLS01);
	return 1;
}

/*******************************************************************************
 * 選択をキャンセル
 */
int VRSEL_Pla_Cancel(
	VRSEL_WORK *work)	/* ワーク */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	if(VRSEL_IsExitPlayable(work) == 0)
		return 0;

	if(LOM_GetCurObj(&work->layoutman_1) == OBJECT_1_TXT_EXIT)
		return 0;

	LOM_SetCurObj(&work->layoutman_1, OBJECT_1_TXT_EXIT, 0);
	work->missions = 0;
	work->mode     = 0;
	work->weapon   = 0;
	work->level    = 0;

	GM_SdSet(SD_S_CUR01);
	return 1;
}
