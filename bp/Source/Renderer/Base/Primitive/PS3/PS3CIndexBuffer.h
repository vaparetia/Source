//----------------------------------------------------------------------------
// PS3CIndexBuffer.h
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderHWAllocator.h"
#include "Renderer/Base/Primitive/ProgShader/CIndexBuffer.h"
#include "Renderer/Base/Backend/RenderBackendTypes.h"

//----------------------------------------------------------------------------

class RENDERER_API CIndexBuffer
{
public:
   CIndexBuffer(EIndexType type, CRenderHWAllocator::SHandle const * pMemory);
   ~CIndexBuffer();

public:
   EIndexType                          mType;
   CRenderHWAllocator::SHandle const * mpMemory_UT;
   CRenderHWAllocator::SHandle const * mpMemory_RT;
};

//----------------------------------------------------------------------------

class RENDERER_API CStaticIndexBuffer : public CIndexBuffer
{
public:
   CStaticIndexBuffer(std::vector<uint16> const & indices);
   CStaticIndexBuffer(std::vector<uint32> const & indices);
   CStaticIndexBuffer(EIndexType type, int indexCount);

   void* Lock();
   void Unlock();
};

//----------------------------------------------------------------------------

class RENDERER_API CDynamicIndexBuffer : public CIndexBuffer
{
public:
   CDynamicIndexBuffer( ERenderResourceThreadUser const modifyThread = kRRTU_Update );
   ~CDynamicIndexBuffer();

	void* Lock(EIndexType const indexType, int const indexCount);
	void Unlock() {}
   void FrameReset();

	uint32 BufferSize() const { return mBufferSize; }

public:
	int			                        mBufferSize;
   ERenderResourceThreadUser           mModifyThread;
   int                                 mGlobalListIndex;

   static const int                    kMaxExtraMemory = 3;
   CRenderHWAllocator::SHandle const * mpExtraMemory_UT[kMaxExtraMemory];
   int                                 mLastUsedExtraMemory;
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

protected:
   CDynamicIndexBuffer                 mIndexBuffer;
   int                                 mTotalIndexCount;
   int                                 mCurrentIndexOffset;

   std::vector<CDynamicIndexBufferPoolChunk_RT>  mChunks;
   int                                 mCurrentChunk;
};

class RENDERER_API CDynamicIndexBufferPoolChunk_RT
{
   friend class CDynamicIndexBufferPool_RT;
public:
   CDynamicIndexBufferPoolChunk_RT() {}

   uint16* Lock() { return (uint16*)(mpPool->mIndexBuffer.mpExtraMemory_UT[mpPool->mIndexBuffer.mLastUsedExtraMemory]->mpAddress + mStartIndex * sizeof(uint16)); }
   void  Unlock() {}

   CDynamicIndexBufferPool_RT const * GetPool() const { return mpPool; }
   int GetStartIndex() const { return mStartIndex; }
   int GetCount() const { return mCount; }

protected:
   CDynamicIndexBufferPool_RT* mpPool;
   int               mStartIndex;
   int               mCount;
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

protected:
   CDynamicIndexBuffer                 mIndexBuffer;
   int                                 mTotalIndexCount;
   int                                 mCurrentIndexOffset;

   std::vector<CDynamicIndexBufferPoolChunk_UT>* mpCurrentChunks_UT;
   std::vector<CDynamicIndexBufferPoolChunk_UT> mChunks[2];
   int                                 mCurrentChunk;
};

class RENDERER_API CDynamicIndexBufferPoolChunk_UT
{
   friend class CDynamicIndexBufferPool_UT;
public:
   CDynamicIndexBufferPoolChunk_UT() {}

   uint16* Lock() { return (uint16*)(mpPool->mIndexBuffer.mpExtraMemory_UT[mpPool->mIndexBuffer.mLastUsedExtraMemory]->mpAddress + mStartIndex * sizeof(uint16)); }
   void  Unlock() {}

   CDynamicIndexBufferPool_UT const * GetPool() const { return mpPool; }
   int GetStartIndex() const { return mStartIndex; }
   int GetCount() const { return mCount; }

protected:
   CDynamicIndexBufferPool_UT* mpPool;
   int               mStartIndex;
   int               mCount;
};