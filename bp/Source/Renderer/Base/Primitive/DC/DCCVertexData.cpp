//----------------------------------------------------------------------------
// DCCVertexData.cpp
// Dreamcast — vertex data binding (mirrors VTACVertexData.cpp)
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Renderer/Base/Primitive/CVertexData.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Primitive/CVertexBuffer.h"

//----------------------------------------------------------------------------

CVertexData::CVertexData()
:  mAttributes(kVDS_Count, SVertexAttribute())
,  mValidStreamsBitMask(0)
{
}

//----------------------------------------------------------------------------

void CVertexData::SetAttribute(EVertexDataStream const vertexStream, uint8 const offset,
   EVertexDataType const type, size_t const bufferIndex)
{
   uint32 const bit = (1 << vertexStream);

   if( bufferIndex != (size_t)-1 )
      mValidStreamsBitMask |= bit;
   else
      mValidStreamsBitMask &= ~bit;

   SVertexAttribute & attr = mAttributes[vertexStream];
   attr.mBufferIndex = bufferIndex;
   attr.mOffset      = offset;
   attr.mType        = type;
}

//----------------------------------------------------------------------------

void CVertexData::SetAttributeBufferIndex(EVertexDataStream const vertexStream, size_t const bufferIndex)
{
   mAttributes[vertexStream].mBufferIndex = bufferIndex;
}

//----------------------------------------------------------------------------

size_t CVertexData::AddBuffer_Unsafe(void* pBuffer, uint32 stride)
{
   if( pBuffer == NULL )
      return (size_t)-1;

   mBuffers.push_back(SBuffer(pBuffer, stride));
   return mBuffers.size() - 1;
}

//----------------------------------------------------------------------------

size_t CVertexData::AddBuffer(CVertexBuffer const* pBuffer, uint32 stride)
{
   return AddBuffer_Unsafe(pBuffer ? pBuffer->mpMemory_RT->mpAddress : NULL, stride);
}

//----------------------------------------------------------------------------

size_t CVertexData::AddBuffer(CDynamicVertexBufferPoolChunk_UT const* pChunk, uint32 stride)
{
   return AddBuffer_Unsafe(pChunk->GetRenderMemory(), stride);
}

//----------------------------------------------------------------------------

size_t CVertexData::AddBuffer(CDynamicVertexBufferPoolChunk_RT const* pChunk, uint32 stride)
{
   return AddBuffer_Unsafe(pChunk->GetRenderMemory(), stride);
}
