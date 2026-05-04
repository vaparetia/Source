//----------------------------------------------------------------------------
// X360CVertexData.h
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------
#include <xtl.h>

#include "Renderer/Base/Material/CShaderVertexDataBinding.h"
#include "Renderer/Base/Primitive/CVertexData.h"

//----------------------------------------------------------------------------

struct SVertexAttribute
{
   SVertexAttribute()
      : mOffset(0)
      , mType(kVDT_Invalid)
      , mStreamIndex(0)
   {
   }

   int8  mOffset;       // offset from beginning of vertex buffer to beginning of data
   int8  mStreamIndex;  // index of stream to use
   int16 mType;         // type of this attribute (EVertexDataType)
};

struct SVertexStream
{
   SVertexStream(CVertexBuffer const * pBuffer, uint32 const stride, uint32 const offset)
      : mpBuffer(pBuffer)
      , mStride(stride)
      , mOffset(offset)
   {
   }

   CVertexBuffer const *   mpBuffer;      // vertex buffer to bind for this stream
   uint32                  mStride;       // stride between elements
   uint32                  mOffset;       // offset from beginning of buffer
};

//----------------------------------------------------------------------------

class RENDERER_API CVertexData
{
public:
   typedef bpe::reserved_vector<D3DVERTEXELEMENT9, 17> TDeclaration;
   typedef bpe::reserved_vector<SVertexAttribute, kVDS_Count> TAttributes;
   typedef bpe::reserved_vector<SVertexStream, 16> TStreams;

public:
   CVertexData();

   void ClearCachedVertexDeclaration() { mpCachedVertexDecl = NULL; }
   void ClearVertexBuffers() { mVertexBuffers.clear(); }
   void SetAttribute(EVertexDataStream const vertexStream, uint8 const offset, uint8 const stride, EVertexDataType const type, CVertexBuffer const * pBuffer );
   void SetAttribute(EVertexDataStream const vertexStream, uint8 const offset, uint8 const stride, EVertexDataType const type, CDynamicVertexBufferPoolChunk_RT const * pChunk );
   void SetAttribute(EVertexDataStream const vertexStream, uint8 const offset, uint8 const stride, EVertexDataType const type, CDynamicVertexBufferPoolChunk_UT const * pChunk );
   
   int const GetStreamIndex(EVertexDataStream const attr) const { return mAttributes[attr].mStreamIndex; }
   uint32 const GetOffset(EVertexDataStream const attr) const { return mAttributes[attr].mOffset; }
   uint32 const GetStride(EVertexDataStream const attr) const { return mVertexBuffers[mAttributes[attr].mStreamIndex].mStride; }
   EVertexDataType const GetType(EVertexDataStream const attr) const { return (EVertexDataType)mAttributes[attr].mType; }

   void BuildDeclaration(CShaderVertexDataBinding const & vertexDataBinding, TDeclaration & output) const;

public:
   TAttributes mAttributes;
   TStreams    mVertexBuffers;
   mutable IDirect3DVertexDeclaration9* mpCachedVertexDecl;
};

//----------------------------------------------------------------------------


