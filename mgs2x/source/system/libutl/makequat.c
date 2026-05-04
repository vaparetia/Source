//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	makequat.c
		しょっちゅう使うので独立させた

	1999/11/4 K.Kano
	$Id: makequat.c,v 1.1.1.3 2002/11/19 11:42:55 Yoshizawa1 Exp $
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <float.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "libutl.h"


int UTL_MakeQuat(FVECTOR *q,FVECTOR *x0,FVECTOR *x1,FVECTOR *srcv)
{
    float s,t,c;

    vu0_Ldv2(srcv);
    vu0_Ldv1(x0);
    vu0_Ldv0(x1);
    vu0_Setv2w0();

    vu0_Subv1v0v1();

    t=vu0_VectorLength2v2();
    s=vu0_InnerProductv1v2();

    t*=vu0_VectorLength2v1();
    t=fpu_Rsqrt(t,s);
    if(t<-0.9999f){
		fpu_ClearVector(q);
		q->vx=1.0f;
		return 0;
    }
    s=(1.0f-t)*0.5f;
    c=fpu_Sqrt((1.0f+t)*0.5f);
    vu0_OuterProductv2v1();
    t=vu0_VectorLength2v2();

    t=fpu_Sqrt( t!=0.0f ? s/t : 1.0e+24f );
    vu0_Mulv2a(t);
    vu0_Stv2(q);
    q->vw=c;

    return 1;
}

int UTL_MakeQuatM(FVECTOR *q,FVECTOR *x0,FVECTOR *x1,FMATRIX *m,FVECTOR *srcv)
{
    float s,t,c;

    vu0_Ldv2(srcv);
    vu0_Ldm0(m);
    vu0_Ldv1(x0);
    vu0_Setv2w0();
    vu0_Ldv0(x1);

    vu0_Mulv2m0v2();
    vu0_Subv1v0v1();

    t=vu0_VectorLength2v2();
    s=vu0_InnerProductv1v2();

    t*=vu0_VectorLength2v1();
    t=fpu_Rsqrt(t,s);
    if(t<-0.9999f){
		fpu_ClearVector(q);
		q->vx=1.0f;
		return 0;
    }
    s=(1.0f-t)*0.5f;
    c=fpu_Sqrt((1.0f+t)*0.5f);
    vu0_OuterProductv2v1();
    t=vu0_VectorLength2v2();

    t=fpu_Sqrt( t!=0.0f ? s/t : 1.0e+24f );
    vu0_Mulv2a(t);
    vu0_Stv2(q);
    q->vw=c;

    return 1;
}

void UTL_MakeQuatFromOBJ(FVECTOR *q,DG_DEF *def,FVECTOR *t,SVECTOR *r,
						 OBJECT *target,int objnum,FVECTOR *x,int objnum2,FVECTOR *x2)
{
	DG_MDL *mdl=def->models;
	FMATRIX rotm;
	FVECTOR from,to;
	FVECTOR v;

	DG_SetPos(&DG_UnitMatrix);
	if(r!=NULL) DG_RotatePosZYX(r);
	DG_GetPos(&rotm);

	if(t!=NULL) fpu_CopyVector(&v,t);
	else{
		if(fpu_Abs(mdl[0].lx)<fpu_Abs(mdl[0].ux)) v.vx=mdl[0].ux;
		else v.vx=mdl[0].lx;
		if(fpu_Abs(mdl[0].ly)<fpu_Abs(mdl[0].uy)) v.vy=mdl[0].uy;
		else v.vy=mdl[0].ly;
		if(fpu_Abs(mdl[0].lz)<fpu_Abs(mdl[0].uz)) v.vz=mdl[0].uz;
		else v.vz=mdl[0].lz;

		if(fpu_Abs(v.vx)>fpu_Abs(v.vy)){
			if(fpu_Abs(v.vx)>fpu_Abs(v.vz)){
				/* X */
				v.vy=v.vz=0.0f;
			}
			else{
				/* Z */
				v.vx=v.vy=0.0f;
			}
		}
		else{
			if(fpu_Abs(v.vy)>fpu_Abs(v.vz)){
				/* Y */
				v.vx=v.vz=0.0f;
			}
			else{
				/* Z */
				v.vx=v.vy=0.0f;
			}
		}
	}

	vu0_Ldv0(x);
	vu0_Ldm0(&(target->objs->objs[objnum].world));
	vu0_Setv0w1();

	vu0_Ldm1(&rotm);

	vu0_Ldv1(x2);

	vu0_Mulv0m0v0();
	vu0_Mulm2m0m1();

	vu0_Setv1w1();

	vu0_Ldm0(&(target->objs->objs[objnum2].world));
	vu0_Stm2(&rotm);

	vu0_Stv0(&from);

	vu0_Mulv1m0v1();

	vu0_Stv1(&to);


	UTL_MakeQuatM(q,&from,&to,&rotm,&v);
}
