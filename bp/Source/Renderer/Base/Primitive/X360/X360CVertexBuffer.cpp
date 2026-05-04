//----------------------------------------------------------------------------
// X360CVertexBuffer.cpp
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "X360CVertexBuffer.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/X360/CRenderBackendPrivate.h"
#include "Renderer/Base/Backend/X360/TDelayComPtr.h"

//----------------------------------------------------------------------------

CVertexBuffer::CVertexBuffer(IDirect3DVertexBuffer9 * buffer)
: mpCurrBuffer(buffer)
{
}

//----------------------------------------------------------------------------

CVertexBuffer::~CVertexBuffer()
{
   mpCurrBuffer = NULL;
}

//----------------------------------------------------------------------------

void CVertexBuffer::Bind(int const streamIndex, int const offset, int const vertexStride) const
{
   GetD3DDevice()->SetStreamSource(streamIndex, mpCurrBuffer, offset, vertexStride);
}

//----------------------------------------------------------------------------

CVertexBufferNormal::CVertexBufferNormal(IDirect3DVertexBuffer9 * buffer)
: CVertexBuffer(buffer)
, mBuffer(buffer)
{
}

//----------------------------------------------------------------------------

CVertexBufferNormal::~CVertexBufferNormal()
{
}

//----------------------------------------------------------------------------

CVertexBufferDouble::CVertexBufferDouble()
: CVertexBuffer(NULL)
{

}

//----------------------------------------------------------------------------

CVertexBufferTriple::CVertexBufferTriple()
: CVertexBuffer(NULL)
{

}

//----------------------------------------------------------------------------

IDirect3DVertexBuffer9* CreateVertexBuffer(int const size, bool const isDynamic)
{
   IDirect3DVertexBuffer9* pVertexBuffer = NULL;

   HRESULT result = D3DDeviceUncached()->CreateVertexBuffer(size, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pVertexBuffer, NULL);
   BPE_VERIFY( pVertexBuffer != NULL, false, "Creation of vertex buffer failed." );

   return pVertexBuffer;
}

//----------------------------------------------------------------------------

CStaticVertexBuffer::CStaticVertexBuffer(int const size)
: CVertexBufferNormal(CreateVertexBuffer(size, false))
{
}

//----------------------------------------------------------------------------

CStaticVertexBuffer::CStaticVertexBuffer(void* pData, int const size)
: CVertexBufferNormal(CreateVertexBuffer(size, false))
{
   void* pDest = Lock();
   memcpy(pDest, pData, size);
   Unlock();
}

//----------------------------------------------------------------------------

void* CStaticVertexBuffer::Lock()
{
   void* pLockedData = NULL;

   D3DVERTEXBUFFER_DESC desc;
   mBuffer->GetDesc(&desc);

   mBuffer->Lock(0, desc.Size, &pLockedData, /* D3DLOCK_DISCARD */ 0 );
   return pLockedData;
}

//----------------------------------------------------------------------------

void CStaticVertexBuffer::Unlock()
{
   mBuffer->Unlock();
}

//----------------------------------------------------------------------------

CDynamicVertexBuffer::CDynamicVertexBuffer()
: mBufferSize(0)
, mGlobalListIndex(-1)
, mFlags(0)
{
}

//----------------------------------------------------------------------------

CDynamicVertexBuffer::~CDynamicVertexBuffer()
{
   if( !mBuffer[0].IsNull() )
   {
      RenderBackend()->RemoveDynamicVertexBuffer(this);
   }
}

//----------------------------------------------------------------------------

void CDynamicVertexBuffer::Create(int byteSize)
{
   // ensure buffer is big enough, if it's not, reallocate.
   if( byteSize > mBufferSize )
   {
      char* pNewMemory = new char[byteSize];
      if( mVertexBufferMemory.get() )
      {
         memcpy(pNewMemory, mVertexBufferMemory.get(), mBufferSize);
         mVertexBufferMemory.reset(pNewMemory);
      }
      else
      {
         mVertexBufferMemory.reset(pNewMemory);
      }
      //
      mBufferSize = byteSize;
      if( !mBuffer[0].IsNull() )
      {
         mBuffer[0].Reset();
         mBuffer[1].Reset();

         RenderBackend()->RemoveDynamicVertexBuffer(this);
      }
   }

   if( mBuffer[0].IsNull() && mBufferSize > 0 )
   {
      mBuffer[0] = CreateVertexBuffer(mBufferSize, true);
      mBuffer[1] = CreateVertexBuffer(mBufferSize, true);

      RenderBackend()->RegisterDynamicVertexBuffer(this);
   }
}

