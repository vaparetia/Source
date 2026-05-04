//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * marker - wall_marker.c
 * ＶＲ壁マーカー  *NewVRWallMarker
 * 2002/03/20 S.Yamashita
 * $Id: wall_marker.c,v 1.1.1.3 2002/11/19 11:51:51 Yoshizawa1 Exp $
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

#define __CHARA_NAME__ "VR Wall Marker"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
#include "../sy_util/sy_util.h"

#include "marker.h"

/*******************************************************************************
 * defines
 */

#define MEM_ADDR1 ((void*)(SCRPAD_ADDR + 0x0000))
#define MEM_ADDR2 ((void*)(SCRPAD_ADDR + 0x2000))

/* ポリゴン＆ラインの最大頂点数は６４まで！！ */
#define N_PRIMS (4)
#define N_VERTS (4)

/*******************************************************************************
 * macros
 */

#define FTOI12(_f) (DG_FTOI(((float)(_f) * 4096.0f)))
#define RND(n)     (((BP_PS2_rand() >> 16) * n) >> 15)

/*******************************************************************************
 * work
 */

/* ワーク */
typedef struct tagWORK
{
	GV_ACT_EX actor;		/* アクター */
	int       name;			/* シナリオ名 */
	int       where;		/* マップＩＤ */
	CONTROL   *control;		/* コントロール */

	float     dist;			/* 最大距離 (デフォルト: SEARCH_DIST) */
	DG_TEX    *tex1;		/* テクスチャ１ */
	DG_TEX    *tex2;		/* テクスチャ２ */
	DG_PRIM2  *prim1;		/* ポリゴンプリミティブ１ */
	DG_PRIM2  *prim2;		/* ポリゴンプリミティブ２ */
	FVECTOR   pos[4];		/* 位置 */
	u_short   count[4];		/* 表示カウンター */

	u_short   alpha1;		/* アルファ値１ (デフォルト: ALPHA_1) */
	u_short   alpha2;		/* アルファ値２ (デフォルト: ALPHA_2) */
	float     speed;		/* スピード (小さいほうが早い。デフォルト: SPEED) */
}
WORK;

/*******************************************************************************
 * actor
 */
/*******************************************************************************
 * メッセージ処理

mesg ＶＲ壁マーカー $s:名前 非表示[0]
mesg ＶＲ壁マーカー $s:名前 表示[1]
 */
static int ReceiveMessage(	/* 処理結果 */
	WORK *work)		/* ワーク */
{
	int    i;
	int    msg_count;
	int    ret = -1;
	GV_MSG *msg;

	msg_count = GV_ReceiveMessage(work->name, &msg);
	for(i = 0; i < msg_count; i++, msg++)
	{
		SY_PRINTF2("Message received.\n");

		switch(msg->message[0])
		{
		case 0:		/* 非表示 */
			DG_InvisiblePrim2(work->prim1);
			DG_InvisiblePrim2(work->prim2);
			ret = 0;
			break;

		case 1:		/* 表示 */
			DG_VisiblePrim2(work->prim1);
			DG_VisiblePrim2(work->prim2);
			ret = 0;
			break;

		default:
			ret = -1;
			break;
		}
	}

	return ret;
}

/******************************************************************************
 * 毎フレーム処理
 */
