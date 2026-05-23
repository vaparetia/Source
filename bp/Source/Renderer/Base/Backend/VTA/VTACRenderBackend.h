//------------------------------------------------------------------------------------------
// VTACRenderBackend.h
// Bluepoint/Armature
// Copyright 2011
//------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

class CDynamicVertexBufferPool_RT;
class CDynamicIndexBufferPool_RT;
class CDynamicVertexBufferPool_UT;
class CDynamicIndexBufferPool_UT;
class CDynamicIndexBufferPoolChunk_RT;
class CDynamicIndexBufferPoolChunk_UT;

struct SFragmentParameter;
struct SVertexParameter;

#include <scetypes.h>
#include <gxm\context.h>
#include <gxm\shader_patcher.h>
#include "Renderer/Base/Backend/CRenderHWAllocator.h"
#include "Renderer/Base/Primitive/CIndexBuffer.h"
#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Frontend/RenderTypes.h"
#include "Engine/System/CSyncCriticalSection.h"
#include "Renderer/Base/Backend/VTA/NVTAState.h"

#define RENDERBACKEND_SUPPORTS_ALPHA_TEST() 0
#define RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET() 0

namespace NVTAState
{
   extern float32x4_t mVertexRegisters[ 256 ];
   extern float32x4_t mFragmentRegisters[ 256 ];
}

namespace NVTATextureState
{
   void SetTextureData( int const texUnit, SceGxmTexture const *texture );
   void SetupDefaultStates( SceGxmTexture *texture );
}

class RENDERER_API CRenderBackend : public CBaseRenderBackend
{
   friend class CBaseRenderBackend;
public:
   enum EBlendFunc
   {
      kBF_Zero        = SCE_GXM_BLEND_FACTOR_ZERO,

      kBF_One         = SCE_GXM_BLEND_FACTOR_ONE,

      kBF_SrcColor    = SCE_GXM_BLEND_FACTOR_SRC_COLOR,
      kBF_InvSrcColor = SCE_GXM_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,

      kBF_DstColor    = SCE_GXM_BLEND_FACTOR_DST_COLOR,
      kBF_InvDstColor = SCE_GXM_BLEND_FACTOR_ONE_MINUS_DST_COLOR,

      kBF_SrcAlpha   = SCE_GXM_BLEND_FACTOR_SRC_ALPHA,
      kBF_InvSrcAlpha= SCE_GXM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,

      kBF_DstAlpha   = SCE_GXM_BLEND_FACTOR_DST_ALPHA,
      kBF_InvDstAlpha= SCE_GXM_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,

      kBF_FixedAlpha = SCE_GXM_BLEND_FACTOR_SRC_ALPHA, // VITA TODO FIX
      kBF_InvFixedAlpha = SCE_GXM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, // VITA TODO FIX

//      kBF_FixedColor,
//      kBF_InvFixedColor,

//      kBF_Count
   };

   enum EBlendOp
   {
      kBO_Add           = SCE_GXM_BLEND_FUNC_ADD,
      kBO_Subtract      = SCE_GXM_BLEND_FUNC_SUBTRACT,
      kBO_RevSubtract   = SCE_GXM_BLEND_FUNC_REVERSE_SUBTRACT,
      kBO_Min           = SCE_GXM_BLEND_FUNC_MIN,
      kBO_Max           = SCE_GXM_BLEND_FUNC_MAX,

//      kBO_Count
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
      kDF_Never      = SCE_GXM_DEPTH_FUNC_NEVER,
      kDF_Less       = SCE_GXM_DEPTH_FUNC_LESS,
      kDF_Equal      = SCE_GXM_DEPTH_FUNC_EQUAL,
      kDF_LEqual     = SCE_GXM_DEPTH_FUNC_LESS_EQUAL,
      kDF_Greater    = SCE_GXM_DEPTH_FUNC_GREATER,
      kDF_NotEqual   = SCE_GXM_DEPTH_FUNC_NOT_EQUAL,
      kDF_GEqual     = SCE_GXM_DEPTH_FUNC_GREATER_EQUAL,
      kDF_Always     = SCE_GXM_DEPTH_FUNC_ALWAYS,

//      kDF_Count
   };

