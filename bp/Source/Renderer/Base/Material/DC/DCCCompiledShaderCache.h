//----------------------------------------------------------------------------
// DCCCompiledShaderCache.h
// Dreamcast stub
//----------------------------------------------------------------------------

#pragma once

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Material/CCompiledShaderCacheTypes.h"
#include "Renderer/Base/Material/DC/DCCCompiledShader.h"
#include "Engine/Resource/CResourceManager.h"

class RENDERER_API CCompiledShaderCache : public CBaseCompiledShaderCache
{
public:
   typedef std::map<uint32, TResource<CCompiledShaderPackage> > TShaderMap;
public:
   CCompiledShaderCache();
   virtual ~CCompiledShaderCache();

   void  SetShader(CShaderFileId const &shaderFileId, char const * const pDefines)               {}
   void  SetShader(CShaderFileId const &shaderFileId, uint32 const definesCRC, char const * const pDefines) {}
   void  SetShader(CCompiledShader const *pCurrentShader) { mpCurrentShader = const_cast<CCompiledShader *>(pCurrentShader); }

   CCompiledShader * CurrentShader()  { return mpCurrentShader; }
   CCompiledShader * GetShader(CShaderFileId const &, char const * const)              { return NULL; }
   CCompiledShader * GetShader(CShaderFileId const &, uint32 const, char const * const){ return NULL; }
   void LoadShaderFromMemory(CShaderFileId const &, char const * const, uint32 const)  {}

   void  FlushAllShaders() {}
   void  FreeShaders()     {}
   void  ValidateState()   {}

   static void PushInhibitShaderWarning() {}
   static void PopInhibitShaderWarning()  {}

private:
   CCompiledShader*  mpCurrentShader;
   TShaderMap        mShaders;

   static int        sInhibitShaderWarningCount;
};
