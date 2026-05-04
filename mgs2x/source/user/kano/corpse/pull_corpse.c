//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	pull_corpse.c
		死体の姿勢制御

	1999/10/5 K.Kano
	$Id: pull_corpse.c,v 1.1.1.3 2002/11/19 11:43:10 Yoshizawa1 Exp $
*/


#include "corpse.h"


#define KASAAGE		CVM2N(1.0f)


void Corpse_WorldCalc(FMATRIX *s,FMATRIX *root,OBJECT *obj,FVECTOR *abs_rots)
{
    DG_MDL *mdl=obj->objs->def->models;
    int size=HUMAN_MODEL_OBJN;
    FVECTOR vec;
    int i;

    MT_QuatToMat(&(s[0]),&(abs_rots[0]));

    vu0_Ldm0(root);
    vu0_Ldm1(&(s[0]));
    vu0_Mulm2m0m1();
    vu0_Stm2(&(s[0]));

    for(i=1;i<size;i++){
	MT_QuatToMat(&(s[i]),&(abs_rots[i]));

	vec.vx=mdl[i].tx;
	vec.vy=mdl[i].ty;
	vec.vz=mdl[i].tz;
	vec.vw=1.0f;

	vu0_Ldm0(&(s[mdl[i].parent]));
	vu0_Ldv0(&vec);

	vu0_Ldm1(root);
	vu0_Ldm2(&(s[i]));

	vu0_Mulv0m0v0();
	vu0_Mulm0m1m2();

	vu0_Stm0(&(s[i]));
	vu0_Stv0((FVECTOR *)&(s[i].m[3][0]));
    }
}


static void StartCorpseForLeg(struct _scratchpad *scratchpad,OBJECT *obj,CORPSE_PARAMETER *corpsep)
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

    MT_QuatMul(&(corpsep->main_abs_rots),&(scratchpad->root),
	       &(scratchpad->abs_rots[HUMAN21_KOSHI]));

    Corpse_WorldCalc(scratchpad->objmat,&(scratchpad->rmat),obj,scratchpad->abs_rots);
    for(i=0;i<size;i++){
	fpu_CopyVector(&(scratchpad->x[i]),
		       (FVECTOR *)&(obj->objs->objs[i].world.m[3][0]));
    }

    corpsep->count=0;
}


static void StartCorpseForShoulder(struct _scratchpad *scratchpad,
				   OBJECT *obj,CORPSE_PARAMETER *corpsep)
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

    DG_GetPos(&(scratchpad->m_rmat));
    MT_MatToQuat(&(scratchpad->m_root),&(scratchpad->m_rmat));

    MT_QuatMul(&(corpsep->main_abs_rots),&(scratchpad->m_root),
	       &(scratchpad->abs_rots[HUMAN21_KOSHI]));

    MT_QuatInverse(&(scratchpad->m_root),&(scratchpad->m_root));

    Corpse_WorldCalc(scratchpad->objmat,&(scratchpad->m_rmat),obj,scratchpad->abs_rots);
    for(i=0;i<size;i++){
	fpu_CopyVector(&(scratchpad->x[i]),
		       (FVECTOR *)&(obj->objs->objs[i].world.m[3][0]));
    }

    fpu_CopyVector(&(scratchpad->root),&(corpsep->main_abs_rots));
    MT_QuatInverse(&(scratchpad->inv_root),&(scratchpad->root));

    MT_QuatToMat(&(scratchpad->rmat),&(scratchpad->root));
    fpu_CopyVector((FVECTOR *)&(scratchpad->rmat.m[3][0]),
		   &(scratchpad->x[HUMAN21_KOSHI]));

    MT_QuatMul(&(scratchpad->m_root),&(scratchpad->m_root),&(scratchpad->root));

    corpsep->count=0;
}


// ---------------------------------------------------------------------
// ---------------------------------------------------------------------

#define SLERP_PARAM	(1.0f/10.0f)

static FVECTOR vg[]={
   { 0.0f, 0      , 0.0f, 0.0f, },
   { 0.0f, 0* 0.5f, 0.0f, 0.0f, },
   { 0.0f, 0*10.0f, 0.0f, 0.0f, },
   { 0.0f, 0* 0.2f, 0.0f, 0.0f, },
/*
//BP JG - original version using a define for P_GRAVITY
    { 0.0f, P_GRAVITY      , 0.0f, 0.0f, },
    { 0.0f, P_GRAVITY* 0.5f, 0.0f, 0.0f, },
    { 0.0f, P_GRAVITY*10.0f, 0.0f, 0.0f, },
    { 0.0f, P_GRAVITY* 0.2f, 0.0f, 0.0f, },
    */
};


#define COS45	(0.707106781f)


void AdjustJoint(int objnum,int param0,int param1,int mode,int flag,
		 struct _scratchpad *scratchpad,Work *work)
{
    //CONTROL *ctrl=work->control;
    OBJECT *obj=work->obj;
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
    DG_MDL *mdl=obj->objs->def->models;
    FMATRIX m;
    FVECTOR vec,vec1;
    FVECTOR q;
    int hzd;
    float h;

    /* 位置の再計算 */
    vec.vx=mdl[objnum].tx;
    vec.vy=mdl[objnum].ty;
    vec.vz=mdl[objnum].tz;
    vec.vw=1.0f;
    UTL_MakeQuat(&q,
		 &(scratchpad->x[objnum-1]),
		 &(scratchpad->x[objnum]),
		 &vec);
    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[objnum-1]));

    vg[0].y = P_GRAVITY;              // BP JG - setup runtime gravity
    vg[1].y = P_GRAVITY* 0.5f;
    vg[2].y = P_GRAVITY*10.0f;
    vg[3].y = P_GRAVITY* 0.2f;
    vu0_Ldv2(&(vg[mode]));
    vu0_Ldv1(&(corpsep->v[objnum]));

    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vu0_Addv1v2();

    vu0_Mulv0m0v0();

    /* ハザードのチェック */
    //vu0_Ldv0(&(scratchpad->x[objnum]));

    vu0_Addv0v1();

    vu0_Stv1(&(corpsep->v[objnum]));
    vu0_Stv0(&(scratchpad->x[objnum]));

#if 0
    hzd=HZX_NearHazardCheck(scratchpad->hzx_id,&(scratchpad->x[objnum]),
			    param0, /* 検出半径 */
			    HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
			    param0 /* 反発半径 */);

    if(hzd){
	FVECTOR v;

	HZX_GetReactVector(&v);

	vu0_Ldv0(&v);
	vu0_Ldv2(&(scratchpad->x[objnum]));
	vu0_Addv2v0();
	vu0_Stv2(&(scratchpad->x[objnum]));
    }

    scratchpad->x[objnum].vy+=KASAAGE;
    HZX_LevelHazardCheck(scratchpad->hzx_id,&(scratchpad->x[objnum]),
			 HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR,0);
    scratchpad->x[objnum].vy-=KASAAGE;

    h=HZX_GetFloorLevel();
    if(scratchpad->x[objnum].vy<h+param1){
	scratchpad->x[objnum].vy=h+param1;
	fpu_ClearVector(&(corpsep->v[objnum]));
	fpu_ClearVector(&(corpsep->v[objnum-1]));
    }
