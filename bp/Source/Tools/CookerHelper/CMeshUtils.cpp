//----------------------------------------------------------------------------
// CMeshUtils.cpp
// Bluepoint
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CMeshUtils.h"

//----------------------------------------------------------------------------

//#include "iostream"

#include "Engine/System/CStopWatch.h"

#include "Engine/Math/CAngle.h"
#include "Engine/Math/CloseEnough.h"
#include "Engine/Math/CHalfFloat.h"

#include "Engine/Memory/CMemoryAllocator.h"
#include "Renderer/Base/Primitive/CVertexData.h"

//----------------------------------------------------------------------------

//#define USE_PACKED_NORMAL
//#define USE_HALF_NORMALS
#define USE_HALF_TEXCOORDS

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

void CMeshUtils::TransformArray( CMatrix34 const & transform, std::vector<CVector3> & points )
{
   for( int i = 0; i < points.size(); ++i )
   {
      points[i] = transform * points[i];
   }
}

//----------------------------------------------------------------------------

void CMeshUtils::TransformArray( CMatrix3 const & transform, std::vector<CVector3> & points )
{
   for( int i = 0; i < points.size(); ++i )
   {
      points[i] = transform * points[i];
   }
}

//----------------------------------------------------------------------------

namespace
{
   template<class T> void* ConvertToPlainMemoryAllocation(std::vector<T> const & input)
   {
      uint32 const size = sizeof(T) * input.size();
      void* pMemory = BPE_MALLOC_ALIGNED(16, size);
      memcpy(pMemory, &input.front(), size);

      return pMemory;
   }

   template<class T> void AddStreamFromVector(uint32 const id, EVertexDataType const dataType, std::vector<T> const & input, CVertexArray * pVertexArray )
   {
      if( !input.empty() )
      {
         void* pData = ConvertToPlainMemoryAllocation(input);
         pVertexArray->AddStream(id, dataType, CVertexArray::kFlags_ManagedMemory, pData, sizeof(T) * input.size());
      }
   }

   static uint32 PackNormalComponent(float const value, uint32 const size,uint32 const shift)
   {
      float const scale = (float)((1 << (size-1)) - 1);
      return (uint32(value * scale) & ((1 << size)-1)) << shift;
   }

   static uint32 PackNormal_11_11_10(CVector3 const & v)
   {
      return PackNormalComponent(v.GetX(),11,0) | PackNormalComponent(v.GetY(),11,11) | PackNormalComponent(v.GetZ(),10,22);
   }

   void Convert_Vector3_to_PackedNormal(std::vector<CVector3> const & input, std::vector<uint32> & output)
   {
      output.reserve(input.size());

      for( int i = 0; i < input.size(); ++i )
      {
         CVector3 const value = input[i];
         output.push_back(PackNormal_11_11_10(value));
      }
   }

   static inline int16 pack_float_to_signed_int16( float const in )
   {
      float normalizedIn = min( 1.0f, max( 0.0f, ( in + 1.0f ) / 2.0f ) );

      int ranged = int( normalizedIn * 65535 );
      return short( ranged - 32768 );
   }

   void Convert_Vector3_to_S16N( std::vector< CVector3 > const &input, std::vector<int16> &output )
   {
      for ( int i = 0; i < input.size(); ++i )
      {
         output.push_back( pack_float_to_signed_int16( input[i].GetX() ) );
         output.push_back( pack_float_to_signed_int16( input[i].GetY() ) );
         output.push_back( pack_float_to_signed_int16( input[i].GetZ() ) );
         output.push_back( 0 );
      }
   }

   void Convert_Vector2_to_Half2(std::vector<CVector2> const & input, std::vector<uint16> & output)
   {
      output.reserve(input.size() * 2);

      for( int i = 0; i < input.size(); ++i )
      {
         CVector2 const value = input[i];
         output.push_back(real16::ConvertFromR32(value.GetX()));
         output.push_back(real16::ConvertFromR32(value.GetY()));
      }
   }

