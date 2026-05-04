//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*******************************************************************************
 * sy_util - sy_util.c
 * 山下便利
 * 2002/03/20 S.Yamashita
 * $Id: sy_util.c,v 1.1.1.3 2002/11/19 11:51:54 Yoshizawa1 Exp $
 */

/*******************************************************************************
 * include
 */

#include <math.h>

#include "sy_util.h"

/*******************************************************************************
 * macros
 */

#define FTOI12(_f) (DG_FTOI(((float)(_f)*4096.0f)))

/*******************************************************************************
 * extern
 */

extern void DG_GetTexelInfo( int *width, int *height, int *offset_x, int *offset_y, DG_TEX *tex );

/*******************************************************************************
 * public
 */
/******************************************************************************
 * 座標値の設定
 */
void sy_setpos(
	FVECTOR *pos,		/* 座標配列 (左上、右上、左下、右下) */
	FVECTOR *center,	/* 中心 */
	float   width,		/* 幅/2 */
	float   height,		/* 高さ/2 */
	int     dir)		/* SY_DIR */
{
	if(dir < SY_DIR_EAST)
	{
		if(dir == SY_DIR_SOUTH)
			width  *= -1;

		(pos    )->vx = center->vx + width;
		(pos    )->vy = center->vy + height;
		(pos    )->vz = center->vz;
		(pos + 1)->vx = center->vx - width;
		(pos + 1)->vy = center->vy + height;
		(pos + 1)->vz = center->vz;
		(pos + 2)->vx = center->vx + width;
		(pos + 2)->vy = center->vy - height;
		(pos + 2)->vz = center->vz;
		(pos + 3)->vx = center->vx - width;
		(pos + 3)->vy = center->vy - height;
		(pos + 3)->vz = center->vz;
	}
	else if(dir < SY_DIR_UP)
	{
		if(dir == SY_DIR_WEST)
			width  *= -1;

		(pos    )->vx = center->vx;
		(pos    )->vy = center->vy + height;
		(pos    )->vz = center->vz + width;
		(pos + 1)->vx = center->vx;
		(pos + 1)->vy = center->vy + height;
		(pos + 1)->vz = center->vz - width;
		(pos + 2)->vx = center->vx;
		(pos + 2)->vy = center->vy - height;
		(pos + 2)->vz = center->vz + width;
		(pos + 3)->vx = center->vx;
		(pos + 3)->vy = center->vy - height;
		(pos + 3)->vz = center->vz - width;
	}
	else
	{
		if(dir == SY_DIR_DOWN)
			height *= -1;

		(pos    )->vx = center->vx - width;
		(pos    )->vy = center->vy;
		(pos    )->vz = center->vz - height;
		(pos + 1)->vx = center->vx + width;
		(pos + 1)->vy = center->vy;
		(pos + 1)->vz = center->vz - height;
		(pos + 2)->vx = center->vx - width;
		(pos + 2)->vy = center->vy;
		(pos + 2)->vz = center->vz + height;
		(pos + 3)->vx = center->vx + width;
		(pos + 3)->vy = center->vy;
		(pos + 3)->vz = center->vz + height;
	}
}

/******************************************************************************
 * ＵＶ値の設定
 */
