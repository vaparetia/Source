//----------------------------------------------------------------------------
// BP_RenderFixModel.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Primitive/CMesh.h"
#include "Renderer/Base/Primitive/CMeshBuffers.h"

#include "BP_Renderer.h"
#include "BP_RendererDebug.h"
#include "BP_RenderGS.h"
#include "BP_VertexAnim.h"

//----------------------------------------------------------------------------

#include "MGS_Common.h"

#include "BP_RenderFixModel.h"
#include "BP_RenderBufferTypes.h"
#include "BP_RenderShared.h"
#include "BP_Preshade.h"

//----------------------------------------------------------------------------

int gFixModel_Enable = 1;
//
int gFixModel_EnableSingleTex = 1;
int gFixModel_EnableMultiTex = 1;
int gFixModel_ForceSingleTex = 0;
int gFixModel_EnableEnvmap = 1;
//
int gFixModel_EnablePreshade = 1;
int gFixModel_EnableVertexAnim = 1;
//
int gFixModel_ShowStats = 0;
int gFixModel_NumRendered = 0;

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

namespace MDBShader
{
   enum EUVType
   {
      kUV_Set0,
      kUV_Set1,
      kUV_Set2,
      kUV_Envmap,

      kUV_Count
   };

   int const kTextureCount = 2;
   int const kPreshadeCount = 2;
   int const kFogCount = 2;
   int const kOptCmfCount = 2;
   int const kTexProjMode = 3;
   int const kForceRGBWhiteCount = 2;

   CCompiledShader*  gpShader[kTextureCount][kPreshadeCount][kFogCount][kOptCmfCount][kTexProjMode][kUV_Count][kForceRGBWhiteCount];
   unsigned int gModelLocalParamShaderMode;

   DG_TEX_BP* gpLastValidTex = NULL;

   enum ERegisters
   {
      kReg_Persp     = 16,
      kReg_Opt0      = 20,
      kReg_Matrix0   = 24,
   };
}

void BP_InitFixModelShader()
{
   // initialize the shader permutations required for fix model rendering.
   CShaderFileId const shaderId("$/EngineSupport/Shaders/MDB.fx");
   for( int texture = 0; texture < MDBShader::kTextureCount; ++texture )
   {
      for( int preshade = 0; preshade < MDBShader::kPreshadeCount; ++preshade )
      {
         for( int fog = 0; fog < MDBShader::kFogCount; ++fog )
         {
            for( int optCmf = 0; optCmf < MDBShader::kOptCmfCount; ++optCmf )
            {
               for( int texProjMode = 0; texProjMode < MDBShader::kTexProjMode; ++texProjMode )
               {
                  for( int uv = 0; uv < MDBShader::kUV_Count; ++uv )
                  {
                     for( int forceRGBWhite = 0; forceRGBWhite < MDBShader::kForceRGBWhiteCount; ++forceRGBWhite )
                     {
                        CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(shaderId, CStringExtras::Stringize_s("TEXTURE=%d;PRESHADE=%d;FOG=%d;OPT_CMF=%d;PROJ_MODE=%d;UV=%d;FORCE_RGB_WHITE=%d", texture, preshade, fog, optCmf, texProjMode, uv, forceRGBWhite));
                        assert(pShader);

                        MDBShader::gpShader[texture][preshade][fog][optCmf][texProjMode][uv][forceRGBWhite] = pShader;
                     }
                  }
               }
            }
         }
      }
   }
}

//----------------------------------------------------------------------------

void BP_FixModel_FrameBegin()
{
   gFixModel_NumRendered = 0;
}

void BP_FixModel_FrameEnd()
{
   if( gFixModel_ShowStats )
   {
      BP_DebugText_Print("Fix Model Count: %d", gFixModel_NumRendered);
   }
}

//----------------------------------------------------------------------------

void BP_FixModel_InitPacket(char* pData)
{
   SBP_MDX_InitPacket* pPacket = (SBP_MDX_InitPacket*)pData;

   gpRenderBackend->SetDepthCompareEnabled(true);
   gpRenderBackend->SetDepthFunc(CRenderBackend::kDF_GEqual);
   gpRenderBackend->SetDepthWriteEnabled(true);
   
   gpRenderBackend->SetVertexRegisters(kSReg_FogParam, 1, (CVector4 const*)&pPacket->fogParam);

   extern CVector4 gBP_FogColor;
   gpRenderBackend->SetVertexRegisters(kSReg_FogColor, 1, &gBP_FogColor);
}