   void Convert_Vector4_XY_to_Half2(std::vector<CVector4> const & input, std::vector<uint16> & output)
   {
      output.reserve(input.size() * 2);

      for( int i = 0; i < input.size(); ++i )
      {
         CVector4 const value = input[i];
         output.push_back(real16::ConvertFromR32(value.GetX()));
         output.push_back(real16::ConvertFromR32(value.GetY()));
      }
   }

   void Convert_Vector4_to_Half4(std::vector<CVector4> const & input, std::vector<uint16> & output)
   {
      output.reserve(input.size() * 4);
      
      for( int i = 0; i < input.size(); ++i )
      {
         CVector4 const value = input[i];
         output.push_back(real16::ConvertFromR32(value.GetX()));
         output.push_back(real16::ConvertFromR32(value.GetY()));
         output.push_back(real16::ConvertFromR32(value.GetZ()));
         output.push_back(real16::ConvertFromR32(value.GetW()));
      }
   }
}

CVertexArray * CMeshUtils::BuildVertexArray( std::vector<CEditorMesh::Vertex> const & vertices,
                                             std::vector<uint32> const & inTriangleListIndices,
                                             bool const buildTextureBasisVectors,
                                             int const verboseOutputLevel)
{
   std::vector<CVector4> positions;
   std::vector<CVector3> normals;
   std::vector<CVector4> color0;
   std::vector<CVector4> color1;
   std::vector<CVector4> color2;
   std::vector<CVector4> uv0;
   std::vector<CVector4> uv1;
   std::vector<CVector4> uv2;
   std::vector<uint32> boneIndices;
   std::vector<CVector4> boneWeights;

   std::vector<uint32> originalIndices;

   bool hasNormals = false;

   bool hasColors0 = false;
   bool hasColors1 = false;
   bool hasColors2 = false;

   bool hasUvs1 = false;
   bool hasUvs2 = false;
   bool hasUvs3 = false;

   bool hasBones = false;

   for( uint32 i = 0; i < vertices.size(); ++i )
   {
      CEditorMesh::Vertex const & vertex = vertices[i];
      
      if( vertex.mValidFlags & CEditorMesh::Vertex::kFlag_Normal )
         hasNormals = true;

      if( vertex.mValidFlags & CEditorMesh::Vertex::kFlag_Color0 )
         hasColors0 = true;
      if( vertex.mValidFlags & CEditorMesh::Vertex::kFlag_Color1 )
         hasColors1 = true;
      if( vertex.mValidFlags & CEditorMesh::Vertex::kFlag_Color2 )
         hasColors2 = true;
      
      if( vertex.mValidFlags & CEditorMesh::Vertex::kFlag_UvSet0 )
         hasUvs1 = true;
      if( vertex.mValidFlags & CEditorMesh::Vertex::kFlag_UvSet1 )
         hasUvs2 = true;
      if( vertex.mValidFlags & CEditorMesh::Vertex::kFlag_UvSet2 )
         hasUvs3 = true;
      
      if( vertex.mValidFlags & CEditorMesh::Vertex::kFlag_Weights )
         hasBones = true;
   }

   for( uint32 i = 0; i < vertices.size(); ++i )
   {
      CEditorMesh::Vertex const & vertex = vertices[i];
   
      if( hasBones )
      {
         CEditorMesh::Vertex::TWeights const & weights = vertex.mWeights;

         uint8    indices[4] = { 0, 0, 0, 0 };
         CVector4 weightVector = CVector4::Zero();

         for( int i = 0; i < weights.size(); ++i )
         {
            CEditorMesh::Vertex::TWeight const & weight = weights[i];

            if( close_enough( weight.second, 0.0f ) )
               continue;

            if( i < 4 )
            {
               indices[i] = static_cast<uint8>(weight.first);
               weightVector[i] = weight.second;
            }
         }

         // weird swizzling for color to ubyte4 conversion
         boneIndices.push_back((uint32)indices[3] << 24 | (uint32)indices[2] << 16 | (uint32)indices[1] << 8 | (uint32)indices[0]);

         boneWeights.push_back(weightVector);
      }

      positions.push_back( vertex.mPosition );
      
      if( hasNormals )
         normals.push_back( vertex.mNormal );

      if( hasColors0 )
         color0.push_back( vertex.mColor0 );

      if( hasColors1 )
         color1.push_back( vertex.mColor1 );

      if( hasColors2 )
         color2.push_back( vertex.mColor2 );

      if( hasUvs1 )
         uv0.push_back( vertex.mUvSet0 );

      if( hasUvs2 )
         uv1.push_back( vertex.mUvSet1 );

      if( hasUvs3 )
         uv2.push_back( vertex.mUvSet2 );

      originalIndices.push_back(vertex.mOriginalIndex);
   }

   CVertexArray* pVertexArray = CVertexArray::CreateVertexArray(11);
   AddStreamFromVector('POS0', kVDT_Float4, positions, pVertexArray);
   
   if( hasNormals ) 
   {
#ifdef USE_PACKED_NORMAL
      std::vector<uint32> packedNormals;
      Convert_Vector3_to_PackedNormal(normals, packedNormals);
      AddStreamFromVector('NRM0', kVDT_Packed3N, packedNormals, pVertexArray);
#elif USE_HALF_NORMALS
      std::vector<int16> packedNormals;
      Convert_Vector3_to_S16N( normals, packedNormals );
      AddStreamFromVector('NRM0', kVDT_Short4N, packedNormals );
#else
      AddStreamFromVector('NRM0', kVDT_Float3, normals, pVertexArray);
#endif
   }
   
   if( hasColors0 )
   {
      std::vector<uint16> colorHalfs;
      Convert_Vector4_to_Half4(color0, colorHalfs);
      AddStreamFromVector('COL0', kVDT_Half4, colorHalfs, pVertexArray);
   }

   if( hasColors1 )
   {
      std::vector<uint16> colorHalfs;
      Convert_Vector4_to_Half4(color1, colorHalfs);
      AddStreamFromVector('COL1', kVDT_Half4, colorHalfs, pVertexArray);
   }

   if( hasColors2 )
   {
      std::vector<uint16> colorHalfs;
      Convert_Vector4_to_Half4(color2, colorHalfs);
      AddStreamFromVector('COL2', kVDT_Half4, colorHalfs, pVertexArray);
   }
   
   if( hasUvs1 )
   {
#ifdef USE_HALF_TEXCOORDS
      std::vector<uint16> uvHalfs;
      Convert_Vector4_XY_to_Half2(uv0, uvHalfs);
      AddStreamFromVector('TEX0', kVDT_Half2, uvHalfs, pVertexArray);
#else
      AddStreamFromVector('TEX0', kVDT_Float2, uv0, pVertexArray);
#endif
   }

   if( hasUvs2 )
   {
#ifdef USE_HALF_TEXCOORDS
      std::vector<uint16> uvHalfs;
      Convert_Vector4_XY_to_Half2(uv1, uvHalfs);
      AddStreamFromVector('TEX1', kVDT_Half2, uvHalfs, pVertexArray);
#else
      AddStreamFromVector('TEX1', kVDT_Float2, uv1, pVertexArray);
#endif
   }

   if( hasUvs3 )
   {
#ifdef USE_HALF_TEXCOORDS
      std::vector<uint16> uvHalfs;
      Convert_Vector4_to_Half4(uv2, uvHalfs);
      AddStreamFromVector('TEX2', kVDT_Half4, uvHalfs, pVertexArray);
#else
      AddStreamFromVector('TEX2', kVDT_Float2, uv2, pVertexArray);
#endif
   }

   if( hasBones )
   {
      AddStreamFromVector('BONI', kVDT_UByte4, boneIndices, pVertexArray);

      std::vector<uint16> boneHalfs;
      Convert_Vector4_to_Half4(boneWeights, boneHalfs);
      AddStreamFromVector('BONW', kVDT_Half4, boneHalfs, pVertexArray);
   }

   // Add original index array
   {
      AddStreamFromVector('OIDX', kVDT_Int32, originalIndices, pVertexArray);
   }

   return pVertexArray;
}

