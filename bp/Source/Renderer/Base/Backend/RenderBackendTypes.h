//------------------------------------------------------------------------------------------
// RenderBackendTypes.h
// Bluepoint
// Copyright 2008
//------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/Graphics/CColor.h"
#include "Engine/Graphics/CColorf.h"

//----------------------------------------------------------------------------

enum ERenderMemory
{
   kRM_Video     = 0,  // CELL_GCM_LOCATION_LOCAL
   kRM_System    = 1,  // CELL_GCM_LOCATION_MAIN
   kRM_Invalid   = -1
};

//----------------------------------------------------------------------------

enum ERenderResourceThreadUser
{
   kRRTU_Update,
   kRRTU_Render
};

//------------------------------------------------------------------------------------------

// this enum must match up with the defines in shader_color_space.fxh in the shaders folder
enum EShaderColorSpace
{
   kSCS_Gamma,
   kSCS_Linear,

   kSCS_Count
};

// this enum must match up with the defines in framebuffer_format.fxh in the shaders folder
enum EShaderFramebufferFormat
{
   kSFF_Gamma,
   kSFF_BlastfactorHdr,
   kSFF_JKE
};

//----------------------------------------------------------------------------

namespace NShaderColorSpaceConversion
{
   static real32 const kShaderGammaValue = 2.0f;

//----------------------------------------------------------------------------

   __inline CColor const ConvertGammaToShaderColorSpace(CColor const & value, EShaderColorSpace const colorSpace)
   {
      if (colorSpace == kSCS_Linear)
         return value.Pow(kShaderGammaValue);

      return value;
   }

//----------------------------------------------------------------------------

   __inline CColorf const ConvertGammaToShaderColorSpace(CColorf const & value, EShaderColorSpace const colorSpace)
   {
      if (colorSpace == kSCS_Linear)
         return CColorf( powf(value.GetR(), kShaderGammaValue), powf(value.GetG(), kShaderGammaValue), powf(value.GetB(), kShaderGammaValue), value.GetA() );

      return value;
   }

//----------------------------------------------------------------------------

   __inline CVector3 const ConvertGammaToShaderColorSpace(CVector3 const & value, EShaderColorSpace const colorSpace)
   {
      if (colorSpace == kSCS_Linear)
         return CVector3( powf(value[kAX], kShaderGammaValue), powf(value[kAY], kShaderGammaValue), powf(value[kAZ], kShaderGammaValue) );

      return value;
   }
}

//----------------------------------------------------------------------------