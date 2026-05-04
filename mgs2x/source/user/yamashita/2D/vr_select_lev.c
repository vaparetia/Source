//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * vr_select - vr_select_lev.c
 * ＶＲセレクト
 * 2002/06/03 S.Yamashita
 * $Id: vr_select_lev.c,v 1.2 2002/12/12 14:25:40 takaki Exp $
 */

/*******************************************************************************
 * include
 */

#include "vr_select.h"

/*******************************************************************************
 * extern
 */

extern void *NewKeyConSel(int page_num, int init_page, int *ret);
extern void *NewVRClearCode(int stage, int *status);

/*******************************************************************************
 * static
 */
/*******************************************************************************
 * プレイ可能か判定
 */
static int IsLevelPlayable(
	VRSEL_WORK *work,	/* ワーク */
	int        level)	/* レベル */
{
	int i, j;

	if(work->missions == MSN_MISSION_VR)
	{
		if(work->mode == MSN_MODE_SNEAKING)
		{
			if(level >= SNEAKING_STAGE_NUM)
				return 0;

			if(work->weapon == MSN_WEAPON_SNEAKING)
			{
				if((0 <= _sneaking_stage[0][level][(int)work->player]) && (_sneaking_stage[0][level][(int)work->player] <= 4))
					return 1;
			}
			else if(work->weapon == MSN_WEAPON_ELIMINATE_ALL)
			{
				if((0 <= _sneaking_stage[1][level][(int)work->player]) && (_sneaking_stage[1][level][(int)work->player] <= 4))
					return 1;
			}
			else
				ASSERT(0);
		}
		else if(work->mode == MSN_MODE_WEAPON)
		{
			if(level >= WEAPON_STAGE_NUM)
				return 0;

			if(work->weapon == MSN_WEAPON_HANDGUN)
			{
				if((0 <= _weapon_stage[0][level][(int)work->player]) && (_weapon_stage[0][level][(int)work->player] <= 4))
					return 1;
			}
			else if(work->weapon == MSN_WEAPON_ASSAULT_RIFLE)
			{
				if((0 <= _weapon_stage[1][level][(int)work->player]) && (_weapon_stage[1][level][(int)work->player] <= 4))
					return 1;
			}
			else if(work->weapon == MSN_WEAPON_C4_CLAYMORE)
			{
				if((0 <= _weapon_stage[2][level][(int)work->player]) && (_weapon_stage[2][level][(int)work->player] <= 4))
					return 1;
			}
			else if(work->weapon == MSN_WEAPON_GRENADE)
			{
				if((0 <= _weapon_stage[3][level][(int)work->player]) && (_weapon_stage[3][level][(int)work->player] <= 4))
					return 1;
			}
			else if(work->weapon == MSN_WEAPON_PSG1)
			{
				if((0 <= _weapon_stage[4][level][(int)work->player]) && (_weapon_stage[4][level][(int)work->player] <= 4))
					return 1;
			}
			else if(work->weapon == MSN_WEAPON_STINGER)
			{
				if((0 <= _weapon_stage[5][level][(int)work->player]) && (_weapon_stage[5][level][(int)work->player] <= 4))
					return 1;
			}
			else if(work->weapon == MSN_WEAPON_NIKITA)
			{
				if((0 <= _weapon_stage[6][level][(int)work->player]) && (_weapon_stage[6][level][(int)work->player] <= 4))
					return 1;
			}
			else if(work->weapon == MSN_WEAPON_HF_BLADE)
			{
				if((0 <= _weapon_stage[7][level][(int)work->player]) && (_weapon_stage[7][level][(int)work->player] <= 4))
					return 1;
			}
			else
				ASSERT(0);
		}
		else if(work->mode == MSN_MODE_FIRST_PERSON_VIEW)
		{
			if(level >= FPV_STAGE_NUM)
				return 0;

			if((0 <= _fpv_stage[level][(int)work->player]) && (_fpv_stage[level][(int)work->player] <= 4))
				return 1;
		}
		else if(work->mode == MSN_MODE_VARIETY)
		{
			/* ＶＡＲＩＥＴＹは特殊 */
			for(i = 0, j = 0; i < VARIETY_STAGE_NUM; i++)
			{
				if(_variety_stage[i][(int)work->player] > -1)
				{
					if(j == level)
						break;
					j++;
				}
			}
			if(i == VARIETY_STAGE_NUM)
				return 0;

			if((0 <= _variety_stage[i][(int)work->player]) && (_variety_stage[i][(int)work->player] <= 4))
				return 1;
		}
		else if(work->mode == MSN_MODE_STREAKING)
		{
			if(level != 0)
				return 0;

			if((0 <= _streaking_stage) && (_streaking_stage <= 4))
				return 1;
		}
		else
			ASSERT(0);
	}
	else if(work->missions == MSN_MISSION_ALTERNATIVE)
	{
		if(work->mode == MSN_MODE_BOMB_DISPOSAL)
		{
			if(level >= BOMB_STAGE_NUM)
				return 0;

			if((0 <= _bomb_stage[level][(int)work->player]) && (_bomb_stage[level][(int)work->player] <= 4))
				return 1;
		}
		else if(work->mode == MSN_MODE_ELIMINATE)
		{
			if(level >= ELIMINATE_STAGE_NUM)
				return 0;

			if((0 <= _eliminate_stage[level][(int)work->player]) && (_eliminate_stage[level][(int)work->player] <= 4))
				return 1;
		}
		else if(work->mode == MSN_MODE_HOLD_UP)
		{
			if(level >= HOLDUP_STAGE_NUM)
				return 0;

			if((0 <= _holdup_stage[level][(int)work->player]) && (_holdup_stage[level][(int)work->player] <= 4))
				return 1;
		}
		else if(work->mode == MSN_MODE_PHOTOGRAPH)
		{
			/* ＰＨＯＴＯＧＲＡＰＨは特殊 */
			for(i = 0, j = 0; i < PHOTOGRAPH_STAGE_NUM; i++)
			{
				if(_photograph_stage[i][(int)work->player] > -1)
				{
					if(j == level)
						break;
					j++;
				}
			}
			if(i == PHOTOGRAPH_STAGE_NUM)
				return 0;

			if((0 <= _photograph_stage[i][(int)work->player]) && (_photograph_stage[i][(int)work->player] <= 4))
				return 1;
		}
		else
			ASSERT(0);
	}
	else
		ASSERT(0);

	return 0;
}

