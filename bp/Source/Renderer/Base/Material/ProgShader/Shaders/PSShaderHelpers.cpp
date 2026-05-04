//------------------------------------------------------------------------------------------
// PSShaderHelpers.h
// Armature
// Copyright 2009
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "PSShaderHelpers.h"
#include "Renderer/Base/ShaderObjects/Properties/CPOLdrShaderCommonLightmapProperties.h"
#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"
#include "Renderer/Base/Material/CShader.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

//------------------------------------------------------------------------------------------

namespace
{
   class CLightmapCRCs : public CShaderCRCs
   {
   public:
      CLightmapCRCs()
      : mLightMapXTexture(GetParameterCRC("g_LightMapXTexture"))
      , mLightMapYTexture(GetParameterCRC("g_LightMapYTexture"))
      , mLightMapZTexture(GetParameterCRC("g_LightMapZTexture"))
      , mLightMapFlatTexture(GetParameterCRC("g_LightMapFlatTexture"))
      {
      };

      uint32   mLightMapXTexture;
      uint32   mLightMapYTexture;
      uint32   mLightMapZTexture;
      uint32   mLightMapFlatTexture;
   };

   CLightmapCRCs  sLightmapCRCs;
}  // anon namespace

//------------------------------------------------------------------------------------------

PSShaderHelpers::ELightmapType PSShaderHelpers::AddCommonLightmapParameters(CRenderBackend const &backend, 
                                                                            CPOLdrShaderCommonLightmapProperties const &commonLightmapProperties, 
                                                                            CShaderParameterBuffer &parameters)
{
   if (commonLightmapProperties.mLightmapTexture_X_Resource)
   {
      // Have 'X' lightmap texture resource, assume directional lighting
      parameters.AddTexture(sLightmapCRCs.mLightMapXTexture, commonLightmapProperties.mLightmapTexture_X_Resource->GetPtr(), true, true );
      parameters.AddTexture(sLightmapCRCs.mLightMapYTexture, commonLightmapProperties.mLightmapTexture_Y_Resource->GetPtr(), true, true );
      parameters.AddTexture(sLightmapCRCs.mLightMapZTexture, commonLightmapProperties.mLightmapTexture_Z_Resource->GetPtr(), true, true );
      return kLT_Directional;
   }

   if (commonLightmapProperties.mLightmapTexture_Flat_Resource)
   {
      // Have 'flat' lightmap texture resource, assume flat lighting
      parameters.AddTexture(sLightmapCRCs.mLightMapFlatTexture, commonLightmapProperties.mLightmapTexture_Flat_Resource->GetPtr(), true, true );

      return kLT_Flat;
   }

   // Not using lightmaps
   return kLT_None;
}

//------------------------------------------------------------------------------------------

