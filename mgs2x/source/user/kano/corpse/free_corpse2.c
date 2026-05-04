//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	free_corpse.c
		死体の姿勢制御(自由落下系統)

	1999/10/14 K.Kano
	$Id: free_corpse2.c,v 1.1.1.3 2002/11/19 11:43:09 Yoshizawa1 Exp $
*/

#include "corpse.h"

#define DEG_BACKBONE		(M_PI*15.0f/180.0f)
#define VAL_BACKBONE		30.0f
#define DEG_BACKBONE_HIP	(M_PI*10.0f/180.0f)
#define VAL_BACKBONE_HIP	50.0f

#if 0
#define SWITCH2HOLD_FRAMES	20
#else
#define SWITCH2HOLD_FRAMES	(60*6)
#endif

#if 0
#define KANOKEN_DEBUG_PRINT
#endif


static void StartCorpse2(struct _scratchpad *scratchpad,OBJECT *obj,CORPSE_PARAMETER *corpsep)
{
    //MAP *map;
    int size=HUMAN_MODEL_OBJN;
    int i;

    for(i=0;i<size;i++){
	fpu_CopyVector(&(scratchpad->abs_rots[i]),&(obj->m_ctrl->abs_rots[i]));
	fpu_CopyVector(&(corpsep->mtn_abs_rots[i]),&(scratchpad->abs_rots[i]));
    }

#if 0

#if 0
    if((map=GM_GetMap(GM_CurrentMap))==NULL){
	scratchpad->hzd=NULL;
    }
    else{
	scratchpad->hzd=map->hzd;
    }
#else
    scratchpad->hzd=HZX_GetCurrentHzd(GM_CurrentMap);
#endif

#endif

    DG_GetPos(&(scratchpad->rmat));
    MT_MatToQuat(&(scratchpad->root),&(scratchpad->rmat));
    MT_QuatInverse(&(scratchpad->inv_root),&(scratchpad->root));

    Corpse_WorldCalc(scratchpad->objmat,&(scratchpad->rmat),obj,scratchpad->abs_rots);
    for(i=0;i<size;i++){
	fpu_CopyVector(&(scratchpad->x[i]),
		       (FVECTOR *)&(obj->objs->objs[i].world.m[3][0]));
    }
    for(i=0;i<size;i++){
	fpu_CopyMatrix(&(scratchpad->kmat[i]),&(scratchpad->objmat[i]));
    }

#if 0
    printf("%f %f %f %f\n",scratchpad->root.vx,scratchpad->root.vy,
	   scratchpad->root.vz,scratchpad->root.vw);
#endif
}


/* 反射ベクトルを取り出し、向きを計算しなおす。
   時々、逆方向を返すため。*/
static void GetReactVector(FVECTOR *v,FVECTOR *target,FVECTOR *base)
{
    HZX_GetReactVector(v);

    if(base->vx>=target->vx) v->vx=fpu_Abs(v->vx);
    else v->vx=-fpu_Abs(v->vx);
    if(base->vz>=target->vz) v->vz=fpu_Abs(v->vz);
    else v->vz=-fpu_Abs(v->vz);
}


#if 0
#define COSLIMIT	(0.923879533f)	/* 45/2=22.5°*/
#elif 1
#define COSLIMIT	(0.866025404f)	/* 60/2=30°*/
#else
#define COSLIMIT	(0.707106781f)	/* 90/2=45°*/
#endif

/* 代表点を壁判定して移動 */
static void MovePoint(struct _scratchpad *scratchpad,DG_OBJS *objs)
{
    DG_MDL *mdl=objs->def->models;
    FMATRIX m;
    FVECTOR q,v,v1,v2;
    int hzd;

    fpu_ClearVector(&v1);

    hzd=HZX_NearHazardCheck(scratchpad->hzx_id,
			    (FVECTOR *)&(scratchpad->kmat[HUMAN21_KUBI].m[3][0]),
			    CHEST_COLLISION, /* 検出半径 */
			    HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
			    CHEST_COLLISION /* 反発半径 */);

    if(hzd){
	float l0,l1,in;

	GetReactVector(&v1,
		       (FVECTOR *)&(scratchpad->kmat[HUMAN21_KUBI].m[3][0]),
		       (FVECTOR *)&(scratchpad->kmat[HUMAN21_KOSHI].m[3][0]));

	vu0_Ldv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_KUBI].m[3][0]));
	vu0_Ldv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_MUNE].m[3][0]));
	vu0_Ldv0(&v1);

	vu0_Subv2v1();

	vu0_Stv0(&v2);

	l0=vu0_VectorLength2v2();
	l1=vu0_VectorLength2v0();
	in=vu0_InnerProductv0v2();

#ifdef KANOKEN_DEBUG_PRINT
	printf("Mune     = %f %f %f\n",
	       scratchpad->kmat[HUMAN21_MUNE].m[3][0],
	       scratchpad->kmat[HUMAN21_MUNE].m[3][1],
	       scratchpad->kmat[HUMAN21_MUNE].m[3][2]);
	printf("Kubi     = %f %f %f\n",
	       scratchpad->kmat[HUMAN21_KUBI].m[3][0],
	       scratchpad->kmat[HUMAN21_KUBI].m[3][1],
	       scratchpad->kmat[HUMAN21_KUBI].m[3][2]);
	printf("React V  = %f %f %f\n",v1.vx,v1.vy,v1.vz);
	printf("L0 L1 IN = %f %f %f\n",l0,l1,in);
#endif

	if(in>=0) l0=fpu_Sqrt(l0-in*in/l1);
	else l0=fpu_Sqrt(l0-in*in/l1+2*in-l1);

#ifdef KANOKEN_DEBUG_PRINT
	printf("L0'      = %f\n",l0);
#endif

	if(fpu_Abs(v1.vz)>fpu_Abs(v1.vx)){
	    if(scratchpad->kmat[HUMAN21_KUBI].m[3][0]
	       >=scratchpad->kmat[HUMAN21_MUNE].m[3][0]){

		v1.vx=scratchpad->kmat[HUMAN21_MUNE].m[3][0]+l0;
		v1.vy=scratchpad->kmat[HUMAN21_KUBI].m[3][1];
		v1.vz+=scratchpad->kmat[HUMAN21_KUBI].m[3][2];

		v2.vx=scratchpad->kmat[HUMAN21_MUNE].m[3][0]-l0;
		v2.vy=scratchpad->kmat[HUMAN21_KUBI].m[3][1];
		v2.vz+=scratchpad->kmat[HUMAN21_KUBI].m[3][2];
	    }
	    else{
		v1.vx=scratchpad->kmat[HUMAN21_MUNE].m[3][0]-l0;
		v1.vy=scratchpad->kmat[HUMAN21_KUBI].m[3][1];
		v1.vz+=scratchpad->kmat[HUMAN21_KUBI].m[3][2];

		v2.vx=scratchpad->kmat[HUMAN21_MUNE].m[3][0]+l0;
		v2.vy=scratchpad->kmat[HUMAN21_KUBI].m[3][1];
		v2.vz+=scratchpad->kmat[HUMAN21_KUBI].m[3][2];
	    }
	}
	else{
	    if(scratchpad->kmat[HUMAN21_KUBI].m[3][2]
	       >=scratchpad->kmat[HUMAN21_MUNE].m[3][2]){

		v1.vx+=scratchpad->kmat[HUMAN21_KUBI].m[3][0];
		v1.vy=scratchpad->kmat[HUMAN21_KUBI].m[3][1];
		v1.vz=scratchpad->kmat[HUMAN21_MUNE].m[3][2]+l0;

		v2.vx+=scratchpad->kmat[HUMAN21_KUBI].m[3][0];
		v2.vy=scratchpad->kmat[HUMAN21_KUBI].m[3][1];
		v2.vz=scratchpad->kmat[HUMAN21_MUNE].m[3][2]-l0;
	    }
	    else{
		v1.vx+=scratchpad->kmat[HUMAN21_KUBI].m[3][0];
		v1.vy=scratchpad->kmat[HUMAN21_KUBI].m[3][1];
		v1.vz=scratchpad->kmat[HUMAN21_MUNE].m[3][2]-l0;

		v2.vx+=scratchpad->kmat[HUMAN21_KUBI].m[3][0];
		v2.vy=scratchpad->kmat[HUMAN21_KUBI].m[3][1];
		v2.vz=scratchpad->kmat[HUMAN21_MUNE].m[3][2]+l0;
	    }
	}

	vu0_Ldv2(&v1);
	vu0_Ldv1(&v2);
	vu0_Ldv0(&(scratchpad->x[HUMAN21_KUBI]));

	vu0_Stv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_KUBI].m[3][0]));

	vu0_Subv2v0();
	vu0_Subv0v1v0();
	l0=vu0_VectorLength2v2();
	l1=vu0_VectorLength2v0();

	if(l0>l1) vu0_Stv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_KUBI].m[3][0]));
    }

    /* 頭、首、肩、腕の位置を再計算 */
    vu0_Ldv0((FVECTOR *)&(scratchpad->objmat[HUMAN21_KUBI].m[3][0]));
    vu0_Ldv1((FVECTOR *)&(scratchpad->objmat[HUMAN21_MUNE].m[3][0]));

    vu0_Subv0v1();

    vu0_Stv0(&v);

    UTL_MakeQuat(&q,(FVECTOR *)&(scratchpad->kmat[HUMAN21_MUNE].m[3][0]),
		 (FVECTOR *)&(scratchpad->kmat[HUMAN21_KUBI].m[3][0]),&v);

    /* 角度の制限を越えている場合は、補正をしない。*/
    if(q.vw<COSLIMIT){
	fpu_CopyVector((FVECTOR *)&(scratchpad->kmat[HUMAN21_KUBI].m[3][0]),
		       (FVECTOR *)&(scratchpad->objmat[HUMAN21_KUBI].m[3][0]));
	goto next1;
    }

    MT_QuatToMat(&m,&q);
    vu0_Ldm1(&m);
    vu0_Ldm2(&(scratchpad->kmat[HUMAN21_MUNE]));

    v.vx=mdl[HUMAN21_KUBI].tx;
    v.vy=mdl[HUMAN21_KUBI].ty;
    v.vz=mdl[HUMAN21_KUBI].tz;
    v.vw=1.0f;
    vu0_Ldv0(&v);

    vu0_Mulm0m1m2();

    v.vx=mdl[HUMAN21_MIGI_KATA].tx;
    v.vy=mdl[HUMAN21_MIGI_KATA].ty;
    v.vz=mdl[HUMAN21_MIGI_KATA].tz;
    vu0_Ldv1(&v);

#ifdef BP_PSX2_ASM
    asm volatile ("vmove.xyzw vf7,vf15");
#else
#endif

    v.vx=mdl[HUMAN21_HIDARI_KATA].tx;
    v.vy=mdl[HUMAN21_HIDARI_KATA].ty;
    v.vz=mdl[HUMAN21_HIDARI_KATA].tz;
    vu0_Ldv2(&v);

    vu0_Mulv0m0v0();
    vu0_Mulv1m0v1();
    vu0_Mulv2m0v2();

    vu0_Ldm2(&(scratchpad->kmat[HUMAN21_KUBI]));
    vu0_Stm0(&(scratchpad->kmat[HUMAN21_MUNE]));

    vu0_Mulm0m1m2();

    vu0_Ldm2(&(scratchpad->kmat[HUMAN21_MIGI_KATA]));

#ifdef BP_PSX2_ASM
    asm volatile ("vmove.xyzw vf7,vf1");
#else
#endif

    vu0_Stm0(&(scratchpad->kmat[HUMAN21_KUBI]));

#ifdef KANOKEN_DEBUG_PRINT
    printf("Kubi2    = %f %f %f\n",
	   scratchpad->kmat[HUMAN21_KUBI].m[3][0],
	   scratchpad->kmat[HUMAN21_KUBI].m[3][1],
	   scratchpad->kmat[HUMAN21_KUBI].m[3][2]);
#endif

    vu0_Mulm0m1m2();

    vu0_Ldm2(&(scratchpad->kmat[HUMAN21_HIDARI_KATA]));

#ifdef BP_PSX2_ASM
    asm volatile ("vmove.xyzw vf7,vf2");
#else
#endif
    vu0_Stm0(&(scratchpad->kmat[HUMAN21_MIGI_KATA]));

    vu0_Mulm0m1m2();

#ifdef BP_PSX2_ASM
    asm volatile ("vmove.xyzw vf7,vf3");
#else
#endif
    vu0_Stm0(&(scratchpad->kmat[HUMAN21_HIDARI_KATA]));

    v.vx=mdl[HUMAN21_ATAMA].tx;
    v.vy=mdl[HUMAN21_ATAMA].ty;
    v.vz=mdl[HUMAN21_ATAMA].tz;
    vu0_Ldv0(&v);
    vu0_Ldm0(&(scratchpad->kmat[HUMAN21_KUBI]));

    v.vx=mdl[HUMAN21_MIGI_UDE1].tx;
    v.vy=mdl[HUMAN21_MIGI_UDE1].ty;
    v.vz=mdl[HUMAN21_MIGI_UDE1].tz;
    vu0_Ldv1(&v);
    vu0_Ldm1(&(scratchpad->kmat[HUMAN21_MIGI_KATA]));

    v.vx=mdl[HUMAN21_HIDARI_UDE1].tx;
    v.vy=mdl[HUMAN21_HIDARI_UDE1].ty;
    v.vz=mdl[HUMAN21_HIDARI_UDE1].tz;
    vu0_Ldv2(&v);
    vu0_Ldm2(&(scratchpad->kmat[HUMAN21_HIDARI_KATA]));

    vu0_Mulv0m0v0();
    vu0_Mulv1m1v1();
    vu0_Mulv2m2v2();

    vu0_Stv0((FVECTOR *)&(scratchpad->kmat[HUMAN21_ATAMA].m[3][0]));
    vu0_Stv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_UDE1].m[3][0]));
    vu0_Stv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_UDE1].m[3][0]));

    v.vx=mdl[HUMAN21_MIGI_UDE2].tx;
    v.vy=mdl[HUMAN21_MIGI_UDE2].ty;
    v.vz=mdl[HUMAN21_MIGI_UDE2].tz;
    vu0_Ldv1(&v);
    vu0_Ldm1(&(scratchpad->kmat[HUMAN21_MIGI_UDE1]));

    v.vx=mdl[HUMAN21_HIDARI_UDE2].tx;
    v.vy=mdl[HUMAN21_HIDARI_UDE2].ty;
    v.vz=mdl[HUMAN21_HIDARI_UDE2].tz;
    vu0_Ldv2(&v);
    vu0_Ldm2(&(scratchpad->kmat[HUMAN21_HIDARI_UDE1]));

    vu0_Mulv1m1v1();
    vu0_Mulv2m2v2();

    vu0_Stv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0]));
    vu0_Stv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0]));


