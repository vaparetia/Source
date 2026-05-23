//----------------------------------------------------------------------------
// CCompiledShaderCacheTypes.cpp
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#include "StdAfx.h"

#include "Engine/Mechanics/CCRC.h"
#include "Renderer/Base/Material/CCompiledShaderCacheTypes.h"

//----------------------------------------------------------------------------

CShaderFileId::CShaderFileId(const char * const pResource)
{
#if BPE_TARGET == BPE_TARGET_PS3
   mResource = pResource;
   mResource = "$/enginesupport/shaders/_ps3" + std::string(pResource + strlen("$/enginesupport/shaders"));
   mResource = mResource.substr(0, mResource.find(".fx")) + ".cfx";
#elif BPE_TARGET == BPE_TARGET_X360
   mResource = pResource;
   mResource = "$/enginesupport/shaders/_360" + std::string(pResource + strlen("$/enginesupport/shaders"));
   mResource = mResource.substr(0, mResource.find(".fx")) + ".cfx";
#elif BPE_TARGET == BPE_TARGET_VITA
   mResource = pResource;
   mResource = "$/enginesupport/shaders/_vta" + std::string(pResource + strlen("$/enginesupport/shaders"));
   mResource = mResource.substr(0, mResource.find(".fx")) + ".cfx";
#elif BPE_TARGET == BPE_TARGET_WIN32
   mResource = pResource;
#elif BPE_TARGET == BPE_TARGET_DREAMCAST
   mResource = pResource;
#else
#  error Unexpected platform
#endif

   mHash = CCRC::CalculateCRC32AsUpperCaseString(pResource);
}

//----------------------------------------------------------------------------

