//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * vr_select - vr_select_mod.c
 * ＶＲセレクト
 * 2002/06/03 S.Yamashita
 * $Id: vr_select_mod.c,v 1.2 2002/12/12 14:25:40 takaki Exp $
 */

/*******************************************************************************
 * include
 */

#include "vr_select.h"

/*******************************************************************************
 * extern
 */
extern void CancelMovieStream(void);
extern void RequestMovieStream(int top_pos, int width, int height, int x, int y, int repeat, int pad_cancel,
							   int disp_width, int disp_height);

/*******************************************************************************
 * public
 */
/*******************************************************************************
 * ムービー関係
 */
int VRSEL_SetMovie(
	VRSEL_WORK *work,			/* ワーク */
	int        movie_index)		/* ムービーインデックス */
{
	SPR_POS pos;

	work->flag &= ~VRSEL_FLAG_REQUEST_MOVIE;

	LOM_Spr_GetPosition(&work->layoutman_2, OBJECT_2_BOX_MOVIE, &pos, 1);
	pos.y = pos.y * 448 / 384;

	CancelMovieStream();
	RequestMovieStream(work->movie[movie_index], 176, 128, pos.x, pos.y, TRUE, 0, 176, 150);

	return 1;
}
int VRSEL_SetMovie2(
	VRSEL_WORK *work,			/* ワーク */
	int        movie_index)		/* ムービーインデックス */
{
	movie_index &= 0xfffffffe;

	switch(work->player)
	{
	case VRSEL_PLA_RAIDEN  : return VRSEL_SetMovie(work, movie_index);
	case VRSEL_PLA_NINJA   : return VRSEL_SetMovie(work, movie_index);
	case VRSEL_PLA_X_RAIDEN: return 0;
	case VRSEL_PLA_SNAKE   : return VRSEL_SetMovie(work, movie_index + 1);
	case VRSEL_PLA_PLISKIN : return VRSEL_SetMovie(work, movie_index + 1);
	case VRSEL_PLA_TUXEDO  : return VRSEL_SetMovie(work, movie_index + 1);
	case VRSEL_PLA_PREVIOUS: return VRSEL_SetMovie(work, movie_index + 1);
	default:
		ASSERT(0);
		return 0;
	}
}

/*******************************************************************************
 * プレイ可能か判定
 */