   enum EStencilOp
   {
      kSO_Keep       = SCE_GXM_STENCIL_OP_KEEP,
      kSO_Zero       = SCE_GXM_STENCIL_OP_ZERO,
      kSO_Replace    = SCE_GXM_STENCIL_OP_REPLACE,
      kSO_Incr       = SCE_GXM_STENCIL_OP_INCR,
      kSO_Decr       = SCE_GXM_STENCIL_OP_DECR,
      kSO_IncrWrap   = SCE_GXM_STENCIL_OP_INCR_WRAP,
      kSO_DecrWrap   = SCE_GXM_STENCIL_OP_DECR_WRAP,
      kSO_Invert     = SCE_GXM_STENCIL_OP_INVERT,

//      kSO_Count
   };

   enum EWrapMode
   {
      kWM_Wrap       = SCE_GXM_TEXTURE_ADDR_REPEAT,
      kWM_Mirror     = SCE_GXM_TEXTURE_ADDR_MIRROR,
      kWM_Clamp      = SCE_GXM_TEXTURE_ADDR_CLAMP,
      kWM_Border     = SCE_GXM_TEXTURE_ADDR_CLAMP_FULL_BORDER,

//      kWM_Count
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
      kCM_None = SCE_GXM_CULL_NONE,
      kCM_CW   = SCE_GXM_CULL_CW,
      kCM_CCW  = SCE_GXM_CULL_CCW
   };

   enum EFPS
   {
      kFPS_30,
      kFPS_60
   };

   struct SInternalRenderTarget
   {
      SceGxmRenderTarget *rt;
      int width, height;
      int flags;
      bool tiled;
      int msaa;
      int scenes;
      SceUID uid;
      SceUInt32 size;
   };

   static int const skMaxUniformBuffers = 14;
   
   explicit CRenderBackend(IResourcePool & resourcePool,
      SRenderInitialization const & initFlags);
   ~CRenderBackend();

   void  SetVertexData(NVTAState::SCachedVertexData *cvd) { NVTAState::SetCachedVertexData( cvd ); }
   void  SetVertexData(CShaderVertexDataBinding const & vertexDataBinding, CVertexData const & vertexData, uint64 const vertexDataHash_1, uint64 const vertexDataHash_2);
   void  ForceVertexDataRebind();

   void RenderPrimitives(CMeshChunk::EPrimitive type, /* uint32 const vertexBufferOffset, uint32 const vertexCount, */ uint32 const indexBufferOffset, uint32 const indexCount);
   void RenderPrimitivesInstanced(CMeshChunk::EPrimitive type, /* uint32 const vertexBufferOffset, uint32 const vertexCount, */ uint32 const indexBufferOffset, uint32 const indexCount, uint32 const indexWrap);

   void RenderPrimitivesNoIndices(CMeshChunk::EPrimitive type, uint32 const vertexBufferOffset, uint32 const vertexCount);
   void BeginRenderPrimitivesCustom();
   void EndRenderPrimitivesCustom();

   void RenderQuadVC(real32 const minX, real32 const maxX, real32 const minY, real32 const maxY, real32 const z, real32 const minU, real32 const maxU, real32 const minV, real32 const maxV);

   void RenderPrimitivesUserVertexData(CMeshChunk::EPrimitive type, 
      CShaderVertexDataBinding binding, 
      void const * pData, 
      uint32 const numVertices);

   void DownsampleClear(CColor const color, CBaseTexture *depth);
   void Clear(int const clearFlags, CColor const color, real32 const z = 1.0f, int const stencil = 0);
   void BeginScene();
   void EndScene();
   void ShutDown();

   void EndCapture();

   void SetRenderTarget(SRenderTarget const & renderTarget);
   void ChangeScene(int const newTarget, SRenderTarget const & renderTarget);
   void BackupScene(CBaseTexture *dst);
   void SetViewport();