//----------------------------------------------------------------------------

real32 const fDistMax = 1e-6f;

struct SVertexPred
{
   bool const operator () (CVector3 const & lhs, CVector3 const & rhs) const
   {
      return lhs.mX < rhs.mX;
   }
};

void CMeshUtils::BuildTextureBasisVectors( std::vector<uint32> const & inTriangleListIndices, 
                                           std::vector<CVector3> const & inPositions,
                                           std::vector<CVector3> const & inNormals,
                                           std::vector<CVector2> const & inUvs,
                                           std::vector<CVector3> & outS, 
                                           std::vector<CVector3> & outT )
{
   // build texture basis
   {
      real32 const SMALL_FLOAT = 1e-12f;

      outS = vector<CVector3>( inPositions.size(), CVector3( 0, 0, 0 ) );
      outT = vector<CVector3>( inPositions.size(), CVector3( 0, 0, 0 ) );

      for( uint32 index = 0; index < inTriangleListIndices.size(); index += 3 )
      {
         int const vertexIndex0 = inTriangleListIndices[ index + 0 ];
         int const vertexIndex1 = inTriangleListIndices[ index + 1 ];
         int const vertexIndex2 = inTriangleListIndices[ index + 2 ];

         real32 const ds1 = inUvs[vertexIndex1].GetX() - inUvs[vertexIndex0].GetX();
         real32 const dt1 = inUvs[vertexIndex1].GetY() - inUvs[vertexIndex0].GetY();

         real32 const ds2 = inUvs[vertexIndex2].GetX() - inUvs[vertexIndex0].GetX();
         real32 const dt2 = inUvs[vertexIndex2].GetY() - inUvs[vertexIndex0].GetY();

         // x, s, t
         {
            CVector3 const edge01( inPositions[vertexIndex1].GetX() - inPositions[vertexIndex0].GetX(), ds1, dt1 );
            CVector3 const edge02( inPositions[vertexIndex2].GetX() - inPositions[vertexIndex0].GetX(), ds2, dt2 );

            CVector3 const cp = CVector3::Cross( edge01, edge02 );

            if( fabs( cp.GetX() ) > SMALL_FLOAT )
            {
               real32 const dsdx = -cp.GetY() / cp.GetX();
               real32 const dtdx = -cp.GetZ() / cp.GetX();

               outS[vertexIndex0][0] += dsdx;
               outS[vertexIndex1][0] += dsdx;
               outS[vertexIndex2][0] += dsdx;
               outT[vertexIndex0][0] += dtdx;
               outT[vertexIndex1][0] += dtdx;
               outT[vertexIndex2][0] += dtdx;
            }
         }

         // y, s, t
         {
            CVector3 const edge01( inPositions[vertexIndex1].GetY() - inPositions[vertexIndex0].GetY(), ds1, dt1 );
            CVector3 const edge02( inPositions[vertexIndex2].GetY() - inPositions[vertexIndex0].GetY(), ds2, dt2 );

            CVector3 const cp = CVector3::Cross( edge01, edge02 );
            if( fabs( cp.GetX() ) > SMALL_FLOAT )
            {
               real32 const dsdx = -cp.GetY() / cp.GetX();
               real32 const dtdx = -cp.GetZ() / cp.GetX();

               outS[vertexIndex0][1] += dsdx;
               outS[vertexIndex1][1] += dsdx;
               outS[vertexIndex2][1] += dsdx;
               outT[vertexIndex0][1] += dtdx;
               outT[vertexIndex1][1] += dtdx;
               outT[vertexIndex2][1] += dtdx;
            }
         }

         // z, s, t
         {
            CVector3 const edge01( inPositions[vertexIndex1].GetZ() - inPositions[vertexIndex0].GetZ(), ds1, dt1 );
            CVector3 const edge02( inPositions[vertexIndex2].GetZ() - inPositions[vertexIndex0].GetZ(), ds2, dt2 );

            CVector3 const cp = CVector3::Cross( edge01, edge02 );
            if( fabs( cp.GetX() ) > SMALL_FLOAT )
            {
               real32 const dsdx = -cp.GetY() / cp.GetX();
               real32 const dtdx = -cp.GetZ() / cp.GetX();

               outS[vertexIndex0][2] += dsdx;
               outS[vertexIndex1][2] += dsdx;
               outS[vertexIndex2][2] += dsdx;
               outT[vertexIndex0][2] += dtdx;
               outT[vertexIndex1][2] += dtdx;
               outT[vertexIndex2][2] += dtdx;
            }
         }
      }

      typedef std::multimap<CVector3, int, SVertexPred> TVertexMap;
      TVertexMap vertexMap;
      for( int i = 0; i < inPositions.size(); ++i )
      {
         vertexMap.insert( make_pair(inPositions[i], i) ); 
      }

      // Parameters that define if verts are shared.
      real32 const fNormAngMin = 0.939693f;  // 20 degrees
      real32 const fTanAngMin = cosf( CAngle::FromDegrees( 89.0f ).AsRadians() );

      bool* bSharingRejected = new bool[ inPositions.size() ];

      memset( bSharingRejected, 0, sizeof(bool) * inPositions.size() );

      // Any verts that share position and normal, but don't share texture
      // co-ordinates should be treated as shared.
      for( uint32 i = 0; i < inPositions.size(); ++i )
      {
         if( bSharingRejected[i] )
            continue;

         CVector3 vS = outS[i];
         CVector3 vT = outT[i];

         std::vector< int > sharedIndices;
         {
            pair<TVertexMap::const_iterator, TVertexMap::const_iterator> const range = vertexMap.equal_range( inPositions[i] );

            for( TVertexMap::const_iterator it = range.first; it != range.second; ++it )
            {
               uint32 const j = it->second;
               if( j == i )
                  continue;

               // ignore points that are too far away
               if( CVector3::Distance( inPositions[i], inPositions[j] ) > fDistMax )
                  continue;

               real32 const fCosA = CVector3::Dot( inNormals[i], inNormals[j] );

               // Less than 20 degrees.
               if( fCosA > fNormAngMin )
               {
                  sharedIndices.push_back( j );
               }
            }
         }

         // Average S and T vectors for i and j.
         for( uint32 k = 0; k < sharedIndices.size(); ++k )
         {
            vS += outS[ sharedIndices[k] ];
            vT += outT[ sharedIndices[k] ];
         }


         // Do a check of the existing basis vectors vs. the newly calculated 
         // ones to make sure we really should average the textures spaces.
         // For example at the poles of a sphere we do not want to average the
         // texture spaces.
         bool bUseAverage = true;

         CVector3 vSavg = vS;
         vSavg.TryNormalize();
         
         CVector3 vTavg = vT;
         vTavg.TryNormalize();

         CVector3 vSi = outS[i];
         vSi.TryNormalize();

         CVector3 vTi = outT[i];
         vTi.TryNormalize();

         real32 const fDotS = CVector3::Dot( vSi, vSavg );
         real32 const fDotT = CVector3::Dot( vTi, vTavg );

         if( fDotS <= fTanAngMin || fDotT <= fTanAngMin )
         {
            bUseAverage = false;
         }

         for( uint32 k = 0; k < sharedIndices.size(); ++k )
         {
            CVector3 vSj = outS[sharedIndices[k]];
            vSj.TryNormalize();
            CVector3 vTj = outT[sharedIndices[k]];
            vTj.TryNormalize();

            real32 const fDotS = CVector3::Dot( vSj, vSavg );
            real32 const fDotT = CVector3::Dot( vTj, vTavg );

            if( fDotS <= fTanAngMin || fDotT <= fTanAngMin )
            {
               bUseAverage = false;
               break;
            }
         }

         if( bUseAverage )
         {
            // Set the basis vectors for all the verts to the average.
            for( uint32 k = 0; k < sharedIndices.size(); ++k )
            {
               outS[sharedIndices[k]] = vSavg;
               outT[sharedIndices[k]] = vTavg;
            }

            outS[i] = vS;
            outT[i] = vT;
         }

         // Prevent the vertices from being considered again.
         for( uint32 k = 0; k < sharedIndices.size(); ++k )
         {
            bSharingRejected[sharedIndices[k]] = true;
         }
      }

      delete[] bSharingRejected;
   }

   for( uint32 i = 0; i < inPositions.size(); ++i )
   {
      outS[i].TryNormalize();
      outT[i] = -outT[i];
      outT[i].TryNormalize();
   }
}

