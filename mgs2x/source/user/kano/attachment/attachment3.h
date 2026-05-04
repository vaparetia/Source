/*
	attachment3.h
		装備品ヘッダファイル

	1999/11/04 K.Kano
	$Id: attachment3.h,v 1.1.1.3 2002/11/19 11:43:04 Yoshizawa1 Exp $
*/


#ifndef _attachment3_h_
#define _attachment3_h_

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
    FVECTOR x[1];

    /* モデルに取り付ける場合に用いるワールドマトリクス */
    FMATRIX *tmat;

    /* モデルにつける場合の、相手のOBJECTとOBJ番号 */
    OBJECT *target;
    int tobjnum;

    FMATRIX *stockm;
    float *stockh;
    int stockp,stock_size;

	int first_flag;

} MODEL_PARAMETER3;


/* attachment3.c */
void Attachment3_GetOptionValue_called(MODEL_PARAMETER3 *mp,DG_OBJS *objs,SVECTOR *r,
				       OBJECT *target,int objnum,FVECTOR *x,int frames);

/* attachact3.c */
int InitAttachment3(MODEL_PARAMETER3 *p);
void InitCalcAttachment3(MODEL_PARAMETER3 *p);
void ExitAttachment3(MODEL_PARAMETER3 *p);
void MoveAttachment3(DG_OBJS *objs,MODEL_PARAMETER3 *p);
void MoveAttachment3A(DG_OBJS *objs,MODEL_PARAMETER3 *p);


#endif
