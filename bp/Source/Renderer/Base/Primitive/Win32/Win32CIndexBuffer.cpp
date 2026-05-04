//----------------------------------------------------------------------------
// Win32CIndexBuffer.cpp
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Win32CIndexBuffer.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/Win32/CRenderBackendPrivate.h"

//----------------------------------------------------------------------------

CIndexBuffer::CIndexBuffer(EIndexType type, TComPtr<IDirect3DIndexBuffer9> const & buffer)
:  mType(type)
,  mBuffer(buffer)
{
}

//----------------------------------------------------------------------------

TComPtr<IDirect3DIndexBuffer9> const CreateIndexBuffer(EIndexType type, void* pData, int size, int usage)
{
   IDirect3DIndexBuffer9* pIndexBuffer = NULL;

   HRESULT result = D3DDeviceUncached()->CreateIndexBuffer(size, 
                                                           usage, 
                                                           (type == kIT_Uint16) ? D3DFMT_INDEX16 : D3DFMT_INDEX32, 
                                                           D3DPOOL_DEFAULT, 
                                                           &pIndexBuffer, 
                                                           NULL);

   BPE_VERIFY( result == D3D_OK, false, "Creation of index buffer failed." );

   if( pData )
   {
      void* pDestData = NULL;
      pIndexBuffer->Lock(0, size, &pDestData, 0);
      
      memcpy(pDestData, pData, size);
      
      pIndexBuffer->Unlock();
   }

   return TComPtr<IDirect3DIndexBuffer9>(pIndexBuffer);
}

//----------------------------------------------------------------------------

CStaticIndexBuffer::CStaticIndexBuffer(std::vector<uint16> const & indices)
:  CIndexBuffer(kIT_Uint16, CreateIndexBuffer(kIT_Uint16, (void*)&indices.front(), sizeof(uint16) * indices.size(), D3DUSAGE_WRITEONLY))
{
}

//----------------------------------------------------------------------------

CStaticIndexBuffer::CStaticIndexBuffer(std::vector<uint32> const & indices)
:  CIndexBuffer(kIT_Uint32, CreateIndexBuffer(kIT_Uint32, (void*)&indices.front(), sizeof(uint32) * indices.size(), D3DUSAGE_WRITEONLY))
{
}

//----------------------------------------------------------------------------

CStaticIndexBuffer::CStaticIndexBuffer(EIndexType type, int const size)
:  CIndexBuffer(type, CreateIndexBuffer(type, NULL, size, D3DUSAGE_WRITEONLY))
{
}

//----------------------------------------------------------------------------

void* CStaticIndexBuffer::Lock()
{
   void* pIndexBufferData = NULL;
   
   // lock entire buffer
   mBuffer->Lock( 0, 0, &pIndexBufferData, 0 );

   return pIndexBufferData;
}

//----------------------------------------------------------------------------

void CStaticIndexBuffer::Unlock()
{
   mBuffer->Unlock();
}

//----------------------------------------------------------------------------

CDynamicIndexBuffer::CDynamicIndexBuffer()
:  CIndexBuffer(kIT_Invalid, TComPtr<IDirect3DIndexBuffer9>(NULL))
,  mBufferSize(0)
{
}

//----------------------------------------------------------------------------

void CDynamicIndexBuffer::Create(EIndexType const indexType, int size)
{
   mType = indexType;

   // ensure buffer is big enough, if it's not, reallocate.
   if( size > mBufferSize )
   {
      mBufferSize = size;
      mBuffer.Reset();
   }

   if( mBuffer.IsNull() && mBufferSize > 0 )
      mBuffer = CreateIndexBuffer(mType, NULL, size, D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC);
}

//----------------------------------------------------------------------------

void* CDynamicIndexBuffer::Lock(EIndexType const indexType, int const indexCount)
{
   const int indexSize[] =
   {
      sizeof(uint32),		//kIT_Uint32,
      sizeof(uint16),		//kIT_Uint16,
   };

   int byteSize = indexSize[indexType] * indexCount;
   Create(indexType, byteSize);

   if( mBuffer )
   {
      void* pLockedData = NULL;
      mBuffer->Lock(0, byteSize, &pLockedData, D3DLOCK_DISCARD);
      return pLockedData;
   }
   else
      return NULL;
}

//----------------------------------------------------------------------------

void CDynamicIndexBuffer::Unlock()
{
	if( mBuffer )
		mBuffer->Unlock();
}

//----------------------------------------------------------------------------

CDynamicIndexBufferPool_RT::CDynamicIndexBufferPool_RT(int const indexCount, int const chunkCount)
:  mIndexBuffer()
,  mTotalIndexCount(indexCount)
,  mCurrentIndexOffset(0)
,  mChunks(chunkCount)
,  mCurrentChunk(0)
,  mIsFirstChunk(1)
{
   mIndexBuffer.Create(kIT_Uint16, indexCount * sizeof(uint16));
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
   void* pLockedData = NULL;
   if( !mpPool->mIsFirstChunk )
   {
      // If this is not the first chunk we're promising we won't clobber other data.
      mpPool->mIndexBuffer.mBuffer->Lock(mStartIndex * sizeof(uint16), mCount * sizeof(uint16), &pLockedData, D3DLOCK_NOOVERWRITE);
   }
   else
   {
      // The first chunk will discard the buffer and allocate a new one
      mpPool->mIndexBuffer.mBuffer->Lock(mStartIndex * sizeof(uint16), mCount * sizeof(uint16), &pLockedData, D3DLOCK_DISCARD);
      mpPool->mIsFirstChunk = 0;
   }

   return (uint16*)pLockedData;
}

//----------------------------------------------------------------------------

void CDynamicIndexBufferPoolChunk_RT::Unlock()
{
   mpPool->mIndexBuffer.Unlock();
}
