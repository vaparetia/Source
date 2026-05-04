//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * msn - msn.c
 * ミッションズ
 * 2002/05/08 S.Yamashita
 * $Id: msn.c,v 1.3 2002/12/10 08:18:38 takaki Exp $
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

#define __CHARA_NAME__ "MSN"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
#include "../sy_util/sy_util.h"

/*******************************************************************************
 * public
 */
/*******************************************************************************
 * 現在のステージ情報の取得
 */
void Msn_GetCurrentStageInfo(
	char *mission,	/* ミッションタイプ */
	char *mode,		/* モードタイプ */
	char *weapon,	/* 武器タイプ */
	char *level,	/* ステージ番号 */
	char *player)	/* プレイヤービット */
{
	Msn_GetStageInfo(VR_STAGE_ID, mission, mode, weapon, level, player);
}

/*******************************************************************************
 * ステージ情報の取得
 */
void Msn_GetStageInfo(
	int  stage,		/* ステージＩＤ */
	char *mission,	/* ミッションタイプ */
	char *mode,		/* モードタイプ */
	char *weapon,	/* 武器タイプ */
	char *level,	/* ステージ番号 */
	char *player)	/* プレイヤービット */
{
	if(stage <= VRM_END)
	{
		/* ＶＲ　ＭＩＳＳＩＯＮＳ */
		if(mission != NULL) *mission = MSN_MISSION_VR;

		if(stage <= VRM_SNK_END)
		{
			/* ＳＮＥＡＫＩＮＧ　ＭＯＤＥ */
			if(mode != NULL) *mode = MSN_MODE_SNEAKING;

			if(stage <= VRM_SNK_SNK_END)
			{
				/* ＳＮＥＡＫＩＮＧ */
				if(weapon != NULL) *weapon = MSN_WEAPON_SNEAKING;
				if(level  != NULL) *level  = (stage - VRM_SNK_SNK_START) / VRM_SNK_SNK_PLAYER_NUM;
				if(player != NULL) *player = Msn_GetPlayer(VRM_SNK_SNK_PLAYER, (stage - VRM_SNK_SNK_START) % VRM_SNK_SNK_PLAYER_NUM);
			}
			else if(stage <= VRM_SNK_ELM_END)
			{
				/* ＥＬＩＭＩＮＡＴＥ　ＡＬＬ */
				if(weapon != NULL) *weapon = MSN_WEAPON_ELIMINATE_ALL;
				if(level  != NULL) *level  = (stage - VRM_SNK_ELM_START) / VRM_SNK_ELM_PLAYER_NUM;
				if(player != NULL) *player = Msn_GetPlayer(VRM_SNK_ELM_PLAYER, (stage - VRM_SNK_ELM_START) % VRM_SNK_ELM_PLAYER_NUM);
			}
			else goto ID_ERROR;
		}
		else if(stage <= VRM_WPN_END)
		{
			/* ＷＥＡＰＯＮ　ＭＯＤＥ */
			if(mode != NULL) *mode = MSN_MODE_WEAPON;

			if(stage <= VRM_WPN_HGN_END)
			{
				/* ＨＡＮＤＧＵＮ */
				if(weapon != NULL) *weapon = MSN_WEAPON_HANDGUN;
				if(level  != NULL) *level  = (stage - VRM_WPN_HGN_START) / VRM_WPN_HGN_PLAYER_NUM;
				if(player != NULL) *player = Msn_GetPlayer(VRM_WPN_HGN_PLAYER, (stage - VRM_WPN_HGN_START) % VRM_WPN_HGN_PLAYER_NUM);
			}
			else if(stage <= VRM_WPN_ASR_END)
			{
				/* ＡＳＳＡＵＬＴ　ＲＩＦＬＥ */
				if(weapon != NULL) *weapon = MSN_WEAPON_ASSAULT_RIFLE;
				if(level  != NULL) *level  = (stage - VRM_WPN_ASR_START) / VRM_WPN_ASR_PLAYER_NUM;
				if(player != NULL) *player = Msn_GetPlayer(VRM_WPN_ASR_PLAYER, (stage - VRM_WPN_ASR_START) % VRM_WPN_ASR_PLAYER_NUM);
			}
			else if(stage <= VRM_WPN_C4C_END)
			{
				/* Ｃ４／ＣＬＡＹＭＯＲＥ */
				if(weapon != NULL) *weapon = MSN_WEAPON_C4_CLAYMORE;
				if(level  != NULL) *level  = (stage - VRM_WPN_C4C_START) / VRM_WPN_C4C_PLAYER_NUM;
				if(player != NULL) *player = Msn_GetPlayer(VRM_WPN_C4C_PLAYER, (stage - VRM_WPN_C4C_START) % VRM_WPN_C4C_PLAYER_NUM);
			}
			else if(stage <= VRM_WPN_GRN_END)
			{
				/* ＧＲＥＮＡＤＥ */
				if(weapon != NULL) *weapon = MSN_WEAPON_GRENADE;
				if(level  != NULL) *level  = (stage - VRM_WPN_GRN_START) / VRM_WPN_GRN_PLAYER_NUM;
				if(player != NULL) *player = Msn_GetPlayer(VRM_WPN_GRN_PLAYER, (stage - VRM_WPN_GRN_START) % VRM_WPN_GRN_PLAYER_NUM);
			}
			else if(stage <= VRM_WPN_PSG_END)
			{
				/* ＰＳＧ－１ */
				if(weapon != NULL) *weapon = MSN_WEAPON_PSG1;
				if(level  != NULL) *level  = (stage - VRM_WPN_PSG_START) / VRM_WPN_PSG_PLAYER_NUM;
				if(player != NULL) *player = Msn_GetPlayer(VRM_WPN_PSG_PLAYER, (stage - VRM_WPN_PSG_START) % VRM_WPN_PSG_PLAYER_NUM);
			}
			else if(stage <= VRM_WPN_STG_END)
			{
				/* ＳＴＩＮＧＥＲ */
				if(weapon != NULL) *weapon = MSN_WEAPON_STINGER;
				if(level  != NULL) *level  = (stage - VRM_WPN_STG_START) / VRM_WPN_STG_PLAYER_NUM;
				if(player != NULL) *player = Msn_GetPlayer(VRM_WPN_STG_PLAYER, (stage - VRM_WPN_STG_START) % VRM_WPN_STG_PLAYER_NUM);
			}
			else if(stage <= VRM_WPN_NKT_END)
			{
				/* ＮＩＫＩＴＡ */
				if(weapon != NULL) *weapon = MSN_WEAPON_NIKITA;
				if(level  != NULL) *level  = (stage - VRM_WPN_NKT_START) / VRM_WPN_NKT_PLAYER_NUM;
				if(player != NULL) *player = Msn_GetPlayer(VRM_WPN_NKT_PLAYER, (stage - VRM_WPN_NKT_START) % VRM_WPN_NKT_PLAYER_NUM);
			}
			else if(stage <= VRM_WPN_HFB_END)
			{
				/* ＨＦ．ＢＬＡＤＥ／ＮＯ　ＷＥＡＰＯＮ */
				if(weapon != NULL) *weapon = MSN_WEAPON_HF_BLADE;
				if(level  != NULL) *level  = (stage - VRM_WPN_HFB_START) / VRM_WPN_HFB_PLAYER_NUM;
				if(player != NULL) *player = Msn_GetPlayer(VRM_WPN_HFB_PLAYER, (stage - VRM_WPN_HFB_START) % VRM_WPN_HFB_PLAYER_NUM);
			}
			else goto ID_ERROR;
		}
		else if(stage <= VRM_FPV_END)
		{
			/* ＦＩＲＳＴ　ＰＥＲＳＯＮ　ＶＩＥＷ　ＭＯＤＥ */
			if(mode   != NULL) *mode   = MSN_MODE_FIRST_PERSON_VIEW;
			if(level  != NULL) *level  = (stage - VRM_FPV_START) / VRM_FPV_PLAYER_NUM;
			if(player != NULL) *player = Msn_GetPlayer(VRM_FPV_PLAYER, (stage - VRM_FPV_START) % VRM_FPV_PLAYER_NUM);
		}
		else if(stage <= VRM_VRT_END)
		{
			/* ＶＡＲＩＥＴＹ　ＭＯＤＥ */
			if(mode   != NULL) *mode   = MSN_MODE_VARIETY;
			if(level  != NULL) *level  = (stage - VRM_VRT_START) / VRM_VRT_PLAYER_NUM;
			if(player != NULL) *player = Msn_GetPlayer(VRM_VRT_PLAYER, (stage - VRM_VRT_START) % VRM_VRT_PLAYER_NUM);

			/* 特別処理 */
			if(level != NULL)
			{
				int player2 = Msn_GetPlayer(VRM_VRT_PLAYER, (stage - VRM_VRT_START) % VRM_VRT_PLAYER_NUM);

				if(player2 & MSN_PLAYER_NINJA)
				{
					if(*level == 4) *level = 0;
					else ASSERT(0);
				}
				else if(player2 & (MSN_PLAYER_PLISKIN | MSN_PLAYER_TUXEDO))
				{
					if     (*level == 3) (*level) = 0;
					else if(*level == 4) (*level) = 1;
					else ASSERT(0);
				}
				else if(player2 & MSN_PLAYER_PREVIOUS)
				{
					if     (*level == 3) (*level) = 0;
					else if(*level == 4) (*level) = 1;
					else if(*level == 6) (*level) = 2;
					else ASSERT(0);
				}
			}
		}
		else if(stage <= VRM_STR_END)
		{
			/* ＳＴＲＥＡＫＩＮＧ　ＭＯＤＥ */
			if(mode   != NULL) *mode   = MSN_MODE_STREAKING;
			if(level  != NULL) *level  = (stage - VRM_STR_START) / VRM_STR_PLAYER_NUM;
			if(player != NULL) *player = Msn_GetPlayer(VRM_STR_PLAYER, (stage - VRM_STR_START) % VRM_STR_PLAYER_NUM);
		}
		else goto ID_ERROR;
	}
	else if(stage <= ALT_END)
	{
		/* ＡＬＴＥＲＮＡＴＩＶＥ　ＭＩＳＳＩＯＮＳ */
		if(mission != NULL) *mission = MSN_MISSION_ALTERNATIVE;

		if(stage <= ALT_BMB_END)
		{
			/* ＢＯＭＢ　ＤＩＳＰＯＳＡＬ　ＭＯＤＥ */
			if(mode   != NULL) *mode   = MSN_MODE_BOMB_DISPOSAL;
			if(level  != NULL) *level  = (stage - ALT_BMB_START) / ALT_BMB_PLAYER_NUM;
			if(player != NULL) *player = Msn_GetPlayer(ALT_BMB_PLAYER, (stage - ALT_BMB_START) % ALT_BMB_PLAYER_NUM);
		}
		else if(stage <= ALT_ELM_END)
		{
			/* ＥＬＩＭＩＮＡＴＥ　ＭＯＤＥ */
			if(mode   != NULL) *mode   = MSN_MODE_ELIMINATE;
			if(level  != NULL) *level  = (stage - ALT_ELM_START) / ALT_ELM_PLAYER_NUM;
			if(player != NULL) *player = Msn_GetPlayer(ALT_ELM_PLAYER, (stage - ALT_ELM_START) % ALT_ELM_PLAYER_NUM);
		}
		else if(stage <= ALT_HLD_END)
		{
			/* ＨＯＬＤ　ＵＰ　ＭＯＤＥ */
			if(mode   != NULL) *mode   = MSN_MODE_HOLD_UP;
			if(level  != NULL) *level  = (stage - ALT_HLD_START) / ALT_HLD_PLAYER_NUM;
			if(player != NULL) *player = Msn_GetPlayer(ALT_HLD_PLAYER, (stage - ALT_HLD_START) % ALT_HLD_PLAYER_NUM);
		}
		else if(stage <= ALT_PHT_END)
		{
			/* ＰＨＯＴＯＧＲＡＰＨ　ＭＯＤＥ */
			if(mode   != NULL) *mode   = MSN_MODE_PHOTOGRAPH;
			if(level  != NULL) *level  = (stage - ALT_PHT_START) / ALT_PHT_PLAYER_NUM;
			if(player != NULL) *player = Msn_GetPlayer(ALT_PHT_PLAYER, (stage - ALT_PHT_START) % ALT_PHT_PLAYER_NUM);
		}
		else goto ID_ERROR;
	}
	else goto ID_ERROR;

	return;

ID_ERROR:
	SY_PRINTF2("!!! INVALID STAGE ID !!!\n");
	SY_PRINTF2("!!! INVALID STAGE ID !!!\n");
	SY_PRINTF2("!!! INVALID STAGE ID !!!\n");
	SY_PRINTF2("!!! INVALID STAGE ID !!!\n");
	SY_PRINTF2("!!! INVALID STAGE ID !!!\n");
	ASSERT(0);
}

