//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * vr_select - vr_select_wea.c
 * ＶＲセレクト
 * 2002/06/03 S.Yamashita
 * $Id: vr_select_wea.c,v 1.1.1.3 2002/11/19 11:51:48 Yoshizawa1 Exp $
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
int VRSEL_IsSneSneakingPlayable(
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

	ret = VRSEL_SneakingClearLevel2(work, 0, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}
int VRSEL_IsSneEliminateAllPlayable(
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

	ret = VRSEL_SneakingClearLevel2(work, 1, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}
int VRSEL_IsWeaHandgunPlayable(
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

	ret = VRSEL_WeaponClearLevel2(work, 0, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}
int VRSEL_IsWeaAssaultRiflePlayable(
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

	ret = VRSEL_WeaponClearLevel2(work, 1, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}
int VRSEL_IsWeaC4ClaymorePlayable(
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

	ret = VRSEL_WeaponClearLevel2(work, 2, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}
int VRSEL_IsWeaGrenadePlayable(
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

	ret = VRSEL_WeaponClearLevel2(work, 3, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}
int VRSEL_IsWeaPsg1Playable(
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

	ret = VRSEL_WeaponClearLevel2(work, 4, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}
int VRSEL_IsWeaStingerPlayable(
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

	ret = VRSEL_WeaponClearLevel2(work, 5, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}
int VRSEL_IsWeaNikitaPlayable(
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

	ret = VRSEL_WeaponClearLevel2(work, 6, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}
int VRSEL_IsWeaHfBladePlayable(
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

	ret = VRSEL_WeaponClearLevel2(work, 7, work->player, 1); if((0 <= ret) && (ret <= 4)) return 1;

	return 0;
}

/*******************************************************************************
 * カーソルを上へ移動
 */
int VRSEL_Wea_MoveUp_Sne(
	VRSEL_WORK *work,	/* ワーク */
	int        pos)		/* 現在位置 */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	/* 一つ上の項目へ移動 */
	if     ((pos > 0) && (VRSEL_IsSneSneakingPlayable(work)    )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_SNEAKING     , 0); work->weapon = MSN_WEAPON_SNEAKING;      }
	else if((pos < 1) && (VRSEL_IsSneEliminateAllPlayable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_ELIMINATE_ALL, 0); work->weapon = MSN_WEAPON_ELIMINATE_ALL; }
	else return 0;
	work->level = 0;

	GM_SdSet(SD_S_CUR01);
	return 1;
}

/*******************************************************************************
 * カーソルを下へ移動
 */
int VRSEL_Wea_MoveDown_Sne(
	VRSEL_WORK *work,	/* ワーク */
	int        pos)		/* 現在位置 */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	/* 一つ下の項目へ移動 */
	if     ((pos < 1) && (VRSEL_IsSneEliminateAllPlayable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_ELIMINATE_ALL, 0); work->weapon = MSN_WEAPON_ELIMINATE_ALL; }
	else if((pos > 0) && (VRSEL_IsSneSneakingPlayable(work)    )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_SNEAKING     , 0); work->weapon = MSN_WEAPON_SNEAKING;      }
	else return 0;
	work->level = 0;

	GM_SdSet(SD_S_CUR01);
	return 1;
}

/*******************************************************************************
 * カーソルを上へ移動
 */
int VRSEL_Wea_MoveUp_Wea(
	VRSEL_WORK *work,	/* ワーク */
	int        pos)		/* 現在位置 */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	/* 一つ上の項目へ移動 */
	if     ((pos > 6) && (VRSEL_IsWeaNikitaPlayable(work)      )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_NIKITA     , 0); work->weapon = MSN_WEAPON_NIKITA;        }
	else if((pos > 5) && (VRSEL_IsWeaStingerPlayable(work)     )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_STINGER    , 0); work->weapon = MSN_WEAPON_STINGER;       }
	else if((pos > 4) && (VRSEL_IsWeaPsg1Playable(work)        )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_PSG1       , 0); work->weapon = MSN_WEAPON_PSG1;          }
	else if((pos > 3) && (VRSEL_IsWeaGrenadePlayable(work)     )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_GRENADE    , 0); work->weapon = MSN_WEAPON_GRENADE;       }
	else if((pos > 2) && (VRSEL_IsWeaC4ClaymorePlayable(work)  )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_C4_CLAYMORE, 0); work->weapon = MSN_WEAPON_C4_CLAYMORE;   }
	else if((pos > 1) && (VRSEL_IsWeaAssaultRiflePlayable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_ASSAULT_R  , 0); work->weapon = MSN_WEAPON_ASSAULT_RIFLE; }
	else if((pos > 0) && (VRSEL_IsWeaHandgunPlayable(work)     )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_HANDGUN    , 0); work->weapon = MSN_WEAPON_HANDGUN;       }
	else if((pos < 7) && (VRSEL_IsWeaHfBladePlayable(work)     )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_HF_BLADE   , 0); work->weapon = MSN_WEAPON_HF_BLADE;      }
	else if((pos < 6) && (VRSEL_IsWeaNikitaPlayable(work)      )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_NIKITA     , 0); work->weapon = MSN_WEAPON_NIKITA;        }
	else if((pos < 5) && (VRSEL_IsWeaStingerPlayable(work)     )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_STINGER    , 0); work->weapon = MSN_WEAPON_STINGER;       }
	else if((pos < 4) && (VRSEL_IsWeaPsg1Playable(work)        )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_PSG1       , 0); work->weapon = MSN_WEAPON_PSG1;          }
	else if((pos < 3) && (VRSEL_IsWeaGrenadePlayable(work)     )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_GRENADE    , 0); work->weapon = MSN_WEAPON_GRENADE;       }
	else if((pos < 2) && (VRSEL_IsWeaC4ClaymorePlayable(work)  )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_C4_CLAYMORE, 0); work->weapon = MSN_WEAPON_C4_CLAYMORE;   }
	else if((pos < 1) && (VRSEL_IsWeaAssaultRiflePlayable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_ASSAULT_R  , 0); work->weapon = MSN_WEAPON_ASSAULT_RIFLE; }
	else return 0;
	work->level = 0;

	GM_SdSet(SD_S_CUR01);
	return 1;
}

