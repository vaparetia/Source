//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * vr_select - vr_select_act.c
 * ＶＲセレクト
 * 2002/06/03 S.Yamashita
 * $Id: vr_select_act.c,v 1.2 2002/12/12 14:25:40 takaki Exp $
 */

/*******************************************************************************
 * include
 */

#include "vr_select.h"

#include "BP_Misc.h"

/*******************************************************************************
 * extern
 */

extern void *NewKeyConSel(int page_num, int init_page, int *ret);

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
 * スプライトのアルファの設定
 */
static void SetSpriteAlpha(
	LAYOUTMAN     *layoutman,		/* レイアウトマネージャ */
	int           strcode,			/* 文字列コード */
	int           active,			/* アクティブフラグ  0:非アクティブ  1:アクティブ */
	int           step,				/* 変化度 */
	unsigned char inactive_alpha,	/* 非アクティブアルファ */
	unsigned char active_alpha,		/* アクティブアルファ */
	int           flag)				/* 表示フラグを変更するか */
{
	SPR_COLOR     sprcol;
	unsigned char alpha;
	int           add;

	{
		SPR_OBJ *spr;
		if((spr = L2D_GetObject(layoutman->layout, strcode)) == NULL)
			return;
	}

	if(active == 0) alpha = inactive_alpha;
	else            alpha = active_alpha;

	LOM_Spr_GetColor(layoutman, strcode, &sprcol);
	{
		if(abs(add = alpha - sprcol.a) <= step)
		                 sprcol.a  = alpha;
		else if(add > 0) sprcol.a += step;
		else             sprcol.a -= step;
	}
	LOM_Spr_SetColor(layoutman, strcode, &sprcol);

	if(flag == 1)
	{
		if(sprcol.a == 0) VRSEL_ShowSprite(layoutman, strcode, 0);
		else              VRSEL_ShowSprite(layoutman, strcode, 1);
	}
}

/*******************************************************************************
 * スプライトのＲＧＢＡの設定
 */
static void SetSpriteABGR(
	LAYOUTMAN    *layoutman,		/* レイアウトマネージャ */
	int          strcode,			/* 文字列コード */
	int          active,			/* アクティブフラグ  0:非アクティブ  1:アクティブ */
	int          step,				/* 変化度 */
	unsigned int inactive_color,	/* 非アクティブ色 */
	unsigned int active_color)		/* アクティブ色 */
{
	SetSpriteColor(layoutman, strcode, active, step, inactive_color, active_color);
	SetSpriteAlpha(layoutman, strcode, active, step, (unsigned char)(inactive_color >> 24), (unsigned char)(active_color >> 24), 1);
}
static void SetSpriteABGR2(
	LAYOUTMAN    *layoutman,		/* レイアウトマネージャ */
	int          strcode,			/* 文字列コード */
	int          active,			/* アクティブフラグ  0:非アクティブ  1:アクティブ */
	int          step,				/* 変化度 */
	unsigned int inactive_color,	/* 非アクティブ色 */
	unsigned int active_color)		/* アクティブ色 */
{
	SetSpriteColor(layoutman, strcode, active, step, inactive_color, active_color);
	SetSpriteAlpha(layoutman, strcode, active, step, (unsigned char)(inactive_color >> 24), (unsigned char)(active_color >> 24), 0);
}

/******************************************************************************
 * クリア率の表示
 */
static void SetClearPercentage(
	VRSEL_WORK *work,		/* ワーク */
	LAYOUTMAN  *layoutman,	/* レイアウトマネージャ */
	int        obj_100,		/* １００の位 */
	int        obj_010,		/* １０の位 */
	int        obj_001,		/* １の位 */
	int        player)		/* プレイヤー */
{
	int percentage;

	percentage = VRSEL_GetClearPercentage(work, player);

	if(percentage == 100)
	{
		VRSEL_ShowSprite(layoutman, obj_100, 1);

		if(layoutman == &work->layoutman_1)
		{
			LOM_SprTex_SetU(layoutman, obj_010, work->tex1_u0);
			LOM_SprTex_SetU(layoutman, obj_001, work->tex1_u0);
		}
		else
		{
			LOM_SprTex_SetU(layoutman, obj_010, work->tex2_u0);
			LOM_SprTex_SetU(layoutman, obj_001, work->tex2_u0);
		}
	}
	else
	{
		VRSEL_ShowSprite(layoutman, obj_100, 0);

		if(layoutman == &work->layoutman_1)
		{
			LOM_SprTex_SetU(layoutman, obj_010, work->tex1_u0 + VRSEL_TEX_WIDTH * (percentage / 10));
			LOM_SprTex_SetU(layoutman, obj_001, work->tex1_u0 + VRSEL_TEX_WIDTH * (percentage % 10));
		}
		else
		{
			LOM_SprTex_SetU(layoutman, obj_010, work->tex2_u0 + VRSEL_TEX_WIDTH * (percentage / 10));
			LOM_SprTex_SetU(layoutman, obj_001, work->tex2_u0 + VRSEL_TEX_WIDTH * (percentage % 10));
		}
	}
}

/******************************************************************************
 * レイアウト１のテキストの設定
 */
static void SetL1Text(
	VRSEL_WORK *work,		/* ワーク */
	int        cur_pos,		/* カーソル位置 */
	int        alp_flag,	/* アルファーフラグ */
	int        bg_flag)		/* 背景フラグ */
{
	int clear_level[9];

	/* raiden */
	if(VRSEL_IsRaidenPlayable(work) == 0)
	{
		/* ％ */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_001_RAIDEN, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_010_RAIDEN, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_100_RAIDEN, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_PCT_RAIDEN, 0);
	}
	else
	{
		/* ％ */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_001_RAIDEN, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_010_RAIDEN, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_100_RAIDEN, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_PCT_RAIDEN, 1);
		SetClearPercentage(work, &work->layoutman_1, OBJECT_1_100_RAIDEN, OBJECT_1_010_RAIDEN, OBJECT_1_001_RAIDEN, VRSEL_PLA_RAIDEN);
	}

	/* ninja */
	if(VRSEL_IsNinjaPlayable(work) == 0)
	{
		/* テキスト */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_NINJA_1, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_NINJA_2, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_NINJA_Q, 1);

		/* ％ */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_001_NINJA, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_010_NINJA, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_100_NINJA, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_PCT_NINJA, 0);
	}
	else
	{
		/* テキスト */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_NINJA_1, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_NINJA_2, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_NINJA_Q, 0);

		/* ％ */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_001_NINJA, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_010_NINJA, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_100_NINJA, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_PCT_NINJA, 1);
		SetClearPercentage(work, &work->layoutman_1, OBJECT_1_100_NINJA, OBJECT_1_010_NINJA, OBJECT_1_001_NINJA, VRSEL_PLA_NINJA);
	}

	/* x-raiden */
	if(VRSEL_IsXRaidenPlayable(work) == 0)
	{
		/* テキスト */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_X_RAIDEN, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_X_RAIDEN_Q, 1);

		/* ％ */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_001_X_RAIDEN, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_010_X_RAIDEN, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_100_X_RAIDEN, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_PCT_X_RAIDEN, 0);
	}
	else
	{
		/* テキスト */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_X_RAIDEN, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_X_RAIDEN_Q, 0);

		/* ％ */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_001_X_RAIDEN, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_010_X_RAIDEN, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_100_X_RAIDEN, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_PCT_X_RAIDEN, 1);
		SetClearPercentage(work, &work->layoutman_1, OBJECT_1_100_X_RAIDEN, OBJECT_1_010_X_RAIDEN, OBJECT_1_001_X_RAIDEN, VRSEL_PLA_X_RAIDEN);
	}

	/* snake */
	if(VRSEL_IsSnakePlayable(work) == 0)
	{
		/* ％ */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_001_SNAKE, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_010_SNAKE, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_100_SNAKE, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_PCT_SNAKE, 0);
	}
	else
	{
		/* ％ */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_001_SNAKE, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_010_SNAKE, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_100_SNAKE, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_PCT_SNAKE, 1);
		SetClearPercentage(work, &work->layoutman_1, OBJECT_1_100_SNAKE, OBJECT_1_010_SNAKE, OBJECT_1_001_SNAKE, VRSEL_PLA_SNAKE);
	}

	/* pliskin */
	if(VRSEL_IsPliskinPlayable(work) == 0)
	{
		/* テキスト */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_PLISKIN, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_PLISKIN_Q, 1);

		/* ％ */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_001_PLISKIN, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_010_PLISKIN, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_100_PLISKIN, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_PCT_PLISKIN, 0);
	}
	else
	{
		/* テキスト */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_PLISKIN, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_PLISKIN_Q, 0);

		/* ％ */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_001_PLISKIN, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_010_PLISKIN, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_100_PLISKIN, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_PCT_PLISKIN, 1);
		SetClearPercentage(work, &work->layoutman_1, OBJECT_1_100_PLISKIN, OBJECT_1_010_PLISKIN, OBJECT_1_001_PLISKIN, VRSEL_PLA_PLISKIN);
	}

	/* tuxedo */
	if(VRSEL_IsTuxedoPlayable(work) == 0)
	{
		/* テキスト */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_TUXEDO_1, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_TUXEDO_2, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_TUXEDO_Q, 1);

		/* ％ */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_001_TUXEDO, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_010_TUXEDO, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_100_TUXEDO, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_PCT_TUXEDO, 0);
	}
	else
	{
		/* テキスト */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_TUXEDO_1, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_TUXEDO_2, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_TUXEDO_Q, 0);

		/* ％ */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_001_TUXEDO, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_010_TUXEDO, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_100_TUXEDO, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_PCT_TUXEDO, 1);
		SetClearPercentage(work, &work->layoutman_1, OBJECT_1_100_TUXEDO, OBJECT_1_010_TUXEDO, OBJECT_1_001_TUXEDO, VRSEL_PLA_TUXEDO);
	}

	/* previous */
	if(VRSEL_IsPreviousPlayable(work) == 0)
	{
		/* テキスト */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_PREVIOUS_1, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_PREVIOUS_2, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_PREVIOUS_Q, 1);

		/* ％ */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_001_PREVIOUS, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_010_PREVIOUS, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_100_PREVIOUS, 0);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_PCT_PREVIOUS, 0);
	}
	else
	{
		/* テキスト */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_PREVIOUS_1, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_PREVIOUS_2, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_TXT_PREVIOUS_Q, 0);

		/* ％ */
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_001_PREVIOUS, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_010_PREVIOUS, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_100_PREVIOUS, 1);
		VRSEL_ShowSprite(&work->layoutman_1, OBJECT_1_PCT_PREVIOUS, 1);
		SetClearPercentage(work, &work->layoutman_1, OBJECT_1_100_PREVIOUS, OBJECT_1_010_PREVIOUS, OBJECT_1_001_PREVIOUS, VRSEL_PLA_PREVIOUS);
	}

	/* クリアレベル */
	/* raiden */
	clear_level[0] = VRSEL_PlayerClearLevel(work, VRSEL_PLA_RAIDEN, 0);
	if((clear_level[0] < 0) || (4 < clear_level[0]))
	{
		clear_level[0] = VRSEL_PlayerClearLevel(work, VRSEL_PLA_RAIDEN, 1);
		if((clear_level[0] < 0) || (4 < clear_level[0]))
			clear_level[0] = 5;
		else
			clear_level[0] = 0;
	}
	/* ninja */
	clear_level[1] = VRSEL_PlayerClearLevel(work, VRSEL_PLA_NINJA, 0);
	if((clear_level[1] < 0) || (4 < clear_level[1]))
	{
		clear_level[1] = VRSEL_PlayerClearLevel(work, VRSEL_PLA_NINJA, 1);
		if((clear_level[1] < 0) || (4 < clear_level[1]))
			clear_level[1] = 5;
		else
			clear_level[1] = 0;
	}
	/* x-raiden */
	clear_level[2] = VRSEL_PlayerClearLevel(work, VRSEL_PLA_X_RAIDEN, 0);
	if((clear_level[2] < 0) || (4 < clear_level[2]))
	{
		clear_level[2] = VRSEL_PlayerClearLevel(work, VRSEL_PLA_X_RAIDEN, 1);
		if((clear_level[2] < 0) || (4 < clear_level[2]))
			clear_level[2] = 5;
		else
			clear_level[2] = 0;
	}
	/* snake */
	clear_level[3] = VRSEL_PlayerClearLevel(work, VRSEL_PLA_SNAKE, 0);
	if((clear_level[3] < 0) || (4 < clear_level[3]))
	{
		clear_level[3] = VRSEL_PlayerClearLevel(work, VRSEL_PLA_SNAKE, 1);
		if((clear_level[3] < 0) || (4 < clear_level[3]))
			clear_level[3] = 5;
		else
			clear_level[3] = 0;
	}
	/* pliskin */
	clear_level[4] = VRSEL_PlayerClearLevel(work, VRSEL_PLA_PLISKIN, 0);
	if((clear_level[4] < 0) || (4 < clear_level[4]))
	{
		clear_level[4] = VRSEL_PlayerClearLevel(work, VRSEL_PLA_PLISKIN, 1);
		if((clear_level[4] < 0) || (4 < clear_level[4]))
			clear_level[4] = 5;
		else
			clear_level[4] = 0;
	}
	/* tuxedo */
	clear_level[5] = VRSEL_PlayerClearLevel(work, VRSEL_PLA_TUXEDO, 0);
	if((clear_level[5] < 0) || (4 < clear_level[5]))
	{
		clear_level[5] = VRSEL_PlayerClearLevel(work, VRSEL_PLA_TUXEDO, 1);
		if((clear_level[5] < 0) || (4 < clear_level[5]))
			clear_level[5] = 5;
		else
			clear_level[5] = 0;
	}
	/* previous */
	clear_level[6] = VRSEL_PlayerClearLevel(work, VRSEL_PLA_PREVIOUS, 0);
	if((clear_level[6] < 0) || (4 < clear_level[6]))
	{
		clear_level[6] = VRSEL_PlayerClearLevel(work, VRSEL_PLA_PREVIOUS, 1);
		if((clear_level[6] < 0) || (4 < clear_level[6]))
			clear_level[6] = 5;
		else
			clear_level[6] = 0;
	}
	clear_level[7] = 0;
	clear_level[8] = 0;

	/* 色の設定 */
	if(bg_flag == 1)
	{
		SetSpriteAlpha(&work->layoutman_1, OBJECT_1_PIC_RAIDEN  , cur_pos == OBJECT_1_TXT_RAIDEN    , 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_1, OBJECT_1_PIC_NINJA   , cur_pos == OBJECT_1_TXT_NINJA_1   , 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_1, OBJECT_1_PIC_X_RAIDEN, cur_pos == OBJECT_1_TXT_X_RAIDEN  , 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_1, OBJECT_1_PIC_SNAKE   , cur_pos == OBJECT_1_TXT_SNAKE     , 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_1, OBJECT_1_PIC_PLISKIN , cur_pos == OBJECT_1_TXT_PLISKIN   , 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_1, OBJECT_1_PIC_TUXEDO  , cur_pos == OBJECT_1_TXT_TUXEDO_1  , 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_1, OBJECT_1_PIC_PREVIOUS, cur_pos == OBJECT_1_TXT_PREVIOUS_1, 16, 0, 128, 1);
	}

	if(alp_flag == 0)
	{
		SetSpriteColor(&work->layoutman_1, OBJECT_1_TXT_RAIDEN    , cur_pos == OBJECT_1_TXT_RAIDEN    , 256, work->i_color[clear_level[0]], work->a_color[clear_level[0]]);
		SetSpriteColor(&work->layoutman_1, OBJECT_1_TXT_NINJA_1   , cur_pos == OBJECT_1_TXT_NINJA_1   , 256, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
		SetSpriteColor(&work->layoutman_1, OBJECT_1_TXT_NINJA_2   , cur_pos == OBJECT_1_TXT_NINJA_1   , 256, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
		SetSpriteColor(&work->layoutman_1, OBJECT_1_TXT_NINJA_Q   , cur_pos == OBJECT_1_TXT_NINJA_1   , 256, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
		SetSpriteColor(&work->layoutman_1, OBJECT_1_TXT_X_RAIDEN  , cur_pos == OBJECT_1_TXT_X_RAIDEN  , 256, work->i_color[clear_level[2]], work->a_color[clear_level[2]]);
		SetSpriteColor(&work->layoutman_1, OBJECT_1_TXT_X_RAIDEN_Q, cur_pos == OBJECT_1_TXT_X_RAIDEN  , 256, work->i_color[clear_level[2]], work->a_color[clear_level[2]]);
		SetSpriteColor(&work->layoutman_1, OBJECT_1_TXT_SNAKE     , cur_pos == OBJECT_1_TXT_SNAKE     , 256, work->i_color[clear_level[3]], work->a_color[clear_level[3]]);
		SetSpriteColor(&work->layoutman_1, OBJECT_1_TXT_PLISKIN   , cur_pos == OBJECT_1_TXT_PLISKIN   , 256, work->i_color[clear_level[4]], work->a_color[clear_level[4]]);
		SetSpriteColor(&work->layoutman_1, OBJECT_1_TXT_PLISKIN_Q , cur_pos == OBJECT_1_TXT_PLISKIN   , 256, work->i_color[clear_level[4]], work->a_color[clear_level[4]]);
		SetSpriteColor(&work->layoutman_1, OBJECT_1_TXT_TUXEDO_1  , cur_pos == OBJECT_1_TXT_TUXEDO_1  , 256, work->i_color[clear_level[5]], work->a_color[clear_level[5]]);
		SetSpriteColor(&work->layoutman_1, OBJECT_1_TXT_TUXEDO_2  , cur_pos == OBJECT_1_TXT_TUXEDO_1  , 256, work->i_color[clear_level[5]], work->a_color[clear_level[5]]);
		SetSpriteColor(&work->layoutman_1, OBJECT_1_TXT_TUXEDO_Q  , cur_pos == OBJECT_1_TXT_TUXEDO_1  , 256, work->i_color[clear_level[5]], work->a_color[clear_level[5]]);
		SetSpriteColor(&work->layoutman_1, OBJECT_1_TXT_PREVIOUS_1, cur_pos == OBJECT_1_TXT_PREVIOUS_1, 256, work->i_color[clear_level[6]], work->a_color[clear_level[6]]);
		SetSpriteColor(&work->layoutman_1, OBJECT_1_TXT_PREVIOUS_2, cur_pos == OBJECT_1_TXT_PREVIOUS_1, 256, work->i_color[clear_level[6]], work->a_color[clear_level[6]]);
		SetSpriteColor(&work->layoutman_1, OBJECT_1_TXT_PREVIOUS_Q, cur_pos == OBJECT_1_TXT_PREVIOUS_1, 256, work->i_color[clear_level[6]], work->a_color[clear_level[6]]);
		SetSpriteColor(&work->layoutman_1, OBJECT_1_TXT_SAVE      , cur_pos == OBJECT_1_TXT_SAVE      , 256, work->i_color[clear_level[7]], work->a_color[clear_level[7]]);
		SetSpriteColor(&work->layoutman_1, OBJECT_1_TXT_EXIT      , cur_pos == OBJECT_1_TXT_EXIT      , 256, work->i_color[clear_level[8]], work->a_color[clear_level[8]]);
	}
	else
	{
		SetSpriteABGR2(&work->layoutman_1, OBJECT_1_TXT_RAIDEN    , cur_pos == OBJECT_1_TXT_RAIDEN    , 16, work->i_color[clear_level[0]], work->a_color[clear_level[0]]);
		SetSpriteABGR2(&work->layoutman_1, OBJECT_1_TXT_NINJA_1   , cur_pos == OBJECT_1_TXT_NINJA_1   , 16, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
		SetSpriteABGR2(&work->layoutman_1, OBJECT_1_TXT_NINJA_2   , cur_pos == OBJECT_1_TXT_NINJA_1   , 16, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
		SetSpriteABGR2(&work->layoutman_1, OBJECT_1_TXT_NINJA_Q   , cur_pos == OBJECT_1_TXT_NINJA_1   , 16, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
		SetSpriteABGR2(&work->layoutman_1, OBJECT_1_TXT_X_RAIDEN  , cur_pos == OBJECT_1_TXT_X_RAIDEN  , 16, work->i_color[clear_level[2]], work->a_color[clear_level[2]]);
		SetSpriteABGR2(&work->layoutman_1, OBJECT_1_TXT_X_RAIDEN_Q, cur_pos == OBJECT_1_TXT_X_RAIDEN  , 16, work->i_color[clear_level[2]], work->a_color[clear_level[2]]);
		SetSpriteABGR2(&work->layoutman_1, OBJECT_1_TXT_SNAKE     , cur_pos == OBJECT_1_TXT_SNAKE     , 16, work->i_color[clear_level[3]], work->a_color[clear_level[3]]);
		SetSpriteABGR2(&work->layoutman_1, OBJECT_1_TXT_PLISKIN   , cur_pos == OBJECT_1_TXT_PLISKIN   , 16, work->i_color[clear_level[4]], work->a_color[clear_level[4]]);
		SetSpriteABGR2(&work->layoutman_1, OBJECT_1_TXT_PLISKIN_Q , cur_pos == OBJECT_1_TXT_PLISKIN   , 16, work->i_color[clear_level[4]], work->a_color[clear_level[4]]);
		SetSpriteABGR2(&work->layoutman_1, OBJECT_1_TXT_TUXEDO_1  , cur_pos == OBJECT_1_TXT_TUXEDO_1  , 16, work->i_color[clear_level[5]], work->a_color[clear_level[5]]);
		SetSpriteABGR2(&work->layoutman_1, OBJECT_1_TXT_TUXEDO_2  , cur_pos == OBJECT_1_TXT_TUXEDO_1  , 16, work->i_color[clear_level[5]], work->a_color[clear_level[5]]);
		SetSpriteABGR2(&work->layoutman_1, OBJECT_1_TXT_TUXEDO_Q  , cur_pos == OBJECT_1_TXT_TUXEDO_1  , 16, work->i_color[clear_level[5]], work->a_color[clear_level[5]]);
		SetSpriteABGR2(&work->layoutman_1, OBJECT_1_TXT_PREVIOUS_1, cur_pos == OBJECT_1_TXT_PREVIOUS_1, 16, work->i_color[clear_level[6]], work->a_color[clear_level[6]]);
		SetSpriteABGR2(&work->layoutman_1, OBJECT_1_TXT_PREVIOUS_2, cur_pos == OBJECT_1_TXT_PREVIOUS_1, 16, work->i_color[clear_level[6]], work->a_color[clear_level[6]]);
		SetSpriteABGR2(&work->layoutman_1, OBJECT_1_TXT_PREVIOUS_Q, cur_pos == OBJECT_1_TXT_PREVIOUS_1, 16, work->i_color[clear_level[6]], work->a_color[clear_level[6]]);
		SetSpriteABGR2(&work->layoutman_1, OBJECT_1_TXT_SAVE      , cur_pos == OBJECT_1_TXT_SAVE      , 16, work->i_color[clear_level[7]], work->a_color[clear_level[7]]);
		SetSpriteABGR2(&work->layoutman_1, OBJECT_1_TXT_EXIT      , cur_pos == OBJECT_1_TXT_EXIT      , 16, work->i_color[clear_level[8]], work->a_color[clear_level[8]]);
	}
}

/******************************************************************************
 * プレイヤー選択画面
 */
static void SetPlayerScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int cur_pos;

	/* カーソル位置 */
	cur_pos = LOM_GetCurObj(&work->layoutman_1);

	/* レイアウト１のテキストの設定 */
	SetL1Text(work, cur_pos, 1, 1);
}

/******************************************************************************
 * Ｌ１開く画面
 */
static void SetL1OpenScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int cur_pos = 0;

	/* カーソル位置 */
	switch(work->player)
	{
	case VRSEL_PLA_NO_PLAYER:
	case VRSEL_PLA_RAIDEN   : cur_pos = OBJECT_1_TXT_RAIDEN;     break;
	case VRSEL_PLA_NINJA    : cur_pos = OBJECT_1_TXT_NINJA_1;    break;
	case VRSEL_PLA_X_RAIDEN : cur_pos = OBJECT_1_TXT_X_RAIDEN;   break;
	case VRSEL_PLA_SNAKE    : cur_pos = OBJECT_1_TXT_SNAKE;      break;
	case VRSEL_PLA_PLISKIN  : cur_pos = OBJECT_1_TXT_PLISKIN;    break;
	case VRSEL_PLA_TUXEDO   : cur_pos = OBJECT_1_TXT_TUXEDO_1;   break;
	case VRSEL_PLA_PREVIOUS : cur_pos = OBJECT_1_TXT_PREVIOUS_1; break;
	case VRSEL_SAVE         : cur_pos = OBJECT_1_TXT_SAVE;       break;
	case VRSEL_EXIT         : cur_pos = OBJECT_1_TXT_EXIT;       break;
	default:
		ASSERT(0);
		break;
	}

	/* レイアウト１のテキストの設定 */
	if(work->count > 0)		/* 最初のフレームだけ処理しない */
		SetL1Text(work, cur_pos, 0, 0);
}

/******************************************************************************
 * Ｌ１閉じる画面
 */
static void SetL1CloseScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int cur_pos = 0;

	/* カーソル位置 */
	switch(work->player)
	{
	case VRSEL_PLA_NO_PLAYER:
	case VRSEL_PLA_RAIDEN   : cur_pos = OBJECT_1_TXT_RAIDEN;     break;
	case VRSEL_PLA_NINJA    : cur_pos = OBJECT_1_TXT_NINJA_1;    break;
	case VRSEL_PLA_X_RAIDEN : cur_pos = OBJECT_1_TXT_X_RAIDEN;   break;
	case VRSEL_PLA_SNAKE    : cur_pos = OBJECT_1_TXT_SNAKE;      break;
	case VRSEL_PLA_PLISKIN  : cur_pos = OBJECT_1_TXT_PLISKIN;    break;
	case VRSEL_PLA_TUXEDO   : cur_pos = OBJECT_1_TXT_TUXEDO_1;   break;
	case VRSEL_PLA_PREVIOUS : cur_pos = OBJECT_1_TXT_PREVIOUS_1; break;
	case VRSEL_SAVE         : cur_pos = OBJECT_1_TXT_SAVE;       break;
	case VRSEL_EXIT         : cur_pos = OBJECT_1_TXT_EXIT;       break;
	default:
		ASSERT(0);
		break;
	}

	/* レイアウト１のテキストの設定 */
	SetL1Text(work, cur_pos, 0, 0);
}

/******************************************************************************
 * レイアウト２のテキストの設定
 */
static void SetL2Text(
	VRSEL_WORK *work)	/* ワーク */
{
	/* ％ */
	SetClearPercentage(work, &work->layoutman_2, OBJECT_2_PCT_100, OBJECT_2_PCT_010, OBJECT_2_PCT_001, work->player);

	/* variety */
	if(work->player != VRSEL_PLA_X_RAIDEN)
	{
		if(VRSEL_IsVarietyPlayable(work) == 0)
		{
			VRSEL_ShowSprite(&work->layoutman_2, OBJECT_2_MOD_VARIETY, 0);
		}
		else
		{
			VRSEL_ShowSprite(&work->layoutman_2, OBJECT_2_MOD_QUESTION_1, 0);
		}
	}

	/* photograph */
	if((work->player == VRSEL_PLA_RAIDEN) || (work->player == VRSEL_PLA_SNAKE))
	{
		if(VRSEL_IsPhotographPlayable(work) == 0)
		{
			VRSEL_ShowSprite(&work->layoutman_2, OBJECT_2_MOD_PHOTOGRAPH, 0);
		}
		else
		{
			VRSEL_ShowSprite(&work->layoutman_2, OBJECT_2_MOD_QUESTION_2, 0);
		}
	}
}

/******************************************************************************
 * ミッションのテキストの設定
 */
static void SetMissionsText(
	VRSEL_WORK *work,		/* ワーク */
	int        cur_pos,		/* カーソル位置 */
	int        alp_flag,	/* アルファーフラグ */
	int        step,		/* 変化度 */
	int        all_item)	/* 全項目処理するフラグ */
{
	int clear_level[2];

	/* クリアレベル */
	/* vr */
	if((all_item == 1) || (cur_pos == OBJECT_2_MIS_VR))
	{
		clear_level[0] = VRSEL_VRClearLevel(work, work->player, 0);
		if((clear_level[0] < 0) || (4 < clear_level[0]))
		{
			clear_level[0] = VRSEL_VRClearLevel(work, work->player, 1);
			if((clear_level[0] < 0) || (4 < clear_level[0]))
				clear_level[0] = 5;
			else
				clear_level[0] = 0;
		}
	}
	/* alternative */
	if((all_item == 1) || (cur_pos == OBJECT_2_MIS_ALTERNATIVE))
	{
		clear_level[1] = VRSEL_AlternativeClearLevel(work, work->player, 0);
		if((clear_level[1] < 0) || (4 < clear_level[1]))
		{
			clear_level[1] = VRSEL_AlternativeClearLevel(work, work->player, 1);
			if((clear_level[1] < 0) || (4 < clear_level[1]))
				clear_level[1] = 5;
			else
				clear_level[1] = 0;
		}
	}

	/* 色の設定 */
	if(alp_flag == 0)
	{
		if((all_item == 1) || (cur_pos == OBJECT_2_MIS_VR))
		{
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MIS_VR         , 0                                  , step, work->i_color[clear_level[0]], work->a_color[clear_level[0]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MIS_VR         , cur_pos == OBJECT_2_MIS_VR         , step, work->i_color[clear_level[0]], work->a_color[clear_level[0]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_MIS_ALTERNATIVE))
		{
			if(work->player != VRSEL_PLA_X_RAIDEN)
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MIS_ALTERNATIVE, 0                                  , step, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MIS_ALTERNATIVE, cur_pos == OBJECT_2_MIS_ALTERNATIVE, step, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
				}
			}
		}
	}
	else
	{
		if((all_item == 1) || (cur_pos == OBJECT_2_MIS_VR))
		{
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_MIS_VR         , cur_pos == OBJECT_2_MIS_VR         , step, work->i_color[clear_level[0]], work->a_color[clear_level[0]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_MIS_ALTERNATIVE))
		{
			if(work->player != VRSEL_PLA_X_RAIDEN)
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_MIS_ALTERNATIVE, cur_pos == OBJECT_2_MIS_ALTERNATIVE, step, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
				}
			}
		}
	}
}
static void SetMissionsText2(
	VRSEL_WORK *work,		/* ワーク */
	int        cur_pos)		/* カーソル位置 */
{
	SetMissionsText(work, OBJECT_2_MIS_VR         , cur_pos == OBJECT_2_MIS_VR         , 256, 2);
	SetMissionsText(work, OBJECT_2_MIS_ALTERNATIVE, cur_pos == OBJECT_2_MIS_ALTERNATIVE, 256, 2);
}

