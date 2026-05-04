/*
	クォータニオン処理ルーチン

	void MT_QuatToMat( FMATRIX *mat, FVECTOR *quat )
	FMATRIX		*mat ;	出力マトリクス
	FVECTOR		*quat ;	入力クォータニオン

	クォータニオンから回転マトリクスへ


	void MT_EulerToQuatXYZ( FVECTOR *quat, FVECTOR *rot)
	FVECTOR		*quat ;	出力クォータニオン
	FVECTOR		*quat ;	入力回転ベクトル

	回転ベクトルからクォータニオンへ


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


	void MT_QuatGetValue( FVECTOR *res, FVECTOR *quat )
	FVECTOR		*res ;	回転軸ベクトル＆回転角度（ラジアン）
	FVECTOR		*quat ;	入力クォータニオン

	クォータニオンから軸、回転量を取得する


	void MT_QuatSetValue( FVECTOR *quat, FVECTOR *param )
	FVECTOR		*quat ;	入力クォータニオン
	FVECTOR		*param ;	回転軸ベクトル＆回転角度（ラジアン）

	軸、回転量からクォータニオンを取得する


	void MT_QuatScaleAngle(FVECTOR *res, FVECTOR * quat, float scale )
	FVECTOR		*res ;	出力クォータニオン
	FVECTOR		*quat ;	入力クォータニオン
	float		scale ; スケール値

	軸回転量にスケール値をかける


	void MT_QuatInverse( FVECTOR *res, FVECTOR *quat )
	FVECTOR		*res ;	出力クォータニオン
	FVECTOR		*quat ;	入力クォータニオン

	逆クォータニオンを求める（q^(-1)）


	void MT_QuatSetFromAx( FVECTOR *quat, FVECTOR *from, FVECTOR *to )
	FVECTOR		*res ;	出力クォータニオン
	FVECTOR		*from ;	回転元方向ベクトル
	FVECTOR		*to ;	回転先方向ベクトル

	方向fromから方向toを向くクォータニオンを求める


	void MT_QuatAdd( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 )
	FVECTOR		*res ;	出力クォータニオン
	FVECTOR		*q1 ;	入力クォータニオン
	FVECTOR		*q2 ;	入力クォータニオン

	クォータニオンの加算（正規化付き）


	void MT_QuatSub( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 )
	FVECTOR		*res ;	出力クォータニオン
	FVECTOR		*q1 ;	入力クォータニオン
	FVECTOR		*q2 ;	入力クォータニオン

	クォータニオンの減算（正規化付き）


	void MT_QuatDiv( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 )
	FVECTOR		*res ;	出力クォータニオン
	FVECTOR		*q1 ;	入力クォータニオン
	FVECTOR		*q2 ;	入力クォータニオン

	クォータニオンの除算（正規化付き）


	void MT_QuatSquare( FVECTOR *res, FVECTOR *q1 )
	FVECTOR		*res ;	出力クォータニオン
	FVECTOR		*q1 ;	入力クォータニオン

	クォータニオンの二乗（q*q）


	void MT_QuatSqrt( FVECTOR *res, FVECTOR *q1 )
	FVECTOR		*res ;	出力クォータニオン
	FVECTOR		*q1 ;	入力クォータニオン

	クォータニオンのルート（sqrt(q)）


	float MT_QuatDot( FVECTOR *q1, FVECTOR *q2 )
	FVECTOR		*q1 ;	入力クォータニオン
	FVECTOR		*q2 ;	入力クォータニオン

	クォータニオンの内積を求める


	float MT_QuatLength( FVECTOR *q1 )
	FVECTOR		*q1 ;	入力クォータニオン

	クォータニオンの長さを求める


	void MT_QuatNegate( FVECTOR *res, FVECTOR *quat )
	FVECTOR		*res ;	出力クォータニオン
	FVECTOR		*quat ;	入力クォータニオン

	クォータニオンの反転（正規化付き）


	void MT_QuatExp( FVECTOR *res, FVECTOR *q1 )
	FVECTOR		*res ;	出力クォータニオン
	FVECTOR		*q1 ;	入力クォータニオン

	クォータニオンの指数を求める（exp(p)）


	void MT_QuatLog( FVECTOR *res, FVECTOR *q1 )
	FVECTOR		*res ;	出力クォータニオン
	FVECTOR		*q1 ;	入力クォータニオン

	クォータニオンの自然対数を求める（log(p)）


	void MT_QuatLnDif( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 )
	FVECTOR		*res ;	出力クォータニオン
	FVECTOR		*q1 ;	入力クォータニオン
	FVECTOR		*q2 ;	入力クォータニオン

	自然対数の差を求める


	void MT_QuatSquad( FVECTOR *res, FVECTOR *p, FVECTOR *a, FVECTOR *b, FVECTOR *q, float t )
	FVECTOR		*res ;	出力クォータニオン
	FVECTOR		*p ;	補間元クォータニオン
	FVECTOR		*a ;	補間元に対する補間定数クォータニオン
	FVECTOR		*q ;	補間先クォータニオン
	FVECTOR		*b ;	補間先に対する補間定数クォータニオン
	float		t ;		補間割合（0.0～1.0）	

	クォータニオンの二次補間を求める
	Squad(p,a,b,q; t) = Slerp(Slerp(p,q;t), Slerp(a,b;t); 2(1-t)t).


	void MT_QuatQCompA( FVECTOR *res, FVECTOR *q_prev, FVECTOR *q, FVECTOR *q_next )
	FVECTOR		*res ;		出力クォータニオン
	FVECTOR		*q_prev ;	前クォータニオン
	FVECTOR		*q ;		定数算出対象クォータニオン
	FVECTOR		*q_next ;	後クォータニオン

	クォータニオンの二次補間定数を求める
	Compute a, the term used in Boehm-type interpolation.
	a[n] = q[n]* qexp(-(1/4)*( ln(qinv(q[n])*q[n+1]) +ln( qinv(q[n])*q[n-1] )))


*/


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include	"quat.h"

