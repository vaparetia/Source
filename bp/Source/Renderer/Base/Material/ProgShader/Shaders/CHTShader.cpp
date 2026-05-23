//----------------------------------------------------------------------------
// CHTShader.cpp
// Bluepoint
// Copyright 2008
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CHTShader.h"

//----------------------------------------------------------------------------

#include "Engine/Evaluators/IEvaluator.h"

#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Frontend/RenderObject/CDrawableRenderEntity.h"
#include "Renderer/Base/Frontend/CLightState.h"

#include "Renderer/Base/ShaderObjects/Shader/CCPLdrShaderHTBackground.h"
#include "Renderer/Base/ShaderObjects/Shader/CCPLdrShaderHTCharacter.h"
#include "Renderer/Base/ShaderObjects/Shader/CCPLdrShaderHTEnvironment.h"
#include "Renderer/Base/ShaderObjects/Shader/CCPLdrShaderHTHolographic.h"
#include "Renderer/Base/ShaderObjects/Shader/CCPLdrShaderHTLightshaft.h"
#include "Renderer/Base/ShaderObjects/Shader/CCPLdrShaderHTTransparent.h"

//----------------------------------------------------------------------------

using namespace std;
CHTShaderCRCs  CHTShader::sCRC;

//----------------------------------------------------------------------------

CHTShaderCRCs::CHTShaderCRCs()
:  mDiffuse(GetParameterCRC("g_DiffuseColor"))
,  mDiffuseTexture(GetParameterCRC("g_DiffuseTexture"))

,  mSpecular(GetParameterCRC("g_SpecularColor"))
,  mSpecularTexture(GetParameterCRC("g_SpecularTexture"))
,  mSpecularLookupTexture(GetParameterCRC("g_SpecularLookupTexture"))

,  mIncandescence(GetParameterCRC("g_IncandescenceColor"))
,  mIncandescenceTexture(GetParameterCRC("g_IncandescenceTexture"))

,  mNormalTexture(GetParameterCRC("g_NormalTexture"))

,  mAmbient(GetParameterCRC("g_AmbientColor"))

,  mEnvMapTexture(GetParameterCRC("g_EnvMapTexture"))
,  mEnvMapColor(GetParameterCRC("g_EnvMapColor"))
,  mEnvMapMaskTexture(GetParameterCRC("g_EnvMapMaskTexture"))

,  mBloomModulationEval(GetParameterCRC("g_BloomModulation"))
,  mBloomFactors(GetParameterCRC("g_BloomFactors"))

,  mUVOffset(GetParameterCRC("g_UVOffset"))
,  mUVScale(GetParameterCRC("g_UVScale"))

,  mDirectionalLight(GetParameterCRC("g_DirectionalLight"))
,  mFadeParams(GetParameterCRC("g_FadeOutParameters"))

,  mNoiseTexture(GetParameterCRC("g_NoiseTexture"))
{
}

//----------------------------------------------------------------------------

CHTShader::CHTShader(TShaderProperties const &pProperties)
: inherited(pProperties)
, mAmbientValue(CVector3::Zero())
{
}

//----------------------------------------------------------------------------

bool CHTShader::HasAlpha(CMaterialFlags const &materialFlags) const
{
   if( inherited::HasAlpha(materialFlags) )
      return true;

   switch(GetProperties()->GetComponentType())
   {
   case CCPLdrShaderHTBackground::kComponentPropertiesType:
      {
         CCPLdrShaderHTBackground const * const pProperties = static_cast<CCPLdrShaderHTBackground const * const>( GetProperties() );
         switch( pProperties->mType )
         {
         case CCPLdrShaderHTBackground::kT_ReplaceAlphaToBloom:
            return false;

         case CCPLdrShaderHTBackground::kT_Additive:
         case CCPLdrShaderHTBackground::kT_Alpha:
         case CCPLdrShaderHTBackground::kT_AlphaReduceBloom:
            return true;

         default:
            BPE_VERIFYA(false, "Invalid shader type");
            return false;
         }
      }
      break;

   case CCPLdrShaderHTHolographic::kComponentPropertiesType:
      {
         CCPLdrShaderHTHolographic const * const pProperties = static_cast<CCPLdrShaderHTHolographic const * const>( GetProperties() );
         switch( pProperties->mType )
         {
         case CCPLdrShaderHTHolographic::kT_ReplaceAlphaToBloom:
            return false;

         case CCPLdrShaderHTHolographic::kT_Additive:
         case CCPLdrShaderHTHolographic::kT_Alpha:
         case CCPLdrShaderHTHolographic::kT_AlphaReduceBloom:
            return true;

         default:
            BPE_VERIFYA(false, "Invalid shader type");
            return false;
         }
      }
      break;

   case CCPLdrShaderHTTransparent::kComponentPropertiesType:
      return true;

   default:
      return false;
   }
}

