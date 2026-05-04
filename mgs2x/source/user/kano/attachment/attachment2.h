/*
	attachment2.h
		装備品ヘッダファイル

	1999/11/04 K.Kano
	$Id: attachment2.h,v 1.1.1.3 2002/11/19 11:43:03 Yoshizawa1 Exp $
*/


#ifndef _attachment2_h_
#define _attachment2_h_

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
    FMATRIX		light[2];

    /* 回転角をマトリクスに展開したもの */
    FMATRIX rotm;

    /* 固定位置 */
    FVECTOR x[2];

    /* 一番先の仮想オブジェクトの座標
       バウンダリボックスから計算 */
    FVECTOR lastobjx;

    /* モデルに取り付ける場合に用いるワールドマトリクス */
    FMATRIX *tmat;

    FMATRIX *tmat2;

    /* モデルにつける場合の、相手のOBJECTとOBJ番号 */
    OBJECT *target;
    int tobjnum;

    /* 目標点を含むOBJ */
    int tobjnum2;

	/* 計算後の値の保存場所 */
	FVECTOR now_q;

	int first_flag;

} MODEL_PARAMETER2;


/* attachment2.c */
void Attachment2_GetOptionValue_called(MODEL_PARAMETER2	*mp,DG_OBJS *objs,
				       FVECTOR *v,SVECTOR *r,
				       OBJECT *target,int objnum,FVECTOR *x,int objnum2,FVECTOR *p);

/* attachact2.c */
int InitAttachment2(DG_OBJS *objs,MODEL_PARAMETER2 *p);
void MoveAttachment2(DG_OBJS *objs,MODEL_PARAMETER2 *p);


#endif