next1:

    hzd=HZX_NearHazardCheck(scratchpad->hzx_id,
			    (FVECTOR *)&(scratchpad->kmat[HUMAN21_ATAMA].m[3][0]),
			    HEAD_COLLISION, /* 検出半径 */
			    HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
			    HEAD_COLLISION /* 反発半径 */);

    if(hzd){
	float l0,l1,in;

	GetReactVector(&v1,
		       (FVECTOR *)&(scratchpad->kmat[HUMAN21_ATAMA].m[3][0]),
		       (FVECTOR *)&(scratchpad->kmat[HUMAN21_KOSHI].m[3][0]));

	vu0_Ldv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_ATAMA].m[3][0]));
	vu0_Ldv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_KUBI].m[3][0]));
	vu0_Ldv0(&v1);

	vu0_Subv2v1();

	vu0_Stv0(&v2);

	l0=vu0_VectorLength2v2();
	l1=vu0_VectorLength2v0();
	in=vu0_InnerProductv0v2();

	if(in>=0) l0=fpu_Sqrt(l0-in*in/l1);
	else l0=fpu_Sqrt(l0-in*in/l1+2*in-l1);

	if(fpu_Abs(v1.vz)>fpu_Abs(v1.vx)){
	    if(scratchpad->kmat[HUMAN21_ATAMA].m[3][0]
	       >=scratchpad->kmat[HUMAN21_KUBI].m[3][0]){

		v1.vx=scratchpad->kmat[HUMAN21_KUBI].m[3][0]+l0;
		v1.vy=scratchpad->kmat[HUMAN21_ATAMA].m[3][1];
		v1.vz+=scratchpad->kmat[HUMAN21_ATAMA].m[3][2];

		v2.vx=scratchpad->kmat[HUMAN21_KUBI].m[3][0]-l0;
		v2.vy=scratchpad->kmat[HUMAN21_ATAMA].m[3][1];
		v2.vz+=scratchpad->kmat[HUMAN21_ATAMA].m[3][2];
	    }
	    else{
		v1.vx=scratchpad->kmat[HUMAN21_KUBI].m[3][0]-l0;
		v1.vy=scratchpad->kmat[HUMAN21_ATAMA].m[3][1];
		v1.vz+=scratchpad->kmat[HUMAN21_ATAMA].m[3][2];

		v2.vx=scratchpad->kmat[HUMAN21_KUBI].m[3][0]+l0;
		v2.vy=scratchpad->kmat[HUMAN21_ATAMA].m[3][1];
		v2.vz+=scratchpad->kmat[HUMAN21_ATAMA].m[3][2];
	    }
	}
	else{
	    if(scratchpad->kmat[HUMAN21_ATAMA].m[3][2]
	       >=scratchpad->kmat[HUMAN21_KUBI].m[3][2]){

		v1.vx+=scratchpad->kmat[HUMAN21_ATAMA].m[3][0];
		v1.vy=scratchpad->kmat[HUMAN21_ATAMA].m[3][1];
		v1.vz=scratchpad->kmat[HUMAN21_KUBI].m[3][2]+l0;

		v2.vx+=scratchpad->kmat[HUMAN21_ATAMA].m[3][0];
		v2.vy=scratchpad->kmat[HUMAN21_ATAMA].m[3][1];
		v2.vz=scratchpad->kmat[HUMAN21_KUBI].m[3][2]-l0;
	    }
	    else{
		v1.vx+=scratchpad->kmat[HUMAN21_ATAMA].m[3][0];
		v1.vy=scratchpad->kmat[HUMAN21_ATAMA].m[3][1];
		v1.vz=scratchpad->kmat[HUMAN21_KUBI].m[3][2]-l0;

		v2.vx+=scratchpad->kmat[HUMAN21_ATAMA].m[3][0];
		v2.vy=scratchpad->kmat[HUMAN21_ATAMA].m[3][1];
		v2.vz=scratchpad->kmat[HUMAN21_KUBI].m[3][2]+l0;
	    }
	}

	vu0_Ldv2(&v1);
	vu0_Ldv1(&v2);
	vu0_Ldv0(&(scratchpad->x[HUMAN21_ATAMA]));

	vu0_Stv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_ATAMA].m[3][0]));

	vu0_Subv2v0();
	vu0_Subv0v1v0();
	l0=vu0_VectorLength2v2();
	l1=vu0_VectorLength2v0();

	if(l0>l1) vu0_Stv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_ATAMA].m[3][0]));
    }

    /* 頭の角度を求める */
    vu0_Ldv0((FVECTOR *)&(scratchpad->objmat[HUMAN21_ATAMA].m[3][0]));
    vu0_Ldv1((FVECTOR *)&(scratchpad->objmat[HUMAN21_KUBI].m[3][0]));

    vu0_Subv0v1();

    vu0_Stv0(&v);

    UTL_MakeQuat(&q,(FVECTOR *)&(scratchpad->kmat[HUMAN21_KUBI].m[3][0]),
		 (FVECTOR *)&(scratchpad->kmat[HUMAN21_ATAMA].m[3][0]),&v);

    /* 角度の制限を越えている場合は、補正をしない。*/
    if(q.vw<COSLIMIT){
	fpu_CopyVector((FVECTOR *)&(scratchpad->kmat[HUMAN21_ATAMA].m[3][0]),
		       (FVECTOR *)&(scratchpad->objmat[HUMAN21_ATAMA].m[3][0]));
    }


    hzd=HZX_NearHazardCheck(scratchpad->hzx_id,
			    (FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0]),
			    ARM_COLLISION, /* 検出半径 */
			    HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
			    ARM_COLLISION /* 反発半径 */);

    if(hzd){
	float l0,l1,in;

	GetReactVector(&v1,
		       (FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0]),
		       (FVECTOR *)&(scratchpad->kmat[HUMAN21_KOSHI].m[3][0]));

	vu0_Ldv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0]));
	vu0_Ldv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_UDE1].m[3][0]));
	vu0_Ldv0(&v1);

	vu0_Subv2v1();

	vu0_Stv0(&v2);

	l0=vu0_VectorLength2v2();
	l1=vu0_VectorLength2v0();
	in=vu0_InnerProductv0v2();

	if(in>=0) l0=fpu_Sqrt(l0-in*in/l1);
	else l0=fpu_Sqrt(l0-in*in/l1+2*in-l1);

	if(fpu_Abs(v1.vz)>fpu_Abs(v1.vx)){
	    if(scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0]
	       >=scratchpad->kmat[HUMAN21_MIGI_UDE1].m[3][0]){

		v1.vx=scratchpad->kmat[HUMAN21_MIGI_UDE1].m[3][0]+l0;
		v1.vy=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][1];
		v1.vz+=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][2];

		v2.vx=scratchpad->kmat[HUMAN21_MIGI_UDE1].m[3][0]-l0;
		v2.vy=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][1];
		v2.vz+=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][2];
	    }
	    else{
		v1.vx=scratchpad->kmat[HUMAN21_MIGI_UDE1].m[3][0]-l0;
		v1.vy=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][1];
		v1.vz+=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][2];

		v2.vx=scratchpad->kmat[HUMAN21_MIGI_UDE1].m[3][0]+l0;
		v2.vy=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][1];
		v2.vz+=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][2];
	    }
	}
	else{
	    if(scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][2]
	       >=scratchpad->kmat[HUMAN21_MIGI_UDE1].m[3][2]){

		v1.vx+=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0];
		v1.vy=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][1];
		v1.vz=scratchpad->kmat[HUMAN21_MIGI_UDE1].m[3][2]+l0;

		v2.vx+=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0];
		v2.vy=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][1];
		v2.vz=scratchpad->kmat[HUMAN21_MIGI_UDE1].m[3][2]-l0;
	    }
	    else{
		v1.vx+=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0];
		v1.vy=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][1];
		v1.vz=scratchpad->kmat[HUMAN21_MIGI_UDE1].m[3][2]-l0;

		v2.vx+=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0];
		v2.vy=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][1];
		v2.vz=scratchpad->kmat[HUMAN21_MIGI_UDE1].m[3][2]+l0;
	    }
	}

	vu0_Ldv2(&v1);
	vu0_Ldv1(&v2);
	vu0_Ldv0(&(scratchpad->x[HUMAN21_MIGI_UDE2]));

	vu0_Stv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0]));

	vu0_Subv2v0();
	vu0_Subv0v1v0();
	l0=vu0_VectorLength2v2();
	l1=vu0_VectorLength2v0();

	if(l0>l1) vu0_Stv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0]));
    }

    /* 肘、手の位置の再計算 */
    vu0_Ldv0((FVECTOR *)&(scratchpad->objmat[HUMAN21_MIGI_UDE2].m[3][0]));
    vu0_Ldv1((FVECTOR *)&(scratchpad->objmat[HUMAN21_MIGI_UDE1].m[3][0]));

    vu0_Subv0v1();

    vu0_Stv0(&v);

    UTL_MakeQuat(&q,(FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_UDE1].m[3][0]),
		 (FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0]),&v);

    /* 角度の制限を越えている場合は、補正をしない。*/
    if(q.vw<COSLIMIT){
	fpu_CopyVector((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0]),
		       (FVECTOR *)&(scratchpad->objmat[HUMAN21_MIGI_UDE2].m[3][0]));
	goto next2;
    }

    MT_QuatToMat(&m,&q);
    vu0_Ldm1(&m);
    vu0_Ldm2(&(scratchpad->kmat[HUMAN21_MIGI_UDE1]));

    v.vx=mdl[HUMAN21_MIGI_UDE2].tx;
    v.vy=mdl[HUMAN21_MIGI_UDE2].ty;
    v.vz=mdl[HUMAN21_MIGI_UDE2].tz;
    vu0_Ldv0(&v);

    vu0_Mulm0m1m2();

    vu0_Ldm1(&(scratchpad->kmat[HUMAN21_MIGI_UDE2]));
#ifdef BP_PSX2_ASM
    asm volatile ("vmove.xyzw vf7,vf15");
#else
#endif

    vu0_Mulv0m0v0();

    vu0_Stm0(&(scratchpad->kmat[HUMAN21_MIGI_UDE1]));

    v.vx=mdl[HUMAN21_MIGI_TE].tx;
    v.vy=mdl[HUMAN21_MIGI_TE].ty;
    v.vz=mdl[HUMAN21_MIGI_TE].tz;
    vu0_Ldv1(&v);

#ifdef BP_PSX2_ASM
    asm volatile ("vmove.xyzw vf11,vf1");
#else
#endif

    vu0_Mulv1m1v1();

    vu0_Stv0((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0]));
    vu0_Stv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_TE].m[3][0]));


next2:

    hzd=HZX_NearHazardCheck(scratchpad->hzx_id,
			    (FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0]),
			    ARM_COLLISION, /* 検出半径 */
			    HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
			    ARM_COLLISION /* 反発半径 */);

    if(hzd){
	float l0,l1,in;

	GetReactVector(&v1,
		       (FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0]),
		       (FVECTOR *)&(scratchpad->kmat[HUMAN21_KOSHI].m[3][0]));

	vu0_Ldv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0]));
	vu0_Ldv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_UDE1].m[3][0]));
	vu0_Ldv0(&v1);

	vu0_Subv2v1();

	vu0_Stv0(&v2);

	l0=vu0_VectorLength2v2();
	l1=vu0_VectorLength2v0();
	in=vu0_InnerProductv0v2();

	if(in>=0) l0=fpu_Sqrt(l0-in*in/l1);
	else l0=fpu_Sqrt(l0-in*in/l1+2*in-l1);

	if(fpu_Abs(v1.vz)>fpu_Abs(v1.vx)){
	    if(scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0]
	       >=scratchpad->kmat[HUMAN21_HIDARI_UDE1].m[3][0]){

		v1.vx=scratchpad->kmat[HUMAN21_HIDARI_UDE1].m[3][0]+l0;
		v1.vy=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][1];
		v1.vz+=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][2];

		v2.vx=scratchpad->kmat[HUMAN21_HIDARI_UDE1].m[3][0]-l0;
		v2.vy=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][1];
		v2.vz+=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][2];
	    }
	    else{
		v1.vx=scratchpad->kmat[HUMAN21_HIDARI_UDE1].m[3][0]-l0;
		v1.vy=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][1];
		v1.vz+=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][2];

		v2.vx=scratchpad->kmat[HUMAN21_HIDARI_UDE1].m[3][0]+l0;
		v2.vy=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][1];
		v2.vz+=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][2];
	    }
	}
	else{
	    if(scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][2]
	       >=scratchpad->kmat[HUMAN21_HIDARI_UDE1].m[3][2]){

		v1.vx+=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0];
		v1.vy=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][1];
		v1.vz=scratchpad->kmat[HUMAN21_HIDARI_UDE1].m[3][2]+l0;

		v2.vx+=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0];
		v2.vy=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][1];
		v2.vz=scratchpad->kmat[HUMAN21_HIDARI_UDE1].m[3][2]-l0;
	    }
	    else{
		v1.vx+=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0];
		v1.vy=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][1];
		v1.vz=scratchpad->kmat[HUMAN21_HIDARI_UDE1].m[3][2]-l0;

		v2.vx+=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0];
		v2.vy=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][1];
		v2.vz=scratchpad->kmat[HUMAN21_HIDARI_UDE1].m[3][2]+l0;
	    }
	}

	vu0_Ldv2(&v1);
	vu0_Ldv1(&v2);
	vu0_Ldv0(&(scratchpad->x[HUMAN21_HIDARI_UDE2]));

	vu0_Stv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0]));

	vu0_Subv2v0();
	vu0_Subv0v1v0();
	l0=vu0_VectorLength2v2();
	l1=vu0_VectorLength2v0();

	if(l0>l1) vu0_Stv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0]));
    }

    /* 肘、手の位置の再計算 */
    vu0_Ldv0((FVECTOR *)&(scratchpad->objmat[HUMAN21_HIDARI_UDE2].m[3][0]));
    vu0_Ldv1((FVECTOR *)&(scratchpad->objmat[HUMAN21_HIDARI_UDE1].m[3][0]));

    vu0_Subv0v1();

    vu0_Stv0(&v);

    UTL_MakeQuat(&q,(FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_UDE1].m[3][0]),
		 (FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0]),&v);

    /* 角度の制限を越えている場合は、補正をしない。*/
    if(q.vw<COSLIMIT){
	fpu_CopyVector((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0]),
		       (FVECTOR *)&(scratchpad->objmat[HUMAN21_HIDARI_UDE2].m[3][0]));
	goto next3;
    }

    MT_QuatToMat(&m,&q);
    vu0_Ldm1(&m);
    vu0_Ldm2(&(scratchpad->kmat[HUMAN21_HIDARI_UDE1]));

    v.vx=mdl[HUMAN21_HIDARI_UDE2].tx;
    v.vy=mdl[HUMAN21_HIDARI_UDE2].ty;
    v.vz=mdl[HUMAN21_HIDARI_UDE2].tz;
    vu0_Ldv0(&v);

    vu0_Mulm0m1m2();

    vu0_Ldm1(&(scratchpad->kmat[HUMAN21_HIDARI_UDE2]));
#ifdef BP_PSX2_ASM
    asm volatile ("vmove.xyzw vf7,vf15");
#else
#endif

    vu0_Mulv0m0v0();

    vu0_Stm0(&(scratchpad->kmat[HUMAN21_HIDARI_UDE1]));

    v.vx=mdl[HUMAN21_HIDARI_TE].tx;
    v.vy=mdl[HUMAN21_HIDARI_TE].ty;
    v.vz=mdl[HUMAN21_HIDARI_TE].tz;
    vu0_Ldv1(&v);

#ifdef BP_PSX2_ASM
    asm volatile ("vmove.xyzw vf11,vf1");
#else
#endif

    vu0_Mulv1m1v1();

    vu0_Stv0((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0]));
    vu0_Stv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][0]));