//----------------------------------------------------------------------------

void BP_FixModel_LocalParam(char* pData)
{
   SBP_MDB_LocalParam* pPacket = (SBP_MDB_LocalParam*)pData;

   MDBShader::gModelLocalParamShaderMode = pPacket->shaderMode;

   real32 temp[16];
   
   BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->world, temp);
   gpRenderBackend->SetVertexRegisters(MDBShader::kReg_Matrix0, 4, (CVector4 const*)temp);
   
   BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->pers_mat, temp);
   gpRenderBackend->SetVertexRegisters(MDBShader::kReg_Persp, 4, (CVector4 const*)temp);

   unsigned int projMode = BP_MDB_SM_GET_PROJ_MODE(MDBShader::gModelLocalParamShaderMode);
   unsigned int isOptCmf = BP_MDB_SM_GET_OPT_CMF(MDBShader::gModelLocalParamShaderMode);
   if( projMode )
   {
      BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->opt0_mat, temp);
      gpRenderBackend->SetVertexRegisters(MDBShader::kReg_Opt0, 4, (CVector4 const*)temp);
   }
   else
   {
      BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->opt0_mat, temp);
      gpRenderBackend->SetVertexRegisters(MDBShader::kReg_Opt0, 4, (CVector4 const*)temp);
   }

   if( !isOptCmf && !projMode )
   {
      BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->lightDir, temp);
      gpRenderBackend->SetVertexRegisters(kSReg_LightDir, 4, (CVector4 const*)temp);

      BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->lightCol, temp);
      gpRenderBackend->SetVertexRegisters(kSReg_LightCol, 4, (CVector4 const*)temp);
   }
   else
   {
      gpRenderBackend->SetVertexRegisters(kSReg_LightDir, 4, (CVector4 const*)&pPacket->lightDir);
      gpRenderBackend->SetVertexRegisters(kSReg_LightCol, 4, (CVector4 const*)&pPacket->lightCol);
   }

   CMesh* pMesh = (CMesh*)pPacket->model;
   if( pMesh )
   {
      CShaderVertexDataBinding binding;
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord0, kVDS_TexCoord0);
      binding.Set(kVDU_TexCoord1, kVDS_Color0);
      binding.Set(kVDU_TexCoord2, kVDS_Normal);
      binding.Set(kVDU_TexCoord3, kVDS_TexCoord1);
      binding.Set(kVDU_TexCoord4, kVDS_TexCoord2);

      CMeshBuffers& meshBuffers = const_cast<CMeshBuffers&>(pMesh->GetMeshBuffers());
      CVertexData& vertexData = meshBuffers.VertexData();

      // Set Vertex Buffers
      {
         // get the model and preshade information.
         DG_MODEL* model = ( DG_MODEL* )pPacket->dgModel;
         CDynamicVertexBuffer* pPreshadeColors = BP_Preshade_GetVertexBuffer( model );
         bool const hasVAnimData = gFixModel_EnableVertexAnim && BP_HasVAnimData( model );
         bool const hasPreshade = gFixModel_EnablePreshade && pPreshadeColors != 0;

         if ( hasVAnimData || hasPreshade )
         {
            // make a copy of the vertex data object.
            CVertexData vertexData = meshBuffers.VertexData();
            
            uint64 hash2 = 0;

            // handle vertex animation.
            if ( hasVAnimData )
            {
               // pack the vertex animation data into a vertex buffer.
               CDynamicVertexBufferPoolChunk_RT* vertexChunk = BP_PackVAnimData( pMesh, model );

               vertexData.SetAttribute(kVDS_Position, 0 * 3 * sizeof( float ), 8 * sizeof( float ), kVDT_Float3, vertexChunk);
               vertexData.SetAttribute(kVDS_Normal, 1 * 3 * sizeof( float ), 8 * sizeof( float ), kVDT_Float3, vertexChunk);
               vertexData.SetAttribute(kVDS_TexCoord0, 2 * 3 * sizeof( float ), 8 * sizeof( float ), kVDT_Float2, vertexChunk);


               // store the vertex animation hash.
               hash2 = (uint64)vertexChunk;
            }

            // handle preshade buffers.
            if( hasPreshade )
            {
               // Update vertex color array in mesh data.
               vertexData.SetAttribute(kVDS_Color0, 0, sizeof(CVector4), kVDT_Float4, pPreshadeColors);
               hash2 ^= (uint64)pPreshadeColors;
            }

            vertexData.ClearCachedVertexDeclaration();
            gpRenderBackend->SetVertexData(binding, vertexData, (uint64)pMesh, hash2);
         }
         else
         {
            meshBuffers.SetVertexData(binding);
         }
      }

      // Set Index Buffer
      gpRenderBackend->SetIndexData(meshBuffers.GetIndexBuffer());
   }
}

