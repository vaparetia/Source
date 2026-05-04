//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	objchange.c
		モデルの部分変更

	1999/12/07 K.Kano
	$Id: objchange.c,v 1.1.1.3 2002/11/19 11:42:56 Yoshizawa1 Exp $
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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


/* モデルの一部をすり替えるモデルのためのInitObject */
OBJCHANGE_WORK *GM_InitObjectForObjChange(OBJECT *object,int model,int parts_model,
					  int *mdl_list,int flag)
{
    OBJCHANGE_WORK *work;
    DG_DEF *def,*defp ;
    CV2_DEF *def_cv2,*defp_cv2;
    DG_DEFMDLPAIR *mdldef_list;
    int i;

    /*
      model == 0 は禁止
      */
    ASSERT( model != 0 ) ;

    def = (DG_DEF*) GV_GetCache( GV_CacheID( model, 'k' ) );
    defp = (DG_DEF*) GV_GetCache( GV_CacheID( parts_model, 'k' ) );
    def_cv2 = (CV2_DEF*) GV_GetCache( GV_CacheID( model, 'c' ) );
    defp_cv2 = (CV2_DEF*) GV_GetCache( GV_CacheID( parts_model, 'c' ) );

    if ( def==NULL || defp==NULL)
    {
	if ( def==NULL )
	    printf( "No Original-Model(%d) : GM_InitObjectForObjChange\n", model ) ;
	if ( defp==NULL)
	    printf( "No Parts-Model(%d) : GM_InitObjectForObjChange\n", parts_model ) ;
	return NULL;
    }

    if(def_cv2!=NULL && defp_cv2!=NULL){
	if((work=(OBJCHANGE_WORK *)GV_Malloc(sizeof(OBJCHANGE_WORK)
					     +sizeof(DG_DEF)+sizeof(DG_MDL)*def->n_models
					     +sizeof(CV2_DEF)+sizeof(CV2_MDL)*def->n_models
					     +sizeof(short)*def->n_models))==NULL){
	    printf( "No memory : GM_InitObjectForObjChange\n" ) ;

	    return NULL;
	}

	work->nowdef=(DG_DEF *)(work+1);
	work->nowcv2=(CV2_DEF *)((int)(work->nowdef)
				 +sizeof(DG_DEF)+sizeof(DG_MDL)*def->n_models);
	work->mdl_list_index
	    =(short *)((int)(work->nowcv2)
		       +sizeof(CV2_DEF)+sizeof(CV2_MDL)*def->n_models);

	memcpy(work->nowdef,def,sizeof(DG_DEF));
	memcpy(work->nowcv2,def_cv2,sizeof(CV2_DEF));
    }
    else{
	if((work=(OBJCHANGE_WORK *)GV_Malloc(sizeof(OBJCHANGE_WORK)
					     +sizeof(DG_DEF)+sizeof(DG_MDL)*def->n_models
					     +sizeof(short)*def->n_models))
	   ==NULL){
	    printf( "No memory : GM_InitObjectForObjChange\n" ) ;

	    return NULL;
	}

	work->nowdef=(DG_DEF *)(work+1);
	work->nowcv2=NULL;
	work->mdl_list_index
	    =(short *)((int)(work->nowdef)
		       +sizeof(DG_DEF)+sizeof(DG_MDL)*def->n_models);

	memcpy(work->nowdef,def,sizeof(DG_DEF));
    }

    if((mdldef_list=(DG_DEFMDLPAIR *)GV_Malloc(sizeof(DG_DEFMDLPAIR)*def->n_models))==NULL){
	GV_Free(work);
	printf( "No memory : GM_InitObjectForObjChange\n" ) ;
	return NULL;
    }

    for(i=0;i<def->n_models;i++){
	if(mdl_list!=NULL){
	    if(*(mdl_list+i)>=0){
		   memcpy(&(work->nowdef->models[i]),
		       &(defp->models[*(mdl_list+i)]),
		       sizeof(DG_MDL));
         mdldef_list[i].def = defp;
         mdldef_list[i].mdl = &(defp->models[*(mdl_list+i)]);
	    }
	    else{
		   memcpy(&(work->nowdef->models[i]),
		          &(def->models[i]),
		          sizeof(DG_MDL));
         mdldef_list[i].def = def;
         mdldef_list[i].mdl = &(def->models[i]);
	    }
	    *(work->mdl_list_index+i)=*(mdl_list+i);
	}
	else{
	    memcpy(&(work->nowdef->models[i]),
		   &(def->models[i]),
		   sizeof(DG_MDL));
       mdldef_list[i].def = def;
       mdldef_list[i].mdl = &(def->models[i]);
	    *(work->mdl_list_index+i)=-1;
	}
    }
    if(work->nowcv2!=NULL){
	for(i=0;i<def->n_models;i++){
	    if(mdl_list!=NULL){
		if(*(mdl_list+i)>=0){
		    memcpy(&(work->nowcv2->models[i]),
			   &(defp_cv2->models[*(mdl_list+i)]),
			   sizeof(CV2_MDL));
		}
		else{
		    memcpy(&(work->nowcv2->models[i]),
			   &(def_cv2->models[i]),
			   sizeof(CV2_MDL));
		}
	    }
	    else{
		memcpy(&(work->nowcv2->models[i]),
		       &(def_cv2->models[i]),
		       sizeof(CV2_MDL));
	    }
	}
    }

    if((object->objs=DG_MakeObjs2(def,flag,0,mdldef_list,def->n_models))==NULL){
	   printf( "Can't make DG_OBJS : GM_InitObjectForObjChange\n" ) ;

	   GV_Free(work);
	   GV_Free(mdldef_list);
	   return NULL;
    }
    DG_QueueObjs( object->objs );
    object->flag = 0 ;

    work->objs=object->objs;
    work->def=def;
    work->parts=defp;
    work->def_cv2=def_cv2;
    work->parts_cv2=defp_cv2;

    GV_Free(mdldef_list);

    return work;
}

