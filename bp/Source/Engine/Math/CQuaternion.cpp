//------------------------------------------------------------------------------------------
// CQuaternion.cpp
// Bluepoint
// Copyright 2004
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CQuaternion.h"


//------------------------------------------------------------------------------------------

const CQuaternion CQuaternion::FromAxisAngle(const CAxisAngle& other)
{
   real32 const halfAngle = 0.5f * other.GetAngle().AsRadians();
   if( close_enough( halfAngle, 0.0f ) )
   {
      return CQuaternion::Identity();
   }
   else
   {
      real32 const sine = sinf( static_cast<real32>( halfAngle ) );
      real32 const scalar = cosf( static_cast<real32>( halfAngle ) );
   
      return CQuaternion( other.GetAxisNormalized() * sine, scalar );
   }
}

//------------------------------------------------------------------------------------------

const CQuaternion CQuaternion::FromAxisAngle(CVector3 const & axis, CAngle const angle )
{
   real32 const halfAngle = 0.5f * angle.AsRadians();
   if( close_enough( halfAngle, 0.0f ) )
   {
      return CQuaternion::Identity();
   }
   else
   {
      real32 const sine = sinf( static_cast<real32>( halfAngle ) );
      real32 const scalar = cosf( static_cast<real32>( halfAngle ) );

      return CQuaternion( axis * sine, scalar );
   }
}

//------------------------------------------------------------------------------------------

const CQuaternion CQuaternion::FromMatrix3( const CMatrix3& otherOrig )
{
   const CMatrix3 other = otherOrig.Transpose();

   const real32 trace = 1.0f + other.Get00() + other.Get11() + other.Get22();

   // Is the scalar value < 0.5f ?
   if( trace > 1.0f )
   {
      real32 w = sqrtf( trace ) / 2.0f;
      real32 fourD = 0.25f / w;
      real32 x = (other.Get21() - other.Get12()) * fourD;
      real32 y = (other.Get02() - other.Get20()) * fourD;
      real32 z = (other.Get10() - other.Get01()) * fourD;

      return CQuaternion( x, y, z, w );
   }
   else
   {
      int index = 0;
      if ( other.Get11 () > other.Get00 () )
      {
         if ( other.Get22 () > other.Get11 () )
            index = 2;
         else
            index = 1;
      }
      else
      {
         if ( other.Get22 () > other.Get00 () )
            index = 2;
         else
            index = 0;
      }

      if ( index == 0 )
      {
         // First component is biggest...
         real32 fourD = 2.0f * sqrtf( other.Get00() - other.Get11() - other.Get22() + 1.0f );

         real32 x = fourD / 4.0f;
         real32 y = (other.Get01() + other.Get10()) / fourD;
         real32 z = (other.Get02() + other.Get20()) / fourD;
         real32 w = (other.Get21() - other.Get12()) / fourD;

         return CQuaternion( CVector3( x, y, z ), w );
      }
      else if ( index == 1 )
      {
         // Second component is biggest...
         real32 fourD = 2.0f * sqrtf( other.Get11() - other.Get22() - other.Get00() + 1.0f );

         real32 y = fourD / 4.0f;
         real32 z = (other.Get12() + other.Get21()) / fourD;
         real32 x = (other.Get10() + other.Get01()) / fourD;
         real32 w = (other.Get02() - other.Get20()) / fourD;

         return CQuaternion( CVector3( x, y, z ), w );
      }
      else
      {
         // Third component is biggest...
         real32 fourD = 2.0f * sqrtf( other.Get22() - other.Get00() - other.Get11() + 1.0f );

         real32 z = fourD / 4.0f;
         real32 x = (other.Get20() + other.Get02()) / fourD;
         real32 y = (other.Get21() + other.Get12()) / fourD;
         real32 w = (other.Get10() - other.Get01()) / fourD;

         return CQuaternion( CVector3( x, y, z ), w );
      }
   }
}

//------------------------------------------------------------------------------------------

CQuaternion const CQuaternion::FromMatrix34( const CMatrix34& other )
{
   return CQuaternion::FromMatrix3( CMatrix3::FromMatrix34( other ) );
}

//------------------------------------------------------------------------------------------

CQuaternion const CQuaternion::FromMatrix4( const CMatrix4& other )
{
   return CQuaternion::FromMatrix3( CMatrix3::FromMatrix4( other ) );
}

//------------------------------------------------------------------------------------------
// Assumes input quaternions are normalized.
real32 CQuaternion::Angle(CQuaternion const & q1, CQuaternion const & q2)
{
   real32 const dot = q1.Dot(q2);
   real32 const angle = 2.0f * acosf(dot);

   return angle;
}

//------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------
// Don Hatch's version of sin(x)/x, which is accurate for very small x.
// Returns 1 for x == 0.
//--------------------------------------------------------------------------


