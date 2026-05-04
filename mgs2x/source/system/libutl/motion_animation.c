//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	motion_animation.c
		頂点アニメーション

	1999/11/09 K.Kano
	$Id: motion_animation.c,v 1.4 2002/11/23 11:50:59 Yoshizawa1 Exp $
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


/* これは手におえん！！！ T.Morita 2002.02.01 */

#if 1//BP_ASM - XBOX version is in C - #ifdef KP_XBOX  /**************XBOX用の専用関数(書換え版)***********************  T.Morita 2002.02.01 */
/* 一つの頂点列を二つのマトリクスで演算する */

#pragma optimize( "", off )

static void MotionVAnimation(FVECTOR *r,FVECTOR *a,FVECTOR *n,int size,FMATRIX *sm,FMATRIX *dm,
							 FMATRIX *psm,FMATRIX *pdm,FVECTOR *tv)
{
    FMATRIX im;
    int i=size;
	FMATRIX lsm, lpsm, ldm, lpdm ;


    FastInverseMatrix(&im,sm);

    if(pdm!=NULL && psm!=NULL){
		lsm  = *sm  ;/*vf28-31*/
		lpsm = *psm ;/*vf20-23*/
		ldm  = *dm  ;/*vf24-27*/
		lpdm = *pdm ;/*vf16-19*/
		vu0_ApplyMatrix( (FVECTOR *)lpsm.m[3], &lpsm, tv ) ;
		vu0_ApplyMatrix( (FVECTOR *)lpdm.m[3], &lpdm, tv ) ;
    }
    else{
		lsm  = *sm ;/*vf28-31*/
		lpsm = *sm ;/*vf20-23*/
		ldm  = *dm ;/*vf24-27*/
		lpdm = *dm ;/*vf16-19*/
    }

    {
		int j ;
		static const FVECTOR c={ 0.0f,0.0f,0.0f,0.5f, };

		for ( j=4 ; --j>=0 ; ) {
			_sceVu0SubVector( (FVECTOR *)lsm.m[j],
							 (FVECTOR *)lsm.m[j],
							 (FVECTOR *)lpsm.m[j] ) ;
			_sceVu0SubVector( (FVECTOR *)ldm.m[j],
							 (FVECTOR *)ldm.m[j],
							 (FVECTOR *)lpdm.m[j] ) ;
	    }
		for ( j=4 ; --j>=0 ; ) {
			_sceVu0SubVector( (FVECTOR *)lpsm.m[j],
							 (FVECTOR *)lpsm.m[j],
							 (FVECTOR *)lpdm.m[j] ) ;
			_sceVu0SubVector( (FVECTOR *)lsm.m[j],
							 (FVECTOR *)lsm.m[j],
							 (FVECTOR *)ldm.m[j] ) ;
	    }

		for ( j=4 ; --j>=0 ; ) {
			vu0_ApplyMatrix( (FVECTOR *)lpdm.m[j], &im, (FVECTOR *)lpdm.m[j] ) ;
			vu0_ApplyMatrix( (FVECTOR *)lpsm.m[j], &im, (FVECTOR *)lpsm.m[j] ) ;
			vu0_ApplyMatrix( (FVECTOR *)ldm.m[j] , &im, (FVECTOR *)ldm.m[j]  ) ;
			vu0_ApplyMatrix( (FVECTOR *)lsm.m[j] , &im, (FVECTOR *)lsm.m[j]  ) ;
		}
    }
    while(i>0){
		/* vf4w = 0.5f */
		FVECTOR vf15, vf14, vf13, vf12, vf6, vf7, vf8 ;
		float Q ;

		vu0_ApplyMatrixXYZ( &vf13, &lpsm, a ) ;
		vu0_ApplyMatrixXYZ( &vf15, &lsm , a ) ;

		vf8.vx = vf13.vx + vf15.vx * a->vw ;
		vf8.vy = vf13.vy + vf15.vy * a->vw ;
		vf8.vz = vf13.vz + vf15.vz * a->vw ;
		vf7.vx = vf8.vx * vf8.vx ;
		vf7.vy = vf8.vy * vf8.vy ;
		vf7.vz = vf8.vz * vf8.vz ;
		vf7.vw = vf7.vx + vf7.vy + vf7.vz ;
		vf6.vx = vf8.vx * n->vx ;
		vf6.vy = vf8.vy * n->vy ;
		vf6.vz = vf8.vz * n->vz ;
		vf6.vw = vf6.vx + vf6.vy + vf6.vz ;
		Q = vf6.vw / bp_sqrtf( vf7.vw ) ;   //BP_MATH - emulate PS2 sqrtf

		vu0_ApplyMatrixXYZ( &vf12, &lpdm, a ) ;
		vu0_ApplyMatrixXYZ( &vf14, &ldm , a ) ;

		vf6.vw = 0.5f*(1.0f + Q) ;

		fpu_MulVectorScaler( &vf15, &vf15, a->vw  ) ;
		fpu_MulVectorScaler( &vf13, &vf13, vf6.vw ) ;
		fpu_MulVectorScaler( &vf14, &vf14, a->vw  ) ;
		fpu_MulVectorScaler( &vf15, &vf15, vf6.vw ) ;
		fpu_AddVectors( &vf13, &vf13, &vf12 ) ;
		fpu_AddVectors( &vf15, &vf15, &vf14 ) ;
		fpu_AddVectors( r    , &vf13, &vf15 ) ;
		r->vw = 1.0f ;

		a++; n++; i--; r++;
    }


}
#pragma optimize( "", on )