//----------------------------------------------------------------------------

// Huge thanks to Tom Forsyth for sharing this:
// http://home.comcast.net/~tom_forsyth/papers/fast_vert_cache_opt.html

const real32 kFindVertexScore_CacheDecayPower = 1.5f;
const real32 kFindVertexScore_LastTriScore = 0.75f;
const real32 kFindVertexScore_ValenceBoostScale = 2.0f;
const real32 kFindVertexScore_ValenceBoostPower = 0.5f;
const int    kMaxSizeVertexCache = 32;

struct vcache_vertex_data
{
   vcache_vertex_data()
   :  mCacheTag(-1)
   ,  mCurrentScore(0)
   ,  mNumActiveTris(0)
   {
   }

   void AddTri(int ti)
   {
      ++mNumActiveTris;
      mTriIndices.push_back(ti);
   }

   int                  mCacheTag;	      // position in the modeled cache (-1 if it is not in the cache)
   real32               mCurrentScore;
   int                  mNumActiveTris;   // number of triangles not yet added that use it
   std::vector<uint32>  mTriIndices;		// TotalTriangles worth of indices
};

struct vcache_triangle
{
   int      mWasAdded;			// nonzero if already added
   real32   mTriangleScore;	// score; sum of the scores of the three vertices
   uint32   mIdx[3];				// vertex indices of the triangle
};

