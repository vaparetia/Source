/*
	attachment.h
		装備品ヘッダファイル

	1999/11/04 K.Kano
	$Id: attachment.h,v 1.1.1.3 2002/11/19 11:43:03 Yoshizawa1 Exp $
*/


#ifndef _attachment_h_
#define _attachment_h_

#include <stdio.h>
#include <stdlib.h>
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


typedef struct {

    /* 前回のワールド座標 */
    FVECTOR x[2];

    /* 前回の速度 */
    FVECTOR v[2];

} MODEL_VARIABLES;

typedef struct {
    FMATRIX light[2];

    /* モデルに取り付ける場合に用いるワールドマトリクス */
    FMATRIX root;

    /* 回転角をマトリクスに展開したもの */
    FMATRIX rotm;

    /* 固定位置 */
    FVECTOR x[1];

    /* 一番先の仮想オブジェクトの座標
       バウンダリボックスから計算 */
    FVECTOR lastobjx;

    /* 可動軸の計算に用いるベクトル */
    FVECTOR axis;

    /* 可動軸数 */
    int n_axis;

    /* モデルの取り付け先 */
    FMATRIX *tmat;

    /* モデルにつける場合の、相手のOBJECTとOBJ番号 */
    OBJECT *target;
    int tobjnum;


    /* オブジェごとの質量の配列 */
    float m;

    /* 速度に対する抵抗値 */
    float k;

    /* 壁に対する反発係数 */
    float we;

    /* 回転角のリミットを角度で指定 */
    float sin_limit;
    float cos_limit;

    /* 楕円球用パラメータ */
    float param_oval;

    MODEL_VARIABLES mval;

	int first_flag;

} MODEL_PARAMETER;


typedef struct {

    /* オブジェごとの質量の配列 */
    float m;

    /* 速度に対する抵抗値 */
    float k;

    /* 壁に対する反発係数 */
    float we;

} SAMPLE_MODEL_PARAMETER;


/* attachment.c */
void Attachment_GetOptionValue_called(MODEL_PARAMETER *mp,DG_OBJS *obj,
				      FVECTOR *v,SVECTOR *r,
				      OBJECT *target,int objnum,FVECTOR *x,
				      int *a,int angle_limit,float oval_param);

/* attachact.c */
int InitAttachment(DG_OBJS *obj,MODEL_PARAMETER *p);
void InitCalcAttachment(MODEL_PARAMETER *p);
void MoveAttachment(DG_OBJS *obj,MODEL_PARAMETER *p);


#endif
