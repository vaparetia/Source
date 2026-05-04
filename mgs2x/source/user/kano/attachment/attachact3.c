//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	attachact3.c
		装備品メイン

	1999/11/04 K.Kano
	$Id: attachact3.c,v 1.1.1.3 2002/11/19 11:43:02 Yoshizawa1 Exp $
*/


#include "attachment3.h"


int InitAttachment3(MODEL_PARAMETER3 *p)
{
    p->stockp=0;

	p->first_flag=1;

    if(p->stock_size==0) return 1;

    if((p->stockm
		=(FMATRIX *)GV_Malloc((sizeof(FMATRIX)+sizeof(float))*p->stock_size))==NULL) return 0;
    p->stockh=(float *)(p->stockm+p->stock_size);

    return 1;
}

void InitCalcAttachment3(MODEL_PARAMETER3 *p)
{
    FVECTOR v;
    float vy;
    int i;

    vu0_Ldm0(p->tmat);
    vu0_Ldv0(&(p->x[0]));
    vu0_Ldm1(&(p->rotm));

    vu0_Mulv0m0v0();
    vu0_Mulm2m0m1();
    
    vu0_Stv0(&v);

    vy=v.vy;
    for(i=0;i<p->stock_size;i++){
		vu0_Stm2(&(p->stockm[i]));
		p->stockh[i]=vy;
    }

}

void ExitAttachment3(MODEL_PARAMETER3 *p)
{
    if(p->stockm!=NULL) GV_Free(p->stockm);
}


void MoveAttachment3(DG_OBJS *objs,MODEL_PARAMETER3 *p)
{
    float vy=p->stockh[p->stockp];

#if 0
	objs->flag &= ~(DG_FLAG_INVISIBLE);
	if( p->target->evmobj ){
		objs->flag |= ((p->target->evmobj->flag & DG_EVMOBJ_INVISIBLE)<<4);
	}else{
		objs->flag |= p->target->objs->flag & DG_FLAG_INVISIBLE;
	}
#endif


    /* 古いマトリクスを利用する。*/
    vu0_Ldm0(p->tmat);

    if(p->stock_size==0){
		vu0_Ldm1(&(p->rotm));
		vu0_Ldv1(&(p->x[0]));

		vu0_Mulm2m0m1();
		vu0_Mulv1m0v1();

		vu0_Stm2(&(objs->objs[0].world));
		vu0_Stm2(&(objs->world));
    }
    else{
		fpu_CopyMatrix(&(objs->objs[0].world),&(p->stockm[p->stockp]));
		fpu_CopyMatrix(&(objs->world),&(p->stockm[p->stockp]));

		vu0_Ldm1(&(p->rotm));
		vu0_Ldv1(&(p->x[0]));

		vu0_Mulm2m0m1();
		vu0_Mulv1m0v1();

		vu0_Stm2(&(p->stockm[p->stockp]));
    }

    vu0_Stv1((FVECTOR *)&(objs->objs[0].world.m[3][0]));
    vu0_Stv1((FVECTOR *)&(objs->world.m[3][0]));

    p->stockh[p->stockp]=objs->world.m[3][1];

    if(vy<p->stockh[p->stockp]-CVC2N(2.0f)) vy=p->stockh[p->stockp]-CVC2N(2.0f);
    else if(vy>p->stockh[p->stockp]+CVC2N(2.0f)) vy=p->stockh[p->stockp]+CVC2N(2.0f);

    objs->world.m[3][1]=objs->objs[0].world.m[3][1]=vy;

    p->stockp++;
    if(p->stockp>=p->stock_size) p->stockp=0;

    if(objs->n_models>1){
		FVECTOR vec;
		int i;

		vu0_Ldm2(&(objs->objs[0].world));

		vec.vx=objs->def->models[1].tx;
		vec.vy=objs->def->models[1].ty;
		vec.vz=objs->def->models[1].tz;
		vec.vw=1.0f;

		vu0_Ldv0(&vec);

		for(i=1;i<objs->n_models;i++){
			vu0_Mulv1m2v0();

			vu0_Stm2(&(objs->objs[i].world));

			vec.vx=objs->def->models[i].tx;
			vec.vy=objs->def->models[i].ty;
			vec.vz=objs->def->models[i].tz;

			vu0_Stv1((FVECTOR *)&(objs->objs[i].world.m[3][0]));
			vu0_Ldv0(&vec);
		}
    }
}


void MoveAttachment3A(DG_OBJS *objs,MODEL_PARAMETER3 *p)
{
    /* 古いマトリクスを利用する。*/
    vu0_Ldm0(p->tmat);

    if(p->stock_size==0){
		vu0_Ldm1(&(p->rotm));
		vu0_Ldv1(&(p->x[0]));

		vu0_Mulm2m0m1();
		vu0_Mulv1m0v1();

		vu0_Stm2(&(objs->objs[0].world));
		vu0_Stm2(&(objs->world));
    }
    else{
		fpu_CopyMatrix(&(objs->objs[0].world),&(p->stockm[p->stockp]));
		fpu_CopyMatrix(&(objs->world),&(p->stockm[p->stockp]));

		vu0_Ldm1(&(p->rotm));
		vu0_Ldv1(&(p->x[0]));

		vu0_Mulm2m0m1();
		vu0_Mulv1m0v1();

		vu0_Stm2(&(p->stockm[p->stockp]));
    }

    vu0_Stv1((FVECTOR *)&(objs->objs[0].world.m[3][0]));
    vu0_Stv1((FVECTOR *)&(objs->world.m[3][0]));

    p->stockp++;
    if(p->stockp>=p->stock_size) p->stockp=0;

    if(objs->n_models>1){
		FVECTOR vec;
		int i;

		vu0_Ldm2(&(objs->objs[0].world));

		vec.vx=objs->def->models[1].tx;
		vec.vy=objs->def->models[1].ty;
		vec.vz=objs->def->models[1].tz;
		vec.vw=1.0f;

		vu0_Ldv0(&vec);

		for(i=1;i<objs->n_models;i++){
			vu0_Mulv1m2v0();

			vu0_Stm2(&(objs->objs[i].world));

			vec.vx=objs->def->models[i+1].tx;
			vec.vy=objs->def->models[i+1].ty;
			vec.vz=objs->def->models[i+1].tz;

			vu0_Stv1((FVECTOR *)&(objs->objs[i].world.m[3][0]));
			vu0_Ldv0(&vec);
		}
    }
}
