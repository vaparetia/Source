//----------------------------------------------------------------------------
// BP_RenderFX.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"

#include "MGS_Common.h"

#include "Engine/system/CSPURSManager.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Primitive/CVertexData.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"
#include "Renderer/Base/Frontend/RenderTypes.h"

#include "BP_RenderFX.h"
#include "BP_RendererDebug.h"
#include "BP_RenderBufferTypes.h"
#include "BP_RenderGS.h"
#include "BP_Memory.h"

#include "Engine/Math/CRandom.h"

#if BP_PS3

#include "Renderer/Base/Backend/PS3/PS3CTexture.h"

#include "Edge/post/edgepost_ppu.h"
#include "Edge/post/edgepost_mlaa_handler_ppu.h"

using namespace cell::Gcm;

#endif

#if BP_VITA
//#  define BP_POSTFX_DISABLE 1

#include "Renderer/Base/Backend/VTA/VTACTexture.h"

#endif

//----------------------------------------------------------------------------

//#include "BP_BaseRenderer.h"

//----------------------------------------------------------------------------

#include "BP_Renderer.h"

//----------------------------------------------------------------------------

namespace FX
{
   enum EShaderType
   {
      kST_TextureRGBA,
      kST_TextureRGB,
      kST_TextureRGBAModulate,
      kST_TextureRGBModulate,
      kST_TextureRGBModulate2x,
      kST_TextureRGBModulate3x,
      kST_TextureRGBModulate4x,
      kST_TextureRGBModulate5x,
      kST_TextureRGBModulate6x,
      kST_Downsample2x,
      kST_DownsampleDepth,
      kST_Blur,
      kST_MonoMini,
      kST_Contrast,
      kST_Blinds,
      kST_GrayScale,
      kST_YUVConversion,
      kST_MSAA_4X_ResolveColorAndDepth,
      kST_MSAA_2X_ResolveColorAndDepth,
      kST_ZCullReload,

      kST_GlareMono,
      kST_GlareBlur,
      kST_GrayScaleTex,
      kST_TextureRGB_HalfresComposite,

      kST_GlareMono_ALU,

      kST_Count
   };

   enum ERegisters
   {
      kReg_UVOffset0    =  96,
      kReg_UVOffset1    =  97,
      kReg_UVOffset2    =  98,
      kReg_UVOffset3    =  99,
      kReg_Scale        =  100,
      kReg_PixelOffset  =  101,
   };

   uint32 gModulateColor;
   uint32 gMonochromeWeights;
   uint32 gContrastParam;
   uint32 gTweakParam;

   CCompiledShader* gpShader[kST_Count];

   int const kMaxMonoPaletteTextureCount = 10;
   int gCurrentMonoPaletteTexture = 0;
   CBaseTexture* pMonoPaletteTextures[kMaxMonoPaletteTextureCount];

#if BP_FX_NEEDS_NOISE_TEXTURE
   CBaseTexture* gpRandomNoiseTexture;
   uint8* gpRandomNoiseValues512x512;
#endif

#if BP_VITA
   struct SUberTweakKnob
   {
      SUberTweakKnob()
         : mBrightnessScale( 1.0f )
         , mBrightnessAdd( 0.0f )
         , mWhoKnows( 0.f )
         , mWhoCares( 0.f )
      {
      }

      float mBrightnessScale;
      float mBrightnessAdd;
      float mWhoKnows;
      float mWhoCares;
   };

   // Uber tweak knobs are a CVector4
   BPE_CTASSERT( sizeof( SUberTweakKnob ) == sizeof( CVector4 ) );

   int const kMaxBlurTextures = 16;
   CBaseTexture* pBlurTarget;
   CBaseTexture* pBlurHalfTarget;
   CBaseTexture* pBlurTextures[kMaxBlurTextures];
   CBaseTexture* pOffscreenBuffer;
   
   SRenderTarget gMainRenderTarget;

   // Halfres rendering targets
   CBaseTexture *pMSAADownresTarget;
   CBaseTexture *pMSAADownresDepth;

   // DoF/Glare/Uberscale machinery
   CBaseTexture *pGlareMonoPalette;
   int gDOFs = 0; // 0 off, -1 scope, 1..2 farfocus
   int gGlarePlanes = 0;
   float gDOF_Z[4];
   float gDOF_Alpha[4];
   float gGlareAlpha;
   CBaseTexture *pHalfresTexture;
   CBaseTexture *pHalfresDepth;
   int gHalfresEnabled = 0;
   int gUberContrast = 0;

   uint32 gMonoScale0;
   uint32 gMonoScale1;
   uint32 gMonoOfs0;
   uint32 gMonoOfs1;

   int gGlareMonoType;
   float gGlareMonoScale0[4];
   float gGlareMonoScale1[4];
   float gGlareMonoOfs0[4];
   float gGlareMonoOfs1[4];
   float gUberContrastParam[4];
   SUberTweakKnob gUberTweakKnobs;

   enum EUberDOF
   {
      kDOF_NONE,
      kDOF1,
      kDOF2,
      kDOFSCOPE,
      kDOF_Count
   };
   
   enum EUberGlare
   {
      kGLARE_NONE,
      kGLARE_1,
      kGLARE_2,
      kGLARE_3,
      kGLARE_4,
      kGLARE_5,
      kGLARE_6,
      kGLARE_Count,
   };

   enum EUberHalfres
   {
      kHR_OFF,
      kHR_ON,
      kHR_Count
   };

   enum EUberRegisters
   {
      kReg_TexelOffset  =  102,
      kReg_QuadScaleOffset  =  103
   }; 
   CCompiledShader* gpUberShader[kHR_Count][kDOF_Count][kGLARE_Count][3][2];

   uint32 gPS_DOF_Z;
   uint32 gPS_DOF_Alpha;
   uint32 gPS_GlareAlpha;
#endif
}

#if BP_VITA
extern int gVTADebugUberscale;
extern int gVTADebugUberscaleForce;
extern int gVTADebugUberscaleShader;
extern int gVTADebugUberscaleQuad;
void UberscaleVTA();
int gVTAUseALUMono;
extern "C" int gVTAHalfresAlways;
int gVTAHalfresSlopeBias = 1;
int gVTAHalfresDepthBias = 0;
#endif

int gFXShowPasses = false;

inline void ComputeQuadUVs(SRect const & rect, int totalWidth, int totalHeight, float & u0, float & u1, float & v0, float & v1)
{
   u0 = float(rect.x1) / totalWidth;
   u1 = float(rect.x2) / totalWidth;
   v0 = float(rect.y1) / totalHeight;
   v1 = float(rect.y2) / totalHeight;
}

inline void ComputeWidthHeightQuadUVs(SRect const & rect, int totalWidth, int totalHeight, float & u0, float & u1, float & v0, float & v1, int & width, int & height)
{
   width = rect.x2 - rect.x1;
   height = rect.y2 - rect.y1;

   ComputeQuadUVs(rect, totalWidth, totalHeight, u0, u1, v0, v1);
}

void BP_InitFXShader()
{
   CShaderFileId shaderId("$/EngineSupport/Shaders/FX.fx");
   
   for( int i = 0; i < FX::kST_Count; ++i )
   {
      CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(shaderId, CStringExtras::Stringize_s("TYPE=%d", i));
      assert(pShader);

      FX::gpShader[i] = pShader;
   }

   FX::gModulateColor      = CShaderCRCs::GetParameterCRC("gPS_ModulateColor");
   FX::gMonochromeWeights  =  CShaderCRCs::GetParameterCRC("gPS_MonochromeWeights");
   FX::gContrastParam      = CShaderCRCs::GetParameterCRC("gPS_ContrastParam");
   FX::gTweakParam      = CShaderCRCs::GetParameterCRC("gPS_TweakKnobs");

#if BP_VITA
#  if MGS_VERSION==2
   // AS(JM) - Brightness values for MGS2 provided by KP
   FX::gUberTweakKnobs.mBrightnessScale = 1.20f;
   FX::gUberTweakKnobs.mBrightnessAdd = 0.03f;
#  endif
#endif

   for( int i = 0; i < FX::kMaxMonoPaletteTextureCount; ++i )
   {
      FX::pMonoPaletteTextures[i] = CBaseTexture::Create(256, 1, 1, CBaseTexture::kFormat_A8R8G8B8, CBaseTexture::kUsage_TextureLinear, CBaseTexture::kAA_None, kRM_System);
   }

#if BP_FX_NEEDS_NOISE_TEXTURE
   {
      int const kRandomNoiseTextureSeed = 56;
      CRandom randomNumber(kRandomNoiseTextureSeed);
      FX::gpRandomNoiseValues512x512 = (uint8*) BP_Memory_Alloc( 512 * 512, 4, kMT_Permanent, kMC_Renderer );
      for( int y = 0; y < 512; ++y )
      {
         for( int x = 0; x < 512; ++x )
         {
            FX::gpRandomNoiseValues512x512[x+y*512] = (uint8)randomNumber.RangeInt(0, 255);
         }
      }

      FX::gpRandomNoiseTexture = CBaseTexture::Create(256, 256, 1, CBaseTexture::kFormat_Luminance8, CBaseTexture::kUsage_TextureLinear, CBaseTexture::kAA_None);
      uint8* pTextureMem = NULL;
      int texturePitch = 0;
      FX::gpRandomNoiseTexture->Lock((void**)&pTextureMem, &texturePitch);
      for( int y = 0; y < 256; ++y )
      {
         for( int x = 0; x < 256; ++x )
         {
            pTextureMem[x+y*texturePitch] = FX::gpRandomNoiseValues512x512[x+y*256];
         }
      }
      FX::gpRandomNoiseTexture->Unlock();
   }
#endif

#if BP_VITA
   CShaderFileId uberscaleId("$/EngineSupport/Shaders/Uberscale.fx");

   FX::gMonoScale0      = CShaderCRCs::GetParameterCRC("gPS_MonoScale0");
   FX::gMonoScale1      = CShaderCRCs::GetParameterCRC("gPS_MonoScale1");
   FX::gMonoOfs0        = CShaderCRCs::GetParameterCRC("gPS_MonoOfs0");
   FX::gMonoOfs1        = CShaderCRCs::GetParameterCRC("gPS_MonoOfs1");

   // Blur textures
   // 1 1024x512 texture to use as a color buffer
   // 4 480x256 textures aliasing on the quadrants of the color buffer
   SCreateTextureParams blurParams;
   blurParams.mBaseAddress = NULL;
   blurParams.mNumScenes = 1;
   blurParams.mXTiles = 4;
   blurParams.mYTiles = 4;
   FX::pBlurTarget = CBaseTexture::Create(1024, 512, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_RenderTarget, CBaseTexture::kAA_None, kRM_Video, &blurParams);

   // UL
   FX::pBlurTarget->Lock((void**)&blurParams.mBaseAddress, &blurParams.mStride);
   blurParams.mXTiles = 0;
   blurParams.mYTiles = 0;
   FX::pBlurHalfTarget = CBaseTexture::Create(360, 224, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_RenderTarget, CBaseTexture::kAA_None, kRM_Video, &blurParams);

   unsigned int base = blurParams.mBaseAddress;
   FX::pBlurTextures[0] = CBaseTexture::Create(180, 112, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_Texture, CBaseTexture::kAA_None, kRM_Video, &blurParams);
   // LL
   blurParams.mBaseAddress += 128 * blurParams.mStride;
   FX::pBlurTextures[1] = CBaseTexture::Create(180, 112, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_Texture, CBaseTexture::kAA_None, kRM_Video, &blurParams);
   // LR
   blurParams.mBaseAddress += 128 * blurParams.mStride;
   FX::pBlurTextures[2] = CBaseTexture::Create(180, 112, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_Texture, CBaseTexture::kAA_None, kRM_Video, &blurParams);
   // UR
   blurParams.mBaseAddress += 128 * blurParams.mStride;
   FX::pBlurTextures[3] = CBaseTexture::Create(180, 112, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_Texture, CBaseTexture::kAA_None, kRM_Video, &blurParams);

   blurParams.mBaseAddress = base + 256*4;
   FX::pBlurTextures[4] = CBaseTexture::Create(180, 112, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_Texture, CBaseTexture::kAA_None, kRM_Video, &blurParams);
   // LL
   blurParams.mBaseAddress += 128 * blurParams.mStride;
   FX::pBlurTextures[5] = CBaseTexture::Create(180, 112, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_Texture, CBaseTexture::kAA_None, kRM_Video, &blurParams);
   // LR
   blurParams.mBaseAddress += 128 * blurParams.mStride;
   FX::pBlurTextures[6] = CBaseTexture::Create(180, 112, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_Texture, CBaseTexture::kAA_None, kRM_Video, &blurParams);
   // UR
   blurParams.mBaseAddress += 128 * blurParams.mStride;
   FX::pBlurTextures[7] = CBaseTexture::Create(180, 112, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_Texture, CBaseTexture::kAA_None, kRM_Video, &blurParams);

   blurParams.mBaseAddress = base + 512*4;
   FX::pBlurTextures[8] = CBaseTexture::Create(180, 112, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_Texture, CBaseTexture::kAA_None, kRM_Video, &blurParams);
   // LL
   blurParams.mBaseAddress += 128 * blurParams.mStride;
   FX::pBlurTextures[9] = CBaseTexture::Create(180, 112, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_Texture, CBaseTexture::kAA_None, kRM_Video, &blurParams);
   // LR
   blurParams.mBaseAddress += 128 * blurParams.mStride;
   FX::pBlurTextures[10] = CBaseTexture::Create(180, 112, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_Texture, CBaseTexture::kAA_None, kRM_Video, &blurParams);
   // UR
   blurParams.mBaseAddress += 128 * blurParams.mStride;
   FX::pBlurTextures[11] = CBaseTexture::Create(180, 112, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_Texture, CBaseTexture::kAA_None, kRM_Video, &blurParams);

   blurParams.mBaseAddress = base + 768*4;
   FX::pBlurTextures[12] = CBaseTexture::Create(180, 112, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_Texture, CBaseTexture::kAA_None, kRM_Video, &blurParams);
   // LL
   blurParams.mBaseAddress += 128 * blurParams.mStride;
   FX::pBlurTextures[13] = CBaseTexture::Create(180, 112, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_Texture, CBaseTexture::kAA_None, kRM_Video, &blurParams);
   // LR
   blurParams.mBaseAddress += 128 * blurParams.mStride;
   FX::pBlurTextures[14] = CBaseTexture::Create(180, 112, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_Texture, CBaseTexture::kAA_None, kRM_Video, &blurParams);
   // UR
   blurParams.mBaseAddress += 128 * blurParams.mStride;
   FX::pBlurTextures[15] = CBaseTexture::Create(180, 112, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_Texture, CBaseTexture::kAA_None, kRM_Video, &blurParams);


   for (int i = 0; i < FX::kHR_Count; ++i)
      for (int j = 0; j < FX::kDOF_Count; ++j)
         for (int k = 0; k < FX::kGLARE_Count; ++k)
            for (int l = 0; l < 3; l++)
               for (int m = 0; m < 2; m++)
         {
            CCompiledShader* pShader = RenderBackend()->ShaderCache()->GetShader(uberscaleId, CStringExtras::Stringize_s("HALF=%d;DOF=%d;GLARE=%d;ALU=%d;DEPTHWRITE=%d", i, j, k, l, m));
            assert(pShader);
            
            FX::gpUberShader[i][j][k][l][m] = pShader;
         }
   
   FX::gPS_DOF_Z        = CShaderCRCs::GetParameterCRC("DOF_Z");
   FX::gPS_DOF_Alpha    = CShaderCRCs::GetParameterCRC("DOF_Alpha");
   FX::gPS_GlareAlpha   = CShaderCRCs::GetParameterCRC("GlareAlpha");

   SCreateTextureParams offscreenParams;
   offscreenParams.mBaseAddress = NULL;
   // alias on top of the blur target, don't use its stride though
   FX::pBlurTarget->Lock((void**)&offscreenParams.mBaseAddress, &offscreenParams.mStride);
   offscreenParams.mStride = 0;
   FX::pOffscreenBuffer = CBaseTexture::Create(960, 544, 1, CBaseTexture::kFormat_A8B8G8R8, CBaseTexture::kUsage_Texture, CBaseTexture::kAA_None, kRM_Video, &offscreenParams);
 
   gpRenderBackend->SetUpscaleOverride(UberscaleVTA);

   {
      SCreateTextureParams downresParams;
      downresParams.mBaseAddress = NULL;
      downresParams.mMSAATrick = 1;
      FX::pMSAADownresTarget = CBaseTexture::Create(360, 224, 1, CBaseTexture::kFormat_A16B16G16R16F, CBaseTexture::kUsage_RenderTarget, CBaseTexture::kAA_None, kRM_Video, &downresParams);
      
      // Alias depth buffer
      RenderBackend()->GetLowresDrawDepth()->Lock((void**)&downresParams.mBaseAddress, &downresParams.mStride);
      downresParams.mLoadStore = CBaseTexture::kLS_Load;
      FX::pMSAADownresDepth = CBaseTexture::Create(360, 224, 1, CBaseTexture::kFormat_D24X8, CBaseTexture::kUsage_DepthBuffer, CBaseTexture::kAA_None, kRM_Video, &downresParams);
   }

#if MGS_VERSION == 3
   BP_PostFx_InitShockwave();
#endif
#endif
}

void BP_SetFXShader(FX::EShaderType type, uint8 * pParameterBuffer = NULL, int const parameterBufferSize = 0)
{
   BP_BeginShader(FX::gpShader[type], pParameterBuffer, parameterBufferSize);
}

#if BP_FX_NEEDS_NOISE_TEXTURE

extern "C" unsigned int GetRandomNoiseTexture()
{
   return (int)FX::gpRandomNoiseTexture;
}

extern "C" unsigned char* GetRandomNoiseValues512x512()
{
   return (unsigned char*)FX::gpRandomNoiseValues512x512;
}

#endif

//----------------------------------------------------------------------------

void BP_YUVConversion(CBaseTexture* pY, CBaseTexture* pU, CBaseTexture* pV, CBaseTexture* pResult)
{
   BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:BP_YUVConversion");

   SRenderTarget const originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();

   gpRenderBackend->SetRenderTarget(SRenderTarget(pResult, NULL, NULL, NULL, NULL));

   gpRenderBackend->SetDepthCompareEnabled(false);
   gpRenderBackend->SetDepthWriteEnabled(false);

   gpRenderBackend->SetTexture(0, pY);
   gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

   gpRenderBackend->SetTexture(1, pU);
   gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

   gpRenderBackend->SetTexture(2, pV);
   gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

   CVector4 const uvOffset(0.5f / pResult->GetWidth(), 0.5f / pResult->GetHeight(), 0, 0);
   gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);

   BP_SetFXShader(FX::kST_YUVConversion, NULL, 0);

   gpRenderBackend->RenderFullscreenQuad(1.0f);
   gpRenderBackend->ResolveRenderTarget(pResult);

   gpRenderBackend->SetRenderTarget(originalRenderTarget);
}

//----------------------------------------------------------------------------

void BP_ResolveColorAndDepth(CBaseTexture* pColor, CBaseTexture* pDepth0, CBaseTexture* pDepth1, CBaseTexture* pDepth2, CBaseTexture* pDepth3, int doZReload, CBaseTexture::EAntiAliasType antiAliasType, SRectf const & uvRect)
{
   BPE_VERIFY(pColor->GetWidth() == pDepth0->GetWidth(), false, "BP_ResolveColorAndDepth: pColor and pDepth must be same resolution!");

   gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);

#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
   gpRenderBackend->SetAlphaTestEnable(false);
#endif
   gpRenderBackend->SetStencilEnable(false);

   gpRenderBackend->SetDepthCompareEnabled(true);
   gpRenderBackend->SetDepthWriteEnabled(true);
   gpRenderBackend->SetDepthFunc(CRenderBackend::kDF_Always);

   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   gpRenderBackend->SetTexture(0, pColor);
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

   gpRenderBackend->SetTexture(1, pColor);
   gpRenderBackend->SetTextureAddressMode(1, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

   gpRenderBackend->SetTexture(2, pDepth0);
   gpRenderBackend->SetTextureFilter(2, CRenderBackend::kFM_Nearest, CRenderBackend::kFM_Nearest);
   gpRenderBackend->SetTextureAddressMode(2, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

   if( pDepth1 )
   {
      gpRenderBackend->SetTexture(3, pDepth1);
      gpRenderBackend->SetTextureFilter(3, CRenderBackend::kFM_Nearest, CRenderBackend::kFM_Nearest);
      gpRenderBackend->SetTextureAddressMode(3, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   }
   if( pDepth2 )
   {
      gpRenderBackend->SetTexture(4, pDepth2);
      gpRenderBackend->SetTextureFilter(4, CRenderBackend::kFM_Nearest, CRenderBackend::kFM_Nearest);
      gpRenderBackend->SetTextureAddressMode(4, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

      gpRenderBackend->SetTexture(5, pDepth3);
      gpRenderBackend->SetTextureFilter(5, CRenderBackend::kFM_Nearest, CRenderBackend::kFM_Nearest);
      gpRenderBackend->SetTextureAddressMode(5, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   }

   if( antiAliasType == CBaseTexture::kAA_MSAA2x )
   {
#if BPE_TARGET == BPE_TARGET_PS3
      cellGcmSetTextureFilter(0, 0, CELL_GCM_TEXTURE_CONVOLUTION_MIN, CELL_GCM_TEXTURE_CONVOLUTION_MAG, CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX);
      cellGcmSetTextureFilter(1, 0, CELL_GCM_TEXTURE_CONVOLUTION_MIN, CELL_GCM_TEXTURE_CONVOLUTION_MAG, CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX_ALT);
      // Set Color uv offset
      {
         CVector4 const uvOffset(0.0001f + 1.0f / pColor->GetWidth(),  0.5f / pColor->GetHeight(), 
            -0.0001f + 1.0f / pColor->GetWidth(), -0.5f / pColor->GetHeight());

         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);
      }
      // Set depth uv offset
      {
         CVector4 const uvOffset(0.5f / pDepth0->GetWidth(), 0.5f / pDepth0->GetHeight(), 
            1.5f / pDepth0->GetWidth(), 0.5f / pDepth0->GetHeight());
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffset);
      }
#else
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Nearest, CRenderBackend::kFM_Nearest);
      gpRenderBackend->SetTextureFilter(1, CRenderBackend::kFM_Nearest, CRenderBackend::kFM_Nearest);
      // Set Color uv offset
      {
         CVector4 const uvOffset(0.5f / pColor->GetWidth(), 0.5f / pColor->GetHeight(), 0, 0);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);
      }
      // Set depth uv offset
      {
         CVector4 const uvOffset(0.5f / pDepth0->GetWidth(), 0.5f / pDepth0->GetHeight(), 
            0.5f / pDepth0->GetWidth(), 0.5f / pDepth0->GetHeight());
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffset);
      }
#endif
      

      BP_SetFXShader(FX::kST_MSAA_2X_ResolveColorAndDepth, NULL, 0);
   }
   else
   {
      // Set depth uv offset
      {
         CVector4 const uvOffset(0.5f / pDepth0->GetWidth(), 0.5f / pDepth0->GetHeight(), 0, 0);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffset);
      }

#if BPE_TARGET == BPE_TARGET_PS3
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);

      // Set Color uv offset
      {
         CVector4 const uvOffset(1.0f / pColor->GetWidth(), 1.0f / pColor->GetHeight(), 0, 0);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);
      }

#else
      // Set Color uv offset
      {
         CVector4 const uvOffset(0.5f / pColor->GetWidth(), 0.5f / pColor->GetHeight(), 0, 0);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);
      }

      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Nearest, CRenderBackend::kFM_Nearest);
#endif

      BP_SetFXShader(FX::kST_MSAA_4X_ResolveColorAndDepth, NULL, 0);
   }

   gpRenderBackend->RenderQuad(
      -1.0f, 1.0f, 
      -1.0f, 1.0f, 
      1.0f,
      uvRect.x1, uvRect.x2,
      uvRect.y1, uvRect.y2);

   if(doZReload)
      BP_ZCullReload();

   gpRenderBackend->SetDepthFunc(CRenderBackend::kDF_GEqual);
}

//----------------------------------------------------------------------------

void BP_DrawFullscreenTexture(CBaseTexture const * pTexture, int isFilter, int isRGBA, real32 const z)
{
   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   CRenderBackend::EFilterMode const filterMode = isFilter ? CRenderBackend::kFM_Linear : CRenderBackend::kFM_Nearest;

   gpRenderBackend->SetTexture(0, pTexture);
   gpRenderBackend->SetTextureFilter(0, filterMode, filterMode);
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

   CVector4 uvOffset(0.5f / pTexture->GetWidth(), 0.5f / pTexture->GetHeight(), 0, 0);
//   uvOffset = CVector4( 0.5f, 0.5f, 0.5f, 0.5f );
   gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);

   BP_SetFXShader(isRGBA ? FX::kST_TextureRGBA : FX::kST_TextureRGB, NULL, 0);

   gpRenderBackend->RenderFullscreenQuad(z);
}

//----------------------------------------------------------------------------

