//------------------------------------------------------------------------------------------
// CMatrix3.cpp
// Bluepoint
//
// Copyright 2001
//------------------------------------------------------------------------------------------

#include "StdAfx.h"

#include "Engine/Math/CMatrix3.h"

//------------------------------------------------------------------------------------------

CMatrix3::CMatrix3(CInputStream& stream)
:  m00(stream.ReadReal32())
,  m01(stream.ReadReal32())
,  m02(stream.ReadReal32())
,  m10(stream.ReadReal32())
,  m11(stream.ReadReal32())
,  m12(stream.ReadReal32())
,  m20(stream.ReadReal32())
,  m21(stream.ReadReal32())
,  m22(stream.ReadReal32())
{
}

//------------------------------------------------------------------------------------------
// Removed unneeded output code when compiling for SPU's
#if !defined(SPU)

void CMatrix3::PutTo(COutputStream& stream) const
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
}

//------------------------------------------------------------------------------------------

CMatrix3::CMatrix3(std::string const & value)
{
   int const numArgs = sscanf( value.c_str(), "%f %f %f %f %f %f %f %f %f", &m00, &m01, &m02, &m10, &m11, &m12, &m20, &m21, &m22 );
   BPE_VERIFY( numArgs  == 9, false, "invalid string" );
}

//------------------------------------------------------------------------------------------

std::string const CMatrix3::ToString() const
{
   return CStringExtras::Stringize( "%f %f %f %f %f %f %f %f %f", m00, m01, m02, m10, m11, m12, m20, m21, m22 );
}

#endif
//------------------------------------------------------------------------------------------

const CMatrix3 CMatrix3::Inverse() const
{
   const real32 determinant = m00*(m11*m22 - m12*m21) + m01*(m12*m20 - m10*m22) + m02*(m10*m21 - m11*m20);

   const real32 oneOverDeterminant = 1.0f / determinant;

   return CMatrix3( (m11*m22-m12*m21) * oneOverDeterminant, -(m01*m22-m02*m21) * oneOverDeterminant,  (m01*m12-m02*m11) * oneOverDeterminant,
                   -(m10*m22-m12*m20) * oneOverDeterminant,  (m00*m22-m02*m20) * oneOverDeterminant, -(m00*m12-m02*m10) * oneOverDeterminant,
                    (m10*m21-m11*m20) * oneOverDeterminant, -(m00*m21-m01*m20) * oneOverDeterminant,  (m00*m11-m01*m10) * oneOverDeterminant );

}

//------------------------------------------------------------------------------------------

const CMatrix3 CMatrix3::Scale(const CVector3& scale)
{
   return CMatrix3(
      scale.GetX(), real32(0.0), real32(0.0), 
      real32(0.0), scale.GetY(), real32(0.0), 
      real32(0.0), real32(0.0), scale.GetZ());
}

//------------------------------------------------------------------------------------------

const CMatrix3 CMatrix3::RotateX(const CAngle& angle)
{
   const real32 sine = static_cast<real32>( sinf( static_cast<real32>( angle.AsRadians() ) ) );
   const real32 cos = static_cast<real32>( cosf( static_cast<real32>( angle.AsRadians() ) ) );

   return CMatrix3( real32(1.0), real32(0.0), real32(0.0),
                    real32(0.0), cos, sine, 
                    real32(0.0), -sine, cos );
}

//------------------------------------------------------------------------------------------

const CMatrix3 CMatrix3::RotateY(const CAngle& angle)
{
   const real32 sine = static_cast<real32>( sinf( static_cast<real32>( angle.AsRadians() ) ) );
   const real32 cos = static_cast<real32>( cosf( static_cast<real32>( angle.AsRadians() ) ) );

   return CMatrix3( cos, real32(0.0), -sine,
                    real32(0.0), real32(1.0), real32(0.0),
                    sine, real32(0.0), cos );
}

//------------------------------------------------------------------------------------------

const CMatrix3 CMatrix3::RotateZ(const CAngle& angle)
{
   const real32 sine = static_cast<real32>( sinf( static_cast<real32>( angle.AsRadians() ) ) );
   const real32 cos = static_cast<real32>( cosf( static_cast<real32>( angle.AsRadians() ) ) );

   return CMatrix3( cos, sine, real32(0.0),
                    -sine, cos, real32(0.0),
                    real32(0.0), real32(0.0), real32(1.0) );
}

//------------------------------------------------------------------------------------------

CMatrix3& CMatrix3::OrthoNormalize()
{
   CVector3 x( m00, m01, m02 );
   CVector3 y( m10, m11, m12 );

   x.Normalize();

   CVector3 const z( CVector3::Cross( x, y ).Normalized() );
   y = CVector3::Cross( z, x ).Normalized();
   
   m00 = x[0]; m01 = x[1]; m02 = x[2];
   m10 = y[0]; m11 = y[1]; m12 = y[2];
   m20 = z[0]; m21 = z[1]; m22 = z[2];
   
   return *this;
}

//------------------------------------------------------------------------------------------

const CMatrix3 CMatrix3::OrthoNormalized() const
{
   CMatrix3 temp = *this;
   temp.OrthoNormalize();
   return temp;
}

//------------------------------------------------------------------------------------------


