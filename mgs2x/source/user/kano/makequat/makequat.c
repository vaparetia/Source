/*
	makequat.c
		しょっちゅう使うので独立させた

	1999/11/4 K.Kano
	$Id: makequat.c,v 1.1.1.3 2002/11/19 11:43:14 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#include <float.h>

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

    t=fpu_Sqrt(s/t);
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

    t=fpu_Sqrt(s/t);
    vu0_Mulv2a(t);
    vu0_Stv2(q);
    q->vw=c;

    return 1;
}