/* 法線用。行列計算から移動ベクトルを抜いたもの。*/
static void MotionVAnimationN(FVECTOR *r,FVECTOR *a,FVECTOR *n,int size,FMATRIX *sm,FMATRIX *dm,
							  FMATRIX *psm,FMATRIX *pdm)
{

    FMATRIX im;
    int i=size;
    static const FVECTOR v={ 0.0f,0.0f,0.0f,1.0f, };
	FMATRIX lsm, lpsm, ldm, lpdm ;


    FastInverseMatrix(&im,sm);

    if(pdm!=NULL && psm!=NULL){
		lsm  = *sm  ;/*vf28-31*/
		lpsm = *psm ;/*vf20-23*/
		ldm  = *dm  ;/*vf24-27*/
		lpdm = *pdm ;/*vf16-19*/
    }
    else{
		lsm  = *sm ;/*vf28-31*/
		lpsm = *sm ;/*vf20-23*/
		ldm  = *dm ;/*vf24-27*/
		lpdm = *dm ;/*vf16-19*/
    }
	_sceVu0CopyVector( (FVECTOR *)lsm.m[3] , &v ) ;
	_sceVu0CopyVector( (FVECTOR *)lpsm.m[3], &v ) ;
	_sceVu0CopyVector( (FVECTOR *)ldm.m[3] , &v ) ;
	_sceVu0CopyVector( (FVECTOR *)lpdm.m[3], &v ) ;
	_sceVu0CopyVector( (FVECTOR *)im.m[3]  , &v ) ;
	
    {
		int j ;
		static const FVECTOR c={ 0.0f,0.0f,0.0f,0.5f, };

		for ( j=4 ; --j>=0 ; ) {
			_sceVu0SubVector( (FVECTOR *)lsm.m[j],
							 (FVECTOR *)lsm.m[j],
							 (FVECTOR *)lpsm.m[j] ) ;
			_sceVu0SubVector( (FVECTOR *)ldm.m[j],
							 (FVECTOR *)ldm.m[j],
							 (FVECTOR *)lpdm.m[j] ) ;
	    }
		for ( j=4 ; --j>=0 ; ) {
			_sceVu0SubVector( (FVECTOR *)lpsm.m[j],
							 (FVECTOR *)lpsm.m[j],
							 (FVECTOR *)lpdm.m[j] ) ;
			_sceVu0SubVector( (FVECTOR *)lsm.m[j],
							 (FVECTOR *)lsm.m[j],
							 (FVECTOR *)ldm.m[j] ) ;
	    }

		for ( j=4 ; --j>=0 ; ) {
			vu0_ApplyMatrix( (FVECTOR *)lpdm.m[j], &im, (FVECTOR *)lpdm.m[j] ) ;
			vu0_ApplyMatrix( (FVECTOR *)lpsm.m[j], &im, (FVECTOR *)lpsm.m[j] ) ;
			vu0_ApplyMatrix( (FVECTOR *)ldm.m[j] , &im, (FVECTOR *)ldm.m[j]  ) ;
			vu0_ApplyMatrix( (FVECTOR *)lsm.m[j] , &im, (FVECTOR *)lsm.m[j]  ) ;
		}
    }

    while(i>0){
		/* vf4w = 0.5f */
		FVECTOR vf15, vf14, vf13, vf12, vf6, vf7, vf8 ;
		float Q ;

		vu0_ApplyMatrixXYZ( &vf13, &lpsm, a ) ;
		vu0_ApplyMatrixXYZ( &vf15, &lsm , a ) ;

		vf8.vx = vf13.vx + vf15.vx * a->vw ;
		vf8.vy = vf13.vy + vf15.vy * a->vw ;
		vf8.vz = vf13.vz + vf15.vz * a->vw ;
		vf7.vx = vf8.vx * vf8.vx ;
		vf7.vy = vf8.vy * vf8.vy ;
		vf7.vz = vf8.vz * vf8.vz ;
		vf7.vw = vf7.vx + vf7.vy + vf7.vz ;
		vf6.vx = vf8.vx * n->vx ;
		vf6.vy = vf8.vy * n->vy ;
		vf6.vz = vf8.vz * n->vz ;
		vf6.vw = vf6.vx + vf6.vy + vf6.vz ;
		Q = vf6.vw / bp_sqrtf( vf7.vw ) ;   //BP_MATH - emulate PS2 sqrtf

		vu0_ApplyMatrixXYZ( &vf12, &lpdm, a ) ;
		vu0_ApplyMatrixXYZ( &vf14, &ldm , a ) ;

		vf6.vw = 0.5f*(1.0f + Q) ;

		fpu_MulVectorScaler( &vf15, &vf15, a->vw  ) ;
		fpu_MulVectorScaler( &vf13, &vf13, vf6.vw ) ;
		fpu_MulVectorScaler( &vf14, &vf14, a->vw  ) ;
		fpu_MulVectorScaler( &vf15, &vf15, vf6.vw ) ;
		fpu_AddVectors( &vf13, &vf13, &vf12 ) ;
		fpu_AddVectors( &vf15, &vf15, &vf14 ) ;
		fpu_AddVectors( r    , &vf13, &vf15 ) ;
		r->vw = 1.0f ;

		a++; n++; i--; r++;
    }


}

#else        /************** PS2用の専用関数(オリジナル)***********************  T.Morita 2002.02.01 */


