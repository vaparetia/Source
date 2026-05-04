//------------------------------------------------------------------------------------------
// PS3CRenderBackend.h
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "Renderer/Base/Backend/CTexture.h"
#include "Renderer/Base/Backend/CRenderHWAllocator.h"

#include "Engine/System/CSyncCriticalSection.h"

//----------------------------------------------------------------------------

#include "cell/gcm.h"
#include "sys/synchronization.h"

//----------------------------------------------------------------------------

class CIndexBuffer;
class CVertexData;
class CellSpursTaskset2;
class CTexture;
class CDynamicVertexBufferPool_RT;
class CDynamicIndexBufferPool_RT;
class CDynamicVertexBufferPool_UT;
class CDynamicIndexBufferPool_UT;
class CDynamicIndexBufferPoolChunk_RT;
class CDynamicIndexBufferPoolChunk_UT;

struct CellSyncLFQueue;
typedef CellSyncLFQueue CellSpursLFQueue;

struct EdgeGeomPpuConfigInfo;

struct SRect;

namespace NRenderQueue
{
   struct SRenderCmd;
}

//----------------------------------------------------------------------------

class CFragmentProgramAllocator
{
public:
   CFragmentProgramAllocator();
   ~CFragmentProgramAllocator();

   // Allocate memory to be used by the RSX for fragment programs
   void Allocate(int const size, uint8** pAllocationEA, uint32* pAllocationOffset);

   // Any memory allocated up to this point is freed (when the RSX processes this command)
   void AddRSXFreeAllocatedMemoryCmd()
   {
      cell::Gcm::Inline::cellGcmSetWriteTextureLabel(mRsxProcessedPtrLabel, mCurrentPtr);
   }

private:
   CRenderHWAllocator::SHandle const * mpMemory;
   // RSX offset of the buffer
   uint32                              mMemoryBaseOffset;
   // Ptr to last allocation (because we allocate backwards in the ring buffer)
   uint32                              mCurrentPtr;
   // Label that stores the GPU-side fragment program processed ptr (any data after this ptr is consumed by the GPU)
   int                                 mRsxProcessedPtrLabel;
   volatile uint32_t*                  mpRsxProcessedPtrLabelAddress;
   uint32_t                            mCachedRsxProcessLabelValue;
};

//----------------------------------------------------------------------------

class RENDERER_API CRenderBackend : public CBaseRenderBackend
{
   typedef CBaseRenderBackend inherited;
   friend class CBaseRenderBackend;
   template< class Tx, class Ty > friend void SetIndexDataTemplate(CRenderBackend* pRenderBackend, Tx const * pChunk);
private:
   struct SDisplayBuffer
   {
      boost::scoped_ptr< CTexture > mpTexture;
   };

   struct STileInfo
   {
      STileInfo()
         : mMemory(NULL)
      {
      }

      CRenderHWAllocator::SHandle const * mMemory;
   };

   struct SZTileInfo
   {
      SZTileInfo()
         : mMemory(NULL)
      {
      }

      CRenderHWAllocator::SHandle const * mMemory;
   };

public:

   enum EBlendFunc
   {
      kBF_Zero = CELL_GCM_ZERO,

      kBF_One = CELL_GCM_ONE,

      kBF_SrcColor = CELL_GCM_SRC_COLOR,
      kBF_InvSrcColor = CELL_GCM_ONE_MINUS_SRC_COLOR,

      kBF_DstColor = CELL_GCM_DST_COLOR,
      kBF_InvDstColor = CELL_GCM_ONE_MINUS_DST_COLOR,

      kBF_SrcAlpha = CELL_GCM_SRC_ALPHA,
      kBF_InvSrcAlpha = CELL_GCM_ONE_MINUS_SRC_ALPHA,

      kBF_DstAlpha = CELL_GCM_DST_ALPHA,
      kBF_InvDstAlpha = CELL_GCM_ONE_MINUS_DST_ALPHA,

      kBF_FixedAlpha = CELL_GCM_CONSTANT_ALPHA,
      kBF_InvFixedAlpha = CELL_GCM_ONE_MINUS_CONSTANT_ALPHA,