void BP_DrawFullscreenTextureModulate(CBaseTexture const * pTexture, CVector4 const & modulate, int isFilter, int isRGBA, real32 const z)
{
   SRect srcRect(
      0, 0,
      pTexture->GetWidth(),
      pTexture->GetHeight()
      );
   BP_DrawFullscreenTextureModulateSRect(pTexture, modulate, isFilter, isRGBA, srcRect, z);
}

//----------------------------------------------------------------------------

void BP_DrawFullscreenTextureModulateSRect(CBaseTexture const * pTexture, CVector4 const & modulate, int isFilter, int isRGBA, SRect const & rect, real32 const z)
{
   int const sourceWidthTotal = pTexture->GetWidth();
   int const sourceHeightTotal = pTexture->GetHeight();
   float u0;
   float u1;
   float v0;
   float v1;
   ComputeQuadUVs(rect, sourceWidthTotal, sourceHeightTotal, 
      u0, u1, v0, v1);

   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   CRenderBackend::EFilterMode const filterMode = isFilter ? CRenderBackend::kFM_Linear : CRenderBackend::kFM_Nearest;

   gpRenderBackend->SetTexture(0, pTexture);
   gpRenderBackend->SetTextureFilter(0, filterMode, filterMode);
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

   CVector4 const uvOffset(0.5f / gpRenderBackend->GetViewportWidth(), 0.5f / gpRenderBackend->GetViewportHeight(), 0, 0);
   gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);

   CShaderParameterBufferAllocator::BeginRegion();

   CShaderParameterBuffer parameterBuffer = CShaderParameterBufferAllocator::AllocateParameterBuffer();
   parameterBuffer.AddParameter(FX::gModulateColor, modulate);
   CShaderParameterBufferAllocator::FinalizeParameterBuffer(parameterBuffer);

   BP_SetFXShader(isRGBA ? FX::kST_TextureRGBAModulate : FX::kST_TextureRGBModulate, parameterBuffer.GetStartAddress(), parameterBuffer.GetSize());

   gpRenderBackend->RenderQuad(-1.0f, 1.0f, -1.0f, 1.0f, z, u0, u1, v0, v1);

   CShaderParameterBufferAllocator::EndRegion();
}


//----------------------------------------------------------------------------

void BP_DrawFullscreenTextureModulateWithXTextures(const CBaseTexture** apTexture, int numTextures, CVector4 const & modulate, int isFilter, real32 const z)
{
   BPE_VERIFY(numTextures >= 1 && numTextures <= 6, false, "BP_DrawFullscreenTextureModulateWithXTextures numTextures must be >=1 && <=6" )

   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   CRenderBackend::EFilterMode const filterMode = isFilter ? CRenderBackend::kFM_Linear : CRenderBackend::kFM_Nearest;

   for( int iTexture=0; iTexture<numTextures; ++iTexture )
   {
      gpRenderBackend->SetTexture(iTexture, apTexture[iTexture]);
      gpRenderBackend->SetTextureFilter(iTexture, filterMode, filterMode);
      gpRenderBackend->SetTextureAddressMode(iTexture, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   }

   CVector4 const uvOffset(0.5f / gpRenderBackend->GetViewportWidth(), 0.5f / gpRenderBackend->GetViewportHeight(), 0, 0);
   gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);

   CShaderParameterBufferAllocator::BeginRegion();

   CShaderParameterBuffer parameterBuffer = CShaderParameterBufferAllocator::AllocateParameterBuffer();
   parameterBuffer.AddParameter(FX::gModulateColor, modulate);
   CShaderParameterBufferAllocator::FinalizeParameterBuffer(parameterBuffer);

   BP_SetFXShader(FX::EShaderType(FX::kST_TextureRGBModulate+(numTextures-1)), parameterBuffer.GetStartAddress(), parameterBuffer.GetSize());

   gpRenderBackend->RenderFullscreenQuad(z);

   CShaderParameterBufferAllocator::EndRegion();
}

//----------------------------------------------------------------------------

void BP_DrawRectTextureModulate(CBaseTexture* pTexture, real32 const x, real32 const y, real32 const width, real32 const height, CVector4 const & modulate, CVector4 const & uvRect, int isFilter, int isRGBA)
{
   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   CRenderBackend::EFilterMode const filterMode = isFilter ? CRenderBackend::kFM_Linear : CRenderBackend::kFM_Nearest;

   gpRenderBackend->SetTexture(0, pTexture);
   gpRenderBackend->SetTextureFilter(0, filterMode, filterMode);
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

   CVector4 const uvOffset(0.5f / pTexture->GetWidth(), 0.5f / pTexture->GetHeight(), 0.0f, 0.0f);
   gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);

   CShaderParameterBufferAllocator::BeginRegion();

   CShaderParameterBuffer parameterBuffer = CShaderParameterBufferAllocator::AllocateParameterBuffer();
   parameterBuffer.AddParameter(FX::gModulateColor, modulate);
   CShaderParameterBufferAllocator::FinalizeParameterBuffer(parameterBuffer);

   BP_SetFXShader(isRGBA ? FX::kST_TextureRGBAModulate : FX::kST_TextureRGBModulate, parameterBuffer.GetStartAddress(), parameterBuffer.GetSize());

   gpRenderBackend->RenderQuad(x, x + width, y, y + height, 1.0f, uvRect.mX, uvRect.mZ, uvRect.mY, uvRect.mW);

   CShaderParameterBufferAllocator::EndRegion();
}

//----------------------------------------------------------------------------

void BP_Blur(CBaseTexture* pSource, CBaseTexture *pDest, CBaseTexture* pTemp, real32 const weight0, real32 const weight12, real32 const weight34, real32 const weight56, real32 const pass1Scale, real32 const pass2Scale, bool doFinalResolve)
{
   SRect srcRect(
      0, 0,
      pSource->GetWidth(),
      pSource->GetHeight()
      );
   SRect dstRect(
      0, 0,
      pDest->GetWidth(),
      pDest->GetHeight()
      );
   BP_BlurSRect(pSource, pDest, pTemp, weight0, weight12, weight34, weight56, pass1Scale, pass2Scale, doFinalResolve, srcRect, dstRect);
}

//----------------------------------------------------------------------------

void BP_BlurSRect(CBaseTexture* pSource, CBaseTexture *pDest, CBaseTexture* pTemp, real32 const weight0, real32 const weight12, real32 const weight34, real32 const weight56, real32 const pass1Scale, real32 const pass2Scale, bool doFinalResolve, SRect const & srcRect, SRect const & dstRect)
{
   int const dstWidthTotal = pDest->GetWidth();
   int const dstHeightTotal = pDest->GetHeight();
   int destWidth;
   int destHeight;
   float u0;
   float u1;
   float v0;
   float v1;
   ComputeWidthHeightQuadUVs(dstRect, dstWidthTotal, dstHeightTotal,
      u0, u1, v0, v1, destWidth, destHeight);
   int const sourceWidth = srcRect.x2 - srcRect.x1;
   int const sourceHeight = srcRect.y2 - srcRect.y1;
   int const actualSourceWidth = srcRect.x2 - srcRect.x1;
   int const actualSourceHeight = srcRect.y2 - srcRect.y1;

   BP_SetFXShader(FX::kST_Blur, NULL, 0);

   real32 const kOffsets[] = { 0.5f, -1.0f, 2.0f, -3.0f, 4.0f, -5.0f, 6.0f };

   CVector4 const uvOffsetX0(kOffsets[0] / actualSourceWidth, 0.5f / actualSourceHeight, 0.0f,                              0.0f);
   CVector4 const uvOffsetX1(kOffsets[1] / actualSourceWidth, 0.5f / actualSourceHeight, kOffsets[2] / actualSourceWidth, 0.5f / actualSourceHeight);
   CVector4 const uvOffsetX2(kOffsets[3] / actualSourceWidth, 0.5f / actualSourceHeight, kOffsets[4] / actualSourceWidth, 0.5f / actualSourceHeight);
   CVector4 const uvOffsetX3(kOffsets[5] / actualSourceWidth, 0.5f / actualSourceHeight, kOffsets[6] / actualSourceWidth, 0.5f / actualSourceHeight);

   CVector4 const uvOffsetY0(0.5f / actualSourceWidth, kOffsets[0] / actualSourceHeight, 0.0f,                       0.0f);
   CVector4 const uvOffsetY1(0.5f / actualSourceWidth, kOffsets[1] / actualSourceHeight, 0.5f / actualSourceWidth, kOffsets[2] / actualSourceHeight);
   CVector4 const uvOffsetY2(0.5f / actualSourceWidth, kOffsets[3] / actualSourceHeight, 0.5f / actualSourceWidth, kOffsets[4] / actualSourceHeight);
   CVector4 const uvOffsetY3(0.5f / actualSourceWidth, kOffsets[5] / actualSourceHeight, 0.5f / actualSourceWidth, kOffsets[6] / actualSourceHeight);

#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
   gpRenderBackend->SetAlphaTestEnable(false);
#endif
   gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   // Blur X
   {
      CVector4 const scale(weight0 * pass1Scale, weight12 * pass1Scale, weight34 * pass1Scale, weight56 * pass1Scale);
      gpRenderBackend->SetVertexRegisters(FX::kReg_Scale, 1, &scale);

      SRenderTarget renderTarget(pTemp, NULL, NULL, NULL, NULL);
      renderTarget.mRenderTargetViewPort.mViewportEnabled = 1;
      renderTarget.mRenderTargetViewPort.mViewportX = dstRect.x1;
      renderTarget.mRenderTargetViewPort.mViewportY = dstRect.y1;
      renderTarget.mRenderTargetViewPort.mViewportWidth = destWidth;
      renderTarget.mRenderTargetViewPort.mViewportHeight = destHeight;
      gpRenderBackend->SetRenderTarget(renderTarget);

      gpRenderBackend->SetTexture(0, pSource);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffsetX0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffsetX1);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset2, 1, &uvOffsetX2);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset3, 1, &uvOffsetX3);

      gpRenderBackend->RenderQuad(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, u0, u1, v0, v1);
      gpRenderBackend->ResolvePartialRenderTarget(pTemp, dstRect);
   }

   // Blur Y
   {
      CVector4 const scale(weight0 * pass2Scale, weight12 * pass2Scale, weight34 * pass2Scale, weight56 * pass2Scale);
      gpRenderBackend->SetVertexRegisters(FX::kReg_Scale, 1, &scale);

      SRenderTarget renderTarget(pDest, NULL, NULL, NULL, NULL);
      renderTarget.mRenderTargetViewPort.mViewportEnabled = 1;
      renderTarget.mRenderTargetViewPort.mViewportX = dstRect.x1;
      renderTarget.mRenderTargetViewPort.mViewportY = dstRect.y1;
      renderTarget.mRenderTargetViewPort.mViewportWidth = destWidth;
      renderTarget.mRenderTargetViewPort.mViewportHeight = destHeight;
      gpRenderBackend->SetRenderTarget(renderTarget);

      gpRenderBackend->SetTexture(0, pTemp);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffsetY0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffsetY1);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset2, 1, &uvOffsetY2);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset3, 1, &uvOffsetY3);

      gpRenderBackend->RenderQuad(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, u0, u1, v0, v1);
      if( doFinalResolve )
      {
         gpRenderBackend->ResolvePartialRenderTarget(pDest, dstRect);
      }
   }
}

//----------------------------------------------------------------------------

void BP_Downsample(CBaseTexture* pSource, CBaseTexture* pDest)
{
   //TODO
   BP_Downsample2x(pSource, pDest);
}

//----------------------------------------------------------------------------

real32 gBP_DownSamplePixelOffsetX = 0.0f;
real32 gBP_DownSamplePixelOffsetY = 0.0f;

real32 gBP_DownSampleTexelOffsetU1 = 0.0f;
real32 gBP_DownSampleTexelOffsetU2 = 2.0f;
real32 gBP_DownSampleTexelOffsetV1 = 0.0f;
real32 gBP_DownSampleTexelOffsetV2 = 2.0f;

void BP_Downsample2x_Internal(CBaseTexture* pSource, CBaseTexture* pDest)
{
   // A pixel covers a 4x4 texel area in the source texture.
   // Pixel origin is in the center of the pixel, not the top left. That means it is at texel [2, 2].
   // We use the bilinear filter trick to get 4 averaged samples in one texture sample, we do this four times to get 16 samples average together in a single pass (resulting in our lossless 1/4 single pass down sample operation).

   // Down sampling from 1280x720 -> 360x180 this produces near identical results with photoshop bilinear filter.

   CVector4 const pixelOffsets(gBP_DownSamplePixelOffsetX / pDest->GetWidth(), gBP_DownSamplePixelOffsetY / pDest->GetHeight(), 0, 0);
   gpRenderBackend->SetVertexRegisters(FX::kReg_PixelOffset, 1, &pixelOffsets);

   real32 const texelOffsetU = 1.0f / pSource->GetWidth();
   real32 const texelOffsetV = 1.0f / pSource->GetHeight();

   CVector4 const uvOffset0( gBP_DownSampleTexelOffsetU1 * texelOffsetU, gBP_DownSampleTexelOffsetV1 * texelOffsetV, 
                             gBP_DownSampleTexelOffsetU2 * texelOffsetU, gBP_DownSampleTexelOffsetV1 * texelOffsetV);
   CVector4 const uvOffset1( gBP_DownSampleTexelOffsetU1 * texelOffsetU, gBP_DownSampleTexelOffsetV2 * texelOffsetV, 
                             gBP_DownSampleTexelOffsetU2 * texelOffsetU, gBP_DownSampleTexelOffsetV2 * texelOffsetV);

   gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset0);
   gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffset1);

   BP_SetFXShader(FX::kST_Downsample2x);

   gpRenderBackend->RenderFullscreenQuad(1.0f);
}

void BP_Downsample2x(CBaseTexture* pSource, CBaseTexture* pDest)
{
#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
   gpRenderBackend->SetAlphaTestEnable(false);
#endif
   gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   gpRenderBackend->SetRenderTarget(SRenderTarget(pDest, NULL, NULL, NULL, NULL));
   gpRenderBackend->SetDepthCompareEnabled(false);
   
   gpRenderBackend->SetTexture(0, pSource);
   gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   
   BP_Downsample2x_Internal(pSource, pDest);

   gpRenderBackend->SetDepthCompareEnabled(true);
}

//----------------------------------------------------------------------------

void BP_DepthDownsample(CBaseTexture* pSource, CBaseTexture* pDest)
{
   gpRenderBackend->SetRenderTarget(SRenderTarget(NULL, NULL, NULL, NULL, pDest));

   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   gpRenderBackend->SetTexture(0, pSource);
   gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Nearest, CRenderBackend::kFM_Nearest);
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
   gpRenderBackend->SetAlphaTestEnable(false);
#endif
   gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);

   gpRenderBackend->SetDepthCompareEnabled(true);
   gpRenderBackend->SetDepthFunc(CRenderBackend::kDF_Always);
   gpRenderBackend->SetDepthWriteEnabled(true);

   real32 const texelOffsetU = 1.0f / pSource->GetWidth();
   real32 const texelOffsetV = 1.0f / pSource->GetHeight();

   CVector4 const uvOffset0( 0 * texelOffsetU, 0 * texelOffsetV, 1 * texelOffsetU, 0 * texelOffsetV);
   CVector4 const uvOffset1( 0 * texelOffsetU, 1 * texelOffsetV, 1 * texelOffsetU, 1 * texelOffsetV);

   gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset0);
   gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffset1);

   BP_SetFXShader(FX::kST_DownsampleDepth, NULL, 0);

   gpRenderBackend->RenderFullscreenQuad(1.0f);
}

//----------------------------------------------------------------------------

namespace RSX
{
   int gRsxInterruptLabel = -1;
   uint32 gInterruptRequestedValue = 0;
   uint32 gInterruptAcknowledgedValue = 0;
}

//----------------------------------------------------------------------------

#if BP_ENABLE_MLAA && defined(BP_PS3)

#define EDGE_POST_MLAA_USER_COMMAND_1 0xF5AAC0D1
#define EDGE_POST_MLAA_USER_COMMAND_2 0xF5AAC0D2

struct SMLAA
{
   SMLAA()
   {
      mMlaaHandlerMemBlock = NULL;

      gCurrentProcessedDataWidth = -1;
      gCurrentProcessedDataHeight = -1;

      gTaskSyncRsxLabel = -1;
      gTaskSyncRsxLabelValue = 0;
   }

   void* mMlaaHandlerMemBlock;
   EdgePostMlaaContext gMlaaContext;

   int gCurrentProcessedDataWidth;
   int gCurrentProcessedDataHeight;

   int gTaskSyncRsxLabel;
   int gTaskSyncRsxLabelValue;
};

namespace MLAA
{
   int const kSPUCount = 4;
   int const kSPUPriority = 4;

   int gWorkBufferWidth = -1;
   int gWorkBufferHeight = -1;
   int gWorkBufferWidthAligned = -1;
   int gWorkBufferHeightAligned = -1;
   int gWorkBufferPitch = -1;
   int gWorkBufferSize = 0;
   CRenderHWAllocator::SHandle const * gWorkBuffer = NULL;

   int const kMLAACount = 2;
   SMLAA gMLAA[kMLAACount];
   int gCurrentMLAASubContext = 0;

   int gEnabled = 1;

   int gShowEdges = 0;
   int gThresholdBase = 10;
   int gThresholdFactor = 50;
}

void BP_InitMLAA()
{
#if 0 //BP_ENABLE_DEBUG_MENU
   int mlaaMenu = BP_DebugMenu_GetMenu("MLAA");
   BP_DebugMenu_AddBool(mlaaMenu, "Enabled", &MLAA::gEnabled);
   BP_DebugMenu_AddBool(mlaaMenu, "Show Edges", &MLAA::gShowEdges);
   BP_DebugMenu_AddInt(mlaaMenu, "Threshold Base", &MLAA::gThresholdBase, 0, 0xFF, 1, 2);
   BP_DebugMenu_AddInt(mlaaMenu, "Threshold Factor", &MLAA::gThresholdFactor, 0, 0xFFFF, 1, 10);
#endif

   // Allocate system buffer for processing (worst case resolution) (we support 1920x1080, 960x1080 and 1280x720)
#if EDGE_MLAA_SUPPORT_1080
   MLAA::gWorkBufferWidth = 1920;
   MLAA::gWorkBufferHeight = 1080;
#else
   MLAA::gWorkBufferWidth = 1280;
   MLAA::gWorkBufferHeight = 1080;
#endif

   MLAA::gWorkBufferWidthAligned = (MLAA::gWorkBufferWidth + 127) & ~127;
   MLAA::gWorkBufferHeightAligned = (MLAA::gWorkBufferHeight + 127) & ~127;

   MLAA::gWorkBufferPitch = MLAA::gWorkBufferWidthAligned * 4;
   
   MLAA::gWorkBufferSize = MLAA::gWorkBufferPitch * MLAA::gWorkBufferHeightAligned;
   
   MLAA::gWorkBuffer = gpRenderBackend->AllocFixed(MLAA::gWorkBufferSize, 1024*1024, kRM_System);

   // Clear MLAA buffer
   BP_ClearMLAABuffer();

   for( int i = 0; i < MLAA::kMLAACount; ++i )
   {
      SMLAA & mlaa = MLAA::gMLAA[i];

      // Initialize MLAA context
      const uint32_t mlaaHandlerMemBlockSize = EDGE_POST_MLAA_HANDLER_BASE_BUFFER_SIZE + EDGE_POST_MLAA_HANDLER_SPU_BUFFER_SIZE(MLAA::kSPUCount);
      mlaa.mMlaaHandlerMemBlock = memalign(EDGE_POST_MLAA_HANDLER_BUFFER_ALIGN, mlaaHandlerMemBlockSize);

      mlaa.gTaskSyncRsxLabel = gpRenderBackend->AllocateRSXLabel();

      uint8_t const kPriorities[8] = { MLAA::kSPUPriority, MLAA::kSPUPriority, MLAA::kSPUPriority, MLAA::kSPUPriority, 0, 0, 0, 0 };

      edgePostMlaaInitializeContext(&mlaa.gMlaaContext, MLAA::kSPUCount, gpSpursManager->mpSPURS_Main.get(), kPriorities, mlaa.gTaskSyncRsxLabel, mlaa.mMlaaHandlerMemBlock, mlaaHandlerMemBlockSize);

      *cellGcmGetLabelAddress(mlaa.gTaskSyncRsxLabel) = mlaa.gTaskSyncRsxLabelValue = 0;
   }
}

void BP_DestroyMLAA()
{
   for(int i = 0; i < MLAA::kMLAACount; ++i )
   {
      SMLAA & mlaa = MLAA::gMLAA[i];

      edgePostMlaaDestroyContext(&mlaa.gMlaaContext);
      free(mlaa.mMlaaHandlerMemBlock);
   }

   gpRenderBackend->Free(MLAA::gWorkBuffer);
}

void BP_ClearMLAABuffer()
{
   BPE_ASSERT_NO_MSG( MLAA::gWorkBuffer != NULL );
   BPE_ASSERT_NO_MSG( MLAA::gWorkBuffer->mpAddress != NULL );
   memset(MLAA::gWorkBuffer->mpAddress, 0, MLAA::gWorkBufferSize);
}

void BP_BeginMLAA(CBaseTexture* pInput)
{
   if( !MLAA::gEnabled && !MLAA::gShowEdges )
      return;

   SMLAA & mlaa = MLAA::gMLAA[MLAA::gCurrentMLAASubContext];

   CTexture* pInputTexture = (CTexture*)pInput;

   mlaa.gCurrentProcessedDataWidth = bpe::min_val(pInput->GetWidth(), MLAA::gWorkBufferWidth);
   mlaa.gCurrentProcessedDataHeight = bpe::min_val(pInput->GetHeight(), MLAA::gWorkBufferHeight);

   uint32 workOffset;
   uint32 workLocation;
   gpRenderBackend->GcmGetOffsetAndLocation(MLAA::gWorkBuffer, &workOffset, &workLocation);

   // Copy input buffer into process buffer
   cellGcmSetTransferImage(CELL_GCM_TRANSFER_LOCAL_TO_MAIN, 
                           workOffset, MLAA::gWorkBufferPitch, 0, 0, 
                           pInputTexture->mTexture.offset, pInputTexture->mTexture.pitch, 0, 0,
                           mlaa.gCurrentProcessedDataWidth, mlaa.gCurrentProcessedDataHeight, 4);

   // We're about to update the structures that control the SPU tasks, we need to make sure the SPU is done processing the previous task before we can update this.
   edgePostMlaaWait(&mlaa.gMlaaContext);

   uint8_t mode = EDGE_POST_MLAA_MODE_ENABLED;
   
   if( MLAA::gShowEdges )
      mode |= EDGE_POST_MLAA_MODE_SHOW_EDGES;

   // Initiate SPU tasks, this does NOT start them however.
   edgePostMlaaPrepareWithRelativeThreshold( &mlaa.gMlaaContext, 
                                             MLAA::gWorkBuffer->mpAddress, MLAA::gWorkBuffer->mpAddress,
                                             MLAA::gWorkBufferWidthAligned, MLAA::gWorkBufferHeight, MLAA::gWorkBufferPitch,
                                             MLAA::gThresholdBase,
                                             MLAA::gThresholdFactor,
                                             mode,
                                             ++mlaa.gTaskSyncRsxLabelValue);

   // We do not want to trigger an interrupt while a previous interrupt is still 
   // pending/executing as this will likely confuse the RSX making it call the
   // interrupt function with the wrong cause word.
   cellGcmSetWaitLabel(RSX::gRsxInterruptLabel, RSX::gInterruptRequestedValue);
   RSX::gInterruptRequestedValue++;

   // We want to start the tasks once RSX has successfully copied the
   // texture to XDR. We do this by using the user command callback
   // and then kicking the tasks form there.
   switch(MLAA::gCurrentMLAASubContext)
   {
   case 0:
      cellGcmSetUserCommand(EDGE_POST_MLAA_USER_COMMAND_1);
      break;
   case 1:
      cellGcmSetUserCommand(EDGE_POST_MLAA_USER_COMMAND_2);
      break;
   }

   // kick rsx, so we do not stall.
   cellGcmFlush();
}

void BP_FinalizeMLAA(CBaseTexture* pOutput)
{
   if( !MLAA::gEnabled && !MLAA::gShowEdges )
      return;

   SMLAA & mlaa = MLAA::gMLAA[MLAA::gCurrentMLAASubContext];

   // Have RSX wait for SPU to finish processing MLAA buffer
   cellGcmSetWaitLabel(mlaa.gTaskSyncRsxLabel, mlaa.gTaskSyncRsxLabelValue);

   uint32 workOffset;
   uint32 workLocation;
   gpRenderBackend->GcmGetOffsetAndLocation(MLAA::gWorkBuffer, &workOffset, &workLocation);

   int const outputWidth = pOutput->GetWidth();
   int const outputHeight = pOutput->GetHeight();

   // Copy process buffer into output buffer
   {
      CTexture* pOutputTexture = (CTexture*)pOutput;

      int const sourceWidth = mlaa.gCurrentProcessedDataWidth;
      int const sourceHeight = mlaa.gCurrentProcessedDataHeight;
      int const destOffsetX = (outputWidth - sourceWidth) / 2;
      int const destOffsetY = (outputHeight - sourceHeight) / 2;

      cellGcmSetTransferImage(CELL_GCM_TRANSFER_MAIN_TO_LOCAL, 
                              pOutputTexture->mTexture.offset, pOutputTexture->mTexture.pitch, destOffsetX, destOffsetY,
                              workOffset, MLAA::gWorkBufferPitch, 0, 0,
                              sourceWidth, sourceHeight, 4);
   }

   MLAA::gCurrentMLAASubContext = (MLAA::gCurrentMLAASubContext + 1) % MLAA::kMLAACount;
}

