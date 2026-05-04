//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vec_util.c

	演算補助ライブラリ

	2001/04/06 K.Takabe
	$Id: vec_util.c,v 1.1.1.3 2002/11/19 11:51:19 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"
#include	"libmt.h"
#include	"gameheader.h"
#include	"debugmenu.h"

#include "vec_util.h"

#if !defined(PSX2) || defined(VEC_UTIL_FORCE_NORMAL)
/* ---------------------------------------------------------------- */
static FMATRIX		matrix_stack[32] ;
static int			matrix_stack_addr = 0 ;
/* ---------------------------------------------------------------- */
void GTE_InitGTE( void )
{
	matrix_stack_addr = 0 ;
}

void GTE_PushMatrix( void )
{
	DG_GetPos( &matrix_stack[ matrix_stack_addr ] );
	matrix_stack_addr++ ;
}

void GTE_PopMatrix( void )
{
	matrix_stack_addr-- ;
	DG_SetPos( &matrix_stack[ matrix_stack_addr ] );
}

void GTE_LoadIdentity( void )
{
	FMATRIX	mat ;
	GTE_UnitMatrix( &mat );
	GTE_LoadMatrix( &mat );
}

void GTE_MultMatrix( FMATRIX *m0 )
{
	FMATRIX	tmp_mat ;
	DG_GetPos( &tmp_mat );
	_sceVu0MulMatrix( &tmp_mat, &tmp_mat, m0 );
	DG_SetPos( &tmp_mat );
}

void GTE_RotateX( float a )
{
	FMATRIX	tmp_mat, org_mat ;
	DG_GetPos( &org_mat );
	GTE_UnitMatrix( &tmp_mat );
	_sceVu0RotMatrixX( &tmp_mat, &tmp_mat, a );
	GTE_MulMatrix( &tmp_mat, &org_mat, &tmp_mat );
	DG_SetPos( &tmp_mat );
}

void GTE_RotateY( float a )
{
	FMATRIX	tmp_mat, org_mat ;
	DG_GetPos( &org_mat );
	GTE_UnitMatrix( &tmp_mat );
	_sceVu0RotMatrixY( &tmp_mat, &tmp_mat, a );
	GTE_MulMatrix( &tmp_mat, &org_mat, &tmp_mat );
	DG_SetPos( &tmp_mat );
}

void GTE_RotateZ( float a )
{
	FMATRIX	tmp_mat, org_mat ;
	DG_GetPos( &org_mat );
	GTE_UnitMatrix( &tmp_mat );
	_sceVu0RotMatrixZ( &tmp_mat, &tmp_mat, a );
	GTE_MulMatrix( &tmp_mat, &org_mat, &tmp_mat );
	DG_SetPos( &tmp_mat );
}

void GTE_SinCos( FVECTOR *res, float angle )
{
	res->vx = sinf( angle );
	res->vy = cosf( angle );
}

void GTE_MakeRotateAxis( FMATRIX *res, FVECTOR *axis, float angle )
{
	float	c, s, ic, tmp ;
	c = cosf( angle );
	s = -sinf( angle );
	ic = 1.0f - c ;
	/* x */
	tmp = ic * axis->vx ;
	res->m[0][0] = tmp * axis->vx + c ;
	res->m[0][1] = tmp * axis->vy - axis->vz * s ;
	res->m[0][2] = tmp * axis->vz + axis->vy * s ;
	res->m[0][3] = 0.0f ;
	/* y */
	tmp = ic * axis->vy ;
	res->m[1][0] = tmp * axis->vx + axis->vz * s ;
	res->m[1][1] = tmp * axis->vy + c ;
	res->m[1][2] = tmp * axis->vz - axis->vx * s ;
	res->m[1][3] = 0.0f ;
	/* z */
	tmp = ic * axis->vz ;
	res->m[2][0] = tmp * axis->vx - axis->vy * s ;
	res->m[2][1] = tmp * axis->vy + axis->vx * s ;
	res->m[2][2] = tmp * axis->vz + c ;
	res->m[2][3] = 0.0f ;
	/* t */
	res->m[3][0] = 0.0f ;
	res->m[3][1] = 0.0f ;
	res->m[3][2] = 0.0f ;
	res->m[3][3] = 1.0f ;
}
void GTE_MakeRotate( FMATRIX *res, float x, float y, float z, float angle )
{
	FVECTOR	axis ;
	axis.vx = x ;
	axis.vy = y ;
	axis.vz = z ;
	GTE_Normalize( &axis, &axis );
	GTE_MakeRotateAxis( res, &axis, angle );
}

