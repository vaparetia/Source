//------------------------------------------------------------------------------------------
// CJKEBlendedShader.cpp
// Armature
// Copyright 2009
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CJKEBlendedShader.h"

//------------------------------------------------------------------------------------------

#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"

#include "Renderer/Base/Frontend/CRenderer.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Frontend/CLightState.h"
#include "Renderer/Base/Frontend/RenderObject/CMeshRenderEntity.h"
#include "Renderer/Base/Backend/CTexture.h"

#include "Renderer/Base/ShaderObjects/Shader/CCPLdrShaderJKEBlended.h"

#include "Engine/Evaluators/IEvaluator.h"
#include "Engine/Graphics/CLight.h"
#include "Engine/System/CProfileMarker.h"

//------------------------------------------------------------------------------------------

using namespace std;

CJKEBlendedShaderCRCs   CJKEBlendedShader::sCRC;

//------------------------------------------------------------------------------------------

CJKEBlendedShaderCRCs::CJKEBlendedShaderCRCs()
: mDiffuseTexture1( GetParameterCRC( "g_DiffuseTexture1" ) )
, mDiffuseTexture2( GetParameterCRC( "g_DiffuseTexture2" ) )
, mNormalTexture1( GetParameterCRC( "g_NormalTexture1" ) )
, mNormalTexture2( GetParameterCRC( "g_NormalTexture2" ) )
, mGloss1( GetParameterCRC( "g_GlossTexture1" ) )
, mGloss2( GetParameterCRC( "g_GlossTexture2" ) )
, mBlendTexture( GetParameterCRC( "g_BlendTexture" ) )
, mAmbientColor( GetParameterCRC( "g_AmbientColor" ) )
, mAmbientLightColor( GetParameterCRC( "g_AmbientLightColor" ) )
{
}

//------------------------------------------------------------------------------------------

CJKEBlendedShader::CJKEBlendedShader(TShaderProperties const &pProperties)
: inherited(pProperties)
, mAmbientValue( CVector3::Zero() )
{
   BPE_VERIFY( GetProperties()->GetComponentType() == CCPLdrShaderJKEBlended::kComponentPropertiesType, false, "invalid property data type" );
}

//------------------------------------------------------------------------------------------

bool CJKEBlendedShader::HasAlpha(CMaterialFlags const &materialFlags) const
{
   if( inherited::HasAlpha(materialFlags) )
      return true;

   CCPLdrShaderJKEBlended const * const pProperties = static_cast<CCPLdrShaderJKEBlended const * const>( GetProperties() );

   if( pProperties->mDiffuse1_Resource )
   {
      if( (*pProperties->mDiffuse1_Resource)->HasAlpha() )
         return true;
   }

   if( pProperties->mDiffuse2_Resource )
   {
      if( (*pProperties->mDiffuse2_Resource)->HasAlpha() )
         return true;
   }

   return false;

}

//----------------------------------------------------------------------------

uint64 CJKEBlendedShader::GetDrawOrder(CDrawableRenderEntity const & object) const
{
   CCPLdrShaderJKEBlended const * const pProperties = static_cast<CCPLdrShaderJKEBlended const * const>( GetProperties() );

   uint64 drawOrder = inherited::GetDrawOrder(object);
   drawOrder |= pProperties->mCommonParameters.mDrawPriority;

   return drawOrder;
}

//------------------------------------------------------------------------------------------

void CJKEBlendedShader::InitializeEvaluators(CEvaluatorUpdateData &updateData) const
{
   CCPLdrShaderJKEBlended const * const pProperties = static_cast<CCPLdrShaderJKEBlended const * const>( GetProperties() );
   pProperties->InitializeEvaluators(updateData);
}

//------------------------------------------------------------------------------------------

