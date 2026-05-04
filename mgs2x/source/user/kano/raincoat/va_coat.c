//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	va_coat.c
		レインコートのコートの揺れ

	1999/12/20 K.Kano
	$Id: va_coat.c,v 1.1.1.3 2002/11/19 11:43:33 Yoshizawa1 Exp $
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


typedef struct {
    VERTEX_ANIME_WORK **va;
    DG_OBJS *objs;
    int va_size;
    int cv2def_size;

    /* 振幅と揺れの速度 */
    float width,speed,yure;

    float wind_min,wind_max;
    float speed_min,speed_max;

    int index;
} VACOAT_WORK;


#define WINDMIN		50.0f
#define WINDMAX		250.0f
#if 0
#define W_SPEED_MIN	(2.0f*M_PI/6.0f)
#define W_SPEED_MAX	(2.0f*M_PI/6.0f)
#else
#define W_SPEED_MIN	(1.0f/3.0f)
#define W_SPEED_MAX	(1.0f/1.5f)
#endif


void ExitVACoat(VACOAT_WORK *work)
{
    int i;
    if(work->va!=NULL){
		for(i=0;i<work->va_size;i++){
			if(work->va[i]!=NULL) ExitVertexAnimation(work->va[i]);
		}
		GV_Free(work->va);
    }
    GV_Free(work);
}

VACOAT_WORK *InitVACoat(DG_OBJS *objs,CV2_DEF **def,int cv2def_size,
						float wind_min,float wind_max,float speed_min,float speed_max)
{
    VACOAT_WORK *work;
    int i,j;


    if((work=(VACOAT_WORK *)GV_Malloc(sizeof(VACOAT_WORK)))==NULL){
#ifdef DEBUG
		printf("Memory Overflow 0\n");
#endif
		return NULL;
    }
    GV_ZeroMemory(work,sizeof(VACOAT_WORK));

    work->objs=objs;
    work->va_size=objs->def->n_x_models;
    work->cv2def_size=cv2def_size;
    if(cv2def_size==0){
		GV_Free(work);
		return NULL;
    }

    if((work->va=(VERTEX_ANIME_WORK **)GV_Malloc(sizeof(VERTEX_ANIME_WORK *)*work->va_size))==NULL){
		GV_Free(work);
		return NULL;
    }

    for(i=0;i<work->va_size;i++){
		if((work->va[i]=InitVertexAnimation(objs->objs+i,&(def[0]->models[i]),
											DG_VANIME_VERTS,cv2def_size))==NULL){

#ifdef DEBUG
			printf("Memory Overflow %d\n",i);
#endif
			while(i>0){
				i--;
				ExitVertexAnimation(work->va[i]);
			}
			GV_Free(work->va);
			GV_Free(work);
			return NULL;
		}
    }
    for(i=0;i<work->va_size;i++){
		for(j=0;j<work->cv2def_size;j++){
			work->va[i]->key[j]=&(def[j]->models[i]);
			work->va[i]->p[j]=0.0f;
		}
		work->va[i]->p[0]=1.0f;
		work->va[i]->count=0;
    }

    work->wind_min=WINDMIN;
    work->wind_max=WINDMAX;
    work->speed_min=W_SPEED_MIN;
    work->speed_max=W_SPEED_MAX;
    if(wind_min>0.0f) work->wind_min=wind_min;
    if(wind_max>0.0f) work->wind_max=wind_max;
    if(speed_min>0.0f) work->speed_min=1.0f/speed_min;
    if(speed_max>0.0f) work->speed_max=1.0f/speed_max;

    return work;
}

/* これらは、DG_OBJS内のworldマトリクスを使うため、GM_ActObject(2)を呼んだ後に
   呼び出さなくてはいけない */
void MoveVACoat(VACOAT_WORK *work)
{
    extern FVECTOR G_wind;
    int OK_GetLocalWind( FVECTOR *pos, FVECTOR *output );
    FVECTOR wind,*pwind=&wind;
    float str;
    float width,speed;
    int i,j;

    for(i=0;i<work->va_size;i++){
		SimpleVertexAnimation(work->va[i]);
    }

    if(!OK_GetLocalWind((FVECTOR *)&(work->objs->world.m[3][0]),&wind)){
		pwind=&G_wind;
    }

    str=fpu_Sqrt(pwind->vx*pwind->vx+pwind->vy*pwind->vy+pwind->vz*pwind->vz);

    // printf("Str = %f\n",str);

    /* 揺れ幅／周期を、現在の風の強さから計算 */
    if(str<work->wind_min){
		width=0.0f;
		speed=work->speed_min;
    }
    else if(str>work->wind_max){
		width=1.0f;
		speed=work->speed_max;
    }
    else{
		width=(str-work->wind_min)/(work->wind_max-work->wind_min);
		speed=work->speed_min+width*(work->speed_max-work->speed_min);
    }

    /* 現在の揺れ幅／周期から、次回の揺れ幅／周期を計算 */
    work->width=width;
    work->speed=speed;
    work->yure+=speed;

    /* vaの挿入値を更新 */
    {
		float yure0,yure1,yure2;
		int index0,index1;

		for(i=0;i<work->va_size;i++){
			for(j=0;j<work->cv2def_size;j++){
				work->va[i]->p[j]=0.0f;
			}
			work->va[i]->count=0;
		}

#if 0
		if(work->yure>=2.0f*M_PI){
			work->yure-=2.0f*M_PI;
			work->index++;
			if(work->index>=work->cv2def_size) work->index=0;
		}
#else
		if(work->yure>=2.0f){
			work->yure-=2.0f;
			work->index++;
			if(work->index>=work->cv2def_size) work->index=0;
		}
#endif

#if 0
		yure1=work->width*(1.0f+cosf(work->yure))/2.0f;
#else
		if(work->yure>1.0f){
			yure1=work->width*(2.0f-work->yure);
		}
		else{
			yure1=work->width*work->yure;
		}
#endif
		yure2=work->width-yure1;
		yure0=1.0f-work->width;

#if 0
		printf("yure0 yure1 yure2 = %d %d %d\n",
			   (int)(yure0*100.0f),(int)(yure1*100.0f),(int)(yure2*100.0f));
#endif

		index0=work->index;
		index1=index0+1;
		if(index1>=work->cv2def_size) index1=0;

		for(i=0;i<work->va_size;i++){
			work->va[i]->p[index0]=yure1;
			work->va[i]->p[index1]=yure2;
		}
		for(i=0;i<work->va_size;i++){
			work->va[i]->p[0]+=yure0;
		}
    }
}
