//----------------------------------------------------------------------------
// Win32CVertexData.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "X360CVertexData.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Primitive/CVertexBuffer.h"

//----------------------------------------------------------------------------

namespace
{
   //NOTE: D3DDECLTYPE must be int size on 360
   int gD3DTypeConversion[] =
   {
      D3DDECLTYPE_FLOAT2,
      D3DDECLTYPE_FLOAT3,
      D3DDECLTYPE_FLOAT4,

      //(custom to match ps3's order)
      MAKED3DDECLTYPE(GPUVERTEXFORMAT_8_8_8_8, GPUENDIAN_8IN32, GPUVERTEXSIGN_UNSIGNED, GPUNUMFORMAT_FRACTION, GPUSWIZZLE_RGBA),//D3DDECLTYPE_UBYTE4N
      MAKED3DDECLTYPE(GPUVERTEXFORMAT_8_8_8_8, GPUENDIAN_8IN32, GPUVERTEXSIGN_UNSIGNED, GPUNUMFORMAT_INTEGER, GPUSWIZZLE_RGBA),//D3DDECLTYPE_UBYTE4

      D3DDECLTYPE_FLOAT16_2,
      D3DDECLTYPE_FLOAT16_4,

      D3DDECLTYPE_SHORT2N,
      D3DDECLTYPE_SHORT4N,

      D3DDECLTYPE_HEND3N,

      D3DDECLTYPE_UNUSED,

      D3DDECLTYPE_SHORT2,
      D3DDECLTYPE_SHORT4,
   };

   BPE_CTASSERT( BPE_ARRAY_SIZE(gD3DTypeConversion) == kVDT_Count );

   D3DVERTEXELEMENT9 const CreateVertexElement( WORD const stream, 
      WORD const offset,
      DWORD const type,//NOTE: D3DDECLTYPE must be int size on 360
      BYTE const method,
      BYTE const usage,
      BYTE const usageIndex )
   {
      D3DVERTEXELEMENT9 const element =
      {
         stream,
         offset,
         type,
         method,
         usage,
         usageIndex
      };

      return element;
   }
}

//----------------------------------------------------------------------------

CVertexData::CVertexData()
:  mAttributes(kVDS_Count, SVertexAttribute())
,  mpCachedVertexDecl(NULL)
{
}

//----------------------------------------------------------------------------

void CVertexData::SetAttribute(EVertexDataStream const vertexStream, uint8 const offset, uint8 const stride, EVertexDataType const type, CVertexBuffer const * pBuffer )
{
   if( pBuffer )
   {
      mAttributes[vertexStream].mOffset = offset;
      mAttributes[vertexStream].mType = type;

      // See if vertex buffer already added.
      for( int i = 0; i < mVertexBuffers.size(); ++i )
      {
         if( mVertexBuffers[i].mpBuffer == pBuffer )
         {
            mAttributes[vertexStream].mStreamIndex = i;
            return;
         }
      }

      mAttributes[vertexStream].mStreamIndex = mVertexBuffers.size();
      mVertexBuffers.push_back(SVertexStream(pBuffer, stride, 0));
   }
   else
   {
      mAttributes[vertexStream].mOffset = 0;
      mAttributes[vertexStream].mType = kVDT_Invalid;
      mAttributes[vertexStream].mStreamIndex = 0;
   }
}

//----------------------------------------------------------------------------