void CJKEBlendedShader::Bind(CMaterialFlags const * const pMaterialFlags)
{
   inherited::Bind(pMaterialFlags);

   // cache some ptrs to make code more readable
   
   //CVLGShaderInstance const &vlgShaderInstance = static_cast<CVLGShaderInstance const &>(shaderInstance);
   CCPLdrShaderJKEBlended const * const pProperties = static_cast<CCPLdrShaderJKEBlended const * const>( GetProperties() );

   CMaterialFlags const materialFlags = pMaterialFlags ? *pMaterialFlags : CMaterialFlags::Normal();

   // figure out values that parameters will be set to and capability requirements for the shader
   
   // we always have diffuse to reduce number of shader combinations
   bool const blendUVsUseAux = pProperties->mBlendUVs == CCPLdrShaderJKEBlended::kBUV_AuxUV1;

   bool const hasNormalMap = pProperties->mNormal1_Resource || pProperties->mNormal2_Resource;

   int staticLightCount = 0;

   mAmbientValue = CVector3::Zero();

   SScreenOutputControl const &outputControl = RenderBackend()->GetScreenOutputControl();

   CLightState const * const pLightState = materialFlags.GetLightState();
   if( pLightState )
   {
      mAmbientValue = pLightState->mAmbient;
   }

   // ---

   // get blend mode from shader parameters
   EBlendMode blendMode = kBM_Replace; // GetBlendMode(shaderInstance, materialFlags);

   // set src/dest blend functions and blend enabled state
   SetBlendModeRenderState(blendMode);

   // set depth write enabled state
   RenderBackend()->SetDepthWriteEnabled(blendMode == kBM_Replace);

   // set depth compare enabled state
   bool const zCompareEnabled = pProperties->mDepthTest == CCPLdrShaderJKEBlended::kDT_Enabled;
   RenderBackend()->SetDepthCompareEnabled(zCompareEnabled);

   // Get env map type (0=none, 1=envmap only, 2=env+glossmap)
   int envmapType = 0;
   if ( pProperties->mEnvironmentCubeMap_Resource.is_initialized() )
   {
      if ( pProperties->mGloss1_Resource.is_initialized() || pProperties->mGloss2_Resource.is_initialized() )
      {
         envmapType = 2;
      }
      else
      {
         envmapType = 1;
      }
   }

   // setup vertex format
   {
      CShaderVertexDataBinding binding(kVDU_Position, kVDU_Color0, kVDU_TexCoord0, kVDU_TexCoord2);

      // with environment maps need at least normal
      if( envmapType != 0 )
      {
         binding.Add(kVDU_Normal);
         
         // if we have a normal map we need tangent and binormal as well
         if( hasNormalMap )
         {
            binding.Add(kVDU_Tangent, kVDU_Binormal);
         }      
      }

      // add lightmap tex coord if we have lightmaps
      if( mLightmapType != PSShaderHelpers::kLT_None )
      {
         binding.Add(kVDU_TexCoord1);
      }

      SetShaderVertexDataBinding(binding);
   }

   bool const isValid =
      ( pProperties->mDiffuse1_Resource != pProperties->mDiffuse2_Resource ) ||
      ( pProperties->mGloss1_Resource != pProperties->mGloss2_Resource ) ||
      ( pProperties->mNormal1_Resource != pProperties->mNormal2_Resource );

   // set shader
   
   if ( isValid )
   {
      static const CShaderFileId skShaderFileId("$/enginesupport/shaders/JKEBlended.fx");
      SetShader(skShaderFileId,
                CStringExtras::StringizeInt_s(
                "HAS_BLEND_UVSET=%d;BUMP=%d;LIGHTMAP=%d;ENVMAP=%d;DISTANCEFOG=%d", 
                blendUVsUseAux, 
                hasNormalMap,
                mLightmapType,
                envmapType,
                RenderBackend()->GetFogEnabled() ));
   }
   else
   {
      static const CShaderFileId skShaderFileId( "$/enginesupport/shaders/jkeinvalid.fx" );
      SetShader( skShaderFileId, "" );
   }
}

