//----------------------------------------------------------------------------
// BP_RenderEvm.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "BP_RenderEvm.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Primitive/CMesh.h"
#include "Renderer/Base/Primitive/CMeshBuffers.h"

#include "BP_Renderer.h"
#include "BP_RendererDebug.h"
#include "BP_RenderGS.h"

//----------------------------------------------------------------------------

#include "MGS_Common.h"

#include "BP_RenderBufferTypes.h"
#include "BP_RenderShared.h"

#include "bp_matrix.h"

extern "C" void DG_GetTexelInfo_BP( int *width, int *height, int *offset_x, int *offset_y, DG_TEX_BP *tex );

#undef BPE_FORCEINLINE
#define BPE_FORCEINLINE

#if BP_VITA
#  define USE_JOINT_UNIFORMS 1
#else
#  define USE_JOINT_UNIFORMS 0
#endif

//----------------------------------------------------------------------------

namespace
{
   int gNumRendered;
   int gEndingDogTagOverride = 0;
}

int gEvm_EnableSingleTex = 1;
int gEvm_EnableMultiTex = 1;
int gEvm_ForceSingleTex = 0;
int gEvm_EnableEnvmap = 1;
int gEvm_EnableBump = 1;
float gEvm_BumpScaleS = 1.0f;
float gEvm_BumpScaleT = 1.0f;
int gEvm_OptimizeAlphaTest_EnableFrameToggle = 0;
int gEvm_OptimizeAlphaTest = 1;
int gEvm_ShowStats = 0;
int gEvm_ForceDisableTex1 = 0;
int gEvm_ForceDisableTex2 = 0;
int gEvm_ForceDisableTex3 = 0;

//----------------------------------------------------------------------------

namespace EvmShader
{
   enum EUVType
   {
      kUV_Set0,
      kUV_Set1,
      kUV_Set2,
      kUV_Envmap,

      kUV_Count
   };

   enum EMultiType
   {
      kMT_Normal,
      kMT_BumpMap,
      
      kMT_Count
   };

   enum ESpecial
   {
      kS_None,
      kS_SolMant,

      kS_Count
   };

   int const kTextureCount = 2;
   CCompiledShader*  gpShader[kTextureCount][kUV_Count][kMT_Count][kS_Count]
#if BP_VITA
      [skGS_Vita_NumSpecialBlends][CRenderBackend::kAF_Count]
#endif
      = { 0 };
   struct SShaderOptions
   {
      SShaderOptions( int const texture, int const uv, int const mt, EvmShader::ESpecial const special)
         : mTexture( texture )
         , mUv( uv )
         , mMt( mt )
         , mSpecial( special )
      {
      }

      int mTexture;
      int mUv;
      int mMt;
      EvmShader::ESpecial mSpecial;
   };

   BPE_FORCEINLINE CCompiledShader *Shader( int const texture, int const uv, int const mt, int const special )
   {
      CCompiledShader* pShader = gpShader[ texture ][ uv ][ mt ][special]
#if BP_VITA
      [gGS_Vita_CurrentSpecialBlend][ BP_GS_GetCurrentVitaAlphaFunc() ]
#endif
      ;

      BPE_ASSERT_NO_MSG(pShader != NULL);
      return pShader;
   }

   BPE_FORCEINLINE CCompiledShader *Shader( SShaderOptions const &options )
   {
      return Shader( options.mTexture, options.mUv, options.mMt, options.mSpecial );
   }

   enum ERegisters
   {
      kReg_Persp           = 16,
      kReg_EyeInv          = 20,
#if !USE_JOINT_UNIFORMS
      kReg_Matrix0         = 24,
#endif
      kReg_BumpLightVec    = 48,
      kReg_SolMantValue    = 49,
   };
}

