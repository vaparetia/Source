//------------------------------------------------------------------------------------------
// CEditorArea.cpp
// Bluepoint
// Abstract
// Copyright 2003
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CEditorArea.h"

//------------------------------------------------------------------------------------------

#include "iostream"
#include "algorithm"

#include "Engine/Mechanics/CXMLTree.h"
#include "Engine/Primitives/RIntersection.h"

#include "CMeshUtils.h"

//------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------

CEditorArea::CEditorArea()
{
}

//------------------------------------------------------------------------------------------
   
CEditorArea::CEditorArea( TMeshVector const & meshes,
                          TLightVector const & lights )
:  mMeshes( meshes )
,  mLights( lights )
{
}

//------------------------------------------------------------------------------------------

CEditorArea::CEditorArea( CXMLTree const & tree )
{
   ParseTree( tree );
}

//------------------------------------------------------------------------------------------
   
void CEditorArea::PutTo( CXMLTree & tree )
{
   for( int i = 0; i < mLights.size(); ++i )
   {
      // write out light
      CXMLTree::TNodePtr const lightNode( new CXMLTree( "Light" ) );
      tree.AddChild( lightNode );

      CEditorLight const & light = mLights[i];
      light.PutTo( *lightNode );
   }

   for( int i = 0; i < mMeshes.size(); ++i )
   {
      // write out mesh data
      {
         CEditorMesh const & mesh = mMeshes[i].second;

         CXMLTree::TNodePtr const meshNode( new CXMLTree( "Mesh" ) );
         mesh.PutTo( *meshNode );

         tree.AddChild( meshNode );

         // write out shader
         {
            CXMLTree::TNodePtr const shaderNode( new CXMLTree( "Shader" ) );
            meshNode->AddChild( shaderNode );

            CEditorShader const & shader = mMeshes[i].first;
            shader.PutTo( *shaderNode );
         }
      }
   }
}

//------------------------------------------------------------------------------------------

class CChunkTriangleSorter
{
public:
   CChunkTriangleSorter( std::vector<CEditorArea::CChunkTriangle> const & triangles )
   :  mTriangles( triangles )
   {
   }

   bool const operator () ( int const lhs, int const rhs ) const
   {
      return mTriangles[lhs].mShaderIndex < mTriangles[rhs].mShaderIndex;
   }

   std::vector<CEditorArea::CChunkTriangle> const & mTriangles;
private:
   BPE_DISABLE_OBJECT_ASSIGN( CChunkTriangleSorter );
};

//------------------------------------------------------------------------------------------

void CEditorArea::ChunkMeshes( int const minTriangleCount,
                               real32 const size,
                               real32 const volumeThreshold )
{
   TMeshVector combinedMeshes;
   cout << "Combining meshes: ";
   ChunkMeshes( gkInt32Max, gkReal32Max, gkReal32Max, mMeshes, combinedMeshes );
   cout << "Done, before: " << mMeshes.size() << " after: " << combinedMeshes.size() << endl;

   cout << "Splitting meshes: ";

   TMeshVector chunkedMeshes;
   for( int i = 0; i < combinedMeshes.size(); ++i )
   {
      TMeshVector inMeshes;
      inMeshes.push_back( combinedMeshes[i] );
      ChunkMeshes( minTriangleCount, size, volumeThreshold, inMeshes, chunkedMeshes );
   }

   cout << "Done, before: " << combinedMeshes.size() << " after: " << chunkedMeshes.size() << endl;
   mMeshes = chunkedMeshes;
}

//------------------------------------------------------------------------------------------
   
void CEditorArea::CombineMeshes()
{
   TMeshVector combinedMeshes;
   cout << "Combining meshes: ";
   ChunkMeshes( gkInt32Max, gkReal32Max, gkReal32Max, mMeshes, combinedMeshes );
   cout << "Done, before: " << mMeshes.size() << " after: " << combinedMeshes.size() << endl;
   
   mMeshes = combinedMeshes;
}

//------------------------------------------------------------------------------------------

int const FindOrAddShader( CEditorShader const & shader, std::vector<CEditorShader> & shaders )
{
   for( int i = 0; i < shaders.size(); ++i )
   {
      if( shaders[i] == shader )
         return i;
   }

   shaders.push_back( shader );
   
   return shaders.size() - 1;
}

//------------------------------------------------------------------------------------------

