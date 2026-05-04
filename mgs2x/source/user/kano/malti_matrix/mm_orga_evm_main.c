//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mm_orga_evm_main.c
		オルガの乳揺れ用のプログラム
		キャラ版

	1999/12/20 K.Kano
	$Id: mm_orga_evm_main.c,v 1.1.1.3 2002/11/19 11:43:15 Yoshizawa1 Exp $
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
    MoveEvmMMOrga(work->mm_orga);
}

static void Die(Work *work)
{
    ExitEvmMMOrga(work->mm_orga);
}


/* 初期化処理 */
static int GetResources(Work *work)
{
    CONTROL *ctrl;
    OBJECT *object=NULL;

    if(GCL_GetOption('y')!=NULL){
		int name=GCL_GetNextInt();
		if((ctrl=GM_SearchWhere(name))==NULL) return 0;
		object=(OBJECT *)(ctrl+1);
    }

    if(object==NULL) return 0;

	switch(object->evmobj->n_skeleton){
	case N_BONE_ORGAWH:
		if((work->mm_orga=InitEvmMMOrgaWithHand(object->evmobj))==NULL) return 0;
		break;
	default:
		if((work->mm_orga=InitEvmMMOrga(object->evmobj))==NULL) return 0;
		break;
	}

    return 1;
}

void *NewEvmMMOrga(void)
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
static int GetResources_called(Work *work,DG_EVMOBJ *evmobj)
{
    if(evmobj==NULL) return 0;

	switch(evmobj->n_skeleton){
	case N_BONE_ORGAWH:
		if((work->mm_orga=InitEvmMMOrgaWithHand(evmobj))==NULL) return 0;
		break;
	default:
		if((work->mm_orga=InitEvmMMOrga(evmobj))==NULL) return 0;
		break;
	}

    return 1;
}

void *NewEvmMMOrga_called(DG_EVMOBJ *evmobj)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof(Work));
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));

		if(!GetResources_called(work,evmobj)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}
