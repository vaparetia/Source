//----------------------------------------------------------------------------
// bp_bound.h
//----------------------------------------------------------------------------
#ifndef __BP_BOUND_H__
#define __BP_BOUND_H__

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
// BOUNDS FUNCTIONS
//----------------------------------------------------------------------------

EXTERN_INLINE
int BP_Bound3_Check( const FVECTOR* min, const FVECTOR* max )
{
   return ( BP_Vec3_Check(min) && 
            BP_Vec3_Check(max) &&
            ( min->vx <= max->vx ) &&
            ( min->vy <= max->vy ) &&
            ( min->vz <= max->vz ) );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Bound3_Set( FVECTOR* min, FVECTOR* max, const FVECTOR* pt )
{
   BP_Vec4_Copy( min, pt );
   BP_Vec4_Copy( max, pt );
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
void BP_Bound3_Resize( FVECTOR* min, FVECTOR* max, const FVECTOR* pt )
{
   bp_math_assert( BP_Vec3_Check(max) );
   bp_math_assert( BP_Vec3_Check(min) );
   bp_math_assert( BP_Vec3_Check(pt) );

   // Update max bounds
   if( max->vx < pt->vx )
   {
      max->vx = pt->vx;
   }
   if( max->vy < pt->vy )
   {
      max->vy = pt->vy;
   }
   if( max->vz < pt->vz )
   {
      max->vz = pt->vz;
   }

   // Update min bounds
   if( min->vx > pt->vx )
   {
      min->vx = pt->vx;
   }
   if( min->vy > pt->vy )
   {
      min->vy = pt->vy;
   }
   if( min->vz > pt->vz )
   {
      min->vz = pt->vz;
   }
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
int BP_Bound3_PtInside( const FVECTOR* min, const FVECTOR* max, const FVECTOR* pt )
{
   return (pt->vx >= min->vx) && (pt->vx <= max->vx) &&
      (pt->vy >= min->vy) && (pt->vy <= max->vy) &&
      (pt->vz >= min->vz) && (pt->vz <= max->vz);
}

//----------------------------------------------------------------------------

EXTERN_INLINE 
int BP_Bound3_Overlap( const FVECTOR* bound_min0, const FVECTOR* bound_max0, 
                       const FVECTOR* bound_min1, const FVECTOR* bound_max1 )
{
   if( bound_min0->vx > bound_max1->vx )
      return 0;
   if( bound_min0->vy > bound_max1->vy )
      return 0;
   if( bound_max0->vy < bound_min1->vy )
      return 0;
   
   if( bound_max0->vx < bound_min1->vx )
      return 0;
   if( bound_min0->vz > bound_max1->vz )
      return 0;
   if( bound_max0->vz < bound_min1->vz )
      return 0;

   return 1;
}

//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------------

#endif   //#ifndef __BP_BOUND_H__