/*******************************************************************************
 * データをデフォルト値へ設定

command ミッションズデータ初期化[Msn_SetDefaultData]
 */
void Msn_SetDefaultData(void)
{
	memset(_MSN_SAVE_DATA, 0x00, MSN_SAVE_DATA_SIZE);

#ifndef KP_WINDOWS
#include "../../../../scn/msn_def_hiscore.h"
#else
#include "msn_def_hiscore.h"
#endif

	*_MSN_SAVE_DATA_VERSION = MSN_SAVE_DATA_VERSION;
	*_MSN_SAVE_DATA_SIZE    = MSN_SAVE_DATA_SIZE;

	MSN_RETRY_COUNT    =  0;
	MSN_CONTINUE_COUNT =  0;
	MSN_2DSTATUS       =  0;
	MSN_STAGE_ID       = -1;

#if 0
	{
		int i; 
		for(i = 0; i < MSN_STAGE_ID_MAX; i++)
		{
			(*MSN_STAGE_DATA(i)) |= MSN_CLEARFLAG | MSN_OVERWRITEFLAG;
		}
	}
#endif
}

/*******************************************************************************
 * 新しいスコアの登録
 *   各順位を更新する
 */
int Msn_SetNewScore(	/* 順位(1～4) */
	int stage,	/* ステージＩＤ */
	int score)	/* スコア */
{
	if(score > 999999)
		score = 999999;

	MSN_SET_CLEARFLAG(stage);

	if(score <= MSN_GET_HISCORE2(stage, 3))
	{
		/* ４位 */
		return 4;
	}
	else if(score <= MSN_GET_HISCORE2(stage, 2))
	{
		/* ３位 */
		MSN_SET_HISCORE(stage, 3, score | MSN_OVERWRITEFLAG);
		return 3;
	}
	else if(score <= MSN_GET_HISCORE2(stage, 1))
	{
		/* ２位 */
		MSN_SET_HISCORE(stage, 3, MSN_GET_HISCORE(stage, 2));
		MSN_SET_HISCORE(stage, 2, score | MSN_OVERWRITEFLAG);
		return 2;
	}
	else
	{
		/* １位 */
		MSN_SET_HISCORE(stage, 3, MSN_GET_HISCORE(stage, 2));
		MSN_SET_HISCORE(stage, 2, MSN_GET_HISCORE(stage, 1));
		MSN_SET_HISCORE(stage, 1, score | MSN_OVERWRITEFLAG);
		return 1;
	}
}