OBJCHANGE_WORK *GM_InitObject2ForObjChange(OBJECT *object,DG_DEF *def,int flag)
{
    OBJCHANGE_WORK *work;
    DG_DEFMDLPAIR *mdldef_list;
    int i;


    if ( def==NULL )
    {
	printf( "DG_DEF is NULL : GM_InitObject2ForObjChange\n" ) ;
	return NULL;
    }

    if((work=(OBJCHANGE_WORK *)GV_Malloc(sizeof(OBJCHANGE_WORK)
					 +sizeof(DG_DEF)+sizeof(DG_MDL)*def->n_models))
       ==NULL){
	printf( "No memory : GM_InitObjectForObjChange\n" ) ;

	return NULL;
    }

    work->nowdef=(DG_DEF *)(work+1);
    work->nowcv2=NULL;

    if((mdldef_list=(DG_DEFMDLPAIR *)GV_Malloc(sizeof(DG_DEFMDLPAIR)*def->n_models))==NULL){
	   printf( "No memory : GM_InitObjectForObjChange\n" ) ;
	   GV_Free(work);
	   return NULL;
    }

    memcpy(work->nowdef,def,sizeof(DG_DEF));
    for(i=0;i<def->n_models;i++){
	   memcpy(&(work->nowdef->models[i]),&(def->models[i]),sizeof(DG_DEF));
      mdldef_list[i].def = NULL;
      mdldef_list[i].mdl = NULL;
    }

    if((object->objs=DG_MakeObjs2(def,flag,0,mdldef_list,def->n_models))==NULL){
	   printf( "No memory : GM_InitObjectForObjChange\n" ) ;
	   GV_Free(work);
	   GV_Free(mdldef_list);
	   return NULL;
    }
    DG_QueueObjs( object->objs );
    object->flag = 0 ;

    work->objs=object->objs;
    work->def=def;
    work->parts=NULL;
    work->def_cv2=NULL;
    work->parts_cv2=NULL;

    GV_Free(mdldef_list);

    return work;
}

/* モデルの一部をすり替えるモデルのためのFreeObject */
void GM_FreeObjectForObjChange(OBJECT *object,OBJCHANGE_WORK *work)
{
    if(object->m_ctrl!=NULL){
	MT_FreeMotion(object->m_ctrl);
	object->m_ctrl=NULL;
    }
    if(object->objs!=NULL){
	DG_DequeueObjs(object->objs) ;
	DG_FreeObjs(object->objs);
	object->objs=NULL ;
    }
    if( work != NULL ){
        GV_Free(work);
    }
}

#if 0 //BP_UNUSED
inline void ChangeObj(OBJCHANGE_WORK *work,int objnum,int partsnum)
{
    DG_MDL *mdl=NULL;
    CV2_MDL *cv2mdl=NULL;

    if(partsnum>=0){
	mdl=&(work->parts->models[partsnum]);
	if(work->parts_cv2!=NULL) cv2mdl=&(work->parts_cv2->models[partsnum]);
    }
    else{
	mdl=&(work->def->models[objnum]);
	if(work->def_cv2!=NULL) cv2mdl=&(work->def_cv2->models[objnum]);
    }

    if(partsnum!=*(work->mdl_list_index+objnum)){
	DG_ChangeModelObj(&(work->objs->objs[objnum]),mdl);
	memcpy(&(work->nowdef->models[objnum]),mdl,sizeof(DG_MDL));
	if(work->nowcv2!=NULL) memcpy(&(work->nowcv2->models[objnum]),cv2mdl,sizeof(CV2_MDL));
	*(work->mdl_list_index+objnum)=partsnum;
    }
}

void ChangeObjs(OBJCHANGE_WORK *work,int *mdl_list)
{
    int i;
    for(i=0;i<work->def->n_models;i++){
	if(*(mdl_list+i)!=-2) ChangeObj(work,i,*(mdl_list+i));
    }
}

inline void ChangeObjByMDL(OBJCHANGE_WORK *work,int objnum,DG_MDL *mdl)
{
    DG_ChangeModelObj(&(work->objs->objs[objnum]),mdl);
    memcpy(&(work->nowdef->models[objnum]),mdl,sizeof(DG_MDL));
}

void ChangeObjsByMDL(OBJCHANGE_WORK *work,DG_MDL **mdl_list)
{
    int i;
    for(i=0;i<work->def->n_models;i++){
	ChangeObjByMDL(work,i,*(mdl_list+i));
    }
}
#endif
