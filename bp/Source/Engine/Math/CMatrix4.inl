//------------------------------------------------------------------------------------------
// CMatrix4.inl
// Bluepoint
//------------------------------------------------------------------------------------------

#include <math.h>
#include "Engine/Streams/CInputStream.h"
#include "Engine/Streams/COutputStream.h"

//------------------------------------------------------------------------------------------

inline CMatrix4::CMatrix4(CInputStream& stream)
:  m00(stream.ReadReal32())
,  m01(stream.ReadReal32())
,  m02(stream.ReadReal32())
,  m03(stream.ReadReal32())
,  m10(stream.ReadReal32())
,  m11(stream.ReadReal32())
,  m12(stream.ReadReal32())
,  m13(stream.ReadReal32())
,  m20(stream.ReadReal32())
,  m21(stream.ReadReal32())
,  m22(stream.ReadReal32())
,  m23(stream.ReadReal32())
,  m30(stream.ReadReal32())
,  m31(stream.ReadReal32())
,  m32(stream.ReadReal32())
,  m33(stream.ReadReal32())
{
}

//------------------------------------------------------------------------------------------

inline void CMatrix4::PutTo(COutputStream& stream) const
{
   stream.WriteReal32(m00);
   stream.WriteReal32(m01);
   stream.WriteReal32(m02);
   stream.WriteReal32(m03);
   stream.WriteReal32(m10);
   stream.WriteReal32(m11);
   stream.WriteReal32(m12);
   stream.WriteReal32(m13);
   stream.WriteReal32(m20);
   stream.WriteReal32(m21);
   stream.WriteReal32(m22);
   stream.WriteReal32(m23);
   stream.WriteReal32(m30);
   stream.WriteReal32(m31);
   stream.WriteReal32(m32);
   stream.WriteReal32(m33);
}

//------------------------------------------------------------------------------------------

inline CMatrix4::CMatrix4(real32 t00, real32 t01, real32 t02, real32 t03,
                          real32 t10, real32 t11, real32 t12, real32 t13,
                          real32 t20, real32 t21, real32 t22, real32 t23,
                          real32 t30, real32 t31, real32 t32, real32 t33 )
:  m00( t00 )
,  m01( t01 )
,  m02( t02 )
,  m03( t03 )
,  m10( t10 )
,  m11( t11 )
,  m12( t12 )
,  m13( t13 )
,  m20( t20 )
,  m21( t21 )
,  m22( t22 )
,  m23( t23 )
,  m30( t30 )
,  m31( t31 )
,  m32( t32 )
,  m33( t33 )
{
}

//------------------------------------------------------------------------------------------

inline CMatrix4::CMatrix4( CVector3 const & right, CVector3 const & up, CVector3 const & forward, CVector3 const & translation )
:  m00( right.GetX() )
,  m01( right.GetY() )
,  m02( right.GetZ() )
,  m03( 0.0f )
,  m10( up.GetX() )
,  m11( up.GetY() )
,  m12( up.GetZ() )
,  m13( 0.0f )
,  m20( forward.GetX() )
,  m21( forward.GetY() )
,  m22( forward.GetZ() )
,  m23( 0.0f )
,  m30( translation.GetX() )
,  m31( translation.GetY() )
,  m32( translation.GetZ() )
,  m33( 1.0f )
{
}

//------------------------------------------------------------------------------------------

inline CMatrix4::CMatrix4( CVector4 const &row0, CVector4 const &row1, CVector4 const &row2, CVector4 const &row3 )
:  m00( row0.mX ) // Right
,  m01( row0.mY )
,  m02( row0.mZ )
,  m03( row0.mW )
,  m10( row1.mX ) // Up
,  m11( row1.mY )
,  m12( row1.mZ )
,  m13( row1.mW )
,  m20( row2.mX ) // Forward
,  m21( row2.mY )
,  m22( row2.mZ )
,  m23( row2.mW )
,  m30( row3.mX ) // Translation
,  m31( row3.mY )
,  m32( row3.mZ )
,  m33( row3.mW )
{
}

//------------------------------------------------------------------------------------------

inline const CVector3 CMatrix4::operator * (CVector3 const & vector) const
{
   real32 inx = vector.GetX();
   real32 iny = vector.GetY();
   real32 inz = vector.GetZ();

   real32 x    =         inx * m00 + iny * m10 + inz * m20 + m30;
   real32 y    =         inx * m01 + iny * m11 + inz * m21 + m31;
   real32 z    =         inx * m02 + iny * m12 + inz * m22 + m32;
   real32 oow  = 1.0f / (inx * m03 + iny * m13 + inz * m23 + m33);

   x *= oow;
   y *= oow;
   z *= oow;

   return CVector3(x, y, z);
}

