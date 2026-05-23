//------------------------------------------------------------------------------------------
// CAxisAngle.inl
// Bluepoint
//------------------------------------------------------------------------------------------

#include <math.h>
#include "Engine/Math/CloseEnough.h"
#include "Engine/Streams/CInputStream.h"
#include "Engine/Streams/COutputStream.h"

//------------------------------------------------------------------------------------------

CAxisAngle::CAxisAngle(CInputStream& stream)
:  mAxis( stream )
{
}

CAxisAngle::CAxisAngle( const CVector3& axis )
:  mAxis( axis )
{
}

CAxisAngle::CAxisAngle(const CVector3& axis, const CAngle& angle )
:  mAxis( axis * angle.AsRadians() )
{
}

//------------------------------------------------------------------------------------------

void CAxisAngle::PutTo(COutputStream& stream) const
{
   mAxis.PutTo(stream);
}

//------------------------------------------------------------------------------------------

const CVector3 CAxisAngle::GetAxisNormalized() const
{
   return mAxis.Normalized();
}

const CVector3& CAxisAngle::GetAxis() const
{
   return mAxis;
}

//------------------------------------------------------------------------------------------

const CAxisAngle CAxisAngle::Identity()
{
   return CAxisAngle( CVector3::Zero() );
}

//------------------------------------------------------------------------------------------
