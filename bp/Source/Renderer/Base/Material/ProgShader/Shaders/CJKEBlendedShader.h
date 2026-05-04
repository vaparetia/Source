//------------------------------------------------------------------------------------------
// CJKEBlendedShader.h
// Armature Studio
// Copyright 2009
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "boost/optional.hpp"

#include "Engine/Graphics/CColorf.h"

#include "Renderer/Base/Material/ProgShader/Shaders/NJKEShaderCommon.h"
#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Material/CShader.h"
#include "Renderer/Base/Material/ProgShader/Shaders/PSShaderHelpers.h"

//------------------------------------------------------------------------------------------

class CCPLdrShaderJKEBlended;

class CJKEBlendedShaderCRCs : public CShaderCRCs
{
public:
   CJKEBlendedShaderCRCs();

   uint32   mDiffuseTexture1;
   uint32   mDiffuseTexture2;

   uint32   mNormalTexture1;
   uint32   mNormalTexture2;

   uint32   mGloss1;
   uint32   mGloss2;

   uint32   mBlendTexture;

   uint32   mAmbientColor;
   uint32   mAmbientLightColor;

   NJKEShaderCommon::SEnvMapParameters mEnvMap;
   NJKEShaderCommon::SFogParameters mFog;

};

//------------------------------------------------------------------------------------------

class CJKEBlendedShader : public CShader
{
   typedef CShader inherited;
public:
   explicit CJKEBlendedShader(TShaderProperties const &pProperties);

   virtual void                  Bind(CMaterialFlags const * const pMaterialFlags);
   virtual void                  BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   virtual void                  BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);

   virtual bool                  HasAlpha(CMaterialFlags const &materialFlags) const;
   virtual uint64                GetDrawOrder(CDrawableRenderEntity const & object) const;
   virtual void                  InitializeEvaluators(CEvaluatorUpdateData &updateData) const;

protected:
   static CJKEBlendedShaderCRCs  sCRC;

   PSShaderHelpers::ELightmapType   mLightmapType;
   CVector3                         mAmbientValue;
};