//----------------------------------------------------------------------------

void CHTShader::InitializeEvaluators(CEvaluatorUpdateData &updateData) const
{
   switch(GetProperties()->GetComponentType())
   {
   case CCPLdrShaderHTBackground::kComponentPropertiesType:
      {
         CCPLdrShaderHTBackground const * const pProperties = static_cast<CCPLdrShaderHTBackground const * const>( GetProperties() );
         pProperties->InitializeEvaluators(updateData);
      }
      break;

   case CCPLdrShaderHTCharacter::kComponentPropertiesType:
      {
         CCPLdrShaderHTCharacter const * const pProperties = static_cast<CCPLdrShaderHTCharacter const * const>( GetProperties() );
         pProperties->InitializeEvaluators(updateData);
      }
      break;

   case CCPLdrShaderHTEnvironment::kComponentPropertiesType:
      {
         CCPLdrShaderHTEnvironment const * const pProperties = static_cast<CCPLdrShaderHTEnvironment const * const>( GetProperties() );
         pProperties->InitializeEvaluators(updateData);
      }
      break;

   case CCPLdrShaderHTHolographic::kComponentPropertiesType:
      {
         CCPLdrShaderHTHolographic const * const pProperties = static_cast<CCPLdrShaderHTHolographic const * const>( GetProperties() );
         pProperties->InitializeEvaluators(updateData);
      }
      break;

   default:
      BPE_VERIFYA( false, "Invalid/Unsupported property data for shader." );
      break;
   }
}

//----------------------------------------------------------------------------

void CHTShader::Bind(CMaterialFlags const * const pMaterialFlags)
{
   switch(GetProperties()->GetComponentType())
   {
   case CCPLdrShaderHTBackground::kComponentPropertiesType:
      BindBackground(pMaterialFlags);
      break;
   case CCPLdrShaderHTCharacter::kComponentPropertiesType:
      BindCharacter(pMaterialFlags);
      break;
   case CCPLdrShaderHTEnvironment::kComponentPropertiesType:
      BindEnvironment(pMaterialFlags);
      break;
   case CCPLdrShaderHTHolographic::kComponentPropertiesType:
      BindHolographic(pMaterialFlags);
      break;
   case CCPLdrShaderHTLightshaft::kComponentPropertiesType:
      BindLightshaft(pMaterialFlags);
      break;
   case CCPLdrShaderHTTransparent::kComponentPropertiesType:
      BindTransparent(pMaterialFlags);
      break;
   default:
      BPE_VERIFYA(false, "Invalid shader type");
      break;
   }
}

//----------------------------------------------------------------------------

void CHTShader::BindBackground(CMaterialFlags const * const pMaterialFlags)
{
   CCPLdrShaderHTBackground const * const pProperties = static_cast<CCPLdrShaderHTBackground const * const>( GetProperties() );

   CShaderVertexDataBinding binding(kVDU_Position, kVDU_Normal, kVDU_TexCoord0);
   SetShaderVertexDataBinding(binding);

   RenderBackend()->SetDepthCompareEnabled(true);

   // set shader
   static const CShaderFileId skShaderFileId("$/enginesupport/shaders/HTBackground.fx");
   SetShader(skShaderFileId, CStringExtras::StringizeInt_s("SHADER_TYPE=%d;DIRECTIONAL_LIGHT=%d;FADEOUT=%d", pProperties->mType, pProperties->mDirectionalLight, pProperties->mFadeout));
}

