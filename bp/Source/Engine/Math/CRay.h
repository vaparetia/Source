//------------------------------------------------------------------------------------------
// CRay.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

class CInputStream;
class COutputStream;

//------------------------------------------------------------------------------------------

#include "Engine/Math/CVector3.h"

//------------------------------------------------------------------------------------------

class CRay
{
public:
   inline explicit CRay( const CVector3& origin, const CVector3& direction );
   
   // read from stream
   inline explicit CRay( CInputStream& stream );

   // put vector to stream
   inline void PutTo( COutputStream& stream ) const;

   // accessors
   inline CVector3 const & GetOrigin() const;
   inline CVector3 const & GetDirection() const;

   inline void SetOrigin(CVector3 const & value);
   inline void SetDirection(CVector3 const & value);

private:
   CVector3 mOrigin;
   CVector3 mDirection;
};

//------------------------------------------------------------------------------------------

#include "Engine/Math/CRay.inl"

//------------------------------------------------------------------------------------------
