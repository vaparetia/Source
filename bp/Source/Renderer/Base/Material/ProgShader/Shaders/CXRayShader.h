//------------------------------------------------------------------------------------------
// CXRayShader.h
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "boost/optional.hpp"

#include "Engine/Graphics/CColorf.h"

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Material/CShader.h"

class CXRayShaderCRCs : public CShaderCRCs
{
public:
   CXRayShaderCRCs();

   uint32   mColor;
   uint32   mFalloff;
};

//------------------------------------------------------------------------------------------

class CXRayShader : public CShader
{
   typedef CShader inherited;
public:
   explicit CXRayShader(TShaderProperties const &pProperties);

   virtual void                  Bind(CMaterialFlags const * const pMaterialFlags);
   virtual void                  BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   virtual void                  BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);

   virtual bool                  HasAlpha(CMaterialFlags const &materialFlags) const;
   virtual void                  InitializeEvaluators(CEvaluatorUpdateData &updateData) const;

private:
   static CXRayShaderCRCs        sCRC;
};