//------------------------------------------------------------------------------------------

inline const CVector4 CMatrix4::operator * (CVector4 const & vector) const
{
   real32 inx = vector.GetX();
   real32 iny = vector.GetY();
   real32 inz = vector.GetZ();
   real32 inw = vector.GetW();

   real32 x = inx * m00 + iny * m10 + inz * m20 + inw * m30;
   real32 y = inx * m01 + iny * m11 + inz * m21 + inw * m31;
   real32 z = inx * m02 + iny * m12 + inz * m22 + inw * m32;
   real32 w = inx * m03 + iny * m13 + inz * m23 + inw * m33;

   return CVector4(x, y, z, w);
}

//------------------------------------------------------------------------------------------

inline real32* const CMatrix4::GetData()
{
   return &m00;
}

//------------------------------------------------------------------------------------------

inline const real32* const CMatrix4::GetData() const
{
   return &m00;
}

//------------------------------------------------------------------------------------------

inline const CMatrix4 CMatrix4::operator * (const CMatrix4& matrix) const
{
   /*
   x   y   z   t
   --------------
   00, 01, 02, 03
   10, 11, 12, 13
   20, 21, 22, 23
   30, 31, 32, 33
   */

   return CMatrix4(
      m00*matrix.m00 + m10*matrix.m01 + m20*matrix.m02 + m30*matrix.m03,
      m01*matrix.m00 + m11*matrix.m01 + m21*matrix.m02 + m31*matrix.m03,
      m02*matrix.m00 + m12*matrix.m01 + m22*matrix.m02 + m32*matrix.m03,
      m03*matrix.m00 + m13*matrix.m01 + m23*matrix.m02 + m33*matrix.m03,
                                                                       
      m00*matrix.m10 + m10*matrix.m11 + m20*matrix.m12 + m30*matrix.m13,
      m01*matrix.m10 + m11*matrix.m11 + m21*matrix.m12 + m31*matrix.m13,
      m02*matrix.m10 + m12*matrix.m11 + m22*matrix.m12 + m32*matrix.m13,
      m03*matrix.m10 + m13*matrix.m11 + m23*matrix.m12 + m33*matrix.m13,
                                                                       
      m00*matrix.m20 + m10*matrix.m21 + m20*matrix.m22 + m30*matrix.m23,
      m01*matrix.m20 + m11*matrix.m21 + m21*matrix.m22 + m31*matrix.m23,
      m02*matrix.m20 + m12*matrix.m21 + m22*matrix.m22 + m32*matrix.m23,
      m03*matrix.m20 + m13*matrix.m21 + m23*matrix.m22 + m33*matrix.m23,
                                                                       
      m00*matrix.m30 + m10*matrix.m31 + m20*matrix.m32 + m30*matrix.m33,
      m01*matrix.m30 + m11*matrix.m31 + m21*matrix.m32 + m31*matrix.m33,
      m02*matrix.m30 + m12*matrix.m31 + m22*matrix.m32 + m32*matrix.m33,
      m03*matrix.m30 + m13*matrix.m31 + m23*matrix.m32 + m33*matrix.m33
      );
}

//------------------------------------------------------------------------------------------

inline const CMatrix4 CMatrix4::ScaleThenTranslate(CVector3 const &sc, CVector3 const &tr) const
{
   /*
      T * S =
      ( sx          )   (         )     ( sx  0  0  0 )
      (    sy       ) * (         ) =   (  0 sy  0  0 )
      (       sz    )   (         )     (  0  0 sz  0 )
      (           1 )   ( x y z 1 )     (  x  y  z  1 )
   */
   return CMatrix4(
      sc.mX*m00                                    + tr.mX*m03,
                     sc.mY*m01                     + tr.mY*m03,
                                    sc.mZ*m02      + tr.mZ*m03,
                                                           m03,
      sc.mX*m10                                    + tr.mX*m13,
                     sc.mY*m11                     + tr.mY*m13,
                                    sc.mZ*m12      + tr.mZ*m13,
                                                           m13,
      sc.mX*m20                                    + tr.mX*m23,
                     sc.mY*m21                     + tr.mY*m23,
                                    sc.mZ*m22      + tr.mZ*m23,
                                                           m23,
      sc.mX*m30                                    + tr.mX*m33,
                     sc.mY*m31                     + tr.mY*m33,
                                    sc.mZ*m32      + tr.mZ*m33,
                                                           m33
      );
}

//------------------------------------------------------------------------------------------

