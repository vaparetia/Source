//----------------------------------------------------------------------------
// CMesh.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Renderer/Base/Primitive/CMesh.h"

//----------------------------------------------------------------------------

#include "Engine/GameObjectSystem/CGameObjectProperties.h"
#include "Engine/Graphics/CIndexArray.h"
#include "Engine/Graphics/CVertexArray.h"
#include "Engine/Resource/CResourceFactory.h"
#include "Engine/Streams/CMemoryInputStream.h"

#include "Renderer/Base/Primitive/CMeshBuffers.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/ShaderObjects/CShaderObjectFactory.h"

//----------------------------------------------------------------------------

CMesh::CMesh(CVertexArray const & vertexArray, 
             CIndexArray const & indexArray, 
             TMeshChunks const & chunks,
             TMaterials const & materials,
             bool const isSkinned,
             int const jointCount )
:  mMeshBuffers(new CMeshBuffers(vertexArray, indexArray, isSkinned))
,  mMaterials(materials)
,  mMeshChunks(chunks)
,  mBounds(CAABox::MakeMaxInvertedBox())
,  mJointCount( jointCount )
{
   for( int i = 0; i < mMeshChunks.size(); ++i )
   {
      mBounds.Include( mMeshChunks[i].mBounds );
   }

   // Generate material chunk table
   {
      int maxUnit = 0;
      
      for( int i = 0; i < mMeshChunks.size(); ++i )
      {
         int const chunkUnitIdx = mMeshChunks[i].mUnitIdx;

         if( chunkUnitIdx > maxUnit )
            maxUnit = chunkUnitIdx;
      }

      mMaterialChunkTable.resize(maxUnit + 1);

      for( int i = 0; i < mMeshChunks.size(); ++i )
      {
         mMaterialChunkTable[mMeshChunks[i].mUnitIdx]++;
      }

      int currentOffset = 0;

      for( int i = 0; i < mMaterialChunkTable.size(); ++i )
      {
         int const currentChunkCount = mMaterialChunkTable[i];
         mMaterialChunkTable[i] |= (currentOffset << 16);
         currentOffset += currentChunkCount;
      }
   }

   LinkMeshChunks();
}

//----------------------------------------------------------------------------

void CMesh::DeleteMesh(CMesh* pMesh)
{
   gpRenderBackend->DeleteRenderResourceFrameDelayed((IObject*)pMesh);
}

//----------------------------------------------------------------------------

CMesh::~CMesh()
{
}

//----------------------------------------------------------------------------

void CMesh::FMeshFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource)
{
   BPE_VERIFY( buildData.mpMemory != NULL, false, "must have memory" );
   CMemoryInputStream stream( buildData.mpMemory, buildData.mSize, CMemoryInputStream::kOwner_App );

   // read header
   unsigned int typeTest = stream.ReadUint32();
   BPE_VERIFY( typeTest == 'MODL', false, "Invalid model header" );

   // check version
   int version = stream.ReadInt32();
   bool vertexArrayInPlace = true;

   BPE_VERIFY( version == 2, false, "Invalid version" );

   // read vertex array
   CVertexArray* pVertexArray = CVertexArray::Load(stream);

   // read index array
   CIndexArray indexArray( stream );

   // read materials
   std::vector< boost::shared_ptr<CShader> > materials;
   {
      int const materialCount = stream.ReadInt32();
   }

   // read mesh chunks
   bpe::vector_s<CMeshChunk> meshChunks;
   {
      int const size = stream.ReadUint32();

      meshChunks.reserve( size );

      for( int i = 0; i < size; ++i )
      {
         meshChunks.push_back( CMeshChunk(stream, version) );
      }
   }

   int const jointCount = stream.ReadInt32();

   if( !indexArray.GetIndices().empty() )
   {
      CMesh* pMesh = new CMesh(*pVertexArray, indexArray, meshChunks, materials, false, jointCount );

#ifndef GOLD_VERSION
      pMesh->mDebugName = buildData.mResourceId;
#endif

      returnResource.mpResource = pMesh;
   }

   if( !vertexArrayInPlace )
   {
      delete pVertexArray;
   }
}