int VRSEL_IsSneakingPlayable(
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

	ret = VRSEL_SneakingClearLevel(work, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}
int VRSEL_IsWeaponPlayable(
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

	ret = VRSEL_WeaponClearLevel(work, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}
int VRSEL_IsFirstPersonViewPlayable(
	VRSEL_WORK *work)	/* ワーク */
{
	int ret;

	switch(work->player)
	{
	case VRSEL_PLA_RAIDEN  : break;
	case VRSEL_PLA_NINJA   : return 0;
	case VRSEL_PLA_X_RAIDEN: return 0;
	case VRSEL_PLA_SNAKE   : break;
	case VRSEL_PLA_PLISKIN : return 0;
	case VRSEL_PLA_TUXEDO  : return 0;
	case VRSEL_PLA_PREVIOUS: return 0;
	default:
		ASSERT(0);
		return 0;
	}

	ret = VRSEL_FPVClearLevel(work, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}
int VRSEL_IsVarietyPlayable(
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

	ret = VRSEL_VarietyClearLevel(work, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}
int VRSEL_IsStreakingPlayable(
	VRSEL_WORK *work)	/* ワーク */
{
	int ret;

	switch(work->player)
	{
	case VRSEL_PLA_RAIDEN  : return 0;
	case VRSEL_PLA_NINJA   : return 0;
	case VRSEL_PLA_X_RAIDEN: break;
	case VRSEL_PLA_SNAKE   : return 0;
	case VRSEL_PLA_PLISKIN : return 0;
	case VRSEL_PLA_TUXEDO  : return 0;
	case VRSEL_PLA_PREVIOUS: return 0;
	default:
		ASSERT(0);
		return 0;
	}

	ret = VRSEL_StreakingClearLevel(work, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}
int VRSEL_IsBombDisposalPlayable(
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

	ret = VRSEL_BombClearLevel(work, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}
int VRSEL_IsEliminatePlayable(
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

	ret = VRSEL_EliminateClearLevel(work, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}
int VRSEL_IsHoldUpPlayable(
	VRSEL_WORK *work)	/* ワーク */
{
	int ret;

	switch(work->player)
	{
	case VRSEL_PLA_RAIDEN  : break;
	case VRSEL_PLA_NINJA   : return 0;
	case VRSEL_PLA_X_RAIDEN: return 0;
	case VRSEL_PLA_SNAKE   : break;
	case VRSEL_PLA_PLISKIN : break;
	case VRSEL_PLA_TUXEDO  : break;
	case VRSEL_PLA_PREVIOUS: break;
	default:
		ASSERT(0);
		return 0;
	}

	ret = VRSEL_HoldupClearLevel(work, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}
int VRSEL_IsPhotographPlayable(
	VRSEL_WORK *work)	/* ワーク */
{
	int ret;

	switch(work->player)
	{
	case VRSEL_PLA_RAIDEN  : break;
	case VRSEL_PLA_NINJA   : return 0;
	case VRSEL_PLA_X_RAIDEN: return 0;
	case VRSEL_PLA_SNAKE   : break;
	case VRSEL_PLA_PLISKIN : return 0;
	case VRSEL_PLA_TUXEDO  : return 0;
	case VRSEL_PLA_PREVIOUS: return 0;
	default:
		ASSERT(0);
		return 0;
	}

	ret = VRSEL_PhotographClearLevel(work, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}

/*******************************************************************************
 * カーソルを上へ移動
 */
int VRSEL_Mod_MoveUp_Vr(
	VRSEL_WORK *work,	/* ワーク */
	int        pos)		/* 現在位置 */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	/* 一つ上の項目へ移動 */
	if     ((pos > 3) && (VRSEL_IsVarietyPlayable(work)        )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_VARIETY  , 0); work->mode = MSN_MODE_VARIETY;           VRSEL_SetMovie2(work, VRSEL_MOV_VARIETY_RAIDEN          ); }
	else if((pos > 2) && (VRSEL_IsFirstPersonViewPlayable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_FSV_1    , 0); work->mode = MSN_MODE_FIRST_PERSON_VIEW; VRSEL_SetMovie2(work, VRSEL_MOV_FIRST_PERSON_VIEW_RAIDEN); }
	else if((pos > 1) && (VRSEL_IsWeaponPlayable(work)         )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_WEAPON   , 0); work->mode = MSN_MODE_WEAPON;            VRSEL_SetMovie2(work, VRSEL_MOV_WEAPON_RAIDEN           ); }
	else if((pos > 0) && (VRSEL_IsSneakingPlayable(work)       )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_SNEAKING , 0); work->mode = MSN_MODE_SNEAKING;          VRSEL_SetMovie2(work, VRSEL_MOV_SNEAKING_RAIDEN         ); }
	else if((pos < 4) && (VRSEL_IsStreakingPlayable(work)      )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_STREAKING, 0); work->mode = MSN_MODE_STREAKING;         CancelMovieStream();                                       }
	else if((pos < 3) && (VRSEL_IsVarietyPlayable(work)        )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_VARIETY  , 0); work->mode = MSN_MODE_VARIETY;           VRSEL_SetMovie2(work, VRSEL_MOV_VARIETY_RAIDEN          ); }
	else if((pos < 2) && (VRSEL_IsFirstPersonViewPlayable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_FSV_1    , 0); work->mode = MSN_MODE_FIRST_PERSON_VIEW; VRSEL_SetMovie2(work, VRSEL_MOV_FIRST_PERSON_VIEW_RAIDEN); }
	else if((pos < 1) && (VRSEL_IsWeaponPlayable(work)         )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_WEAPON   , 0); work->mode = MSN_MODE_WEAPON;            VRSEL_SetMovie2(work, VRSEL_MOV_WEAPON_RAIDEN           ); }
	else return 0;
	work->weapon = 0;
	work->level  = 0;

	GM_SdSet(SD_S_CUR01);
	return 1;
}

/*******************************************************************************
 * カーソルを下へ移動
 */
int VRSEL_Mod_MoveDown_Vr(
	VRSEL_WORK *work,	/* ワーク */
	int        pos)		/* 現在位置 */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	/* 一つ下の項目へ移動 */
	if     ((pos < 1) && (VRSEL_IsWeaponPlayable(work)         )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_WEAPON   , 0); work->mode = MSN_MODE_WEAPON;            VRSEL_SetMovie2(work, VRSEL_MOV_WEAPON_RAIDEN           ); }
	else if((pos < 2) && (VRSEL_IsFirstPersonViewPlayable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_FSV_1    , 0); work->mode = MSN_MODE_FIRST_PERSON_VIEW; VRSEL_SetMovie2(work, VRSEL_MOV_FIRST_PERSON_VIEW_RAIDEN); }
	else if((pos < 3) && (VRSEL_IsVarietyPlayable(work)        )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_VARIETY  , 0); work->mode = MSN_MODE_VARIETY;           VRSEL_SetMovie2(work, VRSEL_MOV_VARIETY_RAIDEN          ); }
	else if((pos < 4) && (VRSEL_IsStreakingPlayable(work)      )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_STREAKING, 0); work->mode = MSN_MODE_STREAKING;         CancelMovieStream();                                       }
	else if((pos > 0) && (VRSEL_IsSneakingPlayable(work)       )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_SNEAKING , 0); work->mode = MSN_MODE_SNEAKING;          VRSEL_SetMovie2(work, VRSEL_MOV_SNEAKING_RAIDEN         ); }
	else if((pos > 1) && (VRSEL_IsWeaponPlayable(work)         )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_WEAPON   , 0); work->mode = MSN_MODE_WEAPON;            VRSEL_SetMovie2(work, VRSEL_MOV_WEAPON_RAIDEN           ); }
	else if((pos > 2) && (VRSEL_IsFirstPersonViewPlayable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_FSV_1    , 0); work->mode = MSN_MODE_FIRST_PERSON_VIEW; VRSEL_SetMovie2(work, VRSEL_MOV_FIRST_PERSON_VIEW_RAIDEN); }
	else if((pos > 3) && (VRSEL_IsVarietyPlayable(work)        )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_VARIETY  , 0); work->mode = MSN_MODE_VARIETY;           VRSEL_SetMovie2(work, VRSEL_MOV_VARIETY_RAIDEN          ); }
	else return 0;
	work->weapon = 0;
	work->level  = 0;

	GM_SdSet(SD_S_CUR01);
	return 1;
}

