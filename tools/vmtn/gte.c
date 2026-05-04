#include <math.h>
#include "gte.h"

static FMATRIX  __rot_x = {{{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}}};
static FMATRIX  __rot_y = {{{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}}};
static FMATRIX  __rot_z = {{{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}}};
static FMATRIX	__tmp1 ;
FMATRIX UnitMatrix = {{{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}}};
FVECTOR	ZeroVector = {0,0,0,0};
FMATRIX	WorkMatrix ;

/* ============================================================ */
	/*
		演算展開用マクロ群
	*/

#define __LineMul3( _mm0, _mm1, _h, _v ) ( \
				  (_mm0)->m[0][_h] * (_mm1)->m[_v][0] + \
				  (_mm0)->m[1][_h] * (_mm1)->m[_v][1] + \
				  (_mm0)->m[2][_h] * (_mm1)->m[_v][2] \
				  )
#define __LineMul4( _mm0, _mm1, _h, _v ) (\
				  (_mm0)->m[0][_h] * (_mm1)->m[_v][0] + \
				  (_mm0)->m[1][_h] * (_mm1)->m[_v][1] + \
				  (_mm0)->m[2][_h] * (_mm1)->m[_v][2] + \
				  (_mm0)->m[3][_h] * (_mm1)->m[_v][3] \
				  )
#define __VecMul3( _mm0, _h, _vv ) (\
				   (_vv)->vx * (_mm0)->m[0][_h] + \
				   (_vv)->vy * (_mm0)->m[1][_h] + \
				   (_vv)->vz * (_mm0)->m[2][_h] \
				)
#if 0
#define __VecMul4( _mm0, _h, _vv ) (\
							   (_vv)->vx * (_mm0)->m[0][_h] + \
							   (_vv)->vy * (_mm0)->m[1][_h] + \
							   (_vv)->vz * (_mm0)->m[2][_h] + \
							   (_vv)->vw * (_mm0)->m[3][_h] \
									)
#else
#define __VecMul4( _mm0, _h, _vv ) (\
							   (_vv)->vx * (_mm0)->m[0][_h] + \
							   (_vv)->vy * (_mm0)->m[1][_h] + \
							   (_vv)->vz * (_mm0)->m[2][_h] + \
							   1.0f * (_mm0)->m[3][_h] \
									)
#endif

/* マトリクス回転部掛け算 (_m0)*(_m1)=(_m2) */
#define MulMatrix3D( _m0, _m1, _m2 ) {\
								   (_m2)->m[0][0] = __LineMul3( (_m0), (_m1), 0, 0 ); \
								   (_m2)->m[1][0] = __LineMul3( (_m0), (_m1), 0, 1 ); \
								   (_m2)->m[2][0] = __LineMul3( (_m0), (_m1), 0, 2 ); \
								   (_m2)->m[0][1] = __LineMul3( (_m0), (_m1), 1, 0 ); \
								   (_m2)->m[1][1] = __LineMul3( (_m0), (_m1), 1, 1 ); \
								   (_m2)->m[2][1] = __LineMul3( (_m0), (_m1), 1, 2 ); \
								   (_m2)->m[0][2] = __LineMul3( (_m0), (_m1), 2, 0 ); \
								   (_m2)->m[1][2] = __LineMul3( (_m0), (_m1), 2, 1 ); \
								   (_m2)->m[2][2] = __LineMul3( (_m0), (_m1), 2, 2 ); \
								}