//----------------------------------------------------------------------------

void BP_FixModel_WrapPacket(char* pData)
{
   SBP_MDB_WrapPacket* pPacket = (SBP_MDB_WrapPacket*)pData;

   //BPE_ADD_SCOPED_GPU_PROFILE_MARKER(CStringExtras::Stringize_s("WrapPacket: %8.8x", pPacket->testValue));

   real32 temp[16];
   BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->pers_mat, temp);
   gpRenderBackend->SetVertexRegisters(MDBShader::kReg_Persp, 4, (CVector4 const*)temp);

   BP_GS_SetTest(pPacket->testValue);
}

//----------------------------------------------------------------------------

class CMDBRender
{
public:
   SBP_MDB_Render* const   mpPacket;
   CMesh* const            mpMesh;
   CMeshChunk const *      pFirstMeshChunk;

   int const isIR;
   int const isTexture;
   int const isOptocamoflage;
   int const projectionMode;
   int const isFog;
   int const isPreshade;

   uint32 minVertex;
   uint32 maxVertex;
   int indexCount;

   BPE_FORCEINLINE CMDBRender(SBP_MDB_Render* pPacket, unsigned int shaderMode, CMesh* pMesh)
      :  mpPacket(pPacket)
      ,  mpMesh(pMesh)
      ,  isIR(BP_MDB_SM_GET_IR(shaderMode))
      ,  isTexture(!BP_MDB_SM_GET_IR(shaderMode) && BP_MDB_SM_GET_TEXTURE(shaderMode))
      ,  isOptocamoflage(BP_MDB_SM_GET_OPT_CMF(shaderMode))
      ,  projectionMode(BP_MDB_SM_GET_PROJ_MODE(shaderMode))
      ,  isFog((pPacket->unit_flag & MDB_UNIT_NOFOG) ? 0 : 1)
      ,  isPreshade((pPacket->flag & MDB_PACKET_VCOLOR) || (pPacket->preshadeBuffer != 0))
   {
   }

