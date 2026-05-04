//------------------------------------------------------------------------------------------
// CVertexLitGeneric.cpp
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CVertexLitGeneric.h"

//------------------------------------------------------------------------------------------

#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"

#include "Renderer/Base/Frontend/CRenderer.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Frontend/CLightState.h"
#include "Renderer/Base/Frontend/RenderObject/CMeshRenderEntity.h"
#include "Renderer/Base/Backend/CTexture.h"

#include "Renderer/Base/ShaderObjects/Shader/CCPLdrShaderDefaultShader.h"

#include "Engine/Evaluators/IEvaluator.h"
#include "Engine/Graphics/CLight.h"
#include "Engine/System/CProfileMarker.h"

//------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------

CVertexLitGenericCRCs CVertexLitGeneric::sCRC;

//------------------------------------------------------------------------------------------

CVertexLitGenericCRCs::CVertexLitGenericCRCs()
:  mDiffuse(GetParameterCRC("g_DiffuseColor"))
,  mDiffuseTexture(GetParameterCRC("g_DiffuseTexture"))

,  mSpecular(GetParameterCRC("g_SpecularColor"))
,  mSpecularTexture(GetParameterCRC("g_SpecularTexture"))
,  mSpecularLookupTexture(GetParameterCRC("g_SpecularLookupTexture"))

,  mIncandescence(GetParameterCRC("g_IncandescenceColor"))
,  mIncandescenceTexture(GetParameterCRC("g_IncandescenceTexture"))

,  mIncandescence2(GetParameterCRC("g_Incandescence2Color"))
,  mIncandescence2Texture(GetParameterCRC("g_Incandescence2Texture"))

,  mNormalTexture(GetParameterCRC("g_NormalTexture"))

,  mAmbient(GetParameterCRC("g_AmbientColor"))

,  mUVOffset(GetParameterCRC("g_UVOffset"))

,  mDynamicLightsTextureDir(GetParameterCRC("g_DynamicLightsTextureDir"))
,  mDynamicLightsTextureColor(GetParameterCRC("g_DynamicLightsTextureColor"))

,  mEnvMapTexture(GetParameterCRC("g_EnvMapTexture"))
,  mEnvMapColor(GetParameterCRC("g_EnvMapColor"))
,  mEnvMapMaskTexture(GetParameterCRC("g_EnvMapMaskTexture"))
{
}

//------------------------------------------------------------------------------------------

CVertexLitGeneric::CVertexLitGeneric(TShaderProperties const &pProperties)
: inherited(pProperties)
,  mAmbientLightValue(CVector3::Zero())
{
   #pragma BPE_TODOMSG("VLG - fix lightmaps, initial parameters not initialized.")
   
   BPE_VERIFY( GetProperties()->GetComponentType() == CCPLdrShaderDefaultShader::kComponentPropertiesType, false, "invalid property data type" );
}

//------------------------------------------------------------------------------------------

bool CVertexLitGeneric::HasAlpha(CMaterialFlags const &materialFlags) const
{
   if( inherited::HasAlpha(materialFlags) )
      return true;

   CCPLdrShaderDefaultShader const * const pProperties = static_cast<CCPLdrShaderDefaultShader const * const>( GetProperties() );

   if( pProperties->mAlbedoColor.IsTransparent() )
      return true;

   if( pProperties->mAlbedoTexture_Resource )
   {
      if( (*pProperties->mAlbedoTexture_Resource)->HasAlpha() )
         return true;
   }

   return false;

}

//------------------------------------------------------------------------------------------

void CVertexLitGeneric::InitializeEvaluators(CEvaluatorUpdateData &updateData) const
{
   CCPLdrShaderDefaultShader const * const pProperties = static_cast<CCPLdrShaderDefaultShader const * const>( GetProperties() );
   pProperties->InitializeEvaluators(updateData);
}

//------------------------------------------------------------------------------------------

