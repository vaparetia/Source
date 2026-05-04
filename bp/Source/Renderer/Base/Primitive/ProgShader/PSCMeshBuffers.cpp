//----------------------------------------------------------------------------
// CMeshBuffers.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Renderer/Base/Primitive/CMeshBuffers.h"

//----------------------------------------------------------------------------

#include "Engine/Graphics/CIndexArray.h"
#include "Engine/Graphics/CVertexArray.h"

#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/CShaderVertexDataBinding.h"

//----------------------------------------------------------------------------

namespace
{
   void CopyData(uint8 * pSource, uint8 * pDest, int const elementSize, int const numElements, int const destStride)
   {
      for( int i = 0; i < numElements; ++i )
      {
         memcpy(pDest, pSource, elementSize);
         pSource += elementSize;
         pDest += destStride;
      }
   }
}

//----------------------------------------------------------------------------

CMeshBuffers::CMeshBuffers(CVertexArray const & vertexArray, 
                           CIndexArray const & indexArray,
                           bool const /*isSkinned*/)
:  mIndices(indexArray.GetIndices())
,  mVertexCount(0)
,  mOriginalIndicesMaxIndexPlusOne( 0 )
{
   CVertexArray::Stream const * pPos = vertexArray.GetStream('POS0');
   CVertexArray::Stream const * pCol0 = vertexArray.GetStream('COL0');
   CVertexArray::Stream const * pCol1 = vertexArray.GetStream('COL1');
   CVertexArray::Stream const * pCol2 = vertexArray.GetStream('COL2');
   CVertexArray::Stream const * pTex0 = vertexArray.GetStream('TEX0');
   CVertexArray::Stream const * pTex1 = vertexArray.GetStream('TEX1');
   CVertexArray::Stream const * pTex2 = vertexArray.GetStream('TEX2');
   CVertexArray::Stream const * pNormal0 = vertexArray.GetStream('NRM0');
   CVertexArray::Stream const * pBoneIndices = vertexArray.GetStream('BONI');
   CVertexArray::Stream const * pBoneWeights = vertexArray.GetStream('BONW');

   // create single stream for positions
   mVertexBuffers.push_back(new CStaticVertexBuffer(pPos->GetDataPtr(), pPos->mSize));

   size_t const positionBufferIndex = mVertexData.AddBuffer( mVertexBuffers.back(), pPos->GetElementSize() );

   mVertexData.SetAttribute(kVDS_Position, 0, (EVertexDataType)pPos->mType, positionBufferIndex);

   int const vertexCount = pPos->GetElementCount();

   mVertexCount = vertexCount;

   // create interleaves stream for all other attributes
   int vertexSize = (pNormal0 ? pNormal0->GetElementSize() : 0) +
                    (pCol0 ? pCol0->GetElementSize() : 0) +
                    (pCol1 ? pCol1->GetElementSize() : 0) +
                    (pCol2 ? pCol2->GetElementSize() : 0) +
                    (pTex0 ? pTex0->GetElementSize() : 0) +
                    (pTex1 ? pTex1->GetElementSize() : 0) +
                    (pTex2 ? pTex2->GetElementSize() : 0) +
                    (pBoneIndices ? pBoneIndices->GetElementSize() : 0) + 
                    (pBoneWeights ? pBoneWeights->GetElementSize() : 0);

   if( vertexSize > 0 )
   {
      mVertexBuffers.push_back( new CStaticVertexBuffer(vertexSize * vertexCount) );
      
      CStaticVertexBuffer* pVertexBuffer = mVertexBuffers.back();

      size_t const everythingElseBufferIndex = mVertexData.AddBuffer( pVertexBuffer, vertexSize );

      uint8* pVertexData = (uint8*)pVertexBuffer->Lock();
      
      int currentOffset = 0;

      if( pNormal0 )
      {
         CopyData((uint8*)pNormal0->GetDataPtr(), pVertexData + currentOffset, pNormal0->GetElementSize(), vertexCount, vertexSize);
         mVertexData.SetAttribute(kVDS_Normal, currentOffset, (EVertexDataType)pNormal0->mType, everythingElseBufferIndex);
         currentOffset += pNormal0->GetElementSize();
      }

      EVertexDataType lastColorDataType = kVDT_Invalid;
      uint8 lastColorStreamElementSize = 0;

      if( pCol0 )
      {
         lastColorStreamElementSize = (uint8)pCol0->GetElementSize();
         lastColorDataType = (EVertexDataType)pCol0->mType;

         CopyData((uint8*)pCol0->GetDataPtr(), pVertexData + currentOffset, lastColorStreamElementSize, vertexCount, vertexSize);
         mVertexData.SetAttribute(kVDS_Color0, currentOffset, lastColorDataType, everythingElseBufferIndex);
         currentOffset += lastColorStreamElementSize;
      }

      if( pCol1 )
      {
         lastColorStreamElementSize = (uint8)pCol1->GetElementSize();
         lastColorDataType = (EVertexDataType)pCol1->mType;

         CopyData((uint8*)pCol1->GetDataPtr(), pVertexData + currentOffset, lastColorStreamElementSize, vertexCount, vertexSize);
         mVertexData.SetAttribute(kVDS_Color1, currentOffset, lastColorDataType, everythingElseBufferIndex);
         currentOffset += lastColorStreamElementSize;
      }
      else if( lastColorDataType != kVDT_Invalid )
      {
         mVertexData.SetAttribute(kVDS_Color1, currentOffset - lastColorStreamElementSize, lastColorDataType, everythingElseBufferIndex);
      }

      if( pCol2 )
      {
         lastColorStreamElementSize = (uint8)pCol2->GetElementSize();
         lastColorDataType = (EVertexDataType)pCol2->mType;

         CopyData((uint8*)pCol2->GetDataPtr(), pVertexData + currentOffset, lastColorStreamElementSize, vertexCount, vertexSize);
         mVertexData.SetAttribute(kVDS_Color2, currentOffset, lastColorDataType, everythingElseBufferIndex);
         currentOffset += lastColorStreamElementSize;
      }
      else if( lastColorDataType != kVDT_Invalid )
      {
         mVertexData.SetAttribute(kVDS_Color2, currentOffset - lastColorStreamElementSize, lastColorDataType, everythingElseBufferIndex);
      }

      EVertexDataType lastUVDataType = kVDT_Invalid;
      uint8 lastUVStreamElementSize = 0;

      if( pTex0 )
      {
         lastUVStreamElementSize = (uint8)pTex0->GetElementSize();
         lastUVDataType = (EVertexDataType)pTex0->mType;

         CopyData((uint8*)pTex0->GetDataPtr(), pVertexData + currentOffset, lastUVStreamElementSize, vertexCount, vertexSize);
         mVertexData.SetAttribute(kVDS_TexCoord0, currentOffset, lastUVDataType, everythingElseBufferIndex);
         currentOffset += lastUVStreamElementSize;
      }

      if( pTex1 )
      {
         lastUVStreamElementSize = (uint8)pTex1->GetElementSize();
         lastUVDataType = (EVertexDataType)pTex1->mType;

         CopyData((uint8*)pTex1->GetDataPtr(), pVertexData + currentOffset, lastUVStreamElementSize, vertexCount, vertexSize);
         mVertexData.SetAttribute(kVDS_TexCoord1, currentOffset, lastUVDataType, everythingElseBufferIndex);
         currentOffset += lastUVStreamElementSize;
      }
      else if( lastUVDataType != kVDT_Invalid )
      {
         mVertexData.SetAttribute(kVDS_TexCoord1, currentOffset - lastUVStreamElementSize, lastUVDataType, everythingElseBufferIndex);
      }

      if( pTex2 )
      {
         lastUVStreamElementSize = (uint8)pTex2->GetElementSize();
         lastUVDataType = (EVertexDataType)pTex2->mType;

         CopyData((uint8*)pTex2->GetDataPtr(), pVertexData + currentOffset, lastUVStreamElementSize, vertexCount, vertexSize);
         mVertexData.SetAttribute(kVDS_TexCoord2, currentOffset, lastUVDataType, everythingElseBufferIndex);
         currentOffset += lastUVStreamElementSize;
      }
      else if( lastUVDataType != kVDT_Invalid )
      {
         mVertexData.SetAttribute(kVDS_TexCoord2, currentOffset - lastUVStreamElementSize, lastUVDataType, everythingElseBufferIndex);
      }

      if( pBoneIndices )
      {
         CopyData((uint8*)pBoneIndices->GetDataPtr(), pVertexData + currentOffset, pBoneIndices->GetElementSize(), vertexCount, vertexSize);
         mVertexData.SetAttribute(kVDS_BlendIndices, currentOffset, (EVertexDataType)pBoneIndices->mType, everythingElseBufferIndex);
         currentOffset += pBoneIndices->GetElementSize();
      }

      if( pBoneWeights )
      {
         CopyData((uint8*)pBoneWeights->GetDataPtr(), pVertexData + currentOffset, pBoneWeights->GetElementSize(), vertexCount, vertexSize);
         mVertexData.SetAttribute(kVDS_BlendWeight, currentOffset, (EVertexDataType)pBoneWeights->mType, everythingElseBufferIndex);
         currentOffset += pBoneWeights->GetElementSize();
      }

      pVertexBuffer->Unlock();

   }

   // Vertex indices
   {
      CVertexArray::Stream const * pIndices = vertexArray.GetStream('OIDX');

      int const elementSize = sizeof(uint32);

      mpOriginalIndices.reset((uint32*)malloc(elementSize * mVertexCount));

      if( pIndices )
      {
         CopyData((uint8*)pIndices->GetDataPtr(), (uint8*)mpOriginalIndices.get(), elementSize, vertexCount, elementSize);

         uint32 const *originalIndices = mpOriginalIndices.get();

         // Find the highest index
         mOriginalIndicesMaxIndexPlusOne = 0;
         for ( size_t i = 0; i < vertexCount; ++i )
         {
            mOriginalIndicesMaxIndexPlusOne = bpe::max_val( mOriginalIndicesMaxIndexPlusOne, static_cast<size_t>( originalIndices[i] + 1 ) );
         }
      }
      else
      {
         memset(mpOriginalIndices.get(), 0, vertexCount * elementSize);
         mOriginalIndicesMaxIndexPlusOne = 1;
      }
   }
}

//----------------------------------------------------------------------------

CMeshBuffers::~CMeshBuffers()
{
   for(int i = 0; i < mVertexBuffers.size(); ++i )
      delete mVertexBuffers[i];

   mVertexBuffers.clear();
}

//----------------------------------------------------------------------------

