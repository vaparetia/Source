//------------------------------------------------------------------------------------------
// DCCRenderBackend.cpp
// Dreamcast render backend — KallistiOS + PowerVR implementation
//------------------------------------------------------------------------------------------

#include "Engine/StdAfx.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

#include <dc/pvr.h>
#include <dc/vblank.h>

//----------------------------------------------------------------------------

void CRenderBackend::VBLHandler(uint32 /*code*/, void *data)
{
   static_cast<CRenderBackend *>(data)->mVBLCount++;
}

//----------------------------------------------------------------------------

CRenderBackend::CRenderBackend(IResourcePool & resourcePool,
   SRenderInitialization const & initFlags)
:  CBaseRenderBackend(resourcePool, initFlags)
,  mCameraMatrix(CMatrix4::kConstructUninitialized)
,  mViewMatrix(CMatrix4::kConstructUninitialized)
,  mProjectionTimesViewMatrix(CMatrix4::kConstructUninitialized)
,  mpDisplayTexture(NULL)
,  mpDisplayDepth(NULL)
,  mVBLCount(0)
,  mVBLHandle(-1)
,  mTargetFPS(kFPS_30)
,  mDepthWriteEnabled(true)
,  mBlendEnabled(false)
,  mBlendSrcRGB(kBF_One)
,  mBlendDstRGB(kBF_Zero)
,  mBlendSrcA(kBF_One)
,  mBlendDstA(kBF_Zero)
,  mDepthFunc(kDF_LEqual)
,  mCullMode(kCM_CCW)
{
   pvr_init_defaults();
   mVBLHandle = vblank_handler_add(VBLHandler, this);
}

CRenderBackend::~CRenderBackend()
{
   ShutDown();
}

void CRenderBackend::ShutDown()
{
   if (mVBLHandle >= 0) {
      vblank_handler_remove(mVBLHandle);
      mVBLHandle = -1;
   }
   pvr_shutdown();
}

void CRenderBackend::BeginScene()
{
   InternalBeginScene();
   pvr_wait_ready();
   pvr_scene_begin();
   pvr_list_begin(PVR_LIST_OP_POLY);
}

void CRenderBackend::EndScene()
{
   pvr_list_finish();
   // Submit an empty translucent list so the PVR doesn't stall waiting for it.
   pvr_list_begin(PVR_LIST_TR_POLY);
   pvr_list_finish();
   pvr_scene_finish();
   InternalPresent();
}

void CRenderBackend::Clear(int const /*clearFlags*/, CColor const /*color*/,
   real32 const /*z*/, int const /*stencil*/)
{
   // TODO Phase 2: KallistiOS clears are done via pvr background plane
}

void CRenderBackend::SetRenderTarget(SRenderTarget const & renderTarget)
{
   mCurrentRenderTarget = renderTarget;
   // TODO Phase 2: bind KallistiOS render target / texture
}

void CRenderBackend::SetViewport()
{
   // TODO Phase 2: glViewport equivalent
}

void CRenderBackend::SetVertexData(CShaderVertexDataBinding const & /*binding*/,
   CVertexData const & /*vertexData*/,
   uint64 const /*hash1*/, uint64 const /*hash2*/)
{
   // TODO Phase 2: bind vertex arrays for PowerVR submission
}

void CRenderBackend::ForceVertexDataRebind()
{
}

void CRenderBackend::RenderPrimitives(CMeshChunk::EPrimitive /*type*/,
   uint32 const /*indexBufferOffset*/, uint32 const /*indexCount*/)
{
   // TODO Phase 2: pvr_prim submission
}

void CRenderBackend::RenderPrimitivesInstanced(CMeshChunk::EPrimitive /*type*/,
   uint32 const /*indexBufferOffset*/, uint32 const /*indexCount*/,
   uint32 const /*indexWrap*/)
{
   // Dreamcast has no hardware instancing; implement via repeated draw calls.
   // TODO Phase 2
}

void CRenderBackend::RenderPrimitivesNoIndices(CMeshChunk::EPrimitive /*type*/,
   uint32 const /*vertexBufferOffset*/, uint32 const /*vertexCount*/)
{
   // TODO Phase 2
}

void CRenderBackend::BeginRenderPrimitivesCustom() {}
void CRenderBackend::EndRenderPrimitivesCustom()   {}

void CRenderBackend::RenderQuadVC(real32 const /*minX*/, real32 const /*maxX*/,
   real32 const /*minY*/, real32 const /*maxY*/, real32 const /*z*/,
   real32 const /*minU*/, real32 const /*maxU*/,
   real32 const /*minV*/, real32 const /*maxV*/)
{
   // TODO Phase 2
}

void CRenderBackend::RenderPrimitivesUserVertexData(CMeshChunk::EPrimitive /*type*/,
   CShaderVertexDataBinding /*binding*/,
   void const * /*pData*/, uint32 const /*numVertices*/)
{
   // TODO Phase 2
}

void CRenderBackend::SetIndexData(CIndexBuffer const * /*indexBuffer*/)
{
   // TODO Phase 2
}

void CRenderBackend::SetIndexData(CIndexBufferChunk const & /*chunk*/)
{
   // TODO Phase 2
}

void CRenderBackend::SetTexture(int const /*texUnit*/,
   CBaseTexture const * /*pTexture*/, bool const /*allowAniso*/)
{
   // TODO Phase 2: pvr_txr_load equivalent
}

