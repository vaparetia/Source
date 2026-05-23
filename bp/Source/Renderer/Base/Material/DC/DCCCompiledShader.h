//----------------------------------------------------------------------------
// DCCCompiledShader.h
// Dreamcast stub
//----------------------------------------------------------------------------

#pragma once

#include "Engine/Mechanics/IObject.h"

struct SFactoryResourceBuildData;
struct SFactoryReturnResource;

struct SShaderPassData
{
   enum EFlags
   {
      kColorMaskR                   =  (1 << 0),
      kColorMaskG                   =  (1 << 1),
      kColorMaskB                   =  (1 << 2),
      kColorMaskA                   =  (1 << 3),
      kDepthTest                    =  (1 << 4),
      kAlphaToCoverage              =  (1 << 5),
      kWaitForRenderTargetResult    =  (1 << 6),
      kForceDisableAA               =  (1 << 7)
   };

   SShaderPassData()
      :  mData(0)
   {
      mData |= kColorMaskR|kColorMaskG|kColorMaskB|kColorMaskA;
      mData |= kDepthTest;
   }

   int GetFlag(EFlags const flag) const  { return mData & flag; }
   void SetFlag(EFlags const flag, bool const value)
   {
      if (value) mData |=  flag;
      else       mData &= ~flag;
   }

   int mData;
};

class CCompiledShader {};
class CCompiledShaderPackage {};
