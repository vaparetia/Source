//------------------------------------------------------------------------------------------
// CVector2.inl
// Bluepoint
//------------------------------------------------------------------------------------------

#include <math.h>

//------------------------------------------------------------------------------------------

CVector2::CVector2( EUninitialized )
{
}

//------------------------------------------------------------------------------------------

CVector2::CVector2( real32 x, real32 y )
:  mX(x)
,  mY(y)
{
}

//------------------------------------------------------------------------------------------
#if defined(_XBOX_VER)
CVector2& CVector2::operator=( const CVector2& InVector )
{
   mX = InVector.mX;
   mY = InVector.mY;
   return *this;
}
#endif
//------------------------------------------------------------------------------------------

void CVector2::SetX(real32 x)
{
   mX = x;
}

//------------------------------------------------------------------------------------------

void CVector2::SetY(real32 y)
{
   mY = y;
}

//------------------------------------------------------------------------------------------

real32 CVector2::GetX() const
{
   return mX;
}

//------------------------------------------------------------------------------------------

real32 CVector2::GetY() const
{
   return mY;
}

//------------------------------------------------------------------------------------------

const CVector2 CVector2::operator + (const CVector2& rhs) const
{
   return CVector2(*this) += rhs;
}

CVector2& CVector2::operator += (const CVector2& rhs)
{
   mX += rhs.mX;
   mY += rhs.mY;
   return *this;
}

//------------------------------------------------------------------------------------------

const CVector2 CVector2::operator - (const CVector2& rhs) const
{
   return CVector2(*this) -= rhs;
}

//------------------------------------------------------------------------------------------

CVector2& CVector2::operator -= (const CVector2& rhs)
{
   mX -= rhs.mX;
   mY -= rhs.mY;
   return *this;
}

//------------------------------------------------------------------------------------------

CVector2 const CVector2::operator * (real32 const rhs) const
{
   return CVector2(rhs * mX, rhs * mY);
}

//------------------------------------------------------------------------------------------

CVector2 const operator * (real32 const lhs, CVector2 const &rhs)
{
   return CVector2(lhs * rhs.mX, lhs * rhs.mY);
}

//------------------------------------------------------------------------------------------

CVector2& CVector2::operator *= (real32 const rhs)
{
   mX *= rhs;
   mY *= rhs;
   return *this;
}

//------------------------------------------------------------------------------------------

CVector2 const CVector2::operator / (real32 const rhs) const
{
   real32 const oneOverRhs = 1.0f / rhs;
   return CVector2(mX * oneOverRhs, mY * oneOverRhs);
}

//------------------------------------------------------------------------------------------

CVector2& CVector2::operator /= (real32 const rhs)
{
   // AndyO: Removed DBZ assert as we can enable FP exception handling.
   real32 const oneOverRhs = 1.0f / rhs;
   mX *= oneOverRhs;
   mY *= oneOverRhs;
   return *this;
}

//------------------------------------------------------------------------------------------

const CVector2 CVector2::operator - () const
{
   return CVector2( -mX, -mY );
}

//------------------------------------------------------------------------------------------

const bool CVector2::operator == ( const CVector2& rhs ) const
{
   return GetX() == rhs.GetX() && GetY() == rhs.GetY();
}

//------------------------------------------------------------------------------------------

const bool CVector2::operator != ( const CVector2& rhs ) const
{
   return ( !( *this == rhs ) );
}
   
//------------------------------------------------------------------------------------------

void CVector2::Normalize()
{
   real32 const oovLength = 1.0f / sqrtf( mX * mX + mY * mY );
   mX *= oovLength;
   mY *= oovLength;
}

//------------------------------------------------------------------------------------------

real32 CVector2::TryNormalize()
{
   real32 const prevLength = GetLength();
   // Arbitrary epsilon
   real32 const kEpsilon_Normalize = 1.192092896e-07f;
   if (prevLength < kEpsilon_Normalize)
   {
      return 0;
   }

   *this /= prevLength;

   return prevLength;
}

//------------------------------------------------------------------------------------------

real32 const & CVector2::operator [] ( const int index ) const
{
   return (&mX)[index];
}

//------------------------------------------------------------------------------------------

real32 & CVector2::operator [] ( const int index ) 
{ 
   return (&mX)[index];
}

//------------------------------------------------------------------------------------------

CVector2 const CVector2::ElementMultiply( CVector2 const & lhs, CVector2 const & rhs )
{ 
   return CVector2( lhs.GetX() * rhs.GetX(), lhs.GetY() * rhs.GetY() );
}

//------------------------------------------------------------------------------------------

real32 const CVector2::Dot( CVector2 const & lhs, CVector2 const & rhs ) 
{ 
   return (lhs.mX * rhs.mX) + (lhs.mY * rhs.mY); 
}

//------------------------------------------------------------------------------------------

real32 const CVector2::Cross( CVector2 const & lhs, CVector2 const & rhs ) 
{ 
   return lhs[0] * rhs[1] - lhs[1] * rhs[0]; 
}

//------------------------------------------------------------------------------------------

real32 const CVector2::Distance(CVector2 const & lhs, CVector2 const & rhs)        
{ 
   CVector2 const delta(rhs - lhs); return sqrtf((delta.mX * delta.mX) + (delta.mY * delta.mY)); 
}

//------------------------------------------------------------------------------------------

real32 const CVector2::DistanceSquared(CVector2 const & lhs, CVector2 const & rhs) 
{ 
   CVector2 const delta(rhs - lhs); return ((delta.mX * delta.mX) + (delta.mY * delta.mY)); 
}

//------------------------------------------------------------------------------------------

real32 CVector2::GetLength() const           
{ 
   return sqrtf((mX * mX )+ (mY * mY)); 
}

//------------------------------------------------------------------------------------------

real32 CVector2::GetLengthSquared() const    
{ 
   return ((mX * mX )+ (mY * mY)); 
}

//------------------------------------------------------------------------------------------

real32 CVector2::Dot(CVector2 const & rhs ) const
{ 
   return (mX * rhs.mX) + (mY * rhs.mY); 
}

//------------------------------------------------------------------------------------------

CVector2 const CVector2::Zero()
{
   return CVector2( 0.0f, 0.0f );
}

//------------------------------------------------------------------------------------------

CVector2 const CVector2::One()
{
   return CVector2( 1.0f, 1.0f );
}

//------------------------------------------------------------------------------------------

CVector2 const CVector2::XAxis()
{
   return CVector2( 1.0f, 0.0f );
}

//------------------------------------------------------------------------------------------

CVector2 const CVector2::YAxis()
{
   return CVector2( 0.0f, 1.0f );
}

//------------------------------------------------------------------------------------------

CVector2 const CVector2::NegXAxis()
{
   return CVector2( -1.0f, 0.0f );
}

//------------------------------------------------------------------------------------------

CVector2 const CVector2::NegYAxis()
{
   return CVector2( 0.0f, -1.0f );
}

//------------------------------------------------------------------------------------------