void CVertexLitGeneric::Bind(CMaterialFlags const * const pMaterialFlags)
{
   inherited::Bind(pMaterialFlags);

   // cache some ptrs to make code more readable
   
   CCPLdrShaderDefaultShader const * const pProperties = static_cast<CCPLdrShaderDefaultShader const * const>( GetProperties() );

   CMaterialFlags const materialFlags = pMaterialFlags ? *pMaterialFlags : CMaterialFlags::Normal();

   // figure out values that parameters will be set to and capability requirements for the shader
   
   // we always have diffuse to reduce number of shader combinations
   bool const hasDiffuse = true; //pProperties->mAlbedoTexture_Resource;

   bool const hasNormalMap = pProperties->mNormalMapTexture_Resource;
   bool const hasSpecular = (pProperties->mSpecularColor != CColorf::Zero());

   bool const hasIncandescence = pProperties->mIncandescenceTexture_Resource;
   bool const hasIncandescence2 = pProperties->mIncandescence2Texture_Resource;

   bool const hasEnvMap = pProperties->mEnvironmentMap_Resource;

   int staticLightCount = 0;

   mAmbientLightValue = CVector3::Zero();

   SScreenOutputControl const &outputControl = RenderBackend()->GetScreenOutputControl();

   CLightState const * const pLightState = materialFlags.GetLightState();
   bool const bHasDynamicLights = (pLightState->mDynamicLights.size() > 0);
   if( pLightState )
   {
      if (bHasDynamicLights)
      {
         BuildIrradianceMap(pLightState);
      }
      mAmbientLightValue = pLightState->mAmbient;
   }

   // ---

   // setup vertex format
   {
      CShaderVertexDataBinding binding(kVDU_Position, kVDU_Normal, kVDU_TexCoord0);
      binding.Add(kVDU_Tangent, kVDU_Binormal);

      if( mLightmapType != PSShaderHelpers::kLT_None )
      {
         binding.Add(kVDU_TexCoord1);
      }

      binding.Add(kVDU_Color0);

      SetShaderVertexDataBinding(binding);
   }

   // get blend mode from shader parameters
   EBlendMode blendMode = GetBlendMode(materialFlags);

   // set src/dest blend functions and blend enabled state
   SetBlendModeRenderState(blendMode);

   // set depth write enabled state
   RenderBackend()->SetDepthWriteEnabled(blendMode == kBM_Replace);

   // set depth compare enabled state
   bool const zCompareEnabled = pProperties->mDepthTest == CCPLdrShaderDefaultShader::kDT_Enabled;
   RenderBackend()->SetDepthCompareEnabled(zCompareEnabled);

   bool const bHasKeyLight = (pLightState->mStaticLights.size() > 0);

   // set shader
   {
      SScreenOutputControl const & outputControl = RenderBackend()->GetScreenOutputControl();

      static const CShaderFileId skShaderFileId("$/enginesupport/shaders/VertexLitGeneric.fx");
      SetShader(skShaderFileId,
                CStringExtras::StringizeInt_s(
                "HAS_KEY_LIGHT=%d;HAS_LIGHT_TEXTURE=%d;DIFFUSE=%d;BUMP=%d;SPECULARMAP=%d;INCANDESCENCE=%d;INCANDESCENCE2=%d;LIGHTMAP=%d;ENVMAP=%d;SHADER_COLOR_SPACE=%d;FRAMEBUFFER_FORMAT=%d", 
                bHasKeyLight,
                bHasDynamicLights,
                hasDiffuse, 
                hasNormalMap,
                hasSpecular,
                hasIncandescence,
                hasIncandescence2,                
                mLightmapType,
                hasEnvMap ? (pProperties->mModulateEnvironmentMapByDiffuse ? 2 : 1) : 0,
                outputControl.GetShaderColorSpace(),
                outputControl.GetFrameBufferFormat()));
   }
}

//----------------------------------------------------------------------------