next3:

    hzd=HZX_NearHazardCheck(scratchpad->hzx_id,
			    (FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_TE].m[3][0]),
			    HAND_COLLISION, /* 検出半径 */
			    HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
			    HAND_COLLISION /* 反発半径 */);

    if(hzd){
	float l0,l1,in;

	GetReactVector(&v1,
		       (FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_TE].m[3][0]),
		       (FVECTOR *)&(scratchpad->kmat[HUMAN21_KOSHI].m[3][0]));

	vu0_Ldv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_TE].m[3][0]));
	vu0_Ldv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0]));
	vu0_Ldv0(&v1);

	vu0_Subv2v1();

	vu0_Stv0(&v2);

	l0=vu0_VectorLength2v2();
	l1=vu0_VectorLength2v0();
	in=vu0_InnerProductv0v2();

	if(in>=0) l0=fpu_Sqrt(l0-in*in/l1);
	else l0=fpu_Sqrt(l0-in*in/l1+2*in-l1);

	if(fpu_Abs(v1.vz)>fpu_Abs(v1.vx)){
	    if(scratchpad->kmat[HUMAN21_MIGI_TE].m[3][0]
	       >=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0]){

		v1.vx=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0]+l0;
		v1.vy=scratchpad->kmat[HUMAN21_MIGI_TE].m[3][1];
		v1.vz+=scratchpad->kmat[HUMAN21_MIGI_TE].m[3][2];

		v2.vx=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0]-l0;
		v2.vy=scratchpad->kmat[HUMAN21_MIGI_TE].m[3][1];
		v2.vz+=scratchpad->kmat[HUMAN21_MIGI_TE].m[3][2];
	    }
	    else{
		v1.vx=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0]-l0;
		v1.vy=scratchpad->kmat[HUMAN21_MIGI_TE].m[3][1];
		v1.vz+=scratchpad->kmat[HUMAN21_MIGI_TE].m[3][2];

		v2.vx=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0]+l0;
		v2.vy=scratchpad->kmat[HUMAN21_MIGI_TE].m[3][1];
		v2.vz+=scratchpad->kmat[HUMAN21_MIGI_TE].m[3][2];
	    }
	}
	else{
	    if(scratchpad->kmat[HUMAN21_MIGI_TE].m[3][2]
	       >=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][2]){

		v1.vx+=scratchpad->kmat[HUMAN21_MIGI_TE].m[3][0];
		v1.vy=scratchpad->kmat[HUMAN21_MIGI_TE].m[3][1];
		v1.vz=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][2]+l0;

		v2.vx+=scratchpad->kmat[HUMAN21_MIGI_TE].m[3][0];
		v2.vy=scratchpad->kmat[HUMAN21_MIGI_TE].m[3][1];
		v2.vz=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][2]-l0;
	    }
	    else{
		v1.vx+=scratchpad->kmat[HUMAN21_MIGI_TE].m[3][0];
		v1.vy=scratchpad->kmat[HUMAN21_MIGI_TE].m[3][1];
		v1.vz=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][2]-l0;

		v2.vx+=scratchpad->kmat[HUMAN21_MIGI_TE].m[3][0];
		v2.vy=scratchpad->kmat[HUMAN21_MIGI_TE].m[3][1];
		v2.vz=scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][2]+l0;
	    }
	}

	vu0_Ldv2(&v1);
	vu0_Ldv1(&v2);
	vu0_Ldv0(&(scratchpad->x[HUMAN21_MIGI_TE]));

	vu0_Stv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_TE].m[3][0]));

	vu0_Subv2v0();
	vu0_Subv0v1v0();
	l0=vu0_VectorLength2v2();
	l1=vu0_VectorLength2v0();

	if(l0>l1) vu0_Stv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_TE].m[3][0]));
    }

    /* 角度の計算 */
    vu0_Ldv0((FVECTOR *)&(scratchpad->objmat[HUMAN21_MIGI_TE].m[3][0]));
    vu0_Ldv1((FVECTOR *)&(scratchpad->objmat[HUMAN21_MIGI_UDE2].m[3][0]));

    vu0_Subv0v1();

    vu0_Stv0(&v);

    UTL_MakeQuat(&q,(FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_UDE2].m[3][0]),
		 (FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_TE].m[3][0]),&v);

    /* 角度の制限を越えている場合は、補正をしない。*/
    if(q.vw<COSLIMIT){
	fpu_CopyVector((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_TE].m[3][0]),
		       (FVECTOR *)&(scratchpad->objmat[HUMAN21_MIGI_TE].m[3][0]));
    }



    hzd=HZX_NearHazardCheck(scratchpad->hzx_id,
			    (FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][0]),
			    HAND_COLLISION, /* 検出半径 */
			    HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
			    HAND_COLLISION /* 反発半径 */);

    if(hzd){
	float l0,l1,in;

	GetReactVector(&v1,
		       (FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][0]),
		       (FVECTOR *)&(scratchpad->kmat[HUMAN21_KOSHI].m[3][0]));

	vu0_Ldv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][0]));
	vu0_Ldv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0]));
	vu0_Ldv0(&v1);

	vu0_Subv2v1();

	vu0_Stv0(&v2);

	l0=vu0_VectorLength2v2();
	l1=vu0_VectorLength2v0();
	in=vu0_InnerProductv0v2();

	if(in>=0) l0=fpu_Sqrt(l0-in*in/l1);
	else l0=fpu_Sqrt(l0-in*in/l1+2*in-l1);

	if(fpu_Abs(v1.vz)>fpu_Abs(v1.vx)){
	    if(scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][0]
	       >=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0]){

		v1.vx=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0]+l0;
		v1.vy=scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][1];
		v1.vz+=scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][2];

		v2.vx=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0]-l0;
		v2.vy=scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][1];
		v2.vz+=scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][2];
	    }
	    else{
		v1.vx=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0]-l0;
		v1.vy=scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][1];
		v1.vz+=scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][2];

		v2.vx=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0]+l0;
		v2.vy=scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][1];
		v2.vz+=scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][2];
	    }
	}
	else{
	    if(scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][2]
	       >=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][2]){

		v1.vx+=scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][0];
		v1.vy=scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][1];
		v1.vz=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][2]+l0;

		v2.vx+=scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][0];
		v2.vy=scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][1];
		v2.vz=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][2]-l0;
	    }
	    else{
		v1.vx+=scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][0];
		v1.vy=scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][1];
		v1.vz=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][2]-l0;

		v2.vx+=scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][0];
		v2.vy=scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][1];
		v2.vz=scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][2]+l0;
	    }
	}

	vu0_Ldv2(&v1);
	vu0_Ldv1(&v2);
	vu0_Ldv0(&(scratchpad->x[HUMAN21_HIDARI_TE]));

	vu0_Stv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][0]));

	vu0_Subv2v0();
	vu0_Subv0v1v0();
	l0=vu0_VectorLength2v2();
	l1=vu0_VectorLength2v0();

	if(l0>l1) vu0_Stv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][0]));
    }

    /* 角度の計算 */
    vu0_Ldv0((FVECTOR *)&(scratchpad->objmat[HUMAN21_HIDARI_TE].m[3][0]));
    vu0_Ldv1((FVECTOR *)&(scratchpad->objmat[HUMAN21_HIDARI_UDE2].m[3][0]));

    vu0_Subv0v1();

    vu0_Stv0(&v);

    UTL_MakeQuat(&q,(FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_UDE2].m[3][0]),
		 (FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][0]),&v);

    /* 角度の制限を越えている場合は、補正をしない。*/
    if(q.vw<COSLIMIT){
	fpu_CopyVector((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][0]),
		       (FVECTOR *)&(scratchpad->objmat[HUMAN21_HIDARI_TE].m[3][0]));
    }



    hzd=HZX_NearHazardCheck(scratchpad->hzx_id,
			    (FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][0]),
			    NEE_COLLISION, /* 検出半径 */
			    HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
			    NEE_COLLISION /* 反発半径 */);

    if(hzd){
	float l0,l1,in;

	GetReactVector(&v1,
		       (FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][0]),
		       (FVECTOR *)&(scratchpad->kmat[HUMAN21_KOSHI].m[3][0]));

	vu0_Ldv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][0]));
	vu0_Ldv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_ASHI1].m[3][0]));
	vu0_Ldv0(&v1);

	vu0_Subv2v1();

	vu0_Stv0(&v2);

	l0=vu0_VectorLength2v2();
	l1=vu0_VectorLength2v0();
	in=vu0_InnerProductv0v2();

	if(in>=0) l0=fpu_Sqrt(l0-in*in/l1);
	else l0=fpu_Sqrt(l0-in*in/l1+2*in-l1);

	if(fpu_Abs(v1.vz)>fpu_Abs(v1.vx)){
	    if(scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][0]
	       >=scratchpad->kmat[HUMAN21_MIGI_ASHI1].m[3][0]){

		v1.vx=scratchpad->kmat[HUMAN21_MIGI_ASHI1].m[3][0]+l0;
		v1.vy=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][1];
		v1.vz+=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][2];

		v2.vx=scratchpad->kmat[HUMAN21_MIGI_ASHI1].m[3][0]-l0;
		v2.vy=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][1];
		v2.vz+=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][2];
	    }
	    else{
		v1.vx=scratchpad->kmat[HUMAN21_MIGI_ASHI1].m[3][0]-l0;
		v1.vy=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][1];
		v1.vz+=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][2];

		v2.vx=scratchpad->kmat[HUMAN21_MIGI_ASHI1].m[3][0]+l0;
		v2.vy=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][1];
		v2.vz+=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][2];
	    }
	}
	else{
	    if(scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][2]
	       >=scratchpad->kmat[HUMAN21_MIGI_ASHI1].m[3][2]){

		v1.vx+=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][0];
		v1.vy=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][1];
		v1.vz=scratchpad->kmat[HUMAN21_MIGI_ASHI1].m[3][2]+l0;

		v2.vx+=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][0];
		v2.vy=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][1];
		v2.vz=scratchpad->kmat[HUMAN21_MIGI_ASHI1].m[3][2]-l0;
	    }
	    else{
		v1.vx+=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][0];
		v1.vy=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][1];
		v1.vz=scratchpad->kmat[HUMAN21_MIGI_ASHI1].m[3][2]-l0;

		v2.vx+=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][0];
		v2.vy=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][1];
		v2.vz=scratchpad->kmat[HUMAN21_MIGI_ASHI1].m[3][2]+l0;
	    }
	}

	vu0_Ldv2(&v1);
	vu0_Ldv1(&v2);
	vu0_Ldv0(&(scratchpad->x[HUMAN21_MIGI_ASHI2]));

	vu0_Stv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][0]));

	vu0_Subv2v0();
	vu0_Subv0v1v0();
	l0=vu0_VectorLength2v2();
	l1=vu0_VectorLength2v0();

	if(l0>l1) vu0_Stv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][0]));
    }

    /* 膝、踵の位置の再計算 */
    vu0_Ldv0((FVECTOR *)&(scratchpad->objmat[HUMAN21_MIGI_ASHI2].m[3][0]));
    vu0_Ldv1((FVECTOR *)&(scratchpad->objmat[HUMAN21_MIGI_ASHI1].m[3][0]));

    vu0_Subv0v1();

    vu0_Stv0(&v);

    UTL_MakeQuat(&q,(FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_ASHI1].m[3][0]),
		 (FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][0]),&v);

    /* 角度の制限を越えている場合は、補正をしない。*/
    if(q.vw<COSLIMIT){
	fpu_CopyVector((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][0]),
		       (FVECTOR *)&(scratchpad->objmat[HUMAN21_MIGI_ASHI2].m[3][0]));
	goto next4;
    }

    MT_QuatToMat(&m,&q);
    vu0_Ldm1(&m);
    vu0_Ldm2(&(scratchpad->kmat[HUMAN21_MIGI_ASHI1]));

    v.vx=mdl[HUMAN21_MIGI_ASHI2].tx;
    v.vy=mdl[HUMAN21_MIGI_ASHI2].ty;
    v.vz=mdl[HUMAN21_MIGI_ASHI2].tz;
    vu0_Ldv0(&v);

    vu0_Mulm0m1m2();

    vu0_Ldm1(&(scratchpad->kmat[HUMAN21_MIGI_ASHI2]));
#ifdef BP_PSX2_ASM
    asm volatile ("vmove.xyzw vf7,vf15");
#else
#endif

    vu0_Mulv0m0v0();

    vu0_Stm0(&(scratchpad->kmat[HUMAN21_MIGI_ASHI1]));

    v.vx=mdl[HUMAN21_MIGI_TE].tx;
    v.vy=mdl[HUMAN21_MIGI_TE].ty;
    v.vz=mdl[HUMAN21_MIGI_TE].tz;
    vu0_Ldv1(&v);

#ifdef BP_PSX2_ASM
    asm volatile ("vmove.xyzw vf11,vf1");
#else
#endif

    vu0_Mulv1m1v1();

    vu0_Stv0((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][0]));
    vu0_Stv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][0]));


next4:

    hzd=HZX_NearHazardCheck(scratchpad->hzx_id,
			    (FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][0]),
			    NEE_COLLISION, /* 検出半径 */
			    HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
			    NEE_COLLISION /* 反発半径 */);

    if(hzd){
	float l0,l1,in;

	GetReactVector(&v1,
		       (FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][0]),
		       (FVECTOR *)&(scratchpad->kmat[HUMAN21_KOSHI].m[3][0]));

	vu0_Ldv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][0]));
	vu0_Ldv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_ASHI1].m[3][0]));
	vu0_Ldv0(&v1);

	vu0_Subv2v1();

	vu0_Stv0(&v2);

	l0=vu0_VectorLength2v2();
	l1=vu0_VectorLength2v0();
	in=vu0_InnerProductv0v2();

	if(in>=0) l0=fpu_Sqrt(l0-in*in/l1);
	else l0=fpu_Sqrt(l0-in*in/l1+2*in-l1);

	if(fpu_Abs(v1.vz)>fpu_Abs(v1.vx)){
	    if(scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][0]
	       >=scratchpad->kmat[HUMAN21_HIDARI_ASHI1].m[3][0]){

		v1.vx=scratchpad->kmat[HUMAN21_HIDARI_ASHI1].m[3][0]+l0;
		v1.vy=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][1];
		v1.vz+=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][2];

		v2.vx=scratchpad->kmat[HUMAN21_HIDARI_ASHI1].m[3][0]-l0;
		v2.vy=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][1];
		v2.vz+=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][2];
	    }
	    else{
		v1.vx=scratchpad->kmat[HUMAN21_HIDARI_ASHI1].m[3][0]-l0;
		v1.vy=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][1];
		v1.vz+=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][2];

		v2.vx=scratchpad->kmat[HUMAN21_HIDARI_ASHI1].m[3][0]+l0;
		v2.vy=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][1];
		v2.vz+=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][2];
	    }
	}
	else{
	    if(scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][2]
	       >=scratchpad->kmat[HUMAN21_HIDARI_ASHI1].m[3][2]){

		v1.vx+=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][0];
		v1.vy=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][1];
		v1.vz=scratchpad->kmat[HUMAN21_HIDARI_ASHI1].m[3][2]+l0;

		v2.vx+=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][0];
		v2.vy=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][1];
		v2.vz=scratchpad->kmat[HUMAN21_HIDARI_ASHI1].m[3][2]-l0;
	    }
	    else{
		v1.vx+=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][0];
		v1.vy=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][1];
		v1.vz=scratchpad->kmat[HUMAN21_HIDARI_ASHI1].m[3][2]-l0;

		v2.vx+=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][0];
		v2.vy=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][1];
		v2.vz=scratchpad->kmat[HUMAN21_HIDARI_ASHI1].m[3][2]+l0;
	    }
	}

	vu0_Ldv2(&v1);
	vu0_Ldv1(&v2);
	vu0_Ldv0(&(scratchpad->x[HUMAN21_HIDARI_ASHI2]));

	vu0_Stv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][0]));

	vu0_Subv2v0();
	vu0_Subv0v1v0();
	l0=vu0_VectorLength2v2();
	l1=vu0_VectorLength2v0();

	if(l0>l1) vu0_Stv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][0]));
    }

    /* 膝、踵の位置の再計算 */
    vu0_Ldv0((FVECTOR *)&(scratchpad->objmat[HUMAN21_HIDARI_ASHI2].m[3][0]));
    vu0_Ldv1((FVECTOR *)&(scratchpad->objmat[HUMAN21_HIDARI_ASHI1].m[3][0]));

    vu0_Subv0v1();

    vu0_Stv0(&v);

    UTL_MakeQuat(&q,(FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_ASHI1].m[3][0]),
		 (FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][0]),&v);

    /* 角度の制限を越えている場合は、補正をしない。*/
    if(q.vw<COSLIMIT){
	fpu_CopyVector((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][0]),
		       (FVECTOR *)&(scratchpad->objmat[HUMAN21_HIDARI_ASHI2].m[3][0]));
	goto next5;
    }

    MT_QuatToMat(&m,&q);
    vu0_Ldm1(&m);
    vu0_Ldm2(&(scratchpad->kmat[HUMAN21_HIDARI_ASHI1]));

    v.vx=mdl[HUMAN21_HIDARI_ASHI2].tx;
    v.vy=mdl[HUMAN21_HIDARI_ASHI2].ty;
    v.vz=mdl[HUMAN21_HIDARI_ASHI2].tz;
    vu0_Ldv0(&v);

    vu0_Mulm0m1m2();

    vu0_Ldm1(&(scratchpad->kmat[HUMAN21_HIDARI_ASHI2]));