void CRenderBackend::SetTextureAddressMode(int const /*texUnit*/,
   EWrapMode const /*wrapU*/, EWrapMode const /*wrapV*/)
{
   // TODO Phase 2
}

void CRenderBackend::SetTextureFilter(int const /*texUnit*/,
   EFilterMode /*minFilter*/, EFilterMode /*magFilter*/)
{
   // TODO Phase 2
}

void CRenderBackend::SetBlendMode(bool const enable,
   EBlendFunc const src, EBlendFunc const dst)
{
   mBlendEnabled = enable;
   mBlendSrcRGB = src; mBlendDstRGB = dst;
   mBlendSrcA = src;   mBlendDstA = dst;
}

void CRenderBackend::SetBlendMode(bool const enable,
   EBlendFunc const src, EBlendFunc const dst, uint32 const /*fixedColor*/)
{
   SetBlendMode(enable, src, dst);
}

void CRenderBackend::SetBlendMode(bool const enable,
   EBlendFunc const rgbSrc, EBlendFunc const rgbDst,
   EBlendFunc const aSrc, EBlendFunc const aDst)
{
   mBlendEnabled = enable;
   mBlendSrcRGB = rgbSrc; mBlendDstRGB = rgbDst;
   mBlendSrcA = aSrc;     mBlendDstA = aDst;
}

void CRenderBackend::SetBlendMode(bool const enable,
   EBlendFunc const rgbSrc, EBlendFunc const rgbDst,
   EBlendFunc const aSrc, EBlendFunc const aDst, uint32 const /*rgbaFixed*/)
{
   SetBlendMode(enable, rgbSrc, rgbDst, aSrc, aDst);
}

void CRenderBackend::SetBlendOp(EBlendOp const /*blendOp*/) {}
void CRenderBackend::SetBlendOp(EBlendOp const /*colorOp*/, EBlendOp const /*alphaOp*/) {}
void CRenderBackend::DisableBlend() { mBlendEnabled = false; }

void CRenderBackend::SetColorAlphaWriteEnabled(bool const /*color*/, bool const /*alpha*/) {}

void CRenderBackend::SetStencilEnable(bool /*enable*/) {}
void CRenderBackend::SetStencilOp(EStencilOp /*fail*/, EStencilOp /*depthFail*/,
   EStencilOp /*depthPass*/) {}
void CRenderBackend::SetStencilMask(uint32 /*mask*/) {}
void CRenderBackend::SetStencilFunc(EAlphaFunc /*func*/, uint32 /*ref*/, uint32 /*mask*/) {}

void CRenderBackend::SetRasterMask(bool /*enable*/) {}
void CRenderBackend::SetDepthBias(int /*slope*/, int /*bias*/) {}

void CRenderBackend::SetVertexRegisters(int /*start*/, int /*num*/,
   CVector4 const * /*pVec*/) {}
void CRenderBackend::SetFragmentRegisters(int /*start*/, int /*num*/,
   CVector4 const * /*pVec*/) {}

void CRenderBackend::UpdateVBLCount()
{
   // mVBLCount is incremented by VBLHandler registered in the constructor.
}

CRenderHWAllocator::SHandle const * CRenderBackend::AllocFixed(int const /*size*/,
   int const /*alignment*/, ERenderMemory const /*memoryType*/,
   ERenderMemory const /*fallbackMemory*/)
{
   return NULL; // TODO Phase 1: KallistiOS malloc / pvr_mem_malloc
}

void CRenderBackend::Free(CRenderHWAllocator::SHandle const * /*pHandle*/) {}
void CRenderBackend::FreeImmediate(CRenderHWAllocator::SHandle const * /*pHandle*/) {}

void CRenderBackend::GetMemoryStats(ERenderMemory const /*memory*/,
   SRenderHWAllocatorStats * pStats) const
{
   if (pStats) { /* TODO Phase 1 */ }
}

void CRenderBackend::SetUnsafeVertexUniformBuffer(int const /*bufferIndex*/,
   void const * /*ptr*/) {}
void CRenderBackend::SetStaticVertexUniformBuffer(int const /*bufferIndex*/,
   void const * /*ptr*/) {}
void * CRenderBackend::AllocOneFrameVertexUniformBuffer(int const /*bufferIndex*/,
   size_t const /*size*/) { return NULL; }
void * CRenderBackend::AllocOneFrameFragmentUniformBuffer(int const /*bufferIndex*/,
   size_t const /*size*/) { return NULL; }

void CRenderBackend::StartThreadSystem(TFnRenderThread /*fn*/) {}
void CRenderBackend::RunThreadFunction(int /*param*/) {}
void CRenderBackend::WaitThreadFunction() {}
bool CRenderBackend::TryWaitThreadFunction() { return true; }
void CRenderBackend::ShutdownThreadSystem() {}

void CRenderBackend::AcquireRenderThreadOwnership() {}
void CRenderBackend::ReleaseRenderThreadOwnership() {}

void CRenderBackend::FlushProjectionTimesViewMatrix()
{
   mProjectionTimesViewMatrix = mProjectionMatrix * mViewMatrix;
}

void CRenderBackend::BeginFrameResourceTick()
{
   InternalBeginFrameResourceTick();
}

//----------------------------------------------------------------------------
