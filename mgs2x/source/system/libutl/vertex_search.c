//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vertex_search.c
		頂点座標の検索

	1999/12/02 K.Kano
	$Id: vertex_search.c,v 1.1.1.3 2002/11/19 11:43:01 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

#include "gameheader.h"
#include "libutl.h"


#ifndef NEW_KMX_FORMAT
void VertexSearch(FVECTOR *vans,FVECTOR *nans,DG_OBJS *objs,int objnum,FVECTOR *target)
{
    FMATRIX im;
    FVECTOR tans;
    float min=FLT_MAX;
    DG_OBJ *tobj=&(objs->objs[objnum]);
    DG_OBJPACK *pack=tobj->model->packs;
    int size=tobj->n_packs;
    int i;
#if 1 //BP_ASM
//#ifndef PSX2
	FVECTOR _vf31, _vf30 ;
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
		
#ifdef BP_PSX2_ASM
		asm volatile ("
	lh		$8,2(%0)
	prot3w		$8,$8
	lh		$8,0(%0)
	lh		$9,4(%0)
	ppacw		$8,$9,$8
	qmtc2.ni	$8,vf31
	" : : "r"(norms) : "$8","$9");
#else
		_vf31.vx = norms->vx ;
		_vf31.vy = norms->vy ;
		_vf31.vz = norms->vz ;
		_vf31.vw = norms->pad ;
#endif

		verts++; norms++;
		
		vu0_IV0toFVv1();
#ifdef BP_PSX2_ASM
		asm volatile ("vitof12.xyzw	vf30,vf31") ;
#else
		_vf30.vx = _vf31.vx/4096.0f ;
		_vf30.vy = _vf31.vy/4096.0f ;
		_vf30.vz = _vf31.vz/4096.0f ;
		_vf30.vw = _vf31.vw/4096.0f ;
#endif

		for(j=0;j<vsize;j++,verts++,norms++){
			float t;
			
			vu0_Subv2v1v0();
			vu0_Stv1(&tans);
			
			vu0_LdSVv1(verts);
			
#ifdef BP_PSX2_ASM
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
			_vf30.vx = -_vf30.vx ;
			_vf30.vy = -_vf30.vy ;
			_vf30.vz = -_vf30.vz ;
			_vf30.vw = -_vf30.vw ;
			_vf31.vx = norms->vx ;
			_vf31.vy = norms->vy ;
			_vf31.vz = norms->vz ;
			_vf31.vw = norms->pad ;
			
#endif
			
			t=vu0_VectorLength2v2();
			vu0_IV0toFVv1();
			
			if(t<min){
				fpu_CopyVector(vans,&tans);
#ifdef BP_PSX2_ASM
				asm volatile ("sqc2	vf30,0x00(%0)" : : "r"(nans) : "memory" );
#else
				nans->vx = _vf30.vx ; 
				nans->vy = _vf30.vy ; 
				nans->vz = _vf30.vz ; 
				nans->vw = _vf30.vw ; 
#endif
				min=t;
			}
			
#ifdef BP_PSX2_ASM
			asm volatile ("vitof12.xyzw	vf30,vf31" ) ;
#else
			_vf30.vx = _vf31.vx/4096.0f ;
			_vf30.vy = _vf31.vy/4096.0f ;
			_vf30.vz = _vf31.vz/4096.0f ;
			_vf30.vw = _vf31.vw/4096.0f ;
#endif
		}
    }
}

#else
void VertexSearch(FVECTOR *vans,FVECTOR *nans,DG_OBJS *objs,int objnum,FVECTOR *target)
{
	FMATRIX	im;
	FVECTOR	tans;
	float	min = FLT_MAX, t ;
	DG_OBJ	*tobj = &( objs->objs[ objnum ] );
	DG_MDL	*mdl ;
	int		size = tobj->n_packs ;
	int		i;

	FVECTOR			pos, tmp, v ;
	DG_VERTEX_KMSS	*verts ;

	/* ワールド絶対座標をモデルからの相対座標に変換する */
    FastInverseMatrix(&im,&(tobj->world));
	pos = *target ;
	pos.vw = 1.0f ;
	_sceVu0ApplyMatrix( &pos, &im, &pos );

	v.vw = 1.0f ;
	verts = tobj->vbuff ;
	mdl = tobj->model ;
	for ( i = 0 ; i < mdl->n_verts ; i++ ){
		v.vx = verts->vx ;
		v.vy = verts->vy ;
		v.vz = verts->vz ;
		_sceVu0SubVector( &tmp, &pos, &v );
		t = _sceVu0InnerProduct( &tmp, &tmp );
		if ( t < min ){
			*vans = v ;
			/* 法線は逆方向に補正 */
			nans->vx = verts->nx * ( -1.0f / 32767.0f ) ;
			nans->vy = verts->ny * ( -1.0f / 32767.0f ) ;
			nans->vz = verts->nz * ( -1.0f / 32767.0f ) ;
			min = t ;
		}
		verts = (void*)( (int)verts + mdl->stride );
	}

}
#endif