inline CMatrix4& CMatrix4::operator *= (const CMatrix4& matrix)
{
   *this = *this * matrix;
   return *this;
}

//------------------------------------------------------------------------------------------

inline const CMatrix4 CMatrix4::Transpose() const
{
   return CMatrix4( m00, m10, m20, m30,
                    m01, m11, m21, m31,
                    m02, m12, m22, m32,
                    m03, m13, m23, m33 );
}

//------------------------------------------------------------------------------------------

inline const real32 CMatrix4::Determinant() const
{
   return  (m00*m11 - m01*m10)*(m22*m33 - m23*m32)
          -(m00*m12 - m02*m10)*(m21*m33 - m23*m31)
          +(m00*m13 - m03*m10)*(m21*m32 - m22*m31)
          +(m01*m12 - m02*m11)*(m20*m33 - m23*m30)
          -(m01*m13 - m03*m11)*(m20*m32 - m22*m30)
          +(m02*m13 - m03*m12)*(m20*m31 - m21*m30);
}

//------------------------------------------------------------------------------------------
// refactored from:
// Streaming SIMD Extensions - Inverse of 4x4 Matrix
// ftp://download.intel.com/design/PentiumIII/sml/24504301.pdf
//------------------------------------------------------------------------------------------

inline const CMatrix4 CMatrix4::Inverse() const
{
   float s = Determinant();

   if( s == 0.0f )
      return *this;	

   s = 1/s;
   
   return CMatrix4(
      s*(m11*(m22*m33 - m23*m32) + m12*(m23*m31 - m21*m33) + m13*(m21*m32 - m22*m31)),
      s*(m21*(m02*m33 - m03*m32) + m22*(m03*m31 - m01*m33) + m23*(m01*m32 - m02*m31)),
      s*(m31*(m02*m13 - m03*m12) + m32*(m03*m11 - m01*m13) + m33*(m01*m12 - m02*m11)),
      s*(m01*(m13*m22 - m12*m23) + m02*(m11*m23 - m13*m21) + m03*(m12*m21 - m11*m22)),
      s*(m12*(m20*m33 - m23*m30) + m13*(m22*m30 - m20*m32) + m10*(m23*m32 - m22*m33)),
      s*(m22*(m00*m33 - m03*m30) + m23*(m02*m30 - m00*m32) + m20*(m03*m32 - m02*m33)),
      s*(m32*(m00*m13 - m03*m10) + m33*(m02*m10 - m00*m12) + m30*(m03*m12 - m02*m13)),
      s*(m02*(m13*m20 - m10*m23) + m03*(m10*m22 - m12*m20) + m00*(m12*m23 - m13*m22)),
      s*(m13*(m20*m31 - m21*m30) + m10*(m21*m33 - m23*m31) + m11*(m23*m30 - m20*m33)),
      s*(m23*(m00*m31 - m01*m30) + m20*(m01*m33 - m03*m31) + m21*(m03*m30 - m00*m33)),
      s*(m33*(m00*m11 - m01*m10) + m30*(m01*m13 - m03*m11) + m31*(m03*m10 - m00*m13)),
      s*(m03*(m11*m20 - m10*m21) + m00*(m13*m21 - m11*m23) + m01*(m10*m23 - m13*m20)),
      s*(m10*(m22*m31 - m21*m32) + m11*(m20*m32 - m22*m30) + m12*(m21*m30 - m20*m31)),
      s*(m20*(m02*m31 - m01*m32) + m21*(m00*m32 - m02*m30) + m22*(m01*m30 - m00*m31)),
      s*(m30*(m02*m11 - m01*m12) + m31*(m00*m12 - m02*m10) + m32*(m01*m10 - m00*m11)),
      s*(m00*(m11*m22 - m12*m21) + m01*(m12*m20 - m10*m22) + m02*(m10*m21 - m11*m20))
      );


   /*
   const CMatrix4 transpose( Transpose() );

   // array of transpose source matrix
   const real32* src = transpose.GetData();
   
   // calculate pairs for first 8 elements (cofactors)
   const real32 firstPairs[] = 
   {
      src[10] * src[15],
      src[11] * src[14],
      src[9] * src[15],
      src[11] * src[13],
      src[9] * src[14],
      src[10] * src[13],
      src[8] * src[15],
      src[11] * src[12],
      src[8] * src[14],
      src[10] * src[12],
      src[8] * src[13],
      src[9] * src[12]
   };

   // calculate first 8 elements (cofactors)
   const real32 dst00 = firstPairs[0]*src[5] + firstPairs[3]*src[6] + firstPairs[4]*src[7]
                  - (firstPairs[1]*src[5] + firstPairs[2]*src[6] + firstPairs[5]*src[7]);
   const real32 dst01 = firstPairs[1]*src[4] + firstPairs[6]*src[6] + firstPairs[9]*src[7]
                  - (firstPairs[0]*src[4] + firstPairs[7]*src[6] + firstPairs[8]*src[7]);
   const real32 dst02 = firstPairs[2]*src[4] + firstPairs[7]*src[5] + firstPairs[10]*src[7]
                  - (firstPairs[3]*src[4] + firstPairs[6]*src[5] + firstPairs[11]*src[7]);
   const real32 dst03 = firstPairs[5]*src[4] + firstPairs[8]*src[5] + firstPairs[11]*src[6]
                  - (firstPairs[4]*src[4] + firstPairs[9]*src[5] + firstPairs[10]*src[6]);
   const real32 dst04 = firstPairs[1]*src[1] + firstPairs[2]*src[2] + firstPairs[5]*src[3]
                  - (firstPairs[0]*src[1] + firstPairs[3]*src[2] + firstPairs[4]*src[3]);
   const real32 dst05 = firstPairs[0]*src[0] + firstPairs[7]*src[2] + firstPairs[8]*src[3]
                  - (firstPairs[1]*src[0] + firstPairs[6]*src[2] + firstPairs[9]*src[3]);
   const real32 dst06 = firstPairs[3]*src[0] + firstPairs[6]*src[1] + firstPairs[11]*src[3]
                  - (firstPairs[2]*src[0] + firstPairs[7]*src[1] + firstPairs[10]*src[3]);
   const real32 dst07 = firstPairs[4]*src[0] + firstPairs[9]*src[1] + firstPairs[10]*src[2]
                  - (firstPairs[5]*src[0] + firstPairs[8]*src[1] + firstPairs[11]*src[2]);

   // calculate pairs for second 8 elements (cofactors)
   const real32 secondPairs[] = 
   {
      src[2]*src[7],
      src[3]*src[6],
      src[1]*src[7],
      src[3]*src[5],
      src[1]*src[6],
      src[2]*src[5],
      src[0]*src[7],
      src[3]*src[4],
      src[0]*src[6],
      src[2]*src[4],
      src[0]*src[5],
      src[1]*src[4]
   };

   // calculate second 8 elements (cofactors)
   const real32 dst08 = secondPairs[0]*src[13] + secondPairs[3]*src[14] + secondPairs[4]*src[15]
            - (secondPairs[1]*src[13] + secondPairs[2]*src[14] + secondPairs[5]*src[15]);
   const real32 dst09 = secondPairs[1]*src[12] + secondPairs[6]*src[14] + secondPairs[9]*src[15]
            - (secondPairs[0]*src[12] + secondPairs[7]*src[14] + secondPairs[8]*src[15]);
   const real32 dst10 = secondPairs[2]*src[12] + secondPairs[7]*src[13] + secondPairs[10]*src[15]
            - (secondPairs[3]*src[12] + secondPairs[6]*src[13] + secondPairs[11]*src[15]);
   const real32 dst11 = secondPairs[5]*src[12] + secondPairs[8]*src[13] + secondPairs[11]*src[14]
            - (secondPairs[4]*src[12] + secondPairs[9]*src[13] + secondPairs[10]*src[14]);
   const real32 dst12 = secondPairs[2]*src[10] + secondPairs[5]*src[11] + secondPairs[1]*src[9]
            - (secondPairs[4]*src[11] + secondPairs[0]*src[9] + secondPairs[3]*src[10]);
   const real32 dst13 = secondPairs[8]*src[11] + secondPairs[0]*src[8] + secondPairs[7]*src[10]
            - (secondPairs[6]*src[10] + secondPairs[9]*src[11] + secondPairs[1]*src[8]);
   const real32 dst14 = secondPairs[6]*src[9] + secondPairs[11]*src[11] + secondPairs[3]*src[8]
            - (secondPairs[10]*src[11] + secondPairs[2]*src[8] + secondPairs[7]*src[9]);
   const real32 dst15 = secondPairs[10]*src[10] + secondPairs[4]*src[8] + secondPairs[9]*src[9]
            - (secondPairs[8]*src[9] + secondPairs[11]*src[10] + secondPairs[5]*src[8]);

   // calculate determinant
   const real32 det = real32(1.0) / (src[0]*dst00 + src[1]*dst01 + src[2]*dst02 + src[3]*dst03);
   
   BPE_ASSERT( (real32(1.0) - bpe::abs(det)) < gkEpsilon32, "CMatrix4::Inverse: determinant to small, can`t get inverse" );

   // calculate matrix inverse
   return CMatrix4( dst00*det, dst01*det, dst02*det, dst03*det,
                    dst04*det, dst05*det, dst06*det, dst07*det,
                    dst08*det, dst09*det, dst10*det, dst11*det,
                    dst12*det, dst13*det, dst14*det, dst15*det );
   */

}

