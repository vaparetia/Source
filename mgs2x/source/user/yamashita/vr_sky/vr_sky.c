//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * vr_sky.c
 * ＶＲ空  *NewVRSky
 * 2002/03/26 S.Yamashita
 * $Id: vr_sky.c,v 1.1.1.3 2002/11/19 11:51:55 Yoshizawa1 Exp $
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
#include "libutl.h"

#include "../../shibata/util/ts_util.h"

#define __CHARA_NAME__ "VR Sky"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
#include "../sy_util/sy_util.h"

/*******************************************************************************
 * defines
 */

#define N_PRIMS             (12)
#define N_VERTS             (8 + 1)
#define N_ALLVERTS          (N_VERTS * N_PRIMS)
#define N_STRIP_VERTS       (8)
#define N_STRIPS            (12)
#define N_MAX_MODLS         (4)			/* 最大モデル数 */

#define SKY_COLOR_R         (130)		/* デフォルトベース用環境光Ｒ */
#define SKY_COLOR_G         (200)		/* デフォルトベース用環境光Ｇ */
#define SKY_COLOR_B         (250)		/* デフォルトベース用環境光Ｂ */
#define SKY_BASESCALE_RATIO (1.5f)		/* ベースのスケール係数 */
#define SKY_SCALE           (32.0f)
#define CLOUD_COLOR         (255)
#define CLOUD_ALPHA         (128)

#define SKY_FLAGS_NO_MOVE   (0x0001)	/* アニメーションなし */
#define SKY_FLAGS_NO_BASE   (0x0002)	/* ベースなし */
#define SKY_FLAGS_NO_CLOUD  (0x0004)	/* 雲テクスチャなし */
#define SKY_FLAGS_CAMERA    (0x0008)	/* カメラアングル対応処理 */

#define SKY_AXISFLAG_XROT   (0x0001)	/* Ｘ軸回転 */
#define SKY_AXISFLAG_YROT   (0x0002)	/* Ｙ軸回転 */
#define SKY_AXISFLAG_ZROT   (0x0004)	/* Ｚ軸回転 */

#define MEM_SCR_UV          ((char*)(SCRPAD_ADDR))
#define MEM_SCR_POS         ((char*)(MEM_SCR_UV  + sizeof(DG_PRIM2_UVRGB) * N_ALLVERTS))
#define MEM_SCR_TMP         ((char*)(MEM_SCR_POS + sizeof(FVECTOR)        * N_ALLVERTS))

/*******************************************************************************
 * work
 */

typedef struct tagWORK
{
	GV_ACT_EX actor;							/* アクター */
	int       name;								/* シナリオ名 */
	int       where;							/* マップＩＤ */
	CVECTOR   color;							/* ベース用環境光 */
	FVECTOR   center;							/* 中心座標 */
	float     sky_bottom;						/* ボトムサイズ (デフォルト: 6000) */
	float     scale;							/* スケール (デフォルト: 110) */
	int       flags;							/* フラグ */

	/* ベース */
	DG_PRIM2  *base_prim;						/* ベースプリミティブ */
	FVECTOR   bound[12][2];						/*  */
	FMATRIX   root;								/* ルート */

	/* 雲テクスチャ */
	DG_PRIM2  *base_cloud[4];					/* 各雲プリミティブ */
	float     base_cloud_angle[4];				/* 各雲Ｙ軸回転値 */
	FVECTOR   base_cloud_pos[4][30];			/*  */
	float     cloud_speed[N_MAX_MODLS + 4];		/* 各雲・各モデル Ｙ軸回転度 */
	CVECTOR   cloud_color[4];					/* 雲用環境光 */
	int       cloud_num;						/* 雲テクスチャ数 */

	/* 各モデル */
	CVECTOR   mdl_color[N_MAX_MODLS];			/* モデル用環境光 */
	int       mdl_num;							/* モデル数 */
	FMATRIX   obj_root[N_MAX_MODLS];			/* 各モデルのルート */
	float     base_rot_y[N_MAX_MODLS];			/* 各モデル初期Ｙ軸回転値 */
	float     rot_y[N_MAX_MODLS];				/* 各モデルＹ軸回転値 */
	FVECTOR   *obj_rots[N_MAX_MODLS][2];		/* 各モデルの関節の軸回転度・軸回転値 */
	DG_OBJS   *objs[N_MAX_MODLS];				/* 各モデルオブジェ */
	FMATRIX   lights[N_MAX_MODLS][2];			/* 各モデルのライト */

	CVECTOR   bas_change_color;					/* ベース用環境光変化 目標色 */
	int       bas_change_frame;					/* ベース用環境光変化 フレーム数 */
	int       bas_change_count;					/* ベース用環境光変化 カウンター */
	CVECTOR   clo_change_color[4];				/* 雲用環境光変化 目標色 */
	int       clo_change_frame[4];				/* 雲用環境光変化 フレーム数 */
	int       clo_change_count[4];				/* 雲用環境光変化 カウンター */
	CVECTOR   mdl_change_color[N_MAX_MODLS];	/* モデル用環境光変化 目標色 */
	int       mdl_change_frame[N_MAX_MODLS];	/* モデル用環境光変化 フレーム数 */
	int       mdl_change_count[N_MAX_MODLS];	/* モデル用環境光変化 カウンター */
}
WORK;

/*******************************************************************************
 * local
 */

static FVECTOR InitStripVertsPos[] = {
	{    0.000f, 10000.000f, 0.000f, 1.0f },
	{ 7071.068f,  8828.727f, 0.000f, 1.0f },
	{10000.000f,  6000.000f, 0.000f, 1.0f },
	{10000.000f,     0.000f, 0.000f, 1.0f },
};

/*******************************************************************************
 * static
 */
/*******************************************************************************
 * 最大値の取得
 */
static float WhichMax3(
	float a,
	float b,
	float c)
{
	if(a > b)
	{
		if(a > c) return a;
		else      return c;
	}
	else
	{
		if(b > c) return b;
		else      return c;
	}
}

