//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	math_util.c
	エルミート補間関数

	2000/04/27 K.Takabe
	$Id: hermite.c,v 1.1.1.3 2002/11/19 11:42:51 Yoshizawa1 Exp $

*/
/*


	float MT_HermiteLerp( float q0, float q1, float d0, float d1, float t )
	float	q0 ;		補間元
	float	q1 ;		補間先
	float	d0 ;		補間元変化量（tに対して正規化しておくこと）
	float	d1 ;		補間先変化量（tに対して正規化しておくこと）
	float	t ;			補間パラメータ（0.0：補間元～1.0：補間先）

		エルミート補間関数


	void MT_HermiteLerpVec( FVECTOR *res, FVECTOR *q0, FVECTOR *q1, FVECTOR *d0, FVECTOR *d1, float t )
	FVECTOR	*res ;		計算結果
	FVECTOR	*q0 ;		補間元座標
	FVECTOR	*q1 ;		補間先座標
	FVECTOR	*d0 ;		補間元変化量（tに対して正規化しておくこと）
	FVECTOR	*d1 ;		補間先変化量（tに対して正規化しておくこと）
	float	t ;			補間パラメータ（0.0：補間元～1.0：補間先）

		エルミート補間関数
		ベクトル成分のＸＹＺＷ全てについて補間計算を行なう


	void MT_HermiteLerpVecScale( *res, *q0, *q1, *d0, *d1, t, scale )
	FVECTOR	*res ;		計算結果
	FVECTOR	*q0 ;		補間元座標
	FVECTOR	*q1 ;		補間先座標
	FVECTOR	*d0 ;		補間元変化量（t/scaleに対して正規化しておくこと）
	FVECTOR	*d1 ;		補間先変化量（t/scaleに対して正規化しておくこと）
	float	t ;			補間パラメータ（0.0：補間元～1.0：補間先）
	float	scale ;		変化量のスケーリング値

		エルミート補間関数（変化量のスケール指定あり）
		モーション再生処理で使用する。

*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif // PSX2

#ifdef KP_XBOX
#include <xtl.h>
#endif	// KP_XBOX

#include	"libgv.h"
#include	"libmt.h"

#include "bp_matrix.h"

#if 0
static FMATRIX c_mat = {{
	{ 2.0f,-3.0f, 0.0f, 1.0f},
	{-2.0f, 3.0f, 0.0f, 0.0f},
	{ 1.0f,-2.0f, 1.0f, 0.0f},
	{ 1.0f,-1.0f, 0.0f, 0.0f}
}};
#endif

#ifdef PSX2
static FMATRIX c_mat2 = {{
	{ 2.0f,-2.0f, 1.0f, 1.0f},
	{-3.0f, 3.0f,-2.0f,-1.0f},
	{ 0.0f, 0.0f, 1.0f, 0.0f},
	{ 1.0f, 0.0f, 0.0f, 0.0f}
}};
static FMATRIX dc_mat2 = {{
	{ 0.0f, 2.0f*3,-2.0f*2, 1.0f },
	{ 0.0f,-3.0f*3, 3.0f*2,-2.0f },
	{ 0.0f, 0.0f*3, 0.0f*2, 1.0f },
	{ 0.0f, 1.0f*3, 0.0f*2, 0.0f }
}};
#else
static FMATRIX c_mat2 = {
	 2.0f,-2.0f, 1.0f, 1.0f,
	-3.0f, 3.0f,-2.0f,-1.0f,
	 0.0f, 0.0f, 1.0f, 0.0f,
	 1.0f, 0.0f, 0.0f, 0.0f
};
static FMATRIX dc_mat2 = {
	 0.0f, 2.0f*3,-2.0f*2, 1.0f ,
	 0.0f,-3.0f*3, 3.0f*2,-2.0f ,
	 0.0f, 0.0f*3, 0.0f*2, 1.0f ,
	 0.0f, 1.0f*3, 0.0f*2, 0.0f 
};
#endif

/* ---------------------------------------------------------------- */
	/*
		エルミート補間関数
	*/