   SRenderTarget const & GetCurrentRenderTarget() const { return mCurrentRenderTarget; }
   CBaseTexture * GetCurrentRenderTexture() const { return mCurrentRenderTarget.mpColorBuffer[0]; }
   CBaseTexture * GetCurrentRenderDepth() const { return mCurrentRenderTarget.mpDepthBuffer; }

   CBaseTexture * GetPreviousDisplayTexture() const { return s_dispTex[s_dispFront]; }
   CBaseTexture * GetCurrentDisplayTexture() const { return s_dispTex[s_dispBack]; }
   CBaseTexture * GetCurrentDisplayDepth() const { return s_depthTex; }
   CBaseTexture * GetCurrentDrawTexture() const { return s_fullResCopied ? s_dispTex[s_dispBack] : s_drawColor; }
   CBaseTexture * GetCurrentDrawDepth() const { return s_fullResCopied ? s_depthTex : s_drawDepth; }
   CBaseTexture * GetLowresDrawTexture() const { return s_drawColor; }
   CBaseTexture * GetLowresDrawDepth() const { return s_drawDepth; }

   bool IsInLowresScene() { return !s_fullResCopied; }
   int GetVisibility(int frame, int slot);
   void EnableVisibility(int slot);
   void DisableVisibility();

   int FindRenderTarget( uint32 width, uint32 height, int numScenes, int MSAATrick, int xTiles, int yTiles );
   bool MakeRenderTarget(SInternalRenderTarget *renderTarget, uint32 const width, uint32 const height, int const numScenes, int const MSAATrick, int const xTiles, int const yTiles);

   void SetUpscaleOverride( void (*func)() );
   void OverrideUpscale();

   //   CBaseTexture::EAntiAliasType const GetCurrentRenderTargetAAType() const { return mCurrentRenderTargetAAType; }

   void SetIndexData(CIndexBuffer const * indexBuffer);
   void SetIndexData(CIndexBufferChunk const &chunk );
   void SetIndexData_OneToOne( uint16 const count );
   void SetIndexSourceOverwrite( uint32 const streamIndex, SceGxmIndexSource indexSource );

   void SetBlendMode(bool const enable, EBlendFunc const srcBlendMode, EBlendFunc const dstBlendMode);
   void SetBlendMode(bool const enable, EBlendFunc const srcBlendMode, EBlendFunc const dstBlendMode, uint32 const fixedColor);
   void SetBlendMode(bool const enable, EBlendFunc const rgbSrcBlendMode, EBlendFunc const rgbDstBlendMode, EBlendFunc const aSrcBlendMode, EBlendFunc const aDstBlendMode);
   void SetBlendMode(bool const enable, EBlendFunc const rgbSrcBlendMode, EBlendFunc const rgbDstBlendMode, EBlendFunc const aSrcBlendMode, EBlendFunc const aDstBlendMode, uint32 const rgbaFixed);

   void SetBlendOp(EBlendOp const blendOp);
   void SetBlendOp(EBlendOp const colorBlendOp, EBlendOp const alphaBlendOp);

   void DisableBlend() { NVTAState::BlendDisable(); }
   void SetBlend(uint32 const blend) { NVTAState::BlendEnableAndSetBlend( blend ); }
   uint32 GetBlend() { return NVTAState::BlendGetBlend(); }
   bool GetBlendEnabled() { return NVTAState::BlendGetEnabled(); }

   void SetDepthCompareEnabled(bool const enable);
   void SetDepthFunc(EDepthFunc const func);
   void SetColorAlphaWriteEnabled(bool const color, bool const alpha);

   void SetDepthWriteEnabled(bool const enable);
   bool GetDepthWriteEnabled() const;
   void SetCullMode(ECullMode const mode);

   // enable disable stencil testing
   void SetStencilEnable(bool enable);
   void SetStencilOp(EStencilOp fail, EStencilOp depthFail, EStencilOp depthPass );
   void SetStencilMask(uint32 mask);
   void SetStencilFunc(EAlphaFunc func, uint32 ref, uint32 mask);

   void SetRasterMask(bool enable);
   void SetDepthBias(int slope, int bias);