#define M_PI 3.14159265358979323846264338327950288419716939937510f

#define DELTA 1e-6f     // error tolerance

#if 1
/* ＰＣ等への移植用 */
#define cosf(n) cos(n)
#define sinf(n) sin(n)
#define tanf(n) tan(n)
#define acosf(n) acos(n)
#define asinf(n) asin(n)
#define atanf(n) atan(n)
#define atan2f(n,m) atan2(n,m)
#define sqrtf(m) sqrt(m)
#endif


/*----------------------------------------------------------------*/
	/*
		クォータニオンから回転マトリクスへ
	*/
void MT_QuatToMat( FMATRIX *mat, FVECTOR *quat )
{
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
}

	/*
		回転ベクトルからクォータニオンへ
	*/
void MT_EulerToQuatXYZ( FVECTOR *quat, FVECTOR *rot)
{
	float cr, cp, cy, sr, sp, sy, cpcy, spsy;

	cr = cosf( rot->vx / 2 );
	cp = cosf( rot->vy / 2 );
	cy = cosf( rot->vz / 2 );

	sr = sinf( rot->vx / 2 );
	sp = sinf( rot->vy / 2 );
	sy = sinf( rot->vz / 2 );

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
	float	to1[4] ;
	float	omega, cosom, sinom ;	/* 本当はdouble型の方がいい */
	float	scale0, scale1 ;		/* 本当はdouble型の方がいい */

	/* 内積を求める */
	cosom = from->vx * to->vx + from->vy * to->vy + from->vz * to->vz + from->vw * to->vw ;

	/* 符号をそろえる */
	if ( cosom < 0.0F ){
		cosom = -cosom ;
		to1[0] = - to->vx ;
		to1[1] = - to->vy ;
		to1[2] = - to->vz ;
		to1[3] = - to->vw ;
	} else  {
		to1[0] = to->vx ;
		to1[1] = to->vy ;
		to1[2] = to->vz ;
		to1[3] = to->vw ;
	}

	/* 係数を求める */
	if ( ( 1.0F - cosom ) > DELTA ){
		/* 通常処理（球形線形補間） */
#if 1
		omega = acosf( cosom );
		sinom = sinf( omega );
#else
		sinom = sqrtf( 1.0F - cosom * cosom );
		omega = atan2f( sinom, cosom );
#endif
		scale0 = sinf( ( 1.0F - t ) * omega ) / sinom ;
		scale1 = sinf( t * omega) / sinom ;
	} else {
		/* ２つの角度の差が小さすぎるときには線形補間で求める */
		scale0 = 1.0F - t ;
		scale1 = t ;
	}

	/* 係数を使って計算 */
	to1[0] = scale0 * from->vx + scale1 * to1[0] ;
	to1[1] = scale0 * from->vy + scale1 * to1[1] ;
	to1[2] = scale0 * from->vz + scale1 * to1[2] ;
	to1[3] = scale0 * from->vw + scale1 * to1[3] ;
	res->vx = to1[0] ;
	res->vy = to1[1] ;
	res->vz = to1[2] ;
	res->vw = to1[3] ;

}

	/*
		クォータニオンの正規化
	*/
