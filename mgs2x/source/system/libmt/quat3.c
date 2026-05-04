//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	quat3.c
	クォータニオン処理ルーチン（ＩＫ関連のための追加）

	2000/04/12 K.Takabe
	$Id: quat3.c,v 1.1.1.3 2002/11/19 11:42:53 Yoshizawa1 Exp $

*/
/*

	void MT_EulerToQuatZYX( FVECTOR *quat, FVECTOR *euler )
	FVECTOR		*quat ;	出力クォータニオン
	FVECTOR		*rot ;	入力回転ベクトル（オイラー角）

	オイラー角からクォータニオンへ（Ｘ、Ｙ、Ｚ順回転用）


	void MT_QuatToEulerZYX( FVECTOR *euler, FVECTOR *quat )
	FVECTOR		*euler ;	出力オイラー角
	FVECTOR		*quat ;		入力クォータニオン

	クォータニオンからオイラー角へ変換（Ｘ、Ｙ、Ｚ順回転用）
	（ほかから持ってきたソースを書き直しただけであっているかどうか保証なし）


	void MT_EulerToQuatYZX( FVECTOR *quat, FVECTOR *euler )
	FVECTOR		*quat ;		出力クォータニオン
	FVECTOR		*euler ;	入力オイラー角

	オイラー角からクォータニオンへ変換（Ｘ、Ｚ、Ｙ順回転用）
	（間違えて作ってしまった）


	void MT_QuatToEulerXZY( FVECTOR *euler, FVECTOR *quat )
	FVECTOR		*euler ;	出力オイラー角
	FVECTOR		*quat ;		入力クォータニオン

	クォータニオンからオイラー角へ変換（Ｙ、Ｚ、Ｘ順回転用）
	（関節ＩＫ用）


	void MT_EulerToQuatXZY( FVECTOR *quat, FVECTOR *euler )
	FVECTOR		*quat ;		出力クォータニオン
	FVECTOR		*euler ;	入力オイラー角

	オイラー角からクォータニオンへ変換（Ｙ、Ｚ、Ｘ順回転用）
	（関節ＩＫ用）


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
#include	"libdg.h"
#include	"libmt.h"


#ifdef M_PI
#undef M_PI
#endif
#define M_PI 3.14159265358979323846264338327950288419716939937510f

#define M_PI_H (M_PI/2)

#define DELTA 1e-6f     // error tolerance

/* ---------------------------------------------------------------- */
	/*
		オイラー角からクォータニオンへ（Ｘ、Ｙ、Ｚ順回転用）
	*/
void MT_EulerToQuatZYX( FVECTOR *quat, FVECTOR *rot)
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
		クォータニオンからオイラー角へ変換（Ｘ、Ｙ、Ｚ順回転用）
		（ほかから持ってきたソースを書き直しただけであっているかどうか保証なし）
	*/
void MT_QuatToEulerZYX( FVECTOR *euler, FVECTOR *quat )
{
	FMATRIX	matrix;
	FVECTOR	c_vec, s_vec ;
	float	div ;

	MT_QuatToMat( &matrix, quat );

	s_vec.vy = -matrix.m[2][0] ;
	c_vec.vy = DG_SQRT(1.0f - ( s_vec.vy * s_vec.vy ) ) ;

	/* ０除算回避 */
	if ( s_vec.vy != 1.0f && s_vec.vy != -1.0f ) {
		div = 1.0f / c_vec.vy ;
		c_vec.vx = matrix.m[2][2] * div ;
		s_vec.vx = matrix.m[2][1] * div ;

		c_vec.vz = matrix.m[0][0] * div ;
		s_vec.vz = matrix.m[1][0] * div ;
	} else {
		c_vec.vx = matrix.m[1][1] ;
		s_vec.vx = -matrix.m[1][2] ;

		c_vec.vz = 1.0f ;
		s_vec.vz = 0.0f ;
	}
	MT_Atan2X4( euler, &s_vec, &c_vec );
}

	/*
		オイラー角からクォータニオンへ変換（Ｘ、Ｚ、Ｙ順回転用）
		（間違えて作ってしまった）
	*/
void MT_EulerToQuatYZX( FVECTOR *quat, FVECTOR *rot)
{
	float cr, cp, cy, sr, sp, sy;
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

	quat->vw = cr * cp * cy - sr * sp * sy ;
	quat->vx = sr * cp * cy + cr * sp * sy ;
	quat->vy = sr * cp * sy + cr * sp * cy ;
	quat->vz = cr * cp * sy - sr * sp * cy ;
}

	/*
		クォータニオンからオイラー角へ変換（Ｙ、Ｚ、Ｘ順回転用）
		（関節ＩＫ用）
	*/
void MT_QuatToEulerXZY( FVECTOR *euler, FVECTOR *quat )
{
	FMATRIX	matrix;
	FVECTOR	c_vec, s_vec ;
	float	div ;

	MT_QuatToMat( &matrix, quat );

	s_vec.vz = -matrix.m[1][0] ;
	c_vec.vz = DG_SQRT( 1.0f - s_vec.vz * s_vec.vz ) ;

	/* ０除算回避 */
	if ( s_vec.vz != 1.0f && s_vec.vz != -1.0f ) {
		div = 1.0f / c_vec.vz ;
		s_vec.vx = matrix.m[1][2] ;
		c_vec.vx = matrix.m[1][1] ;

		s_vec.vy = matrix.m[2][0] ;
		c_vec.vy = matrix.m[0][0] ;
	} else {
		c_vec.vx = matrix.m[0][1] ;
		s_vec.vx = -matrix.m[2][1] ;

		c_vec.vy = 1.0f ;
		s_vec.vy = 0.0f ;
	}
	MT_Atan2X4( euler, &s_vec, &c_vec );
}

	/*
		オイラー角からクォータニオンへ変換（Ｙ、Ｚ、Ｘ順回転用）
		（関節ＩＫ用）
	*/
void MT_EulerToQuatXZY( FVECTOR *quat, FVECTOR *rot)
{
	float cr, cp, cy, sr, sp, sy;
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

	quat->vw = cr * cp * cy + sr * sp * sy ;
	quat->vx = sr * cp * cy - cr * sp * sy ;
	quat->vy = cr * sp * cy - sr * cp * sy ;
	quat->vz = cr * cp * sy + sr * sp * cy ;
}



