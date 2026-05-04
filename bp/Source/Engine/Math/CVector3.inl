//------------------------------------------------------------------------------------------
// CVector3.inl
// Bluepoint
//------------------------------------------------------------------------------------------

#include <math.h>
#include "Engine/Basics/CStringExtras.h"
#include "Engine/Streams/CInputStream.h"
#include "Engine/Streams/COutputStream.h"

//------------------------------------------------------------------------------------------

CVector3::CVector3( EUninitialized )
{
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE CVector3::CVector3(real32 x, real32 y, real32 z)
:  mX(x)
,  mY(y)
,  mZ(z)
{
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE void CVector3::SetX(real32 x)
{
   mX = x;
}

BPE_FORCEINLINE void CVector3::SetY(real32 y)
{
   mY = y;
}

BPE_FORCEINLINE void CVector3::SetZ(real32 z)
{
   mZ = z;
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE real32 CVector3::GetX() const
{
   return mX;
}

BPE_FORCEINLINE real32 CVector3::GetY() const
{
   return mY;
}

BPE_FORCEINLINE real32 CVector3::GetZ() const
{
   return mZ;
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE const CVector3 CVector3::operator + (const CVector3& rhs) const
{
   return CVector3(*this) += rhs;
}

BPE_FORCEINLINE CVector3& CVector3::operator += (const CVector3& rhs)
{
   mX += rhs.mX;
   mY += rhs.mY;
   mZ += rhs.mZ;
   return *this;
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE const CVector3 CVector3::operator - (const CVector3& rhs) const
{
   return CVector3(*this) -= rhs;
}

BPE_FORCEINLINE CVector3& CVector3::operator -= (const CVector3& rhs)
{
   mX -= rhs.mX;
   mY -= rhs.mY;
   mZ -= rhs.mZ;
   return *this;
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE const CVector3 CVector3::operator * (const real32 rhs) const
{
   return CVector3(mX * rhs, mY * rhs, mZ * rhs);
}

BPE_FORCEINLINE const CVector3 operator * (const real32 lhs, CVector3 const &rhs)
{
   return CVector3(rhs.mX * lhs, rhs.mY * lhs, rhs.mZ * lhs);
}

BPE_FORCEINLINE CVector3& CVector3::operator *= (const real32 rhs)
{
   mX *= rhs;
   mY *= rhs;
   mZ *= rhs;
   return *this;
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE const CVector3 CVector3::operator / (const real32 rhs) const
{
   BPE_ASSERT( (rhs != 0) && (rhs != -0), "division by zero" );
   const real32 oneOverRhs = real32(1.0) / rhs;
   return CVector3(mX * oneOverRhs, mY * oneOverRhs, mZ * oneOverRhs);
}

BPE_FORCEINLINE CVector3& CVector3::operator /= (const real32 rhs)
{
   BPE_ASSERT( (rhs != 0) && (rhs != -0), "division by zero" );
   const real32 oneOverRhs = real32(1.0) / rhs;
   mX *= oneOverRhs;
   mY *= oneOverRhs;
   mZ *= oneOverRhs;
   return *this;
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE const CVector3 CVector3::operator - () const
{
   return CVector3( -mX, -mY, -mZ );
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE real32 CVector3::Dot(CVector3 const & rhs) const
{
   return (mX*rhs.mX + mY*rhs.mY + mZ*rhs.mZ);
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE real32 CVector3::Dot( CVector3 const & lhs, CVector3 const & rhs )
{
   return (lhs.mX*rhs.mX + lhs.mY*rhs.mY + lhs.mZ*rhs.mZ);
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE real32 CVector3::Distance(CVector3 const & lhs, CVector3 const & rhs)
{
   return sqrtf( DistanceSquared(lhs, rhs) );
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE real32 CVector3::DistanceSquared(CVector3 const & lhs, CVector3 const & rhs)
{
   CVector3 const dirVector( rhs - lhs );
   
   real32 const dirX = dirVector.GetX();
   real32 const dirY = dirVector.GetY();
   real32 const dirZ = dirVector.GetZ();

   return dirX*dirX + dirY*dirY + dirZ*dirZ;
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE real32 CVector3::GetLength() const
{
   return sqrtf(mX * mX + mY * mY + mZ * mZ);
}

BPE_FORCEINLINE real32 CVector3::GetLengthSquared() const
{
   return (mX * mX + mY * mY + mZ * mZ);
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE real32 CVector3::Normalize()
{
   real32 const prevLength = GetLength();
   *this /= prevLength;

   return prevLength;
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE CVector3 CVector3::Normalized() const
{
   real32 const prevLength = GetLength();
   return *this / prevLength;
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE CVector3 CVector3::Normalized(real32 &prevLength) const
{
   prevLength = GetLength();
   return *this / prevLength;
}

//------------------------------------------------------------------------------------------

CVector3 const CVector3::Cross( CVector3 const & lhs, CVector3 const & rhs )
{
   return CVector3( lhs.GetY()*rhs.GetZ() - lhs.GetZ()*rhs.GetY(),
                    lhs.GetZ()*rhs.GetX() - lhs.GetX()*rhs.GetZ(),
                    lhs.GetX()*rhs.GetY() - lhs.GetY()*rhs.GetX() );
}

//------------------------------------------------------------------------------------------
// Hash function from Opcode/Ville Miettinen
uint32 CVector3::GetHashValue() const
{
	const uint32* h = (const uint32*)(this);
	uint32 f = (h[0]+h[1]*11-(h[2]*17)) & 0x7fffffff;	// avoid problems with +-0
	return (f>>22)^(f>>12)^(f);
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE const CVector3 CVector3::Abs() const
{
   return CVector3(*this).AbsInplace();
}

BPE_FORCEINLINE CVector3& CVector3::AbsInplace()
{
   mX = static_cast<real32>( fabsf(static_cast<float>(mX)) );
   mY = static_cast<real32>( fabsf(static_cast<float>(mY)) );
   mZ = static_cast<real32>( fabsf(static_cast<float>(mZ)) );
   return *this;
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE void CVector3::ClampMin( const CVector3& min )
{
   mX = bpe::max_val( mX, min.mX );
   mY = bpe::max_val( mY, min.mY );
   mZ = bpe::max_val( mZ, min.mZ );
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE void CVector3::ClampMax( const CVector3& max )
{
   mX = bpe::min_val( mX, max.mX );
   mY = bpe::min_val( mY, max.mY );
   mZ = bpe::min_val( mZ, max.mZ );
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE const bool CVector3::operator == ( const CVector3& rhs ) const
{
   return this->GetX() == rhs.GetX() &&
          this->GetY() == rhs.GetY() &&
          this->GetZ() == rhs.GetZ();
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE const bool CVector3::operator != ( const CVector3& rhs ) const
{
   return ( !( *this == rhs ) );
}

//------------------------------------------------------------------------------------------

BPE_FORCEINLINE const CVector3 CVector3::Zero()
{
   return CVector3( real32(0), real32(0), real32(0) );
}

BPE_FORCEINLINE const CVector3 CVector3::One()
{
   return CVector3( real32(1), real32(1), real32(1) );
}


BPE_FORCEINLINE const CVector3 CVector3::XAxis()
{
   return CVector3( real32(1), real32(0), real32(0) );
}


BPE_FORCEINLINE const CVector3 CVector3::YAxis()
{
   return CVector3( real32(0), real32(1), real32(0) );
}


BPE_FORCEINLINE const CVector3 CVector3::ZAxis()
{
   return CVector3( real32(0), real32(0), real32(1) );
}


BPE_FORCEINLINE const CVector3 CVector3::NegXAxis()
{
   return CVector3( real32(-1), real32(0), real32(0) );
}


BPE_FORCEINLINE const CVector3 CVector3::NegYAxis()
{
   return CVector3( real32(0), real32(-1), real32(0) );
}


BPE_FORCEINLINE const CVector3 CVector3::NegZAxis()
{
   return CVector3( real32(0), real32(0), real32(-1) );
}

//------------------------------------------------------------------------------------------

