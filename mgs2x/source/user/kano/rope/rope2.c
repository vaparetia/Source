//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	rope2.c
		紐形式のモデルを、風やモデルの動きに対応させて動かす
		両側(OBJ 0,OBJ n)固定ロープの共通関数部

	1999/09/16 K.Kano
	$Id: rope2.c,v 1.1.1.3 2002/11/19 11:43:34 Yoshizawa1 Exp $
*/


#include "rope.h"


void target_callback(TARGET *ofs,TARGET *def,void *param)
{
    FVECTOR *v=(FVECTOR *)param;
	
	/* powerを持たないターゲットが当たる場合もある
	   M.Sonoyama 修正 2000/07/14 */
	if ( ofs->power != NULL ) {
		fpu_AddVectors(v,v,&(ofs->power->force));
		fpu_AddVectors(v+1,v+1,&(ofs->power->force));
	}
    def->damaged=0;
}

/* 定常状態を算出 */
void Rope2_CalcBase(DG_OBJS *objs,MODEL_PARAMETER2 *p)
{
    DG_MDL *mdl=objs->def->models;
    int size=objs->def->n_models;
    int i;
    float *l;
    float lsum;


    //printf("size = %d\n",size);

    ScratchpadManInit();

    l=(float *)ScratchpadAlloc(sizeof(float)*size);

    if(p->tmat){
		vu0_Ldm1(p->tmat);
		vu0_Ldm2(&(p->root));

		vu0_Mulm0m1m2();

#ifdef BP_PSX2_ASM
		asm volatile ("vmove.xyzw vf1,vf7");
#else
		vu0_Ldv0( (FVECTOR *)vu0_Ptrm0()->m[3] ) ;
#endif

    }
    else{
		vu0_Ldv0((FVECTOR *)&(p->root.m[3][0]));
		vu0_Ldm0(&(p->root));
    }

    l[0]=0.0f;
    for(i=1;i<size;i++){
		FVECTOR vec;
		float ll;
		vec.vx=mdl[i].tx;
		vec.vy=mdl[i].ty;
		vec.vz=mdl[i].tz;
		vec.vw=0.0f;
		vu0_Ldv2(&vec);
		vu0_Stv0(&(p->mval.basex[i-1]));
		vu0_Stv0(&(p->mval.x[i-1]));
		vu0_Mulv1m0v2();
		ll=vu0_VectorLength2v2();
		vu0_Addv0v1();
		l[i]=l[i-1]+fpu_Sqrt(ll);
    }
    vu0_Stv0(&(p->mval.basex[size-1]));
    vu0_Stv0(&(p->mval.x[size-1]));
    lsum=l[size-1];

    vu0_Ldv0(&(p->mval.basex[0]));
    vu0_Ldv1(&(p->mval.basex[1]));
    vu0_Ldv2(&(p->mval.basex[2]));
    vu0_Subv0v1();

#ifdef BP_PSX2_ASM
    for(i=1;i<size-1;i++){
		asm volatile ("
		vsub.xyz	vf28,vf2,vf3
		vadd.xyz	vf29,vf1,vf28
		vmul.xyz	vf30,vf29,vf29
		vmulax.w	ACC,vf0,vf30x
		vmadday.w	ACC,vf0,vf30y
		vmaddz.w	vf31,vf0,vf30z
		vrsqrt		Q,vf0w,vf31w
		vwaitq
		vmulq.xyz	vf29,vf29,Q
		vmove.xyzw	vf1,vf28
		vmove.xyzw	vf2,vf3
		lqc2		vf3,0(%1)
		sqc2		vf29,0(%0)
		" : : "r"(&(p->mval.base_normal[i])),"r"(&(p->mval.basex[i+2])) : "memory" );
    }
#else
    for(i=1;i<size-1;i++){
		FVECTOR _vf28, _vf29, _vf30, _vf31 ;
		extern FVECTOR REG_vf1, REG_vf2, REG_vf3 ;

		fpu_SubVectors( &_vf28, &REG_vf2, &REG_vf3 ) ;
		fpu_AddVectors( &_vf29, &REG_vf1, &_vf28 ) ;
		fpu_MulVectors( &_vf30, &_vf29, &_vf29 ) ;
		_vf31.vw = _vf30.vx + _vf30.vy + _vf30.vz ;
		fpu_MulVectorScaler( &_vf29, &_vf29, fpu_Rsqrt( _vf31.vw, 1.0f ) ) ;
		REG_vf1 = _vf28 ;
		REG_vf2 = REG_vf3  ;
		REG_vf3 = p->mval.basex[i+2] ;
		p->mval.base_normal[i] = _vf29 ;
    }
#endif

    fpu_ClearVector(&(p->mval.base_normal[0]));
    fpu_ClearVector(&(p->mval.base_normal[size-1]));

    for(i=0;i<size;i++){
		p->mval.limit[i]=sinf((float)M_PI*l[i]/lsum)*lsum*p->k_limit;
    }

#if 0
    for(i=0;i<size-1;i++){
		FMATRIX m;
		FVECTOR ofs,sz;

		sz.vx=(mdl[i].ux-mdl[i].lx)/2;
		sz.vy=(mdl[i].uy-mdl[i].ly)/2;
		sz.vz=(mdl[i].uz-mdl[i].lz)/2;
		ofs.vx=mdl[i].lx+sz.vx;
		ofs.vy=mdl[i].ly+sz.vy;
		ofs.vz=mdl[i].lz+sz.vz;

		fpu_CopyUnitMatrix(p->tbasem+i);
		fpu_CopyUnitMatrix(&m);
		fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(p->mval.x[i]));

		GM_SetTarget(p->targetsys+i,
					 TARGET_DEFENSE|TARGET_ROTATE|TARGET_POWER|TARGET_PUSH,
					 0,BOTH_SIDE,&sz,&ofs);
		GM_MoveTarget2(p->targetsys+i,&m);
		GM_SetTargetCallBack(p->targetsys+i,target_callback,(void *)&(p->mval.v[i]));
		GM_PutTarget(p->targetsys+i);
    }
#endif

    ScratchpadFree(l);
}

