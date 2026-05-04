//------------------------------------------------------------------------------------------
// CJKECharacterShader.h
// Armature Studio
// Copyright 2009
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "boost/optional.hpp"

#include "Engine/Graphics/CColorf.h"

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Material/CShader.h"
#include "NJKEShaderCommon.h"

//------------------------------------------------------------------------------------------

class CCPLdrShaderJKECharacter;

class CJKECharacterShaderCRCs : public CShaderCRCs
{
public:
   CJKECharacterShaderCRCs();

   uint32   mSpecularLookupTexture;
   uint32   mDiffuseTexture;

   uint32   mNormalTexture;
   uint32   mGlossTexture;
   uint32   mSpecularity;

   uint32   mAmbientColor;
   uint32   mAmbientLightColor;

   NJKEShaderCommon::SEnvMapParameters mEnvMap;
   NJKEShaderCommon::SFogParameters mFog;

   uint32   mIncandescenceTexture;
   uint32   mIncandescenceToBloom;
};

//------------------------------------------------------------------------------------------

class CJKECharacterShader : public CShader
{
   typedef CShader inherited;
public:
   explicit CJKECharacterShader(TShaderProperties const &pProperties);

   virtual void                  Bind(CMaterialFlags const * const pMaterialFlags);
   virtual void                  BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   virtual void                  BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);

   virtual bool                  HasAlpha(CMaterialFlags const &materialFlags) const;

   virtual void                  InitializeEvaluators(CEvaluatorUpdateData &updateData) const;

protected:
   static CJKECharacterShaderCRCs   sCRC;
   CVector3                         mAmbientValue;
};

