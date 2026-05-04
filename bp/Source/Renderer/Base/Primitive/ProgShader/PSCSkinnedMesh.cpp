//----------------------------------------------------------------------------
// CSkinnedMesh.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Renderer/Base/Primitive/CSkinnedMesh.h"

//----------------------------------------------------------------------------

#include "Engine/Graphics/CIndexArray.h"
#include "Engine/Graphics/CVertexArray.h"
#include "Engine/Streams/CMemoryInputStream.h"
#include "Engine/Memory/CMemoryAllocator.h"
#include "Engine/Resource/CResourceFactory.h"

#include "Engine/GameObjectSystem/CGameObjectProperties.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

#include "Engine/Animation/CSkeleton.h"
#include "Renderer/Base/Primitive/SSkinnedMesh_VertInfoSoA.h"

//----------------------------------------------------------------------------
BPE_CTASSERT(sizeof(SSkinnedMesh_VertInfoSoA) == 64);

// Converts vertex array into SIMD friendly SOA data
static SSkinnedMesh_VertInfoSoA * build_soa_verts(CVertexArray const & vertexArray)
{

   uint32 numVerts = vertexArray.GetPositions().size();
   // Alloc 128 byte aligned buffer, padded to 128 bytes. This makes this data PS3 DMA friendly.
   SSkinnedMesh_VertInfoSoA * pAllocVerts = static_cast<SSkinnedMesh_VertInfoSoA *>(BPE_MALLOC_ALIGNED(128, sizeof(SSkinnedMesh_VertInfoSoA) * (numVerts + (numVerts & 1))));
   SSkinnedMesh_VertInfoSoA * pWorkingVerts = pAllocVerts;
   CVector3 const * pPosition = &vertexArray.GetPositions()[0];
   CVector3 const * pNormal = &vertexArray.GetNormals()[0];
   CVector3 const * pBinormal = &vertexArray.GetTangent()[0];
   CVector3 const * pSBinormal = &vertexArray.GetBinormal()[0];
   CVector4 const * pWeights = &vertexArray.GetBoneWeights()[0];
   uint32 const * pBoneIndices = reinterpret_cast<uint32 const*>(&vertexArray.GetBoneIndices()[0]);

   while(numVerts)
   {
      // Copy data into SoA format
      pWorkingVerts->mX[0] = (*pPosition)[0];
      pWorkingVerts->mY[0] = (*pPosition)[1];
      pWorkingVerts->mZ[0] = (*pPosition)[2];

      pWorkingVerts->mX[1] = (*pNormal)[0];
      pWorkingVerts->mY[1] = (*pNormal)[1];
      pWorkingVerts->mZ[1] = (*pNormal)[2];

      pWorkingVerts->mX[2] = (*pBinormal)[0];
      pWorkingVerts->mY[2] = (*pBinormal)[1];
      pWorkingVerts->mZ[2] = (*pBinormal)[2];

      pWorkingVerts->mX[3] = (*pSBinormal)[0];
      pWorkingVerts->mY[3] = (*pSBinormal)[1];
      pWorkingVerts->mZ[3] = (*pSBinormal)[2];

      pWorkingVerts->mWeights[0] = (*pWeights)[0];
      pWorkingVerts->mWeights[1] = (*pWeights)[1];
      pWorkingVerts->mWeights[2] = (*pWeights)[2];
      pWorkingVerts->mBoneIndices = *pBoneIndices;

      pWorkingVerts++;

      pPosition++;
      pNormal++;
      pBinormal++;
      pSBinormal++;
      pWeights++;
      pBoneIndices++;
      numVerts--;
   }

   return pAllocVerts;
}

//----------------------------------------------------------------------------

CSkinnedMesh::CSkinnedMesh(CVertexArray const & vertexArray, CIndexArray const & indexArray, TMeshChunks const & chunks, TMaterials const & materials)
:  CMesh(vertexArray, indexArray, chunks, materials, true)
,  mVertexArray(vertexArray)
{
   mpSkinVerts = build_soa_verts(mVertexArray);
}

//----------------------------------------------------------------------------

CSkinnedMesh::~CSkinnedMesh()
{
   BPE_FREE_ALIGNED(mpSkinVerts);
}

//----------------------------------------------------------------------------

void CSkinnedMesh::FSkinnedMeshFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource)
{
   BPE_VERIFY( buildData.mpMemory != NULL, false, "must have memory" );
   CMemoryInputStream stream( buildData.mpMemory, buildData.mSize, CMemoryInputStream::kOwner_App );

   // read header
   unsigned int typeTest=stream.ReadUint32();
   BPE_VERIFY( typeTest == 'MODL', false, "Invalid model header" );

   // check version
   int versionTest=stream.ReadInt32();
   BPE_VERIFY( versionTest == 4, false, "Invalid version" );

   // read vertex array
   CVertexArray vertexArray( stream );

   // read index array
   CIndexArray indexArray( stream );

   // read materials
   std::vector< boost::shared_ptr<CShader> > materials;
   {
      int const materialCount = stream.ReadInt32();

      materials.reserve(materialCount);

      for( int i = 0; i < materialCount; ++i )
      {
         // construct property data from array
         IGameObjectFactory &factory = RenderBackend()->ShaderObjectFactory();
         boost::shared_ptr<CGameObjectProperties> shaderObjectProperties( CGameObjectProperties::FGameObjectProperties( stream, &factory ) );

         if( !shaderObjectProperties->mComponentProperties.empty() )
         {
            boost::shared_ptr<CGameObjectComponentProperties const> shaderProperties = shaderObjectProperties->mComponentProperties.front();
            materials.push_back(CShader::CreateMaterial(shaderProperties));
         }
      }
   }

   // read mesh chunks
   bpe::vector_s<CMeshChunk> meshChunks(stream);
   CSkinnedMesh * pSkinnedMesh =  new CSkinnedMesh(vertexArray, indexArray, meshChunks, materials);

   // Build skeleton
   uint32 const skelDataOffset = stream.GetReadPosition();
   uint32 const skelDataSize = stream.GetUnreadBufferLength();
   void *pSkelData = ((uint8*) buildData.mpMemory) + skelDataOffset;
   pSkinnedMesh->mpSkeleton.reset(new CSkeleton(pSkelData, (int) skelDataSize));

   returnResource.mpResource = pSkinnedMesh;
}

//----------------------------------------------------------------------------