#else
    if(flag){
	h=corpsep->basex[objnum].vy;
	if(scratchpad->x[objnum].vy<h){
	    scratchpad->x[objnum].vy=h;
	    fpu_ClearVector(&(corpsep->v[objnum]));
	    fpu_ClearVector(&(corpsep->v[objnum-1]));
	}
    }
    else{
	hzd=HZX_NearHazardCheck(scratchpad->hzx_id,&(scratchpad->x[objnum]),
				param0, /* 検出半径 */
				HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
				param0 /* 反発半径 */);

	if(hzd){
	    FVECTOR v;

	    HZX_GetReactVector(&v);

	    vu0_Ldv0(&v);
	    vu0_Ldv2(&(scratchpad->x[objnum]));
	    vu0_Addv2v0();
	    vu0_Stv2(&(scratchpad->x[objnum]));
	}

	scratchpad->x[objnum].vy+=KASAAGE;
	HZX_LevelHazardCheck(scratchpad->hzx_id,&(scratchpad->x[objnum]),
			     HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR,0);
	scratchpad->x[objnum].vy-=KASAAGE;

	h=HZX_GetFloorLevel();
	if(scratchpad->x[objnum].vy<h+param1){
	    scratchpad->x[objnum].vy=h+param1;
	    fpu_ClearVector(&(corpsep->v[objnum]));
	    fpu_ClearVector(&(corpsep->v[objnum-1]));
	}
    }
#endif

    /* 角度の計算 */
    vu0_Ldm0(&(scratchpad->rmat));
    vu0_Ldv0(&vec);
    vu0_Setv0w0();

    vu0_Mulv0m0v0();
    vu0_Stv0(&vec1);

    UTL_MakeQuat(&q,
		 &(scratchpad->x[objnum-1]),
		 &(scratchpad->x[objnum]),
		 &vec1);

    MT_QuatMul(&q,&q,&(scratchpad->root));
    MT_QuatMul(&(scratchpad->abs_rots[objnum-1]),&(scratchpad->inv_root),&q);

    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[objnum-1]));

    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vu0_Mulv0m0v0();

    vu0_Stv0(&(scratchpad->x[objnum]));
}


void AdjustHead(int param0,int param1,int mode,int flag,
		struct _scratchpad *scratchpad,Work *work)
{
    //CONTROL *ctrl=work->control;
    OBJECT *obj=work->obj;
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
    DG_MDL *mdl=obj->objs->def->models;
    FMATRIX m;
    FVECTOR vec,vec1;
    FVECTOR q;
    int hzd;
    float h;

    /* 位置の再計算 */
    vec.vx=mdl[HUMAN21_ATAMA].tx;
    vec.vy=mdl[HUMAN21_ATAMA].ty;
    vec.vz=mdl[HUMAN21_ATAMA].tz;
    vec.vw=1.0f;
    UTL_MakeQuat(&q,
		 &(scratchpad->x[HUMAN21_KUBI]),
		 &(scratchpad->x[HUMAN21_ATAMA]),
		 &vec);
    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_KUBI]));

    vg[0].y = P_GRAVITY;              // BP JG - setup runtime gravity
    vg[1].y = P_GRAVITY* 0.5f;
    vg[2].y = P_GRAVITY*10.0f;
    vg[3].y = P_GRAVITY* 0.2f;
    vu0_Ldv2(&(vg[mode]));
    vu0_Ldv1(&(corpsep->v[HUMAN21_ATAMA]));

    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vu0_Addv1v2();

    vu0_Mulv0m0v0();

    /* ハザードのチェック */
    //vu0_Ldv0(&(scratchpad->x[HUMAN21_ATAMA]));

    vu0_Addv0v1();

    vu0_Stv1(&(corpsep->v[HUMAN21_ATAMA]));
    vu0_Stv0(&(scratchpad->x[HUMAN21_ATAMA]));

#if 0
    hzd=HZX_NearHazardCheck(scratchpad->hzx_id,&(scratchpad->x[HUMAN21_ATAMA]),
			    param0, /* 検出半径 */
			    HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
			    param0 /* 反発半径 */);

    if(hzd){
	FVECTOR v;

	HZX_GetReactVector(&v);

	vu0_Ldv0(&v);
	vu0_Ldv2(&(scratchpad->x[HUMAN21_ATAMA]));
	vu0_Addv2v0();
	vu0_Stv2(&(scratchpad->x[HUMAN21_ATAMA]));
    }

    scratchpad->x[HUMAN21_ATAMA].vy+=KASAAGE;
    HZX_LevelHazardCheck(scratchpad->hzx_id,&(scratchpad->x[HUMAN21_ATAMA]),
			 HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR,0);
    scratchpad->x[HUMAN21_ATAMA].vy-=KASAAGE;

    h=HZX_GetFloorLevel();
    if(scratchpad->x[HUMAN21_ATAMA].vy<h+param1){
	scratchpad->x[HUMAN21_ATAMA].vy=h+param1;
	fpu_ClearVector(&(corpsep->v[HUMAN21_ATAMA]));
	fpu_ClearVector(&(corpsep->v[HUMAN21_KUBI]));
    }
#else
    if(flag){
	h=corpsep->basex[HUMAN21_ATAMA].vy;
	if(scratchpad->x[HUMAN21_ATAMA].vy<h){
	    scratchpad->x[HUMAN21_ATAMA].vy=h;
	    fpu_ClearVector(&(corpsep->v[HUMAN21_ATAMA]));
	    fpu_ClearVector(&(corpsep->v[HUMAN21_KUBI]));
	}
    }
    else{
	hzd=HZX_NearHazardCheck(scratchpad->hzx_id,&(scratchpad->x[HUMAN21_ATAMA]),
				param0, /* 検出半径 */
				HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
				param0 /* 反発半径 */);

	if(hzd){
	    FVECTOR v;

	    HZX_GetReactVector(&v);

	    vu0_Ldv0(&v);
	    vu0_Ldv2(&(scratchpad->x[HUMAN21_ATAMA]));
	    vu0_Addv2v0();
	    vu0_Stv2(&(scratchpad->x[HUMAN21_ATAMA]));
	}

	scratchpad->x[HUMAN21_ATAMA].vy+=KASAAGE;
	HZX_LevelHazardCheck(scratchpad->hzx_id,&(scratchpad->x[HUMAN21_ATAMA]),
			     HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR,0);
	scratchpad->x[HUMAN21_ATAMA].vy-=KASAAGE;

	h=HZX_GetFloorLevel();
	if(scratchpad->x[HUMAN21_ATAMA].vy<h+param1){
	    scratchpad->x[HUMAN21_ATAMA].vy=h+param1;
	    fpu_ClearVector(&(corpsep->v[HUMAN21_ATAMA]));
	    fpu_ClearVector(&(corpsep->v[HUMAN21_KUBI]));
	}
    }