void sy_setuv(
	DG_PRIM2_UVRGB *uvrgb,	/* ＵＶ配列 (左上、右上、左下、右下) */
	DG_TEX         *tex,	/* ＤＧテクスチャ */
	int            div_w,	/* テクスチャの横方向の分割数 */
	int            div_h,	/* テクスチャの縦方向の分割数 */
	int            index)	/* インデックス */
{
	int tw, th;
	int tex_w, tex_h, off_u, off_v;
	int w, h, ui, vi;
	int u[2], v[2];

	tw = (tex->tex_trans.tex0.data >> 26) & 0x0f;
	th = (tex->tex_trans.tex0.data >> 30) & 0x0f;
	tw = 1 << tw;
	th = 1 << th;

	DG_GetTexelInfo(&tex_w, &tex_h, &off_u, &off_v, tex);

	w  = tex_w / div_w;
	h  = tex_h / div_h;
	ui = (index % div_w) * w;
	vi = (index / div_w) * h;

	u[0] = FTOI12(((off_u + ui    ) + 0.5f) / tw);
	v[0] = FTOI12(((off_v + vi    ) + 0.5f) / th);
	u[1] = FTOI12(((off_u + ui + w) - 0.5f) / tw);
	v[1] = FTOI12(((off_v + vi + h) - 0.5f) / th);

	(uvrgb    )->u = u[0];
	(uvrgb    )->v = v[0];
	(uvrgb + 1)->u = u[1];
	(uvrgb + 1)->v = v[0];
	(uvrgb + 2)->u = u[0];
	(uvrgb + 2)->v = v[1];
	(uvrgb + 3)->u = u[1];
	(uvrgb + 3)->v = v[1];
}

/******************************************************************************
 * ＵＶ値の設定
 */
void sy_setuv2(
	DG_PRIM2_UVRGB *uvrgb,	/* ＵＶ配列 (左上、右上、左下、右下) */
	DG_TEX         *tex,	/* ＤＧテクスチャ */
	int            div_w,	/* テクスチャの横方向の分割数 */
	int            div_h,	/* テクスチャの縦方向の分割数 */
	int            index)	/* インデックス */
{
	int tw, th;
	int tex_w, tex_h, off_u, off_v;
	int w, h, ui, vi;
	int u[2], v[2];

	tw = (tex->tex_trans.tex0.data >> 26) & 0x0f;
	th = (tex->tex_trans.tex0.data >> 30) & 0x0f;
	tw = 1 << tw;
	th = 1 << th;

	DG_GetTexelInfo(&tex_w, &tex_h, &off_u, &off_v, tex);

	w  = tex_w / div_w;
	h  = tex_h / div_h;
	ui = (index % div_w) * w;
	vi = (index / div_w) * h;

	u[0] = FTOI12(((off_u + ui    ) + 0.5f) / tw);
	v[0] = FTOI12(((off_v + vi    ) + 0.5f) / th);
	u[1] = FTOI12(((off_u + ui + w) - 0.5f) / tw);
	v[1] = FTOI12(((off_v + vi + h) - 0.5f) / th);

	(uvrgb    )->u = u[1];
	(uvrgb    )->v = v[1];
	(uvrgb + 1)->u = u[0];
	(uvrgb + 1)->v = v[1];
	(uvrgb + 2)->u = u[1];
	(uvrgb + 2)->v = v[0];
	(uvrgb + 3)->u = u[0];
	(uvrgb + 3)->v = v[0];
}

/******************************************************************************
 * ＲＧＢＡ値の設定
 */
void sy_setrgba(
	DG_PRIM2_UVRGB *uvrgb,	/* ＵＶ配列 */
	u_short        r,		/* Ｒ値 */
	u_short        g,		/* Ｇ値 */
	u_short        b,		/* Ｂ値 */
	u_short        a,		/* Ａ値 */
	int            num)		/* 要素数 */
{
	for(; num > 0; num--)
	{
		(uvrgb + num - 1)->r = r;
		(uvrgb + num - 1)->g = g;
		(uvrgb + num - 1)->b = b;
		(uvrgb + num - 1)->a = a;
	}
}

/******************************************************************************
 * 座標値をグリッドに沿わせる
 */
float sy_grid(	/* グリッドに沿った座標 */
	float a,		/* 座標 */
	float grid)		/* グリッドます目 */
{
	if(grid == 0)
		return a;

	if(fmod(a, grid) > grid / 2)
	{
		a /= grid;
		if(a > 0) a = ceil(a);
		else      a = floor(a);
		a *= grid;
	}
	else
	{
		a /= grid;
		if(a > 0) a = floor(a);
		else      a = ceil(a);
		a *= grid;
	}

	return a;
}
