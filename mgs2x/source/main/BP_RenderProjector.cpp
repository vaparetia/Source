//----------------------------------------------------------------------------
// BP_RenderProjector.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "BP_RenderProjector.h"

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

#if BPE_IS_ENDIAN_BIG()
#define BP_PACK_COLOR(R, G, B, A) ( R << 24 | G << 16 | B << 8 | A )
inline unsigned int ConvertColor_MGS_TO_BP(unsigned int mgsColor)
{
   unsigned int color =
   ( ( mgsColor & 0x000000FF ) << 24 ) |
   ( ( mgsColor & 0x0000FF00 ) << 8 ) |
   ( ( mgsColor & 0x00FF0000 ) >> 8 ) |
   ( ( mgsColor & 0xFF000000 ) >> 24 );

   return color;
}
#else
#define BP_PACK_COLOR(R, G, B, A) ( A << 24 | B << 16 | G << 8 | R )
inline unsigned int ConvertColor_MGS_TO_BP(unsigned int mgsColor)
{
   return mgsColor;
}
#endif

namespace ProjectionShader
{
   int const kModeCount = 5;
   int const M_COPY_VERTEXCOLOR_KEEP_DEPTH = 0;
   int const M_COPY_TEXTURE_KEEP_DEPTH = 1;
   int const M_BLUR_TEXTURE = 2;
   int const M_DRAW_MODEL_BUILD_PROJECTION = 3;
   int const M_DRAW_MODEL_USE_PROJECTION_TEXTURE = 4;
   CCompiledShader* gpShader[kModeCount];
   SRenderTarget gOriginalRenderTarget;

   enum ERegisters
   {
      kReg_Matrix0      = 16,
      kReg_Persp        = 20,
      kReg_Correction   = 24,
      kReg_TextureSize  = 32,
   };

#if BP_VITA
   int const kMaxShadowTextures = 32;
   CBaseTexture* pShadowTarget;
   CBaseTexture* pShadowDepth;
   CBaseTexture* pShadowTextures[kMaxShadowTextures];
#endif
}