void MT_QuatNormalize( FVECTOR *res, FVECTOR *quat )
{
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
}

	/*
		クォータニオンの乗算（正規化付き）
	*/
void MT_QuatMul( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 )
{
	FVECTOR	r ;
	r.vx = q1->vw * q2->vx + q1->vx * q2->vw + q1->vy * q2->vz - q1->vz * q2->vy ;
	r.vy = q1->vw * q2->vy + q1->vy * q2->vw + q1->vz * q2->vx - q1->vx * q2->vz ;
	r.vz = q1->vw * q2->vz + q1->vz * q2->vw + q1->vx * q2->vy - q1->vy * q2->vx ;
	r.vw = q1->vw * q2->vw - q1->vx * q2->vx - q1->vy * q2->vy - q1->vz * q2->vz ;

	/* 正規化をしておく */
	MT_QuatNormalize( res, &r );

}

/*----------------------------------------------------------------*/
	/*
		おまけルーチン
		以下は実際にはほとんど使用しないと思われるので最適化や
		動作確認は行っていません。
	*/


	/*
		クォータニオンから軸、回転量を取得する
	*/
void MT_QuatGetValue( FVECTOR *res, FVECTOR *quat )
{
	float	len ;
	float	tx, ty, tz ;

	tx = quat->vx ;
	ty = quat->vy ;
	tz = quat->vz ;
	len = tx * tx + ty * ty + tz * tz ;

    if (len > DELTA){
		res->vx = tx * ( 1.0F / len );
		res->vy = ty * ( 1.0F / len );
		res->vz = tz * ( 1.0F / len );
	    res->vw = (float)( 2.0F * acosf( quat->vw ) );
    } else {
		res->vx = 0.0F ;
		res->vy = 0.0F ;
		res->vz = 1.0F ;
	    res->vw = 0.0F ;
    }
}

	/*
		軸、回転量からクォータニオンを取得する
	*/
void MT_QuatSetValue( FVECTOR *quat, FVECTOR *param )
{
	float	temp, dist ;

	temp = param->vx * param->vx + param->vy * param->vy + param->vz * param->vz ;

	quat->vw = (float)cosf( param->vw / 2.0F );
	dist = sqrtf( ( 1.0f - quat->vw * quat->vw ) / temp ) ;
	if ( param->vw < 0.0f ) dist = -dist ;
	quat->vx = param->vx * dist ;
	quat->vy = param->vy * dist ;
	quat->vz = param->vz * dist ;
	
}

	/*
		軸回転量にスケール値をかける
	*/
void MT_QuatScaleAngle(FVECTOR *res, FVECTOR * quat, float scale )
{
	FVECTOR	param ;

	MT_QuatGetValue( &param, quat );
	param.vw = param.vw * scale ;
	MT_QuatSetValue( res, &param );
}

	/*
		逆クォータニオンを求める（q^(-1)）
	*/