/*******************************************************************************
 * カーソルを下へ移動
 */
int VRSEL_Wea_MoveDown_Wea(
	VRSEL_WORK *work,	/* ワーク */
	int        pos)		/* 現在位置 */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	/* 一つ下の項目へ移動 */
	if     ((pos < 1) && (VRSEL_IsWeaAssaultRiflePlayable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_ASSAULT_R  , 0); work->weapon = MSN_WEAPON_ASSAULT_RIFLE; }
	else if((pos < 2) && (VRSEL_IsWeaC4ClaymorePlayable(work)  )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_C4_CLAYMORE, 0); work->weapon = MSN_WEAPON_C4_CLAYMORE;   }
	else if((pos < 3) && (VRSEL_IsWeaGrenadePlayable(work)     )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_GRENADE    , 0); work->weapon = MSN_WEAPON_GRENADE;       }
	else if((pos < 4) && (VRSEL_IsWeaPsg1Playable(work)        )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_PSG1       , 0); work->weapon = MSN_WEAPON_PSG1;          }
	else if((pos < 5) && (VRSEL_IsWeaStingerPlayable(work)     )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_STINGER    , 0); work->weapon = MSN_WEAPON_STINGER;       }
	else if((pos < 6) && (VRSEL_IsWeaNikitaPlayable(work)      )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_NIKITA     , 0); work->weapon = MSN_WEAPON_NIKITA;        }
	else if((pos < 7) && (VRSEL_IsWeaHfBladePlayable(work)     )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_HF_BLADE   , 0); work->weapon = MSN_WEAPON_HF_BLADE;      }
	else if((pos > 0) && (VRSEL_IsWeaHandgunPlayable(work)     )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_HANDGUN    , 0); work->weapon = MSN_WEAPON_HANDGUN;       }
	else if((pos > 1) && (VRSEL_IsWeaAssaultRiflePlayable(work))) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_ASSAULT_R  , 0); work->weapon = MSN_WEAPON_ASSAULT_RIFLE; }
	else if((pos > 2) && (VRSEL_IsWeaC4ClaymorePlayable(work)  )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_C4_CLAYMORE, 0); work->weapon = MSN_WEAPON_C4_CLAYMORE;   }
	else if((pos > 3) && (VRSEL_IsWeaGrenadePlayable(work)     )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_GRENADE    , 0); work->weapon = MSN_WEAPON_GRENADE;       }
	else if((pos > 4) && (VRSEL_IsWeaPsg1Playable(work)        )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_PSG1       , 0); work->weapon = MSN_WEAPON_PSG1;          }
	else if((pos > 5) && (VRSEL_IsWeaStingerPlayable(work)     )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_STINGER    , 0); work->weapon = MSN_WEAPON_STINGER;       }
	else if((pos > 6) && (VRSEL_IsWeaNikitaPlayable(work)      )) { LOM_SetCurObj(&work->layoutman_2, OBJECT_2_WEA_NIKITA     , 0); work->weapon = MSN_WEAPON_NIKITA;        }
	else return 0;
	work->level = 0;

	GM_SdSet(SD_S_CUR01);
	return 1;
}

