//----------------------------------------------------------------------------
// X360CIndexBuffer.h
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Primitive/ProgShader/CIndexBuffer.h"
#include "Renderer/Base/Backend/X360/TDelayComPtr.h"

#include "boost/scoped_array.hpp"

#include <xtl.h>

//----------------------------------------------------------------------------

class RENDERER_API CIndexBuffer
{
   friend class CRenderBackend;

public:
   CIndexBuffer(EIndexType type, IDirect3DIndexBuffer9 * buffer);
   EIndexType GetType() const { return mType; }

public:
   EIndexType mType;
   IDirect3DIndexBuffer9 * mpCurrBuffer;
};

//----------------------------------------------------------------------------

class RENDERER_API CIndexBufferNormal : public CIndexBuffer
{
   friend class CRenderBackend;

public:
   CIndexBufferNormal(EIndexType type, IDirect3DIndexBuffer9 * buffer);

public:
   TDelayComPtr<IDirect3DIndexBuffer9> mBuffer;
};

//----------------------------------------------------------------------------

class RENDERER_API CIndexBufferDouble : public CIndexBuffer
{
   friend class CRenderBackend;

public:
   CIndexBufferDouble(EIndexType type);

public:
   TDelayComPtr<IDirect3DIndexBuffer9> mBuffer[2];
};

//----------------------------------------------------------------------------

class RENDERER_API CIndexBufferTriple : public CIndexBuffer
{
   friend class CRenderBackend;

public:
   CIndexBufferTriple(EIndexType type);

public:
   TDelayComPtr<IDirect3DIndexBuffer9> mBuffer[3];
};

//----------------------------------------------------------------------------

class RENDERER_API CStaticIndexBuffer : public CIndexBufferNormal
{
public:
   CStaticIndexBuffer(std::vector<uint16> const & indices);
   CStaticIndexBuffer(std::vector<uint32> const & indices);
   CStaticIndexBuffer(EIndexType type, int indexCount);

   void* Lock();
   void Unlock();
};

//----------------------------------------------------------------------------

class RENDERER_API CDynamicIndexBuffer : public CIndexBufferDouble
{
   friend class CRenderBackend;
public:
   CDynamicIndexBuffer();
   ~CDynamicIndexBuffer();

   void Create(EIndexType const indexType, int byteSize);

   void* Lock(EIndexType const indexType, int const indexCount);
   void* MemLock_Unsafe()
   {
      return mBufferMemory.get();
   }
   void SwapD3DBuffer();
   void Unlock();
   void FrameReset();
   void FrameReset_Unsafe(int size);

   uint32 BufferSize() const { return mBufferSize; }

   //
   int mBufferSize;
   boost::scoped_array<char> mBufferMemory;
   int mGlobalListIndex;
   int mFlags;
};

//----------------------------------------------------------------------------

class RENDERER_API CDynamicIndexBuffer_RT : public CIndexBufferDouble
{
   friend class CRenderBackend;
public:
   CDynamicIndexBuffer_RT();
   ~CDynamicIndexBuffer_RT();

   void Create(EIndexType const indexType, int byteSize);

   void* Lock(EIndexType const indexType, int const indexCount);
   void SwapD3DBuffer();
   void Unlock();
   void FrameReset();

   uint32 BufferSize() const { return mBufferSize; }

private:
   int mBufferSize;
public:
   int mGlobalListIndex;
   int mIsFirstChunk;
};

//----------------------------------------------------------------------------

class CDynamicIndexBufferPoolChunk_RT;

class RENDERER_API CDynamicIndexBufferPool_RT
{
   friend class CDynamicIndexBufferPoolChunk_RT;
public:
   CDynamicIndexBufferPool_RT(int const indexCount, int const chunkCount);
   CDynamicIndexBufferPoolChunk_RT* AllocChunk(int const count);

   void FrameReset();

   CIndexBuffer const * GetIndexBuffer() const { return &mIndexBuffer; }

   int GetInUseSize() const { return mCurrentIndexOffset; }
   int GetTotalSize() const { return mTotalIndexCount; }
   int GetInUseChunkCount() const { return mCurrentChunk; }
   int GetTotalChunkCount() const { return mChunks.size(); }

private:
   CDynamicIndexBuffer_RT mIndexBuffer;
   boost::scoped_array<uint16> mIndexBufferMemory;
   int mTotalIndexCount;
   int mCurrentIndexOffset;

   std::vector<CDynamicIndexBufferPoolChunk_RT> mChunks;
   int mCurrentChunk;
   int mIsFirstChunk;
};

class RENDERER_API CDynamicIndexBufferPoolChunk_RT
{
   friend class CDynamicIndexBufferPool_RT;
public:
   CDynamicIndexBufferPoolChunk_RT();

   uint16* Lock();
   void  Unlock();

   CDynamicIndexBufferPool_RT const * GetPool() const { return mpPool; }
   int GetStartIndex() const { return mStartIndex; }
   int GetCount() const { return mCount; }

protected:
   CDynamicIndexBufferPool_RT* mpPool;
   int mStartIndex;
   int mCount;
};

//----------------------------------------------------------------------------
class CDynamicIndexBufferPoolChunk_UT;

class RENDERER_API CDynamicIndexBufferPool_UT
{
   friend class CDynamicIndexBufferPoolChunk_UT;
public:
   CDynamicIndexBufferPool_UT(int const indexCount, int const chunkCount);
   CDynamicIndexBufferPoolChunk_UT* AllocChunk(int const count);

   void FrameReset();

   CIndexBuffer const * GetIndexBuffer() const { return &mIndexBuffer; }

   int GetInUseSize() const { return mCurrentIndexOffset; }
   int GetTotalSize() const { return mTotalIndexCount; }
   int GetInUseChunkCount() const { return mCurrentChunk; }
   int GetTotalChunkCount() const { return mpCurrentChunks_UT->size(); }

private:
   CDynamicIndexBuffer mIndexBuffer;
   int mTotalIndexCount;
   int mCurrentIndexOffset;

   std::vector<CDynamicIndexBufferPoolChunk_UT>* mpCurrentChunks_UT;
   std::vector<CDynamicIndexBufferPoolChunk_UT> mChunks[2];
   int mCurrentChunk;
};

class RENDERER_API CDynamicIndexBufferPoolChunk_UT
{
   friend class CDynamicIndexBufferPool_UT;
public:
   CDynamicIndexBufferPoolChunk_UT();

   uint16* Lock();
   void  Unlock();

   CDynamicIndexBufferPool_UT const * GetPool() const { return mpPool; }
   int GetStartIndex() const { return mStartIndex; }
   int GetCount() const { return mCount; }

protected:
   CDynamicIndexBufferPool_UT* mpPool;
   int mStartIndex;
   int mCount;
};