/******************************************************************************
 * モード（ＶＲ）のテキストの設定
 */
static void SetVRText(
	VRSEL_WORK *work,		/* ワーク */
	int        cur_pos,		/* カーソル位置 */
	int        alp_flag,	/* アルファーフラグ */
	int        step,		/* 変化度 */
	int        all_item)	/* 全項目処理するフラグ */
{
	int clear_level[5];

	/* クリアレベル */
	/* sneaking */
	if((all_item == 1) || (cur_pos == OBJECT_2_MOD_SNEAKING))
	{
		clear_level[0] = VRSEL_SneakingClearLevel(work, work->player, 0);
		if((clear_level[0] < 0) || (4 < clear_level[0]))
		{
			clear_level[0] = VRSEL_SneakingClearLevel(work, work->player, 1);
			if((clear_level[0] < 0) || (4 < clear_level[0]))
				clear_level[0] = 5;
			else
				clear_level[0] = 0;
		}
	}
	/* weapon */
	if((all_item == 1) || (cur_pos == OBJECT_2_MOD_WEAPON))
	{
		clear_level[1] = VRSEL_WeaponClearLevel(work, work->player, 0);
		if((clear_level[1] < 0) || (4 < clear_level[1]))
		{
			clear_level[1] = VRSEL_WeaponClearLevel(work, work->player, 1);
			if((clear_level[1] < 0) || (4 < clear_level[1]))
				clear_level[1] = 5;
			else
				clear_level[1] = 0;
		}
	}
	/* first person view */
	if((all_item == 1) || (cur_pos == OBJECT_2_MOD_FSV_1))
	{
		clear_level[2] = VRSEL_FPVClearLevel(work, work->player, 0);
		if((clear_level[2] < 0) || (4 < clear_level[2]))
		{
			clear_level[2] = VRSEL_FPVClearLevel(work, work->player, 1);
			if((clear_level[2] < 0) || (4 < clear_level[2]))
				clear_level[2] = 5;
			else
				clear_level[2] = 0;
		}
	}
	/* variety */
	if((all_item == 1) || (cur_pos == OBJECT_2_MOD_VARIETY))
	{
		clear_level[3] = VRSEL_VarietyClearLevel(work, work->player, 0);
		if((clear_level[3] < 0) || (4 < clear_level[3]))
		{
			clear_level[3] = VRSEL_VarietyClearLevel(work, work->player, 1);
			if((clear_level[3] < 0) || (4 < clear_level[3]))
				clear_level[3] = 5;
			else
				clear_level[3] = 0;
		}
	}
	/* streaking */
	if((all_item == 1) || (cur_pos == OBJECT_2_MOD_STREAKING))
	{
		clear_level[4] = VRSEL_StreakingClearLevel(work, work->player, 0);
		if((clear_level[4] < 0) || (4 < clear_level[4]))
		{
			clear_level[4] = VRSEL_StreakingClearLevel(work, work->player, 1);
			if((clear_level[4] < 0) || (4 < clear_level[4]))
				clear_level[4] = 5;
			else
				clear_level[4] = 0;
		}
	}

	/* 色の設定 */
	if(alp_flag == 0)
	{
		if((all_item == 1) || (cur_pos == OBJECT_2_MOD_SNEAKING))
		{
			if(work->player != VRSEL_PLA_X_RAIDEN)
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_SNEAKING , 0                                , step, work->i_color[clear_level[0]], work->a_color[clear_level[0]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_SNEAKING , cur_pos == OBJECT_2_MOD_SNEAKING , step, work->i_color[clear_level[0]], work->a_color[clear_level[0]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_MOD_WEAPON))
		{
			if(work->player != VRSEL_PLA_X_RAIDEN)
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_WEAPON   , 0                                , step, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_WEAPON   , cur_pos == OBJECT_2_MOD_WEAPON   , step, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_MOD_FSV_1))
		{
			if((work->player == VRSEL_PLA_RAIDEN) || (work->player == VRSEL_PLA_SNAKE))
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_FSV_1    , 0                                , step, work->i_color[clear_level[2]], work->a_color[clear_level[2]]);
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_FSV_2    , 0                                , step, work->i_color[clear_level[2]], work->a_color[clear_level[2]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_FSV_1    , cur_pos == OBJECT_2_MOD_FSV_1    , step, work->i_color[clear_level[2]], work->a_color[clear_level[2]]);
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_FSV_2    , cur_pos == OBJECT_2_MOD_FSV_1    , step, work->i_color[clear_level[2]], work->a_color[clear_level[2]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_MOD_VARIETY))
		{
			if(work->player != VRSEL_PLA_X_RAIDEN)
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_VARIETY   , 0                               , step, work->i_color[clear_level[3]], work->a_color[clear_level[3]]);
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_QUESTION_1, 0                               , step, work->i_color[clear_level[3]], work->a_color[clear_level[3]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_VARIETY   , cur_pos == OBJECT_2_MOD_VARIETY , step, work->i_color[clear_level[3]], work->a_color[clear_level[3]]);
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_QUESTION_1, cur_pos == OBJECT_2_MOD_VARIETY , step, work->i_color[clear_level[3]], work->a_color[clear_level[3]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_MOD_STREAKING))
		{
			if(work->player == VRSEL_PLA_X_RAIDEN)
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_STREAKING, 0                                , step, work->i_color[clear_level[4]], work->a_color[clear_level[4]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_STREAKING, cur_pos == OBJECT_2_MOD_STREAKING, step, work->i_color[clear_level[4]], work->a_color[clear_level[4]]);
				}
			}
		}
	}
	else
	{
		if((all_item == 1) || (cur_pos == OBJECT_2_MOD_SNEAKING))
		{
			if(work->player != VRSEL_PLA_X_RAIDEN)
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_MOD_SNEAKING , cur_pos == OBJECT_2_MOD_SNEAKING , step, work->i_color[clear_level[0]], work->a_color[clear_level[0]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_MOD_WEAPON))
		{
			if(work->player != VRSEL_PLA_X_RAIDEN)
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_MOD_WEAPON   , cur_pos == OBJECT_2_MOD_WEAPON   , step, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_MOD_FSV_1))
		{
			if((work->player == VRSEL_PLA_RAIDEN) || (work->player == VRSEL_PLA_SNAKE))
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_MOD_FSV_1    , cur_pos == OBJECT_2_MOD_FSV_1    , step, work->i_color[clear_level[2]], work->a_color[clear_level[2]]);
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_MOD_FSV_2    , cur_pos == OBJECT_2_MOD_FSV_1    , step, work->i_color[clear_level[2]], work->a_color[clear_level[2]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_MOD_VARIETY))
		{
			if(work->player != VRSEL_PLA_X_RAIDEN)
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_MOD_VARIETY   , cur_pos == OBJECT_2_MOD_VARIETY , step, work->i_color[clear_level[3]], work->a_color[clear_level[3]]);
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_MOD_QUESTION_1, cur_pos == OBJECT_2_MOD_VARIETY , step, work->i_color[clear_level[3]], work->a_color[clear_level[3]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_MOD_STREAKING))
		{
			if(work->player == VRSEL_PLA_X_RAIDEN)
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_MOD_STREAKING, cur_pos == OBJECT_2_MOD_STREAKING, step, work->i_color[clear_level[4]], work->a_color[clear_level[4]]);
				}
			}
		}
	}
}
static void SetVRText2(
	VRSEL_WORK *work,		/* ワーク */
	int        cur_pos)		/* カーソル位置 */
{
	SetVRText(work, OBJECT_2_MOD_SNEAKING , cur_pos == OBJECT_2_MOD_SNEAKING , 256, 2);
	SetVRText(work, OBJECT_2_MOD_WEAPON   , cur_pos == OBJECT_2_MOD_WEAPON   , 256, 2);
	SetVRText(work, OBJECT_2_MOD_FSV_1    , cur_pos == OBJECT_2_MOD_FSV_1    , 256, 2);
	SetVRText(work, OBJECT_2_MOD_VARIETY  , cur_pos == OBJECT_2_MOD_VARIETY  , 256, 2);
	SetVRText(work, OBJECT_2_MOD_STREAKING, cur_pos == OBJECT_2_MOD_STREAKING, 256, 2);
}

/******************************************************************************
 * モード（ＡＬＴＥＲＮＡＴＩＶＥ）のテキストの設定
 */
