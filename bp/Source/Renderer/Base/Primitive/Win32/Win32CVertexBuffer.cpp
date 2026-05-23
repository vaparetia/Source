//----------------------------------------------------------------------------
// Win32CVertexBuffer.cpp
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Win32CVertexBuffer.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/Win32/CRenderBackendPrivate.h"

//----------------------------------------------------------------------------

CVertexBuffer::CVertexBuffer(TComPtr<IDirect3DVertexBuffer9> const & buffer)
:  mBuffer(buffer)
{
}

//----------------------------------------------------------------------------

void CVertexBuffer::Bind(int const streamIndex, int const offset, int const vertexStride) const
{
   D3DDevice()->SetStreamSource(streamIndex, mBuffer.GetPtr(), offset, vertexStride);
}

//----------------------------------------------------------------------------

TComPtr<IDirect3DVertexBuffer9> const CreateVertexBuffer(int const size, bool const isDynamic)
{
   IDirect3DVertexBuffer9* pVertexBuffer = NULL;

   HRESULT result = D3DDeviceUncached()->CreateVertexBuffer(size, D3DUSAGE_WRITEONLY | (isDynamic ? D3DUSAGE_DYNAMIC : 0), 0, D3DPOOL_DEFAULT, &pVertexBuffer, NULL);

   if( result != D3D_OK )
      return TComPtr<IDirect3DVertexBuffer9>(NULL);

   return TComPtr<IDirect3DVertexBuffer9>(pVertexBuffer);
}

//----------------------------------------------------------------------------

CStaticVertexBuffer::CStaticVertexBuffer(int const size)
:  CVertexBuffer(CreateVertexBuffer(size, false))
{
}

//----------------------------------------------------------------------------

CStaticVertexBuffer::CStaticVertexBuffer(void* pData, int const size)
: CVertexBuffer(CreateVertexBuffer(size, false))
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
:  CVertexBuffer(TComPtr<IDirect3DVertexBuffer9>(NULL))
,  mBufferSize(0)
{
}

//----------------------------------------------------------------------------

void CDynamicVertexBuffer::Create(int byteSize)
{
   // ensure buffer is big enough, if it's not, reallocate.
   if( byteSize > mBufferSize )
   {
      mBufferSize = byteSize;
      mBuffer.Reset();
   }

   if( mBuffer.IsNull() && mBufferSize > 0 )
   {
      mBuffer = CreateVertexBuffer(mBufferSize, true);
   }
}

//----------------------------------------------------------------------------

void* CDynamicVertexBuffer::Lock(int const size)
{
   Create( size );

   void* pLockedData = NULL;
   mBuffer->Lock(0, size, &pLockedData, D3DLOCK_DISCARD);
   return pLockedData;
}

//----------------------------------------------------------------------------

void CDynamicVertexBuffer::Unlock()
{
   if( mBuffer )
      mBuffer->Unlock();
}

//----------------------------------------------------------------------------

CDynamicVertexBufferPool_RT::CDynamicVertexBufferPool_RT(int const bufferSize, int const chunkCount)
:  mVertexBuffer()
,  mSize(bufferSize)
,  mCurrentOffset(0)
,  mChunks(chunkCount)
,  mCurrentChunk(0)
,  mIsFirstChunk(1)
{
   mVertexBuffer.Create(bufferSize);
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
   void* pLockedData = NULL;
   if( !mpPool->mIsFirstChunk )
   {
      // If this is not the first chunk we're promising we won't clobber other data.
      mpPool->mVertexBuffer.mBuffer->Lock(mOffset, mSize, &pLockedData, D3DLOCK_NOOVERWRITE);
   }
   else
   {
      // The first chunk will discard the buffer and allocate a new one
      mpPool->mVertexBuffer.mBuffer->Lock(mOffset, mSize, &pLockedData, D3DLOCK_DISCARD);
      mpPool->mIsFirstChunk = 0;
   }

   return pLockedData;
}

//----------------------------------------------------------------------------

void CDynamicVertexBufferPoolChunk_RT::Unlock()
{
   mpPool->mVertexBuffer.Unlock();
}
