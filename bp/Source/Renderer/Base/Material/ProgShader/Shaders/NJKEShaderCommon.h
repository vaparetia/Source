//------------------------------------------------------------------------------------------
// NJKEShaderCommon.h
// Armature Studio
// Copyright 2009
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "boost/optional.hpp"
#include "Renderer/Base/BPERendererAPI.h"

//------------------------------------------------------------------------------------------

class CShaderParameterBuffer;
class CRenderBackend;

//------------------------------------------------------------------------------------------

namespace NJKEShaderCommon
{
   typedef uint64 EDrawPriority;

   static uint64 const kDP_First   =  (0ULL << CMaterialFlags::kSSK_MaterialDrawOrderBitPos);
   static uint64 const kDP_Normal  =  (1ULL << CMaterialFlags::kSSK_MaterialDrawOrderBitPos);
   static uint64 const kDP_Last    =  (2ULL << CMaterialFlags::kSSK_MaterialDrawOrderBitPos);

   struct SEnvMapParameters
   {
      SEnvMapParameters();

      uint32   mEnvMapTexture;
      uint32   mGlossFactor;
      uint32   mGlossAdd;
   };

   struct SFogParameters
   {
      SFogParameters();

      uint32 mFogColor;
      uint32 mFogParams;
   };

   void AddFogParameters( CRenderBackend const &backend, SFogParameters const &fog, CShaderParameterBuffer &parameters );
}