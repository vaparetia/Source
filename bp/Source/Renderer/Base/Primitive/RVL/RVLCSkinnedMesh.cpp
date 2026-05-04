//----------------------------------------------------------------------------
// RVLCSkinnedMesh.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Renderer/Base/Primitive/CSkinnedMesh.h"
#include "Engine/Animation/CSkeleton.h"
#include "Engine/Streams/CMemoryInputStream.h"
#include "Engine/Graphics/CVertexArray.h"

//----------------------------------------------------------------------------

CSkinnedMesh::CSkinnedMesh(CVertexArray const & vertexArray, 
                           void const *pDisplayList, 
                           size_t const displayListSize,
                           TMeshChunks const & chunks,
                           TMaterials const & materials)
: CMesh( vertexArray, pDisplayList, displayListSize, chunks, materials, true )
, mVertexArray( vertexArray )
{
}

//----------------------------------------------------------------------------

CSkinnedMesh::~CSkinnedMesh()
{
}

//----------------------------------------------------------------------------

void CSkinnedMesh::FSkinnedMeshFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource)
{
   BPE_VERIFY( buildData.mpMemory != NULL, false, "must have memory" );
   CMemoryInputStream stream( buildData.mpMemory, buildData.mSize, CMemoryInputStream::kOwner_App );

   // read header
   BPE_VERIFY( stream.ReadUint32() == 'MODL', false, "Invalid model header" );

   // check version
   BPE_VERIFY( stream.ReadInt32() == 4, false, "Invalid version" );

   // read vertex array
   CVertexArray vertexArray( stream );

   // read materials
   std::vector< boost::shared_ptr<CShader> > materials;
   CMesh::ReadMaterials( &materials, stream );
   
   // read display list
   bpe::vector_s<uint8> displayList( stream );

   // read mesh chunks
   bpe::vector_s<CMeshChunk> meshChunks(stream);

   CSkinnedMesh *pSkinnedMesh = new CSkinnedMesh( vertexArray, &displayList[0], displayList.size(), meshChunks, materials );

   // Build skeleton
   uint32 const skelDataOffset = stream.GetReadPosition();
   uint32 const skelDataSize = stream.GetUnreadBufferLength();
   void *pSkelData = ((uint8*) buildData.mpMemory) + skelDataOffset;
   pSkinnedMesh->mpSkeleton.reset(new CSkeleton(pSkelData, (int) skelDataSize));

   returnResource.mpResource = pSkinnedMesh;

}