void CEditorArea::ChunkMeshes( int const minTriangleCount,
                               real32 const size,
                               real32 const volumeThreshold,
                               TMeshVector const & inMeshes,
                               TMeshVector & outMeshes )
{
   std::vector<CEditorMesh::Vertex> globalVertices;
   std::vector<CChunkTriangle>      globalTriangles;
   std::vector<CVector3>            globalCentroids;
   std::vector<CEditorShader>       shaders;

   // 1) build list of triangles (contains 3 indices plus index to shader), at same time built list of shaders and vertices
   {
      for( int i = 0; i < inMeshes.size(); ++i )
      {
         CEditorMesh const & mesh = inMeshes[i].second;

         int const shaderIndex = FindOrAddShader( inMeshes[i].first, shaders );

         int const startVertexOffset = globalVertices.size();

         std::vector<CEditorMesh::Vertex> const & meshVertices = mesh.GetVertices();
         globalVertices.insert( globalVertices.end(), meshVertices.begin(), meshVertices.end() );
         
         std::vector<CEditorMesh::Triangle> const & meshTriangles = mesh.GetTriangles();

         for( int j = 0; j < meshTriangles.size(); ++j )
         {
            CEditorMesh::Triangle const & triangle = meshTriangles[j];
            
            int const index1 = startVertexOffset + triangle.mIndex1;
            int const index2 = startVertexOffset + triangle.mIndex2;
            int const index3 = startVertexOffset + triangle.mIndex3;
            
            globalTriangles.push_back( CChunkTriangle( shaderIndex, 
                                                       index1, 
                                                       index2, 
                                                       index3 ) );

            CVector3 const centroid( (globalVertices[index1].GetPosition() +
                                      globalVertices[index2].GetPosition() +
                                      globalVertices[index3].GetPosition()) / 3 );

            globalCentroids.push_back( centroid );
         }
      }
   }

   // 2) run through grid splitter which will output a bunch of sliced pieces made up out of lists of those triangles

   std::list< CChunk > pendingChunks;
   std::list< CChunk > processedChunks;
   
   std::vector< int > globalTriangleIndices;
   globalTriangleIndices.reserve( globalTriangles.size() );
   for( int i = 0; i < globalTriangles.size(); ++i )
      globalTriangleIndices.push_back( i );


   pendingChunks.push_back( CChunk( CChunk::GetBoundsFromTriangles( globalVertices, globalTriangles, globalTriangleIndices ),
                            globalTriangleIndices ) );
   for( ;; )
   {
      std::list< CChunk > tempChunks;

      bool didChunk = false;

      for( std::list< CChunk >::const_iterator chunkIt = pendingChunks.begin(); chunkIt != pendingChunks.end(); ++chunkIt )
      {
         CChunk const & chunk = *chunkIt;

         if( chunk.mTriangleIndices.size() > minTriangleCount )
         {
            int axis = -1;
            real32 axisLength = 0.0f;
         
            CVector3 const boundsSize = chunk.mBounds.GetExtents();
            
            if( boundsSize[X] > size && boundsSize[X] > axisLength )
            {
               axis = X;
               axisLength = boundsSize[X];
            }

            if( boundsSize[Y] > size && boundsSize[Y] > axisLength )
            {
               axis = Y;
               axisLength = boundsSize[Y];
            }
            
            if( boundsSize[Z] > size && boundsSize[Z] > axisLength )
            {
               axis = Z;
               axisLength = boundsSize[Z];
            }
            
            if( axis != -1 )
            {
               if( SplitChunk( chunk, 
                               globalVertices, 
                               globalTriangles, 
                               globalCentroids, 
                               axis, 
                               volumeThreshold, 
                               minTriangleCount, 
                               tempChunks ) )
               {
                  didChunk = true;
                  continue;
               }
            }

         }
         
         // fall through case, couldn't be split
         processedChunks.push_back( chunk );
      }

      if( !didChunk )
         break;

      pendingChunks = tempChunks;
   }

   // 3) for each piece, sort by shader index and then build a mesh for each of the pieces with unique shaders
   
   int const outMeshCountBefore = outMeshes.size();

   for( std::list<CChunk>::iterator chunkIt = processedChunks.begin(); chunkIt != processedChunks.end(); ++chunkIt )
   {
      CChunk & chunk = *chunkIt;
      sort( chunk.mTriangleIndices.begin(), chunk.mTriangleIndices.end(), CChunkTriangleSorter( globalTriangles ) );
      
      int lastShader = globalTriangles[ chunk.mTriangleIndices[0] ].mShaderIndex;
      
      vector< CEditorMesh::Triangle > newMeshTriangles;
      CMergeContainer< CEditorMesh::Vertex > newMeshVertices;

      for( int i = 0; i < chunk.mTriangleIndices.size(); ++i )
      {
         CChunkTriangle const & triangle = globalTriangles[ chunk.mTriangleIndices[i] ];
         
         if( triangle.mShaderIndex != lastShader )
         {
            CEditorMesh const newMesh( newMeshTriangles, newMeshVertices );
            outMeshes.push_back( TMesh( shaders[lastShader], newMesh ) );

            lastShader = triangle.mShaderIndex;
            newMeshTriangles.clear();
            newMeshVertices.Clear();
         }

         newMeshTriangles.push_back( CEditorMesh::Triangle( newMeshVertices.GetIndexForElement( globalVertices[ triangle.mVertex1 ] ),
                                                            newMeshVertices.GetIndexForElement( globalVertices[ triangle.mVertex2 ] ),
                                                            newMeshVertices.GetIndexForElement( globalVertices[ triangle.mVertex3 ] ) ) );
      }

      CEditorMesh const newMesh( newMeshTriangles, newMeshVertices );
      outMeshes.push_back( TMesh( shaders[lastShader], newMesh ) );

   }

   /*
   int const meshCountBefore = inMeshes.size();
   int const meshCountAfter = outMeshes.size() - outMeshCountBefore;

   if( meshCountBefore != meshCountAfter )
   {
      cout << "Split " << meshCountBefore << " meshes (" << globalTriangles.size() << " triangles) into " << meshCountAfter << " meshes" << endl;
   }
   */
}

