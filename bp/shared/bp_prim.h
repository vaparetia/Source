//----------------------------------------------------------------------------
// bp_prim.h
//----------------------------------------------------------------------------
#ifndef __BP_PRIM_H__
#define __BP_PRIM_H__

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
#include "libdg.h"

//----------------------------------------------------------------------------
// PRIM FUNCTIONS
//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Prim_ProjectUV( FVECTOR* out, const FVECTOR* in, const FVECTOR* shiftScreen )
{
    // Compute 1/w
    const float w = (in->vw < 0.0f) ? -in->vw : +in->vw;
    const float q = (w < 0.000001f) ? 0.0f : (shiftScreen->vz / w);
    
    // Project uv
    out->vx = shiftScreen->vx + (in->vx * q);
    out->vy = shiftScreen->vy + (in->vy * q);
    out->vz = (in->vz * q);
    out->vw = (in->vw * q);
}

//----------------------------------------------------------------------------

EXTERN_INLINE
void BP_Prim_ProjectUVNormal( FVECTOR* out, const FVECTOR* in, const FVECTOR* norm, const FVECTOR* shiftScreen )
{
    // Compute 1/w
    const float w = (in->vw < 0.0f) ? -in->vw : +in->vw;
    const float q = (w < 0.000001f) ? 0.0f : (shiftScreen->vz / w);

    // Project uv
    out->vx = norm->vx + shiftScreen->vx + (in->vx * q);
    out->vy = norm->vy + shiftScreen->vy + (in->vy * q);
    out->vz = (in->vz * q);
    out->vw = (in->vw * q);
}

//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------------

#endif   //#ifndef __BP_PRIM_H__
