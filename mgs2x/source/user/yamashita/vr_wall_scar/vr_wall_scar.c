//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * vr_wall_scar - vr_wall_scar.c
 * ＶＲ壁・床弾痕  *NewVRWallScarBase
 * 2002/03/28 S.Yamashita
 * $Id: vr_wall_scar.c,v 1.1.1.3 2002/11/19 11:51:56 Yoshizawa1 Exp $
 */

/*******************************************************************************
 * include
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "../../morita/include/util.h"

#define __CHARA_NAME__ "VR Wall Scar Base"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
#include "../sy_util/sy_util.h"

#include "vr_wall_scar.h"

/*******************************************************************************
 * defines
 */

#define MEM_ADDR1 ((void*)(SCRPAD_ADDR + 0x0000))
#define MEM_ADDR2 ((void*)(SCRPAD_ADDR + 0x2000))

enum
{
	X = 0,
	Y,
	Z,
	XY = Z,
	W,
	XYZ = W,
	XYZW
};

/*******************************************************************************
 * macros
 */

#define FTOI12(_f) (DG_FTOI(((float)(_f)*4096.0f)))

/*******************************************************************************
 * work
 */

typedef struct tagUNIT UNIT;
typedef struct tagSCAR SCAR;
typedef struct tagWORK
{
	GV_ACT actor;	/* アクター */
	int    name;	/* 名前 */
	int    flag;	/* フラグ */
	short  n_unit;	/* 最大弾痕数 */
	short  n_scar;	/* 弾痕種類数 */

	struct tagSCAR
	{
		DG_PRIM2 *prim1;	/* プリミティブ１ */
		DG_PRIM2 *prim2;	/* プリミティブ２ */
		short    front;		/* リングバッファの先頭 */
		short    n_pos;		/* リングバッファの末尾 */
		short    flag;		/* フラグ */
//		float    u0;		/* Ｕの中心 */
//		float    v0;		/* Ｖの中心 */
//		float    r;			/* 半径 */
		DG_TEX   *tex1;		/* テクスチャ */

		struct tagUNIT
		{
			int     count;				/* カウンター */
			int     flag;				/* フラグ */
			FVECTOR pos[4];				/* 位置 */
		}
		*unit;				/* 弾痕配列 */
	}
	*scar;			/* 弾痕種類配列 */
}
WORK;

/*******************************************************************************
 * local
 */

static WORK *wk;

static FVECTOR for_flr[4] = {
	{-WLLSCR_SIZE, 0.0f,  WLLSCR_SIZE, 1.0f},
	{ WLLSCR_SIZE, 0.0f,  WLLSCR_SIZE, 1.0f},
	{-WLLSCR_SIZE, 0.0f, -WLLSCR_SIZE, 1.0f},
	{ WLLSCR_SIZE, 0.0f, -WLLSCR_SIZE, 1.0f},
};
static FVECTOR inv_flr[4] = {
	{ WLLSCR_SIZE, 0.0f,  WLLSCR_SIZE, 1.0f},
	{-WLLSCR_SIZE, 0.0f,  WLLSCR_SIZE, 1.0f},
	{ WLLSCR_SIZE, 0.0f, -WLLSCR_SIZE, 1.0f},
	{-WLLSCR_SIZE, 0.0f, -WLLSCR_SIZE, 1.0f},
};
static FVECTOR for_seg[4] = {
	{-WLLSCR_SIZE,  WLLSCR_SIZE, 0.0f, 1.0f},
	{ WLLSCR_SIZE,  WLLSCR_SIZE, 0.0f, 1.0f},
	{-WLLSCR_SIZE, -WLLSCR_SIZE, 0.0f, 1.0f},
	{ WLLSCR_SIZE, -WLLSCR_SIZE, 0.0f, 1.0f},
};
static FVECTOR inv_seg[4] = {
	{ WLLSCR_SIZE,  WLLSCR_SIZE, 0.0f, 1.0f},
	{-WLLSCR_SIZE,  WLLSCR_SIZE, 0.0f, 1.0f},
	{ WLLSCR_SIZE, -WLLSCR_SIZE, 0.0f, 1.0f},
	{-WLLSCR_SIZE, -WLLSCR_SIZE, 0.0f, 1.0f},
};

/*******************************************************************************
 * static
 */
/******************************************************************************
 * ユニットの位置の設定
 */
