//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ma_test.c
		頂点アニメーション

	1999/11/09 K.Kano
	$Id: ma_test.c,v 1.1.1.3 2002/11/19 11:43:43 Yoshizawa1 Exp $
*/

#if defined(DEBUG) || defined(DEBUG_MODE)
//#define L_DEBUG_MODE
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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


#define HUMAN_MODEL_OBJN	21


/* ワーク */
typedef	struct _Work {
    GV_ACT		actor ;		/* リンク */

    DG_OBJS		*objs;

    MOTION_VANIME_WORK	*ma[HUMAN_MODEL_OBJN];
    HUMANMA_WORK *human_ma;
    HUMANMA_WORK *human_ma2;

    int			flag;
} Work ;


#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)


/* ------------------------------------------------------------------------ */
/* メイン処理 */

static void Act2(Work *work)
{
    int flag;

    if(GV_PadData[1].press & PAD_L1){
	work->flag ^=1;
    }
    flag=work->flag;

#if 0
    MotionVertexAnimation2(work->ma[HUMAN21_MIGI_UDE1],
			   &(work->objs->objs[HUMAN21_MIGI_KATA].world),flag);
    MotionVertexAnimation(work->ma[HUMAN21_MIGI_UDE2],
			  work->ma[HUMAN21_MIGI_UDE1],flag);
    MotionVertexAnimation(work->ma[HUMAN21_MIGI_TE],
			  work->ma[HUMAN21_MIGI_UDE2],flag);

    MotionVertexAnimation2(work->ma[HUMAN21_HIDARI_UDE1],
			   &(work->objs->objs[HUMAN21_HIDARI_KATA].world),flag);
    MotionVertexAnimation(work->ma[HUMAN21_HIDARI_UDE2],
			  work->ma[HUMAN21_HIDARI_UDE1],flag);
    MotionVertexAnimation(work->ma[HUMAN21_HIDARI_TE],
			  work->ma[HUMAN21_HIDARI_UDE2],flag);
#else
    MoveHumanMA(work->human_ma,flag);
    MoveHumanMA(work->human_ma2,flag);
#endif
}

static void Act(Work *work)
{
    // DG_MDL *mdl=&(work->objs->def->models[0]);
    // int i;
    int flag;

    if(GV_PadData[1].press & PAD_L1){
	work->flag ^=1;
    }
    flag=work->flag;

#if 0
    for(i=0;i<HUMAN_MODEL_OBJN;i++,mdl++){
	if(mdl->parent>=0) MotionVertexAnimation(work->ma[i],work->ma[mdl->parent],flag);
	else MotionVertexAnimation(work->ma[i],NULL,flag);
    }
#else
    MoveHumanMA(work->human_ma,flag);
#endif
}

static void Die(Work *work)
{
#if 0
    int i;

    for(i=0;i<HUMAN_MODEL_OBJN;i++){
	if(work->ma[i]) ExitMotionVAnimation(work->ma[i]);
    }
#else
    if(work->human_ma!=NULL) ExitHumanMA(work->human_ma);
    if(work->human_ma2!=NULL) ExitHumanMA(work->human_ma2);
#endif
}


/* ------------------------------------------------------------------------ */
/* 初期化部 */


/* 初期設定値を取得 */
static void GetOptionValue(int *cname,int *mname,int *frames)
{
    *cname=GV_StrCode("スネーク");
    *mname=GV_StrCode("sna_def");
    *frames=6;

    if(GCL_GetOption('n')!=NULL){
	*cname=GCL_GetNextInt();
    }

    if(GCL_GetOption('m')!=NULL){
	*mname=GCL_GetNextInt();
    }

    if(GCL_GetOption('f')!=NULL){
	*frames=GCL_GetNextInt();
    }
}