void Rope2_SetTraget(DG_OBJS *objs,MODEL_PARAMETER2 *p)
{
    DG_MDL *mdl=objs->def->models;
    int size=objs->def->n_models;
    int i;

    for(i=0;i<size-1;i++){
		FMATRIX m;
		FVECTOR ofs,sz;

		sz.vx=(mdl[i].ux-mdl[i].lx)/2;
		sz.vy=(mdl[i].uy-mdl[i].ly)/2;
		sz.vz=(mdl[i].uz-mdl[i].lz)/2;
		ofs.vx=mdl[i].lx+sz.vx;
		ofs.vy=mdl[i].ly+sz.vy;
		ofs.vz=mdl[i].lz+sz.vz;

		fpu_CopyUnitMatrix(p->tbasem+i);
		fpu_CopyUnitMatrix(&m);
		fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(p->mval.x[i]));

		GM_SetTarget(p->targetsys+i,
					 TARGET_DEFENSE|TARGET_ROTATE|TARGET_POWER|TARGET_PUSH,
					 0,BOTH_SIDE,&sz,&ofs);
		GM_MoveTarget2(p->targetsys+i,&m);
		GM_SetTargetCallBack(p->targetsys+i,target_callback,(void *)&(p->mval.v[i]));
		GM_PutTarget(p->targetsys+i);
    }
}

int InitRope2(DG_OBJS *objs,MODEL_PARAMETER2 *p)
{
    int size=objs->def->n_models;

#if 0
    p->mval.basex=NULL;
    p->mval.base_normal=NULL;
    p->mval.x=NULL;
    p->mval.v=NULL;
    p->mval.limit=NULL;
    p->targetsys=NULL;
    p->tbasem=NULL;

    if((p->mval.basex=(FVECTOR *)GV_Malloc(sizeof(FVECTOR)*size))==NULL) return 0;
    if((p->mval.base_normal=(FVECTOR *)GV_Malloc(sizeof(FVECTOR)*size))==NULL) return 0;
    if((p->mval.x=(FVECTOR *)GV_Malloc(sizeof(FVECTOR)*size))==NULL) return 0;
    if((p->mval.v=(FVECTOR *)GV_Malloc(sizeof(FVECTOR)*size))==NULL) return 0;
    if((p->mval.limit=(float *)GV_Malloc(sizeof(float)*size))==NULL) return 0;
    if((p->targetsys=(TARGET *)GV_Malloc(sizeof(TARGET)*size))==NULL) return 0;
    if((p->tbasem=(FMATRIX *)GV_Malloc(sizeof(FMATRIX)*size))==NULL) return 0;
#else

    if((p->tbasem
		=(FMATRIX *)GV_Malloc((sizeof(FVECTOR)*4+sizeof(float)+
							   sizeof(TARGET)+sizeof(FMATRIX))*size))==NULL) return 0;

    p->mval.basex=(FVECTOR *)(p->tbasem+size);
    p->mval.base_normal=p->mval.basex+size;
    p->mval.x=p->mval.base_normal+size;
    p->mval.v=p->mval.x+size;
    p->targetsys=(TARGET *)(p->mval.v+size);
    p->mval.limit=(float *)(p->targetsys+size);

#endif

    GV_ZeroMemory(p->mval.v,sizeof(FVECTOR)*size);

    return 1;
}

