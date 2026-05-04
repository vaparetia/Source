//------------------------------------------------------------------------------------------
// CAxisAngle.cpp
// Bluepoint
// Copyright 2004
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CAxisAngle.h"
#include "CQuaternion.h"
#include "MathUtils.h"

//------------------------------------------------------------------------------------------

const CAngle CAxisAngle::GetAngle() const
{
   if( close_enough( mAxis.GetLengthSquared(), 0.0f ) )
      return CAngle::FromRadians( 0.0f );

   return CAngle::FromRadians( mAxis.GetLength() );
}

//------------------------------------------------------------------------------------------

CAxisAngle CAxisAngle::FromQuaternion(CQuaternion const &q)
{
   // Assume input quaternion is normalized
   real32 const angle = 2.0f * acosf(q.GetScalar());
   if (close_enough(angle, 0.0f)) return CAxisAngle(CVector3::Zero());

   real32 const ooScale = MathUtils::FastInvSqrt(1.0f - (q.GetScalar() * q.GetScalar())); // assuming quaternion normalised then w is less than 1, so term always positive.
   CVector3 const axis = (q.GetVector() * ooScale);
   return CAxisAngle(axis, CAngle(angle));
}

//------------------------------------------------------------------------------------------


