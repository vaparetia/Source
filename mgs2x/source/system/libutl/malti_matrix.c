//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	malti_matrix.c
		OBJを関節以外の行列を使って動かす。
		乳揺れ、腹揺れ用

	1999/12/06 K.Kano
	$Id: malti_matrix.c,v 1.4 2002/11/23 11:50:59 Yoshizawa1 Exp $
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#ifdef PSX2
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


#if 1//BP_ASM - (XBOX version is C) - #ifdef KP_XBOX  /**************XBOX用の専用関数(書換え版)***********************  T.Morita 2002.02.04 */


/* 一つの頂点に複数のマトリクス演算を行なう。*/
/* エンベロープを考慮した場合、下のような式になることがわかったので、
   仕様を一部変更した。インターフェイスに変更は無い。*/
/*
  r = sm^(-1) ( p * ( sm + m0 + m1 + m2 + ... ) + (1-p) * ( psm + m0' + m1' + m2' + ... ) ) a
    = sm^(-1) ( p * sm * ( I + m0'' + m1'' + m2'' + ... )
                + (1-p) * psm * ( I + m0'' + m1'' + m2'' + ... ) ) a
    = sm^(-1) ( psm + p * ( sm - psm ) ) ( I + m0'' + m1'' + m2'' + ... ) a
    = ( sm^(-1) psm + p * sm^(-1) ( sm -  psm ) ) ( I + m0'' + m1'' + m2'' + ... ) a
 */
static void MaltiMatrix(FVECTOR *r,FVECTOR *a,float *p,int size,FMATRIX *m,int msize)
{
    int i,j;

    /* 頂点を4つづつ計算し、結果をスクラッチに格納 */
    for( i=0 ; i<size ; i+=4, a+=4, r+=4 ){
		FVECTOR vf31 = {1.0f, 1.0f, 1.0f, 1.0f } ;
		FVECTOR vf29 ;
		FVECTOR v[4] ;
		
		fpu_ClearVector( &vf29 ) ;
		for( j=0 ; j<4 ;j++ ){
			fpu_ClearVector( r+j ) ;
		}
		
		for(j=0;j<msize;j++,p+=4){

			vu0_ApplyMatrix( &v[0], m+j, a+0 ) ;
			vu0_ApplyMatrix( &v[1], m+j, a+1 ) ;
			vu0_ApplyMatrix( &v[2], m+j, a+2 ) ;
			vu0_ApplyMatrix( &v[3], m+j, a+3 ) ;

			fpu_MulVectorScaler( &v[0], &v[0], *(p+0) ) ;
			fpu_AddVectors( r+0, r+0, &v[0] ) ;
			fpu_MulVectorScaler( &v[1], &v[1], *(p+1) ) ;
			fpu_AddVectors( r+1, r+1, &v[1] ) ;
			fpu_MulVectorScaler( &v[2], &v[2], *(p+2) ) ;
			fpu_AddVectors( r+2, r+2, &v[2] ) ;
			fpu_MulVectorScaler( &v[3], &v[3], *(p+3) ) ;
			fpu_AddVectors( r+3, r+3, &v[3] ) ;

			_sceVu0AddVector( &vf29, &vf29, p ) ;
		}

		_sceVu0SubVector( &vf31, &vf31, &vf29 ) ;
			
		fpu_MulVectorScaler( &v[0], r+0, 1.0f-vf29.vx ) ;
		fpu_AddVectors( r+0, r+0, &v[0] ) ;
		fpu_MulVectorScaler( &v[1], r+1, 1.0f-vf29.vy ) ;
		fpu_AddVectors( r+1, r+1, &v[1] ) ;
		fpu_MulVectorScaler( &v[2], r+2, 1.0f-vf29.vz ) ;
		fpu_AddVectors( r+2, r+2, &v[2] ) ;
		fpu_MulVectorScaler( &v[3], r+3, 1.0f-vf29.vw ) ;
		fpu_AddVectors( r+3, r+3, &v[3] ) ;
	}
}

/* 一つの法線に複数のマトリクス演算を行なう。*/
static void MaltiMatrixN(FVECTOR *r,FVECTOR *a,float *p,int size,FMATRIX *m,int msize)
{
    int i,j;

    /* 法線を4つづつ計算し、結果をスクラッチに格納 */
    for( i=0 ; i<size ; i+=4, a+=4, r+=4 ){
		FVECTOR cv={ 1.0f, 1.0f, 1.0f, 1.0f };

		FVECTOR vf31 = {1.0f, 1.0f, 1.0f, 1.0f } ;
		FVECTOR vf29 ;
		FVECTOR v[4] ;
		
		fpu_ClearVector( &vf29 ) ;
		for( j=0 ; j<4 ;j++ ){
			fpu_ClearVector( r+j ) ;
		}
		
		for(j=0;j<msize;j++,p+=4){

			vu0_ApplyMatrixROT( &v[0], m+j, a+0 ) ;
			vu0_ApplyMatrixROT( &v[1], m+j, a+1 ) ;
			vu0_ApplyMatrixROT( &v[2], m+j, a+2 ) ;
			vu0_ApplyMatrixROT( &v[3], m+j, a+3 ) ;


			fpu_MulVectorScaler( &v[0], &v[0], *(p+0) ) ;
			fpu_AddVectors( r+0, r+0, &v[0] ) ;
			fpu_MulVectorScaler( &v[1], &v[1], *(p+1) ) ;
			fpu_AddVectors( r+1, r+1, &v[1] ) ;
			fpu_MulVectorScaler( &v[2], &v[2], *(p+2) ) ;
			fpu_AddVectors( r+2, r+2, &v[2] ) ;
			fpu_MulVectorScaler( &v[3], &v[3], *(p+3) ) ;
			fpu_AddVectors( r+3, r+3, &v[3] ) ;

			fpu_AddVectors( &vf29, &vf29, p ) ;
		}
		fpu_SubVectors( &vf31, &vf31, &vf29 ) ;
			
		fpu_MulVectorScaler( &v[0], r+0, vf31.vx ) ;
		fpu_AddVectors( r+0, r+0, &v[0] ) ;
		fpu_MulVectorScaler( &v[1], r+1, vf31.vy ) ;
		fpu_AddVectors( r+1, r+1, &v[1] ) ;
		fpu_MulVectorScaler( &v[2], r+2, vf31.vz ) ;
		fpu_AddVectors( r+2, r+2, &v[2] ) ;
		fpu_MulVectorScaler( &v[3], r+3, vf31.vw ) ;
		fpu_AddVectors( r+3, r+3, &v[3] ) ;
	}
}

#else        /************* PS2用の専用関数(オリジナル)***********************  T.Morita 2002.02.04 */


#if 0

/* 一つの頂点に複数のマトリクス演算を行なう。*/
/*
  r = sm^(-1) ( p * sm + (1-p) * psm + m0 + m1 + m2 + ... ) a
    = sm^(-1) ( psm + p * ( sm - psm ) + m0 + m1 + m2 + ... ) a
 */