static void SetAlternativeText(
	VRSEL_WORK *work,		/* ワーク */
	int        cur_pos,		/* カーソル位置 */
	int        alp_flag,	/* アルファーフラグ */
	int        step,		/* 変化度 */
	int        all_item)	/* 全項目処理するフラグ */
{
	int clear_level[4];

	/* クリアレベル */
	/* bomb disposal */
	if((all_item == 1) || (cur_pos == OBJECT_2_MOD_BOMB_D))
	{
		clear_level[0] = VRSEL_BombClearLevel(work, work->player, 0);
		if((clear_level[0] < 0) || (4 < clear_level[0]))
		{
			clear_level[0] = VRSEL_BombClearLevel(work, work->player, 1);
			if((clear_level[0] < 0) || (4 < clear_level[0]))
				clear_level[0] = 5;
			else
				clear_level[0] = 0;
		}
	}
	/* eliminate */
	if((all_item == 1) || (cur_pos == OBJECT_2_MOD_ELIMINATE))
	{
		clear_level[1] = VRSEL_EliminateClearLevel(work, work->player, 0);
		if((clear_level[1] < 0) || (4 < clear_level[1]))
		{
			clear_level[1] = VRSEL_EliminateClearLevel(work, work->player, 1);
			if((clear_level[1] < 0) || (4 < clear_level[1]))
				clear_level[1] = 5;
			else
				clear_level[1] = 0;
		}
	}
	/* hold up */
	if((all_item == 1) || (cur_pos == OBJECT_2_MOD_HOLD_UP))
	{
		clear_level[2] = VRSEL_HoldupClearLevel(work, work->player, 0);
		if((clear_level[2] < 0) || (4 < clear_level[2]))
		{
			clear_level[2] = VRSEL_HoldupClearLevel(work, work->player, 1);
			if((clear_level[2] < 0) || (4 < clear_level[2]))
				clear_level[2] = 5;
			else
				clear_level[2] = 0;
		}
	}
	/* photograph */
	if((all_item == 1) || (cur_pos == OBJECT_2_MOD_PHOTOGRAPH))
	{
		clear_level[3] = VRSEL_PhotographClearLevel(work, work->player, 0);
		if((clear_level[3] < 0) || (4 < clear_level[3]))
		{
			clear_level[3] = VRSEL_PhotographClearLevel(work, work->player, 1);
			if((clear_level[3] < 0) || (4 < clear_level[3]))
				clear_level[3] = 5;
			else
				clear_level[3] = 0;
		}
	}

	/* 色の設定 */
	if(alp_flag == 0)
	{
		if((all_item == 1) || (cur_pos == OBJECT_2_MOD_BOMB_D))
		{
			if(work->player != VRSEL_PLA_X_RAIDEN)
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_BOMB_D    , 0                                 , step, work->i_color[clear_level[0]], work->a_color[clear_level[0]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_BOMB_D    , cur_pos == OBJECT_2_MOD_BOMB_D    , step, work->i_color[clear_level[0]], work->a_color[clear_level[0]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_MOD_ELIMINATE))
		{
			if(work->player != VRSEL_PLA_X_RAIDEN)
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_ELIMINATE , 0                                 , step, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_ELIMINATE , cur_pos == OBJECT_2_MOD_ELIMINATE , step, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_MOD_HOLD_UP))
		{
			if((work->player != VRSEL_PLA_NINJA) && (work->player != VRSEL_PLA_X_RAIDEN))
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_HOLD_UP   , 0                                 , step, work->i_color[clear_level[2]], work->a_color[clear_level[2]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_HOLD_UP   , cur_pos == OBJECT_2_MOD_HOLD_UP   , step, work->i_color[clear_level[2]], work->a_color[clear_level[2]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_MOD_PHOTOGRAPH))
		{
			if((work->player == VRSEL_PLA_RAIDEN) || (work->player == VRSEL_PLA_SNAKE))
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_PHOTOGRAPH, 0                                 , step, work->i_color[clear_level[3]], work->a_color[clear_level[3]]);
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_QUESTION_2, 0                                 , step, work->i_color[clear_level[3]], work->a_color[clear_level[3]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_PHOTOGRAPH, cur_pos == OBJECT_2_MOD_PHOTOGRAPH, step, work->i_color[clear_level[3]], work->a_color[clear_level[3]]);
					SetSpriteColor(&work->layoutman_2, OBJECT_2_MOD_QUESTION_2, cur_pos == OBJECT_2_MOD_PHOTOGRAPH, step, work->i_color[clear_level[3]], work->a_color[clear_level[3]]);
				}
			}
		}
	}
	else
	{
		if((all_item == 1) || (cur_pos == OBJECT_2_MOD_BOMB_D))
		{
			if(work->player != VRSEL_PLA_X_RAIDEN)
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_MOD_BOMB_D    , cur_pos == OBJECT_2_MOD_BOMB_D    , step, work->i_color[clear_level[0]], work->a_color[clear_level[0]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_MOD_ELIMINATE))
		{
			if(work->player != VRSEL_PLA_X_RAIDEN)
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_MOD_ELIMINATE , cur_pos == OBJECT_2_MOD_ELIMINATE , step, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_MOD_HOLD_UP))
		{
			if((work->player != VRSEL_PLA_NINJA) && (work->player != VRSEL_PLA_X_RAIDEN))
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_MOD_HOLD_UP   , cur_pos == OBJECT_2_MOD_HOLD_UP   , step, work->i_color[clear_level[2]], work->a_color[clear_level[2]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_MOD_PHOTOGRAPH))
		{
			if((work->player == VRSEL_PLA_RAIDEN) || (work->player == VRSEL_PLA_SNAKE))
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_MOD_PHOTOGRAPH, cur_pos == OBJECT_2_MOD_PHOTOGRAPH, step, work->i_color[clear_level[3]], work->a_color[clear_level[3]]);
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_MOD_QUESTION_2, cur_pos == OBJECT_2_MOD_PHOTOGRAPH, step, work->i_color[clear_level[3]], work->a_color[clear_level[3]]);
				}
			}
		}
	}
}
static void SetAlternativeText2(
	VRSEL_WORK *work,		/* ワーク */
	int        cur_pos)		/* カーソル位置 */
{
	SetAlternativeText(work, OBJECT_2_MOD_BOMB_D    , cur_pos == OBJECT_2_MOD_BOMB_D    , 256, 2);
	SetAlternativeText(work, OBJECT_2_MOD_ELIMINATE , cur_pos == OBJECT_2_MOD_ELIMINATE , 256, 2);
	SetAlternativeText(work, OBJECT_2_MOD_HOLD_UP   , cur_pos == OBJECT_2_MOD_HOLD_UP   , 256, 2);
	SetAlternativeText(work, OBJECT_2_MOD_PHOTOGRAPH, cur_pos == OBJECT_2_MOD_PHOTOGRAPH, 256, 2);
}

/******************************************************************************
 * 武器（ＳＮＥＡＫＩＮＧ）のテキストの設定
 */
static void SetSneakingText(
	VRSEL_WORK *work,		/* ワーク */
	int        cur_pos,		/* カーソル位置 */
	int        alp_flag,	/* アルファーフラグ */
	int        step,		/* 変化度 */
	int        all_item)	/* 全項目処理するフラグ */
{
	int clear_level[2];

	/* クリアレベル */
	/* sneaking */
	if((all_item == 1) || (cur_pos == OBJECT_2_WEA_SNEAKING))
	{
		clear_level[0] = VRSEL_SneakingClearLevel2(work, 0, work->player, 0);
		if((clear_level[0] < 0) || (4 < clear_level[0]))
		{
			clear_level[0] = VRSEL_SneakingClearLevel2(work, 0, work->player, 1);
			if((clear_level[0] < 0) || (4 < clear_level[0]))
				clear_level[0] = 5;
			else
				clear_level[0] = 0;
		}
	}
	/* eliminate all */
	if((all_item == 1) || (cur_pos == OBJECT_2_WEA_ELIMINATE_ALL))
	{
		clear_level[1] = VRSEL_SneakingClearLevel2(work, 1, work->player, 0);
		if((clear_level[1] < 0) || (4 < clear_level[1]))
		{
			clear_level[1] = VRSEL_SneakingClearLevel2(work, 1, work->player, 1);
			if((clear_level[1] < 0) || (4 < clear_level[1]))
				clear_level[1] = 5;
			else
				clear_level[1] = 0;
		}
	}

	/* 色の設定 */
	if(alp_flag == 0)
	{
		if((all_item == 1) || (cur_pos == OBJECT_2_WEA_SNEAKING))
		{
			if(work->player != VRSEL_PLA_X_RAIDEN)
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_WEA_SNEAKING     , 0                                    , step, work->i_color[clear_level[0]], work->a_color[clear_level[0]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_WEA_SNEAKING     , cur_pos == OBJECT_2_WEA_SNEAKING     , step, work->i_color[clear_level[0]], work->a_color[clear_level[0]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_WEA_ELIMINATE_ALL))
		{
			if(work->player != VRSEL_PLA_X_RAIDEN)
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_WEA_ELIMINATE_ALL, 0                                    , step, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_WEA_ELIMINATE_ALL, cur_pos == OBJECT_2_WEA_ELIMINATE_ALL, step, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
				}
			}
		}
	}
	else
	{
		if((all_item == 1) || (cur_pos == OBJECT_2_WEA_SNEAKING))
		{
			if(work->player != VRSEL_PLA_X_RAIDEN)
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_WEA_SNEAKING     , cur_pos == OBJECT_2_WEA_SNEAKING     , step, work->i_color[clear_level[0]], work->a_color[clear_level[0]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_WEA_ELIMINATE_ALL))
		{
			if(work->player != VRSEL_PLA_X_RAIDEN)
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_WEA_ELIMINATE_ALL, cur_pos == OBJECT_2_WEA_ELIMINATE_ALL, step, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
				}
			}
		}
	}
}
static void SetSneakingText2(
	VRSEL_WORK *work,		/* ワーク */
	int        cur_pos)		/* カーソル位置 */
{
	SetSneakingText(work, OBJECT_2_WEA_SNEAKING     , cur_pos == OBJECT_2_WEA_SNEAKING     , 256, 2);
	SetSneakingText(work, OBJECT_2_WEA_ELIMINATE_ALL, cur_pos == OBJECT_2_WEA_ELIMINATE_ALL, 256, 2);
}

/******************************************************************************
 * 武器（ＷＥＡＰＯＮ）のテキスト設定
 */
static void SetWeaponText(
	VRSEL_WORK *work,		/* ワーク */
	int        cur_pos,		/* カーソル位置 */
	int        alp_flag,	/* アルファーフラグ */
	int        step,		/* 変化度 */
	int        all_item)	/* 全項目処理するフラグ */
{
	int clear_level[8];

	/* クリアレベル */
	/* handgun */
	if((all_item == 1) || (cur_pos == OBJECT_2_WEA_HANDGUN))
	{
		clear_level[0] = VRSEL_WeaponClearLevel2(work, 0, work->player, 0);
		if((clear_level[0] < 0) || (4 < clear_level[0]))
		{
			clear_level[0] = VRSEL_WeaponClearLevel2(work, 0, work->player, 1);
			if((clear_level[0] < 0) || (4 < clear_level[0]))
				clear_level[0] = 5;
			else
				clear_level[0] = 0;
		}
	}
	/* assault rifle */
	if((all_item == 1) || (cur_pos == OBJECT_2_WEA_ASSAULT_R))
	{
		clear_level[1] = VRSEL_WeaponClearLevel2(work, 1, work->player, 0);
		if((clear_level[1] < 0) || (4 < clear_level[1]))
		{
			clear_level[1] = VRSEL_WeaponClearLevel2(work, 1, work->player, 1);
			if((clear_level[1] < 0) || (4 < clear_level[1]))
				clear_level[1] = 5;
			else
				clear_level[1] = 0;
		}
	}
	/* c4/claymore */
	if((all_item == 1) || (cur_pos == OBJECT_2_WEA_C4_CLAYMORE))
	{
		clear_level[2] = VRSEL_WeaponClearLevel2(work, 2, work->player, 0);
		if((clear_level[2] < 0) || (4 < clear_level[2]))
		{
			clear_level[2] = VRSEL_WeaponClearLevel2(work, 2, work->player, 1);
			if((clear_level[2] < 0) || (4 < clear_level[2]))
				clear_level[2] = 5;
			else
				clear_level[2] = 0;
		}
	}
	/* grenade */
	if((all_item == 1) || (cur_pos == OBJECT_2_WEA_GRENADE))
	{
		clear_level[3] = VRSEL_WeaponClearLevel2(work, 3, work->player, 0);
		if((clear_level[3] < 0) || (4 < clear_level[3]))
		{
			clear_level[3] = VRSEL_WeaponClearLevel2(work, 3, work->player, 1);
			if((clear_level[3] < 0) || (4 < clear_level[3]))
				clear_level[3] = 5;
			else
				clear_level[3] = 0;
		}
	}
	/* psg1 */
	if((all_item == 1) || (cur_pos == OBJECT_2_WEA_PSG1))
	{
		clear_level[4] = VRSEL_WeaponClearLevel2(work, 4, work->player, 0);
		if((clear_level[4] < 0) || (4 < clear_level[4]))
		{
			clear_level[4] = VRSEL_WeaponClearLevel2(work, 4, work->player, 1);
			if((clear_level[4] < 0) || (4 < clear_level[4]))
				clear_level[4] = 5;
			else
				clear_level[4] = 0;
		}
	}
	/* stinger */
	if((all_item == 1) || (cur_pos == OBJECT_2_WEA_STINGER))
	{
		clear_level[5] = VRSEL_WeaponClearLevel2(work, 5, work->player, 0);
		if((clear_level[5] < 0) || (4 < clear_level[5]))
		{
			clear_level[5] = VRSEL_WeaponClearLevel2(work, 5, work->player, 1);
			if((clear_level[5] < 0) || (4 < clear_level[5]))
				clear_level[5] = 5;
			else
				clear_level[5] = 0;
		}
	}
	/* nikita */
	if((all_item == 1) || (cur_pos == OBJECT_2_WEA_NIKITA))
	{
		clear_level[6] = VRSEL_WeaponClearLevel2(work, 6, work->player, 0);
		if((clear_level[6] < 0) || (4 < clear_level[6]))
		{
			clear_level[6] = VRSEL_WeaponClearLevel2(work, 6, work->player, 1);
			if((clear_level[6] < 0) || (4 < clear_level[6]))
				clear_level[6] = 5;
			else
				clear_level[6] = 0;
		}
	}
	/* hf blade */
	if((all_item == 1) || (cur_pos == OBJECT_2_WEA_HF_BLADE))
	{
		clear_level[7] = VRSEL_WeaponClearLevel2(work, 7, work->player, 0);
		if((clear_level[7] < 0) || (4 < clear_level[7]))
		{
			clear_level[7] = VRSEL_WeaponClearLevel2(work, 7, work->player, 1);
			if((clear_level[7] < 0) || (4 < clear_level[7]))
				clear_level[7] = 5;
			else
				clear_level[7] = 0;
		}
	}

	/* 色の設定 */
	if(alp_flag == 0)
	{
		if((all_item == 1) || (cur_pos == OBJECT_2_WEA_HANDGUN))
		{
			if((work->player != VRSEL_PLA_NINJA) && (work->player != VRSEL_PLA_X_RAIDEN))
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_WEA_HANDGUN    , 0                                  , step, work->i_color[clear_level[0]], work->a_color[clear_level[0]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_WEA_HANDGUN    , cur_pos == OBJECT_2_WEA_HANDGUN    , step, work->i_color[clear_level[0]], work->a_color[clear_level[0]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_WEA_ASSAULT_R))
		{
			if((work->player != VRSEL_PLA_NINJA) && (work->player != VRSEL_PLA_X_RAIDEN))
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_WEA_ASSAULT_R  , 0                                  , step, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_WEA_ASSAULT_R  , cur_pos == OBJECT_2_WEA_ASSAULT_R  , step, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_WEA_C4_CLAYMORE))
		{
			if((work->player != VRSEL_PLA_NINJA) && (work->player != VRSEL_PLA_X_RAIDEN))
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_WEA_C4_CLAYMORE, 0                                  , step, work->i_color[clear_level[2]], work->a_color[clear_level[2]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_WEA_C4_CLAYMORE, cur_pos == OBJECT_2_WEA_C4_CLAYMORE, step, work->i_color[clear_level[2]], work->a_color[clear_level[2]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_WEA_GRENADE))
		{
			if((work->player != VRSEL_PLA_NINJA) && (work->player != VRSEL_PLA_X_RAIDEN))
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_WEA_GRENADE    , 0                                  , step, work->i_color[clear_level[3]], work->a_color[clear_level[3]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_WEA_GRENADE    , cur_pos == OBJECT_2_WEA_GRENADE    , step, work->i_color[clear_level[3]], work->a_color[clear_level[3]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_WEA_PSG1))
		{
			if((work->player != VRSEL_PLA_NINJA) && (work->player != VRSEL_PLA_X_RAIDEN))
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_WEA_PSG1       , 0                                  , step, work->i_color[clear_level[4]], work->a_color[clear_level[4]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_WEA_PSG1       , cur_pos == OBJECT_2_WEA_PSG1       , step, work->i_color[clear_level[4]], work->a_color[clear_level[4]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_WEA_STINGER))
		{
			if((work->player != VRSEL_PLA_NINJA) && (work->player != VRSEL_PLA_X_RAIDEN))
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_WEA_STINGER    , 0                                  , step, work->i_color[clear_level[5]], work->a_color[clear_level[5]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_WEA_STINGER    , cur_pos == OBJECT_2_WEA_STINGER    , step, work->i_color[clear_level[5]], work->a_color[clear_level[5]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_WEA_NIKITA))
		{
			if((work->player != VRSEL_PLA_NINJA) && (work->player != VRSEL_PLA_X_RAIDEN))
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_WEA_NIKITA     , 0                                  , step, work->i_color[clear_level[6]], work->a_color[clear_level[6]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_WEA_NIKITA     , cur_pos == OBJECT_2_WEA_NIKITA     , step, work->i_color[clear_level[6]], work->a_color[clear_level[6]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_WEA_HF_BLADE))
		{
			if(work->player != VRSEL_PLA_X_RAIDEN)
			{
				if(all_item == 2)
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_WEA_HF_BLADE   , 0                                  , step, work->i_color[clear_level[7]], work->a_color[clear_level[7]]);
				}
				else
				{
					SetSpriteColor(&work->layoutman_2, OBJECT_2_WEA_HF_BLADE   , cur_pos == OBJECT_2_WEA_HF_BLADE   , step, work->i_color[clear_level[7]], work->a_color[clear_level[7]]);
				}
			}
		}
	}
	else
	{
		if((all_item == 1) || (cur_pos == OBJECT_2_WEA_HANDGUN))
		{
			if((work->player != VRSEL_PLA_NINJA) && (work->player != VRSEL_PLA_X_RAIDEN))
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_WEA_HANDGUN    , cur_pos == OBJECT_2_WEA_HANDGUN    , step, work->i_color[clear_level[0]], work->a_color[clear_level[0]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_WEA_ASSAULT_R))
		{
			if((work->player != VRSEL_PLA_NINJA) && (work->player != VRSEL_PLA_X_RAIDEN))
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_WEA_ASSAULT_R  , cur_pos == OBJECT_2_WEA_ASSAULT_R  , step, work->i_color[clear_level[1]], work->a_color[clear_level[1]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_WEA_C4_CLAYMORE))
		{
			if((work->player != VRSEL_PLA_NINJA) && (work->player != VRSEL_PLA_X_RAIDEN))
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_WEA_C4_CLAYMORE, cur_pos == OBJECT_2_WEA_C4_CLAYMORE, step, work->i_color[clear_level[2]], work->a_color[clear_level[2]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_WEA_GRENADE))
		{
			if((work->player != VRSEL_PLA_NINJA) && (work->player != VRSEL_PLA_X_RAIDEN))
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_WEA_GRENADE    , cur_pos == OBJECT_2_WEA_GRENADE    , step, work->i_color[clear_level[3]], work->a_color[clear_level[3]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_WEA_PSG1))
		{
			if((work->player != VRSEL_PLA_NINJA) && (work->player != VRSEL_PLA_X_RAIDEN))
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_WEA_PSG1       , cur_pos == OBJECT_2_WEA_PSG1       , step, work->i_color[clear_level[4]], work->a_color[clear_level[4]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_WEA_STINGER))
		{
			if((work->player != VRSEL_PLA_NINJA) && (work->player != VRSEL_PLA_X_RAIDEN))
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_WEA_STINGER    , cur_pos == OBJECT_2_WEA_STINGER    , step, work->i_color[clear_level[5]], work->a_color[clear_level[5]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_WEA_NIKITA))
		{
			if((work->player != VRSEL_PLA_NINJA) && (work->player != VRSEL_PLA_X_RAIDEN))
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_WEA_NIKITA     , cur_pos == OBJECT_2_WEA_NIKITA     , step, work->i_color[clear_level[6]], work->a_color[clear_level[6]]);
				}
			}
		}
		if((all_item == 1) || (cur_pos == OBJECT_2_WEA_HF_BLADE))
		{
			if(work->player != VRSEL_PLA_X_RAIDEN)
			{
				{
					SetSpriteABGR2(&work->layoutman_2, OBJECT_2_WEA_HF_BLADE   , cur_pos == OBJECT_2_WEA_HF_BLADE   , step, work->i_color[clear_level[7]], work->a_color[clear_level[7]]);
				}
			}
		}
	}
}
static void SetWeaponText2(
	VRSEL_WORK *work,		/* ワーク */
	int        cur_pos)		/* カーソル位置 */
{
	SetWeaponText(work, OBJECT_2_WEA_HANDGUN    , cur_pos == OBJECT_2_WEA_HANDGUN    , 256, 2);
	SetWeaponText(work, OBJECT_2_WEA_ASSAULT_R  , cur_pos == OBJECT_2_WEA_ASSAULT_R  , 256, 2);
	SetWeaponText(work, OBJECT_2_WEA_C4_CLAYMORE, cur_pos == OBJECT_2_WEA_C4_CLAYMORE, 256, 2);
	SetWeaponText(work, OBJECT_2_WEA_GRENADE    , cur_pos == OBJECT_2_WEA_GRENADE    , 256, 2);
	SetWeaponText(work, OBJECT_2_WEA_PSG1       , cur_pos == OBJECT_2_WEA_PSG1       , 256, 2);
	SetWeaponText(work, OBJECT_2_WEA_STINGER    , cur_pos == OBJECT_2_WEA_STINGER    , 256, 2);
	SetWeaponText(work, OBJECT_2_WEA_NIKITA     , cur_pos == OBJECT_2_WEA_NIKITA     , 256, 2);
	SetWeaponText(work, OBJECT_2_WEA_HF_BLADE   , cur_pos == OBJECT_2_WEA_HF_BLADE   , 256, 2);
}