/* マトリクスの合成 (_m0)*(_m1)=(_m2) */
#define MulMatrix( _m0, _m1, _m2 ) {\
									(_m2)->m[0][0] = __LineMul4( (_m0), (_m1), 0, 0 ); \
									(_m2)->m[1][0] = __LineMul4( (_m0), (_m1), 0, 1 ); \
									(_m2)->m[2][0] = __LineMul4( (_m0), (_m1), 0, 2 ); \
									(_m2)->m[3][0] = __LineMul4( (_m0), (_m1), 0, 3 ); \
									(_m2)->m[0][1] = __LineMul4( (_m0), (_m1), 1, 0 ); \
									(_m2)->m[1][1] = __LineMul4( (_m0), (_m1), 1, 1 ); \
									(_m2)->m[2][1] = __LineMul4( (_m0), (_m1), 1, 2 ); \
									(_m2)->m[3][1] = __LineMul4( (_m0), (_m1), 1, 3 ); \
									(_m2)->m[0][2] = __LineMul4( (_m0), (_m1), 2, 0 ); \
									(_m2)->m[1][2] = __LineMul4( (_m0), (_m1), 2, 1 ); \
									(_m2)->m[2][2] = __LineMul4( (_m0), (_m1), 2, 2 ); \
									(_m2)->m[3][2] = __LineMul4( (_m0), (_m1), 2, 3 ); \
									(_m2)->m[0][3] = __LineMul4( (_m0), (_m1), 3, 0 ); \
									(_m2)->m[1][3] = __LineMul4( (_m0), (_m1), 3, 1 ); \
									(_m2)->m[2][3] = __LineMul4( (_m0), (_m1), 3, 2 ); \
									(_m2)->m[3][3] = __LineMul4( (_m0), (_m1), 3, 3 ); \
								 }
/* マトリクスによる回転を行う (_m)*(_v0)=(_v1) */
#define ApplyMatrix3D( _m0, _v0, _v1 ) {\
									 (_v1)->vx = __VecMul3( (_m0), 0, _v0 ); \
									 (_v1)->vy = __VecMul3( (_m0), 1, _v0 ); \
									 (_v1)->vz = __VecMul3( (_m0), 2, _v0 ); \
								  }
/* マトリクスによる回転・移動を行う (_m0*(_v0)=(_v1) */
#define ApplyMatrix( _m0, _v0, _v1 ) {\
									 (_v1)->vx = __VecMul4( (_m0), 0, _v0 ); \
									 (_v1)->vy = __VecMul4( (_m0), 1, _v0 ); \
									 (_v1)->vz = __VecMul4( (_m0), 2, _v0 ); \
									 (_v1)->vw = __VecMul4( (_m0), 3, _v0 ); \
								  }


/* ============================================================ */



/* マトリクス回転部掛け算 (res)=(mat0)*(mat1) */
void GTE_MulMatrix3D( FMATRIX *res, FMATRIX *mat0, FMATRIX *mat1 )
{
	FMATRIX	mat ;
	MulMatrix3D( mat0, mat1, &mat );
	*res = mat ;
}
/* マトリクスの合成 (res)=(mat0)*(mat1) */
void GTE_MulMatrix( FMATRIX *res, FMATRIX *mat0, FMATRIX *mat1 )
{
	FMATRIX	mat ;
	MulMatrix( mat0, mat1, &mat );
	*res = mat ;
}
/* マトリクスによる回転を行う (res)=(mat)*(vec1) */
void GTE_ApplyMatrix3D( FVECTOR *res, FMATRIX *mat, FVECTOR *vec1 )
{
	FVECTOR		vec ;
	ApplyMatrix3D( mat, vec1, &vec );
	res->vx = vec.vx ;
	res->vy = vec.vy ;
	res->vz = vec.vz ;
}
/* マトリクスによる回転・移動を行う (res)=(mat)*(vec1) */
void GTE_ApplyMatrix( FVECTOR *res, FMATRIX *mat, FVECTOR *vec1 )
{
	FVECTOR		vec ;
	ApplyMatrix( mat, vec1, &vec );
	*res = vec ;
}
/* マトリクスにＸ軸回転をかける */
void GTE_RotMatrixX( FMATRIX *mat, float rot_x )
{
	float s, c ;
	s = sin( rot_x );
	c = cos( rot_x );
	__rot_x.m[1][1] =  c ; __rot_x.m[2][1] = -s ;
	__rot_x.m[1][2] =  s ; __rot_x.m[2][2] =  c ;
	__tmp1 = *mat ;
	MulMatrix3D( &__rot_x, &__tmp1, mat );
}
/* マトリクスにＹ軸回転をかける */
void GTE_RotMatrixY( FMATRIX *mat, float rot_y )
{
	float s, c ;
	s = sin( rot_y );
	c = cos( rot_y );
	__rot_y.m[0][0] =  c ; __rot_y.m[2][0] =  s ;
	__rot_y.m[0][2] = -s ; __rot_y.m[2][2] =  c ;
	__tmp1 = *mat ;
	MulMatrix3D( &__rot_y, &__tmp1, mat );
}
/* マトリクスにＺ軸回転をかける */
void GTE_RotMatrixZ( FMATRIX *mat, float rot_z )
{
	float s, c ;
	s = sin( rot_z );
	c = cos( rot_z );
	__rot_z.m[0][0] =  c ; __rot_z.m[1][0] = -s ;
	__rot_z.m[0][1] =  s ; __rot_z.m[1][1] =  c ;
	__tmp1 = *mat ;
	MulMatrix3D( &__rot_z, &__tmp1, mat );
}
/* 回転ベクトルに応じたマトリクスを生成（Ｍ）=（Ｘ）*（Ｙ）*（Ｚ） */
void GTE_RotMatrixXYZ( FMATRIX *mat, FVECTOR *rot )
{
	*mat = UnitMatrix ;
	GTE_RotMatrixZ( mat, rot->vz );
	GTE_RotMatrixY( mat, rot->vy );
	GTE_RotMatrixX( mat, rot->vx );
}
/* 回転ベクトルに応じたマトリクスを生成（Ｍ）=（Ｚ）*（Ｙ）*（Ｘ） */
void GTE_RotMatrixZYX( FMATRIX *mat, FVECTOR *rot )
{
	*mat = UnitMatrix ;
	GTE_RotMatrixX( mat, rot->vx );
	GTE_RotMatrixY( mat, rot->vy );
	GTE_RotMatrixZ( mat, rot->vz );
}

