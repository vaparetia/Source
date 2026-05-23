//------------------------------------------------------------------------------------------
// CAABox.inl
// Bluepoint
//------------------------------------------------------------------------------------------

#include <math.h>
#include "Engine/Math/MathUtils.h"

//------------------------------------------------------------------------------------------

CAABox::CAABox( const CVector3& min, const CVector3& max )
:  mMin( min )
,  mMax( max )
{
}

//------------------------------------------------------------------------------------------

CVector3 const CAABox::GetClosestPointAlongDirection( CVector3 const & direction ) const
{
   real32 const dirX = direction.GetX();
   real32 const dirY = direction.GetY();
   real32 const dirZ = direction.GetZ();

   real32 const minX = mMin.GetX();
   real32 const minY = mMin.GetY();
   real32 const minZ = mMin.GetZ();
   real32 const maxX = mMax.GetX();
   real32 const maxY = mMax.GetY();
   real32 const maxZ = mMax.GetZ();

   real32 const resX = MathUtils::FSel(dirX, minX, maxX);
   real32 const resY = MathUtils::FSel(dirY, minY, maxY);
   real32 const resZ = MathUtils::FSel(dirZ, minZ, maxZ);

   return CVector3(resX, resY, resZ);
}

//----------------------------------------------------------------------------

CVector3 const CAABox::GetFurthestPointAlongDirection( CVector3 const & direction ) const
{
   real32 const dirX = direction.GetX();
   real32 const dirY = direction.GetY();
   real32 const dirZ = direction.GetZ();

   real32 const minX = mMin.GetX();
   real32 const minY = mMin.GetY();
   real32 const minZ = mMin.GetZ();
   real32 const maxX = mMax.GetX();
   real32 const maxY = mMax.GetY();
   real32 const maxZ = mMax.GetZ();

   real32 const resX = MathUtils::FSel(dirX, maxX, minX);
   real32 const resY = MathUtils::FSel(dirY, maxY, minY);
   real32 const resZ = MathUtils::FSel(dirZ, maxZ, minZ);

   return CVector3(resX, resY, resZ);
}

//----------------------------------------------------------------------------

bool const CAABox::InsidePlane( const CPlane& plane ) const
{
   CVector3 const & planeNormal = plane.GetNormal();

   CVector3 const vMin = GetClosestPointAlongDirection(planeNormal);
   return !plane.IsFacing( vMin );
}