float MT_HermiteLerp( float q0, float q1, float d0, float d1, float t )
{
	float	res ;

	/* アセンブラで書き直せばかなり速くなるかも */
	res = ( ( ( t - 1 ) * ( t - 1 ) ) * ( 2 * t + 1 ) ) * q0 +
	  ( ( t * t ) * ( 3 - 2 * t ) ) * q1 +
		( ( 1 - t ) * ( 1 - t ) * t ) * d0 +
		  ( ( t - 1 ) * ( t * t ) ) * d1 ;

	return ( res );
}

	/*
		エルミート補間関数
	*/
void MT_HermiteLerpVec( FVECTOR *res, FVECTOR *q0, FVECTOR *q1, FVECTOR *d0, FVECTOR *d1, float t )
{
#if 0 //BP_PS2
//#ifdef PSX2
	asm("
		mfc1			$4,%6			# 補間パラメータの読み込み
		lqc2			vf04,0x00(%1)	# 定数マトリクス読み込み
		lqc2			vf05,0x10(%1)	# 定数マトリクス読み込み
		lqc2			vf06,0x20(%1)	# 定数マトリクス読み込み
		lqc2			vf07,0x30(%1)	# 定数マトリクス読み込み
		qmtc2			$4,vf01			# 補間パラメータの読み込み
		lqc2			vf08,0x00(%2)	# 補間座標０読み込み
		lqc2			vf09,0x00(%3)	# 補間座標１読み込み
		lqc2			vf10,0x00(%4)	# 微分成分０の読み込み
		lqc2			vf11,0x00(%5)	# 微分成分１の読み込み
		vmulax.xyzw		ACC ,vf08,vf04
		vmadday.xyzw	ACC ,vf09,vf04
		vmaddaz.xyzw	ACC ,vf10,vf04
		vmaddw.xyzw		vf04,vf11,vf04
		vaddx.xyz		vf02,vf00,vf01	# tvec make
		vmulax.xyzw		ACC ,vf08,vf05
		vmadday.xyzw	ACC ,vf09,vf05
		vmaddaz.xyzw	ACC ,vf10,vf05
		vmaddw.xyzw		vf05,vf11,vf05
		vmulx.xy		vf02,vf02,vf01	# tvec make
		vmulax.xyzw		ACC ,vf08,vf06
		vmadday.xyzw	ACC ,vf09,vf06
		vmaddaz.xyzw	ACC ,vf10,vf06
		vmaddw.xyzw		vf06,vf11,vf06
		vmulx.x			vf02,vf02,vf01	# tvec make (t^3,t^2,t,?)
		vmulax.xyzw		ACC ,vf08,vf07
		vmadday.xyzw	ACC ,vf09,vf07
		vmaddaz.xyzw	ACC ,vf10,vf07
		vmaddw.xyzw		vf07,vf11,vf07
		vmulax.xyzw		ACC ,vf04,vf02
		vmadday.xyzw	ACC ,vf05,vf02
		vmaddaz.xyzw	ACC ,vf06,vf02
		vmaddw.xyzw		vf02,vf07,vf00
		sqc2			vf02,0x00(%0)
	"::"r"(res),"r"(&c_mat2),"r"(q0),"r"(q1),"r"(d0),"r"(d1),"f"(t):"$4");
#else
	FMATRIX	data_mat ;
	FVECTOR	tvec ;

	tvec.vx = t * t * t ;
	tvec.vy = t * t ;
	tvec.vz = t ;
	tvec.vw = 1.0f ;
	*(FVECTOR*)data_mat.m[0] = *q0 ;
	*(FVECTOR*)data_mat.m[1] = *q1 ;
	*(FVECTOR*)data_mat.m[2] = *d0 ;
	*(FVECTOR*)data_mat.m[3] = *d1 ;
	_sceVu0MulMatrix( &data_mat, &data_mat, &c_mat2 );
	_sceVu0ApplyMatrix( res, &data_mat, &tvec );
#endif	
}

	/*
		エルミート補間関数（変化量のスケール指定あり）
	*/
void MT_HermiteLerpVecScale( FVECTOR *res, FVECTOR *q0, FVECTOR *q1, FVECTOR *d0, FVECTOR *d1, float t, float scale )
{
#if 0 //BP_PS2
//#ifdef PSX2
	asm("
		mfc1			$4,%7			# スケール値の読み込み
		lqc2			vf10,0x00(%4)	# 微分成分０の読み込み
		qmtc2			$4,vf31			# スケール値の読み込み
		lqc2			vf11,0x00(%5)	# 微分成分１の読み込み
		lqc2			vf04,0x00(%1)	# 定数マトリクス読み込み
		lqc2			vf05,0x10(%1)	# 定数マトリクス読み込み
		lqc2			vf06,0x20(%1)	# 定数マトリクス読み込み
		lqc2			vf07,0x30(%1)	# 定数マトリクス読み込み
		vmulx.xyzw		vf10,vf10,vf31	# 微分成分０のスケーリング
		vmulx.xyzw		vf11,vf11,vf31	# 微分成分１のスケーリング
		lqc2			vf08,0x00(%2)	# 補間座標０読み込み
		mfc1			$4,%6			# 補間パラメータの読み込み
		lqc2			vf09,0x00(%3)	# 補間座標１読み込み
		qmtc2			$4,vf01			# 補間パラメータ値の読み込み
		vmulax.xyzw		ACC ,vf08,vf04
		vmadday.xyzw	ACC ,vf09,vf04
		vmaddaz.xyzw	ACC ,vf10,vf04
		vmaddw.xyzw		vf04,vf11,vf04
		vaddx.xyz		vf02,vf00,vf01	# tvec make
		vmulax.xyzw		ACC ,vf08,vf05
		vmadday.xyzw	ACC ,vf09,vf05
		vmaddaz.xyzw	ACC ,vf10,vf05
		vmaddw.xyzw		vf05,vf11,vf05
		vmulx.xy		vf02,vf02,vf01	# tvec make
		vmulax.xyzw		ACC ,vf08,vf06
		vmadday.xyzw	ACC ,vf09,vf06
		vmaddaz.xyzw	ACC ,vf10,vf06
		vmaddw.xyzw		vf06,vf11,vf06
		vmulx.x			vf02,vf02,vf01	# tvec make (t^3,t^2,t,?)
		vmulax.xyzw		ACC ,vf08,vf07
		vmadday.xyzw	ACC ,vf09,vf07
		vmaddaz.xyzw	ACC ,vf10,vf07
		vmaddw.xyzw		vf07,vf11,vf07
		vmulax.xyzw		ACC ,vf04,vf02
		vmadday.xyzw	ACC ,vf05,vf02
		vmaddaz.xyzw	ACC ,vf06,vf02
		vmaddw.xyzw		vf02,vf07,vf00
		sqc2			vf02,0x00(%0)
	"::"r"(res),"r"(&c_mat2),"r"(q0),"r"(q1),"r"(d0),"r"(d1),"f"(t),"f"(scale):"$4");
#else
	FMATRIX	data_mat ;
	FVECTOR	tvec ;

	tvec.vx = t * t * t ;
	tvec.vy = t * t ;
	tvec.vz = t ;
	tvec.vw = 1.0f ;
	*(FVECTOR*)data_mat.m[0] = *q0 ;
	*(FVECTOR*)data_mat.m[1] = *q1 ;
	*(FVECTOR*)data_mat.m[2] = *d0 ;
	*(FVECTOR*)data_mat.m[3] = *d1 ;
	data_mat.m[2][0] *= scale ;
	data_mat.m[2][1] *= scale ;
	data_mat.m[2][2] *= scale ;
	data_mat.m[2][3] *= scale ;
	data_mat.m[3][0] *= scale ;
	data_mat.m[3][1] *= scale ;
	data_mat.m[3][2] *= scale ;
	data_mat.m[3][3] *= scale ;
	_sceVu0MulMatrix( &data_mat, &data_mat, &c_mat2 );
	_sceVu0ApplyMatrix( res, &data_mat, &tvec );
#endif	
	
}

/* ---------------------------------------------------------------- */
	/*
		エルミート補間関数（微分）
	*/
void MT_DHermiteLerpVec( FVECTOR *res, FVECTOR *q0, FVECTOR *q1, FVECTOR *d0, FVECTOR *d1, float t )
{
#if 0 //BP_PS2
//#ifdef PSX2
	asm("
		mfc1			$4,%6			# 補間パラメータの読み込み
		lqc2			vf04,0x00(%1)	# 定数マトリクス読み込み
		lqc2			vf05,0x10(%1)	# 定数マトリクス読み込み
		lqc2			vf06,0x20(%1)	# 定数マトリクス読み込み
		lqc2			vf07,0x30(%1)	# 定数マトリクス読み込み
		qmtc2			$4,vf01			# 補間パラメータの読み込み
		lqc2			vf08,0x00(%2)	# 補間座標０読み込み
		lqc2			vf09,0x00(%3)	# 補間座標１読み込み
		lqc2			vf10,0x00(%4)	# 微分成分０の読み込み
		lqc2			vf11,0x00(%5)	# 微分成分１の読み込み
		vmulax.xyzw		ACC ,vf08,vf04
		vmadday.xyzw	ACC ,vf09,vf04
		vmaddaz.xyzw	ACC ,vf10,vf04
		vmaddw.xyzw		vf04,vf11,vf04
		vaddx.xyz		vf02,vf00,vf01	# tvec make
		vmulax.xyzw		ACC ,vf08,vf05
		vmadday.xyzw	ACC ,vf09,vf05
		vmaddaz.xyzw	ACC ,vf10,vf05
		vmaddw.xyzw		vf05,vf11,vf05
		vmulx.xy		vf02,vf02,vf01	# tvec make
		vmulax.xyzw		ACC ,vf08,vf06
		vmadday.xyzw	ACC ,vf09,vf06
		vmaddaz.xyzw	ACC ,vf10,vf06
		vmaddw.xyzw		vf06,vf11,vf06
		vmulx.x			vf02,vf02,vf01	# tvec make (t^3,t^2,t,?)
		vmulax.xyzw		ACC ,vf08,vf07
		vmadday.xyzw	ACC ,vf09,vf07
		vmaddaz.xyzw	ACC ,vf10,vf07
		vmaddw.xyzw		vf07,vf11,vf07
		vmulax.xyzw		ACC ,vf04,vf02
		vmadday.xyzw	ACC ,vf05,vf02
		vmaddaz.xyzw	ACC ,vf06,vf02
		vmaddw.xyzw		vf02,vf07,vf00
		sqc2			vf02,0x00(%0)
	"::"r"(res),"r"(&dc_mat2),"r"(q0),"r"(q1),"r"(d0),"r"(d1),"f"(t):"$4");
#else
	FMATRIX	data_mat ;
	FVECTOR	tvec ;

	tvec.vx = t * t * t ;
	tvec.vy = t * t ;
	tvec.vz = t ;
	tvec.vw = 1.0f ;
	*(FVECTOR*)data_mat.m[0] = *q0 ;
	*(FVECTOR*)data_mat.m[1] = *q1 ;
	*(FVECTOR*)data_mat.m[2] = *d0 ;
	*(FVECTOR*)data_mat.m[3] = *d1 ;
	_sceVu0MulMatrix( &data_mat, &data_mat, &dc_mat2 );
	_sceVu0ApplyMatrix( res, &data_mat, &tvec );
#endif	
}