//----------------------------------------------------------------------------

void CHTShader::BindEnvironment(CMaterialFlags const * const pMaterialFlags)
{
   // cache some ptrs to make code more readable
   //CVLGShaderInstance const &vlgShaderInstance = static_cast<CVLGShaderInstance const &>(shaderInstance);
   CCPLdrShaderHTEnvironment const * const pProperties = static_cast<CCPLdrShaderHTEnvironment const * const>( GetProperties() );

   CMaterialFlags const materialFlags = pMaterialFlags ? *pMaterialFlags : CMaterialFlags::Normal();

   bool const hasLightmaps = mLightmapType != PSShaderHelpers::kLT_None;
   bool const hasNormalMap = !pProperties->mNormalMapTexture.empty();
   bool const hasSpecular = !pProperties->mSpecularTexture.empty();
   bool const hasIncandescence = !pProperties->mIncandescenceTexture.empty();
   bool const hasEnvMap = !pProperties->mEnvironmentMap.empty();

   SScreenOutputControl const &outputControl = RenderBackend()->GetScreenOutputControl();

   CLightState const * const pLightState = materialFlags.GetLightState();
   mAmbientValue = pLightState ? pLightState->mAmbient : CVector3::Zero();

   // ---

   // setup vertex format
   {
      CShaderVertexDataBinding binding(kVDU_Position, kVDU_Normal, kVDU_TexCoord0);
      binding.Add(kVDU_Tangent, kVDU_Binormal);
      if( hasLightmaps )
         binding.Add(kVDU_TexCoord1);

      SetShaderVertexDataBinding(binding);
   }

   // get blend mode from shader parameters
   EBlendMode blendMode = GetBlendMode(materialFlags);

   // set src/dest blend functions and blend enabled state
   SetBlendModeRenderState(blendMode);

   // set depth write enabled state
   RenderBackend()->SetDepthWriteEnabled(blendMode == kBM_Replace);

   // set depth compare enabled state
   RenderBackend()->SetDepthCompareEnabled(true);

   // set shader
   static const CShaderFileId skShaderFileId("$/enginesupport/shaders/HTEnvironment.fx");
   SetShader(skShaderFileId, CStringExtras::StringizeInt_s("LIGHTMAP=%d;NORMALMAP=%d;SPECULAR=%d;INCANDESCENCE=%d;ENVMAP=%d", hasLightmaps, hasNormalMap, hasSpecular, hasIncandescence, hasEnvMap));
}

//------------------------------------------------------------------------------------------

void CHTShader::BindCharacter(CMaterialFlags const * const pMaterialFlags)
{
   CCPLdrShaderHTCharacter const * const pProperties = static_cast<CCPLdrShaderHTCharacter const * const>( GetProperties() );

   CMaterialFlags const materialFlags = pMaterialFlags ? *pMaterialFlags : CMaterialFlags::Normal();

   CLightState const * const pLightState = materialFlags.GetLightState();
   mAmbientValue = pLightState ? pLightState->mAmbient : CVector3::Zero();

   // setup vertex format
   {
      CShaderVertexDataBinding binding(kVDU_Position, kVDU_Normal, kVDU_TexCoord0);
      binding.Add(kVDU_Tangent, kVDU_Binormal);
      SetShaderVertexDataBinding(binding);
   }

   // set src/dest blend functions and blend enabled state
   SetBlendModeRenderState(kBM_Replace);

   // set depth write enabled state
   RenderBackend()->SetDepthWriteEnabled(true);

   // set depth compare enabled state
   RenderBackend()->SetDepthCompareEnabled(true);

   // set shader
   static const CShaderFileId skShaderFileId("$/enginesupport/shaders/HTCharacter.fx");
   SetShader(skShaderFileId, CStringExtras::StringizeInt_s("SHADER_TYPE=%d", pProperties->mShaderType));
}

