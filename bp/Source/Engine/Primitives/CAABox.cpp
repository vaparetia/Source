//----------------------------------------------------------------------------
// CAABox.cpp
// Bluepoint
// Copyright 2001
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Primitives/CAABox.h"

//----------------------------------------------------------------------------

#include "Engine/Math/CMatrix34.h"
#include "Engine/Math/CMatrix4.h"
#include "Engine/Streams/CInputStream.h"
#include "Engine/Streams/COutputStream.h"

//----------------------------------------------------------------------------

using namespace std;
using namespace bpe;

//----------------------------------------------------------------------------

static CAABox const sZeroBox(CVector3::Zero(), CVector3::Zero());

static CAABox const sMaxInvertedBox(CVector3(gkReal32Max, gkReal32Max, gkReal32Max), 
                                    CVector3(gkReal32MaxNeg, gkReal32MaxNeg, gkReal32MaxNeg));

static CAABox const sMaxBox(CVector3(gkReal32MaxNeg, gkReal32MaxNeg, gkReal32MaxNeg), 
                            CVector3(gkReal32Max, gkReal32Max, gkReal32Max));

//----------------------------------------------------------------------------

CAABox::CAABox( CInputStream& inputStream )
:  mMin( inputStream )
,  mMax( inputStream )
{
}

//----------------------------------------------------------------------------

void CAABox::PutTo( COutputStream& outputStream ) const
{
   mMin.PutTo(outputStream);
   mMax.PutTo(outputStream);
}

//----------------------------------------------------------------------------

void CAABox::Include( const CVector3& point )
{
   mMin[0] = min_val( mMin[0], point[0] );
   mMin[1] = min_val( mMin[1], point[1] );
   mMin[2] = min_val( mMin[2], point[2] );

   mMax[0] = max_val( mMax[0], point[0] );
   mMax[1] = max_val( mMax[1], point[1] );
   mMax[2] = max_val( mMax[2], point[2] );

}

//----------------------------------------------------------------------------

void CAABox::Include( const CAABox& box )
{
   Include( box.GetMin() );
   Include( box.GetMax() );
}

//----------------------------------------------------------------------------

bool CAABox::Contains( CVector3 const & point ) const
{
   CVector3 const distanceFromMin = point - GetMin();
   CVector3 const distanceFromMax = GetMax() - point;
   // at most, one of each axis is negative
   // if both are positive or zero, we are inside
   CVector3 const perAxisCheck = CVector3::ElementMultiply(distanceFromMin, distanceFromMax);
   // find a negative
   real32 anyNegative = MathUtils::FSel(perAxisCheck.GetX(), perAxisCheck.GetY(), perAxisCheck.GetX());
   anyNegative = MathUtils::FSel(anyNegative, perAxisCheck.GetZ(), anyNegative);
   return anyNegative >= 0.0f;
}

//----------------------------------------------------------------------------

bool CAABox::Overlaps( CAABox const & box ) const
{
   CVector3 const distance = GetCenter() - box.GetCenter();
   CVector3 const absDistance = distance.Abs();
   CVector3 const bothExtents = 0.5f*(GetExtents() + box.GetExtents());
   CVector3 const separation = absDistance - bothExtents;
   // now see if any separation axis > 0 (no overlap)
   real32 anySeparation = MathUtils::FSel(separation.GetX(), separation.GetX(), separation.GetY());
   anySeparation = MathUtils::FSel(anySeparation, anySeparation, separation.GetZ());
   return anySeparation <= 0.0f;
}

//----------------------------------------------------------------------------

CPlane const CAABox::BuildFacePlane( const int faceIndex ) const
{
   switch( faceIndex )
   {
      // left
   case 0:
      return CPlane( GetMin(), CVector3::NegXAxis() );
      break;
      // right
   case 1:
      return CPlane( GetMax(), CVector3::XAxis() );
      break;
      // bottom
   case 2:
      return CPlane( GetMin(), CVector3::NegYAxis() );
      break;
      // top
   case 3:
      return CPlane( GetMax(), CVector3::YAxis() );
      break;
      // front
   case 4:
      return CPlane( GetMin(), CVector3::NegZAxis() );
      break;
      // back
   case 5:
      return CPlane( GetMax(), CVector3::ZAxis() );
      break;
   default:
      return CPlane( CVector3::Zero(), CVector3::Zero() );
   }

}

//----------------------------------------------------------------------------

