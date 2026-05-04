//------------------------------------------------------------------------------------------
// CJKECharacterShader.cpp
// Armature
// Copyright 2009
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CJKECharacterShader.h"

//------------------------------------------------------------------------------------------

#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"

#include "Renderer/Base/Frontend/CRenderer.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Frontend/CLightState.h"
#include "Renderer/Base/Frontend/RenderObject/CMeshRenderEntity.h"
#include "Renderer/Base/Backend/CTexture.h"

#include "Renderer/Base/ShaderObjects/Shader/CCPLdrShaderJKECharacter.h"

#include "Engine/Evaluators/IEvaluator.h"
#include "Engine/Graphics/CLight.h"
#include "Engine/System/CProfileMarker.h"

//------------------------------------------------------------------------------------------

using namespace std;
CJKECharacterShaderCRCs CJKECharacterShader::sCRC;

//------------------------------------------------------------------------------------------

CJKECharacterShaderCRCs::CJKECharacterShaderCRCs()
: mDiffuseTexture( GetParameterCRC( "g_DiffuseTexture" ) )
, mSpecularLookupTexture(GetParameterCRC("g_SpecularLookupTexture"))
, mNormalTexture( GetParameterCRC( "g_NormalTexture" ) )
, mGlossTexture( GetParameterCRC( "g_GlossTexture" ) )
, mAmbientColor( GetParameterCRC( "g_AmbientColor" ) )
, mAmbientLightColor( GetParameterCRC( "g_AmbientLightColor" ) )
, mIncandescenceTexture( GetParameterCRC( "g_Incandescence" ) )
, mIncandescenceToBloom( GetParameterCRC( "g_IncandescenceToBloom" ) )
, mSpecularity( GetParameterCRC( "g_Specularity" ) )
{
}

//------------------------------------------------------------------------------------------

CJKECharacterShader::CJKECharacterShader(TShaderProperties const &pProperties)
: inherited(pProperties)
, mAmbientValue( CVector3::Zero() )
{
   BPE_VERIFY( GetProperties()->GetComponentType() == CCPLdrShaderJKECharacter::kComponentPropertiesType, false, "invalid property data type" );
}

//------------------------------------------------------------------------------------------

bool CJKECharacterShader::HasAlpha(CMaterialFlags const &materialFlags) const
{
   if( inherited::HasAlpha(materialFlags) )
      return true;

   return false;

}

//------------------------------------------------------------------------------------------

void CJKECharacterShader::InitializeEvaluators(CEvaluatorUpdateData &updateData) const
{
   CCPLdrShaderJKECharacter const * const pProperties = static_cast<CCPLdrShaderJKECharacter const * const>( GetProperties() );
   pProperties->InitializeEvaluators(updateData);
}

//------------------------------------------------------------------------------------------

