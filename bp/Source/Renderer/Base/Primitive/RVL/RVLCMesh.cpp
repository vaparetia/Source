#include "stdafx.h"

//----------------------------------------------------------------------------

#include "RVLCMesh.h"
#include "RVLCMeshBuffers.h"
#include "Engine/Streams/CMemoryInputStream.h"
#include "Engine/Graphics/CVertexArray.h"
#include "Engine/GameObjectSystem/CGameObjectProperties.h"

//----------------------------------------------------------------------------

CMesh::CMesh(CVertexArray const & vertexArray, 
      void const *pDisplayList, 
      size_t const displayListSize,
      TMeshChunks const & chunks,
      TMaterials const & materials,
      bool const isSkinned)
: mMeshBuffers( new CMeshBuffers( vertexArray, pDisplayList, displayListSize ) )
, mMaterials( materials )
, mMeshChunks( chunks )
, mBounds(CAABox::MakeMaxInvertedBox())
{
   for( int i = 0; i < mMeshChunks.size(); ++i )
   {
      mBounds.Include( mMeshChunks[i].GetBounds() );
   }
}

//----------------------------------------------------------------------------

CMesh::~CMesh()
{
}

//----------------------------------------------------------------------------

void CMesh::ReadMaterials( std::vector< boost::shared_ptr< CShader > > *pMaterials, CInputStream &stream )
{
   int const materialCount = stream.ReadInt32();

   pMaterials->reserve(materialCount);

   for( int i = 0; i < materialCount; ++i )
   {
      // construct property data from array
      IGameObjectFactory &factory = RenderBackend()->ShaderObjectFactory();
      boost::shared_ptr<CGameObjectProperties> shaderObjectProperties( CGameObjectProperties::FGameObjectProperties( stream, &factory ) );

      BPE_VERIFY( !shaderObjectProperties->mComponentProperties.empty(), false, "Material doesn't have component properties" );
      if( !shaderObjectProperties->mComponentProperties.empty() )
      {
         boost::shared_ptr<CGameObjectComponentProperties const> shaderProperties = shaderObjectProperties->mComponentProperties.front();
         pMaterials->push_back(CShader::CreateMaterial(shaderProperties));
      }
   }

   BPE_VERIFY( materialCount == pMaterials->size(), false, "Materials loaded doesn't match materials requested" );

}

//----------------------------------------------------------------------------

void CMesh::FMeshFactory( SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource )
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
   ReadMaterials( &materials, stream );

   // read display list
   bpe::vector_s<uint8> displayList( stream );

   // read mesh chunks
   bpe::vector_s<CMeshChunk> meshChunks(stream);

   returnResource.mpResource = new CMesh(vertexArray, &displayList[0], displayList.size(), meshChunks, materials, false);
}