static int CheckUnitNear(
	UNIT *unit1,
	UNIT *unit2)
{
	float x[2], y[2], z[2];

	x[0] = (unit1->pos[0].vx + unit1->pos[3].vx) / 2;
	y[0] = (unit1->pos[0].vy + unit1->pos[3].vy) / 2;
	z[0] = (unit1->pos[0].vz + unit1->pos[3].vz) / 2;
	x[1] = (unit2->pos[0].vx + unit2->pos[3].vx) / 2;
	y[1] = (unit2->pos[0].vy + unit2->pos[3].vy) / 2;
	z[1] = (unit2->pos[0].vz + unit2->pos[3].vz) / 2;

	x[0] -= x[1];
	y[0] -= y[1];
	z[0] -= z[1];

	if(pow(x[0] * x[0] + y[0] * y[0] + z[0] * z[0], 0.5) > WLLSCR_NEAR)
		return 0;
	else
		return 1;
}

/******************************************************************************
 * ユニットの位置の設定
 */
static void SetUnitPos(
	UNIT    *unit,
	FVECTOR *pos,
	float   scale)
{
	int   i;
	float x0, y0, z0;

	x0 = (unit->pos[0].vx + unit->pos[3].vx) / 2;
	y0 = (unit->pos[0].vy + unit->pos[3].vy) / 2;
	z0 = (unit->pos[0].vz + unit->pos[3].vz) / 2;

	for(i = 0; i < 4; i++)
	{
		pos[i].vx = x0 + (unit->pos[i].vx - x0) * scale;
		pos[i].vy = y0 + (unit->pos[i].vy - y0) * scale;
		pos[i].vz = z0 + (unit->pos[i].vz - z0) * scale;
		pos[i].vw = 1.0f;
	}
}

/******************************************************************************
 * ユニットのＵＶ値の設定
 */
static void SetUnitUV(
	SCAR           *scar,
	DG_PRIM2_UVRGB *uvrgb,
	float          angle)
{
	float c, s;

	c = cos(angle);
	s = sin(angle);

	(uvrgb    )->u = FTOI12((0.5 - s/2) * scar->tex1->u_scale + scar->tex1->u_offset);
	(uvrgb    )->v = FTOI12((0.5 - c/2) * scar->tex1->v_scale + scar->tex1->v_offset);
	(uvrgb + 1)->u = FTOI12((0.5 + c/2) * scar->tex1->u_scale + scar->tex1->u_offset);
	(uvrgb + 1)->v = FTOI12((0.5 - s/2) * scar->tex1->v_scale + scar->tex1->v_offset);
	(uvrgb + 2)->u = FTOI12((0.5 - c/2) * scar->tex1->u_scale + scar->tex1->u_offset);
	(uvrgb + 2)->v = FTOI12((0.5 + s/2) * scar->tex1->v_scale + scar->tex1->v_offset);
	(uvrgb + 3)->u = FTOI12((0.5 + s/2) * scar->tex1->u_scale + scar->tex1->u_offset);
	(uvrgb + 3)->v = FTOI12((0.5 + c/2) * scar->tex1->v_scale + scar->tex1->v_offset);

//	c = scar->r * cos(angle);
//	s = scar->r * sin(angle);

//	(uvrgb    )->u = FTOI12(scar->u0 - s);
//	(uvrgb    )->v = FTOI12(scar->v0 - c);
//	(uvrgb + 1)->u = FTOI12(scar->u0 + c);
//	(uvrgb + 1)->v = FTOI12(scar->v0 - s);
//	(uvrgb + 2)->u = FTOI12(scar->u0 - c);
//	(uvrgb + 2)->v = FTOI12(scar->v0 + s);
//	(uvrgb + 3)->u = FTOI12(scar->u0 + s);
//	(uvrgb + 3)->v = FTOI12(scar->v0 + c);
}

/******************************************************************************
 * 床座標値の取得
 */