//----------------------------------------------------------------------------

void CMesh::GetChunkRangeForUnit(int const unit, int* chunkStart, int* chunkCount) const
{
   if( unit < mMaterialChunkTable.size() )
   {
      uint32 const offsetCount = mMaterialChunkTable[unit];
      *chunkStart = (offsetCount >> 16);
      *chunkCount = (offsetCount & 0xFFFF);
   }
   else
   {
      *chunkStart = 0;
      *chunkCount = 0;
   }
}

//----------------------------------------------------------------------------

int CMesh::GetChunkStartForUnit( int const unit ) const
{
   return mMaterialChunkTable[ unit ] >> 16;
}

//----------------------------------------------------------------------------

void CMesh::LinkMeshChunks()
{
   if ( GetMeshChunks().empty() )
   {
      return;
   }
   
   int currentStreamId = 0;
   CMeshChunk const *pPrior = &( mMeshChunks[0] );
   
   mMeshChunks[0].SetChunkUniqueStreamId( currentStreamId );

   for ( int meshChunkIndex = 1; meshChunkIndex < mMeshChunks.size(); ++meshChunkIndex )
   {
      CMeshChunk *pCurrent = &( mMeshChunks[meshChunkIndex] );

      if ( !CMeshChunk::AreChunksContinuous( *pPrior, *pCurrent ) )
      {
         ++currentStreamId;
      }

      pCurrent->SetChunkUniqueStreamId( currentStreamId );
      pPrior = pCurrent;
   }
}

//----------------------------------------------------------------------------

void CMesh::GetMeshChunkRange( SMeshChunkRange *pOutRange, int startIndex, int maxCount ) const
{
   uint32 vertexBufferMin = mMeshChunks[ startIndex ].mVertexBufferOffset;
   uint32 vertexBufferMax = mMeshChunks[ startIndex ].mVertexCount + vertexBufferMin; 

   pOutRange->mMaterialIndex = mMeshChunks[ startIndex ].mMaterialIndex;
   pOutRange->mPrimitiveType = mMeshChunks[ startIndex ].mPrimitiveType;
   pOutRange->mIndexBufferOffset = mMeshChunks[ startIndex ].mIndexBufferOffset;
   pOutRange->mIndicesCount = mMeshChunks[ startIndex ].mIndicesCount;

   if ( mMeshChunks[ startIndex ].mJointMap.empty() )
   {
      pOutRange->mJointMapPtr = NULL;
      pOutRange->mJointMapCount = 0;
   }
   else
   {
      pOutRange->mJointMapPtr = &( mMeshChunks[ startIndex ].mJointMap[0] );
      pOutRange->mJointMapCount = mMeshChunks[ startIndex ].mJointMap.size();
   }

   int const maxIndex = startIndex + maxCount;
   int const startStreamId = mMeshChunks[ startIndex ].GetChunkUniqueStreamId();
   
   
   int currentIndex = startIndex + 1;

   if ( startStreamId != -1 )
   {
      for ( ; currentIndex < mMeshChunks.size() && currentIndex < maxIndex; ++currentIndex )
      {
         if ( mMeshChunks[ currentIndex ].GetChunkUniqueStreamId() != startStreamId )
         {
            break;
         }

         pOutRange->mIndicesCount += mMeshChunks[ currentIndex ].mIndicesCount;

         uint32 const cntVertexBufferMin = mMeshChunks[ currentIndex ].mVertexBufferOffset;
         uint32 const cntVertexBufferMax = mMeshChunks[ currentIndex ].mVertexCount + cntVertexBufferMin;

         if ( cntVertexBufferMin < vertexBufferMin )
         {
            vertexBufferMin = cntVertexBufferMin;
         }

         if ( cntVertexBufferMax > vertexBufferMax )
         {
            vertexBufferMax = cntVertexBufferMax;
         }
      }
   }

   pOutRange->mChunkCount = currentIndex - startIndex;
   pOutRange->mVertexBufferOffset = vertexBufferMin;
   pOutRange->mVertexCount = vertexBufferMax - vertexBufferMin;
}