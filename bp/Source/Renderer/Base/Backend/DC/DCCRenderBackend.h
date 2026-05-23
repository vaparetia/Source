//------------------------------------------------------------------------------------------
// DCCRenderBackend.h
// Dreamcast render backend — stub implementation targeting KallistiOS + PowerVR
//------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Backend/CRenderHWAllocator.h"
#include "Renderer/Base/Primitive/CIndexBuffer.h"
#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Frontend/RenderTypes.h"
#include "Engine/System/CSyncCriticalSection.h"
#include "Renderer/Base/Backend/DC/DCCTexture.h"
#include "Renderer/Base/Material/CShaderVertexDataBinding.h"

//----------------------------------------------------------------------------

#define RENDERBACKEND_SUPPORTS_ALPHA_TEST()                  0
#define RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET()  0

//----------------------------------------------------------------------------

class RENDERER_API CRenderBackend : public CBaseRenderBackend
{
   friend class CBaseRenderBackend;
public:
   // Mirror the blend/depth/stencil enums using plain integer values;
   // these will be translated to KallistiOS / OpenGL ES equivalents at
   // draw time once the rendering layer is implemented.

   enum EBlendFunc
   {
      kBF_Zero        = 0,
      kBF_One         = 1,
      kBF_SrcColor    = 2,
      kBF_InvSrcColor = 3,
      kBF_DstColor    = 4,
      kBF_InvDstColor = 5,
      kBF_SrcAlpha    = 6,
      kBF_InvSrcAlpha = 7,
      kBF_DstAlpha    = 8,
      kBF_InvDstAlpha = 9,
      kBF_FixedAlpha      = kBF_SrcAlpha,
      kBF_InvFixedAlpha   = kBF_InvSrcAlpha,
   };

   enum EBlendOp
   {
      kBO_Add         = 0,
      kBO_Subtract    = 1,
      kBO_RevSubtract = 2,
      kBO_Min         = 3,
      kBO_Max         = 4,
   };

   enum EAlphaFunc
   {
      kAF_Never,
      kAF_Less,
      kAF_Equal,
      kAF_LEqual,
      kAF_Greater,
      kAF_NotEqual,
      kAF_GEqual,
      kAF_Always,
      kAF_Count
   };

   enum EDepthFunc
   {
      kDF_Never   = 0,
      kDF_Less    = 1,
      kDF_Equal   = 2,
      kDF_LEqual  = 3,
      kDF_Greater = 4,
      kDF_NotEqual= 5,
      kDF_GEqual  = 6,
      kDF_Always  = 7,
   };

   enum EStencilOp
   {
      kSO_Keep     = 0,
      kSO_Zero     = 1,
      kSO_Replace  = 2,
      kSO_Incr     = 3,
      kSO_Decr     = 4,
      kSO_IncrWrap = 5,
      kSO_DecrWrap = 6,
      kSO_Invert   = 7,
   };

   enum EWrapMode
   {
      kWM_Wrap   = 0,
      kWM_Mirror = 1,
      kWM_Clamp  = 2,
      kWM_Border = 3,
   };

   enum EFilterMode
   {
      kFM_Nearest,
      kFM_Linear,
      kFM_Nearest_Nearest,
      kFM_Linear_Nearest,
      kFM_Nearest_Linear,
      kFM_Linear_Linear,
      kFM_Count
   };

   enum ECullMode
   {
      kCM_None = 0,
      kCM_CW   = 1,
      kCM_CCW  = 2,
   };

   enum EFPS
   {
      kFPS_30,
      kFPS_60
   };

   // Display constants matching Dreamcast native resolution
   static const int skDisplayWidth  = 640;
   static const int skDisplayHeight = 480;
   static const int DISPLAY_BUFFER_COUNT = 2;

   explicit CRenderBackend(IResourcePool & resourcePool,
      SRenderInitialization const & initFlags);
   ~CRenderBackend();

   void SetVertexData(CShaderVertexDataBinding const & vertexDataBinding,
      CVertexData const & vertexData,
      uint64 const vertexDataHash_1,
      uint64 const vertexDataHash_2);
   void ForceVertexDataRebind();

   void RenderPrimitives(CMeshChunk::EPrimitive type,
      uint32 const indexBufferOffset, uint32 const indexCount);
   void RenderPrimitivesInstanced(CMeshChunk::EPrimitive type,
      uint32 const indexBufferOffset, uint32 const indexCount, uint32 const indexWrap);
   void RenderPrimitivesNoIndices(CMeshChunk::EPrimitive type,
      uint32 const vertexBufferOffset, uint32 const vertexCount);
   void BeginRenderPrimitivesCustom();
   void EndRenderPrimitivesCustom();

