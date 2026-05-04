//----------------------------------------------------------------------------
// BP_RenderObj.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "BP_RenderObjOptCmf.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Primitive/CMesh.h"
#include "Renderer/Base/Primitive/CMeshBuffers.h"

#include "BP_Renderer.h"
#include "BP_RendererDebug.h"
#include "BP_RenderGS.h"
#include "BP_RenderFX.h"

//----------------------------------------------------------------------------

#include "MGS_Common.h"

#include "BP_RenderBufferTypes.h"
#include "BP_RenderShared.h"

#include "bp_matrix.h"

//----------------------------------------------------------------------------

namespace ObjOptCmfShader
{
   int gPass           = 0;
   int const kFogCount = 2;
   int const kPassCount = 3;
   CCompiledShader* gpShader[kFogCount][kPassCount];

   enum ERegisters
   {
      kReg_Matrix0      = 16,
      kReg_Persp        = 20,
      kReg_Correction   = 24,
      kReg_EyeInv       = 28,
      kReg_TextureSize  = 32,
   };
}

void BP_InitObjOptCmfShader()
{
   CShaderFileId const shaderId("$/EngineSupport/Shaders/KMS_Opt_Cmf.fx");

   for( int fog = 0; fog < ObjOptCmfShader::kFogCount; ++fog )
   {
      for( int pass = 0; pass < ObjOptCmfShader::kPassCount; ++pass )
      {
         CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(shaderId, CStringExtras::Stringize_s("FOG=%d;PASS=%d", fog, pass));
         assert(pShader);
         
         ObjOptCmfShader::gpShader[fog][pass] = pShader;
      }
   }
}

//----------------------------------------------------------------------------

void BP_Obj_OptCmf_InitPacket(char* pData)
{
#if BP_VITA
   SBP_OBJ_OptCmf_InitPacket* pPacket = (SBP_OBJ_OptCmf_InitPacket*)pData;
   CBaseTexture *fbtex;

   gpRenderBackend->SetDepthWriteEnabled(false);

   fbtex = gpRenderBackend->GetLowresDrawTexture();
   gpRenderBackend->SetTexture(0, fbtex);

   real32 temp[16];
   BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->eye_pers, temp);
   gpRenderBackend->SetVertexRegisters(ObjOptCmfShader::kReg_Persp, 4, (CVector4 const*)temp);
   
   BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->eyeInv, temp);
   gpRenderBackend->SetVertexRegisters(ObjOptCmfShader::kReg_EyeInv, 4, (CVector4 const*)temp);

   temp[0] = fbtex->GetWidth()/2.0f; temp[1] = -fbtex->GetHeight()/2.0f; temp[2] = 0.0f; temp[3] = 0.0f;
   temp[4] = fbtex->GetWidth()/2.0f; temp[5] = fbtex->GetHeight()/2.0f; temp[6] = 0.0f; temp[7] = 0.0f;

   gpRenderBackend->SetVertexRegisters(kSReg_LightCol, 2, (CVector4 const*)temp); // kSReg_LightCol(2-4) are set in BP_Obj_OptCmf_Render

   temp[0] = 1.0f/fbtex->GetWidth()/2.0f; temp[1] = 1.0f/fbtex->GetHeight()/2.0f; temp[2] = 0.0f; temp[3] = 0.0f;
   gpRenderBackend->SetVertexRegisters(ObjOptCmfShader::kReg_TextureSize, 1, (CVector4 const*)temp);  

#endif
#ifndef BP_RENDER_SINGLE_RENDER_TARGET

   SBP_OBJ_OptCmf_InitPacket* pPacket = (SBP_OBJ_OptCmf_InitPacket*)pData;

   CBaseTexture* pTempBuffer = BP_GetRenderTarget(kRT_TempBuffer_NoMSAA);

#if BPE_TARGET == BPE_TARGET_X360
   gpRenderBackend->ResolveRenderTargetPredicated(pTempBuffer);