//----------------------------------------------------------------------------

void CHTShader::BindHolographic(CMaterialFlags const * const pMaterialFlags)
{
   CCPLdrShaderHTHolographic const * const pProperties = static_cast<CCPLdrShaderHTHolographic const * const>( GetProperties() );

   CShaderVertexDataBinding binding(kVDU_Position, kVDU_Normal, kVDU_TexCoord0);
   SetShaderVertexDataBinding(binding);

   // set shader
   static const CShaderFileId skShaderFileId("$/enginesupport/shaders/HTHolographic.fx");
   SetShader(skShaderFileId, CStringExtras::StringizeInt_s("SHADER_TYPE=%d;DO_BLOOM_PASS=%d;DOUBLE_SIDED=%d", pProperties->mType, pProperties->mDoBloomPass, pProperties->mDoubleSided));
}

//----------------------------------------------------------------------------

void CHTShader::BindLightshaft(CMaterialFlags const * const pMaterialFlags)
{
   CCPLdrShaderHTHolographic const * const pProperties = static_cast<CCPLdrShaderHTHolographic const * const>( GetProperties() );

   CShaderVertexDataBinding binding(kVDU_Position, kVDU_Normal, kVDU_TexCoord0);
   SetShaderVertexDataBinding(binding);

   // set shader
   static const CShaderFileId skShaderFileId("$/enginesupport/shaders/HTLightshaft.fx");
   SetShader(skShaderFileId, "");
}

//----------------------------------------------------------------------------

void CHTShader::BindTransparent(CMaterialFlags const * const pMaterialFlags)
{
   // cache some ptrs to make code more readable
   //CVLGShaderInstance const &vlgShaderInstance = static_cast<CVLGShaderInstance const &>(shaderInstance);
   CCPLdrShaderHTTransparent const * const pProperties = static_cast<CCPLdrShaderHTTransparent const * const>( GetProperties() );

   CMaterialFlags const materialFlags = pMaterialFlags ? *pMaterialFlags : CMaterialFlags::Normal();

   bool const hasLightmaps = mLightmapType != PSShaderHelpers::kLT_None;
   bool const hasEnvMap = !pProperties->mEnvironmentMap.empty();

   // setup vertex format
   {
      CShaderVertexDataBinding binding(kVDU_Position, kVDU_Normal, kVDU_TexCoord0);
      binding.Add(kVDU_Tangent, kVDU_Binormal);
      if( hasLightmaps )
         binding.Add(kVDU_TexCoord1);

      SetShaderVertexDataBinding(binding);
   }

   // set shader
   static const CShaderFileId skShaderFileId("$/enginesupport/shaders/HTTransparent.fx");
   SetShader(skShaderFileId, CStringExtras::StringizeInt_s("SHADER_TYPE=%d;LIGHTMAP=%d;ENVMAP=%d;DOUBLE_SIDED=%d", pProperties->mShaderType, hasLightmaps, hasEnvMap, pProperties->mDoubleSided));
}

//----------------------------------------------------------------------------

void CHTShader::BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   switch(GetProperties()->GetComponentType())
   {
   case CCPLdrShaderHTBackground::kComponentPropertiesType:
      BuildMaterialConstantParametersBackground(colorSpace, parameters);
      break;
   case CCPLdrShaderHTCharacter::kComponentPropertiesType:
      BuildMaterialConstantParametersCharacter(colorSpace, parameters);
      break;
   case CCPLdrShaderHTEnvironment::kComponentPropertiesType:
      BuildMaterialConstantParametersEnvironment(colorSpace, parameters);
      break;
   case CCPLdrShaderHTHolographic::kComponentPropertiesType:
      BuildMaterialConstantParametersHolographic(colorSpace, parameters);
      break;
   case CCPLdrShaderHTLightshaft::kComponentPropertiesType:
      BuildMaterialConstantParametersLightshaft(colorSpace, parameters);
      break;
   case CCPLdrShaderHTTransparent::kComponentPropertiesType:
      BuildMaterialConstantParametersTransparent(colorSpace, parameters);
      break;
   default:
      BPE_VERIFYA(false, "Invalid shader type");
      break;
   }

   inherited::BuildMaterialConstantParameters(colorSpace, parameters);
}

