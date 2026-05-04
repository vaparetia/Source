//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
	vertex_animation.c
		頂点アニメーション

	1999/11/09 K.Kano
	$Id: vertex_animation.c,v 1.4 2002/11/23 11:50:59 Yoshizawa1 Exp $
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


#if 1//BP_ASM - Xbox version is in C - #ifdef KP_XBOX  /**************XBOX用の専用関数(書換え版)***********************  T.Morita 2002.02.01 */
static void VertexAnimation(FVECTOR *r,FVECTOR *a,FVECTOR *b,int size,float p1,float p2)
{
    int i=size;
	FVECTOR ACC, vf24, vf25 ;

#ifndef KP_WINDOWS
    while(i>0){
		fpu_MulVectorScaler( &ACC , a, p1 ) ;
		fpu_MulVectorScaler( &vf25, b, p2 ) ;
		fpu_AddVectors( (r+0), &ACC, &vf25 ) ;

		if( i == 1 ) break;

		fpu_MulVectorScaler( &ACC , a+1, p1 ) ;
		fpu_MulVectorScaler( &vf24, b+1, p2 ) ;
		fpu_AddVectors( (r+1), &ACC, &vf24 ) ;

		a+=2; b+=2; r+=2; i-=2;
	}
#else
	if( !(i & 1) )
	{
		/*-- 偶数 --*/
		i >>= 1 ;
	    while(i>0){
			fpu_MulVectorScaler( &ACC , a, p1 ) ;
			fpu_MulVectorScaler( &vf25, b, p2 ) ;
			fpu_AddVectors( (r+0), &ACC, &vf25 ) ;

			fpu_MulVectorScaler( &ACC , a+1, p1 ) ;
			fpu_MulVectorScaler( &vf24, b+1, p2 ) ;
			fpu_AddVectors( (r+1), &ACC, &vf24 ) ;

			a+=2; b+=2; r+=2; i--;
		}
	}
	else
	{
		/*-- 奇数 --*/
		i >>= 1 ;
	    while(i>0){
			fpu_MulVectorScaler( &ACC , a, p1 ) ;
			fpu_MulVectorScaler( &vf25, b, p2 ) ;
			fpu_AddVectors( (r+0), &ACC, &vf25 ) ;

			fpu_MulVectorScaler( &ACC , a+1, p1 ) ;
			fpu_MulVectorScaler( &vf24, b+1, p2 ) ;
			fpu_AddVectors( (r+1), &ACC, &vf24 ) ;

			a+=2; b+=2; r+=2; i--;
		}
		fpu_MulVectorScaler( &ACC , a, p1 ) ;
		fpu_MulVectorScaler( &vf25, b, p2 ) ;
		fpu_AddVectors( (r+0), &ACC, &vf25 ) ;
	}
#endif
}

#else        /************** PS2�用の専用関数(オリジナル)***********************  T.Morita 2002.02.01 */

