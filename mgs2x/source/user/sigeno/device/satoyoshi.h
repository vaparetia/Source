/********************************************************************************/
/*										*/
/*	里吉汎用ヘッダファイル		<satoyoshi.h>				*/
/*	主にステップ管理用							*/
/*					Hiroki Satoyoshi 2001/1/25		*/
/*	$Id: satoyoshi.h,v 1.1.1.3 2002/11/19 11:49:29 Yoshizawa1 Exp $		*/
/*										*/
/********************************************************************************/
#include "camera.h"

#ifndef __def_inc_satoyoshi_h__
#define __def_inc_satoyoshi_h__

/********************************************************************************/
/*	define      								*/
/********************************************************************************/

#define PERROR(...) { printf( __VA_ARGS__ ) ; return -1 ; }


static inline void INIT_ALL_STEP(Work *p){
    p->G1_step = 0;
    p->G2_step = 0;
    p->G3_step = 0;
    p->G4_step = 0;
}

static inline void INIT_G1_STEP(Work *p){
    p->G1_step = 0;
    p->G2_step = 0;
    p->G3_step = 0;
    p->G4_step = 0;
}

static inline void INIT_G2_STEP(Work *p){
    p->G2_step = 0;
    p->G3_step = 0;
    p->G4_step = 0;
}

static inline void NIT_G3_STEP(Work *p){
    p->G3_step = 0;
    p->G4_step = 0;
}

static inline void INIT_G4_STEP(Work *p){
    p->G4_step = 0;
}

static inline void G1_STEP_UP(Work *p){
    p->G1_step ++;
    p->G2_step = 0;
    p->G3_step = 0;
    p->G4_step = 0;
}

static inline void G2_STEP_UP(Work *p){
    p->G2_step ++;
    p->G3_step = 0;
    p->G4_step = 0;
}

static inline void G3_STEP_UP(Work *p){
    p->G3_step ++;
    p->G4_step = 0;
}

static inline void G4_STEP_UP(Work *p){
    p->G4_step ++;
}

static inline void SET_G1_STEP(Work *p, short num){
    p->G1_step = num;
    p->G2_step = 0;
    p->G3_step = 0;
    p->G4_step = 0;
}

static inline void SET_G2_STEP(Work *p, short num){
    p->G2_step = num;
    p->G3_step = 0;
    p->G4_step = 0;
}

static inline void SET_G3_STEP(Work *p, short num){
    p->G3_step = num;
    p->G4_step = 0;
}

static inline void SET_G4_STEP(Work *p, short num){
    p->G4_step = num;
}

#endif



