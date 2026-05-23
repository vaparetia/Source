//----------------------------------------------------------------------------
// CRenderer.cpp
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CRenderer.h"

//----------------------------------------------------------------------------

#include "Engine/Graphics/CFrustumPlanes.h"
#include "Engine/Graphics/CIndexArray.h"
#include "Engine/Graphics/CSphericalHarmonicsHelper.h"
#include "Engine/Graphics/CVertexArray.h"
#include "Engine/Memory/CMemoryAllocator.h"
#include "Engine/Script/CScriptVar.h"
#include "Engine/System/CProfileMarker.h"
#include "Engine/System/CSPURSManager.h"
#include "Engine/System/CStopWatch.h"
#include "Engine/System/CTaskQueue.h"

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Backend/CTexture.h"
#include "Renderer/Base/Primitive/EVertexDataType.h"

#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/CDebugDraw.h"
#include "Renderer/Base/Primitive/CMesh.h"

#include "Renderer/Base/Frontend/CLightState.h"
#include "Renderer/Base/Frontend/CRenderViewport.h"
#include "Renderer/Base/Frontend/RenderObject/CDrawableMasterRenderEntity.h"
#include "Renderer/Base/Frontend/RenderObject/CLightRenderEntity.h"
#include "Renderer/Base/Frontend/RenderObject/CMeshChunkRenderEntity.h"
#include "Renderer/Base/Frontend/RenderObject/CMeshRenderEntity.h"
#include "Renderer/Base/Frontend/RenderObject/CRenderBounds.h"
#include "Renderer/Base/Frontend/RenderObject/CRenderObject.h"

#include "Renderer/Base/Frontend/RendererUtils.h"

#if BPE_RENDERER_TYPE == BPE_RENDERER_TYPE_PROGSHADER
#include "Renderer/Base/Material/ProgShader/Shaders/CGlowShader.h"
#endif
//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

RENDERER_API TRenderHandle const kInvalidRenderHandle(0xFFFF, 0xFFFF);

CRenderer * CRenderer::sRenderer = NULL;

//----------------------------------------------------------------------------

CRenderer::CRenderer()
:  mClearColor(CColor::Black())

,  mpCurrentViewport(NULL)

,  mFullscreenQuad(kInvalidRenderHandle)

,  mGridEnabled(false)
,  mGridExtents(40)
,  mGridMajorLineColor(CColor(64, 64, 64))
,  mGridMinorLineColor(CColor(128, 128, 128))
,  mGridAxisColor(CColor::White())

,  mMetrics()
{
#if 0
   bpe_debugger_printf("------------------------ CRenderer RenderObject Class Sizes ------------------------\n");
   bpe_debugger_printf("sizeof(CRenderObject):               %d\n", sizeof(CRenderObject));
   bpe_debugger_printf("sizeof(CRenderBounds):               %d\n", sizeof(CRenderBounds));
   bpe_debugger_printf("sizeof(CDrawableRenderEntity):       %d\n", sizeof(CDrawableRenderEntity));
   bpe_debugger_printf("sizeof(CMeshChunkRenderEntity):      %d\n", sizeof(CMeshChunkRenderEntity));
   bpe_debugger_printf("sizeof(CDrawableMasterRenderEntity): %d\n", sizeof(CDrawableMasterRenderEntity));
   bpe_debugger_printf("sizeof(CMeshRenderEntity):           %d\n", sizeof(CMeshRenderEntity));
   bpe_debugger_printf("------------------------------------------------------------------------------------\n");
   bpe_debugger_printf("sizeof(CMaterialFlags):              %d\n", sizeof(CMaterialFlags));
   bpe_debugger_printf("------------------------------------------------------------------------------------\n");
#endif

   sRenderer = this;
}
   
//----------------------------------------------------------------------------

CRenderer::~CRenderer()
{
   // set render back to back buffer and clear all shader created render targets
   gpRenderBackend->SetRenderTarget(SRenderTarget());
   gpRenderBackend->ShaderCache()->ClearRenderTargets();

   if (mFullscreenQuad != kInvalidRenderHandle)
   {
      DestroyRenderObject(mFullscreenQuad);
   }

   sRenderer = NULL;
}

