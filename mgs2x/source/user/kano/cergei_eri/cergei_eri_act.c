//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	cergei_eri_act.c
		セルゲイの襟の動き

	2001/03/19 K.Kano
	$Id: cergei_eri_act.c,v 1.1.1.3 2002/11/19 11:43:06 Yoshizawa1 Exp $
*/


#include "cergei_eri.h"


void CergeiEriFirstCalc(CERGEI_ERI_WORK *p)
{
	FMATRIX *evm_pm;
	int i;

	for(i=0;i<p->n_objs;i++){
		evm_pm=p->target->matrix[p->target->use_buffer]+p->parent[i];

		vu0_Ldv0(&(p->prt[i]));
		vu0_Ldv1(&(p->t[i]));
		vu0_Ldv2(&(p->lastobjx[i]));

		vu0_Ldm0(evm_pm);

		vu0_Mulv0m0v0();

#ifdef BP_PSX2_ASM
		asm volatile ("vmove.xyzw	vf7,vf1");
#else
		fpu_CopyVector( (FVECTOR*)vu0_Ptrm0()->m[3], &REG_vf1 ) ;
#endif
		vu0_Mulv1m0v1();

#ifdef BP_PSX2_ASM
		asm volatile ("vmove.xyzw	vf7,vf2");
#else
		fpu_CopyVector( (FVECTOR*)vu0_Ptrm0()->m[3], &REG_vf2 ) ;
#endif
		vu0_Stv1(&(p->px[i]));

		vu0_Mulv2m0v2();

		vu0_Stm0(&(p->pwm[i]));

		vu0_Stv2(&(p->x[i]));
	}

	fpu_CopyVector(&(p->root_x),(FVECTOR *)&(p->target->world.m[3][0]));
}

int InitCergeiEri(CERGEI_ERI_WORK *p)
{
	int i;

	for(i=0;i<p->n_objs;i++){
		p->parent[i]=p->target->def->skeleton[p->tobjnum[i]].parent;

		p->rt[i].vx=-p->target->def->skeleton[p->tobjnum[i]].rt_tx;
		p->rt[i].vy=-p->target->def->skeleton[p->tobjnum[i]].rt_ty;
		p->rt[i].vz=-p->target->def->skeleton[p->tobjnum[i]].rt_tz;
		p->rt[i].vw=1.0f;

		p->t[i].vx=p->target->def->skeleton[p->tobjnum[i]].tx;
		p->t[i].vy=p->target->def->skeleton[p->tobjnum[i]].ty;
		p->t[i].vz=p->target->def->skeleton[p->tobjnum[i]].tz;
		p->t[i].vw=1.0f;

		p->prt[i].vx=p->target->def->skeleton[p->parent[i]].rt_tx;
		p->prt[i].vy=p->target->def->skeleton[p->parent[i]].rt_ty;
		p->prt[i].vz=p->target->def->skeleton[p->parent[i]].rt_tz;
		p->prt[i].vw=1.0f;

		fpu_ClearVector(&(p->q[i]));
		p->q[i].vw=1.0f;
		fpu_ClearVector(&(p->v[i]));
	}

    return 1;
}

void ExitCergeiEri(CERGEI_ERI_WORK *p)
{
}

