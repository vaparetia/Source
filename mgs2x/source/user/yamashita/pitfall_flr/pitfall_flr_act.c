//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * pitfall_flr - pitfall_flr_act.c
 * 落とし穴の床 (毎フレーム処理)
 * 2002/03/13 S.Yamashita
 * $Id: pitfall_flr_act.c,v 1.1.1.3 2002/11/19 11:51:53 Yoshizawa1 Exp $
 */

/*******************************************************************************
 * include
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "_pitfall_flr.h"

/*******************************************************************************
 * local
 */

static char alt = 0;

/*******************************************************************************
 * fuctions
 */

static inline void BRK_FLR_ActReset(WORK *work);
static inline void BRK_FLR_ActShakeStart(WORK *work);
static inline void BRK_FLR_ActShake(WORK *work);
static inline void BRK_FLR_ActSwingStart(WORK *work);
static inline void BRK_FLR_ActSwing(WORK *work);
static inline void BRK_FLR_ActFallStart(WORK *work);
static inline void BRK_FLR_ActFall(WORK *work);
static inline void BRK_FLR_ActLaydownStart(WORK *work);
static inline void BRK_FLR_ActLaydown(WORK *work);
static inline void BRK_FLR_ActReapperStart(WORK *work);
static inline void BRK_FLR_ActReapper(WORK *work);

/*******************************************************************************
 * public
 */
/*******************************************************************************
 * 毎フレーム処理
 */
void PFF_Act(
	WORK *work)		/* ワーク */
{
	switch(work->mode)
	{
	/* 初期状態 */
	case BRK_FLR_ModeReset: BRK_FLR_ActReset(work);
	case BRK_FLR_ModeNone :
		break;

	/* 消滅 */
	case BRK_FLR_ModeShakeStart: BRK_FLR_ActShakeStart(work);
	case BRK_FLR_ModeShake     : BRK_FLR_ActShake(work);
		break;

	/* 落下前の振り子 */
	case BRK_FLR_ModeSwingStart: BRK_FLR_ActSwingStart(work);
	case BRK_FLR_ModeSwing     : BRK_FLR_ActSwing(work);
		break;

	/* 崩落 */
	case BRK_FLR_ModeFallStart: BRK_FLR_ActFallStart(work);
	case BRK_FLR_ModeFall     : BRK_FLR_ActFall(work);
		break;

	/* 倒れる */
	case BRK_FLR_ModeLaydownStart: BRK_FLR_ActLaydownStart(work);
	case BRK_FLR_ModeLaydown     : BRK_FLR_ActLaydown(work);
		break;

	/* 再表示 */
	case BRK_FLR_ModeReapperStart: BRK_FLR_ActReapperStart(work);
	case BRK_FLR_ModeReapper     : BRK_FLR_ActReapper(work);
		break;
	}
}

/*******************************************************************************
 * static
 */
/*******************************************************************************
 * 初期状態
 */
static inline void BRK_FLR_ActReset(
	WORK *work)		/* ワーク */
{
	SY_PRINTF3("BRK_FLR_ActReset\n");

	work->mode = BRK_FLR_ModeNone;
}

/*******************************************************************************
 * 消滅
 */
static inline void BRK_FLR_ActShakeStart(
	WORK *work)		/* ワーク */
{
	SY_PRINTF3("BRK_FLR_ActShakeStart\n");

	work->mode    = BRK_FLR_ModeShake;
	work->value2  = work->value / 3;
	work->value3  = work->value - work->value2;
	work->value   = work->value2;
	work->value2 -= work->value2 / 2;

	if(alt == 0)
	{
		alt = 1;
#ifdef KP_XBOX	// ADD M.Kobayashi 2002/11/08
		GM_SeSetModeAddr(SD_A_V_HOLE03, &work->pos, GM_SEMODE_BOMB, GM_INVALID_ADDR );
#else		
		GM_SeSetMode(SD_A_V_HOLE03, &work->pos, GM_SEMODE_BOMB);
#endif		
	}
	else
	{
		alt = 0;
#ifdef KP_XBOX	// ADD M.Kobayashi 2002/11/08
		GM_SeSetModeAddr(SD_A_V_HOLE13, &work->pos, GM_SEMODE_BOMB, GM_INVALID_ADDR );
#else		
		GM_SeSetMode(SD_A_V_HOLE13, &work->pos, GM_SEMODE_BOMB);
#endif		
	}
}