void CVertexLitGeneric::BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderDefaultShader const * const properties = static_cast<CCPLdrShaderDefaultShader const * const>( GetProperties() );

   parameters.AddParameterGamma(sCRC.mDiffuse, properties->mAlbedoColor, colorSpace);
   parameters.AddTexture(sCRC.mDiffuseTexture, properties->mAlbedoTexture_Resource ? properties->mAlbedoTexture_Resource->GetPtr() : &gpRenderBackend->GetWhiteMap());

   parameters.AddParameterGamma(sCRC.mSpecular, properties->mSpecularColor, colorSpace);
   parameters.AddTexture(sCRC.mSpecularTexture, properties->mSpecularTexture_Resource ? properties->mSpecularTexture_Resource->GetPtr() : &gpRenderBackend->GetWhiteMap());

   //parameters.AddTexture(sCRC.mSpecularLookupTexture, &RenderBackend()->GetSpecularPowerMap());

   if( !properties->mIncandescenceColorEvaluator )
      parameters.AddParameterGamma(sCRC.mIncandescence, properties->mIncandescenceColor, colorSpace);

   parameters.AddTexture(sCRC.mIncandescenceTexture, properties->mIncandescenceTexture_Resource ? properties->mIncandescenceTexture_Resource->GetPtr() : &gpRenderBackend->GetBlackMap());

   parameters.AddParameterGamma(sCRC.mIncandescence2, properties->mIncandescence2Color, colorSpace);

   parameters.AddTexture(sCRC.mIncandescence2Texture, properties->mIncandescence2Texture_Resource ? properties->mIncandescence2Texture_Resource->GetPtr() : &gpRenderBackend->GetBlackMap());

   parameters.AddTexture(sCRC.mNormalTexture, properties->mNormalMapTexture_Resource ? properties->mNormalMapTexture_Resource->GetPtr() : &gpRenderBackend->GetFlatNormalMap());

   mLightmapType = PSShaderHelpers::AddCommonLightmapParameters(*RenderBackend(), properties->mCommonLightmapProperties, parameters);

   parameters.AddTexture(sCRC.mEnvMapTexture, properties->mEnvironmentMap_Resource ? properties->mEnvironmentMap_Resource->GetPtr() : &gpRenderBackend->GetBlackMap());

   parameters.AddParameterGamma(sCRC.mEnvMapColor, properties->mEnvironmentMapColor, colorSpace);

   parameters.AddTexture(sCRC.mEnvMapMaskTexture, properties->mEnvironmentMapMask_Resource ? properties->mEnvironmentMapMask_Resource->GetPtr() : &gpRenderBackend->GetWhiteMap() );

   if( !properties->mUvOffset )
   {
      parameters.AddParameter(sCRC.mUVOffset, CVector3::Zero());
   }

   inherited::BuildMaterialConstantParameters(colorSpace, parameters);
}

//----------------------------------------------------------------------------

