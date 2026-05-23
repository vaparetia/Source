//------------------------------------------------------------------------------------------
// CMatrix34.inl
// Bluepoint
//------------------------------------------------------------------------------------------

#include <math.h>

//------------------------------------------------------------------------------------------

CMatrix34::CMatrix34(real32 t00, real32 t01, real32 t02,
                                          real32 t10, real32 t11, real32 t12,
                                          real32 t20, real32 t21, real32 t22,
                                          real32 t30, real32 t31, real32 t32)
:  m00( t00 )
,  m01( t01 )
,  m02( t02 )
,  m10( t10 )
,  m11( t11 )
,  m12( t12 )
,  m20( t20 )
,  m21( t21 )
,  m22( t22 )
,  m30( t30 )
,  m31( t31 )
,  m32( t32 )
{
}

//------------------------------------------------------------------------------------------

CMatrix34::CMatrix34( CVector3 const & right, CVector3 const & up, CVector3 const & forward, CVector3 const & translation )
:  m00( right.GetX() )
,  m01( right.GetY() )
,  m02( right.GetZ() )
,  m10( up.GetX() )
,  m11( up.GetY() )
,  m12( up.GetZ() )
,  m20( forward.GetX() )
,  m21( forward.GetY() )
,  m22( forward.GetZ() )
,  m30( translation.GetX() )
,  m31( translation.GetY() )
,  m32( translation.GetZ() )
{
}

//------------------------------------------------------------------------------------------

CVector3 CMatrix34::operator *(CVector3 const & vector) const
{
	return CVector3(
		m00*vector.GetX() + m10*vector.GetY() + m20*vector.GetZ() + m30,
		m01*vector.GetX() + m11*vector.GetY() + m21*vector.GetZ() + m31,
		m02*vector.GetX() + m12*vector.GetY() + m22*vector.GetZ() + m32
		);
}

//------------------------------------------------------------------------------------------

CVector3 CMatrix34::Rotate( CVector3 const & vector ) const
{
	return CVector3(
		m00*vector.GetX() + m10*vector.GetY() + m20*vector.GetZ(),
		m01*vector.GetX() + m11*vector.GetY() + m21*vector.GetZ(),
		m02*vector.GetX() + m12*vector.GetY() + m22*vector.GetZ()
		);
}

//------------------------------------------------------------------------------------------

CVector3 CMatrix34::TransposeMultiply(CVector3 const &v) const
{
   CVector3 vector(v.GetX() - m30, v.GetY() - m31, v.GetZ() - m32);
   return CVector3(
      m00*vector.GetX() + m01*vector.GetY() + m02*vector.GetZ(),
      m10*vector.GetX() + m11*vector.GetY() + m12*vector.GetZ(),
      m20*vector.GetX() + m21*vector.GetY() + m22*vector.GetZ());
}

//------------------------------------------------------------------------------------------

CVector3 CMatrix34::TransposeRotate(CVector3 const &vector) const
{
   return CVector3(
      m00*vector.GetX() + m01*vector.GetY() + m02*vector.GetZ(),
      m10*vector.GetX() + m11*vector.GetY() + m12*vector.GetZ(),
      m20*vector.GetX() + m21*vector.GetY() + m22*vector.GetZ());
}

//------------------------------------------------------------------------------------------

CMatrix34 CMatrix34::operator * (const CMatrix34& matrix) const
{
   return CMatrix34(
      m00*matrix.m00 + m10*matrix.m01 + m20*matrix.m02,
      m01*matrix.m00 + m11*matrix.m01 + m21*matrix.m02,
      m02*matrix.m00 + m12*matrix.m01 + m22*matrix.m02,
                                                      
      m00*matrix.m10 + m10*matrix.m11 + m20*matrix.m12,
      m01*matrix.m10 + m11*matrix.m11 + m21*matrix.m12,
      m02*matrix.m10 + m12*matrix.m11 + m22*matrix.m12,
                                                      
      m00*matrix.m20 + m10*matrix.m21 + m20*matrix.m22,
      m01*matrix.m20 + m11*matrix.m21 + m21*matrix.m22,
      m02*matrix.m20 + m12*matrix.m21 + m22*matrix.m22,
                                                      
      m00*matrix.m30 + m10*matrix.m31 + m20*matrix.m32 + m30,
      m01*matrix.m30 + m11*matrix.m31 + m21*matrix.m32 + m31,
      m02*matrix.m30 + m12*matrix.m31 + m22*matrix.m32 + m32
      );
}

//------------------------------------------------------------------------------------------

void CMatrix34::MatrixMultiply_Inplace(CMatrix34 const &matA, CMatrix34 const &matB, CMatrix34 &matOut)
{
   matOut.m00 = matA.m00*matB.m00 + matA.m10*matB.m01 + matA.m20*matB.m02;
   matOut.m01 = matA.m01*matB.m00 + matA.m11*matB.m01 + matA.m21*matB.m02;
   matOut.m02 = matA.m02*matB.m00 + matA.m12*matB.m01 + matA.m22*matB.m02;
                                                           
   matOut.m10 = matA.m00*matB.m10 + matA.m10*matB.m11 + matA.m20*matB.m12;
   matOut.m11 = matA.m01*matB.m10 + matA.m11*matB.m11 + matA.m21*matB.m12;
   matOut.m12 = matA.m02*matB.m10 + matA.m12*matB.m11 + matA.m22*matB.m12;
                                                           
   matOut.m20 = matA.m00*matB.m20 + matA.m10*matB.m21 + matA.m20*matB.m22;
   matOut.m21 = matA.m01*matB.m20 + matA.m11*matB.m21 + matA.m21*matB.m22;
   matOut.m22 = matA.m02*matB.m20 + matA.m12*matB.m21 + matA.m22*matB.m22;
                                                           
   matOut.m30 = matA.m00*matB.m30 + matA.m10*matB.m31 + matA.m20*matB.m32 + matA.m30;
   matOut.m31 = matA.m01*matB.m30 + matA.m11*matB.m31 + matA.m21*matB.m32 + matA.m31;
   matOut.m32 = matA.m02*matB.m30 + matA.m12*matB.m31 + matA.m22*matB.m32 + matA.m32;
}

