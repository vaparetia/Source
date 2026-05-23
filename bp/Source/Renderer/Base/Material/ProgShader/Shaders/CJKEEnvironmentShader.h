//------------------------------------------------------------------------------------------
// CJKEEnvironmentShader.h
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
#include "Renderer/Base/Material/ProgShader/Shaders/PSShaderHelpers.h"

//------------------------------------------------------------------------------------------

class CCPLdrShaderJKEEnvironment;

class CJKEEnvironmentShaderCRCs : public CShaderCRCs
{
public:
   CJKEEnvironmentShaderCRCs();

   uint32   mDiffuseTexture;

   uint32   mNormalTexture;
   uint32   mGlossTexture;

   uint32   mAmbientColor;
   uint32   mAmbientLightColor;

   NJKEShaderCommon::SEnvMapParameters mEnvMap;
   NJKEShaderCommon::SFogParameters mFog;

   uint32   mIncandescenceTexture;
   uint32   mIncandescenceToBloom;
};

//------------------------------------------------------------------------------------------

class CJKEEnvironmentShader : public CShader
{
   typedef CShader inherited;
public:
   explicit CJKEEnvironmentShader(TShaderProperties const &pProperties);

   virtual void                  Bind(CMaterialFlags const * const pMaterialFlags);
   virtual void                  BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   virtual void                  BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);

   virtual bool                  HasAlpha(CMaterialFlags const &materialFlags) const;
   virtual uint64                GetDrawOrder(CDrawableRenderEntity const & object) const;

   virtual void                  InitializeEvaluators(CEvaluatorUpdateData &updateData) const;

protected:
   static CJKEEnvironmentShaderCRCs    sCRC;

   PSShaderHelpers::ELightmapType   mLightmapType;
   CVector3                         mAmbientValue;
};

