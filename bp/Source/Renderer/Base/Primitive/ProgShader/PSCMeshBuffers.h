//----------------------------------------------------------------------------
// CMeshBuffers.h
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Primitive/ProgShader/CIndexBuffer.h"
#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Primitive/CVertexData.h"

//----------------------------------------------------------------------------

class CIndexArray;
class CVertexArray;
class CShaderVertexDataBinding;

//------------------------------------------------------------------------------------------

class RENDERER_API CMeshBuffers
{
   typedef bpe::reserved_vector<CStaticVertexBuffer*, kVDU_Count> TVertexBuffers;

public:
   explicit CMeshBuffers(CVertexArray const & vertexArray, 
      CIndexArray const & indexArray,
      bool const isSkinned);

   ~CMeshBuffers();

   CIndexBuffer const * GetIndexBuffer() const { return &mIndices; }

   void SetVertexData(CShaderVertexDataBinding const & vertexDataBinding) const
   {
      uint64 const declarationHash_1 = vertexDataBinding.GetHash();
      uint64 const declarationHash_2 = uint64(this);
      RenderBackend()->SetVertexData(vertexDataBinding, mVertexData, declarationHash_1, declarationHash_2);
   }

   CVertexData & VertexData() { return mVertexData; }

   int const GetVertexCount() const { return mVertexCount; }
   uint32 const * GetOriginalVertexIndices() const { return mpOriginalIndices.get(); } 
   
   // I did not want to call this "GetOriginalVertexIndexCount" or something like that because
   // I didn't want people to think that this was the size of GetOriginalVertexIndices
   size_t const GetMaxOriginalVertexIndexPlusOne() const { return mOriginalIndicesMaxIndexPlusOne; }

private:
   CVertexData                mVertexData;

   TVertexBuffers             mVertexBuffers;
   CStaticIndexBuffer         mIndices;

   int                        mVertexCount;
   boost::scoped_ptr<uint32>  mpOriginalIndices;

   size_t                     mOriginalIndicesMaxIndexPlusOne;

private:
   BPE_DISABLE_OBJECT_ASSIGN(CMeshBuffers);
};