static void CalcFloorMatrix(
	HZX_FLR *flr,	/* 床情報 */
	FMATRIX *dir,	/* 方向 */
	FVECTOR *v)		/* 位置情報を受け取る配列（４要素） */
{
	FVECTOR nrm;
	FMATRIX mx;
	float   d;
	float   dx, dz;

	nrm.vw = 0.0f;
	mx = DG_UnitMatrix;
	mx.m[X][Y] = nrm.vx = flr->p1.h / 32000.0F;
	mx.m[Y][Y] = nrm.vy = flr->p3.h / 32000.0F;
	mx.m[Z][Y] = nrm.vz = flr->p2.h / 32000.0F;
	if((dx = sceVu0Sqrt(nrm.vx * nrm.vx + nrm.vy * nrm.vy)) > 0.001f)
		mx.m[Y][X] = nrm.vx / dx, mx.m[X][X] = -nrm.vy / dx;
	if((dz = sceVu0Sqrt(nrm.vz * nrm.vz + nrm.vy * nrm.vy)) > 0.001f)
		mx.m[Y][Z] = nrm.vz / dz, mx.m[Z][Z] = -nrm.vy / dz;

	d = _sceVu0InnerProduct((FVECTOR*)dir->m[Y], &nrm);
	_sceVu0ScaleVector(&nrm, &nrm, (d < 0 ? -PUSH_UP : PUSH_UP));
	_sceVu0AddVector((FVECTOR*)mx.m[W], (FVECTOR*)dir->m[W], &nrm);

	DG_SetPos(&mx);
	DG_PutVector((d < 0 ? for_flr : inv_flr), v, 4);
}

/******************************************************************************
 * 壁座標値の取得
 */
static void CalcSegMatrix(
	HZX_SEG *seg,	/* 壁情報 */
	FMATRIX *dir,	/* 方向 */
	FVECTOR *v)		/* 位置情報を受け取る配列（４要素） */
{
	FVECTOR nrm = {seg->p1.z - seg->p2.z, 0.0f, -seg->p1.x + seg->p2.x, 0.0f };
	FMATRIX mx;
	float   d;

	_sceVu0Normalize(&nrm, &nrm);
	mx = DG_UnitMatrix;
	mx.m[X][X] =  nrm.vz;
	mx.m[X][Z] = -nrm.vx;
	mx.m[Z][X] =  nrm.vx;
	mx.m[Z][Z] =  nrm.vz;

	d = dir->m[Y][Z] * nrm.vz + dir->m[Y][X] * nrm.vx;
	_sceVu0ScaleVector(&nrm, &nrm, (d < 0 ? -PUSH_UP : PUSH_UP));
	_sceVu0AddVector((FVECTOR*)mx.m[W], (FVECTOR*)dir->m[W], &nrm);

	DG_SetPos(&mx);
	DG_PutVector((d > 0 ? for_seg : inv_seg), v, 4);
}

/*******************************************************************************
 * public
 */
/******************************************************************************
 * 弾痕登録
 */