#else

   SRenderTarget const originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();
   
   CBaseTexture* pCurrentBuffer = BP_GetRenderTarget(kRT_CurrentFrameBuffer);
   BP_CopyTexture(pCurrentBuffer, pTempBuffer);

   gpRenderBackend->SetRenderTarget(originalRenderTarget);
   
#endif

   gpRenderBackend->SetDepthWriteEnabled(true);

   gpRenderBackend->SetTexture(0, pTempBuffer);

    real32 temp[16];
   BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->eye_pers, temp);
   gpRenderBackend->SetVertexRegisters(ObjOptCmfShader::kReg_Persp, 4, (CVector4 const*)temp);

   BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->eyeInv, temp);
   gpRenderBackend->SetVertexRegisters(ObjOptCmfShader::kReg_EyeInv, 4, (CVector4 const*)temp);

   temp[0] = pTempBuffer->GetWidth()/2.0f; temp[1] = -pTempBuffer->GetHeight()/2.0f; temp[2] = 0.0f; temp[3] = 0.0f;
   temp[4] = pTempBuffer->GetWidth()/2.0f; temp[5] = pTempBuffer->GetHeight()/2.0f; temp[6] = 0.0f; temp[7] = 0.0f;
   gpRenderBackend->SetVertexRegisters(kSReg_LightCol, 2, (CVector4 const*)temp); // kSReg_LightCol(2-4) are set in BP_Obj_OptCmf_Render

   temp[0] = 1.0f/pTempBuffer->GetWidth()/2.0f; temp[1] = 1.0f/pTempBuffer->GetHeight()/2.0f; temp[2] = 0.0f; temp[3] = 0.0f;
   gpRenderBackend->SetVertexRegisters(ObjOptCmfShader::kReg_TextureSize, 1, (CVector4 const*)temp);  

#endif
}

void BP_Obj_OptCmf_PassParam(char* pData)
{
   SBP_OBJ_OptCmf_PassParam* pPacket = (SBP_OBJ_OptCmf_PassParam*)pData;
#if BP_VITA
   switch (pPacket->alphaData)
   {
   default:
   case SCE_GS_SET_ALPHA(2,2,2,1,128): 
      ObjOptCmfShader::gPass = 0; 
      gpRenderBackend->SetDepthWriteEnabled(false);
      break;
   case SCE_GS_SET_ALPHA(0,0,0,0,0): 
      ObjOptCmfShader::gPass = 1; 
      gpRenderBackend->SetDepthWriteEnabled(true);
      break;
   case SCE_GS_SET_ALPHA(0,1,0,1,0): 
      ObjOptCmfShader::gPass = 2; 
      gpRenderBackend->SetDepthWriteEnabled(false);
      break;
   }
   if (pPacket->alphaData)
#endif
   BP_GS_SetAlpha(pPacket->alphaData);
}

namespace NOptCmfLocalParam
{
   CMesh *pLastMeshRendered = NULL;
   CDynamicVertexBuffer *pPreshadeBuffer = NULL;

   static void FlushObjLocalParamForMesh( CMesh *pMesh )
   {
      // If we just worked on this mesh, then punt
      if ( pMesh == pLastMeshRendered )
      {
         return;
      }

      CShaderVertexDataBinding binding;
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord0, kVDS_TexCoord0);
      binding.Set(kVDU_TexCoord1, kVDS_Color0);
      binding.Set(kVDU_TexCoord2, kVDS_Normal);

      CMeshBuffers & meshBuffers = const_cast<CMeshBuffers&>(pMesh->GetMeshBuffers());

      // Only override the vertex buffers if we preshade or animate textures
      if ( pPreshadeBuffer )
      {
         // Update vertex color array in mesh data.
         CVertexData vertexData = meshBuffers.VertexData();

         size_t const vertexBufferIndex = vertexData.AddBuffer( pPreshadeBuffer, sizeof( CVector4 ) );

         vertexData.SetAttribute(kVDS_Color0, 0, kVDT_Float4, vertexBufferIndex);
         gpRenderBackend->SetVertexData(binding, vertexData, (uint64)pMesh, (uint64)pPreshadeBuffer);
      }
      else
      {
         meshBuffers.SetVertexData(binding);
      }