   CMatrix4 const &  GetViewMatrix() const { return mViewMatrix; }
   CMatrix4 const &  GetCameraMatrix() const { return mCameraMatrix; }
   CMatrix4 const &  GetProjectionTimesViewMatrix() const { return mProjectionTimesViewMatrix; }

   void UpdateVBLCount();
   uint32 GetVBLCount() const { return mVBLCount; }

   void SetTexture(int const texUnit, CBaseTexture const * pTexture, bool const allowAniso = true);
   void SetTextureAddressMode(int const texUnit, EWrapMode const wrapU, EWrapMode const wrapV);
   void SetTextureFilter(int const texUnit, EFilterMode minFilter, EFilterMode magFilter);

   void SetVertexRegisters(int startRegister, int numVectors, CVector4 const * pVector);
   void SetFragmentRegisters(int startRegister, int numVectors, CVector4 const * pVector);

   CDynamicVertexBufferPool_RT* GetVertexBufferPool_RT() const { return mpDynamicVertexBufferPool_RT.get(); }
   CDynamicIndexBufferPool_RT* GetIndexBufferPool_RT() const { return mpDynamicIndexBufferPool_RT.get(); }
   CDynamicVertexBufferPool_UT* GetVertexBufferPool_UT() const { return mpDynamicVertexBufferPool_UT.get(); }
   CDynamicIndexBufferPool_UT* GetIndexBufferPool_UT() const { return mpDynamicIndexBufferPool_UT.get(); }

   void BeginTiling() {}
   void EndTiling() {}
   void ResolveRenderTarget(CBaseTexture * pRenderTexture) {}
   void ResolveRenderTargetPredicated(CBaseTexture * pRenderTexture) {}
   void ResolveDepthStencilPredicated(CBaseTexture * pRenderTexture) {}
   void ResolveDepthStencilPredicatedMSAA2(CBaseTexture * pRenderTexture0, CBaseTexture * pRenderTexture1) {}
   void ResolveDepthStencilPredicatedMSAA4(CBaseTexture * pRenderTexture0, CBaseTexture * pRenderTexture1, CBaseTexture * pRenderTexture2, CBaseTexture * pRenderTexture3) {}
   void ResolvePartialRenderTarget(CBaseTexture * pRenderTexture, SRect const & srcRect) {}

   // TODO - Temporarily falling back to system memory if there is not a big enough free block in video memory.
   CRenderHWAllocator::SHandle const * AllocFixed(int const size, int const alignment, ERenderMemory const memoryType, ERenderMemory const fallbackMemory = kRM_System);
   
   void Free(CRenderHWAllocator::SHandle const * pHandle);
   void FreeImmediate( CRenderHWAllocator::SHandle const *pHandle );

   void GetMemoryStats(ERenderMemory const memory, SRenderHWAllocatorStats * pStats) const;

   // Sets (or allocates and sets) managed vertex uniform buffers
   // bufferIndex is from 0 .. skMaxUniformBuffers
   void SetManagedVertexUniformBuffer( int bufferIndex, CRenderHWAllocator::SHandle const *pHandle );
   void SetUnsafeVertexUniformBuffer( int const bufferIndex, void const *ptr );
   void SetStaticVertexUniformBuffer( int const bufferIndex, void const *ptr );
   void *AllocOneFrameVertexUniformBuffer( int const bufferIndex, size_t const size );
   template <class T>
   T *AllocOneFrameVertexUniformBufferTyped( int const bufferIndex )
   {
      return reinterpret_cast<T *>( AllocOneFrameVertexUniformBuffer( bufferIndex, sizeof( T ) ) );
   }

   // Same for fragment uniform buffers
   void SetUnsafeFragmentUniformBuffer( int const bufferIndex, void const *ptr );
   void SetStaticFragmentUniformBuffer( int const bufferIndex, void const *ptr );
   void *AllocOneFrameFragmentUniformBuffer( int const bufferIndex, size_t const size );
   template <class T>
   T *AllocOneFrameFragmentUniformBufferTyped( int const bufferIndex )
   {
      return reinterpret_cast<T *>( AllocOneFrameFragmentUniformBuffer( bufferIndex, sizeof( T ) ) );
   }

