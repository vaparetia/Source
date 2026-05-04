//------------------------------------------------------------------------------------------
// CDefaultShader.h
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "Renderer/Base/Material/CShader.h"
#include "Engine/Resource/CResourceManager.h"
#include "boost/optional.hpp"

//------------------------------------------------------------------------------------------

class CBaseTexture;

//----------------------------------------------------------------------------

class CDefaultShaderCRCs : public CShaderCRCs
{
public:
   CDefaultShaderCRCs();

   uint32   mTextureHandle;
   uint32   mColorHandle;
};

//----------------------------------------------------------------------------
// Simple shader that renders with user specified parameters

class RENDERER_API CDefaultShader : public CShader
{
   typedef CShader inherited;

public:

   enum EBlendMode
   {
      kBM_Replace,
      kBM_Alpha,
      kBM_Additive,
      kBM_PremultipliedAlpha,

      kBM_Invalid = -1
   };

public:
   CDefaultShader();
   ~CDefaultShader();

   virtual void                  Bind(SRenderEntry const * pFirstObject);
   virtual void                  BindPassShader(int const pass, SRenderEntry const * pFirstObject) {}
   virtual void                  UnbindPassShader(int const pass, SRenderEntry const * pFirstObject) {}
   virtual void                  BuildRuntimeParameters(int const pass, SRenderEntry const * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);

   virtual uint64                GetSortKey(SRenderEntry const * object, uint32 const uniqueId) const;

public:
   static CDefaultShaderCRCs                    sCRC;

   CBaseTexture const *                         mpTexture;
   boost::optional<CColorf>                     mConstColor;
   bool                                         mHasColorStream;
   EBlendMode                                   mBlendMode;
   bool                                         mZEnable;
   bool                                         mApplyVertexTransform;
};

//------------------------------------------------------------------------------------------

