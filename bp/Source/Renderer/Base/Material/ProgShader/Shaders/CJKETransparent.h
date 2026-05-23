//------------------------------------------------------------------------------------------
// CJKETransparent.h
// Copyright 2009
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "Renderer/Base/Material/CShader.h"
#include "Renderer/Base/Material/ProgShader/Shaders/NJKEShaderCommon.h"
#include "Renderer/Base/Material/ProgShader/Shaders/PSShaderHelpers.h"

//------------------------------------------------------------------------------------------

class CJKETransparentCRCs : public CShaderCRCs
{
public:
   CJKETransparentCRCs();

   uint32   mDiffuseTexture;
   uint32   mNormalTexture;
   uint32   mIncandescenceTexture;

   uint32   mAmbientColor;

   NJKEShaderCommon::SFogParameters mFog;
};

//------------------------------------------------------------------------------------------

class CJKETransparent : public CShader
{
   typedef CShader inherited;
public:
   explicit CJKETransparent(TShaderProperties const &pProperties);

   virtual void   Bind(CMaterialFlags const * const pMaterialFlags);
   virtual void   BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   virtual void   BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);

   virtual bool   HasAlpha(CMaterialFlags const &materialFlags) const;
   virtual uint64 GetDrawOrder(CDrawableRenderEntity const & object) const;

protected:
   static CJKETransparentCRCs    sCRC;

   PSShaderHelpers::ELightmapType   mLightmapType;
   CVector3                         mAmbientValue;
};

