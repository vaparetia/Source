//------------------------------------------------------------------------------------------
// CDefaultShader.cpp
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "PSCDefaultShader.h"

//------------------------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"

//------------------------------------------------------------------------------------------

using namespace std;

CDefaultShaderCRCs   CDefaultShader::sCRC;

//------------------------------------------------------------------------------------------

CDefaultShaderCRCs::CDefaultShaderCRCs()
//:  mTextureHandle(GetParameterCRC("g_ColorTexture"))
:  mColorHandle(GetParameterCRC("g_Color"))
{
}

//----------------------------------------------------------------------------

CDefaultShader::CDefaultShader()
:  inherited(TShaderProperties())
,  mpTexture(NULL)
,  mHasColorStream(false)
,  mBlendMode(kBM_Replace)
,  mZEnable(false)
,  mApplyVertexTransform(true)
{
   // Note that any constant params will not get updated automatically after construction.
}

//----------------------------------------------------------------------------

CDefaultShader::~CDefaultShader()
{
}

//----------------------------------------------------------------------------

void CDefaultShader::Bind(SRenderEntry const * pFirstObject)
{
   static const CShaderFileId skShaderFileId("$/enginesupport/shaders/Default.fx");

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

   if ( mpTexture )
   {
      RenderBackend()->SetTexture( 0, mpTexture );
   }

   SetShader(skShaderFileId, 
             CStringExtras::StringizeInt_s("TEXTURE=%d;CONSTCOLOR=%d;HASCOLOR=%d;TRANSFORM=%d",
                                           (mpTexture != NULL),
                                           (bool)mConstColor,
                                           mHasColorStream,
                                           mApplyVertexTransform) );
}

//----------------------------------------------------------------------------

void CDefaultShader::BuildRuntimeParameters(int const pass, SRenderEntry const * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
//   parameters.AddTexture(sCRC.mTextureHandle, mpTexture ? mpTexture : &RenderBackend()->GetBlackMap(), true, true);
   parameters.AddParameterGamma(sCRC.mColorHandle, mConstColor ? *mConstColor : CColorf::White(), colorSpace);

   //MARCO: Disabled this as we're explicitly setting vertex registers these days and we're not really using CShader anymore!
   //We don't want to be binding g_Projection by means of the base class, hHave a look at BP_DebugRenderDL to see where the projection is set.
   //inherited::BuildRuntimeParameters(pass, object, parameterType, colorSpace, parameters);
}

//----------------------------------------------------------------------------

uint64 CDefaultShader::GetSortKey(SRenderEntry const * object, uint32 const uniqueId) const
{
   return uniqueId;
}