//----------------------------------------------------------------------------

void* CDynamicVertexBuffer::Lock(int const size)
{
   Create( size );
   mFlags = 1;
   return MemLock_Unsafe();
}

//----------------------------------------------------------------------------

void CDynamicVertexBuffer::Unlock()
{
}

//----------------------------------------------------------------------------

void CDynamicVertexBuffer::FrameReset()
{
   if( mFlags )
   {
      FrameReset_Unsafe(mBufferSize);
      mFlags = 0;
   }
}

//----------------------------------------------------------------------------

void CDynamicVertexBuffer::FrameReset_Unsafe(int size)
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
   XMemCpyStreaming_WriteCombined(pLockedData, mVertexBufferMemory.get(), size);
   mpCurrBuffer->Unlock();
}

//----------------------------------------------------------------------------

CDynamicVertexBuffer_RT::CDynamicVertexBuffer_RT()
: mBufferSize(0)
, mGlobalListIndex(-1)
, mIsFirstChunk(0)
{
}

//----------------------------------------------------------------------------

CDynamicVertexBuffer_RT::~CDynamicVertexBuffer_RT()
{
   if( !mBuffer[0].IsNull() )
   {
      RenderBackend()->RemoveDynamicVertexBuffer_RT(this);
   }
}

//----------------------------------------------------------------------------

void CDynamicVertexBuffer_RT::Create(int byteSize)
{
   // ensure buffer is big enough, if it's not, reallocate.
   if( byteSize > mBufferSize )
   {
      mBufferSize = byteSize;
      if( !mBuffer[0].IsNull() )
      {
         mBuffer[0].Reset();
         mBuffer[1].Reset();

         RenderBackend()->RemoveDynamicVertexBuffer_RT(this);
      }
   }

   if( mBuffer[0].IsNull() && mBufferSize > 0 )
   {
      mBuffer[0] = CreateVertexBuffer(mBufferSize, true);
      mBuffer[1] = CreateVertexBuffer(mBufferSize, true);

      RenderBackend()->RegisterDynamicVertexBuffer_RT(this);
      mpCurrBuffer = mBuffer[0].GetPtr();
   }
}

//----------------------------------------------------------------------------

void* CDynamicVertexBuffer_RT::Lock(int const size)
{
   Create( size );

   SwapD3DBuffer();

   void* pLockedData = NULL;
   mpCurrBuffer->Lock(0, 0, &pLockedData, 0);
   return pLockedData;
}

//----------------------------------------------------------------------------

void CDynamicVertexBuffer_RT::SwapD3DBuffer()
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

void CDynamicVertexBuffer_RT::Unlock()
{
   if( mpCurrBuffer )
      mpCurrBuffer->Unlock();
}

//----------------------------------------------------------------------------

void CDynamicVertexBuffer_RT::FrameReset()
{
   mIsFirstChunk = 1;
}

//----------------------------------------------------------------------------

CDynamicVertexBufferPool_RT::CDynamicVertexBufferPool_RT(int const bufferSize, int const chunkCount)
: mVertexBuffer()
, mSize(bufferSize)
, mCurrentOffset(0)
, mChunks(chunkCount)
, mCurrentChunk(0)
, mIsFirstChunk(1)
{
   mVertexBuffer.Create(bufferSize);

   mVertexBufferMemory.reset(new char[bufferSize]);
}

//----------------------------------------------------------------------------

void CDynamicVertexBufferPool_RT::FrameReset()
{
   mCurrentOffset = 0;
   mCurrentChunk = 0;
   mIsFirstChunk = 1;
}

//----------------------------------------------------------------------------