   SceGxmContext *Context() { return s_context; }

   static const int skDrawWidth           = 768;
   static const int skDrawHeight          = 480;
   static const int skDisplayWidth			= 960;
   static const int skDisplayHeight			= 544;
   static const int DISPLAY_STRIDE			= 1024;

   static const int DISPLAY_BUFFER_COUNT	= 3;
   static const int DISPLAY_PENDING_SWAPS	= 2;
   static const int DISPLAY_BUFFER_SIZE	= 	((4 * DISPLAY_STRIDE * skDisplayHeight + 0xfffffU) & ~0xfffffU);
   static const int DISPLAY_ALIGN_WIDTH	= 	((skDisplayWidth  + SCE_GXM_TILE_SIZEX - 1) & ~(SCE_GXM_TILE_SIZEX - 1));
   static const int DISPLAY_ALIGN_HEIGHT	= ((skDisplayHeight + SCE_GXM_TILE_SIZEY - 1) & ~(SCE_GXM_TILE_SIZEY - 1));
   // large enough to cover both 960x544 and 1024x512
   static const int DISPLAY_DEPTH_SIZE    = (1024 * 512 * 4);

   static const int VISIBILITY_SLOTS      = 4; // only 2 of them are usable

   static const int INTERNAL_RENDERTARGET_COUNT = 12;

   static const int PATCHER_BUFFER_SIZE	= 		(64*1024);
   static const int PATCHER_COMBINED_USSE_SIZE	= (128*1024);

   static float32x4_t const *GetVertexRegsDataPtr( int const registerIndex ) { return &NVTAState::mVertexRegisters[ registerIndex ]; }
   static float32x4_t const *GetFragmentRegsDataPtr( int const registerIndex ) { return &NVTAState::mFragmentRegisters[ registerIndex ]; }

   typedef void (*TFnRenderThread)(int currentRenderBuffer);
   void StartThreadSystem(TFnRenderThread nextThread);
   void RunThreadFunction(int nextParam);
   void WaitThreadFunction();
   bool TryWaitThreadFunction();
   void ShutdownThreadSystem();
   uint64 const GetThreadLastCPUTimeUs() const { return ( mThreadLastCPUTime < mLastDisplayQueueFlipTime ) ? 0 : ( mThreadLastCPUTime - mLastDisplayQueueFlipTime ); }

   void AcquireRenderThreadOwnership();
   void ReleaseRenderThreadOwnership();

   void PrintLiveMetrics();
   void PrintTimers();

   void RenderStartIdle() { mRenderIdleTimer.Reset(); }
   void RenderEndIdle() { mRenderElapsed += mRenderIdleTimer.GetElapsedTime() * 1000.0f; }
   void SetFrameTime(float time) { mDisplayElapsed = time; }

   void VitaRepeatPriorDraw();
   bool VitaReadGPUCanDo60FPS();
private:

   void BeginFrameResourceTick();
   void FlushProjectionTimesViewMatrix();
   void FlushDrawState();
   void UpdateCommonDialog();
   void WrapGxmDraw( CMeshChunk::EPrimitive primType, 
      SceGxmIndexFormat indexType, 
      const void *indexData, 
      uint32_t indexCount );
   void WrapGxmDraw( SceGxmPrimitiveType primType, 
      SceGxmIndexFormat indexType, 
      const void *indexData, 
      uint32_t indexCount );
   void WrapGxmDrawInstanced( CMeshChunk::EPrimitive primType, 
      SceGxmIndexFormat indexType, 
      const void *indexData, 
      uint32_t indexCount,
      uint32_t indexWrap );
   void WrapGxmDrawInstanced( SceGxmPrimitiveType primType, 
      SceGxmIndexFormat indexType, 
      const void *indexData, 
      uint32_t indexCount,
      uint32_t indexWrap );

