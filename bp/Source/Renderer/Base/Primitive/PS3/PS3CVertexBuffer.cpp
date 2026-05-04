//----------------------------------------------------------------------------
// PS3CVertexBuffer.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "PS3CVertexBuffer.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/PS3/PlatformIncludes.h"

//----------------------------------------------------------------------------

CVertexBuffer::CVertexBuffer(CRenderHWAllocator::SHandle const * pMemory)
:  mpMemory_UT(pMemory)
,  mpMemory_RT(pMemory)
{
}
                 
//----------------------------------------------------------------------------

CVertexBuffer::~CVertexBuffer()
{
   if( mpMemory_UT ) 
      RenderBackend()->Free(mpMemory_UT);
}
                 
//----------------------------------------------------------------------------

CStaticVertexBuffer::CStaticVertexBuffer(int const size)
:  CVertexBuffer(RenderBackend()->AllocFixed(size, 4, kRM_System, kRM_Video))
{
   BPE_VERIFY(mpMemory_UT != NULL, false, "Couldn't allocate memory!");
}

//----------------------------------------------------------------------------

CStaticVertexBuffer::CStaticVertexBuffer(void* pData, int const size)
:  CVertexBuffer(RenderBackend()->AllocFixed(size, 4, kRM_System, kRM_Video))
{
   BPE_VERIFY(mpMemory_UT != NULL, false, "Couldn't allocate memory!");
   memcpy(mpMemory_UT->mpAddress, pData, size);
}

//----------------------------------------------------------------------------

void* CStaticVertexBuffer::Lock()
{
   return mpMemory_UT->mpAddress;
}

//----------------------------------------------------------------------------

CDynamicVertexBuffer::CDynamicVertexBuffer( ERenderResourceThreadUser const modifyThread )
:  CVertexBuffer(NULL)
,  mGlobalListIndex(-1)
,  mModifyThread(modifyThread)
,  mLastUsedExtraMemory(-1)
{
   if( modifyThread == kRRTU_Update )
   {
      RenderBackend()->RegisterDynamicVertexBuffer(this);
   }

   for( int ii=0; ii<kMaxExtraMemory; ++ii )
   {
      mpExtraMemory_UT[ii] = NULL;
   }
}

//----------------------------------------------------------------------------

CDynamicVertexBuffer::~CDynamicVertexBuffer()
{
   if( mModifyThread == kRRTU_Update && mGlobalListIndex != -1 )
   {
      RenderBackend()->RemoveDynamicVertexBuffer(this);
   }
   for( int ii=0; ii<kMaxExtraMemory; ++ii )
   {
      if( mpExtraMemory_UT[ii] )
      {
         RenderBackend()->Free(mpExtraMemory_UT[ii]);
      }
   }
}

//----------------------------------------------------------------------------

void * CDynamicVertexBuffer::Lock(int const size)
{
   mLastUsedExtraMemory = (mLastUsedExtraMemory+1)%kMaxExtraMemory;
   if( mpExtraMemory_UT[mLastUsedExtraMemory] == NULL ) 
   {
      // Alignment restriction for vertex buffers is 2, but we align to 128 byte because we're dynamically filling vertex buffers on the SPU's and transfers are more efficient that way.
      mpExtraMemory_UT[mLastUsedExtraMemory] = RenderBackend()->AllocFixed(size, 128, kRM_System, kRM_Video);
   }
   
   // If this buffer is locked/unlocked by the render thread we need to update the render memory ptr right here
   if( mModifyThread == kRRTU_Render )
   {
      mpMemory_RT = mpExtraMemory_UT[mLastUsedExtraMemory];
   }

   return mpExtraMemory_UT[mLastUsedExtraMemory]->mpAddress;
}

//----------------------------------------------------------------------------

void CDynamicVertexBuffer::FrameReset()
{
   // This function gets only called when the buffer is registered with the renderer.
   BPE_VERIFY(mModifyThread == kRRTU_Update, false, "Invalid FrameReset call to dynamic vertex buffer for render thread managed buffer.");

   mpMemory_RT = mpExtraMemory_UT[mLastUsedExtraMemory];
}

//----------------------------------------------------------------------------

CDynamicVertexBufferPool_RT::CDynamicVertexBufferPool_RT(int const bufferSize, int const chunkCount)
:  mVertexBuffer(kRRTU_Render)
,  mSize(bufferSize)
,  mCurrentOffset(0)
,  mChunks(chunkCount)
,  mCurrentChunk(0)
{
   mVertexBuffer.Lock(bufferSize);
}

//----------------------------------------------------------------------------

void CDynamicVertexBufferPool_RT::FrameReset()
{
   mCurrentOffset = 0;
   mCurrentChunk = 0;

   mVertexBuffer.Lock(mSize);
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

CDynamicVertexBufferPool_UT::CDynamicVertexBufferPool_UT(int const bufferSize, int const chunkCount)
:  mVertexBuffer(kRRTU_Update)
,  mSize(bufferSize)
,  mCurrentOffset(0)
,  mpCurrentChunks_UT(&mChunks[0])
,  mCurrentChunk(0)
{
   mVertexBuffer.Lock(bufferSize);
   for( int ii=0; ii<chunkCount; ++ii )
   {
      CDynamicVertexBufferPoolChunk_UT defaultPoolChunk;
      mChunks[0].push_back(defaultPoolChunk);
      mChunks[1].push_back(defaultPoolChunk);
   }
}

//----------------------------------------------------------------------------

void CDynamicVertexBufferPool_UT::FrameReset()
{
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

   mVertexBuffer.Lock(mSize);
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