#ifdef BP_PSX2_ASM
    asm volatile ("vmove.xyzw vf7,vf15");
#else
#endif

    vu0_Mulv0m0v0();

    vu0_Stm0(&(scratchpad->kmat[HUMAN21_HIDARI_ASHI1]));

    v.vx=mdl[HUMAN21_HIDARI_TE].tx;
    v.vy=mdl[HUMAN21_HIDARI_TE].ty;
    v.vz=mdl[HUMAN21_HIDARI_TE].tz;
    vu0_Ldv1(&v);

#ifdef BP_PSX2_ASM
    asm volatile ("vmove.xyzw vf11,vf1");
#else
#endif

    vu0_Mulv1m1v1();

    vu0_Stv0((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][0]));
    vu0_Stv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][0]));


next5:

    hzd=HZX_NearHazardCheck(scratchpad->hzx_id,
			    (FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][0]),
			    HEEL_COLLISION, /* 検出半径 */
			    HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
			    HEEL_COLLISION /* 反発半径 */);

    if(hzd){
	float l0,l1,in;

	GetReactVector(&v1,
		       (FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][0]),
		       (FVECTOR *)&(scratchpad->kmat[HUMAN21_KOSHI].m[3][0]));

	vu0_Ldv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][0]));
	vu0_Ldv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][0]));
	vu0_Ldv0(&v1);

	vu0_Subv2v1();

	vu0_Stv0(&v2);

	l0=vu0_VectorLength2v2();
	l1=vu0_VectorLength2v0();
	in=vu0_InnerProductv0v2();

	if(in>=0) l0=fpu_Sqrt(l0-in*in/l1);
	else l0=fpu_Sqrt(l0-in*in/l1+2*in-l1);

	if(fpu_Abs(v1.vz)>fpu_Abs(v1.vx)){
	    if(scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][0]
	       >=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][0]){

		v1.vx=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][0]+l0;
		v1.vy=scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][1];
		v1.vz+=scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][2];

		v2.vx=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][0]-l0;
		v2.vy=scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][1];
		v2.vz+=scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][2];
	    }
	    else{
		v1.vx=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][0]-l0;
		v1.vy=scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][1];
		v1.vz+=scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][2];

		v2.vx=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][0]+l0;
		v2.vy=scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][1];
		v2.vz+=scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][2];
	    }
	}
	else{
	    if(scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][2]
	       >=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][2]){

		v1.vx+=scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][0];
		v1.vy=scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][1];
		v1.vz=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][2]+l0;

		v2.vx+=scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][0];
		v2.vy=scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][1];
		v2.vz=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][2]-l0;
	    }
	    else{
		v1.vx+=scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][0];
		v1.vy=scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][1];
		v1.vz=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][2]-l0;

		v2.vx+=scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][0];
		v2.vy=scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][1];
		v2.vz=scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][2]+l0;
	    }
	}

	vu0_Ldv2(&v1);
	vu0_Ldv1(&v2);
	vu0_Ldv0(&(scratchpad->x[HUMAN21_MIGI_KAKATO]));

	vu0_Stv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][0]));

	vu0_Subv2v0();
	vu0_Subv0v1v0();
	l0=vu0_VectorLength2v2();
	l1=vu0_VectorLength2v0();

	if(l0>l1) vu0_Stv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][0]));
    }

    /* 角度の計算 */
    vu0_Ldv0((FVECTOR *)&(scratchpad->objmat[HUMAN21_MIGI_KAKATO].m[3][0]));
    vu0_Ldv1((FVECTOR *)&(scratchpad->objmat[HUMAN21_MIGI_ASHI2].m[3][0]));

    vu0_Subv0v1();

    vu0_Stv0(&v);

    UTL_MakeQuat(&q,(FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_ASHI2].m[3][0]),
		 (FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][0]),&v);

    /* 角度の制限を越えている場合は、補正をしない。*/
    if(q.vw<COSLIMIT){
	fpu_CopyVector((FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][0]),
		       (FVECTOR *)&(scratchpad->objmat[HUMAN21_MIGI_KAKATO].m[3][0]));
    }



    hzd=HZX_NearHazardCheck(scratchpad->hzx_id,
			    (FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][0]),
			    HEEL_COLLISION, /* 検出半径 */
			    HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
			    HEEL_COLLISION /* 反発半径 */);

    if(hzd){
	float l0,l1,in;

	GetReactVector(&v1,
		       (FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][0]),
		       (FVECTOR *)&(scratchpad->kmat[HUMAN21_KOSHI].m[3][0]));

	vu0_Ldv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][0]));
	vu0_Ldv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][0]));
	vu0_Ldv0(&v1);

	vu0_Subv2v1();

	vu0_Stv0(&v2);

	l0=vu0_VectorLength2v2();
	l1=vu0_VectorLength2v0();
	in=vu0_InnerProductv0v2();

	if(in>=0) l0=fpu_Sqrt(l0-in*in/l1);
	else l0=fpu_Sqrt(l0-in*in/l1+2*in-l1);

	if(fpu_Abs(v1.vz)>fpu_Abs(v1.vx)){
	    if(scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][0]
	       >=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][0]){

		v1.vx=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][0]+l0;
		v1.vy=scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][1];
		v1.vz+=scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][2];

		v2.vx=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][0]-l0;
		v2.vy=scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][1];
		v2.vz+=scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][2];
	    }
	    else{
		v1.vx=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][0]-l0;
		v1.vy=scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][1];
		v1.vz+=scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][2];

		v2.vx=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][0]+l0;
		v2.vy=scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][1];
		v2.vz+=scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][2];
	    }
	}
	else{
	    if(scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][2]
	       >=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][2]){

		v1.vx+=scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][0];
		v1.vy=scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][1];
		v1.vz=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][2]+l0;

		v2.vx+=scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][0];
		v2.vy=scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][1];
		v2.vz=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][2]-l0;
	    }
	    else{
		v1.vx+=scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][0];
		v1.vy=scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][1];
		v1.vz=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][2]-l0;

		v2.vx+=scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][0];
		v2.vy=scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][1];
		v2.vz=scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][2]+l0;
	    }
	}

	vu0_Ldv2(&v1);
	vu0_Ldv1(&v2);
	vu0_Ldv0(&(scratchpad->x[HUMAN21_HIDARI_KAKATO]));

	vu0_Stv2((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][0]));

	vu0_Subv2v0();
	vu0_Subv0v1v0();
	l0=vu0_VectorLength2v2();
	l1=vu0_VectorLength2v0();

	if(l0>l1) vu0_Stv1((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][0]));
    }

    /* 角度の計算 */
    vu0_Ldv0((FVECTOR *)&(scratchpad->objmat[HUMAN21_HIDARI_KAKATO].m[3][0]));
    vu0_Ldv1((FVECTOR *)&(scratchpad->objmat[HUMAN21_HIDARI_ASHI2].m[3][0]));

    vu0_Subv0v1();

    vu0_Stv0(&v);

    UTL_MakeQuat(&q,(FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_ASHI2].m[3][0]),
		 (FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][0]),&v);

    /* 角度の制限を越えている場合は、補正をしない。*/
    if(q.vw<COSLIMIT){
	fpu_CopyVector((FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][0]),
		       (FVECTOR *)&(scratchpad->objmat[HUMAN21_HIDARI_KAKATO].m[3][0]));
    }
}


#define DIFFROT_SLERP_PARAM	(1.0/4.0f)


static void CalcKQuat(struct _scratchpad *scratchpad,CORPSE_PARAMETER *corpsep)
{
    FVECTOR q,v;
    FVECTOR inv,tmpq;


    vu0_Ldv0((FVECTOR *)&(scratchpad->objmat[HUMAN21_KUBI].m[3][0]));
    vu0_Ldv1((FVECTOR *)&(scratchpad->objmat[HUMAN21_MUNE].m[3][0]));

    vu0_Subv0v1();

    vu0_Stv0(&v);

    UTL_MakeQuat(&q,(FVECTOR *)&(scratchpad->kmat[HUMAN21_MUNE].m[3][0]),
		 (FVECTOR *)&(scratchpad->kmat[HUMAN21_KUBI].m[3][0]),&v);

#ifdef KANOKEN_DEBUG_PRINT
    printf("Root  = %f %f %f %f\n",
	   scratchpad->root.vx,scratchpad->root.vy,scratchpad->root.vz,scratchpad->root.vw);
    printf("Quat1 = %f %f %f %f\n",
	   scratchpad->abs_rots[HUMAN21_MUNE].vx,
	   scratchpad->abs_rots[HUMAN21_MUNE].vy,
	   scratchpad->abs_rots[HUMAN21_MUNE].vz,
	   scratchpad->abs_rots[HUMAN21_MUNE].vw);
    printf("Quat2 = %f %f %f %f\n",q.vx,q.vy,q.vz,q.vw);
#endif

    MT_QuatMul(&tmpq,&(scratchpad->root),&(scratchpad->abs_rots[HUMAN21_MUNE]));
    MT_QuatInverse(&inv,&tmpq);

#ifdef KANOKEN_DEBUG_PRINT
    printf("Quat2.5 = %f %f %f %f\n",tmpq.vx,tmpq.vy,tmpq.vz,tmpq.vw);
#endif

    MT_QuatSlerp(&(corpsep->diff_rots[HUMAN21_MUNE]),
		 &(corpsep->diff_rots[HUMAN21_MUNE]),&q,DIFFROT_SLERP_PARAM);
    MT_QuatNormalize(&(corpsep->diff_rots[HUMAN21_MUNE]),&(corpsep->diff_rots[HUMAN21_MUNE]));
    MT_QuatMul(&tmpq,&(corpsep->diff_rots[HUMAN21_MUNE]),&tmpq);

    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_MUNE]),&(scratchpad->inv_root),&tmpq);
    MT_QuatMul(&q,&tmpq,&inv);

    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_MIGI_KATA]),
	       &q,&(scratchpad->abs_rots[HUMAN21_MIGI_KATA]));
    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]),
	       &q,&(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]));



    vu0_Ldv0((FVECTOR *)&(scratchpad->objmat[HUMAN21_ATAMA].m[3][0]));
    vu0_Ldv1((FVECTOR *)&(scratchpad->objmat[HUMAN21_KUBI].m[3][0]));

    vu0_Subv0v1();

    vu0_Stv0(&v);

    UTL_MakeQuat(&q,(FVECTOR *)&(scratchpad->kmat[HUMAN21_KUBI].m[3][0]),
		 (FVECTOR *)&(scratchpad->kmat[HUMAN21_ATAMA].m[3][0]),&v);

    MT_QuatMul(&tmpq,&(scratchpad->root),&(scratchpad->abs_rots[HUMAN21_KUBI]));
    MT_QuatInverse(&inv,&tmpq);

    MT_QuatSlerp(&(corpsep->diff_rots[HUMAN21_KUBI]),
		 &(corpsep->diff_rots[HUMAN21_KUBI]),&q,DIFFROT_SLERP_PARAM);
    MT_QuatNormalize(&(corpsep->diff_rots[HUMAN21_KUBI]),&(corpsep->diff_rots[HUMAN21_KUBI]));
    MT_QuatMul(&tmpq,&(corpsep->diff_rots[HUMAN21_KUBI]),&tmpq);

    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_KUBI]),&(scratchpad->inv_root),&tmpq);
    MT_QuatMul(&q,&tmpq,&inv);

    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_ATAMA]),
	       &q,&(scratchpad->abs_rots[HUMAN21_ATAMA]));




    vu0_Ldv0((FVECTOR *)&(scratchpad->objmat[HUMAN21_MIGI_TE].m[3][0]));
    vu0_Ldv1((FVECTOR *)&(scratchpad->objmat[HUMAN21_MIGI_UDE1].m[3][0]));

    vu0_Subv0v1();

    vu0_Stv0(&v);

    UTL_MakeQuat(&q,(FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_UDE1].m[3][0]),
		 (FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_TE].m[3][0]),&v);

    MT_QuatMul(&tmpq,&(scratchpad->root),&(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]));
    MT_QuatInverse(&inv,&tmpq);

    MT_QuatSlerp(&(corpsep->diff_rots[HUMAN21_MIGI_UDE1]),
		 &(corpsep->diff_rots[HUMAN21_MIGI_UDE1]),&q,DIFFROT_SLERP_PARAM);
    MT_QuatNormalize(&(corpsep->diff_rots[HUMAN21_MIGI_UDE1]),
		     &(corpsep->diff_rots[HUMAN21_MIGI_UDE1]));
    MT_QuatMul(&tmpq,&(corpsep->diff_rots[HUMAN21_MIGI_UDE1]),&tmpq);

    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]),&(scratchpad->inv_root),&tmpq);
    MT_QuatMul(&q,&tmpq,&inv);

    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]),
	       &q,&(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]));
    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_MIGI_TE]),
	       &q,&(scratchpad->abs_rots[HUMAN21_MIGI_TE]));

    //printf("Q(migi) = %f %f %f %f\n",q.vx,q.vy,q.vz,q.vw);



    vu0_Ldv0((FVECTOR *)&(scratchpad->objmat[HUMAN21_HIDARI_TE].m[3][0]));
    vu0_Ldv1((FVECTOR *)&(scratchpad->objmat[HUMAN21_HIDARI_UDE1].m[3][0]));

    vu0_Subv0v1();

    vu0_Stv0(&v);

    UTL_MakeQuat(&q,(FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_UDE1].m[3][0]),
		 (FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_TE].m[3][0]),&v);

    MT_QuatMul(&tmpq,&(scratchpad->root),&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]));
    MT_QuatInverse(&inv,&tmpq);

    MT_QuatSlerp(&(corpsep->diff_rots[HUMAN21_HIDARI_UDE1]),
		 &(corpsep->diff_rots[HUMAN21_HIDARI_UDE1]),&q,DIFFROT_SLERP_PARAM);
    MT_QuatNormalize(&(corpsep->diff_rots[HUMAN21_HIDARI_UDE1]),
		     &(corpsep->diff_rots[HUMAN21_HIDARI_UDE1]));
    MT_QuatMul(&tmpq,&(corpsep->diff_rots[HUMAN21_HIDARI_UDE1]),&tmpq);

    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]),&(scratchpad->inv_root),&tmpq);
    MT_QuatMul(&q,&tmpq,&inv);

    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]),
	       &q,&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]));
    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_HIDARI_TE]),
	       &q,&(scratchpad->abs_rots[HUMAN21_HIDARI_TE]));

    //printf("Q(hidari) = %f %f %f %f\n",q.vx,q.vy,q.vz,q.vw);




    vu0_Ldv0((FVECTOR *)&(scratchpad->objmat[HUMAN21_MIGI_KAKATO].m[3][0]));
    vu0_Ldv1((FVECTOR *)&(scratchpad->objmat[HUMAN21_MIGI_ASHI1].m[3][0]));

    vu0_Subv0v1();

    vu0_Stv0(&v);

    UTL_MakeQuat(&q,(FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_ASHI1].m[3][0]),
		 (FVECTOR *)&(scratchpad->kmat[HUMAN21_MIGI_KAKATO].m[3][0]),&v);

    MT_QuatMul(&tmpq,&(scratchpad->root),&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]));
    MT_QuatInverse(&inv,&tmpq);

    MT_QuatSlerp(&(corpsep->diff_rots[HUMAN21_MIGI_ASHI1]),
		 &(corpsep->diff_rots[HUMAN21_MIGI_ASHI1]),&q,DIFFROT_SLERP_PARAM);
    MT_QuatNormalize(&(corpsep->diff_rots[HUMAN21_MIGI_ASHI1]),
		     &(corpsep->diff_rots[HUMAN21_MIGI_ASHI1]));
    MT_QuatMul(&tmpq,&(corpsep->diff_rots[HUMAN21_MIGI_ASHI1]),&tmpq);

    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),&(scratchpad->inv_root),&tmpq);
    MT_QuatMul(&q,&tmpq,&inv);

    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
	       &q,&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_MIGI_KAKATO]),
	       &q,&(scratchpad->abs_rots[HUMAN21_MIGI_KAKATO]));
    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_MIGI_TSUMASAKI]),
	       &q,&(scratchpad->abs_rots[HUMAN21_MIGI_TSUMASAKI]));




    vu0_Ldv0((FVECTOR *)&(scratchpad->objmat[HUMAN21_HIDARI_KAKATO].m[3][0]));
    vu0_Ldv1((FVECTOR *)&(scratchpad->objmat[HUMAN21_HIDARI_ASHI1].m[3][0]));

    vu0_Subv0v1();

    vu0_Stv0(&v);

    UTL_MakeQuat(&q,(FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_ASHI1].m[3][0]),
		 (FVECTOR *)&(scratchpad->kmat[HUMAN21_HIDARI_KAKATO].m[3][0]),&v);

    MT_QuatMul(&tmpq,&(scratchpad->root),&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]));
    MT_QuatInverse(&inv,&tmpq);

    MT_QuatSlerp(&(corpsep->diff_rots[HUMAN21_HIDARI_ASHI1]),
		 &(corpsep->diff_rots[HUMAN21_HIDARI_ASHI1]),&q,DIFFROT_SLERP_PARAM);
    MT_QuatNormalize(&(corpsep->diff_rots[HUMAN21_HIDARI_ASHI1]),
		     &(corpsep->diff_rots[HUMAN21_HIDARI_ASHI1]));
    MT_QuatMul(&tmpq,&(corpsep->diff_rots[HUMAN21_HIDARI_ASHI1]),&tmpq);

    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),&(scratchpad->inv_root),&tmpq);
    MT_QuatMul(&q,&tmpq,&inv);

    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
	       &q,&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_HIDARI_KAKATO]),
	       &q,&(scratchpad->abs_rots[HUMAN21_HIDARI_KAKATO]));
    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_HIDARI_TSUMASAKI]),
	       &q,&(scratchpad->abs_rots[HUMAN21_HIDARI_TSUMASAKI]));
}



