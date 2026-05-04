//------------------------------------------------------------------------------------------
// CGlowShader.cpp
// Bluepoint
// Copyright 2004
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CGlowShader.h"

//------------------------------------------------------------------------------------------

#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Frontend/CRenderer.h"
#include "Renderer/Base/Backend/CTexture.h"

#include "Engine/Script/CScriptVar.h"

//------------------------------------------------------------------------------------------

using namespace std;
CGlowShaderCRCs   CGlowShader::sCRC;

//------------------------------------------------------------------------------------------

CGlowShaderCRCs::CGlowShaderCRCs()
:  mOutputDimensions(GetParameterCRC("g_OutputDimensions"))
,  mColorModulate(GetParameterCRC("g_ColorModulate"))
{
}

//----------------------------------------------------------------------------

CGlowShader::CGlowShader()
: inherited(TShaderProperties())
{
}

//----------------------------------------------------------------------------

void CGlowShader::Bind(CDrawableRenderEntity const * pFirstObject)
{
   static const CShaderFileId skShaderFileId("$/enginesupport/shaders/FullScreenQuad.fx");

   CShaderVertexDataBinding vertexDataBinding;
   vertexDataBinding.Set(kVDU_Position, kVDS_Position);
   vertexDataBinding.Set(kVDU_TexCoord0, kVDS_TexCoord0);

   SetShaderVertexDataBinding(vertexDataBinding);

   // Reset some state that may have been set by CGOWShader.cpp
   RenderBackend()->SetBlendOp(CRenderBackend::kBO_Add);
   RenderBackend()->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
   RenderBackend()->SetDepthWriteEnabled(false);

#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
   RenderBackend()->SetAlphaTestEnable(false);
#endif

   if( gpRenderBackend->GetCurrentRenderTarget().mpColorBuffer[0] == NULL )
   {
      SetShader(skShaderFileId, "");
   }
   else
   {
      int quality = 0;

      if( RenderBackend()->GetViewWidth() <= 1280 )
         quality = 1;

#if BPE_TARGET == BPE_TARGET_PS3
      if ( gpRenderBackend->IsLowResolution() )
      {
         quality = 2;
      }
#endif

      SScreenOutputControl const & outputControl = RenderBackend()->GetScreenOutputControl();
      static const CShaderFileId skShaderFileId("$/enginesupport/shaders/PostProcess.fx");
      SetShader(skShaderFileId,
                CStringExtras::StringizeInt_s("QUALITY=%d;SCREEN_OUTPUT_MODE=%d;SHADER_COLOR_SPACE=%d;FRAMEBUFFER_FORMAT=%d", 
                                              quality,
                                              outputControl.GetOutputMode(),
                                              outputControl.GetShaderColorSpace(),
                                              outputControl.GetFrameBufferFormat()));
   }
}

//----------------------------------------------------------------------------

void CGlowShader::BuildRuntimeParameters(int const pass, SRenderEntry const * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   if( parameterType == kPT_Shared )
   {
      // set output dimensions
      // NOTE: this is redundant, this is only used in blur passes where the output render target is the same as the texture that is blurred
      // We could as well use g_RenderTargetDimensions
      {
         real32 width = (real32) RenderBackend()->GetViewWidth();
         real32 height = (real32) RenderBackend()->GetViewHeight();

         parameters.AddParameter(sCRC.mOutputDimensions, CVector4(width, height, 1.0f / width, 1.0f / height));
      }

      // set modulation color
      parameters.AddParameterGamma(sCRC.mColorModulate, RenderBackend()->GetScreenOutputControl().mOutputModulateColor, colorSpace);
   }

   inherited::BuildRuntimeParameters(pass, object, parameterType, colorSpace, parameters);
}

//----------------------------------------------------------------------------

uint64 CGlowShader::GetSortKey(SRenderEntry const * object, uint32 const uniqueId) const
{
   return 0;
}