static void MaltiMatrix(FVECTOR *a,float *p,int size,FMATRIX *sm,FMATRIX *psm,
			FVECTOR *tv,FMATRIX *m,int msize)
{
    FMATRIX *im,*imw;
    SVECTOR *sv=(SVECTOR *)SCRPAD_ADDR;
    int i,j;

    ScratchpadManInit();

    im=ScratchpadAlloc2(sizeof(FMATRIX)*(msize+1));

    imw=im+1;

#if 0
    InverseMatrix(im,sm);
#elif 0
    sceVu0InversMatrix(*(sceVu0FMATRIX *)im,*(sceVu0FMATRIX *)sm);
#else
    FastInverseMatrix(im,sm);
#endif

    vu0_Ldm0(im);
    for(i=0;i<msize;i++){
	vu0_Ldm1(m+i);
	vu0_Mulm2m0m1();
	vu0_Stm2(imw+i);
    }

    if(psm!=NULL){
	asm volatile ("
	lqc2		vf1,0x00(%2)

	lqc2		vf12,0x00(%0)
	lqc2		vf13,0x10(%0)
	lqc2		vf14,0x20(%0)
	lqc2		vf15,0x30(%0)
	lqc2		vf8,0x00(%1)
	lqc2		vf9,0x10(%1)
	lqc2		vf10,0x20(%1)
	lqc2		vf11,0x30(%1)

	vmulax.xyzw	ACC,vf8,vf1x
	vmadday.xyzw	ACC,vf9,vf1y
	vmaddaz.xyzw	ACC,vf10,vf1z
	vmaddw.xyzw	vf11,vf11,vf1w
	vsub.xyzw	vf12,vf12,vf8
	vsub.xyzw	vf13,vf13,vf9
	vsub.xyzw	vf14,vf14,vf10
	vsub.xyzw	vf15,vf15,vf11

	vmulax.xyzw	ACC,vf4,vf8x
	vmadday.xyzw	ACC,vf5,vf8y
	vmaddaz.xyzw	ACC,vf6,vf8z
	vmaddw.xyzw	vf8,vf7,vf8w
	vmulax.xyzw	ACC,vf4,vf9x
	vmadday.xyzw	ACC,vf5,vf9y
	vmaddaz.xyzw	ACC,vf6,vf9z
	vmaddw.xyzw	vf9,vf7,vf9w
	vmulax.xyzw	ACC,vf4,vf10x
	vmadday.xyzw	ACC,vf5,vf10y
	vmaddaz.xyzw	ACC,vf6,vf10z
	vmaddw.xyzw	vf10,vf7,vf10w
	vmulax.xyzw	ACC,vf4,vf11x
	vmadday.xyzw	ACC,vf5,vf11y
	vmaddaz.xyzw	ACC,vf6,vf11z
	vmaddw.xyzw	vf11,vf7,vf11w

	vmulax.xyzw	ACC,vf4,vf12x
	vmadday.xyzw	ACC,vf5,vf12y
	vmaddaz.xyzw	ACC,vf6,vf12z
	vmaddw.xyzw	vf12,vf7,vf12w
	vmulax.xyzw	ACC,vf4,vf13x
	vmadday.xyzw	ACC,vf5,vf13y
	vmaddaz.xyzw	ACC,vf6,vf13z
	vmaddw.xyzw	vf13,vf7,vf13w
	vmulax.xyzw	ACC,vf4,vf14x
	vmadday.xyzw	ACC,vf5,vf14y
	vmaddaz.xyzw	ACC,vf6,vf14z
	vmaddw.xyzw	vf14,vf7,vf14w
	vmulax.xyzw	ACC,vf4,vf15x
	vmadday.xyzw	ACC,vf5,vf15y
	vmaddaz.xyzw	ACC,vf6,vf15z
	vmaddw.xyzw	vf15,vf7,vf15w
	" : : "r"(sm),"r"(psm),"r"(tv) );
    }
    else{
	asm volatile ("
	lqc2		vf12,0x00(%0)
	lqc2		vf13,0x10(%0)
	lqc2		vf14,0x20(%0)
	lqc2		vf15,0x30(%0)
	vmulx.xyzw	vf8,vf0,vf0x
	vmulx.xyzw	vf9,vf0,vf0x
	vmulx.xyzw	vf10,vf0,vf0x
	vmulx.xyzw	vf11,vf0,vf0x
	" : : "r"(&DG_UnitMatrix) );
    }

    /* 頂点を4つづつ計算し、結果をスクラッチに格納 */
    for(i=0;i<size;i+=4,a+=4,sv+=4){
	static const FVECTOR cv={ 1.0f,1.0f,1.0f,1.0f, };

	/* 四捨五入用 */
	static const FVECTOR dt={ 0.5f,0.5f,0.5f,0.0f, };

	asm volatile ("
	lqc2		vf31,0x00(%0)
	lqc2		vf20,0x00(%1)
	lqc2		vf21,0x10(%1)
	lqc2		vf22,0x20(%1)
	lqc2		vf23,0x30(%1)
#if 1
	vmulx.xyzw	vf4,vf0,vf0x
	vmulx.xyzw	vf5,vf0,vf0x
	vmulx.xyzw	vf6,vf0,vf0x
	vmulx.xyzw	vf7,vf0,vf0x
#else
	lqc2		vf4,0x00(%2)
	lqc2		vf5,0x00(%2)
	lqc2		vf6,0x00(%2)
	lqc2		vf7,0x00(%2)
#endif
	" : : "r"(&cv),"r"(a),"r"(&dt) );

	for(j=0;j<msize;j++,p+=4){
	    asm volatile ("
	    lqc2	vf16,0x00(%0)
	    lqc2	vf17,0x10(%0)
	    lqc2	vf18,0x20(%0)
	    lqc2	vf19,0x30(%0)
	    lqc2	vf30,0x00(%1)

	    vmulax.xyzw		ACC,vf16,vf20x
	    vmadday.xyzw	ACC,vf17,vf20y
	    vmaddaz.xyzw	ACC,vf18,vf20z
	    vmaddw.xyzw		vf24,vf19,vf0w
	    vmulax.xyzw		ACC,vf16,vf21x
	    vmadday.xyzw	ACC,vf17,vf21y
	    vmaddaz.xyzw	ACC,vf18,vf21z
	    vmaddw.xyzw		vf25,vf19,vf0w
	    vmulax.xyzw		ACC,vf16,vf22x
	    vmadday.xyzw	ACC,vf17,vf22y
	    vmaddaz.xyzw	ACC,vf18,vf22z
	    vmaddw.xyzw		vf26,vf19,vf0w
	    vmulax.xyzw		ACC,vf16,vf23x
	    vmadday.xyzw	ACC,vf17,vf23y
	    vmaddaz.xyzw	ACC,vf18,vf23z
	    vmaddw.xyzw		vf27,vf19,vf0w

	    pref	0,0x10(%1)

	    vmulx.xyzw		vf24,vf24,vf30x
	    vmuly.xyzw		vf25,vf25,vf30y
	    vmulz.xyzw		vf26,vf26,vf30z
	    vmulw.xyzw		vf27,vf27,vf30w

	    vadd.xyz		vf4,vf4,vf24
	    vadd.xyz		vf5,vf5,vf25
	    vadd.xyz		vf6,vf6,vf26
	    vadd.xyz		vf7,vf7,vf27

	    vsub.xyzw		vf31,vf31,vf30
	    " : : "r"(imw+j),"r"(p) );
	}

	asm volatile ("
	vmulax.xyzw	ACC,vf8,vf20x
	vmadday.xyzw	ACC,vf9,vf20y
	vmaddaz.xyzw	ACC,vf10,vf20z
	vmaddw.xyzw	vf24,vf11,vf0w
	vmulax.xyzw	ACC,vf8,vf21x
	vmadday.xyzw	ACC,vf9,vf21y
	vmaddaz.xyzw	ACC,vf10,vf21z
	vmaddw.xyzw	vf25,vf11,vf0w
	vmulax.xyzw	ACC,vf8,vf22x
	vmadday.xyzw	ACC,vf9,vf22y
	vmaddaz.xyzw	ACC,vf10,vf22z
	vmaddw.xyzw	vf26,vf11,vf0w
	vmulax.xyzw	ACC,vf8,vf23x
	vmadday.xyzw	ACC,vf9,vf23y
	vmaddaz.xyzw	ACC,vf10,vf23z
	vmaddw.xyzw	vf27,vf11,vf0w

	vmulx.xyzw	vf24,vf24,vf31x
	vmuly.xyzw	vf25,vf25,vf31y
	vmulz.xyzw	vf26,vf26,vf31z
	vmulw.xyzw	vf27,vf27,vf31w
	vadd.xyz	vf4,vf4,vf24
	vadd.xyz	vf5,vf5,vf25
	vadd.xyz	vf6,vf6,vf26
	vadd.xyz	vf7,vf7,vf27

	vmulax.xyzw	ACC,vf12,vf20x
	vmadday.xyzw	ACC,vf13,vf20y
	vmaddaz.xyzw	ACC,vf14,vf20z
	vmaddw.xyzw	vf24,vf15,vf0w
	vmulax.xyzw	ACC,vf12,vf21x
	vmadday.xyzw	ACC,vf13,vf21y
	vmaddaz.xyzw	ACC,vf14,vf21z
	vmaddw.xyzw	vf25,vf15,vf0w
	vmulax.xyzw	ACC,vf12,vf22x
	vmadday.xyzw	ACC,vf13,vf22y
	vmaddaz.xyzw	ACC,vf14,vf22z
	vmaddw.xyzw	vf26,vf15,vf0w
	vmulax.xyzw	ACC,vf12,vf23x
	vmadday.xyzw	ACC,vf13,vf23y
	vmaddaz.xyzw	ACC,vf14,vf23z
	vmaddw.xyzw	vf27,vf15,vf0w

	vmulw.xyzw	vf24,vf24,vf20w
	vmulw.xyzw	vf25,vf25,vf21w
	vmulw.xyzw	vf26,vf26,vf22w
	vmulw.xyzw	vf27,vf27,vf23w
	vmulx.xyzw	vf24,vf24,vf31x
	vmuly.xyzw	vf25,vf25,vf31y
	vmulz.xyzw	vf26,vf26,vf31z
	vmulw.xyzw	vf27,vf27,vf31w
	vadd.xyz	vf4,vf4,vf24
	vadd.xyz	vf5,vf5,vf25
	vadd.xyz	vf6,vf6,vf26
	vadd.xyz	vf7,vf7,vf27

	vftoi0.xyz	vf24,vf4
	vftoi0.xyz	vf25,vf5
	vftoi0.xyz	vf26,vf6
	vftoi0.xyz	vf27,vf7
	vftoi12.w	vf24,vf0
	vftoi12.w	vf25,vf0
	vftoi12.w	vf26,vf0
	vftoi12.w	vf27,vf0

	qmfc2		$8,vf24
	qmfc2		$9,vf25
	qmfc2		$10,vf26
	qmfc2		$11,vf27

	ppach		$8,$9,$8
	ppach		$10,$11,$10

	sq		$8,0x00(%0)
	sq		$10,0x10(%0)
	" : : "r"(sv) : "memory","$8","$9","$10","$11" );
    }

    ScratchpadFree2();
}

/* 一つの法線に複数のマトリクス演算を行なう。*/
static void MaltiMatrixN(FVECTOR *a,float *p,int size,FMATRIX *sm,FMATRIX *psm,
			 FMATRIX *m,int msize)
{
    FMATRIX *im,*imw;
    SVECTOR *sv=(SVECTOR *)SCRPAD_ADDR;
    int i,j;

    ScratchpadManInit();

    im=ScratchpadAlloc2(sizeof(FMATRIX)*(msize+1));

    imw=im+1;

#if 0
    InverseMatrix(im,sm);
#elif 0
    sceVu0InversMatrix(*(sceVu0FMATRIX *)im,*(sceVu0FMATRIX *)sm);
#else
    FastInverseMatrix(im,sm);
#endif

    vu0_Ldm0(im);
    for(i=0;i<msize;i++){
	vu0_Ldm1(m+i);
	vu0_Mulm2m0m1();
	vu0_Stm2(imw+i);
    }

    if(psm!=NULL){
	asm volatile ("
	lqc2		vf12,0x00(%0)
	lqc2		vf13,0x10(%0)
	lqc2		vf14,0x20(%0)
	lqc2		vf8,0x00(%1)
	lqc2		vf9,0x10(%1)
	lqc2		vf10,0x20(%1)

	vsub.xyzw	vf12,vf12,vf8
	vsub.xyzw	vf13,vf13,vf9
	vsub.xyzw	vf14,vf14,vf10

	vmulax.xyzw	ACC,vf4,vf8x
	vmadday.xyzw	ACC,vf5,vf8y
	vmaddz.xyzw	vf8,vf6,vf8z
	vmulax.xyzw	ACC,vf4,vf9x
	vmadday.xyzw	ACC,vf5,vf9y
	vmaddz.xyzw	vf9,vf6,vf9z
	vmulax.xyzw	ACC,vf4,vf10x
	vmadday.xyzw	ACC,vf5,vf10y
	vmaddz.xyzw	vf10,vf6,vf10z
	vmulax.xyzw	ACC,vf4,vf11x
	vmadday.xyzw	ACC,vf5,vf11y
	vmaddz.xyzw	vf11,vf6,vf11z

	vmulax.xyzw	ACC,vf4,vf12x
	vmadday.xyzw	ACC,vf5,vf12y
	vmaddz.xyzw	vf12,vf6,vf12z
	vmulax.xyzw	ACC,vf4,vf13x
	vmadday.xyzw	ACC,vf5,vf13y
	vmaddz.xyzw	vf13,vf6,vf13z
	vmulax.xyzw	ACC,vf4,vf14x
	vmadday.xyzw	ACC,vf5,vf14y
	vmaddz.xyzw	vf14,vf6,vf14z
	vmulax.xyzw	ACC,vf4,vf15x
	vmadday.xyzw	ACC,vf5,vf15y
	vmaddz.xyzw	vf15,vf6,vf15z
	" : : "r"(sm),"r"(psm) );
    }
    else{
	asm volatile ("
	lqc2		vf12,0x00(%0)
	lqc2		vf13,0x10(%0)
	lqc2		vf14,0x20(%0)
	vmulx.xyzw	vf8,vf0,vf0x
	vmulx.xyzw	vf9,vf0,vf0x
	vmulx.xyzw	vf10,vf0,vf0x
	" : : "r"(sm) );
    }

    /* 法線を4つづつ計算し、結果をスクラッチに格納 */
    for(i=0;i<size;i+=4,a+=4,sv+=4){
	static const FVECTOR cv={ 1.0f,1.0f,1.0f,1.0f, };

	/* 法線なので回転のみ計算し、移動は計算しない */
	asm volatile ("
	lqc2		vf31,0x00(%0)
	lqc2		vf20,0x00(%1)
	lqc2		vf21,0x10(%1)
	lqc2		vf22,0x20(%1)
	lqc2		vf23,0x30(%1)
	vmulx.xyzw	vf4,vf0,vf0x
	vmulx.xyzw	vf5,vf0,vf0x
	vmulx.xyzw	vf6,vf0,vf0x
	vmulx.xyzw	vf7,vf0,vf0x
	" : : "r"(&cv),"r"(a) );

	for(j=0;j<msize;j++,p+=4){
	    asm volatile ("
	    lqc2	vf16,0x00(%0)
	    lqc2	vf17,0x10(%0)
	    lqc2	vf18,0x20(%0)
	    lqc2	vf30,0x00(%1)

	    vmulax.xyzw		ACC,vf16,vf20x
	    vmadday.xyzw	ACC,vf17,vf20y
	    vmaddz.xyzw		vf24,vf18,vf20z
	    vmulax.xyzw		ACC,vf16,vf21x
	    vmadday.xyzw	ACC,vf17,vf21y
	    vmaddz.xyzw		vf25,vf18,vf21z
	    vmulax.xyzw		ACC,vf16,vf22x
	    vmadday.xyzw	ACC,vf17,vf22y
	    vmaddz.xyzw		vf26,vf18,vf22z
	    vmulax.xyzw		ACC,vf16,vf23x
	    vmadday.xyzw	ACC,vf17,vf23y
	    vmaddz.xyzw		vf27,vf18,vf23z

	    pref	0,0x10(%1)

	    vmulx.xyzw		vf24,vf24,vf30x
	    vmuly.xyzw		vf25,vf25,vf30y
	    vmulz.xyzw		vf26,vf26,vf30z
	    vmulw.xyzw		vf27,vf27,vf30w

	    vadd.xyz		vf4,vf4,vf24
	    vadd.xyz		vf5,vf5,vf25
	    vadd.xyz		vf6,vf6,vf26
	    vadd.xyz		vf7,vf7,vf27

	    vsub.xyzw		vf31,vf31,vf30
	    " : : "r"(imw+j),"r"(p) );
	}

	asm volatile ("
	vmulax.xyzw	ACC,vf8,vf20x
	vmadday.xyzw	ACC,vf9,vf20y
	vmaddz.xyzw	vf24,vf10,vf20z
	vmulax.xyzw	ACC,vf8,vf21x
	vmadday.xyzw	ACC,vf9,vf21y
	vmaddz.xyzw	vf25,vf10,vf21z
	vmulax.xyzw	ACC,vf8,vf22x
	vmadday.xyzw	ACC,vf9,vf22y
	vmaddz.xyzw	vf26,vf10,vf22z
	vmulax.xyzw	ACC,vf8,vf23x
	vmadday.xyzw	ACC,vf9,vf23y
	vmaddz.xyzw	vf27,vf10,vf23z

	vmulx.xyzw	vf24,vf24,vf31x
	vmuly.xyzw	vf25,vf25,vf31y
	vmulz.xyzw	vf26,vf26,vf31z
	vmulw.xyzw	vf27,vf27,vf31w
	vadd.xyz	vf4,vf4,vf24
	vadd.xyz	vf5,vf5,vf25
	vadd.xyz	vf6,vf6,vf26
	vadd.xyz	vf7,vf7,vf27

	vmulax.xyzw	ACC,vf12,vf20x
	vmadday.xyzw	ACC,vf13,vf20y
	vmaddz.xyzw	vf24,vf14,vf20z
	vmulax.xyzw	ACC,vf12,vf21x
	vmadday.xyzw	ACC,vf13,vf21y
	vmaddz.xyzw	vf25,vf14,vf21z
	vmulax.xyzw	ACC,vf12,vf22x
	vmadday.xyzw	ACC,vf13,vf22y
	vmaddz.xyzw	vf26,vf14,vf22z
	vmulax.xyzw	ACC,vf12,vf23x
	vmadday.xyzw	ACC,vf13,vf23y
	vmaddz.xyzw	vf27,vf14,vf23z

	vmulw.xyzw	vf24,vf24,vf20w
	vmulw.xyzw	vf25,vf25,vf21w
	vmulw.xyzw	vf26,vf26,vf22w
	vmulw.xyzw	vf27,vf27,vf23w
	vmulx.xyzw	vf24,vf24,vf31x
	vmuly.xyzw	vf25,vf25,vf31y
	vmulz.xyzw	vf26,vf26,vf31z
	vmulw.xyzw	vf27,vf27,vf31w
	vadd.xyz	vf4,vf4,vf24
	vadd.xyz	vf5,vf5,vf25
	vadd.xyz	vf6,vf6,vf26
	vadd.xyz	vf7,vf7,vf27

	vmul.xyz	vf24,vf4,vf4
	vmul.xyz	vf25,vf5,vf5
	vmul.xyz	vf26,vf6,vf6
	vmul.xyz	vf27,vf7,vf7
	vmulax.w	ACC,vf0,vf24x
	vmadday.w	ACC,vf0,vf24y
	vmaddz.w	vf24,vf0,vf24z
	vmulax.w	ACC,vf0,vf25x
	vmadday.w	ACC,vf0,vf25y
	vmaddz.w	vf25,vf0,vf25z
	vrsqrt		Q,vf0w,vf24w
	vmulax.w	ACC,vf0,vf26x
	vmadday.w	ACC,vf0,vf26y
	vmaddz.w	vf26,vf0,vf26z
	vwaitq
	vmulq.xyz	vf4,vf4,Q
	vrsqrt		Q,vf0w,vf25w
	vmulax.w	ACC,vf0,vf27x
	vmadday.w	ACC,vf0,vf27y
	vmaddz.w	vf27,vf0,vf27z
	vwaitq
	vmulq.xyz	vf5,vf5,Q
	vrsqrt		Q,vf0w,vf26w
	vwaitq
	vmulq.xyz	vf6,vf6,Q
	vrsqrt		Q,vf0w,vf27w
	vwaitq
	vmulq.xyz	vf7,vf7,Q

	vftoi12.xyz	vf24,vf4
	vftoi12.xyz	vf25,vf5
	vftoi12.xyz	vf26,vf6
	vftoi12.xyz	vf27,vf7

	qmfc2		$8,vf24
	qmfc2		$9,vf25
	qmfc2		$10,vf26
	qmfc2		$11,vf27

	ppach		$8,$9,$8
	ppach		$10,$11,$10

	sq		$8,0x00(%0)
	sq		$10,0x10(%0)
	" : : "r"(sv) : "memory","$8","$9","$10","$11" );
    }

    ScratchpadFree2();
}

#elif 0

/* 一つの頂点に複数のマトリクス演算を行なう。*/
/* エンベロープを考慮した場合、下のような式になることがわかったので、
   仕様を一部変更した。インターフェイスに変更は無い。*/
/*
  r = sm^(-1) ( p * ( sm + m0 + m1 + m2 + ... ) + (1-p) * ( psm + m0' + m1' + m2' + ... ) ) a
    = sm^(-1) ( p * sm * ( I + m0'' + m1'' + m2'' + ... )
                + (1-p) * psm * ( I + m0'' + m1'' + m2'' + ... ) ) a
    = sm^(-1) ( psm + p * ( sm - psm ) ) ( I + m0'' + m1'' + m2'' + ... ) a
    = ( sm^(-1) psm + p * sm^(-1) ( sm -  psm ) ) ( I + m0'' + m1'' + m2'' + ... ) a
 */
static void MaltiMatrix(FVECTOR *a,float *p,int size,FMATRIX *sm,FMATRIX *psm,
			FVECTOR *tv,FMATRIX *m,int msize)
{
    FMATRIX im;
    SVECTOR *sv=(SVECTOR *)SCRPAD_ADDR;
    int i,j;


#if 0
    InverseMatrix(&im,sm);
#elif 0
    sceVu0InversMatrix(*(sceVu0FMATRIX *)&im,*(sceVu0FMATRIX *)sm);
#else
    FastInverseMatrix(&im,sm);
#endif


    if(psm!=NULL){
	asm volatile ("
	lqc2		vf1,0x00(%1)

	lqc2		vf12,0x00(%3)
	lqc2		vf13,0x10(%3)
	lqc2		vf14,0x20(%3)
	lqc2		vf15,0x30(%3)
	lqc2		vf8,0x00(%0)
	lqc2		vf9,0x10(%0)
	lqc2		vf10,0x20(%0)
	lqc2		vf11,0x30(%0)
	lqc2		vf4,0x00(%2)
	lqc2		vf5,0x10(%2)
	lqc2		vf6,0x20(%2)
	lqc2		vf7,0x30(%2)

	vmulax.xyzw	ACC,vf8,vf1x
	vmadday.xyzw	ACC,vf9,vf1y
	vmaddaz.xyzw	ACC,vf10,vf1z
	vmaddw.xyzw	vf11,vf11,vf1w
	vsub.xyzw	vf12,vf12,vf8
	vsub.xyzw	vf13,vf13,vf9
	vsub.xyzw	vf14,vf14,vf10
	vsub.xyzw	vf15,vf15,vf11

	vmulax.xyzw	ACC,vf4,vf8x
	vmadday.xyzw	ACC,vf5,vf8y
	vmaddaz.xyzw	ACC,vf6,vf8z
	vmaddw.xyzw	vf8,vf7,vf8w
	vmulax.xyzw	ACC,vf4,vf9x
	vmadday.xyzw	ACC,vf5,vf9y
	vmaddaz.xyzw	ACC,vf6,vf9z
	vmaddw.xyzw	vf9,vf7,vf9w
	vmulax.xyzw	ACC,vf4,vf10x
	vmadday.xyzw	ACC,vf5,vf10y
	vmaddaz.xyzw	ACC,vf6,vf10z
	vmaddw.xyzw	vf10,vf7,vf10w
	vmulax.xyzw	ACC,vf4,vf11x
	vmadday.xyzw	ACC,vf5,vf11y
	vmaddaz.xyzw	ACC,vf6,vf11z
	vmaddw.xyzw	vf11,vf7,vf11w

	vmulax.xyzw	ACC,vf4,vf12x
	vmadday.xyzw	ACC,vf5,vf12y
	vmaddaz.xyzw	ACC,vf6,vf12z
	vmaddw.xyzw	vf12,vf7,vf12w
	vmulax.xyzw	ACC,vf4,vf13x
	vmadday.xyzw	ACC,vf5,vf13y
	vmaddaz.xyzw	ACC,vf6,vf13z
	vmaddw.xyzw	vf13,vf7,vf13w
	vmulax.xyzw	ACC,vf4,vf14x
	vmadday.xyzw	ACC,vf5,vf14y
	vmaddaz.xyzw	ACC,vf6,vf14z
	vmaddw.xyzw	vf14,vf7,vf14w
	vmulax.xyzw	ACC,vf4,vf15x
	vmadday.xyzw	ACC,vf5,vf15y
	vmaddaz.xyzw	ACC,vf6,vf15z
	vmaddw.xyzw	vf15,vf7,vf15w
	" : : "r"(psm),"r"(tv),"r"(&im),"r"(sm) );
    }
    else{
	asm volatile ("
	lqc2		vf12,0x00(%0)
	lqc2		vf13,0x10(%0)
	lqc2		vf14,0x20(%0)
	lqc2		vf15,0x30(%0)
	vmulx.xyzw	vf8,vf0,vf0x
	vmulx.xyzw	vf9,vf0,vf0x
	vmulx.xyzw	vf10,vf0,vf0x
	vmulx.xyzw	vf11,vf0,vf0x
	" : : "r"(&DG_UnitMatrix) );
    }

    /* 頂点を4つづつ計算し、結果をスクラッチに格納 */
    for(i=0;i<size;i+=4,a+=4,sv+=4){
	static const FVECTOR cv={ 1.0f,1.0f,1.0f,1.0f, };

	/* 四捨五入用 */
	static const FVECTOR dt={ 0.5f,0.5f,0.5f,0.0f, };

	asm volatile ("
	lqc2		vf31,0x00(%0)
	lqc2		vf20,0x00(%1)
	lqc2		vf21,0x10(%1)
	lqc2		vf22,0x20(%1)
	lqc2		vf23,0x30(%1)
#if 1
	vmulx.xyzw	vf4,vf0,vf0x
	vmulx.xyzw	vf5,vf0,vf0x
	vmulx.xyzw	vf6,vf0,vf0x
	vmulx.xyzw	vf7,vf0,vf0x
#else
	lqc2		vf4,0x00(%2)
	lqc2		vf5,0x00(%2)
	lqc2		vf6,0x00(%2)
	lqc2		vf7,0x00(%2)
#endif
	" : : "r"(&cv),"r"(a),"r"(&dt) );

	for(j=0;j<msize;j++,p+=4){
	    asm volatile ("
	    lqc2	vf16,0x00(%0)
	    lqc2	vf17,0x10(%0)
	    lqc2	vf18,0x20(%0)
	    lqc2	vf19,0x30(%0)
	    lqc2	vf30,0x00(%1)

	    vmulax.xyzw		ACC,vf16,vf20x
	    vmadday.xyzw	ACC,vf17,vf20y
	    vmaddaz.xyzw	ACC,vf18,vf20z
	    vmaddw.xyzw		vf24,vf19,vf0w
	    vmulax.xyzw		ACC,vf16,vf21x
	    vmadday.xyzw	ACC,vf17,vf21y
	    vmaddaz.xyzw	ACC,vf18,vf21z
	    vmaddw.xyzw		vf25,vf19,vf0w
	    vmulax.xyzw		ACC,vf16,vf22x
	    vmadday.xyzw	ACC,vf17,vf22y
	    vmaddaz.xyzw	ACC,vf18,vf22z
	    vmaddw.xyzw		vf26,vf19,vf0w
	    vmulax.xyzw		ACC,vf16,vf23x
	    vmadday.xyzw	ACC,vf17,vf23y
	    vmaddaz.xyzw	ACC,vf18,vf23z
	    vmaddw.xyzw		vf27,vf19,vf0w

	    pref	0,0x10(%1)

	    vmulx.xyzw		vf24,vf24,vf30x
	    vmuly.xyzw		vf25,vf25,vf30y
	    vmulz.xyzw		vf26,vf26,vf30z
	    vmulw.xyzw		vf27,vf27,vf30w

	    vadd.xyz		vf4,vf4,vf24
	    vadd.xyz		vf5,vf5,vf25
	    vadd.xyz		vf6,vf6,vf26
	    vadd.xyz		vf7,vf7,vf27

	    vsub.xyzw		vf31,vf31,vf30
	    " : : "r"(m+j),"r"(p) );
	}

	asm volatile ("
	vmulx.xyz	vf24,vf20,vf31x
	vmuly.xyz	vf25,vf21,vf31y
	vmulz.xyz	vf26,vf22,vf31z
	vmulw.xyz	vf27,vf23,vf31w
	vadd.xyz	vf4,vf4,vf24
	vadd.xyz	vf5,vf5,vf25
	vadd.xyz	vf6,vf6,vf26
	vadd.xyz	vf7,vf7,vf27

	vmulax.xyzw	ACC,vf8,vf4x
	vmadday.xyzw	ACC,vf9,vf4y
	vmaddaz.xyzw	ACC,vf10,vf4z
	vmaddw.xyzw	vf24,vf11,vf0w
	vmulax.xyzw	ACC,vf8,vf5x
	vmadday.xyzw	ACC,vf9,vf5y
	vmaddaz.xyzw	ACC,vf10,vf5z
	vmaddw.xyzw	vf25,vf11,vf0w
	vmulax.xyzw	ACC,vf8,vf6x
	vmadday.xyzw	ACC,vf9,vf6y
	vmaddaz.xyzw	ACC,vf10,vf6z
	vmaddw.xyzw	vf26,vf11,vf0w
	vmulax.xyzw	ACC,vf8,vf7x
	vmadday.xyzw	ACC,vf9,vf7y
	vmaddaz.xyzw	ACC,vf10,vf7z
	vmaddw.xyzw	vf27,vf11,vf0w

	vmulax.xyzw	ACC,vf12,vf4x
	vmadday.xyzw	ACC,vf13,vf4y
	vmaddaz.xyzw	ACC,vf14,vf4z
	vmaddw.xyzw	vf4,vf15,vf0w
	vmulax.xyzw	ACC,vf12,vf5x
	vmadday.xyzw	ACC,vf13,vf5y
	vmaddaz.xyzw	ACC,vf14,vf5z
	vmaddw.xyzw	vf5,vf15,vf0w
	vmulax.xyzw	ACC,vf12,vf6x
	vmadday.xyzw	ACC,vf13,vf6y
	vmaddaz.xyzw	ACC,vf14,vf6z
	vmaddw.xyzw	vf6,vf15,vf0w
	vmulax.xyzw	ACC,vf12,vf7x
	vmadday.xyzw	ACC,vf13,vf7y
	vmaddaz.xyzw	ACC,vf14,vf7z
	vmaddw.xyzw	vf7,vf15,vf0w

	lqc2		vf31,0x00(%1)

	vmulw.xyzw	vf4,vf4,vf20w
	vmulw.xyzw	vf5,vf5,vf21w
	vmulw.xyzw	vf6,vf6,vf22w
	vmulw.xyzw	vf7,vf7,vf23w
	vadd.xyz	vf4,vf4,vf24
	vadd.xyz	vf5,vf5,vf25
	vadd.xyz	vf6,vf6,vf26
	vadd.xyz	vf7,vf7,vf27
	vadd.xyz	vf4,vf4,vf31
	vadd.xyz	vf5,vf5,vf31
	vadd.xyz	vf6,vf6,vf31
	vadd.xyz	vf7,vf7,vf31

	vftoi0.xyz	vf24,vf4
	vftoi0.xyz	vf25,vf5
	vftoi0.xyz	vf26,vf6
	vftoi0.xyz	vf27,vf7
	vftoi12.w	vf24,vf0
	vftoi12.w	vf25,vf0
	vftoi12.w	vf26,vf0
	vftoi12.w	vf27,vf0

	qmfc2		$8,vf24
	qmfc2		$9,vf25
	qmfc2		$10,vf26
	qmfc2		$11,vf27

	ppach		$8,$9,$8
	ppach		$10,$11,$10

	sq		$8,0x00(%0)
	sq		$10,0x10(%0)
	" : : "r"(sv),"r"(&dt),"r"(&cv) : "memory","$8","$9","$10","$11" );
    }
}

/* 一つの法線に複数のマトリクス演算を行なう。*/
static void MaltiMatrixN(FVECTOR *a,float *p,int size,FMATRIX *sm,FMATRIX *psm,
			 FMATRIX *m,int msize)
{
    FMATRIX im;
    SVECTOR *sv=(SVECTOR *)SCRPAD_ADDR;
    int i,j;

#if 0
    InverseMatrix(&im,sm);
#elif 0
    sceVu0InversMatrix(*(sceVu0FMATRIX *)&im,*(sceVu0FMATRIX *)sm);
#else
    FastInverseMatrix(&im,sm);
#endif


    if(psm!=NULL){
	asm volatile ("
	lqc2		vf12,0x00(%2)
	lqc2		vf13,0x10(%2)
	lqc2		vf14,0x20(%2)
	lqc2		vf8,0x00(%0)
	lqc2		vf9,0x10(%0)
	lqc2		vf10,0x20(%0)
	lqc2		vf4,0x00(%1)
	lqc2		vf5,0x10(%1)
	lqc2		vf6,0x20(%1)

	vmulax.xyzw	ACC,vf4,vf8x
	vmadday.xyzw	ACC,vf5,vf8y
	vmaddz.xyzw	vf8,vf6,vf8z
	vmulax.xyzw	ACC,vf4,vf9x
	vmadday.xyzw	ACC,vf5,vf9y
	vmaddz.xyzw	vf9,vf6,vf9z
	vmulax.xyzw	ACC,vf4,vf10x
	vmadday.xyzw	ACC,vf5,vf10y
	vmaddz.xyzw	vf10,vf6,vf10z

	vsub.xyzw	vf12,vf12,vf8
	vsub.xyzw	vf13,vf13,vf9
	vsub.xyzw	vf14,vf14,vf10
	" : : "r"(psm),"r"(&im),"r"(&DG_UnitMatrix) );
    }
    else{
	asm volatile ("
	lqc2		vf12,0x00(%0)
	lqc2		vf13,0x10(%0)
	lqc2		vf14,0x20(%0)
	vmulx.xyzw	vf8,vf0,vf0x
	vmulx.xyzw	vf9,vf0,vf0x
	vmulx.xyzw	vf10,vf0,vf0x
	" : : "r"(&DG_UnitMatrix) );
    }

    /* 法線を4つづつ計算し、結果をスクラッチに格納 */
    for(i=0;i<size;i+=4,a+=4,sv+=4){
	static const FVECTOR cv={ 1.0f,1.0f,1.0f,1.0f, };

	/* 法線なので回転のみ計算し、移動は計算しない */
	asm volatile ("
	lqc2		vf31,0x00(%0)
	lqc2		vf20,0x00(%1)
	lqc2		vf21,0x10(%1)
	lqc2		vf22,0x20(%1)
	lqc2		vf23,0x30(%1)
	vmulx.xyzw	vf4,vf0,vf0x
	vmulx.xyzw	vf5,vf0,vf0x
	vmulx.xyzw	vf6,vf0,vf0x
	vmulx.xyzw	vf7,vf0,vf0x
	" : : "r"(&cv),"r"(a) );

	for(j=0;j<msize;j++,p+=4){
	    asm volatile ("
	    lqc2	vf16,0x00(%0)
	    lqc2	vf17,0x10(%0)
	    lqc2	vf18,0x20(%0)
	    lqc2	vf30,0x00(%1)

	    vmulax.xyzw		ACC,vf16,vf20x
	    vmadday.xyzw	ACC,vf17,vf20y
	    vmaddz.xyzw		vf24,vf18,vf20z
	    vmulax.xyzw		ACC,vf16,vf21x
	    vmadday.xyzw	ACC,vf17,vf21y
	    vmaddz.xyzw		vf25,vf18,vf21z
	    vmulax.xyzw		ACC,vf16,vf22x
	    vmadday.xyzw	ACC,vf17,vf22y
	    vmaddz.xyzw		vf26,vf18,vf22z
	    vmulax.xyzw		ACC,vf16,vf23x
	    vmadday.xyzw	ACC,vf17,vf23y
	    vmaddz.xyzw		vf27,vf18,vf23z

	    pref	0,0x10(%1)

	    vmulx.xyzw		vf24,vf24,vf30x
	    vmuly.xyzw		vf25,vf25,vf30y
	    vmulz.xyzw		vf26,vf26,vf30z
	    vmulw.xyzw		vf27,vf27,vf30w

	    vadd.xyz		vf4,vf4,vf24
	    vadd.xyz		vf5,vf5,vf25
	    vadd.xyz		vf6,vf6,vf26
	    vadd.xyz		vf7,vf7,vf27

	    vsub.xyzw		vf31,vf31,vf30
	    " : : "r"(m+j),"r"(p) );
	}

	asm volatile ("
	vmulx.xyz	vf24,vf20,vf31x
	vmuly.xyz	vf25,vf21,vf31y
	vmulz.xyz	vf26,vf22,vf31z
	vmulw.xyz	vf27,vf23,vf31w
	vadd.xyz	vf4,vf4,vf24
	vadd.xyz	vf5,vf5,vf25
	vadd.xyz	vf6,vf6,vf26
	vadd.xyz	vf7,vf7,vf27

	vmulax.xyzw	ACC,vf8,vf4x
	vmadday.xyzw	ACC,vf9,vf4y
	vmaddz.xyzw	vf24,vf10,vf4z
	vmulax.xyzw	ACC,vf8,vf5x
	vmadday.xyzw	ACC,vf9,vf5y
	vmaddz.xyzw	vf25,vf10,vf5z
	vmulax.xyzw	ACC,vf8,vf6x
	vmadday.xyzw	ACC,vf9,vf6y
	vmaddz.xyzw	vf26,vf10,vf6z
	vmulax.xyzw	ACC,vf8,vf7x
	vmadday.xyzw	ACC,vf9,vf7y
	vmaddz.xyzw	vf27,vf10,vf7z

	vmulax.xyzw	ACC,vf12,vf4x
	vmadday.xyzw	ACC,vf13,vf4y
	vmaddz.xyzw	vf4,vf14,vf4z
	vmulax.xyzw	ACC,vf12,vf5x
	vmadday.xyzw	ACC,vf13,vf5y
	vmaddz.xyzw	vf5,vf14,vf5z
	vmulax.xyzw	ACC,vf12,vf6x
	vmadday.xyzw	ACC,vf13,vf6y
	vmaddz.xyzw	vf6,vf14,vf6z
	vmulax.xyzw	ACC,vf12,vf7x
	vmadday.xyzw	ACC,vf13,vf7y
	vmaddz.xyzw	vf7,vf14,vf7z

	vmulw.xyzw	vf4,vf4,vf20w
	vmulw.xyzw	vf5,vf5,vf21w
	vmulw.xyzw	vf6,vf6,vf22w
	vmulw.xyzw	vf7,vf7,vf23w
	vadd.xyz	vf4,vf4,vf24
	vadd.xyz	vf5,vf5,vf25
	vadd.xyz	vf6,vf6,vf26
	vadd.xyz	vf7,vf7,vf27

	vmul.xyz	vf24,vf4,vf4
	vmul.xyz	vf25,vf5,vf5
	vmul.xyz	vf26,vf6,vf6
	vmul.xyz	vf27,vf7,vf7
	vmulax.w	ACC,vf0,vf24x
	vmadday.w	ACC,vf0,vf24y
	vmaddz.w	vf24,vf0,vf24z
	vmulax.w	ACC,vf0,vf25x
	vmadday.w	ACC,vf0,vf25y
	vmaddz.w	vf25,vf0,vf25z
	vrsqrt		Q,vf0w,vf24w
	vmulax.w	ACC,vf0,vf26x
	vmadday.w	ACC,vf0,vf26y
	vmaddz.w	vf26,vf0,vf26z
	vwaitq
	vmulq.xyz	vf4,vf4,Q
	vrsqrt		Q,vf0w,vf25w
	vmulax.w	ACC,vf0,vf27x
	vmadday.w	ACC,vf0,vf27y
	vmaddz.w	vf27,vf0,vf27z
	vwaitq
	vmulq.xyz	vf5,vf5,Q
	vrsqrt		Q,vf0w,vf26w
	vwaitq
	vmulq.xyz	vf6,vf6,Q
	vrsqrt		Q,vf0w,vf27w
	vwaitq
	vmulq.xyz	vf7,vf7,Q

	vftoi12.xyz	vf24,vf4
	vftoi12.xyz	vf25,vf5
	vftoi12.xyz	vf26,vf6
	vftoi12.xyz	vf27,vf7

	qmfc2		$8,vf24
	qmfc2		$9,vf25
	qmfc2		$10,vf26
	qmfc2		$11,vf27

	ppach		$8,$9,$8
	ppach		$10,$11,$10

	sq		$8,0x00(%0)
	sq		$10,0x10(%0)
	" : : "r"(sv) : "memory","$8","$9","$10","$11" );
    }

    ScratchpadFree2();
}

#else

/* 一つの頂点に複数のマトリクス演算を行なう。*/
/* エンベロープ演算は、システム側で行なうように変更 */
/*
  r = sm^(-1) ( p * ( sm + m0 + m1 + m2 + ... ) + (1-p) * ( psm + m0' + m1' + m2' + ... ) ) a
    = sm^(-1) ( p * sm * ( I + m0'' + m1'' + m2'' + ... )
                + (1-p) * psm * ( I + m0'' + m1'' + m2'' + ... ) ) a
    = sm^(-1) ( psm + p * ( sm - psm ) ) ( I + m0'' + m1'' + m2'' + ... ) a
    = ( system calc ) ( I + m0'' + m1'' + m2'' + ... ) a
 */
static void MaltiMatrix(FVECTOR *r,FVECTOR *a,float *p,int size,FMATRIX *m,int msize)
{
    int i,j;

    /* 頂点を4つづつ計算し、結果をスクラッチに格納 */
    for(i=0;i<size;i+=4,a+=4,r+=4){
	static const FVECTOR cv={ 1.0f,1.0f,1.0f,1.0f, };

	asm volatile ("
	lqc2		vf31,0x00(%0)
	lqc2		vf20,0x00(%1)
	lqc2		vf21,0x10(%1)
	lqc2		vf22,0x20(%1)
	lqc2		vf23,0x30(%1)

	vmulx.xyzw	vf29,vf0,vf0x

	vmulx.xyzw	vf4,vf0,vf0x
	vmulx.xyzw	vf5,vf0,vf0x
	vmulx.xyzw	vf6,vf0,vf0x
	vmulx.xyzw	vf7,vf0,vf0x
	" : : "r"(&cv),"r"(a) );

	for(j=0;j<msize;j++,p+=4){
	    asm volatile ("
	    lqc2	vf16,0x00(%0)
	    lqc2	vf17,0x10(%0)
	    lqc2	vf18,0x20(%0)
	    lqc2	vf19,0x30(%0)
	    lqc2	vf30,0x00(%1)

	    vmulax.xyzw		ACC,vf16,vf20x
	    vmadday.xyzw	ACC,vf17,vf20y
	    vmaddaz.xyzw	ACC,vf18,vf20z
	    vmaddw.xyzw		vf24,vf19,vf0w
	    vmulax.xyzw		ACC,vf16,vf21x
	    vmadday.xyzw	ACC,vf17,vf21y
	    vmaddaz.xyzw	ACC,vf18,vf21z
	    vmaddw.xyzw		vf25,vf19,vf0w
	    vmulax.xyzw		ACC,vf16,vf22x
	    vmadday.xyzw	ACC,vf17,vf22y
	    vmaddaz.xyzw	ACC,vf18,vf22z
	    vmaddw.xyzw		vf26,vf19,vf0w
	    vmulax.xyzw		ACC,vf16,vf23x
	    vmadday.xyzw	ACC,vf17,vf23y
	    vmaddaz.xyzw	ACC,vf18,vf23z
	    vmaddw.xyzw		vf27,vf19,vf0w

	    pref	0,0x10(%1)

	    vadd.xyzw		vf29,vf29,vf30

	    vaddax.xyz		ACC,vf4,vf0x
	    vmaddx.xyz		vf4,vf24,vf30x
	    vaddax.xyz		ACC,vf5,vf0x
	    vmaddy.xyz		vf5,vf25,vf30y
	    vaddax.xyz		ACC,vf6,vf0x
	    vmaddz.xyz		vf6,vf26,vf30z
	    vaddax.xyz		ACC,vf7,vf0x
	    vmaddw.xyz		vf7,vf27,vf30w
	    " : : "r"(m+j),"r"(p) );
	}

	asm volatile ("
	vsub.xyzw	vf31,vf31,vf29

	vaddax.xyz	ACC,vf4,vf0x
	vmaddx.xyz	vf4,vf20,vf31x
	vaddax.xyz	ACC,vf5,vf0x
	vmaddy.xyz	vf5,vf21,vf31y
	vaddax.xyz	ACC,vf6,vf0x
	vmaddz.xyz	vf6,vf22,vf31z
	vaddax.xyz	ACC,vf7,vf0x
	vmaddw.xyz	vf7,vf23,vf31w

	sqc2		vf4,0x00(%0)
	sqc2		vf5,0x10(%0)
	sqc2		vf6,0x20(%0)
	sqc2		vf7,0x30(%0)
	" : : "r"(r) : "memory");
    }
}

/* 一つの法線に複数のマトリクス演算を行なう。*/
static void MaltiMatrixN(FVECTOR *r,FVECTOR *a,float *p,int size,FMATRIX *m,int msize)
{
    int i,j;

    /* 法線を4つづつ計算し、結果をスクラッチに格納 */
    for(i=0;i<size;i+=4,a+=4,r+=4){
	static const FVECTOR cv={ 1.0f,1.0f,1.0f,1.0f, };

	/* 法線なので回転のみ計算し、移動は計算しない */
	asm volatile ("
	lqc2		vf31,0x00(%0)
	lqc2		vf20,0x00(%1)
	lqc2		vf21,0x10(%1)
	lqc2		vf22,0x20(%1)
	lqc2		vf23,0x30(%1)

	vmulx.xyzw	vf29,vf0,vf0x

	vmulx.xyzw	vf4,vf0,vf0x
	vmulx.xyzw	vf5,vf0,vf0x
	vmulx.xyzw	vf6,vf0,vf0x
	vmulx.xyzw	vf7,vf0,vf0x
	" : : "r"(&cv),"r"(a) );

	for(j=0;j<msize;j++,p+=4){
	    asm volatile ("
	    lqc2	vf16,0x00(%0)
	    lqc2	vf17,0x10(%0)
	    lqc2	vf18,0x20(%0)
	    lqc2	vf30,0x00(%1)

	    vmulax.xyzw		ACC,vf16,vf20x
	    vmadday.xyzw	ACC,vf17,vf20y
	    vmaddz.xyzw		vf24,vf18,vf20z
	    vmulax.xyzw		ACC,vf16,vf21x
	    vmadday.xyzw	ACC,vf17,vf21y
	    vmaddz.xyzw		vf25,vf18,vf21z
	    vmulax.xyzw		ACC,vf16,vf22x
	    vmadday.xyzw	ACC,vf17,vf22y
	    vmaddz.xyzw		vf26,vf18,vf22z
	    vmulax.xyzw		ACC,vf16,vf23x
	    vmadday.xyzw	ACC,vf17,vf23y
	    vmaddz.xyzw		vf27,vf18,vf23z

	    pref	0,0x10(%1)

	    vadd.xyzw		vf29,vf29,vf30

	    vmulx.xyzw		vf24,vf24,vf30x
	    vmuly.xyzw		vf25,vf25,vf30y
	    vmulz.xyzw		vf26,vf26,vf30z
	    vmulw.xyzw		vf27,vf27,vf30w

	    vadd.xyz		vf4,vf4,vf24
	    vadd.xyz		vf5,vf5,vf25
	    vadd.xyz		vf6,vf6,vf26
	    vadd.xyz		vf7,vf7,vf27
	    " : : "r"(m+j),"r"(p) );
	}

	asm volatile ("
	vsub.xyzw	vf31,vf31,vf29

	vmulx.xyz	vf24,vf20,vf31x
	vmuly.xyz	vf25,vf21,vf31y
	vmulz.xyz	vf26,vf22,vf31z
	vmulw.xyz	vf27,vf23,vf31w
	vadd.xyz	vf4,vf4,vf24
	vadd.xyz	vf5,vf5,vf25
	vadd.xyz	vf6,vf6,vf26
	vadd.xyz	vf7,vf7,vf27

	sqc2		vf4,0x00(%0)
	sqc2		vf5,0x10(%0)
	sqc2		vf6,0x20(%0)
	sqc2		vf7,0x30(%0)
	" : : "r"(r) : "memory");
    }
}

#endif


#endif /* KP_XBOX ************** PS2用の専用関数(オリジナル)ここまで******************  T.Morita 2002.02.04 */


MALTI_MATRIX_WORK *InitMaltiMatrix(DG_OBJS *objs,CV2_DEF *def,int objnum,int msize,int flag)
{
    MALTI_MATRIX_WORK *work;
    CV2_MDL *mdl=&(def->models[objnum]);
    DG_OBJ *obj=&(objs->objs[objnum]);
    short *indexes;
    int size;
    int vsize=0;
    int i;

    if((work=(MALTI_MATRIX_WORK *)GV_Malloc(sizeof(MALTI_MATRIX_WORK)))==NULL) return NULL;

    if(flag & DG_VANIME_VERTS){
	if(mdl->n_verts>vsize) vsize=mdl->n_verts;
    }
    if(flag & DG_VANIME_NORMS){
	if(mdl->n_norms>vsize) vsize=mdl->n_norms;
    }
    if(flag & DG_VANIME_UVS){
	if(mdl->n_uvs>vsize) vsize=mdl->n_uvs;
    }

    vsize=(vsize+3) & ~3;

    size=sizeof(FMATRIX)*msize+sizeof(FVECTOR)*(mdl->n_verts+vsize)
	+sizeof(float)*msize*((mdl->n_verts+3) & ~3);

    if((work->m=(FMATRIX *)GV_Malloc(size))==NULL){
	GV_Free(work);
	return NULL;
    }
    GV_ZeroMemory(work->m,size);

    work->v_normal=(FVECTOR *)(work->m+msize);
    work->tmp_work=(FVECTOR *)(work->v_normal+mdl->n_verts);
    work->p=(float *)(work->tmp_work+vsize);

    /* 頂点数と法線数が一致しない時に備えて、頂点数分の法線を取り出す。*/
    indexes=mdl->verts_index;
    for(i=0;i<obj->n_packs;i++){
	DG_OBJPACK *pack=obj->model->packs+i;
	short *norms=pack->norms;
	int j;

	for(j=0;j<pack->n_verts;j++,norms+=4,indexes++){
	    /* 通常、法線はモデルの内を向いているので、外向きに変更する。*/

	    static const float p=-1.0f/4096.0f;

	    (work->v_normal+*indexes)->vx=(float)*(norms+0)*p;
	    (work->v_normal+*indexes)->vy=(float)*(norms+1)*p;
	    (work->v_normal+*indexes)->vz=(float)*(norms+2)*p;
	    (work->v_normal+*indexes)->vw=1.0f;
	}
	if(j & 1) indexes++;
    }

    /* 重み値を扱いやすくするために整列 */
    for(i=0;i<mdl->n_verts;i+=4){
	int j,k;
	for(j=0;j<msize;j++){
	    for(k=0;k<4;k++){
		static const float p=1.0f/4096.0f;
		float x;
#if 1
		if(i+k<mdl->n_verts){
		    x=(float)((mdl->vert_usrdata+i+k)->dWeight[j])*p;
		}
		else{
		    x=0.0f;
		}

		*(work->p+i*msize+j*4+k)=x;
#else
		*(work->p+i*msize+j*4+k)=0.0f;
#endif

	    }
	}
    }

    work->cvd=mdl;
    work->flag=flag;

    work->sm=&(obj->world);
    if(obj->model->parent>=0) work->psm=&(objs->objs[obj->model->parent].world);
    else work->psm=NULL;
    work->msize=msize;

    DG_MakeAnimVertsBuffer(&(work->anime),obj,flag);

    return work;
}

void ExitMaltiMatrix(MALTI_MATRIX_WORK *work)
{
    if(work->m!=NULL) GV_Free(work->m);
    DG_FreeAnimVertsBuffer(&(work->anime));
    GV_Free(work);
}

void MoveMaltiMatrix(MALTI_MATRIX_WORK *work)
{
#ifndef KP_WINDOWS
#if 0
    DG_OBJ *obj=work->anime.obj;
    FVECTOR tv;

    tv.vx=obj->model->tx;
    tv.vy=obj->model->ty;
    tv.vz=obj->model->tz;
    tv.vw=1.0f;
#endif

    DG_SwitchVAnimeBuffer(&(work->anime));

    if(work->flag & DG_VANIME_VERTS){
#if 0
	MaltiMatrix(work->cvd->verts,work->p,work->cvd->n_verts,work->sm,work->psm,
		    &tv,work->m,work->msize);
	DG_RefineStripVertex(&(work->anime),work->cvd->verts_index);
#else
	MaltiMatrix(work->tmp_work,work->cvd->verts,work->p,work->cvd->n_verts,
		    work->m,work->msize);
	DG_RegistCommonVertex2(work->tmp_work,work->cvd->n_verts);
	DG_RefineStripVertex2(&(work->anime),work->cvd->verts_index);
#endif
    }
    if(work->flag & DG_VANIME_NORMS){
#if 0
	MaltiMatrixN(work->v_normal,work->p,work->cvd->n_verts,work->sm,work->psm,
		     work->m,work->msize);
#else
	MaltiMatrixN(work->tmp_work,work->v_normal,work->p,work->cvd->n_verts,
		     work->m,work->msize);
	DG_RegistCommonNormal2(work->tmp_work,work->cvd->n_verts);
#endif
	DG_RefineStripNormal(&(work->anime),work->cvd->verts_index);
    }


#else	// Windows
    DG_SwitchVAnimeBuffer(&(work->anime));

	if(work->flag & DG_VANIME_VERTS){
		MaltiMatrix(work->tmp_work,work->cvd->verts,work->p,work->cvd->n_verts,
			    work->m,work->msize);
#if FALSE
		DG_RegistCommonVertex2(work->tmp_work,work->cvd->n_verts);
		DG_RefineStripVertex2(&(work->anime),work->cvd->verts_index);
#else
		DG_RegistCommonVertex2RefineStripVertex2(&(work->anime), work->tmp_work) ;
#endif
	}
	if(work->flag & DG_VANIME_NORMS){
		MaltiMatrixN(work->tmp_work,work->v_normal,work->p,work->cvd->n_verts,
			     work->m,work->msize);
#if FALSE
		DG_RegistCommonNormal2(work->tmp_work,work->cvd->n_verts);
		DG_RefineStripNormal(&(work->anime),work->cvd->verts_index);
#else
		DG_RegistCommonNormal2RefineStripNormal(&(work->anime), work->tmp_work) ;
#endif
	}
#endif
}
