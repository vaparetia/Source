/*
	malti_test.c
		

	1999/12/20 K.Kano
	$Id: malti_test.c,v 1.1.1.3 2002/11/19 11:43:15 Yoshizawa1 Exp $
*/

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

#include "mm_orga.h"


/* ワーク */
typedef	struct _Work {
    GV_ACT		actor ;		/* リンク */

    DG_OBJS		*objs;

    void		*twork;
} Work ;


#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)


/* ------------------------------------------------------------------------ */
/* メイン処理 */

static void Act(Work *work)
{
    MoveMMOrga(work->twork);
}

static void Die(Work *work)
{
    if(work->twork!=NULL){
	ExitMMOrga(work->twork);
    }
}


/* ------------------------------------------------------------------------ */
/* 初期化部 */


/* 初期設定値を取得 */
static void GetOptionValue(int *cname,int *mname)
{
    *cname=GV_StrCode("スネーク");
    *mname=GV_StrCode("org_tes_muneyure");

    if(GCL_GetOption('n')!=NULL){
	*cname=GCL_GetNextInt();
    }

    if(GCL_GetOption('m')!=NULL){
	*mname=GCL_GetNextInt();
	printf("GetModelName ;;;;\n");
    }
    else{
	printf("NoModelName ----\n");
    }
}


/* 資源を獲得 */
static int GetResources(Work *work)
{
    CONTROL *ctrl;
    OBJECT *object;
    DG_OBJS *objs;
    CV2_DEF *cvd_def;
    int cname,mname;

    GetOptionValue(&cname,&mname);

    ctrl=GM_SearchWhere(cname);
    if(ctrl==NULL) return 0;

    object=(OBJECT *)(ctrl+1);

    objs=work->objs=object->objs;

    cvd_def=(CV2_DEF*)GV_GetCache(GV_CacheID(mname,'c'));
    if(cvd_def==NULL) return 0;

    if((work->twork=InitMMOrga(objs,cvd_def))==NULL){
	return 0;
    }

    return 1;
}

/* 初期化部メイン */
void *NewMMOrga(void)
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
