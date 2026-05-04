//----------------------------------------------------------------------------
// RendererUtils.h
// Bluepoint
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "Renderer/Base/Frontend/RenderTypes.h"
#include "Renderer/Base/Frontend/CRenderer.h"

//----------------------------------------------------------------------------

namespace RendererUtils
{
   /// creates render bounds tree with render entities as leaves for given mesh
   /// Template class can be CMeshRenderEntity or a custom version derived from that class.
   template <class TMeshRenderEntity> TRenderHandle const CreateMesh( CRenderer &renderer, CResource const &meshOwnerToken, CMesh * pMesh, bool const forceNoMeshChunkBounds )
   {
      CMesh::TMeshChunks const & chunks = pMesh->GetMeshChunks();

      // one render object for each mesh chunk itself
      int const meshChunkCount = chunks.size();

      // create mesh chunk bounds if it's not disabled AND we have more than one mesh chunk.
      bool const createMeshChunkBounds = !forceNoMeshChunkBounds && (meshChunkCount > 1);

      // if we want to create mesh chunk bounds, they are the same number as we have mesh chunks.
      int const meshChunkBounds = createMeshChunkBounds ? meshChunkCount : 0;

      // 1x bounds object for the whole mesh
      // 1x mesh object for the actual mesh itself
      int const renderObjectCount = 1 + meshChunkBounds + meshChunkCount + 1;
      uint32 const size = CRenderObjectAllocator::GetSize<CRenderBounds>(1) +
                          CRenderObjectAllocator::GetSize<CRenderBounds>(meshChunkBounds) +
                          CRenderObjectAllocator::GetSize<CMeshChunkRenderEntity>(meshChunkCount) +
                          CRenderObjectAllocator::GetSize<TMeshRenderEntity>(1);

      CRenderObjectAllocator objAllocator;
      TRenderHandle const handle = renderer.AllocRenderObjects(renderObjectCount, size, &objAllocator);

      if( handle != kInvalidRenderHandle )
      {

         // create mesh bounds node
         CRenderBounds * pBounds = new (objAllocator.Allocate<CRenderBounds>()) CRenderBounds();
         pBounds->SetBounds(pMesh->GetBounds());

         // create mesh render entity
         CMeshRenderEntity* pMeshRenderEntity = static_cast<CMeshRenderEntity*>(new (objAllocator.Allocate<TMeshRenderEntity>()) TMeshRenderEntity(meshOwnerToken, pMesh));
         pBounds->SetMasterEntity(pMeshRenderEntity);
         
         for(int currentChunkIndex = 0; currentChunkIndex < chunks.size(); ++currentChunkIndex)
         {
            CMeshChunk const & chunk = chunks[currentChunkIndex];

            // if we want to create mesh chunk bounds, we create a new bounds for each of the chunks using their bounding boxes.
            if( createMeshChunkBounds )
            {
               CRenderBounds * pChunkBounds = new (objAllocator.Allocate<CRenderBounds>()) CRenderBounds(pMeshRenderEntity);
               pChunkBounds->SetBounds(chunk.GetBounds());

               // add chunk as child of chunk bounds
               CMeshChunkRenderEntity * pChunkRenderEntity = new (objAllocator.Allocate<CMeshChunkRenderEntity>()) CMeshChunkRenderEntity(pMeshRenderEntity, &chunk);
               pChunkBounds->AddChild(pChunkRenderEntity);

               // add chunk bounds as child of mesh bounds
               pBounds->AddChild(pChunkBounds);
            }
            // if there is just one mesh chunk for this mesh, we can directly add the chunk to the mesh bounds.
            else
            {
               // add chunk as child of mesh bounds because there is only one single chunk in this mesh
               CMeshChunkRenderEntity * pChunkRenderEntity = new (objAllocator.Allocate<CMeshChunkRenderEntity>()) CMeshChunkRenderEntity(pMeshRenderEntity, &chunk);
               pBounds->AddChild(pChunkRenderEntity);
            }
         }
      }
      
      return handle;
   }

   /// creates linked mesh chunk render entity list for given mesh.
   /// Template class can be CMeshRenderEntity or a custom version derived from that class.
   /// If 'pMesh' isn't NULL, then that is used for mesh reference instead of the token. This allows usage of group models without tokens
   template <class TMeshRenderEntity> TRenderHandle const CreateMeshNoBounds( CRenderer &renderer, CResource const &meshOwnerToken, CMesh * pMesh)
   {
      CMesh::TMeshChunks const & meshChunks = pMesh->GetMeshChunks();

      int const meshCount = 1;
      int const meshChunkCount = meshChunks.size();

      uint32 const size = CRenderObjectAllocator::GetSize<TMeshRenderEntity>(meshCount) +
                          CRenderObjectAllocator::GetSize<CMeshChunkRenderEntity>(meshChunkCount);

      CRenderObjectAllocator objAllocator;
      TRenderHandle const handle = renderer.AllocRenderObjects(meshChunkCount + meshCount, size, &objAllocator);

      if( handle != kInvalidRenderHandle )
      {
         std::vector<void*> meshChunkEntities;
         meshChunkEntities.reserve(meshChunkCount);

         for( int i = 0; i < meshChunkCount; ++i )
         {
            meshChunkEntities.push_back(objAllocator.Allocate<CMeshChunkRenderEntity>());
         }

         CMeshRenderEntity* meshRenderEntity = static_cast<CMeshRenderEntity*>(new (objAllocator.Allocate<TMeshRenderEntity>()) TMeshRenderEntity(meshOwnerToken, pMesh));         

         CMeshChunkRenderEntity* pFirstMeshChunk = NULL;
         CMeshChunkRenderEntity* pLastMeshChunk = NULL;

         // create mesh chunk render entities and chain them together
         for(int currentChunkIndex = 0; currentChunkIndex < meshChunks.size(); ++currentChunkIndex )
         {
            CMeshChunk const & chunk = meshChunks[currentChunkIndex];
            CMeshChunkRenderEntity * pChunkRenderEntity = new (meshChunkEntities[currentChunkIndex]) CMeshChunkRenderEntity(meshRenderEntity, &chunk);
            
            if( pFirstMeshChunk == NULL )
            {
               pFirstMeshChunk = pChunkRenderEntity;
            }

            if( pLastMeshChunk != NULL )
            {
               pLastMeshChunk->SetSibling(pChunkRenderEntity);
            }
            
            pLastMeshChunk = pChunkRenderEntity;
         }
      }

      return handle;
   }
};

//----------------------------------------------------------------------------

