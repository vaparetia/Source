/*******************************************************************************
 * outline - outline.h
 * アウトライン
 * 2002/03/11 S.Yamashita
 * $Id: outline.h,v 1.1.1.3 2002/11/19 11:51:51 Yoshizawa1 Exp $
 */

#ifndef __INC_OUTLINE__
#define __INC_OUTLINE__

/*******************************************************************************
 * include
 */

#include "libdg.h"

/*******************************************************************************
 * definitions and typedefs and structures
 */

/* シグナル */
enum
{
	OL_SIG_SetColorA = 0,	/* アウトライン 色配列Ａの設定                                引数: CVECTOR[4] へのポインタ */
	OL_SIG_SetColorB,		/* アウトライン 色配列Ｂの設定                                引数: CVECTOR[4] へのポインタ */
	OL_SIG_SetColorAB,		/* アウトライン 色ＡＢの設定                                  引数: OL_COLOR へのポインタ */
	OL_SIG_SetColor,		/* アウトライン 色の設定(0～3: 色Ａの0～3  4～7: 色Ｂの0～3)  引数: OL_COLOR へのポインタ */
	OL_SIG_SetRange,		/* アウトライン Ｘ座標・Ｙ座標・Ｚ座標・％範囲の設定          引数: OL_RANGE へのポインタ */
};

/*
 * アウトライン色情報
 */
typedef struct tagOL_COLOR
{
	int     code;		/* シグナル識別コード  7851734 (outline) */
	int     col_num;	/* 色番号              0～4 */
	CVECTOR col[2];		/* 色(要素0: 色Ａ  要素1: 色Ｂ) */
}
OL_COLOR;

/*
 * アウトライン範囲情報
 */
typedef struct tagOL_RANGE
{
	int   code;			/* シグナル識別コード  7851734 (outline) */
	int   mode;			/* 0:Ｘ方向  1:Ｙ方向  2:Ｚ方向  3:部分(全体)  4:部分タイプ２(間接ごと) */
	float range[2];		/* mode=0～2: 座標範囲  mode=3～4: ％範囲(0.0 ～ 1.0) */
}
OL_RANGE;

/*
 * フラグ
 */
#define OL_FLAG_FILEMASK     (0x0000ffff)	/* ファイルフラグマスク */
#define OL_FLAG_COL_MODIFIED (0x00000001)	/* 色更新 */

#define OL_FLAG_OPTIONMASK   (0x00ff0000)	/* 動作フラグマスク */
#define OL_FLAG_ANTIALIASING (0x00010000)	/* アンチエイリアス */
#define OL_FLAG_ONEOBJ       (0x00020000)	/* 間接なし */
#define OL_FLAG_ERASETRANS   (0x00040000)	/* 移動値を消す */

#if 0
#define OL_FLAG_MODEMASK     (0xff000000)	/* 動作フラグマスク */
#define OL_FLAG_SLIDE        (0x01000000)	/* 色スライドモード */
#define OL_FLAG_PGRAD        (0x02000000)	/* 部分グラデーションモード（全体） */
#define OL_FLAG_PGRAD2       (0x04000000)	/* 部分グラデーションモード タイプ２（間接ごと） */
#define OL_FLAG_XGRAD        (0x10000000)	/* Ｘグラデーションモード */
#define OL_FLAG_YGRAD        (0x20000000)	/* Ｙグラデーションモード */
#define OL_FLAG_ZGRAD        (0x40000000)	/* Ｚグラデーションモード */
#endif

#define OLO_FLAG_COLORMASK   (0x00000003)	/* カラーマスク */
#define OLO_FLAG_COLOR1      (0x00000000)	/* カラー１ */
#define OLO_FLAG_COLOR2      (0x00000001)	/* カラー２ */
#define OLO_FLAG_COLOR3      (0x00000002)	/* カラー３ */
#define OLO_FLAG_COLOR4      (0x00000003)	/* カラー４ */

#define OLV_FLAG_DRAWEDGE    (0x00000001)	/* エッジを描画 */

/*
 * アウトライン頂点
 */
typedef struct tagOL_VERTEX
{
	int   flag;		/* フラグ */
	float fx;		/* Ｘ座標 */
	float fy;		/* Ｙ座標 */
	float fz;		/* Ｚ座標 */
}
OL_VERTEX;

/*
 * アウトラインオブジェクト
 */
typedef struct tagOL_OBJECT
{
	int      id;		/* ＩＤ */
	int      flag;		/* フラグ */
	int      prims;		/* プリミティブ数 */
	int      verts;		/* 頂点数 */
	int      verts2;	/* 頂点数２ */
	int      edge;		/* エッジ数 */
	DG_PRIM2 *dgprims;	/* ＤＧプリミティブ */
}
OL_OBJECT;

/*
 * アウトライン
 */
typedef struct tagOUTLINE
{
	int       flag;				/* フラグ */
	int       objcount;			/* オブジェクト数 */
	int       edge;				/* 総エッジ数 */
	OL_OBJECT *ol_obj;			/* アウトラインオブジェクト配列 */
	void      *mem_le;   		/* メモリアドレス */
	DG_OBJS   *dg_objs;			/* ＤＧオブジェクト */
	CVECTOR   col[2][4];		/* 色配列 */

	FVECTOR   offset;			/* オフセット */

#if 0
	int       slide_time;		/* スライド時間 */
	int       slide_counter;	/* スライドカウンター */
	float     x_range[2];		/* Ｘグラデーションモード   座標範囲配列 */
	float     y_range[2];		/* Ｙグラデーションモード   座標範囲配列 */
	float     z_range[2];		/* Ｚグラデーションモード   座標範囲配列 */
	float     p_range[2];		/* 部分グラデーションモード ％範囲配列（0.0 ～ 1.0） */
	char      x_col_num;		/* Ｘグラデーションモード   色番号 */
	char      y_col_num;		/* Ｙグラデーションモード   色番号 */
	char      z_col_num;		/* Ｚグラデーションモード   色番号 */
	char      p_col_num;		/* 部分グラデーションモード 色番号 */
#endif
}
OUTLINE;

/*******************************************************************************
 * functions
 */

int CreateOutline       (OUTLINE *outline, void *mem_le, DG_OBJS *dg_objs, CVECTOR *col_a, FVECTOR *offset, int flag, int was_swapped );
void DestroyOutline     (OUTLINE *outline);
void ActOutline         (OUTLINE *outline);

void OutlineShow        (OUTLINE *outline, int show);
void OutlineSetColorA   (OUTLINE *outline, CVECTOR *col_a);
void OutlineSetColorB   (OUTLINE *outline, CVECTOR *col_b);
void OutlineSetColorAB  (OUTLINE *outline, int col_num, CVECTOR *col_ab);
void OutlineSetColor    (OUTLINE *outline, int col_num, CVECTOR *col);
void OutlineSetXYZRange (OUTLINE *outline, int mode, float *range);
void OutlineSetPRange   (OUTLINE *outline, int mode, float *range);
#if 0
void OutlineSetSlideMode(OUTLINE *outline, int slide_time);
void OutlineSetGradMode (OUTLINE *outline, int mode, float *range, char col_num);
void OutlineSetGradRange(OUTLINE *outline, int mode, float *range);
#endif

/*******************************************************************************
 */

#endif	/* __INC_OUTLINE__ */
