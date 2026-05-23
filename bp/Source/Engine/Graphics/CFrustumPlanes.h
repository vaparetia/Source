//------------------------------------------------------------------------------------------
// CFrustumPlanes.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Primitives/CPlane.h"
#include <vector>

//------------------------------------------------------------------------------------------

class CMatrix4;
class CAngle;
class CAABox;

//------------------------------------------------------------------------------------------

class CFrustumPlanes
{
public:
   ENGINE_API explicit CFrustumPlanes( CMatrix4 const & cameraTransform,
                                       CAngle const & fov, 
                                       real32 const aspect,
                                       real32 const nearClipPlane, 
                                       bool const addFarClippingPlane = false,
                                       real32 const farClipPlane = 1000.0f );

   CPlane const & GetPlane( int i ) const { return mPlanes[i]; }
   int const GetPlaneCount() const { return mPlanes.size(); }

   ENGINE_API bool const IsBoxInFrustum( CAABox const & box ) const;
   ENGINE_API bool const IsPointInFrustum( CVector3 const & point ) const;

private:
   bpe::reserved_vector<CPlane, 6>   mPlanes;
};

//------------------------------------------------------------------------------------------

