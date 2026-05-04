//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	math_util.c
	計算補助ルーチン

	1999/07/07 K.Takabe
	$Id: math_utl.c,v 1.1.1.3 2002/11/19 11:42:52 Yoshizawa1 Exp $

*/
/*

	void MT_SinX4( FVECTOR *res, FVECTOR *x )
	FVECTOR		*res ;	出力×４
	FVECTOR		*x ;	入力×４

	４つ同時に正弦を求める（ PI > x > -PI ）


	void MT_CosX4( FVECTOR *res, FVECTOR *x )
	FVECTOR		*res ;	出力×４
	FVECTOR		*x ;	入力×４

	４つ同時に余弦を求める（ PI > x > -PI ）


	void MT_Atan2X4( FVECTOR *res, FVECTOR *y, FVECTOR *x )
	FVECTOR		*res ;	出力×４
	FVECTOR		*y ;	入力×４
	FVECTOR		*x ;	入力×４

	４つ同時に逆正接を求める


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

#include	"libgv.h"
#include	"libmt.h"

#ifdef M_PI
#undef M_PI
#endif
#define M_PI 3.14159265358979323846264338327950288419716939937510f

#define M_PI_H (M_PI/2)

#define DELTA 1e-6f     // error tolerance

#if 0
#define sqrtf(m) ({float _f=(m);asm("sqrt.s %0,%0":"+f"(_f));_f;})
#endif


/*----------------------------------------------------------------*/

	/*
		４つ同時に正弦を求める
	*/
void MT_SinX4( FVECTOR *res, FVECTOR *x )
{
#if 0 //BP_PS2
//#ifdef PSX2
	static ALIGN16_PRE u_int	param[8] ALIGN16_POST = { 0x3f800000,0xbe2aaaa4,0x3c08873e,0xb94fb21f,0x362e9c14,0,0,0} ;
	FVECTOR	xx ;
	float	*f1, *f2 ;
	int		i ;

	f1 = (float*)x ;
	f2 = (float*)&xx ;
	for ( i = 4 ; i > 0 ; f1++, f2++, i-- ){
		if ( *f1 > (float)(M_PI/2) )		*f2 = (float)(M_PI) - *f1 ;
		else if ( *f1 < (float)(-M_PI/2) )	*f2 = (float)(-M_PI) - *f1 ;
		else								*f2 = *f1 ;
	}

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
#else
	res->vx = sinf( x->vx ) ;
	res->vy = sinf( x->vy ) ;
	res->vz = sinf( x->vz ) ;
	res->vw = sinf( x->vw ) ;
#endif

}

	/*
		４つ同時に余弦を求める
	*/
void MT_CosX4( FVECTOR *res, FVECTOR *x )
{
#if 0 //BP_PS2
//#ifdef PSX2
	static ALIGN16_PRE u_int	param[8] ALIGN16_POST = { 0x3f800000,0xbe2aaaa4,0x3c08873e,0xb94fb21f,0x362e9c14,0,0,0};
	FVECTOR	xx ;
	float	*f1, *f2 ;
	int		i ;

	f1 = (float*)x ;
	f2 = (float*)&xx ;
	for ( i = 4 ; i > 0 ; f1++, f2++, i-- ){
		if ( *f1 > 0.0F )		*f2 = (float)(M_PI/2) - *f1 ;
		else					*f2 = (float)(M_PI/2) + *f1 ;
	}

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
#else
	res->vx = cosf( x->vx ) ;
	res->vy = cosf( x->vy ) ;
	res->vz = cosf( x->vz ) ;
	res->vw = cosf( x->vw ) ;
#endif
}

	/*
		４つ同時に逆正接を求める
	*/
