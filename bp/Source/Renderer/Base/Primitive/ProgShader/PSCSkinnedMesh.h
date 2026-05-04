//----------------------------------------------------------------------------
// CSkinnedMesh.h
// Copyright 2007
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "Renderer/Base/Primitive/CMesh.h"
#include "Engine/Graphics/CVertexArray.h"

//----------------------------------------------------------------------------

struct SFactoryResourceBuildData;
struct SFactoryReturnResource;

class CIndexArray;
class CSkeleton;
struct SSkinnedMesh_VertInfoSoA;

//----------------------------------------------------------------------------

class RENDERER_API CSkinnedMesh : public CMesh
{
public:
   CSkinnedMesh(CVertexArray const & vertexArray, CIndexArray const & indexArray, TMeshChunks const & chunks, TMaterials const & materials);
   virtual ~CSkinnedMesh();

   static void FSkinnedMeshFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource);

public:
   CVertexArray                     mVertexArray;
   boost::scoped_ptr<CSkeleton>     mpSkeleton;
   SSkinnedMesh_VertInfoSoA *       mpSkinVerts;
};

