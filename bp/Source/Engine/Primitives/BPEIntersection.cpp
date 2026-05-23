//------------------------------------------------------------------------------------------
// BPEIntersection.cpp
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "BPEIntersection.h"

//------------------------------------------------------------------------------------------

#include "Engine/Primitives/CAABox.h"

//------------------------------------------------------------------------------------------

bool const BPEIntersection::Test( CRay const & ray, CPlane const & plane, real32 & outT )
{
   real32 const cosAlpha = CVector3::Dot( ray.GetDirection(), plane.GetNormal() );
   
   if( cosAlpha < -gkEpsilon32 ||
       cosAlpha > gkEpsilon32 )
   {
      real32 const deltaD = plane.GetConstant() - CVector3::Dot( ray.GetOrigin(), plane.GetNormal() );
      outT = deltaD / cosAlpha;
      return true;
   }

   return false;
}

//------------------------------------------------------------------------------------------

CPrimPointResult const BPEIntersection::Test( const CRay& ray, const CPlane& plane )
{
   const real32 cosAlpha = CVector3::Dot( ray.GetDirection(), plane.GetNormal() );
   
   // make sure ray is not coolinear or intersecting from the back of the poly
   if( cosAlpha < -gkEpsilon32 ||
       cosAlpha > gkEpsilon32 )
   {
      const real32 deltaD = plane.GetConstant() - CVector3::Dot( ray.GetOrigin(), plane.GetNormal() );
      return CPrimPointResult( true, ray.GetOrigin() + ray.GetDirection() * ( deltaD / cosAlpha ) );
   }
   else
   {
      return CPrimPointResult( false, CVector3::Zero() );
   }
}

//------------------------------------------------------------------------------------------

#define RAYAABB_EPSILON 0.00001f

const CPrimPointResult BPEIntersection::Test( const CRay& ray, const CAABox& box )
{
   bool inside = true;
 
   CVector3 coord( CVector3::Zero() );

   CVector3 boxMin = box.GetMin();
   CVector3 boxMax = box.GetMax();
   
   CVector3 maxT( -1, -1, -1 );
   
   const CVector3 origin = ray.GetOrigin();
   const CVector3 dir = ray.GetDirection();

   // Find candidate planes.
   for( int i = 0; i < 3; i++ )
   {
      if( origin[i] < boxMin[i] )
      {
         coord[i]	= boxMin[i];
         inside = false;
         
         // Calculate T distances to candidate planes
         if( dir[i] != 0.0f )
         {
            maxT[i] = ( boxMin[i] - origin[i] ) / dir[i];
         }
      }
      else if( origin[i] > boxMax[i] )
      {
         coord[i]	= boxMax[i];
         inside = false;
         
         // Calculate T distances to candidate planes
         if( dir[i] != 0.0f )
         {
            maxT[i] = ( boxMax[i] - origin[i] ) / dir[i];
         }
      }
   }
   
   // Ray origin inside bounding box
   if( inside )
   {
      return CPrimPointResult( true, origin );
   }
   
   // Get largest of the maxT's for final choice of intersection
   
   int whichPlane = 0;
   if( maxT[1] > maxT[whichPlane] )
      whichPlane = 1;
   if( maxT[2] > maxT[whichPlane] )	
      whichPlane = 2;
   
   // Check final candidate actually inside box
   
   if( maxT[whichPlane] < 0.0f ) 
   {
      return CPrimPointResult( false, CVector3::Zero() );
   }
   
   for( int i = 0; i < 3; i++ )
   {
      if( i != whichPlane )
      {
         coord[i] = origin[i] + maxT[whichPlane] * dir[i];

         if( coord[i] < boxMin[i] - RAYAABB_EPSILON || coord[i] > boxMax[i] + RAYAABB_EPSILON )
         {
            return CPrimPointResult( false, CVector3::Zero() );
         }
      }
   }

   return CPrimPointResult( true, coord );
}

//------------------------------------------------------------------------------------------

