//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	partchg.c
		人間のモデルの一部すり替え
		＋頂点アニメーション

	2000/03/28 K.Kano
	$Id: partchg.c,v 1.1.1.3 2002/11/19 11:42:57 Yoshizawa1 Exp $
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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

#include "partchg.h"


#define OBJECT_FLAG	(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)


/* 一部を頂点アニメさせるために、モデルの一部切替えと頂点アニメを行なう。
   objchangeは、全身をダイナミックに切替えるが、これはあくまで一部を切替える。
   この時、切替えるモデルとオブジェの数が合わなくても構わない。

   親   -> 切替え元
    :        :
    :        :
   obj0 -> obj1 -> obj2 ...

   という関係になるように、モデルをつくってもらう。
 */

PARTCHANGE_WORK *InitPartChange(DG_OBJS *human,int index,DG_DEF *def,
								CV2_DEF **cv2def,int n_patterns)
{
    PARTCHANGE_WORK *work;
    int i,j;

    if((work=(PARTCHANGE_WORK *)GV_Malloc(sizeof(PARTCHANGE_WORK)))==NULL) return NULL;

    work->human=human;
    work->index=index;
    work->n_patterns=n_patterns;

    if((work->part=DG_MakeObjs(def,OBJECT_FLAG,0))==NULL){
		GV_Free(work);
		return NULL;
    }
    DG_QueueObjs(work->part);

    if((work->va=(VERTEX_ANIME_WORK **)GV_Malloc(sizeof(VERTEX_ANIME_WORK *)*(def->n_x_models-1)))
       ==NULL){

		DG_DequeueObjs(work->part);
		return NULL;
    }

    for(i=1;i<def->n_x_models;i++){
		work->va[i-1]
			=InitVertexAnimation(&(work->part->objs[i]),
								 &(cv2def[0]->models[i]),
								 DG_VANIME_VERTS|DG_VANIME_NORMS,n_patterns);
		if(work->va[i-1]==NULL){
			while(i>1){
				i--;
				ExitVertexAnimation(work->va[i-1]);
			}
			DG_DequeueObjs(work->part);
			GV_Free(work);
			return NULL;
		}
    }

    for(i=1;i<def->n_x_models;i++){
		for(j=0;j<n_patterns;j++){
			work->va[i-1]->key[j]=&(cv2def[j]->models[i]);
			work->va[i-1]->p[j]=0.0f;
		}
		work->va[i-1]->p[0]=1.0f;
		work->va[i-1]->count=0;
    }

    human->objs[index].flag|=DG_FLAG_INVISIBLE;
    work->part->objs[0].flag|=DG_FLAG_INVISIBLE;
	DG_SetLightMatrix(work->part,human->light);
    work->disp_enable=1;

	/* 影を反映させるためにオブジェクトを接続しておく 2000.10.23 added by K.Takabe */
	DG_ConnectObjs( work->human, work->part );

    return work;
}

void ExitPartChange(PARTCHANGE_WORK *work)
{
    DG_DEF *def=work->part->def;
    int i;

    for(i=1;i<def->n_x_models;i++){
		ExitVertexAnimation(work->va[i-1]);
    }
    work->human->objs[work->index].flag&=~DG_FLAG_INVISIBLE;

	/* 接続したオブジェクトを切り離す 2000.10.23 added by K.Takabe */
	DG_DisconnectObjs( work->human, work->part );

    DG_DequeueObjs(work->part);
    GV_Free(work);
}

void MovePartChange(PARTCHANGE_WORK *work)
{
    DG_DEF *def=work->part->def;
    DG_MDL *mdl=def->models;
    int n_models=def->n_models;
    int n_x_models=def->n_x_models;
    int index=work->index;
    int parent=work->human->def->models[index].parent;
    int i;

    /* 表示／非表示設定 */
    if(work->disp_enable){
		work->part->flag&=~DG_FLAG_INVISIBLE;
		work->human->objs[index].flag|=DG_FLAG_INVISIBLE;
    }
    else{
		work->part->flag|=DG_FLAG_INVISIBLE;
		work->human->objs[index].flag&=~DG_FLAG_INVISIBLE;
    }

    /* 親側の表示／非表示を反映 */
    if(work->human->flag & DG_FLAG_INVISIBLE){
		work->part->flag|=DG_FLAG_INVISIBLE;
    }

    /* 頂点アニメーション */
    for(i=1;i<n_x_models;i++){
		SimpleVertexAnimation(work->va[i-1]);
    }

    /* 行列の設定 */
    fpu_CopyMatrix(&(work->part->objs[1].world),&(work->human->objs[index].world));
    if(parent==-1){
		fpu_CopyMatrix(&(work->part->objs[0].world),&(work->human->world));
    }
    else{
		fpu_CopyMatrix(&(work->part->objs[0].world),&(work->human->objs[parent].world));
    }
    fpu_CopyMatrix(&(work->part->world),&(work->part->objs[0].world));

    for(i=2;i<n_models;i++,mdl++){
		FVECTOR vec;

		parent=mdl->parent;

		vec.vx=mdl->tx;
		vec.vy=mdl->ty;
		vec.vz=mdl->tz;
		vec.vw=1.0f;
		if(parent!=-1){
			vu0_Ldv0(&vec);
			vu0_Ldm0(&(work->part->objs[parent].world));
			vu0_Mulv0m0v0();
			vu0_Stm0(&(work->part->objs[i].world));
			vu0_Stv0((FVECTOR *)&(work->part->objs[i].world.m[3][0]));
		}
		else{
			fpu_CopyMatrix(&(work->part->objs[i].world),&(work->part->world));
		}
    }
}

void SetParamForPartChange(PARTCHANGE_WORK *work,int pattern_num,int count)
{
    int n_x_models=work->part->def->n_x_models;
    int n_patterns=work->n_patterns;
    int i,j;

    if(n_patterns<=pattern_num || pattern_num<0) return;

    if(count<0) count=0;

    for(i=1;i<n_x_models;i++){
		for(j=0;j<n_patterns;j++){
			work->va[i-1]->p[j]=0.0f;
		}
		work->va[i-1]->p[pattern_num]=1.0f;
		work->va[i-1]->count=count;
    }
}
