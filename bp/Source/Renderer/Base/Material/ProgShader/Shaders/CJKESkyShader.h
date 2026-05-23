//------------------------------------------------------------------------------------------
// CJKESkyShader.h
// Copyright 2009
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "Renderer/Base/Material/CShader.h"

class CJKESkyShaderCRCs : public CShaderCRCs
{
public:
   CJKESkyShaderCRCs();

   uint32   mTexture;
   uint32   mViewportParameters;
   uint32   mDepthTexture;
   uint32   mBloomFromColor;
   uint32   mParameters;
};

//------------------------------------------------------------------------------------------

class CJKESkyShader : public CShader
{
   typedef CShader inherited;
public:
   explicit CJKESkyShader(TShaderProperties const &pProperties);

   virtual void                  Bind(CMaterialFlags const * const pMaterialFlags);
   virtual void                  BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   virtual void                  BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);

   virtual void                  InitializeEvaluators(CEvaluatorUpdateData &updateData) const;

protected:
   static CJKESkyShaderCRCs      sCRC;
};

