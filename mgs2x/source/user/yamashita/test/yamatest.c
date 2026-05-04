//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * yamatest.c
 * やまテスト  *NewYamaTest
 * 2002/02/19 S.Yamashita
 * $Id: yamatest.c,v 1.1.1.3 2002/11/19 11:51:54 Yoshizawa1 Exp $
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

#include "../pitfall_flr/pitfall_flr.h"

#define __CHARA_NAME__ "YamaTest"
#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
#define SY_PRINTF3_DEBUG
#include "../sy_util/sy_util.h"

/*******************************************************************************
 * defines
 */

#define MEM_ADDR1 ((void*)(SCRPAD_ADDR + 0x0000))
#define MEM_ADDR2 ((void*)(SCRPAD_ADDR + 0x2000))

/* ポリゴン＆ラインの最大頂点数は６４まで！！ */
#define N_PRIMS (21)
#define N_VERTS (16)

/*******************************************************************************
 * macros
 */

#define RND(n) (((BP_PS2_rand() >> 16) * n) >> 15)

/*******************************************************************************
 * work
 */

/*
 * ワーク
 */
typedef struct tagWORK
{
	GV_ACT_EX actor;	/* アクター */
	int       name;		/* シナリオ名 */
	int       where;	/* マップＩＤ */
    
	DG_PRIM2* prim;		/* ＤＧプリミティブ */
	char      pad1;		/* パディング */
	short     pad2;		/* パディング */
	short     pad3;		/* パディング */
	short     pad4;		/* パディング */
	short     pad5;		/* パディング */
	int       pad6;		/* パディング */
	FVECTOR   pos;		/* 箱の位置 */
}
WORK;

/*******************************************************************************
 * fuctions
 */

extern void PosBox(FVECTOR * ,float ,SVECTOR *);

/*******************************************************************************
 * actor
 */
/******************************************************************************
 * 毎フレーム処理
 */
