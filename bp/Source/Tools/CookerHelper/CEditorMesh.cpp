//------------------------------------------------------------------------------------------
// CEditorMesh.cpp
// Bluepoint
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CEditorMesh.h"

//------------------------------------------------------------------------------------------

#include "Engine/Math/CloseEnough.h"
#include "CMeshUtils.h"

//------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------

CEditorMesh::Vertex::Vertex()
:  mValidFlags( 0 )
,  mPosition( CVector4::kConstructUninitialized )
,  mNormal( CVector3::kConstructUninitialized )
,  mUvSet0( CVector4::kConstructUninitialized )
,  mUvSet1( CVector4::kConstructUninitialized )
,  mUvSet2( CVector4::kConstructUninitialized )
,  mColor0( CVector4::kConstructUninitialized )
,  mColor1( CVector4::kConstructUninitialized )
,  mColor2( CVector4::kConstructUninitialized )
,  mWeights()
,  mJointsRemapped(false)
,  mOriginalIndex(-1)
{
}

//------------------------------------------------------------------------------------------
      
bool const CEditorMesh::Vertex::operator == ( Vertex const & lhs ) const
{
   if( !close_enough( mPosition, lhs.mPosition ) )
       return false;

   uint16 const sameFlags = mValidFlags & lhs.mValidFlags;
   
   if( (sameFlags & kFlag_Normal) && !close_enough( mNormal, lhs.mNormal ) )
      return false;

   if( (sameFlags & kFlag_UvSet0) && !close_enough( mUvSet0, lhs.mUvSet0 ) )
      return false;                                                     
   if( (sameFlags & kFlag_UvSet1) && !close_enough( mUvSet1, lhs.mUvSet1 ) )
      return false;                                                     
   if( (sameFlags & kFlag_UvSet2) && !close_enough( mUvSet2, lhs.mUvSet2 ) )
      return false;                                                     

   if( (sameFlags & kFlag_Color0) && ( mColor0 != lhs.mColor0 ) )
      return false;

   if( (sameFlags & kFlag_Color1) && ( mColor2 != lhs.mColor1 ) )
      return false;

   if( (sameFlags & kFlag_Color2) && ( mColor1 != lhs.mColor2 ) )
      return false;

   if( sameFlags & kFlag_Weights )
   {
      if( mWeights.size() != lhs.mWeights.size() )
         return false;

      for( uint32 i = 0; i < mWeights.size(); ++i )
      {
         if( mWeights[i].first != lhs.mWeights[i].first )
            return false;

         if( !close_enough( mWeights[i].second, lhs.mWeights[i].second ) )
            return false;
      }
   }
   // IMPORTANT: mOriginalIndex is intentionally not part of this comparison!

   return true;
}

//------------------------------------------------------------------------------------------

uint32 const CEditorMesh::Vertex::GetHashValue() const
{
   return mPosition.GetHashValue();
}

//------------------------------------------------------------------------------------------

void CEditorMesh::TakeOverData(std::vector< Triangle > & triangles,
                               std::vector< Line > & lines,
                               std::vector< Vertex > & vertices)
{
   // make sure our arrays are clean before we exchange them!
   mTriangles.swap(std::vector<Triangle>());
   mLines.swap(std::vector<Line>());
   mVertices.swap(std::vector<Vertex>());

   mTriangles.swap(triangles);
   mLines.swap(lines);
   mVertices.swap(vertices);
}

//------------------------------------------------------------------------------------------

void CEditorMesh::AddData(std::vector<Triangle> const & triangles,
                          std::vector<Line> const & lines,
                          std::vector<Vertex> const & vertices)
{
   int const baseVertexOffset = mVertices.size();

   mVertices.insert(mVertices.end(), vertices.begin(), vertices.end());

   mTriangles.reserve(mTriangles.size() + triangles.size());
   for( int i = 0; i < triangles.size(); ++i )
   {
      Triangle const & input = triangles[i];
      mTriangles.push_back(Triangle(input.mIndex1 + baseVertexOffset, 
                                    input.mIndex2 + baseVertexOffset,
                                    input.mIndex3 + baseVertexOffset));
   }

   mLines.reserve(mLines.size() + lines.size());
   for( int i = 0; i < lines.size(); ++i )
   {
      Line const & input = lines[i];
      mLines.push_back(Line(input.mIndex1 + baseVertexOffset, 
                            input.mIndex2 + baseVertexOffset));
   }
}

