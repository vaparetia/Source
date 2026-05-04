//------------------------------------------------------------------------------------------
// CUnlitGeneric.cpp
// Copyright 2007
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CUnlitGeneric.h"

//------------------------------------------------------------------------------------------

#include "Engine/Evaluators/CEvaluatorUpdateData.h"
#include "Engine/Evaluators/IEvaluator.h"

#include "Renderer/Base/Frontend/RenderObject/CDrawableRenderEntity.h"
#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/ShaderObjects/Shader/CCPLdrShaderUnlitGeneric.h"

//------------------------------------------------------------------------------------------

using namespace std;
CUnlitGenericCRCs   CUnlitGeneric::sCRC;

//------------------------------------------------------------------------------------------

CUnlitGenericCRCs::CUnlitGenericCRCs()
:  mTexture(GetParameterCRC("g_Texture"))
,  mTextureModulate(GetParameterCRC("g_TextureModulate"))
,  mTextureAdd(GetParameterCRC("g_TextureAdd"))
,  mUvOffset(GetParameterCRC("g_UvOffset"))
{
}

//------------------------------------------------------------------------------------------

CUnlitGeneric::CUnlitGeneric(TShaderProperties const &pProperties)
: inherited(pProperties)
{
   BPE_VERIFY( GetProperties()->GetComponentType() == CCPLdrShaderUnlitGeneric::kComponentPropertiesType, false, "invalid property data type" );
}

//------------------------------------------------------------------------------------------

bool CUnlitGeneric::HasAlpha(CMaterialFlags const &materialFlags) const
{
   // As this is a bit of a special case shader, we use the 'HasAlpha' as a draw order override (alpha draws after opaque).   
   // We might want to revisit this later as this is a bit of a work around.
   // We could also re-factor this function so that it returns an 'int' that determines inter-shader/model sort order.
   // This would work fine for inter model, as material flags render order is a higher sort priority that 'HasAlpha'.   
   CCPLdrShaderUnlitGeneric const * const pProperties = static_cast<CCPLdrShaderUnlitGeneric const * const>( GetProperties() );
   bool const bDrawLast = (pProperties->mDrawPriority == CCPLdrShaderUnlitGeneric::kDP_1);
   return bDrawLast;
}

//------------------------------------------------------------------------------------------

void CUnlitGeneric::InitializeEvaluators(CEvaluatorUpdateData &updateData) const
{
   CCPLdrShaderUnlitGeneric const * const pProperties = static_cast<CCPLdrShaderUnlitGeneric const * const>( GetProperties() );
   pProperties->InitializeEvaluators(updateData);
}

//------------------------------------------------------------------------------------------

void CUnlitGeneric::Bind(CMaterialFlags const * const pMaterialFlags)
{
   inherited::Bind(pMaterialFlags);

   CCPLdrShaderUnlitGeneric const * const pProperties = static_cast<CCPLdrShaderUnlitGeneric const * const>(GetProperties());

   CMaterialFlags const materialFlags = pMaterialFlags ? *pMaterialFlags : CMaterialFlags::Normal();

   // setup vertex format
   SetShaderVertexDataBinding(CShaderVertexDataBinding(kVDU_Position, kVDU_TexCoord0));

   // set shader
   SScreenOutputControl const & outputControl = RenderBackend()->GetScreenOutputControl();
   static const CShaderFileId skShaderFileId("$/enginesupport/shaders/UnlitGeneric.fx");
   SetShader(skShaderFileId,
             CStringExtras::StringizeInt_s("DEPTHCHECK=%d;DEPTHWRITE=%d;BLENDMODE=%d;SHADER_COLOR_SPACE=%d;FRAMEBUFFER_FORMAT=%d", 
                                           pProperties->mDepthCompare ? 1 : 0,
                                           pProperties->mDepthWrite ? 1 : 0,
                                           pProperties->mBlendMode,
                                           outputControl.GetShaderColorSpace(),
                                           outputControl.GetFrameBufferFormat()));
}

//----------------------------------------------------------------------------

void CUnlitGeneric::BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderUnlitGeneric const * const properties = static_cast<CCPLdrShaderUnlitGeneric const * const>( GetProperties() );
   
   parameters.AddTexture(sCRC.mTexture, properties->mTexture_Resource ? properties->mTexture_Resource->GetPtr() : &gpRenderBackend->GetBlackMap());
   
   inherited::BuildMaterialConstantParameters(colorSpace, parameters);
}

//----------------------------------------------------------------------------

void CUnlitGeneric::BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   if( parameterType == kPT_Unshared )
   {
      CCPLdrShaderUnlitGeneric const * const properties = static_cast<CCPLdrShaderUnlitGeneric const * const>( GetProperties() );
      CEvaluatorUpdateData updateData = object->GetUpdateData(this);

      CColorf const textureModulate = properties->mColorModulate->GetValue(updateData);
      parameters.AddParameterGamma(sCRC.mTextureModulate, textureModulate, colorSpace);

      CColorf const textureAdd = properties->mColorAdd->GetValue(updateData);
      parameters.AddParameterGamma(sCRC.mTextureAdd, textureAdd, colorSpace);

      CVector3 const uvOffset = properties->mUvOffset ? properties->mUvOffset->GetValue(updateData) : CVector3::Zero();
      parameters.AddParameter(sCRC.mUvOffset, uvOffset);
   }

   inherited::BuildRuntimeParameters(object, parameterType, colorSpace, parameters);
}