void CVertexLitGeneric::BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderDefaultShader const * const properties = static_cast<CCPLdrShaderDefaultShader const * const>( GetProperties() );

   switch(parameterType)
   {
   case kPT_Shared:
      {
         CLightState const * pLightState = object->GetMaterialFlags().GetLightState();

         if( pLightState )
         {
            if( pLightState->mDynamicLightsTextureDir )
               parameters.AddTexture(sCRC.mDynamicLightsTextureDir, pLightState->mDynamicLightsTextureDir.get());

            if( pLightState->mDynamicLightsTextureColor )
               parameters.AddTexture(sCRC.mDynamicLightsTextureColor, pLightState->mDynamicLightsTextureColor.get());
         }

         parameters.AddParameterGamma(sCRC.mAmbient, mAmbientLightValue, colorSpace);
      }
      break;

   case kPT_Unshared:
      {
         if( properties->mIncandescenceColorEvaluator )
         {
            CEvaluatorUpdateData updateData = object->GetUpdateData(this);
            CColorf incandescenceColor = properties->mIncandescenceColorEvaluator->GetValue(updateData);
            parameters.AddParameterGamma(sCRC.mIncandescence, incandescenceColor, colorSpace);
         }

         if( properties->mUvOffset )
         {
            CEvaluatorUpdateData updateData = object->GetUpdateData(this);
            CVector3 uvOffset = properties->mUvOffset->GetValue(updateData);
            parameters.AddParameter(sCRC.mUVOffset, uvOffset);
         }
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
static real32 const kBuildIrradianceMap_MaxX = 40.0f;
static real32 const kBuildIrradianceMap_MaxY = 32.0f;

static BPE_FORCEINLINE void position_to_uint32(uint32 * pData, CVector3 const &position)
{
   // Light position needs HDR range. Normalize and store mag.
   real32 const kPositionScale = 1.0f / kBuildIrradianceMap_MaxX;

#if BPE_TARGET != BPE_TARGET_PS3   
   CVector3 positionCopy(position);
   real32 positionMag = positionCopy.TryNormalize() * kPositionScale;
   *pData = CColor::FromVector4(CVector4(0.5f, 0.5f, 0.5f, 0.0f) + CVector4(positionCopy * 0.5f, positionMag) ).GetUint32_PlatformSpecific();
#else
   real32 positionMag = position.GetLength();
   real32 positionScale = 255.0f / (2.0f * positionMag);
   // Faster version for PS3 so we don't have to do color conversion
   uint8 *pData8 = (uint8*)pData;
   // ARGB
   *pData8 = (uint8) (positionMag * 255.0f * kPositionScale);
   pData8++;
   *pData8 = (uint8) ((position.mX * positionScale) + 127.5f);
   pData8++;
   *pData8 = (uint8) ((position.mY * positionScale) + 127.5f);
   pData8++;
   *pData8 = (uint8) ((position.mZ * positionScale) + 127.5f);
#endif
}

//----------------------------------------------------------------------------

static BPE_FORCEINLINE void color_and_radius_to_uint32(uint32 * pData, CVector3 const &color, real32 const oovRadiusSq)
{
   real32 const kColorScale = 1.0f / 32.0f;

#if BPE_TARGET != BPE_TARGET_PS3   
   real32 oovRadiusSqScaledClamped = MathUtils::ClampMinMax(2.0f * oovRadiusSq, 0.0f, 1.0f);   // 2.0f to fix rolloff issues
   *pData = CColor::FromVector4(CVector4(color * kColorScale, oovRadiusSqScaledClamped)).GetUint32_PlatformSpecific();
#else
   // Faster version for PS3 so we don't have to do color conversion
   uint8 *pData8 = (uint8*)pData;
   // ARGB
   *pData8 = (uint8) (2.0f * 255.0f * oovRadiusSq);
   pData8++;
   *pData8 = (uint8) (255.0f * kColorScale * color.mX);
   pData8++;
   *pData8 = (uint8) (255.0f * kColorScale * color.mY);
   pData8++;
   *pData8 = (uint8) (255.0f * kColorScale * color.mZ);
#endif
}
//----------------------------------------------------------------------------
struct SIrradianceMapLightCopy
{
   SIrradianceMapLightCopy(CLight const *pLight, real32 oovRadiusSq)
      : mpLight(pLight)
      , mOOVRadiusSq(oovRadiusSq)
   {
   };
   CLight const * mpLight;
   real32         mOOVRadiusSq;  // one over radius squared
};

//----------------------------------------------------------------------------

void CVertexLitGeneric::BuildIrradianceMap(CLightState const * const pLightState) const
{
   // if this light state already has a irradiance cache texture we don't need to create it
   if( pLightState->mDynamicLightsTextureDir.get() )
      return;

   BPE_ADD_SCOPED_PROFILE_MARKER("BuildIrradianceMap");

   int const xSteps = 32;
   int const ySteps = 32;

   pLightState->mDynamicLightsTextureDir.reset(CBaseTexture::CreateTexture(xSteps, ySteps, 1, CBaseTexture::kFormat_A8R8G8B8));
   pLightState->mDynamicLightsTextureColor.reset(CBaseTexture::CreateTexture(xSteps, ySteps, 1, CBaseTexture::kFormat_A8R8G8B8));

   real32 const startX = -kBuildIrradianceMap_MaxX;
   real32 const startY = -kBuildIrradianceMap_MaxY;
   real32 const endX = kBuildIrradianceMap_MaxX;
   real32 const endY = kBuildIrradianceMap_MaxY;

   real32 const xDelta = (endX - startX) / xSteps;
   real32 const yDelta = (endY - startY) / ySteps;

   uint32* pPositionData = NULL;
   int positionDataPitch = 0;
   pLightState->mDynamicLightsTextureDir->Lock((void**)&pPositionData, &positionDataPitch);

   uint32* pColorData = NULL;
   int colorDataPitch = 0;
   pLightState->mDynamicLightsTextureColor->Lock((void**)&pColorData, &colorDataPitch);

   uint32 const dynamicLightCount = pLightState->mDynamicLights.size();

   if( dynamicLightCount == 0 )
   {
      // No lights, set color to black (fixes flickering on PS3 as texture isn't initialized)
      memset(pColorData, 0, xSteps * ySteps * sizeof(uint32));
   }
   else
   {
      // Precalc some frequently used numbers
      real32 allLights_oovRadiusSq[pLightState->mDynamicLights.static_capacity];
      for( int i = 0; i < dynamicLightCount; ++i )
      {
         CLight const * pLight = pLightState->mDynamicLights[i];
         // Also add fudge factor for radius as samples are off by 1/2 pixel
         real32 oovRadiusSq = pLight->GetRadius() + xDelta;
         oovRadiusSq *= oovRadiusSq;
         oovRadiusSq = 1.0f / oovRadiusSq;
         allLights_oovRadiusSq[i] = oovRadiusSq;
      }

      real32 curY = startY;

      // DX Texel center offset
      curY += yDelta * 0.5f;

      for( int y = 0; y < ySteps; ++y, curY += yDelta )
      {
         // Build list of lights for this row
         bpe::reserved_vector<SIrradianceMapLightCopy, CLightState::TDynamicLights::static_capacity> activeLights;

         for( int i = 0; i < dynamicLightCount; ++i )
         {
            CLight const * pLight = pLightState->mDynamicLights[i];
            real32 const lightDeltaY = curY - pLight->GetTransform().GetTranslation().mZ;
            // Check to see if this row is affected by this light
            if (fabsf(lightDeltaY) > (pLight->GetRadius() + yDelta))
            {
               // Yep, skip
               continue;
            }
            // Nope, add to list of lights for this row
            activeLights.push_back(SIrradianceMapLightCopy(pLight, allLights_oovRadiusSq[i]));
         }                                                                            ;

         uint32 positionDataOffset = y * positionDataPitch / 4;
         uint32 colorDataOffset = y * colorDataPitch / 4;
         real32 curX = startX;
         // DX Texel center offset
         curX += xDelta * 0.5f;
         for( int x = 0; x < xSteps; ++x, curX += xDelta, positionDataOffset++, colorDataOffset++ )
         {
            CVector3 const position(curX, 0.0f, curY);

            CVector3 averageLightPosition(CVector3::Zero());
            real32 accumulatedAttenuation = 0.0f;

            foreach(SIrradianceMapLightCopy const &lightCopy, activeLights)
            {
               CLight const * pLight = lightCopy.mpLight;
               CVector3 const lightTranslation(pLight->GetTransform().GetTranslation());
               
               // Use 2d position for attenuation calc, as we only want to use attenuation for color and position weighting.
               // Actual attenuation will be calculated in FP
               real32 attenuation = get_light_sample_attenuation_no_clamp(lightTranslation, position, lightCopy.mOOVRadiusSq);
               if (attenuation < gkEpsilon32)
               {
                  continue;
               }
               attenuation = MathUtils::SmoothStep(attenuation);

               accumulatedAttenuation += attenuation;

               // Add weighted position
               averageLightPosition += lightTranslation * attenuation;
            }
            if (accumulatedAttenuation < (gkEpsilon32 * 100.0f))
            {
               pPositionData[positionDataOffset] = 0;
               pColorData[colorDataOffset] = 0;
               continue;
            }

            // Average the light position
            averageLightPosition /= accumulatedAttenuation;

            // Now calculate the color, attenuated from average position
            CVector3 accumlatedColor(CVector3::Zero());
            real32 averageRadius = 0;

            accumulatedAttenuation = 0.0f;

            foreach(SIrradianceMapLightCopy const &lightCopy, activeLights)
            {
               CLight const * pLight = lightCopy.mpLight;

               CVector3 const lightTranslation(pLight->GetTransform().GetTranslation());
               real32 attenuation = get_light_sample_attenuation_no_clamp(lightTranslation, averageLightPosition, lightCopy.mOOVRadiusSq);
               if (attenuation < gkEpsilon32)
               {
                  continue;
               }
               attenuation = MathUtils::SmoothStep(attenuation);

               // Added weighted color
               accumlatedColor += pLight->GetColor() * attenuation;
               // Add weighted radius
               averageRadius += pLight->GetRadius() * attenuation;
               accumulatedAttenuation += attenuation;
            }

            // Average radius
            averageRadius /= accumulatedAttenuation;

            position_to_uint32(pPositionData + positionDataOffset, averageLightPosition);
            color_and_radius_to_uint32(pColorData + colorDataOffset, accumlatedColor, 1.0f / (averageRadius * averageRadius));
         }
      }
   }

   pLightState->mDynamicLightsTextureDir->Unlock();
   pLightState->mDynamicLightsTextureColor->Unlock();
}

//----------------------------------------------------------------------------