void BP_InitEvmShader()
{
   CShaderFileId const shaderId("$/EngineSupport/Shaders/EVM.fx");

   for( int texture = 0; texture < EvmShader::kTextureCount; ++texture )
   {
      for( int uv = 0; uv < EvmShader::kUV_Count; ++uv )
      {
         for( int multiType = 0; multiType < EvmShader::kMT_Count; ++multiType )
         {
            for( int special = 0; special < EvmShader::kS_Count; ++special )
            {
               if ( uv == 2 && multiType == 1 )
               {
                  continue;
               }
#if BP_VITA
               for ( int blendMode = 0; blendMode < skGS_Vita_NumSpecialBlends; ++blendMode )
               {
                  for ( int alphaFunc = 0; alphaFunc < CRenderBackend::kAF_Count; alphaFunc++ )
                  {
                     if (BP_IsValidVitaShaderCombination(blendMode, alphaFunc))
                     {
                        CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(shaderId, 
                           CStringExtras::Stringize_s("TEXTURE=%d;UV=%d;MULTI_TYPE=%d;SPECIAL=%d;VITA_SPECIAL_BLEND=%d;VITA_PS2_ATEST_FUNC=%d", 
                           texture, uv, multiType, special, blendMode, alphaFunc ) );

                        BPE_ASSERT_NO_MSG(pShader);
                        EvmShader::gpShader[texture][uv][multiType][special][blendMode][alphaFunc] = pShader;
                     }
                  }
               }
#else
               CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(shaderId, CStringExtras::Stringize_s("TEXTURE=%d;UV=%d;MULTI_TYPE=%d;SPECIAL=%d", texture, uv, multiType, special));
               assert(pShader);
               EvmShader::gpShader[texture][uv][multiType][special] = pShader;
#endif
            }

         }
      }
   }
}

//----------------------------------------------------------------------------

void BP_Evm_FrameBegin()
{
   gNumRendered = 0;

   if( gEvm_OptimizeAlphaTest_EnableFrameToggle )
   {
      gEvm_OptimizeAlphaTest = !gEvm_OptimizeAlphaTest;
   }

   gEndingDogTagOverride = ( strcmp(GM_GetArea(), "d082p01") == 0 ) ? 1 : 0;
}

void BP_Evm_FrameEnd()
{
   if( gEvm_ShowStats )
   {
      BP_DebugText_Print("Evm Count: %d", gNumRendered);
   }
}

//----------------------------------------------------------------------------

void BP_Evm_InitPacket(char* pData)
{
   //Restore test value, this is to fix disabled z buffer testing in some cases when drawing the codec
   BP_RestoreFrameInitTestValue();

   SBP_EVM_InitPacket* pPacket = (SBP_EVM_InitPacket*)pData;

   gpRenderBackend->SetDepthCompareEnabled(true);
   gpRenderBackend->SetDepthFunc(CRenderBackend::kDF_GEqual);
   gpRenderBackend->SetDepthWriteEnabled(true);

   real32 temp[16];

   BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->eye_pers, temp);
   gpRenderBackend->SetVertexRegisters(EvmShader::kReg_Persp, 4, (CVector4 const*)temp);

   BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->eye_inv, temp);
   gpRenderBackend->SetVertexRegisters(EvmShader::kReg_EyeInv, 4, (CVector4 const*)temp);

   extern CVector4 gBP_FogColor;
   gpRenderBackend->SetVertexRegisters(kSReg_FogColor, 1, &gBP_FogColor);

   gpRenderBackend->SetVertexRegisters(kSReg_FogParam, 1, (CVector4 const*)&pPacket->fogParam);
}

//----------------------------------------------------------------------------

namespace
{
#if !USE_JOINT_UNIFORMS
   FMATRIX* gCurrentSkinMatrices = NULL;
   uint8 gCurrentSkinLastLoadedJoints[ 255 ] = { 0 };
#endif

   unsigned int gIsIRMode = 0;
   unsigned int gSpecialModel = 0;
}

#if USE_JOINT_UNIFORMS

void load_joints_to_uniforms( FMATRIX const *incomingMatrices, int const jointCount )
{
   if ( jointCount > 0 )
   {
      void *pUniformMemory = gpRenderBackend->AllocOneFrameVertexUniformBuffer( 0, jointCount * 12 * sizeof( float ) );

      for ( int joint = 0; joint < jointCount; ++joint )
      {
         float *pUniformMemoryOutF32 = reinterpret_cast< float * >( pUniformMemory ) + 12 * joint;
         BP_Matrix44_to_VS_Matrix43( (real32 const*)&incomingMatrices[joint], pUniformMemoryOutF32 );
      }
   }
}

#endif