#endif

//----------------------------------------------------------------------------

#if BP_ENABLE_MLAA && defined(BP_PS3)

void BP_RsxDispatcherCallback(uint32_t const cause)
{
   switch(cause)
   {
   case EDGE_POST_MLAA_USER_COMMAND_1:
      edgePostMlaaKickTasks(&MLAA::gMLAA[0].gMlaaContext);
      break;

   case EDGE_POST_MLAA_USER_COMMAND_2:
      edgePostMlaaKickTasks(&MLAA::gMLAA[1].gMlaaContext);
      break;

   default:
      assert(!"Unknown rsx interrupt parameter.");
   }

   // release the RSX interrupt label
   RSX::gInterruptAcknowledgedValue++;
   *cellGcmGetLabelAddress(RSX::gRsxInterruptLabel) = RSX::gInterruptAcknowledgedValue;
}

void BP_RsxInitUserCallback()
{
   // Setup RSX related syncronization functions
   cellGcmSetUserHandler(BP_RsxDispatcherCallback);

   RSX::gRsxInterruptLabel = gpRenderBackend->AllocateRSXLabel();
   *cellGcmGetLabelAddress(RSX::gRsxInterruptLabel) = 0;
}

#endif

//----------------------------------------------------------------------------

void BP_CopyTexture(CBaseTexture* pSource, CBaseTexture* pDest)
{
   int const destWidth = pDest ? pDest->GetWidth() : gpRenderBackend->GetBackBufferWidth();
   int const destHeight = pDest ? pDest->GetHeight() : gpRenderBackend->GetBackBufferHeight();

   bool const isSameSize = (pSource->GetWidth() == destWidth) &&
                           (pSource->GetHeight() == destHeight);

   CRenderBackend::EFilterMode const filterMode = isSameSize ? CRenderBackend::kFM_Nearest : CRenderBackend::kFM_Linear;

   gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   gpRenderBackend->SetRenderTarget(SRenderTarget(pDest, NULL, NULL, NULL, NULL));
   gpRenderBackend->SetDepthCompareEnabled(false);
#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
   gpRenderBackend->SetAlphaTestEnable(false);
#endif

   gpRenderBackend->SetTexture(0, pSource);
   gpRenderBackend->SetTextureFilter(0, filterMode, filterMode);
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

   CVector4 const uvOffset(0.5f / destWidth, 0.5f / destHeight, 0, 0);
   gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);

   BP_SetFXShader(FX::kST_TextureRGBA);

   gpRenderBackend->RenderFullscreenQuad(1.0f);

   gpRenderBackend->SetDepthCompareEnabled(true);
#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
   gpRenderBackend->SetAlphaTestEnable(true);
#endif
}

//----------------------------------------------------------------------------

void BP_CopyTextureRect(CBaseTexture* pSource, CBaseTexture* pDest, int const x0, int const y0, int const x1, int const y1)
{
   int const sourceWidth = x1 - x0;
   int const sourceHeight = y1 - y0;

   bool const isSameSize = (sourceWidth == pDest->GetWidth()) && (sourceHeight == pDest->GetHeight());

   CRenderBackend::EFilterMode const filterMode = isSameSize ? CRenderBackend::kFM_Nearest : CRenderBackend::kFM_Linear;

   gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   gpRenderBackend->SetRenderTarget(SRenderTarget(pDest, NULL, NULL, NULL, NULL));
   gpRenderBackend->SetDepthCompareEnabled(false);
#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
   gpRenderBackend->SetAlphaTestEnable(false);
#endif

   gpRenderBackend->SetTexture(0, pSource);
   gpRenderBackend->SetTextureFilter(0, filterMode, filterMode);
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

   int const destWidth = pDest ? pDest->GetWidth() : gpRenderBackend->GetBackBufferWidth();
   int const destHeight = pDest ? pDest->GetHeight() : gpRenderBackend->GetBackBufferHeight();

   CVector4 const uvOffset(0.5f / destWidth, 0.5f / destHeight, 0, 0);
   gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);

   BP_SetFXShader(FX::kST_TextureRGBA);

   int const sourceBufferWidth = pSource->GetWidth();
   int const sourceBufferHeight = pSource->GetHeight();

   real32 const u0 = (real32)x0 / sourceBufferWidth;
   real32 const u1 = (real32)x1 / sourceBufferWidth;
   real32 const v0 = (real32)y0 / sourceBufferHeight;
   real32 const v1 = (real32)y1 / sourceBufferHeight;

   gpRenderBackend->RenderQuad(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, u0, u1, v0, v1);

   gpRenderBackend->SetDepthCompareEnabled(true);
#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
   gpRenderBackend->SetAlphaTestEnable(true);
#endif
}

//----------------------------------------------------------------------------

void BP_CopyTextureSRect(CBaseTexture* pSource, CBaseTexture* pDest, SRect const & srcRect, SRect const & dstRect)
{
   int const dstWidthTotal = pDest->GetWidth();
   int const dstHeightTotal = pDest->GetHeight();
   int destWidth;
   int destHeight;
   float u0;
   float u1;
   float v0;
   float v1;
   ComputeWidthHeightQuadUVs(dstRect, dstWidthTotal, dstHeightTotal,
      u0, u1, v0, v1, destWidth, destHeight);
   int const sourceWidth = srcRect.x2 - srcRect.x1;
   int const sourceHeight = srcRect.y2 - srcRect.y1;

   bool const isSameSize = (destWidth == sourceWidth) && (destHeight == sourceHeight);

   CRenderBackend::EFilterMode const filterMode = isSameSize ? CRenderBackend::kFM_Nearest : CRenderBackend::kFM_Linear;

   gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   SRenderTarget renderTarget(pDest, NULL, NULL, NULL, NULL);
   renderTarget.mRenderTargetViewPort.mViewportEnabled = 1;
   renderTarget.mRenderTargetViewPort.mViewportX = dstRect.x1;
   renderTarget.mRenderTargetViewPort.mViewportY = dstRect.y1;
   renderTarget.mRenderTargetViewPort.mViewportWidth = destWidth;
   renderTarget.mRenderTargetViewPort.mViewportHeight = destHeight;
   gpRenderBackend->SetRenderTarget(renderTarget);

   gpRenderBackend->SetDepthCompareEnabled(false);
#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
   gpRenderBackend->SetAlphaTestEnable(false);
#endif

   gpRenderBackend->SetTexture(0, pSource);
   gpRenderBackend->SetTextureFilter(0, filterMode, filterMode);
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

   int const actualDestWidth = pDest ? pDest->GetWidth() : gpRenderBackend->GetBackBufferWidth();
   int const actualDestHeight = pDest ? pDest->GetHeight() : gpRenderBackend->GetBackBufferHeight();

   CVector4 const uvOffset(0.5f / actualDestWidth, 0.5f / actualDestHeight, 0, 0);
   gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);

   BP_SetFXShader(FX::kST_TextureRGBA);

   gpRenderBackend->RenderQuad(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, u0, u1, v0, v1);

   gpRenderBackend->SetDepthCompareEnabled(true);
#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
   gpRenderBackend->SetAlphaTestEnable(true);
#endif
}

//----------------------------------------------------------------------------

#if BPE_TARGET == BPE_TARGET_PS3
void BP_CopyMemory(CBaseTexture* pSource, CBaseTexture* pDest, int const bilinear)
{
   bool const isSameSize = (pSource->GetWidth() == pDest->GetWidth()) && (pSource->GetHeight() == pDest->GetHeight());

   uint8_t mode = 0; 

   CTexture* pSourceTexture = (CTexture*)pSource;
   CTexture* pDestTexture = (CTexture*)pDest;

   if( pSourceTexture->mTexture.location == CELL_GCM_LOCATION_LOCAL )
   {
      if( pDestTexture->mTexture.location == CELL_GCM_LOCATION_LOCAL )
         mode = CELL_GCM_TRANSFER_LOCAL_TO_LOCAL;
      else
         mode = CELL_GCM_TRANSFER_LOCAL_TO_MAIN;
   }
   else
   {
      if( pDestTexture->mTexture.location == CELL_GCM_LOCATION_LOCAL )
         mode = CELL_GCM_TRANSFER_MAIN_TO_LOCAL;
      else
         mode = CELL_GCM_TRANSFER_MAIN_TO_MAIN;
   }

   if( isSameSize )
   {
      cellGcmSetTransferImage(mode, 
         pDestTexture->mTexture.offset, pDestTexture->mTexture.pitch, 0, 0,
         pSourceTexture->mTexture.offset, pSourceTexture->mTexture.pitch, 0, 0,
         pSourceTexture->GetWidth(), pSourceTexture->GetHeight(), 4);
   }
   else
   {
      CellGcmTransferScale scale;
      memset( &scale, 0, sizeof(scale));
      scale.conversion = CELL_GCM_TRANSFER_CONVERSION_TRUNCATE;
      scale.operation = CELL_GCM_TRANSFER_OPERATION_SRCCOPY;
      scale.format = CELL_GCM_TRANSFER_SCALE_FORMAT_A8R8G8B8;
      scale.outW = scale.clipW = pDestTexture->GetWidth();
      scale.outH = scale.clipH = pDestTexture->GetHeight();
      scale.ratioX = cellGcmGetFixedSint32(pSourceTexture->GetWidth() / (float)pDestTexture->GetWidth());
      scale.ratioY = cellGcmGetFixedSint32(pSourceTexture->GetHeight() / (float)pDestTexture->GetHeight());
      scale.inW = pSourceTexture->GetWidth();
      scale.inH = pSourceTexture->GetHeight();
      scale.origin = CELL_GCM_TRANSFER_ORIGIN_CENTER;
      scale.interp = bilinear ? CELL_GCM_TRANSFER_INTERPOLATOR_FOH : CELL_GCM_TRANSFER_INTERPOLATOR_ZOH;
      scale.inX = bilinear ? cellGcmGetFixedUint16(0.5f) : cellGcmGetFixedUint16(0.0f);
      scale.inY = bilinear ? cellGcmGetFixedUint16(0.5f) : cellGcmGetFixedUint16(0.0f);
      scale.pitch = pSourceTexture->mTexture.pitch;
      scale.offset = pSourceTexture->mTexture.offset;

      CellGcmTransferSurface surface;
      memset( &surface, 0, sizeof(surface));
      surface.format = CELL_GCM_TRANSFER_SURFACE_FORMAT_A8R8G8B8;
      surface.pitch = pDestTexture->mTexture.pitch;
      surface.offset = pDestTexture->mTexture.offset;

      // transfer/scale
      cellGcmSetTransferScaleMode(mode, CELL_GCM_TRANSFER_SURFACE);
      cellGcmSetTransferScaleSurface(&scale, &surface);
      cellGcmSetTransferLocation(pDestTexture->mTexture.location);
   }
}
#endif

//----------------------------------------------------------------------------

void BP_ZCullReload(int clearStencil)
{
#if BPE_TARGET == BPE_TARGET_PS3
   BPE_ADD_SCOPED_GPU_PROFILE_MARKER("ZCull Reload");

   using namespace cell::Gcm;

   //1
   cellGcmSetInvalidateZcull();

   if( clearStencil )
      cellGcmSetScullControl(CELL_GCM_SCULL_SFUNC_NOTEQUAL, 0x80, 0xFF);

   //2
   gpRenderBackend->SetColorAlphaWriteEnabled(false, false);

   //3
   cellGcmSetDepthMask(CELL_GCM_TRUE);
   cellGcmSetDepthTestEnable(CELL_GCM_TRUE);
   cellGcmSetDepthFunc(CELL_GCM_GEQUAL);

   //4
   if( clearStencil )
   {
      cellGcmSetStencilTestEnable(CELL_GCM_TRUE);
      cellGcmSetStencilFunc(CELL_GCM_ALWAYS, 0, 0);
      cellGcmSetStencilOp(CELL_GCM_KEEP, CELL_GCM_KEEP, CELL_GCM_INVERT);
      cellGcmSetStencilMask(0xFF);
   }
   else
   {
      cellGcmSetStencilTestEnable(CELL_GCM_FALSE);
      cellGcmSetStencilFunc(CELL_GCM_ALWAYS, 0, 0);
      cellGcmSetStencilOp(CELL_GCM_KEEP, CELL_GCM_KEEP, CELL_GCM_KEEP);
   }

   cellGcmSetTwoSidedStencilTestEnable(CELL_GCM_FALSE);

   cellGcmSetBlendEnable(CELL_GCM_FALSE);
   cellGcmSetBlendEnableMrt(CELL_GCM_FALSE, CELL_GCM_FALSE, CELL_GCM_FALSE);
   cellGcmSetAlphaTestEnable(CELL_GCM_FALSE);
   cellGcmSetZMinMaxControl(CELL_GCM_TRUE, CELL_GCM_FALSE, CELL_GCM_FALSE);
   cellGcmSetDepthBoundsTestEnable(CELL_GCM_FALSE);

   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   //5
   cellGcmSetClearZcullSurface(CELL_GCM_TRUE, clearStencil ? CELL_GCM_TRUE : CELL_GCM_FALSE);

   //6
   BP_SetFXShader(FX::kST_ZCullReload, NULL, 0);

   gpRenderBackend->RenderFullscreenQuad(1.0f);

   // Reenable color writes!
   gpRenderBackend->SetColorAlphaWriteEnabled(true, true);
#endif
}

//----------------------------------------------------------------------------

#if BP_VITA
void BP_PostFx_MonoApply(SBP_PFX_Mono* pPacket, int mini)
{
   CBaseTexture* pPaletteTxture = FX::pMonoPaletteTextures[FX::gCurrentMonoPaletteTexture];
   FX::gCurrentMonoPaletteTexture = (FX::gCurrentMonoPaletteTexture + 1) % FX::kMaxMonoPaletteTextureCount;

   // Initialize palette texture
   {
      uint32* pTextureMem = NULL;
      int texturePitch = 0;
      pPaletteTxture->Lock((void**)&pTextureMem, &texturePitch);

      for( int i = 0; i < 256; ++i )
      {
         uint32 const sourceColor = pPacket->palette[i];
         pTextureMem[i] = (sourceColor & 0xFF00FF00) | ((sourceColor & 0x00FF0000) >> 16) | ((sourceColor & 0x000000FF) << 16);
      }

      pPaletteTxture->Unlock();
   }

   // Apply monochrome effect
   if (mini != kMT_Scene)
   {
      if (mini == kMT_MiniGlare && gVTADebugUberscale)
      {
         // glare mono gets done as part of the big downscaling mess later
         FX::pGlareMonoPalette = pPaletteTxture;
         return;
      }

      FX::gMainRenderTarget = gpRenderBackend->GetCurrentRenderTarget();
      gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

      CBaseTexture *pFrameBuffer = gpRenderBackend->GetCurrentRenderTexture();

      gpRenderBackend->SetRenderTarget(SRenderTarget(FX::pBlurTarget, NULL, NULL, NULL, NULL));

      gpRenderBackend->SetTexture(0, pFrameBuffer);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

      gpRenderBackend->SetTexture(1, pPaletteTxture);
      gpRenderBackend->SetTextureFilter(1, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(1, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
      gpRenderBackend->SetAlphaTestEnable(false);
#endif

      gpRenderBackend->SetDepthCompareEnabled(false);

      CShaderParameterBufferAllocator::BeginRegion();

      CShaderParameterBuffer parameterBuffer = CShaderParameterBufferAllocator::AllocateParameterBuffer();
      parameterBuffer.AddParameter(FX::gMonochromeWeights, *(CVector4*)&pPacket->colorWeighting);
      CShaderParameterBufferAllocator::FinalizeParameterBuffer(parameterBuffer);

      gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);

      CVector4 const uvOffset(0, 0, 0, 0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);

      BP_SetFXShader(FX::kST_GlareMono, parameterBuffer.GetStartAddress(), parameterBuffer.GetSize());

      // glare downsamples twice
      if (mini == kMT_MiniGlare)
      {
         gpRenderBackend->RenderQuad(
            -1.0f, -1.0f+180.0f/512.0f,
            1.0f-112.0f/256.0f, 1.0f,
            1.0f,
            0.0f, 1.0f, 0.0f, 1.0f);
      }
      else
      {
         gpRenderBackend->RenderQuad(
            -1.0f, -1.0f+480.0f/512.0f,
            0.0f, 1.0f,
            1.0f,
            0.0f, 1.0f, 0.0f, 1.0f);
      }
      CShaderParameterBufferAllocator::EndRegion();

      gpRenderBackend->SetDepthCompareEnabled(true);

      // if we're not doing this for the glare, restore the main rendertarget, as something expects to use
      // the mono in regular rendering
      if (mini == kMT_Mini)
         gpRenderBackend->SetRenderTarget(FX::gMainRenderTarget);
   }
   else
   {
      BP_PostFx_Offscreen(NULL);

      gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

      gpRenderBackend->SetTexture(0, pPaletteTxture);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
      gpRenderBackend->SetAlphaTestEnable(false);
#endif

      gpRenderBackend->SetDepthCompareEnabled(false);

      CShaderParameterBufferAllocator::BeginRegion();

      CShaderParameterBuffer parameterBuffer = CShaderParameterBufferAllocator::AllocateParameterBuffer();
      parameterBuffer.AddParameter(FX::gMonochromeWeights, *(CVector4*)&pPacket->colorWeighting);
      CShaderParameterBufferAllocator::FinalizeParameterBuffer(parameterBuffer);

      CVector4 const uvOffset(0, 0, 0, 0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);

      BP_SetFXShader(FX::kST_MonoMini, parameterBuffer.GetStartAddress(), parameterBuffer.GetSize());

      gpRenderBackend->RenderFullscreenQuad(1.0f);

      CShaderParameterBufferAllocator::EndRegion();

      gpRenderBackend->SetDepthCompareEnabled(true);
   }
}

extern int gGlareNewEnable;

void BP_PostFx_Mono(void* pData)
{
   SBP_PFX_Mono* pPacket = (SBP_PFX_Mono*)pData;

   switch(pPacket->monoType)
   {
   case kMT_Mini:
      {
         BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:MonoMini");

         if (gFXShowPasses)
         {
            BP_DebugText_Print("MonoMini");
         }

         break; // no mini for now
//         BP_PostFx_MonoApply(pPacket, pPacket->monoType);
      }
      break;

   case kMT_MiniGlare:
      {
         BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:MonoGlare");
#if MGS_VERSION == 3
         if (gFXShowPasses)
            BP_DebugText_Print("MonoGlare");

         if (gGlareNewEnable)
            BP_PostFx_MonoApply(pPacket, pPacket->monoType);
#endif
      }
      break;

   case kMT_Scene:
      {
         BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:MonoScene");

         if (gFXShowPasses)
            BP_DebugText_Print("MonoScene");

         BP_PostFx_MonoApply(pPacket, pPacket->monoType);
      }
      break;
   default:
      break;
   }
}
#else
void BP_PostFx_MonoApply(SBP_PFX_Mono* pPacket, CBaseTexture* pSourceTexture, CBaseTexture* pDestTexture, CRenderBackend::EFilterMode const sourceFilter, int const resolveDest)
{
   CBaseTexture* pPaletteTxture = FX::pMonoPaletteTextures[FX::gCurrentMonoPaletteTexture];
   FX::gCurrentMonoPaletteTexture = (FX::gCurrentMonoPaletteTexture + 1) % FX::kMaxMonoPaletteTextureCount;

   // Initialize palette texture
   {
      uint32* pTextureMem = NULL;
      int texturePitch = 0;
      pPaletteTxture->Lock((void**)&pTextureMem, &texturePitch);

      for( int i = 0; i < 256; ++i )
      {
         uint32 const sourceColor = pPacket->palette[i];
         pTextureMem[i] = (sourceColor & 0xFF00FF00) | ((sourceColor & 0x00FF0000) >> 16) | ((sourceColor & 0x000000FF) << 16);
      }

      pPaletteTxture->Unlock();
   }

   // Apply monochrome effect
   {
      gpRenderBackend->SetRenderTarget(SRenderTarget(pDestTexture, NULL, NULL, NULL, NULL));
      gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

      gpRenderBackend->SetTexture(0, pSourceTexture);
      gpRenderBackend->SetTextureFilter(0, sourceFilter, sourceFilter);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

      gpRenderBackend->SetTexture(1, pPaletteTxture);
      gpRenderBackend->SetTextureFilter(1, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(1, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
      gpRenderBackend->SetAlphaTestEnable(false);
#endif
      gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);

      gpRenderBackend->SetDepthCompareEnabled(false);

      CShaderParameterBufferAllocator::BeginRegion();

      CShaderParameterBuffer parameterBuffer = CShaderParameterBufferAllocator::AllocateParameterBuffer();
      parameterBuffer.AddParameter(FX::gMonochromeWeights, *(CVector4*)&pPacket->colorWeighting);
      CShaderParameterBufferAllocator::FinalizeParameterBuffer(parameterBuffer);

      real32 const uTexelOffset = 0.5f * pSourceTexture->GetWidth() / pDestTexture->GetWidth();
      real32 const vTexelOffset = 0.5f * pSourceTexture->GetWidth() / pDestTexture->GetWidth();

      CVector4 const uvOffset(uTexelOffset / pSourceTexture->GetWidth(), vTexelOffset / pSourceTexture->GetHeight(), 0, 0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);

      BP_SetFXShader(FX::kST_MonoMini, parameterBuffer.GetStartAddress(), parameterBuffer.GetSize());

      gpRenderBackend->RenderFullscreenQuad(1.0f);

      CShaderParameterBufferAllocator::EndRegion();

      gpRenderBackend->SetDepthCompareEnabled(true);

      // Resolve monochrome render back to texture for later use.
      if( resolveDest )
         gpRenderBackend->ResolveRenderTarget(pDestTexture);
   }
}

void BP_PostFx_Mono(void* pData)
{
#ifndef BP_RENDER_SINGLE_RENDER_TARGET

   SBP_PFX_Mono* pPacket = (SBP_PFX_Mono*)pData;

   // Store original render target
   SRenderTarget const originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();

   switch(pPacket->monoType)
   {
   case kMT_Mini:
   case kMT_MiniGlare:
      {
         BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:MonoMini");

         // Resolve current frame buffer to be used as texture input for monochrome pass.
         CBaseTexture* pCurrentRenderTarget = BP_GetRenderTarget(kRT_CurrentFrameBuffer_NoMSAA);
         gpRenderBackend->ResolveRenderTarget(pCurrentRenderTarget);

         // Get ptr to monochrome effect dest texture
         CBaseTexture* pTempBuffer = BP_GetRenderTarget(kRT_MonoMini);

         BP_PostFx_MonoApply(pPacket, pCurrentRenderTarget, pTempBuffer, CRenderBackend::kFM_Linear, 1/*resolveDest*/);      
      }
      break;

   case kMT_Scene:
      {
         BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:MonoScene");

         // Dest buffer
         CBaseTexture* pCurrentRenderTarget = BP_GetRenderTarget(kRT_CurrentFrameBuffer_NoMSAA);

         // Source texture
         CBaseTexture* pTempBuffer = BP_GetRenderTarget(kRT_TempBuffer_NoMSAA);

#if BPE_TARGET == BPE_TARGET_X360
         // Resolve current render target directly to temp buffer
         gpRenderBackend->ResolveRenderTarget(pTempBuffer);
#else
         // Copy current render target into source texture
         BP_CopyTexture(pCurrentRenderTarget, pTempBuffer);
#endif

         BP_PostFx_MonoApply(pPacket, pTempBuffer, pCurrentRenderTarget, CRenderBackend::kFM_Nearest,  0/*resolveDest*/);
      }
      break;

   }

   // Restore original render target
   gpRenderBackend->SetRenderTarget(originalRenderTarget);
#endif
}
#endif
//----------------------------------------------------------------------------
int gBlinds_Enable = 1;
void BP_PostFx_Blinds(void* pData)
{
   if (gFXShowPasses)
      BP_DebugText_Print("Blinds");
   if( gBlinds_Enable )
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:Blinds");

      SBP_PFX_Blinds* pPacket = (SBP_PFX_Blinds*)pData;
      BP_GS_SetAlpha(pPacket->alphaData);
      // calculate
      SRenderTarget const originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();
      int actualRenderTargetHeight;
      if (originalRenderTarget.mpColorBuffer[0])
         actualRenderTargetHeight = originalRenderTarget.mpColorBuffer[0]->GetHeight();
      else
         actualRenderTargetHeight = 544;

      gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

      if( originalRenderTarget.mRenderTargetViewPort.mViewportEnabled )
      {
         actualRenderTargetHeight = originalRenderTarget.mRenderTargetViewPort.mViewportHeight;
      }
      int blindTotalHeight = pPacket->darkenHeight + pPacket->blankHeight;
      int realNumBlinds = actualRenderTargetHeight/blindTotalHeight;
      ++realNumBlinds; // In case not an even divide we'll draw a blind partially at bottom of screen
      float blindDarkenHeightNormalized = (float)pPacket->darkenHeight/actualRenderTargetHeight;
      float blindBlankHeightNormalized = (float)pPacket->blankHeight/actualRenderTargetHeight;
      float startYNormalized = (float)pPacket->startY/actualRenderTargetHeight;
      // we're not rendering to a texture, so the half pixel offset doesn't apply.
#if BP_VITA
      float halfPixelOffset = 0.0f; // We don't actually want to render on texel coordinates, we want to render on pixel coordinates
#else
      float halfPixelOffset = 1.0f/actualRenderTargetHeight/2.0f;
#endif 
      struct SVertexPosCol
      {
         CVector3 pos;
         CVector4 col;
      };
      CVector4 colorBP = BP_DecodeColorF(pPacket->color);
      CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(SVertexPosCol) * 6 * realNumBlinds);
      if( pVertexBuffer )
      {
         SVertexPosCol* pVertex = (SVertexPosCol*)pVertexBuffer->Lock();
         float y0 = startYNormalized + halfPixelOffset;
         for( int iBlind=0;iBlind<realNumBlinds;++iBlind )
         {
            pVertex->pos = CVector3(0.0f, y0, 1.0f); 
            pVertex->col = colorBP;
            pVertex++;
            pVertex->pos = CVector3(1.0f, y0, 1.0f); 
            pVertex->col = colorBP; 
            pVertex++;
            pVertex->pos = CVector3(0.0f, y0+blindDarkenHeightNormalized, 1.0f); 
            pVertex->col = colorBP;
            pVertex++;

            pVertex->pos = CVector3(0.0f, y0+blindDarkenHeightNormalized, 1.0f); 
            pVertex->col = colorBP;
            pVertex++;
            pVertex->pos = CVector3(1.0f, y0, 1.0f); 
            pVertex->col = colorBP; 
            pVertex++;
            pVertex->pos = CVector3(1.0f, y0+blindDarkenHeightNormalized, 1.0f); 
            pVertex->col = colorBP; 
            pVertex++;

            y0 += blindDarkenHeightNormalized+blindBlankHeightNormalized;
         }
         pVertexBuffer->Unlock();

         CVertexData vertexData;
         size_t const bufferIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( SVertexPosCol ) );
         vertexData.SetAttribute(kVDS_Position, offsetof(SVertexPosCol, pos), kVDT_Float3, bufferIndex);
         vertexData.SetAttribute(kVDS_Color0, offsetof(SVertexPosCol, col), kVDT_Float4, bufferIndex);

         CShaderVertexDataBinding binding;
         binding.Set(kVDU_Position, kVDS_Position);
         binding.Set(kVDU_TexCoord0, kVDS_Color0);

         BP_SetFXShader(FX::kST_Blinds, NULL, 0);

         gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
         gpRenderBackend->ForceVertexDataRebind();
         gpRenderBackend->RenderPrimitivesNoIndices(CMeshChunk::kPrimitive_TriangleList, 0, 6*realNumBlinds);
      }
   }
}