static void HoldCorpse(Work *work)
{
    OBJECT *obj=work->obj;
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
    int size=HUMAN_MODEL_OBJN;
    int i;

    for(i=0;i<size;i++){
	fpu_CopyVector(&(obj->m_ctrl->abs_rots[i]),&(corpsep->abs_rots[i]));
	fpu_ClearVector(&(corpsep->diff_rots[i]));
	corpsep->diff_rots[i].vw=1.0f;
	fpu_CopyVector(&(corpsep->x[i]),(FVECTOR *)&(obj->objs->objs[i].world.m[3][0]));
	fpu_CopyVector(&(corpsep->basex[i]),(FVECTOR *)&(obj->objs->objs[i].world.m[3][0]));
	fpu_ClearVector(&(corpsep->v[i]));
    }
}

int FreeCorpse(Work *work)
{
    //CONTROL *ctrl=work->control;
    OBJECT *obj=work->obj;
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
    //DG_MDL *mdl=obj->objs->def->models;
    int size=HUMAN_MODEL_OBJN;
    int i;
    struct _scratchpad *scratchpad;


#ifdef DEBUG
    if(corpsep->count==0){
	corpsep->count++;
	printf("Start FreeCorpse\n");
    }
#endif

    if(corpsep->count>SWITCH2HOLD_FRAMES){

#ifdef DEBUG
	if(corpsep->count==SWITCH2HOLD_FRAMES+1){
	    corpsep->count++;
	    printf("Finish FreeCorpse & Start HoldCorpse\n");
	}
#endif

	HoldCorpse(work);
	return 0;
    }

    /* スクラッチパッドのみで納まるかあやしいので、
       管理関数を介するようにした。*/
    ScratchpadManInit();

#if 0
    scratchpad = (struct _scratchpad *)ScratchpadAlloc(sizeof(struct _scratchpad));
#else
    scratchpad = (struct _scratchpad *)ScratchpadAlloc2(sizeof(struct _scratchpad));
#endif

    scratchpad->hzx_id=work->control->hzx_id;

    StartCorpse2(scratchpad,obj,corpsep);

    MovePoint(scratchpad,obj->objs);

    CalcKQuat(scratchpad,corpsep);

    for(i=0;i<size;i++){
	fpu_CopyVector(&(obj->m_ctrl->abs_rots[i]),&(scratchpad->abs_rots[i]));
	fpu_CopyVector(&(corpsep->abs_rots[i]),&(scratchpad->abs_rots[i]));
    }

    for(i=0;i<size;i++){
	fpu_CopyVector(&(corpsep->x[i]),
		       (FVECTOR *)&(obj->objs->objs[i].world.m[3][0]));
    }

    MT_QuatMul(&(corpsep->main_abs_rots),&(scratchpad->root),
	       &(scratchpad->abs_rots[HUMAN21_KOSHI]));

    corpsep->count++;

#if 0
    ScratchpadFree(scratchpad);
#else
    ScratchpadFree2();
#endif

    return 1;
}

void JumpFreeCorpse(Work *work)
{
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
    OBJECT *obj=work->obj;
    int size=HUMAN_MODEL_OBJN;
    int i;


#ifdef DEBUG
    printf("JumpFreeCorpse\n");
#endif

    if(corpsep->count==0){
	for(i=0;i<size;i++){
	    fpu_CopyVector(&(corpsep->basex[i]),
			   (FVECTOR *)&(obj->objs->objs[i].world.m[3][0]));
	}

#if 0
	for(i=0;i<size;i++){
	    printf("%f %f %f %f\n",
		   corpsep->abs_rots[i].vx,
		   corpsep->abs_rots[i].vy,
		   corpsep->abs_rots[i].vz,
		   corpsep->abs_rots[i].vw);
	}
#endif

    }

    for(i=0;i<size;i++){
#if 0
	corpsep->x[i].vy+=(10.0f+rnd())*P_GRAVITY;
#else
	corpsep->x[i].vy+=(50.0f+rnd())*P_GRAVITY;
#endif
    }

    corpsep->count=0;
}

void JumpFreeCorpse2(Work *work,int objnum)
{
    CORPSE_PARAMETER *corpsep=&(work->corpsep);


#ifdef DEBUG
    printf("JumpFreeCorpse2\n");
#endif


    if(corpsep->count==0){
	OBJECT *obj=work->obj;
	int size=HUMAN_MODEL_OBJN;
	int i;

	for(i=0;i<size;i++){
	    fpu_CopyVector(&(corpsep->basex[i]),
			   (FVECTOR *)&(obj->objs->objs[i].world.m[3][0]));
	}

#if 0
	for(i=0;i<size;i++){
	    printf("%f %f %f %f\n",
		   corpsep->abs_rots[i].vx,
		   corpsep->abs_rots[i].vy,
		   corpsep->abs_rots[i].vz,
		   corpsep->abs_rots[i].vw);
	}
#endif

    }

    switch(objnum){
    case HUMAN21_KOSHI:
    case HUMAN21_ONAKA:
	JumpFreeCorpse(work);
	return;
    case HUMAN21_MUNE:
	objnum=HUMAN21_KUBI;
	break;
    case HUMAN21_MIGI_ASHI1:
	objnum=HUMAN21_MIGI_ASHI2;
	break;
    case HUMAN21_HIDARI_ASHI1:
	objnum=HUMAN21_HIDARI_ASHI2;
	break;
    }

#if 0
    corpsep->x[objnum].vy+=(10.0f+rnd())*P_GRAVITY;
    corpsep->x[HUMAN21_MUNE].vy+=(3.0f+rnd())*P_GRAVITY;
#else
    corpsep->x[objnum].vy+=(50.0f+rnd())*P_GRAVITY;
    corpsep->x[HUMAN21_MUNE].vy+=(30.0f+rnd())*P_GRAVITY;
#endif

    corpsep->count=0;
}


static void StartCorpse21(struct _scratchpad *scratchpad,OBJECT *obj,CORPSE_PARAMETER *corpsep)
{
    //MAP *map;
    int size=HUMAN_MODEL_OBJN;
    int i;


    for(i=0;i<size;i++){
	fpu_CopyVector(&(scratchpad->abs_rots[i]),&(corpsep->abs_rots[i]));
	fpu_CopyVector(&(corpsep->mtn_abs_rots[i]),&(scratchpad->abs_rots[i]));
    }

#if 0

#if 0
    if((map=GM_GetMap(GM_CurrentMap))==NULL){
	scratchpad->hzd=NULL;
    }
    else{
	scratchpad->hzd=map->hzd;
    }
#else
    scratchpad->hzd=HZX_GetCurrentHzd(GM_CurrentMap);
#endif

#endif

    DG_GetPos(&(scratchpad->m_rmat));
    MT_MatToQuat(&(scratchpad->m_root),&(scratchpad->m_rmat));
    MT_QuatInverse(&(scratchpad->inv_m_root),&(scratchpad->m_root));

    //Corpse_WorldCalc(scratchpad->objmat,&(scratchpad->m_rmat),obj,scratchpad->abs_rots);
    if(corpsep->count==0){
	for(i=0;i<size;i++){
	    fpu_CopyVector(&(scratchpad->x[i]),&(corpsep->x[i]));
	    fpu_ClearVector(&(corpsep->v[i]));
	}
    }
    else{
	for(i=0;i<size;i++){
	    fpu_CopyVector(&(scratchpad->x[i]),
			   (FVECTOR *)&(obj->objs->objs[i].world.m[3][0]));
	}
    }

    fpu_CopyVector(&(scratchpad->root),&(corpsep->main_abs_rots));

    MT_QuatInverse(&(scratchpad->inv_root),&(scratchpad->root));

    MT_QuatToMat(&(scratchpad->rmat),&(scratchpad->root));

#if 0
    fpu_CopyVector((FVECTOR *)&(scratchpad->rmat.m[3][0]),
		   &(scratchpad->x[HUMAN21_KOSHI]));
#else
    fpu_CopyVector((FVECTOR *)&(scratchpad->rmat.m[3][0]),
		   (FVECTOR *)&(scratchpad->m_rmat.m[3][0]));
#endif

    MT_QuatMul(&(scratchpad->conv_root),&(scratchpad->inv_m_root),&(scratchpad->root));
}


#define SLERP_PARAM	(1.0f/4.0f)

#if 0
#define SLERP_LIMIT	(0.965925826f) /* cos 15 */
#define SLERP_LIMIT2	(0.707106781f) /* cos 45 */
#elif 0
#define SLERP_LIMIT	(0.866025404f) /* cos 30 */
#define SLERP_LIMIT2	(0.5f)         /* cos 60 */
#else
#define SLERP_LIMIT	(0.866025404f) /* cos 30 */
#define SLERP_LIMIT2	(0.707106781f) /* cos 45 */
#define SLERP_AGREE	(30.0f*M_PI/180.0f)
#define SLERP_AGREE2	(45.0f*M_PI/180.0f)
#endif

#if 0

static void CheckBackBone(struct _scratchpad *scratchpad,Work *work)
{
    OBJECT *obj=work->obj;
    DG_MDL *mdl=obj->objs->def->models;
    // CORPSE_PARAMETER *corpsep=&(work->corpsep);
    FMATRIX m;
    FVECTOR vec;
    FVECTOR q;
    float p;

    /* KAKATO+ASHI2+ASHI1 */
    vec.vx=mdl[HUMAN21_MIGI_ASHI2].tx+mdl[HUMAN21_MIGI_KAKATO].tx;
    vec.vy=mdl[HUMAN21_MIGI_ASHI2].ty+mdl[HUMAN21_MIGI_KAKATO].ty;
    vec.vz=mdl[HUMAN21_MIGI_ASHI2].tz+mdl[HUMAN21_MIGI_KAKATO].tz;
    vec.vw=0.0f;

    vu0_Ldm0(&(scratchpad->rmat));
    vu0_Ldv0(&vec);

    vu0_Mulv0m0v0();
    vu0_Stv0(&vec);

    UTL_MakeQuat(&q,
		 &(scratchpad->x[HUMAN21_MIGI_ASHI1]),
		 &(scratchpad->x[HUMAN21_MIGI_KAKATO]),
		 &vec);
    MT_QuatMul(&q,&q,&(scratchpad->m_root));

    MT_QuatMul(&q,&(scratchpad->inv_m_root),&q);

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
		     &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]));
    }


    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
		     &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
    }


    vec.vx=mdl[HUMAN21_HIDARI_ASHI2].tx+mdl[HUMAN21_HIDARI_KAKATO].tx;
    vec.vy=mdl[HUMAN21_HIDARI_ASHI2].ty+mdl[HUMAN21_HIDARI_KAKATO].ty;
    vec.vz=mdl[HUMAN21_HIDARI_ASHI2].tz+mdl[HUMAN21_HIDARI_KAKATO].tz;
    vec.vw=0.0f;

    vu0_Ldm0(&(scratchpad->rmat));
    vu0_Ldv0(&vec);

    vu0_Mulv0m0v0();
    vu0_Stv0(&vec);

    UTL_MakeQuat(&q,
		 &(scratchpad->x[HUMAN21_HIDARI_ASHI1]),
		 &(scratchpad->x[HUMAN21_HIDARI_KAKATO]),
		 &vec);
    MT_QuatMul(&q,&q,&(scratchpad->m_root));

    MT_QuatMul(&q,&(scratchpad->inv_m_root),&q);

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
		     &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]));
    }


    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
		     &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
    }


    /* ASHI2+ASHI1+KOSHI */
    MT_QuatSlerp(&q,
		 &(scratchpad->abs_rots[HUMAN21_KOSHI]),
		 &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),1.0f/2.0f);
    MT_QuatNormalize(&q,&q);

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
		     &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]));
    }


    MT_QuatSlerp(&q,
		 &(scratchpad->abs_rots[HUMAN21_KOSHI]),
		 &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),1.0f/2.0f);
    MT_QuatNormalize(&q,&q);

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
		     &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]));
    }


