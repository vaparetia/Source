//----------------------------------------------------------------------------
// bp_matrix.h
//----------------------------------------------------------------------------
#ifndef __BP_MATRIX_H__
#define __BP_MATRIX_H__

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
#include "libvu0.h"

#if BP_VITA
extern float32x4x4_t gkUnitMatrix;
#include "BP_Simd.h"
#endif

//----------------------------------------------------------------------------
// MATRIX FUNCTIONS
//----------------------------------------------------------------------------

EXTERN_INLINE
int BP_Mat_Check( const FMATRIX* mat )
{
   return BP_Float_Check(mat->m[0][0]) && 
      BP_Float_Check(mat->m[0][1]) && 
      BP_Float_Check(mat->m[0][2]) && 
      BP_Float_Check(mat->m[0][3]) && 
      BP_Float_Check(mat->m[1][0]) && 
      BP_Float_Check(mat->m[1][1]) && 
      BP_Float_Check(mat->m[1][2]) && 
      BP_Float_Check(mat->m[1][3]) && 
      BP_Float_Check(mat->m[2][0]) && 
      BP_Float_Check(mat->m[2][1]) && 
      BP_Float_Check(mat->m[2][2]) && 
      BP_Float_Check(mat->m[2][3]) && 
      BP_Float_Check(mat->m[3][0]) && 
      BP_Float_Check(mat->m[3][1]) && 
      BP_Float_Check(mat->m[3][2]) && 
      BP_Float_Check(mat->m[3][3]);
}

//----------------------------------------------------------------------------

