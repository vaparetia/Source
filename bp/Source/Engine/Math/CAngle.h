//------------------------------------------------------------------------------------------
// CAngle.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/Basics/BPEConstants.h"

//------------------------------------------------------------------------------------------

class CInputStream;
class COutputStream;

//------------------------------------------------------------------------------------------

class CAngle
{
public:
   enum  EAngle
   {
      kDegrees,
      kRadians
   };

public:
   inline explicit CAngle( CInputStream& stream);
   inline explicit CAngle( real32 const & val, EAngle rep = kRadians );

   inline void PutTo(COutputStream& stream) const;
   
   inline static CAngle const FromDegrees(real32 const deg);
   inline static CAngle const FromRadians(real32 const rad);

   inline real32 const AsRadians() const;
   inline real32 const AsDegrees() const;

   inline CAngle const Abs() const;

   // returns the angle in the range [0..2pi) radians or [0..360) degrees
   inline CAngle const ModuloOneRevolution() const;
   // returns the angle in the range [-pi..pi) radians or [-180..180) degrees
   inline CAngle const ModuloOneRevolutionPlusOrMinus() const;

   inline CAngle const operator + ( CAngle const & other ) const;
   inline CAngle& operator += ( CAngle const & other );

   inline CAngle const operator - ( CAngle const & other ) const;
   inline CAngle& operator -= ( CAngle const & other );

   inline CAngle const operator * ( real32 const scalar ) const;
   inline CAngle const operator / ( real32 const scalar ) const;

   inline CAngle & operator *= ( real32 const scalar );
   inline CAngle & operator /= ( real32 const scalar );

   inline CAngle const operator - () const;

   inline bool const operator < ( CAngle const & other ) const;
   inline bool const operator <= ( CAngle const & other ) const;
   inline bool const operator == ( CAngle const & other ) const;
   inline bool const operator >= ( CAngle const & other ) const;
   inline bool const operator > ( CAngle const & other ) const;

private:
   real32   mRad;
};

//------------------------------------------------------------------------------------------

// Additional element operators
static inline CAngle const operator * ( real32 const lhs, CAngle const & rhs );

//------------------------------------------------------------------------------------------

#include "Engine/Math/CAngle.inl"

