//----------------------------------------------------------------------------
// X360CVertexBuffer.h
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/Primitive/CVertexBuffer.h"

#include "Renderer/Base/Backend/X360/TDelayComPtr.h"
#include "Engine/Mechanics/TComPtr.h"
#include "Engine/System/CSyncCriticalSection.h"

#include "boost/scoped_array.hpp"

//----------------------------------------------------------------------------

class RENDERER_API CVertexBuffer
{
   friend class CRenderBackend;
public:
   CVertexBuffer(IDirect3DVertexBuffer9 * buffer);
   virtual ~CVertexBuffer();

protected:
   void Bind(int const streamIndex, int const offset, int const vertexStride) const;

public:
   IDirect3DVertexBuffer9 * mpCurrBuffer;
};

//----------------------------------------------------------------------------

class RENDERER_API CVertexBufferNormal : public CVertexBuffer
{
   friend class CRenderBackend;
public:
   CVertexBufferNormal(IDirect3DVertexBuffer9 * buffer);
   virtual ~CVertexBufferNormal();

public:
   TDelayComPtr<IDirect3DVertexBuffer9> mBuffer;
};

//----------------------------------------------------------------------------

class RENDERER_API CVertexBufferDouble : public CVertexBuffer
{
   friend class CRenderBackend;

public:
   CVertexBufferDouble();
public:
   TDelayComPtr<IDirect3DVertexBuffer9> mBuffer[2];
};

//----------------------------------------------------------------------------

class RENDERER_API CStaticVertexBuffer : public CVertexBufferNormal
{
   typedef CVertexBufferNormal inherited;

public:
   CStaticVertexBuffer(int const size);
   CStaticVertexBuffer(void* pData, int const size);

   void* Lock();
   void Unlock();
};

//----------------------------------------------------------------------------

class RENDERER_API CVertexBufferTriple : public CVertexBuffer
{
   friend class CRenderBackend;

public:
   CVertexBufferTriple();
public:
   TDelayComPtr<IDirect3DVertexBuffer9> mBuffer[3];
};

//----------------------------------------------------------------------------

class RENDERER_API CDynamicVertexBuffer : public CVertexBufferDouble
{
   friend class CRenderBackend;
   typedef CVertexBufferTriple inherited;

public:
   CDynamicVertexBuffer();
   virtual ~CDynamicVertexBuffer();

   void Create(int byteSize);

   void* Lock(int const size);
   void* MemLock_Unsafe()
   {
      return mVertexBufferMemory.get();
   }
   void Unlock();
   void FrameReset();
   void FrameReset_Unsafe(int size);

   bool HasData() const { return mBufferSize > 0; }

   //
   int mBufferSize;
   boost::scoped_array<char> mVertexBufferMemory;
   int mGlobalListIndex;
   int mFlags;
};

//----------------------------------------------------------------------------

class RENDERER_API CDynamicVertexBuffer_RT : public CVertexBufferDouble
{
   friend class CRenderBackend;
   typedef CDynamicVertexBuffer inherited;

public:
   CDynamicVertexBuffer_RT();
   virtual ~CDynamicVertexBuffer_RT();

   void Create(int byteSize);

   void* Lock(int const size);
   void SwapD3DBuffer();
   void Unlock();
   void FrameReset();

   bool HasData() const { return mBufferSize > 0; }

private:
   int mBufferSize;
public:
   int mGlobalListIndex;
   int mIsFirstChunk;
};

//----------------------------------------------------------------------------

class CDynamicVertexBufferPoolChunk_RT;

class RENDERER_API CDynamicVertexBufferPool_RT
{
   friend class CDynamicVertexBufferPoolChunk_RT;
public:
   CDynamicVertexBufferPool_RT(int const bufferSize, int const chunkCount);
   CDynamicVertexBufferPoolChunk_RT* AllocChunk(int const size);

   void FrameReset();

   CVertexBuffer const * GetVertexBuffer() const { return &mVertexBuffer; }

   int GetInUseSize() const { return mCurrentOffset; }
   int GetTotalSize() const { return mSize; }
   int GetInUseChunkCount() const { return mCurrentChunk; }
   int GetTotalChunkCount() const { return mChunks.size(); }

private:
   CDynamicVertexBuffer_RT mVertexBuffer;
   boost::scoped_array<char> mVertexBufferMemory;
   int mSize;
   int mCurrentOffset;
   
   std::vector<CDynamicVertexBufferPoolChunk_RT> mChunks;
   int mCurrentChunk;

   int mIsFirstChunk;
};

class RENDERER_API CDynamicVertexBufferPoolChunk_RT
{
   friend class CDynamicVertexBufferPool_RT;
public:
   CDynamicVertexBufferPoolChunk_RT();

   void* Lock();
   void  Unlock();

   CDynamicVertexBufferPool_RT const * GetPool() const { return mpPool; }
   int GetOffset() const { return mOffset; }
   int GetSize() const { return mSize; }

protected:
   CDynamicVertexBufferPool_RT* mpPool;
   int mOffset; // Byte offset into vertex buffer where this chunk starts
   int mSize; // Byte size of this chunk
};

//----------------------------------------------------------------------------

class CDynamicVertexBufferPoolChunk_UT;

class RENDERER_API CDynamicVertexBufferPool_UT
{
   friend class CDynamicVertexBufferPoolChunk_UT;
public:
   CDynamicVertexBufferPool_UT(int const bufferSize, int const chunkCount);
   CDynamicVertexBufferPoolChunk_UT* AllocChunk(int const size);

   void FrameReset();

   CVertexBuffer const * GetVertexBuffer() const { return &mVertexBuffer; }

   int GetInUseSize() const { return mCurrentOffset; }
   int GetTotalSize() const { return mSize; }
   int GetInUseChunkCount() const { return mCurrentChunk; }
   int GetTotalChunkCount() const { return mpCurrentChunks_UT->size(); }

private:
   CDynamicVertexBuffer mVertexBuffer;
   int mSize;
   int mCurrentOffset;

   std::vector<CDynamicVertexBufferPoolChunk_UT>* mpCurrentChunks_UT;
   std::vector<CDynamicVertexBufferPoolChunk_UT> mChunks[2];
   int mCurrentChunk;
};

class RENDERER_API CDynamicVertexBufferPoolChunk_UT
{
   friend class CDynamicVertexBufferPool_UT;
public:
   CDynamicVertexBufferPoolChunk_UT();

   void* Lock();
   void  Unlock();

   CDynamicVertexBufferPool_UT const * GetPool() const { return mpPool; }
   int GetOffset() const { return mOffset; }
   int GetSize() const { return mSize; }

protected:
   CDynamicVertexBufferPool_UT* mpPool;
   int mOffset; // Byte offset into vertex buffer where this chunk starts
   int mSize; // Byte size of this chunk
};