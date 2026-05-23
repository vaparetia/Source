//------------------------------------------------------------------------------------------
// CUnlitGeneric.h
// Copyright 2007
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Material/CShader.h"

//------------------------------------------------------------------------------------------

class CCPLdrShaderUnlitGeneric;

//----------------------------------------------------------------------------

class CUnlitGenericCRCs : public CShaderCRCs
{
public:
   CUnlitGenericCRCs();

   uint32   mTexture;
   uint32   mTextureModulate;
   uint32   mTextureAdd;
   uint32   mUvOffset;
};

//------------------------------------------------------------------------------------------

class CUnlitGeneric : public CShader
{
   typedef CShader inherited;
public:
   explicit CUnlitGeneric(TShaderProperties const &pProperties);

   virtual void                  Bind(CMaterialFlags const * const pMaterialFlags);
   virtual bool                  HasAlpha(CMaterialFlags const &materialFlags) const;

   virtual void                  InitializeEvaluators(CEvaluatorUpdateData &updateData) const;

   virtual void                  BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   virtual void                  BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   
   static CUnlitGenericCRCs      sCRC;
};

