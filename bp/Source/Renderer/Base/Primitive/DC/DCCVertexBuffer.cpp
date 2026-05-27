//----------------------------------------------------------------------------
// DCCVertexBuffer.cpp
// Dreamcast — malloc-backed vertex buffer (no PVR VRAM pool for Phase 7)
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

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

CRenderHWAllocator::SHandle const * CStaticVertexBuffer::Alloc(int const size, int const alignment,
   ERenderMemory const memoryType, ERenderMemory const fallbackMemory)
{
   return gpRenderBackend->AllocFixed(size, alignment, memoryType, fallbackMemory);
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

CDynamicVertexBuffer::CDynamicVertexBuffer(ERenderResourceThreadUser const modifyThread)
:  CVertexBuffer(NULL)
,  mGlobalListIndex(-1)
,  mModifyThread(modifyThread)
,  mLastUsedExtraMemory(-1)
{
   for( int ii = 0; ii < kMaxExtraMemory; ++ii )
      mpExtraMemory_UT[ii] = NULL;
}

//----------------------------------------------------------------------------

CDynamicVertexBuffer::~CDynamicVertexBuffer()
{
   for( int ii = 0; ii < kMaxExtraMemory; ++ii )
      if( mpExtraMemory_UT[ii] )
         RenderBackend()->Free(mpExtraMemory_UT[ii]);
}

//----------------------------------------------------------------------------

void* CDynamicVertexBuffer::Lock(int const size)
{
   mLastUsedExtraMemory = (mLastUsedExtraMemory + 1) % kMaxExtraMemory;
   if( mpExtraMemory_UT[mLastUsedExtraMemory] == NULL )
      mpExtraMemory_UT[mLastUsedExtraMemory] = RenderBackend()->AllocFixed(size, 4, kRM_System, kRM_Video);
   if( mModifyThread == kRRTU_Render )
      mpMemory_RT = mpExtraMemory_UT[mLastUsedExtraMemory];
   return mpExtraMemory_UT[mLastUsedExtraMemory]->mpAddress;
}

//----------------------------------------------------------------------------

void CDynamicVertexBuffer::FrameReset()
{
   mpMemory_RT = mpExtraMemory_UT[mLastUsedExtraMemory];
}

//----------------------------------------------------------------------------

CDynamicVertexBufferPool_RT::CDynamicVertexBufferPool_RT(int const bufferSize, int const /*chunkCount*/)
:  mVertexBuffer(kRRTU_Render)
,  mSize(bufferSize)
,  mCurrentOffset(0)
,  mPeak(0)
{
   mPeakArea[0] = '\0';
}

//----------------------------------------------------------------------------

void CDynamicVertexBufferPool_RT::FrameReset()
{
   mCurrentOffset = 0;
}

//----------------------------------------------------------------------------

CDynamicVertexBufferPoolChunk_RT* CDynamicVertexBufferPool_RT::AllocChunk(int const /*size*/)
{
   return NULL;
}

//----------------------------------------------------------------------------

CDynamicVertexBufferPool_UT::CDynamicVertexBufferPool_UT(int const bufferSize, int const /*chunkCount*/)
:  mVertexBuffer(kRRTU_Update)
,  mSize(bufferSize)
,  mCurrentOffset(0)
,  mPeak(0)
{
   mPeakArea[0] = '\0';
}

//----------------------------------------------------------------------------

void CDynamicVertexBufferPool_UT::FrameReset()
{
   mCurrentOffset = 0;
}

//----------------------------------------------------------------------------

CDynamicVertexBufferPoolChunk_UT* CDynamicVertexBufferPool_UT::AllocChunk(int const /*size*/)
{
   return NULL;
}
