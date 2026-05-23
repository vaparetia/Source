//------------------------------------------------------------------------------------------
// CPlane.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

class CInputStream;
class COutputStream;

//------------------------------------------------------------------------------------------

#include "Engine/Math/CVector3.h"

//------------------------------------------------------------------------------------------

class CPlane
{
public:
   inline explicit CPlane( const CVector3& pnt1, const CVector3& pnt2, const CVector3& pnt3 );
   inline explicit CPlane( const CVector3& reference, const CVector3& normal );
   inline explicit CPlane( const real32 constant, const CVector3& normal );

   inline const real32 GetConstant() const;
   inline const CVector3& GetNormal() const;
   
   inline const bool IsFacing( const CVector3& point ) const;
   inline real32 const GetDistance( CVector3 const & point ) const;

private:
   CVector3 mNormal;
   real32   mConstant;
};

//------------------------------------------------------------------------------------------

#include "Engine/Primitives/CPlane.inl"

//------------------------------------------------------------------------------------------