      kBF_FixedColor = CELL_GCM_CONSTANT_COLOR,
      kBF_InvFixedColor = CELL_GCM_ONE_MINUS_CONSTANT_COLOR,
   };

   enum EBlendOp
   {
      kBO_Add = CELL_GCM_FUNC_ADD,
      kBO_Subtract = CELL_GCM_FUNC_SUBTRACT,
      kBO_RevSubtract = CELL_GCM_FUNC_REVERSE_SUBTRACT,
      kBO_Min = CELL_GCM_MIN,
      kBO_Max = CELL_GCM_MAX
   };

   enum EAlphaFunc
   {
      kAF_Never = CELL_GCM_NEVER,
      kAF_Less = CELL_GCM_LESS,
      kAF_Equal = CELL_GCM_EQUAL,
      kAF_LEqual = CELL_GCM_LEQUAL,
      kAF_Greater = CELL_GCM_GREATER,
      kAF_NotEqual = CELL_GCM_NOTEQUAL,
      kAF_GEqual = CELL_GCM_GEQUAL,
      kAF_Always = CELL_GCM_ALWAYS
   };

   enum EDepthFunc
   {
      kDF_Never = CELL_GCM_NEVER,
      kDF_Less = CELL_GCM_LESS,
      kDF_Equal = CELL_GCM_EQUAL,
      kDF_LEqual = CELL_GCM_LEQUAL,
      kDF_Greater = CELL_GCM_GREATER,
      kDF_NotEqual = CELL_GCM_NOTEQUAL,
      kDF_GEqual = CELL_GCM_GEQUAL,
      kDF_Always = CELL_GCM_ALWAYS
   };

   enum EStencilOp
   {
      kSO_Keep = CELL_GCM_KEEP,
      kSO_Zero = CELL_GCM_ZERO,
      kSO_Replace = CELL_GCM_REPLACE,
      kSO_Incr = CELL_GCM_INCR,
      kSO_Decr = CELL_GCM_DECR,
      kSO_IncrWrap = CELL_GCM_INCR_WRAP,
      kSO_DecrWrap = CELL_GCM_DECR_WRAP,
      kSO_Invert = CELL_GCM_INVERT
   };

   enum EWrapMode
   {
      kWM_Wrap = CELL_GCM_TEXTURE_WRAP,
      kWM_Mirror = CELL_GCM_TEXTURE_MIRROR,
      kWM_Clamp = CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
      kWM_Border = CELL_GCM_TEXTURE_BORDER,

      kWM_Count
   };

   enum EFilterMode
   {
      kFM_Nearest          = CELL_GCM_TEXTURE_NEAREST,
      kFM_Linear           = CELL_GCM_TEXTURE_LINEAR,
      kFM_Nearest_Nearest  = CELL_GCM_TEXTURE_NEAREST_NEAREST,
      kFM_Linear_Nearest   = CELL_GCM_TEXTURE_LINEAR_NEAREST,
      kFM_Nearest_Linear   = CELL_GCM_TEXTURE_NEAREST_LINEAR,
      kFM_Linear_Linear    = CELL_GCM_TEXTURE_LINEAR_LINEAR
   };

   enum ECullMode
   {
      kCM_None,
      kCM_CW,
      kCM_CCW
   };

public:
   explicit CRenderBackend(IResourcePool & resourcePool,
                           SRenderInitialization const & initFlags);
   ~CRenderBackend();

   void WaitForPendingAllocationsToClear();

   void Clear(int const clearFlags, CColor const color, real32 const z = 1.0f, int const stencil = 0);

   void BeginScene();
   void EndScene();
   void ShutDown();

   void BeginTiling(SRenderTarget const & renderTarget) {}
   void EndTiling() {}

   void SuspendGameRenderThread();
   void ResumeGameRenderThread();

   void AcquireRenderThreadOwnership();
   void ReleaseRenderThreadOwnership();

