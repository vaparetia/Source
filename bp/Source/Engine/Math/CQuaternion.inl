//------------------------------------------------------------------------------------------
// CQuaternion.inl
// Bluepoint
//------------------------------------------------------------------------------------------

#include <math.h>
#include "Engine/Math/CAxisAngle.h"
#include "Engine/Math/CloseEnough.h"

//------------------------------------------------------------------------------------------

#if !defined(SPU)

CQuaternion::CQuaternion(CInputStream& stream)
:  mVector( stream )
{
}

//------------------------------------------------------------------------------------------

void CQuaternion::PutTo(COutputStream& stream) const
{
   mVector.PutTo(stream);
}

#endif

//------------------------------------------------------------------------------------------

CQuaternion::CQuaternion(const CVector3& vector, const real32 scalar )
:  mVector( vector, scalar )
{
}

//------------------------------------------------------------------------------------------

CQuaternion::CQuaternion(const CVector4& vector)
:  mVector( vector )
{
}

CQuaternion::CQuaternion(real32 const x, real32 const y, real32 const z, real32 const s)
:  mVector(x, y, z, s)
{
}

//------------------------------------------------------------------------------------------

const CQuaternion CQuaternion::Identity()
{
   return CQuaternion( CVector3::Zero(), 1.0f );
}

//------------------------------------------------------------------------------------------

const CVector3& CQuaternion::GetVector() const
{
   return reinterpret_cast<CVector3 const &>(mVector);
}

//------------------------------------------------------------------------------------------

const real32 CQuaternion::GetScalar() const
{
   return mVector.mW;
}

//------------------------------------------------------------------------------------------

const real32 CQuaternion::GetLengthSquared() const
{
   return mVector.GetLengthSquared();
}

//------------------------------------------------------------------------------------------

const real32 CQuaternion::GetLength() const
{
   return static_cast<real32>( sqrtf( static_cast<real32>( GetLengthSquared() ) ) );
}

//------------------------------------------------------------------------------------------

const CQuaternion CQuaternion::operator * (const CQuaternion& other) const
{
   /*
   const CVector3& vector = other.GetVector();
   const real32 scalar = other.GetScalar();

   return CQuaternion( CVector3(
      mScalar*vector.GetX() + scalar*mVector.GetX() + mVector.GetY()*vector.GetZ() - mVector.GetZ()*vector.GetY(),
      mScalar*vector.GetY() + scalar*mVector.GetY() + mVector.GetZ()*vector.GetX() - mVector.GetX()*vector.GetZ(),
      mScalar*vector.GetZ() + scalar*mVector.GetZ() + mVector.GetX()*vector.GetY() - mVector.GetY()*vector.GetX()
      ),
      mScalar*scalar - mVector.GetX()*vector.GetX() - mVector.GetY()*vector.GetY() - mVector.GetZ()*vector.GetZ() );
      */

   CVector3 const & Av = GetVector();
   CVector3 const & Bv = other.GetVector();
   
   real32 const As = GetScalar();
   real32 const Bs = other.GetScalar();

   real32 const scalar = ( As * Bs ) - CVector3::Dot( Av, Bv );
   CVector3 const vec( Bv * As + Av * Bs + CVector3::Cross( Av, Bv ) );

   return CQuaternion( vec, scalar );
}

CQuaternion& CQuaternion::operator *= (const CQuaternion& other)
{
   *this = *this * other;
   return *this;
}

//------------------------------------------------------------------------------------------

const CQuaternion CQuaternion::operator * (const real32 other) const
{
   return CQuaternion( mVector * other);
}

CQuaternion& CQuaternion::operator *= (const real32 other)
{
   mVector *= other;
   return *this;
}

//------------------------------------------------------------------------------------------

const CQuaternion CQuaternion::operator / (const real32 other) const
{
   return (*this) * ( 1.0f / other );
}

CQuaternion& CQuaternion::operator /= (const real32 other)
{
   return (*this) *= ( 1.0f / other );
}

//------------------------------------------------------------------------------------------

const CQuaternion CQuaternion::operator + (const CQuaternion& other) const
{
   return CQuaternion( mVector + other.mVector );
}

CQuaternion& CQuaternion::operator += (const CQuaternion& other)
{
   *this = *this + other;
   return *this;
}

//------------------------------------------------------------------------------------------

const CQuaternion CQuaternion::operator - (const CQuaternion& other) const
{
   return CQuaternion( mVector - other.mVector );
}

CQuaternion& CQuaternion::operator -= (const CQuaternion& other)
{
   *this = *this - other;
   return *this;
}

