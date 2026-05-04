//----------------------------------------------------------------------------
// BP_RenderRadar.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "BP_RenderRadar.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Primitive/CVertexData.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"

#include "BP_Renderer.h"
#include "BP_RenderGS.h"
#include "BP_BaseRenderer.h"
#include "BP_RenderFX.h"

#include "BP_Matrix.h"

//----------------------------------------------------------------------------

#include "MGS_Common.h"

//----------------------------------------------------------------------------

#if BPE_TARGET==BPE_TARGET_VITA
#define BP_RADAR_RENDER_OFFSCREEN()       0
#else
#define BP_RADAR_RENDER_OFFSCREEN()       1
#endif

#if BP_RADAR_RENDER_OFFSCREEN()
static CBaseTexture* gpRadarColorBuffer = NULL;
static CBaseTexture* gpRadarColorBufferHalf = NULL;
static CBaseTexture* gpRadarDepthBuffer = NULL;
int const kRadarOffscreenScale = 2;
float const kRadarOffscreenLineScale = 2.0f;
#endif

float const kRadarLineWidth = 1.0f;

static int gRadarDrawWidth = -1;
static int gRadarDrawHeight = -1;
static CMatrix4 gRadarMatrix = CMatrix4::Identity();
static SRenderTarget gRadarRenderTarget;

int gBP_RadarRenderOffscreen = 1;

//----------------------------------------------------------------------------

namespace RadarShader
{
   int const kShaderCount = 2;

   CCompiledShader*  gpShader[kShaderCount];

   enum
   {
      kReg_Matrix = 0,
      kReg_Color0 = 4,
      kReg_Color1 = 5,
   };
}

void BP_InitRadarShader()
{
   CShaderFileId const shaderId("$/EngineSupport/Shaders/Radar.fx");

   for( int shader = 0; shader < RadarShader::kShaderCount; ++shader )
   {
      CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(shaderId, CStringExtras::Stringize_s("SHADER=%d", shader));
      assert(pShader);

      RadarShader::gpShader[shader] = pShader;
   }
}

//----------------------------------------------------------------------------

