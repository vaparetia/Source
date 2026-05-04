//------------------------------------------------------------------------------------------
// CAngle.inl
// Bluepoint
//------------------------------------------------------------------------------------------

#include <math.h>
#include "Engine/Streams/CInputStream.h"
#include "Engine/Streams/COutputStream.h"

//------------------------------------------------------------------------------------------

CAngle::CAngle( CInputStream& stream )
:  mRad(stream.ReadReal32())
{
}

//------------------------------------------------------------------------------------------

void CAngle::PutTo(COutputStream& stream) const
{
   stream.WriteReal32(mRad);
}

//------------------------------------------------------------------------------------------

CAngle::CAngle( real32 const& val, EAngle rep )
:  mRad(val)
{
   // convert to radians if necessary
   if(rep == kDegrees)
   {
      mRad *= gkDegrees2Radians32;
   }
}

//------------------------------------------------------------------------------------------

CAngle const CAngle::FromDegrees(real32 const deg) 
{ 
   return CAngle(deg, CAngle::kDegrees);
}

//------------------------------------------------------------------------------------------

CAngle const CAngle::FromRadians(real32 const rad)
{ 
   return CAngle(rad, CAngle::kRadians); 
}

//------------------------------------------------------------------------------------------

real32 const CAngle::AsRadians() const
{
   return mRad;
}

//------------------------------------------------------------------------------------------

real32 const CAngle::AsDegrees() const
{ 
   return mRad * gkRadians2Degrees32;
}

//------------------------------------------------------------------------------------------

CAngle const CAngle::Abs() const
{
   return CAngle::FromRadians( fabsf( mRad ) );
}

//------------------------------------------------------------------------------------------
// ModuloOneRevolution
// returns the angle in the range [0..2pi) radians or [0..360) degrees

CAngle const CAngle::ModuloOneRevolution() const
{
   if (mRad < 0)
   {
      real32 const rad = fmodf(-mRad, 2.0f*gkPi32);
      return CAngle::FromRadians(2.0f*gkPi32 - rad);
   }
   return CAngle::FromRadians( fmodf( mRad, 2.0f*gkPi32 ) );
}

//------------------------------------------------------------------------------------------
// ModuloOneRevolutionPlusOrMinus
// returns the angle in the range [-pi..pi) radians or [-180..180) degrees

CAngle const CAngle::ModuloOneRevolutionPlusOrMinus() const
{
   CAngle const moduloed = this->ModuloOneRevolution();
   real32 const rad = moduloed.AsRadians();
   if (rad >= gkPi32)
   {
      return CAngle::FromRadians(rad - 2.0f*gkPi32);
   }
   return moduloed;
}

//------------------------------------------------------------------------------------------

CAngle const CAngle::operator + ( CAngle const & other ) const
{
   CAngle temp( *this );
   temp += other;
   return temp;
}

CAngle& CAngle::operator += ( CAngle const & other )
{
   mRad += other.mRad;
   return *this;
}

//------------------------------------------------------------------------------------------

CAngle const CAngle::operator - ( CAngle const & other ) const
{
   CAngle temp( *this );
   temp -= other;
   return temp;
}

CAngle& CAngle::operator -= ( CAngle const & other )
{
   mRad -= other.mRad;
   return *this;
}

//------------------------------------------------------------------------------------------

CAngle const CAngle::operator * ( real32 const scalar ) const
{
   CAngle temp( *this );
   temp *= scalar;
   return temp;
}

CAngle & CAngle::operator *= ( real32 const scalar )
{
   mRad *= scalar;
   return *this;
}

//------------------------------------------------------------------------------------------

CAngle const operator * ( real32 const lhs, CAngle const & rhs )
{
   return CAngle::FromRadians( lhs * rhs.AsRadians() );
}

//------------------------------------------------------------------------------------------

CAngle const CAngle::operator / ( real32 const scalar ) const
{
   CAngle temp( *this );
   temp /= scalar;
   return temp;
}

CAngle & CAngle::operator /= ( real32 const scalar )
{
   mRad /= scalar;
   return *this;
}

//------------------------------------------------------------------------------------------

CAngle const CAngle::operator - () const
{
   return CAngle::FromRadians(-this->AsRadians());
}

//------------------------------------------------------------------------------------------

bool const CAngle::operator < ( CAngle const & other ) const
{
   return mRad < other.mRad;
}

bool const CAngle::operator <= ( CAngle const & other ) const
{
   return mRad <= other.mRad;
}

bool const CAngle::operator == ( CAngle const & other ) const
{
   return mRad == other.mRad;
}

bool const CAngle::operator >= ( CAngle const & other ) const
{
   return mRad >= other.mRad;
}

bool const CAngle::operator > ( CAngle const & other ) const
{
   return mRad > other.mRad;
}

