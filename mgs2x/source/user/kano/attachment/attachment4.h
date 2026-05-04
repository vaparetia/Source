/*
	attachment4.h
		装備品ヘッダファイル

	1999/12/03 K.Kano
	$Id: attachment4.h,v 1.1.1.3 2002/11/19 11:43:04 Yoshizawa1 Exp $
*/


#ifndef _attachment4_h_
#define _attachment4_h_

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
    FMATRIX light[2];

    /* 回転角をマトリクスに展開したもの */
    FMATRIX rotm;

    /* 固定位置 */
    FVECTOR x[1];

    /* 一番先の仮想オブジェクトの座標
       バウンダリボックスから計算 */
    FVECTOR lastobjx;

    /* モデルの取り付け先 */
    FMATRIX *tmat;

    /* モデルにつける場合の、相手のOBJECTとOBJ番号 */
    OBJECT *target;
    int tobjnum;

    FVECTOR *stockv;
    int stockp,stock_size;

    /* 回転角のリミットを角度で指定 */
    float sin_limit;
    float cos_limit;

	int first_flag;

} MODEL_PARAMETER4;


/* attachment.c */
void Attachment4_GetOptionValue_called(MODEL_PARAMETER4 *mp,DG_OBJS *obj,
				       FVECTOR *v,SVECTOR *r,OBJECT *target,int objnum,FVECTOR *x,
				       int angle_limit,int frames);

/* attachact.c */
void InitAttachment4(DG_OBJS *obj,MODEL_PARAMETER4 *p);
int Attachment4_GetStock(MODEL_PARAMETER4 *p);
void InitCalcAttachment4(MODEL_PARAMETER4 *p);
void ExitAttachment4(MODEL_PARAMETER4 *p);
void MoveAttachment4(DG_OBJS *obj,MODEL_PARAMETER4 *p);


#endif
