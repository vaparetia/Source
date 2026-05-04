//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mw_hukuyure_act.c
		装備品メイン

	2000/04/06 K.Kano
	$Id: mw_hukuyure_act.c,v 1.1.1.3 2002/11/19 11:43:20 Yoshizawa1 Exp $
*/


#include "mw_hukuyure.h"


int InitMWHukuyure(MWHUKUYURE_WORK *p)
{
    int i;

    p->stockp=0;
    p->stockm=NULL;

    p->parent=p->target->def->skeleton[p->tobjnum].parent;

    p->rt.vx=-p->target->def->skeleton[p->tobjnum].rt_tx;
    p->rt.vy=-p->target->def->skeleton[p->tobjnum].rt_ty;
    p->rt.vz=-p->target->def->skeleton[p->tobjnum].rt_tz;
    p->rt.vw=1.0f;

    p->t.vx=p->target->def->skeleton[p->tobjnum].tx;
    p->t.vy=p->target->def->skeleton[p->tobjnum].ty;
    p->t.vz=p->target->def->skeleton[p->tobjnum].tz;
    p->t.vw=1.0f;

    p->prt.vx=p->target->def->skeleton[p->parent].rt_tx;
    p->prt.vy=p->target->def->skeleton[p->parent].rt_ty;
    p->prt.vz=p->target->def->skeleton[p->parent].rt_tz;
    p->prt.vw=1.0f;

    if(p->stock_size==0) return 1;

    if((p->stockm
		=(FMATRIX *)GV_Malloc((sizeof(FMATRIX)+sizeof(float))*p->stock_size))==NULL) return 0;

    vu0_Ldv0(&(p->prt));
    vu0_Ldm0(p->target->matrix[p->target->use_buffer]+p->parent);

    vu0_Mulv0m0v0();

    vu0_Ldv1(&(p->t));
#ifdef BP_PSX2_ASM
    asm volatile ("vmove.xyzw vf7,vf1");
#else
	{
		extern FVECTOR REG_vf1 ;
		fpu_CopyVector( (FVECTOR*)vu0_Ptrm0()->m[3], &REG_vf1 ) ;
	}
#endif

    vu0_Mulv1m0v1();

#ifdef BP_PSX2_ASM
    asm volatile ("vmove.xyzw vf7,vf2");
#else
	{
		extern FVECTOR REG_vf2 ;
		fpu_CopyVector( (FVECTOR*)vu0_Ptrm0()->m[3], &REG_vf2 ) ;
	}

#endif

    for(i=0;i<p->stock_size;i++){
		vu0_Stm0(&(p->stockm[i]));
    }

    return 1;
}

void ExitMWHukuyure(MWHUKUYURE_WORK *p)
{
    if(p->stockm!=NULL) GV_Free(p->stockm);
}

void MoveMWHukuyure(MWHUKUYURE_WORK *p)
{
    FMATRIX m,m2;
    FMATRIX *evm_m,*evm_pm;

    evm_pm=p->target->matrix[p->target->use_buffer]+p->parent;
    evm_m=p->target->matrix[p->target->use_buffer]+p->tobjnum;

    /* 古いマトリクスを利用する。*/
    vu0_Ldv0(&(p->prt));
    vu0_Ldm0(evm_pm);

    vu0_Mulv0m0v0();

    vu0_Ldv1(&(p->t));
#ifdef BP_PSX2_ASM
    asm volatile ("vmove.xyzw	vf7,vf1");
#else
	{
		extern FVECTOR REG_vf1 ;
		fpu_CopyVector( (FVECTOR*)vu0_Ptrm0()->m[3], &REG_vf1 ) ;
	}
#endif

    vu0_Mulv1m0v1();

    vu0_Stm0(&m);
    vu0_Stv1((FVECTOR *)&(m.m[3][0]));


    if(p->stock_size>0){
		fpu_CopyMatrix(&m2,&(p->stockm[p->stockp]));
		fpu_CopyMatrix(&(p->stockm[p->stockp]),&m);
    }
    else{
		fpu_CopyMatrix(&m2,&m);
    }

    if(m2.m[3][1]<m.m[3][1]-p->limity) m2.m[3][1]=m.m[3][1]-p->limity;
    else if(m2.m[3][1]>m.m[3][1]+p->limity) m2.m[3][1]=m.m[3][1]+p->limity;
    m2.m[3][0]=m.m[3][0];
    m2.m[3][2]=m.m[3][2];

    vu0_Ldv0(&(p->rt));
    vu0_Ldm0(&m2);

    vu0_Mulv0m0v0();

    vu0_Stm0(evm_m);
    vu0_Stv0((FVECTOR *)&(evm_m->m[3][0]));

    p->stockp++;
    if(p->stockp>=p->stock_size) p->stockp=0;
}


void MoveMWHukuyure2(MWHUKUYURE_WORK *p)
{
    FMATRIX m,m2;
    FMATRIX *evm_m,*evm_pm;

    evm_pm=p->target->matrix[p->target->use_buffer]+p->parent;
    evm_m=p->target->matrix[p->target->use_buffer]+p->tobjnum;

    /* 古いマトリクスを利用する。*/
    vu0_Ldv0(&(p->prt));
    vu0_Ldm0(evm_pm);

    vu0_Mulv0m0v0();

    vu0_Ldv1(&(p->t));
#ifdef BP_PSX2_ASM
    asm volatile ("vmove.xyzw	vf7,vf1");
#else
	{
		extern FVECTOR REG_vf1 ;
		fpu_CopyVector( (FVECTOR*)vu0_Ptrm0()->m[3], &REG_vf1 ) ;
	}
#endif

    vu0_Mulv1m0v1();

    vu0_Stm0(&m);
    vu0_Stv1((FVECTOR *)&(m.m[3][0]));

    if(p->stock_size>0){
		fpu_CopyMatrix(&m2,&(p->stockm[p->stockp]));
		fpu_CopyMatrix(&(p->stockm[p->stockp]),&m);
    }
    else{
		fpu_CopyMatrix(&m2,&m);
    }

    vu0_Stv1((FVECTOR *)&(m2.m[3][0]));


    vu0_Ldv0(&(p->rt));
    vu0_Ldm0(&m2);

    vu0_Mulv0m0v0();

    vu0_Stm0(evm_m);
    vu0_Stv0((FVECTOR *)&(evm_m->m[3][0]));


    p->stockp++;
    if(p->stockp>=p->stock_size) p->stockp=0;
}
