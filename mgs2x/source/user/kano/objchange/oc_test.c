/*
	oc_test.c
		モデルすり替えのテスト

	1999/12/08 K.Kano
	$Id: oc_test.c,v 1.1.1.3 2002/11/19 11:43:25 Yoshizawa1 Exp $
*/

#if defined(DEBUG) || defined(DEBUG_MODE)
//#define L_DEBUG_MODE
#endif


#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#include <float.h>

#include "gameheader.h"
#include "libutl.h"


#define HUMAN_MODEL_OBJN	21


/* ワーク */
typedef	struct _Work {
    GV_ACT		actor;		/* リンク */
    OBJECT		body;

    DG_OBJS		*objs;

    OBJCHANGE_WORK	*oc;
} Work ;


#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)


/* ------------------------------------------------------------------------ */
/* メイン処理 */

static void Act(Work *work)
{
    DG_OBJS *objs=work->objs;
    int i;

    if(GV_PadData[1].press & PAD_L1){
	ChangeObj(work->oc,HUMAN21_ATAMA,1);
	ChangeObj(work->oc,HUMAN21_KUBI,0); 
    }
    if(GV_PadData[1].press & PAD_R1){
	ChangeObj(work->oc,HUMAN21_ATAMA,-1);
	ChangeObj(work->oc,HUMAN21_KUBI,-1);
    }

    fpu_CopyMatrix(&(work->body.objs->world),&(objs->world));
    work->body.objs->world.m[3][0]+=CVM2N(1.0f);

    for(i=0;i<HUMAN_MODEL_OBJN;i++){
	fpu_CopyMatrix(&(work->body.objs->objs[i].world),&(objs->objs[i].world));
	work->body.objs->objs[i].world.m[3][0]+=CVM2N(1.0f);
    }
}

static void Die(Work *work)
{
    if(work->oc!=NULL){
	GM_FreeObjectForObjChange(&(work->body),work->oc);
    }
}


/* ------------------------------------------------------------------------ */
/* 初期化部 */


/* 初期設定値を取得 */
static void GetOptionValue(int *cname,int *mname,int *mname2)
{
    *cname=GV_StrCode("スネーク");
    *mname=GV_StrCode("usa_debug1");
    *mname2=GV_StrCode("test");

    if(GCL_GetOption('n')!=NULL){
	*cname=GCL_GetNextInt();
    }

    if(GCL_GetOption('m')!=NULL){
	*mname=GCL_GetNextInt();
    }
    if(GCL_GetOption('l')!=NULL){
	*mname2=GCL_GetNextInt();
    }
}


/* 資源を獲得 */
static int GetResources(Work *work)
{
    CONTROL *ctrl;
    OBJECT *object;
    int cname,mname,mname2;

    GetOptionValue(&cname,&mname,&mname2);

    if((work->oc=GM_InitObjectForObjChange(&(work->body),mname,mname2,
					   NULL,OBJECT_FLAG))==NULL) return 0;

    ctrl=GM_SearchWhere(cname);
    if(ctrl==NULL) return 0;

    object=(OBJECT *)(ctrl+1);
    work->objs=object->objs;

    return 1;
}

/* 初期化部メイン */
void *NewOCTest(void)
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
