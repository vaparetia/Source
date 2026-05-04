//----------------------------------------------------------------------------
// RVLCVertexBuffer.h
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Backend/CRenderHWAllocator.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

//----------------------------------------------------------------------------

class RENDERER_API CVertexBuffer
{
   friend class CCompiledShaderCache;
public:
   CVertexBuffer(CRenderHWAllocator::SHandle const * pMemory);
   ~CVertexBuffer();
   
public:
   CRenderHWAllocator::SHandle const * mpMemory;
};

//----------------------------------------------------------------------------

class RENDERER_API CStaticVertexBuffer : public CVertexBuffer
{
   typedef CVertexBuffer inherited;

public:
   CStaticVertexBuffer(int const size);

   template <class T> CStaticVertexBuffer(std::vector<T> const & data)
   :  CVertexBuffer(RenderBackend()->AllocFixed(data.size() * sizeof(T), kRM_Video, kRM_System) )
   {
      BPE_VERIFY(mpMemory != NULL, false, "Couldn't allocate memory!");
      memcpy(mpMemory->mpAddress, &data[0], data.size() * sizeof(T));
   }

   void* Lock();
   void  Unlock() {}
};

//----------------------------------------------------------------------------

class RENDERER_API CDynamicVertexBuffer : public CVertexBuffer
{
   typedef CVertexBuffer inherited;

public:
   CDynamicVertexBuffer();

   void* Lock(int const vertexStride, int const vertexCount);
   void  Unlock();

private:
   int mBufferSize;

};