//------------------------------------------------------------------------------------------
   
bool const CEditorArea::SplitChunk( CChunk const & chunk, 
                                    std::vector<CEditorMesh::Vertex> const & globalVertices,
                                    std::vector<CChunkTriangle> const & globalTriangles,
                                    std::vector<CVector3> const & globalCentroids,
                                    int const axis, 
                                    real32 const volumeThreshold, 
                                    int const minTriangles,
                                    std::list<CChunk> & outChunks )
{
   CVector3 const centerPoint = chunk.mBounds.GetCenter();

   std::vector< int > triangles1;
   std::vector< int > triangles2;

   triangles1.reserve( chunk.mTriangleIndices.size() );
   triangles2.reserve( chunk.mTriangleIndices.size() );

   for( int i = 0; i < chunk.mTriangleIndices.size(); ++i )
   {
      int const triangle = chunk.mTriangleIndices[i];

      if( globalCentroids[triangle][axis] <= centerPoint[axis] )
      {
         triangles1.push_back( triangle );
      }
      else
      {
         triangles2.push_back( triangle );
      }
   }

   CChunk const chunk1( CChunk::GetBoundsFromTriangles( globalVertices, globalTriangles, triangles1 ), triangles1 );
   CChunk const chunk2( CChunk::GetBoundsFromTriangles( globalVertices, globalTriangles, triangles2 ), triangles2 );
   
   real32 const volume1 = chunk1.mBounds.GetVolume() / chunk.mBounds.GetVolume();
   real32 const volume2 = chunk2.mBounds.GetVolume() / chunk.mBounds.GetVolume();

   if( volume1 > volumeThreshold &&
       volume2 > volumeThreshold ||
       triangles1.empty() ||
       triangles2.empty() ||
       triangles1.size() < minTriangles ||
       triangles2.size() < minTriangles )
   {
      //printf( "Didn't split chunk with %d triangles into chunk1 %d triangles and chunk2 %d triangles\n", chunk.mTriangleIndices.size(), triangles1.size(), triangles2.size() );
      return false;
   }

   //printf( "Split chunk with %d triangles into chunk1 %d triangles and chunk2 %d triangles\n", chunk.mTriangleIndices.size(), triangles1.size(), triangles2.size() );

   outChunks.push_back( chunk1 );
   outChunks.push_back( chunk2 );

   return true;
}

//------------------------------------------------------------------------------------------

