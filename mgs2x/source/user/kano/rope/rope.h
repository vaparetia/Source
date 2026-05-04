/*
	rope.h
		紐形式のモデルを、風やモデルの動きに対応させて動かす
		rope.cを稼働させるためのヘッダファイル

	1999/09/16 K.Kano
	$Id: rope.h,v 1.1.1.3 2002/11/19 11:43:34 Yoshizawa1 Exp $
*/


#ifndef _rope_h_
#define _rope_h_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <sys/types.h>

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


typedef struct {

    /* 前回のワールド座標 */
    FVECTOR *x;

    /* 前回の速度 */
    FVECTOR *v;

    /* 基準張力 */
    FVECTOR *base_t;

} MODEL_VARIABLES;

typedef struct {
    /* モデルに取り付ける場合に用いるワールドマトリクス */
    FMATRIX root;
    FMATRIX *tmat;

    /* モデルにつける場合の、相手のOBJECTとOBJ番号 */
    OBJECT *target;
    int tobjnum;

	FVECTOR target_pos;

    /* 空気によって加えられる力の割合の配列 */
    float pa;

    /* オブジェごとの質量の配列 */
    float m;

    /* オブジェごとの質量の逆数の配列 */
    float inv_m;

    /* 速度に対する抵抗値 */
    float k;

    /* 壁に対する反発係数 */
    float we;

    /* 壁検出距離 */
    int wl;

    /* 楕円球用パラメータ */
    float param_oval;

    /* 当たり判定を行なうかどうかのフラグ */
    int collision_flag;

    /* 強制的に表示するかどうかのフラグ */
    int visible_flag;

    /* バンダナモードと髪の毛モード */
    int mode;

    MODEL_VARIABLES mval;

    /* ターゲットシステム */
    TARGET *targetsys;

    /* バウンディングのチェックに使うモデル */
    DG_DEF *boundmodel;

	/* 全体の移動の何割を無視するかのパラメータ */
	float mov_rate,ymov_rate;
} MODEL_PARAMETER;


typedef struct {

    /* 空気によって加えられる力の割合の配列 */
    float pa;

    /* オブジェごとの質量の配列 */
    float m;

    /* 速度に対する抵抗値 */
    float k;

    /* 壁に対する反発係数 */
    float we;

    /* 壁検出距離 */
    int wl;

	/* 全体の移動の何割を無視するかのパラメータ */
	float mov_rate;
	float ymov_rate;

} SAMPLE_MODEL_PARAMETER;


typedef struct {

    /* 基準位置 */
    FVECTOR *basex;

    /* OBJを動かす面の法線 */
    FVECTOR *base_normal;

    /* 前回のワールド座標 */
    FVECTOR *x;

    /* 前回の速度 */
    FVECTOR *v;

    /* 各OBJの稼働範囲限界 */
    float *limit;

} MODEL_VARIABLES2;

typedef struct {
    /* モデルに取り付ける場合に用いるワールドマトリクス */
    FMATRIX root;
    FMATRIX *tmat;

    /* モデルにつける場合の、相手のOBJECTとOBJ番号 */
    OBJECT *target;
    int tobjnum;

    /* 空気によって加えられる力の割合の配列 */
    float pa;

    /* オブジェごとの質量の配列 */
    float m;

    /* オブジェごとの質量の逆数の配列 */
    float inv_m;

    /* 速度に対する抵抗値 */
    float k;

    /* バネ定数 */
    float sp;

    /* リミット算出の際の係数 */
    float k_limit;

    /* 壁に対する反発係数 */
    float we;

    /* 壁検出距離 */
    int wl;

    /* 楕円球用パラメータ */
    float param_oval;

    /* 当たり判定を行なうかどうかのフラグ */
    int collision_flag;

    /* 強制的に表示するかどうかのフラグ */
    int visible_flag;

    MODEL_VARIABLES2 mval;

    /* ターゲットシステム */
    TARGET *targetsys;

    /* ターゲットの基準回転 */
    FMATRIX *tbasem;

} MODEL_PARAMETER2;


typedef struct {

    /* 空気によって加えられる力の割合の配列 */
    float pa;

    /* オブジェごとの質量の配列 */
    float m;

    /* 速度に対する抵抗値 */
    float k;

    /* バネ定数 */
    float sp;

    /* リミット算出の際の係数 */
    float k_limit;

    /* 壁に対する反発係数 */
    float we;

    /* 壁検出距離 */
    int wl;

} SAMPLE_MODEL_PARAMETER2;


/* rope.c */
void Rope_WorldCalc(FMATRIX *s,DG_OBJS *objs,MODEL_PARAMETER *p);
int Rope_GetModelName(int *name);

/* rope1.c */
void InitLocate(DG_OBJS *objs,MODEL_PARAMETER *p);
int InitRope(DG_OBJS *objs,MODEL_PARAMETER *p);
void ExitRope(DG_OBJS *objs,MODEL_PARAMETER *p);
void MoveRope1(DG_OBJS *objs,HZX_GROUP_ID id,MODEL_PARAMETER *p);

/* rope2.c */
void Rope2_CalcBase(DG_OBJS *objs,MODEL_PARAMETER2 *p);
void Rope2_SetTraget(DG_OBJS *objs,MODEL_PARAMETER2 *p);
int InitRope2(DG_OBJS *objs,MODEL_PARAMETER2 *p);
void ExitRope2(DG_OBJS *objs,MODEL_PARAMETER2 *p);
void MoveRope2(DG_OBJS *objs,HZX_GROUP_ID id,MODEL_PARAMETER2 *p);

/* rope3.c */
void MoveRope3(DG_OBJS *objs,HZX_GROUP_ID id,MODEL_PARAMETER *p);


extern FVECTOR	G_wind;
int OK_GetLocalWind( FVECTOR *pos, FVECTOR *output );


#endif
