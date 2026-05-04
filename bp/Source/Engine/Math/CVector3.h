//------------------------------------------------------------------------------------------
// CVector3.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

// TODO: Remove dependency on <string>
#include <string>
#include "CVector2.h"

//------------------------------------------------------------------------------------------

class CInputStream;
class COutputStream;

//------------------------------------------------------------------------------------------

MANAGED_PUBLIC class CVector3
{
public:
   enum EAxis
   {
      kAxis_X,
      kAxis_Y,
      kAxis_Z,

      kAxis_Count
   };

   enum EUninitialized
   {
      kConstructUninitialized
   };

public:
   explicit CVector3()
   {
   }

   // constructs uninitialized vector
   explicit inline CVector3( EUninitialized );

   inline CVector3(real32 x, real32 y, real32 z);

   ENGINE_API explicit CVector3( std::string const & textValue );
   ENGINE_API std::string ToString() const;

   ENGINE_API explicit CVector3(CInputStream& stream);

   // put vector to stream
   ENGINE_API  void PutTo(COutputStream& stream) const;

   // accessors
   inline real32 GetX() const;
   inline real32 GetY() const;
   inline real32 GetZ() const;

   inline void SetX(const real32 x);
   inline void SetY(const real32 y);
   inline void SetZ(const real32 z);

   // plus operator
   inline CVector3 const operator + (const CVector3& rhs) const;
   inline CVector3& operator += (const CVector3& rhs);

   // minus operator
   inline CVector3 const operator - (const CVector3& rhs) const;
   inline CVector3& operator -= (const CVector3& rhs);

   // element multiplication operator
   inline const CVector3 operator * (const real32 rhs) const;
   inline CVector3& operator *= (const real32 rhs);

   // element division operator
   inline const CVector3 operator / (const real32 rhs) const;
   inline CVector3& operator /= (const real32 rhs);

   // negate operator
   inline const CVector3 operator - () const;

   // dot product
   inline real32 Dot(CVector3 const & rhs) const;

   // dot product (static)
   static inline real32 Dot( CVector3 const & lhs, CVector3 const & rhs );

   // cross product and cross product operator
   static inline CVector3 const Cross( CVector3 const & lhs, CVector3 const & rhs );

   // distance between two vectors
   static inline real32 Distance(CVector3 const & lhs, CVector3 const & rhs);
   static inline real32 DistanceSquared(CVector3 const & lhs, CVector3 const & rhs);

   // get length of this vector
   inline real32 GetLength() const;
   inline real32 GetLengthSquared() const;

   // normalize vector to be unit length, returns prev length
   inline real32 Normalize();

   inline CVector3 Normalized() const;
   inline CVector3 Normalized(real32 &prevLength) const;

   // Helper functions, returns previous length or 0 if cannot be normalized 
   ENGINE_API real32 TryNormalize();

   // This version assumes the vector is a delta in game world space, so the magnitude of the vector must be 
   // much larger to be normalized and retain accuracy.
   ENGINE_API real32 TryNormalize_WSDelta();

   // element multiply
   inline static CVector3 const ElementMultiply( CVector3 const & lhs, CVector3 const & rhs ) { return CVector3( lhs.GetX() * rhs.GetX(), lhs.GetY() * rhs.GetY(), lhs.GetZ() * rhs.GetZ() ); }
   
   // Get the axis with the largest magnitude
   ENGINE_API int GetDominantAxis() const;
   // Get the axis that's neither the minor axis or the dominant axis
   ENGINE_API int GetMedianAxis() const;
   // Get the axis with the smallest magnitude
   ENGINE_API int GetMinorAxis() const;

   // Hash function from Opcode/Ville Miettinen
   inline uint32	GetHashValue()	const;

   // make vector absolute
   inline const CVector3 Abs() const;
   inline CVector3& AbsInplace();

   inline void ClampMin( const real32 min ) { ClampMin( CVector3( min, min, min ) ); }
   inline void ClampMin( const CVector3& min );

   inline void ClampMax( const real32 max ) { ClampMax( CVector3( max, max, max ) ); }
   inline void ClampMax( const CVector3& max );

   inline void ClampMinMax( const real32 min, const real32 max ) { ClampMinMax( CVector3( min, min, min ), CVector3( max, max, max ) ); }
   inline void ClampMinMax( const CVector3& min, const CVector3& max ) { ClampMin( min ); ClampMax( max ); }

   inline static CVector3 const Lerp( CVector3 const & src, CVector3 const & dst, real32 const t ) { return src * (1.0f - t) + dst * t; }
   
   // access operators
   const real32& operator [] ( const int index ) const { return (&mX)[index]; }
   real32& operator [] ( const int index ) { return (&mX)[index]; }

   CVector2 const DropZ() const { return CVector2( GetX(), GetY() ); }
   CVector2 const DropY() const { return CVector2( GetX(), GetZ() ); }
   CVector2 const DropX() const { return CVector2( GetY(), GetZ() ); }

   // comparison operators
   inline const bool operator == ( const CVector3& rhs ) const;
   inline const bool operator != ( const CVector3& rhs ) const;

   // vector factories
   static inline const CVector3 Zero();
   static inline const CVector3 One();
   static inline const CVector3 XAxis();
   static inline const CVector3 YAxis();
   static inline const CVector3 ZAxis();
   static inline const CVector3 NegXAxis();
   static inline const CVector3 NegYAxis();
   static inline const CVector3 NegZAxis();

public:
   real32  mX, mY, mZ;
};

//------------------------------------------------------------------------------------------

// Additional element operators
static inline const CVector3 operator * (const real32 lhs, CVector3 const &rhs);

//------------------------------------------------------------------------------------------

#include "Engine/Math/CVector3.inl"

//------------------------------------------------------------------------------------------
