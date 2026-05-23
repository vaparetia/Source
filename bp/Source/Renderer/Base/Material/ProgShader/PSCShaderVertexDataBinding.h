//----------------------------------------------------------------------------
// PSCShaderVertexDataBinding.h
// Bluepoint
// Copyright 2009
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

//----------------------------------------------------------------------------

enum EVertexDataUsage
{
   kVDU_Position     = 0,
   kVDU_BlendWeight  = 1,
   kVDU_Normal       = 2,
   kVDU_Color0       = 3,
   kVDU_Color1       = 4,

   kVDU_FogCoord     = 5,
   kVDU_PSize        = 6,

   kVDU_BlendIndices = 7,

   kVDU_TexCoord0    = 8,
   kVDU_TexCoord1    = 9,
   kVDU_TexCoord2    = 10,
   kVDU_TexCoord3    = 11,
   kVDU_TexCoord4    = 12,
   kVDU_TexCoord5    = 13,

   kVDU_Tangent      = 14,
   kVDU_Binormal     = 15,

   kVDU_Count,
   
   kVDU_Invalid      = -1
};

enum EVertexDataStream
{
   kVDS_Position,
   kVDS_Normal,
   kVDS_Color0,
   kVDS_Color1,
   kVDS_Color2,
   kVDS_TexCoord0,
   kVDS_TexCoord1,
   kVDS_TexCoord2,
   kVDS_TexCoord3,
   kVDS_TexCoord4,
   kVDS_BlendWeight,
   kVDS_BlendIndices,
   
   kVDS_Count
};

//----------------------------------------------------------------------------

class CShaderVertexDataBinding
{
public:
   explicit CShaderVertexDataBinding()
   :  mStreamIndices(0)
   ,  mAttributeValidMask(0)
   {
   }

   void Set( EVertexDataUsage const usage,  EVertexDataStream const stream )
   {
      uint64 const shift = (usage * 4);
      uint64 const mask = (uint64)0xF << shift;
      mStreamIndices = (mStreamIndices & ~mask) | uint64(stream) << shift;

      mAttributeValidMask |= 1 << usage;
   }

   int const HasStream(EVertexDataUsage const usage) const { return (int)(mAttributeValidMask >> usage) & 1; }
   EVertexDataStream const GetStream(EVertexDataUsage const usage) const { return (EVertexDataStream)((mStreamIndices >> (usage * 4)) & 0xF); }
   uint64 const InternalGetStreamIndices() const { return mStreamIndices; }
   uint16 const InternalGetAttributeMask() const { return mAttributeValidMask; }

   bool const operator == ( CShaderVertexDataBinding const & rhs ) const { return mStreamIndices == rhs.mStreamIndices && mAttributeValidMask == rhs.mAttributeValidMask; }
   bool const operator != ( CShaderVertexDataBinding const & rhs ) const { return mStreamIndices != rhs.mStreamIndices || mAttributeValidMask != rhs.mAttributeValidMask; }
   bool const operator < (CShaderVertexDataBinding const & rhs) const { return mStreamIndices < rhs.mStreamIndices || mStreamIndices == rhs.mStreamIndices && mAttributeValidMask < rhs.mAttributeValidMask; }
   bool const operator > (CShaderVertexDataBinding const & rhs) const { return mStreamIndices > rhs.mStreamIndices || mStreamIndices == rhs.mStreamIndices && mAttributeValidMask > rhs.mAttributeValidMask; }

   uint64 const GetHash() const { return mStreamIndices; }

private:
   // 4 bits for each stream
   uint64   mStreamIndices;
   // 1 bit for each stream
   uint16   mAttributeValidMask;
   
   uint16   _pad;
};