   void SetScissor( int const sX, int const sY, int const sWidth, int const sHeight );
   void ResetRasterMask();
   void ResetScissor();
   void DisableScissor();
   void EnableScissor();
   void UpscaleDrawBuffer();
   void FlushStencilTest();

   boost::scoped_ptr<CDynamicVertexBufferPool_RT> mpDynamicVertexBufferPool_RT;
   boost::scoped_ptr<CDynamicIndexBufferPool_RT> mpDynamicIndexBufferPool_RT;
   boost::scoped_ptr<CDynamicVertexBufferPool_UT> mpDynamicVertexBufferPool_UT;
   boost::scoped_ptr<CDynamicIndexBufferPool_UT> mpDynamicIndexBufferPool_UT;

   CMatrix4                   mCameraMatrix;
   CMatrix4                   mViewMatrix;
   CMatrix4                   mProjectionTimesViewMatrix;

   SRenderTarget              mCurrentRenderTarget;

   CRenderHWAllocator*                 mpRenderAllocators[2];
   SceUID                              mRenderAllocUIDs[2];

   void						*s_contextHost;
   SceUID					s_vdmRingBufUid;
   SceUID					s_vertexRingBufUid;
   SceUID					s_fragmentRingBufUid;
   SceUID					s_fragmentUsseRingBufUid;
   SceGxmContext			*s_context;

   // shader patcher
   SceUID					s_patcherBufUid;
   SceUID					s_patcherCombinedUsseUid;

   // render target
   SceGxmRenderTarget		*s_renderTarget;
   SInternalRenderTarget    s_internalRenderTargets[INTERNAL_RENDERTARGET_COUNT];

   // main 3D target texture
   SceUID               s_drawColorUid;
   SceUID               s_drawDepthUid;
   void                 *s_drawColorBuf;
   void                 *s_drawDepthBuf;
   CBaseTexture         *s_drawColor;
   CBaseTexture         *s_drawDepth;

   // display buffer variables
   SceUID					s_dispUid[DISPLAY_BUFFER_COUNT];
   void						*s_dispBuf[DISPLAY_BUFFER_COUNT];
   SceGxmSyncObject			*s_dispSync[DISPLAY_BUFFER_COUNT];
   SceGxmColorSurface		s_dispSurface[DISPLAY_BUFFER_COUNT];
   CBaseTexture         *s_dispTex[DISPLAY_BUFFER_COUNT];
   SceUInt32				s_dispFront;
   SceUInt32				s_dispBack;
   SceUInt32            s_dispFrame[DISPLAY_BUFFER_COUNT];
   CStopWatch           s_dispTimer[DISPLAY_BUFFER_COUNT];
   float                s_dispElapsed[DISPLAY_BUFFER_COUNT];
   SceGxmNotification   s_dispNote[DISPLAY_BUFFER_COUNT];

   // depth buffer variables
   SceGxmDepthStencilSurface	s_depthSurface;
   SceUID						s_depthUid;
   void							*s_depthBuf;
   CBaseTexture            *s_depthTex;

   // visibility buffers
   SceUID               s_visibilityUid;
   uint32               *s_visibilityBuf;
   uint32               *s_visibility[DISPLAY_BUFFER_COUNT];

   bool                 s_inScene;
   bool                 s_fullResCopied;

   SceGxmMultisampleMode s_currentMultisampleMode;
   SceGxmOutputRegisterFormat s_currentOutputFormat;

   CRenderHWAllocator::SHandle const *mpNullAttribute;

   uint8 const *           mpLastIndexBuffer;
   size_t                  mLastIndexBufferCount;

   bool mRasterMaskEnabled;
   bool mRasterMaskValid;
   uint32                  mVBLCount;

   EFPS                    mTargetFPS;
public:
   uint64               s_dispDisplayHardwareAt60[ DISPLAY_BUFFER_COUNT ];

   EFPS const              GetTargetFPS() const { return mTargetFPS; }
   void                    SetTargetFPS( EFPS const fps ) { mTargetFPS = fps; }

