//----------------------------------------------------------------------------
// CHTShader.h
// Bluepoint
// Copyright 2008
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "boost/optional.hpp"

#include "Engine/Graphics/CColorf.h"

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Material/CShader.h"
#include "Renderer/Base/Material/ProgShader/Shaders/PSShaderHelpers.h"

//----------------------------------------------------------------------------
class CHTShaderCRCs : public CShaderCRCs
{
public:
   CHTShaderCRCs();

   uint32   mDiffuse;
   uint32   mDiffuseTexture;

   uint32   mSpecular;
   uint32   mSpecularTexture;
   uint32   mSpecularLookupTexture;

   uint32   mIncandescence;
   uint32   mIncandescenceTexture;

   uint32   mNormalTexture;

   uint32   mAmbient;

   uint32   mEnvMapTexture;
   uint32   mEnvMapColor;
   uint32   mEnvMapMaskTexture;

   uint32   mBloomModulationEval;
   uint32   mBloomFactors;

   uint32   mUVOffset;
   uint32   mUVScale;

   uint32   mDirectionalLight;
   uint32   mFadeParams;

   uint32   mNoiseTexture;
};

//----------------------------------------------------------------------------

class CHTShader : public CShader
{
   typedef CShader inherited;
public:
   explicit CHTShader(TShaderProperties const &pProperties);

   virtual void   Bind(CMaterialFlags const * const pMaterialFlags);
   virtual void   BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   virtual void   BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);

   virtual bool   HasAlpha(CMaterialFlags const &materialFlags) const;

   virtual void   InitializeEvaluators(CEvaluatorUpdateData &updateData) const;

protected:
   void           BindBackground(CMaterialFlags const * const pMaterialFlags);
   void           BindCharacter(CMaterialFlags const * const pMaterialFlags);
   void           BindEnvironment(CMaterialFlags const * const pMaterialFlags);
   void           BindHolographic(CMaterialFlags const * const pMaterialFlags);
   void           BindLightshaft(CMaterialFlags const * const pMaterialFlags);
   void           BindTransparent(CMaterialFlags const * const pMaterialFlags);

   void           BuildMaterialConstantParametersBackground(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);

   void           BuildMaterialConstantParametersCharacter(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   void           BuildRuntimeParametersCharacter(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   
   void           BuildMaterialConstantParametersEnvironment(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   void           BuildRuntimeParametersEnvironment(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   
   void           BuildMaterialConstantParametersHolographic(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   void           BuildRuntimeParametersHolographic(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   
   void           BuildMaterialConstantParametersLightshaft(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   void           BuildRuntimeParametersLightshaft(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   
   void           BuildMaterialConstantParametersTransparent(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   void           BuildRuntimeParametersTransparent(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);

protected:
   static CHTShaderCRCs    sCRC;
protected:
   PSShaderHelpers::ELightmapType   mLightmapType;
   CVector3                         mAmbientValue;
};