//Private helper functions
//----------------------------------------------------------------------------
CVertexData gTriFanSpot_VertexData;
void BP_DrawTriFanSpot(unsigned int mgsColor)
{
   //since we are not drawing to the entire surface we have to clear out any alpha set by other shadow casters
   //in the future we could have the preZ pass not write to the color buffer, this would also allow X360 to do 4x z write speed
   gpRenderBackend->Clear(CRenderBackend::kFlag_Color, CColor::Zero(), 0.0f);

   struct TriSpotFanVertex
   {
      float x,y,z;
      unsigned int color;
   };   

   static const int kMaxTriSpotFanVerts = 18;
   static TriSpotFanVertex triSpotFanVertex[kMaxTriSpotFanVerts];
   static int sInitFlag = 0;
   
   if( sInitFlag == 0 )
   {
      float	a ;
      for( int ii = 1 ; ii < kMaxTriSpotFanVerts ; ++ii )
      {
         // values are originally from DG_WritePacket_Circle
         /*
		   for ( i = 0 ; i < 18 ; i++ )
         {
			     SCE_GS_SET_XYZ2( (int)(2048.0f+cosf(a)*120)*16, (int)(2048.0f+sinf(a)*120)*16, 0 );
		   }
		   gif_buffer[0] = SCE_GS_SET_XYZ2( ( 2048 - 0 ) * 16, ( 2048 - 0 ) * 16, 0 ) ;
         */
         a = (float)((ii-1)&15) / 8.0f * (float)3.14159265f; 
//          triSpotFanVertex[ii].x = 0.0f+cosf(a)*(120.0f/4095.9375f)*(2.0f);
//          triSpotFanVertex[ii].y = 0.0f+sinf(a)*(120.0f/4095.9375f)*(2.0f);
         triSpotFanVertex[ii].x = 0.0f+cosf(a);
         triSpotFanVertex[ii].y = 0.0f+sinf(a);
         triSpotFanVertex[ii].z = 0.0f;
      }
      triSpotFanVertex[0].x = 0.0f;
      triSpotFanVertex[0].y = 0.0f;
      triSpotFanVertex[0].z = 0.0f;

      sInitFlag = 1;
   }
   //colors are animated so they must be updated per function call,
   //we could store these off into a contant, if this is every a performance concern
   unsigned int color = ConvertColor_MGS_TO_BP(mgsColor);
   unsigned int exteriorColor = BP_PACK_COLOR(0, 0, 0, 128);
   {
      for( int ii = 1 ; ii < kMaxTriSpotFanVerts ; ++ii )
      {
         triSpotFanVertex[ii].color = exteriorColor;
      }
      triSpotFanVertex[0].color = color;
   }

   CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(TriSpotFanVertex) * kMaxTriSpotFanVerts);
   if( pVertexBuffer )
   {
      TriSpotFanVertex* pVertex = (TriSpotFanVertex*)pVertexBuffer->Lock();
      memcpy(pVertex, triSpotFanVertex, kMaxTriSpotFanVerts*sizeof(TriSpotFanVertex) );
      pVertexBuffer->Unlock();

      gTriFanSpot_VertexData.ClearVertexBuffers();
      size_t const bufferIndex = gTriFanSpot_VertexData.AddBuffer( pVertexBuffer, sizeof( TriSpotFanVertex ) );

      gTriFanSpot_VertexData.SetAttribute(kVDS_Position , offsetof(TriSpotFanVertex, x)    , kVDT_Float3, bufferIndex);
      gTriFanSpot_VertexData.SetAttribute(kVDS_TexCoord1, offsetof(TriSpotFanVertex, color), kVDT_UByte4N, bufferIndex);

      CShaderVertexDataBinding binding;
      binding.Set(kVDU_Position, kVDS_Position);
      binding.Set(kVDU_TexCoord1, kVDS_TexCoord1);
      gpRenderBackend->SetVertexData(binding, gTriFanSpot_VertexData, 0, 0);
      gpRenderBackend->ForceVertexDataRebind();

      // render states
      BP_GS_SetAlpha( SCE_GS_SET_ALPHA(0,0,0,0,0) );

      gpRenderBackend->SetDepthCompareEnabled(false);
      gpRenderBackend->SetDepthWriteEnabled(false);
      BP_BeginShader(ProjectionShader::gpShader[ProjectionShader::M_COPY_VERTEXCOLOR_KEEP_DEPTH], NULL, 0);
      gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleFan, 0, kMaxTriSpotFanVerts);
      // restore state
      gpRenderBackend->SetDepthCompareEnabled(true);
      gpRenderBackend->SetDepthWriteEnabled(true);
   }
}

//Public functions

#if BP_VITA
namespace FX
{
   extern CBaseTexture *pBlurTarget;
};
#endif

void BP_InitProjectorShader()
{
   using namespace ProjectionShader;

   CShaderFileId const shaderId("$/EngineSupport/Shaders/Projector.fx");

   for( int mode = 0; mode < kModeCount; ++mode )
   {
      CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(shaderId, CStringExtras::Stringize_s("MODE=%d", mode));
      assert(pShader);

      gpShader[mode] = pShader;
   }

#if BP_VITA
   SCreateTextureParams shadowParams;
   shadowParams.mNumScenes = 1;

   FX::pBlurTarget->Lock((void**)&shadowParams.mBaseAddress, &shadowParams.mStride);
   unsigned char *p = (unsigned char*)shadowParams.mBaseAddress;
   pShadowTarget = CBaseTexture::Create(1024, 512, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_RenderTarget, CBaseTexture::kAA_None, kRM_System, &shadowParams);
   gpRenderBackend->GetCurrentDisplayDepth()->Lock((void**)&shadowParams.mBaseAddress, &shadowParams.mStride);
   shadowParams.mStride = 1024 * 4;
   pShadowDepth = CBaseTexture::Create(1024, 512, 1, CBaseTexture::kFormat_D24X8, CBaseTexture::kUsage_DepthBuffer, CBaseTexture::kAA_None, kRM_System, &shadowParams);

   int i, j;
   shadowParams.mStride = 1024 * 4;
   for (j = 0; j < 4; j++)
      for (i = 0; i < 8; i++)
      {
         shadowParams.mBaseAddress = (unsigned int)(p + 1024 * 4 * (j * 128) + i * 128 * 4);
         pShadowTextures[i + j*8] = CBaseTexture::Create(128, 128, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_Texture, CBaseTexture::kAA_None, kRM_System, &shadowParams);
      }
#endif
}