/******************************************************************************
 * クリアレベルの取得
 */
static void GetClearLevel(
	VRSEL_WORK *work,			/* ワーク */
	int        missions,		/* ミッション */
	int        mode,			/* モード */
	int        weapon,			/* 武器 */
	int        *level_num,		/* レベル数を取得するポインタ */
	int        *clear_level)	/* クリアレベルを取得する配列（最大１０要素） */
{
	int  level_num_2 = 0;
	char *stage_list = NULL;
	int  i;

	if(missions == MSN_MISSION_VR)
	{
		if(mode == MSN_MODE_SNEAKING)
		{
			if(weapon == MSN_WEAPON_SNEAKING)
			{
				level_num_2 = SNEAKING_STAGE_NUM;
				stage_list = &_sneaking_stage[0][0][0];
			}
			else if(weapon == MSN_WEAPON_ELIMINATE_ALL)
			{
				level_num_2 = SNEAKING_STAGE_NUM;
				stage_list = &_sneaking_stage[1][0][0];
			}
			else
				ASSERT(0);
		}
		else if(mode == MSN_MODE_WEAPON)
		{
			if(weapon == MSN_WEAPON_HANDGUN)
			{
				level_num_2 = WEAPON_STAGE_NUM;
				stage_list = &_weapon_stage[0][0][0];
			}
			else if(weapon == MSN_WEAPON_ASSAULT_RIFLE)
			{
				level_num_2 = WEAPON_STAGE_NUM;
				stage_list = &_weapon_stage[1][0][0];
			}
			else if(weapon == MSN_WEAPON_C4_CLAYMORE)
			{
				level_num_2 = WEAPON_STAGE_NUM;
				stage_list = &_weapon_stage[2][0][0];
			}
			else if(weapon == MSN_WEAPON_GRENADE)
			{
				level_num_2 = WEAPON_STAGE_NUM;
				stage_list = &_weapon_stage[3][0][0];
			}
			else if(weapon == MSN_WEAPON_PSG1)
			{
				level_num_2 = WEAPON_STAGE_NUM;
				stage_list = &_weapon_stage[4][0][0];
			}
			else if(weapon == MSN_WEAPON_STINGER)
			{
				level_num_2 = WEAPON_STAGE_NUM;
				stage_list = &_weapon_stage[5][0][0];
			}
			else if(weapon == MSN_WEAPON_NIKITA)
			{
				level_num_2 = WEAPON_STAGE_NUM;
				stage_list = &_weapon_stage[6][0][0];
			}
			else if(weapon == MSN_WEAPON_HF_BLADE)
			{
				level_num_2 = WEAPON_STAGE_NUM;
				stage_list = &_weapon_stage[7][0][0];
			}
			else
				ASSERT(0);
		}
		else if(mode == MSN_MODE_FIRST_PERSON_VIEW)
		{
			level_num_2 = FPV_STAGE_NUM;
			stage_list = &_fpv_stage[0][0];
		}
		else if(mode == MSN_MODE_VARIETY)
		{
			switch(work->player)
			{
			case VRSEL_PLA_RAIDEN  : level_num_2 = 7; break;
			case VRSEL_PLA_NINJA   : level_num_2 = 1; break;
			case VRSEL_PLA_X_RAIDEN: ASSERT(0);       break;
			case VRSEL_PLA_SNAKE   : level_num_2 = 7; break;
			case VRSEL_PLA_PLISKIN : level_num_2 = 2; break;
			case VRSEL_PLA_TUXEDO  : level_num_2 = 2; break;
			case VRSEL_PLA_PREVIOUS: level_num_2 = 3; break;
			default:
				ASSERT(0);
				break;
			}
			stage_list = &_variety_stage[0][0];
		}
		else if(mode == MSN_MODE_STREAKING)
		{
			level_num_2 = 1;
		}
		else
			ASSERT(0);
	}
	else if(missions == MSN_MISSION_ALTERNATIVE)
	{
		if(mode == MSN_MODE_BOMB_DISPOSAL)
		{
			level_num_2 = BOMB_STAGE_NUM;
			stage_list = &_bomb_stage[0][0];
		}
		else if(mode == MSN_MODE_ELIMINATE)
		{
			level_num_2 = ELIMINATE_STAGE_NUM;
			stage_list = &_eliminate_stage[0][0];
		}
		else if(mode == MSN_MODE_HOLD_UP)
		{
			level_num_2 = HOLDUP_STAGE_NUM;
			stage_list = &_holdup_stage[0][0];
		}
		else if(mode == MSN_MODE_PHOTOGRAPH)
		{
			switch(work->player)
			{
			case VRSEL_PLA_RAIDEN  : level_num_2 = 6; break;
			case VRSEL_PLA_NINJA   : ASSERT(0);       break;
			case VRSEL_PLA_X_RAIDEN: ASSERT(0);       break;
			case VRSEL_PLA_SNAKE   : level_num_2 = 7; break;
			case VRSEL_PLA_PLISKIN : ASSERT(0);       break;
			case VRSEL_PLA_TUXEDO  : ASSERT(0);       break;
			case VRSEL_PLA_PREVIOUS: ASSERT(0);       break;
			default:
				ASSERT(0);
				break;
			}
			stage_list = &_photograph_stage[0][0];
		}
		else
			ASSERT(0);
	}
	else
		ASSERT(0);

	/* クリアレベル */
	if(mode == MSN_MODE_VARIETY)
	{
		switch(work->player)
		{
		case VRSEL_PLA_RAIDEN:
		case VRSEL_PLA_SNAKE :
			for(i = 0; i < level_num_2; i++)
			{
				clear_level[i] = *(stage_list + (VRSEL_PLA_MAX * i) + work->player);
				if((clear_level[i] < 0) || (4 < clear_level[i]))
				{
					clear_level[i] = 5;
				}
			}
			break;
		case VRSEL_PLA_NINJA:
			clear_level[0] = _variety_stage[4][VRSEL_PLA_NINJA];
			if((clear_level[0] < 0) || (4 < clear_level[0]))
			{
				clear_level[0] = 5;
			}
			break;
		case VRSEL_PLA_X_RAIDEN:
			ASSERT(0);
			break;
		case VRSEL_PLA_PLISKIN:
			clear_level[0] = _variety_stage[3][VRSEL_PLA_PLISKIN];
			if((clear_level[0] < 0) || (4 < clear_level[0]))
			{
				clear_level[0] = 5;
			}
			clear_level[1] = _variety_stage[4][VRSEL_PLA_PLISKIN];
			if((clear_level[1] < 0) || (4 < clear_level[1]))
			{
				clear_level[1] = 5;
			}
			break;
		case VRSEL_PLA_TUXEDO:
			clear_level[0] = _variety_stage[3][VRSEL_PLA_TUXEDO];
			if((clear_level[0] < 0) || (4 < clear_level[0]))
			{
				clear_level[0] = 5;
			}
			clear_level[1] = _variety_stage[4][VRSEL_PLA_TUXEDO];
			if((clear_level[1] < 0) || (4 < clear_level[1]))
			{
				clear_level[1] = 5;
			}
			break;
		case VRSEL_PLA_PREVIOUS:
			clear_level[0] = _variety_stage[3][VRSEL_PLA_PREVIOUS];
			if((clear_level[0] < 0) || (4 < clear_level[0]))
			{
				clear_level[0] = 5;
			}
			clear_level[1] = _variety_stage[4][VRSEL_PLA_PREVIOUS];
			if((clear_level[1] < 0) || (4 < clear_level[1]))
			{
				clear_level[1] = 5;
			}
			clear_level[2] = _variety_stage[6][VRSEL_PLA_PREVIOUS];
			if((clear_level[2] < 0) || (4 < clear_level[2]))
			{
				clear_level[2] = 5;
			}
			break;
		default:
			ASSERT(0);
			break;
		}
	}
	else if(work->mode == MSN_MODE_STREAKING)
	{
		clear_level[0] = _streaking_stage;
		if((clear_level[0] < 0) || (4 < clear_level[0]))
		{
			clear_level[0] = 5;
		}
	}
	else
	{
		for(i = 0; i < level_num_2; i++)
		{
			clear_level[i] = *(stage_list + (VRSEL_PLA_MAX * i) + work->player);
			if((clear_level[i] < 0) || (4 < clear_level[i]))
			{
				clear_level[i] = 5;
			}
		}
	}
	if(work->mode == MSN_MODE_FIRST_PERSON_VIEW)
	{
		// ＣＯＮＴＲＯＬＳ項目
		clear_level[level_num_2] = 0;
	}

	if(level_num != NULL)
		*level_num = level_num_2;
}

/******************************************************************************
 * レベルのテキストの設定
 */
static void SetLevelColor(
	VRSEL_WORK *work,			/* ワーク */
	int        obj1,			/* オブジェクト１ */
	int        obj2,			/* オブジェクト２ */
	int        obj3,			/* オブジェクト３ */
	int        cur_pos,			/* カーソル位置 */
	int        alp_flag,		/* アルファーフラグ */
	int        step,			/* 変化度 */
	int        clear_level)		/* クリアレベル */
{
	if(alp_flag == 0)
	{
		SetSpriteColor(&work->layoutman_2, obj1, cur_pos == obj1, step, work->i_color[clear_level], work->a_color[clear_level]);
		SetSpriteColor(&work->layoutman_2, obj2, cur_pos == obj1, step, work->i_color[clear_level], work->a_color[clear_level]);
		SetSpriteColor(&work->layoutman_2, obj3, cur_pos == obj1, step, work->i_color[clear_level], work->a_color[clear_level]);
	}
	else
	{
		SetSpriteABGR2(&work->layoutman_2, obj1, cur_pos == obj1, step, work->i_color[clear_level], work->a_color[clear_level]);
		SetSpriteABGR2(&work->layoutman_2, obj2, cur_pos == obj1, step, work->i_color[clear_level], work->a_color[clear_level]);
		SetSpriteABGR2(&work->layoutman_2, obj3, cur_pos == obj1, step, work->i_color[clear_level], work->a_color[clear_level]);
	}
}
static void SetAllLevelColor(
	VRSEL_WORK *work,			/* ワーク */
	int        level_num,		/* レベル数 */
	int        alp_flag,		/* アルファーフラグ */
	int        step,			/* 変化度 */
	int        *clear_level)	/* クリアレベル */
{
	int cur_pos;

	/* カーソル位置 */
	cur_pos = LOM_GetCurObj(&work->layoutman_2);

	if(level_num > 0)
	{
		SetLevelColor(work, OBJECT_2_LEV_01, OBJECT_2_LEV_01_01, OBJECT_2_LEV_01_10, cur_pos, alp_flag, step, clear_level[0]);
	}
	if(level_num > 1)
	{
		SetLevelColor(work, OBJECT_2_LEV_02, OBJECT_2_LEV_02_01, OBJECT_2_LEV_02_10, cur_pos, alp_flag, step, clear_level[1]);
	}
	if(level_num > 2)
	{
		SetLevelColor(work, OBJECT_2_LEV_03, OBJECT_2_LEV_03_01, OBJECT_2_LEV_03_10, cur_pos, alp_flag, step, clear_level[2]);
	}
	if(level_num > 3)
	{
		SetLevelColor(work, OBJECT_2_LEV_04, OBJECT_2_LEV_04_01, OBJECT_2_LEV_04_10, cur_pos, alp_flag, step, clear_level[3]);
	}
	if(level_num > 4)
	{
		SetLevelColor(work, OBJECT_2_LEV_05, OBJECT_2_LEV_05_01, OBJECT_2_LEV_05_10, cur_pos, alp_flag, step, clear_level[4]);
	}
	if(level_num > 5)
	{
		SetLevelColor(work, OBJECT_2_LEV_06, OBJECT_2_LEV_06_01, OBJECT_2_LEV_06_10, cur_pos, alp_flag, step, clear_level[5]);
	}
	if(level_num > 6)
	{
		SetLevelColor(work, OBJECT_2_LEV_07, OBJECT_2_LEV_07_01, OBJECT_2_LEV_07_10, cur_pos, alp_flag, step, clear_level[6]);
	}
	if(level_num > 7)
	{
		SetLevelColor(work, OBJECT_2_LEV_08, OBJECT_2_LEV_08_01, OBJECT_2_LEV_08_10, cur_pos, alp_flag, step, clear_level[7]);
	}
	if(level_num > 8)
	{
		SetLevelColor(work, OBJECT_2_LEV_09, OBJECT_2_LEV_09_01, OBJECT_2_LEV_09_10, cur_pos, alp_flag, step, clear_level[8]);
	}
	if(level_num > 9)
	{
		SetLevelColor(work, OBJECT_2_LEV_10, OBJECT_2_LEV_10_01, OBJECT_2_LEV_10_10, cur_pos, alp_flag, step, clear_level[9]);
	}
}
static int SetAllLevelColor2(	/* 1: カーソルを設定した */
								/* 0: カーソルを設定しなかった */
	VRSEL_WORK *work,			/* ワーク */
	int        level_num,		/* レベル数 */
	int        alp_flag,		/* アルファーフラグ */
	int        step,			/* 変化度 */
	int        *clear_level)	/* クリアレベル */
{
	int cur_pos = 0;

	/* カーソル位置 */
	switch(work->level)
	{
	case  0: cur_pos = OBJECT_2_LEV_01; break;
	case  1: cur_pos = OBJECT_2_LEV_02; break;
	case  2: cur_pos = OBJECT_2_LEV_03; break;
	case  3: cur_pos = OBJECT_2_LEV_04; break;
	case  4: cur_pos = OBJECT_2_LEV_05; break;
	case  5: cur_pos = OBJECT_2_LEV_06; break;
	case  6: cur_pos = OBJECT_2_LEV_07; break;
	case  7: cur_pos = OBJECT_2_LEV_08; break;
	case  8: cur_pos = OBJECT_2_LEV_09; break;
	case  9: cur_pos = OBJECT_2_LEV_10; break;
	case 10: break;
	default:
		ASSERT(0);
		break;
	}

	if(level_num > 0)
	{
		SetLevelColor(work, OBJECT_2_LEV_01, OBJECT_2_LEV_01_01, OBJECT_2_LEV_01_10, cur_pos, alp_flag, step, clear_level[0]);
	}
	if(level_num > 1)
	{
		SetLevelColor(work, OBJECT_2_LEV_02, OBJECT_2_LEV_02_01, OBJECT_2_LEV_02_10, cur_pos, alp_flag, step, clear_level[1]);
	}
	if(level_num > 2)
	{
		SetLevelColor(work, OBJECT_2_LEV_03, OBJECT_2_LEV_03_01, OBJECT_2_LEV_03_10, cur_pos, alp_flag, step, clear_level[2]);
	}
	if(level_num > 3)
	{
		SetLevelColor(work, OBJECT_2_LEV_04, OBJECT_2_LEV_04_01, OBJECT_2_LEV_04_10, cur_pos, alp_flag, step, clear_level[3]);
	}
	if(level_num > 4)
	{
		SetLevelColor(work, OBJECT_2_LEV_05, OBJECT_2_LEV_05_01, OBJECT_2_LEV_05_10, cur_pos, alp_flag, step, clear_level[4]);
	}
	if(level_num > 5)
	{
		SetLevelColor(work, OBJECT_2_LEV_06, OBJECT_2_LEV_06_01, OBJECT_2_LEV_06_10, cur_pos, alp_flag, step, clear_level[5]);
	}
	if(level_num > 6)
	{
		SetLevelColor(work, OBJECT_2_LEV_07, OBJECT_2_LEV_07_01, OBJECT_2_LEV_07_10, cur_pos, alp_flag, step, clear_level[6]);
	}
	if(level_num > 7)
	{
		SetLevelColor(work, OBJECT_2_LEV_08, OBJECT_2_LEV_08_01, OBJECT_2_LEV_08_10, cur_pos, alp_flag, step, clear_level[7]);
	}
	if(level_num > 8)
	{
		SetLevelColor(work, OBJECT_2_LEV_09, OBJECT_2_LEV_09_01, OBJECT_2_LEV_09_10, cur_pos, alp_flag, step, clear_level[8]);
	}
	if(level_num > 9)
	{
		SetLevelColor(work, OBJECT_2_LEV_10, OBJECT_2_LEV_10_01, OBJECT_2_LEV_10_10, cur_pos, alp_flag, step, clear_level[9]);
	}

	if(cur_pos == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

/******************************************************************************
 * カウンターの設定
 */
static void SetCounter(
	VRSEL_WORK *work,		/* ワーク */
	int        value,		/* 数値 */
	int        strcode6,	/* ６桁目用オブジェクトの文字列コード */
	int        strcode5,	/* ５桁目用オブジェクトの文字列コード */
	int        strcode4,	/* ４桁目用オブジェクトの文字列コード */
	int        strcode3,	/* ３桁目用オブジェクトの文字列コード */
	int        strcode2,	/* ２桁目用オブジェクトの文字列コード */
	int        strcode1)	/* １桁目用オブジェクトの文字列コード */
{
	int i;

	if(value < 0)
		return;
	if(value > 999999)
	{
		/* カンスト */
		LOM_SprTex_SetU(&work->layoutman_2, strcode6, work->tex2_u0 + VRSEL_TEX_WIDTH * 9);
		LOM_SprTex_SetU(&work->layoutman_2, strcode5, work->tex2_u0 + VRSEL_TEX_WIDTH * 9);
		LOM_SprTex_SetU(&work->layoutman_2, strcode4, work->tex2_u0 + VRSEL_TEX_WIDTH * 9);
		LOM_SprTex_SetU(&work->layoutman_2, strcode3, work->tex2_u0 + VRSEL_TEX_WIDTH * 9);
		LOM_SprTex_SetU(&work->layoutman_2, strcode2, work->tex2_u0 + VRSEL_TEX_WIDTH * 9);
		LOM_SprTex_SetU(&work->layoutman_2, strcode1, work->tex2_u0 + VRSEL_TEX_WIDTH * 9);
		return;
	}

	/* １０００００の位 */
	i = value / 100000;
	LOM_SprTex_SetU(&work->layoutman_2, strcode6, work->tex2_u0 + VRSEL_TEX_WIDTH * i);

	/* １００００の位 */
	value %= 100000;
	i = value / 10000;
	LOM_SprTex_SetU(&work->layoutman_2, strcode5, work->tex2_u0 + VRSEL_TEX_WIDTH * i);

	/* １０００の位 */
	value %= 10000;
	i = value / 1000;
	LOM_SprTex_SetU(&work->layoutman_2, strcode4, work->tex2_u0 + VRSEL_TEX_WIDTH * i);

	/* １００の位 */
	value %= 1000;
	i = value / 100;
	LOM_SprTex_SetU(&work->layoutman_2, strcode3, work->tex2_u0 + VRSEL_TEX_WIDTH * i);

	/* １０の位 */
	value %= 100;
	i = value / 10;
	LOM_SprTex_SetU(&work->layoutman_2, strcode2, work->tex2_u0 + VRSEL_TEX_WIDTH * i);

	/* １の位 */
	value %= 10;
	LOM_SprTex_SetU(&work->layoutman_2, strcode1, work->tex2_u0 + VRSEL_TEX_WIDTH * value);
}

/******************************************************************************
 * ランキングの設定
 */
static void SetRanking(
	VRSEL_WORK *work,	/* ワーク */
	int        show)	/* 表示フラグ */
{
	int stage;
	int overwrite[3];

	stage = VRSEL_GetStageID(work);
	if(stage == -1)
	{
		// 非表示
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_RECORD     , 1, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_BAR        , 1, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_1ST        , 0, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_1ST_10     , 0, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_1ST_100    , 0, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_1ST_1000   , 0, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_1ST_10000  , 0, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_1ST_100000 , 0, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_1ST_1000000, 0, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_2ND        , 0, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_2ND_10     , 0, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_2ND_100    , 0, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_2ND_1000   , 0, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_2ND_10000  , 0, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_2ND_100000 , 0, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_2ND_1000000, 0, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_3RD        , 0, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_3RD_10     , 0, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_3RD_100    , 0, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_3RD_1000   , 0, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_3RD_10000  , 0, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_3RD_100000 , 0, 256, 0x00000000, 0x00000000);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_3RD_1000000, 0, 256, 0x00000000, 0x00000000);

		SetSpriteABGR(&work->layoutman_2, OBJECT_2_Y_BUT_CLR_CODE , 0, 256, 0x00000000, 0x00000000);
		return;
	}

	// ハイスコア値の設定
	overwrite[0] = (MSN_GET_HISCORE(stage, 1) & MSN_OVERWRITEFLAG) != 0;
	overwrite[1] = (MSN_GET_HISCORE(stage, 2) & MSN_OVERWRITEFLAG) != 0;
	overwrite[2] = (MSN_GET_HISCORE(stage, 3) & MSN_OVERWRITEFLAG) != 0;

	SetCounter(work, MSN_GET_HISCORE2(stage, 1),
		OBJECT_2_REC_1ST_1000000, OBJECT_2_REC_1ST_100000, OBJECT_2_REC_1ST_10000,
		OBJECT_2_REC_1ST_1000   , OBJECT_2_REC_1ST_100   , OBJECT_2_REC_1ST_10   );
	SetCounter(work, MSN_GET_HISCORE2(stage, 2),
		OBJECT_2_REC_2ND_1000000, OBJECT_2_REC_2ND_100000, OBJECT_2_REC_2ND_10000,
		OBJECT_2_REC_2ND_1000   , OBJECT_2_REC_2ND_100   , OBJECT_2_REC_2ND_10   );
	SetCounter(work, MSN_GET_HISCORE2(stage, 3),
		OBJECT_2_REC_3RD_1000000, OBJECT_2_REC_3RD_100000, OBJECT_2_REC_3RD_10000,
		OBJECT_2_REC_3RD_1000   , OBJECT_2_REC_3RD_100   , OBJECT_2_REC_3RD_10   );

	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		// フェードアウト（アルファ操作なし）
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_RECORD     , 1           , 256, work->i_color[0], work->a_color[0]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_BAR        , 1           , 256, work->i_color[0], work->a_color[0]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_1ST        , 0           , 256, work->i_color[0], work->a_color[0]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_1ST_10     , overwrite[0], 256, work->i_color[0], work->i_color[1]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_1ST_100    , overwrite[0], 256, work->i_color[0], work->i_color[1]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_1ST_1000   , overwrite[0], 256, work->i_color[0], work->i_color[1]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_1ST_10000  , overwrite[0], 256, work->i_color[0], work->i_color[1]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_1ST_100000 , overwrite[0], 256, work->i_color[0], work->i_color[1]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_1ST_1000000, overwrite[0], 256, work->i_color[0], work->i_color[1]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_2ND        , 0           , 256, work->i_color[0], work->a_color[0]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_2ND_10     , overwrite[1], 256, work->i_color[0], work->i_color[2]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_2ND_100    , overwrite[1], 256, work->i_color[0], work->i_color[2]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_2ND_1000   , overwrite[1], 256, work->i_color[0], work->i_color[2]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_2ND_10000  , overwrite[1], 256, work->i_color[0], work->i_color[2]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_2ND_100000 , overwrite[1], 256, work->i_color[0], work->i_color[2]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_2ND_1000000, overwrite[1], 256, work->i_color[0], work->i_color[2]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_3RD        , 0           , 256, work->i_color[0], work->a_color[0]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_3RD_10     , overwrite[2], 256, work->i_color[0], work->i_color[3]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_3RD_100    , overwrite[2], 256, work->i_color[0], work->i_color[3]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_3RD_1000   , overwrite[2], 256, work->i_color[0], work->i_color[3]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_3RD_10000  , overwrite[2], 256, work->i_color[0], work->i_color[3]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_3RD_100000 , overwrite[2], 256, work->i_color[0], work->i_color[3]);
		SetSpriteColor(&work->layoutman_2, OBJECT_2_REC_3RD_1000000, overwrite[2], 256, work->i_color[0], work->i_color[3]);

		SetSpriteColor(&work->layoutman_2, OBJECT_2_Y_BUT_CLR_CODE , overwrite[0], 256, work->i_color[5], work->a_color[1]);
	}
	else if(show == 1)
	{
		// 表示
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_RECORD     , 1           , 16, work->i_color[0], work->a_color[0]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_BAR        , 1           , 16, work->i_color[0], work->a_color[0]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_1ST        , 0           , 16, work->i_color[0], work->a_color[0]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_1ST_10     , overwrite[0], 16, work->i_color[0], work->i_color[1]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_1ST_100    , overwrite[0], 16, work->i_color[0], work->i_color[1]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_1ST_1000   , overwrite[0], 16, work->i_color[0], work->i_color[1]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_1ST_10000  , overwrite[0], 16, work->i_color[0], work->i_color[1]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_1ST_100000 , overwrite[0], 16, work->i_color[0], work->i_color[1]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_1ST_1000000, overwrite[0], 16, work->i_color[0], work->i_color[1]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_2ND        , 0           , 16, work->i_color[0], work->a_color[0]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_2ND_10     , overwrite[1], 16, work->i_color[0], work->i_color[2]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_2ND_100    , overwrite[1], 16, work->i_color[0], work->i_color[2]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_2ND_1000   , overwrite[1], 16, work->i_color[0], work->i_color[2]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_2ND_10000  , overwrite[1], 16, work->i_color[0], work->i_color[2]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_2ND_100000 , overwrite[1], 16, work->i_color[0], work->i_color[2]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_2ND_1000000, overwrite[1], 16, work->i_color[0], work->i_color[2]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_3RD        , 0           , 16, work->i_color[0], work->a_color[0]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_3RD_10     , overwrite[2], 16, work->i_color[0], work->i_color[3]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_3RD_100    , overwrite[2], 16, work->i_color[0], work->i_color[3]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_3RD_1000   , overwrite[2], 16, work->i_color[0], work->i_color[3]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_3RD_10000  , overwrite[2], 16, work->i_color[0], work->i_color[3]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_3RD_100000 , overwrite[2], 16, work->i_color[0], work->i_color[3]);
		SetSpriteABGR(&work->layoutman_2, OBJECT_2_REC_3RD_1000000, overwrite[2], 16, work->i_color[0], work->i_color[3]);

		SetSpriteABGR(&work->layoutman_2, OBJECT_2_Y_BUT_CLR_CODE , overwrite[0], 16, work->i_color[5], work->a_color[1]);
	}
}