#if 0
    /* ASHI1+KOSHI+ONAKA */
    MT_QuatSlerp(&q,
		 &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
		 &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),1.0f/2.0f);
    MT_QuatNormalize(&q,&q);

    MT_QuatSlerp(&q,&q,&(scratchpad->abs_rots[HUMAN21_ONAKA]),1.0f/2.0f);
    MT_QuatNormalize(&q,&q);

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_KOSHI]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
		     &(scratchpad->abs_rots[HUMAN21_KOSHI]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
			 &(scratchpad->abs_rots[HUMAN21_KOSHI]));
    }


    /* KOSHI+ONAKA+MUNE */
    vec.vx=mdl[HUMAN21_ONAKA].tx+mdl[HUMAN21_MUNE].tx;
    vec.vy=mdl[HUMAN21_ONAKA].ty+mdl[HUMAN21_MUNE].ty;
    vec.vz=mdl[HUMAN21_ONAKA].tz+mdl[HUMAN21_MUNE].tz;
    vec.vw=0.0f;

    vu0_Ldm0(&(scratchpad->rmat));
    vu0_Ldv0(&vec);

    vu0_Mulv0m0v0();
    vu0_Stv0(&vec);

    UTL_MakeQuat(&q,
		 &(scratchpad->x[HUMAN21_KOSHI]),
		 &(scratchpad->x[HUMAN21_MUNE]),
		 &vec);
    MT_QuatMul(&q,&q,&(scratchpad->m_root));

    MT_QuatMul(&q,&(scratchpad->inv_m_root),&q);

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_KOSHI]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
		     &(scratchpad->abs_rots[HUMAN21_KOSHI]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
			 &(scratchpad->abs_rots[HUMAN21_KOSHI]));
    }


    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_ONAKA]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
		     &(scratchpad->abs_rots[HUMAN21_ONAKA]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
			 &(scratchpad->abs_rots[HUMAN21_ONAKA]));
    }
#endif

    MT_QuatMul(&q,&(scratchpad->m_root),&(scratchpad->abs_rots[HUMAN21_KOSHI]));
    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_KOSHI]));

    vec.vx=mdl[HUMAN21_ONAKA].tx;
    vec.vy=mdl[HUMAN21_ONAKA].ty;
    vec.vz=mdl[HUMAN21_ONAKA].tz;
    vec.vw=1.0f;
    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vec.vx=mdl[HUMAN21_MIGI_ASHI1].tx;
    vec.vy=mdl[HUMAN21_MIGI_ASHI1].ty;
    vec.vz=mdl[HUMAN21_MIGI_ASHI1].tz;
    vu0_Ldv1(&vec);

    vec.vx=mdl[HUMAN21_HIDARI_ASHI1].tx;
    vec.vy=mdl[HUMAN21_HIDARI_ASHI1].ty;
    vec.vz=mdl[HUMAN21_HIDARI_ASHI1].tz;
    vu0_Ldv2(&vec);

    vu0_Mulv0m0v0();
    vu0_Mulv1m0v1();
    vu0_Mulv2m0v2();

    vu0_Stv0(&(scratchpad->x[HUMAN21_ONAKA]));
    vu0_Stv1(&(scratchpad->x[HUMAN21_MIGI_ASHI1]));
    vu0_Stv2(&(scratchpad->x[HUMAN21_HIDARI_ASHI1]));


    /* ASHI2 CALC */
    MT_QuatMul(&q,&(scratchpad->m_root),&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]));
    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_MIGI_ASHI1]));

    vec.vx=mdl[HUMAN21_MIGI_ASHI2].tx;
    vec.vy=mdl[HUMAN21_MIGI_ASHI2].ty;
    vec.vz=mdl[HUMAN21_MIGI_ASHI2].tz;
    vec.vw=1.0f;
    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vu0_Mulv0m0v0();

    vu0_Stv0(&(scratchpad->x[HUMAN21_MIGI_ASHI2]));


    MT_QuatMul(&q,&(scratchpad->m_root),&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]));
    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_HIDARI_ASHI1]));

    vec.vx=mdl[HUMAN21_HIDARI_ASHI2].tx;
    vec.vy=mdl[HUMAN21_HIDARI_ASHI2].ty;
    vec.vz=mdl[HUMAN21_HIDARI_ASHI2].tz;
    vec.vw=1.0f;
    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vu0_Mulv0m0v0();

    vu0_Stv0(&(scratchpad->x[HUMAN21_HIDARI_ASHI2]));


    /* KAKATO CALC */
    MT_QuatMul(&q,&(scratchpad->m_root),&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_MIGI_ASHI2]));

    vec.vx=mdl[HUMAN21_MIGI_KAKATO].tx;
    vec.vy=mdl[HUMAN21_MIGI_KAKATO].ty;
    vec.vz=mdl[HUMAN21_MIGI_KAKATO].tz;
    vec.vw=1.0f;
    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vu0_Mulv0m0v0();

    vu0_Stv0(&(scratchpad->x[HUMAN21_MIGI_KAKATO]));


    MT_QuatMul(&q,&(scratchpad->m_root),&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_HIDARI_ASHI2]));

    vec.vx=mdl[HUMAN21_HIDARI_KAKATO].tx;
    vec.vy=mdl[HUMAN21_HIDARI_KAKATO].ty;
    vec.vz=mdl[HUMAN21_HIDARI_KAKATO].tz;
    vec.vw=1.0f;
    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vu0_Mulv0m0v0();

    vu0_Stv0(&(scratchpad->x[HUMAN21_HIDARI_KAKATO]));



    /* ONAKA+MUNE+KUBI */
    vec.vx=mdl[HUMAN21_MUNE].tx+mdl[HUMAN21_KUBI].tx;
    vec.vy=mdl[HUMAN21_MUNE].ty+mdl[HUMAN21_KUBI].ty;
    vec.vz=mdl[HUMAN21_MUNE].tz+mdl[HUMAN21_KUBI].tz;
    vec.vw=0.0f;

    vu0_Ldm0(&(scratchpad->rmat));
    vu0_Ldv0(&vec);

    vu0_Mulv0m0v0();
    vu0_Stv0(&vec);

    UTL_MakeQuat(&q,
		 &(scratchpad->x[HUMAN21_ONAKA]),
		 &(scratchpad->x[HUMAN21_KUBI]),
		 &vec);
    MT_QuatMul(&q,&q,&(scratchpad->m_root));
    MT_QuatMul(&q,&(scratchpad->inv_m_root),&q);

#if 0
    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_ONAKA]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
		     &(scratchpad->abs_rots[HUMAN21_ONAKA]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
			 &(scratchpad->abs_rots[HUMAN21_ONAKA]));
    }
#endif

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_MUNE]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MUNE]),
		     &(scratchpad->abs_rots[HUMAN21_MUNE]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MUNE]),
			 &(scratchpad->abs_rots[HUMAN21_MUNE]));
    }


    MT_QuatMul(&q,&(scratchpad->m_root),&(scratchpad->abs_rots[HUMAN21_ONAKA]));
    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_ONAKA]));

    vec.vx=mdl[HUMAN21_MUNE].tx;
    vec.vy=mdl[HUMAN21_MUNE].ty;
    vec.vz=mdl[HUMAN21_MUNE].tz;
    vec.vw=1.0f;
    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vu0_Mulv0m0v0();

    vu0_Stv0(&(scratchpad->x[HUMAN21_MUNE]));


    /* MUNE+KUBI+ATAMA */
    vec.vx=mdl[HUMAN21_KUBI].tx+mdl[HUMAN21_ATAMA].tx;
    vec.vy=mdl[HUMAN21_KUBI].ty+mdl[HUMAN21_ATAMA].ty;
    vec.vz=mdl[HUMAN21_KUBI].tz+mdl[HUMAN21_ATAMA].tz;
    vec.vw=0.0f;

    vu0_Ldm0(&(scratchpad->rmat));
    vu0_Ldv0(&vec);

    vu0_Mulv0m0v0();
    vu0_Stv0(&vec);

    UTL_MakeQuat(&q,
		 &(scratchpad->x[HUMAN21_MUNE]),
		 &(scratchpad->x[HUMAN21_ATAMA]),
		 &vec);
    MT_QuatMul(&q,&q,&(scratchpad->m_root));

    MT_QuatMul(&q,&(scratchpad->inv_m_root),&q);

    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_MUNE]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MUNE]),
		     &(scratchpad->abs_rots[HUMAN21_MUNE]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MUNE]),
			 &(scratchpad->abs_rots[HUMAN21_MUNE]));
    }


    p=MT_QuatDot(&q,&(scratchpad->abs_rots[HUMAN21_KUBI]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_KUBI]),
		     &(scratchpad->abs_rots[HUMAN21_KUBI]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_KUBI]),
			 &(scratchpad->abs_rots[HUMAN21_KUBI]));
    }


    MT_QuatMul(&q,&(scratchpad->m_root),&(scratchpad->abs_rots[HUMAN21_MUNE]));
    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_MUNE]));

    vec.vx=mdl[HUMAN21_KUBI].tx;
    vec.vy=mdl[HUMAN21_KUBI].ty;
    vec.vz=mdl[HUMAN21_KUBI].tz;
    vec.vw=1.0f;
    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vec.vx=mdl[HUMAN21_MIGI_KATA].tx;
    vec.vy=mdl[HUMAN21_MIGI_KATA].ty;
    vec.vz=mdl[HUMAN21_MIGI_KATA].tz;
    vu0_Ldv1(&vec);

    vec.vx=mdl[HUMAN21_HIDARI_KATA].tx;
    vec.vy=mdl[HUMAN21_HIDARI_KATA].ty;
    vec.vz=mdl[HUMAN21_HIDARI_KATA].tz;
    vu0_Ldv2(&vec);

    vu0_Mulv0m0v0();
    vu0_Mulv1m0v1();
    vu0_Mulv2m0v2();

    vu0_Stv0(&(scratchpad->x[HUMAN21_KUBI]));
    vu0_Stv1(&(scratchpad->x[HUMAN21_MIGI_KATA]));
    vu0_Stv2(&(scratchpad->x[HUMAN21_HIDARI_KATA]));


    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_ATAMA]),
		   &(scratchpad->abs_rots[HUMAN21_KUBI]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MIGI_TE]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_HIDARI_TE]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MIGI_KAKATO]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MIGI_TSUMASAKI]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_HIDARI_KAKATO]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_HIDARI_TSUMASAKI]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
}

#endif


#undef SLERP_PARAM

#if 0
#define SLERP_PARAM	(1.0f/4.0f)
#define SLERP_PARAM2	(4.0f/5.0f)
#elif 0
#define SLERP_PARAM	(4.0f/5.0f)
#define SLERP_PARAM2	(1.0f/4.0f)
#elif 0
#define SLERP_PARAM	(0.0f)
#define SLERP_PARAM2	(1.0f/3.0f)
#define SLERP_PARAM3	(1.0f/8.0f)
#define SLERP_PARAM4	(1.0f/4.0f)
#else
#define SLERP_PARAM	(1.0f/4.0f)
#define SLERP_PARAM2	(1.0f/4.0f)
#define SLERP_PARAM3	(1.0f/4.0f)
#define SLERP_PARAM4	(1.0f/4.0f)
#endif


#if 1

static void CheckMotion(struct _scratchpad *scratchpad,Work *work)
{
    // OBJECT *obj=work->obj;
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
    float p;

#if 0
    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
		 &(corpsep->abs_rots[HUMAN21_KOSHI]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	printf("KOSHI\n");
	printf("KOSHI S = %f %f %f %f\n",
	       scratchpad->abs_rots[HUMAN21_KOSHI].vx,
	       scratchpad->abs_rots[HUMAN21_KOSHI].vy,
	       scratchpad->abs_rots[HUMAN21_KOSHI].vz,
	       scratchpad->abs_rots[HUMAN21_KOSHI].vw);
	printf("KOSHI M = %f %f %f %f\n",
	       corpsep->abs_rots[HUMAN21_KOSHI].vx,
	       corpsep->abs_rots[HUMAN21_KOSHI].vy,
	       corpsep->abs_rots[HUMAN21_KOSHI].vz,
	       corpsep->abs_rots[HUMAN21_KOSHI].vw);

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
		     &(scratchpad->abs_rots[HUMAN21_KOSHI]),
		     &(corpsep->abs_rots[HUMAN21_KOSHI]),SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
			 &(scratchpad->abs_rots[HUMAN21_KOSHI]));
    }
#endif

#if 0
    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
		 &(corpsep->abs_rots[HUMAN21_ONAKA]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	printf("ONAKA\n");

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
		     &(scratchpad->abs_rots[HUMAN21_ONAKA]),
		     &(corpsep->abs_rots[HUMAN21_ONAKA]),SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
			 &(scratchpad->abs_rots[HUMAN21_ONAKA]));
    }
#endif

    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_MUNE]),
		 &(corpsep->abs_rots[HUMAN21_MUNE]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	//printf("MUNE\n");

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MUNE]),
		     &(scratchpad->abs_rots[HUMAN21_MUNE]),
		     &(corpsep->abs_rots[HUMAN21_MUNE]),SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MUNE]),
			 &(scratchpad->abs_rots[HUMAN21_MUNE]));
    }

#if 0
    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_MIGI_KATA]),
		 &(corpsep->abs_rots[HUMAN21_MIGI_KATA]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	//printf("MIGI KATA\n");

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_KATA]),
		     &(scratchpad->abs_rots[HUMAN21_MIGI_KATA]),
		     &(corpsep->abs_rots[HUMAN21_MIGI_KATA]),SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_KATA]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_KATA]));
    }
#endif

    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]),
		 &(corpsep->abs_rots[HUMAN21_MIGI_UDE1]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	//printf("MIGI UDE1\n");

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]),
		     &(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]),
		     &(corpsep->abs_rots[HUMAN21_MIGI_UDE1]),SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]));
    }


    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]),
		 &(corpsep->abs_rots[HUMAN21_MIGI_UDE2]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	//printf("MIGI UDE2\n");

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]),
		     &(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]),
		     &(corpsep->abs_rots[HUMAN21_MIGI_UDE2]),SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]));
    }


#if 0
    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]),
		 &(corpsep->abs_rots[HUMAN21_HIDARI_KATA]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	printf("HIDARI KATA\n");

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]),
		     &(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]),
		     &(corpsep->abs_rots[HUMAN21_HIDARI_KATA]),SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]));
    }
