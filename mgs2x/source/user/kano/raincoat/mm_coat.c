//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mm_coat.c
		レインコートのコートの揺れ

	1999/12/20 K.Kano
	$Id: mm_coat.c,v 1.1.1.3 2002/11/19 11:43:32 Yoshizawa1 Exp $
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


#define N_STOCK_ARRAYS	2


typedef struct {
    MALTI_MATRIX_WORK **mmw;
    FVECTOR x[N_STOCK_ARRAYS],v[N_STOCK_ARRAYS];
    int size;
} MMCOAT_WORK;


MMCOAT_WORK *InitMMCoat(DG_OBJS *objs,CV2_DEF *def)
{
    MMCOAT_WORK *work;
    int i;
    int start;
    int size;

    if((work=(MMCOAT_WORK *)GV_Malloc(sizeof(MMCOAT_WORK)))==NULL){
#ifdef DEBUG
	printf("Memory Overflow 0\n");
#endif
	return NULL;
    }
    GV_ZeroMemory(work,sizeof(MMCOAT_WORK));

    work->size=size=objs->def->n_x_models-objs->def->n_models;
    if(size==0){
	GV_Free(work);
	return NULL;
    }

    if((work->mmw=(MALTI_MATRIX_WORK **)GV_Malloc(sizeof(MALTI_MATRIX_WORK *)*size))==NULL){
	GV_Free(work);
	return NULL;
    }

    start=objs->def->n_models;

    for(i=0;i<size;i++){
	if((work->mmw[i]=InitMaltiMatrix(objs,def,start+i,N_STOCK_ARRAYS,DG_VANIME_VERTS))==NULL){
#ifdef DEBUG
	    printf("Memory Overflow %d\n",start+i);
#endif
	    while(i>0){
		i--;
		ExitMaltiMatrix(work->mmw[i]);
	    }
	    GV_Free(work);
	    return NULL;
	}
    }
    for(i=0;i<N_STOCK_ARRAYS;i++){
	fpu_ClearVector(&(work->x[i]));
	fpu_ClearVector(&(work->v[i]));
    }

    return work;
}

void ExitMMCoat(MMCOAT_WORK *work)
{
    int i;
    if(work->mmw!=NULL){
	for(i=0;i<work->size;i++){
	    if(work->mmw[i]!=NULL) ExitMaltiMatrix(work->mmw[i]);
	}
	GV_Free(work->mmw);
    }
    GV_Free(work);
}

/* これらは、DG_OBJS内のworldマトリクスを使うため、GM_ActObject(2)を呼んだ後に
   呼び出さなくてはいけない */
