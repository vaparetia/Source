//----------------------------------------------------------------------------
// Win32CVertexData.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Win32CVertexData.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Primitive/CVertexBuffer.h"

//----------------------------------------------------------------------------

namespace
{
   D3DDECLTYPE gD3DTypeConversion[] =
   {
      D3DDECLTYPE_FLOAT2,
      D3DDECLTYPE_FLOAT3,
      D3DDECLTYPE_FLOAT4,

      D3DDECLTYPE_UBYTE4N,
      D3DDECLTYPE_UBYTE4,

      D3DDECLTYPE_FLOAT16_2,
      D3DDECLTYPE_FLOAT16_4,

      D3DDECLTYPE_SHORT2N,
      D3DDECLTYPE_SHORT4N,

      D3DDECLTYPE_DEC3N,

      D3DDECLTYPE_UNUSED,

      D3DDECLTYPE_SHORT2,
      D3DDECLTYPE_SHORT4
   };

   BPE_CTASSERT( BPE_ARRAY_SIZE(gD3DTypeConversion) == kVDT_Count );

   D3DVERTEXELEMENT9 const CreateVertexElement( WORD const stream, 
                                                WORD const offset,
                                                BYTE const type,
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

size_t CVertexData::AddBuffer( CVertexBuffer const *pBuffer, uint32 stride )
{
   mVertexBuffers.push_back( SVertexStream( pBuffer, stride, 0 ) );
   return mVertexBuffers.size() - 1;
}

//----------------------------------------------------------------------------

size_t CVertexData::AddBuffer( CDynamicVertexBufferPoolChunk_UT const * pChunk, uint32 stride )
{
   if ( pChunk )
   {
      CVertexBuffer const *pBuffer = pChunk->GetPool()->GetVertexBuffer();

      mVertexBuffers.push_back(SVertexStream(pBuffer, stride, pChunk->GetOffset()));
      return mVertexBuffers.size() - 1;
   }
   else
   {
      return -1;
   }
}

//----------------------------------------------------------------------------

size_t CVertexData::AddBuffer( CDynamicVertexBufferPoolChunk_RT const * pChunk, uint32 stride )
{
   if ( pChunk )
   {
      CVertexBuffer const *pBuffer = pChunk->GetPool()->GetVertexBuffer();

      mVertexBuffers.push_back(SVertexStream(pBuffer, stride, pChunk->GetOffset()));
      return mVertexBuffers.size() - 1;
   }
   else
   {
      return -1;
   }
}

//----------------------------------------------------------------------------

void CVertexData::SetAttribute(EVertexDataStream const vertexStream, uint8 const offset, EVertexDataType const type, size_t const bufferIndex )
{
   if( bufferIndex != -1 )
   {
      mAttributes[vertexStream].mOffset = offset;
      mAttributes[vertexStream].mType = type;
      mAttributes[ vertexStream ].mStreamIndex = bufferIndex;
   }
   else
   {
      mAttributes[vertexStream].mOffset = 0;
      mAttributes[vertexStream].mType = kVDT_Invalid;
      mAttributes[vertexStream].mStreamIndex = 0;
   }
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