static real32 sinx_over_x(real32 x)
{
   if ((x * x) < gkEpsilon32)
      return 1.0f;
   else
      return sin(x) / x;
}

//------------------------------------------------------------------------------------------

real32 CQuaternion::Angle4D(CQuaternion const & q1, CQuaternion const & q2)
{
   CQuaternion const d(q1 - q2);
   real32 const lengthD = d.GetLength();

   CQuaternion const s(q1 + q2);
   real32 const lengthS = s.GetLength();

   real32 const angle = 2.0f * atan2f(lengthD, lengthS);

   return angle;
}

//------------------------------------------------------------------------------------------
// Not a local slerp, use MakeLocalTo for local slerp
CQuaternion const CQuaternion::SlerpAccurate( CQuaternion const & q1, CQuaternion const & q2, real32 const t )
{
   real32 a = Angle4D(q1, q2);
   real32 s = 1 - t;

   CQuaternion q = q1 * sinx_over_x(s * a) / sinx_over_x(a) * s +
      q2 * sinx_over_x(t * a) / sinx_over_x(a) * t;
   q.Normalize();

   return q;
}

//------------------------------------------------------------------------------------------

CQuaternion const CQuaternion::SlerpAccurateLocal( CQuaternion const & q1, CQuaternion const & q2, real32 const t )
{
   CQuaternion q2Local(q2);
   q2Local.MakeLocalTo(q1);
   return SlerpAccurate(q1, q2Local, t);
}
//------------------------------------------------------------------------------------------

CQuaternion const CQuaternion::Squad(CQuaternion const & q1, CQuaternion const & qa, CQuaternion const & qb, CQuaternion const & q2, real32 const t)
{
   CQuaternion const r1(SlerpAccurate(q1, q2, t));
   CQuaternion const r2(SlerpAccurate(qa, qb, t));
   CQuaternion const result(SlerpAccurate(r1, r2, 2.0f * t * (1.0f - t)));
   return result;
}

//------------------------------------------------------------------------------------------

CQuaternion const CQuaternion::Spline(CQuaternion const & q0, CQuaternion const & q1, CQuaternion const & q2, CQuaternion const & q3, real32 const t)
{     
   CQuaternion const qa(Tangent(q0, q1, q2));   
   CQuaternion const qb(Tangent(q1, q2, q3));   
   return Squad(q1, qa, qb, q2, t);
}

//------------------------------------------------------------------------------------------

// Returns the quaternion logarithm.
CQuaternion const CQuaternion::Log() const
{
   real32 theta = acosf(bpe::min_val(GetScalar(), 1.0f));
   if (theta == 0)
      return CQuaternion(GetVector(), 0.0f);

   real32 sintheta = sinf(theta);

   real32 k;
   if ((fabsf(sintheta) < 1.0f) && (fabsf(theta) >= (gkReal32Max * fabsf(sintheta))))
      k = 0;
   else
      k = theta / sintheta;

   return CQuaternion(GetVector() * k, 0.0f);
} 

//------------------------------------------------------------------------------------------
// Returns the quaternion exponential.
CQuaternion const CQuaternion::Exp() const
{
   real32 theta = GetVector().GetLength();
   real32 sintheta = sinf(theta);

   real32 k;
   if ((fabsf(theta) < 1) && (fabsf(sintheta) >= (gkReal32Max * fabsf(theta))))
      k = 0;
   else
      k = sintheta / theta;

   real32 costheta = cosf(theta);

   return CQuaternion(GetVector() * k, costheta);
}

//------------------------------------------------------------------------------------------
// Returns a quaternion tangent, use with Squad
// http://www.csie.ntu.edu.tw/~b92009/zzz88213/rendering/src-1.02/OpenEXR/include/ImathQuat.h
//
CQuaternion const CQuaternion::Tangent(CQuaternion const &q0, CQuaternion const &q1, CQuaternion const &q2)
{
   CQuaternion const q1inv = q1.Inverse();
   CQuaternion const c1 = q1inv * q2;
   CQuaternion const c2 = q1inv * q0;
   CQuaternion const c3 = (c1.Log() + c2.Log()) * -0.25f;
   CQuaternion qa = q1 * c3.Exp();
   qa.Normalize();

   return qa;
}

//------------------------------------------------------------------------------------------

bool const CQuaternion::IsLocalTo( CQuaternion const & other ) const
{
   real32 const dot = Dot(other);
   return (dot >= 0.0f);
}

//------------------------------------------------------------------------------------------

void CQuaternion::MakeLocalTo( CQuaternion const & other )
{
   if (!IsLocalTo(other))
   {
      *this = BuildEquivalent(*this);
   }
   return;
}

//------------------------------------------------------------------------------------------

