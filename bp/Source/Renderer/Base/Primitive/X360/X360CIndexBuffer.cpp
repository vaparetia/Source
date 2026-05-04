//----------------------------------------------------------------------------
// X360CIndexBuffer.cpp
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "X360CIndexBuffer.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/X360/CRenderBackendPrivate.h"

inline int GetIndexBufferSize(EIndexType indexType, int indexCount)
{
   const int indexSize[] =
   {
      sizeof(uint32),		//kIT_Uint32,
      sizeof(uint16),		//kIT_Uint16,
   };

   return indexSize[indexType] * indexCount;
}

//----------------------------------------------------------------------------

CIndexBuffer::CIndexBuffer(EIndexType type, IDirect3DIndexBuffer9 * buffer)
: mType(type)
, mpCurrBuffer(buffer)
{
}

//----------------------------------------------------------------------------

CIndexBufferNormal::CIndexBufferNormal(EIndexType type, IDirect3DIndexBuffer9 * buffer)
: CIndexBuffer(type, buffer)
, mBuffer(buffer)
{
}

//----------------------------------------------------------------------------

CIndexBufferDouble::CIndexBufferDouble(EIndexType type)
: CIndexBuffer(type, NULL)
{
}

//----------------------------------------------------------------------------

CIndexBufferTriple::CIndexBufferTriple(EIndexType type)
: CIndexBuffer(type, NULL)
{
}

//----------------------------------------------------------------------------

IDirect3DIndexBuffer9 * CreateIndexBuffer(EIndexType type, void* pData, int size, int usage)
{
   IDirect3DIndexBuffer9* pIndexBuffer = NULL;

   HRESULT result = D3DDeviceUncached()->CreateIndexBuffer(size, 
      usage, 
      (type == kIT_Uint16) ? D3DFMT_INDEX16 : D3DFMT_INDEX32, 
      D3DPOOL_DEFAULT, 
      &pIndexBuffer, 
      NULL);
   BPE_VERIFY( pIndexBuffer != NULL, false, "Creation of index buffer failed." );

   if( pData )
   {
      void* pDestData = NULL;
      pIndexBuffer->Lock(0, size, &pDestData, 0);

      memcpy(pDestData, pData, size);

      pIndexBuffer->Unlock();
   }

   return pIndexBuffer;
}

//----------------------------------------------------------------------------

CStaticIndexBuffer::CStaticIndexBuffer(std::vector<uint16> const & indices)
: CIndexBufferNormal(kIT_Uint16, CreateIndexBuffer(kIT_Uint16, (void*)&indices.front(), sizeof(uint16) * indices.size(), D3DUSAGE_WRITEONLY))
{
}

//----------------------------------------------------------------------------

CStaticIndexBuffer::CStaticIndexBuffer(std::vector<uint32> const & indices)
: CIndexBufferNormal(kIT_Uint32, CreateIndexBuffer(kIT_Uint32, (void*)&indices.front(), sizeof(uint32) * indices.size(), D3DUSAGE_WRITEONLY))
{
}

//----------------------------------------------------------------------------

CStaticIndexBuffer::CStaticIndexBuffer(EIndexType type, int const size)
: CIndexBufferNormal(type, CreateIndexBuffer(type, NULL, size, D3DUSAGE_WRITEONLY))
{
}

//----------------------------------------------------------------------------

void* CStaticIndexBuffer::Lock()
{
   void* pIndexBufferData = NULL;
   
   // lock entire buffer
   mpCurrBuffer->Lock( 0, 0, &pIndexBufferData, 0 );

   return pIndexBufferData;
}

//----------------------------------------------------------------------------

void CStaticIndexBuffer::Unlock()
{
   mpCurrBuffer->Unlock();
}

//----------------------------------------------------------------------------

CDynamicIndexBuffer::CDynamicIndexBuffer()
: CIndexBufferDouble(kIT_Invalid)
, mBufferSize(0)
, mGlobalListIndex(-1)
, mFlags(0)
{
}

//----------------------------------------------------------------------------

CDynamicIndexBuffer::~CDynamicIndexBuffer()
{
   if( !mBuffer[0].IsNull() )
   {
      RenderBackend()->RemoveDynamicIndexBuffer(this);
   }
}

//----------------------------------------------------------------------------

void CDynamicIndexBuffer::Create(EIndexType const indexType, int byteSize)
{
   mType = indexType;

   // ensure buffer is big enough, if it's not, reallocate.
   if( byteSize > mBufferSize )
   {
      char* pNewMemory = new char[byteSize];
      if( mBufferMemory.get() )
      {
         memcpy(pNewMemory, mBufferMemory.get(), mBufferSize);
         mBufferMemory.reset(pNewMemory);
      }
      else
      {
         mBufferMemory.reset(pNewMemory);
      }
      //
      mBufferSize = byteSize;
      if( !mBuffer[0].IsNull() )
      { 
         mBuffer[0].Reset();
         mBuffer[1].Reset();

         RenderBackend()->RemoveDynamicIndexBuffer(this);
      }
   }

   if( mBuffer[0].IsNull() && mBufferSize > 0 )
   {
      mBuffer[0] = CreateIndexBuffer(mType, NULL, mBufferSize, D3DUSAGE_WRITEONLY);
      mBuffer[1] = CreateIndexBuffer(mType, NULL, mBufferSize, D3DUSAGE_WRITEONLY);

      RenderBackend()->RegisterDynamicIndexBuffer(this);
   }
}