//------------------------------------------------------------------------------------------

inline const CMatrix4 CMatrix4::Identity()
{
   return CMatrix4( real32(1.0), real32(0.0), real32(0.0), real32(0.0),
                    real32(0.0), real32(1.0), real32(0.0), real32(0.0), 
                    real32(0.0), real32(0.0), real32(1.0), real32(0.0), 
                    real32(0.0), real32(0.0), real32(0.0), real32(1.0) );
}

//------------------------------------------------------------------------------------------

inline const CMatrix4 CMatrix4::Translation(const CVector3& translation)
{
   return CMatrix4(
      real32(1.0), real32(0.0), real32(0.0), real32(0.0), 
      real32(0.0), real32(1.0), real32(0.0), real32(0.0), 
      real32(0.0), real32(0.0), real32(1.0), real32(0.0), 
      translation.GetX(), translation.GetY(), translation.GetZ(), real32(1.0) );
}

//------------------------------------------------------------------------------------------

inline const CMatrix4 CMatrix4::Scale(const CVector3& scale)
{
   return CMatrix4(
      scale.GetX(), real32(0.0), real32(0.0), real32(0.0), 
      real32(0.0), scale.GetY(), real32(0.0), real32(0.0), 
      real32(0.0), real32(0.0), scale.GetZ(), real32(0.0), 
      real32(0.0), real32(0.0), real32(0.0), real32(1.0) );
}