   void ResolveRenderTarget(CBaseTexture * pRenderTexture) {}
   void ResolvePartialRenderTarget(CBaseTexture * pRenderTexture, SRect const & srcRect) {}
   void ResolveRenderTargetPredicated(CBaseTexture * pRenderTexture) {}
   void ResolveDepthStencilPredicated(CBaseTexture * pRenderTexture) {}
   void ResolveDepthStencilPredicatedMSAA2(CBaseTexture * pRenderTexture0, CBaseTexture * pRenderTexture1) {}
   void ResolveDepthStencilPredicatedMSAA4(CBaseTexture * pRenderTexture0, CBaseTexture * pRenderTexture1, CBaseTexture * pRenderTexture2, CBaseTexture * pRenderTexture3) {}

   // sets the current render target to the texture passed in, 
   // if no texture is passed in the target is reset to the frame buffer
   void SetRenderTarget(SRenderTarget const & renderTarget);
   CBaseTexture* GetCurrentBackbufferTexture() const;
   CBaseTexture* GetLastBackbufferTexture() const;

   // returns the current active render target texture,
   // note that this function may return NULL for the color buffers if the device back buffer is the currently bound
   SRenderTarget const & GetCurrentRenderTarget() const { return mCurrentRenderTarget; }
   CBaseTexture * GetCurrentRenderTexture() const { return NULL; }
   CBaseTexture * GetCurrentRenderDepth() const { return NULL; }
   CBaseTexture::EAntiAliasType const GetCurrentRenderTargetAAType() const { return mCurrentRenderTargetAAType; }

   int GetPresentationInterval() const { return mPresentationInterval; }

   void SetShadeMode(int isSmooth)
   {
      cell::Gcm::Inline::cellGcmSetShadeMode(isSmooth ? CELL_GCM_SMOOTH : CELL_GCM_FLAT);
   }

   void SetAlphaTestEnable(bool enable)
   {
      cell::Gcm::Inline::cellGcmSetAlphaTestEnable(enable);
   }

   void SetAlphaFunc(EAlphaFunc func, int refValue)
   {
      cell::Gcm::Inline::cellGcmSetAlphaFunc(func, refValue);
   }

   // enable disable stencil testing
   void SetStencilEnable(bool enable)
   {
      cell::Gcm::Inline::cellGcmSetStencilTestEnable(enable);
   }

   // enable disable stencil testing
   void SetBackStencilEnable(bool enable)
   {
      cell::Gcm::Inline::cellGcmSetTwoSidedStencilTestEnable(enable);
   }

   // stencil operation on stencil fail, depth fail and depth pass
   void SetStencilOp(EStencilOp fail, EStencilOp depthFail, EStencilOp depthPass )
   {
      cell::Gcm::Inline::cellGcmSetStencilOp(fail, depthFail, depthPass);
   }

   // stencil operation on stencil fail, depth fail and depth pass
   void SetBackStencilOp(EStencilOp fail, EStencilOp depthFail, EStencilOp depthPass )
   {
      cell::Gcm::Inline::cellGcmSetBackStencilOp(fail, depthFail, depthPass);
   }

   // write mask
   void SetStencilMask(uint32 mask)
   {
      cell::Gcm::Inline::cellGcmReserveMethodSize(4);
      cell::Gcm::UnsafeInline::cellGcmSetStencilMask(mask);
      cell::Gcm::UnsafeInline::cellGcmSetBackStencilMask(mask);
   }

   // stencil test function, test reference value, stencil test mask
   void SetStencilFunc(EAlphaFunc func, uint32 ref, uint32 mask)
   {
      cell::Gcm::Inline::cellGcmSetStencilFunc(func, ref, mask);
   }

   // stencil test function, test reference value, stencil test mask
   void SetBackStencilFunc(EAlphaFunc func, uint32 ref, uint32 mask)
   {
      cell::Gcm::Inline::cellGcmSetBackStencilFunc(func, ref, mask);
   }

   void SetColorAlphaWriteEnabled(bool const color, bool const alpha)
   {
      uint32 mask = 0;
      if( color )
      {
         mask |= CELL_GCM_COLOR_MASK_R | CELL_GCM_COLOR_MASK_G | CELL_GCM_COLOR_MASK_B;
      }
      if( alpha )
      {
         mask |= CELL_GCM_COLOR_MASK_A;
      }
      cell::Gcm::Inline::cellGcmSetColorMask(mask);
   }

