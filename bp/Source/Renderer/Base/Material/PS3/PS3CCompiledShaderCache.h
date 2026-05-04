//----------------------------------------------------------------------------
// PS3CCompiledShaderCache.h
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/Material/PS3/PS3CCompiledShader.h"

#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Primitive/CVertexData.h"
#include "Engine/Resource/CResourceManager.h"

#include "Renderer/Base/Backend/CRenderHWAllocator.h"

//----------------------------------------------------------------------------

class CCompiledShader;
class CCompiledShaderPackage;

//----------------------------------------------------------------------------

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

   CCompiledShader * GetShader(CShaderFileId const &shaderFileId, char const * const pDefines);
   CCompiledShader * GetShader(CShaderFileId const &shaderFileId, uint32 const definesCRC, char const * const pDefines);
   void LoadShaderFromMemory(CShaderFileId const &shaderFileId, char const * const memBuffer, uint32 const memBufferSize);
   void  FlushAllShaders();

   CCompiledShader * CurrentShader() { return mpCurrentShader; }

   void ValidateState();

private:
   void FreeShaders();

public:
   CCompiledShader*  mpCurrentShader;
   TShaderMap        mShaders;

   bool              mColorMask[4];
   bool              mAlphaToCoverage;
   bool              mForceDisableAA;
};

//----------------------------------------------------------------------------