//----------------------------------------------------------------------------

TRenderHandle const CRenderer::CreateMesh(CResource const &meshOwnerToken, CMesh * pMesh, bool const forceNoMeshChunkBounds)
{
   return RendererUtils::CreateMesh<CMeshRenderEntity>(*this, meshOwnerToken, pMesh, forceNoMeshChunkBounds);
}

//----------------------------------------------------------------------------

TRenderHandle const CRenderer::CreateMeshNoBounds(CResource const &meshOwnerToken, CMesh * pMesh)
{
   return RendererUtils::CreateMeshNoBounds<CMeshRenderEntity>(*this, meshOwnerToken, pMesh);
}

//----------------------------------------------------------------------------

TRenderHandle const CRenderer::CreateLight()
{
   CRenderObjectAllocator objAllocator;
   TRenderHandle handle = AllocRenderObjects(1, CRenderObjectAllocator::GetSize<CLightRenderEntity>(1), &objAllocator);
   new (objAllocator.Allocate<CLightRenderEntity>()) CLightRenderEntity();
   return handle;
}

//----------------------------------------------------------------------------

TRenderHandle const CRenderer::AllocRenderObjects(uint32 const objectCount, uint32 const size, CRenderObjectAllocator * pOutAllocator)
{
   // allocate extra space for object ptrs so we can identify where the objects are during DestroyRenderObjects
   uint32 const sizeToAllocate = size + objectCount * sizeof(uint32);

   // size passed to allocator does NOT include the ptrs to the objects
   char * memory = (char*)BPE_MALLOC_ALIGNED(128, sizeToAllocate);

   int const id = mRenderObjects.Add((CRenderObject*)memory, size);
   
   if( id != TRenderObjectHandleMap::kInvalidHandleId )
   {
      pOutAllocator->SetMemory(memory, objectCount, size);
      return TRenderHandle(static_cast<uint16>(objectCount), static_cast<uint16>(id));
   }
   else
   {
      pOutAllocator->SetMemory(NULL, 0, 0);
      BPE_FREE_ALIGNED(memory);

      printf("CRENDERER WARNING: Out of render handles\n");
      return kInvalidRenderHandle;
   }
}

//----------------------------------------------------------------------------

void CRenderer::DestroyRenderObject( TRenderHandle const handle )
{
   if( handle != kInvalidRenderHandle )
   {
      uint32 const size = mRenderObjects.GetHandleSize(handle.GetId() );
      char * pMemory = (char*)mRenderObjects.Remove( handle.GetId() );
      BPE_VERIFY(pMemory != NULL, false, "Render handle must exist.");

      CRenderObject** ppRenderObjects = (CRenderObject**)(pMemory + size);

      int const objectCount = handle.GetObjectCount();

      for( int i = 0; i < objectCount; ++i )
      {
         CRenderObject* pRenderObject = ppRenderObjects[i];
         pRenderObject->~CRenderObject();
      }

      BPE_FREE_ALIGNED(pMemory);
   }
}

//----------------------------------------------------------------------------

CRenderObject const * const CRenderer::GetRenderObject( TRenderHandle const handle ) const
{
   if( handle != kInvalidRenderHandle )
   {
      return mRenderObjects.Get( handle.GetId() );
   }
   return NULL;
}

//----------------------------------------------------------------------------

CRenderObject * const CRenderer::RenderObject( TRenderHandle const handle )
{
   if( handle != kInvalidRenderHandle )
   {
      return mRenderObjects.Get( handle.GetId() );
   }
   return NULL;
}

//----------------------------------------------------------------------------

void CRenderer::FrameBegin()
{
   BPE_ADD_SCOPED_PROFILE_MARKER("R::FrameBegin");   

   mMetrics = SRenderMetrics();

   RenderBackend()->BeginScene();

   if( mGridEnabled )
   {
      gpDebugDraw->AddGrid(mGridExtents, mGridMajorLineColor, mGridMinorLineColor, mGridAxisColor);
   }
}

