//------------------------------------------------------------------------------------------
// CMovieShader.cpp
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "PSCMovieShader.h"

//------------------------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"

//------------------------------------------------------------------------------------------

using namespace std;

CMovieShaderCRCs   CMovieShader::sCRC;

//------------------------------------------------------------------------------------------

CMovieShaderCRCs::CMovieShaderCRCs()
:  mTextureHandle(GetParameterCRC("g_ColorTexture"))
,  mColorHandle(GetParameterCRC("g_Color"))
{
}

//----------------------------------------------------------------------------

CMovieShader::CMovieShader(bool is448)
:  inherited(TShaderProperties())
,  mIs448(is448)
,  mpTexture(NULL)
,  mHasColorStream(false)
,  mBlendMode(kBM_Replace)
,  mZEnable(false)
,  mApplyVertexTransform(true)
{
   // Note that any constant params will not get updated automatically after construction.
}

//----------------------------------------------------------------------------

CMovieShader::~CMovieShader()
{
}

//----------------------------------------------------------------------------

void CMovieShader::Bind(SRenderEntry const * pFirstObject)
{
   static const CShaderFileId skShaderFileId("$/enginesupport/shaders/Movie.fx");

   CShaderVertexDataBinding vertexDataBinding;
   vertexDataBinding.Set(kVDU_Position, kVDS_Position);
   vertexDataBinding.Set(kVDU_TexCoord0, kVDS_TexCoord0);

   SetShaderVertexDataBinding(vertexDataBinding);
   
   SScreenOutputControl const & outputControl = RenderBackend()->GetScreenOutputControl();
   
   switch(mBlendMode)
   {
   case kBM_Alpha:
      RenderBackend()->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_InvSrcAlpha);
      RenderBackend()->SetDepthWriteEnabled(false);
      break;

   case kBM_Additive:
      RenderBackend()->SetBlendMode(true, CRenderBackend::kBF_One, CRenderBackend::kBF_One);
      RenderBackend()->SetDepthWriteEnabled(false);
      break;

   case kBM_PremultipliedAlpha:
      RenderBackend()->SetBlendMode(true, CRenderBackend::kBF_One, CRenderBackend::kBF_InvSrcAlpha);
      RenderBackend()->SetDepthWriteEnabled(false);
      break;

   case kBM_Replace:
      RenderBackend()->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      RenderBackend()->SetDepthWriteEnabled(true);
      break;
   }

   RenderBackend()->SetDepthCompareEnabled(mZEnable);

   SetShader(skShaderFileId, 
             CStringExtras::StringizeInt_s("TEXTURE=%d;CONSTCOLOR=%d;HASCOLOR=%d;TRANSFORM=%d;IS448=%d",
                                           (mpTexture != NULL),
                                           (bool)mConstColor,
                                           mHasColorStream,
                                           mApplyVertexTransform,
										   mIs448) );
}

//----------------------------------------------------------------------------

void CMovieShader::BuildRuntimeParameters(int const pass, SRenderEntry const * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   parameters.AddTexture(sCRC.mTextureHandle, mpTexture ? mpTexture : &RenderBackend()->GetBlackMap(), true, true);
   parameters.AddParameterGamma(sCRC.mColorHandle, mConstColor ? *mConstColor : CColorf::White(), colorSpace);

   inherited::BuildRuntimeParameters(pass, object, parameterType, colorSpace, parameters);
}

//----------------------------------------------------------------------------

uint64 CMovieShader::GetSortKey(SRenderEntry const * object, uint32 const uniqueId) const
{
   return uniqueId;
}
