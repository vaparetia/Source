//----------------------------------------------------------------------------
// VTACIndexBuffer.h
// Bluepoint/Armature
// Copyright 2011
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
   CIndexBuffer(EIndexType type, CRenderHWAllocator::SHandle const * pMemory, size_t indexCount );
   ~CIndexBuffer();
public:
   EIndexType                          mType;
   size_t                              mCount;
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

class CDynamicIndexBufferPool_RT;

class RENDERER_API CIndexBufferChunk
{
public:
   CIndexBufferChunk()
      : mpIndexBuffer( NULL )
      , mpMemory( NULL )
      , mCount( 0 )
   {
   }

   CIndexBufferChunk( CIndexBuffer *pIndexBuffer, uint16 *pMemory, int const count )
      : mpIndexBuffer( pIndexBuffer )
      , mpMemory( pMemory )
      , mCount( count )
   {
   }

   uint16* Lock() const { return mpMemory; }
   void  Unlock() const {}

   uint16 const *GetMemory() const { return mpMemory; }
   CIndexBuffer * GetIndexBuffer() const { return mpIndexBuffer; }
   int GetCount() const { return mCount; }

private:
   CIndexBuffer *mpIndexBuffer;
   uint16 *mpMemory;
   int mCount;
};

class RENDERER_API CDynamicIndexBufferPool_RT
{
public:
   typedef CIndexBufferChunk TChunk;
   typedef CIndexBufferChunk const &TChunkArg;

   CDynamicIndexBufferPool_RT(int const indexCount, int const chunkCount);
   CIndexBufferChunk const AllocChunk(int const count);
   static CIndexBufferChunk const NullChunk() { return CIndexBufferChunk(); }

   void FrameReset();

   CIndexBuffer const * GetIndexBuffer() const { return &mIndexBuffer; }

   int GetInUseSize() const { return mCurrentIndexOffset; }
   int GetTotalSize() const { return mTotalIndexCount; }
   int GetPeakSize() const { return mPeak; }
   char const *GetPeakArea() const { return mPeakArea; }

   static uint16 *LockChunk( CIndexBufferChunk const &chunk ) { return chunk.Lock(); }
   static void UnlockChunk( CIndexBufferChunk const &chunk ) { return chunk.Unlock(); }
   static bool IsChunkNull( CIndexBufferChunk const &chunk ) { return chunk.GetMemory() == NULL; }
   static void CopyToInts( int *pOut, CIndexBufferChunk const &chunk ) { new(pOut) CIndexBufferChunk( chunk ); }
   static CIndexBufferChunk const ChunkFromInts( int const *pIn ) { return CIndexBufferChunk( *reinterpret_cast<CIndexBufferChunk const *>( pIn ) ); }

protected:
   CDynamicIndexBuffer                 mIndexBuffer;
   int                                 mTotalIndexCount;
   int                                 mCurrentIndexOffset;
   int                                 mPeak;
   char                                mPeakArea[256];
};

//----------------------------------------------------------------------------

class CDynamicIndexBufferPool_UT;
class RENDERER_API CDynamicIndexBufferPool_UT
{
public:
   typedef CIndexBufferChunk TChunk;
   typedef CIndexBufferChunk const &TChunkArg;

   CDynamicIndexBufferPool_UT(int const indexCount, int const chunkCount);
   CIndexBufferChunk const AllocChunk(int const count);

   static CIndexBufferChunk const NullChunk() { return CIndexBufferChunk(); }

   void FrameReset();

   CIndexBuffer const * GetIndexBuffer() const { return &mIndexBuffer; }

   int GetInUseSize() const { return mCurrentIndexOffset; }
   int GetTotalSize() const { return mTotalIndexCount; }
   int GetPeakSize() const { return mPeak; }
   char const *GetPeakArea() const { return mPeakArea; }

   static uint16 *LockChunk( CIndexBufferChunk const &chunk ) { return chunk.Lock(); }
   static void UnlockChunk( CIndexBufferChunk const &chunk ) { return chunk.Unlock(); }
   static bool IsChunkNull( CIndexBufferChunk const &chunk ) { return chunk.GetMemory() == NULL; }

   static void CopyToInts( int *pOut, CIndexBufferChunk const &chunk ) { new(pOut) CIndexBufferChunk( chunk ); }
   static CIndexBufferChunk const ChunkFromInts( int const *pIn ) { return CIndexBufferChunk( *reinterpret_cast<CIndexBufferChunk const *>( pIn ) ); }

protected:
   CDynamicIndexBuffer                 mIndexBuffer;
   int                                 mTotalIndexCount;
   int                                 mCurrentIndexOffset;
   int                                 mPeak;
   char                                mPeakArea[256];
};
