/*
	quat.c
	クォータニオン処理ルーチン

	1999/07/07 K.Takabe
	$Id: quat.c,v 1.1.1.3 2002/11/19 11:42:53 Yoshizawa1 Exp $

*/
/*

	void MT_QuatToMat( FMATRIX *mat, FVECTOR *quat )
	FMATRIX		*mat ;	出力マトリクス
	FVECTOR		*quat ;	入力クォータニオン

	クォータニオンから回転マトリクスへ


	void MT_EulerToQuatXYZ( FVECTOR *quat, FVECTOR *rot)
	FVECTOR		*quat ;	出力クォータニオン
	FVECTOR		*quat ;	入力回転ベクトル

	回転ベクトルからクォータニオンへ
	※使用禁止！　名前を統一させるためquat3.cのMT_EulerToQuatZYX()を使用すること！


	void MT_MatToQuat( FVECTOR *quat, FMATRIX *mat )
	FVECTOR		*quat ;	出力クォータニオン
	FMATRIX		*mat ;	入力マトリクス

	マトリクスからクォータニオンへ


	void MT_QuatSlerp( FVECTOR *res, FVECTOR *from, FVECTOR *to, float t )
	FVECTOR		*res ;	出力クォータニオン
	FVECTOR		*from ;	補間元クォータニオン
	FVECTOR		*to ;	補間先クォータニオン
	float		t ;		補間割合（0.0～1.0）	

	クォータニオンの球形線形補間（正規化しないので注意）


	void MT_QuatNormalize( FVECTOR *res, FVECTOR *quat )
	FVECTOR		*res ;	出力クォータニオン
	FVECTOR		*quat ;	入力クォータニオン

	クォータニオンの正規化


	void MT_QuatMul( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 )
	FVECTOR		*res ;	出力クォータニオン
	FVECTOR		*q1 ;	入力クォータニオン
	FVECTOR		*q2 ;	入力クォータニオン

	クォータニオンの乗算（正規化含む）

*/


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

#include	"libgv.h"
#include	"libdg.h"
#include	"libmt.h"


#ifdef M_PI
#undef M_PI
#endif
#define M_PI 3.14159265358979323846264338327950288419716939937510f

#define M_PI_H (M_PI/2)

#define DELTA 1e-6f     // error tolerance

/*----------------------------------------------------------------*/

	/*
		クォータニオンから回転マトリクスへ
	*/