void MT_QuatInverse( FVECTOR *res, FVECTOR *quat )
{
	float norm, invNorm;

	norm = quat->vx * quat->vx + quat->vy * quat->vy + quat->vz * quat->vz + quat->vw * quat->vw ;
	if ( norm > 0.0f ){
		invNorm = 1.0F / norm ;
		res->vx = -quat->vx * invNorm;
		res->vy = -quat->vy * invNorm;
		res->vz = -quat->vz * invNorm;
		res->vw =  quat->vw * invNorm;
	}

}

	/*
		方向fromから方向toを向くクォータニオンを求める
	*/
void MT_QuatSetFromAx( FVECTOR *quat, FVECTOR *from, FVECTOR *to )
{
	float tx, ty, tz, temp, dist;
	float	cost, len, ss;

	/* 内積を求める */
	cost = from->vx * to->vx + from->vy * to->vy + from->vz * to->vz ;

	/* 例外条件のチェック */
	if ( cost > 0.99999f ){
		quat->vx = quat->vy = quat->vz = 0.0F ;
		quat->vw = 1.0F ;
		return;
	} else if ( cost < -0.99999f ){
		/* check if we can use cross product of from vector with [1, 0, 0] */
		tx = 0.0F ;
		ty = from->vx ;
		tz = -from->vy ;
		len = sqrtf( ty * ty + tz * tz );
		if ( len < DELTA ){
			/* nope! we need cross product of from vector with [0, 1, 0] */
			tx = -from->vz ;
			ty = 0.0F ;
			tz = from->vx ;
		}

		temp = tx * tx + ty * ty + tz * tz ;
		dist = (float)( 1.0F / sqrtf( temp ) ) ;
		tx *= dist ;
		ty *= dist ;
		tz *= dist ;
	
		quat->vx = tx;
		quat->vy = ty;
		quat->vz = tz;
		quat->vw = 0.0F ;
		return;
	}

	/* ２つのベクトルが直交しない場合 */
	tx = from->vy * to->vz - from->vz * to->vy ;
	ty = from->vz * to->vx - from->vx * to->vz ;
	tz = from->vx * to->vy - from->vy * to->vx ;

	temp = tx * tx + ty * ty + tz * tz ;
	dist = (float)( 1.0F / sqrtf(temp) );
	tx *= dist;
	ty *= dist;
	tz *= dist;

	/* we have to use half-angle formulae (sin^2 t = ( 1 - cos (2t) ) /2) */
	ss = (float)sqrtf( 0.5F * ( 1.0F - cost ) );
	tx *= ss;
	ty *= ss;
	tz *= ss;

	/* scale the axis to get the normalized quaternion */
	quat->vx = tx;
	quat->vy = ty;
	quat->vz = tz;

	/* cos^2 t = ( 1 + cos (2t) ) / 2 */
	/* w part is cosine of half the rotation angle */
	quat->vw = (float)sqrtf( 0.5F * ( 1.0F + cost ) );

}

	/*
		クォータニオンの加算（正規化付き）
	*/
void MT_QuatAdd( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 )
{
	res->vx = q1->vx + q2->vx;
	res->vy = q1->vy + q2->vy;
	res->vz = q1->vz + q2->vz;
	res->vw = q1->vw + q2->vw;
	MT_QuatNormalize( res, res );
}

	/*
		クォータニオンの減算（正規化付き）
	*/
void MT_QuatSub( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 )
{
	res->vx = q1->vx - q2->vx;
	res->vy = q1->vy - q2->vy;
	res->vz = q1->vz - q2->vz;
	res->vw = q1->vw - q2->vw;
	MT_QuatNormalize( res, res );
}

	/*
		クォータニオンの除算（正規化付き）
	*/
void MT_QuatDiv( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 )
{
	FVECTOR	q, r, s ;
	float	dist ;

	q = *q2 ;

	/* 軸ベクトル反転 */
    q.vx = -q.vx;
    q.vy = -q.vy;
    q.vz = -q.vz;
	MT_QuatMul( &r, q1, &q );
	MT_QuatMul( &s, &q, &q );

	dist = 1.0F / s.vw ;
	res->vx = r.vx * dist ;
	res->vy = r.vy * dist ;
	res->vz = r.vz * dist ;
	res->vw = r.vw * dist ;
}

	/*
		クォータニオンの二乗（q*q）
	*/
