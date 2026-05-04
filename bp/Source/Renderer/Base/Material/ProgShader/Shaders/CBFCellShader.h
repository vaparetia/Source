//------------------------------------------------------------------------------------------
// CBFCellShader.h
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "boost/optional.hpp"

#include "Engine/Graphics/CColorf.h"

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Material/CShader.h"

//------------------------------------------------------------------------------------------

class CBFCellShaderCRCs : public CShaderCRCs
{
public:
   CBFCellShaderCRCs();

   uint32  mColor;

   uint32  mLightOffset;

   uint32  mColorTexture;
   uint32  mLayer1UvOffset;
   uint32  mLayer1UvScale;

   uint32  mLayer2Color;
   uint32  mLayer2Texture;
   uint32  mLayer2UvOffset;
   uint32  mLayer2UvScale;

   uint32  mLayer3Texture;
   uint32  mLayer3UvOffset;
   uint32  mLayer3UvScale;

   uint32  mLayer4Texture;
   uint32  mLayer4UvOffset;
   uint32  mLayer4UvScale;

   uint32  mModulateTexture;

   uint32  mExtraColor1;
   uint32  mExtraColor2;
   uint32  mExtraColor3;
};

//------------------------------------------------------------------------------------------

class CBFCellShader : public CShader
{
   typedef CShader inherited;
public:
   explicit CBFCellShader(TShaderProperties const &pProperties);

   virtual void                  Bind(CMaterialFlags const * const pMaterialFlags);
   virtual void                  RenderGeometry(CDrawableRenderEntity& object);
   virtual bool                  HasAlpha(CMaterialFlags const &materialFlags) const;

   virtual void                  InitializeEvaluators(CEvaluatorUpdateData &updateData) const;

   virtual void                  BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   virtual void                  BuildRuntimeParameters(CDrawableRenderEntity * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);

protected:
   static CBFCellShaderCRCs      sCRC;
};

