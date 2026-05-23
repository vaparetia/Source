//----------------------------------------------------------------------------
// CCompiledShaderCacheTypes.h
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

//----------------------------------------------------------------------------

class CDrawableRenderEntity;

//----------------------------------------------------------------------------

class RENDERER_API CShaderFileId
{
public:
   explicit CShaderFileId(const char * const pResource);

public:
   std::string mResource;
   uint32      mHash;
};


//----------------------------------------------------------------------------
