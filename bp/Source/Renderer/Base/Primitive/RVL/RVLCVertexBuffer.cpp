//----------------------------------------------------------------------------
// PS3CVertexBuffer.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "RVLCVertexBuffer.h"
#include <revolution/gx.h>

//----------------------------------------------------------------------------

CVertexBuffer::CVertexBuffer(CRenderHWAllocator::SHandle const * pMemory)
:  mpMemory(pMemory)
{
}
                 
//----------------------------------------------------------------------------

CVertexBuffer::~CVertexBuffer()
{
   if( mpMemory ) 
      RenderBackend()->Free(mpMemory);
}
                 
//----------------------------------------------------------------------------

CStaticVertexBuffer::CStaticVertexBuffer(int const size)
:  CVertexBuffer( RenderBackend()->AllocFixed(size, kRM_Video, kRM_System) )
{
   BPE_VERIFY(mpMemory != NULL, false, "Couldn't allocate memory!");
}

//----------------------------------------------------------------------------

void* CStaticVertexBuffer::Lock()
{
   return mpMemory->mpAddress;
}

//----------------------------------------------------------------------------

CDynamicVertexBuffer::CDynamicVertexBuffer()
:  CVertexBuffer(NULL)
,  mBufferSize( 0 )
{
}

//----------------------------------------------------------------------------

void * CDynamicVertexBuffer::Lock(int const vertexStride, int const vertexCount)
{
   int byteSize = vertexStride * vertexCount;
   if ( byteSize > mBufferSize )
   {
      if ( mpMemory != NULL )
      {
         RenderBackend()->Free( mpMemory );
      }

      mBufferSize = OSRoundUp32B( byteSize );
      mpMemory = RenderBackend()->AllocFixed( mBufferSize, kRM_Video, kRM_System );
   }

   return mpMemory->mpAddress;
}

//----------------------------------------------------------------------------

void CDynamicVertexBuffer::Unlock()
{
   if ( mBufferSize > 0 )
   {
      DCFlushRange( mpMemory->mpAddress, mBufferSize );
      GXInvalidateVtxCache();
   }
}
