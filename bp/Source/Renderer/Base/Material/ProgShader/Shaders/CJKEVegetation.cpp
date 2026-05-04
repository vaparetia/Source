//------------------------------------------------------------------------------------------
// CJKEVegetation.cpp
// Armature
// Copyright 2009
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CJKEVegetation.h"

//------------------------------------------------------------------------------------------

#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"

#include "Renderer/Base/Frontend/CRenderer.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Frontend/CLightState.h"
#include "Renderer/Base/Frontend/RenderObject/CMeshRenderEntity.h"
#include "Renderer/Base/Backend/CTexture.h"

#include "Renderer/Base/ShaderObjects/Shader/CCPLdrShaderJKEVegetation.h"
#include "Engine/Graphics/CLight.h"

//------------------------------------------------------------------------------------------

using namespace std;

CJKEVegetationCRCs   CJKEVegetation::sCRC;

//------------------------------------------------------------------------------------------

namespace
{
   bool sShakeActive = false;
   real32 sShakeStrength = 0.0f;
   real32 sRadialStrength = 0.0f;
   real32 sShakeOOFalloff = 1.0f;
   CVector3 sShakeSource = CVector3::Zero();
}

//------------------------------------------------------------------------------------------

CJKEVegetationCRCs::CJKEVegetationCRCs()
:  mDiffuseTexture(GetParameterCRC("g_DiffuseTexture"))
,  mNormalTexture(GetParameterCRC("g_NormalTexture"))
,  mIncandescenceTexture( GetParameterCRC( "g_Incandescence" ) )
,  mAmbientColor(GetParameterCRC("g_AmbientColor"))
,  mMovementDirection(GetParameterCRC("g_MovementDirection"))
,  mMovementParameters(GetParameterCRC("g_MovementParameters"))
,  mShakeSource( GetParameterCRC( "g_ShakeSource" ) )
,  mShakeParams( GetParameterCRC( "g_ShakeParams" ) )
,  mProjectionZRows( GetParameterCRC( "g_ProjectionZRows" ) )
{
}

//------------------------------------------------------------------------------------------

CJKEVegetation::CJKEVegetation(TShaderProperties const &pProperties)
: inherited(pProperties)
, mAmbientValue( CVector3::Zero() )
{
}

//------------------------------------------------------------------------------------------

bool CJKEVegetation::HasAlpha(CMaterialFlags const &materialFlags) const
{
   CCPLdrShaderJKEVegetation const * const pProperties = static_cast<CCPLdrShaderJKEVegetation const * const>( GetProperties() );

   switch(pProperties->mType)
   {
   case CCPLdrShaderJKEVegetation::kType_Alpha:
      return true;

   case CCPLdrShaderJKEVegetation::kType_OneBitAlpha:
   default:
      return false;
   }
}

//----------------------------------------------------------------------------

uint64 CJKEVegetation::GetDrawOrder(CDrawableRenderEntity const & object) const
{
   CCPLdrShaderJKEVegetation const * const pProperties = static_cast<CCPLdrShaderJKEVegetation const * const>( GetProperties() );

   uint64 drawOrder = inherited::GetDrawOrder(object);
   drawOrder |= pProperties->mCommonParameters.mDrawPriority;

   return drawOrder;
}

//------------------------------------------------------------------------------------------

void CJKEVegetation::Bind(CMaterialFlags const * const pMaterialFlags)
{
   inherited::Bind(pMaterialFlags);

   CCPLdrShaderJKEVegetation const * const pProperties = static_cast<CCPLdrShaderJKEVegetation const * const>( GetProperties() );

   CMaterialFlags const materialFlags = pMaterialFlags ? *pMaterialFlags : CMaterialFlags::Normal();

   // figure out values that parameters will be set to and capability requirements for the shader

   // we always have diffuse to reduce number of shader combinations
   bool const hasNormalMap = pProperties->mNormalMapTexture_Resource;

   mAmbientValue = CVector3::Zero();

   CLightState const * const pLightState = materialFlags.GetLightState();

   if( pLightState )
   {
      mAmbientValue = pLightState->mAmbient;
   }

   bool const bHasIncandescence = pProperties->mIncandescenceTexture_Resource;

   // ---

   // setup vertex format
   {
      CShaderVertexDataBinding binding(kVDU_Position, kVDU_TexCoord0);

      if( mLightmapType != PSShaderHelpers::kLT_None )
         binding.Add(kVDU_TexCoord1);

      SetShaderVertexDataBinding(binding);
   }

   // set shader
   {
      SScreenOutputControl const &outputControl = RenderBackend()->GetScreenOutputControl();

      static const CShaderFileId skShaderFileId("$/enginesupport/shaders/JKEVegetation.fx");
      SetShader(
         skShaderFileId, 
         CStringExtras::StringizeInt_s("BUMP=%d;TYPE=%d;DOUBLESIDED=%d;LIGHTMAP=%d;INCANDESCENCE=%d;DISTANCEFOG=%d;SHAKE=%d", 
         hasNormalMap, (int)pProperties->mType, pProperties->mDoubleSided, mLightmapType, bHasIncandescence,
         RenderBackend()->GetFogEnabled(),
         sShakeActive ));
   }
}