      // Set Index Buffer
      gpRenderBackend->SetIndexData(meshBuffers.GetIndexBuffer());

      // Don't do it again
      pLastMeshRendered = pMesh;
   }
}

void BP_Obj_OptCmf_LocalParam(char* pData)
{
   SBP_OBJ_OptCmf_LocalParam* pPacket = (SBP_OBJ_OptCmf_LocalParam*)pData;

   extern CVector4 gBP_FogColor;
   gpRenderBackend->SetVertexRegisters(kSReg_FogColor, 1, &gBP_FogColor);
   
   gpRenderBackend->SetVertexRegisters(kSReg_FogParam, 1, (CVector4 const*)&pPacket->fogParam);

   NOptCmfLocalParam::pLastMeshRendered = NULL;
   NOptCmfLocalParam::pPreshadeBuffer = NULL;
}

void BP_Obj_OptCmf_Render(char* pData)
{
   static CVertexData preshadeVertexData;

   SBP_OBJ_OptCmf_Render* pPacket = (SBP_OBJ_OptCmf_Render*)pData;

   int const isFog = (pPacket->obj_flag & DG_FLAG_NOFOG) ? 0 : 1;

   real32 temp[16];
   BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->world, temp);
   gpRenderBackend->SetVertexRegisters(ObjOptCmfShader::kReg_Matrix0, 4, (CVector4 const*)temp);

   BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->correction, temp);
   gpRenderBackend->SetVertexRegisters(ObjOptCmfShader::kReg_Correction, 4, (CVector4 const*)temp);

   BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->lightDir, temp);
   gpRenderBackend->SetVertexRegisters(kSReg_LightDir, 4, (CVector4 const*)temp);

   gpRenderBackend->SetVertexRegisters(kSReg_LightCol+2, 2, (CVector4 const*)&pPacket->lightCol.m[2][0]); // kSReg_LightCol(0-1) are set in BP_Obj_OptCmf_InitPacket

   CMesh* pMesh = (CMesh*)pPacket->model;
   if( pMesh )
   {
      NOptCmfLocalParam::FlushObjLocalParamForMesh( pMesh );

      if( pPacket->flag & (DG_PACKFLAG_CULLAUTO|DG_PACKFLAG_CULLON) )
         gpRenderBackend->SetCullMode(CRenderBackend::kCM_CCW);
      else 
         gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

      BP_BeginShader(ObjOptCmfShader::gpShader[isFog][ObjOptCmfShader::gPass], NULL, 0);

      int chunkStart, chunkCount;
      pMesh->GetChunkRangeForUnit(pPacket->unit, &chunkStart, &chunkCount);

      CMeshChunk const * pFirstMeshChunk = &pMesh->GetMeshChunks()[chunkStart + pPacket->startPacket];

#if RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET()
      uint32 minVertex = gkUint32Max;
      uint32 maxVertex = gkUint32Min;
#endif

      int indexCount = 0;

      for( int i = 0; i < pPacket->packetCount; ++i )
      {
         int const packetIdx = pPacket->startPacket + i;
         CMeshChunk const * pMeshChunk = &pMesh->GetMeshChunks()[chunkStart + packetIdx];
#if RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET()
         minVertex = bpe::min_val(minVertex, pMeshChunk->mVertexBufferOffset);
         maxVertex = bpe::max_val(maxVertex, pMeshChunk->mVertexBufferOffset + pMeshChunk->mVertexCount);
#endif
         indexCount += pMeshChunk->mIndicesCount;
      }

      gpRenderBackend->RenderPrimitives(pFirstMeshChunk->mPrimitiveType, 
#if RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET()
         minVertex, maxVertex - minVertex, 
#endif
         pFirstMeshChunk->mIndexBufferOffset, indexCount);
   }
}