//----------------------------------------------------------------------------

void CHTShader::BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   switch(GetProperties()->GetComponentType())
   {
   case CCPLdrShaderHTBackground::kComponentPropertiesType:
      break;
   case CCPLdrShaderHTCharacter::kComponentPropertiesType:
      BuildRuntimeParametersCharacter(object, parameterType, colorSpace, parameters);
      break;
   case CCPLdrShaderHTEnvironment::kComponentPropertiesType:
      BuildRuntimeParametersEnvironment(object, parameterType, colorSpace, parameters);
      break;
   case CCPLdrShaderHTHolographic::kComponentPropertiesType:
      BuildRuntimeParametersHolographic(object, parameterType, colorSpace, parameters);
      break;
   case CCPLdrShaderHTLightshaft::kComponentPropertiesType:
      BuildRuntimeParametersLightshaft(object, parameterType, colorSpace, parameters);
      break;
   case CCPLdrShaderHTTransparent::kComponentPropertiesType:
      BuildRuntimeParametersTransparent(object, parameterType, colorSpace, parameters);
      break;
   default:
      BPE_VERIFYA(false, "Invalid shader type");
      break;
   }
   inherited::BuildRuntimeParameters(object, parameterType, colorSpace, parameters);
}

//----------------------------------------------------------------------------

void CHTShader::BuildMaterialConstantParametersBackground(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderHTBackground const * properties = static_cast<CCPLdrShaderHTBackground const * const>(GetProperties());

   parameters.AddParameterGamma(sCRC.mDiffuse, properties->mDiffuseColor, colorSpace);
   parameters.AddTexture(sCRC.mDiffuseTexture, properties->mDiffuseTexture_Resource ? properties->mDiffuseTexture_Resource->GetPtr() : &gpRenderBackend->GetWhiteMap());

   parameters.AddParameter(sCRC.mBloomFactors, CVector4(properties->mAlphaToBloomAdd, 0, 0, 0));
   parameters.AddParameter(sCRC.mDirectionalLight, CVector3(1, -0.3f, 0).Normalized());

   // fade params
   {
      real32 const start = properties->mFadeEnd;
      real32 const range = properties->mFadeStart - start;
      parameters.AddParameter(sCRC.mFadeParams, CVector4(1.0f / range, -start / range, 0, 0));
   }
}

//----------------------------------------------------------------------------

void CHTShader::BuildMaterialConstantParametersEnvironment(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderHTEnvironment const * properties = static_cast<CCPLdrShaderHTEnvironment const * const>(GetProperties());

   parameters.AddParameterGamma(sCRC.mDiffuse, properties->mDiffuseColor, colorSpace);
   parameters.AddTexture(sCRC.mDiffuseTexture, properties->mDiffuseTexture_Resource ? properties->mDiffuseTexture_Resource->GetPtr() : &gpRenderBackend->GetWhiteMap());
   parameters.AddParameterGamma(sCRC.mSpecular, properties->mSpecularColor, colorSpace);
   parameters.AddTexture(sCRC.mSpecularTexture, properties->mSpecularTexture_Resource ? properties->mSpecularTexture_Resource->GetPtr() : &RenderBackend()->GetWhiteMap());
   parameters.AddTexture(sCRC.mSpecularLookupTexture, &RenderBackend()->GetSpecularPowerMap());
   parameters.AddParameterGamma(sCRC.mIncandescence, properties->mIncandescenceColor, colorSpace);
   parameters.AddTexture(sCRC.mIncandescenceTexture, properties->mIncandescenceTexture_Resource ? properties->mIncandescenceTexture_Resource->GetPtr() : &RenderBackend()->GetBlackMap());
   parameters.AddTexture(sCRC.mNormalTexture, properties->mNormalMapTexture_Resource ? properties->mNormalMapTexture_Resource->GetPtr() : &RenderBackend()->GetFlatNormalMap() );

   mLightmapType = PSShaderHelpers::AddCommonLightmapParameters(*RenderBackend(), properties->mCommonLightmapProperties, parameters);

   parameters.AddTexture(sCRC.mEnvMapTexture, properties->mEnvironmentMap_Resource ? properties->mEnvironmentMap_Resource->GetPtr() : &gpRenderBackend->GetBlackMap());
   parameters.AddParameterGamma(sCRC.mEnvMapColor, properties->mEnvironmentMapColor, colorSpace);
   parameters.AddTexture(sCRC.mEnvMapMaskTexture, properties->mEnvironmentMapMask_Resource ? properties->mEnvironmentMapMask_Resource->GetPtr() : &gpRenderBackend->GetWhiteMap());
}

