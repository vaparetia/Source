//----------------------------------------------------------------------------
// VTACIndexBuffer.cpp
// Bluepoint/Armature
// Copyright 2011
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Renderer/Base/Primitive/CIndexBuffer.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderBackend.h"

//----------------------------------------------------------------------------

CIndexBuffer::CIndexBuffer(EIndexType type, CRenderHWAllocator::SHandle const * pMemory, size_t const indexCount)
:  mType(type)
,  mCount( indexCount )
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
:  CIndexBuffer(type, RenderBackend()->AllocFixed(GetSizeForIndexType(type) * indexCount, 4, kRM_System, kRM_Video), indexCount )
{
   BPE_VERIFY(mpMemory_UT != NULL, false, "Couldn't allocate memory!");
}

//----------------------------------------------------------------------------

CStaticIndexBuffer::CStaticIndexBuffer(std::vector<uint16> const & indices)
:  CIndexBuffer(kIT_Uint16, RenderBackend()->AllocFixed(2 * indices.size(), 4, kRM_System, kRM_Video), indices.size() )
{
   BPE_VERIFY(mpMemory_UT != NULL, false, "Couldn't allocate memory!");
   memcpy(mpMemory_UT->mpAddress, &indices[0], 2 * indices.size());
}

//----------------------------------------------------------------------------

CStaticIndexBuffer::CStaticIndexBuffer(std::vector<uint32> const & indices)
:  CIndexBuffer(kIT_Uint32, RenderBackend()->AllocFixed(4 * indices.size(), 4, kRM_System, kRM_Video), indices.size() )
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
:  CIndexBuffer(kIT_Invalid, NULL, 0)
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
   mCount = indexCount;

   mBufferSize = indexSize[mType] * indexCount;

   mLastUsedExtraMemory = (mLastUsedExtraMemory+1)%kMaxExtraMemory;
   if( mpExtraMemory_UT[mLastUsedExtraMemory] == NULL ) 
   {
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
,  mPeak(0)
{
   mIndexBuffer.Lock(kIT_Uint16, indexCount);
   mPeakArea[0] = '\0';
}

//----------------------------------------------------------------------------

void CDynamicIndexBufferPool_RT::FrameReset()
{
   if (mCurrentIndexOffset > mPeak)
   {
      mPeak = mCurrentIndexOffset;
      strcpy(mPeakArea, RenderBackend()->GetCurrentAreaDebugName());
   }
   mCurrentIndexOffset = 0;
   mIndexBuffer.Lock(kIT_Uint16, mTotalIndexCount);
}

//----------------------------------------------------------------------------

CIndexBufferChunk const CDynamicIndexBufferPool_RT::AllocChunk(int const count)
{
   int const newIndexOffset = mCurrentIndexOffset + count;
   if( newIndexOffset < mTotalIndexCount )
   {
      CIndexBufferChunk const chunk( 
         &mIndexBuffer, 
         (uint16 *) (mIndexBuffer.mpExtraMemory_UT[mIndexBuffer.mLastUsedExtraMemory]->mpAddress + mCurrentIndexOffset * sizeof(uint16)),
         count );

      mCurrentIndexOffset = newIndexOffset;

      return chunk;
   }

   return CIndexBufferChunk();
}

//----------------------------------------------------------------------------

CDynamicIndexBufferPool_UT::CDynamicIndexBufferPool_UT(int const indexCount, int const chunkCount)
:  mIndexBuffer(kRRTU_Update)
,  mTotalIndexCount(indexCount)
,  mCurrentIndexOffset(0)
,  mPeak(0)
{
   mIndexBuffer.Lock(kIT_Uint16, indexCount);
   mPeakArea[0] = '\0';
}

//----------------------------------------------------------------------------

void CDynamicIndexBufferPool_UT::FrameReset()
{
   if (mCurrentIndexOffset > mPeak)
   {
      mPeak = mCurrentIndexOffset;
      strcpy(mPeakArea, RenderBackend()->GetCurrentAreaDebugName());
   }
   mCurrentIndexOffset = 0;
   mIndexBuffer.Lock(kIT_Uint16, mTotalIndexCount);
}

//----------------------------------------------------------------------------

CIndexBufferChunk const CDynamicIndexBufferPool_UT::AllocChunk(int const count)
{
   int const newIndexOffset = mCurrentIndexOffset + count;
   if( newIndexOffset < mTotalIndexCount )
   {
      CIndexBufferChunk const chunk( &mIndexBuffer, 
         (uint16 *) (mIndexBuffer.mpExtraMemory_UT[mIndexBuffer.mLastUsedExtraMemory]->mpAddress + mCurrentIndexOffset * sizeof(uint16)),
         count );

      mCurrentIndexOffset = newIndexOffset;
      return chunk;
   }

   return CIndexBufferChunk();
}