void NewVRWallScar(
	FMATRIX *dir,	/* 方向 */
	HZX_SEG *seg,	/* 壁情報 */
	HZX_FLR *flr)	/* 床情報 */
{
	SCAR           *s;
	UNIT           *u;
	FMATRIX        lights[2];
	float          lt;
	FVECTOR        *pos0, *pos1;
	DG_PRIM2_UVRGB *rgb0, *rgb1;

	SY_PRINTF3("NewVRWallScar\n");

	if((!seg && !flr) || !wk->flag)
	{
		SY_PRINTF2("Set new scar failed.\n");
		return;
	}

    if(UTL_EFT_CheckBound((FVECTOR *)dir->m[3]) >= 0)
    	return;

	/* 先頭の弾痕種類に登録 */
	s = &wk->scar[0];

	/* ユニットの設定 */
	u        = s->unit + s->n_pos;
	u->count = 0;
	u->flag &= ~UNIT_PHASEMASK;
	u->flag &= ~UNIT_NOPHASE2;
	u->flag |= UNIT_PHASE1;

	/* 座標値の取得 */
	if(seg) CalcSegMatrix  (seg, dir, u->pos);
	else    CalcFloorMatrix(flr, dir, u->pos);

	/* 距離チェック */
	if(!(s->flag & SCAR_NODATA))
	{
		UNIT *u2;
		if(s->n_pos == 0) u2 = s->unit + (wk->n_unit - 1);
		else              u2 = s->unit + (s->n_pos   - 1);

		if(CheckUnitNear(u, u2) == 1)
			u->flag |= UNIT_NOPHASE2;
	}


	/* 位置からライトの取得 */
	DG_GetLightMatrixFix((FVECTOR *)dir->m[3], lights);
	lt = MAX(lights[1].m[0][2], MAX(lights[1].m[0][0], lights[1].m[0][1]));

	/* プリミティブ１ */
	pos0 = ((FVECTOR *)       s->prim1->pos[0]  ) + (s->n_pos * 4);
	pos1 = ((FVECTOR *)       s->prim1->pos[1]  ) + (s->n_pos * 4);
	rgb0 = ((DG_PRIM2_UVRGB *)s->prim1->uvrgb[0]) + (s->n_pos * 4);
	rgb1 = ((DG_PRIM2_UVRGB *)s->prim1->uvrgb[1]) + (s->n_pos * 4);

	_sceVu0CopyVector(&pos0[0], &DG_ZeroVector);
	_sceVu0CopyVector(&pos0[1], &DG_ZeroVector);
	_sceVu0CopyVector(&pos0[2], &DG_ZeroVector);
	_sceVu0CopyVector(&pos0[3], &DG_ZeroVector);
	_sceVu0CopyVector(&pos1[0], &DG_ZeroVector);
	_sceVu0CopyVector(&pos1[1], &DG_ZeroVector);
	_sceVu0CopyVector(&pos1[2], &DG_ZeroVector);
	_sceVu0CopyVector(&pos1[3], &DG_ZeroVector);

	(rgb0 + 3)->r = (rgb0 + 2)->r = (rgb0 + 1)->r = rgb0->r =
	(rgb0 + 3)->g = (rgb0 + 2)->g = (rgb0 + 1)->g = rgb0->g =
	(rgb0 + 3)->b = (rgb0 + 2)->b = (rgb0 + 1)->b = rgb0->b =
	(rgb1 + 3)->r = (rgb1 + 2)->r = (rgb1 + 1)->r = rgb1->r =
	(rgb1 + 3)->g = (rgb1 + 2)->g = (rgb1 + 1)->g = rgb1->g =
	(rgb1 + 3)->b = (rgb1 + 2)->b = (rgb1 + 1)->b = rgb1->b = (int)lt;
	(rgb0 + 3)->a = (rgb0 + 2)->a = (rgb0 + 1)->a = rgb0->a =
	(rgb1 + 3)->a = (rgb1 + 2)->a = (rgb1 + 1)->a = rgb1->a = WLLSCR_ALPHA_1;

	/* プリミティブ２ */
	rgb0 = ((DG_PRIM2_UVRGB *)s->prim2->uvrgb[0]) + (s->n_pos * 4);
	rgb1 = ((DG_PRIM2_UVRGB *)s->prim2->uvrgb[1]) + (s->n_pos * 4);

	(rgb0 + 3)->r = (rgb0 + 2)->r = (rgb0 + 1)->r = rgb0->r =
	(rgb0 + 3)->g = (rgb0 + 2)->g = (rgb0 + 1)->g = rgb0->g =
	(rgb0 + 3)->b = (rgb0 + 2)->b = (rgb0 + 1)->b = rgb0->b =
	(rgb1 + 3)->r = (rgb1 + 2)->r = (rgb1 + 1)->r = rgb1->r =
	(rgb1 + 3)->g = (rgb1 + 2)->g = (rgb1 + 1)->g = rgb1->g =
	(rgb1 + 3)->b = (rgb1 + 2)->b = (rgb1 + 1)->b = rgb1->b = (int)lt;
	(rgb0 + 3)->a = (rgb0 + 2)->a = (rgb0 + 1)->a = rgb0->a =
	(rgb1 + 3)->a = (rgb1 + 2)->a = (rgb1 + 1)->a = rgb1->a = 0;

	/* バッファポインタの更新 */
	if((s->n_pos == s->front) && !(s->flag & SCAR_NODATA))
	{
		s->front = s->n_pos = (s->n_pos + 1) % wk->n_unit;
	}
	else
	{
		s->flag &= ~SCAR_NODATA;
		s->n_pos = (s->n_pos + 1) % wk->n_unit;
	}
}

/*******************************************************************************
 * actor
 */
/******************************************************************************
 * エフェクトバウンドコールバック
 */