#endif

    /* 角度の計算 */
    MT_QuatToMat(&m,&(scratchpad->abs_rots[HUMAN21_MUNE]));
    vu0_Ldm0(&(scratchpad->rmat));
    vu0_Ldm1(&m);
    vu0_Ldv0(&vec);

    vu0_Mulm2m0m1();
    vu0_Setv0w0();

    vu0_Mulv0m2v0();
    vu0_Stv0(&vec1);

    if(!UTL_MakeQuat(&q,
		     &(scratchpad->x[HUMAN21_KUBI]),
		     &(scratchpad->x[HUMAN21_ATAMA]),
		     &vec1)){

	float vx=scratchpad->abs_rots[HUMAN21_MUNE].vx;
	float vy=scratchpad->abs_rots[HUMAN21_MUNE].vy;
	float vz=scratchpad->abs_rots[HUMAN21_MUNE].vz;
	float vw=scratchpad->abs_rots[HUMAN21_MUNE].vw;

	q.vx=1.0f-2.0f*(vy*vy+vz*vz);
	q.vy=2.0f*(vx*vy+vw*vz);
	q.vz=2.0f*(vx*vz-vw*vy);
	q.vw=0.0f;

	fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_KUBI]),&q);
	MT_QuatMul(&q,&(scratchpad->root),&q);
    }
    else{
	MT_QuatMul(&q,&q,&(scratchpad->root));
	MT_QuatMul(&q,&q,&(scratchpad->abs_rots[HUMAN21_MUNE]));
	MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_KUBI]),&(scratchpad->inv_root),&q);
    }


    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_KUBI]));

    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vu0_Mulv0m0v0();

    vu0_Stv0(&(scratchpad->x[HUMAN21_ATAMA]));
}


#define SIN_KATA_LIMIT	(0.130526192f) /* sin 15/2 */
#define COS_KATA_LIMIT	(0.991444861f) /* cos 15/2 */

void AdjustShoulder(int objnum,int flag,struct _scratchpad *scratchpad,Work *work)
{
    //CONTROL *ctrl=work->control;
    OBJECT *obj=work->obj;
    //CORPSE_PARAMETER *corpsep=&(work->corpsep);
    DG_MDL *mdl=obj->objs->def->models;
    FMATRIX m;
    FVECTOR vec;
    FVECTOR q;

    /* 位置の再計算 */
    vec.vx=mdl[objnum+1].tx+mdl[objnum+2].tx;
    vec.vy=mdl[objnum+1].ty+mdl[objnum+2].ty;
    vec.vz=mdl[objnum+1].tz+mdl[objnum+2].tz;
    vec.vw=0.0f;
    vu0_Ldm0(&(scratchpad->rmat));
    vu0_Ldv0(&vec);

    vu0_Mulv0m0v0();
    vu0_Stv0(&vec);

    UTL_MakeQuat(&q,
		 &(scratchpad->x[objnum]),
		 &(scratchpad->x[objnum+2]),
		 &vec);
    MT_QuatMul(&q,&q,&(scratchpad->root));
    MT_QuatMul(&(scratchpad->abs_rots[objnum]),&(scratchpad->inv_root),&q);

    MT_QuatInverse(&q,&(scratchpad->abs_rots[HUMAN21_MUNE]));
    MT_QuatMul(&q,&(scratchpad->abs_rots[objnum]),&q);

    fpu_ClearVector(&vec);
    vec.vz=1.0f;
    MT_QuatSeparate(&q,&q,&vec);

    fpu_CopyVector(&q,&vec);
    if(q.vw<COS_KATA_LIMIT){
	float p;

	p=fpu_Rsqrt(1.0f-q.vw*q.vw,SIN_KATA_LIMIT);
	q.vx*=p;
	q.vy*=p;
	q.vz*=p;
	q.vw=COS_KATA_LIMIT;
    }

    MT_QuatMul(&(scratchpad->abs_rots[objnum]),&q,
	       &(scratchpad->abs_rots[HUMAN21_MUNE]));
    MT_QuatMul(&q,&(scratchpad->root),&(scratchpad->abs_rots[objnum]));

    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[objnum]));

    vec.vx=mdl[objnum+1].tx;
    vec.vy=mdl[objnum+1].ty;
    vec.vz=mdl[objnum+1].tz;
    vec.vw=1.0f;
    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vu0_Mulv0m0v0();

    vu0_Stv0(&(scratchpad->x[objnum+1]));
}

void AdjustChest(int param0,int param1,int mode,int flag,
		 struct _scratchpad *scratchpad,Work *work)
{
    //CONTROL *ctrl=work->control;
    OBJECT *obj=work->obj;
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
    DG_MDL *mdl=obj->objs->def->models;
    FMATRIX m;
    FVECTOR vec,vec1;
    FVECTOR q;
    int hzd;
    float h;

    /* 位置の再計算 */
    vec.vx=mdl[HUMAN21_KUBI].tx+mdl[HUMAN21_ATAMA].tx;
    vec.vy=mdl[HUMAN21_KUBI].ty+mdl[HUMAN21_ATAMA].ty;
    vec.vz=mdl[HUMAN21_KUBI].tz+mdl[HUMAN21_ATAMA].tz;
    vec.vw=1.0f;
    UTL_MakeQuat(&q,
		 &(scratchpad->x[HUMAN21_MUNE]),
		 &(scratchpad->x[HUMAN21_ATAMA]),
		 &vec);

    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_MUNE]));

    vg[0].y = P_GRAVITY;              // BP JG - setup runtime gravity
    vg[1].y = P_GRAVITY* 0.5f;
    vg[2].y = P_GRAVITY*10.0f;
    vg[3].y = P_GRAVITY* 0.2f;
    vu0_Ldv2(&(vg[mode]));
    vu0_Ldv1(&(corpsep->v[HUMAN21_ATAMA]));

    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vu0_Addv1v2();

    vu0_Mulv0m0v0();

    /* ハザードのチェック */
    //vu0_Ldv0(&(scratchpad->x[HUMAN21_KUBI]));

    vu0_Addv0v1();

    vu0_Stv1(&(corpsep->v[HUMAN21_ATAMA]));
    vu0_Stv0(&(scratchpad->x[HUMAN21_ATAMA]));

#if 0
    hzd=HZX_NearHazardCheck(scratchpad->hzx_id,&(scratchpad->x[HUMAN21_ATAMA]),
			    param0, /* 検出半径 */
			    HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
			    param0 /* 反発半径 */);

    if(hzd){
	FVECTOR v;

	HZX_GetReactVectorF(&v);

	vu0_Ldv0(&v);
	vu0_Ldv2(&(scratchpad->x[HUMAN21_ATAMA]));
	vu0_Addv2v0();
	vu0_Stv2(&(scratchpad->x[HUMAN21_ATAMA]));
    }

    scratchpad->x[HUMAN21_ATAMA].vy+=KASAAGE;
    HZX_LevelHazardCheck(scratchpad->hzx_id,&(scratchpad->x[HUMAN21_ATAMA]),
			 HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR,0);
    scratchpad->x[HUMAN21_ATAMA].vy-=KASAAGE;

    h=HZX_GetFloorLevel();
    if(scratchpad->x[HUMAN21_ATAMA].vy<h+param1){
	scratchpad->x[HUMAN21_ATAMA].vy=h+param1;
	fpu_ClearVector(&(corpsep->v[HUMAN21_ATAMA]));
	fpu_ClearVector(&(corpsep->v[HUMAN21_MUNE]));
    }