const bool BPEIntersection::TestBoolean( const CRay& ray, const CAABox& box )
{
   bool inside = true;
 
   const CVector3& boxMin = box.GetMin();
   const CVector3& boxMax = box.GetMax();
   
   CVector3 maxT( -1, -1, -1 );
   
   const CVector3& origin = ray.GetOrigin();
   const CVector3& dir = ray.GetDirection();

   // Find candidate planes.
   for( int i = 0; i < 3; i++ )
   {
      if( origin[i] < boxMin[i] )
      {
         inside = false;
         
         // Calculate T distances to candidate planes
         if( dir[i] != 0.0f )
         {
            maxT[i] = ( boxMin[i] - origin[i] ) / dir[i];
         }
      }
      else if( origin[i] > boxMax[i] )
      {
         inside = false;
         
         // Calculate T distances to candidate planes
         if( dir[i] != 0.0f )
         {
            maxT[i] = ( boxMax[i] - origin[i] ) / dir[i];
         }
      }
   }
   
   // Ray origin inside bounding box
   if( inside )
   {
      return true;
   }
   
   // Get largest of the maxT's for final choice of intersection
   
   int whichPlane = 0;
   if( maxT[1] > maxT[whichPlane] )
      whichPlane = 1;
   if( maxT[2] > maxT[whichPlane] )	
      whichPlane = 2;
   
   // Check final candidate actually inside box
   if( maxT[whichPlane] < 0.0f ) 
   {
      return false;
   }
   
   CVector3 coord( CVector3::Zero() );

   for( int i = 0; i < 3; i++ )
   {
      if( i != whichPlane )
      {
         coord[i] = origin[i] + maxT[whichPlane] * dir[i];

         if( coord[i] < boxMin[i] - RAYAABB_EPSILON || coord[i] > boxMax[i] + RAYAABB_EPSILON )
         {
            return false;
         }
      }
   }

   return true;
}

//------------------------------------------------------------------------------------------

const bool BPEIntersection::TestBoolean( CAABox const & box1, CAABox const & box2 )
{
   return box1.Overlaps(box2);
}

//------------------------------------------------------------------------------------------

#ifdef ATI_DOUBLE_TRI_BOX_INTERSECTION_TEST
#define TBM_FLOAT real64
#define TBM_ZERO 0.0
#else
#define TBM_FLOAT real32
#define TBM_ZERO 0.0f
#endif

#define CROSS(dest,v1,v2) \
          dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
          dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
          dest[2]=v1[0]*v2[1]-v1[1]*v2[0];

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define SUB(dest,v1,v2) \
          dest[0]=v1[0]-v2[0]; \
          dest[1]=v1[1]-v2[1]; \
          dest[2]=v1[2]-v2[2];

#define FINDMINMAX(x0,x1,x2,min,max) \
  min = max = x0;   \
  if(x1<min) min=x1;\
  if(x1>max) max=x1;\
  if(x2<min) min=x2;\
  if(x2>max) max=x2;

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

static inline bool AtiPlaneBoxOverlap( TBM_FLOAT normal[3], TBM_FLOAT d, CVector3 const & maxbox )
{
   TBM_FLOAT vmin[3];
   TBM_FLOAT vmax[3];
   for (int32 q = kAX; q <= kAZ; q++)
   {
      if (normal[q] > TBM_ZERO)
      {
         vmin[q] = -maxbox[q];
         vmax[q] = maxbox[q];
      }
      else
      {
         vmin[q] = maxbox[q];
         vmax[q] = -maxbox[q];
      }
   }
   if (DOT (normal, vmin) + d > TBM_ZERO)
   {
      return false;
   }
   if (DOT (normal, vmax) + d >= TBM_ZERO)
   {
      return true;
   }
   return false;
}

// ======================== X-tests ========================
#define AXISTEST_X01(a, b, fa, fb)             \
    p0 = a*v0[kAY] - b*v0[kAZ];                    \
    p2 = a*v2[kAY] - b*v2[kAZ];                    \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
    rad = fa * boxhalfsize[kAY] + fb * boxhalfsize[kAZ];   \
    if(min>rad || max<-rad) return false;

#define AXISTEST_X2(a, b, fa, fb)              \
    p0 = a*v0[kAY] - b*v0[kAZ];                    \
    p1 = a*v1[kAY] - b*v1[kAZ];                    \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize[kAY] + fb * boxhalfsize[kAZ];   \
    if(min>rad || max<-rad) return false;

// ======================== Y-tests ========================
#define AXISTEST_Y02(a, b, fa, fb)             \
    p0 = -a*v0[kAX] + b*v0[kAZ];                   \
    p2 = -a*v2[kAX] + b*v2[kAZ];                       \
        if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;} \
    rad = fa * boxhalfsize[kAX] + fb * boxhalfsize[kAZ];   \
    if(min>rad || max<-rad) return false;

