//----------------------------------------------------------------------------
// RVLCSkinnedMesh.h
// Copyright 2009
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "Renderer/Base/Primitive/CMesh.h"
#include "Engine/Graphics/CVertexArray.h"

//----------------------------------------------------------------------------

class CSkeleton;
struct SSkinnedMesh_VertInfoSoA;

//----------------------------------------------------------------------------

class RENDERER_API CSkinnedMesh : public CMesh
{
public:
   CSkinnedMesh(CVertexArray const & vertexArray, 
      void const *pDisplayList, 
      size_t const displayListSize,
      TMeshChunks const & chunks,
      TMaterials const & materials);

   virtual ~CSkinnedMesh();

   static void FSkinnedMeshFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &resource);

public:
   CVertexArray                     mVertexArray;
   boost::scoped_ptr<CSkeleton>     mpSkeleton;
//   SSkinnedMesh_VertInfoSoA *       mpSkinVerts;
};