real32 FindVertexScore ( vcache_vertex_data *pVertexData )
{
   if( !pVertexData->mNumActiveTris )
   {
      // No tri needs this vertex!
      return -1.0f;
   }

   real32 score = 0.0f;
   int cachePosition = pVertexData->mCacheTag;
   
   if ( cachePosition < 0 )
   {
      // Vertex is not in FIFO cache - no score.
   }
   else
   {
      if ( cachePosition < 3 )
      {
         // This vertex was used in the last triangle,
         // so it has a fixed score, whichever of the three
         // it's in. Otherwise, you can get very different
         // answers depending on whether you add
         // the triangle 1,2,3 or 3,1,2 - which is silly.
         score = kFindVertexScore_LastTriScore;
      }
      else
      {
         BPE_ASSERT( cachePosition < kMaxSizeVertexCache, "bad" );
         // Points for being high in the cache.
         const real32 scaler = 1.0f / ( kMaxSizeVertexCache - 3 );
         score = 1.0f - ( cachePosition - 3 ) * scaler;
         score = powf ( score, kFindVertexScore_CacheDecayPower );
      }
   }

   // Bonus points for having a low number of tris still to
   // use the vert, so we get rid of lone verts quickly.
   real32 valenceBoost = powf ( real32(pVertexData->mNumActiveTris), -kFindVertexScore_ValenceBoostPower );
   score += kFindVertexScore_ValenceBoostScale * valenceBoost;

   return score;
}

