//------------------------------------------------------------------------------------------
// CMatrix34.cpp
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Math/CMatrix34.h"

//------------------------------------------------------------------------------------------

#include <math.h>

#include "Engine/Math/BPEMath.h"

#include "Engine/Streams/CInputStream.h"
#include "Engine/Streams/COutputStream.h"

//------------------------------------------------------------------------------------------

CMatrix34 const CMatrix34::skIdentity( 1, 0, 0,
                                       0, 1, 0,
                                       0, 0, 1,
                                       0, 0, 0);

//------------------------------------------------------------------------------------------
// Removed unneeded output code when compiling for SPU's
#if !defined(SPU)

CMatrix34::CMatrix34(CInputStream& stream)
:  m00(stream.ReadReal32())
,  m01(stream.ReadReal32())
,  m02(stream.ReadReal32())
,  m10(stream.ReadReal32())
,  m11(stream.ReadReal32())
,  m12(stream.ReadReal32())
,  m20(stream.ReadReal32())
,  m21(stream.ReadReal32())
,  m22(stream.ReadReal32())
,  m30(stream.ReadReal32())
,  m31(stream.ReadReal32())
,  m32(stream.ReadReal32())
{
}

//------------------------------------------------------------------------------------------

void CMatrix34::PutTo(COutputStream& stream) const
{
   stream.WriteReal32(m00);
   stream.WriteReal32(m01);
   stream.WriteReal32(m02);
   stream.WriteReal32(m10);
   stream.WriteReal32(m11);
   stream.WriteReal32(m12);
   stream.WriteReal32(m20);
   stream.WriteReal32(m21);
   stream.WriteReal32(m22);
   stream.WriteReal32(m30);
   stream.WriteReal32(m31);
   stream.WriteReal32(m32);
}

//------------------------------------------------------------------------------------------

CMatrix34 CMatrix34::Slerp( CMatrix34 const & src, CMatrix34 const & dest, real32 const t )
{
   CVector3 const translation(CVector3::Lerp(src.GetTranslation(), dest.GetTranslation(), t));
   CQuaternion const rotation(CQuaternion::Slerp(CQuaternion::FromMatrix34(src), CQuaternion::FromMatrix34(dest), t));
   CMatrix34 newTransform(CMatrix34::FromQuaternion(rotation));
   newTransform.SetTranslation(translation);
   return newTransform;
}

#endif
//------------------------------------------------------------------------------------------

CMatrix34 CMatrix34::Inverse() const
{
   real32 const det = Determinant();
   real32 const oneOverDeterminant = 1.0f / det;
   
   real32 const transX = oneOverDeterminant * (m10*(m22*m31 - m21*m32) + m11*(m20*m32 - m22*m30) + m12*(m21*m30 - m20*m31));
   real32 const transY = oneOverDeterminant * (m20*(m02*m31 - m01*m32) + m21*(m00*m32 - m02*m30) + m22*(m01*m30 - m00*m31));
   real32 const transZ = oneOverDeterminant * (m30*(m02*m11 - m01*m12) + m31*(m00*m12 - m02*m10) + m32*(m01*m10 - m00*m11));

   return CMatrix34( (m11*m22-m12*m21) * oneOverDeterminant, -(m01*m22-m02*m21) * oneOverDeterminant,  (m01*m12-m02*m11) * oneOverDeterminant,
                    -(m10*m22-m12*m20) * oneOverDeterminant,  (m00*m22-m02*m20) * oneOverDeterminant, -(m00*m12-m02*m10) * oneOverDeterminant,
                     (m10*m21-m11*m20) * oneOverDeterminant, -(m00*m21-m01*m20) * oneOverDeterminant,  (m00*m11-m01*m10) * oneOverDeterminant,
                     transX, transY, transZ );
}

//------------------------------------------------------------------------------------------

void CMatrix34::Orthonormalize()
{
   CVector3 const normalizedLeft(GetLeft().Normalized());
   CVector3 const &forward = GetForward();

   CVector3 const up(CVector3::Cross(forward, normalizedLeft));
   CVector3 const normalizedUp(up.Normalized());

   CVector3 const normalizedForward(CVector3::Cross(normalizedLeft, normalizedUp));

   SetLeft(normalizedLeft);
   SetUp(normalizedUp);
   SetForward(normalizedForward);
}