/*******************************************************************************
 * 最小値の取得
 */
static float WhichMin3(
	float a,
	float b,
	float c)
{
	if(a < b)
	{
		if(a < c) return a;
		else      return c;
	}
	else
	{
		if(b < c) return b;
		else      return c;
	}
}

/*******************************************************************************
 * 空の表示ＯＦＦ
 */
static void InvAllSkyPrimObj(
	WORK *work)		/* ワーク */
{
	int i;

	if(work->base_prim    ) DG_InvisiblePrim2(work->base_prim    );
	if(work->base_cloud[0]) DG_InvisiblePrim2(work->base_cloud[0]);
	if(work->base_cloud[1]) DG_InvisiblePrim2(work->base_cloud[1]);
	if(work->base_cloud[2]) DG_InvisiblePrim2(work->base_cloud[2]);
	if(work->base_cloud[3]) DG_InvisiblePrim2(work->base_cloud[3]);

	for(i = 0; i < work->mdl_num; i++)
	{
		if(work->objs[i])
		{
			DG_InvisibleObjs(work->objs[i]);
		}
	}

	SY_PRINTF1("はい消えたー\n");
}

/*******************************************************************************
 * 空の表示ＯＮ
 */
static void VisAllSkyPrimObj(
	WORK *work)		/* ワーク */
{
	int i;

	if(work->base_prim    ) DG_VisiblePrim2(work->base_prim    );
	if(work->base_cloud[0]) DG_VisiblePrim2(work->base_cloud[0]);
	if(work->base_cloud[1]) DG_VisiblePrim2(work->base_cloud[1]);
	if(work->base_cloud[2]) DG_VisiblePrim2(work->base_cloud[2]);
	if(work->base_cloud[3]) DG_VisiblePrim2(work->base_cloud[3]);

	for(i = 0; i < work->mdl_num; i++)
	{
		if(work->objs[i])
		{
			DG_VisibleObjs(work->objs[i]);
		}
	}

	SY_PRINTF1("みえたー\n");
}

/*******************************************************************************
 * 
 */
static void MoveBaseCloud(
	WORK  *work,		/* ワーク */
	float add_angle,
	int   num)
{
	int      clock;
	DG_PRIM2 *prim    = work->base_cloud[num];
	FVECTOR  *pos;
	FMATRIX  world;
	FVECTOR  force    = {0.0f, 0.0f, 0.0f, 0.0f};
	FVECTOR  *eye_vec = (FVECTOR*)DG_Chanls[0].eye.m[2];

	if(!prim)
		return;

	DG_SwitchBuffPrim2(prim);
	clock = prim->buffer_clock;
	pos   = prim->pos[clock];

	_sceVu0ScaleVector(pos, eye_vec, 1600000.0f);
	pos++;
	force.vx = cosf(work->base_cloud_angle[num]);
	force.vz = sinf(work->base_cloud_angle[num]);

	TS_MakeMatrix(&world, &force, &DG_ZeroVector);

	DG_SetPos(&world);
	DG_PutVector(work->base_cloud_pos[num], pos, 30);
	
	work->base_cloud_angle[num] += add_angle;
	if(work->base_cloud_angle[num] > PI)
		work->base_cloud_angle[num] -= 2.0f * PI;
}

/*******************************************************************************
 * actor
 */
/*******************************************************************************
 * メッセージ処理

mesg ＶＲ空 $s:name 表示[0]
mesg ＶＲ空 $s:name 非表示[1]
mesg ＶＲ空 $s:name ベース用環境光変化[2] $i:目標Ｒ  $i:目標Ｇ  $i:目標Ｂ  $i:変化フレーム数
mesg ＶＲ空 $s:name 雲用環境光変化[3]     $i:番号  $i:目標Ｒ  $i:目標Ｇ  $i:目標Ｂ  $i:変化フレーム数
mesg ＶＲ空 $s:name モデル用環境光変化[4] $i:番号  $i:目標Ｒ  $i:目標Ｇ  $i:目標Ｂ  $i:変化フレーム数
 */
static int CheckMesgParam(
	WORK *work)		/* ワーク */
{
	GV_MSG *msg;
	int    mes_num;
	int    num;
	int    ret = 0;
	int    number;

	mes_num = GV_ReceiveMessage(work->name, &msg);
	msg    += mes_num - 1;

	while(--mes_num >= 0)
	{
		num = msg->message[0];
		switch(num)
		{
		case 0:
			/* 空の表示ＯＮ */
			VisAllSkyPrimObj(work);
			ret = 0;
			break;

		case 1:
			/* 空の表示ＯＦＦ */
			InvAllSkyPrimObj(work);
			ret = 1;
			break;

		case 2:
			/* ベース用環境光変化 */
			if(work->bas_change_frame == 0)
			{
				work->bas_change_color.r = msg->message[1];
				work->bas_change_color.g = msg->message[2];
				work->bas_change_color.b = msg->message[3];
				work->bas_change_frame   = msg->message[4];
				work->bas_change_count   = 0;
			}
			ret = 0;
			break;

		case 3:
			/* 雲用環境光変化 */
			number = msg->message[1];
			if(number < 4)
			{
				if(work->clo_change_frame[number] == 0)
				{
					work->clo_change_color[number].r = msg->message[2];
					work->clo_change_color[number].g = msg->message[3];
					work->clo_change_color[number].b = msg->message[4];
					work->clo_change_frame[number]   = msg->message[5];
					work->clo_change_count[number]   = 0;
				}
			}
			ret = 0;
			break;

		case 4:
			/* モデル用環境光変化 */
			number = msg->message[1];
			if(number < N_MAX_MODLS)
			{
				if(work->mdl_change_frame[number] == 0)
				{
					work->mdl_change_color[number].r = msg->message[2];
					work->mdl_change_color[number].g = msg->message[3];
					work->mdl_change_color[number].b = msg->message[4];
					work->mdl_change_frame[number]   = msg->message[5];
					work->mdl_change_count[number]   = 0;
				}
			}
			ret = 0;
			break;

		default:
			SY_PRINTF2("Msg Err!![%d]\n", num);
		}
		msg--;
	}

	return ret;
}

