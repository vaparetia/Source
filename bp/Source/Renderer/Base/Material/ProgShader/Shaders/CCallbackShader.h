//------------------------------------------------------------------------------------------
// CCallbackShader.h
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#if BPE_RENDERER_TYPE != BPE_RENDERER_TYPE_PROGSHADER
#error This file is Programmable-Shader specific - Wrong platform compiled
#endif

//------------------------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "Engine/Mechanics/FastDelegate/FastDelegate.h"
#include "Renderer/Base/Material/CShader.h"

//------------------------------------------------------------------------------------------

class CDrawableRenderEntity;

//------------------------------------------------------------------------------------------

class RENDERER_API CCallbackShader : public CShader
{
   typedef CShader inherited;
public:
   typedef fastdelegate::FastDelegate3<void *, CShader&, SRenderEntry const *>                                  TBindDelegate;
   typedef fastdelegate::FastDelegate3<void *, CShader&, CDrawableRenderEntity &>                                 TRenderGeometryDelegate;
   typedef fastdelegate::FastDelegate3<void *, EShaderColorSpace const, CShaderParameterBuffer &>                 TBuildMaterialConstantParameterDelegate;
   typedef fastdelegate::FastDelegate5<void *, SRenderEntry const *, EParameterType const, EShaderColorSpace const, CShaderParameterBuffer &>                         
                                                                                                                  TBuildRuntimeParameterDelegate;

public:
   CCallbackShader();
   ~CCallbackShader();

   virtual void                  Bind(SRenderEntry const * pFirstObject);
   virtual void                  BindPassShader(int const pass, SRenderEntry const * pFirstObject) {}
   virtual void                  UnbindPassShader(int const pass, SRenderEntry const * pFirstObject) {}
   virtual void                  BuildMaterialConstantParameters(EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);
   virtual void                  BuildRuntimeParameters(int const pass, SRenderEntry const * object, EParameterType const parameterType, EShaderColorSpace const colorSpace, CShaderParameterBuffer & parameters);

   virtual uint64                GetSortKey(SRenderEntry const * object, uint32 const uniqueId) const;
   
public:
   TBindDelegate                             mBindDelegate;
   TBuildMaterialConstantParameterDelegate   mBuildMaterialConstantParameterDelegate;
   TBuildRuntimeParameterDelegate            mBuildRuntimeParameterDelegate;

   bool                       mSort;
   void *                     mpUserData;
};

//------------------------------------------------------------------------------------------



