//------------------------------------------------------------------------------------------
// CJKEFogShader.cpp
// Copyright 2009
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CJKEFogShader.h"

//------------------------------------------------------------------------------------------

#include "Engine/Evaluators/CEvaluatorUpdateData.h"
#include "Engine/Evaluators/IEvaluator.h"

#include "Renderer/Base/Frontend/RenderObject/CDrawableRenderEntity.h"
#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Frontend/CRenderer.h"
#include "Renderer/Base/Frontend/CRenderViewport.h"
#include "Renderer/Base/ShaderObjects/Shader/CCPLdrShaderJKEFog.h"

//------------------------------------------------------------------------------------------

using namespace std;

CJKEFogShaderCRCs CJKEFogShader::sCRC;

//------------------------------------------------------------------------------------------

CJKEFogShaderCRCs::CJKEFogShaderCRCs()
:  mTexture(GetParameterCRC("g_Texture"))
,  mColor(GetParameterCRC("g_FogValue"))
,  mMaxAmount(GetParameterCRC("g_FogMaxAmount" ))
,  mNearFar( GetParameterCRC( "g_FogNearFar" ) )
,  mDepthTexture( GetParameterCRC( "g_DepthTexture" ) )
,  mWorldView( GetParameterCRC( "g_WorldView" ) )
,  mUVOffsetScale( GetParameterCRC( "g_UVOffsetScale" ) )
{
}

//------------------------------------------------------------------------------------------

CJKEFogShader::CJKEFogShader(TShaderProperties const &pProperties)
: inherited(pProperties)
{
   BPE_VERIFY(GetProperties()->GetComponentType() == CCPLdrShaderJKEFog::kComponentPropertiesType, false, "invalid property data type" );
}

//------------------------------------------------------------------------------------------

void CJKEFogShader::InitializeEvaluators(CEvaluatorUpdateData &updateData) const
{
   CCPLdrShaderJKEFog const * const pProperties = static_cast<CCPLdrShaderJKEFog const * const>( GetProperties() );
   pProperties->InitializeEvaluators(updateData);
}

//------------------------------------------------------------------------------------------

void CJKEFogShader::Bind(CMaterialFlags const * const pMaterialFlags)
{
   inherited::Bind(pMaterialFlags);

   CCPLdrShaderJKEFog const * const pProperties = static_cast<CCPLdrShaderJKEFog const * const>( GetProperties() );

   bool const usesNormals = pProperties->mUseNormals;
   bool const usesBackfaces = pProperties->mUseBackfacingTriangles;
   int const type = pProperties->mType;
   int const vertexColor = pProperties->mVertexColor;

   // setup vertex format
   {
      CShaderVertexDataBinding binding(kVDU_Position, kVDU_TexCoord0);

      if( usesNormals )
      {
         binding.Add( kVDU_Normal );
      }

      if ( pProperties->mVertexColor != CCPLdrShaderJKEFog::kVC_Ignore )
      {
         binding.Add( kVDU_Color0 );
      }

      SetShaderVertexDataBinding(binding);
   }

   // set shader
   {
      SScreenOutputControl const &outputControl = RenderBackend()->GetScreenOutputControl();

      static const CShaderFileId skShaderFileId("$/enginesupport/shaders/JKEFog.fx");
      SetShader(skShaderFileId, CStringExtras::StringizeInt_s(
         "USENORMAL=%d;TYPE=%d;USEBACKFACES=%d;VERTEXCOLOR=%d", usesNormals, type, usesBackfaces, vertexColor)
         );
   }
}

//----------------------------------------------------------------------------

bool CJKEFogShader::HasAlpha(CMaterialFlags const &materialFlags) const
{
   return true;
}


//----------------------------------------------------------------------------

static inline CBaseTexture const *get_texture_or_white( boost::optional<TResource<CBaseTexture> > const &resource )
{
   return resource.is_initialized() ? resource->GetPtr() : (&gpRenderBackend->GetWhiteMap() );
}

//----------------------------------------------------------------------------

void CJKEFogShader::BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderJKEFog const * const properties = static_cast<CCPLdrShaderJKEFog const * const>( GetProperties() );

   parameters.AddTexture(sCRC.mTexture, get_texture_or_white(properties->mTexture_Resource));

   parameters.AddParameterGamma(sCRC.mColor, properties->mColor, kSCS_Gamma);
   parameters.AddParameter( sCRC.mMaxAmount, properties->mMaxAmount );
   
   if ( !properties->mUvOffset && !properties->mUvScale )
   {
      parameters.AddParameter( sCRC.mUVOffsetScale, CVector4( 0.0f, 0.0f, 1.0f, 1.0f ) );
   }

   inherited::BuildMaterialConstantParameters(colorSpace, parameters);
}

//----------------------------------------------------------------------------

void CJKEFogShader::BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderJKEFog const * const properties = static_cast<CCPLdrShaderJKEFog const * const>( GetProperties() );

   switch(parameterType)
   {
   case kPT_Unshared:
      parameters.AddParameter( sCRC.mWorldView, RenderBackend()->GetViewMatrix() * CMatrix4::FromMatrix34(object->GetTransform()) );
      if ( properties->mUvOffset || properties->mUvScale )
      {
         CEvaluatorUpdateData updateData = object->GetUpdateData(this);
         CVector4 offsetAndScale( 0.f, 0.f, 1.f, 1.f );
         if ( properties->mUvOffset )
         {
            CVector3 uvOffset = properties->mUvOffset->GetValue(updateData);
            
            offsetAndScale.SetXY( uvOffset.DropZ() );
         }

         if ( properties->mUvScale )
         {
            CVector3 uvScale = properties->mUvScale->GetValue( updateData );

            offsetAndScale.SetZW( uvScale.DropZ() );
         }

         parameters.AddParameter( sCRC.mUVOffsetScale, offsetAndScale );
      }
      break;
   case kPT_Shared:
      {
         CRenderer * pRenderer = Renderer();
         if( pRenderer )
         {
            CRenderViewport const * pViewport = pRenderer->GetCurrentViewport();
            if( pViewport )
            {
               parameters.AddParameter( sCRC.mNearFar, CVector4( properties->mFogStart, 1.f / ( properties->mFogEnd - properties->mFogStart ), pViewport->GetNearClipPlane(), pViewport->GetFarClipPlane() ) );

#if BPE_TARGET == BPE_TARGET_WIN32   
               CBaseTexture const *pTexture = pViewport->GetRenderTarget().mpColorBuffer[1];
#else
               CBaseTexture const *pTexture = pViewport->GetRenderTarget().mpDepthBuffer;
#endif
               if( pTexture != NULL )
               {
                  parameters.AddTexture(sCRC.mDepthTexture, pTexture );
               }
            }
         }
      }
      break;
   }

   inherited::BuildRuntimeParameters(object, parameterType, colorSpace, parameters);
}
