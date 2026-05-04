/*
	mw_hukuyure.h
		装備品ヘッダファイル

	2000/04/06 K.Kano
	$Id: mw_hukuyure.h,v 1.1.1.3 2002/11/19 11:43:20 Yoshizawa1 Exp $
*/


#ifndef _mw_hukuyure_h_
#define _mw_hukuyure_h_


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
    /* モデルとモデルの骨番号 */
    DG_EVMOBJ *target;
    int tobjnum,parent;

    /* モデルの情報 */
    FVECTOR rt,t,prt;

    FMATRIX *stockm;
    int stockp,stock_size;

    /* yの上下幅の制限値 */
    float limity;
} MWHUKUYURE_WORK;

/* mw_hukuyure_act.c */
int InitMWHukuyure(MWHUKUYURE_WORK *p);
void ExitMWHukuyure(MWHUKUYURE_WORK *p);
void MoveMWHukuyure(MWHUKUYURE_WORK *p);
void MoveMWHukuyure2(MWHUKUYURE_WORK *p);

void *NewMWHukuyure_called(OBJECT *target,int objnum,int frames,float limity);


#define CLOCK_PER_HSYNC		(300000000.0f/60.0f/270.0f)
#define DEBUG_BUTTON_CHECK()	(GV_PadData[1].press & PAD_L1)


#endif