extern "C" void BP_RenderRadar(void* pData)
{
#if 1 //ifndef BP_RENDER_SINGLE_RENDER_TARGET

   BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Render Radar");

   SRenderTarget const lastRT = gpRenderBackend->GetCurrentRenderTarget();

#if !BP_VITA
   gpRenderBackend->SetAlphaTestEnable(false);
#endif

   gpRenderBackend->SetDepthCompareEnabled(true);
   gpRenderBackend->SetDepthWriteEnabled(true);
   gpRenderBackend->SetDepthFunc(CRenderBackend::kDF_GEqual);

   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   bool hasCommands = true;
   while(hasCommands)
   {
      uint32 const id = *(uint32*)pData;
      switch(id)
      {
      case kBP_RadarInit:
         {
            SBP_RadarPacket_Init* pPacket = (SBP_RadarPacket_Init*)pData;
            
            gRadarRenderTarget = lastRT;
            
            int const currentWidth = gRadarRenderTarget.mpColorBuffer[0] ? gRadarRenderTarget.mpColorBuffer[0]->GetWidth() : gpRenderBackend->GetBackBufferWidth();
            int const currentHeight = gRadarRenderTarget.mpColorBuffer[0] ? gRadarRenderTarget.mpColorBuffer[0]->GetHeight() : gpRenderBackend->GetBackBufferHeight();

            gRadarDrawWidth = pPacket->viewW * currentWidth / DRAW_WIDTH;
            gRadarDrawHeight = pPacket->viewH * currentHeight / DRAW_HEIGHT;

            gRadarRenderTarget.mRenderTargetViewPort.mViewportEnabled = 1;
            gRadarRenderTarget.mRenderTargetViewPort.mViewportX = pPacket->viewX * currentWidth / DRAW_WIDTH;
            gRadarRenderTarget.mRenderTargetViewPort.mViewportY = (pPacket->viewY - DISPLAY_CUTOFF/2) * currentHeight / DRAW_HEIGHT;
            gRadarRenderTarget.mRenderTargetViewPort.mViewportWidth = gRadarDrawWidth;
            gRadarRenderTarget.mRenderTargetViewPort.mViewportHeight = gRadarDrawHeight;

#if BP_RADAR_RENDER_OFFSCREEN()
            if( gpRadarColorBuffer == NULL && gpRadarColorBufferHalf == NULL && gpRadarDepthBuffer == NULL )
            {
               int const offscreenWidth = gRadarDrawWidth * kRadarOffscreenScale;
               int const offscreenHeight = gRadarDrawHeight * kRadarOffscreenScale;

               // Allocate color buffer
               {
                  SCreateTextureParams textureParams;
#if BPE_TARGET == BPE_TARGET_X360
                  textureParams.mBaseAddress = BP_GetRenderTarget(kRT_DefaultDepthBuffer_NoMSAA)->GetEDRAMEnd();
#endif
                  gpRadarColorBuffer = CBaseTexture::Create(offscreenWidth, offscreenHeight, 1, CBaseTexture::kFormat_A8R8G8B8, CBaseTexture::kUsage_RenderTarget, CBaseTexture::kAA_None, kRM_Video, &textureParams);
               }

               // Allocate half color buffer
               {
                  SCreateTextureParams textureParams;
#if BPE_TARGET == BPE_TARGET_X360
                  textureParams.mBaseAddress = BP_GetRenderTarget(kRT_DefaultDepthBuffer_NoMSAA)->GetEDRAMEnd();
#endif
                  gpRadarColorBufferHalf = CBaseTexture::Create(offscreenWidth / 2, offscreenHeight / 2, 1, CBaseTexture::kFormat_A8R8G8B8, CBaseTexture::kUsage_RenderTarget, CBaseTexture::kAA_None, kRM_Video, &textureParams);
               }

               // Allocate depth buffer
               {
                  SCreateTextureParams textureParams;
#if BPE_TARGET == BPE_TARGET_X360
                  textureParams.mBaseAddress = gpRadarColorBuffer->GetEDRAMEnd();
                  textureParams.mHierarchicalZBase = BP_GetRenderTarget(kRT_DefaultDepthBuffer)->GetEDRAMEndHierarchicalZ();
#endif
                  gpRadarDepthBuffer = CBaseTexture::Create(offscreenWidth, offscreenHeight, 1, CBaseTexture::kFormat_D24X8, CBaseTexture::kUsage_DepthBuffer, CBaseTexture::kAA_None, kRM_Video, &textureParams);
               }
            }
#endif

#if BP_RADAR_RENDER_OFFSCREEN()
            if( gBP_RadarRenderOffscreen )
            {
               // Render lines to offscreen buffer
               SRenderTarget const rt(gpRadarColorBuffer, NULL, NULL, NULL, gpRadarDepthBuffer);
               gpRenderBackend->SetRenderTarget(rt);
               gpRenderBackend->Clear(CRenderBackend::kFlag_Color|CRenderBackend::kFlag_Depth, CColor(0, 0, 0, 0), 0);

               gpRenderBackend->SetLineWidth(kRadarLineWidth*kRadarOffscreenLineScale);
            }
            else
#endif
            {
#if !BP_VITA
               gpRenderBackend->SetLineWidth(kRadarLineWidth);
#endif
               gpRenderBackend->Clear(CRenderBackend::kFlag_Depth, CColor(0, 0, 0, 0), 0);
               gpRenderBackend->SetRenderTarget(gRadarRenderTarget);
            }

            // Setup projection matrix
            gRadarMatrix = *(CMatrix4*)&pPacket->matrix;

            // Adjust matrix to snap coordinates to whole pixels in render target
            {
               CVector3 const zeroPos = gRadarMatrix * CVector3::Zero();
               
               real32 const xScale = gRadarDrawWidth / 2.0f;
               real32 const yScale = gRadarDrawHeight / 2.0f;

               CVector3 const snappedZeroPos( int(zeroPos[0] * xScale) / xScale, int(zeroPos[1] * yScale) / yScale, zeroPos[2]);

               CVector3 const snapDelta = snappedZeroPos - zeroPos;
               
               gRadarMatrix = CMatrix4::Translation(snapDelta) * gRadarMatrix;
            }

            float temp[16];
            BP_Matrix44_to_VS_Matrix44((real32 const*)&gRadarMatrix, temp);
            gpRenderBackend->SetVertexRegisters(RadarShader::kReg_Matrix, 4, (CVector4*)temp);

            pData = pPacket + 1;
         }
         break;

      case kBP_RadarBeginBackPrimitive:
         {
            // Always succeed, no depth test/write.
            gpRenderBackend->SetDepthCompareEnabled(false);
            gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
            gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_InvSrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);

            pData = ((SBP_RadarPacket_BeginSegments*)pData) + 1;
         }
         break;

      case kBP_RadarBeginSegments:
         {
            // Restore depth compare/write
            gpRenderBackend->SetDepthCompareEnabled(true);
            pData = ((SBP_RadarPacket_BeginSegments*)pData) + 1;
         }
         break;

      case kBP_RadarBeginPrimitives:
         {
            //DG_SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
#if !BP_VITA
            gpRenderBackend->SetShadeMode(1);
#endif

            //DG_Clear( 0, NULL, D3DCLEAR_ZBUFFER, 0, 0.0f, 0 );
            gpRenderBackend->Clear(CRenderBackend::kFlag_Depth, CColor::Black(), 0);
         
            //DG_SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
            gpRenderBackend->SetDepthWriteEnabled(false);

            gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
            gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One);

            pData = ((SBP_RadarPacket_BeginSegments*)pData) + 1;
         }
         break;

      case kBP_RadarColor:
         {
            SBP_RadarPacket_Color* pPacket = (SBP_RadarPacket_Color*)pData;
            gpRenderBackend->SetVertexRegisters(RadarShader::kReg_Color0, 1, (CVector4*)&pPacket->color1);
            gpRenderBackend->SetVertexRegisters(RadarShader::kReg_Color1, 1, (CVector4*)&pPacket->color2);

            pData = pPacket + 1;
         }
         break;

      case kBP_RadarGeomPos:
         {
            SBP_RadarPacket_GeomPos* pPacket = (SBP_RadarPacket_GeomPos*)pData;
            SBP_RadarVertexPos* pVerts = pPacket->verts;

            CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(SBP_RadarVertexPos) * pPacket->vertexCount);
            if( pVertexBuffer )
            {
               // Fill buffer
               {
                  void* pDestVerts = pVertexBuffer->Lock();

                  memcpy(pDestVerts, pVerts, sizeof(SBP_RadarVertexPos) * pPacket->vertexCount);
                  pVertexBuffer->Unlock();
               }

               CVertexData vertexData;
               size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( SBP_RadarVertexPos ) );
               vertexData.SetAttribute(kVDS_Position, offsetof(SBP_RadarVertexPos, x), kVDT_Float4, bufferIndex);

               CShaderVertexDataBinding binding;
               binding.Set(kVDU_Position, kVDS_Position);

               gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
               gpRenderBackend->ForceVertexDataRebind();

               // Switch to shader
               BP_BeginShader(RadarShader::gpShader[pPacket->shader], NULL, 0);

               switch(pPacket->prim)
               {
               case kBP_RadarPrim_LineList:
                  gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_LineList, 0, pPacket->vertexCount);
                  break;
               case kBP_RadarPrim_LineStrip:
                  gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_LineStrip, 0, pPacket->vertexCount);
                  break;
               case kBP_RadarPrim_TriStrip:
                  gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleStrip, 0, pPacket->vertexCount);
                  break;
               case kBP_RadarPrim_TriFan:
                  gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleFan, 0, pPacket->vertexCount);
                  break;
               }
            }

            pData = pVerts + pPacket->vertexCount;
         }
         break;

      case kBP_RadarGeomPosCol:
         {
            SBP_RadarPacket_GeomPosCol* pPacket = (SBP_RadarPacket_GeomPosCol*)pData;
            SBP_RadarVertexPosCol* pVerts = pPacket->verts;
            
            CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(SBP_RadarVertexPosCol) * pPacket->vertexCount);
            if( pVertexBuffer )
            {
               // Fill buffer
               {
                  void* pDestVerts = pVertexBuffer->Lock();
                  memcpy(pDestVerts, pVerts, sizeof(SBP_RadarVertexPosCol) * pPacket->vertexCount);
                  pVertexBuffer->Unlock();
               }

               CVertexData vertexData;
               size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( SBP_RadarVertexPosCol ) );
               vertexData.SetAttribute(kVDS_Position, offsetof(SBP_RadarVertexPosCol, x), kVDT_Float3, bufferIndex);
               vertexData.SetAttribute(kVDS_Color0, offsetof(SBP_RadarVertexPosCol, col), kVDT_UByte4N, bufferIndex);

               CShaderVertexDataBinding binding;
               binding.Set(kVDU_Position, kVDS_Position);
               binding.Set(kVDU_TexCoord0, kVDS_Color0);

               gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
               gpRenderBackend->ForceVertexDataRebind();

               // Switch to shader
               BP_BeginShader(RadarShader::gpShader[pPacket->shader], NULL, 0);

               switch(pPacket->prim)
               {
               case kBP_RadarPrim_LineList:
                  gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_LineList, 0, pPacket->vertexCount);
                  break;
               case kBP_RadarPrim_LineStrip:
                  gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_LineStrip, 0, pPacket->vertexCount);
                  break;
               case kBP_RadarPrim_TriStrip:
                  gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleStrip, 0, pPacket->vertexCount);
                  break;
               case kBP_RadarPrim_TriFan:
                  gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleFan, 0, pPacket->vertexCount);
                  break;
               }
            }

            pData = pVerts + pPacket->vertexCount;
         }
         break;

      case kBP_RadarEnd:
         {
#if BP_RADAR_RENDER_OFFSCREEN()
            if( gBP_RadarRenderOffscreen )
            {
               gpRenderBackend->SetDepthCompareEnabled(false);

               gpRenderBackend->ResolveRenderTarget(gpRadarColorBuffer);

               // Downsample radar texture
               {
                  gpRenderBackend->SetRenderTarget(SRenderTarget(gpRadarColorBufferHalf, NULL, NULL, NULL, NULL));

                  gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
                  gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);

                  BP_DrawFullscreenTexture(gpRadarColorBuffer, 1 /*isFilter*/, 1/*isRGBA*/);
                  gpRenderBackend->ResolveRenderTarget(gpRadarColorBufferHalf);
               }

               // Apply radar texture to screen
               {
                  gpRenderBackend->SetRenderTarget(gRadarRenderTarget);

                  gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
                  gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_One, CRenderBackend::kBF_InvSrcAlpha);

                  BP_DrawFullscreenTexture(gpRadarColorBufferHalf, 1 /*isFilter*/, 1 /*isRGBA*/);
               }
            }
#endif
            hasCommands = false;
         }
      }
   }

   // Restore render states to default state for this command
   gpRenderBackend->SetDepthCompareEnabled(false/*true*/);
   gpRenderBackend->SetDepthFunc(CRenderBackend::kDF_Always);
   gpRenderBackend->SetDepthWriteEnabled(false);
#if !BP_VITA
   gpRenderBackend->SetLineWidth(1.0f);
#endif
   gpRenderBackend->SetRenderTarget(lastRT);

#endif
}
