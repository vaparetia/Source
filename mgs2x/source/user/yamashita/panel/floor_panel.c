//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * panel - floor_panel.c
 * ＶＲ床パネル  *NewVRFloorPanel
 * 2002/04/08 S.Yamashita
 * $Id: floor_panel.c,v 1.1.1.3 2002/11/19 11:51:52 Yoshizawa1 Exp $
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

#define __CHARA_NAME__ "VR Floor Panel"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
#include "../sy_util/sy_util.h"

#include "panel.h"

/*******************************************************************************
 * defines
 */

#define MEM_ADDR1 ((void*)(SCRPAD_ADDR + 0x0000))
#define MEM_ADDR2 ((void*)(SCRPAD_ADDR + 0x2000))

/* ポリゴン＆ラインの最大頂点数は６４まで！！ */
#define N_PRIMS (1)
#define N_VERTS (4)

/*******************************************************************************
 * work
 */

/* ワーク */
typedef struct tagWORK
{
	GV_ACT_EX actor;	/* アクター */
	int       name;		/* シナリオ名 */
	int       where;	/* マップＩＤ */
	DG_PRIM2  *prim;	/* プリミティブ */
	FVECTOR   pos;		/* 位置 */
	u_short   alpha;	/* アルファ値 */
}
WORK;

/*******************************************************************************
 * actor
 */
/******************************************************************************
 * 毎フレーム処理
 */
static void Act(
	WORK *work)		/* ワーク */
{
	DG_PRIM2_UVRGB *dst;

	DG_SwitchBuffPrim2(work->prim);
	dst = work->prim->uvrgb[work->prim->buffer_clock];

	if(    (((work->pos.vy - MARGIN      ) <= GM_PlayerControl->levels[0]) && (GM_PlayerControl->levels[0] <= (work->pos.vy + MARGIN      )))
		&& (((work->pos.vx - POLYSIZE / 2) <= GM_PlayerControl->mov.vx   ) && (GM_PlayerControl->mov.vx    <= (work->pos.vx + POLYSIZE / 2)))
		&& (((work->pos.vz - POLYSIZE / 2) <= GM_PlayerControl->mov.vz   ) && (GM_PlayerControl->mov.vz    <= (work->pos.vz + POLYSIZE / 2)))
		&& !(GM_PlayerStatus & HIDE_PSTATUS))
	{
		/* パネル内 */
		if(work->alpha == 0)
		{
			dst[3].f = dst[2].f = dst[1].f = dst[0].f = 0x0fff;
			dst[3].a = dst[2].a = dst[1].a = dst[0].a = work->alpha = ALPHA_FADEIN;
		}
		else if(work->alpha > ALPHA - ALPHA_FADEIN)
		{
			dst[3].f = dst[2].f = dst[1].f = dst[0].f = 0x0fff;
			dst[3].a = dst[2].a = dst[1].a = dst[0].a = work->alpha = ALPHA;
		}
		else
		{
			dst[3].f = dst[2].f = dst[1].f = dst[0].f = 0x0fff;
			dst[3].a = dst[2].a = dst[1].a = dst[0].a = work->alpha += ALPHA_FADEIN;
		}
	}
	else
	{
		/* パネル外 */
		if(work->alpha <= ALPHA_FADEOUT)
		{
			dst[3].f = dst[2].f = dst[1].f = dst[0].f = 0x8fff;
			dst[3].a = dst[2].a = dst[1].a = dst[0].a = work->alpha = 0;
		}
		else
		{
			dst[3].a = dst[2].a = dst[1].a = dst[0].a = work->alpha -= ALPHA_FADEOUT;
		}
	}
}

/******************************************************************************
 * 終了処理
 */
static void Die(
	WORK *work)		/* ワーク */
{
	SY_PRINTF3("Die\n");

	GM_FreePrim2(work->prim);
}

