//------------------------------------------------------------------------------------------
// CJKETransparent.cpp
// Armature
// Copyright 2009
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CJKETransparent.h"

//------------------------------------------------------------------------------------------

#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"

#include "Renderer/Base/Frontend/CRenderer.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Frontend/CLightState.h"
#include "Renderer/Base/Frontend/RenderObject/CMeshRenderEntity.h"
#include "Renderer/Base/Backend/CTexture.h"

#include "Renderer/Base/ShaderObjects/Shader/CCPLdrShaderJKETransparent.h"

#include "Engine/Graphics/CLight.h"

//------------------------------------------------------------------------------------------

using namespace std;
CJKETransparentCRCs   CJKETransparent::sCRC;

//------------------------------------------------------------------------------------------

CJKETransparentCRCs::CJKETransparentCRCs()
:  mDiffuseTexture(GetParameterCRC("g_DiffuseTexture"))
,  mNormalTexture(GetParameterCRC("g_NormalTexture"))
,  mIncandescenceTexture( GetParameterCRC( "g_Incandescence" ) )
,  mAmbientColor(GetParameterCRC("g_AmbientColor"))
{
}

//------------------------------------------------------------------------------------------

CJKETransparent::CJKETransparent(TShaderProperties const &pProperties)
: inherited(pProperties)
,  mAmbientValue( CVector3::Zero() )
{
}

//------------------------------------------------------------------------------------------

bool CJKETransparent::HasAlpha(CMaterialFlags const &materialFlags) const
{
   CCPLdrShaderJKETransparent const * const pProperties = static_cast<CCPLdrShaderJKETransparent const * const>( GetProperties() );

   switch(pProperties->mType)
   {
   case CCPLdrShaderJKETransparent::kType_Alpha:
      return true;

   case CCPLdrShaderJKETransparent::kType_OneBitAlpha:
   default:
      return false;
   }
}

//----------------------------------------------------------------------------

uint64 CJKETransparent::GetDrawOrder(CDrawableRenderEntity const & object) const
{
   CCPLdrShaderJKETransparent const * const pProperties = static_cast<CCPLdrShaderJKETransparent const * const>( GetProperties() );

   uint64 drawOrder = inherited::GetDrawOrder(object);
   drawOrder |= pProperties->mCommonParameters.mDrawPriority;

   return drawOrder;
}

//------------------------------------------------------------------------------------------

void CJKETransparent::Bind(CMaterialFlags const * const pMaterialFlags)
{
   inherited::Bind(pMaterialFlags);

   CCPLdrShaderJKETransparent const * const pProperties = static_cast<CCPLdrShaderJKETransparent const * const>( GetProperties() );


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

      static const CShaderFileId skShaderFileId("$/enginesupport/shaders/JKETransparent.fx");
      SetShader(
         skShaderFileId, 
         CStringExtras::StringizeInt_s("BUMP=%d;TYPE=%d;DOUBLESIDED=%d;LIGHTMAP=%d;INCANDESCENCE=%d;DISTANCEFOG=%d", 
         hasNormalMap, (int)pProperties->mType, pProperties->mDoubleSided, mLightmapType, bHasIncandescence,
         RenderBackend()->GetFogEnabled() ));
   }
}

//----------------------------------------------------------------------------

static inline CBaseTexture const *get_texture_or_white( boost::optional<TResource<CBaseTexture> > const &resource )
{
   return resource.is_initialized() ? resource->GetPtr() : (&gpRenderBackend->GetWhiteMap() );
}

//----------------------------------------------------------------------------

void CJKETransparent::BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   
   CCPLdrShaderJKETransparent const * const properties = static_cast<CCPLdrShaderJKETransparent const * const>( GetProperties() );

   parameters.AddTexture(sCRC.mDiffuseTexture, get_texture_or_white(properties->mDiffuseTexture_Resource));
   parameters.AddTexture(sCRC.mNormalTexture, get_texture_or_white(properties->mNormalMapTexture_Resource));
   parameters.AddTexture(sCRC.mIncandescenceTexture, get_texture_or_white( properties->mIncandescenceTexture_Resource ) );

   mLightmapType = PSShaderHelpers::AddCommonLightmapParameters(*RenderBackend(), properties->mCommonLightmapProperties, parameters);

   inherited::BuildMaterialConstantParameters(colorSpace, parameters);
}

//----------------------------------------------------------------------------

void CJKETransparent::BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderJKETransparent const * const properties = static_cast<CCPLdrShaderJKETransparent const * const>( GetProperties() );

   switch(parameterType)
   {
   case kPT_Shared:
      {
         parameters.AddParameterGamma(sCRC.mAmbientColor, mAmbientValue, colorSpace);
         NJKEShaderCommon::AddFogParameters( *RenderBackend(), sCRC.mFog, parameters );
      }
      break;
   }

   inherited::BuildRuntimeParameters(object, parameterType, colorSpace, parameters);
}