//----------------------------------------------------------------------------

static inline CBaseTexture const *get_texture_or_white( boost::optional<TResource<CBaseTexture> > const &resource )
{
   return resource.is_initialized() ? resource->GetPtr() : (&gpRenderBackend->GetWhiteMap() );
}

//----------------------------------------------------------------------------

static inline CBaseTexture const *get_texture_or_black( boost::optional<TResource<CBaseTexture> > const &resource )
{
   return resource.is_initialized() ? resource->GetPtr() : (&gpRenderBackend->GetBlackMap() );
}

//----------------------------------------------------------------------------

void CJKEBlendedShader::BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderJKEBlended const * const properties = static_cast<CCPLdrShaderJKEBlended const * const>( GetProperties() );

   parameters.AddTexture( sCRC.mDiffuseTexture1, get_texture_or_white( properties->mDiffuse1_Resource ) );
   parameters.AddTexture( sCRC.mDiffuseTexture2, get_texture_or_white( properties->mDiffuse2_Resource ) );

   parameters.AddTexture( sCRC.mNormalTexture1, get_texture_or_white( properties->mNormal1_Resource) );
   parameters.AddTexture( sCRC.mNormalTexture2, get_texture_or_white( properties->mNormal2_Resource) );

   parameters.AddTexture( sCRC.mBlendTexture, get_texture_or_white( properties->mBlendTexture_Resource ) );

   parameters.AddTexture( sCRC.mEnvMap.mEnvMapTexture, get_texture_or_white( properties->mEnvironmentCubeMap_Resource ) );

   if ( properties->mEnvironmentCubeMap_Resource.is_initialized() )
   {
      parameters.AddTexture( sCRC.mGloss1, get_texture_or_black( properties->mGloss1_Resource ) );
      parameters.AddTexture( sCRC.mGloss2, get_texture_or_black( properties->mGloss2_Resource ) );

      parameters.AddParameterGamma( sCRC.mEnvMap.mGlossFactor, properties->mEnvironmentGlossFactor, kSCS_Gamma );
      parameters.AddParameterGamma( sCRC.mEnvMap.mGlossAdd, properties->mEnvironmentGlossAdd, kSCS_Gamma );
   }
   parameters.AddParameterGamma( sCRC.mAmbientColor, properties->mAmbientColor, kSCS_Gamma );

   mLightmapType = PSShaderHelpers::AddCommonLightmapParameters(*RenderBackend(), properties->mCommonLightmapProperties, parameters);

   inherited::BuildMaterialConstantParameters(colorSpace, parameters);
}

//----------------------------------------------------------------------------

void CJKEBlendedShader::BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderJKEBlended const * const properties = static_cast<CCPLdrShaderJKEBlended const * const>( GetProperties() );

   switch(parameterType)
   {
   case kPT_Shared:
      {
         parameters.AddParameterGamma(sCRC.mAmbientLightColor, mAmbientValue, colorSpace);

         NJKEShaderCommon::AddFogParameters( *RenderBackend(), sCRC.mFog, parameters );
      }
      break;
   }

   inherited::BuildRuntimeParameters(object, parameterType, colorSpace, parameters);
}

//----------------------------------------------------------------------------

static BPE_FORCEINLINE real32 get_light_sample_attenuation_no_clamp(CVector3 const & lightPos, CVector3 const &samplePos, real32 const oovRadiusSq)
{
   real32 const deltaX = (samplePos.mX - lightPos.mX);
   real32 const deltaZ = (samplePos.mZ - lightPos.mZ);
   real32 const lightDistanceSq = (deltaX * deltaX) + (deltaZ * deltaZ);
   // This will go -ve!
   real32 const attenuation = 1.0f - (lightDistanceSq * oovRadiusSq);

   return attenuation;
}

//----------------------------------------------------------------------------