#else
    if(flag){
	h=corpsep->basex[HUMAN21_ATAMA].vy;
	if(scratchpad->x[HUMAN21_ATAMA].vy<h){
	    scratchpad->x[HUMAN21_ATAMA].vy=h;
	    fpu_ClearVector(&(corpsep->v[HUMAN21_ATAMA]));
	    fpu_ClearVector(&(corpsep->v[HUMAN21_MUNE]));
	}
    }
    else{
	hzd=HZX_NearHazardCheck(scratchpad->hzx_id,&(scratchpad->x[HUMAN21_ATAMA]),
				param0, /* 検出半径 */
				HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
				param0 /* 反発半径 */);

	if(hzd){
	    FVECTOR v;

	    HZX_GetReactVector(&v);

	    vu0_Ldv0(&v);
	    vu0_Ldv2(&(scratchpad->x[HUMAN21_ATAMA]));
	    vu0_Addv2v0();
	    vu0_Stv2(&(scratchpad->x[HUMAN21_ATAMA]));
	}

	scratchpad->x[HUMAN21_ATAMA].vy+=KASAAGE;
	HZX_LevelHazardCheck(scratchpad->hzx_id,&(scratchpad->x[HUMAN21_ATAMA]),
			     HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR,0);
	scratchpad->x[HUMAN21_ATAMA].vy-=KASAAGE;

	h=HZX_GetFloorLevel();
	if(scratchpad->x[HUMAN21_ATAMA].vy<h+param1){
	    scratchpad->x[HUMAN21_ATAMA].vy=h+param1;
	    fpu_ClearVector(&(corpsep->v[HUMAN21_ATAMA]));
	    fpu_ClearVector(&(corpsep->v[HUMAN21_MUNE]));
	}
    }
#endif

    /* 角度の計算 */
#if 0
    vu0_Ldm0(&(scratchpad->rmat));
    vu0_Ldv0(&vec);
    vu0_Setv0w0();

    vu0_Ldv1(&(scratchpad->x[HUMAN21_ATAMA]));
    vu0_Ldv2(&(scratchpad->x[HUMAN21_MUNE]));

    vu0_Mulv0m0v0();
    vu0_Subv1v2();
    vu0_Stv0(&vec1);

    if(vu0_InnerProductv0v1()<0.0f){
	static const FVECTOR ryv={ 1.0f,0.0f,0.0f,0.0f, };
	static const FMATRIX rym={
	    {
		{ 1.0f, 0.0f, 0.0f, 0.0f, },
		{ 0.0f,-1.0f, 0.0f, 0.0f, },
		{ 0.0f, 0.0f,-1.0f, 0.0f, },
		{ 0.0f, 0.0f, 0.0f, 1.0f, },
	    },
	};
	vu0_Ldm1(&rym);
	vu0_Ldv0(&vec);
	vu0_Mulm2m0m1();
	vu0_Setv0w0();

	vu0_Mulv0m2v0();

	vu0_Stv0(&vec1);

	if(!UTL_MakeQuat(&q,
			 &(scratchpad->x[HUMAN21_MUNE]),
			 &(scratchpad->x[HUMAN21_ATAMA]),
			 &vec1)){

	    float vx=scratchpad->abs_rots[HUMAN21_ONAKA].vx;
	    float vy=scratchpad->abs_rots[HUMAN21_ONAKA].vy;
	    float vz=scratchpad->abs_rots[HUMAN21_ONAKA].vz;
	    float vw=scratchpad->abs_rots[HUMAN21_ONAKA].vw;

#if 0
	    q.vz=2.0f*(vx*vz+vw*vy);
	    q.vy=2.0f*(vy*vz-vw*vx);
	    q.vz=1.0f-2.0f*(vx*vx+vy*vy);
	    q.vw=0.0f;
#else
	    q.vx=1.0f-2.0f*(vy*vy+vz*vz);
	    q.vy=2.0f*(vx*vy+vw*vz);
	    q.vz=2.0f*(vx*vz-vw*vy);
	    q.vw=0.0f;
#endif

	    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MUNE]),&q);
	    MT_QuatMul(&q,&(scratchpad->root),&q);
	}
	else{
	    MT_QuatMul(&q,&q,&(scratchpad->root));
	    MT_QuatMul(&q,&q,&ryv);
	    MT_QuatMul(&(oscratchpad->abs_rots[HUMAN21_MUNE]),&(scratchpad->inv_root),&q);
	}
    }
    else{
	if(!UTL_MakeQuat(&q,
			 &(scratchpad->x[HUMAN21_MUNE]),
			 &(scratchpad->x[HUMAN21_ATAMA]),
			 &vec1)){

	    float vx=scratchpad->abs_rots[HUMAN21_ONAKA].vx;
	    float vy=scratchpad->abs_rots[HUMAN21_ONAKA].vy;
	    float vz=scratchpad->abs_rots[HUMAN21_ONAKA].vz;
	    float vw=scratchpad->abs_rots[HUMAN21_ONAKA].vw;

#if 0
	    q.vz=2.0f*(vx*vz+vw*vy);
	    q.vy=2.0f*(vy*vz-vw*vx);
	    q.vz=1.0f-2.0f*(vx*vx+vy*vy);
	    q.vw=0.0f;
#else
	    q.vx=1.0f-2.0f*(vy*vy+vz*vz);
	    q.vy=2.0f*(vx*vy+vw*vz);
	    q.vz=2.0f*(vx*vz-vw*vy);
	    q.vw=0.0f;
#endif

	    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MUNE]),&q);
	    MT_QuatMul(&q,&(scratchpad->root),&q);
	}
	else{
	    MT_QuatMul(&q,&q,&(scratchpad->root));
	    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_MUNE]),&(scratchpad->inv_root),&q);
	}
    }
#else
    MT_QuatToMat(&m,&(scratchpad->abs_rots[HUMAN21_ONAKA]));
    vu0_Ldm0(&(scratchpad->rmat));
    vu0_Ldm1(&m);
    vu0_Ldv0(&vec);

    vu0_Mulm2m0m1();
    vu0_Setv0w0();

    vu0_Mulv0m2v0();
    vu0_Stv0(&vec1);

    if(!UTL_MakeQuat(&q,
		     &(scratchpad->x[HUMAN21_MUNE]),
		     &(scratchpad->x[HUMAN21_ATAMA]),
		     &vec1)){

	float vx=scratchpad->abs_rots[HUMAN21_ONAKA].vx;
	float vy=scratchpad->abs_rots[HUMAN21_ONAKA].vy;
	float vz=scratchpad->abs_rots[HUMAN21_ONAKA].vz;
	float vw=scratchpad->abs_rots[HUMAN21_ONAKA].vw;

#if 0
	q.vz=2.0f*(vx*vz+vw*vy);
	q.vy=2.0f*(vy*vz-vw*vx);
	q.vz=1.0f-2.0f*(vx*vx+vy*vy);
	q.vw=0.0f;
#else
	q.vx=1.0f-2.0f*(vy*vy+vz*vz);
	q.vy=2.0f*(vx*vy+vw*vz);
	q.vz=2.0f*(vx*vz-vw*vy);
	q.vw=0.0f;
#endif

	fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_MUNE]),&q);
	MT_QuatMul(&q,&(scratchpad->root),&q);
    }
    else{
	MT_QuatMul(&q,&q,&(scratchpad->root));
	MT_QuatMul(&q,&q,&(scratchpad->abs_rots[HUMAN21_ONAKA]));
	MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_MUNE]),&(scratchpad->inv_root),&q);
    }