/*******************************************************************************
 * カーソルを上へ移動
 */
int VRSEL_Mod_MoveUp_Alt(
	VRSEL_WORK *work,	/* ワーク */
	int        pos)		/* 現在位置 */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	/* 一つ上の項目へ移動 */
	if     ((pos > 2) && (VRSEL_IsHoldUpPlayable(work)      )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_HOLD_UP   , 0); work->mode = MSN_MODE_HOLD_UP;       VRSEL_SetMovie2(work, VRSEL_MOV_HOLD_UP_RAIDEN      ); }
	else if((pos > 1) && (VRSEL_IsEliminatePlayable(work)   )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_ELIMINATE , 0); work->mode = MSN_MODE_ELIMINATE;     VRSEL_SetMovie2(work, VRSEL_MOV_ELIMINATE_RAIDEN    ); }
	else if((pos > 0) && (VRSEL_IsBombDisposalPlayable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_BOMB_D    , 0); work->mode = MSN_MODE_BOMB_DISPOSAL; VRSEL_SetMovie2(work, VRSEL_MOV_BOMB_DISPOSAL_RAIDEN); }
	else if((pos < 3) && (VRSEL_IsPhotographPlayable(work)  )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_PHOTOGRAPH, 0); work->mode = MSN_MODE_PHOTOGRAPH;    VRSEL_SetMovie2(work, VRSEL_MOV_PHOTOGRAPH_RAIDEN   ); }
	else if((pos < 2) && (VRSEL_IsHoldUpPlayable(work)      )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_HOLD_UP   , 0); work->mode = MSN_MODE_HOLD_UP;       VRSEL_SetMovie2(work, VRSEL_MOV_HOLD_UP_RAIDEN      ); }
	else if((pos < 1) && (VRSEL_IsEliminatePlayable(work)   )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_ELIMINATE , 0); work->mode = MSN_MODE_ELIMINATE;     VRSEL_SetMovie2(work, VRSEL_MOV_ELIMINATE_RAIDEN    ); }
	else return 0;
	work->level = 0;

	GM_SdSet(SD_S_CUR01);
	return 1;
}

/*******************************************************************************
 * カーソルを下へ移動
 */
