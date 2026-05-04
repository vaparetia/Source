//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mm_orga.c
		オルガの乳揺れ用のプログラム

	1999/12/20 K.Kano
	$Id: mm_orga.c,v 1.1.1.3 2002/11/19 11:43:15 Yoshizawa1 Exp $
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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

#include "mm_orga_def.h"


static const int objnum[]={
    HUMAN21_MUNE,
    /* HUMAN21_KOSHI, */
    /* HUMAN21_ONAKA, */
    HUMAN21_MIGI_KATA,
    /* HUMAN21_MIGI_UDE1, */
    HUMAN21_HIDARI_KATA,
    /* HUMAN21_HIDARI_UDE1, */
};


typedef struct {
    MALTI_MATRIX_WORK *mmw[N_OBJNUM];
    FVECTOR x,v;
} MMORGA_WORK;


void ExitMMOrga(MMORGA_WORK *work)
{
    int i;
    for(i=0;i<N_OBJNUM;i++){
	if(work->mmw[i]!=NULL) ExitMaltiMatrix(work->mmw[i]);
    }
    GV_Free(work);
}

MMORGA_WORK *InitMMOrga(DG_OBJS *objs,CV2_DEF *def)
{
    MMORGA_WORK *work;
    int i;

    if((work=(MMORGA_WORK *)GV_Malloc(sizeof(MMORGA_WORK)))==NULL){
#ifdef DEBUG
	printf("Memory Overflow 0\n");
#endif
	return NULL;
    }
    GV_ZeroMemory(work,sizeof(MMORGA_WORK));

    for(i=0;i<N_OBJNUM;i++){
	if((work->mmw[i]=InitMaltiMatrix(objs,def,objnum[i],N_STOCK_ARRAYS,DG_VANIME_VERTS))==NULL){
#ifdef DEBUG
	    printf("Memory Overflow %d\n",objnum[i]);
#endif
	    ExitMMOrga(work);
	    return NULL;
	}
    }

    return work;
}


/* これらは、DG_OBJS内のworldマトリクスを使うため、GM_ActObject(2)を呼んだ後に
   呼び出さなくてはいけない */