/*******************************************************************************
 * クリアレベルの取得
 */
int Msn_GetClearLevel(	/* 0: 未クリア  1: １位  2: ２位  3: ３位  4: ４位 */
	int stage)	/* ステージＩＤ */
{
	int level;

	level = MSN_GET_CLEARFLAG(stage);

	if(level != 0)
	{
		if     (MSN_GET_HISCORE(stage, 1) & MSN_OVERWRITEFLAG) level = 1;
		else if(MSN_GET_HISCORE(stage, 2) & MSN_OVERWRITEFLAG) level = 2;
		else if(MSN_GET_HISCORE(stage, 3) & MSN_OVERWRITEFLAG) level = 3;
		else                                                   level = 4;
	}

	return level;
}

/*******************************************************************************
 * ステージ数の取得
 */
// weapon が必要ないモードでは weapon = MSN_WEAPON_NULL として呼んだときに値を返す
// （それ以外ではステージ数=０をかえす） MODIFY M.Kobayashi 2002/07/25
int Msn_GetStageCount(		/* 全ステージ数 */
	char mission,	/* ミッションタイプ */
	char mode,		/* モードタイプ */
	char weapon,	/* 武器タイプ */
	int  player,	/* プレイヤービット(複数指定可) */
	int  *cleared)	/* クリアステージ数 */
{
	int i;
	int stage_num     = 0;
	int mode_player   = 0;
	int stage_id      = 0;
	int stage_count   = 0;
	int cleared_count = 0;

	if(mission == MSN_MISSION_VR)
	{
		if(mode == MSN_MODE_SNEAKING)
		{
			if(weapon == MSN_WEAPON_SNEAKING)
			{
				stage_num   = ((VRM_SNK_SNK_END - VRM_SNK_SNK_START) / VRM_SNK_SNK_PLAYER_NUM) + 1;
				mode_player = VRM_SNK_SNK_PLAYER;
				stage_id    = VRM_SNK_SNK_START;
			}
			else if(weapon == MSN_WEAPON_ELIMINATE_ALL)
			{
				stage_num   = ((VRM_SNK_ELM_END - VRM_SNK_ELM_START) / VRM_SNK_ELM_PLAYER_NUM) + 1;
				mode_player = VRM_SNK_ELM_PLAYER;
				stage_id    = VRM_SNK_ELM_START;
			}
		}
		else if(mode == MSN_MODE_WEAPON)
		{
			if(weapon == MSN_WEAPON_HANDGUN)
			{
				stage_num   = ((VRM_WPN_HGN_END - VRM_WPN_HGN_START) / VRM_WPN_HGN_PLAYER_NUM) + 1;
				mode_player = VRM_WPN_HGN_PLAYER;
				stage_id    = VRM_WPN_HGN_START;
			}
			else if(weapon == MSN_WEAPON_ASSAULT_RIFLE)
			{
				stage_num   = ((VRM_WPN_ASR_END - VRM_WPN_ASR_START) / VRM_WPN_ASR_PLAYER_NUM) + 1;
				mode_player = VRM_WPN_ASR_PLAYER;
				stage_id    = VRM_WPN_ASR_START;
			}
			else if(weapon == MSN_WEAPON_C4_CLAYMORE)
			{
				stage_num   = ((VRM_WPN_C4C_END - VRM_WPN_C4C_START) / VRM_WPN_C4C_PLAYER_NUM) + 1;
				mode_player = VRM_WPN_C4C_PLAYER;
				stage_id    = VRM_WPN_C4C_START;
			}
			else if(weapon == MSN_WEAPON_GRENADE)
			{
				stage_num   = ((VRM_WPN_GRN_END - VRM_WPN_GRN_START) / VRM_WPN_GRN_PLAYER_NUM) + 1;
				mode_player = VRM_WPN_GRN_PLAYER;
				stage_id    = VRM_WPN_GRN_START;
			}
			else if(weapon == MSN_WEAPON_PSG1)
			{
				stage_num   = ((VRM_WPN_PSG_END - VRM_WPN_PSG_START) / VRM_WPN_PSG_PLAYER_NUM) + 1;
				mode_player = VRM_WPN_PSG_PLAYER;
				stage_id    = VRM_WPN_PSG_START;
			}
			else if(weapon == MSN_WEAPON_STINGER)
			{
				stage_num   = ((VRM_WPN_STG_END - VRM_WPN_STG_START) / VRM_WPN_STG_PLAYER_NUM) + 1;
				mode_player = VRM_WPN_STG_PLAYER;
				stage_id    = VRM_WPN_STG_START;
			}
			else if(weapon == MSN_WEAPON_NIKITA)
			{
				stage_num   = ((VRM_WPN_NKT_END - VRM_WPN_NKT_START) / VRM_WPN_NKT_PLAYER_NUM) + 1;
				mode_player = VRM_WPN_NKT_PLAYER;
				stage_id    = VRM_WPN_NKT_START;
			}
			else if(weapon == MSN_WEAPON_HF_BLADE)
			{
				stage_num   = ((VRM_WPN_HFB_END - VRM_WPN_HFB_START) / VRM_WPN_HFB_PLAYER_NUM) + 1;
				mode_player = VRM_WPN_HFB_PLAYER;
				stage_id    = VRM_WPN_HFB_START;
			}
		}
		else if(mode == MSN_MODE_FIRST_PERSON_VIEW)
		{
			if( weapon == MSN_WEAPON_NULL ) {
				stage_num   = ((VRM_FPV_END - VRM_FPV_START) / VRM_FPV_PLAYER_NUM) + 1;
				mode_player = VRM_FPV_PLAYER;
				stage_id    = VRM_FPV_START;
			}
		}
		else if(mode == MSN_MODE_VARIETY)
		{
			if( weapon == MSN_WEAPON_NULL ) {
				stage_num   = ((VRM_VRT_END - VRM_VRT_START) / VRM_VRT_PLAYER_NUM) + 1;
				mode_player = VRM_VRT_PLAYER;
				stage_id    = VRM_VRT_START;
			}
		}
		else if(mode == MSN_MODE_STREAKING)
		{
			if( weapon == MSN_WEAPON_NULL ) {
				stage_num   = ((VRM_STR_END - VRM_STR_START) / VRM_STR_PLAYER_NUM) + 1;
				mode_player = VRM_STR_PLAYER;
				stage_id    = VRM_STR_START;
			}
		}
	}
	else if(mission == MSN_MISSION_ALTERNATIVE)
	{
		if( weapon == MSN_WEAPON_NULL ) {
			if(mode == MSN_MODE_BOMB_DISPOSAL)
			{
				stage_num   = ((ALT_BMB_END - ALT_BMB_START) / ALT_BMB_PLAYER_NUM) + 1;
				mode_player = ALT_BMB_PLAYER;
				stage_id    = ALT_BMB_START;
			}
			else if(mode == MSN_MODE_ELIMINATE)
			{
				stage_num   = ((ALT_ELM_END - ALT_ELM_START) / ALT_ELM_PLAYER_NUM) + 1;
				mode_player = ALT_ELM_PLAYER;
				stage_id    = ALT_ELM_START;
			}
			else if(mode == MSN_MODE_HOLD_UP)
			{
				stage_num   = ((ALT_HLD_END - ALT_HLD_START) / ALT_HLD_PLAYER_NUM) + 1;
				mode_player = ALT_HLD_PLAYER;
				stage_id    = ALT_HLD_START;
			}
			else if(mode == MSN_MODE_PHOTOGRAPH)
			{
				stage_num   = ((ALT_PHT_END - ALT_PHT_START) / ALT_PHT_PLAYER_NUM) + 1;
				mode_player = ALT_PHT_PLAYER;
				stage_id    = ALT_PHT_START;
			}
		}
	}

	/* ステージ数のカウント */
	for(i = 0; i < stage_num; i++)
	{
		if(mode_player & MSN_PLAYER_RAIDEN)
		{
			if((player & MSN_PLAYER_RAIDEN))
			{
				if(MSN_GET_HISCORE2(stage_id, 1) != 0)
				{
					/* １位のハイスコアが０でないステージは存在ステージとして扱う */
					stage_count++;
					if(MSN_GET_CLEARFLAG(stage_id) != 0)
						cleared_count++;
				}
			}
			stage_id++;
		}
		if(mode_player & MSN_PLAYER_NINJA)
		{
			if((player & MSN_PLAYER_NINJA))
			{
				if(MSN_GET_HISCORE2(stage_id, 1) != 0)
				{
					/* １位のハイスコアが０でないステージは存在ステージとして扱う */
					stage_count++;
					if(MSN_GET_CLEARFLAG(stage_id) != 0)
						cleared_count++;
				}
			}
			stage_id++;
		}
		if(mode_player & MSN_PLAYER_SNAKE)
		{
			if((player & MSN_PLAYER_SNAKE))
			{
				if(MSN_GET_HISCORE2(stage_id, 1) != 0)
				{
					/* １位のハイスコアが０でないステージは存在ステージとして扱う */
					stage_count++;
					if(MSN_GET_CLEARFLAG(stage_id) != 0)
						cleared_count++;
				}
			}
			stage_id++;
		}
		if(mode_player & MSN_PLAYER_PLISKIN)
		{
			if((player & MSN_PLAYER_PLISKIN))
			{
				if(MSN_GET_HISCORE2(stage_id, 1) != 0)
				{
					/* １位のハイスコアが０でないステージは存在ステージとして扱う */
					stage_count++;
					if(MSN_GET_CLEARFLAG(stage_id) != 0)
						cleared_count++;
				}
			}
			stage_id++;
		}
		if(mode_player & MSN_PLAYER_TUXEDO)
		{
			if((player & MSN_PLAYER_TUXEDO))
			{
				if(MSN_GET_HISCORE2(stage_id, 1) != 0)
				{
					/* １位のハイスコアが０でないステージは存在ステージとして扱う */
					stage_count++;
					if(MSN_GET_CLEARFLAG(stage_id) != 0)
						cleared_count++;
				}
			}
			stage_id++;
		}
		if(mode_player & MSN_PLAYER_PREVIOUS)
		{
			if((player & MSN_PLAYER_PREVIOUS))
			{
				if(MSN_GET_HISCORE2(stage_id, 1) != 0)
				{
					/* １位のハイスコアが０でないステージは存在ステージとして扱う */
					stage_count++;
					if(MSN_GET_CLEARFLAG(stage_id) != 0)
						cleared_count++;
				}
			}
			stage_id++;
		}
	}

	if(cleared != NULL) *cleared = cleared_count;
	return stage_count;
}