/*******************************************************************************
 * 項目を選択
 */
int VRSEL_Wea_Select(
	VRSEL_WORK *work,	/* ワーク */
	int        weapon)	/* 武器 */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	ASSERT(weapon == work->weapon);

	if(weapon == MSN_WEAPON_SNEAKING)
	{
		if(VRSEL_IsSneSneakingPlayable(work) == 0)
			ASSERT(0);

		/* レベル選択に移行 */
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_85, 1, 1, 1); break;
		case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_85, 1, 1, 1); break;
		case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_85, 1, 1, 1); break;
		case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_85, 1, 1, 1); break;
		case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_85, 1, 1, 1); break;
		case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_85, 1, 1, 1); break;
		default:
			ASSERT(0);
			break;
		}
		/* カーソル位置の設定 */
		VRSEL_L2_SetCursorPos(work, 10);
	}
	else if(weapon == MSN_WEAPON_ELIMINATE_ALL)
	{
		if(VRSEL_IsSneEliminateAllPlayable(work) == 0)
			ASSERT(0);

		/* レベル選択に移行 */
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_87, 1, 1, 1); break;
		case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_87, 1, 1, 1); break;
		case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_87, 1, 1, 1); break;
		case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_87, 1, 1, 1); break;
		case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_87, 1, 1, 1); break;
		case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_87, 1, 1, 1); break;
		default:
			ASSERT(0);
			break;
		}
		/* カーソル位置の設定 */
		VRSEL_L2_SetCursorPos(work, 10);
	}
	else if(weapon == MSN_WEAPON_HANDGUN)
	{
		if(VRSEL_IsWeaHandgunPlayable(work) == 0)
			ASSERT(0);

		/* レベル選択に移行 */
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_89, 1, 1, 1); break;
		case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
		case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_89, 1, 1, 1); break;
		case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_89, 1, 1, 1); break;
		case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_89, 1, 1, 1); break;
		case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_89, 1, 1, 1); break;
		default:
			ASSERT(0);
			break;
		}
		/* カーソル位置の設定 */
		VRSEL_L2_SetCursorPos(work, 5);
	}
	else if(weapon == MSN_WEAPON_ASSAULT_RIFLE)
	{
		if(VRSEL_IsWeaAssaultRiflePlayable(work) == 0)
			ASSERT(0);

		/* レベル選択に移行 */
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_91, 1, 1, 1); break;
		case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
		case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_91, 1, 1, 1); break;
		case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_91, 1, 1, 1); break;
		case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_91, 1, 1, 1); break;
		case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_91, 1, 1, 1); break;
		default:
			ASSERT(0);
			break;
		}
		/* カーソル位置の設定 */
		VRSEL_L2_SetCursorPos(work, 5);
	}
	else if(weapon == MSN_WEAPON_C4_CLAYMORE)
	{
		if(VRSEL_IsWeaC4ClaymorePlayable(work) == 0)
			ASSERT(0);

		/* レベル選択に移行 */
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_93, 1, 1, 1); break;
		case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
		case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_93, 1, 1, 1); break;
		case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_93, 1, 1, 1); break;
		case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_93, 1, 1, 1); break;
		case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_93, 1, 1, 1); break;
		default:
			ASSERT(0);
			break;
		}
		/* カーソル位置の設定 */
		VRSEL_L2_SetCursorPos(work, 5);
	}
	else if(weapon == MSN_WEAPON_GRENADE)
	{
		if(VRSEL_IsWeaGrenadePlayable(work) == 0)
			ASSERT(0);

		/* レベル選択に移行 */
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_95, 1, 1, 1); break;
		case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
		case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_95, 1, 1, 1); break;
		case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_95, 1, 1, 1); break;
		case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_95, 1, 1, 1); break;
		case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_95, 1, 1, 1); break;
		default:
			ASSERT(0);
			break;
		}
		/* カーソル位置の設定 */
		VRSEL_L2_SetCursorPos(work, 5);
	}
	else if(weapon == MSN_WEAPON_PSG1)
	{
		if(VRSEL_IsWeaPsg1Playable(work) == 0)
			ASSERT(0);

		/* レベル選択に移行 */
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_97, 1, 1, 1); break;
		case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
		case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_97, 1, 1, 1); break;
		case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_97, 1, 1, 1); break;
		case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_97, 1, 1, 1); break;
		case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_97, 1, 1, 1); break;
		default:
			ASSERT(0);
			break;
		}
		/* カーソル位置の設定 */
		VRSEL_L2_SetCursorPos(work, 5);
	}
	else if(weapon == MSN_WEAPON_STINGER)
	{
		if(VRSEL_IsWeaStingerPlayable(work) == 0)
			ASSERT(0);

		/* レベル選択に移行 */
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_99, 1, 1, 1); break;
		case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
		case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_99, 1, 1, 1); break;
		case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_99, 1, 1, 1); break;
		case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_99, 1, 1, 1); break;
		case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_99, 1, 1, 1); break;
		default:
			ASSERT(0);
			break;
		}
		/* カーソル位置の設定 */
		VRSEL_L2_SetCursorPos(work, 5);
	}
	else if(weapon == MSN_WEAPON_NIKITA)
	{
		if(VRSEL_IsWeaNikitaPlayable(work) == 0)
			ASSERT(0);

		/* レベル選択に移行 */
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_101, 1, 1, 1); break;
		case VRSEL_PLA_NINJA   : ASSERT(0); return 0;
		case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_101, 1, 1, 1); break;
		case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_101, 1, 1, 1); break;
		case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_101, 1, 1, 1); break;
		case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_101, 1, 1, 1); break;
		default:
			ASSERT(0);
			break;
		}
		/* カーソル位置の設定 */
		VRSEL_L2_SetCursorPos(work, 5);
	}
	else if(weapon == MSN_WEAPON_HF_BLADE)
	{
		if(VRSEL_IsWeaHfBladePlayable(work) == 0)
			ASSERT(0);

		/* レベル選択に移行 */
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_103, 1, 1, 1); break;
		case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_103, 1, 1, 1); break;
		case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
		case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_103, 1, 1, 1); break;
		case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_103, 1, 1, 1); break;
		case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_103, 1, 1, 1); break;
		case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_103, 1, 1, 1); break;
		default:
			ASSERT(0);
			break;
		}
		/* カーソル位置の設定 */
		VRSEL_L2_SetCursorPos(work, 5);
	}
	else ASSERT(0);

	/* レコード表示を一時的に消す */
	work->flag |= VRSEL_FLAG_HIDE_RECORD;

	GM_SdSet(SD_S_WIN01);
	return 1;
}