#endif

    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_MUNE]));

    vec.vx=mdl[HUMAN21_KUBI].tx;
    vec.vy=mdl[HUMAN21_KUBI].ty;
    vec.vz=mdl[HUMAN21_KUBI].tz;
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
}

#if 0

void AdjustHip(int param0,int param1,int mode,int flag,
	       struct _scratchpad *scratchpad,Work *work)
{
    CONTROL *ctrl=work->control;
    OBJECT *obj=work->obj;
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
    DG_MDL *mdl=obj->objs->def->models;
    FMATRIX m;
    FVECTOR vec,vec1;
    FVECTOR q;
    int hzd;
    float h;

    /* ハザードのチェック */
#if 0


#if 0
    vg[0].y = P_GRAVITY;              // BP JG - setup runtime gravity
    vg[1].y = P_GRAVITY* 0.5f;
    vg[2].y = P_GRAVITY*10.0f;
    vg[3].y = P_GRAVITY* 0.2f;
    vu0_Ldv2(&(vg[mode]));
    vu0_Ldv1(&(corpsep->v[HUMAN21_KOSHI]));
    vu0_Ldv0(&(scratchpad->x[HUMAN21_KOSHI]));

    vu0_Addv1v2();
    vu0_Addv0v1();

    vu0_Stv1(&(corpsep->v[HUMAN21_KOSHI]));
    vu0_Stv0(&(scratchpad->x[HUMAN21_KOSHI]));
#else
    fpu_CopyVector(&(scratchpad->x[HUMAN21_KOSHI]),
		   (FVECTOR *)&(scratchpad->rmat.m[3][0]));
#endif

    hzd=HZX_NearHazardCheck(scratchpad->hzx_id,&(scratchpad->x[HUMAN21_KOSHI]),
			    param0, /* 検出半径 */
			    HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
			    param0 /* 反発半径 */);

    if(hzd){
	FVECTOR v;

	HZX_GetReactVector(&v);

	vu0_Ldv0(&v);
	vu0_Ldv2(&(scratchpad->x[HUMAN21_KOSHI]));
	vu0_Addv2v0();
	vu0_Stv2(&(scratchpad->x[HUMAN21_KOSHI]));
    }


#if 0
    scratchpad->x[HUMAN21_KOSHI].vy+=KASAAGE;
    HZX_LevelHazardCheck(scratchpad->hzx_id,&(scratchpad->x[HUMAN21_KOSHI]),
			 HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR,0);
    scratchpad->x[HUMAN21_KOSHI].vy-=KASAAGE;

    h=HZX_GetFloorLevel();
    if(scratchpad->x[HUMAN21_KOSHI].vy<h+param1){
	scratchpad->x[HUMAN21_KOSHI].vy=h+param1;
	fpu_ClearVector(&(corpsep->v[HUMAN21_KOSHI]));
    }
#endif


    fpu_CopyVector((FVECTOR *)&(scratchpad->rmat.m[3][0]),&(scratchpad->x[HUMAN21_KOSHI]));
    fpu_CopyVector(&(ctrl->mov),&(scratchpad->x[HUMAN21_KOSHI]));
    //DG_SetPos(&(scratchpad->rmat));
#elif 0
    fpu_CopyVector(&(scratchpad->x[HUMAN21_KOSHI]),
		   (FVECTOR *)&(obj->objs->objs[HUMAN21_KOSHI].world.m[3][0]));
#else
    fpu_CopyVector(&(scratchpad->x[HUMAN21_KOSHI]),
		   (FVECTOR *)&(scratchpad->m_rmat.m[3][0]));
#endif


    vg[0].y = P_GRAVITY;              // BP JG - setup runtime gravity
    vg[1].y = P_GRAVITY* 0.5f;
    vg[2].y = P_GRAVITY*10.0f;
    vg[3].y = P_GRAVITY* 0.2f;
    vu0_Ldv2(&(vg[mode]));
    vu0_Ldv1(&(corpsep->v[HUMAN21_ONAKA]));
    vu0_Ldv0(&(scratchpad->x[HUMAN21_ONAKA]));

    vu0_Addv1v2();
    vu0_Addv0v1();

    vu0_Stv1(&(corpsep->v[HUMAN21_ONAKA]));
    vu0_Stv0(&(scratchpad->x[HUMAN21_ONAKA]));

#if 0
    hzd=HZX_NearHazardCheck(scratchpad->hzx_id,&(scratchpad->x[HUMAN21_ONAKA]),
			    param0, /* 検出半径 */
			    HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
			    param0 /* 反発半径 */);

    if(hzd){
	FVECTOR v;

	HZX_GetReactVectorF(&v);

	vu0_Ldv0(&v);
	vu0_Ldv2(&(scratchpad->x[HUMAN21_ONAKA]));
	vu0_Addv2v0();
	vu0_Stv2(&(scratchpad->x[HUMAN21_ONAKA]));
    }

    scratchpad->x[HUMAN21_ONAKA].vy+=KASAAGE;
    HZX_LevelHazardCheck(scratchpad->hzx_id,&(scratchpad->x[HUMAN21_ONAKA]),
			 HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR,0);
    scratchpad->x[HUMAN21_ONAKA].vy-=KASAAGE;

    h=HZX_GetFloorLevel();
    if(scratchpad->x[HUMAN21_ONAKA].vy<h+param1){
	scratchpad->x[HUMAN21_ONAKA].vy=h+param1;
	fpu_ClearVector(&(corpsep->v[HUMAN21_ONAKA]));
    }
#else
    if(flag){
	h=corpsep->basex[HUMAN21_ONAKA].vy;
	if(scratchpad->x[HUMAN21_ONAKA].vy<h){
	    scratchpad->x[HUMAN21_ONAKA].vy=h;
	    fpu_ClearVector(&(corpsep->v[HUMAN21_ONAKA]));
	}
    }
    else{
	hzd=HZX_NearHazardCheck(scratchpad->hzx_id,&(scratchpad->x[HUMAN21_ONAKA]),
				param0, /* 検出半径 */
				HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
				 param0 /* 反発半径 */);

	if(hzd){
	    FVECTOR v;

	    HZX_GetReactVector(&v);

	    vu0_Ldv0(&v);
	    vu0_Ldv2(&(scratchpad->x[HUMAN21_ONAKA]));
	    vu0_Addv2v0();
	    vu0_Stv2(&(scratchpad->x[HUMAN21_ONAKA]));
	}

	scratchpad->x[HUMAN21_ONAKA].vy+=KASAAGE;
	HZX_LevelHazardCheck(scratchpad->hzx_id,&(scratchpad->x[HUMAN21_ONAKA]),
			     HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR,0);
	scratchpad->x[HUMAN21_ONAKA].vy-=KASAAGE;

	h=HZX_GetFloorLevel();
	if(scratchpad->x[HUMAN21_ONAKA].vy<h+param1){
	    scratchpad->x[HUMAN21_ONAKA].vy=h+param1;
	    fpu_ClearVector(&(corpsep->v[HUMAN21_ONAKA]));
	}
    }
#endif