// Version of Msn_GetStageCount that uses vrScoreDataBuffer instead of the #defined locations
int Transfarring_Msn_GetStageCount(		/* 全ステージ数 */
                                   const char* vrScoreDataBuffer,
                                   char mission,	/* ミッションタイプ */
                                   char mode,		/* モードタイプ */
                                   char weapon,	/* 武器タイプ */
                                   int  player,	/* プレイヤービット(複数指定可) */
                                   int  *cleared)	/* クリアステージ数 */
{
   int i;
   int stage_num     = 0;
   int mode_player   = 0;
   int stage_id      = 0;
   int stage_count   = 0;
   int cleared_count = 0;

   if(mission == MSN_MISSION_VR)
   {
      if(mode == MSN_MODE_SNEAKING)
      {
         if(weapon == MSN_WEAPON_SNEAKING)
         {
            stage_num   = ((VRM_SNK_SNK_END - VRM_SNK_SNK_START) / VRM_SNK_SNK_PLAYER_NUM) + 1;
            mode_player = VRM_SNK_SNK_PLAYER;
            stage_id    = VRM_SNK_SNK_START;
         }
         else if(weapon == MSN_WEAPON_ELIMINATE_ALL)
         {
            stage_num   = ((VRM_SNK_ELM_END - VRM_SNK_ELM_START) / VRM_SNK_ELM_PLAYER_NUM) + 1;
            mode_player = VRM_SNK_ELM_PLAYER;
            stage_id    = VRM_SNK_ELM_START;
         }
      }
      else if(mode == MSN_MODE_WEAPON)
      {
         if(weapon == MSN_WEAPON_HANDGUN)
         {
            stage_num   = ((VRM_WPN_HGN_END - VRM_WPN_HGN_START) / VRM_WPN_HGN_PLAYER_NUM) + 1;
            mode_player = VRM_WPN_HGN_PLAYER;
            stage_id    = VRM_WPN_HGN_START;
         }
         else if(weapon == MSN_WEAPON_ASSAULT_RIFLE)
         {
            stage_num   = ((VRM_WPN_ASR_END - VRM_WPN_ASR_START) / VRM_WPN_ASR_PLAYER_NUM) + 1;
            mode_player = VRM_WPN_ASR_PLAYER;
            stage_id    = VRM_WPN_ASR_START;
         }
         else if(weapon == MSN_WEAPON_C4_CLAYMORE)
         {
            stage_num   = ((VRM_WPN_C4C_END - VRM_WPN_C4C_START) / VRM_WPN_C4C_PLAYER_NUM) + 1;
            mode_player = VRM_WPN_C4C_PLAYER;
            stage_id    = VRM_WPN_C4C_START;
         }
         else if(weapon == MSN_WEAPON_GRENADE)
         {
            stage_num   = ((VRM_WPN_GRN_END - VRM_WPN_GRN_START) / VRM_WPN_GRN_PLAYER_NUM) + 1;
            mode_player = VRM_WPN_GRN_PLAYER;
            stage_id    = VRM_WPN_GRN_START;
         }
         else if(weapon == MSN_WEAPON_PSG1)
         {
            stage_num   = ((VRM_WPN_PSG_END - VRM_WPN_PSG_START) / VRM_WPN_PSG_PLAYER_NUM) + 1;
            mode_player = VRM_WPN_PSG_PLAYER;
            stage_id    = VRM_WPN_PSG_START;
         }
         else if(weapon == MSN_WEAPON_STINGER)
         {
            stage_num   = ((VRM_WPN_STG_END - VRM_WPN_STG_START) / VRM_WPN_STG_PLAYER_NUM) + 1;
            mode_player = VRM_WPN_STG_PLAYER;
            stage_id    = VRM_WPN_STG_START;
         }
         else if(weapon == MSN_WEAPON_NIKITA)
         {
            stage_num   = ((VRM_WPN_NKT_END - VRM_WPN_NKT_START) / VRM_WPN_NKT_PLAYER_NUM) + 1;
            mode_player = VRM_WPN_NKT_PLAYER;
            stage_id    = VRM_WPN_NKT_START;
         }
         else if(weapon == MSN_WEAPON_HF_BLADE)
         {
            stage_num   = ((VRM_WPN_HFB_END - VRM_WPN_HFB_START) / VRM_WPN_HFB_PLAYER_NUM) + 1;
            mode_player = VRM_WPN_HFB_PLAYER;
            stage_id    = VRM_WPN_HFB_START;
         }
      }
      else if(mode == MSN_MODE_FIRST_PERSON_VIEW)
      {
         if( weapon == MSN_WEAPON_NULL ) {
            stage_num   = ((VRM_FPV_END - VRM_FPV_START) / VRM_FPV_PLAYER_NUM) + 1;
            mode_player = VRM_FPV_PLAYER;
            stage_id    = VRM_FPV_START;
         }
      }
      else if(mode == MSN_MODE_VARIETY)
      {
         if( weapon == MSN_WEAPON_NULL ) {
            stage_num   = ((VRM_VRT_END - VRM_VRT_START) / VRM_VRT_PLAYER_NUM) + 1;
            mode_player = VRM_VRT_PLAYER;
            stage_id    = VRM_VRT_START;
         }
      }
      else if(mode == MSN_MODE_STREAKING)
      {
         if( weapon == MSN_WEAPON_NULL ) {
            stage_num   = ((VRM_STR_END - VRM_STR_START) / VRM_STR_PLAYER_NUM) + 1;
            mode_player = VRM_STR_PLAYER;
            stage_id    = VRM_STR_START;
         }
      }
   }
   else if(mission == MSN_MISSION_ALTERNATIVE)
   {
      if( weapon == MSN_WEAPON_NULL ) {
         if(mode == MSN_MODE_BOMB_DISPOSAL)
         {
            stage_num   = ((ALT_BMB_END - ALT_BMB_START) / ALT_BMB_PLAYER_NUM) + 1;
            mode_player = ALT_BMB_PLAYER;
            stage_id    = ALT_BMB_START;
         }
         else if(mode == MSN_MODE_ELIMINATE)
         {
            stage_num   = ((ALT_ELM_END - ALT_ELM_START) / ALT_ELM_PLAYER_NUM) + 1;
            mode_player = ALT_ELM_PLAYER;
            stage_id    = ALT_ELM_START;
         }
         else if(mode == MSN_MODE_HOLD_UP)
         {
            stage_num   = ((ALT_HLD_END - ALT_HLD_START) / ALT_HLD_PLAYER_NUM) + 1;
            mode_player = ALT_HLD_PLAYER;
            stage_id    = ALT_HLD_START;
         }
         else if(mode == MSN_MODE_PHOTOGRAPH)
         {
            stage_num   = ((ALT_PHT_END - ALT_PHT_START) / ALT_PHT_PLAYER_NUM) + 1;
            mode_player = ALT_PHT_PLAYER;
            stage_id    = ALT_PHT_START;
         }
      }
   }

   /* ステージ数のカウント */
   for(i = 0; i < stage_num; i++)
   {
      if(mode_player & MSN_PLAYER_RAIDEN)
      {
         if((player & MSN_PLAYER_RAIDEN))
         {
            if(TRANSFARRING_MSN_GET_HISCORE2(stage_id, 1, vrScoreDataBuffer) != 0)
            {
               /* １位のハイスコアが０でないステージは存在ステージとして扱う */
               stage_count++;
               if(TRANSFARRING_MSN_GET_CLEARFLAG(stage_id, vrScoreDataBuffer) != 0)
                  cleared_count++;
            }
         }
         stage_id++;
      }
      if(mode_player & MSN_PLAYER_NINJA)
      {
         if((player & MSN_PLAYER_NINJA))
         {
            if(TRANSFARRING_MSN_GET_HISCORE2(stage_id, 1, vrScoreDataBuffer) != 0)
            {
               /* １位のハイスコアが０でないステージは存在ステージとして扱う */
               stage_count++;
               if(TRANSFARRING_MSN_GET_CLEARFLAG(stage_id, vrScoreDataBuffer) != 0)
                  cleared_count++;
            }
         }
         stage_id++;
      }
      if(mode_player & MSN_PLAYER_SNAKE)
      {
         if((player & MSN_PLAYER_SNAKE))
         {
            if(TRANSFARRING_MSN_GET_HISCORE2(stage_id, 1, vrScoreDataBuffer) != 0)
            {
               /* １位のハイスコアが０でないステージは存在ステージとして扱う */
               stage_count++;
               if(TRANSFARRING_MSN_GET_CLEARFLAG(stage_id, vrScoreDataBuffer) != 0)
                  cleared_count++;
            }
         }
         stage_id++;
      }
      if(mode_player & MSN_PLAYER_PLISKIN)
      {
         if((player & MSN_PLAYER_PLISKIN))
         {
            if(TRANSFARRING_MSN_GET_HISCORE2(stage_id, 1, vrScoreDataBuffer) != 0)
            {
               /* １位のハイスコアが０でないステージは存在ステージとして扱う */
               stage_count++;
               if(TRANSFARRING_MSN_GET_CLEARFLAG(stage_id, vrScoreDataBuffer) != 0)
                  cleared_count++;
            }
         }
         stage_id++;
      }
      if(mode_player & MSN_PLAYER_TUXEDO)
      {
         if((player & MSN_PLAYER_TUXEDO))
         {
            if(TRANSFARRING_MSN_GET_HISCORE2(stage_id, 1, vrScoreDataBuffer) != 0)
            {
               /* １位のハイスコアが０でないステージは存在ステージとして扱う */
               stage_count++;
               if(TRANSFARRING_MSN_GET_CLEARFLAG(stage_id, vrScoreDataBuffer) != 0)
                  cleared_count++;
            }
         }
         stage_id++;
      }
      if(mode_player & MSN_PLAYER_PREVIOUS)
      {
         if((player & MSN_PLAYER_PREVIOUS))
         {
            if(TRANSFARRING_MSN_GET_HISCORE2(stage_id, 1, vrScoreDataBuffer) != 0)
            {
               /* １位のハイスコアが０でないステージは存在ステージとして扱う */
               stage_count++;
               if(TRANSFARRING_MSN_GET_CLEARFLAG(stage_id, vrScoreDataBuffer) != 0)
                  cleared_count++;
            }
         }
         stage_id++;
      }
   }

   if(cleared != NULL) *cleared = cleared_count;
   return stage_count;
}

