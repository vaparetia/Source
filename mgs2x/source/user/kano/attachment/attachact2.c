//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	attachact2.c
		装備品メイン

	1999/11/04 K.Kano
	$Id: attachact2.c,v 1.1.1.3 2002/11/19 11:43:02 Yoshizawa1 Exp $
*/


#include "attachment2.h"


int InitAttachment2(DG_OBJS *objs,MODEL_PARAMETER2 *p)
{
    DG_MDL *mdl=objs->def->models;

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
    p->lastobjx.vw=0.0f;

	p->first_flag=1;

    return 1;
}

void MoveAttachment2(DG_OBJS *objs,MODEL_PARAMETER2 *p)
{
    struct _scratchpad {
		FMATRIX rmat;

		FVECTOR root;
		FVECTOR q;

		FVECTOR x[2];
    };
    struct _scratchpad *scratchpad;

#if 0
	objs->flag &= ~(DG_FLAG_INVISIBLE);
	if( p->target->evmobj ){
		objs->flag |= (p->target->evmobj->flag & DG_EVMOBJ_INVISIBLE)<<4;
	}else{
		objs->flag |= p->target->objs->flag & DG_FLAG_INVISIBLE;
	}
#endif

    /* スクラッチパッドのみで納まるかあやしいので、
       管理関数を介するようにした。*/
    ScratchpadManInit();

#if 0
    scratchpad = (struct _scratchpad *)ScratchpadAlloc(sizeof(struct _scratchpad));
#else
    scratchpad = (struct _scratchpad *)ScratchpadAlloc2(sizeof(struct _scratchpad));
#endif

    /* 現在位置と角度を算出 */
    vu0_Ldm0(p->tmat);
    vu0_Ldv0(&(p->x[0]));
    vu0_Ldm1(&(p->rotm));

    vu0_Mulv0m0v0();
#ifdef BP_PSX2_ASM
    asm volatile ("vmove.xyz	vf7,vf1");
#else
	{
		extern FVECTOR REG_vf1 ;
		*(FVECTOR*)vu0_Ptrm0()->m[3] = REG_vf1 ;
	}
#endif

    vu0_Stv0(&(scratchpad->x[0]));

    vu0_Mulm2m0m1();

    vu0_Ldm0(p->tmat2);
    vu0_Ldv0(&(p->x[1]));

    vu0_Stm2(&(scratchpad->rmat));

    vu0_Mulv0m0v0();

    vu0_Stm2(&(objs->world));

    vu0_Stv0(&(scratchpad->x[1]));


    MT_MatToQuat(&(scratchpad->root),&(scratchpad->rmat));

    UTL_MakeQuatM(&(scratchpad->q),&(scratchpad->x[0]),&(scratchpad->x[1]),
				  &(scratchpad->rmat),&(p->lastobjx));

	/* 計算値の保存 */
	fpu_CopyVector(&(p->now_q),&(scratchpad->q));

    MT_QuatMul(&(scratchpad->q),&(scratchpad->q),&(scratchpad->root));
    MT_QuatToMat(&(objs->objs[0].world),&(scratchpad->q));
    fpu_CopyVector((FVECTOR *)&(objs->objs[0].world.m[3][0]),&(scratchpad->x[0]));

    //fpu_CopyMatrix(&(objs->world),&(objs->objs[0].world));

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
