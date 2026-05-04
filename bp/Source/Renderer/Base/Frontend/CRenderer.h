//----------------------------------------------------------------------------
// CRenderer.h
// Bluepoint
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "boost/signal.hpp"
#include "boost/array.hpp"

#include "Renderer/Base/BPERendererAPI.h"

#include "Engine/Math/CMatrix4.h"
#include "Engine/Mechanics/FastDelegate/FastDelegate.h"
#include "Engine/Mechanics/THandleMap.h"
#include "Engine/Resource/CResourceManager.h"
#include "Engine/Graphics/CColor.h"

#include "Renderer/Base/Frontend/RenderTypes.h"

//----------------------------------------------------------------------------

class CMesh;
class CShader;
class CRenderViewport;
class CStopWatch;
struct SRenderTarget;

//----------------------------------------------------------------------------

struct SRenderMetrics
{
   SRenderMetrics()
   {
      // Reset all values
      // Don't turn this into a class!
      memset(this, 0, sizeof(SRenderMetrics));
   }

   //--- CPU Rendertime

   // Time to separate objects into different lists
   real32   mFrustumCullObjects;

   // Stalling waiting for SPUs to finish updating particles
   real32   mParticleSpuUpdateWait;

   // Time to pre render all visible objects
   real32   mPreRender;

   // Time to sort objects by material
   real32   mSortByMaterials;

   // Stall time waiting for particles to finish pre-rendering
   real32   mParticleSpuPreRenderWait;

   // Time to render opaque objects
   real32   mRenderObjects;
   
   // Time for post render callbacks
   real32   mPostRenderObjects;

   // Time to render HUD elements
   real32   mHUDTime;

   // Total time spend during Rendering of objects
   real32   mCPUTotal;

   //--- Rendering Batches
   uint32   mNumRenderingBatches;
};

//----------------------------------------------------------------------------

class CRenderObjectAllocator
{
   friend class CRenderer;

public:
   CRenderObjectAllocator()
      :  mpMemory(NULL)
      ,  mpObjectAddresses(NULL)
      ,  mObjectsRemaining(0)
      ,  mSizeRemaining(0)
   {
   }

   ~CRenderObjectAllocator()
   {
      BPE_VERIFY(mObjectsRemaining == 0, false, "Invalid usage, you didn't allocate the number of objects you requested");
      BPE_VERIFY(mSizeRemaining == 0, false, "You didn't write the correct amount of data");
   }

   template <class T> static uint32 const GetSize(uint32 const numberOfObjects)
   {
      return sizeof(T) * numberOfObjects;
   }

   template <class T> void * Allocate()
   {
      int const sizeOfClass = GetSize<T>(1);

      BPE_VERIFY(mObjectsRemaining > 0, false, "Trying to allocate more objects than you originally requested");
      BPE_VERIFY(mSizeRemaining >= sizeOfClass, false, "Trying to allocate more bytes than originally quested");

      mObjectsRemaining--;
      mSizeRemaining -= sizeOfClass;

      char* pMemory = mpMemory;

      *mpObjectAddresses = (CRenderObject*)pMemory;
      mpObjectAddresses++;

      mpMemory += sizeOfClass;

      return pMemory;
   }

protected:
   void SetMemory(char* pMemory, uint32 const objectCount, uint32 const size)
   {
      mpMemory = pMemory;
      mObjectsRemaining = objectCount;
      mSizeRemaining = size;
      mpObjectAddresses = (CRenderObject**)(mpMemory + size);
   }

private:
   char* mpMemory;
   CRenderObject** mpObjectAddresses;

   int   mObjectsRemaining;
   int   mSizeRemaining;

   BPE_DISABLE_COPY_AND_ASSIGNMENT(CRenderObjectAllocator);
};

//----------------------------------------------------------------------------

class RENDERER_API CRenderer
{
public:
   typedef fastdelegate::FastDelegate0<>        TDelegate;

public:
   explicit CRenderer();
   ~CRenderer();

   static CRenderer * const GetGlobalPtr();
   
   /// creates render bounds tree with render entities as leaves for given mesh
   TRenderHandle const CreateMesh(CResource const &meshOwnerToken, CMesh * pMesh, bool const forceNoMeshChunkBounds );

   /// creates linked mesh chunk render entity list for given mesh
   TRenderHandle const CreateMeshNoBounds( CResource const &meshOwnerToken, CMesh * pMesh);

   /// creates render entity for a light
   TRenderHandle const CreateLight();

   /// Allocates a contiguous block of memory to be used for render objects.
   /// Memory is not initialized.
   TRenderHandle const AllocRenderObjects(uint32 const objectCount, uint32 const size, CRenderObjectAllocator * pOutAllocator);