    /* 角度の計算 */
    vu0_Ldm0(&(scratchpad->rmat));

    vec.vx=mdl[HUMAN21_ONAKA].tx;
    vec.vy=mdl[HUMAN21_ONAKA].ty;
    vec.vz=mdl[HUMAN21_ONAKA].tz;
    vec.vw=1.0f;
    vu0_Ldv0(&vec);

    vu0_Setv0w0();

    vu0_Mulv0m0v0();
    vu0_Stv0(&vec1);

    UTL_MakeQuat(&q,
		 &(scratchpad->x[HUMAN21_KOSHI]),
		 &(scratchpad->x[HUMAN21_ONAKA]),
		 &vec1);
    MT_QuatMul(&q,&q,&(scratchpad->root));
    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_KOSHI]),&(scratchpad->inv_root),&q);

    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_KOSHI]));

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
}

#else

void AdjustHip(int param0,int param1,int mode,int flag,
	       struct _scratchpad *scratchpad,Work *work)
{
    // CONTROL *ctrl=work->control;
    OBJECT *obj=work->obj;
    // CORPSE_PARAMETER *corpsep=&(work->corpsep);
    DG_MDL *mdl=obj->objs->def->models;
    FMATRIX m;
    FVECTOR vec;
	// FVECTOR vec1;
    FVECTOR q;

    fpu_CopyVector(&(scratchpad->x[HUMAN21_KOSHI]),
		   (FVECTOR *)&(scratchpad->m_rmat.m[3][0]));

    /* 角度の計算 */
    vec.vx=mdl[HUMAN21_ONAKA].tx;
    vec.vy=mdl[HUMAN21_ONAKA].ty;
    vec.vz=mdl[HUMAN21_ONAKA].tz;
    vec.vw=1.0f;

#if 0
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_KOSHI]),
		   &(obj->m_ctrl->abs_rots[HUMAN21_KOSHI]));
#endif

    MT_QuatMul(&q,&(scratchpad->m_root),&(scratchpad->abs_rots[HUMAN21_KOSHI]));

    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_KOSHI]));

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


    /* 角度の計算 */
    vec.vx=mdl[HUMAN21_MUNE].tx;
    vec.vy=mdl[HUMAN21_MUNE].ty;
    vec.vz=mdl[HUMAN21_MUNE].tz;
    vec.vw=1.0f;

#if 0
    fpu_CopyVector(&(scratchpad->abs_rots[HUMAN21_ONAKA]),
		   &(obj->m_ctrl->abs_rots[HUMAN21_ONAKA]));
#endif

    MT_QuatMul(&q,&(scratchpad->m_root),&(scratchpad->abs_rots[HUMAN21_ONAKA]));

    MT_QuatToMat(&m,&q);
    fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[HUMAN21_ONAKA]));

    vu0_Ldv0(&vec);
    vu0_Ldm0(&m);

    vu0_Mulv0m0v0();

    vu0_Stv0(&(scratchpad->x[HUMAN21_MUNE]));
}

#endif

static void CheckLegDeg(int objnum,int param0,int param1,
			struct _scratchpad *scratchpad,Work *work)
{
    //CONTROL *ctrl=work->control;
    OBJECT *obj=work->obj;
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
    DG_MDL *mdl=obj->objs->def->models;
    FVECTOR q,r,n,n1;

    MT_QuatInverse(&q,&(scratchpad->abs_rots[objnum]));
    MT_QuatMul(&q,&(scratchpad->abs_rots[objnum+1]),&q);
    fpu_ClearVector(&n);
    n.vx=1.0f;
    MT_QuatSeparate(&q,&r,&n);
    vu0_Ldv0(&n);
    fpu_ClearVector(&n1);
    n1.vx=1.0f;
    vu0_Ldv1(&n1);

    if(vu0_InnerProductv0v1()<0.0f){
	FMATRIX m;
	FVECTOR vec;

	fpu_CopyVector(&(scratchpad->x[objnum+2]),
		       (FVECTOR *)&(obj->objs->objs[objnum+2].world.m[3][0]));

	/* 位置の再計算 */
	vec.vx=mdl[objnum+1].tx+mdl[objnum+2].tx;
	vec.vy=mdl[objnum+1].ty+mdl[objnum+2].ty;
	vec.vz=mdl[objnum+1].tz+mdl[objnum+2].tz;
	vec.vw=1.0f;
	UTL_MakeQuat(&q,
		     &(scratchpad->x[objnum]),
		     &(scratchpad->x[objnum+2]),
		     &vec);
	MT_QuatToMat(&m,&q);
	fpu_CopyVector((FVECTOR *)&(m.m[3][0]),&(scratchpad->x[objnum]));

	vu0_Ldv0(&vec);
	vu0_Ldm0(&m);

	vu0_Ldv1(&(corpsep->v[objnum+2]));

	vu0_Mulv0m0v0();

	/* ハザードのチェック */
	{
	    int hzd;
	    float h;

	    //vu0_Ldv0(&(scratchpad->x[objnum+2]));
	    vu0_Addv0v1();

	    vu0_Stv0(&(scratchpad->x[objnum+2]));

	    hzd=HZX_NearHazardCheck(scratchpad->hzx_id,&(scratchpad->x[objnum+2]),
				    param0, /* 検出半径 */
				    HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
				    param0 /* 反発半径 */);

	    if(hzd){
		FVECTOR v;

		HZX_GetReactVector(&v);

		vu0_Ldv0(&v);
		vu0_Ldv2(&(scratchpad->x[objnum+2]));
		vu0_Addv2v0();
		vu0_Stv2(&(scratchpad->x[objnum+2]));
	    }

	    scratchpad->x[objnum+2].vy+=KASAAGE;
	    HZX_LevelHazardCheck(scratchpad->hzx_id,&(scratchpad->x[objnum+2]),
				 HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR,0);
	    scratchpad->x[objnum+2].vy-=KASAAGE;

	    h=HZX_GetFloorLevel();
	    if(scratchpad->x[objnum+2].vy<h+param1){
		scratchpad->x[objnum+2].vy=h+param1;
		fpu_ClearVector(&(corpsep->v[objnum+1]));
		fpu_ClearVector(&(corpsep->v[objnum+2]));
	    }
	}

	/* 角度の計算 */
	vu0_Ldm0(&(scratchpad->rmat));
	vu0_Ldv0(&vec);
	vu0_Setv0w0();

	vu0_Mulv0m0v0();
	vu0_Stv0(&vec);

	UTL_MakeQuat(&q,
		     &(scratchpad->x[objnum]),
		     &(scratchpad->x[objnum+2]),
		     &vec);
	MT_QuatMul(&q,&q,&(scratchpad->root));

	MT_QuatMul(&q,&(scratchpad->inv_root),&q);
	MT_QuatMul(&q,&(scratchpad->m_root),&q);

	MT_QuatSlerp(&(scratchpad->abs_rots[objnum]),
		     &(scratchpad->abs_rots[objnum]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[objnum]),
			 &(scratchpad->abs_rots[objnum]));

	MT_QuatSlerp(&(scratchpad->abs_rots[objnum+1]),
		     &(scratchpad->abs_rots[objnum+1]),&q,SLERP_PARAM);
	MT_QuatNormalize(&(scratchpad->abs_rots[objnum+1]),
			 &(scratchpad->abs_rots[objnum+1]));
    }
#if 0
    else if(r.vw<0.996194698f /* COS 5 */ ){
	MT_QuatInverse(&n,&n);
	MT_QuatMul(&(scratchpad->abs_rots[objnum]),&n,
		   &(scratchpad->abs_rots[objnum+1]));
    }
#endif
}