CAABox const CEditorArea::CChunk::GetBoundsFromTriangles( std::vector< CEditorMesh::Vertex > const & vertices,
                                                           std::vector< CChunkTriangle > const & triangles,
                                                           std::vector< int > const & triangleIndices )
{
   CAABox bounds( CAABox::MakeMaxInvertexBox() );

   for( int i = 0; i < triangleIndices.size(); ++i )
   {
      bounds.Include( vertices[ triangles[ triangleIndices[i] ].mVertex1 ].GetPosition() );
      bounds.Include( vertices[ triangles[ triangleIndices[i] ].mVertex2 ].GetPosition() );
      bounds.Include( vertices[ triangles[ triangleIndices[i] ].mVertex3 ].GetPosition() );
   }

   return bounds;
}

//------------------------------------------------------------------------------------------
   
void CEditorArea::CreateMeshes( std::vector<CMesh> & outputMeshes )
{
   int const maxTriangleCount = 0xffff / 3;

   int triangleCount = 0;
   std::vector<uint32> primitiveIndices;
   CMergeContainer<CEditorMesh::Vertex> exportedVertices;
   std::vector<CMeshChunk> meshChunks;

   TMeshVector::const_iterator it = mMeshes.begin();

   while( it != mMeshes.end() )
   {
      while( it != mMeshes.end() )
      {
         TMesh const & mesh = *it;
         CEditorShader const & shaderData = mesh.first;
         CEditorMesh const & meshData = mesh.second;
         
         // check if we need to flush the current mesh
         if( triangleCount && ((triangleCount + meshData.GetTriangles().size()) > maxTriangleCount) )
         {
            break;
         }

         vector<CEditorMesh::Vertex> const & vertices = meshData.GetVertices();
         vector<CEditorMesh::Triangle> const & triangles = meshData.GetTriangles();
         
         // this will ensure that no vertices are shared with the last chunk
         exportedVertices.ResetFindInfo();

         int const vertexBufferStartRange = exportedVertices.GetArray().size();
         
         std::vector<uint32> triangleListIndices;

         for( uint32 i = 0; i < triangles.size(); ++i )
         {
            triangleListIndices.push_back( static_cast<uint32>( exportedVertices.GetIndexForElement( vertices[ triangles[i].mIndex1 ] ) ) );
            triangleListIndices.push_back( static_cast<uint32>( exportedVertices.GetIndexForElement( vertices[ triangles[i].mIndex2 ] ) ) );
            triangleListIndices.push_back( static_cast<uint32>( exportedVertices.GetIndexForElement( vertices[ triangles[i].mIndex3 ] ) ) );
         }

         int const vertexBufferEndRange = exportedVertices.GetArray().size();

         vector<CMeshPrimitive> primitives;

         uint32 const indicesBefore = primitiveIndices.size();

         CMeshUtils::ProcessTriangleList( triangleListIndices, 
                                          vertices,
                                          vertexBufferStartRange,
                                          vertexBufferEndRange,
                                          primitiveIndices,
                                          primitives );

         meshChunks.push_back( CMeshChunk( shaderData.GetShaderInfo(), 
                                           primitives,
                                           vector<CBoneId>() ) );
         
         uint32 const indicesCount = (primitiveIndices.size() - indicesBefore);
         real32 const stripEfficiency = static_cast<real32>(triangles.size() * 3) / indicesCount;
         //cout << "Chunk, triangles: " << triangles.size() << ", indices: " <<  indicesCount << ", strip efficiency: " << stripEfficiency << endl;

         triangleCount += triangles.size();

         ++it;
      }

      // add new output mesh
      if( !meshChunks.empty() )
      {
         CVertexArray const vertexArray = CMeshUtils::BuildVertexArray( exportedVertices.GetArray() ); 
         outputMeshes.push_back( CMesh( vertexArray, CIndexArray( primitiveIndices ), meshChunks ) ); 
      }
      
      triangleCount = 0;
      primitiveIndices.clear();
      exportedVertices.Clear();
      meshChunks.clear();
   }
}

//------------------------------------------------------------------------------------------
   
void CEditorArea::ParseTree( CXMLTree const & tree )
{
   if( tree.GetName() == "Mesh" )
   {
      ParseMesh( tree );
   }
   else if( tree.GetName() == "Light" )
   {
      ParseLight( tree );
   }
   else
   {
      CXMLTree::TNodePtrs const & children = tree.GetChildren();
      for( CXMLTree::TNodePtrs::const_iterator it = children.begin(); it != children.end(); ++it )
      {
         ParseTree( *(*it) );
      }
   }
}

//------------------------------------------------------------------------------------------
   
