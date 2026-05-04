//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	rope3.c
		紐形式のモデルを、風やモデルの動きに対応させて動かす
		片側(OBJ 0)のみ固定ロープの共通関数部

	1999/09/16 K.Kano
	$Id: rope3.c,v 1.1.1.3 2002/11/19 11:43:35 Yoshizawa1 Exp $
*/


#include "rope.h"


/* 片側(OBJ 0)のみ固定 */
void MoveRope3(DG_OBJS *objs,HZX_GROUP_ID hzx_id,MODEL_PARAMETER *p)
{
    DG_MDL *mdl=objs->def->models;
    int i;
    int size=objs->def->n_models;

    struct _scratchpad {
		FMATRIX rmat;

		FVECTOR tvec0,tvec1,tvec2,tvec3;
		FVECTOR root;

		FMATRIX *inv_m;

		FVECTOR *f;
		FVECTOR *t;
		FVECTOR *ma;
		FVECTOR *x;
    };
    struct _scratchpad *scratchpad;

#if 0
	{ // TEST
		static int flag=0;
		// if(!flag){
			if(p->tmat!=NULL){
				printf("rope first = %f %f %f\n",
						p->tmat->m[3][0],p->tmat->m[3][1],p->tmat->m[3][2]);
			}
			else{
				printf("tmat is NULL\n");
			}
			flag=1;
		// }
	}
#endif
	
    /* スクラッチパッドのみで納まるかあやしいので、
       管理関数を介するようにした。*/
    ScratchpadManInit();

#if 0
    scratchpad = (struct _scratchpad *)ScratchpadAlloc(sizeof(struct _scratchpad));

    if(p->target){
		int tsize=p->target->objs->n_models;
		scratchpad->inv_m  = (FMATRIX *)ScratchpadAlloc(sizeof(FMATRIX)*tsize);
    }

    scratchpad->f      = (FVECTOR *)ScratchpadAlloc(sizeof(FVECTOR)*size*4);
    scratchpad->t      = scratchpad->f+size;
    scratchpad->ma     = scratchpad->t+size;
    scratchpad->x      = scratchpad->ma+size;
#elif 1
    scratchpad = (struct _scratchpad *)ScratchpadAlloc2(sizeof(struct _scratchpad));

    if(p->target){
		int tsize=p->target->objs->n_models;
		scratchpad->inv_m  = (FMATRIX *)ScratchpadAlloc2(sizeof(FMATRIX)*tsize);
    }

    scratchpad->f      = (FVECTOR *)ScratchpadAlloc2(sizeof(FVECTOR)*size*4);
    scratchpad->t      = scratchpad->f+size;
    scratchpad->ma     = scratchpad->t+size;
    scratchpad->x      = scratchpad->ma+size;
#else
    scratchpad = (struct _scratchpad *)GV_Malloc(sizeof(struct _scratchpad));

    if(p->target){
		int tsize=p->target->objs->n_models;
		scratchpad->inv_m  = (FMATRIX *)GV_Malloc(sizeof(FMATRIX)*tsize);
    }

    scratchpad->f      = (FVECTOR *)GV_Malloc(sizeof(FVECTOR)*size*4);
    scratchpad->t      = scratchpad->f+size;
    scratchpad->ma     = scratchpad->t+size;
    scratchpad->x      = scratchpad->ma+size;
#endif

    if(p->tmat){
		vu0_Ldm0(p->tmat);
		vu0_Ldm1(&(p->root));

		vu0_Mulm2m0m1();

		vu0_Stm2(&(scratchpad->rmat));
    }
    else{
		fpu_CopyMatrix(&(scratchpad->rmat),&(p->root));
    }

    MT_MatToQuat(&(scratchpad->root),&(scratchpad->rmat));


    if(p->collision_flag && p->target){
		DG_OBJS *tobjs=p->target->objs;
		int tsize=p->target->objs->n_models;

		for(i=0;i<tsize;i++){
#if 0
			InverseMatrix(scratchpad->inv_m+i,&(tobjs->objs[i].world));
#elif 0
			sceVu0InversMatrix(*(sceVu0FMATRIX *)(scratchpad->inv_m+i),
							   *(sceVu0FMATRIX *)&(tobjs->objs[i].world));
#else
			FastInverseMatrix(scratchpad->inv_m+i,&(tobjs->objs[i].world));
#endif
		}
    }

#if 1
	/* 移動無視の際に参照する座標を変更し、
	   OBJECTの中心にする。*/

	if(p->target!=NULL){

#if 0
		vu0_Ldv0((FVECTOR *)&(p->target->objs->world.m[3][0]));
		vu0_Ldv1(&(p->target_pos));

		vu0_Subv0v1();

		fpu_CopyVector(&(p->target_pos),
					   (FVECTOR *)&(p->target->objs->world.m[3][0]));
#else
		if(p->mode){
			/* 髪の毛モード */
			vu0_Ldv0(&(objs->objs[1].trans));
			vu0_Ldm0(&(scratchpad->rmat));
			vu0_Mulv0m0v0();

			vu0_Ldv1(&(p->target_pos));

			vu0_Stv0(&(p->target_pos));

			vu0_Subv0v1();
		}
		else{
			/* バンダナモード */
			vu0_Ldv0((FVECTOR *)&(scratchpad->rmat.m[3][0]));
			vu0_Ldv1(&(p->target_pos));

			vu0_Subv0v1();

			fpu_CopyVector(&(p->target_pos),
						   (FVECTOR *)&(scratchpad->rmat.m[3][0]));
		}
#endif

		vu0_Ldv1(&(p->mval.x[0]));


#ifdef BP_PSX2_ASM
		asm volatile ("
		qmtc2.ni	%0,vf16
		qmtc2.ni	%1,vf17
		vmulx.xz	vf1,vf1,vf16x
		vmulx.y		vf1,vf1,vf17x
		" : : "r"(p->mov_rate),"r"(p->ymov_rate) );
#else
		{
			extern FVECTOR REG_vf1 ;
			REG_vf1.vx = REG_vf1.vx * p->mov_rate  ;
			REG_vf1.vy = REG_vf1.vy * p->ymov_rate ;
			REG_vf1.vz = REG_vf1.vz * p->mov_rate  ;
		}
#endif

		vu0_Setv2w1();

		for(i=0;i<size;i++){
			vu0_Addv2v0v1();
			vu0_Stv2(&(p->mval.x[i]));
			vu0_Stv2(&(p->mval.x[i]));
			vu0_Ldv1(&(p->mval.x[i+1]));
		}
	}
#endif

    /* 力の計算 */
    for(i=1;i<size;i++){
		FVECTOR wind,*pwind=&wind;

		if(!OK_GetLocalWind(&(p->mval.x[i]),&wind)){
			pwind=&G_wind;
		}

		vu0_Clrv1();
		vu0_Ldm0(&(scratchpad->rmat));

		/* 基準張力 */
		vu0_Ldv1(&(p->mval.base_t[i]));

		/* 重力 */
		{

//		   static const FVECTOR g={ 0.0f, P_GRAVITY, 0.0f, 0.0f, }; //BP JG - runtime 50/60
			static FVECTOR g={ 0.0f, 0.0f, 0.0f, 0.0f, };
         g.y = P_GRAVITY;
			vu0_Ldv0((FVECTOR *)&g);
		}

		vu0_Mulv1m0v1();

		vu0_Mulv0a(p->m);
		vu0_Mulv1a(p->m);

		/* 風 */
		vu0_Ldv2(pwind);

		vu0_Addv1v0();

		vu0_Mulv2a(p->pa);

    	/* 空気抵抗 */
		vu0_Ldv0(&(p->mval.v[i]));

		vu0_Addv1v2();

		vu0_Mulv0a(p->k);
		vu0_Addv1v0();

		/* VU0が使われるため値を保存する */
		vu0_Stv1(&(scratchpad->f[i]));

		/* 当たり判定 */
        if(p->collision_flag){
			if(p->target){
				float l;

				if(p->boundmodel){
					if(!CalcObjsCollisionWithInvM2(&(scratchpad->tvec0),&(p->mval.x[i]),
												   p->boundmodel,p->target->objs,p->param_oval,
												   scratchpad->inv_m)) goto next;
				}
				else{
					if(!CalcObjsCollisionWithInvM(&(scratchpad->tvec0),&(p->mval.x[i]),
												  p->target->objs,p->param_oval,
												  scratchpad->inv_m)) goto next;
				}

				// printf("Check 1\n");

				vu0_Ldv0(&(scratchpad->tvec0));
				vu0_Ldv1(&(p->mval.x[i]));
				vu0_Stv0(&(p->mval.x[i]));
				vu0_Subv0v1();
				vu0_Ldv2(&(p->mval.v[i]));
				l=vu0_VectorLength2v0();
				l=fpu_Rsqrt(l,1.0f);
				vu0_Mulv0a(l);
				l=vu0_InnerProductv0v2();
				vu0_Ldv1(&(scratchpad->f[i]));
				vu0_Mulv0a(l*p->m*p->we);
				vu0_Addv1v0();
				vu0_Stv1(&(scratchpad->f[i]));

			next:
#if 0
				;
#else
				if(HZX_LevelHazardCheck(hzx_id,&(p->mval.x[i]),
										HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR,0)){

					float h;

					h=HZX_GetFloorLevel()+(float)(p->wl);

					/* 床から2mまでは、床に合わせようとする。*/
					if(p->mval.x[i].vy<h && p->mval.x[i].vy>h-CVM2N(2.0f)){
						static const FVECTOR v={ 0.0f,1.0f,0.0f,0.0f, };

						/* 保存した値を再格納 */
						vu0_Ldv1(&(scratchpad->f[i]));

						p->mval.x[i].vy=h;
						vu0_Ldv0(&v);
						vu0_Mulv2v0a(p->mval.v[i].vy*p->m*p->we);
						vu0_Addv1v2();

						vu0_Stv1(&(scratchpad->f[i]));
					}
				}
#endif
			}
			else{
				int fhzd=HZX_NearHazardCheck(hzx_id,&(p->mval.x[i]),
											 p->wl, /* 検出半径 */
											 HZX_CHK_ALL,0,
											 p->wl /* 反発半径 */);

				float h;

				if(fhzd){
					float s,t;

					HZX_GetReactVector(&(scratchpad->tvec0));

					/* 保存した値を再格納 */
					vu0_Ldv1(&(scratchpad->f[i]));

					vu0_Ldv0(&(scratchpad->tvec0));
					vu0_Ldv2(&(p->mval.x[i]));
					s=vu0_VectorLength2v0();
					vu0_Addv2v0();
					vu0_Stv2(&(p->mval.x[i]));

					vu0_Ldv2(&(p->mval.v[i]));
					t=vu0_InnerProductv0v2();
					if ( s==0.0f ) s =0.00001f ; // T.Morita Added 2002.02.25
					vu0_Mulv2v0a(t/s*p->m*p->we);
					vu0_Addv1v2();

					vu0_Stv1(&(scratchpad->f[i]));
				}

				HZX_LevelHazardCheck(hzx_id,&(p->mval.x[i]),
									 HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR,0);
				h=HZX_GetFloorLevel()+p->wl;
				if(p->mval.x[i].vy<h){
					static const FVECTOR v={ 0.0f,1.0f,0.0f,0.0f, };

					/* 保存した値を再格納 */
					vu0_Ldv1(&(scratchpad->f[i]));

					p->mval.x[i].vy=h;
					vu0_Ldv0(&v);
					vu0_Mulv2v0a(p->mval.v[i].vy*p->m*p->we);
					vu0_Addv1v2();

					vu0_Stv1(&(scratchpad->f[i]));
				}
			}
		}
    }

    /* 加速度の算出 */
    {
		float s,t;

		// printf("Check 2\n");

		i=size-1;

		vu0_Ldv0(&(p->mval.x[i]));
		vu0_Ldv1(&(p->mval.x[i-1]));

		fpu_CopyVector(&(scratchpad->x[0]),(FVECTOR *)&(scratchpad->rmat.m[3][0]));
		fpu_CopyVector(&(p->mval.x[0]),(FVECTOR *)&(scratchpad->rmat.m[3][0]));

		vu0_Subv0v1();

		vu0_Ldv2(&(scratchpad->f[i]));

		/* 正規化 + 内積 + 張力算出 */
		t=vu0_VectorLength2v0();
		s=vu0_InnerProductv0v2();
		if ( t==0.0f ) t=0.00001f ; // T.Morita Added 2002.02.25
		vu0_Mulv0a(s/t);

		vu0_Subv2v0();

		for(i--;i>0;i--){
			vu0_Stv0(&(scratchpad->t[i]));
			vu0_Stv2(&(scratchpad->ma[i+1]));

			vu0_Ldv2(&(scratchpad->f[i]));

			/* 張力付加 */
			vu0_Addv2v0();

			vu0_Cpv1v0();
			vu0_Ldv1(&(p->mval.x[i-1]));

			vu0_Subv0v1();

			/* 正規化 + 内積 + 張力算出 */
			t=vu0_VectorLength2v0();
			s=vu0_InnerProductv0v2();
			if ( t==0.0f ) t=0.00001f ; // T.Morita Added 2002.02.25
			vu0_Mulv0a(s/t);

			vu0_Subv2v0();
		}

		vu0_Stv0(&(scratchpad->t[0]));
		vu0_Stv2(&(scratchpad->ma[1]));
    }


#if 0
    for(i=0;i<size;i++){
		printf("F%d = %f %f %f\n",i,
			   scratchpad->f[i].vx,scratchpad->f[i].vy,scratchpad->f[i].vz);
    }
#endif

#if 0
    for(i=0;i<size-1;i++){
		printf("T%d = %f %f %f\n",i,
			   scratchpad->t[i].vx,scratchpad->t[i].vy,scratchpad->t[i].vz);
    }
#endif

#if 0
    for(i=0;i<size;i++){
		printf("ma%d = %f %f %f\n",i,
			   scratchpad->ma[i].vx,scratchpad->ma[i].vy,scratchpad->ma[i].vz);
    }
#endif

#if 0
    for(i=0;i<size;i++){
		printf("V%d = %f %f %f\n",i,
			   p->mval.v[i].vx,p->mval.v[i].vy,p->mval.v[i].vz);
    }
#endif

    // printf("Check 3\n");

    /* 速度、次位置の算出 */
    for(i=size-1;i>0;i--){
		vu0_Ldv2(&(scratchpad->ma[i]));
		vu0_Ldv1(&(p->mval.v[i]));

		vu0_Mulv2a(p->inv_m);

		vu0_Ldv0(&(p->mval.x[i]));

		vu0_Addv1v2();
		vu0_Addv0v1();

		vu0_Stv0(&(scratchpad->x[i]));
    }
    fpu_CopyVector(&(scratchpad->x[0]),(FVECTOR *)&(scratchpad->rmat.m[3][0]));
    fpu_CopyVector(&(p->mval.x[0]),(FVECTOR *)&(scratchpad->rmat.m[3][0]));

#if 1
    /* OBJ 0は、動きも固定 */
    if(p->mode!=0){
		FVECTOR vec;

		// printf("Check 4\n");

		vec.vx=objs->def->models[1].tx;
		vec.vy=objs->def->models[1].ty;
		vec.vz=objs->def->models[1].tz;
		vec.vw=1.0f;

		vu0_Ldv0(&vec);
		vu0_Ldm0(&(scratchpad->rmat));
		vu0_Mulv0m0v0();
		vu0_Stv0(&(scratchpad->x[1]));
		vu0_Stv0(&(p->mval.x[1]));
    }
#endif

    // printf("Check 5\n");

    fpu_CopyVector(&(scratchpad->tvec2),&(scratchpad->root));

    /* 次位置から、次abs_rotsを算出 */
    for(i=0;i<size-1;i++){
		float s,t;
		float a,b;

		scratchpad->tvec0.vx=mdl[i+1].tx;
		scratchpad->tvec0.vy=mdl[i+1].ty;
		scratchpad->tvec0.vz=mdl[i+1].tz;
		scratchpad->tvec0.vw=0.0f;

		vu0_Ldv0(&(scratchpad->x[i+1]));
		vu0_Ldv1(&(scratchpad->x[i]));
		vu0_Ldv2(&(scratchpad->tvec0));
		vu0_Ldm0(&(scratchpad->rmat));
 
		vu0_Subv0v1();
		vu0_Mulv2m0v2();
 
		s=a=vu0_VectorLength2v0();
		s*=(b=vu0_VectorLength2v2());
		t=vu0_InnerProductv0v2();
		vu0_Stv0(&(scratchpad->tvec0));
		vu0_OuterProductv2v0();
		s=fpu_Rsqrt(s,t);
		t=vu0_VectorLength2v2();
		if ( t==0.0f ) t =0.00001f ; // T.Morita Added 2002.02.25
		t=fpu_Sqrt((1.0f-s)*0.5f/t);
		vu0_Mulv2a(t);
		s=fpu_Sqrt((1.0f+s)*0.5f);

		if ( b==0.0f ) b =0.00001f ; // T.Morita Added 2002.02.25
		a/=b;
 
		vu0_Stv2(&(scratchpad->tvec1));
		scratchpad->tvec1.vw=s;
 
		MT_QuatMul(&(scratchpad->tvec1),&(scratchpad->tvec1),&(scratchpad->root));

#if 0
		if(i>=1){
			MT_QuatInverse(&(scratchpad->tvec3),&(scratchpad->tvec2));
			MT_QuatMul(&(scratchpad->tvec3),&(scratchpad->tvec3),&(scratchpad->tvec1));
			if(scratchpad->tvec3.vw<0.707106781f /* COS 45 */ ){
				float r;
				r=0.707106781f /* SIN 45 */ 
					* fpu_Sqrt(1.0f-scratchpad->tvec3.vw*scratchpad->tvec3.vw);
				scratchpad->tvec3.vx*=r;
				scratchpad->tvec3.vy*=r;
				scratchpad->tvec3.vz*=r;
				scratchpad->tvec3.vw=0.965925826f;

				MT_QuatMul(&(scratchpad->tvec1),&(scratchpad->tvec2),&(scratchpad->tvec3));
			}
			fpu_CopyVector(&(scratchpad->tvec2),&(scratchpad->tvec1));
		}
#endif

		MT_QuatToMat(&(objs->objs[i].world),&(scratchpad->tvec1));
		fpu_CopyVector((FVECTOR *)&(objs->objs[i].world.m[3][0]),&(scratchpad->x[i]));


		/* 速度に制限を加えて、震えを抑制 */
		vu0_Ldv1(&(scratchpad->tvec0));
		vu0_Ldv0(&(scratchpad->x[i]));


#define MARGIN		0.02f
#define LIMIT		0.2f
#define MARGIN_BAND	0.05f

#define UPPER_LIMIT_CHECK	(1.0f+LIMIT)
#define LOWER_LIMIT_CHECK	(1.0f-LIMIT)
#define UPPER_LIMIT		(UPPER_LIMIT_CHECK-MARGIN)
#define LOWER_LIMIT		(LOWER_LIMIT_CHECK+MARGIN)
#define UPPER_BAND_START	(UPPER_LIMIT_CHECK-MARGIN_BAND)
#define LOWER_BAND_START	(LOWER_LIMIT_CHECK+MARGIN_BAND)

		if(a>UPPER_LIMIT_CHECK*UPPER_LIMIT_CHECK){
			a=fpu_Rsqrt(a,UPPER_LIMIT);
			// a=UPPER_LIMIT/sqrtf(a);
			vu0_Mulv1a(a);
			vu0_Addv0v1();
			vu0_Stv0(&(scratchpad->x[i+1]));

			a=MARGIN/MARGIN_BAND;
		}
		else if(a<LOWER_LIMIT_CHECK*LOWER_LIMIT_CHECK){
			a=fpu_Rsqrt(a,LOWER_LIMIT);
			// a=LOWER_LIMIT/sqrtf(a);
			vu0_Mulv1a(a);
			vu0_Addv0v1();
			vu0_Stv0(&(scratchpad->x[i+1]));

			a=MARGIN/MARGIN_BAND;
		}
		else if(a>UPPER_BAND_START*UPPER_BAND_START){
			a=fpu_Sqrt(a);
			// a=sqrtf(a);
			a=(UPPER_LIMIT_CHECK-a)/MARGIN_BAND;
		}
		else if(a<LOWER_BAND_START*LOWER_BAND_START){
			a=fpu_Sqrt(a);
			// a=sqrtf(a);
			a=(a-LOWER_LIMIT_CHECK)/MARGIN_BAND;
		}
		else a=1.0f;

		vu0_Ldv0(&(scratchpad->x[i+1]));
		vu0_Ldv1(&(p->mval.x[i+1]));
		vu0_Subv1v0v1();
		vu0_Setv0w1();

#if 0
		if(p->mode==0) vu0_Mulv1a(a);
#else
		vu0_Mulv1a(a);
#endif

		vu0_Stv0(&(p->mval.x[i+1]));
		vu0_Stv1(&(p->mval.v[i+1]));
    }
    fpu_CopyMatrix(&(objs->world),&(objs->objs[0].world));
    fpu_CopyMatrix(&(objs->objs[size-1].world),&(objs->objs[size-2].world));
    fpu_CopyVector((FVECTOR *)&(objs->objs[size-1].world.m[3][0]),&(scratchpad->x[size-1]));

#if 0
    ScratchpadFree(scratchpad->f);

    if(p->target){
		ScratchpadFree(scratchpad->inv_m);
    }

    ScratchpadFree(scratchpad);
#elif 1
    ScratchpadFree2();

    if(p->target){
		ScratchpadFree2();
    }

    ScratchpadFree2();
#else
    GV_Free(scratchpad->f);

    if(p->target){
		GV_Free(scratchpad->inv_m);
    }

    GV_Free(scratchpad);
#endif
}