void MT_Atan2X4( FVECTOR *res, FVECTOR *y, FVECTOR *x )
{
#if 0 //BP_PS2
//#ifdef PSX2
	static ALIGN16_PRE u_int	param[12]  ALIGN16_POST = { 
		0x3f7ffff5,0xbeaaa61c,0x3e4c40a6,0xbe0e6c63,
		0x3dc577df,0xbd6501c4,0x3cb31652,0xbb84d7e7,
		0x3f490fdb,0x3f490fdb,0x3f490fdb,0x3f490fdb
	};
	FVECTOR	xx ;
	float	*f0, *f1, *f2, _f0, _f1, _f2 ;
	int		i, flag = 0 ;

	f0 = (float*)y ;
	f1 = (float*)x ;
	f2 = (float*)&xx ;
	for ( i = 4 ; i > 0 ; f0++, f1++, f2++, i-- ){
		flag <<= 3 ;
		_f0 = *f0 ; _f1 = *f1 ;
		if ( _f0 < 0.0f ){
			_f0 = -_f0 ;
			flag |= 4 ;
		}
		if ( _f1 < 0.0f ){
			_f1 = -_f1 ;
			flag |= 2 ;
		}
		if ( _f1 > _f0 ){
			*f2 = ( _f0 - _f1 ) / ( _f0 + _f1 ) ;
		} else {
			*f2 = ( _f1 - _f0 ) / ( _f1 + _f0 ) ;
			flag |= 1 ;
		}
	}

	asm("
		lqc2			vf4,0(%1)		# vf4 = x
		lqc2			vf1,0(%2)
		lqc2			vf2,16(%2)
		lqc2			vf3,32(%2)
		vmul.xyzw		vf13,vf4,vf4	# vf13 = x^2
		vmulax.xyzw		ACC,vf4,vf1		# ACC = x * s1
		vmul.xyzw		vf6,vf13,vf4	# vf6 = x^3
		vmul.xyzw		vf14,vf13,vf13	# vf14 = x^4
		vmul.xyzw		vf7,vf6,vf13	# vf7 = x^5
		vmul.xyzw		vf15,vf6,vf6	# vf15 = x^6
		vmul.xyzw		vf16,vf14,vf14	# vf16 = x^8
		vmul.xyzw		vf8,vf14,vf6	# vf8 = x^7
		vmul.xyzw		vf9,vf14,vf7	# vf9 = x^9
		vmul.xyzw		vf10,vf7,vf15	# vf10 = x^11
		vmadday.xyzw	ACC,vf6,vf1		# ACC = ACC + x^3 * s2
		vmul.xyzw		vf11,vf8,vf15	# vf11 = x^13
		vmul.xyzw		vf12,vf8,vf16	# vf12 = x^15
		vmaddaz.xyzw	ACC,vf7,vf1		# ACC = ACC + x^5 * s3
		vmaddaw.xyzw	ACC,vf8,vf1		# ACC = ACC + x^7 * s4
		vmaddax.xyzw	ACC,vf9,vf2		# ACC = ACC + x^9 * s5
		vmadday.xyzw	ACC,vf10,vf2	# ACC = ACC + x^11 * s6
		vmaddaz.xyzw	ACC,vf11,vf2	# ACC = ACC + x^13 * s7
		vmaddaw.xyzw	ACC,vf12,vf2	# ACC = ACC + x^15 * s8
		vmaddw.xyzw		vf4,vf3,vf0
		sqc2			vf4,0(%0)
	"::"r"(res),"r"(&xx),"r"(param):"memory" );

	f2 = (float*)res ;
	for ( i = 4 ; i > 0 ; f2++, i-- ){
		flag <<= 3 ;
		_f2 = *f2 ;
		if ( flag & ( 0x1000 ) ){
			_f2 = (float)(M_PI/2) - _f2 ;
		}
		if ( flag & ( 0x2000 ) ){
			_f2 = (float)(M_PI) - _f2 ;
		}
		if ( flag & ( 0x4000 ) ){
			_f2 = -_f2 ;
		}
		*f2 = _f2 ;
	}

#else

	res->vx = atan2f( y->vx, x->vx ) ;
	res->vy = atan2f( y->vy, x->vy ) ;
	res->vz = atan2f( y->vz, x->vz ) ;
	res->vw = atan2f( y->vw, x->vw ) ;

#endif

}

