//----------------------------------------------------------------------------
// RVLCMesh.h
// Copyright 2009
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "Renderer/Base/Primitive/CMeshChunk.h"
#include "Engine/Resource/CResourceFactory.h"

//----------------------------------------------------------------------------

class CVertexArray;
class CMeshBuffers;
class CShader;

//----------------------------------------------------------------------------

class RENDERER_API CMesh
{
public:
   typedef std::vector< boost::shared_ptr<CShader> > TMaterials;
   typedef std::vector<CMeshChunk> TMeshChunks;

public:
   CMesh(CVertexArray const & vertexArray, 
      void const *pDisplayList, 
      size_t const displayListSize,
      TMeshChunks const & chunks,
      TMaterials const & materials,
      bool const isSkinned);

   virtual ~CMesh();

   TMeshChunks const & GetMeshChunks() const { return mMeshChunks; }
   boost::shared_ptr<CShader> const & GetMaterial(int const index) const { return mMaterials[index]; }

   CAABox const & GetBounds() const { return mBounds; }

   CMeshBuffers const & GetMeshBuffers() const { return *mMeshBuffers; }

   static void ReadMaterials( std::vector< boost::shared_ptr< CShader > > *pMaterials, CInputStream &stream );
   static void FMeshFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource);

protected:
   boost::scoped_ptr<CMeshBuffers>  mMeshBuffers;
   TMaterials                       mMaterials;
   TMeshChunks                      mMeshChunks;
   CAABox                           mBounds;
};