//Shadow stage
//----------------------------------------------------------------------------

void BP_Projector_ShadowStage_StartPacket()
{
   ProjectionShader::gOriginalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();
   BP_GS_SetAlphaTestEnabled( false );
//   gpRenderBackend->SetAlphaTestEnable(false);
}

//----------------------------------------------------------------------------

void BP_Projector_ShadowStage_InitPacket(char* pData)
{
   SBP_Projector_ShadowStage_InitPacket* pPacket = (SBP_Projector_ShadowStage_InitPacket*)pData;

   CBaseTexture* pShadowColor = BP_GetRenderTarget(kRT_TempShadow);
   CBaseTexture* pShadowDepth = BP_GetRenderTarget(kRT_TempShadow_Depth);
   gpRenderBackend->SetRenderTarget(SRenderTarget(pShadowColor, NULL, NULL, NULL, pShadowDepth));

   gpRenderBackend->Clear(CRenderBackend::kFlag_Color|CRenderBackend::kFlag_Depth, CColor::Zero(), 0.0f);

   real32 temp[16];
   BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->eye_pers, temp);
   gpRenderBackend->SetVertexRegisters(ProjectionShader::kReg_Persp, 4, (CVector4 const*)temp);

   BP_GS_SetTest(SCE_GS_SET_TEST( 0, 5, 64, 1, 0, 0, 1, 2 ));
   BP_GS_SetupAlphaTestSinglePass(false);
}

//----------------------------------------------------------------------------

void BP_Projector_ShadowStage_PostZScreenPacket()
{   
   gpRenderBackend->Clear(CRenderBackend::kFlag_Color, CColor::Zero(), 0.0f);
}

//----------------------------------------------------------------------------

void BP_Projector_ShadowStage_ShadowProjStart(char* pData)
{
#if BP_VITA
   SBP_Projector_ShadowStage_ShadowProjStart* pPacket = (SBP_Projector_ShadowStage_ShadowProjStart*)pData;

   CBaseTexture* pShadowColor = BP_GetRenderTarget(kRT_TempShadow);
   CBaseTexture* pShadowDepth = BP_GetRenderTarget(kRT_TempShadow_Depth);

   gpRenderBackend->SetRenderTarget(ProjectionShader::gOriginalRenderTarget);

   // set texture
   gpRenderBackend->SetTexture(0, pShadowColor);

   BP_GS_SetAlpha( SCE_GS_SET_ALPHA(0,1,0,1,0) ); // from Shadow_Texture

#if !BPE_PIXELCENTER_MATCH_TEXELCENTER
   CVector4 const uvOffset(0.5f / 256.0f, 0.5f / 256.0f, 0, 0);
   gpRenderBackend->SetVertexRegisters(kSReg_Sampler0Info, 1, &uvOffset);
#endif

   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   gpRenderBackend->SetDepthCompareEnabled(true);
   gpRenderBackend->SetDepthWriteEnabled(true);
   
   real32 temp[16];
   BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->eye_pers, temp);
   gpRenderBackend->SetVertexRegisters(ProjectionShader::kReg_Persp, 4, (CVector4 const*)temp);

   BP_GS_SetTest(SCE_GS_SET_TEST( 1, 2, 0, 1, 0, 0, 1, 2 ));
   BP_GS_SetupAlphaTestSinglePass(false);
#else
   ///////
   SBP_Projector_ShadowStage_ShadowProjStart* pPacket = (SBP_Projector_ShadowStage_ShadowProjStart*)pData;

   CBaseTexture* pShadowColor = BP_GetRenderTarget(kRT_TempShadow);
   CBaseTexture* pShadowColor2 = BP_GetRenderTarget(kRT_TempShadow2);
   CBaseTexture* pShadowDepth = BP_GetRenderTarget(kRT_TempShadow_Depth);
   gpRenderBackend->ResolveRenderTarget(pShadowColor);

   //now we get to make the texture "soft"
   BP_GS_SetAlpha( SCE_GS_SET_ALPHA(0,0,0,0,0) );

   gpRenderBackend->SetTexture(0, pShadowColor);
   gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
