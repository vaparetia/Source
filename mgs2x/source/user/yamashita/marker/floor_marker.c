//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * marker - floor_marker.c
 * ＶＲ床マーカー  *NewVRFloorMarker
 * 2002/03/18 S.Yamashita
 * $Id: floor_marker.c,v 1.1.1.3 2002/11/19 11:51:50 Yoshizawa1 Exp $
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

#define __CHARA_NAME__ "VR Floor Marker"
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
#define N_PRIMS (1)
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

	DG_TEX    *tex1;		/* テクスチャ１ */
	DG_TEX    *tex2;		/* テクスチャ２ */
	DG_PRIM2  *prim1;		/* ポリゴンプリミティブ１ */
	DG_PRIM2  *prim2;		/* ポリゴンプリミティブ２ */
	FVECTOR   pos;			/* 位置 */

	u_short   alpha1;		/* アルファ値１ (デフォルト: ALPHA_1) */
	u_short   alpha2;		/* アルファ値２ (デフォルト: ALPHA_2) */
	float     speed;		/* スピード (小さいほうが早い。デフォルト: SPEED) */
	float     scale;		/* スケール */
	FVECTOR   *mov;			/* 位置（control が NULL の時にはこっちが有効） */
}
WORK;

/*******************************************************************************
 * actor
 */