//----------------------------------------------------------------------------

void* CDynamicIndexBuffer::Lock(EIndexType const indexType, int const indexCount)
{
   int byteSize = GetIndexBufferSize(indexType, indexCount);
   Create(indexType, byteSize);
   mFlags = 1;
   return MemLock_Unsafe();
}

//----------------------------------------------------------------------------

void CDynamicIndexBuffer::Unlock()
{
}

//----------------------------------------------------------------------------

void CDynamicIndexBuffer::FrameReset()
{
   if( mFlags )
   {
      FrameReset_Unsafe(mBufferSize);
      mFlags = 0;
   }
}

//----------------------------------------------------------------------------

void CDynamicIndexBuffer::FrameReset_Unsafe(int size)
{
   if( mpCurrBuffer == mBuffer[0].GetPtr() )
   {
      mpCurrBuffer = mBuffer[1].GetPtr();
   }
   else
   {
      mpCurrBuffer = mBuffer[0].GetPtr();
   }
   void* pLockedData = NULL;
   mpCurrBuffer->Lock(0, 0, &pLockedData, 0);
   XMemCpyStreaming_WriteCombined(pLockedData, mBufferMemory.get(), size);
   mpCurrBuffer->Unlock();
}

//----------------------------------------------------------------------------

CDynamicIndexBuffer_RT::CDynamicIndexBuffer_RT()
: CIndexBufferDouble(kIT_Invalid)
, mBufferSize(0)
, mGlobalListIndex(-1)
, mIsFirstChunk(0)
{
}

//----------------------------------------------------------------------------

CDynamicIndexBuffer_RT::~CDynamicIndexBuffer_RT()
{
   if( !mBuffer[0].IsNull() )
   {
      RenderBackend()->RemoveDynamicIndexBuffer_RT(this);
   }
}

//----------------------------------------------------------------------------

void CDynamicIndexBuffer_RT::Create(EIndexType const indexType, int size)
{
   mType = indexType;

   // ensure buffer is big enough, if it's not, reallocate.
   if( size > mBufferSize )
   {
      mBufferSize = size;
      if( !mBuffer[0].IsNull() )
      { 
         mBuffer[0].Reset();
         mBuffer[1].Reset();

         RenderBackend()->RemoveDynamicIndexBuffer_RT(this);
      }
   }

   if( mBuffer[0].IsNull() && mBufferSize > 0 )
   {
      mBuffer[0] = CreateIndexBuffer(mType, NULL, size, D3DUSAGE_WRITEONLY);
      mBuffer[1] = CreateIndexBuffer(mType, NULL, size, D3DUSAGE_WRITEONLY);

      RenderBackend()->RegisterDynamicIndexBuffer_RT(this);
      mpCurrBuffer = mBuffer[0].GetPtr();
   }
}

//----------------------------------------------------------------------------

void* CDynamicIndexBuffer_RT::Lock(EIndexType const indexType, int const indexCount)
{
   int byteSize = GetIndexBufferSize(indexType, indexCount);
   Create(indexType, byteSize);

   SwapD3DBuffer();

   if( mpCurrBuffer )
   {
      void* pLockedData = NULL;
      mpCurrBuffer->Lock(0, 0, &pLockedData, 0);
      return pLockedData;
   }
   else
      return NULL;
}

//----------------------------------------------------------------------------

void CDynamicIndexBuffer_RT::SwapD3DBuffer()
{
   if( mIsFirstChunk )
   {
      if( mpCurrBuffer )
      {
         if( mpCurrBuffer == mBuffer[0].GetPtr() )
         {
            mpCurrBuffer = mBuffer[1].GetPtr();
         }
         else
         {
            mpCurrBuffer = mBuffer[0].GetPtr();
         }
      }
      mIsFirstChunk = 0;
   }
}

//----------------------------------------------------------------------------

void CDynamicIndexBuffer_RT::Unlock()
{
   if( mpCurrBuffer )
      mpCurrBuffer->Unlock();
}

//----------------------------------------------------------------------------

void CDynamicIndexBuffer_RT::FrameReset()
{
   mIsFirstChunk = 1;
}

//----------------------------------------------------------------------------

CDynamicIndexBufferPool_RT::CDynamicIndexBufferPool_RT(int const indexCount, int const chunkCount)
: mIndexBuffer()
, mTotalIndexCount(indexCount)
, mCurrentIndexOffset(0)
, mChunks(chunkCount)
, mCurrentChunk(0)
, mIsFirstChunk(1)   
{
   mIndexBuffer.Create(kIT_Uint16, indexCount * sizeof(uint16));
   mIndexBufferMemory.reset(new uint16[indexCount]);
}

//----------------------------------------------------------------------------