static void BoundCallBack(
	WORK *work)		/* ワーク */
{
	int            i, j;
	SCAR           *s;
	DG_PRIM2_UVRGB *uvs0, *uvs1, *uvs2, *uvs3;
	FVECTOR        pos;

	SY_PRINTF2("BoundCallBack\n");

	for(i = work->n_scar; --i >= 0; )
	{
		s    = &work->scar[i];
		uvs0 = s->prim1->uvrgb[0];
		uvs1 = s->prim1->uvrgb[1];
		uvs2 = s->prim2->uvrgb[0];
		uvs3 = s->prim2->uvrgb[1];

		for(j = 0; j < work->n_unit; j++)
		{
			pos.vx = (s->unit[j].pos[0].vx + s->unit[j].pos[1].vx + s->unit[j].pos[2].vx + s->unit[j].pos[3].vx) / 4;
			pos.vy = (s->unit[j].pos[0].vy + s->unit[j].pos[1].vy + s->unit[j].pos[2].vy + s->unit[j].pos[3].vy) / 4;
			pos.vz = (s->unit[j].pos[0].vz + s->unit[j].pos[1].vz + s->unit[j].pos[2].vz + s->unit[j].pos[3].vz) / 4;
			pos.vw = 1.0f;

			if(UTL_EFT_CheckBound(&pos) >= 0)
			{
				SY_PRINTF2("In Bound (x:%f, y:%f, z:%f)\n", pos.vx, pos.vy, pos.vz);

				uvs0[j*4+0].a = uvs0[j*4+1].a = uvs0[j*4+2].a = uvs0[j*4+3].a = 0;
				uvs1[j*4+0].a = uvs1[j*4+1].a = uvs1[j*4+2].a = uvs1[j*4+3].a = 0;
				uvs2[j*4+0].a = uvs2[j*4+1].a = uvs2[j*4+2].a = uvs2[j*4+3].a = 0;
				uvs3[j*4+0].a = uvs3[j*4+1].a = uvs3[j*4+2].a = uvs3[j*4+3].a = 0;
			}
		}
	}
}

/******************************************************************************
 * メッセージ処理

mesg ＶＲ弾痕 $s:名前 on[1]
mesg ＶＲ弾痕 $s:名前 off[0]
 */
static void Act_Message(
	WORK *work)		/* ワーク */
{
	GV_MSG *msg;
	int    i, j;

	for(i = GV_ReceiveMessage(work->name, &msg); i > 0; i--, msg++)
	{
		SY_PRINTF2("Received Message.\n");

		switch(work->flag = msg->message[0])
		{
		case 0:
			/* 非表示 */
			for(j = work->n_scar; --j >= 0; )
			{
				if(work->scar[j].prim1) DG_InvisiblePrim2(work->scar[i].prim1);
				if(work->scar[j].prim2) DG_InvisiblePrim2(work->scar[i].prim2);
			}
			break;

		case 1:
			/* 表示 */
			for(j = work->n_scar; --j >= 0; )
			{
				if(work->scar[j].prim1) DG_VisiblePrim2(work->scar[i].prim1);
				if(work->scar[j].prim2) DG_VisiblePrim2(work->scar[i].prim2);
			}
			break;

		case 2:
			break;
		}
	}
}

/******************************************************************************
 * ユニットの処理
 */
static void Act_SetUnit(
	SCAR           *scar,
	UNIT           *unit,
	FVECTOR        *d_pos1,
	FVECTOR        *s_pos1,
	FVECTOR        *d_pos2,
	FVECTOR        *s_pos2,
	DG_PRIM2_UVRGB *d_uvrgb1,
	DG_PRIM2_UVRGB *s_uvrgb1,
	DG_PRIM2_UVRGB *d_uvrgb2,
	DG_PRIM2_UVRGB *s_uvrgb2)
{
	switch(unit->flag & UNIT_PHASEMASK)
	{
	case UNIT_PHASE1:
		unit->count++;

		/* スケール */
		SetUnitPos(unit, d_pos1, unit->count / (float)UNIT_PHASE1_COUNT);

		/* 回転 */
		SetUnitUV(scar, d_uvrgb1, unit->count * WLLSCR_ROTATE + M_PI / 4 + M_PI);

		/* 判定 */
		if(unit->count > UNIT_PHASE1_COUNT - 1)
		{
			SetUnitPos(unit, s_pos1, unit->count / (float)UNIT_PHASE1_COUNT);
			SetUnitUV(scar, s_uvrgb1, unit->count * WLLSCR_ROTATE + M_PI / 4 + M_PI);

			SetUnitPos(unit, d_pos2, 1.5);
			SetUnitPos(unit, s_pos2, 1.5);

			unit->flag &= ~UNIT_PHASEMASK;
			if(!(unit->flag & UNIT_NOPHASE2))
				unit->flag |= UNIT_PHASE2;
			unit->count = 0;
		}
		break;

	case UNIT_PHASE2:
		unit->count++;

		/* フェード */
		(d_uvrgb1 + 3)->a = (d_uvrgb1 + 2)->a = (d_uvrgb1 + 1)->a = d_uvrgb1->a =
			WLLSCR_ALPHA_1 * ((UNIT_PHASE2_COUNT - unit->count) / (float)UNIT_PHASE2_COUNT);

		(d_uvrgb2 + 3)->a = (d_uvrgb2 + 2)->a = (d_uvrgb2 + 1)->a = d_uvrgb2->a =
			WLLSCR_ALPHA_2 * (unit->count / (float)UNIT_PHASE2_COUNT);

		/* 判定 */
		if(unit->count > UNIT_PHASE2_COUNT - 1)
		{
			(s_uvrgb1 + 3)->a = (s_uvrgb1 + 2)->a = (s_uvrgb1 + 1)->a = s_uvrgb1->a =
				WLLSCR_ALPHA_1 * ((UNIT_PHASE2_COUNT - unit->count) / (float)UNIT_PHASE2_COUNT);

			(s_uvrgb2 + 3)->a = (s_uvrgb2 + 2)->a = (s_uvrgb2 + 1)->a = s_uvrgb2->a =
				WLLSCR_ALPHA_2 * (unit->count / (float)UNIT_PHASE2_COUNT);

			unit->flag &= ~UNIT_PHASEMASK;
			unit->flag |= UNIT_PHASE3;
			unit->count = 0;
		}
		break;

	case UNIT_PHASE3:
		break;

	default:
		break;
	}
}

