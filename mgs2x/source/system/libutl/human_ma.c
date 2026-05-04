//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
	human_ma.c
		モーション頂点アニメーションを用いて、
		人間キャラにエフェクトをかける。

	1999/12/08 K.Kano
	$Id: human_ma.c,v 1.3 2002/12/31 08:00:11 takaki Exp $
*/
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#ifndef KP_XBOX
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


HUMANMA_WORK *InitHumanMA(DG_OBJS *objs,CV2_DEF *def,int objnum,int size,int flag,int frame)
{
    HUMANMA_WORK *work;
    int start=objnum;
    int i,j;

    if((work=(HUMANMA_WORK *)GV_Malloc(sizeof(HUMANMA_WORK)+sizeof(MOTION_VANIME_WORK *)
									   *size))==NULL){

#ifdef DEBUG
		printf("Human MA : Memory Overflow\n");
		ASSERT(0);
#endif

    }
    work->human=(MOTION_VANIME_WORK **)(work+1);

    for(i=0,j=start;i<size;i++,j++){
		*(work->human+i)=InitMotionVAnimation(objs->objs+j,def->models+j,flag,frame);

#ifdef KP_WINDOWS
		/*-- 専用VertexBuffer作成 --*/
#if __DG_HUMANMA_USE_PRIVATEVBUFF__
		{
			DG_OBJ	*obj ;
			obj = objs->objs+j;

			DG_CreateDGObjPrivateVertexBuffer(obj, obj->n_indices, obj->n_verts) ;
			DG_AssignDGObjIndexBuffer(obj) ;
			DG_AssignDGObjVertexBuffer(obj) ;
			if( obj->d3d_cvbuff ){ DG_AssignDGObjCVertexBuffer(obj) ; }
		}
#endif
#endif

#ifdef DEBUG
		if(*(work->human+i)==NULL){
			printf("Human MA : Memory Overflow\n");
			ASSERT(0);
		}
#endif

    }

    work->objs=objs;
    work->objnum=objnum;
    work->size=size;

    return work;
}

HUMANMA_WORK *InitHumanMAForObjchange(OBJECT_CHG *objchg,int objnum,int size,int flag,int frame)
{
    HUMANMA_WORK *work;
    int start=objnum;
    int i,j;

    if((work=(HUMANMA_WORK *)GV_Malloc(sizeof(HUMANMA_WORK)+sizeof(MOTION_VANIME_WORK *)
									   *size))==NULL){

#ifdef DEBUG
		printf("Human MA : Memory Overflow\n");
		ASSERT(0);
#endif

    }
    work->human=(MOTION_VANIME_WORK **)(work+1);

    for(i=0,j=start;i<size;i++,j++){
		DG_OBJ *obj;
		CV2_MDL *mdl;

		obj=objchg->objch->objs->objs+j;
		mdl=objchg->objch->nowcv2->models+j;

		*(work->human+i)=InitMotionVAnimation(obj,mdl,flag,frame);

#ifdef KP_WINDOWS
		/*-- 専用VertexBuffer作成 --*/
#if __DG_HUMANMA_USE_PRIVATEVBUFF__
		DG_CreateDGObjPrivateVertexBuffer(obj, obj->n_indices, obj->n_verts) ;
		DG_AssignDGObjIndexBuffer(obj) ;
		DG_AssignDGObjVertexBuffer(obj) ;
		if( obj->d3d_cvbuff ){ DG_AssignDGObjCVertexBuffer(obj) ; }
#endif
#endif

#ifdef DEBUG
		if(*(work->human+i)==NULL){
			printf("Human MA : Memory Overflow\n");
			ASSERT(0);
		}
#endif

    }

    work->objs=objchg->objch->objs;
    work->objnum=objnum;
    work->size=size;

    return work;
}

void ExitHumanMA(HUMANMA_WORK *work)
{
#ifndef KP_WINDOWS
    int i;

    for(i=0;i<work->size;i++){
		ExitMotionVAnimation(*(work->human+i));

    }
#else
#if __DG_HUMANMA_USE_PRIVATEVBUFF__
    int 	i;
	DG_OBJ	*obj;

	obj = work->objs->objs + work->objnum;
    for(i=0;i<work->size;i++,obj++){
		DG_ReleaseDGObjPrivateVertexBuffer(obj) ;	// 専用VertexBuffer解放

		ExitMotionVAnimation(*(work->human+i));
    }
#else
    int i;

    for(i=0;i<work->size;i++){
		ExitMotionVAnimation(*(work->human+i));

    }
#endif
#endif
    GV_Free(work);
}

void MoveHumanMA(HUMANMA_WORK *work,int flag)
{
    DG_OBJ *obj;
    MOTION_VANIME_WORK *s,*p;
    int i=0;
    int parent;

    if(work->size>=21){
		/* 全身 */
		s=*(work->human+i);
		obj=s->anime.obj;
		parent=obj->model->parent;

		if(parent>=0){
			DG_OBJ *pobj=&(work->objs->objs[parent]);
			MotionVertexAnimation2(s,&(pobj->world),flag);
		}
		else{
			MotionVertexAnimation(s,NULL,flag);
		}
		i++;

		for( ;i<work->size;i++){
			s=*(work->human+i);
			obj=s->anime.obj;
			parent=obj->model->parent;

			if(parent>=0) p=*(work->human+parent);
			else p=NULL;
			MotionVertexAnimation(s,p,flag);
		}
    }
    else{
		/* 一部のみ */
		s=*(work->human+i);
		obj=s->anime.obj;
		parent=obj->model->parent;

		if(parent>=0){
			DG_OBJ *pobj=&(work->objs->objs[parent]);
			MotionVertexAnimation2(s,&(pobj->world),flag);
		}
		else{
			MotionVertexAnimation(s,NULL,flag);
		}
		i++;

		p=s;
		for( ;i<work->size;i++){
			s=*(work->human+i);
			MotionVertexAnimation(s,p,flag);
			p=s;
		}
    }
}
