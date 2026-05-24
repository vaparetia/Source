//------------------------------------------------------------------------------------------
// DCCRenderBackend.cpp
// Dreamcast render backend — KallistiOS + PowerVR implementation
//------------------------------------------------------------------------------------------

#include "Engine/StdAfx.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Primitive/CVertexData.h"
#include "Engine/Math/CHalfFloat.h"
#include "Renderer/Base/Primitive/EVertexDataType.h"

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
,  mpBoundVertexData(NULL)
,  mpBoundIndices(NULL)
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

void CRenderBackend::SetVertexData(CShaderVertexDataBinding const & binding,
   CVertexData const & vertexData,
   uint64 const /*hash1*/, uint64 const /*hash2*/)
{
   mpBoundVertexData = &vertexData;
   mBoundBinding     = binding;
}

void CRenderBackend::ForceVertexDataRebind()
{
   mpBoundVertexData = NULL;
}

void CRenderBackend::RenderPrimitives(CMeshChunk::EPrimitive const type,
   uint32 const indexBufferOffset, uint32 const indexCount)
{
   if (!mpBoundVertexData || !mpBoundIndices || indexCount == 0)
      return;
   if (type != CMeshChunk::kPrimitive_TriangleList)
      return;  // strips/fans deferred to Phase 5 Step 2
   if (!mpBoundVertexData->HasAttribute(kVDS_Position))
      return;

   // Polygon header: solid colour, opaque, no texture.
   pvr_poly_cxt_t cxt;
   pvr_poly_cxt_col(&cxt, PVR_LIST_OP_POLY);
   pvr_poly_hdr_t hdr;
   pvr_poly_compile(&hdr, &cxt);
   pvr_prim(&hdr, sizeof(hdr));

   // Position stream.
   const CVertexData::SVertexAttribute &posAttr = mpBoundVertexData->GetAttribute(kVDS_Position);
   const uint8 *posBase   = (const uint8*)mpBoundVertexData->GetBufferPtrByBufferIndex(posAttr.mBufferIndex);
   uint32       posStride = mpBoundVertexData->GetStrideByBufferIndex(posAttr.mBufferIndex);

   // Optional UV stream.
   bool                hasUV     = mpBoundVertexData->HasAttribute(kVDS_TexCoord0);
   const uint8        *uvBase    = NULL;
   uint32              uvStride  = 0, uvOffset = 0;
   EVertexDataType     uvType    = kVDT_Invalid;
   if (hasUV) {
      const CVertexData::SVertexAttribute &a = mpBoundVertexData->GetAttribute(kVDS_TexCoord0);
      uvBase   = (const uint8*)mpBoundVertexData->GetBufferPtrByBufferIndex(a.mBufferIndex);
      uvStride = mpBoundVertexData->GetStrideByBufferIndex(a.mBufferIndex);
      uvOffset = a.mOffset;
      uvType   = (EVertexDataType)a.mType;
   }

   const uint16 *idx = mpBoundIndices + indexBufferOffset;

   // De-index and transform one triangle at a time.
   // Each triangle is submitted as a 3-vertex strip with the last vertex flagged EOL.
   for (uint32 i = 0; i < indexCount; i += 3) {
      for (int v = 0; v < 3; ++v) {
         uint16 vi = idx[i + v];

         pvr_vertex_t vert;
         vert.flags = (v == 2) ? PVR_CMD_VERTEX_EOL : PVR_CMD_VERTEX;
         vert.argb  = 0xFFFFFFFF;  // white; per-vertex colour deferred to Phase 5 Step 2
         vert.oargb = 0x00000000;

         // Transform position (assumed Float3) to PVR screen space.
         const float *f = (const float*)(posBase + (uint32)vi * posStride + posAttr.mOffset);
         CVector4 clip = mProjectionTimesViewMatrix * CVector4(f[0], f[1], f[2], 1.0f);
         float invW = (clip.GetW() != 0.0f) ? (1.0f / clip.GetW()) : 0.0f;
         vert.x = (clip.GetX() * invW + 1.0f) * (0.5f * (float)skDisplayWidth);
         vert.y = (1.0f - clip.GetY() * invW) * (0.5f * (float)skDisplayHeight);
         vert.z = invW;  // PVR depth buffer stores 1/w

         // UV.
         if (hasUV) {
            const uint8 *u = uvBase + (uint32)vi * uvStride + uvOffset;
            if (uvType == kVDT_Half2 || uvType == kVDT_Half4) {
               const uint16 *h = (const uint16*)u;
               vert.u = CHalfFloat::ConvertToR32(h[0]);
               vert.v = CHalfFloat::ConvertToR32(h[1]);
            } else {
               const float *uf = (const float*)u;
               vert.u = uf[0];
               vert.v = uf[1];
            }
         } else {
            vert.u = vert.v = 0.0f;
         }

         pvr_prim(&vert, sizeof(vert));
      }
   }
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
   // CIndexBuffer uses hardware-allocated memory (pvr_mem_malloc); not yet implemented.
   mpBoundIndices = NULL;
}

void CRenderBackend::SetIndexData(CIndexBufferChunk const & chunk)
{
   mpBoundIndices = chunk.GetMemory();
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

CRenderBackend::EInitializeDisplayResult CBaseRenderBackend::InitializeDisplay(EStereoMode const /*stereoMode*/)
{
   // Video mode and PVR are initialized in the CRenderBackend constructor.
   // DC only supports monoscopic display; there is nothing to negotiate here.
   return kIDR_Succeded;
}

void CBaseRenderBackend::UninitializeDisplay()
{
}

//----------------------------------------------------------------------------