#define AXISTEST_Y1(a, b, fa, fb)              \
    p0 = -a*v0[kAX] + b*v0[kAZ];                   \
    p1 = -a*v1[kAX] + b*v1[kAZ];                       \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize[kAX] + fb * boxhalfsize[kAZ];   \
    if(min>rad || max<-rad) return false;

// ======================== Z-tests ========================

#define AXISTEST_Z12(a, b, fa, fb)             \
    p1 = a*v1[kAX] - b*v1[kAY];                    \
    p2 = a*v2[kAX] - b*v2[kAY];                    \
        if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;} \
    rad = fa * boxhalfsize[kAX] + fb * boxhalfsize[kAY];   \
    if(min>rad || max<-rad) return false;

#define AXISTEST_Z0(a, b, fa, fb)              \
    p0 = a*v0[kAX] - b*v0[kAY];                \
    p1 = a*v1[kAX] - b*v1[kAY];                    \
        if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;} \
    rad = fa * boxhalfsize[kAX] + fb * boxhalfsize[kAY];   \
    if(min>rad || max<-rad) return false;

///////////////////////////////////////////////////////////////////////////////

bool const BPEIntersection::TestBoolean( CAABox const & box, CTriangle const & triangle )
{
   CVector3 boxcenter = box.GetCenter();
   CVector3 boxhalfsize = box.GetExtents() * 0.5f;
   CVector3 const & triVert0 = triangle.GetVertex1();
   CVector3 const & triVert1 = triangle.GetVertex2();
   CVector3 const & triVert2 = triangle.GetVertex3();

   // use separating axis theorem to test overlap between triangle and box
   // need to test for overlap in these directions:
   // 1) the {x,y,z}-directions (actually, since we use the AABB of the triangle
   //    we do not even need to test these)
   // 2) normal of the triangle
   // 3) crossproduct(edge from tri, {x,y,z}-directin)
   //    this gives 3x3=9 more tests
   TBM_FLOAT v0[3], v1[3], v2[3];
   TBM_FLOAT min, max, d, p0, p1, p2, rad, fex, fey, fez;
   TBM_FLOAT normal[3], e0[3], e1[3], e2[3];

   // This is the fastest branch on Sun
   // move everything so that the boxcenter is in (0,0,0)
   SUB (v0, triVert0, boxcenter);
   SUB (v1, triVert1, boxcenter);
   SUB (v2, triVert2, boxcenter);

   // compute triangle edges
   SUB (e0, v1, v0);      // tri edge 0
   SUB (e1, v2, v1);      // tri edge 1
   SUB (e2, v0, v2);      // tri edge 2

   // Bullet 3:
   //  test the 9 tests first (this was faster)
   fex = (TBM_FLOAT)fabs (e0[kAX]);
   fey = (TBM_FLOAT)fabs (e0[kAY]);
   fez = (TBM_FLOAT)fabs (e0[kAZ]);
   AXISTEST_X01 (e0[kAZ], e0[kAY], fez, fey);
   AXISTEST_Y02 (e0[kAZ], e0[kAX], fez, fex);
   AXISTEST_Z12 (e0[kAY], e0[kAX], fey, fex);

   fex = (TBM_FLOAT)fabs (e1[kAX]);
   fey = (TBM_FLOAT)fabs (e1[kAY]);
   fez = (TBM_FLOAT)fabs (e1[kAZ]);
   AXISTEST_X01 (e1[kAZ], e1[kAY], fez, fey);
   AXISTEST_Y02 (e1[kAZ], e1[kAX], fez, fex);
   AXISTEST_Z0 (e1[kAY], e1[kAX], fey, fex);

   fex = (TBM_FLOAT)fabs (e2[kAX]);
   fey = (TBM_FLOAT)fabs (e2[kAY]);
   fez = (TBM_FLOAT)fabs (e2[kAZ]);
   AXISTEST_X2 (e2[kAZ], e2[kAY], fez, fey);
   AXISTEST_Y1 (e2[kAZ], e2[kAX], fez, fex);
   AXISTEST_Z12 (e2[kAY], e2[kAX], fey, fex);

   // Bullet 1:
   //  first test overlap in the {x,y,z}-directions
   //  find min, max of the triangle each direction, and test for overlap in
   //  that direction -- this is equivalent to testing a minimal AABB around
   //  the triangle against the AABB

   // test in X-direction
   FINDMINMAX (v0[kAX], v1[kAX], v2[kAX], min, max);
   if (min > boxhalfsize[kAX] || max < -boxhalfsize[kAX])
   {
      return false;
   }

   // test in Y-direction
   FINDMINMAX (v0[kAY], v1[kAY], v2[kAY], min, max);
   if (min > boxhalfsize[kAY] || max < -boxhalfsize[kAY])
   {
      return false;
   }

   // test in Z-direction
   FINDMINMAX (v0[kAZ], v1[kAZ], v2[kAZ], min, max);
   if (min > boxhalfsize[kAZ] || max < -boxhalfsize[kAZ])
   {
      return false;
   }

   // Bullet 2:
   //  test if the box intersects the plane of the triangle
   //  compute plane equation of triangle: normal*x+d=0
   CROSS (normal, e0, e1);
   d = -DOT (normal, v0);  // plane eq: normal.x+d=0
   return AtiPlaneBoxOverlap (normal, d, boxhalfsize);
}