/*******************************************************************************
 * 選択をキャンセル
 */

int VRSEL_Wea_Cancel_1(
	VRSEL_WORK *work)	/* ワーク */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	/* モード選択（ＶＲ）に戻る */
	switch(work->player)
	{
	case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_27, 1, 1, 1); break;
	case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_32, 1, 1, 1); break;
	case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
	case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_27, 1, 1, 1); break;
	case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_32, 1, 1, 1); break;
	case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_32, 1, 1, 1); break;
	case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_32, 1, 1, 1); break;
	default:
		ASSERT(0);
	}
	/* カーソル位置の設定 */
	LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_SNEAKING, 0);
	LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MOD_SNEAKING, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);

	work->flag |= VRSEL_FLAG_REQUEST_MOVIE;

	GM_SdSet(SD_S_V_CANS02);
	return 1;
}

int VRSEL_Wea_Cancel_2(
	VRSEL_WORK *work)	/* ワーク */
{
	if((work->flag & VRSEL_FLAG_INPUT_OK) == 0)
		return 0;

	/* モード選択（ＶＲ）に戻る */
	switch(work->player)
	{
	case VRSEL_PLA_RAIDEN  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_28, 1, 1, 1); break;
	case VRSEL_PLA_NINJA   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_33, 1, 1, 1); break;
	case VRSEL_PLA_X_RAIDEN: ASSERT(0); return 0;
	case VRSEL_PLA_SNAKE   : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_28, 1, 1, 1); break;
	case VRSEL_PLA_PLISKIN : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_33, 1, 1, 1); break;
	case VRSEL_PLA_TUXEDO  : VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_33, 1, 1, 1); break;
	case VRSEL_PLA_PREVIOUS: VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_33, 1, 1, 1); break;
	default:
		ASSERT(0);
	}
	/* カーソル位置の設定 */
	LOM_SetCurObj(&work->layoutman_2, OBJECT_2_MOD_WEAPON, 0);
	LOM_SetIcoPos(&work->layoutman_2, OBJECT_2_MOD_WEAPON, VRSEL_ICO_R, VRSEL_ICO_T, VRDEF_CUR_W, VRDEF_CUR_H, 1);

	work->flag |= VRSEL_FLAG_REQUEST_MOVIE;

	GM_SdSet(SD_S_V_CANS02);
	return 1;
}
