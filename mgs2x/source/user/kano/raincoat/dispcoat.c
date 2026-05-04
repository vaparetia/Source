//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	dispcoat.c
		レインコートのコートの揺れ

	2000/04/05 K.Kano
	$Id: dispcoat.c,v 1.1.1.3 2002/11/19 11:43:32 Yoshizawa1 Exp $
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

#include "dispcoat.h"


void ExitDispCoat(DISP_COAT *work)
{
    if(work->objs!=NULL){
		DG_DequeueObjs(work->objs);
		DG_FreeObjs(work->objs);
    }
    if(work->evm!=NULL){
		DG_DequeueEvmObj(work->evm);
		DG_FreeEvmObj(work->evm);
    }
    GV_Free(work);
}

DISP_COAT *InitDispCoat(int coat_kms,int coat_evm,OBJECT *target)
{
    DISP_COAT *work;
    DG_DEF *kms;
    EVM_DEF *evm;

    if((work=(DISP_COAT *)GV_Malloc(sizeof(DISP_COAT)))==NULL){
		printf( "No memory for InitDispCoat !! :dispcoat.c\n" ) ;
		return NULL;
    }
    GV_ZeroMemory(work,sizeof(DISP_COAT));

    kms=GV_GetCache(GV_CacheID(coat_kms,'k'));
    evm=GV_GetCache(GV_CacheID(coat_evm,'e'));

    if(kms==NULL || evm==NULL){
		GV_Free(work);
		printf( "Cant find %s%x %s%x not in data.cnf !! :dispcoat.c\n",
			(kms?"":"KMS"),coat_kms,
			(evm?"":"EVM"),coat_evm ) ;
		return NULL;
    }

    work->target=target;

    if((work->objs=DG_MakeObjs(kms,DG_FLAG_SHADE|DG_FLAG_FINISHCALC,0))==NULL){
		ExitDispCoat(work);
		printf( "No memory for KMS!! :dispcoat.c\n" ) ;
		return NULL;
    }
    DG_QueueObjs(work->objs);
	DG_SetLightMatrix(work->objs,target->objs->light);
    // work->objs->light=target->objs->light;

    if((work->evm=DG_MakeEvmObj(evm,0,0))==NULL){
		ExitDispCoat(work);
		printf( "No memory for EVM!! :dispcoat.c\n" ) ;
		return NULL;
    }
    DG_QueueEvmObj(work->evm);
    work->evm->light=target->objs->light;

    return work;
}

void MoveDispCoat(DISP_COAT *work)
{
    DG_OBJ *base=work->target->objs->objs;
    DG_OBJ *objs=work->objs->objs;
    DG_MDL *mdl=work->objs->def->models;
    FMATRIX *evmw;
    EVM_SKEL *skel=work->evm->def->skeleton;
    FMATRIX *mat;
    int i;

    if ( work->target->evmobj ){
	if ( work->target->evmobj->flag  & DG_FLAG_INVISIBLE ){
		DG_InvisibleObjs(work->objs);
		work->evm->flag|=DG_EVMOBJ_INVISIBLE;
	}
	else{
		DG_VisibleObjs(work->objs);
		work->evm->flag&=~DG_EVMOBJ_INVISIBLE;
	}
    }
    else {
	if(work->target->objs->flag & DG_FLAG_INVISIBLE){
		DG_InvisibleObjs(work->objs);
		work->evm->flag|=DG_EVMOBJ_INVISIBLE;
	}
	else{
		DG_VisibleObjs(work->objs);
		work->evm->flag&=~DG_EVMOBJ_INVISIBLE;
	}
    }
    if((mat=(FMATRIX *)GV_Malloc(sizeof(FMATRIX)*work->target->objs->def->n_models))==NULL){
		return;
    }

    DG_Arm_SwitchEvmBuffer(work->evm);
    evmw=work->evm->matrix[work->evm->use_buffer];

    fpu_CopyMatrix(&(work->objs->world),&(work->target->objs->world));
    fpu_CopyMatrix(&(work->evm->world),&(work->target->objs->world));

    for(i=0;i<work->target->objs->def->n_models;i++,base++,mdl++,skel++,evmw++){
		FVECTOR vec;
		int parent1=mdl->parent;
		int parent2=skel->parent;


		/* シングルウェイトの計算 */
		fpu_CopyMatrix(&((objs+i)->world),&(base->world));

		if(parent1!=-1){
			vec.vx=mdl->tx;
			vec.vy=mdl->ty;
			vec.vz=mdl->tz;
			vec.vw=1.0f;
			_sceVu0ApplyMatrix(&vec,&((objs+parent1)->world),&vec);
			(objs+i)->world.m[3][0]=vec.vx;
			(objs+i)->world.m[3][1]=vec.vy;
			(objs+i)->world.m[3][2]=vec.vz;
		}


		/* マルチウェイトの計算 */
		fpu_CopyMatrix(mat+i,&(base->world));
		fpu_CopyMatrix(evmw,&(base->world));

		if(parent2!=-1){
			vec.vx=skel->tx;
			vec.vy=skel->ty;
			vec.vz=skel->tz;
			vec.vw=1.0f;
			_sceVu0ApplyMatrix(&vec,mat+parent2,&vec);
			(mat+i)->m[3][0]=vec.vx;
			(mat+i)->m[3][1]=vec.vy;
			(mat+i)->m[3][2]=vec.vz;
		}

		vec.vx=-skel->rt_tx;
		vec.vy=-skel->rt_ty;
		vec.vz=-skel->rt_tz;
		vec.vw=1.0f;
		_sceVu0ApplyMatrix(&vec,mat+i,&vec);
		evmw->m[3][0]=vec.vx;
		evmw->m[3][1]=vec.vy;
		evmw->m[3][2]=vec.vz;

#if 0
		printf("%d : %d %d %d\n",i,
			   (int)((mat+i)->m[3][0]),(int)((mat+i)->m[3][1]),(int)((mat+i)->m[3][2]));
		printf("%d : %d %d %d\n",i,(int)(vec.vx),(int)(vec.vy),(int)(vec.vz));
#endif

    }

    GV_Free(mat);
}