CAABox const CAABox::BuildTransformedBoxFromFace( CMatrix34 const &transform, const int faceIndex ) const
{
   // Take a face index, transforms its four points, and returns a "box" surrounding those four points

   static int const skOtherFaces[] = 
   {
      1, 2,
      0, 2,
      0, 1
   };

   CVector3 pt = CVector3::Zero();
   int const ptIndex = faceIndex >> 1;
   int const ptExtent = faceIndex & 1;
   int const ptOther1 = skOtherFaces[ ptIndex * 2 ];
   int const ptOther2 = skOtherFaces[ ptIndex * 2 + 1 ];

   pt[ ptIndex ] = GetExtentPoint( ptExtent )[ ptIndex ];

   pt[ ptOther1 ] = mMin[ ptOther1 ];
   pt[ ptOther2 ] = mMin[ ptOther2 ];

   CAABox newBox = CAABox::MakeMaxInvertedBox();
   newBox.Include( transform * pt );

   pt[ ptOther1 ] = mMin[ ptOther1 ];
   pt[ ptOther2 ] = mMax[ ptOther2 ];
   newBox.Include( transform * pt );

   pt[ ptOther1 ] = mMax[ ptOther1 ];
   pt[ ptOther2 ] = mMin[ ptOther2 ];
   newBox.Include( transform * pt );

   pt[ ptOther1 ] = mMax[ ptOther1 ];
   pt[ ptOther2 ] = mMax[ ptOther2 ];
   newBox.Include( transform * pt );

   return newBox;
}

//----------------------------------------------------------------------------

void CAABox::Split( int const axis, CAABox & leftChild, CAABox & rightChild ) const
{
   real32 const splitValue = GetCenter()[axis];
   CVector3 maxLeft = GetMax();
   maxLeft[axis] = splitValue;
   CVector3 minRight = GetMin();
   minRight[axis] = splitValue;
   leftChild = CAABox(GetMin(), maxLeft);
   rightChild = CAABox(minRight, GetMax());
}

//----------------------------------------------------------------------------

CAABox const CAABox::GetTransformedAABox( CMatrix4 const & transform ) const
{
   CAABox transformedBox = CAABox::MakeMaxInvertedBox();
   for( int i = 0; i < 8; ++i )
   {
      transformedBox.Include( transform * GetPoint( i ) );
   }
   
   return transformedBox;
}

//----------------------------------------------------------------------------
// Based on code:
//
// Transforming Axis-Aligned Bounding Boxes
// by Jim Arvo
// from "Graphics Gems", Academic Press, 1990
//----------------------------------------------------------------------------

CAABox const CAABox::GetTransformedAABox( CMatrix34 const & transform ) const
{
   // x axis of matrix
   real32 const t00 = transform.Get00();
   real32 const t01 = transform.Get01();
   real32 const t02 = transform.Get02();
   
   // y axis of matrix
   real32 const t10 = transform.Get10();
   real32 const t11 = transform.Get11();
   real32 const t12 = transform.Get12();

   // z axis of matrix
   real32 const t20 = transform.Get20();
   real32 const t21 = transform.Get21();
   real32 const t22 = transform.Get22();

   // translation
   real32 const t30 = transform.Get30();
   real32 const t31 = transform.Get31();
   real32 const t32 = transform.Get32();

   // min/max
   real32 const minX = mMin.GetX();
   real32 const minY = mMin.GetY();
   real32 const minZ = mMin.GetZ();
   real32 const maxX = mMax.GetX();
   real32 const maxY = mMax.GetY();
   real32 const maxZ = mMax.GetZ();

   // output min/max
   real32 outMinX = t30;
   real32 outMinY = t31;
   real32 outMinZ = t32;
   real32 outMaxX = t30;
   real32 outMaxY = t31;
   real32 outMaxZ = t32;

   // determine new min/max
   // outMin = translation + (min < max) ? min : max;
   // outMax = translation + (min < max) ? max : min;

   // which (so fsel can be used) turns into:
   // outMin = translation + (min - max) >= 0 ? max : min;
   // outMax = translation + (min - max) >= 0 ? min : max;

#define EXPAND_BY_TRANSFORMED_ELEMENT(MIN, MAX, TRANSFORM, OUTMIN, OUTMAX)\
   {\
      real32 tempMin = MIN * TRANSFORM;\
      real32 tempMax = MAX * TRANSFORM;\
      real32 tempTest = tempMin - tempMax;\
      OUTMIN += MathUtils::FSel(tempTest, tempMax, tempMin);\
      OUTMAX += MathUtils::FSel(tempTest, tempMin, tempMax);\
   }

   EXPAND_BY_TRANSFORMED_ELEMENT(minX, maxX, t00, outMinX, outMaxX);
   EXPAND_BY_TRANSFORMED_ELEMENT(minY, maxY, t10, outMinX, outMaxX);
   EXPAND_BY_TRANSFORMED_ELEMENT(minZ, maxZ, t20, outMinX, outMaxX);
                                               
   EXPAND_BY_TRANSFORMED_ELEMENT(minX, maxX, t01, outMinY, outMaxY);
   EXPAND_BY_TRANSFORMED_ELEMENT(minY, maxY, t11, outMinY, outMaxY);
   EXPAND_BY_TRANSFORMED_ELEMENT(minZ, maxZ, t21, outMinY, outMaxY);
                                               
   EXPAND_BY_TRANSFORMED_ELEMENT(minX, maxX, t02, outMinZ, outMaxZ);
   EXPAND_BY_TRANSFORMED_ELEMENT(minY, maxY, t12, outMinZ, outMaxZ);
   EXPAND_BY_TRANSFORMED_ELEMENT(minZ, maxZ, t22, outMinZ, outMaxZ);

   return CAABox( CVector3(outMinX, outMinY, outMinZ), CVector3(outMaxX, outMaxY, outMaxZ));
}