static void Act(
	WORK *work)		/* ワーク */
{
	int            i;
	FVECTOR        *dst1, *dst2;
	DG_PRIM2_UVRGB *uvrgb1, *uvrgb2;
	float          a;
	FVECTOR        pos;
	int            result;

	/* メッセージ処理 */
	ReceiveMessage(work);

	DG_SwitchBuffPrim2(work->prim1);
	dst1   = work->prim1->pos[work->prim1->buffer_clock];
	uvrgb1 = work->prim1->uvrgb[work->prim1->buffer_clock];
	DG_SwitchBuffPrim2(work->prim2);
	dst2   = work->prim2->pos[work->prim2->buffer_clock];
	uvrgb2 = work->prim2->uvrgb[work->prim2->buffer_clock];

	for(i = 0; i < N_PRIMS; i++)
	{
		/* オンラインチェック */
		if(i == SY_DIR_NORTH)
		{
			pos.vx = work->control->mov.vx;
			pos.vy = work->control->mov.vy + Y_SEARCH;
			pos.vz = work->control->mov.vz + work->dist;
			pos.vw = work->control->mov.vw;
		}
		else if(i == SY_DIR_SOUTH)
		{
			pos.vx = work->control->mov.vx;
			pos.vy = work->control->mov.vy + Y_SEARCH;
			pos.vz = work->control->mov.vz - work->dist;
			pos.vw = work->control->mov.vw;
		}
		else if(i == SY_DIR_EAST)
		{
			pos.vx = work->control->mov.vx - work->dist;
			pos.vy = work->control->mov.vy + Y_SEARCH;
			pos.vz = work->control->mov.vz;
			pos.vw = work->control->mov.vw;
		}
		else
		{
			pos.vx = work->control->mov.vx + work->dist;
			pos.vy = work->control->mov.vy + Y_SEARCH;
			pos.vz = work->control->mov.vz;
			pos.vw = work->control->mov.vw;
		}
		result = HZX_OnlineHazardCheck(work->control->hzx_id,
			&work->control->mov, &pos, HZX_CHK_SEGMENT, HZX_SEG_NO_PLAYER, 0);

		if(result == 1)
			HZX_GetOnlinePoint(&pos);

		if(i < SY_DIR_EAST)
		{
			/* マーカー１の仮位置 */
			work->pos[i].vx += (pos.vx - work->pos[i].vx) / work->speed;
			work->pos[i].vy  = pos.vy;
			if(result == 1)
			{
				if(i == SY_DIR_NORTH) work->pos[i].vz = pos.vz + ADJ_SHIFT;
				else                  work->pos[i].vz = pos.vz - ADJ_SHIFT;
			}
			else
			{
				pos.vz = work->pos[i].vz;
			}

			/* 中心からの距離よりアルファのパラメータ値を設定 */
			a  = GV_VecLen3F2(&pos, &work->pos[i]);
			a  = a < 1000 ? a : 1000;
			a /= 1000;

			/* マーカーの位置 */
			work->pos[i].vy = work->control->levels[0] + Y_POS;
#if 0
			pos.vx          = pos.vx + (pos.vx - work->pos[i].vx);
#endif
			pos.vy          = work->pos[i].vy;
		}
		else
		{
			/* マーカー１の仮位置 */
			if(result == 1)
			{
				if(i == SY_DIR_EAST) work->pos[i].vx = pos.vx - ADJ_SHIFT;
				else                 work->pos[i].vx = pos.vx + ADJ_SHIFT;
			}
			else
			{
				pos.vx = work->pos[i].vx;
			}
			work->pos[i].vy  = pos.vy;
			work->pos[i].vz += (pos.vz - work->pos[i].vz) / work->speed;

			/* 中心からの距離よりアルファのパラメータ値を設定 */
			a  = GV_VecLen3F2(&pos, &work->pos[i]);
			a  = a < 1000 ? a : 1000;
			a /= 1000;

			/* マーカーの位置 */
			work->pos[i].vy  = work->control->levels[0] + Y_POS;
			pos.vy           = work->pos[i].vy;
#if 0
			pos.vz           = pos.vz + (pos.vz - work->pos[i].vz);
#endif
		}

		/* 表示 */
		if(result == 1)
		{
			if(work->count[i] < ALPHA_FADE)
				work->count[i]++;
		}
		else
		{
			if(work->count[i] > 0)
				work->count[i]--;
		}

		if((work->control == GM_PlayerControl) && (GM_PlayerStatus & HIDE_PSTATUS))
		{
			uvrgb1[0].f = uvrgb1[1].f = uvrgb1[2].f = uvrgb1[3].f = 0x8fff;
			uvrgb2[0].f = uvrgb2[1].f = uvrgb2[2].f = uvrgb2[3].f = 0x8fff;
		}
		else
		{
			sy_setpos(dst1, &work->pos[i], POLY_WIDTH/2, POLY_HEIGHT/2, i);
			sy_setpos(dst2, &pos         , POLY_WIDTH/2, POLY_HEIGHT/2, i);
			if(work->count[i] != 0)
			{
				uvrgb1[0].f = uvrgb1[1].f = uvrgb1[2].f = uvrgb1[3].f = 0x0fff;
				uvrgb2[0].f = uvrgb2[1].f = uvrgb2[2].f = uvrgb2[3].f = 0x0fff;
			}
			else
			{
				uvrgb1[0].f = uvrgb1[1].f = uvrgb1[2].f = uvrgb1[3].f = 0x8fff;
				uvrgb2[0].f = uvrgb2[1].f = uvrgb2[2].f = uvrgb2[3].f = 0x8fff;
			}
			uvrgb1[0].a = uvrgb1[1].a = uvrgb1[2].a = uvrgb1[3].a = (work->alpha1 - ((work->alpha1 -  8) * a)) * (work->count[i] / (float)ALPHA_FADE);
			uvrgb2[0].a = uvrgb2[1].a = uvrgb2[2].a = uvrgb2[3].a = (work->alpha2 - ((work->alpha2 -  6) * a)) * (work->count[i] / (float)ALPHA_FADE);
		}

		dst1   += N_VERTS;
		uvrgb1 += N_VERTS;
		dst2   += N_VERTS;
		uvrgb2 += N_VERTS;
	}
}