//----------------------------------------------------------------------------

void CRenderer::FrameEnd()
{
   BPE_ADD_SCOPED_PROFILE_MARKER("R::FrameEnd");   
   CStopWatch totalTimer;

   // call render hud callbacks
   {
      BPE_ADD_SCOPED_PROFILE_MARKER("R::Hud");   

      CStopWatch hudTimer;

      for( int i = 0; i < mRenderHUDCallbacks.size(); ++i)
         mRenderHUDCallbacks[i]();

      mMetrics.mHUDTime += hudTimer.GetElapsedTime();
   }

   RenderBackend()->EndScene();

   mMetrics.mCPUTotal += totalTimer.GetElapsedTime();
}

//----------------------------------------------------------------------------

void CRenderer::FrameFlip()
{
   BPE_ADD_SCOPED_PROFILE_MARKER("R::Present");   
   //RenderBackend()->Present();
}

//----------------------------------------------------------------------------

void CRenderer::ShutDown()
{
   RenderBackend()->ShutDown();
}

//----------------------------------------------------------------------------
// Main render function

void CRenderer::PrepareViewportRender(CRenderViewport const & viewport, SRenderEntry* pEntries, int const entryCount, SSortedRenderList & outSortedList, uint32 const flags) const
{
   BPE_ADD_SCOPED_PROFILE_MARKER("R::PrepareViewportRender");   
   CStopWatch cpuTotalTimer;

   mpCurrentViewport = &viewport;

   //NOTE: Not really necessary anymore.

   // Setup view port/camera
   {
      RenderBackend()->SetCameraMatrix(viewport.GetTransform());
      if( viewport.HasCustomProjection() )
      {
         RenderBackend()->SetProjectionMatrix(viewport.GetProjectionMatrix());
      }
      else
      {
         RenderBackend()->SetPerspectiveProjection( viewport.GetFOV(), viewport.GetAspect(), viewport.GetNearClipPlane(), viewport.GetFarClipPlane() ); 
      }
   }

   //PrerenderObjects(prerenderObjects);
   BuildSortedRenderMeshLists(pEntries, entryCount, &outSortedList);

   mpCurrentViewport = NULL;

   mMetrics.mCPUTotal += cpuTotalTimer.GetElapsedTime();
}

//----------------------------------------------------------------------------

void CRenderer::ExecuteViewportRender(CRenderViewport const & viewport, SSortedRenderList & sortedList, uint32 const flags, CShader* pOverrideShader) const
{
   BPE_ADD_SCOPED_PROFILE_MARKER("R::ExecuteViewportRender");   
   
   CStopWatch timer;

   ExecuteViewportRenderBegin(viewport, flags);
   ExecuteViewportRenderRange(sortedList, 0, sortedList.mSize, flags, pOverrideShader);
   ExecuteViewportRenderEnd(timer);
}

//----------------------------------------------------------------------------

void CRenderer::ExecuteViewportRenderBegin( CRenderViewport const & viewport, uint32 const flags ) const
{
   mpCurrentViewport = &viewport;
}

//----------------------------------------------------------------------------

void CRenderer::ExecuteViewportRenderRange( SSortedRenderList & sortedList, int const beginObject, int const endObject, uint32 const flags, CShader *pOverrideShader ) const
{
   RenderObjects(sortedList, beginObject, endObject, flags, pOverrideShader);
}

//----------------------------------------------------------------------------

void CRenderer::ExecuteViewportRenderEnd( CStopWatch const &viewportRenderTime ) const
{
   mpCurrentViewport = NULL;
   mMetrics.mCPUTotal += viewportRenderTime.GetElapsedTime();
}

//----------------------------------------------------------------------------

BPE_FORCEINLINE static bool sort_bound_render_entity_shader_key_predicate(SRenderEntry const & lhs, SRenderEntry const & rhs)
{
   bool const lessThan = (lhs.mSortKey < rhs.mSortKey);
   return lessThan;
}

