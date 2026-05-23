//------------------------------------------------------------------------------------------
// CJKESkyShader.cpp
// Copyright 2009
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CJKESkyShader.h"

//------------------------------------------------------------------------------------------

#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Frontend/CRenderer.h"
#include "Renderer/Base/Frontend/CRenderViewport.h"
#include "Renderer/Base/ShaderObjects/Shader/CCPLdrShaderJKESky.h"

//------------------------------------------------------------------------------------------

using namespace std;

CJKESkyShaderCRCs CJKESkyShader::sCRC;

//------------------------------------------------------------------------------------------

CJKESkyShaderCRCs::CJKESkyShaderCRCs()
:  mTexture(GetParameterCRC("g_Texture"))
,  mBloomFromColor(GetParameterCRC("g_BloomFromColor"))
,  mParameters(GetParameterCRC("g_Parameters" ))
,  mViewportParameters( GetParameterCRC( "g_ViewportParameters" ) )
,  mDepthTexture( GetParameterCRC( "g_DepthTexture" ) )
{
}

//------------------------------------------------------------------------------------------

CJKESkyShader::CJKESkyShader(TShaderProperties const &pProperties)
: inherited(pProperties)
{
   BPE_VERIFY( GetProperties()->GetComponentType() == CCPLdrShaderJKESky::kComponentPropertiesType, false, "invalid property data type" );
}

//------------------------------------------------------------------------------------------

void CJKESkyShader::InitializeEvaluators(CEvaluatorUpdateData &updateData) const
{
   CCPLdrShaderJKESky const * const pProperties = static_cast<CCPLdrShaderJKESky const * const>( GetProperties() );
   pProperties->InitializeEvaluators(updateData);
}

//------------------------------------------------------------------------------------------

void CJKESkyShader::Bind(CMaterialFlags const * const pMaterialFlags)
{
   inherited::Bind(pMaterialFlags);
   CCPLdrShaderJKESky const * const pProperties = static_cast<CCPLdrShaderJKESky const * const>( GetProperties() );

   // setup vertex format
   {
      CShaderVertexDataBinding binding(kVDU_Position, kVDU_TexCoord0);
      SetShaderVertexDataBinding(binding);
   }

   // set shader
   {
      static const CShaderFileId skShaderFileId("$/enginesupport/shaders/jkesky.fx");
      SetShader(skShaderFileId, "");
   }
}

//----------------------------------------------------------------------------

static inline CBaseTexture const *get_texture_or_white( boost::optional<TResource<CBaseTexture> > const &resource )
{
   return resource.is_initialized() ? resource->GetPtr() : (&gpRenderBackend->GetWhiteMap() );
}

//----------------------------------------------------------------------------

void CJKESkyShader::BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderJKESky const * const properties = static_cast<CCPLdrShaderJKESky const * const>( GetProperties() );

   parameters.AddTexture(sCRC.mTexture, get_texture_or_white(properties->mTexture_Resource));

   parameters.AddParameterGamma(sCRC.mBloomFromColor, properties->mBloomFromColor, kSCS_Gamma);
   parameters.AddParameter(sCRC.mParameters, CVector4(properties->mConstantBloomAdd, properties->mColorMultiply, properties->mFogStart, properties->mFogEnd ));

   inherited::BuildMaterialConstantParameters(colorSpace, parameters);
}

//----------------------------------------------------------------------------

void CJKESkyShader::BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   switch(parameterType)
   {
   case kPT_Shared:
      {
         CRenderer * pRenderer = Renderer();
         if( pRenderer )
         {
            CRenderViewport const * pViewport = pRenderer->GetCurrentViewport();
            if( pViewport )
            {
#if BPE_TARGET == BPE_TARGET_WIN32   
               CBaseTexture const *pTexture = pViewport->GetRenderTarget().mpColorBuffer[1];
#else
               CBaseTexture const *pTexture = pViewport->GetRenderTarget().mpDepthBuffer;
#endif
               if( pTexture != NULL )
               {
                  parameters.AddTexture(sCRC.mDepthTexture, pTexture );
                  parameters.AddParameter( sCRC.mViewportParameters, CVector4( real32( pTexture->GetAntiAliasWidth() ), real32( pTexture->GetAntiAliasHeight() ), pViewport->GetNearClipPlane(), pViewport->GetFarClipPlane() ) );
               }
            }
         }
      }
      break;
   }

   inherited::BuildRuntimeParameters(object, parameterType, colorSpace, parameters);
}
