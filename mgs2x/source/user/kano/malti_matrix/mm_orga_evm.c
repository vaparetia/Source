//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mm_orga_evm.c
		オルガの乳揺れ用のプログラム

	1999/12/20 K.Kano
	$Id: mm_orga_evm.c,v 1.1.1.3 2002/11/19 11:43:15 Yoshizawa1 Exp $
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

#include "mm_orga_def.h"
#include "../mw_hukuyure/mw_hukuyure.h"


typedef struct {
    DG_EVMOBJ *evm;
	int bone53;

    FVECTOR x,v;

    FVECTOR rt2;
    FVECTOR t53,rt53;

    /* 太股用 */
    MWHUKUYURE_WORK mw[2];
} EVMMMORGA_WORK;


void ExitEvmMMOrga(EVMMMORGA_WORK *work)
{
    ExitMWHukuyure(&(work->mw[0]));
    ExitMWHukuyure(&(work->mw[1]));
    GV_Free(work);
}


#if 0


#define BONE_ORGA_MUNE		53

#define BONE_ORGA_RIGHT_THIGH	54
#define BONE_ORGA_LEFT_THIGH	55


EVMMMORGA_WORK *InitEvmMMOrga(DG_EVMOBJ *evm)
{
    EVMMMORGA_WORK *work;


    if((work=(EVMMMORGA_WORK *)GV_Malloc(sizeof(EVMMMORGA_WORK)))==NULL){
#ifdef DEBUG
		printf("Memory Overflow 0\n");
#endif
		return NULL;
    }
    GV_ZeroMemory(work,sizeof(EVMMMORGA_WORK));

	work->bone53=BONE_ORGA_MUNE;
    work->mw[0].target=work->mw[1].target=evm;
    work->mw[0].tobjnum=BONE_ORGA_RIGHT_THIGH;
    work->mw[1].tobjnum=BONE_ORGA_LEFT_THIGH;
    work->mw[0].stock_size=work->mw[1].stock_size=8;
    // work->mw[0].stock_size=work->mw[1].stock_size=0;
    work->mw[0].limity=work->mw[1].limity=CVC2N(5.0f);

    if(!InitMWHukuyure(&(work->mw[0]))){
		GV_Free(work);
		return NULL;
    }
    if(!InitMWHukuyure(&(work->mw[1]))){
		ExitMWHukuyure(&(work->mw[0]));
		GV_Free(work);
		return NULL;
    }

    work->evm=evm;

    work->rt2.vx=evm->def->skeleton[HUMAN21_MUNE].rt_tx;
    work->rt2.vy=evm->def->skeleton[HUMAN21_MUNE].rt_ty;
    work->rt2.vz=evm->def->skeleton[HUMAN21_MUNE].rt_tz;
    work->rt2.vw=1.0f;

    work->t53.vx=evm->def->skeleton[work->bone53].tx;
    work->t53.vy=evm->def->skeleton[work->bone53].ty;
    work->t53.vz=evm->def->skeleton[work->bone53].tz;
    work->t53.vw=1.0f;
    work->rt53.vx=-evm->def->skeleton[work->bone53].rt_tx;
    work->rt53.vy=-evm->def->skeleton[work->bone53].rt_ty;
    work->rt53.vz=-evm->def->skeleton[work->bone53].rt_tz;
    work->rt53.vw=1.0f;

    return work;
}

#endif


EVMMMORGA_WORK *_InitEvmMMOrga(DG_EVMOBJ *evm,int mune_bone,int rthigh_bone,int lthigh_bone)
{
    EVMMMORGA_WORK *work;


    if((work=(EVMMMORGA_WORK *)GV_Malloc(sizeof(EVMMMORGA_WORK)))==NULL){
#ifdef DEBUG
		printf("Memory Overflow 0\n");
#endif
		return NULL;
    }
    GV_ZeroMemory(work,sizeof(EVMMMORGA_WORK));

	work->bone53=mune_bone;
    work->mw[0].target=work->mw[1].target=evm;
    work->mw[0].tobjnum=rthigh_bone;
    work->mw[1].tobjnum=lthigh_bone;
    work->mw[0].stock_size=work->mw[1].stock_size=8;
    // work->mw[0].stock_size=work->mw[1].stock_size=0;
    work->mw[0].limity=work->mw[1].limity=CVC2N(5.0f);

    if(!InitMWHukuyure(&(work->mw[0]))){
		GV_Free(work);
		return NULL;
    }
    if(!InitMWHukuyure(&(work->mw[1]))){
		ExitMWHukuyure(&(work->mw[0]));
		GV_Free(work);
		return NULL;
    }

    work->evm=evm;

    work->rt2.vx=evm->def->skeleton[HUMAN21_MUNE].rt_tx;
    work->rt2.vy=evm->def->skeleton[HUMAN21_MUNE].rt_ty;
    work->rt2.vz=evm->def->skeleton[HUMAN21_MUNE].rt_tz;
    work->rt2.vw=1.0f;

    work->t53.vx=evm->def->skeleton[work->bone53].tx;
    work->t53.vy=evm->def->skeleton[work->bone53].ty;
    work->t53.vz=evm->def->skeleton[work->bone53].tz;
    work->t53.vw=1.0f;
    work->rt53.vx=-evm->def->skeleton[work->bone53].rt_tx;
    work->rt53.vy=-evm->def->skeleton[work->bone53].rt_ty;
    work->rt53.vz=-evm->def->skeleton[work->bone53].rt_tz;
    work->rt53.vw=1.0f;

    return work;
}