#endif


    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]),
		 &(corpsep->abs_rots[HUMAN21_HIDARI_UDE1]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	//printf("HIDARI UDE1\n");

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]),
		     &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]),
		     &(corpsep->abs_rots[HUMAN21_HIDARI_UDE1]),SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]));
    }


    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]),
		 &(corpsep->abs_rots[HUMAN21_HIDARI_UDE2]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	//printf("HIDARI UDE2\n");

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]),
		     &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]),
		     &(corpsep->abs_rots[HUMAN21_HIDARI_UDE2]),SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]));
    }


    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
		 &(corpsep->abs_rots[HUMAN21_MIGI_ASHI1]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	//printf("MIGI ASHI1\n");

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
		     &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
		     &(corpsep->abs_rots[HUMAN21_MIGI_ASHI1]),SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]));
    }


    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
		 &(corpsep->abs_rots[HUMAN21_MIGI_ASHI2]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	//printf("MIGI ASHI2\n");

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
		     &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
		     &(corpsep->abs_rots[HUMAN21_MIGI_ASHI2]),SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
    }


    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
		 &(corpsep->abs_rots[HUMAN21_HIDARI_ASHI1]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	//printf("HIDARI ASHI1\n");

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
		     &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
		     &(corpsep->abs_rots[HUMAN21_HIDARI_ASHI1]),SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]));
    }


    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
		 &(corpsep->abs_rots[HUMAN21_HIDARI_ASHI2]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	//printf("HIDARI ASHI2\n");

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
		     &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
		     &(corpsep->abs_rots[HUMAN21_HIDARI_ASHI2]),SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
    }


    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_ATAMA]),
		   &(scratchpad->abs_rots[HUMAN21_KUBI]));

    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MIGI_TE]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_HIDARI_TE]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]));

    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MIGI_KAKATO]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MIGI_TSUMASAKI]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_HIDARI_KAKATO]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_HIDARI_TSUMASAKI]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
}

#elif 0

static void CheckMotion(struct _scratchpad *scratchpad,Work *work)
{
    OBJECT *obj=work->obj;
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
    float p;

#if 0
    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
		 &(corpsep->abs_rots[HUMAN21_KOSHI]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	float c;

	if(p<0.0f){
	    scratchpad->abs_rots[HUMAN21_KOSHI].vx=-scratchpad->abs_rots[HUMAN21_KOSHI].vx;
	    scratchpad->abs_rots[HUMAN21_KOSHI].vy=-scratchpad->abs_rots[HUMAN21_KOSHI].vy;
	    scratchpad->abs_rots[HUMAN21_KOSHI].vz=-scratchpad->abs_rots[HUMAN21_KOSHI].vz;
	    scratchpad->abs_rots[HUMAN21_KOSHI].vw=-scratchpad->abs_rots[HUMAN21_KOSHI].vw;
	    p=-p;
	}
	c=acosf(p);

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
		     &(corpsep->abs_rots[HUMAN21_KOSHI]),
		     &(scratchpad->abs_rots[HUMAN21_KOSHI]),
		     SLERP_AGREE/c);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
			 &(scratchpad->abs_rots[HUMAN21_KOSHI]));

	printf("KOSHI\n");
    }
#endif

#if 0
    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
		 &(corpsep->abs_rots[HUMAN21_ONAKA]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	float c;

	if(p<0.0f){
	    scratchpad->abs_rots[HUMAN21_ONAKA].vx=-scratchpad->abs_rots[HUMAN21_ONAKA].vx;
	    scratchpad->abs_rots[HUMAN21_ONAKA].vy=-scratchpad->abs_rots[HUMAN21_ONAKA].vy;
	    scratchpad->abs_rots[HUMAN21_ONAKA].vz=-scratchpad->abs_rots[HUMAN21_ONAKA].vz;
	    scratchpad->abs_rots[HUMAN21_ONAKA].vw=-scratchpad->abs_rots[HUMAN21_ONAKA].vw;
	    p=-p;
	}
	c=acosf(p);

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
		     &(corpsep->abs_rots[HUMAN21_ONAKA]),
		     &(scratchpad->abs_rots[HUMAN21_ONAKA]),
		     SLERP_AGREE/c);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
			 &(scratchpad->abs_rots[HUMAN21_ONAKA]));

	printf("ONAKA\n");
    }
#endif

    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_MUNE]),
		 &(corpsep->abs_rots[HUMAN21_MUNE]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	float c;

	if(p<0.0f){
	    scratchpad->abs_rots[HUMAN21_MUNE].vx=-scratchpad->abs_rots[HUMAN21_MUNE].vx;
	    scratchpad->abs_rots[HUMAN21_MUNE].vy=-scratchpad->abs_rots[HUMAN21_MUNE].vy;
	    scratchpad->abs_rots[HUMAN21_MUNE].vz=-scratchpad->abs_rots[HUMAN21_MUNE].vz;
	    scratchpad->abs_rots[HUMAN21_MUNE].vw=-scratchpad->abs_rots[HUMAN21_MUNE].vw;
	    p=-p;
	}
	c=acosf(p);

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MUNE]),
		     &(corpsep->abs_rots[HUMAN21_MUNE]),
		     &(scratchpad->abs_rots[HUMAN21_MUNE]),
		     SLERP_AGREE/c);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MUNE]),
			 &(scratchpad->abs_rots[HUMAN21_MUNE]));

	//printf("MUNE\n");
    }


#if 0
    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_MIGI_KATA]),
		 &(corpsep->abs_rots[HUMAN21_MIGI_KATA]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	float c;

	if(p<0.0f){
	    scratchpad->abs_rots[HUMAN21_MIGI_KATA].vx=-scratchpad->abs_rots[HUMAN21_MIGI_KATA].vx;
	    scratchpad->abs_rots[HUMAN21_MIGI_KATA].vy=-scratchpad->abs_rots[HUMAN21_MIGI_KATA].vy;
	    scratchpad->abs_rots[HUMAN21_MIGI_KATA].vz=-scratchpad->abs_rots[HUMAN21_MIGI_KATA].vz;
	    scratchpad->abs_rots[HUMAN21_MIGI_KATA].vw=-scratchpad->abs_rots[HUMAN21_MIGI_KATA].vw;
	    p=-p;
	}
	c=acosf(p);

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_KATA]),
		     &(corpsep->abs_rots[HUMAN21_MIGI_KATA]),
		     &(scratchpad->abs_rots[HUMAN21_MIGI_KATA]),
		     SLERP_AGREE/c);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_KATA]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_KATA]));

	//printf("MIGI KATA\n");
    }
#endif

    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]),
		 &(corpsep->abs_rots[HUMAN21_MIGI_UDE1]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	float c;

	if(p<0.0f){
	    scratchpad->abs_rots[HUMAN21_MIGI_UDE1].vx=-scratchpad->abs_rots[HUMAN21_MIGI_UDE1].vx;
	    scratchpad->abs_rots[HUMAN21_MIGI_UDE1].vy=-scratchpad->abs_rots[HUMAN21_MIGI_UDE1].vy;
	    scratchpad->abs_rots[HUMAN21_MIGI_UDE1].vz=-scratchpad->abs_rots[HUMAN21_MIGI_UDE1].vz;
	    scratchpad->abs_rots[HUMAN21_MIGI_UDE1].vw=-scratchpad->abs_rots[HUMAN21_MIGI_UDE1].vw;
	    p=-p;
	}
	c=acosf(p);

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]),
		     &(corpsep->abs_rots[HUMAN21_MIGI_UDE1]),
		     &(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]),
		     SLERP_AGREE2/c);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]));

	//printf("MIGI UDE1\n");
    }


    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]),
		 &(corpsep->abs_rots[HUMAN21_MIGI_UDE2]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	float c;

	if(p<0.0f){
	    scratchpad->abs_rots[HUMAN21_MIGI_UDE2].vx=-scratchpad->abs_rots[HUMAN21_MIGI_UDE2].vx;
	    scratchpad->abs_rots[HUMAN21_MIGI_UDE2].vy=-scratchpad->abs_rots[HUMAN21_MIGI_UDE2].vy;
	    scratchpad->abs_rots[HUMAN21_MIGI_UDE2].vz=-scratchpad->abs_rots[HUMAN21_MIGI_UDE2].vz;
	    scratchpad->abs_rots[HUMAN21_MIGI_UDE2].vw=-scratchpad->abs_rots[HUMAN21_MIGI_UDE2].vw;
	    p=-p;
	}
	c=acosf(p);

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]),
		     &(corpsep->abs_rots[HUMAN21_MIGI_UDE2]),
		     &(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]),
		     SLERP_AGREE2/c);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]));

	//printf("MIGI UDE2\n");
    }


#if 0
    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]),
		 &(corpsep->abs_rots[HUMAN21_HIDARI_KATA]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	float c;

	if(p<0.0f){
	    scratchpad->abs_rots[HUMAN21_HIDARI_KATA].vx=-scratchpad->abs_rots[HUMAN21_HIDARI_KATA].vx;
	    scratchpad->abs_rots[HUMAN21_HIDARI_KATA].vy=-scratchpad->abs_rots[HUMAN21_HIDARI_KATA].vy;
	    scratchpad->abs_rots[HUMAN21_HIDARI_KATA].vz=-scratchpad->abs_rots[HUMAN21_HIDARI_KATA].vz;
	    scratchpad->abs_rots[HUMAN21_HIDARI_KATA].vw=-scratchpad->abs_rots[HUMAN21_HIDARI_KATA].vw;
	    p=-p;
	}
	c=acosf(p);

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]),
		     &(corpsep->abs_rots[HUMAN21_HIDARI_KATA]),
		     &(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]),
		     SLERP_AGREE/c);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]));

	printf("HIDARI KATA\n");
    }
#endif

    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]),
		 &(corpsep->abs_rots[HUMAN21_HIDARI_UDE1]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	float c;

	if(p<0.0f){
	    scratchpad->abs_rots[HUMAN21_HIDARI_UDE1].vx=-scratchpad->abs_rots[HUMAN21_HIDARI_UDE1].vx;
	    scratchpad->abs_rots[HUMAN21_HIDARI_UDE1].vy=-scratchpad->abs_rots[HUMAN21_HIDARI_UDE1].vy;
	    scratchpad->abs_rots[HUMAN21_HIDARI_UDE1].vz=-scratchpad->abs_rots[HUMAN21_HIDARI_UDE1].vz;
	    scratchpad->abs_rots[HUMAN21_HIDARI_UDE1].vw=-scratchpad->abs_rots[HUMAN21_HIDARI_UDE1].vw;
	    p=-p;
	}
	c=acosf(p);

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]),
		     &(corpsep->abs_rots[HUMAN21_HIDARI_UDE1]),
		     &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]),
		     SLERP_AGREE2/c);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]));

	//printf("HIDARI UDE1\n");
    }


    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]),
		 &(corpsep->abs_rots[HUMAN21_HIDARI_UDE2]));
    if(fpu_Abs(p)<SLERP_LIMIT2){
	float c;

	if(p<0.0f){
	    scratchpad->abs_rots[HUMAN21_HIDARI_UDE2].vx=-scratchpad->abs_rots[HUMAN21_HIDARI_UDE2].vx;
	    scratchpad->abs_rots[HUMAN21_HIDARI_UDE2].vy=-scratchpad->abs_rots[HUMAN21_HIDARI_UDE2].vy;
	    scratchpad->abs_rots[HUMAN21_HIDARI_UDE2].vz=-scratchpad->abs_rots[HUMAN21_HIDARI_UDE2].vz;
	    scratchpad->abs_rots[HUMAN21_HIDARI_UDE2].vw=-scratchpad->abs_rots[HUMAN21_HIDARI_UDE2].vw;
	    p=-p;
	}
	c=acosf(p);

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]),
		     &(corpsep->abs_rots[HUMAN21_HIDARI_UDE2]),
		     &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]),
		     SLERP_AGREE2/c);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]));

	//printf("HIDARI UDE2\n");
    }


    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
		 &(corpsep->abs_rots[HUMAN21_MIGI_ASHI1]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	float c;

	if(p<0.0f){
	    scratchpad->abs_rots[HUMAN21_MIGI_ASHI1].vx=-scratchpad->abs_rots[HUMAN21_MIGI_ASHI1].vx;
	    scratchpad->abs_rots[HUMAN21_MIGI_ASHI1].vy=-scratchpad->abs_rots[HUMAN21_MIGI_ASHI1].vy;
	    scratchpad->abs_rots[HUMAN21_MIGI_ASHI1].vz=-scratchpad->abs_rots[HUMAN21_MIGI_ASHI1].vz;
	    scratchpad->abs_rots[HUMAN21_MIGI_ASHI1].vw=-scratchpad->abs_rots[HUMAN21_MIGI_ASHI1].vw;
	    p=-p;
	}
	c=acosf(p);

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
		     &(corpsep->abs_rots[HUMAN21_MIGI_ASHI1]),
		     &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
		     SLERP_AGREE/c);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]));

	//printf("MIGI ASHI1\n");
    }


    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
		 &(corpsep->abs_rots[HUMAN21_MIGI_ASHI2]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	float c;

	if(p<0.0f){
	    scratchpad->abs_rots[HUMAN21_MIGI_ASHI2].vx=-scratchpad->abs_rots[HUMAN21_MIGI_ASHI2].vx;
	    scratchpad->abs_rots[HUMAN21_MIGI_ASHI2].vy=-scratchpad->abs_rots[HUMAN21_MIGI_ASHI2].vy;
	    scratchpad->abs_rots[HUMAN21_MIGI_ASHI2].vz=-scratchpad->abs_rots[HUMAN21_MIGI_ASHI2].vz;
	    scratchpad->abs_rots[HUMAN21_MIGI_ASHI2].vw=-scratchpad->abs_rots[HUMAN21_MIGI_ASHI2].vw;
	    p=-p;
	}
	c=acosf(p);

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
		     &(corpsep->abs_rots[HUMAN21_MIGI_ASHI2]),
		     &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
		     SLERP_AGREE/c);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
			 &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));

	//printf("MIGI ASHI2\n");
    }


    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
		 &(corpsep->abs_rots[HUMAN21_HIDARI_ASHI1]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	float c;

	if(p<0.0f){
	    scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1].vx
		=-scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1].vx;
	    scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1].vy
		=-scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1].vy;
	    scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1].vz
		=-scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1].vz;
	    scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1].vw
		=-scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1].vw;

	    p=-p;
	}
	c=acosf(p);

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
		     &(corpsep->abs_rots[HUMAN21_HIDARI_ASHI1]),
		     &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
		     SLERP_AGREE/c);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]));

	//printf("HIDARI ASHI1\n");
    }


    p=MT_QuatDot(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
		 &(corpsep->abs_rots[HUMAN21_HIDARI_ASHI2]));
    if(fpu_Abs(p)<SLERP_LIMIT){
	float c;

	if(p<0.0f){
	    scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2].vx
		=-scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2].vx;
	    scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2].vy
		=-scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2].vy;
	    scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2].vz
		=-scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2].vz;
	    scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2].vw
		=-scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2].vw;

	    p=-p;
	}
	c=acosf(p);

	MT_QuatSlerp(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
		     &(corpsep->abs_rots[HUMAN21_HIDARI_ASHI2]),
		     &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
		     SLERP_AGREE/c);
	MT_QuatNormalize(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
			 &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));

	//printf("HIDARI ASHI2\n");
    }


    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_ATAMA]),
		   &(scratchpad->abs_rots[HUMAN21_KUBI]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MIGI_TE]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_HIDARI_TE]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MIGI_KAKATO]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MIGI_TSUMASAKI]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_HIDARI_KAKATO]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_HIDARI_TSUMASAKI]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
}