/******************************************************************************
 * ミッション選択画面
 */
static void SetMissionsScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int cur_pos;
	int lom_mode;

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* カーソル位置 */
	cur_pos = LOM_GetCurObj(&work->layoutman_2);

	/* ミッションのテキストの設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		lom_mode = LOM_GetCurMode(&work->layoutman_2);
		if((lom_mode == LOM_MODE_21) || (lom_mode == LOM_MODE_24))
		{
			/* 正方向 */
			SetMissionsText(work, cur_pos, 0, 256, 1);
		}
		else
		{
			/* 逆方向 */
			SetMissionsText2(work, cur_pos);
			if(work->missions == MSN_MISSION_VR)
			{
				if(work->mode == MSN_MODE_SNEAKING)
				{
					SetVRText(work, OBJECT_2_MOD_SNEAKING, 0, 256, 1);
				}
				else if(work->mode == MSN_MODE_WEAPON)
				{
					SetVRText(work, OBJECT_2_MOD_WEAPON, 0, 256, 1);
				}
				else if(work->mode == MSN_MODE_FIRST_PERSON_VIEW)
				{
					SetVRText(work, OBJECT_2_MOD_FSV_1, 0, 256, 1);
				}
				else if(work->mode == MSN_MODE_VARIETY)
				{
					SetVRText(work, OBJECT_2_MOD_VARIETY, 0, 256, 1);
				}
				else if(work->mode == MSN_MODE_STREAKING)
				{
					SetVRText(work, OBJECT_2_MOD_STREAKING, 0, 256, 1);
				}
				else
					ASSERT(0);
			}
			else if(work->missions == MSN_MISSION_ALTERNATIVE)
			{
				if(work->mode == MSN_MODE_BOMB_DISPOSAL)
				{
					SetAlternativeText(work, OBJECT_2_MOD_BOMB_D, 0, 256, 1);
				}
				else if(work->mode == MSN_MODE_ELIMINATE)
				{
					SetAlternativeText(work, OBJECT_2_MOD_ELIMINATE, 0, 256, 1);
				}
				else if(work->mode == MSN_MODE_HOLD_UP)
				{
					SetAlternativeText(work, OBJECT_2_MOD_HOLD_UP, 0, 256, 1);
				}
				else if(work->mode == MSN_MODE_PHOTOGRAPH)
				{
					SetAlternativeText(work, OBJECT_2_MOD_PHOTOGRAPH, 0, 256, 1);
				}
				else
					ASSERT(0);
			}
			else
				ASSERT(0);
		}
	}
	else
	{
		SetMissionsText(work, cur_pos, 1, 16, 1);
	}
}

/******************************************************************************
 * モード選択画面（ＶＲ）
 */
static void SetVRScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int cur_pos;
	int lom_mode;
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText2(work, OBJECT_2_MIS_VR);

	/* カーソル位置 */
	cur_pos = LOM_GetCurObj(&work->layoutman_2);

	/* モード（ＶＲ）のテキストの設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		lom_mode = LOM_GetCurMode(&work->layoutman_2);
		if((lom_mode == LOM_MODE_26) || (lom_mode == LOM_MODE_31) || (lom_mode == LOM_MODE_35))
		{
			/* 正方向 */
			SetVRText(work, cur_pos, 0, 256, 1);
		}
		else
		{
			/* 逆方向 */
			SetVRText2(work, cur_pos);
			if(work->mode == MSN_MODE_SNEAKING)
			{
				if(work->weapon == MSN_WEAPON_SNEAKING)
				{
					SetSneakingText(work, OBJECT_2_WEA_SNEAKING, 0, 256, 1);
				}
				else if(work->weapon == MSN_WEAPON_ELIMINATE_ALL)
				{
					SetSneakingText(work, OBJECT_2_WEA_ELIMINATE_ALL, 0, 256, 1);
				}
				else
					ASSERT(0);
			}
			else if(work->mode == MSN_MODE_WEAPON)
			{
				if(work->weapon == MSN_WEAPON_HANDGUN)
				{
					SetWeaponText(work, OBJECT_2_WEA_HANDGUN, 0, 256, 1);
				}
				else if(work->weapon == MSN_WEAPON_ASSAULT_RIFLE)
				{
					SetWeaponText(work, OBJECT_2_WEA_ASSAULT_R, 0, 256, 1);
				}
				else if(work->weapon == MSN_WEAPON_C4_CLAYMORE)
				{
					SetWeaponText(work, OBJECT_2_WEA_C4_CLAYMORE, 0, 256, 1);
				}
				else if(work->weapon == MSN_WEAPON_GRENADE)
				{
					SetWeaponText(work, OBJECT_2_WEA_GRENADE, 0, 256, 1);
				}
				else if(work->weapon == MSN_WEAPON_PSG1)
				{
					SetWeaponText(work, OBJECT_2_WEA_PSG1, 0, 256, 1);
				}
				else if(work->weapon == MSN_WEAPON_STINGER)
				{
					SetWeaponText(work, OBJECT_2_WEA_STINGER, 0, 256, 1);
				}
				else if(work->weapon == MSN_WEAPON_NIKITA)
				{
					SetWeaponText(work, OBJECT_2_WEA_NIKITA, 0, 256, 1);
				}
				else if(work->weapon == MSN_WEAPON_HF_BLADE)
				{
					SetWeaponText(work, OBJECT_2_WEA_HF_BLADE, 0, 256, 1);
				}
				else
					ASSERT(0);
			}
			else if(work->mode == MSN_MODE_FIRST_PERSON_VIEW)
			{
				GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);
				if(SetAllLevelColor2(work, level_num, 0, 256, clear_level) == 0)
				{
#ifndef KP_WINDOWS
					SetSpriteColor(&work->layoutman_2, OBJECT_2_LEV_CONTROL, 1, 256, work->i_color[clear_level[level_num]], work->a_color[clear_level[level_num]]);
#endif
				}
				else
				{
#ifndef KP_WINDOWS
					SetSpriteColor(&work->layoutman_2, OBJECT_2_LEV_CONTROL, 0, 256, work->i_color[clear_level[level_num]], work->a_color[clear_level[level_num]]);
#endif
				}
			}
			else if(work->mode == MSN_MODE_VARIETY)
			{
				GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);
				SetAllLevelColor2(work, level_num, 0, 256, clear_level);
			}
			else if(work->mode == MSN_MODE_STREAKING)
			{
				GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);
				SetAllLevelColor2(work, level_num, 0, 256, clear_level);
			}
			else
				ASSERT(0);
		}
	}
	else
	{
		SetVRText(work, cur_pos, 1, 16, 1);
	}

	/* ムービー */
	if((work->count2 == 2) && (work->flag & VRSEL_FLAG_REQUEST_MOVIE))
	{
		switch(cur_pos)
		{
		case OBJECT_2_MOD_SNEAKING: VRSEL_SetMovie2(work, VRSEL_MOV_SNEAKING_RAIDEN         ); break;
		case OBJECT_2_MOD_WEAPON  : VRSEL_SetMovie2(work, VRSEL_MOV_WEAPON_RAIDEN           ); break;
		case OBJECT_2_MOD_FSV_1   : VRSEL_SetMovie2(work, VRSEL_MOV_FIRST_PERSON_VIEW_RAIDEN); break;
		case OBJECT_2_MOD_VARIETY : VRSEL_SetMovie2(work, VRSEL_MOV_VARIETY_RAIDEN          ); break;
		default:
			break;
		}
	}

	/* アイコン */
	if((work->player != VRSEL_PLA_NINJA) && (work->player != VRSEL_PLA_X_RAIDEN))
	{
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HANDGUN_1      , 0, 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HANDGUN_2      , 0, 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_ASSAULT_RIFLE_1, 0, 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_ASSAULT_RIFLE_2, 0, 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_C4_CLAYMORE_1  , 0, 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_C4_CLAYMORE_2  , 0, 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_GRENADE_1      , 0, 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_GRENADE_2      , 0, 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_PSG1           , 0, 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_STINGER        , 0, 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_NIKITA         , 0, 16, 0, 128, 1);
	}
	if(work->player != VRSEL_PLA_X_RAIDEN)
	{
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HF_BLADE       , 0, 16, 0, 128, 1);
	}

	/* ランキング */
	SetRanking(work, 0);
}

/******************************************************************************
 * モード選択画面（ＡＬＴＥＲＮＡＴＩＶＥ）
 */
static void SetAlternativeScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int cur_pos;
	int lom_mode;
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText2(work, OBJECT_2_MIS_ALTERNATIVE);

	/* カーソル位置 */
	cur_pos = LOM_GetCurObj(&work->layoutman_2);

	/* モード（ＡＬＴＥＲＮＡＴＩＶＥ）のテキストの設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		lom_mode = LOM_GetCurMode(&work->layoutman_2);
		if((lom_mode == LOM_MODE_37) || (lom_mode == LOM_MODE_42) || (lom_mode == LOM_MODE_45))
		{
			/* 正方向 */
			SetAlternativeText(work, cur_pos, 0, 256, 1);
		}
		else
		{
			/* 逆方向 */
			SetAlternativeText2(work, cur_pos);
			GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);
			SetAllLevelColor2(work, level_num, 0, 256, clear_level);
		}
	}
	else
	{
		SetAlternativeText(work, cur_pos, 1, 16, 1);
	}

	/* ムービー */
	if((work->count2 == 2) && (work->flag & VRSEL_FLAG_REQUEST_MOVIE))
	{
		switch(cur_pos)
		{
		case OBJECT_2_MOD_BOMB_D    : VRSEL_SetMovie2(work, VRSEL_MOV_BOMB_DISPOSAL_RAIDEN); break;
		case OBJECT_2_MOD_ELIMINATE : VRSEL_SetMovie2(work, VRSEL_MOV_ELIMINATE_RAIDEN    ); break;
		case OBJECT_2_MOD_HOLD_UP   : VRSEL_SetMovie2(work, VRSEL_MOV_HOLD_UP_RAIDEN      ); break;
		case OBJECT_2_MOD_PHOTOGRAPH: VRSEL_SetMovie2(work, VRSEL_MOV_PHOTOGRAPH_RAIDEN   ); break;
		default:
			break;
		}
	}

	/* ランキング */
	SetRanking(work, 0);
}

/******************************************************************************
 * 武器選択画面（ＳＮＥＡＫＩＮＧ）
 */
static void SetSneakingScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int cur_pos;
	int lom_mode;
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText(work, OBJECT_2_MIS_VR, 1, 256, 0);

	/* モード（ＶＲ）のテキストの設定 */
	SetVRText2(work, OBJECT_2_MOD_SNEAKING);

	/* カーソル位置 */
	cur_pos = LOM_GetCurObj(&work->layoutman_2);

	/* 武器（ＳＮＥＡＫＩＮＧ）のテキストの設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		lom_mode = LOM_GetCurMode(&work->layoutman_2);
		if(lom_mode == LOM_MODE_49)
		{
			/* 正方向 */
			SetSneakingText(work, cur_pos, 0, 256, 1);
		}
		else
		{
			/* 逆方向 */
			SetSneakingText2(work, cur_pos);
			GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);
			SetAllLevelColor2(work, level_num, 0, 256, clear_level);
		}
	}
	else
	{
		SetSneakingText(work, cur_pos, 1, 16, 1);
	}

	/* ランキング */
	SetRanking(work, 0);
}

/******************************************************************************
 * 武器選択画面（ＷＥＡＰＯＮ）
 */