//------------------------------------------------------------------------------------------
   
void CEditorMesh::BuildJointRemapTable(std::vector<Triangle> const & triangles, std::vector<Vertex> const & vertices, std::vector<uint32> * pRemapTable)
{
   std::set<uint32> jointIndices;

   for( int tIdx = 0; tIdx < triangles.size(); ++tIdx )
   {
      Triangle const & triangle = triangles[tIdx];

      int const realVertexIndices[3] = { triangle.mIndex1, triangle.mIndex2, triangle.mIndex3 };

      for( int vIdx = 0; vIdx < 3; vIdx++ )
      {
         Vertex const & vertex = vertices[ realVertexIndices[vIdx] ];
         Vertex::TWeights const & weights = vertex.mWeights;

         for( int j = 0; j < weights.size(); ++j )
         {
            jointIndices.insert(weights[j].first);
         }
      }
   }

   if( !jointIndices.empty() )
   {
      *pRemapTable = std::vector<uint32>(jointIndices.begin(), jointIndices.end());
   }
}

//------------------------------------------------------------------------------------------

void CEditorMesh::RemapJoints(std::vector<uint32> const & remapTable, std::vector<Triangle> const & triangles, std::vector<Vertex> & vertices)
{
   if( !remapTable.empty() )
   {
      for( int tIdx = 0; tIdx < triangles.size(); ++tIdx )
      {
         Triangle const & triangle = triangles[tIdx];

         int const realVertexIndices[3] = { triangle.mIndex1, triangle.mIndex2, triangle.mIndex3 };

         for( int i = 0; i < 3; ++i )
         {
            Vertex & vertex = vertices[ realVertexIndices[i] ];
            
            // Only remap vertex a single time.
            if( !vertex.mJointsRemapped )
            {
               vertex.mJointsRemapped = true;
               
               Vertex::TWeights & weights = vertex.mWeights;
               for( int j = 0; j < weights.size(); ++j )
               {
                  Vertex::TWeight & weight = weights[j];
                  for( int k = 0; k < remapTable.size(); ++k)
                  {
                     if( weight.first == remapTable[k] )
                     {
                        weight.first = k;
                        break;
                     }
                  }
               }
            }
         }
      }
   }
}

//------------------------------------------------------------------------------------------

