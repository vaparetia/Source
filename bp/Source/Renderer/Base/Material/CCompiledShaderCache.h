//----------------------------------------------------------------------------
// CCompiledShaderCache.h
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Material/CCompiledShaderCacheTypes.h"

//----------------------------------------------------------------------------

class CBaseTexture;

//----------------------------------------------------------------------------

enum EPassDrawType
{
   kPDT_Mesh,
   kPDT_FullscreenQuad,

   kPDT_Invalid = -1
};

enum EPassRenderTarget
{
   kPRT_FrameBuffer,
   kPRT_FrameBufferMasked,
   kPRT_BackBuffer,
   kPRT_Custom,

   kPRT_Invalid = -1
};

typedef std::map<std::string, boost::shared_ptr<CBaseTexture> > TNameTextureMap;

//----------------------------------------------------------------------------

class CBaseCompiledShaderPass
{
public:
   CBaseCompiledShaderPass();
   virtual ~CBaseCompiledShaderPass();

   void SetPassScript(std::string const & passScript);
   
   void EvaluatePassScript();
   void ResetPassScript();

public:
   EPassDrawType     mDrawType;
   EPassRenderTarget mRenderTarget;
   bool              mRenderTargetMask[4];
   
   CBaseTexture *    mpCustomTarget;
   std::string       mCustomTarget;
};

//----------------------------------------------------------------------------

class RENDERER_API CBaseCompiledShaderCache
{
public:
   CBaseCompiledShaderCache();
   virtual ~CBaseCompiledShaderCache();

   CBaseTexture * GetRenderTarget(const char * renderTargetTextureName) const;

   void ClearRenderTargets();

public:
   TNameTextureMap   mNameTextureMap;
};

//----------------------------------------------------------------------------

#include BPE_PLATFORM_PATH2(Renderer/Base/Material, CCompiledShaderCache.h)

//----------------------------------------------------------------------------