/* 一つの頂点列を二つのマトリクスで演算する */
/*
  r = sm^(-1) ( s * ( p * sm + (1-p) * psm ) + (1-s) * ( p * dm + (1-p) * pdm ) ) a
  = sm^(-1) ( pdm + s * ( psm - pdm ) + p * ( dm - pdm ) + s * p * ( sm - psm - dm + pdm ) ) a
  s = ( sm^(-1) ( p * sm + (1-p) * psm ) - ( p * dm + (1-p) * pdm ) a ) . n
  = ( sm^(-1) ( ( psm + p * ( sm - psm ) ) - ( pdm + p * ( dm - pdm ) ) ) a ) . n
  = ( sm^(-1) ( psm - pdm + p * ( sm - psm - dm + pdm ) ) a ) . n
  */
static void MotionVAnimation(FVECTOR *r,FVECTOR *a,FVECTOR *n,int size,FMATRIX *sm,FMATRIX *dm,
							 FMATRIX *psm,FMATRIX *pdm,FVECTOR *tv)
{
    FMATRIX im;
    int i=size;

#if 0
    InverseMatrix(&im,sm);
#elif 0
    sceVu0InversMatrix(*(sceVu0FMATRIX *)&im,*(sceVu0FMATRIX *)sm);
#else
    FastInverseMatrix(&im,sm);
#endif

    if(pdm!=NULL && psm!=NULL){
		asm volatile ("
		lqc2		vf1,0x00(%5)

		lqc2		vf12,0x00(%0)
		lqc2		vf13,0x10(%0)
		lqc2		vf14,0x20(%0)
		lqc2		vf15,0x30(%0)

		lqc2		vf28,0x00(%1)
		lqc2		vf29,0x10(%1)
		lqc2		vf30,0x20(%1)
		lqc2		vf31,0x30(%1)
		lqc2		vf24,0x00(%2)
		lqc2		vf25,0x10(%2)
		lqc2		vf26,0x20(%2)
		lqc2		vf27,0x30(%2)
		lqc2		vf20,0x00(%3)
		lqc2		vf21,0x10(%3)
		lqc2		vf22,0x20(%3)
		lqc2		vf23,0x30(%3)
		lqc2		vf16,0x00(%4)
		lqc2		vf17,0x10(%4)
		lqc2		vf18,0x20(%4)
		lqc2		vf19,0x30(%4)

		vmulax.xyzw	ACC,vf20,vf1x
		vmadday.xyzw	ACC,vf21,vf1y
		vmaddaz.xyzw	ACC,vf22,vf1z
		vmaddw.xyzw	vf23,vf23,vf1w
		vmulax.xyzw	ACC,vf16,vf1x
		vmadday.xyzw	ACC,vf17,vf1y
		vmaddaz.xyzw	ACC,vf18,vf1z
		vmaddw.xyzw	vf19,vf19,vf1w
		" : : "r"(&im),"r"(sm),"r"(dm),"r"(psm),"r"(pdm),"r"(tv) );
    }
    else{
		asm volatile ("
		lqc2		vf12,0x00(%0)
		lqc2		vf13,0x10(%0)
		lqc2		vf14,0x20(%0)
		lqc2		vf15,0x30(%0)

		lqc2		vf28,0x00(%1)
		lqc2		vf29,0x10(%1)
		lqc2		vf30,0x20(%1)
		lqc2		vf31,0x30(%1)
		lqc2		vf24,0x00(%2)
		lqc2		vf25,0x10(%2)
		lqc2		vf26,0x20(%2)
		lqc2		vf27,0x30(%2)
		vmove.xyzw	vf20,vf28
		vmove.xyzw	vf21,vf29
		vmove.xyzw	vf22,vf30
		vmove.xyzw	vf23,vf31
		vmove.xyzw	vf16,vf24
		vmove.xyzw	vf17,vf25
		vmove.xyzw	vf18,vf26
		vmove.xyzw	vf19,vf27
		" : : "r"(&im),"r"(sm),"r"(dm) );
    }

    {
		static const FVECTOR c={ 0.0f,0.0f,0.0f,0.5f, };

		asm volatile ("
		vsub.xyzw	vf28,vf28,vf20
		vsub.xyzw	vf29,vf29,vf21
		vsub.xyzw	vf30,vf30,vf22
		vsub.xyzw	vf31,vf31,vf23

		vsub.xyzw	vf24,vf24,vf16
		vsub.xyzw	vf25,vf25,vf17
		vsub.xyzw	vf26,vf26,vf18
		vsub.xyzw	vf27,vf27,vf19

		vsub.xyzw	vf20,vf20,vf16
		vsub.xyzw	vf21,vf21,vf17
		vsub.xyzw	vf22,vf22,vf18
		vsub.xyzw	vf23,vf23,vf19

		vsub.xyzw	vf28,vf28,vf24
		vsub.xyzw	vf29,vf29,vf25
		vsub.xyzw	vf30,vf30,vf26
		vsub.xyzw	vf31,vf31,vf27

		vmulax.xyzw	ACC,vf12,vf16x
		vmadday.xyzw	ACC,vf13,vf16y
		vmaddaz.xyzw	ACC,vf14,vf16z
		vmaddw.xyzw	vf16,vf15,vf16w
		vmulax.xyzw	ACC,vf12,vf17x
		vmadday.xyzw	ACC,vf13,vf17y
		vmaddaz.xyzw	ACC,vf14,vf17z
		vmaddw.xyzw	vf17,vf15,vf17w
		vmulax.xyzw	ACC,vf12,vf18x
		vmadday.xyzw	ACC,vf13,vf18y
		vmaddaz.xyzw	ACC,vf14,vf18z
		vmaddw.xyzw	vf18,vf15,vf18w
		vmulax.xyzw	ACC,vf12,vf19x
		vmadday.xyzw	ACC,vf13,vf19y
		vmaddaz.xyzw	ACC,vf14,vf19z
		vmaddw.xyzw	vf19,vf15,vf19w

		vmulax.xyzw	ACC,vf12,vf20x
		vmadday.xyzw	ACC,vf13,vf20y
		vmaddaz.xyzw	ACC,vf14,vf20z
		vmaddw.xyzw	vf20,vf15,vf20w
		vmulax.xyzw	ACC,vf12,vf21x
		vmadday.xyzw	ACC,vf13,vf21y
		vmaddaz.xyzw	ACC,vf14,vf21z
		vmaddw.xyzw	vf21,vf15,vf21w
		vmulax.xyzw	ACC,vf12,vf22x
		vmadday.xyzw	ACC,vf13,vf22y
		vmaddaz.xyzw	ACC,vf14,vf22z
		vmaddw.xyzw	vf22,vf15,vf22w
		vmulax.xyzw	ACC,vf12,vf23x
		vmadday.xyzw	ACC,vf13,vf23y
		vmaddaz.xyzw	ACC,vf14,vf23z
		vmaddw.xyzw	vf23,vf15,vf23w

		vmulax.xyzw	ACC,vf12,vf24x
		vmadday.xyzw	ACC,vf13,vf24y
		vmaddaz.xyzw	ACC,vf14,vf24z
		vmaddw.xyzw	vf24,vf15,vf24w
		vmulax.xyzw	ACC,vf12,vf25x
		vmadday.xyzw	ACC,vf13,vf25y
		vmaddaz.xyzw	ACC,vf14,vf25z
		vmaddw.xyzw	vf25,vf15,vf25w
		vmulax.xyzw	ACC,vf12,vf26x
		vmadday.xyzw	ACC,vf13,vf26y
		vmaddaz.xyzw	ACC,vf14,vf26z
		vmaddw.xyzw	vf26,vf15,vf26w
		vmulax.xyzw	ACC,vf12,vf27x
		vmadday.xyzw	ACC,vf13,vf27y
		vmaddaz.xyzw	ACC,vf14,vf27z
		vmaddw.xyzw	vf27,vf15,vf27w

		lqc2		vf4,0x00(%0)

		vmulax.xyzw	ACC,vf12,vf28x
		vmadday.xyzw	ACC,vf13,vf28y
		vmaddaz.xyzw	ACC,vf14,vf28z
		vmaddw.xyzw	vf28,vf15,vf28w
		vmulax.xyzw	ACC,vf12,vf29x
		vmadday.xyzw	ACC,vf13,vf29y
		vmaddaz.xyzw	ACC,vf14,vf29z
		vmaddw.xyzw	vf29,vf15,vf29w
		vmulax.xyzw	ACC,vf12,vf30x
		vmadday.xyzw	ACC,vf13,vf30y
		vmaddaz.xyzw	ACC,vf14,vf30z
		vmaddw.xyzw	vf30,vf15,vf30w
		vmulax.xyzw	ACC,vf12,vf31x
		vmadday.xyzw	ACC,vf13,vf31y
		vmaddaz.xyzw	ACC,vf14,vf31z
		vmaddw.xyzw	vf31,vf15,vf31w
		" : : "r"(&c) );
    }

    while(i>0){

		/* vf4w = 0.5f */

		asm volatile ("
		lqc2		vf2,0(%0)
		lqc2		vf3,0(%1)
		pref		0,0x10(%0)

		vmulax.xyzw	ACC,vf28,vf2x
		vmadday.xyzw	ACC,vf29,vf2y
		vmaddaz.xyzw	ACC,vf30,vf2z
		vmaddw.xyzw	vf15,vf31,vf0w
		vmulax.xyzw	ACC,vf20,vf2x
		vmadday.xyzw	ACC,vf21,vf2y
		vmaddaz.xyzw	ACC,vf22,vf2z
		vmaddaw.xyzw	ACC,vf23,vf0w
		vmaddx.xyzw	vf13,vf0,vf0x

		vmaddw.xyz	vf8,vf15,vf2w

		vmul.xyz	vf7,vf8,vf8
		vmul.xyz	vf6,vf8,vf3

		vmulax.w	ACC,vf0,vf7x
		vmadday.w	ACC,vf0,vf7y
		vmaddz.w	vf7,vf0,vf7z
		vmulax.w	ACC,vf0,vf6x
		vmadday.w	ACC,vf0,vf6y
		vmaddz.w	vf6,vf0,vf6z

		pref		0,0x10(%1)

		vrsqrt		Q,vf6w,vf7w

		vmulax.xyzw	ACC,vf16,vf2x
		vmadday.xyzw	ACC,vf17,vf2y
		vmaddaz.xyzw	ACC,vf18,vf2z
		vmaddw.xyzw	vf12,vf19,vf0w
		vmulax.xyzw	ACC,vf24,vf2x
		vmadday.xyzw	ACC,vf25,vf2y
		vmaddaz.xyzw	ACC,vf26,vf2z
		vmaddw.xyzw	vf14,vf27,vf0w

		vmulw.xyz	vf15,vf15,vf2w

		vwaitq

		vmulq.w		vf6,vf0,Q
		vmula.w		ACC,vf0,vf4
		vmadd.w		vf6,vf6,vf4

		vmulaw.xyzw	ACC,vf12,vf0w
		vmaddaw.xyz	ACC,vf14,vf2w
		vmaddaw.xyz	ACC,vf13,vf6w
		vmaddaw.xyz	ACC,vf15,vf6w
		vmaddx.xyzw	vf2,vf0,vf0x
		" : : "r"(a),"r"(n) );

		a++; n++; i--; r++;

		asm volatile ("
		sqc2		vf2,-0x10(%0)
		" : : "r"(r) );
    }
}

/* 法線用。行列計算から移動ベクトルを抜いたもの。*/
static void MotionVAnimationN(FVECTOR *r,FVECTOR *a,FVECTOR *n,int size,FMATRIX *sm,FMATRIX *dm,
							  FMATRIX *psm,FMATRIX *pdm)
{
    FMATRIX im;
    int i=size;
    static const FVECTOR v={ 0.0f,0.0f,0.0f,1.0f, };

#if 0
    InverseMatrix(&im,sm);
#elif 0
    sceVu0InversMatrix(*(sceVu0FMATRIX *)&im,*(sceVu0FMATRIX *)sm);
#else
    FastInverseMatrix(&im,sm);
#endif

    if(pdm!=NULL && psm!=NULL){
		asm volatile ("
		lqc2		vf12,0x00(%0)
		lqc2		vf13,0x10(%0)
		lqc2		vf14,0x20(%0)
		lqc2		vf15,0x00(%5)

		lqc2		vf28,0x00(%1)
		lqc2		vf29,0x10(%1)
		lqc2		vf30,0x20(%1)
		lqc2		vf31,0x00(%5)
		lqc2		vf24,0x00(%2)
		lqc2		vf25,0x10(%2)
		lqc2		vf26,0x20(%2)
		lqc2		vf27,0x00(%5)
		lqc2		vf20,0x00(%3)
		lqc2		vf21,0x10(%3)
		lqc2		vf22,0x20(%3)
		lqc2		vf23,0x00(%5)
		lqc2		vf16,0x00(%4)
		lqc2		vf17,0x10(%4)
		lqc2		vf18,0x20(%4)
		lqc2		vf19,0x00(%5)
		" : : "r"(&im),"r"(sm),"r"(dm),"r"(psm),"r"(pdm),"r"(&v) );
    }
    else{
		asm volatile ("
		lqc2		vf12,0x00(%0)
		lqc2		vf13,0x10(%0)
		lqc2		vf14,0x20(%0)
		lqc2		vf15,0x00(%3)

		lqc2		vf28,0x00(%1)
		lqc2		vf29,0x10(%1)
		lqc2		vf30,0x20(%1)
		lqc2		vf31,0x00(%3)
		lqc2		vf24,0x00(%2)
		lqc2		vf25,0x10(%2)
		lqc2		vf26,0x20(%2)
		lqc2		vf27,0x00(%3)
		vmove.xyzw	vf20,vf28
		vmove.xyzw	vf21,vf29
		vmove.xyzw	vf22,vf30
		vmove.xyzw	vf23,vf31
		vmove.xyzw	vf16,vf24
		vmove.xyzw	vf17,vf25
		vmove.xyzw	vf18,vf26
		vmove.xyzw	vf19,vf27
		" : : "r"(&im),"r"(sm),"r"(dm),"r"(&v) );
    }

    {
		static const FVECTOR c={ 0.0f,0.0f,0.0f,0.5f, };

		asm volatile ("
		vsub.xyzw	vf28,vf28,vf20
		vsub.xyzw	vf29,vf29,vf21
		vsub.xyzw	vf30,vf30,vf22
		vsub.xyzw	vf31,vf31,vf23

		vsub.xyzw	vf24,vf24,vf16
		vsub.xyzw	vf25,vf25,vf17
		vsub.xyzw	vf26,vf26,vf18
		vsub.xyzw	vf27,vf27,vf19

		vsub.xyzw	vf20,vf20,vf16
		vsub.xyzw	vf21,vf21,vf17
		vsub.xyzw	vf22,vf22,vf18
		vsub.xyzw	vf23,vf23,vf19

		vsub.xyzw	vf28,vf28,vf24
		vsub.xyzw	vf29,vf29,vf25
		vsub.xyzw	vf30,vf30,vf26
		vsub.xyzw	vf31,vf31,vf27

		vmulax.xyzw	ACC,vf12,vf16x
		vmadday.xyzw	ACC,vf13,vf16y
		vmaddaz.xyzw	ACC,vf14,vf16z
		vmaddw.xyzw	vf16,vf15,vf16w
		vmulax.xyzw	ACC,vf12,vf17x
		vmadday.xyzw	ACC,vf13,vf17y
		vmaddaz.xyzw	ACC,vf14,vf17z
		vmaddw.xyzw	vf17,vf15,vf17w
		vmulax.xyzw	ACC,vf12,vf18x
		vmadday.xyzw	ACC,vf13,vf18y
		vmaddaz.xyzw	ACC,vf14,vf18z
		vmaddw.xyzw	vf18,vf15,vf18w
		vmulax.xyzw	ACC,vf12,vf19x
		vmadday.xyzw	ACC,vf13,vf19y
		vmaddaz.xyzw	ACC,vf14,vf19z
		vmaddw.xyzw	vf19,vf15,vf19w

		vmulax.xyzw	ACC,vf12,vf20x
		vmadday.xyzw	ACC,vf13,vf20y
		vmaddaz.xyzw	ACC,vf14,vf20z
		vmaddw.xyzw	vf20,vf15,vf20w
		vmulax.xyzw	ACC,vf12,vf21x
		vmadday.xyzw	ACC,vf13,vf21y
		vmaddaz.xyzw	ACC,vf14,vf21z
		vmaddw.xyzw	vf21,vf15,vf21w
		vmulax.xyzw	ACC,vf12,vf22x
		vmadday.xyzw	ACC,vf13,vf22y
		vmaddaz.xyzw	ACC,vf14,vf22z
		vmaddw.xyzw	vf22,vf15,vf22w
		vmulax.xyzw	ACC,vf12,vf23x
		vmadday.xyzw	ACC,vf13,vf23y
		vmaddaz.xyzw	ACC,vf14,vf23z
		vmaddw.xyzw	vf23,vf15,vf23w

		vmulax.xyzw	ACC,vf12,vf24x
		vmadday.xyzw	ACC,vf13,vf24y
		vmaddaz.xyzw	ACC,vf14,vf24z
		vmaddw.xyzw	vf24,vf15,vf24w
		vmulax.xyzw	ACC,vf12,vf25x
		vmadday.xyzw	ACC,vf13,vf25y
		vmaddaz.xyzw	ACC,vf14,vf25z
		vmaddw.xyzw	vf25,vf15,vf25w
		vmulax.xyzw	ACC,vf12,vf26x
		vmadday.xyzw	ACC,vf13,vf26y
		vmaddaz.xyzw	ACC,vf14,vf26z
		vmaddw.xyzw	vf26,vf15,vf26w
		vmulax.xyzw	ACC,vf12,vf27x
		vmadday.xyzw	ACC,vf13,vf27y
		vmaddaz.xyzw	ACC,vf14,vf27z
		vmaddw.xyzw	vf27,vf15,vf27w

		lqc2		vf4,0x00(%0)

		vmulax.xyzw	ACC,vf12,vf28x
		vmadday.xyzw	ACC,vf13,vf28y
		vmaddaz.xyzw	ACC,vf14,vf28z
		vmaddw.xyzw	vf28,vf15,vf28w
		vmulax.xyzw	ACC,vf12,vf29x
		vmadday.xyzw	ACC,vf13,vf29y
		vmaddaz.xyzw	ACC,vf14,vf29z
		vmaddw.xyzw	vf29,vf15,vf29w
		vmulax.xyzw	ACC,vf12,vf30x
		vmadday.xyzw	ACC,vf13,vf30y
		vmaddaz.xyzw	ACC,vf14,vf30z
		vmaddw.xyzw	vf30,vf15,vf30w
		vmulax.xyzw	ACC,vf12,vf31x
		vmadday.xyzw	ACC,vf13,vf31y
		vmaddaz.xyzw	ACC,vf14,vf31z
		vmaddw.xyzw	vf31,vf15,vf31w
		" : : "r"(&c) );
    }

    while(i>0){

		/* vf4w = 0.5f */

		asm volatile ("
		lqc2		vf2,0(%0)
		lqc2		vf3,0(%1)
		pref		0,0x10(%0)

		vmulax.xyzw	ACC,vf28,vf2x
		vmadday.xyzw	ACC,vf29,vf2y
		vmaddaz.xyzw	ACC,vf30,vf2z
		vmaddw.xyzw	vf15,vf31,vf0w
		vmulax.xyzw	ACC,vf20,vf2x
		vmadday.xyzw	ACC,vf21,vf2y
		vmaddaz.xyzw	ACC,vf22,vf2z
		vmaddaw.xyzw	ACC,vf23,vf0w
		vmaddx.xyzw	vf13,vf0,vf0x

		vmaddw.xyz	vf8,vf15,vf2w

		vmul.xyz	vf7,vf8,vf8
		vmul.xyz	vf6,vf8,vf3

		vmulax.w	ACC,vf0,vf7x
		vmadday.w	ACC,vf0,vf7y
		vmaddz.w	vf7,vf0,vf7z
		vmulax.w	ACC,vf0,vf6x
		vmadday.w	ACC,vf0,vf6y
		vmaddz.w	vf6,vf0,vf6z

		pref		0,0x10(%1)

		vrsqrt		Q,vf6w,vf7w

		vmulax.xyzw	ACC,vf16,vf2x
		vmadday.xyzw	ACC,vf17,vf2y
		vmaddaz.xyzw	ACC,vf18,vf2z
		vmaddw.xyzw	vf12,vf19,vf0w
		vmulax.xyzw	ACC,vf24,vf2x
		vmadday.xyzw	ACC,vf25,vf2y
		vmaddaz.xyzw	ACC,vf26,vf2z
		vmaddw.xyzw	vf14,vf27,vf0w

		vmulw.xyz	vf15,vf15,vf2w

		vwaitq

		vmulq.w		vf6,vf0,Q
		vmula.w		ACC,vf0,vf4
		vmadd.w		vf6,vf6,vf4

		vmulaw.xyzw	ACC,vf12,vf0w
		vmaddaw.xyz	ACC,vf14,vf2w
		vmaddaw.xyz	ACC,vf13,vf6w
		vmaddaw.xyz	ACC,vf15,vf6w
		vmaddx.xyzw	vf2,vf0,vf0x
		" : : "r"(a),"r"(n) );

		a++; n++; i--; r++;

		asm volatile ("
		sqc2		vf2,-0x10(%0)
		" : : "r"(r) );
    }
}
#endif /* KP_XBOX ************** PS2用の専用関数(オリジナル)ここまで***********************  T.Morita 2002.02.01 */


//#include "../../user/sonoyama/raiden/pl_work.h"

MOTION_VANIME_WORK *InitMotionVAnimation(DG_OBJ *obj,CV2_MDL *mdl,int flag,int frame)
{
    MOTION_VANIME_WORK *work;
    short *indexes;
    int msize=0;
    int i;


    //if(mdl->n_verts==0) return NULL;

    if((work=GV_Malloc(sizeof(MOTION_VANIME_WORK)))==NULL) return NULL;

    if(flag & DG_VANIME_VERTS){
		if(mdl->n_verts>msize) msize=mdl->n_verts;
    }
#if 1
    if(flag & DG_VANIME_NORMS){
		if(mdl->n_norms>msize) msize=mdl->n_norms;
    }
#endif

    msize=(msize+3) & ~3;
    msize*=sizeof(FVECTOR);

    work->stock=NULL;
    if((work->stock=(FMATRIX *)GV_Malloc(sizeof(FMATRIX)*(frame+1)+msize
										 +sizeof(FVECTOR)*mdl->n_verts))==NULL){
		GV_Free(work);
		return NULL;
    }
    work->stockp=0;
    work->stock_size=frame+1;
    work->stock_count=0;
    work->frames=frame;
    work->v_normal=(FVECTOR *)(work->stock+frame+1);
    work->tmp_work=(FVECTOR *)(work->v_normal+mdl->n_verts);

#if 0
    for(i=0;i<frame+1;i++){
		fpu_CopyMatrix(work->stock+i,&(obj->world));
    }
#endif

    /* 頂点数と法線数が一致しない時に備えて、頂点数分の法線を取り出す。*/
#ifndef NEW_KMX_FORMAT
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
#else
	{
		DG_VERTEX_KMSS	*v ;
		v = obj->model->vbuff ;
		for ( i = 0 ; i < obj->model->n_verts ; i++ ){
			static const float p=-1.0f/32767.0f;
			work->v_normal[ i ].vx = v->nx * p ;
			work->v_normal[ i ].vy = v->ny * p ;
			work->v_normal[ i ].vz = v->nz * p ;
			v = (void*)( (int)v + obj->model->stride );
		}
	}
#endif

    work->cvd=mdl;
    work->flag=0;

    DG_MakeAnimVertsBuffer(&(work->anime),obj,flag);

    return work;
}

void ExitMotionVAnimation(MOTION_VANIME_WORK *work)
{
    if(work->stock!=NULL) GV_Free(work->stock);
    DG_FreeAnimVertsBuffer(&(work->anime));
    GV_Free(work);
}

void MotionVertexAnimation(MOTION_VANIME_WORK *work,MOTION_VANIME_WORK *pwork,int flag)
{
    FVECTOR *tmp=work->tmp_work;
    FMATRIX *sm,*dm,*psm,*pdm;
    FVECTOR tv;
    DG_OBJ *obj=work->anime.obj;

    if(flag ^ work->flag){
		if(flag){
			DG_RestartAnimVerts(&(work->anime));
			if(work->stock_count<0) work->stock_count=0;
		}
		work->flag=flag;
    }
    if(!flag){
		int index;

		switch(work->stock_count){
		case 0:
			DG_StopAnimVerts(&(work->anime));
			work->stock_count--;
		case -1:
			return;
		default:
			sm=&(obj->world);
			dm=work->stock+work->stockp;

			fpu_CopyMatrix(dm,sm);

			if(pwork) psm=pwork->stock+work->stockp;
			else psm=NULL;

			index=work->stockp-work->stock_count;
			if(index<0) index+=work->stock_size;

			dm=work->stock+index;
			if(pwork) pdm=pwork->stock+index;
			else pdm=NULL;

			work->stockp++;
			if(work->stockp>=work->stock_size) work->stockp=0;

			work->stock_count--;
			break;
		}
    }
    else{
		int index;

		sm=&(obj->world);
		dm=work->stock+work->stockp;

		fpu_CopyMatrix(dm,sm);

		if(pwork) psm=pwork->stock+work->stockp;
		else psm=NULL;

		index=work->stockp-work->stock_count;
		if(index<0) index+=work->stock_size;

		dm=work->stock+index;
		if(pwork) pdm=pwork->stock+index;
		else pdm=NULL;

		work->stockp++;
		if(work->stockp>=work->stock_size) work->stockp=0;

		if(work->stock_count<=work->frames){
			work->stock_count++;
			if(work->stock_count>work->frames) work->stock_count=work->frames;
		}
		else{
			work->stock_count--;
			if(work->stock_count<work->frames) work->stock_count=work->frames;
		}
    }

    tv.vx=obj->model->tx;
    tv.vy=obj->model->ty;
    tv.vz=obj->model->tz;
    tv.vw=1.0f;

    if(work->anime.flag & DG_VANIME_VERTS){
		MotionVAnimation(tmp,work->cvd->verts,work->v_normal,work->cvd->n_verts,
						 sm,dm,psm,pdm,&tv);

#ifndef KP_WINDOWS
		DG_RegistCommonVertex(tmp,work->cvd->n_verts);
		DG_RefineStripVertex(&(work->anime),work->cvd->verts_index);
#else
		DG_RegistCommonVertexRefineStripVertex(&(work->anime), tmp);
#endif
    }
#if 1
    if(work->anime.flag & DG_VANIME_NORMS){
		MotionVAnimationN(tmp,work->cvd->norms,work->cvd->norms,work->cvd->n_norms,
						  sm,dm,psm,pdm);

#ifndef KP_WINDOWS
		DG_RegistCommonNormal2(tmp,work->cvd->n_norms);
		DG_RefineStripNormal(&(work->anime),work->cvd->norms_index);
#else
		DG_RegistCommonNormal2RefineStripNormal(&(work->anime),tmp);
#endif
    }
#endif
}

void MotionVertexAnimation2(MOTION_VANIME_WORK *work,FMATRIX *psm,int flag)
{
    FVECTOR *tmp=work->tmp_work;
    FMATRIX *sm,*dm;
    FVECTOR tv;
    DG_OBJ *obj=work->anime.obj;

    if(flag ^ work->flag){
		if(flag){
			DG_RestartAnimVerts(&(work->anime));
			if(work->stock_count<0) work->stock_count=0;
		}
		work->flag=flag;
    }
    if(!flag){
		int index;

		switch(work->stock_count){
		case 0:
			DG_StopAnimVerts(&(work->anime));
			work->stock_count--;
		case -1:
			return;
		default:
			sm=&(obj->world);
			dm=work->stock+work->stockp;

			fpu_CopyMatrix(dm,sm);

			index=work->stockp-work->stock_count;
			if(index<0) index+=work->stock_size;

			dm=work->stock+index;

			work->stockp++;
			if(work->stockp>=work->stock_size) work->stockp=0;

			work->stock_count--;
			break;
		}
    }
    else{
		int index;

		sm=&(obj->world);
		dm=work->stock+work->stockp;

		fpu_CopyMatrix(dm,sm);

		index=work->stockp-work->stock_count;
		if(index<0) index+=work->stock_size;

		dm=work->stock+index;

		work->stockp++;
		if(work->stockp>=work->stock_size) work->stockp=0;

		if(work->stock_count<=work->frames){
			work->stock_count++;
			if(work->stock_count>work->frames) work->stock_count=work->frames;
		}
		else{
			work->stock_count--;
			if(work->stock_count<work->frames) work->stock_count=work->frames;
		}
    }

    tv.vx=obj->model->tx;
    tv.vy=obj->model->ty;
    tv.vz=obj->model->tz;
    tv.vw=1.0f;

    if(work->anime.flag & DG_VANIME_VERTS){
		MotionVAnimation(tmp,work->cvd->verts,work->v_normal,work->cvd->n_verts,
						 sm,dm,psm,psm,&tv);

#ifndef KP_WINDOWS
		DG_RegistCommonVertex(tmp,work->cvd->n_verts);
		DG_RefineStripVertex(&(work->anime),work->cvd->verts_index);
#else
		DG_RegistCommonVertexRefineStripVertex(&(work->anime),tmp);
#endif
    }
#if 1
    if(work->anime.flag & DG_VANIME_NORMS){
		MotionVAnimationN(tmp,work->cvd->norms,work->cvd->norms,work->cvd->n_norms,
						  sm,dm,psm,psm);

#ifndef KP_WINDOWS
		DG_RegistCommonNormal2(tmp,work->cvd->n_norms);
		DG_RefineStripNormal(&(work->anime),work->cvd->norms_index);
#else
		DG_RegistCommonNormal2RefineStripNormal(&(work->anime),tmp);
#endif
    }
#endif
}

void MotionVertexAnimation3(MOTION_VANIME_WORK *work,MOTION_VANIME_WORK *pwork,int flag)
{
    FVECTOR *tmp=work->tmp_work;
    FMATRIX *sm,*dm,*psm,*pdm;
    FVECTOR tv;
    DG_OBJ *obj=work->anime.obj;


    if(flag ^ work->flag){
		if(flag){
			DG_RestartAnimVerts(&(work->anime));
			if(work->stock_count<0) work->stock_count=0;
		}
		work->flag=flag;
    }
    if(!flag){
		int index;

		switch(work->stock_count){
		case 0:
			DG_StopAnimVerts(&(work->anime));
			work->stock_count--;
		case -1:
			return;
		default:
			sm=&(obj->world);
			dm=work->stock+work->stockp;

			fpu_CopyMatrix(dm,sm);

			if(pwork) psm=pwork->stock+work->stockp;
			else psm=NULL;

			index=work->stockp-work->stock_count;
			if(index<0) index+=work->stock_size;

			if(pwork) pdm=pwork->stock+index;
			else pdm=NULL;

			work->stockp++;
			if(work->stockp>=work->stock_size) work->stockp=0;

			work->stock_count--;
			break;
		}
    }
    else{
		int index;

		sm=&(obj->world);
		dm=work->stock+work->stockp;

		fpu_CopyMatrix(dm,sm);

		if(pwork) psm=pwork->stock+work->stockp;
		else psm=NULL;

		index=work->stockp-work->stock_count;
		if(index<0) index+=work->stock_size;

		if(pwork) pdm=pwork->stock+index;
		else pdm=NULL;

		work->stockp++;
		if(work->stockp>=work->stock_size) work->stockp=0;

		if(work->stock_count<=work->frames){
			work->stock_count++;
			if(work->stock_count>work->frames) work->stock_count=work->frames;
		}
		else{
			work->stock_count--;
			if(work->stock_count<work->frames) work->stock_count=work->frames;
		}
    }

    tv.vx=obj->model->tx;
    tv.vy=obj->model->ty;
    tv.vz=obj->model->tz;
    tv.vw=1.0f;

    DG_SwitchVAnimeBuffer(&(work->anime));

    if(work->anime.flag & DG_VANIME_VERTS){
		MotionVAnimation(tmp,work->cvd->verts,work->v_normal,work->cvd->n_verts,
						 sm,sm,psm,pdm,&tv);

#ifndef KP_WINDOWS
		DG_RegistCommonVertex(tmp,work->cvd->n_verts);
		DG_RefineStripVertex(&(work->anime),work->cvd->verts_index);
#else
		DG_RegistCommonVertexRefineStripVertex(&(work->anime),tmp);
#endif
    }
#if 1
    if(work->anime.flag & DG_VANIME_NORMS){
		MotionVAnimationN(tmp,work->cvd->norms,work->cvd->norms,work->cvd->n_norms,
						  sm,sm,psm,pdm);

#ifndef KP_WINDOWS
		DG_RegistCommonNormal2(tmp,work->cvd->n_norms);
		DG_RefineStripNormal(&(work->anime),work->cvd->norms_index);
#else
		DG_RegistCommonVertexRefineStripVertex(&(work->anime),tmp);
#endif
    }
#endif
}
