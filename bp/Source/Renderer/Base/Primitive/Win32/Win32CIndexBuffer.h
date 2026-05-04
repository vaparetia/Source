//----------------------------------------------------------------------------
// Win32CIndexBuffer.h
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Primitive/ProgShader/CIndexBuffer.h"
#include "Engine/Mechanics/TComPtr.h"
#include "d3d9.h"

//----------------------------------------------------------------------------

class RENDERER_API CIndexBuffer
{
   friend class CRenderBackend;

public:
   CIndexBuffer(EIndexType type, TComPtr<IDirect3DIndexBuffer9> const & buffer);
   EIndexType GetType() const { return mType; }
   TComPtr<IDirect3DIndexBuffer9> const & GetBuffer() const { return mBuffer; }

   public:
   EIndexType                      mType;
   TComPtr<IDirect3DIndexBuffer9>  mBuffer;
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
	CDynamicIndexBuffer();

   void Create(EIndexType const indexType, int byteSize);

	void* Lock(EIndexType const indexType, int const indexCount);
	void Unlock();

	uint32 BufferSize() const { return mBufferSize; }

   void FrameReset() {}

   int         mGlobalListIndex;
private:
	int			mBufferSize;
};

//----------------------------------------------------------------------------

class CDynamicIndexBufferPoolChunk_RT;

class RENDERER_API CDynamicIndexBufferPool_RT
{
   friend class CDynamicIndexBufferPoolChunk_RT;
public:
   typedef CDynamicIndexBufferPoolChunk_RT* TChunk;
   typedef CDynamicIndexBufferPoolChunk_RT* TChunkArg;

   CDynamicIndexBufferPool_RT(int const indexCount, int const chunkCount);
   CDynamicIndexBufferPoolChunk_RT* AllocChunk(int const count);

   void FrameReset();

   CIndexBuffer const * GetIndexBuffer() const { return &mIndexBuffer; }

   int GetInUseSize() const { return mCurrentIndexOffset; }
   int GetTotalSize() const { return mTotalIndexCount; }
   int GetInUseChunkCount() const { return mCurrentChunk; }
   int GetTotalChunkCount() const { return mChunks.size(); }

   static CDynamicIndexBufferPoolChunk_RT* NullChunk() { return NULL; }
   static uint16 *LockChunk( TChunkArg pChunk );
   static void UnlockChunk( TChunkArg pChunk );
   static bool IsChunkNull( TChunkArg pChunk ) { return pChunk == NULL; }
   static void CopyToInts( int *pOut, TChunkArg pChunk ) { *pOut = (int) pChunk; }
   static TChunk const ChunkFromInts( int const *pIn ) { return reinterpret_cast<TChunk>( *pIn ); }

private:
   CDynamicIndexBuffer                 mIndexBuffer;
   int                                 mTotalIndexCount;
   int                                 mCurrentIndexOffset;

   std::vector<CDynamicIndexBufferPoolChunk_RT>  mChunks;
   int                                 mCurrentChunk;

   int                                 mIsFirstChunk;
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
   int               mStartIndex;
   int               mCount;
};

//----------------------------------------------------------------------------

class CDynamicIndexBufferPoolChunk_UT;

class CDynamicIndexBufferPool_UT : public CDynamicIndexBufferPool_RT
{
public:
   typedef CDynamicIndexBufferPoolChunk_UT* TChunk;
   typedef CDynamicIndexBufferPoolChunk_UT* TChunkArg;
   
   CDynamicIndexBufferPoolChunk_UT* AllocChunk(int const count)
   {
      return (CDynamicIndexBufferPoolChunk_UT*)CDynamicIndexBufferPool_RT::AllocChunk(count);
   }

   static CDynamicIndexBufferPoolChunk_UT* NullChunk() { return NULL; }
   static uint16 *LockChunk( TChunkArg pChunk );
   static void UnlockChunk( TChunkArg pChunk );
   static bool IsChunkNull( TChunkArg pChunk ) { return pChunk == NULL; }
   static void CopyToInts( int *pOut, TChunkArg pChunk ) { *pOut = (int) pChunk; }
   static TChunk const ChunkFromInts( int const *pIn ) { return reinterpret_cast<TChunk>( *pIn ); }
};

class CDynamicIndexBufferPoolChunk_UT : public CDynamicIndexBufferPoolChunk_RT
{
public:
   CDynamicIndexBufferPool_UT const * GetPool() const { return (CDynamicIndexBufferPool_UT*)mpPool; }
};

inline uint16 *CDynamicIndexBufferPool_RT::LockChunk( TChunkArg pChunk )
{
   return pChunk->Lock();
}

inline void CDynamicIndexBufferPool_RT::UnlockChunk( TChunkArg pChunk )
{
   pChunk->Unlock();
}

inline uint16 *CDynamicIndexBufferPool_UT::LockChunk( TChunkArg pChunk )
{
   return pChunk->Lock();
}

inline void CDynamicIndexBufferPool_UT::UnlockChunk( TChunkArg pChunk )
{
   pChunk->Unlock();
}