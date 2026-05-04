/*
	cloth.h
		布シミュレート
		cloth.cを稼働させるためのヘッダファイル

	1999/08/16 K.Kano
	$Id: cloth.h,v 1.1.1.3 2002/11/19 11:43:08 Yoshizawa1 Exp $
*/


#ifndef _cloth_h_
#define _cloth_h_


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
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
#define	OBJECT_FLAG	(DG_FLAG_SHADE | DG_FLAG_ONEPIECE)


typedef struct {
    /* ポリゴンを構成する三頂点 */
    int index[3];

    /* ポリゴンの単位垂直ベクトル */
    FVECTOR normal;

    /* このポリゴンの外積の向きを揃えるパラメーター */
    float normal_sign;

} POLY_PARAMETER ALIGN16;

typedef struct {
    /* ラインの繋がり */
    int index[2];

    /* ラインの初期長さ */
    float l0;

} LINE_PARAMETER ALIGN16;

enum {
    VERTEX_TYPE_FREE=0,
    VERTEX_TYPE_STATIC=0x100,
};

typedef struct {
    /* この点の属性、
       自由点、固定点、移動量固定点 */
    int type;

    /* この点を共有するポリゴンの数の逆数 */
    float inv_n;

    /* ネスト、固定点からの距離 */
    float fnest;

    /* この点の法線 */
    FVECTOR normal;

    /* この点の現在位置 */
    FVECTOR now;
    /* この点の現在の移動量 */
    FVECTOR v;
    /* 次の移動量との差分 */
    FVECTOR dv;

    /* 計算した基準位置を格納 */
    FVECTOR base;

    /* ローカル座標系での頂点位置 */
    FVECTOR lbase;

    /* この点の現在位置のローカル座標系での値 */
    FVECTOR lnow;

    /* この点の法線のローカル座標系での値 */
    FVECTOR lnormal;

} VERTEX_PARAMETER ALIGN16;


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
typedef	struct _Work {
    GV_ACT		actor ;		/* リンク */
    CONTROL		control ;	/* 移動制御 */
    OBJECT		body ;		/* モデル */

    /* 初期化したばかりの場合、前回の頂点の値が不定なので、
       それを判定するフラグ */
    int			flag;

    /* モデル固有のパラメータ */
    MODEL_PARAMETER	model_param;

    /* 各OBJの頂点が、どの頂点にあたるかの対応表 */
    int			**array_indexes_vertex;

    /* ポリゴンごとのパラメータ */
    int num_polys;
    POLY_PARAMETER	*poly_params;

    /* 頂点ごとのパラメータ */
    int num_vers;
    VERTEX_PARAMETER	*ver_params;

    /* ラインごとのパラメータ */
    int num_lines;
    LINE_PARAMETER	*line_params;

    /* 頂点の情報の展開先 */
    SVECTOR		**vertex[2];
    SVECTOR		**normal[2];

} Work ;


#endif