#if !BPE_PIXELCENTER_MATCH_TEXELCENTER
   CVector4 const uvOffset(0.5f / pShadowColor2->GetWidth(), 0.5f / pShadowColor2->GetHeight(), 0, 0);
   gpRenderBackend->SetVertexRegisters(kSReg_Sampler0Info, 1, &uvOffset);
#endif
   SRenderTarget softShadowRenderTarget(pShadowColor2, NULL, NULL, NULL, pShadowDepth);
   softShadowRenderTarget.mScissorEnabled = true;
   softShadowRenderTarget.mScissorX = 0;
   softShadowRenderTarget.mScissorY = 0;
   softShadowRenderTarget.mScissorWidth = pShadowColor2->GetWidth()-1;
   softShadowRenderTarget.mScissorHeight = pShadowColor2->GetHeight()-1;
   gpRenderBackend->SetRenderTarget(softShadowRenderTarget);
   gpRenderBackend->SetDepthCompareEnabled(false);
   gpRenderBackend->SetDepthWriteEnabled(false);
   BP_BeginShader(ProjectionShader::gpShader[ProjectionShader::M_BLUR_TEXTURE], NULL, 0);
   //gpRenderBackend->RenderFullscreenQuad(1.0f);
   gpRenderBackend->SetAlphaTestEnable(false);
   gpRenderBackend->RenderQuad(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.5f/pShadowColor->GetWidth(), 1.0f + 0.5f/pShadowColor->GetWidth(), 0.5f/pShadowColor->GetHeight(), 1.0f + 0.5f/pShadowColor->GetHeight());
   gpRenderBackend->ResolveRenderTarget(pShadowColor2);

   //Now on with full screen projection
   gpRenderBackend->SetRenderTarget(ProjectionShader::gOriginalRenderTarget);

   gpRenderBackend->SetTexture(0, pShadowColor2);
   BP_GS_SetAlpha( SCE_GS_SET_ALPHA(0,1,0,1,0) ); // from Shadow_Texture

   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Border, CRenderBackend::kWM_Border);
   gpRenderBackend->SetDepthCompareEnabled(true);
   gpRenderBackend->SetDepthWriteEnabled(true);
   
   real32 temp[16];
   BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->eye_pers, temp);
   gpRenderBackend->SetVertexRegisters(ProjectionShader::kReg_Persp, 4, (CVector4 const*)temp);

   BP_GS_SetTest(SCE_GS_SET_TEST( 1, 2, 0, 1, 0, 0, 1, 2 ));
   BP_GS_SetupAlphaTestSinglePass(false);

#endif
}

//----------------------------------------------------------------------------

void BP_Projector_ShadowStage_EndPacket()
{
   gpRenderBackend->SetRenderTarget(ProjectionShader::gOriginalRenderTarget);
   BP_GS_SetTest(SCE_GS_SET_TEST( 1, 5, 64, 1, 0, 0, 1, 2 ));
   BP_GS_SetAlphaTestEnabled( true );
//   gpRenderBackend->SetAlphaTestEnable(true);
}

//Spot stage
//----------------------------------------------------------------------------

void BP_Projector_SpotStage_StartPacket()
{
   ProjectionShader::gOriginalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();
   BP_GS_SetAlphaTestEnabled( false );
//   gpRenderBackend->SetAlphaTestEnable(false);
}

//----------------------------------------------------------------------------

void BP_Projector_SpotStage_InitPacket(char* pData)
{
   SBP_Projector_SpotStage_InitPacket* pPacket = (SBP_Projector_SpotStage_InitPacket*)pData;

   CBaseTexture* pShadowColor = BP_GetRenderTarget(kRT_TempShadow);
   CBaseTexture* pShadowDepth = BP_GetRenderTarget(kRT_TempShadow_Depth);

   static int showInView = 0;
   if( showInView == 0 )
      gpRenderBackend->SetRenderTarget(SRenderTarget(pShadowColor, NULL, NULL, NULL, pShadowDepth));

   gpRenderBackend->Clear(CRenderBackend::kFlag_Color|CRenderBackend::kFlag_Depth, CColor::Zero(), 0.0f);

   real32 temp[16];
   BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->eye_pers, temp);
   gpRenderBackend->SetVertexRegisters(ProjectionShader::kReg_Persp, 4, (CVector4 const*)temp);

   BP_GS_SetTest(SCE_GS_SET_TEST( 0, 5, 64, 1, 0, 0, 1, 2 ));
   BP_GS_SetupAlphaTestSinglePass(false);

}

