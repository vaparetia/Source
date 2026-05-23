//----------------------------------------------------------------------------
// PS3CVertexData.h
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/Material/CShaderVertexDataBinding.h"

//----------------------------------------------------------------------------

class RENDERER_API CVertexData
{
public:
   struct SVertexAttribute
   {
      SVertexAttribute()
      :  mVertexBuffer(0)
      ,  mVertexBufferLocation(0)
      ,  mOffset(0)
      ,  mType(kVDT_Invalid)
      ,  mStride(0)
      {
      }

      uint32   mVertexBuffer;          // vertex buffer (offset relative to memory base address for location)
      uint32   mVertexBufferLocation;  // location of vertex buffer (CELL_GCM_LOCATION_MAIN/CELL_GCM_LOCATION_LOCAL)
      uint32   mOffset;                // offset to first vertex from beginning of stream
      uint16   mType;                  // type of vertex data
      uint16   mStride;                // stride of vertex data
   };

   typedef bpe::reserved_vector<SVertexAttribute, kVDS_Count> TAttributes;

public:
   CVertexData();

   void ClearCachedVertexDeclaration() {}
   void ClearVertexBuffers() {}
   void SetAttribute(EVertexDataStream const vertexStream, uint8 const offset, uint8 const stride, EVertexDataType const type, CVertexBuffer const * pBuffer);
   void SetAttribute(EVertexDataStream const vertexStream, uint8 const offset, uint8 const stride, EVertexDataType const type, CDynamicVertexBufferPoolChunk_RT const * pChunk );
   void SetAttribute(EVertexDataStream const vertexStream, uint8 const offset, uint8 const stride, EVertexDataType const type, CDynamicVertexBufferPoolChunk_UT const * pChunk );
   void SetAttribute(EVertexDataStream const vertexStream, uint8 const stride, EVertexDataType const type, uint32 const vertexBuffer, uint8 const vertexBufferLocation);

   uint32 const GetOffset(EVertexDataStream const vertexStream) const { return mAttributes[vertexStream].mOffset; }
   uint32 const GetStride(EVertexDataStream const vertexStream) const { return mAttributes[vertexStream].mStride; }
   
public:
   TAttributes mAttributes;
   uint32      mValidStreamsBitMask;
};

//----------------------------------------------------------------------------



