//----------------------------------------------------------------------------
// bp_curve.h
//----------------------------------------------------------------------------
#ifndef __BP_CURVE_H__
#define __BP_CURVE_H__

//----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" 
{
#endif

//----------------------------------------------------------------------------
// INCLUDES
//----------------------------------------------------------------------------
#include "bp_math.h"
#include "bp_vector.h"

//----------------------------------------------------------------------------
// BEZIER CURVE FUNCTIONS
//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_BezierCurve6_SetPoint( FVECTOR* vec )
{
   bp_math_assert( BP_Vec3_Check( &vec[0] ) );
   bp_math_assert( BP_Vec3_Check( &vec[1] ) );
   bp_math_assert( BP_Vec3_Check( &vec[2] ) );
   bp_math_assert( BP_Vec3_Check( &vec[3] ) );
   bp_math_assert( BP_Vec3_Check( &vec[4] ) );
   bp_math_assert( BP_Vec3_Check( &vec[5] ) );
   bp_math_assert( BP_Vec3_Check( &vec[6] ) );

   BP_Vec3_Copy( &gCurveVec[0], &vec[0] );
   BP_Vec3_Copy( &gCurveVec[1], &vec[1] );
   BP_Vec3_Copy( &gCurveVec[2], &vec[2] );
   BP_Vec3_Copy( &gCurveVec[3], &vec[3] );
   BP_Vec3_Copy( &gCurveVec[4], &vec[4] );
   BP_Vec3_Copy( &gCurveVec[5], &vec[5] );
   BP_Vec3_Copy( &gCurveVec[6], &vec[6] );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_BezierCurve6_CalcPoint( FVECTOR* res, float t )
{
   float		a, b, c, d, e;
   FVECTOR	acc, param[2];

   // Check input
   bp_math_assert( BP_Float_Check(t) );
   bp_math_assert( BP_Vec3_Check( &gCurveVec[0] ) );
   bp_math_assert( BP_Vec3_Check( &gCurveVec[1] ) );
   bp_math_assert( BP_Vec3_Check( &gCurveVec[2] ) );
   bp_math_assert( BP_Vec3_Check( &gCurveVec[3] ) );
   bp_math_assert( BP_Vec3_Check( &gCurveVec[4] ) );
   bp_math_assert( BP_Vec3_Check( &gCurveVec[5] ) );
   bp_math_assert( BP_Vec3_Check( &gCurveVec[6] ) );

   // Compute factors
   a = 1.0f - t;
   b = a * a;
   c = t * t;
   d = b * b;
   e = c * c;

   // Setup coefficients0
   param[0].vx = b * d;
   param[0].vy = 6.0f * t * d * a;
   param[0].vz = 15.0f * c * d;
   param[0].vw = 20.0f * t * c * a * b;
   bp_math_assert( BP_Vec4_Check( &param[0] ) );
   
   // Setup coefficients1
   param[1].vx = 15.0f * e * b;
   param[1].vy = 6.0f * t * e * a;
   param[1].vz = c * e;
   bp_math_assert( BP_Vec3_Check( &param[1] ) );

   // Apply
   BP_Vec3_MulFloat   (       &acc, &gCurveVec[0], param[0].vx );      //acc  = 	     (param0.vx * CurveVec[0])
   BP_Vec3_MulAddFloat( &acc, &acc, &gCurveVec[1], param[0].vy );      //acc  = acc + (param0.vy * CurveVec[1])
   BP_Vec3_MulAddFloat( &acc, &acc, &gCurveVec[2], param[0].vz );      //acc  = acc + (param0.vz * CurveVec[2])
   BP_Vec3_MulAddFloat( &acc, &acc, &gCurveVec[3], param[0].vw );      //acc  = acc + (param0.vw * CurveVec[3])
   
   BP_Vec3_MulAddFloat( &acc, &acc, &gCurveVec[4], param[1].vx );      //acc  = acc + (param1.vx * CurveVec[4])
   BP_Vec3_MulAddFloat( &acc, &acc, &gCurveVec[5], param[1].vy );      //acc  = acc + (param1.vy * CurveVec[5])
   BP_Vec3_MulAddFloat( res,  &acc, &gCurveVec[6], param[1].vz );      //res  = acc + (param1.vz * CurveVec[6])
   
   res->vw = 1.0f;

   bp_math_assert( BP_Vec4_Check(res) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_CatmullRomCurve_SetPoint( FVECTOR* vec )
{
   bp_math_assert( BP_Vec3_Check( &vec[0] ) );
   bp_math_assert( BP_Vec3_Check( &vec[1] ) );
   bp_math_assert( BP_Vec3_Check( &vec[2] ) );
   bp_math_assert( BP_Vec3_Check( &vec[3] ) );

   BP_Vec3_Copy( &gCurveVec[0], &vec[0] );
   BP_Vec3_Copy( &gCurveVec[1], &vec[1] );
   BP_Vec3_Copy( &gCurveVec[2], &vec[2] );
   BP_Vec3_Copy( &gCurveVec[3], &vec[3] );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_CatmullRomCurve_CalcPoint( FVECTOR* res, float t )
{
   float	   a, b, c, d, e;
   FVECTOR  acc, param;

   // Check input
   bp_math_assert( BP_Float_Check(t) );
   bp_math_assert( BP_Vec3_Check( &gCurveVec[0] ) );
   bp_math_assert( BP_Vec3_Check( &gCurveVec[1] ) );
   bp_math_assert( BP_Vec3_Check( &gCurveVec[2] ) );
   bp_math_assert( BP_Vec3_Check( &gCurveVec[3] ) );

   // Compute factors
   a = 0.5f * t;
   b = t * t;
   c = 1.5f * t;
   d = a * b;
   e = b * c;

   // Setup coefficients
   param.vx = -d + b - a;
   param.vy = e - ( 2.5f * b ) + 1.0f;
   param.vz = -e + ( 2.0f * b ) + a;
   param.vw = d - a * t;
   bp_math_assert( BP_Vec4_Check( &param ) );

   // Apply
   BP_Vec3_MulFloat   (       &acc, &gCurveVec[0], param.vx ); //acc  =       (param.vx * CurveVec[0])
   BP_Vec3_MulAddFloat( &acc, &acc, &gCurveVec[1], param.vy ); //acc  = acc + (param.vy * CurveVec[1])
   BP_Vec3_MulAddFloat( &acc, &acc, &gCurveVec[2], param.vz ); //acc  = acc + (param.vz * CurveVec[2])
   BP_Vec3_MulAddFloat( res,  &acc, &gCurveVec[3], param.vw ); //acc  = acc + (param.vw * CurveVec[3])

   res->vw = 1.0f;

   bp_math_assert( BP_Vec4_Check(res) );
}

//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------------

#endif   //#ifndef __BP_CURVE_H__