   void SetBlendMode(bool const enable, EBlendFunc const srcBlendMode, EBlendFunc const dstBlendMode)
   {
      if( enable )
      {
         cell::Gcm::Inline::cellGcmReserveMethodSize(2 + 3);
         cell::Gcm::UnsafeInline::cellGcmSetBlendEnable(CELL_GCM_TRUE);
         cell::Gcm::UnsafeInline::cellGcmSetBlendFunc(srcBlendMode, dstBlendMode, srcBlendMode, dstBlendMode);
      }
      else
      {
         cell::Gcm::Inline::cellGcmSetBlendEnable(CELL_GCM_FALSE);
      }
   }

   void SetBlendMode(bool const enable, EBlendFunc const srcBlendMode, EBlendFunc const dstBlendMode, const uint32 rgbaFixed)
   {
      if( enable )
      {
         cell::Gcm::Inline::cellGcmReserveMethodSize(2 + 3 + 4);
         cell::Gcm::UnsafeInline::cellGcmSetBlendEnable(CELL_GCM_TRUE);
         cell::Gcm::UnsafeInline::cellGcmSetBlendFunc(srcBlendMode, dstBlendMode, srcBlendMode, dstBlendMode);
         cell::Gcm::UnsafeInline::cellGcmSetBlendColor(rgbaFixed,0);
      }
      else
      {
         cell::Gcm::Inline::cellGcmSetBlendEnable(CELL_GCM_FALSE);
      }
   }

   void SetBlendMode(bool const enable, EBlendFunc const rgbSrcBlendMode, EBlendFunc const rgbDstBlendMode, EBlendFunc const aSrcBlendMode, EBlendFunc const aDstBlendMode)
   {
      if( enable )
      {
         cell::Gcm::Inline::cellGcmReserveMethodSize(2 + 3);
         cell::Gcm::UnsafeInline::cellGcmSetBlendEnable(CELL_GCM_TRUE);
         cell::Gcm::UnsafeInline::cellGcmSetBlendFunc(rgbSrcBlendMode, rgbDstBlendMode, aSrcBlendMode, aDstBlendMode);
      }
      else
      {
         cell::Gcm::Inline::cellGcmSetBlendEnable(CELL_GCM_FALSE);
      }
   }

   void SetBlendMode(bool const enable, EBlendFunc const rgbSrcBlendMode, EBlendFunc const rgbDstBlendMode, EBlendFunc const aSrcBlendMode, EBlendFunc const aDstBlendMode, const uint32 rgbaFixed)
   {
      if( enable )
      {
         cell::Gcm::Inline::cellGcmReserveMethodSize(2 + 3 + 4);
         cell::Gcm::UnsafeInline::cellGcmSetBlendEnable(CELL_GCM_TRUE);
         cell::Gcm::UnsafeInline::cellGcmSetBlendFunc(rgbSrcBlendMode, rgbDstBlendMode, aSrcBlendMode, aDstBlendMode);
         cell::Gcm::UnsafeInline::cellGcmSetBlendColor(rgbaFixed,0);
      }
      else
      {
         cell::Gcm::Inline::cellGcmSetBlendEnable(CELL_GCM_FALSE);
      }
   }

   void SetBlendOp(EBlendOp const blendOp)
   {
      cell::Gcm::Inline::cellGcmSetBlendEquation(blendOp, blendOp);
   }

   void SetBlendOp(EBlendOp const colorBlendOp, EBlendOp const alphaBlendOp)
   {
      cell::Gcm::Inline::cellGcmSetBlendEquation(colorBlendOp, alphaBlendOp);
   }

   void SetFogEnabled( bool const enable ) { mFogEnabled = enable; }
   void SetFog( EFogMode const fogMode, real32 const nearZ, real32 const farZ, CColorf const &color );
   bool GetFogEnabled() const { return mFogEnabled; }
   CColorf const &GetFogColor() const { return mFogColor; }

   void SetDepthCompareEnabled(bool const enable)
   {
      cell::Gcm::Inline::cellGcmSetDepthTestEnable(enable);
   }

   void SetDepthFunc(EDepthFunc const func)
   {
      cell::Gcm::Inline::cellGcmSetDepthFunc(func);
   }