/* ベクトルの外積を求める（ res = v1 * v2 ） */
void GTE_OuterProduct( FVECTOR *res, FVECTOR *v1, FVECTOR *v2 )
{
	FVECTOR	vec ;
	vec.vx = v1->vy * v2->vz - v1->vz * v2->vy ;
	vec.vy = v1->vz * v2->vx - v1->vx * v2->vz ;
	vec.vz = v1->vx * v2->vy - v1->vy * v2->vx ;
	*res = vec ;
}

/* ベクトルの正規化 */
void GTE_VectorNormal( FVECTOR *res, FVECTOR *v1 )
{
	FVECTOR	vec ;
	float	len ;
	len = sqrt( v1->vx * v1->vx + v1->vy * v1->vy + v1->vz * v1->vz );
	if ( len == 0.0 ) len = 0.00000001 ;
	vec.vx = v1->vx / len ;
	vec.vy = v1->vy / len ;
	vec.vz = v1->vz / len ;
	*res = vec ;
}

/* マトリクスの回転成分に対する逆マトリクスを求める */
void GTE_TransposeMatrix( FMATRIX *mat2, FMATRIX *mat1 )
{
	float	tmp ;
	FMATRIX	mat ;
	mat = *mat1 ;
	mat.m[ 0 ][ 0 ] = mat1->m[ 0 ][ 0 ] ;
	mat.m[ 1 ][ 0 ] = mat1->m[ 0 ][ 1 ] ;
	mat.m[ 2 ][ 0 ] = mat1->m[ 0 ][ 2 ] ;
	mat.m[ 0 ][ 1 ] = mat1->m[ 1 ][ 0 ] ;
	mat.m[ 1 ][ 1 ] = mat1->m[ 1 ][ 1 ] ;
	mat.m[ 2 ][ 1 ] = mat1->m[ 1 ][ 2 ] ;
	mat.m[ 0 ][ 2 ] = mat1->m[ 2 ][ 0 ] ;
	mat.m[ 1 ][ 2 ] = mat1->m[ 2 ][ 1 ] ;
	mat.m[ 2 ][ 2 ] = mat1->m[ 2 ][ 2 ] ;
	*mat2 = mat ;
}

/* マトリクスを単位マトリクスで初期化 */
void GTE_UnitMatrix( FMATRIX *mat )
{
	*mat = UnitMatrix ;
}

/* 逆変換マトリクスの作成 */
void GTE_InversMatrix( FMATRIX *inv, FMATRIX *mat )
{
	FVECTOR	vec, tmp ;
	GTE_UnitMatrix( inv );
	GTE_TransposeMatrix( inv, mat ) ;
	vec.vx = - ( mat->m[ 3 ][ 0 ] ) ;
	vec.vy = - ( mat->m[ 3 ][ 1 ] ) ;
	vec.vz = - ( mat->m[ 3 ][ 2 ] ) ;
	ApplyMatrix3D( inv, &vec, &tmp );
	inv->m[ 3 ][ 0 ] = tmp.vx ;
	inv->m[ 3 ][ 1 ] = tmp.vy ;
	inv->m[ 3 ][ 2 ] = tmp.vz ;
}