void CEditorArea::ParseMesh( CXMLTree const & meshTree )
{
   CEditorShader shader;
   std::vector< CEditorMesh::Vertex > vertices;
   std::vector< CEditorMesh::Triangle > triangles;

   CXMLTree::TNodePtrs const & children = meshTree.GetChildren();
   for( CXMLTree::TNodePtrs::const_iterator it = children.begin(); it != children.end(); ++it )
   {
      CXMLTree const & child = *(*it);
      
      if( child.GetName() == "Vtx" )
      {
         vertices.push_back( ParseVertex( child ) );
      }
      else if( child.GetName() == "Tri" )
      {
         int index1 = -1;
         int index2 = -1;
         int index3 = -1;
         
         child.GetKeyValue( "Idx1", index1 );
         child.GetKeyValue( "Idx2", index2 );
         child.GetKeyValue( "Idx3", index3 );

         triangles.push_back( CEditorMesh::Triangle( index1, index2, index3 ) );
      }
      else if( child.GetName() == "Shader" )
      {
         shader = CEditorShader( child );
      }
   }

   mMeshes.push_back( TMesh( shader, CEditorMesh( triangles, vertices ) ) );
}

//------------------------------------------------------------------------------------------

void CEditorArea::ParseLight( CXMLTree const & meshTree )
{
   mLights.push_back( CEditorLight( meshTree ) );
}

//------------------------------------------------------------------------------------------
   
CEditorMesh::Vertex const CEditorArea::ParseVertex( CXMLTree const & vertexTree )
{
   uint8 flags = 0;

   CVector3 position( CVector3::Zero() );
   CVector3 normal( CVector3::Zero() );
   CVector2 uvSet0( 0, 0 );
   CVector2 uvSet1( 0, 0 );
   CVector2 uvSet2( 0, 0 );
   CVector2 uvSet3( 0, 0 );
   CColor   color = CColor::Black();
   CColor   color2 = CColor::Black();
   CEditorMesh::Vertex::TWeights weights;
   CVector3 tangent( CVector3::Zero() );
   CVector3 binormal( CVector3::Zero() );

   CXMLTree::TNodePtrs const & children = vertexTree.GetChildren();
   for( CXMLTree::TNodePtrs::const_iterator it = children.begin(); it != children.end(); ++it )
   {
      CXMLTree const & child = *(*it);
      if( child.GetName() == "Pos" )
      {
         child.GetKeyValue( "Val", position );
      }
      else if( child.GetName() == "Normal" )
      {
         child.GetKeyValue( "Val", normal );
         flags |= CEditorMesh::Vertex::kFlag_Normal;
      }
      else if( child.GetName() == "Tangent" )
      {
         child.GetKeyValue( "Val", tangent );
         flags |= CEditorMesh::Vertex::kFlag_Tangent;
      }
      else if( child.GetName() == "Binormal" )
      {
         child.GetKeyValue( "Val", binormal );
         flags |= CEditorMesh::Vertex::kFlag_Binormal;
      }
      else if( child.GetName() == "Uv0" )
      {
         child.GetKeyValue( "Val", uvSet0 );
         flags |= CEditorMesh::Vertex::kFlag_UvSet0;
      }
      else if( child.GetName() == "Uv1" )
      {
         child.GetKeyValue( "Val", uvSet1 );
         flags |= CEditorMesh::Vertex::kFlag_UvSet1;
      }
      else if( child.GetName() == "Uv2" )
      {
         child.GetKeyValue( "Val", uvSet2 );
         flags |= CEditorMesh::Vertex::kFlag_UvSet2;
      }
      else if( child.GetName() == "Uv3" )
      {
         child.GetKeyValue( "Val", uvSet3 );
         flags |= CEditorMesh::Vertex::kFlag_UvSet3;
      }
      else if( child.GetName() == "Color" )
      {
         child.GetKeyValue( "Val", color );
         flags |= CEditorMesh::Vertex::kFlag_Color;
      }
      else if( child.GetName() == "Color2" )
      {
         child.GetKeyValue( "Val", color2 );
         flags |= CEditorMesh::Vertex::kFlag_Color2;
      }
   }
   

   return CEditorMesh::Vertex( flags,
                               position,
                               normal,
                               tangent,
                               binormal,
                               uvSet0,
                               uvSet1,
                               uvSet2,
                               uvSet3,
                               color,
                               color2,
                               weights );
}