int VRSEL_Mod_MoveDown_Alt(
	VRSEL_WORK *work,	/* ワーク */
	int        pos)		/* 現在位置 */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	/* 一つ下の項目へ移動 */
	if     ((pos < 1) && (VRSEL_IsEliminatePlayable(work)   )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_ELIMINATE , 0); work->mode = MSN_MODE_ELIMINATE;     VRSEL_SetMovie2(work, VRSEL_MOV_ELIMINATE_RAIDEN    ); }
	else if((pos < 2) && (VRSEL_IsHoldUpPlayable(work)      )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_HOLD_UP   , 0); work->mode = MSN_MODE_HOLD_UP;       VRSEL_SetMovie2(work, VRSEL_MOV_HOLD_UP_RAIDEN      ); }
	else if((pos < 3) && (VRSEL_IsPhotographPlayable(work)  )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_PHOTOGRAPH, 0); work->mode = MSN_MODE_PHOTOGRAPH;    VRSEL_SetMovie2(work, VRSEL_MOV_PHOTOGRAPH_RAIDEN   ); }
	else if((pos > 0) && (VRSEL_IsBombDisposalPlayable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_BOMB_D    , 0); work->mode = MSN_MODE_BOMB_DISPOSAL; VRSEL_SetMovie2(work, VRSEL_MOV_BOMB_DISPOSAL_RAIDEN); }
	else if((pos > 1) && (VRSEL_IsEliminatePlayable(work)   )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_ELIMINATE , 0); work->mode = MSN_MODE_ELIMINATE;     VRSEL_SetMovie2(work, VRSEL_MOV_ELIMINATE_RAIDEN    ); }
	else if((pos > 2) && (VRSEL_IsHoldUpPlayable(work)      )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_HOLD_UP   , 0); work->mode = MSN_MODE_HOLD_UP;       VRSEL_SetMovie2(work, VRSEL_MOV_HOLD_UP_RAIDEN      ); }
	else return 0;
	work->level = 0;

	GM_SdSet(SD_S_CUR01);
	return 1;
}

/*******************************************************************************
 * 項目を選択
 */