static void SetWeaponScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int cur_pos;
	int lom_mode;
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText(work, OBJECT_2_MIS_VR, 1, 256, 0);

	/* モード（ＶＲ）のテキストの設定 */
	SetVRText2(work, OBJECT_2_MOD_WEAPON);

	/* カーソル位置 */
	cur_pos = LOM_GetCurObj(&work->layoutman_2);

	/* 武器（ＷＥＡＰＯＮ）のテキスト設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		lom_mode = LOM_GetCurMode(&work->layoutman_2);
		if((lom_mode == LOM_MODE_52) || (lom_mode == LOM_MODE_61))
		{
			/* 正方向 */
			SetWeaponText(work, cur_pos, 0, 256, 1);
		}
		else
		{
			/* 逆方向 */
			SetWeaponText2(work, cur_pos);
			GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);
			SetAllLevelColor2(work, level_num, 0, 256, clear_level);
		}
	}
	else
	{
		SetWeaponText(work, cur_pos, 1, 16, 1);

		/* アイコン */
		if((work->player != VRSEL_PLA_NINJA) && (work->player != VRSEL_PLA_X_RAIDEN))
		{
			SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HANDGUN_1      , cur_pos == OBJECT_2_WEA_HANDGUN    , 16, 0, 128, 1);
			SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HANDGUN_2      , cur_pos == OBJECT_2_WEA_HANDGUN    , 16, 0, 128, 1);
			SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_ASSAULT_RIFLE_1, cur_pos == OBJECT_2_WEA_ASSAULT_R  , 16, 0, 128, 1);
			SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_ASSAULT_RIFLE_2, cur_pos == OBJECT_2_WEA_ASSAULT_R  , 16, 0, 128, 1);
			SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_C4_CLAYMORE_1  , cur_pos == OBJECT_2_WEA_C4_CLAYMORE, 16, 0, 128, 1);
			SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_C4_CLAYMORE_2  , cur_pos == OBJECT_2_WEA_C4_CLAYMORE, 16, 0, 128, 1);
			SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_GRENADE_1      , cur_pos == OBJECT_2_WEA_GRENADE    , 16, 0, 128, 1);
			SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_GRENADE_2      , cur_pos == OBJECT_2_WEA_GRENADE    , 16, 0, 128, 1);
			SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_PSG1           , cur_pos == OBJECT_2_WEA_PSG1       , 16, 0, 128, 1);
			SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_STINGER        , cur_pos == OBJECT_2_WEA_STINGER    , 16, 0, 128, 1);
			SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_NIKITA         , cur_pos == OBJECT_2_WEA_NIKITA     , 16, 0, 128, 1);
		}
		if(work->player != VRSEL_PLA_X_RAIDEN)
		{
			if((work->player == VRSEL_PLA_RAIDEN) || (work->player == VRSEL_PLA_NINJA))
			SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HF_BLADE       , cur_pos == OBJECT_2_WEA_HF_BLADE   , 16, 0, 128, 1);
		}
	}

	/* ランキング */
	SetRanking(work, 0);
}

/******************************************************************************
 * レベル選択画面
 */
static void SetSneSneakingScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText(work, OBJECT_2_MIS_VR, 1, 256, 0);

	/* モード（ＶＲ）のテキストの設定 */
	SetVRText(work, OBJECT_2_MOD_SNEAKING, 1, 256, 0);

	/* 武器（ＳＮＥＡＫＩＮＧ）のテキストの設定 */
	SetSneakingText2(work, OBJECT_2_WEA_SNEAKING);

	/* クリアレベル */
	GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);

	/* レベルの色の設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		SetAllLevelColor(work, level_num, 0, 256, clear_level);
	}
	else
	{
		SetAllLevelColor(work, level_num, 1, 16, clear_level);
	}

	/* ランキング */
	SetRanking(work, 1);
}
static void SetSneEliminateAllScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText(work, OBJECT_2_MIS_VR, 1, 256, 0);

	/* モード（ＶＲ）のテキストの設定 */
	SetVRText(work, OBJECT_2_MOD_SNEAKING, 1, 256, 0);

	/* 武器（ＳＮＥＡＫＩＮＧ）のテキストの設定 */
	SetSneakingText2(work, OBJECT_2_WEA_ELIMINATE_ALL);

	/* クリアレベル */
	GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);

	/* レベルの色の設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		SetAllLevelColor(work, level_num, 0, 256, clear_level);
	}
	else
	{
		SetAllLevelColor(work, level_num, 1, 16, clear_level);
	}

	/* ランキング */
	SetRanking(work, 1);
}
static void SetWeaHandgunScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText(work, OBJECT_2_MIS_VR, 1, 256, 0);

	/* モード（ＶＲ）のテキストの設定 */
	SetVRText(work, OBJECT_2_MOD_WEAPON, 1, 256, 0);

	/* 武器（ＷＥＡＰＯＮ）のテキスト設定 */
	SetWeaponText2(work, OBJECT_2_WEA_HANDGUN);

	/* クリアレベル */
	GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);

	/* レベルの色の設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		SetAllLevelColor(work, level_num, 0, 256, clear_level);
	}
	else
	{
		SetAllLevelColor(work, level_num, 1, 16, clear_level);
	}

	/* ランキング */
	SetRanking(work, 1);

	/* アイコン */
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HANDGUN_1      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HANDGUN_2      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_ASSAULT_RIFLE_1, 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_ASSAULT_RIFLE_2, 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_C4_CLAYMORE_1  , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_C4_CLAYMORE_2  , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_GRENADE_1      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_GRENADE_2      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_PSG1           , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_STINGER        , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_NIKITA         , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HF_BLADE       , 0, 16, 0, 128, 1);
}
static void SetWeaAssaultRifleScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText(work, OBJECT_2_MIS_VR, 1, 256, 0);

	/* モード（ＶＲ）のテキストの設定 */
	SetVRText(work, OBJECT_2_MOD_WEAPON, 1, 256, 0);

	/* 武器（ＷＥＡＰＯＮ）のテキスト設定 */
	SetWeaponText2(work, OBJECT_2_WEA_ASSAULT_R);

	/* クリアレベル */
	GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);

	/* レベルの色の設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		SetAllLevelColor(work, level_num, 0, 256, clear_level);
	}
	else
	{
		SetAllLevelColor(work, level_num, 1, 16, clear_level);
	}

	/* ランキング */
	SetRanking(work, 1);

	/* アイコン */
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HANDGUN_1      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HANDGUN_2      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_ASSAULT_RIFLE_1, 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_ASSAULT_RIFLE_2, 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_C4_CLAYMORE_1  , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_C4_CLAYMORE_2  , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_GRENADE_1      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_GRENADE_2      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_PSG1           , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_STINGER        , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_NIKITA         , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HF_BLADE       , 0, 16, 0, 128, 1);
}
static void SetWeaC4ClaymoreScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText(work, OBJECT_2_MIS_VR, 1, 256, 0);

	/* モード（ＶＲ）のテキストの設定 */
	SetVRText(work, OBJECT_2_MOD_WEAPON, 1, 256, 0);

	/* 武器（ＷＥＡＰＯＮ）のテキスト設定 */
	SetWeaponText2(work, OBJECT_2_WEA_C4_CLAYMORE);

	/* クリアレベル */
	GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);

	/* レベルの色の設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		SetAllLevelColor(work, level_num, 0, 256, clear_level);
	}
	else
	{
		SetAllLevelColor(work, level_num, 1, 16, clear_level);
	}

	/* ランキング */
	SetRanking(work, 1);

	/* アイコン */
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HANDGUN_1      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HANDGUN_2      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_ASSAULT_RIFLE_1, 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_ASSAULT_RIFLE_2, 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_C4_CLAYMORE_1  , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_C4_CLAYMORE_2  , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_GRENADE_1      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_GRENADE_2      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_PSG1           , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_STINGER        , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_NIKITA         , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HF_BLADE       , 0, 16, 0, 128, 1);
}
static void SetWeaGrenadeScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText(work, OBJECT_2_MIS_VR, 1, 256, 0);

	/* モード（ＶＲ）のテキストの設定 */
	SetVRText(work, OBJECT_2_MOD_WEAPON, 1, 256, 0);

	/* 武器（ＷＥＡＰＯＮ）のテキスト設定 */
	SetWeaponText2(work, OBJECT_2_WEA_GRENADE);

	/* クリアレベル */
	GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);

	/* レベルの色の設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		SetAllLevelColor(work, level_num, 0, 256, clear_level);
	}
	else
	{
		SetAllLevelColor(work, level_num, 1, 16, clear_level);
	}

	/* ランキング */
	SetRanking(work, 1);

	/* アイコン */
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HANDGUN_1      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HANDGUN_2      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_ASSAULT_RIFLE_1, 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_ASSAULT_RIFLE_2, 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_C4_CLAYMORE_1  , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_C4_CLAYMORE_2  , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_GRENADE_1      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_GRENADE_2      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_PSG1           , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_STINGER        , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_NIKITA         , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HF_BLADE       , 0, 16, 0, 128, 1);
}
static void SetWeaPsg1Screen(
	VRSEL_WORK *work)	/* ワーク */
{
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText(work, OBJECT_2_MIS_VR, 1, 256, 0);

	/* モード（ＶＲ）のテキストの設定 */
	SetVRText(work, OBJECT_2_MOD_WEAPON, 1, 256, 0);

	/* 武器（ＷＥＡＰＯＮ）のテキスト設定 */
	SetWeaponText2(work, OBJECT_2_WEA_PSG1);

	/* クリアレベル */
	GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);

	/* レベルの色の設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		SetAllLevelColor(work, level_num, 0, 256, clear_level);
	}
	else
	{
		SetAllLevelColor(work, level_num, 1, 16, clear_level);
	}

	/* ランキング */
	SetRanking(work, 1);

	/* アイコン */
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HANDGUN_1      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HANDGUN_2      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_ASSAULT_RIFLE_1, 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_ASSAULT_RIFLE_2, 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_C4_CLAYMORE_1  , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_C4_CLAYMORE_2  , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_GRENADE_1      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_GRENADE_2      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_PSG1           , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_STINGER        , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_NIKITA         , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HF_BLADE       , 0, 16, 0, 128, 1);
}
static void SetWeaStingerScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText(work, OBJECT_2_MIS_VR, 1, 256, 0);

	/* モード（ＶＲ）のテキストの設定 */
	SetVRText(work, OBJECT_2_MOD_WEAPON, 1, 256, 0);

	/* 武器（ＷＥＡＰＯＮ）のテキスト設定 */
	SetWeaponText2(work, OBJECT_2_WEA_STINGER);

	/* クリアレベル */
	GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);

	/* レベルの色の設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		SetAllLevelColor(work, level_num, 0, 256, clear_level);
	}
	else
	{
		SetAllLevelColor(work, level_num, 1, 16, clear_level);
	}

	/* ランキング */
	SetRanking(work, 1);

	/* アイコン */
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HANDGUN_1      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HANDGUN_2      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_ASSAULT_RIFLE_1, 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_ASSAULT_RIFLE_2, 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_C4_CLAYMORE_1  , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_C4_CLAYMORE_2  , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_GRENADE_1      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_GRENADE_2      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_PSG1           , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_STINGER        , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_NIKITA         , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HF_BLADE       , 0, 16, 0, 128, 1);
}
static void SetWeaNikitaScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText(work, OBJECT_2_MIS_VR, 1, 256, 0);

	/* モード（ＶＲ）のテキストの設定 */
	SetVRText(work, OBJECT_2_MOD_WEAPON, 1, 256, 0);

	/* 武器（ＷＥＡＰＯＮ）のテキスト設定 */
	SetWeaponText2(work, OBJECT_2_WEA_NIKITA);

	/* クリアレベル */
	GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);

	/* レベルの色の設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		SetAllLevelColor(work, level_num, 0, 256, clear_level);
	}
	else
	{
		SetAllLevelColor(work, level_num, 1, 16, clear_level);
	}

	/* ランキング */
	SetRanking(work, 1);

	/* アイコン */
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HANDGUN_1      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HANDGUN_2      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_ASSAULT_RIFLE_1, 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_ASSAULT_RIFLE_2, 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_C4_CLAYMORE_1  , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_C4_CLAYMORE_2  , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_GRENADE_1      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_GRENADE_2      , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_PSG1           , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_STINGER        , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_NIKITA         , 0, 16, 0, 128, 1);
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HF_BLADE       , 0, 16, 0, 128, 1);
}
static void SetWeaHfBladeScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText(work, OBJECT_2_MIS_VR, 1, 256, 0);

	/* モード（ＶＲ）のテキストの設定 */
	SetVRText(work, OBJECT_2_MOD_WEAPON, 1, 256, 0);

	/* 武器（ＷＥＡＰＯＮ）のテキスト設定 */
	SetWeaponText2(work, OBJECT_2_WEA_HF_BLADE);

	/* クリアレベル */
	GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);

	/* レベルの色の設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		SetAllLevelColor(work, level_num, 0, 256, clear_level);
	}
	else
	{
		SetAllLevelColor(work, level_num, 1, 16, clear_level);
	}

	/* ランキング */
	SetRanking(work, 1);

	/* アイコン */
	if(work->player != VRSEL_PLA_NINJA)
	{
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HANDGUN_1      , 0, 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HANDGUN_2      , 0, 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_ASSAULT_RIFLE_1, 0, 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_ASSAULT_RIFLE_2, 0, 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_C4_CLAYMORE_1  , 0, 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_C4_CLAYMORE_2  , 0, 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_GRENADE_1      , 0, 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_GRENADE_2      , 0, 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_PSG1           , 0, 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_STINGER        , 0, 16, 0, 128, 1);
		SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_NIKITA         , 0, 16, 0, 128, 1);
	}
	SetSpriteAlpha(&work->layoutman_2, OBJECT_2_ICO_HF_BLADE       , 0, 16, 0, 128, 1);
}
static void SetFirstPersonViewScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int level_num = 0;
	int clear_level[10];
	int cur_pos;

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText(work, OBJECT_2_MIS_VR, 1, 256, 0);

	/* モード（ＶＲ）のテキストの設定 */
	SetVRText2(work, OBJECT_2_MOD_FSV_1);

	/* クリアレベル */
	GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);

	/* カーソル位置 */
	cur_pos = LOM_GetCurObj(&work->layoutman_2);

	/* レベルの色の設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		SetAllLevelColor(work, level_num, 0, 256, clear_level);
#ifndef KP_WINDOWS
		SetSpriteColor(&work->layoutman_2, OBJECT_2_LEV_CONTROL, cur_pos == OBJECT_2_LEV_CONTROL, 256, work->i_color[clear_level[level_num]], work->a_color[clear_level[level_num]]);
#endif
	}
	else
	{
		SetAllLevelColor(work, level_num, 1, 16, clear_level);
#ifndef KP_WINDOWS
		SetSpriteABGR2(&work->layoutman_2, OBJECT_2_LEV_CONTROL, cur_pos == OBJECT_2_LEV_CONTROL, 16, work->i_color[clear_level[level_num]], work->a_color[clear_level[level_num]]);
#endif
	}

	/* ランキング */
	SetRanking(work, 1);
}
static void SetVarietyScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText(work, OBJECT_2_MIS_VR, 1, 256, 0);

	/* モード（ＶＲ）のテキストの設定 */
	SetVRText2(work, OBJECT_2_MOD_VARIETY);

	/* クリアレベル */
	GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);

	/* レベルの色の設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		SetAllLevelColor(work, level_num, 0, 256, clear_level);
	}
	else
	{
		SetAllLevelColor(work, level_num, 1, 16, clear_level);
	}

	/* ランキング */
	SetRanking(work, 1);
}
static void SetStreakingScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText(work, OBJECT_2_MIS_VR, 1, 256, 0);

	/* モード（ＶＲ）のテキストの設定 */
	SetVRText2(work, OBJECT_2_MOD_STREAKING);

	/* クリアレベル */
	GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);

	/* レベルの色の設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		SetAllLevelColor(work, level_num, 0, 256, clear_level);
	}
	else
	{
		SetAllLevelColor(work, level_num, 1, 16, clear_level);
	}

	/* ランキング */
	SetRanking(work, 1);
}
static void SetBombDisposalScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText(work, OBJECT_2_MIS_ALTERNATIVE, 1, 256, 0);

	/* モード（ＡＬＴＥＲＮＡＴＩＶＥ）のテキストの設定 */
	SetAlternativeText2(work, OBJECT_2_MOD_BOMB_D);

	/* クリアレベル */
	GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);

	/* レベルの色の設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		SetAllLevelColor(work, level_num, 0, 256, clear_level);
	}
	else
	{
		SetAllLevelColor(work, level_num, 1, 16, clear_level);
	}

	/* ランキング */
	SetRanking(work, 1);
}
static void SetEliminateScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText(work, OBJECT_2_MIS_ALTERNATIVE, 1, 256, 0);

	/* モード（ＡＬＴＥＲＮＡＴＩＶＥ）のテキストの設定 */
	SetAlternativeText2(work, OBJECT_2_MOD_ELIMINATE);

	/* クリアレベル */
	GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);

	/* レベルの色の設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		SetAllLevelColor(work, level_num, 0, 256, clear_level);
	}
	else
	{
		SetAllLevelColor(work, level_num, 1, 16, clear_level);
	}

	/* ランキング */
	SetRanking(work, 1);
}
static void SetHoldUpScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText(work, OBJECT_2_MIS_ALTERNATIVE, 1, 256, 0);

	/* モード（ＡＬＴＥＲＮＡＴＩＶＥ）のテキストの設定 */
	SetAlternativeText2(work, OBJECT_2_MOD_HOLD_UP);

	/* クリアレベル */
	GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);

	/* レベルの色の設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		SetAllLevelColor(work, level_num, 0, 256, clear_level);
	}
	else
	{
		SetAllLevelColor(work, level_num, 1, 16, clear_level);
	}

	/* ランキング */
	SetRanking(work, 1);
}
static void SetPhotographScreen(
	VRSEL_WORK *work)	/* ワーク */
{
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* ミッションのテキストの設定 */
	SetMissionsText(work, OBJECT_2_MIS_ALTERNATIVE, 1, 256, 0);

	/* モード（ＡＬＴＥＲＮＡＴＩＶＥ）のテキストの設定 */
	SetAlternativeText2(work, OBJECT_2_MOD_PHOTOGRAPH);

	/* クリアレベル */
	GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);

	/* レベルの色の設定 */
	if(work->flag & VRSEL_FLAG_PHASE_CHANGING)
	{
		SetAllLevelColor(work, level_num, 0, 256, clear_level);
	}
	else
	{
		SetAllLevelColor(work, level_num, 1, 16, clear_level);
	}

	/* ランキング */
	SetRanking(work, 1);
}

/******************************************************************************
 * Ｌ２開く画面（ミッション選択）
 */
static void SetL2Open1Screen(
	VRSEL_WORK *work)	/* ワーク */
{
	int cur_pos = 0;

	/* レイアウト２のテキストの設定 */
	if(work->count > 0)		/* 最初のフレームだけ処理しない */
		SetL2Text(work);

	/* カーソル位置 */
	switch(work->missions)
	{
	case MSN_MISSION_VR         : cur_pos = OBJECT_2_MIS_VR;          break;
	case MSN_MISSION_ALTERNATIVE: cur_pos = OBJECT_2_MIS_ALTERNATIVE; break;
	default:
		ASSERT(0);
		break;
	}

	/* ミッションのテキストの設定 */
	SetMissionsText(work, cur_pos, 0, 256, 1);
}

/******************************************************************************
 * Ｌ２開く画面（レベル選択）
 */
