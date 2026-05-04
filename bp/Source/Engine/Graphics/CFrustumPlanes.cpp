//------------------------------------------------------------------------------------------
// CFrustumPlanes.cpp
// Bluepoint
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Graphics/CFrustumPlanes.h"

//------------------------------------------------------------------------------------------

#include "Engine/Primitives/CAABox.h"
#include "Engine/Math/CAngle.h"
#include "Engine/Math/CMatrix4.h"

//------------------------------------------------------------------------------------------

CFrustumPlanes::CFrustumPlanes( CMatrix4 const & cameraTransform,
                                CAngle const & fov, 
                                real32 const aspect,
                                real32 const nearClipPlane, 
                                bool const addFarClippingPlane,
                                real32 const farClipPlane )
{
   real32 const zNear = nearClipPlane;

	real32 const yHypotenuse = zNear / static_cast<real32>( cos( fov.AsRadians() / 2.0f ) );
	real32 const yDelta      = static_cast<real32>( sin( fov.AsRadians() / 2.0f ) ) * yHypotenuse;

	real32 const xHypotenuse = zNear / static_cast<real32>( cos( fov.AsRadians() / 2.0f * aspect ) );
	real32 const xDelta      = static_cast<real32>( sin( fov.AsRadians() / 2.0f * aspect ) ) * xHypotenuse;

	// generate these points in camera space
	CVector3 const nearPlaneCorners[4] = { CVector3( xDelta, -yDelta, zNear ),  // top right corner
                  								CVector3( xDelta, yDelta, zNear ),  // bottom right corner
                  								CVector3( -xDelta, yDelta, zNear ),  // bottom left corner
                  								CVector3( -xDelta, -yDelta, zNear )  // top left corner
                  								};

	// Rotate these points into world space from camera space
	CVector3 const nearPlaneCornersWorld[4] = { CVector3( cameraTransform * nearPlaneCorners[0] ),
                                               CVector3( cameraTransform * nearPlaneCorners[1] ),
                                               CVector3( cameraTransform * nearPlaneCorners[2] ),
                                               CVector3( cameraTransform * nearPlaneCorners[3] )
                                             };

   CVector3 const originPointWorld( cameraTransform * CVector3::Zero() );

	// construct planes in world space now

	// ALL NORMALS POINT OUTWARD!  This means we specify all points CCW with normal pointing towards you.
   mPlanes.push_back( CPlane( nearPlaneCornersWorld[0], nearPlaneCornersWorld[1], nearPlaneCornersWorld[2] ) ); // near plane
   mPlanes.push_back( CPlane( originPointWorld, nearPlaneCornersWorld[0], nearPlaneCornersWorld[1] ) ); // right plane
	mPlanes.push_back( CPlane( originPointWorld, nearPlaneCornersWorld[2], nearPlaneCornersWorld[3] ) ); // left plane
	mPlanes.push_back( CPlane( originPointWorld, nearPlaneCornersWorld[3], nearPlaneCornersWorld[0] ) ); // top plane
	mPlanes.push_back( CPlane( originPointWorld, nearPlaneCornersWorld[1], nearPlaneCornersWorld[2] ) ); // bottom plane
   
   // Far clipping plane
   if( addFarClippingPlane )
   {
   	mPlanes.push_back( CPlane( -mPlanes[0].GetConstant() + farClipPlane, -mPlanes[0].GetNormal() ) );
   }
   
}

//------------------------------------------------------------------------------------------

bool const CFrustumPlanes::IsPointInFrustum( CVector3 const & point ) const
{
   for( uint32 i = 0; i < mPlanes.size(); ++i )
   {
      CPlane const & plane = mPlanes[i];
      if( plane.IsFacing( point ) )
         return false;
   }
   return true;
}

//------------------------------------------------------------------------------------------

bool const CFrustumPlanes::IsBoxInFrustum( CAABox const & box ) const
{
   for( uint32 i = 0; i < mPlanes.size(); ++i )
   {
      CPlane const & plane = mPlanes[i];
      if( !box.InsidePlane( plane ) )
         return false;
   }

   return true;
}
