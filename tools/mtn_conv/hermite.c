/*
	エルミート補間関数
*/

#include <math.h>

#include "gte.h"


#define __VecMul4( _mm0, _h, _vv ) (\
							   (_vv)->vx * (_mm0)->m[0][_h] + \
							   (_vv)->vy * (_mm0)->m[1][_h] + \
							   (_vv)->vz * (_mm0)->m[2][_h] + \
							   (_vv)->vw * (_mm0)->m[3][_h] \
									)
#define ApplyMatrix( _m0, _v0, _v1 ) {\
									 (_v1)->vx = __VecMul4( (_m0), 0, _v0 ); \
									 (_v1)->vy = __VecMul4( (_m0), 1, _v0 ); \
									 (_v1)->vz = __VecMul4( (_m0), 2, _v0 ); \
									 (_v1)->vw = __VecMul4( (_m0), 3, _v0 ); \
								  }

float HermiteInterpolate2( FVECTOR *q, float t );

float HermiteInterpolate( float q0, float q1, float d0, float d1, float t )
{
	float	res ;

	res = ( ( ( t - 1 ) * ( t - 1 ) ) * ( 2 * t + 1 ) ) * q0 +
	  ( ( t * t ) * ( 3 - 2 * t ) ) * q1 +
		( ( 1 - t ) * ( 1 - t ) * t ) * d0 +
		  ( ( t - 1 ) * ( t * t ) ) * d1 ;

	return ( res );
}

float HermiteInterpolate2( FVECTOR *q, float t )
{
	static FMATRIX c_mat = {{
		{ 2.0f,-3.0f, 0.0f, 1.0f},
		{-2.0f, 3.0f, 0.0f, 0.0f},
		{ 1.0f,-2.0f, 1.0f, 0.0f},
		{ 1.0f,-1.0f, 0.0f, 0.0f}
	}};
	FVECTOR	tmp, tvec ;
	float	res ;

	tvec.vx = t * t * t ;
	tvec.vy = t * t ;
	tvec.vz = t ;
	tvec.vw = 1 ;

	//GTE_ApplyMatrix( &tmp, &c_mat, q );
	ApplyMatrix( &c_mat, q, &tmp );

	res = tvec.vx * tmp.vx + tvec.vy * tmp.vy + tvec.vz * tmp.vz + tvec.vw * tmp.vw ;

	return ( res );
}
