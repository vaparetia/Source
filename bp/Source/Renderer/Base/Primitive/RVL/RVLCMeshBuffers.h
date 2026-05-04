//----------------------------------------------------------------------------
// RVLCMeshBuffers.h
// Copyright 2009
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Backend/CRenderHWAllocator.h"
#include "RVLCVertexBuffer.h"
#include "RVLCVertexData.h"

class CVertexArray;

class RENDERER_API COverridenMeshBuffers
{
public:
   ~COverridenMeshBuffers();

   CVertexData mVertexData;
   CDynamicVertexBuffer mBufferOverrides;
};

class RENDERER_API CMeshBuffers
{
public:
   explicit CMeshBuffers(
      CVertexArray const & vertexArray, 
      void const *pDisplayList,
      uint32 const displayListSize );

   ~CMeshBuffers();

   void SetCurrent( ) const;

   void const *GetDisplayList( uint32 const offset ) const { return mpDisplayList->mpAddress + offset; }

private:
   CVertexBuffer mVertexBuffer;
   CVertexData mVertexData;
   CRenderHWAllocator::SHandle const *mpDisplayList;
   uint32 mDisplayListSize;
};