//----------------------------------------------------------------------------

void CHTShader::BuildRuntimeParametersEnvironment(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   if( parameterType == kPT_Shared )
   {
      parameters.AddParameterGamma(sCRC.mAmbient, mAmbientValue, colorSpace);
   }
}

//----------------------------------------------------------------------------

void CHTShader::BuildMaterialConstantParametersCharacter(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderHTCharacter const * properties = static_cast<CCPLdrShaderHTCharacter const * const>(GetProperties());

   parameters.AddParameterGamma(sCRC.mDiffuse, properties->mDiffuseColor, colorSpace);
   parameters.AddTexture(sCRC.mDiffuseTexture, properties->mDiffuseTexture_Resource ? properties->mDiffuseTexture_Resource->GetPtr() : &gpRenderBackend->GetWhiteMap());

   parameters.AddParameterGamma(sCRC.mSpecular, properties->mSpecularColor, colorSpace);
   parameters.AddTexture(sCRC.mSpecularTexture, properties->mSpecularTexture_Resource ? properties->mSpecularTexture_Resource->GetPtr() : &RenderBackend()->GetWhiteMap());
   parameters.AddTexture(sCRC.mSpecularLookupTexture, &RenderBackend()->GetSpecularPowerMap());

   parameters.AddTexture(sCRC.mNormalTexture, properties->mNormalMapTexture_Resource ? properties->mNormalMapTexture_Resource->GetPtr() : &RenderBackend()->GetFlatNormalMap());

   parameters.AddTexture(sCRC.mEnvMapTexture, properties->mEnvironmentMap_Resource ? properties->mEnvironmentMap_Resource->GetPtr() : &gpRenderBackend->GetBlackMap());
   parameters.AddParameterGamma(sCRC.mEnvMapColor, properties->mEnvironmentMapColor, colorSpace);
   parameters.AddTexture(sCRC.mEnvMapMaskTexture, properties->mEnvironmentMapMask_Resource ? properties->mEnvironmentMapMask_Resource->GetPtr() : &gpRenderBackend->GetWhiteMap() );
}

//----------------------------------------------------------------------------

void CHTShader::BuildRuntimeParametersCharacter(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   if( parameterType == kPT_Shared )
   {
      parameters.AddParameterGamma(sCRC.mAmbient, mAmbientValue, colorSpace);
   }
}

//----------------------------------------------------------------------------

void CHTShader::BuildMaterialConstantParametersHolographic(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderHTHolographic const * properties = static_cast<CCPLdrShaderHTHolographic const * const>(GetProperties());

   parameters.AddTexture(sCRC.mDiffuseTexture, properties->mTexture_Resource ? properties->mTexture_Resource->GetPtr() : &gpRenderBackend->GetWhiteMap());

   CVector4 const bloomFactors(properties->mBloomFromR, properties->mBloomFromG, properties->mBloomFromB, properties->mBloomFromA);
   parameters.AddParameter(sCRC.mBloomFactors, bloomFactors);

   if( !properties->mModulation )
      parameters.AddParameterGamma(sCRC.mBloomModulationEval, CColorf::White(), colorSpace);

   if( !properties->mUvOffset )
      parameters.AddParameter(sCRC.mUVOffset, CVector3::Zero());
}

