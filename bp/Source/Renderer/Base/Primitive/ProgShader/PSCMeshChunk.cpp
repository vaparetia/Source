//----------------------------------------------------------------------------
// CMeshChunk.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Renderer/Base/Primitive/CMeshChunk.h"

//----------------------------------------------------------------------------

#include "Engine/Streams/CInputStream.h"

//----------------------------------------------------------------------------

CMeshChunk::CMeshChunk(CInputStream & stream, int const modelVersion)
:  mBounds(stream)
,  mMaterialIndex(stream.ReadInt16())
,  mPrimitiveType(static_cast<EPrimitive>(stream.ReadUint8()))
,  mVertexBufferOffset(stream.ReadUint32())
,  mVertexCount(stream.ReadUint32())
,  mIndexBufferOffset(stream.ReadUint32())
,  mIndicesCount(stream.ReadUint32())
,  mChunkUniqueStreamId( -1 )
,  mJointMap(stream)
{
   mVertColorMin = stream.ReadUint32();
   mVertColorMax = stream.ReadUint32();
   mAdditionalFlags = stream.ReadUint32();
   mUnitIdx = stream.ReadInt32();
   mPacketIdx = stream.ReadInt32();
}

//----------------------------------------------------------------------------

void CMeshChunk::PutTo(COutputStream & stream) const
{
   stream.Put(mBounds);
   stream.WriteInt16(mMaterialIndex);
   stream.WriteUint8(mPrimitiveType);
   stream.WriteUint32(mVertexBufferOffset);
   stream.WriteUint32(mVertexCount);
   stream.WriteUint32(mIndexBufferOffset);
   stream.WriteUint32(mIndicesCount);
   stream.Put(mJointMap);
   stream.WriteUint32(mVertColorMin);
   stream.WriteUint32(mVertColorMax);
   stream.WriteUint32(mAdditionalFlags);
   stream.WriteInt32(mUnitIdx);
   stream.WriteInt32(mPacketIdx);
}

//----------------------------------------------------------------------------

CMeshChunk::CMeshChunk(CAABox const & bounds,
                       int16 const materialIndex,
                       EPrimitive const primitive,
                       uint32 const vertexBufferOffset,
                       uint32 const vertexCount,
                       uint32 const indexBufferOffset, 
                       uint32 const indicesCount,
                       std::vector<uint32> const & jointMap,
                       uint32 const additionalFlags,
                       int32 const unitIdx,
                       int32 const packetIdx)
:  mBounds(bounds)
,  mMaterialIndex(materialIndex)
,  mPrimitiveType(primitive)
,  mVertexBufferOffset(vertexBufferOffset)
,  mVertexCount(vertexCount)
,  mIndexBufferOffset(indexBufferOffset)
,  mIndicesCount(indicesCount)
,  mJointMap(jointMap)
,  mVertColorMin(0x00000000)
,  mVertColorMax(0xFFFFFFFF)
,  mAdditionalFlags(additionalFlags)
,  mUnitIdx(unitIdx)
,  mChunkUniqueStreamId( -1 )
,  mPacketIdx(packetIdx)
{
}

//----------------------------------------------------------------------------

uint32 CMeshChunk::GetPrimitiveCount(EPrimitive type, uint32 indexCount)
{
   switch(type)
   {
   case kPrimitive_TriangleList:
      return indexCount / 3;
      break;
   case kPrimitive_TriangleStrip:
      return indexCount - 2;
      break;
   case kPrimitive_TriangleFan:
      return indexCount - 2;
      break;
   case kPrimitive_LineList:
      return indexCount / 2;
      break;
   case kPrimitive_LineStrip:
      return indexCount - 1;
      break;
   case kPrimitive_PointList:
      return indexCount;
      break;
   default:
      BPE_ASSERTA( "unsupported primitive type" );
      return 0;
      break;

   }
}

//----------------------------------------------------------------------------

bool CMeshChunk::AreChunksContinuous( CMeshChunk const &first, CMeshChunk const &second ) 
{
   if ( second.mPacketIdx != first.mPacketIdx + 1 )
   {
      return false;
   }

   if ( second.mUnitIdx != first.mUnitIdx )
   {
      return false;
   }

   if ( second.mMaterialIndex != first.mMaterialIndex )
   {
      return false;
   }

   if ( second.mPrimitiveType != first.mPrimitiveType )
   {
      return false;
   }

   if ( second.mIndexBufferOffset != first.mIndexBufferOffset + first.mIndicesCount )
   {
      return false;
   }
   
   if ( second.mAdditionalFlags != first.mAdditionalFlags )
   {
      return false;
   }

   if ( second.mJointMap != first.mJointMap )
   {
      return false;
   }

   return true;
}