static void Act(
	WORK *work)		/* ワーク */
{
	static int i = 0;
	FVECTOR ppos;

	/*
	 * ターミナル出力
	 */
	{
		if(i % 60 == 0)
		{
			SY_PRINTF1("YamaTestYamaTestYamaTestYamaTestYamaTest %d(charaID:%d)\n", i, work->name);

			/*
			 * アライメント実験
			 */
			SY_PRINTF1("size of WORK                    : %3d                  (%3.2f)\n", sizeof(WORK)                 ,                                                 (sizeof(WORK)                 ) / 16.0f);
			SY_PRINTF1("size of GV_ACT_EX               : %3d                  (%3.2f)\n", sizeof(GV_ACT_EX)            ,                                                 (sizeof(GV_ACT_EX)            ) / 16.0f);
			SY_PRINTF1("  GV_ACT_EX actor member address: %3d (%3d bytes long) (%3.2f)\n", (int)&work->actor - (int)work, (int)&work->name  - (int)&work->actor         , ((int)&work->actor - (int)work) / 16.0f);
			SY_PRINTF1("  int       name  member address: %3d (%3d bytes long) (%3.2f)\n", (int)&work->name  - (int)work, (int)&work->where - (int)&work->name          , ((int)&work->name  - (int)work) / 16.0f);
			SY_PRINTF1("  int       where member address: %3d (%3d bytes long) (%3.2f)\n", (int)&work->where - (int)work, (int)&work->prim  - (int)&work->where         , ((int)&work->where - (int)work) / 16.0f);
			SY_PRINTF1("  DG_PRIM2* prim  member address: %3d (%3d bytes long) (%3.2f)\n", (int)&work->prim  - (int)work, (int)&work->pad1  - (int)&work->prim          , ((int)&work->prim  - (int)work) / 16.0f);
			SY_PRINTF1("  char      pad1  member address: %3d (%3d bytes long) (%3.2f)\n", (int)&work->pad1  - (int)work, (int)&work->pad2  - (int)&work->pad1          , ((int)&work->pad1  - (int)work) / 16.0f);
			SY_PRINTF1("  short     pad2  member address: %3d (%3d bytes long) (%3.2f)\n", (int)&work->pad2  - (int)work, (int)&work->pad3  - (int)&work->pad2          , ((int)&work->pad2  - (int)work) / 16.0f);
			SY_PRINTF1("  short     pad3  member address: %3d (%3d bytes long) (%3.2f)\n", (int)&work->pad3  - (int)work, (int)&work->pad4  - (int)&work->pad3          , ((int)&work->pad3  - (int)work) / 16.0f);
			SY_PRINTF1("  short     pad4  member address: %3d (%3d bytes long) (%3.2f)\n", (int)&work->pad4  - (int)work, (int)&work->pad5  - (int)&work->pad4          , ((int)&work->pad4  - (int)work) / 16.0f);
			SY_PRINTF1("  short     pad5  member address: %3d (%3d bytes long) (%3.2f)\n", (int)&work->pad5  - (int)work, (int)&work->pad6  - (int)&work->pad5          , ((int)&work->pad5  - (int)work) / 16.0f);
			SY_PRINTF1("  int       pad6  member address: %3d (%3d bytes long) (%3.2f)\n", (int)&work->pad6  - (int)work, (int)&work->pos   - (int)&work->pad6          , ((int)&work->pad6  - (int)work) / 16.0f);
			SY_PRINTF1("  FVECTOR   pos   member address: %3d (%3d bytes long) (%3.2f)\n", (int)&work->pos   - (int)work, sizeof(WORK) - ((int)&work->pos   - (int)work), ((int)&work->pos   - (int)work) / 16.0f);
		}
		i++;
	}

	/*
	 * プレイヤー位置取得。箱出力
	 */
	{
		SVECTOR color;

		ppos    = GM_PlayerControl->mov;
		ppos.vy = GM_PlayerControl->levels[0];

		work->pos.vx += ((ppos.vx - work->pos.vx)) / 30;
		work->pos.vy += ((ppos.vy - work->pos.vy)) / 30;
		work->pos.vz += ((ppos.vz - work->pos.vz)) / 30;

		color.vx = 255;
		color.vy = 0;
		color.vz = 0;
		PosBox(&ppos, 400.0, &color);

		color.vx = 0;
		color.vy = 255;
		color.vz = 0;
		PosBox(&work->pos, 500.0, &color);
	}

	/*
	 * 文字列出力
	 */
	{
		MENU_Locate(50, 50, MENU_MODE_LEFT);
		MENU_Printf("%f, %f, %f", ppos.vx, ppos.vy, ppos.vz);
	}

	/*
	 * DG_PRIM2のアニメーション手順
	 */
	{
		FVECTOR *dst, *src, *pos;
		int     j;

		/* 頂点やＵＶをアニメーションさせる場合には */
		/* DG_SwitchBufferPrim2()を呼んだ上で、prim->buffer_clockメンバの */
		/* 差すバッファに対してデータ更新を行なう。 */
		DG_SwitchBuffPrim2(work->prim);

		dst = work->prim->pos[work->prim->buffer_clock];
		src = work->prim->pos[1 - work->prim->buffer_clock];
		pos = MEM_ADDR1;

		for(j = 0; j < N_PRIMS; j++)
		{
			sy_memcpy2(pos + 1, src, sizeof(FVECTOR), N_VERTS - 1);

			pos[0].vx = GM_PlayerBody->objs->objs[j].world.m[3][0];
			pos[0].vy = GM_PlayerBody->objs->objs[j].world.m[3][1];
			pos[0].vz = GM_PlayerBody->objs->objs[j].world.m[3][2];

			sy_memcpy(dst, MEM_ADDR1, sizeof(FVECTOR), N_VERTS);

			dst += N_VERTS;
			src += N_VERTS;
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

chara やまテスト[NewYamaTest_Scn] $s:名前
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	WORK *work,		/* ワーク */
	int  name,		/* シナリオ名 */
	int  where)		/* マップＩＤ */
{
	int            i, j;
	FVECTOR        *pos;
	DG_PRIM2_UVRGB *uvrgb ;

	SY_PRINTF3("GetResources\n");

	work->name   = name;
	work->where  = where;

	/*
	 * ラインプリミティブ
	 */
	if((work->prim = GM_MakePrim2(DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_ANTIALIASING, N_PRIMS, N_VERTS)) == NULL)
	{
		SY_PRINTF2("Make Primitive Failed.\n");
		return 0;
	}

	/* スクラッチパッド上で初期化したデータを実際のワークにコピーする */
	pos   = MEM_ADDR1;
	uvrgb = MEM_ADDR2;
	for(i = 0; i < N_PRIMS; i++)
	{
		for(j = 0; j < N_VERTS; j++)
		{
//			pos[j].vx = (float)_RND(3000);
//			pos[j].vy = (float)_RND(3000) + 5000.0f;
//			pos[j].vz = (float)_RND(3000) + j * 5000.0f;

			uvrgb[j].r = ((i & 1) != 0) * 128;
			uvrgb[j].g = ((i & 2) != 0) * 128;
			uvrgb[j].b = ((i & 4) != 0) * 128;
			uvrgb[j].a = 128;					/* アンチエイリアスを使用するときには１２８にする */
		}
		pos   += N_VERTS;
		uvrgb += N_VERTS;
	}
	sy_memcpy(work->prim->pos[0]  , MEM_ADDR1, sizeof(FVECTOR)       , N_VERTS * N_PRIMS);
	sy_memcpy(work->prim->uvrgb[0], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_VERTS * N_PRIMS);
	sy_memcpy(work->prim->pos[1]  , MEM_ADDR1, sizeof(FVECTOR)       , N_VERTS * N_PRIMS);
	sy_memcpy(work->prim->uvrgb[1], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), N_VERTS * N_PRIMS);

	SY_PRINTF1("Set Primitive Succeeded.\n");

	/* 落とし穴の床 */
	{
//		FVECTOR pos2;
//		SVECTOR rot;
//
//		pos2.vx = -1000;
//		pos2.vy = 0;
//		pos2.vz = 5000;
//
//		rot.vx = 0;
//		rot.vy = 0;
//		rot.vz = 0;
//
//		NewPitfallFloor(&pos2, &rot);
	}

//	SY_PRINTF1("Set Pitfall Floor Succeeded.\n");

	return 1;
}
/*
	交互にラインを描画するには

	(libdg.h)
	/ ライン・ポリゴン用頂点要素 /
	typedef struct _dg_prim2_uvrgb {
		u_short		r, g, b, a ;	/ 頂点カラー /
		u_short		u, v, q, f ;	/ ＵＶＱ,フラグ（描画キック：0x0fff、頂点キックのみ：0x8fff） /
		/ ＜補足＞DG_PRIM2_CULLPOLYの時のフラグ設定
		   両面表示：0x0fff 右回り表示：0x0020 左回り表示：0x0000 非表示：0x8fff /
	} DG_PRIM2_UVRGB ;

	q は通常 4096
	f は最上位ビットでキックするかが決まる（0x0000: 描画する、0x8000: 描画しない）

	キックさせると頂点を送っても描画がされない。
*/

/******************************************************************************
 * 起動処理
 */
void *NewYamaTest(
	int where)	/* マップＩＤ */
{
	WORK *work;

	SY_PRINTF3("NewYamaTest\n");
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
		if(!GetResources(work, 0, where))
		{
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}

void *NewYamaTest_Scn(
	int name,	/* シナリオ名 */
	int where)	/* マップＩＤ */
{
	WORK *work;

	SY_PRINTF3("NewYamaTest_Scn\n");
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
		if(!GetResources(work, name, where))
		{
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}
