//----------------------------------------------------------------------------
// RVLCMeshChunk.cpp
// Copyright 2009
//----------------------------------------------------------------------------

#include "stdafx.h"

#include "RVLCMesh.h"
#include "RVLCMeshChunk.h"
#include "RVLCMeshBuffers.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include <revolution/gx.h>
//----------------------------------------------------------------------------

CMeshChunk::CMeshChunk(CInputStream & stream)
: mBounds( stream )
, mVtxDesc( stream )
, mDisplayListOffset( stream.ReadUint32() )
, mDisplayListSize( stream.ReadUint16() )
, mMaterialIndex( stream.ReadInt16() )
{
}

//----------------------------------------------------------------------------

CMeshChunk::CMeshChunk(CAABox const & bounds,
                       CShaderVertexDataBinding const &desc,
                       uint32 const displayListOffset,
                       uint16 const displayListSize,
                       int16 const materialIndex )
: mBounds( bounds )
, mVtxDesc( desc )
, mDisplayListOffset( displayListOffset )
, mDisplayListSize( displayListSize )
, mMaterialIndex( materialIndex )
{
}

//----------------------------------------------------------------------------

void CMeshChunk::Draw(CShaderVertexDataBinding const & vertexDataBinding, CMesh const & mesh, COverridenMeshBuffers const * pOverridenBuffers) const
{
   gpRenderBackend->SetVertexDescriptor( mVtxDesc );

   mesh.GetMeshBuffers().SetCurrent();

   if ( pOverridenBuffers )
   {
      gpRenderBackend->SetVertexData( pOverridenBuffers->mVertexData, 0 );
   }

   gpRenderBackend->RenderDisplayList( mesh.GetMeshBuffers().GetDisplayList( mDisplayListOffset ), mDisplayListSize );
//   GXDrawCube();
}