//------------------------------------------------------------------------------------------
// PSShaderHelpers.h
// Armature Studio
// Copyright 2009
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

//------------------------------------------------------------------------------------------

class CShaderParameterBuffer;
class CRenderBackend;
class CPOLdrShaderCommonLightmapProperties;

//------------------------------------------------------------------------------------------

namespace PSShaderHelpers
{
   enum ELightmapType
   {
      kLT_None          = 0,
      kLT_Directional   = 1,
      kLT_Flat          = 2,

      kLT_Count
   };

   ELightmapType AddCommonLightmapParameters( CRenderBackend const &backend, CPOLdrShaderCommonLightmapProperties const &commonLightmapProperties, CShaderParameterBuffer &parameters );
}