//----------------------------------------------------------------------------

static inline CBaseTexture const *get_texture_or_white( boost::optional<TResource<CBaseTexture> > const &resource )
{
   return resource.is_initialized() ? resource->GetPtr() : (&gpRenderBackend->GetWhiteMap() );
}

//----------------------------------------------------------------------------

void CJKEVegetation::BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{

   CCPLdrShaderJKEVegetation const * const properties = static_cast<CCPLdrShaderJKEVegetation const * const>( GetProperties() );

   parameters.AddTexture(sCRC.mDiffuseTexture, get_texture_or_white(properties->mDiffuseTexture_Resource));
   parameters.AddTexture(sCRC.mNormalTexture, get_texture_or_white(properties->mNormalMapTexture_Resource));
   parameters.AddTexture(sCRC.mIncandescenceTexture, get_texture_or_white( properties->mIncandescenceTexture_Resource ) );

   mLightmapType = PSShaderHelpers::AddCommonLightmapParameters(*RenderBackend(), properties->mCommonLightmapProperties, parameters);

   inherited::BuildMaterialConstantParameters(colorSpace, parameters);
}

//----------------------------------------------------------------------------

void CJKEVegetation::BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   inherited::BuildRuntimeParameters(object, parameterType, colorSpace, parameters);

   CCPLdrShaderJKEVegetation const * const properties = static_cast<CCPLdrShaderJKEVegetation const * const>( GetProperties() );

   switch(parameterType)
   {
   case kPT_Shared:
      {
         CVector3 const ambientLight = mAmbientValue + properties->mAmbient.AsVector3();

         parameters.AddParameterGamma(sCRC.mAmbientColor, ambientLight, colorSpace);

         CVector4 const movementParams(properties->mAnimationNearDistance, 
                                       1.0f / (properties->mAnimationFarDistance - properties->mAnimationNearDistance),
                                       properties->mAnimationNearScale * properties->mAnimationAmount,
                                       properties->mAnimationFarScale * properties->mAnimationAmount);
         
         parameters.AddParameter(sCRC.mMovementParameters, movementParams);

         uint32 const frame = gpRenderBackend->GetFrameCount();
         real32 const time = frame / 60.0f;

         CVector3 direction = CVector3::NegZAxis();
         
         real32 const amountTimeScaled = time * 0.5f;
         real32 const amount = (0.5f - 0.5f * cosf(time)) * (0.5f + 0.5f * sinf(time * 0.5f));

         parameters.AddParameter(sCRC.mMovementDirection, CVector4(direction * (0.15f + amount), gkTwoPi32 * time * 0.75f));
         NJKEShaderCommon::AddFogParameters( *RenderBackend(), sCRC.mFog, parameters );
         parameters.AddParameter( sCRC.mShakeParams, CVector3( sShakeStrength, sShakeOOFalloff, sRadialStrength ) );
      }
      break;
   case kPT_Unshared:
      parameters.AddParameter( sCRC.mShakeSource, object->GetTransform().TransposeMultiply( sShakeSource ) );
      {
         {
            CMatrix4 const &result = mWorldViewProjectionMatrixCached;

            parameters.AddParameter(sCRC.mProjectionZRows, CVector4( result.Get02(), result.Get12(), result.Get22(), result.Get32() ) );
         }
      }
      break;
   }
}

//----------------------------------------------------------------------------

void CJKEVegetation::SetGlobalShakeParameters( CVector3 const &shakeSource, real32 const shakeStrength, real32 const radialStrength, real32 const falloff )
{
   sShakeSource = shakeSource;
   sShakeStrength = shakeStrength;
   sRadialStrength = radialStrength;
   sShakeOOFalloff = 1.f / falloff;
   sShakeActive = !close_enough( shakeStrength, 0.0f ) || !close_enough( radialStrength, 0.0f );
}