void MoveMMOrga(MMORGA_WORK *work)
{
    int i;

    /* バネの式を利用して揺らす。*/

    struct _scratchpad dummy;
    struct _scratchpad *scratchpad;
    float l;

    static const FVECTOR base={
	0.0f,0.0f,CVC2N(30.0f),1.0f,
    };


#if 0
    scratchpad = (struct _scratchpad *)ScratchpadAlloc(sizeof(struct _scratchpad));
#elif 0
    scratchpad = (struct _scratchpad *)ScratchpadAlloc2(sizeof(struct _scratchpad));
#else
    scratchpad = &dummy;
#endif

    vu0_Ldv0(&base);
    vu0_Ldv2(&(work->x));
    vu0_Ldm0(work->mmw[0]->sm);

    vu0_Mulv0m0v0();

    vu0_Stv0(&(scratchpad->xb));

    vu0_Ldv1(&(work->v));

    vu0_Subv0v2();

    vu0_Mulv0a(SPRING_ACC);	/* バネ定数パラメータ */
    vu0_Mulv1a(SPRING_REDUCE);	/* 減速パラメータ */

    vu0_Addv0v1();


    l=vu0_VectorLength2v0();
    l=fpu_Rsqrt(l,SPRING_VLIMIT); /* 限界速度 */

    if(l<0.95f) vu0_Mulv0a(l);

    vu0_Ldv1(&(scratchpad->xb));
    vu0_Stv0(&(work->v));


    vu0_Addv2v0();

    vu0_Subv0v2v1();

    l=vu0_VectorLength2v0();
    l=fpu_Rsqrt(l,SPRING_XLIMIT); /* 限界距離 */

    if(l<0.95f){
	vu0_Mulv0a(l);
	vu0_Addv2v0v1();
    }

    vu0_Stv2(&(scratchpad->xn));

    UTL_MakeQuatM(&(scratchpad->q),
		  (FVECTOR *)&(work->mmw[0]->sm->m[3][0]),
		  &(scratchpad->xn),
		  work->mmw[0]->sm,
		  (FVECTOR *)&base);
    MT_QuatToMat(&(scratchpad->m),&(scratchpad->q));

#if 1

#if 0
    InverseMatrix(&(scratchpad->tm),work->mmw[0]->sm);
#elif 0
    sceVu0InversMatrix(*(sceVu0FMATRIX *)&(scratchpad->tm),
		       *(sceVu0FMATRIX *)work->mmw[0]->sm);
#else
    FastInverseMatrix(&(scratchpad->tm),work->mmw[0]->sm);
#endif

    /* 揺れの中心点の設定 */
    fpu_CopyVector(&(scratchpad->q),(FVECTOR *)&(work->mmw[0]->sm->m[3][0]));
    scratchpad->q.vy+=CVC2N(10.0f);

    vu0_Ldv2(&(scratchpad->q));


    vu0_Ldm1(work->mmw[0]->sm);
    vu0_Ldm0(&(scratchpad->m));

#ifdef BP_PSX2_ASM
    asm volatile ("vsub.xyz vf11,vf11,vf3");
#else
	fpu_SubVectors( (FVECTOR *)vu0_Ptrm1()->m[3], (FVECTOR *)vu0_Ptrm1()->m[3], &REG_vf3 ) ;
#endif
    vu0_Ldv0(&base);

    vu0_Mulm2m0m1();

    vu0_Ldm1(&(scratchpad->tm));

#ifdef BP_PSX2_ASM
    asm volatile ("vadd.xyz vf15,vf15,vf3");
#else
	fpu_AddVectors( (FVECTOR *)vu0_Ptrm2()->m[3], (FVECTOR *)vu0_Ptrm2()->m[3], &REG_vf3 ) ;
#endif

    vu0_Mulv0m2v0();

    vu0_Mulm0m1m2();

    vu0_Stv0(&(work->x));

    vu0_Stm0(&(work->mmw[0]->m[0]));

    MoveMaltiMatrix(work->mmw[0]);


    for(i=1;i<N_OBJNUM;i++){

#if 0
	InverseMatrix(&(scratchpad->tm),work->mmw[i]->sm);
#elif 0
	sceVu0InversMatrix(*(sceVu0FMATRIX *)&(scratchpad->tm),
			   *(sceVu0FMATRIX *)work->mmw[i]->sm);
#else
	FastInverseMatrix(&(scratchpad->tm),work->mmw[i]->sm);
#endif

	vu0_Ldm1(work->mmw[i]->sm);
	vu0_Ldm0(&(scratchpad->m));

#ifdef BP_PSX2_ASM
    asm volatile ("vsub.xyz vf11,vf11,vf3");
#else
	fpu_SubVectors( (FVECTOR *)vu0_Ptrm1()->m[3], (FVECTOR *)vu0_Ptrm1()->m[3], &REG_vf3 ) ;
#endif

	vu0_Mulm2m0m1();

	vu0_Ldm1(&(scratchpad->tm));

#ifdef BP_PSX2_ASM
    asm volatile ("vadd.xyz vf15,vf15,vf3");
#else
	fpu_AddVectors( (FVECTOR *)vu0_Ptrm2()->m[3], (FVECTOR *)vu0_Ptrm2()->m[3], &REG_vf3 ) ;
#endif

	vu0_Mulm0m1m2();

	vu0_Stm0(&(work->mmw[i]->m[0]));

	MoveMaltiMatrix(work->mmw[i]);
    }


#else

#if 0
    InverseMatrix(&(scratchpad->tm),work->mmw[0]->sm);
#elif 0
    sceVu0InversMatrix(*(sceVu0FMATRIX *)&(scratchpad->tm),
		       *(sceVu0FMATRIX *)work->mmw[0]->sm);
#else
    FastInverseMatrix(&(scratchpad->tm),work->mmw[0]->sm);
#endif

    vu0_Ldm0(&(scratchpad->m));
    vu0_Ldm1(work->mmw[0]->sm);

    vu0_Mulm2m0m1();

    vu0_Stm2(&(scratchpad->m));
    vu0_Ldv0(&base);
    vu0_Ldm0(&(scratchpad->tm));

    fpu_CopyVector((FVECTOR *)&(scratchpad->m.m[3][0]),
		   (FVECTOR *)&(work->mmw[0]->sm->m[3][0]));

    vu0_Ldm2(&(scratchpad->m));

    vu0_Mulv0m2v0();
    vu0_Mulm1m0m2();

    vu0_Stv0(&(work->x));
    vu0_Stm1(&(scratchpad->m));

    for(i=0;i<N_OBJNUM;i++){
	fpu_CopyMatrix(&(work->mmw[i]->m[0]),&(scratchpad->m));
    }
    for(i=0;i<N_OBJNUM;i++){
	MoveMaltiMatrix(work->mmw[i]);
    }

#endif


#if 0
    ScratchpadFree(scratchpad);
#elif 0
    ScratchpadFree2();
#endif

}