inline static float normalize_angle( float angle )
{
   static const float skHalf = CAngle::FromDegrees ( 180.f ).AsRadians ();

   if ( angle > skHalf )
      return angle - 2 * skHalf;
   else if ( angle < -skHalf )
      return angle + 2 * skHalf;
   else return angle;
}

CQuaternion const CQuaternion::LookAt( CVector3 const & src, CVector3 const & dst )
{
   /*
   static real32 const fudgeFactor = 0.001f;

   CVector3 const cross = src.Cross( dst );

   if( cross.GetLengthSquared() < fudgeFactor )
   {
   real32 const dot = src.Dot( dst );

   if( dot > 0.0f )
   {
   return CQuaternion::Identity();
   }
   else if( cross.GetLengthSquared() > gkEpsilon32 * gkEpsilon32 )
   {
   return CQuaternion( cross.Normalized(), 0.0f );
   }
   else if( close_enough( dot, -1.0f ) )
   {
   return CQuaternion::RotateY( CAngle::FromDegrees( 180.0f ) );
   }

   // At this point, there is nothing better we can do...
   return CQuaternion::Identity();
   }

   real32 const s = sqrtf( ( 1.0f + bpe::max_val( -1.f, bpe::min_val( src.Dot( dst ), 1.0f ) ) ) * 2.0f );

   CVector3 const vector = cross * ( 1.0f / s );

   CQuaternion const q = CQuaternion( vector, s * 0.5f );

   return q;
   */

   static const float skPivot = 0.0001f;

   CVector3 targetDir = dst;
   CVector3 waypointDir = src;

   targetDir[2] = 0.f;
   waypointDir[2] = 0.f;

   CQuaternion rot = CQuaternion::Identity();
   CVector3 toCross ( 0, 0, 1 );

   float waypointDirMag = waypointDir.GetLengthSquared ();
   float targetDirMag = targetDir.GetLengthSquared ();

   if ( waypointDirMag > skPivot && targetDirMag > skPivot )
   {
      waypointDir.Normalize ();
      targetDir.Normalize ();

      // Find the rotation between waypointDir and targetDir
      float rotWay = atan2f ( waypointDir.GetX (), waypointDir.GetY () );
      float rotTarget = atan2f ( targetDir.GetX (), targetDir.GetY () );

      float rotTotal = normalize_angle ( rotTarget - rotWay );

      rot = CQuaternion::RotateZ( CAngle::FromRadians( -rotTotal ) );
      toCross = ( rot * CQuaternion( waypointDir, 0.0f ) * rot.Inverse () ).GetVector ();
   }
   else if ( waypointDirMag > skPivot )
      toCross = waypointDir.Normalized ();
   else if ( targetDirMag > skPivot )
      toCross = targetDir.Normalized ();
   else
   {
      CVector3 targetDir = dst;
      CVector3 waypointDir = src;

      targetDir[1] = 0.f;
      waypointDir[1] = 0.f;

      CQuaternion rot = CQuaternion::Identity();
      CVector3 toCross ( 0, 1, 0 );

      float waypointDirMag = waypointDir.GetLengthSquared ();
      float targetDirMag = targetDir.GetLengthSquared ();

      if ( waypointDirMag > skPivot && targetDirMag > skPivot )
      {
         waypointDir.Normalize ();
         targetDir.Normalize ();

         // Find the rotation between waypointDir and targetDir
         float rotWay = atan2f ( waypointDir.GetX (), waypointDir.GetZ () );
         float rotTarget = atan2f ( targetDir.GetX (), targetDir.GetZ () );

         float rotTotal = normalize_angle ( rotTarget - rotWay );

         rot = CQuaternion::RotateY( CAngle::FromRadians( -rotTotal ) );
         toCross = ( rot * CQuaternion( waypointDir, 0.0f ) * rot.Inverse () ).GetVector ();
      }
      else if ( waypointDirMag > skPivot )
         toCross = waypointDir.Normalized ();
      else if ( targetDirMag > skPivot )
         toCross = targetDir.Normalized ();
      else
         return CQuaternion::Identity();

      CVector3 perp = CVector3::Cross( toCross, CVector3( 0, 1, 0 ) );
      float yangle = normalize_angle ( acosf ( dst.GetY () ) - acosf ( src.GetY () ) );
      CQuaternion rot2 = CQuaternion::FromAxisAngle( perp, CAngle::FromRadians ( -yangle ) );

      return rot2 * rot;
   }

   CVector3 perp = CVector3::Cross( toCross, CVector3( 0, 0, 1 ) );
   float zangle = normalize_angle ( acosf ( dst.GetZ () ) - acosf ( src.GetZ () ) );
   CQuaternion rot2 = CQuaternion::FromAxisAngle( CAxisAngle( perp, CAngle::FromRadians ( -zangle ) ) );

   return rot2 * rot;
}

//------------------------------------------------------------------------------------------


