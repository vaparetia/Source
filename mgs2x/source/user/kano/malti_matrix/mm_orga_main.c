//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mm_orga_main.c
		オルガの乳揺れ用のプログラム
		キャラ版

	1999/12/20 K.Kano
	$Id: mm_orga_main.c,v 1.1.1.3 2002/11/19 11:43:15 Yoshizawa1 Exp $
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


typedef struct {
    GV_ACT_EX actor;
    void *mm_orga;
} Work;


static void Act(Work *work)
{
    MoveMMOrga(work->mm_orga);
}

static void Die(Work *work)
{
    ExitMMOrga(work->mm_orga);
}


/* 初期化処理 */
static int GetResources(Work *work)
{
    CONTROL *ctrl;
    OBJECT *object=NULL;
    CV2_DEF *cv2=NULL;

    if(GCL_GetOption('y')!=NULL){
	int name=GCL_GetNextInt();
	if((ctrl=GM_SearchWhere(name))==NULL) return 0;
	object=(OBJECT *)(ctrl+1);
    }

    if(GCL_GetOption('m')!=NULL){
	int name=GCL_GetNextInt();
	cv2=(CV2_DEF *)GV_GetCache(GV_CacheID(name,'c'));
    }

    if(object==NULL || cv2==NULL) return 0;

    if((work->mm_orga=InitMMOrga(object->objs,cv2))==NULL) return 0;

    return 1;
}

void *NewMMOrga(void)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof(Work));
    if(work!=NULL) {
	GV_SetActor(&(work->actor),Act,Die) ;
	GV_ActorEX(&(work->actor));

	if(!GetResources(work)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}

/* 初期化処理 */
static int GetResources_called(Work *work,OBJECT *object,int cv2_name)
{
    CV2_DEF *cv2;

    cv2=(CV2_DEF *)GV_GetCache(GV_CacheID(cv2_name,'c'));

    if(object==NULL || cv2==NULL) return 0;

    if((work->mm_orga=InitMMOrga(object->objs,cv2))==NULL) return 0;

    return 1;
}

void *NewMMOrga_called(OBJECT *object,int cv2_name)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof(Work));
    if(work!=NULL) {
	GV_SetActor(&(work->actor),Act,Die) ;
	GV_ActorEX(&(work->actor));

	if(!GetResources_called(work,object,cv2_name)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}