static void SwingedArm(int objnum,struct _scratchpad *scratchpad,Work *work)
{
    OBJECT *obj=work->obj;
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
    DG_MDL *mdl=obj->objs->def->models;
    FVECTOR q;
    FVECTOR vec;

    vu0_Ldv1(&(corpsep->x[objnum+1]));
    vu0_Ldv0(&(scratchpad->x[objnum+1]));
    vu0_Subv1v0v1();

    vg[0].y = P_GRAVITY;              // BP JG - setup runtime gravity
    vg[1].y = P_GRAVITY* 0.5f;
    vg[2].y = P_GRAVITY*10.0f;
    vg[3].y = P_GRAVITY* 0.2f;
    vu0_Ldv2(&(vg[2]));
    vu0_Mulv0v1a(-0.3f); /* 速度に対する抵抗値。特に数値の大きさに意味は無い。*/
    vu0_Addv2v0();
    vu0_Ldv0(&(scratchpad->x[objnum+1]));

    vu0_Addv1v2();
    vu0_Addv0v1();

    vu0_Stv0(&(scratchpad->x[objnum+1]));

    vec.vx=mdl[objnum+1].tx;
    vec.vy=mdl[objnum+1].ty;
    vec.vz=mdl[objnum+1].tz;
    vec.vw=1.0f;
    vu0_Ldm0(&(scratchpad->rmat));
    vu0_Ldv0(&vec);
    vu0_Setv0w0();

    vu0_Mulv0m0v0();
    vu0_Stv0(&vec);

    UTL_MakeQuat(&q,
		 &(scratchpad->x[objnum]),
		 &(scratchpad->x[objnum+1]),
		 &vec);
    MT_QuatMul(&q,&q,&(scratchpad->root));
    MT_QuatMul(&(scratchpad->abs_rots[objnum]),&(scratchpad->inv_root),&q);
    MT_QuatMul(&(scratchpad->abs_rots[objnum]),&(scratchpad->m_root),
	       &(scratchpad->abs_rots[objnum]));

#if 0  /* 後でまとめて行なう */
    fpu_CopyVector(&(scratchpad->abs_rots[objnum+1]),
		   &(scratchpad->abs_rots[objnum]));
#endif
}


// ---------------------------------------------------------------------
// ---------------------------------------------------------------------

void PullCorpseWithShoulder(Work *work)
{
    //CONTROL *ctrl=work->control;
    OBJECT *obj=work->obj;
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
    //DG_MDL *mdl=obj->objs->def->models;
    int size=HUMAN_MODEL_OBJN;
    int i;
    struct _scratchpad *scratchpad;

    /* スクラッチパッドのみで納まるかあやしいので、
       管理関数を介するようにした。*/
    ScratchpadManInit();

#if 0
    scratchpad = (struct _scratchpad *)ScratchpadAlloc(sizeof(struct _scratchpad));
#else
    scratchpad = (struct _scratchpad *)ScratchpadAlloc2(sizeof(struct _scratchpad));
#endif

    scratchpad->hzx_id=work->control->hzx_id;

    StartCorpseForShoulder(scratchpad,obj,corpsep);

    fpu_CopyVector(&(scratchpad->x[HUMAN21_MIGI_ASHI1]),
		   (FVECTOR *)&(scratchpad->objmat[HUMAN21_MIGI_ASHI1].m[3][0]));
    fpu_CopyVector(&(scratchpad->x[HUMAN21_HIDARI_ASHI1]),
		   (FVECTOR *)&(scratchpad->objmat[HUMAN21_HIDARI_ASHI1].m[3][0]));

    /* 右足の補正 */

    /* x[14]の再計算 */
    AdjustJoint(HUMAN21_MIGI_ASHI2,NEE_COLLISION,NEE_HEIGHT,1,0,
		scratchpad,work);
    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]),&(scratchpad->m_root),
	       &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]));

    /* x[15]の再計算 */
    AdjustJoint(HUMAN21_MIGI_KAKATO,HEEL_COLLISION,HEEL_HEIGHT,0,0,
		scratchpad,work);
    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]),&(scratchpad->m_root),
	       &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));

    /* 右足の角度が不正なら修正 */
    CheckLegDeg(HUMAN21_MIGI_ASHI1,HEEL_COLLISION,HEEL_HEIGHT,
		scratchpad,work);


    /* 左足の補正 */

    /* x[17]の再計算 */
    AdjustJoint(HUMAN21_HIDARI_ASHI2,NEE_COLLISION,NEE_HEIGHT,1,0,
		scratchpad,work);
    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]),&(scratchpad->m_root),
	       &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]));

    /* x[18]の再計算 */
    AdjustJoint(HUMAN21_HIDARI_KAKATO,HEEL_COLLISION,HEEL_HEIGHT,0,0,
		scratchpad,work);
    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]),&(scratchpad->m_root),
	       &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));

    /* 左足の角度が不正なら修正 */
    CheckLegDeg(HUMAN21_HIDARI_ASHI1,HEEL_COLLISION,HEEL_HEIGHT,
		scratchpad,work);


    /* ふらふらする両手 */
    fpu_CopyVector(&(scratchpad->x[HUMAN21_MIGI_UDE2]),
		   (FVECTOR *)&(scratchpad->objmat[HUMAN21_MIGI_UDE2].m[3][0]));
    fpu_CopyVector(&(scratchpad->x[HUMAN21_HIDARI_UDE2]),
		   (FVECTOR *)&(scratchpad->objmat[HUMAN21_HIDARI_UDE2].m[3][0]));
    SwingedArm(HUMAN21_MIGI_UDE2,scratchpad,work);
    SwingedArm(HUMAN21_HIDARI_UDE2,scratchpad,work);


#if 0
    /* 爪先は、それぞれの先に合わせる */
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_MIGI_KAKATO]),
		   &(obj->m_ctrl->abs_rots[HUMAN21_MIGI_ASHI2]));
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_MIGI_TSUMASAKI]),
		   &(obj->m_ctrl->abs_rots[HUMAN21_MIGI_ASHI2]));
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_KAKATO]),
		   &(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_ASHI2]));
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_TSUMASAKI]),
		   &(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_ASHI2]));
#endif

    for(i=0;i<size;i++){
	fpu_CopyVector(&(corpsep->x[i]),&(scratchpad->x[i]));
    }


    /* 戻し */
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_MIGI_UDE2]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]));
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_MIGI_TE]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]));

    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_UDE2]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]));
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_TE]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]));

    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_MIGI_ASHI1]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI1]));
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_MIGI_ASHI2]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_MIGI_KAKATO]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_MIGI_TSUMASAKI]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_ASHI2]));

    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_ASHI1]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI1]));
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_ASHI2]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_KAKATO]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_TSUMASAKI]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_ASHI2]));