//----------------------------------------------------------------------------

CVector3 const CAABox::GetPoint( const int index ) const
{
   return CVector3( ( index & 4 ) ? GetMin().GetX() : GetMax().GetX(),
                    ( index & 2 ) ? GetMin().GetY() : GetMax().GetY(),
                    ( index & 1 ) ? GetMin().GetZ() : GetMax().GetZ() );
}

//----------------------------------------------------------------------------

CAABox const & CAABox::MakeZeroBox()
{
   return sZeroBox;
}

//----------------------------------------------------------------------------

CAABox const & CAABox::MakeMaxInvertedBox()
{
   return sMaxInvertedBox;
}

//----------------------------------------------------------------------------

CAABox const & CAABox::MakeMaxBox()
{
   return sMaxBox;
}

//----------------------------------------------------------------------------

bpe::reserved_vector<uint16, 24> CAABox::GetLineLoopIndices() const
{
   bpe::reserved_vector<uint16, 24> indices;
   // front
   indices.push_back( 0 );   
   indices.push_back( 1 );   
   indices.push_back( 1 );   
   indices.push_back( 3 );   
   indices.push_back( 3 );   
   indices.push_back( 2 );   
   indices.push_back( 2 );   
   indices.push_back( 0 );   
   // back      
   indices.push_back( 4 );   
   indices.push_back( 5 );   
   indices.push_back( 5 );   
   indices.push_back( 7 );   
   indices.push_back( 7 );   
   indices.push_back( 6 );   
   indices.push_back( 6 );   
   indices.push_back( 4 );   
   // connections
   indices.push_back( 0 );
   indices.push_back( 4 );
   indices.push_back( 2 );
   indices.push_back( 6 );
   indices.push_back( 3 );
   indices.push_back( 7 );
   indices.push_back( 1 );
   indices.push_back( 5 );
   return indices;
}

//----------------------------------------------------------------------------

real32 CAABox::GetPointDistanceSquared(CVector3 const &point) const
{
   CVector3 const excessMax = point - GetMax();
   CVector3 const excessMin = GetMin() - point;

   real32 squaredDistance = 0.0f;
   for (int i = 0; i < 3; ++i)
   {
      if (excessMin[i] > 0.0f)
      {
         squaredDistance += excessMin[i]*excessMin[i];
      }
      else if (excessMax[i] > 0.0f)
      {
         squaredDistance += excessMax[i]*excessMax[i];
      }
   }

   return squaredDistance;
}

//----------------------------------------------------------------------------

CVector3 const CAABox::GetClosestPoint(CVector3 const &point) const
{
   CVector3 closest(CVector3::kConstructUninitialized);

   for (int i = 0; i < 3; ++i)
   {
      closest[i] = MathUtils::ClampMinMax(point[i], mMin[i], mMax[i]);
   }

   return closest;
}

//----------------------------------------------------------------------------
