//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	objcollision.c
		バウンダリボックスを使って、楕円球計算で当たり判定を
		行なう。

	1999/10/18 K.Kano
	$Id: objcollision.c,v 1.1.1.3 2002/11/19 11:42:57 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "libutl.h"


static inline int CalcObjCollisionSub(FVECTOR *x,DG_MDL *mdl,DG_OBJ *obj,float paramf,FMATRIX *inv_m)
{
    FVECTOR center,length;
    FVECTOR v,v2;
    int flag=0;

#if 1 //BP_ASM
//#ifdef KP_XBOX
    FVECTOR localx;
    float xyz;

    center.vx=mdl->lx+mdl->ux;
    center.vy=mdl->ly+mdl->uy;
    center.vz=mdl->lz+mdl->uz;

    length.vx=mdl->ux-mdl->lx;
    length.vy=mdl->uy-mdl->ly;
    length.vz=mdl->uz-mdl->lz;

    vu0_Ldv0(x);
    vu0_Ldm0(inv_m);
    vu0_Setv0w1();
    vu0_Ldv1(&center);
    vu0_Ldv2(&length);

    vu0_Mulv0m0v0();

    vu0_Mulv1a(0.5f);
    vu0_Mulv2a(0.5f*paramf);
    vu0_Subv0v1();
    vu0_VectorPow2v2();
    vu0_Stv0(&localx);
    vu0_VectorPow2v0();
    vu0_MakeOvalParamsv2();
    xyz=vu0_InnerProductv0v2();
    vu0_Stv2(&v);

    if(xyz<v.vw){
	vu0_SqrtQ(v.vw/xyz);
	vu0_Ldv0(&localx);
	vu0_Ldm0(&(obj->world));

	vu0_WaitQ();
	vu0_Mulv0Q();
	vu0_Addv0v1();

	vu0_Mulv0m0v0();

	flag=1;
	vu0_Stv0(x);
    }

#else

    center.vx=mdl->lx+mdl->ux;
    center.vy=mdl->ly+mdl->uy;
    center.vz=mdl->lz+mdl->uz;

    length.vx=mdl->ux-mdl->lx;
    length.vy=mdl->uy-mdl->ly;
    length.vz=mdl->uz-mdl->lz;

    asm volatile ("
    lqc2	vf16,0x00(%0)
    lqc2	vf20,0x00(%1)
    lqc2	vf21,0x10(%1)
    lqc2	vf22,0x20(%1)
    lqc2	vf23,0x30(%1)
    vmove.w	vf16,vf0
    lqc2	vf17,0x00(%2)
    lqc2	vf18,0x00(%3)
    qmtc2	%4,vf24
    qmtc2	%5,vf25

    vmulax.xyzw		ACC,vf20,vf16x
    vmadday.xyzw	ACC,vf21,vf16y
    vmaddaz.xyzw	ACC,vf22,vf16z
    vmaddw.xyzw		vf16,vf23,vf16w

    vmulx.xyz		vf17,vf17,vf24x
    vmulx.xyz		vf18,vf18,vf25x
    vsub.xyz		vf16,vf16,vf17
    vmul.xyz		vf18,vf18,vf18
    vmul.xyz		vf19,vf16,vf16

    vmove.w	vf18,vf0
    vmr32.xyzw	vf24,vf18
    vmr32.xyzw	vf25,vf24
    vmul.xyzw	vf26,vf18,vf24
    vmul.xyzw	vf18,vf26,vf25
    vmr32.xyzw	vf18,vf18

    vmul.xyz	vf24,vf18,vf19
    vmulax.w	ACC,vf0,vf24x
    vmadday.w	ACC,vf0,vf24y
    vmaddz.w	vf24,vf0,vf24z

    vdiv	Q,vf18w,vf24w
    sqc2	vf18,0x00(%6)
    sqc2	vf24,0x00(%7)
    " :  : "r"(x),"r"(inv_m),"r"(&center),"r"(&length),"r"(0.5f),"r"(0.5f*paramf),
    "r"(&v),"r"(&v2) : "memory" );

    if(v.vw>v2.vw){
	flag=1;

	asm volatile ("
	vwaitq
	vmulq.w		vf24,vf0,Q
	vnop
	vnop
	vsqrt		Q,vf24w
	lqc2		vf20,0x00(%1)
	lqc2		vf21,0x10(%1)
	lqc2		vf22,0x20(%1)
	lqc2		vf23,0x30(%1)
	vwaitq
	vmulq.xyz	vf16,vf16,Q
	vadd.xyz	vf16,vf16,vf17

	vmulax.xyzw	ACC,vf20,vf16x
	vmadday.xyzw	ACC,vf21,vf16y
	vmaddaz.xyzw	ACC,vf22,vf16z
	vmaddw.xyzw	vf16,vf23,vf16w

	sqc2	vf16,0x00(%0)
	" :  : "r"(x),"r"(&(obj->world)) : "memory" );
    }

#endif

    return flag;
}


int CalcObjsCollision(FVECTOR *r,FVECTOR *x,DG_OBJS *objs,float paramf)
{
    DG_MDL *mdl=objs->def->models;
    FMATRIX m;
    int size=objs->n_models;
    int i;
    int flag=0;

    fpu_CopyVector(r,x);

    for(i=0;i<size;i++){
#if 0
	InverseMatrix(&m,&(objs->objs[i].world));
#elif 0
	sceVu0InversMatrix(*(sceVu0FMATRIX *)&m,*(sceVu0FMATRIX *)&(objs->objs[i].world));
#else
	FastInverseMatrix(&m,&(objs->objs[i].world));
#endif

	flag|=CalcObjCollisionSub(r,mdl+i,objs->objs+i,paramf,&m);
    }

    return flag;
}

int CalcObjCollision(FVECTOR *r,FVECTOR *x,DG_OBJS *objs,int objnum,float paramf)
{
    DG_MDL *mdl=objs->def->models;
    FMATRIX m;
    int i=objnum;
    int flag=0;

    fpu_CopyVector(r,x);

#if 0
    InverseMatrix(&m,&(objs->objs[i].world));
#elif 0
    sceVu0InversMatrix(*(sceVu0FMATRIX *)&m,*(sceVu0FMATRIX *)&(objs->objs[i].world));
#else
    FastInverseMatrix(&m,&(objs->objs[i].world));
#endif

    flag=CalcObjCollisionSub(r,mdl+i,objs->objs+i,paramf,&m);

    return flag;
}

int CalcObjsCollisionWithInvM(FVECTOR *r,FVECTOR *x,DG_OBJS *objs,float paramf,FMATRIX *inv_m)
{
    DG_MDL *mdl=objs->def->models;
    int size=objs->n_models;
    int i;
    int flag=0;

    fpu_CopyVector(r,x);

    for(i=0;i<size;i++){
	flag|=CalcObjCollisionSub(r,mdl+i,objs->objs+i,paramf,inv_m+i);
    }

    return flag;
}

int CalcObjsCollisionWithInvM2(FVECTOR *r,FVECTOR *x,DG_DEF *def,DG_OBJS *objs,
			       float paramf,FMATRIX *inv_m)
{
    DG_MDL *mdl=def->models;
    int size=objs->n_models;
    int i;
    int flag=0;

    fpu_CopyVector(r,x);

    for(i=0;i<size;i++){
	flag|=CalcObjCollisionSub(r,mdl+i,objs->objs+i,paramf,inv_m+i);
    }

    return flag;
}

int CalcObjCollisionWithInvM(FVECTOR *r,FVECTOR *x,DG_OBJS *objs,int objnum,
			     float paramf,FMATRIX *inv_m)
{
    DG_MDL *mdl=objs->def->models;
    int flag;

    fpu_CopyVector(r,x);
    flag=CalcObjCollisionSub(r,mdl+objnum,objs->objs+objnum,paramf,inv_m);
    return flag;
}

int CalcObjCollisionWithInvM2(FVECTOR *r,FVECTOR *x,DG_DEF *def,DG_OBJS *objs,int objnum,
			      float paramf,FMATRIX *inv_m)
{
    DG_MDL *mdl=def->models;
    int flag;

    fpu_CopyVector(r,x);
    flag=CalcObjCollisionSub(r,mdl+objnum,objs->objs+objnum,paramf,inv_m);
    return flag;
}