//----------------------------------------------------------------------------

void BP_Projector_SpotStage_PostZScreenPacket(char* pData)
{
   SBP_Projector_SpotStage_PostZScreenPacket* pPacket = (SBP_Projector_SpotStage_PostZScreenPacket*)pData;
   DG_TEX_BP* pTex = pPacket->tex;

   if( !pTex )
   {
      BP_DrawTriFanSpot(pPacket->mgsColor);
   }
   else
   {
      CBaseTexture const * pTexture = (CBaseTexture *)pTex->BP_TextureHandle;

      if( pTexture )
      {
         gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);
         BP_GS_SetAlpha( SCE_GS_SET_ALPHA(0,0,0,0,0) );
         gpRenderBackend->SetTexture(0, pTexture);
         gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Nearest, CRenderBackend::kFM_Nearest);
#if !BPE_PIXELCENTER_MATCH_TEXELCENTER
         CVector4 const uvOffset(0.5f / pTexture->GetWidth(), 0.5f / pTexture->GetHeight(), 0, 0);
         gpRenderBackend->SetVertexRegisters(kSReg_Sampler0Info, 1, &uvOffset);
#endif
         gpRenderBackend->SetDepthCompareEnabled(false);
         gpRenderBackend->SetDepthWriteEnabled(false);
         BP_BeginShader(ProjectionShader::gpShader[ProjectionShader::M_COPY_TEXTURE_KEEP_DEPTH], NULL, 0);

         gpRenderBackend->RenderQuad(-1.0f, 1.0f, 
            -1.0f, 1.0f, 
            1.0f,
            0.0f, 1.0f,
            0.0f, 1.0f); 

         //restore state
         gpRenderBackend->SetDepthCompareEnabled(true);
         gpRenderBackend->SetDepthWriteEnabled(true);
      }
   }
}

//----------------------------------------------------------------------------

void BP_Projector_SpotStage_ShadowProjStart(char* pData)
{
#if BP_VITA
   SBP_Projector_SpotStage_ShadowProjStart* pPacket = (SBP_Projector_SpotStage_ShadowProjStart*)pData;

   CBaseTexture* pShadowColor = BP_GetRenderTarget(kRT_TempShadow);
   CBaseTexture* pShadowDepth = BP_GetRenderTarget(kRT_TempShadow_Depth);
   // no blur
   
   gpRenderBackend->SetRenderTarget(ProjectionShader::gOriginalRenderTarget);
   
   // set texture
   gpRenderBackend->SetTexture(0, pShadowColor);

#if !BPE_PIXELCENTER_MATCH_TEXELCENTER
   CVector4 const uvOffset(0.5f / 256.0f, 0.5f / 256.0f, 0, 0);
   gpRenderBackend->SetVertexRegisters(kSReg_Sampler0Info, 1, &uvOffset);
#endif

   BP_GS_SetAlpha( SCE_GS_SET_ALPHA(0,2,0,1,0) ); // from Spot_Texture
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   gpRenderBackend->SetDepthCompareEnabled(true);
   gpRenderBackend->SetDepthWriteEnabled(true);

   real32 temp[16];
   BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->eye_pers, temp);
   gpRenderBackend->SetVertexRegisters(ProjectionShader::kReg_Persp, 4, (CVector4 const*)temp);

   BP_GS_SetTest(SCE_GS_SET_TEST( 1, 2, 0, 1, 0, 0, 1, 2 ));
   BP_GS_SetupAlphaTestSinglePass(false);