#else


#define QUATCOS_LIMIT	0.984807753f	/* cos 20/2 */
#define QUATSIN_LIMIT	0.173648178f	/* sin 20/2 */


static void CheckMotion(struct _scratchpad *scratchpad,Work *work)
{
    FVECTOR q,inv;
    OBJECT *obj=work->obj;

#if 0
    MT_QuatInverse(&inv,&(corpsep->abs_rots[HUMAN21_KOSHI]));
    MT_QuatMul(&q,&(scratchpad->abs_rots[HUMAN21_KOSHI]),&inv);

    if(q.vw<QUATCOS_LIMIT){
	float p;

	p=fpu_Rsqrt(1.0f-q.vw*q.vw,QUATSIN_LIMIT);
	q.vx*=p;
	q.vy*=p;
	q.vz*=p;
	q.vw=QUATCOS_LIMIT;

	MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
		   &q,&(corpsep->abs_rots[HUMAN21_KOSHI]));
    }
#endif

#if 0
    MT_QuatInverse(&inv,&(corpsep->abs_rots[HUMAN21_ONAKA]));
    MT_QuatMul(&q,&(scratchpad->abs_rots[HUMAN21_ONAKA]),&inv);

    if(q.vw<QUATCOS_LIMIT){
	float p;

	p=fpu_Rsqrt(1.0f-q.vw*q.vw,QUATSIN_LIMIT);
	q.vx*=p;
	q.vy*=p;
	q.vz*=p;
	q.vw=QUATCOS_LIMIT;

	MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
		   &q,&(corpsep->abs_rots[HUMAN21_ONAKA]));
    }
#endif

    MT_QuatInverse(&inv,&(corpsep->abs_rots[HUMAN21_MUNE]));
    MT_QuatMul(&q,&(scratchpad->abs_rots[HUMAN21_MUNE]),&inv);

    if(q.vw<QUATCOS_LIMIT){
	float p;

	p=fpu_Rsqrt(1.0f-q.vw*q.vw,QUATSIN_LIMIT);
	q.vx*=p;
	q.vy*=p;
	q.vz*=p;
	q.vw=QUATCOS_LIMIT;

	MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_MUNE]),
		   &q,&(corpsep->abs_rots[HUMAN21_MUNE]));
    }

#if 0
    MT_QuatInverse(&inv,&(corpsep->abs_rots[HUMAN21_MIGI_KATA]));
    MT_QuatMul(&q,&(scratchpad->abs_rots[HUMAN21_MIGI_KATA]),&inv);

    if(q.vw<QUATCOS_LIMIT){
	float p;

	p=fpu_Rsqrt(1.0f-q.vw*q.vw,QUATSIN_LIMIT);
	q.vx*=p;
	q.vy*=p;
	q.vz*=p;
	q.vw=QUATCOS_LIMIT;

	MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_MIGI_KATA]),
		   &q,&(corpsep->abs_rots[HUMAN21_MIGI_KATA]));
    }
#endif

    MT_QuatInverse(&inv,&(corpsep->abs_rots[HUMAN21_MIGI_UDE1]));
    MT_QuatMul(&q,&(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]),&inv);

    if(q.vw<QUATCOS_LIMIT){
	float p;

	p=fpu_Rsqrt(1.0f-q.vw*q.vw,QUATSIN_LIMIT);
	q.vx*=p;
	q.vy*=p;
	q.vz*=p;
	q.vw=QUATCOS_LIMIT;

	MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]),
		   &q,&(corpsep->abs_rots[HUMAN21_MIGI_UDE1]));
    }


    MT_QuatInverse(&inv,&(corpsep->abs_rots[HUMAN21_MIGI_UDE2]));
    MT_QuatMul(&q,&(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]),&inv);

    if(q.vw<QUATCOS_LIMIT){
	float p;

	p=fpu_Rsqrt(1.0f-q.vw*q.vw,QUATSIN_LIMIT);
	q.vx*=p;
	q.vy*=p;
	q.vz*=p;
	q.vw=QUATCOS_LIMIT;

	MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]),
		   &q,&(corpsep->abs_rots[HUMAN21_MIGI_UDE2]));
    }


#if 0
    MT_QuatInverse(&inv,&(corpsep->abs_rots[HUMAN21_HIDARI_KATA]));
    MT_QuatMul(&q,&(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]),&inv);

    if(q.vw<QUATCOS_LIMIT){
	float p;

	p=fpu_Rsqrt(1.0f-q.vw*q.vw,QUATSIN_LIMIT);
	q.vx*=p;
	q.vy*=p;
	q.vz*=p;
	q.vw=QUATCOS_LIMIT;

	MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]),
		   &q,&(corpsep->abs_rots[HUMAN21_HIDARI_KATA]));
    }
#endif


    MT_QuatInverse(&inv,&(corpsep->abs_rots[HUMAN21_HIDARI_UDE1]));
    MT_QuatMul(&q,&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]),&inv);

    if(q.vw<QUATCOS_LIMIT){
	float p;

	p=fpu_Rsqrt(1.0f-q.vw*q.vw,QUATSIN_LIMIT);
	q.vx*=p;
	q.vy*=p;
	q.vz*=p;
	q.vw=QUATCOS_LIMIT;

	MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]),
		   &q,&(corpsep->abs_rots[HUMAN21_HIDARI_UDE1]));
    }


    MT_QuatInverse(&inv,&(corpsep->abs_rots[HUMAN21_HIDARI_UDE2]));
    MT_QuatMul(&q,&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]),&inv);

    if(q.vw<QUATCOS_LIMIT){
	float p;

	p=fpu_Rsqrt(1.0f-q.vw*q.vw,QUATSIN_LIMIT);
	q.vx*=p;
	q.vy*=p;
	q.vz*=p;
	q.vw=QUATCOS_LIMIT;

	MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]),
		   &q,&(corpsep->abs_rots[HUMAN21_HIDARI_UDE2]));
    }


    MT_QuatInverse(&inv,&(corpsep->abs_rots[HUMAN21_MIGI_ASHI1]));
    MT_QuatMul(&q,&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),&inv);

    if(q.vw<QUATCOS_LIMIT){
	float p;

	p=fpu_Rsqrt(1.0f-q.vw*q.vw,QUATSIN_LIMIT);
	q.vx*=p;
	q.vy*=p;
	q.vz*=p;
	q.vw=QUATCOS_LIMIT;

	MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),
		   &q,&(corpsep->abs_rots[HUMAN21_MIGI_ASHI1]));
    }

    MT_QuatInverse(&inv,&(corpsep->abs_rots[HUMAN21_MIGI_ASHI2]));
    MT_QuatMul(&q,&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),&inv);

    if(q.vw<QUATCOS_LIMIT){
	float p;

	p=fpu_Rsqrt(1.0f-q.vw*q.vw,QUATSIN_LIMIT);
	q.vx*=p;
	q.vy*=p;
	q.vz*=p;
	q.vw=QUATCOS_LIMIT;

	MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),
		   &q,&(corpsep->abs_rots[HUMAN21_MIGI_ASHI2]));
    }

    MT_QuatInverse(&inv,&(corpsep->abs_rots[HUMAN21_HIDARI_ASHI1]));
    MT_QuatMul(&q,&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),&inv);

    if(q.vw<QUATCOS_LIMIT){
	float p;

	p=fpu_Rsqrt(1.0f-q.vw*q.vw,QUATSIN_LIMIT);
	q.vx*=p;
	q.vy*=p;
	q.vz*=p;
	q.vw=QUATCOS_LIMIT;

	MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),
		   &q,&(corpsep->abs_rots[HUMAN21_HIDARI_ASHI1]));
    }

    MT_QuatInverse(&inv,&(corpsep->abs_rots[HUMAN21_HIDARI_ASHI2]));
    MT_QuatMul(&q,&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),&inv);

    if(q.vw<QUATCOS_LIMIT){
	float p;

	p=fpu_Rsqrt(1.0f-q.vw*q.vw,QUATSIN_LIMIT);
	q.vx*=p;
	q.vy*=p;
	q.vz*=p;
	q.vw=QUATCOS_LIMIT;

	MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),
		   &q,&(corpsep->abs_rots[HUMAN21_HIDARI_ASHI2]));
    }

    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_ATAMA]),
		   &(scratchpad->abs_rots[HUMAN21_KUBI]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MIGI_TE]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_HIDARI_TE]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MIGI_KAKATO]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MIGI_TSUMASAKI]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_HIDARI_KAKATO]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_HIDARI_TSUMASAKI]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
}

#endif


static void RecalcAll(struct _scratchpad *scratchpad,Work *work,OBJECT *obj)
{
    /* 体の回転の再計算 */
    AdjustHip(CHEST_COLLISION,CHEST_HEIGHT,3,1,scratchpad,work);

#if 0
    AdjustJoint(HUMAN21_MUNE,CHEST_COLLISION,1,CHEST_HEIGHT,3,
		scratchpad,work);
#endif

    AdjustChest(CHEST_COLLISION,CHEST_HEIGHT,3,1,scratchpad,work);

    AdjustJoint(HUMAN21_ATAMA,HEAD_COLLISION,HEAD_HEIGHT,3,1,
		scratchpad,work);


    AdjustShoulder(HUMAN21_MIGI_KATA,1,scratchpad,work);

    AdjustJoint(HUMAN21_MIGI_UDE2,ARM_COLLISION,ARM_HEIGHT,3,1,
		scratchpad,work);

    AdjustJoint(HUMAN21_MIGI_TE,HAND_COLLISION,HAND_HEIGHT,3,1,
		scratchpad,work);


    AdjustShoulder(HUMAN21_HIDARI_KATA,1,scratchpad,work);

    AdjustJoint(HUMAN21_HIDARI_UDE2,ARM_COLLISION,ARM_HEIGHT,3,1,
		scratchpad,work);

    AdjustJoint(HUMAN21_HIDARI_TE,HAND_COLLISION,HAND_HEIGHT,3,1,
		scratchpad,work);


    AdjustJoint(HUMAN21_MIGI_ASHI2,NEE_COLLISION,NEE_HEIGHT,3,1,
		scratchpad,work);

    AdjustJoint(HUMAN21_MIGI_KAKATO,HEEL_COLLISION,HEEL_HEIGHT,3,1,
		scratchpad,work);


    AdjustJoint(HUMAN21_HIDARI_ASHI2,NEE_COLLISION,NEE_HEIGHT,3,1,
		scratchpad,work);

    AdjustJoint(HUMAN21_HIDARI_KAKATO,HEEL_COLLISION,HEEL_HEIGHT,3,1,
		scratchpad,work);
}

int AfterJumpCorpse(Work *work)
{
    //CONTROL *ctrl=work->control;
    OBJECT *obj=work->obj;
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
    //DG_MDL *mdl=obj->objs->def->models;
    int size=HUMAN_MODEL_OBJN;
    int i;
    struct _scratchpad *scratchpad;


#ifdef DEBUG
    if(corpsep->count==0){
	printf("Start AfterJumpCorpse\n");
    }
#endif

    if(corpsep->count>SWITCH2HOLD_FRAMES){

#ifdef DEBUG
	if(corpsep->count==SWITCH2HOLD_FRAMES+1){
	    corpsep->count++;
	    printf("Finish AfterJumpCorpse & Start HoldCorpse\n");
	}
#endif

	HoldCorpse(work);
	return 0;
    }

    /* スクラッチパッドのみで納まるかあやしいので、
       管理関数を介するようにした。*/
    ScratchpadManInit();

#if 0
    scratchpad = (struct _scratchpad *)ScratchpadAlloc(sizeof(struct _scratchpad));
#else
    scratchpad = (struct _scratchpad *)ScratchpadAlloc2(sizeof(struct _scratchpad));
#endif

    scratchpad->hzx_id=work->control->hzx_id;

    StartCorpse21(scratchpad,obj,corpsep);

    vu0_Ldv0(&(scratchpad->x[0]));
    vu0_Ldv1(&(corpsep->x[0]));
    vu0_Subv2v0v1();
    for(i=1;i<size;i++){
	vu0_Stv0(&(corpsep->x[i-1]));
	vu0_Ldv0(&(scratchpad->x[i]));
	vu0_Ldv1(&(corpsep->x[i]));
	vu0_Stv2(&(scratchpad->v[i-1]));
	vu0_Subv2v0v1();
    }

    vu0_Stv0(&(corpsep->x[size-1]));
    vu0_Stv2(&(scratchpad->v[size-1]));
    corpsep->count++;

#if 1
    vu0_Ldv1(&(scratchpad->v[0]));
    vu0_Ldv0(&(scratchpad->x[0]));

    vu0_Mulv1a(0.7f);

    vu0_Addv2v0v1();

    for(i=1;i<size;i++){
	vu0_Ldv1(&(scratchpad->v[i]));
	vu0_Ldv0(&(scratchpad->x[i]));
	vu0_Mulv1a(0.7f);
	vu0_Stv2(&(scratchpad->x[i-1]));
	vu0_Addv2v0v1();
    }
    vu0_Stv2(&(scratchpad->x[size-1]));
#endif

    RecalcAll(scratchpad,work,obj);

#if 0

    for(i=2;i<size;i++){
	MT_QuatMul(&(scratchpad->abs_rots[i]),&(scratchpad->conv_root),
		   &(scratchpad->abs_rots[i]));
    }

    CheckBackBone(scratchpad,work);

    MT_QuatMul(&(corpsep->main_abs_rots),&(scratchpad->m_root),
	       &(scratchpad->abs_rots[HUMAN21_KOSHI]));


    fpu_CopyVector((FVECTOR *)&(scratchpad->m_rmat.m[3][0]),
		   &(scratchpad->x[HUMAN21_KOSHI]));
    DG_SetPos(&(scratchpad->m_rmat));

    for(i=0;i<size;i++){
	fpu_CopyVector(&(obj->m_ctrl->abs_rots[i]),&(scratchpad->abs_rots[i]));
    }

#else

    for(i=2;i<size;i++){
	MT_QuatMul(&(scratchpad->abs_rots[i]),&(scratchpad->conv_root),
		   &(scratchpad->abs_rots[i]));
    }

    CheckMotion(scratchpad,work);

    MT_QuatMul(&(corpsep->main_abs_rots),&(scratchpad->m_root),
	       &(scratchpad->abs_rots[HUMAN21_KOSHI]));

    fpu_CopyVector((FVECTOR *)&(scratchpad->m_rmat.m[3][0]),
		   &(scratchpad->x[HUMAN21_KOSHI]));
    DG_SetPos(&(scratchpad->m_rmat));

    for(i=0;i<size;i++){
	fpu_CopyVector(&(obj->m_ctrl->abs_rots[i]),&(scratchpad->abs_rots[i]));
    }

#endif

    /* 最後の回のみ、abs_rotsをコピー */
    if(corpsep->count==SWITCH2HOLD_FRAMES+1){
	for(i=0;i<size;i++){
	    fpu_CopyVector(&(corpsep->abs_rots[i]),&(scratchpad->abs_rots[i]));
	}
    }

#if 0
    ScratchpadFree(scratchpad);
#else
    ScratchpadFree2();
#endif

    return 1;
}