void BP_Evm_LocalParam(char* pData)
{
   SBP_EVM_LocalParam* pPacket = (SBP_EVM_LocalParam*)pData;

#if !USE_JOINT_UNIFORMS
   gCurrentSkinMatrices = pPacket->matrix;
   KP_CTASSERT( sizeof( gCurrentSkinLastLoadedJoints[0] ) == 1 );
   memset( gCurrentSkinLastLoadedJoints, 0xFF, sizeof( gCurrentSkinLastLoadedJoints ) );
#endif
   gIsIRMode = pPacket->isInIRMode;
   gSpecialModel = pPacket->specialModel;

   CMesh* pMesh = (CMesh*)pPacket->model;
   if( pMesh )
   {
#if USE_JOINT_UNIFORMS
      load_joints_to_uniforms( pPacket->matrix, pMesh->GetJointCount() );
#endif
      CShaderVertexDataBinding binding;
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord0, kVDS_TexCoord0);
      binding.Set(kVDU_TexCoord2, kVDS_Normal);
      binding.Set(kVDU_TexCoord3, kVDS_TexCoord1);
      binding.Set(kVDU_TexCoord4, kVDS_TexCoord2);
      binding.Set(kVDU_BlendIndices, kVDS_BlendIndices);
      binding.Set(kVDU_BlendWeight, kVDS_BlendWeight);

      CMeshBuffers & meshBuffers = const_cast<CMeshBuffers&>(pMesh->GetMeshBuffers());
      meshBuffers.SetVertexData(binding);

      // Set Index Buffer
      gpRenderBackend->SetIndexData(meshBuffers.GetIndexBuffer());

      real32 temp[16];
      BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->lightDir, temp);
      gpRenderBackend->SetVertexRegisters(kSReg_LightDir, 4, (CVector4 const*)temp);

      BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->lightCol, temp);
      gpRenderBackend->SetVertexRegisters(kSReg_LightCol, 4, (CVector4 const*)temp);

      // Setup special model value.
      {
         temp[0] = pPacket->specialModelValue;
         gpRenderBackend->SetVertexRegisters(EvmShader::kReg_SolMantValue, 1, (CVector4 const*)temp);
      }


      // Bump light vec
      {
         static FVECTOR	nomo_const = { 0.30f, 0.59f, 0.11f, 1.0f };
         FVECTOR		bump_light, light_force ;

         light_force.vx = _sceVu0InnerProduct( pPacket->lightCol.m[0], &nomo_const );
         light_force.vy = _sceVu0InnerProduct( pPacket->lightCol.m[1], &nomo_const );
         light_force.vz = _sceVu0InnerProduct( pPacket->lightCol.m[2], &nomo_const );

         bump_light.vx = _sceVu0InnerProduct( &pPacket->lightDir.m[0], &light_force );
         bump_light.vy = _sceVu0InnerProduct( &pPacket->lightDir.m[1], &light_force );
         bump_light.vz = _sceVu0InnerProduct( &pPacket->lightDir.m[2], &light_force );

         _sceVu0Normalize( &bump_light, &bump_light );

         gpRenderBackend->SetVertexRegisters(EvmShader::kReg_BumpLightVec, 1, (CVector4 const*)&bump_light);
      }
   }
}

//----------------------------------------------------------------------------

class CEvmRender
{
public:
   SBP_EVM_Render* const   mpPacket;
   CMesh* const            mpMesh;

   int const isTexture;

   int chunkStart, chunkCount;

   BPE_FORCEINLINE CEvmRender(SBP_EVM_Render* pPacket, CMesh* pMesh)
      :  mpPacket(pPacket)
      ,  mpMesh(pMesh)
      ,  isTexture( !gIsIRMode )
   {
   }

   BPE_FORCEINLINE void Setup()
   {
      if( mpPacket->flag & (DG_PACKFLAG_CULLAUTO|DG_PACKFLAG_CULLON) )
         gpRenderBackend->SetCullMode(CRenderBackend::kCM_CCW);
      else 
         gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

      mpMesh->GetChunkRangeForUnit(0, &chunkStart, &chunkCount);
   }

   BPE_FORCEINLINE void SetupTextureParameters(DG_TEX_BP* pTex)
   {
      BP_SetTexture(pTex, 0);

      if( gIsIRMode )
         BP_GS_SetAlpha(0);
      else
         BP_GS_SetAlpha(pTex->tex_trans.alpha.data);
   }

#if !USE_JOINT_UNIFORMS
   void load_joint_map( uint32 const *pJointMap, uint32 const jointMapCount )
   {
      BPE_VERIFY( jointMapCount < 64, false, "Bad joint map count" );

      for( int j = 0; j < jointMapCount; ++j )
      {
         int const matrixIdx = pJointMap[j];

         if ( gCurrentSkinLastLoadedJoints[j] != matrixIdx )
         {
            gCurrentSkinLastLoadedJoints[j] = matrixIdx;

            float temp[12];
            BP_Matrix44_to_VS_Matrix43((real32 const*)&gCurrentSkinMatrices[matrixIdx], temp);

            gpRenderBackend->SetVertexRegisters(EvmShader::kReg_Matrix0 + j * 3, 3, (CVector4 const*)temp);
         }
      }
   }