/*******************************************************************************
 * 毎フレーム処理
 */
static void Act(
	WORK *work)		/* ワーク */
{
	int      i, j, k;
	int      clock, flags;
	DG_PRIM2 *base_prim = work->base_prim;
	DG_PRIM2_UVRGB *uvrgb;

	if(CheckMesgParam(work))
	{
		GV_WaitMessage(work, work->name);
		return;
	}

	/* 各雲 */
	if(!(work->flags & (SKY_FLAGS_NO_MOVE|SKY_FLAGS_NO_CLOUD)))
	{
		MoveBaseCloud(work, PI / 65536.0f * 1.0f * work->cloud_speed[0] / 100.0f, 0);
		MoveBaseCloud(work, PI / 65536.0f * 2.0f * work->cloud_speed[1] / 100.0f, 1);
		MoveBaseCloud(work, PI / 65536.0f * 3.0f * work->cloud_speed[2] / 100.0f, 2);
		MoveBaseCloud(work, PI / 65536.0f * 4.0f * work->cloud_speed[3] / 100.0f, 3);

		/* 雲用環境光変化 */
		for(k = 0; k < work->cloud_num; k++)
		{
			if(work->clo_change_frame[k] != 0)
			{
				uvrgb = work->base_cloud[k]->uvrgb[work->base_cloud[k]->buffer_clock];

				work->clo_change_count[k]++;
				uvrgb++;
				for(i = 0; i < 30; i++)
				{
					uvrgb->r = work->cloud_color[k].r
						+ (((work->clo_change_color[k].r - work->cloud_color[k].r) * work->clo_change_count[k])
							/ work->clo_change_frame[k]);
					uvrgb->g = work->cloud_color[k].g
						+ (((work->clo_change_color[k].g - work->cloud_color[k].g) * work->clo_change_count[k])
							/ work->clo_change_frame[k]);
					uvrgb->b = work->cloud_color[k].b
						+ (((work->clo_change_color[k].b - work->cloud_color[k].b) * work->clo_change_count[k])
							/ work->clo_change_frame[k]);
					uvrgb++;
				}
				if(work->clo_change_count[k] >= work->clo_change_frame[k])
				{
					work->clo_change_frame[k] = 0;
					work->cloud_color[k] = work->clo_change_color[k];
				}
			}
		}
	}

	/* 各モデル */
	for(i = 0; i < work->mdl_num; i++)
	{
		if(!(work->flags & SKY_FLAGS_NO_MOVE))
		{
			float rot_y;

			work->rot_y[i] += PI / 65536.0f * work->cloud_speed[i + 4] / 100.0f;
			while(work->rot_y[i] >  PI) work->rot_y[i] -= 2.0f * PI;
			while(work->rot_y[i] < -PI) work->rot_y[i] += 2.0f * PI;

			rot_y = work->rot_y[i] + work->base_rot_y[i];
			while(rot_y >  PI) rot_y -= 2.0f * PI;
			while(rot_y < -PI) rot_y += 2.0f * PI;

			_sceVu0RotMatrixY(&work->obj_root[i], &work->root, rot_y);
			_sceVu0ScaleVector(work->obj_root[i].m[0], work->obj_root[i].m[0], work->scale);
			_sceVu0ScaleVector(work->obj_root[i].m[1], work->obj_root[i].m[1], work->scale);
			_sceVu0ScaleVector(work->obj_root[i].m[2], work->obj_root[i].m[2], work->scale);

			DG_COPY_VEC(work->obj_root[i].m[3], work->root.m[3]);

			/* 軸回転 */
			for(j = 0; j < work->objs[i]->n_models; j++)
			{
				(work->obj_rots[i][1] + j)->vx += PI / 65536.0f * (work->obj_rots[i][0] + j)->vx / 100.0f;
				while((work->obj_rots[i][1] + j)->vx >  PI) (work->obj_rots[i][1] + j)->vx -= 2.0f * PI;
				while((work->obj_rots[i][1] + j)->vx < -PI) (work->obj_rots[i][1] + j)->vx += 2.0f * PI;

				(work->obj_rots[i][1] + j)->vy += PI / 65536.0f * (work->obj_rots[i][0] + j)->vy / 100.0f;
				while((work->obj_rots[i][1] + j)->vy >  PI) (work->obj_rots[i][1] + j)->vy -= 2.0f * PI;
				while((work->obj_rots[i][1] + j)->vy < -PI) (work->obj_rots[i][1] + j)->vy += 2.0f * PI;

				(work->obj_rots[i][1] + j)->vz += PI / 65536.0f * (work->obj_rots[i][0] + j)->vz / 100.0f;
				while((work->obj_rots[i][1] + j)->vz >  PI) (work->obj_rots[i][1] + j)->vz -= 2.0f * PI;
				while((work->obj_rots[i][1] + j)->vz < -PI) (work->obj_rots[i][1] + j)->vz += 2.0f * PI;
			}
		}

		/* カメラアングル対応処理 */
		if(work->flags & SKY_FLAGS_CAMERA)
		{
			DG_COPY_VEC((FVECTOR*)work->obj_root[i].m[3], (FVECTOR*)DG_Chanls[0].eye.m[3]);
			work->obj_root[i].m[3][1] = work->center.vy;
		}
	}
	/* モデル用環境光変化 */
	for(k = 0; k < work->mdl_num; k++)
	{
		if(work->mdl_change_frame[k] != 0)
		{
			work->mdl_change_count[k]++;
			{
				work->lights[k][1].m[3][0] = work->mdl_color[k].r
					+ (((work->mdl_change_color[k].r - work->mdl_color[k].r) * work->mdl_change_count[k])
						/ work->mdl_change_frame[k]);
				work->lights[k][1].m[3][1] = work->mdl_color[k].g
					+ (((work->mdl_change_color[k].g - work->mdl_color[k].g) * work->mdl_change_count[k])
						/ work->mdl_change_frame[k]);
				work->lights[k][1].m[3][2] = work->mdl_color[k].b
					+ (((work->mdl_change_color[k].b - work->mdl_color[k].b) * work->mdl_change_count[k])
						/ work->mdl_change_frame[k]);
			}
			if(work->mdl_change_count[k] >= work->mdl_change_frame[k])
			{
				work->mdl_change_frame[k] = 0;
				work->mdl_color[k] = work->mdl_change_color[k];
			}
		}
	}

	/* ベースプリミティブ */
	if(!(work->flags & SKY_FLAGS_NO_BASE) && base_prim)
	{
		FVECTOR *eye_vec = (FVECTOR*)DG_Chanls[0].eye.m[2];
		FVECTOR *pos;

		DG_SwitchBuffPrim2(base_prim);
		clock = base_prim->buffer_clock;
		pos   = base_prim->pos[clock];

		for(i = 0; i < N_PRIMS; i++)
		{
			flags = DG_BoundCheck(&work->root, &work->bound[i][0], &work->bound[i][1]);
			base_prim->packet[clock][i].flag  = base_prim->packet[clock][i].flag & ~(0x03 << 12);
			base_prim->packet[clock][i].flag |= (flags & 0x3) << 12;
			_sceVu0ScaleVector(pos, eye_vec, 16000000.0f);

			pos += N_VERTS;
		}

		/* ベース用環境光変化 */
		if(work->bas_change_frame != 0)
		{
			uvrgb = base_prim->uvrgb[clock];

			work->bas_change_count++;
			for(i = 0; i < N_PRIMS; i++)
			{
				for(j = 0; j < N_VERTS; j++)
				{
					if(j != 0)
					{
						uvrgb->r = work->color.r
							+ (((work->bas_change_color.r - work->color.r) * work->bas_change_count)
								/ work->bas_change_frame);
						uvrgb->g = work->color.g
							+ (((work->bas_change_color.g - work->color.g) * work->bas_change_count)
								/ work->bas_change_frame);
						uvrgb->b = work->color.b
							+ (((work->bas_change_color.b - work->color.b) * work->bas_change_count)
								/ work->bas_change_frame);
					}
					uvrgb++;
				}
			}
			if(work->bas_change_count >= work->bas_change_frame)
			{
				work->bas_change_frame = 0;
				work->color = work->bas_change_color;
			}
		}
	}
}