void MoveMMCoat(MMCOAT_WORK *work)
{
    extern FVECTOR G_wind;
    int OK_GetLocalWind( FVECTOR *pos, FVECTOR *output );

    static const struct {
	float xlimit;
	float xlimit2;
	
	float reduce_v;
	float spring_u;
	float wind_effect;
	float bound_e;
    } param[N_STOCK_ARRAYS]={

#define XLIMIT		CVC2N(3.0f)
#define XLIMIT2		CVC2N(5.0f)

	{ /* 風で服に密着する */
	    XLIMIT,		/* 位置制限範囲 */
	    XLIMIT*XLIMIT,	/* 制限範囲の自乗 */

	    1.0f-0.005f,	/* 減速率 */
	    0.01f,		/* バネ定数 */
	    0.002f,		/* 風による影響度 */
	    1.0f+0.8f,		/* 跳ね返り係数 */
	},
	{ /* 小刻みに震える */
	    XLIMIT2,		/* 位置制限範囲 */
	    XLIMIT2*XLIMIT2,	/* 制限範囲の自乗 */

	    1.0f-0.0003f,	/* 減速率 */
	    0.0005f,		/* バネ定数 */
	    0.0001f,		/* 風による影響度 */
	    1.0f+1.0f,		/* 跳ね返り係数 */
	},
    };

    FVECTOR wind,*pwind=&wind;
    FVECTOR *x;
    float l;
    FMATRIX m[N_STOCK_ARRAYS];
    FMATRIX inv;
    int i;


    x=(FVECTOR *)&(work->mmw[0]->sm->m[3][0]);
    if(!OK_GetLocalWind(x,&wind)){
	pwind=&G_wind;
    }


#if 0
    printf("wind = %f %f %f\n",pwind->vx,pwind->vy,pwind->vz);
#endif

    for(i=0;i<N_STOCK_ARRAYS;i++){
	vu0_Ldv0(x);
	vu0_Ldv1(&(work->x[i]));
	vu0_Ldv2(&(work->v[i]));

	vu0_Subv0v1();
	vu0_Ldv1(pwind);

	vu0_Mulv2a(param[i].reduce_v);
	vu0_Mulv0a(param[i].spring_u); 
	vu0_Mulv1a(param[i].wind_effect);

	vu0_Addv2v0();
	vu0_Addv2v1();

	vu0_Ldv1(&(work->x[i]));

	vu0_Ldv0(x);

	vu0_Addv1v2();

	vu0_Stv2(&(work->v[i]));
	vu0_Stv1(&(work->x[i]));

	vu0_Subv1v0();

	l=vu0_VectorLength2v1();

	if(l>=param[i].xlimit2){
	    /* 長さをXLIMITに */
	    vu0_Mulv1a(fpu_Rsqrt(l,param[i].xlimit));

	    vu0_Addv0v1();

	    /* さらに長さを1に */
	    vu0_Mulv1a(1.0f/param[i].xlimit);

	    vu0_Stv0(&(work->x[i]));

	    l=vu0_InnerProductv1v2();
	    vu0_Mulv1a(l*param[i].bound_e);

	    vu0_Subv2v1();

	    vu0_Stv2(&(work->v[i]));

#if 0
	    {
		FVECTOR t;
		vu0_Stv1(&t);
		printf("t = %f %f %f\n",t.vx,t.vy,t.vz);
	    }
#endif

	}

#if 0
	printf("x = %f %f %f\n",work->x.vx,work->x.vy,work->x.vz);
	printf("v = %f %f %f\n",work->v.vx,work->v.vy,work->v.vz);
#endif


	fpu_CopyMatrix(&(m[i]),work->mmw[0]->sm);
	fpu_CopyVector((FVECTOR *)&(m[i].m[3][0]),&(work->x[i]));

#if 0
	InverseMatrix(&inv,work->mmw[0]->sm);
#elif 0
	sceVu0InversMatrix(*(sceVu0FMATRIX *)&inv,
			   *(sceVu0FMATRIX *)work->mmw[0]->sm);
#else
	FastInverseMatrix(&inv,work->mmw[0]->sm);
#endif

	vu0_Ldm0(&(m[i]));
	vu0_Ldm1(&inv);
	vu0_Mulm2m0m1();

	vu0_Stm2(&(m[i]));

#if 0
	printf("%f %f %f %f\n",m.m[0][0],m.m[1][0],m.m[2][0],m.m[3][0]);
	printf("%f %f %f %f\n",m.m[0][1],m.m[1][1],m.m[2][1],m.m[3][1]);
	printf("%f %f %f %f\n",m.m[0][2],m.m[1][2],m.m[2][2],m.m[3][2]);
	printf("%f %f %f %f\n\n",m.m[0][3],m.m[1][3],m.m[2][3],m.m[3][3]);
#endif
    }

    for(i=0;i<work->size;i++){
	int j;

#if 0
	InverseMatrix(&inv,work->mmw[i]->sm);
#elif 0
	sceVu0InversMatrix(*(sceVu0FMATRIX *)&inv,
			   *(sceVu0FMATRIX *)work->mmw[i]->sm);
#else
	FastInverseMatrix(&inv,work->mmw[i]->sm);
#endif

	for(j=0;j<N_STOCK_ARRAYS;j++){
	    vu0_Ldm1(work->mmw[i]->sm);
	    vu0_Ldm0(&(m[j]));

	    vu0_Mulm2m0m1();

	    vu0_Ldm1(&inv);

	    vu0_Mulm0m1m2();

	    vu0_Stm0(&(work->mmw[i]->m[j]));
	}

	MoveMaltiMatrix(work->mmw[i]);
    }
}