template< class Tx, class Ty > inline void SetAttributeTemplate(CVertexData* pVertexData, EVertexDataStream const vertexStream, uint8 const offset, uint8 const stride, EVertexDataType const type, Tx const * pChunk )
{
   if( pChunk )
   {
      Ty const * pPool = pChunk->GetPool();
      CVertexBuffer const * pBuffer = pPool->GetVertexBuffer();

      pVertexData->mAttributes[vertexStream].mOffset = offset;
      pVertexData->mAttributes[vertexStream].mType = type;

      // See if vertex buffer already added.
      for( int i = 0; i < pVertexData->mVertexBuffers.size(); ++i )
      {
         if( pVertexData->mVertexBuffers[i].mpBuffer == pBuffer )
         {
            pVertexData->mAttributes[vertexStream].mStreamIndex = i;
            return;
         }
      }

      pVertexData->mAttributes[vertexStream].mStreamIndex = pVertexData->mVertexBuffers.size();
      pVertexData->mVertexBuffers.push_back(SVertexStream(pBuffer, stride, pChunk->GetOffset()));
   }
   else
   {
      pVertexData->mAttributes[vertexStream].mOffset = 0;
      pVertexData->mAttributes[vertexStream].mType = kVDT_Invalid;
      pVertexData->mAttributes[vertexStream].mStreamIndex = 0;
   }
}

void CVertexData::SetAttribute(EVertexDataStream const vertexStream, uint8 const offset, uint8 const stride, EVertexDataType const type, CDynamicVertexBufferPoolChunk_RT const * pChunk )
{
   SetAttributeTemplate<CDynamicVertexBufferPoolChunk_RT, CDynamicVertexBufferPool_RT>(this, vertexStream, offset, stride, type, pChunk);
}

void CVertexData::SetAttribute(EVertexDataStream const vertexStream, uint8 const offset, uint8 const stride, EVertexDataType const type, CDynamicVertexBufferPoolChunk_UT const * pChunk )
{
   SetAttributeTemplate<CDynamicVertexBufferPoolChunk_UT, CDynamicVertexBufferPool_UT>(this, vertexStream, offset, stride, type, pChunk);
}

//----------------------------------------------------------------------------

void CVertexData::BuildDeclaration(CShaderVertexDataBinding const & vertexDataBinding, TDeclaration & output) const
{
   for(int streamIndex = 0; streamIndex < mVertexBuffers.size(); ++streamIndex )
   {
      for( int usage = 0; usage < kVDU_Count; ++usage )
      {
         if( !vertexDataBinding.HasStream((EVertexDataUsage)usage) )
            continue;

         EVertexDataStream const vertexStream = vertexDataBinding.GetStream((EVertexDataUsage)usage);

         if( mAttributes[vertexStream].mStreamIndex != streamIndex )
            continue;

         EVertexDataType const type = (EVertexDataType)mAttributes[vertexStream].mType;
         
         if( type == kVDT_Invalid )
            continue;

         int d3dUsage = -1;
         int d3dUsageIndex = 0;

         switch(usage)
         {
         case kVDU_Position:
            d3dUsage = D3DDECLUSAGE_POSITION;
            break;

         case kVDU_BlendWeight:
            d3dUsage = D3DDECLUSAGE_BLENDWEIGHT;
            break;

         case kVDU_BlendIndices:
            d3dUsage = D3DDECLUSAGE_BLENDINDICES;
            break;

         case kVDU_Normal:
            d3dUsage = D3DDECLUSAGE_NORMAL;
            break;

         case kVDU_TexCoord0:
         case kVDU_TexCoord1:
         case kVDU_TexCoord2:
         case kVDU_TexCoord3:
         case kVDU_TexCoord4:
         case kVDU_TexCoord5:
            d3dUsage = D3DDECLUSAGE_TEXCOORD;
            d3dUsageIndex = usage - kVDU_TexCoord0;
            break;

         case kVDU_Tangent:
            d3dUsage = D3DDECLUSAGE_TANGENT;
            break;

         case kVDU_Binormal:
            d3dUsage = D3DDECLUSAGE_BINORMAL;
            break;

         case kVDU_Color0:
         case kVDU_Color1:
            d3dUsage = D3DDECLUSAGE_COLOR;
            d3dUsageIndex = usage - kVDU_Color0;
            break;
         }

         int const offset = mAttributes[vertexStream].mOffset;

         output.push_back(CreateVertexElement(streamIndex, offset, gD3DTypeConversion[type], D3DDECLMETHOD_DEFAULT, d3dUsage, d3dUsageIndex));

      }
   }
}