//----------------------------------------------------------------------------

void CRenderer::BuildSortedRenderMeshLists(SRenderEntry* pEntries, int const entryCount, SSortedRenderList * const pOutSortedRenderList) const
{
   CStopWatch sortMaterialTimer;
   BPE_ADD_SCOPED_PROFILE_MARKER("R::BuildSortedRenderMeshLists");

   pOutSortedRenderList->Reset();

   if( entryCount > 0 )
   {
      // Add objects to sorted list
      uint32 uniqueId = 1; // Start at 1 to prevent collisions with shader property pointers

      SRenderEntry* pCurrent = pEntries;
      SRenderEntry* pEnd = pCurrent + entryCount;

      for( ; pCurrent < pEnd; ++pCurrent )
      {
         CShader* pShader = pCurrent->mpObject->GetShader();

         SRenderEntry & newEntry = pOutSortedRenderList->Add();
         newEntry = *pCurrent;
         
         if( pShader )
            newEntry.mSortKey |= pShader->GetSortKey(pCurrent, uniqueId);

         uniqueId += 2;
      }
   }

   // Sort into batches
   // TODO: Convert to stable sort to fix flickering issues.
   // Throws memory assertion in GoW allocator.
   std::stable_sort(pOutSortedRenderList->mRenderEntries, pOutSortedRenderList->mRenderEntries + pOutSortedRenderList->mSize, sort_bound_render_entity_shader_key_predicate);

   mMetrics.mSortByMaterials += sortMaterialTimer.GetElapsedTime();
}

//----------------------------------------------------------------------------

void CRenderer::PrerenderObjects(TMasterRenderEntities & prerenderEntities) const
{
   BPE_ADD_SCOPED_PROFILE_MARKER("R::PreRenderList");   

   CStopWatch preRenderTimer;
   CStopWatch particleSpuUpdateWait;
   bool bPreRenderStalled = false;

   while (return_true())
   {        
      int waitCount = 0;
      foreach(CDrawableMasterRenderEntity* pEntity, prerenderEntities)
      {
         bool const bCompleted = pEntity->PreRenderGeometry();
         if (!bCompleted)
         {
            waitCount++;
         }
      }
      if (waitCount != 0 )
      {
         if (!bPreRenderStalled)
         {
            // Starting timing from first stall
            particleSpuUpdateWait.Reset();
            bPreRenderStalled = true;
         }            
      }
      else
      {
         // Nothing waiting
         break;
      }
   }

   if (bPreRenderStalled) mMetrics.mParticleSpuUpdateWait += particleSpuUpdateWait.GetElapsedTime();

#if BPE_TASKQUEUE_DEFINED==1
   // Disabled task queue flush as not enabled on PS3
   //TaskQueue()->FlushTasks();
#endif

   mMetrics.mPreRender += preRenderTimer.GetElapsedTime();
}

//----------------------------------------------------------------------------

namespace
{
   inline void submit_batch( SRenderEntry const * pObjects, int const numObjects, CShader *pShader )
   {
      pShader->Bind(pObjects);
      pShader->DrawRenderObjects(pObjects, numObjects);
   }
}

//----------------------------------------------------------------------------