#else

   SBP_Projector_SpotStage_ShadowProjStart* pPacket = (SBP_Projector_SpotStage_ShadowProjStart*)pData;

   CBaseTexture* pShadowColor = BP_GetRenderTarget(kRT_TempShadow);
   CBaseTexture* pShadowColor2 = BP_GetRenderTarget(kRT_TempShadow2);
   CBaseTexture* pShadowDepth = BP_GetRenderTarget(kRT_TempShadow_Depth);
   gpRenderBackend->ResolveRenderTarget(pShadowColor);

   //now we get to make the texture "soft"
   BP_GS_SetAlpha( SCE_GS_SET_ALPHA(0,0,0,0,0) );
   gpRenderBackend->SetTexture(0, pShadowColor);
   gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
#if !BPE_PIXELCENTER_MATCH_TEXELCENTER
   CVector4 const uvOffset(0.5f / pShadowColor->GetWidth(), 0.5f / pShadowColor->GetHeight(), 0, 0);
   gpRenderBackend->SetVertexRegisters(kSReg_Sampler0Info, 1, &uvOffset);
#endif
   SRenderTarget softShadowRenderTarget(pShadowColor2, NULL, NULL, NULL, pShadowDepth);
   softShadowRenderTarget.mScissorEnabled = true;
   softShadowRenderTarget.mScissorX = 0;
   softShadowRenderTarget.mScissorY = 0;
   softShadowRenderTarget.mScissorWidth = pShadowColor2->GetWidth()-1;
   softShadowRenderTarget.mScissorHeight = pShadowColor2->GetHeight()-1;
   gpRenderBackend->SetRenderTarget(softShadowRenderTarget);
   gpRenderBackend->SetDepthCompareEnabled(false);
   gpRenderBackend->SetDepthWriteEnabled(false);
   BP_BeginShader(ProjectionShader::gpShader[ProjectionShader::M_BLUR_TEXTURE], NULL, 0);
   //gpRenderBackend->RenderFullscreenQuad(1.0f);
   gpRenderBackend->SetAlphaTestEnable(false);
   gpRenderBackend->RenderQuad(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.5f/pShadowColor->GetWidth(), 1.0f + 0.5f/pShadowColor->GetWidth(), 0.5f/pShadowColor->GetHeight(), 1.0f + 0.5f/pShadowColor->GetHeight());
   gpRenderBackend->ResolveRenderTarget(pShadowColor2);

   //Now on with full screen projection
   gpRenderBackend->SetRenderTarget(ProjectionShader::gOriginalRenderTarget);

   gpRenderBackend->SetTexture(0, pShadowColor2);
   BP_GS_SetAlpha( SCE_GS_SET_ALPHA(0,2,0,1,0) ); // from Spot_Texture
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   gpRenderBackend->SetDepthCompareEnabled(true);
   gpRenderBackend->SetDepthWriteEnabled(true);

   real32 temp[16];
   BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->eye_pers, temp);
   gpRenderBackend->SetVertexRegisters(ProjectionShader::kReg_Persp, 4, (CVector4 const*)temp);

   BP_GS_SetTest(SCE_GS_SET_TEST( 1, 2, 0, 1, 0, 0, 1, 2 ));
   BP_GS_SetupAlphaTestSinglePass(false);

#endif
}

//----------------------------------------------------------------------------

void BP_Projector_SpotStage_EndPacket()
{
   gpRenderBackend->SetRenderTarget(ProjectionShader::gOriginalRenderTarget);
   BP_GS_SetTest(SCE_GS_SET_TEST( 1, 5, 64, 1, 0, 0, 1, 2 ));
   BP_GS_SetAlphaTestEnabled( true );
//   gpRenderBackend->SetAlphaTestEnable(true);
}

#if BP_VITA
void BP_ProjectorNew_StartPacket(char* pData)
{
   SBP_ProjectorNew_StartPacket *pPacket = (SBP_ProjectorNew_StartPacket*)pData;

   // link shadow lists into main list
   if (pPacket->tail)
   {
      pPacket->tail->mpNext = pPacket->me->mpNext;
      pPacket->me->mpNext = pPacket->head;
   }

   // Set initial render target
   if (pPacket->nspots)
   {
      int w, h;

      if (pPacket->nspots > 8)
         w = 8;
      h = (pPacket->nspots / 8) + 1;

      SRenderTarget rt = SRenderTarget(ProjectionShader::pShadowTarget, NULL, NULL, NULL, ProjectionShader::pShadowDepth);
      rt.mRenderTargetViewPort.mViewportEnabled = true;
      rt.mRenderTargetViewPort.mViewportWidth = 128 * w;
      rt.mRenderTargetViewPort.mViewportHeight = 128 * h;
      gpRenderBackend->SetRenderTarget(rt);
   }
}