#else
/* ---------------------------------------------------------------- */
void GTE_InitGTE( void )
{
	/* マトリクススタック設定 */
	asm volatile ("
		addi		$4,$0,256
		ctc2		$4,$vi15
	":::"$4" );
}

void GTE_RotateX( float a )
{
	FMATRIX	tmp_mat ;
	GTE_PushMatrix();
	GTE_UnitMatrix( &tmp_mat );
	_sceVu0RotMatrixX( &tmp_mat, &tmp_mat, a );
	GTE_PopMatrix();
	GTE_MultMatrix( &tmp_mat );
}

void GTE_RotateY( float a )
{
	FMATRIX	tmp_mat ;
	GTE_PushMatrix();
	GTE_UnitMatrix( &tmp_mat );
	_sceVu0RotMatrixY( &tmp_mat, &tmp_mat, a );
	GTE_PopMatrix();
	GTE_MultMatrix( &tmp_mat );
}

void GTE_RotateZ( float a )
{
	FMATRIX	tmp_mat ;
	GTE_PushMatrix();
	GTE_UnitMatrix( &tmp_mat );
	_sceVu0RotMatrixZ( &tmp_mat, &tmp_mat, a );
	GTE_PopMatrix();
	GTE_MultMatrix( &tmp_mat );
}

void GTE_SinCos( FVECTOR *res, float angle )
{
	static ALIGN16_PRE u_int	param[8] ALIGN16_POST = { 0x3f800000,0xbe2aaaa4,0x3c08873e,0xb94fb21f,0x362e9c14,0,0,0};
	FVECTOR	xx ;
	if ( angle > (float)(M_PI/2) )			xx.vx = (float)M_PI - angle ;
	else if ( angle < (float)(-M_PI/2) )	xx.vx = (float)(-M_PI) - angle ;
	else									xx.vx = angle ;
	if ( angle > 0.0f )						xx.vy = (float)(M_PI/2) - angle ;
	else									xx.vy = (float)(M_PI/2) + angle ;
	asm("
		lqc2			vf4,0(%1)		# vf4 = x
		lqc2			vf1,0(%2)
		lqc2			vf2,16(%2)
		vmul.xyzw		vf5,vf4,vf4		# vf5 = x^2
		vmulax.xyzw		ACC,vf4,vf1		# ACC = x * s1
		vmul.xyzw		vf6,vf5,vf4		# vf6 = x^3
		vmul.xyzw		vf8,vf5,vf5		# vf8 = x^4
		vmulx.xyzw		vf9,vf5,vf2		# vf9 = x^2 * s5
		vmul.xyzw		vf7,vf6,vf5		# vf7 = x^5
		vmul.xyzw		vf8,vf8,vf6		# vf8 = x^7
		vmadday.xyzw	ACC,vf6,vf1		# ACC = ACC + x^3 * s2
		vmaddaz.xyzw	ACC,vf7,vf1		# ACC = ACC + x^5 * s3
		vmaddaw.xyzw	ACC,vf8,vf1		# ACC = ACC + x^7 * s4
		vmadd.xyzw		vf4,vf9,vf8		# vf4 = ACC + x^7 * x^2 * s5 = ACC + x^9 * s5
		sqc2			vf4,0(%0)
	"::"r"(res),"r"(&xx),"r"(param):"memory" );
}

void GTE_MakeRotateAxis( FMATRIX *res, FVECTOR *axis, float angle )
{
#if 1
	FVECTOR	sincos ;
	GTE_SinCos( &sincos, angle );
	sincos.vw = 1.0f - sincos.vy ;
	asm volatile ("
		lqc2			vf1,0x00(%1)	# vf1 = axis
		lqc2			vf2,0x00(%2)	# vf2 = x:sin y:cos w:1-cos
		vaddw.xyz		vf8,vf0,vf0		# vf8 = 1, 1, 1, ?
		vmulw.xyz		vf3,vf1,vf2		# vf3 = (1-cos)x, (1-cos)y, (1-cos)z
		vmulx.xyz		vf9,vf1,vf2		# vf9 = sinx siny sinz
		vmove.xyzw		vf7,vf0			# 
		vmulax.xyzw		ACC,vf0,vf0		# acc = 0
		# x
		vmulax.xyz		ACC,vf1,vf3		# acc = axis * (1-cos)x
		vmadday.x		ACC,vf8,vf2		# accx += cos
		vmaddaz.y		ACC,vf8,vf9		# accy -= -sinz
		vmsubay.z		ACC,vf8,vf9		# accz += -siny
		vmaddx.xyzw		vf4,vf0,vf0		# 
		# y
		vmulay.xyz		ACC,vf1,vf3		# acc = axis * (1-cos)y
		vmsubaz.x		ACC,vf8,vf9		# accx += -sinz
		vmadday.y		ACC,vf8,vf2		# accy += cos
		vmaddax.z		ACC,vf8,vf9		# accz -= -sinx
		vmaddx.xyzw		vf5,vf0,vf0		# 
		# z
		vmulaz.xyz		ACC,vf1,vf3		# acc = axis * (1-cos)z
		vmadday.x		ACC,vf8,vf9		# accx -= -siny
		vmsubax.y		ACC,vf8,vf9		# accy += -sinx
		vmadday.z		ACC,vf8,vf2		# accz += cos
		vmaddx.xyzw		vf6,vf0,vf0		# 
		#
		sqc2			vf4,0x00(%0)
		sqc2			vf5,0x10(%0)
		sqc2			vf6,0x20(%0)
		sqc2			vf7,0x30(%0)
	"::"r"(res),"r"(axis),"r"(&sincos):"memory");
#else
	float	c, s, ic, tmp ;
	c = cosf( angle );
	s = -sinf( angle );
	ic = 1.0f - c ;
	/* x */
	tmp = ic * axis->vx ;
	res->m[0][0] = tmp * axis->vx + c ;
	res->m[0][1] = tmp * axis->vy - axis->vz * s ;
	res->m[0][2] = tmp * axis->vz + axis->vy * s ;
	res->m[0][3] = 0.0f ;
	/* y */
	tmp = ic * axis->vy ;
	res->m[1][0] = tmp * axis->vx + axis->vz * s ;
	res->m[1][1] = tmp * axis->vy + c ;
	res->m[1][2] = tmp * axis->vz - axis->vx * s ;
	res->m[1][3] = 0.0f ;
	/* z */
	tmp = ic * axis->vz ;
	res->m[2][0] = tmp * axis->vx - axis->vy * s ;
	res->m[2][1] = tmp * axis->vy + axis->vx * s ;
	res->m[2][2] = tmp * axis->vz + c ;
	res->m[2][3] = 0.0f ;
	/* t */
	res->m[3][0] = 0.0f ;
	res->m[3][1] = 0.0f ;
	res->m[3][2] = 0.0f ;
	res->m[3][3] = 1.0f ;
#endif
}
void GTE_MakeRotate( FMATRIX *res, float x, float y, float z, float angle )
{
	FVECTOR	axis ;
	axis.vx = x ;
	axis.vy = y ;
	axis.vz = z ;
	GTE_Normalize( &axis, &axis );
	GTE_MakeRotateAxis( res, &axis, angle );
}

/* ---------------------------------------------------------------- */

#endif



/* ---------------------------------------------------------------- */