/*******************************************************************************
 * 終了処理
 */
static void Die(
	WORK *work)		/* ワーク */
{
	int i;

	SY_PRINTF3("Die\n");

	for(i = 0; i < work->mdl_num; i++)
	{
		if(work->obj_rots[i][0]) GV_Free(work->obj_rots[i][0]);
		if(work->obj_rots[i][1]) GV_Free(work->obj_rots[i][1]);
	}

	if(work->base_prim    ) GM_FreePrim2(work->base_prim    );
	if(work->base_cloud[0]) GM_FreePrim2(work->base_cloud[0]);
	if(work->base_cloud[1]) GM_FreePrim2(work->base_cloud[1]);
	if(work->base_cloud[2]) GM_FreePrim2(work->base_cloud[2]);
	if(work->base_cloud[3]) GM_FreePrim2(work->base_cloud[3]);

	for(i = 0; i < work->mdl_num; i++)
	{
		if(work->objs[i])
		{
			DG_DequeueObjs(work->objs[i]);
			DG_FreeObjs(work->objs[i]);
		}
	}
}

/*******************************************************************************
 * ベースプリミティブの初期化
 */
static int InitBasePrim(
	WORK *work)		/* ワーク */
{
	int            i;
	DG_TEX         *tex;
	DG_PRIM2       *prim;
	FVECTOR        *pos;
	DG_PRIM2_UVRGB *uvrgb;
	FMATRIX        world;
	FVECTOR        *fvtemp;

	FVECTOR        scale = {
		work->scale * SKY_BASESCALE_RATIO,
		work->scale * SKY_BASESCALE_RATIO,
		work->scale * SKY_BASESCALE_RATIO,
		0.0f};
	SVECTOR        rot = {0, 0, 0, 0};

	SY_PRINTF3("InitBasePrim\n");

	/* プリミティブの生成 */
	tex  = DG_GetTexture(0);
	prim = work->base_prim = GM_MakePrim2(DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE, N_PRIMS, N_VERTS);
	if(!prim)
	{
		SY_PRINTF2("ERR!! MAKE PRIM!!\n");
		return -1;
	}
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	prim->root  = &work->root;
	DG_ConfigPrim2Tex(prim, tex);
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA(0, 1, 0, 1, 0x00));
	prim->raise = -1000000;

	/* 天井のストリップの設定 */
	uvrgb = (DG_PRIM2_UVRGB *)MEM_SCR_UV;

	uvrgb[0].u = 0;
	uvrgb[0].v = 0;
	uvrgb[0].f = VERT_KICK_CODE;
	uvrgb[0].q = 0;
	uvrgb[0].r = 0;
	uvrgb[0].g = 0;
	uvrgb[0].b = 0;
	uvrgb[0].a = 0;
	uvrgb++;

	for(i = 0; i < N_STRIP_VERTS; i++)
	{
		uvrgb->u = 0;
		uvrgb->v = 0;
		uvrgb->f = (i < 2 || i == 5 || i == 6) ? VERT_KICK_CODE : DRAW_KICK_CODE;
		uvrgb->q = 4096;
		uvrgb->r = work->color.r;
		uvrgb->g = work->color.g;
		uvrgb->b = work->color.b;
		uvrgb->a = 128;
		uvrgb++;
	}

	pos    = (FVECTOR *)MEM_SCR_POS;
	uvrgb  = (DG_PRIM2_UVRGB *)MEM_SCR_UV;
	fvtemp = (FVECTOR *)MEM_SCR_TMP;

	for(i = 0; i < 4; i++)
	{
		memcpy(uvrgb, MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB) * (N_STRIP_VERTS + 1));
		DG_COPY_VEC(pos, &DG_ZeroVector);
		pos++;

		DG_SetPos2(&DG_ZeroVector,&rot);
		DG_GetPos(&world);
		TS_ScaleMatrix(&world, &world, &scale);
		DG_SetPos(&world);
		DG_PutVector(&InitStripVertsPos[0], &fvtemp[0], 3);
		rot.vy += 4096 / 8;
		DG_SetPos2(&DG_ZeroVector,&rot);
		DG_GetPos(&world);
		TS_ScaleMatrix(&world, &world, &scale);
		DG_SetPos(&world);
		DG_PutVector(&InitStripVertsPos[1], &fvtemp[3], 2);
		rot.vy += 4096 / 8;
		DG_SetPos2(&DG_ZeroVector,&rot);
		DG_GetPos(&world);
		TS_ScaleMatrix(&world, &world, &scale);
		DG_SetPos(&world);
		DG_PutVector(&InitStripVertsPos[1], &fvtemp[5], 2);

		DG_COPY_VEC(&pos[0],&fvtemp[0]);
		DG_COPY_VEC(&pos[1],&fvtemp[5]);
		DG_COPY_VEC(&pos[2],&fvtemp[1]);
		DG_COPY_VEC(&pos[3],&fvtemp[4]);
		DG_COPY_VEC(&pos[4],&fvtemp[2]);
		DG_COPY_VEC(&pos[5],&fvtemp[5]);
		DG_COPY_VEC(&pos[6],&fvtemp[4]);
		DG_COPY_VEC(&pos[7],&fvtemp[6]);

		work->bound[i][0].vx = WhichMax3(fvtemp[2].vx,fvtemp[6].vx,fvtemp[0].vx);
		work->bound[i][0].vy = fvtemp[0].vy;
		work->bound[i][0].vz = WhichMax3(fvtemp[2].vz,fvtemp[6].vz,fvtemp[0].vz);
		work->bound[i][0].vw = 1.0f;

		work->bound[i][1].vx = WhichMin3(fvtemp[2].vx,fvtemp[6].vx,fvtemp[0].vx);
		work->bound[i][1].vy = fvtemp[4].vy;
		work->bound[i][1].vz = WhichMin3(fvtemp[2].vz,fvtemp[6].vz,fvtemp[0].vz);
		work->bound[i][1].vw = 1.0f;

		pos   += N_STRIP_VERTS;
		uvrgb += N_STRIP_VERTS + 1;
	}

	/* 壁ストリップの設定 */
	uvrgb[0].u = 0;
	uvrgb[0].v = 0;
	uvrgb[0].f = VERT_KICK_CODE;
	uvrgb[0].q = 0;
	uvrgb[0].r = 0;
	uvrgb[0].g = 0;
	uvrgb[0].b = 0;
	uvrgb[0].a = 0;
	uvrgb++;

	for(i = 0; i < N_STRIP_VERTS; i++)
	{
		uvrgb->u = 0;
		uvrgb->v = 0;
		uvrgb->f = (i < 2) ? VERT_KICK_CODE : DRAW_KICK_CODE;
		uvrgb->q = 4096;
		uvrgb->r = work->color.r;
		uvrgb->g = work->color.g;
		uvrgb->b = work->color.b;
		uvrgb->a = (i >= 2) ? 128 : 0;
		uvrgb++;
	}

	uvrgb -= N_STRIP_VERTS + 1;
	rot.vy = 0;

	for(i = 0; i < 8; i++)
	{
		memcpy(uvrgb, &((DG_PRIM2_UVRGB*)MEM_SCR_UV)[4 * (N_STRIP_VERTS + 1)], sizeof(DG_PRIM2_UVRGB) * (N_STRIP_VERTS + 1));
		DG_COPY_VEC(pos, &DG_ZeroVector);
		pos++;

		DG_SetPos2(&DG_ZeroVector,&rot);
		DG_GetPos(&world);
		TS_ScaleMatrix(&world, &world, &scale);
		DG_SetPos(&world);
		DG_PutVector(&InitStripVertsPos[2], &fvtemp[0], 2);
		rot.vy += 4096 / 8;
		DG_SetPos2(&DG_ZeroVector,&rot);
		DG_GetPos(&world);
		TS_ScaleMatrix(&world, &world, &scale);
		DG_SetPos(&world);
		DG_PutVector(&InitStripVertsPos[2], &fvtemp[2], 2);

		DG_COPY_VEC(&pos[0],&fvtemp[1]);
		DG_COPY_VEC(&pos[1],&fvtemp[3]);
		DG_COPY_VEC(&pos[2],&fvtemp[0]);
		DG_COPY_VEC(&pos[3],&fvtemp[2]);
		DG_COPY_VEC(&pos[4],&fvtemp[0]);
		DG_COPY_VEC(&pos[5],&fvtemp[2]);
		DG_COPY_VEC(&pos[6],&fvtemp[0]);
		DG_COPY_VEC(&pos[7],&fvtemp[2]);

		work->bound[i + 4][0].vx = (fvtemp[0].vx > fvtemp[2].vx) ? fvtemp[0].vx : fvtemp[2].vx;
		work->bound[i + 4][0].vy = fvtemp[1].vy;
		work->bound[i + 4][0].vz = (fvtemp[0].vz > fvtemp[2].vz) ? fvtemp[0].vz : fvtemp[2].vz;
		work->bound[i + 4][0].vw = 1.0f;

		work->bound[i + 4][1].vx = (fvtemp[0].vx > fvtemp[2].vx) ? fvtemp[2].vx : fvtemp[0].vx;
		work->bound[i + 4][1].vy = fvtemp[0].vy;
		work->bound[i + 4][1].vz = (fvtemp[0].vz > fvtemp[2].vz) ? fvtemp[2].vz : fvtemp[0].vz;
		work->bound[i + 4][1].vw = 1.0f;

		pos[0].vy                = pos[2].vy - work->sky_bottom * work->scale * 1.5f;
		pos[1].vy                = pos[3].vy - work->sky_bottom * work->scale * 1.5f;
		work->bound[i + 4][0].vy = (pos[0].vy > pos[2].vy) ? pos[0].vy : pos[2].vy;
		work->bound[i + 4][1].vy = (pos[0].vy > pos[2].vy) ? pos[2].vy : pos[0].vy;

		pos   += N_STRIP_VERTS;
		uvrgb += N_STRIP_VERTS + 1;
	}

	/* メモリの転送 */
	TS_Scr_Mem( prim->pos[0]  , MEM_SCR_POS, sizeof(FVECTOR)       , N_ALLVERTS);
	TS_Scr_Mem( prim->pos[1]  , MEM_SCR_POS, sizeof(FVECTOR)       , N_ALLVERTS);
	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV , sizeof(DG_PRIM2_UVRGB), N_ALLVERTS);
	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV , sizeof(DG_PRIM2_UVRGB), N_ALLVERTS);

	return 0;
}

