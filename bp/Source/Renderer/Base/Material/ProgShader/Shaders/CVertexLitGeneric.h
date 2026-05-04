//------------------------------------------------------------------------------------------
// CVertexLitGeneric.h
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "boost/optional.hpp"
#include "Engine/Resource/CResourceManager.h"
#include "Engine/Graphics/CColorf.h"

#include "Renderer/Base/Material/CShader.h"
#include "Renderer/Base/Material/ProgShader/Shaders/PSShaderHelpers.h"

//------------------------------------------------------------------------------------------

class CCPLdrShaderDefaultShader;
class CBaseTexture;

//----------------------------------------------------------------------------

class CVertexLitGenericCRCs : public CShaderCRCs
{
public:
   CVertexLitGenericCRCs();

   uint32   mDiffuse;
   uint32   mDiffuseTexture;

   uint32   mSpecular;
   uint32   mSpecularTexture;
   uint32   mSpecularLookupTexture;

   uint32   mIncandescence;
   uint32   mIncandescenceTexture;

   uint32   mIncandescence2;
   uint32   mIncandescence2Texture;

   uint32   mNormalTexture;

   uint32   mAmbient;

   uint32   mUVOffset;

   uint32   mDynamicLightsTextureDir;
   uint32   mDynamicLightsTextureColor;

   uint32   mEnvMapTexture;
   uint32   mEnvMapColor;
   uint32   mEnvMapMaskTexture;
};

//------------------------------------------------------------------------------------------

class CVertexLitGeneric : public CShader
{
   typedef CShader inherited;
public:
   CVertexLitGeneric(inherited::TShaderProperties const &pProperties);

   virtual void                  Bind(CMaterialFlags const * const pMaterialFlags);
   virtual void                  BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   virtual void                  BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);

   virtual bool                  HasAlpha(CMaterialFlags const &materialFlags) const;

   virtual void                  InitializeEvaluators(CEvaluatorUpdateData &updateData) const;

private:
   void                          BuildIrradianceMap(CLightState const * const pLightState) const;

protected:
   static CVertexLitGenericCRCs  sCRC;

   PSShaderHelpers::ELightmapType               mLightmapType;
   CVector3                                     mAmbientLightValue;
};