/*******************************************************************************
 * public
 */
/*******************************************************************************
 * プレイ可能か判定
 */
int VRSEL_IsLevel01Playable(
	VRSEL_WORK *work)	/* ワーク */
{
	return IsLevelPlayable(work, 0);
}
int VRSEL_IsLevel02Playable(
	VRSEL_WORK *work)	/* ワーク */
{
	return IsLevelPlayable(work, 1);
}
int VRSEL_IsLevel03Playable(
	VRSEL_WORK *work)	/* ワーク */
{
	return IsLevelPlayable(work, 2);
}
int VRSEL_IsLevel04Playable(
	VRSEL_WORK *work)	/* ワーク */
{
	return IsLevelPlayable(work, 3);
}
int VRSEL_IsLevel05Playable(
	VRSEL_WORK *work)	/* ワーク */
{
	return IsLevelPlayable(work, 4);
}
int VRSEL_IsLevel06Playable(
	VRSEL_WORK *work)	/* ワーク */
{
	return IsLevelPlayable(work, 5);
}
int VRSEL_IsLevel07Playable(
	VRSEL_WORK *work)	/* ワーク */
{
	return IsLevelPlayable(work, 6);
}
int VRSEL_IsLevel08Playable(
	VRSEL_WORK *work)	/* ワーク */
{
	return IsLevelPlayable(work, 7);
}
int VRSEL_IsLevel09Playable(
	VRSEL_WORK *work)	/* ワーク */
{
	return IsLevelPlayable(work, 8);
}
int VRSEL_IsLevel10Playable(
	VRSEL_WORK *work)	/* ワーク */
{
	return IsLevelPlayable(work, 9);
}
int VRSEL_IsControlPlayable(
	VRSEL_WORK *work)	/* ワーク */
{
	if(work->mode == MSN_MODE_FIRST_PERSON_VIEW)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*******************************************************************************
 * カーソルを上へ移動
 */
int VRSEL_Lev_MoveUp(
	VRSEL_WORK *work,	/* ワーク */
	int        pos)		/* 現在位置 */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	/* 一つ上の項目へ移動 */
	if     ((pos >  9) && (VRSEL_IsLevel10Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_10     , 0); work->level =  9; }
	else if((pos >  8) && (VRSEL_IsLevel09Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_09     , 0); work->level =  8; }
	else if((pos >  7) && (VRSEL_IsLevel08Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_08     , 0); work->level =  7; }
	else if((pos >  6) && (VRSEL_IsLevel07Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_07     , 0); work->level =  6; }
	else if((pos >  5) && (VRSEL_IsLevel06Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_06     , 0); work->level =  5; }
	else if((pos >  4) && (VRSEL_IsLevel05Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_05     , 0); work->level =  4; }
	else if((pos >  3) && (VRSEL_IsLevel04Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_04     , 0); work->level =  3; }
	else if((pos >  2) && (VRSEL_IsLevel03Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_03     , 0); work->level =  2; }
	else if((pos >  1) && (VRSEL_IsLevel02Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_02     , 0); work->level =  1; }
	else if((pos >  0) && (VRSEL_IsLevel01Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_01     , 0); work->level =  0; }
#ifndef KP_WINDOWS
	else if((pos < 10) && (VRSEL_IsControlPlayable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_CONTROL, 0); work->level = 10; }
#endif
	else if((pos <  9) && (VRSEL_IsLevel10Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_10     , 0); work->level =  9; }
	else if((pos <  8) && (VRSEL_IsLevel09Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_09     , 0); work->level =  8; }
	else if((pos <  7) && (VRSEL_IsLevel08Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_08     , 0); work->level =  7; }
	else if((pos <  6) && (VRSEL_IsLevel07Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_07     , 0); work->level =  6; }
	else if((pos <  5) && (VRSEL_IsLevel06Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_06     , 0); work->level =  5; }
	else if((pos <  4) && (VRSEL_IsLevel05Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_05     , 0); work->level =  4; }
	else if((pos <  3) && (VRSEL_IsLevel04Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_04     , 0); work->level =  3; }
	else if((pos <  2) && (VRSEL_IsLevel03Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_03     , 0); work->level =  2; }
	else if((pos <  1) && (VRSEL_IsLevel02Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_02     , 0); work->level =  1; }
	else return 0;

	GM_SdSet(SD_S_CUR01);
	return 1;
}