   void Setup()
   {
      minVertex = gkUint32Max;
      maxVertex = gkUint32Min;

      indexCount = 0;

      int chunkStart, chunkCount;

      mpMesh->GetChunkRangeForUnit(mpPacket->unit, &chunkStart, &chunkCount);

      pFirstMeshChunk = &mpMesh->GetMeshChunks()[chunkStart + mpPacket->startPacket];

      for( int i = 0; i < mpPacket->packetCount; ++i )
      {
         int const packetIdx = mpPacket->startPacket + i;
         CMeshChunk const * pMeshChunk = &mpMesh->GetMeshChunks()[chunkStart + packetIdx];
         minVertex = bpe::min_val(minVertex, pMeshChunk->mVertexBufferOffset);
         maxVertex = bpe::max_val(maxVertex, pMeshChunk->mVertexBufferOffset + pMeshChunk->mVertexCount);
         indexCount += pMeshChunk->mIndicesCount;
      }

      if( mpPacket->flag & (MDB_PACKET_CULLAUTO|MDB_PACKET_CULLON) )
         gpRenderBackend->SetCullMode(CRenderBackend::kCM_CCW);
      else 
         gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);
   }

   BPE_FORCEINLINE void SetupTextureParameters(DG_TEX_BP* pTex)
   {
      MDBShader::gpLastValidTex = pTex;
      BP_SetTexture(pTex, 0);
   }

   BPE_FORCEINLINE void SetupAlpha(DG_TEX_BP* pTex)
   {
      if( !isIR )
         BP_GS_SetAlpha(pTex->tex_trans.alpha.data);
      else
         BP_GS_SetAlpha(0);
   }

   BPE_FORCEINLINE void SetupAlpha_MultiBlendMode(DG_TEX_BP* pTex, int iBlendPass)
   {
      BP_GS_SetAlpha(pTex->tex_trans.alpha.data, iBlendPass);
   }

   void RenderSingle()
   {
#if BP_ENABLE_DEBUG_MESH_GPU_MARKER
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER(CStringExtras::Stringize_s("Single: %s Unit: %d Pack: %d->%d", mpMesh->mDebugName.c_str(), mpPacket->unit, mpPacket->startPacket, mpPacket->startPacket + mpPacket->packetCount - 1));
#endif

      DG_TEX_BP* pTex = mpPacket->tex[0];
      if( pTex )
      {
         SetupTextureParameters(pTex);
         SetupAlpha(pTex);
      }
      
      if( !projectionMode && !isOptocamoflage )
      {
         if( gGS_BlendModeDrawCount == 0 )
         {
            BP_BeginShader(MDBShader::gpShader[isTexture][isPreshade ? 1 : 0][isFog][isOptocamoflage][projectionMode][MDBShader::kUV_Set0][gGS_ForceRGBWhite], NULL, 0);
            for( int alphaTestPass = 0; alphaTestPass < gGS_AlphaFailPassCount; ++alphaTestPass )
            {
               BP_GS_SetupAlphaFailPass(gGS_AlphaFailPassStart + alphaTestPass);
               gpRenderBackend->RenderPrimitives(pFirstMeshChunk->mPrimitiveType, minVertex, maxVertex - minVertex, pFirstMeshChunk->mIndexBufferOffset, indexCount);
               ++gFixModel_NumRendered;
            }
         }
         else
         {
            for( int alphaTestPass = 0; alphaTestPass < gGS_AlphaFailPassCount; ++alphaTestPass )
            {
               SetupAlpha(MDBShader::gpLastValidTex);
               BP_BeginShader(MDBShader::gpShader[isTexture][isPreshade ? 1 : 0][isFog][isOptocamoflage][projectionMode][MDBShader::kUV_Set0][gGS_ForceRGBWhite], NULL, 0);

               BP_GS_SetupAlphaFailPass(gGS_AlphaFailPassStart + alphaTestPass);
               gpRenderBackend->RenderPrimitives(pFirstMeshChunk->mPrimitiveType, minVertex, maxVertex - minVertex, pFirstMeshChunk->mIndexBufferOffset, indexCount);
               ++gFixModel_NumRendered;

               for( int iBlendPass=1;iBlendPass<gGS_BlendModeDrawCount;++iBlendPass )
               {
                  SetupAlpha_MultiBlendMode(MDBShader::gpLastValidTex, iBlendPass);
                  BP_BeginShader(MDBShader::gpShader[isTexture][isPreshade ? 1 : 0][isFog][isOptocamoflage][projectionMode][MDBShader::kUV_Set0][gGS_ForceRGBWhite], NULL, 0);

                  gpRenderBackend->RenderPrimitives(pFirstMeshChunk->mPrimitiveType, minVertex, maxVertex - minVertex, pFirstMeshChunk->mIndexBufferOffset, indexCount);
                  ++gFixModel_NumRendered;
               }
            }
         }
      }
      else
      {
         BP_BeginShader(MDBShader::gpShader[isTexture][isPreshade ? 1 : 0][isFog][isOptocamoflage][projectionMode][MDBShader::kUV_Set0][gGS_ForceRGBWhite], NULL, 0);

         gpRenderBackend->RenderPrimitives(pFirstMeshChunk->mPrimitiveType, minVertex, maxVertex - minVertex, pFirstMeshChunk->mIndexBufferOffset, indexCount);
         ++gFixModel_NumRendered;
      }
   }

   void RenderMulti()
   {
#if BP_ENABLE_DEBUG_MESH_GPU_MARKER
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER(CStringExtras::Stringize_s("Multi: %s Unit: %d Pack: %d->%d", mpMesh->mDebugName.c_str(), mpPacket->unit, mpPacket->startPacket, mpPacket->startPacket + mpPacket->packetCount - 1));
#endif

      int const isTex0 = mpPacket->flag & DG_PACKFLAG_TEX0;
      int const isTex1 = mpPacket->flag & DG_PACKFLAG_TEX1;
      int const isTex2 = mpPacket->flag & DG_PACKFLAG_TEX2;
      int const isUV0 = mpPacket->flag & DG_PACKFLAG_UV0;
      int const isUV1 = mpPacket->flag & DG_PACKFLAG_UV1;
      int const isUV2 = mpPacket->flag & DG_PACKFLAG_UV2;
      int const isEmap = mpPacket->flag & DG_PACKFLAG_EMAP;
      int const isSmap = mpPacket->flag & DG_PACKFLAG_SMAP;

      // Render first pass
      if( isTex0 )
      {
#if BP_ENABLE_DEBUG_MESH_GPU_MARKER
         BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass 0");
#endif
         DG_TEX_BP* pTex = mpPacket->tex[0];
         SetupTextureParameters(pTex);
         SetupAlpha(pTex);
         BP_BeginShader(MDBShader::gpShader[isTexture][isPreshade ? 1 : 0][isFog][isOptocamoflage][projectionMode][MDBShader::kUV_Set0][gGS_ForceRGBWhite], NULL, 0);

         for( int alphaTestPass = 0; alphaTestPass < gGS_AlphaFailPassCount; ++alphaTestPass )
         {
            BP_GS_SetupAlphaFailPass(gGS_AlphaFailPassStart + alphaTestPass);
            gpRenderBackend->RenderPrimitives(pFirstMeshChunk->mPrimitiveType, minVertex, maxVertex - minVertex, pFirstMeshChunk->mIndexBufferOffset, indexCount);
            ++gFixModel_NumRendered;
         }
      }

      // Render second pass
      if( isTex1 )
      {
#if BP_ENABLE_DEBUG_MESH_GPU_MARKER
         BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass 1");
#endif
         DG_TEX_BP* pTex = mpPacket->tex[1];
         SetupTextureParameters(pTex);
         SetupAlpha(pTex);
         BP_BeginShader(MDBShader::gpShader[isTexture][isPreshade ? 1 : 0][isFog][isOptocamoflage][projectionMode][MDBShader::kUV_Set1][gGS_ForceRGBWhite], NULL, 0);

         for( int alphaTestPass = 0; alphaTestPass < gGS_AlphaFailPassCount; ++alphaTestPass )
         {
            BP_GS_SetupAlphaFailPass(gGS_AlphaFailPassStart + alphaTestPass);

            gpRenderBackend->RenderPrimitives(pFirstMeshChunk->mPrimitiveType, minVertex, maxVertex - minVertex, pFirstMeshChunk->mIndexBufferOffset, indexCount);
            ++gFixModel_NumRendered;
         }
      }

      // Render third pass
      if( isTex2 )
      {
         DG_TEX_BP* pTex = mpPacket->tex[2];
         SetupTextureParameters(pTex);
         SetupAlpha(pTex);

         if( isUV2 )
         {
#if BP_ENABLE_DEBUG_MESH_GPU_MARKER
            BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass 2");
#endif
            BP_BeginShader(MDBShader::gpShader[isTexture][isPreshade ? 1 : 0][isFog][isOptocamoflage][projectionMode][MDBShader::kUV_Set2][gGS_ForceRGBWhite], NULL, 0);
            for( int alphaTestPass = 0; alphaTestPass < gGS_AlphaFailPassCount; ++alphaTestPass )
            {
               BP_GS_SetupAlphaFailPass(gGS_AlphaFailPassStart + alphaTestPass);
               gpRenderBackend->RenderPrimitives(pFirstMeshChunk->mPrimitiveType, minVertex, maxVertex - minVertex, pFirstMeshChunk->mIndexBufferOffset, indexCount);
               ++gFixModel_NumRendered;
            }
         }
         else if( gFixModel_EnableEnvmap && isEmap )
         {
#if BP_ENABLE_DEBUG_MESH_GPU_MARKER
            BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass 2 Env");
#endif
            BP_BeginShader(MDBShader::gpShader[isTexture][isPreshade ? 1 : 0][isFog][isOptocamoflage][projectionMode][MDBShader::kUV_Envmap][gGS_ForceRGBWhite], NULL, 0);

            CVector4 const envmapParams(pTex->tex_trans.vec1.vx / pTex->u_scale, 
               pTex->tex_trans.vec1.vy / pTex->v_scale, 
               (pTex->tex_trans.vec2.vx - pTex->u_offset) / pTex->u_scale, 
               (pTex->tex_trans.vec2.vy - pTex->v_offset) / pTex->v_scale);
            gpRenderBackend->SetVertexRegisters(kSReg_EnvmapParams, 1, &envmapParams);

            for( int alphaTestPass = 0; alphaTestPass < gGS_AlphaFailPassCount; ++alphaTestPass )
            {
               BP_GS_SetupAlphaFailPass(gGS_AlphaFailPassStart + alphaTestPass);
               gpRenderBackend->RenderPrimitives(pFirstMeshChunk->mPrimitiveType, minVertex, maxVertex - minVertex, pFirstMeshChunk->mIndexBufferOffset, indexCount);
               ++gFixModel_NumRendered;
            }
         }
         else if( isSmap )
         {
            BP_RENDER_TODO_BREAK;
         }
      }
   }
};

