//----------------------------------------------------------------------------
// PS3CVertexBuffer.h
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Backend/CRenderHWAllocator.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Backend/RenderBackendTypes.h"

//----------------------------------------------------------------------------

class RENDERER_API CVertexBuffer
{
   friend class CCompiledShaderCache;
public:
   CVertexBuffer(CRenderHWAllocator::SHandle const * pMemory);
   virtual ~CVertexBuffer();
   
public:
   CRenderHWAllocator::SHandle const * mpMemory_UT;   // Current memory in use by update thread.
   CRenderHWAllocator::SHandle const * mpMemory_RT;   // Memory currently being used for rendering (may not be the same at the UT version above).
};

//----------------------------------------------------------------------------

class RENDERER_API CStaticVertexBuffer : public CVertexBuffer
{
   typedef CVertexBuffer inherited;

public:
   CStaticVertexBuffer(int const size);
   CStaticVertexBuffer(void* pData, int const size);
   virtual ~CStaticVertexBuffer() {}

   template <class T> CStaticVertexBuffer(std::vector<T> const & data)
   :  CVertexBuffer(RenderBackend()->AllocFixed(data.size() * sizeof(T), 4, kRM_System, kRM_Video))
   {
      BPE_VERIFY(mpMemory_UT != NULL, false, "Couldn't allocate memory!");
      memcpy(mpMemory_UT->mpAddress, &data[0], data.size() * sizeof(T));
   }

   void* Lock();
   void  Unlock() {}
};

//----------------------------------------------------------------------------

class RENDERER_API CDynamicVertexBuffer : public CVertexBuffer
{
   typedef CVertexBuffer inherited;

public:
   CDynamicVertexBuffer( ERenderResourceThreadUser const modifyThread = kRRTU_Update );
   virtual ~CDynamicVertexBuffer();

   void* Lock(int const size);
   void  Unlock() {}
   void FrameReset();

   bool HasData() const { return mpMemory_UT != NULL; }

   ERenderResourceThreadUser           mModifyThread;
   int                                 mGlobalListIndex;

   static const int                    kMaxExtraMemory = 3;
   CRenderHWAllocator::SHandle const * mpExtraMemory_UT[kMaxExtraMemory];
   int                                 mLastUsedExtraMemory;
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
};

class RENDERER_API CDynamicVertexBufferPoolChunk_RT
{
   friend class CDynamicVertexBufferPool_RT;
public:
   CDynamicVertexBufferPoolChunk_RT() {}

   void* Lock() { return mpPool->mVertexBuffer.mpExtraMemory_UT[mpPool->mVertexBuffer.mLastUsedExtraMemory]->mpAddress + mOffset; }
   void  Unlock() {}

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

class CDynamicVertexBufferPool_UT
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
   CDynamicVertexBuffer                mVertexBuffer;
   int                                 mSize;
   int                                 mCurrentOffset;

   std::vector<CDynamicVertexBufferPoolChunk_UT>* mpCurrentChunks_UT;
   std::vector<CDynamicVertexBufferPoolChunk_UT> mChunks[2];
   int                                 mCurrentChunk;    
};

class CDynamicVertexBufferPoolChunk_UT
{
   friend class CDynamicVertexBufferPool_UT;
public:
   CDynamicVertexBufferPoolChunk_UT() {}

   void* Lock() { return mpPool->mVertexBuffer.mpExtraMemory_UT[mpPool->mVertexBuffer.mLastUsedExtraMemory]->mpAddress + mOffset; }
   void  Unlock() {}

   CDynamicVertexBufferPool_UT const * GetPool() const { return mpPool; }
   int GetOffset() const { return mOffset; }
   int GetSize() const { return mSize; }

protected:
   CDynamicVertexBufferPool_UT*   mpPool;
   int                  mOffset;       // Byte offset into vertex buffer where this chunk starts
   int                  mSize;         // Byte size of this chunk
};