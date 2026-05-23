//------------------------------------------------------------------------------------------
// CMatrix3.inl
// Bluepoint
// 
// Copyright 2001
//------------------------------------------------------------------------------------------

CMatrix3::CMatrix3(CVector3 const & right,
                   CVector3 const & up,
                   CVector3 const & forward )
:  m00( right.mX )
,  m01( right.mY )
,  m02( right.mZ )
,  m10( up.mX )
,  m11( up.mY )
,  m12( up.mZ )
,  m20( forward.mX )
,  m21( forward.mY )
,  m22( forward.mZ )
{
}

//------------------------------------------------------------------------------------------

CMatrix3::CMatrix3(real32 t00, real32 t01, real32 t02,
                   real32 t10, real32 t11, real32 t12,
                   real32 t20, real32 t21, real32 t22)
:  m00( t00 )
,  m01( t01 )
,  m02( t02 )
,  m10( t10 )
,  m11( t11 )
,  m12( t12 )
,  m20( t20 )
,  m21( t21 )
,  m22( t22 )
{
}

//------------------------------------------------------------------------------------------

const CVector3 CMatrix3::operator *(const CVector3& vector) const
{
	return CVector3(
		m00*vector.GetX() + m10*vector.GetY() + m20*vector.GetZ(),
		m01*vector.GetX() + m11*vector.GetY() + m21*vector.GetZ(),
		m02*vector.GetX() + m12*vector.GetY() + m22*vector.GetZ()
		);
}

//------------------------------------------------------------------------------------------

const CMatrix3 CMatrix3::operator * (const CMatrix3& matrix) const
{
   return CMatrix3(
      m00*matrix.m00 + m10*matrix.m01 + m20*matrix.m02,
      m01*matrix.m00 + m11*matrix.m01 + m21*matrix.m02,
      m02*matrix.m00 + m12*matrix.m01 + m22*matrix.m02,
                                                      
      m00*matrix.m10 + m10*matrix.m11 + m20*matrix.m12,
      m01*matrix.m10 + m11*matrix.m11 + m21*matrix.m12,
      m02*matrix.m10 + m12*matrix.m11 + m22*matrix.m12,
                                                      
      m00*matrix.m20 + m10*matrix.m21 + m20*matrix.m22,
      m01*matrix.m20 + m11*matrix.m21 + m21*matrix.m22,
      m02*matrix.m20 + m12*matrix.m21 + m22*matrix.m22
      );
}

//------------------------------------------------------------------------------------------

const CMatrix3 CMatrix3::operator * (const real32 scalar) const
{
   return CMatrix3( m00 * scalar, m01 * scalar, m02 * scalar,
                    m01 * scalar, m11 * scalar, m12 * scalar,
                    m02 * scalar, m21 * scalar, m22 * scalar );
}

//------------------------------------------------------------------------------------------

const CMatrix3 CMatrix3::Transpose() const
{
   return CMatrix3( m00, m10, m20,
                    m01, m11, m21,
                    m02, m12, m22);
}

//------------------------------------------------------------------------------------------

const CMatrix3 CMatrix3::operator + ( const CMatrix3& matrix) const
{
   return CMatrix3( m00 + matrix.m00, m01 + matrix.m01, m02 + matrix.m02, 
                    m01 + matrix.m00, m11 + matrix.m11, m12 + matrix.m12, 
                    m02 + matrix.m00, m21 + matrix.m21, m22 + matrix.m22 );
}

//------------------------------------------------------------------------------------------

const CMatrix3 CMatrix3::Identity()
{
   return CMatrix3( real32(1.0), real32(0.0), real32(0.0), 
                    real32(0.0), real32(1.0), real32(0.0),  
                    real32(0.0), real32(0.0), real32(1.0));
}

//------------------------------------------------------------------------------------------

const CVector3 CMatrix3::GetRow(uint32 const row) const
{
   BPE_ASSERT(row < 3, "Row out of range.");
   real32 const *pRow = ((real32 *) &m00) + (row * 3);
   real32 x = *pRow;    pRow++;
   real32 y = *pRow;    pRow++;
   real32 z = *pRow;
   return CVector3(x, y, z);
}

//------------------------------------------------------------------------------------------

void CMatrix3::SetRow( uint32 const row, CVector3 const &value)
{
   BPE_ASSERT(row < 3, "Row out of range.");
   real32 *pRow = ((real32 *) &m00) + (row * 3);
   *pRow = value.mX;    pRow++;
   *pRow = value.mY;    pRow++; 
   *pRow = value.mZ;
}

//------------------------------------------------------------------------------------------