void ExitRope2(DG_OBJS *objs,MODEL_PARAMETER2 *p)
{
#if 0
    if(p->mval.basex!=NULL) GV_Free(p->mval.basex);
    if(p->mval.base_normal!=NULL) GV_Free(p->mval.base_normal);
    if(p->mval.x!=NULL) GV_Free(p->mval.x);
    if(p->mval.v!=NULL) GV_Free(p->mval.v);
    if(p->mval.limit!=NULL) GV_Free(p->mval.limit);
    if(p->targetsys!=NULL){
		int size=objs->def->n_models;
		int i;
		for(i=0;i<size-1;i++){
			GM_FreeTarget(p->targetsys+i);
		}
		GV_Free(p->targetsys);
	}
    if(p->tbasem!=NULL) GV_Free(p->tbasem);
#else
    if(p->tbasem!=NULL){
		int size=objs->def->n_models;
		int i;

		for(i=0;i<size-1;i++){
			GM_FreeTarget(p->targetsys+i);
		}

		GV_Free(p->tbasem);
	}
#endif
}


/* 両側(OBJ 0,OBJ n)固定 */
void MoveRope2(DG_OBJS *objs,HZX_GROUP_ID hzx_id,MODEL_PARAMETER2 *p)
{
    DG_MDL *mdl=objs->def->models;
    int i;
    int size=objs->def->n_models;

    struct _scratchpad {
		FMATRIX rmat;

		FVECTOR tvec0,tvec1;
		FVECTOR root;

		FMATRIX *inv_m;

		FVECTOR *f;
		FVECTOR *x;
		FVECTOR *abs_rots;
    };
    struct _scratchpad *scratchpad;

    /* スクラッチパッドのみで納まるかあやしいので、
       管理関数を介するようにした。*/
    ScratchpadManInit();

#if 0
    scratchpad = (struct _scratchpad *)ScratchpadAlloc(sizeof(struct _scratchpad));

    if(p->target){
		int tsize=p->target->objs->def->n_models;
		scratchpad->inv_m  = (FMATRIX *)ScratchpadAlloc(sizeof(FMATRIX)*tsize);
    }

    scratchpad->f      = (FVECTOR *)ScratchpadAlloc(sizeof(FVECTOR)*size*3);
    scratchpad->x      = scratchpad->f+size;
    scratchpad->abs_rots = scratchpad->x+size;
#else
    scratchpad = (struct _scratchpad *)ScratchpadAlloc2(sizeof(struct _scratchpad));

    if(p->target){
		int tsize=p->target->objs->def->n_models;
		scratchpad->inv_m  = (FMATRIX *)ScratchpadAlloc2(sizeof(FMATRIX)*tsize);
    }

    scratchpad->f      = (FVECTOR *)ScratchpadAlloc2(sizeof(FVECTOR)*size*3);
    scratchpad->x      = scratchpad->f+size;
    scratchpad->abs_rots = scratchpad->x+size;
#endif


    if(p->tmat){
		float *l;
		float lsum;

		vu0_Ldm1(p->tmat);
		vu0_Ldm2(&(p->root));

		vu0_Mulm0m1m2();

		vu0_Stm0(&(scratchpad->rmat));

#ifdef BP_PSX2_ASM
		asm volatile ("vmove.xyzw vf1,vf7");
#else
		vu0_Ldv0( (FVECTOR *)vu0_Ptrm0()->m[3] ) ;
#endif

#if 0
		l=(float *)ScratchpadAlloc(sizeof(float)*size);
#else
		l=(float *)ScratchpadAlloc2(sizeof(float)*size);
#endif

		l[0]=0.0f;
		for(i=1;i<size;i++){
			float ll;

			scratchpad->tvec0.vx=mdl[i].tx;
			scratchpad->tvec0.vy=mdl[i].ty;
			scratchpad->tvec0.vz=mdl[i].tz;
			scratchpad->tvec0.vw=0.0f;
			vu0_Ldv2(&(scratchpad->tvec0));
			vu0_Stv0(&(p->mval.basex[i-1]));
			vu0_Stv0(&(p->mval.x[i-1]));
			vu0_Mulv1m0v2();
			ll=vu0_VectorLength2v2();
			vu0_Addv0v1();
			l[i]=l[i-1]+fpu_Sqrt(ll);
		}
		vu0_Stv0(&(p->mval.basex[size-1]));
		vu0_Stv0(&(p->mval.x[size-1]));
		lsum=l[size-1];

		vu0_Ldv0(&(p->mval.basex[0]));
		vu0_Ldv1(&(p->mval.basex[1]));
		vu0_Ldv2(&(p->mval.basex[2]));
		vu0_Subv0v1();


		for(i=1;i<size-1;i++){
#ifdef BP_PSX2_ASM
			asm volatile ("
			vsub.xyz	vf28,vf2,vf3
			vadd.xyz	vf29,vf1,vf28
			vmul.xyz	vf30,vf29,vf29
			vmulax.w	ACC,vf0,vf30x
			vmadday.w	ACC,vf0,vf30y
			vmaddz.w	vf31,vf0,vf30z
			vrsqrt		Q,vf0w,vf31w
			vwaitq
			vmulq.xyz	vf29,vf29,Q
			vmove.xyzw	vf1,vf28
			vmove.xyzw	vf2,vf3
			lqc2		vf3,0(%1)
			sqc2		vf29,0(%0)
			" : : "r"(&(p->mval.base_normal[i])),"r"(&(p->mval.basex[i+2])) : "memory" );
#else
			FVECTOR _vf28, _vf29, _vf30, _vf31 ;
			extern FVECTOR REG_vf1, REG_vf2, REG_vf3 ;

			fpu_SubVectors( &_vf28, &REG_vf2, &REG_vf3 ) ;
			fpu_AddVectors( &_vf29, &REG_vf1, &_vf28 ) ;
			fpu_MulVectors( &_vf30, &_vf29, &_vf29 ) ;
			_vf31.vw = _vf30.vx + _vf30.vy + _vf30.vz ;
			fpu_MulVectorScaler( &_vf29, &_vf29, fpu_Rsqrt( _vf31.vw, 1.0f ) ) ;
			REG_vf1 = _vf28 ;
			REG_vf2 = REG_vf3  ;
			REG_vf3 = p->mval.basex[i+2] ;
			p->mval.base_normal[i] = _vf29 ;
#endif
		}

		fpu_ClearVector(&(p->mval.base_normal[0]));
		fpu_ClearVector(&(p->mval.base_normal[size-1]));

		for(i=0;i<size;i++){
			p->mval.limit[i]=sinf((float)M_PI*l[i]/lsum)*lsum*p->k_limit;
		}

#if 0
		ScratchpadFree(l);
#else
		ScratchpadFree2();
#endif
    }
    else{
		fpu_CopyMatrix(&(scratchpad->rmat),&(p->root));
    }

    MT_MatToQuat(&(scratchpad->root),&(scratchpad->rmat));

    if(p->collision_flag && p->target){
		DG_OBJS *tobjs=p->target->objs;
		int tsize=p->target->objs->def->n_models;

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

#if 0
    printf("Gwind = %f %f %f\n",G_wind.vx,G_wind.vy,G_wind.vz);
#endif

    /* 力の計算 */
    for(i=1;i<size-1;i++){
		FVECTOR wind,*pwind=&wind;

		if(!OK_GetLocalWind(&(p->mval.x[i]),&wind)){
			pwind=&G_wind;
		}

		vu0_Clrv1();

		/* 風 */
		vu0_Ldv0(pwind);
		vu0_Ldv2(&(p->mval.base_normal[i]));
		vu0_Mulv0a(p->pa);
		vu0_OuterProductv0v2();
		vu0_OuterProductv2v0();

    	/* 空気抵抗 */
		vu0_Ldv0(&(p->mval.v[i]));

		vu0_Addv1v2();

		vu0_Mulv2v0a(p->k);

		/* バネの計算 */
		vu0_Ldv0(&(p->mval.x[i]));

		vu0_Addv1v2();

		vu0_Ldv2(&(p->mval.basex[i]));
		vu0_Subv2v0();
		vu0_Mulv2a(p->sp);
		vu0_Addv1v2();

		/* VU0が使われるため値を保存する */
		vu0_Stv1(&(scratchpad->f[i]));


#if 1 /* 人間当たりの判定 */
		if(p->target==NULL){
			if(ControlsCollision(&(scratchpad->tvec0),&(p->mval.x[i]),(float)(p->wl))){
				fpu_CopyVector(&(p->mval.x[i]),&(scratchpad->tvec0));
			}
		}
#endif

#if 1
		/* 当たり判定 */
        if(p->collision_flag){
			if(p->target){
				if(CalcObjsCollisionWithInvM(&(scratchpad->tvec0),&(p->mval.x[i]),
											 p->target->objs,p->param_oval,
											 scratchpad->inv_m)){

					float l;

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
				}
			}
			else{
#if 1

				int hzd=HZX_NearHazardCheck(hzx_id,&(p->mval.x[i]),
											p->wl, /* 検出半径 */
											HZX_CHK_ALL,0,
											p->wl /* 反発半径 */);

				float h;

				/* 保存した値を再格納 */
				vu0_Ldv1(&(scratchpad->f[i]));

				if(hzd){
					float s,t;

					HZX_GetReactVector(&(scratchpad->tvec0));

					vu0_Ldv0(&(scratchpad->tvec0));
					vu0_Ldv2(&(p->mval.x[i]));
					s=vu0_VectorLength2v0();
					vu0_Addv2v0();
					vu0_Stv2(&(p->mval.x[i]));

					vu0_Ldv2(&(p->mval.v[i]));
					t=vu0_InnerProductv0v2();
					if ( s==0.0 ) s = 0.000001f ; // Added by T.Morita 2002.02.26
					vu0_Mulv2v0a(t/s*p->m*p->we);
					vu0_Addv1v2();
				}

				h=HZX_GetFloorLevel()+p->wl;
				if(p->mval.x[i].vy<h){
					static const FVECTOR v={ 0.0f,1.0f,0.0f,0.0f, };
					p->mval.x[i].vy=h;
					vu0_Ldv0(&v);
					vu0_Mulv2v0a(p->mval.v[i].vy*p->m*p->we);
					vu0_Addv1v2();
				}

				vu0_Stv1(&(scratchpad->f[i]));
#endif

			}
		}

#endif

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

    /* 速度、次位置の算出 */
    for(i=size-2;i>0;i--){
		vu0_Ldv2(&(scratchpad->f[i]));
		vu0_Ldv1(&(p->mval.v[i]));
		vu0_Ldv0(&(p->mval.x[i]));

#if 0
		printf("v%d = %f %f %f\n",i,
			   p->mval.v[i].vx,p->mval.v[i].vy,p->mval.v[i].vz);
		printf("a%d = %f %f %f\n",i,
			   scratchpad->f[i].vx,scratchpad->f[i].vy,scratchpad->f[i].vz);
#endif

		vu0_Mulv2a(p->inv_m);

		vu0_Addv0v1();
		vu0_Addv1v2();
		vu0_Setv0w1();

		vu0_Stv1(&(p->mval.v[i]));
		vu0_Ldv1(&(p->mval.basex[i]));

		{
			float l;

			vu0_Subv2v0v1();
			l=vu0_VectorLength2v2();
			l=fpu_Rsqrt(l,p->mval.limit[i]);
			if(l<0.95f){
				vu0_Mulv0v2a(l);
				vu0_Ldv2(&(p->mval.x[i]));
				vu0_Addv0v1();
				vu0_Subv1v0v2();
				vu0_Stv1(&(p->mval.v[i]));

#if 0
				printf("x%d = %f %f %f\n",i,scratchpad->x[i].vx,scratchpad->x[i].vy,scratchpad->x[i].vz);
				printf("v%d = %f %f %f\n",i,p->mval.v[i].vx,p->mval.v[i].vy,p->mval.v[i].vz);
#endif

			}
		}

		vu0_Stv0(&(p->mval.x[i]));
		vu0_Stv0(&(scratchpad->x[i]));
    }

    fpu_CopyVector(&(scratchpad->x[0]),&(p->mval.x[0]));
    fpu_CopyVector(&(scratchpad->x[size-1]),&(p->mval.x[size-1]));


    /* 次位置から、次abs_rotsを算出 */
    for(i=size-2;i>=0;i--){
		scratchpad->tvec0.vx=mdl[i+1].tx;
		scratchpad->tvec0.vy=mdl[i+1].ty;
		scratchpad->tvec0.vz=mdl[i+1].tz;
		scratchpad->tvec0.vw=0.0f;

		UTL_MakeQuatM(&(scratchpad->tvec1),&(scratchpad->x[i]),&(scratchpad->x[i+1]),
					  &(scratchpad->rmat),&(scratchpad->tvec0));

		MT_QuatMul(&(scratchpad->tvec1),&(scratchpad->tvec1),&(scratchpad->root));
		MT_QuatToMat(&(objs->objs[i].world),&(scratchpad->tvec1));
		fpu_CopyVector((FVECTOR *)&(objs->objs[i].world.m[3][0]),&(scratchpad->x[i]));

		GM_MoveTarget2(p->targetsys+i,&(objs->objs[i].world));
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
#else
    ScratchpadFree2();

    if(p->target){
		ScratchpadFree2();
    }

    ScratchpadFree2();
#endif

}
