/*
	cergei_eri.h
		装備品ヘッダファイル

	2000/04/06 K.Kano
	$Id: cergei_eri.h,v 1.1.1.3 2002/11/19 11:43:06 Yoshizawa1 Exp $
*/


#ifndef _cergei_eri_h_
#define _cergei_eri_h_

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


#define MAX_OBJS		8

#if 0

/* 45/2度のsinとcosの値 */
#define DEFAULT_COS_LIMIT		0.923879533f
#define DEFAULT_SIN_LIMIT		0.382683432f

#else

/* 90/2度のsinとcosの値 */
#define DEFAULT_COS_LIMIT		0.707106781f
#define DEFAULT_SIN_LIMIT		0.707106781f

#endif

enum {
	CERGEI_ERI_FLAG_CALC_DISABLE=0x00000001,
};

typedef struct {
	int n_objs;

    /* モデルとモデルの骨番号 */
    DG_EVMOBJ *target;

    int tobjnum[MAX_OBJS],parent[MAX_OBJS];

    /* モデルの情報 */
    FVECTOR rt[MAX_OBJS],t[MAX_OBJS],prt[MAX_OBJS];

	/* 計算想定点(ローカル座標) */
	FVECTOR lastobjx[MAX_OBJS];

	/* 前回のローカルクォータニオン */
	FVECTOR q[MAX_OBJS];

	/* 前回の親座標、座標、速度 */
	FVECTOR px[MAX_OBJS],x[MAX_OBJS],v[MAX_OBJS];

	/* 前回のモデルの位置 */
	FVECTOR root_x;

	/* 前回のワールドマトリクス */
	FMATRIX pwm[MAX_OBJS];

	/* 角度の限界値 */
	float cos_limit;
	float sin_limit;

	/* 角度に対して戻る力の算出に使うパラメータ */
	float deg_param;

	/* 速度に対する抵抗値 */
	float k;

	/* 風の影響パラメータ */
	float pa;

	/* 速度の何割を無視するかのパラメータ */
	float mov_rate,ymov_rate;

	int flag;
} CERGEI_ERI_WORK;


/* cergei_eri_act.c */
void CergeiEriFirstCalc(CERGEI_ERI_WORK *p);
int InitCergeiEri(CERGEI_ERI_WORK *p);
void ExitCergeiEri(CERGEI_ERI_WORK *p);
void MoveCergeiEri(CERGEI_ERI_WORK *p);

extern FVECTOR	G_wind;
int OK_GetLocalWind( FVECTOR *pos, FVECTOR *output );


#endif