void MT_QuatSquare( FVECTOR *res, FVECTOR *q1 )
{
	float  tt;

	tt = 2 * q1->vw;
	res->vx = tt * q1->vx;
	res->vy = tt * q1->vy;
	res->vz = tt * q1->vz;
	res->vw = q1->vw * q1->vw - q1->vx * q1->vx - q1->vy * q1->vy - q1->vz * q1->vz ;
}

	/*
		クォータニオンのルート（sqrt(q)）
	*/
void MT_QuatSqrt( FVECTOR *res, FVECTOR *q1 )
{
	float	length, m, r1, r2;
	FVECTOR	r;

	length = sqrtf( q1->vw * q1->vw + q1->vx * q1->vx + q1->vy * q1->vy );
	if ( length != 0.0F ){
		length = 1.0F / length ;
	} else {
		length = 1.0F ;
	}

	r.vx = q1->vx * length ;
	r.vy = q1->vz * length ;
	r.vz = 0.0F ;
	r.vw = q1->vw * length ;

	m = 1.0F / sqrtf( r.vw * r.vw + r.vx * r.vx );
	r1 = sqrtf( ( 1.0F + r.vy ) * 0.5F );
	r2 = sqrtf( ( 1.0F - r.vy ) * 0.5F );

	res->vx = sqrtf(length) * r2 * r.vx * m ;
	res->vy = sqrtf(length) * r1 ;
	res->vz = q1->vz ;
	res->vw = sqrtf(length) * r1 * r.vw * m ;

}

	/*
		クォータニオンの内積を求める
	*/
float MT_QuatDot( FVECTOR *q1, FVECTOR *q2 )
{
	FVECTOR	_q1, _q2 ;
	if ( q1->vw < 0.0f ){
		_q1.vw = -q1->vw ;
		_q1.vx = -q1->vx ;
		_q1.vy = -q1->vy ;
		_q1.vz = -q1->vz ;
		q1 = &_q1 ;
	}
	if ( q2->vw < 0.0f ){
		_q2.vw = -q2->vw ;
		_q2.vx = -q2->vx ;
		_q2.vy = -q2->vy ;
		_q2.vz = -q2->vz ;
		q2 = &_q2 ;
	}
	return (float)( q1->vw * q2->vw + q1->vx * q2->vx + q1->vy * q2->vy + q1->vz * q2->vz );
}

	/*
		クォータニオンの長さを求める
	*/
float MT_QuatLength( FVECTOR *q1 )
{
	return sqrtf( q1->vw * q1->vw + q1->vx * q1->vx + q1->vy * q1->vy + q1->vz * q1->vz );
}

	/*
		クォータニオンの反転（正規化付き）
	*/
void MT_QuatNegate( FVECTOR *res, FVECTOR *quat )
{
	*res = *quat ;
	MT_QuatNormalize( res, res );
	res->vx = -res->vx;
	res->vy = -res->vy;
	res->vz = -res->vz;
}

	/*
		クォータニオンの指数を求める（exp(p)）
	*/
void MT_QuatExp( FVECTOR *res, FVECTOR *q1 )
{
	float  len1, len2;

	len1 = (float) sqrtf( q1->vx * q1->vx + q1->vy * q1->vy + q1->vz * q1->vz );
	if ( len1 > DELTA ){
		len2 = (float)sinf( len1 ) / len1; 
	} else {
		len2 = 1.0F ;
	}

	res->vx = q1->vx * len2 ;
	res->vy = q1->vy * len2 ;
	res->vz = q1->vz * len2 ;
	res->vw = cosf( len1 );
}

	/*
		クォータニオンの自然対数を求める（log(p)）
	*/