//----------------------------------------------------------------------------

void CHTShader::BuildRuntimeParametersHolographic(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderHTHolographic const * properties = static_cast<CCPLdrShaderHTHolographic const * const>(GetProperties());

   CEvaluatorUpdateData updateData = object->GetUpdateData(this);

   if( properties->mModulation )
   {
      CColorf const color = properties->mModulation->GetValue(updateData);
      parameters.AddParameterGamma(sCRC.mBloomModulationEval, color, colorSpace);
   }

   if( properties->mUvOffset )
   {
      CVector3 const uvOffset = properties->mUvOffset->GetValue(updateData);
      parameters.AddParameter(sCRC.mUVOffset, uvOffset);
   }
}

//----------------------------------------------------------------------------

void CHTShader::BuildMaterialConstantParametersLightshaft(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderHTLightshaft const * properties = static_cast<CCPLdrShaderHTLightshaft const * const>(GetProperties());

   parameters.AddTexture(sCRC.mDiffuseTexture, properties->mTexture_Resource ? properties->mTexture_Resource->GetPtr() : &gpRenderBackend->GetWhiteMap());
   parameters.AddTexture(sCRC.mNoiseTexture, properties->mNoiseTexture_Resource ? properties->mNoiseTexture_Resource->GetPtr() : &gpRenderBackend->GetWhiteMap());
   parameters.AddParameter(sCRC.mUVScale, properties->mUvScale);

   if( !properties->mUvOffset )
      parameters.AddParameter(sCRC.mUVOffset, CVector3::Zero());
}

//----------------------------------------------------------------------------

void CHTShader::BuildRuntimeParametersLightshaft(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderHTLightshaft const * properties = static_cast<CCPLdrShaderHTLightshaft const * const>(GetProperties());

   CEvaluatorUpdateData updateData = object->GetUpdateData(this);

   if( properties->mUvOffset )
   {
      CVector3 const uvOffset = properties->mUvOffset->GetValue(updateData);
      parameters.AddParameter(sCRC.mUVOffset, uvOffset);
   }
   parameters.AddParameterGamma(sCRC.mDiffuse, properties->mColor * object->GetMaterialFlags().GetModulateColor(), colorSpace);
}

//----------------------------------------------------------------------------

void CHTShader::BuildMaterialConstantParametersTransparent(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderHTTransparent const * properties = static_cast<CCPLdrShaderHTTransparent const * const>(GetProperties());

   parameters.AddParameterGamma(sCRC.mDiffuse, properties->mDiffuseColor, colorSpace);
   parameters.AddTexture(sCRC.mDiffuseTexture, properties->mDiffuseTexture_Resource ? properties->mDiffuseTexture_Resource->GetPtr() : &gpRenderBackend->GetWhiteMap());
   
   mLightmapType = PSShaderHelpers::AddCommonLightmapParameters(*RenderBackend(), properties->mCommonLightmapProperties, parameters);

   parameters.AddTexture(sCRC.mEnvMapTexture, properties->mEnvironmentMap_Resource ? properties->mEnvironmentMap_Resource->GetPtr() : &gpRenderBackend->GetBlackMap());
   parameters.AddParameterGamma(sCRC.mEnvMapColor, properties->mEnvironmentMapColor, colorSpace);
   parameters.AddTexture(sCRC.mEnvMapMaskTexture, properties->mEnvironmentMapMask_Resource ? properties->mEnvironmentMapMask_Resource->GetPtr() : &gpRenderBackend->GetWhiteMap());
}

//----------------------------------------------------------------------------

void CHTShader::BuildRuntimeParametersTransparent(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   if( parameterType == kPT_Shared )
      parameters.AddParameterGamma(sCRC.mAmbient, mAmbientValue, colorSpace);
}