/*******************************************************************************
 * 雲プリミティブの初期化
 */
static DG_PRIM2 *InitBaseCloud(
	WORK  *work,		/* ワーク */
	float len,			/*  */
	float off_y,		/*  */
	float len_y,		/*  */
	short rot_vy,		/*  */
	int   num,			/*  */
	int   tex_code,		/*  */
	int   color,		/*  */
	int   alpha,		/*  */
	int   pri)			/*  */
{
	int            i, j;
	DG_TEX         *tex;
	DG_PRIM2       *prim;
	FVECTOR        *pos;
	DG_PRIM2_UVRGB *uvrgb;
	SVECTOR        rot = {0, rot_vy, 0, 0};

	FVECTOR        InitPos[2] = {
		{0.0f, off_y * work->scale + len_y * work->scale, len * work->scale, 1.0f},
		{0.0f, off_y * work->scale                      , len * work->scale, 1.0f},
	};

	SY_PRINTF3("InitBaseCloud\n");

	/* プリミティブの生成 */
	tex = DG_GetTexture(tex_code);
	if(!tex) return NULL;
	prim = GM_MakePrim2(DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_TEX, 1, 15 * 2 + 1);
	if(!prim)
	{
		SY_PRINTF2("ERR!! MAKE PRIM!!\n");
		return NULL;
	}
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	prim->raise = -pri * 10 * (int)SKY_SCALE;
	prim->root  = &work->root;
	DG_ConfigPrim2Tex(prim, tex);
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA(0, 1, 0, 1, 0x00));

	/* 各頂点の設定 */
	pos   = (FVECTOR *)MEM_SCR_POS;
	uvrgb = (DG_PRIM2_UVRGB *)MEM_SCR_UV;

	DG_COPY_VEC(pos, &DG_ZeroVector);
	uvrgb[0].u = 0;
	uvrgb[0].v = 0;
	uvrgb[0].f = VERT_KICK_CODE;
	uvrgb[0].q = 0;
	uvrgb[0].r = 0;
	uvrgb[0].g = 0;
	uvrgb[0].b = 0;
	uvrgb[0].a = 0;
	pos++;
	uvrgb++;

	for(i = 0; i < 3; i++)
	{
		for(j = 0; j < 5; j++)
		{
			if(j == 4 && i == 2)
				rot.vy = rot_vy;

			DG_SetPos2(&work->center, &rot);
			DG_PutVector(InitPos,pos, 2);
			DG_RotVector(InitPos,&work->base_cloud_pos[num][(i * 5 + j) * 2], 2);

			if(j != 4)
				rot.vy += (short)(4096.0f / 12.0f);

			uvrgb[0].u = FTOI12(1.0f / 4.0f * (float)j * tex->u_scale + tex->u_offset);
			uvrgb[0].v = FTOI12(0.0f * tex->v_scale + tex->v_offset);
			uvrgb[0].f = (j) ? DRAW_KICK_CODE : VERT_KICK_CODE;
			uvrgb[0].q = 4096;
			uvrgb[0].r = color & 0xff;
			uvrgb[0].g = (color >>  8) & 0xff;
			uvrgb[0].b = (color >> 16) & 0xff;
			uvrgb[0].a = alpha;

			uvrgb[1].u = FTOI12(1.0f / 4.0f * (float)j * tex->u_scale + tex->u_offset);
			uvrgb[1].v = FTOI12(1.0f * tex->v_scale + tex->v_offset);
			uvrgb[1].f = (j) ? DRAW_KICK_CODE : VERT_KICK_CODE;
			uvrgb[1].q = 4096;
			uvrgb[1].r = color & 0xff;
			uvrgb[1].g = (color >>  8) & 0xff;
			uvrgb[1].b = (color >> 16) & 0xff;
			uvrgb[1].a = alpha;

			pos   += 2;
			uvrgb += 2;
		}
	}

	/*  */
	work->base_cloud_angle[num] = 0.0f;

	/* メモリの転送 */
	TS_Scr_Mem(prim->pos[0]  , MEM_SCR_POS, sizeof(FVECTOR)       , 31);
	TS_Scr_Mem(prim->pos[1]  , MEM_SCR_POS, sizeof(FVECTOR)       , 31);
	TS_Scr_Mem(prim->uvrgb[0], MEM_SCR_UV , sizeof(DG_PRIM2_UVRGB), 31);
	TS_Scr_Mem(prim->uvrgb[1], MEM_SCR_UV , sizeof(DG_PRIM2_UVRGB), 31);

	return prim;
}

