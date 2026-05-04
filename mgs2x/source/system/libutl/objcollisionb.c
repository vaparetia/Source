//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	objcollisionb.c
		バウンダリボックスを使って、楕円球計算で当たり判定を
		行なう。

	1999/10/18 K.Kano
	$Id: objcollisionb.c,v 1.1.1.3 2002/11/19 11:42:57 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

#include "gameheader.h"
#include "libutl.h"

#if 1//BP_ASM - Xbox version is in C - #ifdef KP_XBOX  /**************XBOX用の専用関数(書換え版)***********************  T.Morita 2002.02.01 */
static inline int CalcObjCollisionSubB(FVECTOR *x,FVECTOR *base,DG_MDL *mdl,DG_OBJ *obj,
				       float paramf,FMATRIX *inv_m)
{
    FVECTOR center,length;
    FVECTOR v,v2;
    int flag=0;
	float Q ;
	FVECTOR vf16, vf17, vf18, vf19, vf30, vf31, vf24, vf25 ;

    /* 当たりに当たっている場合、楕円球の表面の、
       baseに一番近い点へ移動させる */
    center.vx=mdl->lx+mdl->ux;
    center.vy=mdl->ly+mdl->uy;
    center.vz=mdl->lz+mdl->uz;

    length.vx=mdl->ux-mdl->lx;
    length.vy=mdl->uy-mdl->ly;
    length.vz=mdl->uz-mdl->lz;

	vu0_ApplyMatrixXYZ( &vf16, inv_m, x ) ;
	vu0_ApplyMatrix( &vf31, inv_m, base ) ;

	fpu_MulVectorScaler( &vf17, &center, 0.5f ) ;
	fpu_MulVectorScaler( &vf18, &length, 0.5f*paramf ) ;
	fpu_SubVectors( &vf16, &vf16, &vf17 ) ;
	fpu_SubVectors( &vf31, &vf31, &vf17 ) ;
	fpu_MulVectors( &vf18, &vf18, &vf18 ) ;
	fpu_MulVectors( &vf19, &vf16, &vf16 ) ;
	fpu_MulVectors( &vf30, &vf31, &vf31 ) ;

	vf18.vw = 1.0f;
	vf24.vx = vf18.vx * vf18.vy * vf18.vz ;
	vf24.vy = vf18.vy * vf18.vz * vf18.vw ;
	vf24.vz = vf18.vz * vf18.vw * vf18.vx ;
	vf24.vw = vf18.vw * vf18.vx * vf18.vy ;
	vf18.vx = vf24.vy ;
	vf18.vy = vf24.vz ;
	vf18.vz = vf24.vw ;
	vf18.vw = vf24.vx ;

	fpu_MulVectors( &vf24, &vf18, &vf19 ) ;
	vf24.vw = vf24.vx + vf24.vy + vf24.vz ;
	fpu_MulVectors( &vf25, &vf18, &vf30 ) ;
	vf25.vw = vf25.vx + vf25.vy + vf25.vz ;

//	v.vw  = vf18.vw ;
//	v2.vw = vf24.vw ;
	fpu_CopyVector( &v , &vf18 );
	fpu_CopyVector( &v2, &vf24 );

    if(v.vw>v2.vw){
		flag = 1 ;

		if ( vf25.vw==0.0f ) vf25.vw = 0.00001f ;
		Q = bp_sqrtf( vf18.vw / vf25.vw ) ; //BP_MATH - emulate PS2 sqrtf
		fpu_MulVectorScaler( &vf31, &vf31, Q ) ;
		fpu_AddVectors( &vf31, &vf31, &vf17 ) ;
		vu0_ApplyMatrix( x, &obj->world, &vf31 ) ;
	}

    return flag;
}