/******************************************************************************
 * 毎フレーム処理
 */
static void Act(
	WORK *work)		/* ワーク */
{
	int            i, j, k;
	SCAR           *s;
	FVECTOR        *d_pos1  , *s_pos1  , *d_pos2  , *s_pos2  ;
	DG_PRIM2_UVRGB *d_uvrgb1, *s_uvrgb1, *d_uvrgb2, *s_uvrgb2;

	Act_Message(work);
   //AR_PARTICLE_HALF
   if(!AS_WillPrimBuffSwitch())
   {
      return;
   }

	for(j = work->n_scar, s = work->scar; --j >= 0; s++)
	{
		DG_SwitchBuffPrim2(s->prim1);
		DG_SwitchBuffPrim2(s->prim2);
		d_pos1   = s->prim1->pos  [    s->prim1->buffer_clock];
		s_pos1   = s->prim1->pos  [1 - s->prim1->buffer_clock];
		d_pos2   = s->prim2->pos  [    s->prim2->buffer_clock];
		s_pos2   = s->prim2->pos  [1 - s->prim2->buffer_clock];
		d_uvrgb1 = s->prim1->uvrgb[    s->prim1->buffer_clock];
		s_uvrgb1 = s->prim1->uvrgb[1 - s->prim1->buffer_clock];
		d_uvrgb2 = s->prim2->uvrgb[    s->prim2->buffer_clock];
		s_uvrgb2 = s->prim2->uvrgb[1 - s->prim2->buffer_clock];

		/* 各フェーズの処理 */
		if(s->front < s->n_pos)
		{
			for(i = s->front; i < s->n_pos; i++)
			{
				Act_SetUnit(s, &s->unit[i],
					&d_pos1  [i * 4], &s_pos1  [i * 4], &d_pos2  [i * 4], &s_pos2  [i * 4],
					&d_uvrgb1[i * 4], &s_uvrgb1[i * 4], &d_uvrgb2[i * 4], &s_uvrgb2[i * 4]);
			}
		}
		else if(!(s->flag & SCAR_NODATA))
		{
			for(i = 0; i < s->n_pos; i++)
			{
				Act_SetUnit(s, &s->unit[i],
					&d_pos1  [i * 4], &s_pos1  [i * 4], &d_pos2  [i * 4], &s_pos2  [i * 4],
					&d_uvrgb1[i * 4], &s_uvrgb1[i * 4], &d_uvrgb2[i * 4], &s_uvrgb2[i * 4]);
			}
			for(i = s->front; i < work->n_unit; i++)
			{
				Act_SetUnit(s, &s->unit[i],
					&d_pos1  [i * 4], &s_pos1  [i * 4], &d_pos2  [i * 4], &s_pos2  [i * 4],
					&d_uvrgb1[i * 4], &s_uvrgb1[i * 4], &d_uvrgb2[i * 4], &s_uvrgb2[i * 4]);
			}
		}

		/* 少しずつフェードアウトする弾痕 */
		k = s->front - s->n_pos;
		k = WLLSCR_MAX_DIFF - (k < 0 ? work->n_unit + k : k);

		for(i = s->front; --k >= 0; i = (i + 1) % work->n_unit)
		{
			DG_PRIM2_UVRGB *rgb;

			rgb = &d_uvrgb2[i * 4];
			if(rgb->a < 2)
			{
				(rgb + 3)->a = (rgb + 2)->a = (rgb + 1)->a = (rgb->a = 0);
				s->front++;
				s->front %= work->n_unit;
			}
			else
			{
				(rgb + 3)->a = (rgb + 2)->a = (rgb + 1)->a = (rgb->a -= 2);
			}
		}
	}
}