/*******************************************************************************
 * DG_OBJS の初期化
 */
static DG_OBJS *InitItemObjs(	/* オブジェ */
	int     code,		/* モデル名 */
	FMATRIX *root,		/* モデルのルート */
	FMATRIX *light)		/* モデルのライト */
{
	DG_DEF  *def;
	DG_OBJS *objs;

	SY_PRINTF3("InitItemObjs\n");

	/* DG_OBJS の生成 */
	def = (DG_DEF*)GV_GetCache(GV_CacheID(code, 'k'));
	if(!def) return NULL;
	objs = DG_MakeObjs(def, DG_FLAG_NOFOG, 0);
	if(!objs)                  return NULL;
	if(DG_QueueObjs(objs) < 0) return NULL;

	/* 設定 */
	if(root)  objs->root = root;
	if(light) DG_SetLightMatrix(objs, light);

	return objs;
}

/*******************************************************************************
 * 初期化処理

chara ＶＲ空[NewVRSky_Scn] $s:名前 \
	// 以下オプション
	-col $w:ベース用環境光ＲＧＢ ... \
	-pos $i:中心座標 ... \
	-b   $i:ベースのＹ方向高さ \
	-l   $i:スケール \
	-f   $i:フラグ \
	-t   $i:雲テクスチャ数 \
				$s:テクスチャ名  $i:環境光Ｒ  $i:環境光Ｇ  $i:環境光Ｂ  $i:Ｙ軸回転度 ...5 \
	-m   $i:補助モデル数 \
				$s:補助モデル名  $i:環境光Ｒ  $i:環境光Ｇ  $i:環境光Ｂ  $i:Ｙ軸回転度  $i:初期Ｙ軸回転値 ...6 \
	-h   $i:平行光データ数 \
				$s:対象補助モデル名  $i:平行光Ｒ  $i:平行光Ｇ  $i:平行光Ｂ  $i:平行光Ｘ  $i:平行光Ｙ  $i:平行光Ｚ ...7 \
	-r   $i:軸回転データ数 \
				$s:対象補助モデル名  $i:ノード番号（1ベース）  $b:フラグ  $i:軸回転度 ...4
 */
