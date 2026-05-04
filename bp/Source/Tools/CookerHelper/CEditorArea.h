//------------------------------------------------------------------------------------------
// CEditorArea.h
// Bluepoint
// Abstract
// Copyright 2003
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "CEditorMesh.h"
#include "CEditorShader.h"
#include "CEditorLight.h"

#include "Engine/Graphics/CMesh.h"

//------------------------------------------------------------------------------------------

class CXMLTree;

//------------------------------------------------------------------------------------------

class CEditorArea
{
public:
   typedef std::pair<CEditorShader, CEditorMesh>   TMesh;
   typedef std::vector<TMesh>                      TMeshVector;
   typedef std::vector<CEditorLight>               TLightVector;

   class CChunkTriangle
   {
   public:
      CChunkTriangle( int const shaderIndex,
                      int const vertex1,
                      int const vertex2,
                      int const vertex3 )
      :  mShaderIndex( shaderIndex )
      ,  mVertex1( vertex1 )
      ,  mVertex2( vertex2 )
      ,  mVertex3( vertex3 )
      {
      }

      int      mShaderIndex;
      int      mVertex1;
      int      mVertex2;
      int      mVertex3;
   };

   class CChunk
   {
   public:
      CChunk( CAABox const & bounds,
              std::vector< int > const & triangleIndices )
      :  mBounds( bounds )
      ,  mTriangleIndices( triangleIndices )
      {
      }

      static CAABox const GetBoundsFromTriangles( std::vector< CEditorMesh::Vertex > const & vertices,
                                                  std::vector< CChunkTriangle > const & triangles,
                                                  std::vector< int > const & triangleIndices );

      CAABox             mBounds;
      std::vector< int > mTriangleIndices;
   };

public:
   explicit CEditorArea();
   explicit CEditorArea( TMeshVector const & meshes,
                         TLightVector const & lights );
   
   explicit CEditorArea( CXMLTree const & tree );
   void PutTo( CXMLTree & tree );

   void AddMesh( TMesh const & mesh ) { mMeshes.push_back( mesh ); }
   void AddLight( CEditorLight const & light ) { mLights.push_back( light ); }
   
   void CreateMeshes( std::vector<CMesh> & outputMeshes );

   void ChunkMeshes( int const minTriangleCount,
                     real32 const size,
                     real32 const volumeThreshold );

   void CombineMeshes();

   TMeshVector const & GetMeshes() const { return mMeshes; }
   TLightVector const & GetLights() const { return mLights; }

private:
   void ParseTree( CXMLTree const & tree );
   
   void ParseMesh( CXMLTree const & meshTree );
   void ParseLight( CXMLTree const & meshTree );

   CEditorMesh::Vertex const ParseVertex( CXMLTree const & vertexTree );
      
   void ChunkMeshes( int const minTriangleCount,
                     real32 const size,
                     real32 const volumeThreshold,
                     TMeshVector const & inMeshes,
                     TMeshVector & outMeshes );

   bool const SplitChunk( CChunk const & chunk, 
                          std::vector<CEditorMesh::Vertex> const & globalVertices,
                          std::vector<CChunkTriangle> const & globalTriangles,
                          std::vector<CVector3> const & globalCentroids,
                          int const axis, 
                          real32 const volumeThreshold, 
                          int const minTriangles,
                          std::list<CChunk> & outChunks );

private:
   TMeshVector    mMeshes;
   TLightVector   mLights;
};