void CRenderer::RenderObjects(SSortedRenderList const & sortedObjects, int const beginObject, int const endObject, uint32 const flags, CShader * pOverrideShader) const
{
   BPE_ADD_SCOPED_PROFILE_MARKER("R::RenderObjects");   
   CStopWatch renderObjectsTimer;

   uint32 const kMaxBatchCount = 16;   // TEMP: Reduced batch size to prevent parameter buffer overflow.

   SRenderEntry const * it = sortedObjects.mRenderEntries + beginObject;
   SRenderEntry const * end = sortedObjects.mRenderEntries + endObject;


   // Check if we don't have an override shader specified
   if( pOverrideShader == NULL )
   {
      // Objects have already been sorted into render batches, send objects in batches with matching sort keys.
      
      // Do we have any objects to render?
      if( it < end )
      {
         SRenderEntry const * pCurrentBatchBegin = it;
         SRenderEntry const * pPrevEntry = NULL;
         uint32 batchObjectCount = 0;

         for(; it < end; ++it)
         {
            SRenderEntry const * pCurrentEntry = it;

            CShader * pShader = pCurrentBatchBegin->mpObject->GetShader();
            bool const bufferNotFull = batchObjectCount < kMaxBatchCount;
            bool const canBatch = pShader->CanBatch(pPrevEntry, pCurrentEntry);

            int const addToBatch = bufferNotFull & canBatch;

            // Is sort key the same and do we have more room in the buffer?
            if( addToBatch )
            {
               ++batchObjectCount;
            }
            // Sort key has changed or we filled up the buffer
            else
            {
               // Send batch so far
               if (batchObjectCount > 0)
               {
                  mMetrics.mNumRenderingBatches++;

                  submit_batch( pCurrentBatchBegin, batchObjectCount, pShader);
               }
               
               // add object to beginning of new list
               pCurrentBatchBegin = pCurrentEntry;
               batchObjectCount = 1;
            }
            pPrevEntry = pCurrentEntry;
         }

         // Flush remaining batch
         if (batchObjectCount > 0)
         {
            mMetrics.mNumRenderingBatches++;

            submit_batch( pCurrentBatchBegin, batchObjectCount, pCurrentBatchBegin->mpObject->GetShader() );
         }
      }
   }
   // We have an override shader, render all objects using the specified shader
   else
   {
      // Do we have any objects to render?
      if( it < end )
      {
         SRenderEntry const * pCurrentBatchBegin = it;
         SRenderEntry const * pPrevEntry = NULL;
         uint32 batchObjectCount = 0;

         for(; it < end; ++it)
         {
            SRenderEntry const * pCurrentEntry = it;

            bool const bufferNotFull = batchObjectCount < kMaxBatchCount;
            bool const canBatch = pOverrideShader->CanBatch(pPrevEntry, pCurrentEntry);

            // Is sort key the same and do we have more room in the buffer?
            int const addToBatch = bufferNotFull & canBatch;

            if( addToBatch )
            {
               ++batchObjectCount;
            }
            // Sort key has changed or we filled up the buffer
            else
            {
               // Send batch so far
               if (batchObjectCount > 0)
               {
                  mMetrics.mNumRenderingBatches++;

                  submit_batch( pCurrentBatchBegin, batchObjectCount, pOverrideShader);
               }
               
               // add object to beginning of new list
               pCurrentBatchBegin = pCurrentEntry;
               batchObjectCount = 1;
            }
            pPrevEntry = pCurrentEntry;
         }

         // Flush remaining batch
         if (batchObjectCount > 0)
         {
            mMetrics.mNumRenderingBatches++;

            submit_batch( pCurrentBatchBegin, batchObjectCount, pOverrideShader );
         }
      }
   }

   mMetrics.mRenderObjects += renderObjectsTimer.GetElapsedTime();

   // Do post render callbacks
   if ((flags & kRRLF_PostRenderCallback) != 0)
   {
      BPE_ADD_SCOPED_PROFILE_MARKER("R::PostRenderCallback");

      CStopWatch timer;

      for( int i = 0; i < mPostRenderCallbacks.size(); ++i)
         mPostRenderCallbacks[i]();
      mMetrics.mPostRenderObjects += timer.GetElapsedTime();
   }

}

//----------------------------------------------------------------------------

void CRenderer::RenderFullscreenQuad(CShader &shaderInstance) const
{
   // setup full screen render entity
   CreateFullscreenQuadLazily();

   CDrawableRenderEntity * pFullscreenQuad = static_cast<CDrawableRenderEntity*>(Renderer()->RenderObject(mFullscreenQuad));
   pFullscreenQuad->SetTransform(CMatrix34::Identity());

   // create viewport to render to the desired render target
   CRenderViewport viewport;

   // create render list
   SRenderEntry entry(0, NULL, pFullscreenQuad);

   // create sorted render list
   static SSortedRenderList sortedRenderList;
   sortedRenderList.Reset();
   PrepareViewportRender(viewport, &entry, 1, sortedRenderList, 0);

   // execute render
   ExecuteViewportRender(viewport, sortedRenderList, 0, &shaderInstance);
}