/*******************************************************************************
 * プレイヤービットの取得
 */
int Msn_GetPlayer(	/* プレイヤービット */
	int mode_player,	/* 各モードのプレイヤービット */
	int count)			/* 番号(0～) */
{
	if(mode_player & MSN_PLAYER_RAIDEN  ) {if(count-- == 0) return MSN_PLAYER_RAIDEN  ;}
	if(mode_player & MSN_PLAYER_NINJA   ) {if(count-- == 0) return MSN_PLAYER_NINJA   ;}
	if(mode_player & MSN_PLAYER_SNAKE   ) {if(count-- == 0) return MSN_PLAYER_SNAKE   ;}
	if(mode_player & MSN_PLAYER_PLISKIN ) {if(count-- == 0) return MSN_PLAYER_PLISKIN ;}
	if(mode_player & MSN_PLAYER_TUXEDO  ) {if(count-- == 0) return MSN_PLAYER_TUXEDO  ;}
	if(mode_player & MSN_PLAYER_PREVIOUS) {if(count-- == 0) return MSN_PLAYER_PREVIOUS;}

	ASSERT(0);
	return 0;
}

/*******************************************************************************
 * プレイヤー番号の取得
 */
int Msn_GetPlayerNum(	/* プレイヤー番号(0～) */
	int mode_player,	/* 各モードのプレイヤービット */
	int player)			/* プレイヤービット */
{
	int count = 0;
	if(mode_player & MSN_PLAYER_RAIDEN)
	{
		if(player & MSN_PLAYER_RAIDEN)
			return count;
		count++;
	}
	if(mode_player & MSN_PLAYER_NINJA)
	{
		if(player & MSN_PLAYER_NINJA)
			return count;
		count++;
	}
	if(mode_player & MSN_PLAYER_SNAKE)
	{
		if(player & MSN_PLAYER_SNAKE)
			return count;
		count++;
	}
	if(mode_player & MSN_PLAYER_PLISKIN)
	{
		if(player & MSN_PLAYER_PLISKIN)
			return count;
		count++;
	}
	if(mode_player & MSN_PLAYER_TUXEDO)
	{
		if(player & MSN_PLAYER_TUXEDO)
			return count;
		count++;
	}
	if(mode_player & MSN_PLAYER_PREVIOUS)
	{
		if(player & MSN_PLAYER_PREVIOUS)
			return count;
		count++;
	}

	ASSERT(0);
	return 0;
}

