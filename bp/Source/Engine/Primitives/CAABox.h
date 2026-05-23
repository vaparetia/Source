//------------------------------------------------------------------------------------------
// CAABox.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

class CInputStream;
class COutputStream;

//------------------------------------------------------------------------------------------

#include "Engine/Math/CVector3.h"
#include "Engine/Primitives/CPlane.h"

//------------------------------------------------------------------------------------------

class CMatrix34;
class CMatrix4;

//------------------------------------------------------------------------------------------

class CAABox
{
public:
   inline explicit CAABox( const CVector3& min, const CVector3& max );

   CVector3 const & GetMin() const { return mMin; }
   CVector3 const & GetMax() const { return mMax; }

   CVector3 & Min() { return mMin; }
   CVector3 & Max() { return mMax; }
   
   // Gets min if index is 0, max if index is 1
   CVector3 const &GetExtentPoint( int index ) const { return (&mMin)[index]; }

   CVector3 GetExtents() const { return GetMax() - GetMin(); }
   CVector3 GetCenter() const { return ( GetMin() + GetMax() ) * 0.5f; }

   ENGINE_API void Include( const CVector3& point );
   ENGINE_API void Include( const CAABox& box );

   ENGINE_API bool Contains( CVector3 const & point ) const;
   ENGINE_API bool Overlaps( CAABox const & box ) const;
   inline bool const InsidePlane( const CPlane& plane ) const;

   ENGINE_API explicit CAABox( CInputStream& inputStream );
   ENGINE_API void PutTo( COutputStream& outputStream ) const;

   ENGINE_API static CAABox const & MakeZeroBox();
   ENGINE_API static CAABox const & MakeMaxInvertedBox();
   ENGINE_API static CAABox const & MakeMaxBox();

   ENGINE_API CPlane const BuildFacePlane( const int faceIndex ) const;
   ENGINE_API CAABox const BuildTransformedBoxFromFace( CMatrix34 const &transform, const int faceIndex ) const;
   ENGINE_API void Split( int const axis, CAABox& leftChild, CAABox& rightChild ) const;

   ENGINE_API CAABox const GetTransformedAABox( CMatrix4 const & transform ) const;
   ENGINE_API CAABox const GetTransformedAABox( CMatrix34 const & transform ) const;
   
   CAABox const GetScaledAABoxAroundOrigin( CVector3 const &scale ) const
   {
      return CAABox(CVector3::ElementMultiply(mMin, scale), CVector3::ElementMultiply(mMax, scale));
   };

   CAABox const GetScaledAABoxAroundCenter( CVector3 const & scale ) const
   {
      CVector3 const center = GetCenter();
      return CAABox( center + CVector3::ElementMultiply(mMin - center, scale), 
                     center + CVector3::ElementMultiply(mMax - center, scale) );
   };

   ENGINE_API CVector3 const GetPoint( const int index ) const;

   real32 GetVolume() const
   {
      CVector3 const extents = GetExtents();
      return extents.GetX() * extents.GetY() * extents.GetZ();
   }

   real32 GetSurfaceArea() const
   {
      CVector3 const extents = GetExtents();
      return 2.0f*(extents.GetX()*extents.GetY() + extents.GetY()*extents.GetZ() + extents.GetZ()*extents.GetX());
   }

   static CAABox MakeBoxCenteredAt( CVector3 const & pos, CVector3 const & size )
   {
      return CAABox(pos - size, pos + size);
   }

   static CAABox MakeBoxCenteredAt( CVector3 const & pos, real32 const size )
   {
      return MakeBoxCenteredAt(pos, CVector3(size, size, size));
   }

   inline CVector3 const GetClosestPointAlongDirection( CVector3 const & direction ) const;
   inline CVector3 const GetFurthestPointAlongDirection( CVector3 const & direction ) const;

   ENGINE_API real32 GetPointDistanceSquared(CVector3 const &point) const;
   ENGINE_API CVector3 const GetClosestPoint(CVector3 const &point) const;

   #if 0 
   //----------------------------------------------------
   // layout for GetCornerPoint function
   /**/    6--------7
   /**/   /|       /|
   /**/  / |      / |
   /**/ 2--+-----3  |
   /**/ |  |     |  |
   /**/ |  4-----+--5
   /**/ | /      | /
   /**/ |/       |/
   /**/ 0--------1
   //----------------------------------------------------
   #endif
   
   ENGINE_API bpe::reserved_vector<uint16, 24> GetLineLoopIndices() const;

private:
   CVector3 mMin;
   CVector3 mMax;
};

//------------------------------------------------------------------------------------------

#include "Engine/Primitives/CAABox.inl"

//------------------------------------------------------------------------------------------
