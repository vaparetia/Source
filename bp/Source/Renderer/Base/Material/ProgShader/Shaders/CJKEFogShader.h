//------------------------------------------------------------------------------------------
// CJKEFogShader.h
// Copyright 2009
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "Renderer/Base/Material/CShader.h"

class CJKEFogShaderCRCs : public CShaderCRCs
{
public:
   CJKEFogShaderCRCs();

   uint32   mTexture;
   uint32   mColor;
   uint32   mMaxAmount;
   uint32   mNearFar;
   uint32   mDepthTexture;
   uint32   mWorldView;
   uint32   mUVOffsetScale;
};

//------------------------------------------------------------------------------------------

class CJKEFogShader : public CShader
{
   typedef CShader inherited;
public:
   explicit CJKEFogShader(TShaderProperties const &pProperties);

   virtual void                  Bind(CMaterialFlags const * const pMaterialFlags);
   virtual void                  BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   virtual void                  BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   virtual bool                  HasAlpha(CMaterialFlags const &materialFlags) const;

   virtual void                  InitializeEvaluators(CEvaluatorUpdateData &updateData) const;

protected:
   static CJKEFogShaderCRCs      sCRC;
};