//----------------------------------------------------------------------------

void CRenderer::RenderQuad(CVector2 const & min, CVector2 const & max, CShader &shaderInstance) const
{
   // setup quad render entity
   CreateFullscreenQuadLazily();

   CDrawableRenderEntity * pFullscreenQuad = static_cast<CDrawableRenderEntity*>(Renderer()->RenderObject(mFullscreenQuad));
   CVector3 const scalePivot( -1, 1, 0 );

   CMatrix34 const transform = CMatrix34::Translation(CVector3(min.mX, min.mY, 0)) *
      CMatrix34::Translation(scalePivot) *
      CMatrix34::Scale(CVector3(max.mX - min.mX, max.mY - min.mY, 1.0f)) *
      CMatrix34::Translation(-scalePivot);

   pFullscreenQuad->SetTransform(transform);

   // create viewport to render to the desired render target
   CRenderViewport viewport;

   // create render list
   SRenderEntry entry(0, NULL, pFullscreenQuad);

   // create sorted render list
   static SSortedRenderList sortedRenderList;
   sortedRenderList.Reset();
   PrepareViewportRender(viewport, &entry, 1, sortedRenderList, 0);

   // execute render
   ExecuteViewportRender(viewport, sortedRenderList, 0, &shaderInstance);
}

//----------------------------------------------------------------------------
   
void CRenderer::CreateFullscreenQuadLazily() const
{
#pragma BPE_TODOMSG( "Fix for RVL" )
#if BPE_TARGET != BPE_TARGET_RVL
   if( mFullscreenQuad != kInvalidRenderHandle )
      return;

   CVector3 const pos[] = 
   {
      CVector3( -1.0f, +1.0f, 0.0f ),
      CVector3( +1.0f, +1.0f, 0.0f ),
      CVector3( -1.0f, -1.0f, 0.0f ),
      CVector3( +1.0f, -1.0f, 0.0f )
   };
   
   CVector2 const uvs[] =
   {
      CVector2(0.0f, 0.0f),
      CVector2(1.0f, 0.0f),
      CVector2(0.0f, 1.0f),
      CVector2(1.0f, 1.0f)
   };

   CVertexArray* pVertexArray = CVertexArray::CreateVertexArray(2);
   pVertexArray->AddStream('POS0', kVDT_Float3, 0, (void*)pos, sizeof(CVector3) * 4);
   pVertexArray->AddStream('TEX0', kVDT_Float2, 0, (void*)uvs, sizeof(CVector2) * 4);

   vector<uint32> indices;
   indices.reserve( 4 );
   for( int i = 0; i < 4; ++i )
      indices.push_back( i );

   CIndexArray indexArray( indices );
   
   vector<CMeshChunk> chunks;
   chunks.push_back(CMeshChunk(CAABox::MakeMaxBox(), -1, CMeshChunk::kPrimitive_TriangleStrip, 0, 4, 0, 4, bpe::vector_s<uint32>(), 0, 0, 0));

   TResource<CMesh> meshOwnerToken(new CMesh(*pVertexArray, indexArray, chunks, CMesh::TMaterials(), false, 0));
   mFullscreenQuad = Renderer()->CreateMeshNoBounds(meshOwnerToken, meshOwnerToken.Ptr());

   delete pVertexArray;

#endif
}

//----------------------------------------------------------------------------

void CRenderer::SetGrid( bool const enable, 
                         int const extents, 
                         CColor const & majorLineColor, 
                         CColor const & minorLineColor, 
                         CColor const & axisColor )
{
   mGridEnabled = enable;
   mGridExtents = extents;
   mGridMajorLineColor = majorLineColor;
   mGridMinorLineColor = minorLineColor;
   mGridAxisColor = axisColor;
}
