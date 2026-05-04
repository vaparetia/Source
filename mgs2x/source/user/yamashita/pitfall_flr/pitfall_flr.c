//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * pitfall_flr - pitfall_flr.c
 * 落とし穴の床  *NewPitfallFloor
 * 2002/03/13 S.Yamashita
 * $Id: pitfall_flr.c,v 1.1.1.3 2002/11/19 11:51:53 Yoshizawa1 Exp $
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
 * actor
 */
/*******************************************************************************
 * シグナル処理
 *   BRK_FLR_ModeReset       : 初期状態
 *   BRK_FLR_ModeShakeStart  : 揺れ開始
 *   BRK_FLR_ModeReapperStart: 揺れ開始
 */
static int ReceiveSignal(	/* 処理結果 */
	void *pwork,	/* ワーク */
	int  signal,	/* シグナル */
	int  value)		/* 付加情報 */
{
	WORK *work = pwork;

	SY_PRINTF3("ReceiveSignal\n");

	SY_PRINTF2("signal: %d\n", signal);
	switch(signal)
	{
	case BRK_FLR_ModeReset     :
	case BRK_FLR_ModeShakeStart:
	case BRK_FLR_ModeSwingStart:
	case BRK_FLR_ModeFallStart :
	case BRK_FLR_ModeFall      :
		if(work->mode >= BRK_FLR_ModeSwingStart)
			break;

		work->mode  = signal;
		work->value = value;
		break;

	case BRK_FLR_ModeReapperStart:
		if(work->mode != BRK_FLR_ModeFall)
			break;

		work->mode  = signal;
		work->value = value;
		break;

	case BRK_FLR_SetSeMode:
		break;

	default:
		return GV_DefaultSignalFunc(pwork, signal, value);
	}

	return 0;
}

/*******************************************************************************
 * 毎フレーム処理
 */
static void Act(
	WORK *work)		/* ワーク */
{
	if(work->mode)
		PFF_Act(work);
}

/*******************************************************************************
 * 終了処理
 */
static void Die(
	WORK *work)		/* ワーク */
{
	SY_PRINTF3("Die\n");

	/* プリミティブの終了 */
	GM_FreePrim2(work->prim);
}

/*******************************************************************************
 * 初期化処理
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	WORK    *work,	/* ワーク */
	FVECTOR *pos,	/* 位置 */
	SVECTOR *rot)	/* 回転 */
{
	SY_PRINTF3("GetResources\n");

	work->where   = GM_CurrentMap;
	work->mode    = 0;
	work->value   = 0;
	work->value2  = 0;
	work->value3  = 0;
	work->rot     = *rot;
	work->pos     = *pos;

	/* テクスチャ */
	{
		if((work->tex = DG_GetTexture(GV_StrCode(TEXTURE_NAME))) == NULL)
		{
			SY_PRINTF2("Get Texture Failed.\n");
			return 0;
		}

		SY_PRINTF1("Get Texture Succeeded.\n");
	}

	/* プリミティブの初期化 */
	{
		DG_TEX         *tex;
		FVECTOR        *pos2;
		DG_PRIM2_UVRGB *uvrgb;

		if((work->prim = GM_MakePrim2(DG_PRIM2_CULLPOLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, N_PRIMS * N_VERTS)) == NULL)
		{
			SY_PRINTF2("Make Primitive Failed.\n");
			return 0;
		}
		DG_ConfigPrim2Tex(work->prim, work->tex);

		tex   = work->tex;
		pos2  = MEM_ADDR1;
		uvrgb = MEM_ADDR2;

		sy_setpos(pos2, pos, POLY_W/2, POLY_H/2, SY_DIR_UP);
		sy_setuv(uvrgb, tex, N_DIV_H, N_DIV_V, 0);
		sy_setrgba(uvrgb, DEF_RGB, DEF_RGB, DEF_RGB, DEF_ALPHA, 4);
		uvrgb[0].f = uvrgb[1].f = uvrgb[2].f = uvrgb[3].f = 0x0fff;
		uvrgb[0].q = uvrgb[1].q = uvrgb[2].q = uvrgb[3].q = 4096;

		sy_memcpy(work->prim->pos[0]  , MEM_ADDR1, sizeof(FVECTOR)       , N_PRIMS * N_VERTS);
		sy_memcpy(work->prim->uvrgb[0], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);
		sy_memcpy(work->prim->pos[1]  , MEM_ADDR1, sizeof(FVECTOR)       , N_PRIMS * N_VERTS);
		sy_memcpy(work->prim->uvrgb[1], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);

		SY_PRINTF1("Set Primitive Succeeded.\n");
	}

	return 1;
}

/*******************************************************************************
 * 起動処理
 */
void *NewPitfallFloor(
	FVECTOR *pos,	/* 位置 */
	SVECTOR *rot)	/* 回転 */
{
	WORK *work;

	SY_PRINTF3("NewPitfallFloor\n");
	OPERATOR();

	/* アクター生成 */
	work = (WORK *)GV_NewActorPrio(
		GV_ACTOR_USER,	/* プロセスの優先順位 */
		sizeof(WORK),	/* メモリ確保サイズ */
		0x10);			/* 実行プライオリティ */
	if(work != NULL)
	{
		/* アクター登録 */
		GV_SetActor(&work->actor, Act, Die);
		GV_ActorEX(&work->actor);

		/* シグナル処理関数登録 */
		GV_SetActorSignalFunc(work, ReceiveSignal);

		/* ワークの初期化 */
		if(!GetResources(work, pos, rot))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}