#else        /************** PS2用の専用関数(オリジナル)***********************  T.Morita 2002.02.01 */
static inline int CalcObjCollisionSubB(FVECTOR *x,FVECTOR *base,DG_MDL *mdl,DG_OBJ *obj,
				       float paramf,FMATRIX *inv_m)
{
    FVECTOR center,length;
    FVECTOR v,v2;
    int flag=0;

    /* 当たりに当たっている場合、楕円球の表面の、
       baseに一番近い点へ移動させる */
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
    lqc2	vf31,0x00(%8)
    qmtc2	%4,vf24
    qmtc2	%5,vf25

    vmulax.xyzw		ACC,vf20,vf16x
    vmadday.xyzw	ACC,vf21,vf16y
    vmaddaz.xyzw	ACC,vf22,vf16z
    vmaddw.xyzw		vf16,vf23,vf16w

    vmulax.xyzw		ACC,vf20,vf31x
    vmadday.xyzw	ACC,vf21,vf31y
    vmaddaz.xyzw	ACC,vf22,vf31z
    vmaddw.xyzw		vf31,vf23,vf31w

    vmulx.xyz		vf17,vf17,vf24x
    vmulx.xyz		vf18,vf18,vf25x
    vsub.xyz		vf16,vf16,vf17
    vsub.xyz		vf31,vf31,vf17
    vmul.xyz		vf18,vf18,vf18
    vmul.xyz		vf19,vf16,vf16
    vmul.xyz		vf30,vf31,vf31

    vmove.w	vf18,vf0
    vmr32.xyzw	vf24,vf18
    vmr32.xyzw	vf25,vf24
    vmul.xyzw	vf26,vf18,vf24
    vmul.xyzw	vf18,vf26,vf25
    vmr32.xyzw	vf18,vf18

    vmul.xyz	vf24,vf18,vf19
    vmul.xyz	vf25,vf18,vf30
    vmulax.w	ACC,vf0,vf24x
    vmadday.w	ACC,vf0,vf24y
    vmaddz.w	vf24,vf0,vf24z
    vmulax.w	ACC,vf0,vf25x
    vmadday.w	ACC,vf0,vf25y
    vmaddz.w	vf25,vf0,vf25z

    vdiv	Q,vf18w,vf25w
    sqc2	vf18,0x00(%6)
    sqc2	vf24,0x00(%7)
    " :  : "r"(x),"r"(inv_m),"r"(&center),"r"(&length),"r"(0.5f),"r"(0.5f*paramf),
    "r"(&v),"r"(&v2),"r"(base) : "memory" );

    if(v.vw>v2.vw){
	flag=1;

	asm volatile ("
	vwaitq
	vmulq.w		vf24,vf0,Q
	vsqrt		Q,vf24w
	lqc2		vf20,0x00(%1)
	lqc2		vf21,0x10(%1)
	lqc2		vf22,0x20(%1)
	lqc2		vf23,0x30(%1)
	vwaitq
	vmulq.xyz	vf31,vf31,Q
	vadd.xyz	vf31,vf31,vf17

	vmulax.xyzw	ACC,vf20,vf31x
	vmadday.xyzw	ACC,vf21,vf31y
	vmaddaz.xyzw	ACC,vf22,vf31z
	vmaddw.xyzw	vf16,vf23,vf31w

	sqc2	vf16,0x00(%0)
	" :  : "r"(x),"r"(&(obj->world)) : "memory" );
    }

    return flag;
}
#endif /* KP_XBOX ************** PS2用の専用関数(オリジナル)ここまで***********************  T.Morita 2002.02.01 */

int CalcObjsCollisionB(FVECTOR *r,FVECTOR *x,FVECTOR *base,DG_OBJS *objs,float paramf)
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

	flag|=CalcObjCollisionSubB(r,base,mdl+i,objs->objs+i,paramf,&m);
    }

    return flag;
}

int CalcObjsCollisionBWithInvM(FVECTOR *r,FVECTOR *x,FVECTOR *base,DG_OBJS *objs,
			       float paramf,FMATRIX *inv_m)
{
    DG_MDL *mdl=objs->def->models;
    int size=objs->n_models;
    int i;
    int flag=0;

    fpu_CopyVector(r,x);

    for(i=0;i<size;i++){
	flag|=CalcObjCollisionSubB(r,base,mdl+i,objs->objs+i,paramf,inv_m+i);
    }

    return flag;
}

int CalcObjsCollisionBWithInvM2(FVECTOR *r,FVECTOR *x,FVECTOR *base,DG_DEF *def,DG_OBJS *objs,
				float paramf,FMATRIX *inv_m)
{
    DG_MDL *mdl=def->models;
    int size=objs->n_models;
    int i;
    int flag=0;

    fpu_CopyVector(r,x);

    for(i=0;i<size;i++){
	flag|=CalcObjCollisionSubB(r,base,mdl+i,objs->objs+i,paramf,inv_m+i);
    }

    return flag;
}

int CalcObjCollisionB(FVECTOR *r,FVECTOR *x,FVECTOR *base,DG_OBJS *objs,int objnum,float paramf)
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

    flag=CalcObjCollisionSubB(r,base,mdl+i,objs->objs+i,paramf,&m);

    return flag;
}

int CalcObjCollisionBWithInvM(FVECTOR *r,FVECTOR *x,FVECTOR *base,DG_OBJS *objs,int objnum,
			      float paramf,FMATRIX *inv_m)
{
    DG_MDL *mdl=objs->def->models;
    int i=objnum;
    int flag=0;

    fpu_CopyVector(r,x);

    flag=CalcObjCollisionSubB(r,base,mdl+i,objs->objs+i,paramf,inv_m);

    return flag;
}

int CalcObjCollisionBWithInvM2(FVECTOR *r,FVECTOR *x,FVECTOR *base,DG_DEF *def,DG_OBJS *objs,int objnum,
			       float paramf,FMATRIX *inv_m)
{
    DG_MDL *mdl=def->models;
    int i=objnum;
    int flag=0;

    fpu_CopyVector(r,x);

    flag=CalcObjCollisionSubB(r,base,mdl+i,objs->objs+i,paramf,inv_m);

    return flag;
}