   void SetDepthWriteEnabled(bool const enable)
   {
      cell::Gcm::Inline::cellGcmSetDepthMask(enable);
   }

   void SetCullMode(ECullMode const mode)
   {
      if( mode == kCM_None )
      {
         cell::Gcm::Inline::cellGcmSetCullFaceEnable(CELL_GCM_FALSE);
      }
      else
      {
         cell::Gcm::Inline::cellGcmSetCullFaceEnable(CELL_GCM_TRUE);
         cell::Gcm::Inline::cellGcmSetCullFace((mode == kCM_CW) ? CELL_GCM_FRONT : CELL_GCM_BACK);
      }
   }

   void SetPointSize(real32 const size)
   {
      cell::Gcm::Inline::cellGcmSetPointSize(size);
   }

   void SetLineWidth(real32 const width)
   {
      cell::Gcm::Inline::cellGcmSetLineWidth((uint32_t)(width * (1<<3)));// 6 bits integer, 3 bits fraction
   }

   void SetLineSmoothEnabled(bool const enable)
   {
      cell::Gcm::Inline::cellGcmSetLineSmoothEnable(enable ? CELL_GCM_TRUE : CELL_GCM_FALSE);
   }

   // Coords are in pixels
   void RenderQuad2d(CVector2 const & start,
                     CVector2 const & end,
                     CColor const & color);

   void RenderLine2d(CVector2 const & start,
                     CVector2 const & end,
                     CColor const & color);

   void  SetIndexData(CIndexBuffer const * indexBuffer);
   void  SetIndexData(CDynamicIndexBufferPoolChunk_RT const * pChunk);
   void  SetIndexData(CDynamicIndexBufferPoolChunk_UT const * pChunk);
   void  SetIndexData(uint32 const indexBuffer, uint32 const indexBufferLocation, uint32 const indexBufferType);

   void  SetVertexData(CShaderVertexDataBinding const & vertexDataBinding, CVertexData const & vertexData, uint64 const vertexDataHash_1, uint64 const vertexDataHash_2);
   void  ForceVertexDataRebind();

   void RenderPrimitives(CMeshChunk::EPrimitive type, uint32 const vertexBufferOffset, uint32 const vertexCount, uint32 const indexBufferOffset, uint32 const indexCount);
   void RenderPrimitivesNoIndices(CMeshChunk::EPrimitive type, uint32 const vertexBufferOffset, uint32 const vertexCount);
   void BeginRenderPrimitivesCustom();
   void EndRenderPrimitivesCustom();

   void RenderPrimitivesUserVertexData(CMeshChunk::EPrimitive type, 
                                       CShaderVertexDataBinding binding, 
                                       void const * pData, 
                                       uint32 const numVertices);

   CDynamicVertexBufferPool_RT* GetVertexBufferPool_RT() const { return mpDynamicVertexBufferPool_RT.get(); }
   CDynamicIndexBufferPool_RT* GetIndexBufferPool_RT() const { return mpDynamicIndexBufferPool_RT.get(); }
   CDynamicVertexBufferPool_UT* GetVertexBufferPool_UT() const { return mpDynamicVertexBufferPool_UT.get(); }
   CDynamicIndexBufferPool_UT* GetIndexBufferPool_UT() const { return mpDynamicIndexBufferPool_UT.get(); }

#if BPE_USE_EDGE_GEOM
   void AddEdgeSegment(void* skinningMatrices, EdgeGeomPpuConfigInfo const * pSegmentList, int const segmentIdx);
   void SubmitCurrentEdgeJobList();
   void WaitForEdgeCompletion();

   void SetEdgeProjectionMatrix(float* mtx);
   void SetEdgeLocalToWorld(float* mtx);
#endif

   void SetTexture(int const texUnit, CBaseTexture const * pTexture, bool const allowAniso = true);
   void SetTextureAddressMode(int const texUnit, EWrapMode const wrapU, EWrapMode const wrapV);
   void SetTextureFilter(int const texUnit, EFilterMode minFilter, EFilterMode magFilter);
   void SetTextureFilter(int const texUnit, real32 const lodBias, EFilterMode minFilter, EFilterMode magFilter);