//------------------------------------------------------------------------------------------

inline const CMatrix4 CMatrix4::RotateX(const CAngle& angle)
{
   const real32 sine = static_cast<real32>( sinf( static_cast<real32>( angle.AsRadians() ) ) );
   const real32 cos = static_cast<real32>( cosf( static_cast<real32>( angle.AsRadians() ) ) );

   return CMatrix4( real32(1.0), real32(0.0), real32(0.0), real32(0.0),
                    real32(0.0), cos, sine, real32(0.0), 
                    real32(0.0), -sine, cos, real32(0.0), 
                    real32(0.0), real32(0.0), real32(0.0), real32(1.0) );
}

//------------------------------------------------------------------------------------------

inline const CMatrix4 CMatrix4::RotateY(const CAngle& angle)
{
   const real32 sine = static_cast<real32>( sinf( static_cast<real32>( angle.AsRadians() ) ) );
   const real32 cos = static_cast<real32>( cosf( static_cast<real32>( angle.AsRadians() ) ) );

   return CMatrix4( cos, real32(0.0), -sine, real32(0.0),
                    real32(0.0), real32(1.0), real32(0.0), real32(0.0), 
                    sine, real32(0.0), cos, real32(0.0), 
                    real32(0.0), real32(0.0), real32(0.0), real32(1.0) );
}

//------------------------------------------------------------------------------------------

inline const CMatrix4 CMatrix4::RotateZ(const CAngle& angle)
{
   const real32 sine = static_cast<real32>( sinf( static_cast<real32>( angle.AsRadians() ) ) );
   const real32 cos = static_cast<real32>( cosf( static_cast<real32>( angle.AsRadians() ) ) );

   return CMatrix4( cos, sine, real32(0.0), real32(0.0),
                    -sine, cos, real32(0.0), real32(0.0), 
                    real32(0.0), real32(0.0), real32(1.0), real32(0.0), 
                    real32(0.0), real32(0.0), real32(0.0), real32(1.0) );
}

//------------------------------------------------------------------------------------------

void CMatrix4::SetRow( uint32 const row, CVector4 const &value)
{
   BPE_ASSERT(row < 4, "Row out of range."); 
   real32 *pRow = ((real32 *) &m00) + (row * 4);
   *pRow = value.mX;    pRow++;
   *pRow = value.mY;    pRow++; 
   *pRow = value.mZ;    pRow++; 
   *pRow = value.mW;
}

//------------------------------------------------------------------------------------------

const CVector4 CMatrix4::GetRow(uint32 const row) const
{ 
   BPE_ASSERT(row < 4, "Row out of range.");
   real32 const *pRow = ((real32 *) &m00) + (row * 4);
   real32 x = *pRow;    pRow++;
   real32 y = *pRow;    pRow++;
   real32 z = *pRow;    pRow++;
   real32 w = *pRow;
   return CVector4(x, y, z, w);
}

//------------------------------------------------------------------------------------------