#if MGS_VERSION == 3
int gScopeBlurEnable = 1;
void BP_PostFx_ScopeBlur(void* pData)
{
   SBP_PFX_ScopeBlur* pPacket = (SBP_PFX_ScopeBlur*)pData;
   if (gFXShowPasses)
      BP_DebugText_Print("ScopeBlur %f", pPacket->alphaFocusValue/128.0f);
#if BP_VITA
   if( !gScopeBlurEnable )
      return;

   if( pPacket->invisible )
      return;

   FX::gDOFs = -1;
   FX::gDOF_Alpha[0] = pPacket->alphaFocusValue/128.0f;
   gpRenderBackend->OverrideUpscale();
#endif
#ifndef BP_RENDER_SINGLE_RENDER_TARGET

   if( !gScopeBlurEnable )
      return;

   if( pPacket->invisible )
      return;

   BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:ScopeBlur");

   SRenderTarget const originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();
   CVector4 const modulateColor(1.0f, 1.0f, 1.0f, pPacket->alphaFocusValue/128.0f);
   CBaseTexture* pHalfResTexture = BP_GetRenderTarget(kRT_TempBufferHalf1);
   CBaseTexture* pQuaterResTexture = BP_GetRenderTarget(kRT_TempBufferQuarter1);

   // Shrink0
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Shrink0");
      BP_Downsample2x( originalRenderTarget.mpColorBuffer[0] , pHalfResTexture);
      gpRenderBackend->ResolveRenderTarget(pHalfResTexture);
   }
   // Blur0 - Do we want to add this in later?
   // Apply0
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Apply0");
      const CBaseTexture* pAdditionalTextures[] = { pHalfResTexture };
      gpRenderBackend->SetRenderTarget(originalRenderTarget);
      BP_GS_SetAlpha(pPacket->alphaData);
      BP_DrawFullscreenTextureModulateWithXTextures(pAdditionalTextures, 1, modulateColor, 1 /*isFilter*/);
   }


   // Shrink1
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Shrink1");
      BP_Downsample2x( pHalfResTexture, pQuaterResTexture );
      gpRenderBackend->ResolveRenderTarget(pQuaterResTexture);
   }
   // Blur1 - Do we want to add this in later?
   // Apply1
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Apply1");
      const CBaseTexture* pAdditionalTextures[] = { pQuaterResTexture };
      gpRenderBackend->SetRenderTarget(originalRenderTarget);
      BP_GS_SetAlpha(pPacket->alphaData);
      BP_DrawFullscreenTextureModulateWithXTextures(pAdditionalTextures, 1, modulateColor, 1 /*isFilter*/);
   }
#endif
}

#endif

//----------------------------------------------------------------------------

#if BP_VITA
void BP_PostFx_Offscreen(void* pData)
{
   // terrible terrible hack, make sure we haven't been triggered already
   if (!gpRenderBackend->IsInLowresScene())
   {
      return;
   }

   if (gVTADebugUberscale)
   {
      bool OffscreenWorkToDo();
      if (OffscreenWorkToDo())
      {
         SRenderTarget bt = SRenderTarget(FX::pBlurHalfTarget, NULL, NULL, NULL, NULL);
         bt.mPixelCentersHalf = true;
         gpRenderBackend->SetRenderTarget(bt);
         void DoOffscreenVTAMadness();
         DoOffscreenVTAMadness();
      }
      gpRenderBackend->SetRenderTarget(SRenderTarget(gpRenderBackend->GetCurrentDisplayTexture(), NULL, NULL, NULL, gpRenderBackend->GetCurrentDisplayDepth()));
   }
}

void BP_PostFx_ResetDmapackFX()
{
   // clear out the feature settings
   FX::gGlarePlanes = 0;
   FX::gDOFs = 0;
   FX::gHalfresEnabled = 0;
   FX::gUberContrast = 0;
}

#endif

#if MGS_VERSION == 3

void BP_PostFx_MGS3_GlareNew_Border(real32 const width, real32 const height)
{
#ifndef BP_RENDER_SINGLE_RENDER_TARGET

   // Hacky solution to render some black borders, but I doubt it will be a performance issue!
   BP_SetFXShader(FX::kST_TextureRGB, NULL, 0);
   gpRenderBackend->SetTexture(0, &gpRenderBackend->GetBlackMap());

   // top
   gpRenderBackend->RenderQuad(-1.0f, 1.0f, 1.0f - height, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
   // right
   gpRenderBackend->RenderQuad(1.0f - width, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
   // bottom
   gpRenderBackend->RenderQuad(-1.0f, 1.0f, -1.0f + height, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
   // left
   gpRenderBackend->RenderQuad(-1.0f, -1.0f + width, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);

#endif
}

int gGlareNewEnable = 1;
real32 gGlareNewBorderSize = 2.0f;
real32 gGlareNewBloomScale = 1.0f;
real32 gGlareNewBlurWeights[4] = { 0.25f, 0.25f, 0.25f, 0.25f };

#if BP_VITA
static const char *uberscale_names[] =
{
   "Uberscale Rescale",
   "Uberscale DOF1",
   "Uberscale DOF2",
   "Uberscale Scope",
   "Uberscale       Glare1",
   "Uberscale DOF1  Glare1",
   "Uberscale DOF2  Glare1",
   "Uberscale SCOPE Glare1",
   "Uberscale       Glare2",
   "Uberscale DOF1  Glare2",
   "Uberscale DOF2  Glare2",
   "Uberscale SCOPE Glare2",
   "Uberscale       Glare3",
   "Uberscale DOF1  Glare3",
   "Uberscale DOF2  Glare3",
   "Uberscale SCOPE Glare3",
   "Uberscale       Glare4",
   "Uberscale DOF1  Glare4",
   "Uberscale DOF2  Glare4",
   "Uberscale SCOPE Glare4",
   "Uberscale       Glare5",
   "Uberscale DOF1  Glare5",
   "Uberscale DOF2  Glare5",
   "Uberscale SCOPE Glare5",
   "Uberscale       Glare6",
   "Uberscale DOF1  Glare6",
   "Uberscale DOF2  Glare6",
   "Uberscale SCOPE Glare6",
};

//extern int gVTADebugOptCmfHalfresFilter;

void UberscaleSetup(CShaderParameterBuffer &parameterBuffer, bool writeDepth)
{
   CBaseTexture *draw = gpRenderBackend->GetLowresDrawTexture();
   CBaseTexture *depth = gpRenderBackend->GetLowresDrawDepth();

   if (FX::gDOFs)
      parameterBuffer.AddParameter(FX::gPS_DOF_Alpha, *(CVector4*)FX::gDOF_Alpha);
   if (FX::gDOFs > 0)
      parameterBuffer.AddParameter(FX::gPS_DOF_Z, *(CVector4*)FX::gDOF_Z);
   if (FX::gGlarePlanes > 0)
      parameterBuffer.AddParameter(FX::gPS_GlareAlpha, FX::gGlareAlpha);

   if (gVTAUseALUMono)
   {
      parameterBuffer.AddParameter(FX::gMonoScale0, *(CVector4*)FX::gGlareMonoScale0);
      parameterBuffer.AddParameter(FX::gMonoScale1, *(CVector4*)FX::gGlareMonoScale1);
      parameterBuffer.AddParameter(FX::gMonoOfs0, *(CVector4*)FX::gGlareMonoOfs0);
      parameterBuffer.AddParameter(FX::gMonoOfs1, *(CVector4*)FX::gGlareMonoOfs1);
   }

   if (!FX::gUberContrast)
   {
      FX::gUberContrastParam[0] = 0.0f;
      FX::gUberContrastParam[1] = 0.0f;
      FX::gUberContrastParam[2] = 0.0f;
      FX::gUberContrastParam[3] = 1.0f;
   }
   parameterBuffer.AddParameter(FX::gContrastParam, *(CVector4*)FX::gUberContrastParam);
   parameterBuffer.AddParameter(FX::gTweakParam, *(CVector4*)FX::gUberTweakKnobs);

   CShaderParameterBufferAllocator::FinalizeParameterBuffer(parameterBuffer);

   // set base texture
   gpRenderBackend->SetTexture(0, draw);
   gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

   // set depth texture if needed
   if (FX::gHalfresEnabled || FX::gDOFs > 0 || writeDepth)
   {
      gpRenderBackend->SetTexture(1, depth);
      gpRenderBackend->SetTextureFilter(1, CRenderBackend::kFM_Nearest, CRenderBackend::kFM_Nearest);
      gpRenderBackend->SetTextureAddressMode(1, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   }
   // set quarter if needed
   if ((FX::gDOFs > 1) || (FX::gDOFs < 0) || (FX::gGlarePlanes >= 2))
   {
      gpRenderBackend->SetTexture(3, FX::pBlurTextures[2]);
      gpRenderBackend->SetTextureFilter(3, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(3, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   }
   // set mono pal texture if needed
   if (FX::gGlarePlanes >= 2)
   {
      gpRenderBackend->SetTexture(4, FX::pGlareMonoPalette);
      gpRenderBackend->SetTextureFilter(4, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(4, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

      gpRenderBackend->SetTexture(5, FX::pBlurTextures[3]);
      gpRenderBackend->SetTextureFilter(5, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(5, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   }
   // set blur textures if needed
   if (FX::gGlarePlanes >= 3)
   {
      gpRenderBackend->SetTexture(6, FX::pBlurTextures[7]);
      gpRenderBackend->SetTextureFilter(6, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(6, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   }
   if (FX::gGlarePlanes >= 4)
   {
      gpRenderBackend->SetTexture(7, FX::pBlurTextures[9]);
      gpRenderBackend->SetTextureFilter(7, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(7, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   }
   if (FX::gGlarePlanes >= 5)
   {
      gpRenderBackend->SetTexture(8, FX::pBlurTextures[11]);
      gpRenderBackend->SetTextureFilter(8, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(8, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   }
   if (FX::gGlarePlanes >= 6)
   {
      gpRenderBackend->SetTexture(9, FX::pBlurTextures[13]);
      gpRenderBackend->SetTextureFilter(9, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(9, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   }
   // set lowres
   if (FX::gHalfresEnabled)
   {
      gpRenderBackend->SetTexture(10, FX::pHalfresTexture);
      gpRenderBackend->SetTextureFilter(10, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(10, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

      //gpRenderBackend->SetTexture(11, FX::pHalfresDepth);
      //gpRenderBackend->SetTextureFilter(11, CRenderBackend::kFM_Nearest, CRenderBackend::kFM_Nearest);
      //gpRenderBackend->SetTextureAddressMode(11, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   }
   if (FX::gDOFs || FX::gGlarePlanes)
   {
      gpRenderBackend->SetTexture(2, FX::pBlurHalfTarget);
      gpRenderBackend->SetTextureFilter(2, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(2, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   }

   BP_BeginShader(FX::gpUberShader[FX::gHalfresEnabled][(FX::gDOFs >= 0) ? FX::gDOFs : 3][FX::gGlarePlanes][gVTAUseALUMono ? FX::gGlareMonoType : 0][writeDepth], parameterBuffer.GetStartAddress(), parameterBuffer.GetSize());
}

void UberscaleVTA()
{
   // Shader needs:

   // lowres target
   // lowres depth (if DOF > 0)
   // glare mono palette (if glare planes >= 1)
   // half res buffer (if any fullscreen is enabled)
   // quarter res buffer (if DOF == -1, > 1, or glare planes >= 1
   // quarter res mono buffer (if glare planes >= 2)
   // vblur 1 (if glare planes >= 3)
   // vblur 2 (if glare planes >= 4)
   // vblur 3 (if glare planes >= 5)
   // vblur 4 (if glare planes >= 6)

   BP_GS_SetAlphaFunc(BP_GS_SetAlpha_Default);
   gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);

   CBaseTexture *draw = gpRenderBackend->GetCurrentDrawTexture();
   CBaseTexture *depth = gpRenderBackend->GetCurrentDrawDepth();

   ((CTexture*)depth)->SetWidth(draw->GetWidth());
   ((CTexture*)depth)->SetHeight(draw->GetHeight());

   // make sure we're using exact coords
   SRenderTarget rt = SRenderTarget(gpRenderBackend->GetCurrentDisplayTexture(), NULL, NULL, NULL, gpRenderBackend->GetCurrentDisplayDepth());
   rt.mPixelCentersHalf = true;
   gpRenderBackend->SetRenderTarget(rt);

   if (gVTADebugUberscaleShader)
   {
      FX::EShaderType shader;
      CBaseTexture *tex0, *tex1;

      CVector4 const uvOffset(0, 0, 0, 0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureFilter(1, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
      gpRenderBackend->SetTextureAddressMode(1, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

      switch (gVTADebugUberscaleShader)
      {
      default:
      case 1:
         // lowres
         shader = FX::kST_TextureRGB;
         tex0 = draw;
         tex1 = NULL;
         break;
      case 2:
         // lowres+msaa
         shader = FX::kST_TextureRGB_HalfresComposite;
         tex0 = draw;
         tex1 = FX::pHalfresTexture;
         break;
      case 3:
         // lowres
         shader = FX::kST_TextureRGB;
         tex0 = FX::pBlurHalfTarget;
         tex1 = NULL;
         break;
      case 4:
         // quarter
         shader = FX::kST_TextureRGB;
         tex0 = FX::pBlurTextures[2];
         tex1 = NULL;
         break;
      case 5:
         // qmono
         shader = FX::kST_TextureRGB;
         tex0 = FX::pBlurTextures[3];
         tex1 = NULL;
         break;
      case 6:
         // blur 1
         shader = FX::kST_TextureRGB;
         tex0 = FX::pBlurTextures[7];
         tex1 = NULL;
         break;
      case 7:
         // blur 2
         shader = FX::kST_TextureRGB;
         tex0 = FX::pBlurTextures[9];
         tex1 = NULL;
         break;
      case 8:
         // blur 3
         shader = FX::kST_TextureRGB;
         tex0 = FX::pBlurTextures[11];
         tex1 = NULL;
         break;
      case 9:
         // blur 4
         shader = FX::kST_TextureRGB;
         tex0 = FX::pBlurTextures[13];
         tex1 = NULL;
         break;
      }
      if (tex0)
         gpRenderBackend->SetTexture(0, tex0);
      if (tex1)
         gpRenderBackend->SetTexture(1, tex1);

      BP_SetFXShader(shader, NULL, 0);
      gpRenderBackend->RenderQuad(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
   }
   else
   {
      // DOF_Alpha if DOF != 0
      // DOF_Z if DOF > 0
      // GlareAlpha if Glare > 0
      CShaderParameterBufferAllocator::BeginRegion();
      CShaderParameterBuffer parameterBuffer = CShaderParameterBufferAllocator::AllocateParameterBuffer();

      UberscaleSetup(parameterBuffer, true);
      
      gpRenderBackend->SetDepthCompareEnabled(true);
      gpRenderBackend->SetDepthFunc(CRenderBackend::kDF_Always);
      gpRenderBackend->SetDepthWriteEnabled(true);
      {
         BPE_ADD_SCOPED_GPU_PROFILE_MARKER(CStringExtras::Stringize_s("Uber Half%d DOF%d Glare%d ALU%d", FX::gHalfresEnabled, FX::gDOFs, FX::gGlarePlanes, gVTAUseALUMono));
         
         gpRenderBackend->RenderQuadVC(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
                                       0.0f, 1.0f, 0.0f, 1.0f);

      }
      CShaderParameterBufferAllocator::EndRegion();
   }

   // reset back to PS2/DX9 conventions
   gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Nearest, CRenderBackend::kFM_Nearest);
   gpRenderBackend->SetRenderTarget(SRenderTarget(gpRenderBackend->GetCurrentDisplayTexture(), NULL, NULL, NULL, gpRenderBackend->GetCurrentDisplayDepth()));
   gpRenderBackend->SetDepthWriteEnabled(false);

   // Order of operations:
   // read base texture
   // if DOF > 0
   //   read depth
   //   compare, do A/1-A blend if pass for each of the DOF planes
   // if DOF < 0
   //   blend half & quarter with base at DOF_Alpha[0]
   // if glare >= 1
   //   sample half, convert to mono, modulate and add
   // if glare >= 2
   //   sample quarter mono, modulate and add
   // if glare >= 3
   //   sample blur1, modulate and add
   // if glare >= 4
   //   sample blur2, modulate and add
   // if glare >= 5
   //   sample blur3, modulate and add
   // if glare >= 6
   //   sample blur4, modulate and add
}

bool OffscreenWorkToDo()
{
   enum {
      kHalf, kQuarter, kQuarterMono, kBlur1, kBlur2, kBlur3, kBlur4
   };
   bool stage[7];

   stage[kHalf] = (FX::gGlarePlanes >= 1) || (FX::gDOFs >= 1) || (FX::gDOFs < 0);
   stage[kQuarter] = (FX::gGlarePlanes >= 2) || (FX::gDOFs >= 2) || (FX::gDOFs < 0);
   stage[kQuarterMono] = (FX::gGlarePlanes >= 2);
   stage[kBlur1] = (FX::gGlarePlanes >= 3);
   stage[kBlur2] = (FX::gGlarePlanes >= 4);
   stage[kBlur3] = (FX::gGlarePlanes >= 5);
   stage[kBlur4] = (FX::gGlarePlanes >= 6);

   return stage[kHalf] || stage[kQuarter] || stage[kBlur1] || stage[kBlur2] || stage[kBlur3] || stage[kBlur4];
}

// Quads
// 0 4 8  12
// 1 5 9  13
// 2 6 10 14
// 3 7 11 15

float gUberWidth;
float gUberHeight;

enum { kUV_Full, kUV_UL, kUV_UR, kUV_LL, kUV_LR };
enum { kSub_UL, kSub_UR, kSub_LL, kSub_LR };

void UberRenderQuad(int quad, bool half, int uv, int sub)
{
   float u0, u1, v0, v1;
   float x0, x1, y0, y1;
   float ws, hs;

   int x, y;
   x = quad >> 2;
   y = quad & 3;

   ws = gUberWidth;
   hs = gUberHeight;
   if (half)
   {
      ws *= 0.5f;
      hs *= 0.5f;
   }

   switch (uv)
   {
   default:
   case kUV_Full: 
      u0 = v0 = 0.0f; 
      u1 = v1 = 1.0f;
      break;
   case kUV_UL:
      u0 = v0 = 0.0f;
      u1 = v1 = 0.5f;
      break;
   case kUV_UR:
      u0 = 0.5f; u1 = 1.0f;
      v0 = 0.0f; v1 = 0.5f;
      break;
   case kUV_LL:
      u0 = 0.0f; u1 = 0.5f;
      v0 = 0.5f; v1 = 1.0f;
      break;
   case kUV_LR:
      u0 = 0.5f; u1 = 1.0f;
      v0 = 0.5f; v1 = 1.0f;
      break;
   }

   x0 = -1.0f + x * 0.5f;
   y1 = 1.0f - y * 0.5f;

   switch (sub)
   {
   default:
   case kSub_UL:
      x1 = x0 + ws;
      y0 = y1 - hs;
      break;
   case kSub_UR:
      x0 += ws;
      x1 = x0 + ws;
      y0 = y1 - hs;
      break;
   case kSub_LL:
      x1 = x0 + ws;
      y1 -= hs;
      y0 = y1 - hs;
      break;
   case kSub_LR:
      x0 += ws;
      x1 = x0 + ws;
      y1 -= hs;
      y0 = y1 - hs;
      break;
   }

   gpRenderBackend->RenderQuad(x0, x1, y0, y1, 1.0f, u0, u1, v0, v1);
}

void DoOffscreenVTAMadness()
{
   // render target is pointing at the 4x4 array of offscreen buffers
   // depending on the various uberscale knobs, the following sequence needs to happen
   // everything is optional, but if stage N is enabled, stages 0 through N are also enabled

   // determine the sizes for everything below
   CBaseTexture *drawtex = gpRenderBackend->GetCurrentDrawTexture();

   int w, h;
   int draw_width, draw_height;

   draw_width = drawtex->GetWidth();
   draw_height = drawtex->GetHeight();
   w = draw_width / 4;
   h = draw_height / 4;

   float wsize, hsize;
   wsize = (float)w / 512.0f; // sizes for positioning within the 1024x512 4x4 grid, values range from -1 to 1
   hsize = (float)h / 256.0f;

   gUberWidth = wsize;
   gUberHeight = hsize;

   enum {
      kHalf, kQuarter, kQuarterMono, kBlur1, kBlur2, kBlur3, kBlur4
   };
   bool stage[7];

   stage[kHalf] = (FX::gGlarePlanes >= 1) || (FX::gDOFs >= 1) || (FX::gDOFs < 0);
   stage[kQuarter] = (FX::gGlarePlanes >= 2) || (FX::gDOFs >= 2) || (FX::gDOFs < 0);
   stage[kQuarterMono] = (FX::gGlarePlanes >= 2);
   stage[kBlur1] = (FX::gGlarePlanes >= 3);
   stage[kBlur2] = (FX::gGlarePlanes >= 4);
   stage[kBlur3] = (FX::gGlarePlanes >= 5);
   stage[kBlur4] = (FX::gGlarePlanes >= 6);

   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   // Downscale the lowres buffer from 720x448 (or 408, or whatever) to 360x224 (or 204, or whatever), as 4 180x112 buffers down column 0
   if (stage[kHalf])
   {
      int i;
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:Uberscale Half");
      
      // Adjust the texture sizes to match, in case the debug menu options have been changed
      for (i = 0; i < 4; i++)
      {
         ((CTexture*)FX::pBlurTextures[i])->SetWidth(w);
         ((CTexture*)FX::pBlurTextures[i])->SetHeight(h);
      }

      gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
      gpRenderBackend->SetAlphaTestEnable(false);
#endif
      
      gpRenderBackend->SetDepthCompareEnabled(false);

      gpRenderBackend->SetTexture(0, drawtex);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

      CVector4 const uvOffset(0.0f / draw_width, 0.0f / draw_height, 0, 0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);

      if (FX::gHalfresEnabled && FX::pHalfresTexture)
      {
         gpRenderBackend->SetTexture(1, FX::pHalfresTexture);
         gpRenderBackend->SetTextureFilter(1, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
         gpRenderBackend->SetTextureAddressMode(1, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
         BP_SetFXShader(FX::kST_TextureRGB_HalfresComposite, NULL, 0);
      }
      else
         BP_SetFXShader(FX::kST_TextureRGB, NULL, 0);

      // We're rendering to the halfres blur target alias right now
      gpRenderBackend->RenderQuad(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
      // switch over to the macrotiled blur target
      SRenderTarget brt = SRenderTarget(FX::pBlurTarget, NULL, NULL, NULL, NULL);
      brt.mPixelCentersHalf = true;
      gpRenderBackend->SetRenderTarget(brt);
   }
   // Downscale those 4 into a quarter res 180x112 buffer at the top of column 1
   if (stage[kQuarter])
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:Uberscale Quarter");

      // adjust size of quarter tex appropriately
      ((CTexture*)FX::pBlurTextures[4])->SetWidth(w);
      ((CTexture*)FX::pBlurTextures[4])->SetHeight(h);

      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

      CVector4 const uvOffset(0.0f / w, 0.0f / h, 0, 0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);

      gpRenderBackend->SetTexture(0, FX::pBlurHalfTarget);
      BP_SetFXShader(FX::kST_TextureRGB, NULL, 0);
      UberRenderQuad(2, false, kUV_Full, kSub_UL);
   }
   // Copy that buffer to the next one in the second column with the glare mono filter
   if (stage[kQuarterMono])
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:Uberscale Quarter Mono");

      // adjust size of quarter tex appropriately
      ((CTexture*)FX::pBlurTextures[5])->SetWidth(w);
      ((CTexture*)FX::pBlurTextures[5])->SetHeight(h);

      gpRenderBackend->SetTexture(0, FX::pBlurTextures[2]);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

      gpRenderBackend->SetTexture(1, FX::pGlareMonoPalette);
      gpRenderBackend->SetTextureFilter(1, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(1, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

      CVector4 const uvOffset(0.0f / w, 0.0f / h, 0, 0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);

      CShaderParameterBufferAllocator::BeginRegion();

      CShaderParameterBuffer parameterBuffer = CShaderParameterBufferAllocator::AllocateParameterBuffer();

      CVector4 const weights(0.296875f, 0.5859375f, 0.109375f, 0.0f);
      parameterBuffer.AddParameter(FX::gMonochromeWeights, weights);
      if (gVTAUseALUMono)
      {
         parameterBuffer.AddParameter(FX::gMonoScale0, *(CVector4*)FX::gGlareMonoScale0);
         parameterBuffer.AddParameter(FX::gMonoScale1, *(CVector4*)FX::gGlareMonoScale1);
         parameterBuffer.AddParameter(FX::gMonoOfs0, *(CVector4*)FX::gGlareMonoOfs0);
         parameterBuffer.AddParameter(FX::gMonoOfs1, *(CVector4*)FX::gGlareMonoOfs1);
      }
      CShaderParameterBufferAllocator::FinalizeParameterBuffer(parameterBuffer);

      if (gVTAUseALUMono)
         BP_SetFXShader(FX::kST_GlareMono_ALU, parameterBuffer.GetStartAddress(), parameterBuffer.GetSize());
      else
         BP_SetFXShader(FX::kST_GlareMono, parameterBuffer.GetStartAddress(), parameterBuffer.GetSize());

      UberRenderQuad(3, false, kUV_Full, kSub_UL);         
   }

   float actualSourceWidth = w;
   float actualSourceHeight = h;
   
   real32 const kOffsets[] = { 0.5f, -1.0f, 2.0f, -3.0f, 4.0f, -5.0f, 6.0f };
   
   CVector4 const uvOffsetX0(kOffsets[0] / actualSourceWidth, 0.0f / actualSourceHeight, 0.0f,                              0.0f);
   CVector4 const uvOffsetX1(kOffsets[1] / actualSourceWidth, 0.0f / actualSourceHeight, kOffsets[2] / actualSourceWidth, 0.0f / actualSourceHeight);
   CVector4 const uvOffsetX2(kOffsets[3] / actualSourceWidth, 0.0f / actualSourceHeight, kOffsets[4] / actualSourceWidth, 0.0f / actualSourceHeight);
   CVector4 const uvOffsetX3(kOffsets[5] / actualSourceWidth, 0.0f / actualSourceHeight, kOffsets[6] / actualSourceWidth, 0.0f / actualSourceHeight);
   
   CVector4 const uvOffsetY0(0.0f / actualSourceWidth, kOffsets[0] / actualSourceHeight, 0.0f,                       0.0f);
   CVector4 const uvOffsetY1(0.0f / actualSourceWidth, kOffsets[1] / actualSourceHeight, 0.0f / actualSourceWidth, kOffsets[2] / actualSourceHeight);
   CVector4 const uvOffsetY2(0.0f / actualSourceWidth, kOffsets[3] / actualSourceHeight, 0.0f / actualSourceWidth, kOffsets[4] / actualSourceHeight);
   CVector4 const uvOffsetY3(0.0f / actualSourceWidth, kOffsets[5] / actualSourceHeight, 0.0f / actualSourceWidth, kOffsets[6] / actualSourceHeight);
   
   float weight0 = gGlareNewBlurWeights[0];
   float weight12 = gGlareNewBlurWeights[1];
   float weight34 = gGlareNewBlurWeights[2];
   float weight56 = gGlareNewBlurWeights[3];
   
   float passScale = 1.0f / (gGlareNewBlurWeights[0] + 2*gGlareNewBlurWeights[1] + 2*gGlareNewBlurWeights[2] + 2*gGlareNewBlurWeights[3]);
   CVector4 const scale(weight0 * passScale, weight12 * passScale, weight34 * passScale, weight56 * passScale);
   gpRenderBackend->SetVertexRegisters(FX::kReg_Scale, 1, &scale);   
   
   // Apply the 2-pass gaussian to that in the next two buffers, result in buffer 7
   if (stage[kBlur1])
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:Uberscale Blur 1");

      ((CTexture*)FX::pBlurTextures[6])->SetWidth(w);
      ((CTexture*)FX::pBlurTextures[6])->SetHeight(h);
      ((CTexture*)FX::pBlurTextures[7])->SetWidth(w);
      ((CTexture*)FX::pBlurTextures[7])->SetHeight(h);

      gpRenderBackend->SetTexture(0, FX::pBlurTextures[3]);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
      
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffsetX0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffsetX1);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset2, 1, &uvOffsetX2);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset3, 1, &uvOffsetX3);
      
      BP_SetFXShader(FX::kST_GlareBlur, NULL, 0);
      
      UberRenderQuad(6, false, kUV_Full, kSub_UL);
      
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffsetY0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffsetY1);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset2, 1, &uvOffsetY2);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset3, 1, &uvOffsetY3);
      
      gpRenderBackend->SetTexture(0, FX::pBlurTextures[6]);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
      
      UberRenderQuad(7, false, kUV_Full, kSub_UL);
   }
   // Apply the 2-pass gaussian to that in the first two buffers of column 2, result in buffer 9
   if (stage[kBlur2])
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:Uberscale Blur 2");

      ((CTexture*)FX::pBlurTextures[8])->SetWidth(w);
      ((CTexture*)FX::pBlurTextures[8])->SetHeight(h);
      ((CTexture*)FX::pBlurTextures[9])->SetWidth(w);
      ((CTexture*)FX::pBlurTextures[9])->SetHeight(h);

      gpRenderBackend->SetTexture(0, FX::pBlurTextures[7]);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
      
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffsetX0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffsetX1);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset2, 1, &uvOffsetX2);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset3, 1, &uvOffsetX3);
      
      BP_SetFXShader(FX::kST_GlareBlur, NULL, 0);
      
      UberRenderQuad(8, false, kUV_Full, kSub_UL);
      
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffsetY0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffsetY1);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset2, 1, &uvOffsetY2);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset3, 1, &uvOffsetY3);
      
      gpRenderBackend->SetTexture(0, FX::pBlurTextures[8]);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
      
      UberRenderQuad(9, false, kUV_Full, kSub_UL);
   }
   // Apply the 2-pass gaussian to that in the next two buffers of column 2, result in buffer 11
   if (stage[kBlur3])
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:Uberscale Blur 3");

      ((CTexture*)FX::pBlurTextures[10])->SetWidth(w);
      ((CTexture*)FX::pBlurTextures[10])->SetHeight(h);
      ((CTexture*)FX::pBlurTextures[11])->SetWidth(w);
      ((CTexture*)FX::pBlurTextures[11])->SetHeight(h);

      gpRenderBackend->SetTexture(0, FX::pBlurTextures[9]);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
      
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffsetX0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffsetX1);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset2, 1, &uvOffsetX2);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset3, 1, &uvOffsetX3);
      
      BP_SetFXShader(FX::kST_GlareBlur, NULL, 0);
      
      UberRenderQuad(10, false, kUV_Full, kSub_UL);
      
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffsetY0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffsetY1);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset2, 1, &uvOffsetY2);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset3, 1, &uvOffsetY3);
      
      gpRenderBackend->SetTexture(0, FX::pBlurTextures[10]);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
      
      UberRenderQuad(11, false, kUV_Full, kSub_UL);
   }
   // Apply the 2-pass gaussian to that in the first two buffers of column 3, result in buffer 13
   if (stage[kBlur4])
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:Uberscale Blur 4");

      ((CTexture*)FX::pBlurTextures[12])->SetWidth(w);
      ((CTexture*)FX::pBlurTextures[12])->SetHeight(h);
      ((CTexture*)FX::pBlurTextures[13])->SetWidth(w);
      ((CTexture*)FX::pBlurTextures[13])->SetHeight(h);

      gpRenderBackend->SetTexture(0, FX::pBlurTextures[11]);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
      
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffsetX0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffsetX1);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset2, 1, &uvOffsetX2);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset3, 1, &uvOffsetX3);
      
      BP_SetFXShader(FX::kST_GlareBlur, NULL, 0);
      
      UberRenderQuad(12, false, kUV_Full, kSub_UL);
      
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffsetY0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffsetY1);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset2, 1, &uvOffsetY2);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset3, 1, &uvOffsetY3);
      
      gpRenderBackend->SetTexture(0, FX::pBlurTextures[12]);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
      
      UberRenderQuad(13, false, kUV_Full, kSub_UL);
   }
}

