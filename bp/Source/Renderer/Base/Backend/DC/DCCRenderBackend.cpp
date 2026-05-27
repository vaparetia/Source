//------------------------------------------------------------------------------------------
// DCCRenderBackend.cpp
// Dreamcast render backend — KallistiOS + PowerVR implementation
//------------------------------------------------------------------------------------------

#include "Engine/StdAfx.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Primitive/CVertexData.h"
#include "Engine/Math/CHalfFloat.h"
#include "Engine/Math/CMatrix34.h"
#include "Engine/Math/BPEMath.h"
#include "Renderer/Base/Primitive/EVertexDataType.h"

#include <dc/pvr.h>
#include <dc/vblank.h>
#include <stdlib.h>

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
,  mModelMatrix(CMatrix4::kConstructUninitialized)
,  mpBoundVertexData(NULL)
,  mpBoundIndices(NULL)
{
   mModelMatrix = CMatrix4::Identity();
   mCameraMatrix = CMatrix4::Identity();
   mViewMatrix = CMatrix4::Identity();
   FlushProjectionTimesViewMatrix();  // mProjectionMatrix already Identity from base ctor
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
   if (!mpBoundVertexData || !mpBoundIndices || indexCount < 3)
      return;
   if (!mpBoundVertexData->HasAttribute(kVDS_Position))
      return;

   // Determine triangle count and per-triangle index pattern.
   uint32 triCount;
   bool   isStrip = false;
   bool   isFan   = false;
   switch (type) {
   case CMeshChunk::kPrimitive_TriangleList:
      triCount = indexCount / 3;
      break;
   case CMeshChunk::kPrimitive_TriangleStrip:
      triCount = indexCount - 2;
      isStrip  = true;
      break;
   case CMeshChunk::kPrimitive_TriangleFan:
      triCount = indexCount - 2;
      isFan    = true;
      break;
   default:
      return;
   }

   // Polygon header: solid colour, opaque, no texture.
   // CULLING_NONE + DEPTHCMP_ALWAYS: pvr_poly_cxt_col defaults (CCW cull + GEQUAL)
   // fail on Flycast with a fresh depth buffer; override both to be safe.
   pvr_poly_cxt_t cxt;
   pvr_poly_cxt_col(&cxt, PVR_LIST_OP_POLY);
   cxt.gen.culling      = PVR_CULLING_NONE;
   cxt.depth.comparison = PVR_DEPTHCMP_ALWAYS;
   pvr_poly_hdr_t hdr;
   pvr_poly_compile(&hdr, &cxt);
   pvr_prim(&hdr, sizeof(hdr));

   // Position stream (Float3 assumed).
   const CVertexData::SVertexAttribute &posAttr = mpBoundVertexData->GetAttribute(kVDS_Position);
   const uint8 *posBase   = (const uint8*)mpBoundVertexData->GetBufferPtrByBufferIndex(posAttr.mBufferIndex);
   uint32       posStride = mpBoundVertexData->GetStrideByBufferIndex(posAttr.mBufferIndex);

   // Optional UV stream.
   bool            hasUV    = mpBoundVertexData->HasAttribute(kVDS_TexCoord0);
   const uint8    *uvBase   = NULL;
   uint32          uvStride = 0, uvOffset = 0;
   EVertexDataType uvType   = kVDT_Invalid;
   if (hasUV) {
      const CVertexData::SVertexAttribute &a = mpBoundVertexData->GetAttribute(kVDS_TexCoord0);
      uvBase   = (const uint8*)mpBoundVertexData->GetBufferPtrByBufferIndex(a.mBufferIndex);
      uvStride = mpBoundVertexData->GetStrideByBufferIndex(a.mBufferIndex);
      uvOffset = a.mOffset;
      uvType   = (EVertexDataType)a.mType;
   }

   // Optional colour stream (kVDS_Color0, engine stores RGBA bytes → PVR ARGB uint32).
   bool            hasColor    = mpBoundVertexData->HasAttribute(kVDS_Color0);
   const uint8    *colorBase   = NULL;
   uint32          colorStride = 0, colorOffset = 0;
   EVertexDataType colorType   = kVDT_Invalid;
   if (hasColor) {
      const CVertexData::SVertexAttribute &a = mpBoundVertexData->GetAttribute(kVDS_Color0);
      colorBase   = (const uint8*)mpBoundVertexData->GetBufferPtrByBufferIndex(a.mBufferIndex);
      colorStride = mpBoundVertexData->GetStrideByBufferIndex(a.mBufferIndex);
      colorOffset = a.mOffset;
      colorType   = (EVertexDataType)a.mType;
   }

   // Full MVP = ProjectionTimesView * per-object model matrix.
   CMatrix4 const mvp = mProjectionTimesViewMatrix * mModelMatrix;
   const uint16  *idx = mpBoundIndices + indexBufferOffset;

   // De-index and CPU-transform one triangle at a time.
   // Each triangle is submitted as a 3-vertex PVR strip (last vertex flagged EOL).
   for (uint32 t = 0; t < triCount; ++t) {
      // Resolve the three vertex indices for this triangle.
      uint16 vi[3];
      if (isFan) {
         vi[0] = idx[0];
         vi[1] = idx[t + 1];
         vi[2] = idx[t + 2];
      } else if (isStrip) {
         // Odd triangles swap first two indices to maintain consistent winding.
         if (t & 1) {
            vi[0] = idx[t + 1]; vi[1] = idx[t]; vi[2] = idx[t + 2];
         } else {
            vi[0] = idx[t]; vi[1] = idx[t + 1]; vi[2] = idx[t + 2];
         }
      } else {
         vi[0] = idx[t * 3]; vi[1] = idx[t * 3 + 1]; vi[2] = idx[t * 3 + 2];
      }

      for (int v = 0; v < 3; ++v) {
         const uint16 vidx = vi[v];

         pvr_vertex_t vert;
         vert.flags = (v == 2) ? PVR_CMD_VERTEX_EOL : PVR_CMD_VERTEX;
         vert.oargb = 0x00000000;

         // Colour: decode RGBA bytes or float4 → PVR ARGB; default to white.
         if (hasColor) {
            const uint8 *c = colorBase + (uint32)vidx * colorStride + colorOffset;
            if (colorType == kVDT_UByte4N || colorType == kVDT_UByte4) {
               vert.argb = ((uint32)c[3] << 24) | ((uint32)c[0] << 16)
                         | ((uint32)c[1] <<  8) |  (uint32)c[2];
            } else if (colorType == kVDT_Float4) {
               const float *cf = (const float*)c;
               vert.argb = ((uint32)(cf[3] * 255.f) << 24) | ((uint32)(cf[0] * 255.f) << 16)
                         | ((uint32)(cf[1] * 255.f) <<  8) |  (uint32)(cf[2] * 255.f);
            } else {
               vert.argb = 0xFFFFFFFF;
            }
         } else {
            vert.argb = 0xFFFFFFFF;
         }

         // Transform position (Float3) through full MVP to PVR screen space.
         const float *f = (const float*)(posBase + (uint32)vidx * posStride + posAttr.mOffset);
         CVector4 clip = mvp * CVector4(f[0], f[1], f[2], 1.0f);
         float invW = (clip.GetW() != 0.0f) ? (1.0f / clip.GetW()) : 0.0f;
         vert.x = (clip.GetX() * invW + 1.0f) * (0.5f * (float)skDisplayWidth);
         vert.y = (1.0f - clip.GetY() * invW) * (0.5f * (float)skDisplayHeight);
         vert.z = invW;  // PVR depth buffer stores 1/w

         // UV.
         if (hasUV) {
            const uint8 *u = uvBase + (uint32)vidx * uvStride + uvOffset;
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

CRenderHWAllocator::SHandle const * CRenderBackend::AllocFixed(int const size,
   int const /*alignment*/, ERenderMemory const /*memoryType*/,
   ERenderMemory const /*fallbackMemory*/)
{
   CRenderHWAllocator::SHandle* pHandle = new CRenderHWAllocator::SHandle();
   pHandle->mpAddress = (uint8*)malloc(size > 0 ? size : 1);
   return pHandle;
}

void CRenderBackend::Free(CRenderHWAllocator::SHandle const * pHandle)
{
   if( pHandle )
   {
      free(const_cast<CRenderHWAllocator::SHandle*>(pHandle)->mpAddress);
      delete const_cast<CRenderHWAllocator::SHandle*>(pHandle);
   }
}

void CRenderBackend::FreeImmediate(CRenderHWAllocator::SHandle const * pHandle)
{
   Free(pHandle);
}

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

void CBaseRenderBackend::SetProjectionMatrix(CMatrix4 const & matrix)
{
   mProjectionMatrix = matrix;
   static_cast<CRenderBackend*>(this)->FlushProjectionTimesViewMatrix();
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::SetCameraMatrix(CMatrix34 const & matrix)
{
   CRenderBackend* pThis = static_cast<CRenderBackend*>(this);
   pThis->mCameraMatrix = CMatrix4::FromMatrix34(matrix);
   pThis->mViewMatrix = CMatrix4::Scale(CVector3(1.0f, 1.0f, -1.0f)) * pThis->mCameraMatrix.Inverse();
   pThis->FlushProjectionTimesViewMatrix();
}

//----------------------------------------------------------------------------
