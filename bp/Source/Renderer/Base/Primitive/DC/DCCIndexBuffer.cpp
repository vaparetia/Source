//----------------------------------------------------------------------------
// DCCIndexBuffer.cpp
// Dreamcast — malloc-backed index buffer (mirrors VTACIndexBuffer.cpp)
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Renderer/Base/Primitive/CIndexBuffer.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderBackend.h"

//----------------------------------------------------------------------------

CIndexBuffer::CIndexBuffer(EIndexType type, CRenderHWAllocator::SHandle const * pMemory,
   size_t const indexCount)
:  mType(type)
,  mCount(indexCount)
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
:  CIndexBuffer(type, RenderBackend()->AllocFixed(GetSizeForIndexType(type) * indexCount, 4,
      kRM_System, kRM_Video), indexCount)
{
   BPE_VERIFY(mpMemory_UT != NULL, false, "Couldn't allocate memory!");
}

//----------------------------------------------------------------------------

CStaticIndexBuffer::CStaticIndexBuffer(std::vector<uint16> const & indices)
:  CIndexBuffer(kIT_Uint16,
      indices.empty() ? NULL : RenderBackend()->AllocFixed(2 * (int)indices.size(), 4, kRM_System, kRM_Video),
      indices.size())
{
   if( !indices.empty() )
   {
      BPE_VERIFY(mpMemory_UT != NULL, false, "Couldn't allocate memory!");
      memcpy(mpMemory_UT->mpAddress, &indices[0], 2 * indices.size());
   }
}

//----------------------------------------------------------------------------

CStaticIndexBuffer::CStaticIndexBuffer(std::vector<uint32> const & indices)
:  CIndexBuffer(kIT_Uint32,
      indices.empty() ? NULL : RenderBackend()->AllocFixed(4 * (int)indices.size(), 4, kRM_System, kRM_Video),
      indices.size())
{
   if( !indices.empty() )
   {
      BPE_VERIFY(mpMemory_UT != NULL, false, "Couldn't allocate memory!");
      memcpy(mpMemory_UT->mpAddress, &indices[0], 4 * indices.size());
   }
}

//----------------------------------------------------------------------------

void* CStaticIndexBuffer::Lock()
{
   return mpMemory_UT ? mpMemory_UT->mpAddress : NULL;
}

//----------------------------------------------------------------------------

void CStaticIndexBuffer::Unlock()
{
}

//----------------------------------------------------------------------------

CDynamicIndexBuffer::CDynamicIndexBuffer(ERenderResourceThreadUser const modifyThread)
:  CIndexBuffer(kIT_Invalid, NULL, 0)
,  mBufferSize(0)
,  mModifyThread(modifyThread)
,  mGlobalListIndex(-1)
,  mLastUsedExtraMemory(-1)
{
   for( int ii = 0; ii < kMaxExtraMemory; ++ii )
      mpExtraMemory_UT[ii] = NULL;
}

//----------------------------------------------------------------------------

CDynamicIndexBuffer::~CDynamicIndexBuffer()
{
   for( int ii = 0; ii < kMaxExtraMemory; ++ii )
      if( mpExtraMemory_UT[ii] )
         RenderBackend()->Free(mpExtraMemory_UT[ii]);
}

//----------------------------------------------------------------------------

void* CDynamicIndexBuffer::Lock(EIndexType const indexType, int const indexCount)
{
   static const int kIndexSize[] = { sizeof(uint32), sizeof(uint16) };
   mType        = indexType;
   mCount       = indexCount;
   mBufferSize  = kIndexSize[mType] * indexCount;
   mLastUsedExtraMemory = (mLastUsedExtraMemory + 1) % kMaxExtraMemory;
   if( mpExtraMemory_UT[mLastUsedExtraMemory] == NULL )
      mpExtraMemory_UT[mLastUsedExtraMemory] = RenderBackend()->AllocFixed(mBufferSize, 4, kRM_System, kRM_Video);
   if( mModifyThread == kRRTU_Render )
      mpMemory_RT = mpExtraMemory_UT[mLastUsedExtraMemory];
   return mpExtraMemory_UT[mLastUsedExtraMemory]->mpAddress;
}

//----------------------------------------------------------------------------

void CDynamicIndexBuffer::FrameReset()
{
   mpMemory_RT = mpExtraMemory_UT[mLastUsedExtraMemory];
}

//----------------------------------------------------------------------------

CDynamicIndexBufferPool_RT::CDynamicIndexBufferPool_RT(int const indexCount, int const /*chunkCount*/)
:  mIndexBuffer(kRRTU_Render)
,  mTotalIndexCount(indexCount)
,  mCurrentIndexOffset(0)
,  mPeak(0)
{
   mPeakArea[0] = '\0';
}

//----------------------------------------------------------------------------

void CDynamicIndexBufferPool_RT::FrameReset()
{
   mCurrentIndexOffset = 0;
}

//----------------------------------------------------------------------------

CIndexBufferChunk const CDynamicIndexBufferPool_RT::AllocChunk(int const /*count*/)
{
   return CIndexBufferChunk();
}

//----------------------------------------------------------------------------

CDynamicIndexBufferPool_UT::CDynamicIndexBufferPool_UT(int const indexCount, int const /*chunkCount*/)
:  mIndexBuffer(kRRTU_Update)
,  mTotalIndexCount(indexCount)
,  mCurrentIndexOffset(0)
,  mPeak(0)
{
   mPeakArea[0] = '\0';
}

//----------------------------------------------------------------------------

void CDynamicIndexBufferPool_UT::FrameReset()
{
   mCurrentIndexOffset = 0;
}

//----------------------------------------------------------------------------

CIndexBufferChunk const CDynamicIndexBufferPool_UT::AllocChunk(int const /*count*/)
{
   return CIndexBufferChunk();
}