   void load_joint_map( CMeshChunk const * const pMeshChunk )
   {
      // Load skinning matrices
      unsigned int const jointMapCount = pMeshChunk->mJointMap.size();

      load_joint_map( jointMapCount ? &( pMeshChunk->mJointMap[0] ) : NULL , jointMapCount );
   }

#endif
   BPE_FORCEINLINE void RenderAllPrimitives(DG_TEX_BP* pTex, EvmShader::SShaderOptions const &shader )
   {
      int const packetIdxMin = mpPacket->startPacket;
      int const packetIdxMax = mpPacket->startPacket + mpPacket->packetCount;
      int currentPacketIdx = packetIdxMin;

      while ( currentPacketIdx != packetIdxMax )
      {
         SMeshChunkRange meshChunk;
         mpMesh->GetMeshChunkRange( &meshChunk, chunkStart + currentPacketIdx, packetIdxMax - currentPacketIdx );

         currentPacketIdx += meshChunk.mChunkCount;

#if !BP_VITA
         load_joint_map( meshChunk.mJointMapPtr, meshChunk.mJointMapCount );
#endif

         int AlphaFailPassCount_Optimized = gGS_AlphaFailPassCount;
         SBP_TestState* pOptimizeTestState = NULL;
         if( gEvm_OptimizeAlphaTest && gGS_AlphaFailPassCount > 1 )
         {
            int maxAlpha;
            int minAlpha;

            if( isTexture && pTex && pTex->BP_TextureHandle )
            {
               CBaseTexture const * pTexture = (CBaseTexture*)pTex->BP_TextureHandle;
               // gMaxConstantParamAlpha && gMinConstantParamAlpha are always 1.0 for EVM.fx
               maxAlpha = ((pTexture->mMaxRGBA&0x000000FF))<<1; // simulate *2 alpha in shader code
               minAlpha = ((pTexture->mMinRGBA&0x000000FF))<<1; // simulate *2 alpha in shader code
               // MGSTWO-3127 solidus' mantle needs to keep the alpha test on no matter what, or the 
               //       envmap pass will leave a halo in the particles
               if (shader.mSpecial)
               {
                  minAlpha = 0;
               }
            }
            else
            {
               maxAlpha = minAlpha = 0xFF;
            }

            AlphaFailPassCount_Optimized = BP_GS_AlphaFailPass_Optimize(minAlpha, maxAlpha, minAlpha, &pOptimizeTestState);
         } 

         for( int alphaTestPass = 0; alphaTestPass < AlphaFailPassCount_Optimized; ++alphaTestPass )
         {
            BP_GS_SetupAlphaFailPass_Optimize(gGS_AlphaFailPassStart + alphaTestPass, pOptimizeTestState);
            BP_BeginShader( EvmShader::Shader( shader ), NULL, 0 );
            gpRenderBackend->RenderPrimitives(meshChunk.mPrimitiveType, 
#if RENDERBACKEND_PLATFORM_NEEDS_VERTEX_BUFFER_OFFSET()
               meshChunk.mVertexBufferOffset, meshChunk.mVertexCount, 
#endif
               meshChunk.mIndexBufferOffset, meshChunk.mIndicesCount);
            ++gNumRendered;
         }
      }
   }

   BPE_FORCEINLINE void RenderSingle()
   {
#if BP_ENABLE_DEBUG_MESH_GPU_MARKER
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("EVM Single");
#endif
      DG_TEX_BP* pTex = mpPacket->tex[0];
      if( pTex != NULL )
      {
         SetupTextureParameters(pTex);

         RenderAllPrimitives(pTex, EvmShader::SShaderOptions( isTexture, EvmShader::kUV_Set0, EvmShader::kMT_Normal, gSpecialModel ? EvmShader::kS_SolMant : EvmShader::kS_None ) );
      }
   }