//------------------------------------------------------------------------------------------

const CQuaternion CQuaternion::RotateX(const CAngle& angle)
{
   return FromAxisAngle( CAxisAngle( CVector3::XAxis(), angle ) );
}

const CQuaternion CQuaternion::RotateY(const CAngle& angle)
{
   return FromAxisAngle( CAxisAngle( CVector3::YAxis(), angle ) );
}

const CQuaternion CQuaternion::RotateZ(const CAngle& angle)
{
   return FromAxisAngle( CAxisAngle( CVector3::ZAxis(), angle ) );
}

//------------------------------------------------------------------------------------------

const real32 CQuaternion::Dot(const CQuaternion& other) const
{
   return CVector4::Dot( mVector, other.mVector );
}

//------------------------------------------------------------------------------------------

void CQuaternion::Normalize()
{
   const real32 magnitude = sqrtf( Dot( *this ) );
   if( magnitude != 0.0f )
   {
      const real32 oneOverMagnitude = 1.0f / magnitude;

      mVector *= oneOverMagnitude;
   }
}

//------------------------------------------------------------------------------------------

const CQuaternion CQuaternion::AsNormalized() const
{
   CQuaternion temp( *this );
   temp.Normalize();
   return temp;
}

//------------------------------------------------------------------------------------------

const CVector3 CQuaternion::operator * (const CVector3& vector) const
{
   /*
   return CVector3(
      vector*GetScalar() +
      ( GetVector()*GetVector().Dot(vector) )*( real32(1) - GetScalar() ) +
      GetVector().Cross(vector)
      );
      */

   real32 const scalar = CVector3::Dot( -GetVector(), vector );
   CVector3 vec
      (
         mVector.mW * vector.GetX () + mVector.GetY () * vector.GetZ () - vector.GetY () * mVector.GetZ (),
         mVector.mW * vector.GetY () + mVector.GetZ () * vector.GetX () - vector.GetZ () * mVector.GetX (),
         mVector.mW * vector.GetZ () + mVector.GetX () * vector.GetY () - vector.GetX () * mVector.GetY ()
      );

   // Now lets do scalar,vector * mScalar,-mVector

   return CVector3
      (
         vec.GetX () * mVector.mW - scalar * mVector.GetX () - vec.GetY () * mVector.GetZ () + mVector.GetY () * vec.GetZ (),
         vec.GetY () * mVector.mW - scalar * mVector.GetY () - vec.GetZ () * mVector.GetX () + mVector.GetZ () * vec.GetX (),
         vec.GetZ () * mVector.mW - scalar * mVector.GetZ () - vec.GetX () * mVector.GetY () + mVector.GetX () * vec.GetY ()
      );

}

//------------------------------------------------------------------------------------------

CQuaternion const CQuaternion::Slerp( CQuaternion const & src, CQuaternion const & dstOrig, real32 const t )
{
   real32 dot = src.Dot( dstOrig );
   CQuaternion dst = dstOrig;

   // no need to check for (dot > 1) as it's better to lerp in this case, rather than just choosing src
   // (we get (dot > 1) due to numerical inaccuracy when normalizing or constructing quaternions)

   if( dot < 0.0f )
   {  
      dot = -dot;
      dst.mVector = -dst.mVector;
   }
   
   if( dot > 0.87f )
   {
      // Lerp
      CVector4 vector4(src.mVector + ( (dst.mVector - src.mVector) * t ) );
      vector4.Normalize();

      return CQuaternion( vector4 );
   }

   real32 const theta = acosf( dot );
   
   real32 const a = sinf( theta * ( 1 - t ) );
   real32 const b = sinf( theta * t ) ;
   
   
   CVector4 vector4 = src.mVector * a + dst.mVector * b;
   vector4.Normalize();

   return CQuaternion( vector4 );
}

//------------------------------------------------------------------------------------------

CQuaternion const CQuaternion::SlerpLocal( CQuaternion const & src, CQuaternion const & dst, real32 const t )
{
   return src.IsLocalTo( dst ) ? Slerp( src, dst, t ) : Slerp( src, BuildEquivalent( dst ), t );
}

//------------------------------------------------------------------------------------------

CQuaternion const CQuaternion::BuildEquivalent( CQuaternion const & src )
{
   return CQuaternion( -src.GetVector(), -src.GetScalar() );
}

//------------------------------------------------------------------------------------------
   
CQuaternion const CQuaternion::Inverse() const
{
   return CQuaternion( -GetVector(), GetScalar() );
}

//------------------------------------------------------------------------------------------