/******************************************************************************
 * 終了処理
 */
static void Die(
	WORK *work)		/* ワーク */
{
	SY_PRINTF3("Die\n");

	GM_FreePrim2(work->prim1);
	GM_FreePrim2(work->prim2);
}

/******************************************************************************
 * 初期化処理

chara ＶＲ壁マーカー[NewVRWallMarker_Scn] $s:名前 \
	// 以下オプション
	-t $s:テクスチャ１ \
	-u $s:テクスチャ２ \
	-a $b:テクスチャ１のアルファ値 \
	-b $b:テクスチャ２のアルファ値 \
	-s $w:スピード \	// （小さいほうが早い。1～）
	-d $w:最大距離
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	WORK    *work,		/* ワーク */
	int     name,		/* シナリオ名 */
	int     where,		/* マップＩＤ */
	CONTROL *control)	/* コントロール */
{
	int tex_code1, tex_code2;

	SY_PRINTF3("GetResources\n");

	work->name     = name;
	work->where    = where;
	work->control  = control;
	work->alpha1   = ALPHA_1;
	work->alpha2   = ALPHA_2;
	work->speed    = SPEED;
	work->dist     = SEARCH_DIST;
	tex_code1      = GV_StrCode(W_TEXTURE_1);
	tex_code2      = GV_StrCode(W_TEXTURE_2);

	/* シナリオオプション */
	if(name != 0)
	{
		int value;

		/* alpha1 */
		if(GCL_GetOption('a') != NULL)
			work->alpha1 = GCL_GetNextInt();

		/* alpha2 */
		if(GCL_GetOption('b') != NULL)
			work->alpha2 = GCL_GetNextInt();

		/* speed */
		if(GCL_GetOption('s') != NULL)
		{
			if((value = GCL_GetNextInt()) != 0)
				work->speed = value;
		}

		/* dist */
		if(GCL_GetOption('d') != NULL)
			work->dist = GCL_GetNextInt();

		/* tex_code1 */
		if(GCL_GetOption('t') != NULL)
		{
			if((value = GCL_GetNextInt()) != 0)
				tex_code1 = value;
		}

		/* tex_code2 */
		if(GCL_GetOption('u') != NULL)
		{
			if((value = GCL_GetNextInt()) != 0)
				tex_code2 = value;
		}

		SY_PRINTF1("Get Scn Option Succeeded.\n");
	}

	/* テクスチャ */
	{
		if(    ((work->tex1 = DG_GetTexture(tex_code1)) == NULL)
			|| ((work->tex2 = DG_GetTexture(tex_code2)) == NULL))
		{
			SY_PRINTF2("Get Texture Failed.\n");
			return 0;
		}

		SY_PRINTF1("Get Texture Succeeded.\n");
	}

	/* プリミティブの初期化 */
	{
		int            i;
		DG_TEX         *tex;
		FVECTOR        *pos;
		DG_PRIM2_UVRGB *uvrgb;

		if(    ((work->prim1 = GM_MakePrim2(DG_PRIM2_CULLPOLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS)) == NULL)
			|| ((work->prim2 = GM_MakePrim2(DG_PRIM2_CULLPOLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS)) == NULL))
		{
			SY_PRINTF2("Make Primitive Failed.\n");
			return 0;
		}
		DG_ConfigPrim2Tex(work->prim1, work->tex1);
		DG_ConfigPrim2Tex(work->prim2, work->tex2);

		/* プリミティブ１ */
		tex   = work->tex1;
		pos   = MEM_ADDR1;
		uvrgb = MEM_ADDR2;

		for(i = 0; i < N_PRIMS; i++)
		{
			if(i == SY_DIR_NORTH)
			{
				work->pos[i].vx = control->mov.vx;
				work->pos[i].vy = control->levels[0] + Y_POS;
				work->pos[i].vz = control->mov.vz + work->dist;
			}
			else if(i == SY_DIR_SOUTH)
			{
				work->pos[i].vx = control->mov.vx;
				work->pos[i].vy = control->levels[0] + Y_POS;
				work->pos[i].vz = control->mov.vz - work->dist;
			}
			else if(i == SY_DIR_EAST)
			{
				work->pos[i].vx = control->mov.vx - work->dist;
				work->pos[i].vy = control->levels[0] + Y_POS;
				work->pos[i].vz = control->mov.vz;
			}
			else
			{
				work->pos[i].vx = control->mov.vx + work->dist;
				work->pos[i].vy = control->levels[0] + Y_POS;
				work->pos[i].vz = control->mov.vz;
			}

			work->count[i] = 0;
			sy_setpos(pos, &work->pos[i], POLY_WIDTH/2, POLY_HEIGHT/2, i);
			sy_setuv(uvrgb, tex, 1, 1, 0);
			sy_setrgba(uvrgb, 128, 128, 128, work->alpha1, 4);
			uvrgb[0].f = uvrgb[1].f = uvrgb[2].f = uvrgb[3].f = 0x8fff;
			uvrgb[0].q = uvrgb[1].q = uvrgb[2].q = uvrgb[3].q = 4096;

			pos   += N_VERTS;
			uvrgb += N_VERTS;
		}

		sy_memcpy(work->prim1->pos[0]  , MEM_ADDR1, sizeof(FVECTOR)       , N_PRIMS * N_VERTS);
		sy_memcpy(work->prim1->uvrgb[0], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);
		sy_memcpy(work->prim1->pos[1]  , MEM_ADDR1, sizeof(FVECTOR)       , N_PRIMS * N_VERTS);
		sy_memcpy(work->prim1->uvrgb[1], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);

		/* プリミティブ２ */
		tex   = work->tex2;
		pos   = MEM_ADDR1;
		uvrgb = MEM_ADDR2;

		for(i = 0; i < N_PRIMS; i++)
		{
			sy_setpos(pos, &work->pos[i], POLY_WIDTH/2, POLY_HEIGHT/2, i);
			sy_setuv(uvrgb, tex, 1, 1, 0);
			sy_setrgba(uvrgb, 128, 128, 128, work->alpha2, 4);
			uvrgb[0].f = uvrgb[1].f = uvrgb[2].f = uvrgb[3].f = 0x8fff;
			uvrgb[0].q = uvrgb[1].q = uvrgb[2].q = uvrgb[3].q = 4096;

			pos   += N_VERTS;
			uvrgb += N_VERTS;
		}

		sy_memcpy(work->prim2->pos[0]  , MEM_ADDR1, sizeof(FVECTOR)       , N_PRIMS * N_VERTS);
		sy_memcpy(work->prim2->uvrgb[0], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);
		sy_memcpy(work->prim2->pos[1]  , MEM_ADDR1, sizeof(FVECTOR)       , N_PRIMS * N_VERTS);
		sy_memcpy(work->prim2->uvrgb[1], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);

		SY_PRINTF1("Set Primitive Succeeded.\n");
	}

	return 1;
}

/******************************************************************************
 * 起動処理
 */
void *NewVRWallMarker(
	CONTROL *control)	/* コントロール */
{
	WORK *work;

	SY_PRINTF3("NewVRWallMarker\n");
	OPERATOR();

	if(control == NULL)
	{
		SY_PRINTF2("No control!\n");
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
		if(!GetResources(work, 0, control->map, control))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}

void *NewVRWallMarker_Scn(
	int name,	/* シナリオ名 */
	int where)	/* マップＩＤ */
{
	WORK *work;

	SY_PRINTF3("NewVRWallMarker_Scn\n");
	OPERATOR();

	if(GM_PlayerControl == NULL)
	{
		SY_PRINTF2("No control!\n");
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
		if(!GetResources(work, name, where, GM_PlayerControl))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}