/*******************************************************************************
 * カーソルを下へ移動
 */
int VRSEL_Lev_MoveDown(
	VRSEL_WORK *work,	/* ワーク */
	int        pos)		/* 現在位置 */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	/* 一つ下の項目へ移動 */
	if     ((pos <  1) && (VRSEL_IsLevel02Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_02     , 0); work->level =  1; }
	else if((pos <  2) && (VRSEL_IsLevel03Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_03     , 0); work->level =  2; }
	else if((pos <  3) && (VRSEL_IsLevel04Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_04     , 0); work->level =  3; }
	else if((pos <  4) && (VRSEL_IsLevel05Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_05     , 0); work->level =  4; }
	else if((pos <  5) && (VRSEL_IsLevel06Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_06     , 0); work->level =  5; }
	else if((pos <  6) && (VRSEL_IsLevel07Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_07     , 0); work->level =  6; }
	else if((pos <  7) && (VRSEL_IsLevel08Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_08     , 0); work->level =  7; }
	else if((pos <  8) && (VRSEL_IsLevel09Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_09     , 0); work->level =  8; }
	else if((pos <  9) && (VRSEL_IsLevel10Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_10     , 0); work->level =  9; }
#ifndef KP_WINDOWS
	else if((pos < 10) && (VRSEL_IsControlPlayable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_CONTROL, 0); work->level = 10; }
#endif
	else if((pos >  0) && (VRSEL_IsLevel01Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_01     , 0); work->level =  0; }
	else if((pos >  1) && (VRSEL_IsLevel02Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_02     , 0); work->level =  1; }
	else if((pos >  2) && (VRSEL_IsLevel03Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_03     , 0); work->level =  2; }
	else if((pos >  3) && (VRSEL_IsLevel04Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_04     , 0); work->level =  3; }
	else if((pos >  4) && (VRSEL_IsLevel05Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_05     , 0); work->level =  4; }
	else if((pos >  5) && (VRSEL_IsLevel06Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_06     , 0); work->level =  5; }
	else if((pos >  6) && (VRSEL_IsLevel07Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_07     , 0); work->level =  6; }
	else if((pos >  7) && (VRSEL_IsLevel08Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_08     , 0); work->level =  7; }
	else if((pos >  8) && (VRSEL_IsLevel09Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_09     , 0); work->level =  8; }
	else if((pos >  9) && (VRSEL_IsLevel10Playable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_LEV_10     , 0); work->level =  9; }
	else return 0;

	GM_SdSet(SD_S_CUR01);
	return 1;
}