/******************************************************************************
 * 初期化処理

chara ＶＲ床パネル[NewVRFloorPanel_Scn] $s:名前 \
	-pos $v:位置 \
	// 以下オプション
	-tex $s:テクスチャ
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	WORK    *work,	/* ワーク */
	int     name,	/* シナリオ名 */
	int     where,	/* マップＩＤ */
	FVECTOR *pos)	/* 位置 */
{
	int    tex_code;
	DG_TEX *tex;

	SY_PRINTF3("GetResources\n");

	work->name  = name;
	work->where = where;
	tex_code    = GV_StrCode(F_TEXTURE);

	/* シナリオオプション */
	if(name == 0)
	{
		work->pos = *pos;
	}
	else
	{
		int value;

		/* pos */
		if(GCL_GetOption('p') == NULL)
		{
			SY_PRINTF2("Get Pos Failed.\n");
			return 0;
		}
		work->pos.vx = GCL_GetNextInt();
		work->pos.vy = GCL_GetNextInt();
		work->pos.vz = GCL_GetNextInt();
		work->pos.vw = 1.0;

		/* tex */
		if(GCL_GetOption('t') != NULL)
		{
			if((value = GCL_GetNextInt()) != 0)
				tex_code = value;
		}

		SY_PRINTF1("Get Scn Option Succeeded.\n");
	}

	/* テクスチャ */
	{
		if((tex = DG_GetTexture(tex_code)) == NULL)
		{
			SY_PRINTF2("Get Texture Failed.\n");
			return 0;
		}

		SY_PRINTF1("Get Texture Succeeded.\n");
	}

	/* プリミティブの初期化 */
	{
		FVECTOR        *pos2;
		DG_PRIM2_UVRGB *uvrgb;

		if((work->prim = GM_MakePrim2(DG_PRIM2_CULLPOLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS)) == NULL)
		{
			SY_PRINTF2("Make Primitive Failed.\n");
			return 0;
		}
		DG_ConfigPrim2Tex(work->prim, tex);
		DG_SetPrim2Alpha(work->prim, SCE_GS_SET_ALPHA(0, 2, 0, 1, 0x00));

		/* プリミティブ */
		pos2  = MEM_ADDR1;
		uvrgb = MEM_ADDR2;

		work->pos.vy += PUSH_UP;
		sy_setpos(pos2, &work->pos, POLYSIZE/2, POLYSIZE/2, SY_DIR_UP);
		sy_setuv(uvrgb, tex, 1, 1, 0);
		work->alpha = 0;
		sy_setrgba(uvrgb, 128, 128, 128, 0, 4);
		uvrgb[0].f = uvrgb[1].f = uvrgb[2].f = uvrgb[3].f = 0x8fff;
		uvrgb[0].q = uvrgb[1].q = uvrgb[2].q = uvrgb[3].q = 4096;
		work->pos.vy -= PUSH_UP;

		sy_memcpy(work->prim->pos[0]  , MEM_ADDR1, sizeof(FVECTOR)       , N_PRIMS * N_VERTS);
		sy_memcpy(work->prim->uvrgb[0], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);
		sy_memcpy(work->prim->pos[1]  , MEM_ADDR1, sizeof(FVECTOR)       , N_PRIMS * N_VERTS);
		sy_memcpy(work->prim->uvrgb[1], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);

		SY_PRINTF1("Set Primitive Succeeded.\n");
	}

	return 1;
}

/******************************************************************************
 * 起動処理
 */
void *NewVRFloorPanel(
	FVECTOR *pos)	/* 位置 */
{
	WORK *work;

	SY_PRINTF3("NewVRFloorPanel\n");
	OPERATOR();

	if(pos == NULL)
	{
		SY_PRINTF2("No position!\n");
		return NULL;
	}

	/* アクター生成 */
	work = (WORK *)GV_CreateActor(
		GV_ACTOR_USER,				/* プロセスの優先順位 */
		GV_CLASS_CHARA,				/* プロセスクラス */
		sizeof(WORK),				/* メモリ確保サイズ */
		0);							/* 実行プライオリティ */
	if(work != NULL)
	{
		/* アクター登録 */
		GV_SetActor(&work->actor, Act, Die);
		GV_ActorEX(&work->actor);

		/* ワークの初期化 */
		if(!GetResources(work, 0, GM_CurrentMap, pos))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}

void *NewVRFloorPanel_Scn(
	int name,	/* シナリオ名 */
	int where)	/* マップＩＤ */
{
	WORK *work;

	SY_PRINTF3("NewVRFloorPanel_Scn\n");
	OPERATOR();

	/* アクター生成 */
	work = (WORK *)GV_CreateActor(
		GV_ACTOR_USER,				/* プロセスの優先順位 */
		GV_CLASS_CHARA,				/* プロセスクラス */
		sizeof(WORK),				/* メモリ確保サイズ */
		0);							/* 実行プライオリティ */
	if(work != NULL)
	{
		/* アクター登録 */
		GV_SetActor(&work->actor, Act, Die);
		GV_ActorEX(&work->actor);

		/* ワークの初期化 */
		if(!GetResources(work, name, where, NULL))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}
