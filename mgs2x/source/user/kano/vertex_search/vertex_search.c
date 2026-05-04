/*
	vertex_search.c
		頂点座標の検索

	1999/12/02 K.Kano
	$Id: vertex_search.c,v 1.1.1.3 2002/11/19 11:43:44 Yoshizawa1 Exp $
*/


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

#include "gameheader.h"
#include "libutl.h"


void VertexSearch(FVECTOR *vans,FVECTOR *nans,DG_OBJS *objs,int objnum,FVECTOR *target)
{
    FMATRIX im;
    FVECTOR tans;
    float min=FLT_MAX;
    DG_OBJ *tobj=&(objs->objs[objnum]);
    DG_OBJPACK *pack=tobj->packs;
    int size=tobj->n_packs;
    int i;
#ifndef PSX2
	FVECTOR vf31, vf29 ;
	IVECTOR vf30 ;
#endif

#if 0
    InverseMatrix(&im,&(tobj->world));
#elif 0
    sceVu0InversMatrix(*(sceVu0FMATRIX *)&im,*(sceVu0FMATRIX *)&(tobj->world));
#else
    FastInverseMatrix(&im,&(tobj->world));
#endif

    vu0_Ldv0(target);
    vu0_Setv0w1();

    vu0_Ldm0(&im);

    vu0_Mulv0m0v0();

    for(i=0;i<size;i++,pack++){
	SVECTOR *verts=(SVECTOR *)(pack->verts);
	SVECTOR *norms=(SVECTOR *)(pack->norms);
	int vsize=pack->n_verts;
	int j;

	vu0_LdSVv1(verts);

#ifdef PSX2
	asm volatile ("
	lh		$8,2(%0)
	prot3w		$8,$8
	lh		$8,0(%0)
	lh		$9,4(%0)
	ppacw		$8,$9,$8
	qmtc2.ni	$8,vf31
	" : : "r"(norms) : "$8","$9");
#else
	vf31.vx = norms->vx ;
	vf31.vy = norms->vy ;
	vf31.vz = norms->vz ;
	vf31.vw = norms->vw ;
#endif

	verts++; norms++;

	vu0_IV0toFVv1();

#ifdef PSX2
	asm volatile ("
	vitof12.xyzw	vf30,vf31
	");
#else
	vf30.vx = vf31.vx/4096.0f ;
	vf30.vy = vf31.vy/4096.0f ;
	vf30.vz = vf31.vz/4096.0f ;
	vf30.vw = vf31.vw/4096.0f ;
#endif

	for(j=0;j<vsize;j++,verts++,norms++){
	    float t;

	    vu0_Subv2v1v0();
	    vu0_Stv1(&tans);

	    vu0_LdSVv1(verts);

#ifdef PSX2
	    asm volatile ("
	    lh		$8,2(%0)
	    prot3w	$8,$8
	    lh		$8,0(%0)
	    lh		$9,4(%0)
	    qmtc2.ni	%1,vf29
	    ppacw	$8,$9,$8
	    vmulx.xyz	vf30,vf30,vf29x
	    qmtc2.ni	$8,vf31
	    " : : "r"(norms),"r"(-1.0f) : "$8","$9");
#else
		vf30.vx = -(vf31.vx = norms->vx) ;
		vf30.vy = -(vf31.vy = norms->vy) ;
		vf30.vz = -(vf31.vz = norms->vz) ;
		vf30.vw = -(vf31.vw = norms->vw) ;
#endif

	    t=vu0_VectorLength2v2();
	    vu0_IV0toFVv1();

	    if(t<min){
		fpu_CopyVector(vans,&tans);
#ifdef PSX2
		asm volatile ("sqc2	vf30,0x00(%0)" : : "r"(nans) : "memory" );
#else
		nans->vx = vf30.vx ; 
		nans->vy = vf30.vy ; 
		nans->vz = vf30.vz ; 
		nans->vw = vf30.vw ; 
#endif
		min=t;
	    }

#ifdef PSX2
	    asm volatile ("
	    vitof12.xyzw	vf30,vf31
	    ");
#else
		vf30.vx = vf31.vx/4096.0f ;
		vf30.vy = vf31.vy/4096.0f ;
		vf30.vz = vf31.vz/4096.0f ;
		vf30.vw = vf31.vw/4096.0f ;
#endif
	}
    }
}