/*******************************************************************************
 * メッセージ処理

mesg ＶＲ床マーカー $s:名前 非表示[0]
mesg ＶＲ床マーカー $s:名前 表示[1]
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
	float   a;
	FVECTOR pos2;

	/* メッセージ処理 */
	ReceiveMessage(work);

	if(work->control != NULL)
	{
		/* マーカー１の仮位置 */
		work->pos.vx += (work->control->mov.vx - work->pos.vx) / work->speed;
		work->pos.vy  =  work->control->mov.vy;
		work->pos.vz += (work->control->mov.vz - work->pos.vz) / work->speed;

		/* 中心からの距離よりアルファのパラメータ値を設定 */
		a  = GV_VecLen3F2(&work->control->mov, &work->pos);
		a  = a < 1000 ? a : 1000;
		a /= 1000;

		/* マーカーの位置 */
		work->pos.vy = work->control->levels[0] + ADJ_SHIFT;
#if 0
		pos2.vx      = work->control->mov.vx + (work->control->mov.vx - work->pos.vx);
		pos2.vy      = work->pos.vy;
		pos2.vz      = work->control->mov.vz + (work->control->mov.vz - work->pos.vz);
#else
		pos2.vx      = work->control->mov.vx;
		pos2.vy      = work->pos.vy;
		pos2.vz      = work->control->mov.vz;
#endif
	}
	else
	{
		/* マーカー１の仮位置 */
		work->pos.vx += (work->mov->vx - work->pos.vx) / work->speed;
		work->pos.vy  =  work->mov->vy;
		work->pos.vz += (work->mov->vz - work->pos.vz) / work->speed;

		/* 中心からの距離よりアルファのパラメータ値を設定 */
		a  = GV_VecLen3F2(work->mov, &work->pos);
		a  = a < 1000 ? a : 1000;
		a /= 1000;

		/* マーカーの位置 */
		work->pos.vy = work->mov->vy + ADJ_SHIFT;
#if 0
		pos2.vx      = work->mov->vx + (work->mov->vx - work->pos.vx);
		pos2.vy      = work->pos.vy;
		pos2.vz      = work->mov->vz + (work->mov->vz - work->pos.vz);
#else
		pos2.vx      = work->mov->vx;
		pos2.vy      = work->pos.vy;
		pos2.vz      = work->mov->vz;
#endif
	}

	/* プリミティブのアニメーション */
	{
		FVECTOR        *dst1, *dst2;
		DG_PRIM2_UVRGB *uvrgb1, *uvrgb2;

		DG_SwitchBuffPrim2(work->prim1);
		dst1   = work->prim1->pos[work->prim1->buffer_clock];
		uvrgb1 = work->prim1->uvrgb[work->prim1->buffer_clock];
		DG_SwitchBuffPrim2(work->prim2);
		dst2   = work->prim2->pos[work->prim2->buffer_clock];
		uvrgb2 = work->prim2->uvrgb[work->prim2->buffer_clock];

		if(work->control == GM_PlayerControl)
		{
			if((GM_PlayerStatus & HIDE_PSTATUS) || (work->control->flr_atrs[0] & HIDE_FLRFLAG))
			{
				uvrgb1[0].f = uvrgb1[1].f = uvrgb1[2].f = uvrgb1[3].f = 0x8fff;
				uvrgb2[0].f = uvrgb2[1].f = uvrgb2[2].f = uvrgb2[3].f = 0x8fff;
				return;
			}
		}
		uvrgb1[0].f = uvrgb1[1].f = uvrgb1[2].f = uvrgb1[3].f = 0x0fff;
		uvrgb2[0].f = uvrgb2[1].f = uvrgb2[2].f = uvrgb2[3].f = 0x0fff;

		sy_setpos(dst1, &work->pos, (POLYSIZE/2) * work->scale, (POLYSIZE/2) * work->scale, SY_DIR_UP);
		uvrgb1[0].a = uvrgb1[1].a = uvrgb1[2].a = uvrgb1[3].a = work->alpha1 - ((work->alpha1 - 8) * a);
		sy_setpos(dst2, &pos2, (POLYSIZE/2) * work->scale, (POLYSIZE/2) * work->scale, SY_DIR_UP);
		uvrgb2[0].a = uvrgb2[1].a = uvrgb2[2].a = uvrgb2[3].a = work->alpha2 - ((work->alpha2 - 6) * a);
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

chara ＶＲ床マーカー[NewVRFloorMarker_Scn] $s:名前 \
	// 以下オプション
	-t $s:テクスチャ１ \
	-u $s:テクスチャ２ \
	-a $b:テクスチャ１のアルファ値 \
	-b $b:テクスチャ２のアルファ値 \
	-s $w:スピード		// （小さいほうが早い。1～）
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	WORK    *work,		/* ワーク */
	int     name,		/* シナリオ名 */
	int     where,		/* マップＩＤ */
	CONTROL *control,	/* コントロール */
	FVECTOR *mov,		/* 位置 */
	float   scale)		/* スケール */
{
	int tex_code1, tex_code2;

	SY_PRINTF3("GetResources\n");

	work->name    = name;
	work->where   = where;
	work->control = control;
	work->alpha1  = ALPHA_1;
	work->alpha2  = ALPHA_2;
	work->speed   = SPEED;
	work->scale   = scale;
	work->mov     = mov;
	tex_code1     = GV_StrCode(F_TEXTURE_1);
	tex_code2     = GV_StrCode(F_TEXTURE_2);

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
		DG_TEX         *tex;
		FVECTOR        *pos;
		DG_PRIM2_UVRGB *uvrgb;
		FVECTOR        pos2;

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

		if(control != NULL)
		{
			work->pos.vx = control->mov.vx + START_DIST;
			work->pos.vy = control->levels[0];
			work->pos.vz = control->mov.vz + START_DIST;
		}
		else
		{
			work->pos.vx = mov->vx + START_DIST;
			work->pos.vy = mov->vy;
			work->pos.vz = mov->vz + START_DIST;
		}

		sy_setpos(pos, &work->pos, (POLYSIZE/2) * scale, (POLYSIZE/2) * scale, SY_DIR_UP);
		sy_setuv(uvrgb, tex, 1, 1, 0);
		sy_setrgba(uvrgb, 128, 128, 128, work->alpha1, 4);
		uvrgb[0].f = uvrgb[1].f = uvrgb[2].f = uvrgb[3].f = 0x0fff;
		uvrgb[0].q = uvrgb[1].q = uvrgb[2].q = uvrgb[3].q = 4096;

		sy_memcpy(work->prim1->pos[0]  , MEM_ADDR1, sizeof(FVECTOR)       , N_PRIMS * N_VERTS);
		sy_memcpy(work->prim1->uvrgb[0], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);
		sy_memcpy(work->prim1->pos[1]  , MEM_ADDR1, sizeof(FVECTOR)       , N_PRIMS * N_VERTS);
		sy_memcpy(work->prim1->uvrgb[1], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);

		/* プリミティブ２ */
		tex   = work->tex2;
		pos   = MEM_ADDR1;
		uvrgb = MEM_ADDR2;

		if(control != NULL)
		{
			pos2.vx = control->mov.vx + (control->mov.vx - work->pos.vx);
			pos2.vy = work->pos.vy;
			pos2.vz = control->mov.vz + (control->mov.vz - work->pos.vz);
		}
		else
		{
			pos2.vx = mov->vx + (mov->vx - work->pos.vx);
			pos2.vy = work->pos.vy;
			pos2.vz = mov->vz + (mov->vz - work->pos.vz);
		}

		sy_setpos(pos, &pos2, (POLYSIZE/2) * scale, (POLYSIZE/2) * scale, SY_DIR_UP);
		sy_setuv(uvrgb, tex, 1, 1, 0);
		sy_setrgba(uvrgb, 128, 128, 128, work->alpha2, 4);
		uvrgb[0].f = uvrgb[1].f = uvrgb[2].f = uvrgb[3].f = 0x0fff;
		uvrgb[0].q = uvrgb[1].q = uvrgb[2].q = uvrgb[3].q = 4096;

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
void *NewVRFloorMarker(
	CONTROL *control,	/* コントロール */
	float   scale)		/* スケール */
{
	WORK *work;

	SY_PRINTF3("NewVRFloorMarker\n");
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
		if(!GetResources(work, 0, control->map, control, NULL, scale))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}

void *NewVRFloorMarker2(
	FVECTOR *mov,	/* 位置 */
	float   scale)	/* スケール */
{
	WORK *work;

	SY_PRINTF3("NewVRFloorMarker2\n");
	OPERATOR();

	if(mov == NULL)
	{
		SY_PRINTF2("No mov!\n");
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
		if(!GetResources(work, 0, GM_CurrentMap, NULL, mov, scale))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}

void *NewVRFloorMarker_Scn(
	int name,	/* シナリオ名 */
	int where)	/* マップＩＤ */
{
	WORK *work;

	SY_PRINTF3("NewVRFloorMarker_Scn\n");
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
		if(!GetResources(work, name, where, GM_PlayerControl, NULL, 1.0f))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}