   void RenderQuadVC(real32 const minX, real32 const maxX,
      real32 const minY, real32 const maxY, real32 const z,
      real32 const minU, real32 const maxU,
      real32 const minV, real32 const maxV);

   void RenderPrimitivesUserVertexData(CMeshChunk::EPrimitive type,
      CShaderVertexDataBinding binding,
      void const * pData,
      uint32 const numVertices);

   void Clear(int const clearFlags, CColor const color,
      real32 const z = 1.0f, int const stencil = 0);
   void BeginScene();
   void EndScene();
   void ShutDown();

   void SetRenderTarget(SRenderTarget const & renderTarget);
   void SetViewport();

   SRenderTarget const & GetCurrentRenderTarget() const { return mCurrentRenderTarget; }
   CBaseTexture * GetCurrentRenderTexture() const { return mCurrentRenderTarget.mpColorBuffer[0]; }
   CBaseTexture * GetCurrentRenderDepth()   const { return mCurrentRenderTarget.mpDepthBuffer; }

   CBaseTexture * GetCurrentDisplayTexture() const { return mpDisplayTexture; }
   CBaseTexture * GetCurrentDisplayDepth()   const { return mpDisplayDepth; }
   CBaseTexture * GetCurrentDrawTexture()    const { return mpDisplayTexture; }
   CBaseTexture * GetCurrentDrawDepth()      const { return mpDisplayDepth; }

   void SetIndexData(CIndexBuffer const * indexBuffer);
   void SetIndexData(CIndexBufferChunk const & chunk);

   void SetBlendMode(bool const enable,
      EBlendFunc const srcBlendMode, EBlendFunc const dstBlendMode);
   void SetBlendMode(bool const enable,
      EBlendFunc const srcBlendMode, EBlendFunc const dstBlendMode,
      uint32 const fixedColor);
   void SetBlendMode(bool const enable,
      EBlendFunc const rgbSrc, EBlendFunc const rgbDst,
      EBlendFunc const aSrc,   EBlendFunc const aDst);
   void SetBlendMode(bool const enable,
      EBlendFunc const rgbSrc, EBlendFunc const rgbDst,
      EBlendFunc const aSrc,   EBlendFunc const aDst,
      uint32 const rgbaFixed);

   void SetBlendOp(EBlendOp const blendOp);
   void SetBlendOp(EBlendOp const colorBlendOp, EBlendOp const alphaBlendOp);

   void DisableBlend();
   void SetDepthCompareEnabled(bool const enable);
   void SetDepthFunc(EDepthFunc const func);
   void SetColorAlphaWriteEnabled(bool const color, bool const alpha);
   void SetDepthWriteEnabled(bool const enable);
   bool GetDepthWriteEnabled() const;
   void SetCullMode(ECullMode const mode);

   void SetStencilEnable(bool enable);
   void SetStencilOp(EStencilOp fail, EStencilOp depthFail, EStencilOp depthPass);
   void SetStencilMask(uint32 mask);
   void SetStencilFunc(EAlphaFunc func, uint32 ref, uint32 mask);

   void SetRasterMask(bool enable);
   void SetDepthBias(int slope, int bias);

   CMatrix4 const & GetViewMatrix()                  const { return mViewMatrix; }
   CMatrix4 const & GetCameraMatrix()                const { return mCameraMatrix; }
   CMatrix4 const & GetProjectionTimesViewMatrix()   const { return mProjectionTimesViewMatrix; }

   void UpdateVBLCount();
   uint32 GetVBLCount() const { return mVBLCount; }

   void SetTexture(int const texUnit, CBaseTexture const * pTexture,
      bool const allowAniso = true);
   void SetTextureAddressMode(int const texUnit,
      EWrapMode const wrapU, EWrapMode const wrapV);
   void SetTextureFilter(int const texUnit,
      EFilterMode minFilter, EFilterMode magFilter);

   void SetVertexRegisters(int startRegister, int numVectors,
      CVector4 const * pVector);
   void SetFragmentRegisters(int startRegister, int numVectors,
      CVector4 const * pVector);

