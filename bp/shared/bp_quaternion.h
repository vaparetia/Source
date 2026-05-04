//----------------------------------------------------------------------------
// bp_quaternion.h
//----------------------------------------------------------------------------
#ifndef __BP_QUATERNION_H__
#define __BP_QUATERNION_H__

//----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" 
{
#endif

//----------------------------------------------------------------------------
// INCLUDES
//----------------------------------------------------------------------------
#include "bp_math.h"
#include "bp_matrix.h"

//----------------------------------------------------------------------------
// QUATERNION FUNCTIONS
//----------------------------------------------------------------------------

EXTERN_INLINE
int BP_Quat_Check( const FVECTOR* vec )
{
   return BP_Float_Check(vec->vx) && 
      BP_Float_Check(vec->vy) && 
      BP_Float_Check(vec->vz) && 
      BP_Float_Check(vec->vw);
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Quat_Copy( FVECTOR* dst, const FVECTOR* src )
{
   bp_math_assert( BP_Quat_Check(src) );

   dst->vx = src->vx;
   dst->vy = src->vy;
   dst->vz = src->vz;
   dst->vw = src->vw;
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Quat_ToMat( FMATRIX* mat, const FVECTOR* quat )
{
   float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2 ;

   bp_math_assert( BP_Quat_Check(quat) );

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

   bp_math_assert( BP_Mat_Check(mat) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Quat_Slerp( FVECTOR* res, const FVECTOR* from, const FVECTOR* to, const float t )
{
   float	to1[4];
   float	omega, cosom, sinom;
   float	scale0, scale1;

   bp_math_assert( BP_Quat_Check(from) );
   bp_math_assert( BP_Quat_Check(to) );
   bp_math_assert( BP_Float_Check(t) );

   cosom = (from->vx * to->vx) + (from->vy * to->vy) + (from->vz * to->vz) + (from->vw * to->vw);

   if ( cosom < 0.0F )
   {
      cosom = -cosom;
      to1[0] = - to->vx;
      to1[1] = - to->vy;
      to1[2] = - to->vz;
      to1[3] = - to->vw;
   } 
   else  
   {
      to1[0] = to->vx;
      to1[1] = to->vy;
      to1[2] = to->vz;
      to1[3] = to->vw;
   }

   if ( ( 1.0F - cosom ) > DELTA )
   {
      sinom = BP_Sqrt( 1.0F - cosom * cosom );
      omega = BP_ASin(sinom);
      scale0 = BP_SinFast( ( 1.0F - t ) * omega ) / sinom;
      scale1 = BP_SinFast( t * omega) / sinom;
   }
   else 
   {
      scale0 = 1.0F - t;
      scale1 = t;
   }

   to1[0] = (scale0 * from->vx) + (scale1 * to1[0]);
   to1[1] = (scale0 * from->vy) + (scale1 * to1[1]);
   to1[2] = (scale0 * from->vz) + (scale1 * to1[2]);
   to1[3] = (scale0 * from->vw) + (scale1 * to1[3]);
   
   res->vx = to1[0];
   res->vy = to1[1];
   res->vz = to1[2];
   res->vw = to1[3];

   bp_math_assert( BP_Quat_Check(res) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Quat_Normalize( FVECTOR* res, const FVECTOR* quat )
{
   float	dist, square;

   bp_math_assert( BP_Quat_Check(quat) );

   square = (quat->vx * quat->vx) + (quat->vy * quat->vy) + (quat->vz * quat->vz) + (quat->vw * quat->vw);
   if ( square > 0.00001F )
   {
      dist = (float)( 1.0F / BP_Sqrt( square ) );
      res->vx = quat->vx * dist;
      res->vy = quat->vy * dist;
      res->vz = quat->vz * dist;
      res->vw = quat->vw * dist;
   } 
   else 
   {
      *res = *quat;
   }

   bp_math_assert( BP_Quat_Check(res) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Quat_Normalize_Fast( FVECTOR *res, FVECTOR const *quat )
{
   // for the results of QuatMul etc that return effectively already normalized quats
   // this just does another pass to make sure
   float scale, square;

   bp_math_assert( BP_Quat_Check(quat) );

   square = (quat->vx * quat->vx) + (quat->vy * quat->vy) + (quat->vz * quat->vz) + (quat->vw * quat->vw);

#ifdef _DEBUG
   {
      float error = fabsf(square - 1.0f);
      // approx. error such that final magnitude error > 1e-6f
      if (error > 0.0005f)
      {
         printf("Quat normalize could be inaccurate!\n");
      }
   }
#endif

   // for small x, 1/sqrt(1+x) ~= 1-x/2
   scale = 1.0f - 0.5f*(square - 1.0f);

   BP_Vec4_MulFloat(res, quat, scale);

   bp_math_assert( BP_Quat_Check(res) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Quat_MulFast( FVECTOR* res, const FVECTOR* q1, const FVECTOR* q2 )
{
   FVECTOR	vec ;

   bp_math_assert( BP_Quat_Check(q1) );
   bp_math_assert( BP_Quat_Check(q2) );

   vec.vx = (q1->vw * q2->vx) + (q1->vx * q2->vw) + (q1->vy * q2->vz) - (q1->vz * q2->vy);
   vec.vy = (q1->vw * q2->vy) + (q1->vy * q2->vw) + (q1->vz * q2->vx) - (q1->vx * q2->vz);
   vec.vz = (q1->vw * q2->vz) + (q1->vz * q2->vw) + (q1->vx * q2->vy) - (q1->vy * q2->vx);
   vec.vw = (q1->vw * q2->vw) - (q1->vx * q2->vx) - (q1->vy * q2->vy) - (q1->vz * q2->vz);

   BP_Quat_Copy( res, &vec );

   bp_math_assert( BP_Quat_Check(res) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Quat_Mul( FVECTOR* res, const FVECTOR* q1, const FVECTOR* q2 )
{
   FVECTOR	vec ;

   bp_math_assert( BP_Quat_Check(q1) );
   bp_math_assert( BP_Quat_Check(q2) );

   vec.vx = (q1->vw * q2->vx) + (q1->vx * q2->vw) + (q1->vy * q2->vz) - (q1->vz * q2->vy);
   vec.vy = (q1->vw * q2->vy) + (q1->vy * q2->vw) + (q1->vz * q2->vx) - (q1->vx * q2->vz);
   vec.vz = (q1->vw * q2->vz) + (q1->vz * q2->vw) + (q1->vx * q2->vy) - (q1->vy * q2->vx);
   vec.vw = (q1->vw * q2->vw) - (q1->vx * q2->vx) - (q1->vy * q2->vy) - (q1->vz * q2->vz);

   BP_Quat_Normalize_Fast( res, &vec );

   bp_math_assert( BP_Quat_Check(res) );
}

//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------------

#endif   //#ifndef __BP_QUATERNION_H__