//------------------------------------------------------------------------------------------

CMatrix34 CMatrix34::Orthonormalized() const
{
   CMatrix34 ortho(*this);
   ortho.Orthonormalize();
   return ortho;
}

//------------------------------------------------------------------------------------------

CMatrix34 CMatrix34::LookAt( CVector3 const & sourcePosition, CVector3 const & destPosition, CVector3 const &up)
{
   CVector3 newForward(destPosition - sourcePosition);
   CVector3 newUp(up);
   real32 const newForwardLength = newForward.GetLength();
   if (newForwardLength < gkEpsilon32 )
   {
      if (newUp.GetDominantAxis() == CVector3::kAxis_X)
      {
         newForward = CVector3::YAxis();
      }
      else if (newUp.GetDominantAxis() == CVector3::kAxis_Y)
      {
         newForward = CVector3::ZAxis();
      }
      else if (newUp.GetDominantAxis() == CVector3::kAxis_Z)
      {
         newForward = CVector3::YAxis();
      }
   }
   else
   {
      newForward /= newForwardLength;
   }
   real32 const forwardUpDot = CVector3::Dot(newForward, newUp);
   if (close_enough(fabsf(forwardUpDot), 1.0f))
   {
      // Forward and up have the same axis
      if (newUp.GetDominantAxis() == CVector3::kAxis_X)
      {
         newUp = CVector3::YAxis();
      }
      else if (newUp.GetDominantAxis() == CVector3::kAxis_Y)
      {
         newUp = CVector3::ZAxis();
      }
      else if (newUp.GetDominantAxis() == CVector3::kAxis_Z)
      {
         newUp = CVector3::XAxis();
      }

   }

   CVector3 const newRight = CVector3::Cross( newUp, newForward ).Normalized();
   newUp = CVector3::Cross( newForward, newRight ).Normalized();
   return CMatrix34( newRight, newUp, newForward, sourcePosition );
}

//------------------------------------------------------------------------------------------

CMatrix34 CMatrix34::MakeRotationFromTo(CVector3 const &unnormalizedFrom, CVector3 const &unnormalizedTo)
{
   /*
   * A function for creating a rotation matrix that rotates a vector called
   * "from" into another vector called "to".
   * Input : from[3], to[3] which both must be *normalized* non-zero vectors
   * Output: mtx[3][3] -- a 3x3 matrix in column-major form
   * Authors: Tomas Möller, John Hughes
   *          "Efficiently Building a Matrix to Rotate One Vector to Another"
   *          Journal of Graphics Tools, 4(4):1-4, 1999
   */

   // original comments above.
   // function has been modified to take non-normalized vectors and output a row-major matrix

   CVector3 from = unnormalizedFrom;
   CVector3 to = unnormalizedTo;

   if (from.TryNormalize() > 0.0f && to.TryNormalize() > 0.0f)
   {
      float const e = CVector3::Dot(from, to);
      float const f = fabsf(e);

      if (f < 1.0 - gkEpsilon32)  /* the most common case, unless "from"="to", or "from"=-"to" */
      {
         CVector3 v = CVector3::Cross(from, to);

         /* h = (1.0 - e)/DOT(v, v); old code */
         float const h = 1.0f/(1.0f + e);      /* optimization by Gottfried Chen */
         float const hvx = h * v[0];
         float const hvz = h * v[2];
         float const hvxy = hvx * v[1];
         float const hvxz = hvx * v[2];
         float const hvyz = hvz * v[1];

         return CMatrix34(
            e + hvx * v[0],
            hvxy + v[2],
            hvxz - v[1],

            hvxy - v[2],
            e + h * v[1] * v[1],
            hvyz + v[0],

            hvxz + v[1],
            hvyz - v[0],
            e + hvz * v[2],

            0.0f,
            0.0f,
            0.0f);
      }
      else     /* "from" and "to"-vector almost parallel */
      {
         int const minorAxis = from.Abs().GetMinorAxis();
         CVector3 x = CVector3::Zero();
         x[minorAxis] = 1.0f;

         CVector3 const u = x - from;
         CVector3 const v = x - to;

         float const c1 = 2.0f/CVector3::Dot(u, u);
         float const c2 = 2.0f/CVector3::Dot(v, v);
         float const c3 = c1*c2*CVector3::Dot(u, v);

         return CMatrix34(
            1.0f  - c1*u[0]*u[0] - c2*v[0]*v[0] + c3*v[0]*u[0],
                  - c1*u[1]*u[0] - c2*v[1]*v[0] + c3*v[1]*u[0],
                  - c1*u[2]*u[0] - c2*v[2]*v[0] + c3*v[2]*u[0],

                  - c1*u[0]*u[1] - c2*v[0]*v[1] + c3*v[0]*u[1],
            1.0f  - c1*u[1]*u[1] - c2*v[1]*v[1] + c3*v[1]*u[1],
                  - c1*u[2]*u[1] - c2*v[2]*v[1] + c3*v[2]*u[1],

                  - c1*u[0]*u[2] - c2*v[0]*v[2] + c3*v[0]*u[2],
                  - c1*u[1]*u[2] - c2*v[1]*v[2] + c3*v[1]*u[2],
            1.0f  - c1*u[2]*u[2] - c2*v[2]*v[2] + c3*v[2]*u[2],

            0.0f,
            0.0f,
            0.0f);
      }
   }

   return CMatrix34::Identity();
}

