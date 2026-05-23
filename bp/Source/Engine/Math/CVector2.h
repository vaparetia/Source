//------------------------------------------------------------------------------------------
// CVector2.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "math.h"

//------------------------------------------------------------------------------------------

class CInputStream;
class COutputStream;

//------------------------------------------------------------------------------------------

class CVector2
{
public:
   enum EUninitialized
   {
      kConstructUninitialized
   };

public:
   // constructs uninitialized vector
   explicit CVector2() {}
   explicit inline CVector2( EUninitialized );

   inline CVector2( real32 x, real32 y );

#if !defined(SPU)
   // No stream support on SPU's
   explicit CVector2(CInputStream& stream);   
   void PutTo(COutputStream& stream) const;  // put vector to stream
#endif

#if defined(_XBOX_VER)
   //This structure is used in non cacheable memory structures (mostly vertex buffers), the default assignment operator
   //is doing copies as 1 doubles, this causes and alignment exception in non cacheable memory
   inline CVector2& operator=( const CVector2& InVector );
#endif

   // accessors
   inline real32 GetX() const;
   inline real32 GetY() const;

   inline void SetX( const real32 x );
   inline void SetY( const real32 y );

   // plus operator
   inline CVector2 const operator + (const CVector2& rhs) const;
   inline CVector2& operator += (const CVector2& rhs);

   // minus operator
   inline CVector2 const operator - (const CVector2& rhs) const;
   inline CVector2& operator -= (const CVector2& rhs);

   // element multiplication operator
   inline CVector2 const operator * (const real32 rhs) const;
   inline CVector2& operator *= (const real32 rhs);

   // element division operator
   inline CVector2 const operator / (const real32 rhs) const;
   inline CVector2& operator /= (const real32 rhs);

   // negate operator
   inline const CVector2 operator - () const;

   // dot product (non-static)
   inline real32 Dot(CVector2 const & rhs ) const;

   // get length of this vector
   inline real32 GetLength() const;
   inline real32 GetLengthSquared() const;

   // Normalization
   inline void Normalize();

   // Helper functions, returns previous length or 0 if cannot be normalized 
   inline real32 TryNormalize();

   // comparison operators
   inline bool const operator == ( const CVector2& rhs ) const;
   inline bool const operator != ( const CVector2& rhs ) const;

   // access operators
   inline real32 const & operator [] ( const int index ) const;
   inline real32 &       operator [] ( const int index );

   // element multiply
   static inline CVector2 const ElementMultiply( CVector2 const & lhs, CVector2 const & rhs );

   // dot product
   static inline real32 const Dot( CVector2 const & lhs, CVector2 const & rhs );

   // cross product
   static inline real32 const Cross( CVector2 const & lhs, CVector2 const & rhs );

   // distance between two vectors
   static inline real32 const Distance(CVector2 const & lhs, CVector2 const & rhs);
   static inline real32 const DistanceSquared(CVector2 const & lhs, CVector2 const & rhs);

   static inline CVector2 const Lerp( CVector2 const & src, CVector2 const & dst, real32 const t ) { return src * (1.0f - t) + dst * t; }

   static inline CVector2 const Zero();
   static inline CVector2 const One();
   static inline CVector2 const XAxis();
   static inline CVector2 const YAxis();
   static inline CVector2 const NegXAxis();
   static inline CVector2 const NegYAxis();

public:
   real32   mX;
   real32   mY;
};

//------------------------------------------------------------------------------------------

// Additional element operators
static inline CVector2 const operator * (real32 const lhs, CVector2 const &rhs);

//------------------------------------------------------------------------------------------

#include "Engine/Math/CVector2.inl"

//------------------------------------------------------------------------------------------