#endif

void BP_PostFx_MGS3_GlareNew(void* pData)
{
   if (gFXShowPasses)
      BP_DebugText_Print("Glare");
#if BP_VITA
   // just restore the original target
   static const int kMaxPlanes = 6;// This matches MAX_PLANE from glare_new.c
   SBP_PFX_GlareNew* pPacket = (SBP_PFX_GlareNew*)pData;
   int numPlanes = pPacket->maxPlanes;
   numPlanes = bpe::min_val( pPacket->maxPlanes, kMaxPlanes );
   numPlanes = bpe::max_val( numPlanes, 0 );

   // The PS2 glare code sets the test reg, and does not reset it to their standard afterwards
   // some of the survival viewer rendering depends on this
   BP_GS_SetTest(SCE_GS_SET_TEST( 1, 0, 0, 1, 0, 0, 1, 1 ));
   if (gVTADebugUberscale)
   {
      if (!gGlareNewEnable)
         FX::gGlarePlanes = 0;
      else
         FX::gGlarePlanes = numPlanes;
      FX::gGlareAlpha = (pPacket->alpha / 128.0f) * gGlareNewBloomScale;
      gpRenderBackend->OverrideUpscale();

      bool mono0, mono1;
      if ((pPacket->scale0.vx == 0.0f) && 
          (pPacket->scale0.vy == 0.0f) && 
          (pPacket->scale0.vz == 0.0f))
         mono0 = false;
      else
         mono0 = true;
      if ((pPacket->scale1.vx == 0.0f) && 
          (pPacket->scale1.vy == 0.0f) && 
          (pPacket->scale1.vz == 0.0f))
         mono1 = false;
      else
         mono1 = true;

      if (mono0 && mono1)
      {
         memcpy(FX::gGlareMonoScale0, &pPacket->scale0, sizeof(pPacket->scale0));
         memcpy(FX::gGlareMonoScale1, &pPacket->scale1, sizeof(pPacket->scale1));
         memcpy(FX::gGlareMonoOfs0, &pPacket->ofs0, sizeof(pPacket->ofs0));
         memcpy(FX::gGlareMonoOfs1, &pPacket->ofs1, sizeof(pPacket->ofs1));
         FX::gGlareMonoType = 2;
      }
      else if (mono0)
      {
         memcpy(FX::gGlareMonoScale0, &pPacket->scale0, sizeof(pPacket->scale0));
         memcpy(FX::gGlareMonoOfs0, &pPacket->ofs0, sizeof(pPacket->ofs0));
         FX::gGlareMonoType = 1;
      }
      else if (mono1)
      {
         memcpy(FX::gGlareMonoScale0, &pPacket->scale1, sizeof(pPacket->scale1));
         memcpy(FX::gGlareMonoOfs0, &pPacket->ofs1, sizeof(pPacket->ofs1));
      }
      // this probably needs to be done elsewhere, since we can't count on glare always being turned on
      return;
   }

   if (!gGlareNewEnable)
   {
      //gpRenderBackend->SetRenderTarget(FX::gMainRenderTarget);
      FX::gGlarePlanes = 0;
      return;
   }

   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   if( numPlanes == 0 )
   {
      return;
   }

   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:Glare Blur");
   
      gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);

      gpRenderBackend->SetRenderTarget(SRenderTarget(FX::pBlurTarget, NULL, NULL, NULL, NULL));

#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
      gpRenderBackend->SetAlphaTestEnable(false);
#endif
      
      gpRenderBackend->SetDepthCompareEnabled(false);
      
      float actualSourceWidth = 180.0f;
      float actualSourceHeight = 112.0f;
      
      real32 const kOffsets[] = { 0.5f, -1.0f, 2.0f, -3.0f, 4.0f, -5.0f, 6.0f };
      
      CVector4 const uvOffsetX0(kOffsets[0] / actualSourceWidth, 0.5f / actualSourceHeight, 0.0f,                              0.0f);
      CVector4 const uvOffsetX1(kOffsets[1] / actualSourceWidth, 0.5f / actualSourceHeight, kOffsets[2] / actualSourceWidth, 0.5f / actualSourceHeight);
      CVector4 const uvOffsetX2(kOffsets[3] / actualSourceWidth, 0.5f / actualSourceHeight, kOffsets[4] / actualSourceWidth, 0.5f / actualSourceHeight);
      CVector4 const uvOffsetX3(kOffsets[5] / actualSourceWidth, 0.5f / actualSourceHeight, kOffsets[6] / actualSourceWidth, 0.5f / actualSourceHeight);

      CVector4 const uvOffsetY0(0.5f / actualSourceWidth, kOffsets[0] / actualSourceHeight, 0.0f,                       0.0f);
      CVector4 const uvOffsetY1(0.5f / actualSourceWidth, kOffsets[1] / actualSourceHeight, 0.5f / actualSourceWidth, kOffsets[2] / actualSourceHeight);
      CVector4 const uvOffsetY2(0.5f / actualSourceWidth, kOffsets[3] / actualSourceHeight, 0.5f / actualSourceWidth, kOffsets[4] / actualSourceHeight);
      CVector4 const uvOffsetY3(0.5f / actualSourceWidth, kOffsets[5] / actualSourceHeight, 0.5f / actualSourceWidth, kOffsets[6] / actualSourceHeight);
   
      float weight0 = gGlareNewBlurWeights[0];
      float weight12 = gGlareNewBlurWeights[1];
      float weight34 = gGlareNewBlurWeights[2];
      float weight56 = gGlareNewBlurWeights[3];

      float passScale = 1.0f / (gGlareNewBlurWeights[0] + 2*gGlareNewBlurWeights[1] + 2*gGlareNewBlurWeights[2] + 2*gGlareNewBlurWeights[3]);
      CVector4 const scale(weight0 * passScale, weight12 * passScale, weight34 * passScale, weight56 * passScale);
      gpRenderBackend->SetVertexRegisters(FX::kReg_Scale, 1, &scale);   

      if (numPlanes >= 2)
      {
         gpRenderBackend->SetTexture(0, FX::pBlurTextures[0]);
         gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
         gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
      
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffsetX0);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffsetX1);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset2, 1, &uvOffsetX2);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset3, 1, &uvOffsetX3);
         
         BP_SetFXShader(FX::kST_GlareBlur, NULL, 0);
         
         gpRenderBackend->RenderQuad(
            -1.0f, -1.0f+180.0f/512.0f,
            0.5f-112.0f/256.0f, 0.5f,
            1.0f,
            0.0f, 1.0f, 0.0f, 1.0f);
         
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffsetY0);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffsetY1);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset2, 1, &uvOffsetY2);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset3, 1, &uvOffsetY3);
         
         gpRenderBackend->SetTexture(0, FX::pBlurTextures[1]);
         gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
         gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
         
         gpRenderBackend->RenderQuad(
            -1.0f, -1.0f+180.0f/512.0f,
            0.0f-112.0f/256.0f, 0.0f,
            1.0f,
            0.0f, 1.0f, 0.0f, 1.0f);
      }

      if (numPlanes >= 3)
      {
         gpRenderBackend->SetTexture(0, FX::pBlurTextures[2]);
         gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
         gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffsetX0);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffsetX1);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset2, 1, &uvOffsetX2);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset3, 1, &uvOffsetX3);
         
         BP_SetFXShader(FX::kST_GlareBlur, NULL, 0);
         
         gpRenderBackend->RenderQuad(
            -1.0f, -1.0f+180.0f/512.0f,
            -0.5f-112.0f/256.0f, -0.5f,
            1.0f,
            0.0f, 1.0f, 0.0f, 1.0f);
         
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffsetY0);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffsetY1);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset2, 1, &uvOffsetY2);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset3, 1, &uvOffsetY3);
         
         gpRenderBackend->SetTexture(0, FX::pBlurTextures[3]);
         gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
         gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
         
         gpRenderBackend->RenderQuad(
            -0.5f, -0.5f+180.0f/512.0f,
            1.0f-112.0f/256.0f, 1.0f,
            1.0f,
            0.0f, 1.0f, 0.0f, 1.0f);
      }

      if (numPlanes >= 4)
      {
         gpRenderBackend->SetTexture(0, FX::pBlurTextures[4]);
         gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
         gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffsetX0);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffsetX1);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset2, 1, &uvOffsetX2);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset3, 1, &uvOffsetX3);
         
         BP_SetFXShader(FX::kST_GlareBlur, NULL, 0);
         
         gpRenderBackend->RenderQuad(
            -0.5f, -0.5f+180.0f/512.0f,
            0.5f-112.0f/256.0f, 0.5f,
            1.0f,
            0.0f, 1.0f, 0.0f, 1.0f);
         
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffsetY0);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffsetY1);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset2, 1, &uvOffsetY2);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset3, 1, &uvOffsetY3);
         
         gpRenderBackend->SetTexture(0, FX::pBlurTextures[5]);
         gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
         gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
         
         gpRenderBackend->RenderQuad(
            -0.5f, -0.5f+180.0f/512.0f,
            0.0f-112.0f/256.0f, 0.0f,
            1.0f,
            0.0f, 1.0f, 0.0f, 1.0f);
      }

      if (numPlanes >= 5)
      {
         gpRenderBackend->SetTexture(0, FX::pBlurTextures[6]);
         gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
         gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffsetX0);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffsetX1);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset2, 1, &uvOffsetX2);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset3, 1, &uvOffsetX3);
         
         BP_SetFXShader(FX::kST_GlareBlur, NULL, 0);
         
         gpRenderBackend->RenderQuad(
            -0.5f, -0.5f+180.0f/512.0f,
            -0.5f-112.0f/256.0f, -0.5f,
            1.0f,
            0.0f, 1.0f, 0.0f, 1.0f);
         
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffsetY0);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffsetY1);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset2, 1, &uvOffsetY2);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset3, 1, &uvOffsetY3);
         
         gpRenderBackend->SetTexture(0, FX::pBlurTextures[7]);
         gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
         gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
         
         gpRenderBackend->RenderQuad(
            0.0f, 0.0f+180.0f/512.0f,
            1.0f-112.0f/256.0f, 1.0f,
            1.0f,
            0.0f, 1.0f, 0.0f, 1.0f);
      }

      if (numPlanes >= 6)
      {
         gpRenderBackend->SetTexture(0, FX::pBlurTextures[8]);
         gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
         gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffsetX0);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffsetX1);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset2, 1, &uvOffsetX2);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset3, 1, &uvOffsetX3);
         
         BP_SetFXShader(FX::kST_GlareBlur, NULL, 0);
         
         gpRenderBackend->RenderQuad(
            0.0f, 0.0f+180.0f/512.0f,
            0.5f-112.0f/256.0f, 0.5f,
            1.0f,
            0.0f, 1.0f, 0.0f, 1.0f);
         
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffsetY0);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset1, 1, &uvOffsetY1);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset2, 1, &uvOffsetY2);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset3, 1, &uvOffsetY3);
         
         gpRenderBackend->SetTexture(0, FX::pBlurTextures[9]);
         gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
         gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
         
         gpRenderBackend->RenderQuad(
            0.0f, 0.0f+180.0f/512.0f,
            0.0f-112.0f/256.0f, 0.0f,
            1.0f,
            0.0f, 1.0f, 0.0f, 1.0f);
      }

      gpRenderBackend->SetRenderTarget(FX::gMainRenderTarget);
   }
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:Glare Apply");
   
      gpRenderBackend->SetDepthCompareEnabled(false);
      
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_One, CRenderBackend::kBF_One);

      CShaderParameterBufferAllocator::BeginRegion();
      
      CVector4 const uvOffset(0.5f / 180.0f, 0.5f / 112.0f, 0, 0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);
      
      real32 const alphaScale = (pPacket->alpha / 128.0f) * gGlareNewBloomScale;
      CVector4 const modulate(alphaScale, alphaScale, alphaScale, 1.0f);
      
      CShaderParameterBuffer parameterBuffer = CShaderParameterBufferAllocator::AllocateParameterBuffer();
      parameterBuffer.AddParameter(FX::gModulateColor, modulate);
      CShaderParameterBufferAllocator::FinalizeParameterBuffer(parameterBuffer);

      for (int i = 0; i < numPlanes; i++)
      {
         gpRenderBackend->SetTexture(i, FX::pBlurTextures[i*2]);
         gpRenderBackend->SetTextureFilter(i, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
         gpRenderBackend->SetTextureAddressMode(i, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
      }
      BP_SetFXShader(FX::EShaderType(FX::kST_TextureRGBModulate+(numPlanes-1)), parameterBuffer.GetStartAddress(), parameterBuffer.GetSize());
      gpRenderBackend->RenderFullscreenQuad(1.0f);
      CShaderParameterBufferAllocator::EndRegion();
      gpRenderBackend->SetDepthCompareEnabled(true);
   }

#endif
#ifndef BP_RENDER_SINGLE_RENDER_TARGET
   static const int kMaxPlanes = 6;// This matches MAX_PLANE from glare_new.c

   //BP_TODO First pass implementation, need to generate all the different render targets and apply them all in a single full screen pass for increased performance.
   if( !gGlareNewEnable )
      return;

   BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:GlareNew");

   SBP_PFX_GlareNew* pPacket = (SBP_PFX_GlareNew*)pData;

   int numPlanes = pPacket->maxPlanes;
   numPlanes = bpe::min_val( pPacket->maxPlanes, kMaxPlanes );
   numPlanes = bpe::max_val( numPlanes, 0 );
   if( numPlanes == 0 )
   {
      return;
   }

   real32 const alphaScale = (pPacket->alpha / 128.0f) * gGlareNewBloomScale;
   CVector4 const modulateColor(alphaScale, alphaScale, alphaScale, 1.0f);

   SRenderTarget const originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();
   CBaseTexture* pOriginalRenderTargetTexture = originalRenderTarget.mpColorBuffer[0];

   CBaseTexture* pMonoEffectInput = BP_GetRenderTarget(kRT_MonoMini);

   real32 const kBloomBorderX = (gGlareNewBorderSize / 512.0f) * 2.0f;
   real32 const kBloomBorderY = (gGlareNewBorderSize / 448.0f) * 2.0f;

   gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);

   // 1) Draw border
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass1");
      gpRenderBackend->SetRenderTarget(SRenderTarget(pMonoEffectInput, NULL, NULL, NULL, NULL));

      BP_PostFx_MGS3_GlareNew_Border(kBloomBorderX, kBloomBorderY);
      gpRenderBackend->ResolveRenderTarget(pMonoEffectInput);
   }

   CBaseTexture* pQuarterBuf0 = BP_GetRenderTarget(kRT_TempBufferQuarter1);
   CBaseTexture* pQuarterBuf1 = BP_GetRenderTarget(kRT_TempBufferQuarter2);
   CBaseTexture* pQuarterBuf2 = BP_GetRenderTarget(kRT_TempBufferQuarter3);
   CBaseTexture* pQuarterBuf3 = BP_GetRenderTarget(kRT_TempBufferQuarter4);
   CBaseTexture* pQuarterBuf4 = BP_GetRenderTarget(kRT_TempBufferQuarter5);
   CBaseTexture* pQuarterBufTempWork = BP_GetRenderTarget(kRT_TempBufferQuarterTemp);

   // 2) Downsample, draw border
   if( numPlanes >= 2 )
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass2");
      BP_Downsample(pMonoEffectInput, pQuarterBuf0);

      gpRenderBackend->SetRenderTarget(SRenderTarget(pQuarterBuf0, NULL, NULL, NULL, NULL));
      BP_PostFx_MGS3_GlareNew_Border(kBloomBorderX, kBloomBorderY);
      gpRenderBackend->ResolveRenderTarget(pQuarterBuf0);
   }

   // Determine blur parameters
   real32 const totalWeight = gGlareNewBlurWeights[0] + 2*gGlareNewBlurWeights[1] + 2*gGlareNewBlurWeights[2] + 2*gGlareNewBlurWeights[3];
   real32 const oneOverTotalWeight = 1.0f / totalWeight;

   // 3) Blur, draw border
   if( numPlanes >= 3 )
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass3");
      BP_Blur(pQuarterBuf0, pQuarterBuf1, pQuarterBufTempWork, gGlareNewBlurWeights[0], gGlareNewBlurWeights[1], gGlareNewBlurWeights[2], gGlareNewBlurWeights[3], oneOverTotalWeight, oneOverTotalWeight, false);
      BP_PostFx_MGS3_GlareNew_Border(kBloomBorderX, kBloomBorderY);
      gpRenderBackend->ResolveRenderTarget(pQuarterBuf1);
   }

   // 4) Blur, draw border
   if( numPlanes >= 4 )
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass4");
      BP_Blur(pQuarterBuf1, pQuarterBuf2, pQuarterBufTempWork, gGlareNewBlurWeights[0], gGlareNewBlurWeights[1], gGlareNewBlurWeights[2], gGlareNewBlurWeights[3], oneOverTotalWeight, oneOverTotalWeight, false);
      BP_PostFx_MGS3_GlareNew_Border(kBloomBorderX, kBloomBorderY);
      gpRenderBackend->ResolveRenderTarget(pQuarterBuf2);
   }

   // 5) Blur, draw border
   if( numPlanes >= 5 )
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass5");
      BP_Blur(pQuarterBuf2, pQuarterBuf3, pQuarterBufTempWork, gGlareNewBlurWeights[0], gGlareNewBlurWeights[1], gGlareNewBlurWeights[2], gGlareNewBlurWeights[3], oneOverTotalWeight, oneOverTotalWeight, false);
      BP_PostFx_MGS3_GlareNew_Border(kBloomBorderX, kBloomBorderY);
      gpRenderBackend->ResolveRenderTarget(pQuarterBuf3);
   }

   // 6) Blur, draw border
   if( numPlanes >= 6 )
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass6");
      BP_Blur(pQuarterBuf3, pQuarterBuf4, pQuarterBufTempWork, gGlareNewBlurWeights[0], gGlareNewBlurWeights[1], gGlareNewBlurWeights[2], gGlareNewBlurWeights[3], oneOverTotalWeight, oneOverTotalWeight, false);
      BP_PostFx_MGS3_GlareNew_Border(kBloomBorderX, kBloomBorderY);
      gpRenderBackend->ResolveRenderTarget(pQuarterBuf4);
   }

   const CBaseTexture* pAdditionalTextures[] = { pMonoEffectInput, pQuarterBuf0, pQuarterBuf1, pQuarterBuf2, pQuarterBuf3, pQuarterBuf4 };

   // apply all to screen in one pass
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Apply to screen");
      gpRenderBackend->SetRenderTarget(originalRenderTarget);
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_One, CRenderBackend::kBF_One, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      BP_DrawFullscreenTextureModulateWithXTextures(pAdditionalTextures, numPlanes, modulateColor, 1 /*isFilter*/);
   }

   gpRenderBackend->ResolveRenderTarget(pOriginalRenderTargetTexture);