   BPE_FORCEINLINE void RenderMulti()
   {
#if BP_ENABLE_DEBUG_MESH_GPU_MARKER
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("EVM Multi");
#endif
      int const isTex0 = mpPacket->flag & DG_PACKFLAG_TEX0;
      int const isTex1 = mpPacket->flag & DG_PACKFLAG_TEX1;
      int const isTex2 = mpPacket->flag & DG_PACKFLAG_TEX2;
      int const isUV0 = mpPacket->flag & DG_PACKFLAG_UV0;
      int const isUV1 = mpPacket->flag & DG_PACKFLAG_UV1;
      int const isUV2 = mpPacket->flag & DG_PACKFLAG_UV2;
      int const isEmap = mpPacket->flag & DG_PACKFLAG_EMAP;
      int const isSmap = mpPacket->flag & DG_PACKFLAG_SMAP;
      int const isBmap = mpPacket->flag & DG_PACKFLAG_BMAP;

      if( !isBmap )
      {
         if( isSmap )
         {
            BP_RENDER_TODO_BREAK;
         }

         // Render first pass
         if( isTex0 && !gEvm_ForceDisableTex1)
         {
#if BP_ENABLE_DEBUG_MESH_GPU_MARKER
            BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass 0");
#endif
            DG_TEX_BP* pTex = mpPacket->tex[0];
            if( pTex != NULL )
            {
               SetupTextureParameters(pTex);
               RenderAllPrimitives(pTex, EvmShader::SShaderOptions( isTexture, EvmShader::kUV_Set0, EvmShader::kMT_Normal, gSpecialModel ? EvmShader::kS_SolMant : EvmShader::kS_None ));
            }
         }

         // Render second pass
         if( isTex1 && !gEvm_ForceDisableTex2 )
         {
#if BP_ENABLE_DEBUG_MESH_GPU_MARKER
            BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass 1");
#endif
            DG_TEX_BP* pTex = mpPacket->tex[1];
            if( pTex != NULL )
            {
               SetupTextureParameters(pTex);

               RenderAllPrimitives(pTex, EvmShader::SShaderOptions( isTexture, EvmShader::kUV_Set1, EvmShader::kMT_Normal, gSpecialModel ? EvmShader::kS_SolMant : EvmShader::kS_None ));
            }
         }

         // Render third pass
         if( isTex2 && !gEvm_ForceDisableTex3 )
         {
            DG_TEX_BP* pTex = mpPacket->tex[2];
            if( pTex != NULL )
            {
               SetupTextureParameters(pTex);

               if( isUV2 )
               {
#if BP_ENABLE_DEBUG_MESH_GPU_MARKER
                  BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass 2");
#endif
                  RenderAllPrimitives(pTex, EvmShader::SShaderOptions( isTexture, EvmShader::kUV_Set2, EvmShader::kMT_Normal, gSpecialModel ? EvmShader::kS_SolMant : EvmShader::kS_None ));
               }
               else if( gEvm_EnableEnvmap && isEmap )
               {
#if BP_ENABLE_DEBUG_MESH_GPU_MARKER
                  BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass 2 Env");
#endif
                  CVector4 const envmapParams(pTex->tex_trans.vec1.vx / pTex->u_scale, 
                                              pTex->tex_trans.vec1.vy / pTex->v_scale, 
                                              (pTex->tex_trans.vec2.vx - pTex->u_offset) / pTex->u_scale, 
                                              (pTex->tex_trans.vec2.vy - pTex->v_offset) / pTex->v_scale);

                  gpRenderBackend->SetVertexRegisters(kSReg_EnvmapParams, 1, &envmapParams);

                  RenderAllPrimitives(pTex, EvmShader::SShaderOptions( isTexture, EvmShader::kUV_Envmap, EvmShader::kMT_Normal, gSpecialModel ? EvmShader::kS_SolMant : EvmShader::kS_None ));
               }
            }
         }
      }
      else
      {
         // Bump map rendering

#if BP_ENABLE_DEBUG_MESH_GPU_MARKER
         BPE_ADD_SCOPED_GPU_PROFILE_MARKER(CStringExtras::Stringize_s("Bump: %s", mpMesh->mDebugName.c_str()));
#endif
         DG_TEX_BP* pColorTex = mpPacket->tex[0];

         if( pColorTex != NULL )
            SetupTextureParameters(pColorTex);

         // Set bump map texture
         DG_TEX_BP* pBumpTex = mpPacket->tex[1];
         BP_SetTexture(pBumpTex, 1);

         int width, height, offsetX, offsetY;
         DG_GetTexelInfo_BP(&width, &height, &offsetX, &offsetY, pBumpTex);

         if( gEvm_EnableBump )
         {
            if( !gEndingDogTagOverride )
            {
               CVector4 const bumpParams(-gEvm_BumpScaleS / width, 
                                         -gEvm_BumpScaleT / height, 
                                         0.0f, 0.0f);

               gpRenderBackend->SetVertexRegisters(kSReg_EnvmapParams, 1, &bumpParams);
            }
            else
            {
               // Custom tweaked setting to look good for ending dog tag in MGS2.
               CVector4 const bumpParams(-4.0f / width, 
                                          8.0f / height, 
                                          0.0f, 0.0f);

               gpRenderBackend->SetVertexRegisters(kSReg_EnvmapParams, 1, &bumpParams);
            }
         }
         else
         {
            CVector4 const bumpParams = CVector4::Zero();
            gpRenderBackend->SetVertexRegisters(kSReg_EnvmapParams, 1, &bumpParams);
         }

         RenderAllPrimitives(pColorTex, EvmShader::SShaderOptions( isTexture, EvmShader::kUV_Set0, EvmShader::kMT_BumpMap, gSpecialModel ? EvmShader::kS_SolMant : EvmShader::kS_None ));
      }
   }
};

