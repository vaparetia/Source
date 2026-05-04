//----------------------------------------------------------------------------
// CMeshChunkRenderEntity.h
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Frontend/RenderObject/CDrawableRenderEntity.h"
#include "Renderer/Base/Frontend/RenderObject/CMeshRenderEntity.h"

//----------------------------------------------------------------------------

class CMeshChunk;

//----------------------------------------------------------------------------

class CMeshChunkRenderEntity : public CDrawableRenderEntity
{
public:
   RENDERER_API explicit CMeshChunkRenderEntity(CMeshRenderEntity* pMeshEntity,
                                                CMeshChunk const * pChunk);
   RENDERER_API virtual ~CMeshChunkRenderEntity();

   virtual void                  RenderGeometry(CShaderVertexDataBinding const & vertexDataBinding);

   virtual CMatrix34 const &     GetTransform() const                      { return static_cast<CMeshRenderEntity const *>(mpOwner)->mTransform; }
   virtual void                  SetTransform(CMatrix34 const & transform) { static_cast<CMeshRenderEntity *>(mpOwner)->mTransform = transform; }

   CMeshChunk const &            GetChunk() const                          { return *mpChunk; }

private:
   CMeshChunk const *   mpChunk;
};