/*******************************************************************************
 * 項目を選択
 */
int VRSEL_Lev_Select_1(
	VRSEL_WORK *work,	/* ワーク */
	int        level)	/* モード */
{
	int i, j;

	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	ASSERT(level == work->level);

	if     (level == 0) { if(VRSEL_IsLevel01Playable(work) == 0) ASSERT(0); }
	else if(level == 1) { if(VRSEL_IsLevel02Playable(work) == 0) ASSERT(0); }
	else if(level == 2) { if(VRSEL_IsLevel03Playable(work) == 0) ASSERT(0); }
	else if(level == 3) { if(VRSEL_IsLevel04Playable(work) == 0) ASSERT(0); }
	else if(level == 4) { if(VRSEL_IsLevel05Playable(work) == 0) ASSERT(0); }
	else if(level == 5) { if(VRSEL_IsLevel06Playable(work) == 0) ASSERT(0); }
	else if(level == 6) { if(VRSEL_IsLevel07Playable(work) == 0) ASSERT(0); }
	else if(level == 7) { if(VRSEL_IsLevel08Playable(work) == 0) ASSERT(0); }
	else if(level == 8) { if(VRSEL_IsLevel09Playable(work) == 0) ASSERT(0); }
	else if(level == 9) { if(VRSEL_IsLevel10Playable(work) == 0) ASSERT(0); }
	else ASSERT(0);

	if(work->missions == MSN_MISSION_VR)
	{
		if(work->mode == MSN_MODE_SNEAKING)
		{
			/* レベルを選択 */
			if(work->weapon == MSN_WEAPON_SNEAKING)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_112, 1, 1, 1); break;
				case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_112, 1, 1, 1); break;
				case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_112, 1, 1, 1); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_112, 1, 1, 1); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_112, 1, 1, 1); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_112, 1, 1, 1); break;
				default:
					ASSERT(0);
					break;
				}
			}
			else if(work->weapon == MSN_WEAPON_ELIMINATE_ALL)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_113, 1, 1, 1); break;
				case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_113, 1, 1, 1); break;
				case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_113, 1, 1, 1); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_113, 1, 1, 1); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_113, 1, 1, 1); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_113, 1, 1, 1); break;
				default:
					ASSERT(0);
					break;
				}
			}
			else ASSERT(0);
		}
		else if(work->mode == MSN_MODE_WEAPON)
		{
			if(work->weapon == MSN_WEAPON_HANDGUN)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_114, 1, 1, 1); break;
				case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
				case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_114, 1, 1, 1); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_114, 1, 1, 1); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_114, 1, 1, 1); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_114, 1, 1, 1); break;
				default:
					ASSERT(0);
					break;
				}
			}
			else if(work->weapon == MSN_WEAPON_ASSAULT_RIFLE)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_115, 1, 1, 1); break;
				case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
				case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_115, 1, 1, 1); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_115, 1, 1, 1); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_115, 1, 1, 1); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_115, 1, 1, 1); break;
				default:
					ASSERT(0);
					break;
				}
			}
			else if(work->weapon == MSN_WEAPON_C4_CLAYMORE)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_116, 1, 1, 1); break;
				case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
				case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_116, 1, 1, 1); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_116, 1, 1, 1); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_116, 1, 1, 1); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_116, 1, 1, 1); break;
				default:
					ASSERT(0);
					break;
				}
			}
			else if(work->weapon == MSN_WEAPON_GRENADE)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_117, 1, 1, 1); break;
				case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
				case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_117, 1, 1, 1); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_117, 1, 1, 1); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_117, 1, 1, 1); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_117, 1, 1, 1); break;
				default:
					ASSERT(0);
					break;
				}
			}
			else if(work->weapon == MSN_WEAPON_PSG1)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_118, 1, 1, 1); break;
				case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
				case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_118, 1, 1, 1); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_118, 1, 1, 1); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_118, 1, 1, 1); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_118, 1, 1, 1); break;
				default:
					ASSERT(0);
					break;
				}
			}
			else if(work->weapon == MSN_WEAPON_STINGER)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_119, 1, 1, 1); break;
				case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
				case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_119, 1, 1, 1); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_119, 1, 1, 1); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_119, 1, 1, 1); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_119, 1, 1, 1); break;
				default:
					ASSERT(0);
					break;
				}
			}
			else if(work->weapon == MSN_WEAPON_NIKITA)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_120, 1, 1, 1); break;
				case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
				case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_120, 1, 1, 1); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_120, 1, 1, 1); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_120, 1, 1, 1); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_120, 1, 1, 1); break;
				default:
					ASSERT(0);
					break;
				}
			}
			else if(work->weapon == MSN_WEAPON_HF_BLADE)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_121, 1, 1, 1); break;
				case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_121, 1, 1, 1); break;
				case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_121, 1, 1, 1); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_121, 1, 1, 1); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_121, 1, 1, 1); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_121, 1, 1, 1); break;
				default:
					ASSERT(0);
					break;
				}
			}
			else ASSERT(0);
		}
		else if(work->mode == MSN_MODE_FIRST_PERSON_VIEW)
		{
			switch(work->player)
			{
			case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_105, 1, 1, 1); break;
			case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
			case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
			case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_105, 1, 1, 1); break;
			case VRSEL_PLA_PLISKIN : ASSERT(0); return 0;
			case VRSEL_PLA_TUXEDO  : ASSERT(0); return 0;
			case VRSEL_PLA_PREVIOUS: ASSERT(0); return 0;
			default:
				ASSERT(0);
				break;
			}
		}
		else if(work->mode == MSN_MODE_VARIETY)
		{
			switch(work->player)
			{
			case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_106, 1, 1, 1); break;
			case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_106, 1, 1, 1); break;
			case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
			case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_106, 1, 1, 1); break;
			case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_106, 1, 1, 1); break;
			case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_106, 1, 1, 1); break;
			case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_106, 1, 1, 1); break;
			default:
				ASSERT(0);
				break;
			}

			/* ＶＡＲＩＥＴＹは特殊 */
			for(i = 0, j = 0; i < VARIETY_STAGE_NUM; i++)
			{
				if(_variety_stage[i][(int)work->player] > -1)
				{
					if(j == level)
						break;
					j++;
				}
			}
			ASSERT(i != VARIETY_STAGE_NUM);

			work->level = i;
		}
		else if(work->mode == MSN_MODE_STREAKING)
		{
			switch(work->player)
			{
			case VRSEL_PLA_RAIDEN  : ASSERT(0); return 0;
			case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
			case VRSEL_PLA_X_RAIDEN: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_107, 1, 1, 1); break;
			case VRSEL_PLA_SNAKE   : ASSERT(0); return 0;
			case VRSEL_PLA_PLISKIN : ASSERT(0); return 0;
			case VRSEL_PLA_TUXEDO  : ASSERT(0); return 0;
			case VRSEL_PLA_PREVIOUS: ASSERT(0); return 0;
			default:
				ASSERT(0);
				break;
			}
		}
		else ASSERT(0);
	}
	else if(work->missions == MSN_MISSION_ALTERNATIVE)
	{
		if(work->mode == MSN_MODE_BOMB_DISPOSAL)
		{
			switch(work->player)
			{
			case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_108, 1, 1, 1); break;
			case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_108, 1, 1, 1); break;
			case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
			case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_108, 1, 1, 1); break;
			case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_108, 1, 1, 1); break;
			case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_108, 1, 1, 1); break;
			case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_108, 1, 1, 1); break;
			default:
				ASSERT(0);
				break;
			}
		}
		else if(work->mode == MSN_MODE_ELIMINATE)
		{
			switch(work->player)
			{
			case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_109, 1, 1, 1); break;
			case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_109, 1, 1, 1); break;
			case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
			case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_109, 1, 1, 1); break;
			case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_109, 1, 1, 1); break;
			case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_109, 1, 1, 1); break;
			case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_109, 1, 1, 1); break;
			default:
				ASSERT(0);
				break;
			}
		}
		else if(work->mode == MSN_MODE_HOLD_UP)
		{
			switch(work->player)
			{
			case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_110, 1, 1, 1); break;
			case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
			case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
			case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_110, 1, 1, 1); break;
			case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_110, 1, 1, 1); break;
			case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_110, 1, 1, 1); break;
			case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_110, 1, 1, 1); break;
			default:
				ASSERT(0);
				break;
			}
		}
		else if(work->mode == MSN_MODE_PHOTOGRAPH)
		{
			switch(work->player)
			{
			case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_111, 1, 1, 1); break;
			case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
			case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
			case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_111, 1, 1, 1); break;
			case VRSEL_PLA_PLISKIN : ASSERT(0); return 0;
			case VRSEL_PLA_TUXEDO  : ASSERT(0); return 0;
			case VRSEL_PLA_PREVIOUS: ASSERT(0); return 0;
			default:
				ASSERT(0);
				break;
			}

			/* ＰＨＯＴＯＧＲＡＰＨは特殊 */
			for(i = 0, j = 0; i < PHOTOGRAPH_STAGE_NUM; i++)
			{
				if(_photograph_stage[i][(int)work->player] > -1)
				{
					if(j == level)
						break;
					j++;
				}
			}
			ASSERT(i != PHOTOGRAPH_STAGE_NUM);

			work->level = i;
		}
		else ASSERT(0);
	}
	else ASSERT(0);

	GM_SdSet(SD_S_WIN01);
	GM_SdSet(SD_S_WINCLS01);
	return 1;
}

