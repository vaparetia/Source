//------------------------------------------------------------------------------------------
// CJKEEnvironmentShader.cpp
// Armature
// Copyright 2009
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CJKEEnvironmentShader.h"

//------------------------------------------------------------------------------------------

#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"

#include "Renderer/Base/Frontend/CRenderer.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Frontend/CLightState.h"
#include "Renderer/Base/Frontend/RenderObject/CMeshRenderEntity.h"
#include "Renderer/Base/Backend/CTexture.h"

#include "Renderer/Base/ShaderObjects/Shader/CCPLdrShaderJKEEnvironment.h"


#include "Engine/Evaluators/IEvaluator.h"
#include "Engine/Graphics/CLight.h"
#include "Engine/System/CProfileMarker.h"

//------------------------------------------------------------------------------------------

using namespace std;
CJKEEnvironmentShaderCRCs  CJKEEnvironmentShader::sCRC;

//------------------------------------------------------------------------------------------

CJKEEnvironmentShaderCRCs::CJKEEnvironmentShaderCRCs()
: mDiffuseTexture( GetParameterCRC( "g_DiffuseTexture" ) )
, mNormalTexture( GetParameterCRC( "g_NormalTexture" ) )
, mGlossTexture( GetParameterCRC( "g_GlossTexture" ) )
, mAmbientColor( GetParameterCRC( "g_AmbientColor" ) )
, mAmbientLightColor( GetParameterCRC( "g_AmbientLightColor" ) )
, mIncandescenceTexture( GetParameterCRC( "g_Incandescence" ) )
, mIncandescenceToBloom( GetParameterCRC( "g_IncandescenceToBloom" ) )
{
}

//------------------------------------------------------------------------------------------

CJKEEnvironmentShader::CJKEEnvironmentShader(TShaderProperties const &pProperties)
: inherited(pProperties)
, mAmbientValue( CVector3::Zero() )
{
   BPE_VERIFY( GetProperties()->GetComponentType() == CCPLdrShaderJKEEnvironment::kComponentPropertiesType, false, "invalid property data type" );
}

//------------------------------------------------------------------------------------------

bool CJKEEnvironmentShader::HasAlpha(CMaterialFlags const &materialFlags) const
{
   if( inherited::HasAlpha(materialFlags) )
      return true;

   return false;
}

//----------------------------------------------------------------------------

uint64 CJKEEnvironmentShader::GetDrawOrder(CDrawableRenderEntity const & object) const
{
   CCPLdrShaderJKEEnvironment const * const pProperties = static_cast<CCPLdrShaderJKEEnvironment const * const>( GetProperties() );

   uint64 drawOrder = inherited::GetDrawOrder(object);
   drawOrder |= pProperties->mCommonParameters.mDrawPriority;

   return drawOrder;
}

//------------------------------------------------------------------------------------------

void CJKEEnvironmentShader::InitializeEvaluators(CEvaluatorUpdateData &updateData) const
{
   CCPLdrShaderJKEEnvironment const * const pProperties = static_cast<CCPLdrShaderJKEEnvironment const * const>( GetProperties() );
   pProperties->InitializeEvaluators(updateData);
}

//------------------------------------------------------------------------------------------

void CJKEEnvironmentShader::Bind(CMaterialFlags const * const pMaterialFlags)
{
   inherited::Bind(pMaterialFlags);

   // cache some ptrs to make code more readable
   
   CCPLdrShaderJKEEnvironment const * const pProperties = static_cast<CCPLdrShaderJKEEnvironment const * const>( GetProperties() );

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
   bool const zCompareEnabled = pProperties->mDepthTest == CCPLdrShaderJKEEnvironment::kDT_Enabled;
   RenderBackend()->SetDepthCompareEnabled(zCompareEnabled);

   // Get env map type (0=none, 1=envmap only, 2=env+glossmap)
   int envmapType = 0;
   if ( pProperties->mEnvironmentCubeMap_Resource.is_initialized() )
   {
      if ( pProperties->mGlossTexture_Resource.is_initialized() )
      {
         envmapType = 2;
      }
      else
      {
         envmapType = 1;
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
   

   // setup vertex format
   {
      CShaderVertexDataBinding binding(kVDU_Position, kVDU_TexCoord0);

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

   // set shader
   {
      SScreenOutputControl const & outputControl = RenderBackend()->GetScreenOutputControl();

      static const CShaderFileId skShaderFileId("$/enginesupport/shaders/JKEEnvironment.fx");
      SetShader(skShaderFileId,
                CStringExtras::StringizeInt_s(
                "BUMP=%d;LIGHTMAP=%d;ENVMAP=%d;INCANDESCENCE=%d;DISTANCEFOG=%d", 
                hasNormalMap,
                mLightmapType,
                envmapType,
                incandescenceType,
                RenderBackend()->GetFogEnabled() ));
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

void CJKEEnvironmentShader::BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderJKEEnvironment const * const properties = static_cast<CCPLdrShaderJKEEnvironment const * const>( GetProperties() );

   parameters.AddTexture( sCRC.mDiffuseTexture, get_texture_or_white( properties->mAlbedoTexture_Resource ) );
   parameters.AddTexture( sCRC.mNormalTexture, get_texture_or_white( properties->mNormalMapTexture_Resource) );

   parameters.AddTexture( sCRC.mEnvMap.mEnvMapTexture, get_texture_or_white( properties->mEnvironmentCubeMap_Resource ) );

   if ( properties->mEnvironmentCubeMap_Resource.is_initialized() )
   {
      parameters.AddTexture( sCRC.mGlossTexture, get_texture_or_black( properties->mGlossTexture_Resource ) );

      parameters.AddParameterGamma( sCRC.mEnvMap.mGlossFactor, properties->mEnvironmentGlossFactor, kSCS_Gamma );
      parameters.AddParameterGamma( sCRC.mEnvMap.mGlossAdd, properties->mEnvironmentGlossAdd, kSCS_Gamma );
   }

   parameters.AddParameterGamma( sCRC.mAmbientColor, properties->mAmbientColor, kSCS_Gamma );

   parameters.AddTexture( sCRC.mIncandescenceTexture, get_texture_or_white( properties->mIncandescenceTexture_Resource ) );
   parameters.AddParameterGamma( sCRC.mIncandescenceToBloom, properties->mIncandescenceToBloom, kSCS_Gamma );

   mLightmapType = PSShaderHelpers::AddCommonLightmapParameters(*RenderBackend(), properties->mCommonLightmapProperties, parameters);

   inherited::BuildMaterialConstantParameters(colorSpace, parameters);
}

//----------------------------------------------------------------------------

void CJKEEnvironmentShader::BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderJKEEnvironment const * const properties = static_cast<CCPLdrShaderJKEEnvironment const * const>( GetProperties() );

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

