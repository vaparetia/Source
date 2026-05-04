//------------------------------------------------------------------------------------------
// CXRayShader.cpp
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CXRayShader.h"

//------------------------------------------------------------------------------------------

#include "Engine/Evaluators/IEvaluator.h"

#include "Renderer/Base/Frontend/RenderObject/CDrawableRenderEntity.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"

#include "Renderer/Base/ShaderObjects/Shader/CCPLdrShaderXRay.h"

//------------------------------------------------------------------------------------------

using namespace std;
CXRayShaderCRCs   CXRayShader::sCRC;

//------------------------------------------------------------------------------------------

CXRayShaderCRCs::CXRayShaderCRCs()
:  mColor(GetParameterCRC("XRayColor"))
,  mFalloff(GetParameterCRC("XRayFalloff"))
{
}

//------------------------------------------------------------------------------------------

CXRayShader::CXRayShader(TShaderProperties const &pProperties)
: inherited(pProperties)
{
   BPE_VERIFY( GetProperties()->GetComponentType() == CCPLdrShaderXRay::kComponentPropertiesType, false, "invalid property data type" );
}

//------------------------------------------------------------------------------------------

bool CXRayShader::HasAlpha(CMaterialFlags const &materialFlags) const
{
   if( inherited::HasAlpha(materialFlags) )
      return true;

   return false;
}

//------------------------------------------------------------------------------------------

void CXRayShader::InitializeEvaluators(CEvaluatorUpdateData &updateData) const
{
   CCPLdrShaderXRay const * const pProperties = static_cast<CCPLdrShaderXRay const * const>( GetProperties() );
   pProperties->InitializeEvaluators(updateData);
}

//------------------------------------------------------------------------------------------

void CXRayShader::Bind(CMaterialFlags const * const pMaterialFlags)
{
   inherited::Bind(pMaterialFlags);

   // cache some ptrs to make code more readable

   CCPLdrShaderXRay const * const pProperties = static_cast<CCPLdrShaderXRay const * const>( GetProperties() );

   CMaterialFlags const materialFlags = pMaterialFlags ? *pMaterialFlags : CMaterialFlags::Normal();

   // setup vertex format
   CShaderVertexDataBinding binding(kVDU_Position, kVDU_Normal);
   SetShaderVertexDataBinding(binding);

   EBlendMode blendMode = GetBlendMode(materialFlags);

   // set shader
   static const CShaderFileId skShaderFileId("$/enginesupport/shaders/XRay.fx");
   SetShader(skShaderFileId, CStringExtras::StringizeInt_s("BLENDMODE=%d", blendMode));
}

//----------------------------------------------------------------------------

void CXRayShader::BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderXRay const * const pProperties = static_cast<CCPLdrShaderXRay const * const>( GetProperties() );

   parameters.AddParameter(sCRC.mFalloff, pProperties->mFalloff);

   inherited::BuildMaterialConstantParameters(colorSpace, parameters);
}

//----------------------------------------------------------------------------

void CXRayShader::BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters)
{
   CCPLdrShaderXRay const * const pProperties = static_cast<CCPLdrShaderXRay const * const>( GetProperties() );

   if( parameterType == kPT_Unshared )
   {
      CEvaluatorUpdateData updateData = object->GetUpdateData(this);
      CColorf const color = pProperties->mColor->GetValue(updateData);

      parameters.AddParameterGamma(sCRC.mColor, color, colorSpace);
   }

   inherited::BuildRuntimeParameters(object, parameterType, colorSpace, parameters);
}