/* 資源を獲得 */
static int GetResources(Work *work)
{
    CONTROL *ctrl;
    OBJECT *object;
    DG_OBJ *obj;
    CV2_DEF *cvd_def;
    // CV2_MDL *cvd;
    int cname,mname,frames;
    // int i;

    GetOptionValue(&cname,&mname,&frames);

    ctrl=GM_SearchWhere(cname);
    if(ctrl==NULL) return 0;

    object=(OBJECT *)(ctrl+1);

    work->flag=1;
    work->objs=object->objs;
    obj=work->objs->objs;

    cvd_def=(CV2_DEF*)GV_GetCache(GV_CacheID(mname,'c'));
    if(cvd_def==NULL) return 0;

#if 0
    cvd=&(cvd_def->models[0]);

    for(i=0;i<HUMAN_MODEL_OBJN;i++){
	if((work->ma[i]=InitMotionVAnimation(obj+i,cvd+i,DG_VANIME_VERTS,frames))==NULL){
	    return 0;
	}
    }
#else
    if((work->human_ma
	=InitHumanBodyMA(work->objs,cvd_def,DG_VANIME_VERTS,frames))==NULL){

	return 0;
    }
#endif

    return 1;
}

/* 初期化部メイン */
void *NewMATest(void)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),Act,Die) ;
	
	if(!GetResources(work)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}


/* 資源を獲得 */
static int GetResources2(Work *work)
{
    CONTROL *ctrl;
    OBJECT *object;
    DG_OBJ *obj;
    CV2_DEF *cvd_def;
    // CV2_MDL *cvd;
    int cname,mname,frames;

    GetOptionValue(&cname,&mname,&frames);

    ctrl=GM_SearchWhere(cname);
    if(ctrl==NULL) return 0;

    object=(OBJECT *)(ctrl+1);

    work->flag=1;
    work->objs=object->objs;
    obj=work->objs->objs;

    cvd_def=(CV2_DEF*)GV_GetCache(GV_CacheID(mname,'c'));
    if(cvd_def==NULL) return 0;

#if 0
    cvd=&(cvd_def->models[0]);

    if((work->ma[HUMAN21_MIGI_UDE1]=InitMotionVAnimation(obj+HUMAN21_MIGI_UDE1,
							 cvd+HUMAN21_MIGI_UDE1,
							 DG_VANIME_VERTS,frames))==NULL){
	return 0;
    }

    if((work->ma[HUMAN21_MIGI_UDE2]=InitMotionVAnimation(obj+HUMAN21_MIGI_UDE2,
							 cvd+HUMAN21_MIGI_UDE2,
							 DG_VANIME_VERTS,frames))==NULL){
	return 0;
    }

    if((work->ma[HUMAN21_MIGI_TE]=InitMotionVAnimation(obj+HUMAN21_MIGI_TE,
						       cvd+HUMAN21_MIGI_TE,
						       DG_VANIME_VERTS,frames))==NULL){
	return 0;
    }

    if((work->ma[HUMAN21_HIDARI_UDE1]=InitMotionVAnimation(obj+HUMAN21_HIDARI_UDE1,
							   cvd+HUMAN21_HIDARI_UDE1,
							   DG_VANIME_VERTS,frames))==NULL){
	return 0;
    }

    if((work->ma[HUMAN21_HIDARI_UDE2]=InitMotionVAnimation(obj+HUMAN21_HIDARI_UDE2,
							   cvd+HUMAN21_HIDARI_UDE2,
							   DG_VANIME_VERTS,frames))==NULL){
	return 0;
    }

    if((work->ma[HUMAN21_HIDARI_TE]=InitMotionVAnimation(obj+HUMAN21_HIDARI_TE,
							 cvd+HUMAN21_HIDARI_TE,
							 DG_VANIME_VERTS,frames))==NULL){
	return 0;
    }
#else
    if((work->human_ma
	=InitHumanArmMA(work->objs,cvd_def,HUMAN21_MIGI_UDE1,DG_VANIME_VERTS,frames))==NULL){

	return 0;
    }
    if((work->human_ma2
	=InitHumanArmMA(work->objs,cvd_def,HUMAN21_HIDARI_UDE1,DG_VANIME_VERTS,frames))==NULL){

	return 0;
    }
#endif

    return 1;
}

/* 部分エフェクトのテスト */
void *NewMATest2(void)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),Act2,Die) ;
	
	if(!GetResources2(work)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}
