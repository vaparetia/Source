//----------------------------------------------------------------------------
// RVLCVertexData.h
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
         :  mOffset(0)
         ,  mStride(0)
         ,  mIsValid( false )
         ,  mpBuffer(NULL)
      {
      }

      uint32                  mOffset;
      uint8                   mStride;
      uint8                   mIsValid;
      CVertexBuffer const *   mpBuffer;
   };

   typedef bpe::reserved_vector<SVertexAttribute, kVDU_LastValid + 1> TAttributes;

public:
   CVertexData(int baseVertexIndex = 0);

   void SetAttribute(EVertexDataUsage const attr, uint32 const offset, uint8 const stride, CVertexBuffer const * pBuffer )
   {
      mAttributes[attr].mOffset = offset;
      mAttributes[attr].mStride = stride;
      mAttributes[attr].mIsValid = true;
      mAttributes[attr].mpBuffer = pBuffer;
   }

   void ClearAttribute( EVertexDataUsage const attr )
   {
      mAttributes[ attr ] = SVertexAttribute();
   }

   void OverrideAttributes(CVertexData const & rhs);
   void OverrideStreams(CVertexBuffer const * pBuffer);

   bool   const IsValid  (EVertexDataUsage const attr) const { return bool( mAttributes[attr].mIsValid ); }
   uint32 const GetOffset(EVertexDataUsage const attr) const { return mAttributes[attr].mOffset; }
   uint32 const GetStride(EVertexDataUsage const attr) const { return mAttributes[attr].mStride; }

   SVertexAttribute const &GetAttribute( EVertexDataUsage const attr ) const { return mAttributes[attr]; }

public:
   int         mBaseVertexIndex;
   TAttributes mAttributes;
};

//----------------------------------------------------------------------------