static int GetResources(	/* 1: 成功 */
							/* 0: 失敗 */
	WORK *work)		/* ワーク */
{
	int i, n, mdl_code;
	int code[N_MAX_MODLS];
	int p_count[N_MAX_MODLS];

	SY_PRINTF3("GetResources\n");

	work->bas_change_frame = 0;
	for(i = 0; i < 4; i++)
		work->clo_change_frame[i] = 0;
	for(i = 0; i < N_MAX_MODLS; i++)
		work->mdl_change_frame[i] = 0;

	/* シナリオオプション */
	if(GCL_GetOption('c'))
	{
		work->color.r = GCL_GetNextInt();
		work->color.g = GCL_GetNextInt();
		work->color.b = GCL_GetNextInt();
	}
	else
	{
		work->color.r = SKY_COLOR_R;
		work->color.g = SKY_COLOR_G;
		work->color.b = SKY_COLOR_B;
	}

	if(GCL_GetOption('p'))
	{
		work->center.vx = GCL_GetNextInt();
		work->center.vy = GCL_GetNextInt();
		work->center.vz = GCL_GetNextInt();
		work->center.vw = 1.0f;
	}
	else
	{
		work->center.vx = 0.0f;
		work->center.vy = 0.0f;
		work->center.vz = 0.0f;
		work->center.vw = 1.0f;
	}

	work->sky_bottom = (float)GCL_GetOptionValue('b', 6000);
	work->scale      = (float)GCL_GetOptionValue('l', 110);
	work->flags      = GCL_GetOptionValue('f', 0);

	/* ベースプリミティブの初期化 */
	if(!(work->flags & SKY_FLAGS_NO_BASE))
	{
		if(InitBasePrim(work))
			return 0;
	}

	/* ルート トランス値 */
	DG_COPY_MAT(&work->root, &DG_UnitMatrix);
	DG_COPY_VEC((FVECTOR*)work->root.m[3], &work->center);

	/* 雲プリミティブの初期化 */
	if((work->cloud_num = GCL_GetOptionValue('t', 0)) > 4)
	{
		SY_PRINTF2("too many texture. (max: 4)\n");
		return 0;
	}
	for(i = 0; i < work->cloud_num; i++)
	{
		int tex_code = GCL_GetNextInt();
		int rgb;

		rgb  = GCL_GetNextInt();
		rgb |= GCL_GetNextInt() << 8;
		rgb |= GCL_GetNextInt() << 16;
		work->cloud_color[i].r =  rgb        & 0xff;
		work->cloud_color[i].g = (rgb >>  8) & 0xff;
		work->cloud_color[i].b = (rgb >> 16) & 0xff;

		work->cloud_speed[i] = (float)GCL_GetNextInt();
		if(tex_code == 48)
			continue;
		work->base_cloud[i] = InitBaseCloud(
			work    , 9600.0f - 800.0f * (float)i, ((i < 2) ? 500.0f : 300.0f) ,
			6000.0f , i * 1024 / 4               , i                           ,
			tex_code, rgb                        , ((i < 2) ? 96 : CLOUD_ALPHA),
			i * 100);
	}

	/* モデルの初期化 */
	if((work->mdl_num = GCL_GetOptionValue('m', 0)) > N_MAX_MODLS)
	{
		SY_PRINTF2("too many model. (max: 4)\n");
		return 0;
	}
	for(i = 0; i < work->mdl_num; i++)
	{
		/* トランスとスケール */
		_sceVu0ScaleVector(work->obj_root[i].m[0], work->root.m[0], work->scale);
		_sceVu0ScaleVector(work->obj_root[i].m[1], work->root.m[1], work->scale);
		_sceVu0ScaleVector(work->obj_root[i].m[2], work->root.m[2], work->scale);
		DG_COPY_VEC(work->obj_root[i].m[3], &work->center);

		/* DG_OBJS の初期化 */
		code[i]                 = GCL_GetNextInt();
		work->objs[i]           = InitItemObjs(code[i], &work->obj_root[i], work->lights[i]);
		work->objs[i]->group_id = work->where;
		if(!work->objs[i])
			SY_PRINTF2("mdl code [%d] is nothing\n", code[i]);

		/* 関節の軸回転 */
		if(    ((work->obj_rots[i][0] = (FVECTOR*)GV_Malloc(sizeof(FVECTOR) * work->objs[i]->n_models)) == NULL)
			|| ((work->obj_rots[i][1] = (FVECTOR*)GV_Malloc(sizeof(FVECTOR) * work->objs[i]->n_models)) == NULL))
		{
			SY_PRINTF2("no memory\n");
			return 0;
		}
		memset(work->obj_rots[i][0], 0x00, sizeof(FVECTOR) * work->objs[i]->n_models);
		memset(work->obj_rots[i][1], 0x00, sizeof(FVECTOR) * work->objs[i]->n_models);
		work->objs[i]->rots = work->obj_rots[i][1];

		/* ライト、モデルのＹ軸回転 */
		work->mdl_color[i].r = work->lights[i][1].m[3][0] = (float)GCL_GetNextInt();
		work->mdl_color[i].g = work->lights[i][1].m[3][1] = (float)GCL_GetNextInt();
		work->mdl_color[i].b = work->lights[i][1].m[3][2] = (float)GCL_GetNextInt();
		work->cloud_speed[i + 4]   = (float)GCL_GetNextInt();
		work->base_rot_y[i]        = (float)GCL_GetNextInt() / 10000.0f;
		work->rot_y[i]             = 0.0f;
	}

	/* 平行光 */
	memset(p_count, 0x00, sizeof(p_count));
	for(n = GCL_GetOptionValue('h', 0); n > 0; n--)
	{
		// 対象モデルの検索
		mdl_code = GCL_GetNextInt();
		for(i = 0; i < work->mdl_num; i++)
		{
			if(mdl_code == code[i])
				break;
		}
		if(i == work->mdl_num)
		{
			for(i = 0; i < 6; i++)
				GCL_GetNextInt();
		}
		else
		{
			// 平行光の設定
			if(p_count[i] < 3)
			{
				FVECTOR vtmp;

				work->lights[i][1].m[p_count[i]][0] = (float)GCL_GetNextInt() / work->scale;
				work->lights[i][1].m[p_count[i]][1] = (float)GCL_GetNextInt() / work->scale;
				work->lights[i][1].m[p_count[i]][2] = (float)GCL_GetNextInt() / work->scale;
				work->lights[i][1].m[p_count[i]][3] = 0.0f;

				vtmp.vx = (float)GCL_GetNextInt();
				vtmp.vy = (float)GCL_GetNextInt();
				vtmp.vz = (float)GCL_GetNextInt();
				vtmp.vw = 0.0f;
				_sceVu0Normalize(&vtmp, &vtmp);

				work->lights[i][0].m[0][p_count[i]] = vtmp.vx;
				work->lights[i][0].m[1][p_count[i]] = vtmp.vy;
				work->lights[i][0].m[2][p_count[i]] = vtmp.vz;
				work->lights[i][0].m[3][p_count[i]] = 0.0f;

				p_count[i]++;
			}
		}
	}

	/* 関節の軸回転 */
	for(n = GCL_GetOptionValue('r', 0); n > 0; n--)
	{
		// 対象モデルの検索
		mdl_code = GCL_GetNextInt();
		for(i = 0; i < work->mdl_num; i++)
		{
			if(mdl_code == code[i])
				break;
		}
		if(i == work->mdl_num)
		{
			GCL_GetNextInt();
			GCL_GetNextInt();
			GCL_GetNextInt();
		}
		else
		{
			// 関節番号の取得
			int obj_num;
			if((obj_num = GCL_GetNextInt()) >= work->objs[i]->n_models)
			{
				GCL_GetNextInt();
				GCL_GetNextInt();
			}
			else
			{
				// 回転度の設定
				int   axis;
				float rot;
				axis = GCL_GetNextInt();
				rot  = (float)GCL_GetNextInt();

				if(axis & SKY_AXISFLAG_XROT) (work->obj_rots[i][0] + obj_num)->vx = rot;
				if(axis & SKY_AXISFLAG_YROT) (work->obj_rots[i][0] + obj_num)->vy = rot;
				if(axis & SKY_AXISFLAG_ZROT) (work->obj_rots[i][0] + obj_num)->vz = rot;
			}
		}
	}

	return 1;
}