#if 0
    ScratchpadFree(scratchpad);
#else
    ScratchpadFree2();
#endif
}

void PullCorpseWithLegs(Work *work)
{
    //CONTROL *ctrl=work->control;
    OBJECT *obj=work->obj;
    CORPSE_PARAMETER *corpsep=&(work->corpsep);
    DG_MDL *mdl=obj->objs->def->models;
    int size=HUMAN_MODEL_OBJN;
    int i;
    struct _scratchpad *scratchpad;

    /* スクラッチパッドのみで納まるかあやしいので、
       管理関数を介するようにした。*/
    ScratchpadManInit();

#if 0
    scratchpad = (struct _scratchpad *)ScratchpadAlloc(sizeof(struct _scratchpad));
#else
    scratchpad = (struct _scratchpad *)ScratchpadAlloc2(sizeof(struct _scratchpad));
#endif

    scratchpad->hzx_id=work->control->hzx_id;

    StartCorpseForLeg(scratchpad,obj,corpsep);

    for(i=0;i<size;i++){
	fpu_CopyVector(&(corpsep->x[i]),(FVECTOR *)&(obj->objs->objs[i].world.m[3][0]));
    }


#if 0
    fpu_CopyVector(&(scratchpad->x[HUMAN21_ONAKA]),
		   (FVECTOR *)&(scratchpad->objmat[HUMAN21_ONAKA].m[3][0]));

    /* 腹の補正 */
    AdjustJoint(HUMAN21_MUNE,CHEST_COLLISION,CHEST_HEIGHT,1,0,
		scratchpad,work);
#else
    fpu_CopyVector(&(scratchpad->x[HUMAN21_MUNE]),
		   (FVECTOR *)&(scratchpad->objmat[HUMAN21_MUNE].m[3][0]));
#endif


#define MUNE_COS	(0.965925826f /* cos 15   */)
#define MUNE_SIN	(0.258819045f /* sin 15   */)
#define MUNE_COS2	(0.991444861f /* cos 15/2 */)
#define MUNE_SIN2	(0.130526192f /* sin 15/2 */)


    /* 胸の補正 */
    AdjustChest(CHEST_COLLISION,CHEST_HEIGHT,1,0,
		scratchpad,work);

    /* 胸の回転に対する禁則処理 */
    {
	FVECTOR r,n,q;
	int flag=0;

	MT_QuatInverse(&q,&(scratchpad->abs_rots[HUMAN21_ONAKA]));
	MT_QuatMul(&q,&(scratchpad->abs_rots[HUMAN21_MUNE]),&q);

	//printf("Q  = %f %f %f %f\n",q.vx,q.vy,q.vz,q.vw);

	fpu_ClearVector(&n);
	n.vy=1.0f;
	MT_QuatSeparate(&q,&r,&n);

	if(n.vw<0.0f){
	    n.vx=-n.vx;
	    n.vy=-n.vy;
	    n.vz=-n.vz;
	    n.vw=-n.vw;
	}
	if(n.vw<MUNE_COS2){
	    FVECTOR n1;
	    float p;

	    p=fpu_Rsqrt(1.0f-n.vw*n.vw,MUNE_SIN2);
	    n1.vx=n.vx*p;
	    n1.vy=n.vy*p;
	    n1.vz=n.vz*p;
	    n1.vw=MUNE_COS2;

#if 0
	    printf("q  = %f %f %f %f\n",q.vx,q.vy,q.vz,q.vw);
	    printf("r  = %f %f %f %f\n",r.vx,r.vy,r.vz,r.vw);
	    printf("n  = %f %f %f %f\n",n.vx,n.vy,n.vz,n.vw);
	    printf("n1 = %f %f %f %f\n",n1.vx,n1.vy,n1.vz,n1.vw);

	    puts("");
#endif

	    MT_QuatSlerp(&n,&n,&n1,SLERP_PARAM);
	    MT_QuatNormalize(&n,&n);

	    flag=1;
	}
	if(flag){
	    MT_QuatMul(&q,&r,&n);
	    MT_QuatMul(&(scratchpad->abs_rots[HUMAN21_MUNE]),
		       &q,&(scratchpad->abs_rots[HUMAN21_ONAKA]));
	}
	{
	    FMATRIX m;
	    FVECTOR vec;

	    /* 首、肩の位置を再計算 */
	    MT_QuatMul(&q,&(scratchpad->root),&(scratchpad->abs_rots[HUMAN21_MUNE]));
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
	}
    }


    /* 首の補正 */
    AdjustHead(HEAD_COLLISION,HEAD_HEIGHT,0,0,
	       scratchpad,work);


    /* 肩の位置を計算 */
    AdjustShoulder(HUMAN21_MIGI_KATA,0,scratchpad,work);
    AdjustShoulder(HUMAN21_HIDARI_KATA,0,scratchpad,work);


    /* 右腕の補正 */

    /* x[4]の再計算 */
    AdjustJoint(HUMAN21_MIGI_UDE2,ARM_COLLISION,ARM_HEIGHT,0,0,
		scratchpad,work);

    /* x[5]の再計算 */
    AdjustJoint(HUMAN21_MIGI_TE,HAND_COLLISION,HAND_HEIGHT,0,0,
		scratchpad,work);


    /* 左腕の補正 */

    /* x[8]の再計算 */
    AdjustJoint(HUMAN21_HIDARI_UDE2,ARM_COLLISION,ARM_HEIGHT,0,0,
		scratchpad,work);

    /* x[9]の再計算 */
    AdjustJoint(HUMAN21_HIDARI_TE,HAND_COLLISION,HAND_HEIGHT,0,0,
		scratchpad,work);


#if 0
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_ATAMA]),
		   &(obj->m_ctrl->abs_rots[HUMAN21_KUBI]));
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_MIGI_TE]),
		   &(obj->m_ctrl->abs_rots[HUMAN21_MIGI_UDE2]));
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_TE]),
		   &(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_UDE2]));
#endif


    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_MUNE]),
		   &(scratchpad->abs_rots[HUMAN21_MUNE]));

    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_MIGI_KATA]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_KATA]));
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_MIGI_UDE1]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_UDE1]));
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_MIGI_UDE2]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]));
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_MIGI_TE]),
		   &(scratchpad->abs_rots[HUMAN21_MIGI_UDE2]));

    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_KATA]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_KATA]));
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_UDE1]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE1]));
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_UDE2]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]));
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_HIDARI_TE]),
		   &(scratchpad->abs_rots[HUMAN21_HIDARI_UDE2]));

    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_KUBI]),
		   &(scratchpad->abs_rots[HUMAN21_KUBI]));
    fpu_CopyVector(&(obj->m_ctrl->abs_rots[HUMAN21_ATAMA]),
		   &(scratchpad->abs_rots[HUMAN21_ATAMA]));


#if 0
    ScratchpadFree(scratchpad);
#else
    ScratchpadFree2();
#endif
}