void CEditorMesh::BuildMeshes( std::list<CMeshInfo> const & meshes,
                               std::vector<CMeshChunk> & outChunks,
                               std::vector<uint32> & outPrimitiveIndices,
                               std::vector<uint32> & outTriListIndices,
                               CMergeContainer<CEditorMesh::Vertex> & outVertices,
                               int const verboseOutputLevel)
{
   int maxJointCount = -1;
   uint32 maxJointIndex = 0;

   foreach(CMeshInfo const & mesh, meshes)
   {
      CEditorMesh const & meshData = mesh.mMeshData;

      vector<Vertex> const & vertices = meshData.GetVertices();
      vector<Triangle> const & triangles = meshData.GetTriangles();
      vector<Line> const & lines = meshData.GetLines();
      
      outVertices.ResetFindInfo();

      // keep track of where the first vertex added was
      int const vertexBufferStartRange = outVertices.GetArray().size();
      int const triListIndexStartRange = outTriListIndices.size();
      int const primitiveIndexStartRange = outPrimitiveIndices.size();

      // bounds of all mesh data
      CAABox bounds = CAABox::MakeMaxInvertedBox();

      // triangle list indices
      std::vector<uint32> meshTriListIndices;

      // process triangles
      {
         // if the mesh is not skinned we don't care about the order of the triangles, 
         // we just push them through the same way they came in
         for( int i = 0; i < triangles.size(); ++i )
         {
            bounds.Include( vertices[ triangles[i].mIndex1 ].mPosition.DropW() );
            bounds.Include( vertices[ triangles[i].mIndex2 ].mPosition.DropW() );
            bounds.Include( vertices[ triangles[i].mIndex3 ].mPosition.DropW() );

            meshTriListIndices.push_back( static_cast<uint32>( outVertices.GetIndexForElement( vertices[ triangles[i].mIndex1 ] ) ) );
            meshTriListIndices.push_back( static_cast<uint32>( outVertices.GetIndexForElement( vertices[ triangles[i].mIndex2 ] ) ) );
            meshTriListIndices.push_back( static_cast<uint32>( outVertices.GetIndexForElement( vertices[ triangles[i].mIndex3 ] ) ) );
         }

         // keep track of where end of the vertices affected by this set of triangles are
         uint32 const vertexCount = outVertices.GetArray().size() - vertexBufferStartRange;

         CMeshChunk chunk(bounds, -1, CMeshChunk::kPrimitive_Invalid, vertexBufferStartRange, vertexCount, 0, 0, mesh.mMeshData.GetJointRemapTable(), mesh.mAdditionalFlags, mesh.mUnitIdx, mesh.mPacketIdx);

         maxJointCount = bpe::max_val(maxJointCount, (int)chunk.mJointMap.size());
         for ( int i = 0; i < chunk.mJointMap.size(); ++i )
         {
            maxJointIndex = bpe::max_val( maxJointIndex, chunk.mJointMap[i] );
         }

         CMeshUtils::ProcessTriangleList( meshTriListIndices, 
                                          outVertices.GetArray(),
                                          vertexBufferStartRange,
                                          vertexCount, 
                                          &outPrimitiveIndices, 
                                          &chunk.mPrimitiveType,
                                          &chunk.mIndexBufferOffset,
                                          &chunk.mIndicesCount,
                                          verboseOutputLevel);
         
         // Store additional data about the vertex colors used on this model
         {
            int minR = 255, maxR = 0;
            int minG = 255, maxG = 0;
            int minB = 255, maxB = 0;
            int minA = 255, maxA = 0;

            std::vector<CEditorMesh::Vertex> const & verts = outVertices.GetArray();

            bool hadAtLeastOneValidVertexColor = false;
            for( int vIdx = 0; vIdx != vertexCount; ++vIdx )
            {
               CEditorMesh::Vertex const & vert = verts[vertexBufferStartRange + vIdx];
               if( vert.mValidFlags & CEditorMesh::Vertex::kFlag_Color0 )
               {
                  hadAtLeastOneValidVertexColor = true;

                  int const r = int(vert.mColor0.GetX() * 128.0f + 0.5f);
                  int const g = int(vert.mColor0.GetY() * 128.0f + 0.5f);
                  int const b = int(vert.mColor0.GetZ() * 128.0f + 0.5f);
                  int const a = int(vert.mColor0.GetW() * 128.0f + 0.5f);
                  
                  minR = bpe::min_val(minR, r);
                  minG = bpe::min_val(minG, g);
                  minB = bpe::min_val(minB, b);
                  minA = bpe::min_val(minA, a);

                  maxR = bpe::max_val(maxR, r);
                  maxG = bpe::max_val(maxG, g);
                  maxB = bpe::max_val(maxB, b);
                  maxA = bpe::max_val(maxA, a);
               }
            }

            if( !hadAtLeastOneValidVertexColor )
            {
               // Set as 1.0 on PS2
               minR = 128;
               minG = 128;
               minB = 128;
               minA = 128;

               maxR = 128;
               maxG = 128;
               maxB = 128;
               maxA = 128;
            }

            minR = bpe::min_val(bpe::max_val(minR, 0), 255);
            minG = bpe::min_val(bpe::max_val(minG, 0), 255);
            minB = bpe::min_val(bpe::max_val(minB, 0), 255);
            minA = bpe::min_val(bpe::max_val(minA, 0), 255);

            maxR = bpe::min_val(bpe::max_val(maxR, 0), 255);
            maxG = bpe::min_val(bpe::max_val(maxG, 0), 255);
            maxB = bpe::min_val(bpe::max_val(maxB, 0), 255);
            maxA = bpe::min_val(bpe::max_val(maxA, 0), 255);

            chunk.mVertColorMin = minR << 24 | minG << 16 | minB << 8 | minA;
            chunk.mVertColorMax = maxR << 24 | maxG << 16 | maxB << 8 | maxA;
         }

         if( chunk.mPrimitiveType != CMeshChunk::kPrimitive_Invalid )
         {
            outChunks.push_back(chunk);
         }

         // push triangles indices into global triangle lists
         outTriListIndices.insert( outTriListIndices.end(), meshTriListIndices.begin(), meshTriListIndices.end() );
      }

#ifdef DO_VERTEX_CACHE_OPTIMIZATION
      uint32 const vertexBufferEndRange = outVertices.GetArray().size();

      uint32 const meshVertexCount = vertexBufferEndRange - vertexBufferStartRange;

      // keep track of where end of the indices created from this mesh chunk are
      int const triListIndexEndRange = outTriListIndices.size();
      int const primitiveIndexEndRange = outPrimitiveIndices.size();

      // remap indices/vertex data for this mesh chunk to be optimized for the pre-transform vertex cache
      // this array converts x -> y as such: y = remap[x];
      // x == vertex index from primitive index list
      // y == new vertex index
      std::vector<int32> mapOldToNew(meshVertexCount, -1);
      std::vector<int32> mapNewToOld(meshVertexCount, -1);
      
      int newIndex = vertexBufferStartRange;
      
      // we go over all the indices of the final output triangle list
      for( int i = primitiveIndexStartRange; i < primitiveIndexEndRange; ++i )
      {
         int relIndex = outPrimitiveIndices[i] - vertexBufferStartRange;
         
         // check to see if this is a new unique index we have not remapped before
         if( mapOldToNew[relIndex] == -1 )
         {
            // this becomes our current vertex index, and we increment one
            mapOldToNew[relIndex] = newIndex;
            mapNewToOld[newIndex - vertexBufferStartRange] = outPrimitiveIndices[i];

            ++newIndex;
         }
      }

      // remap mesh primitive indices
      for( int i = primitiveIndexStartRange; i < primitiveIndexEndRange; ++i )
      {
         outPrimitiveIndices[i] = mapOldToNew[outPrimitiveIndices[i] - vertexBufferStartRange];
      }

      // remap mesh triangle list indices
      for( int i = triListIndexStartRange; i < triListIndexEndRange; ++i )
      {
         outTriListIndices[i] = mapOldToNew[outTriListIndices[i] - vertexBufferStartRange];
      }

      // remap vertices according to their new order
      std::vector<CEditorMesh::Vertex> copiedVertices( outVertices.GetArray().begin() + vertexBufferStartRange, outVertices.GetArray().begin() + vertexBufferEndRange );
      std::vector<CEditorMesh::Vertex> & remappedVertices = const_cast<std::vector<CEditorMesh::Vertex> &>( outVertices.GetArray() );
      for( int i = vertexBufferStartRange; i < vertexBufferEndRange; ++i )
      {
         int oldIndex = mapNewToOld[i - vertexBufferStartRange];
         remappedVertices[i] = copiedVertices[oldIndex - vertexBufferStartRange];
      }
#endif
   }

   if( /*verboseOutputLevel &&*/ maxJointCount > 0 )
   {
      bpe_console_printf("Max joint count, index for mesh: %d, %d\n", maxJointCount, maxJointIndex);
   }
}