   // Tile resolve stubs — PowerVR handles tiling internally; these are no-ops.
   void BeginTiling()  {}
   void EndTiling()    {}
   void ResolveRenderTarget(CBaseTexture *)                            {}
   void ResolveRenderTargetPredicated(CBaseTexture *)                 {}
   void ResolveDepthStencilPredicated(CBaseTexture *)                 {}
   void ResolveDepthStencilPredicatedMSAA2(CBaseTexture *, CBaseTexture *)   {}
   void ResolveDepthStencilPredicatedMSAA4(CBaseTexture *, CBaseTexture *,
                                            CBaseTexture *, CBaseTexture *) {}
   void ResolvePartialRenderTarget(CBaseTexture *, SRect const &)     {}

   CRenderHWAllocator::SHandle const * AllocFixed(int const size,
      int const alignment, ERenderMemory const memoryType,
      ERenderMemory const fallbackMemory = kRM_System);
   void Free(CRenderHWAllocator::SHandle const * pHandle);
   void FreeImmediate(CRenderHWAllocator::SHandle const * pHandle);

   void GetMemoryStats(ERenderMemory const memory,
      SRenderHWAllocatorStats * pStats) const;

   void SetUnsafeVertexUniformBuffer(int const bufferIndex, void const * ptr);
   void SetStaticVertexUniformBuffer(int const bufferIndex, void const * ptr);
   void * AllocOneFrameVertexUniformBuffer(int const bufferIndex, size_t const size);
   template <class T>
   T * AllocOneFrameVertexUniformBufferTyped(int const bufferIndex)
   {
      return reinterpret_cast<T *>(AllocOneFrameVertexUniformBuffer(bufferIndex, sizeof(T)));
   }

   void SetUnsafeFragmentUniformBuffer(int const bufferIndex, void const * ptr);
   void SetStaticFragmentUniformBuffer(int const bufferIndex, void const * ptr);
   void * AllocOneFrameFragmentUniformBuffer(int const bufferIndex, size_t const size);
   template <class T>
   T * AllocOneFrameFragmentUniformBufferTyped(int const bufferIndex)
   {
      return reinterpret_cast<T *>(AllocOneFrameFragmentUniformBuffer(bufferIndex, sizeof(T)));
   }

   EFPS const  GetTargetFPS() const { return mTargetFPS; }
   void        SetTargetFPS(EFPS const fps) { mTargetFPS = fps; }

   typedef void (*TFnRenderThread)(int currentRenderBuffer);
   void StartThreadSystem(TFnRenderThread nextThread);
   void RunThreadFunction(int nextParam);
   void WaitThreadFunction();
   bool TryWaitThreadFunction();
   void ShutdownThreadSystem();

   void AcquireRenderThreadOwnership();
   void ReleaseRenderThreadOwnership();

   CSyncCriticalSection mCommandBufferCriticalSection;

private:
   void BeginFrameResourceTick();
   void FlushProjectionTimesViewMatrix();

   CMatrix4           mCameraMatrix;
   CMatrix4           mViewMatrix;
   CMatrix4           mProjectionTimesViewMatrix;

   SRenderTarget      mCurrentRenderTarget;

   CBaseTexture *     mpDisplayTexture;
   CBaseTexture *     mpDisplayDepth;

   uint32             mVBLCount;
   EFPS               mTargetFPS;

   bool               mDepthWriteEnabled;
   bool               mBlendEnabled;
   EBlendFunc         mBlendSrcRGB;
   EBlendFunc         mBlendDstRGB;
   EBlendFunc         mBlendSrcA;
   EBlendFunc         mBlendDstA;
   EDepthFunc         mDepthFunc;
   ECullMode          mCullMode;
};

//----------------------------------------------------------------------------

inline void CRenderBackend::SetDepthCompareEnabled(bool const /*enable*/) {}
inline void CRenderBackend::SetDepthFunc(EDepthFunc const func) { mDepthFunc = func; }
inline void CRenderBackend::SetDepthWriteEnabled(bool const enable) { mDepthWriteEnabled = enable; }
inline bool CRenderBackend::GetDepthWriteEnabled() const { return mDepthWriteEnabled; }
inline void CRenderBackend::SetCullMode(ECullMode const mode) { mCullMode = mode; }

inline void CRenderBackend::SetUnsafeFragmentUniformBuffer(int const /*bufferIndex*/, void const * /*ptr*/) {}
inline void CRenderBackend::SetStaticFragmentUniformBuffer(int const /*bufferIndex*/, void const * /*ptr*/) {}

//----------------------------------------------------------------------------
