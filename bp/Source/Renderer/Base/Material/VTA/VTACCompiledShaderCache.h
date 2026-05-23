//----------------------------------------------------------------------------
// VTACCompiledShaderCache.h
// Copyright 2011
//----------------------------------------------------------------------------

#pragma once

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Material/CCompiledShaderCacheTypes.h"
#include "Renderer/Base/Material/VTA/VTACCompiledShader.h"
#include "Engine/Resource/CResourceManager.h"

class RENDERER_API CCompiledShaderCache : public CBaseCompiledShaderCache
{
public:
   typedef std::map<uint32, TResource<CCompiledShaderPackage> > TShaderMap; // key is CRC hash from CShaderFileId
public:
   CCompiledShaderCache();
   virtual ~CCompiledShaderCache();

   void  SetShader(CShaderFileId const &shaderFileId, char const * const pDefines);
   void  SetShader(CShaderFileId const &shaderFileId, uint32 const definesCRC, char const * const pDefines);
   void  SetShader(CCompiledShader const *  pCurrentShader)      { mpCurrentShader = const_cast<CCompiledShader *>(pCurrentShader); };

   CCompiledShader * CurrentShader() { return mpCurrentShader; }

   CCompiledShader * GetShader(CShaderFileId const &shaderFileId, char const * const pDefines);
   CCompiledShader * GetShader(CShaderFileId const &shaderFileId, uint32 const definesCRC, char const * const pDefines);
   void LoadShaderFromMemory(CShaderFileId const &shaderFileId, char const * const memBuffer, uint32 const memBufferSize);

   void  FlushAllShaders();
   void  FreeShaders();
   void  ValidateState();

   static void PushInhibitShaderWarning() { ++sInhibitShaderWarningCount; }
   static void PopInhibitShaderWarning() { --sInhibitShaderWarningCount; }
private:
   CCompiledShader*  mpCurrentShader;
   TShaderMap        mShaders;

   bool              mColorMask[4];
   bool              mAlphaToCoverage;
   bool              mForceDisableAA;

   static int        sInhibitShaderWarningCount;
};