int VRSEL_Mod_Select(
	VRSEL_WORK *work,	/* ワーク */
	int        mode)	/* モード */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	ASSERT(mode == work->mode);

	if(mode == MSN_MODE_SNEAKING)
	{
		if(VRSEL_IsSneakingPlayable(work) == 0)
			ASSERT(0);

		/* 武器選択（ＳＮＥＡＫＩＮＧ）に移行 */
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_49, 1, 1, 1); break;
		case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_49, 1, 1, 1); break;
		case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_49, 1, 1, 1); break;
		case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_49, 1, 1, 1); break;
		case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_49, 1, 1, 1); break;
		case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_49, 1, 1, 1); break;
		default:
			ASSERT(0);
			break;
		}
		/* カーソル位置の設定 */
		switch(work->weapon)
		{
		case MSN_WEAPON_SNEAKING     : LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_SNEAKING     , 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_WEA_SNEAKING     , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case MSN_WEAPON_ELIMINATE_ALL: LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_ELIMINATE_ALL, 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_WEA_ELIMINATE_ALL, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		default:
			ASSERT(0);
			break;
		}
	}
	else if(mode == MSN_MODE_WEAPON)
	{
		if(VRSEL_IsWeaponPlayable(work) == 0)
			ASSERT(0);

		/* 武器選択（ＷＥＡＰＯＮ）に移行 */
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_52, 1, 1, 1); if(work->weapon == 0) work->weapon = MSN_WEAPON_HANDGUN; break;
		case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_61, 1, 1, 1); work->weapon = MSN_WEAPON_HF_BLADE; break;
		case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_52, 1, 1, 1); if(work->weapon == 0) work->weapon = MSN_WEAPON_HANDGUN; break;
		case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_52, 1, 1, 1); if(work->weapon == 0) work->weapon = MSN_WEAPON_HANDGUN; break;
		case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_52, 1, 1, 1); if(work->weapon == 0) work->weapon = MSN_WEAPON_HANDGUN; break;
		case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_52, 1, 1, 1); if(work->weapon == 0) work->weapon = MSN_WEAPON_HANDGUN; break;
		default:
			ASSERT(0);
			break;
		}
		/* カーソル位置の設定 */
		switch(work->weapon)
		{
		case MSN_WEAPON_HANDGUN      : LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_HANDGUN    , 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_WEA_HANDGUN    , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case MSN_WEAPON_ASSAULT_RIFLE: LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_ASSAULT_R  , 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_WEA_ASSAULT_R  , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case MSN_WEAPON_C4_CLAYMORE  : LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_C4_CLAYMORE, 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_WEA_C4_CLAYMORE, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case MSN_WEAPON_GRENADE      : LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_GRENADE    , 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_WEA_GRENADE    , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case MSN_WEAPON_PSG1         : LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_PSG1       , 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_WEA_PSG1       , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case MSN_WEAPON_STINGER      : LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_STINGER    , 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_WEA_STINGER    , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case MSN_WEAPON_NIKITA       : LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_NIKITA     , 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_WEA_NIKITA     , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		case MSN_WEAPON_HF_BLADE     : LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_HF_BLADE   , 0); LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_WEA_HF_BLADE   , VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1); break;
		default:
			ASSERT(0);
			break;
		}
	}
	else if(mode == MSN_MODE_FIRST_PERSON_VIEW)
	{
		if(VRSEL_IsFirstPersonViewPlayable(work) == 0)
			ASSERT(0);

		/* レベル選択に移行 */
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_63, 1, 1, 1); break;
		case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
		case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_63, 1, 1, 1); break;
		case VRSEL_PLA_PLISKIN : ASSERT(0); return 0;
		case VRSEL_PLA_TUXEDO  : ASSERT(0); return 0;
		case VRSEL_PLA_PREVIOUS: ASSERT(0); return 0;
		default:
			ASSERT(0);
			break;
		}
		/* カーソル位置の設定 */
		if(!(*_MSN_SAVE_DATA_FLAG & MSN_DFLAG_FPV_PLAYED))
		{
			*_MSN_SAVE_DATA_FLAG |= MSN_DFLAG_FPV_PLAYED;
#ifndef KP_WINDOWS
			work->level = 10;
#else
			work->level = 0;
#endif
		}
		VRSEL_L2_SetCursorPos(work, 5);

		/* レコード表示を一時的に消す */
		work->flag |= VRSEL_FLAG_HIDE_RECORD;
	}
	else if(mode == MSN_MODE_VARIETY)
	{
		if(VRSEL_IsVarietyPlayable(work) == 0)
			ASSERT(0);

		/* レベル選択に移行・カーソル位置の設定 */
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_65, 1, 1, 1); VRSEL_L2_SetCursorPos(work, 7); break;
		case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_67, 1, 1, 1); VRSEL_L2_SetCursorPos(work, 1); break;
		case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_65, 1, 1, 1); VRSEL_L2_SetCursorPos(work, 7); break;
		case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_69, 1, 1, 1); VRSEL_L2_SetCursorPos(work, 2); break;
		case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_69, 1, 1, 1); VRSEL_L2_SetCursorPos(work, 2); break;
		case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_71, 1, 1, 1); VRSEL_L2_SetCursorPos(work, 3); break;
		default:
			ASSERT(0);
			break;
		}

		/* レコード表示を一時的に消す */
		work->flag |= VRSEL_FLAG_HIDE_RECORD;
	}
	else if(mode == MSN_MODE_STREAKING)
	{
		if(VRSEL_IsStreakingPlayable(work) == 0)
			ASSERT(0);

		/* レベル選択に移行 */
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : ASSERT(0); return 0;
		case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
		case VRSEL_PLA_X_RAIDEN: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_73, 1, 1, 1); break;
		case VRSEL_PLA_SNAKE   : ASSERT(0); return 0;
		case VRSEL_PLA_PLISKIN : ASSERT(0); return 0;
		case VRSEL_PLA_TUXEDO  : ASSERT(0); return 0;
		case VRSEL_PLA_PREVIOUS: ASSERT(0); return 0;
		default:
			ASSERT(0);
			break;
		}
		/* カーソル位置の設定 */
		VRSEL_L2_SetCursorPos(work, 1);

		/* レコード表示を一時的に消す */
		work->flag |= VRSEL_FLAG_HIDE_RECORD;
	}
	else if(mode == MSN_MODE_BOMB_DISPOSAL)
	{
		if(VRSEL_IsBombDisposalPlayable(work) == 0)
			ASSERT(0);

		/* レベル選択に移行 */
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_75, 1, 1, 1); break;
		case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_75, 1, 1, 1); break;
		case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_75, 1, 1, 1); break;
		case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_75, 1, 1, 1); break;
		case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_75, 1, 1, 1); break;
		case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_75, 1, 1, 1); break;
		default:
			ASSERT(0);
			break;
		}
		/* カーソル位置の設定 */
		VRSEL_L2_SetCursorPos(work, 5);

		/* レコード表示を一時的に消す */
		work->flag |= VRSEL_FLAG_HIDE_RECORD;
	}
	else if(mode == MSN_MODE_ELIMINATE)
	{
		if(VRSEL_IsEliminatePlayable(work) == 0)
			ASSERT(0);

		/* レベル選択に移行 */
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_77, 1, 1, 1); break;
		case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_77, 1, 1, 1); break;
		case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_77, 1, 1, 1); break;
		case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_77, 1, 1, 1); break;
		case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_77, 1, 1, 1); break;
		case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_77, 1, 1, 1); break;
		default:
			ASSERT(0);
			break;
		}
		/* カーソル位置の設定 */
		VRSEL_L2_SetCursorPos(work, 10);

		/* レコード表示を一時的に消す */
		work->flag |= VRSEL_FLAG_HIDE_RECORD;
	}
	else if(mode == MSN_MODE_HOLD_UP)
	{
		if(VRSEL_IsHoldUpPlayable(work) == 0)
			ASSERT(0);

		/* レベル選択に移行 */
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_79, 1, 1, 1); break;
		case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
		case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_79, 1, 1, 1); break;
		case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_79, 1, 1, 1); break;
		case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_79, 1, 1, 1); break;
		case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_79, 1, 1, 1); break;
		default:
			ASSERT(0);
			break;
		}
		/* カーソル位置の設定 */
		VRSEL_L2_SetCursorPos(work, 10);

		/* レコード表示を一時的に消す */
		work->flag |= VRSEL_FLAG_HIDE_RECORD;
	}
	else if(mode == MSN_MODE_PHOTOGRAPH)
	{
		if(VRSEL_IsPhotographPlayable(work) == 0)
			ASSERT(0);

		/* レベル選択に移行・カーソル位置の設定 */
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_81, 1, 1, 1); VRSEL_L2_SetCursorPos(work, 6); break;
		case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
		case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_83, 1, 1, 1); VRSEL_L2_SetCursorPos(work, 7); break;
		case VRSEL_PLA_PLISKIN : ASSERT(0); return 0;
		case VRSEL_PLA_TUXEDO  : ASSERT(0); return 0;
		case VRSEL_PLA_PREVIOUS: ASSERT(0); return 0;
		default:
			ASSERT(0);
			break;
		}

		/* レコード表示を一時的に消す */
		work->flag |= VRSEL_FLAG_HIDE_RECORD;
	}
	else ASSERT(0);

	if(mode != MSN_MODE_STREAKING)
	{
		CancelMovieStream();
	}
	GM_SdSet(SD_S_WIN01);
	return 1;
}