   void SetVertexRegisters(int startRegister, int numVectors, CVector4 const * pVector);

   // PS3 implementation specific functions
   void TakeScreenshot(int const screenShotJitterCount);

   void GetMemoryStats(ERenderMemory const memory, SRenderHWAllocatorStats * pStats) const;

   // Allocate a block of memory from the given pool that is fixed (i.e. unrelocateable)
   CRenderHWAllocator::SHandle const * AllocFixed(int const size, int const alignment, ERenderMemory const memoryType, ERenderMemory const fallbackMemory = kRM_Invalid);
   // Free memory, handle must not be NULL
   void Free(CRenderHWAllocator::SHandle const * pHandle, int const delayedFree = 1);

   static void AdjustSizeForZCull(int* pWidth, int *pHeight);

   CRenderHWAllocator::SHandle const * AllocTiledRenderTarget(bool const isDepth, CBaseTexture::EAntiAliasType const aa, ERenderMemory const memoryType, int const width, int * pHeight, int * pPitch, bool const createZCull, int const zCullOffset);
   void FreeTiledRenderTargetImmediate(CRenderHWAllocator::SHandle const * pMemory);

   void GcmGetOffsetAndLocation(CRenderHWAllocator::SHandle const * pMemory, uint32 * pOffset, uint32 * pLocation) const
   {
      cellGcmAddressToOffset(pMemory->mpAddress, (uint32_t*)pOffset);
      *pLocation = pMemory->mUserFlags;
   }

   static uint32 GcmGetPrimitiveType(CMeshChunk::EPrimitive const type);

   // specifically used for rendering OSD elements like system message boxes where the PPU control is handed to a different thread
   void ClearAndSwapForOSD();

   // Add a JTS blocker to the command buffer, the address returned points to the JTS command.
   // IMPORTANT: It is crucial not to add any further commands to the command buffer after this function is called until
   // the blocker has either been turned into a NOP or an SPU task has been started which will clear the blocker.
   // If this rule is not observed a dead lock situation can occur where the PPU tries to find space in the command buffer and waits
   // for the RSX to process commands, and the RSX is stuck in the JTS.
   void* AddJTSBlocker();

   // This allocates a command buffer for the given size
   // It is initialized with JTS's, NOPS and a RETURN at the very end.
   // NOTE: To free a command buffer call gpRenderBackend->Free. 
   // NOTE2: The returned allocation is 128 byte aligned, this makes it a little easier for an SPU DMA to overwrite it!
   CRenderHWAllocator::SHandle const* AllocateCommandBuffer(uint32 size);

   // Inserts a call to the allocated command buffer into the master command buffer
   // Make sure that by the time you call this function the JTS at the beginning is either already overwritten or the SPU job that will overwrite the JTS is already started.
   // Otherwise a dead lock can occur just like with the AddJTSBlocker function above.
   void CallCommandBuffer(CRenderHWAllocator::SHandle const* pCommandBuffer);

   void AddFragmentRenderCmd(NRenderQueue::SRenderCmd* pCommand);
   void WaitForFragmentRenderQueueIdle();

   CFragmentProgramAllocator* FragmentProgramAllocator() { return mpFragmentProgramAllocator.get(); }

   int const AllocateRSXLabel();

   CMatrix4 const &  GetViewMatrix() const { return mViewMatrix; }
   CMatrix4 const &  GetCameraMatrix() const { return mCameraMatrix; }
   CMatrix4 const &  GetProjectionTimesViewMatrix() const { return mProjectionTimesViewMatrix; }

   bool IsReadyToFlip() const;
   bool HasMoreFrameBuffersToFlip() const;
   uint32 GetFrameBuffersToFlip() const;

   void SPU_SyncDMA(uint32 const destEA, uint32 const sourceEA, uint32 const size);

   uint32 GetVBLCount() const { return mVBLCount; }

   //Threading support
   //----------------------------------------------------------------------------
public:
   typedef void (*TFnRenderThread)(int currentRenderBuffer);

   void StartThreadSystem(TFnRenderThread renderFunction);
   void ShutdownThreadSystem();

