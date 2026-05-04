//------------------------------------------------------------------------------------------
// CAxisAngle.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

class CInputStream;
class COutputStream;

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

#include "Engine/Math/CVector3.h"
#include "Engine/Math/CAngle.h"

//------------------------------------------------------------------------------------------
class CQuaternion;
//------------------------------------------------------------------------------------------

class CAxisAngle
{
public:
   inline explicit CAxisAngle( CInputStream& stream);
   inline explicit CAxisAngle( const CVector3& axis, const CAngle& angle );
   inline explicit CAxisAngle( const CVector3& axis );

   inline void PutTo(COutputStream & stream) const;

   inline CVector3 const GetAxisNormalized() const;
   inline CVector3 const & GetAxis() const;
   ENGINE_API CAngle const GetAngle() const;

   inline CAxisAngle const operator + (const CAxisAngle& other ) const
   {
      return CAxisAngle( mAxis + other.mAxis );
   }
   inline CAxisAngle & operator += (const CAxisAngle& other )
   {
      mAxis += other.mAxis;
      return *this;
   }

   inline CAxisAngle const operator - (const CAxisAngle& other ) const
   {
      return CAxisAngle( mAxis - other.mAxis );
   }
   inline CAxisAngle & operator -= (const CAxisAngle& other )
   {
      mAxis -= other.mAxis;
      return *this;
   }

   inline CAxisAngle const operator * (const real32 other ) const
   {
      return CAxisAngle( mAxis * other );
   }

   // axis angle factory
   inline static CAxisAngle const Identity();
   ENGINE_API static CAxisAngle FromQuaternion(CQuaternion const &q);

public:
   CVector3 mAxis;
};

//------------------------------------------------------------------------------------------

#include "Engine/Math/CAxisAngle.inl"
