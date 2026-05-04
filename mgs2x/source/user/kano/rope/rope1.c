//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	rope1.c
		紐形式のモデルを、風やモデルの動きに対応させて動かす
		片側(OBJ 0)のみ固定ロープの共通関数部

	1999/09/16 K.Kano
	$Id: rope1.c,v 1.1.1.3 2002/11/19 11:43:34 Yoshizawa1 Exp $
*/


#include "rope.h"


void InitLocate(DG_OBJS *objs,MODEL_PARAMETER *p)
{
    DG_MDL *mdl=objs->def->models;
    int size=objs->def->n_models;
    int i;

    if(p->tmat){
		vu0_Ldm0(p->tmat);
		vu0_Ldm1(&(p->root));

		vu0_Mulm2m0m1();
	}
	else{
		vu0_Ldm2(&(p->root));
	}

#if 0
	{ // TEST
		FMATRIX m;
		vu0_Stm2(&m);
		printf("<><><><>rope init = %f %f %f\n",m.m[3][0],m.m[3][1],m.m[3][2]);
	}
#endif
	
    vu0_Clrv0();
    vu0_Clrv2();
	vu0_Setv0w1();
    for(i=1;i<size;i++){
		FVECTOR vec;
		vu0_Mulv2m2v0();
		vec.vx=mdl[i].tx;
		vec.vy=mdl[i].ty;
		vec.vz=mdl[i].tz;
		vu0_Ldv1(&vec);
		vu0_Stv2(&(p->mval.x[i-1]));
		vu0_Addv0v1();
    }
    vu0_Stv2(&(p->mval.x[size-1]));

    {
		FVECTOR f;

		f.vx=0.0f;
		f.vy=0.0f;
		f.vz=0.0f;

		i=size-1;
		while(i>=0){
			FVECTOR t;
			float n;

			f.vy+=P_GRAVITY;

			t.vx=mdl[i].tx;
			t.vy=mdl[i].ty;
			t.vz=mdl[i].tz;
#if 1//BP_MATH - #ifdef KP_XBOX
if( t.vy == 0.0F ) t.vy = 0.0001F;	// 0.0check 20002/02/21 K.Uehara
#endif
			n=-f.vy/t.vy;
			t.vx*=n;
			t.vy*=n;
			t.vz*=n;

			p->mval.base_t[i].vx=-(f.vx+t.vx);
			p->mval.base_t[i].vy=0.0f;
			p->mval.base_t[i].vz=-(f.vz+t.vz);
			p->mval.base_t[i].vw=0.0f;

			f.vx=-t.vx;
			f.vy=-t.vy;
			f.vz=-t.vz;

			i--;
		}
    }

#if 0
    for(i=0;i<size;i++){
		printf("Obj %d  tx,ty,tz = %d %d %d\n",
			   i,(int)(mdl[i].tx),(int)(mdl[i].ty),(int)(mdl[i].tz));
		printf("tx ty tz = %f %f %f\n",
			   p->mval.base_t[i].vx,p->mval.base_t[i].vy,p->mval.base_t[i].vz);
    }
#endif

	if(p->target!=NULL){

#if 0
		fpu_CopyVector(&(p->target_pos),
					   (FVECTOR *)&(p->target->objs->world.m[3][0]));
#else
		if(p->mode){
			/* 髪の毛モード */
			vu0_Ldv0(&(objs->objs[1].trans));
			vu0_Mulv0m0v0();
			vu0_Stv0(&(p->target_pos));
		}
		else{
#ifdef BP_PSX2_ASM
			asm volatile ("sqc2 vf7,(%0)" : : "r"(&(p->target_pos)));
#else
			p->target_pos = *(FVECTOR *)vu0_Ptrm0()->m[3] ;
#endif
		}
#endif

	}
}

