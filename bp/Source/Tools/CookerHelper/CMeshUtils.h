//------------------------------------------------------------------------------------------
// CMeshUtils.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/Graphics/CVertexArray.h"
#include "CEditorMesh.h"
#include "vector"

//------------------------------------------------------------------------------------------

#define DO_VERTEX_CACHE_OPTIMIZATION
#define DO_POST_TRANSFORM_CACHE_OPTIOMIZATION

//------------------------------------------------------------------------------------------

class CMatrix3;

//------------------------------------------------------------------------------------------

class CMeshUtils
{
public:
   static void TransformArray( CMatrix34 const & transform, std::vector<CVector3> & points );
   static void TransformArray( CMatrix3 const & transform, std::vector<CVector3> & points );

   static CVertexArray * BuildVertexArray( std::vector<CEditorMesh::Vertex> const & vertices,
                                           std::vector<uint32> const & inTriangleListIndices,
                                           bool const buildTextureBasisVectors,
                                           int const verboseOutputLevel);
   
   static void BuildTextureBasisVectors( std::vector<uint32> const & inTriangleListIndices, 
                                         std::vector<CVector3> const & inPositions,
                                         std::vector<CVector3> const & inNormals,
                                         std::vector<CVector2> const & inUvs,
                                         std::vector<CVector3> & outS, 
                                         std::vector<CVector3> & outT );


   static void ProcessTriangleList( std::vector<uint32> const & triangleListIndices, 
                                    std::vector<CEditorMesh::Vertex> const & vertices,
                                    int const vertexBufferOffset,
                                    int const vertexCount,
                                    std::vector<uint32> * pOutIndices, 
                                    CMeshChunk::EPrimitive * pPrimitiveType,
                                    uint32 * pPrimitiveIndexBufferOffset,
                                    uint32 * pPrimitiveIndexCount,
                                    int const verboseOutputLevel );

   static void ProcessLineList( std::vector<uint32> const & lineListIndices, 
                                std::vector<CEditorMesh::Vertex> const & vertices,
                                int const vertexBufferOffset,
                                int const vertexCount,
                                std::vector<uint32> * pOutIndices, 
                                CMeshChunk::EPrimitive * pPrimitiveType,
                                uint32 * pPrimitiveIndexBufferOffset,
                                uint32 * pPrimitiveIndexCount,
                                int const verboseOutputLevel );
};

//------------------------------------------------------------------------------------------