   SceGxmOutputRegisterFormat GetCurrentOutputFormat() { return s_currentOutputFormat; }
   SceGxmMultisampleMode GetCurrentMultisampleMode() { return s_currentMultisampleMode; }

   CSyncCriticalSection mCommandBufferCriticalSection;
   uint64 mThreadLastCPUTime;
   uint64 mLastDisplayQueueFlipTime;
private:
//   uint32 **mppParameters;
//   uint32 **mppParametersEnd;

   SceGxmShaderPatcherId clearVertexProgramId;
   SceGxmShaderPatcherId clearFragmentProgramId;
   SceGxmShaderPatcherId scissorFragmentProgramId;
   SceGxmShaderPatcherId maskFragmentProgramId;
   SceGxmShaderPatcherId maskHalfFragmentProgramId;
   SceGxmShaderPatcherId upscaleVertexProgramId;
   SceGxmShaderPatcherId upscaleFragmentProgramId;
   SceGxmShaderPatcherId downclearVertexProgramId;
   SceGxmShaderPatcherId downclearFragmentProgramId;
   SceGxmVertexProgram *clearVertexProgram;
   SceGxmFragmentProgram *clearFragmentProgram;
   SceGxmFragmentProgram *scissorFragmentProgram;
   SceGxmFragmentProgram *maskFragmentProgram;
   SceGxmVertexProgram *upscaleVertexProgram;
   SceGxmFragmentProgram *upscaleFragmentProgram;
   SceGxmVertexProgram *downclearVertexProgram;
   SceGxmFragmentProgram *downclearFragmentProgram;

   SceGxmFragmentProgram *clearFragmentProgramHalf;
   SceGxmFragmentProgram *scissorFragmentProgramHalf;
   SceGxmFragmentProgram *maskFragmentProgramHalf;
   SceGxmFragmentProgram *downclearFragmentProgramHalf;

   int scissorTop, scissorLeft, scissorRight, scissorBottom;
   int scissorTrivial;

   int mCurrentSceneDrawNum;
   int mMSAATrickEnabled;

   boost::scoped_ptr<CStaticIndexBuffer> mpNoPrimitiveIndexBuffer;
   boost::scoped_ptr<CStaticVertexBuffer> mpRasterMaskVertexBuffer;

   CRenderHWAllocator::SHandle const *mpVertexUniformHandles[skMaxUniformBuffers];

   CStopWatch mFrameTimer;

   CStopWatch mRenderIdleTimer;
   CStopWatch mGameWaitTimer;
   CStopWatch mDisplayWaitTimer;
   CStopWatch mRenderBusyTimer;

   float mRenderElapsed;
   float mGameElapsed;
   float mDisplayElapsed;
   float mRenderBusyElapsed;

   bool mOverrideUpscale;
   void (*mpUpscaleFunc)();

   friend SceInt32 bp_note_thread(SceSize, void*);
};

//----------------------------------------------------------------------------

inline void CRenderBackend::SetDepthCompareEnabled(bool const enable)
{
   NVTAState::DepthSetUnitEnabled( enable );
}

inline void CRenderBackend::SetDepthFunc(EDepthFunc const func)
{
   NVTAState::DepthSetFunc( SceGxmDepthFunc( func ) );
}

inline void CRenderBackend::SetDepthWriteEnabled(bool const enable) 
{
   NVTAState::DepthSetWriteEnabled( enable );
}

inline bool CRenderBackend::GetDepthWriteEnabled() const
{
   return NVTAState::DepthGetWriteEnabled();
}

inline void CRenderBackend::SetCullMode(ECullMode const mode) 
{
   NVTAState::CullSetMode( SceGxmCullMode( mode ) );
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetUnsafeFragmentUniformBuffer( int const bufferIndex, void const *ptr )
{
   sceGxmSetFragmentUniformBuffer( Context(), bufferIndex, ptr );
}

//----------------------------------------------------------------------------

inline void CRenderBackend::SetStaticFragmentUniformBuffer( int const bufferIndex, void const *ptr )
{
   sceGxmSetFragmentUniformBuffer( Context(), bufferIndex, ptr );
}
