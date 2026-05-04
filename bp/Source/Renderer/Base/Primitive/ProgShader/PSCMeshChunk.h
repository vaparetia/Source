//----------------------------------------------------------------------------
// CMeshChunk.h
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "Engine/Primitives/CAABox.h"

//----------------------------------------------------------------------------

class CMesh;
class COverridenMeshBuffers;
class CShaderVertexDataBinding;

//----------------------------------------------------------------------------

class RENDERER_API CMeshChunk
{
public:
   enum EPrimitive
   {
      kPrimitive_TriangleList,
      kPrimitive_TriangleStrip,
      kPrimitive_TriangleFan,
      kPrimitive_LineList,
      kPrimitive_LineStrip,
      kPrimitive_PointList,

      kPrimitive_Count,

      kPrimitive_Invalid = -1,
   };

   enum EAdditionalFlags
   {
      kAF_EnableCulling          =  (1 << 0)
   };

public:
   explicit CMeshChunk(CInputStream & stream, int modelVersion);
   void PutTo(COutputStream & stream) const;

   explicit CMeshChunk(CAABox const & bounds,
                       int16 const materialIndex,
                       EPrimitive const primitive,
                       uint32 const vertexBufferOffset,
                       uint32 const vertexCount,
                       uint32 const indexBufferOffset, 
                       uint32 const indicesCount,
                       std::vector<uint32> const & jointMap,
                       uint32 const additionalFlags,
                       int32 const unitIdx,
                       int32 const packetIdx);

   static uint32 GetPrimitiveCount(EPrimitive type, uint32 indexCount);

   CAABox const &GetBounds() const { return mBounds; }
   int16 const   GetMaterialIndex() const { return mMaterialIndex; }

   int32 const   GetChunkUniqueStreamId() const { return mChunkUniqueStreamId; }
   void          SetChunkUniqueStreamId( int i ) { mChunkUniqueStreamId = i; }

   static bool   AreChunksContinuous( CMeshChunk const &first, CMeshChunk const &second );
public:
   CAABox      mBounds;

   int16       mMaterialIndex;
   
   EPrimitive  mPrimitiveType;
   uint32      mVertexBufferOffset;
   uint32      mVertexCount;
   uint32      mIndexBufferOffset;
   uint32      mIndicesCount;

   uint32      mVertColorMin; // contains minimum of each color component used by this mesh chunk (in RGBA format)
   uint32      mVertColorMax; // contains maximum of each color component used by this mesh chunk (in RGBA format)

   uint32      mAdditionalFlags;
   
   int32       mUnitIdx;
   int32       mPacketIdx;
   
private:
   int32       mChunkUniqueStreamId;

public:
   // this maps local joint indices to mesh global joint indices
   bpe::vector_s<uint32> mJointMap;
};

struct SMeshChunkRange
{
   int16 mMaterialIndex;
   CMeshChunk::EPrimitive mPrimitiveType;
   uint32 const *mJointMapPtr;
   uint32 mJointMapCount;
   uint32 mIndexBufferOffset;
   uint32 mIndicesCount;
   uint32 mVertexBufferOffset;
   uint32 mVertexCount;

   uint32 mChunkCount;
};