/******************************************************************************
 * 終了処理
 */
static void Die(
	WORK *work)		/* ワーク */
{
	int i;

	SY_PRINTF3("Die\n");

	for(i = work->n_scar; --i >= 0; )
	{
		if(work->scar[i].unit ) GV_Free(work->scar[i].unit);
		if(work->scar[i].prim2) GM_FreePrim2(work->scar[i].prim2);
		if(work->scar[i].prim1) GM_FreePrim2(work->scar[i].prim1);
	}
	GV_Free(work->scar);

	UTL_EFT_DelCallback(work);
}

/******************************************************************************
 * 初期化処理

chara ＶＲ弾痕[NewVRWallScarBase] $s:名前 \
	// 以下オプション
	-u $i:ユニット数
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	WORK *work,		/* ワーク */
	int  name)		/* シナリオ名 */
{
	int     i;
	DG_TEX  *texture[8][2];
	SCAR    *s;

	SY_PRINTF3("GetResources\n");

	UTL_EFT_AddCallback(BoundCallBack, work);

	/* テクスチャ */
	if(GCL_GetOption('m') != NULL)
	{
		/* 複数の種類 */
		for(i = 0; GCL_NextStr(); i++)
		{
			if(i > 7)
			{
				SY_PRINTF2("Too many textures! Get Texture Failed.\n");
				return 0;
			}

			if(    ((texture[i][0] = DG_GetTexture(GCL_GetNextInt())) == NULL)
				|| ((texture[i][1] = DG_GetTexture(GCL_GetNextInt())) == NULL))
			{
				SY_PRINTF2("No texture data! Get Texture Failed.\n");
				return 0;
			}
		}
		if(i == 0)
		{
			SY_PRINTF2("No texture! Get Texture Failed.\n");
			return 0;
		}
	}
	else
	{
		/* デフォルトの１種類のみ */
		i = 1;
		if(    ((texture[0][0] = DG_GetTexture(GV_StrCode(WLLSCR_TEXTURE_1))) == NULL)
			|| ((texture[0][1] = DG_GetTexture(GV_StrCode(WLLSCR_TEXTURE_2))) == NULL))
		{
			SY_PRINTF2("No texture data! Get Texture Failed.\n");
			return 0;
		}
	}

	/* ワークの初期化 */
	work->name   = name;
	work->flag   = 1;
	work->n_unit = GCL_GetOption('u') ? GCL_GetNextInt() : WLLSCR_MAX_UNIT;
	work->n_scar = i;

	/* メモリ確保 */
	if((work->scar = GV_Malloc(sizeof(SCAR) * i)) == NULL)
	{
		SY_PRINTF2("no memory!\n");
		return 0;
	}

	/* 弾痕種類配列 */
	for(s = work->scar; --i >= 0; s++)
	{
		int            j;
		DG_TEX         *tex;
		FVECTOR        *pos;
		DG_PRIM2_UVRGB *uvrgb;

		/* プリミティブの初期化 */
		if(    ((s->prim1 = GM_MakePrim2(WALLSCR_PRIM_FLAG, work->n_unit, 4)) == NULL)
			|| ((s->prim2 = GM_MakePrim2(WALLSCR_PRIM_FLAG, work->n_unit, 4)) == NULL))
		{
			SY_PRINTF2("Make Primitive Failed!\n");
			return 0;
		}
		s->prim1->group_id = 0x7fffffff;
		s->prim2->group_id = 0x7fffffff;
		DG_ConfigPrim2Tex(s->prim1, texture[i][0]);
		DG_ConfigPrim2Tex(s->prim2, texture[i][1]);

		/* プリミティブ１ */
		tex   = texture[i][0];
		pos   = MEM_ADDR1;
		uvrgb = MEM_ADDR2;

		sy_setuv(uvrgb, tex, 1, 1, 0);
//		s->u0 = ((uvrgb + 1)->u - (uvrgb)->u) / 4096.0f / 2.0f / 1.5f;
//		s->v0 = ((uvrgb + 2)->v - (uvrgb)->v) / 4096.0f / 2.0f / 1.5f;
//		s->r  = pow(s->u0 * s->u0 + s->v0 * s->v0, 0.5);
//		s->u0 = ((uvrgb + 1)->u - (uvrgb)->u) / 4096.0f / 2.0f + (uvrgb)->u / 4096.0f;
//		s->v0 = ((uvrgb + 2)->v - (uvrgb)->v) / 4096.0f / 2.0f + (uvrgb)->v / 4096.0f;

		for(j = 0; j < work->n_unit; j++)
		{
			uvrgb[0].a = uvrgb[1].a = uvrgb[2].a = uvrgb[3].a = 0;
			uvrgb[0].f = uvrgb[1].f = uvrgb[2].f = uvrgb[3].f = 0x0fff;
			uvrgb[0].q = uvrgb[1].q = uvrgb[2].q = uvrgb[3].q = 4096;

			_sceVu0CopyVector(&pos[0], &DG_ZeroVector);
			_sceVu0CopyVector(&pos[1], &DG_ZeroVector);
			_sceVu0CopyVector(&pos[2], &DG_ZeroVector);
			_sceVu0CopyVector(&pos[3], &DG_ZeroVector);

			pos   += 4;
			uvrgb += 4;
		}
		sy_memcpy(s->prim1->pos[0]  , MEM_ADDR1, sizeof(FVECTOR)       , work->n_unit * 4);
		sy_memcpy(s->prim1->uvrgb[0], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), work->n_unit * 4);
		sy_memcpy(s->prim1->pos[1]  , MEM_ADDR1, sizeof(FVECTOR)       , work->n_unit * 4);
		sy_memcpy(s->prim1->uvrgb[1], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), work->n_unit * 4);