#endif
}
#endif //MGS_VERSION == 3

#if MGS_VERSION == 3

int gGhostSmokeEnable = 1;

inline void BP_PostFX_MGS3_GhostSmoke_ApplyToScreen(SRenderTarget const & applyTarget, CBaseTexture* source, CVector4 const & modulateColor)
{
#ifndef BP_RENDER_SINGLE_RENDER_TARGET

   gpRenderBackend->SetRenderTarget(applyTarget);
   BP_GS_SetAlpha( DG_ALPHA_MODE_ADD );
   // texture color * vertex color, NOTE: special case: vertex color = 2.0
   // TODO: we might need a custom shader to handle alpha * 4 being added to the dst buffer, test this before checking in
   BP_DrawFullscreenTextureModulate( source, modulateColor, 1 /*isFilter*/, 1 /*isRGBA*/ );
#endif
}

inline void BP_PostFX_MGS3_GhostSmoke_Pass0Plus(CBaseTexture* halfTarget, CBaseTexture* source, SRenderTarget const & applyTarget, CVector4 const & modulateColor, real32 borderX, real32 borderY)
{
#if BP_POSTFX_DISABLE
   return;
#endif

   BP_GS_SetAlpha( SCE_GS_SET_ALPHA( 0, 1, 2, 1, 128 ) );

   gpRenderBackend->SetRenderTarget(SRenderTarget(halfTarget, NULL, NULL, NULL, NULL));
   gpRenderBackend->SetTexture(0, source);

   BP_Downsample2x_Internal(halfTarget, source);
   //BP_PostFx_MGS3_GlareNew_Border(borderX, borderY);
   gpRenderBackend->ResolveRenderTarget(halfTarget);

   BP_PostFX_MGS3_GhostSmoke_ApplyToScreen(applyTarget, halfTarget, modulateColor);
}

void BP_PostFX_MGS3_GhostSmoke(void* pData)
{
   if (gFXShowPasses)
      BP_DebugText_Print("GhostSmoke");
#ifndef BP_RENDER_SINGLE_RENDER_TARGET

   //BP_TODO First pass implementation, need to generate all the different render targets and apply them all in a single full screen pass for increased performance.
   if( !gGhostSmokeEnable )
      return;

   BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:GhostSmoke");

   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);
   gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   gpRenderBackend->SetDepthWriteEnabled(false);
   gpRenderBackend->SetDepthCompareEnabled(false);

   SRenderTarget const originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();

   CVector4 modulateColor(2, 2, 2, 4);
   real32 const kBorderX = (gGlareNewBorderSize / 512.0f) * 2.0f;
   real32 const kBorderY = (gGlareNewBorderSize / 448.0f) * 2.0f;

   // 0) Shrink with special alpha test, Draw border, apply to screen
   CBaseTexture* pHalfBuffer = BP_GetRenderTarget(kRT_TempBufferHalf1);
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass0");
      // TODO: PS2 was using special over brightened 255 value, we actually we need to get the prim in ghost_smoke.c to update the stencil buffer
      // also right now the alpha values are getting overwritten by prims from fog_gas.c, this is not allowing the effect to work at all!
      gpRenderBackend->SetAlphaFunc(CRenderBackend::kAF_Equal, 255);
      gpRenderBackend->SetAlphaTestEnable(true);
      BP_GS_SetAlpha( SCE_GS_SET_ALPHA( 0, 1, 2, 1, 128 ) );

      gpRenderBackend->ResolveRenderTarget(originalRenderTarget.mpColorBuffer[0]);
      gpRenderBackend->SetRenderTarget(SRenderTarget(pHalfBuffer, NULL, NULL, NULL, NULL));
      gpRenderBackend->Clear(CRenderBackend::kFlag_Color, CColor::Zero());
      gpRenderBackend->SetTexture(0, originalRenderTarget.mpColorBuffer[0]);

      BP_Downsample2x_Internal(originalRenderTarget.mpColorBuffer[0], pHalfBuffer);
      gpRenderBackend->SetAlphaTestEnable(false);
      //BP_PostFx_MGS3_GlareNew_Border(kBorderX, kBorderY);
      gpRenderBackend->ResolveRenderTarget(pHalfBuffer);

      BP_PostFX_MGS3_GhostSmoke_ApplyToScreen(originalRenderTarget, pHalfBuffer, modulateColor);
   }

   CBaseTexture* pQuarterBuf = BP_GetRenderTarget(kRT_TempBufferQuarter1);
   CBaseTexture* pBuf8th = BP_GetRenderTarget(kRT_TempBuffer8th);
   CBaseTexture* pBuf16th = BP_GetRenderTarget(kRT_TempBuffer16th);
   CBaseTexture* pBuf32th = BP_GetRenderTarget(kRT_TempBuffer32th);
   CBaseTexture* pBuf64th = BP_GetRenderTarget(kRT_TempBuffer64th);
   CBaseTexture* pBuf128th = BP_GetRenderTarget(kRT_TempBuffer128th);
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass1");
      BP_PostFX_MGS3_GhostSmoke_Pass0Plus(pQuarterBuf, pHalfBuffer, originalRenderTarget, modulateColor, kBorderX, kBorderY);
   }
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass2");
      BP_PostFX_MGS3_GhostSmoke_Pass0Plus(pBuf8th, pQuarterBuf, originalRenderTarget, modulateColor, kBorderX, kBorderY);
   }
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass3");
      BP_PostFX_MGS3_GhostSmoke_Pass0Plus(pBuf16th, pBuf8th, originalRenderTarget, modulateColor, kBorderX, kBorderY);
   }
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass4");
      BP_PostFX_MGS3_GhostSmoke_Pass0Plus(pBuf32th, pBuf16th, originalRenderTarget, modulateColor, kBorderX, kBorderY);
   }
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass5");
      BP_PostFX_MGS3_GhostSmoke_Pass0Plus(pBuf64th, pBuf32th, originalRenderTarget, modulateColor, kBorderX, kBorderY);
   }
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Pass6");
      BP_PostFX_MGS3_GhostSmoke_Pass0Plus(pBuf128th, pBuf64th, originalRenderTarget, modulateColor, kBorderX, kBorderY);
   }
#endif
}

#endif

//----------------------------------------------------------------------------
#if MGS_VERSION == 2
int gFarFocusEnable = 1;
int gFarFocusBlurEnable = 0;
real32 gFarFocusBlurWeights[4] = { 4.0f, 0.2f, 0.05f, 0.01f };
int gFarFocusMaxPlaneCount = 1;

#elif MGS_VERSION == 3
int gFarFocusEnable = 1;
int gFarFocusBlurEnable = 0;
real32 gFarFocusBlurWeights[4] = { 2.0f, 0.15f, 0.05f, 0.01f };

#endif

int gFarFocusVisualize = 0;
int gFarFocusStats = 0;

#if MGS_VERSION == 2

#if BP_VITA
static const char *uberscale_names[] =
{
   "Uberscale Rescale",
   "Uberscale DOF1",
   "Uberscale DOF2",
   "Uberscale Scope",
   "Uberscale       Glare1",
   "Uberscale DOF1  Glare1",
   "Uberscale DOF2  Glare1",
   "Uberscale SCOPE Glare1",
   "Uberscale       Glare2",
   "Uberscale DOF1  Glare2",
   "Uberscale DOF2  Glare2",
   "Uberscale SCOPE Glare2",
   "Uberscale       Glare3",
   "Uberscale DOF1  Glare3",
   "Uberscale DOF2  Glare3",
   "Uberscale SCOPE Glare3",
   "Uberscale       Glare4",
   "Uberscale DOF1  Glare4",
   "Uberscale DOF2  Glare4",
   "Uberscale SCOPE Glare4",
   "Uberscale       Glare5",
   "Uberscale DOF1  Glare5",
   "Uberscale DOF2  Glare5",
   "Uberscale SCOPE Glare5",
   "Uberscale       Glare6",
   "Uberscale DOF1  Glare6",
   "Uberscale DOF2  Glare6",
   "Uberscale SCOPE Glare6",
};

//extern int gVTADebugOptCmfHalfresFilter;

