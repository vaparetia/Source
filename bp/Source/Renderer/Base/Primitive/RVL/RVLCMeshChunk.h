//----------------------------------------------------------------------------
// RVLCMeshChunk.h
// Copyright 2009
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "Renderer/Base/Material/CShaderVertexDataBinding.h"
#include "Engine/Primitives/CAABox.h"
#include <revolution/gx/GXEnum.h>
//----------------------------------------------------------------------------

class CMesh;
class COverridenMeshBuffers;
class CShaderVertexDataBinding;

//----------------------------------------------------------------------------

class RENDERER_API CMeshChunk
{
public:
   enum EPrimitive
   {
      kPrimitive_TriangleList  = GX_TRIANGLES,
      kPrimitive_TriangleStrip = GX_TRIANGLESTRIP,
      kPrimitive_TriangleFan   = GX_TRIANGLEFAN,
      kPrimitive_LineList      = GX_LINES
   };

   explicit CMeshChunk(CInputStream & stream);

   explicit CMeshChunk(CAABox const & bounds,
      CShaderVertexDataBinding const &desc,
      uint32 const displayListOffset,
      uint16 const displayListSize,
      int16 const materialIndex );

   void Draw(CShaderVertexDataBinding const & vertexDataBinding, CMesh const & mesh, COverridenMeshBuffers const * pOverridenBuffers) const;

   CAABox const &GetBounds() const { return mBounds; }
   int16 const &GetMaterialIndex() const { return mMaterialIndex; }


private:
   CAABox mBounds;
   
   CShaderVertexDataBinding mVtxDesc;
   uint32 mDisplayListOffset;
   uint16 mDisplayListSize;
   int16 mMaterialIndex;

};