//----------------------------------------------------------------------------
// VTACVertexBuffer.h
// Bluepoint/Armature
// Copyright 2011
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Backend/CRenderHWAllocator.h"
#include "Renderer/Base/Backend/RenderBackendTypes.h"

#if BPE_TARGET==BPE_TARGET_VITA
#  include <sce_atomic.h>
#endif

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

   static CRenderHWAllocator::SHandle const * Alloc(int const size, int const alignment, ERenderMemory const, ERenderMemory const);

   template <class T> CStaticVertexBuffer(std::vector<T> const & data)
      :  CVertexBuffer(Alloc(data.size() * sizeof(T), 4, kRM_Video, kRM_System))
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
public:
   CDynamicVertexBufferPool_RT(int const bufferSize, int const chunkCount);
   CDynamicVertexBufferPoolChunk_RT* AllocChunk(int const size);

   void FrameReset();

   CVertexBuffer const * GetVertexBuffer() const { return &mVertexBuffer; }

   int GetInUseSize() const { return mCurrentOffset; }
   int GetTotalSize() const { return mSize; }
   int GetPeakSize() const { return mPeak; }
   char const *GetPeakArea() const { return mPeakArea; }

private:
   CDynamicVertexBuffer                mVertexBuffer;
   int                                 mSize;
   int                                 mCurrentOffset;
   int                                 mPeak;
   char                                mPeakArea[256];
};

//----------------------------------------------------------------------------
// CDynamicVertexBufferPoolChunk_RT is just a pointer to memory now
// that means that this == memory to write to
// it couldn't be this way on Win32
//----------------------------------------------------------------------------

class RENDERER_API CDynamicVertexBufferPoolChunk_RT
{
public:
   CDynamicVertexBufferPoolChunk_RT() {}

   void* Lock() { return this; }
   void* GetRenderMemory() const { return const_cast<CDynamicVertexBufferPoolChunk_RT *>( this )->Lock(); }
   void  Unlock() {}
};

//----------------------------------------------------------------------------

class CDynamicVertexBufferPoolChunk_UT;

class CDynamicVertexBufferPool_UT
{
public:
   CDynamicVertexBufferPool_UT(int const bufferSize, int const chunkCount);
   CDynamicVertexBufferPoolChunk_UT* AllocChunk(int const size);
   void *AllocChunkUnsafeVita(int const size);

   void FrameReset();

   CVertexBuffer const * GetVertexBuffer() const { return &mVertexBuffer; }

   int GetInUseSize() const { return mCurrentOffset; }
   int GetTotalSize() const { return mSize; }
   int GetPeakSize() const { return mPeak; }
   char const *GetPeakArea() const { return mPeakArea; }

private:
   CDynamicVertexBuffer                mVertexBuffer;
   int                                 mSize;
   volatile int                        mCurrentOffset;
   int                                 mPeak;
   char                                mPeakArea[256];
};

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// CDynamicVertexBufferPoolChunk_UT is just a pointer to memory now
// that means that this == memory to write to
// it couldn't be this way on Win32
//----------------------------------------------------------------------------

class CDynamicVertexBufferPoolChunk_UT
{
public:
   CDynamicVertexBufferPoolChunk_UT() {}

   void* Lock() { return this; }
   void* GetRenderMemory() const { return const_cast<CDynamicVertexBufferPoolChunk_UT *>( this )->Lock(); }
   void  Unlock() {}
};

inline void * CDynamicVertexBufferPool_UT::AllocChunkUnsafeVita(int const size)
{
   int const oldOffset = sceAtomicAdd32( &mCurrentOffset, size );
   BPE_ASSERT( oldOffset + size < mSize, "Overrun of vertex buffer pool!" );
   return ( mVertexBuffer.mpExtraMemory_UT[mVertexBuffer.mLastUsedExtraMemory]->mpAddress + oldOffset );
}