/* ２点の中間ベクトルを生成（ v1(t=0) ~ v2(t=1) ） */
int GTE_InterpolateVector( FVECTOR *res, FVECTOR *v1, FVECTOR *v2, float t )
{
	FVECTOR v ;

	v.vx = ( v2->vx - v1->vx ) * t + v1->vx ;
	v.vy = ( v2->vy - v1->vy ) * t + v1->vy ;
	v.vz = ( v2->vz - v1->vz ) * t + v1->vz ;
	v.vw = 1.0f ;

	*res = v ;
}

/* ポリゴンと線分の当たり判定チェック */
int GTE_RayCheck( FVECTOR *res, FVECTOR *from, FVECTOR *to, FVECTOR *poly, int n_point )
{
	FVECTOR	norm, dir, l1, l2, point, tmp ;
	float	f1, f2, f ;
	int		i, flag ;

	/* 法線を求める */
	l1.vx = poly[1].vx - poly[0].vx ;
	l1.vy = poly[1].vy - poly[0].vy ;
	l1.vz = poly[1].vz - poly[0].vz ;
	l2.vx = poly[2].vx - poly[1].vx ;
	l2.vy = poly[2].vy - poly[1].vy ;
	l2.vz = poly[2].vz - poly[1].vz ;
	GTE_OuterProduct( &norm, &l1, &l2 );
	GTE_VectorNormal( &norm, &norm );

	/* 線分のベクトルを求める */
	dir.vx = to->vx - from->vx ;
	dir.vy = to->vy - from->vy ;
	dir.vz = to->vz - from->vz ;

	/* 任意のポリゴン頂点までのベクトルを求める */
	l1.vx = poly[0].vx - from->vx ;
	l1.vy = poly[0].vy - from->vy ;
	l1.vz = poly[0].vz - from->vz ;

	/* ポリゴン平面までの距離を求める */
	f1 = norm.vx * l1.vx + norm.vy * l1.vy + norm.vz * l1.vz ;

	/* 線分ベクトルと法線の内積を求める */
	f2 = norm.vx * dir.vx + norm.vy * dir.vy + norm.vz * dir.vz ;

	if ( f2 == 0.0f ) return ( 0 );

	/* 長さによるチェック */
	f = f1 / f2 ;
	if ( 1.0f < f ) return ( 0 ) ;

	/* 交点を求める */
	point.vx = from->vx + dir.vx * f ;
	point.vy = from->vy + dir.vy * f ;
	point.vz = from->vz + dir.vz * f ;

	/* 交点がポリゴンの中に入っているか調べる */
	for ( i = 0 ; i < n_point ; i++ ){
		/* チェックする線分を求める */
		if ( i < ( n_point - 1 ) ){
			l1.vx = poly[i+1].vx - poly[i].vx ;
			l1.vy = poly[i+1].vy - poly[i].vy ;
			l1.vz = poly[i+1].vz - poly[i].vz ;
		} else {
			l1.vx = poly[0].vx - poly[i].vx ;
			l1.vy = poly[0].vy - poly[i].vy ;
			l1.vz = poly[0].vz - poly[i].vz ;
		}
		/* ポリゴン頂点－交点のベクトルを求める */
		l2.vx = point.vx - poly[i].vx ;
		l2.vy = point.vy - poly[i].vy ;
		l2.vz = point.vz - poly[i].vz ;

		/* 全ての外積方向が同一ならポリゴン内に交点が存在する */
		GTE_OuterProduct( &tmp, &l1, &l2 );
		/* 法線と比べる */
		f = norm.vx * tmp.vx + norm.vy * tmp.vy + norm.vz * tmp.vz ;

		if ( i == 0 ){
			/* 最初の外積ベクトルの方向を記憶する */
			if ( f < 0 )	flag = 0 ;
			else			flag = 1 ;
		} else {
			/* 最初の外積ベクトルと同じかどうかをチェックする */
			if ( f < 0 ){
				if ( flag == 1 ) return ( 0 );
			} else {
				if ( flag == 0 ) return ( 0 );
			}
		}
	}

	*res = point ;

	return ( 1 );

}
