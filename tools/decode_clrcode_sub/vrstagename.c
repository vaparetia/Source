/*
  ＶＲステージＩＤからステージ名を得る
  本編のプログラムと密接に関わっているので注意
  
  2002/08/21 M.Kobayashi
  
  $Id: vrstagename.c,v 1.2 2002/08/21 09:28:40 usr03700 Exp $

 */

#include <stdio.h>
#include "msn_stage_id2_xbox_usa.h"

/* ミッションタイプ */
enum {
	MSN_MISSION_VR = 0,
	MSN_MISSION_ALTERNATIVE,
	MSN_MISSION_MAX,
};
static char* mission_name[] = {
	"VR Missions",
	"Alternative Missions",
	"-"
};

/* モードタイプ */
enum {
	MSN_MODE_SNEAKING = 0,
	MSN_MODE_WEAPON,
	MSN_MODE_FIRST_PERSON_VIEW,
	MSN_MODE_VARIETY,
	MSN_MODE_STREAKING,
	MSN_MODE_BOMB_DISPOSAL,
	MSN_MODE_ELIMINATE,
	MSN_MODE_HOLD_UP,
	MSN_MODE_PHOTOGRAPH,
	MSN_MODE_MAX,
};
static char* mode_name[] = {
	"Sneaking mode", "Weapon mode", "First Person View mode", "Variety mode", "Streaking mode",
	"Bomb Disposal mode", "Eliminate mode", "Hold up mode", "Photograph mode", "-",
};

/* 武器タイプ */
enum {
	MSN_WEAPON_NULL = 0,	// weapon 選択がないモードのときの便宜的な定義
	MSN_WEAPON_SNEAKING = 0,
	MSN_WEAPON_ELIMINATE_ALL,
	MSN_WEAPON_HANDGUN,
	MSN_WEAPON_ASSAULT_RIFLE,
	MSN_WEAPON_C4_CLAYMORE,
	MSN_WEAPON_GRENADE,
	MSN_WEAPON_PSG1,
	MSN_WEAPON_STINGER,
	MSN_WEAPON_NIKITA,
	MSN_WEAPON_HF_BLADE,
	MSN_WEAPON_MAX,
};

static char* weapon_name[] = {
	"Sneaking", "Eliminate all", "Handgun", "Assult rifle", "C4/Claymore",
	"Grenade", "Psg-1", "Stinger", "Nikita", "Hf.blade",
};

static char* player_name[] = {
	"Raiden", "Raiden(ninja)",
	"Snake", "Pliskin", "Snake(Tuxedo)", "Snake(MGS1)"
};

// 本編 yamashita/2D/msn.c からコピー
static int Msn_GetPlayer(	/* プレイヤービット */
		int mode_player,	/* 各モードのプレイヤービット */
		int count)			/* 番号(0～) */
{
	if(mode_player & MSN_PLAYER_RAIDEN  ) {if(count-- == 0) return MSN_PLAYER_RAIDEN  ;}
	if(mode_player & MSN_PLAYER_NINJA   ) {if(count-- == 0) return MSN_PLAYER_NINJA   ;}
	if(mode_player & MSN_PLAYER_SNAKE   ) {if(count-- == 0) return MSN_PLAYER_SNAKE   ;}
	if(mode_player & MSN_PLAYER_PLISKIN ) {if(count-- == 0) return MSN_PLAYER_PLISKIN ;}
	if(mode_player & MSN_PLAYER_TUXEDO  ) {if(count-- == 0) return MSN_PLAYER_TUXEDO  ;}
	if(mode_player & MSN_PLAYER_PREVIOUS) {if(count-- == 0) return MSN_PLAYER_PREVIOUS;}

	return 0;
}

#define ASSERT( i ) return i