void UberscaleSetup(CShaderParameterBuffer &parameterBuffer, bool writeDepth)
{
   CBaseTexture *draw = gpRenderBackend->GetCurrentDrawTexture();
   CBaseTexture *depth = gpRenderBackend->GetCurrentDrawDepth();

   if (FX::gDOFs)
      parameterBuffer.AddParameter(FX::gPS_DOF_Alpha, *(CVector4*)FX::gDOF_Alpha);
   if (FX::gDOFs > 0)
      parameterBuffer.AddParameter(FX::gPS_DOF_Z, *(CVector4*)FX::gDOF_Z);
   if (FX::gGlarePlanes > 0)
      parameterBuffer.AddParameter(FX::gPS_GlareAlpha, FX::gGlareAlpha);

   if (!FX::gUberContrast)
   {
      FX::gUberContrastParam[0] = 0.0f;
      FX::gUberContrastParam[1] = 0.0f;
      FX::gUberContrastParam[2] = 0.0f;
      FX::gUberContrastParam[3] = 1.0f;
   }

   parameterBuffer.AddParameter(FX::gContrastParam, *(CVector4*)FX::gUberContrastParam);
   parameterBuffer.AddParameter(FX::gTweakParam, * reinterpret_cast<CVector4 const *>( &FX::gUberTweakKnobs ));

   CShaderParameterBufferAllocator::FinalizeParameterBuffer(parameterBuffer);

   // set base texture
   gpRenderBackend->SetTexture(0, draw);
   gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

   // set depth texture if needed
   if (FX::gHalfresEnabled || (FX::gDOFs > 0) || writeDepth)
   {
      gpRenderBackend->SetTexture(1, depth);
      gpRenderBackend->SetTextureFilter(1, CRenderBackend::kFM_Nearest, CRenderBackend::kFM_Nearest);
      gpRenderBackend->SetTextureAddressMode(1, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   }
   // set quarter if needed
   if ((FX::gDOFs > 1) || (FX::gDOFs < 0) || (FX::gGlarePlanes >= 2))
   {
      gpRenderBackend->SetTexture(3, FX::pBlurTextures[4]);
      gpRenderBackend->SetTextureFilter(3, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(3, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   }
   // set lowres
   if (FX::gHalfresEnabled)
   {
      gpRenderBackend->SetTexture(10, FX::pHalfresTexture);
      gpRenderBackend->SetTextureFilter(10, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(10, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

      //gpRenderBackend->SetTexture(11, FX::pHalfresDepth);
      //gpRenderBackend->SetTextureFilter(11, CRenderBackend::kFM_Nearest, CRenderBackend::kFM_Nearest);
      //gpRenderBackend->SetTextureAddressMode(11, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   }

   if (FX::gDOFs || FX::gGlarePlanes)
   {
      gpRenderBackend->SetTexture(2, FX::pBlurHalfTarget);
      gpRenderBackend->SetTextureFilter(2, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(2, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   }

   BP_BeginShader(FX::gpUberShader[FX::gHalfresEnabled][(FX::gDOFs >= 0) ? FX::gDOFs : 3][FX::gGlarePlanes][0][writeDepth], parameterBuffer.GetStartAddress(), parameterBuffer.GetSize());
}

void UberscaleVTA()
{
   // Shader needs:

   // lowres target
   // lowres depth (if DOF > 0)
   // glare mono palette (if glare planes >= 1)
   // half res buffer (if any fullscreen is enabled)
   // quarter res buffer (if DOF == -1, > 1, or glare planes >= 1

   BP_GS_SetAlphaFunc(BP_GS_SetAlpha_Default);
   gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);

   // ignore DOF for now
   CBaseTexture *draw = gpRenderBackend->GetCurrentDrawTexture();
   CBaseTexture *depth = gpRenderBackend->GetCurrentDrawDepth();

   ((CTexture*)depth)->SetWidth(draw->GetWidth());
   ((CTexture*)depth)->SetHeight(draw->GetHeight());

   // make sure we're using exact coords
   SRenderTarget rt = SRenderTarget(gpRenderBackend->GetCurrentDisplayTexture(), NULL, NULL, NULL, gpRenderBackend->GetCurrentDisplayDepth());
   rt.mPixelCentersHalf = true;
   gpRenderBackend->SetRenderTarget(rt);

   // DOF_Alpha if DOF != 0
   // DOF_Z if DOF > 0
   // GlareAlpha if Glare > 0
   CShaderParameterBufferAllocator::BeginRegion();
   CShaderParameterBuffer parameterBuffer = CShaderParameterBufferAllocator::AllocateParameterBuffer();

   UberscaleSetup(parameterBuffer, true);

   gpRenderBackend->SetDepthCompareEnabled(true);
   gpRenderBackend->SetDepthFunc(CRenderBackend::kDF_Always);
   gpRenderBackend->SetDepthWriteEnabled(true);
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER(CStringExtras::Stringize_s("Uber Half%d DOF%d Glare%d", FX::gHalfresEnabled, FX::gDOFs, FX::gGlarePlanes));
      
      gpRenderBackend->RenderQuadVC(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
                                    0.0f, 1.0f, 0.0f, 1.0f);
      CShaderParameterBufferAllocator::EndRegion();
   }

   // reset back to PS2/DX9 conventions
   gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Nearest, CRenderBackend::kFM_Nearest);
   gpRenderBackend->SetRenderTarget(SRenderTarget(gpRenderBackend->GetCurrentDisplayTexture(), NULL, NULL, NULL, gpRenderBackend->GetCurrentDisplayDepth()));
   gpRenderBackend->SetDepthWriteEnabled(false);

   // Order of operations:
   // read base texture
   // if DOF > 0
   //   read depth
   //   compare, do A/1-A blend if pass for each of the DOF planes
   // if DOF < 0
   //   blend half & quarter with base at DOF_Alpha[0]
}

bool OffscreenWorkToDo()
{
   enum {
      kHalf, kQuarter, kQuarterMono, kBlur1, kBlur2, kBlur3, kBlur4
   };
   bool stage[7];

   stage[kHalf] = (FX::gGlarePlanes >= 1) || (FX::gDOFs >= 1) || (FX::gDOFs < 0);
   stage[kQuarter] = (FX::gGlarePlanes >= 2) || (FX::gDOFs >= 2) || (FX::gDOFs < 0);

   return stage[kHalf] || stage[kQuarter];
}

float gUberWidth;
float gUberHeight;

enum { kUV_Full, kUV_UL, kUV_UR, kUV_LL, kUV_LR };
enum { kSub_UL, kSub_UR, kSub_LL, kSub_LR };

void UberRenderQuad(int quad, bool half, int uv, int sub)
{
   float u0, u1, v0, v1;
   float x0, x1, y0, y1;
   float ws, hs;

   int x, y;
   x = quad >> 2;
   y = quad & 3;

   ws = gUberWidth;
   hs = gUberHeight;
   if (half)
   {
      ws *= 0.5f;
      hs *= 0.5f;
   }

   switch (uv)
   {
   default:
   case kUV_Full: 
      u0 = v0 = 0.0f; 
      u1 = v1 = 1.0f;
      break;
   case kUV_UL:
      u0 = v0 = 0.0f;
      u1 = v1 = 0.5f;
      break;
   case kUV_UR:
      u0 = 0.5f; u1 = 1.0f;
      v0 = 0.0f; v1 = 0.5f;
      break;
   case kUV_LL:
      u0 = 0.0f; u1 = 0.5f;
      v0 = 0.5f; v1 = 1.0f;
      break;
   case kUV_LR:
      u0 = 0.5f; u1 = 1.0f;
      v0 = 0.5f; v1 = 1.0f;
      break;
   }

   x0 = -1.0f + x * 0.5f;
   y1 = 1.0f - y * 0.5f;

   switch (sub)
   {
   default:
   case kSub_UL:
      x1 = x0 + ws;
      y0 = y1 - hs;
      break;
   case kSub_UR:
      x0 += ws;
      x1 = x0 + ws;
      y0 = y1 - hs;
      break;
   case kSub_LL:
      x1 = x0 + ws;
      y1 -= hs;
      y0 = y1 - hs;
      break;
   case kSub_LR:
      x0 += ws;
      x1 = x0 + ws;
      y1 -= hs;
      y0 = y1 - hs;
      break;
   }

   gpRenderBackend->RenderQuad(x0, x1, y0, y1, 1.0f, u0, u1, v0, v1);
}

void DoOffscreenVTAMadness()
{
   // render target is pointing at the 4x4 array of offscreen buffers
   // depending on the various uberscale knobs, the following sequence needs to happen
   // everything is optional, but if stage N is enabled, stages 0 through N are also enabled

   // determine the sizes for everything below
   CBaseTexture *drawtex = gpRenderBackend->GetCurrentDrawTexture();

   int w, h;
   int draw_width, draw_height;

   draw_width = drawtex->GetWidth();
   draw_height = drawtex->GetHeight();
   w = draw_width / 4;
   h = draw_height / 4;

   float wsize, hsize;
   wsize = (float)w / 512.0f; // sizes for positioning within the 1024x512 4x4 grid, values range from -1 to 1
   hsize = (float)h / 256.0f;

   gUberWidth = wsize;
   gUberHeight = hsize;

   enum {
      kHalf, kQuarter, kQuarterMono, kBlur1, kBlur2, kBlur3, kBlur4
   };
   bool stage[7];

   stage[kHalf] = (FX::gGlarePlanes >= 1) || (FX::gDOFs >= 1) || (FX::gDOFs < 0);
   stage[kQuarter] = (FX::gGlarePlanes >= 2) || (FX::gDOFs >= 2) || (FX::gDOFs < 0);
   stage[kQuarterMono] = (FX::gGlarePlanes >= 2);
   stage[kBlur1] = (FX::gGlarePlanes >= 3);
   stage[kBlur2] = (FX::gGlarePlanes >= 4);
   stage[kBlur3] = (FX::gGlarePlanes >= 5);
   stage[kBlur4] = (FX::gGlarePlanes >= 6);

   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   // Downscale the lowres buffer from 720x448 (or 408, or whatever) to 360x224 (or 204, or whatever), as 4 180x112 buffers down column 0
   if (stage[kHalf])
   {
      int i;
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:Uberscale Half");
      
      // Adjust the texture sizes to match, in case the debug menu options have been changed
      for (i = 0; i < 4; i++)
      {
         ((CTexture*)FX::pBlurTextures[i])->SetWidth(w);
         ((CTexture*)FX::pBlurTextures[i])->SetHeight(h);
      }

      gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
      gpRenderBackend->SetAlphaTestEnable(false);
#endif
      
      gpRenderBackend->SetDepthCompareEnabled(false);

      gpRenderBackend->SetTexture(0, drawtex);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

      CVector4 const uvOffset(1.0f / draw_width, 1.0f / draw_height, 0, 0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);

      if (FX::gHalfresEnabled && FX::pHalfresTexture)
      {
         gpRenderBackend->SetTexture(1, FX::pHalfresTexture);
         gpRenderBackend->SetTextureFilter(1, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
         gpRenderBackend->SetTextureAddressMode(1, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
         BP_SetFXShader(FX::kST_TextureRGB_HalfresComposite, NULL, 0);
      }
      else
         BP_SetFXShader(FX::kST_TextureRGB, NULL, 0);

      // We're rendering to the halfres blur target alias right now
      gpRenderBackend->RenderQuad(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
      // switch over to the macrotiled blur target
      SRenderTarget brt = SRenderTarget(FX::pBlurTarget, NULL, NULL, NULL, NULL);
      brt.mPixelCentersHalf = true;
      gpRenderBackend->SetRenderTarget(brt);
   }
   // Downscale those 4 into a quarter res 180x112 buffer at the top of column 1
   if (stage[kQuarter])
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:Uberscale Quarter");

      // adjust size of quarter tex appropriately
      ((CTexture*)FX::pBlurTextures[4])->SetWidth(w);
      ((CTexture*)FX::pBlurTextures[4])->SetHeight(h);

      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

      CVector4 const uvOffset(0.0f / w, 0.0f / h, 0, 0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);

      gpRenderBackend->SetTexture(0, FX::pBlurHalfTarget);
      BP_SetFXShader(FX::kST_TextureRGB, NULL, 0);
      UberRenderQuad(2, false, kUV_Full, kSub_UL);
   }
}
#endif

void BP_PostFx_MGS2_FarFocus(void* pData)
{
   SBP_PFX_FarFocus* pPacket = (SBP_PFX_FarFocus*)pData;
   if (gFXShowPasses)
   {
      BP_DebugText_Print("DoF FarFocus");
      BP_DebugText_Print("Planes %d, near/far (%f/%f)", pPacket->max_plane, pPacket->focus_near, pPacket->focus_far);
   }
#if BP_VITA
   // disable if we aren't doing the uberscale. defaults to off for now, as the half res screen is visibly indistinguishable
   if( !gFarFocusEnable || (pPacket->focus_near <= pPacket->focus_far) || !gVTADebugUberscale )
   {
      FX::gDOFs = 0;
      return;
   }

   FX::gDOFs = 0;
   FX::gDOF_Z[0] = pPacket->focus_near; // + (pPacket->focus_far - pPacket->focus_near) * i / (pPacket->max_plane - 1); // max planes is capped at 1 for now
   FX::gDOF_Alpha[0] = 1.0f;
   FX::gDOF_Z[1] = pPacket->focus_near; // + (pPacket->focus_far - pPacket->focus_near) * i / (pPacket->max_plane - 1); // max planes is capped at 1 for now
   FX::gDOF_Alpha[1] = 1.0f;
   gpRenderBackend->OverrideUpscale();

   BP_PostFx_Offscreen(NULL); // trigger the big offscreen pass
#endif
#ifndef BP_RENDER_SINGLE_RENDER_TARGET

   if( !gFarFocusEnable )
      return;

   // NOTE: Remember that z = 1.0f at near clip plane for this test to make sense.
   if( pPacket->focus_near <= pPacket->focus_far )
      return;

   BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:FarFocus");

   // Store original render target
   SRenderTarget const originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();

   // Resolve current frame buffer to be used as texture input for downsampling.
   CBaseTexture* pCurrentRenderTarget = BP_GetRenderTarget(kRT_CurrentFrameBuffer_NoMSAA);
   gpRenderBackend->ResolveRenderTarget(pCurrentRenderTarget);

   CBaseTexture* pTempBuffers[2] = { BP_GetRenderTarget(kRT_TempBufferHalf1), BP_GetRenderTarget(kRT_TempBufferHalf2) };

   // Downsample current render target
   BP_Downsample(pCurrentRenderTarget, pTempBuffers[0]);
   gpRenderBackend->ResolveRenderTarget(pTempBuffers[0]);

   // Pass 2: Apply downsampled textures to frame buffer!
   {
      gpRenderBackend->SetAlphaTestEnable(false);
      gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetDepthFunc(CRenderBackend::kDF_GEqual);
      gpRenderBackend->SetDepthWriteEnabled(false);
   
      int const maxPlane = bpe::min_val((int)pPacket->max_plane, gFarFocusMaxPlaneCount);

      // Restore original render target
      gpRenderBackend->SetRenderTarget(originalRenderTarget);

      for( int i = 0; i < maxPlane; ++i )
      {
         real32 const z = pPacket->focus_near + (pPacket->focus_far - pPacket->focus_near) * i / (pPacket->max_plane - 1);

         if ( z <= 0.0f )
            continue;

         if ( z >= 1.0f )
            continue;

         // Blur
         if( gFarFocusBlurEnable )
         {
            // Determine blur parameters
            real32 const totalWeight = gFarFocusBlurWeights[0] + 2*gFarFocusBlurWeights[1] + 2*gFarFocusBlurWeights[2] + 2*gFarFocusBlurWeights[3];
            real32 const oneOverTotalWeight = 1.0f / totalWeight;

            gpRenderBackend->SetDepthCompareEnabled(false);

            // Blur
            BP_Blur(pTempBuffers[0], pTempBuffers[0], pTempBuffers[1], gFarFocusBlurWeights[0], gFarFocusBlurWeights[1], gFarFocusBlurWeights[2], gFarFocusBlurWeights[3], oneOverTotalWeight, oneOverTotalWeight, true);

            // Restore original render target
            gpRenderBackend->SetRenderTarget(originalRenderTarget);
         }

         // Apply

         gpRenderBackend->SetDepthCompareEnabled(true);

         if( !gFarFocusVisualize )
         {
            BP_DrawFullscreenTexture(pTempBuffers[0], 1 /*isFilter*/, 0 /*isRGBA*/, z );
         }
         else
         {
            real32 const nearFarFactor = (real32)i / (pPacket->max_plane - 1);
            real32 const visualizeScale = 1.0f - nearFarFactor;

            CVector4 const modColor(visualizeScale, visualizeScale, visualizeScale, 1);
            BP_DrawFullscreenTextureModulate(pTempBuffers[0], modColor, 1 /*isFilter*/, 0 /*isRGBA*/, z );
         }
      }
   }

   if( gFarFocusStats )
   {
      BP_DebugText_Print("FarFocus: Max_Plane: %d", pPacket->max_plane);
   }

#endif
}

#elif MGS_VERSION == 3

void BP_PostFx_MGS3_FarFocus(void* pData)
{
#if BP_VITA
   SBP_PFX_FarFocus* pPacket = (SBP_PFX_FarFocus*)pData;

   if (gFXShowPasses)
   {
      BP_DebugText_Print("DoF FarFocus");

   
      BP_DebugText_Print("DoF near/far: %9.6f/%9.6f", pPacket->focus_near, pPacket->focus_far);
      BP_DebugText_Print("Planes: %d, alpha %d", pPacket->max_plane, pPacket->alpha);
   }

   if (!gFarFocusEnable)
   {
      FX::gDOFs = 0;
      return;
   }
   
   int const kMaxFocusFill = 3;
   if (pPacket->max_plane > kMaxFocusFill)
   {
      FX::gDOFs = 2;
   }
   else
   {
      FX::gDOFs = 1;
   }

   for (int i = 0; i < pPacket->max_plane; i += kMaxFocusFill)
   {
      float z = pPacket->focus_near + (pPacket->focus_far - pPacket->focus_near) * i / (pPacket->max_plane - 1);

      if (z <= 0.0f)
         continue;
      if (z >= 1.0f)
         continue;

      {
         float layer = pPacket->alpha / 128.0f;
         float rem = 1.0f;
         for (int j = i; (j < pPacket->max_plane) && j < (i + kMaxFocusFill); ++j)
            rem = rem * (1.0f - layer);
         FX::gDOF_Alpha[i/kMaxFocusFill] = 1.0f - rem;
         FX::gDOF_Z[i/kMaxFocusFill] = z;
      }
   }
   gpRenderBackend->OverrideUpscale();
   return;
#endif
#ifndef BP_RENDER_SINGLE_RENDER_TARGET

   if( !gFarFocusEnable )
      return;

   SBP_PFX_FarFocus* pPacket = (SBP_PFX_FarFocus*)pData;

   // NOTE: Remember that z = 1.0f at near clip plane for this test to make sense.
   if( pPacket->focus_near <= pPacket->focus_far )
      return;

   if( pPacket->alpha == 0 )
      return;

   BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:FarFocus");

   // Store original render target
   SRenderTarget const originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();

   // Get Rects based on viewports
   SRect srcRect_FullResolution;
   SRect srcRect_HalfResolution;
   SRect srcRect_QuaterResoltuion;
   if( originalRenderTarget.mRenderTargetViewPort.mViewportEnabled )
   {
      srcRect_FullResolution.x1 = originalRenderTarget.mRenderTargetViewPort.mViewportX;
      srcRect_FullResolution.x2 = originalRenderTarget.mRenderTargetViewPort.mViewportX + originalRenderTarget.mRenderTargetViewPort.mViewportWidth;
      srcRect_FullResolution.y1 = originalRenderTarget.mRenderTargetViewPort.mViewportY;
      srcRect_FullResolution.y2 = originalRenderTarget.mRenderTargetViewPort.mViewportY + originalRenderTarget.mRenderTargetViewPort.mViewportHeight;
   }
   else
   {
      srcRect_FullResolution.x1 = 0.0f;
      srcRect_FullResolution.x2 = originalRenderTarget.mpColorBuffer[0]->GetWidth();
      srcRect_FullResolution.y1 = 0.0f;
      srcRect_FullResolution.y2 = originalRenderTarget.mpColorBuffer[0]->GetHeight();
   }

   {
      srcRect_HalfResolution.MultiplyInto(srcRect_FullResolution, 0.5f);
      srcRect_QuaterResoltuion.MultiplyInto(srcRect_HalfResolution, 0.5f);
   }
   SRect* aSampledSRect[] = { &srcRect_FullResolution, &srcRect_HalfResolution, &srcRect_QuaterResoltuion };
   SRect* aDownSampledSRect[] = { &srcRect_HalfResolution, &srcRect_QuaterResoltuion };

   // Resolve current frame buffer to be used as texture input for downsampling.
   CBaseTexture* pCurrentRenderTarget = BP_GetRenderTarget(kRT_CurrentFrameBuffer_NoMSAA);
   gpRenderBackend->ResolvePartialRenderTarget(pCurrentRenderTarget, srcRect_FullResolution);

   // Clamp max plane count if above supported range!
   int const kMaxFocus = 4;
   int const kMaxFocusFill = 3;
   int maxPlane = pPacket->max_plane;
   {
      int const kMaxPlaneCount = kMaxFocusFill * 2;
      if( maxPlane > kMaxPlaneCount)
      {
         maxPlane = kMaxPlaneCount;
#if MARCO
         BP_DebugText_Print("FarFocus: Requires too many downsampled buffers, let Marco know (Max Plane: %d)!", pPacket->max_plane);
#endif
      }
   }

   // Pass 1: Create down sample textures
   CBaseTexture* pDownsampledTextures[2];
   {
      // Downsample current render target
      pDownsampledTextures[0] = BP_GetRenderTarget(kRT_TempBufferHalf1);
      BP_CopyTextureSRect(pCurrentRenderTarget, pDownsampledTextures[0], *aSampledSRect[0], *aDownSampledSRect[0]);
      gpRenderBackend->ResolvePartialRenderTarget(pDownsampledTextures[0], *aDownSampledSRect[0]);

      if( maxPlane > kMaxFocusFill )
      {
         // Downsample one more time
         pDownsampledTextures[1] = BP_GetRenderTarget(kRT_TempBufferQuarter1);
         BP_CopyTextureSRect(pDownsampledTextures[0], pDownsampledTextures[1], *aSampledSRect[1], *aDownSampledSRect[1]);
         gpRenderBackend->ResolvePartialRenderTarget(pDownsampledTextures[1], *aDownSampledSRect[1]);

         // Apply optional blur to second downsample!
         if( gFarFocusBlurEnable )
         {
            // Determine blur parameters
            real32 const totalWeight = gFarFocusBlurWeights[0] + 2*gFarFocusBlurWeights[1] + 2*gFarFocusBlurWeights[2] + 2*gFarFocusBlurWeights[3];
            real32 const oneOverTotalWeight = 1.0f / totalWeight;

            // Blur, NOTE: Temp buffer must match in dimension!
            CBaseTexture* pTempBuffer = BP_GetRenderTarget(kRT_TempBufferQuarter2);
            BP_BlurSRect(pDownsampledTextures[1], pDownsampledTextures[1], pTempBuffer, gFarFocusBlurWeights[0], gFarFocusBlurWeights[1], gFarFocusBlurWeights[2], gFarFocusBlurWeights[3], oneOverTotalWeight, oneOverTotalWeight, true, *aSampledSRect[1], *aDownSampledSRect[1]);
         }
      }
   }

   // Restore original render target
   gpRenderBackend->SetRenderTarget(originalRenderTarget);

   // Pass 2: Apply downsampled textures to frame buffer!
   {
      gpRenderBackend->SetAlphaTestEnable(false);
      gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_InvSrcAlpha, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One);
      gpRenderBackend->SetDepthCompareEnabled(true);
      gpRenderBackend->SetDepthFunc(CRenderBackend::kDF_GEqual);
      gpRenderBackend->SetDepthWriteEnabled(false);

      int currentSourceTexture = 0;

      for( int i = 0; i < maxPlane; i += kMaxFocusFill, ++currentSourceTexture )
      {
         real32 const z = pPacket->focus_near + (pPacket->focus_far - pPacket->focus_near) * i / (maxPlane - 1);
         
         if ( z <= 0.0f )
            continue;

         if ( z >= 1.0f )
            continue;

         CVector4 modulateColor(1, 1, 1, 1);

         if( gFarFocusVisualize )
         {
            switch( currentSourceTexture )
            {
            case 0:
               modulateColor = CVector4(1.0f, 0.5f, 0.5f, 1);
               break;
            case 1:
               modulateColor = CVector4(0.5f, 1.0f, 0.5f, 1);
               break;
            }
         }

         real32 const minAlpha = pPacket->alpha / 128.0f;
         real32 maxAlpha;

         // Accumulate alpha of all sub layers to determine max layer alpha
         {
            real32 const layerAlpha = pPacket->alpha / 128.0f;
            real32 remainingAlpha = 1.0f;
            for( int j = i; (j < maxPlane) && j < (i + kMaxFocusFill); ++j )
            {
               remainingAlpha = remainingAlpha * (1.0f - layerAlpha);
            }

            maxAlpha = 1.0f - remainingAlpha;
         }

         //TODO: For now just draw a single quad at the nearest blend, later on need to smoothly blend between minAlpha and maxAlpha based on depth buffer!
         modulateColor.SetW(maxAlpha);

         BP_DrawFullscreenTextureModulateSRect(pDownsampledTextures[currentSourceTexture], modulateColor, 1 /*isFilter*/, 0 /*isRGBA*/, *aDownSampledSRect[currentSourceTexture], z );
      }
   }

   if( gFarFocusStats )
   {
      BP_DebugText_Print("FarFocus: Alpha: %d Max_Plane: %d", pPacket->alpha, pPacket->max_plane);
   }
#endif
}
#endif //MGS_VERSION == 3

#if MGS_VERSION == 2

void BP_PostFX_MGS2_VRClear(void* pData)
{
   SBP_PFX_VRClear* pPacket = (SBP_PFX_VRClear*)pData;
   if (gFXShowPasses)
   {
      BP_DebugText_Print("VRClear %f", pPacket->percentage);
   }

#if BP_VITA
   if ( pPacket->percentage > 0.0f )
   {
      // apply grayscale lerp effect to screen, in-place
      gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

      gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);

      gpRenderBackend->SetDepthCompareEnabled(false);

      CVector4 const scale(pPacket->percentage, pPacket->percentage, pPacket->percentage, pPacket->percentage);
      gpRenderBackend->SetVertexRegisters(FX::kReg_Scale, 1, &scale);

      BP_SetFXShader(FX::kST_GrayScale, NULL, 0);
      gpRenderBackend->RenderFullscreenQuad(1.0f);
      gpRenderBackend->SetDepthCompareEnabled(true);
   }
#endif
#ifndef BP_RENDER_SINGLE_RENDER_TARGET

   // get current frame buffer, draw to frame buffer as grayscale

   if( pPacket->percentage > 0.0f )
   {

      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:VRClear");

      // Store original render target
      SRenderTarget const originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();

      CBaseTexture* pInputTexture = BP_GetRenderTarget(kRT_TempBuffer_NoMSAA);
   #if BPE_TARGET == BPE_TARGET_X360
      // On 360 we can resolve the current render target directly into a different texture.
      gpRenderBackend->ResolveRenderTarget(pInputTexture);
   #else
      // On other platforms we render directly into the textures, so we have to copy it from the current texture!
      BP_CopyTexture(BP_GetRenderTarget(kRT_CurrentFrameBuffer_NoMSAA), pInputTexture);
   #endif
      
      // Restore original render target
      gpRenderBackend->SetRenderTarget(originalRenderTarget);

      // Apply grayscale
      {
         gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

         gpRenderBackend->SetTexture(0, pInputTexture);
         gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Nearest, CRenderBackend::kFM_Nearest);
         gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

         gpRenderBackend->SetAlphaTestEnable(false);
         gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);

         gpRenderBackend->SetDepthCompareEnabled(false);

         CVector4 const uvOffset(0.5f / pInputTexture->GetWidth(), 0.5f / pInputTexture->GetHeight(), 0, 0);
         gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);

         CVector4 const scale(pPacket->percentage, pPacket->percentage, pPacket->percentage, pPacket->percentage);
         gpRenderBackend->SetVertexRegisters(FX::kReg_Scale, 1, &scale);

         BP_SetFXShader(FX::kST_GrayScale, NULL, 0);

         gpRenderBackend->RenderFullscreenQuad(1.0f);

         gpRenderBackend->SetDepthCompareEnabled(true);
      }
   }

#endif
}

#endif //MGS_VERSION == 2

#if MGS_VERSION == 2

int gCodexInOutEnabled = 1;

void BP_PostFX_MGS2_CodexInOut(void* pData)
{
   if (!gCodexInOutEnabled)
      return;

   if (gFXShowPasses)
      BP_DebugText_Print("CodexInOut");
   BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:CodexInOut");

#if BP_VITA
   SBP_PFX_CodexInOut* pPacket = (SBP_PFX_CodexInOut*)pData;

   CBaseTexture* Screen = gpRenderBackend->GetLowresDrawTexture();
   
   BP_SetFXShader(FX::kST_GrayScaleTex, NULL, 0);
   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);
   gpRenderBackend->SetTexture(0, Screen);
   gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);
   
   if(pPacket->needFaceBoxes)
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("ZBuffer Setup");
      //overwrite codec faces z value so that full screen gray zoom does not overwrite
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One);
      gpRenderBackend->SetDepthWriteEnabled(true);

      gpRenderBackend->RenderQuad(
         pPacket->faceBoxes[0][0], pPacket->faceBoxes[0][1], 
         pPacket->faceBoxes[0][2], pPacket->faceBoxes[0][3], 
         1.0f,
         0.0f, 0.0f,
         0.0f, 0.0f);
      gpRenderBackend->RenderQuad(
         pPacket->faceBoxes[1][0], pPacket->faceBoxes[1][1], 
         pPacket->faceBoxes[1][2], pPacket->faceBoxes[1][3], 
         1.0f,
         0.0f, 0.0f,
         0.0f, 0.0f);

   }
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER(CStringExtras::Stringize_s("Shrink Zoom (%f,%f,%f,%f)", 
                                                                   pPacket->uMinMax_vMinMax[0], pPacket->uMinMax_vMinMax[1],
                                                                   pPacket->uMinMax_vMinMax[2], pPacket->uMinMax_vMinMax[3]));

      gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);

      CVector4 const uvOffset(0.5f / Screen->GetWidth(), 0.5f / Screen->GetHeight(), 0, 0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);
      gpRenderBackend->SetVertexRegisters(FX::kReg_Scale, 1, (CVector4*)&pPacket->colorWeight);

      gpRenderBackend->RenderQuad(
         -1.0f, 1.0f, 
         -1.0f, 1.0f, 
         0.9f,
         pPacket->uMinMax_vMinMax[0], pPacket->uMinMax_vMinMax[1],
         pPacket->uMinMax_vMinMax[2], pPacket->uMinMax_vMinMax[3]);

      gpRenderBackend->SetDepthWriteEnabled(false);
   }
#endif
#ifndef BP_RENDER_SINGLE_RENDER_TARGET


   SBP_PFX_CodexInOut* pPacket = (SBP_PFX_CodexInOut*)pData;
   CBaseTexture* pCodexTexture = BP_GetRenderTarget(kRT_TempBufferHalf1);
   SRenderTarget const originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();
   if(pPacket->needPreviousFrameCopy)
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("CopyFrame");
      
      BP_CopyTexture(BP_GetRenderTarget(kRT_PreviousFrameBuffer_NoMSAA), pCodexTexture);
      gpRenderBackend->ResolveRenderTarget(pCodexTexture);
      gpRenderBackend->SetRenderTarget(originalRenderTarget);
   }
   BP_SetFXShader(FX::kST_GrayScale, NULL, 0);
   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

   gpRenderBackend->SetTexture(0, pCodexTexture);
   gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

   gpRenderBackend->SetAlphaTestEnable(false);
   if(pPacket->needFaceBoxes)
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("ZBuffer Setup");
      //overwrite codec faces z value so that full screen gray zoom does not overwrite
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One);
      gpRenderBackend->SetDepthWriteEnabled(true);

      gpRenderBackend->RenderQuad(
         pPacket->faceBoxes[0][0], pPacket->faceBoxes[0][1], 
         pPacket->faceBoxes[0][2], pPacket->faceBoxes[0][3], 
         1.0f,
         0.0f, 0.0f,
         0.0f, 0.0f);
      gpRenderBackend->RenderQuad(
         pPacket->faceBoxes[1][0], pPacket->faceBoxes[1][1], 
         pPacket->faceBoxes[1][2], pPacket->faceBoxes[1][3], 
         1.0f,
         0.0f, 0.0f,
         0.0f, 0.0f);

   }
   {
      BPE_ADD_SCOPED_GPU_PROFILE_MARKER("Shrink Zoom");

      gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);

      CVector4 const uvOffset(0.5f / originalRenderTarget.mpColorBuffer[0]->GetWidth(), 0.5f / originalRenderTarget.mpColorBuffer[0]->GetHeight(), 0, 0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);
      gpRenderBackend->SetVertexRegisters(FX::kReg_Scale, 1, (CVector4*)&pPacket->colorWeight);

      gpRenderBackend->RenderQuad(
         -1.0f, 1.0f, 
         -1.0f, 1.0f, 
         0.9f,
         pPacket->uMinMax_vMinMax[0], pPacket->uMinMax_vMinMax[1],
         pPacket->uMinMax_vMinMax[2], pPacket->uMinMax_vMinMax[3]);

      gpRenderBackend->SetDepthWriteEnabled(false);
   }

#endif
}

void BP_PostFx_MGS2_ReduceScreen(void* pData)
{
   if (gFXShowPasses)
      BP_DebugText_Print("ReduceScreen");
   
#if BP_VITA
   SBP_ReduceScreen* pPacket = (SBP_ReduceScreen*)pData;

   gpRenderBackend->BackupScene(FX::pOffscreenBuffer);

   gpRenderBackend->SetTexture(0, FX::pOffscreenBuffer);
   gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

   CVector4 const uvOffset(0.5f / FX::pOffscreenBuffer->GetWidth(), 0.5f / FX::pOffscreenBuffer->GetHeight(), 0.0f, 0.0f);
   gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);

   CVector4 modulate = BP_DecodeColorF(pPacket->color);
   modulate *= 2.0f;

   CShaderParameterBufferAllocator::BeginRegion();

   CShaderParameterBuffer parameterBuffer = CShaderParameterBufferAllocator::AllocateParameterBuffer();
   parameterBuffer.AddParameter(FX::gModulateColor, modulate);
   CShaderParameterBufferAllocator::FinalizeParameterBuffer(parameterBuffer);

   BP_SetFXShader(FX::kST_TextureRGB, NULL, 0);

   gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
   gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);

   float x[2], y[2];
   x[0] = pPacket->x1 / 512.0f;
   x[1] = pPacket->x2 / 512.0f;
   y[0] = pPacket->y1 / 448.0f;
   y[1] = pPacket->y2 / 448.0f;

   x[0] = 2.0f * x[0] - 1.0f;
   x[1] = 2.0f * x[1] - 1.0f;
   y[0] = -2.0f * (y[0] - 0.5f);
   y[1] = -2.0f * (y[1] - 0.5f);
   gpRenderBackend->RenderQuad(
      x[0], x[1], y[1], y[0],
      1.0f,
      0.0f, 1.0f, 0.0f, 1.0f);
   CShaderParameterBufferAllocator::EndRegion();
#endif
}

#endif //MGS_VERSION == 2

//----------------------------------------------------------------------------

#if MGS_VERSION == 3

float gLensflareScale = 0.5f;
int gLensflareVis = 0;

void BP_PostFx_MGS3_LensflareNew(void* pData)
{
#if BP_VITA
   if (gFXShowPasses)
   {
      BP_DebugText_Print("Lensflare");
      BP_DebugText_Print("0: %7d / %7d", gpRenderBackend->GetVisibility(0,0), gpRenderBackend->GetVisibility(0,1));
      BP_DebugText_Print("1: %7d / %7d", gpRenderBackend->GetVisibility(1,0), gpRenderBackend->GetVisibility(1,1));
      BP_DebugText_Print("2: %7d / %7d", gpRenderBackend->GetVisibility(2,0), gpRenderBackend->GetVisibility(2,1));
   }

   SBP_LensflareNew *p = (SBP_LensflareNew*)pData;
   
   float t, b, l, r;

   l = (float)p->u0 / DRAW_WIDTH;
   t = (float)p->v0 / DRAW_HEIGHT;
   r = (float)p->u1 / DRAW_WIDTH;
   b = (float)p->v1 / DRAW_HEIGHT;
   l = (l - 0.5f) * 2.0f;
   r = (r - 0.5f) * 2.0f;
   t = (t - 0.5f) * -2.0f;
   b = (b - 0.5f) * -2.0f;

   // render mask
   gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One);
   gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);
   gpRenderBackend->SetDepthWriteEnabled(false);
   gpRenderBackend->SetDepthCompareEnabled(true);
   gpRenderBackend->SetDepthFunc(CRenderBackend::kDF_GEqual);

   gpRenderBackend->SetTexture(0, &gpRenderBackend->GetBlackMap());

   BP_SetFXShader(FX::kST_TextureRGBA);
   gpRenderBackend->EnableVisibility(0);
   gpRenderBackend->RenderQuad(l, r, b, t, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
   gpRenderBackend->EnableVisibility(1);
   if (gLensflareVis)
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero);
   gpRenderBackend->RenderQuad(l, r, b, t, p->z, 0.0f, 1.0f, 0.0f, 1.0f);
   gpRenderBackend->DisableVisibility();

   float alpha;

   {
      int a, b;
      a = gpRenderBackend->GetVisibility(0,1);
      b = gpRenderBackend->GetVisibility(0,0);
      
      // if nothing is visible, don't bother rendering flares
      if (!b)
         return;
      
      alpha = (float)a / (float)b;
   }

   CBaseTexture *tex = (CBaseTexture*)p->tex;
   if (!tex)
      gpRenderBackend->SetTexture(0, &gpRenderBackend->GetBlackMap());
   else
      gpRenderBackend->SetTexture(0, tex);
   gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Linear, CRenderBackend::kFM_Linear);
   gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

   // this happens during the halfres portion of the scene, so dst alpha is actually visibility
   gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One);

   CVector4 modulate;

   float oodw = 1.0f / DRAW_WIDTH;
   float oodh = 1.0f / DRAW_HEIGHT;
   float ootw = 1.0f / tex->GetWidth();
   float ooth = 1.0f / tex->GetHeight();
   float oo128 = 1.0f / 128.0f;

   SBP_Flare *f;
   for (int i = 0; i < 32; ++i)
   {
      // ps2 modulates texture RGB with flare RGB, then additive blends with the fb alpha
      f = &p->flares[i];

      l = f->xywh[0];
      r = f->xywh[0] + f->xywh[2];
      t = f->xywh[1];
      b = f->xywh[1] + f->xywh[3];
      l = (l * oodw - 0.5f) * 2.0f;
      r = (r * oodw - 0.5f) * 2.0f;
      t = (t * oodh - 0.5f) * -2.0f;
      b = (b * oodh - 0.5f) * -2.0f;
      
      modulate.mX = f->color[0] * oo128;
      modulate.mY = f->color[1] * oo128;
      modulate.mZ = f->color[2] * oo128;
      modulate.mW = alpha;
      
      CShaderParameterBufferAllocator::BeginRegion();
      CShaderParameterBuffer parameterBuffer = CShaderParameterBufferAllocator::AllocateParameterBuffer();
      parameterBuffer.AddParameter(FX::gModulateColor, modulate);
      CShaderParameterBufferAllocator::FinalizeParameterBuffer(parameterBuffer);

      BP_SetFXShader(FX::kST_TextureRGBModulate, parameterBuffer.GetStartAddress(), parameterBuffer.GetSize());

      gpRenderBackend->RenderQuad(l, r, b, t, 1.0f, f->uv0[0], f->uv1[0], f->uv0[1], f->uv1[1] );
      CShaderParameterBufferAllocator::EndRegion();
   }