static inline void BRK_FLR_ActShake(
	WORK *work)		/* ワーク */
{
	float          a;
	DG_PRIM2_UVRGB *dst;

	if(work->value != 0)
	{
		if(work->value - work->value2 < 0)
			a = (work->value * ALPHA_GLARE) / work->value2;
		else
			a = ((work->value2 * 2 - work->value) * ALPHA_GLARE) / work->value2;

		DG_SwitchBuffPrim2(work->prim);
		dst = work->prim->uvrgb[work->prim->buffer_clock];
		sy_setrgba(dst, DEF_RGB, DEF_RGB, DEF_RGB, DEF_ALPHA + (u_short)a, 4);
	}

	if(--work->value <= 0)
	{
		work->mode  = BRK_FLR_ModeSwing;
		work->value = work->value3;
	}
}

/*******************************************************************************
 * 落下前の振り子
 */
static inline void BRK_FLR_ActSwingStart(
	WORK *work)		/* ワーク */
{
	SY_PRINTF3("BRK_FLR_ActSwingStart\n");

	work->mode  = BRK_FLR_ModeSwing;
	work->value = 0;
}

static inline void BRK_FLR_ActSwing(
	WORK *work)		/* ワーク */
{
	int            index;
	DG_PRIM2_UVRGB *dst, *uvrgb;

	/* 分割インデックスの決定 */
	if(work->value <= 1) index = N_DIV - 1;
	else                 index = (int)(((work->value3 - work->value) / (float)(work->value3 - 1)) * (N_DIV - 2)) + 1;

	/* テクスチャアニメーション */
	DG_SwitchBuffPrim2(work->prim);
	dst   = work->prim->uvrgb[work->prim->buffer_clock];
	uvrgb = MEM_ADDR1;

	sy_memcpy2(MEM_ADDR1, dst, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);
	sy_setuv(uvrgb, work->tex, N_DIV_H, N_DIV_V, index);
	sy_memcpy(dst, MEM_ADDR1, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);

	if(--work->value <= 0)
	{
		work->mode  = BRK_FLR_ModeFallStart;
		work->value = 0;
	}
}

/*******************************************************************************
 * 崩落
 */
static inline void BRK_FLR_ActFallStart(
	WORK *work)		/* ワーク */
{
	SY_PRINTF3("BRK_FLR_ActFallStart\n");

	work->mode  = BRK_FLR_ModeFall;
	work->value = 0;
}

static inline void BRK_FLR_ActFall(
	WORK *work)		/* ワーク */
{
	/* 崩落したままの状態になる */
}

/*******************************************************************************
 * 倒れる
 */
static inline void BRK_FLR_ActLaydownStart(
	WORK *work)		/* ワーク */
{
	SY_PRINTF3("BRK_FLR_ActLaydownStart\n");

	work->mode  = BRK_FLR_ModeLaydown;
	work->value = 0;
}

static inline void BRK_FLR_ActLaydown(
	WORK *work)		/* ワーク */
{
	/* 倒れたままの状態になる */
}

/*******************************************************************************
 * 再表示
 */
static inline void BRK_FLR_ActReapperStart(
	WORK *work)		/* ワーク */
{
	SY_PRINTF3("BRK_FLR_ActReapperStart\n");

	work->mode   = BRK_FLR_ModeReapper;
	work->value2 = work->value;
}

static inline void BRK_FLR_ActReapper(
	WORK *work)		/* ワーク */
{
	int            index;
	DG_PRIM2_UVRGB *dst, *uvrgb;

	/* 分割インデックスの決定 */
	if(work->value <= 1) index = 0;
	else                 index = (N_DIV - 2) - (int)(((work->value2 - work->value) / (float)(work->value2 - 1)) * (N_DIV - 2));

	/* テクスチャアニメーション */
	DG_SwitchBuffPrim2(work->prim);
	dst   = work->prim->uvrgb[work->prim->buffer_clock];
	uvrgb = MEM_ADDR1;

	sy_memcpy2(MEM_ADDR1, dst, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);
	sy_setuv(uvrgb, work->tex, N_DIV_H, N_DIV_V, index);
	sy_memcpy(dst, MEM_ADDR1, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);

	if(--work->value <= 0)
	{
		work->mode  = BRK_FLR_ModeReset;
		work->value = 0;
	}
}
