/*
	motion_animation.h
		モーションに応じた頂点アニメーション

	1999/11/09 K.Kano
	$Id: motion_animation.h,v 1.1.1.3 2002/11/19 11:42:56 Yoshizawa1 Exp $
*/

#ifndef _motion_animation_h_
#define _motion_animation_h_


typedef struct {
    DG_VERTS_ANIME anime;
    CV2_MDL *cvd;
    FVECTOR *v_normal;
    FVECTOR *tmp_work;

    FMATRIX *stock;
    int stockp,stock_size;
    int stock_count;
    int frames;

    int flag;
} MOTION_VANIME_WORK;


typedef struct {
    MOTION_VANIME_WORK **human;
    DG_OBJS *objs;
    int objnum;
    int size;
} HUMANMA_WORK;


/* 初期化関数 */
MOTION_VANIME_WORK *InitMotionVAnimation(DG_OBJ *obj,CV2_MDL *mdl,int flag,int frame);
/* 終了関数 */
void ExitMotionVAnimation(MOTION_VANIME_WORK *work);

/* モーション頂点アニメメイン処理 */
/* 親、子、共にアニメーションする場合 */
void MotionVertexAnimation(MOTION_VANIME_WORK *work,MOTION_VANIME_WORK *pwork,int flag);
/* 親はアニメーションせず、子のみする場合 */
void MotionVertexAnimation2(MOTION_VANIME_WORK *work,FMATRIX *psm,int flag);
/* 子がアニメーションせず、親がする場合 */
void MotionVertexAnimation3(MOTION_VANIME_WORK *work,MOTION_VANIME_WORK *pwork,int flag);

/* アニメーションフレーム数の変更 */
static inline void MAFrameChange(MOTION_VANIME_WORK *work,int newframe)
{
    if(newframe<work->stock_size){
	work->frames=newframe;
    }
}


/* 人間に限定し、ワーク確保やアニメーションを行なう */
HUMANMA_WORK *InitHumanMA(DG_OBJS *objs,CV2_DEF *def,int objnum,int size,int flag,int frame);
HUMANMA_WORK *InitHumanMAForObjchange(OBJECT_CHG *objchg,int objnum,int size,int flag,int frame);
void ExitHumanMA(HUMANMA_WORK *work);
void MoveHumanMA(HUMANMA_WORK *work,int flag);

/* 全身をアニメーションする場合 */
static inline HUMANMA_WORK *InitHumanBodyMA(DG_OBJS *objs,CV2_DEF *def,int flag,int frame)
{
    return InitHumanMA(objs,def,0,objs->n_models,flag,frame);
}

static inline HUMANMA_WORK *InitHumanBodyMAForObjchange(OBJECT_CHG *objchg,int flag,int frame)
{
    return InitHumanMAForObjchange(objchg,0,objchg->objch->objs->n_models,flag,frame);
}

/* 頭と首のみ */
static inline HUMANMA_WORK *InitHumanHeadMA(DG_OBJS *objs,CV2_DEF *def,int flag,int frame)
{
    return InitHumanMA(objs,def,HUMAN21_KUBI,2,flag,frame);
}

static inline HUMANMA_WORK *InitHumanHeadMAForObjchange(OBJECT_CHG *objchg,int flag,int frame)
{
    return InitHumanMAForObjchange(objchg,HUMAN21_KUBI,2,flag,frame);
}

/* 腕、手のみ */
static inline HUMANMA_WORK *InitHumanArmMA(DG_OBJS *objs,CV2_DEF *def,int objnum,int flag,int frame)
{
    return InitHumanMA(objs,def,objnum,3,flag,frame);
}

static inline HUMANMA_WORK *InitHumanArmMAForObjchange(OBJECT_CHG *objchg,int objnum,int flag,int frame)
{
    return InitHumanMAForObjchange(objchg,objnum,3,flag,frame);
}

/* 足のみ */
static inline HUMANMA_WORK *InitHumanLegMA(DG_OBJS *objs,CV2_DEF *def,int objnum,int flag,int frame)
{
    return InitHumanMA(objs,def,objnum,4,flag,frame);
}

static inline HUMANMA_WORK *InitHumanLegMAForObjchange(OBJECT_CHG *objchg,int objnum,
						       int flag,int frame)
{
    return InitHumanMAForObjchange(objchg,objnum,4,flag,frame);
}


static inline void HumanMAFrameChange(HUMANMA_WORK *work,int newframe)
{
    int i;
    for(i=0;i<work->size;i++){
	MAFrameChange(*(work->human+i),newframe);
    }
}

#endif