static void SetL2Open2Screen(
	VRSEL_WORK *work)	/* ワーク */
{
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	if(work->count > 0)		/* 最初のフレームだけ処理しない */
		SetL2Text(work);

	if(work->missions == MSN_MISSION_VR)
	{
		/* ミッションのテキストの設定 */
		SetMissionsText(work, OBJECT_2_MIS_VR, 0, 256, 0);

		if(work->mode == MSN_MODE_SNEAKING)
		{
			/* モード（ＶＲ）のテキストの設定 */
			SetVRText(work, OBJECT_2_MOD_SNEAKING, 0, 256, 0);

			/* 武器（ＳＮＥＡＫＩＮＧ）のテキストの設定 */
			if(work->weapon == MSN_WEAPON_SNEAKING)
			{
				SetSneakingText(work, OBJECT_2_WEA_SNEAKING, 0, 256, 0);
			}
			else if(work->weapon == MSN_WEAPON_ELIMINATE_ALL)
			{
				SetSneakingText(work, OBJECT_2_WEA_ELIMINATE_ALL, 0, 256, 0);
			}
			else
				ASSERT(0);
		}
		else if(work->mode == MSN_MODE_WEAPON)
		{
			/* モード（ＶＲ）のテキストの設定 */
			SetVRText(work, OBJECT_2_MOD_WEAPON, 0, 256, 0);

			/* 武器（ＷＥＡＰＯＮ）のテキスト設定 */
			if(work->weapon == MSN_WEAPON_HANDGUN)
			{
				SetWeaponText(work, OBJECT_2_WEA_HANDGUN, 0, 256, 0);
			}
			else if(work->weapon == MSN_WEAPON_ASSAULT_RIFLE)
			{
				SetWeaponText(work, OBJECT_2_WEA_ASSAULT_R, 0, 256, 0);
			}
			else if(work->weapon == MSN_WEAPON_C4_CLAYMORE)
			{
				SetWeaponText(work, OBJECT_2_WEA_C4_CLAYMORE, 0, 256, 0);
			}
			else if(work->weapon == MSN_WEAPON_GRENADE)
			{
				SetWeaponText(work, OBJECT_2_WEA_GRENADE, 0, 256, 0);
			}
			else if(work->weapon == MSN_WEAPON_PSG1)
			{
				SetWeaponText(work, OBJECT_2_WEA_PSG1, 0, 256, 0);
			}
			else if(work->weapon == MSN_WEAPON_STINGER)
			{
				SetWeaponText(work, OBJECT_2_WEA_STINGER, 0, 256, 0);
			}
			else if(work->weapon == MSN_WEAPON_NIKITA)
			{
				SetWeaponText(work, OBJECT_2_WEA_NIKITA, 0, 256, 0);
			}
			else if(work->weapon == MSN_WEAPON_HF_BLADE)
			{
				SetWeaponText(work, OBJECT_2_WEA_HF_BLADE, 0, 256, 0);
			}
			else
				ASSERT(0);
		}
		else if(work->mode == MSN_MODE_FIRST_PERSON_VIEW)
		{
			/* モード（ＶＲ）のテキストの設定 */
			SetVRText(work, OBJECT_2_MOD_FSV_1, 0, 256, 0);
		}
		else if(work->mode == MSN_MODE_VARIETY)
		{
			/* モード（ＶＲ）のテキストの設定 */
			SetVRText(work, OBJECT_2_MOD_VARIETY, 0, 256, 0);
		}
		else if(work->mode == MSN_MODE_STREAKING)
		{
			/* モード（ＶＲ）のテキストの設定 */
			SetVRText(work, OBJECT_2_MOD_STREAKING, 0, 256, 0);
		}
		else
			ASSERT(0);
	}
	else if(work->missions == MSN_MISSION_ALTERNATIVE)
	{
		/* ミッションのテキストの設定 */
		SetMissionsText(work, OBJECT_2_MIS_ALTERNATIVE, 0, 256, 0);

		/* モード（ＡＬＴＥＲＮＡＴＩＶＥ）のテキストの設定 */
		if(work->mode == MSN_MODE_BOMB_DISPOSAL)
		{
			SetAlternativeText(work, OBJECT_2_MOD_BOMB_D, 0, 256, 0);
		}
		else if(work->mode == MSN_MODE_ELIMINATE)
		{
			SetAlternativeText(work, OBJECT_2_MOD_ELIMINATE, 0, 256, 0);
		}
		else if(work->mode == MSN_MODE_HOLD_UP)
		{
			SetAlternativeText(work, OBJECT_2_MOD_HOLD_UP, 0, 256, 0);
		}
		else if(work->mode == MSN_MODE_PHOTOGRAPH)
		{
			SetAlternativeText(work, OBJECT_2_MOD_PHOTOGRAPH, 0, 256, 0);
		}
		else
			ASSERT(0);
	}
	else
		ASSERT(0);

	/* クリアレベル */
	GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);

	/* レベルの色の設定 */
	SetAllLevelColor2(work, level_num, 0, 256, clear_level);
	if(work->mode == MSN_MODE_FIRST_PERSON_VIEW)
	{
#ifndef KP_WINDOWS
		SetSpriteColor(&work->layoutman_2, OBJECT_2_LEV_CONTROL, 0, 256, work->i_color[clear_level[level_num]], work->a_color[clear_level[level_num]]);
#endif
	}

	/* ランキング */
	SetRanking(work, 1);
}

/******************************************************************************
 * Ｌ２閉じる画面（ミッション選択）
 */
static void SetL2Close1Screen(
	VRSEL_WORK *work)	/* ワーク */
{
	int cur_pos = 0;

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	/* カーソル位置 */
	switch(work->missions)
	{
	case MSN_MISSION_VR         : cur_pos = OBJECT_2_MIS_VR;          break;
	case MSN_MISSION_ALTERNATIVE: cur_pos = OBJECT_2_MIS_ALTERNATIVE; break;
	default:
		ASSERT(0);
		break;
	}

	/* ミッションのテキストの設定 */
	SetMissionsText(work, cur_pos, 0, 256, 1);
}

/******************************************************************************
 * Ｌ２閉じる画面（レベル選択）
 */
static void SetL2Close2Screen(
	VRSEL_WORK *work)	/* ワーク */
{
	int level_num = 0;
	int clear_level[10];

	/* レイアウト２のテキストの設定 */
	SetL2Text(work);

	if(work->missions == MSN_MISSION_VR)
	{
		/* ミッションのテキストの設定 */
		SetMissionsText(work, OBJECT_2_MIS_VR, 0, 256, 0);

		if(work->mode == MSN_MODE_SNEAKING)
		{
			/* モード（ＶＲ）のテキストの設定 */
			SetVRText(work, OBJECT_2_MOD_SNEAKING, 0, 256, 0);

			/* 武器（ＳＮＥＡＫＩＮＧ）のテキストの設定 */
			if(work->weapon == MSN_WEAPON_SNEAKING)
			{
				SetSneakingText(work, OBJECT_2_WEA_SNEAKING, 0, 256, 0);
			}
			else if(work->weapon == MSN_WEAPON_ELIMINATE_ALL)
			{
				SetSneakingText(work, OBJECT_2_WEA_ELIMINATE_ALL, 0, 256, 0);
			}
			else
				ASSERT(0);
		}
		else if(work->mode == MSN_MODE_WEAPON)
		{
			/* モード（ＶＲ）のテキストの設定 */
			SetVRText(work, OBJECT_2_MOD_WEAPON, 0, 256, 0);

			/* 武器（ＷＥＡＰＯＮ）のテキスト設定 */
			if(work->weapon == MSN_WEAPON_HANDGUN)
			{
				SetWeaponText(work, OBJECT_2_WEA_HANDGUN, 0, 256, 0);
			}
			else if(work->weapon == MSN_WEAPON_ASSAULT_RIFLE)
			{
				SetWeaponText(work, OBJECT_2_WEA_ASSAULT_R, 0, 256, 0);
			}
			else if(work->weapon == MSN_WEAPON_C4_CLAYMORE)
			{
				SetWeaponText(work, OBJECT_2_WEA_C4_CLAYMORE, 0, 256, 0);
			}
			else if(work->weapon == MSN_WEAPON_GRENADE)
			{
				SetWeaponText(work, OBJECT_2_WEA_GRENADE, 0, 256, 0);
			}
			else if(work->weapon == MSN_WEAPON_PSG1)
			{
				SetWeaponText(work, OBJECT_2_WEA_PSG1, 0, 256, 0);
			}
			else if(work->weapon == MSN_WEAPON_STINGER)
			{
				SetWeaponText(work, OBJECT_2_WEA_STINGER, 0, 256, 0);
			}
			else if(work->weapon == MSN_WEAPON_NIKITA)
			{
				SetWeaponText(work, OBJECT_2_WEA_NIKITA, 0, 256, 0);
			}
			else if(work->weapon == MSN_WEAPON_HF_BLADE)
			{
				SetWeaponText(work, OBJECT_2_WEA_HF_BLADE, 0, 256, 0);
			}
			else
				ASSERT(0);
		}
		else if(work->mode == MSN_MODE_FIRST_PERSON_VIEW)
		{
			/* モード（ＶＲ）のテキストの設定 */
			SetVRText(work, OBJECT_2_MOD_FSV_1, 0, 256, 0);
		}
		else if(work->mode == MSN_MODE_VARIETY)
		{
			/* モード（ＶＲ）のテキストの設定 */
			SetVRText(work, OBJECT_2_MOD_VARIETY, 0, 256, 0);
		}
		else if(work->mode == MSN_MODE_STREAKING)
		{
			/* モード（ＶＲ）のテキストの設定 */
			SetVRText(work, OBJECT_2_MOD_STREAKING, 0, 256, 0);
		}
		else
			ASSERT(0);
	}
	else if(work->missions == MSN_MISSION_ALTERNATIVE)
	{
		/* ミッションのテキストの設定 */
		SetMissionsText(work, OBJECT_2_MIS_ALTERNATIVE, 0, 256, 0);

		/* モード（ＡＬＴＥＲＮＡＴＩＶＥ）のテキストの設定 */
		if(work->mode == MSN_MODE_BOMB_DISPOSAL)
		{
			SetAlternativeText(work, OBJECT_2_MOD_BOMB_D, 0, 256, 0);
		}
		else if(work->mode == MSN_MODE_ELIMINATE)
		{
			SetAlternativeText(work, OBJECT_2_MOD_ELIMINATE, 0, 256, 0);
		}
		else if(work->mode == MSN_MODE_HOLD_UP)
		{
			SetAlternativeText(work, OBJECT_2_MOD_HOLD_UP, 0, 256, 0);
		}
		else if(work->mode == MSN_MODE_PHOTOGRAPH)
		{
			SetAlternativeText(work, OBJECT_2_MOD_PHOTOGRAPH, 0, 256, 0);
		}
		else
			ASSERT(0);
	}
	else
		ASSERT(0);

	/* クリアレベル */
	GetClearLevel(work, work->missions, work->mode, work->weapon, &level_num, &clear_level[0]);

	/* レベルの色の設定 */
	if(SetAllLevelColor2(work, level_num, 0, 256, clear_level) == 0)
	{
#ifndef KP_WINDOWS
		if(work->mode == MSN_MODE_FIRST_PERSON_VIEW)
			SetSpriteColor(&work->layoutman_2, OBJECT_2_LEV_CONTROL, 1, 256, work->i_color[clear_level[level_num]], work->a_color[clear_level[level_num]]);
#endif
	}
	else
	{
#ifndef KP_WINDOWS
		if(work->mode == MSN_MODE_FIRST_PERSON_VIEW)
			SetSpriteColor(&work->layoutman_2, OBJECT_2_LEV_CONTROL, 0, 256, work->i_color[clear_level[level_num]], work->a_color[clear_level[level_num]]);
#endif
	}

	/* ランキング */
	SetRanking(work, 0);
}

/*******************************************************************************
 * public
 */
/*******************************************************************************
 * スプライトの表示設定
 */
void VRSEL_ShowSprite(
	LAYOUTMAN     *layoutman,	/* レイアウトマネージャ */
	int           strcode,		/* 文字列コード */
	int           show)			/* 表示フラグ */
{
	int flags;

	if(show == 0)
	{
		LOM_Spr_GetFlags(layoutman, strcode, &flags); flags |= SPR_FLAG_HIDDEN;
		LOM_Spr_SetFlags(layoutman, strcode,  flags);
	}
	else
	{
		LOM_Spr_GetFlags(layoutman, strcode, &flags); flags &= ~SPR_FLAG_HIDDEN;
		LOM_Spr_SetFlags(layoutman, strcode,  flags);
	}
}

/******************************************************************************
 * 入力の許可/禁止の設定
 */
void VRSEL_InputOK(
	VRSEL_WORK *work)	/* ワーク */
{
	work->flag  |=   VRSEL_FLAG_INPUT_OK;
	work->flag  &= ~(VRSEL_FLAG_WAIT_ACTION | VRSEL_FLAG_WAIT_ACTION2);
	work->flag  &= ~VRSEL_FLAG_PHASE_CHANGING;
	work->count2 = 1;

	if(work->flag & VRSEL_FLAG_LAYOUT_2)
	{
		if(work->phase < VRSEL_PHASE_LEV_SNEAKING)
		{
			work->layoutman_2.lom_icolist[0].lom_ico[0].sprite->head.flags &= ~SPR_FLAG_HIDDEN;
		}
		else
		{
			work->layoutman_2.lom_icolist[1].lom_ico[0].sprite->head.flags &= ~SPR_FLAG_HIDDEN;
		}
	}
	else
	{
		work->layoutman_1.lom_icolist[0].lom_ico[0].sprite->head.flags &= ~SPR_FLAG_HIDDEN;
	}
}
void VRSEL_InputNG(
	VRSEL_WORK *work)	/* ワーク */
{
	work->flag  &= ~VRSEL_FLAG_INPUT_OK;
	work->flag  &= ~(VRSEL_FLAG_WAIT_ACTION | VRSEL_FLAG_WAIT_ACTION2);
	work->count2 = 1;

	if(work->flag & VRSEL_FLAG_LAYOUT_2)
	{
		if(work->phase < VRSEL_PHASE_LEV_SNEAKING)
		{
			work->layoutman_2.lom_icolist[0].lom_ico[0].sprite->head.flags |= SPR_FLAG_HIDDEN;
		}
		else
		{
			work->layoutman_2.lom_icolist[1].lom_ico[0].sprite->head.flags |= SPR_FLAG_HIDDEN;
		}
	}
	else
	{
		work->layoutman_1.lom_icolist[0].lom_ico[0].sprite->head.flags |= SPR_FLAG_HIDDEN;
	}
}
void VRSEL_InputStop(
	VRSEL_WORK *work,	/* ワーク */
	short      stop)	/* 停止フレーム数 */
{
	work->flag  &= ~VRSEL_FLAG_INPUT_OK;
	work->count2 = -stop;
}
void VRSEL_InputStop2(
	VRSEL_WORK *work,	/* ワーク */
	short      stop)	/* 停止フレーム数 */
{
	work->flag  &= ~VRSEL_FLAG_INPUT_OK;
	work->flag  |=  VRSEL_FLAG_WAIT_ACTION | VRSEL_FLAG_WAIT_ACTION2;
	work->count2 = -stop;

	if(work->flag & VRSEL_FLAG_LAYOUT_2)
	{
		if(work->phase < VRSEL_PHASE_LEV_SNEAKING)
		{
			work->layoutman_2.lom_icolist[0].lom_ico[0].sprite->head.flags |= SPR_FLAG_HIDDEN;
		}
		else
		{
			work->layoutman_2.lom_icolist[1].lom_ico[0].sprite->head.flags |= SPR_FLAG_HIDDEN;
		}
	}
	else
	{
		work->layoutman_1.lom_icolist[0].lom_ico[0].sprite->head.flags |= SPR_FLAG_HIDDEN;
	}
}

/******************************************************************************
 * フェーズの設定
 */
void VRSEL_SetPhase(
	VRSEL_WORK *work,	/* ワーク */
	int        phase)	/* フェーズ */
{
	if(work->phase != phase)
	{
		work->phase = phase;
		work->count = 0;
		work->flag |= VRSEL_FLAG_PHASE_CHANGING;
	}
}

/*******************************************************************************
 * カレントＬＯＭモードの設定（拡張）
 */
int VRSEL_LOM_SetCurMode(	/* 1: 成功 */
							/* 0: 失敗 */
	VRSEL_WORK *work,			/* ワーク */
	LAYOUTMAN  *layoutman,		/* レイアウトマネージャ */
	int        lom_mode_name,	/* ＬＯＭモード名 */
	int        default_act,		/* 1: ＬＯＭアクションを　　デフォルトに設定する  0: しない  -1: 処理をしない */
	int        default_ico,		/* 1: ＬＯＭアイコンを　　　デフォルトに設定する  0: しない */
	int        default_obj)		/* 1: ＬＯＭオブジェクトを　デフォルトに設定する  0: しない */
{
	if(layoutman == &work->layoutman_1)
	{
		/* レイアウトマネージャ１ */
		switch(lom_mode_name)
		{
		case LOM_MODE_00: break;
		case LOM_MODE_01: VRSEL_SetPhase(work, VRSEL_PHASE_L1_OPEN ); break;
		case LOM_MODE_02: VRSEL_SetPhase(work, VRSEL_PHASE_PLA     ); break;
		case LOM_MODE_03: VRSEL_SetPhase(work, VRSEL_PHASE_L1_CLOSE); break;

		default:
			ASSERT(0);
			break;
		}
	}
	else
	{
		/* レイアウトマネージャ２ */
		switch(lom_mode_name)
		{
		case LOM_MODE_00: break;
		case LOM_MODE_01: VRSEL_SetPhase(work, VRSEL_PHASE_L2_OPEN_1); break;
		case LOM_MODE_02:
		case LOM_MODE_03:
		case LOM_MODE_04:
		case LOM_MODE_05:
		case LOM_MODE_06:
		case LOM_MODE_07:
		case LOM_MODE_08:
		case LOM_MODE_09:
		case LOM_MODE_10:
		case LOM_MODE_11:
		case LOM_MODE_12:
		case LOM_MODE_13:
		case LOM_MODE_14:
		case LOM_MODE_15:
		case LOM_MODE_16:
		case LOM_MODE_17:
		case LOM_MODE_18: VRSEL_SetPhase(work, VRSEL_PHASE_L2_OPEN_2); break;
		case LOM_MODE_19: VRSEL_SetPhase(work, VRSEL_PHASE_L2_CLOSE_1); break;
		case LOM_MODE_20: VRSEL_SetPhase(work, VRSEL_PHASE_L2_CLOSE_2); break;

		case LOM_MODE_21:
		case LOM_MODE_22:
		case LOM_MODE_23:
		case LOM_MODE_24:
		case LOM_MODE_25:
			VRSEL_SetPhase(work, VRSEL_PHASE_MIS);
			break;

		case LOM_MODE_26:
		case LOM_MODE_27:
		case LOM_MODE_28:
		case LOM_MODE_29:
		case LOM_MODE_30:
		case LOM_MODE_31:
		case LOM_MODE_32:
		case LOM_MODE_33:
		case LOM_MODE_34:
		case LOM_MODE_35:
		case LOM_MODE_36:
			VRSEL_SetPhase(work, VRSEL_PHASE_MOD_VR);
			break;

		case LOM_MODE_37:
		case LOM_MODE_38:
		case LOM_MODE_39:
		case LOM_MODE_40:
		case LOM_MODE_41:
		case LOM_MODE_42:
		case LOM_MODE_43:
		case LOM_MODE_44:
		case LOM_MODE_45:
		case LOM_MODE_46:
		case LOM_MODE_47:
		case LOM_MODE_48:
			VRSEL_SetPhase(work, VRSEL_PHASE_MOD_ALT);
			break;

		case LOM_MODE_49:
		case LOM_MODE_50:
		case LOM_MODE_51:
			VRSEL_SetPhase(work, VRSEL_PHASE_WEA_SNEAKING);
			break;

		case LOM_MODE_52:
		case LOM_MODE_53:
		case LOM_MODE_54:
		case LOM_MODE_55:
		case LOM_MODE_56:
		case LOM_MODE_57:
		case LOM_MODE_58:
		case LOM_MODE_59:
		case LOM_MODE_60:
		case LOM_MODE_61:
		case LOM_MODE_62:
			VRSEL_SetPhase(work, VRSEL_PHASE_WEA_WEAPON);
			break;

		case LOM_MODE_63:
		case LOM_MODE_64: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_FIRST_PERSON_VIEW); break;
		case LOM_MODE_65:
		case LOM_MODE_66: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_VARIETY          ); break;
		case LOM_MODE_67:
		case LOM_MODE_68: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_VARIETY          ); break;
		case LOM_MODE_69:
		case LOM_MODE_70: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_VARIETY          ); break;
		case LOM_MODE_71:
		case LOM_MODE_72: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_VARIETY          ); break;
		case LOM_MODE_73:
		case LOM_MODE_74: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_STREAKING        ); break;
		case LOM_MODE_75:
		case LOM_MODE_76: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_BOMB_DISPOSAL    ); break;
		case LOM_MODE_77:
		case LOM_MODE_78: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_ELIMINATE        ); break;
		case LOM_MODE_79:
		case LOM_MODE_80: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_HOLD_UP          ); break;
		case LOM_MODE_81:
		case LOM_MODE_82: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_PHOTOGRAPH       ); break;
		case LOM_MODE_83:
		case LOM_MODE_84: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_PHOTOGRAPH       ); break;
		case LOM_MODE_85:
		case LOM_MODE_86: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_SNEAKING         ); break;
		case LOM_MODE_87:
		case LOM_MODE_88: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_ELIMINATE_ALL    ); break;
		case LOM_MODE_89:
		case LOM_MODE_90: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_HANDGUN          ); break;
		case LOM_MODE_91:
		case LOM_MODE_92: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_ASSAULT_RIFLE    ); break;
		case LOM_MODE_93:
		case LOM_MODE_94: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_C4_CLAYMORE      ); break;
		case LOM_MODE_95:
		case LOM_MODE_96: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_GRENADE          ); break;
		case LOM_MODE_97:
		case LOM_MODE_98: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_PSG1             ); break;
		case LOM_MODE_99:
		case LOM_MODE_100: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_STINGER          ); break;
		case LOM_MODE_101:
		case LOM_MODE_102: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_NIKITA           ); break;
		case LOM_MODE_103:
		case LOM_MODE_104: VRSEL_SetPhase(work, VRSEL_PHASE_LEV_HF_BLADE         ); break;

		case LOM_MODE_105: VRSEL_SetPhase(work, VRSEL_PHASE_HIDE_FIRST_PERSON_VIEW); break;
		case LOM_MODE_106: VRSEL_SetPhase(work, VRSEL_PHASE_HIDE_VARIETY          ); break;
		case LOM_MODE_107: VRSEL_SetPhase(work, VRSEL_PHASE_HIDE_STREAKING        ); break;
		case LOM_MODE_108: VRSEL_SetPhase(work, VRSEL_PHASE_HIDE_BOMB_DISPOSAL    ); break;
		case LOM_MODE_109: VRSEL_SetPhase(work, VRSEL_PHASE_HIDE_ELIMINATE        ); break;
		case LOM_MODE_110: VRSEL_SetPhase(work, VRSEL_PHASE_HIDE_HOLD_UP          ); break;
		case LOM_MODE_111: VRSEL_SetPhase(work, VRSEL_PHASE_HIDE_PHOTOGRAPH       ); break;
		case LOM_MODE_112: VRSEL_SetPhase(work, VRSEL_PHASE_HIDE_SNEAKING         ); break;
		case LOM_MODE_113: VRSEL_SetPhase(work, VRSEL_PHASE_HIDE_ELIMINATE_ALL    ); break;
		case LOM_MODE_114: VRSEL_SetPhase(work, VRSEL_PHASE_HIDE_HANDGUN          ); break;
		case LOM_MODE_115: VRSEL_SetPhase(work, VRSEL_PHASE_HIDE_ASSAULT_RIFLE    ); break;
		case LOM_MODE_116: VRSEL_SetPhase(work, VRSEL_PHASE_HIDE_C4_CLAYMORE      ); break;
		case LOM_MODE_117: VRSEL_SetPhase(work, VRSEL_PHASE_HIDE_GRENADE          ); break;
		case LOM_MODE_118: VRSEL_SetPhase(work, VRSEL_PHASE_HIDE_PSG1             ); break;
		case LOM_MODE_119: VRSEL_SetPhase(work, VRSEL_PHASE_HIDE_STINGER          ); break;
		case LOM_MODE_120: VRSEL_SetPhase(work, VRSEL_PHASE_HIDE_NIKITA           ); break;
		case LOM_MODE_121: VRSEL_SetPhase(work, VRSEL_PHASE_HIDE_HF_BLADE         ); break;

		default:
			ASSERT(0);
			break;
		}
	}
	return LOM_SetCurMode(layoutman, lom_mode_name, default_act, default_ico, default_obj);
}