   void RunThreadFunction(int nextParam);
   void WaitThreadFunction();
   bool TryWaitThreadFunction();
   static void RenderThread(uint64_t arg);

public:
   void GcmIssueWaitForBackEndFinish();

   void SetProjectionMatrix(CMatrix4 const &matrix );

   void GcmWaitForIdle();
   void GcmFinish();

private:
   void BeginEdgeJobList(int segmentCountHint);

   void Present();

   static void GcmFlipHandler(uint32_t const /*head*/);
   static void GcmVBlankHandler(uint32_t const /*head*/);

   void TryFlipImmediate();

   void InitializeSPU();
   void ShutdownSPU();

   void InitializeSPUMemcpy();
   void ShutdownSPUMemcpy();

   void FlushProjectionTimesViewMatrix();

   void BeginFrameResourceTick();

#if BPE_USE_EDGE_GEOM
   void InitializeEdge();
   void ShutdownEdge();
#endif
private:
   uint32                              mBootupResolutionId;
   bool                                mDisplayIsInitialized;

   bool                                mRenderTargetsNeedsRebind;
   CBaseTexture::EAntiAliasType        mCurrentRenderTargetAAType;
   SRenderTarget                       mCurrentRenderTarget;

   uint32                              mBoundIndexBuffer;
   uint32                              mBoundIndexBufferLocation;
   uint32                              mBoundIndexBufferType;

   uint64                              mBoundVertexDataHash_1;
   uint64                              mBoundVertexDataHash_2;
   uint32                              mDisabledVertexAttributeMask;
   
   // Allocator for main memory
   void*                               mpHostMemory;
   CRenderHWAllocator*                 mpMainAllocator;

   // Allocator for local memory (video memory)
   CRenderHWAllocator*                 mpLocalAllocator;

   SDisplayBuffer                      mDisplayBufferInfo[3];
   // the buffer currently being rendered to
   int                                 mCurrentDisplayBuffer;

   STileInfo                           mTiles[15];
   SZTileInfo                          mZCullTiles[15];

   boost::scoped_ptr<CellSpursTaskset2>   mpFragmentTaskSet;
   boost::scoped_ptr<uint8>               mpFragmentTaskContextMemory;
   
   boost::scoped_ptr<CellSpursLFQueue> mpFragmentRenderQueue;
   boost::scoped_ptr<uint8>            mpFragmentRenderQueueBuffer;

   boost::scoped_ptr<CFragmentProgramAllocator> mpFragmentProgramAllocator;
   boost::scoped_ptr<CDynamicVertexBufferPool_RT> mpDynamicVertexBufferPool_RT;
   boost::scoped_ptr<CDynamicIndexBufferPool_RT> mpDynamicIndexBufferPool_RT;
   boost::scoped_ptr<CDynamicVertexBufferPool_UT> mpDynamicVertexBufferPool_UT;
   boost::scoped_ptr<CDynamicIndexBufferPool_UT> mpDynamicIndexBufferPool_UT;

   int                                 mRsxLabel;

   int                                 mWaitLabel;
   uint32                              mWaitLabelValue;

   bool                                mFogEnabled;
   CColorf                             mFogColor;

   CMatrix4                   mCameraMatrix;
   CMatrix4                   mViewMatrix;
   CMatrix4                   mProjectionTimesViewMatrix;

   bool                                mAllowImmediate;
   sys_lwmutex_t                       mFlipMutex;
   int                                 mFlipLabel;

   volatile uint32                     mLastFlipId;
   // number of vsyncs until we can flip again
   volatile int32                      mVSyncCountUntilNextFlip;
   // number of flips that have been queued up
   uint32                              mFlipsQueued;
   // number of flips that have been executed
   volatile uint32                     mFlipsExecuted;

   volatile uint32                     mVBLCount;

   CSyncCriticalSection                mCommandBufferCriticalSection;

public:
   int                                 mPresentationInterval;
};

//------------------------------------------------------------------------------------------

//Used for doing work while waiting for a free frame buffer rather than stalling in Present()
extern "C" int bpe_renderer_ready_to_flip();

//------------------------------------------------------------------------------------------