/* 二つの頂点列を適当な重み値で混ぜ合わせる */
static void VertexAnimation(FVECTOR *r,FVECTOR *a,FVECTOR *b,int size,float p1,float p2)
{
    int i=size;

#if 1
    asm volatile ("
    lqc2	vf18,0x00(%0)
    lqc2	vf19,0x00(%1)
    qmtc2.ni	%2,vf16
    qmtc2.ni	%3,vf17
    lqc2	vf20,0x10(%0)
    lqc2	vf21,0x10(%1)
    vmulax.xyz	ACC,vf18,vf16x
    vmaddx.xyz	vf24,vf19,vf17x
    " : : "r"(a),"r"(b),"r"(p1),"r"(p2) );

    a+=2; b+=2;

    while(i>0){
	asm volatile ("
	lqc2		vf18,0x00(%0)
	lqc2		vf19,0x00(%1)
	vmulax.xyz	ACC,vf20,vf16x
	vmaddx.xyz	vf25,vf21,vf17x
	sqc2		vf24,0x00(%2)
	lqc2		vf20,0x10(%0)
	lqc2		vf21,0x10(%1)
	vmulax.xyz	ACC,vf18,vf16x
	vmaddx.xyz	vf24,vf19,vf17x
	sqc2		vf25,0x10(%2)
	" : : "r"(a),"r"(b),"r"(r) : "memory" );

	a+=2; b+=2; r+=2; i-=2;
    }
#else
    asm volatile ("
    lqc2	vf18,0x00(%0)
    lqc2	vf19,0x00(%1)
    qmtc2.ni	%2,vf16
    qmtc2.ni	%3,vf17
    lqc2	vf20,0x10(%0)
    lqc2	vf21,0x10(%1)
    vaddax.xyz	ACC,vf18,vf0x
    vmaddx.xyz	vf24,vf19,vf17x
    " : : "r"(a),"r"(b),"r"(p1),"r"(p2) );

    a+=2; b+=2;

    while(i>0){
	asm volatile ("
	lqc2		vf18,0x00(%0)
	lqc2		vf19,0x00(%1)
	vaddax.xyz	ACC,vf20,vf0x
	vmaddx.xyz	vf25,vf21,vf17x
	sqc2		vf24,0x00(%2)
	lqc2		vf20,0x10(%0)
	lqc2		vf21,0x10(%1)
	vaddax.xyz	ACC,vf18,vf0x
	vmaddx.xyz	vf24,vf19,vf17x
	sqc2		vf25,0x10(%2)
	" : : "r"(a),"r"(b),"r"(r) : "memory" );

	a+=2; b+=2; r+=2; i-=2;
    }
#endif
}
#endif /* KP_XBOX ************** PS2用の専用関数(オリジナル)ここまで***********************  T.Morita 2002.02.01 */

VERTEX_ANIME_WORK *InitVertexAnimation(DG_OBJ *obj,CV2_MDL *mdl,int flag,int n_mdl)
{
    VERTEX_ANIME_WORK *work;
    int msize=0;

    if((work=GV_Malloc(sizeof(VERTEX_ANIME_WORK)))==NULL) return NULL;

    if(flag & DG_VANIME_VERTS){
	if(mdl->n_verts>msize) msize=mdl->n_verts;
    }
    if(flag & DG_VANIME_NORMS){
	if(mdl->n_norms>msize) msize=mdl->n_norms;
    }
    if(flag & DG_VANIME_UVS){
	if(mdl->n_uvs>msize) msize=mdl->n_uvs;
    }

    msize=(msize+3) & ~3;
    msize*=sizeof(FVECTOR);

    work->tmp_work=NULL;
    if((work->tmp_work=(FVECTOR *)GV_Malloc((sizeof(CV2_MDL *)+sizeof(float)*2)*n_mdl+msize))
       ==NULL){

	GV_Free(work);
	return NULL;
    }

    work->key=(CV2_MDL **)((unsigned int)(work->tmp_work)+msize);
    work->p=(float *)(work->key+n_mdl);
    work->now_p=(float *)(work->p+n_mdl);

    work->size=n_mdl;
    work->count=0;

    DG_MakeAnimVertsBuffer(&(work->anime),obj,flag);
    if ( work->anime.mem[0] == NULL ){
	GV_Free(work);
	return NULL;
    }


    return work;
}

void ExitVertexAnimation(VERTEX_ANIME_WORK *work)
{
    if(work->tmp_work!=NULL) GV_Free(work->tmp_work);
    DG_FreeAnimVertsBuffer(&(work->anime));
    GV_Free(work);
}

void SimpleVertexAnimation(VERTEX_ANIME_WORK *work)
{
    FVECTOR *tmp=work->tmp_work;
    int i;

    DG_SwitchVAnimeBuffer(&(work->anime));

    /* 一次補間 */
    if(work->count<=0){
	for(i=work->size-1;i>=0;i--){
	    work->now_p[i]=work->p[i];
	}
	work->count=0;
    }
    else{
	float c=1.0f/(float)(work->count);
	float sum=0.0f;

	for(i=work->size-1;i>=0;i--){
	    work->now_p[i]+=(work->p[i]-work->now_p[i])*c;
	    sum+=work->now_p[i];
	}
#if 0
	if(sum!=0.0f){
	    sum=1.0f/sum;
	    for(i=work->size-1;i>=0;i--){
		work->now_p[i]*=sum;
	    }
	}
#endif

#if 0
	if(sum!=1.0f) printf("Sum = %f\n",sum);
#endif

	work->count--;
    }

    if(work->anime.flag & DG_VANIME_VERTS){
	int vsize=(work->key[0]->n_verts+3) & ~3;
	int msize=sizeof(FVECTOR)*vsize;

	GV_ZeroMemory(tmp,msize);

	for(i=work->size-1;i>=0;i--){

#if 0
	    if(work->now_p[i]>0.5f){
		int j;
		for(j=0;j<work->key[0]->n_verts;j++){
		    fpu_CopyVector(tmp+j,work->key[i]->verts+j);
		}
	    }
	    else
#endif

	    if(work->now_p[i]!=0.0f){
		static const float p=1.0f+(1.19209290E-07f)*8.0f; /* 1.0+機械イプシロン*8 */
		VertexAnimation(tmp,tmp,work->key[i]->verts,work->key[0]->n_verts,
				1.0f,work->now_p[i]*p);
	    }
	}

#ifndef KP_WINDOWS
	DG_RegistCommonVertex2(tmp,work->key[0]->n_verts);
	DG_RefineStripVertex2(&(work->anime),work->key[0]->verts_index);
#else
	DG_RegistCommonVertex2RefineStripVertex2(&(work->anime), tmp) ;
#endif

    }
    if(work->anime.flag & DG_VANIME_NORMS){
		int msize=sizeof(FVECTOR)*((work->key[0]->n_norms+1) & ~1);

		GV_ZeroMemory(tmp,msize);

		for(i=work->size-1;i>=0;i--){
		    if(work->now_p[i]!=0.0f){
			VertexAnimation(tmp,tmp,work->key[i]->norms,work->key[0]->n_norms,
					1.0f,work->now_p[i]);
		    }
		}
#ifndef KP_WINDOWS
		DG_RegistCommonNormal2(tmp,work->key[0]->n_norms);
		DG_RefineStripNormal(&(work->anime),work->key[0]->norms_index);
#else
		DG_RegistCommonNormal2RefineStripNormal(&(work->anime), tmp) ;
#endif
    }

    if(work->anime.flag & DG_VANIME_UVS){
		int msize=sizeof(FVECTOR)*((work->key[0]->n_uvs+1) & ~1);

		GV_ZeroMemory(tmp,msize);

		for(i=work->size-1;i>=0;i--){
		    if(work->now_p[i]!=0.0f){
			VertexAnimation(tmp,tmp,work->key[i]->uvs,work->key[0]->n_uvs,
					1.0f,work->now_p[i]);
	   	 }
		}
#if FALSE
		DG_RegistCommonUV(tmp,work->key[0]->n_uvs);
		DG_RefineStripUV(&(work->anime),work->key[0]->uvs_index);
#else
      BP_RENDER_TODO_BREAK; //this function is in C:\Bluepoint\CP4\dev\MGS2\mgs2x\source\system\libdg\wvanime.c
		//DG_RegistCommonUVRefineStripUV(&(work->anime), tmp) ;
#endif
    }
}
