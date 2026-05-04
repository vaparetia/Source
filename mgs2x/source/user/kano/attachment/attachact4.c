//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	attachact4.c
		装備品メイン

	1999/12/03 K.Kano
	$Id: attachact4.c,v 1.1.1.3 2002/11/19 11:43:03 Yoshizawa1 Exp $
*/


#include "attachment4.h"

#if 1 //BP_PS2
//#ifndef PSX2 
static FVECTOR _vf15 ;
#endif

void InitAttachment4(DG_OBJS *objs,MODEL_PARAMETER4 *p)
{
    DG_MDL *mdl=objs->def->models;

    vu0_Clrv0();

    if(fpu_Abs(mdl[0].lx)<fpu_Abs(mdl[0].ux)) p->lastobjx.vx=mdl[0].ux;
    else p->lastobjx.vx=mdl[0].lx;
    if(fpu_Abs(mdl[0].ly)<fpu_Abs(mdl[0].uy)) p->lastobjx.vy=mdl[0].uy;
    else p->lastobjx.vy=mdl[0].ly;
    if(fpu_Abs(mdl[0].lz)<fpu_Abs(mdl[0].uz)) p->lastobjx.vz=mdl[0].uz;
    else p->lastobjx.vz=mdl[0].lz;

    if(fpu_Abs(p->lastobjx.vx)>fpu_Abs(p->lastobjx.vy)){
		if(fpu_Abs(p->lastobjx.vx)>fpu_Abs(p->lastobjx.vz)){
			/* X */
			p->lastobjx.vy=p->lastobjx.vz=0.0f;
		}
		else{
			/* Z */
			p->lastobjx.vx=p->lastobjx.vy=0.0f;
		}
    }
    else{
		if(fpu_Abs(p->lastobjx.vy)>fpu_Abs(p->lastobjx.vz)){
			/* Y */
			p->lastobjx.vx=p->lastobjx.vz=0.0f;
		}
		else{
			/* Z */
			p->lastobjx.vx=p->lastobjx.vy=0.0f;
		}
    }
    p->lastobjx.vw=1.0f;

	p->first_flag=1;
}

int Attachment4_GetStock(MODEL_PARAMETER4 *p)
{
    if((p->stockv=(FVECTOR *)GV_Malloc(sizeof(FVECTOR)*p->stock_size))==NULL) return 0;
    return 1;
}

void InitCalcAttachment4(MODEL_PARAMETER4 *p)
{
	int i;

    vu0_Ldm2(&(p->rotm));
    vu0_Ldm1(p->tmat);

#ifdef BP_PSX2_ASM
    asm volatile ("lqc2		vf15,0x00(%0)" : : "r"(&(p->x[0])) );
#else
	_vf15 = p->x[0] ;
#endif
    vu0_Ldv0(&(p->lastobjx));

    vu0_Mulm0m1m2();

    vu0_Mulv0m0v0();

    for(i=0;i<p->stock_size;i++){
		vu0_Stv0(p->stockv+i);
    }
}

void ExitAttachment4(MODEL_PARAMETER4 *p)
{
    if(p->stockv!=NULL) GV_Free(p->stockv);
}

void MoveAttachment4(DG_OBJS *objs,MODEL_PARAMETER4 *p)
{
    struct _scratchpad {
		FMATRIX m,m2;
		FVECTOR q;
		FVECTOR x[2];
		FVECTOR srcv;
    };
    struct _scratchpad *scratchpad;

    /* スクラッチパッドのみで納まるかあやしいので、
       管理関数を介するようにした。*/
    ScratchpadManInit();

#if 0
    scratchpad = (struct _scratchpad *)ScratchpadAlloc(sizeof(struct _scratchpad));
#else
    scratchpad = (struct _scratchpad *)ScratchpadAlloc2(sizeof(struct _scratchpad));
#endif

    vu0_Ldv1(&(p->x[0]));
    vu0_Ldv0(&(p->lastobjx));

    vu0_Ldm2(&(p->rotm));
    vu0_Ldm1(p->tmat);

#ifdef BP_PSX2_ASM
    asm volatile ("vmove.xyzw	vf15,vf2");
#else
	*(FVECTOR*)vu0_Ptrm2()->m[3] = REG_vf2 ;
#endif

    vu0_Mulv1m1v1();
    vu0_Mulm0m1m2();

    fpu_CopyVector(&(scratchpad->x[1]),p->stockv+p->stockp);

    vu0_Mulv0m0v0();

    vu0_Stv1(&(scratchpad->x[0]));
    vu0_Stv0(p->stockv+p->stockp);

    vu0_Subv0v1();

    vu0_Stm0(&(scratchpad->m2));
    vu0_Stv0(&(scratchpad->srcv));

    UTL_MakeQuat(&(scratchpad->q),&(scratchpad->x[0]),&(scratchpad->x[1]),&(scratchpad->srcv));

    if(scratchpad->q.vw<p->cos_limit){
		float s;

		s=fpu_Rsqrt(1.0f-scratchpad->q.vw*scratchpad->q.vw,p->sin_limit);
		scratchpad->q.vx*=s;
		scratchpad->q.vy*=s;
		scratchpad->q.vz*=s;
		scratchpad->q.vw=p->cos_limit;
    }

    MT_QuatToMat(&(scratchpad->m),&(scratchpad->q));


    //printf("Q = %f %f %f %f\n",scratchpad->q.vx,scratchpad->q.vy,scratchpad->q.vz,scratchpad->q.vw);


    vu0_Ldm1(&(scratchpad->m));
    vu0_Ldm2(&(scratchpad->m2));

    vu0_Mulm0m1m2();

    vu0_Stm0(&(objs->world));
    vu0_Stm0(&(objs->objs[0].world));
    fpu_CopyVector((FVECTOR *)&(objs->world.m[3][0]),&(scratchpad->x[0]));
    fpu_CopyVector((FVECTOR *)&(objs->objs[0].world.m[3][0]),&(scratchpad->x[0]));


    p->stockp++;
    if(p->stockp>=p->stock_size) p->stockp=0;

    if(objs->n_models>1){
		int i;

		vu0_Ldm2(&(objs->objs[0].world));

		scratchpad->x[0].vx=objs->def->models[1].tx;
		scratchpad->x[0].vy=objs->def->models[1].ty;
		scratchpad->x[0].vz=objs->def->models[1].tz;
		scratchpad->x[0].vw=1.0f;

		vu0_Ldv0(&(scratchpad->x[0]));

		for(i=1;i<objs->n_models;i++){
			vu0_Mulv1m2v0();

			vu0_Stm2(&(objs->objs[i].world));

			scratchpad->x[0].vx=objs->def->models[i+1].tx;
			scratchpad->x[0].vy=objs->def->models[i+1].ty;
			scratchpad->x[0].vz=objs->def->models[i+1].tz;

			vu0_Stv1((FVECTOR *)&(objs->objs[i].world.m[3][0]));
			vu0_Ldv0(&(scratchpad->x[0]));
		}
    }

#if 0
    ScratchpadFree(scratchpad);
#else
    ScratchpadFree2();
#endif
}