//		DG_SetPrim2Alpha(s->prim1, SCE_GS_SET_ALPHA(0, 1, 0, 1, 0x00));
		DG_SetPrim2Alpha(s->prim1, SCE_GS_SET_ALPHA(0, 2, 0, 1, 0x00));
		s->prim1->raise = 0;

		/* プリミティブ２ */
		tex   = texture[i][1];
		pos   = MEM_ADDR1;
		uvrgb = MEM_ADDR2;

		for(j = 0; j < work->n_unit; j++)
		{
			sy_setuv(uvrgb, tex, 1, 1, 0);

			uvrgb += 4;
		}
		sy_memcpy(s->prim2->pos[0]  , MEM_ADDR1, sizeof(FVECTOR)       , work->n_unit * 4);
		sy_memcpy(s->prim2->uvrgb[0], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), work->n_unit * 4);
		sy_memcpy(s->prim2->pos[1]  , MEM_ADDR1, sizeof(FVECTOR)       , work->n_unit * 4);
		sy_memcpy(s->prim2->uvrgb[1], MEM_ADDR2, sizeof(DG_PRIM2_UVRGB), work->n_unit * 4);

//		DG_SetPrim2Alpha(s->prim2, SCE_GS_SET_ALPHA(0, 1, 0, 1, 0x00));
		DG_SetPrim2Alpha(s->prim2, SCE_GS_SET_ALPHA(0, 2, 0, 1, 0x00));
		s->prim2->raise = 0;

		/* リングバッファの初期化 */
		s->front = 0;
		s->n_pos = 0;
		s->flag  = SCAR_NODATA;

		s->tex1 = texture[i][0];

		/* メモリ確保 */
		if((s->unit = GV_Malloc(sizeof(UNIT) * work->n_unit)) == NULL)
		{
			SY_PRINTF2("no memory!\n");
			return 0;
		}
		memset(s->unit, 0x00, sizeof(UNIT) * work->n_unit);
	}
	SY_PRINTF1("Set Scar Array Succeeded.\n");

	/* ワークへのポインタを格納 */
	wk = work;

	return 1;
}

/******************************************************************************
 * 起動処理
 */
void *NewVRWallScarBase(
	int name,	/* シナリオ名 */
	int where)	/* マップＩＤ */
{
	WORK *work;

	SY_PRINTF3("NewVRWallScarBase\n");
	OPERATOR();

	/* アクター生成 */
	work = (WORK *)GV_NewActor(
		GV_ACTOR_USER,
		sizeof(WORK));
	if(work != NULL)
	{
		/* アクター登録 */
		GV_SetActor(&work->actor, Act, Die);

		/* ワークの初期化 */
		if(!GetResources(work, name))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}

/******************************************************************************
 * 弾痕リスト登録コマンド

command ＶＲ弾痕ノーマル[NewEntryVRWallScar] -n $b:登録番号
 */
void NewEntryVRWallScar(void)
{
	SY_PRINTF3("NewEntryVRWallScar\n");

	GM_EntryScarFunction((void *)NewVRWallScar, GCL_GetOptionValue('n', 32));
}