//----------------------------------------------------------------------------

void BP_Evm_Render(char* pData)
{
   if( !gEvm_EnableSingleTex )
      return;

   SBP_EVM_Render* pPacket = (SBP_EVM_Render*)pData;

   CMesh* pMesh = (CMesh*)pPacket->model;
   if( pMesh )
   {
      CEvmRender render(pPacket, pMesh);
      render.Setup();
      render.RenderSingle();
   }
}

//----------------------------------------------------------------------------

void BP_Evm_RenderMulti(char* pData)
{
   if( !gEvm_EnableMultiTex )
      return;

   SBP_EVM_Render* pPacket = (SBP_EVM_Render*)pData;

   CMesh* pMesh = (CMesh*)pPacket->model;
   if( pMesh )
   {
      CEvmRender render(pPacket, pMesh);
      render.Setup();
      if( !gEvm_ForceSingleTex )
         render.RenderMulti();
      else
         render.RenderSingle();
   }
}

//----------------------------------------------------------------------------

void BP_Evm_InitDebugMenu()
{
#if BP_ENABLE_DEBUG_MENU
   int const renderMenu = BP_DebugMenu_GetMenu("Render");

   int const debugMenu = BP_DebugMenu_AddMenu("Evm", renderMenu);
   BP_DebugMenu_AddBool(debugMenu, "Render Single Tex", &gEvm_EnableSingleTex);
   BP_DebugMenu_AddBool(debugMenu, "Render Multi Tex", &gEvm_EnableMultiTex);
   BP_DebugMenu_AddBool(debugMenu, "Force Single Tex", &gEvm_ForceSingleTex);
   BP_DebugMenu_AddBool(debugMenu, "Enable Envmap", &gEvm_EnableEnvmap);
   BP_DebugMenu_AddBool(debugMenu, "Enable Bump", &gEvm_EnableBump);
   BP_DebugMenu_AddFloat(debugMenu, "Bump Scale S", &gEvm_BumpScaleS, -8.0f, 8.0f, 0.01f, 0.1f);
   BP_DebugMenu_AddFloat(debugMenu, "Bump Scale T", &gEvm_BumpScaleT, -8.0f, 8.0f, 0.01f, 0.1f);

   BP_DebugMenu_AddBool(debugMenu, "Enable optimize alpha test", &gEvm_OptimizeAlphaTest);
   BP_DebugMenu_AddBool(debugMenu, "Enable optimize alpha test every other frame", &gEvm_OptimizeAlphaTest_EnableFrameToggle);

   BP_DebugMenu_AddBool(debugMenu, "Show Stats", &gEvm_ShowStats);

   BP_DebugMenu_AddBool(debugMenu, "Disable Tex 1", &gEvm_ForceDisableTex1);
   BP_DebugMenu_AddBool(debugMenu, "Disable Tex 2", &gEvm_ForceDisableTex2);
   BP_DebugMenu_AddBool(debugMenu, "Disable Tex 3", &gEvm_ForceDisableTex3);
#endif
}

