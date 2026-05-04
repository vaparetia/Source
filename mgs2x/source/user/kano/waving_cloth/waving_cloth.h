/*
	waving_cloth.h
		布シミュレート
		waving_cloth.cを稼働させるためのヘッダファイル

	1999/08/16 K.Kano
	$Id: waving_cloth.h,v 1.1.1.3 2002/11/19 11:43:45 Yoshizawa1 Exp $
*/


#ifndef _waving_cloth_h_
#define _waving_cloth_h_



#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include <float.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "libutl.h"
#include "utl_dma.h"


#define	CLOTHMODEL_NAME	GV_StrCode( "布モデル" )

#if 1
#define OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE)
#else
#define OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#endif


typedef struct {
	int id;
	int n_verts;
	int n_lines;
	int n_polys;
} CLOTHDATA_FILE;


typedef ALIGN16_DECL(struct) {
    /* ポリゴンを構成する三頂点 */
    int index[3];

    /* このポリゴンの外積の向きを揃えるパラメーター */
    float normal_sign;

} POLY_PARAMETER;

typedef ALIGN16_DECL(struct) {
    /* ラインの繋がり */
    int index[2];

    /* ラインの初期長さ */
    float l0;

    /* 大きさを揃えるためのダミー */
    int dummy;

} LINE_PARAMETER;


typedef struct {
    /* この点を共有するポリゴンの数の逆数 */
    float inv_n;

    /* ネスト、固定点からの距離 */
    float fnest;

} VERTEX_PARAMETER;


typedef struct {
    /* 空気によって加えられる力の割合 */
    float pa;

    /* 頂点同士を結ぶバネの強さ */
    float spring_u;

    /* ポリゴンの元位置から、どれだけ離れることができるかのリミット値 */
    float xlimit;

    /* 元位置に戻ろうとするバネの係数 */
    float recover_u;

    /* 速度に対する抵抗の割合 */
    float k;
} MODEL_PARAMETER;


/* ワーク */
typedef	struct waving_cloth__Work {
    GV_ACT_EX	actor ;		/* リンク */

	DG_OBJS		*objs;		/* モデル */
    // OBJECT		body ;		/* モデル */

    FMATRIX		light[2];

    /* このcharaの名前 */
    int		address;

    /* 強制表示フラグ */
    int		force_disp_enable;

    /* 何かのモデルについてくる布モデルの場合 */
    OBJECT	*target;
    float	param_oval;
    DG_DEF	*boundmodel;

    FMATRIX	*troot,*tmat;

    FMATRIX	rot_mat;

    /* 頂点アニメコントロール用 */
    DG_VERTS_ANIME va;

    /* 頂点情報を格納したファイル */
    CV2_DEF	**cvd_def;
    int		cvd_size;
    int		cvd_sel;

    /* モデル固有のパラメータ */
    MODEL_PARAMETER	model_param;


    /* 頂点の総数 */
    int num_vers;

    /* 計算した基準位置を格納 */
    FVECTOR *base;

    /* この点の現在位置 */
    FVECTOR *now;
    /* この点の現在の移動量 */
    FVECTOR *v;
    /* 次の移動量との差分 */
    FVECTOR *dv;
    /* この点の法線 */
    FVECTOR *normal;

    /* モデルでの法線の値 */
    FVECTOR *lbase_normal;
    FVECTOR *base_normal;


    /* 頂点ごとのパラメータ */
    VERTEX_PARAMETER	*ver_params;

    /* ポリゴンごとのパラメータ */
    int num_polys;
    POLY_PARAMETER	*poly_params;

    /* ラインごとのパラメータ */
    int num_lines;
    LINE_PARAMETER	*line_params;

	/* 当たりの計算方法の変更フラグ */
	int collision_flag;
} Work ;


/* waving_clothsub.c */
int InitClothModel(Work *work);
void ExitClothModel(Work *work);
int InitClothModel_withoutDATAFILE(Work *work);
void ExitClothModel_withoutDATAFILE(Work *work);


void WavingCloth_CalcBase(Work *work,FMATRIX *m);
void MoveWavingCloth(Work *work);

void WavingClothWithOBJ_CalcBase(Work *work);
void MoveWavingClothWithOBJ(Work *work);

/* jacket.c */
void CopyMatrixforJacket(Work *work);


#endif