void MT_QuatLog( FVECTOR *res, FVECTOR *q1 )
{
#if 1
	float  length;

	length = sqrtf( q1->vx * q1->vx + q1->vy * q1->vy + q1->vz * q1->vz );

	/* make sure we do not divide by 0 */
	if (q1->vw != 0.0){ 
		length = atanf( length / q1->vw ); 
	} else {
		length = (float)M_PI / 2 ;
	}

	res->vw = 0.0F ;
	res->vx = q1->vx * length ;
	res->vy = q1->vy * length ;
	res->vz = q1->vz * length ;
#else
	float  s, d, theta ;

	s = sqrtf( q1->vx * q1->vx + q1->vy * q1->vy + q1->vz * q1->vz );

	theta = atan2f( s, q1->vw );
	if ( s > DELTA ) d = theta / s ; else d = 0 ;

	res->vw = 0.0F ;
	res->vx = q1->vx * d ;
	res->vy = q1->vy * d ;
	res->vz = q1->vz * d ;
#endif
}

	/*
		差の自然対数を求める
	*/
void MT_QuatLnDif( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 )
{
	FVECTOR	inv, dif ;
	float	len, len1, s ;

	MT_QuatInverse( &inv, q1 );
	MT_QuatMul( &dif, &inv, q2 );
	len = sqrtf( dif.vx * dif.vx + dif.vy * dif.vy + dif.vz * dif.vz );
	//s = MT_QuatDot( q1, q2 );
	s = dif.vw ;
	if ( s != 0.0F ) len1 = atanf( len / s ); else len1 = (float)M_PI/2 ;
	if ( len > DELTA ) len1 /= len ; else len1 = 0.0f ;
	res->vw = 0.0F ;
	res->vx = dif.vx * len1 ;
	res->vy = dif.vy * len1 ;
	res->vz = dif.vz * len1 ;
}

	/*
		クォータニオンの二次補間を求める
		Squad(p,a,b,q; t) = Slerp(Slerp(p,q;t), Slerp(a,b;t); 2(1-t)t).
	*/
void MT_QuatSquad( FVECTOR *res, FVECTOR *p, FVECTOR *a, FVECTOR *b, FVECTOR *q, float t )
{
	FVECTOR	q1, q2 ;
	float	u ;
	MT_QuatSlerp( &q1, p, q, t );
	MT_QuatSlerp( &q2, a, b, t );
	MT_QuatNormalize( &q1, &q1 );
	MT_QuatNormalize( &q2, &q2 );
	u = 2.0F * t * ( 1.0F - t ) ;
	MT_QuatSlerp( res, &q1, &q2, u );
	MT_QuatNormalize( res, res );
}


	/*
		クォータニオンの二次補間定数を求める
		Compute a, the term used in Boehm-type interpolation.
		a[n] = q[n]* exp(-(1/4)*( ln(qinv(q[n])*q[n+1]) +ln( qinv(q[n])*q[n-1] )))
	*/
void MT_QuatQCompA( FVECTOR *res, FVECTOR *q_prev, FVECTOR *q, FVECTOR *q_next )
{
	FVECTOR	q1, q2, inv, tmp ;
#if 0
	MT_QuatLnDif( &q1, q, q_prev );
	MT_QuatLnDif( &q2, q, q_next );
#else
	MT_QuatInverse( &inv, q );
	MT_QuatMul( &tmp, &inv, q_prev );
	MT_QuatLog( &q1, &tmp );
	MT_QuatMul( &tmp, &inv, q_next );
	MT_QuatLog( &q2, &tmp );
#endif
#if 0
	q1.vw += q2.vw ; q1.vx += q2.vx ; q1.vy += q2.vy ; q1.vz += q2.vz ; q2 = q1 ;
	q2.vw = - q1.vw * .25F ; q2.vx = - q1.vx * .25F ; q2.vy = - q1.vy * .25F ; q2.vz = - q1.vz * .25F ;
#else
	q2.vx = - 0.25f * ( q1.vx + q2.vx ) ;
	q2.vy = - 0.25f * ( q1.vy + q2.vy ) ;
	q2.vz = - 0.25f * ( q1.vz + q2.vz ) ;
	q2.vw = 0.0f ;
#endif
	MT_QuatExp( &q1, &q2 );
	//MT_QuatNormalize( &q1, &q1 );	// 既に正規化されているため必要ない
	MT_QuatMul( res, q, &q1 );
}