void MT_QuatToMat( FMATRIX *mat, FVECTOR *quat )
{
#if 0
	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2 ;
	x2 = quat->vx + quat->vx ; y2 = quat->vy + quat->vy ; z2 = quat->vz + quat->vz ;
	xx = quat->vx * x2 ; xy = quat->vx * y2 ; xz = quat->vx * z2 ;
	yy = quat->vy * y2 ; yz = quat->vy * z2 ; zz = quat->vz * z2 ;
	wx = quat->vw * x2 ; wy = quat->vw * y2 ; wz = quat->vw * z2 ;

	mat->m[0][0] = 1.0F - (yy + zz) ;
	mat->m[1][0] = xy - wz ;
	mat->m[2][0] = xz + wy ;
	mat->m[3][0] = 0.0F ;
 
	mat->m[0][1] = xy + wz ;
	mat->m[1][1] = 1.0F - (xx + zz) ;
	mat->m[2][1] = yz - wx ;
	mat->m[3][1] = 0.0F ;

	mat->m[0][2] = xz - wy ;
	mat->m[1][2] = yz + wx ;
	mat->m[2][2] = 1.0F - (xx + yy) ;
	mat->m[3][2] = 0.0F ;

	mat->m[0][3] = 0.0F ;
	mat->m[1][3] = 0.0F ;
	mat->m[2][3] = 0.0F ;
	mat->m[3][3] = 1.0F ;
#else
#if 0
	/* VU命令を使った改訂版 
	   上のプログラムを移植
	   1999/11/19  K.Kano */

	asm volatile ("
	lqc2		vf16,0x00(%1)
	lqc2		vf20,0x00(%2)
	lqc2		vf21,0x10(%2)
	lqc2		vf22,0x20(%2)

	vopmula.xyz	ACC,vf16,vf16
	vmaddw.xyz	vf17,vf16,vf16w
	vmsubw.xyz	vf18,vf16,vf16w
	vmul.xyz	vf19,vf16,vf16
	lqc2		vf23,0x30(%2)
	vadd.xyz	vf17,vf17,vf17
	vadd.xyz	vf18,vf18,vf18
	vadd.xyz	vf19,vf19,vf19
	vmove.xyzw	vf24,vf20
	vmove.xyzw	vf25,vf21
	vmove.xyzw	vf26,vf22

	vmula.xyzw	ACC,vf20,vf20
	vmsubay.x	ACC,vf20,vf19y
	vmsubaz.x	ACC,vf20,vf19z
	vmaddaz.y	ACC,vf21,vf17z
	vmadday.z	ACC,vf22,vf18y
	vmaddx.xyzw	vf24,vf0,vf0

	vmula.xyzw	ACC,vf21,vf21
	vmsubax.y	ACC,vf21,vf19x
	vmsubaz.y	ACC,vf21,vf19z
	vmaddax.z	ACC,vf22,vf17x
	vmaddaz.x	ACC,vf20,vf18z
	vmaddx.xyzw	vf25,vf0,vf0

	vmula.xyzw	ACC,vf22,vf22
	vmsubax.z	ACC,vf22,vf19x
	vmsubay.z	ACC,vf22,vf19y
	vmadday.x	ACC,vf20,vf17y
	vmaddax.y	ACC,vf21,vf18x
	vmaddx.xyzw	vf26,vf0,vf0

	sqc2		vf23,0x30(%0)
	sqc2		vf24,0x00(%0)
	sqc2		vf25,0x10(%0)
	sqc2		vf26,0x20(%0)
	" : : "r"(mat),"r"(quat),"r"(&DG_UnitMatrix) : "memory" );
#else
	/* 最新のＥＥに対応した最適化済み */
	asm volatile ("
	lqc2		vf16,0x00(%1)

	vaddw.xyz	vf23,vf00,vf00	# vf23 = ( 1, 1, 1, ? )
	vmr32.xyzw	vf22,vf00
	vopmula.xyz	ACC ,vf16,vf16
	vmaddw.xyz	vf17,vf16,vf16w
	vmsubw.xyz	vf18,vf16,vf16w
	vmul.xyz	vf19,vf16,vf16
	vmr32.xyzw	vf21,vf22
	vadd.xyz	vf17,vf17,vf17
	vadd.xyz	vf18,vf18,vf18
	vadd.xyz	vf19,vf19,vf19
	vmr32.xyzw	vf20,vf21

	vmula.xyzw	ACC ,vf22,vf22
	vmadday.x	ACC ,vf23,vf17y
	vmaddax.y	ACC ,vf23,vf18x
	vmsubax.z	ACC ,vf23,vf19x
	vmsubay.z	ACC ,vf23,vf19y
	vmaddx.xyzw	vf26,vf00,vf00

	vmula.xyzw	ACC ,vf21,vf21
	vmsubax.y	ACC ,vf23,vf19x
	vmsubaz.y	ACC ,vf23,vf19z
	vmaddax.z	ACC ,vf23,vf17x
	vmaddaz.x	ACC ,vf23,vf18z
	vmaddx.xyzw	vf25,vf00,vf00

	vmula.xyzw	ACC ,vf20,vf20
	vmsubay.x	ACC ,vf23,vf19y
	vmsubaz.x	ACC ,vf23,vf19z
	vmaddaz.y	ACC ,vf23,vf17z
	vmadday.z	ACC ,vf23,vf18y
	vmaddx.xyzw	vf24,vf00,vf00

	sqc2		vf00,0x30(%0)
	sqc2		vf26,0x20(%0)
	sqc2		vf25,0x10(%0)
	sqc2		vf24,0x00(%0)
	" : : "r"(mat),"r"(quat) : "memory" );
#endif
#endif
}

	/*
		回転ベクトルからクォータニオンへ
	*/
void MT_EulerToQuatXYZ( FVECTOR *quat, FVECTOR *rot)
{
	float cr, cp, cy, sr, sp, sy, cpcy, spsy;
	FVECTOR	func_res, param ;

	param.vx = rot->vx / 2 ;
	param.vy = rot->vy / 2 ;
	param.vz = rot->vz / 2 ;
	MT_CosX4( &func_res, &param );
	cr = func_res.vx ;
	cp = func_res.vy ;
	cy = func_res.vz ;
	MT_SinX4( &func_res, &param );
	sr = func_res.vx ;
	sp = func_res.vy ;
	sy = func_res.vz ;

	cpcy = cp * cy;
	spsy = sp * sy;

	quat->vw = cr * cpcy + sr * spsy;
	quat->vx = sr * cpcy - cr * spsy;
	quat->vy = cr * sp * cy + sr * cp * sy;
	quat->vz = cr * cp * sy - sr * sp * cy;
}

	/*
		マトリクスからクォータニオンへ
	*/
void MT_MatToQuat( FVECTOR *quat, FMATRIX *mat )
{
	float	tr, s;
	float	q[4];
	int		i, j, k;
	int		nxt[3] = {1, 2, 0};

	tr = mat->m[0][0] + mat->m[1][1] + mat->m[2][2] ;

	/* 符号チェック */
	if ( tr > 0.0F ){
		s = sqrtf( tr + 1.0F );

		quat->vw = s / 2.0F ;

		s = 0.5F / s ;

		quat->vx = ( mat->m[1][2] - mat->m[2][1] ) * s ;
		quat->vy = ( mat->m[2][0] - mat->m[0][2] ) * s ;
		quat->vz = ( mat->m[0][1] - mat->m[1][0] ) * s ;
	} else {		
		i = 0;
		if ( mat->m[1][1] > mat->m[0][0] ) i = 1;
		if ( mat->m[2][2] > mat->m[i][i] ) i = 2;
		j = nxt[i];
		k = nxt[j];

		s = sqrtf( ( mat->m[i][i] - ( mat->m[j][j] + mat->m[k][k] ) ) + 1.0F );
      
		q[i] = s * 0.5F ;

		if ( s != 0.0F ) s = 0.5F / s ;

		q[3] = ( mat->m[j][k] - mat->m[k][j] ) * s ;
		q[j] = ( mat->m[i][j] + mat->m[j][i] ) * s ;
		q[k] = ( mat->m[i][k] + mat->m[k][i] ) * s ;

		quat->vx = q[0];
		quat->vy = q[1];
		quat->vz = q[2];
		quat->vw = q[3];
	}

}

	/*
		クォータニオンの球形線形補間（正規化しないので注意）
	*/
void MT_QuatSlerp( FVECTOR *res, FVECTOR *from, FVECTOR *to, float t )
{
	float	vx,vy,vz,vw ;
	float	omega, cosom, sinom ;	/* 本当はdouble型の方がいい */
	FVECTOR	func_res, param_x, param_y ;
	FVECTOR	scale_vec, to_vec ;

	vx = to->vx ;
	vy = to->vy ;
	vz = to->vz ;
	vw = to->vw ;
	/* 内積を求める */
	cosom = from->vx * vx + from->vy * vy + from->vz * vz + from->vw * vw ;

	/* 符号をそろえる */
	if ( cosom < 0.0F ){
		cosom = -cosom ;
		vx = - vx ;
		vy = - vy ;
		vz = - vz ;
		vw = - vw ;
	}
	to_vec.vx = vx ;
	to_vec.vy = vy ;
	to_vec.vz = vz ;
	to_vec.vw = vw ;

	/* 係数を求める */
	if ( ( 1.0F - cosom ) > (float)DELTA ){
		/* 通常処理（球形線形補間） */
		sinom = sqrtf( 1.0F - cosom * cosom );
		param_x.vx = sinom ;
		param_y.vx = cosom ;
		MT_Atan2X4( &func_res, &param_y, &param_x );
		omega = func_res.vx ;
		param_x.vx = ( 1.0F - t ) * omega ;
		param_x.vy = t * omega ;
		MT_SinX4( &func_res, &param_x );
		scale_vec.vx = func_res.vx / sinom ;
		scale_vec.vy = func_res.vy / sinom ;
	} else {
		/* ２つの角度の差が小さすぎるときには線形補間で求める */
		scale_vec.vx = 1.0F - t ;
		scale_vec.vy = t ;
	}
	asm("
		lqc2			vf4,0(%1)
		lqc2			vf6,0(%3)
		lqc2			vf5,0(%2)
		vmulax.xyzw		ACC,vf4,vf6
		vmaddy.xyzw		vf4,vf5,vf6
		sqc2			vf4,0(%0)
	"::"r"(res),"r"(from),"r"(&to_vec),"r"(&scale_vec):"memory" );

}

	/*
		クォータニオンの正規化
	*/
void MT_QuatNormalize( FVECTOR *res, FVECTOR *quat )
{
#if 0
    float	dist, square;
	square = quat->vx * quat->vx + quat->vy * quat->vy + quat->vz * quat->vz + quat->vw * quat->vw;
	if ( square > 0.0F ){
		dist = (float)( 1.0F / sqrtf( square ) );
		res->vx = quat->vx * dist ;
		res->vy = quat->vy * dist ;
		res->vz = quat->vz * dist ;
		res->vw = quat->vw * dist ;
	} else {
		*res = *quat ;
	}
#else
#if 1
	asm ( "
		lwc1		$f4,0(%1)
		lwc1		$f5,4(%1)
		lwc1		$f6,8(%1)
		lwc1		$f7,12(%1)
		mula.s		$f4,$f4
		madda.s		$f5,$f5
		madda.s		$f6,$f6
		madd.s		$f8,$f7,$f7
		rsqrt.s		$f8,%2,$f8
		mul.s		$f4,$f4,$f8
		mul.s		$f5,$f5,$f8
		mul.s		$f6,$f6,$f8
		mul.s		$f7,$f7,$f8
		swc1		$f4,0(%0)
		swc1		$f5,4(%0)
		swc1		$f6,8(%0)
		swc1		$f7,12(%0)
	"::"r"(res),"r"(quat),"f"(1.0F):"$f4","$f5","$f6","$f7","$f8","memory");
#else
	asm ( "
		lwc1		$f4,0(%1)
		lwc1		$f5,4(%1)
		lwc1		$f6,8(%1)
		lwc1		$f7,12(%1)
		mula.s		$f4,$f4
		madda.s		$f5,$f5
		madda.s		$f6,$f6
		madd.s		$f8,$f7,$f7
		sqrt.s		$f8,$f8
		div.s		$f8,%2,$f8
		mul.s		$f4,$f4,$f8
		mul.s		$f5,$f5,$f8
		mul.s		$f6,$f6,$f8
		mul.s		$f7,$f7,$f8
		swc1		$f4,0(%0)
		swc1		$f5,4(%0)
		swc1		$f6,8(%0)
		swc1		$f7,12(%0)
	"::"r"(res),"r"(quat),"f"(1.0F):"$f4","$f5","$f6","$f7","$f8","memory");
#endif
#endif
}

	/*
		クォータニオンの乗算（正規化付き）
	*/
void MT_QuatMul( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 )
{
#if 0
	asm ( "
		lqc2			vf04,0(%1)
		lqc2			vf05,0(%2)
		vmulx.xyzw		vf01,vf00,vf00
		vaddw.xyzw		vf01,vf01,vf00
		vmulx.xyzw		vf06,vf05,vf04
		vmuly.xyzw		vf07,vf05,vf04
		vmulz.xyzw		vf08,vf05,vf04
		vmulaw.xyzw		ACC ,vf05,vf04
		vmaddaw.x		ACC ,vf01,vf06
		vmaddaz.x		ACC ,vf01,vf07
		vmsuby.x		vf04,vf01,vf08
		vmaddaw.y		ACC ,vf01,vf07
		vmaddax.y		ACC ,vf01,vf08
		vmsubz.y		vf04,vf01,vf06
		vmaddaw.z		ACC ,vf01,vf08
		vmadday.z		ACC ,vf01,vf06
		vmsubx.z		vf04,vf01,vf07
		vmsubax.w		ACC ,vf01,vf06
		vmsubay.w		ACC ,vf01,vf07
		vmsubz.w		vf04,vf01,vf08
		sqc2			vf04,0(%0)
	"::"r"(res),"r"(q1),"r"(q2):"memory");
#else
	/* 最新ＥＥ対応 */
	asm ( "
		lqc2			vf04,0(%1)
		lqc2			vf05,0(%2)
		vaddw.xyz		vf01,vf00,vf00
		vmulx.xyzw		vf06,vf05,vf04
		vmuly.xyzw		vf07,vf05,vf04
		vmulz.xyzw		vf08,vf05,vf04
		vmulaw.xyzw		ACC ,vf05,vf04
		vmaddaw.x		ACC ,vf01,vf06
		vmaddaz.x		ACC ,vf01,vf07
		vmsuby.x		vf04,vf01,vf08
		vmaddaw.y		ACC ,vf01,vf07
		vmaddax.y		ACC ,vf01,vf08
		vmsubz.y		vf04,vf01,vf06
		vmaddaw.z		ACC ,vf01,vf08
		vmadday.z		ACC ,vf01,vf06
		vmsubx.z		vf04,vf01,vf07
		vmsubax.w		ACC ,vf00,vf06
		vmsubay.w		ACC ,vf00,vf07
		vmsubz.w		vf04,vf00,vf08
		sqc2			vf04,0(%0)
	"::"r"(res),"r"(q1),"r"(q2):"memory");
#endif

	/* 正規化をしておく */
	MT_QuatNormalize( res, res );

}


