//------------------------------------------------------------------------------------------
// BPEIntersection.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Math/CVector3.h"
#include "Engine/Math/CRay.h"
#include "Engine/Primitives/CPlane.h"

//------------------------------------------------------------------------------------------

class CAABox;

//------------------------------------------------------------------------------------------

class CTriangle
{
public:
   CTriangle( CVector3 const & v1, CVector3 const & v2, CVector3 const & v3 )
   :  mVertex1( v1 )
   ,  mVertex2( v2 )
   ,  mVertex3( v3 )
   {
   }

   CVector3 const & GetVertex1() const { return mVertex1; }
   CVector3 const & GetVertex2() const { return mVertex2; }
   CVector3 const & GetVertex3() const { return mVertex3; }

private:
   CVector3 mVertex1;
   CVector3 mVertex2;
   CVector3 mVertex3;

};

//------------------------------------------------------------------------------------------

class CPrimResult
{
public:
   CPrimResult( const bool valid ) : mValid( valid ) { }
   const bool IsValid() const { return mValid; }

private:
   bool        mValid;
};

//------------------------------------------------------------------------------------------

class CPrimPointResult : public CPrimResult
{
public:
   CPrimPointResult( const bool valid, const CVector3& position )
   :  CPrimResult( valid )
   ,  mPosition( position )
   {
   }
   
   const CVector3& GetPosition() const
   {
      return mPosition;
   }

private:
   CVector3 mPosition;
};

//------------------------------------------------------------------------------------------

namespace BPEIntersection
{
   ENGINE_API CPrimPointResult const Test( const CRay& ray, const CPlane& plane );
   ENGINE_API CPrimPointResult const Test( const CRay& ray, const CAABox& box );
   
   // ray -> plane
   ENGINE_API bool const Test( CRay const & ray, CPlane const & plane, real32 & outT );
   // ray -> triangle
   ENGINE_API bool const Test( CRay const & ray, CTriangle const & triangle, real32 & outT, real32 & outU, real32 & outV );

   // ray -> triangle
   ENGINE_API bool const TestRayTriangle( CRay const & ray, CVector3 const & v1, CVector3 const & v2, CVector3 const & v3, bool const ignoreBackfaces, real32 & outT, real32 & outU, real32 & outV );

   // ray <-> aabox
   ENGINE_API bool const TestBoolean( CRay const & ray, CAABox const & box );
   // aabox <-> aabox
   ENGINE_API bool const TestBoolean( CAABox const & box1, CAABox const & box2 );
   // aabox <-> triangle
   ENGINE_API bool const TestBoolean( CAABox const & box, CTriangle const & triangle );
   // ray -> triangle
   ENGINE_API bool const TestBoolean( CRay const & ray, CTriangle const & triangle );

} // namespace BPEIntersection

//------------------------------------------------------------------------------------------
