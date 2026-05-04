//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	boundary.c
		バウンダリボックスを使って、当たり判定を行なう。

	1999/10/18 K.Kano
	$Id: boundary.c,v 1.1.1.3 2002/11/19 11:42:53 Yoshizawa1 Exp $
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


static inline int CalcObjBoundarySub(FVECTOR *x,DG_MDL *mdl,DG_OBJ *obj,float paramf,FMATRIX *inv_m)
{
    FVECTOR center,length;
    FVECTOR localx;
    int ans;
    int flag=0;
#ifndef PSX2
	extern FVECTOR REG_vf1, REG_vf2, REG_vf3 ;
#endif


    vu0_Ldv0(x);
    vu0_Ldm0(inv_m);
    vu0_Setv0w1();

    center.vx=mdl->lx+mdl->ux;
    center.vy=mdl->ly+mdl->uy;
    center.vz=mdl->lz+mdl->uz;

    vu0_Mulv0m0v0();

    length.vx=mdl->ux-mdl->lx;
    length.vy=mdl->uy-mdl->ly;
    length.vz=mdl->uz-mdl->lz;

    vu0_Ldv1(&center);
    vu0_Ldv2(&length);
    vu0_Mulv1a(0.5f);
    vu0_Mulv2a(0.5f*paramf);
    vu0_Stv0(&localx);
    vu0_Subv0v1();

#ifdef BP_PSX2_ASM
    asm volatile ("
    vmulx.w	vf16,vf0,vf3
    vmuly.w	vf17,vf0,vf3
    vmulz.w	vf18,vf0,vf3
    vclipw.xyz	vf1,vf16
    vclipw.xyz	vf1,vf17
    vclipw.xyz	vf1,vf18
    vnop
    vnop
    vnop
    vnop
    cfc2	$8,$vi18
    sw		$8,0(%0)
    " : : "r"(&ans) : "$8","memory" );
#else
	ans  = REG_vf1.vx < REG_vf3.vx ? 0xffffffff : 0 ;
	ans |= REG_vf1.vx >-REG_vf3.vx ? 0xffffffff : 0 ;
	ans |= REG_vf1.vy < REG_vf3.vy ? 0xffffffff : 0 ;
	ans |= REG_vf1.vy >-REG_vf3.vy ? 0xffffffff : 0 ;
	ans |= REG_vf1.vz < REG_vf3.vz ? 0xffffffff : 0 ;
	ans |= REG_vf1.vz >-REG_vf3.vz ? 0xffffffff : 0 ;
#endif

    if(!(ans & ((1<<13)|(1<<12)|(1<<9)|(1<<8)|(1<<5)|(1<<4)))){
	FVECTOR v,w;

#ifdef BP_PSX2_ASM
	asm volatile("
	vabs.xyz	vf16,vf1
	vsub.xyz	vf16,vf3,vf16
	sqc2		vf1,(%0)
	sqc2		vf16,(%1)
	" : : "r"(&v),"r"(&w) : "memory" );
#else
	w.vx = fpu_Abs( REG_vf1.vx ) ;
	w.vy = fpu_Abs( REG_vf1.vy ) ;
	w.vz = fpu_Abs( REG_vf1.vz ) ;
	fpu_SubVectors( &w, &REG_vf3, &w ) ;
	vu0_Stv0( &v ) ;
#endif

	vu0_Ldv0(&localx);
	vu0_Ldm0(&(obj->world));
	vu0_Setv0w1();

	if(w.vx<w.vy){
	    if(w.vx<w.vz){
		/* X */
#ifdef BP_PSX2_ASM
		if(v.vx<0.0f) asm volatile("vsub.x vf1,vf2,vf3");
		else  asm volatile("vadd.x vf1,vf2,vf3");
#else
		if(v.vx<0.0f)
		  REG_vf1.vx = REG_vf2.vx - REG_vf3.vx ;
		else
		  REG_vf1.vx = REG_vf2.vx + REG_vf3.vx ;
#endif
	    }
	    else{
		/* Z */
#ifdef BP_PSX2_ASM
		if(v.vz<0.0f) asm volatile("vsub.z vf1,vf2,vf3");
		else asm volatile("vadd.z vf1,vf2,vf3");
#else
		if(v.vz<0.0f)
		  REG_vf1.vz = REG_vf2.vz - REG_vf3.vz ;
		else
		  REG_vf1.vz = REG_vf2.vz + REG_vf3.vz ;
#endif
	    }
	}
	else{

	    if(w.vy<w.vz){
		/* Y */
#ifdef BP_PSX2_ASM
		if(v.vy<0.0f) asm volatile("vsub.y vf1,vf2,vf3");
		else asm volatile("vadd.y vf1,vf2,vf3");
#else
		if(v.vy<0.0f)
		  REG_vf1.vz = REG_vf2.vz - REG_vf3.vz ;
		else
		  REG_vf1.vz = REG_vf2.vz + REG_vf3.vz ;
#endif
	    }
	    else{
		/* Z */
#ifdef BP_PSX2_ASM
		if(v.vz<0.0f) asm volatile("vsub.z vf1,vf2,vf3");
		else asm volatile("vadd.z vf1,vf2,vf3");
#else
		if(v.vz<0.0f)
		  REG_vf1.vz = REG_vf2.vz - REG_vf3.vz ;
		else
		  REG_vf1.vz = REG_vf2.vz + REG_vf3.vz ;
#endif
	    }
	}

	vu0_Mulv0m0v0();

	flag=1;
	vu0_Stv0(x);
    }

    return flag;
}


int CalcObjsBoundary(FVECTOR *r,FVECTOR *x,DG_OBJS *objs,float paramf)
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
#else
	sceVu0InversMatrix(*(sceVu0FMATRIX *)&m,*(sceVu0FMATRIX *)&(objs->objs[i].world));
#endif

	flag|=CalcObjBoundarySub(r,mdl+i,objs->objs+i,paramf,&m);
    }

    return flag;
}

int CalcObjsBoundaryWithInvM(FVECTOR *r,FVECTOR *x,DG_OBJS *objs,float paramf,FMATRIX *inv_m)
{
    DG_MDL *mdl=objs->def->models;
    int size=objs->n_models;
    int i;
    int flag=0;

    fpu_CopyVector(r,x);

    for(i=0;i<size;i++){
	flag|=CalcObjBoundarySub(r,mdl+i,objs->objs+i,paramf,inv_m+i);
    }

    return flag;
}

int CalcObjBoundary(FVECTOR *r,FVECTOR *x,DG_OBJS *objs,int objnum,float paramf)
{
    DG_MDL *mdl=objs->def->models;
    FMATRIX m;
    int i=objnum;
    int flag=0;

    fpu_CopyVector(r,x);


#if 0
    InverseMatrix(&m,&(objs->objs[i].world));
#else
    sceVu0InversMatrix(*(sceVu0FMATRIX *)&m,*(sceVu0FMATRIX *)&(objs->objs[i].world));
#endif

    flag=CalcObjBoundarySub(r,mdl+i,objs->objs+i,paramf,&m);

    return flag;
}
