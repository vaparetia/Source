//------------------------------------------------------------------------------------------
// CGlowShader.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Renderer/Base/Material/CShader.h"
#include "Renderer/Base/BPERendererAPI.h"

//------------------------------------------------------------------------------------------

class CGlowShaderCRCs : public CShaderCRCs
{
public:
   CGlowShaderCRCs();

   uint32   mOutputDimensions;
   uint32   mColorModulate;
};

//------------------------------------------------------------------------------------------

class CGlowShader : public CShader
{
   typedef CShader inherited;
public:
   explicit CGlowShader();

   virtual void                  Bind(CDrawableRenderEntity const * pFirstObject);
   virtual void                  BuildRuntimeParameters(int const pass, SRenderEntry const * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   virtual uint64                GetSortKey(SRenderEntry const * object, uint32 const uniqueId) const;

private:
   static CGlowShaderCRCs        sCRC;
};

