//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	attachact.c
		装備品メイン

	1999/11/04 K.Kano
	$Id: attachact.c,v 1.1.1.3 2002/11/19 11:43:02 Yoshizawa1 Exp $
*/


#include "attachment.h"


int InitAttachment(DG_OBJS *objs,MODEL_PARAMETER *p)
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
    p->lastobjx.vw=0.0f;

    vu0_Ldv1(&(p->lastobjx));
    vu0_Stv0(&(p->mval.x[0]));
    vu0_Addv0v1();
    vu0_Stv0(&(p->mval.x[1]));

	p->first_flag=1;

    return 1;
}

void InitCalcAttachment(MODEL_PARAMETER *p)
{
    vu0_Ldv0(&(p->x[0]));
    vu0_Ldm0(p->tmat);

    vu0_Mulv0m0v0();

    vu0_Ldv1(&(p->lastobjx));

    vu0_Addv1v0();

    vu0_Stv0(&(p->mval.x[0]));
    vu0_Stv1(&(p->mval.x[1]));
}

void MoveAttachment(DG_OBJS *objs,MODEL_PARAMETER *p)
{
    struct _scratchpad {
		FMATRIX rmat;
		FMATRIX tmat;

		FVECTOR root,inv_root;

		FVECTOR f[2];
		FVECTOR t[2];
		FVECTOR ma[2];
		FVECTOR x[2];
		FVECTOR abs_rots;
    };
    struct _scratchpad *scratchpad;
    int flag=1;

#if 0
	objs->flag &= ~(DG_FLAG_INVISIBLE);
	if( p->target->evmobj ){
		objs->flag |= ((p->target->evmobj->flag & DG_EVMOBJ_INVISIBLE)<<4);
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

    /* ワールド座標の計算、同時に前回の速度を計算 */
    vu0_Ldm0(p->tmat);
    vu0_Ldv0(&(p->x[0]));
    vu0_Ldm1(&(p->rotm));

    vu0_Mulv0m0v0();

#ifdef BP_PSX2_ASM
    asm volatile ("vadd.xyz	vf7,vf7,vf1");
#else
	{
		extern FVECTOR REG_vf1 ;
		fpu_AddVectors( (FVECTOR *)vu0_Ptrm0()->m[3],(FVECTOR *)vu0_Ptrm0()->m[3], &REG_vf1 ) ;
	}
#endif

    vu0_Mulm2m0m1();

    vu0_Stm2(&(scratchpad->rmat));
    vu0_Stm2(&(p->root));

    MT_MatToQuat(&(scratchpad->root),&(scratchpad->rmat));
    MT_QuatInverse(&(scratchpad->inv_root),&(scratchpad->root));


    /* 力の計算 */
    vu0_Clrv1();

    /* 空気抵抗 */
    vu0_Ldv2(&(p->mval.v[1]));

    /* 重力 */
    {
//		static const FVECTOR g={ 0.0f, P_GRAVITY, 0.0f, 0.0f, }; //BP JG - runtime 50/60
		static FVECTOR g={ 0.0f, 0.0f, 0.0f, 0.0f, };
      g.y = P_GRAVITY;
		vu0_Ldv0((FVECTOR *)&g);
    }
    vu0_Mulv2a(p->k);

    vu0_Addv1v0();

    vu0_Addv1v2();

    /* VU0が使われるため値を保存する */
    vu0_Stv1(&(scratchpad->f[1]));

    /* 当たり判定 */
    if(p->target){
		FVECTOR x;

		if(CalcObjsCollision(&x,&(p->mval.x[1]),p->target->objs,p->param_oval)){
			float l;

#if 1
			vu0_Ldv0(&x);
			vu0_Ldv1(&(p->mval.x[1]));
			vu0_Stv0(&(p->mval.x[1]));
			vu0_Subv0v1();
			vu0_Ldv2(&(p->mval.v[1]));
			l=vu0_VectorLength2v0();
			l=fpu_Rsqrt(l,1.0f);
			vu0_Mulv0a(l);
			l=vu0_InnerProductv0v2();
			vu0_Ldv1(&(scratchpad->f[1]));
			vu0_Mulv0a(l*p->we);
			vu0_Addv1v0();
			vu0_Stv1(&(scratchpad->f[1]));
#else
			fpu_CopyVector(&(p->mval.x[1]),&x);
#endif

			flag=0;
		}
    }

    /* 加速度の算出 */
    {
		float s,t;

		vu0_Ldv0(&(p->mval.x[1]));
		vu0_Ldv1(&(p->mval.x[0]));

		fpu_CopyVector(&(scratchpad->x[0]),(FVECTOR *)&(scratchpad->rmat.m[3][0]));
		fpu_CopyVector(&(p->mval.x[0]),(FVECTOR *)&(scratchpad->rmat.m[3][0]));

		vu0_Subv0v1();

		vu0_Ldv2(&(scratchpad->f[1]));

		/* 正規化 + 内積 + 張力算出 */
		t=vu0_VectorLength2v0();
		s=vu0_InnerProductv0v2();

		vu0_Mulv0a(s/t);

		vu0_Subv2v0();

		vu0_Stv0(&(scratchpad->t[0]));
		vu0_Stv2(&(scratchpad->ma[1]));
    }

    /* 速度、次位置の算出 */
    vu0_Ldv2(&(scratchpad->ma[1]));
    vu0_Ldv1(&(p->mval.v[1]));
    vu0_Ldv0(&(p->mval.x[1]));

    vu0_Addv1v2();
    vu0_Addv0v1();

    vu0_Stv0(&(scratchpad->x[1]));


    /* 次位置から、次abs_rotsを算出 */
    {
		FVECTOR vec;
		FVECTOR q;

		UTL_MakeQuatM(&q,&(scratchpad->x[0]),&(scratchpad->x[1]),
					  &(scratchpad->rmat),&(p->lastobjx));

		MT_QuatMul(&q,&q,&(scratchpad->root));
		MT_QuatMul(&(scratchpad->abs_rots),&(scratchpad->inv_root),&q);

		switch(p->n_axis){
		case 0:
		case 2:
		case 3:
			break;
		case 1:
			fpu_CopyVector(&vec,&(p->axis));
			MT_QuatSeparate(&(scratchpad->abs_rots),&q,&vec);
			fpu_CopyVector(&(scratchpad->abs_rots),&vec);
			break;
		}

		if(flag){
			if(scratchpad->abs_rots.vw<p->cos_limit){
				float s;
				s=fpu_Rsqrt(1.0f-scratchpad->abs_rots.vw*scratchpad->abs_rots.vw,
							p->sin_limit);
				scratchpad->abs_rots.vx*=s;
				scratchpad->abs_rots.vy*=s;
				scratchpad->abs_rots.vz*=s;
				scratchpad->abs_rots.vw=p->cos_limit;
			}
		}
    }

    MT_QuatToMat(&(scratchpad->tmat),&(scratchpad->abs_rots));

    vu0_Ldm0(&(scratchpad->rmat));
    vu0_Ldm1(&(scratchpad->tmat));

    vu0_Mulm2m0m1();

    vu0_Ldv1(&(p->mval.x[0]));
#ifdef BP_PSX2_ASM
    asm volatile ("vmove.xyzw	vf1,vf15");
#else
	{
		extern FVECTOR REG_vf1 ;
		REG_vf1 = *(FVECTOR*)vu0_Ptrm2()->m[3] ;
	}
#endif

    vu0_Stm2(&(objs->objs[0].world));
    vu0_Stm2(&(objs->world));
    vu0_Stv0(&(p->mval.x[0]));

    vu0_Subv0v1();

    vu0_Stv0(&(p->mval.v[0]));
    vu0_Ldv0(&(p->lastobjx));
    vu0_Setv0w1();

    vu0_Mulv0m2v0();
    vu0_Ldv1(&(p->mval.x[1]));
    vu0_Stv0(&(p->mval.x[1]));

    vu0_Subv0v1();

    vu0_Stv0(&(p->mval.v[1]));

    if(objs->n_models>1){
		int i;

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