int InitRope(DG_OBJS *objs,MODEL_PARAMETER *p)
{
    int size=objs->def->n_models;

    p->mval.x=NULL;
    p->mval.v=NULL;

    if((p->mval.x=(FVECTOR *)GV_Malloc(sizeof(FVECTOR)*size*3))==NULL) return 0;
    p->mval.v=p->mval.x+size;
    p->mval.base_t=p->mval.v+size;

    GV_ZeroMemory(p->mval.v,sizeof(FVECTOR)*size);

	// InitLocate(objs,p);

#if 0
    if(p->tmat){
		vu0_Ldm0(p->tmat);
		vu0_Ldm1(&(p->root));

		vu0_Mulm2m0m1();
	}
	else{
		vu0_Ldm2(&(p->root));
	}

#if 0
	{ // TEST
		FMATRIX m;
		vu0_Stm2(&m);
		printf("rope init = %f %f %f\n",m.m[3][0],m.m[3][1],m.m[3][2]);
	}
#endif
	
    vu0_Clrv0();
    vu0_Clrv2();
	vu0_Setv0w1();
    for(i=1;i<size;i++){
		FVECTOR vec;
		vu0_Mulv2m2v0();
		vec.vx=mdl[i].tx;
		vec.vy=mdl[i].ty;
		vec.vz=mdl[i].tz;
		vu0_Ldv1(&vec);
		vu0_Stv2(&(p->mval.x[i-1]));
		vu0_Addv0v1();
    }
    vu0_Stv2(&(p->mval.x[size-1]));

    {
		FVECTOR f;

		f.vx=0.0f;
		f.vy=0.0f;
		f.vz=0.0f;

		i=size-1;
		while(i>=0){
			FVECTOR t;
			float n;

			f.vy+=P_GRAVITY;

			t.vx=mdl[i].tx;
			t.vy=mdl[i].ty;
			t.vz=mdl[i].tz;

#ifdef KP_XBOX
			if( t.vy == 0.0F ) t.vy = 0.0001F;	// Added by T.Morita  2002.02.26
#endif
			n=-f.vy/t.vy;
			t.vx*=n;
			t.vy*=n;
			t.vz*=n;

			p->mval.base_t[i].vx=-(f.vx+t.vx);
			p->mval.base_t[i].vy=0.0f;
			p->mval.base_t[i].vz=-(f.vz+t.vz);
			p->mval.base_t[i].vw=0.0f;

			f.vx=-t.vx;
			f.vy=-t.vy;
			f.vz=-t.vz;

			i--;
		}
    }

#if 0
    for(i=0;i<size;i++){
		printf("Obj %d  tx,ty,tz = %d %d %d\n",
			   i,(int)(mdl[i].tx),(int)(mdl[i].ty),(int)(mdl[i].tz));
		printf("tx ty tz = %f %f %f\n",
			   p->mval.base_t[i].vx,p->mval.base_t[i].vy,p->mval.base_t[i].vz);
    }
#endif

#endif
	
    return 1;
}

void ExitRope(DG_OBJS *objs,MODEL_PARAMETER *p)
{
    if(p->mval.x!=NULL) GV_Free(p->mval.x);
}