static int Msn_GetCurrentStageInfo_XBOX_US(
		int VR_STAGE_ID,
		char *mission,	/* ミッションタイプ */
		char *mode,		/* モードタイプ */
		char *weapon,	/* 武器タイプ */
		char *level,	/* ステージ番号 */
		char *player)	/* プレイヤービット */
{
	if(VR_STAGE_ID <= VRM_END)
	{
		/* ＶＲ　ＭＩＳＳＩＯＮＳ */
		if(mission != NULL) *mission = MSN_MISSION_VR;

		if(VR_STAGE_ID <= VRM_SNK_END)
		{
			/* ＳＮＥＡＫＩＮＧ　ＭＯＤＥ */
			if(mode != NULL) *mode = MSN_MODE_SNEAKING;

			if(VR_STAGE_ID <= VRM_SNK_SNK_END)
			{
				/* ＳＮＥＡＫＩＮＧ */
				if(weapon != NULL) *weapon = MSN_WEAPON_SNEAKING;
				if(level  != NULL) *level  = (VR_STAGE_ID - VRM_SNK_SNK_START) / VRM_SNK_SNK_PLAYER_NUM;
				if(player != NULL) *player = Msn_GetPlayer(VRM_SNK_SNK_PLAYER, (VR_STAGE_ID - VRM_SNK_SNK_START) % VRM_SNK_SNK_PLAYER_NUM);
			}
			else if(VR_STAGE_ID <= VRM_SNK_ELM_END)
			{
				/* ＥＬＩＭＩＮＡＴＥ　ＡＬＬ */
				if(weapon != NULL) *weapon = MSN_WEAPON_ELIMINATE_ALL;
				if(level  != NULL) *level  = (VR_STAGE_ID - VRM_SNK_ELM_START) / VRM_SNK_ELM_PLAYER_NUM;
				if(player != NULL) *player = Msn_GetPlayer(VRM_SNK_ELM_PLAYER, (VR_STAGE_ID - VRM_SNK_ELM_START) % VRM_SNK_ELM_PLAYER_NUM);
			}
			else goto ID_ERROR;
		}
		else if(VR_STAGE_ID <= VRM_WPN_END)
		{
			/* ＷＥＡＰＯＮ　ＭＯＤＥ */
			if(mode != NULL) *mode = MSN_MODE_WEAPON;

			if(VR_STAGE_ID <= VRM_WPN_HGN_END)
			{
				/* ＨＡＮＤＧＵＮ */
				if(weapon != NULL) *weapon = MSN_WEAPON_HANDGUN;
				if(level  != NULL) *level  = (VR_STAGE_ID - VRM_WPN_HGN_START) / VRM_WPN_HGN_PLAYER_NUM;
				if(player != NULL) *player = Msn_GetPlayer(VRM_WPN_HGN_PLAYER, (VR_STAGE_ID - VRM_WPN_HGN_START) % VRM_WPN_HGN_PLAYER_NUM);
			}
			else if(VR_STAGE_ID <= VRM_WPN_ASR_END)
			{
				/* ＡＳＳＡＵＬＴ　ＲＩＦＬＥ */
				if(weapon != NULL) *weapon = MSN_WEAPON_ASSAULT_RIFLE;
				if(level  != NULL) *level  = (VR_STAGE_ID - VRM_WPN_ASR_START) / VRM_WPN_ASR_PLAYER_NUM;
				if(player != NULL) *player = Msn_GetPlayer(VRM_WPN_ASR_PLAYER, (VR_STAGE_ID - VRM_WPN_ASR_START) % VRM_WPN_ASR_PLAYER_NUM);
			}
			else if(VR_STAGE_ID <= VRM_WPN_C4C_END)
			{
				/* Ｃ４／ＣＬＡＹＭＯＲＥ */
				if(weapon != NULL) *weapon = MSN_WEAPON_C4_CLAYMORE;
				if(level  != NULL) *level  = (VR_STAGE_ID - VRM_WPN_C4C_START) / VRM_WPN_C4C_PLAYER_NUM;
				if(player != NULL) *player = Msn_GetPlayer(VRM_WPN_C4C_PLAYER, (VR_STAGE_ID - VRM_WPN_C4C_START) % VRM_WPN_C4C_PLAYER_NUM);
			}
			else if(VR_STAGE_ID <= VRM_WPN_GRN_END)
			{
				/* ＧＲＥＮＡＤＥ */
				if(weapon != NULL) *weapon = MSN_WEAPON_GRENADE;
				if(level  != NULL) *level  = (VR_STAGE_ID - VRM_WPN_GRN_START) / VRM_WPN_GRN_PLAYER_NUM;
				if(player != NULL) *player = Msn_GetPlayer(VRM_WPN_GRN_PLAYER, (VR_STAGE_ID - VRM_WPN_GRN_START) % VRM_WPN_GRN_PLAYER_NUM);
			}
			else if(VR_STAGE_ID <= VRM_WPN_PSG_END)
			{
				/* ＰＳＧ－１ */
				if(weapon != NULL) *weapon = MSN_WEAPON_PSG1;
				if(level  != NULL) *level  = (VR_STAGE_ID - VRM_WPN_PSG_START) / VRM_WPN_PSG_PLAYER_NUM;
				if(player != NULL) *player = Msn_GetPlayer(VRM_WPN_PSG_PLAYER, (VR_STAGE_ID - VRM_WPN_PSG_START) % VRM_WPN_PSG_PLAYER_NUM);
			}
			else if(VR_STAGE_ID <= VRM_WPN_STG_END)
			{
				/* ＳＴＩＮＧＥＲ */
				if(weapon != NULL) *weapon = MSN_WEAPON_STINGER;
				if(level  != NULL) *level  = (VR_STAGE_ID - VRM_WPN_STG_START) / VRM_WPN_STG_PLAYER_NUM;
				if(player != NULL) *player = Msn_GetPlayer(VRM_WPN_STG_PLAYER, (VR_STAGE_ID - VRM_WPN_STG_START) % VRM_WPN_STG_PLAYER_NUM);
			}
			else if(VR_STAGE_ID <= VRM_WPN_NKT_END)
			{
				/* ＮＩＫＩＴＡ */
				if(weapon != NULL) *weapon = MSN_WEAPON_NIKITA;
				if(level  != NULL) *level  = (VR_STAGE_ID - VRM_WPN_NKT_START) / VRM_WPN_NKT_PLAYER_NUM;
				if(player != NULL) *player = Msn_GetPlayer(VRM_WPN_NKT_PLAYER, (VR_STAGE_ID - VRM_WPN_NKT_START) % VRM_WPN_NKT_PLAYER_NUM);
			}
			else if(VR_STAGE_ID <= VRM_WPN_HFB_END)
			{
				/* ＨＦ．ＢＬＡＤＥ／ＮＯ　ＷＥＡＰＯＮ */
				if(weapon != NULL) *weapon = MSN_WEAPON_HF_BLADE;
				if(level  != NULL) *level  = (VR_STAGE_ID - VRM_WPN_HFB_START) / VRM_WPN_HFB_PLAYER_NUM;
				if(player != NULL) *player = Msn_GetPlayer(VRM_WPN_HFB_PLAYER, (VR_STAGE_ID - VRM_WPN_HFB_START) % VRM_WPN_HFB_PLAYER_NUM);
			}
			else goto ID_ERROR;
		}
		else if(VR_STAGE_ID <= VRM_FPV_END)
		{
			/* ＦＩＲＳＴ　ＰＥＲＳＯＮ　ＶＩＥＷ　ＭＯＤＥ */
			if(mode   != NULL) *mode   = MSN_MODE_FIRST_PERSON_VIEW;
			if(level  != NULL) *level  = (VR_STAGE_ID - VRM_FPV_START) / VRM_FPV_PLAYER_NUM;
			if(player != NULL) *player = Msn_GetPlayer(VRM_FPV_PLAYER, (VR_STAGE_ID - VRM_FPV_START) % VRM_FPV_PLAYER_NUM);
		}
		else if(VR_STAGE_ID <= VRM_VRT_END)
		{
			/* ＶＡＲＩＥＴＹ　ＭＯＤＥ */
			if(mode   != NULL) *mode   = MSN_MODE_VARIETY;
			if(level  != NULL) *level  = (VR_STAGE_ID - VRM_VRT_START) / VRM_VRT_PLAYER_NUM;
			if(player != NULL) *player = Msn_GetPlayer(VRM_VRT_PLAYER, (VR_STAGE_ID - VRM_VRT_START) % VRM_VRT_PLAYER_NUM);

			/* 特別処理 */
			if(level != NULL)
			{
				int player2 = Msn_GetPlayer(VRM_VRT_PLAYER, (VR_STAGE_ID - VRM_VRT_START) % VRM_VRT_PLAYER_NUM);

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
		else if(VR_STAGE_ID <= VRM_STR_END)
		{
			/* ＳＴＲＥＡＫＩＮＧ　ＭＯＤＥ */
			if(mode   != NULL) *mode   = MSN_MODE_STREAKING;
			if(level  != NULL) *level  = (VR_STAGE_ID - VRM_STR_START) / VRM_STR_PLAYER_NUM;
			if(player != NULL) *player = Msn_GetPlayer(VRM_STR_PLAYER, (VR_STAGE_ID - VRM_STR_START) % VRM_STR_PLAYER_NUM);
		}
		else goto ID_ERROR;
	}
	else if(VR_STAGE_ID <= ALT_END)
	{
		/* ＡＬＴＥＲＮＡＴＩＶＥ　ＭＩＳＳＩＯＮＳ */
		if(mission != NULL) *mission = MSN_MISSION_ALTERNATIVE;

		if(VR_STAGE_ID <= ALT_BMB_END)
		{
			/* ＢＯＭＢ　ＤＩＳＰＯＳＡＬ　ＭＯＤＥ */
			if(mode   != NULL) *mode   = MSN_MODE_BOMB_DISPOSAL;
			if(level  != NULL) *level  = (VR_STAGE_ID - ALT_BMB_START) / ALT_BMB_PLAYER_NUM;
			if(player != NULL) *player = Msn_GetPlayer(ALT_BMB_PLAYER, (VR_STAGE_ID - ALT_BMB_START) % ALT_BMB_PLAYER_NUM);
		}
		else if(VR_STAGE_ID <= ALT_ELM_END)
		{
			/* ＥＬＩＭＩＮＡＴＥ　ＭＯＤＥ */
			if(mode   != NULL) *mode   = MSN_MODE_ELIMINATE;
			if(level  != NULL) *level  = (VR_STAGE_ID - ALT_ELM_START) / ALT_ELM_PLAYER_NUM;
			if(player != NULL) *player = Msn_GetPlayer(ALT_ELM_PLAYER, (VR_STAGE_ID - ALT_ELM_START) % ALT_ELM_PLAYER_NUM);
		}
		else if(VR_STAGE_ID <= ALT_HLD_END)
		{
			/* ＨＯＬＤ　ＵＰ　ＭＯＤＥ */
			if(mode   != NULL) *mode   = MSN_MODE_HOLD_UP;
			if(level  != NULL) *level  = (VR_STAGE_ID - ALT_HLD_START) / ALT_HLD_PLAYER_NUM;
			if(player != NULL) *player = Msn_GetPlayer(ALT_HLD_PLAYER, (VR_STAGE_ID - ALT_HLD_START) % ALT_HLD_PLAYER_NUM);
		}
		else if(VR_STAGE_ID <= ALT_PHT_END)
		{
			/* ＰＨＯＴＯＧＲＡＰＨ　ＭＯＤＥ */
			if(mode   != NULL) *mode   = MSN_MODE_PHOTOGRAPH;
			if(level  != NULL) *level  = (VR_STAGE_ID - ALT_PHT_START) / ALT_PHT_PLAYER_NUM;
			if(player != NULL) *player = Msn_GetPlayer(ALT_PHT_PLAYER, (VR_STAGE_ID - ALT_PHT_START) % ALT_PHT_PLAYER_NUM);
		}
		else goto ID_ERROR;
	}
	else goto ID_ERROR;

	return 1;

ID_ERROR:
	return 0;
}

int GetVrStageName( int id,
					int platform, int region,
					char* pplayer, char* pmission, char* pmode, char* psubmode, char* plevel)
{
	char mission = 0;
	char mode = 0;
	char weapon = 0;
	char level = 0;
	char player = 0;
	int i;

	strcpy( pplayer, "Unknown" );
	strcpy( pmission, "Unknown" );
	strcpy( pmode, "Unknown" );
	strcpy( psubmode, "Unknown" );
	strcpy( plevel, "Unknown" );
	
	if( !Msn_GetCurrentStageInfo_XBOX_US( id, &mission, &mode, &weapon, &level, &player ) ||
		mission >= MSN_MISSION_MAX ||
		mode >= MSN_MODE_MAX ||
		weapon >= MSN_WEAPON_MAX ||
		(player & MSN_PLAYER_ALL) == 0 ) {
		return 0;
	}

	for( i = 0 ; (1 << i) & MSN_PLAYER_ALL ; ++i ) {
		if( player & (1 << i) ) {
			strcpy( pplayer, player_name[ i ] );
		}
	}
	strcpy( pmission, mission_name[ mission ] );
	strcpy( pmode, mode_name[ mode ] );
	if( mode == MSN_MODE_SNEAKING || mode == MSN_MODE_WEAPON ) {
		strcpy( psubmode, weapon_name[ weapon ] );
	} else {
		*psubmode = '\0';
	}
	sprintf( plevel, "Level %02d", level + 1 );

	// 例外処理
	if( mode == MSN_MODE_STREAKING ) {
		strcpy( pplayer, "X Raiden");
	}
	if( !(player & ( MSN_PLAYER_RAIDEN | MSN_PLAYER_NINJA )) &&
		mode == MSN_MODE_WEAPON && weapon == MSN_WEAPON_HF_BLADE ) {
		strcpy( psubmode, "No weapon" );
	}
	return 1;
}
					
