//----------------------------------------------------------------------------
// CMeshChunkRenderEntity.cpp
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CMeshChunkRenderEntity.h"

//----------------------------------------------------------------------------

#include "CMeshRenderEntity.h"
#include "Renderer/Base/Primitive/CMesh.h"
#include "Renderer/Base/Primitive/CMeshChunk.h"
#include "Renderer/Base/Primitive/CMeshBuffers.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

BPE_CTASSERT(sizeof(CMeshChunkRenderEntity) < kMaxRenderObjectSize);

//----------------------------------------------------------------------------

CMeshChunkRenderEntity::CMeshChunkRenderEntity(CMeshRenderEntity* pMeshEntity,
                                               CMeshChunk const * pChunk)
:  CDrawableRenderEntity(kType_RenderEntity)
,  mpChunk(pChunk)
{
   mpOwner = pMeshEntity;
   if( pChunk->GetMaterialIndex() != -1)
   {
      SetShader(static_cast<CMeshRenderEntity*>(mpOwner)->GetMesh()->GetMaterial(pChunk->GetMaterialIndex()).get());
   }
}

//----------------------------------------------------------------------------

CMeshChunkRenderEntity::~CMeshChunkRenderEntity()
{
}

//----------------------------------------------------------------------------

void CMeshChunkRenderEntity::RenderGeometry(CShaderVertexDataBinding const & vertexDataBinding)
{
   CMeshRenderEntity * pMeshEntity = static_cast<CMeshRenderEntity*>(mpOwner);

   CMeshBuffers & meshBuffers = const_cast<CMeshBuffers&>(pMeshEntity->GetMesh()->GetMeshBuffers());

#if 0
   // Nice and hacky: it's safe though, because mVertexData is free to change after the call to meshBuffers.SetStreams()!
   // The only real scary thing is the assumption about the dynamic vertex buffer layout/stride.
   if( BPE_BRANCH_HINT_FALSE( pMeshEntity->mDynamicData.HasData() ) )
   {
      CVertexData & vertexData = meshBuffers.VertexData();

      int const dynamicVertexDataStride = 16 + 12;
      vertexData.SetAttribute(kVDS_Position, 0, dynamicVertexDataStride, kVDT_Float4, &pMeshEntity->mDynamicData);
      vertexData.SetAttribute(kVDS_Normal, 16, dynamicVertexDataStride, kVDT_Float3, &pMeshEntity->mDynamicData);
   }
#endif

   gpRenderBackend->SetIndexData(meshBuffers.GetIndexBuffer());
   meshBuffers.SetVertexData(vertexDataBinding);

   gpRenderBackend->RenderPrimitives(
      mpChunk->mPrimitiveType, 
#if RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET()
      mpChunk->mVertexBufferOffset, 
      mpChunk->mVertexCount, 
#endif
      mpChunk->mIndexBufferOffset, mpChunk->mIndicesCount);
}