// This algorithm works by keeping track of some vertex specific information (in vdata) during evaluation.
// For this the indices need to be converted into local range on the way in, and back into vertex array space on the way out (using vertexBufferOffset)

void CacheOptimize( int const vertexBufferOffset, int const vertexCount, std::vector<uint32> const & triangleIndices, std::vector<uint32> & outTriangleIndices )
{
   // init the vertex array
   std::vector<vcache_vertex_data> vdata(vertexCount);

   int const triangleCount = triangleIndices.size() / 3;

   // init the tri array and track which triangles are used by which vertices
   std::vector<vcache_triangle> tdata;
   tdata.reserve(triangleCount);

   for( int i = 0; i < triangleCount; ++i)
   {
      vcache_triangle tri;

      tri.mWasAdded = false;
      tri.mTriangleScore = 0;

      for (int j=0; j<3; j++)
      {
         int index = triangleIndices[i * 3 + j] - vertexBufferOffset;
         tri.mIdx[j] = index;
         vdata[index].AddTri(i);
      }

      tdata.push_back(tri);
   }

   // compute the vertex scores
   for (int i=0; i<vdata.size(); i++)
   {
      vdata[i].mCurrentScore = FindVertexScore(&vdata[i]);
   }

   // compute the initial triangle scores
   for (int i=0; i<tdata.size(); i++)
   {
      tdata[i].mTriangleScore = vdata[tdata[i].mIdx[0]].mCurrentScore + vdata[tdata[i].mIdx[1]].mCurrentScore + vdata[tdata[i].mIdx[2]].mCurrentScore;
   }

   // extra room so array never has to grow
   std::vector<uint32> vcache;
   vcache.resize(kMaxSizeVertexCache + 3);

   int bestTri = -1;
   real32 bestScore = -2.0f;
   
   // iterate over all triangles
   for (int i = 0; i < tdata.size(); i++ )
   {
      // Rescan all suitable remaining triangles if necessary
      if (bestScore < 0.1f)
      {
         for (int j = 0; j < tdata.size(); j++)
         {
            if (tdata[j].mWasAdded)
               continue;

            if (bestScore < tdata[j].mTriangleScore) 
            {
               bestTri = j;
               bestScore = tdata[j].mTriangleScore;
            }
         }
      }

      BPE_VERIFY(bestTri != -1, false, "bad");

      // add the triangle to the draw list
      vcache_triangle &bt = tdata[bestTri];
      bt.mWasAdded = true;
      bt.mTriangleScore = -1.0f;
      for (int k=0; k<3; k++) 
      {
         int vidx = bt.mIdx[k];
         vcache_vertex_data &v = vdata[vidx];
      
         // Reduce valence of the vertex
         v.mNumActiveTris--;
         BPE_VERIFY(v.mNumActiveTris >= 0, false, "bad");

         // Add the vertex index to the output trilist
         outTriangleIndices.push_back(vidx + vertexBufferOffset);

         // If it was already in the cache, delete it
         if (v.mCacheTag != -1)
         {
            vcache.erase(vcache.begin() + v.mCacheTag);
         }

         // Reinsert vertex at head of cache
         vcache.insert(vcache.begin(), vidx);
      }

      // Revalidate all cache tag entries
      for (int m=0; m<vcache.size(); m++)
      {
         vdata[vcache[m]].mCacheTag = m;
      }

      // update all vertices in the cache
      for( int k=0; k<vcache.size(); k++ )
      {
         int vidx = vcache[k];
         vcache_vertex_data &v = vdata[vidx];
         // update tag to reflect new position (which may no longer be in the cache!)
         v.mCacheTag = k < kMaxSizeVertexCache? k : -1;
         v.mCurrentScore = FindVertexScore(&v);
      }

      // reset best triangle
      bestTri = -1;
      bestScore = -2.0f;

      // update all triangle scores associated with vertices in the cache
      // this is a separate pass because we need all vertex scores stabilized first.
      for (int k=0; k<vcache.size(); k++) 
      {
         int vidx = vcache[k];
         vcache_vertex_data &v = vdata[vidx];
         for (int t=0; t<v.mTriIndices.size(); t++) 
         {
            vcache_triangle &tri = tdata[v.mTriIndices[t]];
            // ignore triangle if it is already gone (we set its score to an impossibly low number already when we pulled it)
            if (tri.mWasAdded)
               continue;
            // recompute triangle score
            tri.mTriangleScore = vdata[tri.mIdx[0]].mCurrentScore + vdata[tri.mIdx[1]].mCurrentScore + vdata[tri.mIdx[2]].mCurrentScore;
            // ..and see if it's the best one for next iteration
            if (bestScore < tri.mTriangleScore) 
            {
               bestTri = v.mTriIndices[t];
               bestScore = tri.mTriangleScore;
            }
         }
      }

      // remove excess vertices from the cache
      // any excess have already had their bookkeeping properly performed.
      while (vcache.size() > kMaxSizeVertexCache)
         vcache.pop_back();
   }
}