/*******************************************************************************
 * 選択をキャンセル
 */

int VRSEL_Mod_Cancel_1(
	VRSEL_WORK *work)	/* ワーク */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	/* ミッション選択に戻る */
	switch(work->player)
	{
	case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_22, 1, 1, 1); break;
	case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_22, 1, 1, 1); break;
	case VRSEL_PLA_X_RAIDEN: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_25, 1, 1, 1); break;
	case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_22, 1, 1, 1); break;
	case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_22, 1, 1, 1); break;
	case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_22, 1, 1, 1); break;
	case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_22, 1, 1, 1); break;
	default:
		ASSERT(0);
		break;
	}
	/* カーソル位置の設定 */
	LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MIS_VR, 0);
	LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MIS_VR, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);

	if(work->mode != MSN_MODE_STREAKING)
	{
		CancelMovieStream();
	}
	GM_SdSet(SD_S_V_CANS02);
	return 1;
}

int VRSEL_Mod_Cancel_2(
	VRSEL_WORK *work)	/* ワーク */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	/* ミッション選択に戻る */
	switch(work->player)
	{
	case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_23, 1, 1, 1); break;
	case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_23, 1, 1, 1); break;
	case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
	case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_23, 1, 1, 1); break;
	case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_23, 1, 1, 1); break;
	case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_23, 1, 1, 1); break;
	case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_23, 1, 1, 1); break;
	default:
		ASSERT(0);
		break;
	}
	/* カーソル位置の設定 */
	LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MIS_ALTERNATIVE, 0);
	LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MIS_ALTERNATIVE, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);

	if(work->mode != MSN_MODE_STREAKING)
	{
		CancelMovieStream();
	}
	GM_SdSet(SD_S_V_CANS02);
	return 1;
}