//------------------------------------------------------------------------------------------

bool const CMatrix34::operator == ( CMatrix34 const & rhs ) const
{
   return memcmp( this, &rhs, sizeof(CMatrix34) ) == 0;
}

//------------------------------------------------------------------------------------------

CMatrix34 CMatrix34::QuickInverse() const
{
   // transpose, negate translation
   CVector3 translation = -TransposeRotate(CVector3(m30, m31, m32));
   return CMatrix34(m00, m10, m20, m01, m11, m21, m02, m12, m22, translation[0], translation[1], translation[2]);
}

//------------------------------------------------------------------------------------------

CMatrix34 CMatrix34::QuickRotationInverse() const
{
   // just return the transpose
   return CMatrix34(m00, m10, m20, m01, m11, m21, m02, m12, m22, 0.0f, 0.0f, 0.0f);
}

//------------------------------------------------------------------------------------------

CMatrix34 CMatrix34::Rotate( CMatrix34 const & rotation ) const
{
   return CMatrix34(
      m00*rotation.m00 + m10*rotation.m01 + m20*rotation.m02,
      m01*rotation.m00 + m11*rotation.m01 + m21*rotation.m02,
      m02*rotation.m00 + m12*rotation.m01 + m22*rotation.m02,
                                                      
      m00*rotation.m10 + m10*rotation.m11 + m20*rotation.m12,
      m01*rotation.m10 + m11*rotation.m11 + m21*rotation.m12,
      m02*rotation.m10 + m12*rotation.m11 + m22*rotation.m12,
                                                      
      m00*rotation.m20 + m10*rotation.m21 + m20*rotation.m22,
      m01*rotation.m20 + m11*rotation.m21 + m21*rotation.m22,
      m02*rotation.m20 + m12*rotation.m21 + m22*rotation.m22,
      
      0,
      0,
      0                                                      
      );

}

//------------------------------------------------------------------------------------------

void CMatrix34::SetRotation( CMatrix34 const & rotation )
{
   m00 = rotation.Get00();
   m01 = rotation.Get01();
   m02 = rotation.Get02();
   
   m10 = rotation.Get10();
   m11 = rotation.Get11();
   m12 = rotation.Get12();
   
   m20 = rotation.Get20();
   m21 = rotation.Get21();
   m22 = rotation.Get22();
}

//------------------------------------------------------------------------------------------

void CMatrix34::SetRotation( CMatrix3 const & rotation )
{
   m00 = rotation.Get00();
   m01 = rotation.Get01();
   m02 = rotation.Get02();
   
   m10 = rotation.Get10();
   m11 = rotation.Get11();
   m12 = rotation.Get12();
   
   m20 = rotation.Get20();
   m21 = rotation.Get21();
   m22 = rotation.Get22();
}

//------------------------------------------------------------------------------------------

void CMatrix34::SetRotation( CMatrix4 const & rotation )
{
   m00 = rotation.Get00();
   m01 = rotation.Get01();
   m02 = rotation.Get02();
   
   m10 = rotation.Get10();
   m11 = rotation.Get11();
   m12 = rotation.Get12();
   
   m20 = rotation.Get20();
   m21 = rotation.Get21();
   m22 = rotation.Get22();
}

//------------------------------------------------------------------------------------------
