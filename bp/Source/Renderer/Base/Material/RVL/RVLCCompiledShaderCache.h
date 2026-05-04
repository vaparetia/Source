//----------------------------------------------------------------------------
// RVLCCompiledShaderCache.h
// Bluepoint
// Copyright 2009
//----------------------------------------------------------------------------

#pragma once

#include "Renderer/Base/Material/CCompiledShaderCache.h"

//----------------------------------------------------------------------------

class CCompiledShaderPass : public CBaseCompiledShaderPass
{
public:
   CCompiledShaderPass()
   {
   }
};

//----------------------------------------------------------------------------

class CCompiledShader
{
public:
   // Start using compiled shader, returns number of passes
   int const Begin();
   // Finish compiled shader
   void End();

   // Start using pass, returns pass type
   int const BeginPass(int const currentPass);
   void EndPass();

   void BeginShaderBatch(uint8* pMaterialConstantParameters, int const materialConstantParametersSize,
      uint8* pMaterialRuntimeParametersShared, int const materialRuntimeParametersSharedSize,
      uint8* pMaterialRuntimeParametersUnshared, int const materialRuntimeParametersUnsharedSize);

   void EndShaderBatch();

   int const GetCurrentPassIndex() const;
   int const GetPassCount() const;
};

//----------------------------------------------------------------------------

class RENDERER_API CCompiledShaderCache : public CBaseCompiledShaderCache
{
public:
   CCompiledShaderCache();
   virtual ~CCompiledShaderCache();

   void  SetShader(CShaderFileId const &shaderFileId, char const * const pDefines);
   CCompiledShader * CurrentShader();
   void  FlushAllShaders();

};

//----------------------------------------------------------------------------