EXTERN_INLINE
int BP_Mat_IsZeroMatrix( const FMATRIX* mat )
{
   return (mat->m[0][0] == 0.0f) && 
      (mat->m[0][1] == 0.0f) && 
      (mat->m[0][2] == 0.0f) && 
      (mat->m[0][3] == 0.0f) && 
      (mat->m[1][0] == 0.0f) && 
      (mat->m[1][1] == 0.0f) && 
      (mat->m[1][2] == 0.0f) && 
      (mat->m[1][3] == 0.0f) && 
      (mat->m[2][0] == 0.0f) && 
      (mat->m[2][1] == 0.0f) && 
      (mat->m[2][2] == 0.0f) && 
      (mat->m[2][3] == 0.0f) && 
      (mat->m[3][0] == 0.0f) && 
      (mat->m[3][1] == 0.0f) && 
      (mat->m[3][2] == 0.0f) && 
      (mat->m[3][3] == 0.0f);
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Mat_CopyColumn( FMATRIX* mat, int to, int from )
{
   // BP_VITA_MATRIX_TODO
   float* dst = &mat->m[0][to];
   float* src = &mat->m[0][from];

   bp_math_assert( BP_Mat_Check(mat) );

   dst[0]  = src[0];
   dst[4]  = src[4];
   dst[8]  = src[8];
   dst[12] = src[12];
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Mat_CopyLine( FMATRIX* mat, int to, int from )
{
#if BP_VITA
   float32x4x4_t *nmat = M2NEONPTR(mat);

   bp_math_assert( BP_Mat_Check(mat) );

   nmat->val[to] = nmat->val[from];
#else
   FVECTOR* dst = (FVECTOR*)&mat->m[to][0];
   FVECTOR* src = (FVECTOR*)&mat->m[from][0];

   bp_math_assert( BP_Mat_Check(mat) );

   BP_Vec4_Copy( dst, src );
#endif
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Mat_Copy( FMATRIX* dst, const FMATRIX* src )
{
#if BP_VITA
   *M2NEONPTR(dst) = M2NEON(src);
#else
   BP_Vec4_Copy( (FVECTOR*)(&dst->m[0][0]), (const FVECTOR*)(&src->m[0][0]) );
   BP_Vec4_Copy( (FVECTOR*)(&dst->m[1][0]), (const FVECTOR*)(&src->m[1][0]) );
   BP_Vec4_Copy( (FVECTOR*)(&dst->m[2][0]), (const FVECTOR*)(&src->m[2][0]) );
   BP_Vec4_Copy( (FVECTOR*)(&dst->m[3][0]), (const FVECTOR*)(&src->m[3][0]) );
#endif
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Mat_SetUnit( FMATRIX *r )
{
#if BP_VITA
   *M2NEONPTR(r) = gkUnitMatrix;
#else
   r->m[0][0] = r->m[1][1] = r->m[2][2] = r->m[3][3] = 1.0f ;
   r->m[0][1] = r->m[0][2] = r->m[0][3] = 0.0f ;
   r->m[1][0] = r->m[1][2] = r->m[1][3] = 0.0f ;
   r->m[2][0] = r->m[2][1] = r->m[2][3] = 0.0f ;
   r->m[3][0] = r->m[3][1] = r->m[3][2] = 0.0f ;
#endif
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Mat_MakeRotateAxis( FMATRIX* res, FVECTOR* in_axis, float angle )
{
   // BP_VITA_MATRIX_TODO

   FVECTOR axis;
   float	c, s, ic, tmp ;
   
   // Make copy of input axis, since it is sometimes points to the resultant matrix!
   BP_Vec3_Copy( &axis, in_axis );

   bp_math_assert( BP_Vec3_Check(&axis) );

   c = cosf( angle );
   s = -sinf( angle );
   ic = 1.0f - c ;
   
   /* x */
   tmp = ic * axis.vx ;
   res->m[0][0] = tmp * axis.vx + c ;
   res->m[0][1] = tmp * axis.vy - axis.vz * s ;
   res->m[0][2] = tmp * axis.vz + axis.vy * s ;
   res->m[0][3] = 0.0f ;

   /* y */
   tmp = ic * axis.vy ;
   res->m[1][0] = tmp * axis.vx + axis.vz * s ;
   res->m[1][1] = tmp * axis.vy + c ;
   res->m[1][2] = tmp * axis.vz - axis.vx * s ;
   res->m[1][3] = 0.0f ;

   /* z */
   tmp = ic * axis.vz ;
   res->m[2][0] = tmp * axis.vx - axis.vy * s ;
   res->m[2][1] = tmp * axis.vy + axis.vx * s ;
   res->m[2][2] = tmp * axis.vz + c ;
   res->m[2][3] = 0.0f ;

   /* t */
   res->m[3][0] = 0.0f ;
   res->m[3][1] = 0.0f ;
   res->m[3][2] = 0.0f ;
   res->m[3][3] = 1.0f ;

   bp_math_assert( BP_Mat_Check(res) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Mat_MakeDropShadow( FMATRIX* res, FVECTOR* lp, float a, float b, float c, int mode )
{	
   //libvu0.c
   if( mode )
   {	
      // spot light 
      float x = lp->vx, y = lp->vy, z = lp->vz;
      float d = 1.0f-(a*x+b*y+c*z);

      res->m[0][0] = a*x+d, res->m[1][0] = b*x,   res->m[2][0] = c*x,   res->m[3][0] = -x;
      res->m[0][1] = a*y,   res->m[1][1] = b*y+d, res->m[2][1] = c*y,   res->m[3][1] = -y;
      res->m[0][2] = a*z,   res->m[1][2] = b*z,   res->m[2][2] = c*z+d, res->m[3][2] = -z;
      res->m[0][3] = a,     res->m[1][3] = b,     res->m[2][3] = c,     res->m[3][3] = d-1.0f;
   }
   else 
   {
      // parallel light
      float p  = lp->vx, q = lp->vy, r = lp->vz;
      float n  = a*p+b*q+c*r;
      float nr ;
      
      // fix zero div
      if( n == 0.0f ) n = 0.0000001f;
      nr = -(float)1.0f/n;

      res->m[0][0] = nr*(a*p-n), res->m[1][0] = nr*(b*p),   res->m[2][0] = nr*(c*p),   res->m[3][0] = nr*(-p);
      res->m[0][1] = nr*(a*q),   res->m[1][1] = nr*(b*q-n), res->m[2][1] = nr*(c*q),   res->m[3][1] = nr*(-q);
      res->m[0][2] = nr*(a*r),   res->m[1][2] = nr*(b*r),   res->m[2][2] = nr*(c*r-n), res->m[3][2] = nr*(-r);
      res->m[0][3] = 0.0f,       res->m[1][3] = 0.0f,       res->m[2][3] = 0.0f,       res->m[3][3] = nr*(-n);
   }
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Mat_TransformVec3( FVECTOR* res, const FMATRIX* m, const FVECTOR* v )
{
#if BP_VITA
   float32x4_t ret;
   float32x4x4_t const *a_src = ((float32x4x4_t const *) m );
   float32x4_t const v_src = *((float32x4_t const *) v );

   bp_math_assert( BP_Mat_Check(m) );
   bp_math_assert( BP_Vec3_Check(v) );

   ret = vmlaq_n_f32( a_src->val[3],  a_src->val[0], v_src[0] );
   ret = vmlaq_n_f32( ret,            a_src->val[1], v_src[1] );
   ret = vmlaq_n_f32( ret,            a_src->val[2], v_src[2] );

   *((float32x4_t *) res ) = ret;

   bp_math_assert( BP_Vec4_Check(res) );
#else
   FVECTOR tv;

   bp_math_assert( BP_Mat_Check(m) );
   bp_math_assert( BP_Vec3_Check(v) );

   // Assumes vector w=1
   tv.vx = (m->m[0][0] * v->vx) + (m->m[1][0] * v->vy) + (m->m[2][0] * v->vz) + (m->m[3][0]);
   tv.vy = (m->m[0][1] * v->vx) + (m->m[1][1] * v->vy) + (m->m[2][1] * v->vz) + (m->m[3][1]);
   tv.vz = (m->m[0][2] * v->vx) + (m->m[1][2] * v->vy) + (m->m[2][2] * v->vz) + (m->m[3][2]);
   tv.vw = (m->m[0][3] * v->vx) + (m->m[1][3] * v->vy) + (m->m[2][3] * v->vz) + (m->m[3][3]);

   BP_Vec4_Copy( res, &tv );

   bp_math_assert( BP_Vec4_Check(res) );
#endif
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Mat_TransformVec4( FVECTOR* res, const FMATRIX* m, const FVECTOR* v )
{
#if BP_VITA
   float32x4_t ret;
   float32x4x4_t const *a_src = ((float32x4x4_t const *) m );
   float32x4_t const v_src = *((float32x4_t const *) v );

   bp_math_assert( BP_Mat_Check(m) );
   bp_math_assert( BP_Vec4_Check(v) );

   ret = vmulq_n_f32(                 a_src->val[0], v_src[0] );
   ret = vmlaq_n_f32( ret,            a_src->val[1], v_src[1] );
   ret = vmlaq_n_f32( ret,            a_src->val[2], v_src[2] );
   ret = vmlaq_n_f32( ret,            a_src->val[3], v_src[3] );

   *((float32x4_t *) res ) = ret;

   bp_math_assert( BP_Vec4_Check(res) );
#else
   FVECTOR tv;

   bp_math_assert( BP_Mat_Check(m) );
   bp_math_assert( BP_Vec4_Check(v) );

   tv.vx = (m->m[0][0] * v->vx) + (m->m[1][0] * v->vy) + (m->m[2][0] * v->vz) + (m->m[3][0] * v->vw);
   tv.vy = (m->m[0][1] * v->vx) + (m->m[1][1] * v->vy) + (m->m[2][1] * v->vz) + (m->m[3][1] * v->vw);
   tv.vz = (m->m[0][2] * v->vx) + (m->m[1][2] * v->vy) + (m->m[2][2] * v->vz) + (m->m[3][2] * v->vw);
   tv.vw = (m->m[0][3] * v->vx) + (m->m[1][3] * v->vy) + (m->m[2][3] * v->vz) + (m->m[3][3] * v->vw);

   BP_Vec4_Copy( res, &tv );

   bp_math_assert( BP_Vec4_Check(res) );
#endif
}

//----------------------------------------------------------------------------

// C version of PS2 asm in "MA_MulMatrix"
EXTERN_INLINE
void BP_Mat_MulMatrix(FMATRIX* res, const FMATRIX* a, const FMATRIX* b)
{
#if BP_VITA
   float32x4x4_t ret;

   float32x4x4_t const *a_src = ((float32x4x4_t const *) a );
   float32x4x4_t const *b_src = ((float32x4x4_t const *) b );

   bp_math_assert( BP_Mat_Check(a) );
   bp_math_assert( BP_Mat_Check(b) );

   ret.val[0] = vmulq_n_f32(             a_src->val[0], b_src->val[0][0] );
   ret.val[0] = vmlaq_n_f32( ret.val[0], a_src->val[1], b_src->val[0][1] );
   ret.val[0] = vmlaq_n_f32( ret.val[0], a_src->val[2], b_src->val[0][2] );
   ret.val[0] = vmlaq_n_f32( ret.val[0], a_src->val[3], b_src->val[0][3] );

   ret.val[1] = vmulq_n_f32(             a_src->val[0], b_src->val[1][0] );
   ret.val[1] = vmlaq_n_f32( ret.val[1], a_src->val[1], b_src->val[1][1] );
   ret.val[1] = vmlaq_n_f32( ret.val[1], a_src->val[2], b_src->val[1][2] );
   ret.val[1] = vmlaq_n_f32( ret.val[1], a_src->val[3], b_src->val[1][3] );

   ret.val[2] = vmulq_n_f32(             a_src->val[0], b_src->val[2][0] );
   ret.val[2] = vmlaq_n_f32( ret.val[2], a_src->val[1], b_src->val[2][1] );
   ret.val[2] = vmlaq_n_f32( ret.val[2], a_src->val[2], b_src->val[2][2] );
   ret.val[2] = vmlaq_n_f32( ret.val[2], a_src->val[3], b_src->val[2][3] );

   ret.val[3] = vmulq_n_f32(             a_src->val[0], b_src->val[3][0] );
   ret.val[3] = vmlaq_n_f32( ret.val[3], a_src->val[1], b_src->val[3][1] );
   ret.val[3] = vmlaq_n_f32( ret.val[3], a_src->val[2], b_src->val[3][2] );
   ret.val[3] = vmlaq_n_f32( ret.val[3], a_src->val[3], b_src->val[3][3] );

   *((float32x4x4_t *)res) = ret;

#else
   FMATRIX mat;

   bp_math_assert( BP_Mat_Check(a) );
   bp_math_assert( BP_Mat_Check(b) );

   mat.m[0][0] = (a->m[0][0] * b->m[0][0]) + (a->m[1][0] * b->m[0][1]) + (a->m[2][0] * b->m[0][2]) + (a->m[3][0] * b->m[0][3]);
   mat.m[0][1] = (a->m[0][1] * b->m[0][0]) + (a->m[1][1] * b->m[0][1]) + (a->m[2][1] * b->m[0][2]) + (a->m[3][1] * b->m[0][3]);
   mat.m[0][2] = (a->m[0][2] * b->m[0][0]) + (a->m[1][2] * b->m[0][1]) + (a->m[2][2] * b->m[0][2]) + (a->m[3][2] * b->m[0][3]);
   mat.m[0][3] = (a->m[0][3] * b->m[0][0]) + (a->m[1][3] * b->m[0][1]) + (a->m[2][3] * b->m[0][2]) + (a->m[3][3] * b->m[0][3]);
   
   mat.m[1][0] = (a->m[0][0] * b->m[1][0]) + (a->m[1][0] * b->m[1][1]) + (a->m[2][0] * b->m[1][2]) + (a->m[3][0] * b->m[1][3]);
   mat.m[1][1] = (a->m[0][1] * b->m[1][0]) + (a->m[1][1] * b->m[1][1]) + (a->m[2][1] * b->m[1][2]) + (a->m[3][1] * b->m[1][3]);
   mat.m[1][2] = (a->m[0][2] * b->m[1][0]) + (a->m[1][2] * b->m[1][1]) + (a->m[2][2] * b->m[1][2]) + (a->m[3][2] * b->m[1][3]);
   mat.m[1][3] = (a->m[0][3] * b->m[1][0]) + (a->m[1][3] * b->m[1][1]) + (a->m[2][3] * b->m[1][2]) + (a->m[3][3] * b->m[1][3]);

   mat.m[2][0] = (a->m[0][0] * b->m[2][0]) + (a->m[1][0] * b->m[2][1]) + (a->m[2][0] * b->m[2][2]) + (a->m[3][0] * b->m[2][3]);
   mat.m[2][1] = (a->m[0][1] * b->m[2][0]) + (a->m[1][1] * b->m[2][1]) + (a->m[2][1] * b->m[2][2]) + (a->m[3][1] * b->m[2][3]);
   mat.m[2][2] = (a->m[0][2] * b->m[2][0]) + (a->m[1][2] * b->m[2][1]) + (a->m[2][2] * b->m[2][2]) + (a->m[3][2] * b->m[2][3]);
   mat.m[2][3] = (a->m[0][3] * b->m[2][0]) + (a->m[1][3] * b->m[2][1]) + (a->m[2][3] * b->m[2][2]) + (a->m[3][3] * b->m[2][3]);

   mat.m[3][0] = (a->m[0][0] * b->m[3][0]) + (a->m[1][0] * b->m[3][1]) + (a->m[2][0] * b->m[3][2]) + (a->m[3][0] * b->m[3][3]);
   mat.m[3][1] = (a->m[0][1] * b->m[3][0]) + (a->m[1][1] * b->m[3][1]) + (a->m[2][1] * b->m[3][2]) + (a->m[3][1] * b->m[3][3]);
   mat.m[3][2] = (a->m[0][2] * b->m[3][0]) + (a->m[1][2] * b->m[3][1]) + (a->m[2][2] * b->m[3][2]) + (a->m[3][2] * b->m[3][3]);
   mat.m[3][3] = (a->m[0][3] * b->m[3][0]) + (a->m[1][3] * b->m[3][1]) + (a->m[2][3] * b->m[3][2]) + (a->m[3][3] * b->m[3][3]);

   BP_Mat_Copy( res, &mat );
#endif

   bp_math_assert( BP_Mat_Check(res) );

}

//----------------------------------------------------------------------------

// From Graphics gems, Volume 1, Page 548 - "Transforming axis-aligned bounding boxes"
EXTERN_INLINE 
void BP_Mat_TransformBound3( FVECTOR* res_bound_min, FVECTOR* res_bound_max, const FMATRIX* m, const FVECTOR* bound_min, const FVECTOR* bound_max )
{
   int i, j;
   float a, b;

   // Use arrays
   const float* in_min  = (const float*)bound_min;
   const float* in_max  = (const float*)bound_max;
   float* out_min = (float*)res_bound_min;
   float* out_max = (float*)res_bound_max;

   // Make sure input/output do not overlap
   assert( out_min != in_min );
   assert( out_min != in_max );
   assert( out_max != in_min );
   assert( out_max != in_max );

   // Check input
   bp_math_assert( BP_Mat_Check(m) );
   bp_math_assert( BP_Vec3_Check(bound_min) );
   bp_math_assert( BP_Vec3_Check(bound_max) );

   // Start with translation from matrix
   out_min[0] = out_max[0] = m->m[3][0];
   out_min[1] = out_max[1] = m->m[3][1];
   out_min[2] = out_max[2] = m->m[3][2];
   out_min[3] = out_max[3] = 1.0f;

   // Expand extents
   for( i = 0; i < 3; i++ )
   {
      for( j = 0; j < 3; j++ )
      {
         a = m->m[i][j] * in_min[j];
         b = m->m[i][j] * in_max[j];
         if( a < b )
         {
            out_min[i] += a;
            out_max[i] += b;
         }
         else
         {
            out_min[i] += b;
            out_max[i] += a;
         }
      }
   }

   bp_math_assert( BP_Vec4_Check(res_bound_min) );
   bp_math_assert( BP_Vec4_Check(res_bound_max) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Mat_RotateVec3( FVECTOR* res, const FMATRIX* m, const FVECTOR* v )
{
#if BP_VITA
   float32x4_t ret;
   float32x4x4_t const *a_src = ((float32x4x4_t const *) m );
   float32x4_t const v_src = *((float32x4_t const *) v );

   bp_math_assert( BP_Mat_Check(m) );
   bp_math_assert( BP_Vec3_Check(v) );

   ret = vmulq_n_f32(                 a_src->val[0], v_src[0] );
   ret = vmlaq_n_f32( ret,            a_src->val[1], v_src[1] );
   ret = vmlaq_n_f32( ret,            a_src->val[2], v_src[2] );
   ret[3] = v_src[3];

   *((float32x4_t *) res ) = ret;
   bp_math_assert( BP_Vec3_Check(res) );
#else
   FVECTOR tv;

   bp_math_assert( BP_Mat_Check(m) );
   bp_math_assert( BP_Vec3_Check(v) );

   tv.vx = (m->m[0][0]*v->vx) + (m->m[1][0]*v->vy) + (m->m[2][0]*v->vz);
   tv.vy = (m->m[0][1]*v->vx) + (m->m[1][1]*v->vy) + (m->m[2][1]*v->vz);
   tv.vz = (m->m[0][2]*v->vx) + (m->m[1][2]*v->vy) + (m->m[2][2]*v->vz);
   tv.vw = v->vw;

   BP_Vec4_Copy( res, &tv );

   bp_math_assert( BP_Vec3_Check(res) );
#endif
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Mat_RotateVec4( FVECTOR* res, const FMATRIX* m, const FVECTOR* v )
{
   FVECTOR tv;

   bp_math_assert( BP_Mat_Check(m) );
   bp_math_assert( BP_Vec4_Check(v) );

   tv.vx = (m->m[0][0] * v->vx) + (m->m[1][0] * v->vy) + (m->m[2][0] * v->vz);
   tv.vy = (m->m[0][1] * v->vx) + (m->m[1][1] * v->vy) + (m->m[2][1] * v->vz);
   tv.vz = (m->m[0][2] * v->vx) + (m->m[1][2] * v->vy) + (m->m[2][2] * v->vz);
   tv.vw = (m->m[0][3] * v->vx) + (m->m[1][3] * v->vy) + (m->m[2][3] * v->vz);
   
   BP_Vec4_Copy( res, &tv );

   bp_math_assert( BP_Vec4_Check(res) );
}

//----------------------------------------------------------------------------

// Assumes matrix:
// - only contains rotation and translation 
// - scale = 1,1,1
// - last column is {0,0,0,1}
EXTERN_INLINE 
void BP_Mat_FastInverse( FMATRIX *r, const FMATRIX *m )
{
#if BP_VITA && BP_USE_NEON
   static float const sk0001[] = { 0.0f, 0.0f, 0.0f, 1.0f };
   float *rf = (float *) r;
   float const *mf = (float const *) m;

   __vec_float4 zero1 = vec_ld(0, sk0001);
   __vec_float4 matpos = vec_ld(48, mf);

   vec_st(zero1, 48, mf);
   {
      // transpose the matrix rotation
      float32x4x4_t const transpose = vld4q_f32(mf);
      // replace the position
      vec_st(matpos, 48, mf);

      // Extract components
      __vec_float4 const x = vec_splat( matpos, 0 );
      __vec_float4 const y = vec_splat( matpos, 1 );
      __vec_float4 const z = vec_splat( matpos, 2 );

      // Inverse rotate
      __vec_float4 xfpoint = vec_mul( transpose.val[0], x );
      xfpoint = vec_madd( transpose.val[1], y, xfpoint );
      xfpoint = vec_madd( transpose.val[2], z, xfpoint );

      // negate
      xfpoint = vec_sub(zero1, xfpoint);

      // stuff back into matrix
      vec_st(transpose.val[0], 0, rf);
      vec_st(transpose.val[1], 16, rf);
      vec_st(transpose.val[2], 32, rf);
      vec_st(xfpoint, 48, rf);
   }
#else
   FMATRIX inv;

   FVECTOR* dstTrans = (FVECTOR*)&inv.m[3][0];
   const FVECTOR* srcTrans = (FVECTOR*)&m->m[3][0];

   bp_math_assert( BP_Mat_Check(m) );

   // Transpose rotation
   inv.m[0][0] = m->m[0][0];
   inv.m[0][1] = m->m[1][0];
   inv.m[0][2] = m->m[2][0];
   inv.m[0][3] = 0.0f;

   inv.m[1][0] = m->m[0][1];
   inv.m[1][1] = m->m[1][1];
   inv.m[1][2] = m->m[2][1];
   inv.m[1][3] = 0.0f;

   inv.m[2][0] = m->m[0][2];
   inv.m[2][1] = m->m[1][2];
   inv.m[2][2] = m->m[2][2];
   inv.m[2][3] = 0.0f;

   inv.m[3][0] = 0.0f;
   inv.m[3][1] = 0.0f;
   inv.m[3][2] = 0.0f;
   inv.m[3][3] = m->m[3][3];

   // Rotate and negate original translation through inverse rotation
   BP_Mat_RotateVec3( dstTrans, &inv, srcTrans );
   BP_Vec3_NegVec( dstTrans, dstTrans );

   // Copy to result
   BP_Mat_Copy( r, &inv );
#endif
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Mat_CalcSklMatrix( FMATRIX* jointMat, const FMATRIX* refMat, const FVECTOR* rootTrans )
{
   FVECTOR v;
   BP_Vec4_Copy( (FVECTOR*)&jointMat->m[0][0], (const FVECTOR*)&refMat->m[0][0] );
   BP_Vec4_Copy( (FVECTOR*)&jointMat->m[1][0], (const FVECTOR*)&refMat->m[1][0] );
   BP_Vec4_Copy( (FVECTOR*)&jointMat->m[2][0], (const FVECTOR*)&refMat->m[2][0] );
   BP_Mat_RotateVec4( &v, refMat, rootTrans );
   BP_Vec4_SubVec( (FVECTOR*)&jointMat->m[3][0], (const FVECTOR*)&refMat->m[3][0], &v );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Mat_ApplyRotX( FMATRIX *r, const FMATRIX *m, float angle )
{
   FMATRIX rot_mat;
   FVECTOR sin_cos;
   float s, c;
   
   BP_SinCos( &sin_cos, angle );
   s = sin_cos.vx;
   c = sin_cos.vy;

   bp_math_assert( BP_Mat_Check(m) );
   bp_math_assert( BP_Float_Check(angle) );

   BP_Mat_SetUnit( &rot_mat );
   
   rot_mat.m[1][1] = c;
   rot_mat.m[1][2] = s;
   rot_mat.m[2][1] = -s;
   rot_mat.m[2][2] = c;
   
   BP_Mat_MulMatrix( r, &rot_mat, m ) ;
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Mat_ApplyRotY( FMATRIX *r, const FMATRIX *m, float angle )
{
   FMATRIX rot_mat;
   FVECTOR sin_cos;
   float s, c;

   BP_SinCos( &sin_cos, angle );
   s = sin_cos.vx;
   c = sin_cos.vy;

   bp_math_assert( BP_Mat_Check(m) );
   bp_math_assert( BP_Float_Check(angle) );

   BP_Mat_SetUnit( &rot_mat );
   
   rot_mat.m[0][0] = c;
   rot_mat.m[0][2] = -s;
   rot_mat.m[2][0] = s;
   rot_mat.m[2][2] = c;
   
   BP_Mat_MulMatrix( r, &rot_mat, m ) ;
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Mat_ApplyRotZ( FMATRIX *r, const FMATRIX *m, float angle )
{
   FMATRIX rot_mat;
   FVECTOR sin_cos;
   float s, c;

   BP_SinCos( &sin_cos, angle );
   s = sin_cos.vx;
   c = sin_cos.vy;

   bp_math_assert( BP_Mat_Check(m) );
   bp_math_assert( BP_Float_Check(angle) );

   BP_Mat_SetUnit( &rot_mat );
   
   rot_mat.m[0][0] = c;
   rot_mat.m[0][1] = s;
   rot_mat.m[1][0] = -s;
   rot_mat.m[1][1] = c;

   BP_Mat_MulMatrix( r, &rot_mat, m ) ;
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Mat_ApplyRotXFast( FMATRIX *r, const FMATRIX *m, float angle )
{
   FMATRIX rot_mat;
   FVECTOR sin_cos;
   float s, c;

   BP_SinCosFast( &sin_cos, angle );
   s = sin_cos.vx;
   c = sin_cos.vy;

   bp_math_assert( BP_Mat_Check(m) );
   bp_math_assert( BP_Float_Check(angle) );

   BP_Mat_SetUnit( &rot_mat );

   rot_mat.m[1][1] = c;
   rot_mat.m[1][2] = s;
   rot_mat.m[2][1] = -s;
   rot_mat.m[2][2] = c;

   BP_Mat_MulMatrix( r, &rot_mat, m ) ;
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Mat_ApplyRotYFast( FMATRIX *r, const FMATRIX *m, float angle )
{
   FMATRIX rot_mat;
   FVECTOR sin_cos;
   float s, c;

   BP_SinCosFast( &sin_cos, angle );
   s = sin_cos.vx;
   c = sin_cos.vy;

   bp_math_assert( BP_Mat_Check(m) );
   bp_math_assert( BP_Float_Check(angle) );

   BP_Mat_SetUnit( &rot_mat );

   rot_mat.m[0][0] = c;
   rot_mat.m[0][2] = -s;
   rot_mat.m[2][0] = s;
   rot_mat.m[2][2] = c;

   BP_Mat_MulMatrix( r, &rot_mat, m ) ;
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Mat_ApplyRotZFast( FMATRIX *r, const FMATRIX *m, float angle )
{
   FMATRIX rot_mat;
   FVECTOR sin_cos;
   float s, c;

   BP_SinCosFast( &sin_cos, angle );
   s = sin_cos.vx;
   c = sin_cos.vy;

   bp_math_assert( BP_Mat_Check(m) );
   bp_math_assert( BP_Float_Check(angle) );

   BP_Mat_SetUnit( &rot_mat );

   rot_mat.m[0][0] = c;
   rot_mat.m[0][1] = s;
   rot_mat.m[1][0] = -s;
   rot_mat.m[1][1] = c;

   BP_Mat_MulMatrix( r, &rot_mat, m ) ;
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Mat_Transpose( FMATRIX *r, const FMATRIX *m )
{
   FMATRIX tm;

   bp_math_assert( BP_Mat_Check(m) );

   tm.m[0][0] = m->m[0][0], tm.m[1][1] = m->m[1][1] ;
   tm.m[2][2] = m->m[2][2], tm.m[3][3] = m->m[3][3] ;

   tm.m[0][1] = m->m[1][0], tm.m[1][0] = m->m[0][1] ;
   tm.m[0][2] = m->m[2][0], tm.m[2][0] = m->m[0][2] ;
   tm.m[0][3] = m->m[3][0], tm.m[3][0] = m->m[0][3] ;
   tm.m[1][2] = m->m[2][1], tm.m[2][1] = m->m[1][2] ;
   tm.m[1][3] = m->m[3][1], tm.m[3][1] = m->m[1][3] ;
   tm.m[2][3] = m->m[3][2], tm.m[3][2] = m->m[2][3] ;

   BP_Mat_Copy( r, &tm );
}

//----------------------------------------------------------------------------
// SHADER FUNCTIONS
//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Matrix44_to_VS_Matrix43(float const * input, float * output)
{
   output[0] = input[0];
   output[1] = input[4];
   output[2] = input[8];
   output[3] = input[12];

   output[4] = input[1];
   output[5] = input[5];
   output[6] = input[9];
   output[7] = input[13];

   output[8] = input[2];
   output[9] = input[6];
   output[10] = input[10];
   output[11] = input[14];
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Matrix44_to_VS_Matrix44(float const * input, float * output)
{
#if BP_VITA
   *((float32x4x4_t *) output) = vld4q_f32( (float32_t const *) input );
#else
   output[0] = input[0];
   output[1] = input[4];
   output[2] = input[8];
   output[3] = input[12];

   output[4] = input[1];
   output[5] = input[5];
   output[6] = input[9];
   output[7] = input[13];

   output[8] = input[2];
   output[9] = input[6];
   output[10] = input[10];
   output[11] = input[14];

   output[12] = input[3];
   output[13] = input[7];
   output[14] = input[11];
   output[15] = input[15];
#endif
}

//----------------------------------------------------------------------------
// SONY VU0 FUNCTIONS
//----------------------------------------------------------------------------

EXTERN_INLINE
void sceVu0ApplyMatrix(sceVu0FVECTOR v0, sceVu0FMATRIX m, sceVu0FVECTOR v1)
{
   BP_Mat_TransformVec4( (FVECTOR*)v0, (const FMATRIX*)m, (const FVECTOR*)v1 );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void sceVu0MulMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1, sceVu0FMATRIX m2)
{
   BP_Mat_MulMatrix( (FMATRIX*)m0, (const FMATRIX*)m1, (const FMATRIX*)m2 );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void sceVu0InversMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1)
{
   BP_Mat_FastInverse( (FMATRIX*)m0, (const FMATRIX*)m1 );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void  sceVu0UnitMatrix(sceVu0FMATRIX m)
{
   BP_Mat_SetUnit( (FMATRIX*)m );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void  sceVu0TransposeMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1)
{
   BP_Mat_Transpose( (FMATRIX*)m0, (const FMATRIX*)m1 );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void  sceVu0RotMatrixX(sceVu0FMATRIX m0, sceVu0FMATRIX m1, float rx)
{
   BP_Mat_ApplyRotX( (FMATRIX*)m0, (const FMATRIX*)m1, rx );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void  sceVu0RotMatrixY(sceVu0FMATRIX m0, sceVu0FMATRIX m1, float ry)
{
   BP_Mat_ApplyRotY( (FMATRIX*)m0, (const FMATRIX*)m1, ry );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void  sceVu0RotMatrixZ(sceVu0FMATRIX m0, sceVu0FMATRIX m1, float rz)
{
   BP_Mat_ApplyRotZ( (FMATRIX*)m0, (const FMATRIX*)m1, rz );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void  sceVu0RotMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1, sceVu0FVECTOR rot)
{
   sceVu0RotMatrixZ(m0, m1, rot[2]);
   sceVu0RotMatrixY(m0, m0, rot[1]);
   sceVu0RotMatrixX(m0, m0, rot[0]);
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void  sceVu0ViewScreenMatrix(sceVu0FMATRIX m, float scrz, float ax, float ay, 
                             float cx, float cy, float zmin, float zmax, float nearz, float farz)
{
   if( scrz != 0.0f )
   {
      float	az, cz;
      sceVu0FMATRIX	mt;

      cz = (-zmax * nearz + zmin * farz) / (-nearz + farz);
      az  = farz * nearz * (-zmin + zmax) / (-nearz + farz);

      //     | scrz    0  0 0 |
      // m = |    0 scrz  0 0 | 
      //     |    0    0  0 1 |
      //     |    0    0  1 0 |
      sceVu0UnitMatrix(m);
      m[0][0] = scrz;
      m[1][1] = scrz;
      m[2][2] = 0.0f;
      m[3][3] = 0.0f;
      m[3][2] = 1.0f;
      m[2][3] = 1.0f;

      //     | ax  0  0 cx |
      // m = |  0 ay  0 cy | 
      //     |  0  0 az cz |
      //     |  0  0  0  1 |
      sceVu0UnitMatrix(mt);
      mt[0][0] = ax;
      mt[1][1] = ay;
      mt[2][2] = az;
      mt[3][0] = cx;
      mt[3][1] = cy;
      mt[3][2] = cz;

      sceVu0MulMatrix(m, mt, m);
   }
   else
   {
      // 0.0f scrz is essentially a projection matrix
      // that can not see anything, we alter the numbers here
      // so that the bounding code will cull more objects when
      // nothing would end up drawing anyways, culling code projects
      // bound box through projection matrix and compares each component with
      // -w/w to try and get consistent clip flags
      sceVu0UnitMatrix(m);
      m[3][0] = 100000.0f;
      m[3][1] = 100000.0f;
      m[3][2] = 100000.0f;
   }
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void sceVu0CopyMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1)
{
   BP_Mat_Copy( (FMATRIX*)m0, (const FMATRIX*)m1 );
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void sceVu0TransMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1, sceVu0FVECTOR tv)
{
   BP_Mat_Copy( (FMATRIX*)m0, (const FMATRIX*)m1 );
   BP_Vec3_AddVec( (FVECTOR*)(&m0[3][0]), (const FVECTOR*)(&m1[3][0]), (const FVECTOR*)tv );
}

//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------------

#endif   //#ifndef __BP_MATRIX_H__
