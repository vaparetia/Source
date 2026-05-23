
//----------------------------------------------------------------------------
// Win32CVertexBuffer.h
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/Primitive/CVertexBuffer.h"

#include "Engine/Mechanics/TComPtr.h"
#include "Engine/System/CSyncCriticalSection.h"

//----------------------------------------------------------------------------

class RENDERER_API CVertexBuffer
{
public:
   CVertexBuffer(TComPtr<IDirect3DVertexBuffer9> const & buffer);
   virtual ~CVertexBuffer() {}
   void Bind(int const streamIndex, int const offset, int const vertexStride) const;

public:
   TComPtr<IDirect3DVertexBuffer9>  mBuffer;
};

//----------------------------------------------------------------------------

class RENDERER_API CStaticVertexBuffer : public CVertexBuffer
{
   typedef CVertexBuffer inherited;

public:
   CStaticVertexBuffer(int const size);
   CStaticVertexBuffer(void* pData, int const size);
   virtual ~CStaticVertexBuffer() {}

   void* Lock();
   void Unlock();
};

//----------------------------------------------------------------------------

class RENDERER_API CDynamicVertexBuffer : public CVertexBuffer
{
   typedef CVertexBuffer inherited;

public:
   CDynamicVertexBuffer();
   virtual ~CDynamicVertexBuffer() {}

   void Create(int byteSize);

   void* Lock(int const size);
   void  Unlock();

   bool HasData() const { return mBufferSize > 0; }

   void FrameReset() {}

   int   mGlobalListIndex;
private:
   int   mBufferSize;
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
   CDynamicVertexBuffer                mVertexBuffer;
   int                                 mSize;
   int                                 mCurrentOffset;
   
   std::vector<CDynamicVertexBufferPoolChunk_RT> mChunks;
   int                                 mCurrentChunk;

   int                                 mIsFirstChunk;
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
   CDynamicVertexBufferPool_RT*   mpPool;
   int                  mOffset;       // Byte offset into vertex buffer where this chunk starts
   int                  mSize;         // Byte size of this chunk
};

//----------------------------------------------------------------------------

class CDynamicVertexBufferPoolChunk_UT;

class CDynamicVertexBufferPool_UT : public CDynamicVertexBufferPool_RT
{
public:
   CDynamicVertexBufferPoolChunk_UT* AllocChunk(int const size)
   {
      return (CDynamicVertexBufferPoolChunk_UT*)CDynamicVertexBufferPool_RT::AllocChunk(size);
   }
};

class CDynamicVertexBufferPoolChunk_UT : public CDynamicVertexBufferPoolChunk_RT
{
public:
   CDynamicVertexBufferPool_UT const * GetPool() const { return (CDynamicVertexBufferPool_UT*)mpPool; }
};