/******************************************************************************
 * 毎フレーム処理
 */
void VRSEL_Act(
	VRSEL_WORK *work)	/* ワーク */
{
	int flags;
	static int alpha;

	if(work->flag & VRSEL_FLAG_INIT)
	{
		/*
		 * レイアウトの初期化
		 */
		work->flag &= ~VRSEL_FLAG_INIT;
		work->flag |=  VRSEL_FLAG_INIT2;
		return;
	}
	if(work->flag & VRSEL_FLAG_INIT2)
	{
		/*
		 * レイアウトの初期化
		 */
		work->flag &= ~VRSEL_FLAG_INIT2;

		/* テクスチャのＵ原点の取得 */
		LOM_SprTex_GetU(&work->layoutman_1, OBJECT_1_001_RAIDEN, &work->tex1_u0);
		SY_PRINTF1("tex1 u0: %f\n", work->tex1_u0);
		if(!(work->flag & VRSEL_FLAG_NO_PLAYER))
		{
			LOM_SprTex_GetU(&work->layoutman_2, OBJECT_2_REC_1ST_1, &work->tex2_u0);
			SY_PRINTF1("tex2 u0: %f\n", work->tex2_u0);
		}

		/* モードの設定 */
		if(work->flag & VRSEL_FLAG_NO_PLAYER)
		{
			VRSEL_LOM_SetCurMode(work, &work->layoutman_1, LOM_MODE_01, 1, 1, 1);
		}
		else
		{
			VRSEL_LOM_SetCurMode(work, &work->layoutman_1, LOM_MODE_00, 1, 1, 1);

			if(MSN_STAGE_ID != -1)
			{
				if(work->missions == MSN_MISSION_VR)
				{
					if(work->mode == MSN_MODE_SNEAKING)
					{
						if(work->weapon == MSN_WEAPON_SNEAKING)
						{
							VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_09, 1, 1, 1);
						}
						else if(work->weapon == MSN_WEAPON_ELIMINATE_ALL)
						{
							VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_10, 1, 1, 1);
						}
						else
							ASSERT(0);
					}
					else if(work->mode == MSN_MODE_WEAPON)
					{
						if(work->weapon == MSN_WEAPON_HANDGUN)
						{
							VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_11, 1, 1, 1);
						}
						else if(work->weapon == MSN_WEAPON_ASSAULT_RIFLE)
						{
							VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_12, 1, 1, 1);
						}
						else if(work->weapon == MSN_WEAPON_C4_CLAYMORE)
						{
							VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_13, 1, 1, 1);
						}
						else if(work->weapon == MSN_WEAPON_GRENADE)
						{
							VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_14, 1, 1, 1);
						}
						else if(work->weapon == MSN_WEAPON_PSG1)
						{
							VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_15, 1, 1, 1);
						}
						else if(work->weapon == MSN_WEAPON_STINGER)
						{
							VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_16, 1, 1, 1);
						}
						else if(work->weapon == MSN_WEAPON_NIKITA)
						{
							VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_17, 1, 1, 1);
						}
						else if(work->weapon == MSN_WEAPON_HF_BLADE)
						{
							VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_18, 1, 1, 1);
						}
						else
							ASSERT(0);
					}
					else if(work->mode == MSN_MODE_FIRST_PERSON_VIEW)
					{
						VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_02, 1, 1, 1);
					}
					else if(work->mode == MSN_MODE_VARIETY)
					{
						VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_03, 1, 1, 1);
					}
					else if(work->mode == MSN_MODE_STREAKING)
					{
						VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_04, 1, 1, 1);
					}
					else
						ASSERT(0);
				}
				else if(work->missions == MSN_MISSION_ALTERNATIVE)
				{
					if(work->mode == MSN_MODE_BOMB_DISPOSAL)
					{
						VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_05, 1, 1, 1);
					}
					else if(work->mode == MSN_MODE_ELIMINATE)
					{
						VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_06, 1, 1, 1);
					}
					else if(work->mode == MSN_MODE_HOLD_UP)
					{
						VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_07, 1, 1, 1);
					}
					else if(work->mode == MSN_MODE_PHOTOGRAPH)
					{
						VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_08, 1, 1, 1);
					}
					else
						ASSERT(0);
				}
				else
					ASSERT(0);
			}
			else
			{
				VRSEL_LOM_SetCurMode(work, &work->layoutman_2, LOM_MODE_01, 1, 1, 1);
			}
		}
		GM_SdSet(SD_S_LINEMOV1);

		/* 入力不可 */
		VRSEL_InputNG(work);
	}

	/* キーコンフィグ選択 */
	if(work->keyconsel_ret != -2)
	{
		/* 起動中 */
		if(work->keyconsel_ret == 0)
			return;
		else
		{
			if(work->keyconsel_ret == -3)
			{
				/* フェード用スプライト */
				if(alpha > 16) alpha -= 16;
				else           alpha  =  0;
				SPR_SetColorSprite(work->fade, 0, 0, 0, alpha);
				if(alpha != 0)
					return;
				work->fade->sprite.head.head.flags |= SPR_FLAG_HIDDEN;

				work->keyconsel_ret = -2;

				/* 強引に非表示にしていたのを表示に */
				work->layoutman_2.lom_icolist[1].lom_ico[0].sprite->head.flags &= ~SPR_FLAG_HIDDEN;

				/* 入力可 */
				VRSEL_InputOK(work);
			}
			else
			{
				if(work->keyconsel_ret != -1)
				{
					/* プロック呼び出し */
					GCL_ARGS args;
					int      buf[1];
					args.argv = buf;
					args.argc = 1;
					buf[0]    = work->keyconsel_ret;
					ASSERT(work->proc_keyconfig);
					SY_PRINTF1("PROC CALLED\n");
					GM_ExecProc(work->proc_keyconfig, &args);

					work->keyconsel_set = work->keyconsel_ret;
				}
				work->keyconsel_ret = -3;

				/* フェード用スプライト */
				alpha = 128;
				SPR_SetColorSprite(work->fade, 0, 0, 0, alpha);
				work->fade->sprite.head.head.flags &= ~SPR_FLAG_HIDDEN;

				/* 強引に非表示にしていたのを表示に */
				LOM_Emp_GetFlags(&work->layoutman_2, 2770484 /* ROOT */, &flags);
				flags &= ~SPR_FLAG_HIDDEN;
				LOM_Emp_SetFlags(&work->layoutman_2, 2770484 /* ROOT */,  flags);
				return;
			}
		}
	}

	/* ＶＲクリアコード */
	if(work->vrclc_status != -1)
	{
		/* 起動中 */
		if(work->vrclc_status == 0)
			return;
		else
		{
			if(work->vrclc_status == -2)
			{
				/* フェード用スプライト */
				if(alpha > 16) alpha -= 16;
				else           alpha  =  0;
				SPR_SetColorSprite(work->fade, 0, 0, 0, alpha);
				if(alpha != 0)
					return;
				work->fade->sprite.head.head.flags |= SPR_FLAG_HIDDEN;

				work->vrclc_status = -1;

				/* 強引に非表示にしていたのを表示に */
				work->layoutman_2.lom_icolist[1].lom_ico[0].sprite->head.flags &= ~SPR_FLAG_HIDDEN;

				/* 入力可 */
				VRSEL_InputOK(work);
			}
			else
			{
				work->vrclc_status = -2;

				/* フェード用スプライト */
				alpha = 128;
				SPR_SetColorSprite(work->fade, 0, 0, 0, alpha);
				work->fade->sprite.head.head.flags &= ~SPR_FLAG_HIDDEN;

				/* 強引に非表示にしていたのを表示に */
				LOM_Emp_GetFlags(&work->layoutman_2, 2770484 /* ROOT */, &flags);
				flags &= ~SPR_FLAG_HIDDEN;
				LOM_Emp_SetFlags(&work->layoutman_2, 2770484 /* ROOT */,  flags);
				return;
			}
		}
	}

	/* レイアウトマネージャ */
	if(work->flag & VRSEL_FLAG_LAYOUT_2)
	{
		ActLayoutman(&work->layoutman_2);
	}
	else
	{
		ActLayoutman(&work->layoutman_1);
	}

	/* 各フェーズ */
	switch(work->phase)
	{
	case VRSEL_PHASE_PLA:
		SetPlayerScreen(work);
		break;
	case VRSEL_PHASE_L1_OPEN:
		if( BP_IsPAL()==TRUE && work->count == 7) GM_SdSet(SD_S_WINOPN01);
		if( BP_IsPAL()!=TRUE && work->count == 9) GM_SdSet(SD_S_WINOPN01);

      SetL1OpenScreen(work);
		break;
	case VRSEL_PHASE_L1_CLOSE:
		if( BP_IsPAL()==TRUE && work->count == 7) GM_SdSet(SD_S_LINEMOV1);
		if( BP_IsPAL()!=TRUE && work->count == 9) GM_SdSet(SD_S_LINEMOV1);

      SetL1CloseScreen(work);
		break;
	case VRSEL_PHASE_MIS:
		SetMissionsScreen(work);
		break;
	case VRSEL_PHASE_MOD_VR:
		SetVRScreen(work);
		break;
	case VRSEL_PHASE_MOD_ALT:
		SetAlternativeScreen(work);
		break;
	case VRSEL_PHASE_WEA_SNEAKING:
		SetSneakingScreen(work);
		break;
	case VRSEL_PHASE_WEA_WEAPON:
		SetWeaponScreen(work);
		break;
	case VRSEL_PHASE_LEV_SNEAKING:
		SetSneSneakingScreen(work);
		break;
	case VRSEL_PHASE_LEV_ELIMINATE_ALL:
		SetSneEliminateAllScreen(work);
		break;
	case VRSEL_PHASE_LEV_HANDGUN:
		SetWeaHandgunScreen(work);
		break;
	case VRSEL_PHASE_LEV_ASSAULT_RIFLE:
		SetWeaAssaultRifleScreen(work);
		break;
	case VRSEL_PHASE_LEV_C4_CLAYMORE:
		SetWeaC4ClaymoreScreen(work);
		break;
	case VRSEL_PHASE_LEV_GRENADE:
		SetWeaGrenadeScreen(work);
		break;
	case VRSEL_PHASE_LEV_PSG1:
		SetWeaPsg1Screen(work);
		break;
	case VRSEL_PHASE_LEV_STINGER:
		SetWeaStingerScreen(work);
		break;
	case VRSEL_PHASE_LEV_NIKITA:
		SetWeaNikitaScreen(work);
		break;
	case VRSEL_PHASE_LEV_HF_BLADE:
		SetWeaHfBladeScreen(work);
		break;
	case VRSEL_PHASE_LEV_FIRST_PERSON_VIEW:
		SetFirstPersonViewScreen(work);
		break;
	case VRSEL_PHASE_LEV_VARIETY:
		SetVarietyScreen(work);
		break;
	case VRSEL_PHASE_LEV_STREAKING:
		SetStreakingScreen(work);
		break;
	case VRSEL_PHASE_LEV_BOMB_DISPOSAL:
		SetBombDisposalScreen(work);
		break;
	case VRSEL_PHASE_LEV_ELIMINATE:
		SetEliminateScreen(work);
		break;
	case VRSEL_PHASE_LEV_HOLD_UP:
		SetHoldUpScreen(work);
		break;
	case VRSEL_PHASE_LEV_PHOTOGRAPH:
		SetPhotographScreen(work);
		break;
	case VRSEL_PHASE_HIDE_SNEAKING:
		SetL2Close2Screen(work);
		break;
	case VRSEL_PHASE_HIDE_ELIMINATE_ALL:
		SetL2Close2Screen(work);
		break;
	case VRSEL_PHASE_HIDE_HANDGUN:
		SetL2Close2Screen(work);
		break;
	case VRSEL_PHASE_HIDE_ASSAULT_RIFLE:
		SetL2Close2Screen(work);
		break;
	case VRSEL_PHASE_HIDE_C4_CLAYMORE:
		SetL2Close2Screen(work);
		break;
	case VRSEL_PHASE_HIDE_GRENADE:
		SetL2Close2Screen(work);
		break;
	case VRSEL_PHASE_HIDE_PSG1:
		SetL2Close2Screen(work);
		break;
	case VRSEL_PHASE_HIDE_STINGER:
		SetL2Close2Screen(work);
		break;
	case VRSEL_PHASE_HIDE_NIKITA:
		SetL2Close2Screen(work);
		break;
	case VRSEL_PHASE_HIDE_HF_BLADE:
		SetL2Close2Screen(work);
		break;
	case VRSEL_PHASE_HIDE_FIRST_PERSON_VIEW:
		SetL2Close2Screen(work);
		break;
	case VRSEL_PHASE_HIDE_VARIETY:
		SetL2Close2Screen(work);
		break;
	case VRSEL_PHASE_HIDE_STREAKING:
		SetL2Close2Screen(work);
		break;
	case VRSEL_PHASE_HIDE_BOMB_DISPOSAL:
		SetL2Close2Screen(work);
		break;
	case VRSEL_PHASE_HIDE_ELIMINATE:
		SetL2Close2Screen(work);
		break;
	case VRSEL_PHASE_HIDE_HOLD_UP:
		SetL2Close2Screen(work);
		break;
	case VRSEL_PHASE_HIDE_PHOTOGRAPH:
		SetL2Close2Screen(work);
		break;
	case VRSEL_PHASE_L2_OPEN_1:

      if( BP_IsPAL()==TRUE && work->count == 7) GM_SdSet(SD_S_WINOPN01);
		if( BP_IsPAL()!=TRUE && work->count == 9) GM_SdSet(SD_S_WINOPN01);
		SetL2Open1Screen(work);
		break;
	case VRSEL_PHASE_L2_OPEN_2:
		if( BP_IsPAL()==TRUE && work->count == 7) GM_SdSet(SD_S_WINOPN01);
		if( BP_IsPAL()!=TRUE && work->count == 9) GM_SdSet(SD_S_WINOPN01);
		SetL2Open2Screen(work);
		break;
	case VRSEL_PHASE_L2_CLOSE_1:
		if( BP_IsPAL()==TRUE && work->count == 7) GM_SdSet(SD_S_LINEMOV1);
      if( BP_IsPAL()!=TRUE && work->count == 9) GM_SdSet(SD_S_LINEMOV1);

      SetL2Close1Screen(work);
		break;
	case VRSEL_PHASE_L2_CLOSE_2:
		if( BP_IsPAL()==TRUE && work->count == 7) GM_SdSet(SD_S_LINEMOV1);
		if( BP_IsPAL()!=TRUE && work->count == 9) GM_SdSet(SD_S_LINEMOV1);
		SetL2Close2Screen(work);
		break;
	default:
		ASSERT(0);
		break;
	}

	/* 入力制御 */
	if(work->count2 == 0)
	{
		VRSEL_InputOK(work);
	}
	if(work->flag & VRSEL_FLAG_WAIT_ACTION2)
	{
		work->flag &= ~VRSEL_FLAG_WAIT_ACTION2;
	}
	else if(work->flag & VRSEL_FLAG_WAIT_ACTION)
	{
		if(work->flag & VRSEL_FLAG_LAYOUT_2)
		{
			if(L2D_ActionStatus(work->layoutman_2.layout) != L2D_STAT_BUSY)
			{
				VRSEL_InputOK(work);
			}
		}
		else
		{
			if(L2D_ActionStatus(work->layoutman_1.layout) != L2D_STAT_BUSY)
			{
				VRSEL_InputOK(work);
			}
		}
	}
	work->count++;
	work->count2++;
	if(work->count > 30000)
		work->count = 30000;
	if(work->count2 > 30000)
		work->count2 = 30000;

/* デバッグモード */
#ifdef DEBUG_MODE
	VRSEL_Dbg_Act(work);
#endif
}
