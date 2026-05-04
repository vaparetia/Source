//------------------------------------------------------------------------------------------
// CVector4.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "CVector3.h"
#include "CVector2.h"

//------------------------------------------------------------------------------------------

class CInputStream;
class COutputStream;

//------------------------------------------------------------------------------------------

class CVector4
{
public:
   enum EUninitialized
   {
      kConstructUninitialized
   };

public:
   explicit CVector4()
   {
   }

   explicit CVector4(EUninitialized)
   {
   }

   explicit CVector4( real32 const x, real32 const y, real32 const z, real32 const w )
   :  mX( x )
   ,  mY( y )
   ,  mZ( z )
   ,  mW( w )
   {
   }

   explicit CVector4( CVector3 const & vec, real32 const w )
   :  mX( vec.GetX() )
   ,  mY( vec.GetY() )
   ,  mZ( vec.GetZ() )
   ,  mW( w )
   {
   }

#if !defined(SPU)
   // Exclude unneeded/bulky code when compiling for SPU's
   explicit CVector4( CInputStream & stream );     // Stream constructor
   void PutTo( COutputStream & stream ) const;     // Put to stream
#endif

#if BP_VITA
   float16x4_t const AsF16() const { return vcvt_f16_f32( *reinterpret_cast<float32x4_t const *>(this) ); }
#endif

   // accessors
   real32 const GetX() const { return mX; }
   real32 const GetY() const { return mY; }
   real32 const GetZ() const { return mZ; }
   real32 const GetW() const { return mW; }

   void SetX( real32 const x ) { mX = x; }
   void SetY( real32 const y ) { mY = y; }
   void SetZ( real32 const z ) { mZ = z; }
   void SetW( real32 const w ) { mW = w; }

   inline const real32 GetLengthSquared() const { return ( mX * mX + mY * mY + mZ * mZ + mW * mW ); }
   inline const real32 GetLength() const        { return sqrtf( mX * mX + mY * mY + mZ * mZ + mW * mW ); }

   inline real32 Dot(CVector4 const & rhs) const                              { return (mX*rhs.mX + mY*rhs.mY + mZ*rhs.mZ + mW*rhs.mW); }
   static inline real32 Dot( CVector4 const & lhs, CVector4 const & rhs )     { return (lhs.mX*rhs.mX + lhs.mY*rhs.mY + lhs.mZ*rhs.mZ + lhs.mW*rhs.mW); }

   inline real32 const   Normalize()                              { real32 const prevLength = GetLength();  *this /= prevLength;   return prevLength; }
   inline CVector4 const Normalized() const                       { real32 const prevLength = GetLength(); return *this / prevLength; }
   inline CVector4 const Normalized(real32 &prevLength) const     { prevLength = GetLength(); return *this / prevLength; }
   CVector3 const DropW() const                                   { return CVector3( mX, mY, mZ ); }

   inline void           SetXY( CVector2 const &xy )              { mX = xy.GetX(); mY = xy.GetY(); }
   inline void           SetZW( CVector2 const &zw )              { mZ = zw.GetX(); mW = zw.GetY(); }
   inline CVector2 const GetXY() const                            { return CVector2( mX, mY ); }
   inline CVector2 const GetZW() const                            { return CVector2( mZ, mW ); }

   // element multiply
   static CVector4 const ElementMultiply( CVector4 const & lhs, CVector4 const & rhs ) { return CVector4( lhs.mX * rhs.mX, lhs.mY * rhs.mY, lhs.mZ * rhs.mZ, lhs.mW * rhs.mW ); }

   static CVector4 const Lerp( CVector4 const & src, CVector4 const & dst, real32 const t ) { return src * (1.0f - t) + dst * t; }

   // access operators
   const real32& operator [] ( const int index ) const { return (&mX)[index]; }
   real32& operator [] ( const int index ) { return (&mX)[index]; }

   // plus operator
   inline const CVector4 operator + (const CVector4& rhs) const { return CVector4( mX + rhs.mX, mY + rhs.mY, mZ + rhs.mZ, mW + rhs.mW ); }
   inline CVector4& operator += (const CVector4& rhs) { mX += rhs.mX; mY += rhs.mY; mZ += rhs.mZ; mW += rhs.mW; return *this; }

   // minus operator
   inline const CVector4 operator - (const CVector4& rhs) const { return CVector4( mX - rhs.mX, mY - rhs.mY, mZ - rhs.mZ, mW - rhs.mW ); }
   inline CVector4& operator -= (const CVector4& rhs) { mX -= rhs.mX; mY -= rhs.mY; mZ -= rhs.mZ; mW -= rhs.mW; return *this; }

   // element multiplication operator
   inline const CVector4 operator * (const real32 rhs) const { return CVector4( mX * rhs, mY * rhs, mZ * rhs, mW * rhs ); }
   inline CVector4& operator *= (const real32 rhs) { mX *= rhs; mY *= rhs; mZ *= rhs; mW *= rhs; return *this; }

   // element division operator
   inline const CVector4 operator / (const real32 rhs) const { return CVector4( mX / rhs, mY / rhs, mZ / rhs, mW / rhs ); }
   inline CVector4& operator /= (const real32 rhs) { mX /= rhs; mY /= rhs; mZ /= rhs; mW /= rhs; return *this; }

   // negate operator
   inline const CVector4 operator - () const { return CVector4( -mX, -mY, -mZ, -mW ); }

   // comparison operators
   bool operator == ( CVector4 const & rhs ) const { return GetX() == rhs.GetX() && GetY() == rhs.GetY() && GetZ() == rhs.GetZ(); }

   bool operator != ( CVector4 const & rhs ) const { return ( !( *this == rhs ) ); }

   uint32 GetHashValue() const
   {
      const uint32* h = (const uint32*)(this);
      uint32 f = (h[0]+h[1]*11-(h[2]*17)+(h[3]*20)) & 0x7fffffff;	// avoid problems with +-0
      return (f>>22)^(f>>12)^(f);
   }

   // vector factories
   static inline const CVector4 Zero() { return CVector4( 0, 0, 0, 0 ); }

public:
   real32  mX, mY, mZ, mW;
};

//------------------------------------------------------------------------------------------