void MoveCergeiEri(CERGEI_ERI_WORK *p)
{
	struct _scratchpad {
		FVECTOR wind,*pwind;
		FVECTOR px,x,v;
		FVECTOR q;
		FVECTOR tvec;
		FVECTOR mov;
		FMATRIX pwm,lm;
		FMATRIX *evm_m,*evm_pm;
		int flag;
	} *scratchpad=(struct _scratchpad *)SCRPAD_ADDR;

	int i;

	vu0_Ldv0((FVECTOR *)&(p->target->world.m[3][0]));
	vu0_Ldv1(&(p->root_x));

	scratchpad->pwind=&(scratchpad->wind);
	scratchpad->flag=p->flag;

	/* モデル全体の移動値を算出 */
	vu0_Subv1v0v1();

#ifdef BP_PSX2_ASM
	asm volatile ("
	qmtc2.ni	%0,vf16
	qmtc2.ni	%1,vf17
	vmulx.xz	vf2,vf2,vf16x
	vmulx.y		vf2,vf2,vf17x
	" : : "r"(p->mov_rate),"r"(p->ymov_rate) );
#else
	REG_vf2.vx *= p->mov_rate ;
	REG_vf2.vz *= p->mov_rate ;
	REG_vf2.vy *= p->ymov_rate ;
#endif

	vu0_Stv0(&(p->root_x));
	vu0_Stv1(&(scratchpad->mov));


	/* 風の取得 */
	if(!OK_GetLocalWind((FVECTOR *)&(p->target->world.m[3][0]),&(scratchpad->wind))){
		scratchpad->pwind=&G_wind;
	}

	for(i=0;i<p->n_objs;i++){
		scratchpad->evm_pm=p->target->matrix[p->target->use_buffer]+p->parent[i];
		scratchpad->evm_m=p->target->matrix[p->target->use_buffer]+p->tobjnum[i];

		vu0_Clrv0();

		vu0_Ldv1(scratchpad->pwind);
		vu0_Ldv2(&(p->v[i]));

		vu0_Mulv1a(p->pa);
		vu0_Mulv2a(p->k);

		vu0_Addv0v1();

		/* m0 ... 現在のEVM親マトリクスから、マトリクスを算出する */
		vu0_Ldv1(&(p->prt[i]));
		vu0_Ldm0(scratchpad->evm_pm);

		vu0_Addv0v2();

		vu0_Mulv1m0v1();

#ifdef DEBUG_MODE
		{
			FVECTOR v;
			vu0_Stv0(&v);
			// printf("%d  %f %f %f %f\n",i,v.vx,v.vy,v.vz,v.vw);
		}
#endif

		/* 現在位置に力を演算 */


		/* 現クォータニオンから、ばねの戻す力を算出 */
		/* forceの加算 */

		/* 力のかかり方は、現在の回転を打ち消すようにかからなければ
		   ならない。そこで、現在の回転量と回転向きが必要となる。
		   回転向き、回転量は、ともにローカルクオータニオン
		   そのものの事であるので、ワークとしてlrotsを用意し、
		   前回の回転量を保存しておく事とする。*/

		/* cosine = (work->rots+i)->vw;             */
		/* sine^2 = 1-cosine^2                      */

		/* f = ( lq * t ) * sine^2 * a    ( )内は外積
		   lq ... ローカルクオータニオンの軸方向ベクトル
		   t  ... 前回の自位置と親位置の差分( = x - xp )
		   a  ... 係数 */

		{
			float len;
			float sine,cosine;
			FVECTOR *nx,*px;

			nx=&(p->x[i]);
			px=&(p->px[i]);

#ifdef BP_PSX2_ASM
			asm volatile ("vmove.xyzw	vf7,vf2");
#else
			fpu_CopyVector( (FVECTOR*)vu0_Ptrm0()->m[3], &REG_vf2 ) ;
#endif
			vu0_Ldv1(nx);
			vu0_Ldv2(px);

			vu0_Subv1v2();

			vu0_Ldv2(&(p->q[i]));
			vu0_Ldm1(&(p->pwm[i]));
			vu0_Setv2w0();

			cosine=p->q[i].vw;
			sine=fpu_Sqrt(1.0f-cosine*cosine);

			vu0_Mulv2m1v2();

			sine*=p->deg_param;

			vu0_OuterProductv1v2();

			len=vu0_VectorLength2v1();
			len=fpu_Rsqrt(len,sine);
			vu0_Mulv1a(len);

			// printf("llll = %f\n",len);
		}


		vu0_Ldv2(&(p->t[i]));
		vu0_Stm0(&(scratchpad->pwm));

		vu0_Mulv2m0v2();
		vu0_Addv0v1();

		vu0_Ldv1(&(p->lastobjx[i]));

#ifdef BP_PSX2_ASM
		asm volatile ("vmove.xyz vf7,vf3");
#else
		fpu_CopyVector( (FVECTOR*)vu0_Ptrm0()->m[3], &REG_vf3 ) ;
#endif
		vu0_Stv2((FVECTOR *)&(scratchpad->pwm.m[3][0]));
		vu0_Stv2(&(scratchpad->px));

		vu0_Ldv2(&(p->x[i]));

		vu0_Mulv1m0v1();

		vu0_Subv1v2();

		/* 移動による影響を抑える */

#ifdef BP_PSX2_ASM
		asm volatile ("
	    qmtc2.ni	%0,vf16
	    qmtc2.ni	%1,vf17
	    vmulx.xz	vf2,vf2,vf16x
	    vmulx.y		vf2,vf2,vf17x
		" : : "r"(p->mov_rate),"r"(p->ymov_rate) );
#else
		REG_vf2.vx *= p->mov_rate ;
		REG_vf2.vz *= p->mov_rate ;
		REG_vf2.vy *= p->ymov_rate ;
#endif
		vu0_Ldv1(&(scratchpad->mov));

		vu0_Addv1v2();

#ifdef DEBUG_MODE
		{
			FVECTOR v;
			vu0_Stv0(&v);
			// printf("%d  %f %f %f %f\n",i,v.vx,v.vy,v.vz,v.vw);
		}
#endif

		vu0_Ldv2(&(p->v[i]));

		vu0_Stv1(&(p->x[i]));
		// vu0_Ldv1(&(p->x[i]));

		vu0_Addv2v0();
		vu0_Addv1v2();

		vu0_Stv1(&(scratchpad->x));

		if(scratchpad->flag & CERGEI_ERI_FLAG_CALC_DISABLE){
			fpu_ClearVector(&(scratchpad->q));
			scratchpad->q.vw=1.0f;
		}
		else{
			UTL_MakeQuatM(&(scratchpad->q),&(scratchpad->px),&(scratchpad->x),
						  &(scratchpad->pwm),&(p->lastobjx[i]));

#if 1
			if(scratchpad->q.vw<p->cos_limit){
				float sin=1.0f-scratchpad->q.vw*scratchpad->q.vw;
				float param;

				param=fpu_Rsqrt(sin,p->sin_limit);

				scratchpad->q.vx*=param;
				scratchpad->q.vy*=param;
				scratchpad->q.vz*=param;
				scratchpad->q.vw=p->cos_limit;
			}
#endif
		}

		MT_QuatToMat(&(scratchpad->lm),&(scratchpad->q));


		// printf("a = %f\n",a);

#if 1

		vu0_Ldm1(&(scratchpad->pwm));
		vu0_Ldm0(&(scratchpad->lm));

		vu0_Ldv0(&(p->lastobjx[i]));
		vu0_Ldv1(&(p->x[i]));
		vu0_Ldv2(&(p->rt[i]));

		vu0_Mulm2m0m1();


#ifdef BP_PSX2_ASM
		asm volatile ("vmove.xyzw	vf15,vf11");
#else
		fpu_CopyVector( (FVECTOR*)REG_m2.m[3],(FVECTOR*)REG_m1.m[3] ) ;
#endif

		vu0_Mulv0m2v0();
		vu0_Mulv2m2v2();

		vu0_Subv1v0v1();

		/* evmのマトリクスの形式に変更 */
		vu0_Stm2(scratchpad->evm_m);
		vu0_Stv2((FVECTOR *)&(scratchpad->evm_m->m[3][0]));

		vu0_Stv0(&(p->x[i]));
		vu0_Stv1(&(p->v[i]));

#endif


		fpu_CopyMatrix(&(p->pwm[i]),&(scratchpad->pwm));
		fpu_CopyVector(&(p->q[i]),&(scratchpad->q));
		fpu_CopyVector(&(p->px[i]),&(scratchpad->px));

		// printf("%d  %f %f %f %f\n",i,p->v[i].vx,p->v[i].vy,p->v[i].vz,p->v[i].vw);
	}
}