//------------------------------------------------------------------------------------------

real32 CMatrix34::Determinant() const
{
   return m00*(m11*m22 - m12*m21) + 
          m01*(m12*m20 - m10*m22) + 
          m02*(m10*m21 - m11*m20);
}

//------------------------------------------------------------------------------------------

CMatrix34 const & CMatrix34::Identity()
{
   return skIdentity;
}

//------------------------------------------------------------------------------------------

CMatrix34 CMatrix34::Translation(const CVector3& translation)
{
   return CMatrix34( real32(1.0), real32(0.0), real32(0.0),
                        real32(0.0), real32(1.0), real32(0.0),
                        real32(0.0), real32(0.0), real32(1.0),
                        translation.GetX(), translation.GetY(), translation.GetZ() );
}

//------------------------------------------------------------------------------------------

CMatrix34 CMatrix34::Scale(const CVector3& scale)
{
   return CMatrix34(
      scale.GetX(), real32(0.0), real32(0.0),
      real32(0.0), scale.GetY(), real32(0.0),
      real32(0.0), real32(0.0), scale.GetZ(),
      real32(0.0), real32(0.0), real32(0.0) );
}

//------------------------------------------------------------------------------------------

CMatrix34 CMatrix34::Scale( real32 const scale )
{
   return CMatrix34(
      scale, 0.0f, 0.0f,
      0.0f, scale, 0.0f,
      0.0f, 0.0f, scale,
      0.0f, 0.0f, 0.0f );
}

//------------------------------------------------------------------------------------------

CMatrix34 CMatrix34::RotateX(const CAngle& angle)
{
   const real32 sine = static_cast<real32>( sinf( static_cast<real32>( angle.AsRadians() ) ) );
   const real32 cos = static_cast<real32>( cosf( static_cast<real32>( angle.AsRadians() ) ) );

   return CMatrix34( real32(1.0), real32(0.0), real32(0.0),
                       real32(0.0), cos, sine,
                       real32(0.0), -sine, cos,
                       real32(0.0), real32(0.0), real32(0.0));
}

//------------------------------------------------------------------------------------------

CMatrix34 CMatrix34::RotateY(const CAngle& angle)
{
   const real32 sine = static_cast<real32>( sinf( static_cast<real32>( angle.AsRadians() ) ) );
   const real32 cos = static_cast<real32>( cosf( static_cast<real32>( angle.AsRadians() ) ) );

   return CMatrix34( cos, real32(0.0), -sine,
                       real32(0.0), real32(1.0), real32(0.0),
                       sine, real32(0.0), cos,
                       real32(0.0), real32(0.0), real32(0.0));
}

//------------------------------------------------------------------------------------------

CMatrix34 CMatrix34::RotateZ(const CAngle& angle)
{
   const real32 sine = static_cast<real32>( sinf( static_cast<real32>( angle.AsRadians() ) ) );
   const real32 cos = static_cast<real32>( cosf( static_cast<real32>( angle.AsRadians() ) ) );

   return CMatrix34( cos, sine, real32(0.0),
                       -sine, cos, real32(0.0),
                       real32(0.0), real32(0.0), real32(1.0),
                       real32(0.0), real32(0.0), real32(0.0));
}

//------------------------------------------------------------------------------------------

CVector3 CMatrix34::GetTranslation() const
{
   return CVector3( m30, m31, m32 );
}

//------------------------------------------------------------------------------------------

CVector3 CMatrix34::GetRow(uint32 const row) const
{
   BPE_ASSERT(row < 4, "Row out of range.");
   real32 const *pRow = ((real32 *) &m00) + (row * 3);
   real32 x = *pRow;    pRow++;
   real32 y = *pRow;    pRow++;
   real32 z = *pRow;
   return CVector3(x, y, z);
}

//------------------------------------------------------------------------------------------

CVector3 & CMatrix34::Row(uint32 const row)
{
   BPE_ASSERT(row < 4, "Row out of range.");
   real32 *pRow = ((real32 *) &m00) + (row * 3);
   return *reinterpret_cast<CVector3*>(pRow);
}

//------------------------------------------------------------------------------------------

void CMatrix34::SetTranslation( CVector3 const & translation )
{
   m30 = translation.GetX();
   m31 = translation.GetY();
   m32 = translation.GetZ();
}

//------------------------------------------------------------------------------------------

void CMatrix34::AddTranslation( CVector3 const &translationDelta )
{
   m30 += translationDelta.GetX();
   m31 += translationDelta.GetY();
   m32 += translationDelta.GetZ();   
}
  
//------------------------------------------------------------------------------------------

void CMatrix34::SetRow( uint32 const row, CVector3 const &value)
{
   BPE_ASSERT(row < 4, "Row out of range.");
   real32 *pRow = ((real32 *) &m00) + (row * 3);
   *pRow = value.mX;    pRow++;
   *pRow = value.mY;    pRow++; 
   *pRow = value.mZ;
}

//------------------------------------------------------------------------------------------