void CJKECharacterShader::Bind(CMaterialFlags const * const pMaterialFlags)
{
   inherited::Bind(pMaterialFlags);
   // cache some ptrs to make code more readable
   
   CCPLdrShaderJKECharacter const * const pProperties = static_cast<CCPLdrShaderJKECharacter const * const>( GetProperties() );

   CMaterialFlags const materialFlags = pMaterialFlags ? *pMaterialFlags : CMaterialFlags::Normal();

   // figure out values that parameters will be set to and capability requirements for the shader
   
   // we always have diffuse to reduce number of shader combinations
   bool const hasNormalMap = pProperties->mNormalMapTexture_Resource;

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
   EBlendMode blendMode = GetBlendMode(materialFlags);

   // set src/dest blend functions and blend enabled state
   SetBlendModeRenderState(blendMode);

   // set depth write enabled state
   RenderBackend()->SetDepthWriteEnabled(blendMode == kBM_Replace);

   // set depth compare enabled state
   bool const zCompareEnabled = pProperties->mDepthTest == CCPLdrShaderJKECharacter::kDT_Enabled;
   RenderBackend()->SetDepthCompareEnabled(zCompareEnabled);

   // Right now, there's only two lights max
   int lightCount = bpe::min_val(2, pLightState->mStaticLights.size());

   bool bHasEnvMap = pProperties->mEnvironmentCubeMap_Resource.is_initialized();

   // specular lighting type - 0 = no specular, 1 = ignore gloss, 2 = use gloss from color
   int specularType = lightCount ? pProperties->mSpecularType : CCPLdrShaderJKECharacter::kST_None;
   
   // Now we figure out if we need the gloss map from environment mapping and specular
   // glossType = ( 0:no gloss, 1:gloss from color, 2:gloss from glossmap )
   int glossType = 0;

   if ( bHasEnvMap || specularType == CCPLdrShaderJKECharacter::kST_UseGloss )
   {
      // Ok, we WANT gloss
      if ( pProperties->mGlossTexture_Resource.is_initialized() )
      {
         glossType = 2;
      }
      else
      {
         glossType = 1;
      }
   }

   int incandescenceType; // 0 = none, 1 = normal, 2 = using external shader param

   if ( pProperties->mIncandescenceTexture_Resource )
   {
      incandescenceType = pProperties->mModulateIncandescenceWithExternalParam ? 2 : 1;
   }
   else
   {
      incandescenceType = 0;
   }

   //bool const hasAdditiveOrModulate = !close_enough( CColorf::Zero(), pMaterialFlags->GetAdditiveColor(), CColorf::kOneOver255 ) || !close_enough( CColorf::White(), pMaterialFlags->GetModulateColor(), CColorf::kOneOver255 );
   // Disabled due to shader batching issues, as sort key doesn't take additive color into account.
   bool const hasAdditiveOrModulate = true;

   // setup vertex format
   {
      CShaderVertexDataBinding binding(kVDU_Position, kVDU_TexCoord0);

      if ( lightCount > 0 || bHasEnvMap || specularType > 0 )
      {
         binding.Add( kVDU_Normal );

         if ( hasNormalMap )
         {
            binding.Add(kVDU_Tangent, kVDU_Binormal);
         }
      }

      SetShaderVertexDataBinding(binding);
   }

   // set shader
   {
      SScreenOutputControl const & outputControl = RenderBackend()->GetScreenOutputControl();

      static const CShaderFileId skShaderFileId("$/enginesupport/shaders/JKECharacter.fx");
      SetShader(skShaderFileId,
                CStringExtras::StringizeInt_s(
                "BUMP=%d;ADD_OR_MOD=%d;LIGHT_COUNT=%d;INCANDESCENCE=%d;DISTANCEFOG=%d;SPECULAR=%d;GLOSS=%d;ENVMAP=%d;PLAYER_TOOL=%d", 
                hasNormalMap,
                hasAdditiveOrModulate,
                lightCount,
                incandescenceType,
                RenderBackend()->GetFogEnabled(),
                specularType,
                glossType,
                bHasEnvMap,
                pProperties->mIsPlayerToolShader));
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

void CJKECharacterShader::BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderJKECharacter const * const properties = static_cast<CCPLdrShaderJKECharacter const * const>( GetProperties() );

   parameters.AddTexture( sCRC.mDiffuseTexture, get_texture_or_white( properties->mAlbedoTexture_Resource ) );
   parameters.AddTexture( sCRC.mNormalTexture, get_texture_or_white( properties->mNormalMapTexture_Resource) );

   parameters.AddTexture( sCRC.mEnvMap.mEnvMapTexture, get_texture_or_white( properties->mEnvironmentCubeMap_Resource ) );

   if ( properties->mEnvironmentCubeMap_Resource.is_initialized() || properties->mSpecularType == CCPLdrShaderJKECharacter::kST_UseGloss )
   {
      parameters.AddTexture( sCRC.mGlossTexture, get_texture_or_black( properties->mGlossTexture_Resource ) );

      parameters.AddParameterGamma( sCRC.mEnvMap.mGlossAdd, properties->mEnvironmentGlossAdd, kSCS_Gamma );
   }

   if ( properties->mEnvironmentCubeMap_Resource.is_initialized() || properties->mSpecularType != CCPLdrShaderJKECharacter::kST_None )
   {
      parameters.AddParameterGamma( sCRC.mEnvMap.mGlossFactor, properties->mEnvironmentGlossFactor, kSCS_Gamma );
   }

   if ( properties->mSpecularType != CCPLdrShaderJKECharacter::kST_None )
   {
      parameters.AddTexture( sCRC.mSpecularLookupTexture, &RenderBackend()->GetSpecularPowerMap() );
      parameters.AddParameter( sCRC.mSpecularity, properties->mSpecularSharpness );
   }

   parameters.AddParameterGamma( sCRC.mAmbientColor, properties->mAmbientColor, kSCS_Gamma );

   parameters.AddTexture( sCRC.mIncandescenceTexture, get_texture_or_white( properties->mIncandescenceTexture_Resource ) );
   parameters.AddParameterGamma( sCRC.mIncandescenceToBloom, properties->mIncandescenceToBloom, kSCS_Gamma );

   inherited::BuildMaterialConstantParameters(colorSpace, parameters);
}

//----------------------------------------------------------------------------

void CJKECharacterShader::BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderJKECharacter const * const properties = static_cast<CCPLdrShaderJKECharacter const * const>( GetProperties() );

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

