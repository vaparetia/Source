#ifdef __cplusplus
extern "C" {
#endif

#ifndef __GTE__
typedef struct _fvector{
	float	vx,vy,vz,vw ;
} FVECTOR ;

typedef struct _fmatrix{
	float	m[4][4] ;
} FMATRIX ;
#endif

/*

	m[0][0] m[1][0] m[2][0] m[3][0]   p1.vx   p2.vx
	m[0][1] m[1][1] m[2][1] m[3][1] * p1.vy = p2.vy
	m[0][2] m[1][2] m[2][2] m[3][2]   p1.vz   p2.vz
	m[0][3] m[1][3] m[2][3] m[3][3]   p1.vw   p2.vw

*/


extern void MT_QuatToMat( FMATRIX *mat, FVECTOR *quat );
extern void MT_EulerToQuatXYZ( FVECTOR *quat, FVECTOR *rot);
extern void MT_MatToQuat( FVECTOR *quat, FMATRIX *mat );
extern void MT_QuatSlerp( FVECTOR *res, FVECTOR *from, FVECTOR *to, float t );
extern void MT_QuatNormalize( FVECTOR *res, FVECTOR *quat );
extern void MT_QuatMul( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 );
extern void MT_QuatGetValue( FVECTOR *res, FVECTOR *quat );
extern void MT_QuatSetValue( FVECTOR *quat, FVECTOR *param );
extern void MT_QuatScaleAngle(FVECTOR *res, FVECTOR * quat, float scale );
extern void MT_QuatInverse( FVECTOR *res, FVECTOR *quat );
extern void MT_QuatSetFromAx( FVECTOR *quat, FVECTOR *from, FVECTOR *to );
extern void MT_QuatAdd( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 );
extern void MT_QuatSub( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 );
extern void MT_QuatDiv( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 );
extern void MT_QuatSquare( FVECTOR *res, FVECTOR *q1 );
extern void MT_QuatSqrt( FVECTOR *res, FVECTOR *q1 );
extern float MT_QuatDot( FVECTOR *q1, FVECTOR *q2 );
extern float MT_QuatLength( FVECTOR *q1 );
extern void MT_QuatNegate( FVECTOR *res, FVECTOR *quat );
extern void MT_QuatExp( FVECTOR *res, FVECTOR *q1 );
extern void MT_QuatLog( FVECTOR *res, FVECTOR *q1 );
extern void MT_QuatLnDif( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 );
extern void MT_QuatSquad( FVECTOR *res, FVECTOR *p, FVECTOR *a, FVECTOR *b, FVECTOR *q, float t );
extern void MT_QuatQCompA( FVECTOR *res, FVECTOR *q_prev, FVECTOR *q, FVECTOR *q_next );

#ifdef __cplusplus
}
#endif