//------------------------------------------------------------------------------------------

bool const BPEIntersection::Test( CRay const & ray, 
                                CTriangle const & triangle, 
                                real32 & outT, 
                                real32 & outU, 
                                real32 & outV )
{
   CVector3 const edge1( triangle.GetVertex2() - triangle.GetVertex1() );
   CVector3 const edge2( triangle.GetVertex3() - triangle.GetVertex1() );

   CVector3 const pvec( CVector3::Cross( ray.GetDirection(), edge2 ) );

   real32 const det = CVector3::Dot( edge1, pvec );

   if( (det > -gkEpsilon32) && (det < gkEpsilon32) )
   {
      return false;
   }

   real32 const invDet = 1.0f / det;

   CVector3 tvec( ray.GetOrigin() - triangle.GetVertex1() );

   outU = CVector3::Dot( tvec, pvec ) * invDet;
   
   if( outU < -gkEpsilon32 )
   {
      return false;
   }
   
   CVector3 const qvec( CVector3::Cross( tvec, edge1 ) );

   outV = CVector3::Dot( ray.GetDirection(), qvec ) * invDet;
   
   if( outV < -gkEpsilon32 ||
      ( outU + outV ) > ( 1.0f + gkEpsilon32 ) )
   {
      return false;
   }
   
   outT = CVector3::Dot( edge2, qvec ) * invDet;

   if( outT < -gkEpsilon32 )
      return false;

   return true;
}

//------------------------------------------------------------------------------------------

bool const BPEIntersection::TestRayTriangle( CRay const & ray, CVector3 const & v1, CVector3 const & v2, CVector3 const & v3, bool const ignoreBackfaces, real32 & outT, real32 & outU, real32 & outV )
{
   CVector3 const edge1( v2 - v1 );
   CVector3 const edge2( v3 - v1 );

   CVector3 const pvec( CVector3::Cross( ray.GetDirection(), edge2 ) );

   real32 const det = CVector3::Dot( edge1, pvec );

   if( det > -gkEpsilon32 && (ignoreBackfaces | (det < gkEpsilon32)) )
   {
      return false;
   }

   real32 const invDet = 1.0f / det;

   CVector3 tvec( ray.GetOrigin() - v1 );

   outU = CVector3::Dot( tvec, pvec ) * invDet;

   if( outU < -gkEpsilon32 )
   {
      return false;
   }

   CVector3 const qvec( CVector3::Cross( tvec, edge1 ) );

   outV = CVector3::Dot( ray.GetDirection(), qvec ) * invDet;

   if( outV < -gkEpsilon32 ||
      ( outU + outV ) > ( 1.0f + gkEpsilon32 ) )
   {
      return false;
   }

   outT = CVector3::Dot( edge2, qvec ) * invDet;

   if( outT < -gkEpsilon32 )
      return false;

   return true;
}

//------------------------------------------------------------------------------------------
   
bool const BPEIntersection::TestBoolean( CRay const & ray, CTriangle const & triangle )
{
   real32 t1, t2, t3;
   return Test( ray, triangle, t1, t2, t3 );
}