void CDynamicIndexBufferPool_RT::FrameReset()
{
   mCurrentIndexOffset = 0;
   mCurrentChunk = 0;
   mIsFirstChunk = 1;
}

//----------------------------------------------------------------------------

CDynamicIndexBufferPoolChunk_RT* CDynamicIndexBufferPool_RT::AllocChunk(int const count)
{
   int const newIndexOffset = mCurrentIndexOffset + count;
   if( newIndexOffset < mTotalIndexCount )
   {
      if( mCurrentChunk < mChunks.size() )
      {
         CDynamicIndexBufferPoolChunk_RT* pChunk = &mChunks[mCurrentChunk];
         pChunk->mpPool = this;
         pChunk->mStartIndex = mCurrentIndexOffset;
         pChunk->mCount = count;

         mCurrentChunk++;
         mCurrentIndexOffset = newIndexOffset;
         return pChunk;
      }
   }

   return NULL;
}

//----------------------------------------------------------------------------

CDynamicIndexBufferPoolChunk_RT::CDynamicIndexBufferPoolChunk_RT()
{
}

//----------------------------------------------------------------------------

uint16* CDynamicIndexBufferPoolChunk_RT::Lock()
{
   uint16* pMemoryBuffer = mpPool->mIndexBufferMemory.get() + mStartIndex;
   return pMemoryBuffer;
}

//----------------------------------------------------------------------------

void CDynamicIndexBufferPoolChunk_RT::Unlock()
{
   uint16* pLockedData = NULL;
   if( !mpPool->mIsFirstChunk )
   {
      // If this is not the first chunk we're promising we won't clobber other data.
      mpPool->mIndexBuffer.mpCurrBuffer->Lock(0, 0, (void**)&pLockedData, D3DLOCK_NOOVERWRITE);
   }
   else
   {
      // The first chunk will discard the buffer and allocate a new one
      mpPool->mIndexBuffer.SwapD3DBuffer();
      mpPool->mIndexBuffer.mpCurrBuffer->Lock(0, 0, (void**)&pLockedData, 0);
      mpPool->mIsFirstChunk = 0;
   }

   uint16* pDataDest = pLockedData + mStartIndex;
   uint16* pDataSrc = mpPool->mIndexBufferMemory.get() + mStartIndex;
   XMemCpyStreaming_WriteCombined(pDataDest, pDataSrc, mCount*sizeof(uint16));

   mpPool->mIndexBuffer.Unlock();
}

//----------------------------------------------------------------------------

CDynamicIndexBufferPool_UT::CDynamicIndexBufferPool_UT(int const indexCount, int const chunkCount)
: mIndexBuffer()
, mTotalIndexCount(indexCount)
, mCurrentIndexOffset(0)
, mCurrentChunk(0)  
{
   for( int ii=0; ii<chunkCount; ++ii )
   {
      CDynamicIndexBufferPoolChunk_UT defaultPoolChunk;
      mChunks[0].push_back(defaultPoolChunk);
      mChunks[1].push_back(defaultPoolChunk);
   }
   mpCurrentChunks_UT = &mChunks[0];
   mIndexBuffer.Create(kIT_Uint16, indexCount * sizeof(uint16));
}

//----------------------------------------------------------------------------

void CDynamicIndexBufferPool_UT::FrameReset()
{
   mIndexBuffer.FrameReset_Unsafe(mCurrentIndexOffset*sizeof(uint16));

   if( mpCurrentChunks_UT == &mChunks[0] )
   {
      mpCurrentChunks_UT = &mChunks[1];
   }
   else
   {
      mpCurrentChunks_UT = &mChunks[0];
   }

   mCurrentIndexOffset = 0;
   mCurrentChunk = 0;
}

//----------------------------------------------------------------------------

CDynamicIndexBufferPoolChunk_UT* CDynamicIndexBufferPool_UT::AllocChunk(int const count)
{
   int const newIndexOffset = mCurrentIndexOffset + count;
   if( newIndexOffset < mTotalIndexCount )
   {
      if( mCurrentChunk < mpCurrentChunks_UT->size() )
      {
         CDynamicIndexBufferPoolChunk_UT* pChunk = &(*mpCurrentChunks_UT)[mCurrentChunk];
         pChunk->mpPool = this;
         pChunk->mStartIndex = mCurrentIndexOffset;
         pChunk->mCount = count;

         mCurrentChunk++;
         mCurrentIndexOffset = newIndexOffset;
         return pChunk;
      }
   }

   return NULL;
}

//----------------------------------------------------------------------------

CDynamicIndexBufferPoolChunk_UT::CDynamicIndexBufferPoolChunk_UT()
{
}

//----------------------------------------------------------------------------

uint16* CDynamicIndexBufferPoolChunk_UT::Lock()
{
   uint16* pMemoryBuffer = (uint16*)mpPool->mIndexBuffer.MemLock_Unsafe();
   pMemoryBuffer += mStartIndex;
   return pMemoryBuffer;
}

//----------------------------------------------------------------------------

void CDynamicIndexBufferPoolChunk_UT::Unlock()
{
}