/*******************************************************************************
 * １位クリアデータの設定
 */
void Msn_Set1stClearData(
	int stage,				/* ステージＩＤ */
	int time_score,			/* タイムのスコア */
	int bullet_score,		/* 残弾数のスコア */
	int sneaking_score,		/* 隠密のスコア（０～３） */
	int no_kill_score)		/* 不殺のスコア（０～１） */
{
	int tmp, tmp2;

	/* タイムのスコア */
	if(time_score > 99999) tmp  = 99999;
	else                   tmp  = time_score;

	/* 残弾数のスコア */
	if(bullet_score > 99999) tmp2 = 99999;
	else                     tmp2 = bullet_score;

	*(MSN_STAGE_DATA(stage) + 3) = ((tmp << 16) & MSN_TIMEMASK_1) | (tmp2 & MSN_BULLETSMASK_1);
	if(tmp  & 0x00010000) { *MSN_STAGE_DATA(stage) |=  MSN_TIMEMASK_2; }
	else                  { *MSN_STAGE_DATA(stage) &= ~MSN_TIMEMASK_2; }
	if(tmp2 & 0x00010000) { *MSN_STAGE_DATA(stage) |=  MSN_BULLETSMASK_2; }
	else                  { *MSN_STAGE_DATA(stage) &= ~MSN_BULLETSMASK_2; }

	/* 隠密のスコア */
	*MSN_STAGE_DATA(stage) &= ~MSN_SNEAKINGMASK;
	if     (sneaking_score == 3) { *MSN_STAGE_DATA(stage) |= 0x0c000000; }
	else if(sneaking_score == 2) { *MSN_STAGE_DATA(stage) |= 0x08000000; }
	else if(sneaking_score == 1) { *MSN_STAGE_DATA(stage) |= 0x04000000; }

	/* 不殺のスコア */
	if(no_kill_score == 1) { *MSN_STAGE_DATA(stage) |=  MSN_NOKILLMASK; }
	else                   { *MSN_STAGE_DATA(stage) &= ~MSN_NOKILLMASK; }

	/* ランダムシード */
	*(MSN_STAGE_DATA(stage) + 1) &= ~MSN_RNDSEEDMASK;
	*(MSN_STAGE_DATA(stage) + 1) |= (((unsigned int)GV_Time) << 24) & MSN_RNDSEEDMASK;
}