void BP_ProjectorNew_InitPacket(char* pData)
{
   SBP_ProjectorNew_InitPacket* pPacket = (SBP_ProjectorNew_InitPacket*)pData;

   // set render target
   SRenderTarget rt = SRenderTarget(ProjectionShader::pShadowTarget, NULL, NULL, NULL, ProjectionShader::pShadowDepth);
   rt.mRenderTargetViewPort.mViewportEnabled = true;
   rt.mRenderTargetViewPort.mViewportWidth = 128;
   rt.mRenderTargetViewPort.mViewportHeight = 128;

   int sx, sy;
   sx = pPacket->shadow & 7;
   sy = pPacket->shadow >> 3;
   rt.mRenderTargetViewPort.mViewportX = sx * 128;
   rt.mRenderTargetViewPort.mViewportY = sy * 128;

   // set viewport, region clip, upload perspective matrix
   gpRenderBackend->SetRenderTarget(rt);

   gpRenderBackend->Clear(CRenderBackend::kFlag_Color|CRenderBackend::kFlag_Depth, CColor::Zero(), 0.0f);

   real32 temp[16];
   BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->eye_pers, temp);
   gpRenderBackend->SetVertexRegisters(ProjectionShader::kReg_Persp, 4, (CVector4 const*)temp);

   BP_GS_SetTest(SCE_GS_SET_TEST( 0, 5, 64, 1, 0, 0, 1, 2 ));
   BP_GS_SetupAlphaTestSinglePass(false);

   BP_GS_SetAlpha( SCE_GS_SET_ALPHA(0,0,0,0,0) ); // ps2 disables ABE in the prim reg
}

void BP_ProjectorNew_ShadowProjStart(char* pData)
{
   SBP_ProjectorNew_ShadowProjStart* pPacket = (SBP_ProjectorNew_ShadowProjStart*)pData;

   //gpRenderBackend->SetRenderTarget(ProjectionShader::gOriginalRenderTarget);

   // set texture
   gpRenderBackend->SetTexture(0, ProjectionShader::pShadowTextures[pPacket->shadow]);

   BP_GS_SetAlpha( SCE_GS_SET_ALPHA(0,1,0,1,0) ); // from Shadow_Texture

#if !BPE_PIXELCENTER_MATCH_TEXELCENTER
   CVector4 const uvOffset(0.5f / 256.0f, 0.5f / 256.0f, 0, 0);
   gpRenderBackend->SetVertexRegisters(kSReg_Sampler0Info, 1, &uvOffset);
#endif

   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   gpRenderBackend->SetDepthCompareEnabled(true);
   gpRenderBackend->SetDepthWriteEnabled(true);
   
   real32 temp[16];
   BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->eye_pers, temp);
   gpRenderBackend->SetVertexRegisters(ProjectionShader::kReg_Persp, 4, (CVector4 const*)temp);

   BP_GS_SetTest(SCE_GS_SET_TEST( 1, 2, 0, 1, 0, 0, 1, 2 ));
   BP_GS_SetupAlphaTestSinglePass(false);
}

