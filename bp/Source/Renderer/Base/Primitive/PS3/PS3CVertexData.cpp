//----------------------------------------------------------------------------
// PS3CVertexData.cpp
// Copyright 2006
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

void CVertexData::SetAttribute(EVertexDataStream const vertexStream, uint8 const offset, uint8 const stride, EVertexDataType const type, CVertexBuffer const * pBuffer)
{
   uint32 dataOffset = 0;
   uint32 dataLocation = 0;

   uint32 const bit = (1 < vertexStream);

   if( pBuffer != NULL )
   {
      BPE_ASSERT(pBuffer->mpMemory_RT != NULL, "Vertexbuffer hasn't been initialized yet");
      gpRenderBackend->GcmGetOffsetAndLocation(pBuffer->mpMemory_RT, &dataOffset, &dataLocation);
      mValidStreamsBitMask |= bit;
   }
   else
   {
      mValidStreamsBitMask &= ~bit;
   }
   
   SVertexAttribute & vertexAttr = mAttributes[vertexStream];
   vertexAttr.mVertexBuffer = dataOffset;
   vertexAttr.mVertexBufferLocation = dataLocation;
   vertexAttr.mOffset = offset;
   vertexAttr.mType = type;
   vertexAttr.mStride = stride;
}

//----------------------------------------------------------------------------

template< class Tx, class Ty > inline void SetAttributeTemplate(CVertexData* pVertexData, EVertexDataStream const vertexStream, uint8 const offset, uint8 const stride, EVertexDataType const type, Tx const * pChunk)
{
   Ty const * pPool = pChunk->GetPool();
   CVertexBuffer const * pBuffer = pPool->GetVertexBuffer();

   uint32 dataOffset = 0;
   uint32 dataLocation = 0;

   uint32 const bit = (1 < vertexStream);

   if( pBuffer != NULL )
   {
      BPE_ASSERT(pBuffer->mpMemory_RT != NULL, "Vertexbuffer hasn't been initialized yet");
      gpRenderBackend->GcmGetOffsetAndLocation(pBuffer->mpMemory_RT, &dataOffset, &dataLocation);
      pVertexData->mValidStreamsBitMask |= bit;
   }
   else
   {
       pVertexData->mValidStreamsBitMask &= ~bit;
   }

   CVertexData::SVertexAttribute & vertexAttr =  pVertexData->mAttributes[vertexStream];
   vertexAttr.mVertexBuffer = dataOffset + pChunk->GetOffset();
   vertexAttr.mVertexBufferLocation = dataLocation;
   vertexAttr.mOffset = offset;
   vertexAttr.mType = type;
   vertexAttr.mStride = stride;
}

//----------------------------------------------------------------------------

void CVertexData::SetAttribute(EVertexDataStream const vertexStream, uint8 const offset, uint8 const stride, EVertexDataType const type, CDynamicVertexBufferPoolChunk_RT const * pChunk)
{
   SetAttributeTemplate<CDynamicVertexBufferPoolChunk_RT, CDynamicVertexBufferPool_RT>(this, vertexStream, offset, stride, type, pChunk);
}

//----------------------------------------------------------------------------

void CVertexData::SetAttribute(EVertexDataStream const vertexStream, uint8 const offset, uint8 const stride, EVertexDataType const type, CDynamicVertexBufferPoolChunk_UT const * pChunk)
{
   SetAttributeTemplate<CDynamicVertexBufferPoolChunk_UT, CDynamicVertexBufferPool_UT>(this, vertexStream, offset, stride, type, pChunk);
}

//----------------------------------------------------------------------------

void CVertexData::SetAttribute(EVertexDataStream const vertexStream, uint8 const stride, EVertexDataType const type, uint32 const vertexBuffer, uint8 const vertexBufferLocation)
{
   SVertexAttribute & vertexAttr = mAttributes[vertexStream];
   vertexAttr.mVertexBuffer = vertexBuffer;
   vertexAttr.mVertexBufferLocation = vertexBufferLocation;
   vertexAttr.mOffset = 0;
   vertexAttr.mType = type;
   vertexAttr.mStride = stride;

   uint32 const bit = (1 < vertexStream);

   if( vertexBuffer )
      mValidStreamsBitMask |= bit;
   else
      mValidStreamsBitMask &= ~bit;
}