int VRSEL_Lev_Select_2(
	VRSEL_WORK *work)	/* ワーク */
{
	int flags;

	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	ASSERT(work->level == 10);

	if(VRSEL_IsControlPlayable(work) == 0)
		ASSERT(0);

	/* キーコンフィグ選択 */
#ifdef KP_XBOX
	NewKeyConSel(3, work->keyconsel_set, &work->keyconsel_ret);
#else
	NewKeyConSel(2, work->keyconsel_set, &work->keyconsel_ret);
#endif

	/* 強引に非表示に */
	LOM_Emp_GetFlags(&work->layoutman_2, 2770484 /* ROOT */, &flags);
	flags |= SPR_FLAG_HIDDEN;
	LOM_Emp_SetFlags(&work->layoutman_2, 2770484 /* ROOT */,  flags);
	work->layoutman_2.lom_icolist[1].lom_ico[0].sprite->head.flags |= SPR_FLAG_HIDDEN;

	GM_SdSet(SD_S_WIN01);
	return 1;
}

int VRSEL_Lev_Select_3(
	VRSEL_WORK *work)	/* ワーク */
{
	int stage;
	int flags;

	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	if(work->level == 10)
		return 0;

	/* ステージＩＤの取得 */
	stage = VRSEL_GetStageID(work);
	if(stage == -1)
		return 0;

	/* １位取得してるかチェック */
	if(!(MSN_GET_HISCORE(stage, 1) & MSN_OVERWRITEFLAG))
		return 0;

	/* ＶＲクリアコード */
	NewVRClearCode(stage, &work->vrclc_status);

	/* 強引に非表示に */
	LOM_Emp_GetFlags(&work->layoutman_2, 2770484 /* ROOT */, &flags);
	flags |= SPR_FLAG_HIDDEN;
	LOM_Emp_SetFlags(&work->layoutman_2, 2770484 /* ROOT */,  flags);
	work->layoutman_2.lom_icolist[1].lom_ico[0].sprite->head.flags |= SPR_FLAG_HIDDEN;

	GM_SdSet(SD_S_WIN01);
	return 1;
}

