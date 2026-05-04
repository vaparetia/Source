//----------------------------------------------------------------------------
// PS3CIndexBuffer.cpp
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "PS3CIndexBuffer.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/PS3/PlatformIncludes.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

//----------------------------------------------------------------------------

CIndexBuffer::CIndexBuffer(EIndexType type, CRenderHWAllocator::SHandle const * pMemory)
:  mType(type)
,  mpMemory_UT(pMemory)
,  mpMemory_RT(pMemory)
{
}

//----------------------------------------------------------------------------

CIndexBuffer::~CIndexBuffer()
{
   if( mpMemory_UT ) 
      RenderBackend()->Free(mpMemory_UT);
}

//----------------------------------------------------------------------------

CStaticIndexBuffer::CStaticIndexBuffer(EIndexType type, int indexCount)
:  CIndexBuffer(type, RenderBackend()->AllocFixed(GetSizeForIndexType(type) * indexCount, 4, kRM_System, kRM_Video))
{
   BPE_VERIFY(mpMemory_UT != NULL, false, "Couldn't allocate memory!");
}

//----------------------------------------------------------------------------

CStaticIndexBuffer::CStaticIndexBuffer(std::vector<uint16> const & indices)
:  CIndexBuffer(kIT_Uint16, RenderBackend()->AllocFixed(2 * indices.size(), 4, kRM_System, kRM_Video))
{
   BPE_VERIFY(mpMemory_UT != NULL, false, "Couldn't allocate memory!");
   memcpy(mpMemory_UT->mpAddress, &indices[0], 2 * indices.size());
}

//----------------------------------------------------------------------------

CStaticIndexBuffer::CStaticIndexBuffer(std::vector<uint32> const & indices)
:  CIndexBuffer(kIT_Uint32, RenderBackend()->AllocFixed(4 * indices.size(), 4, kRM_System, kRM_Video))
{
   BPE_VERIFY(mpMemory_UT != NULL, false, "Couldn't allocate memory!");
   memcpy(mpMemory_UT->mpAddress, &indices[0], 4 * indices.size());
}

//----------------------------------------------------------------------------

void* CStaticIndexBuffer::Lock()
{
   return mpMemory_UT->mpAddress;
}

//----------------------------------------------------------------------------

void CStaticIndexBuffer::Unlock()
{
}

//----------------------------------------------------------------------------

CDynamicIndexBuffer::CDynamicIndexBuffer( ERenderResourceThreadUser const modifyThread )
:  CIndexBuffer(kIT_Invalid, NULL)
,  mBufferSize(0)
,  mModifyThread(modifyThread)
,  mGlobalListIndex(-1)
,  mLastUsedExtraMemory(-1)
{
   if( modifyThread == kRRTU_Update )
   {
      RenderBackend()->RegisterDynamicIndexBuffer(this);
   }

   for( int ii=0; ii<kMaxExtraMemory; ++ii )
   {
      mpExtraMemory_UT[ii] = NULL;
   }
}

//----------------------------------------------------------------------------

CDynamicIndexBuffer::~CDynamicIndexBuffer()
{
   if( mModifyThread == kRRTU_Update && mGlobalListIndex != -1 )
   {
      RenderBackend()->RemoveDynamicIndexBuffer(this);
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

void* CDynamicIndexBuffer::Lock(EIndexType const indexType, int const indexCount)
{
	const int indexSize[] =
	{
		sizeof(uint32),		//kIT_Uint32,
		sizeof(uint16),		//kIT_Uint16,
	};

	mType = indexType;

	mBufferSize = indexSize[mType] * indexCount;

   mLastUsedExtraMemory = (mLastUsedExtraMemory+1)%kMaxExtraMemory;
	if( mpExtraMemory_UT[mLastUsedExtraMemory] == NULL ) 
   {
      // Alignment restriction for vertex buffers is 2, but we align to 128 byte because we're dynamically filling vertex buffers on the SPU's and transfers are more efficient that way.
      mpExtraMemory_UT[mLastUsedExtraMemory] = RenderBackend()->AllocFixed(mBufferSize, 128, kRM_System, kRM_Video);
   }
   
   if( mModifyThread == kRRTU_Render )
   {
      mpMemory_RT = mpExtraMemory_UT[mLastUsedExtraMemory];
   }
	
   return mpExtraMemory_UT[mLastUsedExtraMemory]->mpAddress;
}

//----------------------------------------------------------------------------

void CDynamicIndexBuffer::FrameReset()
{
   // This function gets only called when the buffer is registered with the renderer.
   BPE_VERIFY(mModifyThread == kRRTU_Update, false, "Invalid FrameReset call to dynamic index buffer for render thread managed buffer.");
      
   mpMemory_RT = mpExtraMemory_UT[mLastUsedExtraMemory];
}

//----------------------------------------------------------------------------

CDynamicIndexBufferPool_RT::CDynamicIndexBufferPool_RT(int const indexCount, int const chunkCount)
:  mIndexBuffer(kRRTU_Render)
,  mTotalIndexCount(indexCount)
,  mCurrentIndexOffset(0)
,  mChunks(chunkCount)
,  mCurrentChunk(0)
{
   mIndexBuffer.Lock(kIT_Uint16, indexCount);
}

//----------------------------------------------------------------------------

void CDynamicIndexBufferPool_RT::FrameReset()
{
   mCurrentIndexOffset = 0;
   mCurrentChunk = 0;
   mIndexBuffer.Lock(kIT_Uint16, mTotalIndexCount);
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

CDynamicIndexBufferPool_UT::CDynamicIndexBufferPool_UT(int const indexCount, int const chunkCount)
:  mIndexBuffer(kRRTU_Update)
,  mTotalIndexCount(indexCount)
,  mCurrentIndexOffset(0)
,  mpCurrentChunks_UT(&mChunks[0])
,  mCurrentChunk(0)
{
   mIndexBuffer.Lock(kIT_Uint16, indexCount);
   for( int ii=0; ii<chunkCount; ++ii )
   {
      CDynamicIndexBufferPoolChunk_UT defaultPoolChunk;
      mChunks[0].push_back(defaultPoolChunk);
      mChunks[1].push_back(defaultPoolChunk);
   }
}

//----------------------------------------------------------------------------

void CDynamicIndexBufferPool_UT::FrameReset()
{
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
   mIndexBuffer.Lock(kIT_Uint16, mTotalIndexCount);
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