//----------------------------------------------------------------------------

void BP_FixModel_Render(char* pData)
{
   if( !gFixModel_Enable || !gFixModel_EnableSingleTex )
      return;

   SBP_MDB_Render* pPacket = (SBP_MDB_Render*)pData;
   CMesh* pMesh = (CMesh*)pPacket->model;
   if( pMesh )
   {
      CMDBRender render(pPacket, MDBShader::gModelLocalParamShaderMode, pMesh);
      render.Setup();
      render.RenderSingle();
   }
}

//----------------------------------------------------------------------------

void BP_FixModel_RenderMultiTex(char* pData)
{
   if( !gFixModel_Enable || !gFixModel_EnableMultiTex )
      return;

   SBP_MDB_Render* pPacket = (SBP_MDB_Render*)pData;
   CMesh* pMesh = (CMesh*)pPacket->model;
   if( pMesh )
   {
      CMDBRender render(pPacket, MDBShader::gModelLocalParamShaderMode, pMesh);
      render.Setup();
      if( !gFixModel_ForceSingleTex )
         render.RenderMulti();
      else
         render.RenderSingle();
   }
}

//----------------------------------------------------------------------------

void BP_UpdateVertexAnime(void* pDGModel)
{
   // get the current model.
   DG_MODEL* pModel = (DG_MODEL*)pDGModel;

   // iterate over model units.

}

//----------------------------------------------------------------------------

void BP_FixModel_InitDebugMenu()
{
   int const renderMenu = BP_DebugMenu_GetMenu("Render");

   int const debugMenu = BP_DebugMenu_AddMenu("FixModel", renderMenu);
   BP_DebugMenu_AddBool(debugMenu, "Enable Render", &gFixModel_Enable);
   //
   BP_DebugMenu_AddBool(debugMenu, "Render Single Tex", &gFixModel_EnableSingleTex);
   BP_DebugMenu_AddBool(debugMenu, "Render Multi Tex", &gFixModel_EnableMultiTex);
   BP_DebugMenu_AddBool(debugMenu, "Force Single Tex", &gFixModel_ForceSingleTex);
   BP_DebugMenu_AddBool(debugMenu, "Enable Envmap", &gFixModel_EnableEnvmap);
   //
   BP_DebugMenu_AddBool(debugMenu, "Enable Preshade", &gFixModel_EnablePreshade);
   BP_DebugMenu_AddBool(debugMenu, "Enable Vertex Anim", &gFixModel_EnableVertexAnim);
   //
   BP_DebugMenu_AddBool(debugMenu, "Show Stats", &gFixModel_ShowStats);
}

//----------------------------------------------------------------------------