/*******************************************************************************
 * 選択をキャンセル
 */

int VRSEL_Lev_Cancel(
	VRSEL_WORK *work)	/* ワーク */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	if(work->missions == MSN_MISSION_VR)
	{
		if(work->mode == MSN_MODE_SNEAKING)
		{
			/* レベルを選択 */
			if(work->weapon == MSN_WEAPON_SNEAKING)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_50, 1, 1, 1); break;
				case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_50, 1, 1, 1); break;
				case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_50, 1, 1, 1); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_50, 1, 1, 1); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_50, 1, 1, 1); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_50, 1, 1, 1); break;
				default:
					ASSERT(0);
					break;
				}
				/* カーソル位置の設定 */
				LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_SNEAKING, 0);
				LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_WEA_SNEAKING, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
			}
			else if(work->weapon == MSN_WEAPON_ELIMINATE_ALL)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_51, 1, 1, 1); break;
				case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_51, 1, 1, 1); break;
				case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_51, 1, 1, 1); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_51, 1, 1, 1); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_51, 1, 1, 1); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_51, 1, 1, 1); break;
				default:
					ASSERT(0);
					break;
				}
				/* カーソル位置の設定 */
				LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_ELIMINATE_ALL, 0);
				LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_WEA_ELIMINATE_ALL, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
			}
			else ASSERT(0);
		}
		else if(work->mode == MSN_MODE_WEAPON)
		{
			if(work->weapon == MSN_WEAPON_HANDGUN)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_53, 1, 1, 1); break;
				case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
				case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_53, 1, 1, 1); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_53, 1, 1, 1); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_53, 1, 1, 1); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_53, 1, 1, 1); break;
				default:
					ASSERT(0);
					break;
				}
				/* カーソル位置の設定 */
				LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_HANDGUN, 0);
				LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_WEA_HANDGUN, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
			}
			else if(work->weapon == MSN_WEAPON_ASSAULT_RIFLE)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_54, 1, 1, 1); break;
				case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
				case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_54, 1, 1, 1); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_54, 1, 1, 1); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_54, 1, 1, 1); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_54, 1, 1, 1); break;
				default:
					ASSERT(0);
					break;
				}
				/* カーソル位置の設定 */
				LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_ASSAULT_R, 0);
				LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_WEA_ASSAULT_R, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
			}
			else if(work->weapon == MSN_WEAPON_C4_CLAYMORE)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_55, 1, 1, 1); break;
				case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
				case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_55, 1, 1, 1); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_55, 1, 1, 1); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_55, 1, 1, 1); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_55, 1, 1, 1); break;
				default:
					ASSERT(0);
					break;
				}
				/* カーソル位置の設定 */
				LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_C4_CLAYMORE, 0);
				LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_WEA_C4_CLAYMORE, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
			}
			else if(work->weapon == MSN_WEAPON_GRENADE)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_56, 1, 1, 1); break;
				case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
				case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_56, 1, 1, 1); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_56, 1, 1, 1); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_56, 1, 1, 1); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_56, 1, 1, 1); break;
				default:
					ASSERT(0);
					break;
				}
				/* カーソル位置の設定 */
				LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_GRENADE, 0);
				LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_WEA_GRENADE, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
			}
			else if(work->weapon == MSN_WEAPON_PSG1)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_57, 1, 1, 1); break;
				case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
				case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_57, 1, 1, 1); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_57, 1, 1, 1); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_57, 1, 1, 1); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_57, 1, 1, 1); break;
				default:
					ASSERT(0);
					break;
				}
				/* カーソル位置の設定 */
				LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_PSG1, 0);
				LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_WEA_PSG1, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
			}
			else if(work->weapon == MSN_WEAPON_STINGER)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_58, 1, 1, 1); break;
				case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
				case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_58, 1, 1, 1); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_58, 1, 1, 1); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_58, 1, 1, 1); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_58, 1, 1, 1); break;
				default:
					ASSERT(0);
					break;
				}
				/* カーソル位置の設定 */
				LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_STINGER, 0);
				LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_WEA_STINGER, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
			}
			else if(work->weapon == MSN_WEAPON_NIKITA)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_59, 1, 1, 1); break;
				case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
				case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_59, 1, 1, 1); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_59, 1, 1, 1); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_59, 1, 1, 1); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_59, 1, 1, 1); break;
				default:
					ASSERT(0);
					break;
				}
				/* カーソル位置の設定 */
				LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_NIKITA, 0);
				LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_WEA_NIKITA, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
			}
			else if(work->weapon == MSN_WEAPON_HF_BLADE)
			{
				switch(work->player)
				{
				case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_60, 1, 1, 1); break;
				case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_62, 1, 1, 1); break;
				case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
				case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_60, 1, 1, 1); break;
				case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_60, 1, 1, 1); break;
				case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_60, 1, 1, 1); break;
				case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_60, 1, 1, 1); break;
				default:
					ASSERT(0);
					break;
				}
				/* カーソル位置の設定 */
				LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_HF_BLADE, 0);
				LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_WEA_HF_BLADE, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
			}
			else ASSERT(0);
		}
		else if(work->mode == MSN_MODE_FIRST_PERSON_VIEW)
		{
			switch(work->player)
			{
			case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_29, 1, 1, 1); break;
			case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
			case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
			case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_29, 1, 1, 1); break;
			case VRSEL_PLA_PLISKIN : ASSERT(0); return 0;
			case VRSEL_PLA_TUXEDO  : ASSERT(0); return 0;
			case VRSEL_PLA_PREVIOUS: ASSERT(0); return 0;
			default:
				ASSERT(0);
				break;
			}
			/* カーソル位置の設定 */
			LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_FSV_1, 0);
			LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MOD_FSV_1, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
		}
		else if(work->mode == MSN_MODE_VARIETY)
		{
			switch(work->player)
			{
			case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_30, 1, 1, 1); break;
			case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_34, 1, 1, 1); break;
			case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
			case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_30, 1, 1, 1); break;
			case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_34, 1, 1, 1); break;
			case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_34, 1, 1, 1); break;
			case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_34, 1, 1, 1); break;
			default:
				ASSERT(0);
				break;
			}
			/* カーソル位置の設定 */
			LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_VARIETY, 0);
			LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MOD_VARIETY, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
		}
		else if(work->mode == MSN_MODE_STREAKING)
		{
			switch(work->player)
			{
			case VRSEL_PLA_RAIDEN  : ASSERT(0); return 0;
			case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
			case VRSEL_PLA_X_RAIDEN: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_36, 1, 1, 1); break;
			case VRSEL_PLA_SNAKE   : ASSERT(0); return 0;
			case VRSEL_PLA_PLISKIN : ASSERT(0); return 0;
			case VRSEL_PLA_TUXEDO  : ASSERT(0); return 0;
			case VRSEL_PLA_PREVIOUS: ASSERT(0); return 0;
			default:
				ASSERT(0);
				break;
			}
			/* カーソル位置の設定 */
			LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_STREAKING, 0);
			LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MOD_STREAKING, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
		}
		else ASSERT(0);
	}
	else if(work->missions == MSN_MISSION_ALTERNATIVE)
	{
		if(work->mode == MSN_MODE_BOMB_DISPOSAL)
		{
			switch(work->player)
			{
			case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_38, 1, 1, 1); break;
			case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_43, 1, 1, 1); break;
			case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
			case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_38, 1, 1, 1); break;
			case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_46, 1, 1, 1); break;
			case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_46, 1, 1, 1); break;
			case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_46, 1, 1, 1); break;
			default:
				ASSERT(0);
				break;
			}
			/* カーソル位置の設定 */
			LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_BOMB_D, 0);
			LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MOD_BOMB_D, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
		}
		else if(work->mode == MSN_MODE_ELIMINATE)
		{
			switch(work->player)
			{
			case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_39, 1, 1, 1); break;
			case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_44, 1, 1, 1); break;
			case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
			case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_39, 1, 1, 1); break;
			case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_47, 1, 1, 1); break;
			case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_47, 1, 1, 1); break;
			case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_47, 1, 1, 1); break;
			default:
				ASSERT(0);
				break;
			}
			/* カーソル位置の設定 */
			LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_ELIMINATE, 0);
			LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MOD_ELIMINATE, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
		}
		else if(work->mode == MSN_MODE_HOLD_UP)
		{
			switch(work->player)
			{
			case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_40, 1, 1, 1); break;
			case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
			case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
			case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_40, 1, 1, 1); break;
			case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_48, 1, 1, 1); break;
			case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_48, 1, 1, 1); break;
			case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_48, 1, 1, 1); break;
			default:
				ASSERT(0);
				break;
			}
			/* カーソル位置の設定 */
			LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_HOLD_UP, 0);
			LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MOD_HOLD_UP, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
		}
		else if(work->mode == MSN_MODE_PHOTOGRAPH)
		{
			switch(work->player)
			{
			case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_41, 1, 1, 1); break;
			case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
			case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
			case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_41, 1, 1, 1); break;
			case VRSEL_PLA_PLISKIN : ASSERT(0); return 0;
			case VRSEL_PLA_TUXEDO  : ASSERT(0); return 0;
			case VRSEL_PLA_PREVIOUS: ASSERT(0); return 0;
			default:
				ASSERT(0);
				break;
			}
			/* カーソル位置の設定 */
			LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_PHOTOGRAPH, 0);
			LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MOD_PHOTOGRAPH, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);
		}
		else ASSERT(0);
	}
	else ASSERT(0);

	if((work->mode != MSN_MODE_SNEAKING) && (work->mode != MSN_MODE_WEAPON) && (work->flag != MSN_MODE_STREAKING))
		work->flag |= VRSEL_FLAG_REQUEST_MOVIE;

	GM_SdSet(SD_S_V_CANS02);
	return 1;
}