CDynamicVertexBufferPoolChunk_RT* CDynamicVertexBufferPool_RT::AllocChunk(int const size)
{
   int const newOffset = mCurrentOffset + size;
   if( newOffset < mSize )
   {
      if( mCurrentChunk < mChunks.size() )
      {
         CDynamicVertexBufferPoolChunk_RT* pChunk = &mChunks[mCurrentChunk];
         pChunk->mpPool = this;
         pChunk->mOffset = mCurrentOffset;
         pChunk->mSize = size;

         mCurrentChunk++;
         mCurrentOffset = newOffset;
         return pChunk;
      }
   }

   return NULL;
}

//----------------------------------------------------------------------------

CDynamicVertexBufferPoolChunk_RT::CDynamicVertexBufferPoolChunk_RT()
{
}

//----------------------------------------------------------------------------

void* CDynamicVertexBufferPoolChunk_RT::Lock()
{
   char* pMemoryBuffer = mpPool->mVertexBufferMemory.get() + mOffset;
   return pMemoryBuffer;
}

//----------------------------------------------------------------------------

void CDynamicVertexBufferPoolChunk_RT::Unlock()
{
   char* pLockedData = NULL;
   if( !mpPool->mIsFirstChunk )
   {
      // If this is not the first chunk we're promising we won't clobber other data.
      mpPool->mVertexBuffer.mpCurrBuffer->Lock(0, 0, (void**)&pLockedData, D3DLOCK_NOOVERWRITE);
   }
   else
   {

      // The first chunk will discard the buffer and allocate a new one
      mpPool->mVertexBuffer.SwapD3DBuffer();
      mpPool->mVertexBuffer.mpCurrBuffer->Lock(0, 0, (void**)&pLockedData, 0);
      mpPool->mIsFirstChunk = 0;
   }

   char* pDataDest = pLockedData + mOffset;
   char* pDataSrc = mpPool->mVertexBufferMemory.get() + mOffset;
   XMemCpyStreaming_WriteCombined(pDataDest, pDataSrc, mSize);

   mpPool->mVertexBuffer.Unlock();
}

//----------------------------------------------------------------------------

CDynamicVertexBufferPool_UT::CDynamicVertexBufferPool_UT(int const bufferSize, int const chunkCount)
: mVertexBuffer()
, mSize(bufferSize)
, mCurrentOffset(0)
, mCurrentChunk(0)
{
   for( int ii=0; ii<chunkCount; ++ii )
   {
      CDynamicVertexBufferPoolChunk_UT defaultPoolChunk;
      mChunks[0].push_back(defaultPoolChunk);
      mChunks[1].push_back(defaultPoolChunk);
   }
   mpCurrentChunks_UT = &mChunks[0];
   mVertexBuffer.Create(bufferSize);
}

//----------------------------------------------------------------------------

void CDynamicVertexBufferPool_UT::FrameReset()
{
   mVertexBuffer.FrameReset_Unsafe(mCurrentOffset);

   if( mpCurrentChunks_UT == &mChunks[0] )
   {
      mpCurrentChunks_UT = &mChunks[1];
   }
   else
   {
      mpCurrentChunks_UT = &mChunks[0];
   }

   mCurrentOffset = 0;
   mCurrentChunk = 0;
}

//----------------------------------------------------------------------------

CDynamicVertexBufferPoolChunk_UT* CDynamicVertexBufferPool_UT::AllocChunk(int const size)
{
   int const newOffset = mCurrentOffset + size;
   if( newOffset < mSize )
   {
      if( mCurrentChunk < mpCurrentChunks_UT->size() )
      {
         CDynamicVertexBufferPoolChunk_UT* pChunk = &(*mpCurrentChunks_UT)[mCurrentChunk];
         pChunk->mpPool = this;
         pChunk->mOffset = mCurrentOffset;
         pChunk->mSize = size;

         mCurrentChunk++;
         mCurrentOffset = newOffset;
         return pChunk;
      }
   }

   return NULL;
}

//----------------------------------------------------------------------------

CDynamicVertexBufferPoolChunk_UT::CDynamicVertexBufferPoolChunk_UT()
{
}

//----------------------------------------------------------------------------

void* CDynamicVertexBufferPoolChunk_UT::Lock()
{
   char* pMemoryBuffer = (char*)mpPool->mVertexBuffer.MemLock_Unsafe();
   pMemoryBuffer += mOffset;
   return pMemoryBuffer;
}

//----------------------------------------------------------------------------

void CDynamicVertexBufferPoolChunk_UT::Unlock()
{
}