#endif
}

void BP_PostFx_MGS3_LensflareNewMakeAlpha(void* pData)
{
   if (gFXShowPasses)
      BP_DebugText_Print("LensflareAlpha");
#ifndef BP_RENDER_SINGLE_RENDER_TARGET

   BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:LensflareNewMakeAlpha");

   SBP_LensflareNewMakeAlpha* pPacket = (SBP_LensflareNewMakeAlpha*)pData;

   SRenderTarget const originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();

   gpRenderBackend->SetAlphaTestEnable(false);

   CVector4 const sourceUV((real32)pPacket->u0 / DRAW_WIDTH, (real32)pPacket->v0 / DRAW_HEIGHT, (real32)pPacket->u1 / DRAW_WIDTH, (real32)pPacket->v1 / DRAW_HEIGHT);
#if BPE_TARGET == BPE_TARGET_X360
   //Note: This resolves width+2 and height+2 just in case
   SRect srcRect;
   srcRect.x1 = (pPacket->u0*originalRenderTarget.mpColorBuffer[0]->GetWidth() / DRAW_WIDTH) - 1;
   srcRect.x2 = (pPacket->u1*originalRenderTarget.mpColorBuffer[0]->GetWidth() / DRAW_WIDTH) + 1;
   srcRect.y1 = (pPacket->v0*originalRenderTarget.mpColorBuffer[0]->GetHeight() / DRAW_HEIGHT) - 1;
   srcRect.y2 = (pPacket->v1*originalRenderTarget.mpColorBuffer[0]->GetHeight() / DRAW_HEIGHT) + 1;
   gpRenderBackend->ResolvePartialRenderTarget(originalRenderTarget.mpColorBuffer[0], srcRect);
#endif

   // Downsample lensflare screen area down to 32x32 texture
   gpRenderBackend->SetRenderTarget(SRenderTarget(BP_GetRenderTarget(kRT_TempBuffer32px), NULL, NULL, NULL, NULL));
   gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
   gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);

   BP_DrawRectTextureModulate(originalRenderTarget.mpColorBuffer[0], -1.0f, -1.0f, 2.0f, 2.0f, CVector4(1, 1, 1, 1), sourceUV, 1, 1);
   gpRenderBackend->ResolveRenderTarget(BP_GetRenderTarget(kRT_TempBuffer32px));

   // Downsample 32x32 > 8x8
   BP_Downsample2x(BP_GetRenderTarget(kRT_TempBuffer32px), BP_GetRenderTarget(kRT_TempBuffer8px));
   gpRenderBackend->ResolveRenderTarget(BP_GetRenderTarget(kRT_TempBuffer8px));
   // Downsample 8x8 > 2x2
   BP_Downsample2x(BP_GetRenderTarget(kRT_TempBuffer8px), BP_GetRenderTarget(kRT_TempBuffer2px));
   gpRenderBackend->ResolveRenderTarget(BP_GetRenderTarget(kRT_TempBuffer2px));

   // Restore render target back to original
   gpRenderBackend->SetRenderTarget(originalRenderTarget);

   // Apply downsampled lensflare occlusion to whole frame buffer alpha
   // NOTE: This should be optimized later on by combining with the rendering of the actual lensflares instead of doing a fullscreen pass to fill the dest alpha.
   gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
   gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
   CVector4 const centerUV(0.5f, 0.5f, 0.5f, 0.5f);
   BP_DrawRectTextureModulate(BP_GetRenderTarget(kRT_TempBuffer2px), -1.0f, -1.0f, 2.0f, 2.0f, CVector4(1, 1, 1, gLensflareScale), centerUV, 1, 1);

   // Debug: Visualize occlusion rectangle copy
#if 0
   gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
   BP_DrawRectTextureModulate(BP_GetRenderTarget(kRT_TempBuffer32px), -1.0f, 0.75f, 0.25f, 0.25f, CVector4(1, 1, 1, 1), CVector4(0, 0, 1, 1), 1, 0);
#endif
#endif // !BP_RENDER_SINGLE_RENDER_TARGET
}
#endif //MGS_VERSION == 3

//----------------------------------------------------------------------------

#if MGS_VERSION == 3

int gContrastEnable = 1;

#if BP_VITA
void BP_PostFx_MGS3_Contrast(void* pData)
{
   if (gFXShowPasses)
      BP_DebugText_Print("Contrast");
   if( !gContrastEnable )
      return;

   SBP_PFX_Contrast* pPacket = (SBP_PFX_Contrast*)pData;

   {
      FX::gUberContrast = 1;
      gpRenderBackend->OverrideUpscale();
      real32 const rOffset = pPacket->r / 255.0f;
      real32 const gOffset = pPacket->g / 255.0f;
      real32 const bOffset = pPacket->b / 255.0f;
      real32 const scale = pPacket->a / 128.0f;
      
      FX::gUberContrastParam[0] = rOffset * -scale;
      FX::gUberContrastParam[1] = gOffset * -scale;
      FX::gUberContrastParam[2] = bOffset * -scale;
      FX::gUberContrastParam[3] = scale;
   }
   return;

   BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:Contrast");

   // Apply contrast
   {
      gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

      //gpRenderBackend->SetAlphaTestEnable(false);
      gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);

      gpRenderBackend->SetDepthCompareEnabled(false);

      CShaderParameterBufferAllocator::BeginRegion();

      CShaderParameterBuffer parameterBuffer = CShaderParameterBufferAllocator::AllocateParameterBuffer();
      
      real32 const rOffset = pPacket->r / 255.0f;
      real32 const gOffset = pPacket->g / 255.0f;
      real32 const bOffset = pPacket->b / 255.0f;
      real32 const scale = pPacket->a / 128.0f;

      CVector4 const contrastParam(rOffset * -scale, gOffset * -scale, bOffset * -scale, scale);

      parameterBuffer.AddParameter(FX::gContrastParam, contrastParam);
      CShaderParameterBufferAllocator::FinalizeParameterBuffer(parameterBuffer);

      BP_SetFXShader(FX::kST_Contrast, parameterBuffer.GetStartAddress(), parameterBuffer.GetSize());

      gpRenderBackend->RenderFullscreenQuad(1.0f);

      CShaderParameterBufferAllocator::EndRegion();

      gpRenderBackend->SetDepthCompareEnabled(true);
   }
}
#else
void BP_PostFx_MGS3_Contrast(void* pData)
{
#ifndef BP_RENDER_SINGLE_RENDER_TARGET

   if( !gContrastEnable )
      return;

   SBP_PFX_Contrast* pPacket = (SBP_PFX_Contrast*)pData;

   BPE_ADD_SCOPED_GPU_PROFILE_MARKER("PFX:Contrast");

   // Store original render target
   SRenderTarget const originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();

   CBaseTexture* pInputTexture = BP_GetRenderTarget(kRT_TempBuffer_NoMSAA);

#if BPE_TARGET == BPE_TARGET_X360
   // On 360 we can resolve the current render target directly into a different texture.
   gpRenderBackend->ResolveRenderTarget(pInputTexture);
#else
   // On other platforms we render directly into the textures, so we have to copy it from the current texture!
   BP_CopyTexture(BP_GetRenderTarget(kRT_CurrentFrameBuffer_NoMSAA), pInputTexture);
#endif

   // Restore original render target
   gpRenderBackend->SetRenderTarget(originalRenderTarget);

   // Apply contrast
   {
      gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

      gpRenderBackend->SetTexture(0, pInputTexture);
      gpRenderBackend->SetTextureFilter(0, CRenderBackend::kFM_Nearest, CRenderBackend::kFM_Nearest);
      gpRenderBackend->SetTextureAddressMode(0, CRenderBackend::kWM_Clamp, CRenderBackend::kWM_Clamp);

      gpRenderBackend->SetAlphaTestEnable(false);
      gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);

      gpRenderBackend->SetDepthCompareEnabled(false);

      CShaderParameterBufferAllocator::BeginRegion();

      CShaderParameterBuffer parameterBuffer = CShaderParameterBufferAllocator::AllocateParameterBuffer();
      
      real32 const rOffset = pPacket->r / 255.0f;
      real32 const gOffset = pPacket->g / 255.0f;
      real32 const bOffset = pPacket->b / 255.0f;
      real32 const scale = pPacket->a / 128.0f;

      CVector4 const contrastParam(rOffset * -scale, gOffset * -scale, bOffset * -scale, scale);

      parameterBuffer.AddParameter(FX::gContrastParam, contrastParam);
      CShaderParameterBufferAllocator::FinalizeParameterBuffer(parameterBuffer);

      CVector4 const uvOffset(0.5f / pInputTexture->GetWidth(), 0.5f / pInputTexture->GetHeight(), 0, 0);
      gpRenderBackend->SetVertexRegisters(FX::kReg_UVOffset0, 1, &uvOffset);

      BP_SetFXShader(FX::kST_Contrast, parameterBuffer.GetStartAddress(), parameterBuffer.GetSize());

      gpRenderBackend->RenderFullscreenQuad(1.0f);

      CShaderParameterBufferAllocator::EndRegion();

      gpRenderBackend->SetDepthCompareEnabled(true);
   }
#endif
}
#endif

//----------------------------------------------------------------------------
#if BP_VITA
struct SVertexSFPos
{
   CVector4 pos;
};

struct SVertexSFPos_POD
{
   float pos[4];
};

struct SVertexSFUvCol
{
   CVector4 col;
   CVector2 uv;
};

struct SVertexSFUvCol_POD
{
   float col[4];
   float uv[2];
};

#define SF_WIDTH 17
#define SF_HEIGHT 15

namespace FX
{
   boost::scoped_ptr<CStaticVertexBuffer> pShockwaveVertexBuffer;
   boost::scoped_ptr<CStaticIndexBuffer> pShockwaveIndexBuffer;
};

void BP_PostFx_InitShockwave()
{
   FX::pShockwaveVertexBuffer.reset( new CStaticVertexBuffer(sizeof(SVertexSFPos) * SF_WIDTH * SF_HEIGHT) );
   FX::pShockwaveIndexBuffer.reset( new CStaticIndexBuffer( kIT_Uint16, (SF_WIDTH-1) * (SF_HEIGHT-1) * 6 ) );

   CVector4 *pVert = reinterpret_cast<CVector4 *>( FX::pShockwaveVertexBuffer->Lock() );
   uint16 *pPrim = reinterpret_cast<uint16 *>( FX::pShockwaveIndexBuffer->Lock() );

   for (size_t i = 0; i < SF_WIDTH; i++)
      for (size_t j = 0; j < SF_HEIGHT; j++)
      {
         pVert[i * SF_HEIGHT + j] = CVector4( (i * 2.0f * 32.0f / DRAW_WIDTH - 1.0f), 
                                              -(j * 2.0f * 32.0f / DRAW_HEIGHT - 1.0f), 
                                              1.0f, 1.0f );
      }

   for (size_t i = 0; i < SF_WIDTH-1; i++)
      for (size_t j = 0; j < SF_HEIGHT-1; j++)
      {
         int base = i * SF_HEIGHT + j;
         pPrim[0] = base;
         pPrim[1] = base + SF_HEIGHT;
         pPrim[2] = base + 1;
         pPrim[3] = base + 1;
         pPrim[4] = base + SF_HEIGHT;
         pPrim[5] = base + SF_HEIGHT + 1;
         pPrim += 6;
      }
}

void BP_PostFx_ShockwaveFlutter(void *pData)
{
#if BP_VITA
   SBP_ShockwaveFlutter *pPacket = (SBP_ShockwaveFlutter *)pData;

   if (gFXShowPasses)
      BP_DebugText_Print("Shockwave");

   BP_GS_SetAlphaFunc(BP_GS_SetAlpha_Default);
   gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_InvSrcAlpha);

   SRenderTarget rt = SRenderTarget(gpRenderBackend->GetCurrentDisplayTexture(), NULL, NULL, NULL, gpRenderBackend->GetCurrentDisplayDepth());
   rt.mPixelCentersHalf = true;
   gpRenderBackend->SetRenderTarget(rt);

   // data is provided as an array of UVs + alphas, plus some sizing and a base color
   // construct the vertex and index buffers
   int nverts = 17 * 15;
   CDynamicVertexBufferPoolChunk_RT* pVertexBuffer = gpRenderBackend->GetVertexBufferPool_RT()->AllocChunk(sizeof(SVertexSFUvCol) * nverts);

   if (pVertexBuffer)
   {
      SVertexSFUvCol* pVertex = (SVertexSFUvCol*)pVertexBuffer->Lock();
      SBP_ShockwaveFlutterVert *pSrc = pPacket->verts;

      float oow = 1.0f / DRAW_WIDTH;
      float ooh = 1.0f / DRAW_HEIGHT;
      CVector4 col = BP_DecodeColorF(pPacket->color);

      float ratio = pPacket->ratio;
      float r = col.mX * 255.0f / 128.0f;
      float g = col.mY * 255.0f / 128.0f;
      float b = col.mZ * 255.0f / 128.0f;

      for (int i = 0; i < nverts; i++)
      {
         pVertex->uv[0] = pSrc->u * oow;
         pVertex->uv[1] = pSrc->v * ooh;
         pVertex->col[0] = r;
         pVertex->col[1] = g;
         pVertex->col[2] = b;
         pVertex->col[3] = pSrc->a * ratio / 128.0f;
         pSrc++;
         pVertex++;
      }
   }

   CVertexData vertexData;
   size_t const posIndex = vertexData.AddBuffer( FX::pShockwaveVertexBuffer.get(), sizeof( SVertexSFPos ) );
   vertexData.SetAttribute(kVDS_Position, offsetof(SVertexSFPos_POD, pos), kVDT_Float4, posIndex);
   size_t const uvIndex = vertexData.AddBuffer( pVertexBuffer, sizeof( SVertexSFUvCol ) );
   vertexData.SetAttribute(kVDS_TexCoord0, offsetof(SVertexSFUvCol_POD, uv), kVDT_Float2, uvIndex);
   vertexData.SetAttribute(kVDS_Color0, offsetof(SVertexSFUvCol_POD, col), kVDT_Float4, uvIndex);

   CShaderVertexDataBinding binding;
   binding.Set(kVDU_Position, kVDS_Position);
   binding.Set(kVDU_Color0, kVDS_Color0);
   binding.Set(kVDU_TexCoord0, kVDS_TexCoord0);

   // get uberscale shader set up
   CShaderParameterBufferAllocator::BeginRegion();
   CShaderParameterBuffer parameterBuffer = CShaderParameterBufferAllocator::AllocateParameterBuffer();

   UberscaleSetup(parameterBuffer, false);

   gpRenderBackend->SetVertexData(binding, vertexData, 0, 0);
   gpRenderBackend->SetIndexData( FX::pShockwaveIndexBuffer.get() );
   gpRenderBackend->ForceVertexDataRebind();
   gpRenderBackend->RenderPrimitives(CMeshChunk::kPrimitive_TriangleList, 0, (SF_WIDTH-1)*(SF_HEIGHT-1)*6);

   CShaderParameterBufferAllocator::EndRegion();

   gpRenderBackend->SetRenderTarget(SRenderTarget(gpRenderBackend->GetCurrentDisplayTexture(), NULL, NULL, NULL, gpRenderBackend->GetCurrentDisplayDepth()));
#endif
}
#endif

//----------------------------------------------------------------------------

#endif //MGS_VERSION == 3

#if BP_VITA
void BP_FX_Halfres_MSAA(void *pData)
{
   // bail out if we've already done this switch for other reasons
   if (!gpRenderBackend->IsInLowresScene())
      return;

   // bail out if we're already rendering to this, the above doesn't kick in until after the rescale
   SRenderTarget const originalRenderTarget = gpRenderBackend->GetCurrentRenderTarget();
   if (originalRenderTarget.mpColorBuffer[0] == FX::pMSAADownresTarget)
   {
      return;
   }

#if MGS_VERSION == 2
   // force the raster mask setup
   gpRenderBackend->SetRasterMask(false);
#endif
   {
      gpRenderBackend->SetRenderTarget(SRenderTarget(FX::pMSAADownresTarget, NULL, NULL, NULL, FX::pMSAADownresDepth));
      gpRenderBackend->Clear(CRenderBackend::kFlag_Color, CColor(0, 0, 0, 255));
   }

   // set alpha override function
   BP_GS_SetAlphaFunc(BP_GS_SetAlpha_OffscreenPrim);
   // flip uberscale flag
   gpRenderBackend->OverrideUpscale();

   {
      FX::pHalfresTexture = FX::pMSAADownresTarget;
      FX::pHalfresDepth = FX::pMSAADownresDepth;
   }
   FX::gHalfresEnabled = true;

   gpRenderBackend->SetDepthBias(gVTAHalfresSlopeBias, gVTAHalfresDepthBias);
   gpRenderBackend->SetTexture(0, gpRenderBackend->GetLowresDrawTexture());
}
#endif

//----------------------------------------------------------------------------
extern "C"
{
   int g_IR_Blinds_DarkenHeight = 2;
   int g_IR_Blinds_BlankHeight = 2;
   int g_CCRTLaster__Blinds_DarkenHeight = 2;
   int g_CCRTLaster__Blinds_BlankHeight = 2;
};

//extern int gVTADebugOptCmf;
//extern int gVTADebugOptCmfHalfres;
//extern int gVTADebugOptCmfHalfresFilter;
//extern int gVTADebugOptCmfMSAA;
//extern int gVTAEnableScissorOptimization;
extern "C" int gVTAHalfresAlways = 1;

void BP_InitTweakDebugMenu()
{
#if BP_VITA
#  if BP_ENABLE_DEBUG_MENU
   int const tweakMenu = BP_DebugMenu_AddMenu("Tweak", -1);

   BP_DebugMenu_AddFloat(tweakMenu, "Brightness Add", &FX::gUberTweakKnobs.mBrightnessAdd, -1.0f, 1.0f, 0.01f, 0.1f);
   BP_DebugMenu_AddFloat(tweakMenu, "Brightness Scale", &FX::gUberTweakKnobs.mBrightnessScale, 0.0f, 2.0f, 0.01f, 0.1f);
#     if MGS_VERSION == 3
   BP_DebugMenu_AddFloat(tweakMenu, "Glare Scale", &gGlareNewBloomScale, 0.0f, 2.0f, 0.01f, 0.1f);
#     endif
#  endif
#endif
}

void BP_FX_InitDebugMenu()
{
#if BP_ENABLE_DEBUG_MENU
   int const renderMenu = BP_DebugMenu_GetMenu("Render");

   int const postFXMenu = BP_DebugMenu_AddMenu("Post FX", renderMenu);
   {
      // Bloom
      {
#if MGS_VERSION == 3
         int const glareNewMenu = BP_DebugMenu_AddMenu("Glare New", postFXMenu);
         BP_DebugMenu_AddBool(glareNewMenu, "Enable", &gGlareNewEnable);
         BP_DebugMenu_AddFloat(glareNewMenu, "Glare Scale", &gGlareNewBloomScale, 0.0f, 2.0f, 0.01f, 0.1f);
         BP_DebugMenu_AddFloat(glareNewMenu, "Border Size", &gGlareNewBorderSize, 0.0f, 10.0f, 0.1f, 1.0f);
         BP_DebugMenu_AddFloat(glareNewMenu, "Blur Weight 0", &gGlareNewBlurWeights[0], 0.0f, 10.0f, 0.01f, 0.1f);
         BP_DebugMenu_AddFloat(glareNewMenu, "Blur Weight 12", &gGlareNewBlurWeights[1], 0.0f, 10.0f, 0.01f, 0.1f);
         BP_DebugMenu_AddFloat(glareNewMenu, "Blur Weight 34", &gGlareNewBlurWeights[2], 0.0f, 10.0f, 0.01f, 0.1f);
         BP_DebugMenu_AddFloat(glareNewMenu, "Blur Weight 56", &gGlareNewBlurWeights[3], 0.0f, 10.0f, 0.01f, 0.1f);
#endif
      }

      // Ghost Smoke
      {
#if MGS_VERSION == 3
         int const glareNewMenu = BP_DebugMenu_AddMenu("Ghost Smoke", postFXMenu);
         BP_DebugMenu_AddBool(glareNewMenu, "Enable", &gGhostSmokeEnable);
#endif
      }

      // Depth of field
      {
         int const farFocusMenu = BP_DebugMenu_AddMenu("Far Focus", postFXMenu);
         BP_DebugMenu_AddBool(farFocusMenu, "Enable", &gFarFocusEnable);
         BP_DebugMenu_AddBool(farFocusMenu, "Visualize", &gFarFocusVisualize);
         BP_DebugMenu_AddBool(farFocusMenu, "Stats", &gFarFocusStats);

#if MGS_VERSION == 2
         BP_DebugMenu_AddInt(farFocusMenu, "Max Plane Limit", &gFarFocusMaxPlaneCount, 0, 128, 1, 8);
#endif

         BP_DebugMenu_AddBool(farFocusMenu, "Blur Enable", &gFarFocusBlurEnable);
         BP_DebugMenu_AddFloat(farFocusMenu, "Blur Weight 0", &gFarFocusBlurWeights[0], 0.0f, 10.0f, 0.01f, 0.1f);
         BP_DebugMenu_AddFloat(farFocusMenu, "Blur Weight 12", &gFarFocusBlurWeights[1], 0.0f, 10.0f, 0.01f, 0.1f);
         BP_DebugMenu_AddFloat(farFocusMenu, "Blur Weight 34", &gFarFocusBlurWeights[2], 0.0f, 10.0f, 0.01f, 0.1f);
         BP_DebugMenu_AddFloat(farFocusMenu, "Blur Weight 56", &gFarFocusBlurWeights[3], 0.0f, 10.0f, 0.01f, 0.1f);
      }

#if MGS_VERSION == 3
      // Contrast
      {
         BP_DebugMenu_AddBool(postFXMenu, "Contrast Enable", &gContrastEnable);
      }

      BP_DebugMenu_AddFloat(postFXMenu, "Lensflare Scale", &gLensflareScale, 0.0f, 2.0f, 0.01f, 0.1f);
      BP_DebugMenu_AddBool(postFXMenu, "Lensflare Visualize", &gLensflareVis);
#endif
   // Blinds
      {
         int const blindsMenu = BP_DebugMenu_AddMenu("Blinds", postFXMenu);
         BP_DebugMenu_AddBool(blindsMenu, "Enable", &gBlinds_Enable);
         BP_DebugMenu_AddInt(blindsMenu, "IR Darken Height", &g_IR_Blinds_DarkenHeight, 1, 40, 1, 5);
         BP_DebugMenu_AddInt(blindsMenu, "IR Blank Height", &g_IR_Blinds_BlankHeight, 1, 40, 1, 5);
         BP_DebugMenu_AddInt(blindsMenu, "Codec Darken Height", &g_CCRTLaster__Blinds_DarkenHeight, 1, 40, 1, 5);
         BP_DebugMenu_AddInt(blindsMenu, "Codec Blank Height", &g_CCRTLaster__Blinds_BlankHeight, 1, 40, 1, 5);
      }
      BP_DebugMenu_AddBool(postFXMenu, "Show Passes", &gFXShowPasses);
#if BP_VITA
      BP_DebugMenu_AddBool(postFXMenu, "Use ALU Mono", &gVTAUseALUMono);
      BP_DebugMenu_AddBool(postFXMenu, "Halfres Particles", &gVTAHalfresAlways);
#endif
      //BP_DebugMenu_AddBool(postFXMenu, "Scissor Opt", &gVTAEnableScissorOptimization);
#if BP_VITA
      BP_DebugMenu_AddBool(postFXMenu, "Always Halfres Particles", &gVTAHalfresAlways);

      BP_DebugMenu_AddInt(postFXMenu, "Halfres Z Slope Bias", &gVTAHalfresSlopeBias, -16, 15, 1, 1);
      BP_DebugMenu_AddInt(postFXMenu, "Halfres Z Depth Bias", &gVTAHalfresDepthBias, -16, 15, 1, 1);
#endif
#if MGS_VERSION == 2
      BP_DebugMenu_AddBool(postFXMenu, "Codex In/Out", &gCodexInOutEnabled);
#endif
   }
#endif
}