/*******************************************************************************
 * １位クリアデータの取得
 */
void Msn_Get1stClearData(
	int stage,				/* ステージＩＤ */
	int *time_score,		/* タイムのスコア */
	int *bullet_score,		/* 残弾数のスコア */
	int *sneaking_score,	/* 隠密のスコア */
	int *no_kill_score,		/* 不殺のスコア */
	int *seed)				/* ランダムシード */
{
	/* タイムのスコア */
	if(time_score != NULL)
	{
		*time_score = ((*(MSN_STAGE_DATA(stage) + 3)) & MSN_TIMEMASK_1) >> 16;
		if((*MSN_STAGE_DATA(stage)) & MSN_TIMEMASK_2)
		{
			*time_score |= 0x00010000;
		}
	}

	/* 残弾数のスコア */
	if(bullet_score != NULL)
	{
		*bullet_score = ((*(MSN_STAGE_DATA(stage) + 3)) & MSN_BULLETSMASK_1);
		if((*MSN_STAGE_DATA(stage)) & MSN_BULLETSMASK_2)
		{
			*bullet_score |= 0x00010000;
		}
	}

	/* 隠密のスコア */
	if(sneaking_score != NULL)
	{
		*sneaking_score = ((*MSN_STAGE_DATA(stage)) & MSN_SNEAKINGMASK) >> 26;
	}

	/* 不殺のスコア */
	if(no_kill_score != NULL)
	{
		*no_kill_score = ((*MSN_STAGE_DATA(stage)) & MSN_NOKILLMASK) >> 28;
	}

	/* ランダムシード */
	if(seed != NULL)
	{
		*seed = ((*(MSN_STAGE_DATA(stage) + 1)) & MSN_RNDSEEDMASK) >> 24;
	}
}

/*******************************************************************************
 * クリアコードの取得
 */