void BP_ProjectorNew_SpotProjStart(char* pData)
{
   SBP_ProjectorNew_ShadowProjStart* pPacket = (SBP_ProjectorNew_ShadowProjStart*)pData;

   //gpRenderBackend->SetRenderTarget(ProjectionShader::gOriginalRenderTarget);

   // set texture
   gpRenderBackend->SetTexture(0, ProjectionShader::pShadowTextures[pPacket->shadow]);

   BP_GS_SetAlpha( SCE_GS_SET_ALPHA(0,2,0,1,0) ); // from Shadow_Texture

#if !BPE_PIXELCENTER_MATCH_TEXELCENTER
   CVector4 const uvOffset(0.5f / 256.0f, 0.5f / 256.0f, 0, 0);
   gpRenderBackend->SetVertexRegisters(kSReg_Sampler0Info, 1, &uvOffset);
#endif

   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   gpRenderBackend->SetDepthCompareEnabled(true);
   gpRenderBackend->SetDepthWriteEnabled(true);
   
   real32 temp[16];
   BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->eye_pers, temp);
   gpRenderBackend->SetVertexRegisters(ProjectionShader::kReg_Persp, 4, (CVector4 const*)temp);

   BP_GS_SetTest(SCE_GS_SET_TEST( 1, 2, 0, 1, 0, 0, 1, 2 ));
   BP_GS_SetupAlphaTestSinglePass(false);
}

void BP_ProjectorNew_EndPacket()
{
   //gpRenderBackend->SetRenderTarget(ProjectionShader::gOriginalRenderTarget);
   BP_GS_SetTest(SCE_GS_SET_TEST( 1, 5, 64, 1, 0, 0, 1, 2 ));
   BP_GS_SetAlphaTestEnabled( true );
//   gpRenderBackend->SetAlphaTestEnable(true);
}
#endif

//Draw functions
//----------------------------------------------------------------------------

namespace NOProjectorLocalParam
{
   CMesh *pLastMeshRendered = NULL;

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
      meshBuffers.SetVertexData(binding);

      // Set Index Buffer
      gpRenderBackend->SetIndexData(meshBuffers.GetIndexBuffer());

      // Don't do it again
      pLastMeshRendered = pMesh;
   }
}

void BP_Projector_LocalParam(char* pData)
{
   SBP_OBJ_Projector_LocalParam* pPacket = (SBP_OBJ_Projector_LocalParam*)pData;
  
   BP_BeginShader(ProjectionShader::gpShader[ProjectionShader::M_DRAW_MODEL_BUILD_PROJECTION + pPacket->shaderType], NULL, 0);
   NOProjectorLocalParam::pLastMeshRendered = NULL;
}

void BP_Projector_Render(char* pData)
{
   static CVertexData preshadeVertexData;

   SBP_OBJ_Projector_Render* pPacket = (SBP_OBJ_Projector_Render*)pData;

   real32 temp[16];

   BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->world, temp);
   gpRenderBackend->SetVertexRegisters(ProjectionShader::kReg_Matrix0, 4, (CVector4 const*)temp);

   BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->correction, temp);
   gpRenderBackend->SetVertexRegisters(ProjectionShader::kReg_Correction, 4, (CVector4 const*)temp);

   BP_KPPersMatrix_to_VS_Matrix44((real32 const*)&pPacket->lightDir, temp);
   gpRenderBackend->SetVertexRegisters(kSReg_LightDir, 4, (CVector4 const*)temp);

   //BP_Matrix44_to_VS_Matrix44((real32 const*)&pPacket->lightCol, temp);
   gpRenderBackend->SetVertexRegisters(kSReg_LightCol, 4, (CVector4 const*)&pPacket->lightCol);

   CMesh* pMesh = (CMesh*)pPacket->model;
   if( pMesh )
   {
      NOProjectorLocalParam::FlushObjLocalParamForMesh( pMesh );

      if( pPacket->flag & (DG_PACKFLAG_CULLAUTO|DG_PACKFLAG_CULLON) )
         gpRenderBackend->SetCullMode(CRenderBackend::kCM_CCW);
      else 
         gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

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

//----------------------------------------------------------------------------

extern "C" int gBP_ShadowStageEnabled;
extern "C" int gBP_SpotStageEnabled;

void BP_Projector_InitDebugMenu()
{
#if BP_ENABLE_DEBUG_MENU
   int const renderMenu = BP_DebugMenu_GetMenu("Render");

   int const debugMenu = BP_DebugMenu_AddMenu("Projector", renderMenu);
 
   BP_DebugMenu_AddBool(debugMenu, "Enable Shadow Stage", &gBP_ShadowStageEnabled);
   BP_DebugMenu_AddBool(debugMenu, "Enable Spot Stage", &gBP_SpotStageEnabled);
#endif
}