/*******************************************************************************
 * 起動処理
 */
void *NewVRSky(
	int where)	/* マップＩＤ */
{
	WORK *work;

	SY_PRINTF3("NewVRSky\n");
	OPERATOR();

	/* エフェクトアクター生成 */
	work = (WORK*)GV_NewEffect(
		GV_ACTOR_EFFECT,
		sizeof(WORK));
	if(work != NULL)
	{
		/* アクター登録 */
		GV_SetActor(&work->actor, Act, Die);
		GV_ActorEX(&work->actor);

		/* ワークの初期化 */
		work->name  = 0;
		work->where = where;
		if(!GetResources(work))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}

void *NewVRSky_Scn(
	int name,	/* シナリオ名 */
	int where)	/* マップＩＤ */
{
	WORK *work;

	SY_PRINTF3("NewVRSky_Scn\n");
	OPERATOR();

	/* エフェクトアクター生成 */
	work = (WORK*)GV_NewEffect(
		GV_ACTOR_EFFECT,
		sizeof(WORK));
	if(work != NULL)
	{
		/* アクター登録 */
		GV_SetActor(&work->actor, Act, Die);
		GV_ActorEX(&work->actor);

		/* ワークの初期化 */
		work->name  = name;
		work->where = where;
		if(!GetResources(work))
		{
			SY_PRINTF2("GetResources Failed.\n");
			GV_DestroyActor(work);
			return NULL;
		}
	}

	return (void *)work;
}
