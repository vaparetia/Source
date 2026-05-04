//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * vr_select - vr_select_mis.c
 * ＶＲセレクト
 * 2002/06/03 S.Yamashita
 * $Id: vr_select_mis.c,v 1.1.1.3 2002/11/19 11:51:47 Yoshizawa1 Exp $
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
int VRSEL_IsVRPlayable(
	VRSEL_WORK *work)	/* ワーク */
{
	int ret;

	switch(work->player)
	{
	case VRSEL_PLA_RAIDEN  : break;
	case VRSEL_PLA_NINJA   : break;
	case VRSEL_PLA_X_RAIDEN: break;
	case VRSEL_PLA_SNAKE   : break;
	case VRSEL_PLA_PLISKIN : break;
	case VRSEL_PLA_TUXEDO  : break;
	case VRSEL_PLA_PREVIOUS: break;
	default:
		ASSERT(0);
		return 0;
	}

	ret = VRSEL_VRClearLevel(work, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}
int VRSEL_IsAlternativePlayable(
	VRSEL_WORK *work)	/* ワーク */
{
	int ret;

	switch(work->player)
	{
	case VRSEL_PLA_RAIDEN  : break;
	case VRSEL_PLA_NINJA   : break;
	case VRSEL_PLA_X_RAIDEN: return 0;
	case VRSEL_PLA_SNAKE   : break;
	case VRSEL_PLA_PLISKIN : break;
	case VRSEL_PLA_TUXEDO  : break;
	case VRSEL_PLA_PREVIOUS: break;
	default:
		ASSERT(0);
		return 0;
	}

	ret = VRSEL_AlternativeClearLevel(work, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}

/*******************************************************************************
 * カーソルを上へ移動
 */
int VRSEL_Mis_Move_Up(
	VRSEL_WORK *work,	/* ワーク */
	int        pos)		/* 現在位置 */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	/* 一つ上の項目へ移動 */
	if     ((pos > 0) && (VRSEL_IsVRPlayable(work)         )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MIS_VR         , 0); work->missions = MSN_MISSION_VR;          }
	else if((pos < 1) && (VRSEL_IsAlternativePlayable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MIS_ALTERNATIVE, 0); work->missions = MSN_MISSION_ALTERNATIVE; }
	else return 0;
	work->mode   = 0;
	work->weapon = 0;
	work->level  = 0;

	GM_SdSet(SD_S_CUR01);
	return 1;
}

/*******************************************************************************
 * カーソルを下へ移動
 */
int VRSEL_Mis_Move_Down(
	VRSEL_WORK *work,	/* ワーク */
	int        pos)		/* 現在位置 */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	/* 一つ下の項目へ移動 */
	if     ((pos < 1) && (VRSEL_IsAlternativePlayable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MIS_ALTERNATIVE, 0); work->missions = MSN_MISSION_ALTERNATIVE; }
	else if((pos > 0) && (VRSEL_IsVRPlayable(work)         )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MIS_VR         , 0); work->missions = MSN_MISSION_VR;          }
	else return 0;
	work->mode   = 0;
	work->weapon = 0;
	work->level  = 0;

	GM_SdSet(SD_S_CUR01);
	return 1;
}

/*******************************************************************************
 * 項目を選択
 */
int VRSEL_Mis_Select(
	VRSEL_WORK *work,		/* ワーク */
	int        missions)	/* ミッション */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	ASSERT(missions == work->missions);

	if(missions == MSN_MISSION_VR)
	{
		if(VRSEL_IsVRPlayable(work) == 0)
			ASSERT(0);

		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_26, 1, 1, 1); break;
		case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_31, 1, 1, 1); break;
		case VRSEL_PLA_X_RAIDEN: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_35, 1, 1, 1); work->mode = MSN_MODE_STREAKING; break;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_26, 1, 1, 1); break;
		case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_31, 1, 1, 1); break;
		case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_31, 1, 1, 1); break;
		case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_31, 1, 1, 1); break;
		default:
			ASSERT(0);
			break;
		}
		/* カーソル位置の設定 */
		switch(work->mode)
		{
		case MSN_MODE_SNEAKING         : LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_SNEAKING , 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MOD_SNEAKING , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case MSN_MODE_WEAPON           : LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_WEAPON   , 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MOD_WEAPON   , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case MSN_MODE_FIRST_PERSON_VIEW: LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_FSV_1    , 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MOD_FSV_1    , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case MSN_MODE_VARIETY          : LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_VARIETY  , 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MOD_VARIETY  , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case MSN_MODE_STREAKING        : LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_STREAKING, 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MOD_STREAKING, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		default:
			ASSERT(0);
			break;
		}
	}
	else if(missions == MSN_MISSION_ALTERNATIVE)
	{
		if(VRSEL_IsAlternativePlayable(work) == 0)
			ASSERT(0);

		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_37, 1, 1, 1); if(work->mode == 0) work->mode = MSN_MODE_BOMB_DISPOSAL; break;
		case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_42, 1, 1, 1); if(work->mode == 0) work->mode = MSN_MODE_BOMB_DISPOSAL; break;
		case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_37, 1, 1, 1); if(work->mode == 0) work->mode = MSN_MODE_BOMB_DISPOSAL; break;
		case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_45, 1, 1, 1); if(work->mode == 0) work->mode = MSN_MODE_BOMB_DISPOSAL; break;
		case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_45, 1, 1, 1); if(work->mode == 0) work->mode = MSN_MODE_BOMB_DISPOSAL; break;
		case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_45, 1, 1, 1); if(work->mode == 0) work->mode = MSN_MODE_BOMB_DISPOSAL; break;
		default:
			ASSERT(0);
			break;
		}
		/* カーソル位置の設定 */
		switch(work->mode)
		{
		case MSN_MODE_BOMB_DISPOSAL: LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_BOMB_D    , 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MOD_BOMB_D    , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case MSN_MODE_ELIMINATE    : LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_ELIMINATE , 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MOD_ELIMINATE , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case MSN_MODE_HOLD_UP      : LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_HOLD_UP   , 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MOD_HOLD_UP   , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case MSN_MODE_PHOTOGRAPH   : LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_PHOTOGRAPH, 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MOD_PHOTOGRAPH, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		default:
			ASSERT(0);
			break;
		}
	}
	else ASSERT(0);

	if(work->mode != MSN_MODE_STREAKING)
	{
		work->flag |= VRSEL_FLAG_REQUEST_MOVIE;
	}

	GM_SdSet(SD_S_WIN01);
	return 1;
}

/*******************************************************************************
 * 選択をキャンセル
 */
int VRSEL_Mis_Cancel(
	VRSEL_WORK *work)	/* ワーク */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	/* レイアウトを閉じる */
	VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_19, 1, 1, 1);

	GM_SdSet(SD_S_V_CANS02);
	GM_SdSet(SD_S_WINCLS01);
	return 1;
}