int Msn_GetClearCode(	/* 生成されたクリアコード文字列長 */
	int           stage,				/* ステージＩＤ */
	unsigned char *clear_code,			/* クリアコード取得用バッファ */
	int           clear_code_length)	/* 希望クリアコード文字列長（＜＝バッファ長さ） */
{
	extern u_int ClearCodeCalcStringsCrc(char* pstr);
	extern int   ClearCodeGenerate(u_char* pDst, int code_len, int dst_radix, void* pSrcbit, int bitlen, int seed);

	int  name;
	int  version;
	int  score;
	int  time;
	int  bullets;
	int  sneaking;
	int  no_kill;
	int  seed;
	char input[10];

	name    = ClearCodeCalcStringsCrc((char*)GM_MyName);
	version = *_MSN_SAVE_DATA_VERSION;
	score   = MSN_GET_HISCORE2(stage, 1);
	Msn_Get1stClearData(stage, &time, &bullets, &sneaking, &no_kill, &seed);

#if 0	// デコーダの都合でちょっと並び順変えます MODIFY M.Kobayashi 2002/08/12
	input[0] = (unsigned char)  (name    & 0x000000ff);
	input[1] = (unsigned char)(((version & 0x0000000f) <<  4) | ((stage    & 0x000003c0) >>  6));
	input[2] = (unsigned char)(((stage   & 0x0000003f) <<  2) | ((score    & 0x000c0000) >> 18));
	input[3] = (unsigned char) ((score   & 0x0003fc00) >> 10);
	input[4] = (unsigned char) ((score   & 0x000003fc) >>  2);
	input[5] = (unsigned char)(((score   & 0x00000003) <<  6) | ((time     & 0x0001f800) >> 11));
	input[6] = (unsigned char) ((time    & 0x000007f8) >>  3);
	input[7] = (unsigned char)(((time    & 0x00000007) <<  5) | ((bullets  & 0x0001f000) >> 12));
	input[8] = (unsigned char) ((bullets & 0x00000ff0) >>  4);
	input[9] = (unsigned char)(((bullets & 0x0000000f) <<  3) | ((sneaking & 0x00000003) << 1) | (no_kill & 0x00000001));
#else
	// バージョン抜きました（ビットが足りなくなった）2002/08/19 M.Kobayashi	
	input[0] = (unsigned char)  (name    & 0x000000ff);
	input[1] = (unsigned char)  (stage   & 0x000000ff);
	input[2] = (unsigned char)(((stage   & 0x00000300) >>  8) | ((score    & 0x0000003f) <<  2));
	input[3] = (unsigned char) ((score   & 0x00003fc0) >>  6);
	input[4] = (unsigned char)(((score   & 0x000fc000) >> 14) | ((time     & 0x00000003) <<  6));
	input[5] = (unsigned char) ((time    & 0x000003fc) >>  2);
	input[6] = (unsigned char)(((time    & 0x0001fc00) >> 10) | ((bullets  & 0x00000001) <<  7));
	input[7] = (unsigned char) ((bullets & 0x000001fe) >>  1);
	input[8] = (unsigned char) ((bullets & 0x0001fe00) >>  9);
	input[9] = (unsigned char) ((sneaking& 0x00000003)        | ((no_kill & 0x00000001) << 2));
#endif
#ifndef KP_WINDOWS
//	return ClearCodeGenerate(clear_code, clear_code_length, 26, (void*)input, 79, seed);
	return ClearCodeGenerate(clear_code, clear_code_length, 26, (void*)input, 75, seed);
#else
	return ClearCodeGenerate(CLEARCODE_MODE_MISSIONS,
							clear_code, clear_code_length, 26, (void*)input, 75, seed);
#endif
}


/*******************************************************************************
 * リトライ回数の取得

command ミッションズリトライ回数取得[Msn_GetRetryCount]
 */
int Msn_GetRetryCount(void)
{
	return MSN_RETRY_COUNT;
}

/*******************************************************************************
 * コンティニュー回数の取得

command ミッションズコンティニュー回数取得[Msn_GetContinueCount]
 */
int Msn_GetContinueCount(void)
{
	return MSN_CONTINUE_COUNT;
}

/*******************************************************************************
 * ＶＲウィンドウ状態のリセット

command ＶＲウィンドウ状態のリセット[Msn_ResetQuickWindow]
 */
void Msn_ResetQuickWindow(void)
{
	MSN_2DSTATUS &= ~MSN_2DSTAT_QUICK_WINDOW;
}

/*******************************************************************************
 * コンティニュー回数の取得

command セットステージＩＤ[SetVrStageId] $i:ステージID
 */
void SetVrStageId(void)
{
	VR_STAGE_ID  = GCL_GetNextInt();
	MSN_STAGE_ID = VR_STAGE_ID;
}

/*******************************************************************************
 * クリアレベルの取得

command クリアレベル取得[Msn_GetClearLevel2]

#enum {
	MSN_NOT_CLEARED = 0,	// 未クリア
	MSN_RANKING_1ST,		// １位
	MSN_RANKING_2ND,		// ２位
	MSN_RANKING_3RD,		// ３位
	MSN_CLEARED,			// ４位
};
 */
int Msn_GetClearLevel2(void)
{
	return Msn_GetClearLevel(VR_STAGE_ID);
}

/*******************************************************************************
 * ハイスコアの取得

command ハイスコア取得[Msn_GetHiScore_Scn] $i:順位（１～３）
 */
int Msn_GetHiScore_Scn(void)
{
	int ranking = GCL_GetNextInt();
	if((ranking < 1) || (3 < ranking))
	{
		SY_PRINTF2("Invalid parameter.\n");
		ASSERT(0);
	}

	return MSN_GET_HISCORE2(VR_STAGE_ID, ranking);
}

/*******************************************************************************
 * 上書きフラグの取得

command 上書きフラグ取得[Msn_GetOverwriteFlag_Scn] $i:順位（１～３）
 */
int Msn_GetOverwriteFlag_Scn(void)
{
	int ranking = GCL_GetNextInt();
	if((ranking < 1) || (3 < ranking))
	{
		SY_PRINTF2("Invalid parameter.\n");
		ASSERT(0);
	}

	return ((MSN_GET_HISCORE(VR_STAGE_ID, ranking) & MSN_OVERWRITEFLAG) != 0);
}