void CMeshUtils::ProcessTriangleList( std::vector<uint32> const & triangleListIndices, 
                                      std::vector<CEditorMesh::Vertex> const & vertices,
                                      int const vertexBufferOffset,
                                      int const vertexCount,
                                      std::vector<uint32> * pOutIndices, 
                                      CMeshChunk::EPrimitive * pPrimitiveType,
                                      uint32 * pPrimitiveIndexBufferOffset,
                                      uint32 * pPrimitiveIndexCount,
                                      int const verboseOutputLevel )
{
   if( triangleListIndices.empty() )
   {
      *pPrimitiveType = CMeshChunk::kPrimitive_TriangleList;
      pPrimitiveIndexBufferOffset = 0;
      pPrimitiveIndexCount = 0;
      return;
   }

   int const outIndicesStartOffset = pOutIndices->size();

#ifdef DO_POST_TRANSFORM_CACHE_OPTIOMIZATION
   if (verboseOutputLevel)
   {
      bpe_console_printf("Optimizing for vertex cache...\n");
   }

   CacheOptimize(vertexBufferOffset, vertexCount, triangleListIndices, *pOutIndices);
#else
   // build triangle list
   for( uint32 currentIndex = 0; currentIndex < triangleListIndices.size(); ++currentIndex )
   {
      pOutIndices->push_back( triangleListIndices[ currentIndex ] );
   }
#endif

#pragma region Evaluate cache hit rate
   // evaluate vertex cache hit rate
   if (verboseOutputLevel)
   {
      int cacheHits = 0;
      int cacheMisses = 0;

      typedef bpe::reserved_vector<uint32, 32> TVertexCache;
      TVertexCache vertexCache;

      for( int i = outIndicesStartOffset; i < pOutIndices->size(); ++i )
      {
         int index = (*pOutIndices)[i];
         TVertexCache::iterator found = std::find(vertexCache.begin(), vertexCache.end(), index);
         
         if( found != vertexCache.end() )
         {
            ++cacheHits;
            vertexCache.erase(found);
         }
         else
         {
            ++cacheMisses;
         }

         if( vertexCache.size() == vertexCache.capacity() )
            vertexCache.erase(vertexCache.begin());

         vertexCache.push_back(index);
      }

      bpe_console_printf("Estimated post transform cache hits: %d, misses: %d\n", cacheHits, cacheMisses);
   }
#pragma endregion

   *pPrimitiveType = CMeshChunk::kPrimitive_TriangleList;
   *pPrimitiveIndexBufferOffset = outIndicesStartOffset;
   *pPrimitiveIndexCount = pOutIndices->size() - outIndicesStartOffset;
}

//----------------------------------------------------------------------------

void CMeshUtils::ProcessLineList( std::vector<uint32> const & lineListIndices, 
                                 std::vector<CEditorMesh::Vertex> const & vertices,
                                 int const vertexBufferOffset,
                                 int const vertexCount,
                                 std::vector<uint32> * pOutIndices, 
                                 CMeshChunk::EPrimitive * pPrimitiveType,
                                 uint32 * pPrimitiveIndexBufferOffset,
                                 uint32 * pPrimitiveIndexCount,
                                 int const verboseOutputLevel )
{
   if( lineListIndices.empty() )
      return;

   // build list list
   int const outIndicesStartOffset = pOutIndices->size();

   for( uint32 currentIndex = 0; currentIndex < lineListIndices.size(); ++currentIndex )
   {
      pOutIndices->push_back( lineListIndices[ currentIndex ] );
   }

   *pPrimitiveType = CMeshChunk::kPrimitive_LineList;
   *pPrimitiveIndexBufferOffset = outIndicesStartOffset;
   *pPrimitiveIndexCount = pOutIndices->size() - outIndicesStartOffset;
}
