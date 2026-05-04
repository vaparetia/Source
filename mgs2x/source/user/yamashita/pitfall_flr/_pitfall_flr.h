/*******************************************************************************
 * pitfall_flr - _pitfall_flr.h
 * 内部ヘッダー
 * 2002/03/13 S.Yamashita
 * $Id: _pitfall_flr.h,v 1.1.1.3 2002/11/19 11:51:52 Yoshizawa1 Exp $
 */

#ifndef __INC__PITFALL_FLR__
#define __INC__PITFALL_FLR__

/*******************************************************************************
 * include
 */

#include "pitfall_flr.h"

#define __CHARA_NAME__ "Pitfall Floor"
//#define SY_PRINTF1_DEBUG
#define SY_PRINTF2_DEBUG
#include "../sy_util/sy_util.h"

/*******************************************************************************
 * definitions and typedefs and structures
 */

#define MEM_ADDR1 ((void*)(SCRPAD_ADDR + 0x0000))
#define MEM_ADDR2 ((void*)(SCRPAD_ADDR + 0x2000))

#define TEXTURE_NAME "vr2_pitfall_b0_all_add_alp"	/* テクスチャ名 */
#define DEF_RGB      (128)							/* デフォルトの色 */
#define DEF_ALPHA    (48)							/* デフォルトの明るさ */
#define ALPHA_GLARE  (128.0f)						/* 光る時の明るさ */
#define POLY_W       (2000.0)						/* ポリゴンの幅 */
#define POLY_H       (2000.0)						/* ポリゴンの高さ */
#define N_DIV_H      (3)							/* テクスチャの横方向の分割数 */
#define N_DIV_V      (3)							/* テクスチャの縦方向の分割数 */
#define N_DIV        (N_DIV_H * N_DIV_V)			/* 総分割数 */
#define N_PRIMS      (1)							/* ポリゴン数 */
#define N_VERTS      (4)							/* 頂点数 */

/*
 * ワーク
 */
typedef struct tagWORK
{
	GV_ACT_EX actor;	/* アクター */
	int       where;	/* マップＩＤ */

	short     mode;		/* 動作モード */
	int       value;	/* シグナルの引数  各動作モードによって用途が変わる */
	int       value2;	/* シグナルの引数保管等 */
	int       value3;	/* シグナルの引数保管等 */
	DG_TEX    *tex;		/* ＤＧテクスチャ */
	DG_PRIM2  *prim;	/* ＤＧプリミティブ */
	FVECTOR   pos;		/* 位置 */
	SVECTOR   rot;		/* 回転 */
}
WORK;

/*******************************************************************************
 * macros
 */

#define RND(n)       (((BP_PS2_rand() >> 16) * n) >> 15)
#define FTOI12(_f)   (DG_FTOI(((float)(_f)*4096.0f)))
#define DEGtoANG(_a) ((_a) * 8192 / 45)
#define ANGtoDEG(_a) ((int)(_a) * 180 / 32768)
#define ANGtoRAD(_a) ((_a) * (float)M_PI / 32768.0f)

/*******************************************************************************
 * functions
 */

void PFF_Act(WORK *work);

/*******************************************************************************
 */

#endif	/* __INC__PITFALL_FLR__ */