   /// Frees a block of render objects
   /// CRenderObject virtual destructors are called for all the objects originally allocated through AllocRenderObjects, so make sure you don't allocate more objects than you're actually initializing!
   void DestroyRenderObject(TRenderHandle const handle);

   /// queries const version of render entity by handle
   CRenderObject const * const GetRenderObject( TRenderHandle const handle ) const;
   /// queries nonconst version of render entity by handle
   CRenderObject * const RenderObject( TRenderHandle const handle );

   CRenderViewport const * GetCurrentViewport() const { return mpCurrentViewport; }

   /// marks beginning of a new frame
   void FrameBegin();
   /// marks end of current frame
   void FrameEnd();

   void FrameFlip();

   /// shut down the renderer so we can exit the game safely
   void ShutDown();

   /// Draws all render objects to the current viewport
   enum ERender_RenderListFlags
   {
      kRRLF_ClearColor                       = 1 << 0,
      kRRLF_ClearDepth                       = 1 << 1,
      kRRLF_PostRenderCallback               = 1 << 2,
      kRRLF_FrustumCull                      = 1 << 3,

      kRRLF_Terminate
   };

   void PrepareViewportRender(CRenderViewport const & viewport, SRenderEntry* pEntries, int const entryCount, SSortedRenderList & outSortedList, uint32 const flags) const;

   // Execute viewport render, rendering all objects in the sorted list.
   void ExecuteViewportRender(CRenderViewport const & viewport, SSortedRenderList & sortedList, uint32 const flags, CShader* pOverrideShader = NULL) const;

   // Execute viewport render, allows rendering of multiple ranges inside the sorted list with different flags and/or override shaders.
   void ExecuteViewportRenderBegin(CRenderViewport const & viewport, uint32 const flags) const;
   void ExecuteViewportRenderRange(SSortedRenderList & sortedList, int const beginObject, int const endObject, uint32 const flags, CShader *pOverrideShader = NULL) const;
   void ExecuteViewportRenderEnd(CStopWatch const &viewportRenderTime) const;
   
   void RenderFullscreenQuad(CShader &shaderInstance) const;
   void RenderQuad(CVector2 const & min, CVector2 const & max, CShader &shaderInstance) const;

   /// sets the clear color
   void SetClearColor( CColor const & color ) { mClearColor = color; }

   void SetGrid( bool const enable, 
                              int const extents = 40, 
                              CColor const & majorLineColor = CColor(70, 70, 70), 
                              CColor const & minorLineColor = CColor(32, 32, 32), 
                              CColor const & axisColor = CColor::White() );

   // Delegates called after objects rendered
   std::vector<TDelegate> & PostRenderCallbacks() { return mPostRenderCallbacks; }
   
   // accesses the signal for render hud, this happens after all geometry and post processing filters are run
   std::vector<TDelegate> & RenderHudCallbacks() { return mRenderHUDCallbacks; }

   SRenderMetrics const & GetMetrics() const { return mMetrics; }
   SRenderMetrics &       Metrics()          { return mMetrics; }

private:
   void PrerenderObjects(TMasterRenderEntities & prerenderEntities) const;
   void BuildSortedRenderMeshLists(SRenderEntry* pEntries, int const entryCount, SSortedRenderList * const pOutSortedRenderList) const;
   void RenderObjects(SSortedRenderList const & sortedObjects, int const startObject, int const endObject, uint32 const flags, CShader * pOverrideShader) const;

   void CreateFullscreenQuadLazily() const;


private:
   typedef THandleMap< CRenderObject, 4096 >    TRenderObjectHandleMap;

   static CRenderer *                  sRenderer;

   TRenderObjectHandleMap              mRenderObjects;

   CColor                              mClearColor;

   // other

   std::vector<TDelegate>              mPostRenderCallbacks;
   std::vector<TDelegate>              mRenderHUDCallbacks;

   mutable CRenderViewport const *     mpCurrentViewport;
   mutable TRenderHandle               mFullscreenQuad;

   bool                                mGridEnabled;
   int                                 mGridExtents;
   CColor                              mGridMajorLineColor;
   CColor                              mGridMinorLineColor;
   CColor                              mGridAxisColor;

   mutable SRenderMetrics              mMetrics;
private:
   BPE_DISABLE_OBJECT_ASSIGN( CRenderer );
};

//----------------------------------------------------------------------------

BPE_FORCEINLINE CRenderer * const CRenderer::GetGlobalPtr()
{
   return sRenderer;
}

//----------------------------------------------------------------------------

BPE_FORCEINLINE CRenderer * const Renderer()
{
   return CRenderer::GetGlobalPtr();
}

//----------------------------------------------------------------------------