void MoveEvmMMOrga(EVMMMORGA_WORK *work)
{
#ifndef PSX2
    extern FVECTOR REG_vf3, REG_vf2 ;
#endif

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

    vu0_Ldv1(&(work->rt2));
    vu0_Ldm0(work->evm->matrix[work->evm->use_buffer]+HUMAN21_MUNE);
    vu0_Mulv1m0v1();
    vu0_Stm0(&(scratchpad->sm));
    vu0_Stv1((FVECTOR *)&(scratchpad->sm.m[3][0]));

    vu0_Ldv0(&base);
    vu0_Ldv2(&(work->x));
    vu0_Ldm0(&(scratchpad->sm));

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
				  (FVECTOR *)&(scratchpad->sm.m[3][0]),
				  &(scratchpad->xn),
				  &(scratchpad->sm),
				  (FVECTOR *)&base);
    MT_QuatToMat(&(scratchpad->m),&(scratchpad->q));

#if 0
    InverseMatrix(&(scratchpad->tm),&(scratchpad->sm));
#elif 0
    sceVu0InversMatrix(*(sceVu0FMATRIX *)&(scratchpad->tm),
					   *(sceVu0FMATRIX *)&(scratchpad->sm));
#else
    FastInverseMatrix(&(scratchpad->tm),&(scratchpad->sm));
#endif

    /* 揺れの中心点の設定 */
    fpu_CopyVector(&(scratchpad->q),(FVECTOR *)&(scratchpad->sm.m[3][0]));
    scratchpad->q.vy+=CVC2N(10.0f);

    vu0_Ldv2(&(scratchpad->q));


    vu0_Ldm1(&(scratchpad->sm));
    vu0_Ldm0(&(scratchpad->m));

#ifdef BP_PSX2_ASM
    asm volatile ("vsub.xyz vf11,vf11,vf3");
#else
	fpu_SubVectors( (FVECTOR *)vu0_Ptrm1()->m[3], (FVECTOR *)vu0_Ptrm1()->m[3], &REG_vf3 ) ;
#endif

    vu0_Ldv0(&base);

    vu0_Mulm2m0m1();

    // vu0_Ldm1(&(scratchpad->tm));

#ifdef BP_PSX2_ASM
    asm volatile ("vadd.xyz vf15,vf15,vf3");
#else
	fpu_AddVectors( (FVECTOR *)vu0_Ptrm2()->m[3], (FVECTOR *)vu0_Ptrm2()->m[3], &REG_vf3 ) ;
#endif

    vu0_Mulv0m2v0();

    // vu0_Mulm0m1m2();
    vu0_Cpm2m0();


    vu0_Ldv1(&(work->t53));

    vu0_Stv0(&(work->x));

    // vu0_Stm0(&(work->mmw[0]->m[0]));

    vu0_Mulv1m0v1();

    vu0_Ldv2(&(work->rt53));

#ifdef BP_PSX2_ASM
    asm volatile ("vmove.xyzw	vf7,vf2");
#else
	fpu_CopyVector( (FVECTOR *)vu0_Ptrm0()->m[3], &REG_vf2 ) ;
#endif

    vu0_Mulv2m0v2();

    vu0_Stm0(work->evm->matrix[work->evm->use_buffer]+work->bone53);
    vu0_Stv2((FVECTOR *)&((work->evm->matrix[work->evm->use_buffer]+work->bone53)->m[3][0]));


    MoveMWHukuyure(&(work->mw[0]));
    MoveMWHukuyure(&(work->mw[1]));
}