/* 片側(OBJ 0)のみ固定 */
void MoveRope1(DG_OBJS *objs,HZX_GROUP_ID hzx_id,MODEL_PARAMETER *p)
{
    DG_MDL *mdl=objs->def->models;
    int i;
    int size=objs->def->n_models;

    struct _scratchpad {
		FMATRIX rmat;

		FVECTOR tvec0,tvec1;
		FVECTOR root;

		FVECTOR *f;
		FVECTOR *t;
		FVECTOR *ma;
		FVECTOR *x;
		FVECTOR *abs_rots;
    };
    struct _scratchpad *scratchpad;

	
    /* スクラッチパッドのみで納まるかあやしいので、
       管理関数を介するようにした。*/
    ScratchpadManInit();

#if 0
    scratchpad = (struct _scratchpad *)ScratchpadAlloc(sizeof(struct _scratchpad));

    scratchpad->f      = (FVECTOR *)ScratchpadAlloc(sizeof(FVECTOR)*size*5);
    scratchpad->t      = scratchpad->f+size;
    scratchpad->ma     = scratchpad->t+size;
    scratchpad->x      = scratchpad->ma+size;
    scratchpad->abs_rots = scratchpad->x+size;
#else
    scratchpad = (struct _scratchpad *)ScratchpadAlloc2(sizeof(struct _scratchpad));

    scratchpad->f      = (FVECTOR *)ScratchpadAlloc2(sizeof(FVECTOR)*size*5);
    scratchpad->t      = scratchpad->f+size;
    scratchpad->ma     = scratchpad->t+size;
    scratchpad->x      = scratchpad->ma+size;
    scratchpad->abs_rots = scratchpad->x+size;
#endif

    /* ワールド座標の計算、同時に前回の速度を計算 */
    fpu_CopyMatrix(&(scratchpad->rmat),&(p->root));

    MT_MatToQuat(&(scratchpad->root),&(scratchpad->rmat));


    /* 力の計算 */
    for(i=1;i<size;i++){
		FVECTOR wind,*pwind=&wind;

		if(!OK_GetLocalWind(&(p->mval.x[i]),&wind)){
			pwind=&G_wind;
		}

		vu0_Clrv1();

		/* 重力 */
		{
//		   static const FVECTOR g={ 0.0f, P_GRAVITY, 0.0f, 0.0f, }; //BP JG - runtime 50/60
         static FVECTOR g={ 0.0f, 0.0f, 0.0f, 0.0f, };
         g.y = P_GRAVITY;

			vu0_Ldv0((FVECTOR *)&g);
		}
		vu0_Mulv0a(p->m);

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
			int fhzd=HZX_NearHazardCheck(hzx_id,&(p->mval.x[i]),
										 p->wl, /* 検出半径 */
										 HZX_CHK_ALL,0,
										 p->wl /* 反発半径 */);

			float h;

			if(fhzd){
				FVECTOR v;
				float s,t;

				HZX_GetReactVector(&v);

				/* 保存した値を再格納 */
				vu0_Ldv1(&(scratchpad->f[i]));

				vu0_Ldv0(&v);
				vu0_Ldv2(&(p->mval.x[i]));
				s=vu0_VectorLength2v0();
				vu0_Addv2v0();
				vu0_Stv2(&(p->mval.x[i]));

				vu0_Ldv2(&(p->mval.v[i]));
				t=vu0_InnerProductv0v2();
#ifdef KP_XBOX
				if( s == 0.0F ) s = 0.0001F;	// Added by T.Morita  2002.02.26
#endif
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

    /* 加速度の算出 */
    {
		float s,t;

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

#ifdef KP_XBOX
			if( t == 0.0F ) t = 0.0001F;	// Added by T.Morita  2002.02.26
#endif
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
	
    /* 次位置から、次abs_rotsを算出 */
    for(i=0;i<size-1;i++){
		float s,t;

		scratchpad->tvec0.vx=mdl[i+1].tx;
		scratchpad->tvec0.vy=mdl[i+1].ty;
		scratchpad->tvec0.vz=mdl[i+1].tz;
		scratchpad->tvec0.vw=0.0f;


		vu0_Ldm0(&(scratchpad->rmat));
		vu0_Ldv0(&(scratchpad->x[i+1]));
		vu0_Ldv1(&(scratchpad->x[i]));
		vu0_Ldv2(&(scratchpad->tvec0));
 
		vu0_Subv0v1();
		vu0_Mulv2m0v2();
 
		s=vu0_VectorLength2v0();
		s*=vu0_VectorLength2v2();
		t=vu0_InnerProductv0v2();
		vu0_Stv0(&(scratchpad->tvec0));
		vu0_OuterProductv2v0();
		s=fpu_Rsqrt(s,t);
		t=vu0_VectorLength2v2();
#ifdef KP_XBOX
		if( t == 0.0F ) t = 0.0001F;	// Added by T.Morita  2002.02.26
#endif
		t=fpu_Sqrt((1.0f-s)*0.5f/t);
		vu0_Mulv2a(t);
		s=fpu_Sqrt((1.0f+s)*0.5f);
 
		vu0_Stv2(&(scratchpad->tvec1));
		scratchpad->tvec1.vw=s;

		MT_QuatMul(&(scratchpad->tvec1),&(scratchpad->tvec1),&(scratchpad->root));
		MT_QuatToMat(&(objs->objs[i].world),&(scratchpad->tvec1));
		fpu_CopyVector((FVECTOR *)&(objs->objs[i].world.m[3][0]),&(scratchpad->x[i]));

		vu0_Ldv1(&(scratchpad->tvec0));
		vu0_Ldv0(&(scratchpad->x[i]));
		vu0_Ldv2(&(p->mval.x[i+1]));
		vu0_Addv0v1();
		vu0_Stv0(&(scratchpad->x[i+1]));

		vu0_Subv1v0v2();
		vu0_Setv0w1();
		vu0_Stv1(&(p->mval.v[i+1]));
		vu0_Stv0(&(p->mval.x[i+1]));
    }
    fpu_CopyMatrix(&(objs->world),&(objs->objs[0].world));
    fpu_CopyMatrix(&(objs->objs[size-1].world),&(objs->objs[size-2].world));
    fpu_CopyVector((FVECTOR *)&(objs->objs[size-1].world.m[3][0]),&(scratchpad->x[size-1]));


#if 0
    ScratchpadFree(scratchpad->f);

    ScratchpadFree(scratchpad);
#else
    ScratchpadFree2();

    ScratchpadFree2();
#endif
}
