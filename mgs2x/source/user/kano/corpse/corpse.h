/*
	corpse.h
		死体の姿勢制御

	1999/10/5 K.Kano
	$Id: corpse.h,v 1.1.1.3 2002/11/19 11:43:09 Yoshizawa1 Exp $
*/


#ifndef _corpse_h_
#define _corpse_h_

#include <stdio.h>
#include <stdlib.h>
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
#include "utl_dma.h"


#define HUMAN_MODEL_OBJN	21


#define WHEST_LIMIT	0.965925826f

#define HEEL_HEIGHT	CVC2N(17.0f)
#define HEEL_COLLISION	((int)CVC2N(17.0f))
#define NEE_HEIGHT	CVC2N(15.0f)
#define NEE_COLLISION	((int)CVC2N(30.0f))


#define HEAD_HEIGHT	CVC2N(15.0f)
#define HEAD_COLLISION	((int)CVC2N(15.0f))
#define CHEST_HEIGHT	CVC2N(15.0f)
#define CHEST_COLLISION	((int)CVC2N(30.0f))
#define ARM_HEIGHT	CVC2N(12.0f)
#define ARM_COLLISION	((int)CVC2N(15.0f))
#define HAND_HEIGHT	CVC2N(7.0f)
#define HAND_COLLISION	((int)CVC2N(10.0f))


typedef struct {

    /* 前回の各OBJのワールド座標 */
    FVECTOR x[HUMAN_MODEL_OBJN];

    /* 前回の各OBJの速度 */
    FVECTOR v[HUMAN_MODEL_OBJN];

    /* モーションが作り出した、補正前のabs_rots */
    FVECTOR mtn_abs_rots[HUMAN_MODEL_OBJN];

    /* 前回のabs_rots */
    FVECTOR abs_rots[HUMAN_MODEL_OBJN];

    /* free_corpseで用いる、前回の補正abs_rots */
    FVECTOR diff_rots[HUMAN_MODEL_OBJN];

    /* 各OBJの基準ワールド座標(AfterJumpCorpse) */
    FVECTOR basex[HUMAN_MODEL_OBJN];

    /* 腰のabs_rotsを基準にするため、その値を保存する */
    FVECTOR main_abs_rots;

    /* カウンター、
       FreeCorpse -> HoldCorpseで使用 */
    int count;
} CORPSE_PARAMETER;


typedef struct _Work {
    /* 相手のコントロール */
    CONTROL *control;

    /* 相手のオブジェクト */
    OBJECT *obj;

    /* 死体用のパラメータ */
    CORPSE_PARAMETER corpsep;

} Work;


struct _scratchpad {
    FMATRIX rmat;
    FMATRIX m_rmat;

    FVECTOR root,inv_root;
    FVECTOR m_root,inv_m_root;
    FVECTOR conv_root;

    FMATRIX objmat[HUMAN_MODEL_OBJN];
    FVECTOR x[HUMAN_MODEL_OBJN];
    FVECTOR v[HUMAN_MODEL_OBJN];

    FVECTOR abs_rots[HUMAN_MODEL_OBJN];

    /* 壁判定に用いるワーク */
    FMATRIX kmat[HUMAN_MODEL_OBJN];

    /* 相手のハザードID */
    HZX_GROUP_ID hzx_id;
};


/* pull_corpse.c */
void Corpse_WorldCalc(FMATRIX *s,FMATRIX *root,OBJECT *obj,FVECTOR *abs_rots);
void AdjustJoint(int objnum,int param0,int param1,int mode,int flag,
		 struct _scratchpad *scratchpad,Work *work);
void AdjustShoulder(int objnum,int flag,struct _scratchpad *scratchpad,Work *work);
void AdjustChest(int param0,int param1,int mode,int flag,
		 struct _scratchpad *scratchpad,Work *work);
void AdjustHip(int param0,int param1,int mode,int flag,
	       struct _scratchpad *scratchpad,Work *work);

void PullCorpseWithShoulder(Work *work);
void PullCorpseWithLegs(Work *work);
void BackFromFreeCorpse(Work *work);


#endif
